#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> // For fabs

// Global variables (from decompiler output)
static int X, Y, Z;
static double TC[8]; // Thermal Conductivities
static double HC[8]; // Heat Capacities
static double *TGrid = NULL; // Temperature Grid
static double *HGrid = NULL; // Heat Source Grid

// Decompiler-derived constants (example values, actual values unknown from snippet)
// These would typically be in a data section or defined. Plausible values are used.
static const double AIR_TC_DEFAULTS[8] = {0.024, 0.0247, 0.0254, 0.0262, 0.027, 0.0278, 0.0286, 0.0294};
static const double AIR_HC_DEFAULTS[8] = {1200.0, 1205.0, 1210.0, 1215.0, 1220.0, 1225.0, 1230.0, 1235.0};

static const double ALUMINUM_TC_DEFAULTS[8] = {205.0, 206.0, 207.0, 208.0, 209.0, 210.0, 211.0, 212.0};
static const double ALUMINUM_HC_DEFAULTS[8] = {2420000.0, 2430000.0, 2440000.0, 2450000.0, 2460000.0, 2470000.0, 2480000.0, 2490000.0};

static const double COPPER_TC_DEFAULTS[8] = {401.0, 402.0, 403.0, 404.0, 405.0, 406.0, 407.0, 408.0};
static const double COPPER_HC_DEFAULTS[8] = {3900000.0, 3910000.0, 3920000.0, 3930000.0, 3940000.0, 3950000.0, 3960000.0, 3970000.0};

// Constants for custom material validation (e.g., within 5% range)
static const double PERCENT_LOWER_BOUND = 0.95; // DAT_000165e0 (0.95 for -5%)
static const double PERCENT_UPPER_BOUND = 1.05; // DAT_000165e8 (1.05 for +5%)

// Constants for isothermic temperature range and offset to Kelvin
static const double TEMP_MIN_C = -273.15; // DAT_00016600 (Absolute zero in Celcius)
static const double TEMP_MAX_C = 2000.0;  // DAT_000165f8 (Arbitrary high temp)
static const double TEMP_OFFSET_C_TO_K = 273.15; // DAT_00016608 (Offset to Kelvin)
static const double DEFAULT_HEAT_SOURCE_VALUE = 0.0; // For '@f' in HGrid, representing no heat source

// Placeholder for `DAT_0001613b`
static const char *PROMPT_FORMAT_TEMP = "  Temp %s C: ";

// Helper functions (replacing decompiler artifacts)
int read_until(char *buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1; // Error or EOF
    }
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
    return 0; // Success
}

void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void _terminate(void) {
    exit(1);
}

// Function: ValidStr
int ValidStr(char *param_1, const char *param_2) {
  while (*param_1 != '\0') {
    if (strchr(param_2, (int)*param_1) == NULL) {
      return 0; // Invalid character found
    }
    param_1++;
  }
  return 1; // All characters are valid
}

// Function: ParseDimensions
int ParseDimensions(char *param_1) {
  // Assuming valid characters are digits, comma, period, and space.
  const char *valid_chars = "0123456789,. ";

  // Validate characters in the input string
  if (ValidStr(param_1, valid_chars) == 0) {
    puts("Invalid characters in dimensions");
    return -1;
  }

  char *token;

  // Parse X dimension
  token = strtok(param_1, ",");
  if (token == NULL) {
    puts("Invalid X dimension\n");
    return -1;
  }
  X = atoi(token);

  // Parse Y dimension
  token = strtok(NULL, ",");
  if (token == NULL) {
    puts("Invalid Y dimension\n");
    return -1;
  }
  Y = atoi(token);

  // Parse Z dimension
  token = strtok(NULL, ",");
  if (token == NULL) {
    puts("Invalid Z dimension\n");
    return -1;
  }
  Z = atoi(token);

  return 0; // Success
}

