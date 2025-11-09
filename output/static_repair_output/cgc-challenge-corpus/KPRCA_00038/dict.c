#include <stdint.h>   // For uint32_t
#include <stdbool.h>  // For bool
#include <stdlib.h>   // For realloc, free, NULL
#include <string.h>   // For memset, strcmp, strdup

// Forward declarations of structures
typedef struct Dict Dict;
typedef struct DictEntry DictEntry;

// Function pointer type for freeing DictEntry structures
typedef void (*DictEntryFreeFunc)(DictEntry *);

// DictEntry structure definition
// Offsets are based on original code's interpretation (assuming 32-bit pointers for calculations).
// Direct member access is used for portability across different pointer sizes.
struct DictEntry {
    DictEntry *next_overall;      // Offset 0x00: Next entry in the global linked list
    DictEntry *prev_overall;      // Offset 0x04: Previous entry in the global linked list
    DictEntry *next_in_bucket;    // Offset 0x08: Next entry in the hash bucket linked list
    DictEntry *prev_in_bucket;    // Offset 0x0C: Previous entry in the hash bucket linked list
    char *key;                    // Offset 0x10: Key string for this entry
    // ... potentially other data fields would follow here
};

// Dict structure definition
struct Dict {
    DictEntryFreeFunc free_func; // Offset 0x00: Function to free custom data within a DictEntry
    DictEntry *head;             // Offset 0x04: Head of the overall linked list of all entries
    DictEntry **table;           // Offset 0x08: Pointer to the hash table array (array of DictEntry pointers)
    uint32_t count;              // Offset 0x0C: Number of entries currently in the dictionary
    uint32_t capacity;           // Offset 0x10: Current size of the hash table array
};

// Function prototypes (to handle inter-function dependencies)
uint32_t do_hash(Dict *dict, char *key);
void do_insert(Dict *dict, DictEntry *entry);
void do_unlink(Dict *dict, DictEntry *entry);
uint32_t enlarge_tbl(Dict *dict, uint32_t new_capacity);
DictEntry *dict_get(Dict *dict, char *key);


// Function: do_hash
// Calculates a hash for the given key string and returns it modulo the dictionary's capacity.
uint32_t do_hash(Dict *dict, char *key) {
  uint32_t hash = 0;
  while (*key != '\0') {
    hash = (uint32_t)*key + hash * 0x539; // 0x539 is 1337 in decimal, a common prime for hashing
    key++;
  }
  return hash % dict->capacity;
}

// Function: do_insert
// Inserts a DictEntry into the dictionary. It updates both the hash table
// (bucket list) and the overall linked list of all entries.
void do_insert(Dict *dict, DictEntry *entry) {
  uint32_t hash_idx = do_hash(dict, entry->key);
  DictEntry **bucket_head_ptr = &dict->table[hash_idx];

  // Insert into the hash table bucket (doubly linked list)
  entry->prev_in_bucket = NULL;
  entry->next_in_bucket = *bucket_head_ptr;
  if (entry->next_in_bucket != NULL) {
    entry->next_in_bucket->prev_in_bucket = entry;
  }
  *bucket_head_ptr = entry;

  // Insert into the overall linked list (doubly linked list, acts as head insertion)
  entry->prev_overall = NULL;
  entry->next_overall = dict->head;
  if (entry->next_overall != NULL) {
    entry->next_overall->prev_overall = entry;
  }
  dict->head = entry;
}

// Function: do_unlink
// Removes a DictEntry from the dictionary. It updates both the hash table
// (bucket list) and the overall linked list of all entries.
void do_unlink(Dict *dict, DictEntry *entry) {
  uint32_t hash_idx = do_hash(dict, entry->key);
  DictEntry **bucket_head_ptr = &dict->table[hash_idx];

  // Unlink from hash table bucket
  if (entry == *bucket_head_ptr) {
    *bucket_head_ptr = entry->next_in_bucket;
  }
  if (entry->prev_in_bucket != NULL) {
    entry->prev_in_bucket->next_in_bucket = entry->next_in_bucket;
  }
  if (entry->next_in_bucket != NULL) {
    entry->next_in_bucket->prev_in_bucket = entry->prev_in_bucket;
  }

  // Unlink from overall linked list
  if (entry == dict->head) {
    dict->head = entry->next_overall;
  }
  if (entry->prev_overall != NULL) {
    entry->prev_overall->next_overall = entry->next_overall;
  }
  if (entry->next_overall != NULL) {
    entry->next_overall->prev_overall = entry->prev_overall;
  }
}

