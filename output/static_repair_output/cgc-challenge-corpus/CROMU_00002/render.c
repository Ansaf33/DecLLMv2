#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, exit, perror
#include <math.h>    // For round

// Define constants based on analysis
#define GRID_DIM 20   // 0x14
#define CELL_SIZE 20  // 0x14, sizeof(GridCell)

// Structure for a single grid cell
typedef struct {
    double y_coord; // Offset 0
    double x_coord; // Offset 8
    int count;      // Offset 16 (0x10)
} GridCell;

// Global variables (mocked/assumed declarations)
unsigned char *g_renderGrid = NULL;
double g_render_delta_x = 1.0;  // DAT_00014008
double g_render_delta_y = 1.0;  // DAT_00014008
const double _DAT_00014010 = 0.0; // Base coordinate value
const double DAT_00014008 = 1.0; // Delta value for grid steps

// Mock allocation function
// Returns 0 on success, 1 on failure. Stores allocated pointer in ptr_array.
int allocate(size_t size, int flags, unsigned char **ptr_array) {
    *ptr_array = (unsigned char *)malloc(size);
    if (*ptr_array == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }
    // flags parameter is ignored in this mock
    (void)flags; 
    return 0;
}

// Mock termination function
void _terminate(int status) {
    exit(status);
}

// Function: init_render_grid
void init_render_grid(void) {
  unsigned char *allocated_ptr = NULL;
  
  g_renderGrid = NULL; // Initialize global pointer
  
  // Allocate memory for the grid (GRID_DIM x GRID_DIM cells, each CELL_SIZE bytes)
  if (allocate(GRID_DIM * GRID_DIM * CELL_SIZE, 0, &allocated_ptr) != 0) {
    _terminate(1);
  }
  
  g_renderGrid = allocated_ptr;
  g_render_delta_x = DAT_00014008;
  g_render_delta_y = DAT_00014008;
  
  // Initialize each grid cell
  for (unsigned int row = 0; row < GRID_DIM; ++row) {
    for (unsigned int col = 0; col < GRID_DIM; ++col) {
      // Calculate linear index for row-major order (col + row * GRID_DIM)
      unsigned int linear_idx = col + row * GRID_DIM;
      
      // Get a pointer to the current GridCell
      GridCell *current_cell = (GridCell *)(g_renderGrid + linear_idx * CELL_SIZE);
      
      // Assign coordinates and initialize count to 0
      current_cell->y_coord = _DAT_00014010 + (double)row * g_render_delta_y;
      current_cell->x_coord = _DAT_00014010 + (double)col * g_render_delta_x;
      current_cell->count = 0;
    }
  }
}

// Function: clear_render_grid
void clear_render_grid(void) {
  for (unsigned int row = 0; row < GRID_DIM; ++row) {
    for (unsigned int col = 0; col < GRID_DIM; ++col) {
      // Calculate linear index for row-major order
      unsigned int linear_idx = col + row * GRID_DIM;
      GridCell *current_cell = (GridCell *)(g_renderGrid + linear_idx * CELL_SIZE);
      current_cell->count = 0; // Reset count
    }
  }
}

// Function: add_render_grid
void add_render_grid(double *param_1) {
  // Calculate grid indices (col_idx for x, row_idx for y)
  int col_idx = (int)round((param_1[0] - _DAT_00014010) / g_render_delta_x);
  int row_idx = (int)round((param_1[1] - _DAT_00014010) / g_render_delta_y);
  
  // Bounds check to ensure indices are within grid dimensions
  if (col_idx < 0 || col_idx >= GRID_DIM || 
      row_idx < 0 || row_idx >= GRID_DIM) {
      return; // Ignore out-of-bounds points
  }

  // Calculate linear index for row-major order
  unsigned int linear_idx = col_idx + row_idx * GRID_DIM;
  
  GridCell *current_cell = (GridCell *)(g_renderGrid + linear_idx * CELL_SIZE);
  current_cell->count++; // Increment the count for the corresponding cell
}

// Function: display_render_grid
void display_render_grid(void) {
  // Print top border
  for (unsigned int i = 0; i < GRID_DIM; ++i) {
    printf("-");
  }
  printf("\n");
  
  // Print grid content, iterating rows from top (GRID_DIM-1) down to bottom (0)
  for (int row = GRID_DIM - 1; row >= 0; --row) { 
    for (unsigned int col = 0; col < GRID_DIM; ++col) {
      // Calculate linear index for row-major order
      unsigned int linear_idx = col + row * GRID_DIM;
      GridCell *current_cell = (GridCell *)(g_renderGrid + linear_idx * CELL_SIZE);
      printf("%d", current_cell->count); // Print cell count
    }
    printf("\n");
  }
  
  // Print bottom border
  for (unsigned int i = 0; i < GRID_DIM; ++i) {
    printf("-");
  }
  printf("\n");
}