#include <stdio.h>   // For printf, fflush, fread, fwrite, stdin, stdout, FILE, fgetc, perror
#include <stdlib.h>  // For malloc, free, realloc, strtol
#include <string.h>  // For strlen, memcpy
#include <stdint.h>  // For uint32_t, uint8_t
#include <stddef.h>  // For size_t

// --- Type definitions from decompiler output ---
typedef unsigned int uint;
typedef uint8_t byte;
typedef uint32_t undefined4;

// --- Dummy definitions for missing external types/functions ---

// Dummy struct for EVP_PKEY_CTX (from OpenSSL, not fully used in snippet)
struct EVP_PKEY_CTX;
typedef struct EVP_PKEY_CTX EVP_PKEY_CTX;

// Dummy allocate function
void* allocate(size_t size, size_t count, void** ptr_out) {
    *ptr_out = malloc(size * count);
    if (*ptr_out == NULL) {
        perror("Memory allocation failed in allocate");
    }
    return *ptr_out;
}

// Dummy freaduntil function
// This is a simplified version. It reads characters into buf until buf_len-1 characters are read,
// or 'delimiter' is encountered, or EOF.
// It returns the number of characters read (excluding delimiter).
int freaduntil(char *buf, int buf_len, char delimiter, FILE *stream) {
    int i = 0;
    int c;
    while (i < buf_len - 1 && (c = fgetc(stream)) != EOF && c != delimiter) {
        buf[i++] = (char)c;
    }
    buf[i] = '\0'; // Null-terminate the buffer
    if (c == EOF && i == 0) { // If EOF immediately and nothing read
        return -1;
    }
    // If delimiter was read, it's consumed from the stream.
    return i;
}

// --- Global variables from decompiler output ---
unsigned char *selection_sort_memory;
unsigned char selection_sort_bytes[0x157] = {0}; // Placeholder, assume these are initialized elsewhere
unsigned char *selection_sort_unpacked;

unsigned char *heap_propagate_memory;
unsigned char heap_propagate_bytes[0x1e0] = {0}; // Placeholder
unsigned char *heap_propagate_unpacked;

unsigned char *heapify_memory;
unsigned char heapify_bytes[0x2b7] = {0}; // Placeholder
unsigned char *heapify_unpacked;

unsigned char *heap_sort_memory;
unsigned char heap_sort_bytes[0x12a] = {0}; // Placeholder
unsigned char *heap_sort_unpacked;

unsigned char *merge_helper_memory;
unsigned char merge_helper_bytes[0x1ef] = {0}; // Placeholder
unsigned char *merge_helper_unpacked;

unsigned char *merge_sort_memory;
unsigned char merge_sort_bytes[400] = {0}; // Placeholder
unsigned char *merge_sort_unpacked;

unsigned char *insertion_sort_memory;
unsigned char insertion_sort_bytes[0x15d] = {0}; // Placeholder
unsigned char *insertion_sort_unpacked;

int _DAT_4347c000 = 0x12345678; // Example seed value
char DAT_4347c000[0x1000] = "This is some dummy data that would be written if the seed matches. "
                            "It's a placeholder for 4096 bytes of data.\n"; // Example data

// Function: unpack
void unpack(unsigned char *data, unsigned int data_len) {
    const char key[] = "CS10FUN!"; // Represents 0x30315343, 0x214e5546, 0
    size_t key_len = strlen(key);

    for (unsigned int i = 0; i < data_len; ++i) {
        data[i] ^= key[i % key_len];
    }
    return;
}

