#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For malloc/free and strtoul
#include <stddef.h> // For size_t

// --- Dummy Declarations for External Functions ---
// These are minimal implementations to allow compilation.
// In a real application, these would be provided by a specific library or system.

// Simulates receiving 'len' bytes into 'buf'. Returns 0 on success, non-zero on error.
// For testing purposes, it defaults to simulating a 'deserialize' command (0x01)
// for the first byte read in main, and then simulates success for subsequent calls.
int receive_all(int fd, void *buf, size_t len, size_t *received_len) {
    static int first_byte_read = 0;
    if (received_len) *received_len = len;
    if (buf == NULL) return 0x20; // Invalid buffer

    if (fd == 0 && len == 1 && !first_byte_read) {
        *(unsigned char*)buf = 0x01; // Simulate deserialize command
        first_byte_read = 1;
    } else {
        memset(buf, 0, len); // Clear buffer for predictability
    }
    return 0;
}

// Simulates transmitting 'len' bytes from 'buf'. Returns 0 on success, non-zero on error.
int transmit_all(int fd, const void *buf, size_t len, size_t *sent_len) {
    if (sent_len) *sent_len = len;
    // For demonstration, you might print to stderr/stdout:
    // fprintf(stderr, "Transmit (fd %d, len %zu): '%.*s'\n", fd, (int)len, (int)len, (char*)buf);
    return 0;
}

// Allocates 'size' bytes and stores the pointer in 'out_ptr'. Returns 0 on success.
int allocate(size_t size, int flags, unsigned char **out_ptr) {
    *out_ptr = (unsigned char *)malloc(size);
    return (*out_ptr != NULL) ? 0 : 0x20; // 0x20 for allocation error
}

// Frees memory pointed to by 'ptr'. Returns 0 on success.
int deallocate(void *ptr, size_t size) {
    free(ptr);
    return 0;
}

// Finds the first occurrence of character 'c' in the first 'n' bytes of string 's'.
// Returns a pointer to the found character, or NULL if not found.
char* strnchr(const char *s, int c, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == (char)c) {
            return (char *)(s + i);
        }
    }
    return NULL;
}

// Converts a string of 'len' characters to an unsigned 32-bit integer.
// 'base' is the numerical base (e.g., 10 for decimal).
// 'result' stores the converted value. Returns 0 on success, non-zero on error.
int str2unt32n(const char *str, size_t len, unsigned int base, int flags, unsigned int *result) {
    char *temp_str = (char *)malloc(len + 1);
    if (!temp_str) return 0x20; // Allocation error
    strncpy(temp_str, str, len);
    temp_str[len] = '\0';

    char *endptr;
    // The original code used 0xff or 0xffffffff for base, which is likely intended as 10 (decimal).
    unsigned long val = strtoul(temp_str, &endptr, (base == 0 || base > 36) ? 10 : base);
    free(temp_str);

    if (endptr == temp_str || *endptr != '\0' || val > 0xFFFFFFFFU) {
        return 0x25; // Conversion error / not a valid number or out of range
    }
    *result = (unsigned int)val;
    return 0; // Success
}

// Converts an unsigned 32-bit integer to a string in 'buf'.
// 'buf_len' is the maximum size of the buffer. Returns the number of characters written (excluding null terminator).
size_t uint2str32(char *buf, size_t buf_len, unsigned int value) {
    int written = snprintf(buf, buf_len, "%u", value);
    if (written < 0 || (size_t)written >= buf_len) {
        if (buf_len > 0) buf[buf_len - 1] = '\0'; // Ensure null termination if truncated
        return buf_len > 0 ? buf_len - 1 : 0; // Return max possible written length
    }
    return (size_t)written;
}

// --- Global Variables ---
void *ns[256]; // Namespace: Array of pointers to objects (max 256 objects)
char json[4096]; // Buffer for incoming JSON data

// Inferred string literals from memcmp calls
const char DAT_0001300e[] = "NEW"; // Corresponds to "NEW" command
const char DAT_00013012[] = "SET"; // Corresponds to "SET" command
const char DAT_00013016[] = "DEL"; // Corresponds to "DEL" command

