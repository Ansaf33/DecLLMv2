#include <stdio.h>    // For printf
#include <stdlib.h>   // For malloc, strtol, free, perror
#include <string.h>   // For strlen, strsep
#include <stdint.h>   // For fixed-width integers like int16_t, uint8_t
#include <stdbool.h>  // For bool type

// Global variables from the snippet
unsigned int g_random_usage = 0;
unsigned char rxbuf_0[64]; // Assuming 0x40 is 64 bytes
int rxcnt_1 = 0;
int rxidx_2 = 0;

// Placeholder for the hardcoded address 0x4347c000.
// This is a critical assumption. The original code reads bytes from this address.
// We must provide a source of data for it. For a compilable example, we'll
// create a global array. In a real scenario, this would be specific data
// from the original program's memory layout.
#define INITIAL_RANDOM_DATA_SIZE (0x400 * 3) // 1024 * 3 = 3072 bytes
unsigned char g_initial_random_data[INITIAL_RANDOM_DATA_SIZE];

// Forward declarations for functions that use each other
struct Matrix; // Forward declaration
typedef struct Matrix Matrix;

// Function pointer types for matrix operations
typedef int (*MatrixResizeFunc)(Matrix *m, int new_rows, int new_cols);
typedef int (*MatrixSetValueFunc)(Matrix *m, int row, int col, int value);

// A simplified Matrix structure based on how the original code accesses members
// The original code uses `int *param_1` and accesses `param_1[0]`, `param_1[1]`, `param_1[6]`, `param_1[8]`.
// This suggests an array of integers that holds metadata and function pointers.
// We model this using a struct for type safety and clarity.
struct Matrix {
    int rows;
    int cols;
    int *data; // Placeholder for matrix data (e.g., a dynamically allocated flat array)
    MatrixSetValueFunc set_value;
    MatrixResizeFunc resize;
};

// --- Placeholder functions for external dependencies ---
// These functions are not provided in the snippet and must be implemented or linked.
// For compilation, we provide minimal stubs.

// Placeholder for `receive` function
// int receive(int fd, void *buf, int len, int *bytes_read)
int receive(int fd, void *buf, int len, int *bytes_read) {
    // Simulate receiving some data for testing
    if (len > 0) {
        ((unsigned char*)buf)[0] = 'h';
        ((unsigned char*)buf)[1] = 'e';
        ((unsigned char*)buf)[2] = 'l';
        ((unsigned char*)buf)[3] = 'l';
        ((unsigned char*)buf)[4] = 'o';
        ((unsigned char*)buf)[5] = '\n'; // Simulate newline for readline
        *bytes_read = 6;
        return 0; // Success
    }
    *bytes_read = 0;
    return 1; // Error or no data
}

// Placeholder for matrix specific set_value and resize functions
int default_matrix_set_value(Matrix *m, int row, int col, int value) {
    if (m == NULL || m->data == NULL || row < 0 || row >= m->rows || col < 0 || col >= m->cols) {
        return 1; // Error
    }
    // Assuming m->data points to a flat array:
    m->data[row * m->cols + col] = value;
    return 0;
}

int default_matrix_resize(Matrix *m, int new_rows, int new_cols) {
    if (m == NULL || new_rows < 1 || new_rows > 16 || new_cols < 1 || new_cols > 16) {
        return 1; // Error: sizes out of bounds (1-16)
    }
    
    // If data was previously allocated, reallocate or free it.
    // For this stub, we simplify.
    if (m->data != NULL && (m->rows != new_rows || m->cols != new_cols)) {
        free(m->data);
        m->data = NULL;
    }

    if (m->data == NULL) {
        m->data = (int *)malloc(new_rows * new_cols * sizeof(int));
        if (m->data == NULL) {
            perror("malloc failed for matrix data");
            return 1; // Allocation error
        }
    }
    
    m->rows = new_rows;
    m->cols = new_cols;
    return 0; // Success
}

