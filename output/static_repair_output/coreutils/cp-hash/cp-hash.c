#include <stdlib.h> // For free, size_t, NULL
#include <stdint.h> // For unsigned long long, uintptr_t
#include <string.h> // For strdup (if xstrdup is a wrapper for it)

// --- External declarations (assuming these are provided elsewhere) ---
// These functions are placeholders for the actual implementations.
// In a real project, these would be linked from a library or defined in other files.
extern void* xmalloc(size_t size);
extern char* xstrdup(const char* s);
extern _Noreturn void xalloc_die(void); // _Noreturn hint for functions that don't return

// Type for the hash table handle
typedef void* hash_table_t;

// Hash table function prototypes (simplified based on usage)
// For hash_initialize, the parameters suggest:
// size_t initial_capacity, int flags_or_strategy, size_t max_capacity,
// int (*compare_func)(const void* key1, const void* key2),
// void (*free_func)(void* item_to_free)
extern hash_table_t hash_initialize(size_t param1, int param2, size_t param3,
                                   int (*compare_func)(const void*, const void*),
                                   void (*free_func)(void*));
extern void* hash_insert(hash_table_t ht, void* item);     // item is a pointer to the data to insert
extern void* hash_lookup(hash_table_t ht, const void* key); // key is a pointer to the key data
extern void* hash_remove(hash_table_t ht, const void* key); // key is a pointer to the key data

// Global hash table instance
extern hash_table_t src_to_dest;

// --- Structures derived from memory access patterns ---

// Key structure for src_to_dest hash table
// Used in forget_created and src_to_dest_lookup
// Corresponds to the 16 bytes passed as `&local_28` where `local_28` and `local_20` were `undefined8`
typedef struct {
    unsigned long long key1; // Corresponds to param_1 in forget_created/src_to_dest_lookup
    unsigned long long key2; // Corresponds to param_2 in forget_created/src_to_dest_lookup
} hash_key_t;

// Item structure stored in src_to_dest hash table
// Allocated with xmalloc(0x18) (24 bytes) in remember_copied
// field0 at offset 0, field1 at offset 8, string_data at offset 16
typedef struct {
    unsigned long long field0;      // Corresponds to param_2 in remember_copied
    unsigned long long field1;      // Corresponds to param_3 in remember_copied
    char*              string_data; // Corresponds to xstrdup(param_1) in remember_copied
} hash_item_t;


// Function: src_to_dest_hash
// Original: ulong src_to_dest_hash(ulong *param_1,ulong param_2)
// Assuming param_1 points to an unsigned long, and param_2 is an unsigned long.
unsigned long src_to_dest_hash(unsigned long *param_1, unsigned long param_2) {
  return *param_1 % param_2;
}

// Function: src_to_dest_compare
// Original: undefined8 src_to_dest_compare(ulong *param_1,ulong *param_2)
// This is a comparison function for the hash table.
// It takes two pointers to keys (hash_key_t).
// Returns 0 if equal, non-zero if not equal.
int src_to_dest_compare(const void *param_1_ptr, const void *param_2_ptr) {
  const hash_key_t *key1 = (const hash_key_t *)param_1_ptr;
  const hash_key_t *key2 = (const hash_key_t *)param_2_ptr;

  // uVar1 = *param_2 ^ *param_1 | param_1[1] ^ param_2[1];
  // This logic checks if key1->key1 == key2->key1 AND key1->key2 == key2->key2.
  // If uVar1 is 0, then both pairs are equal.
  unsigned long long uVar1 = (key2->key1 ^ key1->key1) | (key1->key2 ^ key2->key2);
  // Original return was CONCAT71((int7)(uVar1 >> 8),uVar1 == 0);
  // Interpreting this for a comparison function: return 0 if equal, non-zero otherwise.
  return uVar1 != 0;
}

// Function: src_to_dest_free
// Original: void src_to_dest_free(void *param_1)
// This is the free function for hash_item_t.
void src_to_dest_free(void *param_1) {
  hash_item_t *item = (hash_item_t *)param_1;
  // free(*(void **)((long)param_1 + 0x10));
  // This frees the string_data member.
  free(item->string_data);
  // free(param_1);
  // This frees the item itself.
  free(item);
}

// Function: forget_created
// Original: void forget_created(undefined8 param_1,undefined8 param_2)
// param_1 and param_2 form the key for the hash table.
void forget_created(unsigned long long param_1, unsigned long long param_2) {
  hash_key_t key = { .key1 = param_1, .key2 = param_2 };
  void* removed_item = hash_remove(src_to_dest, &key);
  if (removed_item != NULL) {
    src_to_dest_free(removed_item);
  }
  // Removed stack canary check (compiler handles this automatically).
}

// Function: src_to_dest_lookup
// Original: undefined8 src_to_dest_lookup(undefined8 param_1,undefined8 param_2)
// param_1 and param_2 form the key for the hash table.
// Returns the string_data pointer from the found item, or NULL if not found.
char* src_to_dest_lookup(unsigned long long param_1, unsigned long long param_2) {
  hash_key_t key = { .key1 = param_1, .key2 = param_2 };
  hash_item_t* found_item = (hash_item_t*)hash_lookup(src_to_dest, &key);
  // Removed stack canary check (compiler handles this automatically).
  return (found_item == NULL) ? NULL : found_item->string_data;
}

// Function: remember_copied
// Original: undefined8 remember_copied(undefined8 param_1,undefined8 param_2,undefined8 param_3)
// param_1 is string data, param_2 and param_3 are other fields.
// Returns the string_data of the *existing* item if a collision occurred,
// otherwise returns NULL (indicating the new item was inserted).
char* remember_copied(const char* param_1, unsigned long long param_2, unsigned long long param_3) {
  hash_item_t* new_item = (hash_item_t*)xmalloc(sizeof(hash_item_t));
  if (new_item == NULL) {
      xalloc_die(); // xmalloc might return NULL before xalloc_die is called.
  }
  new_item->field0 = param_2;
  new_item->field1 = param_3;
  new_item->string_data = xstrdup(param_1);
  if (new_item->string_data == NULL) {
      // If xstrdup fails, free the partially allocated item and die.
      free(new_item);
      xalloc_die();
  }

  void* existing_item_ptr = hash_insert(src_to_dest, new_item);

  if (existing_item_ptr == NULL) {
    // hash_insert returning NULL usually indicates an allocation failure or similar fatal error
    // xalloc_die() is called in the original, so this path is for critical failure.
    xalloc_die();
  }

  if (existing_item_ptr == new_item) {
    // The new item was successfully inserted, no existing item found.
    return NULL;
  } else {
    // An existing item was found. Free the newly created item and its string.
    src_to_dest_free(new_item); // This frees new_item->string_data and new_item itself.
    // Return the string_data from the existing item.
    return ((hash_item_t*)existing_item_ptr)->string_data;
  }
}

// Function: hash_init
void hash_init(void) {
  // src_to_dest = hash_initialize(0x67,0,0x100000,src_to_dest_compare,src_to_dest_free);
  // Parameters for hash_initialize:
  // 0x67 (103) - initial capacity?
  // 0 - flags?
  // 0x100000 (1048576) - max capacity?
  // src_to_dest_compare - comparison function pointer
  // src_to_dest_free - free function pointer
  src_to_dest = hash_initialize(103, 0, 1048576, src_to_dest_compare, src_to_dest_free);
  if (src_to_dest == NULL) {
    xalloc_die();
  }
}