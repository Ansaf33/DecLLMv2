#include <stdint.h> // For uint32_t, uint8_t, uint64_t
#include <stdlib.h> // For exit
#include <string.h> // For memset

// Custom carry detection for 32-bit addition
#define CARRY4(a, b) ((uint32_t)(a) > (UINT32_MAX - (uint32_t)(b)))

// Custom concatenation of two 32-bit values into a 64-bit value
#define CONCAT44(high, low) (((uint64_t)(high) << 32) | (uint32_t)(low))

// Forward declarations
void increment_read(uint32_t *param_1, uint32_t param_2);
int32_t increment_write(uint32_t *param_1, uint32_t param_2);
uint32_t buffer_read_bit(uint32_t *param_1);
uint32_t buffer_read_bits(uint32_t *param_1, uint32_t param_2);
uint64_t buffer_read_remaining(uint32_t *param_1);
void buffer_write_bit(uint32_t *param_1, int32_t param_2);
void buffer_write_bits(uint32_t *param_1, uint32_t param_2, int32_t param_3);


// Function: increment_read
// param_1: Pointer to buffer context (uint32_t array)
//   param_1[0]: read_current_offset_low (bits)
//   param_1[1]: read_current_offset_high (bits)
//   param_1[5]: buffer_size_bytes (bytes) - used to derive total capacity
void increment_read(uint32_t *param_1, uint32_t param_2) {
    uint32_t carry = CARRY4(param_1[0], param_2);
    param_1[0] += param_2;
    param_1[1] += carry;

    uint64_t current_pos = CONCAT44(param_1[1], param_1[0]);
    uint64_t capacity_bits = (uint64_t)param_1[5] * 8; // Total capacity in bits

    if (current_pos == capacity_bits) {
        param_1[0] = 0;
        param_1[1] = 0;
    } else if (current_pos > capacity_bits) {
        exit(1); // Read pointer exceeded buffer capacity
    }
}

// Function: increment_write
// param_1: Pointer to buffer context (uint32_t array)
//   param_1[2]: write_current_offset_low (bits)
//   param_1[3]: write_current_offset_high (bits)
//   param_1[5]: buffer_size_bytes (bytes) - used to derive total capacity
int32_t increment_write(uint32_t *param_1, uint32_t param_2) {
    uint32_t carry = CARRY4(param_1[2], param_2);
    param_1[2] += param_2;
    param_1[3] += carry;

    uint64_t current_write_pos = CONCAT44(param_1[3], param_1[2]);
    uint64_t write_capacity_bits = (uint64_t)param_1[5] * 8; // Total capacity in bits

    if (current_write_pos >= write_capacity_bits) {
        // Wrap around: subtract total capacity from current position
        uint32_t capacity_low_bits = write_capacity_bits & 0xFFFFFFFF;
        uint32_t capacity_high_bits = write_capacity_bits >> 32;

        uint32_t borrow_from_high = (param_1[2] < capacity_low_bits);
        param_1[2] -= capacity_low_bits;
        param_1[3] -= (capacity_high_bits + borrow_from_high);
    }
    return (int32_t)param_1[2]; // Return new low part of write offset
}

// Function: buffer_read_bit
// param_1: Pointer to buffer context (uint32_t array)
//   param_1[0]: read_current_offset_low (bits)
//   param_1[1]: read_current_offset_high (bits)
//   param_1[4]: data_buffer_ptr (uint8_t*)
//   param_1[5]: buffer_size_bytes (bytes) - used to derive total capacity
uint32_t buffer_read_bit(uint32_t *param_1) {
    uint64_t current_pos = CONCAT44(param_1[1], param_1[0]);
    uint64_t total_capacity_bits = (uint64_t)param_1[5] * 8; // Total capacity in bits

    if (current_pos == total_capacity_bits) {
        return UINT32_MAX; // Indicates end of buffer or error
    } else {
        uint64_t byte_offset = current_pos / 8;
        uint8_t *data_buffer = (uint8_t *)(uintptr_t)param_1[4];
        uint8_t byte_val = data_buffer[byte_offset];

        uint32_t original_bit_offset_low = param_1[0]; // Get original low part before increment
        increment_read(param_1, 1);
        uint32_t bit_index_within_byte = original_bit_offset_low & 7; // Bit index from LSB (0-7)
        return (uint32_t)((byte_val >> bit_index_within_byte) & 1);
    }
}

