#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For exit, EXIT_FAILURE
#include <stdint.h>  // For uint16_t, uint32_t, etc.

// Define custom types based on common decompilation patterns
// ushort maps to 16-bit unsigned integer
// uint maps to 32-bit unsigned integer
// undefined4 maps to 32-bit unsigned integer (often used for raw memory access)
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint32_t undefined4; // Used for 4-byte raw memory access

// --- Function Stubs (placeholders for external functions) ---
// In a real application, these would be implemented elsewhere or linked from a library.

// Simulates a program termination with an error message.
// The original code used _terminate() after setting error messages on the stack.
// This version prints to stderr and exits.
void _terminate_with_message(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

// Placeholder for byte swapping a 16-bit short.
// Returns the value as is for demonstration, but would typically reverse byte order.
ushort swap_short(ushort val) {
    // Example for little-endian to big-endian or vice-versa:
    // return (val << 8) | (val >> 8);
    return val; // Identity for compilation
}

// Placeholder for byte swapping a 32-bit word.
// Returns the value as is for demonstration, but would typically reverse byte order.
uint swap_word(uint val) {
    // Example for little-endian to big-endian or vice-versa:
    // return ((val << 24) & 0xFF000000) | ((val << 8) & 0x00FF0000) |
    //        ((val >> 8) & 0x0000FF00) | ((val >> 24) & 0x000000FF);
    return val; // Identity for compilation
}

// Placeholder to print the text representation of a GPS tag.
void print_gps_tag_text(ushort tag) {
    printf("GPS_TAG_0x%X", tag); // Example output
}

// Placeholder to print the text representation of a type.
void print_type(ushort type) {
    printf("TYPE_0x%X", type); // Example output
}

// Global data placeholder (DAT_00016abb)
// Based on its usage like `printf(*(char **)(puVar5 + -0x10));` after printing a value,
// it appears to be a format string, likely containing a closing parenthesis and space.
static const char* DAT_00016abb = ") ";

// Function: process_gps_ifd
void process_gps_ifd(ushort *param_1, ushort *param_2, ushort param_3, ushort *param_4) {
  ushort local_20 = param_3; // Renamed from param_3 to local_20 for consistency with original code
  uint local_18;
  ushort local_12;
  int local_10;
  
  // Check if param_4 (end pointer) is before param_1 (start pointer), indicating an invalid range.
  if (param_4 < param_1) {
    _terminate_with_message("Invalid GPS_IFD pointer range (param_4 < param_1).");
  }

  // Check if param_1 (start of GPS_IFD) is before param_2 (overall data block start).
  if (param_1 < param_2) {
    _terminate_with_message("Invalid GPS_IFD pointer range (param_1 < param_2).");
  }

  // local_12 is the count of GPS IFD entries.
  // The value is read from the first ushort pointed to by param_1.
  local_12 = swap_short(*param_1);

  // local_18 is the number of bytes available from param_1 to param_4.
  // Cast to uint8_t* for byte-accurate pointer arithmetic to get byte difference.
  local_18 = (uint)((uint8_t*)param_4 - (uint8_t*)param_1);

  // Check if the expected size (param_3/local_20, assumed to be bytes) is less than the actual available size.
  // This implies param_3 acts as a maximum allowed byte length.
  if (local_20 < local_18) {
    _terminate_with_message("GPS_IFD expected size (param_3) is less than actual available size.");
  }

  // Check if the available space is sufficient for the IFD entries.
  // Each IFD entry is 12 bytes (0xc). Plus 2 bytes for the entry count itself (*param_1).
  if (local_18 < (uint)local_12 * 12 + sizeof(ushort)) {
    fprintf(stderr, "Invalid GPS_IFD count value\n");
    _terminate_with_message("Insufficient space for GPS_IFD entries based on count.");
  }

  // Print the number of arrays (IFD entries).
  printf("# of arrays: %d\n", local_12);

  // The actual IFD entries start after the initial count field (*param_1).
  // So, param_1 + 1 points to the first ushort of the first entry.
  ushort *base_entry_ptr = param_1 + 1;

  for (local_10 = 0; local_10 < (int)local_12; local_10++) {
    // Each IFD entry is 12 bytes, which is 6 ushorts.
    // entry_start points to the beginning of the current IFD entry.
    ushort *entry_start = base_entry_ptr + (local_10 * 6);

    // --- Tag (2 bytes) ---
    // The tag is the first ushort in the entry.
    ushort tag = swap_short(entry_start[0]);

    printf("Tag: %x (", tag);
    print_gps_tag_text(tag); // Pass the tag to the print function
    printf("%s", DAT_00016abb); // Prints ") "

    // --- Type (2 bytes) ---
    // The type is the second ushort in the entry.
    ushort type = swap_short(entry_start[1]);

    printf("Type: %x (", type);
    print_type(type); // Pass the type to the print function
    printf("%s", DAT_00016abb); // Prints ") "

    // --- Count (4 bytes) ---
    // The 4-byte count value starts at the third ushort (index 2) of the entry.
    // Direct casting `*(undefined4*)` is used as in the original decompiled code.
    // Note: This might cause alignment issues on some architectures if `entry_start + 2` is not 4-byte aligned.
    // For robust, portable code, `memcpy` or combining two `ushort` reads would be safer.
    uint count = swap_word(*(undefined4*)(entry_start + 2));

    printf("Count: %u\n", count); // Changed @x to %u as count is typically unsigned decimal

    // --- Value/Offset (4 bytes) ---
    // The 4-byte value/offset starts at the fifth ushort (index 4) of the entry.
    // Same alignment considerations as for 'count'.
    uint value = swap_word(*(undefined4*)(entry_start + 4));

    printf("Value: %u\n", value);
  }
}

// --- Main function for compilation and basic testing ---
int main() {
    // A dummy buffer representing image data containing a GPS IFD.
    // This example sets up 1 GPS IFD entry.
    // IFD Structure (each entry is 12 bytes):
    // ushort entry_count
    // For each entry:
    //   ushort Tag
    //   ushort Type
    //   uint Count
    //   uint Value/Offset

    // Example data: 1 entry
    // Total size: 2 bytes (count) + 1 * 12 bytes (entry) = 14 bytes
    // Represented as ushorts: 7 ushorts.
    ushort dummy_data_single_entry[7];

    dummy_data_single_entry[0] = 1;      // Entry count = 1
    dummy_data_single_entry[1] = 0x0001; // Tag = 0x0001
    dummy_data_single_entry[2] = 0x0003; // Type = 0x0003 (USHORT)
    
    // Count (4 bytes): 0x0000000A (10)
    // Directly writing 4 bytes by casting to uint32_t*.
    // Assumes target system's endianness matches data representation.
    *(uint32_t*)&dummy_data_single_entry[3] = 0x0000000A;

    // Value/Offset (4 bytes): 0x00000064 (100)
    *(uint32_t*)&dummy_data_single_entry[5] = 0x00000064;

    ushort *param_1_single = dummy_data_single_entry;
    ushort *param_2_single = dummy_data_single_entry; // Assume param_1 is at the start of the block
    ushort param_3_single = sizeof(dummy_data_single_entry); // Total size in bytes
    ushort *param_4_single = dummy_data_single_entry + (sizeof(dummy_data_single_entry) / sizeof(ushort)); // Pointer to end of buffer

    printf("--- Processing GPS IFD with a single dummy entry ---\n");
    process_gps_ifd(param_1_single, param_2_single, param_3_single, param_4_single);
    printf("--- Single entry processing complete ---\n\n");

    // Test with multiple entries
    ushort dummy_data_multi_entry[1 + 2 * 6]; // 1 ushort for count + 2 entries * 6 ushorts/entry
    dummy_data_multi_entry[0] = 2; // Two entries

    // Entry 1
    dummy_data_multi_entry[1] = 0x0002; // Tag 0x0002
    dummy_data_multi_entry[2] = 0x0004; // Type 0x0004 (ULONG)
    *(uint32_t*)&dummy_data_multi_entry[3] = 0x0000001E; // Count 30
    *(uint32_t*)&dummy_data_multi_entry[5] = 0x000000C8; // Value 200

    // Entry 2
    dummy_data_multi_entry[7] = 0x0003; // Tag 0x0003
    dummy_data_multi_entry[8] = 0x0005; // Type 0x0005 (URATIONAL)
    *(uint32_t*)&dummy_data_multi_entry[9] = 0x00000001; // Count 1
    *(uint32_t*)&dummy_data_multi_entry[11] = 0x0000012C; // Value 300

    ushort *param_1_multi = dummy_data_multi_entry;
    ushort *param_2_multi = dummy_data_multi_entry;
    ushort param_3_multi = sizeof(dummy_data_multi_entry);
    ushort *param_4_multi = dummy_data_multi_entry + (sizeof(dummy_data_multi_entry) / sizeof(ushort));

    printf("--- Processing GPS IFD with multiple dummy entries ---\n");
    process_gps_ifd(param_1_multi, param_2_multi, param_3_multi, param_4_multi);
    printf("--- Multiple entries processing complete ---\n\n");

    // Example of an error case (uncomment to test termination)
    /*
    printf("--- Testing error: param_4 < param_1 ---\n");
    ushort invalid_range_data[2] = {0, 0};
    // This call should trigger _terminate_with_message
    process_gps_ifd(&invalid_range_data[1], invalid_range_data, 4, &invalid_range_data[0]);
    */

    return 0;
}