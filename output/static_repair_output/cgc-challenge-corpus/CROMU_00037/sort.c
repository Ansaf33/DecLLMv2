#include <stdbool.h> // For bool type
#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free (if needed, not directly used in snippet)
#include <string.h>  // For memcpy (if needed, not directly used in snippet)

// Define custom types based on common assumptions for embedded/low-level C
typedef unsigned char byte;
typedef unsigned int uint;

// Function: sort_n_sum
byte sort_n_sum(byte *param_1, uint param_2) {
  byte bVar1;     // Temporary variable for swapping elements
  bool bVar2;     // Flag to indicate if a swap occurred in a pass (for bubble sort)
  uint local_c;   // Loop counter
  byte local_d = 0; // Accumulator for the XOR sum, initialized to 0

  // Bubble sort algorithm
  // The outer loop continues as long as swaps are made, ensuring array is sorted.
  if (param_2 > 1) { // Only attempt to sort if there are at least two elements
    bVar2 = true;
    while (bVar2) {
      bVar2 = false; // Assume no swaps will be made in this pass
      // Inner loop iterates through the array, comparing adjacent elements
      for (local_c = 1; local_c < param_2; local_c++) {
        // Compare elements as signed characters, preserving original snippet's behavior
        if ((char)param_1[local_c] < (char)param_1[local_c - 1]) {
          // If elements are out of order, swap them
          bVar1 = param_1[local_c - 1];
          param_1[local_c - 1] = param_1[local_c];
          param_1[local_c] = bVar1;
          bVar2 = true; // A swap occurred, so another pass is needed
        }
      }
    }
  }

  // Calculate the XOR sum of the elements in the (now sorted) array.
  // This calculation is performed once after sorting, which is more efficient
  // and semantically appropriate for a function named "sort_n_sum".
  // The original snippet's repeated calculation of local_d inside the sorting loop
  // was redundant, as the XOR sum of an array is invariant to element permutations.
  for (local_c = 0; local_c < param_2; local_c++) {
    local_d ^= param_1[local_c];
  }
  
  return local_d; // Return the calculated XOR sum
}

// Main function to demonstrate the usage of sort_n_sum
int main() {
    // Test case 1
    byte data1[] = {5, 2, 8, 1, 9};
    uint size1 = sizeof(data1) / sizeof(data1[0]);
    byte result1;

    printf("Original array 1: ");
    for (uint i = 0; i < size1; i++) {
        printf("%hhu ", data1[i]);
    }
    printf("\n");

    result1 = sort_n_sum(data1, size1);

    printf("Sorted array 1:   ");
    for (uint i = 0; i < size1; i++) {
        printf("%hhu ", data1[i]);
    }
    printf("\n");
    printf("XOR Sum 1: %hhu\n\n", result1); // Expected: sorted {1,2,5,8,9}, XOR sum: 1^2^5^8^9 = 7

    // Test case 2
    byte data2[] = {3, 1, 2};
    uint size2 = sizeof(data2) / sizeof(data2[0]);
    byte result2;

    printf("Original array 2: ");
    for (uint i = 0; i < size2; i++) {
        printf("%hhu ", data2[i]);
    }
    printf("\n");

    result2 = sort_n_sum(data2, size2);

    printf("Sorted array 2:   ");
    for (uint i = 0; i < size2; i++) {
        printf("%hhu ", data2[i]);
    }
    printf("\n");
    printf("XOR Sum 2: %hhu\n\n", result2); // Expected: sorted {1,2,3}, XOR sum: 1^2^3 = 0

    // Test case 3: Single element array
    byte data3[] = {10};
    uint size3 = sizeof(data3) / sizeof(data3[0]);
    byte result3;

    printf("Original array 3: ");
    for (uint i = 0; i < size3; i++) {
        printf("%hhu ", data3[i]);
    }
    printf("\n");

    result3 = sort_n_sum(data3, size3);

    printf("Sorted array 3:   ");
    for (uint i = 0; i < size3; i++) {
        printf("%hhu ", data3[i]);
    }
    printf("\n");
    printf("XOR Sum 3: %hhu\n\n", result3); // Expected: sorted {10}, XOR sum: 10

    // Test case 4: Empty array
    byte data4[] = {};
    uint size4 = 0;
    byte result4;

    printf("Original array 4: (empty)\n");

    result4 = sort_n_sum(data4, size4);

    printf("Sorted array 4:   (empty)\n");
    printf("XOR Sum 4: %hhu\n\n", result4); // Expected: 0

    return 0;
}