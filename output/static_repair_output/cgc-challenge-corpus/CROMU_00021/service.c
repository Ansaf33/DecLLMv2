#include <stdio.h>   // For printf, puts, fflush
#include <stdlib.h>  // For exit, calloc, free
#include <string.h>  // For strlen, strcmp, strchr, strtok, memset, memcpy, strcpy, strcat
#include <ctype.h>   // For isdigit

// --- Forward Declarations for external/unknown functions and structures ---

// ValueNode structure inferred from usage patterns (offsets +4, +8, +12)
// This structure maps to how the disassembler accesses fields at specific offsets.
typedef struct ValueNode {
    // Offset 0x0: Unused or private member, not directly accessed in the snippet.
    char *type;          // Offset 0x4: Stores type string ("Integer", "String", "Boolean", "Function")
    void *data_ptr;      // Offset 0x8: For strings, points to char*. For functions, points to void(*)(void).
                         //             For integers/booleans, this field holds the actual int/bool value.
    int len_or_val;      // Offset 0xc: For strings, stores length. For integers/booleans, stores the actual value.
                         //             (This field is redundant for int/bool if data_ptr holds the value,
                         //             but it's used consistently for `len` function and comparisons.)
} ValueNode;

// Stack structure (abstracted)
typedef struct Stack Stack;

// TrieNode structure (abstracted)
typedef struct TrieNode TrieNode;

// Global variables
TrieNode *root;
Stack *operStack;
Stack *funcStack;

// External function declarations (actual implementations are not provided in the snippet)
// ValueNode creation functions - assume they return a dynamically allocated ValueNode*
ValueNode *lfunc(const char *name, void (*func_ptr)(void));
ValueNode *lstring(const char *name, char *str_val);
ValueNode *lint(const char *name, const char *int_str);
ValueNode *lbool(const char *name, const char *bool_str);

// Trie operations
TrieNode *initTrie(void);
void insertInTrie(TrieNode *trie, const char *key, ValueNode *value);
ValueNode *findInTrie(TrieNode *trie, const char *key); // Returns ValueNode*

// Stack operations
Stack *initStack(void);
void push(Stack *stack, ValueNode *value);
ValueNode *pop(Stack *stack);
int isEmpty(Stack *stack);

// I/O and utility functions
void transmit(void); // Signature inferred from `transmit();`
int receive_until(char *buffer, int max_len, char delimiter); // Inferred signature
void _terminate(void); // Actual termination function, likely calling exit()

// Utility to convert integer to string (inferred signature from `int_to_str(val, buffer)`)
void int_to_str(int value, char *buffer);

// --- Global String Literals (DAT_...) ---
const char * const DAT_00016028 = "+";
const char * const DAT_0001602c = "-";
const char * const DAT_00016030 = "*";
const char * const DAT_00016034 = "/";
const char * const DAT_00016038 = "%";
const char * const DAT_0001603c = "not";
const char * const DAT_00016040 = "int";
const char * const DAT_00016055 = "type";
const char * const DAT_0001605a = "len";
const char * const DAT_0001605e = ">>> ";
const char * const DAT_00016070 = ""; // Placeholder name, or empty string value
const char * const DAT_000160c2 = "True";
const char * const DAT_000160c7 = "0";
const char * const DAT_000161a6 = "1";
const char * const DAT_00016360 = "False";
const char * const DAT_00016364 = "OMG!"; // For not "OMG" -> True

// --- Function Prototypes for the supplied snippet functions ---
void add(void);
void process(char *param_1);
void _int(void);
void mul(void);
void sub(void);
void div_op(void); // Renamed to avoid conflict with standard library div
void mod(void);
void not(void);
void len(void);
void equals(void);
void type(void);
void terminate(void); // The user-defined 'terminate' function

// --- Fixed Functions ---

