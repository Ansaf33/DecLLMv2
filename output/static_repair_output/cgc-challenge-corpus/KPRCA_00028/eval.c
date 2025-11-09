#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For true/false, though int 0/1 is common in C

// Define types for better readability and 64-bit compatibility
enum NodeType {
    NODE_INVALID = 0,
    NODE_SYMBOL = 1,
    NODE_FUNCTION = 2, // Built-in function
    NODE_LIST = 3,
    NODE_LAMBDA = 4
};

// Forward declarations for Node and SymEntry
typedef struct Node Node;
typedef struct SymEntry SymEntry;

// Function pointer type for built-in functions
typedef Node *(*BuiltinFunc)(Node *args, SymEntry *env);

// Node structure (designed for 64-bit alignment and logical field access)
// Total size 32 bytes: int (4) + padding (4) + union (8) + Node* (8) + Node* (8)
struct Node {
    int type; // Offset 0
    char _padding_type_val[4]; // Padding to align val_union to 8 bytes
    union {
        char *symbol_val; // For NODE_SYMBOL
        BuiltinFunc func_ptr; // For NODE_FUNCTION
    } val_union; // Offset 8
    Node *car; // For NODE_LIST, NODE_LAMBDA (parameters). Offset 16.
    Node *cdr; // For NODE_LIST, NODE_LAMBDA (body). Offset 24.
};

// Symbol table entry structure (24 bytes for 64-bit alignment)
struct SymEntry {
    char *name; // Offset 0
    Node *value; // Offset 8 (the actual value associated with the symbol)
    SymEntry *next; // Offset 16 (pointer to the next symbol in the list)
};

// Global string literals (from DAT_ addresses in original snippet)
static const char * const FORMAT_STRING_S = "%s";
static const char * const PAREN_OPEN = "(";
static const char * const PAREN_CLOSE = ")";
static const char * const SPACE = " ";
static const char * const SYMBOL_T_STR = "t";
static const char * const SYMBOL_NIL_STR = "nil";
static const char * const SYMBOL_CAR_STR = "car";
static const char * const SYMBOL_CDR_STR = "cdr";
static const char * const SYMBOL_CONS_STR = "cons";
static const char * const SYMBOL_ATOM_STR = "atom";
static const char * const SYMBOL_COND_STR = "cond";
static const char * const SYMBOL_QUOTE_STR = "quote";
static const char * const SYMBOL_LAMBDA_STR = "lambda";
static const char * const CAKE_STR = "CAKE";
static const char * const CAKE_MESSAGE = "That's a lot of CAKE!";

// Placeholder for external tokenize function.
// A real Lisp would have a proper lexer. This dummy one leaks memory from strdup.
// It splits input by spaces and treats parentheses as individual tokens.
char **tokenize(char *input) {
    static char *dummy_tokens[100]; // Static buffer for simplicity, not thread-safe.
    static char buffer[1024]; // Static buffer for input copy
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    int token_idx = 0;
    char *token_start = buffer;
    char *p = buffer;

    while (*p != '\0' && token_idx < 99) {
        if (*p == '(' || *p == ')') {
            if (p > token_start && *token_start != ' ' && *token_start != '\t' && *token_start != '\n') {
                *p = '\0'; // Null-terminate previous token
                dummy_tokens[token_idx++] = strdup(token_start);
            }
            if (token_idx < 99) {
                char paren_str[2];
                paren_str[0] = *p;
                paren_str[1] = '\0';
                dummy_tokens[token_idx++] = strdup(paren_str);
            }
            token_start = p + 1;
        } else if (*p == ' ' || *p == '\t' || *p == '\n') {
            if (p > token_start && *token_start != ' ' && *token_start != '\t' && *token_start != '\n') {
                *p = '\0'; // Null-terminate token
                dummy_tokens[token_idx++] = strdup(token_start);
            }
            token_start = p + 1;
        }
        p++;
    }
    if (token_idx < 99 && p > token_start && *token_start != ' ' && *token_start != '\t' && *token_start != '\n') {
        // Handle last token if not followed by space/paren
        *p = '\0';
        dummy_tokens[token_idx++] = strdup(token_start);
    }
    dummy_tokens[token_idx] = NULL; // Null-terminate the token array
    return dummy_tokens;
}


