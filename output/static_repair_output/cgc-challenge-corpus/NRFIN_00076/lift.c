#include <stdlib.h> // For calloc, free, exit, NULL
#include <stdint.h> // For uint32_t, etc., though unsigned int is used for undefined4.

// --- External Function Declarations (Assumed) ---

// Functions to manage a generic rider list
// 'list_head_addr' is an integer that represents the address of the list head.
// This matches the decompiler's representation of pointers as integers.
void rider_append(unsigned int list_head_addr, int rider_id);
int rider_pop(unsigned int list_head_addr);

// --- Forward Declarations for Structs and Function Pointers ---

typedef struct Chair Chair;
typedef struct Decider Decider;
typedef struct Lift Lift;

// Chair function pointer types
typedef void (*ChairEmbarkRiderFn)(Chair* chair, unsigned int rider_id);
typedef int (*ChairDisembarkRiderFn)(Chair* chair);

// Decider function pointer types (assuming a simple structure for now)
typedef void (*DeciderProcessRiderFn)(Decider* decider, int rider_id);

// Lift function pointer types
typedef int (*LiftEmbarkRidersFn)(Lift* lift);
typedef void (*LiftOneStepFn)(Lift* lift);
typedef int (*LiftDisembarkRidersFn)(Lift* lift);

// --- Struct Definitions ---

struct Chair {
    unsigned int field0;          // 0x0 (original param_2 in chair_new)
    int capacity;                 // 0x4 (original param_3 in chair_new)
    int available_slots;          // 0x8 (initially capacity, decrements on embark)
    unsigned int rider_list_head; // 0xC (head of riders currently on this chair)
    ChairEmbarkRiderFn embark_rider_fn; // 0x10
    ChairDisembarkRiderFn disembark_rider_fn; // 0x14
    Chair *next;                  // 0x18 (next chair in a linked list)
}; // Size: 0x1C (28 bytes)

// Minimal Decider struct based on usage in lift_disembark_riders
struct Decider {
    // ... potentially other fields
    DeciderProcessRiderFn process_rider_fn; // 0x18 (function called to process a disembarked rider)
};

// Function to get a decider object by its ID (assumed external)
Decider* get_decider_by_id(unsigned int id);

struct Lift {
    unsigned int field0;                // 0x0 (from param_2[0] in lift_new)
    unsigned int num_chairs;            // 0x4 (from param_2[3])
    unsigned int chair_capacity;        // 0x8 (from param_2[4])
    int riders_on_lift_count;           // 0xC
    unsigned int rider_list_head;       // 0x10 (riders waiting to embark on the lift)
    Chair *chairs_head;                 // 0x14 (head of the chair list for this lift)
    Chair *current_chair_embark;        // 0x18 (chair currently embarking riders)
    Chair *current_chair_disembark;     // 0x1C (chair currently disembarking riders)
    unsigned int field20;               // 0x20 (unused or another field)
    unsigned int decider_id;            // 0x24 (from param_2[1])
    LiftEmbarkRidersFn embark_riders_fn; // 0x28
    LiftOneStepFn one_step_fn;          // 0x2C
    LiftDisembarkRidersFn disembark_riders_fn; // 0x30
}; // Size: 0x34 (52 bytes)

// --- Function Implementations ---

// Function: chair_embark_rider
void chair_embark_rider(Chair* chair, unsigned int rider_id) {
  rider_append(chair->rider_list_head, rider_id);
  chair->available_slots--;
}

// Function: chair_disembark_rider
int chair_disembark_rider(Chair* chair) {
  int rider_id = rider_pop(chair->rider_list_head);
  if (rider_id != 0) {
    chair->available_slots++;
  }
  return rider_id;
}

// Function: chair_new
// param_1: pointer to a Chair* to store the newly created chair
// param_2: value for Chair->field0 (often 0)
// param_3: capacity for the chair
unsigned int chair_new(Chair **chair_out, unsigned int field0_val, int capacity) {
  if (capacity == 0) {
    return 0xffffffff; // Error code
  }
  
  Chair *new_chair = (Chair *)calloc(1, sizeof(Chair));
  if (new_chair == NULL) {
    exit(EXIT_FAILURE); // Terminate on allocation failure
  }
  
  new_chair->field0 = field0_val;
  new_chair->capacity = capacity;
  new_chair->available_slots = capacity; // Initially, all slots are available
  new_chair->embark_rider_fn = chair_embark_rider;
  new_chair->disembark_rider_fn = chair_disembark_rider;
  
  *chair_out = new_chair;
  return 0; // Success
}

