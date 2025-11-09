#include <stdio.h>    // For printf, fprintf, fflush, stdin, stdout, FILE, EOF
#include <stdlib.h>   // For malloc, free, exit, strtoul
#include <string.h>   // For memset, strlen, memcpy
#include <stdint.h>   // For ssize_t (if not implicitly available), uint8_t, uint32_t, uint64_t
#include <stdbool.h>  // For bool type
#include <unistd.h>   // For ssize_t on some systems, though often in stdio.h or sys/types.h

// --- MOCK/HELPER DEFINITIONS ---
// Dummy struct for SC context, inferred from memory access patterns
typedef struct SCContext {
    unsigned char key[95];
    char *data_ptr;
    size_t data_len;
    // Other members might exist, but these are the ones accessed.
    // The original code accesses offsets 0x60 and 100 (0x64).
    // Let's ensure our struct matches these offsets if possible.
    // For simplicity, we'll assume data_ptr is at 0x60 and data_len at 100.
    // This requires specific padding or member ordering. For a mock,
    // direct member access is fine, the offsets are just decompiler artifacts.
} SCContext;

// Mock declaration for freaduntil
// Reads up to max_len-1 characters into buf, or until delim is found, or EOF.
// Adds null terminator. Returns number of characters read (excluding null terminator).
// Returns -1 on error or if no characters could be read before EOF.
ssize_t freaduntil(char *buf, size_t max_len, int delim, FILE *stream) {
    size_t count = 0;
    int c;
    if (max_len == 0) {
        return 0;
    }
    while (count < max_len - 1 && (c = fgetc(stream)) != EOF && c != delim) {
        buf[count++] = (char)c;
    }
    if (c == EOF && count == 0) { // No characters read before EOF
        buf[0] = '\0'; // Ensure buffer is null-terminated even on empty read
        return -1;
    }
    buf[count] = '\0';
    return count;
}

// Mock for sc_new
SCContext* sc_new(unsigned char* key) {
    SCContext* ctx = (SCContext*)malloc(sizeof(SCContext));
    if (ctx) {
        memcpy(ctx->key, key, 95);
        ctx->data_ptr = NULL;
        ctx->data_len = 0;
    }
    return ctx;
}

// Mock for sc_scompress
int sc_scompress(SCContext* ctx, void** compressed_data_ptr, unsigned int* compressed_len) {
    if (!ctx || !ctx->data_ptr || !compressed_data_ptr || !compressed_len) {
        return -1;
    }
    size_t original_len = ctx->data_len;
    if (original_len == 0) {
        *compressed_data_ptr = NULL;
        *compressed_len = 0;
        return 0;
    }
    // Simulate compression: half size, min 1 byte
    size_t comp_len = (original_len / 2) > 0 ? (original_len / 2) : 1;
    *compressed_data_ptr = malloc(comp_len);
    if (!*compressed_data_ptr) {
        return -1;
    }
    memcpy(*compressed_data_ptr, ctx->data_ptr, comp_len); // Copy first portion
    *compressed_len = (unsigned int)comp_len;
    return 0;
}

// Mock for sc_sdecompress
int sc_sdecompress(SCContext* ctx, char** decompressed_data_ptr, size_t* decompressed_len) {
    if (!ctx || !ctx->data_ptr || !decompressed_data_ptr || !decompressed_len) {
        return -1;
    }
    size_t compressed_len = ctx->data_len;
    if (compressed_len == 0) {
        *decompressed_data_ptr = NULL;
        *decompressed_len = 0;
        return 0;
    }
    // Simulate decompression: double size
    size_t decomp_len = compressed_len * 2;
    *decompressed_data_ptr = (char*)malloc(decomp_len + 1); // +1 for null terminator
    if (!*decompressed_data_ptr) {
        return -1;
    }
    memcpy(*decompressed_data_ptr, ctx->data_ptr, compressed_len); // Copy compressed data
    // Fill the rest with some pattern for demonstration
    for (size_t i = compressed_len; i < decomp_len; ++i) {
        (*decompressed_data_ptr)[i] = 'X';
    }
    (*decompressed_data_ptr)[decomp_len] = '\0'; // Null terminate
    *decompressed_len = decomp_len;
    return 0;
}

// Global mocks for constants
double _DAT_000160c8 = 100.0; // For percentage calculation
char DAT_00016078 = '\n';    // For fwrite in handle_decompress, likely a newline character
unsigned char secret[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE, 0x00}; // Example 8 bytes + null

// --- ORIGINAL FUNCTIONS (FIXED) ---

// Function: print_menu
void print_menu(void) {
  printf("1. Compress\n");
  printf("2. Decompress\n");
  printf("3. Quit\n");
}

