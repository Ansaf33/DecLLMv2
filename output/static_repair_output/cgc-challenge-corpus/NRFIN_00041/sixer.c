#include <stdint.h> // For uint32_t, uint8_t, int32_t
#include <string.h> // For strlen
#include <stddef.h> // For size_t

// Define the sixer state structure based on usage patterns
typedef struct {
    uint32_t bit_offset;
    char *data_ptr;
    char *start_ptr;
} sixer_state_t;

// Function: get_byte_mask
uint32_t get_byte_mask(uint8_t param_1) {
    switch (param_1) {
        case 0: return 0x3f;
        case 1: return 0x1f;
        case 2: return 0xf;
        case 3: return 7;
        case 4: return 3;
        case 5: return 1;
        default: return 0;
    }
}

// Function: init_sixer
void init_sixer(sixer_state_t *state, char *data_start) {
    state->bit_offset = 0;
    state->start_ptr = data_start;
    state->data_ptr = data_start;
}

// Function: sixer_strlen
int sixer_strlen(sixer_state_t *state) {
    size_t len = strlen(state->data_ptr);
    return (int)(len * 6 - state->bit_offset);
}

// Function: unarmor_ASCII_char
uint32_t unarmor_ASCII_char(uint8_t ascii_char) {
    if ((ascii_char < 0x78) && (ascii_char < 0x58 || ascii_char > 0x5f)) {
        uint8_t temp_val = ascii_char - 0x30;
        if (temp_val > 0x27) {
            temp_val = ascii_char - 0x38;
        }
        return (uint32_t)temp_val;
    } else {
        return 0xffffffff; // Error value
    }
}

// Function: get_bits_from_sixer
uint32_t get_bits_from_sixer(sixer_state_t *state, unsigned int num_bits) {
    uint32_t result_bits = 0;
    while (num_bits > 0) {
        uint32_t unarmored_val = unarmor_ASCII_char(*(uint8_t *)state->data_ptr);
        uint32_t bits_available_in_current_char = 6 - state->bit_offset;
        uint32_t bits_to_extract_this_step = (num_bits < bits_available_in_current_char) ? num_bits : bits_available_in_current_char;

        uint32_t relevant_bits_masked = unarmored_val & get_byte_mask(state->bit_offset);
        uint32_t extracted_portion = relevant_bits_masked >> (bits_available_in_current_char - bits_to_extract_this_step);

        result_bits = (result_bits << (bits_to_extract_this_step & 0x1f)) | extracted_portion;

        state->bit_offset += bits_to_extract_this_step;
        num_bits -= bits_to_extract_this_step;

        if (state->bit_offset == 6) {
            state->data_ptr++;
            state->bit_offset = 0;
        }
    }
    return result_bits;
}

// Function: get_msg_type
void get_msg_type(char *msg_data) {
    sixer_state_t state;
    init_sixer(&state, msg_data);
    (void)get_bits_from_sixer(&state, 3); // Discarding return value
}

// Function: sixer_bits_twos_to_sint
int32_t sixer_bits_twos_to_sint(uint32_t value, uint32_t sign_bit_mask) {
    if ((value & sign_bit_mask) == 0) {
        return (int32_t)value;
    } else {
        return -((int32_t)(sign_bit_mask - 1) & -(int32_t)value);
    }
}

// Function: sixer_bits_to_ASCII_str_char
uint32_t sixer_bits_to_ASCII_str_char(uint8_t value) {
    if (value < 0x40) {
        if (value < 0x20) {
            return (uint32_t)(value + 0x40);
        } else {
            return (uint32_t)value;
        }
    } else {
        return 0xffffffff;
    }
}

// Main function (minimal for compilation)
int main() {
    sixer_state_t state;
    char *test_data = "SAMPLE";

    init_sixer(&state, test_data);
    (void)sixer_strlen(&state);
    (void)get_msg_type(test_data);
    (void)get_bits_from_sixer(&state, 1);
    (void)get_byte_mask(0);
    (void)unarmor_ASCII_char('A');
    (void)sixer_bits_twos_to_sint(5, 4);
    (void)sixer_bits_to_ASCII_str_char(17);

    return 0;
}