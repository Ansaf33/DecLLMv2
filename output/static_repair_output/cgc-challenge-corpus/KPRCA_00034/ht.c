#include <stdlib.h>  // For calloc, free, malloc, strdup
#include <string.h>  // For memset, strdup (often here or stdlib.h)
#include <strings.h> // For strcasecmp on Linux/POSIX systems
#include <ctype.h>   // For tolower
#include <stdbool.h> // For bool
#include <stdint.h>  // For fixed-width integer types, though 'unsigned int' is used for 'uint'

// Using standard C types for clarity
typedef unsigned int uint;

// Hash table entry structure
typedef struct ht_entry {
    struct ht_entry *next; // Pointer to the next entry in the global linked list
    struct ht_entry *prev; // Pointer to the previous entry in the global linked list
    char *key;             // The string key
    void *value;           // The associated value
} ht_entry;

// Hash table structure
typedef struct {
    ht_entry *head;        // Head of the global linked list of all entries
    ht_entry **table;      // Array of pointers to ht_entry for the hash table itself
    uint capacity;         // Current capacity of the hash table array
    uint count;            // Number of actual entries stored
} ht;

// Constant for hash table load factor
static const double HT_LOAD_FACTOR = 0.75; // A common load factor

// Max entries, derived from original code's `0x2000` (8192)
#define HT_MAX_ENTRIES 8192

// Forward declarations for internal functions
uint _hash(uint capacity, const char *key);
int ht_compare(const char *s1, const char *s2);
int _do_insert(ht_entry **table, uint capacity, ht_entry *entry);
int _ht_resize(ht *h);
bool ht_lookup(ht *h, const char *key, ht_entry **found_entry_out);


// Function: _hash
// Calculates a hash for the given key and returns an index within the table capacity.
uint _hash(uint capacity, const char *key) {
  uint hash = 0x3505; // Initial hash value
  for (int i = 0; key[i] != '\0'; ++i) {
    hash = (uint)tolower((unsigned char)key[i]) + hash * 0x25;
  }
  return hash % capacity;
}

// Function: ht_compare
// Compares two strings case-insensitively.
int ht_compare(const char *s1, const char *s2) {
  return strcasecmp(s1, s2);
}

// Function: _do_insert
// Inserts an entry into the hash table array using linear probing.
// Returns 0 on success, 1 on failure (table full or probe path exhausted).
int _do_insert(ht_entry **table, uint capacity, ht_entry *entry) {
  uint initial_idx = _hash(capacity, entry->key);
  uint idx = initial_idx;

  for (;;) {
    if (table[idx] == NULL) {
      table[idx] = entry;
      return 0; // Success
    }
    // Check if we've probed all slots in the current capacity.
    // (initial_idx - 1 + capacity) % capacity is the slot just before initial_idx, circularly.
    if (idx == ((initial_idx - 1 + capacity) % capacity)) {
      return 1; // Table full or probe path exhausted
    }
    idx = (idx + 1) % capacity; // Linear probing
  }
}

// Function: _ht_resize
// Doubles the hash table capacity and rehashes all existing entries.
// Returns 0 on success, 1 on failure (memory allocation).
int _ht_resize(ht *h) {
  uint new_capacity = h->capacity * 2;
  // Allocate new table with doubled capacity
  ht_entry **new_table = calloc(new_capacity, sizeof(ht_entry *));

  if (new_table == NULL) {
    return 1; // Memory allocation failed
  }

  // Rehash all existing entries into the new table
  for (ht_entry *entry = h->head; entry != NULL; entry = entry->next) {
    // _do_insert handles linear probing for the new table
    if (_do_insert(new_table, new_capacity, entry) != 0) {
      // This should not happen if new_capacity is sufficiently large.
      free(new_table);
      return 1;
    }
  }

  free(h->table); // Free the old table array (not the entries themselves)
  h->table = new_table;
  h->capacity = new_capacity;
  return 0; // Success
}

// Function: ht_init
// Initializes a hash table structure.
void ht_init(ht *h) {
  memset(h, 0, sizeof(ht)); // Initialize all members to 0/NULL
  h->capacity = 4;          // Initial capacity
  _ht_resize(h);            // Allocate initial table and set capacity
}

// Function: ht_lookup
// Looks up a key in the hash table.
// Returns true if key is NOT found, false if key IS found.
// If found, `found_entry_out` points to the entry.
bool ht_lookup(ht *h, const char *key, ht_entry **found_entry_out) {
  uint idx = _hash(h->capacity, key);

  while (h->table[idx] != NULL && ht_compare(h->table[idx]->key, key) != 0) {
    idx = (idx + 1) % h->capacity;
  }

  if (h->table[idx] != NULL) { // Found a matching entry
    *found_entry_out = h->table[idx];
    return false; // Found
  } else { // Slot is empty, key not found
    *found_entry_out = NULL;
    return true; // Not found
  }
}

