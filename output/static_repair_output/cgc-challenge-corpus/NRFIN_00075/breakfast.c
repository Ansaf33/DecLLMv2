#include <stdint.h>  // For uint32_t, uint8_t, uint16_t
#include <stddef.h>  // For size_t, NULL
#include <stdbool.h> // For bool (optional, but good practice)

// --- Type Definitions ---
// Original snippet used 'uint' for unsigned 32-bit integers.
typedef uint32_t uint; 
// Original snippet used 'undefined4' for unsigned 32-bit values.
typedef uint32_t undefined4; 
// Original snippet used 'byte' for unsigned 8-bit values.
typedef uint8_t byte; 

// Generic object pointer type for the "objects" managed by the system.
// This is used for objects in liquids_ptrs, cereals_ptrs, toppings_ptrs.
typedef void* ObjectPtr;

// Function pointer types for object methods (destructors and initializers).
// These are inferred from the raw pointer arithmetic and calls in the original code.
typedef void (*ObjectDestructor)(ObjectPtr);
typedef int (*ObjectInitializer)(ObjectPtr, uint, byte); // Initializer for liquids/cereals/toppings

// --- Global Variables (External Declarations) ---
// These arrays are used to store pointers to the dynamically managed objects.
// Their actual memory allocation would be external to this snippet.
ObjectPtr liquids_ptrs[5];
ObjectPtr cereals_ptrs[7];
ObjectPtr toppings_ptrs[5];

// --- External Function Declarations (Dummy implementations for compilation) ---
// These functions are called in the snippet but are not defined within it.
// They are assumed to be defined elsewhere and are declared here to allow compilation.
void init_dispenser(void) { /* Dummy implementation */ }
// The 'send' function's third argument 'in_stack_ffffffdc' appears to be a stack variable
// used as a dummy argument. We'll use size_t and assume 0 for safety.
void send(uint32_t arg1, void* arg2, size_t arg3, int arg4) { /* Dummy implementation */ }
void dispense_bowl(void) { /* Dummy implementation */ }
void completion_buzzer(void) { /* Dummy implementation */ }

// Dispense liquid functions (specific implementations for each liquid type)
void dispense_dairy_milk(void) { /* Dummy implementation */ }
void dispense_dairy_cream(void) { /* Dummy implementation */ }
void dispense_water(void) { /* Dummy implementation */ }
void dispense_soy_milk(void) { /* Dummy implementation */ }
void dispense_almond_milk(void) { /* Dummy implementation */ }

// Dispense cereal functions (specific implementations for each cereal type)
void dispense_sugar_loops(void) { /* Dummy implementation */ }
void dispense_maize_flakes(void) { /* Dummy implementation */ }
void dispense_marshmallow_figuringes(void) { /* Dummy implementation */ }
void dispense_chocolate_rice_pellets(void) { /* Dummy implementation */ }
void dispense_oohs_of_oats(void) { /* Dummy implementation */ }
void dispense_crunchy_puffs(void) { /* Dummy implementation */ }
void dispense_frutiz_n_nuts(void) { /* Dummy implementation */ }

// Dispense topping functions (specific implementations for each topping type)
void dispense_chocolate_drops(void) { /* Dummy implementation */ }
void dispense_blueberries(void) { /* Dummy implementation */ }
void dispense_berry_medley(void) { /* Dummy implementation */ }
void dispense_sugar_cube(void) { /* Dummy implementation */ }
void dispense_strawberries(void) { /* Dummy implementation */ }

// Constructor/Deserializer functions for objects.
// These are assumed to allocate and initialize objects, returning a pointer to them.
ObjectPtr constructor_liquids(void) { /* Dummy implementation */ return NULL; }
ObjectPtr constructor_cereals(void) { /* Dummy implementation */ return NULL; }
// Note: constructor_toppings has specific arguments in its call site.
ObjectPtr constructor_toppings(uint type_param, int init_val) { /* Dummy implementation */ return NULL; } 
// Deserialization functions are expected to read from the input buffer and advance the pointer.
ObjectPtr deserialize_toppings(uint** input_ptr_ref) { /* Dummy implementation */ return NULL; }
ObjectPtr deserialize_liquids(uint** input_ptr_ref) { /* Dummy implementation */ return NULL; }
ObjectPtr deserialize_cereals(uint** input_ptr_ref) { /* Dummy implementation */ return NULL; }
// Function to check inherited types, also expected to advance the input pointer.
int check_inherited_types(uint type_param, uint** input_ptr_ref) { /* Dummy implementation */ return 0; }


