#include <stdbool.h> // For bool
#include <stdlib.h>  // For calloc, exit, EXIT_FAILURE, NULL
#include <stdio.h>   // For puts, printf

// Function: pop
int pop(int *param_1) {
  if (*param_1 == 0) {
    puts("Stack underflow!");
    exit(EXIT_FAILURE);
  }
  return param_1[(*param_1)-- + 1];
}

// Function: push
void push(int *param_1, int param_2) {
  // Assuming a maximum capacity of 31 elements based on 0x84 bytes allocation (33 ints)
  // param_1[0] is count, param_1[1] is unused. Max element index is 32.
  // So max count is 31 (param_1[31+1] = param_1[32]).
  if (*param_1 >= 31) {
    puts("Stack overflow!");
    exit(EXIT_FAILURE);
  }
  param_1[++(*param_1) + 1] = param_2;
}

// Function: isEmpty
bool isEmpty(int *param_1) {
  return *param_1 == 0;
}

// Function: initStack
void * initStack(void) {
  void *stack_ptr;
  if ((stack_ptr = calloc(0x84, 1)) == NULL) { // 0x84 bytes = 33 integers
    puts("Critical memory error. Cowardly exiting.");
    exit(EXIT_FAILURE);
  }
  // Initialize the stack pointer (count of elements) to 0
  ((int*)stack_ptr)[0] = 0;
  return stack_ptr;
}

int main() {
  int *myStack = (int *)initStack();

  printf("Is stack empty? %s\n", isEmpty(myStack) ? "Yes" : "No");

  printf("Pushing 10, 20, 30...\n");
  push(myStack, 10);
  push(myStack, 20);
  push(myStack, 30);

  printf("Is stack empty? %s\n", isEmpty(myStack) ? "Yes" : "No");
  printf("Current stack size: %d\n", myStack[0]);

  printf("Popped: %d\n", pop(myStack));
  printf("Current stack size: %d\n", myStack[0]);
  printf("Popped: %d\n", pop(myStack));
  printf("Current stack size: %d\n", myStack[0]);

  printf("Pushing 40...\n");
  push(myStack, 40);
  printf("Current stack size: %d\n", myStack[0]);

  printf("Popped: %d\n", pop(myStack));
  printf("Current stack size: %d\n", myStack[0]);
  printf("Popped: %d\n", pop(myStack));
  printf("Current stack size: %d\n", myStack[0]);

  printf("Is stack empty? %s\n", isEmpty(myStack) ? "Yes" : "No");

  // Attempt to pop from an empty stack (will trigger underflow error and exit)
  // printf("Attempting to pop from empty stack...\n");
  // pop(myStack); 

  free(myStack);
  return EXIT_SUCCESS;
}