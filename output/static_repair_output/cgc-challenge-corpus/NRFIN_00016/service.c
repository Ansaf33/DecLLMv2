#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // For mock_recv, close, etc.
#include <sys/types.h>   // For ssize_t, size_t
#include <math.h>        // For isnan

// Ghidra type aliases to standard C types
typedef unsigned char byte;
typedef unsigned int uint;

// Global variables (from snippet context)
char resp[256];             // Buffer for string responses, arbitrary size
unsigned int valvepos = 0;  // Global valve position
unsigned int current_temperature = 25; // Dummy for gett function

// Dummy global constants for admin_fp_login
double DAT_000130f0 = 100.0;
double DAT_000130f8 = 1.0;

// --- Mock/Helper Functions ---

// Program termination function
void _terminate(void) {
    fprintf(stderr, "Program terminated.\n");
    exit(1);
}

// Simplified sendline to just print to stdout
int sendline(const char *msg) {
    if (msg == NULL) {
        return -1;
    }
    printf("%s\n", msg);
    fflush(stdout);
    return 0; // Assume success
}

// uint2str: convert unsigned int to string
void uint2str(char *buf, size_t buf_len, unsigned int val) {
    snprintf(buf, buf_len, "%u", val);
}

// str2uint: convert string to unsigned int
unsigned int str2uint(const char *str) {
    return (unsigned int)strtoul(str, NULL, 10);
}

