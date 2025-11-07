#include <stdio.h>   // For NULL, basic I/O (not strictly used but good practice)
#include <stdlib.h>  // For NULL, potential future malloc/free if allocate/deallocate are implemented
#include <string.h>  // For memset

// Define types based on common disassembler output
typedef char undefined;
typedef short undefined2;
typedef int undefined4;
typedef unsigned short ushort;

// Define a generic function pointer type based on usage in processMessage
typedef void (*code)(int);

// Declare external functions and global variables (placeholders for compilation)
extern char **_GLOBAL_OFFSET_TABLE_; // Assumed to be a pointer to a pointer to char
extern int receive(void);
extern void _terminate(int); // Deduced from calls with arguments
extern int transmit_all(int, int, int, int); // Deduced from sendMessage
extern int allocate(int); // Deduced from computeResult and main
extern int deallocate(int, int); // Deduced from main
extern void *constantMap(void); // Declared as used in main but not defined

// Function: identityMap
char **identityMap(void) {
  return _GLOBAL_OFFSET_TABLE_;
}

// Function: absoluteValueMap
char **absoluteValueMap(int param_1) {
  for (ushort local_6 = 1; local_6 < 3; local_6++) {
    short val = *(short *)(param_1 + local_6 * 2);
    short sign_ext = val >> 15; // Sign extension for 16-bit
    *(short *)(param_1 + local_6 * 2) = (sign_ext + val) ^ sign_ext;
  }
  // This return value (e.g., (char**)4) is likely an artifact of register usage
  // in the original assembly. Returning NULL for compilability.
  return NULL;
}

// Function: modulus
int modulus(short param_1, short param_2) {
  if (param_2 != 0) {
    return param_1 % param_2;
  }
  return (int)param_1;
}

// Function: modulusCoordinatesWithDimensions
void modulusCoordinatesWithDimensions(int param_1) {
  *(undefined2 *)(param_1 + 6) = modulus(*(short *)(param_1 + 6), *(short *)(param_1 + 2));
  *(undefined2 *)(param_1 + 8) = modulus(*(short *)(param_1 + 8), *(short *)(param_1 + 4));
}

// Function: processMessage
void processMessage(int *param_1) {
  if (*param_1 != 0) {
    (*(code)param_1[2])(*param_1);
    param_1[1] = *param_1;
    *param_1 = 0;
  }
}

// Function: receiveMessage
void receiveMessage(int param_1) {
  int local_18 = 0;
  int current_param = param_1; // Maps to local_14 in original
  // Simulate stack frame for arguments/local values
  char stack_buffer[20]; // Sufficient for 5 undefined4 (int) values

  for (int local_10 = 10; local_10 != 0; local_10--) { // local_10 from 10 down to 1
    int iVar1 = current_param; // iVar1 = local_14
    current_param++;           // local_14 = local_14 + 1

    // Simulate stack writes, assuming stack_buffer + 20 is the "puVar3" base
    *(int **)(stack_buffer + 16) = &local_18; // puVar3 + -4
    *(int *)(stack_buffer + 12) = 1;          // puVar3 + -8
    *(int *)(stack_buffer + 8) = iVar1;       // puVar3 + -0xc
    *(int *)(stack_buffer + 4) = 0;           // puVar3 + -0x10
    *(int *)(stack_buffer + 0) = 0x111b2;    // puVar3 + -0x14

    if (receive() != 0) {
      _terminate(4); // Deduced argument from `*(undefined4 *)(puVar3 + -0x10) = 4;`
    }
    // `puVar3 = puVar2;` logic is effectively a no-op if _terminate is called,
    // or if receive() returns 0. So no actual adjustment to stack_ptr here.
  }
}

// Function: sendMessage
void sendMessage(undefined4 param_1) {
  if (transmit_all(1, param_1, 0xc, 0x111e0) != 0) {
    _terminate(5);
  }
}

// Function: swap
void swap(int param_1, int param_2) {
  int temp = *(undefined4 *)(param_1 + 8);
  *(undefined4 *)(param_1 + 8) = *(undefined4 *)(param_2 + 8);
  *(undefined4 *)(param_2 + 8) = temp;
}

// Function: permute
void permute(int param_1, int param_2, int param_3, int *param_4, int param_5) {
  if (param_5 != *param_4) {
    if (param_2 == param_3) {
      (*param_4)++;
    } else {
      for (int local_10 = param_2; local_10 <= param_3; local_10++) {
        swap(param_1 + param_2 * 0xc, local_10 * 0xc + param_1);
        permute(param_1, param_2 + 1, param_3, param_4, param_5);
        if (param_5 == *param_4) {
          return;
        }
        swap(param_1 + param_2 * 0xc, local_10 * 0xc + param_1);
      }
    }
  }
}