// Function: InitAir
void InitAir(void) {
  for (int i = 0; i < 8; ++i) {
    TC[i] = AIR_TC_DEFAULTS[i];
    HC[i] = AIR_HC_DEFAULTS[i];
  }
}

// Function: InitAluminum
void InitAluminum(void) {
  for (int i = 0; i < 8; ++i) {
    TC[i] = ALUMINUM_TC_DEFAULTS[i];
    HC[i] = ALUMINUM_HC_DEFAULTS[i];
  }
}

// Function: InitCopper
void InitCopper(void) {
  for (int i = 0; i < 8; ++i) {
    TC[i] = COPPER_TC_DEFAULTS[i];
    HC[i] = COPPER_HC_DEFAULTS[i];
  }
}

// Function: SetTC
void SetTC(int param_1, char *param_2) {
  TC[param_1] = atof(param_2);
}

// Function: SetHC
void SetHC(int param_1, char *param_2) {
  HC[param_1] = atof(param_2);
}

// Function: InitCustom
void InitCustom(void) {
  char temp_labels[8][10] = {
      "-264.15", "-193.15", "-112.15", " -31.15",
      "  49.85", " 129.85", " 210.85", " 291.85"
  };
  char input_buffer[100];
  double current_value;
  bool valid_input;

  puts("Enter the thermal conductivities in W/(m*K) for your material at the following temperatures (Celcius):");
  for (int i = 0; i < 8; ++i) {
    valid_input = false;
    while (!valid_input) {
      printf(PROMPT_FORMAT_TEMP, temp_labels[i]);
      if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
        _terminate();
      }
      current_value = atof(input_buffer);

      valid_input = true;
      if (i != 0) { // Validation for subsequent values
        double lower_limit, upper_limit;
        if (TC[i-1] >= 0.0) { // Previous value is non-negative
            lower_limit = PERCENT_LOWER_BOUND * TC[i-1];
            upper_limit = PERCENT_UPPER_BOUND * TC[i-1];
        } else { // Previous value is negative
            // For negative values, the meaning of 'lower' and 'upper' bounds flips
            lower_limit = PERCENT_UPPER_BOUND * TC[i-1];
            upper_limit = PERCENT_LOWER_BOUND * TC[i-1];
        }

        if (current_value < lower_limit || current_value > upper_limit) {
            valid_input = false;
        }

        if (!valid_input) {
          printf("Invalid value...must be within %.0f%% of the previous value\n", (PERCENT_UPPER_BOUND - 1.0) * 100);
        }
      }
    }
    SetTC(i, input_buffer);
  }

  puts("Enter the heat capacity in J/(m^3*K) for your material when it is at the following temperatures (Celcius):");
  for (int i = 0; i < 8; ++i) {
    valid_input = false;
    while (!valid_input) {
      printf(PROMPT_FORMAT_TEMP, temp_labels[i]);
      if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
        _terminate();
      }
      current_value = atof(input_buffer);

      valid_input = true;
      if (i != 0) { // Validation for subsequent values
        double lower_limit, upper_limit;
        if (HC[i-1] >= 0.0) {
            lower_limit = PERCENT_LOWER_BOUND * HC[i-1];
            upper_limit = PERCENT_UPPER_BOUND * HC[i-1];
        } else {
            lower_limit = PERCENT_UPPER_BOUND * HC[i-1];
            upper_limit = PERCENT_LOWER_BOUND * HC[i-1];
        }

        if (current_value < lower_limit || current_value > upper_limit) {
            valid_input = false;
        }

        if (!valid_input) {
          printf("Invalid value...must be within %.0f%% of the previous value\n", (PERCENT_UPPER_BOUND - 1.0) * 100);
        }
      }
    }
    SetHC(i, input_buffer);
  }
}

// Function: AllocateGrid
int AllocateGrid(double **grid_ptr, int x_dim, int y_dim, int z_dim) {
  *grid_ptr = (double *)calloc((size_t)x_dim * y_dim * z_dim, sizeof(double));
  if (*grid_ptr == NULL) {
    return -1; // Allocation failed
  }
  return 0; // Success
}

