#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Define custom types from the decompilation
// 'undefined4' typically means a 4-byte unsigned integer or a pointer.
// 'undefined' typically means a 1-byte value.
// 'byte' typically means an unsigned char.

#define SENTENCE_PAYLOAD_BUFFER_SIZE 0x41 // 65 bytes

// Error codes based on decompiled values
#define ERR_INVALID_POINTER      -49 // 0xffffffcf
#define ERR_SENTENCE_NOT_FOUND   -50 // 0xffffffce
#define ERR_PARSE_FAILED         -50 // -0x32

// Structure for the sentence information, inferred from memory access patterns.
// The original code uses fixed offsets, implying a specific memory layout (likely 32-bit).
// `__attribute__((packed))` is used to ensure the compiler does not add padding
// and matches the byte offsets inferred from the decompiled code.
// This structure assumes `sizeof(int) == 4` and `sizeof(char*) == 4`.
// On a 64-bit system, `sizeof(char*)` is typically 8, which would break the original offsets.
typedef struct __attribute__((packed)) {
    int count;             // Offset 0
    int total_count;       // Offset 4
    int id;                // Offset 8
    char *payload_ptr;     // Offset 12 (points to current write position within payload_buffer)
    char state;            // Offset 16 (State: 1=initial, 2=partial, 3=complete)
    char msg_type;         // Offset 17 (Message type)
    char _padding[2];      // Offset 18 (2 bytes padding to align payload_buffer at offset 20)
    char payload_buffer[SENTENCE_PAYLOAD_BUFFER_SIZE]; // Offset 20 (Actual buffer for payload)
} SentenceInfo;

// --- Forward Declarations for missing functions ---
static int is_hex_digit(int c);
static char ascii_hex_to_bin(int c);
static char get_msg_type(const char *field_ptr);

// --- Provided functions (modified) ---

// Function: get_sentence_start
static int get_sentence_start(const char *sentence_str, char **sentence_start_ptr) {
  if (sentence_str == NULL || sentence_start_ptr == NULL) {
    return ERR_INVALID_POINTER;
  }

  const char *current = sentence_str;
  while (*current != '!' && *current != '\0') {
    current++;
  }

  if (*current == '!') {
    *sentence_start_ptr = (char *)current;
    return 0;
  } else {
    *sentence_start_ptr = NULL;
    return ERR_SENTENCE_NOT_FOUND;
  }
}

// Function: get_next_field
static char * get_next_field(char *current_field_start) {
  if (current_field_start == NULL) {
    return NULL;
  }

  while (*current_field_start != '\0' && *current_field_start != '*' && *current_field_start != ',') {
    current_field_start++;
  }

  if (*current_field_start == '\0') {
    return NULL;
  }
  
  return current_field_start + 1;
}

// Function: is_checksum_correct
static int is_checksum_correct(const char *sentence) {
  char *sentence_start_ptr = NULL;
  int ret_code = get_sentence_start(sentence, &sentence_start_ptr);
  if (ret_code != 0) {
    return ERR_SENTENCE_NOT_FOUND;
  }

  unsigned char calculated_checksum = 0;
  const char *current_char = sentence_start_ptr;

  current_char++; // Skip the '!'

  while (*current_char != '\0' && *current_char != '*') {
    calculated_checksum ^= *current_char;
    current_char++;
  }

  if (*current_char != '*') {
    return ERR_SENTENCE_NOT_FOUND;
  }

  if (!is_hex_digit((int)current_char[1]) || !is_hex_digit((int)current_char[2])) {
    return ERR_SENTENCE_NOT_FOUND;
  }

  char hex_val_msb = ascii_hex_to_bin((int)current_char[1]);
  char hex_val_lsb = ascii_hex_to_bin((int)current_char[2]);

  if (hex_val_msb < 0 || hex_val_lsb < 0) {
      return ERR_SENTENCE_NOT_FOUND;
  }

  unsigned int expected_checksum = (unsigned int)hex_val_msb * 0x10 + (unsigned int)hex_val_lsb;

  return (calculated_checksum == expected_checksum) ? 1 : 0;
}

// Function: field_to_uint
static int field_to_uint(const char *field_str, unsigned int *value) {
  *value = 0;
  if (field_str == NULL) {
      return ERR_INVALID_POINTER;
  }

  const char *current = field_str;
  while (*current != '\0' && *current != '*' && *current != ',') {
    *value *= 10;
    char digit_val = ascii_hex_to_bin((int)*current);
    if (digit_val < 0 || digit_val > 9) {
        return ERR_PARSE_FAILED;
    }
    *value += (unsigned int)digit_val;
    current++;
  }
  return 0;
}

