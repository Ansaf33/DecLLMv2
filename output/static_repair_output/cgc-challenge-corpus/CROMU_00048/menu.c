#include <stdio.h>    // For printf, puts (if cromu_ versions map to these)
#include <string.h>   // For strncpy, strcpy
#include <stdlib.h>   // For atoi, atof (if cromu_ versions map to these)
#include <stdint.h>   // For uint32_t, uint8_t
#include <stdarg.h>   // For va_list in cromu_printf mock

// Decompiled type aliases
typedef unsigned int uint;
typedef unsigned char byte;
// Assuming longdouble maps to double for simplicity as it's assigned to double
typedef double longdouble;

// Mock external functions (replace with actual implementations if available)
void cromu_puts(const char* s) {
    puts(s);
}

// cromu_printf arguments are often set on stack. Simplification: assume it takes format string and variable args like printf.
void cromu_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

int cromu_atoi(const char* s) {
    return atoi(s);
}

double cromu_atof(const char* s) {
    return atof(s);
}

void _terminate() {
    cromu_puts("Program terminated due to error.");
    exit(1); // Standard way to terminate on error
}

// Mock receive function for standard input
int receive(int fd, char* buf, size_t count, int* bytes_received) {
    if (fd != 0) { // Assuming fd 0 is stdin
        *bytes_received = 0;
        return -1; // Error: invalid file descriptor
    }
    if (count == 0) {
        *bytes_received = 0;
        return 0;
    }

    int c = getchar();
    if (c == EOF) {
        *bytes_received = 0;
        return -1; // Indicate EOF or error
    }
    buf[0] = (char)c;
    *bytes_received = 1;
    return 0; // Success
}

// Plant component structures inferred from usage (0xe4 = 228 bytes)
typedef struct {
    char name[32]; // Inferred from 0x1f (31) + 1 for null terminator
    double aeration_percent; // Used in ChangeAerationPercent
    double glycerin_rate;    // Used in ChangeGlycerinRate
    double alum_rate;        // Used in ChangeAlumRate
    char _padding[228 - 32 - (3 * sizeof(double))]; // Fill to 0xe4 bytes
} PlantComponent;

typedef struct {
    char name[32]; // Similar naming pattern
    byte num_active_filters;
    byte total_filter_slots; // Inferred from `Filter[(uint)local_11 * 0xe4 + 0x55]` being a max value
    char _padding[228 - 32 - (2 * sizeof(byte))]; // Fill to 0xe4 bytes
} FilterComponent;

typedef struct {
    char name[32]; // Similar naming pattern
    double chlorine_rate; // Used in ChangeChlorineRate
    char _padding[228 - 32 - sizeof(double)]; // Fill to 0xe4 bytes
} DisinfectionComponent;

typedef struct {
    char name[32]; // Valve names
    double output_percentage; // Used in SetValveOutput
} ValveComponent;

// Global variables (mocked values)
byte NumAsp = 2; // Example: 2 ASPs
byte NumFilters = 3; // Example: 3 Filters
byte NumDisinfection = 1; // Example: 1 Disinfection system
int SimStepCount = 0;

// Placeholder for the arrays, assuming they are dynamically allocated or large global arrays.
static PlantComponent Asp[10]; // Assuming max 10 ASPs
static FilterComponent Filter[10]; // Assuming max 10 Filters
static DisinfectionComponent Disinfection[10]; // Assuming max 10 Disinfection systems

// Global name storage (inferred from DAT_0001XXXX addresses)
char DAT_0001ed41[32] = "Influent";
char DAT_0001e241[32] = "Headworks";
char DAT_0001f3c1[32] = "Effluent";
char DAT_0001ee41[32] = "V1";
char DAT_0001e041[32] = "V2";
char DAT_0001e7c1[32] = "V3";
char DAT_0001e141[32] = "V4";

// Global constant, likely 100.0 for percentages
const double _DAT_00018610 = 100.0; // Max rate/percent

// Global valve components
ValveComponent HeadworksOutputValve;
ValveComponent AspOutputValve;
ValveComponent FilterOutputValve;
ValveComponent DisinfectionOutputValve;

