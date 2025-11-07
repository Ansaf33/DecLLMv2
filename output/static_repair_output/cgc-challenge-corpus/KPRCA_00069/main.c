#include <stdio.h>    // For printf, fflush, fread, fwrite, stdin, stdout, FILE
#include <stdlib.h>   // For malloc, free, exit, strtoul, strtol
#include <string.h>   // For memcpy, sprintf, strlen, strdup, memset
#include <stdbool.h>  // For bool type
#include <stdint.h>   // For uint32_t, uint16_t (used in BMP header manipulation)

// Custom type for barcode objects, inferred to be a pointer to some structure.
// Using void* for the barcode handle, as its internal structure isn't fully defined.
typedef void* BarcodeHandle;

// Global variables (inferred from usage)
// These should ideally be encapsulated or passed as arguments, but sticking to the snippet's style.
int g_replace_oldest = 0;
int g_c_idx = 0;
int g_c_oldest = 0;
#define CACHE_SIZE 10
BarcodeHandle g_barcode_cache[CACHE_SIZE];
#define INPUT_BUFFER_SIZE 2048 // 0x800
char g_input[INPUT_BUFFER_SIZE];

// External variables from the original snippet (likely from a specific memory address)
// For a compilable example, let's define them as static globals.
// Assuming _DAT_4347c000 is an int and DAT_4347c000 is a char array.
static int _DAT_4347c000 = 0x12345678; // Placeholder value
static char DAT_4347c000[0x1000] = {0}; // Placeholder array

// --- Placeholder declarations and minimal implementations for custom functions ---

// Reads input until a delimiter (e.g., '\n') or max_len. Returns length read or -1 on error.
int freaduntil(char *buf, size_t max_len, int delimiter, FILE *stream) {
    if (!fgets(buf, max_len, stream)) {
        return -1; // Error or EOF
    }
    // Remove trailing delimiter (newline) if present
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == delimiter) {
        buf[len - 1] = '\0';
        len--;
    }
    return len;
}

// Assumed structure for barcode objects based on memory access in `add_to_cache` and `main`
// This is a minimal interpretation to allow compilation and correct freeing.
// The actual `Barcode` struct might have more members or different types.
// We use `void*` for `BarcodeHandle` and cast to `struct InternalBarcode` when freeing/accessing members.
// This assumes the `BarcodeHandle` points to the start of this structure.
struct InternalBarcode {
    void *field0; // At offset 0x0 (4 bytes)
    void *field4; // At offset 0x4 (4 bytes)
    char *text_data; // At offset 0x8 (4/8 bytes for pointer)
    void *encoded_data; // At offset 0xc (4/8 bytes for pointer)
    // More fields might follow
};

// Function to validate BMP headers
int validate_bmp_headers(const void *bmp_header, void *unknown_stack_param) {
    // Placeholder: Always return 0 for success
    (void)bmp_header; // Suppress unused parameter warning
    (void)unknown_stack_param; // Suppress unused parameter warning
    return 0;
}

// Function to create barcode from BMP data
BarcodeHandle create_barcode_from_bmp(const void *bmp_data) {
    // Placeholder: Return a dummy barcode handle
    struct InternalBarcode *barcode = malloc(sizeof(struct InternalBarcode));
    if (barcode) {
        barcode->field0 = NULL;
        barcode->field4 = NULL;
        barcode->text_data = strdup("BMP Barcode Text");
        barcode->encoded_data = NULL;
    }
    return barcode;
}

// Function to create barcode from string
BarcodeHandle create_barcode_from_str(const char *str) {
    // Placeholder: Return a dummy barcode handle
    struct InternalBarcode *barcode = malloc(sizeof(struct InternalBarcode));
    if (barcode) {
        barcode->field0 = NULL;
        barcode->field4 = NULL;
        barcode->text_data = strdup(str);
        barcode->encoded_data = NULL;
    }
    return barcode;
}

// Function to create barcode from encoded data
BarcodeHandle create_barcode_from_encoded_data(const char *encoded_data_str) {
    // Placeholder: Return a dummy barcode handle
    struct InternalBarcode *barcode = malloc(sizeof(struct InternalBarcode));
    if (barcode) {
        barcode->field0 = NULL;
        barcode->field4 = NULL;
        barcode->text_data = strdup("Encoded Barcode Text");
        barcode->encoded_data = strdup(encoded_data_str);
    }
    return barcode;
}