// Function: enlarge_tbl
// Resizes the hash table array. If successful, it rehashes all existing entries
// into the new table.
uint32_t enlarge_tbl(Dict *dict, uint32_t new_capacity) {
  if (dict->capacity >= new_capacity) {
    return 1; // No enlargement needed or smaller capacity requested
  }

  DictEntry **new_table = (DictEntry **)realloc(dict->table, new_capacity * sizeof(DictEntry*));
  if (new_table == NULL) {
    return 0; // Allocation failed
  }

  // Initialize new table with NULLs
  memset(new_table, 0, new_capacity * sizeof(DictEntry*));

  dict->table = new_table;
  dict->capacity = new_capacity;

  // Rehash all existing entries into the new table
  DictEntry *old_head = dict->head;
  dict->head = NULL; // Clear the overall list head, do_insert will rebuild it
  
  while (old_head != NULL) {
    DictEntry *current_entry = old_head;
    old_head = old_head->next_overall; // Move to the next entry before modifying current_entry

    // Reset bucket linkage for re-insertion
    current_entry->next_in_bucket = NULL;
    current_entry->prev_in_bucket = NULL;

    do_insert(dict, current_entry);
  }
  return 1;
}

// Function: dict_init
// Initializes a new dictionary structure.
bool dict_init(Dict *dict, DictEntryFreeFunc free_func) {
  dict->free_func = free_func;
  dict->head = NULL;
  dict->table = NULL;
  dict->count = 0;
  dict->capacity = 0; // Initial capacity will be set by enlarge_tbl

  // Enlarge table to initial size of 8
  return enlarge_tbl(dict, 8) != 0;
}

// Function: dict_free
// Frees all entries in the dictionary and the dictionary's internal table.
void dict_free(Dict *dict) {
  DictEntry *current_entry = dict->head;
  while (current_entry != NULL) {
    DictEntry *next_entry = current_entry->next_overall;
    dict->free_func(current_entry); // Call custom free function for entry data
    free(current_entry);            // Free the entry structure itself
    current_entry = next_entry;
  }
  free(dict->table); // Free the hash table array
  
  // Reset dict state
  dict->table = NULL;
  dict->head = NULL;
  dict->count = 0;
  dict->capacity = 0;
}

// Function: dict_get
// Retrieves a DictEntry from the dictionary based on its key.
DictEntry * dict_get(Dict *dict, char *key) {
  uint32_t hash_idx = do_hash(dict, key);
  DictEntry *entry = dict->table[hash_idx];

  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      break; // Found matching entry
    }
    entry = entry->next_in_bucket;
  }
  return entry;
}

// Function: dict_add
// Adds a new entry to the dictionary or replaces an existing one.
// Returns 1 on success, 0 on failure (e.g., memory allocation error).
uint32_t dict_add(Dict *dict, char *new_key_str, DictEntry *new_entry) {
  DictEntry *existing_entry = dict_get(dict, new_key_str);

  if (existing_entry == NULL) {
    // Key does not exist, add a new entry
    // Check if table needs enlargement based on a load factor (0.75)
    if ((double)dict->capacity * 0.75 <= (double)(dict->count + 1)) {
      if (!enlarge_tbl(dict, dict->capacity * 2)) {
        return 0; // Failed to enlarge table
      }
    }
    
    new_entry->key = strdup(new_key_str); // Duplicate the key string
    if (new_entry->key == NULL) {
      return 0; // Failed to duplicate key string
    }
    do_insert(dict, new_entry);
    dict->count++;
  } else {
    // Key already exists, replace the entry
    // The original code transfers the existing key pointer to the new entry.
    // This implies the responsibility for freeing the old key (if not transferred)
    // and the new key (if not `strdup`'d) is outside this function or by free_func.
    new_entry->key = existing_entry->key; // Transfer key pointer from old entry to new entry

    do_unlink(dict, existing_entry);
    dict->free_func(existing_entry); // Call custom free function for old entry's data
    free(existing_entry);            // Free the old entry structure

    do_insert(dict, new_entry);
    // dict->count is not changed as one entry is replaced by another
  }
  return 1;
}