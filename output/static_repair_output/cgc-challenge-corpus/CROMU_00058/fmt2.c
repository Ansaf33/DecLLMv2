#include <stdint.h> // For uint32_t, uint8_t, int32_t
#include <string.h> // For memset (optional, for main's example memory)

// Function: doBranch
void doBranch(uint32_t* mem, uint32_t param_2) {
  if (((uint8_t)param_2 & 0x78) == 8) {
    mem[param_2 >> 27] = mem[32] + 1;
  }
  if ((param_2 & 0x80) == 0) {
    mem[32] = mem[(param_2 >> 11) & 0x1f];
  } else {
    mem[32] = (param_2 >> 8) & 0xffff;
  }
}

// Function: doLdi
void doLdi(uint32_t* mem, uint32_t param_2) {
  mem[param_2 >> 27] = (param_2 >> 8) & 0xffff;
}

// Function: doLmbd
void doLmbd(uint32_t* mem, uint32_t param_2) {
  int32_t local_8 = 31; // 0x1f

  uint32_t val1_idx = (param_2 >> 11) & 0x1f;
  uint32_t val2_idx = (param_2 >> 27);

  // local_8 is guaranteed to be >= 0 within the loop, so (local_8 & 0x1f) is just local_8.
  while (local_8 >= 0 &&
         (((mem[val1_idx] ^ (mem[val2_idx] >> local_8)) & 1) != 0)) {
    local_8--;
  }

  if (local_8 < 0) {
    mem[val2_idx] = 32; // 0x20
  } else {
    mem[val2_idx] = local_8;
  }
}

// Function: doScan
void doScan(uint32_t* mem, uint32_t param_2) {
  uint32_t target_idx = param_2 >> 27;
  uint32_t val_idx_2 = param_2 >> 19;
  uint32_t val_idx_1_shifted = param_2 >> 11;
  uint32_t byte_1_val = (param_2 >> 8);

  if ((param_2 & 0x80) == 0) {
    mem[target_idx] = mem[1 + mem[val_idx_2] + mem[val_idx_1_shifted] + 32];
  } else if (byte_1_val + mem[val_idx_2] < 0x1000) {
    mem[target_idx] = mem[1 + mem[val_idx_2] + byte_1_val + 32];
  }
}

// Main function for compilation
int main() {
    // Example memory region, adjust size as needed for actual use.
    // This array serves as the 'param_1' argument (base memory address).
    uint32_t example_memory[128];
    memset(example_memory, 0, sizeof(example_memory)); // Initialize for a clean state

    // Example calls (replace with actual logic as needed)
    // doBranch(example_memory, 0x08000808);
    // doLdi(example_memory, 0x20567800);
    // doLmbd(example_memory, 0x30002800);
    // doScan(example_memory, 0x38484800);

    return 0;
}