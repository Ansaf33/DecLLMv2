#include <stdlib.h> // For malloc, free, div_t
#include <stdio.h>  // For read_all, write_all (mocked)
#include <stdint.h> // For uint32_t, uint64_t
#include <math.h>   // For isinf, isnan, double operations
#include <string.h> // For memcpy

// --- Type Definitions and Mocks ---
typedef uint32_t undefined4;
typedef uint64_t undefined8;
typedef unsigned int uint;

// CONCAT44 macro: Combines two 32-bit values into a 64-bit value.
// high is the most significant 32 bits, low is the least significant 32 bits.
#define CONCAT44(high, low) (((uint64_t)(high) << 32) | (uint64_t)(low))

// RPNCalc_State structure for the calculator's stack (linked list)
// A node is allocated with malloc(sizeof(RPNCalc_State_Node))
// The first 8 bytes (offset 0) is the 'next' pointer.
// The next 8 bytes (offset 8) is the 'value' (undefined8).
typedef struct RPNCalc_State_Node {
    struct RPNCalc_State_Node *next; // At offset 0
    undefined8 value;                 // At offset 8
} RPNCalc_State_Node;

typedef struct RPNCalc_State {
    RPNCalc_State_Node *head;
} RPNCalc_State;

// Mock function signatures for external dependencies
// These are minimal mocks to allow compilation. Actual behavior would vary.

// get_type: Returns 0 for integer, 1 for matrix, 2 for double.
// Assumes the undefined8 value is tagged in its high 32 bits.
int get_type(undefined8 val) {
    uint32_t high_part = (uint32_t)(val >> 32);
    if (high_part == 0x00000000) return 0; // Integer
    if (high_part == 0x00000001) return 1; // Matrix
    if (high_part == 0x00000002) return 2; // Double
    return -1; // Unknown type
}

// as_double: Extracts a double from undefined8. Returns 0 on success.
int as_double(undefined8 val, double *out) {
    if (get_type(val) != 2) return -1; // Type mismatch
    memcpy(out, &val, sizeof(double)); // Assume the double is directly stored in the undefined8
    return 0;
}

// as_integer: Extracts an int from undefined8. Returns 0 on success.
int as_integer(undefined8 val, int *out) {
    if (get_type(val) != 0) return -1; // Type mismatch
    *out = (int)(uint32_t)val; // Assume integer is stored in the low 32 bits
    return 0;
}

// as_matrix: Extracts a matrix (uint*) from undefined8. Returns 0 on success.
// Assumes undefined8 contains a pointer to the matrix data in its low 32 bits.
int as_matrix(undefined8 val, uint **out) {
    if (get_type(val) != 1) return -1; // Type mismatch
    *out = (uint *)(uintptr_t)(val & 0xFFFFFFFFULL); // Assume pointer is in low 32 bits
    return 0;
}

// set_double: Sets undefined8 with a double value.
void set_double(double val, undefined8 *out) {
    memcpy(out, &val, sizeof(double));
    // Tag the high 32 bits as double type
    *out = (*out & 0x00000000FFFFFFFFULL) | (0x00000002ULL << 32);
}

// set_integer: Sets undefined8 with an integer value.
void set_integer(int val, undefined8 *out) {
    *out = (undefined8)(uint32_t)val;
    // Tag the high 32 bits as integer type (0x00000000)
    *out = (*out & 0x00000000FFFFFFFFULL) | (0x00000000ULL << 32);
}

// set_matrix: Sets undefined8 with a matrix pointer.
void set_matrix(uint *matrix_data, undefined8 *out) {
    *out = (undefined8)(uintptr_t)matrix_data;
    // Tag the high 32 bits as matrix type (0x00000001)
    *out = (*out & 0x00000000FFFFFFFFULL) | (0x00000001ULL << 32);
}

// Mock I/O functions
int read_all(int fd, void *buf, size_t count) { return (int)count; }
int write_all(int fd, const void *buf, size_t count) { return (int)count; }

// Mock List functions for RPNCalc_State
void list_push_front(RPNCalc_State *state, RPNCalc_State_Node *node_to_push) {
    if (!state || !node_to_push) return;
    node_to_push->next = state->head;
    state->head = node_to_push;
}

RPNCalc_State_Node *list_pop_front(RPNCalc_State *state) {
    if (!state || !state->head) return NULL;
    RPNCalc_State_Node *node = state->head;
    state->head = node->next;
    node->next = NULL; // Detach
    return node;
}

