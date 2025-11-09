#include <stdint.h> // For uint8_t, uint16_t, uint32_t
#include <string.h> // For memset, memcpy, strncpy, memcmp, strlen, snprintf
#include <stdlib.h> // For malloc, free, strtoul
#include <unistd.h> // For read, write (stubs for receive_all, transmit_all)
#include <stdio.h>  // For snprintf (stub for uint2str32)

// --- Global Constants and Declarations ---
#define NS_SIZE 256 // Size of the global object pointer array
#define JSON_BUFFER_SIZE 1024 // Example buffer size for JSON input
#define OBJECT_ALLOC_SIZE 0x1000 // 4096 bytes allocated per object
#define MAX_OBJECT_STRING_STORAGE_LEN 236 // Max length of string value stored within an object
#define MAX_STRING_BUF_LEN (MAX_OBJECT_STRING_STORAGE_LEN + 1) // Buffer size for local string copies (includes null terminator)
#define MAX_ID_VAL 0xff // Max ID value (0-255)

// Error codes (guessed from original snippet's return values)
#define ERR_NONE 0x00
#define ERR_RECEIVE_FAILED 0x20
#define ERR_TRANSMIT_FAILED 0x21
#define ERR_MALFORMED_INPUT 0x24
#define ERR_CONVERSION_FAILED 0x25
#define ERR_MEMORY_ALLOC_FAILED 0x26

void* ns[NS_SIZE]; // Global array to store pointers to objects

char json[JSON_BUFFER_SIZE]; // Global buffer for JSON input

// Global string literals from the original snippet's DAT_ addresses
const char DAT_0001300e[] = "NEW";
const char DAT_00013012[] = "SET";
const char DAT_00013016[] = "DEL";

// --- Stub Implementations for Missing Functions ---

// Simplified strnchr, returns char* or NULL
char* strnchr(const char* s, int c, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == (char)c) {
            return (char*)(s + i);
        }
    }
    return NULL;
}

// Simplified str2unt32n, converts a string segment to uint32_t
// Assumes base 10 if base is 0, otherwise specified base.
// The original uses 0xff as max_val for ID and 0xffffffff for number, and 0 as base.
int str2unt32n(const char* s, size_t n, uint32_t max_val, int base, uint32_t* out) {
    if (n == 0) return ERR_CONVERSION_FAILED;

    char* temp_buf = (char*)malloc(n + 1);
    if (!temp_buf) return ERR_MEMORY_ALLOC_FAILED;
    strncpy(temp_buf, s, n);
    temp_buf[n] = '\0';

    char* endptr;
    unsigned long val = strtoul(temp_buf, &endptr, base == 0 ? 10 : base);
    free(temp_buf);

    if (endptr != temp_buf + n || val > max_val) {
        return ERR_CONVERSION_FAILED;
    }
    *out = (uint32_t)val;
    return ERR_NONE;
}

// Simplified uint2str32, converts uint32_t to string
int uint2str32(char* buf, size_t buf_len, uint32_t val) {
    int chars_written = snprintf(buf, buf_len, "%u", val);
    if (chars_written < 0 || (size_t)chars_written >= buf_len) {
        return 0; // Error or buffer too small, return 0 length
    }
    return chars_written;
}

// Simplified allocate, returns ERR_NONE on success, non-zero on failure
int allocate(size_t size, int flags, void** ptr) {
    (void)flags; // Unused
    *ptr = malloc(size);
    return (*ptr != NULL) ? ERR_NONE : ERR_MEMORY_ALLOC_FAILED;
}

// Simplified deallocate, returns ERR_NONE on success
int deallocate(void* ptr, size_t size) {
    (void)size; // Unused
    free(ptr);
    return ERR_NONE;
}

// Simplified receive_all, returns ERR_NONE on success
int receive_all(int fd, void* buf, size_t len, size_t* actual_len) {
    ssize_t bytes_read = read(fd, buf, len);
    if (bytes_read < 0) {
        *actual_len = 0;
        return ERR_RECEIVE_FAILED;
    }
    *actual_len = (size_t)bytes_read;
    return ERR_NONE;
}

// Simplified transmit_all, returns ERR_NONE on success
int transmit_all(int fd, const void* buf, size_t len, size_t* actual_len) {
    ssize_t bytes_written = write(fd, buf, len);
    if (bytes_written < 0) {
        *actual_len = 0;
        return ERR_TRANSMIT_FAILED;
    }
    *actual_len = (size_t)bytes_written;
    return ERR_NONE;
}


