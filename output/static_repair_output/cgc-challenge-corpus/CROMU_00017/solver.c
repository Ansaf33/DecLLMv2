#include <stdio.h>   // For printf, sprintf
#include <stdlib.h>  // For rand, srand
#include <time.h>    // For time
#include <string.h>  // For strlen
#include <ctype.h>   // For isdigit, isspace
#include <limits.h>  // For INT_MAX, INT_MIN

// --- Stack implementation ---
#define MAX_STACK_SIZE 100
int num_stack[MAX_STACK_SIZE];
int curr_num_stack = -1; // -1 indicates empty
char op_stack[MAX_STACK_SIZE];
int curr_op_stack = -1; // -1 indicates empty

int push_num(int val) {
    if (curr_num_stack >= MAX_STACK_SIZE - 1) return 0; // Stack full
    num_stack[++curr_num_stack] = val;
    return 1;
}

int pop_num(int* val) {
    if (curr_num_stack < 0) return 0; // Stack empty
    *val = num_stack[curr_num_stack--];
    return 1;
}

int push_op(char op) {
    if (curr_op_stack >= MAX_STACK_SIZE - 1) return 0; // Stack full
    op_stack[++curr_op_stack] = op;
    return 1;
}

int pop_op(char* op) {
    if (curr_op_stack < 0) return 0; // Stack empty
    *op = op_stack[curr_op_stack--];
    return 1;
}

int peek_op(char* op) {
    if (curr_op_stack < 0) return 0; // Stack empty
    *op = op_stack[curr_op_stack];
    return 1;
}

// --- stoi implementation ---
// Converts string to integer, updates length_parsed with number of digits consumed.
int stoi(const char* str, int* length_parsed) {
    int val = 0;
    int len = 0;
    while (isdigit(str[len])) {
        val = val * 10 + (str[len] - '0');
        len++;
    }
    *length_parsed = len;
    return val;
}

// --- generate_one_equation implementation ---
// Generates a simple equation string and returns its result.
int generate_one_equation(char* buffer) {
    int a = rand() % 100 + 1; // Avoid zero for division often
    int b = rand() % 100 + 1;
    char op_char;
    int op_type = rand() % 4; // 0:+, 1:-, 2:*, 3:/
    int result;

    switch (op_type) {
        case 0: op_char = '+'; result = a + b; break;
        case 1: op_char = '-'; result = a - b; break;
        case 2: op_char = '*'; result = a * b; break;
        case 3: op_char = '/'; result = a / b; break; // b is guaranteed > 0
        default: op_char = '+'; result = a + b; break;
    }

    sprintf(buffer, "%d %c %d", a, op_char, b);
    return result;
}

// --- Global test data ---
const char *tests[] = {
    "1+2",
    "10-5",
    "2*3",
    "8/4",
    "(1+2)*3",
    "10-(2+3)",
    "2+3*4",
    "12/3-1",
    "5*(2+3)",
    "20/(4-2)",
    "-5+10",
    "10--5" // This is "10 minus negative 5", which is 10 + 5 = 15
};

// Expected answers for the tests
const int answers[] = {
    3,
    5,
    6,
    2,
    9,
    5,
    14, // 2 + (3*4) = 2 + 12 = 14
    3,  // (12/3) - 1 = 4 - 1 = 3
    25,
    10,
    5,  // -5 + 10 = 5
    15  // 10 - (-5) = 10 + 5 = 15
};

// Function: get_pow
// Implements the original logic: param_2 * (param_1 * param_1)
int get_pow(int base, int exponent) {
  int result = 0;
  if (exponent == 0) {
    result = 0;
  }
  else {
    for (int i = 1; i <= exponent; ++i) {
      result += base * base;
    }
  }
  return result;
}

// Function: evaluate
// Evaluates a binary operation with overflow checks.
// Returns 1 on success, 0 on failure (overflow/div by zero).
int evaluate(int param_1, char param_2, int param_3, int *param_4) {
    long long temp_result; // For overflow checks

    switch (param_2) {
        case '+':
            temp_result = (long long)param_1 + param_3;
            if (temp_result > INT_MAX || temp_result < INT_MIN) {
                return 0; // Overflow
            }
            *param_4 = (int)temp_result;
            return 1;
        case '-':
            temp_result = (long long)param_1 - param_3;
            if (temp_result > INT_MAX || temp_result < INT_MIN) {
                return 0; // Overflow
            }
            *param_4 = (int)temp_result;
            return 1;
        case '*':
            temp_result = (long long)param_1 * param_3;
            if (temp_result > INT_MAX || temp_result < INT_MIN) {
                return 0; // Overflow
            }
            *param_4 = (int)temp_result;
            return 1;
        case '/':
            if (param_3 == 0) {
                return 0; // Division by zero
            }
            // Division overflow can occur with INT_MIN / -1.
            // E.g., if INT_MIN is -2147483648, -INT_MIN is 2147483648, which exceeds INT_MAX.
            if (param_1 == INT_MIN && param_3 == -1) {
                return 0; // Overflow
            }
            *param_4 = param_1 / param_3;
            return 1;
        default:
            return 0; // Unknown operator
    }
}

