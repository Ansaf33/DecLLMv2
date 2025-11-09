#include <stdio.h>   // For sprintf
#include <stdlib.h>  // For malloc, free, calloc, strdup
#include <string.h>  // For strlen, strcpy, strncpy, memcpy, memset, strcmp, strncmp
#include <stdint.h>  // For intptr_t
#include <math.h>    // For roundf

// --- Placeholder for custom functions and data ---

// Placeholder for DAT_00015264 and DAT_00015267
// Assuming these are string literals or similar constant data.
static const char * const DAT_00015264 = "<!--";
static const char * const DAT_00015267 = "-->";

// Placeholder for DAT_00015280 (used in sadface_var2str for float formatting)
static const float DAT_00015280 = 100.0f;

// Define ROUND macro based on typical decompiled usage (rounds to nearest integer)
#define ROUND(x) roundf(x)

// Define a simple global stack for demonstration purposes.
// In a real application, this would be a dynamically allocated structure.
#define MAX_STACK_SIZE 64
static void *g_stack_items[MAX_STACK_SIZE];
static int g_stack_top = -1;

void *stack_new(size_t capacity) {
    // For simplicity, reset the global stack.
    // In a real implementation, it would allocate and return a stack context.
    g_stack_top = -1;
    return (void*)1; // Return non-NULL to indicate success
}

void stack_destroy(void *stack_handle) {
    // In a real implementation, free stack memory.
    g_stack_top = -1; // Reset global stack
}

void stack_push(void *stack_handle, void *item) {
    if (g_stack_top < MAX_STACK_SIZE - 1) {
        g_stack_items[++g_stack_top] = item;
    }
}

void *stack_pop(void *stack_handle) {
    if (g_stack_top >= 0) {
        return g_stack_items[g_stack_top--];
    }
    return NULL;
}

void *stack_peek(void *stack_handle) {
    if (g_stack_top >= 0) {
        return g_stack_items[g_stack_top];
    }
    return NULL;
}

// Placeholder for a dictionary lookup function
// Assuming it returns an integer representing a pointer to a variable definition (unsigned int*)
// or 0 if not found.
int dict_find(void *dict_handle, const char *key) {
    // Dummy implementation: returns a dummy pointer for specific keys
    // In a real system, this would look up `key` in `dict_handle` and return
    // a pointer to a variable structure.
    static unsigned int dummy_var_int[] = {1, 123}; // Type 1 (int), Value 123
    static unsigned int dummy_var_float[] = {2}; // Type 2 (float)
    float f_val = 45.67f;
    memcpy(&dummy_var_float[1], &f_val, sizeof(float)); // Store float bits
    static unsigned int dummy_var_bool_true[] = {3, 1}; // Type 3 (bool), Value 1 (true)
    static unsigned int dummy_var_bool_false[] = {3, 0}; // Type 3 (bool), Value 0 (false)
    static unsigned int dummy_var_string[] = {0}; // Type 0 (string)
    static char dummy_string_val[] = "hello world";
    // Place the pointer into the unsigned int array. This assumes intptr_t fits in unsigned int[1].
    // If sizeof(char*) > sizeof(unsigned int), this will cause issues.
    // For 64-bit systems, char* is 8 bytes, unsigned int is 4 bytes.
    // So this array should be unsigned long long or intptr_t.
    // Let's assume the underlying system handles this type-punning safely or it's 32-bit.
    // For safety, we should use a union or a proper struct.
    // Given the original code's `param_1[1]` access for `char*` and `uint`,
    // this is the most faithful interpretation of the original code's intent.
    *(char**)&dummy_var_string[1] = dummy_string_val;

    if (strcmp(key, "my_int_var") == 0) return (intptr_t)dummy_var_int;
    if (strcmp(key, "my_float_var") == 0) return (intptr_t)dummy_var_float;
    if (strcmp(key, "my_bool_true") == 0) return (intptr_t)dummy_var_bool_true;
    if (strcmp(key, "my_bool_false") == 0) return (intptr_t)dummy_var_bool_false;
    if (strcmp(key, "my_string_var") == 0) return (intptr_t)dummy_var_string;

    return 0; // Not found
}

