#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

// Type definitions
typedef uint32_t uint;
typedef uint32_t undefined4;
typedef uint8_t undefined;

// Structure for a calculation entry
typedef struct {
    uint32_t op;
    int val1;
    int val2;
    int result;
} CalcEntry;

// Global variables
CalcEntry *g_calcs = NULL;
uint g_num_calcs = 0;
uint g_sz_calcs = 0;
int g_rslr = 0; // Assuming this is an accumulator for results, initialized to 0.

// String literals for print_cmp (operator symbols)
static const char DAT_00015000[] = "*"; // Corresponds to 0x45fd1d19
static const char DAT_00015002[] = "/"; // Corresponds to 0x9b6a4495
static const char DAT_00015004[] = "+"; // Corresponds to 0xa2f78b10
static const char DAT_00015006[] = "-"; // Corresponds to 0xe8bbacd2
static const char DAT_00015008[] = "%"; // Corresponds to 0x2bae191d
static const char DAT_0001500a[] = "?"; // Default/unknown operator

// Dummy definition for calc_compute
// This function performs the calculation based on the operation code
// and stores the result in the CalcEntry.
void calc_compute(CalcEntry *entry) {
    if (!entry) return;

    switch (entry->op) {
        case 0x45fd1d19: // Multiply '*'
            entry->result = entry->val1 * entry->val2;
            break;
        case 0x9b6a4495: // Divide '/'
            if (entry->val2 != 0) {
                entry->result = entry->val1 / entry->val2;
            } else {
                entry->result = 0; // Division by zero
            }
            break;
        case 0xa2f78b10: // Add '+'
            entry->result = entry->val1 + entry->val2;
            break;
        case 0xe8bbacd2: // Subtract '-'
            entry->result = entry->val1 - entry->val2;
            break;
        case 0x2bae191d: // Modulo '%'
            if (entry->val2 != 0) {
                entry->result = entry->val1 % entry->val2;
            } else {
                entry->result = 0; // Modulo by zero
            }
            break;
        default:
            entry->result = 0; // Unknown operation
            break;
    }
}

// Function: handle_calc
void handle_calc(uint param_1) {
  int val1;
  int val2;
  undefined4 op_code;
  
  op_code = 0; // Initialize operator code

  if (param_1 == 0xc66ac07f) {
    op_code = 0x9b6a4495; // Division
  } else if (param_1 == 0xc5554a87) {
    op_code = 0xe8bbacd2; // Subtraction
  } else if (param_1 == 0x903c5ce4) {
    op_code = 0x45fd1d19; // Multiplication
  } else if (param_1 == 0x3fd26000) {
    op_code = 0xa2f78b10; // Addition
  } else if (param_1 == 0x5eac467d) {
    op_code = 0x2bae191d; // Modulo
  }

  // Read val1 from stdin
  if (fread(&val1, 1, sizeof(val1), stdin) != sizeof(val1)) {
    exit(0);
  }
  // Read val2 from stdin
  if (fread(&val2, 1, sizeof(val2), stdin) != sizeof(val2)) {
    exit(0);
  }

  // Check for valid pointers and array bounds before writing
  if (g_calcs == NULL || g_num_calcs >= g_sz_calcs) {
      exit(0);
  }

  if ((val1 != 0) && (val2 != 0)) {
    g_calcs[g_num_calcs].op = op_code;
    g_calcs[g_num_calcs].val1 = val1;
    g_calcs[g_num_calcs].val2 = val2;
    g_calcs[g_num_calcs].result = 0; // Result is computed later
    g_num_calcs++;
  }
  return;
}

// Function: handle_exp
void handle_exp(uint32_t *output_buffer) {
  if (g_calcs == NULL) {
      return;
  }

  for (uint i = 0; i < g_num_calcs; i++) {
    CalcEntry *current_entry = &g_calcs[i];
    
    calc_compute(current_entry);
    
    current_entry->result = g_rslr + current_entry->result;
    
    if (output_buffer != NULL) {
        output_buffer[i] = current_entry->result;
    }
  }
  return;
}

