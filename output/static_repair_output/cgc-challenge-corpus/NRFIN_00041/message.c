#include <stdio.h>  // For snprintf
#include <stdint.h> // For uint32_t, uint8_t

// Define custom types
// sixer_t is a placeholder for a structure or array that manages bit-level data.
// The original code uses `undefined local_24 [12]`, suggesting an array of 12 units
// of `undefined` type. Assuming `undefined` maps to `uint32_t` (4 bytes), this would be 48 bytes.
typedef uint32_t sixer_t[12];
typedef uint8_t byte;

// Define constants
#define ERROR_VAL 0xffffffcd

// Forward declarations for external functions
// These functions are assumed to be defined elsewhere in the project.
// Their parameters are mapped from the original `undefined4`, `int`, `byte`, `uint` to standard C types.
byte get_msg_type(uint32_t msg_content_ptr);
void init_sixer(sixer_t *sixer_data, uint32_t msg_content_ptr);
int sixer_strlen(const sixer_t *sixer_data);
int get_bits_from_sixer(sixer_t *sixer_data, int num_bits);
int sixer_bits_twos_to_sint(int value, int sign_bit_mask);
char sixer_bits_to_ASCII_str_char(uint8_t six_bit_char_code);

// Forward declarations for parse functions
uint32_t parse_msg_type_1(char *output_buf, uint32_t msg_data_ptr);
uint32_t parse_msg_type_4(char *output_buf, uint32_t msg_data_ptr);
uint32_t parse_msg_type_5(char *output_buf, uint32_t msg_data_ptr);

// Global data (placeholders for compilation)
// The original code accesses these as `*(undefined4 *)(ARRAY_NAME + index * 4)`,
// implying an array of 4-byte entities, likely pointers to strings.
const char *const STATUS[] = {
    "Status 0", "Status 1", "Status 2", "Status 3", "Status 4",
    "Status 5", "Status 6", "Status 7", "Status 8"
};

const char *const MANEUEVER[] = {
    "Maneuver 0", "Maneuver 1", "Maneuver 2"
};

const char *const EPFD[] = {
    "EPFD 0", "EPFD 1", "EPFD 2", "EPFD 3", "EPFD 4",
    "EPFD 5", "EPFD 6", "EPFD 7", "EPFD 8"
};

// DAT_000141de is used as a string literal.
const char DAT_000141de[] = "unk";

