#include <stdlib.h> // Required for malloc, realloc, free
#include <stdio.h>  // Required for printf in the main function

// Define the Stack structure for better type safety and readability
typedef struct Stack {
    int count;      // Number of elements currently in the stack
    int capacity;   // Total allocated capacity of the stack
    int* data;      // Pointer to the array of stack elements
} Stack;

// Function: stack_new
Stack* stack_new(int initial_capacity) {
    // Ensure a positive initial capacity
    if (initial_capacity <= 0) {
        initial_capacity = 4; // Default to a small, reasonable capacity
    }

    Stack* s = (Stack*)malloc(sizeof(Stack));
    if (s == NULL) {
        return NULL; // Failed to allocate stack structure
    }

    s->count = 0;
    s->capacity = initial_capacity;
    s->data = (int*)malloc(s->capacity * sizeof(int));

    if (s->data == NULL) {
        free(s); // Free the stack structure if data allocation fails
        return NULL;
    }

    return s;
}

// Function: _stack_resize
// This is an internal helper function, hence the leading underscore.
// It tries to resize the stack's underlying data array.
void _stack_resize(Stack* s, int new_capacity) {
    if (s == NULL || new_capacity <= 0) {
        return; // Invalid stack or capacity
    }

    // Ensure new capacity is at least the current count
    if (new_capacity < s->count) {
        new_capacity = s->count;
        if (new_capacity == 0) new_capacity = 1; // Minimum capacity of 1 if count is 0
    }

    int* new_data = (int*)realloc(s->data, new_capacity * sizeof(int));
    if (new_data != NULL) {
        s->data = new_data;
        s->capacity = new_capacity;
    }
    // If realloc fails, s->data and s->capacity remain unchanged,
    // and the original memory block is still valid.
}

// Function: stack_push
void stack_push(Stack* s, int value) {
    if (s == NULL) {
        return; // Invalid stack
    }

    // Resize if the stack is full
    if (s->count == s->capacity) {
        _stack_resize(s, s->capacity * 2); // Double the capacity
        // If resize failed (e.g., out of memory), capacity might not have changed.
        // In such a case, we cannot push the element.
        if (s->count == s->capacity) {
            fprintf(stderr, "Error: Stack resize failed, cannot push element.\n");
            return;
        }
    }
    s->data[s->count] = value;
    s->count++;
}

// Function: stack_pop
int stack_pop(Stack* s) {
    if (s == NULL || s->count == 0) {
        return 0; // Return 0 or an appropriate error indicator for an empty stack
    }

    // Shrink the stack if it's significantly underutilized
    // (e.g., count is a quarter of capacity and capacity is not too small)
    if (s->count <= s->capacity / 4 && s->capacity > 8) { // Shrink when 1/4 full, but not below a certain size
        _stack_resize(s, s->capacity / 2); // Halve the capacity
    }

    s->count--;
    return s->data[s->count]; // Return the value that was at the top
}

// Function: stack_peek
int stack_peek(Stack* s) {
    if (s == NULL || s->count == 0) {
        return 0; // Return 0 or an appropriate error indicator for an empty stack
    }
    return s->data[s->count - 1]; // Return the value at the top without removing it
}

// Function: stack_destroy
Stack* stack_destroy(Stack* s) {
    if (s != NULL) {
        if (s->data != NULL) {
            free(s->data); // Free the data array
            s->data = NULL;
        }
        s->count = 0;
        s->capacity = 0;
        free(s); // Free the stack structure itself
    }
    return NULL; // Return NULL to indicate the stack no longer exists
}

// Main function to demonstrate stack usage
int main() {
    Stack* my_stack = stack_new(5); // Initial capacity of 5

    if (my_stack == NULL) {
        fprintf(stderr, "Failed to create stack.\n");
        return 1;
    }

    printf("Stack created. Capacity: %d, Count: %d\n", my_stack->capacity, my_stack->count);

    printf("Pushing 10, 20, 30...\n");
    stack_push(my_stack, 10);
    stack_push(my_stack, 20);
    stack_push(my_stack, 30);
    printf("Count: %d, Peek: %d\n", my_stack->count, stack_peek(my_stack));

    printf("Pushing 40, 50, 60 (triggering resize)...\n");
    stack_push(my_stack, 40);
    stack_push(my_stack, 50);
    printf("Capacity after push: %d, Count: %d\n", my_stack->capacity, my_stack->count); // Capacity should double
    stack_push(my_stack, 60);
    printf("Capacity after push: %d, Count: %d\n", my_stack->capacity, my_stack->count); // Capacity should double again
    printf("Peek: %d\n", stack_peek(my_stack));

    printf("\nPopping elements:\n");
    printf("Popped: %d, Count: %d, Capacity: %d\n", stack_pop(my_stack), my_stack->count, my_stack->capacity);
    printf("Popped: %d, Count: %d, Capacity: %d\n", stack_pop(my_stack), my_stack->count, my_stack->capacity);
    printf("Popped: %d, Count: %d, Capacity: %d\n", stack_pop(my_stack), my_stack->count, my_stack->capacity);
    printf("Popped: %d, Count: %d, Capacity: %d\n", stack_pop(my_stack), my_stack->count, my_stack->capacity); // Should trigger shrink

    printf("Peek after pops: %d\n", stack_peek(my_stack));

    printf("\nPopping remaining elements:\n");
    printf("Popped: %d, Count: %d, Capacity: %d\n", stack_pop(my_stack), my_stack->count, my_stack->capacity);
    printf("Popped: %d, Count: %d, Capacity: %d\n", stack_pop(my_stack), my_stack->count, my_stack->capacity);
    printf("Popped (empty stack): %d, Count: %d, Capacity: %d\n", stack_pop(my_stack), my_stack->count, my_stack->capacity); // Popping from empty stack

    printf("\nAttempting to peek/pop from empty stack:\n");
    printf("Peek (empty): %d\n", stack_peek(my_stack));
    printf("Pop (empty): %d\n", stack_pop(my_stack));

    my_stack = stack_destroy(my_stack);
    if (my_stack == NULL) {
        printf("\nStack destroyed successfully.\n");
    }

    return 0;
}