// Function: reset_buffers
// Clears all stored liquid, cereal, and topping pointers, calling their destructors.
void reset_buffers(void) {
  // Reset liquids
  for (unsigned int i = 0; i < 5; ++i) {
    if (liquids_ptrs[i] != NULL) {
      ObjectPtr obj = liquids_ptrs[i];
      // Call destructor method, assumed to be at offset 0x14 from object base.
      ((ObjectDestructor)((char*)obj + 0x14))(obj);
      liquids_ptrs[i] = NULL; // Clear the pointer after destruction
    }
  }
  // Reset cereals
  for (unsigned int i = 0; i < 7; ++i) {
    if (cereals_ptrs[i] != NULL) {
      ObjectPtr obj = cereals_ptrs[i];
      // Call destructor method, assumed to be at offset 0x14 from object base.
      ((ObjectDestructor)((char*)obj + 0x14))(obj);
      cereals_ptrs[i] = NULL; // Clear the pointer after destruction
    }
  }
  // Reset toppings
  for (unsigned int i = 0; i < 5; ++i) {
    if (toppings_ptrs[i] != NULL) {
      ObjectPtr obj = toppings_ptrs[i];
      // Call destructor method, assumed to be at offset 0x18 from object base.
      ((ObjectDestructor)((char*)obj + 0x18))(obj);
      toppings_ptrs[i] = NULL; // Clear the pointer after destruction
    }
  }
}

// Function: units_of_liquids
// Counts the number of active liquid units.
int units_of_liquids(void) {
  int count = 0;
  for (unsigned int i = 0; i < 5; ++i) {
    if (liquids_ptrs[i] != NULL) {
      count++;
    }
  }
  return count;
}

// Function: units_of_cereals
// Counts the number of active cereal units.
int units_of_cereals(void) {
  int count = 0;
  for (unsigned int i = 0; i < 7; ++i) {
    if (cereals_ptrs[i] != NULL) {
      count++;
    }
  }
  return count;
}

// Function: units_of_toppings
// Counts the number of active topping units.
int units_of_toppings(void) {
  int count = 0;
  for (unsigned int i = 0; i < 5; ++i) {
    if (toppings_ptrs[i] != NULL) {
      count++;
    }
  }
  return count;
}

// Function: dispense_liquid
// Dispenses a liquid based on the provided ID.
void dispense_liquid(uint param_1) {
  switch(param_1) {
  case 0:
    dispense_dairy_milk();
    break;
  case 1:
    dispense_dairy_cream();
    break;
  case 2:
    dispense_water();
    break;
  case 3:
    dispense_soy_milk();
    break;
  case 4:
    dispense_almond_milk();
    break; // Added missing break statement
  }
}

// Function: dispense_cereal
// Dispenses a cereal based on the provided ID.
void dispense_cereal(uint param_1) {
  switch(param_1) {
  case 0:
    dispense_sugar_loops();
    break;
  case 1:
    dispense_maize_flakes();
    break;
  case 2:
    dispense_marshmallow_figuringes();
    break;
  case 3:
    dispense_chocolate_rice_pellets();
    break;
  case 4:
    dispense_oohs_of_oats();
    break;
  case 5:
    dispense_crunchy_puffs();
    break;
  case 6:
    dispense_frutiz_n_nuts();
    break; // Added missing break statement
  }
}

// Function: dispense_toppings
// Dispenses a topping based on the provided ID.
void dispense_toppings(uint param_1) {
  switch(param_1) {
  case 0:
    dispense_chocolate_drops();
    break;
  case 1:
    dispense_blueberries();
    break;
  case 2:
    dispense_berry_medley();
    break;
  case 3:
    dispense_sugar_cube();
    break;
  case 4:
    dispense_strawberries();
    break; // Added missing break statement
  }
}

