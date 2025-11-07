#include <unistd.h> // For ssize_t, and potential `read` if `receive` is a wrapper
#include <string.h> // For strncpy, strcpy, strlen
#include <stdio.h>  // For printf, puts
#include <stdlib.h> // For atoi, atof
#include <stddef.h> // For size_t

// Basic types for clarity, matching common decompiled patterns
// In a real project, these would likely be standard types like unsigned int, unsigned char.
typedef unsigned int uint;
typedef unsigned char byte;

// Placeholder definitions for external functions and variables
// These would typically be in a header file or other compilation units.

// External functions (placeholders)
// Assuming 'receive' behaves like a simplified 'read' returning 0 on success, non-zero on error.
// And 'bytes_read' is the actual count of bytes read.
extern int receive(int fd, char *buf, size_t count, int *bytes_read);
extern void cromu_puts(const char *s);
extern void cromu_printf(const char *format, ...);
extern int cromu_atoi(const char *s);
extern double cromu_atof(const char *s);
extern void _terminate(void); // Assuming it takes no arguments and exits the program

// External global variables (placeholders)
extern uint NumAsp;
extern uint NumFilters;
extern uint NumDisinfection;
extern double _DAT_00018610; // Assuming this is a constant, e.g., 100.0
extern uint SimStepCount;

// Pointers to structures/arrays (placeholders)
// Assuming these are pointers to the base of arrays of structures, each 0xE4 bytes long.
#define STRUCT_SIZE_E4 0xE4
extern void *Asp;
extern void *Filter;
extern void *Disinfection;

// Pointers to valve structures/variables (placeholders)
extern void *HeadworksOutputValve;
extern void *AspOutputValve;
extern void *FilterOutputValve;
extern void *DisinfectionOutputValve;

// Functions for changing rates/percentages (placeholders)
extern void ChangeAlumRate(void *asp_ptr, double rate);
extern void ChangeAerationPercent(void *asp_ptr, double percent);
extern void ChangeGlycerinRate(void *asp_ptr, double rate);
extern void ChangeNumActiveFilters(void *filter_ptr, byte count);
extern void ChangeChlorineRate(void *disinfection_ptr, double rate);
extern void SetValveOutput(void *valve_ptr, byte output_index, double percent);

// Functions for printing (placeholders)
extern void PrintPlantSchematic(void);
extern void PrintPlantStatus(void);

// Global char arrays for names (placeholders, assuming they are char[32] or similar)
// Assuming a max name length of 31 characters + null terminator for these.
#define MAX_NAME_LEN 31
extern char DAT_0001ed41[MAX_NAME_LEN + 1]; // Influent name
extern char DAT_0001e241[MAX_NAME_LEN + 1]; // Headworks name
extern char DAT_0001ef41[MAX_NAME_LEN + 1]; // ASP name base (first ASP name)
extern char DAT_0001e341[MAX_NAME_LEN + 1]; // Filter name base (first Filter name)
extern char DAT_0001e8c1[MAX_NAME_LEN + 1]; // Disinfection name base (first Disinfection name)
extern char DAT_0001f3c1[MAX_NAME_LEN + 1]; // Effluent name
extern char DAT_0001ee41[MAX_NAME_LEN + 1]; // V1 name
extern char DAT_0001e041[MAX_NAME_LEN + 1]; // V2 name
extern char DAT_0001e7c1[MAX_NAME_LEN + 1]; // V3 name
extern char DAT_0001e141[MAX_NAME_LEN + 1]; // V4 name

// Function: read_until
// Reads from file descriptor 0 (stdin) until 'delimiter' is found or 'max_len - 1' characters are read.
// Null-terminates the buffer. Returns number of characters read (excluding delimiter/null) or -1 on error.
ssize_t read_until(char *buffer, char delimiter, size_t max_len) {
  if (buffer == NULL || max_len == 0) {
    return -1;
  }

  size_t count = 0;
  char current_char;
  int bytes_read_single;

  while (count < max_len - 1) { // Leave space for null terminator
    if (receive(0, &current_char, 1, &bytes_read_single) != 0) {
      return -1; // Receive error
    }
    if (bytes_read_single == 0) { // EOF or no bytes read
      buffer[count] = '\0'; // Null-terminate what was read so far
      return -1; // Original code returned -1 for this case
    }

    if (delimiter == current_char) {
      break;
    }
    buffer[count] = current_char;
    count++;
  }
  buffer[count] = '\0'; // Null-terminate the string
  return count;
}