// Function: main
int main(void) {
  char input_buffer[256];
  
  root = initTrie();
  operStack = initStack();
  funcStack = initStack();

  // Register built-in functions
  insertInTrie(root, DAT_00016028, lfunc(DAT_00016028, add));
  insertInTrie(root, DAT_0001602c, lfunc(DAT_0001602c, sub));
  insertInTrie(root, DAT_00016030, lfunc(DAT_00016030, mul));
  insertInTrie(root, DAT_00016034, lfunc(DAT_00016034, div_op));
  insertInTrie(root, DAT_00016038, lfunc(DAT_00016038, mod));
  insertInTrie(root, DAT_0001603c, lfunc(DAT_0001603c, not));
  insertInTrie(root, DAT_00016040, lfunc(DAT_00016040, _int));
  insertInTrie(root, "terminate", lfunc("terminate", terminate));
  insertInTrie(root, "equals", lfunc("equals", equals));
  insertInTrie(root, DAT_00016055, lfunc(DAT_00016055, type));
  insertInTrie(root, DAT_0001605a, lfunc(DAT_0001605a, len));
  
  while(1) {
    memset(input_buffer, 0, sizeof(input_buffer)); // Clear buffer
    printf("%s", DAT_0001605e); // Print prompt ">>> "
    fflush(stdout); // Ensure prompt is displayed before receive_until

    int bytes_received = receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');
    
    if (bytes_received == 0) { // If receive_until returns 0, it indicates EOF or error
      _terminate(); // Exit the program
    }
    
    // Remove the newline character if present and null-terminate
    if (bytes_received > 0 && input_buffer[bytes_received - 1] == '\n') {
        input_buffer[bytes_received - 1] = '\0';
    } else {
        input_buffer[bytes_received] = '\0'; // Ensure null-termination
    }

    process(input_buffer);
  }
  return 0; // Should not be reached in this infinite loop
}