// Generic list entry for the `exptup` list in `lambda`
typedef struct GenericListEntry {
    void *data;
    struct GenericListEntry *next;
} GenericListEntry;

// Container for the generic list (used in lambda to pass head pointer)
typedef struct ExpTupleList {
    GenericListEntry *head;
} ExpTupleList;

// Forward declarations for functions defined in the snippet
Node *parse(char **current_token_ptr_ref, char **next_unconsumed_token_out_ref);
void print(Node *node);
SymEntry *make_fp(char *name, BuiltinFunc func);
Node *eval(Node *node, SymEntry *env);
Node *find_sym(SymEntry *env, char *name); // Returns Node* (the value)
SymEntry *make_syms(void); // Returns the head of the global symbol list
Node *lambda(Node *lambda_node, Node *args_list, SymEntry *env);


// Helper to create a new Node
Node *create_node(int type) {
    Node *node = (Node *)calloc(1, sizeof(Node));
    if (node == NULL) {
        perror("calloc failed");
        exit(1);
    }
    node->type = type;
    return node; // calloc initializes all members to 0/NULL
}

// Helper to create a symbol node
Node *create_symbol_node(char *val_str) {
    Node *node = create_node(NODE_SYMBOL);
    node->val_union.symbol_val = val_str; // Assumes val_str is a static string or owned elsewhere
    return node;
}

// Helper to create a list node
Node *create_list_node(Node *car, Node *cdr) {
    Node *node = create_node(NODE_LIST);
    node->car = car;
    node->cdr = cdr;
    return node;
}

// Helper to create a function node
Node *create_function_node(BuiltinFunc func_ptr) {
    Node *node = create_node(NODE_FUNCTION);
    node->val_union.func_ptr = func_ptr;
    return node;
}

// Helper to create a lambda node
Node *create_lambda_node(Node *params, Node *body) {
    Node *node = create_node(NODE_LAMBDA);
    node->car = params; // Parameters list is stored in car
    node->cdr = body;   // Body expression is stored in cdr
    return node;
}

// Simple list append for SymEntry
void sym_list_append_impl(SymEntry **head, SymEntry *new_entry) {
    if (!new_entry) return;
    new_entry->next = NULL; 
    if (!*head) {
        *head = new_entry;
    } else {
        SymEntry *current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_entry;
    }
}

// Generic list append for void* (used by lambda for exptup list)
void generic_list_append_impl(GenericListEntry **head, void *data) {
    GenericListEntry *new_entry = (GenericListEntry *)malloc(sizeof(GenericListEntry));
    if (!new_entry) {
        perror("malloc failed");
        exit(1);
    }
    new_entry->data = data;
    new_entry->next = NULL;

    if (!*head) {
        *head = new_entry;
    } else {
        GenericListEntry *current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_entry;
    }
}

// Helper for exptup_list_append used in lambda
void exptup_list_append_impl(ExpTupleList *list_container, void *data) {
    if (!list_container) {
        fprintf(stderr, "Error: ExpTupleList container is NULL.\n");
        exit(1);
    }
    generic_list_append_impl(&(list_container->head), data);
}


