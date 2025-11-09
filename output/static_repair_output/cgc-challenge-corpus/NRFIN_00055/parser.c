#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> // For size_t
#include <string.h> // For memset, strncpy

// Define the ASTNode structure based on memory offsets
typedef struct ASTNode {
    struct ASTNode *next; // Offset 0x0 (pointer to next node in stack)
    uint32_t type;        // Offset 0x4 (node type: literal, identifier, bin_op, unary_op, paren)
    uint32_t value_or_op; // Offset 0x8 (value for literals/identifiers, operator enum for ops)
    struct ASTNode *left; // Offset 0xc (left child for binary, operand for unary)
    struct ASTNode *right;// Offset 0x10 (right child for binary, NULL for unary)
} ASTNode; // Total size: 5 * 4 = 20 bytes (0x14)

// Define the token structure as an array of these is passed to parse
typedef struct ASTNodeToken {
    uint32_t type;  // Token type (e.g., 0=literal, 1=identifier, 2-6=binary ops, 7-9=unary ops, 10='(', 11=')')
    uint32_t value; // Token value (e.g., integer literal, identifier hash/id, operator enum)
} ASTNodeToken; // Total size: 2 * 4 = 8 bytes

// External function declarations (assuming these exist elsewhere)
extern int pool_init(void *context, size_t node_size);
extern void *pool_alloc(void *context);
extern void pool_destroy(void *context);

// External data declaration
// Assuming this is an array of integers representing precedence for binary operators.
// Indices correspond to ASTNode.value_or_op for binary operators.
extern int bin_op_precedence_0[];

// Function: stack_empty
// Checks if the stack is empty.
// param_1: A pointer to the stack head (which itself is an ASTNode*).
// Returns true if empty, false otherwise.
bool stack_empty(ASTNode **stack_head) {
  return *stack_head == NULL;
}

// Function: push_ast_node
// Pushes an ASTNode onto the stack.
// stack_head: A pointer to the stack head pointer.
// node_to_push: The ASTNode to be pushed.
void push_ast_node(ASTNode **stack_head, ASTNode *node_to_push) {
  node_to_push->next = *stack_head;
  *stack_head = node_to_push;
}

// Function: pop_ast_node
// Pops an ASTNode from the stack.
// stack_head: A pointer to the stack head pointer.
// popped_node_ptr: A pointer to an ASTNode* where the popped node will be stored.
// Returns 0 on success, -1 on error (stack empty).
int pop_ast_node(ASTNode **stack_head, ASTNode **popped_node_ptr) {
  if (stack_empty(stack_head)) {
    return -1; // Error: stack is empty
  }
  *popped_node_ptr = *stack_head;
  *stack_head = (*stack_head)->next;
  return 0; // Success
}

// Function: peek_ast_node
// Peeks at the top ASTNode of the stack without removing it.
// stack_head: A pointer to the stack head pointer.
// peeked_node_ptr: A pointer to an ASTNode* where the peeked node will be stored.
// Returns 0 on success, -1 on error (stack empty).
int peek_ast_node(ASTNode **stack_head, ASTNode **peeked_node_ptr) {
  if (stack_empty(stack_head)) {
    return -1; // Error: stack is empty
  }
  *peeked_node_ptr = *stack_head;
  return 0; // Success
}

// Function: push_bin_op
// Handles pushing a binary operator onto the operator stack,
// processing higher precedence operators already on the stack.
// operand_stack_head: Pointer to the head of the operand stack.
// operator_stack_head: Pointer to the head of the operator stack.
// current_operator_node: The binary operator node to be pushed.
// Returns 0 on success, -1 on error.
int push_bin_op(ASTNode **operand_stack_head, ASTNode **operator_stack_head, ASTNode *current_operator_node) {
  ASTNode *top_op_node = NULL;
  ASTNode *right_operand = NULL;
  ASTNode *left_operand = NULL;

  while (!stack_empty(operator_stack_head)) {
    if (peek_ast_node(operator_stack_head, &top_op_node) < 0) {
      return -1; // Should not happen if stack_empty check passes
    }

    // Process operators on stack based on type and precedence
    if (top_op_node->type == 2) { // Binary operator
      if (bin_op_precedence_0[top_op_node->value_or_op] <
          bin_op_precedence_0[current_operator_node->value_or_op]) {
        break; // Current operator has higher precedence, push it later
      }

      if (pop_ast_node(operator_stack_head, &top_op_node) < 0) return -1;
      if (pop_ast_node(operand_stack_head, &right_operand) < 0) return -1;
      if (pop_ast_node(operand_stack_head, &left_operand) < 0) return -1;

      // Special check for 'power' operator (value_or_op == 0) and its left operand type
      if (top_op_node->value_or_op == 0 && left_operand->type != 1 &&
          !(left_operand->type == 3 && left_operand->value_or_op == 2)) {
        return -1;
      }

      top_op_node->left = left_operand;
      top_op_node->right = right_operand;
      push_ast_node(operand_stack_head, top_op_node);
    } else if (top_op_node->type == 3) { // Unary operator
      if (pop_ast_node(operator_stack_head, &top_op_node) < 0) return -1;
      if (pop_ast_node(operand_stack_head, &right_operand) < 0) return -1; // Unary ops take one operand

      // Special checks for unary ops
      if (top_op_node->value_or_op == 1 && right_operand->type != 1) { // Unary minus/plus on non-literal
        return -1;
      }
      if (top_op_node->value_or_op == 2 && right_operand->type != 1 && right_operand->type != 3) { // Logical NOT on non-literal/unary op
        return -1;
      }

      top_op_node->left = right_operand; // Unary ops typically store operand in 'left'
      top_op_node->right = NULL; // No right operand for unary
      push_ast_node(operand_stack_head, top_op_node);
    } else {
      break; // Not a binary or unary operator (e.g., an opening parenthesis), stop processing
    }
  }

  push_ast_node(operator_stack_head, current_operator_node);
  return 0;
}

