#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, calloc, free, srand, exit
#include <string.h>  // For memset, strlen, strcpy, memcpy
#include <time.h>    // For time (used as fallback for srand)

// Custom type definitions (assuming these from the context)
typedef unsigned int uint;
typedef unsigned char undefined;
typedef unsigned int undefined4; // Used for 4-byte values, often addresses or return codes

// External function declarations (placeholders)
// These functions are not defined in the snippet, so we declare them as extern.
// Their actual signatures might differ, but this is a best guess based on usage.
// fdwait: int fdwait(int maxfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
// The snippet's usage suggests: fdwait(maxfd + 1, read_fds_ptr, nfds_count_or_flags_ptr, timeout_val_ptr)
extern int fdwait(int maxfd_plus_1, uint *readfds, unsigned int *nfds_count_or_flags, int *timeout_val);
extern int receive(uint fd, void *buf, uint len, int *bytes_received); // Returns 0 on success, non-zero on error
extern uint getshifty(void);
extern uint sendall(uint fd, const void *buf, uint len); // Returns bytes sent, 0 on error
extern int add_dir(const char *path); // Returns 0 on success, non-zero on error
extern char *ls_dir(const char *path); // Returns allocated string on success, NULL on error (needs free)
extern int add_file(const char *name, const char *content); // Returns 0 on success, non-zero on error
extern char *readfile(const char *name); // Returns allocated string/buffer on success, NULL on error (needs free)
extern int rm_file(const char *name); // Returns 0 on success, non-zero on error
extern int rm_dir(const char *name); // Returns 0 on success, non-zero on error
extern void init_fs(void);
extern int readall(uint fd, void *buf, uint len); // Returns bytes read, 0 on error

// Global variables (placeholders)
unsigned int conf;
void *last = NULL;
unsigned int _DAT_00016008 = 0; // Initialized to 0 for compilation
unsigned int DAT_0001600c = 0; // Initialized to 0 for compilation
unsigned int DAT_00016010 = 0; // Initialized to 0 for compilation
unsigned int DAT_00016014 = 0; // Initialized to 0 for compilation

// Function: _terminate - acts like exit
void _terminate(int status) {
    exit(status);
}

// Function: readall_timeout
uint readall_timeout(uint fd, void *buffer, uint total_size) {
  uint bytes_read_total = 0;
  uint read_fds[32]; // Represents an fd_set for up to 1024 FDs (32 * 32 bits)
  unsigned int nfds_count = 2; // Arbitrary value, its meaning depends on fdwait implementation
  int timeout_val = 0;
  int bytes_received_current = 0;

  memset(read_fds, 0, sizeof(read_fds));
  read_fds[fd >> 5] |= (1U << (fd & 0x1f)); // Set the bit for the given fd

  while (bytes_read_total < total_size) {
    int fdwait_ret = fdwait(fd + 1, read_fds, &nfds_count, &timeout_val);
    if (fdwait_ret != 0) {
      // Error or actual timeout (if timeout_val was > 0 and expired)
      break;
    }
    if (timeout_val == 0) {
      // Original code returns if local_9c (timeout_val) is 0.
      // This might mean no FDs were ready, or a non-blocking timeout occurred.
      return bytes_read_total;
    }

    int receive_ret = receive(fd, (char*)buffer + bytes_read_total, total_size - bytes_read_total, &bytes_received_current);
    if (receive_ret != 0 || bytes_received_current == 0) {
      _terminate(0x84); // Unrecoverable receive error or 0 bytes received unexpectedly
    }
    bytes_read_total += bytes_received_current;
  }
  return bytes_read_total;
}

// Function: ereadall
uint ereadall(uint fd, void *buffer, uint total_size) {
  if ((total_size & 3) != 0) { // Check if total_size is not a multiple of 4
    _terminate(0x84);
  }

  uint bytes_read = readall_timeout(fd, buffer, total_size);

  if (bytes_read == total_size) {
    for (uint i = 0; i < total_size >> 2; i++) { // Loop total_size / 4 times (for uints)
      *(uint *)((char *)buffer + i * 4) ^= getshifty();
    }
  }
  return bytes_read;
}

// Function: esendall
uint esendall(uint fd, void *buffer, uint total_size) {
  if ((total_size & 3) != 0) { // Check if total_size is not a multiple of 4
    _terminate(0x85);
  }

  for (uint i = 0; i < total_size >> 2; i++) { // Loop total_size / 4 times (for uints)
    *(uint *)((char *)buffer + i * 4) ^= getshifty();
  }

  uint bytes_sent = sendall(fd, (const void*)buffer, total_size);

  if (total_size != bytes_sent) {
    _terminate(0x85);
  }
  return total_size;
}

// Function: do_mkdir
undefined4 * do_mkdir(void *request_data) {
  undefined4 *result_buf = (undefined4 *)calloc(1, 0x404); // Allocate 0x404 bytes, initialized to zero
  if (result_buf == NULL) {
    return NULL;
  }

  if (add_dir((char *)request_data + 4) == 0) {
    *result_buf = 0; // Success code
  } else {
    *result_buf = 1; // Error code
  }
  return result_buf;
}

