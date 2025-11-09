#include <stdio.h>    // For fprintf, stderr, FILE
#include <stdlib.h>   // For calloc, free, exit
#include <string.h>   // For memcpy
#include <stdint.h>   // For uintptr_t

// Define types based on common decompilation patterns
typedef unsigned int undefined4; // Generic 4-byte unsigned integer
typedef unsigned char byte;      // 1-byte unsigned
typedef unsigned int uint;       // Generic unsigned integer

// External function declarations (placeholders)
// These functions are assumed to be defined elsewhere and linked.
// Return types and parameter types are inferred from usage in the snippets.
extern int* GatherTerminals(void* config_ptr, uint *out_terminal_count); // Returns array of int (pointers)
extern undefined4 GetTrieCount(); // Returns unsigned int
extern int GetDataString(); // Returns int (possibly a pointer as an int)
extern void AllocateAndInitializeListHead(void** list_head_ptr_location); // Initializes the pointer at the given location
extern int FindInTrieByIdentifier(uint id, void* config_ptr); // Returns int (possibly a pointer as an int)
extern void FreeList(undefined4 list_head); // Takes a value

// Global string literals (placeholders for DAT_0001703a)
const char *DAT_0001703a = "Error: Allocation failed or similar critical issue.\n";

// Helper function for terminating on critical errors, replacing _terminate()
static void _terminate() {
    exit(1);
}

// Helper function for xcalloc, replacing xcalloc()
static void* xcalloc(size_t nmemb, size_t size) {
    void* ptr = calloc(nmemb, size);
    if (ptr == NULL) {
        fprintf(stderr, DAT_0001703a);
        _terminate();
    }
    return ptr;
}

// A structure to represent the SearchMachine state, making param_1 more readable
// The original param_1 is int*, implying an array of int-sized values.
// These values are often pointers in decompiled code.
typedef struct SearchMachineState {
    int root_node_id;       // param_1[0], stores an int value (likely an address cast to int)
    void **list_heads;      // param_1[1], array of void* (pointers to list heads)
    void **trie_nodes_array_1; // param_1[2], array of void* (pointers to trie nodes)
    void **trie_nodes_array_2; // param_1[3], array of void* (each element points to an array of ints)
} SearchMachineState;

// Function: InitializeSearchMachine
// 'machine' is a pointer to the SearchMachineState structure.
// 'config_ptr' is a pointer to some configuration structure, treated as void* for flexibility.
undefined4 InitializeSearchMachine(SearchMachineState *machine, void* config_ptr) {
    uint terminal_count;
    int *terminals_array; // Returned by GatherTerminals, array of int-sized pointers
    unsigned int trie_count;

    machine->root_node_id = (int)(uintptr_t)config_ptr; // Store the address as an int.

    terminals_array = GatherTerminals(config_ptr, &terminal_count);
    if (terminals_array == NULL) { // Assuming GatherTerminals returns NULL on error
        fprintf(stderr, DAT_0001703a);
        _terminate();
    }

    trie_count = GetTrieCount();
    // param_1[1] = iVar2; iVar2 from xcalloc(trie_count, sizeof(void*))
    machine->list_heads = (void**)xcalloc(trie_count, sizeof(void*));

    for (uint i = 0; i < terminal_count; ++i) {
        // terminals_array[i] is an int, but used as a pointer to a structure.
        // `*(int*)((char*)(uintptr_t)terminals_array[i] + 0x408)` accesses a member at offset 0x408.
        int terminal_id_offset_408 = *(int*)((char*)(uintptr_t)terminals_array[i] + 0x408);

        // GetDataString() is called, but its return value `data_string_result`
        // is not explicitly used by `AllocateAndInitializeListHead` in the decompiled snippet.
        // It might be implicitly used (e.g., via a global variable) or is an artifact.
        int data_string_result = GetDataString();
        if (data_string_result == 0) { // Assuming 0 indicates an error.
            fprintf(stderr, DAT_0001703a);
            _terminate();
        }
        // AllocateAndInitializeListHead initializes the pointer at the specified location.
        AllocateAndInitializeListHead(&machine->list_heads[terminal_id_offset_408]);
    }

    trie_count = GetTrieCount();
    machine->trie_nodes_array_1 = (void**)xcalloc(trie_count, sizeof(void*));

    trie_count = GetTrieCount();
    machine->trie_nodes_array_2 = (void**)xcalloc(trie_count, sizeof(void*));

    for (uint i = 0; i < trie_count; ++i) {
        // Each element of trie_nodes_array_2 points to an array of 0x100 ints.
        void* allocated_mem_for_node = xcalloc(0x100, sizeof(int));
        machine->trie_nodes_array_2[i] = allocated_mem_for_node;

        // FindInTrieByIdentifier returns an int, used as a pointer.
        int find_result_ptr_val = FindInTrieByIdentifier(i, config_ptr);
        // Copy 0x400 bytes from `find_result_ptr_val + 4` to the newly allocated memory.
        memcpy(machine->trie_nodes_array_2[i], (char*)(uintptr_t)find_result_ptr_val + 4, 0x400);
    }

    // This loop initializes some entries in the first array pointed to by machine->trie_nodes_array_2[0].
    for (uint i = 0; i < 0x100; ++i) {
        // Accesses an int at `config_ptr + 4 + i * 4`.
        // This means `config_ptr` points to an array of ints, and it checks `config_ptr[1+i]`.
        if (((int*)((char*)config_ptr + 4))[i] == 0) {
            // Assigns the int value of `config_ptr` into the `i`-th element of the array
            // pointed to by `machine->trie_nodes_array_2[0]`.
            ((int*)machine->trie_nodes_array_2[0])[i] = (int)(uintptr_t)config_ptr;
        }
    }
    return 0; // Return success
}

