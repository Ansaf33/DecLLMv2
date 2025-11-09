#include <stdlib.h>   // For malloc, free
#include <string.h>   // For strcmp
#include <stdint.h>   // For uint32_t, intptr_t, uintptr_t

// Fix missing types
typedef unsigned int uint;
typedef uint32_t undefined4; // Assuming undefined4 is a 4-byte unsigned integer

// Forward declaration for movie_find, as it's used by movie_add
undefined4 * movie_find(undefined4 *param_1, char *param_2);

// Function: movie_g2s
char * movie_g2s(uint param_1) {
  if (param_1 == 4) return "Horror";
  if (param_1 == 3) return "Comedy";
  if (param_1 == 1) return "Action";
  if (param_1 == 2) return "Romance";
  return "Other";
}

// Function: movie_r2s
char * movie_r2s(uint param_1) {
  if (param_1 == 4) return "R";
  if (param_1 == 3) return "PG13";
  if (param_1 == 1) return "G";
  if (param_1 == 2) return "PG";
  return "Unknown";
}

// Function: movie_add
// param_1: pointer to an integer that holds the head node's address. `*param_1` is essentially a `MovieNode*`
// param_2: pointer to an `undefined4`. Based on usage in `movie_find`, this is intended to be a `char*` (the title string).
// We'll cast `param_2` to `char*` internally to resolve the type contradiction in the original snippet.
undefined4 movie_add(int *param_1, undefined4 *param_2_as_title_ptr) {
  if (param_1 == NULL) return 0xffffffff;

  // Allocate space for two undefined4s (one for title pointer, one for next pointer)
  undefined4 *newNode = (undefined4 *)malloc(sizeof(undefined4) * 2);
  if (newNode == NULL) return 0xffffffff;

  // Reconcile the type of param_2 with its usage in movie_find
  char *title_string = (char *)(uintptr_t)param_2_as_title_ptr;

  newNode[0] = (undefined4)(uintptr_t)title_string; // Store the title string pointer
  newNode[1] = 0; // Set next pointer to NULL

  // Check if list is empty
  if (*param_1 == 0) { // *param_1 holds the head node's address
    *param_1 = (int)(uintptr_t)newNode; // Store newNode's address in *param_1
    return 0;
  } else {
    // Check for duplicates
    // movie_find expects `undefined4 *param_1` (head node) and `char *param_2` (title string)
    undefined4 *head_node = (undefined4 *)(uintptr_t)*param_1;
    undefined4 *foundNode = movie_find(head_node, title_string);
    if (foundNode != NULL) { // Movie already exists
      free(newNode); // Don't add duplicate, free allocated node
      return 0xffffffff; // Error for duplicate
    } else {
      // Traverse to the end of the list
      undefined4 *current = head_node;
      while ((undefined4 *)(uintptr_t)current[1] != NULL) { // current[1] is the next pointer
        current = (undefined4 *)(uintptr_t)current[1];
      }
      current[1] = (undefined4)(uintptr_t)newNode; // Link new node
      return 0;
    }
  }
}

// Function: free_movie
// The original logic of this function is problematic for a linked list.
// It frees `*param_1`, then `param_1[1]`, then `param_1`. This would free node data and the next node pointer.
// Given that `movie_delete` directly calls `free()` on the node, this function
// seems to be for a different data structure or is logically flawed for a linked list.
// We keep its original logic, replacing `0x0` with `NULL`.
void free_movie(void **param_1) {
  if (param_1 != NULL) {
    if (*param_1 != NULL) {
      free(*param_1);
    }
    if (param_1[1] != NULL) {
      free(param_1[1]);
    }
    free(param_1);
  }
}

// Function: movie_delete
// param_1: pointer to a `void*` that holds the head node's address. `*param_1` is essentially a `MovieNode*`
// param_2: 1-based index of the node to delete
undefined4 movie_delete(void **param_1, int param_2) {
  if (param_1 == NULL) return 0xffffffff;

  // `*param_1` is the head node (MovieNode*). Cast to undefined4* for consistent pointer arithmetic.
  undefined4 *head = (undefined4 *)(uintptr_t)*param_1;

  if (head == NULL || param_2 < 1) return 0xffffffff; // List empty or invalid index

  if (param_2 == 1) { // Delete the head node
    *param_1 = (void *)(uintptr_t)head[1]; // Update head to point to the next node
    free(head); // Free the original head node
    return 0;
  } else {
    undefined4 *current = head;
    int current_index = 1;
    while ((undefined4 *)(uintptr_t)current[1] != NULL) { // Iterate through next pointers
      current_index++;
      if (param_2 == current_index) {
        undefined4 *node_to_delete = (undefined4 *)(uintptr_t)current[1];
        current[1] = node_to_delete[1]; // Link previous node to the node after the one being deleted
        free(node_to_delete);
        return 0;
      }
      current = (undefined4 *)(uintptr_t)current[1];
    }
  }
  return 0xffffffff; // Node not found at index
}

// Function: movie_update
undefined4 movie_update(void) {
  return 0;
}

// Function: movie_find
// param_1: head of the list (MovieNode* cast to undefined4*)
// param_2: title string to find
undefined4 * movie_find(undefined4 *param_1, char *param_2) {
  if (param_1 != NULL) {
    undefined4 *current = param_1;
    while (current != NULL) {
      // current[0] holds the address of the title string
      if (strcmp(*(char **)(uintptr_t)current[0], param_2) == 0) {
        return current;
      }
      current = (undefined4 *)(uintptr_t)current[1]; // current[1] is the next pointer
    }
  }
  return NULL;
}

// Function: movie_find_by_id
// param_1: head of the list (MovieNode* address cast to int)
// param_2: 1-based index to find
int movie_find_by_id(int param_1, int param_2) {
  if (param_1 != 0 && param_2 >= 1) {
    int current_index = 0;
    int current_node_addr = param_1;
    while (current_node_addr != 0) {
      current_index++;
      if (param_2 == current_index) {
        return current_node_addr;
      }
      // The next pointer is at an offset of 4 bytes from the start of the node
      current_node_addr = *(int *)(uintptr_t)(current_node_addr + 4);
    }
  }
  return 0;
}