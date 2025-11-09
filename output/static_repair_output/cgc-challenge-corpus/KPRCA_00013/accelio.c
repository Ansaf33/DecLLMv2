#include <string.h> // For strlen, strchr, memcpy, memmove
#include <stdlib.h> // For calloc, free, size_t
#include <ctype.h>  // For toupper, isdigit

// Function: strtrim
int strtrim(char *param_1, unsigned int param_2, int param_3) {
    size_t current_len = strlen(param_1);
    size_t required_len_with_null = current_len + 1;

    // Check if the destination buffer size (param_2) is sufficient for the original string
    if (param_2 < required_len_with_null) {
        return -1; // Destination buffer too small
    }
    // If the string is empty or only a null terminator, nothing to trim
    if (current_len == 0) {
        return 0;
    }

    // Trim from left (param_3 == 1 or param_3 == 3)
    if ((param_3 & 1) != 0) {
        char *read_ptr = param_1;
        while (*read_ptr != '\0' && strchr("\r\n\t ", *read_ptr) != NULL) {
            read_ptr++;
        }
        if (read_ptr != param_1) { // If leading whitespace was found
            size_t trimmed_len = strlen(read_ptr);
            memmove(param_1, read_ptr, trimmed_len + 1); // Move string including null terminator
            current_len = trimmed_len; // Update current length after left trim
        }
    }

    // Trim from right (param_3 == 2 or param_3 == 3)
    if ((param_3 & 2) != 0) {
        if (current_len == 0) { // If string became empty after left trim
            return 0;
        }
        int end_idx = (int)current_len - 1;
        while (end_idx >= 0 && strchr("\r\n\t ", param_1[end_idx]) != NULL) {
            end_idx--;
        }
        param_1[end_idx + 1] = '\0'; // Null-terminate at the new end
    }

    return 0;
}

// Function: valid_cell_id
int valid_cell_id(const char *param_1) {
    size_t len = strlen(param_1);

    // Cell ID must be at least 2 characters ("A1", "AB") and at most 4 characters ("AB12", "A12")
    // The original loop checks indices up to param_1[4], implying max 5 chars,
    // but the `digit_count` logic limits valid length to 4.
    if (len < 2 || len > 4) {
        return -1;
    }

    char c1 = toupper(param_1[0]);
    if (c1 < 'A' || c1 > 'Z') {
        return -1; // First character must be an uppercase letter
    }

    int digit_count = 0;
    int current_idx = 1; // Start checking from the second character

    char c_curr = toupper(param_1[current_idx]);
    if (c_curr >= 'A' && c_curr <= 'Z') {
        // Second character is a letter (e.g., "AB...")
        current_idx++;
    } else if (c_curr >= '0' && c_curr <= '9') {
        // Second character is a digit (e.g., "A1...")
        digit_count++;
        current_idx++;
    } else {
        return -1; // Invalid second character (not letter or digit)
    }

    // Check remaining characters (param_1[2], param_1[3], up to param_1[4] in original)
    // The loop iterates up to `len` (exclusive), checking `param_1[current_idx]`
    for (; current_idx < len; current_idx++) {
        if (!isdigit(param_1[current_idx])) {
            return -1; // Subsequent characters must be digits
        }
        if (digit_count >= 2) {
            return -1; // Maximum of 2 digits allowed for the suffix
        }
        digit_count++;
    }

    // If the entire string was processed and conditions met, it's valid if at least one digit was found.
    // This handles "A1", "AB1", "A12", "AB12" as valid.
    // And "AA", "AB" as invalid (digit_count would be 0).
    return (digit_count > 0) ? 0 : -1;
}

// Function: get_rowcol
int get_rowcol(const char *param_1, char *param_2, char *param_3, char param_4) {
    size_t len = strlen(param_1);

    // Similar length constraints as valid_cell_id
    if (len < 2 || len > 4) {
        return -1;
    }

    char c1 = toupper(param_1[0]);
    if (c1 < 'A' || c1 > 'Z') {
        return -1; // First character must be an uppercase letter
    }
    param_2[0] = c1; // Store first char of row

    int digit_count = 0;
    int current_idx = 1; // Start checking from the second character

    char c_curr = toupper(param_1[current_idx]);
    if (c_curr >= 'A' && c_curr <= 'Z') {
        // Second character is a letter (e.g., "AB...")
        param_2[1] = c_curr; // Store second char of row
        param_2[2] = '\0';   // Null-terminate row string
        param_3[0] = '\0';   // Ensure column string is empty initially
        current_idx++;
    } else if (c_curr >= '0' && c_curr <= '9') {
        // Second character is a digit (e.g., "A1...")
        param_2[1] = '\0';   // Null-terminate row string (only one char)
        param_3[0] = c_curr; // Store first char of column
        digit_count++;
        current_idx++;
    } else {
        return -1; // Invalid second character
    }

    // Loop for remaining characters (param_1[2], param_1[3], up to param_1[4] in original)
    // `param_4` is effectively `\0` based on the original comparison logic.
    for (; current_idx < len; current_idx++) {
        // Original code checks `param_1[local_10] == param_4` (i.e. '\0')
        // This means if the string terminates early, and we have digits, it's valid.
        if (param_1[current_idx] == param_4) {
            if (digit_count > 0) {
                param_3[digit_count] = '\0'; // Null-terminate column string
                return 0; // Valid, string ended with digits
            } else {
                return -1; // Invalid, string ended without digits
            }
        }

        if (!isdigit(param_1[current_idx])) {
            return -1; // Must be a digit
        }
        if (digit_count >= 2) {
            return -1; // Max 2 digits total for the suffix
        }
        param_3[digit_count] = param_1[current_idx]; // Store digit in column string
        digit_count++;
    }

    // If the loop completes (string processed up to `len`),
    // it's valid if at least one digit was found.
    return (digit_count > 0) ? 0 : -1;
}

// Function: sanitize_formula
int sanitize_formula(char *param_1, unsigned int max_len) {
    size_t original_len = strlen(param_1);

    // If the destination buffer (param_1) is smaller than the original string,
    // it cannot reliably hold the sanitized string.
    if (max_len < original_len) {
        return -1;
    }

    // Allocate a temporary buffer to build the sanitized string.
    // `calloc` initializes memory to zero, which ensures null termination.
    char *temp_buffer = (char *)calloc(1, max_len);
    if (temp_buffer == NULL) {
        return -1; // Memory allocation failed
    }

    int write_idx = 0;
    for (size_t read_idx = 0; read_idx < original_len; ++read_idx) {
        // If the character is NOT one of "\r\n\t ", copy it to temp_buffer.
        if (strchr("\r\n\t ", param_1[read_idx]) == NULL) {
            // Ensure we don't write beyond the allocated `max_len - 1` for the null terminator.
            if (write_idx < max_len - 1) {
                temp_buffer[write_idx++] = param_1[read_idx];
            } else {
                // Buffer is full, cannot copy more characters. Truncate.
                break;
            }
        }
    }
    // Explicitly null-terminate the sanitized string (redundant if `calloc` was used and `write_idx < max_len`).
    temp_buffer[write_idx] = '\0';

    // Copy the sanitized string back to param_1.
    // The original copies `max_len` bytes, so we follow that behavior.
    // This assumes param_1 points to a buffer of at least `max_len` size.
    memcpy(param_1, temp_buffer, max_len);
    free(temp_buffer);

    return 0;
}