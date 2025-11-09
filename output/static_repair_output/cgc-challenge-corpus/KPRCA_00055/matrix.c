#include <stdio.h>    // For printf, sprintf, fprintf
#include <stdlib.h>   // For calloc, free
#include <string.h>   // For strlen, memcpy
#include <math.h>     // For roundf, fabsf, truncf
#include <stdint.h>   // For uint32_t, etc. (using unsigned int for simplicity as per original uint)

// Define standard C types for decompiled types
// undefined4 -> int (assuming signed for 0xffffffff which is -1)
// uint -> unsigned int
// undefined -> void (for generic pointers)

// --- Global Constants and Variables ---

// Global buffer for printing matrices.
// The original code checks against 0x1c18 (7192), so a larger buffer is needed.
#define MATRIX_PRINT_BUFFER_SIZE 8192
static char buf_0[MATRIX_PRINT_BUFFER_SIZE];

// Dummy transmit function for compilation.
// In a real system, this would send data over a network or file descriptor.
int transmit(int fd, const void *buf, size_t count, int *bytes_written) {
    if (count > 0) {
        // For demonstration, print to stderr. In a real system, use write().
        *bytes_written = fprintf(stderr, "TRANSMIT (fd %d): %.*s\n", fd, (int)count, (char*)buf);
        return 0; // Success
    }
    *bytes_written = 0;
    return 0; // Success
}

// Dummy pcRam00000020 function.
// This appears to be a placeholder for a specific system or matrix-related setup function.
// It's called in m_transpose with various integer arguments.
void pcRam00000020(int a, int b, int c, int d) {
    // This function is ignored for compilation purposes as its exact behavior is unknown.
    // fprintf(stderr, "pcRam00000020 called with %d, %d, %d, %d\n", a, b, c, d);
}

// Constants from decompiled code, interpreted for floating-point arithmetic.
// These are often specific memory addresses in decompiled output.
static const float DAT_000174b0 = 10000.0f; // Multiplier for fractional part (e.g., to get 4 decimal places)
static const float DAT_000174b4 = 10000.0f; // For rounding check (e.g., if fractional part is 0.9999, round up)
static const float DAT_000174b8 = 0.0001f;  // Threshold to consider a float value as non-zero for printing
static const float EPSILON = 1e-6f;        // Small value for float comparisons (e.g., checking for zero)
static const float _DAT_000174c8 = EPSILON; // Positive epsilon for range checks
static const float DAT_000174c0 = -EPSILON; // Negative epsilon for range checks

// ROUND macro/function, using standard C99 roundf for floats.
#define ROUND roundf

// Matrix structure definition
typedef struct Matrix Matrix; // Forward declaration for function pointers

struct Matrix {
    unsigned int rows;
    unsigned int cols;
    unsigned int type; // 1: short, 2: int, 4: float
    void *data;        // Pointer to the actual matrix data
    // Function pointers (vtable-like) for matrix operations
    int (*get_cell)(Matrix*, unsigned int, unsigned int);
    float (*get_fcell)(Matrix*, unsigned int, unsigned int);
    int (*set_cell)(Matrix*, unsigned int, unsigned int, int);
    int (*set_fcell)(Matrix*, unsigned int, unsigned int, float);
    int (*set_rows_cols)(Matrix*, unsigned int, unsigned int);
    void (*print_matrix)(Matrix*);
    int owns_data;     // Flag: 1 if the matrix allocated its data, 0 if using external data
};

