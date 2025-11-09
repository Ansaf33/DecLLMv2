#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // For setvbuf, though often in stdio.h

// --- Type Definitions (based on decompiled structure) ---

// Assuming Barcode structure based on memory accesses
typedef struct Barcode {
    void *internal_data_1; // Decompiler's offset 0, type unknown, potentially unused or internal
    void *internal_data_2; // Decompiler's offset 4, type unknown, potentially unused or internal
    char *text;            // Decompiler's offset 8 (0x8)
    void *data;            // Decompiler's offset 12 (0xC), likely bitmap data or similar
} Barcode;

// Minimal BMP header structure for accessing bfSize and biSizeImage
// BITMAPFILEHEADER (14 bytes) + BITMAPINFOHEADER (40 bytes) = 54 bytes
#pragma pack(push, 1) // Ensure no padding for the BMP header struct
typedef struct {
    unsigned short bfType;      // 0x00
    unsigned int   bfSize;      // 0x02
    unsigned short bfReserved1; // 0x06
    unsigned short bfReserved2; // 0x08
    unsigned int   bfOffBits;   // 0x0A
    // BITMAPINFOHEADER starts here
    unsigned int   biSize;          // 0x0E
    int            biWidth;         // 0x12
    int            biHeight;        // 0x16
    unsigned short biPlanes;        // 0x1A
    unsigned short biBitCount;      // 0x1C
    unsigned int   biCompression;   // 0x1E
    unsigned int   biSizeImage;     // 0x22
    int            biXPelsPerMeter; // 0x26
    int            biYPelsPerMeter; // 0x2A
    unsigned int   biClrUsed;       // 0x2E
    unsigned int   biClrImportant;  // 0x32
} BMPHeader_t;
#pragma pack(pop)

// --- Global Variables (based on decompiled usage) ---
#define CACHE_SIZE 10
Barcode *g_barcode_cache[CACHE_SIZE];
int g_c_idx = 0;
int g_c_oldest = 0;
int g_replace_oldest = 0; // boolean flag, 0 or 1
char g_input[2048]; // 0x800 bytes

// Decompiler's _DAT_4347c000 and DAT_4347c000
static int g_seed_val = 0; // Example value, actual value unknown from snippet
static char g_seed_data[4096]; // 0x1000 bytes, example size

// --- External / Stub Functions (declarations for compilation) ---
// These functions are not provided in the snippet and need to be defined elsewhere.
// Their signatures are inferred from usage.
int freaduntil(char *buffer, int max_len, char terminator, FILE *stream);
int validate_bmp_headers(const char *header_buffer, size_t *image_size_out);
Barcode *create_barcode_from_bmp(void *bmp_data);
Barcode *create_barcode_from_str(const char *str);
Barcode *create_barcode_from_encoded_data(const char *data);
void print_barcode_ascii(const Barcode *barcode);
void *create_barcode_bmp(const Barcode *barcode);

// --- Function Implementations ---

// Function: add_to_cache
void add_to_cache(Barcode *new_barcode) {
  if (!g_replace_oldest && g_c_idx < CACHE_SIZE) {
    g_barcode_cache[g_c_idx++] = new_barcode;
  } else {
    g_replace_oldest = 1; // Ensure flag is set after first fill
    Barcode *old_barcode = g_barcode_cache[g_c_oldest];
    if (old_barcode) { // Free existing barcode data
      free(old_barcode->text);
      free(old_barcode->data);
      free(old_barcode);
    }
    g_barcode_cache[g_c_oldest] = new_barcode;
    g_c_oldest = (g_c_oldest + 1) % CACHE_SIZE;
  }
}

