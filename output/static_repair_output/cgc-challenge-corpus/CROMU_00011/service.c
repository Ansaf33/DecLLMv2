#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h> // For bool type

// --- Custom memory management functions ---
// allocate returns 0 on success, non-zero on failure.
int custom_allocate(size_t size, int flags, void **ptr) {
    *ptr = malloc(size);
    if (*ptr == NULL) {
        fprintf(stderr, "Memory allocation failed for size %zu\n", size);
        return 1; // Failure
    }
    return 0; // Success
}

void custom_deallocate(void *ptr, size_t size) {
    // Size argument is ignored for free, but kept for compatibility with original signature
    if (ptr != NULL) {
        free(ptr);
    }
}

// Replace bzero with memset
#define bzero(ptr, size) memset(ptr, 0, size)

// Replace _terminate with exit(1)
#define _terminate() exit(1)

// --- Struct definitions ---
#define MAX_SET_NAME_LEN 15
#define MAX_SET_ELEMENTS 15 // (0x58 - (sizeof(Set*)*2 + MAX_SET_NAME_LEN+1 + sizeof(int))) / sizeof(SetElement*) -> (88 - (4*2 + 16 + 4)) / 4 = (88 - 28) / 4 = 60 / 4 = 15 (assuming 32-bit pointers)

typedef struct SetElement {
    char data[MAX_SET_NAME_LEN + 1];
    int type; // 0 for set name, 1 for string literal
} SetElement;

typedef struct Set {
    struct Set *next;
    struct Set *prev;
    char name[MAX_SET_NAME_LEN + 1]; // Offset 0x8 (if 32-bit pointers)
    int num_elements;                // Offset 0x18
    SetElement *elements[MAX_SET_ELEMENTS]; // Offset 0x1C
} Set;

// Command line argument structure (deduced from parse_command_line)
typedef struct CmdLine {
    char *name;
    char op;
    char _padding[3]; // Pad to ensure 'operand' aligns correctly for 32-bit (4+1+3+4=12)
    char *operand;
} CmdLine; // Size 0xc (12 bytes) on a 32-bit system

// --- Global variables ---
Set *root = NULL; // Head of the circular doubly linked list of sets

// --- Forward declarations for internal functions ---
void print_set(Set *s);
SetElement *create_element(char *name, int type);
int element_in_set(Set *s, SetElement *el);
int add_element_to_set(Set *s, SetElement *el);
void free_element(SetElement *el);
void free_set_array(Set *s);
SetElement *copy_element(SetElement *el);
Set *copy_set(Set *s);
int receive_until(char *buffer, int max_len, char delimiter);
Set *retrieve_set(char *name);
Set *parse_operations(char *target_name, char *operation_str);
void add_set(Set *new_set);
CmdLine * parse_command_line(char *command_line);
Set * unionset(Set *set1, Set *set2);
Set * intersect(Set *set1, Set *set2);
Set * set_difference(Set *set1, Set *set2);
Set * symmetric_difference(Set *set1, Set *set2);
void exec_command(char *command_str);
int handle_set_var(CmdLine *cmd);
void issubset(char *left_set_str, char *right_set_str);
Set *parse_set(char *set_string); // Added missing forward declaration

