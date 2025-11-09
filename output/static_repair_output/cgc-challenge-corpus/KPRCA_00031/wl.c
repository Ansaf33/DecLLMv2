#include <stdlib.h>  // For calloc, realloc, free
#include <string.h>  // For strlen, strncpy, strcat, strcpy, memset
#include <ctype.h>   // For isspace, isprint
#include <stdbool.h> // For bool
#include <stdint.h>  // For uint32_t, uint64_t

// Assume these are defined elsewhere or are part of a specific environment (CGC)
// Placeholder for external functions/variables to ensure compilability
extern void _error(int, const char *, int);
extern const char *word_sep;
extern const char *chain_term;

// List manipulation functions (assuming they return list head or status)
// append_list's data argument is char*, but sometimes wordlist_t is cast to char*.
// This implies the list can store different types, or the value itself is stored.
extern uint32_t append_list(uint32_t *list_head, char *data, int type);
extern uint32_t len_list(uint32_t list_head);
extern uint32_t copy_list(uint32_t list_head, uint32_t start_idx, uint32_t end_idx);
extern uint32_t lindex(uint32_t list_head, int index); // index as int, 0xffffffff implies -1

// Tree manipulation functions (assuming they return tree head or status)
extern uint32_t init_tree(uint64_t key, uint32_t value); // Returns tree head
extern int ins_tree(uint32_t tree_head, uint64_t key, uint32_t value); // Returns status (<0 for error)

// Type aliases for clarity and to match original undefined types
typedef uint32_t wordlist_t; // Represents a list head
typedef uint32_t tree_t;     // Represents a tree head

// Macro to check for carry on 32-bit addition
#define CARRY4(a, b) ((uint32_t)(a) + (uint32_t)(b) < (uint32_t)(a))

// Macro to concatenate two 32-bit uints into a 64-bit uint
#define CONCAT44(high, low) (((uint64_t)(high) << 32) | (uint32_t)(low))


// Function: word_list_to_str
char * word_list_to_str(uint32_t *param_1) {
  if (param_1 == NULL) {
    return NULL;
  }

  size_t current_buffer_size = 0x80; // Initial buffer size
  char *result_str = (char *)calloc(1, current_buffer_size);
  if (result_str == NULL) {
    _error(1, "wl.c", 0x2e); // Original error line and file
  }

  // Iterate through the word list.
  // Each node is assumed to be an array of uint32_t: [next_node_ptr, type, data_ptr]
  for (uint32_t *current_node = param_1; current_node != NULL && current_node[2] != 0;
       current_node = (uint32_t *)*current_node) {

    char *word = (char *)current_node[2];
    size_t word_len = strlen(word);
    size_t sep_len = strlen(word_sep);
    size_t current_str_len = strlen(result_str);

    // Check if current buffer is large enough for word + separator + null terminator
    if (current_buffer_size < current_str_len + word_len + sep_len + 1) {
      size_t required_size = current_str_len + word_len + sep_len + 1;
      current_buffer_size = required_size * 2; // Double the required size
      result_str = (char *)realloc(result_str, current_buffer_size);
      if (result_str == NULL) {
        _error(1, "wl.c", 0x38); // Original error line and file
      }
      // memset the newly allocated part to zero, starting from where the old string ended.
      memset(result_str + current_str_len, 0, current_buffer_size - current_str_len);
    }
    strcat(result_str, word);
    strcat(result_str, word_sep);
  }
  return result_str;
}

// Function: hash_str
uint64_t hash_str(char *param_1) {
  uint32_t hash_low = 0xfacedead;
  uint32_t hash_high = 0xdeadbeef; // -0x21125313 as uint32_t

  while (*param_1 != '\0') {
    uint32_t char_val = (uint32_t)*param_1; // Fix: use current char, not param_1[1]

    uint32_t carry_term1 = hash_low >> 0x1b;
    bool carry_flag1 = CARRY4(hash_low * 0x20, hash_low); // Carry for hash_high update
    uint32_t term_low_multiplier = hash_low * 0x21; // Part for hash_low update

    // Update hash_low
    bool carry_flag2 = CARRY4(char_val, term_low_multiplier); // Carry for hash_low update
    hash_low = char_val + term_low_multiplier;

    // Update hash_high
    // Original: ((int)uVar3 >> 0x1f) + (local_10 << 5 | uVar1) + local_10 + (uint)bVar4 + (uint)CARRY4(uVar3,uVar2);
    // (int)char_val >> 0x1f is 0 for typical char values (0-255).
    // (hash_high << 5 | carry_term1) + hash_high = (hash_high * 32 + carry_term1) + hash_high = hash_high * 33 + carry_term1
    hash_high = (hash_high << 5 | carry_term1) + hash_high + (uint32_t)carry_flag1 + (uint32_t)carry_flag2;

    param_1++;
  }
  return CONCAT44(hash_high, hash_low);
}

