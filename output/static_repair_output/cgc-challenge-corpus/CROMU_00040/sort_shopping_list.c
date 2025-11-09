#include <stdbool.h> // For bool
#include <string.h>  // For strcmp, strdup
#include <stdio.h>   // For printf, perror
#include <stdlib.h>  // For malloc, free, exit

// Function to compare two strings
// Returns 1 if s1 > s2, -1 if s1 < s2, 0 if s1 == s2
int compare_strings(const char *s1, const char *s2) {
    int cmp = strcmp(s1, s2);
    if (cmp > 0) {
        return 1;
    } else if (cmp < 0) {
        return -1;
    }
    return 0;
}

// Function: sort_shopping_list
// param_1: A pointer to a pointer to a char (char**).
// In the context of the original snippet's `undefined4 *param_1`,
// this assumes `undefined4` is `char*` (a pointer type) and `undefined4 *` is `char**`.
//
// The list structure implied by the original snippet's pointer arithmetic is:
// Each conceptual "node" in the list is a contiguous block of two `char*` pointers.
//   - The first `char*` pointer (at index 0) holds the actual string data.
//   - The second `char*` pointer (at index 1) holds the *address* of the first `char*`
//     of the *next* conceptual node block in the list.
// The list terminates when the second `char*` pointer (the "next" pointer) is NULL.
//
// `param_1` points to the `char*` (string data) of the first item.
void sort_shopping_list(char **param_1) {
    char* temp_item; // Variable corresponding to uVar1 in the original snippet
    bool swapped;     // Variable corresponding to bVar2 in the original snippet
    char **current_item_ptr; // Variable corresponding to local_10 in the original snippet

    if (param_1 == NULL || *param_1 == NULL) { // Handle empty list case
        return;
    }

    do {
        swapped = false;
        // Loop through the list to perform bubble sort passes.
        // `current_item_ptr` points to the `char*` (string) of the current item.
        // `current_item_ptr[1]` is the `char*` that points to the *next conceptual node block*.
        // The loop continues as long as `current_item_ptr[1]` is not NULL, meaning there's a next node.
        for (current_item_ptr = param_1; current_item_ptr[1] != NULL; current_item_ptr = (char**)current_item_ptr[1]) {
            // Compare the current item with the next item.
            // `*current_item_ptr` is the current string.
            // `*(char**)current_item_ptr[1]` is the string of the next item.
            // The cast `(char**)` is necessary because `current_item_ptr[1]` is a `char*`
            // that *points to* another `char*` (the next item's string pointer),
            // so it needs to be dereferenced as a `char**`.
            if (compare_strings(*current_item_ptr, *(char**)current_item_ptr[1]) == 1) {
                // If the current item is "greater" than the next item, swap their string pointers.
                temp_item = *current_item_ptr;
                *current_item_ptr = *(char**)current_item_ptr[1];
                *(char**)current_item_ptr[1] = temp_item;
                swapped = true; // A swap occurred, so another pass is needed
            }
        }
    } while (swapped); // Continue sorting as long as swaps are being made
}

// Helper function to create a "node" for this specific list structure.
// It allocates a block of memory sufficient for two `char*` pointers:
// one for the item string itself, and one for the "next node" pointer.
// Returns a `char**` which points to the first `char*` (the item string)
// within the allocated block.
char** createNode(const char* item_string) {
    // Allocate space for two `char*` pointers.
    // Index 0 will store the actual string pointer.
    // Index 1 will store the pointer to the next node's item.
    char** node_block = (char**)malloc(2 * sizeof(char*));
    if (node_block == NULL) {
        perror("Failed to allocate node block");
        exit(EXIT_FAILURE);
    }
    node_block[0] = strdup(item_string); // Duplicate the string to manage its memory
    if (node_block[0] == NULL) {
        perror("Failed to duplicate string");
        free(node_block);
        exit(EXIT_FAILURE);
    }
    node_block[1] = NULL; // Initialize the "next" pointer to NULL
    return node_block; // Return pointer to the block (which is also the address of node_block[0])
}

// Helper function to print the list
void printList(char** head_item_ptr) {
    char** current = head_item_ptr;
    while (current != NULL && *current != NULL) {
        printf("%s -> ", *current);
        // Move to the next conceptual node block using the stored "next" pointer.
        current = (char**)current[1];
    }
    printf("NULL\n");
}

// Helper function to free the list's memory
void freeList(char** head_item_ptr) {
    char** current = head_item_ptr;
    while (current != NULL && *current != NULL) {
        char** next = (char**)current[1]; // Get the pointer to the next node block
        free(current[0]); // Free the duplicated string
        free(current);    // Free the node block itself
        current = next;
    }
}

// Main function for testing the sort_shopping_list
int main() {
    // Create a sample shopping list using the specific memory structure.
    // Each `nodeX` is a `char**` that points to a block of `char*`s.
    char** node1 = createNode("Milk");
    char** node2 = createNode("Apples");
    char** node3 = createNode("Bread");
    char** node4 = createNode("Eggs");
    char** node5 = createNode("Yogurt");
    char** node6 = createNode("Cheese");

    // Link the nodes: node_block[1] stores the address of the *next* node_block.
    node1[1] = (char*)node2;
    node2[1] = (char*)node3;
    node3[1] = (char*)node4;
    node4[1] = (char*)node5;
    node5[1] = (char*)node6;
    node6[1] = NULL; // The last node's "next" pointer is NULL

    char** head = node1; // The head of the list is the pointer to the first node block.

    printf("Original list: ");
    printList(head);

    sort_shopping_list(head);

    printf("Sorted list:   ");
    printList(head);

    // Test with an already sorted list
    char** head2_node1 = createNode("Apples");
    char** head2_node2 = createNode("Bread");
    char** head2_node3 = createNode("Milk");
    head2_node1[1] = (char*)head2_node2;
    head2_node2[1] = (char*)head2_node3;
    head2_node3[1] = NULL;
    char** head2 = head2_node1;

    printf("\nOriginal (sorted) list: ");
    printList(head2);
    sort_shopping_list(head2);
    printf("Sorted list:            ");
    printList(head2);

    // Test with an empty list
    char** empty_head = NULL;
    printf("\nOriginal (empty) list: ");
    printList(empty_head);
    sort_shopping_list(empty_head);
    printf("Sorted list:           ");
    printList(empty_head);

    // Test with a single-node list
    char** single_head_node = createNode("Zucchini");
    single_head_node[1] = NULL;
    char** single_head = single_head_node;

    printf("\nOriginal (single) list: ");
    printList(single_head);
    sort_shopping_list(single_head);
    printf("Sorted list:            ");
    printList(single_head);

    // Clean up allocated memory
    freeList(head);
    freeList(head2);
    freeList(single_head);

    return 0;
}