#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h> // For boolean types, though not strictly used in current logic

// Assuming a 32-bit architecture for original pointer arithmetic,
// but defining a portable struct for clarity and correctness.
// This struct represents the managed buffer state.
typedef struct {
    void* data;
    uint16_t size;   // Current total data size in bytes
    uint16_t offset; // Current read/write offset within the data
} ManagedBuffer;

// Error codes, assuming 0xffffffe0 is a success-like code
// and other negative values are errors.
// These specific values might indicate WinAPI status codes or similar.
#define STATUS_SUCCESS       0xffffffe0 // -32 (signed int)
#define STATUS_INVALID_PARAM 0xffffffe2 // -30
#define STATUS_NOT_FOUND     0xffffffe3 // -29
#define STATUS_BAD_PATH      0xffffffe4 // -28
#define STATUS_UNKNOWN_TYPE  0xffffffe5 // -27
#define STATUS_CANON_FAIL    0xffffffe6 // -26
#define STATUS_ERROR         0xffffffff // -1

// Path character constants, assuming these are specific byte values used as delimiters.
// If they are actual ASCII characters, they should be ':', '/', etc.
// Based on the code, they are likely specific byte values.
#define PATH_DELIM_SERVICE_TYPE 0xD2 // Used in FileStat, NetPathType, CanonicalizePathName (for '.')
#define PATH_DELIM_PART         0xD0 // Used in FileStat (for '..')
#define PATH_DELIM_SLASH        0xD1 // Used in ConvertPathMacros (for '/')
#define PATH_MACRO_DOT          0xD2 // -0x2e in original, interpreted as 0xD2
#define PATH_MACRO_DOUBLE_DOT   0xD0 // -0x30 in original, interpreted as 0xD0

// Global variable 'serviceTypes' used in ServiceEnum and NetPathType
// The size 0x40 (64) suggests fixed-size strings.
static char serviceTypes[5][0x40] = {
    "TypeA", "TypeB", "TypeC", "TypeD", "TypeE"
};

// Forward declarations for functions that call each other
// and might not be defined in order.
// Mock declarations for external functions used in the snippet
// Assuming they return an integer handle or pointer to a data structure.
// The return value 0 indicates failure/not found.
static int FindTreeByPath(int tree_id, const char *path_part, const char *service_type);
static int FindFileByName(int tree_handle, const char *file_name);

static int Length(const char *str);
static char *BufCpy(char *dest, const char *src);
static void AddData(ManagedBuffer *buffer, void *data_to_add, uint16_t data_size);
static uint32_t ReadData(ManagedBuffer *buffer, void *output_buffer, uint16_t bytes_to_read);
static intptr_t FindChar(const char *buffer, uint16_t buffer_len, char char_to_find);
static uint32_t BufCat(char *dest, const char *src);
static int BufCmp(const uint8_t *s1, const uint8_t *s2);
static uint32_t ServiceEnum(uint32_t param_1_unused, uint32_t param_2_unused, ManagedBuffer *output_buffer);
static uint32_t FileStat(const void *path_ptr, int16_t path_len, ManagedBuffer *output_buffer);
static uint32_t NetPathCanonicalize(uint32_t path_ptr_as_int, uint16_t path_len, ManagedBuffer *output_buffer);
static int NetPathType(const void *path_ptr, uint16_t path_len);
static uint32_t CanonicalizePathName(const void *path_ptr, uint32_t path_len, uint8_t *output_buf, uint16_t *output_len);
static uint32_t ConvertPathMacros(char *path);