// --- Function Implementations ---

// Finds an object by its ID character in the global namespace.
void* object_find(char id_char) {
  for (unsigned int i = 0; i < 256; ++i) {
    if (ns[i] != NULL && *(char*)ns[i] == id_char) {
      return ns[i];
    }
  }
  return NULL; // Not found
}

// Parses an ID from the input string.
int parse_id(char **current_ptr_ref, unsigned short *remaining_len_ref, unsigned char *id_char_ref) {
  char *start_ptr = *current_ptr_ref;
  unsigned short current_len = *remaining_len_ref;

  char *end_ptr = strnchr(start_ptr, ' ', current_len);
  if (end_ptr == NULL) {
    return 0x24; // Error: space delimiter not found
  }

  unsigned short id_len = end_ptr - start_ptr;
  unsigned int id_val;
  int ret = str2unt32n(start_ptr, id_len, 10, 0, &id_val); // Assume base 10 for ID
  if (ret != 0) {
    return ret;
  }

  *id_char_ref = (unsigned char)id_val;
  *current_ptr_ref += id_len + 1; // Move past ID and space
  *remaining_len_ref -= (id_len + 1);
  return 0; // Success
}

// Parses an object type (NUMBER or STRING) from the input string.
int parse_type(char **current_ptr_ref, unsigned short *remaining_len_ref, unsigned int *type_ref) {
  char *start_ptr = *current_ptr_ref;
  unsigned short current_len = *remaining_len_ref;

  char *end_ptr = strnchr(start_ptr, ' ', current_len);
  if (end_ptr == NULL) {
    return 0x24; // Error: space delimiter not found
  }

  unsigned short type_str_len = end_ptr - start_ptr;

  if (type_str_len == 6 && memcmp("NUMBER", start_ptr, 6) == 0) {
    *type_ref = 1; // NUMBER type
  } else if (type_str_len == 6 && memcmp("STRING", start_ptr, 6) == 0) {
    *type_ref = 2; // STRING type
  } else {
    return 0x24; // Error: Unknown type
  }

  *current_ptr_ref += type_str_len + 1; // Move past type string and space
  *remaining_len_ref -= (type_str_len + 1);
  return 0; // Success
}

// Parses a number value from the input string.
int parse_number(char **current_ptr_ref, unsigned short *remaining_len_ref, unsigned int *number_val_ptr) {
  char *start_ptr = *current_ptr_ref;
  unsigned short current_len = *remaining_len_ref;

  char *end_ptr = strnchr(start_ptr, ' ', current_len);
  if (end_ptr == NULL) {
    return 0x24; // Error: space delimiter not found
  }

  unsigned short num_str_len = end_ptr - start_ptr;
  int ret = str2unt32n(start_ptr, num_str_len, 10, 0, number_val_ptr); // Base 10 for number value
  if (ret != 0) {
    return ret;
  }

  *current_ptr_ref += num_str_len + 1; // Move past number and space
  *remaining_len_ref -= (num_str_len + 1);
  return 0; // Success
}

// Parses a string value from the input string.
int parse_string(char **current_ptr_ref, unsigned short *remaining_len_ref, char *string_buffer) {
  char *start_ptr = *current_ptr_ref;
  unsigned short current_len = *remaining_len_ref;

  char *end_ptr = strnchr(start_ptr, ' ', current_len);
  if (end_ptr == NULL) {
    return 0x24; // Error: space delimiter not found
  }

  unsigned short str_val_len = end_ptr - start_ptr;
  size_t copy_len = str_val_len;
  if (copy_len > 235) { // Max string length is 235 for a 236-byte buffer (0xec)
    copy_len = 235;
  }
  strncpy(string_buffer, start_ptr, copy_len);
  string_buffer[copy_len] = '\0'; // Ensure null termination

  *current_ptr_ref += str_val_len + 1; // Move past string and space
  *remaining_len_ref -= (str_val_len + 1);
  return 0; // Success
}

