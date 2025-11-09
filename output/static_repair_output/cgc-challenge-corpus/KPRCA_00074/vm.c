#include <stdio.h>    // For FILE, stdin, stdout, fflush, fread, fwrite
#include <stdlib.h>   // For malloc, free, rand
#include <string.h>   // For memset
#include <stdint.h>   // For unsigned int, uintptr_t, size_t
#include <stdbool.h>  // For bool

// Placeholder declarations for global tables.
// These tables are arrays of void* which act as "instruction pointers" or state identifiers.
// The original code implies they are global and defined elsewhere.
// Sizes are determined by the index operations (e.g., & 7 for type_table_2 means size 8).
void* type_table_2[8];
void* alu_table_1[16]; // Used with (*pbVar2 >> 4) which results in indices 0-15
void* jmp_table_0[16]; // Used with (*pbVar2 >> 4) which results in indices 0-15

// Function: swap32
unsigned int swap32(unsigned int param_1) {
  return (param_1 << 0x18) |
         (param_1 >> 0x18) |
         ((param_1 >> 0x10) & 0xff) << 8 |
         ((param_1 >> 8) & 0xff) << 0x10;
}

// Function: valid_mem
// param_1: Assumed to point to an array of unsigned ints: [base_address, size_of_region, extra_field].
//          This interpretation is made to make the original decompiler's logic compilable,
//          as its original use of `*(uint *)param_1[1]` and `*(int *)(param_1[1] + 4)`
//          is inconsistent with `new_mem` if `param_1[1]` is just a size value.
//          This assumes `param_1[1]` holds a value, and `param_1[2]` is the "param_1[1]+4" equivalent
//          due to `param_1` being `unsigned int*`.
// param_2: offset from base_address
// param_3: length of access
unsigned int valid_mem(unsigned int *param_1, int param_2, int param_3) {
  if (param_3 < 0) {
    return 0;
  }
  // Original condition:
  // ((uint)(*param_1 + param_2) < *(uint *)param_1[1]) ||
  // ((uint)(*(int *)(param_1[1] + 4) + *(int *)param_1[1]) < (uint)(param_3 + param_2 + *param_1))
  // Interpreting `*(uint *)param_1[1]` as `param_1[1]` (the size value).
  // Interpreting `*(int *)(param_1[1] + 4)` as `param_1[2]` (the next unsigned int in the array).
  if (((unsigned int)(param_1[0] + param_2) < param_1[1]) ||
      ((unsigned int)(param_1[2] + param_1[1]) < (unsigned int)(param_3 + param_2 + param_1[0]))) {
    return 0;
  }
  return 1;
}

// Function: new_mem
// param_1: Pointer to a location where base and size will be stored. Assumed unsigned int*.
// param_2: Base address value.
// param_3: Size value.
unsigned int * new_mem(unsigned int *param_1, unsigned int param_2, unsigned int param_3) {
  if (param_1 == NULL) {
    param_1 = (unsigned int *)malloc(8); // Allocate space for two unsigned ints (base and size)
    if (param_1 == NULL) { // Check for malloc failure
        return NULL;
    }
  }
  param_1[0] = param_2; // Store base address
  param_1[1] = param_3; // Store size
  return param_1;
}