// Placeholder for matrix creation.
// type 1: dynamic allocation, type 2: use provided buffer.
Matrix *create_matrix(int type, void *buffer) {
    Matrix *m = (Matrix *)malloc(sizeof(Matrix));
    if (!m) {
        perror("malloc failed for Matrix struct");
        return NULL;
    }
    m->rows = 0; // Initialize to 0, resize_matrix will set them
    m->cols = 0;
    m->set_value = default_matrix_set_value;
    m->resize = default_matrix_resize;

    if (type == 2 && buffer != NULL) {
        // For type 2, assume 'data' points to the provided buffer.
        // This means the buffer must be large enough to hold matrix elements.
        // The original code implies local_42c is a char array, but matrix elements are int.
        // This is a potential mismatch. For this stub, we'll just assign it
        // and assume the user ensures buffer size/type.
        m->data = (int *)buffer; // Casting char array to int array
    } else {
        m->data = NULL; // Will be allocated by resize
    }
    return m;
}

// Placeholder for matrix print function
void print_matrix(const char *title, Matrix *matrix) {
    printf("[STUB] %s: Matrix (rows=%d, cols=%d)\n", title, matrix->rows, matrix->cols);
    if (matrix->rows > 0 && matrix->cols > 0 && matrix->data != NULL) {
        printf("  Data:\n");
        for (int r = 0; r < matrix->rows; ++r) {
            printf("  ");
            for (int c = 0; c < matrix->cols; ++c) {
                printf("%4d ", matrix->data[r * matrix->cols + c]);
            }
            printf("\n");
        }
    } else {
        printf("  (Empty or uninitialized)\n");
    }
}

// Placeholder for matrix arithmetic functions
int m_add(Matrix *m1, Matrix *m2, Matrix *result) {
    if (m1 == NULL || m2 == NULL || result == NULL || m1->rows != m2->rows || m1->cols != m2->cols) {
        return 1; // Error
    }
    result->resize(result, m1->rows, m1->cols);
    // Simulate addition
    for (int r = 0; r < m1->rows; ++r) {
        for (int c = 0; c < m1->cols; ++c) {
            // Assuming a get_value function or direct data access for m1, m2
            result->set_value(result, r, c, m1->data[r*m1->cols+c] + m2->data[r*m2->cols+c]);
        }
    }
    return 0; // Success
}

int m_subtract(Matrix *m1, Matrix *m2, Matrix *result) {
    if (m1 == NULL || m2 == NULL || result == NULL || m1->rows != m2->rows || m1->cols != m2->cols) {
        return 1; // Error
    }
    result->resize(result, m1->rows, m1->cols);
    // Simulate subtraction
    for (int r = 0; r < m1->rows; ++r) {
        for (int c = 0; c < m1->cols; ++c) {
            result->set_value(result, r, c, m1->data[r*m1->cols+c] - m2->data[r*m2->cols+c]);
        }
    }
    return 0; // Success
}

int m_multiply(Matrix *m1, Matrix *m2, Matrix *result) {
    if (m1 == NULL || m2 == NULL || result == NULL || m1->cols != m2->rows) {
        return 1; // Error
    }
    result->resize(result, m1->rows, m2->cols);
    // Simulate multiplication
    for (int r = 0; r < m1->rows; ++r) {
        for (int c = 0; c < m2->cols; ++c) {
            int sum = 0;
            for (int k = 0; k < m1->cols; ++k) {
                sum += m1->data[r * m1->cols + k] * m2->data[k * m2->cols + c];
            }
            result->set_value(result, r, c, sum);
        }
    }
    return 0; // Success
}

int swap_row(Matrix *m, int r1, int r2) {
    if (m == NULL || r1 < 0 || r1 >= m->rows || r2 < 0 || r2 >= m->rows) return 1; // Error
    // Simulate row swap
    int *temp_row = (int *)malloc(m->cols * sizeof(int));
    if (!temp_row) return 1;
    memcpy(temp_row, &m->data[r1 * m->cols], m->cols * sizeof(int));
    memcpy(&m->data[r1 * m->cols], &m->data[r2 * m->cols], m->cols * sizeof(int));
    memcpy(&m->data[r2 * m->cols], temp_row, m->cols * sizeof(int));
    free(temp_row);
    return 0; // Success
}

int swap_col(Matrix *m, int c1, int c2) {
    if (m == NULL || c1 < 0 || c1 >= m->cols || c2 < 0 || c2 >= m->cols) return 1; // Error
    // Simulate column swap
    for (int r = 0; r < m->rows; ++r) {
        int temp = m->data[r * m->cols + c1];
        m->data[r * m->cols + c1] = m->data[r * m->cols + c2];
        m->data[r * m->cols + c2] = temp;
    }
    return 0; // Success
}