// Function: AddData
void AddData(ManagedBuffer *buffer, void *data_to_add, uint16_t data_size) {
    if (buffer == NULL || data_to_add == NULL) {
        return;
    }

    if (buffer->data == NULL) {
        // Initial allocation
        buffer->data = calloc(data_size, 1);
        if (buffer->data == NULL) {
            // Handle allocation failure if necessary
            return;
        }
        memcpy(buffer->data, data_to_add, data_size);
        buffer->size = data_size;
        buffer->offset = 0; // Initialize offset
    } else {
        // Reallocation and append
        void *old_data = buffer->data;
        size_t new_total_size = (size_t)buffer->size + data_size;
        buffer->data = calloc(new_total_size, 1);
        if (buffer->data == NULL) {
            // Handle allocation failure, restore old_data, free, etc.
            free(old_data); // Free old data to prevent leak if realloc fails
            return;
        }
        memcpy(buffer->data, old_data, buffer->size);
        memcpy((uint8_t*)buffer->data + buffer->size, data_to_add, data_size);
        buffer->size = (uint16_t)new_total_size; // Cast back to uint16_t, assuming it won't overflow
        free(old_data);
    }
}

// Function: ReadData
uint32_t ReadData(ManagedBuffer *buffer, void *output_buffer, uint16_t bytes_to_read) {
    if (buffer == NULL || buffer->data == NULL || output_buffer == NULL) {
        return STATUS_ERROR; // 0xffffffff
    }

    if ((uint32_t)buffer->size < (uint32_t)buffer->offset + bytes_to_read) {
        return STATUS_ERROR; // Not enough data to read
    }

    memcpy(output_buffer, (uint8_t*)buffer->data + buffer->offset, bytes_to_read);
    buffer->offset += bytes_to_read;
    return STATUS_SUCCESS; // 0
}

// Function: NETSTUFF_Handler
void NETSTUFF_Handler(uint16_t param_1, uint32_t param_2, uint16_t param_3,
                     uint32_t *out_data_ptr, uint16_t *out_size) {
    ManagedBuffer result_buffer = { .data = NULL, .size = 0, .offset = 0 };
    uint16_t status_code = 0xE1; // Default error code

    // The original logic with goto is replaced by a clearer if/else if chain.
    if (param_1 > 0xB5 && param_1 < 0xBD) { // 0xB6 to 0xBC
        status_code = (uint16_t)NetPathCanonicalize(param_2, param_3, &result_buffer);
    } else if (param_1 > 0xB2 && param_1 < 0xB6) { // 0xB3 to 0xB5
        status_code = (uint16_t)FileStat((void*)(uintptr_t)param_2, (int16_t)param_3, &result_buffer);
    } else if (param_1 == 0xB2) {
        status_code = (uint16_t)ServiceEnum(param_2, param_3, &result_buffer);
    } else if (param_1 > 0xAF && param_1 < 0xB2) { // 0xB0 to 0xB1
        // This range also maps to FileStat in the original logic.
        status_code = (uint16_t)FileStat((void*)(uintptr_t)param_2, (int16_t)param_3, &result_buffer);
    }
    // For other values, status_code remains 0xE1

    // Common exit code: Add the final status code to the buffer.
    AddData(&result_buffer, &status_code, sizeof(status_code));

    // Store the data pointer and size in the output parameters.
    // Assuming param_4_data_ptr receives the pointer cast to a uint32_t.
    // This implies a 32-bit pointer or truncation on 64-bit.
    // For Linux compilable C code, it's safer to use uintptr_t for pointer-to-integer conversion.
    *out_data_ptr = (uint32_t)(uintptr_t)result_buffer.data;
    *out_size = result_buffer.size;

    // The memory pointed to by result_buffer.data is now "owned" by the caller.
    // The caller is responsible for freeing it.
}

// Function: FindChar
intptr_t FindChar(const char *buffer, uint16_t buffer_len, char char_to_find) {
    if (buffer == NULL) {
        return 0; // NULL
    }
    for (uint16_t i = 0; i < buffer_len; ++i) {
        if (char_to_find == buffer[i]) {
            return (intptr_t)(buffer + i); // Return pointer to the char as an integer address
        }
    }
    return 0; // NULL
}