// Function: parse
// param_1: A pointer to the current token string (char **).
//          E.g., if tokens are {"(", "a", "b", ")", "c"}, param_1 could be &tokens[0].
//          *param_1 would be "(".
// param_2: A pointer to a char* which will be updated to point to the next unconsumed token.
//          E.g., if parse consumes "( a b )", *param_2 will be updated to point to "c".
Node * parse(char **current_token_ptr_ref, char **next_unconsumed_token_out_ref) {
  if (current_token_ptr_ref == NULL || *current_token_ptr_ref == NULL) {
    return NULL;
  }

  char *current_token_str = *current_token_ptr_ref;
  
  if (strcmp(current_token_str, PAREN_OPEN) == 0) {
    // Consume "("
    char **car_token_ptr_ref = current_token_ptr_ref + 1; // Point to token after "("
    Node *car_node = parse(car_token_ptr_ref, car_token_ptr_ref); // Parse CAR, update car_token_ptr_ref to next
    
    Node *cdr_node = parse(car_token_ptr_ref, car_token_ptr_ref); // Parse CDR, update car_token_ptr_ref to next

    if (next_unconsumed_token_out_ref != NULL) {
        *next_unconsumed_token_out_ref = *car_token_ptr_ref; // Update caller's pointer to the token after this list
    }
    return create_list_node(car_node, cdr_node);
  } else if (strcmp(current_token_str, PAREN_CLOSE) == 0) {
    // Consume ")"
    if (next_unconsumed_token_out_ref != NULL) {
        *next_unconsumed_token_out_ref = *(current_token_ptr_ref + 1); // Update caller's pointer to token after ")"
    }
    return NULL; // End of list
  } else { // It's a symbol
    Node *symbol_node = create_symbol_node(current_token_str);
    
    char **cdr_token_ptr_ref = current_token_ptr_ref + 1; // Point to token after the symbol
    Node *cdr_node = parse(cdr_token_ptr_ref, cdr_token_ptr_ref); // Parse rest of list, update cdr_token_ptr_ref to next

    if (next_unconsumed_token_out_ref != NULL) {
        *next_unconsumed_token_out_ref = *cdr_token_ptr_ref; // Update caller's pointer
    }
    return create_list_node(symbol_node, cdr_node);
  }
}

// Function: print
void print(Node *node) {
  if (node == NULL) {
    return;
  }

  if (node->type == NODE_SYMBOL) {
    printf(FORMAT_STRING_S, node->val_union.symbol_val);
  } else if (node->type == NODE_LIST) {
    printf(PAREN_OPEN);
    Node *current_list_element = node;
    bool first_element = true;
    while (current_list_element != NULL) {
        if (!first_element) {
            printf(SPACE);
        }
        print(current_list_element->car); // Print the CAR
        
        current_list_element = current_list_element->cdr; // Move to the CDR
        
        // If there's a next element and it's not a list (improper list tail)
        if (current_list_element != NULL && current_list_element->type != NODE_LIST) {
            printf(" . "); // Print dot for improper list
            print(current_list_element); // Print the non-list tail
            break; // Stop iteration after printing the tail
        }
        first_element = false;
    }
    printf(PAREN_CLOSE);
  }
  // NODE_FUNCTION and NODE_LAMBDA types are not meant for direct printing.
}

// Function: find_sym (returns the Node* value associated with the symbol)
Node *find_sym(SymEntry *env, char *name) {
  if (env == NULL || name == NULL) {
    return NULL;
  }

  SymEntry *current_sym = env;
  while (current_sym != NULL) {
    if (current_sym->name != NULL && strcmp(current_sym->name, name) == 0) {
      return current_sym->value;
    }
    current_sym = current_sym->next;
  }
  return NULL;
}

// Function: make_fp
// Creates a new SymEntry for a built-in function and returns it.
SymEntry * make_fp(char *name, BuiltinFunc func_ptr) {
  SymEntry *new_sym_entry = (SymEntry *)malloc(sizeof(SymEntry));
  if (new_sym_entry == NULL) {
    perror("malloc failed for SymEntry");
    exit(1);
  }

  new_sym_entry->name = strdup(name);
  if (new_sym_entry->name == NULL) {
      perror("strdup failed for function name");
      exit(1);
  }

  Node *func_node = create_function_node(func_ptr);
  
  new_sym_entry->value = func_node;
  new_sym_entry->next = NULL;

  return new_sym_entry;
}

// Built-in functions must have signature: Node *(*BuiltinFunc)(Node *args, SymEntry *env)

// Function: quote_fn
Node *quote_fn(Node *args, SymEntry *env) {
  if (args == NULL || args->type != NODE_LIST) {
    return NULL;
  }
  return args->car; // Return the first argument directly
}

// Function: car_fn
Node *car_fn(Node *args, SymEntry *env) {
  if (args == NULL || args->type != NODE_LIST) {
    return NULL;
  }
  Node *list_arg = eval(args->car, env);
  if (list_arg == NULL || list_arg->type != NODE_LIST) {
    return NULL;
  }
  return list_arg->car;
}