// Function: fieldncpy
static char * fieldncpy(const char *source, char *destination, int max_len) {
  if (source == NULL || destination == NULL || max_len <= 0) {
    if (destination != NULL) {
        *destination = '\0';
    }
    return destination;
  }

  while (max_len > 0 && *source != '\0' && *source != '*' && *source != ',') {
    *destination = *source;
    destination++;
    source++;
    max_len--;
  }
  *destination = '\0';
  return destination;
}

// Function: reset_sentence_struct
void reset_sentence_struct(SentenceInfo *info) {
  if (info == NULL) return;
  info->count = 0;
  info->total_count = 0;
  info->id = 0;
  info->state = 1; // Initial state
  info->msg_type = 0;
  memset(info->payload_buffer, 0, sizeof(info->payload_buffer));
  info->payload_ptr = info->payload_buffer; // Points to the start of the buffer
}

// Function: parse_sentence
int parse_sentence(const char *sentence_str, SentenceInfo *info) {
  char *current_field_ptr = NULL;
  unsigned int count_field = 0;
  unsigned int total_count_field = 0;
  unsigned int id_field = 0;
  
  if (info == NULL) {
      return ERR_INVALID_POINTER;
  }

  int ret_code = get_sentence_start(sentence_str, &current_field_ptr);
  if (ret_code != 0) {
    return ret_code;
  }

  const char *prefix = "!AAAAA";

  if (strncmp(prefix, current_field_ptr, strlen(prefix)) != 0) {
    return ERR_PARSE_FAILED;
  }

  if (is_checksum_correct(current_field_ptr) != 1) {
    return ERR_PARSE_FAILED;
  }

  current_field_ptr = get_next_field(current_field_ptr);
  if (current_field_ptr == NULL) return ERR_PARSE_FAILED;

  if (field_to_uint(current_field_ptr, &count_field) != 0) return ERR_PARSE_FAILED;
  current_field_ptr = get_next_field(current_field_ptr);
  if (current_field_ptr == NULL) return ERR_PARSE_FAILED;

  if (field_to_uint(current_field_ptr, &total_count_field) != 0) return ERR_PARSE_FAILED;
  current_field_ptr = get_next_field(current_field_ptr);
  if (current_field_ptr == NULL) return ERR_PARSE_FAILED;

  if (field_to_uint(current_field_ptr, &id_field) != 0) return ERR_PARSE_FAILED;
  char *payload_field_start_ptr = current_field_ptr; // Store this for payload copy
  current_field_ptr = get_next_field(current_field_ptr);
  if (current_field_ptr == NULL) return ERR_PARSE_FAILED; // Expected more fields, but none found

  if (info->state == 1) { // Initial state
    if (count_field != 1 || total_count_field == 0) {
      return ERR_PARSE_FAILED;
    }
    info->total_count = total_count_field;
    info->count = 1;
    info->id = id_field;
  } else if (info->state == 2) { // Partial state (subsequent fragments)
    if ((info->count + 1U != count_field) || (info->total_count != total_count_field) ||
        (info->id != id_field) || (total_count_field < count_field)) {
      return ERR_PARSE_FAILED;
    }
    info->count++;
  } else { // Invalid state
    return ERR_PARSE_FAILED;
  }

  char *next_field_ptr_for_payload = get_next_field(payload_field_start_ptr);
  int payload_field_len = 0;
  if (next_field_ptr_for_payload != NULL) {
      payload_field_len = (int)(next_field_ptr_for_payload - payload_field_start_ptr - 1);
  } else {
      payload_field_len = (int)strlen(payload_field_start_ptr);
  }
  
  int remaining_buffer_space = SENTENCE_PAYLOAD_BUFFER_SIZE - (int)(info->payload_ptr - info->payload_buffer);
  if (payload_field_len > remaining_buffer_space -1) {
      payload_field_len = remaining_buffer_space -1;
  }

  info->payload_ptr = fieldncpy(payload_field_start_ptr, info->payload_ptr, payload_field_len);
  
  if (info->state == 1) {
    char msg_type_val = get_msg_type(current_field_ptr);
    info->msg_type = msg_type_val;
    if (info->msg_type != 1 && info->msg_type != 4 && info->msg_type != 5) {
      return ERR_PARSE_FAILED;
    }
  }
  
  if (info->total_count == info->count) {
    info->state = 3; // Complete
  } else {
    info->state = 2; // Partial
  }

  return 0; // Success
}

