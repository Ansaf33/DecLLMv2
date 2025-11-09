#include <stdlib.h> // For calloc, free, realloc, exit
#include <string.h> // For memcpy
#include <stdio.h>  // For fprintf, stderr

// Define ALPHABET_SIZE for the children array
#define ALPHABET_SIZE 256

// Custom calloc wrapper to handle allocation failures
static void* xcalloc(size_t num, size_t size) {
    void* ptr = calloc(num, size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Custom realloc wrapper to handle re-allocation failures
static void* xrealloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL && size > 0) { // realloc can return NULL if size is 0 or if it fails.
                                       // If it fails, original ptr is still valid.
        fprintf(stderr, "Memory re-allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

// Global variable for trie node count
static int TrieCount = 0;

// TrieNode structure derived from memory access patterns in the original code.
// The original code implies a 32-bit environment where sizeof(int) == sizeof(void*).
// This refactored structure uses standard C types and direct member access,
// allowing the compiler to lay out the struct optimally for the target architecture
// (e.g., 32-bit or 64-bit). The exact byte offsets (e.g., 0x404, 0x405, 0x408)
// and total size (0x40c) from the decompiled code are replaced by portable member access.
typedef struct TrieNode {
    struct TrieNode* parent;           // Pointer to parent node
    struct TrieNode* children[ALPHABET_SIZE]; // Array of pointers to child nodes
    unsigned char node_char;           // The character this node represents (0 for root)
    unsigned char is_terminal;         // 1 if this node marks the end of a word, 0 otherwise
    unsigned char padding[2];          // Padding for alignment (optional, compiler handles alignment)
    int identifier;                    // Unique identifier for this node
} TrieNode;

// Function: AllocateAndInitializeTrieRoot
void AllocateAndInitializeTrieRoot(TrieNode **root_ptr) {
    if (root_ptr == NULL) {
        return;
    }
    
    TrieNode *newNode = (TrieNode *)xcalloc(1, sizeof(TrieNode));
    // xcalloc already zeroes out memory, so parent, children, node_char, is_terminal are 0/NULL.
    newNode->identifier = TrieCount++;
    
    *root_ptr = newNode;
}

// Function: InitializeTrieChild
TrieNode *InitializeTrieChild(unsigned char node_char_val) {
    TrieNode *newNode = (TrieNode *)xcalloc(1, sizeof(TrieNode));
    // xcalloc already zeroes out memory.
    newNode->node_char = node_char_val;
    newNode->identifier = TrieCount++;
    return newNode;
}

// Function: FreeTrie
void FreeTrie(TrieNode *node) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (node->children[i] != NULL) {
            FreeTrie(node->children[i]);
            node->children[i] = NULL; // Clear pointer after freeing for safety
        }
    }
    free(node);
}

// Function: InsertIntoTrie
TrieNode *InsertIntoTrie(TrieNode *current_node, const char *str, unsigned int len) {
    if (current_node == NULL) {
        return NULL;
    }

    for (unsigned int i = 0; i < len; ++i) {
        unsigned char char_idx = (unsigned char)str[i];
        
        if (current_node->children[char_idx] == NULL) {
            current_node->children[char_idx] = InitializeTrieChild(char_idx);
        }
        
        current_node->children[char_idx]->parent = current_node;
        current_node = current_node->children[char_idx];
    }
    current_node->is_terminal = 1;
    return current_node;
}

// Function: FindInTrie
TrieNode *FindInTrie(TrieNode *current_node, const char *str, unsigned int len) {
    if (current_node == NULL) {
        return NULL;
    }

    for (unsigned int i = 0; i < len; ++i) {
        unsigned char char_idx = (unsigned char)str[i];
        if (current_node->children[char_idx] == NULL) {
            return NULL;
        }
        current_node = current_node->children[char_idx];
    }
    
    if (current_node->is_terminal) {
        return current_node;
    }
    return NULL;
}

// Function: FindInTrieByIdentifier
TrieNode *FindInTrieByIdentifier(TrieNode *current_node, int target_id) {
    if (current_node == NULL) {
        return NULL;
    }

    if (current_node->identifier == target_id) {
        return current_node;
    }

    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (current_node->children[i] != NULL) {
            TrieNode *found_node = FindInTrieByIdentifier(current_node->children[i], target_id);
            if (found_node != NULL) {
                return found_node;
            }
        }
    }
    return NULL;
}

