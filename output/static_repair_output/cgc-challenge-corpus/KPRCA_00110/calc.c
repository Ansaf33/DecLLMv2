#include <stdint.h> // For intptr_t

// Function: calc_add
void calc_add(int *param_1) {
  if (*param_1 == 0x45fd1d19) {
    param_1[3] = param_1[1] + param_1[2];
  }
}

// Function: calc_sub
void calc_sub(int *param_1) {
  if (*param_1 == -0x6495bb6b) {
    param_1[3] = param_1[1] - param_1[2];
  }
}

// Function: calc_mul
void calc_mul(int *param_1) {
  if (*param_1 == -0x5d0874f0) {
    param_1[3] = param_1[1] * param_1[2];
  }
}

// Function: calc_div
void calc_div(int *param_1) {
  if ((*param_1 == -0x1744532e) && (param_1[2] != 0)) {
    param_1[3] = param_1[1] / param_1[2];
  }
}

// Function: calc_mod
void calc_mod(int *param_1) {
  if ((*param_1 == 0x2bae191d) && (param_1[2] != 0)) {
    param_1[3] = param_1[1] % param_1[2];
  }
}

// Function: calc_compute
void calc_compute(int *param_1) {
  if (*param_1 == 0x76fc2ed2) {
    // 'uint' is interpreted as 'unsigned int'.
    // param_1[1] holds an address, which is an int.
    // Safely cast int to pointer type using intptr_t for portability.
    // The pointer to the inner array is used directly in function calls.
    // The opcode for the inner array is stored in a single intermediate variable.
    unsigned int inner_opcode = *(unsigned int *)(intptr_t)param_1[1];

    if (inner_opcode == 0xe8bbacd2) {
      calc_div((int *)(intptr_t)param_1[1]);
    } else if (inner_opcode < 0xe8bbacd3) {
      if (inner_opcode == 0xa2f78b10) {
        calc_mul((int *)(intptr_t)param_1[1]);
      } else if (inner_opcode < 0xa2f78b11) {
        if (inner_opcode == 0x9b6a4495) {
          calc_sub((int *)(intptr_t)param_1[1]);
        } else if (inner_opcode < 0x9b6a4496) {
          if (inner_opcode == 0x2bae191d) {
            calc_mod((int *)(intptr_t)param_1[1]);
          } else if (inner_opcode == 0x45fd1d19) {
            calc_add((int *)(intptr_t)param_1[1]);
          }
        }
      }
    }
  }
}