// --- Placeholder Implementations for missing external functions ---
static int is_hex_digit(int c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static char ascii_hex_to_bin(int c) {
    if (c >= '0' && c <= '9') {
        return (char)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return (char)(c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F') {
        return (char)(c - 'A' + 10);
    }
    return -1; // Error
}

static char get_msg_type(const char *field_ptr) {
    if (field_ptr == NULL || *field_ptr == '\0') {
        return 0;
    }
    char val = ascii_hex_to_bin((int)*field_ptr);
    if (val < 0 || val > 9) {
        return 0;
    }
    return val;
}

// --- Main function for compilation and demonstration ---
int main() {
    SentenceInfo my_sentence_info;
    reset_sentence_struct(&my_sentence_info);

    printf("--- Test Case 1: First fragment (state 1 -> 2) ---\n");
    const char *sentence1 = "!AAAAA,1,2,123,PAYLOAD_PART1,1*FF"; // count=1, total=2, id=123, payload="PAYLOAD_PART1", msg_type=1
    int result1 = parse_sentence(sentence1, &my_sentence_info);
    if (result1 == 0) {
        printf("Parsed sentence 1 successfully.\nState: %d, Count: %d, Total: %d, ID: %d, MsgType: %d, Payload: '%s'\n",
               my_sentence_info.state, my_sentence_info.count, my_sentence_info.total_count,
               my_sentence_info.id, my_sentence_info.msg_type, my_sentence_info.payload_buffer);
    } else {
        printf("Failed to parse sentence 1: %d\n", result1);
    }
    printf("\n");

    printf("--- Test Case 2: Second fragment (state 2 -> 3) ---\n");
    const char *sentence2 = "!AAAAA,2,2,123,PAYLOAD_PART2,1*FF"; // count=2, total=2, id=123, payload="PAYLOAD_PART2"
    int result2 = parse_sentence(sentence2, &my_sentence_info);
    if (result2 == 0) {
        printf("Parsed sentence 2 successfully.\nState: %d, Count: %d, Total: %d, ID: %d, MsgType: %d, Payload: '%s'\n",
               my_sentence_info.state, my_sentence_info.count, my_sentence_info.total_count,
               my_sentence_info.id, my_sentence_info.msg_type, my_sentence_info.payload_buffer);
    } else {
        printf("Failed to parse sentence 2: %d\n", result2);
    }
    printf("\n");

    printf("--- Test Case 3: Malformed sentence (no '!') ---\n");
    const char *bad_sentence1 = "NOT_A_SENTENCE";
    SentenceInfo bad_info1;
    reset_sentence_struct(&bad_info1);
    int bad_result1 = parse_sentence(bad_sentence1, &bad_info1);
    if (bad_result1 != 0) {
        printf("Correctly failed to parse bad sentence 1: %d\n", bad_result1);
    } else {
        printf("Unexpectedly parsed bad sentence 1.\n");
    }
    printf("\n");

    printf("--- Test Case 4: Incorrect prefix ---\n");
    const char *bad_sentence2 = "!BBBBB,1,1,123,PAYLOAD,1*FF";
    SentenceInfo bad_info2;
    reset_sentence_struct(&bad_info2);
    int bad_result2 = parse_sentence(bad_sentence2, &bad_info2);
    if (bad_result2 != 0) {
        printf("Correctly failed to parse bad sentence 2 (incorrect prefix): %d\n", bad_result2);
    } else {
        printf("Unexpectedly parsed bad sentence 2.\n");
    }
    printf("\n");

    printf("--- Test Case 5: Incorrect checksum ---\n");
    const char *bad_sentence3 = "!AAAAA,1,1,123,PAYLOAD,1*00"; // Checksum should be different
    SentenceInfo bad_info3;
    reset_sentence_struct(&bad_info3);
    int bad_result3 = parse_sentence(bad_sentence3, &bad_info3);
    if (bad_result3 != 0) {
        printf("Correctly failed to parse bad sentence 3 (incorrect checksum): %d\n", bad_result3);
    } else {
        printf("Unexpectedly parsed bad sentence 3.\n");
    }
    printf("\n");

    return 0;
}