// Mock change functions
void ChangeAerationPercent(PlantComponent* asp, double percent) {
    if (asp) {
        printf("Changing ASP aeration percent for %s to %.2f%%\n", asp->name, percent);
        asp->aeration_percent = percent;
    }
}
void ChangeGlycerinRate(PlantComponent* asp, double rate) {
    if (asp) {
        printf("Changing ASP glycerin rate for %s to %.2f gallons/hr\n", asp->name, rate);
        asp->glycerin_rate = rate;
    }
}
void ChangeAlumRate(PlantComponent* asp, double rate) {
    if (asp) {
        printf("Changing ASP alum rate for %s to %.2f gallons/hr\n", asp->name, rate);
        asp->alum_rate = rate;
    }
}
void ChangeNumActiveFilters(FilterComponent* filter, uint count) {
    if (filter) {
        printf("Changing filter active count for %s to %u\n", filter->name, count);
        filter->num_active_filters = (byte)count;
    }
}
void ChangeChlorineRate(DisinfectionComponent* disinfection, double rate) {
    if (disinfection) {
        printf("Changing disinfection chlorine rate for %s to %.2f\n", disinfection->name, rate);
        disinfection->chlorine_rate = rate;
    }
}
void SetValveOutput(ValveComponent* valve, uint valve_idx, double percent) {
    if (valve) {
        // valve_idx is likely just an identifier in this context, not an array index
        printf("Setting valve %s (output channel %u) to %.2f%%\n", valve->name, valve_idx, percent);
        valve->output_percentage = percent;
    }
}

// Mock print functions
void PrintPlantSchematic() {
    cromu_puts("--- Plant Schematic ---");
    cromu_puts("Influent -> Headworks -> ASPs -> Filters -> Disinfections -> Effluent");
    cromu_puts("Valves: V1 (Headworks), V2 (ASPs), V3 (Filters), V4 (Disinfections)");
    cromu_puts("-----------------------");
}

void PrintPlantStatus() {
    cromu_puts("--- Plant Status ---");
    cromu_printf("Influent Name: %s\n", DAT_0001ed41);
    cromu_printf("Headworks Name: %s\n", DAT_0001e241);
    for (int i = 0; i < NumAsp; ++i) {
        cromu_printf("ASP %d Name: %s, Aeration: %.2f%%, Glycerin: %.2f gph, Alum: %.2f gph\n",
                     i, Asp[i].name, Asp[i].aeration_percent, Asp[i].glycerin_rate, Asp[i].alum_rate);
    }
    for (int i = 0; i < NumFilters; ++i) {
        cromu_printf("Filter %d Name: %s, Active Filters: %u/%u\n",
                     i, Filter[i].name, Filter[i].num_active_filters, Filter[i].total_filter_slots);
    }
    for (int i = 0; i < NumDisinfection; ++i) {
        cromu_printf("Disinfection %d Name: %s, Chlorine Rate: %.2f\n",
                     i, Disinfection[i].name, Disinfection[i].chlorine_rate);
    }
    cromu_printf("Effluent Name: %s\n", DAT_0001f3c1);
    cromu_printf("Valve V1 (%s) Output: %.2f%%\n", DAT_0001ee41, HeadworksOutputValve.output_percentage);
    cromu_printf("Valve V2 (%s) Output: %.2f%%\n", DAT_0001e041, AspOutputValve.output_percentage);
    cromu_printf("Valve V3 (%s) Output: %.2f%%\n", DAT_0001e7c1, FilterOutputValve.output_percentage);
    cromu_printf("Valve V4 (%s) Output: %.2f%%\n", DAT_0001e141, DisinfectionOutputValve.output_percentage);
    cromu_puts("--------------------");
}

void RunSimulationStep() {
    cromu_puts("Running simulation step...");
    SimStepCount++;
    // Simulate some changes or checks here based on current plant settings
    cromu_puts("Simulation step complete.");
}