// Function: handle_compress
void handle_compress(void) {
  unsigned char key_counts[256] = {0}; // To track key character uniqueness
  unsigned char key_buffer[95];
  char data_buffer[4096];
  void *compressed_data = NULL;
  unsigned int compressed_len = 0;
  SCContext *ctx = NULL;

  bool error_occurred = false;

  printf("Key?\n");
  // Read 95 bytes for the key
  if (fread(key_buffer, 1, sizeof(key_buffer), stdin) != sizeof(key_buffer)) {
    error_occurred = true;
  } else {
    // Validate key characters: must be printable ASCII (' ' to '~') and unique
    for (unsigned int i = 0; i < sizeof(key_buffer); ++i) {
      unsigned char c = key_buffer[i];
      if (c < 0x20 || c > 0x7e || key_counts[c] > 0) {
        error_occurred = true;
        break;
      }
      key_counts[c]++;
    }
  }

  if (!error_occurred) {
    printf("Data?\n");
    fflush(stdout);
    // Read data until newline or buffer full
    ssize_t bytes_read = freaduntil(data_buffer, sizeof(data_buffer), '\n', stdin);

    if (bytes_read == -1) { // Error or no data read
      error_occurred = true;
    } else {
      ctx = sc_new(key_buffer);
      if (ctx == NULL) {
        error_occurred = true;
      } else {
        ctx->data_ptr = data_buffer;
        ctx->data_len = (size_t)bytes_read;

        if (sc_scompress(ctx, &compressed_data, &compressed_len) == -1) {
          error_occurred = true;
        } else {
          printf("Original Size: %zu\n", ctx->data_len);
          int percentage = 0;
          if (ctx->data_len > 0) {
              percentage = (int)(_DAT_000160c8 * ((double)compressed_len / (double)ctx->data_len));
          }
          printf("Compressed Size: %u (%d%%)\n", compressed_len, percentage);
          printf("Compressed Data: ");
          // Print up to 32 bytes of compressed data in hex
          for (unsigned int i = 0; i < compressed_len && i < 0x20; ++i) {
            printf("%02X", ((unsigned char*)compressed_data)[i]);
          }
          printf("\n");
        }
      }
    }
  }

  if (error_occurred) {
    printf("error.\n");
  }

  // Cleanup
  if (ctx != NULL) {
    free(ctx);
  }
  if (compressed_data != NULL) {
    free(compressed_data);
  }
}

// Function: handle_decompress
void handle_decompress(void) {
  unsigned char key_counts[256] = {0}; // To track key character uniqueness
  unsigned char key_buffer[95];
  char input_buffer[4096]; // Used for length and compressed data
  char *decompressed_data = NULL;
  size_t decompressed_len = 0;
  unsigned long compressed_input_len = 0; // Length read from user
  SCContext *ctx = NULL;

  bool error_occurred = false;

  printf("Key?\n");
  // Read 95 bytes for the key
  if (fread(key_buffer, 1, sizeof(key_buffer), stdin) != sizeof(key_buffer)) {
    error_occurred = true;
  } else {
    // Validate key characters: must be printable ASCII (' ' to '~') and unique
    for (unsigned int i = 0; i < sizeof(key_buffer); ++i) {
      unsigned char c = key_buffer[i];
      if (c < 0x20 || c > 0x7e || key_counts[c] > 0) {
        error_occurred = true;
        break;
      }
      key_counts[c]++;
    }
  }

  if (!error_occurred) {
    printf("Length?\n");
    fflush(stdout);
    // Read length string until newline
    ssize_t bytes_read_len = freaduntil(input_buffer, sizeof(input_buffer), '\n', stdin);

    if (bytes_read_len == -1) { // Error or no data read
      error_occurred = true;
    } else {
      compressed_input_len = strtoul(input_buffer, NULL, 10);
      // Check if the length is within reasonable bounds (e.g., max 4096, matching buffer)
      if (compressed_input_len == 0 || compressed_input_len > sizeof(input_buffer)) {
        error_occurred = true;
      } else {
        printf("Data?\n");
        // Read the actual compressed data based on the length provided
        if (fread(input_buffer, 1, compressed_input_len, stdin) != compressed_input_len) {
          error_occurred = true;
        } else {
          ctx = sc_new(key_buffer);
          if (ctx == NULL) {
            error_occurred = true;
          } else {
            ctx->data_ptr = input_buffer;
            ctx->data_len = compressed_input_len;

            if (sc_sdecompress(ctx, &decompressed_data, &decompressed_len) == -1) {
              error_occurred = true;
            } else {
              printf("Compressed Size: %zu\n", ctx->data_len);
              printf("Original Size: %zu\n", decompressed_len); // Use decompressed_len directly
              printf("Original Data: ");
              // Write decompressed data and a newline
              fwrite(decompressed_data, 1, decompressed_len, stdout);
              fwrite(&DAT_00016078, 1, 1, stdout); // DAT_00016078 is a newline char
            }
          }
        }
      }
    }
  }

  if (error_occurred) {
    printf("error.\n");
  }

  // Cleanup
  if (ctx != NULL) {
    free(ctx);
  }
  if (decompressed_data != NULL) {
    free(decompressed_data);
  }
}

// Function: main
int main(void) {
  char menu_choice_str[8]; // Buffer for menu choice input

  // Set stdin to unbuffered mode
  setvbuf(stdin, NULL, _IONBF, 0);

  // Print secret (first 8 bytes in hex)
  for (unsigned int i = 0; i < 8; ++i) {
    printf("%02X", secret[i]);
  }
  printf("\n");

  while (true) {
    print_menu();
    fflush(stdout);

    // Read menu choice until newline or buffer full
    ssize_t bytes_read = freaduntil(menu_choice_str, sizeof(menu_choice_str), '\n', stdin);

    if (bytes_read == -1) { // Error or EOF on input
      fprintf(stderr, "Error reading menu choice or EOF.\n");
      return 1; // Exit with error
    }

    unsigned long choice = strtoul(menu_choice_str, NULL, 10);

    if (choice == 3) {
      printf("Bye.\n");
      fflush(stdout);
      exit(0);
    } else if (choice == 1) {
      handle_compress();
    } else if (choice == 2) {
      handle_decompress();
    } else {
      printf("Invalid menu.\n");
    }
  }

  return 0; // Should not be reached due to exit(0)
}