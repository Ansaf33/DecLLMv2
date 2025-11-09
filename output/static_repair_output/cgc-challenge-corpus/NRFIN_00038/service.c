#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> // For write and ssize_t

// --- Type definitions from original snippet ---
typedef unsigned int uint;
typedef uint8_t byte;
typedef uint32_t undefined4; // Used for return values like 0, 0xffffffff, or sizes like 0x10.
// 'undefined' type is typically a placeholder for a byte array or generic pointer. Using uint8_t for buffers.

// --- Global variables ---
static unsigned int cur_0 = 0; // Used in login function for rotating algorithms.
static uint32_t global_token_counter = 0; // For make_token's "counter=" field.

// --- Dummy/Stub Functions (to make code compilable) ---

// bin_to_hex: Converts binary data to a hexadecimal string.
// dest should be large enough to hold 2*len characters + null terminator.
void bin_to_hex(char *dest, const void *src, size_t len) {
    const uint8_t *byte_src = (const uint8_t *)src;
    for (size_t i = 0; i < len; ++i) {
        sprintf(dest + (i * 2), "%02x", byte_src[i]);
    }
    dest[len * 2] = '\0';
}

// adler32ish: A placeholder for an Adler32-like checksum.
uint32_t adler32ish(const char *buf, size_t len) {
    // Simple placeholder: sum of bytes modulo a large prime. Not actual Adler32.
    uint32_t sum1 = 1;
    uint32_t sum2 = 0;
    for (size_t i = 0; i < len; ++i) {
        sum1 = (sum1 + (uint8_t)buf[i]) % 65521;
        sum2 = (sum2 + sum1) % 65521;
    }
    return (sum2 << 16) | sum1;
}

// md5: A placeholder for an MD5 hash function.
void md5(const char *buf, size_t len, uint8_t *output) {
    // Placeholder: fill with a repeating pattern or zeros. Actual MD5 would be complex.
    // MD5 produces 16 bytes (128 bits).
    (void)buf; // Suppress unused parameter warning
    (void)len; // Suppress unused parameter warning
    memset(output, 0xAA, 16); // Fill with dummy data
}

// md5_hmac: A placeholder for an HMAC-MD5 function.
void md5_hmac(const char *key, size_t keylen, const char *data, size_t datalen, uint8_t *output) {
    // Placeholder: fill with a repeating pattern or zeros.
    (void)key; (void)keylen; (void)data; (void)datalen;
    memset(output, 0xBB, 16); // Fill with dummy data
}

// read_line: Reads a line from a file descriptor. Allocates memory.
// Returns number of bytes read, or -1 on error.
ssize_t read_line(int fd, char **lineptr) {
    FILE *stream = fd == 0 ? stdin : NULL; // Assuming fd 0 is stdin
    if (!stream) {
        fprintf(stderr, "read_line only supports stdin (fd 0) in this stub.\n");
        return -1;
    }

    size_t len = 0;
    ssize_t bytes_read = getline(lineptr, &len, stream);
    return bytes_read;
}

// write_all: Writes all bytes from a buffer to a file descriptor.
// Returns total bytes written, or -1 on error.
ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    const uint8_t *ptr = (const uint8_t *)buf;
    while (total_written < count) {
        ssize_t written = write(fd, ptr + total_written, count - total_written);
        if (written == -1) {
            perror("write_all error");
            return -1;
        }
        total_written += written;
    }
    return total_written;
}

// hex_to_uint: Converts a hexadecimal string to an unsigned integer.
uint32_t hex_to_uint(const char *hex_str) {
    uint32_t val;
    sscanf(hex_str, "%x", &val);
    return val;
}

// _terminate: Placeholder for program termination.
void _terminate(void) {
    exit(0);
}

// --- Function pointer types for command dispatch and algorithm dispatch ---
// For login/greet/quit commands
typedef uint32_t (*command_handler_func)(char *);

// For xor_sig/adler32_sig/md5_sig/md5_hmac_sig functions in greet
// Using a common signature that can be cast to specific ones or handles void* for output
typedef uint32_t (*sig_algo_func)(char *, int, void *);

// For xor_login/adler32_login/md5_login/md5_hmac_login functions in login
typedef uint32_t (*login_algo_func)(char *);


