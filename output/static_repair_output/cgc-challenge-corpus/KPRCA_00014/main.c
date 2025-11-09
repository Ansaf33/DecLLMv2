#include <stdio.h>    // For basic I/O (not directly used by receive/transmit stubs, but good practice)
#include <string.h>   // For memset
#include <unistd.h>   // For ssize_t (though not strictly needed for the stubs)
#include <stdbool.h>  // For bool type

// --- Global variables ---
// Global state buffer as inferred from memset(state, 0, 0x10040) and various accesses.
// It's treated as a byte array, but also has structured access at specific offsets.
unsigned char g_state[0x10040];

// Global variable DAT_00014000 as inferred from main.
// Assuming it's a 4-byte unsigned integer.
unsigned int DAT_00014000 = 0xDEADBEEF; // Dummy value for compilation

// --- Stub implementations for external functions ---
// These functions are not part of standard C libraries with these exact signatures.
// Providing basic stubs to allow compilation. Their actual behavior would depend on the target environment.

// Mimics a receive function. Returns 0 on success, 1 on error (or bytes read).
// The original code uses `int receive(0, local_10, param_2, &local_14);`
// where `local_14` is an `int` receiving bytes read.
int receive(int fd, void* buf, int count, int* bytes_received) {
    if (count == 0) {
        if (bytes_received) *bytes_received = 0;
        return 0; // Success, 0 bytes read
    }
    // Simulate reading all 'count' bytes successfully.
    if (bytes_received) *bytes_received = count;
    return 0; // Simulate success (0 means success for read_fully's check)
}

// Mimics a transmit function. Returns 0 on success, 1 on error.
// The original code uses `transmit(1, (state._8_4_ & 0xffff) + 0x16100, state._12_4_ & 0xffff, local_18);`
// where `local_18` is an `undefined` array of size 4.
int transmit(int fd, const void* buf, int count, void* status_ptr) {
    // In a real system, this would write from buf to fd.
    // For compilation, we just simulate a write.
    if (status_ptr) {
        // Simulate writing a status, e.g., number of bytes written
        *((int*)status_ptr) = count;
    }
    return 0; // Simulate success
}

// Function: read_fully
// Original signature: undefined4 read_fully(int param_1,int param_2)
// Corrected signature: int read_fully(void* buf, int size_to_read)
int read_fully(void* buf, int size_to_read) {
  int bytes_read_current_call;
  char* current_buf_ptr = (char*)buf; // Use char* for byte-wise pointer arithmetic

  while(size_to_read > 0) {
    if (receive(0, current_buf_ptr, size_to_read, &bytes_read_current_call) != 0) {
      return 1; // Error during receive
    }
    
    // If receive returns 0 bytes but `size_to_read` is still positive, it's a read error/EOF.
    if (bytes_read_current_call == 0) {
      return 1; // Did not read any bytes, but still expected more.
    }

    size_to_read -= bytes_read_current_call;
    current_buf_ptr += bytes_read_current_call;
  }
  return 0; // Fully read
}

// Function: process_syscall
// Original signature: undefined4 process_syscall(void)
int process_syscall(void) {
  // Access state members directly via pointer casting
  unsigned int state_4_4_ = ((unsigned int*)g_state)[1]; // state._4_4_ at offset 4
  unsigned int state_8_4_ = ((unsigned int*)g_state)[2]; // state._8_4_ at offset 8
  unsigned int state_12_4_ = ((unsigned int*)g_state)[3]; // state._12_4_ at offset 12

  unsigned short val_12_4_low = (unsigned short)(state_12_4_ & 0xffff);
  unsigned short val_8_4_low = (unsigned short)(state_8_4_ & 0xffff);

  char transmit_status_buf[4]; // Buffer for transmit status (local_18 in original)

  if (state_4_4_ == 0) {
    if ((val_12_4_low + val_8_4_low) < 0x10001) {
      // (state._8_4_ & 0xffff) + 0x16100 is an address. Cast to void*.
      transmit(1, (void*)((unsigned long)val_8_4_low + 0x16100), val_12_4_low, transmit_status_buf);
      return 0;
    } else {
      return 1;
    }
  } else if (state_4_4_ == 1) {
    if ((val_12_4_low + val_8_4_low) < 0x10001) {
      read_fully((void*)((unsigned long)val_8_4_low + 0x16100), val_12_4_low);
      return 0;
    } else {
      return 1;
    }
  } else {
    return 1;
  }
}

