#include <stdio.h>  // For printf
#include <stdlib.h> // For malloc, free
#include <string.h> // For strlen, memcpy

// Node structure for an integer stack
typedef struct IntStackNode {
    int data;
    struct IntStackNode* next;
} IntStackNode;

// Node structure for a generic data stack (e.g., storing pointers to dynamically allocated strings)
typedef struct DataStackNode {
    void* data; // Pointer to the actual data (e.g., a string)
    struct DataStackNode* next;
} DataStackNode;

// Function: peek_top
// Peeks at the top element of the integer stack without removing it.
// Returns 0 if the stack is empty, otherwise returns the data of the top element.
int peek_top(IntStackNode **stack_head_ptr) {
    if (stack_head_ptr == NULL || *stack_head_ptr == NULL) {
        return 0; // Stack is empty or invalid pointer
    }
    return (*stack_head_ptr)->data;
}

// Function: push
// Pushes an integer onto the integer stack.
// Returns 0 on success, -1 on failure (e.g., malloc failure or if data is 0,
// as per original code's specific condition).
int push(IntStackNode **stack_head_ptr, int data) {
    // Original code treated data == 0 as an error condition (returned 0xffffffff).
    // Retaining this specific behavior.
    if (data == 0) {
        return -1;
    }

    IntStackNode *newNode = (IntStackNode *)malloc(sizeof(IntStackNode));
    if (newNode == NULL) {
        return -1; // Malloc failed
    }

    newNode->data = data;
    newNode->next = *stack_head_ptr; // Link new node to current head
    *stack_head_ptr = newNode;       // Update head pointer to the new node

    return 0; // Success
}

// Function: push_copy
// Pushes a copy of a string onto the generic data stack.
// The string data is dynamically allocated and copied.
// Returns 0 on success, -1 on failure (e.g., null string, buffer too small, malloc failure).
int push_copy(DataStackNode **stack_head_ptr, char *src_string, unsigned int max_size) {
    if (src_string == NULL) {
        return -1; // Null source string
    }

    size_t string_len = strlen(src_string);
    unsigned int required_size = string_len + 1; // +1 for null terminator

    if (max_size < required_size) {
        return -1; // Provided max_size is too small for the string
    }

    DataStackNode *newNode = (DataStackNode *)malloc(sizeof(DataStackNode));
    if (newNode == NULL) {
        return -1; // Malloc failed for the stack node
    }

    newNode->data = malloc(required_size);
    if (newNode->data == NULL) {
        free(newNode); // Clean up the allocated node if data malloc fails
        return -1; // Malloc failed for the string data
    }

    memcpy(newNode->data, src_string, required_size); // Copy the string data
    newNode->next = *stack_head_ptr;                 // Link new node to current head
    *stack_head_ptr = newNode;                       // Update head pointer to the new node

    return 0; // Success
}

// Function: pop_copy
// Pops a node from the generic data stack and returns a pointer to its data.
// The caller is responsible for freeing the returned data pointer (e.g., the string).
// Returns NULL if the stack is empty.
void* pop_copy(DataStackNode **stack_head_ptr) {
    if (stack_head_ptr == NULL || *stack_head_ptr == NULL) {
        return NULL; // Stack is empty or invalid pointer
    }

    DataStackNode *temp = *stack_head_ptr; // Get the current head node
    void *data_to_return = temp->data;     // Get the data pointer from the node
    *stack_head_ptr = temp->next;         // Update head to point to the next node
    free(temp);                           // Free the popped node itself

    return data_to_return; // Return the data pointer
}

// Function: clear_stack
// Clears the entire generic data stack, freeing both the data stored in each node
// and the nodes themselves.
void clear_stack(DataStackNode **stack_head_ptr) {
    if (stack_head_ptr == NULL) {
        return; // Invalid pointer
    }

    DataStackNode *current = *stack_head_ptr;
    while (current != NULL) {
        DataStackNode *next_node = current->next;
        free(current->data); // Free the data pointed to by the node
        free(current);       // Free the node itself
        current = next_node;
    }
    *stack_head_ptr = NULL; // Mark the stack as empty
}

int main() {
    // --- Demonstrate IntStack operations ---
    printf("--- Int Stack Operations ---\n");
    IntStackNode *int_stack_head = NULL; // Initialize integer stack head to NULL

    printf("Pushing 10, 20, 30 onto int stack...\n");
    push(&int_stack_head, 10);
    push(&int_stack_head, 20);
    push(&int_stack_head, 30); // 30 is now at the top

    printf("Top element: %d\n", peek_top(&int_stack_head)); // Should be 30

    // Manually pop an int for demonstration (no explicit pop function provided for int stack)
    if (int_stack_head != NULL) {
        IntStackNode *temp_int_node = int_stack_head;
        int_stack_head = temp_int_node->next;
        printf("Popped int: %d\n", temp_int_node->data);
        free(temp_int_node);
    }
    printf("New top element: %d\n", peek_top(&int_stack_head)); // Should be 20

    // Manually clear int stack for demonstration (no explicit clear function provided for int stack)
    printf("Clearing int stack...\n");
    while (int_stack_head != NULL) {
        IntStackNode *next_int = int_stack_head->next;
        free(int_stack_head);
        int_stack_head = next_int;
    }
    printf("Int stack head after clear: %p\n", (void*)int_stack_head);


    // --- Demonstrate DataStack (string) operations ---
    printf("\n--- Data Stack (String) Operations ---\n");
    DataStackNode *data_stack_head = NULL; // Initialize generic data stack head to NULL
    unsigned int buffer_size = 256;         // Max size for copied strings

    printf("Pushing strings onto data stack...\n");
    push_copy(&data_stack_head, "Hello", buffer_size);
    push_copy(&data_stack_head, "World", buffer_size);
    push_copy(&data_stack_head, "C Programming", buffer_size); // "C Programming" is now at the top

    char *popped_str;

    popped_str = (char*)pop_copy(&data_stack_head);
    if (popped_str) {
        printf("Popped string: \"%s\"\n", popped_str); // Should be "C Programming"
        free(popped_str); // IMPORTANT: Free the dynamically allocated string data
    }

    popped_str = (char*)pop_copy(&data_stack_head);
    if (popped_str) {
        printf("Popped string: \"%s\"\n", popped_str); // Should be "World"
        free(popped_str); // IMPORTANT: Free the dynamically allocated string data
    }

    popped_str = (char*)pop_copy(&data_stack_head);
    if (popped_str) {
        printf("Popped string: \"%s\"\n", popped_str); // Should be "Hello"
        free(popped_str); // IMPORTANT: Free the dynamically allocated string data
    }

    popped_str = (char*)pop_copy(&data_stack_head);
    if (popped_str == NULL) {
        printf("Attempted to pop from empty stack (correctly returned NULL).\n");
    }

    printf("Pushing another string for clear_stack demo...\n");
    push_copy(&data_stack_head, "Cleanup Test", buffer_size);
    printf("Clearing data stack...\n");
    clear_stack(&data_stack_head); // Use the provided clear_stack function
    printf("Data stack head after clear: %p\n", (void*)data_stack_head);

    return 0;
}