#include <stdio.h>   // For fprintf, stderr, perror
#include <stdlib.h>  // For malloc, free, exit, EXIT_FAILURE
#include <string.h>  // For memcmp, memcpy
#include <stdbool.h> // For bool type
#include <stdint.h>  // For uint32_t

// --- Dummy List Implementation (to make the provided functions compilable) ---
// In a real project, these would be in a separate header/source file.

typedef void (*free_func_t)(void *);

typedef struct list_node {
    void *data;
    struct list_node *next;
} list_node_t;

typedef struct list {
    list_node_t *head;
    list_node_t *tail;
    free_func_t free_data;
} list_t;

// Global list instance, initialized by init_stamp_roll
static list_t *stamp_roll = NULL;

// Function to initialize the list
void list_init(list_t **list_ptr, free_func_t free_func) {
    if (list_ptr == NULL) {
        return;
    }
    *list_ptr = (list_t *)malloc(sizeof(list_t));
    if (*list_ptr == NULL) {
        perror("Failed to allocate list");
        exit(EXIT_FAILURE);
    }
    (*list_ptr)->head = NULL;
    (*list_ptr)->tail = NULL;
    (*list_ptr)->free_data = free_func;
}

// Function to insert data at the end of the list
void list_insert_at_end(list_t *list, void *data) {
    if (list == NULL) {
        return;
    }
    list_node_t *new_node = (list_node_t *)malloc(sizeof(list_node_t));
    if (new_node == NULL) {
        perror("Failed to allocate list node");
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    new_node->next = NULL;

    if (list->tail == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }
}

// Function to find a node with specific data using a comparison function
list_node_t* list_find_node_with_data(list_t *list, bool (*cmp)(void*, void*), void *data) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    list_node_t *current = list->head;
    while (current != NULL) {
        if (cmp(current->data, data)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function to remove a specific node from the list
void list_remove_node(list_t *list, list_node_t *node_to_remove) {
    if (list == NULL || node_to_remove == NULL) {
        return;
    }

    if (list->head == node_to_remove) {
        list->head = node_to_remove->next;
        if (list->tail == node_to_remove) {
            list->tail = NULL;
        }
        return;
    }

    list_node_t *current = list->head;
    while (current != NULL && current->next != node_to_remove) {
        current = current->next;
    }

    if (current != NULL) { // current->next is node_to_remove
        current->next = node_to_remove->next;
        if (list->tail == node_to_remove) {
            list->tail = current;
        }
    }
}

// Function to destroy a list node (and its data if a free_func is provided)
void list_destroy_node(list_t *list, list_node_t *node) {
    if (list == NULL || node == NULL) {
        return;
    }
    if (list->free_data && node->data) {
        list->free_data(node->data);
    }
    free(node);
}

// --- Global variables from the original snippet's context ---
static uint32_t seed = 0x12345678; // Example initial value for seed
static short seed_idx = 0;
static const unsigned char BAD_STAMP[3] = {0x00, 0x00, 0x00}; // Example bad stamp value
static const short MOD_VAL_FOR_SEED_IDX = 0xffd; // 4093, used for seed_idx modulo

// --- Original _terminate function (adapted for standard C) ---
void _terminate(void) {
    fprintf(stderr, "Fatal error: _terminate called.\n");
    exit(EXIT_FAILURE);
}

// --- Original functions (fixed and refactored) ---

// Function: stamp_cmp
bool stamp_cmp(void *param_1, void *param_2) {
  return memcmp(param_1, param_2, 3) == 0;
}

// Function: init_stamp_roll
void init_stamp_roll(void) {
  list_init(&stamp_roll, free); // Pass address of the global list_t pointer
}

// Function: get_new_stamp
void * get_new_stamp(void) {
  void *new_stamp_data;
  
  new_stamp_data = malloc(3);
  if (new_stamp_data == NULL) {
    _terminate(); // Call the defined _terminate function
  }

  uint32_t current_seed_val; // Temporary variable to hold the seed value for memcpy

  do {
    current_seed_val = seed + seed_idx;
    // Copy the first 3 bytes of current_seed_val into the allocated memory
    memcpy(new_stamp_data, &current_seed_val, 3);
    
    // Update seed_idx using modulo arithmetic
    seed_idx = (short)((seed_idx + 3) % MOD_VAL_FOR_SEED_IDX);
    
    // Check if the newly generated stamp matches BAD_STAMP
  } while (memcmp(new_stamp_data, BAD_STAMP, 3) == 0); // Loop until a unique stamp is generated
  
  list_insert_at_end(stamp_roll, new_stamp_data);
  return new_stamp_data;
}

// Function: use_stamp
int use_stamp(void *param_1) {
  list_node_t *found_node;
  
  found_node = list_find_node_with_data(stamp_roll, stamp_cmp, param_1);
  if (found_node == NULL) {
    return -1; // Indicate failure (stamp not found)
  } else {
    list_remove_node(stamp_roll, found_node);
    list_destroy_node(stamp_roll, found_node); // Free the node and its data
    return 0; // Indicate success
  }
}