// Function: process
// Original signature: undefined4 process(uint param_1)
int process(unsigned int param_1) {
  unsigned int uVar1_reg_idx = (param_1 >> 0x14) & 0xf;
  unsigned int uVar2_reg_idx = (param_1 >> 0x10) & 0xf;
  short sVar7_immediate = (short)param_1; // Lower 16 bits as signed short

  // Helper macros for common pattern: get value from g_state based on register index.
  // If index is 0, value is 0. Otherwise, read 4-byte value from g_state.
  // This reduces repeated if/else blocks and temporary variables like `iVar4`.
  #define GET_STATE_VALUE_INT(idx) ((idx) == 0 ? 0 : *((int *)(g_state + (idx) * 4)))
  #define GET_STATE_VALUE_UINT(idx) ((idx) == 0 ? 0 : *((unsigned int *)(g_state + (idx) * 4)))

  switch((param_1 >> 0x18) & 0xFF) { // Mask with 0xFF to ensure it's treated as an 8-bit value
    default:
      return 1;
    case 1: { // Read byte from memory into register
      int mem_base_addr = GET_STATE_VALUE_INT(uVar2_reg_idx);
      *((unsigned int *)(g_state + uVar1_reg_idx * 4)) = (unsigned int)((unsigned char)g_state[(mem_base_addr + (param_1 & 0xffff)) + 0x40]);
      return 0;
    }
    case 2: { // Write byte from register into memory
      unsigned char val_to_write = (unsigned char)GET_STATE_VALUE_INT(uVar2_reg_idx); // Cast to char to get byte
      int mem_base_addr = GET_STATE_VALUE_INT(uVar1_reg_idx);
      g_state[(mem_base_addr + (param_1 & 0xffff)) + 0x40] = val_to_write;
      return 0;
    }
    case 3: // Call syscall handler
      return process_syscall();
    case 4: { // Add
      int operand_val = GET_STATE_VALUE_INT(uVar2_reg_idx);
      *((int *)(g_state + uVar1_reg_idx * 4)) += (operand_val + sVar7_immediate);
      return 0;
    }
    case 5: { // Subtract
      int operand_val = GET_STATE_VALUE_INT(uVar2_reg_idx);
      *((int *)(g_state + uVar1_reg_idx * 4)) -= (operand_val + sVar7_immediate);
      return 0;
    }
    case 6: { // Multiply
      int operand_val = GET_STATE_VALUE_INT(uVar2_reg_idx);
      *((int *)(g_state + uVar1_reg_idx * 4)) *= (operand_val + sVar7_immediate);
      return 0;
    }
    case 7: { // Divide
      int divisor_val = GET_STATE_VALUE_INT(uVar2_reg_idx) + sVar7_immediate;
      if (divisor_val == 0) {
        return 1; // Division by zero error
      } else {
        *((unsigned int *)(g_state + uVar1_reg_idx * 4)) /= (unsigned int)divisor_val;
        return 0;
      }
    }
    case 8: { // Bitwise OR
      unsigned int operand_val = GET_STATE_VALUE_UINT(uVar2_reg_idx);
      *((unsigned int *)(g_state + uVar1_reg_idx * 4)) |= (operand_val | (unsigned int)sVar7_immediate);
      return 0;
    }
    case 9: { // Bitwise AND
      unsigned int operand_val = GET_STATE_VALUE_UINT(uVar2_reg_idx);
      *((unsigned int *)(g_state + uVar1_reg_idx * 4)) &= (operand_val | (unsigned int)sVar7_immediate);
      return 0;
    }
    case 10: { // Bitwise XOR
      unsigned int operand_val = GET_STATE_VALUE_UINT(uVar2_reg_idx);
      *((unsigned int *)(g_state + uVar1_reg_idx * 4)) ^= (operand_val | (unsigned int)sVar7_immediate);
      return 0;
    }
    case 0xb: { // Less than (unsigned comparison)
      unsigned int operand1_val = GET_STATE_VALUE_UINT(uVar2_reg_idx);
      unsigned int operand2_reg_idx = param_1 & 0xf; // The lowest 4 bits are another register index
      unsigned int operand2_val = GET_STATE_VALUE_UINT(operand2_reg_idx);
      *((unsigned int *)(g_state + uVar1_reg_idx * 4)) = (unsigned int)(operand1_val < operand2_val);
      return 0;
    }
    case 0xc: { // Less than or equal (unsigned comparison)
      unsigned int operand1_val = GET_STATE_VALUE_UINT(uVar2_reg_idx);
      unsigned int operand2_reg_idx = param_1 & 0xf;
      unsigned int operand2_val = GET_STATE_VALUE_UINT(operand2_reg_idx);
      *((unsigned int *)(g_state + uVar1_reg_idx * 4)) = (unsigned int)(operand1_val <= operand2_val);
      return 0;
    }
  }
}

