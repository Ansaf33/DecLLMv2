#include <stdint.h> // For uint32_t
#include <string.h> // For memcpy

// Function: geegee
int geegee(const unsigned char* param_1, unsigned int param_2) {
    int accumulator = 0;
    for (unsigned int i = 0; i < param_2; ++i) {
        accumulator = accumulator * 0x807f + param_1[i];
    }
    return accumulator;
}

// Function: yooyoo
int yooyoo(const unsigned char* param_1, unsigned int param_2) {
    int accumulator = 0x72b;
    for (unsigned int i = 0; i < param_2; ++i) {
        accumulator = param_1[i] + accumulator * 0x25;
    }
    return accumulator;
}

// Function: moomoo
uint32_t moomoo(const unsigned char* param_1, uint32_t param_2) {
    uint32_t hash = 0x41414141U;
    uint32_t num_blocks = param_2 >> 2;

    for (uint32_t i = 0; i < num_blocks; ++i) {
        // Original code used *(int*), which might be an unaligned access.
        // On many architectures (e.g., x86/x64), this works but can be slower for unaligned access.
        // For strict portability, memcpy(&block, param_1 + i * 4, sizeof(uint32_t)) would be used.
        // This translation keeps the direct cast as it's common in such patterns.
        uint32_t block = *(const uint32_t*)(param_1 + i * 4);

        uint32_t combined_val_part1 = block * 0x5d398000U;
        uint32_t combined_val_part2 = (block * 0x15d2e9ccU) >> 0x13;
        hash ^= (combined_val_part1 | combined_val_part2) * 0x395378b1U;
        hash = (hash << 0xf | hash >> 0x11) * 7U + 0x46b6456eU;
    }

    // Tail processing for remaining bytes
    const unsigned char* pb_tail = param_1 + (num_blocks * 4);
    uint32_t remaining_bytes_len = param_2 & 3; // param_2 % 4

    uint32_t padded_tail_block = 0;
    if (remaining_bytes_len > 0) {
        // Copy remaining bytes into a 4-byte buffer to avoid out-of-bounds reads.
        // This ensures safe access to p0, p1, p2 even if remaining_bytes_len is small.
        // For example, if remaining_bytes_len is 1, only pb_tail[0] is copied; p1 and p2 will be 0.
        memcpy(&padded_tail_block, pb_tail, remaining_bytes_len);
    }

    // Extract bytes from the padded block
    uint32_t p0 = (padded_tail_block >> 0) & 0xFF;
    uint32_t p1 = (padded_tail_block >> 8) & 0xFF;
    uint32_t p2 = (padded_tail_block >> 16) & 0xFF;

    uint32_t lc_temp = 0;
    // The original code's conditions are based on the *total length* (param_2), not remaining_bytes_len.
    // This translation preserves that specific logic using safely extracted bytes.
    if (param_2 < 4) {
        lc_temp = p2 << 16;
    }
    if (param_2 < 3) {
        lc_temp ^= p1 << 8;
    }
    if (param_2 < 2) {
        uint32_t combined_val = p0 ^ lc_temp;
        uint32_t term1 = combined_val * 0x5d398000U;
        uint32_t term2 = (combined_val * 0x15d2e9ccU) >> 0x13;
        hash ^= (term1 | term2) * 0x395378b1U;
    }

    // Finalization steps
    hash ^= param_2;
    hash ^= hash >> 0xd;
    hash *= 0xB6ACBE58U; // -0x495341a8 as unsigned 32-bit
    hash ^= hash >> 0x10;
    hash *= 0x35ea2b2cU;
    hash ^= hash >> 0xd;

    return hash;
}