// Function: select_from_cache
Barcode *select_from_cache(void) {
  int cache_current_size = g_replace_oldest ? CACHE_SIZE : g_c_idx;
  int selection_idx = -1; // Initialize to an invalid selection

  while (true) {
    if (selection_idx >= 0 && selection_idx < cache_current_size) {
      return g_barcode_cache[(g_c_oldest + selection_idx) % CACHE_SIZE];
    }

    printf("Select Cached Barcode\n");
    for (int i = 0; i < cache_current_size; ++i) {
      printf("    %d. Text: %s\n", i + 1, g_barcode_cache[(g_c_oldest + i) % CACHE_SIZE]->text);
    }
    printf("    0. Return to main menu\n");
    printf(":-$  ");
    fflush(stdout);

    if (freaduntil(g_input, sizeof(g_input) - 1, '\n', stdin) == -1) {
      printf("Invalid Selection\n");
      fflush(stdout);
      exit(0);
    }

    unsigned long input_val = strtoul(g_input, NULL, 10);
    if (input_val == 0) {
      return NULL; // Return to main menu
    }
    selection_idx = (int)input_val - 1; // Adjust to 0-indexed
  }
}

// Function: input_barcode
void input_barcode(void) {
  int choice = -1;
  Barcode *new_barcode = NULL;
  char message_buffer[256]; // For sprintf messages

  while (choice < 0 || choice > 3) {
    printf("Input barcode as:\n");
    printf("    1. String to encode\n");
    printf("    2. Barcode encoded in ascii (\"|\" and \" \")\n");
    printf("    3. Barcode bitmap\n");
    printf("    0. Return to main menu\n");
    printf(":-$  ");
    fflush(stdout);

    if (freaduntil(g_input, sizeof(g_input) - 1, '\n', stdin) == -1) {
      printf("Invalid Selection\n");
      fflush(stdout);
      exit(0);
    }
    choice = (int)strtoul(g_input, NULL, 10);
  }

  if (choice == 0) {
    return; // Return to main menu
  }

  fflush(stdout); // Flush before potential binary input

  if (choice == 3) { // Barcode bitmap
    char bmp_header_buffer[54]; // Standard BMP header size
    void *bmp_data = NULL;
    size_t image_data_size = 0;

    if (fread(bmp_header_buffer, 1, sizeof(bmp_header_buffer), stdin) != sizeof(bmp_header_buffer)) {
      sprintf(message_buffer, "Bad input (failed to read BMP header)\n");
    } else {
      if (validate_bmp_headers(bmp_header_buffer, &image_data_size) == 0) {
        bmp_data = malloc(sizeof(bmp_header_buffer) + image_data_size);
        if (!bmp_data) {
            sprintf(message_buffer, "Memory allocation failed for BMP data.\n");
        } else {
            memcpy(bmp_data, bmp_header_buffer, sizeof(bmp_header_buffer));

            if (fread((char *)bmp_data + sizeof(bmp_header_buffer), 1, image_data_size, stdin) != image_data_size) {
                sprintf(message_buffer, "Bad input (failed to read BMP image data)\n");
            } else {
                new_barcode = create_barcode_from_bmp(bmp_data);
                if (!new_barcode) {
                  sprintf(message_buffer, "Bad barcode bitmap\n");
                } else {
                  add_to_cache(new_barcode);
                  sprintf(message_buffer, "Successfully added bitmap barcode to cache\nBarcode text: %s\n", new_barcode->text);
                }
            }
        }
      } else {
        sprintf(message_buffer, "Bad input (invalid BMP headers)\n");
      }
    }

    if (bmp_data) {
      free(bmp_data); // Free the temporary BMP data buffer
    }
    printf("%s", message_buffer); // Use %s to prevent format string vulnerabilities
  } else if (choice == 1) { // String to encode
    printf("Enter string:\n:-$ ");
    fflush(stdout);
    if (freaduntil(g_input, sizeof(g_input) - 1, '\n', stdin) == -1) {
      printf("Bad input\n");
      fflush(stdout);
      exit(0);
    }
    new_barcode = create_barcode_from_str(g_input);
    if (!new_barcode) {
      printf("Bad barcode string\n");
    } else {
      add_to_cache(new_barcode);
      printf("Successfully added barcode to cache\n");
      printf("Barcode text: %s\n", new_barcode->text);
    }
  } else if (choice == 2) { // Barcode encoded in ascii
    printf("Enter encoded data:\n:-$ ");
    fflush(stdout);
    if (freaduntil(g_input, sizeof(g_input) - 1, '\n', stdin) == -1) {
      printf("Bad input\n");
      fflush(stdout);
      exit(0);
    }
    new_barcode = create_barcode_from_encoded_data(g_input);
    if (!new_barcode) {
      printf("Bad barcode encoding\n");
    } else {
      add_to_cache(new_barcode);
      printf("Successfully added barcode to cache\n");
      printf("Barcode text: %s\n", new_barcode->text);
    }
  }
}

