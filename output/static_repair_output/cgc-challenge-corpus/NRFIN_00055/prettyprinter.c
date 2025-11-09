#include <stdio.h>   // For printf
#include <string.h>  // For memset, strncpy
#include <stdlib.h>  // For malloc (used in main for demonstration)

// --- Type Definitions based on memory access patterns ---

// Token Types (derived from switch cases in pretty_print_tokens)
enum TokenType {
    TOKEN_INT_LITERAL    = 0,
    TOKEN_STRING_LITERAL = 1,
    TOKEN_EQ             = 2,
    TOKEN_PLUS           = 3,
    TOKEN_MINUS          = 4,
    TOKEN_MULTIPLY       = 5,
    TOKEN_DIVIDE         = 6,
    TOKEN_TILDE          = 7,
    TOKEN_AMPERSAND      = 8,
    TOKEN_DOLLAR         = 9,
    TOKEN_LPAREN         = 10,
    TOKEN_RPAREN         = 11
};

// Represents an 8-byte token structure
typedef struct {
    int type; // Offset 0
    union {
        int int_value;     // Offset 4, for integer literals
        char str_value[4]; // Offset 4, for string literals (e.g., variable names)
    } value;
} Token;

// AST Node Types (derived from if/else if conditions in pretty_print_ast_node,
// based on the value at offset 4 within the node structure)
enum AstNodeType {
    AST_INT_LITERAL    = 0,
    AST_STRING_LITERAL = 1, // Represents a variable name
    AST_BINARY_OP      = 2,
    AST_UNARY_OP       = 3
};

// Forward declaration for recursive structure
typedef struct AstNode AstNode;

// Represents an AST node structure
// (param_1 in pretty_print_ast_node points to an AstNode)
struct AstNode {
    // An unused field might exist at offset 0, or param_1 directly points to node_type.
    // Assuming param_1 points to the start of this struct.
    int dummy_field_0;       // Placeholder for potential 4 bytes at offset 0
    unsigned int node_type;  // Offset 4 (corresponds to uVar1 in original)
    union {
        int int_literal;        // Offset 8, for AST_INT_LITERAL
        char str_literal[4];    // Offset 8, for AST_STRING_LITERAL (variable name)
        int operator_idx;       // Offset 8, for AST_BINARY_OP, AST_UNARY_OP (index into operator arrays)
    } data;
    AstNode* left_child;     // Offset 12 (0xc)
    AstNode* right_child;    // Offset 16 (0x10) (only for binary ops)
};

// Top-level AST structure
// (param_1 in pretty_print_ast points to an Ast structure)
typedef struct {
    // ... other fields if any ...
    int dummy_field_0; // Placeholder for potential 8 bytes before root
    int dummy_field_4;
    AstNode* root;     // Offset 8
} Ast;

// --- Global Operator Arrays ---
// These are inferred from the usage in pretty_print_ast_node.
// The content is example data, actual values depend on the grammar.
char unary_operators_1[] = { '~', '-', '!' }; // Example: index 0 for '~', 1 for '-', etc.
char binary_operators_0[] = { '+', '-', '*', '/', '=', '&', '$' }; // Example

// --- Function Implementations ---

// Function: pretty_print_tokens
void pretty_print_tokens(const Token* tokens, unsigned int count) {
    for (unsigned int i = 0; i < count; ++i) {
        const Token* current_token = &tokens[i];
        char buffer[5]; // Buffer for string literals, ensures null-termination

        switch (current_token->type) {
            case TOKEN_INT_LITERAL:
                printf("%d", current_token->value.int_value);
                break;
            case TOKEN_STRING_LITERAL:
                memset(buffer, 0, sizeof(buffer)); // Null-terminate the buffer
                // Copy up to 4 characters from the token's string value
                strncpy(buffer, current_token->value.str_value, 4);
                printf("%s", buffer);
                break;
            case TOKEN_EQ:
                printf("=");
                break;
            case TOKEN_PLUS:
                printf("+");
                break;
            case TOKEN_MINUS:
                printf("-");
                break;
            case TOKEN_MULTIPLY:
                printf("*");
                break;
            case TOKEN_DIVIDE:
                printf("/");
                break;
            case TOKEN_TILDE:
                printf("~");
                break;
            case TOKEN_AMPERSAND:
                printf("&");
                break;
            case TOKEN_DOLLAR:
                printf("$");
                break;
            case TOKEN_LPAREN:
                printf("(");
                break;
            case TOKEN_RPAREN:
                printf(")");
                break;
        }
    }
    printf("\n");
}

