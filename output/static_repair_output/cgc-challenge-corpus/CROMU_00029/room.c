#include <stdio.h>   // For printf, puts, getchar, EOF, stdin, fprintf
#include <stdlib.h>  // For atoi, calloc, free, exit, strtod
#include <string.h>  // For strchr, strtok, strcspn, strcmp
#include <stdbool.h> // For bool type and true/false

// Global variables (inferred from decompiler output)
int X, Y, Z;
double TC[8]; // Thermal Conductivities
double HC[8]; // Heat Capacities
double *TGrid = NULL; // Temperature Grid
double *HGrid = NULL; // Heat Source Grid

// Constants (inferred from decompiler output, using plausible values where actual values were not provided)
// Values for Air
const double AIR_TC_VALUES[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
const double AIR_HC_VALUES[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

// Values for Aluminum (example values)
const double AL_TC_VALUES[8] = {205.0, 208.0, 210.0, 213.0, 216.0, 219.0, 222.0, 225.0};
const double AL_HC_VALUES[8] = {2.42e6, 2.42e6, 2.42e6, 2.42e6, 2.42e6, 2.42e6, 2.42e6, 2.42e6};

// Values for Copper (example values)
const double CU_TC_VALUES[8] = {401.0, 398.0, 395.0, 392.0, 389.0, 386.0, 383.0, 380.0};
const double CU_HC_VALUES[8] = {3.45e6, 3.45e6, 3.45e6, 3.45e6, 3.45e6, 3.45e6, 3.45e6, 3.45e6};

// InitCustom validation constants (inferred from DAT_000165e0 and DAT_000165e8, representing 19% deviation)
const double VALIDATION_MULTIPLIER_LOW = 0.81; // 1 - 0.19
const double VALIDATION_MULTIPLIER_HIGH = 1.19; // 1 + 0.19

// Isothermic temperature range constants (inferred from DAT_000165f0, DAT_000165f8, DAT_00016600, DAT_00016608)
const double ISO_TEMP_MIN_CELSIUS = -50.0;
const double ISO_TEMP_MAX_CELSIUS = 300.0;
const double ISO_TEMP_DEFAULT_CELSIUS = 25.0;
const double CELSIUS_TO_KELVIN_OFFSET = 273.15;

// Other constants
const char *INVALID_VALUE_MESSAGE = "Invalid value...must be within 19%% of the previous value\n";
const char *HEAT_SOURCE_SENTINEL = "@f"; // Inferred from DAT_00016610

// Global buffer for input
char input_buffer[100];

// Dummy functions (implementations based on common patterns)
void _terminate(void) {
    fprintf(stderr, "Program terminated due to an error.\n");
    exit(1);
}

// Reads a line from stdin into the provided buffer
// Returns 0 on success, -1 on error/EOF.
int read_until(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        return -1;
    }
    // Remove trailing newline character if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 0;
}

// Converts string to long double, similar to atof or strtod
long double cgcatof(const char *str) {
    return strtod(str, NULL);
}

// Flushes stdin (clears remaining characters in the input buffer)
void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
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
  // Combine allowed characters into a single string literal
  const char valid_chars[] = "0123456789., ";

  if (ValidStr(param_1, valid_chars) == 0) {
    puts("Invalid characters in dimensions");
    return -1;
  }

  char *token = strtok(param_1, ",");
  if (token == NULL) {
    puts("Invalid X dimension\n");
    return -1;
  }
  X = atoi(token);

  token = strtok(NULL, ",");
  if (token == NULL) {
    puts("Invalid Y dimension\n");
    return -1;
  }
  Y = atoi(token);

  token = strtok(NULL, ",");
  if (token == NULL) {
    puts("Invalid Z dimension\n");
    return -1;
  }
  Z = atoi(token);

  return 0;
}

// Function: InitAir
void InitAir(void) {
  for (int i = 0; i < 8; i++) {
    TC[i] = AIR_TC_VALUES[i];
    HC[i] = AIR_HC_VALUES[i];
  }
}

// Function: InitAluminum
void InitAluminum(void) {
  for (int i = 0; i < 8; i++) {
    TC[i] = AL_TC_VALUES[i];
    HC[i] = AL_HC_VALUES[i];
  }
}

// Function: InitCopper
void InitCopper(void) {
  for (int i = 0; i < 8; i++) {
    TC[i] = CU_TC_VALUES[i];
    HC[i] = CU_HC_VALUES[i];
  }
}

// Function: SetTC
void SetTC(int index, const char *value_str) {
  TC[index] = (double)cgcatof(value_str);
}

// Function: SetHC
void SetHC(int index, const char *value_str) {
  HC[index] = (double)cgcatof(value_str);
}

