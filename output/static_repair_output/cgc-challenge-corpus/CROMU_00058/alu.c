#include <stdint.h> // For uint32_t, uint8_t, int32_t, int8_t
#include <stdbool.h> // For bool

// Custom types mapping Ghidra's output to standard C types
// Assuming 'uint' and 'undefined4' map to 32-bit unsigned integer
// Assuming 'byte' maps to 8-bit unsigned integer
typedef uint32_t uint;
typedef uint8_t byte;
typedef uint32_t undefined4; // Used for memory access where type is not strictly defined, typically 4 bytes.

// Function: getALUOp2
uint getALUOp2(int param_1, uint param_2) {
  if ((param_2 & 0x80) == 0) {
    // param_2._1_1_ >> 3 corresponds to ((param_2 & 0xFF) >> 3)
    // This extracts the lower 8 bits of param_2, then shifts right by 3,
    // effectively taking bits 3-7 as a 5-bit register index.
    return *(uint *)(param_1 + (uint)((param_2 & 0xFF) >> 3) * 4);
  } else {
    // param_2._1_1_ corresponds to (param_2 & 0xFF)
    // This extracts the lower 8 bits of param_2 as an immediate value.
    return (uint)(param_2 & 0xFF);
  }
}

// Function: doAdd
void doAdd(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t result = getALUOp2(param_1, param_2) + reg_rn_val;
  // param_2._3_1_ >> 3 corresponds to (((param_2 >> 16) & 0xFF) >> 3)
  // This extracts bits 19-23 as the destination register index (Rd).
  uint32_t rd_addr_offset = ((param_2 >> 19) & 0x1f);

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = result;
  // Set sign flag (bit 31 of result) in byte at param_1 + 0x4084
  *(uint8_t *)(param_1 + 0x4084) = (uint8_t)(result >> 31);
}

// Function: doAdc
void doAdc(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t carry_flag = (uint32_t)*(uint8_t *)(param_1 + 0x4084); // Assuming 0x4084 stores a carry/borrow flag (0 or 1)
  uint32_t result = reg_rn_val + getALUOp2(param_1, param_2) + carry_flag;
  uint32_t rd_addr_offset = ((param_2 >> 19) & 0x1f);

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = result;
  *(uint8_t *)(param_1 + 0x4084) = (uint8_t)(result >> 31);
}

// Function: doSub
void doSub(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t result = reg_rn_val - getALUOp2(param_1, param_2);
  uint32_t rd_addr_offset = ((param_2 >> 19) & 0x1f);

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = result;
  *(uint8_t *)(param_1 + 0x4084) = (uint8_t)(result >> 31);
}

// Function: doSuc
void doSuc(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t carry_flag = (uint32_t)*(uint8_t *)(param_1 + 0x4084);
  uint32_t result = (reg_rn_val - getALUOp2(param_1, param_2)) - carry_flag;
  uint32_t rd_addr_offset = ((param_2 >> 19) & 0x1f);

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = result;
  *(uint8_t *)(param_1 + 0x4084) = (uint8_t)(result >> 31);
}

// Function: doRsb
void doRsb(int param_1, uint param_2) {
  uint32_t result = getALUOp2(param_1, param_2) - *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t rd_addr_offset = ((param_2 >> 19) & 0x1f);

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = result;
  *(uint8_t *)(param_1 + 0x4084) = (uint8_t)(result >> 31);
}

// Function: doRsc
void doRsc(int param_1, uint param_2) {
  uint32_t carry_flag = (uint32_t)*(uint8_t *)(param_1 + 0x4084);
  uint32_t result = (getALUOp2(param_1, param_2) - *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4)) - carry_flag;
  uint32_t rd_addr_offset = ((param_2 >> 19) & 0x1f);

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = result;
  *(uint8_t *)(param_1 + 0x4084) = (uint8_t)(result >> 31);
}

// Function: doLsl
void doLsl(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  // getALUOp2 returns uint, but shift amount is typically 5 bits
  uint32_t shift_amount = getALUOp2(param_1, param_2) & 0x1f;
  // Destination register index (Rd) is in bits 27-31
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = reg_rn_val << shift_amount;
}

// Function: doLsr
void doLsr(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t shift_amount = getALUOp2(param_1, param_2) & 0x1f;
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = reg_rn_val >> shift_amount;
}

// Function: doAnd
void doAnd(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t op2_val = getALUOp2(param_1, param_2);
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = op2_val & reg_rn_val;
}

// Function: doOr
void doOr(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t op2_val = getALUOp2(param_1, param_2);
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = op2_val | reg_rn_val;
}

// Function: doXor
void doXor(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t op2_val = getALUOp2(param_1, param_2);
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = op2_val ^ reg_rn_val;
}

