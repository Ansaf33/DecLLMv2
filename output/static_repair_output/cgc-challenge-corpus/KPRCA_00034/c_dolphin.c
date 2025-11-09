#include <stdint.h> // For uint32_t
#include <stdlib.h> // For malloc, free
#include <stdio.h>  // For printf, fprintf (used in placeholder main)

// Define placeholder for KSconst (Key Schedule Constants).
// The size is 18 (initial keys) + 4 * 256 (S-boxes) = 1042 uint32_t values.
// In a real application, this array would be initialized with specific constant values.
static uint32_t KSconst[1042] = {0};

// Structure to hold the context for Dolphin cipher operations.
// This allows passing a pointer to the context instead of raw integers.
typedef struct {
    uint32_t *key_schedule; // Pointer to the dynamically allocated key schedule
} DolphinContext;

// Helper function for the repeated S-box lookup and XOR/ADD operation.
// This function encapsulates the complex lookup pattern, reducing code repetition
// and improving readability in the main encode/decode loops.
static inline uint32_t S_func(uint32_t val, const uint32_t *ks) {
    return ks[(val & 0xff) + 0x312] +
           (ks[(val >> 0x18) + 0x12] +
            ks[(val >> 0x10 & 0xff) + 0x112] ^
            ks[(val >> 8 & 0xff) + 0x212]);
}

// Function: dolphin_encode
// Encrypts a 64-bit block (two uint32_t values) using the Dolphin cipher.
// ctx_ptr: Pointer to the DolphinContext containing the key schedule.
// block: Pointer to an array of two uint32_t values (plaintext input, ciphertext output).
uint32_t dolphin_encode(DolphinContext *ctx_ptr, uint32_t *block) {
    const uint32_t *ks = ctx_ptr->key_schedule; // Key schedule pointer

    // Initial transformation
    uint32_t A = block[0] ^ ks[0];
    uint32_t B = (block[1] ^ ks[1]) ^ S_func(A, ks);

    // 16 rounds of encryption
    // The keys ks[2] through ks[17] are used.
    // A and B swap roles in terms of which is updated in alternating rounds.
    for (int i = 0; i < 16; ++i) {
        if (i % 2 == 0) { // Even rounds (0, 2, ..., 14) update A
            A = S_func(B, ks) ^ A ^ ks[i + 2];
        } else { // Odd rounds (1, 3, ..., 15) update B
            B = S_func(A, ks) ^ B ^ ks[i + 2];
        }
    }

    // Final permutation and output
    block[0] = ks[17] ^ B;
    block[1] = ks[16] ^ A ^ S_func(B, ks);
    return 0; // Success
}

// Function: dolphin_decode
// Decrypts a 64-bit block (two uint32_t values) using the Dolphin cipher.
// This function is the logical inverse of dolphin_encode.
// ctx_ptr: Pointer to the DolphinContext containing the key schedule.
// block: Pointer to an array of two uint32_t values (ciphertext input, plaintext output).
uint32_t dolphin_decode(DolphinContext *ctx_ptr, uint32_t *block) {
    const uint32_t *ks = ctx_ptr->key_schedule; // Key schedule pointer

    // Reverse the final permutation to get the state before the final output transformation
    uint32_t B_prev_final = ks[17] ^ block[0]; // Corresponds to B after all rounds in encode
    uint32_t A_prev_final = ks[16] ^ block[1] ^ S_func(B_prev_final, ks); // Corresponds to A after all rounds in encode

    // Set current state for decryption rounds
    uint32_t A = A_prev_final;
    uint32_t B = B_prev_final;

    // 16 rounds of decryption (reverse order and operations of encryption)
    // The keys ks[17] down to ks[2] are used.
    for (int i = 15; i >= 0; --i) {
        if (i % 2 == 0) { // Even rounds (14, 12, ..., 0) reversed, update A
            A = S_func(B, ks) ^ A ^ ks[i + 2];
        } else { // Odd rounds (15, 13, ..., 1) reversed, update B
            B = S_func(A, ks) ^ B ^ ks[i + 2];
        }
    }

    // Reverse the initial transformation
    // After rounds, A and B are the values before the rounds in encode:
    // A = block[0] ^ ks[0]
    // B = (block[1] ^ ks[1]) ^ S_func(A, ks)
    block[0] = A ^ ks[0];
    block[1] = B ^ S_func(A, ks) ^ ks[1];
    return 0; // Success
}

