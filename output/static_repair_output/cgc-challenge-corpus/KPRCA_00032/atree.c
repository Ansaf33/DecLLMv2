#include <stdlib.h> // For calloc, realloc, free
#include <string.h> // For strcmp, memmove, memcpy, memset
#include <stdint.h> // For uint32_t, uint16_t
#include <stdbool.h> // For bool type
#include <time.h>   // For srand, rand

// --- Inferred Structures ---

// A node in the Atree. Can be an internal node or a leaf node.
// Internal nodes store frequencies and pointers to child nodes.
// Leaf nodes (id == 0x1a) store words (char*).
typedef struct AtreeNode AtreeNode;

// Entry for internal nodes (frequency and child node pointer)
typedef struct FreqEntry {
    uint32_t freq;
    AtreeNode* child;
} FreqEntry;

struct AtreeNode {
    uint32_t id;         // Character index (0-25) or 0x1a for leaf nodes
    uint16_t count;      // Number of entries/words currently used
    uint16_t capacity;   // Allocated capacity for entries/words
    AtreeNode* prev;     // Pointer to previous node in the global linked list of all nodes
    AtreeNode* next;     // Pointer to next node in the global linked list of all nodes
    // Variable size array starts here:
    // FreqEntry entries[]; // For internal nodes (id < 0x1a)
    // char* words[];       // For leaf nodes (id == 0x1a)
};

// Main Atree structure
typedef struct Atree {
    uint32_t total_words; // Total number of words in the tree
    AtreeNode* root_node; // Root node of the tree (for character 'A')
    AtreeNode* last_node; // Last node added to the global linked list
    char chars_freq_map[26]; // Sorted character map based on initial frequencies
} Atree;

// --- Helper Functions (minimal implementations as they were used but not provided) ---

// ftab_init: Populates a frequency table for characters in a given word.
// param_1: uint32_t array of size 26 to be populated.
// param_2: The word for which to calculate character frequencies.
void ftab_init(uint32_t *freq_table, const char *word) {
    memset(freq_table, 0, sizeof(uint32_t) * 26);
    for (const char *p = word; *p != '\0'; ++p) {
        if (*p >= 'A' && *p <= 'Z') {
            freq_table[*p - 'A']++;
        }
    }
}

// randint: Generates a random integer within a range [min, max_exclusive - 1].
static bool rand_seeded = false;
uint32_t randint(uint32_t min, uint32_t max_exclusive) {
    if (!rand_seeded) {
        srand(time(NULL));
        rand_seeded = true;
    }
    if (max_exclusive <= min) return min;
    return min + (rand() % (max_exclusive - min));
}

// --- Original Functions Refactored ---

// Function: atree_init
void *atree_init(int param_1) {
    Atree *tree = calloc(1, sizeof(Atree));
    if (!tree) {
        return NULL;
    }

    for (int i = 0; i < 26; ++i) {
        tree->chars_freq_map[i] = (char)('A' + i);
    }

    // Insertion sort based on frequencies from param_1
    // param_1 is assumed to be a pointer to an array of uint32_t frequencies for 'A' through 'Z'
    for (int i = 1; i < 26; ++i) {
        char current_char = tree->chars_freq_map[i];
        uint32_t current_freq = ((uint32_t*)param_1)[current_char - 'A'];
        int j = i;
        while (j > 0 && ((uint32_t*)param_1)[tree->chars_freq_map[j - 1] - 'A'] < current_freq) {
            tree->chars_freq_map[j] = tree->chars_freq_map[j - 1];
            j--;
        }
        tree->chars_freq_map[j] = current_char;
    }
    return tree;
}