// Function: read_temps
// Reads comma-separated temperatures into a grid.
// `is_hgrid` flag is true if reading for HGrid, which means '@f' is allowed for 'no heat source'.
int read_temps(double *grid, int x_dim, int y_dim, int z_dim, bool is_hgrid) {
    char input_buffer[4096]; // Larger buffer for potentially long input
    printf("  Send the grid temperatures as a comma separated list of Celcius decimal values.\n");
    printf("  The program will populate the X, then Y, then Z dimensions of the room.\n");
    if (is_hgrid) {
        printf("  Send @f for any grid location which doesn\'t have a heat source.\n");
    }
    printf("Input: ");

    if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
        return -1;
    }

    char *token = strtok(input_buffer, ",");
    int count = 0;
    int total_elements = x_dim * y_dim * z_dim;

    while (token != NULL && count < total_elements) {
        // Trim leading/trailing whitespace from token
        while (*token == ' ') token++;
        char *end_token = token + strlen(token) - 1;
        while (end_token > token && *end_token == ' ') end_token--;
        *(end_token + 1) = '\0';

        if (is_hgrid && (strcmp(token, "@f") == 0 || strcmp(token, "@F") == 0)) {
            grid[count] = DEFAULT_HEAT_SOURCE_VALUE + TEMP_OFFSET_C_TO_K; // Store 0 for no heat, converted to Kelvin.
        } else {
            char *endptr;
            double temp_c = strtod(token, &endptr);
            if (token == endptr || *endptr != '\0') { // Check for valid number conversion
                printf("Invalid number format: %s\n", token);
                return -1;
            }
            grid[count] = temp_c + TEMP_OFFSET_C_TO_K; // Convert Celcius to Kelvin
        }
        count++;
        token = strtok(NULL, ",");
    }

    if (count < total_elements) {
        puts("Not enough temperature values provided.");
        return -1;
    }
    if (token != NULL) {
        puts("Too many temperature values provided.");
        return -1;
    }

    return 0;
}