// Function: close_parenthesis
// Processes operators on the stack until an opening parenthesis is found.
// operand_stack_head: Pointer to the head of the operand stack.
// operator_stack_head: Pointer to the head of the operator stack.
// Returns 0 on success (matching '(' found), -1 on error (unmatched ')' or invalid state).
int close_parenthesis(ASTNode **operand_stack_head, ASTNode **operator_stack_head) {
  ASTNode *op_node = NULL;
  ASTNode *right_operand = NULL;
  ASTNode *left_operand = NULL;

  while (true) {
    if (stack_empty(operator_stack_head)) {
      return -1; // Error: No matching open parenthesis found
    }
    if (pop_ast_node(operator_stack_head, &op_node) < 0) {
      return -1; // Should not happen if stack_empty check passes
    }

    if (op_node->type == 4) { // Found opening parenthesis
      return 0; // Success
    }

    if (op_node->type == 2) { // Binary operator
      if (pop_ast_node(operand_stack_head, &right_operand) < 0) return -1;
      if (pop_ast_node(operand_stack_head, &left_operand) < 0) return -1;

      // Special check for 'power' operator (value_or_op == 0) and its left operand type
      if (op_node->value_or_op == 0 && left_operand->type != 1 &&
          !(left_operand->type == 3 && left_operand->value_or_op == 2)) {
        return -1;
      }

      op_node->left = left_operand;
      op_node->right = right_operand;
      push_ast_node(operand_stack_head, op_node);
    } else if (op_node->type == 3) { // Unary operator
      if (pop_ast_node(operand_stack_head, &right_operand) < 0) return -1;

      // Special checks for unary ops
      if (op_node->value_or_op == 1 && right_operand->type != 1) { // Unary minus/plus on non-literal
        return -1;
      }
      if (op_node->value_or_op == 2 && right_operand->type != 1 && right_operand->type != 3) { // Logical NOT on non-literal/unary op
        return -1;
      }

      op_node->left = right_operand;
      op_node->right = NULL;
      push_ast_node(operand_stack_head, op_node);
    } else {
      // Unexpected node type on operator stack (e.g., another unmatched parenthesis type)
      return -1;
    }
  }
}

// Function: clear_operator_stack
// Processes all remaining operators on the operator stack after all tokens have been parsed.
// operand_stack_head: Pointer to the head of the operand stack.
// operator_stack_head: Pointer to the head of the operator stack.
// Returns 0 on success, -1 on error (e.g., unmatched opening parenthesis, invalid operator).
int clear_operator_stack(ASTNode **operand_stack_head, ASTNode **operator_stack_head) {
  ASTNode *op_node = NULL;
  ASTNode *right_operand = NULL;
  ASTNode *left_operand = NULL;

  while (!stack_empty(operator_stack_head)) {
    if (pop_ast_node(operator_stack_head, &op_node) < 0) {
      return -1; // Should not happen if stack_empty check passes
    }

    if (op_node->type == 4) { // Unmatched opening parenthesis found at end of parsing
      return -1;
    }

    if (op_node->type == 2) { // Binary operator
      if (pop_ast_node(operand_stack_head, &right_operand) < 0) return -1;
      if (pop_ast_node(operand_stack_head, &left_operand) < 0) return -1;

      // Special check for 'power' operator (value_or_op == 0) and its left operand type
      if (op_node->value_or_op == 0 && left_operand->type != 1 &&
          !(left_operand->type == 3 && left_operand->value_or_op == 2)) {
        return -1;
      }

      op_node->left = left_operand;
      op_node->right = right_operand;
      push_ast_node(operand_stack_head, op_node);
    } else if (op_node->type == 3) { // Unary operator
      if (pop_ast_node(operand_stack_head, &right_operand) < 0) return -1;

      // Special checks for unary ops
      if (op_node->value_or_op == 1 && right_operand->type != 1) { // Unary minus/plus on non-literal
        return -1;
      }
      if (op_node->value_or_op == 2 && right_operand->type != 1 && right_operand->type != 3) { // Logical NOT on non-literal/unary op
        return -1;
      }

      op_node->left = right_operand;
      op_node->right = NULL;
      push_ast_node(operand_stack_head, op_node);
    } else {
      // Unexpected node type on operator stack (should only contain operators or parentheses)
      return -1;
    }
  }
  return 0; // Success, all operators processed
}

