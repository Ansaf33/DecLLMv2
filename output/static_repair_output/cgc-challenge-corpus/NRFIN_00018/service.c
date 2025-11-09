#include <stdio.h>    // For fprintf in _terminate (debugging)
#include <stdlib.h>   // For exit, calloc, free
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uint32_t, uint16_t, uintptr_t

// Decompiler-specific type aliases
typedef unsigned short ushort;
typedef uint32_t undefined4; // Assuming undefined4 is 4 bytes
typedef uint16_t undefined2; // Assuming undefined2 is 2 bytes

// 'undefined' is often a placeholder for a type the decompiler couldn't determine.
// Given its usage as `undefined **` or `undefined *`, `void` is a reasonable substitute for generic pointers in C.
typedef void undefined;

// 'code' is a function pointer type.
// In processMessage, it's used as (*(code *)param_1[2])(iVar1); where iVar1 is an int.
typedef void (*code)(int);

// Dummy global offset table to replace _GLOBAL_OFFSET_TABLE_
void *dummy_global_offset_table = NULL;

// External functions (dummy implementations to allow compilation on Linux)
// In a real system, these would be provided by a specific runtime environment or library.

// Simulates program termination with a status code.
void _terminate(int status) {
    fprintf(stderr, "Program terminated with status %d\n", status);
    exit(status);
}

// Simulates receiving data. Assumed to take an integer argument based on usage in receiveMessage.
// Returns 0 for success, non-zero for failure (triggering _terminate).
int receive(int arg) {
    // Dummy implementation: always succeed
    (void)arg; // Suppress unused parameter warning
    return 0;
}

// Simulates transmitting data. The last argument (0x111e0 in original) is treated as a decompiler artifact.
// Returns 0 for success, non-zero for failure (triggering _terminate).
int transmit_all(int a, undefined4 b, int c) {
    // Dummy implementation: always succeed
    (void)a; (void)b; (void)c; // Suppress unused parameter warnings
    return 0;
}

// Simulates memory allocation. Returns a pointer to zero-initialized memory or NULL on failure.
void *allocate(size_t size) {
    return calloc(1, size);
}

// Simulates memory deallocation. The size argument from the original decompiler output
// is usually an artifact or specific to a custom memory manager; standard C `free` doesn't use it.
// Returns 0 for success, non-zero for failure (though `free` typically doesn't return).
int deallocate(void *ptr) {
    free(ptr);
    return 0; // Always succeed for dummy implementation
}

// Dummy function for 'constantMap', which is referenced in `main` but not defined in the snippet.
void *constantMap(void) {
    return NULL;
}

// Function: identityMap
void ** identityMap(void) {
  return &dummy_global_offset_table;
}

// Function: absoluteValueMap
void ** absoluteValueMap(int param_1) {
  ushort current_index = 1;
  char *base_address = (char *)(uintptr_t)param_1; // Treat param_1 as a base address for byte arithmetic
  
  // Loop for current_index = 1 and 2
  while (current_index < 3) {
    short value_at_offset = *(short *)(base_address + current_index * 2);
    ushort sign_bit = (ushort)(value_at_offset >> 15); // Extract sign bit for 16-bit short
    
    // Compute absolute value using bitwise operations: abs(x) = (x ^ (x >> 15)) - (x >> 15)
    *(ushort *)(base_address + current_index * 2) = sign_bit + value_at_offset ^ sign_bit;
    
    current_index++; // Increment loop counter
  }
  
  // The original decompiled code returned (undefined **)(local_6 + 1) which evaluates to (void **)3.
  // This is an integer value cast to a pointer. We preserve this behavior.
  return (void **)(uintptr_t)current_index; // Will return (void **)3
}

// Function: modulus
int modulus(short param_1, short param_2) {
  if (param_2 != 0) {
    param_1 %= param_2;
  }
  return (int)param_1;
}

// Function: modulusCoordinatesWithDimensions
void modulusCoordinatesWithDimensions(int param_1) {
  char *base_address = (char *)(uintptr_t)param_1;
  
  // Directly assign the results of modulus to reduce intermediate variables
  *(undefined2 *)(base_address + 6) = (undefined2)modulus(*(short *)(base_address + 6), *(short *)(base_address + 2));
  *(undefined2 *)(base_address + 8) = (undefined2)modulus(*(short *)(base_address + 8), *(short *)(base_address + 4));
}

// Function: processMessage
void processMessage(int *param_1) {
  int message_id = *param_1;
  if (message_id != 0) {
    (*(code *)param_1[2])(message_id); // Call function pointer stored at param_1[2]
    param_1[1] = message_id;
    *param_1 = 0;
  }
}