// --- Function Prototypes ---
// (To resolve potential circular dependencies and allow any order of definition)
int m_get_cell(Matrix *m, unsigned int row, unsigned int col);
float m_get_fcell(Matrix *m, unsigned int row, unsigned int col);
int m_set_cell(Matrix *m, unsigned int row, unsigned int col, int value);
int m_set_fcell(Matrix *m, unsigned int row, unsigned int col, float value);
int m_set_rows_cols(Matrix *m, unsigned int rows, unsigned int cols);
void print_matrix(Matrix *m); // The actual matrix print function (second one from snippet)
Matrix *create_matrix(unsigned int type, void *initial_data_ptr);
int m_add(Matrix *A, Matrix *B, Matrix *C);
int m_subtract(Matrix *A, Matrix *B, Matrix *C);
int m_multiply(Matrix *A, Matrix *B, Matrix *C);
int swap_row(Matrix *m, unsigned int row1, unsigned int row2);
int swap_col(Matrix *m, unsigned int col1, unsigned int col2);
int m_transpose(Matrix *m);
int swap_nonzero_cell(Matrix *m, unsigned int row, unsigned int col);
int subtract_row(Matrix *m, unsigned int src_row, unsigned int dest_row, unsigned int start_col, float factor);
int m_rref(Matrix *src_m, Matrix *dest_m);

// --- Function Implementations ---

// Function: m_get_cell
// Retrieves an integer value from a matrix cell.
// Returns 1 on NULL matrix, 0xffffffff (-1) on out-of-bounds, or the cell value.
int m_get_cell(Matrix *m, unsigned int row, unsigned int col) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    if (row >= m->rows || col >= m->cols) {
        return -1; // Error: Out of bounds
    }

    unsigned int data_offset_idx = col + m->cols * row;
    void *cell_ptr;

    switch (m->type) {
        case 1: // short
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(short);
            return (int)*(short*)cell_ptr;
        case 2: // int
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(int);
            return *(int*)cell_ptr;
        case 4: // float
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(float);
            return (int)ROUND(*(float*)cell_ptr);
        default:
            return -1; // Unknown type
    }
}

// Function: m_get_fcell
// Retrieves a float value from a matrix cell.
// Returns 0.0f on NULL matrix or out-of-bounds.
float m_get_fcell(Matrix *m, unsigned int row, unsigned int col) {
    if (m == NULL || row >= m->rows || col >= m->cols) {
        return 0.0f; // Error or out of bounds, returning 0.0f as default
    }

    unsigned int data_offset_idx = col + m->cols * row;
    void *cell_ptr;

    switch (m->type) {
        case 1: // short
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(short);
            return (float)*(short*)cell_ptr;
        case 2: // int
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(int);
            return (float)*(int*)cell_ptr;
        case 4: // float
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(float);
            return *(float*)cell_ptr;
        default:
            return 0.0f; // Unknown type
    }
}

// Function: m_set_cell
// Sets an integer value in a matrix cell.
// Returns 1 on NULL matrix, 0xffffffff (-1) on out-of-bounds, 0 on success.
int m_set_cell(Matrix *m, unsigned int row, unsigned int col, int value) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    if (row >= m->rows || col >= m->cols) {
        return -1; // Error: Out of bounds
    }

    unsigned int data_offset_idx = col + m->cols * row;
    void *cell_ptr;

    switch (m->type) {
        case 1: // short
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(short);
            *(short*)cell_ptr = (short)value;
            break;
        case 2: // int
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(int);
            *(int*)cell_ptr = value;
            break;
        case 4: // float
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(float);
            *(float*)cell_ptr = (float)value;
            break;
        default:
            return -1; // Unknown type
    }
    return 0; // Success
}

// Function: m_set_fcell
// Sets a float value in a matrix cell.
// Returns 1 on NULL matrix, 0xffffffff (-1) on out-of-bounds, 0 on success.
int m_set_fcell(Matrix *m, unsigned int row, unsigned int col, float value) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    if (row >= m->rows || col >= m->cols) {
        return -1; // Error: Out of bounds
    }

    unsigned int data_offset_idx = col + m->cols * row;
    void *cell_ptr;

    switch (m->type) {
        case 1: // short
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(short);
            *(short*)cell_ptr = (short)ROUND(value);
            break;
        case 2: // int
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(int);
            *(int*)cell_ptr = (int)ROUND(value);
            break;
        case 4: // float
            cell_ptr = (char*)m->data + data_offset_idx * sizeof(float);
            *(float*)cell_ptr = value;
            break;
        default:
            return -1; // Unknown type
    }
    return 0; // Success
}

