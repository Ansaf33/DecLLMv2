#include <stdio.h>   // For fprintf, stderr
#include <stdlib.h>  // For calloc, realloc, free, exit
#include <string.h>  // For strlen, strcat, memset, strncpy, strcpy
#include <ctype.h>   // For isspace, isprint
#include <stdint.h>  // For uint32_t, int32_t, UINT32_MAX

// --- Placeholder for custom error function ---
void _error(int err_code, const char *file, int line) {
    fprintf(stderr, "Error %d at %s:%d\n", err_code, file, line);
    exit(err_code);
}

// --- Global variables (placeholders) ---
// These would typically be defined in another file or globally in the main program.
char *word_sep = " ";   // Example separator
char *chain_term = "TERM"; // Example chain terminator

// --- External List/Tree Function Declarations (placeholders) ---
// These functions are not provided in the snippet, but are called.
// Their signatures are inferred from their usage in the provided code.

// Node structure for word lists (e.g., in word_list_to_str, key_from_wordlist)
typedef struct GenericListNode {
    struct GenericListNode *next;       // Pointer to the next node
    void *padding_or_other_field;       // Placeholder for a second field, if any (e.g., a counter)
    char *word_data;                    // The actual string data
} GenericListNode;

// Node structure for lists of lists (e.g., in insert_wordlists)
typedef struct ListOfListsNode {
    struct ListOfListsNode *next;       // Pointer to the next node
    void *padding_or_other_field;       // Placeholder for a second field, if any
    void *sub_list;                     // The actual list to be processed (e.g., a GenericListNode*)
} ListOfListsNode;

extern void append_list(void **list_head, void *data, int type); // Appends data to a list
extern unsigned int len_list(void *list_head);                     // Returns the length of a list
extern void *copy_list(void *list_head, unsigned int start_idx, unsigned int end_idx_exclusive); // Copies a sub-list
extern void *lindex(void *list_head, int index);                  // Returns data at a specific index (e.g., -1 for last)

extern int init_tree(unsigned long long key, void *value);        // Initializes a tree with a key-value pair
extern int ins_tree(int tree_root, unsigned long long key, void *value); // Inserts into an existing tree

// CONCAT44: Concatenates two 32-bit unsigned integers into a 64-bit unsigned integer.
#define CONCAT44(high, low) (((unsigned long long)(high) << 32) | (low))

// CARRY4: Checks for carry in a 32-bit unsigned integer addition.
// Returns 1 if (a + b) would overflow a uint32_t, 0 otherwise.
#define CARRY4(a, b) ((unsigned int)(a) > (UINT32_MAX - (unsigned int)(b)))

// Function: word_list_to_str
char * word_list_to_str(GenericListNode *param_1) {
    if (param_1 == NULL) {
        return NULL;
    }

    char *result_str = (char *)calloc(1, 0x80);
    size_t current_buffer_size = 0x80;

    if (result_str == NULL) {
        _error(1, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0x2e);
    }

    for (GenericListNode *current_node = param_1;
         current_node != NULL && current_node->word_data != NULL;
         current_node = current_node->next) {

        size_t current_str_len = strlen(result_str);
        size_t word_len = strlen(current_node->word_data);
        size_t sep_len = strlen(word_sep);

        // Check if buffer is large enough for word + separator + null terminator
        if (current_buffer_size < current_str_len + word_len + sep_len + 1) {
            current_buffer_size = (current_str_len + word_len + sep_len + 1) * 2;
            char *new_result_str = (char *)realloc(result_str, current_buffer_size);
            if (new_result_str == NULL) {
                _error(1, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0x38);
            }
            result_str = new_result_str;
            memset(result_str + current_str_len, 0, current_buffer_size - current_str_len);
        }
        strcat(result_str, current_node->word_data);
        strcat(result_str, word_sep);
    }
    return result_str;
}

// Function: hash_str
unsigned long long hash_str(char *param_1) {
    uint32_t hash_val = 0xfacedead;
    int32_t accumulator = -0x21125313; // -554910483

    while (*param_1 != '\0') {
        uint32_t next_char_val = (uint32_t)param_1[1];

        uint32_t carry_bVar4 = CARRY4(hash_val * 0x20U, hash_val);
        uint32_t temp_uVar2 = hash_val * 0x21U;
        
        uint32_t carry_uVar3_uVar2 = CARRY4(next_char_val, temp_uVar2);
        hash_val = next_char_val + temp_uVar2;

        uint32_t uVar1_shifted = hash_val >> 0x1b;

        accumulator = ((int32_t)next_char_val >> 0x1f) +
                      ((accumulator << 5) | uVar1_shifted) +
                      accumulator +
                      carry_bVar4 +
                      carry_uVar3_uVar2;
        
        param_1++;
    }
    return CONCAT44(accumulator, hash_val);
}