// Function: buffer_read_bits
// param_1: Pointer to buffer context (uint32_t array)
// param_2: Number of bits to read
uint32_t buffer_read_bits(uint32_t *param_1, uint32_t param_2) {
    uint32_t result = 0;
    uint32_t bits_read_count = 0;

    // Recalculate current_pos and total_capacity_bits inside loop, as param_1 is modified by buffer_read_bit
    uint64_t total_capacity_bits = (uint64_t)param_1[5] * 8;
    while ((bits_read_count < param_2) && (CONCAT44(param_1[1], param_1[0]) != total_capacity_bits)) {
        uint32_t bit = buffer_read_bit(param_1);
        result = (result * 2) | bit; // Build result LSB to MSB
        bits_read_count++;
    }
    return result;
}

// Function: buffer_read_bytes
// param_1: Pointer to buffer context (uint32_t array)
// param_2_data_ptr: Pointer to destination buffer (uint8_t*)
// param_3_num_bytes: Number of bytes to read
void buffer_read_bytes(uint32_t *param_1, uint8_t *param_2_data_ptr, uint32_t param_3_num_bytes) {
    uint64_t remaining_bits = buffer_read_remaining(param_1);
    uint64_t remaining_bytes = remaining_bits / 8;

    // Check if current read offset is byte-aligned and enough bytes are remaining
    if (((param_1[0] & 7) == 0) && (remaining_bytes >= param_3_num_bytes)) {
        for (uint32_t i = 0; i < param_3_num_bytes; i++) {
            uint64_t current_pos_bits = CONCAT44(param_1[1], param_1[0]);
            uint64_t current_byte_offset = current_pos_bits / 8;
            uint8_t *data_buffer = (uint8_t *)(uintptr_t)param_1[4];

            param_2_data_ptr[i] = data_buffer[current_byte_offset];
            increment_read(param_1, 8); // Advance by 1 byte (8 bits)
        }
    } else {
        // Not byte-aligned or insufficient remaining bytes for direct copy, read bit by bit
        for (uint32_t i = 0; i < param_3_num_bytes; i++) {
            param_2_data_ptr[i] = (uint8_t)buffer_read_bits(param_1, 8);
        }
    }
}

// Function: buffer_write_bit
// param_1: Pointer to buffer context (uint32_t array)
//   param_1[2]: write_current_offset_low (bits)
//   param_1[3]: write_current_offset_high (bits)
//   param_1[4]: data_buffer_ptr (uint8_t*)
// param_2_bit: Bit value to write (0 or 1)
void buffer_write_bit(uint32_t *param_1, int32_t param_2_bit) {
    uint64_t current_write_pos = CONCAT44(param_1[3], param_1[2]);
    uint64_t byte_offset = current_write_pos / 8;
    uint32_t bit_index_within_byte = param_1[2] & 7; // Bit index from LSB (0-7)

    uint8_t *data_buffer = (uint8_t *)(uintptr_t)param_1[4];
    uint8_t byte_val = data_buffer[byte_offset];

    uint8_t shift_amount = 7 - bit_index_within_byte; // Shift for MSB-first writing within byte
    if (bit_index_within_byte == 0) { // If it's the first bit in a new byte, clear the byte
        byte_val = 0;
    }
    data_buffer[byte_offset] = (uint8_t)(((param_2_bit & 1) << shift_amount) | byte_val);
    increment_write(param_1, 1); // Advance by 1 bit
}

// Function: buffer_write_bits
// param_1: Pointer to buffer context (uint32_t array)
// param_2_value: Value containing bits to write
// param_3_num_bits: Number of bits to write from param_2_value
void buffer_write_bits(uint32_t *param_1, uint32_t param_2_value, int32_t param_3_num_bits) {
    // Write bits from MSB to LSB
    for (int32_t i = param_3_num_bits - 1; i >= 0; i--) {
        buffer_write_bit(param_1, (param_2_value >> (i & 0x1f)) & 1);
    }
}