// Function: parse_msg_type_1
uint32_t parse_msg_type_1(char *output_buf, uint32_t msg_data_ptr) {
    sixer_t sixer_data;
    init_sixer(&sixer_data, msg_data_ptr);

    if (sixer_strlen(&sixer_data) != 0x90) { // Expected total bit length 144
        return ERROR_VAL;
    }

    int val; // Variable to hold results from get_bits_from_sixer

    val = get_bits_from_sixer(&sixer_data, 3);
    if (val != 1) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 8, "%s,", "type 1");

    val = get_bits_from_sixer(&sixer_data, 0x1e); // 30 bits
    if (val < 0) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 0xb, "%d,", val);

    val = get_bits_from_sixer(&sixer_data, 4);
    if (val < 0 || val > 8) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 0x1c, "%s,", STATUS[val]);

    val = get_bits_from_sixer(&sixer_data, 8); // 8 bits
    if (val > 0x80) { // If sign bit is set (0x80 for 8-bit two's complement)
        val = sixer_bits_twos_to_sint(val, 0x80);
    }
    if (val == 0) {
        output_buf += snprintf(output_buf, 0xd, "%s,", "not turning");
    } else if (val < 0) {
        output_buf += snprintf(output_buf, 0x12, "%s %d,", "turning left", -val);
    } else {
        output_buf += snprintf(output_buf, 0x13, "%s %d,", "turning right", val);
    }

    val = get_bits_from_sixer(&sixer_data, 10); // 10 bits
    if (val == 0x3ff) { // 1023
        output_buf += snprintf(output_buf, 0xb, "%s,", "speed unk");
    } else if (val < 0x3fd) { // < 1021
        output_buf += snprintf(output_buf, 8, "%dkts,", val / 10);
    } else { // 1021 or 1022
        output_buf += snprintf(output_buf, 9, "%s,", ">102kts");
    }

    (void)get_bits_from_sixer(&sixer_data, 1); // Result unused

    val = get_bits_from_sixer(&sixer_data, 0x1b); // 27 bits
    if (val > 0x535020) { // If sign bit is set (0x800000 for 27-bit two's complement)
        val = sixer_bits_twos_to_sint(val, 0x800000);
    }
    if (val == 0x535020) { // Specific unknown value
        output_buf += snprintf(output_buf, 9, "%s,", "lat unk");
    } else if (val == 0) {
        output_buf += snprintf(output_buf, 3, "%d,", 0);
    } else if (val < 1) { // Negative
        output_buf += snprintf(output_buf, 10, "%dS,", -val);
    } else { // Positive
        output_buf += snprintf(output_buf, 10, "%dN,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 0x1c); // 28 bits
    if (val > 0xa5b5e0) { // If sign bit is set (0x1000000 for 28-bit two's complement)
        val = sixer_bits_twos_to_sint(val, 0x1000000);
    }
    if (val == 0xa5b5e0) { // Specific unknown value
        output_buf += snprintf(output_buf, 9, "%s,", "lon unk");
    } else if (val == 0) {
        output_buf += snprintf(output_buf, 3, "%d,", 0);
    } else if (val < 1) { // Negative
        output_buf += snprintf(output_buf, 0xb, "%dW,", -val);
    } else { // Positive
        output_buf += snprintf(output_buf, 0xb, "%dE,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 0xc); // 12 bits
    if (val == 0xe10) { // 3600
        output_buf += snprintf(output_buf, 0xc, "%s,", "course unk");
    } else {
        if (val > 0xe06) { // Max 3590 (359.0 degrees)
            return ERROR_VAL;
        }
        output_buf += snprintf(output_buf, 8, "c:%d,", val / 10);
    }

    val = get_bits_from_sixer(&sixer_data, 9); // 9 bits
    if (val == 0x1ff) { // 511
        output_buf += snprintf(output_buf, 0xd, "%s,", "heading unk");
    } else {
        if (val > 0x167) { // Max 359 degrees
            return ERROR_VAL;
        }
        output_buf += snprintf(output_buf, 10, "h:%d TN,", val);
    }

    (void)get_bits_from_sixer(&sixer_data, 6); // Result unused

    val = get_bits_from_sixer(&sixer_data, 2); // 2 bits
    if (val == 3) {
        return ERROR_VAL;
    }
    snprintf(output_buf, 0x18, "%s.", MANEUEVER[val]);

    return 0; // Success
}

// Function: parse_msg_type_4
uint32_t parse_msg_type_4(char *output_buf, uint32_t msg_data_ptr) {
    sixer_t sixer_data;
    init_sixer(&sixer_data, msg_data_ptr);

    if (sixer_strlen(&sixer_data) != 0x8a) { // Expected total bit length 138
        return ERROR_VAL;
    }

    int val;

    val = get_bits_from_sixer(&sixer_data, 3);
    if (val != 4) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 8, "%s,", "type 4");

    val = get_bits_from_sixer(&sixer_data, 0x1e); // 30 bits
    if (val < 0) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 0xb, "%d,", val);

    val = get_bits_from_sixer(&sixer_data, 0xe); // 14 bits
    if (val == 0) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 6, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 4); // 4 bits
    if (val >= 0xd) { // Max 12
        return ERROR_VAL;
    }
    if (val == 0) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 5); // 5 bits
    if (val >= 0x20) { // Max 31
        return ERROR_VAL;
    }
    if (val == 0) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 5); // 5 bits
    if (val >= 0x19) { // Max 24 (0x18 is 24)
        return ERROR_VAL;
    }
    if (val == 0x18) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 6); // 6 bits
    if (val >= 0x3d) { // Max 60 (0x3c is 60)
        return ERROR_VAL;
    }
    if (val == 0x3c) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 6); // 6 bits
    if (val >= 0x3d) { // Max 60 (0x3c is 60)
        return ERROR_VAL;
    }
    if (val == 0x3c) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    (void)get_bits_from_sixer(&sixer_data, 1); // Result unused

    val = get_bits_from_sixer(&sixer_data, 0x1b); // 27 bits
    if (val > 0x535020) {
        val = sixer_bits_twos_to_sint(val, 0x800000);
    }
    if (val == 0x535020) {
        output_buf += snprintf(output_buf, 9, "%s,", "lat unk");
    } else if (val == 0) {
        output_buf += snprintf(output_buf, 3, "%d,", 0);
    } else if (val < 1) { // Negative
        output_buf += snprintf(output_buf, 10, "%dS,", -val);
    } else { // Positive
        output_buf += snprintf(output_buf, 10, "%dN,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 0x1c); // 28 bits
    if (val > 0xa5b5e0) {
        val = sixer_bits_twos_to_sint(val, 0x1000000);
    }
    if (val == 0xa5b5e0) {
        output_buf += snprintf(output_buf, 9, "%s,", "lon unk");
    } else if (val == 0) {
        output_buf += snprintf(output_buf, 3, "%d,", 0);
    } else if (val < 1) { // Negative
        output_buf += snprintf(output_buf, 0xb, "%dW,", -val);
    } else { // Positive
        output_buf += snprintf(output_buf, 0xb, "%dE,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 4); // 4 bits
    if (val >= 9) { // Max 8
        return ERROR_VAL;
    }
    snprintf(output_buf, 0x1e, "%s.", EPFD[val]);

    return 0; // Success
}

// Function: parse_msg_type_5
uint32_t parse_msg_type_5(char *output_buf, uint32_t msg_data_ptr) {
    sixer_t sixer_data;
    init_sixer(&sixer_data, msg_data_ptr);

    if (sixer_strlen(&sixer_data) != 0x180) { // Expected total bit length 384
        return ERROR_VAL;
    }

    int val;
    int is_at_char_seen = 0; // Flag to stop printing characters after an '@' is encountered

    val = get_bits_from_sixer(&sixer_data, 3);
    if (val != 5) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 8, "%s,", "type 5");

    val = get_bits_from_sixer(&sixer_data, 0x1e); // 30 bits
    if (val < 0) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 0xb, "%d,", val);

    val = get_bits_from_sixer(&sixer_data, 0x1e); // 30 bits
    if (val < 0) {
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 0xb, "%d,", val);

    is_at_char_seen = 0;
    for (int i = 0; i < 7; ++i) {
        val = get_bits_from_sixer(&sixer_data, 6);
        if (val < 0) {
            return ERROR_VAL;
        }
        char char_val = sixer_bits_to_ASCII_str_char((uint8_t)(val & 0xff));
        if (char_val == '@' || is_at_char_seen) {
            is_at_char_seen = 1;
        } else {
            *output_buf++ = char_val;
        }
    }
    *output_buf++ = ',';

    is_at_char_seen = 0;
    for (int i = 0; i < 0x14; ++i) { // 20 iterations
        val = get_bits_from_sixer(&sixer_data, 6);
        if (val < 0) {
            return ERROR_VAL;
        }
        char char_val = sixer_bits_to_ASCII_str_char((uint8_t)(val & 0xff));
        if (char_val == '@' || is_at_char_seen) {
            is_at_char_seen = 1;
        } else {
            *output_buf++ = char_val;
        }
    }
    *output_buf++ = ',';

    val = get_bits_from_sixer(&sixer_data, 4); // 4 bits
    if (val >= 9) { // Max 8
        return ERROR_VAL;
    }
    output_buf += snprintf(output_buf, 0x1e, "%s,", EPFD[val]);

    val = get_bits_from_sixer(&sixer_data, 4); // 4 bits
    if (val >= 0xd) { // Max 12
        return ERROR_VAL;
    }
    if (val == 0) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 5); // 5 bits
    if (val >= 0x20) { // Max 31
        return ERROR_VAL;
    }
    if (val == 0) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 5); // 5 bits
    if (val >= 0x19) { // Max 24 (0x18 is 24)
        return ERROR_VAL;
    }
    if (val == 0x18) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 6); // 6 bits
    if (val >= 0x3d) { // Max 60 (0x3c is 60)
        return ERROR_VAL;
    }
    if (val == 0x3c) {
        output_buf += snprintf(output_buf, 5, "%s,", DAT_000141de);
    } else {
        output_buf += snprintf(output_buf, 4, "%d,", val);
    }

    val = get_bits_from_sixer(&sixer_data, 8); // 8 bits
    output_buf += snprintf(output_buf, 5, "%d,", val);

    is_at_char_seen = 0;
    for (int i = 0; i < 0x14; ++i) { // 20 iterations
        val = get_bits_from_sixer(&sixer_data, 6);
        if (val < 0) {
            return ERROR_VAL;
        }
        char char_val = sixer_bits_to_ASCII_str_char((uint8_t)(val & 0xff));
        if (char_val == '@' || is_at_char_seen) {
            is_at_char_seen = 1;
        } else {
            *output_buf++ = char_val;
        }
    }
    *output_buf = '.'; // Final character, no trailing comma

    return 0; // Success
}

// Function: to_english
// param_1 is `undefined4` in the snippet, but used as `char *` for output buffer.
// Changed to `char *output_buf` for correct type usage.
uint32_t to_english(char *output_buf, int msg_data_ptr) {
    // Check message header byte at offset 0x10
    if (*(char *)(msg_data_ptr + 0x10) != '\x03') {
        return ERROR_VAL;
    }

    // Get pointer to message content at offset 0x14
    uint32_t msg_content_ptr = *(uint32_t *)(msg_data_ptr + 0x14);
    // Get message type
    byte msg_type = get_msg_type(msg_content_ptr);

    // Check if message type matches expected value at offset 0x11
    if (msg_type != *(byte *)(msg_data_ptr + 0x11)) {
        return ERROR_VAL;
    }

    // Dispatch to appropriate parser function based on message type
    if (msg_type == 5) {
        return parse_msg_type_5(output_buf, msg_content_ptr);
    } else if (msg_type == 1) {
        return parse_msg_type_1(output_buf, msg_content_ptr);
    } else if (msg_type == 4) {
        return parse_msg_type_4(output_buf, msg_content_ptr);
    }
    // If message type is not handled (e.g., 0, 2, 3, or >= 6)
    return ERROR_VAL;
}