// Function: object_find
// Searches for an object by its ID in the global 'ns' array.
// Returns a pointer to the object if found, otherwise NULL.
void* object_find(char id_char) {
    for (uint32_t i = 0; i < NS_SIZE; ++i) {
        char* obj_ptr = (char*)ns[i];
        if (obj_ptr != NULL && *obj_ptr == id_char) {
            return obj_ptr;
        }
    }
    return NULL;
}

// Function: parse_id
// Parses an object ID from the input stream.
// stream (char**): Pointer to the current position in the input string.
// len (int16_t*): Pointer to the remaining length of the input string.
// id_char (uint8_t*): Pointer to store the parsed object ID.
// Returns ERR_NONE on success, non-zero on error.
int parse_id(char **stream, int16_t *len, uint8_t *id_char) {
    char *space_pos = strnchr(*stream, ' ', *len);
    if (!space_pos) {
        return ERR_MALFORMED_INPUT; // Error: No space found, malformed ID
    }

    int16_t id_len = space_pos - *stream;
    uint32_t temp_id_val;
    int result = str2unt32n(*stream, id_len, MAX_ID_VAL, 0, &temp_id_val);
    if (result == ERR_NONE) {
        *id_char = (uint8_t)temp_id_val;
        *stream += id_len + 1; // Advance stream past ID and space
        *len -= (id_len + 1);  // Reduce remaining length
    }
    return result;
}

// Function: parse_type
// Parses an object type (NUMBER or STRING) from the input stream.
// stream (char**): Pointer to the current position in the input string.
// len (int16_t*): Pointer to the remaining length of the input string.
// type_val (uint32_t*): Pointer to store the parsed type (1 for NUMBER, 2 for STRING).
// Returns ERR_NONE on success, non-zero on error.
int parse_type(char **stream, int16_t *len, uint32_t *type_val) {
    char *space_pos = strnchr(*stream, ' ', *len);
    if (!space_pos) {
        return ERR_MALFORMED_INPUT; // Error: No space found, malformed type
    }

    int16_t type_len = space_pos - *stream;
    if (memcmp("NUMBER", *stream, 6) == 0) {
        *type_val = 1;
    } else if (memcmp("STRING", *stream, 6) == 0) {
        *type_val = 2;
    } else {
        return ERR_MALFORMED_INPUT; // Unknown type
    }

    *stream += type_len + 1; // Advance stream past type and space
    *len -= (type_len + 1);  // Reduce remaining length
    return ERR_NONE;
}

// Function: parse_number
// Parses a number value from the input stream.
// stream (char**): Pointer to the current position in the input string.
// len (int16_t*): Pointer to the remaining length of the input string.
// num_val (uint32_t*): Pointer to store the parsed number.
// Returns ERR_NONE on success, non-zero on error.
int parse_number(char **stream, int16_t *len, uint32_t *num_val) {
    char *space_pos = strnchr(*stream, ' ', *len);
    if (!space_pos) {
        return ERR_MALFORMED_INPUT; // Error: No space found, malformed number
    }

    int16_t num_len = space_pos - *stream;
    int result = str2unt32n(*stream, num_len, 0xffffffff, 0, num_val);
    if (result == ERR_NONE) {
        *stream += num_len + 1; // Advance stream past number and space
        *len -= (num_len + 1);  // Reduce remaining length
    }
    return result;
}

// Function: parse_string
// Parses a string value from the input stream.
// stream (char**): Pointer to the current position in the input string.
// len (int16_t*): Pointer to the remaining length of the input string.
// str_buf (char*): Buffer to store the parsed string.
// Returns ERR_NONE on success, non-zero on error.
int parse_string(char **stream, int16_t *len, char *str_buf) {
    char *space_pos = strnchr(*stream, ' ', *len);
    if (!space_pos) {
        return ERR_MALFORMED_INPUT; // Error: No space found, malformed string
    }

    int16_t str_len = space_pos - *stream;
    size_t copy_len = (size_t)str_len;
    if (copy_len > MAX_OBJECT_STRING_STORAGE_LEN) {
        copy_len = MAX_OBJECT_STRING_STORAGE_LEN;
    }
    strncpy(str_buf, *stream, copy_len);
    str_buf[copy_len] = '\0'; // Ensure null termination

    *stream += str_len + 1; // Advance stream past string and space
    *len -= (str_len + 1);  // Reduce remaining length
    return ERR_NONE;
}

