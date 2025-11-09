#include <stdio.h>   // For fprintf, stderr, printf
#include <stdlib.h>  // For calloc, exit, free
#include <stddef.h>  // For size_t

// Function: xcalloc
void * xcalloc(size_t param_1, size_t param_2) {
  // Use a single variable to store the result of calloc for checking and returning
  void *ptr = calloc(param_1, param_2);
  if (ptr == NULL) { // Use NULL for clarity instead of (void *)0x0
    fprintf(stderr, "FATAL: Allocation failed\n");
    exit(1); // Use standard exit function instead of _terminate
  }
  return ptr;
}

// Main function to demonstrate xcalloc
int main() {
    printf("Attempting to allocate 5 integers using xcalloc...\n");
    int *arr = (int *)xcalloc(5, sizeof(int));

    printf("Allocation successful. Initial values (should be zeros):\n");
    for (int i = 0; i < 5; ++i) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }

    // Modify some values to show functionality
    arr[0] = 100;
    arr[2] = 200;
    arr[4] = 300;
    printf("\nModified values:\n");
    for (int i = 0; i < 5; ++i) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }

    free(arr); // Free the allocated memory
    printf("\nMemory freed. Program finished successfully.\n");

    // This part demonstrates what would happen if allocation fails,
    // but xcalloc's exit(1) prevents reaching here in case of actual failure.
    // For a real test of failure, one might try to allocate an extremely large amount,
    // which would cause the program to exit.
    // Example (uncomment with caution, it will terminate the program if it fails):
    // printf("\nAttempting to allocate a very large block (might fail and exit)...\n");
    // char *large_block = (char *)xcalloc(SIZE_MAX, 1);
    // free(large_block);

    return 0;
}