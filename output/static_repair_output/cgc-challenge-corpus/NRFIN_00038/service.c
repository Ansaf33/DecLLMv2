#include <stdio.h>    // For snprintf, sscanf
#include <stdlib.h>   // For calloc, free, realloc, exit
#include <string.h>   // For strlen, strcpy, strcat, strncmp, strrchr, strtok, memset
#include <unistd.h>   // For write, read
#include <stdint.h>   // For uint32_t, intptr_t, ssize_t

// --- Decompiler Type Mappings ---
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned char undefined;
typedef unsigned int undefined4; // Represents a 4-byte unsigned integer

// --- Global Variables ---
static int cur_0 = 0; // Global counter for cycling through login methods

// --- Dummy/Helper Function Prototypes (replace with actual implementations if available) ---

// bin_to_hex: converts a binary buffer to its hexadecimal string representation
void bin_to_hex(char *dest, const void *src, size_t len) {
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < len; i++) {
        sprintf(dest + (i * 2), "%02x", s[i]);
    }
    dest[len * 2] = '\0';
}

// hex_to_uint: converts a hexadecimal string to an unsigned integer
unsigned int hex_to_uint(const char *hex_str) {
    unsigned int val;
    sscanf(hex_str, "%x", &val);
    return val;
}

// adler32ish: a dummy Adler32-like checksum function
unsigned int adler32ish(const char *buf, size_t len) {
    unsigned int a = 1, b = 0;
    for (size_t i = 0; i < len; i++) {
        a = (a + (unsigned char)buf[i]) % 65521;
        b = (b + a) % 65521;
    }
    return (b << 16) | a;
}

// md5: a dummy MD5 hash function
void md5(const char *buf, size_t len, unsigned char *output) {
    // For compilation, just fill with some dummy data
    for (size_t i = 0; i < 16; i++) {
        output[i] = (unsigned char)(i + len + (buf ? buf[0] : 0)); // Dummy hash
    }
}

// md5_hmac: a dummy MD5 HMAC function
void md5_hmac(const char *key, size_t keylen, const char *data, size_t datalen, unsigned char *output) {
    // For compilation, just fill with some dummy data
    for (size_t i = 0; i < 16; i++) {
        output[i] = (unsigned char)(i + keylen + datalen + (key ? key[0] : 0) + (data ? data[0] : 0)); // Dummy HMAC
    }
}

// write_all: wrapper for write to ensure all bytes are written
ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    while (total_written < count) {
        ssize_t bytes_written = write(fd, (const char *)buf + total_written, count - total_written);
        if (bytes_written < 0) {
            return -1; // Error
        }
        total_written += bytes_written;
    }
    return total_written;
}

// read_line: reads a line from fd, allocates buffer, returns length
ssize_t read_line(int fd, char **buf) {
    size_t capacity = 128;
    size_t len = 0;
    char *line = (char *)malloc(capacity);
    if (!line) return -1;

    while (1) {
        if (len + 1 >= capacity) { // +1 for null terminator
            capacity *= 2;
            char *new_line = (char *)realloc(line, capacity);
            if (!new_line) {
                free(line);
                return -1;
            }
            line = new_line;
        }

        ssize_t bytes_read = read(fd, line + len, 1);
        if (bytes_read <= 0) { // EOF or error
            if (bytes_read == 0 && len > 0) { // EOF, but some data read
                line[len] = '\0';
                *buf = line;
                return len;
            }
            free(line);
            return -1; // Error or empty EOF
        }
        len += bytes_read;

        if (line[len - 1] == '\n') {
            line[len] = '\0';
            *buf = line;
            return len;
        }
    }
}

// --- Function Declarations ---
char * make_token(char *param_1);
uint xor_sig(char *param_1, uint param_2, unsigned char *param_3);
undefined4 xor_login(char *username);
unsigned int adler32_sig(char *param_1, int param_2, unsigned int *param_3);
undefined4 adler32_login(char *username);
unsigned int md5_sig(char *param_1, int param_2, unsigned char *param_3);
undefined4 md5_login(char *username);
unsigned int md5_hmac_sig(char *param_1, int param_2, unsigned char *param_3);
undefined4 md5_hmac_login(char *username);
undefined4 login(char *param_1);
undefined4 greet(char *param_1);
undefined4 quit(void);

