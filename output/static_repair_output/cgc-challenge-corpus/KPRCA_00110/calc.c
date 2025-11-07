#include <stdint.h> // Required for intptr_t
#include <stdio.h>  // Required for printf in the main function

// Function: calc_add
void calc_add(int *param_1) {
  if (*param_1 == 0x45fd1d19) {
    param_1[3] = param_1[1] + param_1[2];
  }
}

// Function: calc_sub
void calc_sub(int *param_1) {
  // Note: -0x6495bb6b is equivalent to 0x9b6a4495 as an unsigned 32-bit integer.
  if (*param_1 == -0x6495bb6b) {
    param_1[3] = param_1[1] - param_1[2];
  }
}

// Function: calc_mul
void calc_mul(int *param_1) {
  // Note: -0x5d0874f0 is equivalent to 0xa2f78b10 as an unsigned 32-bit integer.
  if (*param_1 == -0x5d0874f0) {
    param_1[3] = param_1[1] * param_1[2];
  }
}

// Function: calc_div
void calc_div(int *param_1) {
  // Note: -0x1744532e is equivalent to 0xe8bbacd2 as an unsigned 32-bit integer.
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
    // The original code used `uint *puVar1;` and `uint uVar2;`.
    // These intermediate variables are eliminated by directly casting and dereferencing
    // `param_1[1]` at each point of use.
    //
    // The expression `(int *)(intptr_t)param_1[1]` converts the integer value
    // stored in `param_1[1]` into a pointer to an `int` array.
    // The `intptr_t` cast is used for safer integer-to-pointer conversion,
    // especially on 64-bit systems where `sizeof(int)` might be less than `sizeof(void*)`.
    //
    // The expression `*(unsigned int *)(intptr_t)param_1[1]` dereferences
    // this pointer as an `unsigned int` to get the operation code,
    // consistent with the unsigned hex literals in the original snippet.

    if (*(unsigned int *)(intptr_t)param_1[1] == 0xe8bbacd2) {
      calc_div((int *)(intptr_t)param_1[1]);
    } else if (*(unsigned int *)(intptr_t)param_1[1] < 0xe8bbacd3) {
      if (*(unsigned int *)(intptr_t)param_1[1] == 0xa2f78b10) {
        calc_mul((int *)(intptr_t)param_1[1]);
      } else if (*(unsigned int *)(intptr_t)param_1[1] < 0xa2f78b11) {
        if (*(unsigned int *)(intptr_t)param_1[1] == 0x9b6a4495) {
          calc_sub((int *)(intptr_t)param_1[1]);
        } else if (*(unsigned int *)(intptr_t)param_1[1] < 0x9b6a4496) {
          if (*(unsigned int *)(intptr_t)param_1[1] == 0x2bae191d) {
            calc_mod((int *)(intptr_t)param_1[1]);
          } else if (*(unsigned int *)(intptr_t)param_1[1] == 0x45fd1d19) {
            calc_add((int *)(intptr_t)param_1[1]);
          }
        }
      }
    }
  }
}

// Main function to make the code compilable and demonstrate usage.
// Note: On 64-bit systems, storing a 64-bit pointer address into a 32-bit `int`
// (as `main_params_add[1] = (int)(intptr_t)sub_params_add;` does)
// may result in truncation and warnings. This behavior is inherent to the
// original snippet's design where `param_1[1]` is an `int` holding a pointer.
// The code here faithfully translates that design for compilation.
int main() {
    // Example 1: Addition
    int sub_params_add[] = {0x45fd1d19, 10, 20, 0}; // Op: ADD, Op1: 10, Op2: 20, Result: 0
    int main_params_add[] = {0x76fc2ed2, (int)(intptr_t)sub_params_add, 0, 0}; // Main Op, Pointer to sub_params

    printf("--- Addition Test ---\n");
    printf("Before: sub_params_add[3] = %d\n", sub_params_add[3]);
    calc_compute(main_params_add);
    printf("After:  sub_params_add[3] = %d\n", sub_params_add[3]); // Expected: 30

    // Example 2: Subtraction
    int sub_params_sub[] = {-0x6495bb6b, 50, 15, 0}; // Op: SUB, Op1: 50, Op2: 15, Result: 0
    int main_params_sub[] = {0x76fc2ed2, (int)(intptr_t)sub_params_sub, 0, 0};

    printf("\n--- Subtraction Test ---\n");
    printf("Before: sub_params_sub[3] = %d\n", sub_params_sub[3]);
    calc_compute(main_params_sub);
    printf("After:  sub_params_sub[3] = %d\n", sub_params_sub[3]); // Expected: 35

    // Example 3: Multiplication
    int sub_params_mul[] = {-0x5d0874f0, 7, 8, 0}; // Op: MUL, Op1: 7, Op2: 8, Result: 0
    int main_params_mul[] = {0x76fc2ed2, (int)(intptr_t)sub_params_mul, 0, 0};

    printf("\n--- Multiplication Test ---\n");
    printf("Before: sub_params_mul[3] = %d\n", sub_params_mul[3]);
    calc_compute(main_params_mul);
    printf("After:  sub_params_mul[3] = %d\n", sub_params_mul[3]); // Expected: 56

    // Example 4: Division
    int sub_params_div[] = {-0x1744532e, 100, 10, 0}; // Op: DIV, Op1: 100, Op2: 10, Result: 0
    int main_params_div[] = {0x76fc2ed2, (int)(intptr_t)sub_params_div, 0, 0};

    printf("\n--- Division Test ---\n");
    printf("Before: sub_params_div[3] = %d\n", sub_params_div[3]);
    calc_compute(main_params_div);
    printf("After:  sub_params_div[3] = %d\n", sub_params_div[3]); // Expected: 10

    // Example 5: Modulo
    int sub_params_mod[] = {0x2bae191d, 25, 7, 0}; // Op: MOD, Op1: 25, Op2: 7, Result: 0
    int main_params_mod[] = {0x76fc2ed2, (int)(intptr_t)sub_params_mod, 0, 0};

    printf("\n--- Modulo Test ---\n");
    printf("Before: sub_params_mod[3] = %d\n", sub_params_mod[3]);
    calc_compute(main_params_mod);
    printf("After:  sub_params_mod[3] = %d\n", sub_params_mod[3]); // Expected: 4

    // Example 6: Division by zero (should not change result)
    int sub_params_div_zero[] = {-0x1744532e, 100, 0, 999}; // Op: DIV, Op1: 100, Op2: 0, Result: 999
    int main_params_div_zero[] = {0x76fc2ed2, (int)(intptr_t)sub_params_div_zero, 0, 0};

    printf("\n--- Division by Zero Test ---\n");
    printf("Before: sub_params_div_zero[3] = %d\n", sub_params_div_zero[3]);
    calc_compute(main_params_div_zero);
    printf("After:  sub_params_div_zero[3] = %d\n", sub_params_div_zero[3]); // Expected: 999 (unchanged)

    // Example 7: Modulo by zero (should not change result)
    int sub_params_mod_zero[] = {0x2bae191d, 25, 0, 888}; // Op: MOD, Op1: 25, Op2: 0, Result: 888
    int main_params_mod_zero[] = {0x76fc2ed2, (int)(intptr_t)sub_params_mod_zero, 0, 0};

    printf("\n--- Modulo by Zero Test ---\n");
    printf("Before: sub_params_mod_zero[3] = %d\n", sub_params_mod_zero[3]);
    calc_compute(main_params_mod_zero);
    printf("After:  sub_params_mod_zero[3] = %d\n", sub_params_mod_zero[3]); // Expected: 888 (unchanged)

    return 0;
}