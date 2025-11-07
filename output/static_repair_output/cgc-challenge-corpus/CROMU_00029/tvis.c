#include <stdio.h>
#include <math.h>     // For round()
#include <stdint.h>   // For uint32_t
#include <stdlib.h>   // For malloc/free

// --- Global variables (placeholders, types inferred from usage) ---
// Assuming 'output' and 'TGrid' are dynamically allocated arrays of doubles.
// The indexing 'output[col_idx * OUTPUT_ROW_STRIDE + row_idx]' implies that
// OUTPUT_ROW_STRIDE is the stride for column-major access.
// The original code used '10' as a literal, suggesting a fixed maximum number of rows.
// We use a symbolic constant for clarity and potential flexibility.
#define OUTPUT_ROW_STRIDE 100 // Maximum number of rows in the output grid for indexing
#define MAX_OUTPUT_COLS   100 // Maximum number of columns in the output grid

static double *output; // Flattened 2D array, accessed as output[col_idx * OUTPUT_ROW_STRIDE + row_idx]
static double *TGrid;  // Flattened 3D array, accessed as TGrid[z_idx * Y * X + y_idx * X + x_idx]

static int X = 50;    // Example value for X dimension of TGrid
static int Y = 30;    // Example value for Y dimension of TGrid
static int Z = 3;     // Example value for Z dimension of TGrid

// --- Global constants (placeholders, types inferred) ---
static double DAT_000166c0 = 100.0; // Offset/baseline for temperature calculations
static double DAT_000166c8 = 5.0;  // Scaling factor for grid blocks
static double DAT_000166d0 = -10.0; // Offset applied for negative values
static double DAT_000166d8 = 10.0; // Offset applied for positive values
static double DAT_000166e0 = 100.0; // Threshold for printing format
static double DAT_000166e8 = 110.0; // Threshold for printing format
static double DAT_000166f0 = 120.0; // Threshold for printing format (renamed from _DAT_000166f0)

// --- Forward declaration for functions not provided ---
// A simple stub for float_to_str to ensure compilability.
void float_to_str(double value, uint32_t format_param) {
    // In a real application, this function would format the double into a string,
    // possibly using 'format_param' to specify formatting rules.
    // For this exercise, it serves as a placeholder.
    (void)value;         // Suppress unused variable warning
    (void)format_param;  // Suppress unused variable warning
}

// Function: output_str
void output_str(int col_idx, int row_idx, uint32_t format_param) {
  // Accesses output array using column-major indexing: output[column * row_stride + row]
  float_to_str(output[col_idx * OUTPUT_ROW_STRIDE + row_idx] - DAT_000166c0, format_param);
}

// Function: TGridAverage
long double TGridAverage(int output_col_idx, int output_row_idx, unsigned int start_x, unsigned int start_y,
                         int width, int height, int z_index) {
  double sum = 0.0;
  double count = 0.0;

  // Adjust width and height to not exceed TGrid boundaries
  if ((unsigned int)X < start_x + width) {
    width = X - start_x;
  }
  if ((unsigned int)Y < start_y + height) {
    height = Y - start_y;
  }

  // Ensure width and height are non-negative after adjustment
  if (width < 0) width = 0;
  if (height < 0) height = 0;

  for (unsigned int x = start_x; x < start_x + width; ++x) {
    for (unsigned int y = start_y; y < start_y + height; ++y) {
      // Access TGrid as a flattened 3D array: TGrid[z][y][x]
      sum += TGrid[z_index * Y * X + y * X + x];
      count += 1.0;
    }
  }

  long double average = 0.0L; // Use L suffix for long double literal
  if (count > 0.0) { // Avoid division by zero
    average = (long double)sum / count;
  }

  // Store result in output grid using column-major indexing
  output[output_col_idx * OUTPUT_ROW_STRIDE + output_row_idx] = (double)average; // Store as double

  return average;
}