// Function: make_token
char * make_token(char *param_1) {
  // `local_18` in original snippet was `calloc(4,...)` but never initialized before being passed to `bin_to_hex`
  // as `&local_18`. This suggests `local_18` was meant to be a `uint32_t` on the stack, not a heap pointer.
  // Given `greet` parses a counter, it seems `make_token` should include a counter.
  // We'll use a `static` global counter for this purpose, incrementing it each time a token is made.
  // Total length calculation:
  // "counter=" (8) + hex_counter (8) + "|" (1) + "name=" (5) + param_1_len + null (1)
  // `bin_to_hex` uses `sizeof(uint32_t)` which is 4 bytes, so 8 hex chars.

  size_t param_1_len = strlen(param_1);
  size_t required_len = strlen("counter=") + (sizeof(uint32_t) * 2) + strlen("|name=") + param_1_len + 1;

  char *token_buffer = (char *)calloc(required_len, sizeof(char));
  if (token_buffer == NULL) {
    return NULL;
  }

  strcpy(token_buffer, "counter=");
  bin_to_hex(token_buffer + strlen(token_buffer), &global_token_counter, sizeof(uint32_t));
  strcat(token_buffer, "|name=");
  strcat(token_buffer, param_1);

  global_token_counter++; // Increment for the next token

  return token_buffer;
}

// Function: xor_sig
uint32_t xor_sig(char *data_in, unsigned int len, char *data_out) {
  const char *secret = "s00pEr5eCretsAUc3"; // 0x11 = 17 bytes long
  size_t secret_len = strlen(secret);

  if (secret_len < len) { // Cap XOR length at secret length
    len = secret_len;
  }
  if (strlen(data_in) < len) { // Cap XOR length at input data length
      len = strlen(data_in);
  }

  for (unsigned int i = 0; i < len; i++) {
    data_out[i] = data_in[i] ^ secret[i];
  }
  return len; // Return actual length XORed
}

// Function: xor_login
undefined4 xor_login(char *param_1) {
  char *token_ptr;
  size_t token_len;
  size_t max_xor_len = strlen("s00pEr5eCretsAUc3"); // 0x11 = 17 bytes
  size_t total_len;
  char *reallocated_token;
  uint8_t xor_output_buffer[max_xor_len]; // Buffer to hold XORed signature

  token_ptr = make_token(param_1);
  if (token_ptr == NULL) {
    return 0xffffffff;
  }

  token_len = strlen(token_ptr);
  // token_len + '|' + hex_sig (max_xor_len * 2 chars) + '\n' + '\0'
  total_len = token_len + 1 + (max_xor_len * 2) + 1 + 1;

  reallocated_token = (char *)realloc(token_ptr, total_len);
  if (reallocated_token == NULL) {
    free(token_ptr);
    return 0xffffffff;
  }

  // Calculate XOR signature
  unsigned int xor_output_len = xor_sig(reallocated_token, token_len, (char*)xor_output_buffer);
  if (xor_output_len == 0) { // xor_sig returns the length XORed, 0 would be an error
    free(reallocated_token);
    return 0xffffffff;
  }

  // Append separator and hex signature
  reallocated_token[token_len] = '|';
  bin_to_hex(reallocated_token + token_len + 1, xor_output_buffer, xor_output_len);

  // Append newline and null terminator
  reallocated_token[total_len - 2] = '\n'; // Newline before null terminator
  reallocated_token[total_len - 1] = '\0'; // Ensure null termination

  // Write to stdout (fd 1)
  ssize_t bytes_written = write_all(1, reallocated_token, total_len - 1); // Exclude null terminator from write
  if (bytes_written != (ssize_t)(total_len - 1)) {
    return 0xffffffff;
  }

  // Clean up sensitive data
  memset(xor_output_buffer, 0, sizeof(xor_output_buffer));
  memset(reallocated_token, 0, total_len);
  free(reallocated_token);

  return 0;
}

// Function: adler32_sig
uint32_t adler32_sig(char *param_1, int param_2, uint32_t *param_3_output) {
  char *temp_buffer;
  uint32_t hash_val;
  size_t secret_len = strlen("s00pEr5eCretsAUc3"); // 0x11
  size_t buffer_size = param_2 + secret_len + 1; // param_2 is length of param_1

  temp_buffer = (char *)calloc(buffer_size, sizeof(char));
  if (temp_buffer == NULL) {
    return 0; // Return 0 for error
  }

  strcpy(temp_buffer, "s00pEr5eCretsAUc3");
  strcat(temp_buffer, param_1);

  hash_val = adler32ish(temp_buffer, param_2 + secret_len);
  *param_3_output = hash_val;

  memset(temp_buffer, 0, buffer_size);
  free(temp_buffer);

  return sizeof(uint32_t); // Adler32 hash is 4 bytes
}

