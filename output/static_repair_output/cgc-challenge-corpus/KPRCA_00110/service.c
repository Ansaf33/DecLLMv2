#include <stdio.h>   // For FILE, stdin, printf, fread, stderr, perror
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For memcpy (though direct assignment is better for structs)
#include <stdint.h>  // For uint32_t

// Type definitions for clarity
// 'uint' from original snippet is assumed to be uint32_t based on context (hex constants)
// 'undefined4' is also assumed to be uint32_t
typedef uint32_t uint;

// Structure to represent a calculation entry, 16 bytes total (0x10)
typedef struct {
    uint32_t op_code;
    int val_a;
    int val_b;
    int result; // Used to store computed result
} CalcEntry;

// Global variables based on usage in the snippet
uint32_t g_num_calcs = 0;
CalcEntry *g_calcs = NULL; // This will point to an array of CalcEntry structs
uint32_t g_sz_calcs = 0;   // Max number of entries g_calcs can hold
int g_rslr = 0;            // Appears to be an int based on usage in handle_exp and handle_cmp

// Forward declaration for calc_compute
// The signature is implied by `calc_compute(&local_20);` where local_20 is undefined4 (uint32_t)
// So, it likely takes a pointer to uint32_t.
void calc_compute(uint32_t *param_1);

// Static string literals (DAT_0001500x)
// These are placeholders; actual strings would depend on the original program's data section.
static const char *DAT_00015000 = "MUL"; // Example for 0x45fd1d19
static const char *DAT_00015002 = "ADD"; // Example for 0x9b6a4495
static const char *DAT_00015004 = "SUB"; // Example for 0xa2f78b10
static const char *DAT_00015006 = "DIV"; // Example for 0xe8bbacd2
static const char *DAT_00015008 = "XOR"; // Example for 0x2bae191d
static const char *DAT_0001500a = "UNK"; // Example for unknown operation

// Function: handle_calc
void handle_calc(uint param_1) {
    size_t items_read;
    uint32_t op_code = 0; // Initialize op_code to 0 (unknown)

    // Determine op_code based on param_1
    // The original nested `if (param_1 < CONST)` conditions are equivalent to a flat `if-else if` chain
    // for direct equality checks if `param_1` is only expected to match specific values.
    if (param_1 == 0xc66ac07f) {
        op_code = 0x9b6a4495;
    } else if (param_1 == 0xc5554a87) {
        op_code = 0xe8bbacd2;
    } else if (param_1 == 0x903c5ce4) {
        op_code = 0x45fd1d19;
    } else if (param_1 == 0x3fd26000) {
        op_code = 0xa2f78b10;
    } else if (param_1 == 0x5eac467d) {
        op_code = 0x2bae191d;
    }

    int val_a = 0;
    int val_b = 0;

    // Read val_a from stdin
    items_read = fread(&val_a, sizeof(int), 1, stdin);
    if (items_read == 1) {
        // Read val_b from stdin
        items_read = fread(&val_b, sizeof(int), 1, stdin);
        if (items_read == 1) {
            // Check for non-zero values and available capacity
            if ((val_a != 0) && (val_b != 0) && (g_calcs != NULL) && (g_num_calcs < g_sz_calcs)) {
                CalcEntry *current_entry = &g_calcs[g_num_calcs];
                current_entry->op_code = op_code;
                current_entry->val_a = val_a;
                current_entry->val_b = val_b;
                current_entry->result = 0; // Initialize result, will be computed later
                g_num_calcs++;
            }
            return; // Return after successful read or if conditions not met
        }
        exit(0); // Exit if second fread fails
    }
    exit(0); // Exit if first fread fails
}

// Function: handle_exp
void handle_exp(uint32_t *results_array) { // param_1 is a pointer to an array of uint32_t
    uint32_t dummy_val = 0x76fc2ed2; // Original value passed to calc_compute

    for (uint32_t i = 0; i < g_num_calcs; i++) {
        CalcEntry *current_entry = &g_calcs[i];

        // Call calc_compute. The value of dummy_val itself is not used after the call;
        // it's likely meant to trigger a side effect or update global state like g_rslr.
        calc_compute(&dummy_val);

        // Update the result for the current calculation entry
        current_entry->result = g_rslr + current_entry->result;

        // Store the updated result in the provided results_array
        results_array[i] = current_entry->result;
    }
    return;
}

// Function: handle_imp
void handle_imp(CalcEntry **calcs_ptr) { // param_1 is a pointer to the global g_calcs pointer
    size_t items_read;

    // Read the size of calculations array from stdin
    items_read = fread(&g_sz_calcs, sizeof(uint32_t), 1, stdin);
    if (items_read != 1) {
        exit(0); // Exit if fread fails
    }

    // Validate g_sz_calcs: must be between 3 and 30 (inclusive)
    if ((g_sz_calcs >= 3) && (g_sz_calcs <= 0x1e)) { // 0x1e = 30
        if (g_calcs != NULL) {
            free(g_calcs); // Free previous allocation if any
        }
        g_calcs = (CalcEntry *)malloc(g_sz_calcs * sizeof(CalcEntry));
        if (g_calcs == NULL) {
            perror("malloc failed in handle_imp");
            exit(1); // Exit on memory allocation failure
        }
        *calcs_ptr = g_calcs; // Update the pointer held by the caller (e.g., in main)
        g_num_calcs = 0;      // Reset number of calculations

        // Read each calculation entry from stdin
        for (uint32_t i = 0; i < g_sz_calcs; i++) {
            CalcEntry temp_entry; // Use a properly sized struct for reading 16 bytes
            items_read = fread(&temp_entry, sizeof(CalcEntry), 1, stdin);
            if (items_read != 1) {
                exit(0); // Exit if fread fails
            }

            // Check if val_a or val_b within the read entry are zero
            if ((temp_entry.val_a == 0) || (temp_entry.val_b == 0)) {
                exit(0); // Exit if an invalid entry is found
            }
            g_calcs[g_num_calcs] = temp_entry; // Copy the entry to the global array
            g_num_calcs++;
        }
    }
    // If g_sz_calcs is out of bounds, the function returns without allocating/processing.
    return;
}