// Function: _atree_find_freq (Binary search for frequency in a FreqEntry array)
uint32_t _atree_find_freq(AtreeNode *node, uint32_t freq_to_find) {
    if (node->count == 0) {
        return 0xFFFFFFFF; // Not found
    }

    uint32_t low_idx = 0;
    uint32_t high_idx = node->count - 1;
    FreqEntry *entries = (FreqEntry *)(node + 1);

    uint32_t result_idx = 0;

    while (low_idx + 1 < high_idx) {
        result_idx = low_idx + (high_idx - low_idx) / 2;
        if (freq_to_find == entries[result_idx].freq) {
            return result_idx;
        }
        if (freq_to_find < entries[result_idx].freq) {
            high_idx = result_idx;
        } else {
            low_idx = result_idx;
        }
    }
    if (entries[high_idx].freq <= freq_to_find) {
        result_idx = high_idx;
    } else {
        result_idx = low_idx;
    }
    return result_idx;
}

// Function: _atree_find_word (Binary search for a word in a char* array)
uint32_t _atree_find_word(AtreeNode *node, const char *word_to_find) {
    if (node->count == 0) {
        return 0xFFFFFFFF; // Not found
    }

    uint32_t low_idx = 0;
    uint32_t high_idx = node->count - 1;
    char **words = (char **)(node + 1);

    uint32_t result_idx = 0;

    while (low_idx + 1 < high_idx) {
        result_idx = low_idx + (high_idx - low_idx) / 2;
        int cmp = strcmp(words[result_idx], word_to_find);
        if (cmp == 0) {
            return result_idx;
        }
        if (cmp > 0) { // word_to_find is smaller
            high_idx = result_idx;
        } else { // word_to_find is larger
            low_idx = result_idx;
        }
    }
    int cmp = strcmp(words[high_idx], word_to_find);
    if (cmp < 1) { // cmp < 0 (word_to_find is larger) or cmp == 0 (match)
        result_idx = high_idx;
    } else {
        result_idx = low_idx;
    }
    return result_idx;
}

// Function: _atree_walk (Navigates the tree based on character frequencies)
AtreeNode** _atree_walk(Atree *tree, uint32_t *char_freqs, bool create_if_missing) {
    AtreeNode** current_node_ptr = &tree->root_node;
    uint32_t char_idx = 0;

    while (true) {
        if (char_idx >= 26) { // Leaf level reached
            if (!*current_node_ptr) {
                if (!create_if_missing) {
                    return NULL;
                }
                AtreeNode *new_node = calloc(1, sizeof(AtreeNode) + 2 * sizeof(char*));
                if (!new_node) {
                    return NULL;
                }
                new_node->id = 26; // Mark as leaf node
                new_node->capacity = 2;
                
                new_node->prev = tree->last_node;
                if (tree->last_node) {
                    tree->last_node->next = new_node;
                }
                tree->last_node = new_node;
                *current_node_ptr = new_node;
            }
            return current_node_ptr;
        }

        uint32_t current_char_freq = char_freqs[tree->chars_freq_map[char_idx] - 'A'];
        AtreeNode *node = *current_node_ptr;

        if (!node || char_idx < node->id) { // Node does not exist for this char_idx, or existing node has higher ID
            if (current_char_freq == 0) {
                return NULL;
            }
            if (!create_if_missing) {
                return NULL;
            }

            AtreeNode *new_node = calloc(1, sizeof(AtreeNode) + 2 * sizeof(FreqEntry));
            if (!new_node) {
                return NULL;
            }
            new_node->id = char_idx;
            new_node->capacity = 2;

            if (node) { // If there was a node but it had a larger ID, it becomes a child of the new node
                FreqEntry *entries = (FreqEntry *)(new_node + 1);
                entries[0].child = node;
                new_node->count = 1;
            }
            *current_node_ptr = new_node;
            node = new_node;
        }
        
        FreqEntry *entries = (FreqEntry *)(node + 1);
        uint32_t freq_idx = _atree_find_freq(node, current_char_freq);
        bool freq_found_exact = (freq_idx < node->count && entries[freq_idx].freq == current_char_freq);

        if (!freq_found_exact) {
            if (!create_if_missing) {
                return NULL;
            }

            node->count++;
            if (node->count > node->capacity) {
                uint16_t new_capacity = node->capacity + 2;
                size_t new_size = sizeof(AtreeNode) + new_capacity * sizeof(FreqEntry);
                AtreeNode *reallocated_node = realloc(node, new_size);
                if (!reallocated_node) {
                    node->count--;
                    return NULL;
                }
                *current_node_ptr = reallocated_node;
                node = reallocated_node;
                node->capacity = new_capacity;
                entries = (FreqEntry *)(node + 1);
            }

            if (freq_idx < node->count - 1 && entries[freq_idx].freq < current_char_freq) {
                freq_idx++;
            }

            if (freq_idx < node->count - 1) {
                memmove(&entries[freq_idx + 1], &entries[freq_idx], (node->count - 1 - freq_idx) * sizeof(FreqEntry));
            }
            entries[freq_idx].freq = current_char_freq;
            entries[freq_idx].child = NULL;
        }
        current_node_ptr = &entries[freq_idx].child;
        char_idx++;
    }
}

