#include <stdlib.h> // For calloc, free, exit
#include <stdint.h> // For uint32_t, uint16_t, uint8_t, uintptr_t
#include <stdio.h>  // For NULL

// --- Global Variables ---
uint16_t fp_idx = 0;
uintptr_t fp = 0; // Assuming 'fp' is a base address for an array of bytes

// --- Placeholder for original _terminate function ---
void _terminate(int status) {
    // In a real application, this might log an error and exit.
    // For compilation, we just exit.
    exit(EXIT_FAILURE);
}

// --- Forward Declarations for Function Pointers ---
typedef void (*code_ptr_t)();
typedef void (*code_ptr_with_args_t)(void*, void*);

// --- Structure Definitions ---

// OptionNode: Represents an item in a linked list of options.
// Structure: [data1, data2, next_ptr] - 12 bytes
typedef struct OptionNode {
    uint32_t data1;
    uint32_t data2;
    struct OptionNode *next;
} OptionNode;

// RiderNode: Represents an item in a linked list of riders.
// Structure: [data[0], data[1], next_ptr, ..., func_ptr_0x1c] - 32 bytes (0x20)
// The 'next' pointer is assumed to be at offset 8 (index 2 if viewed as uint32_t array).
// The 'func_ptr_0x1c' is assumed to be at offset 0x1c (index 7 if viewed as uint32_t array).
typedef struct RiderNode {
    uint32_t data[2]; // data[0] could be 'data1', data[1] could be 'data2'
    struct RiderNode *next; // next pointer (offset 8)
    uint32_t _padding[4]; // Padding to reach offset 0x1c (4 * 4 = 16 bytes)
    code_ptr_t func_ptr_0x1c; // Function pointer (offset 0x1c)
} RiderNode;

// ConditionNode: Used in option type 1 for comparison and action.
// Structure: [data1, data2, ..., func_ptr_0x1c] - 32 bytes (0x20)
// 'data2' is at offset 4. 'func_ptr_0x1c' is at offset 0x1c.
typedef struct ConditionNode {
    uint32_t data1; // offset 0x0
    uint32_t data2; // offset 0x4
    uint32_t _padding[5]; // Padding to reach offset 0x1c (5 * 4 = 20 bytes)
    code_ptr_with_args_t func_ptr_0x1c; // Function pointer (offset 0x1c)
} ConditionNode;

// DeciderState: Main state structure for the decider logic.
// Structure: [id, type, active_riders_head, retired_riders_head, option_count, option_list_head, embark_fn, disembark_fn] - 32 bytes (0x20)
typedef struct DeciderState {
    uint32_t id; // offset 0x0
    uint32_t type; // offset 0x4
    void *active_riders_head; // offset 0x8 - head of a generic list of riders
    void *retired_riders_head; // offset 0xc - head of a generic list of riders
    uint32_t option_count; // offset 0x10
    OptionNode *option_list_head; // offset 0x14 - head of options list
    // Function pointers for embark/disembark operations
    void (*embark_fn)(struct DeciderState*, void*); // offset 0x18
    int (*disembark_fn)(struct DeciderState*); // offset 0x1c
} DeciderState;


// --- Function Prototypes ---
void option_new(void **output_option_ptr, uint32_t data1, uint32_t data2);
void option_append_new(OptionNode **head_ptr_addr, OptionNode *new_option_ptr);
OptionNode* option_pop(OptionNode **head_ptr_addr);
void option_destroy_single(void **option_ptr_addr);
void option_destroy_list(OptionNode **head_ptr_addr);

uint16_t get_next_fp_idx(void);

OptionNode* decider_select_option(DeciderState *decider);

void decider_add_option(DeciderState *decider, uint32_t data1, uint32_t data2);

// Generic list management functions for riders, assuming 'next' pointer is at offset 8
void rider_append(void **list_head_ptr_addr, void *new_item_ptr);
void* rider_pop(void **list_head_ptr_addr);
void lift_enqueue_riders(uint32_t data_value, void **rider_ptr_addr);

void decider_embark(DeciderState *decider, void *new_rider);
int decider_disembark(DeciderState *decider);
uint32_t decider_new(DeciderState **output_decider_ptr, uint32_t *init_params);
void decider_reset(DeciderState *decider, void **destination_list_head_ptr_addr);
void decider_destroy(DeciderState **decider_ptr_addr, void **cleanup_list_head_ptr_addr);


// --- Function Implementations ---

