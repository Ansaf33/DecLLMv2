#include <stdio.h>   // Required for potential debugging output, though not used in final code
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For strcmp, strcpy, memset
#include <stdint.h>  // For uintptr_t for robust pointer-to-integer conversions

// Alias for the original `undefined4` type, assuming it's a 4-byte unsigned integer.
typedef unsigned int undefined4;

// Define the MapEntry structure.
// Based on the observed 0x20 (32 bytes) size per entry and field access patterns.
// Assumes a 32-bit pointer size for `next` (4 bytes).
typedef struct MapEntry {
    struct MapEntry *next; // Offset 0: Pointer to the next entry in the linked list (4 bytes)
    undefined4 value;      // Offset 4: The value associated with the key (4 bytes)
    char key[24];          // Offset 8: The key string (max 23 chars + null terminator) (24 bytes)
} MapEntry; // Total size: 4 + 4 + 24 = 32 bytes (0x20)

// --- Helper/Dummy Functions (Originals not provided) ---

// Global variables to manage a single allocated block for map entries.
// This is a simplified memory management strategy derived from initMap's behavior.
static MapEntry *g_map_block_start = NULL; // Points to the beginning of the allocated block.
static MapEntry *g_map_block_end = NULL;   // Points one past the last entry in the allocated block.

// Dummy `allocate` function. Simulates a custom memory allocator.
// It allocates `size` bytes and stores the pointer in `*out_ptr`.
// Returns 0 on success, 1 on failure.
static int allocate(size_t size, int flags_a, MapEntry **out_ptr, int flags_b) {
    *out_ptr = (MapEntry *)malloc(size);
    if (*out_ptr == NULL) {
        return 1; // Allocation failed
    }
    memset(*out_ptr, 0, size); // Initialize allocated memory to zeros
    return 0; // Success
}

// Dummy `_terminate` function. Simulates a program termination routine.
static void _terminate(int status) {
    exit(status);
}

// --- Original Functions (Fixed and Refactored) ---

// Function: initMap
// `param_1` is either 0 (to allocate a new block) or an address (uintptr_t) of an entry
// to start searching backwards from for a free slot.
int initMap(uintptr_t param_1) {
    MapEntry *found_entry_ptr; // Renamed from local_14

    if (param_1 == 0) {
        // If no block has been allocated yet, allocate one.
        // This logic assumes a single block for simplicity based on the original snippet.
        if (g_map_block_start == NULL) {
            // Allocate 0x1000 bytes (4096 bytes), which can hold 128 MapEntry structs (4096 / 32).
            if (allocate(0x1000, 0, &g_map_block_start, 0x1100c) != 0) {
                _terminate(3); // Allocation failed, terminate program.
            }
            g_map_block_end = g_map_block_start + (0x1000 / sizeof(MapEntry));
        }
        // The original code `local_14 = local_14 + 0xfe0;` (where local_14 was the block start)
        // implies returning the address of the last entry in the block (index 127).
        found_entry_ptr = g_map_block_start + (0x1000 / sizeof(MapEntry) - 1);
    } else {
        // Search backwards from the given address (`param_1`) for an empty slot (key[0] == '\0').
        found_entry_ptr = (MapEntry *)param_1;
        // Ensure we don't search outside the bounds of the allocated block.
        // This assumes param_1 is within g_map_block_start and g_map_block_end.
        while (found_entry_ptr >= g_map_block_start && found_entry_ptr->key[0] != '\0') {
            found_entry_ptr--; // Move to the previous 32-byte slot.
        }
        // If we've gone past the beginning of the block without finding an empty slot,
        // or if param_1 was initially out of bounds.
        if (found_entry_ptr < g_map_block_start) {
            return 0; // Indicate no free slot found.
        }
    }
    // Return the address of the found (or last in block) entry as an integer.
    return (int)((uintptr_t)found_entry_ptr);
}