// Function: main
// Original signature: undefined4 main(void)
int main(void) {
  int read_status;
  unsigned int magic_val;   // Corresponds to local_18 in original
  unsigned int data_len;    // Corresponds to local_1c in original
  unsigned int instruction; // Corresponds to local_20 in original
  char transmit_status_buf[4]; // Corresponds to local_14 in original

  // local_10 = &stack0x00000004; -- Removed as it's a decompilation artifact.

  memset(g_state, 0, 0x10040);

  // Initial sequence of reads and checks
  // The complex conditional from the original is broken down for readability.
  read_status = read_fully(&magic_val, 4);
  if (read_status != 0 || magic_val != 0x69564d00) {
    transmit(1, "BAD_INIT", 8, transmit_status_buf);
    return 0;
  }

  read_status = read_fully(&data_len, 4);
  if (read_status != 0) {
    transmit(1, "BAD_INIT", 8, transmit_status_buf);
    return 0;
  }

  // Original: ((-1 < (int)local_1c || (iVar1 = read_fully(state,0x40), iVar1 == 0)))
  // This evaluates to: (data_len != 0xFFFFFFFF || (read_fully(g_state, 0x40) == 0))
  // If this condition is FALSE, then "BAD_INIT".
  if (!((int)data_len != -1 || (read_fully(g_state, 0x40) == 0))) {
    transmit(1, "BAD_INIT", 8, transmit_status_buf);
    return 0;
  }

  // Original: (((local_1c & 0xffff) == 0 || (iVar1 = read_fully(0x16100,local_1c & 0xffff), iVar1 == 0))))
  // If lower 16 bits of data_len are 0, skip the read. Otherwise, read into 0x16100.
  unsigned short data_len_low_16_bits = (unsigned short)(data_len & 0xffff);
  if (!(data_len_low_16_bits == 0 || (read_fully((void*)(unsigned long)0x16100, data_len_low_16_bits) == 0))) {
    transmit(1, "BAD_INIT", 8, transmit_status_buf);
    return 0;
  }

  // Main processing loop
  while (true) {
    read_status = read_fully(&instruction, 4);
    if (read_status != 0) {
      break; // Error reading instruction, exit loop
    }

    read_status = process(instruction);
    if (read_status != 0) {
      break; // Error processing instruction, exit loop
    }

    transmit(1, g_state, 0x40, transmit_status_buf);
  }

  transmit(1, &DAT_00014000, 4, transmit_status_buf);

  return 0;
}