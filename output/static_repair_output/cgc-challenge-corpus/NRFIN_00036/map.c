#include <stdio.h>    // For fprintf (in _terminate)
#include <stdlib.h>   // For malloc, exit
#include <string.h>   // For strcmp, strcpy, memset
#include <stdint.h>   // For uintptr_t, uint32_t

// Define undefined4 as a 32-bit unsigned integer type.
// This assumes pointers are either 32-bit or their addresses are stored as 32-bit values,
// consistent with the 0x20 (32-byte) size of MapEntry.
typedef uint32_t undefined4;

// Define the MapEntry structure.
// The 'next' pointer is stored as a 32-bit unsigned integer (next_ptr_val) to maintain
// a fixed 32-byte size for each entry, crucial for the arithmetic in initMap and getSize.
typedef struct MapEntry {
    undefined4 next_ptr_val; // Stores the address of the next MapEntry as a 32-bit unsigned integer
    undefined4 value;        // The value associated with the key
    char key[24];            // The key string (24 bytes)
} MapEntry; // Total size: 4 (next_ptr_val) + 4 (value) + 24 (key) = 32 bytes (0x20)

// --- Dummy Implementations for Missing Functions ---

// _terminate: Exits the program with a given status.
void _terminate(int status) {
    fprintf(stderr, "Error: _terminate called with status %d\n", status);
    exit(status);
}

// allocate: Simulates memory allocation.
// Returns 0 on success, non-zero on failure.
// `out_ptr` receives the address of the newly allocated memory block.
int allocate(size_t size, int flags1, MapEntry **out_ptr, int flags2) {
    *out_ptr = (MapEntry *)malloc(size);
    if (*out_ptr == NULL) {
        fprintf(stderr, "Allocation failed for size %zu\n", size);
        return -1; // Indicate failure
    }
    return 0; // Indicate success
}

// --- Forward Declarations ---
int getSize(MapEntry *head);

// --- Function Implementations ---

// Function: initMap
// Initializes or finds a map entry.
// If param_1 is 0, allocates a new block and returns a specific offset within it.
// If param_1 is non-zero, it's treated as a pointer to a MapEntry and searches backward.
int initMap(int param_1) {
    MapEntry *map_ptr;

    if (param_1 == 0) {
        if (allocate(0x1000, 0, &map_ptr, 0x1100c) != 0) {
            _terminate(3);
        }
        // Offset by 0xfe0 bytes from the allocated block's base.
        map_ptr = (MapEntry *)((char *)map_ptr + 0xfe0);
    } else {
        // Cast param_1 (an int representing an address) to MapEntry pointer.
        map_ptr = (MapEntry *)(uintptr_t)param_1;
        // Search backward in memory, decrementing by 0x20 (32 bytes) until an empty key is found.
        for (; map_ptr->key[0] != '\0'; map_ptr = (MapEntry *)((char *)map_ptr - 0x20)) {
        }
    }
    // Return the final pointer address as an int.
    return (int)(uintptr_t)map_ptr;
}

// Function: setMap
// Sets a value for a given key in the map.
// If the key doesn't exist, a new entry is created (up to a limit).
// Returns 1 on success, 0 if the map is full and a new entry cannot be created.
undefined4 setMap(undefined4 *param_1, char *param_2, undefined4 param_3) {
    // param_1 is a pointer to the head of the linked list (which is an undefined4 value).
    MapEntry **head_ptr_addr = (MapEntry **)param_1;
    MapEntry *prev = NULL;
    // Cast the head address value to a MapEntry pointer.
    MapEntry *curr = (MapEntry *)(uintptr_t)*head_ptr_addr;

    // Traverse the list to find the key or the end of the list.
    while (curr != NULL && strcmp(param_2, curr->key) != 0) {
        prev = curr;
        // Move to the next node by casting its stored address value to a MapEntry pointer.
        curr = (MapEntry *)(uintptr_t)curr->next_ptr_val;
    }

    if (curr == NULL) { // Key not found, create a new entry.
        // Check if the map has reached its maximum size (0x81 = 129 entries).
        if (getSize((MapEntry *)(uintptr_t)*head_ptr_addr) == 0x81) {
            return 0; // Map is full.
        }
        // Allocate a new map entry using initMap.
        // Pass prev (or NULL if head) to initMap, which finds an available slot.
        curr = (MapEntry *)(uintptr_t)initMap((int)(uintptr_t)prev);
        
        // Link the new node: its 'next' points to the current head.
        curr->next_ptr_val = *head_ptr_addr;
        // Update the head of the list to be the new node.
        *head_ptr_addr = (undefined4)(uintptr_t)curr;
    }

    // If the key field is empty (first char is null), copy the new key.
    if (curr->key[0] == '\0') {
        strcpy(curr->key, param_2);
    }
    curr->value = param_3; // Set the value.
    return 1;
}

// Function: getValue
// Retrieves the value associated with a given key.
// Returns the value if found, otherwise returns 0.
undefined4 getValue(undefined4 *param_1, char *param_2) {
    // Start from the head of the list.
    MapEntry *curr = (MapEntry *)(uintptr_t)*param_1;

    // Traverse the list.
    while (curr != NULL) {
        // Check for a specific sentinel value (0xfffffff8) or if the key matches.
        if ((uintptr_t)curr != 0xfffffff8 && strcmp(param_2, curr->key) == 0) {
            break; // Key found.
        }
        // Move to the next node.
        curr = (MapEntry *)(uintptr_t)curr->next_ptr_val;
    }

    // If key not found or sentinel value reached, return 0.
    if (curr == NULL || (uintptr_t)curr == 0xfffffff8) {
        return 0;
    }
    return curr->value; // Return the associated value.
}

// Function: getSize
// Counts the number of active entries in the linked list.
// Returns the count.
int getSize(MapEntry *head) {
    int count = 0;
    MapEntry *curr = head;

    // Traverse the list and increment count for each node.
    while (curr != NULL) {
        count++;
        curr = (MapEntry *)(uintptr_t)curr->next_ptr_val;
    }
    return count;
}

// Function: removeMap
// Removes an entry with the specified key from the map.
void removeMap(undefined4 *param_1, char *param_2) {
    // param_1 is a pointer to the head of the linked list.
    MapEntry **head_ptr_addr = (MapEntry **)param_1;
    MapEntry *prev = NULL;
    MapEntry *curr = (MapEntry *)(uintptr_t)*head_ptr_addr;

    // Traverse the list to find the key to remove.
    while (curr != NULL && curr->key[0] != '\0' && strcmp(param_2, curr->key) != 0) {
        prev = curr;
        curr = (MapEntry *)(uintptr_t)curr->next_ptr_val;
    }

    if (curr != NULL) { // If a node with the matching key was found.
        if (prev == NULL) { // If it's the head node.
            // Update the head of the list to point to the next node.
            *head_ptr_addr = curr->next_ptr_val;
        } else { // If it's not the head node.
            // Link the previous node to the current node's next.
            prev->next_ptr_val = curr->next_ptr_val;
        }
        // Clear the contents of the removed node, effectively marking it as "free".
        // The original code does not call `free()`, implying a memory pool management.
        memset(curr->key, 0, sizeof(curr->key));
        curr->value = 0;
        curr->next_ptr_val = 0;
    }
    return;
}