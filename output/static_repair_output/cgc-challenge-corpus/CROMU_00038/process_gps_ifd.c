#include <stdio.h>    // For printf, fprintf
#include <stdlib.h>   // For exit, EXIT_FAILURE
#include <stdint.h>   // For uint16_t, uint32_t, uint8_t

// Using standard integer types for clarity and portability
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint32_t undefined4; // Represents a 4-byte undefined type, typically unsigned int
typedef uint8_t undefined;   // Represents a 1-byte undefined type, typically unsigned char

// Dummy implementation for _terminate
// The original code passed fixed magic numbers (e.g., 0x12318) which seem like
// program counter values or internal error codes. We'll pass them as an error_code
// to make _terminate more informative.
static void _terminate(const char* message, uint error_code) {
    fprintf(stderr, "Fatal Error (Code 0x%X): %s\n", error_code, message);
    exit(EXIT_FAILURE);
}

// Dummy implementation for swap_short (assumes byte swapping for 16-bit value)
// This is typical for converting between little-endian and big-endian representations.
static ushort swap_short(ushort val) {
    return (val << 8) | (val >> 8);
}

// Dummy implementation for swap_word (assumes byte swapping for 32-bit value)
static undefined4 swap_word(undefined4 val) {
    return ((val << 24) & 0xFF000000) |
           ((val << 8)  & 0x00FF0000) |
           ((val >> 8)  & 0x0000FF00) |
           ((val >> 24) & 0x000000FF);
}

// Dummy implementation for print_gps_tag_text
// The original code implied it would print text based on a previously stored value.
// We pass the tag explicitly for clarity and correctness.
static void print_gps_tag_text(ushort tag) {
    // In a real application, this would map 'tag' to a descriptive string.
    // For now, just print a placeholder.
    printf("GPS Tag 0x%04X description", tag);
}

// Dummy implementation for print_type
// Similar to print_gps_tag_text, we pass the type explicitly.
static void print_type(ushort type) {
    // In a real application, this would map 'type' to a descriptive string.
    // For now, just print a placeholder.
    printf("Type 0x%04X description", type);
}

// Assuming DAT_00016abb is a global string literal for closing parenthesis and newline
static const char* DAT_00016abb = ")\n";

// Function: process_gps_ifd
void process_gps_ifd(ushort *param_1, ushort *param_2, ushort param_3, ushort *param_4) {
  // Removed intermediate variables and stack manipulation artifacts from decompiler output.
  // Renamed for clarity.
  ushort num_ifd_entries;
  unsigned int available_buffer_bytes;
  unsigned int i; // Loop counter
  
  // Check 1: param_4 (end pointer) must be greater than or equal to param_1 (start pointer).
  // If param_4 is before param_1, the buffer range is invalid.
  if (param_4 < param_1) {
    _terminate("Invalid buffer range: end pointer is before start pointer.", 0x12318);
  }
  
  // Check 2: param_1 (GPS_IFD start) must be greater than or equal to param_2 (base pointer).
  // This implies param_2 is a base address for the entire data block, and param_1 must be within it or after it.
  if (param_1 < param_2) {
    _terminate("GPS_IFD start pointer is before the allowed base pointer.", 0x12349);
  }
  
  // Read the number of IFD entries from the first ushort pointed by param_1, and swap bytes.
  num_ifd_entries = swap_short(*param_1);
  
  // Calculate available buffer size in bytes.
  // param_4 and param_1 are ushort pointers. Their difference gives a count of ushorts.
  // Cast to char* pointers to get the difference in bytes.
  available_buffer_bytes = (unsigned char *)param_4 - (unsigned char *)param_1;
  
  // Check 3: param_3 (originally local_20) vs available_buffer_bytes (originally local_18).
  // The original logic `if (local_20 < local_18)` is preserved.
  // This comparison suggests that if param_3 (a ushort, likely representing a maximum allowed size in bytes)
  // is less than the actual available buffer size in bytes, it's considered an error.
  // This implies the available buffer is "too large" or exceeds a specified boundary.
  if (param_3 < available_buffer_bytes) {
    _terminate("Available buffer size exceeds specified maximum size for GPS_IFD.", 0x12383);
  }
  
  // Check 4: Ensure available buffer is large enough for the IFD structure.
  // The GPS IFD starts with a 2-byte count, followed by entries.
  // Each entry consists of 6 ushorts (12 bytes).
  // Total required bytes = 2 (for count) + num_ifd_entries * 12 (for entries).
  unsigned int required_ifd_bytes = 2 + (unsigned int)num_ifd_entries * 12;
  
  if (available_buffer_bytes < required_ifd_bytes) {
    fprintf(stderr, "Invalid GPS_IFD data: Available buffer (%u bytes) is too small for %u entries (%u bytes required).\n",
            available_buffer_bytes, num_ifd_entries, required_ifd_bytes);
    _terminate("Invalid GPS_IFD count value.", 0x123b4);
  }
  
  // Print the number of arrays (IFD entries)
  // Original format specifier '@d' is replaced with '%d'.
  printf("# of arrays: %d\n", num_ifd_entries);
  
  // Loop through each IFD entry
  // Entries start after the initial count (param_1[0]).
  // Each entry's fields are located relative to param_1.
  for (i = 0; i < num_ifd_entries; i++) {
    // Tag (ushort) - located at param_1[1 + i*6]
    // Original format specifier '@x' is replaced with '0x%x'.
    ushort tag_value = swap_short(param_1[1 + i * 6]);
    printf("Tag: 0x%x (", tag_value);
    print_gps_tag_text(tag_value);
    printf("%s", DAT_00016abb); // prints ")\n"
    
    // Type (ushort) - located at param_1[2 + i*6]
    // Original format specifier '@x' is replaced with '0x%x'.
    ushort type_value = swap_short(param_1[2 + i * 6]);
    printf("Type: 0x%x (", type_value);
    print_type(type_value);
    printf("%s", DAT_00016abb); // prints ")\n"
    
    // Count (undefined4 - 32-bit) - located at param_1[3 + i*6] and param_1[4 + i*6]
    // Safely combine two ushorts into a 32-bit unsigned int to avoid unaligned access,
    // which might occur if param_1 is not 4-byte aligned or the offset is odd.
    // Original format specifier '@x' is replaced with '0x%x'.
    undefined4 raw_count_word = ( (undefined4)param_1[3 + i * 6] ) | ( (undefined4)param_1[4 + i * 6] << 16 );
    undefined4 count_value = swap_word(raw_count_word);
    printf("Count: 0x%x\n", count_value);
    
    // Value/Offset (undefined4 - 32-bit) - located at param_1[5 + i*6] and param_1[6 + i*6]
    // Safely combine two ushorts into a 32-bit unsigned int.
    // Original format specifier '@u' is replaced with '%u'.
    undefined4 raw_value_word = ( (undefined4)param_1[5 + i * 6] ) | ( (undefined4)param_1[6 + i * 6] << 16 );
    undefined4 value_offset = swap_word(raw_value_word);
    printf("Value: %u\n", value_offset);
  }
  return;
}

