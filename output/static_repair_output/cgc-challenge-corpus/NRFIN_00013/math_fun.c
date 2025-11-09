#include <stddef.h>  // For size_t
#include <stdbool.h> // For true
#include <math.h>    // For round
#include <float.h>   // For DBL_MAX
#include <limits.h>  // For INT_MAX, INT_MIN

// Define a generic node structure for the linked list
// Assuming a structure where data_ptr, next_node, and prev_node are consecutive pointers.
// This is portable C. Original code's pointer arithmetic for `next_node` (e.g., `(int)local_10 + 4`)
// implies a 32-bit system where pointers are 4 bytes. This modern C code will use
// standard struct member access, which is correct for both 32-bit and 64-bit.
typedef struct Node {
    void *data_ptr;     // Pointer to the actual data (int* or double*)
    struct Node *next_node; // Pointer to the next Node
    struct Node *prev_node; // Pointer to the previous Node
} Node;

// Define the list structure.
// The original code accesses `param_1 + 8` for the count. This implies a 32-bit layout
// for `List` (e.g., two 4-byte pointers followed by an int).
// For standard C and portability (e.g., 64-bit systems), `list->count` will access
// the `count` member correctly, but its offset might be 16 bytes if pointers are 8 bytes.
// This implementation assumes `list->count` correctly accesses the list's count.
typedef struct List {
    // The actual head and tail nodes are accessed via `get_first_node`, `get_list_tail`, etc.
    // The original code implies `count` is a member of this structure.
    Node *head; // Placeholder for possible internal structure
    Node *tail; // Placeholder for possible internal structure
    unsigned int count; // Directly accessed as `*(uint *)(param_1 + 8)` in original code
} List;

// External function declarations (assuming these are provided elsewhere)
// The `send` function's first parameter in the original code is problematic (casting `&local_var` to `int`).
// It's corrected here to take a `const void *` pointer for data, which is standard practice.
int send(const void *data, size_t data_size, size_t stack_param, int flags);
Node *get_first_node(const List *list);
Node *get_list_tail(const List *list);
Node *get_list_head(const List *list);
Node *get_last_node(const List *list);

// Global constants derived from the original snippet's raw data addresses
const double DAT_00014000 = 2.0;    // Used as divisor for median in do_dbl_median
const double DAT_00014008 = DBL_MAX; // Used as initial minimum in do_dbl_min
const double DAT_00014010 = -DBL_MAX; // Used as initial maximum in do_dbl_max

// Helper macro for ROUND, assuming it's standard round() from math.h
#define ROUND(x) round(x)

// Function: do_int_mean
void do_int_mean(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x1100c;
  int sum = 0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    sum += *(int *)current_node->data_ptr;
    current_node = current_node->next_node;
  }

  int mean = 0;
  if (list->count != 0) {
    mean = sum / (int)list->count;
  }
  send(&mean, sizeof(mean), stack_param_val, flags);
}

// Function: do_int_median
void do_int_median(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffcc, uninitialized as in original
  int flags = 0x1109d;

  Node *current_node = get_first_node(list);
  unsigned int median_idx = list->count >> 1; // list->count / 2

  Node *tail_node = get_list_tail(list);
  while ((current_node != tail_node) && (median_idx > 0)) {
    current_node = current_node->next_node;
    median_idx--;
  }

  int median_val;
  if ((list->count & 1) == 0) { // Even number of elements
    // Median is the average of the two middle elements.
    // current_node points to the N/2-th element (0-indexed after loop).
    // The original code uses the previous node for the second value.
    int val1 = *(int *)current_node->data_ptr;
    int val2 = *(int *)current_node->prev_node->data_ptr;
    median_val = (val1 + val2) / 2;
  } else { // Odd number of elements
    // Median is the middle element.
    median_val = *(int *)current_node->data_ptr;
  }
  send(&median_val, sizeof(median_val), stack_param_val, flags);
}

