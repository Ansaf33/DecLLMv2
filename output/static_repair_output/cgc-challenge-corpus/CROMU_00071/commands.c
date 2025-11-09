#include <stdio.h>  // Required for printf in stub functions and main
#include <stdlib.h> // Required for atoi

// --- Stub Functions (to make the code compilable) ---
// These stubs assume 'airport_data' is a pointer to some data structure
// and 'airport_data_ptr' is a pointer to that pointer, allowing modification
// of the main data structure pointer by functions like 'addAirport'.

int showAirports(void *airport_data, char *arg) {
    // printf("DEBUG: showAirports called with data_ptr=%p, arg=%s\n", airport_data, arg);
    return 0;
}

int addAirport(void **airport_data_ptr, char *arg) {
    // printf("DEBUG: addAirport called with data_ptr_ptr=%p, arg=%s\n", airport_data_ptr, arg);
    // Example: If airport_data_ptr points to a 'struct AirportCollection *',
    // you might reallocate or modify *airport_data_ptr here.
    return 0;
}

int deleteAirport(void **airport_data_ptr, char *arg) {
    // printf("DEBUG: deleteAirport called with data_ptr_ptr=%p, arg=%s\n", airport_data_ptr, arg);
    // Example: You might free resources pointed to by *airport_data_ptr
    // or modify the collection it represents.
    return 0;
}

int findRoutes(void *airport_data, char *arg) {
    // printf("DEBUG: findRoutes called with data_ptr=%p, arg=%s\n", airport_data, arg);
    return 0;
}

// --- Fixed execute_cmd function ---
// Replaced 'undefined4' with 'int' for return types and parameters where appropriate.
// Interpreted 'param_1' as 'void **' based on its usage (*param_1 vs param_1).
// Eliminated the intermediate variable 'local_10' by direct returns.
// Renamed variables for clarity.
int execute_cmd(void **airport_data_ptr_ptr, char *cmd_arg_str) {
  int cmd_id = atoi(cmd_arg_str); // Parse the command ID from the string

  switch(cmd_id) {
  default:
    return 0; // Default return for unhandled command IDs
  case 1:
    // Passes the dereferenced pointer (the actual data structure pointer)
    return showAirports(*airport_data_ptr_ptr, cmd_arg_str);
  case 2:
    // Passes the pointer to the data structure pointer, allowing modification
    return addAirport(airport_data_ptr_ptr, cmd_arg_str);
  case 3:
    // Passes the pointer to the data structure pointer, allowing modification
    return deleteAirport(airport_data_ptr_ptr, cmd_arg_str);
  case 0xd: // Hexadecimal 13
    // 0xffffff9d as a 32-bit signed integer is -99
    return -99;
  case 0xf: // Hexadecimal 15
    // Passes the dereferenced pointer (the actual data structure pointer)
    return findRoutes(*airport_data_ptr_ptr, cmd_arg_str);
  }
}

// --- Main function for testing ---
int main() {
    // Represents the actual data structure (e.g., a linked list head, array, etc.)
    void *airport_data_collection = NULL;
    // Pointer to the pointer, allowing execute_cmd to pass it to functions
    // that might modify where airport_data_collection points (like add/delete).
    void **airport_data_handle = &airport_data_collection;

    printf("--- Testing execute_cmd ---\n");

    printf("\nExecuting command 1 (showAirports):\n");
    int result1 = execute_cmd(airport_data_handle, "1");
    printf("Result: %d\n", result1);

    printf("\nExecuting command 2 (addAirport):\n");
    int result2 = execute_cmd(airport_data_handle, "2");
    printf("Result: %d\n", result2);

    printf("\nExecuting command 3 (deleteAirport):\n");
    int result3 = execute_cmd(airport_data_handle, "3");
    printf("Result: %d\n", result3);

    printf("\nExecuting command 13 (0xd):\n");
    int result_13 = execute_cmd(airport_data_handle, "13");
    printf("Result: %d (Expected: -99)\n", result_13);

    printf("\nExecuting command 15 (0xf - findRoutes):\n");
    int result_15 = execute_cmd(airport_data_handle, "15");
    printf("Result: %d\n", result_15);

    printf("\nExecuting an unknown command (99):\n");
    int result_unknown = execute_cmd(airport_data_handle, "99");
    printf("Result: %d (Expected: 0)\n", result_unknown);

    printf("\nExecuting command with non-numeric argument (e.g., 'abc'):\n");
    int result_bad_arg = execute_cmd(airport_data_handle, "abc"); // atoi("abc") returns 0
    printf("Result: %d (Expected: 0, as atoi('abc') is 0, matching default case)\n", result_bad_arg);

    return 0;
}