// Function: BufCat
uint32_t BufCat(char *dest, const char *src) {
    if (dest == NULL || src == NULL) {
        return STATUS_ERROR;
    }
    while (*dest != '\0') {
        dest++;
    }
    while (*src != '\0') {
        *dest++ = *src++;
    }
    *dest = '\0'; // Ensure null termination
    return STATUS_SUCCESS;
}

// Function: Length
int Length(const char *str) {
    if (str == NULL) {
        return 0;
    }
    int len = 0;
    while (*str != '\0') {
        len++;
        str++;
    }
    return len;
}

// Function: BufCmp
int BufCmp(const uint8_t *s1, const uint8_t *s2) {
    if (s1 == NULL && s2 == NULL) {
        return 0;
    }
    if (s1 == NULL) { // s1 is NULL, s2 is not
        return -1; // s1 < s2
    }
    if (s2 == NULL) { // s2 is NULL, s1 is not
        return 1; // s1 > s2
    }

    while (*s1 != 0 && *s2 != 0) {
        if (*s1 < *s2) {
            return -1;
        }
        if (*s1 > *s2) {
            return 1;
        }
        s1++;
        s2++;
    }

    // One or both strings reached null terminator
    if (*s1 == 0 && *s2 == 0) {
        return 0; // Both are equal
    }
    if (*s1 == 0) { // s1 ended, s2 continues
        return -1; // s1 < s2
    }
    // s2 ended, s1 continues
    return 1; // s1 > s2
}

// Function: BufCpy
char * BufCpy(char *dest, const char *src) {
    char *original_dest = dest;
    if (dest == NULL || src == NULL) {
        return NULL;
    }
    while (*src != '\0') {
        *dest++ = *src++;
    }
    *dest = '\0';
    return original_dest;
}

// Function: ServiceEnum
uint32_t ServiceEnum(uint32_t param_1_unused, uint32_t param_2_unused, ManagedBuffer *output_buffer) {
    // param_1 and param_2 are unused in the original code.
    // They are likely placeholders or part of a larger context not provided.

    if (output_buffer == NULL) {
        return STATUS_ERROR;
    }

    uint16_t num_services = 5; // Fixed number of services
    AddData(output_buffer, &num_services, sizeof(num_services));

    for (uint16_t i = 0; i < num_services; ++i) {
        size_t len = strlen(serviceTypes[i]);
        AddData(output_buffer, serviceTypes[i], (uint16_t)(len + 1)); // +1 for null terminator
    }
    return STATUS_SUCCESS;
}

