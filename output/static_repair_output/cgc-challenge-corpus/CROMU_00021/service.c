#include <stdio.h>   // For printf, puts
#include <stdlib.h>  // For exit, calloc, free, strtok, div_t (standard div function), sprintf
#include <string.h>  // For strlen, strcmp, strchr, memcpy, memset
#include <ctype.h>   // For isdigit

// Define generic types for decompiled artifacts
// Assuming 'undefined4' is int, 'undefined' is char, 'uint' is unsigned int
// Assuming stack-related 'undefined *' are local stack variables and can be removed or simplified.

// Forward declarations for global variables
// Assuming 'root', 'operStack', 'funcStack' are pointers to custom data structures
// (e.g., Trie* and Stack*)
typedef void* Trie;
typedef void* Stack;
Trie root;
Stack operStack;
Stack funcStack;

// Forward declarations for external/custom functions
// Assuming generic void* for return/argument types where specific structure is unknown
Trie initTrie(void);
Stack initStack(void);
void insertInTrie(Trie trie, const char* key, void* value);
void* lfunc(const char* name, void (*func_ptr)(void)); // Assuming lfunc registers a function
void* lstring(const char* name, const char* value);    // Assuming lstring creates a string object
void* lint(const char* name, const char* value);       // Assuming lint creates an integer object
void* lbool(const char* name, const char* value);      // Assuming lbool creates a boolean object
int isEmpty(Stack stack);
void* pop(Stack stack);
void push(Stack stack, void* item);
int findInTrie(Trie trie, const char* key);
void transmit(void);
int receive_until(char* buffer, int size, char delimiter); // Adjusted signature based on usage

// Helper for exiting the program
void _terminate(int status) {
    exit(status);
}

// Function declarations (from snippet, adjusted)
void process(char *param_1);
void add(void);
void my_int(void); // Renamed from _int to avoid conflict with standard library
void mul(void);
void sub(void);
void my_div(void); // Renamed from div to avoid conflict with standard library
void mod(void);
void not(void);
void len(void);
void equals(void);
void type(void);
void handle_terminate(void); // Renamed from terminate to avoid conflict and clarify purpose

// Global string literals (from DAT_ addresses)
const char* STR_ADD = "+";            // DAT_00016028
const char* STR_SUB = "-";            // DAT_0001602c
const char* STR_MUL = "*";            // DAT_00016030
const char* STR_DIV = "/";            // DAT_00016034
const char* STR_MOD = "%";            // DAT_00016038
const char* STR_NOT = "!";            // DAT_0001603c (in original, _int follows, so ! might be a placeholder, but not is later used)
const char* STR_INT = "int";          // DAT_00016040 (_int function)
const char* STR_TERMINATE = "terminate";
const char* STR_EQUALS = "equals";
const char* STR_TYPE = "type";        // DAT_00016055
const char* STR_LEN = "len";          // DAT_0001605a
const char* STR_PROMPT = "> ";        // DAT_0001605e
const char* STR_EMPTY = "";           // DAT_00016070 (used as a placeholder for anonymous objects)
const char* STR_TRUE = "True";        // DAT_000160c2
const char* STR_FALSE = "False";      // DAT_000160c7
const char* STR_ONE = "1";            // DAT_000161a6
const char* STR_OMG = "OMG";          // DAT_00016360 (used in not function)
const char* STR_NOT_OMG = "NOT OMG";  // DAT_00016364

// Assuming the structure of the objects pushed onto operStack and funcStack
// These are guesses based on usage like `*(char **)(obj + 4)` for type and `*(int *)(obj + 8)` for value.
// `*(int *)(obj + 0xc)` for length/size.
// Let's define a generic object structure.
typedef struct {
    const char* name;
    const char* type;
    void* value_ptr; // Points to actual value (int*, char*, bool*)
    size_t size;     // Length for strings, 0 for others, or specific size for ints/bools
} Object;

// Placeholder for `int_to_str`. This function is typically `sprintf`.
// The original code calls `int_to_str()` sometimes without args, sometimes with `(int val, char* buf)`.
// We'll assume `sprintf` is intended.
#define int_to_str(val, buf) sprintf(buf, "%d", val)

