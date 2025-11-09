#include <stdio.h>   // For printf, fprintf, putc, NULL, FILE
#include <stdlib.h>  // For calloc, free, exit, EXIT_FAILURE, atoi
#include <string.h>  // For strchr, strlen, strncmp, memcpy, memset

// Global variables
char line[81]; // Buffer for output line. Max 78 chars for content + 2 stars + null terminator = 81, or 80 for normal.
int line_length = 0; // Current actual content length in 'line' buffer.
int in_a_box = 0;    // Boolean flag: 0 for false, non-zero for true.

// Forward declarations
typedef struct PageVar PageVar;

// Helper function to verify memory allocation
void VerifyPointerOrTerminate(void *ptr, const char *msg) {
    if (ptr == NULL) {
        fprintf(stderr, "Fatal Error: %s failed to allocate memory.\n", msg);
        exit(EXIT_FAILURE);
    }
}

// Structure definition for PageVar
struct PageVar {
    char name[64]; // Variable name. Max length 64 bytes.
    char *value;   // Pointer to dynamically allocated value string.
    PageVar *next; // Pointer to the next PageVar in the list.
}; // Total size 64 + sizeof(char*) + sizeof(PageVar*). Assumed 72 bytes (0x48) from original decompiler output.

// Function: GetPageVar
// list_dummy_head: A dummy PageVar node whose 'next' points to the first actual variable.
// var_name_start: Pointer to the beginning of the variable name in the input string.
// var_name_end: Pointer to the end of the variable name in the input string.
PageVar *GetPageVar(PageVar *list_dummy_head, const char *var_name_start, const char *var_name_end) {
    size_t name_len = var_name_end - var_name_start;

    PageVar *current = list_dummy_head->next; // Start searching from the first actual variable
    while (current != NULL) {
        // Compare with the variable name stored in the struct
        if (strncmp(current->name, var_name_start, name_len) == 0 && current->name[name_len] == '\0') {
            return current; // Return pointer to the found PageVar struct
        }
        current = current->next;
    }
    return NULL;
}

// Function: DestroyVarList
// list_dummy_head: The dummy PageVar node that initiated the list.
void DestroyVarList(PageVar *list_dummy_head) {
    if (list_dummy_head == NULL) return;

    PageVar *current = list_dummy_head->next;
    PageVar *next_node;
    while (current != NULL) {
        next_node = current->next;
        if (current->value != NULL) {
            free(current->value);
        }
        free(current);
        current = next_node;
    }
    free(list_dummy_head); // Free the dummy head itself
}

// Function: AddPageVar
// list_dummy_head: A dummy PageVar node for the list.
// var_string_segment: A string segment like "type:name:value]"
int AddPageVar(PageVar *list_dummy_head, char *var_string_segment) {
    char *first_colon = strchr(var_string_segment, ':');
    if (first_colon == NULL) return 0; // Malformed string

    char *name_start = first_colon + 1;
    char *second_colon = strchr(name_start, ':');
    if (second_colon == NULL) return 0; // Malformed string

    char *value_start = second_colon + 1;
    char *closing_bracket = strchr(var_string_segment, ']');
    if (closing_bracket == NULL) return 0; // Malformed string

    size_t name_len = second_colon - name_start;
    size_t value_len = closing_bracket - value_start;

    if (name_len >= sizeof(list_dummy_head->name)) { // Max name length 64 bytes
        return 0; // Name too long
    }

    PageVar *current_var = GetPageVar(list_dummy_head, name_start, second_colon);

    if (current_var == NULL) {
        // Variable not found, create a new one and add to list
        PageVar *new_var = calloc(1, sizeof(PageVar));
        VerifyPointerOrTerminate(new_var, "New PageVar");

        // Find the end of the list (starting from dummy head) to append
        PageVar *tail = list_dummy_head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = new_var;
        current_var = new_var; // Use the newly created variable
    } else {
        // Variable found, free existing value if any
        if (current_var->value != NULL) {
            free(current_var->value);
            current_var->value = NULL;
        }
    }

    // Copy name
    strncpy(current_var->name, name_start, name_len);
    current_var->name[name_len] = '\0'; // Null-terminate the name

    // Allocate and copy value
    current_var->value = calloc(value_len + 1, 1);
    VerifyPointerOrTerminate(current_var->value, "PageVar->value");
    strncpy(current_var->value, value_start, value_len);
    current_var->value[value_len] = '\0'; // Null-terminate the value

    return 1; // Success
}

