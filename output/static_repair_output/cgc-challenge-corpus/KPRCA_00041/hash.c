#include <stdio.h> // Required for printf

// Function: get_lhash
// Calculates two hash values for a null-terminated string.
// param_1: A pointer to an array of at least two integers where the hash values will be stored.
// param_2: A pointer to the null-terminated string (using unsigned char for byte-level processing).
// Returns: The original param_1 pointer.
int * get_lhash(int *param_1, unsigned char *param_2) {
  // Initialize hash accumulators.
  // Renamed from 'local_8' to 'hash1' and 'local_c' to 'hash2' for clarity.
  int hash1 = 0x105;
  int hash2 = 0;

  // Loop through the string until the null terminator ('\0') is encountered.
  // The original 'bVar1' intermediate variable has been removed,
  // directly using '*param_2' in calculations to reduce variable count.
  while (*param_2 != '\0') {
    // Cast to unsigned int ensures arithmetic is done with unsigned types,
    // consistent with the original 'uint' cast and common for hash functions.
    hash1 = (unsigned int)*param_2 + hash1 * 0x21;
    hash2 = (unsigned int)*param_2 + hash2 * 0x1003f;
    param_2++; // Move to the next character in the string.
  }

  // Store the calculated hash values in the array pointed to by param_1.
  *param_1 = hash1;
  param_1[1] = hash2; // param_1[1] is equivalent to *(param_1 + 1)

  return param_1; // Return the pointer to the stored hash values.
}

// Main function to demonstrate the get_lhash function.
int main() {
  int hashes[2]; // Array to store the two hash values
  unsigned char test_string_1[] = "Hello, World!"; // Test string
  unsigned char test_string_2[] = "";             // Empty string
  unsigned char test_string_3[] = "A";            // Single character string

  // Demonstrate with "Hello, World!"
  get_lhash(hashes, test_string_1);
  printf("String: \"%s\"\n", test_string_1);
  printf("Hash 1: 0x%x (%d)\n", hashes[0], hashes[0]);
  printf("Hash 2: 0x%x (%d)\n\n", hashes[1], hashes[1]);

  // Demonstrate with an empty string
  get_lhash(hashes, test_string_2);
  printf("String: \"%s\"\n", test_string_2);
  printf("Hash 1: 0x%x (%d)\n", hashes[0], hashes[0]);
  printf("Hash 2: 0x%x (%d)\n\n", hashes[1], hashes[1]);

  // Demonstrate with a single character string
  get_lhash(hashes, test_string_3);
  printf("String: \"%s\"\n", test_string_3);
  printf("Hash 1: 0x%x (%d)\n", hashes[0], hashes[0]);
  printf("Hash 2: 0x%x (%d)\n\n", hashes[1], hashes[1]);

  return 0; // Indicate successful execution
}