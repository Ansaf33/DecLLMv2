#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For NULL, (long) casts
#include <stdbool.h> // For bool

// Define undefined4 as int for Linux compatibility.
// Assuming 0xffffffff indicates an error (-1).
typedef int undefined4;

// Define AstNode structure based on memory accesses from the original snippet.
// The fields are interpreted from their usage:
//   - `param_1 + 4` is `type`
//   - `param_1 + 8` is `subtype_or_value_or_varname_offset` (this field is heavily overloaded)
//   - `param_1 + 0xc` is `child1`
//   - `param_1 + 0x10` is `child2`
typedef struct AstNode {
    undefined4 field_0; // Placeholder for param_1 + 0
    int type;           // Node type (e.g., 0 for literal, 1 for var lookup, 2 for binary op, 3 for compound)
    int subtype_or_value_or_varname_offset; // Overloaded: literal value, var ID, or operator subtype
    struct AstNode *child1; // First child node (e.g., left operand, unary operand, variable name node)
    struct AstNode *child2; // Second child node (e.g., right operand for binary ops)
} AstNode;

// Structure for evaluation results.
// This consolidates `local_20`, `local_1c`, `local_18`, `local_2c`, `local_28`, `local_24`, `local_38`, `local_34`, `local_30`
// into structured variables, reducing the total number of intermediate variables.
// Based on `param_3[0]`, `param_3[1]`, `param_3[2]` accesses:
//   - `val_ptr_or_data`: A pointer to a variable's EvalResult (if type is 2), or 0 (if type is 1).
//   - `type`: The result type (1 for integer literal, 2 for variable reference).
//   - `value`: The actual integer value (if type is 1), or variable ID/address (if type is 2).
typedef struct {
    int val_ptr_or_data;
    int type;
    int value;
} EvalResult;

// --- Mock Symbol Table and Variable Management Functions ---
// These are mock implementations to make the code compilable and runnable.
// In a real system, `context` would point to an actual symbol table structure.
// The `var_identifier` parameter is ambiguous in the original snippet, sometimes an `int` ID,
// sometimes an address (e.g., `param_1 + 8`), and sometimes an `EvalResult*`.
// The mocks attempt to handle these inconsistencies with simple heuristics.

#define MAX_VARS 10 // Maximum number of mock variables
EvalResult symbol_table[MAX_VARS]; // Stores EvalResult for each variable
int next_available_var_id_index = 0; // Simple index for new variables

// Helper to determine variable ID from `var_identifier` (due to original snippet's ambiguity)
// This function attempts to parse `var_identifier` as either a direct integer ID,
// or extract an ID if `var_identifier` points to an `EvalResult` struct.
static int get_actual_var_id(void *var_identifier) {
    if (var_identifier == NULL) return 0;

    // Check if `var_identifier` is an address within our mock symbol_table.
    // This heuristic handles cases where `var_identifier` is `&result_left`.
    if ((long)var_identifier >= (long)&symbol_table[0] &&
        (long)var_identifier < (long)&symbol_table[MAX_VARS]) {
        EvalResult *res = (EvalResult *)var_identifier;
        if (res->value != 0) { // If EvalResult contains a non-zero value, use it as ID.
            return res->value;
        }
        // If value is 0, it means the EvalResult itself is the temporary identifier for a new anonymous var.
        // We'll assign a new ID later. For now, indicate 'no specific ID'.
        return 0;
    }

    // Otherwise, treat `var_identifier` as a direct integer ID.
    return (int)(long)var_identifier;
}

// Mock function to look up a variable.
// Returns the "address" (index into `symbol_table`) of the variable's `EvalResult`, or 0 if not found.
int lookup_variable(void *context, void *var_identifier) {
    int id = get_actual_var_id(var_identifier);

    // Map specific IDs from test cases to internal indices for consistency.
    if (id == 0x10000000) id = 0; // Mock ID for 'x'
    else if (id == 0x20000000) id = 1; // Mock ID for 'y'
    else if (id >= MAX_VARS || id < 0) return 0; // Out of bounds.

    // If the ID corresponds to an already allocated variable.
    if (id < next_available_var_id_index) {
        return (int)(long)&symbol_table[id];
    }
    return 0; // Not found
}

