#include <stdlib.h> // For abs
#include <string.h> // For memcpy

// Define custom types based on common decompiler output
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

// Declare global arrays.
// The original code implies they are arrays of int, accessed with `*(int *)(array + index * 4)`.
// ClosestMatch uses '8' or '4' as limit.
// For param_1=0 or 2, limit is 8, so red_blue needs at least 8 elements.
// For param_1=1, limit is 4, so green needs at least 4 elements.
// Declaring them as int[8] to be safe and cover all accesses.
int red_blue[8];
int green[8];

// Function: ClosestMatch
byte ClosestMatch(char param_1, short param_2) {
    unsigned char min_diff = 0xFF;
    unsigned char best_match_idx = 0;

    // Determine the array limit and the array to use based on param_1
    unsigned char limit = (param_1 == 0 || param_1 == 2) ? 8 : 4;
    const int* current_array = (param_1 == 0 || param_1 == 2) ? red_blue : green;

    for (int i = 0; i < limit; ++i) {
        int diff = param_2 - current_array[i];
        unsigned char current_diff = (unsigned char)abs(diff);

        if (current_diff < min_diff) {
            min_diff = current_diff;
            best_match_idx = (unsigned char)i;
        }
    }
    return best_match_idx;
}

// Function: Compress
void Compress(int param_1, int param_2, void *param_3) {
    if (param_1 != 0) {
        // 0xB2F (2863) is likely the number of items to process
        unsigned short item_count = 0xB2F;
        unsigned short dest_offset = 0; // Current offset in the destination buffer (param_2)

        // param_1 is an array of pointers (int* or void**), each pointing to a source item.
        void** source_item_pointers = (void**)param_1;
        // param_2 is the destination buffer (unsigned char*)
        unsigned char* dest_buffer = (unsigned char*)param_2;

        for (int i = 0; i < item_count; ++i) {
            void* source_item_ptr = source_item_pointers[i];
            unsigned char* source_item_byte_ptr = (unsigned char*)source_item_ptr;

            unsigned char compressed_byte = 0;

            // Call ClosestMatch for bytes at offsets 6, 7, 8 from source_item_ptr
            // and combine results into a single compressed byte.
            // Each ClosestMatch result (0-7 or 0-3) is masked to fit the allocated bits.
            compressed_byte |= (ClosestMatch(0, source_item_byte_ptr[6]) & 0x7) << 5; // 3 bits for val 0-7
            compressed_byte |= (ClosestMatch(1, source_item_byte_ptr[7]) & 0x3) << 3; // 2 bits for val 0-3
            compressed_byte |= (ClosestMatch(2, source_item_byte_ptr[8]) & 0x7);      // 3 bits for val 0-7

            // Copy first 2 bytes from source item to destination
            memcpy(dest_buffer + dest_offset, source_item_ptr, 2);
            dest_offset += 2;
            // Copy next 2 bytes from source item (offset 2)
            memcpy(dest_buffer + dest_offset, source_item_byte_ptr + 2, 2);
            dest_offset += 2;
            // Copy next 2 bytes from source item (offset 4)
            memcpy(dest_buffer + dest_offset, source_item_byte_ptr + 4, 2);
            dest_offset += 2;
            // Write the compressed byte
            memcpy(dest_buffer + dest_offset, &compressed_byte, 1);
            dest_offset += 1; // Total 7 bytes written per item (6 data + 1 compressed)
        }
        // Store the final destination offset (total compressed size) in param_3
        memcpy(param_3, &dest_offset, sizeof(ushort));
    }
}

// Function: Decompress
void Decompress(int param_1, int param_2, ushort *param_3) {
    unsigned int current_src_offset = 0;  // Current offset in the source buffer (param_1)
    unsigned int current_dest_offset = 0; // Current offset in the destination buffer (param_2)

    unsigned char* src_byte_ptr = (unsigned char*)param_1;
    unsigned char* dest_byte_ptr = (unsigned char*)param_2;

    // *param_3 holds the total compressed size (from Compress function)
    while (current_src_offset < *param_3) {
        // Copy 3 blocks of 2 bytes from source to destination
        memcpy(dest_byte_ptr + current_dest_offset, src_byte_ptr + current_src_offset, 2);
        current_src_offset += 2;
        memcpy(dest_byte_ptr + current_dest_offset + 2, src_byte_ptr + current_src_offset, 2);
        current_src_offset += 2;
        memcpy(dest_byte_ptr + current_dest_offset + 4, src_byte_ptr + current_src_offset, 2);
        current_src_offset += 2; // current_src_offset now advanced by 6 bytes

        // Read the single compressed byte
        unsigned char compressed_byte = src_byte_ptr[current_src_offset];
        current_src_offset += 1; // current_src_offset now advanced by 7 bytes (6 data + 1 compressed)

        // Extract color indices from the compressed byte
        unsigned char val_rb_1 = compressed_byte >> 5;         // 3 bits
        unsigned char val_g = (compressed_byte >> 3) & 0x3;    // 2 bits
        unsigned char val_rb_2 = compressed_byte & 0x7;        // 3 bits

        // Write the decompressed color values and 0xff byte
        // These are written to offsets +6, +7, +8, +9 relative to the start of the current 10-byte item.
        // The original code implies only the least significant byte of the int from red_blue/green is used.
        dest_byte_ptr[current_dest_offset + 6] = (unsigned char)((unsigned int)red_blue[val_rb_1]);
        dest_byte_ptr[current_dest_offset + 7] = (unsigned char)((unsigned int)green[val_g]);
        dest_byte_ptr[current_dest_offset + 8] = (unsigned char)((unsigned int)red_blue[val_rb_2]);
        dest_byte_ptr[current_dest_offset + 9] = 0xff;

        current_dest_offset += 10; // Each decompressed item is 10 bytes in the destination
    }
    // Store the final destination offset (total decompressed size) back into param_3
    memcpy(param_3, &current_dest_offset, sizeof(ushort));
}

// Function: WriteOut
void WriteOut(int param_1, int param_2, ushort param_3) {
    if (param_1 != 0) {
        // param_1 is an array of pointers to original items (void**)
        void** original_item_ptrs = (void**)param_1;
        // param_2 is the buffer containing decompressed data (unsigned char*)
        unsigned char* decompressed_data_ptr = (unsigned char*)param_2;

        unsigned short source_array_index = 0;
        // param_3 is the total size of decompressed data in param_2
        // Loop iterates through decompressed items, each 10 bytes long
        for (unsigned short current_param_2_offset = 0; current_param_2_offset < param_3; current_param_2_offset += 10) {
            // Get the pointer to the current original item from the array
            void* item_ptr = original_item_ptrs[source_array_index];
            unsigned char* item_byte_ptr = (unsigned char*)item_ptr;

            // Copy the 3 color bytes from decompressed data (offsets +6, +7, +8 relative to item start)
            // into the original item's structure (same offsets)
            memcpy(item_byte_ptr + 6, decompressed_data_ptr + current_param_2_offset + 6, 1);
            memcpy(item_byte_ptr + 7, decompressed_data_ptr + current_param_2_offset + 7, 1);
            memcpy(item_byte_ptr + 8, decompressed_data_ptr + current_param_2_offset + 8, 1);
            
            // Set the byte at offset +9 in the original item to 0xff
            item_byte_ptr[9] = 0xff;

            source_array_index += 1; // Move to the next pointer in the original array
        }
    }
}