// Function: process
void process(char *param_1) {
  char *token;
  ValueNode *val;
  ValueNode *func_node;
  ValueNode *oper_node;
  int processing_error = 0; // Flag to indicate an error during token processing

  token = strtok(param_1, " ");

  while (token != NULL && !processing_error) {
    if (isdigit((unsigned char)*token) || (*token == '-' && strlen(token) > 1 && isdigit((unsigned char)token[1]))) {
      // It's a number (or potentially a negative number)
      size_t i = 1;
      while (token[i] != '\0' && isdigit((unsigned char)token[i])) {
        i++;
      }
      if (token[i] != '\0') { // If not all characters are digits (after optional minus)
        puts("NOT A NUMBER");
        processing_error = 1;
      } else {
        push(operStack, lint(DAT_00016070, token)); // Anonymous integer
      }
    } else if (*token == '\"') {
      // It's a string literal
      char *end_quote = strchr(token + 1, '\"');
      if (end_quote == NULL) {
        printf("Unterminated string constant: %s\n", token);
        processing_error = 1;
      } else {
        *end_quote = '\0'; // Null-terminate the string
        push(operStack, lstring(DAT_00016070, token + 1)); // Anonymous string
      }
    } else {
      // Keywords, operators, or identifiers
      if (strcmp(token, "var") == 0) {
        char *var_name = strtok(NULL, " ");
        char *assignment_op = strtok(NULL, " ");
        
        if (var_name == NULL) {
            printf("Malformed variable declaration: missing variable name\n");
            processing_error = 1;
        } else if (assignment_op != NULL && strcmp(assignment_op, "=") == 0) {
          char *var_value_str = strtok(NULL, " ");
          if (var_value_str == NULL) {
            printf("Malformed variable declaration: missing value for '%s'\n", var_name);
            processing_error = 1;
          } else if (*var_value_str == '\"') {
            char *end_quote = strchr(var_value_str + 1, '\"');
            if (end_quote != NULL) {
              *end_quote = '\0';
              insertInTrie(root, var_name, lstring(var_name, var_value_str + 1));
            } else {
              printf("Unterminated string constant in variable declaration: %s\n", var_value_str);
              processing_error = 1;
            }
          } else if (strcmp(var_value_str, DAT_000160c2) == 0) { // "True"
            insertInTrie(root, var_name, lbool(var_name, DAT_000160c2));
          } else if (strcmp(var_value_str, DAT_00016360) == 0) { // "False"
            insertInTrie(root, var_name, lbool(var_name, DAT_00016360));
          } else {
            // Assume it's an integer
            insertInTrie(root, var_name, lint(var_name, var_value_str));
          }
        } else { // No '=' or invalid operator, just declare variable with default value "0"
          insertInTrie(root, var_name, lint(var_name, DAT_000160c7));
        }
      } else if (strcmp(token, DAT_000160c2) == 0) { // "True"
        push(operStack, lbool(DAT_00016070, DAT_000160c2));
      } else if (strcmp(token, DAT_00016360) == 0) { // "False"
        push(operStack, lbool(DAT_00016070, DAT_00016360));
      } else if (strcmp(token, DAT_00016028) == 0) { // "+"
        push(funcStack, findInTrie(root, DAT_00016028));
      } else if (strcmp(token, DAT_0001602c) == 0) { // "-"
        push(funcStack, findInTrie(root, DAT_0001602c));
      } else if (strcmp(token, DAT_00016030) == 0) { // "*"
        push(funcStack, findInTrie(root, DAT_00016030));
      } else if (strcmp(token, DAT_00016034) == 0) { // "/"
        push(funcStack, findInTrie(root, DAT_00016034));
      } else if (strcmp(token, DAT_00016038) == 0) { // "%"
        push(funcStack, findInTrie(root, DAT_00016038));
      } else if (strcmp(token, "==") == 0) { // "equals"
        push(funcStack, findInTrie(root, "equals"));
      } else {
        // Identifier (variable or function call)
        val = findInTrie(root, token);
        if (val == NULL) {
          printf("Undeclared identifier: %s\n", token);
          processing_error = 1;
        } else {
          if (strcmp(val->type, "Function") == 0) {
            push(funcStack, val);
          } else {
            push(operStack, val);
          }
        }
      }
    }
    token = strtok(NULL, " ");
  }

  // After all tokens are processed (or an error occurred)
  if (!processing_error) {
    while (!isEmpty(funcStack)) {
      func_node = pop(funcStack);
      if (func_node != NULL && func_node->type != NULL && strcmp(func_node->type, "Function") == 0) {
          if (func_node->data_ptr != NULL) {
              ((void (*)(void))func_node->data_ptr)(); // Call the function pointer
          } else {
              puts("Error: Function node has NULL data_ptr.");
          }
      } else {
          puts("Error: Popped non-function from funcStack.");
      }
    }

    if (!isEmpty(operStack)) {
      oper_node = pop(operStack);
      if (oper_node != NULL) {
        if (strcmp(oper_node->type, "Integer") == 0) {
          printf("%d\n", oper_node->len_or_val); // Integer value is in len_or_val
        } else if (strcmp(oper_node->type, "String") == 0) {
          printf("%s\n", (char *)oper_node->data_ptr); // String value is pointed to by data_ptr
        } else if (strcmp(oper_node->type, "Boolean") == 0) {
          printf("%s\n", oper_node->len_or_val == 1 ? "True" : "False"); // Boolean value is in len_or_val
        }
      }
    }
  }
}