int main(void) {
    char input_buffer[256];

    root = initTrie();
    operStack = initStack();
    funcStack = initStack();

    // Initialize trie with operators and functions
    insertInTrie(root, STR_ADD, lfunc(STR_ADD, add));
    insertInTrie(root, STR_SUB, lfunc(STR_SUB, sub));
    insertInTrie(root, STR_MUL, lfunc(STR_MUL, mul));
    insertInTrie(root, STR_DIV, lfunc(STR_DIV, my_div));
    insertInTrie(root, STR_MOD, lfunc(STR_MOD, mod));
    insertInTrie(root, STR_NOT, lfunc(STR_NOT, not));
    insertInTrie(root, STR_INT, lfunc(STR_INT, my_int));
    insertInTrie(root, STR_TERMINATE, lfunc(STR_TERMINATE, handle_terminate));
    insertInTrie(root, STR_EQUALS, lfunc(STR_EQUALS, equals));
    insertInTrie(root, STR_TYPE, lfunc(STR_TYPE, type));
    insertInTrie(root, STR_LEN, lfunc(STR_LEN, len));

    while (1) {
        memset(input_buffer, 0, sizeof(input_buffer));
        transmit(); // Print prompt
        if (receive_until(input_buffer, sizeof(input_buffer) - 1, '\n') == 0) {
            _terminate(0); // Exit on EOF or read error
        }
        process(input_buffer);
    }
}

void process(char *input_line) {
    char *token = strtok(input_line, " ");
    int should_flush_stacks = 0;

    while (token != NULL) {
        if (isdigit((unsigned char)*token) || (*token == '-' && strlen(token) > 1 && isdigit((unsigned char)token[1]))) {
            // Integer literal
            unsigned int i = (*token == '-') ? 1 : 0;
            while (i < strlen(token) && isdigit((unsigned char)token[i])) {
                i++;
            }
            if (i < strlen(token)) {
                puts("NOT A NUMBER");
            } else {
                push(operStack, lint(STR_EMPTY, token));
            }
        } else if (*token == '\"') {
            // String literal
            char *end_quote = strchr(token + 1, '\"');
            if (end_quote == NULL) {
                printf("Unterminated string constant: %s\n", token);
                should_flush_stacks = 1; // Treat as error, flush stacks and return
                break;
            } else {
                *end_quote = '\0'; // Null-terminate the string
                push(operStack, lstring(STR_EMPTY, token + 1));
            }
        } else if (strcmp(token, "var") == 0) {
            // Variable declaration
            char *var_name = strtok(NULL, " ");
            char *assign_op = strtok(NULL, " ");

            if (var_name == NULL) {
                puts("Malformed variable declaration: missing variable name");
                should_flush_stacks = 1;
                break;
            }

            if (assign_op == NULL || strcmp(assign_op, "=") != 0) {
                // Declaration without assignment or with invalid assignment op
                insertInTrie(root, var_name, lint(var_name, STR_FALSE)); // Default to 0/False
            } else {
                // Declaration with assignment
                char *var_value_str = strtok(NULL, " ");
                if (var_value_str == NULL) {
                    puts("Malformed variable declaration: missing value");
                    should_flush_stacks = 1;
                    break;
                }

                if (*var_value_str == '\"') {
                    char *end_quote = strchr(var_value_str + 1, '\"');
                    if (end_quote != NULL) {
                        *end_quote = '\0';
                        insertInTrie(root, var_name, lstring(var_name, var_value_str + 1));
                    } else {
                        printf("Unterminated string constant: %s\n", var_value_str);
                        should_flush_stacks = 1;
                        break;
                    }
                } else if (strcmp(var_value_str, STR_TRUE) == 0) {
                    insertInTrie(root, var_name, lbool(var_name, STR_TRUE));
                } else if (strcmp(var_value_str, STR_FALSE) == 0) {
                    insertInTrie(root, var_name, lbool(var_name, STR_FALSE));
                } else {
                    insertInTrie(root, var_name, lint(var_name, var_value_str));
                }
            }
        } else if (strcmp(token, STR_TRUE) == 0) {
            push(operStack, lbool(STR_EMPTY, STR_TRUE));
        } else if (strcmp(token, STR_FALSE) == 0) {
            push(operStack, lbool(STR_EMPTY, STR_FALSE));
        } else if (strcmp(token, STR_ADD) == 0) {
            push(funcStack, (void*)findInTrie(root, STR_ADD));
        } else if (strcmp(token, STR_SUB) == 0) {
            push(funcStack, (void*)findInTrie(root, STR_SUB));
        } else if (strcmp(token, STR_MUL) == 0) {
            push(funcStack, (void*)findInTrie(root, STR_MUL));
        } else if (strcmp(token, STR_DIV) == 0) {
            push(funcStack, (void*)findInTrie(root, STR_DIV));
        } else if (strcmp(token, STR_MOD) == 0) {
            push(funcStack, (void*)findInTrie(root, STR_MOD));
        } else if (strcmp(token, "==") == 0) {
            push(funcStack, (void*)findInTrie(root, STR_EQUALS));
        } else {
            // Identifier (variable or function)
            void* obj_ptr = (void*)findInTrie(root, token);
            if (obj_ptr == NULL) {
                printf("Undeclared identifier: %s\n", token);
            } else {
                Object* obj = (Object*)obj_ptr; // Cast to Object to access members
                if (strcmp(obj->type, "Function") == 0) {
                    push(funcStack, obj_ptr);
                } else {
                    push(operStack, obj_ptr);
                }
            }
        }
        token = strtok(NULL, " ");
    } // End of token processing loop

    if (should_flush_stacks) {
        // Clear stacks on error before exiting or continuing
        while (!isEmpty(funcStack)) pop(funcStack);
        while (!isEmpty(operStack)) pop(operStack);
        return;
    }

    // Process functions on funcStack
    while (!isEmpty(funcStack)) {
        Object* func_obj = (Object*)pop(funcStack);
        if (func_obj != NULL && func_obj->value_ptr != NULL) {
            void (*func_ptr)(void) = (void (*)(void))func_obj->value_ptr;
            func_ptr();
        }
    }

    // Print result from operStack if any
    if (!isEmpty(operStack)) {
        Object* result = (Object*)pop(operStack);
        if (result != NULL) {
            if (strcmp(result->type, "Integer") == 0) {
                printf("%d\n", *(int*)result->value_ptr);
            } else if (strcmp(result->type, "String") == 0) {
                printf("%s\n", (char*)result->value_ptr);
            } else if (strcmp(result->type, "Boolean") == 0) {
                printf("%s\n", *(int*)result->value_ptr == 1 ? "True" : "False");
            }
        }
    }
}

