#include <stdlib.h> // For calloc, malloc, free
#include <string.h> // For strcmp, strdup

// Define the dictionary node structure
typedef struct Node {
    char *key;
    char *value;
    struct Node *next;
} Node;

// Define the number of hash table buckets.
// The original code uses `iVar1 * 4` for pointer arithmetic, implying sizeof(pointer) is 4 bytes,
// and `calloc(4, 0x100)` implies 4 elements. We'll use 4 buckets.
#define DICT_BUCKET_COUNT 4

// Function: dict_new
// Allocates and initializes a new dictionary.
// Returns a pointer to the dictionary (an array of Node pointers).
Node** dict_new(void) {
  return (Node**)calloc(DICT_BUCKET_COUNT, sizeof(Node*));
}

// Function: _hash
// Computes a hash value for a given key.
// key: The string key to hash.
// Returns a char representing the hash bucket index.
static char _hash(const char *key) {
  char hash_val = '+'; // ASCII 43
  if (key != NULL) {
    for (; *key != '\0'; key++) {
      hash_val = *key + hash_val * '%'; // '%': ASCII 37
    }
  }
  // Ensure the result is non-negative and within the bucket count.
  return (char)((unsigned char)hash_val % DICT_BUCKET_COUNT);
}

// Function: dict_find
// Finds a key in the dictionary and returns its associated value.
// dict: Pointer to the dictionary (array of Node pointers).
// key: The key to search for.
// Returns the value associated with the key, or NULL if not found.
char * dict_find(Node** dict, const char *key) {
  char hash_val = _hash(key);
  Node *current_node = dict[hash_val];

  while(current_node != NULL) {
    if (strcmp(current_node->key, key) == 0) {
      return current_node->value;
    }
    current_node = current_node->next;
  }
  return NULL;
}

// Function: dict_insert
// Inserts a key-value pair into the dictionary.
// If the key already exists, the operation is skipped (as per original logic).
// dict: Pointer to the dictionary.
// key: The key to insert.
// value: The value to associate with the key.
void dict_insert(Node** dict, const char *key, char *value) {
  // Only insert if the key does not already exist
  if (dict_find(dict, key) == NULL) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if (new_node != NULL) {
      new_node->key = strdup(key);
      if (new_node->key != NULL) { // Check if strdup was successful
        new_node->value = value;
        char hash_val = _hash(key);
        new_node->next = dict[hash_val]; // Prepend to the list
        dict[hash_val] = new_node;
      } else {
        // strdup failed, free the allocated node
        free(new_node);
      }
    }
  }
}

// Function: dict_remove
// Removes a key-value pair from the dictionary.
// dict: Pointer to the dictionary.
// key: The key to remove.
// Returns the value associated with the removed key, or NULL if not found.
char * dict_remove(Node** dict, const char *key) {
  char hash_val = _hash(key);
  Node *current_node = dict[hash_val];
  Node *prev_node = NULL;

  while(current_node != NULL) {
    if (strcmp(current_node->key, key) == 0) {
      // Key found, remove the node
      if (prev_node == NULL) {
        // Node to remove is the head of the list
        dict[hash_val] = current_node->next;
      } else {
        // Node to remove is not the head
        prev_node->next = current_node->next;
      }
      char *removed_value = current_node->value; // Store value before freeing node
      free(current_node->key); // Free the duplicated key string
      free(current_node); // Free the node itself
      return removed_value;
    }
    prev_node = current_node;
    current_node = current_node->next;
  }
  return NULL; // Key not found
}