// Returns operator precedence. Higher number means higher precedence.
// Unary minus ('_') has highest precedence.
int get_precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '_') return 3; // Special symbol for unary minus
    return 0; // For '(' or other non-operators
}

// Applies an operator from the op_stack to numbers from the num_stack.
// Returns 1 on success, 0 on failure.
int apply_operator(char op, int* result_val) {
    int num1, num2;

    if (op == '_') { // Unary minus
        if (!pop_num(&num1)) return 0;
        *result_val = -num1;
        return 1;
    } else { // Binary operator
        if (!pop_num(&num2)) return 0; // Right operand
        if (!pop_num(&num1)) return 0; // Left operand
        return evaluate(num1, op, num2, result_val);
    }
}

// Function: solve_equation
// Solves an arithmetic expression using a shunting-yard like algorithm.
// Returns 1 on success, 0 on failure (syntax error, div by zero, overflow).
int solve_equation(const char *equation_str, int *result_ptr) {
    int current_pos = 0;
    curr_num_stack = -1; // Reset stacks for new equation
    curr_op_stack = -1;

    char current_char;
    int result_val;

    // Flag to help distinguish unary minus: true if last token was a number or ')'
    int last_token_was_operand = 0;

    while (current_pos < strlen(equation_str)) {
        current_char = equation_str[current_pos];

        if (isspace(current_char)) {
            current_pos++;
            continue;
        }

        if (isdigit(current_char)) {
            int num_val;
            int parsed_len;
            num_val = stoi(equation_str + current_pos, &parsed_len);
            if (!push_num(num_val)) return 0;
            last_token_was_operand = 1;
            current_pos += parsed_len;
            continue;
        }

        if (current_char == '(') {
            if (!push_op(current_char)) return 0;
            last_token_was_operand = 0; // An opening parenthesis means the next operator could be unary
            current_pos++;
            continue;
        }

        if (current_char == ')') {
            // Evaluate everything inside the parentheses
            while (curr_op_stack != -1 && op_stack[curr_op_stack] != '(') {
                if (!apply_operator(op_stack[curr_op_stack], &result_val)) return 0;
                pop_op(&current_char); // Pop the applied operator
                if (!push_num(result_val)) return 0;
            }
            if (curr_op_stack == -1 || op_stack[curr_op_stack] != '(') return 0; // Mismatched parentheses
            pop_op(&current_char); // Pop the '('
            last_token_was_operand = 1; // A closing parenthesis means the next operator is binary
            current_pos++;
            continue;
        }

        // Handle operators (+, -, *, /)
        if (current_char == '+' || current_char == '-' || current_char == '*' || current_char == '/') {
            char op_to_push = current_char;

            // Determine if it's a unary minus: if current_char is '-' and previous token was not an operand
            if (current_char == '-' && !last_token_was_operand) {
                op_to_push = '_'; // Use special unary minus operator
            }

            // Apply shunting-yard logic for operator precedence
            while (curr_op_stack != -1 && op_stack[curr_op_stack] != '(' && get_precedence(op_stack[curr_op_stack]) >= get_precedence(op_to_push)) {
                if (!apply_operator(op_stack[curr_op_stack], &result_val)) return 0;
                pop_op(&current_char);
                if (!push_num(result_val)) return 0;
            }
            if (!push_op(op_to_push)) return 0;
            last_token_was_operand = 0; // An operator means the next could be unary (if it's '-')
            current_pos++;
            continue;
        }

        return 0; // Unrecognized character
    }

    // After parsing the entire string, evaluate any remaining operators
    while (curr_op_stack != -1) {
        if (!apply_operator(op_stack[curr_op_stack], &result_val)) return 0;
        pop_op(&current_char);
        if (!push_num(result_val)) return 0;
    }

    // The final result should be the only number left on the number stack
    if (pop_num(result_ptr) && curr_num_stack == -1) {
        return 1; // Success
    }
    return 0; // Failed (e.g., stack not empty or empty)
}

// Function: run_tests
void run_tests(void) {
    char equation_buffer[256];
    int solved_value; // Result of solve_equation

    srand(time(NULL)); // Seed for random equation generation

    // Loop for generating and printing equations
    for (int i = 0; i < 1000; ++i) {
        int generated_result = generate_one_equation(equation_buffer);
        printf("r: %d %s = %d\n", i, equation_buffer, generated_result);
    }

    // Loop for solving predefined test equations (0xc is 12 in decimal)
    for (int i = 0; i < 12; ++i) {
        int success = solve_equation(tests[i], &solved_value);
        printf("equation: %s = %d ", tests[i], solved_value);

        if (success && answers[i] == solved_value) {
            printf("correct\n");
        } else {
            printf("wrong (expected %d, got %d)\n", answers[i], solved_value);
        }
        printf("-----------------\n");
    }
    return;
}

// main function
int main() {
    run_tests();
    return 0;
}