// Function: object_new
// Creates a new object and stores it in the global 'ns' array.
// id_char (uint8_t): The ID of the new object.
// type (int): The type of the object (1 for NUMBER, 2 for STRING).
// number_val (uint32_t): The initial number value if type is NUMBER.
// string_val (char*): The initial string value if type is STRING.
// Returns ERR_NONE on success, non-zero on error.
int object_new(uint8_t id_char, int type, uint32_t number_val, char *string_val) {
    if (object_find(id_char) != NULL) {
        return ERR_MALFORMED_INPUT; // Object with this ID already exists (guessed error)
    }

    char* new_obj_ptr;
    int result = allocate(OBJECT_ALLOC_SIZE, 0, (void**)&new_obj_ptr);
    if (result != ERR_NONE) {
        return result; // Allocation failed
    }

    // Initialize object fields using pointer arithmetic
    *new_obj_ptr = id_char; // ID at offset 0
    *(uint32_t*)(new_obj_ptr + 4) = type; // Type at offset 4

    // Initialize value pointers and values
    *(uint32_t**)(new_obj_ptr + 8) = NULL;  // Number value pointer (initially null)
    *(char**)(new_obj_ptr + 0xC) = NULL; // String value pointer (initially null)

    if (type == 1) { // NUMBER type
        *(uint32_t**)(new_obj_ptr + 8) = (uint32_t*)(new_obj_ptr + 0x40); // Pointer to number storage
        **(uint32_t**)(new_obj_ptr + 8) = number_val; // Store number value
    } else if (type == 2) { // STRING type
        *(char**)(new_obj_ptr + 0xC) = (char*)(new_obj_ptr + 0x80); // Pointer to string storage
        if (string_val != NULL) {
            strncpy(*(char**)(new_obj_ptr + 0xC), string_val, MAX_OBJECT_STRING_STORAGE_LEN);
            (*(char**)(new_obj_ptr + 0xC))[MAX_OBJECT_STRING_STORAGE_LEN] = '\0'; // Ensure null termination
        } else {
            (*(char**)(new_obj_ptr + 0xC))[0] = '\0'; // Empty string
        }
    } else {
        deallocate(new_obj_ptr, OBJECT_ALLOC_SIZE); // Deallocate if unknown type
        return ERR_MALFORMED_INPUT; // Unknown type, error
    }

    // Store object pointer in global ns array
    ns[id_char] = new_obj_ptr;

    return ERR_NONE;
}

// Function: op_del
// Deletes an object by its ID.
// stream (char**): Pointer to the current position in the input string.
// len (int16_t*): Pointer to the remaining length of the input string.
// Returns ERR_NONE on success, non-zero on error.
int op_del(char **stream, int16_t *len) {
    uint8_t id_char;
    int result = parse_id(stream, len, &id_char);
    if (result != ERR_NONE) {
        return result;
    }

    for (uint32_t i = 0; i < NS_SIZE; ++i) {
        char* obj_ptr = (char*)ns[i];
        if (obj_ptr != NULL && *obj_ptr == id_char) {
            result = deallocate(obj_ptr, OBJECT_ALLOC_SIZE);
            ns[i] = NULL; // Clear pointer in global array
            return result;
        }
    }
    return ERR_MALFORMED_INPUT; // Object not found (guessed error)
}

// Function: op_set
// Sets the value of an existing object.
// stream (char**): Pointer to the current position in the input string.
// len (int16_t*): Pointer to the remaining length of the input string.
// Returns ERR_NONE on success, non-zero on error.
int op_set(char **stream, int16_t *len) {
    uint8_t id_char;
    int result = parse_id(stream, len, &id_char);
    if (result != ERR_NONE) {
        return result;
    }

    char* obj_ptr = (char*)object_find(id_char);
    if (obj_ptr == NULL) {
        // Object not found, consume remaining input for this line as per original logic
        *stream += *len;
        *len = 0;
        return ERR_NONE;
    }

    uint32_t obj_type = *(uint32_t*)(obj_ptr + 4); // Type at offset 4

    if (obj_type == 1) { // NUMBER type
        char* temp_stream_ptr = *stream;
        int16_t temp_len = *len;
        uint32_t number_val;
        result = parse_number(stream, len, &number_val);

        if (result == ERR_NONE) {
            **(uint32_t**)(obj_ptr + 8) = number_val; // Set number value
        } else if (result == ERR_CONVERSION_FAILED) {
            // Original code attempted to parse as string and write to 0xc offset.
            // This is a type mismatch for a NUMBER object. Assuming this should be an error.
            return ERR_MALFORMED_INPUT; // Expected number, got something else
        } else {
            return result; // Other number parsing error
        }
    } else if (obj_type == 2) { // STRING type
        result = parse_string(stream, len, *(char**)(obj_ptr + 0xC)); // Set string value
    } else {
        return ERR_MALFORMED_INPUT; // Unknown object type (should not happen for valid objects)
    }
    return result;
}

