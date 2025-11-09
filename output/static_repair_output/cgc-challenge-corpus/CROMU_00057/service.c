#include <stdio.h>    // For printf
#include <stdlib.h>   // For calloc, exit
#include <string.h>   // For strlen, strncpy
#include <stdint.h>   // For uintptr_t (used for pointer value manipulation)

// ITEM_DATA_SIZE represents the fixed 24-byte size (0x18) allocated for an item.
// This size, combined with the decompiled code's access patterns using `undefined4` for pointers,
// strongly suggests the original code was compiled for a 32-bit architecture where pointers are 4 bytes.
// To make this code compilable and functional on a 64-bit Linux system while preserving the original
// memory layout and access logic, we manually manage memory as a char array and use `uintptr_t`
// to store and retrieve pointer values, effectively treating them as 32-bit values within the 24-byte block.
#define ITEM_DATA_SIZE 24

// Helper macros for accessing fields within an ITEM_DATA_SIZE char array.
// `item_ptr` is expected to be a `char*` pointing to the start of an item's data block.
// Each macro performs the necessary type casting and offset calculation.
// Pointers (like `name`, `sub_list_head`, `next`) are stored as `unsigned int` (4 bytes)
// to match the 32-bit assumption, then cast to `char*` (8 bytes on 64-bit) for use.
#define GET_ITEM_ID(item_ptr)           (*(short*)(item_ptr + 0))
#define SET_ITEM_ID(item_ptr, val)      (*(short*)(item_ptr + 0) = (val))

#define GET_ITEM_NAME_PTR(item_ptr)     ((char*)(uintptr_t)(*(unsigned int*)(item_ptr + 4)))
#define SET_ITEM_NAME_PTR(item_ptr, val) (*(unsigned int*)(item_ptr + 4) = (unsigned int)(uintptr_t)(val))

#define GET_ITEM_F4(item_ptr)           (*(short*)(item_ptr + 8))
#define SET_ITEM_F4(item_ptr, val)      (*(short*)(item_ptr + 8) = (val))

#define GET_ITEM_F5(item_ptr)           (*(short*)(item_ptr + 10))
#define SET_ITEM_F5(item_ptr, val)      (*(short*)(item_ptr + 10) = (val))

#define GET_ITEM_F6(item_ptr)           (*(short*)(item_ptr + 12))
#define SET_ITEM_F6(item_ptr, val)      (*(short*)(item_ptr + 12) = (val))

#define GET_ITEM_SUB_LIST_HEAD(item_ptr) ((char*)(uintptr_t)(*(unsigned int*)(item_ptr + 16)))
#define SET_ITEM_SUB_LIST_HEAD(item_ptr, val) (*(unsigned int*)(item_ptr + 16) = (unsigned int)(uintptr_t)(val))

#define GET_ITEM_NEXT(item_ptr)         ((char*)(uintptr_t)(*(unsigned int*)(item_ptr + 20)))
#define SET_ITEM_NEXT(item_ptr, val)    (*(unsigned int*)(item_ptr + 20) = (unsigned int)(uintptr_t)(val))


// --- Mock/Stub functions ---
// These functions are not provided in the snippet but are called by `main`.
// Their signatures and basic functionality are inferred from `main`'s usage.

// `calc_version` is expected to return a pointer to the head of a linked list of items.
static char* calc_version(void) {
    // These static char arrays represent ITEM_DATA_SIZE blocks of memory for items.
    static char item_data_1[ITEM_DATA_SIZE];
    static char item_data_2[ITEM_DATA_SIZE];

    // Initialize item_data_1
    SET_ITEM_ID(item_data_1, 1);
    char *name1_str = strdup("Version One");
    SET_ITEM_NAME_PTR(item_data_1, name1_str);
    SET_ITEM_F4(item_data_1, 0);
    SET_ITEM_F5(item_data_1, 0);
    SET_ITEM_F6(item_data_1, 0);
    SET_ITEM_SUB_LIST_HEAD(item_data_1, NULL);
    SET_ITEM_NEXT(item_data_1, item_data_2); // Link to the next item

    // Initialize item_data_2
    SET_ITEM_ID(item_data_2, 2);
    char *name2_str = strdup("Version Two");
    SET_ITEM_NAME_PTR(item_data_2, name2_str);
    SET_ITEM_F4(item_data_2, 0);
    SET_ITEM_F5(item_data_2, 0);
    SET_ITEM_F6(item_data_2, 0);
    SET_ITEM_SUB_LIST_HEAD(item_data_2, NULL);
    SET_ITEM_NEXT(item_data_2, NULL); // End of list

    return item_data_1; // Return the head of the dummy list
}

