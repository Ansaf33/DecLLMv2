#include <stdio.h>    // For printf, fgets, fprintf
#include <stdlib.h>   // For malloc, free, calloc, strtold
#include <string.h>   // For strcspn
#include <math.h>     // For round
#include <stdbool.h>  // For bool type

// --- Global Constants and Variables ---
// Define grid dimensions (arbitrary values for compilation)
const int X = 10;
const int Y = 10;
const int Z = 10;

// Grid pointers
double* TGrid = NULL; // Temperature Grid
double* HGrid = NULL; // Heat Grid (optional, can be NULL if not used)

// Simulation parameters
double SIM_TIME = 0.0;
double TimeStep = 0.0;

// Placeholder constants from original code, assigned arbitrary values
const double DAT_00016680 = 1.0; // Scaling factor for TC/HC array indexing
const double DAT_00016688 = 50.0; // Threshold for HGrid in Tnew
const double DAT_00016690 = 100.0; // Initial max_timestep in CalcTimeStep

// Placeholder arrays for TC and HC
// Assuming a reasonable max index based on possible values of *pdVar1 / DAT_00016680
#define MAX_TC_HC_INDEX 200
double TC[MAX_TC_HC_INDEX];
double HC[MAX_TC_HC_INDEX];

// --- Helper Functions (replacements for original undefined ones) ---

// Replacement for read_until. Simplified to read a line from stdin.
// Returns -1 on EOF or error, 0 on success.
int read_until(char* buffer, const char* delimiter_ignored, size_t max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1; // EOF or error
    }
    // Remove trailing newline character if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 0; // Success
}

// Replacement for cgcatof
long double cgcatof(const char* str) {
    return strtold(str, NULL);
}

// Replacement for ROUND
// Using standard C99 round function
#define ROUND round

// Dummy function for GraphTemps
void GraphTemps(double* grid) {
    // In a real application, this would visualize the temperature grid.
    // For now, it's a placeholder.
    // printf("Graphing temperatures...\n");
}

// Dummy function for kbhit
int kbhit(void) {
    // In a real application, this would check for keyboard input.
    // For now, it's a placeholder.
    return 0;
}

// --- Original Functions (fixed and refactored) ---

// Function: pGRID
// param_1 is now expected to be a pointer to the base of the grid (e.g., TGrid, HGrid)
// The `* 8` factor in the original code implies `sizeof(double)`.
// By making `base_grid` a `double*`, pointer arithmetic handles scaling by `sizeof(double)`.
double* pGRID(double* base_grid, int x, int y, int z) {
  // Assuming a row-major order: index = x + X*y + X*Y*z
  // Cast to long long for intermediate products to prevent overflow for large X, Y, Z
  return base_grid + (x + (long long)X * y + (long long)X * Y * z);
}

// Function: GetSimLength
int GetSimLength(void) {
  char input_buffer[104]; // Buffer for user input
  
  SIM_TIME = 0.0;
  while(true) {
    // The original logic returns 0 immediately if SIM_TIME > 0.
    // Given SIM_TIME is initialized to 0.0, this condition is only met
    // if a previous iteration successfully set SIM_TIME to a positive value.
    if (SIM_TIME > 0.0) {
      return 0; // Simulation length successfully set
    }
    
    printf("For how long would you like to run the simulation? (s): ");
    if (read_until(input_buffer, NULL, sizeof(input_buffer) - 1) == -1) {
        // Error or EOF during read
        return 0xffffffff; // Original error return value
    }
    
    SIM_TIME = (double)cgcatof(input_buffer);
  }
}

// Function: L
long double L(unsigned int x, unsigned int y, unsigned int z) {
  if (x < X && y < Y && z < Z) {
    double grid_value = *pGRID(TGrid, x, y, z);
    // Assuming TC is an array and DAT_00016680 is a divisor for indexing.
    // &TC in original was likely a typo for just TC.
    int index = (int)ROUND(grid_value / DAT_00016680);
    // Basic bounds checking for the array index
    if (index < 0 || index >= MAX_TC_HC_INDEX) {
        fprintf(stderr, "L: TC index out of bounds: %d at (%u, %u, %u)\n", index, x, y, z);
        return -(long double)1;
    }
    return (long double)TC[index];
  }
  return -(long double)1;
}

// Function: C
long double C(unsigned int x, unsigned int y, unsigned int z) {
  if (x < X && y < Y && z < Z) {
    double grid_value = *pGRID(TGrid, x, y, z);
    // Assuming HC is an array and DAT_00016680 is a divisor for indexing.
    // &HC in original was likely a typo for just HC.
    int index = (int)ROUND(grid_value / DAT_00016680);
    // Basic bounds checking for the array index
    if (index < 0 || index >= MAX_TC_HC_INDEX) {
        fprintf(stderr, "C: HC index out of bounds: %d at (%u, %u, %u)\n", index, x, y, z);
        return -(long double)1;
    }
    return (long double)HC[index];
  }
  return -(long double)1;
}