void m_transpose(Matrix *m) {
    if (m == NULL) return;
    // For in-place transpose, requires square matrix or new allocation.
    // For stub, just swap dimensions.
    int temp_rows = m->rows;
    m->rows = m->cols;
    m->cols = temp_rows;
}

int m_rref(Matrix *m_input, Matrix *m_output) {
    if (m_input == NULL || m_output == NULL) return 1;
    m_output->resize(m_output, m_input->rows, m_input->cols);
    // Simulate RREF by copying input to output
    for (int r = 0; r < m_input->rows; ++r) {
        for (int c = 0; c < m_input->cols; ++c) {
            m_output->set_value(m_output, r, c, m_input->data[r * m_input->cols + c]);
        }
    }
    return 0;
}

// Function: create_random_shorts
short *create_random_shorts(void) {
  short *random_shorts_array = (short *)malloc(0x800); // 0x800 bytes = 0x400 shorts
  if (random_shorts_array == NULL) {
    perror("malloc failed for random_shorts_array");
    return NULL;
  }
  
  for (unsigned int i = 0; i < 0x400; ++i) {
    // This line accesses a hardcoded memory address (0x4347c000)
    // and reads a byte from it, then casts to short.
    // We replace 0x4347c000 with our global array g_initial_random_data.
    // The access pattern `i * 3` suggests it reads a byte every 3 positions.
    if ((i * 3) < INITIAL_RANDOM_DATA_SIZE) {
        random_shorts_array[i] = (short)g_initial_random_data[i * 3];
    } else {
        random_shorts_array[i] = 0; // Default or error value if source data is too small
    }
  }
  return random_shorts_array;
}

// Function: get_rand_short
short get_rand_short(short *random_shorts_array) {
  if (0x3ff < g_random_usage) { // 0x3ff is 1023, so 1024 shorts total (0 to 1023)
    g_random_usage = 0;
  }
  return random_shorts_array[g_random_usage++];
}

// Function: get_byte
int get_byte(void) {
  int bytes_received;
  
  if (rxidx_2 == rxcnt_1) {
    if (receive(0, rxbuf_0, sizeof(rxbuf_0), &bytes_received) != 0 || bytes_received == 0) {
      return -1; // Using -1 for 0xffffffff error
    }
    rxcnt_1 = bytes_received;
    rxidx_2 = 0;
  }
  return (int)rxbuf_0[rxidx_2++];
}

// Function: readline
int readline(char *buffer, unsigned int max_len) {
  if (buffer == NULL || max_len < 2) {
    return -1; // Error: invalid buffer or too small
  }
  
  unsigned int i;
  for (i = 0; i < max_len; ++i) {
    int byte_val = get_byte();
    if (byte_val == -1) {
      return 1; // Error: get_byte failed
    }
    buffer[i] = (char)byte_val;
    if (buffer[i] == '\n') {
      buffer[i] = '\0'; // Null-terminate the string
      break;
    }
  }
  
  // If loop finished because i reached max_len or if no characters were read (i == 0)
  if (i == max_len || i == 0) {
    return -1; // Error: buffer full or empty line
  }
  return 0; // Success
}

// Function: readnum
int readnum(char *buffer, unsigned int buffer_len, long *output_num) {
  int result = readline(buffer, buffer_len);
  if (result == 0) {
    *output_num = strtol(buffer, NULL, 10);
  } else {
    *output_num = 0; // Default value on error
  }
  return result;
}

// Function: select_menu_choice
int select_menu_choice(char *input_buffer, unsigned int buffer_len) {
  long choice_val = 0; // Using long as readnum expects long*
  printf(
        "Make a selection: \n1. Input Matrix\n2. Print Matrix\n3. Add Matrices\n4. Subtract Matrices\n5. Multiply Matrices\n6. Swap Matrix Row\n7. Swap Matrix Col\n8. Transpose Matrix\n9. Perform Reduced Row Echelon Form on Matrix\n10. Randomize Matrix\n11. Exit\n>> "
        );
  int read_result = readnum(input_buffer, buffer_len, &choice_val);
  if (read_result == 1) { // Original code returned 1 for readline error
    return 11; // Map to exit choice in case of read error
  }
  return (int)choice_val;
}