// Function: doNot
void doNot(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = ~reg_rn_val;
}

// Function: doMin
void doMin(int param_1, uint param_2) {
  // param_2._2_1_ >> 3 corresponds to (((param_2 >> 8) & 0xFF) >> 3)
  // This extracts bits 11-15 as the Rm register index.
  uint32_t rm_val = *(uint32_t *)(param_1 + (((param_2 >> 11) & 0x1f)) * 4);
  uint32_t op2_val = getALUOp2(param_1, param_2);
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  if (rm_val < op2_val) {
    *(uint32_t *)(param_1 + rd_addr_offset * 4) = rm_val;
  } else {
    *(uint32_t *)(param_1 + rd_addr_offset * 4) = op2_val;
  }
}

// Function: doMax
void doMax(int param_1, uint param_2) {
  // param_2._2_1_ >> 3 corresponds to (((param_2 >> 8) & 0xFF) >> 3)
  // This extracts bits 11-15 as the Rm register index.
  uint32_t rm_val = *(uint32_t *)(param_1 + (((param_2 >> 11) & 0x1f)) * 4);
  uint32_t op2_val = getALUOp2(param_1, param_2);
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  if (op2_val < rm_val) { // if Op2 < Rm, then Rm is Max
    *(uint32_t *)(param_1 + rd_addr_offset * 4) = rm_val;
  } else { // else Op2 is Max (or equal)
    *(uint32_t *)(param_1 + rd_addr_offset * 4) = op2_val;
  }
}

// Function: doClr
void doClr(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t bit_to_clear = getALUOp2(param_1, param_2) & 0x1f;
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = ~(1U << bit_to_clear) & reg_rn_val;
}

// Function: doSet
void doSet(int param_1, uint param_2) {
  uint32_t reg_rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);
  uint32_t bit_to_set = getALUOp2(param_1, param_2) & 0x1f;
  uint32_t rd_addr_offset = (param_2 >> 27) & 0x1f;

  *(uint32_t *)(param_1 + rd_addr_offset * 4) = (1U << bit_to_set) | reg_rn_val;
}

// Function: doQATB
void doQATB(int param_1, uint param_2) {
  uint32_t op2_val;
  uint32_t rn_val; // Corresponds to local_10, assumed to be Rn register
  bool condition_met = false; // Corresponds to bVar1

  // Determine op2_val (corresponds to local_8 in original)
  if ((param_2 & 0x80) == 0) {
    op2_val = *(uint32_t *)(param_1 + (uint32_t)((param_2 & 0xFF) >> 3) * 4);
  } else {
    op2_val = (uint32_t)(param_2 & 0xFF);
  }

  // Initialize rn_val (local_10 was uninitialized in original snippet)
  // Assuming it's the Rn register, which is common in ALU operations
  rn_val = *(uint32_t *)(param_1 + ((param_2 >> 19) & 0x1f) * 4);

  // Evaluate condition_met based on param_2 bits (condition codes)
  // Note: Original code used `bVar1 = bVar1 || ...` which is equivalent to multiple 'if' statements setting `condition_met = true`.
  if ((param_2 & 8) != 0 && rn_val == op2_val) { // EQ (Equal)
    condition_met = true;
  }
  // Original: `(int)local_8 < (int)local_10` which is `(int32_t)op2_val < (int32_t)rn_val`
  if ((param_2 & 4) != 0 && (int32_t)op2_val < (int32_t)rn_val) { // LT (Less Than, signed)
    condition_met = true;
  }
  // Original: `(int)local_10 < (int)local_8` which is `(int32_t)rn_val < (int32_t)op2_val`
  if ((param_2 & 0x10) != 0 && (int32_t)rn_val < (int32_t)op2_val) { // GT (Greater Than, signed)
    condition_met = true;
  }

  // If any condition is met, update the register at 0x80 (likely a Program Counter or similar)
  if (condition_met) {
    // param_2._3_1_ corresponds to ((param_2 >> 16) & 0xFF)
    // The cast `(int)param_2._3_1_` implies sign extension of an 8-bit value.
    int8_t branch_offset = (int8_t)((param_2 >> 16) & 0xFF);
    int32_t current_pc_val = *(int32_t *)(param_1 + 0x80); // Read as signed int for arithmetic
    current_pc_val += branch_offset;
    *(int32_t *)(param_1 + 0x80) = current_pc_val; // Write back as signed int

    // If the PC becomes negative, saturate it to 0.
    // The original `*(undefined4 *)(param_1 + 0x80) = 0;` implies a 4-byte assignment.
    if (*(int32_t *)(param_1 + 0x80) < 0) {
      *(uint32_t *)(param_1 + 0x80) = 0; // Saturate to 0, typically for an unsigned PC
    }
  }
}