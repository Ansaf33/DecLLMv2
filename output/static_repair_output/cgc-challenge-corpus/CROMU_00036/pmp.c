#include <stdint.h> // For uint32_t, uint16_t, uint8_t
#include <string.h> // For memset, memcpy
#include <stdlib.h> // For NULL, exit, malloc, free

// Type aliases for clarity and consistency with original snippet
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint32_t uint;   // Assuming uint maps to 32-bit unsigned int
typedef uint16_t ushort; // Assuming ushort maps to 16-bit unsigned short

// --- Mock implementations for external functions ---
// These functions are assumed to be provided by the environment
// and are mocked here to make the code compilable on Linux.

// Simple mock for allocate using malloc.
// In a real system, this would be a specific memory allocation function.
void* allocate(size_t size) {
    return malloc(size);
}

// Simple mock for _terminate.
// In a real system, this might perform specific cleanup before exiting.
void _terminate() {
    exit(1);
}

// Simple mock for GetColor.
// In a real system, this would return an actual color value.
uint32_t GetColor() {
    return 0xFF00FF; // Example color (magenta)
}

// Simple mock for SendAll.
// In a real system, this would send data over a communication channel.
int SendAll(int fd, size_t size) {
    (void)fd;   // Suppress unused parameter warning
    (void)size; // Suppress unused parameter warning
    return 0;   // Assume success
}

// Simple mock for deallocate using free.
// In a real system, this would be a specific memory deallocation function.
void deallocate(void* ptr, size_t size) {
    (void)size; // Size might be used for tracking in custom allocators
    free(ptr);
}

// --- Original functions with fixes and reductions ---

// Function: PMPGenerate
undefined4 PMPGenerate(undefined2 *param_1, ushort *param_2) {
  // Calculate padding for each row of data
  uint32_t row_data_payload_size = (uint32_t)param_2[1] * 3;
  uint32_t row_padding = 0;
  if (row_data_payload_size % 4 != 0) {
    row_padding = 4 - (row_data_payload_size % 4);
  }

  // Set initial header fields
  *param_1 = 0x4d50; // Magic number
  *(uint32_t *)(param_1 + 3) = 0; // Reserved field
  *(uint32_t *)(param_1 + 5) = 0x36; // Header size (fixed value)

  // Calculate total file size and adjust for alignment
  uint32_t total_size_unaligned = *(uint32_t *)(param_1 + 5) + (row_padding + row_data_payload_size) * (uint32_t)*param_2;
  *(uint32_t *)(param_1 + 1) = total_size_unaligned;

  uint32_t file_padding = 0;
  if (total_size_unaligned % 4 != 0) {
    file_padding = 4 - (total_size_unaligned % 4);
    *(uint32_t *)(param_1 + 1) += file_padding; // Adjust total size
  }

  // Calculate the offset to the actual data block
  uint32_t data_block_size = (uint32_t)*param_2 * (row_padding + row_data_payload_size) + file_padding;
  *(uint32_t *)(param_1 + 0x1b) = data_block_size;

  // Allocate memory for the data block
  void *data_ptr = allocate(data_block_size);
  if (data_ptr == NULL) {
    _terminate(); // Terminate if allocation fails
  }
  *(uint32_t *)(param_1 + 0x1d) = (uint32_t)data_ptr; // Store the data pointer in the header

  // Initialize the allocated data block to zero
  memset(data_ptr, 0, data_block_size);

  // Set remaining header fields
  *(uint32_t *)(param_1 + 7) = 0x28;
  *(uint32_t *)(param_1 + 9) = (uint32_t)param_2[1]; // Number of columns
  *(uint32_t *)(param_1 + 0xb) = (uint32_t)*param_2; // Number of rows
  *(uint32_t *)(param_1 + 0xd) = 0x180001;
  *(uint32_t *)(param_1 + 0xf) = 0; // Reserved
  *(uint32_t *)(param_1 + 0x11) = (row_padding + row_data_payload_size) * (uint32_t)*param_2; // Raw data size without final padding
  *(uint32_t *)(param_1 + 0x13) = 0xb13;
  *(uint32_t *)(param_1 + 0x15) = 0xb13;
  *(uint32_t *)(param_1 + 0x17) = 0; // Reserved
  *(uint32_t *)(param_1 + 0x19) = 0; // Reserved

  // Fill the data block with color information
  uint8_t *current_data_write_ptr = (uint8_t *)data_ptr;
  uint32_t num_rows = (uint32_t)*param_2;
  uint32_t num_cols = (uint32_t)param_2[1];

  for (uint32_t row_idx = 0; row_idx < num_rows; ++row_idx) {
    for (uint32_t col_idx = 0; col_idx < num_cols; ++col_idx) {
      uint32_t color = GetColor();
      memcpy(current_data_write_ptr, &color, 3); // Copy 3 bytes (e.g., RGB)
      current_data_write_ptr += 3;
    }
    current_data_write_ptr += row_padding; // Apply row padding
  }
  return 0;
}

// Function: PMPTransmit
undefined4 PMPTransmit(uint16_t *param_1) {
  // Send the first part of the header (0xe bytes)
  if (SendAll((int)param_1, 0xe) != 0) {
    return 0xffffffff;
  }
  // Send the second part of the header (0x28 bytes)
  if (SendAll((int)(param_1 + 0xe), 0x28) != 0) {
    return 0xffffffff;
  }
  // Send the actual data block
  // param_1 + 0x1d (offset 0x3a bytes) holds the data pointer
  // param_1 + 0x1b (offset 0x36 bytes) holds the data size
  if (SendAll(*(uint32_t *)(param_1 + 0x1d), *(uint32_t *)(param_1 + 0x1b)) != 0) {
    return 0xffffffff;
  }
  return 0; // Success
}

// Function: PMPDeallocate
void PMPDeallocate(uint16_t *param_1) {
  // Retrieve the data pointer and size from the header
  void *data_ptr = (void*)*(uint32_t *)(param_1 + 0x1d);
  uint32_t data_size = *(uint32_t *)(param_1 + 0x1b);

  // Deallocate memory if the pointer is valid
  if (data_ptr != NULL) {
    deallocate(data_ptr, data_size);
  }
  // Clear the data size and pointer fields in the header
  *(uint32_t *)(param_1 + 0x1b) = 0;
  *(uint32_t *)(param_1 + 0x1d) = 0;
  return;
}

// --- Example main function for compilation and testing ---
int main() {
    // Allocate a buffer to simulate the 'param_1' structure
    // A size of 50 * sizeof(uint16_t) = 100 bytes is typically enough for the header.
    uint16_t header_buffer[50];
    
    // Example input parameters for PMPGenerate:
    // param_2[0] = number of rows (e.g., 10)
    // param_2[1] = number of columns (e.g., 20)
    ushort input_params[] = {10, 20};

    // Call PMPGenerate to fill the header_buffer and allocate data
    undefined4 generate_status = PMPGenerate((undefined2*)header_buffer, input_params);
    if (generate_status != 0) {
        // Handle generation error
        return 1;
    }

    // Call PMPTransmit to simulate sending the data
    undefined4 transmit_status = PMPTransmit(header_buffer);
    if (transmit_status != 0) {
        // Handle transmission error
        // It's important to deallocate even on transmit error to prevent memory leaks
        PMPDeallocate(header_buffer);
        return 1;
    }

    // Call PMPDeallocate to free allocated memory
    PMPDeallocate(header_buffer);

    return 0;
}