// Function: atree_add
uint32_t atree_add(Atree *tree, char *word_to_add) {
    uint32_t char_freqs[26];
    ftab_init(char_freqs, word_to_add);

    AtreeNode** leaf_node_ptr_loc = _atree_walk(tree, char_freqs, true);
    if (!leaf_node_ptr_loc) {
        return 0;
    }

    AtreeNode *leaf_node = *leaf_node_ptr_loc;
    char **words = (char **)(leaf_node + 1);
    uint32_t word_idx = _atree_find_word(leaf_node, word_to_add);
    bool word_exists_exact = (word_idx < leaf_node->count && strcmp(words[word_idx], word_to_add) == 0);

    if (word_exists_exact) {
        return 0;
    }

    if (word_idx < leaf_node->count && strcmp(words[word_idx], word_to_add) < 0) {
        word_idx++;
    }

    leaf_node->count++;
    if (leaf_node->count > leaf_node->capacity) {
        uint16_t new_capacity = leaf_node->capacity + 8;
        size_t new_size = sizeof(AtreeNode) + new_capacity * sizeof(char*);
        AtreeNode *reallocated_node = realloc(leaf_node, new_size);
        if (!reallocated_node) {
            leaf_node->count--;
            return 0;
        }

        if (reallocated_node->prev) {
            reallocated_node->prev->next = reallocated_node;
        } else {
            tree->root_node = reallocated_node; // If root node was reallocated
        }
        if (reallocated_node->next) {
            reallocated_node->next->prev = reallocated_node;
        } else {
            tree->last_node = reallocated_node; // If last node was reallocated
        }

        *leaf_node_ptr_loc = reallocated_node;
        leaf_node = reallocated_node;
        leaf_node->capacity = new_capacity;
        words = (char **)(leaf_node + 1);
    }

    if (word_idx < leaf_node->count - 1) {
        memmove(&words[word_idx + 1], &words[word_idx], (leaf_node->count - 1 - word_idx) * sizeof(char*));
    }
    words[word_idx] = word_to_add;
    tree->total_words++;
    return 1;
}

// Function: atree_remove
uint32_t atree_remove(Atree *tree, const char *word_to_remove) {
    uint32_t char_freqs[26];
    ftab_init(char_freqs, word_to_remove);

    AtreeNode** leaf_node_ptr_loc = _atree_walk(tree, char_freqs, false);
    if (!leaf_node_ptr_loc) {
        return 0;
    }

    AtreeNode *leaf_node = *leaf_node_ptr_loc;
    char **words = (char **)(leaf_node + 1);
    uint32_t word_idx = 0;

    while (word_idx < leaf_node->count && strcmp(word_to_remove, words[word_idx]) != 0) {
        word_idx++;
    }

    if (word_idx < leaf_node->count) {
        if (word_idx < leaf_node->count - 1) {
            memmove(&words[word_idx], &words[word_idx + 1], (leaf_node->count - 1 - word_idx) * sizeof(char*));
        }
        leaf_node->count--;
        tree->total_words--;
        return 1;
    }
    return 0;
}

