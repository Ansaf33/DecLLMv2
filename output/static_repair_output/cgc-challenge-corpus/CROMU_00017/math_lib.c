#include <stdio.h>    // For printf
#include <string.h>   // For strlen, strncpy, memset, memcpy
#include <stdlib.h>   // For atoi
#include <ctype.h>    // For isdigit
#include <unistd.h>   // For read, STDIN_FILENO
#include <strings.h>  // For bzero (though memset is used)

// Type definitions from original context (assuming 4-byte int and char)
typedef int undefined4;
typedef unsigned int uint;
typedef char undefined;

// Forward declarations for custom/external functions
extern int random_in_range(int min, int max);
extern void itos(int num, char *buffer); // Converts int to string
extern int get_str_end(const char *str); // Assumed to be equivalent to strlen, returning int
extern int solve_equation(const char *equation, int *result);
extern void sprng(unsigned int seed1, unsigned int seed2);
extern int stoi(const char *str, int *length_parsed); // Converts string to int, returns int value and parsed length
extern char acceptable_char[]; // Array of acceptable characters for equations
extern char operator_list[];   // Array of operators

// Helper function to mimic the "receive" behavior as interpreted from the snippet.
// Returns 0 on success (read call was fine, actual_bytes_read will hold count), 1 on failure (read error).
static int read_input_wrapper(char *buffer, size_t max_len, ssize_t *actual_bytes_read) {
    ssize_t n = read(STDIN_FILENO, buffer, max_len);
    if (n < 0) {
        perror("read");
        *actual_bytes_read = 0; // Indicate no bytes read on error
        return 1; // Failure
    }
    *actual_bytes_read = n; // Store the actual number of bytes read
    return 0; // Success (even if n=0 for EOF, the original logic proceeds with 0 bytes)
}

// Function: get_user_answer
undefined4 get_user_answer(int *param_1) {
  char buffer[16]; // Max 0xe bytes + 1 for null-terminator + 1 for initial offset
  ssize_t actual_bytes_read;

  memset(buffer + 1, 0, 0xf); // Clear buffer[1] to buffer[15]
  
  // `read_input_wrapper` returns 0 on success (read call was fine), 1 on error.
  // The original `if (local_18 == 0)` implies success.
  if (read_input_wrapper(buffer + 1, 0xe, &actual_bytes_read) == 0) {
    // If read was successful, null-terminate the buffer.
    // buffer+1 starts at index 1. Max read 0xe bytes goes up to index 1+0xe-1 = 14.
    // Null terminator at 1+actual_bytes_read. Max 1+14 = 15. buffer[15] is valid.
    buffer[1 + actual_bytes_read] = '\0'; 
    
    size_t len = strlen(buffer + 1);
    // Remove trailing newline if present
    if (len > 0 && (buffer + 1)[len - 1] == '\n') {
      (buffer + 1)[--len] = '\0';
    }

    // Validate characters
    for (size_t i = 0; i < len; ++i) {
      if (!isdigit((int)(buffer + 1)[i]) && ((buffer + 1)[i] != '-')) {
        printf("incorrect item entered\n");
        return 0; // Invalid character found
      }
    }
    
    *param_1 = atoi(buffer + 1);
    return 1; // Success
  }
  
  return 0; // Read error (read_input_wrapper returned non-zero)
}

// Function: get_user_equation
undefined4 get_user_equation(char *param_1) {
  char buffer[257]; // Max 0xff bytes + 1 for null-terminator + 1 for initial offset
  ssize_t actual_bytes_read;

  memset(buffer + 1, 0, 0x100); // Clear buffer[1] to buffer[256]
  
  // `read_input_wrapper` returns 0 on success (read call was fine), 1 on error.
  if (read_input_wrapper(buffer + 1, 0xff, &actual_bytes_read) == 0) {
    // Null-terminate the string. Max 0xff bytes read goes up to index 1+0xff-1 = 255.
    // Null terminator at 1+actual_bytes_read. Max 1+255 = 256. buffer[256] is valid.
    buffer[1 + actual_bytes_read] = '\0'; 

    size_t len = strlen(buffer + 1);
    
    // Check for trailing newline and copy
    if (len > 0 && (buffer + 1)[len - 1] == '\n') {
      strncpy(param_1, buffer + 1, len - 1);
      param_1[len - 1] = '\0'; // Null-terminate
    } else {
      strncpy(param_1, buffer + 1, len);
      param_1[len] = '\0'; // Null-terminate
    }
    return 1; // Success
  }
  return 0; // Read error
}

