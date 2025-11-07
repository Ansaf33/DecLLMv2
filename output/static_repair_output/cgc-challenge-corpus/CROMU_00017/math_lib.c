#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // For strlen, strncpy, memset, memcpy
#include <ctype.h>    // For isdigit
#include <unistd.h>   // For read, STDIN_FILENO
#include <time.h>     // For time (to seed rand)

// --- Global variables as inferred from usage ---
// `acceptable_char` used in `isValidChar` and implicitly by `parse_input`.
// It should include digits, operators, and parentheses.
// `isValidChar` checks `acceptable_char[i]` where `i` goes from 0 up to `sizeof(acceptable_char) - 2`.
const char acceptable_char[] = "0123456789+-*/()"; // 16 characters + null terminator
// `operator_list` used in `generate_one_equation` and `prompt_for_equation`.
const char operator_list[] = "+-*/"; // 4 characters + null terminator

// --- Mock/Helper functions to replace custom ones ---

// Mimics random_in_range(min, max)
int random_in_range(int min, int max) {
    if (min > max) return min; // Handle invalid range gracefully
    return min + rand() % (max - min + 1);
}

// Mimics itos(num, buffer)
void itos(int num, char* buffer) {
    sprintf(buffer, "%d", num);
}

// Mimics get_str_end(s)
size_t get_str_end(const char* s) {
    return strlen(s);
}

// Mimics solve_equation(equation, result_ptr)
// A dummy implementation for compilation. In a real scenario, this would parse and evaluate.
// It needs to return 1 (success) and a non-zero *result for generate_one_equation's loop.
int solve_equation(const char* equation, int* result) {
    // Simple heuristic for a non-zero dummy result
    *result = (int)(strlen(equation) % 10) + 1;
    return 1; // Always successful for mock
}

// Mimics sprng(s1, s2)
void sprng(unsigned int s1, unsigned int s2) {
    srand(s1 ^ s2); // Combine two seeds for srand
}

// Mimics stoi(s, chars_read_ptr)
int stoi(const char* s, int* chars_read) {
    char* endptr;
    long val = strtol(s, &endptr, 10);
    if (chars_read) {
        *chars_read = (int)(endptr - s);
    }
    return (int)val;
}

// --- Original functions, fixed and reduced ---

// Function: get_user_answer
int get_user_answer(int *answer_ptr) { // param_1 -> answer_ptr
  char buffer[16];
  memset(buffer, 0, sizeof(buffer)); // Clear the whole buffer
  
  // Read into buffer+1 to leave buffer[0] unused, and ensure space for null terminator
  ssize_t bytes_read = read(STDIN_FILENO, buffer + 1, sizeof(buffer) - 2);
  if (bytes_read <= 0) { // If read failed or no bytes read (EOF)
    return 0;
  }

  // Ensure null termination at the end of actual read bytes
  buffer[1 + bytes_read] = '\0';
  
  size_t len = strlen(buffer + 1); // Get length of string starting at buffer[1]
  if (len > 0 && (buffer + 1)[len - 1] == '\n') {
    (buffer + 1)[len - 1] = '\0'; // Remove trailing newline
    len--; // Adjust length
  }

  for (size_t i = 0; i < len; ++i) {
    if (!isdigit((unsigned char)(buffer + 1)[i]) && ((buffer + 1)[i] != '-')) {
      printf("incorrect item entered\n");
      return 0;
    }
  }
  *answer_ptr = atoi(buffer + 1);
  return 1;
}

