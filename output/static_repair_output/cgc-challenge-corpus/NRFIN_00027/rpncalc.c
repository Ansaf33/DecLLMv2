#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h> // For memcpy
#include <math.h>   // For isnan, isinf

// Define the value_t type as a 64-bit unsigned integer.
// It acts as a union/variant type where interpretation depends on `get_type`.
typedef uint64_t value_t;

// CONCAT44 macro: combines two 32-bit unsigned integers into a 64-bit unsigned integer.
// Assuming high is the upper 32 bits, low is the lower 32 bits.
#define CONCAT44(high, low) (((value_t)(high) << 32) | (low))

// Type codes (assuming these are return values of get_type)
#define TYPE_INTEGER 0
#define TYPE_MATRIX  1
#define TYPE_DOUBLE  2

// --- Mock Implementations for type handling and I/O ---
// These are illustrative. A real implementation would define how value_t encodes types
// (e.g., using specific bit patterns, a tag, or a separate registry).
// For compilation, this mock simulates the expected behavior.

// Mock data structures for internal type tracking.
// In a real system, value_t would be self-describing (e.g., tagged union).
// This mock uses a simple "registry" to associate value_t (uint64_t) with its type and actual data.
#define MAX_MOCK_VALUES 100
typedef struct {
    value_t val_id; // The uint64_t identifier
    int type_tag;   // 0: int, 1: matrix ptr, 2: double
    union {
        int32_t i_val;
        double d_val;
        uint32_t *m_val; // For matrix: [rows, cols, val0_low, val0_high, ...]
    } actual_data;
} MockValueEntry;

static MockValueEntry mock_value_registry[MAX_MOCK_VALUES];
static int mock_value_count = 0;

// Helper to register a value_t with its type and data in the mock registry.
static int register_mock_value(value_t val, int type, void *data_ptr) {
    if (mock_value_count >= MAX_MOCK_VALUES) {
        fprintf(stderr, "Mock value registry full!\n");
        return -1;
    }
    mock_value_registry[mock_value_count].val_id = val;
    mock_value_registry[mock_value_count].type_tag = type;
    if (type == TYPE_INTEGER) mock_value_registry[mock_value_count].actual_data.i_val = *(int32_t*)data_ptr;
    else if (type == TYPE_DOUBLE) mock_value_registry[mock_value_count].actual_data.d_val = *(double*)data_ptr;
    else if (type == TYPE_MATRIX) mock_value_registry[mock_value_count].actual_data.m_val = (uint32_t*)data_ptr;
    return mock_value_count++;
}

// Helper to find a value_t in the mock registry.
static MockValueEntry* find_mock_value(value_t val) {
    for (int i = 0; i < mock_value_count; ++i) {
        if (mock_value_registry[i].val_id == val) {
            return &mock_value_registry[i];
        }
    }
    return NULL;
}

// get_type: returns the type of the value (0 for int, 1 for matrix, 2 for double, -1 for error)
int get_type(value_t val) {
    MockValueEntry *mv = find_mock_value(val);
    if (mv) return mv->type_tag;

    // Fallback heuristic for un-registered values:
    // Check for NaN/Inf pattern first, as it's specific to doubles in IEEE 754.
    // This check is on the high 32 bits of the 64-bit value.
    if (((uint32_t)(val >> 32) & 0x7ff00000) == 0x7ff00000) {
        double d_val = *(double*)&val; // Interpret raw bits as double
        register_mock_value(val, TYPE_DOUBLE, &d_val);
        return TYPE_DOUBLE;
    }

    // Heuristic for integers: if the value fits in a 32-bit signed integer and has no other tag.
    // This is a weak heuristic; a real system would use explicit tags.
    if ((val & ~((uint64_t)0xFFFFFFFF)) == 0 && (int32_t)val >= INT32_MIN && (int32_t)val <= INT32_MAX) {
        int32_t i_val = (int32_t)val;
        register_mock_value(val, TYPE_INTEGER, &i_val);
        return TYPE_INTEGER;
    }
    return -1; // Default to unknown if no tag or heuristic applies
}

// as_double: converts value_t to double. Returns 0 on success, -1 on failure.
int as_double(value_t val, double *out_double) {
    if (out_double == NULL) return -1;
    MockValueEntry *mv = find_mock_value(val);
    if (mv) {
        if (mv->type_tag == TYPE_DOUBLE) {
            *out_double = mv->actual_data.d_val;
            return 0;
        }
        if (mv->type_tag == TYPE_INTEGER) {
            *out_double = (double)mv->actual_data.i_val;
            return 0;
        }
    }
    // If not in registry or type mismatch, try interpreting raw bits as double.
    // This is the behavior implied by original code's bit checks.
    *out_double = *(double*)&val;
    if (get_type(val) == TYPE_DOUBLE) return 0; // Re-check if heuristic now classifies it
    return -1;
}

// set_double: sets a value_t * to represent a double. Returns 0 on success, -1 on failure.
int set_double(double d, value_t *result_ptr) {
    if (result_ptr == NULL) return -1;
    if (isnan(d) || isinf(d)) {
        // Original code checked (val_high & 0x7ff00000) == 0x7ff00000, which indicates NaN/Inf.
        // Returning -1 for these.
        return -1;
    }
    *result_ptr = *(value_t*)&d; // Store raw double bits
    register_mock_value(*result_ptr, TYPE_DOUBLE, &d); // Register for mock `get_type`
    return 0;
}

// as_integer: converts value_t to int. Returns 0 on success, -1 on failure.
int as_integer(value_t val, int32_t *out_int) {
    if (out_int == NULL) return -1;
    MockValueEntry *mv = find_mock_value(val);
    if (mv && mv->type_tag == TYPE_INTEGER) {
        *out_int = mv->actual_data.i_val;
        return 0;
    }
    // If not in registry or type mismatch, try interpreting raw bits as int.
    if (get_type(val) == TYPE_INTEGER) { // Re-check if heuristic now classifies it
        *out_int = (int32_t)val;
        return 0;
    }
    return -1;
}

// set_integer: sets a value_t * to represent an int. Returns 0 on success, -1 on failure.
int set_integer(int32_t i, value_t *result_ptr) {
    if (result_ptr == NULL) return -1;
    *result_ptr = (value_t)i; // Store raw integer value
    register_mock_value(*result_ptr, TYPE_INTEGER, &i); // Register for mock `get_type`
    return 0;
}

