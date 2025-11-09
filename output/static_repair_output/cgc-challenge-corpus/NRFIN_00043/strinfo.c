#include <ctype.h>   // For isprint, isdigit, isalpha
#include <stdbool.h> // For _Bool type
#include <string.h>  // For memset

// Define the structure for analysis results
// Offsets are derived from the original code's pointer arithmetic.
typedef struct {
    unsigned int hash_value;       // offset 0x0
    short total_chars;             // offset 0x4
    short actual_words;            // offset 0x6
    short sentences;               // offset 0x8
    short paragraphs;              // offset 0xA (10)
    short printable_chars;         // offset 0xC (12)
    short non_printable_chars;     // offset 0xE (14)
    short letters;                 // offset 0x10 (16)
    short digits;                  // offset 0x12 (18)
    short other_printables;        // offset 0x14 (20)
    unsigned short session_id;     // offset 0x16 (22)
} AnalysisResult;

// Placeholder for DAT_4347c000 lookup table
// The largest index accessed is `lookup_index + 9`.
// `lookup_index` can be up to `0xff5` (from `char % 0xff6`).
// So, `0xff5 + 9 = 0xffe`. An array of 2048 (0x800) bytes is sufficient.
static const unsigned char DAT_4347c000[2048] = {0}; // Initialize with zeros or actual data if known

// Placeholder for other global byte constants
static const unsigned char DAT_4347cbb8 = 0xAA; // Example value, replace with actual if known
static const unsigned char DAT_4347c7d0 = 0xBB; // Example value, replace with actual if known
static const unsigned char DAT_4347c3e8 = 0xCC; // Example value, replace with actual if known

// Helper functions (returning char 0 or 1 for boolean, consistent with original usage)
static char is_non_printable(char c) {
    return !isprint((unsigned char)c);
}

static char is_printable(char c) {
    return isprint((unsigned char)c);
}

static char is_digit(char c) {
    return isdigit((unsigned char)c);
}

static char is_letter(char c) {
    return isalpha((unsigned char)c);
}

// Function: compute_char_type
unsigned int compute_char_type(const char *text, char terminator, AnalysisResult *result) {
  for (const char *current_char = text; terminator != *current_char; current_char++) {
    if (is_non_printable(*current_char)) {
      result->non_printable_chars++;
    } else {
      // is_printable(*current_char) is redundant here because it's already known to be printable
      // if it's not non-printable. However, keeping the structure similar to original if-else.
      if (is_printable(*current_char)) {
        result->printable_chars++;
        if (is_digit(*current_char)) {
          result->digits++;
        } else if (is_letter(*current_char)) {
          result->letters++;
        } else {
          result->other_printables++;
        }
      }
    }
  }
  return 0;
}

// Function: is_sentence_end_char
char is_sentence_end_char(char c) {
  return (c == '.') || (c == '!') || (c == '?');
}

// Function: is_word_end_char
char is_word_end_char(char c) {
  // Original logic: `(param_1 != ' ') && (cVar1 = is_sentence_end_char((int)param_1), cVar1 != '\x01'))`
  // then `return 0;` else `return 1;`. This is equivalent to:
  // `if (c == ' ' || is_sentence_end_char(c))` then `return 1;` else `return 0;`
  return (c == ' ') || is_sentence_end_char(c);
}

// Function: is_paragraph_end_char
_Bool is_paragraph_end_char(char c) {
  return c == '\n';
}

// Function: compute_grammar_components
void compute_grammar_components(const char *text, char terminator, AnalysisResult *result) {
  char prev_char = '\0';
  for (const char *current_char = text; terminator != *current_char; current_char++) {
    result->total_chars++;
    if (is_word_end_char(*current_char)) {
      if (is_letter(prev_char)) {
        result->actual_words++;
      }
    }
    // The original code checks is_sentence_end_char(local_11) (prev_char)
    // then checks *local_10 (current_char) for ' ' or '\n'.
    // This means a sentence is counted when a space or newline follows a sentence-ending char.
    if (is_sentence_end_char(prev_char)) {
      if (*current_char == ' ') {
        result->sentences++;
      } else if (is_paragraph_end_char(*current_char)) {
        result->sentences++;
        result->paragraphs++;
      }
    }
    prev_char = *current_char;
  }
}

// Function: compute_hash
void compute_hash(const unsigned char *text, unsigned char terminator, unsigned int *hash_output) {
  unsigned int hash_accumulator = 0;
  unsigned short char_index = 0;

  for (const unsigned char *current_char = text; terminator != *current_char; current_char++) {
    // This accesses bytes of hash_accumulator (assuming little-endian)
    ((unsigned char*)&hash_accumulator)[char_index & 3] ^= *current_char;
    char_index++;
  }

  // Initial byte values from the accumulator
  // Assuming little-endian: byte0 is LSB, byte3 is MSB.
  unsigned char byte0 = ((unsigned char*)&hash_accumulator)[0];
  unsigned char byte1 = ((unsigned char*)&hash_accumulator)[1];
  unsigned char byte2 = ((unsigned char*)&hash_accumulator)[2];
  unsigned char byte3 = ((unsigned char*)&hash_accumulator)[3];

  // The first character of the text is used for indexing into DAT_4347c000
  unsigned short lookup_index = (*text) % 0xff6;

  // Apply transformations using DAT_4347c000
  unsigned char final_byte0 = DAT_4347c000[lookup_index + 9] ^ byte0;
  unsigned char final_byte1 = DAT_4347c000[lookup_index + 7] ^ byte1;
  unsigned char final_byte2 = DAT_4347c000[lookup_index + 3] ^ byte2;
  unsigned char final_byte3 = DAT_4347c000[lookup_index] ^ byte3;

  // Reconstruct the hash value from bytes.
  // The original CONCAT operations imply a big-endian reconstruction:
  // CONCAT13(B3, CONCAT12(B2, CONCAT11(B1, B0)))
  // where B3 is MSB, B0 is LSB.
  *hash_output = ((unsigned int)final_byte3 << 24) |
                 ((unsigned int)final_byte2 << 16) |
                 ((unsigned int)final_byte1 << 8) |
                 ((unsigned int)final_byte0);
}

// Function: compute_session_id
void compute_session_id(AnalysisResult *result) {
  // Original: local_e = CONCAT11(DAT_4347cbb8 ^ DAT_4347c7d0, DAT_4347c3e8 ^ DAT_4347c000);
  // Assuming DAT_4347c000 refers to its first byte when used as a single byte value.
  unsigned char byte_high = DAT_4347cbb8 ^ DAT_4347c7d0;
  unsigned char byte_low = DAT_4347c3e8 ^ DAT_4347c000[0];

  result->session_id = ((unsigned short)byte_high << 8) | byte_low;
}

// Function: process
unsigned int process(const char *text, char terminator, AnalysisResult *result) {
  // Initialize result struct to zeros before computation
  memset(result, 0, sizeof(AnalysisResult));

  // The original call had an extra argument (0x11870) for compute_char_type,
  // which is omitted here as it doesn't fit the inferred C signature.
  compute_char_type(text, terminator, result);
  compute_grammar_components(text, terminator, result);
  // Pass the address of the hash_value member where the hash should be stored
  compute_hash((const unsigned char *)text, (unsigned char)terminator, &result->hash_value);
  compute_session_id(result);
  return 0;
}