// Function: get_user_equation
int get_user_equation(char *equation_buffer) { // param_1 -> equation_buffer
  char buffer[257];
  memset(buffer, 0, sizeof(buffer)); // Clear the whole buffer
  
  // Read into buffer+1 to leave buffer[0] unused, and ensure space for null terminator
  ssize_t bytes_read = read(STDIN_FILENO, buffer + 1, sizeof(buffer) - 2);
  if (bytes_read <= 0) { // If read failed or no bytes read (EOF)
    return 0;
  }

  buffer[1 + bytes_read] = '\0'; // Null-terminate at the end of actual read bytes

  size_t len = strlen(buffer + 1);
  if (len > 0 && (buffer + 1)[len - 1] == '\n') {
    strncpy(equation_buffer, buffer + 1, len - 1);
    equation_buffer[len - 1] = '\0'; // Ensure destination is null-terminated
  } else {
    strncpy(equation_buffer, buffer + 1, len);
    equation_buffer[len] = '\0'; // Ensure destination is null-terminated
  }
  return 1;
}

// Function: isValidChar
int isValidChar(char c) { // param_1 -> c
  for (size_t i = 0; i < sizeof(acceptable_char) - 1; ++i) { // Loop through defined acceptable chars
    if (c == acceptable_char[i]) {
      return 1;
    }
  }
  return 0;
}

// Function: parse_input
int parse_input(const char *equation) { // param_1 -> equation
  size_t len = get_str_end(equation);
  for (size_t i = 0; i < len; ++i) {
    if (isValidChar(equation[i]) == 0) {
      return 0;
    }
  }
  return 1;
}

// Function: generate_one_equation
int generate_one_equation(char *equation_buffer) { // param_1 -> equation_buffer
  int result_val;
  int success_flag;
  
  do {
    char last_op_char = '\0';
    int current_num_val = 1;
    int current_offset = 0;
    int num_tokens = random_in_range(4, 0xf); // 4 to 15 tokens
    int open_paren_target = random_in_range(0, num_tokens);
    int open_paren_count = 0;
    int close_paren_count = 0;
    int is_operand_expected = 0; // 0 for number, 1 for operator
    
    // Loop for generating tokens (numbers and operators)
    for (int i = 0; i < num_tokens + 1; ++i) { // Iterates num_tokens + 1 times for alternating numbers/operators
      if (is_operand_expected == 0) { // Expecting a number
        if ((i < (num_tokens + 1) / 2) && (open_paren_count < open_paren_target)) {
          equation_buffer[current_offset++] = '(';
          open_paren_count++;
        }
        
        int rand_num = random_in_range(1, 0x100); // 1 to 256
        if (last_op_char == '/') {
          // Prevent division by zero or very small numbers
          rand_num = random_in_range(1, current_num_val > 1 ? current_num_val : 1);
        }
        current_num_val = rand_num;
        
        char num_str[16]; // Buffer for integer to string conversion
        itos(rand_num, num_str);
        size_t num_str_len = strlen(num_str);
        memcpy(equation_buffer + current_offset, num_str, num_str_len);
        current_offset += num_str_len;

        if (((num_tokens + 1) / 2 <= i) && (close_paren_count < open_paren_count)) {
          equation_buffer[current_offset++] = ')';
          close_paren_count++;
        }
        is_operand_expected = 1; // Next token expected is an operator
      } else { // Expecting an operator
        char op_char = operator_list[random_in_range(0, 3)]; // Randomly pick from "+-*/"
        equation_buffer[current_offset++] = op_char;
        is_operand_expected = 0; // Next token expected is a number
        last_op_char = op_char;
      }
    }
    
    // Close any remaining open parentheses
    while (close_paren_count < open_paren_count) {
      equation_buffer[current_offset++] = ')';
      close_paren_count++;
    }
    equation_buffer[current_offset] = '\0'; // Null-terminate the generated string
    
    success_flag = solve_equation(equation_buffer, &result_val);
  } while ((success_flag != 1) || (result_val == 0)); // Loop until a solvable equation with non-zero result is generated
  
  return result_val;
}

// Function: generate_equation
int generate_equation(void) {
  char equation_buffer[256];
  int user_answer = 0;
  
  int correct_answer = generate_one_equation(equation_buffer);
  printf("Equation: %s\n", equation_buffer);
  printf("gimme answer: ");
  
  if (get_user_answer(&user_answer) == 1) {
    if (correct_answer == user_answer) {
      printf("success!!\n");
      return 1;
    } else {
      printf("Incorrect answer\n");
      return 0;
    }
  } else {
    printf("Bad input\n");
    return 0;
  }
}