// Function: adler32_login
undefined4 adler32_login(char *param_1) {
  char *token_ptr;
  size_t token_len;
  size_t total_len; // token_len + '|' + hex_sig (8 chars) + '\n' + '\0'
  char *reallocated_token;
  uint32_t adler_hash; // Buffer for Adler32 hash
  uint32_t adler_hash_len; // Will be 4

  token_ptr = make_token(param_1);
  if (token_ptr == NULL) {
    return 0xffffffff;
  }

  token_len = strlen(token_ptr);
  adler_hash_len = adler32_sig(token_ptr, token_len, &adler_hash);
  if (adler_hash_len == 0) { // adler32_sig returns 0 on error
    free(token_ptr);
    return 0xffffffff;
  }

  // token_len + '|' + hex_sig (adler_hash_len * 2 chars) + '\n' + '\0'
  total_len = token_len + 1 + (adler_hash_len * 2) + 1 + 1;

  reallocated_token = (char *)realloc(token_ptr, total_len);
  if (reallocated_token == NULL) {
    free(token_ptr);
    return 0xffffffff;
  }

  reallocated_token[token_len] = '|';
  bin_to_hex(reallocated_token + token_len + 1, &adler_hash, adler_hash_len);

  reallocated_token[total_len - 2] = '\n';
  reallocated_token[total_len - 1] = '\0';

  ssize_t bytes_written = write_all(1, reallocated_token, total_len - 1);
  if (bytes_written != (ssize_t)(total_len - 1)) {
    return 0xffffffff;
  }

  // Clean up sensitive data
  memset(&adler_hash, 0, sizeof(adler_hash)); // Zero out the hash
  memset(reallocated_token, 0, total_len);
  free(reallocated_token);

  return 0;
}

// Function: md5_sig
uint32_t md5_sig(char *param_1, int param_2, uint8_t *param_3_output) {
  char *temp_buffer;
  size_t secret_len = strlen("s00pEr5eCretsAUc3"); // 0x11
  size_t buffer_size = param_2 + secret_len + 1;

  temp_buffer = (char *)calloc(buffer_size, sizeof(char));
  if (temp_buffer == NULL) {
    return 0; // Return 0 for error
  }

  strcpy(temp_buffer, "s00pEr5eCretsAUc3");
  strcat(temp_buffer, param_1);

  md5(temp_buffer, param_2 + secret_len, param_3_output);

  memset(temp_buffer, 0, buffer_size);
  free(temp_buffer);

  return 16; // MD5 hash is 16 bytes
}

// Function: md5_login
undefined4 md5_login(char *param_1) {
  char *token_ptr;
  size_t token_len;
  size_t total_len; // token_len + '|' + hex_sig (32 chars) + '\n' + '\0'
  char *reallocated_token;
  uint8_t md5_hash_buffer[16]; // MD5 produces 16 bytes
  uint32_t md5_hash_len; // Will be 16

  token_ptr = make_token(param_1);
  if (token_ptr == NULL) {
    return 0xffffffff;
  }

  token_len = strlen(token_ptr);
  md5_hash_len = md5_sig(token_ptr, token_len, md5_hash_buffer);
  if (md5_hash_len == 0) { // md5_sig returns 0 on error
    free(token_ptr);
    return 0xffffffff;
  }

  // token_len + '|' + hex_sig (md5_hash_len * 2 chars) + '\n' + '\0'
  total_len = token_len + 1 + (md5_hash_len * 2) + 1 + 1;

  reallocated_token = (char *)realloc(token_ptr, total_len);
  if (reallocated_token == NULL) {
    free(token_ptr);
    return 0xffffffff;
  }

  reallocated_token[token_len] = '|';
  bin_to_hex(reallocated_token + token_len + 1, md5_hash_buffer, md5_hash_len);

  reallocated_token[total_len - 2] = '\n';
  reallocated_token[total_len - 1] = '\0';

  ssize_t bytes_written = write_all(1, reallocated_token, total_len - 1);
  if (bytes_written != (ssize_t)(total_len - 1)) {
    return 0xffffffff;
  }

  // Clean up sensitive data
  memset(md5_hash_buffer, 0, sizeof(md5_hash_buffer));
  memset(reallocated_token, 0, total_len);
  free(reallocated_token);

  return 0;
}

