#include <stdlib.h> // Required for malloc and free

// Define the function pointer type for comparison (used by sl_insert)
// It compares two integer values and returns:
//   < 0 if the first argument is less than the second
//   = 0 if the first argument is equal to the second
//   > 0 if the first argument is greater than the second
typedef int (*compare_func)(int, int);

// Define the function pointer type for data destruction (used by sl_destroy)
// It takes an integer value and performs any necessary cleanup.
typedef void (*destroy_data_func)(int);

// Define the node structure for the singly linked list
typedef struct Node {
    int value;
    struct Node *next;
} Node;

// Function: sl_insert
// Inserts a new node with 'value' into a sorted singly linked list 'head'.
// 'cmp_func' is used to determine the correct insertion order.
// Returns the head of the list (which might be the newly inserted node if it's placed at the beginning).
Node *sl_insert(Node *head, int value, compare_func cmp_func) {
    // As per the original code's `if (param_2 != 0)` check,
    // if 'value' is 0, the function returns without insertion.
    if (value == 0) {
        return head;
    }

    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        // Handle allocation failure: return the original head.
        return head;
    }
    newNode->value = value;
    newNode->next = NULL; // Initialize the next pointer

    // Case 1: The list is empty, or the new node should be inserted at the beginning.
    // The original logic's `cmp_func(*local_10, *piVar1) < 1` (i.e., `<= 0`) implies
    // insertion after elements that are less than or equal to the new value.
    // So, if `newNode->value` is strictly less than `head->value`, it comes before head.
    if (head == NULL || cmp_func(head->value, newNode->value) > 0) {
        newNode->next = head;
        return newNode; // The new node is now the head of the list
    }

    // Case 2: Traverse the list to find the correct insertion point.
    // 'current' will stop at the node AFTER which 'newNode' should be inserted.
    // We continue as long as `current->next` exists and its value is less than or equal to `newNode->value`.
    Node *current = head;
    while (current->next != NULL && cmp_func(current->next->value, newNode->value) <= 0) {
        current = current->next;
    }

    // Insert newNode after 'current'.
    newNode->next = current->next;
    current->next = newNode;

    return head; // The head of the list remains unchanged
}

// Function: sl_destroy
// Frees all nodes in the singly linked list starting from 'head'.
// 'destroy_func' is an optional function to perform cleanup on the data stored in each node.
void sl_destroy(Node *head, destroy_data_func destroy_func) {
    Node *current = head;
    while (current != NULL) {
        Node *next_node = current->next; // Store the next pointer before freeing the current node
        
        // As per the original code's `if (*local_10 != 0)` check,
        // if the node's value is 0, 'destroy_func' is not called for it.
        // Also, check if 'destroy_func' itself is not NULL before calling it.
        if (current->value != 0 && destroy_func != NULL) {
            destroy_func(current->value);
        }
        free(current); // Free the current node's memory
        current = next_node; // Move to the next node in the list
    }
}