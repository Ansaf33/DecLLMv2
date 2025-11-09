#include <stdio.h>   // For printf
#include <string.h>  // For strcmp
#include <stdlib.h>  // For malloc, free, exit, NULL

// --- Mock/Placeholder declarations for external functions and structures ---

// Structure representing the item being searched for.
// The original `param_3` was `int param_3`, and `*(undefined4 *)(param_3 + 4)`
// was used to access a character pointer. This implies `param_3` points to a structure
// where a `const char *name` field is located at an offset of 4 bytes.
typedef struct Item {
    char _padding[4]; // Padding to align `name` at offset 4
    const char *name;
} Item;

// Mock function: make_key_from_name
// Takes a string (name) and returns an unsigned integer key.
unsigned int make_key_from_name(const char *name) {
    unsigned int key = 0;
    if (name == NULL) return 0;
    while (*name) {
        key = (key << 5) + *name++; // Simple hash
    }
    return key;
}

// Global storage for mock nodes to allow cleanup in main
static char *mock_node_blocks[10]; // Max 10 mock nodes
static int mock_node_count = 0;

// Mock function: find_node_by_key
// Simulates finding the first node matching a key and returning its name string pointer.
// It also sets up a linked list where the pointer to the *next* name string
// is located at offset 0x400 from the *current* name string's address.
// This is a highly specific memory layout derived from the Ghidra decompiler output.
void *find_node_by_key(void *tree_root, unsigned int key) {
    // Mock data for demonstration
    static const char *mock_names[] = {
        "apple", "banana", "orange", "apple_match_2", "grape", "apple_match_3", NULL
    };
    static int initialized = 0;

    if (!initialized) {
        // Initialize mock nodes on first call
        for (int i = 0; mock_names[i] != NULL; ++i) {
            // Allocate a block of memory for the "node".
            // It must be large enough to hold the name string and the next pointer at 0x400 offset.
            char *node_block = (char *)malloc(0x400 + sizeof(char *));
            if (!node_block) {
                perror("malloc failed in mock_find_node_by_key");
                exit(EXIT_FAILURE);
            }
            mock_node_blocks[mock_node_count++] = node_block;

            // Copy the name string to the beginning of the allocated block.
            strncpy(node_block, mock_names[i], 0x400 - 1);
            node_block[0x400 - 1] = '\0'; // Ensure null termination

            // Get the address where the pointer to the next name string will be stored.
            char **next_ptr_location = (char **)(node_block + 0x400);

            // Link the nodes: the 'next' pointer of the previous node should point to the current node's name string.
            if (i > 0) {
                char *prev_node_block = mock_node_blocks[i-1];
                char **prev_next_ptr_location = (char **)(prev_node_block + 0x400);
                *prev_next_ptr_location = node_block; // Link previous node's next to current node's name string
            }
        }
        // The last node's 'next' pointer should be NULL.
        if (mock_node_count > 0) {
            char *last_node_block = mock_node_blocks[mock_node_count - 1];
            char **last_next_ptr_location = (char **)(last_node_block + 0x400);
            *last_next_ptr_location = NULL;
        }
        initialized = 1;
    }

    // Simple mock logic: Return the first "name string" whose key matches.
    // In a real system, this would search a data structure (like a tree or hash table).
    if (key == make_key_from_name("apple")) {
        return mock_node_blocks[0]; // Return pointer to "apple" string
    } else if (key == make_key_from_name("banana")) {
        return mock_node_blocks[1]; // Return pointer to "banana" string
    } else if (key == make_key_from_name("orange")) {
        return mock_node_blocks[2]; // Return pointer to "orange" string
    }
    return NULL; // No match found
}

// --- Fixed find_matches function ---

// Function: find_matches
// Searches for a matching item in a tree/list structure.
// param_1: A pointer to the root of the data structure (e.g., a tree or list).
// item_to_find: A pointer to an Item structure containing the name to search for.
void find_matches(void *tree_root, const Item *item_to_find) {
    // `current_match_name` will hold a `char*` to the name string of the current node.
    // The `find_node_by_key` function returns the `char*` to the first matching name.
    // `make_key_from_name` is called directly within the `find_node_by_key` arguments,
    // reducing the need for an intermediate `uVar1`.
    char *current_match_name = (char *)find_node_by_key(
        tree_root,
        make_key_from_name(item_to_find->name)
    );

    // Loop through the linked list of matching names.
    // The loop continues as long as `current_match_name` is not NULL.
    // In each iteration, `current_match_name` is updated to point to the next name string
    // by dereferencing the `char**` located at `current_match_name + 0x400`.
    for (; current_match_name != NULL; current_match_name = *(char **)(current_match_name + 0x400)) {
        // Compare the name from `item_to_find` with the `current_match_name`.
        // The result of `strcmp` is used directly in the `if` condition,
        // reducing the need for an intermediate `iVar2`.
        if (strcmp(item_to_find->name, current_match_name) == 0) {
            // If an exact match is found, print the name and immediately return.
            // `@s` is replaced with standard C `%s`.
            printf("%s\n", current_match_name);
            return;
        }
    }
    return;
}

// --- Main function for compilation and testing ---

int main() {
    // Create a dummy tree root (can be NULL for this mock implementation)
    void *my_tree_root = NULL;

    // Create some test items
    Item item1 = {{0}, "apple"};
    Item item2 = {{0}, "banana"};
    Item item3 = {{0}, "nonexistent"};
    Item item4 = {{0}, "apple_match_2"}; // This should not be found if "apple" is found first
    Item item_orange = {{0}, "orange"};

    printf("Searching for 'apple':\n");
    find_matches(my_tree_root, &item1); // Should print "apple"

    printf("\nSearching for 'banana':\n");
    find_matches(my_tree_root, &item2); // Should print "banana"

    printf("\nSearching for 'nonexistent':\n");
    find_matches(my_tree_root, &item3); // Should print nothing

    printf("\nSearching for 'apple_match_2' (should not be found if 'apple' is found first due to `return`):\n");
    find_matches(my_tree_root, &item4); // Should print nothing, as "apple" is found first and returns.

    printf("\nSearching for 'orange':\n");
    find_matches(my_tree_root, &item_orange); // Should print "orange"

    // Clean up memory allocated by mock_find_node_by_key
    for (int i = 0; i < mock_node_count; ++i) {
        if (mock_node_blocks[i] != NULL) {
            free(mock_node_blocks[i]);
        }
    }

    return 0;
}