// Function: op_new
// Handles the "NEW" command, creating a new object.
// stream (char**): Pointer to the current position in the input string.
// len (int16_t*): Pointer to the remaining length of the input string.
// Returns ERR_NONE on success, non-zero on error.
int op_new(char **stream, int16_t *len) {
    uint8_t id_char;
    int result = parse_id(stream, len, &id_char);
    if (result != ERR_NONE) {
        return result;
    }

    uint32_t type_val;
    result = parse_type(stream, len, &type_val);
    if (result != ERR_NONE) {
        return result;
    }

    // Check for optional value part
    char *space_pos = strnchr(*stream, ' ', *len);
    if (!space_pos) {
        // No value specified, create with default value
        return object_new(id_char, (int)type_val, 0, NULL);
    }

    // Value is specified, parse it
    if (type_val == 1) { // NUMBER type
        char* temp_stream_ptr = *stream;
        int16_t temp_len = *len;
        uint32_t number_val;
        result = parse_number(stream, len, &number_val);

        if (result == ERR_NONE) {
            return object_new(id_char, 1, number_val, NULL);
        } else if (result == ERR_CONVERSION_FAILED) {
            // Revert stream position to try parsing as string (original logic)
            *stream = temp_stream_ptr;
            *len = temp_len;

            char string_buf[MAX_STRING_BUF_LEN] = {0}; // Buffer for string value
            result = parse_string(stream, len, string_buf);
            if (result == ERR_NONE) {
                return object_new(id_char, 2, 0, string_buf); // Create as STRING type
            } else {
                return ERR_MALFORMED_INPUT; // Neither number nor string, error
            }
        } else {
            return result; // Other number parsing error
        }
    } else if (type_val == 2) { // STRING type
        char string_buf[MAX_STRING_BUF_LEN] = {0}; // Buffer for string value
        result = parse_string(stream, len, string_buf);
        if (result == ERR_NONE) {
            return object_new(id_char, 2, 0, string_buf);
        }
        return result;
    } else {
        return ERR_MALFORMED_INPUT; // Unknown object type
    }
}

// Function: parse_json
// Parses the entire JSON input buffer, line by line.
// total_len (int16_t): The total length of the JSON input.
// Returns ERR_NONE on success, non-zero on error.
int parse_json(int16_t total_len) {
    char* current_line_start = json;
    int16_t remaining_len = total_len;
    int result = ERR_NONE;

    while (remaining_len > 0) {
        char* line_end_ptr = strnchr(current_line_start, '\n', remaining_len);
        if (!line_end_ptr) {
            // No more full lines. Return current result.
            break;
        }

        int16_t line_length = line_end_ptr - current_line_start;

        // Find the first space to separate command from data
        char* command_end_ptr = strnchr(current_line_start, ' ', line_length);
        if (!command_end_ptr) {
            return ERR_MALFORMED_INPUT; // Malformed line: no space after command
        }

        int16_t command_len = command_end_ptr - current_line_start;
        if (command_len != 3) {
            return ERR_MALFORMED_INPUT; // Command must be 3 characters
        }

        // Set up pointers/lengths for the data part of the line
        char* data_stream_ptr = command_end_ptr + 1;
        int16_t data_stream_len = line_length - (command_len + 1);

        // Compare command and call appropriate function
        if (memcmp(DAT_0001300e, current_line_start, 3) == 0) { // "NEW"
            result = op_new(&data_stream_ptr, &data_stream_len);
        } else if (memcmp(DAT_00013012, current_line_start, 3) == 0) { // "SET"
            result = op_set(&data_stream_ptr, &data_stream_len);
        } else if (memcmp(DAT_00013016, current_line_start, 3) == 0) { // "DEL"
            result = op_del(&data_stream_ptr, &data_stream_len);
        } else {
            return ERR_MALFORMED_INPUT; // Unknown command
        }

        if (result != ERR_NONE) {
            return result; // Error occurred during command processing
        }

        // Move to the next line
        remaining_len -= (line_length + 1); // +1 for the newline character
        current_line_start = line_end_ptr + 1;
    }
    return result;
}