// Function: split_words
void * split_words(char *param_1, int add_chain_term) {
    void *word_list_head = NULL;

    if (param_1 == NULL) {
        return NULL;
    }

    size_t input_len = strlen(param_1);
    char *input_copy = (char *)calloc(1, input_len + 1);
    if (input_copy == NULL) {
        _error(1, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0x59);
    }
    strncpy(input_copy, param_1, input_len);
    input_copy[input_len] = '\0';

    char *current_pos = input_copy;
    char *end_of_input = input_copy + input_len;

    while (current_pos < end_of_input) {
        // Skip leading whitespace
        while (current_pos < end_of_input && isspace((unsigned char)*current_pos)) {
            current_pos++;
        }
        if (current_pos == end_of_input) {
            break;
        }

        char *word_start = current_pos;
        char *word_end = current_pos;

        // Find end of word (non-printable or space)
        while (word_end < end_of_input && isprint((unsigned char)*word_end) && !isspace((unsigned char)*word_end)) {
            word_end++;
        }

        // Null-terminate the word in the copy
        if (word_end < end_of_input) {
            *word_end = '\0';
        }

        size_t word_len = strlen(word_start);
        if (word_len > 0) {
            char *copied_word = (char *)calloc(1, word_len + 1);
            if (copied_word == NULL) {
                _error(1, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0x6e);
            }
            strcpy(copied_word, word_start);
            append_list(&word_list_head, copied_word, 1);
        }

        current_pos = word_end + 1;
    }

    if (add_chain_term != 0) {
        size_t term_len = strlen(chain_term);
        char *term_copy = (char *)calloc(1, term_len + 1);
        if (term_copy == NULL) {
            _error(1, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0x76);
        }
        strcpy(term_copy, chain_term);
        append_list(&word_list_head, term_copy, 1);
    }

    free(input_copy);
    return word_list_head;
}

// Function: chunk_words
void * chunk_words(void *param_1_list, unsigned int ngram_size_minus_1) {
    void *result_list = NULL;

    unsigned int total_words = len_list(param_1_list);

    // If ngram_size_minus_1 is UINT32_MAX, actual_ngram_size would be 0, which is invalid.
    if (ngram_size_minus_1 == UINT32_MAX) {
        return NULL;
    }
    unsigned int actual_ngram_size = ngram_size_minus_1 + 1;

    if (actual_ngram_size == 0 || param_1_list == NULL) {
        return NULL;
    }

    if (actual_ngram_size <= total_words) {
        unsigned int num_iterations = total_words - ngram_size_minus_1; // Loop limit

        for (unsigned int i = 0; i < num_iterations; i++) {
            void *copied_chunk = copy_list(param_1_list, i, i + actual_ngram_size);
            if (copied_chunk == NULL) {
                _error(1, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0x87);
            }
            append_list(&result_list, copied_chunk, 1);
        }
    }
    return result_list;
}

// Function: upto_last
void * upto_last(void *param_1_list) {
    if (param_1_list == NULL) {
        return NULL;
    }

    unsigned int list_len = len_list(param_1_list);
    if (list_len < 2) { // Need at least 2 elements to remove the last one
        return NULL;
    }

    return copy_list(param_1_list, 0, list_len - 1);
}

// Function: key_from_wordlist
unsigned long long key_from_wordlist(GenericListNode *param_1_wordlist) {
    char *wordlist_str = word_list_to_str(param_1_wordlist);
    if (wordlist_str == NULL) {
        return 0;
    }
    unsigned long long hash = hash_str(wordlist_str);
    free(wordlist_str);
    return hash;
}

// Function: insert_wordlist
int insert_wordlist(void *param_1_list, int *param_2_tree_root) {
    if (param_1_list == NULL || param_2_tree_root == NULL) {
        return -1; // Indicate error
    }

    void *upto_last_list = upto_last(param_1_list);
    if (upto_last_list == NULL) {
        return 0; // No key to generate if list has < 2 elements (upto_last returns NULL)
    }

    unsigned long long key = key_from_wordlist((GenericListNode*)upto_last_list);
    // The original code doesn't free `upto_last_list`. Assuming it's a temporary structure
    // that doesn't require explicit freeing here, or it's implicitly handled by the list API.

    if (key == 0) { // Error in key generation
        return -1;
    }

    void *last_element_data = lindex(param_1_list, -1); // -1 for last element
    if (last_element_data == NULL) { // Should not happen if upto_last_list was not NULL
        _error(3, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0xb0);
    }

    if (*param_2_tree_root == 0) { // Tree is empty, initialize it
        int new_tree_root = init_tree(key, last_element_data);
        if (new_tree_root == 0) { // Assuming 0 is an error or invalid root
             return -1;
        }
        *param_2_tree_root = new_tree_root;
    } else { // Tree exists, insert into it
        int insert_result = ins_tree(*param_2_tree_root, key, last_element_data);
        if (insert_result < 0) {
            _error(2, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0xb6);
        }
    }
    return 0; // Success
}

// Function: insert_wordlists
int insert_wordlists(ListOfListsNode *param_1_list_of_lists, int *param_2_tree_root) {
    if (param_1_list_of_lists == NULL) {
        return -1; // Indicate error
    }

    for (ListOfListsNode *current_node = param_1_list_of_lists;
         current_node != NULL;
         current_node = current_node->next) {
        
        int result = insert_wordlist(current_node->sub_list, param_2_tree_root);
        if (result < 0) {
            return -1; // Propagate error
        }
    }
    return 0; // Success
}

// Function: str_to_wordlists
void *str_to_wordlists(char *param_1_str) {
    if (param_1_str == NULL) {
        return NULL;
    }

    size_t input_len = strlen(param_1_str);
    char *input_copy = (char *)calloc(1, input_len + 1);
    if (input_copy == NULL) {
        _error(1, "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/wl.c", 0x135);
    }
    strncpy(input_copy, param_1_str, input_len);
    input_copy[input_len] = '\0';

    void *word_list = split_words(input_copy, 1);
    
    free(input_copy); // input_copy can be freed after split_words has made copies of its words

    if (word_list == NULL) {
        return NULL;
    }

    // `chunk_words(iVar1, 2)` implies ngram_size_minus_1 = 2 (for 3-grams)
    void *chunked_lists = chunk_words(word_list, 2);
    // The original code does not free `word_list` after `chunk_words`.
    // This implies either `chunk_words` takes ownership, or it's a memory leak.
    // Adhering to the original code's memory management pattern.

    if (chunked_lists == NULL) {
        return NULL;
    }
    return chunked_lists;
}