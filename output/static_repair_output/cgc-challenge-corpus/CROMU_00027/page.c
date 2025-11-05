#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h> // For atoi

// Max length for a PageVar name. Derived from 0x41 (65) in original code,
// implying 64 characters plus null terminator.
#define MAX_PAGEVAR_NAME_LENGTH 64

// Structure to hold page variables.
// The memory access patterns (offsets 0x40 and 0x44) suggest:
// Offset 0x00: name (char array)
// Offset 0x40: value (char*)
// Offset 0x44: next (PageVar*)
// A total size of 0x48 (72 bytes) implies 32-bit pointers (64+4+4=72).
// Using sizeof(PageVar) makes it portable across 32/64-bit systems.
typedef struct PageVar {
    char name[MAX_PAGEVAR_NAME_LENGTH]; // Variable name (null-terminated)
    char *value;                        // Dynamically allocated value string
    struct PageVar *next;               // Pointer to the next PageVar in the list
} PageVar;

// Global variables (from decompiled code)
static char line[0x51]; // Buffer for output lines (81 chars: index 0-80)
static int line_length = 0; // Current content length in 'line' buffer
static int in_a_box = 0;    // Flag indicating if output is currently within a box

// String literals used in comparisons (DAT_00016220, DAT_00016229 in original)
static const char *const_str_line = "line";
static const char *const_str_var = "var";
static const char *const_str_box = "box";

// Forward declarations
void VerifyPointerOrTerminate(void *ptr, const char *msg);
PageVar *GetPageVar(PageVar *list_head, const char *name_start, const char *name_end);
void DestroyVarList(PageVar *list_head);
int AddPageVar(PageVar *list_head, char *full_var_string);
void FlushOutput(void);
void OutputChar(char c);
void OutputStr(const char *str);
int ServePageWithOverride(const char *page_content, int remaining_depth, PageVar *override_list);
void ServePage(const char *page_content, int remaining_depth);
void InteractWithPage(void *context1, void *context2, char *input_string); // Types matched to original undefined4

// Function: VerifyPointerOrTerminate
// Checks if a pointer is NULL, prints an error message, and terminates the program.
void VerifyPointerOrTerminate(void *ptr, const char *msg) {
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: %s allocation failed. Terminating.\n", msg);
        exit(EXIT_FAILURE);
    }
}