// --- sadface_node_t structure definition ---
// This structure is inferred from how nodes are allocated (0x14 bytes = 5 * sizeof(unsigned int) on 32-bit, or mixed types)
// and how their fields are accessed (e.g., node[0], node[1], node[2], node[3], node[4]).
// Assuming a 64-bit system, intptr_t for pointers, size_t for lengths where appropriate.
typedef struct sadface_node {
    unsigned int type; // 0: root, 1: text, 2: variable, 3: conditional block
    struct sadface_node *next; // Points to the next sibling node in the linked list
    void *data_ptr;    // Pointer to content (e.g., text start, var name start, condition name start)
    size_t data_len;   // Length of content
    void *child_or_value; // For conditional, points to first child node of block. For var, points to value struct.
} sadface_node_t;

// --- Function Prototypes (to allow mutual recursion/forward declaration) ---
void sadface_node_destroy_recursive(sadface_node_t *node);
void sadface_destroy(intptr_t *sadface_data); // Forward declaration

// Function: strstr (custom implementation)
char * strstr(const char *__haystack, const char *__needle) {
    if (!__haystack || !__needle) {
        return NULL;
    }

    for (; *__haystack != '\0'; ++__haystack) {
        const char *h = __haystack;
        const char *n = __needle;
        while (*h != '\0' && *n != '\0' && *h == *n) {
            ++h;
            ++n;
        }
        if (*n == '\0') { // Entire needle found
            return (char *)__haystack;
        }
    }
    return NULL;
}

// Function: sadface_init
// param_1: receives a pointer to the allocated sadface_data structure
// param_2: optional array of intptr_t for custom start/end tags
// param_3: string containing the content to be parsed
// param_4: dictionary handle (void*)
int sadface_init(intptr_t **param_1, const intptr_t *param_2, const char *param_3, intptr_t param_4_dict_handle) {
    intptr_t *data_block = NULL;

    if (!param_1 || !param_3) {
        return -1; // Invalid input parameters
    }

    data_block = (intptr_t *)malloc(0x18); // Allocate 24 bytes
    if (!data_block) {
        return -1; // Malloc failed
    }

    // Initialize data_block[0] and data_block[1] (start/end tags)
    if (!param_2) {
        data_block[0] = (intptr_t)DAT_00015264;
        data_block[1] = (intptr_t)DAT_00015267;
    } else {
        data_block[0] = (param_2[0] == 0) ? (intptr_t)DAT_00015264 : param_2[0];
        data_block[1] = (param_2[1] == 0) ? (intptr_t)DAT_00015267 : param_2[1];
    }

    // Duplicate param_3 (content string) and store its pointer and length
    data_block[2] = (intptr_t)strdup(param_3); // content_base_str
    if (!((char*)data_block[2])) { // strdup failed
        free(data_block);
        return -1;
    }

    data_block[3] = (intptr_t)strlen(param_3); // content_len
    data_block[4] = (intptr_t)((char*)data_block[2]); // current_pos_str, initially points to content_base
    data_block[5] = param_4_dict_handle; // dict_handle

    *param_1 = data_block;
    return 0;
}

// Function: sadface_var2str
// param_1: array of unsigned int, where param_1[0] is type, param_1[1] is value (or pointer)
char * sadface_var2str(const unsigned int *param_1) {
    char *dest = (char *)calloc(1, 0x100); // Allocate 256 bytes for string
    if (!dest) {
        return NULL;
    }

    if (!param_1) {
        free(dest);
        return NULL;
    }

    unsigned int type = param_1[0];

    if (type == 3) { // Boolean
        strcpy(dest, (param_1[1] == 0) ? "false" : "true");
    } else if (type == 2) { // Float/Decimal
        float f_val;
        // Interpret param_1[1] as a float (type-punning via memcpy for safety)
        memcpy(&f_val, &param_1[1], sizeof(float));
        
        int int_part = (int)ROUND(f_val);
        int frac_part = (int)ROUND(DAT_00015280 * (f_val - (float)int_part));
        
        if (frac_part < 0) {
            frac_part = -frac_part;
        }
        frac_part %= 100; // Ensure it's 0-99

        int len = sprintf(dest, "%d", int_part);
        sprintf(dest + len, ".%02d", frac_part);
    } else if (type == 0) { // String (param_1[1] is char*)
        free(dest); // Free the calloc'd buffer, as we're returning an existing pointer
        dest = (char *)(intptr_t)param_1[1]; // Return the stored string pointer directly
    } else if (type == 1) { // Integer
        sprintf(dest, "%d", (int)param_1[1]);
    }
    return dest;
}