// `obf_strings` returns a string used as a format for `printf`.
static const char* obf_strings(void) {
    return "Application started.\n";
}

// `receive_commands` is called without arguments and returns void.
static void receive_commands(void) {
    // Placeholder for functionality.
}

// `destroy_database` is called without arguments and returns void.
static void destroy_database(void) {
    // Placeholder for functionality.
}

// `create_sprint` is assigned to a local variable `pcStack_34` but not directly called from `main`.
// It's likely a decompiler artifact or part of a larger, unshown control flow.
static void create_sprint(void) {
    // Placeholder for functionality.
}

// `_terminate` is called at various points and seems to be an exit function.
static void _terminate(void) {
    exit(0); // Terminate the program.
}

// Mock input data that `psStack_28` points to in the original decompiled code.
// This structure maps the byte offsets used by `psStack_28` in `main`.
static struct {
    short search_key;
    short new_item_id;
    char *new_item_name;
} mock_input_data = {
    .search_key = 1, // Value to search for in the item list (corresponds to `*psStack_28`)
    .new_item_id = 99, // ID for the new item to be created (corresponds to `psStack_28[1]`)
    .new_item_name = "New Item Name String" // Name for the new item (corresponds to `*(char**)((char*)psStack_28 + 4)`)
};

// Dummy for `_GLOBAL_OFFSET_TABLE_`, which is a linker concept, not a C variable.
static void *_GLOBAL_OFFSET_TABLE_ = NULL;