// Function: GetPageVar
// Searches for a PageVar by name in a linked list.
// list_head: The head of the PageVar list.
// name_start: Pointer to the start of the name to search for.
// name_end: Pointer to the end of the name to search for (exclusive).
// Returns a pointer to the found PageVar, or NULL if not found.
PageVar *GetPageVar(PageVar *list_head, const char *name_start, const char *name_end) {
    size_t name_len = name_end - name_start;

    PageVar *current = list_head;
    while (current != NULL) {
        // Ensure name length fits, and compare stored name with the segment
        if (name_len < MAX_PAGEVAR_NAME_LENGTH &&
            strlen(current->name) == name_len &&
            strncmp(current->name, name_start, name_len) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: DestroyVarList
// Recursively frees all memory associated with a PageVar linked list.
void DestroyVarList(PageVar *list_head) {
    if (list_head != NULL) {
        if (list_head->next != NULL) {
            DestroyVarList(list_head->next);
            list_head->next = NULL; // Clear pointer after destroying subtree
        }
        if (list_head->value != NULL) {
            free(list_head->value);
            list_head->value = NULL; // Clear pointer after freeing
        }
        free(list_head);
    }
}

// Function: AddPageVar
// Parses a string like "[name:value]" and adds/updates it in a PageVar list.
// list_head: The head of the PageVar list to add to.
// full_var_string: The string containing the variable definition.
// Returns 1 on success, -1 on failure.
int AddPageVar(PageVar *list_head, char *full_var_string) {
    if (full_var_string == NULL || *full_var_string != '[') {
        return -1; // Invalid format, expected to start with '['
    }

    char *name_start = strchr(full_var_string, ':');
    if (!name_start) return -1;
    name_start++; // Skip first ':' to get to actual name

    char *name_end = strchr(name_start, ':');
    if (!name_end) return -1; // Second ':' marks end of name, start of value

    char *value_start = name_end + 1; // Start of value

    char *value_end = strchr(full_var_string, ']');
    if (!value_end) return -1; // ']' marks end of value

    size_t name_len = name_end - name_start;
    size_t value_len = value_end - value_start;

    if (name_len >= MAX_PAGEVAR_NAME_LENGTH) {
        return -1; // Name too long
    }

    PageVar *page_var_ptr = GetPageVar(list_head, name_start, name_end);

    if (page_var_ptr == NULL) {
        // Variable not found, create a new one and append to the list
        PageVar *current = list_head;
        while (current->next != NULL) {
            current = current->next;
        }

        PageVar *new_var = (PageVar *)calloc(1, sizeof(PageVar));
        VerifyPointerOrTerminate(new_var, "New PageVar");
        current->next = new_var;
        page_var_ptr = new_var;
    } else {
        // Variable found, free existing value if any
        if (page_var_ptr->value != NULL) {
            free(page_var_ptr->value);
            page_var_ptr->value = NULL;
        }
    }

    // Copy name
    memcpy(page_var_ptr->name, name_start, name_len);
    page_var_ptr->name[name_len] = '\0'; // Null-terminate the name

    // Allocate and copy value
    page_var_ptr->value = (char *)calloc(value_len + 1, 1);
    VerifyPointerOrTerminate(page_var_ptr->value, "PageVar->value");
    memcpy(page_var_ptr->value, value_start, value_len);
    page_var_ptr->value[value_len] = '\0'; // Null-terminate the value

    return 1; // Success
}

// Function: FlushOutput
// Prints the current content of the 'line' buffer and resets it.
// If 'in_a_box' is true, it prepares a new box border line.
void FlushOutput(void) {
    if (line_length > 0) {
        line[line_length] = '\0'; // Null-terminate for printf
        printf("%s\n", line);
    }

    memset(line, 0, sizeof(line)); // Clear the buffer
    line_length = 0;

    if (in_a_box != 0) {
        memset(line, ' ', 0x50); // Fill line[0] through line[79] with spaces
        line[0] = '*';
        line[0x4f] = '*'; // line[79]
        line[0x50] = '*'; // line[80] (DAT_000195cf in original code)
        line_length = 2; // Represents the two '*' characters at the ends for content tracking
    }
}

// Function: OutputChar
// Appends a character to the 'line' buffer. Flushes if the buffer is full.
void OutputChar(char c) {
    line[line_length] = c;
    line_length++;

    // Flush if the buffer is completely full (80 characters, index 0-79)
    // OR if we are in a box and have reached the maximum content length within the box (index 77)
    if (line_length == 0x50 || (in_a_box != 0 && line_length == 0x4e)) {
        FlushOutput();
    }
}

// Function: OutputStr
// Appends a string to the 'line' buffer. Flushes if necessary or prints directly if too long.
void OutputStr(const char *str) {
    size_t str_len = strlen(str);

    // If string plus current line content exceeds buffer size, flush first
    if (0x50 - line_length < str_len) { // Check if remaining space is less than string length
        FlushOutput();
    }

    // If string fits into the buffer (after potential flush)
    if (str_len < 0x51 - line_length) { // Check if string fits in remaining buffer space
        memcpy(line + line_length, str, str_len);
        line_length += str_len;
    } else {
        // If string is too long for the buffer even after flushing, print directly
        printf("%s\n", str);
    }
}

// Function: ServePageWithOverride
// Processes page content, interpreting special commands and variable substitutions.
// page_content: The string containing the page content.
// remaining_depth: A recursion depth limit.
// override_list: An optional list of PageVar's that override page-local variables.
// Returns 0 on success, -1 on error.
int ServePageWithOverride(const char *page_content, int remaining_depth, PageVar *override_list) {
    PageVar *page_var_list_head = (PageVar *)calloc(1, sizeof(PageVar)); // Dummy head for page-local variables
    VerifyPointerOrTerminate(page_var_list_head, "VarList initialization");

    in_a_box = 0;
    memset(line, 0, sizeof(line));
    line_length = 0;

    int ret_val = 0;
    bool error_occurred = false;

    while (*page_content != '\0' && remaining_depth >= 1 && !error_occurred) {
        char current_char = *page_content;
        page_content++; // Advance page_content immediately after reading current_char

        if (current_char == '~') {
            char control_char = *page_content;
            if (control_char == '\0') {
                fprintf(stderr, "ERROR: Incomplete control code at end of page.\n");
                error_occurred = true;
                ret_val = -1;
                break;
            }
            page_content++; // Skip control_char

            switch (control_char) {
                case '#': OutputChar('#'); break;
                case '[': OutputChar('['); break;
                case ']': OutputChar(']'); break;
                case 'n': FlushOutput(); break;
                case 't': for (int i = 0; i < 4; i++) OutputChar(' '); break;
                case '~': OutputChar('~'); break;
                default:
                    fprintf(stderr, "ERROR: Invalid control code '%c'\n", control_char);
                    error_occurred = true;
                    ret_val = -1;
            }
        } else if (current_char == '[') {
            const char *command_start = page_content;
            const char *command_end = strchr(command_start, ']');

            if (command_end == NULL) {
                fprintf(stderr, "ERROR: Mismatched bracket in page content.\n");
                error_occurred = true;
                ret_val = -1;
                break;
            }

            char original_char_at_end = *command_end;
            *((char *)command_end) = '\0'; // Temporarily null-terminate to parse command

            if (strncmp(command_start, const_str_line, strlen(const_str_line)) == 0) {
                const char *line_cmd_param = command_start + strlen(const_str_line);
                if (*line_cmd_param == ':' && *(line_cmd_param + 2) == ':') {
                    char char_to_print = *(line_cmd_param + 1);
                    int count = atoi(line_cmd_param + 3);
                    for (int i = 0; i < count; i++) OutputChar(char_to_print);
                } else {
                    fprintf(stderr, "ERROR: Invalid line command format.\n");
                    error_occurred = true;
                    ret_val = -1;
                }
            } else if (strncmp(command_start, const_str_var, strlen(const_str_var)) == 0) {
                const char *var_name_value_start = command_start + strlen(const_str_var);
                if (*var_name_value_start == ':') {
                    // Create a temporary string in the format AddPageVar expects "[name:value]"
                    // +4 for `[` `]` `:` `\0`
                    char temp_var_str[MAX_PAGEVAR_NAME_LENGTH + MAX_PAGEVAR_NAME_LENGTH + 4]; 
                    snprintf(temp_var_str, sizeof(temp_var_str), "[%s]", var_name_value_start + 1);
                    AddPageVar(page_var_list_head, temp_var_str);
                } else {
                    fprintf(stderr, "ERROR: Invalid var command format.\n");
                    error_occurred = true;
                    ret_val = -1;
                }
            } else if (strncmp(command_start, const_str_box, strlen(const_str_box)) == 0) {
                if (in_a_box == 0) {
                    in_a_box = 1;
                    FlushOutput(); // Prints top border
                } else {
                    fprintf(stderr, "ERROR: Nested boxes are not supported.\n");
                    error_occurred = true;
                    ret_val = -1;
                }
            } else {
                fprintf(stderr, "ERROR: Unknown command in brackets: [%s]\n", command_start);
                error_occurred = true;
                ret_val = -1;
            }
            
            *((char *)command_end) = original_char_at_end; // Restore original char
            page_content = command_end + 1; // Move past ']'
        } else if (current_char == ']') {
            if (in_a_box != 0) {
                in_a_box = 0;
                FlushOutput(); // Prints bottom border
            } else {
                fprintf(stderr, "ERROR: Mismatched closing bracket ']' (not in a box).\n");
                error_occurred = true;
                ret_val = -1;
            }
        } else if (current_char == '#') {
            const char *var_name_start = page_content;
            const char *var_name_end = strchr(var_name_start, '#');

            if (var_name_end == NULL) {
                fprintf(stderr, "ERROR: Mismatched hash for variable lookup.\n");
                error_occurred = true;
                ret_val = -1;
                break;
            }

            char original_char_at_end = *var_name_end;
            *((char *)var_name_end) = '\0'; // Temporarily null-terminate

            PageVar *found_var = NULL;
            if (override_list != NULL) {
                found_var = GetPageVar(override_list, var_name_start, var_name_end);
            }
            if (found_var == NULL) {
                found_var = GetPageVar(page_var_list_head, var_name_start, var_name_end);
            }

            if (found_var != NULL && found_var->value != NULL) {
                OutputStr(found_var->value);
            }

            *((char *)var_name_end) = original_char_at_end; // Restore original char
            page_content = var_name_end + 1; // Move past second '#'
        } else {
            OutputChar(current_char);
        }
    }

    // Cleanup after loop
    if (line_length > 0) { // Only flush if there's content left in the line buffer
        FlushOutput();
    }
    DestroyVarList(page_var_list_head);
    DestroyVarList(override_list);
    return ret_val;
}

// Function: InteractWithPage
// Processes an input string to create an override list and then serves a page.
// context1: (const char* page_content) The page content to process.
// context2: (int remaining_depth) The remaining recursion depth.
// input_string: The string containing page overrides, e.g., "[var1:value1][var2:value2]".
void InteractWithPage(void *context1, void *context2, char *input_string) {
    PageVar *override_list_head = (PageVar *)calloc(1, sizeof(PageVar)); // Dummy head
    VerifyPointerOrTerminate(override_list_head, "Override_list initialization");

    char *current_pos = input_string;
    while (*current_pos != '\0' && *current_pos == '[') {
        char *end_bracket = strchr(current_pos, ']');
        if (end_bracket == NULL) {
            fprintf(stderr, "ERROR: Mismatched bracket in InteractWithPage input: %s\n", input_string);
            DestroyVarList(override_list_head);
            return;
        }

        char original_char = *end_bracket;
        *end_bracket = '\0'; // Temporarily null-terminate to pass to AddPageVar

        AddPageVar(override_list_head, current_pos); // current_pos points to '[name:value]'

        *end_bracket = original_char; // Restore the character

        current_pos = end_bracket + 1; // Move past ']' to the next potential '[' or end of string
    }

    // ServePageWithOverride will destroy override_list_head
    ServePageWithOverride((const char*)context1, (int)context2, override_list_head);
    return;
}

// Function: ServePage
// Serves a page without any external variable overrides.
void ServePage(const char *page_content, int remaining_depth) {
    ServePageWithOverride(page_content, remaining_depth, NULL);
}

// Function: main
// Entry point for the program, demonstrating usage of the page rendering functions.
int main() {
    // Example page content
    const char *page1 = "Hello, world!~nThis is a [box]boxed message[box].~n~tIndented text.~n"
                       "Variable test: #myVar# and #anotherVar#.~n";
    const char *page2 = "[var:myVar:Hello][var:anotherVar:World]~n"
                       "This is #myVar# #anotherVar#!~n"
                       "[line:x:5]~n";
    const char *page3 = "Example with override: #overrideMe#~n";

    printf("--- Serving Page 1 (basic content) ---\n");
    ServePage(page1, 1);
    printf("\n");

    printf("--- Serving Page 2 (with internal variable definitions) ---\n");
    ServePage(page2, 1);
    printf("\n");

    printf("--- Interacting with Page 3 (with external override) ---\n");
    char override_input[] = "[overrideMe:OverriddenValue]";
    InteractWithPage((void*)page3, (void*)1, override_input);
    printf("\n");

    printf("--- Interacting with Page 3 (with external override - two vars) ---\n");
    char override_input2[] = "[overrideMe:NewValue][anotherVar:FromOverride]";
    InteractWithPage((void*)page3, (void*)1, override_input2);
    printf("\n");

    return 0;
}