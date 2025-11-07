#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Global variables
uint32_t *the_vault;
uint32_t current_time;
uint32_t DAT_0001600c;

// Helper functions for byte swapping
uint16_t swap16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

uint32_t swap32(uint32_t val) {
    return ((val << 24) & 0xff000000) |
           ((val << 8)  & 0x00ff0000) |
           ((val >> 8)  & 0x0000ff00) |
           ((val >> 24) & 0x000000ff);
}

// Placeholder for write_bytes function
void write_bytes(const void *buf, size_t len) {
    // In a real application, this would write to a file descriptor, socket, etc.
    // For compilation, it can be a no-op or print to stdout for debugging.
    // Example: fwrite(buf, 1, len, stdout);
}

// Function: write_msg
void write_msg(uint8_t msg_type, const void *data, int data_len) {
    uint16_t header_type_fixed = swap16(2);
    unsigned int total_len = data_len + 1;

    if (total_len < 0x8000) {
        struct {
            uint16_t type;
            uint16_t length;
            uint8_t msg_type_byte;
        } __attribute__((packed)) header_small;
        header_small.type = header_type_fixed;
        header_small.length = swap16((uint16_t)total_len);
        header_small.msg_type_byte = msg_type;
        write_bytes(&header_small, sizeof(header_small));
    } else {
        struct {
            uint16_t type;
            uint32_t length;
            uint8_t msg_type_byte;
        } __attribute__((packed)) header_large;
        header_large.type = header_type_fixed;
        header_large.length = swap32(total_len | 0x80000000);
        header_large.msg_type_byte = msg_type;
        write_bytes(&header_large, sizeof(header_large));
    }
    write_bytes(data, data_len);
}

// Function: init_vault
void init_vault(void) {
    the_vault = (uint32_t *)malloc(0xfb0); // Allocate 4016 bytes
    if (the_vault == NULL) {
        // Handle allocation failure appropriately
        return;
    }
    the_vault[0] = 0xd0856180;
    the_vault[1] = 0x34c;
    the_vault[2] = 3600000;
    the_vault[0x3eb] = 0; // Index 0x3eb (1003) corresponds to 0xfac bytes offset
}

// Function: store_in_vault
uint32_t *store_in_vault(uint32_t *item_id_ptr_or_null, const void *data, size_t data_len) {
    uint32_t *vault_base = the_vault;
    uint32_t *vault_entry_ptr = NULL;

    if (item_id_ptr_or_null == NULL) { // New item
        if (the_vault[0x3eb] >= 500) { // Max 500 items (0x3eb is index for item count)
            return NULL;
        }
        vault_entry_ptr = vault_base + 3 + the_vault[0x3eb] * 2; // the_vault[3] is start, each item is 2 uint32_t
        the_vault[0x3eb]++;
    } else { // Update existing item
        vault_entry_ptr = item_id_ptr_or_null;
        // Validate item_id_ptr_or_null is within bounds and properly aligned
        if (vault_entry_ptr < vault_base + 3 ||
            vault_entry_ptr >= vault_base + 3 + the_vault[0x3eb] * 2 ||
            ((uintptr_t)vault_entry_ptr - (uintptr_t)(vault_base + 3)) % (2 * sizeof(uint32_t)) != 0) {
            return NULL; // Invalid item_id_ptr_or_null
        }
        // Free existing data before overwriting
        if (*vault_entry_ptr != 0) {
            free((void *)(uintptr_t)*vault_entry_ptr);
        }
    }

    void *new_data_buffer = malloc(data_len);
    if (new_data_buffer == NULL) {
        return NULL; // Failed to allocate new data buffer
    }

    memcpy(new_data_buffer, data, data_len);
    *vault_entry_ptr = (uint32_t)(uintptr_t)new_data_buffer; // Store pointer (assuming 32-bit pointers or truncation is acceptable)
    vault_entry_ptr[1] = (uint32_t)data_len; // Store size

    return vault_entry_ptr;
}

