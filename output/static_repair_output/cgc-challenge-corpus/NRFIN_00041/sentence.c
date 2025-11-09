#include <string.h> // For memset, strncmp
#include <stdio.h>  // For NULL (standard for C)
#include <stdlib.h> // For general utilities (not strictly needed by snippet, but common)

// Define error codes
#define ERR_INVALID_PARAM -49
#define ERR_NOT_FOUND -50
#define ERR_PARSE_FAIL -50 // Using same value as NOT_FOUND as in original

// Helper function: checks if a character is a hexadecimal digit
static int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

// Helper function: converts an ASCII hex character to its binary value
static char ascii_hex_to_bin(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1; // Error for non-hex digit
}

// Helper function: converts an ASCII decimal digit character to its integer value
static int char_to_decimal_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    return -1; // Error for non-decimal digit
}

// Placeholder for get_msg_type (not provided in snippet)
// Assuming it extracts a single digit from the field
static char get_msg_type(char *field) {
    if (field == NULL || *field == '\0') return 0;
    if (*field >= '0' && *field <= '9') return char_to_decimal_digit(*field);
    return 0; // Default or error
}

// Struct definition inferred from param_2 usage in original code
// This helps make the code more readable and type-safe
typedef struct {
    int current_part;   // Corresponds to *param_2
    int total_parts;    // Corresponds to param_2[1]
    int session_id;     // Corresponds to param_2[2]
    char *data_ptr;     // Corresponds to param_2[3] (points into data_buffer)
    char status;        // Corresponds to *(char *)((char *)s_info + 16)
    char message_type;  // Corresponds to *(char *)((char *)s_info + 17)
    char _padding[2];   // Padding to align data_buffer to 4-byte boundary for param_2[5] access
    char data_buffer[0x41]; // Corresponds to param_2[5]
} SentenceInfo;

// Function: get_sentence_start
int get_sentence_start(char *sentence_str, char **sentence_start_ptr) {
    if (sentence_str == NULL || sentence_start_ptr == NULL) {
        return ERR_INVALID_PARAM;
    }

    while (*sentence_str != '!' && *sentence_str != '\0') {
        sentence_str++;
    }

    if (*sentence_str == '!') {
        *sentence_start_ptr = sentence_str;
        return 0;
    } else {
        *sentence_start_ptr = NULL;
        return ERR_NOT_FOUND;
    }
}

// Function: is_checksum_correct
int is_checksum_correct(char *sentence_str) {
    char *checksum_data_start_ptr = NULL;
    int ret = get_sentence_start(sentence_str, &checksum_data_start_ptr);

    if (ret != 0) {
        return ERR_PARSE_FAIL;
    }

    unsigned char calculated_checksum = 0;
    char *p = checksum_data_start_ptr + 1; // Start after '!'

    while (*p != '\0' && *p != '*') {
        calculated_checksum ^= *p;
        p++;
    }

    if (*p != '*') { // Checksum separator not found
        return ERR_PARSE_FAIL;
    }

    // Now p points to '*'
    char hex_digit1_char = p[1];
    char hex_digit2_char = p[2];

    if (!is_hex_digit(hex_digit1_char) || !is_hex_digit(hex_digit2_char)) {
        return ERR_PARSE_FAIL;
    }

    char hex_digit1_val = ascii_hex_to_bin(hex_digit1_char);
    char hex_digit2_val = ascii_hex_to_bin(hex_digit2_char);

    unsigned char expected_checksum = (unsigned char)(hex_digit1_val * 0x10 + hex_digit2_val);

    return (calculated_checksum == expected_checksum) ? 1 : 0;
}

// Function: field_to_uint
int field_to_uint(char *field_str, unsigned int *value) {
    if (field_str == NULL || value == NULL) {
        return ERR_INVALID_PARAM;
    }
    
    *value = 0;

    while (*field_str != '\0' && *field_str != '*' && *field_str != ',') {
        int digit = char_to_decimal_digit(*field_str);
        if (digit < 0) {
            return ERR_PARSE_FAIL; // Not a valid decimal digit
        }
        *value = *value * 10 + digit;
        field_str++;
    }
    return 0;
}

// Function: get_next_field
char *get_next_field(char *current_field_start) {
    if (current_field_start == NULL) {
        return NULL;
    }

    while (*current_field_start != '\0' && *current_field_start != '*' && *current_field_start != ',') {
        current_field_start++;
    }

    if (*current_field_start == '\0') { // End of string
        return NULL;
    } else { // Found separator (',' or '*')
        return current_field_start + 1;
    }
}

// Function: fieldncpy
char *fieldncpy(char *src, char *dest, int max_len) {
    if (src == NULL || dest == NULL) {
        if (dest != NULL) *dest = '\0'; // Ensure dest is null-terminated if valid
        return NULL; 
    }

    int i = 0;
    while (i < max_len && src[i] != '\0' && src[i] != '*' && src[i] != ',') {
        dest[i] = src[i];
        i++;
    }
    
    dest[i] = '\0'; // Null-terminate the copied string
    return dest + i; // Return pointer to the null terminator
}

