#include <stdbool.h> // For bool type
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For memcmp, memcpy
#include <stdint.h>  // For uint32_t, uint8_t

// --- Minimal List Library Stub (for compilation) ---
// In a real scenario, these would come from a separate header/source file.

typedef struct ListNode {
    void *data;
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

typedef struct List {
    ListNode *head;
    ListNode *tail;
    void (*free_data_fn)(void *); // Function to free data when node is destroyed
} List;

// Global list head instance (statically allocated)
static List stamp_roll_instance;
// Pointer to the global List struct, used by functions
// Initialized to point to the statically allocated instance
static List *stamp_roll = &stamp_roll_instance;

// Function stubs for list operations
void list_init(List *list_instance, void (*free_data_fn)(void *)) {
    // Assumes list_instance points to an already allocated List struct
    if (!list_instance) {
        exit(1); // Should not happen with static allocation
    }
    list_instance->head = NULL;
    list_instance->tail = NULL;
    list_instance->free_data_fn = free_data_fn;
}

void list_insert_at_end(List *list, void *data) {
    if (!list) exit(1);
    ListNode *newNode = (ListNode *)malloc(sizeof(ListNode));
    if (!newNode) {
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    newNode->prev = list->tail;

    if (list->tail) {
        list->tail->next = newNode;
    } else {
        list->head = newNode;
    }
    list->tail = newNode;
}

// Function pointer type for comparison
typedef bool (*list_cmp_func)(void *, void *);

ListNode *list_find_node_with_data(List *list, list_cmp_func cmp_fn, void *data) {
    if (!list) return NULL;
    ListNode *current = list->head;
    while (current) {
        if (cmp_fn(current->data, data)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void list_remove_node(List *list, ListNode *node) {
    if (!list || !node) return;

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }
}

void list_destroy_node(List *list, ListNode *node) {
    if (!list || !node) return;
    if (list->free_data_fn && node->data) {
        list->free_data_fn(node->data);
    }
    free(node);
}

// --- Global variables ---
static unsigned int seed = 0x12345678; // Example initial value
static short seed_idx = 0;
static unsigned char BAD_STAMP[3] = {0xFF, 0xFF, 0xFF}; // Example BAD_STAMP

// Function: stamp_cmp
bool stamp_cmp(void *param_1,void *param_2) {
  return memcmp(param_1,param_2,3) == 0;
}

// Function: init_stamp_roll
void init_stamp_roll(void) {
  list_init(stamp_roll, free);
  return;
}

// Function: get_new_stamp
void * get_new_stamp(void) {
  void *stamp_ptr;
  uint32_t stamp_data_val;
  const size_t STAMP_SIZE = 3; // Stamps are 3 bytes long
  
  stamp_ptr = malloc(STAMP_SIZE);
  if (stamp_ptr == NULL) {
    exit(1); // Terminate on memory allocation failure
  }

  do {
    stamp_data_val = seed + seed_idx;
    // Copy 3 bytes from the generated 4-byte integer to the 3-byte stamp buffer.
    // This typically copies the least significant 3 bytes on a little-endian system.
    memcpy(stamp_ptr, &stamp_data_val, STAMP_SIZE);

    // Update seed_idx using modulo arithmetic to keep it within bounds
    // 0xffd is 4093 in decimal
    seed_idx = (seed_idx + STAMP_SIZE) % 4093;

    // Check if the newly generated stamp matches the BAD_STAMP
  } while (memcmp(stamp_ptr, BAD_STAMP, STAMP_SIZE) == 0);

  list_insert_at_end(stamp_roll, stamp_ptr);
  return stamp_ptr;
}

// Function: use_stamp
uint32_t use_stamp(const void *stamp_data) {
  ListNode *node_ptr;
  
  node_ptr = list_find_node_with_data(stamp_roll, stamp_cmp, stamp_data);
  if (node_ptr == NULL) { // If stamp not found
    return 0xffffffff; // Return -1 (or ULONG_MAX for uint32_t) for failure
  }
  else {
    list_remove_node(stamp_roll, node_ptr);
    list_destroy_node(stamp_roll, node_ptr); // This frees the node and its data (the stamp_ptr)
    return 0; // Return 0 for success
  }
}