// Function: do_int_mode
void do_int_mode(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffcc, uninitialized as in original
  int flags = 0x11175;
  unsigned int max_freq = 0;
  int mode_count = 0;
  unsigned int current_freq = 0;
  int current_val = 0; // Stores the value whose frequency is being counted

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  // Handle empty list case explicitly
  if (current_node == tail_node) {
    int zero_val = 0;
    send(&zero_val, sizeof(zero_val), stack_param_val, flags);
    return;
  }

  // First pass: Find max_freq and mode_count
  // Initialize with the first element's data
  current_val = *(int *)current_node->data_ptr;
  current_freq = 1;
  max_freq = 1;
  mode_count = 1; // If only one element, it's a mode with frequency 1

  current_node = current_node->next_node; // Move to the second element

  while (current_node != tail_node) {
    int node_val = *(int *)current_node->data_ptr;
    if (node_val == current_val) {
      current_freq++;
    } else {
      current_val = node_val;
      current_freq = 1;
    }

    if (max_freq < current_freq) {
      max_freq = current_freq;
      mode_count = 1; // Reset mode_count for a new highest frequency
    } else if (current_freq == max_freq) {
      mode_count++; // Increment count for modes with the same max frequency
    }
    current_node = current_node->next_node;
  }

  if (max_freq < 2) { // No mode found (all elements unique or only one element)
    int zero_val = 0;
    send(&zero_val, sizeof(zero_val), stack_param_val, flags);
  } else {
    send(&mode_count, sizeof(mode_count), stack_param_val, flags);

    // Second pass: Send the actual mode values
    current_node = get_first_node(list);
    // Re-initialize for the second pass with the first element
    current_val = *(int *)current_node->data_ptr;
    current_freq = 1;

    if (current_freq == max_freq) { // Check the first element
        send(&current_val, sizeof(current_val), stack_param_val, flags);
    }
    current_node = current_node->next_node; // Move to the second element

    while (current_node != tail_node) {
      int node_val = *(int *)current_node->data_ptr;
      if (node_val == current_val) {
        current_freq++;
      } else {
        current_val = node_val;
        current_freq = 1;
      }
      if (current_freq == max_freq) {
        send(&current_val, sizeof(current_val), stack_param_val, flags);
      }
      current_node = current_node->next_node;
    }
  }
}

// Function: do_int_range
void do_int_range(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffcc, uninitialized as in original
  int flags = 0x112d0;

  Node *first_node = get_first_node(list);
  Node *last_node = get_last_node(list);

  int range = *(int *)last_node->data_ptr - *(int *)first_node->data_ptr;
  send(&range, sizeof(range), stack_param_val, flags);
}

// Function: do_int_sum
void do_int_sum(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11347;
  int sum = 0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    sum += *(int *)current_node->data_ptr;
    current_node = current_node->next_node;
  }
  send(&sum, sizeof(sum), stack_param_val, flags);
}

// Function: do_int_product
void do_int_product(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x113bb;
  int product = 1; // Initialize product to 1

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    product *= *(int *)current_node->data_ptr;
    current_node = current_node->next_node;
  }
  send(&product, sizeof(product), stack_param_val, flags);
}

// Function: do_int_min
void do_int_min(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11430;
  int min_val = INT_MAX; // Initialize with maximum possible int value

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    int node_val = *(int *)current_node->data_ptr;
    if (node_val < min_val) {
      min_val = node_val;
    }
    current_node = current_node->next_node;
  }
  send(&min_val, sizeof(min_val), stack_param_val, flags);
}

// Function: do_int_max
void do_int_max(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x114ab;
  int max_val = INT_MIN; // Initialize with minimum possible int value

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    int node_val = *(int *)current_node->data_ptr;
    if (max_val < node_val) {
      max_val = node_val;
    }
    current_node = current_node->next_node;
  }
  send(&max_val, sizeof(max_val), stack_param_val, flags);
}

// Function: do_int_sort
void do_int_sort(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11526;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    // Corrected: send the pointer to the integer value, not the value itself as an int type.
    send(current_node->data_ptr, sizeof(int), stack_param_val, flags);
    current_node = current_node->next_node;
  }
}

// Function: do_int_rsort
void do_int_rsort(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11587;

  Node *current_node = get_last_node(list); // Start from the last node
  Node *head_node = get_list_head(list);    // Loop until we reach the head

  // The original loop condition `local_10 == piVar1` (current_node == head_node)
  // means the head node itself is not processed by the loop. This behavior is preserved.
  while (current_node != head_node) {
    // Corrected: send the pointer to the integer value.
    send(current_node->data_ptr, sizeof(int), stack_param_val, flags);
    current_node = current_node->prev_node;
  }
}

// Function: do_int_odds
void do_int_odds(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x115e8;
  int odd_count = 0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  // First pass: count odd numbers
  while (current_node != tail_node) {
    if ((*(unsigned int *)current_node->data_ptr & 1) != 0) {
      odd_count++;
    }
    current_node = current_node->next_node;
  }
  send(&odd_count, sizeof(odd_count), stack_param_val, flags);

  // Second pass: send odd numbers if any were found
  if (odd_count != 0) {
    current_node = get_first_node(list); // Reset iterator to the beginning
    while (current_node != tail_node) {
      if ((*(unsigned int *)current_node->data_ptr & 1) != 0) {
        // Corrected: send the pointer to the integer value.
        send(current_node->data_ptr, sizeof(int), stack_param_val, flags);
      }
      current_node = current_node->next_node;
    }
  }
}

