#include <stdlib.h> // For malloc, free, strtoul
#include <string.h> // For memcmp, memcpy
#include <stdio.h>  // For sprintf, snprintf, NULL
#include <stdint.h> // For uint8_t, UINT_MAX
#include <limits.h> // For SIZE_MAX

// Define the structure for the parsed data
typedef struct pkk_data {
    unsigned int height;
    unsigned int width;
    uint8_t *data; // Assuming 3 bytes per pixel (RGB), so uint8_t is appropriate
} pkk_data_t;

// Global constant for "PK\n" header
static const char PK_HEADER[] = "PK\n";

// Function: skip_whitespace
// Advances 'buffer' past whitespace characters and updates the remaining length.
// Returns the advanced pointer.
char *skip_whitespace(char *buffer, size_t *len_ptr) {
    if (buffer == NULL || len_ptr == NULL) {
        return buffer;
    }
    size_t current_len = *len_ptr;
    while (current_len > 0 && (*buffer == '\n' || *buffer == '\r' || *buffer == ' ' || *buffer == '\t')) {
        buffer++;
        current_len--;
    }
    *len_ptr = current_len; // Update the remaining length
    return buffer;
}

// Function: free_pkk
// Frees memory associated with a pkk_data_t object.
void free_pkk(pkk_data_t *pkk_obj) {
    if (pkk_obj != NULL) {
        if (pkk_obj->data != NULL) {
            free(pkk_obj->data);
        }
        free(pkk_obj);
    }
}

// Function: parse_pkk
// Parses a buffer containing PKK format data into a pkk_data_t structure.
// param_1: Pointer to the input buffer.
// param_2: Length of the input buffer.
// Returns a pointer to a newly allocated pkk_data_t structure, or NULL on failure.
pkk_data_t *parse_pkk(char *param_1, int param_2) {
    pkk_data_t *pkk_obj = NULL;
    char *current_pos = param_1;
    char *endptr; // Used by strtoul to indicate where parsing stopped
    size_t len = (size_t)param_2; // Use size_t for length for safety
    char *buffer_end = param_1 + len; // Calculate buffer end once

    if (param_1 == NULL || param_2 <= 0) {
        return NULL;
    }

    // Allocate the main structure
    pkk_obj = (pkk_data_t *)malloc(sizeof(pkk_data_t));
    if (pkk_obj == NULL) {
        return NULL;
    }
    // Initialize to safe defaults for partial failure cleanup
    pkk_obj->height = 0;
    pkk_obj->width = 0;
    pkk_obj->data = NULL;

    // 1. Check for PK_HEADER
    if (len < sizeof(PK_HEADER) - 1 || memcmp(current_pos, PK_HEADER, sizeof(PK_HEADER) - 1) != 0) {
        free_pkk(pkk_obj);
        return NULL;
    }
    current_pos += sizeof(PK_HEADER) - 1; // Advance past "PK\n"
    len = buffer_end - current_pos; // Update remaining length

    // 2. Skip any whitespace or comments after header
    current_pos = skip_whitespace(current_pos, &len);
    while (len > 0 && *current_pos == '#') {
        while (len > 0 && *current_pos != '\n') { // Skip until newline
            current_pos++;
            len--;
        }
        if (len > 0 && *current_pos == '\n') { // Skip the newline itself
            current_pos++;
            len--;
        }
        current_pos = skip_whitespace(current_pos, &len); // Skip whitespace after comment
    }

    // 3. Read width
    if (len == 0) { free_pkk(pkk_obj); return NULL; }
    unsigned long width_val = strtoul(current_pos, &endptr, 10);
    if (endptr == current_pos || endptr > buffer_end || width_val > UINT_MAX) { // No number read, endptr out of bounds, or value overflow
        free_pkk(pkk_obj);
        return NULL;
    }
    pkk_obj->width = (unsigned int)width_val;
    current_pos = endptr;
    len = buffer_end - current_pos; // Update remaining length

    // 4. Read height
    current_pos = skip_whitespace(current_pos, &len);
    if (len == 0) { free_pkk(pkk_obj); return NULL; }
    unsigned long height_val = strtoul(current_pos, &endptr, 10);
    if (endptr == current_pos || endptr > buffer_end || height_val > UINT_MAX) { // No number read, endptr out of bounds, or value overflow
        free_pkk(pkk_obj);
        return NULL;
    }
    pkk_obj->height = (unsigned int)height_val;
    current_pos = endptr;
    len = buffer_end - current_pos; // Update remaining length

    // 5. Read format (expect 255)
    current_pos = skip_whitespace(current_pos, &len);
    if (len == 0) { free_pkk(pkk_obj); return NULL; }
    unsigned long format_val = strtoul(current_pos, &endptr, 10);
    if (endptr == current_pos || endptr > buffer_end || format_val != 255) { // No number read, endptr out of bounds, or not 255
        free_pkk(pkk_obj);
        return NULL;
    }
    current_pos = endptr;
    len = buffer_end - current_pos; // Update remaining length

    // 6. Skip any trailing whitespace/newline before data
    current_pos = skip_whitespace(current_pos, &len);

    // 7. Calculate data size and allocate memory
    size_t total_pixels = 0;
    if (pkk_obj->width > 0 && pkk_obj->height > 0) {
        // Check for total_pixels overflow (width * height)
        if (pkk_obj->height > SIZE_MAX / pkk_obj->width) {
            free_pkk(pkk_obj);
            return NULL;
        }
        total_pixels = (size_t)pkk_obj->width * pkk_obj->height;
    }

    size_t data_size = 0; // size of pixel data in bytes
    if (total_pixels > 0) {
        // Check for data_size overflow (total_pixels * 3)
        if (total_pixels > SIZE_MAX / 3) {
            free_pkk(pkk_obj);
            return NULL;
        }
        data_size = total_pixels * 3;
    }

    if (data_size > len) { // Not enough data in buffer for all pixels
        free_pkk(pkk_obj);
        return NULL;
    }

    if (data_size > 0) {
        pkk_obj->data = (uint8_t *)malloc(data_size);
        if (pkk_obj->data == NULL) {
            free_pkk(pkk_obj);
            return NULL;
        }
        memcpy(pkk_obj->data, current_pos, data_size);
    } // If data_size is 0, pkk_obj->data remains NULL, which is fine.

    return pkk_obj;
}

