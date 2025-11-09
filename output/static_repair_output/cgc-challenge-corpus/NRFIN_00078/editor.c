#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h> // For bool type

// Define custom types based on the snippet's undefined types
typedef uint32_t undefined4;
typedef char undefined;
typedef unsigned int uint;

// Constants for return values
#define SUCCESS 0
#define FAILURE 0xFFFFFFFF // -1 in two's complement for uint32_t

// Structure for a line in the linked list
typedef struct Line Line;
struct Line {
    Line *next;     // Pointer to the next line in the list (4 bytes on 32-bit assumption)
    uint32_t dummy; // Padding or another field, making text start at offset 8
    char text[];    // Flexible array member for the actual text content (starts at offset 8)
};
// Size of Line struct header is 8 bytes (sizeof(Line*) + sizeof(uint32_t))

// Global variables, re-typed and given meaningful names
Line *state = NULL; // Head of the linked list
Line *g_last_inserted_line = NULL; // Corresponds to DAT_00017004, likely a sentinel or last line
Line *g_current_line = NULL; // Corresponds to DAT_00017070, current line or last affected line
Line *g_last_accessed_line = NULL; // Corresponds to DAT_00017074, another last accessed pointer
int g_flag_transform = 0; // Corresponds to DAT_00017078, possibly a flag for transform command type
Line *g_marked_lines[28] = {0}; // Corresponds to DAT_00017008, array of marked lines (0x1c = 28)

// Type for transform functions (takes a char, returns a char)
typedef char (*char_transform_func_t)(char);
char_transform_func_t g_transform_funcs[56] = {0}; // Corresponds to DAT_0001700c

// Dummy implementations for external functions
size_t list_length(Line **head) {
    size_t count = 0;
    Line *current = *head;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

void list_insert_after(Line **head, Line *prev, Line *new_node) {
    if (!new_node) return;
    if (!prev) { // Insert at head if prev is NULL
        new_node->next = *head;
        *head = new_node;
    } else {
        new_node->next = prev->next;
        prev->next = new_node;
    }
}

void list_insert_before(Line **head, Line *next_node, Line *new_node) {
    if (!new_node) return;
    if (!next_node || *head == next_node) { // Insert at head
        new_node->next = *head;
        *head = new_node;
    } else {
        Line *current = *head;
        while (current && current->next != next_node) {
            current = current->next;
        }
        if (current) {
            new_node->next = current->next;
            current->next = new_node;
        } else { // next_node not found, insert at end (or head if list empty)
            list_insert_after(head, NULL, new_node); // Fallback to insert at head/end if next_node not found
        }
    }
}

void list_remove(Line **head, Line *node_to_remove) {
    if (!head || !*head || !node_to_remove) return;

    if (*head == node_to_remove) {
        *head = node_to_remove->next;
        return;
    }

    Line *current = *head;
    while (current && current->next != node_to_remove) {
        current = current->next;
    }

    if (current && current->next == node_to_remove) {
        current->next = node_to_remove->next;
    }
}

int regex_match(const char *pattern, const char *text, void *match_info, void *dummy) {
    // Dummy regex_match: simple substring search
    if (strstr(text, pattern) != NULL) {
        return 0; // Match found
    }
    return -1; // No match
}

void _terminate(void) {
    exit(0);
}

// itoa is non-standard, using sprintf instead.
char *my_itoa(int value, char *str) {
    sprintf(str, "%d", value);
    return str;
}


// Function: get_line_by_address
Line *get_line_by_address(uint32_t address_param) {
    Line *current_line = state;

    if ((int)address_param < 0) {
        // Accessing marked lines array using negative index
        // `~address_param` converts negative index to positive (e.g., -1 -> 0, -2 -> 1)
        uint32_t index = ~address_param;
        if (index < 28) {
            current_line = g_marked_lines[index];
        } else {
            current_line = NULL; // Out of bounds
        }
    } else {
        // Iterate through the list by index
        while (current_line && address_param != 0) {
            current_line = current_line->next;
            address_param--;
        }
    }
    return current_line;
}

// Function: do_insert
undefined4 do_insert(char *command_ptr, undefined4 *result_out, int insert_mode) {
    *result_out = 0;

    uint32_t line_start_idx = *(uint32_t*)(command_ptr + 4);
    uint32_t command_len = *(uint32_t*)(command_ptr + 36);
    char *command_arg_str = command_ptr + 52;

    Line *insert_target_line = get_line_by_address(line_start_idx);
    if (!insert_target_line) {
        insert_target_line = g_last_inserted_line;
    }

    // Validate command string for null terminators
    for (uint i = 0; i < command_len; ++i) {
        if (command_arg_str[i] == '\0') {
            return FAILURE;
        }
    }

    uint32_t current_line_count = list_length(&state);
    char *token_str_copy = strdup(command_arg_str); // Duplicate to allow strtok to modify
    if (!token_str_copy) return FAILURE;

    char *token = strtok(token_str_copy, "\n");
    Line *last_new_line = NULL; // Keep track of the last line inserted

    while (token) {
        size_t token_len = strlen(token);
        if (token_len > 0x100) { // Check line length limit
            free(token_str_copy);
            return FAILURE;
        }

        current_line_count++;
        if (current_line_count > 0x100) { // Check total line count limit
            free(token_str_copy);
            return FAILURE;
        }

        Line *new_line = (Line *)calloc(1, sizeof(Line) + token_len + 1); // +1 for null terminator
        if (!new_line) {
            free(token_str_copy);
            return FAILURE;
        }
        strcpy(new_line->text, token);

        if (insert_mode == 0) { // Insert mode: insert before `insert_target_line`
            list_insert_before(&state, insert_target_line, new_line);
            // If `insert_target_line` was NULL, `list_insert_before` will insert at end.
            // If it was the head, it inserts at head.
            // For subsequent lines in multi-line insert, they should be inserted before the same `insert_target_line`
        } else { // Append mode: insert after `insert_target_line`
            list_insert_after(&state, insert_target_line, new_line);
            insert_target_line = new_line; // For subsequent appends in this loop, append after the newly inserted line
        }
        
        last_new_line = new_line;

        token = strtok(NULL, "\n");
    }

    free(token_str_copy);

    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }
    g_current_line = last_new_line;

    return SUCCESS;
}