// Function: chair_reset
void chair_reset(Chair* chair, unsigned int global_rider_list_head) {
  while (1) {
    int rider_id = chair->disembark_rider_fn(chair);
    if (rider_id == 0) break;
    rider_append(global_rider_list_head, rider_id);
  }
}

// Function: chair_reset_list
void chair_reset_list(Chair* head_chair, unsigned int global_rider_list_head) {
  for (Chair *current = head_chair; current != NULL; current = current->next) {
    chair_reset(current, global_rider_list_head);
  }
}

// Function: chair_append_new
// head_ptr_addr: pointer to the integer that stores the head address of the chair list (e.g., &lift->chairs_head)
// new_chair_addr: the integer address of the new chair to append
int chair_append_new(int *head_ptr_addr, int new_chair_addr) {
  if (*head_ptr_addr == 0) { // If list is empty
    *head_ptr_addr = new_chair_addr;
  } else {
    Chair *current = (Chair *)*head_ptr_addr;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = (Chair *)new_chair_addr;
  }
  return new_chair_addr;
}

// Function: chair_pop
// head_ptr_addr: pointer to the integer that stores the head address of the chair list
int chair_pop(int *head_ptr_addr) {
  if (*head_ptr_addr == 0) {
    return 0; // List is empty
  } else {
    int popped_chair_addr = *head_ptr_addr;
    Chair *popped_chair = (Chair *)popped_chair_addr;
    *head_ptr_addr = (int)popped_chair->next; // Update head to next chair
    popped_chair->next = NULL; // Detach popped chair from list
    return popped_chair_addr;
  }
}

// Function: chair_destroy_single
void chair_destroy_single(Chair **chair_ptr_out, unsigned int global_rider_list_head) {
  chair_reset(*chair_ptr_out, global_rider_list_head);
  free(*chair_ptr_out);
  *chair_ptr_out = NULL;
}

// Function: chair_destroy_list
// head_ptr_addr: pointer to the integer that stores the head address of the chair list
void chair_destroy_list(int *head_ptr_addr, unsigned int global_rider_list_head) {
  Chair *temp_chair; // Single intermediate variable for the popped chair
  while (*head_ptr_addr != 0) {
    temp_chair = (Chair *)chair_pop(head_ptr_addr);
    chair_destroy_single(&temp_chair, global_rider_list_head);
  }
}

// Function: lift_enqueue_riders
void lift_enqueue_riders(Chair* current_rider_list_head_chair, unsigned int global_rider_list_head) {
  while (1) {
    int rider_id = rider_pop(global_rider_list_head);
    if (rider_id == 0) break;
    rider_append(current_rider_list_head_chair->rider_list_head, rider_id);
  }
}

// Function: lift_embark_riders
int lift_embark_riders(Lift* lift) {
  int embarked_count = 0;
  Chair *current_chair = lift->current_chair_embark;

  while (current_chair->available_slots != 0) {
    int rider_id = rider_pop(lift->rider_list_head);
    if (rider_id == 0) break; // No more riders to embark

    current_chair->embark_rider_fn(current_chair, rider_id);
    lift->riders_on_lift_count++;
    embarked_count++;
  }

  // Move to the next chair for embarking
  if (current_chair->next == NULL) {
    lift->current_chair_embark = lift->chairs_head; // Loop back to the first chair
  } else {
    lift->current_chair_embark = current_chair->next;
  }
  return embarked_count;
}

// Function: lift_one_step
void lift_one_step(Lift* lift) {
  lift->disembark_riders_fn(lift); // Disembark riders first
  lift->embark_riders_fn(lift);    // Then embark new riders
}

