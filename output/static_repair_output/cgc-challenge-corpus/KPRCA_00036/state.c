#include <stdlib.h> // For malloc, free
#include <stdint.h> // For uint8_t, uint32_t

// Using unsigned int for uint as it's common and typically 32-bit.
// If strict 32-bit is required for the 'id' field, use uint32_t.
typedef unsigned int uint;

// Define the State structure based on memory accesses, assuming 32-bit architecture
// where pointers are 4 bytes and unsigned int is 4 bytes.
typedef struct State {
    uint id;          // Offset 0x00 (4 bytes)
    uint8_t type;     // Offset 0x04 (1 byte)
    uint8_t flags;    // Offset 0x05 (1 byte)
    uint8_t padding[2]; // Offset 0x06 (2 bytes) - padding to align next pointers to 4-byte boundary
    struct State *next1; // Offset 0x08 (4 bytes)
    struct State *next2; // Offset 0x0C (4 bytes)
    struct State *next_end; // Offset 0x10 (4 bytes)
} State; // Total size: 4+1+1+2+4+4+4 = 20 bytes (0x14)

// Global variables
State **g_all_states = NULL; // Initialized to NULL (0x0)
uint g_id = 0; // Initialized to 0

// Function: init_trex
void init_trex(void) {
  if (g_all_states == NULL) {
    // Allocate space for 1000 State* pointers (1000 * 4 bytes = 4000 bytes on 32-bit)
    g_all_states = (State **)malloc(sizeof(State *) * 1000);
    // In a production environment, one should check if malloc returns NULL
    g_id = 0;
  }
}

// Function: clear_trex
void clear_trex(void) {
  uint i; // Renamed local_10 to i
  
  for (i = 0; i < g_id; ++i) {
    free(g_all_states[i]); // Free each allocated State object
  }
  g_id = 0; // Reset the counter
  // Note: The g_all_states array itself is not freed here,
  // matching the behavior of the original snippet.
}

// Function: create_state
State * create_state(uint8_t param_1) { // Changed param_1 type to uint8_t
  State *newState = NULL; // Renamed puVar1 to newState
  
  if (g_id < 1000) { // Check if there's capacity for a new state
    newState = (State *)malloc(sizeof(State)); // Allocate 20 bytes for the State object
    if (newState == NULL) { // Handle allocation failure
        return NULL;
    }
    newState->id = g_id;
    newState->type = param_1;
    newState->flags = 1;
    newState->next1 = NULL; // Initialize pointers to NULL
    newState->next2 = NULL;
    newState->next_end = NULL;
    
    g_all_states[g_id] = newState; // Store the new state in the global array
    g_id++; // Increment the global ID counter
  }
  return newState;
}

// Function: op_concat
State *op_concat(State *param_1, State *param_2) { // Changed types to State*
  if (param_1 == NULL || param_2 == NULL) return NULL; // Safety check

  param_1->flags = 0;

  if (param_1->next_end == NULL) {
    param_1->next1 = param_2;
  } else {
    param_1->next_end->flags = 0;
    if (param_1->next_end->next1 == NULL) {
      param_1->next_end->next1 = param_2;
    } else {
      param_1->next_end->next2 = param_2;
    }
  }

  // Original: if (*(int *)(param_2 + 0x10) != 0) { param_2 = *(int *)(param_2 + 0x10); }
  // *(int *)(param_1 + 0x10) = param_2;
  // This means 'param_2' variable itself is updated if 'param_2->next_end' exists,
  // and then the (potentially updated) 'param_2' value is assigned to 'param_1->next_end'.
  if (param_2->next_end != NULL) {
    param_2 = param_2->next_end; // Update param_2 pointer
  }
  param_1->next_end = param_2; // Assign the (potentially updated) param_2 to param_1's end
  return param_1;
}

// Function: op_union
State *op_union(State *param_1, State *param_2) { // Changed types to State*
  State *s1 = create_state(0x80); // Corresponds to original iVar1
  State *s2 = create_state(0x80); // Corresponds to original iVar2
  
  if (s1 == NULL || s2 == NULL) { // Check if both states were created successfully
    if (s1) free(s1); // Free any successfully created state before returning NULL
    if (s2) free(s2);
    return NULL;
  }

  s1->flags = 0;
  s1->next1 = param_1;
  s1->next2 = param_2;
  s1->next_end = s2;

  // Modifications for param_1's outgoing transitions
  param_1->flags = 0;
  if (param_1->next_end == NULL) {
    param_1->next1 = s2;
  } else {
    param_1->next_end->flags = 0;
    if (param_1->next_end->next1 == NULL) {
      param_1->next_end->next1 = s2;
    } else {
      param_1->next_end->next2 = s2;
    }
  }
  // Original: iVar3 = iVar2; if (iVar2->next_end != 0) { iVar3 = iVar2->next_end; }
  // *(int *)(param_1 + 0x10) = iVar3;
  // This requires a temporary variable (corresponding to iVar3) because s2 is still needed in its original form.
  State *temp_s2_end = s2; 
  if (s2->next_end != NULL) {
    temp_s2_end = s2->next_end;
  }
  param_1->next_end = temp_s2_end;

  // Modifications for param_2's outgoing transitions
  param_2->flags = 0;
  if (param_2->next_end == NULL) {
    param_2->next1 = s2;
  } else {
    param_2->next_end->flags = 0;
    if (param_2->next_end->next1 == NULL) {
      param_2->next_end->next1 = s2;
    } else {
      param_2->next_end->next2 = s2;
    }
  }
  // Original: if (*(int *)(iVar2 + 0x10) != 0) { iVar2 = *(int *)(iVar2 + 0x10); }
  // *(int *)(param_2 + 0x10) = iVar2;
  // Here, 's2' (our iVar2) is updated and then used. Since 's2' is not used in its original form
  // after this point, we can modify it directly.
  if (s2->next_end != NULL) {
    s2 = s2->next_end; // Update s2 pointer
  }
  param_2->next_end = s2; // Assign the (potentially updated) s2 to param_2's end

  return s1; // Return the starting state of the union
}