void add(void) {
    Object* op2 = (Object*)pop(operStack);
    Object* op1 = (Object*)pop(operStack);

    if (op1 == NULL || op2 == NULL) {
        puts("Not enough operands for add");
        return;
    }

    if (strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "Integer") == 0) {
        char buffer[32];
        int_to_str(*(int*)op1->value_ptr + *(int*)op2->value_ptr, buffer);
        push(operStack, lint(STR_EMPTY, buffer));
    } else if (strcmp(op1->type, "String") == 0 && strcmp(op2->type, "String") == 0) {
        size_t new_len = strlen((char*)op1->value_ptr) + strlen((char*)op2->value_ptr) + 1;
        char* new_str = (char*)calloc(new_len, 1);
        if (new_str == NULL) {
            puts("Critical memory error. Exiting.");
            _terminate(1);
        }
        memcpy(new_str, (char*)op1->value_ptr, strlen((char*)op1->value_ptr));
        memcpy(new_str + strlen((char*)op1->value_ptr), (char*)op2->value_ptr, strlen((char*)op2->value_ptr));
        push(operStack, lstring(STR_EMPTY, new_str));
        free(new_str); // lstring should copy the string, so we can free our temporary buffer
    } else if (strcmp(op1->type, "String") == 0 && strcmp(op2->type, "Integer") == 0) {
        char int_str[32];
        int_to_str(*(int*)op2->value_ptr, int_str);
        size_t new_len = strlen((char*)op1->value_ptr) + strlen(int_str) + 1;
        char* new_str = (char*)calloc(new_len, 1);
        if (new_str == NULL) {
            puts("Critical memory error. Exiting.");
            _terminate(1);
        }
        memcpy(new_str, (char*)op1->value_ptr, strlen((char*)op1->value_ptr));
        memcpy(new_str + strlen((char*)op1->value_ptr), int_str, strlen(int_str));
        push(operStack, lstring(STR_EMPTY, new_str));
        free(new_str);
    } else if (strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "String") == 0) {
        char int_str[32];
        int_to_str(*(int*)op1->value_ptr, int_str);
        size_t new_len = strlen(int_str) + strlen((char*)op2->value_ptr) + 1;
        char* new_str = (char*)calloc(new_len, 1);
        if (new_str == NULL) {
            puts("Critical memory error. Exiting.");
            _terminate(1);
        }
        memcpy(new_str, int_str, strlen(int_str));
        memcpy(new_str + strlen(int_str), (char*)op2->value_ptr, strlen((char*)op2->value_ptr));
        push(operStack, lstring(STR_EMPTY, new_str));
        free(new_str);
    } else {
        printf("Add doesn't make sense on %s and %s\n", op1->type, op2->type);
    }
}