// Function: choose_matrix
Matrix *choose_matrix(Matrix *matrix1, Matrix *matrix2, char *input_buffer, unsigned int buffer_len) {
  long selection_val = 0;
  printf("Select Matrix 1 or Matrix 2\n>> ");
  int read_result = readnum(input_buffer, buffer_len, &selection_val);
  if (read_result == 1) {
    return NULL; // Error
  } else if (selection_val == 2) {
    return matrix2;
  }
  // Default to matrix1 or if selection_val is 1
  return matrix1;
}

// Function: resize_matrix
int resize_matrix(Matrix *matrix, char *input_buffer, unsigned int buffer_len) {
  long new_row_size = 0;
  long new_col_size = 0;
  
  while(true) {
    printf("New Row Size (0 to stay the same): ");
    if (readnum(input_buffer, buffer_len, &new_row_size) == 1) {
      return 1; // Error from readnum
    }
    if (new_row_size == 0) {
      new_row_size = matrix->rows;
    }

    printf("New Column Size (0 to stay the same): ");
    if (readnum(input_buffer, buffer_len, &new_col_size) == 1) {
        return 1; // Error from readnum
    }
    if (new_col_size == 0) {
      new_col_size = matrix->cols;
    }
    
    // Call the matrix's resize function
    if (matrix->resize(matrix, (int)new_row_size, (int)new_col_size) == 0) {
      return 0; // Success
    }
    printf("Row and Column Sizes must be between 1-16\n");
  }
}

// Function: input_matrix
int input_matrix(Matrix *matrix, char *input_buffer, unsigned int buffer_len) {
  int resize_result = resize_matrix(matrix, input_buffer, buffer_len);
  if (resize_result == 1) {
    return 1; // Error from resize
  }
  
  int readline_result = readline(input_buffer, buffer_len);
  if (readline_result == 1 || readline_result == -1) {
    printf("Bad Input\n");
    return -1; // Error
  }
  
  int expected_elements = matrix->rows * matrix->cols;
  int element_count = 1; // Start with 1 for the first number before any space
  for (unsigned int i = 0; i < strlen(input_buffer); ++i) {
    if (input_buffer[i] == ' ') {
      element_count++;
    }
  }
  
  if (element_count != expected_elements) {
    printf("Bad Input\n");
    return -1; // Error
  }
  
  // Parse elements and set matrix values
  char *str_ptr = input_buffer; // strsep modifies the pointer
  for (int row = 0; row < matrix->rows; ++row) {
    for (int col = 0; col < matrix->cols; ++col) {
      char *token = strsep(&str_ptr, " ");
      if (token == NULL) {
        return 1; // Not enough tokens
      }
      long value = strtol(token, NULL, 10);
      matrix->set_value(matrix, row, col, (int)value);
    }
  }
  return 0; // Success
}

// Function: random_matrix
int random_matrix(Matrix *matrix, char *input_buffer, unsigned int buffer_len, short *random_shorts_ptr) {
  // The original call passes 0x11565 which is not a buffer_len, it's garbage.
  // Assuming it should be 0x800, like other calls to resize_matrix.
  int resize_result = resize_matrix(matrix, input_buffer, buffer_len);
  if (resize_result == 1) {
    return 1; // Error from resize
  }
  
  for (int row = 0; row < matrix->rows; ++row) {
    for (int col = 0; col < matrix->cols; ++col) {
      short rand_val = get_rand_short(random_shorts_ptr);
      matrix->set_value(matrix, row, col, (int)rand_val);
    }
  }
  return 0; // Success
}

// Function: print_matrices
void print_matrices(Matrix *matrix1, Matrix *matrix2, Matrix *result_matrix) {
  print_matrix("-Matrix 1-", matrix1);
  printf("\n");
  print_matrix("-Matrix 2-", matrix2);
  printf("\n");
  print_matrix("-Resultant Matrix-", result_matrix);
}

// Function: add_matrices
void add_matrices(Matrix *matrix1, Matrix *matrix2, Matrix *result_matrix) {
  if (m_add(matrix1, matrix2, result_matrix) == 0) {
    print_matrix("Result:", result_matrix);
  } else {
    printf("Could not add matrices together. Check sizes\n");
  }
}

// Function: subtract_matrices
void subtract_matrices(Matrix *matrix1, Matrix *matrix2, Matrix *result_matrix) {
  if (m_subtract(matrix1, matrix2, result_matrix) == 0) {
    print_matrix("Result:", result_matrix);
  } else {
    printf("Could not subtract matrices. Check sizes\n");
  }
}