// Mock function to insert/create a variable.
// Returns the "address" (index into `symbol_table`) of the variable's `EvalResult`.
int insert_variable(void *context, void *var_identifier, int var_type) {
    int id = get_actual_var_id(var_identifier);

    // Map specific IDs from test cases to internal indices.
    if (id == 0x10000000) id = 0; // Mock ID for 'x'
    else if (id == 0x20000000) id = 1; // Mock ID for 'y'
    else if (id == 0) { // If no specific ID, assign a new one
        id = next_available_var_id_index;
    }

    if (id >= MAX_VARS) {
        fprintf(stderr, "Error: Too many variables (max %d).\n", MAX_VARS);
        return 0;
    }

    // If this is a new variable, allocate space and update next_available_var_id_index.
    if (id >= next_available_var_id_index) {
        next_available_var_id_index = id + 1;
    }

    // Initialize the variable's entry in the symbol table.
    symbol_table[id].type = var_type;
    symbol_table[id].value = 0; // Default value
    // For type 2 (variable reference), val_ptr_or_data points to its own EvalResult.
    symbol_table[id].val_ptr_or_data = (int)(long)&symbol_table[id];

    return (int)(long)&symbol_table[id]; // Return address of the EvalResult for this var
}

// Forward declaration of the main evaluation function.
int eval_ast_node(AstNode *node, void *context, EvalResult *result_out);

// Helper function to handle binary operations.
// This extracts common logic for `node->type == 2` and various `node->type == 3` subtypes.
static int handle_binary_operation(AstNode *node, void *context, EvalResult *result_out) {
    EvalResult result_left = {0, 1, 0};  // Default type 1, value 0
    EvalResult result_right = {0, 1, 0}; // Default type 1, value 0

    unsigned int node_subtype = node->subtype_or_value_or_varname_offset; // This field is the operator

    // Evaluate left child if it's not an assignment operator (subtype 0).
    // For assignment, the left child (variable name) is handled specially within the switch case.
    if (node_subtype != 0) {
        int res = eval_ast_node(node->child1, context, &result_left);
        if (res < 0) return -1;
    }

    // Evaluate right child for all binary operations.
    int res = eval_ast_node(node->child2, context, &result_right);
    if (res < 0) return -1;

    // Type check for arithmetic operations (operands must be integer literals).
    // This check is skipped for assignment (subtype 0).
    if (node_subtype != 0) {
        if (result_left.type != 1 || result_right.type != 1) {
            return -1; // Type mismatch for arithmetic ops
        }
    }

    switch (node_subtype) {
        case 0: { // Assignment (`var = expr`)
            void *var_identifier_for_assign;
            // Determine the variable identifier from the left child.
            if (node->child1->type == 1) { // Left child is a literal node (e.g., variable name string literal)
                var_identifier_for_assign = (void *)(long)node->child1->subtype_or_value_or_varname_offset;
            } else {
                // Left child is an expression that evaluates to a variable identifier.
                // The original code `local_10 = &local_2c;` suggests passing the `EvalResult` struct itself.
                // Re-evaluate the left child to get its identifier result.
                int res_child1 = eval_ast_node(node->child1, context, &result_left);
                if (res_child1 < 0) return -1;
                var_identifier_for_assign = &result_left; // Pass EvalResult* as identifier
            }

            // Insert/update the variable in the symbol table.
            int inserted_var_addr = insert_variable(context, var_identifier_for_assign, result_right.type);
            if (inserted_var_addr == 0) return -1;

            // Store the value and type of the right-hand side into the variable's EvalResult.
            ((EvalResult *)(long)inserted_var_addr)->value = result_right.value;
            ((EvalResult *)(long)inserted_var_addr)->type = result_right.type;

            // The original code performs a lookup again after insert, possibly to ensure consistency
            // or update an existing variable's type if `insert_variable` only creates new entries.
            int existing_var_addr = lookup_variable(context, var_identifier_for_assign);
            if (existing_var_addr != 0) {
                ((EvalResult *)(long)existing_var_addr)->type = result_right.type;
            }

            // Assignment expression typically evaluates to the value of the right-hand side.
            result_out->val_ptr_or_data = result_right.val_ptr_or_data;
            result_out->type = result_right.type;
            result_out->value = result_right.value;
            return 0;
        }
        case 1: // Addition
            result_out->type = 1;
            result_out->value = result_left.value + result_right.value;
            result_out->val_ptr_or_data = 0;
            return 0;
        case 2: // Subtraction
            result_out->type = 1;
            result_out->value = result_left.value - result_right.value;
            result_out->val_ptr_or_data = 0;
            return 0;
        case 3: // Multiplication
            result_out->type = 1;
            result_out->value = result_left.value * result_right.value;
            result_out->val_ptr_or_data = 0;
            return 0;
        case 4: // Division
            if (result_right.value == 0) {
                return -1; // Division by zero error
            } else {
                result_out->type = 1;
                result_out->value = result_left.value / result_right.value;
                result_out->val_ptr_or_data = 0;
                return 0;
            }
        default:
            return -1; // Unhandled binary operation subtype
    }
}