// Function: view_cached_barcodes
void view_cached_barcodes(void) {
  int cache_current_size = g_replace_oldest ? CACHE_SIZE : g_c_idx;

  printf("Cached Barcodes\n");
  for (int i = 0; i < cache_current_size; ++i) {
    printf("    %d. Text: %s\n", i + 1, g_barcode_cache[(g_c_oldest + i) % CACHE_SIZE]->text);
  }
}

// Function: clear_cached_barcodes
void clear_cached_barcodes(void) {
  int cache_current_size = g_replace_oldest ? CACHE_SIZE : g_c_idx;

  for (int i = 0; i < cache_current_size; ++i) {
    Barcode *barcode_to_free = g_barcode_cache[(g_c_oldest + i) % CACHE_SIZE];
    if (barcode_to_free) { // Ensure it's not NULL
        free(barcode_to_free->text);
        free(barcode_to_free->data);
        free(barcode_to_free);
        g_barcode_cache[(g_c_oldest + i) % CACHE_SIZE] = NULL; // Clear pointer after freeing
    }
  }
  g_replace_oldest = 0;
  g_c_idx = 0;
  g_c_oldest = 0;
}

// Function: check_seed
void check_seed(void) {
  int seed_input_val = 0;
  // fread(ptr, size, count, stream)
  // The original code had fread(local_10, 4, (size_t)stdin, (FILE *)0x12e3d);
  // (size_t)stdin is incorrect for 'count', and (FILE *)0x12e3d is a decompiler artifact.
  // Assuming it meant to read 4 bytes into seed_input_val from stdin.
  fread(&seed_input_val, sizeof(int), 1, stdin);

  if (g_seed_val == seed_input_val) {
    // fwrite(ptr, size, count, stream)
    // The original code had fwrite(&DAT_4347c000, 0x1000, (size_t)stdout, __s);
    // (size_t)stdout is incorrect for 'count', and __s is a decompiler artifact.
    // Assuming it meant to write g_seed_data (4096 bytes) to stdout.
    fwrite(g_seed_data, sizeof(char), sizeof(g_seed_data), stdout);
  }
}

// Function: main
int main(void) {
  // Set stdout to line buffered mode
  setvbuf(stdout, NULL, _IOLBF, 0);

  check_seed();
  printf("Welcome to the EZ Barcode Encoder/Decoder Service\n");

  bool running = true;
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

    if (freaduntil(g_input, sizeof(g_input) - 1, '\n', stdin) == -1) {
      printf("Invalid Selection\n");
      fflush(stdout);
      exit(0);
    }

    long choice = strtol(g_input, NULL, 10);

    switch (choice) {
      case 1:
        input_barcode();
        break;
      case 2: {
        Barcode *selected_barcode = select_from_cache();
        if (selected_barcode) {
          printf("Raw String: ");
          print_barcode_ascii(selected_barcode);
        }
        break;
      }
      case 3: {
        Barcode *selected_barcode = select_from_cache();
        if (selected_barcode) {
          void *bmp_output = create_barcode_bmp(selected_barcode);
          if (!bmp_output) {
            printf("Bad barcode\n");
          } else {
            printf("    Printable Barcode:\n");
            // Cast to BMPHeader_t to access bfSize at offset 2
            fwrite(bmp_output, 1, ((BMPHeader_t *)bmp_output)->bfSize, stdout);
            printf("\n    Barcode String: %s\n", selected_barcode->text);
            free(bmp_output);
          }
        }
        break;
      }
      case 4:
        view_cached_barcodes();
        break;
      case 5:
        clear_cached_barcodes();
        break;
      case 6:
        running = false;
        break;
      default:
        printf("Invalid Selection\n");
        break;
    }
  }

  printf("Thanks for using the EZ Barcode Encoder/Decoder Service\n");
  fflush(stdout);
  return 0;
}