// Function: InitCustom
void InitCustom(void) {
  const char *temp_strings[] = {
    "-264.15", "-193.15", "-112.15", " -31.15",
    "  49.85", " 129.85", " 210.85", " 291.85"
  };
  double current_value;
  bool input_is_valid;

  puts("Enter the thermal conductivities in W/(m*K) for your material at the following temperatures (Celcius):");
  for (unsigned int i = 0; i < 8; i++) {
    input_is_valid = false;
    while (!input_is_valid) {
      input_is_valid = true;
      printf("  T = %s C: ", temp_strings[i]);
      if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
        _terminate();
      }
      current_value = (double)cgcatof(input_buffer);

      if (i != 0) {
        double prev_val = TC[i - 1];
        double lower_bound, upper_bound;

        if (prev_val >= 0.0) {
          lower_bound = prev_val * VALIDATION_MULTIPLIER_LOW;
          upper_bound = prev_val * VALIDATION_MULTIPLIER_HIGH;
        } else { // prev_val < 0.0
          lower_bound = prev_val * VALIDATION_MULTIPLIER_HIGH;
          upper_bound = prev_val * VALIDATION_MULTIPLIER_LOW;
        }

        if (current_value < lower_bound || current_value > upper_bound) {
          input_is_valid = false;
        }
        if (!input_is_valid) {
          printf(INVALID_VALUE_MESSAGE);
        }
      }
    }
    SetTC(i, input_buffer);
  }

  puts("Enter the heat capacity in J/(m^3*K) for your material when it is at the following temperatures (Celcius):");
  for (unsigned int i = 0; i < 8; i++) {
    input_is_valid = false;
    while (!input_is_valid) {
      input_is_valid = true;
      printf("  T = %s C: ", temp_strings[i]);
      if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
        _terminate();
      }
      current_value = (double)cgcatof(input_buffer);

      if (i != 0) {
        double prev_val = HC[i - 1];
        double lower_bound, upper_bound;

        if (prev_val >= 0.0) {
          lower_bound = prev_val * VALIDATION_MULTIPLIER_LOW;
          upper_bound = prev_val * VALIDATION_MULTIPLIER_HIGH;
        } else { // prev_val < 0.0
          lower_bound = prev_val * VALIDATION_MULTIPLIER_HIGH;
          upper_bound = prev_val * VALIDATION_MULTIPLIER_LOW;
        }

        if (current_value < lower_bound || current_value > upper_bound) {
          input_is_valid = false;
        }
        if (!input_is_valid) {
          printf(INVALID_VALUE_MESSAGE);
        }
      }
    }
    SetHC(i, input_buffer);
  }
}

// Function: AllocateGrid
int AllocateGrid(double **grid_ptr, int dim_x, int dim_y, int dim_z) {
  // Allocate space for dim_x * dim_y * dim_z doubles (8 bytes each)
  *grid_ptr = (double *)calloc((size_t)dim_x * dim_y * dim_z, sizeof(double));
  if (*grid_ptr == NULL) {
    return -1; // Allocation failed
  }
  return 0; // Success
}

// Function: read_temps
// Reads a comma-separated list of temperatures into a grid.
// Return 0 on success, -1 on failure.
int read_temps(double *grid, int dim_x, int dim_y, int dim_z) {
    printf("Enter temperatures (comma-separated): ");
    if (read_until(input_buffer, sizeof(input_buffer)) == -1) {
        return -1;
    }

    char *token = strtok(input_buffer, ",");
    int i = 0;
    size_t total_elements = (size_t)dim_x * dim_y * dim_z;
    while (token != NULL && i < total_elements) {
        if (strcmp(token, HEAT_SOURCE_SENTINEL) == 0) {
            grid[i] = 0.0; // Assume 0.0 for no heat source
        } else {
            grid[i] = cgcatof(token);
        }
        token = strtok(NULL, ",");
        i++;
    }

    if (i < total_elements) {
        fprintf(stderr, "Warning: Not enough temperature values provided. Filling remaining with 0.\n");
        while (i < total_elements) {
            grid[i++] = 0.0;
        }
    }
    return 0;
}