// Function: ht_delete
// Deletes an entry by key from the hash table.
// Returns 0 on success, 1 on failure (key not found).
int ht_delete(ht *h, const char *key, void **value_out) {
  ht_entry *entry_to_delete = NULL;
  if (ht_lookup(h, key, &entry_to_delete)) {
    return 1; // Not found
  }

  *value_out = entry_to_delete->value;

  // Unlink from the global linked list
  if (entry_to_delete->prev == NULL) {
    h->head = entry_to_delete->next;
  } else {
    entry_to_delete->prev->next = entry_to_delete->next;
  }
  if (entry_to_delete->next != NULL) {
    entry_to_delete->next->prev = entry_to_delete->prev;
  }

  // Find the actual index of the entry to delete in the hash table array
  uint deleted_entry_initial_hash = _hash(h->capacity, entry_to_delete->key);
  uint deleted_entry_actual_idx = deleted_entry_initial_hash;
  while (h->table[deleted_entry_actual_idx] != entry_to_delete) {
    deleted_entry_actual_idx = (deleted_entry_actual_idx + 1) % h->capacity;
  }

  h->table[deleted_entry_actual_idx] = NULL; // Clear the slot
  h->count--;

  // Rehash subsequent elements in the cluster to fill the gap (Robin Hood deletion / backward shift)
  uint hole_idx = deleted_entry_actual_idx;
  uint current_idx;
  uint current_entry_hash; // Original hash of the entry at current_idx

  // Iterate forward from the slot *after* the hole
  for (current_idx = (hole_idx + 1) % h->capacity; h->table[current_idx] != NULL; current_idx = (current_idx + 1) % h->capacity) {
    current_entry_hash = _hash(h->capacity, h->table[current_idx]->key);

    // Condition to move the entry from current_idx to hole_idx:
    // The entry at current_idx should be moved if its ideal hash position
    // is "before or at" the current hole_idx (circularly) AND
    // "after" the current_idx (circularly).
    // This is equivalent to checking if `current_entry_hash` is in the circular range `[hole_idx, current_idx)`.
    bool should_move = false;
    if (hole_idx <= current_idx) { // No wrap-around between hole_idx and current_idx
        if (current_entry_hash <= hole_idx || current_entry_hash > current_idx) {
            should_move = true;
        }
    } else { // Wrap-around between hole_idx and current_idx (e.g., hole=CAP-1, current=0)
        if (current_entry_hash <= hole_idx && current_entry_hash > current_idx) {
            should_move = true;
        }
    }

    if (should_move) {
      h->table[hole_idx] = h->table[current_idx]; // Move entry
      h->table[current_idx] = NULL;              // Clear old slot
      hole_idx = current_idx;                     // The hole moves forward
    }
    // If should_move is false, the element at 'current_idx' is correctly placed or
    // cannot be moved to 'hole_idx' without breaking its probe chain.
    // In this case, no further elements in this cluster need to be moved
    // to fill 'hole_idx' (or a hole that propagates from 'hole_idx').
  }

  free(entry_to_delete->key);
  free(entry_to_delete);
  return 0; // Success
}

// Function: ht_insert
// Inserts a new key-value pair into the hash table.
// Returns 0 on success, 1 on failure (key exists, max entries, or memory allocation).
int ht_insert(ht *h, const char *key, void *value) {
  ht_entry *existing_entry = NULL;
  if (!ht_lookup(h, key, &existing_entry)) {
    return 1; // Key already exists
  }
  if (h->count >= HT_MAX_ENTRIES) {
    return 1; // Max entries reached
  }

  char *new_key = strdup(key);
  if (new_key == NULL) {
    return 1; // Memory allocation failed
  }

  ht_entry *new_entry = malloc(sizeof(ht_entry));
  if (new_entry == NULL) {
    free(new_key);
    return 1; // Memory allocation failed
  }
  new_entry->key = new_key;
  new_entry->value = value;

  // Check if resize is needed (load factor exceeded)
  // (h->count + 1) because we are about to add one more entry
  if ((double)(h->count + 1) >= (HT_LOAD_FACTOR * h->capacity)) {
    if (_ht_resize(h) != 0) {
      free(new_key);
      free(new_entry);
      return 1; // Resize failed
    }
  }

  // Insert into the hash table array
  if (_do_insert(h->table, h->capacity, new_entry) != 0) {
    // This should ideally not fail after a resize that guarantees space,
    // unless there's a bug in _do_insert or the resize logic.
    free(new_key);
    free(new_entry);
    return 1; // Insert failed
  }

  h->count++;

  // Add to the global linked list (at head)
  new_entry->prev = NULL;
  new_entry->next = h->head;
  if (h->head != NULL) {
    h->head->prev = new_entry;
  }
  h->head = new_entry;

  return 0; // Success
}

// Function: ht_first
// Returns the first entry in the global linked list of all entries.
ht_entry *ht_first(ht *h) {
  return h->head;
}

// Function: ht_next
// Returns the next entry in the global linked list.
ht_entry *ht_next(ht_entry *current_entry) {
  if (current_entry == NULL) {
    return NULL;
  }
  return current_entry->next;
}