void my_int(void) {
    Object* op = (Object*)pop(operStack);
    if (op == NULL) {
        puts("Not enough operands for int");
        return;
    }

    if (strcmp(op->type, "Integer") == 0) {
        push(operStack, op);
    } else if (strcmp(op->type, "String") == 0) {
        char* s = (char*)op->value_ptr;
        for (unsigned int i = 0; i < strlen(s); i++) {
            if (!isdigit((unsigned char)s[i]) && !(i == 0 && s[i] == '-')) { // Allow leading minus sign
                puts("Could not convert argument to int");
                return;
            }
        }
        push(operStack, lint(STR_EMPTY, s));
    } else if (strcmp(op->type, "Boolean") == 0) {
        push(operStack, lint(STR_EMPTY, *(int*)op->value_ptr == 0 ? STR_FALSE : STR_ONE));
    }
}

void mul(void) {
    Object* op2 = (Object*)pop(operStack);
    Object* op1 = (Object*)pop(operStack);

    if (op1 == NULL || op2 == NULL) {
        puts("Not enough operands for multiplication");
        return;
    }

    if (strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "Integer") == 0) {
        char buffer[32];
        int_to_str(*(int*)op1->value_ptr * *(int*)op2->value_ptr, buffer);
        push(operStack, lint(STR_EMPTY, buffer));
    } else if (strcmp(op1->type, "String") == 0 && strcmp(op2->type, "Integer") == 0) {
        int count = *(int*)op2->value_ptr;
        if (count < 0) {
            puts("Cannot multiply string by negative integer.");
            return;
        }
        size_t str_len = strlen((char*)op1->value_ptr);
        size_t new_len = str_len * count + 1;
        char* new_str = (char*)calloc(new_len, 1);
        if (new_str == NULL) {
            puts("Critical memory error. Terminating.");
            _terminate(1);
        }
        for (int i = 0; i < count; i++) {
            memcpy(new_str + (i * str_len), (char*)op1->value_ptr, str_len);
        }
        push(operStack, lstring(STR_EMPTY, new_str));
        free(new_str);
    } else if (strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "String") == 0) {
        int count = *(int*)op1->value_ptr;
        if (count < 0) {
            puts("Cannot multiply string by negative integer.");
            return;
        }
        size_t str_len = strlen((char*)op2->value_ptr);
        size_t new_len = str_len * count + 1;
        char* new_str = (char*)calloc(new_len, 1);
        if (new_str == NULL) {
            puts("Critical memory error. Terminating.");
            _terminate(1);
        }
        for (int i = 0; i < count; i++) {
            memcpy(new_str + (i * str_len), (char*)op2->value_ptr, str_len);
        }
        push(operStack, lstring(STR_EMPTY, new_str));
        free(new_str);
    } else {
        printf("Mul does not make sense with %s and %s.\n", op1->type, op2->type);
    }
}

void sub(void) {
    Object* op2 = (Object*)pop(operStack);
    Object* op1 = (Object*)pop(operStack);

    if (op1 == NULL || op2 == NULL) {
        puts("Not enough operands for sub");
        return;
    }

    if (strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "Integer") == 0) {
        char buffer[32];
        int_to_str(*(int*)op1->value_ptr - *(int*)op2->value_ptr, buffer);
        push(operStack, lint(STR_EMPTY, buffer));
    } else {
        printf("Sub does not make sense on %s and %s\n", op1->type, op2->type);
    }
}

void my_div(void) {
    Object* op2 = (Object*)pop(operStack);
    Object* op1 = (Object*)pop(operStack);

    if (op1 == NULL || op2 == NULL) {
        puts("Not enough operands for division.");
        return;
    }

    if (strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "Integer") == 0) {
        if (*(int*)op2->value_ptr == 0) {
            puts("Cannot divide by zero.");
        } else {
            char buffer[32];
            int_to_str(*(int*)op1->value_ptr / *(int*)op2->value_ptr, buffer);
            push(operStack, lint(STR_EMPTY, buffer));
        }
    } else {
        printf("Division does not make sense on %s and %s\n", op1->type, op2->type);
    }
}

