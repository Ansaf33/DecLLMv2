#include <stdio.h>   // For puts, printf, perror
#include <stdlib.h>  // For malloc, free, exit, EXIT_FAILURE
#include <string.h>  // For memset (replacement for bzero)
#include <stddef.h>  // For size_t

// --- Mock / Helper Functions ---
// These functions are assumed to be external but are mocked here for compilability.

// Simulates a memory allocation function.
// It allocates memory and exits if allocation fails.
void* allocate(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Simulates a memory deallocation function.
// The 'size' parameter is ignored as free() does not require it.
void deallocate(void* ptr, size_t size) {
    (void)size; // Suppress unused parameter warning
    free(ptr);
}

// Simulates a program termination function.
void _terminate(void) {
    exit(EXIT_FAILURE);
}

// --- Structure Definitions ---
// These structures are inferred from the pointer arithmetic and sizes in the original code.
// Assuming a 64-bit environment where sizeof(void*) == 8 bytes.

// Single Linked List Node structure
// Node size: 0xff8 (4088 bytes)
// 'next' pointer is at offset 0xff0.
struct SingleNode {
    char data[0xff0]; // Placeholder for data payload (4080 bytes)
    struct SingleNode *next; // Next node pointer (8 bytes)
};

// Single Linked List Header structure
// List header size: 0xc (12 bytes)
// Contains a pointer to the head node and a count of nodes.
struct SingleList {
    struct SingleNode *head; // Pointer to the first node (8 bytes)
    int count;               // Number of nodes in the list (4 bytes)
};

// Double Linked List Node structure
// Node size: 0x8c (140 bytes)
// 'prev' pointer is at offset 0, 'next' pointer at offset 8 (for 64-bit).
struct DoubleNode {
    struct DoubleNode *prev; // Previous node pointer (8 bytes)
    struct DoubleNode *next; // Next node pointer (8 bytes)
    char data[0x8c - (sizeof(void*) * 2)]; // Placeholder for data payload (140 - 16 = 124 bytes)
};

// Double Linked List Header structure
// List header size: 0xc (12 bytes)
// Contains a pointer to the head node and a count of nodes.
struct DoubleList {
    struct DoubleNode *head; // Pointer to the first node (8 bytes)
    int count;               // Number of nodes in the list (4 bytes)
};

// --- Function Implementations ---

// Function: get_last_element_s
// Retrieves the last node in a single linked list.
// Returns a pointer to the last node, or NULL if the list is empty.
struct SingleNode *get_last_element_s(struct SingleList *list) {
    struct SingleNode *last_node = NULL;
    struct SingleNode *current = list->head;

    while (current != NULL) {
        last_node = current;
        current = current->next;
    }
    return last_node;
}

// Function: get_last_element_d
// Retrieves the last node in a double linked list.
// Returns a pointer to the last node, or NULL if the list is empty.
struct DoubleNode *get_last_element_d(struct DoubleList *list) {
    struct DoubleNode *last_node = NULL;
    struct DoubleNode *current = list->head;

    while (current != NULL) {
        last_node = current;
        current = current->next;
    }
    return last_node;
}

// Function: create_single_list
// Creates and initializes an empty single linked list header.
// Returns a pointer to the new list header.
struct SingleList *create_single_list(void) {
    struct SingleList *list = (struct SingleList *)allocate(sizeof(struct SingleList));
    memset(list, 0, sizeof(struct SingleList)); // Zero out the allocated memory
    // Explicitly initialize members (redundant if memset to 0, but good practice)
    list->head = NULL;
    list->count = 0;
    return list;
}

// Function: create_double_list
// Creates and initializes an empty double linked list header.
// Returns a pointer to the new list header.
struct DoubleList *create_double_list(void) {
    struct DoubleList *list = (struct DoubleList *)allocate(sizeof(struct DoubleList));
    // The original bzero size (0x8) was likely incorrect for a 12-byte struct.
    // Zeroing out the entire struct for proper initialization.
    memset(list, 0, sizeof(struct DoubleList));
    list->head = NULL;
    list->count = 0;
    return list;
}

// Function: insert_double_list_node
// Inserts a new node into a double linked list.
// If prev_node is NULL, the new node is inserted at the head of the list.
// Otherwise, it's inserted after prev_node.
// Returns a pointer to the newly inserted node.
struct DoubleNode *insert_double_list_node(struct DoubleList *list, struct DoubleNode *prev_node) {
    struct DoubleNode *new_node = (struct DoubleNode *)allocate(sizeof(struct DoubleNode));
    memset(new_node, 0, sizeof(struct DoubleNode)); // Initialize new node to zeros

    if (prev_node == NULL) { // Insert at the head
        new_node->next = list->head;
        list->head = new_node;
        // new_node->prev is already NULL from memset, which is correct for head.
    } else { // Insert after prev_node
        new_node->next = prev_node->next;
    }

    new_node->prev = prev_node; // Set new node's previous pointer

    // Update pointers of adjacent nodes
    if (new_node->next != NULL) {
        new_node->next->prev = new_node;
    }
    if (new_node->prev != NULL) {
        new_node->prev->next = new_node;
    }
    list->count++;
    return new_node;
}

// Function: delete_double_list_node
// Deletes a specified node from a double linked list.
// Returns the 'prev' pointer of the deleted node, or NULL if deletion fails.
struct DoubleNode *delete_double_list_node(struct DoubleList *list, struct DoubleNode *node_to_delete) {
    struct DoubleNode *return_prev_node = NULL;

    if (node_to_delete == NULL || list == NULL || list->count == 0) {
        puts("**Cannot delete from an empty list or a NULL node.");
        return NULL;
    }

    return_prev_node = node_to_delete->prev; // Capture value before modification

    if (node_to_delete == list->head) {
        list->head = node_to_delete->next;
    } else {
        node_to_delete->prev->next = node_to_delete->next;
    }

    if (node_to_delete->next != NULL) {
        node_to_delete->next->prev = node_to_delete->prev;
    }

    deallocate(node_to_delete, sizeof(struct DoubleNode));
    list->count--;

    return return_prev_node;
}

// Function: insert_single_list_node
// Inserts a new node into a single linked list.
// If prev_node is NULL, the new node is inserted at the head of the list.
// Otherwise, it's inserted after prev_node.
// Returns a pointer to the newly inserted node.
struct SingleNode *insert_single_list_node(struct SingleList *list, struct SingleNode *prev_node) {
    struct SingleNode *new_node = (struct SingleNode *)allocate(sizeof(struct SingleNode));
    memset(new_node, 0, sizeof(struct SingleNode)); // Initialize new node to zeros

    if (prev_node == NULL) { // Insert at the head
        new_node->next = list->head;
        list->head = new_node;
    } else { // Insert after prev_node
        new_node->next = prev_node->next;
        prev_node->next = new_node;
    }
    list->count++;
    return new_node;
}

// --- Main Function for Testing ---
int main() {
    printf("--- Single Linked List Test ---\n");
    struct SingleList *s_list = create_single_list();
    printf("Initial single list count: %d\n", s_list->count);

    // Insert node1 at head
    struct SingleNode *s_node1 = insert_single_list_node(s_list, NULL);
    s_node1->data[0] = 'A';
    printf("Inserted node1. Count: %d, Head data: %c\n", s_list->count, s_list->head->data[0]);

    // Insert node2 after node1
    struct SingleNode *s_node2 = insert_single_list_node(s_list, s_node1);
    s_node2->data[0] = 'B';
    printf("Inserted node2 after node1. Count: %d, Head data: %c, Head->next data: %c\n", s_list->count, s_list->head->data[0], s_list->head->next->data[0]);

    // Insert node3 at head
    struct SingleNode *s_node3 = insert_single_list_node(s_list, NULL);
    s_node3->data[0] = 'C';
    printf("Inserted node3 at head. Count: %d, Head data: %c, Head->next data: %c\n", s_list->count, s_list->head->data[0], s_list->head->next->data[0]);

    struct SingleNode *last_s_node = get_last_element_s(s_list);
    printf("Last single list element data: %c\n", last_s_node ? last_s_node->data[0] : ' ');

    // Free single list
    struct SingleNode *current_s = s_list->head;
    while (current_s != NULL) {
        struct SingleNode *next_s = current_s->next;
        deallocate(current_s, sizeof(struct SingleNode));
        current_s = next_s;
    }
    deallocate(s_list, sizeof(struct SingleList));
    printf("Single list freed.\n\n");

    printf("--- Double Linked List Test ---\n");
    struct DoubleList *d_list = create_double_list();
    printf("Initial double list count: %d\n", d_list->count);

    // Insert node1 at head
    struct DoubleNode *d_node1 = insert_double_list_node(d_list, NULL);
    d_node1->data[0] = 'X';
    printf("Inserted node1. Count: %d, Head data: %c\n", d_list->count, d_list->head->data[0]);

    // Insert node2 after node1
    struct DoubleNode *d_node2 = insert_double_list_node(d_list, d_node1);
    d_node2->data[0] = 'Y';
    printf("Inserted node2 after node1. Count: %d, Head data: %c, Head->next data: %c\n", d_list->count, d_list->head->data[0], d_list->head->next->data[0]);
    printf("Node2 prev data: %c, Node2 next: %s\n", d_node2->prev->data[0], d_node2->next ? "NOT NULL" : "NULL");

    // Insert node3 at head
    struct DoubleNode *d_node3 = insert_double_list_node(d_list, NULL);
    d_node3->data[0] = 'Z';
    printf("Inserted node3 at head. Count: %d, Head data: %c, Head->next data: %c\n", d_list->count, d_list->head->data[0], d_list->head->next->data[0]);
    printf("Node3 prev: %s, Node3 next data: %c\n", d_node3->prev ? "NOT NULL" : "NULL", d_node3->next->data[0]);

    struct DoubleNode *last_d_node = get_last_element_d(d_list);
    printf("Last double list element data: %c\n", last_d_node ? last_d_node->data[0] : ' ');

    printf("Deleting node2 (data 'Y')...\n");
    struct DoubleNode *deleted_prev_d_node = delete_double_list_node(d_list, d_node2);
    printf("Deleted node2. Count: %d. Returned prev node data: %c\n", d_list->count, deleted_prev_d_node ? deleted_prev_d_node->data[0] : ' ');
    if (d_list->head != NULL) {
        printf("New head data: %c\n", d_list->head->data[0]);
        if (d_list->head->next != NULL) {
            printf("New head->next data: %c\n", d_list->head->next->data[0]);
        }
    }

    // Free double list
    struct DoubleNode *current_d = d_list->head;
    while (current_d != NULL) {
        struct DoubleNode *next_d = current_d->next;
        deallocate(current_d, sizeof(struct DoubleNode));
        current_d = next_d;
    }
    deallocate(d_list, sizeof(struct DoubleList));
    printf("Double list freed.\n");

    return 0;
}