// Function to print barcode ASCII representation
void print_barcode_ascii(BarcodeHandle barcode_ptr) {
    // Placeholder: Print a generic message
    printf("[ASCII representation of %s]\n", ((struct InternalBarcode*)barcode_ptr)->text_data);
}

// Function to create barcode BMP image
void* create_barcode_bmp(BarcodeHandle barcode_ptr) {
    // Placeholder: Return a dummy BMP buffer
    // A minimal BMP header is 54 bytes. Let's create a tiny dummy BMP.
    // The original code uses *(size_t *)((int)__ptr + 2) for total size.
    // Let's assume a 54-byte header + 10 bytes of image data, total 64 bytes.
    char *bmp_buffer = malloc(64);
    if (bmp_buffer) {
        memset(bmp_buffer, 0, 64);
        // Minimal BMP header (BM, size=64, data_offset=54, DIB_header_size=40, width=1, height=1, planes=1, bpp=8)
        bmp_buffer[0] = 'B'; bmp_buffer[1] = 'M';          // BM signature
        *(uint32_t*)(bmp_buffer + 2) = 64;                  // File size
        *(uint32_t*)(bmp_buffer + 10) = 54;                 // Pixel data offset
        *(uint32_t*)(bmp_buffer + 14) = 40;                 // DIB header size
        *(uint32_t*)(bmp_buffer + 18) = 1;                  // Width
        *(uint32_t*)(bmp_buffer + 22) = 1;                  // Height
        *(uint16_t*)(bmp_buffer + 26) = 1;                  // Color planes
        *(uint16_t*)(bmp_buffer + 28) = 8;                  // Bits per pixel
        // Add some dummy pixel data
        memset(bmp_buffer + 54, 0xFF, 10);
    }
    return bmp_buffer;
}

// Standard C setvbuf for line buffering
void fbuffered(FILE *stream, int mode) {
    if (mode == 1) { // Assuming 1 means line buffering
        setvbuf(stream, NULL, _IOLBF, 0);
    } else { // Default to unbuffered for other cases
        setvbuf(stream, NULL, _IONBF, 0);
    }
}
// --- End of placeholder declarations and minimal implementations ---


// Function: add_to_cache
void add_to_cache(BarcodeHandle barcode_ptr) {
  if (!g_replace_oldest && g_c_idx < CACHE_SIZE) {
    g_barcode_cache[g_c_idx] = barcode_ptr;
    g_c_idx++;
  } else {
    // Free old barcode's resources
    BarcodeHandle old_barcode_ptr = g_barcode_cache[g_c_oldest];
    if (old_barcode_ptr) {
        struct InternalBarcode *old_internal_barcode = (struct InternalBarcode*)old_barcode_ptr;
        free(old_internal_barcode->text_data);
        free(old_internal_barcode->encoded_data);
        free(old_internal_barcode); // Free the barcode object itself
    }

    g_replace_oldest = 1; // This line seems redundant if it's already true here, but keeping original logic.
    g_barcode_cache[g_c_oldest] = barcode_ptr;
    g_c_oldest = (g_c_oldest + 1) % CACHE_SIZE;
  }
}

// Function: select_from_cache
BarcodeHandle select_from_cache(void) {
  size_t current_cache_size = g_c_idx;
  if (g_replace_oldest) {
    current_cache_size = CACHE_SIZE;
  }

  size_t selected_index = (size_t)-1; // Using (size_t)-1 for 0xffffffff
  while (true) {
    if (selected_index < current_cache_size) {
      return g_barcode_cache[(g_c_oldest + selected_index) % CACHE_SIZE];
    }

    printf("Select Cached Barcode\n");
    for (size_t i = 0; i < current_cache_size; i++) {
      BarcodeHandle current_barcode = g_barcode_cache[(g_c_oldest + i) % CACHE_SIZE];
      printf("    %zu. Text: %s\n", i + 1, ((struct InternalBarcode*)current_barcode)->text_data);
    }
    printf("    0. Return to main menu\n");
    printf(":-$  ");
    fflush(stdout);

    if (freaduntil(g_input, INPUT_BUFFER_SIZE, '\n', stdin) == -1) {
        printf("Invalid Selection\n");
        fflush(stdout);
        exit(0);
    }

    unsigned long uVar2 = strtoul(g_input, NULL, 10);
    if (uVar2 == 0) {
      return NULL;
    }
    selected_index = uVar2 - 1;
  }
}