// Function: FileStat
uint32_t FileStat(const void *path_ptr, int16_t path_len, ManagedBuffer *output_buffer) {
    const char *path = (const char *)path_ptr;
    uint32_t status = STATUS_INVALID_PARAM;

    if (path == NULL || output_buffer == NULL) {
        return status;
    }

    // Find first delimiter (PATH_DELIM_SERVICE_TYPE, 0xD2)
    intptr_t split_pos1_addr = FindChar(path, (uint16_t)path_len, PATH_DELIM_SERVICE_TYPE);
    if (split_pos1_addr == 0) { // Not found
        return STATUS_INVALID_PARAM;
    }

    size_t part1_len = (size_t)split_pos1_addr - (size_t)path;
    char *part1_buf = calloc(part1_len + 1, 1);
    if (part1_buf == NULL) return STATUS_ERROR;
    memcpy(part1_buf, path, part1_len);
    part1_buf[part1_len] = '\0';

    const char *remaining_path_ptr = (const char *)(split_pos1_addr + 1);
    uint16_t remaining_path_len = path_len - (uint16_t)(part1_len + 1);

    // Find second delimiter (PATH_DELIM_PART, 0xD0) in the remaining path
    intptr_t split_pos2_addr = FindChar(remaining_path_ptr, remaining_path_len, PATH_DELIM_PART);
    if (split_pos2_addr == 0) { // Not found
        free(part1_buf);
        return STATUS_INVALID_PARAM;
    }

    size_t part2_len = (size_t)split_pos2_addr - (size_t)remaining_path_ptr;
    char *part2_buf = calloc(part2_len + 1, 1);
    if (part2_buf == NULL) {
        free(part1_buf);
        return STATUS_ERROR;
    }
    memcpy(part2_buf, remaining_path_ptr, part2_len);
    part2_buf[part2_len] = '\0';

    const char *part3_name_ptr = (const char *)(split_pos2_addr + 1);
    uint16_t part3_len = remaining_path_len - (uint16_t)(part2_len + 1);
    char *part3_buf = calloc(part3_len + 1, 1);
    if (part3_buf == NULL) {
        free(part1_buf);
        free(part2_buf);
        return STATUS_ERROR;
    }
    memcpy(part3_buf, part3_name_ptr, part3_len);
    part3_buf[part3_len] = '\0';

    // Original call: FindTreeByPath(0,__dest,pvVar4);
    // __dest was part2_buf, pvVar4 was part1_buf.
    int tree_handle = FindTreeByPath(0, part2_buf, part1_buf);
    if (tree_handle == 0) {
        status = STATUS_NOT_FOUND;
    } else {
        int file_info_ptr = FindFileByName(tree_handle, part3_buf);
        if (file_info_ptr == 0) {
            status = STATUS_NOT_FOUND;
        } else {
            // Assuming file_info_ptr points to a struct or data block
            // and the offsets 0x82, 0x84 are valid within that block.
            // These are likely uint16_t values (size, attributes, etc.)
            AddData(output_buffer, (void*)(uintptr_t)file_info_ptr, sizeof(uint16_t));
            AddData(output_buffer, (void*)(uintptr_t)(file_info_ptr + 0x82), sizeof(uint16_t));
            AddData(output_buffer, (void*)(uintptr_t)(file_info_ptr + 0x84), sizeof(uint8_t));
            status = STATUS_SUCCESS;
        }
    }

    free(part1_buf);
    free(part2_buf);
    free(part3_buf);
    return status;
}

// Function: NetPathCanonicalize
uint32_t NetPathCanonicalize(uint32_t path_ptr_as_int, uint16_t path_len, ManagedBuffer *output_buffer) {
    if (path_len >= 0x1ad || output_buffer == NULL) { // 0x1AD = 429 max path length
        return STATUS_INVALID_PARAM;
    }

    const char *path = (const char*)(uintptr_t)path_ptr_as_int;
    int path_type = NetPathType(path, path_len);
    if (path_type == -1) {
        return STATUS_UNKNOWN_TYPE;
    }

    char canonical_path_buf[428]; // Max size for local_1b9 in original
    uint16_t canonical_path_len;  // local_1bc in original

    uint32_t canonicalize_status = CanonicalizePathName(path, path_len, (uint8_t*)canonical_path_buf, &canonical_path_len);

    if (canonicalize_status == STATUS_SUCCESS) {
        AddData(output_buffer, canonical_path_buf, canonical_path_len);
        return STATUS_SUCCESS;
    } else {
        return canonicalize_status; // Return the specific error from CanonicalizePathName
    }
}

// Function: NetPathType
int NetPathType(const void *path_ptr, uint16_t path_len) {
    const char *path = (const char *)path_ptr;
    if (path == NULL) return -1;

    // Find the first delimiter (PATH_DELIM_SERVICE_TYPE, 0xD2)
    intptr_t split_pos_addr = FindChar(path, path_len, PATH_DELIM_SERVICE_TYPE);
    if (split_pos_addr == 0) { // Delimiter not found
        return -1;
    }

    size_t service_type_len = (size_t)split_pos_addr - (size_t)path;
    char *service_type_buf = calloc(service_type_len + 1, 1);
    if (service_type_buf == NULL) {
        return -1; // Allocation failure
    }
    memcpy(service_type_buf, path, service_type_len);
    service_type_buf[service_type_len] = '\0';

    for (int i = 0; i < 5; ++i) {
        if (BufCmp((const uint8_t*)service_type_buf, (const uint8_t*)serviceTypes[i]) == 0) {
            free(service_type_buf);
            return i;
        }
    }

    free(service_type_buf);
    return -1;
}

