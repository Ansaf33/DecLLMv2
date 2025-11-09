#include <stdint.h> // For uint32_t
#include <stdlib.h> // For calloc, free, exit
#include <stdio.h>  // For NULL, potentially for debugging (not strictly required by problem statement)
#include <unistd.h> // For send (or sys/socket.h)
#include <sys/socket.h> // For send flags (MSG_NOSIGNAL, 0, etc.)

// Define custom types from decompilation
typedef uint32_t undefined4;
typedef unsigned int uint;

// --- Global Variables (Assumed to be defined and possibly initialized elsewhere) ---
// Counts/Sizes
uint32_t DAT_000180a8 = 0; // Number of trails
uint32_t DAT_000180a4 = 0; // Number of lifts
uint32_t resort = 0;       // Number of deciders
uint32_t DAT_000180ac = 0; // Number of riders

// Pointers to arrays of object pointers (void**)
void* DAT_000180b0 = NULL; // Array of deciders (void**)
void* DAT_000180b4 = NULL; // Array of lifts (void**)
void* DAT_000180b8 = NULL; // Array of trails (void**)

// Head of rider linked list (void*)
void* DAT_000180bc = NULL;

// Global socket file descriptor for send operations (assumed to be initialized elsewhere)
int g_sockfd = 1; // Default to stdout for basic compilation, should be a valid socket FD

// --- Forward Declarations for External (Missing) Functions ---
// These functions are assumed to be implemented elsewhere and linked.
// Their signatures are inferred from the provided snippet.

// Rider related functions
int rider_new(void** rider_ptr, int param_2);
void rider_destroy(void** rider_ptr);
void rider_append(void** list_head, void* rider_obj);
void rider_reset(void* rider_obj);
void rider_destroy_list(void** list_head);
void* rider_pop(); // Returns a rider object pointer

// Trail related functions
int trail_new(void** trail_ptr, int param_2);
void trail_destroy(void** trail_ptr, void** list_head); // list_head seems unused in provided snippet
void trail_reset(void* trail_obj, void** list_head);

// Lift related functions
int lift_new(void** lift_ptr, int param_2);
void lift_destroy(void** lift_ptr, void** list_head); // list_head seems unused in provided snippet
void lift_reset(void* lift_obj, void** list_head);

// Decider related functions
int decider_new(void** decider_ptr, int param_2);
void decider_destroy(void** decider_ptr, void** list_head); // list_head seems unused in provided snippet
void decider_add_option(undefined4 decider_obj_handle, int type, void* option_obj);
void decider_reset(void* decider_obj, void** list_head);

// Utility functions
// Assumed to read 'len' bytes into 'buf' and return number of bytes read.
ssize_t recv_all(void* buf, size_t len);
// Replaces _terminate() calls, which often indicate unrecoverable errors.
void _terminate() {
    exit(1);
}

// --- Function Implementations ---

// Function: get_trail_by_id
void* get_trail_by_id(int id) {
  for (uint i = 0; i < DAT_000180a8; ++i) {
    void* current_trail_ptr = ((void**)DAT_000180b8)[i];
    // Assumes the ID is the first 4 bytes (int) of the object
    if (current_trail_ptr != NULL && id == *(int*)current_trail_ptr) {
      return current_trail_ptr;
    }
  }
  return NULL;
}

// Function: get_lift_by_id
void* get_lift_by_id(int id) {
  for (uint i = 0; i < DAT_000180a4; ++i) {
    void* current_lift_ptr = ((void**)DAT_000180b4)[i];
    // Assumes the ID is the first 4 bytes (int) of the object
    if (current_lift_ptr != NULL && id == *(int*)current_lift_ptr) {
      return current_lift_ptr;
    }
  }
  return NULL;
}

// Function: get_decider_by_id
void* get_decider_by_id(int id) {
  for (uint i = 0; i < resort; ++i) {
    void* current_decider_ptr = ((void**)DAT_000180b0)[i];
    // Assumes the ID is the first 4 bytes (int) of the object
    if (current_decider_ptr != NULL && id == *(int*)current_decider_ptr) {
      return current_decider_ptr;
    }
  }
  return NULL;
}