// Function: retrieve_from_vault
uint32_t retrieve_from_vault(uint32_t *item_entry_ptr, uint32_t *retrieved_len_ptr) {
    uint32_t ret_data_ptr_val = 0;

    // Reconstructing CARRY4 for *the_vault + the_vault[2] overflow check
    unsigned int carry_val_for_uVar2 = (UINT32_MAX - *the_vault < the_vault[2]) ? 1 : 0;
    unsigned int uVar2_val = the_vault[1] + carry_val_for_uVar2;

    uint32_t sum_vault0_vault2 = *the_vault + the_vault[2];

    unsigned int cond_current_time_lt_vault0 = (current_time < *the_vault) ? 1U : 0U;
    unsigned int cond_current_time_lt_sum = (current_time < sum_vault0_vault2) ? 1U : 0U;

    bool condition_part1 = (DAT_0001600c < the_vault[1]) || (DAT_0001600c - the_vault[1] < cond_current_time_lt_vault0);
    bool condition_part2 = (uVar2_val <= DAT_0001600c) && (cond_current_time_lt_sum <= (DAT_0001600c - uVar2_val));

    if (condition_part1 || condition_part2) {
        ret_data_ptr_val = 0; // Vault expired or invalid
    } else {
        ret_data_ptr_val = *item_entry_ptr;
        *retrieved_len_ptr = item_entry_ptr[1];
        *item_entry_ptr = 0; // Clear pointer in vault
        item_entry_ptr[1] = 0; // Clear size in vault
    }
    return ret_data_ptr_val;
}

// Function: handle_msg_vault
uint32_t handle_msg_vault(char *msg_buf, int msg_len) {
    uint32_t ret_val = 1;

    if (msg_len == 0) {
        return 0;
    }

    uint8_t msg_type_byte = (uint8_t)*msg_buf;
    int data_len_remaining = msg_len - 1;
    char *data_ptr = msg_buf + 1;

    if (msg_type_byte == 0x01) { // List items
        unsigned int vault_item_count = the_vault[0x3eb];
        size_t total_list_size = vault_item_count * 8;

        void *temp_buf = malloc(total_list_size);
        if (temp_buf == NULL) {
            write_msg(1, NULL, 0);
        } else {
            for (unsigned int i = 0; i < vault_item_count; ++i) {
                uint32_t *current_item_output = (uint32_t *)((char *)temp_buf + i * 8);
                uint32_t *vault_entry = the_vault + 3 + i * 2;
                current_item_output[0] = swap32(*vault_entry);
                current_item_output[1] = swap32(vault_entry[1]);
            }
            write_msg(1, temp_buf, total_list_size);
            free(temp_buf);
        }
    } else if (msg_type_byte == 0x02) { // Store new item
        uint32_t *stored_ptr = store_in_vault(NULL, data_ptr, data_len_remaining);
        uint32_t stored_id_swapped = swap32((uint32_t)(uintptr_t)stored_ptr);
        write_msg(2, &stored_id_swapped, 4);
    } else if (msg_type_byte == 0x03 && data_len_remaining > 3) { // Update item
        uint32_t item_id_to_update = swap32(*(uint32_t*)data_ptr);
        uint32_t *updated_ptr = store_in_vault((uint32_t*)(uintptr_t)item_id_to_update, data_ptr + 4, data_len_remaining - 4);
        uint32_t update_result_swapped = swap32((uint32_t)(uintptr_t)updated_ptr);
        write_msg(3, &update_result_swapped, 4);
    } else if (msg_type_byte == 0x04 && data_len_remaining > 3) { // Retrieve item
        uint32_t item_id_to_retrieve = swap32(*(uint32_t*)data_ptr);
        uint32_t retrieved_len = 0;
        uint32_t retrieved_data_ptr_val = retrieve_from_vault((uint32_t*)(uintptr_t)item_id_to_retrieve, &retrieved_len);
        void *retrieved_data = (void *)(uintptr_t)retrieved_data_ptr_val;

        if (retrieved_data == NULL) {
            write_msg(4, NULL, 0);
        } else {
            write_msg(4, retrieved_data, retrieved_len);
        }
        free(retrieved_data);
    } else {
        ret_val = 0; // Unknown message type or invalid length
    }
    return ret_val;
}

// Main function (minimal for compilation)
int main() {
    init_vault();

    // Example: Clean up the vault if it was initialized
    if (the_vault != NULL) {
        // In a real scenario, you'd free all dynamically allocated data
        // pointed to by vault entries before freeing the vault itself.
        // For this minimal example, we just free the vault itself.
        free(the_vault);
        the_vault = NULL;
    }

    return 0;
}