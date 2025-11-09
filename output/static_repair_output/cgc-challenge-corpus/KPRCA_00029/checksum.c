#include <stdint.h> // For uint8_t, uint32_t
#include <stdio.h>  // For printf

// Type definitions for clarity and matching original snippet's intent
typedef uint8_t byte;
typedef uint32_t undefined4; // Assuming undefined4 means a 4-byte unsigned integer

// Dummy buffer_read_bit function for compilation and demonstration purposes.
// In a real application, this function would read a single bit from a data source.
// For this example, it cycles through bits of a dummy state variable.
static uint32_t dummy_buffer_state = 0xDEADBEEF; // Initial state for demonstration

byte buffer_read_bit(undefined4 param_1) {
    (void)param_1; // Suppress unused parameter warning for the dummy implementation
    byte bit = (dummy_buffer_state & 1);
    dummy_buffer_state >>= 1;
    if (dummy_buffer_state == 0) { // Reset or loop if all bits consumed
        dummy_buffer_state = 0xDEADBEEF;
    }
    return bit;
}

// Function: checksum8
byte checksum8(undefined4 param_1, uint32_t param_2, byte param_3) {
  byte current_checksum = param_3;

  for (uint32_t i = 0; i < param_2; ++i) {
    // Store the most significant bit (MSB) of the current checksum *before* shifting.
    byte msb_was_set = current_checksum & 0x80;
    
    // Read the next bit from the buffer.
    byte new_bit = buffer_read_bit(param_1);

    // Shift the current checksum left by one bit and incorporate the new bit.
    current_checksum = (current_checksum << 1) | new_bit;

    // If the MSB was set before the shift, XOR with the CRC polynomial (0x97).
    if (msb_was_set) {
      current_checksum ^= 0x97;
    }
  }
  return current_checksum;
}

// Main function to demonstrate the usage of checksum8
int main() {
    undefined4 buffer_handle = 0x12345678; // A dummy handle/address for the buffer
    uint32_t num_bits_to_process = 8;     // Number of bits to process for the checksum
    byte initial_checksum = 0x00;         // Starting value for the checksum

    printf("--- Checksum Calculation Example 1 ---\n");
    printf("Initial checksum: 0x%02X\n", initial_checksum);
    printf("Number of bits to process: %u\n", num_bits_to_process);

    // Calculate the checksum
    byte final_checksum = checksum8(buffer_handle, num_bits_to_process, initial_checksum);

    printf("Final checksum: 0x%02X\n", final_checksum);

    printf("\n--- Checksum Calculation Example 2 ---\n");
    // Reset dummy buffer state for a fresh example
    dummy_buffer_state = 0xA5A5A5A5; 
    num_bits_to_process = 16;
    initial_checksum = 0xFF; // A different initial value

    printf("Initial checksum: 0x%02X\n", initial_checksum);
    printf("Number of bits to process: %u\n", num_bits_to_process);
    
    final_checksum = checksum8(buffer_handle, num_bits_to_process, initial_checksum);
    printf("Final checksum: 0x%02X\n", final_checksum);

    return 0;
}