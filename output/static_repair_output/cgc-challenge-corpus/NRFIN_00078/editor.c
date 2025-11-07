#include <stdio.h>    // For sprintf
#include <stdlib.h>   // For calloc, free, exit
#include <string.h>   // For strlen, strcpy, strcat, strtok, memcpy
#include <stdint.h>   // For uint32_t
#include <stdbool.h>  // For bool type

// Define the core types used in the original snippet
// undefined4 is likely a 32-bit unsigned integer, used for pointers and integer values.
// undefined is likely a char.
// uint is unsigned int.
typedef uint32_t uint_fast; // Using uint_fast for generic unsigned int where specific width isn't critical
typedef uint32_t undefined4; // Represents a 4-byte unsigned integer or pointer
typedef char undefined;      // Represents a single byte, typically for string data

// --- Struct Definitions (inferred from usage patterns) ---

// Doubly linked list node structure
typedef struct ListNode {
    struct ListNode *next; // Offset 0
    struct ListNode *prev; // Offset 4
    char data[];           // Offset 8 (flexible array member for string data)
} ListNode;

// Command structure (inferred from param_1 + offset accesses)
// `param_1` in `run_command` is `undefined*`, so `Command*` is appropriate.
typedef struct Command {
    char command_char;      // At offset 0
    uint_fast line_start_address; // At offset 1 (used as index/count for get_line_by_address)
    uint_fast line_end_address;   // At offset 5 (similarly an index/count)
    uint_fast data_length;        // At offset 9 (length of the text data)
    char text_data[];             // At offset 0xd (flexible array member for the text itself)
} Command;

// Result structure for commands that return data (inferred from param_2 usage)
typedef struct ResultLine {
    uint_fast length;    // At offset 4 (length of the data in bytes)
    char data[];        // At offset 0xc (flexible array member for the result string/data)
} ResultLine;

// --- Global Variables (inferred from DAT_xxxx addresses) ---

ListNode *state = NULL; // Head of the linked list
ListNode *marks[28] = {NULL}; // Corresponds to DAT_00017008, an array of 28 line marks
ListNode *current_line = NULL; // Corresponds to DAT_00017004, the "current" line
ListNode *last_current_line = NULL; // Corresponds to DAT_00017074
ListNode *last_modified_line = NULL; // Corresponds to DAT_00017070

// Type for transform functions: takes a char, returns a char
typedef char (*TransformFunc)(char);
// Corresponds to DAT_0001700c, an array of 56 transform function pointers
TransformFunc transform_funcs[56] = {NULL};

int transform_mode = 0; // Corresponds to DAT_00017078, a flag for transform behavior

// --- Dummy External/Helper Functions (not provided in snippet) ---