// Function: lift_disembark_riders
int lift_disembark_riders(Lift* lift) {
  int disembarked_count = 0;
  Decider *decider = get_decider_by_id(lift->decider_id);

  if (decider == NULL) {
    return 0; // No decider, no disembarkation
  }
  
  Chair *current_chair = lift->current_chair_disembark;
  while (current_chair->available_slots < current_chair->capacity) { // While there are riders on the chair
    int rider_id = current_chair->disembark_rider_fn(current_chair);
    if (rider_id == 0) break; // No more riders to disembark from this chair
    
    decider->process_rider_fn(decider, rider_id);
    disembarked_count++;
  }

  // Move to the next chair for disembarking
  if (current_chair->next == NULL) {
    lift->current_chair_disembark = lift->chairs_head; // Loop back to the first chair
  } else {
    lift->current_chair_disembark = current_chair->next;
  }
  return disembarked_count;
}

// Function: lift_new
// param_1: pointer to a Lift* to store the newly created lift
// param_2: pointer to an array of unsigned int configuration values:
//          param_2[0]: Lift->field0
//          param_2[1]: Lift->decider_id
//          param_2[2]: Lift->field9 (unused in this context)
//          param_2[3]: Lift->num_chairs
//          param_2[4]: Lift->chair_capacity
unsigned int lift_new(Lift **lift_out, unsigned int *config_params) {
  Lift *new_lift = (Lift *)calloc(1, sizeof(Lift));
  if (new_lift == NULL) {
    exit(EXIT_FAILURE); // Terminate on allocation failure
  }
  
  new_lift->field0 = config_params[0];
  new_lift->decider_id = config_params[1];
  // config_params[2] is unused based on original code
  new_lift->num_chairs = config_params[3];
  new_lift->chair_capacity = config_params[4];
  
  new_lift->embark_riders_fn = lift_embark_riders;
  new_lift->one_step_fn = lift_one_step;
  new_lift->disembark_riders_fn = lift_disembark_riders;
  
  // Validation checks
  if (new_lift->num_chairs == 0 || new_lift->decider_id == config_params[2] ||
      (new_lift->chair_capacity != 2 && new_lift->chair_capacity != 4)) {
    free(new_lift);
    return 0xffffffff; // Error code
  }
  
  Chair *new_chair_obj;
  for (unsigned int i = 0; i < new_lift->num_chairs; i++) {
    // Create new chair with 0 initial field0_val and capacity from lift config
    if (chair_new(&new_chair_obj, 0, new_lift->chair_capacity) != 0) {
        // Handle error: free partially created chairs and lift
        chair_destroy_list((int*)&new_lift->chairs_head, new_lift->rider_list_head); // Clean up already added chairs
        free(new_lift);
        return 0xffffffff;
    }
    
    // Append the new chair to the lift's chair list
    chair_append_new((int*)&new_lift->chairs_head, (int)new_chair_obj);
    
    if (i == 0) {
      new_lift->current_chair_embark = new_chair_obj;
    }
    if (i == new_lift->num_chairs / 2) {
      new_lift->current_chair_disembark = new_chair_obj;
    }
  }
  
  *lift_out = new_lift;
  return new_lift->field0; // Success, returning field0 as status
}

// Function: lift_reset
void lift_reset(Lift* lift, unsigned int global_rider_list_head) {
  chair_reset_list(lift->chairs_head, global_rider_list_head);
  rider_append(global_rider_list_head, lift->rider_list_head); // Move riders from lift's queue to global queue
  
  lift->rider_list_head = 0; // Clear lift's rider queue
  lift->riders_on_lift_count = 0;
  
  lift->current_chair_embark = lift->chairs_head;
  lift->current_chair_disembark = lift->chairs_head;
  
  // Advance disembark chair to the middle of the lift chain
  for (unsigned int i = 0; i < lift->num_chairs && (i != lift->num_chairs / 2); i++) {
    if (lift->current_chair_disembark->next != NULL) {
        lift->current_chair_disembark = lift->current_chair_disembark->next;
    } else {
        // Should not happen if num_chairs > 0 and loop is correct
        break; 
    }
  }
}

// Function: lift_destroy
void lift_destroy(Lift **lift_ptr_out, unsigned int global_rider_list_head) {
  Lift *lift = *lift_ptr_out;
  chair_destroy_list((int*)&lift->chairs_head, global_rider_list_head); // Destroy all chairs
  rider_append(global_rider_list_head, lift->rider_list_head); // Move any remaining riders to global queue
  free(lift);
  *lift_ptr_out = NULL;
}