// Function: isValidChar
undefined4 isValidChar(char param_1) {
  for (unsigned int i = 0; i <= 0x10; ++i) { // Loop up to and including index 0x10
    if (param_1 == acceptable_char[i]) {
      return 1;
    }
  }
  return 0;
}

// Function: parse_input
undefined4 parse_input(char *param_1) { // Changed param_1 type to char*
  int str_len = get_str_end(param_1); // Assumed to be strlen
  for (int i = 0; i < str_len; ++i) { // Loop from 0 to str_len - 1
    if (isValidChar(param_1[i]) == 0) {
      return 0; // Invalid character found
    }
  }
  return 1; // All characters are valid
}

// Function: generate_one_equation
int generate_one_equation(char *param_1) { // Changed param_1 type to char*
  int result_val = 0;
  int solve_status; // Declared outside loop to be used in condition

  do {
    char last_op = '\0';
    int prev_num_val = 1; // Used for division constraint (divisor != 0)
    int current_pos = 0; // Current write position in param_1

    int num_ops_target = random_in_range(4, 0xf); // Number of operators
    int total_elements = num_ops_target + 1; // Total numbers + operators
    int num_open_target = random_in_range(0, num_ops_target); // Number of opening parentheses

    int num_open_count = 0;  // Count of currently open parentheses
    int num_closed_count = 0; // Count of closed parentheses
    
    int is_operator_next = 0; // 0 for number, 1 for operator

    for (int k = 0; k < total_elements; ++k) {
      if (!is_operator_next) { // Expecting a number
        if ((k < total_elements / 2) && (num_open_count < num_open_target)) {
          param_1[current_pos++] = '(';
          num_open_count++;
        }
        
        int current_num = random_in_range(1, 0x100);
        if (last_op == '/') {
          // Ensure current_num is a divisor of prev_num_val (or just not zero for simplicity)
          // Original code ensures divisor is smaller than dividend, but not necessarily a factor.
          // For integer arithmetic, this needs to be carefully handled.
          // Assuming random_in_range(1, prev_num_val) implies a non-zero divisor.
          current_num = random_in_range(1, prev_num_val); 
        }
        prev_num_val = current_num; // Store current number for next division check

        char num_str[15]; // Buffer for integer to string conversion
        itos(current_num, num_str);
        int num_str_len = get_str_end(num_str); // Assumed strlen

        memcpy(param_1 + current_pos, num_str, num_str_len);
        current_pos += num_str_len;

        if ((total_elements / 2 <= k) && (num_closed_count < num_open_count)) {
          param_1[current_pos++] = ')';
          num_closed_count++;
        }
        is_operator_next = 1; // Next will be an operator
      } else { // Expecting an operator
        int op_idx = random_in_range(0, 3);
        char op_char = operator_list[op_idx];
        param_1[current_pos++] = op_char;
        is_operator_next = 0; // Next will be a number
        last_op = op_char;
      }
    }

    // Close any remaining open parentheses
    while (num_closed_count < num_open_count) {
      param_1[current_pos++] = ')';
      num_closed_count++;
    }
    
    param_1[current_pos] = '\0'; // Null-terminate the equation string

    solve_status = solve_equation(param_1, &result_val); // Call solve_equation once
    // Loop until equation is valid (solve_status == 1) and result is not 0
  } while ((solve_status != 1) || (result_val == 0));

  return result_val;
}

// Function: generate_equation
undefined4 generate_equation(void) {
  char equation[256];
  int correct_answer = generate_one_equation(equation);
  int user_answer = 0;
  
  printf("Equation: %s\n", equation); // Fixed format string
  printf("gimme answer: ");
  
  if (get_user_answer(&user_answer) == 1) { // get_user_answer returns 1 on success
    if (correct_answer == user_answer) {
      printf("success!!\n"); // Removed extra arg
      return 1;
    } else {
      printf("Incorrect answer\n"); // Removed extra arg
      return 0;
    }
  } else {
    printf("Bad input\n"); // Removed extra arg
    return 0;
  }
}

