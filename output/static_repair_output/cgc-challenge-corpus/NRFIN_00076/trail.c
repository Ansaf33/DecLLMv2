#include <stdlib.h> // For calloc, free, exit
#include <stdint.h> // For uint32_t, uint8_t

// Forward declarations for structs
struct Rider;
struct Decider;
struct Trail;

// Define function pointer types for Trail methods and external functions
// Assuming 32-bit architecture where pointers are 4 bytes, matching uint32_t.
typedef void (*TrailEmbarkFunc)(struct Trail*, uint32_t);
typedef void (*TrailOneStepFunc)(struct Trail*);
typedef int (*TrailDisembarkFunc)(struct Trail*); // Returns int

typedef void (*DeciderFunc)(struct Decider*, struct Rider*);
typedef struct Decider* (*GetDeciderByIdFunc)(uint32_t id);
typedef void (*RiderAppendFunc)(struct Rider**, uint32_t); // Appends rider_data to list_head_ptr
typedef struct Rider* (*RiderPopFunc)(struct Rider**);     // Pops rider from list_head_ptr

// --- Struct Definitions based on observed memory access patterns ---

// Struct Rider: Fields accessed at specific offsets
struct Rider {
    uint8_t _padding_0_7[8]; // Padding to reach offset 0x8
    uint32_t counter_8;      // Accessed at offset 0x8
    uint8_t _padding_C_F[4]; // Padding to reach offset 0x10
    uint32_t counter_10;     // Accessed at offset 0x10
    uint32_t counter_14;     // Accessed at offset 0x14
    uint8_t _padding_18_1F[8]; // Padding to reach offset 0x20
    struct Rider* next_20;   // Accessed at offset 0x20, used for linked list traversal
    // Assuming no other fields are relevant or accessed in these functions
};

// Struct Decider: Field accessed at specific offset
struct Decider {
    uint8_t _padding_0_17[24]; // Padding to reach offset 0x18
    DeciderFunc func_18;       // Accessed at offset 0x18
    // Assuming no other fields are relevant or accessed in these functions
};

// Struct Trail: Fields accessed at specific offsets, total size 0x28 (40 bytes)
struct Trail {
    uint32_t field_0; // Initialized by param_2[0] in trail_new
    uint32_t value_4; // Initialized by param_2[3] in trail_new, used in trail_disembark
    uint32_t value_8; // Initialized by param_2[4] in trail_new, used in trail_disembark
    uint32_t counter_c; // Incremented in trail_embark, reset in trail_reset
    struct Rider* riders_list_10; // Head of rider list, accessed at 0x10
    uint32_t field_14; // Initialized by param_2[1] in trail_new
    uint32_t decider_id_18; // Initialized by param_2[2] in trail_new, used in trail_disembark

    // Function pointers for Trail methods, assigned in trail_new
    TrailEmbarkFunc embark_method_1c;    // Assigned to trail_embark
    TrailOneStepFunc one_step_method_20; // Assigned to trail_one_step
    // Note: trail_one_step *calls* the function stored at this offset (0x24),
    // which trail_new assigns to trail_disembark. This is an unusual design.
    TrailDisembarkFunc disembark_method_24; // Assigned to trail_disembark, called by trail_one_step
};

// --- Stub Implementations for External Functions ---
// These functions are not provided in the snippet but are called by the given code.
// Their signatures are inferred from context.
void rider_append(struct Rider** list_head_ptr, uint32_t rider_data) {
    // Placeholder implementation for rider_append
    // In a real scenario, this would manage a linked list of Riders
    (void)list_head_ptr; // Suppress unused parameter warning
    (void)rider_data;    // Suppress unused parameter warning
    // Example: append a new rider identified by rider_data
    // struct Rider* new_rider = (struct Rider*)malloc(sizeof(struct Rider));
    // if (new_rider) {
    //     new_rider->rider_id = rider_data; // Assuming rider_data is an ID
    //     new_rider->next_20 = *list_head_ptr;
    //     *list_head_ptr = new_rider;
    // }
}

struct Rider* rider_pop(struct Rider** list_head_ptr) {
    // Placeholder implementation for rider_pop (LIFO stack-like behavior assumed)
    if (list_head_ptr == NULL || *list_head_ptr == NULL) {
        return NULL; // List is empty
    }
    struct Rider* popped = *list_head_ptr;
    *list_head_ptr = popped->next_20; // Update head of the list
    popped->next_20 = NULL; // Detach popped rider from list
    return popped;
}

