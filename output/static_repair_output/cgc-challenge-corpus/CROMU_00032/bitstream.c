#include <stdlib.h> // For malloc, free
#include <string.h> // For memset, memcpy
#include <stdint.h> // For uintptr_t
#include <stddef.h> // For NULL

// Define uint as unsigned int, as it's used in the snippet but not a standard C type by default.
typedef unsigned int uint;

// Function: freeStream
void freeStream(int **param_1) {
    if (param_1 == NULL) {
        return;
    }
    // param_1 points to a void** structure (e.g., returned by initStream)
    // *param_1 is the actual void** structure
    void **stream_data_ptr = (void **)*param_1; // Retained as it's necessary before freeing stream_data_ptr
    if (stream_data_ptr != NULL) {
        if (*stream_data_ptr != NULL) { // stream_data_ptr[0] is the data buffer
            free(*stream_data_ptr);
        }
        free(stream_data_ptr); // Free the stream structure itself
        *param_1 = NULL;        // Clear the caller's pointer to the freed structure
    }
}

// Function: initStream
void ** initStream(void *param_1, void *param_2) {
    // param_1 is the initial data buffer, param_2 is its size (as a void*)
    if (param_1 == NULL || param_2 == NULL) { // param_2 (size) cannot be NULL (0)
        return NULL;
    }

    // Allocate space for the stream metadata (e.g., [data_ptr, size_val])
    // The original 0x10 (16 bytes) suggests sizeof(void*) * 2 on a 64-bit system.
    void **stream_ptr = (void **)malloc(sizeof(void*) * 2);
    if (stream_ptr == NULL) {
        return NULL;
    }

    memset(stream_ptr, 0, sizeof(void*) * 2);

    // param_2 is used as a size_t, so cast it.
    // This assumes param_2 actually holds a size value, not a pointer.
    size_t data_size = (size_t)param_2;

    // Allocate space for the actual data buffer
    stream_ptr[0] = malloc(data_size); // Assigned directly, removing intermediate pvVar1
    if (stream_ptr[0] == NULL) {
        free(stream_ptr); // Clean up stream_ptr if data allocation fails
        return NULL;
    }

    stream_ptr[1] = param_2; // Store the original void* value of param_2 (which is the size)
    memcpy(stream_ptr[0], param_1, data_size);

    return stream_ptr;
}

// Function: readBits
// param_1 is an int array representing the stream state:
// param_1[0]: (int)(uintptr_t)data_buffer_ptr - The actual data buffer address
// param_1[1]: (int)total_size_bytes          - Total size in bytes of the data buffer
// param_1[2]: current_byte_offset           - Current byte position in the buffer
// param_1[3]: current_bit_offset_in_byte    - Current bit position within the current byte (0-7)
int readBits(int *param_1, uint param_2, uint *param_3) {
    // Combine initial checks and return early for invalid inputs
    if (param_1 == NULL || param_3 == NULL || param_2 == 0) {
        return 0;
    }

    // If the data buffer pointer (stored as an int at param_1[0]) is NULL/0
    if (*param_1 == 0) {
        return 0;
    }

    // The maximum number of bits to read is 32 (0x20). param_2 must be less than 33 (0x21).
    if (param_2 >= 33) {
        return 0;
    }

    // Calculate total bits available and required.
    // param_1[1] is total size in bytes (as an int), param_1[2] is current byte offset, param_1[3] is current bit offset.
    unsigned int total_bits_in_buffer = (unsigned int)param_1[1] * 8;
    unsigned int current_position_bits = (unsigned int)param_1[2] * 8 + param_1[3];
    unsigned int required_bits = param_2;

    // Check if enough bits are available in the buffer
    if (total_bits_in_buffer < current_position_bits + required_bits) {
        return 0;
    }

    uint result_bits = 0; // Renamed local_10
    int bits_read_count = 0; // Renamed local_c

    // Safely cast the data buffer pointer from int to char* using uintptr_t for portability.
    // This assumes `*param_1` holds a valid memory address cast to an int.
    char *data_buffer = (char *)(uintptr_t)*param_1;

    for (unsigned int i = 0; i < required_bits; ++i) {
        // Read the current bit
        // param_1[2] is byte offset, param_1[3] is bit offset (0-7)
        unsigned char current_byte = (unsigned char)data_buffer[param_1[2]];
        unsigned int shift_amount = 7U - param_1[3]; // Calculate shift for MSB-first reading
        unsigned int bit = (current_byte >> shift_amount) & 1;

        result_bits = (result_bits << 1) | bit;

        // Update bit and byte offsets within the stream state
        param_1[3]++; // Increment bit offset
        if (param_1[3] == 8) { // If bit offset wraps to the next byte
            param_1[3] = 0;   // Reset bit offset
            param_1[2]++;   // Increment byte offset
        }
        bits_read_count++;
    }

    *param_3 = result_bits;
    return bits_read_count;
}