// Function: handle_imp
void handle_imp(void **param_1) {
  uint8_t temp_buffer[sizeof(CalcEntry)];
  
  // Read g_sz_calcs from stdin
  if (fread(&g_sz_calcs, 1, sizeof(g_sz_calcs), stdin) != sizeof(g_sz_calcs)) {
    exit(0);
  }

  // Validate g_sz_calcs range (3 to 30 inclusive)
  if ((g_sz_calcs < 3) || (g_sz_calcs >= 0x1f)) {
      exit(0);
  }

  if (g_calcs != NULL) {
    free(g_calcs);
    g_calcs = NULL; // Set to NULL after freeing
  }

  g_calcs = (CalcEntry*)malloc(g_sz_calcs * sizeof(CalcEntry));
  if (g_calcs == NULL) {
      exit(0); // Malloc failed
  }
  *param_1 = g_calcs; // Update the pointer passed in
  g_num_calcs = 0;

  for (uint i = 0; i < g_sz_calcs; i++) {
    // Read 16 bytes (size of CalcEntry) into temp_buffer
    if (fread(temp_buffer, 1, sizeof(temp_buffer), stdin) != sizeof(temp_buffer)) {
      exit(0);
    }

    // Extract val1 and val2 from the temporary buffer for validation
    int current_val1 = *(int*)(temp_buffer + offsetof(CalcEntry, val1));
    int current_val2 = *(int*)(temp_buffer + offsetof(CalcEntry, val2));

    if ((current_val1 == 0) || (current_val2 == 0)) {
      exit(0);
    }
    
    memcpy(&g_calcs[g_num_calcs], temp_buffer, sizeof(CalcEntry));
    g_num_calcs++;
  }
  return;
}

// Function: print_cmp
void print_cmp(CalcEntry *current_entry) {
  const char *op_symbol;
  
  if (current_entry == NULL) {
      return;
  }

  // Determine operator symbol without using goto
  if (current_entry->op == 0xe8bbacd2) {
    op_symbol = DAT_00015006; // "-"
  } else if (current_entry->op == 0xa2f78b10) {
    op_symbol = DAT_00015004; // "+"
  } else if (current_entry->op == 0x9b6a4495) {
    op_symbol = DAT_00015002; // "/"
  } else if (current_entry->op == 0x2bae191d) {
    op_symbol = DAT_00015008; // "%"
  } else if (current_entry->op == 0x45fd1d19) {
    op_symbol = DAT_00015000; // "*"
  } else {
    op_symbol = DAT_0001500a; // "?"
  }

  printf("A: %d, B: %d\n", current_entry->val1, current_entry->val2);
  printf("Result of A %s B: %d\n", op_symbol, current_entry->result);
  return;
}

// Function: handle_cmp
void handle_cmp(void) {
  printf("\n");

  if (g_calcs == NULL) {
      return;
  }

  for (uint i = 0; i < g_num_calcs; i++) {
    CalcEntry *current_entry = &g_calcs[i];
    
    calc_compute(current_entry);
    
    current_entry->result = g_rslr + current_entry->result;
    
    printf("Slot #%02d\n", i + 1);
    print_cmp(current_entry);
  }
  return;
}

// Main function for compilation and demonstration
int main() {
    printf("--- Program Start ---\n");

    // Example usage:
    // To run handle_imp, feed input to stdin:
    // First 4 bytes: g_sz_calcs (e.g., 0x03 for 3 entries)
    // Then 3 * 16 bytes for CalcEntry data (op, val1, val2, result)
    // Example: echo -en "\x03\x00\x00\x00" | xxd -r -p | cat - <(echo -en "\x19\x1d\xfd\x45\x0a\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xd2\xac\xbb\xe8\x0f\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x10\x8b\xf7\xa2\x07\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00") | ./your_program

    printf("Calling handle_imp to read initial calculations from stdin...\n");
    handle_imp((void**)&g_calcs);

    printf("Current number of calculations after import: %u (Max: %u)\n", g_num_calcs, g_sz_calcs);

    // To run handle_calc, feed 8 bytes (two integers) to stdin.
    // This will add one more calculation if space is available.
    if (g_num_calcs < g_sz_calcs) {
        printf("\nCalling handle_calc to add one more calculation (e.g., division 20 / 4) from stdin...\n");
        // Example for stdin: echo -en "\x14\x00\x00\x00\x04\x00\x00\x00" | xxd -r -p
        handle_calc(0xc66ac07f); // Operation: Division
        printf("Number of calculations after adding: %u\n", g_num_calcs);
    } else {
        printf("\nNo space to add more calculations with handle_calc.\n");
    }

    if (g_num_calcs > 0) {
        uint32_t exp_results[g_num_calcs]; // Array to store results from handle_exp

        printf("\nCalling handle_exp to compute and export results...\n");
        handle_exp(exp_results);

        printf("\nCalling handle_cmp to compute and print all calculations...\n");
        handle_cmp();

        printf("\nResults exported by handle_exp:\n");
        for (uint i = 0; i < g_num_calcs; ++i) {
            printf("Exported result for slot #%02d: %d\n", i + 1, exp_results[i]);
        }
    } else {
        printf("\nNo calculations available to process.\n");
    }

    if (g_calcs != NULL) {
        free(g_calcs);
        g_calcs = NULL;
        printf("\nGlobal calculations array freed.\n");
    }

    printf("--- Program End ---\n");
    return 0;
}