// Function: filter_execute
unsigned int filter_execute(int param_1, int param_2, unsigned int param_3) {
  void *mem_buffer;
  unsigned int *regs_ptr; // Assuming this is a register block, array of unsigned ints
  unsigned int instr_ptr_base;
  unsigned char *opcode_ptr;
  void* current_dispatch_target;
  unsigned int val_to_write;
  unsigned int return_status = 0; // 0 for success, 0xffffffff for error

  mem_buffer = malloc(0x400);
  regs_ptr = (unsigned int *)malloc(0x98); // 0x98 bytes / 4 bytes per unsigned int = 38 unsigned ints
  if (!mem_buffer || !regs_ptr) {
      free(mem_buffer);
      free(regs_ptr);
      return 0xffffffff;
  }

  regs_ptr[0] = (unsigned int)param_1; // param_1 (instruction base address) stored in regs_ptr[0]
  regs_ptr[5] = 0; // Instruction counter/program counter
  memset(regs_ptr + 6, 0, 0x80); // Clear 0x80 bytes (32 unsigned ints) starting from regs_ptr[6]

  // Create memory region 1 descriptor: regs_ptr[1] (base), regs_ptr[2] (size)
  // `new_mem` returns a pointer to the `[base, size]` block it initialized.
  unsigned int* region1_desc_ptr = new_mem(regs_ptr + 1, (unsigned int)param_2, param_3);
  // Store the pointer to this descriptor block in regs_ptr[7].
  // Note: This means regs_ptr[7] holds a pointer, while valid_mem expects regs_ptr[7] to be a size.
  // This is an inherent inconsistency in the original code's register usage.
  regs_ptr[7] = (unsigned int)(uintptr_t)region1_desc_ptr;

  // Create memory region 2 descriptor: regs_ptr[3] (base), regs_ptr[4] (size)
  unsigned int* region2_desc_ptr = new_mem(regs_ptr + 3, (unsigned int)mem_buffer, 0x400);
  // Store the pointer to this descriptor block in regs_ptr[0x25].
  regs_ptr[0x25] = (unsigned int)(uintptr_t)region2_desc_ptr;

  memset(mem_buffer, 0, 0x400);

  bool keep_running_outer_loop = true;
  while (keep_running_outer_loop) { // Main instruction fetch and execute loop (replaces LAB_00011f7a)
    instr_ptr_base = (unsigned int)param_1 + regs_ptr[5] * 8; // Calculate current instruction address
    opcode_ptr = (unsigned char *)(instr_ptr_base + 4); // Opcode byte at offset +4 from instruction start
    regs_ptr[5]++; // Increment instruction counter

    // Initial dispatch target based on lower 3 bits of opcode
    current_dispatch_target = type_table_2[*opcode_ptr & 7];

    bool instruction_completed_this_cycle = false;
    while (!instruction_completed_this_cycle) { // Inner dispatch loop (replaces do-while(true) with switch)
      // Use uintptr_t for comparing void* with literal addresses (compiler specific, but common)
      if (current_dispatch_target == (void*)(uintptr_t)0x11fb6) { // Corresponds to read operation and caseD_11fb6 label
        unsigned int source_reg_idx_shifted = (*(unsigned char *)(instr_ptr_base + 5) >> 4);
        // Calculate index for the region descriptor in regs_ptr
        // e.g., if source_reg_idx_shifted=0, idx=6. If source_reg_idx_shifted=1, idx=8.
        unsigned int source_reg_desc_start_idx = (source_reg_idx_shifted + 2) * 2 + 2;
        int access_offset = *(unsigned short *)(instr_ptr_base + 6);
        int access_length = 1 << (*opcode_ptr >> 3 & 3); // 1, 2, or 4 bytes

        // Call valid_mem to check memory access.
        // Assumes regs_ptr[source_reg_desc_start_idx] is base, regs_ptr[source_reg_desc_start_idx + 1] is size,
        // and regs_ptr[source_reg_desc_start_idx + 2] is the extra field for valid_mem.
        if (valid_mem(regs_ptr + source_reg_desc_start_idx, access_offset, access_length) == 0) {
          return_status = 0xffffffff;
          keep_running_outer_loop = false;
        } else {
          unsigned int access_address = regs_ptr[source_reg_desc_start_idx] + (unsigned int)access_offset;
          if ((*opcode_ptr & 0x18) == 0) { // Byte access (0x00)
            val_to_write = (unsigned int)*(unsigned char *)access_address;
          } else if ((*opcode_ptr & 0x18) == 8) { // Short access (0x08)
            val_to_write = (unsigned int)*(unsigned short *)access_address;
          } else if ((*opcode_ptr & 0x18) == 0x10) { // Word access (0x10)
            val_to_write = *(unsigned int *)access_address;
          } else {
            return_status = 0xffffffff; // Unknown access size
            keep_running_outer_loop = false;
          }

          if (return_status == 0) { // Only proceed if no error occurred
            unsigned int dest_reg_idx = (*(unsigned char *)(instr_ptr_base + 5) & 0xf);
            unsigned int dest_reg_addr_idx = (dest_reg_idx + 2) * 2 + 2;
            regs_ptr[dest_reg_addr_idx] = val_to_write;

            // If bit 0x20 of opcode is set, update the source register's base address
            if ((*opcode_ptr & 0xe0) == 0x20) {
              regs_ptr[source_reg_desc_start_idx] += (unsigned int)access_offset + access_length;
            }
          }
        }
        instruction_completed_this_cycle = true; // This instruction type completes the cycle, continue outer loop
      } else if (current_dispatch_target == (void*)(uintptr_t)0x1212a) { // Corresponds to write operation
        unsigned int dest_reg_idx = (*(unsigned char *)(instr_ptr_base + 5) & 0xf);
        unsigned int dest_reg_desc_start_idx = (dest_reg_idx + 2) * 2 + 2;
        int access_offset = *(unsigned short *)(instr_ptr_base + 6);
        int access_length = 1 << (*opcode_ptr >> 3 & 3); // 1, 2, or 4 bytes

        if (valid_mem(regs_ptr + dest_reg_desc_start_idx, access_offset, access_length) == 0) {
          return_status = 0xffffffff;
          keep_running_outer_loop = false;
        } else {
          // Determine value to write: immediate or from source register
          if ((*opcode_ptr & 7) == 1) { // If bit 0 of opcode is 1, use immediate value
            val_to_write = *(unsigned int *)(instr_ptr_base + 8);
          } else { // Else, use value from source register
            unsigned int source_reg_idx_shifted = (*(unsigned char *)(instr_ptr_base + 5) >> 4);
            unsigned int source_reg_addr_idx = (source_reg_idx_shifted + 2) * 2 + 2;
            val_to_write = regs_ptr[source_reg_addr_idx];
          }

          unsigned int access_address = regs_ptr[dest_reg_desc_start_idx] + (unsigned int)access_offset;
          if ((*opcode_ptr & 0x18) == 0) { // Byte write
            *(unsigned char *)access_address = (unsigned char)val_to_write;
          } else if ((*opcode_ptr & 0x18) == 8) { // Short write
            *(unsigned short *)access_address = (unsigned short)val_to_write;
          } else if ((*opcode_ptr & 0x18) == 0x10) { // Word write
            *(unsigned int *)access_address = val_to_write;
          } else {
            return_status = 0xffffffff; // Unknown access size
            keep_running_outer_loop = false;
          }

          if (return_status == 0) { // Only proceed if no error occurred
            // If bit 0x20 of opcode is set, update the destination register's base address
            if ((*opcode_ptr & 0xe0) == 0x20) {
              regs_ptr[dest_reg_desc_start_idx] += (unsigned int)access_offset + access_length;
            }
          }
        }
        instruction_completed_this_cycle = true; // This instruction type completes the cycle, continue outer loop
      } else if (current_dispatch_target == (void*)(uintptr_t)0x122ab) { // Corresponds to ALU operation
        // Determine value for ALU operation: immediate or from source register
        if ((*opcode_ptr & 8) == 0) { // If bit 3 of opcode is 0, use source register value
          unsigned int source_reg_idx_shifted = (*(unsigned char *)(instr_ptr_base + 5) >> 4);
          unsigned int source_reg_addr_idx = (source_reg_idx_shifted + 2) * 2 + 2;
          val_to_write = regs_ptr[source_reg_addr_idx];
        } else { // Else, use immediate value
          val_to_write = *(unsigned int *)(instr_ptr_base + 8);
        }
        // Update dispatch target for next iteration of inner loop (alu_table_1 dispatch)
        // Original: puVar3 = *(undefined **)(alu_table_1 + (uint)(*pbVar2 >> 4) * 4);
        // Corrected for array of void* pointers:
        current_dispatch_target = alu_table_1[(*opcode_ptr >> 4) & 0xF];
        // This instruction type does not complete the cycle; re-dispatch with the new target.
      } else if (current_dispatch_target == (void*)(uintptr_t)0x1248f) { // Corresponds to JMP operation
        // Determine value for JMP operation: immediate or from source register
        if ((*opcode_ptr & 8) == 0) { // If bit 3 of opcode is 0, use source register value
          unsigned int source_reg_idx_shifted = (*(unsigned char *)(instr_ptr_base + 5) >> 4);
          unsigned int source_reg_addr_idx = (source_reg_idx_shifted + 2) * 2 + 2;
          val_to_write = regs_ptr[source_reg_addr_idx];
        } else { // Else, use immediate value
          val_to_write = *(unsigned int *)(instr_ptr_base + 8);
        }
        // Update dispatch target for next iteration of inner loop (jmp_table_0 dispatch)
        // Original: puVar3 = *(undefined **)(jmp_table_0 + (uint)(*pbVar2 >> 4) * 4);
        // Corrected for array of void* pointers:
        current_dispatch_target = jmp_table_0[(*opcode_ptr >> 4) & 0xF];
        // This instruction type does not complete the cycle; re-dispatch with the new target.
      } else {
        // Unknown dispatch target, treat as error
        return_status = 0xffffffff;
        keep_running_outer_loop = false;
        instruction_completed_this_cycle = true;
      }

      // If an error occurred or instruction completed, break out of the inner dispatch loop
      if (return_status != 0 || instruction_completed_this_cycle) {
        break;
      }
    } // End of inner dispatch loop (while (!instruction_completed_this_cycle))

    // If an error occurred, break out of the outer instruction loop
    if (return_status != 0) {
      break;
    }
  } // End of outer instruction loop (while (keep_running_outer_loop))

  free(mem_buffer);
  free(regs_ptr);
  return return_status;
}

