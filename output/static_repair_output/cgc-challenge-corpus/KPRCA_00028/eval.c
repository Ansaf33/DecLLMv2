#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> // For va_list in fdprintf mock

// Type definition for function pointers used by built-in functions
typedef struct SExp* (*builtin_func)(struct SExp*, struct SExp*);

// SExp structure definition, adjusted for 64-bit safety and clarity.
// This structure maps the original 20-byte `undefined4` array access pattern
// to proper C types. The `calloc` calls will be adjusted to `sizeof(SExp)`.
typedef struct SExp {
    unsigned int type; // [0] type (1: atom, 2: builtin, 3: list, 4: lambda)
    char* symbol_name; // [1] For type 1 (atom), holds symbol name (char*).
                       //     For other types, typically NULL.
    builtin_func func_ptr; // [2] For type 2 (builtin), holds function pointer.
                           //     For other types, typically NULL.
    struct SExp* car_or_params; // [3] For type 3 (list), 'car'. For type 4 (lambda), 'params'.
    struct SExp* cdr_or_body;   // [4] For type 3 (list), 'cdr'. For type 4 (lambda), 'body'.
} SExp;

// Global string constants (matching DAT_ addresses from original snippet)
const char DAT_00015004[] = "%s"; // Format string for printing symbols
const char DAT_00015000[] = "("; // Format string for printing list start
const char DAT_00015007[] = " "; // Format string for printing list separator
const char DAT_00015002[] = ")"; // Format string for printing list end
const char DAT_00015009[] = "t"; // Symbol string for true
const char DAT_0001500b[] = "nil"; // Symbol string for false
const char DAT_00015030[] = "car";
const char DAT_00015034[] = "cdr";
const char DAT_00015038[] = "cons";
const char DAT_00015043[] = "atom";
const char DAT_00015048[] = "cond";

// Forward declarations for functions
SExp* parse(char **current_token_ptr_ref, char **out_next_token_str_ptr_ref);
void print(SExp *sexp_node);
SExp* eval(SExp *expr, SExp *sym_list_env);

// --- Mock/Helper Functions (for compilation) ---

// Helper to create a new SExp node, replacing calloc(1, 0x14)
SExp* create_sexp(unsigned int type) {
    SExp* node = (SExp*)calloc(1, sizeof(SExp));
    if (node == NULL) {
        exit(1); // WARNING: Subroutine does not return
    }
    node->type = type;
    return node;
}

// Mock fdprintf: Replaces fdprintf(1, ...) with fprintf(stdout, ...)
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(fd == 1 ? stdout : stderr, format, args);
    va_end(args);
    return ret;
}

// Structure for symbol table nodes (used internally by make_syms and find_sym)
typedef struct SymNode {
    char* name;
    SExp* value;
    struct SymNode* next;
} SymNode;

// Structure to manage a list of symbols
struct SymbolList {
    SymNode* head;
    SymNode* tail;
};

// Appends a symbol-value pair to the symbol list
void sym_list_append(struct SymbolList* list_head, char** sym_pair) {
    SymNode* new_node = (SymNode*)malloc(sizeof(SymNode));
    if (new_node == NULL) {
        exit(1);
    }
    new_node->name = sym_pair[0]; // First element is symbol name
    new_node->value = (SExp*)sym_pair[1]; // Second element is SExp value
    new_node->next = NULL;

    if (list_head->head == NULL) {
        list_head->head = new_node;
        list_head->tail = new_node;
    } else {
        list_head->tail->next = new_node;
        list_head->tail = new_node;
    }
    free(sym_pair); // Free the temporary `sym_pair` array
}

// Structure for expression tuple nodes (used internally by lambda and subst)
typedef struct ExpTupleNode {
    SExp* param_name;
    SExp* param_value;
    struct ExpTupleNode* next;
} ExpTupleNode;

// Structure to manage a list of expression tuples (bindings)
struct ExpTupleList {
    ExpTupleNode* head;
    ExpTupleNode* tail;
};