// startswith: check if string starts with prefix
int startswith(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Dummy cryptographic/hash functions (signatures adjusted to take char* and byte length)
long long additive(const char *data, size_t len_bytes) {
    // Dummy implementation: returns a fixed value
    return 0x1111111111111111; // Default to not matching
}

long long addxoradd(const char *data, size_t len_bytes) {
    // Dummy implementation: returns a fixed value
    return 0x2222222222222222; // Default to not matching
}

unsigned int crc32(const char *data, size_t len_bytes) {
    // Dummy implementation: returns a fixed value
    return 0x33333333; // Default to not matching
}

double fpadditive(const char *data, size_t len_bytes) {
    // Dummy implementation: returns a fixed value
    return 123.45; // Default value
}

void md5(const char *data, size_t len_bytes, unsigned char *output) {
    // Dummy MD5 implementation: fills output with zeros
    (void)data; // Suppress unused parameter warning
    (void)len_bytes; // Suppress unused parameter warning
    memset(output, 0, 16);
}

// Mock recv for main function to simulate network input from stdin
ssize_t mock_recv(int fd, void *buf, size_t len, int flags) {
    (void)fd;    // Suppress unused parameter warning
    (void)flags; // Suppress unused parameter warning
    printf("Enter input (max %zu bytes): ", len);
    if (fgets((char *)buf, len, stdin) != NULL) {
        // Remove newline if present
        size_t actual_len = strlen((char *)buf);
        if (actual_len > 0 && ((char *)buf)[actual_len - 1] == '\n') {
            ((char *)buf)[actual_len - 1] = '\0';
            actual_len--;
        }
        return actual_len;
    }
    return -1; // Error
}
// Replace original recv with mock_recv
#define recv mock_recv

// --- Original Functions (Fixed) ---

// Function: getv
char * getv(void) {
  uint2str(resp, sizeof(resp), valvepos);
  return resp;
}

// Function: setv
char * setv(const char *param_1) {
  // Assuming param_1 is a pointer to a packet, and param_1 + 4 points to a string
  valvepos = str2uint(param_1 + 4);
  return "Valve successfully modified.";
}

// Function: gett
char * gett(void) {
  if (random() != 0) {
    sendline("The essence of chaos.");
    _terminate();
  }
  // Assuming it should print a temperature value, using a dummy global
  uint2str(resp, sizeof(resp), current_temperature);
  return resp;
}

// Function: redacted
char * redacted(const char *param_1) {
  // Assuming param_1 + 4 is length, param_1 + 8 is data
  unsigned int copy_len = *(unsigned int *)(param_1 + 4);
  // Ensure copy_len does not exceed valvepos size to prevent buffer overflow
  if (copy_len > sizeof(valvepos)) {
      copy_len = sizeof(valvepos);
  }
  memcpy(&valvepos, param_1 + 8, copy_len);
  return "This is not the function you\'re looking for.";
}

// Function: admin_add_login
char * admin_add_login(const char *param_1) {
  unsigned char len_to_copy = *(unsigned char *)(param_1 + 4);
  char local_buffer[130]; // Matches original stack allocation size (local_9d, auStack_9c, uStack_21, local_1d)
  memset(local_buffer, 0, sizeof(local_buffer)); // Zero out the buffer
  if (len_to_copy > 0 && len_to_copy < sizeof(local_buffer)) {
      // Original copies from param_1 + 4, meaning the length byte itself is copied.
      memcpy(local_buffer, param_1 + 4, len_to_copy);
  }
  return "This is not the function you\'re looking for.";
}

// Function: admin_addxoradd_login
char * admin_addxoradd_login(const char *param_1) {
  char local_buffer[16]; // Matches original stack allocation size (local_11 and local_d)
  memset(local_buffer, 0, sizeof(local_buffer));
  if (startswith(param_1 + 4, "robots only")) {
    // Use strncpy to prevent buffer overflow, ensuring null termination
    strncpy(local_buffer, param_1 + 4, sizeof(local_buffer) - 1);
    local_buffer[sizeof(local_buffer) - 1] = '\0';
  }
  return "This is not the function you\'re looking for.";
}

// Function: admin_crc_login
char * admin_crc_login(const char *param_1) {
  char local_buffer[64]; // local_46
  unsigned char len_field = *(unsigned char *)(param_1 + 4); // The value at param_1 + 4
  unsigned char copy_len; // local_6

  if (len_field < 0x41) { // Check if the value at param_1 + 4 is less than 0x41
    copy_len = len_field - 4; // Calculate actual number of bytes to copy
    if (copy_len > 0 && copy_len < sizeof(local_buffer)) { // Ensure it doesn't overflow buffer
        // Copies 'copy_len' bytes starting from param_1 + 4.
        // This means the 'len_field' itself is part of the data being copied,
        // and the actual data starts at param_1 + 4.
        memcpy(local_buffer, param_1 + 4, copy_len);
    }
  }
  return "This is not the function you\'re looking for.";
}

// Function: admin_fp_login
char * admin_fp_login(const char *param_1) {
  long long buffer[10]; // auStack_6c
  double limit = *(double *)(param_1 + 0xc); // local_1c
  int i = 0; // local_8

  if (limit <= DAT_000130f0) {
    // Add bounds check for 'i' to prevent buffer overflow
    for (double current = 0.0; current < limit && i < sizeof(buffer) / sizeof(buffer[0]); current += DAT_000130f8) {
      buffer[i] = *(long long *)(param_1 + 4 + (unsigned int)i * 8); // Accessing param_1 + 4 + offset
      i++;
    }
  }
  return "This is not the function you\'re looking for.";
}

// Function: admin_md5_login
char * admin_md5_login(const char *param_1) {
  unsigned int buffer[32]; // auStack_88
  unsigned char len = *(unsigned char *)(param_1 + 9); // Length for the loop
  unsigned char i; // local_5

  // Add buffer bounds check for 'i'
  for (i = 0; i < len && i < sizeof(buffer) / sizeof(buffer[0]); i++) {
    buffer[i] = *(unsigned int *)(param_1 + 10 + (unsigned int)i * 4);
  }
  return "This is not the function you\'re looking for.";
}

// Function: process_pkt
char * process_pkt(const unsigned int *param_1) {
  char *result_str = "NaNaNaNaNaNaNaNaNaNaNaNaNaN watman!"; // Default return message

  // The first uint in param_1 is the command type. Data for checksums starts after it.
  const char *data_for_checksums_start = (const char *)(param_1 + 1);
  // The original code uses a fixed length of 0x80 for checksum functions.
  size_t data_len_for_checksums = 0x80; 
  
  long long val_additive = additive(data_for_checksums_start, data_len_for_checksums); // local_14
  
  // Expected MD5 hash values (reconstructed from local_44, local_40, etc. in little-endian byte order)
  unsigned char expected_md5_1[16] = {
      0x21, 0x23, 0x2f, 0x29, // From 0x292f2321
      0x7a, 0x57, 0xa5, 0xa7, // From 0xa7a5577a
      0x43, 0x89, 0x4a, 0x0e, // From 0xe4a8943
      0x4a, 0x80, 0x1f, 0xc3  // From 0xc31f804a
  };

  if (val_additive == 0x4242424242424242) {
    result_str = admin_add_login((const char *)param_1);
  } else {
    // Accessing long long at offset 0x21 * sizeof(unsigned int)
    long long val_packet_21 = *(long long *)((const char *)param_1 + 0x21 * sizeof(unsigned int));
    if (val_additive == val_packet_21) {
      long long val_addxoradd = addxoradd(data_for_checksums_start, data_len_for_checksums); // local_14 (reused)
      if (val_addxoradd == 0x2b992ddfa23249d6) {
        result_str = admin_addxoradd_login((const char *)param_1);
      } else {
        // Accessing long long at offset 0x23 * sizeof(unsigned int)
        long long val_packet_23 = *(long long *)((const char *)param_1 + 0x23 * sizeof(unsigned int));
        if (val_addxoradd == val_packet_23) {
          unsigned int val_crc32 = crc32(data_for_checksums_start, data_len_for_checksums); // local_18
          if (val_crc32 == 0x6f4e3700) {
            result_str = admin_crc_login((const char *)param_1);
          } else {
            unsigned int val_packet_27 = param_1[0x27]; // Accessing uint at index 0x27
            if (val_crc32 == val_packet_27) {
              double val_fpadditive = fpadditive(data_for_checksums_start, data_len_for_checksums); // local_24
              // Accessing double at offset 0x25 * sizeof(unsigned int)
              double val_packet_25 = *(double *)((const char *)param_1 + 0x25 * sizeof(unsigned int));
              // Check for NaN is important for floating point comparisons
              if (!isnan(val_packet_25) && (val_fpadditive != val_packet_25)) {
                result_str = "Invalid checksum.";
              } else {
                unsigned char md5_output[16]; // local_34
                // MD5 over 5 bytes from data_for_checksums_start
                md5(data_for_checksums_start, 5, md5_output);
                int iVar3 = memcmp(md5_output, expected_md5_1, 0x10);
                if (iVar3 == 0) {
                  result_str = admin_md5_login((const char *)param_1);
                } else {
                  // MD5 over 0x80 bytes from data_for_checksums_start
                  md5(data_for_checksums_start, data_len_for_checksums, md5_output);
                  // Compare with 16 bytes at offset 0x28 * sizeof(unsigned int)
                  iVar3 = memcmp(md5_output, (const char *)param_1 + 0x28 * sizeof(unsigned int), 0x10);
                  if (iVar3 == 0) {
                    uint uVar1 = *param_1; // The first uint in the packet is the command ID
                    if (uVar1 == 3) {
                      result_str = redacted((const char *)param_1);
                    } else if (uVar1 == 2) {
                      result_str = gett();
                    } else if (uVar1 == 0) {
                      result_str = getv();
                    } else if (uVar1 == 1) {
                      result_str = setv((const char *)param_1);
                    }
                  } else {
                    result_str = "Invalid checksum.";
                  }
                }
              }
            } else {
              result_str = "Invalid checksum.";
            }
          }
        } else {
          result_str = "Invalid checksum.";
        }
      }
    } else {
      result_str = "Invalid checksum.";
    }
  }
  return result_str;
}

// Function: main
int main(void) {
  // local_d8 is an array of 44 unsigned ints (176 bytes total)
  unsigned int packet_buffer[44];
  char *response_message;
  ssize_t bytes_received;

  // Initialize packet_buffer to zeros
  memset(packet_buffer, 0, sizeof(packet_buffer));

  // Simulating initial random check and terminate path
  if (random() != 0) {
    sendline("The essence of chaos.");
    _terminate();
  }

  // Call recv: fd=0 (stdin), buf=packet_buffer, len=0xb0 (176 bytes), flags=0
  bytes_received = recv(0, packet_buffer, 0xb0, 0);

  if (bytes_received < 0) {
    sendline("You didn\'t say the magic word.");
    _terminate();
  }

  // process_pkt expects const unsigned int *param_1, pass packet_buffer
  response_message = process_pkt(packet_buffer);

  // Call sendline: fd=1 (stdout), buf=response_message, len=strlen(response_message)
  if (sendline(response_message) < 0) {
    _terminate();
  }

  return 0;
}