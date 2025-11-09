#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For malloc, free, exit
#include <stdint.h>  // For uintptr_t, intptr_t

// Defines the structure for a node in the shopping list.
// The original code used malloc(8), which suggests a 32-bit environment
// where a pointer and an int are both 4 bytes.
// Using sizeof(ShoppingItemNode) makes the code portable to 64-bit systems,
// where sizeof(void*) would typically be 8 bytes, making the node 16 bytes.
typedef struct ShoppingItemNode {
    void *item_data_ptr; // Pointer to the actual item data (expected to be a string based on printf usage)
    struct ShoppingItemNode *next; // Pointer to the next node in the list
} ShoppingItemNode;

// Dummy declaration for sort_shopping_list, as its implementation is not provided.
void sort_shopping_list(ShoppingItemNode *list_head) {
    // Placeholder for sorting logic.
    // In a real application, this would sort the linked list.
    (void)list_head; // Suppress unused parameter warning
}

// Function: build_shopping_list
// param_1 is expected to be a pointer to a structure that contains:
// - a flag at offset +200
// - a pointer to a sub-list head at offset +0xcc
// - a pointer to the next such structure at offset +0xd4
ShoppingItemNode *build_shopping_list(void *param_1) {
    ShoppingItemNode *head = NULL;
    ShoppingItemNode *prev_node = NULL; // Tracks the previous node to link new nodes

    // Use a char pointer to safely perform byte-offset arithmetic on param_1
    char *current_param_struct = (char *)param_1;

    while (current_param_struct != NULL) {
        // Check the flag at offset 200. *(int *)(current_param_struct + 200) == 1
        if (*(int *)(current_param_struct + 200) == 1) {
            // Get the head of a sub-list. This value is an `int` that represents a pointer.
            // Cast to uintptr_t to handle pointer values robustly across 32-bit/64-bit.
            uintptr_t sub_item_ptr_val = (uintptr_t)*(int *)(current_param_struct + 0xcc);

            while (sub_item_ptr_val != 0) {
                // Allocate a new node for the shopping list
                ShoppingItemNode *new_node = (ShoppingItemNode *)malloc(sizeof(ShoppingItemNode));
                if (new_node == NULL) {
                    fprintf(stderr, "unable to malloc memory\n");
                    exit(EXIT_FAILURE); // Terminate on memory allocation failure
                }

                // Set the item data pointer for the new node.
                // It's `0x14` bytes past the start of the `sub_item_ptr_val`.
                new_node->item_data_ptr = (void *)(sub_item_ptr_val + 0x14);
                new_node->next = NULL; // Initialize next pointer

                // Link the new node into the shopping list
                if (head == NULL) {
                    head = new_node; // First node becomes the head
                } else {
                    prev_node->next = new_node; // Link previous node to the new one
                }
                prev_node = new_node; // Update prev_node to the newly added node

                // Move to the next item in the sub-list.
                // This value is also an `int` representing a pointer.
                sub_item_ptr_val = (uintptr_t)*(int *)(sub_item_ptr_val + 0x78);
            }
        }
        // Move to the next `param_1` type structure in the main chain.
        // This pointer is at offset 0xd4. It's an `int` representing a pointer.
        current_param_struct = (char *)(uintptr_t)*(int *)(current_param_struct + 0xd4);
    }

    return head;
}

// Function: print_shopping_list
// param_1 is passed to build_shopping_list, so it should be a void pointer.
void print_shopping_list(void *param_1) {
    ShoppingItemNode *list_head = build_shopping_list(param_1);
    ShoppingItemNode *current_node;

    printf("\n");
    printf("Shopping List\n");
    printf("-------------\n");

    if (list_head != NULL) {
        sort_shopping_list(list_head); // Sort the list (dummy function provided)

        // Iterate and print each item in the sorted list
        for (current_node = list_head; current_node != NULL; current_node = current_node->next) {
            // The original printf format "@s" implies printing a string.
            // item_data_ptr holds a void* which points to the string.
            printf("%s\n", (char *)current_node->item_data_ptr);
        }

        // Free all nodes in the list to prevent memory leaks
        current_node = list_head;
        while (current_node != NULL) {
            ShoppingItemNode *next_node_to_free = current_node->next;
            free(current_node);
            current_node = next_node_to_free;
        }
    }
    printf("\n");
}