// Appends an expression tuple (param, value) to the list
void exptup_list_append(struct ExpTupleList* list_head, SExp** exp_pair) {
    ExpTupleNode* new_node = (ExpTupleNode*)malloc(sizeof(ExpTupleNode));
    if (new_node == NULL) {
        exit(1);
    }
    new_node->param_name = exp_pair[0];
    new_node->param_value = exp_pair[1];
    new_node->next = NULL;

    if (list_head->head == NULL) {
        list_head->head = new_node;
        list_head->tail = new_node;
    } else {
        list_head->tail->next = new_node;
        list_head->tail = new_node;
    }
    free(exp_pair); // Free the temporary array
}

// Mock tokenize function for REPL.
// For demonstration, it returns a fixed token stream for `( A B )`.
// In a real application, this would parse an input string.
char** tokenize(const char* input_string) {
    // This is a minimal mock for compilation.
    // In a real Lisp, it would parse `input_string`.
    // For `repl` to call `parse` with `tokens + 0`, this mock creates `{"(", "A", "B", ")", NULL}`
    // or similar.
    const char* example_tokens_str[] = {"(", "A", "B", ")", NULL};
    int count = 0;
    while(example_tokens_str[count] != NULL) count++;

    char** result = (char**)malloc((count + 1) * sizeof(char*));
    if (!result) exit(1);

    for (int i = 0; i < count; ++i) {
        result[i] = strdup(example_tokens_str[i]);
        if (!result[i]) {
            for (int j = 0; j < i; ++j) free(result[j]);
            free(result);
            exit(1);
        }
    }
    result[count] = NULL;
    return result;
}


// --- Original Functions with Fixes ---

// Function: parse
// Parses a sequence of tokens into an S-expression.
// `current_token_ptr_ref`: A pointer to the current token string pointer in the token array.
// `out_next_token_str_ptr_ref`: A pointer to a char* to store the token string pointer
//                                 immediately after the parsed S-expression.
SExp *parse(char **current_token_ptr_ref, char **out_next_token_str_ptr_ref) {
    SExp *result_sexp = NULL;
    char *next_token_str_after_subexpr = NULL; // Temp for recursive calls

    if (current_token_ptr_ref == NULL || *current_token_ptr_ref == NULL) {
        return NULL;
    }

    if (strcmp(*current_token_ptr_ref, "(") == 0) {
        // Handle list: ( CAR CDR )
        // Parse CAR: advance token stream past '('
        SExp *car_sexp = parse(current_token_ptr_ref + 1, &next_token_str_after_subexpr);
        // Parse CDR: start from where CAR parsing left off
        SExp *cdr_sexp = parse(&next_token_str_after_subexpr, &next_token_str_after_subexpr);

        if (out_next_token_str_ptr_ref != NULL) {
            *out_next_token_str_ptr_ref = next_token_str_after_subexpr;
        }

        result_sexp = create_sexp(3); // Type 3 for list
        result_sexp->car_or_params = car_sexp;
        result_sexp->cdr_or_body = cdr_sexp;
    } else if (strcmp(*current_token_ptr_ref, ")") == 0) {
        // Handle end of list ')'
        if (out_next_token_str_ptr_ref != NULL) {
            *out_next_token_str_ptr_ref = *(current_token_ptr_ref + 1); // Advance past ')'
        }
        return NULL; // ')' itself doesn't form an SExp value
    } else {
        // Handle atom/symbol
        result_sexp = create_sexp(1); // Type 1 for atom
        result_sexp->symbol_name = strdup(*current_token_ptr_ref);
        if (result_sexp->symbol_name == NULL) {
            exit(1);
        }

        if (out_next_token_str_ptr_ref != NULL) {
            *out_next_token_str_ptr_ref = *(current_token_ptr_ref + 1); // Advance past the symbol
        }
    }
    return result_sexp;
}

// Function: print
void print(SExp *sexp_node) {
    if (sexp_node == NULL) {
        return;
    }

    if (sexp_node->type == 1) { // Atom/Symbol
        fdprintf(1, DAT_00015004, sexp_node->symbol_name);
    } else { // List or other complex type
        fdprintf(1, DAT_00015000); // Print "("
        SExp* current_list_node = sexp_node;
        while (current_list_node != NULL) {
            print(current_list_node->car_or_params);
            current_list_node = current_list_node->cdr_or_body;
            if (current_list_node != NULL && current_list_node->type == 3) {
                fdprintf(1, DAT_00015007); // Print " " separator
            } else {
                break; // End of proper list or improper list tail
            }
        }
        fdprintf(1, DAT_00015002); // Print ")"
    }
}