// Function: init
int init(EVP_PKEY_CTX *ctx) {
  // Note: ctx parameter is unused in the provided snippet, so no dereferencing issues.
  allocate(0x157,1,(void**)&selection_sort_memory);
  memcpy(selection_sort_memory,selection_sort_bytes,0x157);
  unpack(selection_sort_memory,0x157);
  selection_sort_unpacked = selection_sort_memory;

  allocate(0x1e0,1,(void**)&heap_propagate_memory);
  memcpy(heap_propagate_memory,heap_propagate_bytes,0x1e0);
  unpack(heap_propagate_memory,0x1e0);
  heap_propagate_unpacked = heap_propagate_memory;

  allocate(0x2b7,1,(void**)&heapify_memory);
  memcpy(heapify_memory,heapify_bytes,0x2b7);
  unpack(heapify_memory,0x2b7);
  heapify_unpacked = heapify_memory;

  allocate(0x12a,1,(void**)&heap_sort_memory);
  memcpy(heap_sort_memory,heap_sort_bytes,0x12a);
  unpack(heap_sort_memory,0x12a);
  heap_sort_unpacked = heap_sort_memory;

  allocate(0x1ef,1,(void**)&merge_helper_memory);
  memcpy(merge_helper_memory,merge_helper_bytes,0x1ef);
  unpack(merge_helper_memory,0x1ef);
  merge_helper_unpacked = merge_helper_memory;

  allocate(400,1,(void**)&merge_sort_memory);
  memcpy(merge_sort_memory,merge_sort_bytes,400);
  unpack(merge_sort_memory,400);
  merge_sort_unpacked = merge_sort_memory;

  allocate(0x15d,1,(void**)&insertion_sort_memory);
  memcpy(insertion_sort_memory,insertion_sort_bytes,0x15d);
  unpack(insertion_sort_memory,0x15d);
  insertion_sort_unpacked = insertion_sort_memory;
  return 0x199ac;
}

// Function: swap
void swap(undefined4 *a, undefined4 *b) {
  undefined4 temp = *a;
  *a = *b;
  *b = temp;
  return;
}

// Function: print_array
void print_array(int *arr, unsigned int count) {
  if (arr == NULL || count == 0) {
    printf("Empty Array\n");
  } else {
    printf("Array data\n[");
    for (unsigned int i = 0; i < count; ++i) {
      printf("%d", arr[i]);
      if (i < count - 1) {
        printf(", ");
      } else {
        printf("]\n");
      }
    }
  }
  return;
}

// Function: create_number_array
void * create_number_array(int *num_elements_ptr) {
    int *arr = NULL;
    int capacity = 2; // Initial capacity for 2 integers
    int current_elements = 0;
    char input_buffer[64]; // Max 63 chars + null terminator
    int buffer_idx = 0;

    arr = (int*)malloc(capacity * sizeof(int));
    if (arr == NULL) {
        perror("malloc failed for initial array");
        *num_elements_ptr = 0;
        return NULL;
    }

    printf("Enter a list of numbers to sort. End the list with \';;\'\n");
    fflush(stdout);

    while (buffer_idx < sizeof(input_buffer) - 1) { // Leave space for null terminator
        int c = fgetc(stdin); // Read one character
        if (c == EOF) {
            // If EOF before ';;', process any number in buffer
            if (buffer_idx > 0) {
                input_buffer[buffer_idx] = '\0';
                long val = strtol(input_buffer, NULL, 10);
                if (current_elements == capacity) {
                    capacity *= 2;
                    int *new_arr = (int*)realloc(arr, capacity * sizeof(int));
                    if (new_arr == NULL) {
                        perror("realloc failed");
                        free(arr);
                        *num_elements_ptr = 0;
                        return NULL;
                    }
                    arr = new_arr;
                }
                arr[current_elements++] = (int)val;
            }
            break;
        }

        input_buffer[buffer_idx] = (char)c;

        // Check for ';;' sequence
        if (buffer_idx > 0 && input_buffer[buffer_idx] == ';' && input_buffer[buffer_idx - 1] == ';') {
            input_buffer[buffer_idx - 1] = '\0'; // Null terminate before the first ';'
            // Process the last number if any before ';;'
            if (buffer_idx - 1 > 0) { // Check if there's actual data before ';;'
                long val = strtol(input_buffer, NULL, 10);
                if (current_elements == capacity) {
                    capacity *= 2;
                    int *new_arr = (int*)realloc(arr, capacity * sizeof(int));
                    if (new_arr == NULL) {
                        perror("realloc failed");
                        free(arr);
                        *num_elements_ptr = 0;
                        return NULL;
                    }
                    arr = new_arr;
                }
                arr[current_elements++] = (int)val;
            }
            break; // End of input
        }

        // Check for comma delimiter
        if (input_buffer[buffer_idx] == ',') {
            input_buffer[buffer_idx] = '\0'; // Null terminate to parse number
            if (buffer_idx != 0) { // Only process if something was typed before comma
                long val = strtol(input_buffer, NULL, 10);
                if (current_elements == capacity) {
                    capacity *= 2;
                    int *new_arr = (int*)realloc(arr, capacity * sizeof(int));
                    if (new_arr == NULL) {
                        perror("realloc failed");
                        free(arr);
                        *num_elements_ptr = 0;
                        return NULL;
                    }
                    arr = new_arr;
                }
                arr[current_elements++] = (int)val;
            }
            buffer_idx = 0; // Reset buffer for next number
        } else {
            buffer_idx++;
        }
    }

    // Clear any remaining input on the line (e.g., if ';;' was not at the very end of the line)
    int c;
    while ((c = fgetc(stdin)) != EOF && c != '\n');

    *num_elements_ptr = current_elements;
    return arr;
}