// Function: GetTrieCount
int GetTrieCount(void) {
    return TrieCount;
}

// Function: _GatherTerminals (recursive helper)
static void _GatherTerminals(TrieNode *current_node, TrieNode ***terminals_array_ptr, int *current_count_ptr, int *capacity_ptr) {
    if (current_node == NULL) {
        return;
    }

    if (current_node->is_terminal) {
        if (*current_count_ptr == *capacity_ptr) {
            *capacity_ptr *= 2;
            *terminals_array_ptr = (TrieNode **)xrealloc(*terminals_array_ptr, *capacity_ptr * sizeof(TrieNode*));
        }
        (*terminals_array_ptr)[(*current_count_ptr)++] = current_node;
    }

    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (current_node->children[i] != NULL) {
            _GatherTerminals(current_node->children[i], terminals_array_ptr, current_count_ptr, capacity_ptr);
        }
    }
}

// Function: GatherTerminals
TrieNode **GatherTerminals(TrieNode *root, int *count_ptr) {
    if (root == NULL || count_ptr == NULL) {
        if (count_ptr != NULL) *count_ptr = 0;
        return NULL;
    }

    int initial_capacity = 4;
    TrieNode **terminals_array = (TrieNode **)xcalloc(initial_capacity, sizeof(TrieNode*));
    *count_ptr = 0;
    
    _GatherTerminals(root, &terminals_array, count_ptr, &initial_capacity);
    
    if (*count_ptr == 0) {
        free(terminals_array);
        return NULL;
    }
    
    // Shrink the array to the actual count for efficiency
    terminals_array = (TrieNode **)xrealloc(terminals_array, *count_ptr * sizeof(TrieNode*));

    return terminals_array;
}

// Function: ReverseArray
int ReverseArray(char *arr, unsigned int len) {
    if (arr == NULL || len == 0) {
        return -1; // Error code
    }

    for (unsigned int i = 0; i < len / 2; ++i) {
        char temp = arr[i];
        arr[i] = arr[len - 1 - i];
        arr[len - 1 - i] = temp;
    }
    return 0; // Success
}

// Function: GetDataString
char *GetDataString(TrieNode *terminal_node, int *len_ptr) {
    if (terminal_node == NULL) {
        if (len_ptr != NULL) *len_ptr = 0;
        return NULL;
    }

    int current_capacity = 64;
    int current_len = 0;
    char *str_buffer = (char *)xcalloc(1, current_capacity);

    TrieNode *node = terminal_node;
    // Traverse up using parent pointers, stopping before the root node.
    // The root node's node_char (0) should not be part of the extracted string.
    while (node != NULL && node->parent != NULL) {
        if (current_len + 1 >= current_capacity) { // +1 for the null terminator later
            current_capacity *= 2;
            str_buffer = (char *)xrealloc(str_buffer, current_capacity);
        }
        str_buffer[current_len++] = node->node_char;
        node = node->parent;
    }

    // Add null terminator
    if (current_len + 1 >= current_capacity) {
        str_buffer = (char *)xrealloc(str_buffer, current_len + 1);
    }
    str_buffer[current_len] = '\0';

    if (ReverseArray(str_buffer, current_len) != 0) {
        free(str_buffer);
        if (len_ptr != NULL) *len_ptr = 0;
        return NULL;
    }

    if (len_ptr != NULL) *len_ptr = current_len;
    return str_buffer;
}