// Function: dispense_breakfast
// Orchestrates the dispensing process for a complete breakfast.
void dispense_breakfast(void) {
  // 'in_stack_ffffffdc' from original snippet, appears to be an unused or dummy size_t argument.
  size_t dummy_send_arg = 0; 
  
  init_dispenser();
  send(0x14000, (void *)0x4, dummy_send_arg, 0x11894);
  dispense_bowl();

  if (units_of_cereals() != 0) {
    send(0x14004, (void *)0x4, dummy_send_arg, 0x118b2);
    for (unsigned int i = 0; i < 7; ++i) {
      if (cereals_ptrs[i] != NULL) {
        dispense_cereal(i); // Assuming 'i' is the cereal ID
      }
    }
  }
  
  if (units_of_liquids() != 0) {
    send(0x14008, (void *)0x4, dummy_send_arg, 0x118fe);
    for (unsigned int i = 0; i < 5; ++i) {
      if (liquids_ptrs[i] != NULL) {
        dispense_liquid(i); // Assuming 'i' is the liquid ID
      }
    }
  }
  
  if (units_of_toppings() != 0) {
    send(0x1400c, (void *)0x4, dummy_send_arg, 0x1194a);
    for (unsigned int i = 0; i < 5; ++i) {
      if (toppings_ptrs[i] != NULL) {
        dispense_toppings(i); // Assuming 'i' is the topping ID
      }
    }
  }
  send(0x14010, (void *)0x4, dummy_send_arg, 0x1194a); // Uses the last 'iVar2' value
  completion_buzzer();
}

// Function: process_plain_input
// Processes a plain (non-serialized) input buffer to configure breakfast items.
// param_1 is assumed to be a pointer to an input structure:
// [arbitrary_data (4 bytes)] [data_length (uint16_t at offset 4)] [actual_data_start (at offset 6)]
int process_plain_input(const char* param_1) { 
  ObjectPtr new_obj = NULL;
  int init_result = 0; // Corresponds to 'iVar5' in original, used for constructor return and constructor_toppings arg

  // Calculate start and end pointers for the input data.
  // Input format: [uint16_t header_val_ignored_offset_0_4] [uint16_t data_length_at_offset_4] [data_start_at_offset_6]
  uint* current_input_ptr = (uint*)((char*)param_1 + 6);
  uint* end_input_ptr = (uint*)((char*)current_input_ptr + *(uint16_t*)((char*)param_1 + 4));
  
  reset_buffers();

  while (current_input_ptr < end_input_ptr) {
    uint type = *current_input_ptr;
    // The byte after the type (4 bytes) is the index.
    byte index = *(byte*)((char*)current_input_ptr + sizeof(uint)); 
    // Advance the pointer by 4 bytes for 'type' and 1 byte for 'index'.
    current_input_ptr = (uint*)((char*)current_input_ptr + 5); 

    if (type == 0) { // Liquid item
      if (index > 4) { return -1; } // Index out of bounds
      if (liquids_ptrs[index] != NULL) { return -1; } // Item already exists at this index
      
      new_obj = constructor_liquids();
      if (new_obj == NULL) { return -1; } // Check for allocation failure
      // Call initializer method, assumed to be at offset 0x10.
      init_result = ((ObjectInitializer)((char*)new_obj + 0x10))(new_obj, type, index);
      if (init_result < 0) { // Initialization failed
        ((ObjectDestructor)((char*)new_obj + 0x14))(new_obj); // Call destructor
        new_obj = NULL;
      }
      liquids_ptrs[index] = new_obj;
    } else if (type == 1) { // Cereal item
      if (index > 6) { return -1; } // Index out of bounds
      if (cereals_ptrs[index] != NULL) { return -1; } // Item already exists at this index
      
      new_obj = constructor_cereals();
      if (new_obj == NULL) { return -1; } // Check for allocation failure
      // Call initializer method, assumed to be at offset 0x10.
      init_result = ((ObjectInitializer)((char*)new_obj + 0x10))(new_obj, type, index);
      if (init_result < 0) { // Initialization failed
        ((ObjectDestructor)((char*)new_obj + 0x14))(new_obj); // Call destructor
        new_obj = NULL;
      }
      cereals_ptrs[index] = new_obj;
    } else if (type == 2) { // Topping item
      if (index > 4) { return -1; } // Index out of bounds
      if (toppings_ptrs[index] != NULL) { 
        // Original code breaks the outer loop and returns -1 if topping exists.
        return -1; 
      }
      
      // constructor_toppings takes additional parameters: '2' and the last 'init_result'.
      new_obj = constructor_toppings(2, init_result); 
      if (new_obj == NULL) { return -1; } // Check for allocation failure
      // Call initializer method, assumed to be at offset 0x14.
      init_result = ((ObjectInitializer)((char*)new_obj + 0x14))(new_obj, type, index);
      if (init_result < 0) { // Initialization failed
        ((ObjectDestructor)((char*)new_obj + 0x18))(new_obj); // Call destructor
        new_obj = NULL;
      }
      toppings_ptrs[index] = new_obj;
    } else { // Unknown type encountered
      return -1;
    }
  }
  
  dispense_breakfast(); // All items processed, now dispense.
  return 0; // Success
}

