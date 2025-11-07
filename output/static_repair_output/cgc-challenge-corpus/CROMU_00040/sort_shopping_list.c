#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free, exit, perror
#include <stdbool.h> // For bool
#include <string.h>  // For strcmp

// Define the Node structure based on the observed access patterns.
// The original snippet's `undefined4 *param_1` and `local_10[1]`
// implies that `item` and `next` are contiguous memory locations
// and are both of the size of `undefined4` (typically 4 bytes on a 32-bit system
// where `undefined4` is a pointer type, or 8 bytes on a 64-bit system).
// We use `char *` for `item` and `struct Node *` for `next`, assuming they are
// pointer types and thus have consistent sizing for the pointer arithmetic to work.
typedef struct Node {
    char *item;
    struct Node *next;
} Node;

// Helper function to compare strings.
// Returns 1 if str1 is lexicographically greater than str2, 0 otherwise.
// This matches the `if (iVar3 == 1)` condition for triggering a swap in the original snippet.
int compare_strings(char *str1, char *str2) {
    return strcmp(str1, str2) > 0;
}

// Function: sort_shopping_list
// This function sorts a singly linked list using a bubble sort algorithm.
// `head_item_ptr` is a pointer to the 'item' field of the head node.
// This allows the function to access both the 'item' and 'next' fields
// of a node using pointer arithmetic (`current_item_ptr[0]` and `current_item_ptr[1]`).
void sort_shopping_list(char **head_item_ptr) {
    bool swapped;
    char **current_item_ptr; // Corresponds to `local_10` in the original snippet
    Node *next_node_ptr;     // Temporary for checking the next node

    // Handle empty list or invalid head pointer.
    // If head_item_ptr is NULL, it's an empty list or an invalid call.
    if (head_item_ptr == NULL) {
        return;
    }

    do {
        swapped = false;
        current_item_ptr = head_item_ptr; // Start from the beginning of the list for each pass

        // The loop condition `local_10[1] != 0` checks if the 'next' pointer is non-null.
        // `current_item_ptr[1]` (which is `*(current_item_ptr + 1)`) is the `next` pointer of the current node.
        // We cast it to `Node *` to check if it's NULL, indicating the end of the list segment.
        while ((next_node_ptr = (Node *)(current_item_ptr[1])) != NULL) {
            // Compare `current_node->item` with `current_node->next->item`.
            // `*current_item_ptr` is `current_node->item`.
            // `*(char **)(current_item_ptr[1])` effectively dereferences the `next` pointer
            // (which is `current_item_ptr[1]`) and then treats the memory it points to
            // as a `char **` to get `next_node->item`.
            if (compare_strings(*current_item_ptr, *(char **)(current_item_ptr[1])) == 1) {
                // Swap the 'item' string pointers.
                // This is equivalent to `uVar1 = *local_10; ...` in the original snippet.
                char *temp_item = *current_item_ptr;
                *current_item_ptr = *(char **)(current_item_ptr[1]);
                *(char **)(current_item_ptr[1]) = temp_item;
                swapped = true;
            }
            // Move to the next node.
            // `local_10 = (undefined4 *)local_10[1];` means
            // `current_item_ptr = (char **)(current_item_ptr[1]);`
            // `current_item_ptr[1]` is the `Node *next` pointer. Casting it to `char **`
            // makes `current_item_ptr` point to the `item` field of the next node,
            // preparing for the next iteration.
            current_item_ptr = (char **)(current_item_ptr[1]);
        }
    } while (swapped);
}

// --- Main function for demonstration and compilation ---

// Helper function to print the list
void print_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        printf("%s -> ", current->item);
        current = current->next;
    }
    printf("NULL\n");
}

// Helper function to create a new node
Node *create_node(char *item) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    new_node->item = item; // Assuming item strings are static or managed elsewhere
    new_node->next = NULL;
    return new_node;
}

// Helper function to free the list memory
void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
}

int main() {
    // Create a sample shopping list
    Node *head = create_node("Milk");
    head->next = create_node("Bread");
    head->next->next = create_node("Apples");
    head->next->next->next = create_node("Butter");
    head->next->next->next->next = create_node("Cheese");
    head->next->next->next->next->next = create_node("Yogurt");

    printf("Original Shopping List:\n");
    print_list(head);

    // Call sort_shopping_list with a pointer to the 'item' field of the head node.
    // This matches the `undefined4 *param_1` interpretation.
    sort_shopping_list(&(head->item));

    printf("\nSorted Shopping List:\n");
    print_list(head);

    // Test with an already sorted list
    Node *head2 = create_node("Apple");
    head2->next = create_node("Banana");
    head2->next->next = create_node("Cherry");
    printf("\nOriginal (sorted) List 2:\n");
    print_list(head2);
    sort_shopping_list(&(head2->item));
    printf("Sorted List 2:\n");
    print_list(head2);

    // Test with an empty list
    Node *head3 = NULL;
    printf("\nOriginal (empty) List 3:\n");
    print_list(head3);
    // Ensure not to take address of NULL for &(head3->item)
    if (head3 != NULL) {
        sort_shopping_list(&(head3->item));
    }
    printf("Sorted List 3:\n");
    print_list(head3); // Should remain NULL

    // Test with a single-node list
    Node *head4 = create_node("Zebra");
    printf("\nOriginal (single) List 4:\n");
    print_list(head4);
    sort_shopping_list(&(head4->item));
    printf("Sorted List 4:\n");
    print_list(head4);

    // Clean up allocated memory
    free_list(head);
    free_list(head2);
    free_list(head4);

    return 0;
}