// Function: seed_prng
int seed_prng(void) {
  char buffer[256];
  memset(buffer, 0, sizeof(buffer));
  printf("Enter some data\n");
  
  ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1); // Read up to 255 bytes
  if (bytes_read <= 0) { // If read failed or no bytes read (EOF)
    return 0;
  }
  buffer[bytes_read] = '\0'; // Ensure null termination

  unsigned int s1 = 0xffffffff;
  unsigned int s2 = 0xffffffff;
  for (ssize_t i = 0; i < bytes_read; ++i) {
    unsigned int uVar1_temp = s1 >> 0x1c;
    s1 = (unsigned int)buffer[i] ^ (s1 << 4 | s2 >> 0x1c);
    s2 = (unsigned int)buffer[i] >> 0x1f ^ (s2 << 4 | uVar1_temp);
  }
  sprng(s1, s2);
  return 1;
}

// Function: prompt_for_equation
int prompt_for_equation(void) {
  char user_equation[256];
  memset(user_equation, 0, sizeof(user_equation));

  int expected_paren_count = random_in_range(0, 5);
  int expected_result = random_in_range(0, 0x8000); // 0 to 32768
  char expected_operator = operator_list[random_in_range(0, 3)];

  int required_numbers[4];
  for (int i = 0; i < 4; ++i) {
    required_numbers[i] = random_in_range(1, 0x100); // 1 to 256
  }

  printf("Enter an equation that has %d sets of parenthesis\n", expected_paren_count);
  printf("It must evaluate to %d and contain the %c operator\n", expected_result, expected_operator);
  printf("and must use the numbers: %d %d %d and %d\n", required_numbers[0], required_numbers[1], required_numbers[2], required_numbers[3]);

  int actual_paren_count = 0;
  int actual_operator_count = 0;
  int number_usage_count[4] = {0};

  if (get_user_equation(user_equation) != 1) {
    return 0;
  }

  if (parse_input(user_equation) != 1) {
    printf("string is formatted incorrect\n");
    return 0;
  }
  printf("string is formatted correct\n");

  size_t eq_len = strlen(user_equation);
  for (size_t i = 0; i < eq_len; ++i) {
    if (user_equation[i] == '(') {
      actual_paren_count++;
    }
    if (expected_operator == user_equation[i]) {
      actual_operator_count++;
    }
    // Check for numbers (digits '0' through '9')
    if ((user_equation[i] >= '0') && (user_equation[i] <= '9')) {
      int num_chars_read = 0;
      int num_from_equation = stoi(user_equation + i, &num_chars_read);
      for (int j = 0; j < 4; ++j) {
        if (num_from_equation == required_numbers[j]) {
          number_usage_count[j]++;
        }
      }
      i += num_chars_read - 1; // Advance loop counter by number length - 1 (since i increments again in loop header)
    }
  }

  if (actual_paren_count != expected_paren_count) {
    printf("Incorrect number of parenthesis\n");
    return 0;
  }
  
  if (actual_operator_count == 0) {
    printf("Did not use %c operator\n", expected_operator);
    return 0;
  }
  
  for (int i = 0; i < 4; ++i) {
    if (number_usage_count[i] == 0) {
      printf("Did not use number %d\n", required_numbers[i]);
      return 0;
    }
  }

  int solved_result = 0;
  if (solve_equation(user_equation, &solved_result) != 1) {
    printf("Invalid equation format\n");
    return 0;
  }
  
  if (expected_result == solved_result) {
    printf("%s does resolve to %d, good!\n", user_equation, solved_result);
    return 1;
  } else {
    printf("incorrect answer: %d\n", solved_result);
    return 0;
  }
}