// Function: InitMaterial
int InitMaterial(void) {
  char material_choice = 0;
  int input_status;

  puts("For what material would you like to run this simulation?");
  puts("  1. Air");
  puts("  2. Aluminum");
  puts("  3. Copper");
  puts("  4. Custom");
  while ((material_choice < 1 || (4 < material_choice))) {
    printf("Selection: ");
    input_status = read_until(input_buffer, sizeof(input_buffer));
    if (input_status == -1) {
      return -1;
    }
    material_choice = (char)atoi(input_buffer);
  }

  switch (material_choice) {
    case 1: InitAir(); break;
    case 2: InitAluminum(); break;
    case 3: InitCopper(); break;
    case 4: InitCustom(); break;
    default: return -1; // Should not happen due to while loop condition
  }
  flush_stdin();

  size_t grid_elements_count;
  while (true) {
    grid_elements_count = (size_t)X * Y * Z;
    // Original check `27000 < local_28` where local_28 was `iVar2 * 8` (size in bytes).
    // So `27000 < X * Y * Z * 8`. Max elements approx `27000 / 8 = 3375`.
    if (X > 0 && Y > 0 && Z > 0 && grid_elements_count <= 3375) {
        break;
    }
    printf("How large is the material (X,Y,Z meters): ");
    input_status = read_until(input_buffer, sizeof(input_buffer));
    if (input_status == -1) {
      return -1;
    }
    input_status = ParseDimensions(input_buffer);
    if (input_status != 0) {
      return -1;
    }
  }
  flush_stdin();

  if (AllocateGrid(&TGrid, X, Y, Z) != 0) {
    return -1; // Allocation failed
  }

  char isothermic_choice = -1;
  while (isothermic_choice < 0) {
    printf("Should the material be isothermic? (y,n): ");
    input_status = read_until(input_buffer, sizeof(input_buffer));
    if (input_status == -1) {
      free(TGrid);
      return -1;
    }
    if (input_buffer[0] == 'y' || input_buffer[0] == 'Y') {
      isothermic_choice = 1;
    } else if (input_buffer[0] == 'n' || input_buffer[0] == 'N') {
      isothermic_choice = 0;
    }
  }
  flush_stdin();

  if (isothermic_choice == 0) { // Not isothermic
    printf("  Send the grid temperatures as a comma separated list of Celcius decimal values.\n");
    printf("  The program will populate the X, then Y, then Z dimensions of the room.\n");
    if (read_temps(TGrid, X, Y, Z) != 0) {
      free(TGrid);
      return -1;
    }
  } else { // Isothermic
    double iso_temp_celsius = ISO_TEMP_DEFAULT_CELSIUS;
    bool temp_is_valid = false;
    while (!temp_is_valid) {
      printf("  What temperature? (%.2f - %.2f degrees C): ", ISO_TEMP_MIN_CELSIUS, ISO_TEMP_MAX_CELSIUS);
      input_status = read_until(input_buffer, sizeof(input_buffer));
      if (input_status == -1) {
        free(TGrid);
        return -1;
      }
      iso_temp_celsius = (double)cgcatof(input_buffer);
      if ((ISO_TEMP_MIN_CELSIUS <= iso_temp_celsius) && (iso_temp_celsius <= ISO_TEMP_MAX_CELSIUS)) {
          temp_is_valid = true;
      } else {
          fprintf(stderr, "Invalid temperature. Please enter a value within the specified range.\n");
      }
    }
    flush_stdin();
    double iso_temp_kelvin = CELSIUS_TO_KELVIN_OFFSET + iso_temp_celsius;
    for (int i_x = 0; i_x < X; i_x++) {
      for (int i_y = 0; i_y < Y; i_y++) {
        for (int i_z = 0; i_z < Z; i_z++) {
          TGrid[i_x + (size_t)X * i_y + (size_t)X * Y * i_z] = iso_temp_kelvin;
        }
      }
    }
  }

  char energy_source_choice = -1;
  while (energy_source_choice < 0) {
    printf("Are there any constant energy sources in the room? (y,n): ");
    input_status = read_until(input_buffer, sizeof(input_buffer));
    if (input_status == -1) {
      free(TGrid);
      return -1;
    }
    if (input_buffer[0] == 'y' || input_buffer[0] == 'Y') {
      energy_source_choice = 1;
    } else if (input_buffer[0] == 'n' || input_buffer[0] == 'N') {
      energy_source_choice = 0;
    }
  }
  flush_stdin();

  if (energy_source_choice != 0) { // Yes, there are energy sources
    if (AllocateGrid(&HGrid, X, Y, Z) != 0) {
      free(TGrid);
      return -1;
    }
    printf("  Send the heat sources as temperatures in a comma separated list of Celcius decimal values.\n");
    printf("  The program will populate the X, then Y, then Z dimensions of the room.\n");
    printf("  Send %s for any grid location which doesn't have a heat source.\n", HEAT_SOURCE_SENTINEL);
    if (read_temps(HGrid, X, Y, Z) != 0) {
      free(TGrid);
      free(HGrid);
      return -1;
    }
  }
  return 0; // Success
}

// Main function
int main() {
    // Example usage of InitMaterial
    if (InitMaterial() != 0) {
        fprintf(stderr, "Failed to initialize material properties or grid.\n");
        // Clean up any allocated memory before exiting
        if (TGrid != NULL) free(TGrid);
        if (HGrid != NULL) free(HGrid);
        return 1;
    }

    printf("Material and grid initialized successfully.\n");
    printf("Grid dimensions: X=%d, Y=%d, Z=%d\n", X, Y, Z);
    // Further simulation logic would go here

    // Don't forget to free allocated memory
    if (TGrid != NULL) free(TGrid);
    if (HGrid != NULL) free(HGrid);

    return 0;
}