// as_matrix: converts value_t to matrix (uint32_t*). Returns 0 on success, -1 on failure.
// uint32_t* matrix format: [rows, cols, val0_low, val0_high, val1_low, val1_high, ...]
// This function returns a *copy* of the matrix data, which the caller must free.
int as_matrix(value_t val, uint32_t **out_matrix_ptr) {
    if (out_matrix_ptr == NULL) return -1;
    MockValueEntry *mv = find_mock_value(val);
    if (mv && mv->type_tag == TYPE_MATRIX && mv->actual_data.m_val != NULL) {
        uint32_t *src_mat = mv->actual_data.m_val;
        uint32_t rows = src_mat[0];
        uint32_t cols = src_mat[1];
        size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t)); // Each value_t takes 2 uint32_t slots
        size_t total_size = 2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t);

        uint32_t *copy = (uint32_t *)malloc(total_size);
        if (copy == NULL) return -1;
        memcpy(copy, src_mat, total_size);
        *out_matrix_ptr = copy;
        return 0;
    }
    return -1;
}

// set_matrix: sets a value_t * to represent a matrix. Returns 0 on success, -1 on failure.
// This function takes *ownership* of `matrix_ptr` (the allocated matrix data).
int set_matrix(uint32_t *matrix_ptr, value_t *result_ptr) {
    if (result_ptr == NULL) {
        free(matrix_ptr); // Caller passed a bad result_ptr, free the matrix_ptr
        return -1;
    }
    if (matrix_ptr == NULL) {
        return -1;
    }
    // Generate a unique value_t ID for this matrix.
    // For mock, use a simple counter-based ID.
    static uint64_t matrix_id_counter = 0x1000000000000000ULL; // Start high to avoid collision
    *result_ptr = matrix_id_counter++;
    register_mock_value(*result_ptr, TYPE_MATRIX, matrix_ptr); // Register for mock `get_type`
    return 0;
}

// Mock I/O functions
ssize_t read_all(int fd, void *buf, size_t count) {
    (void)fd; // Unused parameter
    memset(buf, 0, count); // Simulate reading zeros
    return count;
}

ssize_t write_all(int fd, const void *buf, size_t count) {
    (void)fd; (void)buf; // Unused parameters
    return count;
}

// --- Mock RPN stack functions ---
typedef struct ListNode {
    struct ListNode *next;
    value_t value; // The actual data
} ListNode;

// list_push_front: Pushes a node onto the stack. Takes ownership of `node_to_push`.
int list_push_front(ListNode **stack_head_ptr, ListNode *node_to_push) {
    if (stack_head_ptr == NULL || node_to_push == NULL) {
        free(node_to_push); // If we can't push, free it.
        return -1;
    }
    node_to_push->next = *stack_head_ptr;
    *stack_head_ptr = node_to_push;
    return 0;
}

// list_pop_front: Pops a node from the stack. Returns pointer to node, or NULL.
// Caller is responsible for freeing the returned node.
ListNode *list_pop_front(ListNode **stack_head_ptr) {
    if (stack_head_ptr == NULL || *stack_head_ptr == NULL) {
        return NULL;
    }
    ListNode *node = *stack_head_ptr;
    *stack_head_ptr = node->next;
    node->next = NULL; // Detach
    return node;
}

