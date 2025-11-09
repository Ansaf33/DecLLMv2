#include <stdio.h>   // For FILE, fprintf, fflush, fread, fwrite, stdin, stdout
#include <stdlib.h>  // For strtoul, NULL
#include <string.h>  // For memset, memcpy, strcmp, sprintf, snprintf
#include <stdint.h>  // For uint32_t, uintptr_t, size_t (though size_t is in stddef.h/stdlib.h)

// Dummy declarations for external functions, assuming their signatures
// match common usage patterns in the original snippet.
// These are placeholders to make the code compilable.
static unsigned int BytesToUnsigned(unsigned int low, unsigned int high) {
    // Placeholder implementation: Combines two 32-bit parts into a 64-bit unsigned long,
    // then truncates to 32-bit unsigned int if the return type is 'unsigned int'.
    // If the intent was a 64-bit result, the return type should be unsigned long.
    // Given the original 'uVar1 = BytesToUnsigned(...)' and 'uVar1' being 'undefined4',
    // it implies a 32-bit return.
    return (unsigned int)(((unsigned long)high << 32) | low);
}

static const char* SeverityString(unsigned int severity_code) {
    // Placeholder implementation for severity code to string conversion.
    switch (severity_code) {
        case 0: return "INFO";
        case 1: return "WARNING";
        case 2: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Structure definition based on access patterns in ReportMatches and CompareSignatureEnclosedInList.
// The original code implies a 32-bit environment where pointers and size_t might be 4 bytes.
// For modern Linux (typically 64-bit), char* and size_t are 8 bytes.
// This struct uses standard types, assuming the decompiler's 'undefined4' and 'int' for pointers
// were artifacts of a 32-bit view that should map to correct C types for 64-bit compilation.
typedef struct {
    unsigned int severity;
    char* signature_ptr;
    size_t signature_len;
    unsigned int bytes_low;
    unsigned int bytes_high;
} MatchEntry;

// Custom freaduntil function, as it's non-standard in the original snippet.
// Reads into buffer until delimiter or max_len-1 bytes are read.
// Null-terminates the buffer. Returns number of bytes read (excluding null terminator), or -1 on error.
static int freaduntil(void *buffer, size_t max_len, int delimiter, FILE *stream) {
    char *buf = (char *)buffer;
    size_t count = 0;
    int c;

    if (max_len == 0) return 0; // No space in buffer

    while (count < max_len - 1) { // Leave space for null terminator
        c = fgetc(stream);
        if (c == EOF) {
            break; // EOF reached
        }
        buf[count++] = (char)c;
        if (c == delimiter) {
            break;
        }
    }
    buf[count] = '\0'; // Null-terminate the string
    return (int)count;
}

// Global variables from decompiled output, declared as static to limit scope.
static unsigned int _DAT_4347c000 = 0; // Placeholder for a single integer
static unsigned char DAT_4347c000[0x1000] = {0}; // Placeholder for a 4KB data block

// Function: ReportMatches
void ReportMatches(FILE *output_stream, MatchEntry **matches, unsigned int num_matches) {
  char buffer_output[512];
  size_t current_offset = 0;

  memset(buffer_output, 0, sizeof(buffer_output));

  for (size_t i = 0; i < num_matches; ++i) {
    MatchEntry *entry = matches[i];

    // Copy signature string, ensuring no buffer overflow
    size_t remaining_buffer_space = sizeof(buffer_output) - current_offset - 1; // -1 for null terminator
    size_t bytes_to_copy = entry->signature_len;
    if (bytes_to_copy > remaining_buffer_space) {
        bytes_to_copy = remaining_buffer_space;
    }
    memcpy(buffer_output + current_offset, entry->signature_ptr, bytes_to_copy);
    current_offset += bytes_to_copy;
    buffer_output[current_offset] = '\0'; // Ensure null-termination after copy

    unsigned int combined_bytes = BytesToUnsigned(entry->bytes_low, entry->bytes_high);
    const char* severity_str = SeverityString(entry->severity);

    // Format and append the rest of the match info, ensuring no buffer overflow
    int chars_written = snprintf(buffer_output + current_offset, sizeof(buffer_output) - current_offset,
                                 " - %s - %x\n", severity_str, combined_bytes);
    if (chars_written > 0) {
        current_offset += chars_written;
    } else if (chars_written < 0) {
        // Encoding error or other snprintf failure
        break;
    }
  }
  fprintf(output_stream, "%s", buffer_output);
}

// Function: ReadLine
// Reads a line from the stream into the buffer, up to buffer_size-1 characters.
// Strips the trailing newline character if present. Returns number of bytes read (excluding newline and null terminator), or -1 on error.
int ReadLine(FILE *stream, void *buffer, size_t buffer_size) {
  memset(buffer, 0, buffer_size);
  fflush(stdout); // Original code flushes stdout
  int bytes_read = freaduntil(buffer, buffer_size, '\n', stream);
  
  if (bytes_read < 0) {
    return -1; // Error during read
  } else {
    // If a newline was read, strip it
    if (bytes_read > 0 && ((char*)buffer)[bytes_read - 1] == '\n') {
        ((char*)buffer)[bytes_read - 1] = '\0';
        return bytes_read - 1; // Return length without newline
    }
  }
  return bytes_read;
}

// Function: ReadExactlyNBytes
// Reads exactly 'count' items of 'element_size' bytes from 'stream' into 'buffer'.
// Returns 0 on success, -1 on error (or if not enough bytes were read).
int ReadExactlyNBytes(FILE *stream, void *buffer, size_t element_size, size_t count) {
  fflush(stdout); // Original code flushes stdout
  size_t items_read = fread(buffer, element_size, count, stream);
  if (items_read != count) {
    return -1; // Not enough items read or error
  }
  return 0; // Success
}

// Function: ReadNByteLine
// Reads 'num_bytes' bytes into 'buffer', then reads the *next* character to check if it's a newline.
// Returns 0 if num_bytes were successfully read and the next character was a newline.
// Returns 1 if num_bytes were successfully read and the next character was NOT a newline (or EOF).
// Returns -1 on error during the initial num_bytes read.
int ReadNByteLine(FILE *stream, void *buffer, size_t num_bytes) {
  int read_status = ReadExactlyNBytes(stream, buffer, 1, num_bytes); // Read num_bytes bytes, element size 1
  if (read_status != 0) { // ReadExactlyNBytes returns 0 for success
    return -1;
  }
  
  char next_char_buf[1];
  int peek_char = fgetc(stream); // Read the next character
  if (peek_char == EOF) {
      return 1; // No newline found (EOF)
  }
  // The original code implies consuming this character, so we do.
  next_char_buf[0] = (char)peek_char;

  return (next_char_buf[0] != '\n'); // 0 if newline, 1 if not newline
}

// Function: ReadUnsigned
// Reads a line from the stream, attempts to parse it as an unsigned long.
// Returns the parsed unsigned long value, or -1 (as unsigned long) on error.
unsigned long ReadUnsigned(FILE *stream, char *buffer, size_t buffer_size) {
  int read_status = ReadLine(stream, buffer, buffer_size);
  if (read_status < 0) {
    return (unsigned long)-1; // Using (unsigned long)-1 as an error indicator
  }
  return strtoul(buffer, NULL, 10);
}

// Function: CompareSignatureEnclosedInList
// Comparator function for sorting MatchEntry pointers based on their signature_ptr string.
// Designed to be compatible with standard C library sort functions like qsort.
int CompareSignatureEnclosedInList(const void *a_ptr, const void *b_ptr) {
  // Cast void pointers to pointers to (const MatchEntry *), then dereference.
  const MatchEntry *const *match_a_ptr = (const MatchEntry *const *)a_ptr;
  const MatchEntry *const *match_b_ptr = (const MatchEntry *const *)b_ptr;

  const MatchEntry *match_a = *match_a_ptr;
  const MatchEntry *match_b = *match_b_ptr;
  
  return strcmp(match_a->signature_ptr, match_b->signature_ptr);
}

// Function: SortArray
// Implements a bubble sort algorithm. Sorts an array of 'num_elements' items,
// each of 'element_size' bytes, using the provided 'compare' function.
// Returns 0 on success.
int SortArray(void *base, size_t num_elements, size_t element_size, 
              int (*compare)(const void *, const void *)) {
  
  // Allocate a temporary buffer for swapping elements. Using VLA (C99 standard).
  char temp_buf[element_size]; 
  
  size_t n = num_elements; // 'n' tracks the unsorted portion of the array

  while (n > 0) {
    size_t last_swap_idx = 0; // Index of the last element swapped in this pass
    for (size_t i = 1; i < n; ++i) {
      // Calculate pointers to the current pair of elements to compare
      void *elem1 = (char *)base + (i - 1) * element_size;
      void *elem2 = (char *)base + i * element_size;

      if (compare(elem1, elem2) > 0) { // If elem1 is greater than elem2, swap them
        memcpy(temp_buf, elem1, element_size);
        memcpy(elem1, elem2, element_size);
        memcpy(elem2, temp_buf, element_size);
        last_swap_idx = i; // Update the last swap position
      }
    }
    // After a pass, all elements from 'last_swap_idx' to 'n-1' are sorted.
    // So, the next pass only needs to sort up to 'last_swap_idx'.
    n = last_swap_idx; 
  }
  return 0; // Original returns undefined4, 0 is a common success code.
}

// Function: check_seed
void check_seed(void) {
  int seed_value = 0; // Variable to store the read seed value

  // Corrected fread arguments: reads 1 item of sizeof(int) bytes from stdin.
  fread(&seed_value, sizeof(int), 1, stdin); 
  
  if (_DAT_4347c000 == (unsigned int)seed_value) {
    // Corrected fwrite arguments: writes 1 item of sizeof(DAT_4347c000) bytes from DAT_4347c000 to stdout.
    fwrite(DAT_4347c000, 1, sizeof(DAT_4347c000), stdout); 
  }
}