// Function: _find_sadface
// sadface_data structure: [start_tag_str, end_tag_str, content_base_str, content_len, current_pos_str, dict_handle]
// start_offset: receives offset of found start tag from content_base_str
// end_ptr: receives pointer to position after found end tag
int _find_sadface(intptr_t *sadface_data, int *start_offset, char **end_ptr) {
    if (!sadface_data || !start_offset || !end_ptr) {
        return 0;
    }

    const char *start_tag = (const char *)sadface_data[0];
    const char *end_tag = (const char *)sadface_data[1];
    const char *content_base = (const char *)sadface_data[2];
    const char *current_pos = (const char *)sadface_data[4]; // Current search position (char*)

    if (!start_tag || !end_tag || !content_base || !current_pos) {
        return 0;
    }

    char *found_start_tag_pos = strstr(current_pos, start_tag);
    if (!found_start_tag_pos) {
        return 0; // Start tag not found
    }

    size_t start_tag_len = strlen(start_tag);
    char *found_end_tag_pos = strstr(found_start_tag_pos + start_tag_len, end_tag);

    if (!found_end_tag_pos) {
        return -1; // End tag not found after start tag (error condition)
    }

    *start_offset = (int)(found_start_tag_pos - content_base);
    *end_ptr = found_end_tag_pos + strlen(end_tag);

    return 1; // Success
}

