#include <stdio.h>   // Required for main function's printf
#include <string.h>  // Required for main function's memset, if used, but not strictly for this snippet

// Global S matrix, typically a quantization matrix or similar for DCT/IDCT.
// Provide a dummy definition for compilation. In a real application, this would be
// defined with actual values relevant to the DCT/IDCT standard being implemented.
static const int S[64] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

// Function: idct
// param_1: short* (input coefficients, e.g., quantized DCT coefficients)
// param_2: char* (output pixel data, e.g., -128 to 127)
// param_3: unsigned char* (input quantization table or similar, used for scaling)
void idct(short *param_1, char *param_2, unsigned char *param_3) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int temp_block[64]; // Replaced local_1b4[104] with a 64-element array for 8x8 block
  int row;            // Renamed local_14
  int col;            // Renamed local_10
  
  // Temporary variables that were previously indices [64] to [70] of local_1b4
  int t64, t65, t66, t67, t68, t69, t70;
  
  for (row = 0; row < 8; row++) {
    for (col = 0; col < 8; col++) {
      int idx = col + row * 8;
      temp_block[idx] =
           (unsigned int)param_3[idx] *
           (int)param_1[idx] *
           S[idx];
    }
  }
  temp_block[0] = temp_block[0] + 0x800; // Add 2048
  
  // Row-wise transformation
  for (row = 0; row < 8; row++) {
    int block_offset = row * 8;
    iVar1 = temp_block[block_offset + 2];
    iVar2 = temp_block[block_offset + 6];
    iVar7 = temp_block[block_offset + 7] + temp_block[block_offset + 1];
    iVar3 = temp_block[block_offset + 1] - temp_block[block_offset + 7];
    iVar5 = iVar3 + temp_block[block_offset + 5];
    iVar6 = iVar7 + temp_block[block_offset + 3];
    iVar7 = iVar7 - temp_block[block_offset + 3];
    iVar3 = iVar3 - temp_block[block_offset + 5];
    iVar4 = (iVar6 >> 9) - iVar6;
    iVar8 = (iVar5 >> 9) - iVar5;
    iVar5 = ((iVar4 >> 2) - iVar4) + (iVar5 >> 1);
    iVar6 = ((iVar8 >> 2) - iVar8) - (iVar6 >> 1);
    iVar4 = (iVar7 >> 3) - (iVar7 >> 7);
    t70 = iVar4 - (iVar7 >> 0xb);
    t69 = (iVar3 >> 3) - (iVar3 >> 7);
    t68 = t69 - (iVar3 >> 0xb);
    iVar7 = (iVar7 - iVar4) - (t69 + (t68 >> 1));
    iVar3 = (iVar3 - t69) + iVar4 + (t70 >> 1);
    t67 = iVar1 + (iVar1 >> 5);
    t66 = t67 >> 2;
    t65 = iVar2 + (iVar2 >> 5);
    t64 = t65 >> 2;
    iVar1 = (t66 + (iVar1 >> 4)) - (t65 - t64);
    iVar2 = t64 + (iVar2 >> 4) + (t67 - t66);
    iVar10 = temp_block[block_offset + 4] + temp_block[block_offset];
    iVar9 = temp_block[block_offset] - temp_block[block_offset + 4];
    iVar4 = iVar2 + iVar10;
    iVar8 = iVar1 + iVar9;
    iVar9 = iVar9 - iVar1;
    iVar10 = iVar10 - iVar2;
    temp_block[block_offset] = iVar5 + iVar4;
    temp_block[block_offset + 1] = iVar8 + iVar3;
    temp_block[block_offset + 2] = iVar7 + iVar9;
    temp_block[block_offset + 3] = iVar6 + iVar10;
    temp_block[block_offset + 4] = iVar10 - iVar6;
    temp_block[block_offset + 5] = iVar9 - iVar7;
    temp_block[block_offset + 6] = iVar8 - iVar3;
    temp_block[block_offset + 7] = iVar4 - iVar5;
  }

  // Column-wise transformation
  for (col = 0; col < 8; col++) {
    iVar1 = temp_block[col + 16];
    iVar2 = temp_block[col + 48];
    iVar3 = temp_block[col + 56] + temp_block[col + 8];
    iVar4 = temp_block[col + 8] - temp_block[col + 56];
    iVar5 = iVar4 + temp_block[col + 40];
    iVar6 = iVar3 + temp_block[col + 24];
    iVar3 = iVar3 - temp_block[col + 24];
    iVar4 = iVar4 - temp_block[col + 40];
    iVar7 = (iVar6 >> 9) - iVar6;
    iVar8 = (iVar5 >> 9) - iVar5;
    iVar5 = ((iVar7 >> 2) - iVar7) + (iVar5 >> 1);
    iVar6 = ((iVar8 >> 2) - iVar8) - (iVar6 >> 1);
    iVar8 = (iVar3 >> 3) - (iVar3 >> 7);
    iVar9 = (iVar4 >> 3) - (iVar4 >> 7);
    // Consistent use of t70 and t69 for inline calculations
    t70 = iVar8 - (iVar3 >> 0xb);
    t69 = iVar9 - (iVar4 >> 0xb);
    iVar7 = (iVar3 - iVar8) - (iVar9 + (t69 >> 1));
    iVar3 = (iVar4 - iVar9) + iVar8 + (t70 >> 1);
    t67 = iVar1 + (iVar1 >> 5);
    t66 = t67 >> 2;
    t65 = iVar2 + (iVar2 >> 5);
    t64 = t65 >> 2;
    iVar1 = (t66 + (iVar1 >> 4)) - (t65 - t64);
    iVar2 = t64 + (iVar2 >> 4) + (t67 - t66);
    iVar10 = temp_block[col + 32] + temp_block[col];
    iVar9 = temp_block[col] - temp_block[col + 32];
    iVar4 = iVar2 + iVar10;
    iVar8 = iVar1 + iVar9;
    iVar9 = iVar9 - iVar1;
    iVar10 = iVar10 - iVar2;
    temp_block[col] = iVar4 + iVar5;
    temp_block[col + 8] = iVar8 + iVar3;
    temp_block[col + 16] = iVar7 + iVar9;
    temp_block[col + 24] = iVar6 + iVar10;
    temp_block[col + 32] = iVar10 - iVar6;
    temp_block[col + 40] = iVar9 - iVar7;
    temp_block[col + 48] = iVar8 - iVar3;
    temp_block[col + 56] = iVar4 - iVar5;
  }

  // Final scaling and assignment to output
  for (row = 0; row < 8; row++) {
    for (col = 0; col < 8; col++) {
      int idx = col + row * 8;
      param_2[idx] = (char)(temp_block[idx] >> 13);
    }
  }
}

