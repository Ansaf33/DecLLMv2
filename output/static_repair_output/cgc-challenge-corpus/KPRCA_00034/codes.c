#include <stdint.h> // For uint32_t, uint8_t, intptr_t
#include <string.h> // For memcpy, memcmp

// --- Forward declarations and type definitions ---

// Dummy global variables - actual definitions would be elsewhere in a real project.
// For standalone compilation, we provide minimal definitions.
uint32_t test_k = 0x12345678; // Example value
uint8_t test_b[16] = { // Example data
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

// Function pointer types inferred from usage.
// The context_ptr is passed as intptr_t* as it stores a pointer value.
typedef uint32_t (*init_func_ptr)(intptr_t* context_ptr, uint32_t arg);
typedef void (*encrypt_func_ptr)(intptr_t* context_ptr, uint8_t* data);
typedef void (*decrypt_func_ptr)(intptr_t* context_ptr, uint8_t* data);
typedef void (*cleanup_func_ptr)(intptr_t* context_ptr);

// Structure representing the "code object" that elements of 'codes' point to.
// The offsets (0x8, 0xc, 0x10, 0x14, 0x18, 0x1c) are relative to the start of this struct.
typedef struct {
    uint32_t dummy_0x0;
    uint32_t dummy_0x4;
    uint32_t field_0x8;          // Accessed at offset 0x8
    uint32_t field_0xc;          // Accessed at offset 0xc
    init_func_ptr init_func;     // Accessed at offset 0x10
    cleanup_func_ptr cleanup_func; // Accessed at offset 0x14
    encrypt_func_ptr encrypt_func; // Accessed at offset 0x18
    decrypt_func_ptr decrypt_func; // Accessed at offset 0x1c
    // ... potentially more fields
} code_object_t;

// 'codes' is an array of pointers to code_object_t.
// It's declared extern, assuming it's defined elsewhere, but for compilation,
// we provide a dummy definition.
extern code_object_t* codes[];

// Dummy function implementations for the code_object_t function pointers
uint32_t dummy_init(intptr_t* ctx, uint32_t arg) {
    (void)ctx; // Suppress unused parameter warning
    (void)arg; // Suppress unused parameter warning
    return 0; // Success
}
void dummy_encrypt(intptr_t* ctx, uint8_t* data) {
    (void)ctx;
    // Simple XOR for demonstration
    for (int i = 0; i < 16; ++i) {
        data[i] ^= 0x55;
    }
}
void dummy_decrypt(intptr_t* ctx, uint8_t* data) {
    (void)ctx;
    // Simple XOR for demonstration (undoes dummy_encrypt)
    for (int i = 0; i < 16; ++i) {
        data[i] ^= 0x55;
    }
}
void dummy_cleanup(intptr_t* ctx) {
    (void)ctx;
}

// Dummy code_object_t instances to populate the 'codes' array
code_object_t dummy_obj_0 = {0,0,0x80,0x100, dummy_init, dummy_cleanup, dummy_encrypt, dummy_decrypt};
code_object_t dummy_obj_1 = {0,0,0x70,0x90, dummy_init, dummy_cleanup, dummy_encrypt, dummy_decrypt};
code_object_t dummy_obj_2 = {0,0,0x60,0x80, dummy_init, dummy_cleanup, dummy_encrypt, dummy_decrypt};
code_object_t dummy_obj_3 = {0,0,0x50,0x70, dummy_init, dummy_cleanup, dummy_encrypt, dummy_decrypt};
code_object_t dummy_obj_4 = {0,0,0x40,0x60, dummy_init, dummy_cleanup, dummy_encrypt, dummy_decrypt};

// The 'codes' array itself
code_object_t* codes[] = {
    &dummy_obj_0,
    &dummy_obj_1,
    &dummy_obj_2,
    &dummy_obj_3,
    &dummy_obj_4
};

// --- Fixed functions ---

// Function: codes_ksize
uint32_t codes_ksize(uint32_t param_1) {
    if (param_1 < 5) {
        return codes[param_1]->field_0xc;
    }
    return 0xffffffff;
}

// Function: codes_init
uint32_t codes_init(intptr_t *p_context_ptr, uint32_t param_2, uint32_t param_3) {
    if (param_2 < 5) {
        // Store the address of the selected code_object_t into *p_context_ptr
        *p_context_ptr = (intptr_t)codes[param_2];

        // Call the init function from the code_object_t
        // Cast the stored pointer value back to code_object_t* to access its members.
        return ((code_object_t*)(*p_context_ptr))->init_func(p_context_ptr, param_3);
    }
    return 1; // Error code
}

// Function: codes_self_test
uint32_t codes_self_test(void) {
    uint8_t buffer[16]; // Corresponds to local_2c
    intptr_t context_handle; // Corresponds to local_1c[0], stores the pointer value

    for (int i = 0; i < 5; ++i) { // Loop from 0 to 4 (5 iterations)
        context_handle = (intptr_t)codes[i];
        code_object_t* current_obj = (code_object_t*)context_handle;

        // Condition check: (0x100 < obj->field_0xc) || (0x80 < obj->field_0x8) || init_func(...) != 0
        if ((0x100 < current_obj->field_0xc) ||
            (0x80 < current_obj->field_0x8) ||
            (current_obj->init_func(&context_handle, test_k) != 0)) {
            return 1; // Break and return 1 if any condition is true
        }

        memcpy(buffer, test_b, 16);
        current_obj->encrypt_func(&context_handle, buffer);
        current_obj->decrypt_func(&context_handle, buffer);
        current_obj->cleanup_func(&context_handle);

        if (memcmp(buffer, test_b, 16) != 0) {
            return 1;
        }
    }
    return 0; // Loop completed successfully
}