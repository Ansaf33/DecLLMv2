#include <stdio.h>    // For printf, fprintf
#include <stdlib.h>   // For general utilities (not strictly used, but good practice)
#include <string.h>   // For strcpy, strlen
#include <stdbool.h>  // For bool type
#include <limits.h>   // For INT_MAX, INT_MIN (used in evaluate for overflow checks)

// --- Global Stacks and Pointers ---
#define MAX_STACK_SIZE 100

int num_stack[MAX_STACK_SIZE];
int curr_num_stack = -1; // Top of number stack, -1 means empty

char op_stack[MAX_STACK_SIZE];
int curr_op_stack = -1; // Top of operator stack, -1 means empty

// --- Stack Helper Functions ---

// Pushes an integer value onto the number stack. Returns true on success, false on overflow.
bool push_num(int val) {
    if (curr_num_stack >= MAX_STACK_SIZE - 1) {
        fprintf(stderr, "Error: Number stack overflow\n");
        return false;
    }
    num_stack[++curr_num_stack] = val;
    return true;
}

// Pops an integer value from the number stack into *val. Returns true on success, false on underflow.
bool pop_num(int *val) {
    if (curr_num_stack < 0) {
        // fprintf(stderr, "Error: Number stack underflow\n"); // Can be normal for checking emptiness
        return false;
    }
    *val = num_stack[curr_num_stack--];
    return true;
}

// Peeks at the top integer value on the number stack into *val without removing it. Returns true on success, false if empty.
bool peek_num(int *val) {
    if (curr_num_stack < 0) {
        return false;
    }
    *val = num_stack[curr_num_stack];
    return true;
}

// Pushes a character operator onto the operator stack. Returns true on success, false on overflow.
bool push_op(char op) {
    if (curr_op_stack >= MAX_STACK_SIZE - 1) {
        fprintf(stderr, "Error: Operator stack overflow\n");
        return false;
    }
    op_stack[++curr_op_stack] = op;
    return true;
}

// Pops a character operator from the operator stack into *op. Returns true on success, false on underflow.
bool pop_op(char *op) {
    if (curr_op_stack < 0) {
        // fprintf(stderr, "Error: Operator stack underflow\n"); // Can be normal
        return false;
    }
    *op = op_stack[curr_op_stack--];
    return true;
}

// Peeks at the top character operator on the operator stack into *op without removing it. Returns true on success, false if empty.
bool peek_op(char *op) {
    if (curr_op_stack < 0) {
        return false;
    }
    *op = op_stack[curr_op_stack];
    return true;
}

// --- Other Helper Functions ---

// Converts a string of digits starting at `s` to an integer.
// `*chars_read` will be updated with the number of characters consumed (digits).
// Returns the parsed integer value.
int stoi(const char *s, int *chars_read) {
    int val = 0;
    int i = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        val = val * 10 + (s[i] - '0');
        i++;
    }
    *chars_read = i;
    return val;
}

// Returns the precedence of an operator. Higher value means higher precedence.
// Returns 0 for non-operators or parentheses.
int get_precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0; // For '(' or other non-operators (like stack bottom)
}

// Dummy function: generate_one_equation
// Generates a simple equation string into `buffer` and returns its expected integer result.
int generate_one_equation(char *buffer) {
    static int eq_count = 0;
    eq_count++;
    // Simple dummy equations for testing
    if (eq_count % 3 == 0) {
        strcpy(buffer, "10+5*2"); // Expected: 20
        return 20;
    } else if (eq_count % 3 == 1) {
        strcpy(buffer, "(15-3)/2"); // Expected: 6
        return 6;
    } else {
        strcpy(buffer, "7*8-4"); // Expected: 52
        return 52;
    }
}

// Dummy arrays for predefined tests and their answers
const char *tests[] = {
    "10+5*2",    // 20
    "(15-3)/2",  // 6
    "7*8-4",     // 52
    "20/4+1",    // 6
    "3*(4+5)",   // 27
    "10-2*3",    // 4
    "100/10+5",  // 15
    "2+2*2",     // 6
    "10-(2+3)",  // 5
    "5*5",       // 25
    "10/0",      // Error case, division by zero
    "5+2-3"      // 4
};
int answers[] = {
    20,
    6,
    52,
    6,
    27,
    4,
    15,
    6,
    5,
    25,
    0, // Expected result for 10/0, assuming 0 on error
    4
};
#define NUM_TESTS (sizeof(tests) / sizeof(tests[0]))