// Function: pretty_print_ast_node
void pretty_print_ast_node(const AstNode* node) {
    if (node == NULL) {
        return; // Handle null nodes gracefully
    }

    char buffer[5]; // Buffer for string literals, ensures null-termination

    switch (node->node_type) {
        case AST_UNARY_OP: // Corresponds to uVar1 == 3
            printf("%c", unary_operators_1[node->data.operator_idx]);
            pretty_print_ast_node(node->left_child);
            break;
        case AST_BINARY_OP: // Corresponds to uVar1 == 2
            printf("(");
            pretty_print_ast_node(node->left_child);
            printf("%c", binary_operators_0[node->data.operator_idx]);
            pretty_print_ast_node(node->right_child);
            printf(")");
            break;
        case AST_INT_LITERAL: // Corresponds to uVar1 == 0
            printf("%d", node->data.int_literal);
            break;
        case AST_STRING_LITERAL: // Corresponds to uVar1 == 1
            memset(buffer, 0, sizeof(buffer)); // Null-terminate the buffer
            // Copy up to 4 characters from the node's string literal value
            strncpy(buffer, node->data.str_literal, 4);
            printf("%s", buffer);
            break;
        default:
            // Handle unknown node types if necessary
            fprintf(stderr, "Warning: Unknown AST node type %u\n", node->node_type);
            break;
    }
}

// Function: pretty_print_ast
void pretty_print_ast(const Ast* ast) {
    if (ast == NULL || ast->root == NULL) {
        return; // Handle null AST or root gracefully
    }
    pretty_print_ast_node(ast->root);
    printf("\n");
}

// --- Main function for demonstration ---
int main() {
    printf("--- pretty_print_tokens demonstration ---\n");

    // Example tokens: 10 + var = 20
    Token tokens[] = {
        {TOKEN_INT_LITERAL, .value.int_value = 10},
        {TOKEN_PLUS},
        {TOKEN_STRING_LITERAL, .value.str_value = {'v', 'a', 'r', '\0'}}, // 'var'
        {TOKEN_EQ},
        {TOKEN_INT_LITERAL, .value.int_value = 20},
        {TOKEN_LPAREN},
        {TOKEN_RPAREN}
    };
    pretty_print_tokens(tokens, sizeof(tokens) / sizeof(tokens[0])); // Expected: 10+var=20()

    printf("\n--- pretty_print_ast demonstration ---\n");

    // Create a dummy AST: (10 + var) * -20
    // Nodes are allocated dynamically for flexibility, but could be static.

    // Node: 20 (int literal)
    AstNode* node_20 = (AstNode*)malloc(sizeof(AstNode));
    node_20->dummy_field_0 = 0;
    node_20->node_type = AST_INT_LITERAL;
    node_20->data.int_literal = 20;
    node_20->left_child = NULL;
    node_20->right_child = NULL;

    // Node: - (unary operator, index 1 in unary_operators_1)
    AstNode* node_neg = (AstNode*)malloc(sizeof(AstNode));
    node_neg->dummy_field_0 = 0;
    node_neg->node_type = AST_UNARY_OP;
    node_neg->data.operator_idx = 1; // '-'
    node_neg->left_child = node_20;
    node_neg->right_child = NULL;

    // Node: var (string literal)
    AstNode* node_var = (AstNode*)malloc(sizeof(AstNode));
    node_var->dummy_field_0 = 0;
    node_var->node_type = AST_STRING_LITERAL;
    strncpy(node_var->data.str_literal, "var", 4);
    node_var->left_child = NULL;
    node_var->right_child = NULL;

    // Node: 10 (int literal)
    AstNode* node_10 = (AstNode*)malloc(sizeof(AstNode));
    node_10->dummy_field_0 = 0;
    node_10->node_type = AST_INT_LITERAL;
    node_10->data.int_literal = 10;
    node_10->left_child = NULL;
    node_10->right_child = NULL;

    // Node: + (binary operator, index 0 in binary_operators_0)
    AstNode* node_plus = (AstNode*)malloc(sizeof(AstNode));
    node_plus->dummy_field_0 = 0;
    node_plus->node_type = AST_BINARY_OP;
    node_plus->data.operator_idx = 0; // '+'
    node_plus->left_child = node_10;
    node_plus->right_child = node_var;

    // Node: * (binary operator, index 2 in binary_operators_0)
    AstNode* node_mult = (AstNode*)malloc(sizeof(AstNode));
    node_mult->dummy_field_0 = 0;
    node_mult->node_type = AST_BINARY_OP;
    node_mult->data.operator_idx = 2; // '*'
    node_mult->left_child = node_plus;
    node_mult->right_child = node_neg;

    // Top-level AST structure
    Ast ast_root_obj;
    ast_root_obj.dummy_field_0 = 0;
    ast_root_obj.dummy_field_4 = 0;
    ast_root_obj.root = node_mult;

    pretty_print_ast(&ast_root_obj); // Expected: (10+var)*-20

    // Clean up allocated memory
    free(node_mult);
    free(node_plus);
    free(node_10);
    free(node_var);
    free(node_neg);
    free(node_20);

    return 0;
}