// Function: input_barcode
void input_barcode(void) {
  unsigned long choice = 0xffffffffUL; // Initialize to a value > 3
  BarcodeHandle new_barcode = NULL;
  char message_buffer[256]; // For sprintf messages

  while (choice > 3) { // Original condition was `4 < local_10`
    printf("Input barcode as:\n");
    printf("    1. String to encode\n");
    printf("    2. Barcode encoded in ascii (\"|\" and \" \")\n");
    printf("    3. Barcode bitmap\n");
    printf("    0. Return to main menu\n");
    printf(":-$  ");
    fflush(stdout);

    if (freaduntil(g_input, INPUT_BUFFER_SIZE, '\n', stdin) == -1) {
      printf("Invalid Selection\n");
      fflush(stdout);
      exit(0);
    }
    choice = strtoul(g_input, NULL, 10);
  }

  if (choice == 3) {
    char bmp_header_buffer[54]; // Standard BMP header size
    void *bmp_data_buffer = NULL;

    fflush(stdout);
    // Read BMP header (54 bytes)
    if (fread(bmp_header_buffer, 1, sizeof(bmp_header_buffer), stdin) != sizeof(bmp_header_buffer)) {
      sprintf(message_buffer, "Bad input (failed to read BMP header)\n");
    } else {
      if (validate_bmp_headers(bmp_header_buffer, NULL) == 0) { // Assuming NULL is okay for the second param
        // Extract total BMP file size from header (offset 0x02)
        uint32_t total_bmp_size = *(uint32_t*)(bmp_header_buffer + 2);
        
        // Allocate buffer for entire BMP file. Ensure at least header size.
        size_t alloc_size = (total_bmp_size > sizeof(bmp_header_buffer)) ? total_bmp_size : sizeof(bmp_header_buffer);

        bmp_data_buffer = malloc(alloc_size);
        if (!bmp_data_buffer) {
            sprintf(message_buffer, "Memory allocation failed for BMP data.\n");
        } else {
            memcpy(bmp_data_buffer, bmp_header_buffer, sizeof(bmp_header_buffer));
            size_t bytes_to_read = alloc_size - sizeof(bmp_header_buffer);

            if (fread((char*)bmp_data_buffer + sizeof(bmp_header_buffer), 1, bytes_to_read, stdin) != bytes_to_read) {
                sprintf(message_buffer, "Bad input (failed to read BMP image data)\n");
            } else {
                new_barcode = create_barcode_from_bmp(bmp_data_buffer);
                if (new_barcode == NULL) {
                    sprintf(message_buffer, "Bad barcode bitmap\n");
                } else {
                    add_to_cache(new_barcode);
                    sprintf(message_buffer, "Successfully added bitmap barcode to cache\nBarcode text: %s\n",
                            ((struct InternalBarcode*)new_barcode)->text_data);
                }
            }
        }
      } else {
        sprintf(message_buffer, "Bad input (invalid BMP headers)\n");
      }
    }

    if (bmp_data_buffer) {
      free(bmp_data_buffer);
    }
    printf("%s", message_buffer); // Print the accumulated message
  }
  else if (choice == 1) {
    if (freaduntil(g_input, INPUT_BUFFER_SIZE, '\n', stdin) == -1) {
      printf("Bad input\n");
      fflush(stdout);
      exit(0);
    }
    new_barcode = create_barcode_from_str(g_input);
    if (new_barcode == NULL) {
      printf("Bad barcode string\n");
    } else {
      add_to_cache(new_barcode);
      printf("Successfully added barcode to cache\n");
      printf("Barcode text: %s\n", ((struct InternalBarcode*)new_barcode)->text_data);
    }
  }
  else if (choice == 2) {
    if (freaduntil(g_input, INPUT_BUFFER_SIZE, '\n', stdin) == -1) {
      printf("Bad input\n");
      fflush(stdout);
      exit(0);
    }
    new_barcode = create_barcode_from_encoded_data(g_input);
    if (new_barcode == NULL) {
      printf("Bad barcode encoding\n");
    } else {
      add_to_cache(new_barcode);
      printf("Successfully added barcode to cache\n");
      printf("Barcode text: %s\n", ((struct InternalBarcode*)new_barcode)->text_data);
    }
  }
  // choice 0 returns to main menu, so no action needed here.
}

