#include <stdint.h> // For uint32_t, uint8_t
#include <string.h> // For strcmp, strncmp, strcpy
#include <stddef.h> // For NULL

// --- Memory Simulation ---
// In a real embedded system, 0x4347c000 would be a fixed memory address
// or a pointer to a memory-mapped region. For compilation and execution
// on a standard Linux system, we simulate this memory region using a static array.
#define DATA_BASE_ADDRESS_SIMULATED_SIZE (0x80 * 0x20) // 128 entries * 32 bytes/entry
static uint8_t s_data_memory[DATA_BASE_ADDRESS_SIMULATED_SIZE];

// --- Constants for clarity and maintainability ---
#define N_ENTRIES           0x80 // Number of entries (128)
#define ENTRY_SIZE          0x20 // Size of each entry in bytes (32)
#define NAME_LENGTH         7    // Length of the name string part
#define NAME_OFFSET         0    // Offset of the name within an entry
#define SUM_START_OFFSET    0xe  // Start offset for sum calculation (14)
#define SUM_END_OFFSET      0x1c // End offset for sum calculation (28, exclusive)
#define AUTH_VALUE_OFFSET   0x1c // Offset of the authentication value within an entry (28)
#define AUTH_VALUE_MASK     0x7fffffff // Mask for the authentication value
#define ASCII_MOD_VAL       0x1a // Modulo value for character conversion (26)
#define ASCII_BASE_CHAR     0x61 // ASCII 'a'

// Function: authenticate
uint32_t authenticate(char *param_1, uint32_t param_2) {
    char entry_name[NAME_LENGTH + 1]; // Buffer for the name, +1 for null terminator

    for (uint32_t i = 0; i < N_ENTRIES; ++i) {
        // Calculate the starting address of the current entry in the simulated memory
        const uint8_t* current_entry_ptr = s_data_memory + i * ENTRY_SIZE;

        // Generate the name string from the entry data
        for (int k = 0; k < NAME_LENGTH; ++k) {
            entry_name[k] = *(current_entry_ptr + NAME_OFFSET + k) % ASCII_MOD_VAL + ASCII_BASE_CHAR;
        }
        entry_name[NAME_LENGTH] = '\0'; // Null-terminate the generated string

        // Compare input string with the generated entry name
        if (strncmp(param_1, entry_name, NAME_LENGTH) == 0) {
            uint32_t sum = 0;
            // Calculate sum of bytes within a specific range of the entry
            for (int k = SUM_START_OFFSET; k < SUM_END_OFFSET; ++k) {
                sum += *(current_entry_ptr + k);
            }

            // If the calculated sum matches param_2, return the masked value from the entry
            if (sum == param_2) {
                // Read 4 bytes as uint32_t from the specified offset and apply mask
                return *(uint32_t*)(current_entry_ptr + AUTH_VALUE_OFFSET) & AUTH_VALUE_MASK;
            }
        }
    }

    // If no matching entry is found after iterating through all entries,
    // execute the fallback logic.
    if (strcmp(param_1, "\x02") != 0 || (param_2 > 7 || param_2 < 2) || ((param_2 & 1) != 0)) {
        param_2 = 0; // Set param_2 to 0 if conditions are met
    }
    return param_2; // Return the (possibly modified) param_2
}

// Function: lookupName
uint32_t lookupName(char *param_1, uint32_t param_2) {
    // If the output buffer is NULL, return an error
    if (param_1 == NULL) {
        return 0xffffffff;
    }

    char entry_name[NAME_LENGTH + 1]; // Buffer for the name, +1 for null terminator

    for (uint32_t i = 0; i < N_ENTRIES; ++i) {
        // Calculate the starting address of the current entry in the simulated memory
        const uint8_t* current_entry_ptr = s_data_memory + i * ENTRY_SIZE;

        // Read the authentication value from the entry and apply the mask
        uint32_t entry_auth_value = *(uint32_t*)(current_entry_ptr + AUTH_VALUE_OFFSET) & AUTH_VALUE_MASK;

        // If param_2 matches the entry's authentication value
        if (param_2 == entry_auth_value) {
            // Generate the name string from the entry data
            for (int k = 0; k < NAME_LENGTH; ++k) {
                entry_name[k] = *(current_entry_ptr + NAME_OFFSET + k) % ASCII_MOD_VAL + ASCII_BASE_CHAR;
            }
            entry_name[NAME_LENGTH] = '\0'; // Null-terminate the generated string

            // Copy the generated name to the output buffer
            strcpy(param_1, entry_name);
            return 0; // Success
        }
    }

    // If no matching entry is found after iterating through all entries,
    // handle specific range for param_2.
    if (((int)param_2 < 3) || (7 < (int)param_2)) { // param_2 < 3 or param_2 > 7
        return 0xffffffff; // Return error
    } else { // 3 <= param_2 <= 7
        // This part performs specific string manipulation as per original code.
        // It writes '\x02' at param_1[0], then ensures a null terminator at param_1[1].
        // Then it writes param_2+'0' at param_1[5], and a null terminator at param_1[6].
        // This assumes param_1 is large enough (at least 7 bytes) and intermediate
        // bytes (param_1[2] to param_1[4]) are either irrelevant or handled by the caller.
        param_1[0] = '\x02';
        param_1[1] = '\0';
        param_1[5] = (char)param_2 + '0';
        param_1[6] = '\0';
        return 0; // Success
    }
}