// Function: buffer_write_bytes
// param_1: Pointer to buffer context (uint32_t array)
// param_2_data_ptr: Pointer to source data (uint8_t*)
// param_3_num_bytes: Number of bytes to write
void buffer_write_bytes(uint32_t *param_1, uint8_t *param_2_data_ptr, uint32_t param_3_num_bytes) {
    // Check if current write offset is byte-aligned
    if ((param_1[2] & 7) == 0) {
        for (uint32_t i = 0; i < param_3_num_bytes; i++) {
            uint64_t current_write_pos = CONCAT44(param_1[3], param_1[2]);
            uint64_t byte_offset = current_write_pos / 8;
            uint8_t *data_buffer = (uint8_t *)(uintptr_t)param_1[4];
            data_buffer[byte_offset] = param_2_data_ptr[i];
            increment_write(param_1, 8); // Advance by 1 byte (8 bits)
        }
    } else {
        // Not byte-aligned, write bit by bit
        for (uint32_t i = 0; i < param_3_num_bytes; i++) {
            buffer_write_bits(param_1, param_2_data_ptr[i], 8);
        }
    }
}

// Function: buffer_read_remaining
// param_1: Pointer to buffer context (uint32_t array)
//   param_1[0]: read_current_offset_low (bits)
//   param_1[1]: read_current_offset_high (bits)
//   param_1[5]: buffer_size_bytes (bytes)
uint64_t buffer_read_remaining(uint32_t *param_1) {
    uint64_t total_capacity_bits = (uint64_t)param_1[5] * 8;
    uint64_t current_pos_bits = CONCAT44(param_1[1], param_1[0]);

    int64_t signed_remaining = (int64_t)total_capacity_bits - (int64_t)current_pos_bits;

    // If current position exceeds total capacity, it implies a wrap-around calculation
    if (signed_remaining < 0) {
        return (uint64_t)signed_remaining + total_capacity_bits;
    } else {
        return (uint64_t)signed_remaining;
    }
}

// Function: buffer_read_tell
// param_1_ptr_to_offset: Pointer to the 64-bit current read offset (param_1[0] and param_1[1])
uint64_t buffer_read_tell(uint64_t *param_1_ptr_to_offset) {
    return *param_1_ptr_to_offset;
}

// Function: buffer_read_seek
// param_1_ptr_to_offset_low: Pointer to the low part of the 64-bit current read offset (param_1[0])
void buffer_read_seek(uint32_t *param_1_ptr_to_offset_low, uint32_t param_2_low, uint32_t param_3_high) {
    param_1_ptr_to_offset_low[0] = param_2_low;
    param_1_ptr_to_offset_low[1] = param_3_high;
}

// Function: buffer_write_tell
// param_1: Pointer to buffer context (uint32_t array)
//   param_1[2]: write_current_offset_low (bits)
//   param_1[3]: write_current_offset_high (bits)
uint64_t buffer_write_tell(uint32_t *param_1) {
    return CONCAT44(param_1[3], param_1[2]);
}

// Function: buffer_write_seek
// param_1: Pointer to buffer context (uint32_t array)
void buffer_write_seek(uint32_t *param_1, uint32_t param_2_low, uint32_t param_3_high) {
    param_1[2] = param_2_low;
    param_1[3] = param_3_high;
}

// Function: buffer_init
// param_1_ptr: Pointer to the buffer context structure (void*)
// param_2_data_ptr_val: Value of the data buffer pointer (uint32_t, cast to uint8_t*)
// param_3_capacity_bytes: Total capacity of the buffer in bytes
void buffer_init(void *param_1_ptr, uint32_t param_2_data_ptr_val, uint32_t param_3_capacity_bytes) {
    // Clear 0x18 bytes (24 bytes), which is the size of 6 uint32_t fields.
    // Structure:
    // [0x00] ctx[0]: read_current_offset_low
    // [0x04] ctx[1]: read_current_offset_high
    // [0x08] ctx[2]: write_current_offset_low
    // [0x0C] ctx[3]: write_current_offset_high
    // [0x10] ctx[4]: data_buffer_ptr (uint8_t*)
    // [0x14] ctx[5]: buffer_size_bytes
    memset(param_1_ptr, 0, 0x18);

    uint32_t *ctx_as_uint32 = (uint32_t *)param_1_ptr;

    ctx_as_uint32[4] = param_2_data_ptr_val;     // data_buffer_ptr
    ctx_as_uint32[5] = param_3_capacity_bytes; // buffer_size_bytes
}