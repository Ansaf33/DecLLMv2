#include <stdio.h>   // For printf in the example main function
#include <stdbool.h> // For bool, if needed (not strictly used here, but good practice)
#include <stdint.h>  // For uint8_t, uint32_t for precise integer types

// Define types as inferred from common C practice and the snippet's usage
typedef uint8_t byte;  // Represents an 8-bit unsigned integer
typedef uint32_t uint; // Represents a 32-bit unsigned integer

// Define the function pointer type for the output callback
typedef void (*output_func_ptr)(byte);

// Define the BitWriter context structure.
// This structure replaces the implicit and non-portable memory layout
// suggested by the original pointer arithmetic.
typedef struct BitWriter {
    output_func_ptr output_func; // The function to call when a byte is full
    byte current_byte;           // Buffer for the current byte being assembled
    byte bit_count;              // Number of bits currently in current_byte (0-7)
} BitWriter;

// Function: bitwriter_output
// Writes 'num_bits' from 'value' to the bit stream.
// The 'writer' parameter is assumed to be a pointer to a BitWriter context.
void bitwriter_output(BitWriter *writer, uint value, int num_bits) {
  // Validate num_bits: must be between 1 and 32 (inclusive)
  if (num_bits > 0 && num_bits <= 32) {
    // Iterate from the most significant bit (MSB) down to the least significant bit (LSB)
    for (int i = num_bits - 1; i >= 0; --i) {
      // Shift the current_byte left by 1 and append the next bit from 'value'
      writer->current_byte = (writer->current_byte << 1) | ((value >> i) & 1);
      writer->bit_count++;

      // If a full byte has been accumulated, call the output function
      if (writer->bit_count == 8) {
        writer->output_func(writer->current_byte);
        writer->bit_count = 0;
        writer->current_byte = 0; // Reset for the next byte
      }
    }
  }
}

// Function: bitwriter_flush
// Flushes any remaining bits in the current_byte buffer, padding with 'fill_bit'.
// The 'writer' parameter is assumed to be a pointer to a BitWriter context.
void bitwriter_flush(BitWriter *writer, byte fill_bit) {
  // Continue flushing until the bit_count is zero (all bits written)
  while (writer->bit_count != 0) {
    // Shift the current_byte left by 1 and append the fill_bit (only the LSB of fill_bit is used)
    writer->current_byte = (writer->current_byte << 1) | (fill_bit & 1);
    writer->bit_count++;

    // If a full byte has been accumulated, call the output function
    if (writer->bit_count == 8) {
      writer->output_func(writer->current_byte);
      writer->bit_count = 0;
      writer->current_byte = 0; // Reset for the next byte
    }
  }
}

// Example output function to demonstrate usage
void my_output_func(byte b) {
    printf("Output byte: 0x%02X (%d)\n", b, b);
}

// Main function for compilation and testing
int main() {
    // Initialize a BitWriter context
    BitWriter writer_context = {
        .output_func = my_output_func,
        .current_byte = 0,
        .bit_count = 0
    };

    printf("--- Testing bitwriter_output ---\n");
    // Example 1: Write 5 bits (0b10110) for value 22
    printf("Writing 5 bits (0b10110) for value 22...\n");
    bitwriter_output(&writer_context, 22, 5);
    printf("After 5 bits: current_byte=0x%02X, bit_count=%d\n", writer_context.current_byte, writer_context.bit_count); // Expected: 0x16 (0b00010110), count 5

    // Example 2: Write 3 bits (0b111) for value 7
    printf("\nWriting 3 bits (0b111) for value 7...\n");
    bitwriter_output(&writer_context, 7, 3);
    printf("After 3 bits: current_byte=0x%02X, bit_count=%d\n", writer_context.current_byte, writer_context.bit_count); // Expected: output 0b10110111 (0xB7), then reset to 0,0

    // Example 3: Write 10 bits (0b1010101010) for value 682
    printf("\nWriting 10 bits (0b1010101010) for value 682...\n");
    bitwriter_output(&writer_context, 682, 10);
    printf("After 10 bits: current_byte=0x%02X, bit_count=%d\n", writer_context.current_byte, writer_context.bit_count); // Expected: output 0b10101010 (0xAA), then current_byte=0b10, count 2

    printf("\n--- Testing bitwriter_flush ---\n");
    // Example 4: Flush remaining bits, padding with 0
    printf("Flushing remaining bits with 0s...\n");
    bitwriter_flush(&writer_context, 0);
    printf("After flush: current_byte=0x%02X, bit_count=%d\n", writer_context.current_byte, writer_context.bit_count); // Expected: output 0b10000000 (0x80), then reset to 0,0

    // Reset context for another flush test
    writer_context.current_byte = 0;
    writer_context.bit_count = 0;

    // Example 5: Write 3 bits (0b101) for value 5, then flush with 1s
    printf("\nWriting 3 bits (0b101) for value 5...\n");
    bitwriter_output(&writer_context, 5, 3);
    printf("After 3 bits: current_byte=0x%02X, bit_count=%d\n", writer_context.current_byte, writer_context.bit_count); // Expected: 0x05 (0b00000101), count 3
    printf("Flushing remaining bits with 1s...\n");
    bitwriter_flush(&writer_context, 1);
    printf("After flush: current_byte=0x%02X, bit_count=%d\n", writer_context.current_byte, writer_context.bit_count); // Expected: output 0b10111111 (0xBF), then reset to 0,0

    return 0;
}