// Function: deserialize
// Receives JSON data from an input source and parses it.
// Returns ERR_NONE on success, non-zero on error.
uint32_t deserialize(void) {
    uint16_t json_len = 0;
    size_t actual_len = 0;
    int result = receive_all(0, &json_len, sizeof(json_len), &actual_len);

    if (result != ERR_NONE || actual_len != sizeof(json_len)) {
        return ERR_RECEIVE_FAILED; // Error receiving length or incorrect length received
    }

    // Convert network byte order if necessary (assuming host byte order for simplicity here)
    // json_len = ntohs(json_len); // If json_len comes from network

    result = receive_all(0, json, json_len, &actual_len);

    if (result != ERR_NONE || json_len != actual_len) {
        return ERR_RECEIVE_FAILED; // Error receiving JSON data or incomplete data
    }

    return parse_json(json_len);
}

// Function: serialize
// Serializes objects from the global 'ns' array into a string format and transmits them.
// Returns ERR_NONE on success, non-zero on error.
int serialize(void) {
    char line_buf[256]; // Buffer for each line of serialized output
    int result = ERR_NONE;

    for (uint32_t i = 0; i < NS_SIZE; ++i) {
        memset(line_buf, 0, sizeof(line_buf));
        char* current_buf_pos = line_buf;
        char* obj_ptr = (char*)ns[i];

        if (obj_ptr != NULL) {
            // ID
            int chars_written = uint2str32(current_buf_pos, sizeof(line_buf) - (current_buf_pos - line_buf), *obj_ptr);
            if (chars_written == 0) return ERR_TRANSMIT_FAILED;
            current_buf_pos += chars_written;

            uint32_t obj_type = *(uint32_t*)(obj_ptr + 4); // Type at offset 4

            if (obj_type == 1) { // NUMBER type
                // " NUMBER "
                chars_written = snprintf(current_buf_pos, sizeof(line_buf) - (current_buf_pos - line_buf), " NUMBER ");
                if (chars_written < 0 || (size_t)chars_written >= sizeof(line_buf) - (current_buf_pos - line_buf)) return ERR_TRANSMIT_FAILED;
                current_buf_pos += chars_written;

                // Number value
                chars_written = uint2str32(current_buf_pos, sizeof(line_buf) - (current_buf_pos - line_buf), **(uint32_t**)(obj_ptr + 8));
                if (chars_written == 0) return ERR_TRANSMIT_FAILED;
                current_buf_pos += chars_written;
            } else if (obj_type == 2) { // STRING type
                // " STRING "
                chars_written = snprintf(current_buf_pos, sizeof(line_buf) - (current_buf_pos - line_buf), " STRING ");
                if (chars_written < 0 || (size_t)chars_written >= sizeof(line_buf) - (current_buf_pos - line_buf)) return ERR_TRANSMIT_FAILED;
                current_buf_pos += chars_written;

                // String value
                chars_written = snprintf(current_buf_pos, sizeof(line_buf) - (current_buf_pos - line_buf), "%s", *(char**)(obj_ptr + 0xC));
                if (chars_written < 0 || (size_t)chars_written >= sizeof(line_buf) - (current_buf_pos - line_buf)) return ERR_TRANSMIT_FAILED;
                current_buf_pos += chars_written;
            } else {
                // Unknown type, skip this object
                continue;
            }

            *current_buf_pos = '\n'; // Add newline
            current_buf_pos++;

            size_t line_len = current_buf_pos - line_buf;
            size_t actual_len;
            result = transmit_all(1, line_buf, line_len, &actual_len);
            if (result != ERR_NONE || actual_len != line_len) {
                return ERR_TRANSMIT_FAILED; // Error transmitting or incomplete transmission
            }
        }
    }
    return result;
}

// Function: main
int main(void) {
    int result = ERR_NONE;
    char command_byte;
    size_t actual_len;

    memset(ns, 0, sizeof(ns)); // Initialize global object array to NULL pointers

    while (1) {
        command_byte = '\0'; // Reset command byte
        actual_len = 0;      // Reset actual_len

        // Receive a single byte command
        int receive_result = receive_all(0, &command_byte, 1, &actual_len);
        if (receive_result != ERR_NONE) {
            return ERR_RECEIVE_FAILED; // Error receiving command
        }
        if (actual_len == 0) {
            return ERR_NONE; // Exit cleanly if no more input
        }

        if (command_byte == '\x01') { // Deserialize command
            result = deserialize();
            if (result != ERR_NONE) {
                return result;
            }
        } else if (command_byte == '\x02') { // Serialize command
            result = serialize();
            if (result != ERR_NONE) {
                return result;
            }
        } else if (command_byte == '\x03') { // Exit command
            return result; // Exit with current result (ERR_NONE if no errors)
        } else {
            return ERR_MALFORMED_INPUT; // Unknown command
        }
    }
    return result; // Should not be reached
}