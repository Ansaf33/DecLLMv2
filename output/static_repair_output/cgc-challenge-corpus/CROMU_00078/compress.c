#include <stdlib.h> // For abs
#include <string.h> // For memcpy
#include <stdint.h> // For intptr_t

// Define custom types
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char undefined; // Used for 0xff assignment

// Assume these are global arrays of int, defined elsewhere
extern int red_blue[];
extern int green[];

// Function: ClosestMatch
byte ClosestMatch(char param_1, short param_2) {
    byte best_diff = 0xff;
    byte best_index = 0;
    byte count_limit;

    if (param_1 == '\0' || param_1 == '\x02') {
        count_limit = 8;
    } else {
        count_limit = 4;
    }

    for (int i = 0; i < count_limit; ++i) {
        int diff_val;
        if (param_1 == '\0' || param_1 == '\x02') {
            diff_val = abs((int)param_2 - red_blue[i]);
        } else {
            diff_val = abs((int)param_2 - green[i]);
        }

        if ((byte)diff_val < best_diff) {
            best_diff = (byte)diff_val;
            best_index = (byte)i;
        }
    }
    return best_index;
}

// Function: Compress
// input_ptrs: array of pointers to SourceStruct-like data (original param_1)
// output_buf: buffer to write compressed data (original param_2)
// output_len_ptr: pointer to ushort to store the total compressed length (original param_3)
void Compress(void **input_ptrs, void *output_buf, ushort *output_len_ptr) {
    if (input_ptrs == NULL) {
        return;
    }

    ushort output_offset = 0;
    const ushort num_items = 0xb2f; // Original local_1a

    for (int item_idx = 0; item_idx < num_items; ++item_idx) {
        // current_source_item points to a structure with data and color bytes
        void *current_source_item = input_ptrs[item_idx];

        byte compressed_byte = 0;
        // ClosestMatch takes short, so cast the byte at offset 6, 7, 8 to short
        compressed_byte |= (ClosestMatch(0, *(byte *)((byte *)current_source_item + 6)) & 0x7) << 5;
        compressed_byte |= (ClosestMatch(1, *(byte *)((byte *)current_source_item + 7)) & 0x3) << 3;
        compressed_byte |= (ClosestMatch(2, *(byte *)((byte *)current_source_item + 8)) & 0x7);

        // Copy 3 ushorts (6 bytes) from current_source_item
        memcpy((byte *)output_buf + output_offset, current_source_item, 2);
        output_offset += 2;
        memcpy((byte *)output_buf + output_offset, (byte *)current_source_item + 2, 2);
        output_offset += 2;
        memcpy((byte *)output_buf + output_offset, (byte *)current_source_item + 4, 2);
        output_offset += 2;

        // Copy the compressed byte
        memcpy((byte *)output_buf + output_offset, &compressed_byte, 1);
        output_offset += 1;
    }

    *output_len_ptr = output_offset;
}

// Function: Decompress
// input_buf: buffer containing compressed data (original param_1)
// output_buf: buffer to write decompressed data (original param_2)
// total_compressed_len_ptr: pointer to ushort holding total compressed length from Compress,
//                           updated with total decompressed length (original param_3)
void Decompress(void *input_buf, void *output_buf, ushort *total_compressed_len_ptr) {
    uint input_offset = 0;
    int output_offset = 0;

    while (input_offset < *total_compressed_len_ptr) {
        // Copy first 2 bytes (ushort)
        memcpy((byte *)output_buf + output_offset, (byte *)input_buf + input_offset, 2);
        output_offset += 2;
        input_offset += 2;

        // Copy second 2 bytes (ushort)
        memcpy((byte *)output_buf + output_offset, (byte *)input_buf + input_offset, 2);
        output_offset += 2;
        input_offset += 2;

        // Copy third 2 bytes (ushort)
        memcpy((byte *)output_buf + output_offset, (byte *)input_buf + input_offset, 2);
        
        // compressed_byte_input_pos points to the byte immediately after the third ushort
        int compressed_byte_input_pos = input_offset + 2;
        input_offset += 3; // Advance input_offset past the 3rd ushort (2 bytes) and the compressed byte (1 byte)

        // At this point, output_offset is 6 (after 3 ushorts)
        
        byte compressed_val = *(byte *)((byte *)input_buf + compressed_byte_input_pos);
        byte b6_idx = compressed_val >> 5;
        byte b7_idx = (compressed_val >> 3) & 3;
        byte b8_idx = compressed_val & 7;

        // Write derived bytes to output_buf + output_offset + 2, +3, +4
        // These write to output_buf + 8, +9, +10
        *((char *)output_buf + output_offset + 2) = (char)red_blue[(uint)b6_idx];
        *((char *)output_buf + output_offset + 3) = (char)green[(uint)b7_idx];
        *((char *)output_buf + output_offset + 4) = (char)red_blue[(uint)b8_idx];
        
        // Advance output_offset by 6 bytes (to account for the 3 ushorts and the following 3 derived chars, plus a 3-byte gap)
        output_offset += 6; // output_offset becomes 12 (6 + 6)

        // Write 0xff to output_buf + output_offset + 5 (12 + 5 = 17)
        *((byte *)output_buf + output_offset + 5) = 0xff;
        
        // Advance output_offset by 6 bytes to prepare for next item (total 18 bytes per item)
        output_offset += 6;
    }
    // Update total_compressed_len_ptr with the actual total decompressed length
    *total_compressed_len_ptr = output_offset;
}

// Function: WriteOut
// output_ptr_array: array of int (pointers to destination structures) (original param_1)
// decompressed_buf: buffer containing decompressed data (10 bytes per item expected by WriteOut) (original param_2)
// decompressed_total_len: total length of decompressed_buf (original param_3)
//                         (Note: Decompress produces 18 bytes per item, WriteOut consumes 10 bytes per item.
//                         This implies a potential mismatch or specific data interpretation in the original logic.)
void WriteOut(int *output_ptr_array, void *decompressed_buf, ushort decompressed_total_len) {
    if (output_ptr_array == NULL) {
        return;
    }

    // The loop iterates based on decompressed_total_len but increments by 10.
    // This behavior from the original code is preserved.
    for (ushort data_offset = 0, item_idx = 0; data_offset < decompressed_total_len; data_offset += 10, ++item_idx) {
        // output_ptr_array contains int values that are treated as destination addresses
        void *current_dest_ptr = (void *)(intptr_t)output_ptr_array[item_idx];

        // Read 3 bytes from decompressed_buf + data_offset + 6, +7, +8
        // Write to current_dest_ptr + 6, +7, +8
        memcpy((byte *)current_dest_ptr + 6, (byte *)decompressed_buf + data_offset + 6, 1);
        memcpy((byte *)current_dest_ptr + 7, (byte *)decompressed_buf + data_offset + 7, 1);
        memcpy((byte *)current_dest_ptr + 8, (byte *)decompressed_buf + data_offset + 8, 1);
        
        // Write 0xff to current_dest_ptr + 9
        *((byte *)current_dest_ptr + 9) = 0xff;
    }
}