// Function: computeResult
void computeResult(int param_1) {
  short local_e = *(short *)(param_1 + 6);
  short local_10 = *(short *)(param_1 + 8);
  int local_20; // Result of an allocate call

  if (*(undefined4 *)(param_1 + 0xc) == 0) {
    // First allocate call. Arguments are inferred as 0x10 for size
    if (allocate(0x10) != 0) { // Assuming allocate takes a size argument
      _terminate(1); // Deduced argument for _terminate
    }

    short sVar1 = *(short *)(param_1 + 2); // param_1 + 2
    short sVar2 = *(short *)(param_1 + 4); // param_1 + 4
    // Second allocate call. Arguments are inferred from stack writes.
    // `(int)sVar2 * (int)sVar1 * 2` is the size.
    local_20 = allocate((int)sVar2 * (int)sVar1 * 2);
    if (local_20 != 0) {
      _terminate(1); // Deduced argument for _terminate
    }

    // Initialize allocated memory
    for (int local_14 = 0; local_14 < sVar1; local_14++) {
      *(undefined4 *)(*(undefined4 *)(param_1 + 0xc) + local_14 * 4) =
          local_20 + sVar2 * local_14 * 2;
    }
  }

  // Main loop for result computation
  for (int local_18 = 0; local_18 < (int)*(short *)(param_1 + 4) * (int)*(short *)(param_1 + 2);
       local_18++) {
    *(undefined2 *)(*(undefined4 *)(*(undefined4 *)(param_1 + 0xc) + local_e * 4) + local_10 * 2) =
        1;

    // Call to modulus with inferred arguments from stack writes
    // `local_e = modulus((short)(local_e + *(short *)(param_1 + 6)), *(short *)(param_1 + 2));`
    local_e = modulus((short)(local_e + *(short *)(param_1 + 6)), *(short *)(param_1 + 2));

    // Another call to modulus with inferred arguments
    // `local_10 = modulus((short)(local_10 + *(short *)(param_1 + 8)), *(short *)(param_1 + 4));`
    local_10 = modulus((short)(local_10 + *(short *)(param_1 + 8)), *(short *)(param_1 + 4));
  }
}

// Function: main
void main(void) {
  // `local_2c` is a `short *`, initialized to NULL.
  short *local_2c = NULL;
  // `local_5c` is treated as a base address for an array of 12-byte structs/blocks.
  // `permute`'s param_3 is 3, implying 4 elements (0 to 3). So 4 * 12 = 48 bytes.
  char local_5c_buffer[4 * 12]; // Array of 4 elements, each 12 bytes long

  // Initialize `local_5c_buffer` to zeros.
  // The original loop initializes 12 `short *` sized locations, which is inconsistent
  // with the 4-element 12-byte struct usage later.
  // A full memset is safer for compilation.
  memset(local_5c_buffer, 0, sizeof(local_5c_buffer));

  // `local_28 = allocate();` in original implies `allocate()` with size `0x10` from `uStack_70`.
  if (allocate(0x10) != 0) {
    _terminate(1); // Deduced argument for _terminate
  }

  // `receiveMessage()` call, inferred argument `local_2c`.
  receiveMessage((int)local_2c);

  code local_54 = (code)identityMap; // Cast to common function pointer type
  void *local_48 = constantMap(); // constantMap is not defined, assumed to return void*
  code local_3c = (code)absoluteValueMap;
  code local_30 = modulusCoordinatesWithDimensions;

  int local_60 = 0; // Used as `param_4` for permute
  int local_34 = 0; // Uninitialized in original, causing potential crash. Initialize for safety.

  // `permute()` call with inferred arguments from stack writes
  // `param_1`: (int)local_5c_buffer
  // `param_2`: 0
  // `param_3`: 3
  // `param_4`: &local_60
  // `param_5`: (int)*local_2c. `local_2c` is NULL, dereferencing would crash. Assuming 0.
  permute((int)local_5c_buffer, 0, 3, &local_60, 0);

  do {
    // `local_5c[0] = local_2c;` -> `*(short **)(local_5c_buffer) = local_2c;`
    // This assigns `local_2c` (a `short *`) to the first `short *` sized slot in `local_5c_buffer`.
    *(short **)(local_5c_buffer) = local_2c;

    for (int local_24 = 3; local_24 >= -1; local_24--) {
      // `processMessage()` call with inferred argument: `(int*)(local_5c_buffer + local_24 * 12)`
      processMessage((int *)(local_5c_buffer + local_24 * 12));

      if (local_24 < 3) {
        // `local_5c[(local_24 + 1) * 3] = local_5c[local_24 * 3 + 1];`
        // Interpreted as copying the second `int` from the `local_24`-th 12-byte block
        // to the first `int` of the `(local_24+1)`-th 12-byte block.
        *(int *)(local_5c_buffer + (local_24 + 1) * 12) =
            *(int *)(local_5c_buffer + local_24 * 12 + sizeof(int));
      }
    }

    if (local_34 != 0) {
      computeResult(local_34);
      sendMessage(local_34);

      // Deallocate calls, arguments inferred from stack writes
      undefined4 uVar1_dealloc_arg1 = **(undefined4 **)(local_34 + 0xc);
      int iVar_dealloc_arg2 =
          (int)*(short *)(local_34 + 4) * (int)*(short *)(local_34 + 2) * 2;
      if (deallocate(iVar_dealloc_arg2, uVar1_dealloc_arg1) != 0) {
        _terminate(2); // Deduced argument for _terminate
      }

      uVar1_dealloc_arg1 = *(undefined4 *)(local_34 + 0xc);
      iVar_dealloc_arg2 = *(short *)(local_34 + 2) * 4;
      if (deallocate(iVar_dealloc_arg2, uVar1_dealloc_arg1) != 0) {
        _terminate(2); // Deduced argument for _terminate
      }

      if (deallocate(0x10, local_34) != 0) {
        _terminate(2); // Deduced argument for _terminate
      }
    }

    // Allocate again, inferred size 0x10
    if (allocate(0x10) != 0) {
      _terminate(1); // Deduced argument for _terminate
    }

    // Receive message again
    receiveMessage((int)local_2c);

  } while (1); // Original `do ... while(true)`
}