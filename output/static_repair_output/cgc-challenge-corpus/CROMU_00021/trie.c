#include <stdlib.h> // For calloc
#include <string.h> // For strlen

// Function: initTrie
// Allocates memory for a new Trie node.
// The size 0x10 (16 bytes) implies a specific 32-bit memory layout
// (e.g., 4 pointers of 4 bytes each). For portability on 64-bit systems,
// where pointers are 8 bytes, we explicitly allocate for 4 void pointers.
// This ensures the memory layout for child, sibling, char, and value is consistent.
void *initTrie(void) {
  return calloc(1, 4 * sizeof(void*));
}

// Function: insertInTrie
// Inserts a word and its associated value into the Trie.
// param_1: A pointer to the root of the Trie (void**).
// param_2: The word (char*) to insert.
// param_3: A pointer to the integer value (int*) to associate with the word.
void insertInTrie(void **root_ptr, char *word, int *val_ptr) {
    size_t word_len = strlen(word);
    unsigned int char_index = 0; // Current character index in word
    void *current_node; // Pointer to the current Trie node being processed

    // Handle initial Trie creation if the root is NULL.
    // The first node created is considered the root, and its character field
    // is set to the first character of the word being inserted.
    if (*root_ptr == NULL) {
        *root_ptr = calloc(1, 4 * sizeof(void*));
        // The character field is located at an offset of 2 * sizeof(void*) from the node's base address.
        *(char *)((char*)*root_ptr + 2 * sizeof(void*)) = word[0];
    }

    current_node = *root_ptr;

    // Main loop to traverse or create nodes for the word.
    // The loop condition `char_index <= word_len` allows processing up to and including
    // the implicit null terminator position to correctly set the value.
    while (char_index <= word_len) {
        // If we have reached the end of the word (either by index or null terminator character),
        // we break to set the value at the `current_node`.
        if (char_index == word_len || word[char_index] == '\0') {
            break;
        }

        // Search for the character `word[char_index]` within the current sibling list.
        // `current_node` initially points to the head of the current sibling list.
        void *iter_node = current_node;
        // Iterate through siblings until a match is found or the end of the sibling list is reached.
        while (((void**)iter_node)[1] != NULL && *(char *)((char*)iter_node + 2 * sizeof(void*)) != word[char_index]) {
            iter_node = ((void**)iter_node)[1]; // Move to the next sibling (index 1)
        }
        current_node = iter_node; // `current_node` now points to the found sibling or the last sibling.

        // If the character `word[char_index]` was not found among the siblings (including `current_node`).
        if (*(char *)((char*)current_node + 2 * sizeof(void*)) != word[char_index]) {
            // Create a new node and link it as a sibling to `current_node`.
            void *new_node = calloc(1, 4 * sizeof(void*));
            ((void**)current_node)[1] = new_node; // Link new_node as a sibling (index 1).
            *(char *)((char*)new_node + 2 * sizeof(void*)) = word[char_index]; // Set new sibling's character.
            current_node = new_node; // Move to the newly created sibling.
            char_index++; // Increment index to process the next character.

            // Create subsequent child nodes for the rest of the word.
            while (char_index < word_len) {
                new_node = calloc(1, 4 * sizeof(void*));
                ((void**)current_node)[0] = new_node; // Link new_node as a child (index 0).
                *(char *)((char*)new_node + 2 * sizeof(void*)) = word[char_index]; // Set new child's character.
                current_node = new_node; // Move to the newly created child.
                char_index++;
            }
            break; // All remaining characters have been inserted; exit loop to set the value.
        }

        // If the character `word[char_index]` was found in the sibling list.
        // Move to the child path for the next character.
        if (((void**)current_node)[0] == NULL) { // If `current_node` has no child (index 0).
            void *new_node = calloc(1, 4 * sizeof(void*));
            ((void**)current_node)[0] = new_node; // Link new_node as a child.
            // A specific behavior: the new child's character is set to the *next* character in the word.
            *(char *)((char*)new_node + 2 * sizeof(void*)) = word[char_index + 1];
        }
        current_node = ((void**)current_node)[0]; // Move to the child node.
        char_index++; // Increment index for the next character.
    }

    // Set the value pointer for the final node reached/created.
    // The value pointer is stored at an offset of 3 * sizeof(void*) from the node's base.
    ((void**)current_node)[3] = val_ptr;
}

// Function: findInTrie
// Searches for a word in the Trie.
// param_1: A pointer to the root of the Trie (void**).
// param_2: The word (char*) to find.
// Returns a pointer to the Trie node if the word is found and has an associated value,
// otherwise returns NULL.
void *findInTrie(void **root_ptr, char *word) {
  unsigned int char_index = 0; // Current character index in word
  void *found_node = NULL;     // Stores the node matching the current character
  void *current_search_head;   // Head of the current sibling list to search

  // If the Trie is empty, the word cannot be found.
  if (*root_ptr == NULL) {
    return NULL;
  }

  current_search_head = *root_ptr;

  // Loop to traverse the Trie based on the characters in the word.
  while (1) {
    found_node = NULL; // Reset for each level of search.
    void *iter = current_search_head;

    // Search for the character `word[char_index]` in the current sibling list.
    while (iter != NULL) {
      // Check if the character at the current node (offset 2 * sizeof(void*)) matches.
      if (*(char *)((char*)iter + 2 * sizeof(void*)) == word[char_index]) {
        found_node = iter;
        break; // Character found, break from sibling search.
      }
      iter = ((void**)iter)[1]; // Move to the next sibling (index 1).
    }

    // If no matching character was found in the current sibling list.
    if (found_node == NULL) {
      return NULL; // Word cannot be found.
    }

    // If the current character in the word is the null terminator, it means we've reached the end of the word.
    if (word[char_index] == '\0') {
      // Check if the `found_node` has an associated value (index 3).
      if (((void**)found_node)[3] != NULL) {
        return found_node; // Word found and has a value.
      }
      return NULL; // Word found but no value associated.
    }

    // If the character was found and it's not the end of the word, move to its children.
    current_search_head = ((void**)found_node)[0]; // Move to the child of the found node (index 0).

    // If there are no children, the word path cannot continue.
    if (current_search_head == NULL) {
      return NULL;
    }

    char_index++; // Move to the next character in the word.
  }
}