// Function: seed_prng
undefined4 seed_prng(void) {
  char buffer[256];
  ssize_t actual_bytes_read; // For actual bytes read by `read`
  
  memset(buffer, 0, sizeof(buffer));
  printf("Enter some data\n");
  
  // `read_input_wrapper` returns 0 on success, 1 on error.
  if (read_input_wrapper(buffer, 0xff, &actual_bytes_read) == 0) {
    unsigned int seed1 = 0xffffffff;
    unsigned int seed2 = 0xffffffff;
    
    // Loop up to actual_bytes_read
    for (size_t i = 0; i < (size_t)actual_bytes_read; ++i) {
      unsigned int temp_seed = seed1 >> 0x1c;
      seed1 = (unsigned int)buffer[i] ^ (seed1 << 4 | seed2 >> 0x1c);
      seed2 = (unsigned int)buffer[i] >> 0x1f ^ (seed2 << 4 | temp_seed);
    }
    sprng(seed1, seed2);
    return 1; // Success
  } else {
    return 0; // Read error
  }
}

// Function: prompt_for_equation
undefined4 prompt_for_equation(void) {
  char equation_buffer[256];
  int target_numbers[4];
  int used_numbers[4] = {0, 0, 0, 0}; // Initialize to zero

  memset(equation_buffer, 0, sizeof(equation_buffer));
  
  int target_parens = random_in_range(0, 5);
  int target_result = random_in_range(0, 0x8000);
  char target_operator = operator_list[random_in_range(0, 3)];

  for (int i = 0; i < 4; ++i) {
    target_numbers[i] = random_in_range(1, 0x100);
  }
  
  printf("Enter an equation that has %d sets of parenthesis\n", target_parens); // Fixed format string
  printf("It must evaluate to %d and contain the %c operator\n", target_result, target_operator); // Fixed format string
  printf("and must use the numbers: %d %d %d and %d\n", target_numbers[0], target_numbers[1], target_numbers[2], target_numbers[3]); // Fixed format string

  int paren_count = 0;
  int operator_count = 0;
  
  if (get_user_equation(equation_buffer) == 1) { // get_user_equation returns 1 on success
    int parse_status = parse_input(equation_buffer);
    printf("string is formatted %s\n", parse_status == 1 ? "correct" : "incorrect"); // Fixed format string and simplified

    size_t eq_len = strlen(equation_buffer);
    for (size_t k = 0; k < eq_len; ++k) {
      if (equation_buffer[k] == '(') {
        paren_count++;
      }
      if (target_operator == equation_buffer[k]) {
        operator_count++;
      }
      // Check for numbers (digits)
      if (equation_buffer[k] >= '0' && equation_buffer[k] <= '9') { // Simplified digit check
        int num_len = 0;
        int parsed_num = stoi(equation_buffer + k, &num_len);
        for (int j = 0; j < 4; ++j) {
          if (parsed_num == target_numbers[j]) {
            used_numbers[j]++;
          }
        }
        k += num_len - 1; // Advance k by the length of the parsed number
      }
    }

    if (paren_count == target_parens) {
      if (operator_count == 0) { // Check operator_count *after* the loop
        printf("Did not use %c operator\n", target_operator); // Fixed format string
        return 0;
      } else {
        for (int m = 0; m < 4; ++m) {
          if (used_numbers[m] == 0) {
            printf("Did not use number %d\n", target_numbers[m]); // Fixed format string
            return 0;
          }
        }
        
        int actual_result = 0;
        int solve_status = solve_equation(equation_buffer, &actual_result);
        if (solve_status == 1) {
          if (target_result == actual_result) {
            printf("%s does resolve to %d, good!\n", equation_buffer, actual_result); // Fixed format string
            return 1;
          } else {
            printf("incorrect answer: %d\n", actual_result); // Fixed format string
            return 0;
          }
        } else {
          printf("Invalid equation format\n");
          return 0;
        }
      }
    } else {
      printf("Incorrect number of parenthesis\n"); // Removed extra arg
      return 0;
    }
  } else {
    return 0; // get_user_equation failed
  }
}