void mod(void) {
    Object* op2 = (Object*)pop(operStack);
    Object* op1 = (Object*)pop(operStack);

    if (op1 == NULL || op2 == NULL) {
        puts("Not enough operands for mod.");
        return;
    }

    if (strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "Integer") == 0) {
        if (*(int*)op2->value_ptr == 0) {
            puts("Cannot mod by zero.");
        } else {
            char buffer[32];
            int_to_str(*(int*)op1->value_ptr % *(int*)op2->value_ptr, buffer);
            push(operStack, lint(STR_EMPTY, buffer));
        }
    } else {
        printf("Mod does not make sense on %s and %s\n", op1->type, op2->type);
    }
}

void not(void) {
    Object* op = (Object*)pop(operStack);
    if (op == NULL) {
        puts("Not enough operands for not.");
        return;
    }

    if (strcmp(op->type, "String") == 0) {
        if (strcmp((char*)op->value_ptr, STR_OMG) == 0) {
            push(operStack, lstring(STR_EMPTY, STR_NOT_OMG));
        } else {
            push(operStack, lstring(STR_EMPTY, STR_OMG));
        }
    } else if (strcmp(op->type, "Integer") == 0) {
        char buffer[32];
        int_to_str(~*(unsigned int*)op->value_ptr, buffer); // Bitwise NOT for integers
        push(operStack, lint(STR_EMPTY, buffer));
    } else if (strcmp(op->type, "Boolean") == 0) {
        if (*(int*)op->value_ptr == 1) { // If True
            push(operStack, lbool(STR_EMPTY, STR_FALSE));
        } else { // If False
            push(operStack, lbool(STR_EMPTY, STR_TRUE));
        }
    } else {
        printf("Not does not make sense on type %s\n", op->type);
    }
}

void len(void) {
    Object* op = (Object*)pop(operStack);
    if (op == NULL) {
        puts("Not enough operands for len.");
        return;
    }

    char buffer[32];
    // Assuming size member holds the length for strings or 0 for other types
    // The original code uses *(undefined4 *)(local_10 + 0xc) which maps to size_t size in Object
    int_to_str(op->size, buffer);
    push(operStack, lint(STR_EMPTY, buffer));
}

void equals(void) {
    Object* op2 = (Object*)pop(operStack);
    Object* op1 = (Object*)pop(operStack);

    if (op1 == NULL || op2 == NULL) {
        puts("Not enough operands for equality");
        return;
    }

    int result = 0;
    if ((strcmp(op1->type, "Integer") == 0 && strcmp(op2->type, "Integer") == 0) ||
        (strcmp(op1->type, "Boolean") == 0 && strcmp(op2->type, "Boolean") == 0)) {
        result = (*(int*)op1->value_ptr == *(int*)op2->value_ptr);
    } else if (strcmp(op1->type, "String") == 0 && strcmp(op2->type, "String") == 0) {
        result = (strcmp((char*)op1->value_ptr, (char*)op2->value_ptr) == 0);
    }

    push(operStack, lbool(STR_EMPTY, result ? STR_TRUE : STR_FALSE));
}

void type(void) {
    Object* op = (Object*)pop(operStack);
    if (op == NULL) {
        // According to original code, if no operand, nothing is done.
        // But it's usually an error or returns a special "null" type.
        // For now, doing nothing.
        return;
    }
    push(operStack, lstring(STR_EMPTY, op->type));
}

void handle_terminate(void) {
    Object* op = (Object*)pop(operStack);
    if (op != NULL && strcmp(op->type, "Integer") == 0) {
        _terminate(*(int*)op->value_ptr); // Exit with the integer value
    } else {
        _terminate(0); // Exit with 0 if no integer operand or operand is not an integer
    }
}


// --- Placeholder Implementations for external functions ---
// These would typically be in separate .c files and declared in a .h file.
// For compilation, we need minimal definitions.

#include <string.h> // For strlen, strcpy, strdup
#include <stdio.h>  // For printf, snprintf

// Simple Trie node
typedef struct TrieNode {
    char* key;
    void* value;
    struct TrieNode* next; // For linked list-like collision handling or simple list
} TrieNode;

