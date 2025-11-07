#include <string.h> // For memset, memcpy
#include <stdint.h> // For uint16_t, uint32_t, int32_t, uint8_t, uintptr_t
#include <stddef.h> // For size_t

// --- Dummy declarations for external functions ---
// These are placeholders to make the code compilable.
// Their actual implementation and behavior would depend on the target system.

// Represents an allocation function.
// Returns 0 on success, non-zero on failure.
// Assumed to internally update a pointer that PMPGenerate then reads from `param_1 + 0x1d`.
int allocate() {
    // Placeholder: In a real system, this would allocate memory.
    // For compilation, we assume success.
    return 0;
}

// Represents a termination or error handling function.
void _terminate() {
    // Placeholder: In a real system, this might log an error and exit.
    // The original code uses this on allocation failure. We've replaced it with a return.
    // For compilation, it can be an empty function.
}

// Represents a function to get a color value.
// The original decompiled code implies arguments might be passed via global state
// or registers, as the signature `undefined4 GetColor()` has no parameters.
// For compilation, we'll assume it returns a uint32_t color value.
uint32_t GetColor() {
    // Placeholder: Return a dummy color.
    return 0x00FF00; // Green
}

// Represents a function to send data.
// param_1: likely a base address or handle (interpreted as an int for the call).
// param_2: size of data to send (interpreted as an int for the call).
// Returns 0 on success, non-zero on failure.
int SendAll(int param_1, int param_2) {
    // Placeholder: Simulate sending data.
    // For compilation, assume success.
    (void)param_1; // Suppress unused parameter warning
    (void)param_2; // Suppress unused parameter warning
    return 0;
}

// Represents a deallocation function.
// ptr: pointer to memory to deallocate.
// size: size of memory block.
void deallocate(void *ptr, uint32_t size) {
    // Placeholder: In a real system, this would free memory.
    (void)ptr;  // Suppress unused parameter warning
    (void)size; // Suppress unused parameter warning
}

// --- Type Aliases for Clarity ---
typedef uint32_t PMP_STATUS;
typedef uint16_t PMP_USHORT;
typedef uint32_t PMP_UINT;
typedef int32_t PMP_INT; // Used for `param_1` in PMPTransmit/PMPDeallocate and `local_18` in PMPGenerate