// Function: op_star
State *op_star(State *param_1) { // Changed type to State*
  if (param_1 == NULL) return NULL; // Safety check

  // Original: if (param_1 != *(int *)(param_1 + 0x10))
  // This checks if the start state is not already its own end state (a self-loop)
  if (param_1 != param_1->next_end) {
    State *s1 = create_state(0x80); // Corresponds to original iVar1
    if (s1 == NULL) { // Handle allocation failure
      return NULL;
    }

    param_1->flags = 0;
    if (param_1->next_end == NULL) {
      param_1->next1 = s1;
    } else {
      param_1->next_end->flags = 0;
      if (param_1->next_end->next1 == NULL) {
        param_1->next_end->next1 = s1;
      } else {
        param_1->next_end->next2 = s1;
      }
    }
    // Original: iVar2 = iVar1; if (iVar1->next_end != 0) { iVar2 = iVar1->next_end; }
    // *(int *)(param_1 + 0x10) = iVar2;
    // This requires a temporary variable (corresponding to iVar2) because s1 is still needed in its original form.
    State *temp_s1_end = s1;
    if (s1->next_end != NULL) {
      temp_s1_end = s1->next_end;
    }
    param_1->next_end = temp_s1_end;

    s1->next1 = param_1; // Loop back from new end state to original start state
    s1->next_end = s1; // The new end state loops to itself (Kleene star property)
    s1->flags = 1; // Set flag for the new start state
    param_1 = s1; // The new start state for the starred expression is s1
  }
  return param_1;
}

// Function: op_qmark
State *op_qmark(State *param_1) { // Changed type to State*
  State *s1 = create_state(0x80); // Corresponds to original iVar1
  State *s2 = create_state(0x80); // Corresponds to original iVar2
  
  if (s1 == NULL || s2 == NULL) { // Check if both states were created successfully
    if (s1) free(s1); // Free any successfully created state before returning NULL
    if (s2) free(s2);
    return NULL;
  }

  s1->flags = 0;
  s1->next1 = param_1; // Path from s1 to param_1
  s1->next2 = s2; // Optional path from s1 directly to s2 (the end state)
  s1->next_end = s2; // s2 is the effective end state for s1

  param_1->flags = 0;
  if (param_1->next_end == NULL) {
    param_1->next1 = s2;
  } else {
    param_1->next_end->flags = 0;
    if (param_1->next_end->next1 == NULL) {
      param_1->next_end->next1 = s2;
    } else {
      param_1->next_end->next2 = s2;
    }
  }
  // Original: if (*(int *)(iVar2 + 0x10) != 0) { iVar2 = *(int *)(iVar2 + 0x10); }
  // *(int *)(param_1 + 0x10) = iVar2;
  // Here, 's2' (our iVar2) is updated and then used. Since 's2' is not used in its original form
  // after this point, we can modify it directly.
  if (s2->next_end != NULL) {
    s2 = s2->next_end; // Update s2 pointer
  }
  param_1->next_end = s2; // Assign the (potentially updated) s2 to param_1's end
  
  return s1; // Return the starting state of the question mark expression
}

// Function: op_plus
State *op_plus(State *param_1) { // Changed type to State*
  State *s1 = create_state(0x80); // Corresponds to original iVar1
  if (s1 == NULL) { // Handle allocation failure
    return NULL;
  }

  param_1->flags = 0;
  if (param_1->next_end == NULL) {
    param_1->next1 = s1;
  } else {
    param_1->next_end->flags = 0;
    if (param_1->next_end->next1 == NULL) {
      param_1->next_end->next1 = s1;
    } else {
      param_1->next_end->next2 = s1;
    }
  }
  // Original: iVar2 = iVar1; if (iVar1->next_end != 0) { iVar2 = iVar1->next_end; }
  // *(int *)(param_1 + 0x10) = iVar2;
  // This requires a temporary variable (corresponding to iVar2) because s1 is still needed in its original form.
  State *temp_s1_end = s1;
  if (s1->next_end != NULL) {
    temp_s1_end = s1->next_end;
  }
  param_1->next_end = temp_s1_end;

  s1->next1 = param_1; // Loop back from s1 to param_1 (the "plus" operation)
  // s1->next_end is not explicitly set in the original for op_plus,
  // so it remains NULL as initialized by create_state.

  return param_1; // Return the original start state
}