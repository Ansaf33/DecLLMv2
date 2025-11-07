#include <stdbool.h> // For bool
#include <stdint.h>  // For uint8_t, uint16_t, uint32_t, uintptr_t
#include <stdlib.h>  // For calloc, free
#include <string.h>  // For memcmp, strlen
#include <unistd.h>  // For read, write (assuming receive/transmit map to these)

// Custom type definitions based on common patterns and variable sizes
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint32_t undefined4; // Generic 4-byte unsigned
typedef uint8_t undefined;   // Generic 1-byte unsigned

// Define the VM state structure
// 16 registers (4 bytes each) + 65536 bytes of memory
#define VM_REG_COUNT 16
#define VM_REG_BLOCK_SIZE (VM_REG_COUNT * sizeof(uint32_t))
#define VM_MEM_SIZE 0x10000 // 65536 bytes
#define VM_STATE_SIZE (VM_REG_BLOCK_SIZE + VM_MEM_SIZE)

typedef struct {
    uint32_t regs[VM_REG_COUNT];
    uint8_t mem[VM_MEM_SIZE];
} vm_state;

// Dummy external functions for compilation.
// These would be provided by the environment in a real system.
// `receive_bytes_helper` is a specific helper used by `read_n_bytes`
// that returns 0 on success and populates `*bytes_read_ptr`.
// `receive` and `transmit` are used by `process_sys` and `main` and map to `read`/`write`.
int receive_bytes_helper(int fd, void *buf, unsigned int len, int *bytes_read_ptr) {
    ssize_t res = read(fd, buf, len);
    if (res < 0) return -1; // Error
    *bytes_read_ptr = (int)res;
    return 0; // Success
}

ssize_t receive(int fd, void *buf, size_t count, int flags) {
    (void)flags; // Unused
    return read(fd, buf, count);
}

ssize_t transmit(int fd, const void *buf, size_t count, int flags) {
    (void)flags; // Unused
    return write(fd, buf, count);
}


// Function: read_n_bytes
// Reads exactly `count` bytes into `buf` from `fd`.
// Returns total bytes read on success, or 0xffffffff on error.
// If less than `count` bytes are read (EOF), returns the partial count.
uint read_n_bytes(int fd, uint count, void *buf_ptr) {
  if (count == 0 || buf_ptr == NULL) {
    return 0xffffffff;
  }
  
  unsigned int total_read = 0;
  while (total_read < count) {
    int bytes_read_this_call;
    int iVar1 = receive_bytes_helper(fd, (char*)buf_ptr + total_read, count - total_read, &bytes_read_this_call);
    
    if (iVar1 != 0) { // Error from receive_bytes_helper
      return 0xffffffff;
    }
    if (bytes_read_this_call == 0) { // EOF or no bytes read
      return total_read; // Return partial count
    }
    total_read += bytes_read_this_call;
  }
  return total_read;
}

// Function: read_header
uint read_header(int fd) {
  uint header_val;
  uint expected_header_magic = 0x69564d00; // "MVi\0" or similar magic
  
  if (read_n_bytes(fd, 4, &header_val) == 4) {
    return (uint)(memcmp(&expected_header_magic, &header_val, 4) == 0);
  }
  return 0xffffffff;
}

// Function: read_flags
undefined4 read_flags(int fd, int *error_flag) {
  uint flags_val;
  
  if (read_n_bytes(fd, 4, &flags_val) != 4) {
    *error_flag = 1; // Indicate error
  }
  return flags_val;
}

// Function: init_state
void * init_state(int fd, uint flags) {
  vm_state *state = (vm_state*)calloc(1, VM_STATE_SIZE);
  if (state == NULL) {
    return NULL;
  }

  uint reg_read_flag = flags & 0x80000000; // Check if registers need to be read
  uint mem_read_size = flags & 0xffff;     // Size of initial memory to read

  // Read registers if flag is set
  if (reg_read_flag != 0) {
    if (read_n_bytes(fd, VM_REG_BLOCK_SIZE, state->regs) != VM_REG_BLOCK_SIZE) {
      free(state);
      return NULL;
    }
  }
  
  // Read initial memory if size is non-zero
  if (mem_read_size != 0) {
    if (read_n_bytes(fd, mem_read_size, state->mem) != mem_read_size) {
      free(state);
      return NULL;
    }
  }

  return state;
}

// Function: dump_regs
bool dump_regs(int fd, vm_state *state) {
  // Transmit the register block (first 0x40 bytes)
  return transmit(fd, state->regs, VM_REG_BLOCK_SIZE, 0) == VM_REG_BLOCK_SIZE;
}