// Simple Stack node
typedef struct StackNode {
    void* data;
    struct StackNode* next;
} StackNode;

// Global placeholder for trie storage (a simple linked list for demonstration)
TrieNode* global_trie_head = NULL;

// Global placeholder for stack storage
StackNode* global_oper_stack_head = NULL;
StackNode* global_func_stack_head = NULL;

// Object creation helpers (simplified, assuming values are copied or pointed to directly)
Object* create_object(const char* name, const char* type, void* value_ptr, size_t size) {
    Object* obj = (Object*)malloc(sizeof(Object));
    if (obj == NULL) {
        fprintf(stderr, "Memory allocation failed for Object.\n");
        _terminate(1);
    }
    obj->name = (name && strlen(name) > 0) ? strdup(name) : NULL;
    obj->type = strdup(type);
    obj->size = size;

    if (strcmp(type, "Integer") == 0) {
        obj->value_ptr = malloc(sizeof(int));
        if (obj->value_ptr == NULL) { fprintf(stderr, "Memory allocation failed.\n"); _terminate(1); }
        *(int*)obj->value_ptr = atoi((char*)value_ptr);
    } else if (strcmp(type, "String") == 0) {
        obj->value_ptr = strdup((char*)value_ptr);
    } else if (strcmp(type, "Boolean") == 0) {
        obj->value_ptr = malloc(sizeof(int));
        if (obj->value_ptr == NULL) { fprintf(stderr, "Memory allocation failed.\n"); _terminate(1); }
        *(int*)obj->value_ptr = (strcmp((char*)value_ptr, "True") == 0 || strcmp((char*)value_ptr, "1") == 0);
    } else if (strcmp(type, "Function") == 0) {
        obj->value_ptr = value_ptr; // Store the function pointer directly
    } else {
        obj->value_ptr = NULL; // Unknown type, no value
    }
    return obj;
}

// Implementations of external functions
Trie initTrie(void) {
    // In this simplified model, the global_trie_head serves as the Trie handle.
    // We could return it, or just operate on it globally.
    // For now, just a placeholder.
    return (Trie)&global_trie_head;
}

Stack initStack(void) {
    // Return a pointer to the head of the respective global stack.
    // This allows differentiation between operStack and funcStack.
    if (operStack == NULL) return (Stack)&global_oper_stack_head;
    return (Stack)&global_func_stack_head;
}

void insertInTrie(Trie trie, const char* key, void* value) {
    TrieNode** head = (TrieNode**)trie;
    TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for TrieNode.\n");
        _terminate(1);
    }
    newNode->key = strdup(key);
    newNode->value = value; // Value is an Object*
    newNode->next = *head;
    *head = newNode;
}

void* lfunc(const char* name, void (*func_ptr)(void)) {
    // Create an object representing the function
    return create_object(name, "Function", (void*)func_ptr, 0);
}

void* lstring(const char* name, const char* value) {
    return create_object(name, "String", (void*)value, strlen(value));
}

void* lint(const char* name, const char* value) {
    return create_object(name, "Integer", (void*)value, sizeof(int));
}

void* lbool(const char* name, const char* value) {
    return create_object(name, "Boolean", (void*)value, sizeof(int));
}

int isEmpty(Stack stack) {
    StackNode** head = (StackNode**)stack;
    return (*head == NULL);
}

void* pop(Stack stack) {
    StackNode** head = (StackNode**)stack;
    if (*head == NULL) {
        return NULL;
    }
    StackNode* temp = *head;
    void* data = temp->data;
    *head = temp->next;
    free(temp);
    return data;
}

void push(Stack stack, void* item) {
    StackNode** head = (StackNode**)stack;
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for StackNode.\n");
        _terminate(1);
    }
    newNode->data = item;
    newNode->next = *head;
    *head = newNode;
}

int findInTrie(Trie trie, const char* key) {
    TrieNode** head = (TrieNode**)trie;
    TrieNode* current = *head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return (int)current->value; // Return the Object* as an int (pointer value)
        }
        current = current->next;
    }
    return 0; // Not found
}

void transmit(void) {
    printf("%s", STR_PROMPT);
    fflush(stdout);
}

int receive_until(char* buffer, int size, char delimiter) {
    if (fgets(buffer, size, stdin) == NULL) {
        return 0; // EOF or error
    }
    // Remove trailing newline if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 1; // Success
}