// Function: add
void add(void) {
  ValueNode *operand1 = NULL; // Corresponds to local_10
  ValueNode *operand2 = NULL; // Corresponds to local_14
  char result_buffer[32];
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for add");
    return;
  }
  operand1 = pop(operStack);
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for add");
    return;
  }
  operand2 = pop(operStack);

  if (operand1 == NULL || operand2 == NULL) { // Defensive check
      puts("Error: Missing operand for add after pop.");
      return;
  }

  if (strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "Integer") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    int_to_str(operand2->len_or_val + operand1->len_or_val, result_buffer);
    push(operStack, lint(DAT_00016070, result_buffer));
  } else if (strcmp(operand1->type, "String") == 0 && strcmp(operand2->type, "String") == 0) {
    size_t len2 = strlen((char *)operand2->data_ptr);
    size_t len1 = strlen((char *)operand1->data_ptr);
    char *concat_str = (char *)calloc(len2 + len1 + 1, 1);
    if (concat_str == NULL) {
      puts("Critical memory error. Exiting.");
      _terminate();
    }
    memcpy(concat_str, (char *)operand2->data_ptr, len2); // Copy operand2 first
    memcpy(concat_str + len2, (char *)operand1->data_ptr, len1); // Then operand1
    concat_str[len2 + len1] = '\0'; // Ensure null termination
    push(operStack, lstring(DAT_00016070, concat_str));
    free(concat_str); // lstring should copy the string, so we can free the temp buffer
  } else if (strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "String") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    int_to_str(operand1->len_or_val, result_buffer); // Convert int to string
    size_t len2 = strlen((char *)operand2->data_ptr);
    size_t len_int_str = strlen(result_buffer);
    char *concat_str = (char *)calloc(len2 + len_int_str + 1, 1);
    if (concat_str == NULL) {
      puts("Critical memory error. Exiting.");
      _terminate();
    }
    memcpy(concat_str, (char *)operand2->data_ptr, len2);
    memcpy(concat_str + len2, result_buffer, len_int_str);
    concat_str[len2 + len_int_str] = '\0';
    push(operStack, lstring(DAT_00016070, concat_str));
    free(concat_str);
  } else if (strcmp(operand1->type, "String") == 0 && strcmp(operand2->type, "Integer") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    int_to_str(operand2->len_or_val, result_buffer); // Convert int to string
    size_t len1 = strlen((char *)operand1->data_ptr);
    size_t len_int_str = strlen(result_buffer);
    char *concat_str = (char *)calloc(len_int_str + len1 + 1, 1);
    if (concat_str == NULL) {
      puts("Critical memory error. Exiting.");
      _terminate();
    }
    memcpy(concat_str, result_buffer, len_int_str); // Order matters: operand2 (int) then operand1 (string) based on original logic.
    memcpy(concat_str + len_int_str, (char *)operand1->data_ptr, len1);
    concat_str[len_int_str + len1] = '\0';
    push(operStack, lstring(DAT_00016070, concat_str));
    free(concat_str);
  } else {
    printf("Add doesn't make sense on %s and %s\n", operand2->type, operand1->type);
  }
}

// Function: _int
void _int(void) {
  ValueNode *operand = NULL;
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for int");
    return;
  }
  operand = pop(operStack);

  if (operand == NULL) {
      puts("Error: Missing operand for int after pop.");
      return;
  }

  if (strcmp(operand->type, "Integer") == 0) {
    push(operStack, operand); // Already an integer, push it back
  } else if (strcmp(operand->type, "String") == 0) {
    char *s = (char *)operand->data_ptr;
    if (s == NULL) {
        puts("Could not convert NULL string to int");
        return;
    }
    for (size_t i = 0; i < strlen(s); i++) {
      if (!isdigit((unsigned char)s[i]) && !(i == 0 && s[i] == '-' && strlen(s) > 1)) { // Allow initial minus for negative numbers
        puts("Could not convert argument to int");
        return;
      }
    }
    push(operStack, lint(DAT_00016070, s));
  } else if (strcmp(operand->type, "Boolean") == 0) {
    if (operand->len_or_val == 0) { // False
      push(operStack, lint(DAT_00016070, DAT_000160c7)); // "0"
    } else { // True
      push(operStack, lint(DAT_00016070, DAT_000161a6)); // "1"
    }
  }
}