// Function: cdr_fn
Node *cdr_fn(Node *args, SymEntry *env) {
  if (args == NULL || args->type != NODE_LIST) {
    return NULL;
  }
  Node *list_arg = eval(args->car, env);
  if (list_arg == NULL || list_arg->type != NODE_LIST) {
    return NULL;
  }
  return list_arg->cdr;
}

// Function: cons_fn
Node *cons_fn(Node *args, SymEntry *env) {
  if (args == NULL || args->type != NODE_LIST) {
    return NULL;
  }
  
  Node *evaluated_elem = eval(args->car, env);
  Node *evaluated_list = NULL;
  if (args->cdr != NULL && args->cdr->type == NODE_LIST) {
      evaluated_list = eval(args->cdr->car, env);
  }

  // Create a new list node: (evaluated_elem . evaluated_list)
  return create_list_node(evaluated_elem, evaluated_list);
}

// Function: equal_fn
Node *equal_fn(Node *args, SymEntry *env) {
  if (args == NULL || args->type != NODE_LIST || args->cdr == NULL || args->cdr->type != NODE_LIST) {
    return find_sym(env, SYMBOL_NIL_STR);
  }

  Node *arg1 = eval(args->car, env);
  Node *arg2 = eval(args->cdr->car, env);

  if (arg1 == NULL || arg2 == NULL || arg1->type != NODE_SYMBOL || arg2->type != NODE_SYMBOL) {
    return find_sym(env, SYMBOL_NIL_STR);
  }

  if (strcmp(arg1->val_union.symbol_val, arg2->val_union.symbol_val) == 0) {
    return find_sym(env, SYMBOL_T_STR);
  } else {
    return find_sym(env, SYMBOL_NIL_STR);
  }
}

// Function: atom_fn
Node *atom_fn(Node *args, SymEntry *env) {
  if (args == NULL || args->type != NODE_LIST) {
    return find_sym(env, SYMBOL_NIL_STR);
  }

  Node *arg = eval(args->car, env);

  if (arg == NULL || arg == find_sym(env, SYMBOL_NIL_STR)) { // nil is considered an atom in some Lisps
      return find_sym(env, SYMBOL_T_STR);
  }
  if (arg->type == NODE_SYMBOL) {
    return find_sym(env, SYMBOL_T_STR);
  } else {
    return find_sym(env, SYMBOL_NIL_STR);
  }
}

// Function: cond_fn
Node *cond_fn(Node *args, SymEntry *env) {
  Node *current_clause = args;
  while (current_clause != NULL && current_clause->type == NODE_LIST) {
    Node *clause_pair = current_clause->car; // (test expr)
    if (clause_pair == NULL || clause_pair->type != NODE_LIST || clause_pair->cdr == NULL || clause_pair->cdr->type != NODE_LIST) {
      return NULL; // Malformed clause
    }

    Node *test_expr = clause_pair->car;
    Node *result_expr = clause_pair->cdr->car;

    Node *test_result = eval(test_expr, env);
    Node *t_sym_node = find_sym(env, SYMBOL_T_STR);

    if (test_result == t_sym_node) { // Pointer comparison is fine if 't' is a singleton
      return eval(result_expr, env);
    }
    current_clause = current_clause->cdr;
  }
  return NULL; // No condition met
}

// Function: get (Helper for lambda and subst)
// get(N, list) returns the N-th element of the list (0-indexed).
Node *get(int index, Node *list_node) {
  Node *current = list_node;
  for (int i = 0; i < index && current != NULL; ++i) {
    current = current->cdr;
  }
  if (current == NULL || current->type != NODE_LIST) { // Ensure it's a list node before accessing car
    return NULL;
  } else {
    return current->car;
  }
}