// Example main function to make the code compilable and testable.
// This main function demonstrates how to call process_gps_ifd with dummy data.
int main() {
    // Dummy data for testing a GPS IFD block.
    // Data is set up so that `swap_short` and `swap_word` will produce the expected values
    // if they perform standard byte-swapping (e.g., converting big-endian data on a little-endian machine).

    // Expected values after swapping:
    // Count: 2
    // Entry 1: Tag=0x0001, Type=0x0002, Count=0x0000000A (10), Value=0x00000064 (100)
    // Entry 2: Tag=0x0003, Type=0x0004, Count=0x0000000B (11), Value=0x000000C8 (200)

    // `gps_ifd_raw_data` is filled with values that, when passed through `swap_short`/`swap_word`,
    // will yield the desired numeric values. For a little-endian host, if the source data is big-endian,
    // these values would be the byte-swapped versions of the desired values.
    ushort gps_ifd_raw_data[] = {
        swap_short(2), // IFD count: 2 entries

        // --- Entry 1 ---
        swap_short(1),   // Tag: 0x0001
        swap_short(2),   // Type: 0x0002
        // Count: 0x0000000A (10). Stored as two ushorts, then byte-swapped.
        swap_short(0x000A), swap_short(0x0000), 
        // Value: 0x00000064 (100). Stored as two ushorts, then byte-swapped.
        swap_short(0x0064), swap_short(0x0000), 

        // --- Entry 2 ---
        swap_short(3),   // Tag: 0x0003
        swap_short(4),   // Type: 0x0004
        // Count: 0x0000000B (11). Stored as two ushorts, then byte-swapped.
        swap_short(0x000B), swap_short(0x0000), 
        // Value: 0x000000C8 (200). Stored as two ushorts, then byte-swapped.
        swap_short(0x00C8), swap_short(0x0000)  
    };
    
    // Calculate param_4 (pointer to one past the end of the buffer)
    ushort *param_4_ptr = gps_ifd_raw_data + sizeof(gps_ifd_raw_data) / sizeof(gps_ifd_raw_data[0]);
    
    // Call the function with valid data
    printf("--- Processing GPS IFD (Valid Data) ---\n");
    process_gps_ifd(
        gps_ifd_raw_data,             // param_1: Start of GPS IFD data
        gps_ifd_raw_data,             // param_2: Base pointer (assume same as param_1 for simplicity)
        sizeof(gps_ifd_raw_data),     // param_3: Max expected size in bytes (total size of our buffer)
        param_4_ptr                   // param_4: Pointer to one past the end of the buffer
    );
    printf("--- Finished processing GPS IFD (Valid Data) ---\n");

    // Example of an error case: buffer too small for required IFD bytes
    printf("\n--- Testing error: buffer too small for entries ---\n");
    ushort small_buffer[] = {swap_short(1), swap_short(1)}; // Only 2 ushorts (4 bytes), but 1 entry needs 14 bytes
    ushort *small_buffer_end = small_buffer + sizeof(small_buffer) / sizeof(small_buffer[0]);
    // This call is expected to terminate the program.
    process_gps_ifd(small_buffer, small_buffer, sizeof(small_buffer), small_buffer_end); 

    return 0; // This line might not be reached if an error occurs and exits.
}