// Function: receiveMessage
void receiveMessage(int param_1) {
  // The original code involved complex stack manipulation before calling receive().
  // Interpreting this as passing param_1 (incremented each loop) as an argument to receive().
  // The loop runs 10 times.
  for (int i = 0; i < 10; ++i) {
    if (receive(param_1 + i) != 0) {
      _terminate(4); // Call terminate with status 4 on receive error
    }
  }
}

// Function: sendMessage
void sendMessage(undefined4 param_1) {
  // The value 0x111e0 from the original snippet is treated as a decompiler artifact (e.g., return address) and removed.
  if (transmit_all(1, param_1, 0xc) != 0) {
    _terminate(5); // Call terminate with status 5 on transmit error
  }
}

// Function: swap
void swap(int param_1, int param_2) {
  char *base1 = (char *)(uintptr_t)param_1;
  char *base2 = (char *)(uintptr_t)param_2;
  
  undefined4 temp_val = *(undefined4 *)(base1 + 8);
  *(undefined4 *)(base1 + 8) = *(undefined4 *)(base2 + 8);
  *(undefined4 *)(base2 + 8) = temp_val;
}

// Function: permute
void permute(int param_1, int param_2, int param_3, int *param_4, int param_5) {
  if (param_5 != *param_4) { // Check against permutation count limit
    if (param_2 == param_3) {
      (*param_4)++; // Increment permutation count
    } else {
      for (int current_index = param_2; current_index <= param_3; current_index++) {
        // Swap elements to generate permutations. Elements are 12 bytes apart.
        swap((int)(uintptr_t)((char *)(uintptr_t)param_1 + param_2 * 12), 
             (int)(uintptr_t)((char *)(uintptr_t)param_1 + current_index * 12));
        
        permute(param_1, param_2 + 1, param_3, param_4, param_5);
        
        if (param_5 == *param_4) {
          return; // Early exit if target permutation count is reached
        }
        
        // Backtrack: swap elements back to restore original order for next iteration
        swap((int)(uintptr_t)((char *)(uintptr_t)param_1 + param_2 * 12), 
             (int)(uintptr_t)((char *)(uintptr_t)param_1 + current_index * 12));
      }
    }
  }
}

// Function: computeResult
void computeResult(int param_1) {
  char *base_param = (char *)(uintptr_t)param_1;
  short current_e = *(short *)(base_param + 6);
  short current_10 = *(short *)(base_param + 8);
  
  void *allocated_ptr_c = NULL; // Corresponds to *(int *)(param_1 + 0xc)
  void *allocated_ptr_20 = NULL; // Corresponds to local_20 in original decompilation
  
  // Check if the pointer at offset +0xc is null (needs allocation)
  if (*(undefined4 *)(base_param + 0xc) == 0) {
    // Allocate memory for an array of pointers/integers
    size_t size_for_ptrs = (size_t)*(short *)(base_param + 2) * 4; // Size based on param_1+2 (dimension) * 4 bytes per pointer
    allocated_ptr_c = allocate(size_for_ptrs);
    if (allocated_ptr_c == NULL) {
      _terminate(1); // Error status 1 for allocation failure
    }
    *(undefined4 *)(base_param + 0xc) = (undefined4)(uintptr_t)allocated_ptr_c;

    // Allocate memory for actual data
    size_t size_for_data = (size_t)*(short *)(base_param + 4) * *(short *)(base_param + 2) * 2; // Dimensions * 2 bytes per element
    allocated_ptr_20 = allocate(size_for_data);
    if (allocated_ptr_20 == NULL) {
      _terminate(1); // Error status 1 for allocation failure
    }

    // Initialize the array of pointers (allocated_ptr_c) to point into the data block (allocated_ptr_20)
    for (int i = 0; i < *(short *)(base_param + 2); i++) {
      *(undefined4 *)((char *)allocated_ptr_c + i * 4) = 
          (undefined4)((uintptr_t)allocated_ptr_20 + (size_t)*(short *)(base_param + 4) * i * 2);
    }
  } else {
      // If already allocated, retrieve the pointers
      allocated_ptr_c = (void *)(uintptr_t)*(undefined4 *)(base_param + 0xc);
      // Assuming allocated_ptr_20 is the first element pointed to by allocated_ptr_c
      if (allocated_ptr_c != NULL) {
          allocated_ptr_20 = (void*)(uintptr_t)*(undefined4*)allocated_ptr_c;
      }
  }

  // Second loop: modify values based on modulo arithmetic
  // Loop count is product of dimensions
  for (int i = 0; i < (int)*(short *)(base_param + 4) * (int)*(short *)(base_param + 2); i++) {
    // Access a 2D-like array element: *(type *)(row_ptr[current_e] + current_10 * element_size) = value;
    undefined4 *row_ptr_array = (undefined4 *)allocated_ptr_c;
    char *row_base_address = (char *)(uintptr_t)row_ptr_array[current_e];
    *(undefined2 *)(row_base_address + current_10 * 2) = 1; // Set value to 1
    
    // Update current_e and current_10 using modulus
    current_e = (short)modulus((short)(current_e + *(short *)(base_param + 6)), *(short *)(base_param + 2));
    current_10 = (short)modulus((short)(current_10 + *(short *)(base_param + 8)), *(short *)(base_param + 4));
  }
}