// Function: dolphin_init
// Initializes the Dolphin cipher context, including key schedule generation.
// ctx_ptr: Pointer to the DolphinContext to be initialized.
// user_key: Pointer to the user-supplied key (assumed to be 8 uint32_t words, i.e., 32 bytes).
uint32_t dolphin_init(DolphinContext *ctx_ptr, const uint32_t *user_key) {
    // Allocate memory for the key schedule (0x1048 bytes = 1042 uint32_t words)
    ctx_ptr->key_schedule = (uint32_t *)malloc(0x1048);
    if (ctx_ptr->key_schedule == NULL) {
        return 1; // Memory allocation failure
    }

    uint32_t *ks = ctx_ptr->key_schedule;
    int ks_const_idx = 0; // Index for reading from KSconst

    // Initialize the first 18 words of the key schedule from KSconst
    for (int i = 0; i < 18; ++i) { // 0x12 = 18
        ks[i] = KSconst[ks_const_idx++];
    }

    // Initialize four S-box tables (each 256 words) from KSconst.
    // The offsets (0x12, 0x112, 0x212, 0x312) correspond to the indices used by S_func.
    for (int i = 0; i < 256; ++i) { // 0x100 = 256
        ks[0x12 + i] = KSconst[ks_const_idx++]; // S-box 1 (offset 18)
    }
    for (int i = 0; i < 256; ++i) {
        ks[0x112 + i] = KSconst[ks_const_idx++]; // S-box 2 (offset 274)
    }
    for (int i = 0; i < 256; ++i) {
        ks[0x212 + i] = KSconst[ks_const_idx++]; // S-box 3 (offset 530)
    }
    for (int i = 0; i < 256; ++i) {
        ks[0x312 + i] = KSconst[ks_const_idx++]; // S-box 4 (offset 786)
    }

    // XOR the initial 18 key schedule words with the user key.
    // Assumes user_key is 8 words (32 bytes) and uses modulo 8 for key expansion.
    for (int i = 0; i < 18; ++i) {
        ks[i] ^= user_key[i % 8];
    }

    // Encrypt parts of the key schedule using dolphin_encode itself.
    // This process "mixes" the user key into the S-boxes.
    // Blocks of 2 words are encrypted.
    for (int i = 0; i < 18; i += 2) {
        dolphin_encode(ctx_ptr, &ks[i]);
    }
    for (int i = 0; i < 256; i += 2) {
        dolphin_encode(ctx_ptr, &ks[0x12 + i]);
    }
    for (int i = 0; i < 256; i += 2) {
        dolphin_encode(ctx_ptr, &ks[0x112 + i]);
    }
    for (int i = 0; i < 256; i += 2) {
        dolphin_encode(ctx_ptr, &ks[0x212 + i]);
    }
    for (int i = 0; i < 256; i += 2) {
        dolphin_encode(ctx_ptr, &ks[0x312 + i]);
    }

    return 0; // Success
}

// Function: dolphin_destroy
// Frees the memory allocated for the key schedule within the DolphinContext.
// ctx_ptr: Pointer to the DolphinContext to be destroyed.
void dolphin_destroy(DolphinContext *ctx_ptr) {
    free(ctx_ptr->key_schedule);
    ctx_ptr->key_schedule = NULL; // Good practice to nullify after freeing
}

// Placeholder main function for demonstration and compilation testing.
int main() {
    DolphinContext ctx;
    // Example user key (8 words / 32 bytes)
    uint32_t user_key[8] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210,
                            0xAABBCCDD, 0xEEFF0011, 0x22334455, 0x66778899};
    // Example plaintext block (2 words / 8 bytes)
    uint32_t plaintext[2] = {0x11223344, 0x55667788};
    uint32_t ciphertext[2];
    uint32_t decryptedtext[2];

    // Initialize KSconst with some dummy data for compilation.
    // In a real application, these would be fixed, pre-defined constants.
    for(int i = 0; i < 1042; ++i) {
        KSconst[i] = i * 0xDEADBEEF + 0xCAFEBABE;
    }

    // Initialize the Dolphin cipher context
    if (dolphin_init(&ctx, user_key) != 0) {
        fprintf(stderr, "Dolphin initialization failed!\n");
        return 1;
    }

    printf("Original plaintext: %08X %08X\n", plaintext[0], plaintext[1]);

    // Encrypt the plaintext
    ciphertext[0] = plaintext[0];
    ciphertext[1] = plaintext[1];
    dolphin_encode(&ctx, ciphertext);
    printf("Ciphertext:         %08X %08X\n", ciphertext[0], ciphertext[1]);

    // Decrypt the ciphertext
    decryptedtext[0] = ciphertext[0];
    decryptedtext[1] = ciphertext[1];
    dolphin_decode(&ctx, decryptedtext);
    printf("Decrypted text:     %08X %08X\n", decryptedtext[0], decryptedtext[1]);

    // Verify decryption
    if (decryptedtext[0] == plaintext[0] && decryptedtext[1] == plaintext[1]) {
        printf("Encryption and decryption successful!\n");
    } else {
        printf("Encryption and decryption failed (mismatch)!\n");
    }

    // Clean up the Dolphin context
    dolphin_destroy(&ctx);

    return 0;
}