// Function: multiply_matrices
void multiply_matrices(Matrix *matrix1, Matrix *matrix2, Matrix *result_matrix) {
  if (m_multiply(matrix1, matrix2, result_matrix) == 0) {
    print_matrix("Result:", result_matrix);
  } else {
    printf("Could not multiply matrices together. Check sizes\n");
  }
}

// Function: swap_matrix_row_col
int swap_matrix_row_col(Matrix *matrix, int type, char *input_buffer, unsigned int buffer_len) {
  long idx1 = 0;
  long idx2 = 0;
  const char *axis_name;
  int max_idx;
  
  if (type == 1) { // Row
    axis_name = "Row";
    max_idx = matrix->rows;
  } else if (type == 2) { // Column
    axis_name = "Column";
    max_idx = matrix->cols;
  } else {
    return -1; // Invalid type
  }
  
  printf("Enter %s Index 1: ", axis_name);
  if (readnum(input_buffer, buffer_len, &idx1) == 1) {
    return 1; // Error from readnum
  }
  
  if (idx1 < 0 || idx1 >= max_idx) { // Check bounds
    printf("Bad Input\n");
    return -1;
  }
  
  printf("Enter %s Index 2: ", axis_name);
  if (readnum(input_buffer, buffer_len, &idx2) == 1) {
    return 1; // Error from readnum
  }
  
  if (idx2 < 0 || idx2 >= max_idx) { // Check bounds
    printf("Bad Input\n");
    return -1;
  }
  
  print_matrix("Original Matrix", matrix);
  int swap_result;
  if (type == 1) {
    swap_result = swap_row(matrix, (int)idx1, (int)idx2);
  } else { // type == 2
    swap_result = swap_col(matrix, (int)idx1, (int)idx2);
  }
  
  return swap_result;
}

// Function: transpose_matrix
void transpose_matrix(Matrix *matrix) {
  print_matrix("Original Matrix", matrix);
  m_transpose(matrix);
  print_matrix("Transposed Matrix", matrix);
}

// Function: rref_matrix
void rref_matrix(Matrix *input_matrix, Matrix *result_matrix) {
  if (m_rref(input_matrix, result_matrix) == 0) {
    print_matrix("RREF Result:", result_matrix);
  } else {
    printf("Could not complete reduced row echelon form\n");
  }
}