// Function: do_list
undefined4 * do_list(void *request_data) {
  undefined4 *result_buf = (undefined4 *)calloc(1, 0x404);
  if (result_buf == NULL) {
    return NULL;
  }

  char *list_str = ls_dir((char *)request_data + 4);
  if (list_str == NULL) {
    *result_buf = 3; // Error code
  } else {
    size_t len = strlen(list_str);
    if (len > 0x3ff) { // If string is too long for the 0x400 buffer
      list_str[0x3ff] = '\0'; // Truncate
      len = 0x3ff;
    }
    strcpy((char *)(result_buf + 1), list_str); // Copy into the buffer after the status code
    *result_buf = 2; // Success code
    free(list_str);
  }
  return result_buf;
}

// Function: do_put
undefined4 * do_put(void *request_data) {
  undefined4 *result_buf = (undefined4 *)calloc(1, 0x404);
  if (result_buf == NULL) {
    return NULL;
  }

  char *file_name_ptr = (char *)request_data + 4;
  size_t name_len = strlen(file_name_ptr);

  if (name_len >= 0x80) { // Filename too long (128 bytes)
    *result_buf = 5; // Error code
    return result_buf;
  }

  char *file_name = (char *)malloc(name_len + 1);
  if (file_name == NULL) {
    *result_buf = 5; // Error code
    return result_buf;
  }
  strcpy(file_name, file_name_ptr);

  char *file_content_ptr = (char *)request_data + 4 + name_len + 1; // Content starts after name + null terminator
  size_t content_len = strlen(file_content_ptr);

  if (content_len >= 0x400) { // Content too long (1024 bytes)
    *result_buf = 5; // Error code
    free(file_name);
    return result_buf;
  }

  char *file_content = (char *)malloc(content_len + 1);
  if (file_content == NULL) {
    *result_buf = 5; // Error code
    free(file_name);
    return result_buf;
  }
  strcpy(file_content, file_content_ptr);

  if (add_file(file_name, file_content) == 0) {
    *result_buf = 4; // Success code
  } else {
    *result_buf = 5; // Error code
  }

  free(file_name);
  free(file_content);
  return result_buf;
}

// Function: do_get
undefined4 * do_get(void *request_data) {
  undefined4 *result_buf = (undefined4 *)calloc(1, 0x404);
  if (result_buf == NULL) {
    return NULL;
  }

  char *file_content = readfile((char *)request_data + 4);
  if (file_content == NULL) {
    *result_buf = 7; // Error code
  } else {
    *result_buf = 6; // Success code
    memcpy(result_buf + 1, file_content, 0x400); // Copy up to 0x400 bytes of content
    free(file_content); // Assuming readfile returns a malloc'd buffer
  }
  return result_buf;
}

// Function: do_rm
undefined4 * do_rm(void *request_data) {
  undefined4 *result_buf = (undefined4 *)calloc(1, 0x404);
  if (result_buf == NULL) {
    return NULL;
  }

  if (rm_file((char *)request_data + 4) == 0) {
    *result_buf = 8; // Success code
  } else {
    *result_buf = 9; // Error code
  }
  return result_buf;
}

// Function: do_rmdir
undefined4 * do_rmdir(void *request_data) {
  undefined4 *result_buf = (undefined4 *)calloc(1, 0x404);
  if (result_buf == NULL) {
    return NULL;
  }

  if (rm_dir((char *)request_data + 4) == 0) {
    *result_buf = 10; // Success code
  } else {
    *result_buf = 0xb; // Error code (11)
  }
  return result_buf;
}

// Function: process_req
undefined4 process_req(void) {
  undefined4 *request_buf = (undefined4 *)malloc(0x404);
  if (request_buf == NULL) {
    return 3; // Allocation error
  }

  int bytes_read = ereadall(0, request_buf, 0x404);
  if (bytes_read != 0x404) {
    free(request_buf);
    return 2; // Read error
  }

  // Ensure null termination for string operations if buffer contains strings
  ((char*)request_buf)[0x403] = 0;

  undefined4 *response_buf = NULL;
  undefined4 return_code = 0;

  switch (*request_buf) {
    case 0:
      response_buf = do_mkdir(request_buf);
      break;
    case 1:
      response_buf = do_list(request_buf);
      break;
    case 2:
      response_buf = do_put(request_buf);
      break;
    case 3:
      response_buf = do_get(request_buf);
      break;
    case 4:
      response_buf = do_rm(request_buf);
      break;
    case 5:
      response_buf = do_rmdir(request_buf);
      break;
    case 6: // Exit command
      free(request_buf);
      return 1;
    default: // Unknown command
      free(request_buf);
      return 3;
  }

  free(request_buf); // Request buffer is processed, free it.

  if (response_buf == NULL) {
    return_code = 3; // Error creating response
  } else {
    esendall(1, response_buf, 0x404);
    if (last != NULL) {
      free(last);
    }
    last = response_buf; // Store the response buffer in 'last'
    return_code = 0; // Success
  }
  return return_code;
}