// Function: PrintAspMenu
void PrintAspMenu(void) {
  char selection_buf[9]; // Max input "1234567\n" -> 8 chars + null
  ssize_t bytes_read;
  byte asp_idx;
  double rate_val;

  while (1) {
    cromu_puts("1. Change aeration percent");
    cromu_puts("2. Change glycerin rate");
    cromu_puts("3. Change alum rate");
    cromu_puts("4. Return to previous menu");
    cromu_printf("Selection: ");

    bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
    if (bytes_read == -1) {
      _terminate();
    }

    if (bytes_read == 1) { // Expecting a single digit input
      switch (selection_buf[0]) {
        case '1':
          cromu_printf("Which ASP 0-%u: ", NumAsp - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            asp_idx = cromu_atoi(selection_buf);
            if (asp_idx < NumAsp) {
              cromu_printf("New aeration percent: ");
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              if (bytes_read < 8) { // Max 7 chars + null for double input
                rate_val = cromu_atof(selection_buf);
                if (0.0 <= rate_val && rate_val <= _DAT_00018610) {
                  ChangeAerationPercent((char *)Asp + asp_idx * STRUCT_SIZE_E4, rate_val);
                }
              }
            }
          }
          break;
        case '2':
          cromu_printf("Which ASP 0-%u: ", NumAsp - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            asp_idx = cromu_atoi(selection_buf);
            if (asp_idx < NumAsp) {
              cromu_printf("New glycerin rate (0-100 gallons/hr): ");
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              if (bytes_read < 8) {
                rate_val = cromu_atof(selection_buf);
                if (0.0 <= rate_val && rate_val <= _DAT_00018610) {
                  ChangeGlycerinRate((char *)Asp + asp_idx * STRUCT_SIZE_E4, rate_val);
                }
              }
            }
          }
          break;
        case '3':
          cromu_printf("Which ASP 0-%u: ", NumAsp - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            asp_idx = cromu_atoi(selection_buf);
            if (asp_idx < NumAsp) {
              cromu_printf("New alum rate (0-100 gallons/hr): ");
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              if (bytes_read < 8) {
                rate_val = cromu_atof(selection_buf);
                if (0.0 <= rate_val && rate_val <= _DAT_00018610) {
                  ChangeAlumRate((char *)Asp + asp_idx * STRUCT_SIZE_E4, rate_val);
                }
              }
            }
          }
          break;
        case '4':
          return; // Exit menu
      }
    }
  }
}

// Function: PrintFilterMenu
void PrintFilterMenu(void) {
  char selection_buf[4]; // Max input "1\n" -> 2 chars + null
  ssize_t bytes_read;
  byte filter_idx;
  byte active_filters_count;

  while (1) {
    cromu_puts("1. Change active filter count");
    cromu_puts("2. Return to previous menu");
    cromu_printf("Selection: ");

    bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
    if (bytes_read == -1) { _terminate(); }

    if (bytes_read == 1) {
      switch (selection_buf[0]) {
        case '1':
          cromu_printf("Which Filter 0-%u: ", NumFilters - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            filter_idx = cromu_atoi(selection_buf);
            // Corrected: used NumFilters instead of NumAsp as per context
            if (filter_idx < NumFilters) {
              // Assuming Filter structure has a field at offset 0x55 for max active filters
              byte max_active_filters = *((byte*)((char*)Filter + filter_idx * STRUCT_SIZE_E4 + 0x55));
              cromu_printf("How many filters (0-%u): ", max_active_filters);
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              if (bytes_read < 7) { // Max 6 chars + null for integer input
                active_filters_count = cromu_atoi(selection_buf);
                if (active_filters_count <= max_active_filters) {
                  ChangeNumActiveFilters((char *)Filter + filter_idx * STRUCT_SIZE_E4, active_filters_count);
                }
              }
            }
          }
          break;
        case '2':
          return; // Exit menu
      }
    }
  }
}

// Function: PrintDisinfectionMenu
void PrintDisinfectionMenu(void) {
  char selection_buf[9]; // Max input "1234567\n" -> 8 chars + null
  ssize_t bytes_read;
  byte disinfection_idx;
  double rate_val;

  while (1) {
    cromu_puts("1. Change chlorine rate");
    cromu_puts("2. Return to previous menu");
    cromu_printf("Selection: ");

    bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
    if (bytes_read == -1) { _terminate(); }

    if (bytes_read == 1) {
      switch (selection_buf[0]) {
        case '1':
          cromu_printf("Which Disinfection system 0-%u: ", NumDisinfection - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            disinfection_idx = cromu_atoi(selection_buf);
            // Corrected: used NumDisinfection instead of NumAsp as per context
            if (disinfection_idx < NumDisinfection) {
              cromu_printf("New chlorine rate (0-100): ");
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              if (bytes_read < 8) {
                rate_val = cromu_atof(selection_buf);
                if (0.0 <= rate_val && rate_val <= _DAT_00018610) {
                  ChangeChlorineRate((char *)Disinfection + disinfection_idx * STRUCT_SIZE_E4, rate_val);
                }
              }
            }
          }
          break;
        case '2':
          return; // Exit menu
      }
    }
  }
}

