#include <stdint.h> // For uintptr_t
#include <stddef.h> // For standard definitions

// Function: bytecode_exec
int bytecode_exec(unsigned char *param_1, unsigned short param_2) {
  // header_val stores the initial 4-byte integer from the start of param_1.
  // It is used in the calculation of the target address `pbVar4`.
  int header_val = *(int *)param_1;

  // Loop through instruction blocks.
  // Each block is assumed to be 8 bytes long based on the original loop increment:
  // local_8 = local_8 + 2; where local_8 was uint3* (unsigned int*),
  // so local_8 advances by 2 * sizeof(unsigned int) = 8 bytes.
  // Instructions start at an offset of 3 bytes from param_1.
  for (unsigned char *instr_ptr = param_1 + 3;
       instr_ptr < param_1 + param_2;
       instr_ptr += 8) {

    // Extract opcode (4 bytes) and operands (1 byte each) from the current instruction block.
    // Assuming little-endian for opcode reading based on typical decompiler output for constants like 0x564944.
    unsigned int opcode = *(unsigned int *)instr_ptr;
    unsigned char operand1 = *(instr_ptr + 5);
    unsigned char operand2_offset = *(instr_ptr + 7);

    // Calculate the target memory address (pbVar4) where operations will be performed.
    // This calculation is a direct translation of the decompiled code,
    // which includes potentially non-portable pointer arithmetic.
    // (uintptr_t)param_1 is used to correctly handle pointer-to-integer conversion on 64-bit systems.
    unsigned char *pbVar4 = param_1 + (unsigned int)param_2 +
                            (((uintptr_t)param_1 & 0xff) | ((unsigned int)header_val << 8)) +
                            3 + (unsigned int)operand2_offset;

    // Perform operations based on the opcode and operands.
    if (opcode == 0x564944) { // 'VID' (Divide)
      if (operand1 == 0) {
        *pbVar4 = 0xaa; // Handle division by zero
      } else {
        *pbVar4 = *pbVar4 / operand1;
      }
    } else if (opcode < 0x564945) { // Check for opcodes less than 'VID'+1
      if (opcode == 0x544553) { // 'TES' (Set)
        *pbVar4 = operand1;
      } else if (opcode < 0x544554) { // Check for opcodes less than 'TES'+1
        if (opcode == 0x524853) { // 'RHS' (Right Shift)
          if (operand1 < 8) {
            // Perform a logical right shift on the unsigned char value
            *pbVar4 = (unsigned char)((unsigned int)*pbVar4 >> operand1);
          } else {
            *pbVar4 = 0; // Shifting by 8 or more bits results in 0 for a byte
          }
        } else if (opcode < 0x524854) { // Check for opcodes less than 'RHS'+1
          if (opcode == 0x4c554e) { // 'NUL' (Null/Zero)
            *pbVar4 = 0;
          } else if (opcode < 0x4c554f) { // Check for opcodes less than 'NUL'+1
            if (opcode == 0x4c554d) { // 'MUL' (Multiply)
              *pbVar4 = *pbVar4 * operand1;
            } else if (opcode < 0x4c554e) { // Check for opcodes less than 'MUL'+1
              if (opcode == 0x4c4853) { // 'LHS' (Left Shift)
                if (operand1 < 8) {
                  // Perform a left shift on the unsigned char value
                  *pbVar4 = (unsigned char)(*pbVar4 << operand1);
                } else {
                  *pbVar4 = 0; // Shifting by 8 or more bits results in 0 for a byte
                }
              } else if (opcode < 0x4c4854) { // Check for opcodes less than 'LHS'+1
                if (opcode == 0x425553) { // 'SUB' (Subtract)
                  *pbVar4 = *pbVar4 - operand1;
                } else if (opcode == 0x444441) { // 'ADD' (Add)
                  *pbVar4 = *pbVar4 + operand1;
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}