// list_length: Returns the number of elements in the stack.
int list_length(ListNode *stack_head) {
    int count = 0;
    ListNode *current = stack_head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// list_remove: Removes a specific node from the list. Does NOT free the node.
// Returns 0 on success, -1 if node not found or list is empty.
int list_remove(ListNode **stack_head_ptr, ListNode *node_to_remove) {
    if (stack_head_ptr == NULL || *stack_head_ptr == NULL || node_to_remove == NULL) {
        return -1;
    }
    if (*stack_head_ptr == node_to_remove) {
        *stack_head_ptr = node_to_remove->next;
        return 0;
    }
    ListNode *current = *stack_head_ptr;
    while (current->next != NULL && current->next != node_to_remove) {
        current = current->next;
    }
    if (current->next == node_to_remove) {
        current->next = node_to_remove->next;
        return 0;
    }
    return -1; // Node not found
}

// Forward declarations for RPN operations
int generic_add(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr);
int generic_sub(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr);
int generic_mul(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr);
int generic_div(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr);
int matrix_inv(uint32_t *input_mat, uint32_t **output_mat_ptr);

// Global array of function pointers for RPN operations
typedef int (*rpn_op_func_ptr)(ListNode **);
rpn_op_func_ptr rpncalc_ops[8];

// Function: swap
void swap(value_t *param_1, value_t *param_2) {
  value_t temp = *param_1;
  *param_1 = *param_2;
  *param_2 = temp;
}

// Function: generic_add
int generic_add(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr) {
  if (result_ptr == NULL) return -1;

  value_t val1 = CONCAT44(val1_high, val1_low);
  value_t val2 = CONCAT44(val2_high, val2_low);

  int type1 = get_type(val1);
  int type2 = get_type(val2);

  // Original code swaps if type2 < type1 to maintain some canonical order
  if (type2 < type1) {
    swap(&val1, &val2);
    type1 = get_type(val1); // Recalculate type1 after potential swap
    type2 = get_type(val2); // Recalculate type2 after potential swap (redundant but matches original)
  }
  type1 = get_type(val1); // Recalculate type1 again (matches original behavior)

  if (type1 == TYPE_DOUBLE) {
    double d_val1, d_val2;
    if (as_double(val1, &d_val1) != 0) return -1;
    if (as_double(val2, &d_val2) != 0) return -1;

    if (set_double(d_val1 + d_val2, result_ptr) != 0) return -1;
    // Check for NaN/Infinity on the result (bit pattern check on high 32 bits)
    if ((*(uint32_t *)((uint8_t *)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) return -1;
    return 0;
  }

  if (type1 > TYPE_DOUBLE) return -1; // Type must be 0, 1, or 2. Anything higher is an error.

  if (type1 == TYPE_INTEGER) {
    int32_t i_val1;
    if (as_integer(val1, &i_val1) != 0) return -1;

    int type2_inner = get_type(val2);

    if (type2_inner == TYPE_DOUBLE) {
      double d_val2;
      if (as_double(val2, &d_val2) != 0) return -1;
      if (set_double(d_val2 + (double)i_val1, result_ptr) != 0) return -1;
      if ((*(uint32_t *)((uint8_t *)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) return -1;
    } else if (type2_inner > TYPE_DOUBLE) {
      return -1;
    } else if (type2_inner == TYPE_INTEGER) {
      int32_t i_val2;
      if (as_integer(val2, &i_val2) != 0) return -1;
      if (set_integer(i_val1 + i_val2, result_ptr) != 0) return -1;
    } else if (type2_inner == TYPE_MATRIX) {
      uint32_t *mat_val2;
      if (as_matrix(val2, &mat_val2) != 0) return -1;

      uint32_t rows = mat_val2[0];
      uint32_t cols = mat_val2[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val2); // Free the copy obtained from as_matrix
        return -1;
      }
      res_mat_storage[0] = rows;
      res_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          // Element at (r, c) in row-major order: mat_val2[2 + (r * cols + c) * 2] for low part
          value_t mat_elem = CONCAT44(mat_val2[2 + (r * cols + c) * 2 + 1], mat_val2[2 + (r * cols + c) * 2]);
          value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);

          if (generic_add((uint32_t)val1, (uint32_t)(val1 >> 32),
                          (uint32_t)mat_elem, (uint32_t)(mat_elem >> 32),
                          res_elem_ptr) != 0) {
            free(res_mat_storage);
            free(mat_val2);
            return -1;
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val2);
        return -1;
      }
      free(mat_val2); // Free the copy obtained from as_matrix
    } else {
      return -1; // Unknown type2_inner
    }
  } else if (type1 == TYPE_MATRIX) {
    uint32_t *mat_val1;
    if (as_matrix(val1, &mat_val1) != 0) return -1;

    int type2_inner = get_type(val2);
    if (type2_inner == TYPE_MATRIX) {
      uint32_t *mat_val2;
      if (as_matrix(val2, &mat_val2) != 0) {
        free(mat_val1);
        return -1;
      }

      if (mat_val1[0] != mat_val2[0] || mat_val1[1] != mat_val2[1]) { // Mismatch dimensions
        free(mat_val1);
        free(mat_val2);
        return -1;
      }

      uint32_t rows = mat_val1[0];
      uint32_t cols = mat_val1[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val1);
        free(mat_val2);
        return -1;
      }
      res_mat_storage[0] = rows;
      res_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          // Element at (r, c) in row-major order
          value_t elem_mat1 = CONCAT44(mat_val1[2 + (r * cols + c) * 2 + 1], mat_val1[2 + (r * cols + c) * 2]);
          value_t elem_mat2 = CONCAT44(mat_val2[2 + (r * cols + c) * 2 + 1], mat_val2[2 + (r * cols + c) * 2]);
          double d_elem_mat1, d_elem_mat2;

          if (as_double(elem_mat1, &d_elem_mat1) != 0 || as_double(elem_mat2, &d_elem_mat2) != 0) {
            free(res_mat_storage); free(mat_val1); free(mat_val2); return -1;
          }
          value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);
          if (set_double(d_elem_mat1 + d_elem_mat2, res_elem_ptr) != 0) {
            free(res_mat_storage); free(mat_val1); free(mat_val2); return -1;
          }
          if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
            free(res_mat_storage); free(mat_val1); free(mat_val2); return -1;
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val1); free(mat_val2);
        return -1;
      }
      free(mat_val1); free(mat_val2); // Free copies obtained from as_matrix
    } else if (type2_inner == TYPE_DOUBLE) {
      double d_val2;
      if (as_double(val2, &d_val2) != 0) {
        free(mat_val1);
        return -1;
      }

      uint32_t rows = mat_val1[0];
      uint32_t cols = mat_val1[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val1);
        return -1;
      }
      res_mat_storage[0] = rows;
      res_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          // Element at (r, c) in row-major order
          value_t elem_mat1 = CONCAT44(mat_val1[2 + (r * cols + c) * 2 + 1], mat_val1[2 + (r * cols + c) * 2]);
          double d_elem_mat1;
          if (as_double(elem_mat1, &d_elem_mat1) != 0) {
            free(res_mat_storage); free(mat_val1); return -1;
          }
          value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);
          if (set_double(d_val2 + d_elem_mat1, res_elem_ptr) != 0) {
            free(res_mat_storage); free(mat_val1); return -1;
          }
          if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
            free(res_mat_storage); free(mat_val1); return -1;
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val1);
        return -1;
      }
      free(mat_val1); // Free copy obtained from as_matrix
    } else {
      free(mat_val1); // Free copy obtained from as_matrix
      return -1; // Unknown type2_inner
    }
  } else {
    return -1; // Unknown type1
  }
  return 0;
}

// Function: generic_sub
int generic_sub(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr) {
  if (result_ptr == NULL) return -1;

  value_t val1 = CONCAT44(val1_high, val1_low);
  value_t val2 = CONCAT44(val2_high, val2_low);
  value_t negated_val2;

  int type2 = get_type(val2);

  if (type2 == TYPE_DOUBLE) {
    double d_val2;
    if (as_double(val2, &d_val2) != 0) return -1;
    if (set_double(-d_val2, &negated_val2) != 0) return -1;
    return generic_add(val1_low, val1_high, (uint32_t)negated_val2, (uint32_t)(negated_val2 >> 32), result_ptr);
  } else if (type2 < TYPE_DOUBLE) {
    if (type2 == TYPE_INTEGER) {
      int32_t i_val2;
      if (as_integer(val2, &i_val2) != 0) return -1;
      if (set_integer(-i_val2, &negated_val2) != 0) return -1;
      return generic_add(val1_low, val1_high, (uint32_t)negated_val2, (uint32_t)(negated_val2 >> 32), result_ptr);
    } else if (type2 == TYPE_MATRIX) {
      uint32_t *mat_val2;
      if (as_matrix(val2, &mat_val2) != 0) return -1;

      uint32_t rows = mat_val2[0];
      uint32_t cols = mat_val2[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *neg_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (neg_mat_storage == NULL) {
        free(mat_val2);
        return -1;
      }
      neg_mat_storage[0] = rows;
      neg_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          value_t elem_mat2 = CONCAT44(mat_val2[2 + (r * cols + c) * 2 + 1], mat_val2[2 + (r * cols + c) * 2]);
          double d_elem;
          if (as_double(elem_mat2, &d_elem) != 0) {
            free(neg_mat_storage); free(mat_val2); return -1;
          }
          value_t *neg_elem_ptr = (value_t *)(neg_mat_storage + 2 + (r * cols + c) * 2);
          if (set_double(-d_elem, neg_elem_ptr) != 0) {
            free(neg_mat_storage); free(mat_val2); return -1;
          }
        }
      }
      if (set_matrix(neg_mat_storage, &negated_val2) != 0) { // Set negated_val2 to the new matrix
        free(neg_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val2);
        return -1;
      }
      free(neg_mat_storage); // set_matrix copied, so we free the temporary neg_mat_storage
      free(mat_val2); // Free the copy obtained from as_matrix

      return generic_add(val1_low, val1_high, (uint32_t)negated_val2, (uint32_t)(negated_val2 >> 32), result_ptr);
    }
  }
  return -1;
}