int list_length(RPNCalc_State *state) {
    if (!state) return 0;
    int count = 0;
    RPNCalc_State_Node *current = state->head;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

void list_remove(RPNCalc_State *state, RPNCalc_State_Node *node_to_remove) {
    if (!state || !state->head || !node_to_remove) return;

    if (state->head == node_to_remove) {
        state->head = node_to_remove->next;
        node_to_remove->next = NULL;
        return;
    }

    RPNCalc_State_Node *current = state->head;
    while (current && current->next != node_to_remove) {
        current = current->next;
    }
    if (current && current->next == node_to_remove) {
        current->next = node_to_remove->next;
        node_to_remove->next = NULL;
    }
}

// --- Original Functions (Fixed) ---

// Function: swap
void swap(undefined8 *param_1, undefined8 *param_2) {
  undefined8 temp = *param_1;
  *param_1 = *param_2;
  *param_2 = temp;
}

// Function: generic_add
undefined4 generic_add(undefined8 op1_val, undefined8 op2_val, undefined8 *result_ptr) {
  if (result_ptr == NULL) {
    return 0xffffffff;
  }

  int type1 = get_type(op1_val);
  int type2 = get_type(op2_val);

  // Ensure op1_val has the higher or equal type (double > matrix > integer)
  if (type1 < type2) {
    swap(&op1_val, &op2_val);
    int temp_type = type1;
    type1 = type2;
    type2 = temp_type;
  }

  if (type1 == 2) { // op1 is double
    double d_op1, d_op2;
    if (as_double(op1_val, &d_op1) != 0) return 0xffffffff;
    if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
    
    set_double(d_op1 + d_op2, result_ptr);
    if ((*(uint32_t *)((uintptr_t)result_ptr + 4) & 0x7ff00000) != 0x7ff00000) { // Check high 32 bits for NaN/Inf
      return 0;
    }
    return 0xffffffff;
  }
  
  if (type1 > 2) return 0xffffffff; // Unknown type

  if (type1 == 0) { // op1 is integer
    int i_op1;
    if (as_integer(op1_val, &i_op1) != 0) return 0xffffffff;

    if (type2 == 2) { // op2 is double
      double d_op2;
      if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
      
      set_double(d_op2 + (double)i_op1, result_ptr);
      if ((*(uint32_t *)((uintptr_t)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
        return 0xffffffff;
      }
    } else if (type2 > 2) {
      return 0xffffffff;
    } else if (type2 == 0) { // op2 is integer
      int i_op2;
      if (as_integer(op2_val, &i_op2) != 0) return 0xffffffff;
      set_integer(i_op1 + i_op2, result_ptr);
    } else { // type2 == 1 (matrix)
      uint *m_op2;
      if (as_matrix(op2_val, &m_op2) != 0) return 0xffffffff;
      
      uint *result_matrix = (uint *)malloc((size_t)(m_op2[1] * m_op2[0] + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = m_op2[0];
      result_matrix[1] = m_op2[1];

      for (uint r = 0; r < m_op2[0]; ++r) {
        for (uint c = 0; c < m_op2[1]; ++c) {
          undefined8 element_val_op2;
          memcpy(&element_val_op2, m_op2 + (r + m_op2[0] * c) * 2 + 2, sizeof(undefined8));
          
          undefined8 element_result_val;
          if (generic_add(op1_val, element_val_op2, &element_result_val) != 0) {
            free(result_matrix);
            return 0xffffffff;
          }
          memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &element_result_val, sizeof(undefined8));
        }
      }
      set_matrix(result_matrix, result_ptr);
    }
  } else { // type1 == 1 (op1 is matrix)
    uint *m_op1;
    if (as_matrix(op1_val, &m_op1) != 0) return 0xffffffff;

    if (type2 == 1) { // op2 is matrix
      uint *m_op2;
      if (as_matrix(op2_val, &m_op2) != 0) return 0xffffffff;

      if ((m_op1[0] != m_op2[0]) || (m_op1[1] != m_op2[1])) { // Dimensions must match
        return 0xffffffff;
      }
      
      uint *result_matrix = (uint *)malloc((size_t)(m_op1[1] * m_op1[0] + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = m_op1[0];
      result_matrix[1] = m_op1[1];

      for (uint r = 0; r < m_op1[0]; ++r) {
        for (uint c = 0; c < m_op1[1]; ++c) {
          // Matrix elements are doubles
          double d_op1_elem, d_op2_elem;
          memcpy(&d_op1_elem, m_op1 + (r + m_op1[0] * c) * 2 + 2, sizeof(double));
          memcpy(&d_op2_elem, m_op2 + (r + m_op2[0] * c) * 2 + 2, sizeof(double));
          
          double sum = d_op1_elem + d_op2_elem;
          memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &sum, sizeof(double));
          
          if ((result_matrix[(r + result_matrix[0] * c) * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
            free(result_matrix);
            return 0xffffffff;
          }
        }
      }
      set_matrix(result_matrix, result_ptr);
    } else if (type2 == 2) { // op2 is double
      double d_op2;
      if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
      
      uint *result_matrix = (uint *)malloc((size_t)(m_op1[1] * m_op1[0] + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = m_op1[0];
      result_matrix[1] = m_op1[1];

      for (uint r = 0; r < m_op1[0]; ++r) {
        for (uint c = 0; c < m_op1[1]; ++c) {
          double d_op1_elem;
          memcpy(&d_op1_elem, m_op1 + (r + m_op1[0] * c) * 2 + 2, sizeof(double));
          
          double sum = d_op1_elem + d_op2;
          memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &sum, sizeof(double));

          if ((result_matrix[(r + result_matrix[0] * c) * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
            free(result_matrix);
            return 0xffffffff;
          }
        }
      }
      set_matrix(result_matrix, result_ptr);
    } else { // Matrix + Integer or other unsupported type
      return 0xffffffff;
    }
  }
  return 0;
}

// Function: generic_sub
undefined4 generic_sub(undefined8 op1_val, undefined8 op2_val, undefined8 *result_ptr) {
  if (result_ptr == NULL) return 0xffffffff;

  int type2 = get_type(op2_val);
  undefined8 neg_op2_val;
  
  if (type2 == 2) { // op2 is double
    double d_op2;
    if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
    set_double(-d_op2, &neg_op2_val);
    return generic_add(op1_val, neg_op2_val, result_ptr);
  }
  
  if (type2 < 3) {
    if (type2 == 0) { // op2 is integer
      int i_op2;
      if (as_integer(op2_val, &i_op2) != 0) return 0xffffffff;
      set_integer(-i_op2, &neg_op2_val);
      return generic_add(op1_val, neg_op2_val, result_ptr);
    }
    if (type2 == 1) { // op2 is matrix
      uint *m_op2;
      if (as_matrix(op2_val, &m_op2) != 0) return 0xffffffff;
      
      uint *neg_matrix = (uint *)malloc((size_t)(m_op2[1] * m_op2[0] + 1) * sizeof(undefined8));
      if (neg_matrix == NULL) return 0xffffffff;
      
      neg_matrix[0] = m_op2[0];
      neg_matrix[1] = m_op2[1];

      for (uint r = 0; r < m_op2[0]; ++r) {
        for (uint c = 0; c < m_op2[1]; ++c) {
          double d_op2_elem;
          memcpy(&d_op2_elem, m_op2 + (r + m_op2[0] * c) * 2 + 2, sizeof(double));
          double neg_elem = -d_op2_elem;
          memcpy(neg_matrix + (r + neg_matrix[0] * c) * 2 + 2, &neg_elem, sizeof(double));
        }
      }
      set_matrix(neg_matrix, &neg_op2_val);
      undefined4 ret = generic_add(op1_val, neg_op2_val, result_ptr);
      free(neg_matrix); 
      return ret;
    }
  }
  return 0xffffffff; // Unhandled type
}

// Function: generic_mul
undefined4 generic_mul(undefined8 op1_val, undefined8 op2_val, undefined8 *result_ptr) {
  if (result_ptr == NULL) return 0xffffffff;

  int type1 = get_type(op1_val);
  int type2 = get_type(op2_val);

  // Ensure op1_val has the higher or equal type (double > matrix > integer)
  if (type1 < type2) {
    swap(&op1_val, &op2_val);
    int temp_type = type1;
    type1 = type2;
    type2 = temp_type;
  }

  if (type1 == 2) { // op1 is double
    double d_op1, d_op2;
    if (as_double(op1_val, &d_op1) != 0) return 0xffffffff;
    if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
    
    set_double(d_op1 * d_op2, result_ptr);
    if ((*(uint32_t *)((uintptr_t)result_ptr + 4) & 0x7ff00000) != 0x7ff00000) {
      return 0;
    }
    return 0xffffffff;
  }
  
  if (type1 > 2) return 0xffffffff; // Unknown type

  if (type1 == 0) { // op1 is integer
    int i_op1;
    if (as_integer(op1_val, &i_op1) != 0) return 0xffffffff;

    if (type2 == 2) { // op2 is double
      double d_op2;
      if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
      
      set_double(d_op2 * (double)i_op1, result_ptr);
      if ((*(uint32_t *)((uintptr_t)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
        return 0xffffffff;
      }
    } else if (type2 > 2) {
      return 0xffffffff;
    } else if (type2 == 0) { // op2 is integer
      int i_op2;
      if (as_integer(op2_val, &i_op2) != 0) return 0xffffffff;
      set_integer(i_op1 * i_op2, result_ptr);
    } else { // type2 == 1 (matrix)
      uint *m_op2;
      if (as_matrix(op2_val, &m_op2) != 0) return 0xffffffff;
      
      uint *result_matrix = (uint *)malloc((size_t)(m_op2[1] * m_op2[0] + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = m_op2[0];
      result_matrix[1] = m_op2[1];

      for (uint r = 0; r < m_op2[0]; ++r) {
        for (uint c = 0; c < m_op2[1]; ++c) {
          undefined8 element_val_op2;
          memcpy(&element_val_op2, m_op2 + (r + m_op2[0] * c) * 2 + 2, sizeof(undefined8));
          
          int element_type = get_type(element_val_op2);
          if (element_type == 0) { // Element is integer
            int i_elem_op2;
            if (as_integer(element_val_op2, &i_elem_op2) != 0) {
              free(result_matrix);
              return 0xffffffff;
            }
            undefined8 element_result_val;
            set_integer(i_elem_op2 * i_op1, &element_result_val);
            memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &element_result_val, sizeof(undefined8));
          } else if (element_type == 2) { // Element is double
            double d_elem_op2;
            if (as_double(element_val_op2, &d_elem_op2) != 0) {
              free(result_matrix);
              return 0xffffffff;
            }
            double prod_double = d_elem_op2 * (double)i_op1;
            memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &prod_double, sizeof(double));
            if ((result_matrix[(r + result_matrix[0] * c) * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
              free(result_matrix);
              return 0xffffffff;
            }
          } else { // Other element types in matrix not supported for scalar mult.
            free(result_matrix);
            return 0xffffffff;
          }
        }
      }
      set_matrix(result_matrix, result_ptr);
    }
  } else { // type1 == 1 (op1 is matrix)
    uint *m_op1;
    if (as_matrix(op1_val, &m_op1) != 0) return 0xffffffff;

    if (type2 == 1) { // op2 is matrix (matrix multiplication)
      uint *m_op2;
      if (as_matrix(op2_val, &m_op2) != 0) return 0xffffffff;

      // Check dimensions for matrix multiplication (m1_cols == m2_rows)
      if (m_op1[1] != m_op2[0]) {
        return 0xffffffff;
      }
      
      uint result_rows = m_op1[0];
      uint result_cols = m_op2[1];
      uint *result_matrix = (uint *)malloc((size_t)(result_cols * result_rows + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = result_rows;
      result_matrix[1] = result_cols;

      // Initialize result matrix elements to 0
      for (uint i = 0; i < result_rows * result_cols; ++i) {
        undefined8 zero_val;
        set_double(0.0, &zero_val); // Assuming matrix multiplication deals with doubles
        memcpy(result_matrix + i * 2 + 2, &zero_val, sizeof(undefined8));
      }

      for (uint r = 0; r < result_rows; ++r) {
        for (uint c = 0; c < result_cols; ++c) {
          double sum = 0.0;
          for (uint k = 0; k < m_op1[1]; ++k) { // m_op1[1] is m1_cols, which is m2_rows
            double m1_elem, m2_elem;
            memcpy(&m1_elem, m_op1 + (r + m_op1[0] * k) * 2 + 2, sizeof(double));
            memcpy(&m2_elem, m_op2 + (k + m_op2[0] * c) * 2 + 2, sizeof(double));
            sum += m1_elem * m2_elem;
            if (isinf(sum) || isnan(sum)) { // Check for NaN/Inf during accumulation
              free(result_matrix);
              return 0xffffffff;
            }
          }
          memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &sum, sizeof(double));
        }
      }
      set_matrix(result_matrix, result_ptr);
    } else if (type2 == 2) { // op2 is double (scalar multiplication)
      double d_op2;
      if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
      
      uint *result_matrix = (uint *)malloc((size_t)(m_op1[1] * m_op1[0] + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = m_op1[0];
      result_matrix[1] = m_op1[1];

      for (uint r = 0; r < m_op1[0]; ++r) {
        for (uint c = 0; c < m_op1[1]; ++c) {
          double d_op1_elem;
          memcpy(&d_op1_elem, m_op1 + (r + m_op1[0] * c) * 2 + 2, sizeof(double));
          
          double prod = d_op1_elem * d_op2;
          memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &prod, sizeof(double));
          if ((result_matrix[(r + result_matrix[0] * c) * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
            free(result_matrix);
            return 0xffffffff;
          }
        }
      }
      set_matrix(result_matrix, result_ptr);
    } else { // Scalar multiplication by integer or other type, not handled explicitly in original
      return 0xffffffff;
    }
  }
  return 0;
}

// Function: generic_div
undefined4 generic_div(undefined8 op1_val, undefined8 op2_val, undefined8 *result_ptr) {
  if (result_ptr == NULL) return 0xffffffff;

  int type1 = get_type(op1_val);
  int type2 = get_type(op2_val);

  // Matrix / Matrix division is not supported
  if (type1 == 1 && type2 == 1) {
    return 0xffffffff;
  }

  if (type1 == 0 && type2 == 0) { // Integer / Integer
    int i_op1, i_op2;
    if (as_integer(op1_val, &i_op1) != 0) return 0xffffffff;
    if (as_integer(op2_val, &i_op2) != 0) return 0xffffffff;
    if (i_op2 == 0) return 0xffffffff; // Division by zero
    set_integer(i_op1 / i_op2, result_ptr);
    return 0;
  }

  if (type1 == 2) { // op1 is double
    double d_op1;
    if (as_double(op1_val, &d_op1) != 0) return 0xffffffff;

    if (type2 == 1) { // Double / Matrix (element-wise inverse multiplication)
      uint *m_op2;
      if (as_matrix(op2_val, &m_op2) != 0) return 0xffffffff;
      
      uint *result_matrix = (uint *)malloc((size_t)(m_op2[1] * m_op2[0] + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = m_op2[0];
      result_matrix[1] = m_op2[1];

      for (uint r = 0; r < m_op2[0]; ++r) {
        for (uint c = 0; c < m_op2[1]; ++c) {
          double d_op2_elem;
          memcpy(&d_op2_elem, m_op2 + (r + m_op2[0] * c) * 2 + 2, sizeof(double));
          if (d_op2_elem == 0.0) {
            free(result_matrix);
            return 0xffffffff;
          }
          double res = d_op1 / d_op2_elem;
          memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &res, sizeof(double));
          if ((result_matrix[(r + result_matrix[0] * c) * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
            free(result_matrix);
            return 0xffffffff;
          }
        }
      }
      set_matrix(result_matrix, result_ptr);
    } else { // Double / Scalar (integer or double)
      double d_op2;
      if (type2 == 0) { // op2 is integer
        int i_op2;
        if (as_integer(op2_val, &i_op2) != 0) return 0xffffffff;
        d_op2 = (double)i_op2;
      } else if (type2 == 2) { // op2 is double
        if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
      } else {
        return 0xffffffff; // Unhandled type
      }

      if (d_op2 == 0.0) return 0xffffffff; // Division by zero
      set_double(d_op1 / d_op2, result_ptr);
      if ((*(uint32_t *)((uintptr_t)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
        return 0xffffffff;
      }
    }
  } else if (type1 == 0) { // op1 is integer
    int i_op1;
    if (as_integer(op1_val, &i_op1) != 0) return 0xffffffff;

    if (type2 == 1) { // Integer / Matrix (element-wise inverse multiplication)
      uint *m_op2;
      if (as_matrix(op2_val, &m_op2) != 0) return 0xffffffff;
      
      uint *result_matrix = (uint *)malloc((size_t)(m_op2[1] * m_op2[0] + 1) * sizeof(undefined8));
      if (result_matrix == NULL) return 0xffffffff;
      
      result_matrix[0] = m_op2[0];
      result_matrix[1] = m_op2[1];

      for (uint r = 0; r < m_op2[0]; ++r) {
        for (uint c = 0; c < m_op2[1]; ++c) {
          double d_op2_elem;
          memcpy(&d_op2_elem, m_op2 + (r + m_op2[0] * c) * 2 + 2, sizeof(double));
          if (d_op2_elem == 0.0) {
            free(result_matrix);
            return 0xffffffff;
          }
          double res = (double)i_op1 / d_op2_elem;
          memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &res, sizeof(double));
          if ((result_matrix[(r + result_matrix[0] * c) * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
            free(result_matrix);
            return 0xffffffff;
          }
        }
      }
      set_matrix(result_matrix, result_ptr);
    } else { // Integer / Scalar (double)
      double d_op2;
      if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
      if (d_op2 == 0.0) return 0xffffffff; // Division by zero
      set_double((double)i_op1 / d_op2, result_ptr);
      if ((*(uint32_t *)((uintptr_t)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
        return 0xffffffff;
      }
    }
  } else if (type1 == 1) { // op1 is matrix
    uint *m_op1;
    if (as_matrix(op1_val, &m_op1) != 0) return 0xffffffff;

    // Matrix / Scalar (integer or double)
    double d_op2;
    if (type2 == 0) { // op2 is integer
      int i_op2;
      if (as_integer(op2_val, &i_op2) != 0) return 0xffffffff;
      d_op2 = (double)i_op2;
    } else if (type2 == 2) { // op2 is double
      if (as_double(op2_val, &d_op2) != 0) return 0xffffffff;
    } else {
      return 0xffffffff; // Unhandled type
    }
    
    if (d_op2 == 0.0) return 0xffffffff; // Division by zero

    uint *result_matrix = (uint *)malloc((size_t)(m_op1[1] * m_op1[0] + 1) * sizeof(undefined8));
    if (result_matrix == NULL) return 0xffffffff;
    
    result_matrix[0] = m_op1[0];
    result_matrix[1] = m_op1[1];

    for (uint r = 0; r < m_op1[0]; ++r) {
      for (uint c = 0; c < m_op1[1]; ++c) {
        double d_op1_elem;
        memcpy(&d_op1_elem, m_op1 + (r + m_op1[0] * c) * 2 + 2, sizeof(double));
        double res = d_op1_elem / d_op2;
        memcpy(result_matrix + (r + result_matrix[0] * c) * 2 + 2, &res, sizeof(double));
        if ((result_matrix[(r + result_matrix[0] * c) * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
          free(result_matrix);
          return 0xffffffff;
        }
      }
    }
    set_matrix(result_matrix, result_ptr);
  } else {
    return 0xffffffff; // Unhandled type combination
  }
  return 0;
}

// Function: matrix_inv
undefined4 matrix_inv(uint *matrix_in, uint **matrix_out_ptr) {
  if (matrix_out_ptr == NULL) return 0xffffffff;

  // Matrix must be square and small (1x1 or 2x2)
  if (matrix_in[0] != matrix_in[1] || matrix_in[0] == 0 || matrix_in[0] > 2) {
    return 0xffffffff;
  }

  double determinant;
  if (matrix_in[0] == 1) { // 1x1 matrix
    memcpy(&determinant, matrix_in + 2, sizeof(double));
  } else { // 2x2 matrix
    double a, b, c, d;
    memcpy(&a, matrix_in + 2, sizeof(double)); // M[0][0]
    memcpy(&b, matrix_in + 4, sizeof(double)); // M[0][1]
    memcpy(&c, matrix_in + 6, sizeof(double)); // M[1][0]
    memcpy(&d, matrix_in + 8, sizeof(double)); // M[1][1]
    determinant = a * d - b * c;
  }

  if (isinf(determinant) || isnan(determinant) || determinant == 0.0) {
    return 0xffffffff;
  }

  uint *result_matrix = (uint *)malloc((size_t)(matrix_in[1] * matrix_in[0] + 1) * sizeof(undefined8));
  if (result_matrix == NULL) return 0xffffffff;
  
  result_matrix[0] = matrix_in[0];
  result_matrix[1] = matrix_in[1];
  *matrix_out_ptr = result_matrix;

  if (matrix_in[0] == 1) {
    double inv_det = 1.0 / determinant;
    memcpy(result_matrix + 2, &inv_det, sizeof(double));
  } else { // 2x2 matrix
    double a, b, c, d;
    memcpy(&a, matrix_in + 2, sizeof(double)); // M[0][0]
    memcpy(&b, matrix_in + 4, sizeof(double)); // M[0][1]
    memcpy(&c, matrix_in + 6, sizeof(double)); // M[1][0]
    memcpy(&d, matrix_in + 8, sizeof(double)); // M[1][1]

    double inv_det = 1.0 / determinant;
    double inv_a = d * inv_det;
    double inv_b = -b * inv_det;
    double inv_c = -c * inv_det;
    double inv_d = a * inv_det;

    memcpy(result_matrix + 2, &inv_a, sizeof(double)); // [0][0]
    memcpy(result_matrix + 4, &inv_b, sizeof(double)); // [0][1]
    memcpy(result_matrix + 6, &inv_c, sizeof(double)); // [1][0]
    memcpy(result_matrix + 8, &inv_d, sizeof(double)); // [1][1]
  }
  return 0;
}

// Function: push
undefined4 push(RPNCalc_State *state) {
  RPNCalc_State_Node *new_node = NULL;
  uint *matrix_data = NULL;
  undefined4 ret_val = 0xffffffff;

  new_node = (RPNCalc_State_Node *)malloc(sizeof(RPNCalc_State_Node));
  if (new_node == NULL) return 0xffffffff;

  uint type_and_dims;
  if (read_all(0, &type_and_dims, sizeof(uint)) != sizeof(uint)) {
    goto cleanup_push;
  }

  uint type_val = type_and_dims & 0xffff;

  if (type_val == 2) { // Double
    undefined4 double_low, double_high;
    if (read_all(0, &double_low, sizeof(undefined4)) != sizeof(undefined4) ||
        read_all(0, &double_high, sizeof(undefined4)) != sizeof(undefined4)) {
      goto cleanup_push;
    }
    // Check for NaN/Inf (high 32 bits of double)
    if ((double_high & 0x7ff00000) == 0x7ff00000) {
      goto cleanup_push;
    }
    set_double(CONCAT44(double_high, double_low), &new_node->value);
    ret_val = 0;
  } else if (type_val == 0) { // Integer
    undefined4 int_val;
    if (read_all(0, &int_val, sizeof(undefined4)) != sizeof(undefined4)) {
      goto cleanup_push;
    }
    set_integer(int_val, &new_node->value);
    ret_val = 0;
  } else if (type_val == 1) { // Matrix
    uint rows = (type_and_dims >> 0x18) & 0xff;
    uint cols = (type_and_dims >> 0x10) & 0xff;
    size_t matrix_data_size = (size_t)rows * cols * sizeof(undefined8);
    
    if (rows == 0 || cols == 0) {
      goto cleanup_push;
    }

    matrix_data = (uint *)malloc(matrix_data_size + 2 * sizeof(uint)); // +2 for rows/cols
    if (matrix_data == NULL) goto cleanup_push;

    matrix_data[0] = rows;
    matrix_data[1] = cols;

    if (read_all(0, matrix_data + 2, matrix_data_size) != matrix_data_size) {
      free(matrix_data);
      matrix_data = NULL;
      goto cleanup_push;
    }

    for (uint i = 0; i < matrix_data_size / sizeof(undefined8); ++i) {
      // Check high 32 bits for NaN/Inf for each double element
      if ((matrix_data[i * 2 + 3] & 0x7ff00000) == 0x7ff00000) {
        free(matrix_data);
        matrix_data = NULL;
        goto cleanup_push;
      }
    }
    set_matrix(matrix_data, &new_node->value);
    ret_val = 0;
  } else { // Unknown type
    goto cleanup_push;
  }

  if (ret_val == 0) {
    list_push_front(state, new_node);
  } else {
cleanup_push: // Refactored from original goto labels
    if (new_node) free(new_node);
    // matrix_data is freed above if it was allocated but read failed.
    // If set_matrix was successful, it is assumed to own the matrix_data, and it will be
    // freed when the RPNCalc_State_Node is freed (e.g., by clear or pop).
  }
  return ret_val;
}

// Function: pop
undefined4 pop(RPNCalc_State *state) {
  RPNCalc_State_Node *node_to_pop = NULL;
  uint *matrix_data = NULL;
  undefined4 ret_val = 0xffffffff;

  node_to_pop = list_pop_front(state);
  if (node_to_pop == NULL) {
    return 0xffffffff;
  }

  undefined8 *value_ptr = &node_to_pop->value;
  uint type_val = get_type(*value_ptr);

  int result_ok = 0;
  if (type_val == 2) { // Double
    double d_val;
    if (as_double(*value_ptr, &d_val) == 0) {
      uint type_code = 2; // Type identifier
      if (write_all(1, &type_code, sizeof(uint)) == sizeof(uint) &&
          write_all(1, &d_val, sizeof(double)) == sizeof(double)) {
        result_ok = 1;
      }
    }
  } else if (type_val == 0) { // Integer
    int i_val;
    if (as_integer(*value_ptr, &i_val) == 0) {
      uint type_code = 0; // Type identifier
      if (write_all(1, &type_code, sizeof(uint)) == sizeof(uint) &&
          write_all(1, &i_val, sizeof(int)) == sizeof(int)) {
        result_ok = 1;
      }
    }
  } else if (type_val == 1) { // Matrix
    if (as_matrix(*value_ptr, &matrix_data) == 0) {
      uint rows = matrix_data[0];
      uint cols = matrix_data[1];
      size_t matrix_data_size = (size_t)rows * cols * sizeof(undefined8);
      // Combine type (1), rows, and cols into a single uint for output
      uint type_and_dims = 1 | (cols << 16) | (rows << 24);
      
      if (write_all(1, &type_and_dims, sizeof(uint)) == sizeof(uint) &&
          write_all(1, matrix_data + 2, matrix_data_size) == matrix_data_size) {
        result_ok = 1;
      }
    }
  }

  if (result_ok) {
    ret_val = 0;
  }

  // Common cleanup (freed whether operation succeeded or failed)
  // `as_matrix` might return a pointer to data owned by the `undefined8` value.
  // The original code frees `local_38` (matrix_data) and `local_18` (value_ptr's node).
  // This implies `as_matrix` returns a newly allocated copy of the matrix data.
  if (matrix_data) free(matrix_data);
  if (node_to_pop) free(node_to_pop); // Free the RPNCalc_State_Node
  
  return ret_val;
}

// Function: clear
undefined4 clear(RPNCalc_State *state) {
  RPNCalc_State_Node *current = state->head;
  RPNCalc_State_Node *next_node;
  uint *matrix_data = NULL;

  while (current != NULL) {
    next_node = current->next;
    
    // Check if the value is a matrix and free its data
    if (get_type(current->value) == 1 && as_matrix(current->value, &matrix_data) == 0) {
      free(matrix_data);
    }
    free(current); // Free the node itself
    current = next_node;
  }
  state->head = NULL; // Empty the list
  return 0;
}

// Function: binary_op
typedef undefined4 (*generic_op_func)(undefined8, undefined8, undefined8 *);

undefined4 binary_op(RPNCalc_State *state, generic_op_func op_func) {
  RPNCalc_State_Node *op1_node = NULL;
  RPNCalc_State_Node *op2_node = NULL;
  RPNCalc_State_Node *result_node = NULL;
  uint *matrix_data_op1 = NULL;
  uint *matrix_data_op2 = NULL;
  undefined4 ret_val = 0xffffffff;

  if (list_length(state) < 2) {
    return 0xffffffff;
  }

  op1_node = list_pop_front(state); // First operand popped (top of stack)
  if (op1_node == NULL) goto cleanup_binary_op;

  op2_node = list_pop_front(state); // Second operand popped
  if (op2_node == NULL) {
    list_push_front(state, op1_node); // Push back op1 if op2 is missing
    goto cleanup_binary_op;
  }

  result_node = (RPNCalc_State_Node *)malloc(sizeof(RPNCalc_State_Node));
  if (result_node == NULL) {
    list_push_front(state, op2_node); // Push back op2
    list_push_front(state, op1_node); // Push back op1
    goto cleanup_binary_op;
  }

  // Call the generic operation function
  if (op_func(op2_node->value, op1_node->value, &result_node->value) != 0) {
    list_push_front(state, op2_node); // Push back op2
    list_push_front(state, op1_node); // Push back op1
    free(result_node);
    result_node = NULL;
    goto cleanup_binary_op;
  }

  list_push_front(state, result_node);
  ret_val = 0;

cleanup_binary_op:
  // Free operands (matrix data first, then nodes)
  if (op1_node) {
    if (get_type(op1_node->value) == 1 && as_matrix(op1_node->value, &matrix_data_op1) == 0) {
      free(matrix_data_op1);
    }
    free(op1_node);
  }
  if (op2_node) {
    if (get_type(op2_node->value) == 1 && as_matrix(op2_node->value, &matrix_data_op2) == 0) {
      free(matrix_data_op2);
    }
    free(op2_node);
  }

  return ret_val;
}

// Function: add
void add(RPNCalc_State *state) {
  binary_op(state, generic_add);
}

// Function: sub
void sub(RPNCalc_State *state) {
  binary_op(state, generic_sub);
}

// Function: mul
void mul(RPNCalc_State *state) {
  binary_op(state, generic_mul);
}

// Function: div (renamed to avoid conflict with stdlib.h div_t div(int,int))
undefined4 divide(RPNCalc_State *state) {
  return binary_op(state, generic_div);
}

// Function: inv
undefined4 inv(RPNCalc_State *state) {
  RPNCalc_State_Node *op_node = NULL;
  RPNCalc_State_Node *result_node = NULL;
  uint *matrix_in_data = NULL;
  uint *matrix_out_data = NULL;
  undefined4 ret_val = 0xffffffff;

  if (list_length(state) == 0) {
    return 0xffffffff;
  }

  op_node = list_pop_front(state);
  if (op_node == NULL) return 0xffffffff;

  if (get_type(op_node->value) != 1) { // Only matrix inversion is supported
    list_push_front(state, op_node); // Push back if not a matrix
    return 0xffffffff;
  }
  
  if (as_matrix(op_node->value, &matrix_in_data) != 0) {
    list_push_front(state, op_node);
    return 0xffffffff;
  }

  result_node = (RPNCalc_State_Node *)malloc(sizeof(RPNCalc_State_Node));
  if (result_node == NULL) {
    list_push_front(state, op_node);
    goto cleanup_inv;
  }

  if (matrix_inv(matrix_in_data, &matrix_out_data) != 0) {
    list_push_front(state, op_node);
    free(result_node);
    result_node = NULL;
    goto cleanup_inv;
  }

  set_matrix(matrix_out_data, &result_node->value);
  list_push_front(state, result_node);
  ret_val = 0;

cleanup_inv:
  if (matrix_in_data) free(matrix_in_data); // Free input matrix data if allocated by as_matrix
  if (op_node) free(op_node); // Free the operand node

  return ret_val;
}

// Function: rpncalc_init
void rpncalc_init(RPNCalc_State *state) {
  if (state) {
    state->head = NULL;
  }
}

// Function: rpncalc_destroy
void rpncalc_destroy(RPNCalc_State *state) {
  clear(state);
}

// Function: perform_rpncalc_op
typedef undefined4 (*rpncalc_op_func)(RPNCalc_State *);

// Array of function pointers for RPN operations
rpncalc_op_func rpncalc_ops[] = {
    push,      // 0
    pop,       // 1
    add,       // 2
    sub,       // 3
    mul,       // 4
    divide,    // 5 (renamed from div to avoid conflict)
    inv,       // 6
    (rpncalc_op_func)clear // 7 (clear expects RPNCalc_State*, but its return value is ignored by perform_rpncalc_op)
};

undefined4 perform_rpncalc_op(RPNCalc_State *state, uint op_code) {
  if (op_code < sizeof(rpncalc_ops) / sizeof(rpncalc_ops[0])) {
    return rpncalc_ops[op_code](state);
  } else {
    return 0xffffffff; // Invalid operation code
  }
}