// Function: multiply_array
void multiply_array(void **array_ptr_ptr, int *array_count_ptr) {
    char input_buffer[64]; // For multiplier input
    int original_count = *array_count_ptr;
    int *original_arr = (int*)*array_ptr_ptr;
    int *new_arr = NULL;
    long multiplier = 0;

    if (original_count >= 0x2711) { // 10001
        printf("Array is too long. Can't multiply any more\n");
        // Clear any remaining input on the line
        int c;
        while ((c = fgetc(stdin)) != EOF && c != '\n');
        return;
    }

    // Only proceed if there's an array to multiply
    if (original_arr == NULL || original_count == 0) {
        return; // No array or empty array means no multiplication
    }

    printf("Quick Grow! Enter a list multiplier. End number with \';\'\n");
    fflush(stdout);

    if (freaduntil(input_buffer, sizeof(input_buffer), ';', stdin) != -1) {
        multiplier = strtol(input_buffer, NULL, 10);

        if (multiplier == 0 || multiplier > 10000) {
            printf("Multiplier too big, try again\n");
            // Clear the rest of the line after invalid input
            int c;
            while ((c = fgetc(stdin)) != EOF && c != '\n');
            return;
        }

        long new_count_long = (long)multiplier * original_count;
        // Check for potential overflow of int or exceeding max allowed size
        if (new_count_long < 0 || new_count_long > 0x2711 || new_count_long > (long)0x7FFFFFFF / sizeof(int)) {
             printf("Resulting array size too large or would overflow, try again\n");
             int c;
             while ((c = fgetc(stdin)) != EOF && c != '\n');
             return;
        }
        int new_count = (int)new_count_long;

        new_arr = (int*)malloc(new_count * sizeof(int));
        if (new_arr == NULL) {
            perror("malloc failed for multiplied array");
            // Clear the rest of the line
            int c;
            while ((c = fgetc(stdin)) != EOF && c != '\n');
            return;
        }

        for (int i = 0; i < multiplier; ++i) {
            memcpy(&new_arr[i * original_count], original_arr, original_count * sizeof(int));
        }

        *array_count_ptr = new_count;
        free(original_arr);
        *array_ptr_ptr = new_arr;
    }

    // Clear any remaining input on the line
    int c;
    while ((c = fgetc(stdin)) != EOF && c != '\n');
    return;
}

// Function: check_seed
void check_seed(void) {
  int seed_input = 0;
  // Read 4 bytes (size of int) from stdin
  fread(&seed_input, sizeof(int), 1, stdin);

  if (_DAT_4347c000 == seed_input) {
    // Write 0x1000 bytes from DAT_4347c000 to stdout
    fwrite(DAT_4347c000, 1, sizeof(DAT_4347c000), stdout);
  }
  fflush(stdout);
  return;
}

// Dummy main function to make the code compilable and runnable as a standalone program
int main() {
    printf("--- Initializing ---\n");
    init(NULL); // Pass NULL as ctx is unused

    printf("\n--- Create Number Array ---\n");
    int array_size = 0;
    int *my_array = (int*)create_number_array(&array_size);
    print_array(my_array, array_size);

    printf("\n--- Multiply Array ---\n");
    multiply_array((void**)&my_array, &array_size);
    print_array(my_array, array_size);

    printf("\n--- Check Seed (enter a 4-byte integer, e.g., 0x%x) ---\n", _DAT_4347c000);
    check_seed();

    if (my_array != NULL) {
        free(my_array);
        my_array = NULL;
    }

    // Free memory allocated in init
    free(selection_sort_memory);
    free(heap_propagate_memory);
    free(heapify_memory);
    free(heap_sort_memory);
    free(merge_helper_memory);
    free(merge_sort_memory);
    free(insertion_sort_memory);

    printf("\n--- Program finished ---\n");
    return 0;
}