// Function: K
long double K(int x, int y, int z, int dx, int dy, int dz) {
  long double L1, L2;
  
  // Check for boundary conditions (dx, dy, dz are direction vectors)
  bool is_boundary = 
      ((dx == -1 && x == 0) || (dx == 1 && x == X - 1)) ||
      ((dy == -1 && y == 0) || (dy == 1 && y == Y - 1)) ||
      ((dz == -1 && z == 0) || (dz == 1 && z == Z - 1));

  if (is_boundary) {
    L1 = L(x, y, z);
    // The original expression `1 / ((longdouble)1 / (L1 + L1))` literally means `L1 + L1`.
    return L1 + L1;
  } else {
    L1 = L(x, y, z);
    L2 = L(x + dx, y + dy, z + dz);
    // Original: 1 / (1 / (L2 + L2) + 1 / (L1 + L1))
    return (long double)1 / ((long double)1 / (L2 + L2) + (long double)1 / (L1 + L1));
  }
}

// Function: H
long double H(int x, int y, int z) {
  long double accumulated_flux = 0.0L;
  double current_temp = *pGRID(TGrid, x, y, z);
  long double K_val;
  double neighbor_temp;

  // Check neighbors and sum heat flux
  // -X direction
  if (x != 0) {
    K_val = K(x, y, z, -1, 0, 0);
    neighbor_temp = *pGRID(TGrid, x - 1, y, z);
    accumulated_flux += (neighbor_temp - current_temp) * (double)K_val;
  }
  // +X direction
  if (x != X - 1) {
    K_val = K(x, y, z, 1, 0, 0);
    neighbor_temp = *pGRID(TGrid, x + 1, y, z);
    accumulated_flux += (neighbor_temp - current_temp) * (double)K_val;
  }
  // -Y direction
  if (y != 0) {
    K_val = K(x, y, z, 0, -1, 0);
    neighbor_temp = *pGRID(TGrid, x, y - 1, z);
    accumulated_flux += (neighbor_temp - current_temp) * (double)K_val;
  }
  // +Y direction
  if (y != Y - 1) {
    K_val = K(x, y, z, 0, 1, 0);
    neighbor_temp = *pGRID(TGrid, x, y + 1, z);
    accumulated_flux += (neighbor_temp - current_temp) * (double)K_val;
  }
  // -Z direction
  if (z != 0) {
    K_val = K(x, y, z, 0, 0, -1);
    neighbor_temp = *pGRID(TGrid, x, y, z - 1);
    accumulated_flux += (neighbor_temp - current_temp) * (double)K_val;
  }
  // +Z direction
  if (z != Z - 1) {
    K_val = K(x, y, z, 0, 0, 1);
    neighbor_temp = *pGRID(TGrid, x, y, z + 1);
    accumulated_flux += (neighbor_temp - current_temp) * (double)K_val;
  }
  return accumulated_flux;
}

// Function: Tnew
long double Tnew(unsigned int x, unsigned int y, unsigned int z) {
  long double new_temp_val;

  // Refactored to remove goto
  if (HGrid != NULL) { // Check if HGrid is initialized (assuming NULL means 0)
    double h_grid_val = *pGRID(HGrid, x, y, z);
    if (DAT_00016688 < h_grid_val) {
      new_temp_val = (long double)h_grid_val;
      return new_temp_val;
    }
  }
  
  double current_t_val = *pGRID(TGrid, x, y, z);
  long double C_val = C(x, y, z);
  long double H_val = H(x, y, z);
  
  // Calculate new temperature based on heat flow and time step
  // Reduced intermediate variables by direct calculation
  new_temp_val = H_val * ((long double)TimeStep / C_val) + (long double)current_t_val;
  
  return new_temp_val;
}