// Function: generic_mul
int generic_mul(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr) {
  if (result_ptr == NULL) return -1;

  value_t val1 = CONCAT44(val1_high, val1_low);
  value_t val2 = CONCAT44(val2_high, val2_low);

  int type1 = get_type(val1);
  int type2 = get_type(val2);

  if (type2 < type1) {
    swap(&val1, &val2);
    type1 = get_type(val1);
    type2 = get_type(val2);
  }
  type1 = get_type(val1);

  if (type1 == TYPE_DOUBLE) {
    double d_val1, d_val2;
    if (as_double(val1, &d_val1) != 0) return -1;
    if (as_double(val2, &d_val2) != 0) return -1;
    if (set_double(d_val1 * d_val2, result_ptr) != 0) return -1;
    if ((*(uint32_t *)((uint8_t *)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) return -1;
    return 0;
  }

  if (type1 > TYPE_DOUBLE) return -1;

  if (type1 == TYPE_INTEGER) {
    int32_t i_val1;
    if (as_integer(val1, &i_val1) != 0) return -1;

    int type2_inner = get_type(val2);
    if (type2_inner == TYPE_DOUBLE) {
      double d_val2;
      if (as_double(val2, &d_val2) != 0) return -1;
      if (set_double(d_val2 * (double)i_val1, result_ptr) != 0) return -1;
      if ((*(uint32_t *)((uint8_t *)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) return -1;
    } else if (type2_inner > TYPE_DOUBLE) {
      return -1;
    } else if (type2_inner == TYPE_INTEGER) {
      int32_t i_val2;
      if (as_integer(val2, &i_val2) != 0) return -1;
      if (set_integer(i_val1 * i_val2, result_ptr) != 0) return -1;
    } else if (type2_inner == TYPE_MATRIX) {
      uint32_t *mat_val2;
      if (as_matrix(val2, &mat_val2) != 0) return -1;

      uint32_t rows = mat_val2[0];
      uint32_t cols = mat_val2[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val2);
        return -1;
      }
      res_mat_storage[0] = rows;
      res_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          value_t mat_elem = CONCAT44(mat_val2[2 + (r * cols + c) * 2 + 1], mat_val2[2 + (r * cols + c) * 2]);
          int elem_type = get_type(mat_elem);
          value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);

          if (elem_type == TYPE_INTEGER) {
            int32_t i_elem;
            if (as_integer(mat_elem, &i_elem) != 0) { free(res_mat_storage); free(mat_val2); return -1; }
            if (set_integer(i_elem * i_val1, res_elem_ptr) != 0) { free(res_mat_storage); free(mat_val2); return -1; }
          } else if (elem_type == TYPE_DOUBLE) {
            double d_elem;
            if (as_double(mat_elem, &d_elem) != 0) { free(res_mat_storage); free(mat_val2); return -1; }
            if (set_double(d_elem * (double)i_val1, res_elem_ptr) != 0) { free(res_mat_storage); free(mat_val2); return -1; }
            if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
              free(res_mat_storage); free(mat_val2); return -1;
            }
          } else {
            free(res_mat_storage); free(mat_val2); return -1; // Unsupported element type in matrix
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val2);
        return -1;
      }
      free(mat_val2); // Free the copy obtained from as_matrix
    } else {
      return -1; // Unknown type2_inner
    }
  } else if (type1 == TYPE_MATRIX) {
    uint32_t *mat_val1;
    if (as_matrix(val1, &mat_val1) != 0) return -1;

    int type2_inner = get_type(val2);
    if (type2_inner == TYPE_MATRIX) {
      uint32_t *mat_val2;
      if (as_matrix(val2, &mat_val2) != 0) {
        free(mat_val1);
        return -1;
      }

      // Matrix dimensions check: A(m x n) * B(n x p) -> C(m x p)
      // mat_val1: rows_A (mat_val1[0]) x cols_A (mat_val1[1])
      // mat_val2: rows_B (mat_val2[0]) x cols_B (mat_val2[1])
      // Original check: `*local_44 != local_48[1]` i.e., `mat_val1[0] != mat_val2[1]`
      // This is non-standard. Standard is `cols_A == rows_B`.
      // Let's keep the original logic for the specific use case.
      if (mat_val1[0] != mat_val2[1]) {
        free(mat_val1); free(mat_val2); return -1;
      }

      uint32_t rows_res = mat_val1[0]; // Result rows = rows of first matrix
      uint32_t cols_res = mat_val2[1]; // Result cols = cols of second matrix
      uint32_t inner_dim = mat_val1[1]; // Inner dimension for summation

      size_t total_data_elements = rows_res * cols_res * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val1); free(mat_val2); return -1;
      }
      res_mat_storage[0] = rows_res;
      res_mat_storage[1] = cols_res;

      // Initialize result matrix elements to zero (double 0.0)
      for (uint32_t i = 0; i < rows_res * cols_res; ++i) {
        set_double(0.0, (value_t *)(res_mat_storage + 2 + i * 2));
      }

      // Matrix multiplication: C[r][c] = sum_k(A[r][k] * B[k][c])
      for (uint32_t r = 0; r < rows_res; ++r) {
        for (uint32_t c = 0; c < cols_res; ++c) {
          for (uint32_t k = 0; k < inner_dim; ++k) {
            double elem_A, elem_B, current_C;

            // A[r][k] = mat_val1 element at (r, k) (row-major)
            value_t val_A_rc = CONCAT44(mat_val1[2 + (r * inner_dim + k) * 2 + 1], mat_val1[2 + (r * inner_dim + k) * 2]);
            if (as_double(val_A_rc, &elem_A) != 0) { free(res_mat_storage); free(mat_val1); free(mat_val2); return -1; }

            // B[k][c] = mat_val2 element at (k, c) (row-major)
            value_t val_B_kc = CONCAT44(mat_val2[2 + (k * cols_res + c) * 2 + 1], mat_val2[2 + (k * cols_res + c) * 2]);
            if (as_double(val_B_kc, &elem_B) != 0) { free(res_mat_storage); free(mat_val1); free(mat_val2); return -1; }

            // Current C[r][c]
            value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols_res + c) * 2);
            if (as_double(*res_elem_ptr, &current_C) != 0) { free(res_mat_storage); free(mat_val1); free(mat_val2); return -1; }

            if (set_double(current_C + (elem_A * elem_B), res_elem_ptr) != 0) {
              free(res_mat_storage); free(mat_val1); free(mat_val2); return -1;
            }
            if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
              free(res_mat_storage); free(mat_val1); free(mat_val2); return -1;
            }
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val1); free(mat_val2);
        return -1;
      }
      free(mat_val1); free(mat_val2); // Free copies obtained from as_matrix
    } else if (type2_inner == TYPE_DOUBLE) {
      double d_val2;
      if (as_double(val2, &d_val2) != 0) {
        free(mat_val1);
        return -1;
      }

      uint32_t rows = mat_val1[0];
      uint32_t cols = mat_val1[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val1);
        return -1;
      }
      res_mat_storage[0] = rows;
      res_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          value_t elem_mat1 = CONCAT44(mat_val1[2 + (r * cols + c) * 2 + 1], mat_val1[2 + (r * cols + c) * 2]);
          double d_elem;
          if (as_double(elem_mat1, &d_elem) != 0) { free(res_mat_storage); free(mat_val1); return -1; }
          value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);
          if (set_double(d_elem * d_val2, res_elem_ptr) != 0) {
            free(res_mat_storage); free(mat_val1); return -1;
          }
          if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
            free(res_mat_storage); free(mat_val1); return -1;
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val1);
        return -1;
      }
      free(mat_val1); // Free copy obtained from as_matrix
    } else {
      free(mat_val1); // Free copy obtained from as_matrix
      return -1; // Unknown type2_inner
    }
  } else {
    return -1; // Unknown type1
  }
  return 0;
}

