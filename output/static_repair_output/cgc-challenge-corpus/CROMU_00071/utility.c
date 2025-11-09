#include <stdio.h> // Required for printf in the main function

// Declare DAT_4347c000 as a global array of unsigned characters.
// Its size is 0x1000 (4096) bytes.
// It's declared 'static' to limit its scope to this compilation unit,
// and it will be initialized in the main function for a predictable sum.
static unsigned char DAT_4347c000[0x1000];

// Function: check_db
int check_db(void) {
  unsigned int sum = 0; // Accumulator for the sum of byte values
  unsigned int i;       // Loop counter

  for (i = 0; i < 0x1000; ++i) {
    // Access the array element directly.
    // The unsigned char value is implicitly promoted to unsigned int for addition.
    sum += DAT_4347c000[i];
  }
  // Return the sum as an int, as per the function signature.
  // This assumes the total sum will not exceed the maximum value of an int.
  return (int)sum;
}

// Main function to demonstrate the usage of check_db.
int main(void) {
  // Initialize DAT_4347c000 with sample data for a reproducible result.
  // For example, fill it with values cycling from 0 to 255.
  for (unsigned int k = 0; k < 0x1000; ++k) {
    DAT_4347c000[k] = (unsigned char)(k % 256);
  }

  // Call check_db and print its result.
  int result = check_db();
  printf("Result of check_db: %d\n", result);

  return 0;
}