// Function: output_pkk
// Generates a string representation of pkk_data_t in PKK format.
// param_1: Pointer to the pkk_data_t structure to output.
// param_2: Pointer to a size_t variable that will store the length of the output string.
// Returns a newly allocated string, or NULL on failure.
char *output_pkk(const pkk_data_t *param_1, int *param_2) {
    if (param_1 == NULL || param_2 == NULL) {
        if (param_2 != NULL) *param_2 = 0;
        return NULL;
    }
    *param_2 = 0; // Initialize output_len to 0 in case of error

    // Calculate required size for header part: "PK\n" + "width height\n" + "255\n"
    int header_len_chars = snprintf(NULL, 0, "PK\n%u %u\n255\n", param_1->width, param_1->height);
    if (header_len_chars < 0) { // Error in snprintf
        return NULL;
    }

    size_t total_pixels = 0;
    if (param_1->width > 0 && param_1->height > 0) {
        // Check for total_pixels overflow (width * height)
        if (param_1->height > SIZE_MAX / param_1->width) {
            return NULL;
        }
        total_pixels = (size_t)param_1->width * param_1->height;
    }

    size_t data_size = 0; // size of pixel data in bytes
    if (total_pixels > 0) {
        // Check for data_size overflow (total_pixels * 3)
        if (total_pixels > SIZE_MAX / 3) {
            return NULL;
        }
        data_size = total_pixels * 3;
    }
    
    // Check for total_output_size overflow
    size_t total_output_size;
    if ((size_t)header_len_chars > SIZE_MAX - data_size) {
        return NULL;
    }
    total_output_size = (size_t)header_len_chars + data_size;

    char *output_buffer = (char *)malloc(total_output_size + 1); // +1 for null terminator
    if (output_buffer == NULL) {
        return NULL;
    }

    // Write header part
    int written_chars = sprintf(output_buffer, "PK\n%u %u\n255\n", param_1->width, param_1->height);
    if (written_chars < 0 || (size_t)written_chars != (size_t)header_len_chars) {
        free(output_buffer);
        return NULL;
    }

    char *current_pos = output_buffer + written_chars;

    // Write pixel data part
    if (data_size > 0 && param_1->data != NULL) {
        memcpy(current_pos, param_1->data, data_size);
    }
    
    output_buffer[total_output_size] = '\0'; // Null terminate the entire buffer

    *param_2 = (int)total_output_size; // Store the actual length in the output parameter
    return output_buffer;
}