// --- Function Implementations ---

// Function: make_token
char * make_token(char *param_1) {
  size_t total_len;
  char *result_token;
  
  unsigned int counter_val = 1; // Initial counter value
  size_t counter_prefix_len = strlen("counter=");
  size_t name_prefix_len = strlen("|name=");
  size_t param_1_len = strlen(param_1);

  // Calculate total length:
  // "counter=" (8) + "00000001" (8 chars for 4 bytes hex) + "|name=" (6) + param_1_len + null terminator (1)
  total_len = counter_prefix_len + (sizeof(unsigned int) * 2) + name_prefix_len + param_1_len + 1;

  result_token = (char *)calloc(total_len, sizeof(char));
  if (result_token == NULL) {
    return NULL;
  }
  
  strcpy(result_token, "counter=");
  bin_to_hex(result_token + counter_prefix_len, &counter_val, sizeof(unsigned int));
  strcat(result_token, "|name=");
  strcat(result_token, param_1);
  
  return result_token;
}

// Function: xor_sig
uint xor_sig(char *src_buf, uint len, unsigned char *dest_buf) {
  if (0x11 < len) { // 0x11 is 17, the length of the XOR key
    len = 0x11;
  }
  for (uint i = 0; i < len; i++) {
    dest_buf[i] = (unsigned char)src_buf[i] ^ (unsigned char)"s00pEr5eCretsAUc3"[i];
  }
  return len;
}

// Function: xor_login
undefined4 xor_login(char *username) {
  char *token = make_token(username);
  if (token == NULL) {
    return 0xffffffff;
  }

  size_t token_len = strlen(token);
  // Max XOR signature length is 17 bytes (0x11). Each byte is 2 hex chars.
  // Add 1 for '|', 1 for '\n', and 1 for null terminator.
  size_t new_size = token_len + (0x11 * 2) + 2 + 1;

  char *realloc_token = (char *)realloc(token, new_size);
  if (realloc_token == NULL) {
    free(token);
    return 0xffffffff;
  }
  token = realloc_token;

  unsigned char xor_signature_bytes[0x11]; // Buffer for 17-byte signature
  uint sig_len_bytes = xor_sig(token, token_len, xor_signature_bytes);

  if (sig_len_bytes == 0) {
    free(token);
    return 0xffffffff;
  }

  token[token_len] = '|';
  bin_to_hex(token + token_len + 1, xor_signature_bytes, sig_len_bytes);
  token[new_size - 2] = '\n';
  token[new_size - 1] = '\0'; // Ensure null termination

  ssize_t bytes_written = write_all(1, token, new_size - 1); // Exclude null terminator for write

  memset(xor_signature_bytes, 0, sizeof(xor_signature_bytes)); // Clear sensitive signature
  memset(token, 0, new_size); // Clear sensitive token data
  free(token);

  if (bytes_written != (ssize_t)(new_size - 1)) {
    return 0xffffffff;
  }
  return 0;
}

// Function: adler32_sig
unsigned int adler32_sig(char *param_1, int param_2, unsigned int *param_3) {
  // Size needed: "s00pEr5eCretsAUc3" (17) + param_2 (data length) + 1 (null terminator)
  size_t total_len = (size_t)param_2 + strlen("s00pEr5eCretsAUc3") + 1;
  char *temp_buffer = (char *)calloc(total_len, sizeof(char));
  if (temp_buffer == NULL) {
    return 0;
  }

  strcpy(temp_buffer, "s00pEr5eCretsAUc3");
  strcat(temp_buffer, param_1);

  *param_3 = adler32ish(temp_buffer, total_len - 1); // Calculate Adler32, exclude null terminator

  memset(temp_buffer, 0, total_len); // Clear sensitive data
  free(temp_buffer);
  return sizeof(unsigned int); // Return 4 (size of Adler32 hash)
}