// Function: CanonicalizePathName
uint32_t CanonicalizePathName(const void *path_ptr, uint32_t path_len,
                              uint8_t *output_buf, uint16_t *output_len) {
    const char *path = (const char *)path_ptr;
    char temp_canonical_path[428];
    memset(temp_canonical_path, 0, sizeof(temp_canonical_path));

    if (path_len >= 0x1ad || output_buf == NULL || output_len == NULL) {
        if (output_buf != NULL) *output_buf = '\0';
        if (output_len != NULL) *output_len = 0;
        return STATUS_INVALID_PARAM;
    }

    char *mutable_path_copy = calloc(path_len + 1, 1);
    if (mutable_path_copy == NULL) {
        *output_buf = '\0';
        *output_len = 0;
        return STATUS_ERROR;
    }
    memcpy(mutable_path_copy, path, path_len);
    mutable_path_copy[path_len] = '\0';

    char *service_type_buf = NULL;
    intptr_t first_delimiter_addr = FindChar(mutable_path_copy, (uint16_t)path_len, PATH_DELIM_SERVICE_TYPE);

    if (first_delimiter_addr != 0) { // If first delimiter (0xD2) is found
        size_t service_type_len = (size_t)first_delimiter_addr - (size_t)mutable_path_copy;
        service_type_buf = calloc(service_type_len + 1, 1);
        if (service_type_buf == NULL) {
            free(mutable_path_copy);
            *output_buf = '\0';
            *output_len = 0;
            return STATUS_ERROR;
        }
        memcpy(service_type_buf, mutable_path_copy, service_type_len);
        service_type_buf[service_type_len] = '\0';
        BufCat(temp_canonical_path, service_type_buf);

        // This is the part of the path *after* the first 0xD2
        char *path_after_first_delimiter = (char *)(first_delimiter_addr + 1);
        uint16_t len_after_first_delimiter = path_len - (uint16_t)(service_type_len + 1);

        // Now search for a second 0xD2 *in the remaining part*
        intptr_t second_delimiter_addr = FindChar(path_after_first_delimiter, len_after_first_delimiter, PATH_DELIM_SERVICE_TYPE);

        if (second_delimiter_addr == 0) { // No second 0xD2 found
            // Modify the remaining path part: replace -0x2d with 0xD0 (PATH_DELIM_PART)
            // The original code uses -0x2d (211) to 0xd0 (208).
            for (uint16_t i = 0; i < len_after_first_delimiter; ++i) {
                if (path_after_first_delimiter[i] == (char)0xD3) { // -0x2d is 0xD3
                    path_after_first_delimiter[i] = (char)PATH_DELIM_PART; // 0xD0
                }
            }
            BufCat(temp_canonical_path, path_after_first_delimiter);

            uint32_t convert_status = ConvertPathMacros(temp_canonical_path);
            if (convert_status == STATUS_SUCCESS) {
                *output_len = (uint16_t)Length(temp_canonical_path);
                memcpy(output_buf, temp_canonical_path, *output_len);
                output_buf[*output_len] = '\0';
                free(service_type_buf);
                free(mutable_path_copy);
                return STATUS_SUCCESS;
            }
        }
        // If second_delimiter_addr != 0, it falls through to error.
    }

    free(service_type_buf); // free if allocated
    free(mutable_path_copy);
    temp_canonical_path[0] = '\0';
    *output_buf = '\0';
    *output_len = 0;
    return STATUS_BAD_PATH;
}