// Function: read_until
uint read_until(char *buffer, char delimiter, size_t max_len) {
    uint bytes_read = 0;
    char c;
    int received_bytes_count;

    if (!buffer) {
        return (uint)-1; // Indicate error
    }

    // Read characters until delimiter, max_len-1 reached, or error
    while (bytes_read < max_len - 1) {
        if (receive(0, &c, 1, &received_bytes_count) != 0) {
            return (uint)-1; // Receive error
        }
        if (received_bytes_count == 0) {
            return (uint)-1; // No bytes received (EOF or error)
        }
        if (c == delimiter) {
            break; // Delimiter found
        }
        buffer[bytes_read] = c;
        bytes_read++;
    }

    // Null-terminate the buffer
    buffer[bytes_read] = '\0';
    return bytes_read;
}

// Function: PrintAspMenu
void PrintAspMenu(void) {
    char selection_buf[9]; // For 'local_2d', max_len 8 + null
    int bytes_read;
    byte asp_idx;
    double rate_val;

    while (1) {
        cromu_puts("1. Change aeration percent");
        cromu_puts("2. Change glycerin rate");
        cromu_puts("3. Change alum rate");
        cromu_puts("4. Return to previous menu");
        cromu_printf("Selection: ");

        bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
        if (bytes_read == (uint)-1) {
            _terminate();
        }

        if (bytes_read == 1) {
            if (selection_buf[0] == '4') {
                break; // Return to previous menu
            }

            if (selection_buf[0] == '1' || selection_buf[0] == '2' || selection_buf[0] == '3') {
                cromu_printf("Which ASP 0-%d: ", NumAsp - 1);
                bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
                if (bytes_read == (uint)-1) {
                    _terminate();
                }

                if (bytes_read == 1) {
                    asp_idx = cromu_atoi(selection_buf);
                    if (asp_idx < NumAsp) {
                        if (selection_buf[0] == '1') { // Change aeration percent
                            cromu_printf("New aeration percent (0-100): ");
                            bytes_read = read_until(selection_buf, '\n', 8); // Max 7 chars + null
                            if (bytes_read == (uint)-1) {
                                _terminate();
                            }
                            rate_val = cromu_atof(selection_buf);
                            if ((0.0 <= rate_val) && (rate_val <= _DAT_00018610)) {
                                ChangeAerationPercent(&Asp[asp_idx], rate_val);
                            } else {
                                cromu_puts("Invalid aeration percent. Must be 0-100.");
                            }
                        } else if (selection_buf[0] == '2') { // Change glycerin rate
                            cromu_printf("New glycerin rate (0-100 gallons/hr): ");
                            bytes_read = read_until(selection_buf, '\n', 8); // Max 7 chars + null
                            if (bytes_read == (uint)-1) {
                                _terminate();
                            }
                            rate_val = cromu_atof(selection_buf);
                            if ((0.0 <= rate_val) && (rate_val <= _DAT_00018610)) {
                                ChangeGlycerinRate(&Asp[asp_idx], rate_val);
                            } else {
                                cromu_puts("Invalid glycerin rate. Must be 0-100.");
                            }
                        } else if (selection_buf[0] == '3') { // Change alum rate
                            cromu_printf("New alum rate (0-100 gallons/hr): ");
                            bytes_read = read_until(selection_buf, '\n', 8); // Max 7 chars + null
                            if (bytes_read == (uint)-1) {
                                _terminate();
                            }
                            rate_val = cromu_atof(selection_buf);
                            if ((0.0 <= rate_val) && (rate_val <= _DAT_00018610)) {
                                ChangeAlumRate(&Asp[asp_idx], rate_val);
                            } else {
                                cromu_puts("Invalid alum rate. Must be 0-100.");
                            }
                        }
                    } else {
                        cromu_puts("Invalid ASP index.");
                    }
                } else {
                    cromu_puts("Invalid input for ASP index.");
                }
            } else {
                cromu_puts("Invalid selection.");
            }
        } else {
            cromu_puts("Invalid input for menu selection.");
        }
    }
}