// Function: process_load
// Loads a byte from memory into a register
undefined4 process_load(vm_state *state, byte dest_reg_idx, byte src_reg_idx, ushort offset) {
  if (dest_reg_idx >= VM_REG_COUNT || (src_reg_idx != 0 && src_reg_idx >= VM_REG_COUNT)) {
      return 0xffffffff; // Invalid register index
  }

  uint32_t effective_offset = offset;
  if (src_reg_idx != 0) { // If src_reg_idx is not 0, it's an address register
    effective_offset += state->regs[src_reg_idx];
  }
  
  if (effective_offset >= VM_MEM_SIZE) {
      return 0xffffffff; // Indicate an error or out-of-bounds access
  }

  state->regs[dest_reg_idx] = state->mem[effective_offset];
  return 0;
}

// Function: process_str
// Stores a byte from a register into memory
undefined4 process_str(vm_state *state, byte src_reg_idx_for_addr, byte src_reg_idx_for_val, ushort offset) {
  if ((src_reg_idx_for_addr != 0 && src_reg_idx_for_addr >= VM_REG_COUNT) || 
      (src_reg_idx_for_val != 0 && src_reg_idx_for_val >= VM_REG_COUNT)) {
      return 0xffffffff; // Invalid register index
  }

  uint32_t effective_offset = offset;
  if (src_reg_idx_for_addr != 0) { // If src_reg_idx_for_addr is not 0, it's an address register
    effective_offset += state->regs[src_reg_idx_for_addr];
  }

  if (effective_offset >= VM_MEM_SIZE) {
      return 0xffffffff; // Indicate an error or out-of-bounds access
  }

  uint8_t value_to_store = 0;
  if (src_reg_idx_for_val != 0) { // If src_reg_idx_for_val is not 0, get value from register
    value_to_store = (uint8_t)state->regs[src_reg_idx_for_val];
  }
  
  state->mem[effective_offset] = value_to_store;
  return 0;
}

// Function: frob (XORs memory region with 0x42)
// Original signature: undefined4 frob(undefined4 param_1,int param_2,uint param_3)
// param_1 is unused. param_2 is memory pointer (as int), param_3 is length.
undefined4 frob(undefined4 unused_param_1, int mem_ptr_int, uint len) {
  (void)unused_param_1; // Explicitly mark as unused
  unsigned char *mem_ptr = (unsigned char *)mem_ptr_int;
  for (uint i = 0; i < len; ++i) {
    if (mem_ptr + i >= (unsigned char*)0 + VM_MEM_SIZE) { // Basic bounds check for frob
        break; 
    }
    mem_ptr[i] ^= 0x42;
  }
  return 0;
}

// Function: process_sys
// Handles system calls (transmit, receive, frob)
undefined4 process_sys(vm_state *state) {
  // Regs: R1=syscall_id, R2=arg1, R3=arg2, R8=arg3 (for frob length, or transmit/receive length)
  uint32_t syscall_id = state->regs[1];
  uint32_t arg1_fd = state->regs[2];
  uint32_t arg2_offset = state->regs[3];
  uint32_t arg3_len = state->regs[8]; // Used for frob length, or transmit/receive length

  // System call type 2 is frob, which has a different signature.
  if (syscall_id == 2) { // frob
      // frob operates on the VM's memory region (state->mem)
      // The original code implies an offset of 0x40 (where mem starts)
      // and a length from state->regs[8].
      // The `frob` function itself takes the starting address and length.
      return frob(0, (int)state->mem, arg3_len);
  }

  // For transmit/receive, the first argument is fd, second is buffer, third is length.
  // The buffer is `state->mem + offset`.
  void *buf_ptr = (char*)state->mem + (arg2_offset & 0xffff);
  uint len = arg3_len & 0xffff;
  int fd = arg1_fd;

  // Bounds check for memory access
  if (buf_ptr < (void*)state->mem || (char*)buf_ptr + len > (char*)state->mem + VM_MEM_SIZE) {
      return 0xffffffff; // Out of bounds access
  }
  // Check for zero length as in original.
  if (len == 0) {
      return 0; // Success for 0-byte transfer
  }

  ssize_t result;
  if (syscall_id == 0) { // transmit
    result = transmit(fd, buf_ptr, len, 0);
  } else if (syscall_id == 1) { // receive
    result = receive(fd, buf_ptr, len, 0);
  } else {
    return 0xffffffff; // Unknown syscall_id
  }
  
  // Check if the operation was successful and transferred all bytes
  if (result < 0 || (uint)result != len) {
      return 0xffffffff;
  }
  return 0;
}

// Arithmetic helper: get operand value (immediate or register + immediate)
static inline int get_operand_val(vm_state *state, byte src_reg_idx, short immediate_val) {
    // Check src_reg_idx bounds only if it's non-zero (i.e., actually a register reference)
    if (src_reg_idx != 0 && src_reg_idx >= VM_REG_COUNT) {
        return 0; // Invalid register index, return 0 to avoid crash, may cause incorrect arithmetic.
    }
    if (src_reg_idx == 0) {
        return (int)immediate_val;
    } else {
        return (int)immediate_val + (int)state->regs[src_reg_idx];
    }
}