// Function: get_decider_min
void* get_decider_min(void) {
  if (resort == 0) {
    return NULL;
  }

  void** deciders = (void**)DAT_000180b0;
  void* min_decider = deciders[0];

  for (uint i = 1; i < resort; ++i) {
    void* current_decider = deciders[i];
    // Assumes decider object has a uint value at offset 4 for comparison
    if (*(uint*)((char*)current_decider + 4) < *(uint*)((char*)min_decider + 4)) {
      min_decider = current_decider;
    }
  }
  return min_decider;
}

// Function: validate_rider
undefined4 validate_rider(void* rider_obj) {
  // Assumes rider object has an int value at offset 8
  if (*(int*)((char*)rider_obj + 8) == 0) {
    return 0xffffffff; // Error condition
  }
  return 0; // Success condition
}

// Function: gen_riders
undefined4 gen_riders(int* initial_offset_ptr, int count) {
  int current_offset = *initial_offset_ptr;
  while (count > 0) {
    void* rider_obj;
    int iVar1 = rider_new(&rider_obj, current_offset);
    if (iVar1 < 0) {
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    iVar1 = validate_rider(rider_obj);
    if (iVar1 < 0) {
      rider_destroy(&rider_obj);
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    rider_append(&DAT_000180bc, rider_obj);
    DAT_000180ac++;
    current_offset += 0x10; // Increment offset for next rider
    count--;
  }
  *initial_offset_ptr = current_offset; // Update initial_offset_ptr with the final offset
  return 0;
}

// Function: validate_trail
undefined4 validate_trail(void* trail_obj) {
  // Assumes the ID is the first 4 bytes (int) of the object
  if (get_trail_by_id(*(int*)trail_obj) == NULL) {
    return 0; // Success (ID not found, so it's a new valid ID)
  }
  return 0xffffffff; // Error (ID already exists)
}

// Function: gen_trails
undefined4 gen_trails(int* initial_offset_ptr) {
  int current_offset = *initial_offset_ptr;
  for (uint i = 0; i < DAT_000180a8; ++i) {
    void* trail_obj;
    int iVar1 = trail_new(&trail_obj, current_offset);
    if (iVar1 < 0) {
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    iVar1 = validate_trail(trail_obj);
    if (iVar1 < 0) {
      trail_destroy(&trail_obj, &DAT_000180bc);
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    ((void**)DAT_000180b8)[i] = trail_obj; // Store trail object in array
    current_offset += 0x14; // Increment offset for next trail
    // Assumes trail_obj has decider ID (undefined4) at offset 0x14
    void* decider_obj = get_decider_by_id(*(int*)((char*)trail_obj + 0x14));
    decider_add_option((undefined4)(uintptr_t)decider_obj, 1, trail_obj); // Cast to undefined4 for decider_add_option
  }
  *initial_offset_ptr = current_offset;
  return 0;
}

// Function: validate_lift
undefined4 validate_lift(void* lift_obj) {
  // Assumes the ID is the first 4 bytes (int) of the object
  if (get_lift_by_id(*(int*)lift_obj) == NULL) {
    return 0; // Success
  }
  return 0xffffffff; // Error
}

// Function: gen_lifts
undefined4 gen_lifts(int* initial_offset_ptr) {
  int current_offset = *initial_offset_ptr;
  for (uint i = 0; i < DAT_000180a4; ++i) {
    void* lift_obj;
    int iVar1 = lift_new(&lift_obj, current_offset);
    if (iVar1 < 0) {
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    iVar1 = validate_lift(lift_obj);
    if (iVar1 < 0) {
      lift_destroy(&lift_obj, &DAT_000180bc);
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    ((void**)DAT_000180b4)[i] = lift_obj; // Store lift object in array
    current_offset += 0x14; // Increment offset for next lift
    // Assumes lift_obj has decider ID (undefined4) at offset 0x20
    void* decider_obj = get_decider_by_id(*(int*)((char*)lift_obj + 0x20));
    decider_add_option((undefined4)(uintptr_t)decider_obj, 0, lift_obj); // Cast to undefined4 for decider_add_option
  }
  *initial_offset_ptr = current_offset;
  return 0;
}

// Function: validate_decider
undefined4 validate_decider(void* decider_obj) {
  // Assumes the ID is the first 4 bytes (int) of the object
  if (get_decider_by_id(*(int*)decider_obj) == NULL) {
    return 0; // Success
  }
  return 0xffffffff; // Error
}

// Function: gen_deciders
undefined4 gen_deciders(int* initial_offset_ptr) {
  int current_offset = *initial_offset_ptr;
  for (uint i = 0; i < resort; ++i) {
    void* decider_obj;
    int iVar1 = decider_new(&decider_obj, current_offset);
    if (iVar1 < 0) {
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    iVar1 = validate_decider(decider_obj);
    if (iVar1 < 0) {
      decider_destroy(&decider_obj, &DAT_000180bc);
      *initial_offset_ptr = current_offset;
      return 0xffffffff;
    }
    ((void**)DAT_000180b0)[i] = decider_obj; // Store decider object in array
    current_offset += 8; // Increment offset for next decider
  }
  *initial_offset_ptr = current_offset;
  return 0;
}

// Function: resort_destroy_digraph
void resort_destroy_digraph(void) {
  // Destroy deciders
  while (resort > 0) {
    resort--;
    void* decider_obj = ((void**)DAT_000180b0)[resort];
    decider_destroy(&decider_obj, &DAT_000180bc);
  }
  if (DAT_000180b0 != NULL) {
    free(DAT_000180b0);
    DAT_000180b0 = NULL;
  }

  // Destroy lifts
  while (DAT_000180a4 > 0) {
    DAT_000180a4--;
    void* lift_obj = ((void**)DAT_000180b4)[DAT_000180a4];
    lift_destroy(&lift_obj, &DAT_000180bc);
  }
  if (DAT_000180b4 != NULL) {
    free(DAT_000180b4);
    DAT_000180b4 = NULL;
  }

  // Destroy trails
  while (DAT_000180a8 > 0) {
    DAT_000180a8--;
    void* trail_obj = ((void**)DAT_000180b8)[DAT_000180a8];
    trail_destroy(&trail_obj, &DAT_000180bc);
  }
  if (DAT_000180b8 != NULL) {
    free(DAT_000180b8);
    DAT_000180b8 = NULL;
  }

  // Reset riders (iterates through the list and calls rider_reset)
  void* current_rider = DAT_000180bc;
  for (uint i = 0; i < DAT_000180ac; ++i) {
    if (current_rider == NULL) break; // Safety break if list is shorter than count
    rider_reset(current_rider);
    current_rider = *(void**)((char*)current_rider + 0x20); // Assumes next pointer is at offset 0x20
  }
}

// Function: reset_simulation_do
undefined4 reset_simulation_do(void) {
  // Reset deciders
  for (uint i = 0; i < resort; ++i) {
    void* decider_obj = ((void**)DAT_000180b0)[i];
    decider_reset(decider_obj, &DAT_000180bc);
  }

  // Reset lifts
  for (uint i = 0; i < DAT_000180a4; ++i) {
    void* lift_obj = ((void**)DAT_000180b4)[i];
    lift_reset(lift_obj, &DAT_000180bc);
  }

  // Reset trails
  for (uint i = 0; i < DAT_000180a8; ++i) {
    void* trail_obj = ((void**)DAT_000180b8)[i];
    trail_reset(trail_obj, &DAT_000180bc);
  }

  // Reset riders
  void* current_rider = DAT_000180bc;
  for (uint i = 0; i < DAT_000180ac; ++i) {
    if (current_rider == NULL) return 0xffffffff; // Error if list ends prematurely
    rider_reset(current_rider);
    current_rider = *(void**)((char*)current_rider + 0x20); // Assumes next pointer is at offset 0x20
  }
  return 0;
}

// Function: go_simul8
undefined4 go_simul8(int num_iterations) {
  for (int iter = num_iterations; iter > 0; --iter) {
    // Deciders simulation step
    for (uint i = 0; i < resort; ++i) {
      void* decider_obj = ((void**)DAT_000180b0)[i];
      // Assumes function pointer for decider simulation is at offset 0x1c
      ((void (*)(void*))((char*)decider_obj + 0x1c))(decider_obj);
    }

    // Trails simulation step
    for (uint i = 0; i < DAT_000180a8; ++i) {
      void* trail_obj = ((void**)DAT_000180b8)[i];
      // Assumes function pointer for trail simulation is at offset 0x20
      ((void (*)(void*))((char*)trail_obj + 0x20))(trail_obj);
    }

    // Lifts simulation step
    for (uint i = 0; i < DAT_000180a4; ++i) {
      void* lift_obj = ((void**)DAT_000180b4)[i];
      // Assumes function pointer for lift simulation is at offset 0x2c
      ((void (*)(void*))((char*)lift_obj + 0x2c))(lift_obj);
    }
  }
  return 0;
}

// Function: load_resort_digraph
undefined4 load_resort_digraph(void) {
  resort_destroy_digraph();

  uint32_t counts_buffer[3]; // For resort, DAT_000180a4, DAT_000180a8
  if (recv_all(counts_buffer, sizeof(counts_buffer)) != sizeof(counts_buffer)) {
    _terminate();
  }
  resort = counts_buffer[0];
  DAT_000180a4 = counts_buffer[1];
  DAT_000180a8 = counts_buffer[2];

  // Allocate memory for arrays of object pointers
  DAT_000180b0 = calloc(resort, sizeof(void*));
  if (DAT_000180b0 == NULL) {
    _terminate();
  }

  DAT_000180b4 = calloc(DAT_000180a4, sizeof(void*));
  if (DAT_000180b4 == NULL) {
    _terminate();
  }

  DAT_000180b8 = calloc(DAT_000180a8, sizeof(void*));
  if (DAT_000180b8 == NULL) {
    _terminate();
  }

  // Calculate total size for object data to be received
  int total_obj_data_size = resort * 8 + DAT_000180a4 * 0x14 + DAT_000180a8 * 0x14;
  void* data_buffer = calloc(total_obj_data_size, 1);
  if (data_buffer == NULL) {
    _terminate();
  }

  // Receive all object data into the buffer
  if (recv_all(data_buffer, total_obj_data_size) != total_obj_data_size) {
    free(data_buffer);
    _terminate();
  }

  int current_data_offset = 0; // This will track the offset into `data_buffer`

  // Generate deciders, lifts, and trails using data from the buffer
  if (gen_deciders(&current_data_offset) < 0) {
    free(data_buffer);
    return 0xffffffff;
  }

  if (gen_lifts(&current_data_offset) < 0) {
    free(data_buffer);
    return 0xffffffff;
  }

  if (gen_trails(&current_data_offset) < 0) {
    free(data_buffer);
    return 0xffffffff;
  }

  free(data_buffer);
  return 0;
}

// Function: load_rider_group
undefined4 load_rider_group(void) {
  int num_riders_to_gen;
  if (recv_all(&num_riders_to_gen, sizeof(num_riders_to_gen)) != sizeof(num_riders_to_gen)) {
    _terminate();
  }

  int total_rider_data_size = num_riders_to_gen * 0x10; // Each rider is 0x10 bytes
  void* data_buffer = calloc(total_rider_data_size, 1);
  if (data_buffer == NULL) {
    _terminate();
  }

  if (recv_all(data_buffer, total_rider_data_size) != total_rider_data_size) {
    free(data_buffer);
    _terminate();
  }

  int current_data_offset = 0; // This will be passed to gen_riders as the initial offset

  if (gen_riders(&current_data_offset, num_riders_to_gen) < 0) {
    free(data_buffer);
    return 0xffffffff;
  }

  free(data_buffer);
  return 0;
}

// Function: load_rider_single
undefined4 load_rider_single(void) {
  int num_riders_to_gen;
  if (recv_all(&num_riders_to_gen, sizeof(num_riders_to_gen)) != sizeof(num_riders_to_gen)) {
    _terminate();
  }

  if (num_riders_to_gen != 1) {
    return 0xffffffff;
  }

  int total_rider_data_size = 0x10; // Fixed size for a single rider
  void* data_buffer = calloc(total_rider_data_size, 1);
  if (data_buffer == NULL) {
    _terminate();
  }

  if (recv_all(data_buffer, total_rider_data_size) != total_rider_data_size) {
    free(data_buffer);
    _terminate();
  }

  int current_data_offset = 0;

  if (gen_riders(&current_data_offset, num_riders_to_gen) < 0) {
    free(data_buffer);
    return 0xffffffff;
  }

  free(data_buffer);
  return 0;
}

// Function: unload_riders
undefined4 unload_riders(void) {
  undefined4 result = reset_simulation_do();
  rider_destroy_list(&DAT_000180bc);
  DAT_000180ac = 0;
  return result;
}

// Function: start_simulation
undefined4 start_simulation(void) {
  int num_steps;
  if (recv_all(&num_steps, sizeof(num_steps)) != sizeof(num_steps)) {
    _terminate();
  }

  if (num_steps == 0) {
    return 0xffffffff;
  }

  void* min_decider_obj = get_decider_min();
  if (min_decider_obj == NULL) {
    return 0xffffffff;
  }
  if (DAT_000180ac == 0) { // No riders to simulate
    return 0xffffffff;
  }

  // Distribute riders to the min_decider_obj
  while (DAT_000180bc != NULL) { // While rider list is not empty
    void* rider_obj = rider_pop();
    // Assumes function pointer for decider's rider distribution is at offset 0x18
    ((void (*)(void*, void*))((char*)min_decider_obj + 0x18))(min_decider_obj, rider_obj);
  }

  go_simul8(num_steps);
  return 0;
}

// Function: reset_simulation
void reset_simulation(void) {
  reset_simulation_do();
  return;
}

// Function: lift_stats
undefined4 lift_stats(void) {
  if (DAT_000180a4 == 0) {
    return 0xffffffff;
  }

  size_t data_size = DAT_000180a4 * 8; // Each lift sends ID (4 bytes) and some stat (4 bytes)
  void* data_buffer = calloc(data_size, 1);
  if (data_buffer == NULL) {
    _terminate();
  }

  int current_buffer_idx = 0;
  for (uint i = 0; i < DAT_000180a4; ++i) {
    void* current_lift_obj = ((void**)DAT_000180b4)[i];
    // Assumes lift obj has ID at offset 0 and stat at offset 0x0C (index 3 of undefined4 array)
    ((undefined4*)data_buffer)[current_buffer_idx] = *(undefined4*)current_lift_obj;
    ((undefined4*)data_buffer)[current_buffer_idx + 1] = *(undefined4*)((char*)current_lift_obj + 0xC);
    current_buffer_idx += 2;
  }

  send(g_sockfd, data_buffer, data_size, 0); // Using global socket FD, 0 for default flags

  free(data_buffer);
  return 0;
}

// Function: trail_stats
undefined4 trail_stats(void) {
  if (DAT_000180a8 == 0) {
    return 0xffffffff;
  }

  size_t data_size = DAT_000180a8 * 8; // Each trail sends ID (4 bytes) and some stat (4 bytes)
  void* data_buffer = calloc(data_size, 1);
  if (data_buffer == NULL) {
    _terminate();
  }

  int current_buffer_idx = 0;
  for (uint i = 0; i < DAT_000180a8; ++i) {
    void* current_trail_obj = ((void**)DAT_000180b8)[i];
    // Assumes trail obj has ID at offset 0 and stat at offset 0x0C (index 3 of undefined4 array)
    ((undefined4*)data_buffer)[current_buffer_idx] = *(undefined4*)current_trail_obj;
    ((undefined4*)data_buffer)[current_buffer_idx + 1] = *(undefined4*)((char*)current_trail_obj + 0xC);
    current_buffer_idx += 2;
  }

  send(g_sockfd, data_buffer, data_size, 0);

  free(data_buffer);
  return 0;
}

// Function: rider_stats
undefined4 rider_stats(void) {
  if (DAT_000180ac == 0) {
    return 0xffffffff;
  }

  size_t data_size = DAT_000180ac * 8; // Each rider sends ID (4 bytes) and some stat (4 bytes)
  void* data_buffer = calloc(data_size, 1);
  if (data_buffer == NULL) {
    _terminate();
  }

  uint current_data_idx = 0;

  // Collect stats from riders in deciders
  for (uint i = 0; i < resort; ++i) {
    void* decider_obj = ((void**)DAT_000180b0)[i];
    // Assumes decider has two rider lists at offset 8 and 0xC
    // List 1:
    for (void* rider_node = *(void**)((char*)decider_obj + 8);
         rider_node != NULL;
         rider_node = *(void**)((char*)rider_node + 0x20)) { // Assumes next pointer at 0x20
      ((undefined4*)data_buffer)[current_data_idx] = *(undefined4*)rider_node;     // Rider ID at offset 0
      ((undefined4*)data_buffer)[current_data_idx + 1] = *(undefined4*)((char*)rider_node + 8); // Rider stat at offset 8
      current_data_idx += 2;
    }
    // List 2:
    for (void* rider_node = *(void**)((char*)decider_obj + 0xC);
         rider_node != NULL;
         rider_node = *(void**)((char*)rider_node + 0x20)) {
      ((undefined4*)data_buffer)[current_data_idx] = *(undefined4*)rider_node;
      ((undefined4*)data_buffer)[current_data_idx + 1] = *(undefined4*)((char*)rider_node + 8);
      current_data_idx += 2;
    }
  }

  // Collect stats from riders in lifts
  for (uint i = 0; i < DAT_000180a4; ++i) {
    void* lift_obj = ((void**)DAT_000180b4)[i];
    // Assumes lift has a rider list at offset 0x10
    for (void* rider_node = *(void**)((char*)lift_obj + 0x10);
         rider_node != NULL;
         rider_node = *(void**)((char*)rider_node + 0x20)) {
      ((undefined4*)data_buffer)[current_data_idx] = *(undefined4*)rider_node;
      ((undefined4*)data_buffer)[current_data_idx + 1] = *(undefined4*)((char*)rider_node + 8);
      current_data_idx += 2;
    }
    // Assumes lift has a queue of nodes, each with a rider list, starting at offset 0x14
    for (void* queue_node = *(void**)((char*)lift_obj + 0x14);
         queue_node != NULL;
         queue_node = *(void**)((char*)queue_node + 0x18)) { // Assumes next queue node at 0x18
      for (void* rider_node = *(void**)((char*)queue_node + 0xC); // Assumes rider list in queue node at 0xC
           rider_node != NULL;
           rider_node = *(void**)((char*)rider_node + 0x20)) {
        ((undefined4*)data_buffer)[current_data_idx] = *(undefined4*)rider_node;
        ((undefined4*)data_buffer)[current_data_idx + 1] = *(undefined4*)((char*)rider_node + 8);
        current_data_idx += 2;
      }
    }
  }

  // Collect stats from riders in trails
  for (uint i = 0; i < DAT_000180a8; ++i) {
    void* trail_obj = ((void**)DAT_000180b8)[i];
    // Assumes trail has a rider list at offset 0x10
    for (void* rider_node = *(void**)((char*)trail_obj + 0x10);
         rider_node != NULL;
         rider_node = *(void**)((char*)rider_node + 0x20)) {
      ((undefined4*)data_buffer)[current_data_idx] = *(undefined4*)rider_node;
      ((undefined4*)data_buffer)[current_data_idx + 1] = *(undefined4*)((char*)rider_node + 8);
      current_data_idx += 2;
    }
  }

  // Collect stats from riders in the general rider list (DAT_000180bc)
  for (void* rider_node = DAT_000180bc;
       rider_node != NULL;
       rider_node = *(void**)((char*)rider_node + 0x20)) {
    ((undefined4*)data_buffer)[current_data_idx] = *(undefined4*)rider_node;
    ((undefined4*)data_buffer)[current_data_idx + 1] = *(undefined4*)((char*)rider_node + 8);
    current_data_idx += 2;
  }

  send(g_sockfd, data_buffer, data_size, 0);

  free(data_buffer);
  return 0;
}