// Main AST evaluation function.
// `node`: Pointer to the current AST node.
// `context`: Pointer to the execution context (e.g., symbol table).
// `result_out`: Pointer to an EvalResult struct where the evaluation result will be stored.
int eval_ast_node(AstNode *node, void *context, EvalResult *result_out) {
    // `result_temp` is used for intermediate results of unary operations or child evaluations.
    EvalResult result_temp = {0, 1, 0}; // Default type 1, value 0

    unsigned int node_type = node->type;
    // `subtype_or_value_or_varname_offset` is an overloaded field, its meaning depends on `node_type`.
    unsigned int node_subtype = node->subtype_or_value_or_varname_offset;

    switch (node_type) {
        case 0: { // Literal (e.g., an integer constant)
            result_out->type = 1; // Type 1: integer literal
            result_out->value = node_subtype; // The literal value is in the overloaded field.
            result_out->val_ptr_or_data = 0; // Not a pointer.
            return 0;
        }
        case 1: { // Variable lookup (e.g., `x`)
            // The overloaded field `node->subtype_or_value_or_varname_offset` holds the variable identifier.
            int var_addr = lookup_variable(context, (void *)(long)node_subtype);
            if (var_addr == 0) {
                return -1; // Variable not found.
            }
            // `lookup_variable` returns the address of the variable's `EvalResult` in the symbol table.
            EvalResult *var_data = (EvalResult *)(long)var_addr;
            result_out->val_ptr_or_data = var_data->val_ptr_or_data;
            result_out->type = var_data->type;
            result_out->value = var_data->value;
            return 0;
        }
        case 2: { // Generic binary operation (e.g., arithmetic ops like `+`, `-`, `*`, `/`)
            // The `node_type == 2` case directly maps to binary operations, with `node_subtype` as the operator.
            return handle_binary_operation(node, context, result_out);
        }
        case 3: { // Compound expression (e.g., unary ops, dereference, address-of, or other binary ops)
            // This node type uses `node_subtype` to distinguish different operations.
            if (node_subtype == 2) { // Special case: Dereference (e.g., `*ptr`)
                // Evaluate the child node (which should yield a variable reference).
                int res = eval_ast_node(node->child1, context, &result_temp);
                if (res < 0) return -1;
                if (result_temp.type != 2) { // Expected a variable reference (type 2).
                    return -1;
                }
                // `result_temp.val_ptr_or_data` holds the address of the referenced variable's `EvalResult`.
                EvalResult *referenced_var = (EvalResult *)(long)result_temp.val_ptr_or_data;
                result_out->val_ptr_or_data = referenced_var->val_ptr_or_data;
                result_out->type = referenced_var->type;
                result_out->value = referenced_var->value;
                return 0;
            } else if (node_subtype == 0) { // Special case: Unary negation (e.g., `-num`)
                // Evaluate the child node (which should yield an integer literal).
                int res = eval_ast_node(node->child1, context, &result_temp);
                if (res < 0) return -1;
                if (result_temp.type != 1) { // Expected an integer literal (type 1).
                    return -1;
                }
                result_out->type = 1; // Result is an integer literal.
                // The original code `param_3[2] = -(int)local_18;` was ambiguous.
                // Assuming `local_18` would point to `result_temp.val_ptr_or_data`
                // and for type 1, `val_ptr_or_data` is 0.
                // It's much more logical to negate the actual integer value `result_temp.value`.
                result_out->value = -result_temp.value;
                result_out->val_ptr_or_data = 0;
                return 0;
            } else if (node_subtype == 1) { // Special case: Address-of (e.g., `&var`)
                // The child1 node represents the variable name/identifier.
                // Its `subtype_or_value_or_varname_offset` field holds the actual identifier (e.g., an integer ID).
                int var_id = node->child1->subtype_or_value_or_varname_offset;
                int var_addr = lookup_variable(context, (void *)(long)var_id);
                if (var_addr == 0) {
                    return -1; // Variable not found.
                }
                result_out->type = 2; // Type 2: variable reference.
                result_out->value = var_addr; // Store the address/ID of the variable.
                result_out->val_ptr_or_data = var_addr; // Also store the address for dereference.
                return 0;
            } else { // Other subtypes of type 3 are binary operations.
                return handle_binary_operation(node, context, result_out);
            }
        }
        default: // Unhandled or unknown node type.
            return -1;
    }
}