// Function: getSize
// Counts the number of active entries in the linked list starting from `head`.
// The original calculation `((uint)((int)local_8 - (int)param_1) >> 5) + 1`
// implies a contiguous array and is inconsistent with linked list traversal.
// This implementation counts nodes in the linked list, which is a more standard
// interpretation of "getSize" for a map-like structure.
int getSize(MapEntry *head) {
    int count = 0;
    MapEntry *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Function: setMap
// Sets a value for a given key in the map.
// `head_ptr` is a pointer to the head of the linked list (MapEntry**).
// `key_str` is the key to set.
// `value_to_set` is the value to associate with the key.
undefined4 setMap(MapEntry **head_ptr, char *key_str, undefined4 value_to_set) {
    MapEntry *current = *head_ptr;
    MapEntry *prev = NULL;

    // Search for an existing entry with the given key.
    while (current != NULL && strcmp(key_str, current->key) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) { // Key not found, create a new entry.
        // Check if the map has reached its maximum size (0x81 = 129 entries).
        if (getSize(*head_ptr) == 0x81) {
            return 0; // Map is full, cannot add new entry.
        }

        // Allocate or find a free slot for a new entry using initMap.
        // initMap returns an address as an int, which is cast back to MapEntry*.
        // If `prev` is NULL, `initMap(0)` allocates a new block (if not already done)
        // and returns the last slot in it.
        // If `prev` is not NULL, `initMap((uintptr_t)prev)` searches backwards from `prev`
        // for an empty slot.
        MapEntry *new_entry = (MapEntry *)initMap((uintptr_t)prev);
        if (new_entry == NULL) {
            return 0; // Failed to find/allocate a slot.
        }

        // Link the new entry into the linked list.
        if (prev == NULL) { // New entry becomes the new head.
            new_entry->next = *head_ptr; // Link new entry to the original head.
            *head_ptr = new_entry;       // Update the head pointer.
        } else { // New entry is inserted after `prev`.
            new_entry->next = prev->next; // Link new entry to what `prev` used to point to.
            prev->next = new_entry;       // `prev` now points to the new entry.
        }
        current = new_entry; // `current` now refers to the newly added entry.
    }

    // At this point, `current` points to the `MapEntry` for the `key_str` (either existing or new).
    // If the key field is empty (first character is null), copy the provided key.
    if (current->key[0] == '\0') {
        strcpy(current->key, key_str);
    }
    current->value = value_to_set; // Set the value.
    return 1; // Success.
}

// Function: getValue
// Retrieves the value associated with a given key.
// `head_ptr` is a pointer to the head of the linked list (MapEntry**).
// `key_str` is the key to search for.
undefined4 getValue(MapEntry **head_ptr, char *key_str) {
    MapEntry *current = *head_ptr; // Start search from the head of the list.

    while (current != NULL) {
        // The original code included a check `(local_10 != (undefined4 *)0xfffffff8)`.
        // This is an environment-specific check for an invalid pointer value,
        // often related to stack canaries or specific memory regions.
        // It's omitted here for portability as its exact meaning is unknown without context.
        if (strcmp(key_str, current->key) == 0) {
            return current->value; // Key found, return its value.
        }
        current = current->next; // Move to the next entry.
    }
    return 0; // Key not found, return 0 (or a specific error value).
}

// Function: removeMap
// Removes an entry with the specified key from the map.
// `head_ptr` is a pointer to the head of the linked list (MapEntry**).
// `key_str` is the key of the entry to remove.
void removeMap(MapEntry **head_ptr, char *key_str) {
    MapEntry *current = *head_ptr;
    MapEntry *prev = NULL;

    // Search for the entry to remove.
    // The condition `current->key[0] == '\0'` is added to match original behavior
    // where empty keys might be skipped or treated specially during search.
    while (current != NULL && (current->key[0] == '\0' || strcmp(key_str, current->key) != 0)) {
        prev = current;
        current = current->next;
    }

    if (current != NULL) { // Entry found.
        if (prev == NULL) { // Removing the head entry.
            *head_ptr = current->next;
        } else { // Removing a non-head entry.
            prev->next = current->next;
        }
        // Clear the removed entry's data to mark it as an "empty" slot.
        // The original `memset(local_10 + 2,0,0x18)` clears the key field.
        memset(current->key, 0, sizeof(current->key));
        current->value = 0;   // Clear the value.
        current->next = NULL; // Clear the next pointer.
        // Note: The memory for `current` is not freed here. It's likely intended
        // to be reused as an empty slot within the pre-allocated block.
    }
    // If `current` is NULL, the key was not found, so nothing is removed.
}