// Function: ConvertPathMacros
uint32_t ConvertPathMacros(char *path) {
    if (path == NULL) return STATUS_BAD_PATH;

    char *dot_ptr = NULL;        // Tracks the position of the last single dot (PATH_MACRO_DOT)
    char *current_ptr = path;    // Current position being scanned
    char *macro_segment_start_ptr = NULL; // Where a macro operation began for replacement

    while (*current_ptr != '\0') {
        char current_char = *current_ptr;
        char next_char = *(current_ptr + 1);

        if (dot_ptr == NULL && current_char == PATH_MACRO_DOT) {
            dot_ptr = current_ptr; // Found a single dot
        } else if (current_char == PATH_MACRO_DOUBLE_DOT) { // Found '..'
            if (next_char == PATH_MACRO_DOUBLE_DOT) { // Found '...' -> error
                return STATUS_BAD_PATH;
            }
            if (next_char == PATH_DELIM_SLASH) { // Found "../"
                uint8_t leading_slashes_count = 0; // Reset for each "../" processing
                char *segment_start_after_slashes = current_ptr + 1; // Points to the first '/'

                // Count leading slashes
                while (*segment_start_after_slashes == PATH_DELIM_SLASH) {
                    leading_slashes_count++;
                    segment_start_after_slashes++;
                }

                if (*segment_start_after_slashes == '\0') { // Path ends with "../"
                    return STATUS_BAD_PATH;
                }

                // Find the end of the segment after "../"
                char *segment_end_ptr = segment_start_after_slashes;
                uint8_t segment_len = 0;
                while (*segment_end_ptr != PATH_MACRO_DOUBLE_DOT && *segment_end_ptr != '\0') {
                    segment_len++;
                    segment_end_ptr++;
                }

                // Original logic for handling ".."
                if (macro_segment_start_ptr == NULL) { // First macro segment
                    if (leading_slashes_count == 1) { // Normal "./segment" case
                        macro_segment_start_ptr = dot_ptr; // If there was a '.', replace from there
                        if (dot_ptr != NULL) {
                            BufCpy(dot_ptr + 1, segment_end_ptr);
                        } else { // No dot, replace from current '..' position
                            BufCpy(current_ptr, segment_end_ptr);
                        }
                    } else if (leading_slashes_count == segment_len) { // "////segment" matching length
                        macro_segment_start_ptr = current_ptr - (leading_slashes_count * 2); // Rewind by 2*slashes
                        if (macro_segment_start_ptr < path) macro_segment_start_ptr = path; // Bounds check
                        BufCpy(macro_segment_start_ptr, segment_end_ptr);
                    } else { // Fallback, same as leading_slashes_count == 1
                        macro_segment_start_ptr = dot_ptr;
                        if (dot_ptr != NULL) {
                            BufCpy(dot_ptr + 1, segment_end_ptr);
                        } else {
                            BufCpy(current_ptr, segment_end_ptr);
                        }
                    }
                } else { // Subsequent '..' operation
                    BufCpy(macro_segment_start_ptr + 1, segment_end_ptr);
                }
                
                // Reset scan to re-evaluate after modification
                current_ptr = macro_segment_start_ptr == NULL ? path : macro_segment_start_ptr;
                dot_ptr = NULL; // Reset dot pointer
                continue; // Continue outer loop from new current_ptr
            } else { // Found '..' not followed by '/', implies start of a macro segment
                macro_segment_start_ptr = current_ptr;
            }
        }
        current_ptr++;
    }
    return STATUS_SUCCESS;
}


// Mock implementations for external functions to make the code compilable.
// In a real system, these would be actual library functions.
static int FindTreeByPath(int tree_id, const char *path_part, const char *service_type) {
    (void)tree_id; (void)path_part; (void)service_type;
    // Simple mock: always "find" a tree if path_part and service_type are not empty
    if (path_part != NULL && *path_part != '\0' && service_type != NULL && *service_type != '\0') {
        return 1; // Return a dummy handle
    }
    return 0; // Not found
}

static int FindFileByName(int tree_handle, const char *file_name) {
    (void)tree_handle; (void)file_name;
    // Simple mock: always "find" a file if file_name is not empty
    if (file_name != NULL && *file_name != '\0') {
        // Return a dummy address for file info.
        // In a real system, this would point to actual file metadata.
        // For testing, just return a non-zero value.
        return 0x10000000;
    }
    return 0; // Not found
}