// Function: CalcTimeStep
void CalcTimeStep(void) {
  long double min_tau = (long double)DAT_00016690; // Initialize with a large value
  unsigned int x_idx, y_idx, z_idx;

  for (z_idx = 0; z_idx < Z; ++z_idx) {
    for (y_idx = 0; y_idx < Y; ++y_idx) {
      for (x_idx = 0; x_idx < X; ++x_idx) {
        // Accumulate sum of K values to reduce intermediate variables
        long double sum_K = 0.0L;
        sum_K += K(x_idx, y_idx, z_idx, -1, 0, 0);
        sum_K += K(x_idx, y_idx, z_idx, 1, 0, 0);
        sum_K += K(x_idx, y_idx, z_idx, 0, -1, 0);
        sum_K += K(x_idx, y_idx, z_idx, 0, 1, 0);
        sum_K += K(x_idx, y_idx, z_idx, 0, 0, -1);
        sum_K += K(x_idx, y_idx, z_idx, 0, 0, 1);
        
        long double C_val = C(x_idx, y_idx, z_idx);
        
        long double current_tau = 0.0L;
        if (sum_K != 0.0L) { // Avoid division by zero
            current_tau = C_val / sum_K;
        }

        if (current_tau < min_tau && current_tau > 0.0L) {
          min_tau = current_tau;
        }
      }
    }
  }
  TimeStep = (double)min_tau;
}

// Function: SimStep
int SimStep(void) {
  // Use a more descriptive name for the new grid pointer
  double* new_TGrid_ptr = (double*)calloc((long long)X * Y * Z, sizeof(double));
  
  if (new_TGrid_ptr == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for new_TGrid.\n");
    return 0xffffffff; // Error code
  }

  unsigned int x_idx, y_idx, z_idx;
  for (z_idx = 0; z_idx < Z; ++z_idx) {
    for (y_idx = 0; y_idx < Y; ++y_idx) {
      for (x_idx = 0; x_idx < X; ++x_idx) {
        // Calculate new temperature and store it in the temporary grid
        *pGRID(new_TGrid_ptr, x_idx, y_idx, z_idx) = (double)Tnew(x_idx, y_idx, z_idx);
      }
    }
  }
  
  // Free the old grid and update TGrid to the new one
  free(TGrid);
  TGrid = new_TGrid_ptr;
  
  return 0; // Success
}

// Function: IncrementTimestep
void IncrementTimestep(double* current_sim_time) {
  *current_sim_time += TimeStep;
}

// Function: RunSim
int RunSim(void) {
  double current_sim_time = 0.0;
  
  CalcTimeStep(); // Determine the appropriate time step for the simulation
  
  while(true) {
    if (SIM_TIME <= current_sim_time) {
      return 0; // Simulation finished
    }
    
    int sim_step_result = SimStep();
    if (sim_step_result != 0) {
      fprintf(stderr, "Error: SimStep failed.\n");
      break; // Exit loop on error
    }
    
    IncrementTimestep(&current_sim_time);
    GraphTemps(TGrid); // Visualize temperatures (dummy for now)
    printf("At %.2f seconds\n", current_sim_time); // Fixed format specifier
    kbhit(); // Check for user input (dummy for now)
  }
  return 0xffffffff; // Error return
}

// --- Main Function ---
int main() {
    // Initialize TC and HC arrays with some dummy values
    for (int i = 0; i < MAX_TC_HC_INDEX; ++i) {
        TC[i] = 10.0 + (double)i * 0.1; // Example values
        HC[i] = 5.0 + (double)i * 0.05; // Example values
    }

    // Initialize TGrid with some initial temperatures (e.g., all 20.0)
    // Allocate memory for TGrid
    TGrid = (double*)calloc((long long)X * Y * Z, sizeof(double));
    if (TGrid == NULL) {
        fprintf(stderr, "Failed to allocate initial TGrid.\n");
        return 1;
    }
    // Set initial temperatures
    for (int i = 0; i < X * Y * Z; ++i) {
        TGrid[i] = 20.0; // Initial uniform temperature
    }

    // Optional: Initialize HGrid if used. For now, keep it NULL as per original code's check.
    // If HGrid were to be used, it would be allocated here similarly to TGrid.
    // HGrid = (double*)calloc((long long)X * Y * Z, sizeof(double));
    // if (HGrid != NULL) {
    //     for (int i = 0; i < X * Y * Z; ++i) {
    //         HGrid[i] = 0.0; // Example initial HGrid values
    //     }
    // }

    printf("Starting simulation...\n");
    
    // Get simulation length from user
    if (GetSimLength() != 0) {
        fprintf(stderr, "Failed to get simulation length or invalid input.\n");
        free(TGrid);
        // if (HGrid != NULL) free(HGrid); // Free HGrid if it was allocated
        return 1;
    }

    if (SIM_TIME <= 0.0) {
        printf("Simulation time is zero or negative. Exiting.\n");
        free(TGrid);
        // if (HGrid != NULL) free(HGrid); // Free HGrid if it was allocated
        return 0;
    }

    // Run the simulation
    int result = RunSim();

    if (result == 0) {
        printf("Simulation completed successfully.\n");
    } else {
        printf("Simulation ended with an error.\n");
    }

    // Cleanup
    free(TGrid);
    // if (HGrid != NULL) free(HGrid); // Free HGrid if it was allocated

    return 0;
}