#include <stdint.h> // For uint32_t, uint16_t, int16_t, uint8_t
#include <stdbool.h> // For true

// Stubs for missing functions to ensure compilability.
// The actual implementations of these functions are not provided.
void PaintRectangle(uint32_t ctx, uint8_t p1, uint8_t p2, uint8_t p3, char *data) {}
void PaintTriangle(uint32_t ctx, uint8_t p1, uint8_t p2, uint8_t p3, char *data) {}
void PaintLine(uint32_t ctx, uint8_t p1, uint8_t p2, uint8_t p3, char *data) {}
void PaintCircle(uint32_t ctx, uint8_t p1, uint8_t p2, uint8_t p3, char *data) {}
void PaintSquare(uint32_t ctx, uint8_t p1, uint8_t p2, uint8_t p3, char *data) {}
void PaintSpray(uint32_t ctx, uint8_t p1, uint8_t p2, uint8_t p3, char *data) {}

// Function: VGFVerify
uint32_t VGFVerify(const void *param_1) {
  // Cast param_1 to uint8_t* for byte-level pointer arithmetic and type-safe access.
  const uint8_t *buf = (const uint8_t *)param_1;

  // The original decompiler output implies specific byte offsets based on an assumed int* type for param_1.
  //   *param_1: Accesses the first 4 bytes as uint32_t.
  //   *(short *)(param_1 + 1): Accesses bytes 4-5 as int16_t (param_1 + 1 (int*) means buf + 4 bytes).
  //   *(ushort *)((int)param_1 + 6): Accesses bytes 6-7 as uint16_t.
  //   *(ushort *)(param_1 + 2): Accesses bytes 8-9 as uint16_t (param_1 + 2 (int*) means buf + 8 bytes).

  if (*(const uint32_t*)buf == 0x78330909) {
    if (*(const int16_t*)(buf + 4) == 1) {
      if (*(const uint16_t*)(buf + 6) < 0x201) {
        if (*(const uint16_t*)(buf + 8) < 0x201) {
          return 0; // All conditions met
        }
      }
    }
  }
  return 0xffffffff; // Any condition failed
}

// Function: VGFProcess
uint32_t VGFProcess(char **param_1, char *param_2, uint32_t param_3) {
  char *current_pos = *param_1; // Initialize current_pos from the caller's pointer

  while (true) {
    // Check if current_pos has reached or surpassed the end of the buffer
    if (param_2 <= current_pos) {
      return 0xffffffff;
    }

    // This variable will hold the address of the next instruction.
    // It defaults to current_pos + 4, but can be overridden by specific cases.
    char *next_pos = current_pos + 4;

    // Check for the 'd' (done) instruction, which terminates processing.
    if (*current_pos == 'd') {
      current_pos = next_pos; // Advance past the 'd' instruction for the final *param_1 update
      break;
    }

    // Extract common instruction bytes for clarity and efficiency
    uint8_t val1 = (uint8_t)current_pos[1];
    uint8_t val2 = (uint8_t)current_pos[2];
    uint8_t val3 = (uint8_t)current_pos[3];
    char *data_ptr = current_pos + 4; // Pointer to additional data for paint functions

    // Check validity of instruction parameters before dispatching
    if ((val1 < 6) && (val2 < 0x14)) {
      switch (*current_pos) {
        case '\0': // Rectangle instruction
          PaintRectangle(param_3, val1, val2, val3, data_ptr);
          next_pos = current_pos + 0xc; // Instruction length is 12 bytes
          break;
        case '\x01': // Triangle instruction
          PaintTriangle(param_3, val1, val2, val3, data_ptr);
          next_pos = current_pos + 0x10; // Instruction length is 16 bytes
          break;
        case '\x02': // Line instruction
          PaintLine(param_3, val1, val2, val3, data_ptr);
          next_pos = current_pos + 0xc; // Instruction length is 12 bytes
          break;
        case '\x03': // Circle instruction
          PaintCircle(param_3, val1, val2, val3, data_ptr);
          next_pos = current_pos + 10; // Instruction length is 10 bytes
          break;
        case '\x04': // Square instruction
          PaintSquare(param_3, val1, val2, val3, data_ptr);
          next_pos = current_pos + 10; // Instruction length is 10 bytes
          break;
        case '\x05': // Spray instruction
          PaintSpray(param_3, val1, val2, val3, data_ptr);
          next_pos = current_pos + 0x10; // Instruction length is 16 bytes
          break;
        // If *current_pos is not one of the cases above, and the (val1 < 6 && val2 < 0x14) check passed,
        // next_pos remains its default value of current_pos + 4. This matches the original logic.
      }
    }
    // If (val1 < 6 && val2 < 0x14) is false, next_pos remains its default value of current_pos + 4.
    // This also matches the original logic.

    current_pos = next_pos; // Move to the start of the next instruction
  }

  *param_1 = current_pos; // Update the caller's pointer to the final processed position
  return 0; // Processing completed successfully
}