// Function: adler32_login
undefined4 adler32_login(char *username) {
  char *token = make_token(username);
  if (token == NULL) {
    return 0xffffffff;
  }

  size_t token_len = strlen(token);
  // Adler32 is 4 bytes. Each byte is 2 hex chars (8 total).
  // Add 1 for '|', 1 for '\n', and 1 for null terminator.
  size_t new_size = token_len + (sizeof(unsigned int) * 2) + 2 + 1;

  char *realloc_token = (char *)realloc(token, new_size);
  if (realloc_token == NULL) {
    free(token);
    return 0xffffffff;
  }
  token = realloc_token;

  unsigned int adler_signature_val; // For 4-byte Adler32 signature
  uint sig_len_bytes = adler32_sig(token, token_len, &adler_signature_val);

  if (sig_len_bytes == 0) {
    free(token);
    return 0xffffffff;
  }

  token[token_len] = '|';
  bin_to_hex(token + token_len + 1, &adler_signature_val, sig_len_bytes);
  token[new_size - 2] = '\n';
  token[new_size - 1] = '\0'; // Ensure null termination

  ssize_t bytes_written = write_all(1, token, new_size - 1);

  memset(&adler_signature_val, 0, sizeof(adler_signature_val)); // Clear sensitive signature
  memset(token, 0, new_size); // Clear sensitive token data
  free(token);

  if (bytes_written != (ssize_t)(new_size - 1)) {
    return 0xffffffff;
  }
  return 0;
}

// Function: md5_sig
unsigned int md5_sig(char *param_1, int param_2, unsigned char *param_3) {
  // Size needed: "s00pEr5eCretsAUc3" (17) + param_2 (data length) + 1 (null terminator)
  size_t total_len = (size_t)param_2 + strlen("s00pEr5eCretsAUc3") + 1;
  char *temp_buffer = (char *)calloc(total_len, sizeof(char));
  if (temp_buffer == NULL) {
    return 0;
  }

  strcpy(temp_buffer, "s00pEr5eCretsAUc3");
  strcat(temp_buffer, param_1);

  md5(temp_buffer, total_len - 1, param_3);

  memset(temp_buffer, 0, total_len);
  free(temp_buffer);
  return 0x10; // Return 16 (size of MD5 hash in bytes)
}

// Function: md5_login
undefined4 md5_login(char *username) {
  char *token = make_token(username);
  if (token == NULL) {
    return 0xffffffff;
  }

  size_t token_len = strlen(token);
  // MD5 is 16 bytes (0x10). Each byte is 2 hex chars (32 total).
  // Add 1 for '|', 1 for '\n', and 1 for null terminator.
  size_t new_size = token_len + (0x10 * 2) + 2 + 1;

  char *realloc_token = (char *)realloc(token, new_size);
  if (realloc_token == NULL) {
    free(token);
    return 0xffffffff;
  }
  token = realloc_token;

  unsigned char md5_signature_bytes[0x10]; // For 16-byte MD5 signature
  uint sig_len_bytes = md5_sig(token, token_len, md5_signature_bytes);

  if (sig_len_bytes == 0) {
    free(token);
    return 0xffffffff;
  }

  token[token_len] = '|';
  bin_to_hex(token + token_len + 1, md5_signature_bytes, sig_len_bytes);
  token[new_size - 2] = '\n';
  token[new_size - 1] = '\0'; // Ensure null termination

  ssize_t bytes_written = write_all(1, token, new_size - 1);

  memset(md5_signature_bytes, 0, sizeof(md5_signature_bytes)); // Clear sensitive signature
  memset(token, 0, new_size); // Clear sensitive token data
  free(token);

  if (bytes_written != (ssize_t)(new_size - 1)) {
    return 0xffffffff;
  }
  return 0;
}

