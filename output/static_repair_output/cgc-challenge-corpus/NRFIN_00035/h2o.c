#include <stdlib.h> // For malloc, free
#include <stdio.h>  // For printf, fprintf

// Function: create_h2o
char *create_h2o(void) {
  char *h2o_data = (char *)malloc(4); // Allocate 4 bytes for "HHO\0"
  if (h2o_data == NULL) {
    return NULL; // Handle allocation failure
  }
  h2o_data[0] = 0x48; // 'H'
  h2o_data[1] = 0x48; // 'H'
  h2o_data[2] = 0x4f; // 'O'
  h2o_data[3] = '\0'; // Null terminator for C string
  return h2o_data;
}

// Function: destroy_h2o
void destroy_h2o(void *param_1) {
  free(param_1);
}

// Main function to demonstrate usage
int main() {
  char *water_molecule = create_h2o();
  if (water_molecule == NULL) {
    fprintf(stderr, "Error: Failed to create H2O molecule.\n");
    return 1; // Indicate an error
  }

  printf("Created H2O molecule: %s\n", water_molecule);

  destroy_h2o(water_molecule);
  water_molecule = NULL; // Good practice to nullify freed pointers

  printf("Destroyed H2O molecule.\n");

  return 0; // Indicate success
}