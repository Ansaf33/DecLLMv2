#include <stdio.h>  // Required for printf
#include <stdlib.h> // For EXIT_SUCCESS

// Declare a global array of string pointers (char*) for gps_tags.
// The original code implies an array of 4-byte entities that are pointers to strings.
// Using `const char *` for string literals is good practice.
const char *gps_tags[] = {
    "GPS Tag 0: Location",
    "GPS Tag 1: Time",
    "GPS Tag 2: Speed",
    "GPS Tag 3: Heading",
    "GPS Tag 4: Altitude",
    "GPS Tag 5: Satellites",
    "GPS Tag 6: HDOP",
    "GPS Tag 7: VDOP",
    "GPS Tag 8: Fix Quality",
    "GPS Tag 9: Date",
    "GPS Tag 10: Latitude Ref",
    "GPS Tag 11: Longitude Ref",
    "GPS Tag 12: Altitude Ref",
    "GPS Tag 13: Status",
    "GPS Tag 14: Mode",
    "GPS Tag 15: Type",
    "GPS Tag 16: Device ID",
    "GPS Tag 17: Firmware Version",
    "GPS Tag 18: Battery Level",
    "GPS Tag 19: Signal Strength",
    "GPS Tag 20: Error Code",
    "GPS Tag 21: Calibration Status",
    "GPS Tag 22: Sensor Temperature",
    "GPS Tag 23: Pressure",
    "GPS Tag 24: Humidity",
    "GPS Tag 25: Orientation X",
    "GPS Tag 26: Orientation Y",
    "GPS Tag 27: Orientation Z",
    "GPS Tag 28: Magnetic Declination",
    "GPS Tag 29: Horizontal Error",
    "GPS Tag 30: Vertical Error"
    // The condition `param_1 < 0x1f` (param_1 < 31) means valid indices are 0-30.
    // So, 31 elements are needed.
};

// Function: print_gps_tag_text
// Renamed `ushort` to `unsigned short` for standard C compliance.
void print_gps_tag_text(unsigned short param_1) {
  // The original condition `param_1 < 0x1f` means param_1 can be 0 up to 30.
  // 0x1f in hexadecimal is 31 in decimal.
  if (param_1 < 31) {
    // Original: printf("@s",*(undefined4 *)(gps_tags + (uint)param_1 * 4));
    // Fixes:
    // 1. `@s` -> `%s` for printing a string.
    // 2. `undefined4` is replaced by `const char *` as it's a pointer to a string.
    // 3. `gps_tags + (uint)param_1 * 4` implies manual byte offset if `gps_tags`
    //    was treated as a raw `void*` and elements were 4 bytes.
    //    For a standard C array `const char *gps_tags[]`, direct array indexing
    //    `gps_tags[param_1]` is the correct, simpler, and portable way.
    //    It implicitly handles the size of `const char *` (e.g., 8 bytes on 64-bit Linux).
    //    This also reduces intermediate variables by avoiding manual pointer arithmetic.
    printf("%s", gps_tags[param_1]);
  }
}

// Main function to demonstrate the usage of print_gps_tag_text
int main() {
    printf("Demonstrating print_gps_tag_text:\n");

    // Test with a valid index
    printf("Tag at index 0: ");
    print_gps_tag_text(0);
    printf("\n");

    // Test with an intermediate valid index
    printf("Tag at index 15: ");
    print_gps_tag_text(15);
    printf("\n");

    // Test with the maximum valid index (30)
    printf("Tag at index 30: ");
    print_gps_tag_text(30);
    printf("\n");

    // Test with an invalid index (out of bounds, should print nothing)
    printf("Tag at index 31 (invalid): ");
    print_gps_tag_text(31);
    printf(" (nothing printed if invalid)\n");

    printf("Tag at index 50 (invalid): ");
    print_gps_tag_text(50);
    printf(" (nothing printed if invalid)\n");

    return EXIT_SUCCESS; // Standard way to indicate successful execution
}