// Function: go
long go(void) {
  init_fs();

  // Read config
  int bytes_read_conf = readall_timeout(0, &conf, 0x14); // 0x14 = 20 bytes
  if (bytes_read_conf != 0x14) {
    int error_resp = 0;
    sendall(1, &error_resp, 4);
    return 3; // Error reading conf
  }

  // Check conf value against _DAT_00016008
  if ((_DAT_00016008 < 0xc0ff34) && (0xc0ff32 < conf)) {
    int success_resp = 1;
    sendall(1, &success_resp, 4);

    // Placeholder for the hardcoded address 0x4347c020, assuming it's a constant data block
    static const uint initial_seed_data[8] = {
        0x4347c020, 0x1A2B3C4D, 0x5E6F7A8B, 0x9C0D1E2F,
        0x30415263, 0x748596A7, 0xB8C9DAEB, 0xF0E1D2C3
    };

    void *temp_buf = malloc(0x20); // Allocate 32 bytes
    if (temp_buf == NULL) {
      return 1; // Allocation error
    }

    memcpy(temp_buf, initial_seed_data, 0x20); // Copy 32 bytes from the "hardcoded" address

    if (DAT_0001600c == 0) {
      // Original code was `local_14 = calloc(0x20,(size_t)__size); srand((uint)local_14);`
      // `__size` was `local_14` (0x4347c020), which is clearly a decompilation error for `calloc`.
      // Assuming it intends to get a seed value.
      // A common pattern is to use the address of a freshly allocated block as a "random" seed.
      void *seed_val_ptr = calloc(1, 0x20);
      if (seed_val_ptr != NULL) {
          srand((uint)(unsigned long)seed_val_ptr); // Use the address as a seed
          free(seed_val_ptr);
      } else {
          srand(time(NULL)); // Fallback to time if calloc fails
      }
    } else {
        srand(time(NULL)); // Fallback to time if DAT_0001600c is not 0
    }

    uint current_xor_val = 0;
    for (int i = 0; i < 8; i += 2) {
      if (DAT_0001600c != 0) {
        current_xor_val = *(uint *)((char *)temp_buf + (i + 1) * 4) ^ *(uint *)((char *)temp_buf + i * 4);
      }
      sendall(1, &current_xor_val, 4);
    }
    free(temp_buf);

    int req_status;
    do {
      req_status = process_req();
      if (req_status == 2) { // Special handling for status 2
        if (DAT_00016010 != 0 && last != NULL) {
          uint offset_val; // Corresponds to local_2c
          int bytes_read_offset = readall(0, &offset_val, 4);
          if (bytes_read_offset != 4) {
            return 4; // Error
          }

          uint copy_len = 0x404 - offset_val; // Corresponds to local_20

          if (offset_val > DAT_00016014 || offset_val > 0x404) {
            return 1; // Error
          }

          // Allocate buffer for copying. Original: DAT_00016014 + 0x404.
          // Assuming 0x404 is the max size needed for the copy operation.
          void *copy_buf = malloc(0x404); 
          if (copy_buf == NULL) {
              return 4; // Allocation error
          }

          memcpy(copy_buf, (char *)last + offset_val, copy_len);

          uint bytes_sent_copy = esendall(1, copy_buf, copy_len);
          if (copy_len != bytes_sent_copy) {
            free(copy_buf);
            return 4; // Error
          }
          free(copy_buf);
          free(last);
          last = NULL;
        }
      }
    } while (req_status != 3 && req_status != 1); // Loop until status is 3 (error) or 1 (exit)
    return 0; // Success
  } else {
    int error_resp = 0;
    sendall(1, &error_resp, 4);
    return 5; // Config check failed
  }
}

// Function: get_text_checksum
uint get_text_checksum(void) {
  // This function in the original snippet attempts to checksum the 'go' function's code
  // by iterating through its memory region. This is highly platform-specific and relies
  // on compiler/linker behavior to determine function start/end addresses.
  // In standard C, getting a function's size or iterating its machine code is not portable.
  // To make this compilable and avoid undefined behavior or platform-specific hacks,
  // we will return a dummy checksum value. In a real scenario, this would involve
  // linker scripts or platform-specific APIs to get code segment information.
  return 0xDEADBEEF; // A dummy checksum for compilation
}

// Function: verify_integrity
uint verify_integrity(void) {
  uint calculated_checksum = get_text_checksum();
  uint shifty_val = getshifty();
  uint final_checksum = calculated_checksum ^ shifty_val;

  printf("@h%x\n", final_checksum); // Assuming @h is a placeholder for hex format specifier %x

  uint received_checksum;
  int bytes_read = readall(0, &received_checksum, 4);
  if (bytes_read != 4) {
    received_checksum = 0; // If read fails, assume 0
  }
  return (final_checksum == received_checksum); // Return 1 for match, 0 for mismatch
}

// Function: main
long main(void) {
  if (verify_integrity() == 0) { // If integrity check fails (returns 0)
    return 2; // Error code 2
  } else {
    return go(); // Run main logic
  }
}