// Function: subst (substitute)
Node * subst(SymEntry *alist_head, Node *tree) {
  if (tree == NULL) {
    return NULL;
  }

  if (tree->type == NODE_SYMBOL) {
    SymEntry *current_pair = alist_head;
    while (current_pair != NULL) {
      if (current_pair->name != NULL && strcmp(tree->val_union.symbol_val, current_pair->name) == 0) {
        return current_pair->value;
      }
      current_pair = current_pair->next;
    }
    return tree;
  } else if (tree->type == NODE_LIST) {
    Node *new_car = subst(alist_head, tree->car);
    Node *new_cdr = subst(alist_head, tree->cdr);
    return create_list_node(new_car, new_cdr);
  } else {
    return tree;
  }
}

// Function: lambda (executes a lambda node)
// param_1: The lambda Node (type 4). `lambda_node->car` is params, `lambda_node->cdr` is body.
// param_2: The arguments to the lambda, as a list `(arg1 arg2 ...)`.
// param_3: The current environment `SymEntry *env`.
Node *lambda(Node *lambda_node, Node *args_list, SymEntry *env) {
  if (lambda_node == NULL || lambda_node->type != NODE_LAMBDA || args_list == NULL || env == NULL) {
    return NULL;
  }

  Node *params = lambda_node->car; // The list of parameter symbols
  Node *body = lambda_node->cdr;   // The lambda body expression

  ExpTupleList *alist_container = (ExpTupleList *)malloc(sizeof(ExpTupleList));
  if (alist_container == NULL) {
      perror("malloc failed for alist_container");
      exit(1);
  }
  alist_container->head = NULL;

  int param_index = 0;
  bool args_params_match = true;
  while (true) {
    Node *param_sym_node = get(param_index, params);
    Node *arg_node = get(param_index, args_list);

    if (param_sym_node == NULL && arg_node == NULL) {
      break; // Both lists exhausted
    }
    if (param_sym_node == NULL || arg_node == NULL) {
        args_params_match = false;
        break;
    }

    if (param_sym_node->type != NODE_SYMBOL) {
        fprintf(stderr, "Error: Lambda parameter is not a symbol.\n");
        args_params_match = false;
        break;
    }
    
    // Create a new SymEntry for the substitution (param_sym_node.name, arg_node)
    SymEntry *subst_entry = (SymEntry *)malloc(sizeof(SymEntry));
    if (subst_entry == NULL) {
        perror("malloc failed for subst_entry");
        exit(1);
    }
    subst_entry->name = param_sym_node->val_union.symbol_val; // Use the parameter symbol's string
    subst_entry->value = arg_node; // The evaluated argument node
    subst_entry->next = NULL;

    exptup_list_append_impl(alist_container, subst_entry); // Append to the list of substitutions
    param_index++;
  }

  if (!args_params_match) {
      fprintf(stderr, "Error: Mismatch in number of lambda arguments and parameters.\n");
      // Free alist_container and its contents
      GenericListEntry *current = alist_container->head;
      while(current) {
          GenericListEntry *next = current->next;
          SymEntry *subst_entry = (SymEntry *)current->data;
          free(subst_entry); // Free the SymEntry allocated for substitution
          free(current);
          current = next;
      }
      free(alist_container);
      return NULL;
  }
  
  Node *substituted_body = subst((SymEntry *)alist_container->head, body); // Perform substitutions on the body
  Node *result = eval(substituted_body, env); // Evaluate the substituted body

  // --- Cake logic from original code ---
  unsigned int cake_count = 0;
  Node *cake_checker_node = result;
  Node *final_result_for_cake_check = result; // Keep track of the original result for return

  while (true) {
    if (cake_count > 3 || cake_checker_node == NULL || cake_checker_node->car == NULL || cake_checker_node->car->type != NODE_SYMBOL) {
      break; // Break condition 1
    }
    if (strcmp(cake_checker_node->car->val_union.symbol_val, CAKE_STR) != 0) {
      break; // Break condition 2
    }
    if (cake_checker_node->cdr == NULL || cake_checker_node->cdr->type != NODE_LIST) {
        break; // Break condition 3 (improper list or end of list)
    }
    cake_checker_node = cake_checker_node->cdr;
    cake_count++;
  }

  // After the loop, if cake_count is 4, perform the special action
  if (cake_count == 4) {
    Node *cake_message_symbol = create_symbol_node((char *)CAKE_MESSAGE);
    Node *cake_list = create_list_node(cake_message_symbol, NULL);

    if (cake_checker_node != NULL) { // cake_checker_node is the node whose cdr needs to be modified
        cake_checker_node->cdr = cake_list;
    }
  }

  // Free alist_container and its contents
  GenericListEntry *current = alist_container->head;
  while(current) {
      GenericListEntry *next = current->next;
      SymEntry *subst_entry = (SymEntry *)current->data;
      free(subst_entry); // Free the SymEntry allocated for substitution
      free(current);
      current = next;
  }
  free(alist_container);
  
  return final_result_for_cake_check; // Return the original result, possibly modified by cake logic
}

