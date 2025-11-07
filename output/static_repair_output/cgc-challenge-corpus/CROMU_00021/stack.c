#include <stdlib.h>  // For calloc, exit, EXIT_FAILURE, free
#include <stdio.h>   // For puts, printf
#include <stdbool.h> // For bool

// Function: pop
// Reduces the intermediate variable 'iVar1' by using postfix decrement.
int pop(int *param_1) {
  // *param_1 holds the 1-based count of elements.
  // Elements are stored from param_1[2] onwards.
  // The top element is at param_1[*param_1 + 1].
  // (*param_1)-- evaluates *param_1 (the current count) first,
  // uses that value to access the array, then decrements *param_1.
  return param_1[(*param_1)-- + 1];
}

// Function: push
// param_1[0] stores the 1-based count of elements.
// Elements are stored from param_1[2] onwards.
void push(int *param_1, int param_2) {
  (*param_1)++;                   // Increment stack pointer (count)
  param_1[*param_1 + 1] = param_2; // Store at new_count + 1
}

// Function: isEmpty
// param_1[0] stores the 1-based count of elements.
bool isEmpty(int *param_1) {
  return *param_1 == 0;
}

// Function: initStack
// Allocates memory for the stack.
// The stack is an array of ints. The first element (param_1[0]) is the stack pointer/count.
// Max capacity is 0x84 bytes (33 integers).
// Given the push/pop logic, param_1[1] is unused, and elements are stored from param_1[2] onwards.
// Therefore, the actual stack can hold 33 - 2 = 31 elements.
void *initStack(void) {
  void *stack_ptr = calloc(0x84, 1); // Allocate 0x84 bytes, initialized to zero
  if (stack_ptr == NULL) {           // Check for allocation failure
    puts("Critical memory error. Cowardly exiting."); // Fixed typo and standard error message
    exit(EXIT_FAILURE);              // Replaced _terminate with standard exit
  }
  // calloc already initializes all bytes to zero, so stack_ptr[0] will be 0,
  // correctly indicating an empty stack.
  return stack_ptr;
}

// Main function to demonstrate usage and make the code compilable
int main() {
  // Initialize the stack (initStack returns void*, cast to int* for type safety)
  int *myStack = (int *)initStack();

  printf("Is stack empty? %s\n", isEmpty(myStack) ? "Yes" : "No"); // Expected: Yes

  printf("Pushing 10, 20, 30...\n");
  push(myStack, 10);
  push(myStack, 20);
  push(myStack, 30);

  printf("Is stack empty? %s\n", isEmpty(myStack) ? "Yes" : "No"); // Expected: No

  printf("Popped: %d\n", pop(myStack)); // Expected: 30
  printf("Popped: %d\n", pop(myStack)); // Expected: 20

  printf("Is stack empty? %s\n", isEmpty(myStack) ? "Yes" : "No"); // Expected: No

  printf("Pushing 40...\n");
  push(myStack, 40);

  printf("Popped: %d\n", pop(myStack)); // Expected: 40
  printf("Popped: %d\n", pop(myStack)); // Expected: 10

  printf("Is stack empty? %s\n", isEmpty(myStack) ? "Yes" : "No"); // Expected: Yes

  // Free the allocated memory to prevent memory leaks
  free(myStack);
  myStack = NULL;

  return 0;
}