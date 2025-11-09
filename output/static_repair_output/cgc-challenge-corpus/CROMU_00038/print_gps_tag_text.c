#include <stdio.h> // Required for printf

// Define ushort type for portability
typedef unsigned short ushort;

// Declare and initialize gps_tags array.
// The original code implies an array of 4-byte pointers (char*).
// The size is chosen to accommodate indices 0 through 30 (0x1e), as param_1 < 0x1f.
const char* gps_tags[] = {
    "GPS Tag 0", "GPS Tag 1", "GPS Tag 2", "GPS Tag 3", "GPS Tag 4",
    "GPS Tag 5", "GPS Tag 6", "GPS Tag 7", "GPS Tag 8", "GPS Tag 9",
    "GPS Tag 10", "GPS Tag 11", "GPS Tag 12", "GPS Tag 13", "GPS Tag 14",
    "GPS Tag 15", "GPS Tag 16", "GPS Tag 17", "GPS Tag 18", "GPS Tag 19",
    "GPS Tag 20", "GPS Tag 21", "GPS Tag 22", "GPS Tag 23", "GPS Tag 24",
    "GPS Tag 25", "GPS Tag 26", "GPS Tag 27", "GPS Tag 28", "GPS Tag 29",
    "GPS Tag 30"
};

// Function: print_gps_tag_text
void print_gps_tag_text(ushort param_1) {
  // Check bounds to ensure param_1 is within the valid range of gps_tags array.
  // 0x1f (decimal 31) is the upper limit, meaning param_1 can be 0-30.
  // sizeof(gps_tags) / sizeof(gps_tags[0]) calculates the number of elements in the array.
  if (param_1 < (sizeof(gps_tags) / sizeof(gps_tags[0]))) {
    // Corrected format specifier from "@s" to "%s" for string output.
    // Simplified array access from pointer arithmetic to direct indexing.
    printf("%s\n", gps_tags[param_1]);
  }
  return;
}

// Main function for demonstration and compilation
int main() {
    printf("--- Testing print_gps_tag_text ---\n");

    print_gps_tag_text(0);      // Valid: prints "GPS Tag 0"
    print_gps_tag_text(15);     // Valid: prints "GPS Tag 15"
    print_gps_tag_text(30);     // Valid: prints "GPS Tag 30"
    print_gps_tag_text(31);     // Invalid: param_1 is not less than 31, no output
    print_gps_tag_text(0x1e);   // Valid (0x1e is 30 in decimal): prints "GPS Tag 30"
    print_gps_tag_text(0x1f);   // Invalid (0x1f is 31 in decimal): no output

    printf("--- Test complete ---\n");
    return 0;
}