// Function: PrintFilterMenu
void PrintFilterMenu(void) {
    char selection_buf[8]; // For 'local_16', max_len 7 + null
    int bytes_read;
    byte filter_idx;
    byte num_filters_to_set;

    while (1) {
        cromu_puts("1. Change active filter count");
        cromu_puts("2. Return to previous menu");
        cromu_printf("Selection: ");

        bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
        if (bytes_read == (uint)-1) {
            _terminate();
        }

        if (bytes_read == 1) {
            if (selection_buf[0] == '2') {
                break; // Return to previous menu
            } else if (selection_buf[0] == '1') {
                cromu_printf("Which Filter 0-%d: ", NumFilters - 1);
                bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
                if (bytes_read == (uint)-1) {
                    _terminate();
                }

                if (bytes_read == 1) {
                    filter_idx = cromu_atoi(selection_buf);
                    if (filter_idx < NumFilters) {
                        byte filter_max_capacity = Filter[filter_idx].total_filter_slots;
                        cromu_printf("How many filters (0-%d): ", filter_max_capacity);
                        bytes_read = read_until(selection_buf, '\n', 7); // Max 6 chars + null
                        if (bytes_read == (uint)-1) {
                            _terminate();
                        }
                        num_filters_to_set = cromu_atoi(selection_buf);
                        if (num_filters_to_set <= filter_max_capacity) {
                            ChangeNumActiveFilters(&Filter[filter_idx], num_filters_to_set);
                        } else {
                            cromu_printf("Invalid number of filters. Must be 0-%d.\n", filter_max_capacity);
                        }
                    } else {
                        cromu_puts("Invalid Filter index.");
                    }
                } else {
                    cromu_puts("Invalid input for Filter index.");
                }
            } else {
                cromu_puts("Invalid selection.");
            }
        } else {
            cromu_puts("Invalid input for menu selection.");
        }
    }
}

// Function: PrintDisinfectionMenu
void PrintDisinfectionMenu(void) {
    char selection_buf[9]; // For 'local_25', max_len 8 + null
    int bytes_read;
    byte disinfection_idx;
    double rate_val;

    while (1) {
        cromu_puts("1. Change chlorine rate");
        cromu_puts("2. Return to previous menu");
        cromu_printf("Selection: ");

        bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
        if (bytes_read == (uint)-1) {
            _terminate();
        }

        if (bytes_read == 1) {
            if (selection_buf[0] == '2') {
                break; // Return to previous menu
            } else if (selection_buf[0] == '1') {
                cromu_printf("Which Disinfection system 0-%d: ", NumDisinfection - 1);
                bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
                if (bytes_read == (uint)-1) {
                    _terminate();
                }

                if (bytes_read == 1) {
                    disinfection_idx = cromu_atoi(selection_buf);
                    if (disinfection_idx < NumDisinfection) {
                        cromu_printf("New chlorine rate (0-100): ");
                        bytes_read = read_until(selection_buf, '\n', 8); // Max 7 chars + null
                        if (bytes_read == (uint)-1) {
                            _terminate();
                        }
                        rate_val = cromu_atof(selection_buf);
                        if ((0.0 <= rate_val) && (rate_val <= _DAT_00018610)) {
                            ChangeChlorineRate(&Disinfection[disinfection_idx], rate_val);
                        } else {
                            cromu_puts("Invalid chlorine rate. Must be 0-100.");
                        }
                    } else {
                        cromu_puts("Invalid Disinfection system index.");
                    }
                } else {
                    cromu_puts("Invalid input for Disinfection system index.");
                }
            } else {
                cromu_puts("Invalid selection.");
            }
        } else {
            cromu_puts("Invalid input for menu selection.");
        }
    }
}