// Function: FlushOutput
void FlushOutput(void) {
    if (line_length > 0 || in_a_box != 0) { // Always flush if in a box, even if empty line, to print border
        if (in_a_box == 0) {
            line[line_length] = '\0'; // Null-terminate for printf
            printf("%s\n", line);
        } else {
            char buffer[81];
            memset(buffer, ' ', 80); // Fill with spaces for 80 characters
            memcpy(buffer + 1, line, line_length); // Copy content from line[0] to buffer[1]
            buffer[0] = '*';
            buffer[79] = '*';
            buffer[80] = '\0';
            printf("%s\n", buffer);
        }
    }
    memset(line, 0, sizeof(line)); // Clear the buffer
    line_length = 0; // Reset content length
}

// Function: OutputChar
void OutputChar(char c) {
    int max_content_len = (in_a_box == 0) ? 80 : 78; // 80 for normal, 78 for box content

    if (line_length < max_content_len) {
        line[line_length] = c;
        line_length++;
    }

    if (line_length == max_content_len) {
        FlushOutput();
    }
}

// Function: OutputStr
void OutputStr(const char *str) {
    size_t len = strlen(str);
    int max_content_len = (in_a_box == 0) ? 80 : 78;

    // If the string is too long for the current line buffer, flush and print directly
    if (len + line_length > max_content_len) {
        FlushOutput();
        // If it's still too long for a single line, print directly
        if (len > max_content_len) {
            printf("%s\n", str);
            return;
        }
    }

    // Copy to line buffer
    memcpy(line + line_length, str, len);
    line_length += len;

    // Flush if buffer is full
    if (line_length == max_content_len) {
        FlushOutput();
    }
}

// Function: ServePageWithOverride
// page_content: The actual page template string.
// content_length: Max chars to process (0 to process until null terminator).
// override_list_dummy_head: Dummy head of a linked list of PageVar structs for overrides.
void ServePageWithOverride(const char *page_content, int content_length, PageVar *override_list_dummy_head) {
    PageVar *var_list_dummy_head = calloc(1, sizeof(PageVar)); // Dummy head for general page variables
    VerifyPointerOrTerminate(var_list_dummy_head, "VarList initialization");

    in_a_box = 0;
    memset(line, 0, sizeof(line));
    line_length = 0;

    const char *current_char_ptr = page_content;
    int chars_processed = 0;

    while (*current_char_ptr != '\0' && (content_length == 0 || chars_processed < content_length)) {
        char current_char = *current_char_ptr;

        if (current_char == '~') {
            current_char_ptr++;
            chars_processed++;
            if (*current_char_ptr == '\0') {
                fprintf(stderr, "ERROR: Incomplete escape sequence.\n");
                break;
            }
            char escaped_char = *current_char_ptr;
            current_char_ptr++;
            chars_processed++;

            switch (escaped_char) {
                case '#': OutputChar('#'); break;
                case '[': OutputChar('['); break;
                case ']': OutputChar(']'); break;
                case 'n': FlushOutput(); break;
                case 't':
                    for (int i = 0; i < 4; i++) OutputChar(' ');
                    break;
                case '~': OutputChar('~'); break;
                default:
                    fprintf(stderr, "ERROR: Invalid control code ~%c\n", escaped_char);
                    break;
            }
        } else if (current_char == '[') {
            const char *command_start = current_char_ptr + 1; // After '['
            const char *command_end = strchr(command_start, ']');

            if (command_end == NULL) {
                fprintf(stderr, "ERROR: Invalid syntax, missing ']'.\n");
                break;
            }

            size_t command_name_len = command_end - command_start; // Length of content inside []

            if (command_name_len >= 4 && strncmp(command_start, "line", 4) == 0) { // "[line:C:N]"
                const char *first_colon = strchr(command_start, ':');
                if (first_colon == NULL || first_colon >= command_end) {
                    fprintf(stderr, "ERROR: Malformed [line:C:N] command.\n");
                    break;
                }
                char fill_char = first_colon[1];
                const char *second_colon = strchr(first_colon + 1, ':');
                if (second_colon == NULL || second_colon >= command_end) {
                    fprintf(stderr, "ERROR: Malformed [line:C:N] command.\n");
                    break;
                }
                int count = atoi(second_colon + 1);
                for (int i = 0; i < count; i++) {
                    OutputChar(fill_char);
                }
            } else if (command_name_len >= 3 && strncmp(command_start, "var", 3) == 0) { // "[var:type:name:value]"
                // AddPageVar expects string starting from "type:name:value"
                if (!AddPageVar(var_list_dummy_head, (char *)command_start + 4)) { // Skip "var:"
                    fprintf(stderr, "Warning: Failed to add page variable from command: [%.*s]\n", (int)command_name_len, command_start);
                }
            } else if (command_name_len == 3 && strncmp(command_start, "box", 3) == 0) { // "[box]"
                in_a_box = 1;
                FlushOutput(); // Flush any current line content before drawing box
                // Print top border for the box
                for (int i = 0; i < 80; i++) {
                    putc('*', stdout);
                }
                printf("\n");
            } else {
                fprintf(stderr, "ERROR: Unrecognized command: [%.*s]\n", (int)command_name_len, command_start);
            }
            current_char_ptr = command_end + 1; // Move past ']'
            chars_processed = (int)(current_char_ptr - page_content); // Update total processed chars

        } else if (current_char == ']') {
            if (in_a_box != 0) {
                in_a_box = 0;
                FlushOutput(); // Flush content within the box
                // Print bottom border for the box
                for (int i = 0; i < 80; i++) {
                    putc('*', stdout);
                }
                printf("\n");
            } else {
                fprintf(stderr, "ERROR: Mismatched ']' without preceding '[box]'.\n");
                break;
            }
            current_char_ptr++;
            chars_processed++;
        } else if (current_char == '#') {
            const char *var_name_start = current_char_ptr + 1; // After '#'
            const char *var_name_end = strchr(var_name_start, '#');

            if (var_name_end == NULL) {
                fprintf(stderr, "ERROR: Invalid syntax, missing closing '#'.\n");
                break;
            }

            PageVar *found_var = NULL;
            if (override_list_dummy_head != NULL) {
                found_var = GetPageVar(override_list_dummy_head, var_name_start, var_name_end);
            }
            if (found_var == NULL) {
                found_var = GetPageVar(var_list_dummy_head, var_name_start, var_name_end);
            }

            if (found_var != NULL && found_var->value != NULL) {
                OutputStr(found_var->value);
            }
            current_char_ptr = var_name_end + 1; // Move past closing '#'
            chars_processed = (int)(current_char_ptr - page_content); // Update total processed chars
        } else {
            OutputChar(current_char);
            current_char_ptr++;
            chars_processed++;
        }
    }

    if (line_length != 0) {
        FlushOutput();
    }
    
    // ServePageWithOverride takes ownership of the lists and frees them.
    DestroyVarList(var_list_dummy_head);
    if (override_list_dummy_head != NULL) {
        DestroyVarList(override_list_dummy_head);
    }
}