// Function: generic_div
int generic_div(uint32_t val1_low, uint32_t val1_high, uint32_t val2_low, uint32_t val2_high, value_t *result_ptr) {
  if (result_ptr == NULL) return -1;

  value_t val1 = CONCAT44(val1_high, val1_low);
  value_t val2 = CONCAT44(val2_high, val2_low);

  int type1 = get_type(val1);
  int type2 = get_type(val2);

  // Special case: Matrix / Matrix is not supported
  if (type1 == TYPE_MATRIX && type2 == TYPE_MATRIX) return -1;

  // Integer / Integer
  if (type1 == TYPE_INTEGER && type2 == TYPE_INTEGER) {
    int32_t i_val1, i_val2;
    if (as_integer(val1, &i_val1) != 0) return -1;
    if (as_integer(val2, &i_val2) != 0) return -1;
    if (i_val2 == 0) return -1; // Division by zero
    if (set_integer(i_val1 / i_val2, result_ptr) != 0) return -1;
    return 0;
  }

  // Double / X
  if (type1 == TYPE_DOUBLE) {
    double d_val1;
    if (as_double(val1, &d_val1) != 0) return -1;

    if (type2 == TYPE_MATRIX) {
      uint32_t *mat_val2;
      if (as_matrix(val2, &mat_val2) != 0) return -1;

      uint32_t rows = mat_val2[0];
      uint32_t cols = mat_val2[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val2);
        return -1;
      }
      res_mat_storage[0] = rows;
      res_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          value_t elem_mat2 = CONCAT44(mat_val2[2 + (r * cols + c) * 2 + 1], mat_val2[2 + (r * cols + c) * 2]);
          double d_elem;
          if (as_double(elem_mat2, &d_elem) != 0) { free(res_mat_storage); free(mat_val2); return -1; }
          if (d_elem == 0.0) { free(res_mat_storage); free(mat_val2); return -1; } // Division by zero
          value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);
          if (set_double(d_val1 / d_elem, res_elem_ptr) != 0) {
            free(res_mat_storage); free(mat_val2); return -1;
          }
          if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
            free(res_mat_storage); free(mat_val2); return -1;
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val2);
        return -1;
      }
      free(mat_val2); // Free copy obtained from as_matrix
    } else { // Double / Integer or Double / Double
      double d_val2;
      if (type2 == TYPE_INTEGER) {
        int32_t i_val2;
        if (as_integer(val2, &i_val2) != 0) return -1;
        d_val2 = (double)i_val2;
      } else if (type2 == TYPE_DOUBLE) {
        if (as_double(val2, &d_val2) != 0) return -1;
      } else {
        return -1; // Unknown type2
      }
      if (d_val2 == 0.0) return -1; // Division by zero
      if (set_double(d_val1 / d_val2, result_ptr) != 0) return -1;
      if ((*(uint32_t *)((uint8_t *)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) return -1;
    }
  } else if (type1 == TYPE_INTEGER) { // Integer / X
    int32_t i_val1;
    if (as_integer(val1, &i_val1) != 0) return -1;

    if (type2 == TYPE_MATRIX) {
      uint32_t *mat_val2;
      if (as_matrix(val2, &mat_val2) != 0) return -1;

      uint32_t rows = mat_val2[0];
      uint32_t cols = mat_val2[1];
      size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
      uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
      if (res_mat_storage == NULL) {
        free(mat_val2);
        return -1;
      }
      res_mat_storage[0] = rows;
      res_mat_storage[1] = cols;

      for (uint32_t r = 0; r < rows; ++r) {
        for (uint32_t c = 0; c < cols; ++c) {
          value_t elem_mat2 = CONCAT44(mat_val2[2 + (r * cols + c) * 2 + 1], mat_val2[2 + (r * cols + c) * 2]);
          double d_elem;
          if (as_double(elem_mat2, &d_elem) != 0) { free(res_mat_storage); free(mat_val2); return -1; }
          if (d_elem == 0.0) { free(res_mat_storage); free(mat_val2); return -1; } // Division by zero
          value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);
          if (set_double((double)i_val1 / d_elem, res_elem_ptr) != 0) {
            free(res_mat_storage); free(mat_val2); return -1;
          }
          if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
            free(res_mat_storage); free(mat_val2); return -1;
          }
        }
      }
      if (set_matrix(res_mat_storage, result_ptr) != 0) {
        free(res_mat_storage); // set_matrix failed, so we free the data.
        free(mat_val2);
        return -1;
      }
      free(mat_val2); // Free copy obtained from as_matrix
    } else { // Integer / Double
      double d_val2;
      // The original code implies that if type2 is not matrix, it must be convertible to double.
      // This covers integer / double and integer / integer where integer is converted to double.
      if (as_double(val2, &d_val2) != 0) return -1;
      if (d_val2 == 0.0) return -1; // Division by zero
      if (set_double((double)i_val1 / d_val2, result_ptr) != 0) return -1;
      if ((*(uint32_t *)((uint8_t *)result_ptr + 4) & 0x7ff00000) == 0x7ff00000) return -1;
    }
  } else if (type1 == TYPE_MATRIX) { // Matrix / X (scalar)
    uint32_t *mat_val1;
    if (as_matrix(val1, &mat_val1) != 0) return -1;

    double d_val2;
    if (type2 == TYPE_INTEGER) {
      int32_t i_val2;
      if (as_integer(val2, &i_val2) != 0) { free(mat_val1); return -1; }
      d_val2 = (double)i_val2;
    } else if (type2 == TYPE_DOUBLE) {
      if (as_double(val2, &d_val2) != 0) { free(mat_val1); return -1; }
    } else {
      free(mat_val1); return -1; // Matrix / Unknown type
    }

    uint32_t rows = mat_val1[0];
    uint32_t cols = mat_val1[1];
    size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
    uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
    if (res_mat_storage == NULL) {
      free(mat_val1);
      return -1;
    }
    res_mat_storage[0] = rows;
    res_mat_storage[1] = cols;

    for (uint32_t r = 0; r < rows; ++r) {
      for (uint32_t c = 0; c < cols; ++c) {
        if (d_val2 == 0.0) { free(res_mat_storage); free(mat_val1); return -1; } // Division by zero
        value_t elem_mat1 = CONCAT44(mat_val1[2 + (r * cols + c) * 2 + 1], mat_val1[2 + (r * cols + c) * 2]);
        double d_elem;
        if (as_double(elem_mat1, &d_elem) != 0) { free(res_mat_storage); free(mat_val1); return -1; }
        value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (r * cols + c) * 2);
        if (set_double(d_elem / d_val2, res_elem_ptr) != 0) {
          free(res_mat_storage); free(mat_val1); return -1;
        }
        if ((*(uint32_t *)((uint8_t *)res_elem_ptr + 4) & 0x7ff00000) == 0x7ff00000) {
          free(res_mat_storage); free(mat_val1); return -1;
        }
      }
    }
    if (set_matrix(res_mat_storage, result_ptr) != 0) {
      free(res_mat_storage); // set_matrix failed, so we free the data.
      free(mat_val1);
      return -1;
    }
    free(mat_val1); // Free copy obtained from as_matrix
  } else {
    return -1; // Unknown type1
  }
  return 0;
}