// Function: md5_hmac_sig
unsigned int md5_hmac_sig(char *param_1, int param_2, unsigned char *param_3) {
  md5_hmac("s00pEr5eCretsAUc3", strlen("s00pEr5eCretsAUc3"), param_1, (size_t)param_2, param_3);
  return 0x10; // Return 16 (size of MD5 HMAC hash in bytes)
}

// Function: md5_hmac_login
undefined4 md5_hmac_login(char *username) {
  char *token = make_token(username);
  if (token == NULL) {
    return 0xffffffff;
  }

  size_t token_len = strlen(token);
  // MD5 HMAC is 16 bytes (0x10). Each byte is 2 hex chars (32 total).
  // Add 1 for '|', 1 for '\n', and 1 for null terminator.
  size_t new_size = token_len + (0x10 * 2) + 2 + 1;

  char *realloc_token = (char *)realloc(token, new_size);
  if (realloc_token == NULL) {
    free(token);
    return 0xffffffff;
  }
  token = realloc_token;

  unsigned char md5_hmac_signature_bytes[0x10]; // For 16-byte MD5 HMAC signature
  uint sig_len_bytes = md5_hmac_sig(token, token_len, md5_hmac_signature_bytes);

  if (sig_len_bytes == 0) {
    free(token);
    return 0xffffffff;
  }

  token[token_len] = '|';
  bin_to_hex(token + token_len + 1, md5_hmac_signature_bytes, sig_len_bytes);
  token[new_size - 2] = '\n';
  token[new_size - 1] = '\0'; // Ensure null termination

  ssize_t bytes_written = write_all(1, token, new_size - 1);

  memset(md5_hmac_signature_bytes, 0, sizeof(md5_hmac_signature_bytes)); // Clear sensitive signature
  memset(token, 0, new_size); // Clear sensitive token data
  free(token);

  if (bytes_written != (ssize_t)(new_size - 1)) {
    return 0xffffffff;
  }
  return 0;
}

// Function: login
undefined4 login(char *param_1) {
  typedef unsigned int (*login_func_ptr)(char *);
  login_func_ptr login_methods[] = {
    xor_login,
    adler32_login,
    md5_login,
    md5_hmac_login // Assuming this is the fourth login method
  };
  const int NUM_LOGIN_METHODS = sizeof(login_methods) / sizeof(login_methods[0]);

  if (strlen(param_1) >= 0x11) { // Max username length 16 (0x10), so check for >= 17 (0x11)
    return 0xffffffff;
  }

  unsigned int result = login_methods[cur_0](param_1);
  cur_0 = (cur_0 + 1) % NUM_LOGIN_METHODS; // Cycle through methods

  return result;
}