// --- Stubs for internal functions ---
void print_set(Set *s) {
    if (!s) return;
    printf("Set: %s (elements: %d) {", s->name, s->num_elements);
    for (int i = 0; i < s->num_elements; ++i) {
        if (s->elements[i]) {
            if (s->elements[i]->type == 1) { // String literal
                printf("\"%s\"", s->elements[i]->data);
            } else { // Set name
                printf("%s", s->elements[i]->data);
            }
        }
        if (i < s->num_elements - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

SetElement *create_element(char *name, int type) {
    SetElement *new_el;
    if (custom_allocate(sizeof(SetElement), 0, (void **)&new_el) != 0) {
        return NULL;
    }
    strncpy(new_el->data, name, MAX_SET_NAME_LEN);
    new_el->data[MAX_SET_NAME_LEN] = '\0';
    new_el->type = type;
    return new_el;
}

int element_in_set(Set *s, SetElement *el) {
    if (!s || !el) return -1; // Error
    for (int i = 0; i < s->num_elements; ++i) {
        if (s->elements[i] && strcmp(s->elements[i]->data, el->data) == 0 && s->elements[i]->type == el->type) {
            return 1; // Found
        }
    }
    return 0; // Not found
}

int add_element_to_set(Set *s, SetElement *el) {
    if (!s || !el || s->num_elements >= MAX_SET_ELEMENTS) {
        fprintf(stderr, "Failed to add element to set (set or element NULL, or set full)\n");
        return 1; // Failure
    }
    s->elements[s->num_elements++] = el;
    return 0; // Success
}

void free_element(SetElement *el) {
    if (el) free(el);
}

void free_set_array(Set *s) {
    if (s) {
        for (int i = 0; i < s->num_elements; ++i) {
            free_element(s->elements[i]);
            s->elements[i] = NULL;
        }
        custom_deallocate(s, sizeof(Set)); // Use custom deallocate for consistency
    }
}

SetElement *copy_element(SetElement *el) {
    if (!el) return NULL;
    return create_element(el->data, el->type);
}

Set *copy_set(Set *s) {
    if (!s) return NULL;
    Set *new_set;
    if (custom_allocate(sizeof(Set), 0, (void **)&new_set) != 0) {
        return NULL;
    }
    bzero(new_set, sizeof(Set));
    strncpy(new_set->name, s->name, MAX_SET_NAME_LEN);
    new_set->name[MAX_SET_NAME_LEN] = '\0';
    new_set->num_elements = 0; // Will be populated by copying elements
    for (int i = 0; i < s->num_elements; ++i) {
        SetElement *copied_el = copy_element(s->elements[i]);
        if (copied_el) {
            if (add_element_to_set(new_set, copied_el) != 0) {
                free_set_array(new_set);
                return NULL;
            }
        } else {
            free_set_array(new_set);
            return NULL;
        }
    }
    return new_set;
}

// Dummy receive_until function - reads a line from stdin
int receive_until(char *buffer, int max_len, char delimiter) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1; // Error or EOF
    }
    // Remove trailing newline if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 0; // Success
}


// Function: parse_command_line
CmdLine * parse_command_line(char *command_line) {
    CmdLine *cmd = NULL;
    int i = 0;
    
    if (command_line == NULL || custom_allocate(sizeof(CmdLine), 0, (void **)&cmd) != 0) {
        return NULL; // Allocation failed or invalid input
    }

    bzero(cmd, sizeof(CmdLine)); // Initialize the struct

    bool error_flag = false;

    // Loop through the command_line string
    for (i = 0; command_line[i] != '\0' && i < 0x100; ++i) {
        char current_char = command_line[i];

        if (current_char == '=' || current_char == '@') {
            if (cmd->name == NULL) { // Error: Operator found before name
                error_flag = true;
                break;
            }
            cmd->op = current_char;
            command_line[i] = '\0'; // Null-terminate the first part (name)
        }
        else if (current_char == ' ') {
            if (cmd->operand == NULL && cmd->op == '\0') {
                // If operator hasn't been found and operand not started, this space is a separator for name
                command_line[i] = '\0';
            }
            // Otherwise, it's a space after name or before operand, can be ignored or part of operand parsing
        }
        else {
            if (isalnum((int)current_char) == 0) { // Not alphanumeric
                // If character is not one of the allowed special characters, OR name/op not set, it's an error.
                if ( (current_char != '|' && current_char != '-' && current_char != '\"' &&
                      current_char != ',' && current_char != '*' && current_char != '^' &&
                      current_char != '+' && current_char != '~') ||
                     (cmd->name == NULL) || (cmd->op == '\0') )
                {
                    error_flag = true;
                    break;
                }
                // If it's an allowed special char and name/op are set, continue to set operand
                if (cmd->operand == NULL && cmd->op != '\0') {
                    cmd->operand = &command_line[i];
                }
            }
            else if (cmd->name == NULL) { // Alphanumeric, name not set
                cmd->name = &command_line[i];
            }
            else if (cmd->operand == NULL && cmd->op != '\0') { // Alphanumeric, operand not set, op IS set
                cmd->operand = &command_line[i];
            }
        }
    }

    // Final check after loop: if name was not set, it's an error
    if (cmd->name == NULL) {
        error_flag = true;
    }

    if (error_flag) {
        printf("!!Error parsing commandline\n");
        custom_deallocate(cmd, sizeof(CmdLine));
        return NULL;
    }

    return cmd;
}

// Function: print_sets
void print_sets(void) {
    Set *current_set = root;
    if (current_set == NULL) { // Handle empty list
        return;
    }
    do {
        print_set(current_set);
        current_set = current_set->next;
    } while (current_set != root);
    return;
}

// Function: copymem
size_t copymem(char *dest, const char *src, size_t dest_offset, size_t count) {
    memcpy(dest + dest_offset, src, count);
    return dest_offset + count;
}

// Function: print_subsets
char * print_subsets(Set *s, int is_root_call) {
    char buffer[3201]; // 0xc80 + 1 for null terminator = 3201 bytes
    char *result_str = NULL;
    char *element_str = NULL;
    size_t current_len = 0;

    if (s == NULL) {
        return NULL;
    }

    bzero(buffer, sizeof(buffer));

    if (is_root_call != 0) {
        current_len = copymem(buffer, s->name, current_len, strlen(s->name));
        current_len = copymem(buffer, " = ", current_len, 3); // DAT_0001501d
    }

    buffer[current_len++] = '|'; // Add initial '|'

    for (int i = 0; i < s->num_elements; ++i) {
        SetElement *current_element = s->elements[i];
        if (current_element == NULL) {
            continue;
        }

        if (current_element->type == 1) { // String literal
            buffer[current_len++] = '\"';
            current_len = copymem(buffer, current_element->data, current_len, strlen(current_element->data));
            buffer[current_len++] = '\"';
        }
        else { // Set name (recursive call)
            Set *retrieved_set = retrieve_set(current_element->data);
            if (retrieved_set == NULL) {
                fprintf(stderr, "!!Error: Cannot find subset '%s'\n", current_element->data);
                return NULL; // Error, return NULL
            }
            element_str = print_subsets(retrieved_set, 0); // Recursive call, not a root call
            if (element_str == NULL) {
                return NULL; // Propagate error
            }
            current_len = copymem(buffer, element_str, current_len, strlen(element_str));
            custom_deallocate(element_str, strlen(element_str) + 1); // Free recursive result
            element_str = NULL;
        }
        buffer[current_len++] = ','; // Add comma after each element
    }

    // Adjust last character: replace last ',' with '|' or add '|'
    if (current_len > 0 && buffer[current_len - 1] == ',') {
        buffer[current_len - 1] = '|';
        buffer[current_len] = '\0';
    } else {
        // If no elements or last char was not comma, append '|' and null-terminate
        buffer[current_len++] = '|';
        buffer[current_len] = '\0';
    }

    // Allocate memory for the final string and copy
    size_t final_len = strlen(buffer);
    if (custom_allocate(final_len + 1, 0, (void **)&result_str) != 0) {
        return NULL; // Allocation failed
    }
    copymem(result_str, buffer, 0, final_len);
    result_str[final_len] = '\0';
    return result_str;
}

// Function: memcmp (standard library function, original implementation removed)
int std_memcmp(const void *s1, const void *s2, size_t n) {
    return memcmp(s1, s2, n);
}

// Function: exec_command
void exec_command(char *command_str) {
    if (command_str == NULL) {
        return;
    }

    if (strcmp(command_str, ".l") == 0) {
        _terminate(); // Exit the program
    }
    else if (strcmp(command_str, ".h") == 0) { // DAT_00015024
        printf("setx = |\"data\", sety|\tInitialize a set\n");
        printf("setx = seta+setb\tunion\n");
        printf("setx = seta^setb\tintersect\n");
        printf("setx = seta-setb\tset difference\n");
        printf("setx = seta~setb\tsymmetric difference\n");
        printf("seta @@ setb\t\tsubset. Returns TRUE or FALSE\n");
        printf(".h\t\tPrint this menu\n");
        printf(".l\t\tExit the program\n");
        printf(".p\t\tPrint sets and their elements\n");
        printf(".ps <setvar>\tPrint an expanded set\n");
    }
    else if (strncmp(command_str, ".ps", 3) == 0) { // DAT_00015170
        char *set_name_start = command_str + 3;
        // Skip leading spaces
        while (*set_name_start == ' ') {
            set_name_start++;
        }

        int len = 0;
        while (isalnum((int)set_name_start[len])) {
            len++;
        }
        char set_name[MAX_SET_NAME_LEN + 1];
        strncpy(set_name, set_name_start, len);
        set_name[len] = '\0';

        Set *target_set = retrieve_set(set_name);
        if (target_set == NULL) {
            printf("!!Failed to find set: %s\n", set_name);
        }
        else {
            char *subset_str = print_subsets(target_set, 1); // The '1' implies it's a root call for printing
            if (subset_str != NULL) {
                printf(".ps output: %s\n", subset_str); // DAT_0001518e
                custom_deallocate(subset_str, strlen(subset_str) + 1);
            }
        }
    }
    else if (strcmp(command_str, ".p") == 0) { // DAT_00015192
        print_sets();
    }
    else {
        printf("!!Unrecognized command: %s\n", command_str);
    }
    return;
}

// Function: retrieve_set
Set * retrieve_set(char *name) {
    if (name == NULL || root == NULL) {
        return NULL;
    }

    Set *current_set = root;
    do {
        if (strcmp(name, current_set->name) == 0) {
            return current_set;
        }
        current_set = current_set->next;
    } while (current_set != root);

    return NULL; // Not found
}

// Function: add_set
void add_set(Set *new_set) {
    if (new_set == NULL) {
        return;
    }

    if (root == NULL) {
        root = new_set;
        new_set->next = new_set;
        new_set->prev = new_set;
    } else {
        new_set->next = root;
        new_set->prev = root->prev;
        root->prev->next = new_set;
        root->prev = new_set;
    }
    return;
}

// Function: parse_set
Set * parse_set(char *set_string) {
    Set *new_set = NULL;
    char element_name_buffer[MAX_SET_NAME_LEN + 1];
    char *current_char_ptr = NULL;
    char *element_start = NULL;
    int state = 0; // 0=initial, 1=after '|', 2=success, 3=in quoted string, 4=after quoted string, 5=after element (expecting comma or '|'), 6=in unquoted element name, 7=after space in unquoted element name, 8=error
    bool error_occurred = false;

    if (set_string == NULL) {
        return NULL;
    }

    if (custom_allocate(sizeof(Set), 0, (void **)&new_set) != 0) {
        return NULL;
    }
    bzero(new_set, sizeof(Set));

    current_char_ptr = set_string;
    while (*current_char_ptr != '\0' && state != 8 && state != 2) {
        char c = *current_char_ptr;

        switch (state) {
            case 0: // Initial state, expecting '|'
                if (c == '|') {
                    state = 1;
                } else {
                    printf("!!Malformed set: Expected '|' at start\n");
                    error_occurred = true;
                }
                break;
            case 1: // After '|', expecting '\"' or alphanumeric for element
            case 5: // After an element (or after comma), expecting '\"' or alphanumeric for next element
            case 7: // After space in unquoted name, expecting '\"' or alphanumeric or '|'
                if (c == '\"') {
                    state = 3;
                    element_start = current_char_ptr + 1; // Start of string literal
                } else if (isalnum((int)c)) {
                    state = 6;
                    element_start = current_char_ptr; // Start of set name
                } else if (c == '|') { // End of set
                    state = 2; // Success
                } else if (c == ' ') {
                    // Skip spaces, state remains 1, 5 or 7
                } else {
                    printf("!!Malformed set: Unexpected character '%c' (state %d)\n", c, state);
                    error_occurred = true;
                }
                break;
            case 3: // Inside string literal (after '\"')
                if (c == '\"') {
                    size_t len = (size_t)(current_char_ptr - element_start);
                    if (len == 0 || len > MAX_SET_NAME_LEN) {
                        printf("!!Invalid set data length (quoted string)\n");
                        error_occurred = true;
                        break;
                    }
                    strncpy(element_name_buffer, element_start, len);
                    element_name_buffer[len] = '\0';

                    SetElement *new_element = create_element(element_name_buffer, 1); // Type 1 for literal
                    if (new_element == NULL) {
                        printf("!!Failed to create element (literal)\n");
                        error_occurred = true;
                        break;
                    }
                    if (element_in_set(new_set, new_element) == 1) {
                        printf("!!Elements must be unique\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    if (add_element_to_set(new_set, new_element) != 0) {
                        printf("Failed to add element (literal)\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    state = 4; // After string literal (expecting comma or '|')
                }
                // If not '\"', continue in state 3 (inside string)
                break;
            case 4: // After string literal, expecting ',' or '|'
                if (c == ',') {
                    state = 5; // Expecting next element
                } else if (c == '|') {
                    state = 2; // Success
                } else if (c == ' ') {
                    // Skip spaces, state remains 4
                } else {
                    printf("!!Malformed set: Expected ',' or '|' after string literal\n");
                    error_occurred = true;
                }
                break;
            case 6: // Inside unquoted element name (alphanumeric)
                if (c == ',') {
                    size_t len = (size_t)(current_char_ptr - element_start);
                    if (len == 0 || len > MAX_SET_NAME_LEN) {
                        printf("!!Invalid set name length\n");
                        error_occurred = true;
                        break;
                    }
                    strncpy(element_name_buffer, element_start, len);
                    element_name_buffer[len] = '\0';

                    SetElement *new_element = create_element(element_name_buffer, 0); // Type 0 for set name
                    if (new_element == NULL) {
                        printf("!!Failed to create element (name)\n");
                        error_occurred = true;
                        break;
                    }
                    if (element_in_set(new_set, new_element) == 1) {
                        printf("!!Elements must be unique\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    if (add_element_to_set(new_set, new_element) != 0) {
                        printf("!!Failed to add element (name)\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    state = 5; // After element, expecting comma or '|'
                } else if (c == '|') {
                    size_t len = (size_t)(current_char_ptr - element_start);
                    if (len == 0 || len > MAX_SET_NAME_LEN) {
                        printf("!!Invalid set name length\n");
                        error_occurred = true;
                        break;
                    }
                    strncpy(element_name_buffer, element_start, len);
                    element_name_buffer[len] = '\0';

                    Set *retrieved_set_ptr = retrieve_set(element_name_buffer);
                    if (retrieved_set_ptr == NULL) {
                        printf("!!Cannot have a non-existant set as an element: '%s'\n", element_name_buffer);
                        error_occurred = true;
                        break;
                    }
                    SetElement *new_element = create_element(element_name_buffer, 0); // Type 0 for set name
                    if (new_element == NULL) {
                        printf("!!Failed to create element (name)\n");
                        error_occurred = true;
                        break;
                    }
                    if (element_in_set(new_set, new_element) == 1) {
                        printf("!!Elements must be unique\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    if (add_element_to_set(new_set, new_element) != 0) {
                        printf("!!Failed to add element (name)\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    state = 2; // Success
                } else if (c == ' ') {
                    // Space indicates end of this element, transition to state 7
                    size_t len = (size_t)(current_char_ptr - element_start);
                    if (len == 0 || len > MAX_SET_NAME_LEN) {
                        printf("!!Invalid set name length\n");
                        error_occurred = true;
                        break;
                    }
                    strncpy(element_name_buffer, element_start, len);
                    element_name_buffer[len] = '\0';
                    SetElement *new_element = create_element(element_name_buffer, 0);
                    if (new_element == NULL) {
                        printf("!!Create element failed\n");
                        error_occurred = true;
                        break;
                    }
                    if (element_in_set(new_set, new_element) == 1) {
                        printf("!!Elements must be unique\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    if (add_element_to_set(new_set, new_element) != 0) {
                        printf("!!Failed to add element\n");
                        free_element(new_element);
                        error_occurred = true;
                        break;
                    }
                    state = 7; // After space
                } else if (!isalnum((int)c)) {
                    printf("!!Invalid character in set name '%c'\n", c);
                    error_occurred = true;
                }
                // If alphanumeric, continue in state 6
                break;
            default:
                // Should not happen, but for safety
                error_occurred = true;
                break;
        }

        if (error_occurred) { // A fatal error occurred
            break;
        }
        current_char_ptr++;
    }

    // Final checks after loop
    if (state != 2 || error_occurred) { // If parsing didn't end in success state or an error occurred
        free_set_array(new_set);
        return NULL;
    }

    return new_set;
}

// Function: symmetric_difference
Set * symmetric_difference(Set *set1, Set *set2) {
    Set *diff1 = NULL;
    Set *diff2 = NULL;
    Set *result_set = NULL;

    if (set1 == NULL || set2 == NULL) {
        return NULL;
    }

    diff1 = set_difference(set1, set2);
    if (diff1 == NULL) {
        return NULL;
    }

    diff2 = set_difference(set2, set1);
    if (diff2 == NULL) {
        free_set_array(diff1);
        return NULL;
    }

    result_set = unionset(diff1, diff2);

    free_set_array(diff1);
    free_set_array(diff2);

    return result_set;
}

// Function: set_difference
Set * set_difference(Set *set1, Set *set2) {
    Set *new_set = NULL;

    if (set1 == NULL || set2 == NULL) {
        return NULL;
    }

    if (custom_allocate(sizeof(Set), 0, (void **)&new_set) != 0) {
        return NULL;
    }
    bzero(new_set, sizeof(Set));

    for (int i = 0; i < set1->num_elements; ++i) {
        SetElement *current_element = set1->elements[i];
        int in_set2 = element_in_set(set2, current_element);
        if (in_set2 == -1) { // Error
            free_set_array(new_set);
            return NULL;
        }
        if (in_set2 == 0) { // Element not in set2, add to difference
            SetElement *copied_element = copy_element(current_element);
            if (copied_element == NULL) {
                free_set_array(new_set);
                return NULL;
            }
            if (add_element_to_set(new_set, copied_element) != 0) {
                free_element(copied_element);
                free_set_array(new_set);
                return NULL;
            }
        }
    }
    return new_set;
}

// Function: intersect
Set * intersect(Set *set1, Set *set2) {
    Set *new_set = NULL;

    if (set1 == NULL || set2 == NULL) {
        return NULL;
    }

    if (custom_allocate(sizeof(Set), 0, (void **)&new_set) != 0) {
        printf("!!Failed to allocate newset\n");
        return NULL;
    }
    bzero(new_set, sizeof(Set));

    for (int i = 0; i < set1->num_elements; ++i) {
        SetElement *current_element = set1->elements[i];
        int in_set2 = element_in_set(set2, current_element);
        if (in_set2 == -1) { // Error
            printf("!!Unknown error\n");
            free_set_array(new_set);
            return NULL;
        }
        if (in_set2 == 1) { // Element is in set2, add to intersection
            SetElement *copied_element = copy_element(current_element);
            if (copied_element == NULL) {
                free_set_array(new_set);
                return NULL;
            }
            if (add_element_to_set(new_set, copied_element) != 0) {
                printf("!!Error adding element in intersect\n");
                free_element(copied_element);
                free_set_array(new_set);
                return NULL;
            }
        }
    }
    return new_set;
}

// Function: unionset
Set * unionset(Set *set1, Set *set2) {
    Set *new_set = NULL;

    if (set1 == NULL || set2 == NULL) {
        return NULL;
    }

    if (custom_allocate(sizeof(Set), 0, (void **)&new_set) != 0) {
        printf("!!Failed to allocate new nset\n");
        return NULL;
    }
    bzero(new_set, sizeof(Set));

    // Add all elements from set1
    for (int i = 0; i < set1->num_elements; ++i) {
        SetElement *current_element = set1->elements[i];
        SetElement *copied_element = copy_element(current_element);
        if (copied_element == NULL) {
            free_set_array(new_set);
            return NULL;
        }
        if (add_element_to_set(new_set, copied_element) != 0) {
            printf("!!Failed to add element in union\n");
            free_element(copied_element);
            free_set_array(new_set);
            return NULL;
        }
    }

    // Add elements from set2 that are not already in new_set
    for (int i = 0; i < set2->num_elements; ++i) {
        SetElement *current_element = set2->elements[i];
        int in_new_set = element_in_set(new_set, current_element);
        if (in_new_set == -1) { // Error
            printf("!!!Error in intersect\n"); // Original message, seems out of place here
            free_set_array(new_set);
            return NULL;
        }
        if (in_new_set == 0) { // Element not in new_set, add it
            SetElement *copied_element = copy_element(current_element);
            if (copied_element == NULL) {
                free_set_array(new_set);
                return NULL;
            }
            if (add_element_to_set(new_set, copied_element) != 0) {
                printf("!!!Failed to add element in intersect\n"); // Original message
                free_element(copied_element);
                free_set_array(new_set);
                return NULL;
            }
        }
    }
    return new_set;
}

// Function: parse_operations
Set * parse_operations(char *target_set_name, char *operation_string) {
    Set *set1 = NULL;
    bool set1_was_newly_parsed = false;
    Set *set2 = NULL;
    bool set2_was_newly_parsed = false;
    char operator_char = 0;
    Set *result_set = NULL;
    char set_name_buffer[MAX_SET_NAME_LEN + 1];

    if (target_set_name == NULL || operation_string == NULL) {
        return NULL;
    }

    char *current_op_ptr = operation_string;
    bool parsing_error = false;

    while (*current_op_ptr != '\0' && !parsing_error) {
        char c = *current_op_ptr;

        if (c == '|') {
            if (set1 == NULL) {
                set1 = parse_set(current_op_ptr);
                if (set1 == NULL) {
                    printf("!!Set parsing failed\n");
                    parsing_error = true;
                    break;
                }
                set1_was_newly_parsed = true;
                // Advance current_op_ptr past the parsed set
                while (*current_op_ptr != '\0' && *current_op_ptr != '|') {
                    current_op_ptr++;
                }
                if (*current_op_ptr == '|') current_op_ptr++; // Move past the closing '|'
            } else { // set1 is already parsed/retrieved
                if (set2 != NULL) {
                    printf("!!Too many sets\n");
                    parsing_error = true;
                    break;
                }
                set2 = parse_set(current_op_ptr);
                if (set2 == NULL) {
                    printf("!!Set parsing failed\n");
                    parsing_error = true;
                    break;
                }
                set2_was_newly_parsed = true;
                // Advance current_op_ptr past the parsed set
                while (*current_op_ptr != '\0' && *current_op_ptr != '|') {
                    current_op_ptr++;
                }
                if (*current_op_ptr == '|') current_op_ptr++; // Move past the closing '|'
            }
            continue; // Continue loop after parsing a set
        }
        else if (c == '^' || c == '+' || c == '-' || c == '~') {
            if (operator_char == 0) {
                operator_char = c;
            } else {
                printf("!!Only one operation allowed.\n");
                parsing_error = true;
                break;
            }
        }
        else if (c == ' ') {
            // Skip spaces
        }
        else if (isalnum((int)c)) {
            char *name_start = current_op_ptr;
            while (isalnum((int)*current_op_ptr)) {
                current_op_ptr++;
            }
            size_t name_len = (size_t)(current_op_ptr - name_start);
            current_op_ptr--; // Adjust pointer to last char of name, loop will increment it

            if (name_len == 0 || name_len > MAX_SET_NAME_LEN) {
                printf("!!Invalid set name length\n");
                parsing_error = true;
                break;
            }
            strncpy(set_name_buffer, name_start, name_len);
            set_name_buffer[name_len] = '\0';

            if (set1 == NULL) {
                set1 = retrieve_set(set_name_buffer);
                if (set1 == NULL) {
                    printf("!!Nonexistant set: %s\n", set_name_buffer);
                    parsing_error = true;
                    break;
                }
            } else if (set2 == NULL) {
                set2 = retrieve_set(set_name_buffer);
                if (set2 == NULL) {
                    printf("!!Nonexistant set: %s\n", set_name_buffer);
                    parsing_error = true;
                    break;
                }
            } else {
                printf("!!Too many sets\n");
                parsing_error = true;
                break;
            }
        }
        else { // Invalid character
            printf("!!Invalid sets\n");
            parsing_error = true;
            break;
        }
        current_op_ptr++;
    }

    // --- Operation execution ---
    if (!parsing_error) {
        if (operator_char == '+') {
            result_set = unionset(set1, set2);
        }
        else if (operator_char == '^') {
            result_set = intersect(set1, set2);
        }
        else if (operator_char == '-') {
            result_set = set_difference(set1, set2);
        }
        else if (operator_char == '~') {
            result_set = symmetric_difference(set1, set2);
        }
        else { // No operator, just a single set assignment (copy)
            result_set = copy_set(set1);
            if (result_set == NULL) {
                printf("!!Copy failed\n");
                parsing_error = true;
            }
        }
    }

    // --- Post-operation checks and cleanup ---
    if (!parsing_error && result_set != NULL) {
        strncpy(result_set->name, target_set_name, MAX_SET_NAME_LEN);
        result_set->name[MAX_SET_NAME_LEN] = '\0';

        // Check if set contains itself
        for (int i = 0; i < result_set->num_elements; ++i) {
            SetElement *el = result_set->elements[i];
            if (el != NULL && el->type == 0 && strcmp(el->data, result_set->name) == 0) {
                printf("!!A set cannot contain itself\n");
                free_set_array(result_set);
                result_set = NULL;
                parsing_error = true; // Mark as error
                break;
            }
        }
    }

    // Cleanup temporary sets if they were newly parsed
    if (set1 != NULL && set1_was_newly_parsed) {
        free_set_array(set1);
    }
    if (set2 != NULL && set2_was_newly_parsed) {
        free_set_array(set2);
    }

    return result_set;
}

// Function: handle_set_var
int handle_set_var(CmdLine *cmd) {
    if (cmd == NULL || cmd->name == NULL || cmd->operand == NULL) {
        return -1; // Error
    }

    Set *newly_created_set = parse_operations(cmd->name, cmd->operand);
    if (newly_created_set == NULL) {
        printf("!!Error parsing sets\n");
        return -1; // Error
    }

    Set *existing_set = retrieve_set(newly_created_set->name);
    if (existing_set == NULL) {
        add_set(newly_created_set);
    } else {
        // Update existing set with new elements
        for (int i = 0; i < existing_set->num_elements; ++i) {
            free_element(existing_set->elements[i]);
            existing_set->elements[i] = NULL;
        }
        existing_set->num_elements = 0;

        for (int i = 0; i < newly_created_set->num_elements; ++i) {
            existing_set->elements[i] = newly_created_set->elements[i]; // Transfer ownership
            newly_created_set->elements[i] = NULL; // Clear source pointer
        }
        existing_set->num_elements = newly_created_set->num_elements;
        custom_deallocate(newly_created_set, sizeof(Set)); // Free the wrapper set, not its elements
    }
    return 0; // Success
}

// Function: issubset
void issubset(char *left_set_str, char *right_set_str) {
    Set *left_set = NULL;
    Set *right_set = NULL;
    bool left_is_temp_set = false;
    bool right_is_temp_set = false;
    bool result_printed = false;

    if (left_set_str == NULL || right_set_str == NULL) {
        return;
    }

    // Parse left set
    while (*left_set_str != '\0' && *left_set_str == ' ') {
        left_set_str++;
    }
    if (*left_set_str == '|') {
        left_set = parse_set(left_set_str);
        left_is_temp_set = true;
    } else {
        int len = 0;
        while (left_set_str[len] != ' ' && left_set_str[len] != '\0') {
            len++;
        }
        char temp_name[MAX_SET_NAME_LEN + 1];
        strncpy(temp_name, left_set_str, len);
        temp_name[len] = '\0'; // Null terminate the name
        left_set = retrieve_set(temp_name);
    }

    if (left_set == NULL) {
        printf("!!Failed to parse left set\n");
        goto cleanup_issubset;
    }

    // Parse right set
    while (*right_set_str != '\0' && *right_set_str == ' ') {
        right_set_str++;
    }
    if (*right_set_str == '|') {
        right_set = parse_set(right_set_str);
        right_is_temp_set = true;
    } else {
        int len = 0;
        while (right_set_str[len] != ' ' && right_set_str[len] != '\0') {
            len++;
        }
        char temp_name[MAX_SET_NAME_LEN + 1];
        strncpy(temp_name, right_set_str, len);
        temp_name[len] = '\0'; // Null terminate the name
        right_set = retrieve_set(temp_name);
    }

    if (right_set == NULL) {
        printf("!!Failed to parse right set\n");
        goto cleanup_issubset;
    }

    // Check for subset
    for (int i = 0; i < left_set->num_elements; ++i) {
        if (element_in_set(right_set, left_set->elements[i]) == 0) {
            printf("FALSE\n");
            result_printed = true;
            break;
        }
    }

    if (!result_printed) {
        printf("TRUE\n");
    }

cleanup_issubset:
    if (left_set != NULL && left_is_temp_set) {
        free_set_array(left_set);
    }
    if (right_set != NULL && right_is_temp_set) {
        free_set_array(right_set);
    }
    return;
}

// Function: command_loop
void command_loop(void) {
    char input_buffer[256];
    CmdLine *cmd_args = NULL;
    int read_status;

    while (true) {
        bzero(input_buffer, sizeof(input_buffer));
        printf(">> "); // DAT_000154db

        read_status = receive_until(input_buffer, sizeof(input_buffer), '\n'); // Read until newline
        if (read_status == -1) { // EOF or error
            break;
        }

        if (input_buffer[0] == '.') {
            exec_command(input_buffer);
        } else {
            cmd_args = parse_command_line(input_buffer);
            if (cmd_args == NULL) {
                _terminate(); // Error parsing, exit
            }

            if (cmd_args->op == '=') {
                handle_set_var(cmd_args);
            } else if (cmd_args->op == '@') {
                issubset(cmd_args->name, cmd_args->operand);
            }
            custom_deallocate(cmd_args, sizeof(CmdLine));
            cmd_args = NULL; // Ensure pointer is null after freeing
        }
    }
}

// Function: main
int main(void) {
    command_loop();
    return 0;
}