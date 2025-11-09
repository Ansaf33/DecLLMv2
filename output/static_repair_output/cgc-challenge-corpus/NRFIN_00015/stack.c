#include <stdbool.h> // For bool type
#include <stdint.h>  // For uint32_t and fixed-width integer types
#include <stdio.h>   // For fprintf, stderr, printf
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For memcpy

// Define fixed-size integer type for stack elements, assuming 'undefined4' refers to 32-bit unsigned int
typedef uint32_t stack_element_t;

// Define the Stack structure based on the usage patterns in the original snippet
typedef struct {
    int top;          // Index of the top element (-1 if empty)
    int element_size; // Size of each element in bytes
    int capacity;     // Maximum number of elements
    void* data;       // Pointer to the allocated memory for stack elements
} Stack;

// --- Helper/Replacement Functions (simulating original behavior) ---

// Replaces the original `_terminate` function.
// It prints an error message to stderr and exits the program.
void terminate_program(int exit_code) {
    fprintf(stderr, "Program terminated with code 0x%x\n", exit_code);
    exit(exit_code);
}

// Replaces the original `transmit_all` function.
// It prints a message to stderr. Returns 0 to simulate success.
int transmit_all_message(const char* message) {
    fprintf(stderr, "%s", message);
    return 0; // Assuming 0 for success, non-zero for failure in original.
}

// --- Stack Functions ---

// Initializes a stack.
// param_1 (stack): Pointer to the Stack structure to initialize.
// param_2 (capacity): The maximum number of elements the stack can hold.
// param_3 (element_size): The size of each element in bytes.
void initStack(Stack *stack, int capacity, int element_size) {
    if (stack == NULL) {
        terminate_program(0xBAD); // Custom error for an invalid stack pointer
    }

    // Original check: `(int)(2000000 / (longlong)param_3) < param_2`
    // This implies a total memory limit (2MB) and a check to prevent overflow
    // when calculating total allocation size (`capacity * element_size`).
    if ((long long)capacity * element_size > 2000000 || capacity < 0 || element_size < 0) {
        stack->data = NULL; // Indicate failure or uninitialized state
        stack->capacity = 0;
        stack->element_size = 0;
        stack->top = -1;
        return; // Original code would return without allocation
    }

    stack->data = malloc((size_t)capacity * element_size);
    if (stack->data == NULL) {
        terminate_program(0x10); // Original error code for allocation failure
    }
    stack->capacity = capacity;
    stack->element_size = element_size;
    stack->top = -1; // Initialize top to -1 for an empty stack (0xffffffff as signed int)
}

// Destroys a stack, freeing its allocated memory.
// param_1 (stack): Pointer to the Stack structure to destroy.
void destroyStack(Stack *stack) {
    if (stack == NULL) {
        return; // Nothing to destroy
    }
    if (stack->data != NULL) {
        free(stack->data);
        stack->data = NULL;
    }
    stack->capacity = 0;
    stack->element_size = 0;
    stack->top = -1; // Reset top to -1
}

// Checks if the stack is full.
// param_1 (stack): Pointer to the Stack structure.
// Returns true if full, false otherwise.
bool isStackFull(const Stack *stack) {
    if (stack == NULL || stack->data == NULL) {
        return true; // An uninitialized or invalid stack cannot accept pushes
    }
    // Stack is full when 'top' points to the last valid index (capacity - 1)
    return stack->top == stack->capacity - 1;
}

// Checks if the stack is empty.
// param_1 (stack): Pointer to the Stack structure.
// Returns true if empty, false otherwise.
bool isStackEmpty(const Stack *stack) {
    if (stack == NULL || stack->data == NULL) {
        return true; // An uninitialized or invalid stack has no elements to pop
    }
    // Stack is empty when 'top' is -1
    return stack->top == -1;
}