// Function: InitMaterial
int InitMaterial(void) {
  char input_buffer[100];
  int material_selection_int;
  bool isothermic_selection_made = false;
  bool isothermic_value = false;
  bool heat_source_selection_made = false;
  bool heat_source_value = false;
  double isothermic_temp_c;

  puts("For what material would you like to run this simulation?");
  puts("  1. Air");
  puts("  2. Aluminum");
  puts("  3. Copper");
  puts("  4. Custom");
  while (true) {
    printf("Selection: ");
    if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
      return -1;
    }
    material_selection_int = atoi(input_buffer);
    if ((material_selection_int >= 1) && (material_selection_int <= 4)) {
        break; // Valid selection
    }
    puts("Invalid selection. Please enter a number between 1 and 4.");
  }

  switch (material_selection_int) {
    case 1: InitAir(); break;
    case 2: InitAluminum(); break;
    case 3: InitCopper(); break;
    case 4: InitCustom(); break;
  }

  flush_stdin();

  // Dimension input loop
  int total_elements = 0;
  while (X <= 0 || Y <= 0 || Z <= 0 || total_elements > 3375) {
    printf("How large is the material (X,Y,Z meters, total elements <= 3375): ");
    if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
      return -1;
    }
    if (ParseDimensions(input_buffer) != 0) {
      X = Y = Z = 0; // Reset to force re-entry
      continue;
    }
    total_elements = X * Y * Z;
    if (total_elements <= 0) {
        puts("Dimensions must be positive.");
    } else if (total_elements > 3375) {
        printf("Total elements (%d) exceeds maximum allowed (3375).\n", total_elements);
    }
  }

  flush_stdin();

  // Allocate TGrid
  if (AllocateGrid(&TGrid, X, Y, Z) != 0) {
    puts("Failed to allocate TGrid.");
    return -1;
  }

  // Isothermic selection
  while (!isothermic_selection_made) {
    printf("Should the material be isothermic? (y,n): ");
    if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
      free(TGrid);
      return -1;
    }
    if ((input_buffer[0] == 'y') || (input_buffer[0] == 'Y')) {
      isothermic_value = true;
      isothermic_selection_made = true;
    }
    else if ((input_buffer[0] == 'n') || (input_buffer[0] == 'N')) {
      isothermic_value = false;
      isothermic_selection_made = true;
    } else {
        puts("Invalid input. Please enter 'y' or 'n'.");
    }
  }
  flush_stdin();

  if (!isothermic_value) { // Not isothermic, read grid temperatures
    if (read_temps(TGrid, X, Y, Z, false) != 0) { // false for TGrid (no '@f' check)
      free(TGrid);
      return -1;
    }
  } else { // Isothermic, get single temperature
    bool valid_temp = false;
    while (!valid_temp) {
      printf("  What temperature? (%.2f - %.2f degrees C): ", TEMP_MIN_C, TEMP_MAX_C);
      if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
        free(TGrid);
        return -1;
      }
      isothermic_temp_c = atof(input_buffer);
      if ((TEMP_MIN_C <= isothermic_temp_c) && (isothermic_temp_c <= TEMP_MAX_C)) {
        valid_temp = true;
      } else {
        puts("Invalid temperature. Please enter a value within the specified range.");
      }
    }
    flush_stdin();
    // Populate TGrid with the isothermic temperature (converted to Kelvin)
    for (int k = 0; k < Z; ++k) {
      for (int j = 0; j < Y; ++j) {
        for (int i = 0; i < X; ++i) {
          TGrid[i + X * j + X * Y * k] = isothermic_temp_c + TEMP_OFFSET_C_TO_K;
        }
      }
    }
  }

  // Heat source selection
  while (!heat_source_selection_made) {
    printf("Are there any constant energy sources in the room? (y,n): ");
    if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
      free(TGrid);
      return -1;
    }
    if ((input_buffer[0] == 'y') || (input_buffer[0] == 'Y')) {
      heat_source_value = true;
      heat_source_selection_made = true;
    }
    else if ((input_buffer[0] == 'n') || (input_buffer[0] == 'N')) {
      heat_source_value = false;
      heat_source_selection_made = true;
    } else {
        puts("Invalid input. Please enter 'y' or 'n'.");
    }
  }
  flush_stdin();

  if (heat_source_value) {
    if (AllocateGrid(&HGrid, X, Y, Z) != 0) {
      puts("Failed to allocate HGrid.");
      free(TGrid);
      return -1;
    }
    if (read_temps(HGrid, X, Y, Z, true) != 0) { // true for HGrid (check for '@f')
      free(TGrid);
      free(HGrid);
      return -1;
    }
  }

  return 0; // Success
}

// Main function (for compilation and testing)
int main() {
    printf("Starting material initialization...\n");
    if (InitMaterial() != 0) {
        printf("Material initialization failed.\n");
        // Clean up any allocated grids before exiting
        if (TGrid) free(TGrid);
        if (HGrid) free(HGrid);
        return 1;
    }
    printf("Material initialization successful.\n");
    printf("Grid dimensions: X=%d, Y=%d, Z=%d\n", X, Y, Z);
    
    // Example: print first TC/HC values and first TGrid/HGrid values
    printf("TC[0]: %.2f, HC[0]: %.2f\n", TC[0], HC[0]);
    if (TGrid) {
        printf("TGrid[0]: %.2f K (%.2f C)\n", TGrid[0], TGrid[0] - TEMP_OFFSET_C_TO_K);
    }
    if (HGrid) {
        printf("HGrid[0]: %.2f K (%.2f C)\n", HGrid[0], HGrid[0] - TEMP_OFFSET_C_TO_K);
    } else {
        printf("No heat sources defined.\n");
    }

    // Free allocated memory
    if (TGrid) free(TGrid);
    if (HGrid) free(HGrid);
    return 0;
}