// Function: mul
void mul(void) {
  ValueNode *operand1 = NULL; // Corresponds to local_10
  ValueNode *operand2 = NULL; // Corresponds to local_14
  char result_buffer[32];
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for multiplication");
    return;
  }
  operand1 = pop(operStack);
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for multiplication");
    return;
  }
  operand2 = pop(operStack);

  if (operand1 == NULL || operand2 == NULL) {
      puts("Error: Missing operand for mul after pop.");
      return;
  }

  if (strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "Integer") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    int_to_str(operand2->len_or_val * operand1->len_or_val, result_buffer);
    push(operStack, lint(DAT_00016070, result_buffer));
  } else if (strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "String") == 0) {
    int count = operand1->len_or_val;
    if (count < 0) count = 0; // Don't multiply by negative
    char *str = (char *)operand2->data_ptr;
    size_t str_len = strlen(str);
    size_t total_len = str_len * count + 1;
    char *repeated_str = (char *)calloc(total_len, 1);
    if (repeated_str == NULL) {
      puts("Critical memory error. Terminating.");
      _terminate();
    }
    for (int i = 0; i < count; ++i) {
      strcat(repeated_str, str);
    }
    push(operStack, lstring(DAT_00016070, repeated_str));
    free(repeated_str);
  } else if (strcmp(operand1->type, "String") == 0 && strcmp(operand2->type, "Integer") == 0) {
    // Same logic as above, just swapped operands
    int count = operand2->len_or_val;
    if (count < 0) count = 0;
    char *str = (char *)operand1->data_ptr;
    size_t str_len = strlen(str);
    size_t total_len = str_len * count + 1;
    char *repeated_str = (char *)calloc(total_len, 1);
    if (repeated_str == NULL) {
      puts("Critical memory error. Terminating.");
      _terminate();
    }
    for (int i = 0; i < count; ++i) {
      strcat(repeated_str, str);
    }
    push(operStack, lstring(DAT_00016070, repeated_str));
    free(repeated_str);
  } else {
    printf("Mul does not make sense with %s and %s.\n", operand2->type, operand1->type);
  }
}

// Function: sub
void sub(void) {
  ValueNode *operand1 = NULL; // Corresponds to local_10
  ValueNode *operand2 = NULL; // Corresponds to local_14
  char result_buffer[32];
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for sub");
    return;
  }
  operand1 = pop(operStack);
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for sub");
    return;
  }
  operand2 = pop(operStack);

  if (operand1 == NULL || operand2 == NULL) {
      puts("Error: Missing operand for sub after pop.");
      return;
  }

  if (strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "Integer") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    int_to_str(operand2->len_or_val - operand1->len_or_val, result_buffer);
    push(operStack, lint(DAT_00016070, result_buffer));
  } else {
    printf("Sub does not make sense on %s and %s\n", operand2->type, operand1->type);
  }
}

// Function: div_op (renamed from div to avoid conflict)
void div_op(void) {
  ValueNode *operand1 = NULL; // Corresponds to local_10
  ValueNode *operand2 = NULL; // Corresponds to local_14
  char result_buffer[32];
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for division.");
    return;
  }
  operand1 = pop(operStack);
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for division.");
    return;
  }
  operand2 = pop(operStack);

  if (operand1 == NULL || operand2 == NULL) {
      puts("Error: Missing operand for div after pop.");
      return;
  }

  if (strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "Integer") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    if (operand1->len_or_val == 0) {
      puts("Cannot divide by zero.");
    } else {
      int_to_str(operand2->len_or_val / operand1->len_or_val, result_buffer);
      push(operStack, lint(DAT_00016070, result_buffer));
    }
  } else {
    printf("Division does not make sense on %s and %s\n", operand2->type, operand1->type);
  }
}

// Function: mod
void mod(void) {
  ValueNode *operand1 = NULL; // Corresponds to local_10
  ValueNode *operand2 = NULL; // Corresponds to local_14
  char result_buffer[32];
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for mod.");
    return;
  }
  operand1 = pop(operStack);
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for mod.");
    return;
  }
  operand2 = pop(operStack);

  if (operand1 == NULL || operand2 == NULL) {
      puts("Error: Missing operand for mod after pop.");
      return;
  }

  if (strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "Integer") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    if (operand1->len_or_val == 0) {
      puts("Cannot mod by zero.");
    } else {
      int_to_str(operand2->len_or_val % operand1->len_or_val, result_buffer);
      push(operStack, lint(DAT_00016070, result_buffer));
    }
  } else {
    printf("Mod does not make sense on %s and %s\n", operand2->type, operand1->type);
  }
}