// Function: get_pow
// Calculates `exponent_or_count * (base * base)`.
// This is not a standard power function (e.g., base^exponent), but preserves the original logic.
int get_pow(int base, int exponent_or_count) {
    int result = 0;
    if (exponent_or_count == 0) {
        return 0; // As per original logic
    } else {
        for (int i = 1; i <= exponent_or_count; ++i) {
            result += base * base;
        }
    }
    return result;
}

// Function: evaluate
// Performs an arithmetic operation (operand1 `operator_char` operand2) and stores the result in `*result_ptr`.
// Returns true on success, false on overflow, underflow, or division by zero.
bool evaluate(int operand1, char operator_char, int operand2, int *result_ptr) {
    long long res; // Use long long for intermediate calculations to check for overflow

    switch (operator_char) {
        case '+':
            res = (long long)operand1 + operand2;
            if (res > INT_MAX || res < INT_MIN) {
                fprintf(stderr, "Error: Addition overflow/underflow detected for %d %c %d\n", operand1, operator_char, operand2);
                return false;
            }
            *result_ptr = (int)res;
            break;
        case '-':
            res = (long long)operand1 - operand2;
            if (res > INT_MAX || res < INT_MIN) {
                fprintf(stderr, "Error: Subtraction overflow/underflow detected for %d %c %d\n", operand1, operator_char, operand2);
                return false;
            }
            *result_ptr = (int)res;
            break;
        case '*':
            res = (long long)operand1 * operand2;
            if (res > INT_MAX || res < INT_MIN) {
                fprintf(stderr, "Error: Multiplication overflow/underflow detected for %d %c %d\n", operand1, operator_char, operand2);
                return false;
            }
            *result_ptr = (int)res;
            break;
        case '/':
            if (operand2 == 0) {
                fprintf(stderr, "Error: Division by zero detected for %d %c %d\n", operand1, operator_char, operand2);
                return false;
            }
            *result_ptr = operand1 / operand2; // Integer division truncates towards zero
            break;
        default:
            fprintf(stderr, "Error: Unknown operator '%c'\n", operator_char);
            return false;
    }
    return true;
}

// Function: satisfy_paren
// Processes operators on the stack until an opening parenthesis is found or the stack is empty.
// If an opening parenthesis is found, it is popped.
// Returns true on success, false on error (e.g., stack underflow during evaluation).
bool satisfy_paren(void) {
    char op_char;
    int operand1, operand2, result;

    // Process operators until an opening parenthesis is found or the operator stack is empty
    while (peek_op(&op_char) && op_char != '(') {
        if (!pop_op(&op_char)) return false; // Pop current operator

        if (!pop_num(&operand2)) return false; // Pop second operand
        if (!pop_num(&operand1)) return false; // Pop first operand

        if (!evaluate(operand1, op_char, operand2, &result)) return false;
        if (!push_num(result)) return false;
    }

    // If an opening parenthesis is at the top of the stack, pop it (matching the closing parenthesis)
    if (peek_op(&op_char) && op_char == '(') {
        if (!pop_op(&op_char)) return false;
    }
    // If the stack is empty here, it means no matching '(' was found, which is an error in a balanced expression.
    // However, the original code's logic implies this might be a normal exit if no '(' was there to begin with.
    // The `solve_equation` calling context will handle unmatched `(` at the end.
    return true;
}