// Function: m_set_rows_cols
// Sets the dimensions (rows and columns) of a matrix.
// Returns 1 on NULL matrix, 0xffffffff (-1) if dimensions exceed 16, 0 on success.
int m_set_rows_cols(Matrix *m, unsigned int rows, unsigned int cols) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    // Original code used 0x11 (17) as a limit. Assuming max 16x16 matrix (0-15).
    if (rows >= 17 || cols >= 17) {
        return -1; // Error: Dimensions too large
    }
    m->rows = rows;
    m->cols = cols;
    return 0; // Success
}

// Function: transmitall
// Transmits a buffer of data using the 'transmit' function until all data is sent
// or an error occurs.
void transmitall(char *buffer, unsigned int size) {
    unsigned int total_sent = 0;
    while (total_sent < size) {
        int sent_this_call;
        int res = transmit(1, buffer + total_sent, size - total_sent, &sent_this_call);
        if (res != 0) { // Error in transmit
            break;
        }
        if (sent_this_call == 0) { // No bytes sent, avoid infinite loop
            break;
        }
        total_sent += sent_this_call;
    }
}

// Function: print_matrix (the actual matrix printer, assigned to Matrix->print_matrix)
// Prints the content of a matrix to the global buffer 'buf_0' and transmits it.
void print_matrix(Matrix *m) {
    if (m == NULL) {
        return;
    }

    buf_0[0] = '\0'; // Clear buffer

    for (unsigned int row = 0; row < m->rows; row++) {
        strcat(buf_0, "|"); // Start row separator
        for (unsigned int col = 0; col < m->cols; col++) {
            size_t current_len = strlen(buf_0);
            if (current_len > 7192) { // Check buffer overflow (0x1c18 from original)
                transmitall(buf_0, current_len);
                buf_0[0] = '\0'; // Reset buffer after transmission
            }

            if (m->type == 4) { // Float matrix
                float cell_val = m->get_fcell(m, row, col);
                float abs_cell_val = fabsf(cell_val);

                // Original logic for fixed-point float display
                if (abs_cell_val >= DAT_000174b8) { // If value is significant
                    int int_part = (int)truncf(abs_cell_val);
                    float fractional_part = abs_cell_val - (float)int_part;
                    int dec_part = (int)ROUND(DAT_000174b0 * fractional_part);

                    // Special handling for rounding near 0.9999 or 0.0001
                    // DAT_000174b4 is likely 10000.0f
                    if ((int)ROUND(DAT_000174b4 * fractional_part) == 9999) {
                        int_part++;
                        dec_part = 0;
                    } else if ((int)ROUND(DAT_000174b4 * fractional_part) == 1) {
                        dec_part = 0;
                    }

                    if (cell_val < 0.0f) { // Check for negative value
                        sprintf(buf_0, "%s -%d.%d |", buf_0, int_part, dec_part);
                    } else {
                        sprintf(buf_0, "%s %d.%d |", buf_0, int_part, dec_part);
                    }
                } else {
                    sprintf(buf_0, "%s 0.0 |", buf_0); // Value is effectively zero
                }
            } else { // Integer/short matrix
                int cell_val = m->get_cell(m, row, col);
                sprintf(buf_0, "%s %d |", buf_0, cell_val);
            }
        }
        sprintf(buf_0, "%s\n", buf_0); // End of row
    }

    // Transmit any remaining content in the buffer
    size_t final_len = strlen(buf_0);
    if (final_len > 0) {
        transmitall(buf_0, final_len);
        buf_0[0] = '\0'; // Clear buffer
    }
}