// Function: FreeSearchMachine
// 'machine' is a pointer to the SearchMachineState structure to be freed.
void FreeSearchMachine(SearchMachineState *machine) {
    if (machine != NULL) {
        unsigned int trie_count = GetTrieCount();

        // Free list_heads and associated lists
        if (machine->list_heads != NULL) {
            for (uint i = 0; i < trie_count; ++i) {
                if (machine->list_heads[i] != NULL) {
                    // FreeList expects undefined4 (unsigned int). Cast the pointer value.
                    FreeList((undefined4)(uintptr_t)machine->list_heads[i]);
                    // Mark duplicate pointers as NULL to avoid double freeing.
                    for (uint j = i + 1; j < trie_count; ++j) { // Start from i+1 to avoid self-comparison and redundant checks
                        if (machine->list_heads[j] == machine->list_heads[i]) {
                            machine->list_heads[j] = NULL; // Mark as freed by the current iteration
                        }
                    }
                    machine->list_heads[i] = NULL; // Explicitly nullify after freeing
                }
            }
            free(machine->list_heads);
            machine->list_heads = NULL;
        }

        // Free trie_nodes_array_1
        // As nothing was assigned to individual elements in InitializeSearchMachine,
        // only the base array allocated by xcalloc is freed.
        if (machine->trie_nodes_array_1 != NULL) {
            free(machine->trie_nodes_array_1);
            machine->trie_nodes_array_1 = NULL;
        }

        // Free trie_nodes_array_2 and its contents
        if (machine->trie_nodes_array_2 != NULL) {
            for (uint i = 0; i < trie_count; ++i) {
                if (machine->trie_nodes_array_2[i] != NULL) {
                    free(machine->trie_nodes_array_2[i]); // Free the individual array of ints
                    machine->trie_nodes_array_2[i] = NULL;
                }
            }
            free(machine->trie_nodes_array_2); // Free the array of pointers itself
            machine->trie_nodes_array_2 = NULL;
        }

        free(machine); // Free the SearchMachineState structure itself
    }
    return;
}

// Function: FindMatches
// 'machine' is a pointer to the SearchMachineState structure.
// 'text' is a pointer to the input text (array of bytes).
// 'text_len' is the length of the input text.
// 'out_match_count' is a pointer to store the total number of matches found.
// Returns a pointer to a dynamically allocated array of matches (as an int, cast from uintptr_t).
int FindMatches(SearchMachineState *machine, const byte *text, uint text_len, uint *out_match_count) {
    uint current_match_array_capacity = 0;
    // match_array will store pairs of (match_start_index, list_head_value)
    void* match_array = NULL;
    *out_match_count = 0;

    // current_node_ptr stores an address, which was machine->root_node_id (an int).
    // Cast to void* for pointer arithmetic.
    void* current_node_ptr = (void*)(uintptr_t)machine->root_node_id;

    for (uint i = 0; i < text_len; ++i) {
        // node_data_id is an int read from `current_node_ptr + 0x408`.
        // This implies `current_node_ptr` points to a structure with an int member at offset 0x408.
        int node_data_id = *(int*)((char*)current_node_ptr + 0x408);

        // Loop to traverse the trie based on input character.
        // It continues if the current node's trie_nodes_array_2 entry is NULL,
        // or if there's no transition for the current character `text[i]`.
        while ( (machine->trie_nodes_array_2[node_data_id] == NULL) ||
                (((int*)machine->trie_nodes_array_2[node_data_id])[text[i]] == 0) ) {
            // If no transition, move to the failure/fallback link.
            // current_node_ptr becomes the node pointed to by trie_nodes_array_1[node_data_id].
            current_node_ptr = machine->trie_nodes_array_1[node_data_id];
            // Update node_data_id for the new current_node_ptr.
            node_data_id = *(int*)((char*)current_node_ptr + 0x408);
        }

        // Transition to the next node based on the current character.
        // The value `((int*)machine->trie_nodes_array_2[node_data_id])[text[i]]` is an int (pointer).
        current_node_ptr = (void*)(uintptr_t)((int*)machine->trie_nodes_array_2[node_data_id])[text[i]];

        // Recalculate node_data_id for the *new* current_node_ptr to check for matches.
        node_data_id = *(int*)((char*)current_node_ptr + 0x408);

        // Check if a match is found at the current node (i.e., if it has an associated list head).
        if (machine->list_heads[node_data_id] != NULL) {
            // If the match array is full, reallocate with more capacity.
            if (*out_match_count == current_match_array_capacity) {
                uint new_capacity = current_match_array_capacity * 2 + 1;
                // Check for integer overflow.
                if (new_capacity <= current_match_array_capacity) {
                    fprintf(stderr, DAT_0001703a);
                    _terminate();
                }

                // Allocate new memory for the match array. Each entry is two uints (8 bytes).
                void* new_match_array = xcalloc(new_capacity, sizeof(uint) * 2);

                // Copy existing matches to the new array.
                if (match_array != NULL) {
                    memcpy(new_match_array, match_array, *out_match_count * sizeof(uint) * 2);
                    free(match_array); // Free the old array
                }
                match_array = new_match_array;
                current_match_array_capacity = new_capacity;
            }

            // Store the match: current character index and the value of the list head.
            // Each match entry is a pair of uints: (match_start_index, list_head_value).
            ((uint*)match_array)[*out_match_count * 2] = i;
            ((uint*)match_array)[*out_match_count * 2 + 1] = (uintptr_t)machine->list_heads[node_data_id];
            *out_match_count = *out_match_count + 1;
        }
    }
    return (int)(uintptr_t)match_array; // Return the pointer to the match array as an int.
}