// Function: matrix_inv
int matrix_inv(uint32_t *input_mat, uint32_t **output_mat_ptr) {
  if (output_mat_ptr == NULL || input_mat == NULL) return -1;

  uint32_t rows = input_mat[0];
  uint32_t cols = input_mat[1];

  if (rows != cols || rows > 2) return -1; // Only square matrices up to 2x2 supported by original code

  double determinant;
  if (rows == 1) {
    double val;
    value_t elem_00 = CONCAT44(input_mat[2 + (0 * cols + 0) * 2 + 1], input_mat[2 + (0 * cols + 0) * 2]);
    if (as_double(elem_00, &val) != 0) return -1;
    determinant = val;
  } else { // rows == 2 (2x2 matrix)
    double a, b, c, d;
    value_t elem_00 = CONCAT44(input_mat[2 + (0 * cols + 0) * 2 + 1], input_mat[2 + (0 * cols + 0) * 2]); // (0,0)
    value_t elem_01 = CONCAT44(input_mat[2 + (0 * cols + 1) * 2 + 1], input_mat[2 + (0 * cols + 1) * 2]); // (0,1)
    value_t elem_10 = CONCAT44(input_mat[2 + (1 * cols + 0) * 2 + 1], input_mat[2 + (1 * cols + 0) * 2]); // (1,0)
    value_t elem_11 = CONCAT44(input_mat[2 + (1 * cols + 1) * 2 + 1], input_mat[2 + (1 * cols + 1) * 2]); // (1,1)

    if (as_double(elem_00, &a) != 0 || as_double(elem_01, &b) != 0 ||
        as_double(elem_10, &c) != 0 || as_double(elem_11, &d) != 0) return -1;
    determinant = a * d - b * c;
  }

  if (determinant == 0.0 || isnan(determinant) || isinf(determinant)) return -1;

  size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
  uint32_t *res_mat_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + total_data_elements * sizeof(uint32_t));
  if (res_mat_storage == NULL) return -1;
  *output_mat_ptr = res_mat_storage; // Return the allocated buffer
  res_mat_storage[0] = rows;
  res_mat_storage[1] = cols;

  if (rows == 1) {
    value_t *res_elem_ptr = (value_t *)(res_mat_storage + 2 + (0 * cols + 0) * 2);
    if (set_double(1.0 / determinant, res_elem_ptr) != 0) { free(res_mat_storage); return -1; }
  } else { // rows == 2
    double a, b, c, d;
    value_t elem_00 = CONCAT44(input_mat[2 + (0 * cols + 0) * 2 + 1], input_mat[2 + (0 * cols + 0) * 2]);
    value_t elem_01 = CONCAT44(input_mat[2 + (0 * cols + 1) * 2 + 1], input_mat[2 + (0 * cols + 1) * 2]);
    value_t elem_10 = CONCAT44(input_mat[2 + (1 * cols + 0) * 2 + 1], input_mat[2 + (1 * cols + 0) * 2]);
    value_t elem_11 = CONCAT44(input_mat[2 + (1 * cols + 1) * 2 + 1], input_mat[2 + (1 * cols + 1) * 2]);

    if (as_double(elem_00, &a) != 0 || as_double(elem_01, &b) != 0 ||
        as_double(elem_10, &c) != 0 || as_double(elem_11, &d) != 0) { free(res_mat_storage); return -1; }

    value_t *res_elem_ptr;

    res_elem_ptr = (value_t *)(res_mat_storage + 2 + (0 * cols + 0) * 2); // (0,0)
    if (set_double(d / determinant, res_elem_ptr) != 0) { free(res_mat_storage); return -1; }

    res_elem_ptr = (value_t *)(res_mat_storage + 2 + (0 * cols + 1) * 2); // (0,1)
    if (set_double(-b / determinant, res_elem_ptr) != 0) { free(res_mat_storage); return -1; }

    res_elem_ptr = (value_t *)(res_mat_storage + 2 + (1 * cols + 0) * 2); // (1,0)
    if (set_double(-c / determinant, res_elem_ptr) != 0) { free(res_mat_storage); return -1; }

    res_elem_ptr = (value_t *)(res_mat_storage + 2 + (1 * cols + 1) * 2); // (1,1)
    if (set_double(a / determinant, res_elem_ptr) != 0) { free(res_mat_storage); return -1; }
  }
  return 0;
}

