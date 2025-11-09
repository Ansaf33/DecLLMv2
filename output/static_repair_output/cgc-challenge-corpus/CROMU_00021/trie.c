#include <stdlib.h> // For calloc, NULL
#include <string.h> // For strlen
#include <stddef.h> // For size_t

// Define the TrieNode structure.
// The original code implies a node size of 16 bytes (0x10) and pointer arithmetic
// that suggests 4-byte pointers (e.g., on a 32-bit system).
// For standard Linux compilation (typically 64-bit), pointers are 8 bytes.
// This struct definition is portable; `sizeof(TrieNode)` will correctly determine
// the necessary memory size (e.g., 32 bytes on 64-bit systems due to pointer size and padding).
typedef struct TrieNode {
    struct TrieNode *child;   // Points to the first child node (downwards in the trie)
    struct TrieNode *sibling; // Points to the next sibling node (sideways in the trie)
    char data;                // The character stored in this node
    // Padding will be added by the compiler to align `value` member.
    // On a 32-bit system: 4 (child) + 4 (sibling) + 1 (data) + 3 (padding) + 4 (value) = 16 bytes.
    // On a 64-bit system: 8 (child) + 8 (sibling) + 1 (data) + 7 (padding) + 8 (value) = 32 bytes.
    int *value;               // Pointer to an integer value associated with the end of a word
} TrieNode;

// Helper function to create and initialize a new TrieNode.
static TrieNode *createTrieNode(char data) {
    TrieNode *newNode = (TrieNode *)calloc(1, sizeof(TrieNode));
    if (newNode != NULL) {
        newNode->data = data;
    }
    return newNode;
}

// Function: initTrie
// Initializes and returns a new empty trie (root node).
void * initTrie(void) {
  return calloc(1, sizeof(TrieNode));
}

// Function: insertInTrie
// Inserts a string `str` into the trie, associating it with `value_ptr`.
// `root_ptr` is a pointer to the root of the trie (or a child pointer in a recursive context).
void insertInTrie(TrieNode **root_ptr, char *str, int *value_ptr) {
    size_t str_len = strlen(str);
    size_t str_idx = 0;

    // `current_node_ptr` is a pointer to a `TrieNode*` variable.
    // It is used to correctly update `root_ptr`, `node->child`, or `node->sibling`.
    TrieNode **current_node_ptr = root_ptr;

    // Iterate through the characters of the string, including the null terminator.
    // The null terminator marks the end of the word where the `value_ptr` should be stored.
    while (str_idx <= str_len) {
        char char_to_insert = str[str_idx];
        TrieNode *found_node = NULL;

        // Traverse the current level's sibling list to find `char_to_insert`.
        // `traverse_sibling_ptr` points to the `sibling` pointer that needs to be updated.
        TrieNode **traverse_sibling_ptr = current_node_ptr;
        while (*traverse_sibling_ptr != NULL) {
            if ((*traverse_sibling_ptr)->data == char_to_insert) {
                found_node = *traverse_sibling_ptr;
                break;
            }
            traverse_sibling_ptr = &((*traverse_sibling_ptr)->sibling); // Move to the next sibling's pointer
        }

        if (found_node == NULL) {
            // If the character was not found at this level, create a new node.
            TrieNode *new_node = createTrieNode(char_to_insert);
            if (new_node == NULL) {
                // Handle memory allocation failure.
                return;
            }
            *traverse_sibling_ptr = new_node; // Attach the new node to the sibling list.
            found_node = new_node;
        }

        // If the current character is the null terminator, the word ends here.
        if (char_to_insert == '\0') {
            found_node->value = value_ptr; // Store the associated value.
            return;
        }

        // For the next character, move to the child level of the `found_node`.
        current_node_ptr = &(found_node->child);
        str_idx++;
    }
}

// Function: findInTrie
// Searches for a string `str` in the trie.
// `root_ptr` is a pointer to the root of the trie.
// Returns a pointer to the TrieNode representing the end of the word if found and has a value,
// otherwise returns NULL.
TrieNode * findInTrie(TrieNode **root_ptr, char *str) {
    size_t str_idx = 0;
    // Start the search from the node pointed to by `root_ptr`.
    TrieNode *current_level_node = *root_ptr;

    // Iterate through the characters of the string, including the null terminator.
    while (1) {
        char char_to_match = str[str_idx];
        TrieNode *found_node = NULL;

        // Search for `char_to_match` among the siblings at the current level.
        TrieNode *traverse_node = current_level_node;
        while (traverse_node != NULL) {
            if (traverse_node->data == char_to_match) {
                found_node = traverse_node;
                break;
            }
            traverse_node = traverse_node->sibling;
        }

        if (found_node == NULL) {
            // Character not found at this level, so the string is not in the trie.
            return NULL;
        }

        // If the current character is the null terminator, we've reached the end of the search string.
        if (char_to_match == '\0') {
            // Check if this node actually marks the end of a complete word (has an associated value).
            if (found_node->value != NULL) {
                return found_node; // Return the node if it's a complete word.
            }
            return NULL; // String found as a prefix, but not as a complete word with a value.
        }

        // Not the end of the string, so move to the child level for the next character.
        current_level_node = found_node->child;
        str_idx++;
    }
}