// Function: split_words
wordlist_t split_words(char *param_1, int add_chain_term) {
  wordlist_t word_list_head = 0;
  
  if (param_1 == NULL) {
    return 0;
  }

  size_t param_1_len = strlen(param_1);
  char *temp_buffer = (char *)calloc(1, param_1_len + 1);
  if (temp_buffer == NULL) {
    _error(1, "wl.c", 0x59); // Original error line and file
  }
  strncpy(temp_buffer, param_1, param_1_len);
  temp_buffer[param_1_len] = '\0'; // Ensure null termination

  char *current_pos = temp_buffer;
  char *end_of_buffer = temp_buffer + param_1_len;

  while (current_pos < end_of_buffer) {
    // Skip leading whitespace
    while (current_pos < end_of_buffer && isspace((int)*current_pos)) {
      current_pos++;
    }
    if (current_pos == end_of_buffer) break; // Reached end after skipping spaces

    char *word_start = current_pos;

    // Find end of word (non-printable or space)
    char *word_end = current_pos;
    while (word_end < end_of_buffer && isprint((int)*word_end) && !isspace((int)*word_end)) {
      word_end++;
    }
    // Null-terminate the word in the temporary buffer
    *word_end = '\0';

    // Duplicate the word and append to list
    char *new_word_copy = (char *)calloc(1, strlen(word_start) + 1);
    strcpy(new_word_copy, word_start);
    append_list(&word_list_head, new_word_copy, 1);

    // Move to the next potential word
    current_pos = word_end + 1;
  }

  if (add_chain_term != 0) {
    char *chain_term_copy = (char *)calloc(1, strlen(chain_term) + 1);
    strcpy(chain_term_copy, chain_term);
    append_list(&word_list_head, chain_term_copy, 1);
  }

  free(temp_buffer);
  return word_list_head;
}

// Function: chunk_words
wordlist_t chunk_words(wordlist_t param_1, uint32_t chunk_size) {
  wordlist_t result_list = 0;
  uint32_t total_words = len_list(param_1);

  if (chunk_size < total_words) {
    uint32_t num_chunks_to_create_max_start_idx = total_words - chunk_size;

    // Loop for chunks. If total_words=5, chunk_size=2, num_chunks_to_create_max_start_idx=3.
    // i=0: (0,1), i=1: (1,2), i=2: (2,3), i=3: (3,4)
    // The number of chunks is total_words - chunk_size + 1.
    // So loop for i from 0 to (total_words - chunk_size).
    for (uint32_t i = 0; i <= num_chunks_to_create_max_start_idx; i++) {
      wordlist_t copied_list = copy_list(param_1, i, i + chunk_size);
      if (copied_list == 0) {
        _error(1, "wl.c", 0x87); // Original error line and file
      }
      append_list(&result_list, (char*)copied_list, 1); // Cast uint32_t to char* as per original
    }
  }
  return result_list;
}

// Function: upto_last
wordlist_t upto_last(wordlist_t param_1) {
  if (param_1 == 0) {
    return 0;
  }
  uint32_t list_len = len_list(param_1);
  if (list_len < 2) { // If list has 0 or 1 element, cannot get 'upto_last'
    return 0;
  }
  return copy_list(param_1, 0, list_len - 1);
}

// Function: key_from_wordlist
uint64_t key_from_wordlist(wordlist_t param_1) {
  char *str_representation = word_list_to_str(param_1);
  if (str_representation == NULL) {
    return 0;
  }
  uint64_t key = hash_str(str_representation);
  free(str_representation);
  return key;
}

// Function: insert_wordlist
uint32_t insert_wordlist(wordlist_t param_1, tree_t *param_2) {
  if (param_1 == 0 || param_2 == NULL) {
    return 0xffffffff; // Error
  }

  wordlist_t list_upto_last = upto_last(param_1);
  if (list_upto_last == 0) {
    return 0; // Empty or single-word list, no key to form
  }

  uint64_t key = key_from_wordlist(list_upto_last);
  // Assuming list_upto_last is handled by list management; no explicit free_list function provided.

  if (key == 0) {
    return 0xffffffff; // Error generating key
  }

  wordlist_t last_word_list_element = lindex(param_1, -1); // -1 for last element
  if (last_word_list_element == 0) {
    _error(3, "wl.c", 0xb0); // Original error line and file
  }

  if (*param_2 == 0) { // If tree is empty
    *param_2 = init_tree(key, last_word_list_element);
  } else {
    int insert_status = ins_tree(*param_2, key, last_word_list_element);
    if (insert_status < 0) {
      _error(2, "wl.c", 0xb6); // Original error line and file
    }
  }
  return 0; // Success
}

// Function: insert_wordlists
uint32_t insert_wordlists(uint32_t *param_1, tree_t *param_2) {
  if (param_1 == NULL) {
    return 0xffffffff; // Error
  }
  // Iterate through a list of wordlists. Each element is `[next_ptr, type, wordlist_data]`
  for (uint32_t *current_wordlist_node = param_1; current_wordlist_node != NULL;
       current_wordlist_node = (uint32_t *)*current_wordlist_node) {
    
    // current_wordlist_node[2] holds the actual wordlist_t (uint32_t)
    int status = insert_wordlist(current_wordlist_node[2], param_2);
    if (status < 0) {
      return 0xffffffff; // Error
    }
  }
  return 0; // Success
}

// Function: str_to_wordlists
wordlist_t str_to_wordlists(char *param_1) {
  if (param_1 == NULL) {
      return 0;
  }

  size_t param_1_len = strlen(param_1);
  char *temp_str_buffer = (char *)calloc(1, param_1_len + 1);
  if (temp_str_buffer == NULL) {
      _error(1, "wl.c", __LINE__); // Placeholder error line
  }
  strncpy(temp_str_buffer, param_1, param_1_len);
  temp_str_buffer[param_1_len] = '\0'; // Ensure null termination

  // Fix: split_words signature mismatch. Assuming split_words(char*, int)
  wordlist_t words_list = split_words(temp_str_buffer, 1);
  if (words_list == 0) {
    free(temp_str_buffer);
    return 0;
  }

  free(temp_str_buffer); // Free the temporary buffer after splitting

  wordlist_t chunked_wordlists = chunk_words(words_list, 2);
  // Assuming words_list needs to be freed if it's dynamically allocated by split_words.
  // No `free_list` function is provided, so we omit it, assuming management by external code.
  if (chunked_wordlists == 0) {
    return 0;
  }
  return chunked_wordlists;
}