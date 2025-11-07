#include <stdbool.h> // For bool type

#define STACK_SIZE 64

// Global variables for the number stack
static int num_stack[STACK_SIZE];
static int curr_num_stack = -1; // -1 indicates an empty stack

// Global variables for the operator stack
static char op_stack[STACK_SIZE];
static int curr_op_stack = -1; // -1 indicates an empty stack

// Function: push_num
bool push_num(int value) {
  if (curr_num_stack < STACK_SIZE - 1) {
    curr_num_stack++;
    num_stack[curr_num_stack] = value;
    return true;
  }
  return false; // Stack full
}

// Function: pop_num
bool pop_num(int *value) {
  if (curr_num_stack >= 0) {
    *value = num_stack[curr_num_stack];
    curr_num_stack--;
    return true;
  }
  return false; // Stack empty
}

// Function: peek_op
bool peek_op(char *op) {
  if (curr_op_stack >= 0) {
    *op = op_stack[curr_op_stack];
    return true;
  }
  return false; // Stack empty
}

// Function: push_op
bool push_op(char op) {
  if (curr_op_stack < STACK_SIZE - 1) {
    curr_op_stack++;
    op_stack[curr_op_stack] = op;
    return true;
  }
  return false; // Stack full
}

// Function: pop_op
bool pop_op(char *op) {
  if (curr_op_stack >= 0) {
    *op = op_stack[curr_op_stack];
    curr_op_stack--;
    return true;
  }
  return false; // Stack empty
}