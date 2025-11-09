#include <stdlib.h> // for malloc, free, atof, realloc
#include <string.h> // for strlen
#include <stdio.h>  // for NULL
#include <math.h>   // for fabs, cos, log, log10, pow, remainder, sqrt, isnan
#include <stddef.h> // for size_t

// Type aliases for clarity
typedef long double long_double_t;
typedef unsigned int uint_t;
typedef unsigned long long ulonglong_t;

// Placeholder for DAT_00017150, which appears to be 2.0 based on usage
const double DAT_00017150 = 2.0;

// Define a node for a stack of strings
typedef struct StackNode {
    char *data;
    struct StackNode *next;
} StackNode;

// Dummy pop_copy function
// stack_head_ptr: A pointer to the stack head (StackNode**).
// error_flag: An optional error flag (int*).
// The original code uses pop_copy(param_1) and pop_copy(param_1, iVar2, piVar4).
// The 'iVar2' (size_t) argument is ignored as it doesn't fit standard stack pop logic.
// 'piVar4' is interpreted as the error_flag.
char* pop_copy(StackNode **stack_head_ptr, int *error_flag) {
    if (stack_head_ptr == NULL || *stack_head_ptr == NULL) {
        if (error_flag) *error_flag = 1; // Indicate error/empty
        return NULL;
    }
    StackNode *node = *stack_head_ptr;
    char *data = node->data; // The string data
    *stack_head_ptr = node->next; // Move head to next node
    free(node); // Free the node itself
    return data; // Return the string data (caller is responsible for freeing it later)
}

// Dummy push function
// stack_head_ptr: A pointer to the stack head (StackNode**).
// data: The string to push (char*). This string should be malloc'd if it's owned by the stack.
// error_flag: An optional error flag (int*).
void push(StackNode **stack_head_ptr, char *data, int *error_flag) {
    if (stack_head_ptr == NULL) {
        if (error_flag) *error_flag = 1;
        return;
    }
    StackNode *newNode = (StackNode *)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        if (error_flag) *error_flag = 1;
        return;
    }
    newNode->data = data; // Assume data is already allocated and owned by the stack now
    newNode->next = *stack_head_ptr;
    *stack_head_ptr = newNode;
    if (error_flag) *error_flag = 0; // Success
}

// Dummy clear_stack function
void clear_stack(StackNode **stack_head_ptr) {
    if (stack_head_ptr == NULL) return;
    StackNode *current = *stack_head_ptr;
    while (current != NULL) {
        StackNode *next = current->next;
        free(current->data); // Free the string data
        free(current);       // Free the node itself
        current = next;
    }
    *stack_head_ptr = NULL;
}

// Helper function to pop a string, convert to double, and handle errors
// Returns 1 on success, 0 on failure (error_flag set)
int get_double_from_stack(StackNode **stack_head, int *error_flag, double *out_val) {
    char *s = pop_copy(stack_head, error_flag);
    if (s == NULL || *error_flag != 0) {
        return 0; // Failed to pop or error already set
    }
    *out_val = atof(s);
    free(s); // Free the string after conversion
    return 1; // Success
}

// Function: handle_op_avg
long_double_t handle_op_avg(StackNode **stack_head, int *error_flag) {
  double sum = 0.0;
  uint_t count = 0;
  char *current_str;

  if (*stack_head != NULL) {
    current_str = pop_copy(stack_head, error_flag);
    while (current_str != NULL && *error_flag == 0) {
      sum += atof(current_str);
      free(current_str);
      count++;
      current_str = pop_copy(stack_head, error_flag);
    }
  }

  if (*error_flag != 0 || count == 0) {
    clear_stack(stack_head);
    *error_flag = 1;
    if (count == 0) count = 1; // Prevent division by zero, result will be 0.0
    sum = 0.0;
  }
  
  return (long_double_t)sum / (long_double_t)count;
}

// Function: handle_op_count
long_double_t handle_op_count(StackNode **stack_head, int *error_flag) {
  uint_t count = 0;
  char *current_str;

  if (*stack_head != NULL) {
    current_str = pop_copy(stack_head, error_flag);
    while (current_str != NULL && *error_flag == 0) {
      atof(current_str); // Value is read but not used for count
      free(current_str);
      count++;
      current_str = pop_copy(stack_head, error_flag);
    }
  }

  if (*error_flag != 0 || count == 0) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  
  return (long_double_t)count;
}

// Function: handle_op_max
long_double_t handle_op_max(StackNode **stack_head, int *error_flag) {
  double max_val = 0.0;
  char *current_str;
  int found_any_value = 0;

  if (*stack_head != NULL) {
    current_str = pop_copy(stack_head, error_flag);
    if (current_str != NULL && *error_flag == 0) {
      max_val = atof(current_str);
      free(current_str);
      found_any_value = 1;
    } else {
        free(current_str); // Free if not NULL but error occurred
    }
  }
  
  if (*error_flag == 0 && found_any_value) {
    current_str = pop_copy(stack_head, error_flag);
    while (current_str != NULL && *error_flag == 0) {
      double val = atof(current_str);
      free(current_str);
      if (max_val < val) {
        max_val = val;
      }
      current_str = pop_copy(stack_head, error_flag);
    }
  }

  if (*error_flag != 0 || !found_any_value) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  
  return (long_double_t)max_val;
}