// Function: ServePage
// page_content: The actual page template string.
// content_length: Max chars to process (0 to process until null terminator).
void ServePage(const char *page_content, int content_length) {
    ServePageWithOverride(page_content, content_length, NULL); // No overrides
}

// Function: InteractWithPage
// page_content: The actual page template string.
// content_length: Max chars to process (0 to process until null terminator).
// override_definitions_string: A string containing override definitions like "[type:name:value][type2:name2:value2]"
void InteractWithPage(const char *page_content, int content_length, char *override_definitions_string) {
    PageVar *override_list_dummy_head = calloc(1, sizeof(PageVar)); // Dummy head for the list
    VerifyPointerOrTerminate(override_list_dummy_head, "Override_list initialization");

    char *current_override_pos = override_definitions_string;

    // Loop through override definitions like "[type:name:value]"
    while (*current_override_pos != '\0' && *current_override_pos == '[') {
        current_override_pos++; // Skip '['
        // AddPageVar expects string starting from "type:name:value"
        if (!AddPageVar(override_list_dummy_head, current_override_pos)) {
            fprintf(stderr, "Warning: Failed to add page variable from: %s\n", current_override_pos);
        }
        current_override_pos = strchr(current_override_pos, ']');
        if (current_override_pos == NULL) {
            fprintf(stderr, "Error: Malformed override string, missing ']'.\n");
            break;
        }
        current_override_pos++; // Skip ']'
    }
    ServePageWithOverride(page_content, content_length, override_list_dummy_head);
    // ServePageWithOverride takes ownership of override_list_dummy_head and frees it.
}

// Main function for demonstration and compilation
int main() {
    printf("--- ServePage Example ---\n");
    const char *simple_page = "Hello, world!~nThis is a simple page.~tTabbed line.~n"
                              "[box]This is in a box.[var:type:myvar:123]This is a var: #myvar#.[line:X:5] [/box]~n"
                              "End of page. Another var: #another#";
    ServePage(simple_page, 0); // 0 for content_length means process until null terminator

    printf("\n--- InteractWithPage Example (Overrides) ---\n");
    const char *page_with_vars = "My name is #name#. My city is #city#. Default var: #defaultvar#. "
                                 "This is #anothervar#.";
    char overrides[] = "[type:name:Alice][type:city:Wonderland][type:anothervar:OverrideValue]";

    InteractWithPage(page_with_vars, 0, overrides);

    printf("\n--- ServePage with no vars ---\n");
    const char *no_vars_page = "This page has no variables.";
    ServePage(no_vars_page, 0);

    return 0;
}