// Dummy linked list functions (doubly linked list assumed based on usage)
size_t list_length(ListNode **head) {
    size_t count = 0;
    ListNode *current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

void list_insert_after(ListNode **head, ListNode *prev_node, ListNode *new_node) {
    if (new_node == NULL) return;

    if (prev_node == NULL) { // Insert at head
        new_node->next = *head;
        new_node->prev = NULL;
        if (*head != NULL) {
            (*head)->prev = new_node;
        }
        *head = new_node;
    } else { // Insert after prev_node
        new_node->next = prev_node->next;
        new_node->prev = prev_node;
        if (prev_node->next != NULL) {
            prev_node->next->prev = new_node;
        }
        prev_node->next = new_node;
    }
}

void list_insert_before(ListNode **head, ListNode *next_node, ListNode *new_node) {
    if (new_node == NULL) return;

    if (next_node == NULL || next_node == *head) { // Insert at head (or if next_node is null)
        list_insert_after(head, NULL, new_node);
    } else { // Insert before next_node (which means after next_node->prev)
        list_insert_after(head, next_node->prev, new_node);
    }
}

void list_remove(ListNode **head, ListNode *node_to_remove) {
    if (*head == NULL || node_to_remove == NULL) return;

    if (node_to_remove->prev != NULL) {
        node_to_remove->prev->next = node_to_remove->next;
    } else { // node_to_remove is the head
        *head = node_to_remove->next;
    }

    if (node_to_remove->next != NULL) {
        node_to_remove->next->prev = node_to_remove->prev;
    }
    // Note: The caller is responsible for freeing node_to_remove
}

// Dummy regex_match function
// Returns 0 on match, -1 on no match, 1 on error (example behavior)
int regex_match(const char *pattern, const char *text, void *match_info, void *sub_match_info) {
    // Basic dummy implementation: check if pattern is a substring of text
    (void)match_info; // Unused parameter
    (void)sub_match_info; // Unused parameter
    if (strstr(text, pattern) != NULL) {
        return 0; // Match
    }
    return -1; // No match
}

// Dummy _terminate function
void _terminate() {
    exit(0);
}

// --- Function Implementations (fixed and reduced) ---

// Function: get_line_by_address
// param_1 is an integer line number or a negative mark index.
ListNode *get_line_by_address(int line_address) {
    if (line_address < 0) {
        // Access marks array: -1 maps to marks[0], -2 to marks[1], etc.
        uint_fast mark_index = (uint_fast)(-line_address - 1);
        if (mark_index < 28) { // Check bounds for marks array
            return marks[mark_index];
        }
        return NULL; // Out of bounds mark
    } else {
        // Traverse the list for positive line numbers (0-indexed)
        ListNode *current = state;
        while (current != NULL && line_address > 0) {
            current = current->next;
            line_address--;
        }
        return current;
    }
}

// Function: do_insert
// is_append_mode: 1 for append, 0 for insert
int do_insert(const Command *cmd, int *result_code, int is_append_mode) {
    *result_code = 0; // Initialize result_code to success

    ListNode *insert_after_node = get_line_by_address(cmd->line_start_address);

    if (insert_after_node == NULL) {
        insert_after_node = current_line; // Fallback if address not found
    }

    if (!is_append_mode && insert_after_node != NULL) {
        // If inserting (not appending), we want to insert BEFORE insert_after_node.
        // `list_insert_after` inserts after `prev_node`. So, we need `insert_after_node->prev`.
        insert_after_node = insert_after_node->prev;
    }

    // Validate input string for null bytes
    for (uint_fast i = 0; i < cmd->data_length; ++i) {
        if (cmd->text_data[i] == '\0') {
            return -1; // Error: null byte in input data
        }
    }

    uint_fast current_list_length = (uint_fast)list_length(&state);
    char *token_data_copy = strdup(cmd->text_data); // Duplicate to allow strtok to modify
    if (token_data_copy == NULL) return -1;

    char *line_str = strtok(token_data_copy, "\n");
    ListNode *last_new_node = NULL; // Tracks the most recently inserted node

    while (line_str != NULL) {
        size_t line_len = strlen(line_str);
        if (line_len > 0x100) { // Max line length check (256 characters)
            free(token_data_copy);
            return -1;
        }

        current_list_length++;
        if (current_list_length > 0x100) { // Max total lines check (256 lines)
            free(token_data_copy);
            return -1;
        }

        // Allocate memory for the new node (ListNode header + string data + null terminator)
        ListNode *new_node = (ListNode *)calloc(1, sizeof(ListNode) + line_len + 1);
        if (new_node == NULL) {
            free(token_data_copy);
            return -1;
        }
        strcpy(new_node->data, line_str);

        // For the first line, insert after `insert_after_node`.
        // For subsequent lines, insert after `last_new_node`.
        list_insert_after(&state, (last_new_node != NULL) ? last_new_node : insert_after_node, new_node);
        last_new_node = new_node;

        line_str = strtok(NULL, "\n");
    }

    free(token_data_copy);

    last_current_line = current_line;
    last_modified_line = last_new_node; // The last line inserted
    return 0;
}

// Function: append_command
int append_command(const Command *cmd, int *result_code) {
    *result_code = 0;
    return do_insert(cmd, result_code, 1); // 1 for append mode
}

// Function: insert_command
int insert_command(const Command *cmd, int *result_code) {
    *result_code = 0;
    return do_insert(cmd, result_code, 0); // 0 for insert mode
}

// Function: delete_command
int delete_command(const Command *cmd, int *result_code) {
    *result_code = 0;
    if (state == NULL) return -1;

    ListNode *start_node = get_line_by_address(cmd->line_start_address);
    if (start_node == NULL) return -1;

    ListNode *end_node = get_line_by_address(cmd->line_end_address);
    if (end_node == NULL) return -1;

    ListNode *current = start_node;
    ListNode *next_node_to_delete;

    // Loop from start_node up to and including end_node
    while (current != NULL && current != end_node->next) {
        next_node_to_delete = current->next; // Save next node before current is removed

        // Clear any marks pointing to the current node
        for (int i = 0; i < 28; ++i) {
            if (marks[i] == current) {
                marks[i] = NULL;
            }
        }

        list_remove(&state, current);
        free(current); // Free the node after removal

        current = next_node_to_delete;
    }

    last_modified_line = get_line_by_address(cmd->line_start_address); // Original param_1 + 1 is start_node index
    last_current_line = current_line;
    return 0;
}

// Function: change_command
int change_command(const Command *cmd, int *result_code) {
    *result_code = 0;
    if (delete_command(cmd, result_code) == 0) {
        return append_command(cmd, result_code);
    }
    return -1;
}

// Function: join_command
int join_command(const Command *cmd, int *result_code) {
    *result_code = 0;
    if (state == NULL) return -1;

    ListNode *start_node = get_line_by_address(cmd->line_start_address);
    if (start_node == NULL) return -1;

    ListNode *end_node = get_line_by_address(cmd->line_end_address);
    if (end_node == NULL) return -1;

    size_t total_length = 0;
    ListNode *current = start_node;
    // Calculate total length of strings to be joined
    while (current != NULL && current != end_node->next) {
        total_length += strlen(current->data);
        current = current->next;
    }

    // Allocate memory for the new joined node (ListNode header + total_length + null terminator)
    ListNode *joined_node = (ListNode *)calloc(1, sizeof(ListNode) + total_length + 1);
    if (joined_node == NULL) return -1;

    char *dest_ptr = joined_node->data;
    current = start_node;
    ListNode *nodes_to_free[256]; // Temporary storage for nodes to free
    int node_count = 0;

    // Copy data and collect nodes for removal
    while (current != NULL && current != end_node->next) {
        // Clear any marks pointing to the current node
        for (int i = 0; i < 28; ++i) {
            if (marks[i] == current) {
                marks[i] = NULL;
            }
        }
        strcpy(dest_ptr, current->data);
        dest_ptr += strlen(current->data);

        // Store current node for later removal and freeing
        if (node_count < sizeof(nodes_to_free) / sizeof(nodes_to_free[0])) {
            nodes_to_free[node_count++] = current;
        }
        current = current->next;
    }

    // Insert the new joined node before the original start_node
    list_insert_before(&state, start_node, joined_node);

    // Remove and free the original nodes that were joined
    for (int i = 0; i < node_count; ++i) {
        list_remove(&state, nodes_to_free[i]);
        free(nodes_to_free[i]);
    }

    last_current_line = current_line;
    last_modified_line = joined_node; // The newly joined line
    return 0;
}

// Function: mark_command
int mark_command(const Command *cmd, int *result_code) {
    *result_code = 0;
    if (state == NULL) return -1;

    // The original code implies data_length of 4, but uses `*(uint *)(param_1 + 0xd)`
    // then subtracts 'a'. This suggests a single character mark name.
    if (cmd->data_length < 1) { // Expecting at least one character for the mark name
        return -1;
    }

    uint_fast mark_index = (uint_fast)cmd->text_data[0] - 'a';
    if (mark_index >= 28) { // Marks are 'a' through 'z' (0-25), plus two more (26, 27) if 0x1b is 27.
        return -1;          // 0x1b is 27, so valid indices are 0 to 27.
    }

    ListNode *line_to_mark = get_line_by_address(cmd->line_start_address);
    if (line_to_mark == NULL) return -1;

    marks[mark_index] = line_to_mark;
    last_current_line = current_line;
    last_modified_line = line_to_mark;
    return 0;
}

// Function: get_mark_command
int get_mark_command(const Command *cmd, ResultLine **result_output) {
    *result_output = NULL;

    if (cmd->data_length < 1) { // Expecting at least one character for the mark name
        return -1;
    }

    uint_fast mark_index = (uint_fast)cmd->text_data[0] - 'a';
    if (mark_index >= 28) {
        return -1;
    }

    // Allocate ResultLine struct + space for the mark pointer
    ResultLine *result = (ResultLine *)calloc(1, sizeof(ResultLine) + sizeof(ListNode *));
    if (result == NULL) return -1;

    result->length = (uint_fast)sizeof(ListNode *); // Length of the data being returned (the pointer itself)
    // Copy the actual pointer value into the data field of the ResultLine
    memcpy(result->data, &marks[mark_index], sizeof(ListNode *));

    *result_output = result; // Assign the result to the output pointer
    last_modified_line = marks[mark_index]; // The line associated with the mark
    last_current_line = current_line;
    return 0;
}

// Function: list_command
int list_command(const Command *cmd, ResultLine **result_output) {
    *result_output = NULL;
    if (state == NULL) return -1;

    ListNode *start_node = get_line_by_address(cmd->line_start_address);
    if (start_node == NULL) return -1;

    ListNode *end_node = get_line_by_address(cmd->line_end_address);
    if (end_node == NULL) return -1;

    size_t total_length = 0;
    ListNode *current = start_node;
    // Calculate total length of strings including newlines
    while (current != NULL && current != end_node->next) {
        total_length += strlen(current->data) + 1; // +1 for newline character
        current = current->next;
    }

    // Allocate ResultLine struct + space for the combined string + null terminator
    ResultLine *result = (ResultLine *)calloc(1, sizeof(ResultLine) + total_length + 1);
    if (result == NULL) return -1;

    result->length = (uint_fast)total_length;
    char *dest_ptr = result->data;

    current = start_node;
    // Copy data to the result buffer
    while (current != NULL && current != end_node->next) {
        strcpy(dest_ptr, current->data);
        dest_ptr += strlen(current->data);
        *dest_ptr = '\n'; // Add newline
        dest_ptr++;
        current = current->next;
    }
    *dest_ptr = '\0'; // Null-terminate the entire string

    *result_output = result;
    last_current_line = current_line;
    last_modified_line = end_node;
    return 0;
}

// Function: num_command
int num_command(const Command *cmd, ResultLine **result_output) {
    *result_output = NULL;
    if (state == NULL) return -1;

    ListNode *start_node = get_line_by_address(cmd->line_start_address);
    if (start_node == NULL) return -1;

    ListNode *end_node = get_line_by_address(cmd->line_end_address);
    if (end_node == NULL) return -1;

    uint_fast current_line_num = 0;
    ListNode *current = state;
    // Determine the line number of the start_node
    while (current != NULL && current != start_node) {
        current_line_num++;
        current = current->next;
    }

    size_t total_output_len = 0;
    current = start_node;
    // Calculate total length for numbered output
    while (current != NULL && current != end_node->next) {
        current_line_num++;
        char num_buf[16]; // Buffer for line number string (e.g., up to 4 billion)
        sprintf(num_buf, "%u", current_line_num);
        total_output_len += strlen(num_buf) + 1; // Number + space
        total_output_len += strlen(current->data) + 1; // Line data + newline
        current = current->next;
    }

    // Allocate ResultLine struct + space for the combined string + null terminator
    ResultLine *result = (ResultLine *)calloc(1, sizeof(ResultLine) + total_output_len + 1);
    if (result == NULL) return -1;

    result->length = (uint_fast)total_output_len;
    char *dest_ptr = result->data;

    current_line_num = 0; // Reset line_num for actual output
    current = state;
    // Re-determine starting line number
    while (current != NULL && current != start_node) {
        current_line_num++;
        current = current->next;
    }

    current = start_node;
    // Copy numbered data to the result buffer
    while (current != NULL && current != end_node->next) {
        current_line_num++;
        sprintf(dest_ptr, "%u ", current_line_num); // Write line number and space
        dest_ptr += strlen(dest_ptr);
        strcpy(dest_ptr, current->data); // Write line data
        dest_ptr += strlen(current->data);
        *dest_ptr = '\n'; // Add newline
        dest_ptr++;
        current = current->next;
    }
    *dest_ptr = '\0'; // Null-terminate the entire string

    *result_output = result;
    last_current_line = current_line;
    last_modified_line = end_node;
    return 0;
}

// Function: do_search_command
// is_inverse_search: 1 for inverse search (no match), 0 for normal search (match)
int do_search_command(const Command *cmd, ResultLine **result_output, int is_inverse_search) {
    *result_output = NULL;
    if (state == NULL) return -1;

    ListNode *start_node = get_line_by_address(cmd->line_start_address);
    if (start_node == NULL) return -1;

    ListNode *end_node = get_line_by_address(cmd->line_end_address);
    if (end_node == NULL) return -1;

    size_t total_matched_len = 0;
    ListNode *current = start_node;
    // Dummy variables for regex_match, as their internal usage (local_38, local_3c) is unclear
    void *regex_match_info = NULL;
    void *regex_sub_match_info = NULL;

    // First pass: Calculate total length of matching lines
    while (current != NULL && current != end_node->next) {
        int match_result = regex_match(cmd->text_data, current->data, regex_match_info, regex_sub_match_info);
        bool should_include = false;

        if (!is_inverse_search) { // Normal search: include if regex_match returns 0 (match)
            if (match_result == 0) {
                should_include = true;
            }
        } else { // Inverse search: include if regex_match returns -1 (no match)
            if (match_result == -1) {
                should_include = true;
            }
        }

        if (should_include) {
            total_matched_len += strlen(current->data) + 1; // +1 for newline
        }
        current = current->next;
    }

    if (total_matched_len == 0) {
        return -1; // No matching lines found
    }

    // Allocate ResultLine struct + space for the combined matched lines + null terminator
    ResultLine *result = (ResultLine *)calloc(1, sizeof(ResultLine) + total_matched_len + 1);
    if (result == NULL) return -1;

    result->length = (uint_fast)total_matched_len;
    char *dest_ptr = result->data;

    current = start_node;
    // Second pass: Copy matching lines to the result buffer
    while (current != NULL && current != end_node->next) {
        int match_result = regex_match(cmd->text_data, current->data, regex_match_info, regex_sub_match_info);
        bool should_include = false;

        if (!is_inverse_search) {
            if (match_result == 0) {
                should_include = true;
            }
        } else {
            if (match_result == -1) {
                should_include = true;
            }
        }

        if (should_include) {
            strcpy(dest_ptr, current->data);
            dest_ptr += strlen(current->data);
            *dest_ptr = '\n'; // Add newline
            dest_ptr++;
        }
        current = current->next;
    }
    *dest_ptr = '\0'; // Null-terminate the entire string

    *result_output = result;
    last_current_line = current_line;
    last_modified_line = end_node;
    return 0;
}

// Function: search_command
int search_command(const Command *cmd, ResultLine **result_output) {
    *result_output = NULL;
    return do_search_command(cmd, result_output, 0); // 0 for normal search
}

// Function: inverse_search_command
int inverse_search_command(const Command *cmd, ResultLine **result_output) {
    *result_output = NULL;
    return do_search_command(cmd, result_output, 1); // 1 for inverse search
}

// Function: transform_command
int transform_command(const Command *cmd, int *result_code) {
    *result_code = 0;
    if (state == NULL) return -1;

    uint_fast transform_func_array_index;
    char char_to_transform = '\0'; // Character passed as argument to the transform function

    if (transform_mode == 0) {
        if (cmd->data_length < 1) return -1; // Expecting at least one character
        transform_func_array_index = (uint_fast)cmd->text_data[0] - 'a';
        char_to_transform = cmd->text_data[0]; // The character itself is passed
        if (transform_func_array_index >= 28) return -1; // 0x1b is 27, so 0-27
    } else {
        if (cmd->data_length < 1) return -1; // Still expecting a single character
        transform_func_array_index = 26; // Corresponds to 'z' index (0x1a)
        char_to_transform = cmd->text_data[0]; // The character itself is passed
    }

    // The original code uses `local_10 + 0x1c` for the index into the function pointer array.
    // `0x1c` is 28. So, `transform_funcs[transform_func_array_index + 28]`
    if (transform_funcs[transform_func_array_index + 28] == NULL) {
        return -1;
    }

    ListNode *start_node = get_line_by_address(cmd->line_start_address);
    if (start_node == NULL) return -1;

    ListNode *end_node = get_line_by_address(cmd->line_end_address);
    if (end_node == NULL) return -1;

    ListNode *current = start_node;
    while (current != NULL && current != end_node->next) {
        for (char *char_ptr = current->data; *char_ptr != '\0'; ++char_ptr) {
            *char_ptr = transform_funcs[transform_func_array_index + 28](char_to_transform);
        }
        current = current->next;
    }

    last_current_line = current_line;
    last_modified_line = end_node;
    return 0;
}

// Function: run_command
// `output_ptr` is `int*` for commands returning status, or `ResultLine**` for commands returning data.
int run_command(const Command *cmd, void *output_ptr) {
    if (cmd == NULL) return -1;

    // For commands that return a ResultLine*, `output_ptr` is `ResultLine**`
    // For commands that return an int status code, `output_ptr` is `int*`
    // The original code uses `*param_2 = 0;` at the beginning of many functions,
    // which suggests param_2 is a general output/status pointer.

    switch (cmd->command_char) {
        case '=': // get_mark_command
            return get_mark_command(cmd, (ResultLine **)output_ptr);
        case 'a': // append_command
            return append_command(cmd, (int *)output_ptr);
        case 'c': // change_command
            return change_command(cmd, (int *)output_ptr);
        case 'd': // delete_command
            return delete_command(cmd, (int *)output_ptr);
        case 'g': // search_command
            return search_command(cmd, (ResultLine **)output_ptr);
        case 'i': // insert_command
            return insert_command(cmd, (int *)output_ptr);
        case 'j': // join_command
            return join_command(cmd, (int *)output_ptr);
        case 'l': // list_command
            return list_command(cmd, (ResultLine **)output_ptr);
        case 'm': // mark_command
            return mark_command(cmd, (int *)output_ptr);
        case 'n': // num_command
            return num_command(cmd, (ResultLine **)output_ptr);
        case 'q': // quit command
            _terminate(); // Exits the program
            return 0; // Should not be reached
        case 't': // transform_command
            return transform_command(cmd, (int *)output_ptr);
        case 'v': // inverse_search_command
            return inverse_search_command(cmd, (ResultLine **)output_ptr);
        default:
            return -1; // Unknown command
    }
}