// Function: main
int main(void) {
  // local_5c is used as an array of elements. In permute, it's param_1 + N * 12.
  // In processMessage, it's local_5c + N * 3.
  // This suggests elements are 12 bytes, and local_5c + N * 3 is an address to an element's component.
  // Assuming each element is a struct of 3 undefined4 (12 bytes).
  undefined4 local_5c[12]; // Array of 4 elements, each 3 undefined4 (12 bytes), totaling 48 bytes.
  
  // Initialize local_5c to zeros.
  for (int i = 0; i < 12; i++) {
    local_5c[i] = 0;
  }
  
  undefined4 permute_counter = 0; // Corresponds to local_60, used as param_4 in permute
  void *allocated_message_buffer = NULL; // Corresponds to local_2c
  
  // Allocate initial message buffer, size 0x10 (16 bytes)
  allocated_message_buffer = allocate(0x10);
  if (allocated_message_buffer == NULL) {
    _terminate(1); // Terminate on allocation failure
  }
  
  // Receive initial message, passing the allocated buffer's address
  receiveMessage((int)(uintptr_t)allocated_message_buffer);
  
  // Assign function pointers to local variables
  code func_ptr_identity = (code)identityMap;
  code func_ptr_constant = (code)constantMap;
  code func_ptr_absolute = (code)absoluteValueMap;
  code func_ptr_modulus_coords = (code)modulusCoordinatesWithDimensions;
  
  // Call permute function
  // param_1: base address of local_5c array
  // param_2: 0 (start index)
  // param_3: 3 (end index)
  // param_4: pointer to permute_counter
  // param_5: value from the allocated message buffer (first 4 bytes)
  permute((int)(uintptr_t)local_5c, 0, 3, (int *)&permute_counter, *(int *)allocated_message_buffer);
  
  undefined4 message_ptr = 0; // Corresponds to local_34, likely a pointer to a message struct
  
  do { // Main infinite loop
    // Assign the allocated buffer to the first element of local_5c
    local_5c[0] = (undefined4)(uintptr_t)allocated_message_buffer;

    // Process messages in reverse order of local_5c elements (from index 3 down to 0)
    for (int i = 3; i >= 0; i--) {
      // Each element of local_5c is a struct of 3 undefined4s.
      // processMessage is called with the address of the i-th struct.
      processMessage((int *)(local_5c + i * 3));
      
      if (i < 3) {
        // Copy a specific field from the current struct to the next struct
        // local_5c[(i + 1) * 3] is the first field of the (i+1)-th struct.
        // local_5c[i * 3 + 1] is the second field of the i-th struct.
        local_5c[(i + 1) * 3] = local_5c[i * 3 + 1];
      }
    }
    
    // If message_ptr is not null, process and deallocate resources
    if (message_ptr != 0) {
      computeResult(message_ptr);
      sendMessage(message_ptr);
      
      // Deallocation sequence
      // 1. Deallocate data pointed to by the pointer stored at (message_ptr + 0xc),
      //    which itself is a pointer to another pointer (double dereference).
      void *ptr_to_free1 = (void *)(uintptr_t)**(undefined4 **)((char *)(uintptr_t)message_ptr + 0xc);
      if (deallocate(ptr_to_free1) != 0) {
        _terminate(2); // Error status 2 for deallocation failure
      }
      
      // 2. Deallocate the array of pointers stored at (message_ptr + 0xc).
      void *ptr_to_free2 = (void *)(uintptr_t)*(undefined4 *)((char *)(uintptr_t)message_ptr + 0xc);
      if (deallocate(ptr_to_free2) != 0) {
        _terminate(2); // Error status 2 for deallocation failure
      }
      
      // 3. Deallocate the message structure itself (message_ptr).
      void *ptr_to_free3 = (void *)(uintptr_t)message_ptr;
      if (deallocate(ptr_to_free3) != 0) {
        _terminate(2); // Error status 2 for deallocation failure
      }
    }
    
    // Re-allocate message buffer for the next cycle
    allocated_message_buffer = allocate(0x10);
    if (allocated_message_buffer == NULL) {
      _terminate(1); // Terminate on allocation failure
    }
    
    receiveMessage((int)(uintptr_t)allocated_message_buffer);
  } while(1); // Loop indefinitely

  return 0; // main function should return an int
}