// Function: PrintRenameMenu
void PrintRenameMenu(void) {
    char input_buf[256]; // For 'local_10e', max_len 255 + null
    int bytes_read;
    byte idx;

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

        bytes_read = read_until(input_buf, '\n', 3); // Max 2 chars + null
        if (bytes_read == (uint)-1) {
            _terminate();
        }

        if (bytes_read == 1) {
            switch (input_buf[0]) {
                case '1': // Influent
                    cromu_printf("New Influent name: ");
                    bytes_read = read_until(input_buf, '\n', sizeof(DAT_0001ed41));
                    if (bytes_read == (uint)-1) {
                        _terminate();
                    }
                    strncpy(DAT_0001ed41, input_buf, bytes_read);
                    DAT_0001ed41[bytes_read] = '\0'; // Ensure null termination
                    break;
                case '2': // Headworks
                    cromu_printf("New Headworks name: ");
                    bytes_read = read_until(input_buf, '\n', sizeof(DAT_0001e241));
                    if (bytes_read == (uint)-1) {
                        _terminate();
                    }
                    strncpy(DAT_0001e241, input_buf, bytes_read);
                    DAT_0001e241[bytes_read] = '\0';
                    break;
                case '3': // ASPs
                    cromu_printf("Which ASP 0-%d: ", NumAsp - 1);
                    bytes_read = read_until(input_buf, '\n', 3); // Max 2 chars + null
                    if (bytes_read == (uint)-1) {
                        _terminate();
                    }
                    if (bytes_read == 1) {
                        idx = cromu_atoi(input_buf);
                        if (idx < NumAsp) {
                            cromu_printf("New ASP%d name: ", idx);
                            bytes_read = read_until(input_buf, '\n', sizeof(Asp[idx].name));
                            if (bytes_read == (uint)-1) {
                                _terminate();
                            }
                            strncpy(Asp[idx].name, input_buf, bytes_read);
                            Asp[idx].name[bytes_read] = '\0';
                        } else {
                            cromu_puts("Invalid ASP index.");
                        }
                    } else {
                        cromu_puts("Invalid input for ASP index.");
                    }
                    break;
                case '4': // Filters
                    cromu_printf("Which Filter 0-%d: ", NumFilters - 1);
                    bytes_read = read_until(input_buf, '\n', 3); // Max 2 chars + null
                    if (bytes_read == (uint)-1) {
                        _terminate();
                    }
                    if (bytes_read == 1) {
                        idx = cromu_atoi(input_buf);
                        if (idx < NumFilters) {
                            cromu_printf("New Filter%d name: ", idx);
                            bytes_read = read_until(input_buf, '\n', sizeof(Filter[idx].name));
                            if (bytes_read == (uint)-1) {
                                _terminate();
                            }
                            strncpy(Filter[idx].name, input_buf, bytes_read);
                            Filter[idx].name[bytes_read] = '\0';
                        } else {
                            cromu_puts("Invalid Filter index.");
                        }
                    } else {
                        cromu_puts("Invalid input for Filter index.");
                    }
                    break;
                case '5': // Disinfections
                    cromu_printf("Which Disinfection system 0-%d: ", NumDisinfection - 1);
                    bytes_read = read_until(input_buf, '\n', 3); // Max 2 chars + null
                    if (bytes_read == (uint)-1) {
                        _terminate();
                    }
                    if (bytes_read == 1) {
                        idx = cromu_atoi(input_buf);
                        if (idx < NumDisinfection) {
                            cromu_printf("New Disinfection%d name: ", idx);
                            bytes_read = read_until(input_buf, '\n', sizeof(Disinfection[idx].name));
                            if (bytes_read == (uint)-1) {
                                _terminate();
                            }
                            strncpy(Disinfection[idx].name, input_buf, bytes_read);
                            Disinfection[idx].name[bytes_read] = '\0';
                        } else {
                            cromu_puts("Invalid Disinfection system index.");
                        }
                    } else {
                        cromu_puts("Invalid input for Disinfection system index.");
                    }
                    break;
                case '6': // Effluent
                    cromu_printf("New Effluent name: ");
                    bytes_read = read_until(input_buf, '\n', sizeof(DAT_0001f3c1));
                    if (bytes_read == (uint)-1) {
                        _terminate();
                    }
                    strncpy(DAT_0001f3c1, input_buf, bytes_read);
                    DAT_0001f3c1[bytes_read] = '\0';
                    break;
                case '7': // Valves
                    cromu_puts("1. V1");
                    cromu_puts("2. V2");
                    cromu_puts("3. V3");
                    cromu_puts("4. V4");
                    cromu_printf("Which valve: ");
                    bytes_read = read_until(input_buf, '\n', 3); // Max 2 chars + null
                    if (bytes_read == (uint)-1) {
                        _terminate();
                    }
                    if (bytes_read == 1) {
                        idx = cromu_atoi(input_buf);
                        char *valve_name_ptr = NULL;
                        const char *prompt = NULL;
                        size_t name_buf_size = 0;

                        switch (idx) {
                            case 1: valve_name_ptr = DAT_0001ee41; prompt = "New V1 name: "; name_buf_size = sizeof(DAT_0001ee41); break;
                            case 2: valve_name_ptr = DAT_0001e041; prompt = "New V2 name: "; name_buf_size = sizeof(DAT_0001e041); break;
                            case 3: valve_name_ptr = DAT_0001e7c1; prompt = "New V3 name: "; name_buf_size = sizeof(DAT_0001e7c1); break;
                            case 4: valve_name_ptr = DAT_0001e141; prompt = "New V4 name: "; name_buf_size = sizeof(DAT_0001e141); break;
                            default: cromu_puts("Invalid Valve index."); continue; // Go to next iteration of while loop
                        }
                        
                        cromu_printf(prompt);
                        bytes_read = read_until(input_buf, '\n', name_buf_size);
                        if (bytes_read == (uint)-1) {
                            _terminate();
                        }
                        
                        if (idx == 4) { // Special handling for V4 based on original code
                            size_t len = strlen(input_buf);
                            if (len < name_buf_size - 1) { // Original was `len < 31`
                                strcpy(valve_name_ptr, input_buf);
                            } else {
                                cromu_puts("Valve name too long, truncating.");
                                strncpy(valve_name_ptr, input_buf, name_buf_size - 1);
                                valve_name_ptr[name_buf_size - 1] = '\0';
                            }
                        } else {
                            strncpy(valve_name_ptr, input_buf, bytes_read);
                            valve_name_ptr[bytes_read] = '\0';
                        }
                    } else {
                        cromu_puts("Invalid input for Valve index.");
                    }
                    break;
                case '8': // Return to previous menu
                    return;
                default:
                    cromu_puts("Invalid selection.");
                    break;
            }
        } else {
            cromu_puts("Invalid input for menu selection.");
        }
    }
}