// Function: do_int_evens
void do_int_evens(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x116bf;
  int even_count = 0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  // First pass: count even numbers
  while (current_node != tail_node) {
    if ((*(unsigned int *)current_node->data_ptr & 1) == 0) {
      even_count++;
    }
    current_node = current_node->next_node;
  }
  send(&even_count, sizeof(even_count), stack_param_val, flags);

  // Second pass: send even numbers if any were found
  if (even_count != 0) {
    current_node = get_first_node(list); // Reset iterator to the beginning
    while (current_node != tail_node) {
      if ((*(unsigned int *)current_node->data_ptr & 1) == 0) {
        // Corrected: send the pointer to the integer value.
        send(current_node->data_ptr, sizeof(int), stack_param_val, flags);
      }
      current_node = current_node->next_node;
    }
  }
}

// Function: do_dbl_mean
void do_dbl_mean(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffbc, uninitialized as in original
  int flags = 0x11796;
  double sum = 0.0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    sum += *(double *)current_node->data_ptr;
    current_node = current_node->next_node;
  }

  double mean = 0.0;
  if (list->count != 0) {
    mean = sum / (double)list->count;
  }
  send(&mean, sizeof(mean), stack_param_val, flags);
}

// Function: do_dbl_median
void do_dbl_median(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffbc, uninitialized as in original
  int flags = 0x11832;

  Node *current_node = get_first_node(list);
  unsigned int median_idx = list->count >> 1; // list->count / 2

  Node *tail_node = get_list_tail(list);
  while ((current_node != tail_node) && (median_idx > 0)) {
    current_node = current_node->next_node;
    median_idx--;
  }

  double median_val;
  if ((list->count & 1) == 0) { // Even number of elements
    // Median is the average of the two middle elements.
    // current_node points to the N/2-th element (0-indexed after loop).
    // The original code for doubles uses the next node for the second value.
    double val1 = *(double *)current_node->data_ptr;
    double val2 = *(double *)current_node->next_node->data_ptr;
    median_val = (val1 + val2) / DAT_00014000; // Using defined constant 2.0
  } else { // Odd number of elements
    // Median is the middle element.
    median_val = *(double *)current_node->data_ptr;
  }
  send(&median_val, sizeof(median_val), stack_param_val, flags);
}

// Function: do_dbl_mode
void do_dbl_mode(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffcc, uninitialized as in original
  int flags = 0x11901;
  unsigned int max_freq = 0;
  int mode_count = 0;
  unsigned int current_freq = 0;
  double current_val = 0.0; // Stores the value whose frequency is being counted

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  // Handle empty list case explicitly
  if (current_node == tail_node) {
    int zero_val = 0;
    send(&zero_val, sizeof(zero_val), stack_param_val, flags);
    return;
  }

  // First pass: Find max_freq and mode_count
  // Initialize with the first element's data
  current_val = *(double *)current_node->data_ptr;
  current_freq = 1;
  max_freq = 1;
  mode_count = 1;

  current_node = current_node->next_node; // Move to the second element

  while (current_node != tail_node) {
    double node_val = *(double *)current_node->data_ptr;
    if (node_val == current_val) {
      current_freq++;
    } else {
      current_val = node_val;
      current_freq = 1;
    }

    if (max_freq < current_freq) {
      max_freq = current_freq;
      mode_count = 1;
    } else if (current_freq == max_freq) {
      mode_count++;
    }
    current_node = current_node->next_node;
  }

  if (max_freq < 2) {
    int zero_val = 0;
    send(&zero_val, sizeof(zero_val), stack_param_val, flags);
  } else {
    send(&mode_count, sizeof(mode_count), stack_param_val, flags); // mode_count is an int

    // Second pass: Send the actual mode values
    current_node = get_first_node(list);
    // Re-initialize for the second pass with the first element
    current_val = *(double *)current_node->data_ptr;
    current_freq = 1;

    if (current_freq == max_freq) { // Check the first element
        send(&current_val, sizeof(current_val), stack_param_val, flags);
    }
    current_node = current_node->next_node; // Move to the second element

    while (current_node != tail_node) {
      double node_val = *(double *)current_node->data_ptr;
      if (node_val == current_val) {
        current_freq++;
      } else {
        current_val = node_val;
        current_freq = 1;
      }
      if (current_freq == max_freq) {
        send(&current_val, sizeof(current_val), stack_param_val, flags);
      }
      current_node = current_node->next_node;
    }
  }
}