// Function: not
void not(void) {
  ValueNode *operand = NULL;
  char result_buffer[32];
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for not.");
    return;
  }
  operand = pop(operStack);

  if (operand == NULL) {
      puts("Error: Missing operand for not after pop.");
      return;
  }

  if (strcmp(operand->type, "String") == 0) {
    if (operand->data_ptr != NULL && strcmp((char *)operand->data_ptr, DAT_00016364) == 0) { // "OMG!"
      push(operStack, lbool(DAT_00016070, DAT_000160c2)); // True
    } else {
      push(operStack, lbool(DAT_00016070, DAT_00016360)); // False
    }
  } else if (strcmp(operand->type, "Integer") == 0) {
    memset(result_buffer, 0, sizeof(result_buffer));
    int_to_str(~operand->len_or_val, result_buffer); // Bitwise NOT
    push(operStack, lint(DAT_00016070, result_buffer));
  } else if (strcmp(operand->type, "Boolean") == 0) {
    if (operand->len_or_val == 1) { // True
      push(operStack, lbool(DAT_00016070, DAT_00016360)); // False
    } else { // False
      push(operStack, lbool(DAT_00016070, DAT_000160c2)); // True
    }
  }
}

// Function: len
void len(void) {
  ValueNode *operand = NULL;
  char result_buffer[32];
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for len.");
    return;
  }
  operand = pop(operStack);

  if (operand == NULL) {
      puts("Error: Missing operand for len after pop.");
      return;
  }

  memset(result_buffer, 0, sizeof(result_buffer));
  // The length for strings is in len_or_val. For integers/booleans, len_or_val holds the value.
  // The original code uses `*(undefined4 *)(local_10 + 0xc)` which is `operand->len_or_val`.
  // This means for non-string types, `len` returns the value itself.
  int_to_str(operand->len_or_val, result_buffer);
  push(operStack, lint(DAT_00016070, result_buffer));
}

// Function: equals
void equals(void) {
  ValueNode *operand1 = NULL; // Corresponds to local_10
  ValueNode *operand2 = NULL; // Corresponds to local_14
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for equality");
    return;
  }
  operand1 = pop(operStack);
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for equality");
    return;
  }
  operand2 = pop(operStack);

  if (operand1 == NULL || operand2 == NULL) {
      puts("Error: Missing operand for equals after pop.");
      push(operStack, lbool(DAT_00016070, DAT_00016360)); // False if any operand is missing
      return;
  }

  int are_equal = 0;

  if ((strcmp(operand1->type, "Integer") == 0 && strcmp(operand2->type, "Integer") == 0) ||
      (strcmp(operand1->type, "Boolean") == 0 && strcmp(operand2->type, "Boolean") == 0)) {
    if (operand1->len_or_val == operand2->len_or_val) { // Compare values directly
      are_equal = 1;
    }
  } else if (strcmp(operand1->type, "String") == 0 && strcmp(operand2->type, "String") == 0) {
    if (operand1->data_ptr != NULL && operand2->data_ptr != NULL &&
        strcmp((char *)operand1->data_ptr, (char *)operand2->data_ptr) == 0) {
      are_equal = 1;
    }
  }
  // All other type combinations are considered unequal by default (implicit in original logic)

  if (are_equal) {
    push(operStack, lbool(DAT_00016070, DAT_000160c2)); // True
  } else {
    push(operStack, lbool(DAT_00016070, DAT_00016360)); // False
  }
}

// Function: type
void type(void) {
  ValueNode *operand = NULL;
  
  if (isEmpty(operStack)) {
    puts("Not enough operands for type. Pushing empty string as type.");
    push(operStack, lstring(DAT_00016070, DAT_00016070)); // Push empty string as type
    return;
  }
  operand = pop(operStack);

  if (operand != NULL) {
    push(operStack, lstring(DAT_00016070, operand->type));
  } else {
    puts("Error: Popped NULL operand for type.");
    push(operStack, lstring(DAT_00016070, DAT_00016070)); // Push empty string as type
  }
}

// Function: terminate (user-defined, not _terminate)
void terminate(void) {
  // The original function is heavily obfuscated by the disassembler.
  // Based on other calls to `_terminate()` in `main` and `add`,
  // it's clear `_terminate()` is the actual program exit function.
  // This user-defined `terminate` function, when called, should simply trigger program exit.
  // Any operand on the stack is ignored as the original code's logic is too garbled to reliably interpret.
  _terminate(); 
}