// Function: solve_equation
// Parses and solves an arithmetic equation string using a shunting-yard-like algorithm.
// Stores the final integer result in `*result_ptr`.
// Returns true on successful evaluation, false on any error (e.g., syntax, stack issues, arithmetic errors).
bool solve_equation(const char *equation_str, int *result_ptr) {
    int current_pos = 0;
    char current_char;
    char op_char;
    int operand1, operand2, result;

    // Reset stacks for each equation evaluation
    curr_num_stack = -1;
    curr_op_stack = -1;

    while ((current_char = equation_str[current_pos]) != '\0') {
        if (current_char == ' ') { // Skip whitespace
            current_pos++;
            continue;
        }

        if (current_char >= '0' && current_char <= '9') {
            int num_len;
            int num_val = stoi(equation_str + current_pos, &num_len);

            // Replicated original decompiler's specific "unary minus" handling:
            // If '-' is on top of the operator stack when a number is parsed,
            // it's treated as a unary minus for that number by converting the '-' to '+'
            // and negating the number. This effectively changes `A - B` to `A + (-B)`.
            char top_op_peek;
            if (peek_op(&top_op_peek) && top_op_peek == '-') {
                if (!pop_op(&top_op_peek)) return false; // Pop the '-'
                if (!push_op('+')) return false;         // Push '+'
                num_val = -num_val;                      // Negate the parsed number
            }
            
            if (!push_num(num_val)) return false;
            current_pos += num_len;
        } else if (current_char == '(') {
            if (!push_op(current_char)) return false;
            current_pos++;
        } else if (current_char == ')') {
            if (!satisfy_paren()) return false; // Process ops inside parentheses
            current_pos++;
        } else if (current_char == '+' || current_char == '-' || current_char == '*' || current_char == '/') {
            // Operator precedence handling:
            // While there's an operator on the stack with higher or equal precedence (and not an opening paren),
            // pop it, pop two numbers, evaluate, and push the result.
            while (peek_op(&op_char) && op_char != '(' &&
                   get_precedence(op_char) >= get_precedence(current_char)) {
                if (!pop_op(&op_char)) return false;
                if (!pop_num(&operand2)) return false;
                if (!pop_num(&operand1)) return false;
                if (!evaluate(operand1, op_char, operand2, &result)) return false;
                if (!push_num(result)) return false;
            }
            if (!push_op(current_char)) return false; // Push the current operator
            current_pos++;
        } else {
            fprintf(stderr, "Error: Invalid character in equation: '%c' at position %d\n", current_char, current_pos);
            return false; // Invalid character found
        }
    }

    // After processing the entire string, evaluate any remaining operators on the stack
    while (curr_op_stack != -1) {
        if (!pop_op(&op_char)) return false;
        if (op_char == '(') { // Unmatched opening parenthesis indicates a syntax error
            fprintf(stderr, "Error: Unmatched opening parenthesis in expression\n");
            return false;
        }
        if (!pop_num(&operand2)) return false;
        if (!pop_num(&operand1)) return false;
        if (!evaluate(operand1, op_char, operand2, &result)) return false;
        if (!push_num(result)) return false;
    }

    // The final result should be the only number left on the number stack
    if (curr_num_stack != 0 || !pop_num(result_ptr)) {
        fprintf(stderr, "Error: Malformed expression or unexpected stack state (number stack count: %d)\n", curr_num_stack + 1);
        return false;
    }

    return true; // Equation solved successfully
}

// Function: run_tests
void run_tests(void) {
    char equation_buffer[256];
    int solved_result;    // To hold results from solve_equation
    int expected_result;  // To hold results from generate_one_equation

    printf("--- Generating and solving random equations ---\n");
    for (int i = 0; i < 1000; ++i) {
        expected_result = generate_one_equation(equation_buffer);
        bool success = solve_equation(equation_buffer, &solved_result);

        printf("r: %d %s = %d ", i, equation_buffer, solved_result);
        if (success) {
            if (solved_result == expected_result) {
                printf("(Correct)\n");
            } else {
                printf("(Wrong! Expected: %d)\n", expected_result);
            }
        } else {
            printf("(Error during evaluation)\n");
        }
    }

    printf("\n--- Running predefined tests ---\n");
    for (int i = 0; i < NUM_TESTS; ++i) {
        bool success = solve_equation(tests[i], &solved_result);
        const char *status_message;

        printf("equation: %s = %d ", tests[i], solved_result);
        if (success) {
            if (solved_result == answers[i]) {
                status_message = "correct\n";
            } else {
                status_message = "wrong (Expected: %d)\n";
                printf(status_message, answers[i]); // Print expected answer for wrong case
                printf("-----------------\n");
                continue; // Skip the generic status message print below
            }
        } else {
            status_message = "error\n";
        }
        printf("%s", status_message); // Use %s for string
        printf("-----------------\n");
    }
    return;
}

// Main function to run the tests
int main() {
    run_tests();
    return 0;
}