// Function: make_syms
// Initializes the global symbol table with built-in functions and constants.
SymEntry * make_syms(void) {
  SymEntry *global_env_head = NULL; // Head of the global symbol list

  #define APPEND_SYM_ENTRY(name_str, func) \
    sym_list_append_impl(&global_env_head, make_fp(name_str, func));

  APPEND_SYM_ENTRY(SYMBOL_QUOTE_STR, quote_fn);
  APPEND_SYM_ENTRY(SYMBOL_CAR_STR, car_fn);
  APPEND_SYM_ENTRY(SYMBOL_CDR_STR, cdr_fn);
  APPEND_SYM_ENTRY(SYMBOL_CONS_STR, cons_fn);
  APPEND_SYM_ENTRY("equal", equal_fn); // Original used "equal"
  APPEND_SYM_ENTRY(SYMBOL_ATOM_STR, atom_fn);
  APPEND_SYM_ENTRY(SYMBOL_COND_STR, cond_fn);

  // Add 'nil' symbol
  SymEntry *nil_sym_entry = (SymEntry *)malloc(sizeof(SymEntry));
  if (nil_sym_entry == NULL) { perror("malloc failed for nil_sym_entry"); exit(1); }
  nil_sym_entry->name = strdup(SYMBOL_NIL_STR);
  if (nil_sym_entry->name == NULL) { perror("strdup failed for nil symbol name"); exit(1); }
  nil_sym_entry->value = create_list_node(NULL, NULL); // nil is an empty list
  nil_sym_entry->next = NULL;
  sym_list_append_impl(&global_env_head, nil_sym_entry);

  // Add 't' symbol
  SymEntry *t_sym_entry = (SymEntry *)malloc(sizeof(SymEntry));
  if (t_sym_entry == NULL) { perror("malloc failed for t_sym_entry"); exit(1); }
  t_sym_entry->name = strdup(SYMBOL_T_STR);
  if (t_sym_entry->name == NULL) { perror("strdup failed for t symbol name"); exit(1); }
  t_sym_entry->value = create_symbol_node((char *)SYMBOL_T_STR); // 't' evaluates to itself as a symbol
  t_sym_entry->next = NULL;
  sym_list_append_impl(&global_env_head, t_sym_entry);

  return global_env_head;
}