struct Decider* get_decider_by_id(uint32_t id) {
    // Placeholder implementation for get_decider_by_id
    (void)id; // Suppress unused parameter warning
    // In a real scenario, this would lookup and return a Decider object
    return NULL; // Return NULL if not found or for simplicity
}

void _terminate() {
    // Placeholder for a custom termination function, replaced with standard exit
    exit(EXIT_FAILURE);
}

// --- Fixed and Refactored Functions ---

// Function: trail_embark
void trail_embark(struct Trail* trail_ptr, uint32_t rider_data) {
  rider_append(&(trail_ptr->riders_list_10), rider_data);
  trail_ptr->counter_c++;
}

// Function: trail_one_step
void trail_one_step(struct Trail* trail_ptr) {
  // Call the function pointer stored at offset 0x24 (disembark_method_24)
  if (trail_ptr->disembark_method_24) {
      trail_ptr->disembark_method_24(trail_ptr);
  }
  
  for (struct Rider* current_rider = trail_ptr->riders_list_10; current_rider != NULL; current_rider = current_rider->next_20) {
    current_rider->counter_14++;
  }
}

// Function: trail_disembark
int trail_disembark(struct Trail* trail_ptr) {
  int disembarked_count = 0;
  struct Decider* decider = get_decider_by_id(trail_ptr->decider_id_18);

  if (decider == NULL) {
    return 0;
  }
  
  while (trail_ptr->riders_list_10 != NULL &&
         trail_ptr->value_8 == trail_ptr->riders_list_10->counter_14) {
    
    struct Rider* popped_rider = rider_pop(&(trail_ptr->riders_list_10));
    
    if (popped_rider != NULL) {
        popped_rider->counter_14 = 0;
        popped_rider->counter_10++;
        popped_rider->counter_8 -= trail_ptr->value_4;

        if (decider->func_18) {
            decider->func_18(decider, popped_rider);
        }
        disembarked_count++;
    } else {
        // Should not happen if riders_list_10 was not NULL, but good for robustness
        break; 
    }
  }
  return disembarked_count;
}

// Function: trail_new
uint32_t trail_new(struct Trail** new_trail_out, uint32_t* init_data) {
  struct Trail* trail = (struct Trail*)calloc(1, sizeof(struct Trail));
  if (trail == NULL) {
    _terminate(); // Use the stubbed _terminate function
  }

  // Initialize fields based on init_data (param_2 in original) and observed offsets
  trail->field_0 = init_data[0];
  trail->field_14 = init_data[1]; // Corresponds to local_10[5]
  trail->decider_id_18 = init_data[2]; // Corresponds to local_10[6]
  trail->value_4 = init_data[3]; // Corresponds to local_10[1]
  trail->value_8 = init_data[4]; // Corresponds to local_10[2]

  // Assign function pointers (methods of the Trail object)
  trail->embark_method_1c = trail_embark;
  trail->one_step_method_20 = trail_one_step;
  trail->disembark_method_24 = trail_disembark;

  // Error condition check: (value_8 == 0) || (field_14 == decider_id_18) || (value_4 == 0)
  if (trail->value_8 == 0 || trail->field_14 == trail->decider_id_18 || trail->value_4 == 0) {
    free(trail);
    *new_trail_out = NULL;
    return 0xFFFFFFFF; // Return -1 as uint32_t
  } else {
    *new_trail_out = trail;
    return trail->field_0; // Return field_0 (original *local_10)
  }
}

// Function: trail_reset
void trail_reset(struct Trail* trail_ptr, uint32_t target_list_head_addr) {
  // Cast target_list_head_addr back to the expected type for rider_append
  rider_append((struct Rider**)target_list_head_addr, (uint32_t)trail_ptr->riders_list_10);
  trail_ptr->riders_list_10 = NULL;
  trail_ptr->counter_c = 0;
}

// Function: trail_destroy
void trail_destroy(struct Trail** trail_ptr_ptr, uint32_t target_list_head_addr) {
  struct Trail* trail = *trail_ptr_ptr;
  if (trail == NULL) return;

  // Append the trail's current rider list head to the target list
  rider_append((struct Rider**)target_list_head_addr, (uint32_t)trail->riders_list_10);
  
  free(trail);
  *trail_ptr_ptr = NULL;
}