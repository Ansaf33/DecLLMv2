#include <stdio.h>    // For printf, fprintf
#include <stdlib.h>   // For exit
#include <stdint.h>   // For uint16_t, uint32_t, uintptr_t
#include <string.h>   // For memcpy

// Type definitions from the original snippet (mapped to standard C types)
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint32_t undefined4; // Assuming 4-byte undefined

// External function declarations (assuming these are defined elsewhere)
// These functions are likely for endianness conversion.
// They are assumed to take a value and return the byte-swapped value.
extern ushort swap_short(ushort val);
extern undefined4 swap_word(undefined4 val);

// External functions for printing tag and type descriptions.
// The original code implies they take their arguments implicitly from stack,
// but for standard C, explicit arguments are necessary.
extern void print_xif_tag_text(ushort tag);
extern void print_type(ushort type);

// Function: process_xif_ifd
// param_1: Base pointer to the IFD data (array of ushorts)
// param_2: Original unused parameter, kept for ABI compatibility if necessary
// param_3: Allocated size of the buffer pointed to by param_1 (in bytes)
// param_4: End address of the valid data within the buffer (exclusive end pointer)
void process_xif_ifd(ushort *param_1, undefined4 param_2, uint param_3, int param_4) {
    uint data_len = (uint)((uintptr_t)param_4 - (uintptr_t)param_1); // Total length of valid data in bytes

    // Check if the allocated buffer size (param_3) is less than the actual data length (data_len)
    if (param_3 < data_len) {
        fprintf(stderr, "Error: Buffer too small for the indicated data length.\n");
        exit(1);
    }

    // The first ushort at param_1[0] contains the count of IFD entries.
    ushort ifd_count = swap_short(param_1[0]);

    // Calculate the minimum required data length for the count (2 bytes)
    // plus all IFD entries (each 12 bytes).
    uint required_len = (uint)ifd_count * 12 + sizeof(ushort);

    if (data_len < required_len) {
        fprintf(stderr, "Invalid IFD count value or actual data length is insufficient.\n");
        exit(1);
    }

    printf("# of arrays: %d\n", ifd_count);

    // Loop through each IFD entry
    for (int i = 0; i < ifd_count; i++) {
        // Each IFD entry is 6 ushorts (12 bytes) long.
        // Entries start immediately after the initial count ushort (param_1[0]).
        // So, the i-th entry starts at param_1[1 + i * 6].
        const ushort *current_entry_ptr = param_1 + 1 + i * 6;

        ushort tag = swap_short(current_entry_ptr[0]);
        ushort type = swap_short(current_entry_ptr[1]);

        uint32_t raw_count_val;
        // Safely copy 4 bytes (uint32_t) from the two ushorts representing count
        // This handles potential unaligned access and strict aliasing rules.
        memcpy(&raw_count_val, &current_entry_ptr[2], sizeof(uint32_t));
        uint32_t count_val = swap_word(raw_count_val);

        uint32_t raw_value_offset;
        // Safely copy 4 bytes (uint32_t) from the two ushorts representing value/offset
        memcpy(&raw_value_offset, &current_entry_ptr[4], sizeof(uint32_t));
        uint32_t value_offset = swap_word(raw_value_offset);

        printf("Tag: %x (", tag);
        print_xif_tag_text(tag);
        printf(")\n"); // Replaced DAT_00016a54 with literal string ")\n"

        printf("Type: %x (", type);
        print_type(type);
        printf(")\n"); // Replaced DAT_00016a54 with literal string ")\n"

        printf("Count: %d\n", count_val);

        // Type 2 typically indicates an ASCII string
        if (type == 2) {
            // value_offset is an offset in bytes from the beginning of param_1,
            // representing a pointer to the string data.
            printf("Value: %s\n", (char *)((uintptr_t)param_1 + value_offset));
        } else {
            // For other types, value_offset contains the actual value.
            printf("Value: %u\n", value_offset);
        }
    }
    return;
}