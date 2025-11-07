#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h> // For UINT64_MAX
#include <stdarg.h> // For va_list in error function

// Global variables for range pair management
typedef struct {
    uint64_t start;
    uint64_t end;
} range_pair;

range_pair *frp = NULL;
size_t n_frp = 0;
size_t n_frp_allocated = 0;

// Placeholder for GNU coreutils-like functions and globals
const char DAT_00100b58[] = "-"; // Likely represents the string "-"
char *num_start_0 = NULL; // Global to keep track of the start of a number string

// Minimal implementations for external functions
const char* gettext(const char* msgid) { return msgid; }

void error(int status, int errnum, const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) exit(status);
}

void usage(int status) {
    fprintf(stderr, "Usage: program <options> <fields>\n");
    exit(status);
}

char* quote(const char* s) {
    size_t len = strlen(s);
    char* quoted = (char*)malloc(len + 3); // For '', and null terminator
    if (!quoted) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    quoted[0] = '\'';
    memcpy(quoted + 1, s, len);
    quoted[len + 1] = '\'';
    quoted[len + 2] = '\0';
    return quoted;
}

int streq(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}

int c_isdigit(int c) {
    return isdigit(c);
}

void* ximemdup0(const char* s, size_t len) {
    char *new_s = (char*)malloc(len + 1);
    if (!new_s) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(new_s, s, len);
    new_s[len] = '\0';
    return new_s;
}

// Custom memory allocation function that grows the array
// Doubles capacity when needed.
void* xpalloc_grow(void *ptr, size_t *allocated_count, size_t element_size, size_t current_count) {
    if (current_count == *allocated_count) {
        size_t new_allocated_count = (*allocated_count == 0) ? 4 : (*allocated_count * 2); // Start with 4 elements, then double
        void *new_ptr = realloc(ptr, new_allocated_count * element_size);
        if (!new_ptr) {
            fprintf(stderr, "Memory allocation failed (xpalloc_grow).\n");
            exit(EXIT_FAILURE);
        }
        *allocated_count = new_allocated_count;
        return new_ptr;
    }
    return ptr; // Should not be reached if called correctly
}

// Custom realloc with error checking
void* xrealloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        fprintf(stderr, "Memory allocation failed (xrealloc).\n");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

// Function: add_range_pair
void add_range_pair(uint64_t start_val, uint64_t end_val) {
  if (n_frp == n_frp_allocated) {
    frp = (range_pair*)xpalloc_grow(frp, &n_frp_allocated, sizeof(range_pair), n_frp);
  }
  frp[n_frp].start = start_val;
  frp[n_frp].end = end_val;
  n_frp++;
}

// Function: compare_ranges (for qsort)
int compare_ranges(const void *a, const void *b) {
  const range_pair *range1 = (const range_pair *)a;
  const range_pair *range2 = (const range_pair *)b;
  // This logic returns -1, 0, or 1 based on comparison of start values
  return (unsigned int)(range2->start < range1->start) - (unsigned int)(range1->start < range2->start);
}

// Function: complement_rp
void complement_rp(void) {
  range_pair *old_frp = frp;
  size_t old_n_frp = n_frp;

  frp = NULL; // Reset globals for new range set
  n_frp = 0;
  n_frp_allocated = 0;

  // Add complement for the initial gap (from 1 to the start of the first range)
  if (old_n_frp > 0 && old_frp[0].start > 1) {
    add_range_pair(1, old_frp[0].start - 1);
  }

  // Add complements for gaps between ranges
  for (size_t i = 1; i < old_n_frp; ++i) {
    if (old_frp[i-1].end != UINT64_MAX && old_frp[i-1].end + 1 < old_frp[i].start) {
      add_range_pair(old_frp[i-1].end + 1, old_frp[i].start - 1);
    }
  }

  // Add complement for the final gap, if the last range doesn't extend to UINT64_MAX
  if (old_n_frp > 0 && old_frp[old_n_frp - 1].end != UINT64_MAX) {
    add_range_pair(old_frp[old_n_frp - 1].end + 1, UINT64_MAX);
  }
  
  free(old_frp); // Free the old range array
}