// Function: PrintValveMenu
void PrintValveMenu(void) {
    char selection_buf[10]; // For 'local_26', max_len 9 + null
    int bytes_read;
    char valve_choice_char;
    uint valve_output_channel = 1; // Assuming a single output channel for simplicity if not specified by user
    double percent_open;

    while (1) {
        cromu_puts("1. Change valve V1 output percentage");
        cromu_puts("2. Change valve V2 output percentage");
        cromu_puts("3. Change valve V3 output percentage");
        cromu_puts("4. Change valve V4 output percentage");
        cromu_puts("5. Return to previous menu");
        cromu_printf("Selection: ");

        bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
        if (bytes_read == (uint)-1) {
            _terminate();
        }

        if (bytes_read == 1) {
            valve_choice_char = selection_buf[0];
            if (valve_choice_char == '5') {
                break; // Return to previous menu
            }

            // The "Valve Output: " prompt in original code is confusing.
            // It reads a single digit and uses it as `valve_idx` in `SetValveOutput`.
            // For now, I'll keep the prompt and pass the result as `valve_output_channel`.
            cromu_printf("Valve Output (e.g., '1' for primary output): "); // Clarified prompt
            bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null
            if (bytes_read == (uint)-1) {
                _terminate();
            }
            if (bytes_read == 1) {
                valve_output_channel = cromu_atoi(selection_buf); // Convert char to int
            } else {
                cromu_puts("Invalid input for Valve Output, defaulting to 1.");
                valve_output_channel = 1;
            }

            cromu_printf("Percent Open (0.00-100.00): ");
            bytes_read = read_until(selection_buf, '\n', 10); // Max 9 chars + null (e.g., "100.00\n")
            if (bytes_read == (uint)-1) {
                _terminate();
            }

            if (bytes_read > 0) { // Check if any characters were read for the percentage
                percent_open = cromu_atof(selection_buf);

                if ((0.0 <= percent_open) && (percent_open <= _DAT_00018610)) {
                    switch (valve_choice_char) {
                        case '1':
                            SetValveOutput(&HeadworksOutputValve, valve_output_channel, percent_open);
                            break;
                        case '2':
                            SetValveOutput(&AspOutputValve, valve_output_channel, percent_open);
                            break;
                        case '3':
                            SetValveOutput(&FilterOutputValve, valve_output_channel, percent_open);
                            break;
                        case '4':
                            SetValveOutput(&DisinfectionOutputValve, valve_output_channel, percent_open);
                            break;
                        default:
                            cromu_puts("Invalid valve selection.");
                            break;
                    }
                } else {
                    cromu_puts("Invalid percentage. Must be 0.00-100.00.");
                }
            } else {
                cromu_puts("Invalid input for percentage.");
            }
        } else {
            cromu_puts("Invalid input for menu selection.");
        }
    }
}