// Function: view_cached_barcodes
void view_cached_barcodes(void) {
  size_t current_cache_size = g_c_idx;
  if (g_replace_oldest) {
    current_cache_size = CACHE_SIZE;
  }

  printf("Cached Barcodes\n");
  for (size_t i = 0; i < current_cache_size; i++) {
    BarcodeHandle current_barcode = g_barcode_cache[(g_c_oldest + i) % CACHE_SIZE];
    printf("    %zu. Text: %s\n", i + 1, ((struct InternalBarcode*)current_barcode)->text_data);
  }
}

// Function: clear_cached_barcodes
void clear_cached_barcodes(void) {
  size_t current_cache_size = g_c_idx;
  if (g_replace_oldest) {
    current_cache_size = CACHE_SIZE;
  }

  for (size_t i = 0; i < current_cache_size; i++) {
    BarcodeHandle barcode_ptr = g_barcode_cache[i]; // Original code iterates physical indices 0 to uVar1-1
    if (barcode_ptr) { // Check if slot is occupied
        struct InternalBarcode *internal_barcode = (struct InternalBarcode*)barcode_ptr;
        free(internal_barcode->text_data);
        free(internal_barcode->encoded_data);
        free(internal_barcode);
        g_barcode_cache[i] = NULL; // Clear the pointer after freeing
    }
  }
  g_replace_oldest = 0;
  g_c_idx = 0;
  g_c_oldest = 0;
}

// Function: check_seed
void check_seed(void) {
  int input_seed_val;
  // Original `fread(local_10,4,(size_t)stdin,(FILE *)0x12e3d);`
  // Interpreted as reading one integer from stdin.
  if (fread(&input_seed_val, sizeof(int), 1, stdin) == 1) {
    if (_DAT_4347c000 == input_seed_val) {
      // Original `fwrite(&DAT_4347c000,0x1000,(size_t)stdout,__s);`
      // Interpreted as writing the entire DAT_4347c000 array to stdout.
      fwrite(DAT_4347c000, 1, sizeof(DAT_4347c000), stdout);
    }
  }
}

// Function: main
int main(void) {
  bool running = true;

  fbuffered(stdout, 1); // Set stdout to line-buffered
  check_seed();
  printf("Welcome to the EZ Barcode Encoder/Decoder Service\n");

  while (running) {
    printf("--Select an option--\n");
    printf("1. Input a new barcode\n");
    printf("2. View barcode ascii\n");
    printf("3. Generate barcode image\n");
    printf("4. View cached barcodes\n");
    printf("5. Clear cached barcodes\n");
    printf("6. Quit\n");
    printf(":-$  ");
    fflush(stdout);

    if (freaduntil(g_input, INPUT_BUFFER_SIZE, '\n', stdin) == -1) {
      printf("Invalid Selection\n");
      fflush(stdout);
      exit(0); // Exit on read error
    }

    long choice = strtol(g_input, NULL, 10);
    BarcodeHandle selected_barcode = NULL; // Variable for barcode selected from cache
    void *bmp_output = NULL; // Variable for BMP output

    switch (choice) {
      default:
        printf("Invalid Selection\n");
        break;
      case 1:
        input_barcode();
        break;
      case 2:
        selected_barcode = select_from_cache();
        if (selected_barcode != NULL) {
          printf("Raw String: ");
          print_barcode_ascii(selected_barcode);
        }
        break;
      case 3:
        selected_barcode = select_from_cache();
        if (selected_barcode != NULL) {
          bmp_output = create_barcode_bmp(selected_barcode);
          if (bmp_output == NULL) {
            printf("Bad barcode (failed to create BMP)\n");
          } else {
            printf("    Printable Barcode:\n");
            // The total size of the BMP data is at offset 0x02 in the BMP header.
            // Assuming bmp_output points to the start of the BMP header.
            size_t bmp_size = *(uint32_t*)((char*)bmp_output + 2);
            fwrite(bmp_output, 1, bmp_size, stdout);
            printf("\n    Barcode String: %s\n", ((struct InternalBarcode*)selected_barcode)->text_data);
            free(bmp_output);
          }
        }
        break;
      case 4:
        view_cached_barcodes();
        break;
      case 5:
        clear_cached_barcodes();
        break;
      case 6:
        running = false; // Set flag to exit loop
        break;
    }
  }

  printf("Thanks for using the EZ Barcode Encoder/Decoder Service\n");
  fflush(stdout);
  return 0;
}