// Function: handle_op_median
long_double_t handle_op_median(StackNode **stack_head, int *error_flag) {
  StackNode *temp_stack_head = NULL; // Head of the sorted linked list
  uint_t count = 0;
  double median_val = 0.0;
  char *current_str;

  if (*stack_head != NULL) {
    current_str = pop_copy(stack_head, error_flag);
    while (current_str != NULL && *error_flag == 0) {
      double current_val = atof(current_str);
      
      StackNode *newNode = (StackNode *)malloc(sizeof(StackNode));
      if (newNode == NULL) {
          free(current_str); // Must free if not pushed
          *error_flag = 1;
          break; // Memory allocation error
      }
      newNode->data = current_str; // newNode takes ownership of current_str
      newNode->next = NULL;

      if (temp_stack_head == NULL || atof(temp_stack_head->data) >= current_val) {
        // Insert at head
        newNode->next = temp_stack_head;
        temp_stack_head = newNode;
      } else {
        // Find insertion point
        StackNode *current = temp_stack_head;
        while (current->next != NULL && atof(current->next->data) < current_val) {
          current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
      }
      count++;
      current_str = pop_copy(stack_head, error_flag);
    }
  }

  // Cleanup original stack in case of error
  if (*error_flag != 0) {
    clear_stack(stack_head);
    clear_stack(&temp_stack_head); // Clear temporary stack as well
    *error_flag = 1;
    return 0.0L;
  }

  // Calculate median from sorted list
  if (count == 0) {
    clear_stack(stack_head); // Original also clears param_1 here
    *error_flag = 1;
    return 0.0L;
  }

  StackNode *current = temp_stack_head;
  uint_t mid_idx = count / 2;

  if (count % 2 != 0) { // Odd number of elements
    for (uint_t i = 0; i < mid_idx; i++) {
      current = current->next;
    }
    median_val = atof(current->data);
  } else { // Even number of elements
    for (uint_t i = 0; i < mid_idx - 1; i++) {
      current = current->next;
    }
    median_val = atof(current->data);
    current = current->next;
    median_val = (median_val + atof(current->data)) / DAT_00017150; // DAT_00017150 is 2.0
  }

  clear_stack(&temp_stack_head); // Free the temporary sorted list
  return (long_double_t)median_val;
}

// Function: handle_op_min
long_double_t handle_op_min(StackNode **stack_head, int *error_flag) {
  double min_val = 0.0;
  char *current_str;
  int found_any_value = 0;

  if (*stack_head != NULL) {
    current_str = pop_copy(stack_head, error_flag);
    if (current_str != NULL && *error_flag == 0) {
      min_val = atof(current_str);
      free(current_str);
      found_any_value = 1;
    } else {
        free(current_str); // Free if not NULL but error occurred
    }
  }
  
  if (*error_flag == 0 && found_any_value) {
    current_str = pop_copy(stack_head, error_flag);
    while (current_str != NULL && *error_flag == 0) {
      double val = atof(current_str);
      free(current_str);
      if (val < min_val) { // Compare for min
        min_val = val;
      }
      current_str = pop_copy(stack_head, error_flag);
    }
  }

  if (*error_flag != 0 || !found_any_value) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  
  return (long_double_t)min_val;
}

// Function: handle_op_stddev
long_double_t handle_op_stddev(StackNode **stack_head, int *error_flag) {
  double sum = 0.0;
  uint_t count = 0;
  char *current_str;
  double *values = NULL; // Temporary array to store values for two passes
  size_t values_capacity = 0;
  size_t values_size = 0;

  // First pass: Read all values, calculate sum, count, and store values
  if (*stack_head != NULL) {
    current_str = pop_copy(stack_head, error_flag);
    while (current_str != NULL && *error_flag == 0) {
      double val = atof(current_str);
      free(current_str); // Free the string after conversion

      // Store value in dynamic array
      if (values_size >= values_capacity) {
        values_capacity = (values_capacity == 0) ? 10 : values_capacity * 2;
        double *new_values = (double *)realloc(values, values_capacity * sizeof(double));
        if (new_values == NULL) {
          free(values); // Clean up old allocation if realloc fails
          *error_flag = 1;
          break; // Memory allocation error
        }
        values = new_values;
      }
      values[values_size++] = val;
      sum += val;
      count++;

      current_str = pop_copy(stack_head, error_flag);
    }
  }

  // Handle errors or insufficient data
  if (*error_flag != 0 || count == 0) {
    clear_stack(stack_head); // Clear any remaining elements from original stack
    free(values);            // Free temporary array
    *error_flag = 1;
    return 0.0L;
  }

  // Calculate mean
  double mean = sum / (double)count;
  double sum_sq_diff = 0.0;

  // Second pass: Calculate sum of squared differences
  for (uint_t i = 0; i < count; i++) {
    double diff = values[i] - mean;
    sum_sq_diff += diff * diff;
  }

  free(values); // Free the temporary array after use

  // Calculate standard deviation
  return (long_double_t)sqrt(sum_sq_diff / (double)count);
}

// Function: handle_op_abs
long_double_t handle_op_abs(StackNode **stack_head, int *error_flag) {
  double val;
  if (!get_double_from_stack(stack_head, error_flag, &val)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL) { // Check if stack is empty after popping 1 value and no error occurred
    return (long_double_t)fabs(val);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_add
long_double_t handle_op_add(StackNode **stack_head, int *error_flag) {
  double val1, val2;
  if (!get_double_from_stack(stack_head, error_flag, &val1) ||
      !get_double_from_stack(stack_head, error_flag, &val2)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL) {
    return (long_double_t)(val1 + val2);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_cos
long_double_t handle_op_cos(StackNode **stack_head, int *error_flag) {
  double val;
  if (!get_double_from_stack(stack_head, error_flag, &val)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL) {
    return (long_double_t)cos(val);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_ln
long_double_t handle_op_ln(StackNode **stack_head, int *error_flag) {
  double val;
  if (!get_double_from_stack(stack_head, error_flag, &val)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL && val != 0.0) {
    return (long_double_t)log(val);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_log10
long_double_t handle_op_log10(StackNode **stack_head, int *error_flag) {
  double val;
  if (!get_double_from_stack(stack_head, error_flag, &val)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL && val != 0.0) {
    return (long_double_t)log10(val);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_power
long_double_t handle_op_power(StackNode **stack_head, int *error_flag) {
  double base, exponent;
  if (!get_double_from_stack(stack_head, error_flag, &base) || // First value is base
      !get_double_from_stack(stack_head, error_flag, &exponent)) { // Second value is exponent
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  
  if (*error_flag == 0 && *stack_head == NULL) {
    if (!isnan(exponent)) { // Use isnan from math.h
      if ((base != 0.0) || (exponent <= 0.0)) {
        if ((0.0 <= base) || (remainder(exponent, 1.0) != 0.0)) {
          return (long_double_t)pow(base, exponent);
        } else {
          // Negative base, integer exponent
          double result = pow(-base, exponent);
          if (remainder(exponent, DAT_00017150) == 0.0) { // DAT_00017150 is 2.0
            // Even exponent, result is positive
          } else {
            // Odd exponent, result is negative
            result = -result;
          }
          return (long_double_t)result;
        }
      } else {
        // base == 0.0 and exponent > 0.0, result is 0.0
        return 0.0L;
      }
    }
    return 0.0L; // Default for NaN exponent or other unhandled cases, matches original cleanup
  }

  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_product
long_double_t handle_op_product(StackNode **stack_head, int *error_flag) {
  double val1, val2;
  if (!get_double_from_stack(stack_head, error_flag, &val1) ||
      !get_double_from_stack(stack_head, error_flag, &val2)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL) {
    return (long_double_t)(val1 * val2);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_quotient
long_double_t handle_op_quotient(StackNode **stack_head, int *error_flag) {
  double dividend, divisor;
  if (!get_double_from_stack(stack_head, error_flag, &dividend) || // First value is dividend
      !get_double_from_stack(stack_head, error_flag, &divisor)) {   // Second value is divisor
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL && divisor != 0.0) {
    return (long_double_t)(dividend / divisor);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_sin
long_double_t handle_op_sin(StackNode **stack_head, int *error_flag) {
  double val;
  if (!get_double_from_stack(stack_head, error_flag, &val)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL) {
    return (long_double_t)sin(val);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_sqrt
long_double_t handle_op_sqrt(StackNode **stack_head, int *error_flag) {
  double val;
  if (!get_double_from_stack(stack_head, error_flag, &val)) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL && val >= 0.0) {
    return (long_double_t)sqrt(val);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_subtract
long_double_t handle_op_subtract(StackNode **stack_head, int *error_flag) {
  double val1, val2;
  if (!get_double_from_stack(stack_head, error_flag, &val1) || // First value
      !get_double_from_stack(stack_head, error_flag, &val2)) {   // Second value
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  if (*error_flag == 0 && *stack_head == NULL) {
    return (long_double_t)(val1 - val2);
  }
  clear_stack(stack_head);
  *error_flag = 1;
  return 0.0L;
}

// Function: handle_op_sum
long_double_t handle_op_sum(StackNode **stack_head, int *error_flag) {
  double sum = 0.0;
  char *current_str;

  if (*stack_head == NULL) {
    *error_flag = 1;
    return 0.0L;
  }

  current_str = pop_copy(stack_head, error_flag);
  while (current_str != NULL && *error_flag == 0) {
    sum += atof(current_str);
    free(current_str);
    current_str = pop_copy(stack_head, error_flag);
  }

  if (*error_flag != 0) {
    clear_stack(stack_head);
    *error_flag = 1;
    return 0.0L;
  }
  
  return (long_double_t)sum;
}