// Function: option_new
void option_new(void **output_option_ptr, uint32_t data1, uint32_t data2) {
    OptionNode *new_option = (OptionNode *)calloc(1, sizeof(OptionNode));
    if (new_option == NULL) {
        _terminate(0);
    }
    new_option->data1 = data1;
    new_option->data2 = data2;
    new_option->next = NULL;
    *output_option_ptr = new_option;
}

// Function: option_append_new
void option_append_new(OptionNode **head_ptr_addr, OptionNode *new_option_ptr) {
    if (*head_ptr_addr == NULL) {
        *head_ptr_addr = new_option_ptr;
    } else {
        OptionNode *current_node = *head_ptr_addr;
        while (current_node->next != NULL) {
            current_node = current_node->next;
        }
        current_node->next = new_option_ptr;
    }
}

// Function: option_pop
OptionNode* option_pop(OptionNode **head_ptr_addr) {
    if (*head_ptr_addr == NULL) {
        return NULL;
    } else {
        OptionNode *popped_option = *head_ptr_addr;
        *head_ptr_addr = popped_option->next;
        popped_option->next = NULL;
        return popped_option;
    }
}

// Function: option_destroy_single
void option_destroy_single(void **option_ptr_addr) {
    free(*option_ptr_addr);
    *option_ptr_addr = NULL;
}

// Function: option_destroy_list
void option_destroy_list(OptionNode **head_ptr_addr) {
    OptionNode *current_option;
    while (*head_ptr_addr != NULL) {
        current_option = option_pop(head_ptr_addr);
        void *temp_ptr = current_option; // current_option is already a pointer
        option_destroy_single(&temp_ptr); // Pass address of the void* temp_ptr
    }
}

// Function: get_next_fp_idx
uint16_t get_next_fp_idx(void) {
    uint16_t current_idx = fp_idx;
    fp_idx = (current_idx + 1) % 0x1000;
    return current_idx;
}

// Function: decider_select_option
OptionNode* decider_select_option(DeciderState *decider) {
    uint16_t fp_index = get_next_fp_idx();
    OptionNode *selected_option = decider->option_list_head;

    if (decider->option_count == 0 || selected_option == NULL) {
        return NULL; // No options to select from
    }

    // Calculate the number of steps to advance based on fp_index
    // Access byte at (fp + fp_index) and take modulo option_count
    uint8_t num_steps = ((uint8_t*)fp)[fp_index] % decider->option_count;

    for (uint8_t i = 0; i < num_steps; ++i) {
        if (selected_option->next == NULL) {
            break; // Reached end of options list
        }
        selected_option = selected_option->next;
    }
    return selected_option;
}

// Function: decider_add_option
void decider_add_option(DeciderState *decider, uint32_t data1, uint32_t data2) {
    OptionNode *new_option_ptr;
    option_new((void**)&new_option_ptr, data1, data2);
    option_append_new(&(decider->option_list_head), new_option_ptr);
    decider->option_count++;
}

// Rider list management functions - assumed to handle generic lists where 'next' is at offset 8
// These functions assume a generic list node structure where the 'next' pointer is at offset 8
// (i.e., the third element if viewed as an array of 4-byte pointers/integers).
// This is a common pattern in decompiled code for generic list operations.
void rider_append(void **list_head_ptr_addr, void *new_item_ptr) {
    if (*list_head_ptr_addr == NULL) {
        *list_head_ptr_addr = new_item_ptr;
    } else {
        void *current_node = *list_head_ptr_addr;
        while (((void**)current_node)[2] != NULL) {
            current_node = ((void**)current_node)[2];
        }
        ((void**)current_node)[2] = new_item_ptr;
    }
}

void* rider_pop(void **list_head_ptr_addr) {
    if (*list_head_ptr_addr == NULL) {
        return NULL;
    } else {
        void *popped_item = *list_head_ptr_addr;
        *list_head_ptr_addr = ((void**)popped_item)[2];
        ((void**)popped_item)[2] = NULL;
        return popped_item;
    }
}

// Placeholder for lift_enqueue_riders
void lift_enqueue_riders(uint32_t data_value, void **rider_ptr_addr) {
    // Dummy implementation: assume it consumes and frees the rider
    free(*rider_ptr_addr);
    *rider_ptr_addr = NULL;
}

// Function: decider_embark
void decider_embark(DeciderState *decider, void *new_rider) {
    rider_append(&(decider->active_riders_head), new_rider);
}