// Function: main
int main(void) {
  // Initialize g_initial_random_data for testing purposes
  for (int i = 0; i < INITIAL_RANDOM_DATA_SIZE; ++i) {
      g_initial_random_data[i] = (unsigned char)(i % 256);
  }

  // local_42c is a char array, but create_matrix(2, ...) uses it for matrix data.
  // If matrix elements are ints, this buffer might be too small or misaligned.
  // For safety, align to int and size accordingly, assuming 1020 bytes -> 255 ints.
  // Max matrix size 16x16 = 256 ints. Let's make it 16x16 just in case.
  int result_matrix_data_buffer[16 * 16]; // Buffer for create_matrix(2, ...)
  char input_buffer[0x800]; // 2048 bytes for user input
  unsigned int input_buffer_len = sizeof(input_buffer);

  // Initialize matrix pointers and random shorts
  short *random_shorts_ptr = create_random_shorts();
  if (random_shorts_ptr == NULL) {
      fprintf(stderr, "Failed to create random shorts array. Exiting.\n");
      return 1;
  }

  Matrix *matrix1 = create_matrix(1, NULL); // Dynamically allocated data
  Matrix *matrix2 = create_matrix(1, NULL); // Dynamically allocated data
  Matrix *result_matrix = create_matrix(2, result_matrix_data_buffer); // Uses provided buffer

  if (matrix1 == NULL || matrix2 == NULL || result_matrix == NULL) {
      fprintf(stderr, "Failed to create matrices. Exiting.\n");
      free(random_shorts_ptr);
      free(matrix1); // Free only if not NULL
      free(matrix2);
      free(result_matrix);
      return 1;
  }
  
  // Set initial sizes for matrix1 and matrix2 for operations to work
  // (e.g., if they are added before being input/randomized)
  matrix1->resize(matrix1, 2, 2);
  matrix2->resize(matrix2, 2, 2);


  printf("Matrix math is fun!\n");
  printf("-------------------\n");

  bool exit_program = false;
  while (!exit_program) {
    int choice = select_menu_choice(input_buffer, input_buffer_len);
    Matrix *selected_matrix = NULL; // Used for operations that need a single matrix
    int op_result = 0; // Generic result for operations

    switch(choice) {
    case 1: // Input Matrix
      printf("Inputting Matrix Values:\n");
      selected_matrix = choose_matrix(matrix1, matrix2, input_buffer, input_buffer_len);
      if (selected_matrix == NULL) {
          printf("Invalid matrix selection.\n");
          break; // Continue loop
      }
      op_result = input_matrix(selected_matrix, input_buffer, input_buffer_len);
      if (op_result == 1) { // Input matrix error (from readnum) means exit in original logic
          exit_program = true;
      }
      break;
    case 2: // Print Matrix
      printf("Print Matrices:\n");
      print_matrices(matrix1, matrix2, result_matrix);
      break;
    case 3: // Add Matrices
      printf("Adding Matrices:\n");
      add_matrices(matrix1, matrix2, result_matrix);
      break;
    case 4: // Subtract Matrices
      printf("Subtracting Matrices:\n");
      subtract_matrices(matrix1, matrix2, result_matrix);
      break;
    case 5: // Multiply Matrices
      printf("Multiplying Matrices:\n");
      multiply_matrices(matrix1, matrix2, result_matrix);
      break;
    case 6: // Swap Matrix Row
      printf("Swap Rows in a Matrix:\n");
      selected_matrix = choose_matrix(matrix1, matrix2, input_buffer, input_buffer_len);
      if (selected_matrix == NULL) {
          printf("Invalid matrix selection.\n");
          break;
      }
      op_result = swap_matrix_row_col(selected_matrix, 1, input_buffer, input_buffer_len);
      if (op_result == 1) { // Error from readnum in swap_matrix_row_col
          exit_program = true;
      } else if (op_result == 0) {
          print_matrix("Swapped Rows", selected_matrix);
      }
      break;
    case 7: // Swap Matrix Col
      printf("Swap Columns in a Matrix:\n");
      selected_matrix = choose_matrix(matrix1, matrix2, input_buffer, input_buffer_len);
      if (selected_matrix == NULL) {
          printf("Invalid matrix selection.\n");
          break;
      }
      op_result = swap_matrix_row_col(selected_matrix, 2, input_buffer, input_buffer_len);
      if (op_result == 1) { // Error from readnum in swap_matrix_row_col
          exit_program = true;
      } else if (op_result == 0) {
          print_matrix("Swapped Columns", selected_matrix);
      }
      break;
    case 8: // Transpose Matrix
      printf("Transpose a Matrix:\n");
      selected_matrix = choose_matrix(matrix1, matrix2, input_buffer, input_buffer_len);
      if (selected_matrix == NULL) {
          printf("Invalid matrix selection.\n");
          break;
      }
      transpose_matrix(selected_matrix);
      break;
    case 9: // Perform Reduced Row Echelon Form on Matrix
      printf("Perform Reduced Row Echelon Form on Matrix\n");
      selected_matrix = choose_matrix(matrix1, matrix2, input_buffer, input_buffer_len);
      if (selected_matrix == NULL) {
          printf("Invalid matrix selection.\n");
          break;
      }
      rref_matrix(selected_matrix, result_matrix);
      break;
    case 10: // Randomize Matrix
      printf("Create a Random Matrix:\n");
      selected_matrix = choose_matrix(matrix1, matrix2, input_buffer, input_buffer_len);
      if (selected_matrix == NULL) {
          printf("Invalid matrix selection.\n");
          break;
      }
      op_result = random_matrix(selected_matrix, input_buffer, input_buffer_len, random_shorts_ptr);
      if (op_result == 1) { // Random matrix error (from resize_matrix) means exit in original logic
          exit_program = true;
      }
      break;
    case 11: // Exit
      exit_program = true;
      break;
    default:
      printf("Bad Selection\n");
      break;
    }
  }

  printf("Exiting...\n");

  // Cleanup allocated memory
  free(random_shorts_ptr);
  // Free matrix structs and their internal data if they allocated it
  if (matrix1) {
      if (matrix1->data) free(matrix1->data);
      free(matrix1);
  }
  if (matrix2) {
      if (matrix2->data) free(matrix2->data);
      free(matrix2);
  }
  if (result_matrix) {
      // result_matrix->data points to result_matrix_data_buffer, which is stack allocated, so don't free it.
      free(result_matrix);
  }

  return 0;
}