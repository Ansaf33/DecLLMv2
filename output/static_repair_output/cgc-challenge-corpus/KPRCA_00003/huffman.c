#include <stdint.h>
#include <stddef.h>

// Function: huffman_decode_table
void huffman_decode_table(uint8_t *output_base, const uint8_t *input_base) {
    int16_t huffman_codes[256];
    uint8_t huffman_lengths[256];
    uint16_t num_codes = 0;
    
    for (uint8_t huff_len = 0; huff_len < 16; ++huff_len) {
        for (uint8_t len_count = 1; len_count <= input_base[huff_len]; ++len_count) {
            huffman_lengths[num_codes++] = huff_len + 1;
        }
    }
    huffman_lengths[num_codes] = 0;

    uint8_t current_huff_len = huffman_lengths[0];
    int16_t current_huff_code = 0;
    uint16_t code_idx = 0;

    while (huffman_lengths[code_idx] != 0) {
        while (current_huff_len != huffman_lengths[code_idx]) {
            current_huff_code <<= 1;
            current_huff_len++;
        }
        while (current_huff_len == huffman_lengths[code_idx]) {
            huffman_codes[code_idx++] = current_huff_code++;
        }
    }

    for (code_idx = 0; code_idx < num_codes; ++code_idx) {
        uint8_t symbol_val = input_base[0x10 + code_idx];
        output_base[symbol_val] = huffman_lengths[code_idx];
        *(int16_t *)(output_base + (symbol_val + 0x80) * 2) = huffman_codes[code_idx];
    }
}

int main() {
    uint8_t output_buffer[512];
    uint8_t input_data[256];

    for (size_t i = 0; i < sizeof(output_buffer); ++i) {
        output_buffer[i] = 0;
    }
    for (size_t i = 0; i < sizeof(input_data); ++i) {
        input_data[i] = 0;
    }

    huffman_decode_table(output_buffer, input_data);

    return 0;
}