// Function: eval
// This function acts as a wrapper, extracting the root AST node from a container.
// `ast_container_addr`: An integer representing the base address of a container structure.
// `context`: Pointer to the execution context.
// `result_out`: Pointer to an EvalResult struct where the final result will be stored.
void eval(int ast_container_addr, void *context, EvalResult *result_out) {
    // The original snippet `eval_ast_node(*(undefined4 *)(param_1 + 8), ...)`
    // implies that `param_1` is an address, and at `param_1 + 8` there's a pointer to the actual root AstNode.
    AstNode *root_ast_node = *(AstNode **)(long)(ast_container_addr + 8);
    eval_ast_node(root_ast_node, context, result_out);
    return;
}

// --- Example `main` function for compilation and testing ---
int main() {
    // Mock context (e.g., pointer to a global state or symbol table)
    void *mock_context = (void *)0xFEEDFACE;

    // --- Test Case 1: Literal (5) ---
    printf("--- Test Case 1: Literal (5) ---\n");
    AstNode node_literal_5 = { .type = 0, .subtype_or_value_or_varname_offset = 5 };
    EvalResult res1;
    int status = eval_ast_node(&node_literal_5, mock_context, &res1);
    if (status == 0) {
        printf("Result: type=%d, value=%d\n", res1.type, res1.value); // Expected: type=1, value=5
    } else {
        printf("Error evaluating literal.\n");
    }

    // --- Test Case 2: Addition (5 + 3) ---
    printf("\n--- Test Case 2: Addition (5 + 3) ---\n");
    AstNode node_literal_3 = { .type = 0, .subtype_or_value_or_varname_offset = 3 };
    // Type 2, subtype 1 (addition)
    AstNode node_add = { .type = 2, .subtype_or_value_or_varname_offset = 1, .child1 = &node_literal_5, .child2 = &node_literal_3 };
    EvalResult res2;
    status = eval_ast_node(&node_add, mock_context, &res2);
    if (status == 0) {
        printf("Result: type=%d, value=%d\n", res2.type, res2.value); // Expected: type=1, value=8
    } else {
        printf("Error evaluating addition.\n");
    }

    // --- Test Case 3: Assignment (x = 10) ---
    printf("\n--- Test Case 3: Assignment (x = 10) ---\n");
    // Variable 'x' is identified by a mock ID 0x10000000.
    AstNode node_var_x_name = { .type = 1, .subtype_or_value_or_varname_offset = 0x10000000 };
    AstNode node_literal_10 = { .type = 0, .subtype_or_value_or_varname_offset = 10 };
    // Type 3, subtype 0 (assignment)
    AstNode node_assign_x = { .type = 3, .subtype_or_value_or_varname_offset = 0, .child1 = &node_var_x_name, .child2 = &node_literal_10 };
    EvalResult res3;
    status = eval_ast_node(&node_assign_x, mock_context, &res3);
    if (status == 0) {
        printf("Assignment result: type=%d, value=%d\n", res3.type, res3.value); // Expected: type=1, value=10
        // Verify 'x' in symbol table
        EvalResult *x_var = (EvalResult *)(long)lookup_variable(mock_context, (void*)0x10000000);
        if (x_var) printf("Variable 'x' in symbol table: type=%d, value=%d\n", x_var->type, x_var->value); // Expected: type=1, value=10
    } else {
        printf("Error evaluating assignment.\n");
    }

    // --- Test Case 4: Variable lookup (x) ---
    printf("\n--- Test Case 4: Variable lookup (x) ---\n");
    AstNode node_lookup_x = { .type = 1, .subtype_or_value_or_varname_offset = 0x10000000 };
    EvalResult res4;
    status = eval_ast_node(&node_lookup_x, mock_context, &res4);
    if (status == 0) {
        printf("Result: type=%d, value=%d\n", res4.type, res4.value); // Expected: type=1, value=10
    } else {
        printf("Error evaluating variable lookup.\n");
    }

    // --- Test Case 5: Unary Negation (-x) ---
    printf("\n--- Test Case 5: Unary Negation (-x) ---\n");
    // Type 3, subtype 0 (unary negation, child1 is operand)
    AstNode node_negate_x = { .type = 3, .subtype_or_value_or_varname_offset = 0, .child1 = &node_lookup_x };
    EvalResult res5;
    status = eval_ast_node(&node_negate_x, mock_context, &res5);
    if (status == 0) {
        printf("Result: type=%d, value=%d\n", res5.type, res5.value); // Expected: type=1, value=-10
    } else {
        printf("Error evaluating unary negation.\n");
    }

    // --- Test Case 6: Address-of (&x) ---
    printf("\n--- Test Case 6: Address-of (&x) ---\n");
    // Node representing the name 'x' (type 1, with its ID)
    AstNode node_addr_x_name_node = { .type = 1, .subtype_or_value_or_varname_offset = 0x10000000 };
    // Type 3, subtype 1 (address-of, child1 is variable name node)
    AstNode node_addr_of_x = { .type = 3, .subtype_or_value_or_varname_offset = 1, .child1 = &node_addr_x_name_node };
    EvalResult res6;
    status = eval_ast_node(&node_addr_of_x, mock_context, &res6);
    if (status == 0) {
        printf("Result: type=%d, value=0x%x (address of x)\n", res6.type, res6.value); // Expected: type=2, value=address of x
        EvalResult *x_var_addr = (EvalResult *)(long)lookup_variable(mock_context, (void*)0x10000000);
        printf("Expected address of x: 0x%lx\n", (long)x_var_addr);
    } else {
        printf("Error evaluating address-of.\n");
    }

    // --- Test Case 7: Dereference (*(&x)) ---
    printf("\n--- Test Case 7: Dereference (*(&x)) ---\n");
    // Type 3, subtype 2 (dereference, child1 is the pointer expression)
    AstNode node_deref_addr_x = { .type = 3, .subtype_or_value_or_varname_offset = 2, .child1 = &node_addr_of_x };
    EvalResult res7;
    status = eval_ast_node(&node_deref_addr_x, mock_context, &res7);
    if (status == 0) {
        printf("Result: type=%d, value=%d\n", res7.type, res7.value); // Expected: type=1, value=10
    } else {
        printf("Error evaluating dereference.\n");
    }

    // --- Test Case 8: Eval function wrapper (x + 1) ---
    printf("\n--- Test Case 8: Eval function wrapper (x + 1) ---\n");
    AstNode node_literal_1 = { .type = 0, .subtype_or_value_or_varname_offset = 1 };
    // Type 2, subtype 1 (addition)
    AstNode node_add_x_1 = { .type = 2, .subtype_or_value_or_varname_offset = 1, .child1 = &node_lookup_x, .child2 = &node_literal_1 };
    // Create a mock container structure for the `eval` function.
    // As per `*(AstNode **)(long)(ast_container_addr + 8)` in `eval`,
    // the actual AST root pointer is at offset 8 within this container.
    struct EvalContainer {
        int f0;
        int f1;
        AstNode *root_ast; // This will be at offset +8
    } eval_container = {0, 0, &node_add_x_1};
    EvalResult res8;
    eval((int)(long)&eval_container, mock_context, &res8);
    if (res8.type == 1) {
        printf("Eval result: value=%d\n", res8.value); // Expected: 11
    } else {
        printf("Error in eval function wrapper.\n");
    }

    return 0;
}