// Pushes an element onto the stack.
// param_1 (stack): Pointer to the Stack structure.
// param_2 (value): The element value to push.
void pushElement(Stack *stack, stack_element_t value) {
    if (isStackFull(stack)) {
        transmit_all_message("Too many elements!\n");
        terminate_program(0x12); // Original termination code
    }

    stack->top++; // Increment top index before storing the new element

    // Calculate the memory address where the new element should be stored
    void* target_address = (char*)stack->data + (size_t)stack->top * stack->element_size;

    // Copy the value into the calculated memory location
    memcpy(target_address, &value, stack->element_size);
}

// Pops an element from the stack.
// param_1 (stack): Pointer to the Stack structure.
// Returns the popped element.
stack_element_t popElement(Stack *stack) {
    if (isStackEmpty(stack)) {
        transmit_all_message("Not enough elements!\n");
        terminate_program(0x14); // Original termination code
    }

    // Calculate the memory address of the top element
    void* source_address = (char*)stack->data + (size_t)stack->top * stack->element_size;

    stack_element_t value;
    // Copy the value from the stack to a local variable
    memcpy(&value, source_address, stack->element_size);

    stack->top--; // Decrement top index after retrieving the element

    return value;
}

// --- Main function for demonstration ---

int main() {
    Stack myStack;
    int capacity = 5;
    int element_size = sizeof(stack_element_t); // Size of uint32_t

    printf("Initializing stack with capacity %d and element size %d bytes.\n", capacity, element_size);
    initStack(&myStack, capacity, element_size);

    if (myStack.data == NULL) {
        printf("Stack initialization failed (possibly due to size limit or invalid parameters).\n");
        return 1;
    }

    printf("Is stack empty? %s\n", isStackEmpty(&myStack) ? "Yes" : "No");
    printf("Is stack full? %s\n", isStackFull(&myStack) ? "Yes" : "No");

    printf("\nPushing elements...\n");
    for (int i = 0; i < capacity; ++i) {
        printf("Pushing %u\n", (uint32_t)(100 + i));
        pushElement(&myStack, (uint32_t)(100 + i));
        printf("Current top index: %d\n", myStack.top);
    }

    printf("Is stack empty? %s\n", isStackEmpty(&myStack) ? "Yes" : "No");
    printf("Is stack full? %s\n", isStackFull(&myStack) ? "Yes" : "No");

    // Uncomment the following line to test stack full error handling
    // printf("\nAttempting to push one more element (should terminate):\n");
    // pushElement(&myStack, 200); 

    printf("\nPopping elements...\n");
    while (!isStackEmpty(&myStack)) {
        stack_element_t val = popElement(&myStack);
        printf("Popped %u, Current top index: %d\n", val, myStack.top);
    }

    printf("Is stack empty? %s\n", isStackEmpty(&myStack) ? "Yes" : "No");
    printf("Is stack full? %s\n", isStackFull(&myStack) ? "Yes" : "No");

    // Uncomment the following line to test stack empty error handling
    // printf("\nAttempting to pop from an empty stack (should terminate):\n");
    // popElement(&myStack);

    printf("\nDestroying stack.\n");
    destroyStack(&myStack);

    // Test with zero capacity
    printf("\nTesting initStack with zero capacity:\n");
    Stack zeroCapacityStack;
    initStack(&zeroCapacityStack, 0, sizeof(stack_element_t));
    if (zeroCapacityStack.data == NULL) {
        printf("Successfully handled zero capacity (data is NULL).\n");
    } else {
        printf("Failed to handle zero capacity (data is not NULL).\n");
        destroyStack(&zeroCapacityStack);
    }
    
    // Test with allocation that exceeds the 2MB limit
    printf("\nTesting initStack with excessive capacity (should fail):\n");
    Stack largeStack;
    // This capacity will make (capacity * element_size) exceed 2MB
    initStack(&largeStack, (2000000 / sizeof(stack_element_t)) + 1, sizeof(stack_element_t));
    if (largeStack.data == NULL) {
        printf("Successfully handled excessive capacity (data is NULL).\n");
    } else {
        printf("Failed to handle excessive capacity (data is not NULL).\n");
        destroyStack(&largeStack);
    }

    return 0;
}