// Function: dct
// param_1: char* (input pixel data, e.g., -128 to 127)
// param_2: short* (output coefficients, e.g., quantized DCT coefficients)
// param_3: unsigned char* (input quantization table or similar, used for scaling)
void dct(char *param_1, short *param_2, unsigned char *param_3) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int temp_block[64]; // Replaced aiStack_1c0[108] with a 64-element array for 8x8 block
  int row;            // Renamed local_10
  int col;            // Renamed local_c
  
  // Temporary variables that were previously indices [64] to [70] of aiStack_1c0
  int t64, t65, t66, t67, t68, t69, t70;
  
  for (row = 0; row < 8; row++) {
    for (col = 0; col < 8; col++) {
      int idx = col + row * 8;
      temp_block[idx] = (int)param_1[idx] << 7;
    }
  }

  // Column-wise transformation
  for (col = 0; col < 8; col++) {
    iVar1 = temp_block[col + 24] - temp_block[col + 32];
    iVar5 = temp_block[col + 16] - temp_block[col + 40];
    iVar2 = temp_block[col + 8] - temp_block[col + 48];
    iVar4 = temp_block[col] - temp_block[col + 56];
    iVar9 = temp_block[col] + temp_block[col + 56];
    iVar8 = temp_block[col + 8] + temp_block[col + 48];
    iVar10 = temp_block[col + 16] + temp_block[col + 40];
    iVar11 = temp_block[col + 24] + temp_block[col + 32];
    iVar3 = (iVar1 >> 9) - iVar1;
    iVar6 = (iVar4 >> 9) - iVar4;
    iVar4 = ((iVar3 >> 2) - iVar3) + (iVar4 >> 1);
    iVar1 = ((iVar6 >> 2) - iVar6) - (iVar1 >> 1);
    iVar3 = (iVar5 >> 3) - (iVar5 >> 7);
    t70 = iVar3 - (iVar5 >> 0xb); // Explicitly calculate the part previously inline
    t69 = (iVar2 >> 3) - (iVar2 >> 7); // Explicitly calculate the part previously inline
    t68 = t69 - (iVar2 >> 0xb); // Explicitly calculate the part previously inline
    iVar5 = (iVar5 - iVar3) + t69 + (t68 >> 1);
    iVar2 = (iVar2 - t69) - (iVar3 + (t70 >> 1));
    iVar3 = iVar2 + iVar4;
    iVar6 = iVar1 + iVar5;
    temp_block[col + 24] = iVar1 - iVar5;
    temp_block[col + 40] = iVar4 - iVar2;
    temp_block[col + 56] = iVar6 - iVar3;
    temp_block[col + 8] = iVar6 + iVar3;
    iVar4 = iVar11 + iVar9;
    iVar1 = iVar10 + iVar8;
    iVar8 = iVar8 - iVar10;
    iVar9 = iVar9 - iVar11;
    temp_block[col] = iVar4 + iVar1;
    temp_block[col + 32] = iVar4 - iVar1;
    t67 = iVar8 + (iVar8 >> 5);
    t66 = t67 >> 2;
    t65 = iVar9 + (iVar9 >> 5);
    t64 = t65 >> 2;
    temp_block[col + 16] =
         (t65 - t64) + t66 + (iVar8 >> 4);
    temp_block[col + 48] =
         (t64 + (iVar9 >> 4)) - (t67 - t66);
  }

  // Row-wise transformation
  for (row = 0; row < 8; row++) {
    int block_offset = row * 8;
    iVar1 = temp_block[block_offset + 3] - temp_block[block_offset + 4];
    iVar2 = temp_block[block_offset + 2] - temp_block[block_offset + 5];
    iVar3 = temp_block[block_offset + 1] - temp_block[block_offset + 6];
    iVar4 = temp_block[block_offset] - temp_block[block_offset + 7];
    iVar9 = temp_block[block_offset] + temp_block[block_offset + 7];
    iVar8 = temp_block[block_offset + 1] + temp_block[block_offset + 6];
    iVar10 = temp_block[block_offset + 2] + temp_block[block_offset + 5];
    iVar11 = temp_block[block_offset + 3] + temp_block[block_offset + 4];
    iVar5 = (iVar1 >> 9) - iVar1;
    iVar6 = (iVar4 >> 9) - iVar4;
    iVar4 = ((iVar5 >> 2) - iVar5) + (iVar4 >> 1);
    iVar1 = ((iVar6 >> 2) - iVar6) - (iVar1 >> 1);
    iVar6 = (iVar2 >> 3) - (iVar2 >> 7);
    iVar7 = (iVar3 >> 3) - (iVar3 >> 7);
    // Consistent use of t70 and t69 for inline calculations
    t70 = iVar6 - (iVar2 >> 0xb);
    t69 = iVar7 - (iVar3 >> 0xb);
    iVar5 = (iVar2 - iVar6) + iVar7 + (t69 >> 1);
    iVar2 = (iVar3 - iVar7) - (iVar6 + (t70 >> 1));
    iVar3 = iVar2 + iVar4;
    iVar6 = iVar1 + iVar5;
    temp_block[block_offset + 3] = iVar1 - iVar5;
    temp_block[block_offset + 5] = iVar4 - iVar2;
    temp_block[block_offset + 7] = iVar6 - iVar3;
    temp_block[block_offset + 1] = iVar6 + iVar3;
    iVar4 = iVar11 + iVar9;
    iVar1 = iVar10 + iVar8;
    iVar8 = iVar8 - iVar10;
    iVar9 = iVar9 - iVar11;
    temp_block[block_offset] = iVar1 + iVar4;
    temp_block[block_offset + 4] = iVar4 - iVar1;
    iVar10 = iVar8 + (iVar8 >> 5);
    iVar11 = iVar10 >> 2;
    iVar4 = iVar9 + (iVar9 >> 5);
    iVar1 = iVar4 >> 2;
    temp_block[block_offset + 2] = (iVar4 - iVar1) + iVar11 + (iVar8 >> 4);
    temp_block[block_offset + 6] = (iVar1 + (iVar9 >> 4)) - (iVar10 - iVar11);
  }

  // Final scaling and assignment to output
  for (row = 0; row < 8; row++) {
    for (col = 0; col < 8; col++) {
      int idx = col + row * 8;
      int term_sign_extension = (temp_block[idx] >> 31); // Equivalent to (val < 0 ? -1 : 0) for 32-bit int
      int term_numerator = S[idx] * temp_block[idx];
      unsigned int term_divisor_val = param_3[idx];
      int term_division_result = 0;
      if (term_divisor_val != 0) { // Prevent division by zero
        term_division_result = term_numerator / (int)term_divisor_val;
      }
      int final_val = (term_sign_extension + term_division_result + 0x80000) >> 20;
      param_2[idx] = (short)final_val;
    }
  }
}

