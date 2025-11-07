#include <string.h> // For strcmp, strncmp, strcpy, memcpy
#include <stdio.h>  // For NULL

// The original code accesses a hardcoded memory address 0x4347c000.
// For a Linux-compilable C program, this memory must be defined.
// We'll simulate this with a static const array.
// The data block is 0x80 records * 0x20 bytes/record = 0x1000 bytes.
static const unsigned char g_data_block[0x1000] = {
    // Example data for the first record (0x20 bytes) to allow testing functionality.
    // Name: "test1" (first 7 bytes)
    't', 'e', 's', 't', '1', '\0', '\0',
    // Padding (7 bytes)
    0, 0, 0, 0, 0, 0, 0,
    // Bytes for sum (0xE to 0x1B, total 14 bytes). Sum = 14 * 10 = 140.
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    // ID (0x1C to 0x1F, 4 bytes). Example: 0x04030201 (little-endian)
    0x01, 0x02, 0x03, 0x04,
    // The rest of the 0x1000 bytes are implicitly zero-initialized by C standard.
};

// Function: authenticate
// Original signature: uint authenticate(char *param_1,uint param_2)
unsigned int authenticate(char *param_1, unsigned int param_2) {
    char name_buffer[8]; // Buffer for 7 characters + null terminator
    unsigned int sum_bytes;
    unsigned int i; // Loop counter for records
    int j;          // Loop counter for characters/bytes within a record

    // Iterate through 0x80 records
    for (i = 0; i < 0x80; ++i) {
        unsigned int current_record_offset = i * 0x20; // Each record is 0x20 bytes long

        // Generate a 7-character name string from the data block
        for (j = 0; j < 7; ++j) {
            // Character generation: (byte_value % 26) + 'a'
            name_buffer[j] = (g_data_block[current_record_offset + j] % 0x1a) + 0x61;
        }
        name_buffer[7] = '\0'; // Null-terminate the generated string

        // Compare the input string (param_1) with the generated name
        if (strncmp(param_1, name_buffer, 7) == 0) {
            sum_bytes = 0;
            // Sum bytes from offset + 0xe to offset + 0x1b (14 bytes total)
            for (j = 0xe; j < 0x1c; ++j) {
                sum_bytes += g_data_block[current_record_offset + j];
            }

            // If the calculated sum matches param_2
            if (sum_bytes == param_2) {
                unsigned int id_value;
                // Read a 4-byte integer (ID) from offset + 0x1c.
                // Using memcpy to handle potential alignment issues safely.
                memcpy(&id_value, g_data_block + current_record_offset + 0x1c, sizeof(unsigned int));
                // Return the ID value masked with 0x7fffffff
                return id_value & 0x7fffffff;
            }
        }
    }

    // If no matching record was found after iterating through all 0x80 records
    // Apply a fallback authentication logic
    // Check if param_1 is not "\x02", OR param_2 is out of range [2, 7], OR param_2 is odd.
    if ((strcmp(param_1, "\x02") != 0) || (param_2 > 7 || param_2 < 2) || ((param_2 & 1) != 0)) {
        param_2 = 0; // Set param_2 to 0 if any of the conditions are true
    }
    return param_2; // Return the (possibly modified) param_2
}

// Function: lookupName
// Original signature: undefined4 lookupName(char *param_1,uint param_2)
int lookupName(char *param_1, unsigned int param_2) {
    // Check for null pointer input
    if (param_1 == NULL) {
        return -1; // Return -1 (equivalent to 0xffffffff for a 32-bit signed int)
    }

    char name_buffer[8]; // Buffer for 7 characters + null terminator
    unsigned int i;       // Loop counter for records
    int j;                // Loop counter for characters/bytes within a record

    // Iterate through 0x80 records
    for (i = 0; i < 0x80; ++i) {
        unsigned int current_record_offset = i * 0x20; // Each record is 0x20 bytes long

        unsigned int id_value;
        // Read a 4-byte integer (ID) from offset + 0x1c.
        // Using memcpy for safe alignment handling.
        memcpy(&id_value, g_data_block + current_record_offset + 0x1c, sizeof(unsigned int));
        id_value &= 0x7fffffff; // Mask the ID value

        // If the masked ID matches param_2
        if (param_2 == id_value) {
            // Generate a 7-character name string from the data block
            for (j = 0; j < 7; ++j) {
                name_buffer[j] = (g_data_block[current_record_offset + j] % 0x1a) + 0x61;
            }
            name_buffer[7] = '\0'; // Null-terminate the generated string

            // Copy the generated name to param_1
            strcpy(param_1, name_buffer);
            return 0; // Success
        }
    }

    // If no matching record was found
    // Check if param_2 is out of range [3, 7]
    if (((int)param_2 < 3) || (7 < (int)param_2)) {
        return -1; // Return -1 if out of range
    } else {
        // This is a peculiar fallback logic from the original snippet.
        // It writes "\x02" to param_1, then overwrites param_1[5] with (param_2 + '0')
        // and param_1[6] with '\0'.
        strcpy(param_1, "\x02");
        param_1[5] = (char)param_2 + '0';
        param_1[6] = '\0';
        return 0; // Success
    }
}