// Function: syscall_receive
// param_1: Assumed to point to a structure containing:
//          - A pointer to a region descriptor (base, size, extra_field) at offset 0x18.
//          - A count (size_t) at offset 0x20.
unsigned int syscall_receive(int param_1) {
  // Cast param_1 to unsigned char* for byte-accurate pointer arithmetic
  unsigned char *syscall_data_ptr = (unsigned char *)param_1;

  // Read count from param_1 + 0x20
  size_t bytes_to_read = *(size_t *)(syscall_data_ptr + 0x20);

  // Validate the memory region specified by param_1 + 0x18.
  // This region descriptor is assumed to be an array of unsigned ints: [base, size, extra_field].
  // The base address within this descriptor is also the buffer for fread.
  if (valid_mem((unsigned int*)(syscall_data_ptr + 0x18), 0, (int)bytes_to_read) == 0) {
    return 0; // Return 0 on validation failure
  } else {
    fflush(stdout); // Flush stdout before reading from stdin
    // Read into the buffer whose address is stored at param_1 + 0x18
    size_t bytes_read = fread(*(void **)(syscall_data_ptr + 0x18), 1, bytes_to_read, stdin);
    
    // Original code: *(size_t *)(param_1 + 0x18) = bytes_read;
    // This would overwrite the buffer pointer with the bytes read.
    // Assuming it meant to update the count field (at 0x20) with the actual bytes read.
    *(size_t *)(syscall_data_ptr + 0x20) = bytes_read;

    // Original return: ~sVar3 >> 0x1f;
    // This evaluates to 1 if bytes_read is 0, and 0 if bytes_read is non-zero.
    return (bytes_read == 0);
  }
}