// Function: parse
// Parses a sequence of tokens into an Abstract Syntax Tree (AST).
// tokens: Pointer to an array of ASTNodeToken representing the input expression.
// num_tokens: The number of tokens in the input array.
// pool_context_and_result: A pointer to a context structure for memory pooling.
//                          It is also assumed that the final AST root pointer will be stored
//                          at an offset of 8 bytes within this context structure.
// Returns 0 on success, -1 on error.
int parse(ASTNodeToken *tokens, uint32_t num_tokens, void *pool_context_and_result) {
  ASTNode *operand_stack_head = NULL;  // Head of the stack for operands (literals, identifiers, sub-expressions)
  ASTNode *operator_stack_head = NULL; // Head of the stack for operators and parentheses
  ASTNode *current_node;
  int ret_val;

  // Initialize the memory pool for AST nodes
  if (pool_init(pool_context_and_result, sizeof(ASTNode)) < 0) {
    return -1;
  }

  // Iterate through each token
  for (uint32_t i = 0; i < num_tokens; ++i) {
    current_node = (ASTNode *)pool_alloc(pool_context_and_result);
    if (current_node == NULL) {
      return -1; // Memory allocation failed
    }
    memset(current_node, 0, sizeof(ASTNode)); // Initialize node to zeros

    switch (tokens[i].type) {
      case 0: // Literal (e.g., integer value)
        current_node->type = 0;
        current_node->value_or_op = tokens[i].value;
        push_ast_node(&operand_stack_head, current_node);
        break;
      case 1: // Identifier
        current_node->type = 1;
        // Assuming tokens[i].value directly holds the 4-byte identifier data
        current_node->value_or_op = tokens[i].value;
        push_ast_node(&operand_stack_head, current_node);
        break;
      case 2: // Binary operator: +
      case 3: // Binary operator: -
      case 4: // Binary operator: *
      case 5: // Binary operator: /
      case 6: // Binary operator: ^ (power)
        current_node->type = 2; // All these are binary operators
        current_node->value_or_op = tokens[i].type - 2; // Map token type to internal operator enum (0 to 4)
        ret_val = push_bin_op(&operand_stack_head, &operator_stack_head, current_node);
        if (ret_val < 0) return -1;
        break;
      case 7: // Unary operator: +
      case 8: // Unary operator: -
      case 9: // Unary operator: ! (logical NOT)
        current_node->type = 3; // All these are unary operators
        current_node->value_or_op = tokens[i].type - 7; // Map token type to internal operator enum (0 to 2)
        push_ast_node(&operator_stack_head, current_node);
        break;
      case 10: // Opening parenthesis: (
        current_node->type = 4;
        push_ast_node(&operator_stack_head, current_node);
        break;
      case 11: // Closing parenthesis: )
        ret_val = close_parenthesis(&operand_stack_head, &operator_stack_head);
        if (ret_val < 0) return -1;
        break;
      default:
        // Unknown token type, indicates an error in input
        return -1;
    }
  }

  // After processing all tokens, clear any remaining operators on the stack
  ret_val = clear_operator_stack(&operand_stack_head, &operator_stack_head);
  if (ret_val < 0) {
    return -1;
  }

  // The final result should be a single AST node on the operand stack
  // Store the root of the AST at pool_context_and_result + 8
  if (pop_ast_node(&operand_stack_head, (ASTNode **)((uintptr_t)pool_context_and_result + 8)) < 0) {
    return -1; // Operand stack should not be empty at the end
  }

  // The operand stack should now be completely empty
  if (!stack_empty(&operand_stack_head)) {
    return -1; // More than one node left on operand stack, indicates an error
  }

  return 0; // Parsing successful
}

// Function: ast_destroy
// Destroys the AST by releasing the memory pool.
// pool_context_and_result: The context structure used for memory pooling.
void ast_destroy(void *pool_context_and_result) {
  pool_destroy(pool_context_and_result);
  // Clear the stored AST root pointer in the context structure
  *(ASTNode **)((uintptr_t)pool_context_and_result + 8) = NULL;
}