// Creates a new object and adds it to the namespace.
int object_new(unsigned char id, int type, unsigned int number_val, char *string_val) {
  unsigned char *obj_ptr = NULL;
  int ret = allocate(0x1000, 0, &obj_ptr); // Allocate 4096 bytes for the object

  if (ret != 0) {
    return ret; // Allocation failed
  }

  if (object_find(id) != NULL) {
    deallocate(obj_ptr, 0x1000); // ID already exists, deallocate and return error
    return 0x24;
  }

  *obj_ptr = id; // Object ID
  *((unsigned int *)(obj_ptr + 4)) = 0; // Object Type (offset 4), initialized to 0
  *((void **)(obj_ptr + 8)) = NULL; // Pointer to number value (offset 8), initialized to NULL
  *((void **)(obj_ptr + 0xc)) = NULL; // Pointer to string value (offset 0xc), initialized to NULL

  if (type == 1) { // NUMBER type
    *((unsigned int *)(obj_ptr + 4)) = 1;
    *((unsigned int **)(obj_ptr + 8)) = (unsigned int *)(obj_ptr + 0x40); // Number value stored at offset 0x40
    **((unsigned int **)(obj_ptr + 8)) = number_val; // Store number value
  } else if (type == 2) { // STRING type
    *((unsigned int *)(obj_ptr + 4)) = 2;
    *((char **)(obj_ptr + 0xc)) = (char *)(obj_ptr + 0x80); // String value stored at offset 0x80
    if (string_val != NULL) {
      strncpy(*((char **)(obj_ptr + 0xc)), string_val, 235); // Copy max 235 chars
      (*((char **)(obj_ptr + 0xc)))[235] = '\0'; // Ensure null termination
    } else {
      (*((char **)(obj_ptr + 0xc)))[0] = '\0'; // Empty string
    }
  } else {
    deallocate(obj_ptr, 0x1000); // Invalid type, deallocate
    return 0x24;
  }
  ns[id] = obj_ptr; // Store object pointer in global namespace array
  return 0; // Success
}

// Deletes an object from the namespace.
int op_del(char **current_ptr_ref, unsigned short *remaining_len_ref) {
  unsigned char id;
  int ret = parse_id(current_ptr_ref, remaining_len_ref, &id);
  if (ret != 0) {
    return ret;
  }

  for (unsigned int i = 0; i < 256; ++i) {
    if (ns[i] != NULL && *(char*)ns[i] == id) {
      ret = deallocate(ns[i], 0x1000);
      ns[i] = NULL;
      return ret;
    }
  }
  return 0x24; // Object not found for deletion
}

// Sets the value of an existing object.
int op_set(char **current_ptr_ref, unsigned short *remaining_len_ref) {
  unsigned char id;
  int ret = parse_id(current_ptr_ref, remaining_len_ref, &id);
  if (ret != 0) {
    return ret;
  }

  void *obj_ptr = object_find(id);
  if (obj_ptr == NULL) {
    // If object not found, consume remaining input but report no error
    *current_ptr_ref += *remaining_len_ref;
    *remaining_len_ref = 0;
    return 0;
  }

  unsigned int obj_type = *((unsigned int*)((char*)obj_ptr + 4));

  if (obj_type == 1) { // NUMBER type
    // Pass pointer to the number value storage
    ret = parse_number(current_ptr_ref, remaining_len_ref, *((unsigned int**)((char*)obj_ptr + 8)));
  } else if (obj_type == 2) { // STRING type
    // Pass pointer to the string buffer
    ret = parse_string(current_ptr_ref, remaining_len_ref, *((char**)((char*)obj_ptr + 0xc)));
  } else {
    ret = 0x24; // Unknown object type
  }
  return ret;
}