// Function: parse_sentence
int parse_sentence(char *sentence_str, SentenceInfo *s_info) {
    char *current_field_ptr = NULL;
    int ret = 0; // Return code variable

    if (sentence_str == NULL || s_info == NULL) {
        return ERR_INVALID_PARAM;
    }

    // 1. Get sentence start
    ret = get_sentence_start(sentence_str, &current_field_ptr);
    if (ret != 0) {
        return ret;
    }

    // 2. Check sentence type ("!AAAAA")
    if (strncmp("!AAAAA", current_field_ptr, 6) != 0) {
        return ERR_PARSE_FAIL;
    }

    // 3. Check checksum
    if (is_checksum_correct(current_field_ptr) != 1) {
        return ERR_PARSE_FAIL;
    }

    // Advance to first data field (after "!AAAAA")
    current_field_ptr = get_next_field(current_field_ptr);
    if (current_field_ptr == NULL) return ERR_PARSE_FAIL;

    // 4. Parse field 1 (current part number)
    unsigned int current_part_num = 0;
    ret = field_to_uint(current_field_ptr, &current_part_num);
    if (ret != 0) return ret;

    // Advance to next field
    current_field_ptr = get_next_field(current_field_ptr);
    if (current_field_ptr == NULL) return ERR_PARSE_FAIL;

    // 5. Parse field 2 (total parts)
    unsigned int total_parts_num = 0;
    ret = field_to_uint(current_field_ptr, &total_parts_num);
    if (ret != 0) return ret;

    // Advance to next field
    current_field_ptr = get_next_field(current_field_ptr);
    if (current_field_ptr == NULL) return ERR_PARSE_FAIL;

    // 6. Parse field 3 (session ID)
    unsigned int session_id_val = 0;
    ret = field_to_uint(current_field_ptr, &session_id_val);
    if (ret != 0) return ret;

    // Process parsed fields based on s_info->status
    if (s_info->status == 1) { // Initial message, first part
        if (current_part_num != 1 || total_parts_num == 0) {
            return ERR_PARSE_FAIL;
        }
        s_info->total_parts = total_parts_num;
        s_info->current_part = 1; // Set to 1 as it's the first part
        s_info->session_id = session_id_val;
    } else { // Subsequent message parts
        if (s_info->status != 2) { // Expected status is 2 for subsequent parts
            return ERR_PARSE_FAIL;
        }
        // Check if message sequence and IDs match
        if ((s_info->current_part + 1U != current_part_num) ||
            (s_info->total_parts != total_parts_num) ||
            (s_info->session_id != session_id_val) ||
            (total_parts_num < current_part_num)) {
            return ERR_PARSE_FAIL;
        }
        s_info->current_part++; // Increment current part
    }

    // Advance to next field (message content field)
    current_field_ptr = get_next_field(current_field_ptr);
    if (current_field_ptr == NULL) return ERR_PARSE_FAIL;

    // 7. Copy message content field
    char *field_end_ptr = get_next_field(current_field_ptr);
    int field_len = 0;
    if (field_end_ptr != NULL) {
        field_len = (int)(field_end_ptr - 1 - current_field_ptr);
    } else {
        // If it's the last field and no '*' is found, copy until '\0'
        field_len = (int)strlen(current_field_ptr);
    }
    
    // Ensure field_len does not exceed buffer size
    if (field_len >= sizeof(s_info->data_buffer)) {
        field_len = sizeof(s_info->data_buffer) - 1; // Leave space for null terminator
    }

    s_info->data_ptr = fieldncpy(current_field_ptr, s_info->data_buffer, field_len);

    // 8. Get message type (only for initial message)
    if (s_info->status == 1) {
        s_info->message_type = get_msg_type(current_field_ptr);
        if (!((s_info->message_type == 1) || (s_info->message_type == 4) || (s_info->message_type == 5))) {
            return ERR_PARSE_FAIL;
        }
    }

    // 9. Update sentence status
    if (s_info->total_parts == s_info->current_part) {
        s_info->status = 3; // Complete
    } else {
        s_info->status = 2; // In progress
    }

    return 0; // Success
}

// Function: reset_sentence_struct
void reset_sentence_struct(SentenceInfo *s_info) {
    if (s_info == NULL) return;

    s_info->current_part = 0;
    s_info->total_parts = 0;
    s_info->session_id = 0;
    s_info->status = 1; // Initial state
    s_info->message_type = 0;
    memset(s_info->data_buffer, 0, sizeof(s_info->data_buffer));
    s_info->data_ptr = s_info->data_buffer; // data_ptr points to the start of the buffer
    return;
}