// Function: syscall_transmit
// param_1: Assumed to point to a structure containing:
//          - A pointer to a region descriptor (base, size, extra_field) at offset 0x18.
//          - A count (size_t) at offset 0x20.
unsigned int syscall_transmit(int param_1) {
  unsigned char *syscall_data_ptr = (unsigned char *)param_1;

  size_t bytes_to_write = *(size_t *)(syscall_data_ptr + 0x20);

  // Validate the memory region specified by param_1 + 0x18.
  if (valid_mem((unsigned int*)(syscall_data_ptr + 0x18), 0, (int)bytes_to_write) == 0) {
    return 0; // Return 0 on validation failure
  } else {
    // Write from the buffer whose address is stored at param_1 + 0x18
    size_t bytes_written = fwrite(*(void **)(syscall_data_ptr + 0x18), 1, bytes_to_write, stdout);
    
    // Original code: *(size_t *)(param_1 + 0x18) = sVar3;
    // Assuming it meant to update the count field (at 0x20) with the actual bytes written.
    *(size_t *)(syscall_data_ptr + 0x20) = bytes_written;

    // Original return: ~sVar3 >> 0x1f;
    // This evaluates to 1 if bytes_written is 0, and 0 if bytes_written is non-zero.
    return (bytes_written == 0);
  }
}

// Function: syscall_random
// param_1: Assumed to point to a structure containing:
//          - A pointer to a region descriptor (base, size, extra_field) at offset 0x18.
//          - A length (unsigned int) at offset 0x20.
bool syscall_random(int param_1) {
  unsigned char *syscall_data_ptr = (unsigned char *)param_1;

  // The third argument to valid_mem is the length of access.
  unsigned int length = *(unsigned int *)(syscall_data_ptr + 0x20);

  // Validate the memory region specified by param_1 + 0x18 for the given length.
  int is_valid = valid_mem((unsigned int*)(syscall_data_ptr + 0x18), 0, (int)length);
  if (is_valid != 0) {
    rand(); // Original uses random(), but rand() is standard for simple cases.
            // For proper randomness, `srand()` would be needed to seed it.
  }
  return is_valid != 0;
}