// Function: decider_disembark
int decider_disembark(DeciderState *decider) {
    int processed_count = 0;
    if (decider->option_count == 0) {
        return 0; // No options, cannot disembark
    }

    void *current_rider;
    OptionNode *selected_option;

    while ((current_rider = rider_pop(&(decider->active_riders_head))) != NULL) {
        selected_option = decider_select_option(decider);

        if (selected_option == NULL) {
            // Should not happen if option_count > 0, but safety check.
            // If no option selected, put rider back and stop.
            rider_append(&(decider->active_riders_head), current_rider);
            return processed_count;
        }

        if (selected_option->data1 == 0) { // Option type 0
            // Check rider's 'next' field (at offset 8)
            if (((uint32_t*)current_rider)[2] == 0) {
                // Call rider's function at offset 0x1c
                ((RiderNode*)current_rider)->func_ptr_0x1c();
                rider_append(&(decider->retired_riders_head), current_rider);
            } else {
                // Enqueue rider to a lift using data2 from selected_option
                lift_enqueue_riders(selected_option->data2, &current_rider);
            }
            processed_count++;
        } else if (selected_option->data1 == 1) { // Option type 1
            ConditionNode *condition_ptr = (ConditionNode*)(uintptr_t)selected_option->data2;
            if (condition_ptr == NULL) {
                // Safety check: if condition_ptr is NULL, put rider back
                rider_append(&(decider->active_riders_head), current_rider);
                return processed_count;
            }

            // Compare rider's 'next' field (at offset 8) with condition's data2 (at offset 4)
            if (((uint32_t*)current_rider)[2] < condition_ptr->data2) {
                ((RiderNode*)current_rider)->func_ptr_0x1c();
                rider_append(&(decider->retired_riders_head), current_rider);
            } else {
                // Call condition's function at offset 0x1c
                condition_ptr->func_ptr_0x1c((void*)(uintptr_t)condition_ptr, current_rider);
            }
            processed_count++;
        } else {
            // Unknown option type, put rider back
            rider_append(&(decider->active_riders_head), current_rider);
        }
    }
    return processed_count;
}

// Function: decider_new
uint32_t decider_new(DeciderState **output_decider_ptr, uint32_t *init_params) {
    DeciderState *new_decider = (DeciderState *)calloc(1, sizeof(DeciderState));
    if (new_decider == NULL) {
        _terminate(0);
    }

    new_decider->id = init_params[0];
    new_decider->type = init_params[1];
    new_decider->option_count = 0;
    new_decider->option_list_head = NULL;
    new_decider->active_riders_head = NULL;
    new_decider->retired_riders_head = NULL;

    // Assign function pointers
    new_decider->embark_fn = decider_embark;
    new_decider->disembark_fn = decider_disembark;

    if (new_decider->type == 0) {
        free(new_decider);
        *output_decider_ptr = NULL;
        return 0xFFFFFFFF; // Return -1 for error
    } else {
        *output_decider_ptr = new_decider;
        return new_decider->id;
    }
}

// Function: decider_reset
void decider_reset(DeciderState *decider, void **destination_list_head_ptr_addr) {
    // Move active riders to destination
    if (decider->active_riders_head != NULL) {
        rider_append(destination_list_head_ptr_addr, decider->active_riders_head);
        decider->active_riders_head = NULL;
    }
    // Move retired riders to destination
    if (decider->retired_riders_head != NULL) {
        rider_append(destination_list_head_ptr_addr, decider->retired_riders_head);
        decider->retired_riders_head = NULL;
    }
}

// Function: decider_destroy
void decider_destroy(DeciderState **decider_ptr_addr, void **cleanup_list_head_ptr_addr) {
    DeciderState *decider_to_destroy = *decider_ptr_addr;
    if (decider_to_destroy == NULL) {
        return;
    }

    // Move active and retired riders to the cleanup list.
    // This effectively concatenates the decider's rider lists to the cleanup list.
    // The cleanup_list_head_ptr_addr is expected to point to a list that will eventually be processed and freed.
    if (decider_to_destroy->active_riders_head != NULL) {
        rider_append(cleanup_list_head_ptr_addr, decider_to_destroy->active_riders_head);
    }
    if (decider_to_destroy->retired_riders_head != NULL) {
        rider_append(cleanup_list_head_ptr_addr, decider_to_destroy->retired_riders_head);
    }

    // Destroy the option list associated with the decider
    option_destroy_list(&(decider_to_destroy->option_list_head));

    free(decider_to_destroy);
    *decider_ptr_addr = NULL;
}