#include <stdio.h>  // For printf
#include <stdlib.h> // For malloc, free
#include <string.h> // For strlen, memcpy

// Generic Node structure capable of storing any dynamically allocated data
typedef struct Node {
    void *data;
    struct Node *next;
} Node;

// Function: peek_front
// Returns the data pointer of the front element, or NULL if the queue is empty.
void *peek_front(Node *head) {
    return head ? head->data : NULL;
}

// Function: enqueue
// Enqueues an integer value. Memory is allocated for the integer itself.
// Returns 0 on success, -1 on failure.
int enqueue(Node **head_ptr, int value) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        return -1; // Malloc failed for node
    }

    int *data_value = (int *)malloc(sizeof(int));
    if (!data_value) {
        free(newNode);
        return -1; // Malloc failed for data
    }
    *data_value = value;

    newNode->data = data_value;
    newNode->next = NULL;

    if (!*head_ptr) {
        *head_ptr = newNode;
    } else {
        Node *current = *head_ptr;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
    return 0;
}

// Function: enqueue_copy
// Enqueues a copy of a string.
// Returns 0 on success, -1 on failure.
int enqueue_copy(Node **head_ptr, const char *str, unsigned int max_size) {
    if (!str) {
        return -1;
    }
    size_t str_len = strlen(str);
    unsigned int required_size = str_len + 1;

    if (max_size < required_size) {
        return -1; // Supplied max_size is too small for string + null terminator
    }

    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        return -1;
    }

    char *copied_str = (char *)malloc(required_size);
    if (!copied_str) {
        free(newNode);
        return -1;
    }
    memcpy(copied_str, str, required_size);
    newNode->data = copied_str;
    newNode->next = NULL;

    if (!*head_ptr) {
        *head_ptr = newNode;
    } else {
        Node *current = *head_ptr;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
    return 0;
}

// Function: dequeue_copy
// Dequeues the front element, frees the node, and returns the data pointer.
// The caller is responsible for freeing the returned data if it was dynamically allocated
// by functions like enqueue or enqueue_copy.
// Returns the data pointer, or NULL if the queue is empty.
void *dequeue_copy(Node **head_ptr) {
    if (!*head_ptr) {
        return NULL; // Queue is empty
    }
    Node *temp = *head_ptr;
    void *data_to_return = temp->data;
    *head_ptr = temp->next;
    free(temp); // Free the node itself
    return data_to_return;
}

// Function: clear_queue
// Frees all nodes and their dynamically allocated data in the queue.
void clear_queue(Node **head_ptr) {
    Node *current = *head_ptr;
    while (current) {
        Node *next_node = current->next;
        free(current->data); // Free the data (e.g., copied string or int)
        free(current);       // Free the node itself
        current = next_node;
    }
    *head_ptr = NULL; // Mark queue as empty
}

// Main function to demonstrate usage
int main() {
    Node *int_queue_head = NULL;
    Node *string_queue_head = NULL;

    printf("--- Integer Queue Demo ---\n");
    enqueue(&int_queue_head, 10);
    enqueue(&int_queue_head, 20);
    enqueue(&int_queue_head, 30);

    int *peeked_int = (int *)peek_front(int_queue_head);
    if (peeked_int) {
        printf("Peeked int: %d\n", *peeked_int);
    }

    int *dequeued_int;
    while ((dequeued_int = (int *)dequeue_copy(&int_queue_head))) {
        printf("Dequeued int: %d\n", *dequeued_int);
        free(dequeued_int); // Free the dynamically allocated int
    }
    printf("Int queue cleared. Head: %p\n", (void*)int_queue_head);

    printf("\n--- String Queue Demo ---\n");
    enqueue_copy(&string_queue_head, "Hello", 10);
    enqueue_copy(&string_queue_head, "World", 10);
    enqueue_copy(&string_queue_head, "C Programming", 20);

    char *peeked_str = (char *)peek_front(string_queue_head);
    if (peeked_str) {
        printf("Peeked string: %s\n", peeked_str);
    }

    char *dequeued_str;
    while ((dequeued_str = (char *)dequeue_copy(&string_queue_head))) {
        printf("Dequeued string: %s\n", dequeued_str);
        free(dequeued_str); // Free the dynamically allocated string
    }
    printf("String queue cleared. Head: %p\n", (void*)string_queue_head);

    // Test clear_queue on a non-empty queue
    printf("\n--- Clear Queue Demo ---\n");
    enqueue_copy(&string_queue_head, "One", 10);
    enqueue_copy(&string_queue_head, "Two", 10);
    printf("String queue before clear: %s, %s\n", 
           (char*)peek_front(string_queue_head), 
           (char*)(((Node*)string_queue_head->next)->data));
    clear_queue(&string_queue_head);
    printf("String queue after clear. Head: %p\n", (void*)string_queue_head);

    return 0;
}