// Function: process_add
undefined4 process_add(vm_state *state, byte dest_reg_idx, byte src_reg_idx, short immediate_val) {
  if (dest_reg_idx >= VM_REG_COUNT) { return 0xffffffff; }
  state->regs[dest_reg_idx] += get_operand_val(state, src_reg_idx, immediate_val);
  return 0;
}

// Function: process_sub
undefined4 process_sub(vm_state *state, byte dest_reg_idx, byte src_reg_idx, short immediate_val) {
  if (dest_reg_idx >= VM_REG_COUNT) { return 0xffffffff; }
  state->regs[dest_reg_idx] -= get_operand_val(state, src_reg_idx, immediate_val);
  return 0;
}

// Function: process_mul
undefined4 process_mul(vm_state *state, byte dest_reg_idx, byte src_reg_idx, short immediate_val) {
  if (dest_reg_idx >= VM_REG_COUNT) { return 0xffffffff; }
  state->regs[dest_reg_idx] *= get_operand_val(state, src_reg_idx, immediate_val);
  return 0;
}

// Function: process_div
undefined4 process_div(vm_state *state, byte dest_reg_idx, byte src_reg_idx, short immediate_val) {
  if (dest_reg_idx >= VM_REG_COUNT) { return 0xffffffff; }
  int operand_val = get_operand_val(state, src_reg_idx, immediate_val);
  if (operand_val == 0) {
    return 0xffffffff; // Division by zero
  }
  state->regs[dest_reg_idx] /= (uint)operand_val;
  return 0;
}

// Bitwise helper: get operand value (immediate or register OR immediate)
static inline uint get_bitwise_operand_val(vm_state *state, byte src_reg_idx, short immediate_val) {
    // Check src_reg_idx bounds only if it's non-zero (i.e., actually a register reference)
    if (src_reg_idx != 0 && src_reg_idx >= VM_REG_COUNT) {
        return 0; // Invalid register index, return 0 to avoid crash.
    }
    if (src_reg_idx == 0) {
        return (uint)immediate_val;
    } else {
        // Original code: (int)param_4 | *(uint *)(param_1 + (uint)param_3 * 4)
        // This is a bitwise OR, not an addition for the second operand calculation.
        return (uint)immediate_val | state->regs[src_reg_idx];
    }
}

// Function: process_or
undefined4 process_or(vm_state *state, byte dest_reg_idx, byte src_reg_idx, short immediate_val) {
  if (dest_reg_idx >= VM_REG_COUNT) { return 0xffffffff; }
  state->regs[dest_reg_idx] |= get_bitwise_operand_val(state, src_reg_idx, immediate_val);
  return 0;
}

// Function: process_and
undefined4 process_and(vm_state *state, byte dest_reg_idx, byte src_reg_idx, short immediate_val) {
  if (dest_reg_idx >= VM_REG_COUNT) { return 0xffffffff; }
  state->regs[dest_reg_idx] &= get_bitwise_operand_val(state, src_reg_idx, immediate_val);
  return 0;
}

// Function: process_xor
undefined4 process_xor(vm_state *state, byte dest_reg_idx, byte src_reg_idx, short immediate_val) {
  if (dest_reg_idx >= VM_REG_COUNT) { return 0xffffffff; }
  state->regs[dest_reg_idx] ^= get_bitwise_operand_val(state, src_reg_idx, immediate_val);
  return 0;
}

// Comparison helper: get value for comparison (register from lower 4 bits of `reg_or_imm` or 0 if `reg_or_imm` is 0)
static inline uint get_cmp_val(vm_state *state, ushort reg_or_imm_val) {
    if (reg_or_imm_val == 0) {
        return 0;
    } else {
        // `param_4 & 0xf` implies the lower 4 bits of param_4 is the register index
        byte reg_idx = (byte)(reg_or_imm_val & 0xf);
        if (reg_idx >= VM_REG_COUNT) { // Basic bounds check
            return 0; // Invalid register index, return 0 to avoid crash.
        }
        return state->regs[reg_idx];
    }
}

// Function: process_slt (set less than)
undefined4 process_slt(vm_state *state, byte dest_reg_idx, byte src1_reg_idx, ushort src2_reg_or_imm) {
  if (dest_reg_idx >= VM_REG_COUNT || (src1_reg_idx != 0 && src1_reg_idx >= VM_REG_COUNT)) { return 0xffffffff; }
  uint val1 = (src1_reg_idx == 0) ? 0 : state->regs[src1_reg_idx];
  uint val2 = get_cmp_val(state, src2_reg_or_imm);
  state->regs[dest_reg_idx] = (val1 < val2);
  return 0;
}