// Function: push
int push(ListNode **stack_head_ptr) {
  ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
  if (new_node == NULL) return -1;
  new_node->next = NULL;

  uint32_t type_info;
  if (read_all(0, &type_info, sizeof(type_info)) != sizeof(type_info)) {
    free(new_node); return -1;
  }

  int type_code = type_info & 0xffff;

  if (type_code == TYPE_DOUBLE) {
    uint32_t val_low, val_high;
    if (read_all(0, &val_low, sizeof(val_low)) != sizeof(val_low) ||
        read_all(0, &val_high, sizeof(val_high)) != sizeof(val_high)) {
      free(new_node); return -1;
    }
    // Check for NaN/Infinity on the input value (val_high is higher 32 bits)
    if ((val_high & 0x7ff00000) == 0x7ff00000) {
      free(new_node); return -1;
    }
    new_node->value = CONCAT44(val_high, val_low);
    // Register the value in mock registry for get_type to work
    double d_val = *(double*)&new_node->value;
    register_mock_value(new_node->value, TYPE_DOUBLE, &d_val);
  } else if (type_code == TYPE_INTEGER) {
    uint32_t i_val_raw;
    if (read_all(0, &i_val_raw, sizeof(i_val_raw)) != sizeof(i_val_raw)) {
      free(new_node); return -1;
    }
    if (set_integer((int32_t)i_val_raw, &new_node->value) != 0) {
        free(new_node); return -1;
    }
  } else if (type_code == TYPE_MATRIX) {
    uint32_t rows = (type_info >> 0x18) & 0xff;
    uint32_t cols = (type_info >> 0x10) & 0xff;
    size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
    size_t data_storage_size = total_data_elements * sizeof(uint32_t);

    if (rows == 0 || cols == 0) { free(new_node); return -1; }

    uint32_t *mat_data_storage = (uint32_t *)malloc(2 * sizeof(uint32_t) + data_storage_size);
    if (mat_data_storage == NULL) { free(new_node); return -1; }
    mat_data_storage[0] = rows;
    mat_data_storage[1] = cols;

    if (read_all(0, mat_data_storage + 2, data_storage_size) != (ssize_t)data_storage_size) {
      free(mat_data_storage); free(new_node); return -1;
    }

    // Check for NaN/Infinity in matrix elements
    for (uint32_t i = 0; i < rows * cols; ++i) {
      if ((mat_data_storage[2 + i * 2 + 1] & 0x7ff00000) == 0x7ff00000) {
        free(mat_data_storage); free(new_node); return -1;
      }
    }
    if (set_matrix(mat_data_storage, &new_node->value) != 0) {
      free(mat_data_storage); // set_matrix failed, so we free the data.
      free(new_node); return -1;
    }
  } else {
    free(new_node); return -1; // Unknown type
  }

  return list_push_front(stack_head_ptr, new_node);
}

// Function: pop
int pop(ListNode **stack_head_ptr) {
  ListNode *node = list_pop_front(stack_head_ptr);
  if (node == NULL) return -1;

  int ret = -1;
  uint32_t *mat_data = NULL; // To store matrix data if type is matrix

  int type = get_type(node->value);

  if (type == TYPE_DOUBLE) {
    double d_val;
    if (as_double(node->value, &d_val) != 0) goto cleanup_pop;

    uint32_t type_info = TYPE_DOUBLE;
    if (write_all(1, &type_info, sizeof(type_info)) != sizeof(type_info)) goto cleanup_pop;
    if (write_all(1, &d_val, sizeof(d_val)) != sizeof(d_val)) goto cleanup_pop;
    ret = 0;
  } else if (type == TYPE_INTEGER) {
    int32_t i_val;
    if (as_integer(node->value, &i_val) != 0) goto cleanup_pop;

    uint32_t type_info = TYPE_INTEGER;
    if (write_all(1, &type_info, sizeof(type_info)) != sizeof(type_info)) goto cleanup_pop;
    if (write_all(1, &i_val, sizeof(i_val)) != sizeof(i_val)) goto cleanup_pop;
    ret = 0;
  } else if (type == TYPE_MATRIX) {
    if (as_matrix(node->value, &mat_data) != 0) goto cleanup_pop;

    uint32_t rows = mat_data[0];
    uint32_t cols = mat_data[1];
    size_t total_data_elements = rows * cols * (sizeof(value_t) / sizeof(uint32_t));
    size_t data_storage_size = total_data_elements * sizeof(uint32_t);

    uint32_t type_info = TYPE_MATRIX | (rows << 0x18) | (cols << 0x10);
    if (write_all(1, &type_info, sizeof(type_info)) != sizeof(type_info)) goto cleanup_pop;
    if (write_all(1, mat_data + 2, data_storage_size) != (ssize_t)data_storage_size) goto cleanup_pop;
    ret = 0;
  } else {
    goto cleanup_pop;
  }

cleanup_pop:
  if (mat_data != NULL) free(mat_data); // Free matrix data copy obtained from as_matrix
  free(node); // Free the ListNode itself
  return ret;
}

