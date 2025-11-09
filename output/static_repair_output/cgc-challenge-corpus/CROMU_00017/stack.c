#include <stdbool.h>
#include <stdint.h>

#define STACK_SIZE 64

static int curr_num_stack = -1;
static uint32_t num_stack[STACK_SIZE];

static int curr_op_stack = -1;
static char op_stack[STACK_SIZE];

bool push_num(uint32_t value) {
  if (curr_num_stack < STACK_SIZE - 1) {
    num_stack[++curr_num_stack] = value;
    return true;
  }
  return false;
}

bool pop_num(uint32_t *value) {
  if (curr_num_stack >= 0) {
    *value = num_stack[curr_num_stack--];
    return true;
  }
  return false;
}

bool peek_op(char *value) {
  if (curr_op_stack >= 0) {
    *value = op_stack[curr_op_stack];
    return true;
  }
  return false;
}

bool push_op(char value) {
  if (curr_op_stack < STACK_SIZE - 1) {
    op_stack[++curr_op_stack] = value;
    return true;
  }
  return false;
}

bool pop_op(char *value) {
  if (curr_op_stack >= 0) {
    *value = op_stack[curr_op_stack--];
    return true;
  }
  return false;
}