// Function: find_sym
// Searches for a symbol in the given symbol list environment.
// Returns the SExp value associated with the symbol, or NULL if not found.
SExp* find_sym(struct SymbolList* sym_list_env, char *symbol_name) {
    if (sym_list_env == NULL || symbol_name == NULL) {
        return NULL;
    }
    SymNode* current = sym_list_env->head;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, symbol_name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

// Function: make_fp
// Creates a symbol-value pair for a built-in function.
// Returns `char**` where `[0]` is symbol name and `[1]` is SExp* of the function.
char **make_fp(char *symbol_name, builtin_func func_ptr) {
    char **sym_pair = (char **)malloc(2 * sizeof(char*));
    if (sym_pair == NULL) {
        exit(1);
    }
    
    sym_pair[0] = strdup(symbol_name);
    if (sym_pair[0] == NULL) {
        free(sym_pair);
        exit(1);
    }

    SExp *func_sexp = create_sexp(2); // Type 2 for built-in function
    func_sexp->func_ptr = func_ptr;
    
    sym_pair[1] = (char*)func_sexp; // Store SExp* as char* for `sym_list_append`
    return sym_pair;
}

// Function: quote_fn
SExp* quote_fn(SExp* args_list, SExp* sym_list_env) {
    (void)sym_list_env; // Unused parameter
    if (args_list == NULL) {
        return NULL;
    }
    return args_list->car_or_params; // Returns the first argument as is
}

// Function: car_fn
SExp* car_fn(SExp* args_list, SExp* sym_list_env) {
    (void)sym_list_env; // Unused parameter
    if (args_list == NULL || args_list->car_or_params == NULL || args_list->car_or_params->type != 3) {
        return NULL;
    }
    return args_list->car_or_params->car_or_params; // Returns the car of the first argument
}

// Function: cdr_fn
SExp* cdr_fn(SExp* args_list, SExp* sym_list_env) {
    (void)sym_list_env; // Unused parameter
    if (args_list == NULL || args_list->car_or_params == NULL || args_list->car_or_params->type != 3) {
        return NULL;
    }
    return args_list->car_or_params->cdr_or_body; // Returns the cdr of the first argument
}

// Function: cons_fn (behaves like append in original code)
SExp* cons_fn(SExp* args_list, SExp* sym_list_env) {
    (void)sym_list_env; // Unused parameter
    if (args_list == NULL || args_list->car_or_params == NULL || args_list->cdr_or_body == NULL) {
        return NULL;
    }
    SExp* list1 = args_list->car_or_params;
    SExp* list2 = args_list->cdr_or_body->car_or_params; // Second argument (the list to append to)

    if (list1 == NULL) return list2;
    if (list2 == NULL) return list1;

    SExp* new_list_head = NULL;
    SExp* current_new_list_node = NULL;
    SExp* current_list1_node = list1;

    // Deep copy list1
    while (current_list1_node != NULL && current_list1_node->type == 3) {
        SExp* new_node = create_sexp(3);
        new_node->car_or_params = current_list1_node->car_or_params; // Shallow copy of car content
        new_node->cdr_or_body = NULL;

        if (new_list_head == NULL) {
            new_list_head = new_node;
            current_new_list_node = new_node;
        } else {
            current_new_list_node->cdr_or_body = new_node;
            current_new_list_node = new_node;
        }
        current_list1_node = current_list1_node->cdr_or_body;
    }

    // Append list2 to the end of the copied list1
    if (current_new_list_node != NULL) {
        current_new_list_node->cdr_or_body = list2;
    } else { // list1 was not a list (e.g., atom) or empty, then result is just list2
        new_list_head = list2;
    }

    return new_list_head;
}

// Function: equal_fn
SExp* equal_fn(SExp* args_list, SExp* sym_list_env) {
    if (args_list == NULL || args_list->car_or_params == NULL || args_list->cdr_or_body == NULL || args_list->cdr_or_body->car_or_params == NULL) {
        return NULL; // Not enough arguments
    }
    SExp* arg1 = args_list->car_or_params;
    SExp* arg2 = args_list->cdr_or_body->car_or_params;

    if (arg1->type == 1 && arg2->type == 1 && strcmp(arg1->symbol_name, arg2->symbol_name) == 0) {
        return find_sym(sym_list_env, (char*)DAT_00015009); // Return 't'
    } else {
        return find_sym(sym_list_env, (char*)DAT_0001500b); // Return 'nil'
    }
}

// Function: atom_fn
SExp* atom_fn(SExp* args_list, SExp* sym_list_env) {
    if (args_list == NULL || args_list->car_or_params == NULL) {
        return NULL;
    }
    SExp* arg = args_list->car_or_params;

    if (arg->type == 1) { // It's an atom
        return find_sym(sym_list_env, (char*)DAT_00015009); // Return 't'
    } else {
        return find_sym(sym_list_env, (char*)DAT_0001500b); // Return 'nil'
    }
}

// Function: cond_fn
SExp* cond_fn(SExp* args_list, SExp* sym_list_env) {
    if (args_list == NULL) {
        return NULL;
    }
    SExp* current_clause = args_list;
    while (current_clause != NULL) {
        if (current_clause->type != 3 || current_clause->car_or_params == NULL || current_clause->car_or_params->type != 3 ||
            current_clause->car_or_params->car_or_params == NULL || current_clause->car_or_params->cdr_or_body == NULL ||
            current_clause->car_or_params->cdr_or_body->car_or_params == NULL) {
            return NULL; // Invalid clause format
        }
        
        SExp* predicate = current_clause->car_or_params->car_or_params;
        SExp* consequence = current_clause->car_or_params->cdr_or_body->car_or_params;

        SExp* evaluated_predicate = eval(predicate, sym_list_env);
        SExp* true_sym = find_sym(sym_list_env, (char*)DAT_00015009);

        if (evaluated_predicate == true_sym) {
            return eval(consequence, sym_list_env);
        }
        current_clause = current_clause->cdr_or_body;
    }
    return NULL; // No condition met
}

// Function: get
// Retrieves the Nth element of a list.
SExp* get(int index, SExp* list_sexp) {
    SExp* current_node = list_sexp;
    while (index > 0 && current_node != NULL && current_node->type == 3) {
        current_node = current_node->cdr_or_body;
        index--;
    }
    if (current_node == NULL || current_node->type != 3) {
        return NULL; // Index out of bounds or not a list node
    }
    return current_node->car_or_params;
}

// Function: subst
// Substitutes variables in an expression based on a list of bindings.
// Performs a deep copy of lists and substitutes atoms.
SExp* subst(ExpTupleNode* bindings, SExp* expr) {
    if (expr == NULL) {
        return NULL;
    }

    if (expr->type == 1) { // If it's an atom/symbol
        ExpTupleNode* current_binding = bindings;
        while (current_binding != NULL) {
            if (current_binding->param_name->type == 1 && 
                strcmp(expr->symbol_name, current_binding->param_name->symbol_name) == 0) {
                return current_binding->param_value;
            }
            current_binding = current_binding->next;
        }
        return expr; // If not found, return the original symbol
    } else if (expr->type == 3) { // If it's a list, recursively substitute car and cdr
        SExp* new_list_head = NULL;
        SExp* current_new_list_node = NULL;
        
        SExp* current_expr_node = expr;
        while (current_expr_node != NULL && current_expr_node->type == 3) {
            SExp* substituted_car = subst(bindings, current_expr_node->car_or_params);
            
            SExp* new_node = create_sexp(3);
            new_node->car_or_params = substituted_car;
            new_node->cdr_or_body = NULL;

            if (new_list_head == NULL) {
                new_list_head = new_node;
                current_new_list_node = new_node;
            } else {
                current_new_list_node->cdr_or_body = new_node;
                current_new_list_node = new_node;
            }
            current_expr_node = current_expr_node->cdr_or_body;
        }
        // If the original expression ended with a non-list SExp (e.g., a symbol), append it.
        if (current_expr_node != NULL) {
            SExp* substituted_tail = subst(bindings, current_expr_node);
            if (current_new_list_node != NULL) {
                current_new_list_node->cdr_or_body = substituted_tail;
            } else { // This case happens if the original expr was just an atom (not a list)
                new_list_head = substituted_tail;
            }
        }
        return new_list_head;
    }
    return expr; // For other types, return as is (e.g., builtin, lambda)
}

// Function: lambda
// Evaluates a lambda expression.
SExp* lambda(SExp* lambda_sexp, SExp* args_list, SExp* sym_list_env) {
    if (lambda_sexp == NULL || args_list == NULL || sym_list_env == NULL) {
        return NULL;
    }

    struct ExpTupleList bindings_list = {NULL, NULL};
    int i = 0;
    while (1) {
        SExp* param_name = get(i, lambda_sexp->car_or_params); // lambda_sexp->car_or_params is the param list
        SExp* arg_value = get(i, args_list);

        if (param_name == NULL && arg_value == NULL) {
            break; // No more parameters and no more arguments
        }
        if (param_name == NULL || arg_value == NULL) { // Mismatch in number of args/params
            return NULL; // Error in argument count
        }

        SExp** binding_pair = (SExp**)malloc(2 * sizeof(SExp*));
        if (binding_pair == NULL) exit(1);
        binding_pair[0] = param_name;
        binding_pair[1] = arg_value;
        exptup_list_append(&bindings_list, binding_pair);
        i++;
    }

    SExp* substituted_body = subst(bindings_list.head, lambda_sexp->cdr_or_body); // lambda_sexp->cdr_or_body is the body
    SExp* eval_result = eval(substituted_body, sym_list_env);

    // CAKE logic (based on original `goto LAB_00011d13` section)
    unsigned int cake_count = 0;
    SExp* current_cake_check_node = eval_result;
    SExp* final_eval_result = eval_result;

    // Iterate through the list to check for "CAKE" sequence
    SExp* prev_cake_node = NULL; // Keep track of the node before current_cake_check_node
    while (current_cake_check_node != NULL && current_cake_check_node->type == 3) {
        SExp* car_content = current_cake_check_node->car_or_params;
        if (car_content != NULL && car_content->type == 1 && strcmp(car_content->symbol_name, "CAKE") == 0) {
            cake_count++;
            if (cake_count >= 4) { // Found 4 "CAKE"s
                SExp* new_sym_sexp = create_sexp(1);
                new_sym_sexp->symbol_name = strdup("That's a lot of CAKE!");
                if (new_sym_sexp->symbol_name == NULL) exit(1);

                SExp* new_list_sexp = create_sexp(3);
                new_list_sexp->car_or_params = new_sym_sexp;
                new_list_sexp->cdr_or_body = NULL;

                // The original code `*(undefined4 **)(*(int *)(local_18 + 0x10) + 0x10) = puVar6;`
                // is ambiguous. A common interpretation is to replace the remainder of the list
                // from the point where the 4th "CAKE" was found.
                // If `current_cake_check_node` is the 4th "CAKE" node, its `cdr_or_body` should be replaced.
                current_cake_check_node->cdr_or_body = new_list_sexp;
                break; // Stop processing after modification
            }
        } else {
            // Not a "CAKE" or not a list node, stop checking sequence
            break;
        }
        prev_cake_node = current_cake_check_node;
        current_cake_check_node = current_cake_check_node->cdr_or_body;
    }
    
    // TODO: Free bindings_list (not implemented for brevity, but crucial for memory management)
    return final_eval_result;
}

// Function: make_syms
// Initializes the global symbol table with built-in functions and constants.
// Returns the head of the symbol list.
SExp* make_syms(void) {
    struct SymbolList sym_list = {NULL, NULL};

    sym_list_append(&sym_list, make_fp("quote", quote_fn));
    sym_list_append(&sym_list, make_fp((char*)DAT_00015030, car_fn)); // "car"
    sym_list_append(&sym_list, make_fp((char*)DAT_00015034, cdr_fn)); // "cdr"
    sym_list_append(&sym_list, make_fp((char*)DAT_00015038, cons_fn)); // "cons"
    sym_list_append(&sym_list, make_fp("equal", equal_fn));
    sym_list_append(&sym_list, make_fp((char*)DAT_00015043, atom_fn)); // "atom"
    sym_list_append(&sym_list, make_fp((char*)DAT_00015048, cond_fn)); // "cond"

    // Add "nil" symbol
    char **nil_sym_pair = (char **)malloc(2 * sizeof(char*));
    if (nil_sym_pair == NULL) exit(1);
    nil_sym_pair[0] = strdup("nil");
    if (nil_sym_pair[0] == NULL) { free(nil_sym_pair); exit(1); }
    SExp *nil_sexp = create_sexp(3); // Type 3 for list, represents empty list
    nil_sexp->car_or_params = NULL;
    nil_sexp->cdr_or_body = NULL;
    nil_sym_pair[1] = (char*)nil_sexp;
    sym_list_append(&sym_list, nil_sym_pair);

    // Add "t" symbol
    char **t_sym_pair = (char **)malloc(2 * sizeof(char*));
    if (t_sym_pair == NULL) exit(1);
    t_sym_pair[0] = strdup("t");
    if (t_sym_pair[0] == NULL) { free(t_sym_pair); exit(1); }
    SExp *t_sexp = create_sexp(1); // Type 1 for atom
    t_sexp->symbol_name = strdup("t");
    if (t_sexp->symbol_name == NULL) { free(t_sym_pair[0]); free(t_sym_pair); exit(1); }
    t_sym_pair[1] = (char*)t_sexp;
    sym_list_append(&sym_list, t_sym_pair);

    return (SExp*)sym_list.head; // Return the head of the symbol list
}

// Function: eval
// Evaluates an S-expression in the given symbol environment.
SExp *eval(SExp *expr, SExp *sym_list_env) {
    if (expr == NULL) {
        return NULL;
    }

    if (expr->type == 1) { // Atom/Symbol
        SExp* found_sym = find_sym((struct SymbolList*)sym_list_env, expr->symbol_name);
        return (found_sym != NULL) ? found_sym : expr; // Return value if found, else the symbol itself
    } else if (expr->type == 3) { // List (function call or special form)
        if (expr->car_or_params == NULL) { // Empty list or just a tail
            return NULL;
        }

        // Special form: lambda definition
        if (expr->car_or_params->type == 1 && strcmp(expr->car_or_params->symbol_name, "lambda") == 0) {
            // (lambda (params) body)
            if (expr->cdr_or_body == NULL || expr->cdr_or_body->type != 3 ||
                expr->cdr_or_body->car_or_params == NULL || expr->cdr_or_body->cdr_or_body == NULL) {
                return NULL; // Invalid lambda syntax
            }
            SExp* lambda_params = expr->cdr_or_body->car_or_params;
            SExp* lambda_body = expr->cdr_or_body->cdr_or_body;

            SExp* lambda_sexp = create_sexp(4); // Type 4 for lambda
            lambda_sexp->car_or_params = lambda_params;
            lambda_sexp->cdr_or_body = lambda_body;
            return lambda_sexp;
        }

        // Evaluate the function (first element of the list)
        SExp* func_sexp = eval(expr->car_or_params, sym_list_env);
        if (func_sexp == NULL) {
            return NULL;
        }

        // Evaluate arguments (rest of the list)
        SExp* evaluated_args_head = NULL;
        SExp* current_eval_arg_node = NULL;
        SExp* current_arg_node = expr->cdr_or_body;

        while (current_arg_node != NULL && current_arg_node->type == 3) {
            SExp* eval_arg = eval(current_arg_node->car_or_params, sym_list_env);
            SExp* new_arg_node = create_sexp(3);
            new_arg_node->car_or_params = eval_arg;
            new_arg_node->cdr_or_body = NULL;

            if (evaluated_args_head == NULL) {
                evaluated_args_head = new_arg_node;
                current_eval_arg_node = new_arg_node;
            } else {
                current_eval_arg_node->cdr_or_body = new_arg_node;
                current_eval_arg_node = new_arg_node;
            }
            current_arg_node = current_arg_node->cdr_or_body;
        }

        // Apply the function
        if (func_sexp->type == 4) { // Lambda function
            return lambda(func_sexp, evaluated_args_head, sym_list_env);
        } else if (func_sexp->type == 2) { // Built-in function
            return func_sexp->func_ptr(evaluated_args_head, (struct SymbolList*)sym_list_env);
        } else {
            return NULL; // Not a callable function
        }
    }
    return expr; // For other types (e.g., already evaluated built-in/lambda objects), return as is.
}

// Function: repl
// Read-Eval-Print Loop.
int repl(const char* input_string) {
    char **tokens = tokenize(input_string);
    if (tokens == NULL || *tokens == NULL) {
        if (tokens) free(tokens);
        return -1;
    }

    if (strcmp(*tokens, "(") == 0) {
        char **current_token_stream_ptr = tokens;
        SExp *parsed_sexp = parse(current_token_stream_ptr, &current_token_stream_ptr);

        SExp *symbol_env_list_head = make_syms();
        SExp *eval_result = eval(parsed_sexp, symbol_env_list_head);
        
        if (eval_result == NULL) {
            // TODO: Free tokens, parsed_sexp, symbol_env_list_head for proper memory management
            return -1;
        }
        print(eval_result);
        fprintf(stdout, "\n"); // Add newline for readability

        // Free tokens
        char **temp_tokens = tokens;
        while (*temp_tokens) {
            free(*temp_tokens);
            temp_tokens++;
        }
        free(tokens);

        // TODO: Free parsed_sexp and symbol_env_list_head recursively
        return 0;
    }

    // Free tokens if the initial check `strcmp(*tokens, "(") == 0` failed
    char **temp_tokens = tokens;
    while (*temp_tokens) {
        free(*temp_tokens);
        temp_tokens++;
    }
    free(tokens);
    return -1;
}

// Main function (example usage)
int main() {
    // Example usage of REPL
    // The tokenize mock will provide "( A B )"
    // So `parse` will parse `(A B)`.
    // `eval` will try to evaluate `A` as a function with `B` as argument.
    // Since `A` is not a defined function, it will likely return NULL or an error.
    // For a working example, you might need to manually set up `tokenize` to produce
    // something like `(cons (quote A) (quote (B C)))` or `(car (quote (A B)))`.
    printf("--- Lisp REPL Simulation ---\n");
    printf("Input: ( A B ) (Mocked input)\n");
    int result = repl("( A B )"); // The input string is mocked by `tokenize`
    if (result == 0) {
        printf("REPL executed successfully.\n");
    } else {
        printf("REPL failed.\n");
    }

    printf("\nInput: (car (quote (X Y)))\n");
    // To make this work, `tokenize` needs to be more sophisticated or manually set for this test.
    // For now, it will still use the `(A B)` mock.
    // Let's create a direct test.
    char* test_tokens[] = {"(", "car", "(", "quote", "(", "X", "Y", ")", ")", ")", NULL};
    char** test_token_stream = (char**)malloc(sizeof(test_tokens));
    for(int i = 0; test_tokens[i] != NULL; ++i) test_token_stream[i] = strdup(test_tokens[i]);
    test_token_stream[sizeof(test_tokens)/sizeof(char*)-1] = NULL;

    char **current_test_token_ptr = test_token_stream;
    SExp *test_parsed_sexp = parse(current_test_token_ptr, &current_test_token_ptr);
    SExp *test_symbol_env = make_syms();
    SExp *test_eval_result = eval(test_parsed_sexp, test_symbol_env);

    if (test_eval_result != NULL) {
        printf("Result: ");
        print(test_eval_result);
        printf("\n");
    } else {
        printf("Test evaluation failed.\n");
    }
    
    // Free test tokens
    char **temp_test_tokens = test_token_stream;
    while (*temp_test_tokens) {
        free(*temp_test_tokens);
        temp_test_tokens++;
    }
    free(test_token_stream);

    // TODO: Implement SExp freeing for `test_parsed_sexp` and `test_symbol_env`

    return 0;
}