// Function: do_dbl_range
void do_dbl_range(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffcc, uninitialized as in original
  int flags = 0x11a6e;

  Node *first_node = get_first_node(list);
  Node *last_node = get_last_node(list);

  double range = *(double *)last_node->data_ptr - *(double *)first_node->data_ptr;
  send(&range, sizeof(range), stack_param_val, flags);
}

// Function: do_dbl_sum
void do_dbl_sum(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11ae3;
  double sum = 0.0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    sum += *(double *)current_node->data_ptr;
    current_node = current_node->next_node;
  }
  send(&sum, sizeof(sum), stack_param_val, flags);
}

// Function: do_dbl_product
void do_dbl_product(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11b55;
  double product = 1.0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    product *= *(double *)current_node->data_ptr;
    current_node = current_node->next_node;
  }
  send(&product, sizeof(product), stack_param_val, flags);
}

// Function: do_dbl_min
void do_dbl_min(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11bc7;
  double min_val = DAT_00014008; // Initialize with DBL_MAX

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    double node_val = *(double *)current_node->data_ptr;
    if (node_val < min_val) {
      min_val = node_val;
    }
    current_node = current_node->next_node;
  }
  send(&min_val, sizeof(min_val), stack_param_val, flags);
}

// Function: do_dbl_max
void do_dbl_max(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11c46;
  double max_val = DAT_00014010; // Initialize with -DBL_MAX

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    double node_val = *(double *)current_node->data_ptr;
    if (max_val < node_val) {
      max_val = node_val;
    }
    current_node = current_node->next_node;
  }
  send(&max_val, sizeof(max_val), stack_param_val, flags);
}

// Function: do_dbl_sort
void do_dbl_sort(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11cc5;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  while (current_node != tail_node) {
    // Corrected: send the pointer to the double value.
    send(current_node->data_ptr, sizeof(double), stack_param_val, flags);
    current_node = current_node->next_node;
  }
}

// Function: do_dbl_rsort
void do_dbl_rsort(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffdc, uninitialized as in original
  int flags = 0x11d26;

  Node *current_node = get_last_node(list); // Start from the last node
  Node *head_node = get_list_head(list);    // Loop until we reach the head

  // The original loop condition `local_10 == piVar1` (current_node == head_node)
  // means the head node itself is not processed by the loop. This behavior is preserved.
  while (current_node != head_node) {
    // Corrected: send the pointer to the double value.
    send(current_node->data_ptr, sizeof(double), stack_param_val, flags);
    current_node = current_node->prev_node;
  }
}

// Function: do_dbl_odds
void do_dbl_odds(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffbc, uninitialized as in original
  int flags = 0x11d87;
  int odd_count = 0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  // First pass: count odd numbers (after rounding)
  while (current_node != tail_node) {
    unsigned int rounded_val = (unsigned int)ROUND(*(double *)current_node->data_ptr);
    if ((rounded_val & 1) != 0) {
      odd_count++;
    }
    current_node = current_node->next_node;
  }
  send(&odd_count, sizeof(odd_count), stack_param_val, flags);

  // Second pass: send odd numbers if any were found
  if (odd_count != 0) {
    current_node = get_first_node(list); // Reset iterator to the beginning
    while (current_node != tail_node) {
      unsigned int rounded_val = (unsigned int)ROUND(*(double *)current_node->data_ptr);
      if ((rounded_val & 1) != 0) {
        // Corrected: send the pointer to the double value.
        send(current_node->data_ptr, sizeof(double), stack_param_val, flags);
      }
      current_node = current_node->next_node;
    }
  }
}

// Function: do_dbl_evens
void do_dbl_evens(const List *list) {
  size_t stack_param_val; // Corresponds to in_stack_ffffffbc, uninitialized as in original
  int flags = 0x11e99;
  int even_count = 0;

  Node *current_node = get_first_node(list);
  Node *tail_node = get_list_tail(list);

  // First pass: count even numbers (after rounding)
  while (current_node != tail_node) {
    unsigned int rounded_val = (unsigned int)ROUND(*(double *)current_node->data_ptr);
    if ((rounded_val & 1) == 0) {
      even_count++;
    }
    current_node = current_node->next_node;
  }
  send(&even_count, sizeof(even_count), stack_param_val, flags);

  // Second pass: send even numbers if any were found
  if (even_count != 0) {
    current_node = get_first_node(list); // Reset iterator to the beginning
    while (current_node != tail_node) {
      unsigned int rounded_val = (unsigned int)ROUND(*(double *)current_node->data_ptr);
      if ((rounded_val & 1) == 0) {
        // Corrected: send the pointer to the double value.
        send(current_node->data_ptr, sizeof(double), stack_param_val, flags);
      }
      current_node = current_node->next_node;
    }
  }
}