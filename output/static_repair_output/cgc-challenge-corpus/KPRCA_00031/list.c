#include <stdio.h>    // For fprintf, stderr
#include <stdlib.h>   // For calloc, free, exit, random
#include <stdint.h>   // For fixed-size integer types like uint32_t (though not explicitly used for Node fields after refactoring)
#include <string.h>   // For strcmp
// #include <stddef.h>   // For offsetof (not directly used after struct refactoring)

// Simple _error function to replace the one from the challenge environment
void _error(int exit_code, const char *file, int line) {
    fprintf(stderr, "ERROR: %s:%d - Exiting with code %d\n", file, line, exit_code);
    exit(exit_code);
}

// Define the list node structure for 64-bit Linux.
// The `data` field is `char*` as implied by `strcmp` and `append_list`'s `param_2`.
// For lists of lists, this implies `Node*` values are cast to `char*` for storage and back.
// This is a common pattern in C for generic lists without explicit type tags, though it requires careful handling.
typedef struct Node {
    struct Node *next;
    struct Node *prev;
    char *data; // Stores string data, or for list-of-lists, a Node* cast to char*.
} Node;

// Function: init_list
// param_1 (original `undefined4`) is `char*` based on `append_list` usage.
Node *init_list(char *data_str) {
    // Original `calloc(1, 0xc)` implies 12 bytes. On a 64-bit system, `sizeof(Node)` would be 24 bytes (3 * 8 bytes).
    // Using `sizeof(Node)` ensures correct allocation for 64-bit architecture, which is standard for Linux.
    Node *newNode = (Node *)calloc(1, sizeof(Node));
    if (newNode == NULL) {
        _error(1, __FILE__, __LINE__);
    }
    newNode->data = data_str;
    // `next` and `prev` are already NULL due to `calloc` initializing memory to zero.
    return newNode;
}

// Forward declaration for `len_list` as it's used before its definition
int len_list(const Node *head);

// Function: append_list
void append_list(Node **head_ptr, char *data_str, int unique_check) {
    if (*head_ptr == NULL) {
        *head_ptr = init_list(data_str);
    } else {
        Node *current = *head_ptr;
        if (unique_check == 0) {
            for (; current != NULL; current = current->next) {
                if (strcmp(data_str, current->data) == 0) {
                    return; // Element already exists, do nothing
                }
            }
        }

        Node *newNode = init_list(data_str);
        // Find the last node
        current = *head_ptr; // Reset current to head if unique_check was 0 and no match found
        for (; current->next != NULL; current = current->next) {
            // Loop until current is the last node
        }
        current->next = newNode;
        newNode->prev = current;
    }
}

// Function: concat_list
// Appends elements of `list2_head` to `list1_head` by copying, but with specific handling for `list1_head == NULL`.
// Returns the head of the resulting list.
Node *concat_list(Node *list1_head, Node *list2_head) {
    Node *result_head = list1_head;

    if (list1_head == NULL) {
        // If `list1_head` is initially NULL, the original code returned `list2_head` directly,
        // implying no copying of `list2_head` elements in this case.
        result_head = list2_head;
    } else {
        // If `list1_head` exists, append elements of `list2_head` to it by copying.
        if (list2_head != NULL) {
            for (Node *current_list2 = list2_head; current_list2 != NULL; current_list2 = current_list2->next) {
                // `append_list` will append to `result_head` (which is `list1_head` here).
                append_list(&result_head, current_list2->data, 1); // `1` means no unique check
            }
        }
    }
    return result_head;
}

// Function: len_list
// Calculates the number of elements in the list.
int len_list(const Node *head) {
    int count = 0;
    for (const Node *current = head; current != NULL; current = current->next) {
        count++;
    }
    return count;
}

// Function: lindex
// Retrieves the data of the element at the specified index. Supports negative indexing from the end.
char *lindex(const Node *head, int index) {
    if (head == NULL) {
        return NULL;
    }

    int len = len_list(head);

    if (index < 0) {
        index = len + index; // Adjust negative index (e.g., -1 is len-1)
    }

    if (index < 0 || index >= len) { // Check if index is still out of bounds after adjustment
        return NULL;
    }

    const Node *current = head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return current->data;
}

// Function: copy_list
// Creates a new list containing a sub-section of the original list.
Node *copy_list(const Node *head, unsigned int start_idx, unsigned int end_idx) {
    if (head == NULL) {
        return NULL;
    }

    unsigned int list_len = len_list(head);
    Node *new_list_head = NULL;

    // Adjust `end_idx`: if 0 or beyond list length, set to list length.
    if (end_idx == 0 || list_len < end_idx) {
        end_idx = list_len;
    }

    // Ensure `start_idx` is not beyond `end_idx`.
    if (start_idx >= end_idx) {
        return NULL; // Empty range, return NULL
    }

    // Traverse to the starting position.
    const Node *current = head;
    for (unsigned int i = 0; i < start_idx; i++) {
        if (current == NULL) return NULL; // Should not happen if `start_idx` is within bounds
        current = current->next;
    }

    // Copy elements from `start_idx` to `end_idx`.
    for (unsigned int i = start_idx; i < end_idx; i++) {
        if (current == NULL) break; // Reached end of original list prematurely
        append_list(&new_list_head, current->data, 1); // `1` means no unique check
        current = current->next;
    }
    return new_list_head;
}

// Function: free_list
// Frees all nodes in a list. If `free_data_flag` is 0, it also frees the `data` (assumed to be `char*`).
void free_list(Node *head, int free_data_flag) {
    Node *current = head;
    Node *next_node;

    while (current != NULL) {
        next_node = current->next; // Store next before freeing current
        // `free_data_flag == 0` means free the data (string).
        if (free_data_flag == 0 && current->data != NULL) {
            free(current->data);
        }
        free(current); // Free the node itself
        current = next_node;
    }
    // Original code had `free((void *)0x0);`, which is a no-op and unnecessary. Removed.
}

// Function: free_list_of_lists
// Frees a list where each node's `data` field is itself a pointer to another list.
void free_list_of_lists(Node *head_of_list_of_lists, int depth) {
    Node *current = head_of_list_of_lists;
    Node *next_node;

    while (current != NULL) {
        next_node = current->next; // Save next node before freeing current

        if (depth > 0 && current->data != NULL) {
            // Cast `current->data` to `Node*` because it's the head of a sublist.
            // Recursively free the sublist and its data.
            // `depth - 1` determines if the string data in the sublist should be freed.
            free_list((Node *)current->data, depth - 1);
        }
        // Original code had `if (local_10[1] != 0) { free((void *)local_10[1]); }`, which would attempt
        // to free the `prev` pointer (a pointer to another node), leading to errors. This is removed.
        free(current); // Free the node itself
        current = next_node;
    }
    // Original code had `free((void *)0x0);`, which is a no-op and unnecessary. Removed.
}

// Function: random_element
// Returns the data of a randomly selected element from the list.
char *random_element(const Node *head) {
    if (head == NULL) {
        return NULL;
    }

    int len = len_list(head);
    if (len == 0) {
        return NULL;
    }

    long rand_val = random();
    // POSIX `random()` returns a non-negative value. The original check `lVar1 < 0`
    // is likely dead code or for a specific environment where `random` might return negative.
    if (rand_val < 0) {
        _error(5, __FILE__, __LINE__);
    }

    unsigned int random_index = (unsigned int)(rand_val % len);

    return lindex(head, random_index);
}