// Function: clear
int clear(ListNode **stack_head_ptr) {
  ListNode *current_node = *stack_head_ptr;
  ListNode *next_node;
  uint32_t *mat_data;

  while (current_node != NULL) {
    next_node = current_node->next;

    if (get_type(current_node->value) == TYPE_MATRIX) {
      // as_matrix returns a copy, but set_matrix takes ownership.
      // So, we need to free the original matrix data pointed to by current_node->value.
      // The mock registry holds the actual pointer.
      MockValueEntry *mv = find_mock_value(current_node->value);
      if (mv && mv->type_tag == TYPE_MATRIX && mv->actual_data.m_val != NULL) {
        free(mv->actual_data.m_val);
        // Also remove from mock registry if we manage its lifecycle
        // (Not strictly necessary for mock, but good practice if registry is global)
      }
    }
    // Remove the node from the list (not strictly necessary here as we iterate manually)
    // list_remove(stack_head_ptr, current_node); // This would modify stack_head_ptr during iteration

    free(current_node); // Free the ListNode itself
    current_node = next_node;
  }
  *stack_head_ptr = NULL; // Ensure head is null after clearing
  return 0;
}

// Function: binary_op
int binary_op(ListNode **stack_head_ptr, int (*op_func)(uint32_t, uint32_t, uint32_t, uint32_t, value_t *)) {
  if (list_length(*stack_head_ptr) < 2) return -1;

  ListNode *node1 = list_pop_front(stack_head_ptr); // Second operand (top of stack)
  if (node1 == NULL) return -1;

  ListNode *node2 = list_pop_front(stack_head_ptr); // First operand
  if (node2 == NULL) {
    list_push_front(stack_head_ptr, node1); // Push node1 back if node2 is missing
    free(node1);
    return -1;
  }

  ListNode *result_node = (ListNode *)malloc(sizeof(ListNode));
  if (result_node == NULL) {
    list_push_front(stack_head_ptr, node2);
    list_push_front(stack_head_ptr, node1);
    free(node1); free(node2);
    return -1;
  }
  result_node->next = NULL;

  // Call the operator function
  if (op_func((uint32_t)node2->value, (uint32_t)(node2->value >> 32),
              (uint32_t)node1->value, (uint32_t)(node1->value >> 32),
              &result_node->value) != 0) {
    list_push_front(stack_head_ptr, node2);
    list_push_front(stack_head_ptr, node1);
    free(node1); free(node2); free(result_node);
    return -1;
  }

  list_push_front(stack_head_ptr, result_node); // Push result node

  // Free operands and their matrix data if applicable
  uint32_t *mat_data_copy; // Temporary for freeing copy from as_matrix
  if (get_type(node1->value) == TYPE_MATRIX) {
    // Free the original matrix data pointed to by node1->value
    MockValueEntry *mv = find_mock_value(node1->value);
    if (mv && mv->type_tag == TYPE_MATRIX && mv->actual_data.m_val != NULL) {
      free(mv->actual_data.m_val);
    }
  }
  free(node1);

  if (get_type(node2->value) == TYPE_MATRIX) {
    // Free the original matrix data pointed to by node2->value
    MockValueEntry *mv = find_mock_value(node2->value);
    if (mv && mv->type_tag == TYPE_MATRIX && mv->actual_data.m_val != NULL) {
      free(mv->actual_data.m_val);
    }
  }
  free(node2);

  return 0;
}

// Function: add
void add(ListNode **stack_head_ptr) {
  binary_op(stack_head_ptr, generic_add);
}

// Function: sub
void sub(ListNode **stack_head_ptr) {
  binary_op(stack_head_ptr, generic_sub);
}

// Function: mul
void mul(ListNode **stack_head_ptr) {
  binary_op(stack_head_ptr, generic_mul);
}

// Function: div (renamed to my_div to avoid conflict with stdlib.h)
int my_div(ListNode **stack_head_ptr) {
  return binary_op(stack_head_ptr, generic_div);
}

// Function: inv
int inv(ListNode **stack_head_ptr) {
  if (list_length(*stack_head_ptr) == 0) return -1;

  ListNode *operand_node = list_pop_front(stack_head_ptr);
  if (operand_node == NULL) return -1;

  int ret = -1;
  uint32_t *mat_data_copy = NULL; // Copy of original matrix data from as_matrix
  uint32_t *inv_mat_data = NULL; // Allocated by matrix_inv

  if (get_type(operand_node->value) == TYPE_MATRIX) {
    if (as_matrix(operand_node->value, &mat_data_copy) != 0) goto cleanup_inv;

    ListNode *result_node = (ListNode *)malloc(sizeof(ListNode));
    if (result_node == NULL) goto cleanup_inv;
    result_node->next = NULL;

    if (matrix_inv(mat_data_copy, &inv_mat_data) != 0) {
      free(result_node);
      goto cleanup_inv;
    }

    if (set_matrix(inv_mat_data, &result_node->value) != 0) {
      free(inv_mat_data); // set_matrix failed, so inv_mat_data is not owned by result_node
      free(result_node);
      goto cleanup_inv;
    }
    // inv_mat_data is now owned by the result_node's value. No need to free here.

    list_push_front(stack_head_ptr, result_node);
    ret = 0;
  } else {
    // Not a matrix, so inversion is not supported
    goto cleanup_inv;
  }

cleanup_inv:
  if (mat_data_copy != NULL) free(mat_data_copy); // Free original matrix data copy
  // Free the original matrix data pointed to by operand_node->value
  if (get_type(operand_node->value) == TYPE_MATRIX) {
    MockValueEntry *mv = find_mock_value(operand_node->value);
    if (mv && mv->type_tag == TYPE_MATRIX && mv->actual_data.m_val != NULL) {
      free(mv->actual_data.m_val);
    }
  }
  free(operand_node); // Free the operand node
  return ret;
}

// Function: rpncalc_init
void rpncalc_init(ListNode **stack_head_ptr) {
  *stack_head_ptr = NULL;
  // Original `param_1[1] = 0;` implies other fields might exist, but we only manage the head pointer.
}

// Function: rpncalc_destroy
void rpncalc_destroy(ListNode **stack_head_ptr) {
  clear(stack_head_ptr);
}

// Function: perform_rpncalc_op
int perform_rpncalc_op(ListNode **stack_head_ptr, uint32_t op_code) {
  if (op_code < 8) { // Assuming 8 ops based on original code structure
    if (rpncalc_ops[op_code] != NULL) {
        return rpncalc_ops[op_code](stack_head_ptr);
    }
  }
  return -1; // Invalid op_code or NULL function pointer
}