// Function: PrintRenameMenu
void PrintRenameMenu(void) {
  char selection_buf[MAX_NAME_LEN + 1]; // Max input for names, plus null
  ssize_t bytes_read;
  byte idx;
  size_t copy_len;

  while (1) {
    cromu_puts("1. Influent");
    cromu_puts("2. Headworks");
    cromu_puts("3. ASPs");
    cromu_puts("4. Filters");
    cromu_puts("5. Disinfections");
    cromu_puts("6. Effluent");
    cromu_puts("7. Valves");
    cromu_puts("8. Return to previous menu");
    cromu_printf("Selection: ");

    bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
    if (bytes_read == -1) { _terminate(); }

    if (bytes_read == 1) { // Expecting a single digit input
      switch (selection_buf[0]) {
        case '1':
          cromu_printf("New Influent name: ");
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          copy_len = (bytes_read > MAX_NAME_LEN) ? MAX_NAME_LEN : bytes_read;
          strncpy(DAT_0001ed41, selection_buf, copy_len);
          DAT_0001ed41[copy_len] = '\0';
          break;
        case '2':
          cromu_printf("New Headworks name: ");
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          copy_len = (bytes_read > MAX_NAME_LEN) ? MAX_NAME_LEN : bytes_read;
          strncpy(DAT_0001e241, selection_buf, copy_len);
          DAT_0001e241[copy_len] = '\0';
          break;
        case '3':
          cromu_printf("Which ASP 0-%u: ", NumAsp - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            idx = cromu_atoi(selection_buf);
            if (idx < NumAsp) {
              cromu_printf("New ASP%u name: ", idx);
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              copy_len = (bytes_read > MAX_NAME_LEN) ? MAX_NAME_LEN : bytes_read;
              strncpy((char *)DAT_0001ef41 + idx * STRUCT_SIZE_E4, selection_buf, copy_len);
              ((char *)DAT_0001ef41 + idx * STRUCT_SIZE_E4)[copy_len] = '\0';
            }
          }
          break;
        case '4':
          cromu_printf("Which Filter 0-%u: ", NumFilters - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            idx = cromu_atoi(selection_buf);
            // Corrected: used NumFilters instead of NumAsp as per context
            if (idx < NumFilters) {
              cromu_printf("New Filter%u name: ", idx);
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              copy_len = (bytes_read > MAX_NAME_LEN) ? MAX_NAME_LEN : bytes_read;
              strncpy((char *)DAT_0001e341 + idx * STRUCT_SIZE_E4, selection_buf, copy_len);
              ((char *)DAT_0001e341 + idx * STRUCT_SIZE_E4)[copy_len] = '\0';
            }
          }
          break;
        case '5':
          cromu_printf("Which Disinfection system 0-%u: ", NumDisinfection - 1);
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            idx = cromu_atoi(selection_buf);
            // Corrected: used NumDisinfection instead of NumAsp as per context
            if (idx < NumDisinfection) {
              cromu_printf("New Disinfection%u name: ", idx);
              bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
              if (bytes_read == -1) { _terminate(); }
              copy_len = (bytes_read > MAX_NAME_LEN) ? MAX_NAME_LEN : bytes_read;
              strncpy((char *)DAT_0001e8c1 + idx * STRUCT_SIZE_E4, selection_buf, copy_len);
              ((char *)DAT_0001e8c1 + idx * STRUCT_SIZE_E4)[copy_len] = '\0';
            }
          }
          break;
        case '6':
          cromu_printf("New Effluent name: ");
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          copy_len = (bytes_read > MAX_NAME_LEN) ? MAX_NAME_LEN : bytes_read;
          strncpy(DAT_0001f3c1, selection_buf, copy_len);
          DAT_0001f3c1[copy_len] = '\0';
          break;
        case '7':
          cromu_puts("1. V1");
          cromu_puts("2. V2");
          cromu_puts("3. V3");
          cromu_puts("4. V4");
          cromu_printf("Which valve: ");
          bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
          if (bytes_read == -1) { _terminate(); }
          if (bytes_read == 1) {
            idx = cromu_atoi(selection_buf);

            cromu_printf("New V%u name: ", idx);
            bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
            if (bytes_read == -1) { _terminate(); }

            copy_len = (bytes_read > MAX_NAME_LEN) ? MAX_NAME_LEN : bytes_read;

            switch (idx) {
              case 1:
                strncpy(DAT_0001ee41, selection_buf, copy_len);
                DAT_0001ee41[copy_len] = '\0';
                break;
              case 2:
                strncpy(DAT_0001e041, selection_buf, copy_len);
                DAT_0001e041[copy_len] = '\0';
                break;
              case 3:
                strncpy(DAT_0001e7c1, selection_buf, copy_len);
                DAT_0001e7c1[copy_len] = '\0';
                break;
              case 4:
                // Original had specific (and potentially unsafe) logic for V4.
                // Using strncpy for consistency and safety.
                strncpy(DAT_0001e141, selection_buf, copy_len);
                DAT_0001e141[copy_len] = '\0';
                break;
            }
          }
          break;
        case '8':
          return; // Exit menu
      }
    }
  }
}