// A minimal main function to make the code compilable and runnable.
// It initializes dummy data and calls the idct and dct functions.
int main() {
    // Dummy data for IDCT
    short param1_idct_data[64];
    unsigned char param3_idct_data[64];
    char output_idct[64];

    // Dummy data for DCT
    char param1_dct_data[64];
    short output_dct[64];
    unsigned char param3_dct_data[64];

    // Initialize with some example values
    for (int i = 0; i < 64; ++i) {
        param1_idct_data[i] = i * 10; // Example short coefficients
        param3_idct_data[i] = (unsigned char)((i % 8) + 1); // Avoid 0 for division
        
        param1_dct_data[i] = (char)(i % 256 - 128); // Example char pixel values
        param3_dct_data[i] = (unsigned char)((i % 8) + 1); // Avoid 0 for division
    }

    // Call idct
    printf("Calling idct...\n");
    idct(param1_idct_data, output_idct, param3_idct_data);
    printf("IDCT output (first 8 values): ");
    for (int i = 0; i < 8; ++i) {
        printf("%d ", output_idct[i]);
    }
    printf("\n");

    // Call dct
    printf("Calling dct...\n");
    dct(param1_dct_data, output_dct, param3_dct_data);
    printf("DCT output (first 8 values): ");
    for (int i = 0; i < 8; ++i) {
        printf("%d ", output_dct[i]);
    }
    printf("\n");

    return 0;
}