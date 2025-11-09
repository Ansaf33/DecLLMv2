#include <stdio.h>   // For printf in main
#include <string.h>  // For strlen in main

// Global variables (replacing 'byte', 'undefined', 'uint' with standard C types)
unsigned char g_state;
unsigned char DAT_000160a1;
unsigned char DAT_000160a2[256]; // Array of 256 bytes

// Function: parcour_byte (RC4-like PRGA)
// Returns an unsigned char (byte)
unsigned char parcour_byte(void) {
  // Intermediate variables 'i' and 'j' are essential for distinct indices and updates.
  // 'temp_val_for_swap' is essential for swapping two values.
  // These variables are already minimal for the algorithm's logic.
  unsigned char i = g_state + 1; // Unsigned char arithmetic handles modulo 256 automatically
  unsigned char j = DAT_000160a2[i] + DAT_000160a1; // Unsigned char arithmetic handles modulo 256 automatically
  
  unsigned char temp_val_for_swap = DAT_000160a2[i]; // Temporary variable for the swap
  
  // Update global state variables
  g_state = i;
  DAT_000160a1 = j;
  
  // Swap DAT_000160a2[i] and DAT_000160a2[j]
  DAT_000160a2[i] = DAT_000160a2[j];
  DAT_000160a2[j] = temp_val_for_swap;
  
  // Return value, based on the newly swapped elements
  return DAT_000160a2[DAT_000160a2[i] + DAT_000160a2[j]]; // Unsigned char arithmetic handles modulo 256
}

// Function: parcour_init (RC4-like KSA)
// param_1 is interpreted as a pointer to a key string, param_2 as its length.
void parcour_init(const char *key, unsigned int key_len) {
  unsigned int i; // Loop counter, 'local_c' in original snippet
  unsigned char j = 0; // Accumulator for permutation, 'local_10' in original snippet
  
  // Initialize S-box (DAT_000160a2)
  for (i = 0; i < 256; i++) {
    DAT_000160a2[i] = (unsigned char)i;
  }
  
  // Permute S-box based on key
  for (i = 0; i < 256; i++) {
    // The original expression was:
    // local_10 = (int)(((local_10 + 0x100) - (uint)(byte)(&DAT_000160a2)[local_c]) + (int)*(char *)(param_1 + local_c % param_2)) % 0x100;
    // This logic corresponds to: j = (j - S[i] + K[i % key_len]) % 256.
    // Using unsigned char for 'j' and 'DAT_000160a2' elements, and allowing C's
    // integer promotion and wrapping rules for unsigned types, correctly implements
    // the modulo 256 arithmetic even with potential negative intermediate results
    // when a signed 'char' from the key is involved.
    j = (unsigned char)(j - DAT_000160a2[i] + key[i % key_len]);
    
    // Swap DAT_000160a2[i] and DAT_000160a2[j]
    unsigned char temp_val_for_swap = DAT_000160a2[i]; // 'uVar1' in original snippet
    DAT_000160a2[i] = DAT_000160a2[j];
    DAT_000160a2[j] = temp_val_for_swap;
  }
  
  // Reset state variables after KSA
  g_state = 0;
  DAT_000160a1 = 0;
  
  // Discard first 5000 bytes (common practice in RC4 for security)
  for (i = 0; i < 5000; i++) {
    parcour_byte();
  }
}

// Main function for compilation and demonstration
int main() {
    const char *my_key = "example_key";
    unsigned int my_key_len = strlen(my_key);

    printf("Initializing PRNG with key: \"%s\"\n", my_key);
    parcour_init(my_key, my_key_len);

    printf("Generating 10 bytes:\n");
    for (int k = 0; k < 10; k++) {
        unsigned char byte_val = parcour_byte();
        printf("Byte %d: %02x\n", k + 1, byte_val);
    }

    return 0;
}