// Handles the "NEW" command to create objects.
int op_new(char **current_ptr_ref, unsigned short *remaining_len_ref) {
  unsigned char id;
  int ret = parse_id(current_ptr_ref, remaining_len_ref, &id);
  if (ret != 0) {
    return ret;
  }

  unsigned int obj_type;
  ret = parse_type(current_ptr_ref, remaining_len_ref, &obj_type);
  if (ret != 0) {
    return ret;
  }

  char *end_ptr = strnchr(*current_ptr_ref, ' ', *remaining_len_ref); // Check for optional value

  if (end_ptr == NULL) { // No value supplied
    ret = object_new(id, obj_type, 0, NULL);
  } else if (obj_type == 1) { // Number type
    unsigned int number_value;
    char *original_ptr = *current_ptr_ref;
    unsigned short original_len = *remaining_len_ref;

    ret = parse_number(current_ptr_ref, remaining_len_ref, &number_value);
    if (ret == 0) { // Successfully parsed a number
      ret = object_new(id, 1, number_value, NULL);
    } else if (ret == 0x25) { // parse_number failed with specific error (e.g., format mismatch)
      // Original code's logic: if number parsing fails, try to parse as string and create a STRING type object.
      ret = 0; // Reset error to proceed
      *current_ptr_ref = original_ptr; // Rewind input stream
      *remaining_len_ref = original_len;

      char string_buffer[236]; // Max 0xec bytes (235 chars + null)
      memset(string_buffer, 0, sizeof(string_buffer));
      ret = parse_string(current_ptr_ref, remaining_len_ref, string_buffer);
      if (ret == 0) { // If string parse is successful, create as STRING type
        ret = object_new(id, 2, 0, string_buffer);
      }
    } else {
      ret = 0x24; // Other error from parse_number
    }
  } else if (obj_type == 2) { // String type
    char string_buffer[236]; // Max 0xec bytes
    memset(string_buffer, 0, sizeof(string_buffer));
    ret = parse_string(current_ptr_ref, remaining_len_ref, string_buffer);
    if (ret == 0) {
      ret = object_new(id, 2, 0, string_buffer);
    }
  } else {
    ret = 0x24; // Invalid object type
  }
  return ret;
}

// Parses JSON input to execute commands.
int parse_json(unsigned short total_len) {
  char *current_pos = json;
  unsigned short remaining_len = total_len;
  int ret = 0;

  while (remaining_len > 0) {
    char *first_nl = strnchr(current_pos, '\n', remaining_len);
    if (first_nl == NULL) { // No more newlines, processing ends
      return ret;
    }

    char *line_content_start = first_nl + 1;
    // Adjust remaining_len to exclude content before first_nl and first_nl itself
    remaining_len -= (first_nl - current_pos + 1);

    if (remaining_len == 0) { // If it was the last char, nothing more to process
        return ret;
    }

    char *second_nl = strnchr(line_content_start, '\n', remaining_len);
    if (second_nl == NULL) { // Expected a second newline, malformed input
      return 0x24;
    }

    unsigned short line_len = second_nl - line_content_start;
    char *cmd_line_ptr = line_content_start; // This is the line containing CMD ARGS

    char *space_ptr = strnchr(cmd_line_ptr, ' ', line_len);
    if (space_ptr == NULL) {
      return 0x24; // Malformed command line: no space
    }

    unsigned short cmd_len = space_ptr - cmd_line_ptr;
    if (cmd_len != 3) { // Commands "NEW", "SET", "DEL" are 3 chars
      return 0x24;
    }

    char *args_start_ptr = space_ptr + 1;
    unsigned short args_len = line_len - (cmd_len + 1);

    if (memcmp(DAT_0001300e, cmd_line_ptr, 3) == 0) { // "NEW"
      ret = op_new(&args_start_ptr, &args_len);
    } else if (memcmp(DAT_00013012, cmd_line_ptr, 3) == 0) { // "SET"
      ret = op_set(&args_start_ptr, &args_len);
    } else if (memcmp(DAT_00013016, cmd_line_ptr, 3) == 0) { // "DEL"
      ret = op_del(&args_start_ptr, &args_len);
    } else {
      return 0x24; // Unknown command
    }

    if (ret != 0) {
      return ret;
    }

    current_pos = second_nl + 1; // Move to the start of the next block
    // Adjust remaining_len to exclude the processed line and its newline
    remaining_len -= (line_len + 1);
  }
  return ret;
}