// Function: create_matrix
// Allocates and initializes a new matrix.
// 'type': 1 for short, 2 for int, 4 for float.
// 'initial_data_ptr': If NULL, data buffer is allocated internally. Otherwise, uses provided pointer.
// Returns a pointer to the new Matrix struct, or NULL on allocation failure.
Matrix *create_matrix(unsigned int type, void *initial_data_ptr) {
    Matrix *m = (Matrix*)calloc(1, sizeof(Matrix));
    if (m == NULL) {
        return NULL;
    }

    size_t data_element_size = 0;
    switch (type) {
        case 1: data_element_size = sizeof(short); break;
        case 2: data_element_size = sizeof(int); break;
        case 4: data_element_size = sizeof(float); break;
        default:
            free(m);
            return NULL; // Invalid type
    }
    // Default matrix size 1x1, max 16x16. Max data size for 16x16 would be 16*16*sizeof(float) = 256*4 = 1024 bytes.
    // The original code uses 0x400 (1024) for float/int and 0x200 (512) for short, for a 16x16 matrix.
    // This implies a default maximum capacity, not initial allocation for 1x1.
    // Let's assume create_matrix allocates for a 16x16 matrix if initial_data_ptr is NULL.
    size_t max_data_size = 16 * 16 * data_element_size;

    if (initial_data_ptr == NULL) {
        m->data = calloc(1, max_data_size);
        if (m->data == NULL) {
            free(m);
            return NULL;
        }
        m->owns_data = 1; // Matrix owns its data buffer
    } else {
        m->data = initial_data_ptr;
        m->owns_data = 0; // Matrix does not own the data buffer
    }

    m->rows = 1;
    m->cols = 1;
    m->type = type;

    // Assign function pointers (vtable-like setup)
    m->get_cell = m_get_cell;
    m->get_fcell = m_get_fcell;
    m->set_cell = m_set_cell;
    m->set_fcell = m_set_fcell;
    m->set_rows_cols = m_set_rows_cols;
    m->print_matrix = print_matrix;

    return m;
}

// Function: m_add
// Adds two matrices A and B, storing the result in matrix C.
// Returns 1 on NULL input, 0xffffffff (-1) if dimensions don't match, 0 on success.
int m_add(Matrix *A, Matrix *B, Matrix *C) {
    if (A == NULL || B == NULL || C == NULL) {
        return 1; // Error: NULL matrix
    }
    if (A->rows != B->rows || A->cols != B->cols) {
        return -1; // Error: Dimensions mismatch
    }

    C->type = 2; // Result matrix type is int (original used 1 for short, but addition can overflow short)
    C->set_rows_cols(C, A->rows, A->cols);

    for (unsigned int row = 0; row < C->rows; row++) {
        for (unsigned int col = 0; col < C->cols; col++) {
            int val_A = A->get_cell(A, row, col);
            int val_B = B->get_cell(B, row, col);
            C->set_cell(C, row, col, val_A + val_B);
        }
    }
    return 0; // Success
}

// Function: m_subtract
// Subtracts matrix B from matrix A, storing the result in matrix C.
// Returns 1 on NULL input, 0xffffffff (-1) if dimensions don't match, 0 on success.
int m_subtract(Matrix *A, Matrix *B, Matrix *C) {
    if (A == NULL || B == NULL || C == NULL) {
        return 1; // Error: NULL matrix
    }
    if (A->rows != B->rows || A->cols != B->cols) {
        return -1; // Error: Dimensions mismatch
    }

    C->type = 2; // Result matrix type is int (original used 1 for short)
    C->set_rows_cols(C, A->rows, A->cols);

    for (unsigned int row = 0; row < C->rows; row++) {
        for (unsigned int col = 0; col < C->cols; col++) {
            int val_A = A->get_cell(A, row, col);
            int val_B = B->get_cell(B, row, col);
            C->set_cell(C, row, col, val_A - val_B);
        }
    }
    return 0; // Success
}

// Function: m_multiply
// Multiplies matrix A by matrix B, storing the result in matrix C.
// Returns 1 on NULL input, 0xffffffff (-1) if dimensions are incompatible, 0 on success.
int m_multiply(Matrix *A, Matrix *B, Matrix *C) {
    if (A == NULL || B == NULL || C == NULL) {
        return 1; // Error: NULL matrix
    }
    if (A->cols != B->rows) {
        return -1; // Error: Incompatible dimensions for multiplication
    }

    C->type = 2; // Result matrix type is int
    C->set_rows_cols(C, A->rows, B->cols);

    for (unsigned int row_A = 0; row_A < A->rows; row_A++) {
        for (unsigned int col_B = 0; col_B < B->cols; col_B++) {
            int sum = 0;
            for (unsigned int k = 0; k < A->cols; k++) { // Iterate through A's columns / B's rows
                int val_A = A->get_cell(A, row_A, k);
                int val_B = B->get_cell(B, k, col_B);
                sum += val_A * val_B;
            }
            C->set_cell(C, row_A, col_B, sum);
        }
    }
    return 0; // Success
}