// Function: md5_hmac_sig
uint32_t md5_hmac_sig(char *param_1_data, int param_2_datalen, uint8_t *param_3_output) {
  const char *hmac_key = "s00pEr5eCretsAUc3";
  size_t hmac_key_len = strlen(hmac_key); // 0x11 = 17 bytes

  md5_hmac(hmac_key, hmac_key_len, param_1_data, param_2_datalen, param_3_output);

  return 16; // HMAC-MD5 hash is 16 bytes
}

// Function: md5_hmac_login
undefined4 md5_hmac_login(char *param_1) {
  char *token_ptr;
  size_t token_len;
  size_t total_len; // token_len + '|' + hex_sig (32 chars) + '\n' + '\0'
  char *reallocated_token;
  uint8_t hmac_hash_buffer[16]; // HMAC-MD5 produces 16 bytes
  uint32_t hmac_hash_len; // Will be 16

  token_ptr = make_token(param_1);
  if (token_ptr == NULL) {
    return 0xffffffff;
  }

  token_len = strlen(token_ptr);
  hmac_hash_len = md5_hmac_sig(token_ptr, token_len, hmac_hash_buffer);
  if (hmac_hash_len == 0) { // md5_hmac_sig returns 0 on error (though stub always returns 16)
    free(token_ptr);
    return 0xffffffff;
  }

  // token_len + '|' + hex_sig (hmac_hash_len * 2 chars) + '\n' + '\0'
  total_len = token_len + 1 + (hmac_hash_len * 2) + 1 + 1;

  reallocated_token = (char *)realloc(token_ptr, total_len);
  if (reallocated_token == NULL) {
    free(token_ptr);
    return 0xffffffff;
  }

  reallocated_token[token_len] = '|';
  bin_to_hex(reallocated_token + token_len + 1, hmac_hash_buffer, hmac_hash_len);

  reallocated_token[total_len - 2] = '\n';
  reallocated_token[total_len - 1] = '\0';

  ssize_t bytes_written = write_all(1, reallocated_token, total_len - 1);
  if (bytes_written != (ssize_t)(total_len - 1)) {
    return 0xffffffff;
  }

  // Clean up sensitive data
  memset(hmac_hash_buffer, 0, sizeof(hmac_hash_buffer));
  memset(reallocated_token, 0, total_len);
  free(reallocated_token);

  return 0;
}

// Function: login
undefined4 login(char *param_1) {
  login_algo_func login_algorithms[] = {
    xor_login,
    adler32_login,
    md5_login,
    md5_hmac_login // Added md5_hmac_login
  };
  const unsigned int num_login_algos = sizeof(login_algorithms) / sizeof(login_algorithms[0]);

  if (strlen(param_1) >= strlen("s00pEr5eCretsAUc3")) { // 0x11 = 17
    // Original condition was `if (sVar1 < 0x11)` for `strlen(param_1)`.
    // Assuming this means the username must be shorter than the secret length.
    return 0xffffffff;
  }

  uint32_t result = login_algorithms[cur_0](param_1);

  cur_0 = (cur_0 + 1) % num_login_algos; // Cycle through algorithms

  return result;
}