// Function: greet
undefined4 greet(char *param_1) {
  typedef unsigned int (*sig_func_ptr)(char *, int, unsigned char *);
  sig_func_ptr sig_methods[] = {
    (sig_func_ptr)xor_sig,
    (sig_func_ptr)adler32_sig,
    (sig_func_ptr)md5_sig
  };
  const int NUM_SIG_METHODS = sizeof(sig_methods) / sizeof(sig_methods[0]);

  char *pipe_char_ptr = strrchr(param_1, '|');
  if (pipe_char_ptr == NULL) {
    return 0xffffffff; // No '|' found
  }

  *pipe_char_ptr = '\0'; // Null-terminate the token part
  char *received_signature_hex = pipe_char_ptr + 1;

  char token_copy[256]; // Use a fixed-size buffer for token parsing
  strncpy(token_copy, param_1, sizeof(token_copy) - 1);
  token_copy[sizeof(token_copy) - 1] = '\0'; // Ensure null termination

  int signature_match = 0;
  unsigned char calculated_signature_bytes[0x10]; // Max size needed for MD5/HMAC (16 bytes)
  char calculated_signature_hex[0x10 * 2 + 1]; // Max 32 hex chars + null

  size_t token_len = strlen(param_1); // Length of the token part before the pipe

  for (int i = 0; i < NUM_SIG_METHODS; i++) {
    memset(calculated_signature_bytes, 0, sizeof(calculated_signature_bytes));
    memset(calculated_signature_hex, 0, sizeof(calculated_signature_hex));

    uint sig_len_bytes = sig_methods[i](param_1, token_len, calculated_signature_bytes);

    if (sig_len_bytes > 0) {
      bin_to_hex(calculated_signature_hex, calculated_signature_bytes, sig_len_bytes);
      if (strncmp(received_signature_hex, calculated_signature_hex, sig_len_bytes * 2) == 0) {
        signature_match = 1;
        break;
      }
    }
  }

  if (signature_match == 0) {
    return 0xffffffff; // Signature mismatch
  }

  // Parse token components
  char username_str[0x11]; // Max name length 16 + null
  memset(username_str, 0, sizeof(username_str));
  unsigned int counter_val = 0;
  int counter_found = 0;

  char *token_part = strtok(token_copy, "|");
  while (token_part != NULL) {
    if (strncmp(token_part, "name=", 5) == 0) {
      strncpy(username_str, token_part + 5, sizeof(username_str) - 1);
      username_str[sizeof(username_str) - 1] = '\0';
    } else if (strncmp(token_part, "counter=", 8) == 0) {
      char *hex_counter_str = token_part + 8;
      if (strlen(hex_counter_str) == (sizeof(unsigned int) * 2)) { // Expect 8 hex chars for 4 bytes
        counter_val = hex_to_uint(hex_counter_str);
        counter_found = 1;
      }
    }
    token_part = strtok(NULL, "|");
  }

  if (strlen(username_str) == 0 || !counter_found) {
    return 0xffffffff; // Missing name or counter
  }

  counter_val++; // Increment the counter

  char greeting_buffer[256]; // Sufficiently large buffer for greeting message
  snprintf(greeting_buffer, sizeof(greeting_buffer), "Hello %s, we've seen you %u times!\n", username_str, counter_val);

  ssize_t greeting_len = strlen(greeting_buffer);
  ssize_t bytes_written = write_all(1, greeting_buffer, greeting_len);

  if (bytes_written != greeting_len) {
    return 0xffffffff; // Error writing
  }

  return 0;
}

// Function: quit
undefined4 quit(void) {
  exit(0); // Terminate the program cleanly
  return 0; // Should not be reached
}

// Function: main
undefined4 main(void) {
  typedef unsigned int (*command_func_ptr)(char *);

  struct Command {
    const char *name;
    command_func_ptr func;
  };

  struct Command commands[] = {
    {"login ", login},
    {"greet ", greet},
    {"quit ", (command_func_ptr)quit}
  };
  const int NUM_COMMANDS = sizeof(commands) / sizeof(commands[0]);

  while (1) {
    char *input_line = NULL;
    ssize_t line_len = read_line(0, &input_line);

    if (line_len < 0) {
      if (input_line) free(input_line);
      return 0xffffffff;
    }

    if (line_len > 0 && input_line[line_len - 1] == '\n') {
      input_line[line_len - 1] = '\0';
      line_len--;
    }

    unsigned int command_result = 0xffffffff;

    for (int i = 0; i < NUM_COMMANDS; i++) {
      size_t command_name_len = strlen(commands[i].name);
      if (strncmp(input_line, commands[i].name, command_name_len) == 0) {
        command_result = commands[i].func(input_line + command_name_len);
        break;
      }
    }

    memset(input_line, 0, line_len + 1);
    free(input_line);
    input_line = NULL;

    char result_hex[9]; // 4 bytes = 8 hex chars + null terminator
    bin_to_hex(result_hex, &command_result, sizeof(command_result));
    strcat(result_hex, "\n");
    ssize_t bytes_written = write_all(1, result_hex, strlen(result_hex));

    if (bytes_written != (ssize_t)strlen(result_hex)) {
      return 0xffffffff;
    }
  }

  return 0;
}