// Function: main
int main(void) {
  int result = 0; // Corresponds to `uVar1`
  char *current_item_list_ptr; // Corresponds to `psStack_2c`
  char *new_item_ptr;         // Corresponds to `psStack_44`
  
  // `psStack_28` is a `short*` in the original, pointing to our `mock_input_data`.
  // We explicitly cast to `short*` to match the original's type usage for indexing.
  short *psStack_28_mock_ptr = (short*)&mock_input_data;

  // Decompiler artifacts (program counter assignments) are removed.
  current_item_list_ptr = calc_version();
  printf(obf_strings());
  receive_commands();
  destroy_database();
  // `pcStack_34 = create_sprint; _terminate();`
  // The unconditional `_terminate()` here would make the rest of main unreachable.
  // Assuming this was a conditional jump or part of a different execution path in the original program,
  // we comment it out to allow the subsequent code to execute for demonstration.
  // _terminate(); 

  // Loop through the list of items to find one matching `search_key`.
  // Original: for (; (psStack_2c != (short *)0x0 && (*psStack_2c != *psStack_28)); psStack_2c = *(short **)(psStack_2c + 8))
  for (; (current_item_list_ptr != NULL && (GET_ITEM_ID(current_item_list_ptr) != psStack_28_mock_ptr[0]));
      current_item_list_ptr = GET_ITEM_NEXT(current_item_list_ptr)) {
  }

  if (current_item_list_ptr == NULL) {
    result = -1; // Item not found, return error.
  }
  else {
    // Decompiler artifacts (`ppuStack_38`, `pcStack_34` assignments) are removed.

    // Allocate memory for a new item, using the fixed ITEM_DATA_SIZE (24 bytes).
    // Original: psStack_44 = (short *)calloc(0x18,in_stack_ffffffa8);
    // `in_stack_ffffffa8` is uninitialized in the snippet; assuming `1` for `calloc`'s element count.
    new_item_ptr = (char *)calloc(1, ITEM_DATA_SIZE);
    if (new_item_ptr == NULL) {
      _terminate(); // Exit if allocation fails.
    }

    // Set the ID of the new item.
    // Original: *psStack_44 = psStack_28[1];
    SET_ITEM_ID(new_item_ptr, psStack_28_mock_ptr[1]);

    // Get the source name string from `mock_input_data`.
    char *source_name_str = mock_input_data.new_item_name;
    size_t name_len = strlen(source_name_str);
    size_t name_buffer_size = name_len + 1; // +1 for null terminator

    // Allocate memory for the name string.
    // Original: pvVar3 = calloc(*(size_t *)(puVar4 + -0x10),*(size_t *)(puVar4 + -0xc));
    // This translates to `calloc(name_buffer_size, 1)`.
    char *name_buffer = (char *)calloc(name_buffer_size, 1);
    
    // Store the pointer to the newly allocated name buffer in the new item's data block.
    // Original: *(void **)(psStack_44 + 2) = pvVar3;
    SET_ITEM_NAME_PTR(new_item_ptr, name_buffer);

    // Check if name buffer allocation failed.
    // Original: if (*(int *)(psStack_44 + 2) == 0)
    if (GET_ITEM_NAME_PTR(new_item_ptr) == NULL) {
      _terminate(); // Exit if allocation fails.
    }

    // Copy the name string into the allocated buffer.
    // Original: strncpy(*(char **)(puVar5 + -0x10),*(char **)(puVar5 + -0xc),*(size_t *)(puVar5 + -8));
    // This translates to `strncpy(destination, source, length)`.
    strncpy(GET_ITEM_NAME_PTR(new_item_ptr), source_name_str, name_len);
    GET_ITEM_NAME_PTR(new_item_ptr)[name_len] = '\0'; // Ensure null termination

    // Initialize fields f4, f5, f6 to 0.
    // Original: psStack_44[4] = 0; psStack_44[5] = 0; psStack_44[6] = 0;
    SET_ITEM_F4(new_item_ptr, 0);
    SET_ITEM_F5(new_item_ptr, 0);
    SET_ITEM_F6(new_item_ptr, 0);

    // Initialize `sub_list_head` and `next` pointers of the new item to NULL.
    // Original: *(undefined4 *)(psStack_44 + 8) = 0; *(undefined4 *)(psStack_44 + 10) = 0;
    SET_ITEM_SUB_LIST_HEAD(new_item_ptr, NULL);
    SET_ITEM_NEXT(new_item_ptr, NULL);

    // Add the new item to the sub-list of the found item.
    // Original: if (*(int *)(psStack_2c + 6) == 0)
    if (GET_ITEM_SUB_LIST_HEAD(current_item_list_ptr) == NULL) {
      // If the sub-list is empty, make the new item its head.
      // Original: *(short **)(psStack_2c + 6) = psStack_44;
      SET_ITEM_SUB_LIST_HEAD(current_item_list_ptr, new_item_ptr);
      result = 0;
    }
    else {
      // Traverse the sub-list to find its end.
      // Original: for (iStack_40 = *(int *)(psStack_2c + 6); *(int *)(iStack_40 + 0x14) != 0; iStack_40 = *(int *)(iStack_40 + 0x14))
      char *sub_list_current_ptr = GET_ITEM_SUB_LIST_HEAD(current_item_list_ptr);
      for (; GET_ITEM_NEXT(sub_list_current_ptr) != NULL;
          sub_list_current_ptr = GET_ITEM_NEXT(sub_list_current_ptr)) {
      }
      // Add the new item to the end of the sub-list.
      // Original: *(short **)(iStack_40 + 0x14) = psStack_44;
      SET_ITEM_NEXT(sub_list_current_ptr, new_item_ptr);
      result = 0;
    }
  }
  return result;
}