// Function: greet
undefined4 greet(char *param_1) {
  // `sig_algo_func` is `uint32_t (*)(char*, int, void*)`
  sig_algo_func sig_algorithms[] = {
    (sig_algo_func)xor_sig, // Cast to common signature
    (sig_algo_func)adler32_sig,
    (sig_algo_func)md5_sig,
    (sig_algo_func)md5_hmac_sig // Added md5_hmac_sig
  };
  const unsigned int num_sig_algos = sizeof(sig_algorithms) / sizeof(sig_algorithms[0]);

  char *signature_start = strrchr(param_1, '|');
  if (signature_start == NULL) {
    return 0xffffffff;
  }

  *signature_start = '\0'; // Null-terminate token part
  char *received_hex_signature = signature_start + 1;

  uint8_t calculated_signature_buffer[16]; // Max size for MD5/HMAC-MD5 (16 bytes)
  memset(calculated_signature_buffer, 0, sizeof(calculated_signature_buffer));

  int signature_match = 0;
  for (unsigned int i = 0; i < num_sig_algos; i++) {
    size_t token_len = strlen(param_1);
    uint32_t calculated_sig_len = sig_algorithms[i](param_1, token_len, calculated_signature_buffer);

    char hex_output_buffer[33]; // Max 16 bytes * 2 hex chars + null terminator
    bin_to_hex(hex_output_buffer, calculated_signature_buffer, calculated_sig_len);

    // Compare received signature with calculated one
    if (strncmp(received_hex_signature, hex_output_buffer, calculated_sig_len * 2) == 0) {
      signature_match = 1;
      break;
    }
  }

  if (signature_match == 0) {
    return 0xffffffff;
  }

  // Signature matched, now parse token data
  char name_buffer[18]; // Max 17 chars + null for "s00pEr5eCretsAUc3"
  uint32_t counter_value = 0;
  int name_found = 0;
  int counter_found = 0;

  // Use a copy of param_1 for strtok because strtok modifies the string.
  // param_1 has already been modified to be the token part (before '|').
  char *token_part_copy = strdup(param_1);
  if (token_part_copy == NULL) {
      return 0xffffffff;
  }

  char *current_field = strtok(token_part_copy, "|");

  while (current_field != NULL) {
    if (strncmp(current_field, "name=", 5) == 0) {
      strncpy(name_buffer, current_field + 5, sizeof(name_buffer) - 1);
      name_buffer[sizeof(name_buffer) - 1] = '\0';
      name_found = 1;
    } else if (strncmp(current_field, "counter=", 8) == 0) {
      // Counter is 8 hex chars (4 bytes)
      if (strlen(current_field + 8) == 8) {
        counter_value = hex_to_uint(current_field + 8);
        counter_found = 1;
      }
    }
    current_field = strtok(NULL, "|");
  }
  free(token_part_copy); // Free the duplicated string

  if (!name_found || !counter_found) {
    return 0xffffffff;
  }

  char response_buffer[256]; // Sufficiently large for "Hello NAME, we've seen you N times!\n"
  memset(response_buffer, 0, sizeof(response_buffer));

  counter_value++; // Increment counter for display

  sprintf(response_buffer, "Hello %s, we\'ve seen you %u times!\n", name_buffer, counter_value);

  size_t response_len = strlen(response_buffer);
  ssize_t bytes_written = write_all(1, response_buffer, response_len);

  if (bytes_written != (ssize_t)response_len) {
    return 0xffffffff;
  }

  return 0;
}

// Function: quit
undefined4 quit(void) {
  _terminate(); // Call the termination function (exit)
  return 0; // Should not be reached
}

// Function: main
int main(void) {
  // Command table for main loop
  struct {
    const char *command_str;
    command_handler_func handler;
  } command_table[] = {
    {"login ", login},
    {"greet ", greet},
    {"quit", quit}
  };
  const unsigned int num_commands = sizeof(command_table) / sizeof(command_table[0]);

  while (1) {
    char *line_buffer = NULL;
    ssize_t bytes_read = read_line(0, &line_buffer);

    if (bytes_read < 0) {
      if (line_buffer) free(line_buffer);
      return 1; // Error reading input
    }
    if (bytes_read == 0) { // EOF
        if (line_buffer) free(line_buffer);
        break;
    }

    // Remove trailing newline if present
    if (bytes_read > 0 && line_buffer[bytes_read - 1] == '\n') {
      line_buffer[bytes_read - 1] = '\0';
      bytes_read--; // Adjust length
    } else {
      line_buffer[bytes_read] = '\0'; // Ensure null termination
    }

    uint32_t handler_result = 0xffffffff; // Default to error
    int command_found = 0;

    for (unsigned int i = 0; i < num_commands; i++) {
      size_t cmd_len = strlen(command_table[i].command_str);
      if (strncmp(line_buffer, command_table[i].command_str, cmd_len) == 0) {
        // Pass the argument part of the line to the handler
        handler_result = command_table[i].handler(line_buffer + cmd_len);
        command_found = 1;
        break;
      }
    }

    // Print command execution status
    char status_hex_buffer[sizeof(uint32_t) * 2 + 2]; // 8 hex chars + newline + null
    bin_to_hex(status_hex_buffer, &handler_result, sizeof(uint32_t));
    strcat(status_hex_buffer, "\n"); // Add newline

    if (write_all(1, status_hex_buffer, strlen(status_hex_buffer)) != (ssize_t)strlen(status_hex_buffer)) {
      if (line_buffer) free(line_buffer);
      return 1; // Error writing status
    }

    // Clean up line buffer
    memset(line_buffer, 0, bytes_read + 1); // Clear sensitive input
    free(line_buffer);
  }

  return 0;
}