// Function: process_slte (set less than or equal to)
undefined4 process_slte(vm_state *state, byte dest_reg_idx, byte src1_reg_idx, ushort src2_reg_or_imm) {
  if (dest_reg_idx >= VM_REG_COUNT || (src1_reg_idx != 0 && src1_reg_idx >= VM_REG_COUNT)) { return 0xffffffff; }
  uint val1 = (src1_reg_idx == 0) ? 0 : state->regs[src1_reg_idx];
  uint val2 = get_cmp_val(state, src2_reg_or_imm);
  state->regs[dest_reg_idx] = (val1 <= val2);
  return 0;
}

// Function: handle_inst
undefined4 handle_inst(vm_state *state, uint32_t *instruction_ptr) {
  // Instruction format: [ushort immediate/offset] [byte R_dest:4 R_src1:4] [byte opcode]
  // Read as a single 32-bit word (assuming little-endian for direct bitfield extraction)
  uint32_t instruction_word = *instruction_ptr;

  ushort immediate_or_offset = (ushort)(instruction_word & 0xFFFF);
  byte reg_fields = (byte)((instruction_word >> 16) & 0xFF);
  byte opcode = (byte)((instruction_word >> 24) & 0xFF);

  byte dest_reg = reg_fields >> 4;
  byte src_reg = reg_fields & 0xF;

  switch (opcode) {
    case 1: return process_load(state, dest_reg, src_reg, immediate_or_offset);
    case 2: return process_str(state, dest_reg, src_reg, immediate_or_offset);
    case 3: return process_sys(state);
    case 4: return process_add(state, dest_reg, src_reg, (short)immediate_or_offset);
    case 5: return process_sub(state, dest_reg, src_reg, (short)immediate_or_offset);
    case 6: return process_mul(state, dest_reg, src_reg, (short)immediate_or_offset);
    case 7: return process_div(state, dest_reg, src_reg, (short)immediate_or_offset);
    case 8: return process_or(state, dest_reg, src_reg, (short)immediate_or_offset);
    case 9: return process_and(state, dest_reg, src_reg, (short)immediate_or_offset);
    case 10: return process_xor(state, dest_reg, src_reg, (short)immediate_or_offset);
    case 0xb: return process_slt(state, dest_reg, src_reg, immediate_or_offset);
    case 0xc: return process_slte(state, dest_reg, src_reg, immediate_or_offset);
    default: return 0xffffffff;
  }
}

// Function: read_inst
undefined4 read_inst(int fd, vm_state *state, uint32_t *instruction_buf) {
  if (read_n_bytes(fd, 4, instruction_buf) != 4) {
    return 0xffffffff; // Error reading instruction
  }
  return handle_inst(state, instruction_buf);
}

// Function: main
undefined4 main(void) {
  // fd 0 is stdin, fd 1 is stdout
  int input_fd = 0; // Assuming input comes from stdin
  int output_fd = 1; // Assuming output goes to stdout

  // The original code transmits `&frob` function pointer as data.
  // This behavior is unusual and potentially platform-dependent (size of function pointer).
  // For Linux compilation, `frob` is `undefined4 frob(undefined4, int, uint)`.
  // `uintptr_t` is guaranteed to be large enough to hold any pointer.
  // We explicitly cast to `void*` for `transmit` which expects `const void*`.
  uintptr_t frob_addr = (uintptr_t)frob; 
  transmit(output_fd, &frob_addr, sizeof(frob_addr), 0); 

  if (read_header(input_fd) == 0) { // If header check failed (returns 0 for failure, 1 for success)
    return 0xffffffff;
  }

  int error_flag = 0;
  uint vm_flags = read_flags(input_fd, &error_flag);
  if (error_flag != 0) {
    return 0xffffffff; // Error reading flags
  }
  
  vm_state *state = init_state(input_fd, vm_flags);
  if (state == NULL) {
    return 0xffffffff; // Error initializing state
  }

  uint32_t instruction_word; // Buffer to read 4-byte instruction
  undefined4 inst_result;
  do {
    inst_result = read_inst(input_fd, state, &instruction_word);
    if ((int)inst_result < 0) break; // Error or end of instructions (read_inst returns 0xffffffff on error)
    
    // Dump registers after each instruction
    if (!dump_regs(output_fd, state)) {
        // If dump_regs fails, it's an I/O error, probably should terminate.
        free(state);
        return 0xffffffff;
    }
  } while (true); // Loop indefinitely until break condition inside

  // DAT_00014034 is likely a string literal in the original binary
  // Replaced with a simple "DONE\n" string.
  const char *done_msg = "DONE\n";
  transmit(output_fd, done_msg, strlen(done_msg), 0);

  free(state); // Clean up allocated state
  return 0;
}