// Function: parse_sadface
// sadface_data structure: [start_tag_str, end_tag_str, content_base_str, content_len, current_pos_str, dict_handle]
// Returns the root node of the parsed sadface tree, or NULL on error.
sadface_node_t * parse_sadface(intptr_t *sadface_data) {
    const char *content_base_ptr = (const char *)sadface_data[2];
    const char *content_end_ptr = content_base_ptr + (size_t)sadface_data[3]; // content_len is at sadface_data[3]
    void *dict_handle = (void *)sadface_data[5];

    void *stack_handle = NULL;
    sadface_node_t *root_node = NULL;
    sadface_node_t *current_linked_node = NULL; // The node whose `next` pointer we are currently setting

    stack_handle = stack_new(MAX_STACK_SIZE);
    root_node = (sadface_node_t *)malloc(sizeof(sadface_node_t));

    if (!stack_handle || !root_node) {
        if (stack_handle) stack_destroy(stack_handle);
        if (root_node) free(root_node);
        sadface_destroy(sadface_data); // Free the sadface_data object as per original code's cleanup
        return NULL;
    }

    memset(root_node, 0, sizeof(sadface_node_t));
    root_node->type = 0; // Root node
    stack_push(stack_handle, root_node);
    current_linked_node = root_node;

    while ((const char *)sadface_data[4] < content_end_ptr) {
        int found_status;
        int placeholder_start_offset_from_base;
        char *placeholder_end_ptr; // Absolute pointer to end of placeholder (e.g., "-->" + strlen("-->"))

        found_status = _find_sadface(sadface_data, &placeholder_start_offset_from_base, &placeholder_end_ptr);

        if (found_status < 1) { // No more placeholders or error
            // If there's remaining plain text
            if ((const char *)sadface_data[4] < content_end_ptr) {
                sadface_node_t *text_node = (sadface_node_t *)malloc(sizeof(sadface_node_t));
                if (!text_node) {
                    goto cleanup_error;
                }
                memset(text_node, 0, sizeof(sadface_node_t));
                text_node->type = 1; // Text type
                text_node->data_ptr = (void *)sadface_data[4];
                text_node->data_len = (size_t)(content_end_ptr - (const char *)sadface_data[4]);
                current_linked_node->next = text_node;
                stack_push(stack_handle, text_node);
            }
            sadface_data[4] = (intptr_t)content_end_ptr; // Advance current_pos_str to end
            break; // Finished parsing
        }

        const char *placeholder_start_ptr = content_base_ptr + placeholder_start_offset_from_base;

        // Handle plain text before the found placeholder
        if ((const char *)sadface_data[4] < placeholder_start_ptr) {
            sadface_node_t *text_node = (sadface_node_t *)malloc(sizeof(sadface_node_t));
            if (!text_node) {
                goto cleanup_error;
            }
            memset(text_node, 0, sizeof(sadface_node_t));
            text_node->type = 1; // Text type
            text_node->data_ptr = (void *)sadface_data[4];
            text_node->data_len = (size_t)(placeholder_start_ptr - (const char *)sadface_data[4]);
            current_linked_node->next = text_node;
            stack_push(stack_handle, text_node);
            current_linked_node = text_node;
        }
        sadface_data[4] = (intptr_t)placeholder_start_ptr; // Advance current_pos_str to start of placeholder

        // Process the placeholder itself
        const char *start_tag_str = (const char *)sadface_data[0];
        const char *end_tag_str = (const char *)sadface_data[1];
        const char *inner_content_start = placeholder_start_ptr + strlen(start_tag_str);
        const char *inner_content_end = placeholder_end_ptr - strlen(end_tag_str);
        size_t inner_content_len = (size_t)(inner_content_end - inner_content_start);

        char placeholder_char_type = *inner_content_start;
        
        if (placeholder_char_type == '@') { // Conditional block
            sadface_node_t *block_node = (sadface_node_t *)malloc(sizeof(sadface_node_t));
            sadface_node_t *condition_node = (sadface_node_t *)malloc(sizeof(sadface_node_t));
            if (!block_node || !condition_node) {
                goto cleanup_error;
            }
            memset(block_node, 0, sizeof(sadface_node_t));
            block_node->type = 0; // Block node (acts as a root for its children)
            memset(condition_node, 0, sizeof(sadface_node_t));
            condition_node->type = 3; // Conditional type
            condition_node->data_ptr = (void *)(inner_content_start + 1); // Content after '@'
            condition_node->data_len = inner_content_len - 1; // Length of content after '@'
            condition_node->child_or_value = block_node; // Link block to condition

            current_linked_node->next = condition_node;
            stack_push(stack_handle, condition_node);
            stack_push(stack_handle, block_node); // Push block for its children
            current_linked_node = block_node; // Next items will be children of this block
            sadface_data[4] = (intptr_t)placeholder_end_ptr; // Advance current_pos_str past the placeholder
        } else if (placeholder_char_type == '#') { // Comment/Skip
            sadface_data[4] = (intptr_t)placeholder_end_ptr; // Advance current_pos_str past the placeholder
        } else if (placeholder_char_type == '/') { // End of block
            sadface_node_t *popped_block = (sadface_node_t *)stack_pop(stack_handle); // Pop the block node
            sadface_node_t *popped_condition = (sadface_node_t *)stack_pop(stack_handle); // Pop the conditional node

            if (!popped_block || !popped_condition || popped_condition->type != 3) {
                // Stack mismatch or invalid node type, indicates parsing error
                goto cleanup_error;
            }

            // Check if names match
            if (inner_content_len - 1 != popped_condition->data_len ||
                strncmp((const char*)popped_condition->data_ptr, inner_content_start + 1, popped_condition->data_len) != 0) {
                goto cleanup_error; // Mismatch in block name
            }
            current_linked_node = popped_condition; // Now linking to the conditional node's next
            sadface_data[4] = (intptr_t)placeholder_end_ptr; // Advance current_pos_str past the placeholder
        } else { // Variable replacement
            char *var_name_buffer = (char *)calloc(inner_content_len + 1, 1);
            if (!var_name_buffer) {
                goto cleanup_error;
            }
            strncpy(var_name_buffer, inner_content_start, inner_content_len);

            sadface_node_t *var_node = (sadface_node_t *)malloc(sizeof(sadface_node_t));
            if (!var_node) {
                free(var_name_buffer);
                goto cleanup_error;
            }
            memset(var_node, 0, sizeof(sadface_node_t));
            var_node->type = 2; // Variable type
            var_node->data_ptr = (void *)inner_content_start; // Pointer to variable name in original content
            var_node->data_len = inner_content_len;   // Length of variable name
            var_node->child_or_value = (void*)(intptr_t)dict_find(dict_handle, var_name_buffer); // Store dict lookup result

            free(var_name_buffer);

            current_linked_node->next = var_node;
            stack_push(stack_handle, var_node);
            current_linked_node = var_node;
            sadface_data[4] = (intptr_t)placeholder_end_ptr; // Advance current_pos_str past the placeholder
        }
    }

    stack_destroy(stack_handle);
    return root_node;

cleanup_error:
    if (stack_handle) stack_destroy(stack_handle);
    if (root_node) sadface_node_destroy_recursive(root_node); // Free the partially built tree
    sadface_destroy(sadface_data);
    return NULL;
}