// Function: PrintValveMenu
void PrintValveMenu(void) {
  char selection_buf[10]; // Max input for percentage "100.00\n" -> 7 chars + null, plus other inputs
  ssize_t bytes_read;
  byte valve_output_idx;
  char valve_choice_char;
  double percent_open;

  while (1) {
    cromu_puts("1. Change valve V1 output percentage");
    cromu_puts("2. Change valve V2 output percentage");
    cromu_puts("3. Change valve V3 output percentage");
    cromu_puts("4. Change valve V4 output percentage");
    cromu_puts("5. Return to previous menu");
    cromu_printf("Selection: ");

    bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
    if (bytes_read == -1) { _terminate(); }

    if (bytes_read == 1) {
      valve_choice_char = selection_buf[0];
      if (valve_choice_char == '5') {
        return; // Exit menu
      }

      // This prompt seems to ask for an output index for the selected valve.
      // Example: "Valve Output: 1" for V1's first output.
      cromu_printf("Valve Output: ");
      bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
      if (bytes_read == -1) { _terminate(); }

      if (bytes_read == 1) { // Expecting a single digit for output index
        valve_output_idx = selection_buf[0] - '0'; // Convert char '0'-'9' to int 0-9

        cromu_printf("Percent Open (0.00-100.00): ");
        bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
        if (bytes_read == -1) { _terminate(); }

        if (bytes_read < 8) { // Max 7 chars + null for double input
          percent_open = cromu_atof(selection_buf);
          if (0.0 <= percent_open && percent_open <= _DAT_00018610) {
            switch (valve_choice_char) {
              case '1':
                SetValveOutput(HeadworksOutputValve, valve_output_idx, percent_open);
                break;
              case '2':
                SetValveOutput(AspOutputValve, valve_output_idx, percent_open);
                break;
              case '3':
                SetValveOutput(FilterOutputValve, valve_output_idx, percent_open);
                break;
              case '4':
                SetValveOutput(DisinfectionOutputValve, valve_output_idx, percent_open);
                break;
            }
          }
        }
      }
    }
  }
}

// Function: PrintSystemBanner
void PrintSystemBanner(void) {
  cromu_puts("Welcome to the Wastewater Treatement Plant Simulator");
  cromu_puts("###################################################");
  cromu_puts("Your job is to take the plant described in the schematic above and make potable water.");
  cromu_puts("The simulation starts with a random plant status.");
  cromu_puts("Press Enter to continue..."); // Original used &DAT_000184e2, assuming it's a string literal.
  return;
}

// Function: PrintMenu
// Returns 1 to continue simulation, 0 to exit.
int PrintMenu(void) {
  char selection_buf[4]; // Max input "1\n" -> 2 chars + null
  ssize_t bytes_read;

  while (1) {
    cromu_puts("Wastewater Treatment Plant Simulation");
    cromu_printf("Simulation Hour %u\n", SimStepCount);
    cromu_puts("1. Print plant schematic");
    cromu_puts("2. Print plant status");
    cromu_puts("3. Control Activated Sludge Process (ASP)");
    cromu_puts("4. Control Filters");
    cromu_puts("5. Control Disinfection");
    cromu_puts("6. Control Valves");
    cromu_puts("7. Rename System");
    cromu_puts("8. Run Simulation Step");
    cromu_puts("9. Exit");
    cromu_printf("Selection: ");

    bytes_read = read_until(selection_buf, '\n', sizeof(selection_buf));
    if (bytes_read == -1) { _terminate(); }

    if (bytes_read == 1) { // Expecting a single digit input
      switch (selection_buf[0]) {
        case '1':
          PrintPlantSchematic();
          break;
        case '2':
          PrintPlantStatus();
          break;
        case '3':
          PrintAspMenu();
          break;
        case '4':
          PrintFilterMenu();
          break;
        case '5':
          PrintDisinfectionMenu();
          break;
        case '6':
          PrintValveMenu();
          break;
        case '7':
          PrintRenameMenu();
          break;
        case '8':
          return 1; // Signal to run simulation step
        case '9':
          return 0; // Signal to exit
      }
    }
  }
}