// Function: PrintSystemBanner
void PrintSystemBanner(void) {
    cromu_puts("Welcome to the Wastewater Treatement Plant Simulator");
    cromu_puts("###################################################");
    cromu_puts("Your job is to take the plant described in the schematic above and make potable water.");
    cromu_puts("The simulation starts with a random plant status.");
    cromu_puts("Good luck, Engineer!"); // Assuming DAT_000184e2 is this string or similar.
    return;
}

// Function: PrintMenu
uint PrintMenu(void) {
    char selection_buf[4]; // For 'local_14', max_len 3 + null
    int bytes_read;

    while (1) {
        cromu_puts("\nWastewater Treatment Plant Simulation");
        cromu_printf("Simulation Hour %d\n", SimStepCount);
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

        bytes_read = read_until(selection_buf, '\n', 3); // Max 2 chars + null for single digit input
        if (bytes_read == (uint)-1) {
            _terminate();
        }

        if (bytes_read == 1) { // Only process if exactly one character was entered
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
                    return 1; // Run simulation step and continue
                case '9':
                    return 0; // Exit
                default:
                    cromu_puts("Invalid selection.");
                    break;
            }
        } else {
            cromu_puts("Invalid input for menu selection.");
        }
    }
}

// Main function to run the simulation
int main() {
    // Initialize plant components with default names and values
    for (int i = 0; i < NumAsp; ++i) {
        snprintf(Asp[i].name, sizeof(Asp[i].name), "ASP%d", i);
        Asp[i].aeration_percent = 50.0;
        Asp[i].glycerin_rate = 10.0;
        Asp[i].alum_rate = 5.0;
    }
    for (int i = 0; i < NumFilters; ++i) {
        snprintf(Filter[i].name, sizeof(Filter[i].name), "Filter%d", i);
        Filter[i].num_active_filters = 3;
        Filter[i].total_filter_slots = 5; // Example capacity
    }
    for (int i = 0; i < NumDisinfection; ++i) {
        snprintf(Disinfection[i].name, sizeof(Disinfection[i].name), "Disinfection%d", i);
        Disinfection[i].chlorine_rate = 20.0;
    }

    // Initialize valve components names from global string storage
    strncpy(HeadworksOutputValve.name, DAT_0001ee41, sizeof(HeadworksOutputValve.name) - 1); HeadworksOutputValve.name[sizeof(HeadworksOutputValve.name) - 1] = '\0';
    HeadworksOutputValve.output_percentage = 50.0;
    strncpy(AspOutputValve.name, DAT_0001e041, sizeof(AspOutputValve.name) - 1); AspOutputValve.name[sizeof(AspOutputValve.name) - 1] = '\0';
    AspOutputValve.output_percentage = 50.0;
    strncpy(FilterOutputValve.name, DAT_0001e7c1, sizeof(FilterOutputValve.name) - 1); FilterOutputValve.name[sizeof(FilterOutputValve.name) - 1] = '\0';
    FilterOutputValve.output_percentage = 50.0;
    strncpy(DisinfectionOutputValve.name, DAT_0001e141, sizeof(DisinfectionOutputValve.name) - 1); DisinfectionOutputValve.name[sizeof(DisinfectionOutputValve.name) - 1] = '\0';
    DisinfectionOutputValve.output_percentage = 50.0;

    PrintSystemBanner();

    uint continue_simulation = 1;
    while (continue_simulation) {
        continue_simulation = PrintMenu();
        if (continue_simulation == 1) { // Only run step if menu returned 1 (Run Simulation Step)
            RunSimulationStep();
        }
    }

    cromu_puts("Exiting Wastewater Treatment Plant Simulator. Goodbye!");
    return 0;
}