// Function: sadface_render
// param_sadface_data: sadface_data structure (intptr_t*)
// output_buffer: buffer to write rendered output to
// output_len: receives the total number of bytes written
int sadface_render(intptr_t param_sadface_data, char *output_buffer, int *output_len) {
    intptr_t *sadface_data_config = (intptr_t *)param_sadface_data;
    if (!sadface_data_config || !output_buffer || !output_len) {
        return -1;
    }

    int current_bytes_written = 0;
    sadface_node_t *current_node = parse_sadface(sadface_data_config);
    if (!current_node) {
        return -1;
    }

    void *stack_handle = stack_new(MAX_STACK_SIZE);
    if (!stack_handle) {
        sadface_node_destroy_recursive(current_node); // Free the parsed tree
        return -1;
    }

    sadface_node_t *head_node_for_cleanup = current_node; // Keep reference to the head of the tree for freeing later

    // Traverse the parsed tree
    while (current_node) {
        unsigned int type = current_node->type;

        if (type == 3) { // Conditional block
            const char *var_name = (const char*)current_node->data_ptr;
            size_t var_name_len = current_node->data_len;

            char *name_buffer = (char *)calloc(var_name_len + 1, 1);
            if (!name_buffer) {
                goto render_cleanup_error;
            }
            strncpy(name_buffer, var_name, var_name_len);

            unsigned int *var_value_struct = (unsigned int *)(intptr_t)dict_find((void*)sadface_data_config[5], name_buffer);
            free(name_buffer);

            // Condition check: if var_value_struct is not null AND NOT (it's boolean type AND its value is false).
            if (var_value_struct && !(var_value_struct[0] == 3 && var_value_struct[1] == 0)) {
                stack_push(stack_handle, current_node); // Push conditional node
                current_node = (sadface_node_t *)current_node->child_or_value; // Descend into the block
            } else {
                current_node = current_node->next; // Skip the block, go to next sibling
            }
        } else if (type == 2) { // Variable replacement
            const unsigned int *var_value_struct = (const unsigned int *)current_node->child_or_value;
            if (var_value_struct) {
                char *var_string = sadface_var2str(var_value_struct);
                if (var_string) {
                    size_t str_len = strlen(var_string);
                    // Check buffer bounds (0x100 is sadface_var2str's internal buffer size)
                    if (current_bytes_written + str_len < 0x100) {
                        memcpy(output_buffer + current_bytes_written, var_string, str_len);
                        current_bytes_written += str_len;
                    }
                    if (var_value_struct[0] != 0) { // If type is not 0 (string literal returned directly)
                        free(var_string);
                    }
                }
            }
            current_node = current_node->next;
        } else if (type == 1) { // Plain text
            const char *text_ptr = (const char *)current_node->data_ptr;
            size_t text_len = current_node->data_len;
            if (current_bytes_written + text_len < 0x100) { // Check buffer bounds
                memcpy(output_buffer + current_bytes_written, text_ptr, text_len);
                current_bytes_written += text_len;
            }
            current_node = current_node->next;
        } else { // Type 0 (root or block node)
            // If it has children, go to the first child. Otherwise, go to its next sibling.
            if (current_node->child_or_value) {
                current_node = (sadface_node_t *)current_node->child_or_value;
            } else {
                current_node = current_node->next;
            }
        }

        // If current node has no next sibling, and stack is not empty, pop and continue from popped node's next sibling.
        if (!current_node && g_stack_top >= 0) {
            sadface_node_t *popped_node = (sadface_node_t *)stack_pop(stack_handle);
            current_node = popped_node->next; // Continue from the sibling after the block that was just completed
        }
    }

    *output_len = current_bytes_written;
    stack_destroy(stack_handle);
    sadface_node_destroy_recursive(head_node_for_cleanup);
    return 0;

render_cleanup_error:
    stack_destroy(stack_handle);
    sadface_node_destroy_recursive(head_node_for_cleanup);
    return -1;
}

// Function: sadface_destroy
// Frees the sadface_data structure and its internal content string.
void sadface_destroy(intptr_t *sadface_data) {
    if (sadface_data != NULL) {
        // sadface_data[2] is the char* content allocated by strdup
        if ((char*)sadface_data[2] != NULL) {
            free((void *)sadface_data[2]);
        }
        free(sadface_data);
    }
}

// Helper to recursively free sadface_node_t tree
void sadface_node_destroy_recursive(sadface_node_t *node) {
    if (!node) return;

    // Free the linked list first (depth-first for siblings)
    sadface_node_destroy_recursive(node->next);

    // If it's a parent node (type 0 or 3), free its children (depth-first for children)
    if (node->type == 0 || node->type == 3) {
        sadface_node_destroy_recursive((sadface_node_t *)node->child_or_value);
    }

    free(node);
}