// Function: set_fields
void set_fields(char *field_string, unsigned int flags) {
  char *current_char_ptr = field_string;
  uint64_t current_start_field = 1; // Tracks the start of the current range/field
  uint64_t current_end_field = 0;   // Tracks the number currently being parsed (could be start or end)
  bool has_parsed_start_num = false; // True if a number has been established for current_start_field
  bool has_parsed_end_num = false;   // True if a number has been established for current_end_field (as end of range)
  bool is_parsing_range = false;        // True if a '-' has been encountered, indicating a range
  bool num_parsing_in_progress = false; // True if digits are currently being read

  // Handle initial '-' prefix (e.g., "-5") when CUT_DELIMITED_ONLY flag (1) is set
  if (((flags & 1) != 0) && streq(current_char_ptr, DAT_00100b58)) {
    current_start_field = 1; // Implicitly starts from 1
    has_parsed_start_num = true; // A start number (1) has been established
    is_parsing_range = true; // We are now parsing a range
    current_char_ptr++; // Move past '-'
    // current_end_field remains 0, ready to parse the number after '-'
    // has_parsed_end_num remains false, as no end number has been parsed yet
  }

  while (true) {
    // Process '-' character (indicates a range)
    while (*current_char_ptr == '-') {
      num_parsing_in_progress = false; 
      if (is_parsing_range) { // Already in a range, cannot have another '-' immediately
        error(0, 0, gettext((flags & 4) == 0 ? "invalid field range" : "invalid byte or character range"));
        usage(1);
      }
      is_parsing_range = true;
      current_char_ptr++;
      
      // If a number was previously parsed (e.g., "5-")
      if (has_parsed_start_num && current_end_field == 0) { 
        // This condition implies a '0' was parsed as a field number, which is invalid.
        // Or it indicates a malformed input like "0-" if 0 were allowed to be parsed.
        error(0, 0, gettext((flags & 4) == 0 ? "fields are numbered from 1" : "byte/character positions are numbered from 1"));
        usage(1);
      }
      current_start_field = has_parsed_start_num ? current_end_field : 1; 
      current_end_field = 0; // Reset to 0 to parse the END of the range
      has_parsed_start_num = false; // Reset, as we are now looking for the end number of the range
    }

    // Process ',' or '\0' (end of a field/range definition)
    if (*current_char_ptr == ',' || *current_char_ptr == '\0') {
      num_parsing_in_progress = false; 

      if (is_parsing_range) { // If a range was being parsed (e.g., "5-", "5-10")
        is_parsing_range = false; // Range processing finished

        if (!has_parsed_start_num && !has_parsed_end_num) { 
          // This case: only "-" was seen (e.g., "-," or "-")
          if ((flags & 1) == 0) { // If not CUT_DELIMITED_ONLY, '-' alone is invalid
            error(0, 0, gettext("invalid range with no endpoint: -"));
            usage(1);
          } else { // For CUT_DELIMITED_ONLY, '-' means "all fields"
            current_start_field = 1; // Treat as "1-"
          }
        }

        if (has_parsed_end_num) { // If an end number was parsed (e.g., "5-10")
          if (current_end_field < current_start_field) {
            error(0, 0, gettext("invalid decreasing range"));
            usage(1);
          }
          add_range_pair(current_start_field, current_end_field);
        } else { // If no end number was parsed (e.g., "5-", or just "-")
          add_range_pair(current_start_field, UINT64_MAX); // Range goes to infinity
        }
      } else { // Not a range, single field (e.g., "5")
        if (current_end_field == 0) { // No number seen, just a comma or initial state (e.g., ",5" or empty string)
          error(0, 0, gettext((flags & 4) == 0 ? "fields are numbered from 1" : "byte/character positions are numbered from 1"));
          usage(1);
        }
        add_range_pair(current_end_field, current_end_field); // Add single field as a range
      }

      current_end_field = 0; // Reset for next field/range
      has_parsed_start_num = false; // Reset flags
      has_parsed_end_num = false;

      if (*current_char_ptr == '\0') { // End of input string, finalize processing
        if (n_frp == 0) {
          error(0, 0, gettext((flags & 4) == 0 ? "missing list of fields" : "missing list of byte/character positions"));
          usage(1);
        }

        qsort(frp, n_frp, sizeof(range_pair), compare_ranges);

        // Merge overlapping/adjacent ranges
        size_t i = 0;
        while (i < n_frp - 1) {
            if (frp[i+1].start <= frp[i].end + 1) { // Overlap or adjacent
                frp[i].end = (frp[i].end > frp[i+1].end) ? frp[i].end : frp[i+1].end;
                memmove(&frp[i+1], &frp[i+2], (n_frp - (i + 2)) * sizeof(range_pair));
                n_frp--;
            } else {
                i++;
            }
        }

        if ((flags & 2) != 0) { // If complement flag is set
          complement_rp();
        }

        // Add a sentinel range [UINT64_MAX, UINT64_MAX] at the end
        add_range_pair(UINT64_MAX, UINT64_MAX); 
        
        return; // Function ends
      }
      current_char_ptr++; // Move past ','
      continue; // Continue parsing the string
    }

    // If not '-', ',' or '\0', then it must be a digit or an invalid character.
    if (!c_isdigit((unsigned char)*current_char_ptr)) {
      char* quoted_char = quote(current_char_ptr);
      error(0, 0, gettext((flags & 4) == 0 ? "invalid field value %s" : "invalid byte/character position %s"), quoted_char);
      free(quoted_char);
      usage(1);
    }

    // It's a digit, parse the number
    if (!num_parsing_in_progress || num_start_0 == NULL) {
      num_start_0 = current_char_ptr;
    }
    num_parsing_in_progress = true;

    uint64_t digit_val = *current_char_ptr - '0';

    // Overflow check for current_end_field = current_end_field * 10 + digit_val
    if (UINT64_MAX / 10 < current_end_field || (UINT64_MAX / 10 == current_end_field && UINT64_MAX % 10 < digit_val)) {
      size_t num_len = strspn(num_start_0, "0123456789");
      char* num_str_copy = ximemdup0(num_start_0, num_len);
      char* quoted_num = quote(num_str_copy);
      error(0, 0, gettext((flags & 4) == 0 ? "field number %s is too large" : "byte/character offset %s is too large"), quoted_num);
      free(num_str_copy);
      free(quoted_num);
      usage(1);
    }
    current_end_field = current_end_field * 10 + digit_val;

    // Set flags based on whether this number is a start or end of a range
    if (is_parsing_range) {
      has_parsed_end_num = true;
    } else {
      has_parsed_start_num = true;
    }

    current_char_ptr++; 
  }
}

// Minimal main function for compilation and demonstration
int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(1);
    }

    // Example flags for param_2 in set_fields:
    // 0: Default behavior
    // 1: CUT_DELIMITED_ONLY (enables special handling for leading '-' and single '-' range)
    // 2: COMPLEMENT_RANGES (calculates the complement of the ranges)
    // 4: BYTE_OR_CHAR_RANGE (changes error messages to refer to bytes/characters instead of fields)

    unsigned int flags = 0;
    if (argc > 2) {
        flags = (unsigned int)atoi(argv[2]);
    }

    set_fields(argv[1], flags);

    printf("Parsed Ranges (total %zu ranges):\n", n_frp);
    for (size_t i = 0; i < n_frp; ++i) {
        if (frp[i].end == UINT64_MAX) {
            printf("[%llu, MAX]\n", frp[i].start);
        } else {
            printf("[%llu, %llu]\n", frp[i].start, frp[i].end);
        }
    }

    free(frp); // Free allocated memory
    frp = NULL;
    n_frp = 0;
    n_frp_allocated = 0;
    // num_start_0 points to memory owned by argv, no need to free.

    return 0;
}