// Function: atree_query
void *atree_query(Atree *tree, const char *word_prefix) {
    uint32_t char_freqs[26];
    ftab_init(char_freqs, word_prefix);

    AtreeNode** leaf_node_ptr_loc = _atree_walk(tree, char_freqs, false);

    if (!leaf_node_ptr_loc) {
        char** result = calloc(1, sizeof(char*));
        if (!result) return NULL;
        result[0] = NULL;
        return result;
    }

    AtreeNode *leaf_node = *leaf_node_ptr_loc;
    char **words_in_node = (char **)(leaf_node + 1);

    char** result = calloc(leaf_node->count + 1, sizeof(char*));
    if (!result) {
        return NULL;
    }

    memcpy(result, words_in_node, leaf_node->count * sizeof(char*));
    result[leaf_node->count] = NULL;
    return result;
}

// Function: _atree_gather_subset (Recursive helper for atree_query_subset)
void _atree_gather_subset(Atree *tree, uint32_t *char_freqs, AtreeNode *node, char ***collected_words_ptr, int *collected_count_ptr) {
    if (!node) {
        return;
    }

    if (node->id == 26) { // Leaf node
        char **words_in_leaf = (char **)(node + 1);
        size_t new_size = (*collected_count_ptr + node->count) * sizeof(char*);
        char **reallocated_words = realloc(*collected_words_ptr, new_size);
        if (!reallocated_words) {
            return;
        }
        *collected_words_ptr = reallocated_words;
        memcpy(&reallocated_words[*collected_count_ptr], words_in_leaf, node->count * sizeof(char*));
        *collected_count_ptr += node->count;
    } else { // Internal node
        uint32_t current_char_freq = char_freqs[tree->chars_freq_map[node->id] - 'A'];
        FreqEntry *entries = (FreqEntry *)(node + 1);

        if (node->count == 0) return;

        uint32_t current_freq_idx_limit = _atree_find_freq(node, current_char_freq);

        if (current_freq_idx_limit < node->count && current_char_freq < entries[current_freq_idx_limit].freq) {
            if (current_freq_idx_limit == 0) {
                return;
            }
            current_freq_idx_limit--;
        }

        for (uint32_t i = 0; i <= current_freq_idx_limit; ++i) {
            _atree_gather_subset(tree, char_freqs, entries[i].child, collected_words_ptr, collected_count_ptr);
        }
    }
}

// Function: atree_query_subset
void *atree_query_subset(Atree *tree, const char *word_prefix) {
    char **collected_words = malloc(sizeof(char*));
    if (!collected_words) return NULL;
    collected_words[0] = NULL;
    int collected_count = 0;

    uint32_t char_freqs[26];
    ftab_init(char_freqs, word_prefix);

    _atree_gather_subset(tree, char_freqs, tree->root_node, &collected_words, &collected_count);

    char **final_words = realloc(collected_words, (collected_count + 1) * sizeof(char*));
    if (!final_words) {
        free(collected_words);
        return NULL;
    }
    final_words[collected_count] = NULL;
    return final_words;
}

// Function: atree_random
uint32_t atree_random(Atree *tree) {
    if (tree->total_words == 0) {
        return 0;
    }

    uint32_t random_idx = randint(0, tree->total_words);
    AtreeNode *current_node = tree->last_node;

    while (current_node) {
        if (current_node->count > random_idx) {
            char **words = (char **)(current_node + 1);
            return (uint32_t)words[random_idx];
        }
        random_idx -= current_node->count;
        current_node = current_node->prev;
    }

    return 0;
}