// Function: process_serialized_input
// Processes a serialized input buffer to configure breakfast items.
// param_1 is assumed to be a pointer to an input structure, similar to process_plain_input.
int process_serialized_input(const char* param_1) {
  uint* current_input_ptr = (uint*)((char*)param_1 + 6);
  uint* end_input_ptr = (uint*)((char*)current_input_ptr + *(uint16_t*)((char*)param_1 + 4));
  
  reset_buffers();

  while (current_input_ptr < end_input_ptr) { // Loop until all input is processed
    uint type = *current_input_ptr;
    ObjectPtr new_obj = NULL;

    if (type == 2) { // Topping item
      // deserialize_toppings is expected to read from *current_input_ptr and advance it.
      new_obj = deserialize_toppings(&current_input_ptr); 
      if (new_obj == NULL) { return -1; } // Deserialization failed

      // The index for toppings is assumed to be at offset 0x10 within the deserialized object.
      int index = *(int*)((char*)new_obj + 0x10); 
      if (index < 0 || index >= 5 || toppings_ptrs[index] != NULL) { // Index out of bounds or slot taken
        if (new_obj != NULL) { // Clean up if object was allocated but cannot be stored
            ((ObjectDestructor)((char*)new_obj + 0x18))(new_obj);
        }
        return -1;
      }
      toppings_ptrs[index] = new_obj;
    } else if (type == 0) { // Liquid item
      // deserialize_liquids is expected to read from *current_input_ptr and advance it.
      new_obj = deserialize_liquids(&current_input_ptr); 
      if (new_obj == NULL) { return -1; } // Deserialization failed
      
      // The index for liquids is assumed to be at offset 0xC within the deserialized object.
      int index = *(int*)((char*)new_obj + 0xC);
      if (index < 0 || index >= 5 || liquids_ptrs[index] != NULL) { // Index out of bounds or slot taken
        if (new_obj != NULL) {
            ((ObjectDestructor)((char*)new_obj + 0x14))(new_obj);
        }
        return -1;
      }
      liquids_ptrs[index] = new_obj;
    } else if (type == 1) { // Cereal item
      // deserialize_cereals is expected to read from *current_input_ptr and advance it.
      new_obj = deserialize_cereals(&current_input_ptr); 
      if (new_obj == NULL) { return -1; } // Deserialization failed
      
      // The index for cereals is assumed to be at offset 0xC within the deserialized object.
      int index = *(int*)((char*)new_obj + 0xC);
      if (index < 0 || index >= 7 || cereals_ptrs[index] != NULL) { // Index out of bounds or slot taken
        if (new_obj != NULL) {
            ((ObjectDestructor)((char*)new_obj + 0x14))(new_obj);
        }
        return -1;
      }
      cereals_ptrs[index] = new_obj;
    } else { // Handle inherited types or unknown types
      // check_inherited_types is expected to read from *current_input_ptr and advance it.
      int result = check_inherited_types(type, &current_input_ptr); 
      if (result == -1) {
        return -1; // Error in processing inherited types
      }
    }
  }
  
  dispense_breakfast(); // All items processed, now dispense.
  return 0; // Success
}