// Function: swap_row
// Swaps two rows (row1 and row2) in a matrix.
// Returns 1 on NULL matrix, 0xffffffff (-1) on out-of-bounds, 0 on success.
int swap_row(Matrix *m, unsigned int row1, unsigned int row2) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    if (row1 >= m->rows || row2 >= m->rows) {
        return -1; // Error: Out of bounds
    }

    for (unsigned int col = 0; col < m->cols; col++) {
        float val1 = m->get_fcell(m, row1, col);
        float val2 = m->get_fcell(m, row2, col);
        m->set_fcell(m, row1, col, val2);
        m->set_fcell(m, row2, col, val1);
    }
    return 0; // Success
}

// Function: swap_col
// Swaps two columns (col1 and col2) in a matrix.
// Returns 1 on NULL matrix, 0xffffffff (-1) on out-of-bounds, 0 on success.
int swap_col(Matrix *m, unsigned int col1, unsigned int col2) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    if (col1 >= m->cols || col2 >= m->cols) {
        return -1; // Error: Out of bounds
    }

    for (unsigned int row = 0; row < m->rows; row++) {
        int val1 = m->get_cell(m, row, col1);
        int val2 = m->get_cell(m, row, col2);
        m->set_cell(m, row, col1, val2);
        m->set_cell(m, row, col2, val1);
    }
    return 0; // Success
}

// Function: m_transpose
// Transposes the matrix in-place.
// Returns 1 on NULL matrix, 0 on success.
int m_transpose(Matrix *m) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }

    unsigned int original_rows = m->rows;
    unsigned int original_cols = m->cols;
    unsigned int original_type = m->type;
    size_t element_size = 0;

    switch (original_type) {
        case 1: element_size = sizeof(short); break;
        case 2: element_size = sizeof(int); break;
        case 4: element_size = sizeof(float); break;
        default: return -1; // Unknown type
    }

    // Create a temporary matrix to store the transposed data
    Matrix *temp_m = create_matrix(original_type, NULL); // temp_m owns its data
    if (temp_m == NULL) {
        return -1; // Allocation failed
    }
    temp_m->set_rows_cols(temp_m, original_cols, original_rows); // Set transposed dimensions

    // Copy and transpose elements
    for (unsigned int row = 0; row < original_rows; row++) {
        for (unsigned int col = 0; col < original_cols; col++) {
            // Use float getter/setter for generality, but the original used int get/set.
            // Sticking to original's type-specific access for transpose
            if (original_type == 4) { // Float
                float val = m->get_fcell(m, row, col);
                temp_m->set_fcell(temp_m, col, row, val);
            } else { // Short/Int
                int val = m->get_cell(m, row, col);
                temp_m->set_cell(temp_m, col, row, val);
            }
        }
    }

    // Update the original matrix's dimensions and data
    m->set_rows_cols(m, original_cols, original_rows);

    // If the original matrix owned its data, free it before memcpy-ing new data
    if (m->owns_data && m->data != NULL) {
        free(m->data);
    }
    // Allocate new data buffer for the transposed matrix
    m->data = calloc(1, m->rows * m->cols * element_size);
    if (m->data == NULL) {
        // Handle error, maybe revert state or indicate failure
        free(temp_m->data);
        free(temp_m);
        return -1;
    }
    m->owns_data = 1;

    // Copy the transposed data from temp_m to m
    memcpy(m->data, temp_m->data, m->rows * m->cols * element_size);

    // Free the temporary matrix and its data
    free(temp_m->data);
    free(temp_m);

    return 0; // Success
}