// Function: append_command
undefined4 append_command(char *command_ptr, undefined4 *result_out) {
    return do_insert(command_ptr, result_out, 1); // 1 for append mode
}

// Function: insert_command
undefined4 insert_command(char *command_ptr, undefined4 *result_out) {
    return do_insert(command_ptr, result_out, 0); // 0 for insert mode
}

// Function: delete_command
undefined4 delete_command(char *command_ptr, undefined4 *result_out) {
    *result_out = 0;

    if (!state) {
        return FAILURE;
    }

    uint32_t line_start_idx = *(uint32_t*)(command_ptr + 4);
    uint32_t line_end_idx = *(uint32_t*)(command_ptr + 20);

    Line *start_line = get_line_by_address(line_start_idx);
    if (!start_line) {
        return FAILURE;
    }

    Line *end_line_exclusive = get_line_by_address(line_end_idx);

    Line *current = start_line;
    while (current && current != end_line_exclusive) {
        Line *next_line = current->next;

        for (uint i = 0; i < 28; ++i) { // Clear any marks pointing to the current line
            if (g_marked_lines[i] == current) {
                g_marked_lines[i] = NULL;
            }
        }

        list_remove(&state, current);
        free(current);
        current = next_line;
    }

    g_current_line = get_line_by_address(line_start_idx);
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: change_command
undefined4 change_command(char *command_ptr, undefined4 *result_out) {
    *result_out = 0;
    undefined4 delete_result = delete_command(command_ptr, result_out);
    if (delete_result == SUCCESS) {
        return append_command(command_ptr, result_out);
    }
    return FAILURE;
}

// Function: join_command
undefined4 join_command(char *command_ptr, undefined4 *result_out) {
    *result_out = 0;

    if (!state) {
        return FAILURE;
    }

    uint32_t line_start_idx = *(uint32_t*)(command_ptr + 4);
    uint32_t line_end_idx = *(uint32_t*)(command_ptr + 20);

    Line *start_line = get_line_by_address(line_start_idx);
    if (!start_line) {
        return FAILURE;
    }

    Line *end_line_exclusive = get_line_by_address(line_end_idx);

    size_t total_len = 0;
    for (Line *current = start_line; current && current != end_line_exclusive; current = current->next) {
        total_len += strlen(current->text);
    }

    Line *joined_line = (Line *)calloc(1, sizeof(Line) + total_len + 1); // +1 for null terminator
    if (!joined_line) {
        return FAILURE;
    }
    char *dest_ptr = joined_line->text;

    Line *current = start_line;
    while (current && current != end_line_exclusive) {
        Line *next_line = current->next;

        for (uint i = 0; i < 28; ++i) { // Clear any marks pointing to the current line
            if (g_marked_lines[i] == current) {
                g_marked_lines[i] = NULL;
            }
        }

        strcpy(dest_ptr, current->text);
        dest_ptr += strlen(current->text);

        if (current != start_line) { // Remove and free all lines except the first one
            list_remove(&state, current);
            free(current);
        }
        current = next_line;
    }

    list_insert_before(&state, start_line, joined_line); // Insert joined line before the original start_line
    list_remove(&state, start_line); // Remove original start_line
    free(start_line); // Free original start_line

    g_current_line = joined_line;
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: mark_command
undefined4 mark_command(char *command_ptr, undefined4 *result_out) {
    *result_out = 0;

    if (!state) {
        return FAILURE;
    }

    uint32_t command_len = *(uint32_t*)(command_ptr + 36);
    uint32_t mark_char_val = *(uint32_t*)(command_ptr + 52); // First 4 bytes of arg

    if (command_len != 4) { // Expecting a 4-byte character for the mark (e.g., 'a\0\0\0')
        return FAILURE;
    }

    uint32_t mark_idx = mark_char_val - 'a'; // Convert char 'a'-'z' to index 0-25
    if (mark_idx >= 28) { // Max index 27
        return FAILURE;
    }

    Line *line_to_mark = get_line_by_address(*(uint32_t*)(command_ptr + 4));
    if (!line_to_mark) {
        return FAILURE;
    }

    g_marked_lines[mark_idx] = line_to_mark;
    g_current_line = line_to_mark;
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: get_mark_command
undefined4 get_mark_command(char *command_ptr, void **result_out) {
    *result_out = NULL;

    uint32_t command_len = *(uint32_t*)(command_ptr + 36);
    uint32_t mark_char_val = *(uint32_t*)(command_ptr + 52);

    if (command_len != 4) {
        return FAILURE;
    }

    uint32_t mark_idx = mark_char_val - 'a';
    if (mark_idx >= 28) {
        return FAILURE;
    }

    // Structure for the returned mark data (16 bytes total)
    typedef struct {
        uint32_t dummy0;
        uint32_t data_len; // set to 4 (size of pointer)
        uint32_t dummy1;
        Line *marked_line_ptr; // The actual marked line pointer
    } MarkResult;

    MarkResult *result = (MarkResult *)calloc(1, sizeof(MarkResult));
    if (!result) {
        return FAILURE;
    }
    *result_out = result;

    result->data_len = 4; // Size of the pointer
    result->marked_line_ptr = g_marked_lines[mark_idx];

    g_current_line = result->marked_line_ptr;
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: list_command
undefined4 list_command(char *command_ptr, void **result_out) {
    *result_out = NULL;

    if (!state) {
        return FAILURE;
    }

    uint32_t line_start_idx = *(uint32_t*)(command_ptr + 4);
    uint32_t line_end_idx = *(uint32_t*)(command_ptr + 20);

    Line *start_line = get_line_by_address(line_start_idx);
    if (!start_line) {
        return FAILURE;
    }

    Line *end_line_exclusive = get_line_by_address(line_end_idx);

    size_t total_output_len = 0;
    for (Line *current = start_line; current && current != end_line_exclusive; current = current->next) {
        total_output_len += strlen(current->text) + 1; // +1 for newline character
    }

    // Structure for the returned list data (12 bytes metadata + text_data)
    typedef struct {
        uint32_t dummy0;
        uint32_t data_len; // offset 4
        uint32_t dummy1; // offset 8
        char text_data[]; // offset 12
    } ResultData;

    ResultData *result = (ResultData *)calloc(1, sizeof(ResultData) + total_output_len + 1); // +1 for null terminator
    if (!result) {
        return FAILURE;
    }
    *result_out = result;

    result->data_len = total_output_len;
    char *dest_ptr = result->text_data;

    for (Line *current = start_line; current && current != end_line_exclusive; current = current->next) {
        strcpy(dest_ptr, current->text);
        strcat(dest_ptr, "\n");
        dest_ptr += strlen(current->text) + 1; // Advance past line and newline
    }

    g_current_line = end_line_exclusive;
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: num_command
undefined4 num_command(char *command_ptr, void **result_out) {
    *result_out = NULL;

    if (!state) {
        return FAILURE;
    }

    uint32_t line_start_idx = *(uint32_t*)(command_ptr + 4);
    uint32_t line_end_idx = *(uint32_t*)(command_ptr + 20);

    Line *start_line = get_line_by_address(line_start_idx);
    if (!start_line) {
        return FAILURE;
    }

    Line *end_line_exclusive = get_line_by_address(line_end_idx);

    int current_line_num = 0; // 0-based index initially
    Line *current = state;
    while (current && current != start_line) {
        current_line_num++;
        current = current->next;
    }

    size_t total_output_len = 0;
    char num_buf[20]; // Buffer for my_itoa/sprintf
    
    current = start_line; // Reset for calculation pass
    int temp_line_num = current_line_num; // Use temporary counter for calculation
    while (current && current != end_line_exclusive) {
        temp_line_num++; // Line numbers are 1-based for display
        my_itoa(temp_line_num, num_buf);
        total_output_len += strlen(num_buf) + 1; // Number + space
        total_output_len += strlen(current->text) + 1; // Line text + newline
        current = current->next;
    }

    typedef struct {
        uint32_t dummy0;
        uint32_t data_len;
        uint32_t dummy1;
        char text_data[];
    } ResultData;

    ResultData *result = (ResultData *)calloc(1, sizeof(ResultData) + total_output_len + 1);
    if (!result) {
        return FAILURE;
    }
    *result_out = result;

    char *dest_ptr = result->text_data;
    current = start_line; // Reset for copy pass
    temp_line_num = current_line_num; // Reset counter for copy pass

    while (current && current != end_line_exclusive) {
        temp_line_num++;
        my_itoa(temp_line_num, num_buf);
        strcpy(dest_ptr, num_buf);
        strcat(dest_ptr, " ");
        strcat(dest_ptr, current->text);
        strcat(dest_ptr, "\n");
        dest_ptr += strlen(dest_ptr); // Advance by the length of the string just copied
        current = current->next;
    }

    result->data_len = strlen(result->text_data);

    g_current_line = end_line_exclusive;
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: do_search_command
undefined4 do_search_command(char *command_ptr, void **result_out, int inverse_search_mode) {
    *result_out = NULL;

    if (!state) {
        return FAILURE;
    }

    uint32_t line_start_idx = *(uint32_t*)(command_ptr + 4);
    uint32_t line_end_idx = *(uint32_t*)(command_ptr + 20);
    char *pattern = command_ptr + 52;

    Line *start_line = get_line_by_address(line_start_idx);
    if (!start_line) {
        return FAILURE;
    }

    Line *end_line_exclusive = get_line_by_address(line_end_idx);

    size_t total_output_len = 0;
    Line *current = start_line;
    while (current && current != end_line_exclusive) {
        int match_result = regex_match(pattern, current->text, NULL, NULL);
        bool should_include = false;

        if (inverse_search_mode == 0) { // Normal search
            if (match_result == 0) { // Found a match
                should_include = true;
            }
        } else { // Inverse search
            if (match_result == -1) { // Did NOT find a match
                should_include = true;
            }
        }

        if (should_include) {
            total_output_len += strlen(current->text) + 1; // +1 for newline
        }
        current = current->next;
    }

    if (total_output_len == 0) {
        return FAILURE; // No lines matched the search criteria
    }

    typedef struct {
        uint32_t dummy0;
        uint32_t data_len;
        uint32_t dummy1;
        char text_data[];
    } ResultData;

    ResultData *result = (ResultData *)calloc(1, sizeof(ResultData) + total_output_len + 1);
    if (!result) {
        return FAILURE;
    }
    *result_out = result;

    result->data_len = total_output_len;
    char *dest_ptr = result->text_data;
    current = start_line; // Reset for second pass

    while (current && current != end_line_exclusive) {
        int match_result = regex_match(pattern, current->text, NULL, NULL);
        bool should_include = false;

        if (inverse_search_mode == 0) { // Normal search
            if (match_result == 0) {
                should_include = true;
            }
        } else { // Inverse search
            if (match_result == -1) {
                should_include = true;
            }
        }

        if (should_include) {
            strcpy(dest_ptr, current->text);
            strcat(dest_ptr, "\n");
            dest_ptr += strlen(current->text) + 1;
        }
        current = current->next;
    }

    g_current_line = end_line_exclusive;
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: search_command
undefined4 search_command(char *command_ptr, undefined4 *result_out) {
    return do_search_command(command_ptr, (void**)result_out, 0); // 0 for normal search
}

// Function: inverse_search_command
undefined4 inverse_search_command(char *command_ptr, undefined4 *result_out) {
    return do_search_command(command_ptr, (void**)result_out, 1); // 1 for inverse search
}

// Function: transform_command
undefined4 transform_command(char *command_ptr, undefined4 *result_out) {
    *result_out = 0;

    if (!state) {
        return FAILURE;
    }

    uint32_t command_len = *(uint32_t*)(command_ptr + 36);
    char *transform_arg = command_ptr + 52;

    uint32_t transform_func_idx;

    if (g_flag_transform == 0) {
        if (command_len != 4) { // Expecting a single character for transform type
            return FAILURE;
        }
        transform_func_idx = (*(uint32_t*)transform_arg) - 'a'; // 'a' to 'z' map to 0-25
        if (transform_func_idx >= 28) {
            return FAILURE;
        }
    } else {
        if (command_len != 8) { // Expecting an 8-byte argument for this mode
            return FAILURE;
        }
        transform_func_idx = 26; // Special transform func index (0x1a)
    }

    // Transform functions are stored at an offset of 28 (0x1c) in the g_transform_funcs array
    if (!g_transform_funcs[transform_func_idx + 28]) {
        return FAILURE;
    }

    Line *start_line = get_line_by_address(*(uint32_t*)(command_ptr + 4));
    if (!start_line) {
        return FAILURE;
    }

    Line *end_line_exclusive = get_line_by_address(*(uint32_t*)(command_ptr + 20));

    Line *current = start_line;
    while (current && current != end_line_exclusive) {
        char *line_char_ptr = current->text;
        while (*line_char_ptr != '\0') {
            *line_char_ptr = g_transform_funcs[transform_func_idx + 28](*line_char_ptr);
            line_char_ptr++;
        }
        current = current->next;
    }

    g_current_line = end_line_exclusive;
    g_last_accessed_line = g_last_inserted_line;
    if (!g_last_inserted_line) {
        g_last_accessed_line = NULL;
    }

    return SUCCESS;
}

// Function: run_command
undefined4 run_command(char *command_ptr, undefined4 *result_out) {
    *result_out = 0; // Initialize output parameter

    switch (*command_ptr) { // The first byte of the command_ptr is the command character
        case '=': return get_mark_command(command_ptr, (void**)result_out);
        case 'a': return append_command(command_ptr, result_out);
        case 'c': return change_command(command_ptr, result_out);
        case 'd': return delete_command(command_ptr, result_out);
        case 'g': return search_command(command_ptr, result_out);
        case 'i': return insert_command(command_ptr, result_out);
        case 'j': return join_command(command_ptr, result_out);
        case 'l': return list_command(command_ptr, (void**)result_out);
        case 'm': return mark_command(command_ptr, result_out);
        case 'n': return num_command(command_ptr, (void**)result_out);
        case 'q': _terminate(); return SUCCESS; // Should not reach here
        case 't': return transform_command(command_ptr, result_out);
        case 'v': return inverse_search_command(command_ptr, result_out);
        default: return FAILURE;
    }
}