// Deserializes data from input.
int deserialize(void) {
  unsigned short json_len;
  size_t bytes_received;
  int ret;

  ret = receive_all(0, &json_len, sizeof(json_len), &bytes_received);
  if (ret != 0 || bytes_received != sizeof(json_len)) {
    return 0x20; // Error: failed to receive length or received wrong amount
  }

  ret = receive_all(0, json, json_len, &bytes_received);
  if (ret != 0 || json_len != bytes_received) {
    return 0x20; // Error: failed to receive JSON data or received wrong amount
  }

  return parse_json(json_len);
}

// Serializes objects from the namespace to output.
int serialize(void) {
  char line_buffer[256]; // Buffer for serializing a single object's line
  int ret = 0;

  for (unsigned int i = 0; i < 256; ++i) { // Iterate through ns array
    void *obj_ptr = ns[i];
    if (obj_ptr == NULL) {
      continue; // Skip empty slots
    }

    memset(line_buffer, 0, sizeof(line_buffer));
    char *current_write_pos = line_buffer;
    size_t space_left = sizeof(line_buffer); // Total buffer size

    // Append object ID
    int chars_written = snprintf(current_write_pos, space_left, "%u", *(unsigned char*)obj_ptr);
    if (chars_written < 0 || (size_t)chars_written >= space_left) { /* Handle error / truncation */ }
    current_write_pos += chars_written;
    space_left -= chars_written;

    unsigned int obj_type = *((unsigned int*)((char*)obj_ptr + 4));

    if (obj_type == 1) { // NUMBER type
      chars_written = snprintf(current_write_pos, space_left, " NUMBER ");
      if (chars_written < 0 || (size_t)chars_written >= space_left) { /* Handle error / truncation */ }
      current_write_pos += chars_written;
      space_left -= chars_written;

      chars_written = snprintf(current_write_pos, space_left, "%u", **((unsigned int**)((char*)obj_ptr + 8)));
      if (chars_written < 0 || (size_t)chars_written >= space_left) { /* Handle error / truncation */ }
      current_write_pos += chars_written;
      space_left -= chars_written;
    } else if (obj_type == 2) { // STRING type
      chars_written = snprintf(current_write_pos, space_left, " STRING ");
      if (chars_written < 0 || (size_t)chars_written >= space_left) { /* Handle error / truncation */ }
      current_write_pos += chars_written;
      space_left -= chars_written;

      const char *obj_string = *((char**)((char*)obj_ptr + 0xc));
      if (obj_string != NULL) {
        chars_written = snprintf(current_write_pos, space_left, "%s", obj_string);
        if (chars_written < 0 || (size_t)chars_written >= space_left) { /* Handle error / truncation */ }
        current_write_pos += chars_written;
        space_left -= chars_written;
      }
    }

    // Add newline terminator
    if (space_left > 0) {
        *current_write_pos = '\n';
        current_write_pos++;
    } else {
        // Buffer overflow, consider error handling or truncation message
    }

    // Transmit actual length of data written to buffer
    ret = transmit_all(1, line_buffer, current_write_pos - line_buffer, NULL);
    if (ret != 0) {
      return 0x21; // Error during transmission
    }
  }
  return 0; // Success
}

// Main function to handle commands.
int main(void) {
  int ret = 0;
  memset(ns, 0, sizeof(ns)); // Initialize all object pointers in namespace to NULL

  while (1) { // Infinite loop for command processing
    unsigned char command_byte = 0;
    size_t bytes_read = 0;

    int receive_ret = receive_all(0, &command_byte, 1, &bytes_read);
    if (receive_ret != 0) {
      return 0x20; // Error in receive_all
    }
    if (bytes_read == 0) {
      // No byte was read, indicating EOF or connection closed. Exit.
      return ret;
    }

    if (command_byte == 0x01) { // Deserialize command
      ret = deserialize();
    } else if (command_byte == 0x02) { // Serialize command
      ret = serialize();
    } else if (command_byte == 0x03) { // Exit command
      return ret; // Exit with current status
    } else {
      ret = 0x24; // Unknown command byte
    }

    if (ret != 0) {
      return ret; // If any operation failed, return error
    }
  }
  // This part of the code should ideally not be reached if the loop handles all exit conditions.
  return ret;
}