// Function: GraphTemps
unsigned int GraphTemps(void) {
  printf("\x1b[2J\x1b[H"); // ANSI escape codes: Clear screen, move cursor home

  // Calculate scaled X and Y dimensions for averaging blocks
  int block_width = (int)round((double)X / DAT_000166c8 + 1.0);
  int block_height = (int)round((double)Y / DAT_000166c8 + 1.0);

  // Ensure block_width and block_height are at least 1 to avoid infinite loops or division by zero
  if (block_width <= 0) block_width = 1;
  if (block_height <= 0) block_height = 1;

  // These variables track the actual dimensions of the output grid generated for the current Z layer.
  // They are reset for each Z layer, reflecting the original logic.
  int actual_output_cols_for_z = 0;
  int actual_output_rows_for_z = 0;

  for (int z_idx = 0; z_idx < Z; ++z_idx) { // Loop through Z layers
    printf("z: %u\n", z_idx);

    int current_output_row_idx = 0;
    for (int y_start = 0; y_start < Y; y_start += block_height) { // Loop through Y-axis (rows of blocks)
      int current_output_col_idx = 0;
      for (int x_start = 0; x_start < X; x_start += block_width) { // Loop through X-axis (columns of blocks)
        TGridAverage(current_output_col_idx, current_output_row_idx, x_start, y_start, block_width, block_height, z_idx);
        current_output_col_idx++;
      }
      // After processing all columns for this row, update max columns for the current Z layer
      if (current_output_col_idx > actual_output_cols_for_z) {
          actual_output_cols_for_z = current_output_col_idx;
      }
      current_output_row_idx++;
    }
    // After processing all rows, update max rows for the current Z layer
    if (current_output_row_idx > actual_output_rows_for_z) {
        actual_output_rows_for_z = current_output_row_idx;
    }

    // Print the averaged grid for the current Z layer
    // The loop structure (outer: col, inner: row) and newline placement
    // imply a column-major display, where each column's data is printed
    // on a new line. This behavior is preserved from the original snippet.
    for (int col = 0; col < actual_output_cols_for_z; ++col) {
      for (int row = 0; row < actual_output_rows_for_z; ++row) {
        // Accessing output[col * OUTPUT_ROW_STRIDE + row]
        double val = output[col * OUTPUT_ROW_STRIDE + row] - DAT_000166c0;

        double offset_val = 0.0;
        if (val > 0.0) {
          offset_val = DAT_000166d8;
        } else if (val < 0.0) {
          offset_val = DAT_000166d0;
        }

        double combined_val = val + offset_val;

        // Conditional printing based on combined_val, replicating original spacing logic
        // Original non-standard format specifiers ('@f') are replaced with standard '%f'.
        if (DAT_000166e0 < combined_val) {
          if (DAT_000166e8 < combined_val) {
            if (0.0 <= combined_val) {
              if (DAT_000166c8 <= combined_val) {
                if (DAT_000166f0 <= combined_val) { // Renamed from _DAT_000166f0
                  printf(" %f ", val);
                } else {
                  printf("  %f ", val);
                }
              } else {
                printf("   %f ", val);
              }
            } else { // combined_val < 0.0
              printf("  %f ", val);
            }
          } else {
            printf(" %f ", val);
          }
        } else {
          printf("%f ", val);
        }
      }
      printf("\n"); // Newline after each column's data
    }
    printf("\n"); // Extra newline after each Z layer's output
  }
  return 0; // Return 0 for success
}

// Main function to make the snippet compilable and runnable for demonstration.
int main() {
    // Allocate memory for TGrid (Z * Y * X doubles)
    TGrid = (double*)malloc(sizeof(double) * Z * Y * X);
    if (TGrid == NULL) {
        fprintf(stderr, "Failed to allocate TGrid\n");
        return 1;
    }

    // Initialize TGrid with some dummy data for demonstration
    for (int z = 0; z < Z; ++z) {
        for (int y = 0; y < Y; ++y) {
            for (int x = 0; x < X; ++x) {
                // Example data: temperature increasing with z, y, x
                TGrid[z * Y * X + y * X + x] = (double)(100.0 + z * 5.0 + y * 0.5 + x * 0.1);
            }
        }
    }

    // Allocate memory for output grid.
    // The total size is MAX_OUTPUT_COLS * OUTPUT_ROW_STRIDE doubles,
    // to accommodate column-major indexing up to these maximums.
    output = (double*)malloc(sizeof(double) * MAX_OUTPUT_COLS * OUTPUT_ROW_STRIDE);
    if (output == NULL) {
        fprintf(stderr, "Failed to allocate output\n");
        free(TGrid);
        return 1;
    }
    // Initialize output buffer to 0.0 to ensure predictable values for unwritten cells.
    for (int i = 0; i < MAX_OUTPUT_COLS * OUTPUT_ROW_STRIDE; ++i) {
        output[i] = 0.0;
    }

    GraphTemps();

    // Free allocated memory
    free(TGrid);
    free(output);

    return 0;
}