// Function: swap_nonzero_cell
// Finds the first row below 'row' (inclusive) with a non-zero element at 'col'
// and swaps it with 'row'. Used in Gaussian elimination.
// Returns 1 on NULL matrix, 0xffffffff (-1) if no non-zero row is found or out-of-bounds, 0 on success.
int swap_nonzero_cell(Matrix *m, unsigned int row, unsigned int col) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    if (row >= m->rows || col >= m->cols) {
        return -1; // Error: Out of bounds
    }

    float cell_val = m->get_fcell(m, row, col);

    // If the current cell is near zero, search for a non-zero row below it
    if (fabsf(cell_val) < _DAT_000174c8) { // _DAT_000174c8 is positive epsilon
        unsigned int search_row = row + 1;
        while (search_row < m->rows) {
            float next_cell_val = m->get_fcell(m, search_row, col);
            if (fabsf(next_cell_val) > _DAT_000174c8) { // Found a non-zero pivot
                return swap_row(m, row, search_row);
            }
            search_row++;
        }
        return -1; // Error: No non-zero row found
    }
    return 0; // Success: Current cell is already non-zero
}

// Function: subtract_row
// Performs the row operation: dest_row = dest_row - (factor * src_row).
// Returns 1 on NULL matrix, 0xffffffff (-1) on out-of-bounds, 0 on success.
int subtract_row(Matrix *m, unsigned int src_row, unsigned int dest_row, unsigned int start_col, float factor) {
    if (m == NULL) {
        return 1; // Error: NULL matrix
    }
    if (src_row >= m->rows || dest_row >= m->rows) {
        return -1; // Error: Out of bounds
    }

    for (unsigned int col = start_col; col < m->cols; col++) {
        float src_val = m->get_fcell(m, src_row, col);
        float dest_val = m->get_fcell(m, dest_row, col);
        float new_val = dest_val - (factor * src_val);
        m->set_fcell(m, dest_row, col, new_val);
    }
    return 0; // Success
}

// Function: m_rref
// Computes the Reduced Row Echelon Form (RREF) of a matrix.
// Stores the result in dest_m. src_m remains unchanged.
// Returns 1 on NULL input, 0 on success.
int m_rref(Matrix *src_m, Matrix *dest_m) {
    if (src_m == NULL || dest_m == NULL) {
        return 1; // Error: NULL matrix
    }

    // Initialize dest_m as a float matrix with same dimensions as src_m
    dest_m->type = 4; // RREF typically uses float values
    dest_m->set_rows_cols(dest_m, src_m->rows, src_m->cols);

    // Copy src_m to dest_m, converting all values to float
    for (unsigned int row = 0; row < src_m->rows; row++) {
        for (unsigned int col = 0; col < src_m->cols; col++) {
            dest_m->set_fcell(dest_m, row, col, src_m->get_fcell(src_m, row, col));
        }
    }

    unsigned int pivot_row = 0;
    for (unsigned int pivot_col = 0; pivot_col < dest_m->cols && pivot_row < dest_m->rows; pivot_col++) {
        // Find a non-zero pivot for the current column
        if (swap_nonzero_cell(dest_m, pivot_row, pivot_col) != 0) {
            // If no non-zero pivot found in this column, move to next column
            continue;
        }

        float pivot_val = dest_m->get_fcell(dest_m, pivot_row, pivot_col);

        // Normalize the pivot row (make pivot element 1)
        for (unsigned int current_col = pivot_col; current_col < dest_m->cols; current_col++) {
            float val = dest_m->get_fcell(dest_m, pivot_row, current_col);
            dest_m->set_fcell(dest_m, pivot_row, current_col, val / pivot_val);
        }

        // Eliminate other rows (make elements above and below pivot zero)
        for (unsigned int current_row = 0; current_row < dest_m->rows; current_row++) {
            if (current_row != pivot_row) {
                float factor = dest_m->get_fcell(dest_m, current_row, pivot_col);
                if (fabsf(factor) > EPSILON) { // Only subtract if factor is not zero
                    subtract_row(dest_m, pivot_row, current_row, pivot_col, factor);
                }
            }
        }
        pivot_row++; // Move to the next pivot row
    }
    return 0; // Success
}