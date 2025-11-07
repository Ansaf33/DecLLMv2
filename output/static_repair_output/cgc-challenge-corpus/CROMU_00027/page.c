#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global variables (inferred from usage)
char line[0x51]; // Buffer for output lines, 81 bytes (index 0-80)
int line_length = 0; // Current length of content in 'line'
bool in_a_box = false; // Flag for box mode

// Placeholder for VerifyPointerOrTerminate
void VerifyPointerOrTerminate(void *ptr, const char *msg) {
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: %s failed. Terminating.\n", msg);
        exit(EXIT_FAILURE);
    }
}

// Inferred structure for PageVar (32-bit compilation assumption based on 0x48 bytes)
// 0x40 (64 bytes) for name + 4 bytes for value* + 4 bytes for next* = 72 bytes (0x48)
typedef struct PageVar {
    char name[64];           // Stores the name of the variable (null-terminated)
    char *value;             // Pointer to the dynamically allocated value string (null-terminated)
    struct PageVar *next;    // Pointer to the next PageVar in the list
} PageVar;

// Function: GetPageVar
// Searches for a PageVar by name within a specified range.
// list_head: The head of the PageVar linked list (can be a dummy head).
// name_start: Pointer to the beginning of the name in the input buffer.
// name_end: Pointer to the character immediately after the end of the name in the input buffer.
PageVar *GetPageVar(PageVar *list_head, const char *name_start, const char *name_end) {
    size_t name_len = name_end - name_start;
    PageVar *current = list_head;

    // Skip the dummy head if it exists and is empty
    if (current != NULL && current->name[0] == '\0' && current->value == NULL && current->next != NULL) {
        current = current->next;
    }

    while (current != NULL) {
        // Compare the name stored in the PageVar with the target name segment
        if (name_len == strlen(current->name) && strncmp(current->name, name_start, name_len) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: DestroyVarList
// Frees all memory associated with a PageVar linked list.
// list_head: The head of the PageVar linked list (can be a dummy head).
void DestroyVarList(PageVar *list_head) {
    if (list_head == NULL) {
        return;
    }

    // Recursively destroy the rest of the list
    if (list_head->next != NULL) {
        DestroyVarList(list_head->next);
        list_head->next = NULL;
    }

    // Free the value string if it exists
    if (list_head->value != NULL) {
        free(list_head->value);
        list_head->value = NULL;
    }

    // Free the PageVar structure itself
    free(list_head);
}

// Function: FlushOutput
// Prints the current line buffer and resets it.
void FlushOutput(void) {
    line[line_length] = '\0'; // Ensure line is null-terminated before printing
    printf("%s\n", line);
    memset(line, 0, sizeof(line));
    if (!in_a_box) {
        line_length = 0;
    } else {
        memset(line, ' ', 0x50); // Fill with spaces up to index 79
        line[0] = '*';           // Set first char to '*'
        line[0x4f] = '*';        // Set char at index 79 to '*' (0x50 - 1)
        line[0x50] = '\0';       // Null terminate the string
        line_length = 2;         // Indicates 2 characters (boundaries) are already set
    }
}

// Function: OutputChar
// Outputs a single character to the line buffer, flushing if necessary.
void OutputChar(char c) {
    if (line_length < (sizeof(line) - 1)) { // Check if there's space for char + null terminator
        line[line_length] = c;
        line_length++;
        line[line_length] = '\0'; // Always keep null-terminated

        // Conditions to NOT flush:
        // If line_length is not 0x50 AND (not in_a_box OR line_length is not 0x4e) then RETURN
        if (line_length != 0x50 && (!in_a_box || line_length != 0x4e)) {
            return;
        }
    }
    // If buffer full, or conditions met for flushing
    FlushOutput();
}

// Function: OutputStr
// Outputs a string to the line buffer, flushing if necessary.
void OutputStr(const char *str) {
    size_t str_len = strlen(str);

    if (str_len >= sizeof(line)) { // If the string itself is too long for the buffer
        if (line_length > 0) { // Flush existing content first
            FlushOutput();
        }
        printf("%s\n", str); // Print directly
        return;
    }

    if (line_length + str_len >= sizeof(line)) { // If string won't fit entirely
        FlushOutput();
    }

    // Now it should fit
    memcpy(&line[line_length], str, str_len);
    line_length += str_len;
    line[line_length] = '\0'; // Null terminate
}

// Function: AddPageVar
// Adds or updates a PageVar in the list.
// list_head: The head of the PageVar linked list (can be a dummy head).
// param_2: The input string containing "[TAG:NAME:VALUE]" format.
// Returns 0 on success, -1 on error.
int AddPageVar(PageVar *list_head, char *param_2) {
    // Expected format: [TAG:NAME:VALUE]
    // param_2 points to the '['
    if (param_2[0] != '[') return -1; // Must start with '['

    char *tag_colon = strchr(param_2, ':'); // Finds ':' after TAG
    if (!tag_colon) return -1;

    char *name_start = tag_colon + 1; // Points to start of NAME

    char *name_colon = strchr(name_start, ':'); // Finds ':' after NAME
    if (!name_colon) return -1;

    char *value_start = name_colon + 1; // Points to start of VALUE

    char *block_end = strchr(param_2, ']'); // Finds ']'
    if (!block_end) return -1;

    size_t name_len = name_colon - name_start;
    size_t value_len = block_end - value_start;

    if (name_len == 0 || name_len >= sizeof(((PageVar*)0)->name)) { // Name too long or empty
        return -1;
    }

    PageVar *page_var = GetPageVar(list_head, name_start, name_start + name_len);

    if (page_var == NULL) {
        // Find the last node (or the dummy head if list is empty)
        PageVar *current = list_head;
        while (current->next != NULL) {
            current = current->next;
        }
        page_var = (PageVar *)calloc(1, sizeof(PageVar));
        VerifyPointerOrTerminate(page_var, "New PageVar");
        current->next = page_var; // Append new node
    } else {
        // PageVar already exists, free its old value if any
        if (page_var->value != NULL) {
            free(page_var->value);
            page_var->value = NULL;
        }
    }

    // Copy name into the PageVar structure
    memcpy(page_var->name, name_start, name_len);
    page_var->name[name_len] = '\0'; // Null-terminate the name

    // Allocate and copy value
    page_var->value = (char *)calloc(value_len + 1, 1);
    VerifyPointerOrTerminate(page_var->value, "PageVar->value");
    memcpy(page_var->value, value_start, value_len);
    page_var->value[value_len] = '\0'; // Null-terminate the value

    return 0; // Success
}

// Function: ServePageWithOverride
// Processes page content from a buffer, interpreting tags and variables.
// page_content_ptr: Pointer to the current character in the page content buffer.
// page_content_len: Remaining length of the page content buffer.
// override_list_head: Head of a PageVar list for overriding variables.
// Returns 0 on success, -1 on error.
int ServePageWithOverride(char *page_content_ptr, int page_content_len, PageVar *override_list_head) {
    PageVar *page_var_list_head = (PageVar *)calloc(1, sizeof(PageVar)); // Dummy head for local page vars
    VerifyPointerOrTerminate(page_var_list_head, "VarList initialization");

    in_a_box = false;
    memset(line, 0, sizeof(line));
    line_length = 0;

    char *current_char_ptr = page_content_ptr;
    int current_remaining_len = page_content_len;
    bool error_occurred = false;

    while (current_remaining_len > 0 && !error_occurred) {
        char c = *current_char_ptr;
        current_char_ptr++;
        current_remaining_len--;

        if (c == '~') {
            if (current_remaining_len == 0) {
                fprintf(stderr, "ERROR: Incomplete control code at end of page.\n");
                error_occurred = true;
                break;
            }
            char control_char = *current_char_ptr;
            current_char_ptr++;
            current_remaining_len--;

            switch (control_char) {
                case '#': OutputChar('#'); break;
                case '[': OutputChar('['); break;
                case ']': OutputChar(']'); break;
                case 'n': FlushOutput(); break;
                case 't':
                    for (int i = 0; i < 4; i++) OutputChar(' ');
                    break;
                case '~': OutputChar('~'); break;
                default:
                    fprintf(stderr, "ERROR: Invalid control code ~%c\n", control_char);
                    error_occurred = true;
            }
        } else if (c == '[') {
            char *tag_start_in_buffer = current_char_ptr;
            int tag_content_len = 0;
            while (current_remaining_len > 0 && *current_char_ptr != ']' && *current_char_ptr != '\0') {
                current_char_ptr++;
                current_remaining_len--;
                tag_content_len++;
            }

            if (*current_char_ptr != ']') {
                fprintf(stderr, "ERROR: Missing closing ']' for tag.\n");
                error_occurred = true;
                break;
            }
            current_char_ptr++; // Consume ']'
            current_remaining_len--;

            // Temporarily null-terminate the tag content for string operations
            char temp_tag_content[tag_content_len + 1];
            memcpy(temp_tag_content, tag_start_in_buffer, tag_content_len);
            temp_tag_content[tag_content_len] = '\0';

            if (strncmp(temp_tag_content, "line:", strlen("line:")) == 0) {
                char *char_code_str = temp_tag_content + strlen("line:");
                char *num_str = strchr(char_code_str, ':');
                if (num_str && num_str > char_code_str) {
                    char char_to_repeat = *(num_str - 1);
                    int num_repeats = atoi(num_str + 1);
                    for (int i = 0; i < num_repeats; i++) {
                        OutputChar(char_to_repeat);
                    }
                } else {
                    fprintf(stderr, "ERROR: Malformed [line:C:N] tag.\n");
                    error_occurred = true;
                }
            } else if (strncmp(temp_tag_content, "var:", strlen("var:")) == 0) {
                // Construct the full "[var:name:value]" string for AddPageVar
                char full_var_string[tag_content_len + 3]; // + '[' + ']' + '\0'
                snprintf(full_var_string, sizeof(full_var_string), "[%s]", temp_tag_content);

                if (AddPageVar(page_var_list_head, full_var_string) == -1) {
                    fprintf(stderr, "ERROR: Failed to add page variable from [var:%s]\n", temp_tag_content + strlen("var:"));
                    error_occurred = true;
                }
            } else if (strcmp(temp_tag_content, "box") == 0) {
                in_a_box = true;
                FlushOutput();
                for (int i = 0; i < 0x50; i++) putc('*', stdout);
                printf("\n");
            } else {
                fprintf(stderr, "ERROR: Invalid tag [%s]\n", temp_tag_content);
                error_occurred = true;
            }
        } else if (c == ']') {
            if (!in_a_box) {
                fprintf(stderr, "ERROR: Closing ']' without opening 'box'.\n");
                error_occurred = true;
            } else {
                in_a_box = false;
                FlushOutput();
                for (int i = 0; i < 0x50; i++) putc('*', stdout);
                printf("\n");
            }
        } else if (c == '#') {
            char *var_name_start_in_buffer = current_char_ptr;
            int var_name_len = 0;
            while (current_remaining_len > 0 && *current_char_ptr != '#' && *current_char_ptr != '\0') {
                current_char_ptr++;
                current_remaining_len--;
                var_name_len++;
            }

            if (*current_char_ptr != '#') {
                fprintf(stderr, "ERROR: Missing closing '#' for variable name.\n");
                error_occurred = true;
                break;
            }
            current_char_ptr++; // Consume '#'
            current_remaining_len--;

            PageVar *found_var = NULL;

            // Search in override list first
            if (override_list_head != NULL) {
                found_var = GetPageVar(override_list_head, var_name_start_in_buffer, var_name_start_in_buffer + var_name_len);
            }

            // If not found in override, search in local page var list
            if (found_var == NULL) {
                found_var = GetPageVar(page_var_list_head, var_name_start_in_buffer, var_name_start_in_buffer + var_name_len);
            }

            if (found_var != NULL && found_var->value != NULL) {
                OutputStr(found_var->value);
            }
        } else {
            OutputChar(c);
        }
    } // End while loop

    if (line_length != 0) {
        FlushOutput();
    }

    DestroyVarList(page_var_list_head);
    // The override_list_head is managed by its caller (InteractWithPage or main)
    // The original code `DestroyVarList(param_3)` in ServePageWithOverride implies it *is* destroyed here.
    DestroyVarList(override_list_head);

    return error_occurred ? -1 : 0;
}

// Function: InteractWithPage
// Processes an input string containing override variable definitions and serves a page.
// page_content_buffer: The content of the page to serve.
// page_content_len: The length of the page content buffer.
// input_string: A string containing override variable definitions, e.g., "[tag:name:value][tag2:name2:value2]".
void InteractWithPage(char *page_content_buffer, int page_content_len, char *input_string) {
    PageVar *override_list_head = (PageVar *)calloc(1, sizeof(PageVar)); // Dummy head for the override list
    VerifyPointerOrTerminate(override_list_head, "Override_list initialization");

    char *current_pos = input_string;
    bool error_parsing_overrides = false;

    while (*current_pos != '\0' && !error_parsing_overrides) {
        if (*current_pos == '[') {
            char *block_start = current_pos;
            char *block_end = strchr(current_pos, ']');
            if (block_end == NULL) {
                fprintf(stderr, "ERROR: Missing closing ']' in override string: %s\n", block_start);
                error_parsing_overrides = true;
                break;
            }

            // Temporarily null-terminate the block for AddPageVar
            char temp_block_buffer[block_end - block_start + 1];
            memcpy(temp_block_buffer, block_start, block_end - block_start);
            temp_block_buffer[block_end - block_start] = '\0';

            if (AddPageVar(override_list_head, temp_block_buffer) == -1) {
                fprintf(stderr, "ERROR: Failed to add override variable from: %s\n", temp_block_buffer);
                error_parsing_overrides = true;
                break;
            }
            current_pos = block_end + 1; // Move past ']'
        } else {
            current_pos++; // Skip non-'[' characters
        }
    }

    if (!error_parsing_overrides) {
        ServePageWithOverride(page_content_buffer, page_content_len, override_list_head);
    } else {
        DestroyVarList(override_list_head); // Clean up if an error occurred before serving
    }
}

// Function: ServePage
// Serves a page without any override variables.
// page_content_buffer: The content of the page to serve.
// page_content_len: The length of the page content buffer.
void ServePage(char *page_content_buffer, int page_content_len) {
    ServePageWithOverride(page_content_buffer, page_content_len, NULL);
}

// Minimal main function for demonstration and compilation
int main() {
    // Example page content (as a string literal)
    char *page_content =
        "This is a sample page.\n"
        "~n"
        "[box]This is in a box.[var:myvar:Hello World!][var:another:123][line:x:5]~n"
        "Value of myvar: #myvar#~n"
        "Value of another: #another#~n"
        "~n"
        "This is outside the box. ~tIndented.~n"
        "[var:localvar:Local Value]"
        "Local var: #localvar#~n"
        "[box]Another box. #localvar# is still available.[var:myvar:Override Box Value]#myvar#[/box]~n"
        "After box, myvar is #myvar# (should be Hello World again).~n"
        "~[~]~#~#~n" // Escaped characters
        "End of page.\n";
    int page_len = strlen(page_content);

    printf("--- Serving Page (No Overrides) ---\n");
    ServePage(page_content, page_len);
    printf("\n");

    printf("--- Interacting With Page (Overrides) ---\n");
    char *override_string = "[override:myvar:Overridden Value][override:newvar:New Variable]";
    InteractWithPage(page_content, page_len, override_string);
    printf("\n");

    // Test error cases
    printf("--- Testing Error: Malformed Override String ---\n");
    char *malformed_override = "[override:badvar:value"; // Missing ']'
    InteractWithPage(page_content, page_len, malformed_override);
    printf("\n");

    printf("--- Testing Error: Malformed Page Content (Invalid Control Code) ---\n");
    char *bad_control_page = "Hello ~z World!\n";
    ServePage(bad_control_page, strlen(bad_control_page));
    printf("\n");

    printf("--- Testing Error: Malformed Page Content (Missing ']') ---\n");
    char *bad_tag_page = "Hello [box World!\n";
    ServePage(bad_tag_page, strlen(bad_tag_page));
    printf("\n");

    printf("--- Testing Error: Malformed Page Content (Missing '#') ---\n");
    char *bad_var_page = "Hello #myvar\n";
    ServePage(bad_var_page, strlen(bad_var_page));
    printf("\n");

    return 0;
}