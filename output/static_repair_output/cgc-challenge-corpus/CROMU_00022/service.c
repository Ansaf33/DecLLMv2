#include <stdlib.h> // For malloc, free, exit, NULL
#include <stdint.h> // For intptr_t (though explicit void** makes it less critical)

// Replace _terminate with exit(EXIT_FAILURE) for standard error handling.
// The original code uses 0xfffffffe, which is -2.
// exit(EXIT_FAILURE) is a standard way to indicate abnormal termination.
void _terminate(int code) {
    (void)code; // Suppress unused parameter warning
    exit(EXIT_FAILURE);
}

// Forward declaration for MainMenu.
// The actual implementation of MainMenu is not provided in the snippet.
void MainMenu(void *param_1);

// Function: init_data
void * init_data(void) {
  void *pvVar1 = malloc(0x148); // Allocate 328 bytes
  if (pvVar1 == NULL) {
    _terminate(0xfffffffe);
  }
  return pvVar1;
}

// Function: destroy_data
void destroy_data(void *param_1) {
  free(param_1);
}

// Function: AddDive
// param_1 is a pointer to the main data structure which holds the linked list head and count.
void * AddDive(void *param_1) {
  // A node in the linked list structure: [prev_ptr][data_ptr][next_ptr]
  // Original malloc(0xc) suggests 3 * 4-byte entries (32-bit system).
  // For 64-bit Linux, pointers are 8 bytes. To ensure compatibility,
  // we allocate space for 3 `void*` pointers.
  void **new_node = (void **)malloc(3 * sizeof(void *));
  void *data_buffer = malloc(0xbc); // Allocate 188 bytes for dive data

  if (new_node == NULL || data_buffer == NULL) {
    _terminate(0xfffffffe);
  }

  new_node[1] = data_buffer; // Store the pointer to the actual dive data
  new_node[2] = NULL;        // Initialize the 'next' pointer of the new node to NULL

  // Access the head pointer and count from the main data structure (param_1)
  // These are located at fixed offsets within the structure.
  void ***head_ptr_location = (void ***)((char *)param_1 + 0x140);
  int *count_location = (int *)((char *)param_1 + 0x144);

  if (*head_ptr_location == NULL) {
    // The list is empty, so the new node becomes the head.
    *head_ptr_location = new_node;
    new_node[0] = NULL; // The 'previous' pointer for the head node is NULL
    *count_location = 1; // Initialize the count of dives
  } else {
    // The list is not empty, traverse to find the last node.
    void **current_node = *head_ptr_location;
    void **last_node = NULL;
    while (current_node != NULL) {
      last_node = current_node;
      current_node = (void **)current_node[2]; // Move to the next node
    }
    // 'last_node' is now the actual last node in the list.
    last_node[2] = new_node;  // Link the last node's 'next' pointer to the new node
    new_node[0] = last_node;  // Link the new node's 'previous' pointer to the last node
    (*count_location)++;      // Increment the total dive count
  }
  return data_buffer; // Return the pointer to the allocated dive data buffer
}

// Function: DeleteDive
// param_1 is a pointer to the main data structure.
// param_2 is a pointer to the specific node (void**) to be deleted from the list.
void * DeleteDive(void *param_1, void **param_2) {
  void ***head_ptr_location = (void ***)((char *)param_1 + 0x140);
  int *count_location = (int *)((char *)param_1 + 0x144);
  void *return_previous_node = NULL; // Initialize return value (pointer to previous node, or NULL)

  // Traverse the list to find the node pointed to by param_2.
  void **node_to_find = *head_ptr_location;
  while (node_to_find != NULL && node_to_find != param_2) {
    node_to_find = (void **)node_to_find[2]; // Move to the next node
  }

  if (node_to_find == NULL) {
    // The node to delete (param_2) was not found in the list.
    // 'return_previous_node' remains NULL.
  } else {
    // The node (param_2) was found. Proceed with deletion.

    void **prev_node = (void **)param_2[0]; // Get the 'previous' node
    void **next_node = (void **)param_2[2]; // Get the 'next' node

    // Update the 'next' pointer of the previous node (if it exists).
    if (prev_node != NULL) {
      prev_node[2] = next_node;
      return_previous_node = prev_node; // Set return value to the previous node
    }

    // Update the 'previous' pointer of the next node (if it exists).
    if (next_node != NULL) {
      next_node[0] = prev_node;
    }

    // If the node being deleted was the head of the list, update the head pointer.
    if (param_2 == *head_ptr_location) {
      *head_ptr_location = next_node; // The new head is the deleted node's 'next'
    }

    // Decrement the dive count, with a safety check.
    if (*count_location > 0) {
        (*count_location)--;
    }

    // Free the associated data buffer and the node structure itself.
    free(param_2[1]); // Free the dive data buffer
    free(param_2);    // Free the node structure
  }
  return return_previous_node; // Return the previous node (or NULL if head/not found)
}

// Dummy MainMenu function for compilation.
// Its actual implementation is not provided in the snippet.
void MainMenu(void *param_1) {
    (void)param_1; // Suppress unused parameter warning for compilation
    // In a complete application, this function would contain the main menu logic.
}

// Function: main
int main(void) {
  void *data_handle = init_data(); // Initialize the main data structure
  MainMenu(data_handle);           // Call the main menu function
  destroy_data(data_handle);       // Clean up the main data structure
  return EXIT_SUCCESS;             // Indicate successful execution
}