// Function: print_cmp
void print_cmp(CalcEntry *entry) { // param_1 is a pointer to a CalcEntry
    const char *op_string = DAT_0001500a; // Default to "UNK"

    // Determine the operation string based on op_code, removing the goto structure
    if (entry->op_code == 0xe8bbacd2) {
        op_string = DAT_00015006; // DIV
    } else if (entry->op_code == 0xa2f78b10) {
        op_string = DAT_00015004; // SUB
    } else if (entry->op_code == 0x9b6a4495) {
        op_string = DAT_00015002; // ADD
    } else if (entry->op_code == 0x2bae191d) {
        op_string = DAT_00015008; // XOR
    } else if (entry->op_code == 0x45fd1d19) {
        op_string = DAT_00015000; // MUL
    }
    // If op_code doesn't match any of the above, op_string remains DAT_0001500a ("UNK").

    printf("A: %d, B: %d\n", entry->val_a, entry->val_b);
    printf("Result of A %s B: %d\n", op_string, entry->result);
    return;
}

// Function: handle_cmp
void handle_cmp(void) {
    uint32_t dummy_val = 0x76fc2ed2; // Original value passed to calc_compute

    printf("\n");
    for (uint32_t i = 0; i < g_num_calcs; i++) {
        CalcEntry *current_entry = &g_calcs[i];

        // Call calc_compute. The value of dummy_val itself is not used after the call;
        // it's likely meant to trigger a side effect or update global state like g_rslr.
        calc_compute(&dummy_val);

        // Update the result for the current calculation entry
        current_entry->result = g_rslr + current_entry->result;

        printf("Slot #%02d\n", i + 1);
        print_cmp(current_entry); // Pass the current entry for printing
    }
    return;
}

// Dummy implementation for calc_compute
// This function's actual logic is not provided in the snippet,
// so a placeholder is created for compilation.
void calc_compute(uint32_t *param_1) {
    // In a real scenario, this function would perform a calculation
    // based on the current context (e.g., g_calcs[i].op_code, val_a, val_b)
    // and update g_rslr or other global state.
    // As its definition is missing, we provide a dummy implementation.
    // The value of *param_1 (dummy_val) is never used after the call in the snippet,
    // so it's likely meant to trigger a side effect or its return value is implicitly global.
    // For compilation, we'll just increment g_rslr.
    g_rslr++; // Arbitrary update for the dummy function
    (void)param_1; // Suppress unused parameter warning
}

// Main function to make the code compilable and demonstrate usage
int main() {
    CalcEntry *calcs_from_imp = NULL; // Pointer to receive g_calcs from handle_imp

    printf("--- Initializing system ---\n");
    printf("Calling handle_imp to load calculations.\n");
    printf("This function requires input from stdin:\n");
    printf("1. A 4-byte unsigned integer (e.g., 3) for g_sz_calcs (number of calculations).\n");
    printf("   g_sz_calcs must be between 3 and 30 (inclusive).\n");
    printf("2. Then, for each calculation (g_sz_calcs times), 16 bytes (4 integers) representing:\n");
    printf("   OpCode, Value A, Value B, Result (Result is ignored by handle_imp).\n");
    printf("   Value A and Value B must not be zero.\n");
    printf("Example stdin (for 3 calculations: ADD 10 5, DIV 20 4, MUL 3 2):\n");
    printf("0x00000003\n");                                       // g_sz_calcs = 3
    printf("0x9b6a4495 0x0000000a 0x00000005 0x00000000\n"); // ADD (10, 5)
    printf("0xe8bbacd2 0x00000014 0x00000004 0x00000000\n"); // DIV (20, 4)
    printf("0x45fd1d19 0x00000003 0x00000002 0x00000000\n"); // MUL (3, 2)
    printf("Please provide input to stdin now, or the program may exit.\n");

    handle_imp(&calcs_from_imp);

    if (g_num_calcs > 0) {
        printf("\n--- Calling handle_calc to add one more calculation ---\n");
        printf("This function requires 2 integers from stdin for Value A and Value B.\n");
        printf("Example stdin: 100 10 (for Val_A=100, Val_B=10)\n");
        handle_calc(0xc66ac07f); // Add an entry with op_code 0x9b6a4495 (ADD)

        printf("\n--- Calling handle_cmp to compute and print results ---\n");
        handle_cmp();

        printf("\n--- Calling handle_exp to recompute and store results in a dummy array ---\n");
        // Create a dummy array to store results from handle_exp
        uint32_t *exp_results = (uint32_t *)malloc(g_num_calcs * sizeof(uint32_t));
        if (exp_results == NULL) {
            perror("malloc failed for exp_results");
            exit(1);
        }
        handle_exp(exp_results);

        printf("\n--- handle_exp results stored (first few values): ---\n");
        for (uint32_t i = 0; i < g_num_calcs && i < 3; i++) { // Print up to 3 results as example
            printf("Exp Result %u: %d\n", i, exp_results[i]);
        }
        free(exp_results);
    } else {
        printf("No calculations loaded/added. Skipping handle_cmp and handle_exp.\n");
    }

    // Clean up global allocation
    if (g_calcs != NULL) {
        free(g_calcs);
        g_calcs = NULL;
    }

    return 0;
}