// Function: eval
Node * eval(Node *node, SymEntry *env) {
  if (node == NULL) {
    return find_sym(env, SYMBOL_NIL_STR);
  }

  if (node->type == NODE_SYMBOL) {
    Node *sym_value = find_sym(env, node->val_union.symbol_val);
    if (sym_value == NULL) {
      return node; // If symbol not found, it evaluates to itself (e.g., free variable)
    }
    return sym_value;
  } else if (node->type == NODE_LIST) {
    if (node->car == NULL) { // Empty list literal, evaluates to nil
        return find_sym(env, SYMBOL_NIL_STR);
    }

    // Check for special forms (quote, lambda) first, as their arguments are not always evaluated
    if (node->car->type == NODE_SYMBOL) {
        char *operator_name = node->car->val_union.symbol_val;
        if (strcmp(operator_name, SYMBOL_QUOTE_STR) == 0) {
            if (node->cdr == NULL || node->cdr->type != NODE_LIST || node->cdr->car == NULL) {
                fprintf(stderr, "Error: Malformed quote expression.\n");
                return NULL;
            }
            return node->cdr->car; // Return the first argument directly
        } else if (strcmp(operator_name, SYMBOL_LAMBDA_STR) == 0) {
            // (lambda (params) body) -> returns a NODE_LAMBDA node
            if (node->cdr == NULL || node->cdr->type != NODE_LIST || // (params) part
                node->cdr->car == NULL || // params list itself
                node->cdr->cdr == NULL || node->cdr->cdr->type != NODE_LIST || // body part
                node->cdr->cdr->car == NULL) { // body expression
                fprintf(stderr, "Error: Malformed lambda expression.\n");
                return NULL;
            }
            Node *params_list = node->cdr->car;
            Node *body_expr = node->cdr->cdr->car;
            return create_lambda_node(params_list, body_expr);
        }
    }

    // If not a special form, evaluate the operator and then the arguments
    Node *operator_node = eval(node->car, env);

    if (operator_node == NULL) {
      return NULL; // Operator evaluation failed
    }

    // Collect and evaluate arguments into a new list
    Node *evaluated_args_head = NULL;
    Node *evaluated_args_tail = NULL;
    Node *current_arg_list_node = node->cdr; // This is the list of arguments to be evaluated

    while (current_arg_list_node != NULL && current_arg_list_node->type == NODE_LIST) {
        Node *evaluated_arg = eval(current_arg_list_node->car, env);
        Node *new_arg_list_node = create_list_node(evaluated_arg, NULL);
        
        if (evaluated_args_head == NULL) {
            evaluated_args_head = new_arg_list_node;
            evaluated_args_tail = new_arg_list_node;
        } else {
            evaluated_args_tail->cdr = new_arg_list_node;
            evaluated_args_tail = new_arg_list_node;
        }
        current_arg_list_node = current_arg_list_node->cdr;
    }
    // Handle potential improper list of arguments (if current_arg_list_node is not NULL but not a list)
    if (current_arg_list_node != NULL && current_arg_list_node->type != NODE_LIST) {
        Node *evaluated_arg = eval(current_arg_list_node, env);
        Node *new_arg_list_node = create_list_node(evaluated_arg, NULL);
        if (evaluated_args_head == NULL) {
            evaluated_args_head = new_arg_list_node;
            evaluated_args_tail = new_arg_list_node;
        } else {
            evaluated_args_tail->cdr = new_arg_list_node;
            evaluated_args_tail = new_arg_list_node;
        }
    }


    if (operator_node->type == NODE_FUNCTION) {
        return operator_node->val_union.func_ptr(evaluated_args_head, env);
    } else if (operator_node->type == NODE_LAMBDA) {
        return lambda(operator_node, evaluated_args_head, env);
    } else {
      fprintf(stderr, "Error: Cannot apply non-function/non-lambda object: ");
      print(operator_node);
      fprintf(stderr, "\n");
      // TODO: Free evaluated_args_head list if not consumed by the called function
      return NULL;
    }
  } else {
    // For other node types (FUNCTION, LAMBDA when they are not the operator, but a value)
    return node;
  }
}

// Function: repl
void repl(void) {
  char input_buffer[1024];
  SymEntry *global_env = make_syms();

  printf("> ");
  while (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
    char **tokens = tokenize(input_buffer);
    if (tokens == NULL || *tokens == NULL) {
      // In a real application, you'd free memory allocated by tokenize here.
      printf("> ");
      continue;
    }

    char **current_token_ptr_ref = tokens;
    Node *parsed_expression = parse(&current_token_ptr_ref, &current_token_ptr_ref);

    if (parsed_expression == NULL) {
      printf("Error: Failed to parse expression.\n");
    } else {
      Node *result = eval(parsed_expression, global_env);
      if (result == NULL) {
        printf("Error: Evaluation failed or returned nil.\n");
      } else {
        print(result);
        printf("\n");
      }
    }

    // TODO: Implement proper memory deallocation for tokens, parsed_expression, and result.
    // For a simple REPL, this might be deferred or handled by OS on exit.

    printf("> ");
  }
}

// Main function
int main() {
    repl();
    return 0;
}