// Function: PMPGenerate
PMP_STATUS PMPGenerate(PMP_USHORT *param_1_ushort_ptr, const PMP_USHORT *param_2) {
    // Use uint8_t* for byte-level addressing as implied by the decompiled code's pointer arithmetic
    uint8_t *param_1 = (uint8_t *)param_1_ushort_ptr;

    PMP_INT row_padding = 0;
    PMP_INT total_padding = 0;
    PMP_UINT num_rows = (PMP_UINT)param_2[0];
    PMP_UINT num_cols = (PMP_UINT)param_2[1];

    // Calculate row padding for 4-byte alignment
    PMP_UINT bytes_per_row_unpadded = num_cols * 3;
    PMP_UINT alignment_remainder = bytes_per_row_unpadded & 3;
    if (alignment_remainder != 0) {
        row_padding = 4 - alignment_remainder;
    }
    PMP_UINT bytes_per_row_padded = bytes_per_row_unpadded + row_padding;

    // Set initial header fields (using explicit byte offsets and casts)
    // Offset 0: uint16_t
    *(uint16_t *)param_1 = 0x4d50;
    // Offset 6 (param_1 + 3 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 6) = 0;
    // Offset 10 (param_1 + 5 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 10) = 0x36; // Header size

    // Calculate total data size including row padding
    PMP_UINT total_data_bytes_unpadded = bytes_per_row_padded * num_rows;

    // Calculate total structure size (header + data) and apply overall alignment
    uint32_t total_structure_size = *(uint32_t *)(param_1 + 10) + total_data_bytes_unpadded;
    alignment_remainder = total_structure_size & 3;
    if (alignment_remainder != 0) {
        total_padding = 4 - alignment_remainder;
        total_structure_size += total_padding;
    }
    // Offset 2 (param_1 + 1 * sizeof(ushort)): uint32_t - Store calculated total structure size
    *(uint32_t *)(param_1 + 2) = total_structure_size;

    // Offset 54 (param_1 + 0x1b * sizeof(ushort)): uint32_t - Store size of the data block to be allocated
    uint32_t allocated_data_size = total_data_bytes_unpadded + total_padding;
    *(uint32_t *)(param_1 + 54) = allocated_data_size;

    // Call allocate(). If it fails, return error.
    if (allocate() != 0) {
        return 0xFFFFFFFF; // Indicate failure
    }

    // The allocated memory pointer is expected to be stored at param_1 + 0x1d (byte offset 58)
    void *allocated_memory_ptr = (void*)(uintptr_t)*(uint32_t *)(param_1 + 58);
    memset(allocated_memory_ptr, 0, allocated_data_size);

    // Set more header fields
    // Offset 14 (param_1 + 7 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 14) = 0x28;
    // Offset 22 (param_1 + 0xb * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 22) = num_rows;
    // Offset 18 (param_1 + 9 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 18) = num_cols;
    // Offset 26 (param_1 + 0xd * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 26) = 0x180001;
    // Offset 30 (param_1 + 0xf * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 30) = 0;
    // Offset 34 (param_1 + 0x11 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 34) = total_data_bytes_unpadded;
    // Offset 38 (param_1 + 0x13 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 38) = 0xb13;
    // Offset 42 (param_1 + 0x15 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 42) = 0xb13;
    // Offset 46 (param_1 + 0x17 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 46) = 0;
    // Offset 50 (param_1 + 0x19 * sizeof(ushort)): uint32_t
    *(uint32_t *)(param_1 + 50) = 0;

    // `current_write_ptr` will iterate through the allocated memory block
    uint8_t *current_write_ptr = (uint8_t *)allocated_memory_ptr;

    for (PMP_UINT r = 0; r < num_rows; r++) {
        for (PMP_UINT c = 0; c < num_cols; c++) {
            uint32_t color_value = GetColor();
            // Copy 3 bytes from color_value into the current position.
            // This assumes `color_value` is an integer that contains 3 bytes of data.
            memcpy(current_write_ptr, &color_value, 3);
            current_write_ptr += 3; // Advance pointer by 3 bytes
        }
        current_write_ptr += row_padding; // Add row padding
    }
    return 0; // Success
}

// Function: PMPTransmit
PMP_STATUS PMPTransmit(PMP_INT param_1) {
    // Cast the integer address to a byte pointer for byte-level arithmetic
    uint8_t *base_ptr = (uint8_t *)(uintptr_t)param_1;

    // Send first chunk (14 bytes from base)
    if (SendAll((PMP_INT)(uintptr_t)base_ptr, 0xe) != 0) {
        return 0xffffffff;
    }

    // Send second chunk (28 bytes from base + 0xe)
    if (SendAll((PMP_INT)(uintptr_t)(base_ptr + 0xe), 0x28) != 0) {
        return 0xffffffff;
    }

    // Send third chunk (data pointer from offset 0x3a, size from offset 0x36)
    uint32_t data_ptr_value = *(uint32_t *)(base_ptr + 0x3a);
    uint32_t data_size_value = *(uint32_t *)(base_ptr + 0x36);
    if (SendAll((PMP_INT)data_ptr_value, (PMP_INT)data_size_value) != 0) {
        return 0xffffffff;
    }

    return 0; // Success
}

// Function: PMPDeallocate
void PMPDeallocate(PMP_INT param_1) {
    // Cast the integer address to a byte pointer for byte-level arithmetic
    uint8_t *base_ptr = (uint8_t *)(uintptr_t)param_1;

    // Check if the pointer to the allocated data (at offset 0x3a) is non-null
    uint32_t allocated_ptr_value = *(uint32_t *)(base_ptr + 0x3a);
    if (allocated_ptr_value != 0) {
        uint32_t allocated_size_value = *(uint32_t *)(base_ptr + 0x36);
        deallocate((void*)(uintptr_t)allocated_ptr_value, allocated_size_value);
    }
    // Clear the size field at offset 0x36, regardless of deallocation success
    *(uint32_t *)(base_ptr + 0x36) = 0;
    return;
}