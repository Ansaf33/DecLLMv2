#include <stdbool.h> // For _Bool
#include <stdint.h>  // For uint32_t, uint8_t, uintptr_t
#include <stddef.h>  // For size_t
#include <string.h>  // For NULL

// Decompiler-generated types mapped to standard C types
typedef long double long_double;
typedef uint32_t uint;
typedef uint32_t undefined4;
typedef uint8_t undefined;
typedef uint8_t byte;

// Global variables from the snippet
// Assuming e_model is a pointer to a load center structure,
// and that pointer arithmetic refers to byte offsets.
// A flexible type like uint8_t* allows byte arithmetic.
uint8_t *e_model = NULL;

// Data constants (assuming these are global data sections from the decompiler)
// Assuming these are long double or float based on usage.
// Providing example values as actual values are unknown.
long_double DAT_00014000 = 240.0L; // Example: Assuming this is a voltage for Watts to Amps conversion
long_double DAT_00014004 = -1.0L; // Placeholder for error/default value
long_double DAT_00014008 = -1.0L; // Placeholder for error/default value
long_double DAT_0001400c = -1.0L; // Placeholder for error/default value
long_double DAT_00014010 = -1.0L; // Placeholder for error/default value
long_double DAT_00014014 = -1.0L; // Placeholder for error/default value
long_double DAT_00014018 = -1.0L; // Placeholder for error/default value

// External/Placeholder functions (their actual implementations are not provided)
// The types are inferred from their usage in the snippet.
extern void* get_first_node(void* list_handle);
extern void* get_list_tail(void* list_handle);
extern uint32_t node_create(void *data); // Returns a handle/ID, not a pointer to the node data itself
extern uint32_t list_append(void* list_handle, uint32_t node_handle); // Returns status, takes handle
extern void deallocate(void *ptr, size_t size);
extern void* get_new_load_center_by_model_id(uint32_t model_id);
extern _Bool get_new_breaker_by_model_id(uint32_t model_id, uint8_t *breaker_address, uint8_t breaker_id_to_assign);
extern void* get_new_outlet_by_model_id(uint32_t model_id);
extern void* get_new_n_way_splitter_by_model_id(uint32_t model_id);
extern void* get_new_light_string_by_model_id(uint32_t model_id);
extern long_double get_max_amps_of_light_string(void);

// Forward declarations for functions defined in the snippet itself
// This is necessary because some functions call others before they are defined.
long_double convert_watts_to_amps(float param_1);
_Bool load_center_is_created(void);
uint32_t load_center_breaker_spaces_are_full(void);
uint32_t get_receptacle_load_type(void* receptacle_ptr);
_Bool receptacle_is_loaded(void* receptacle_ptr);
void* search_receptacle_for_receptacle_id(void* receptacle_ptr, int receptacle_id);
void* search_outlet_for_receptacle_id(void* outlet_ptr, uint32_t receptacle_id);
void* search_splitter_for_receptacle_id(void* splitter_ptr, uint32_t receptacle_id);
void* search_light_string_for_receptacle_id(void* light_string_ptr, uint32_t receptacle_id);
void* search_receptacle_for_splitter_id(void* receptacle_ptr, uint32_t splitter_id);
void* search_light_string_for_splitter_id(void* light_string_ptr, uint32_t splitter_id);
void* search_outlet_for_splitter_id(void* outlet_ptr, uint32_t splitter_id);
void* search_splitter_for_splitter_id(void* splitter_ptr, int splitter_id);
void* search_receptacle_for_light_string_id(void* receptacle_ptr, uint32_t light_string_id);
void* search_light_string_for_light_string_id(void* light_string_ptr, int light_string_id);
void* search_outlet_for_light_string_id(void* outlet_ptr, uint32_t light_string_id);
void* search_splitter_for_light_string_id(void* splitter_ptr, uint32_t light_string_id);
void* get_receptacle_by_id_from_breaker_id(uint32_t breaker_id, uint32_t receptacle_id);
void* get_receptacle_by_id(uint32_t receptacle_id);
void* get_breaker_by_id(int breaker_id);
void* get_outlet_by_id(int outlet_id);
void* get_splitter_by_id(uint32_t splitter_id);
void* get_light_string_by_id(uint32_t light_string_id);
_Bool breaker_id_is_installed(uint32_t breaker_id);
void* get_outlet_list_on_breaker(uint32_t breaker_id);
uint32_t get_count_outlets_on_breaker(uint32_t breaker_id);
uint32_t get_amp_rating_of_breaker(uint32_t breaker_id);
uint32_t get_amp_rating_of_outlet(uint32_t outlet_id);
uint32_t get_amp_rating_of_splitter(uint32_t splitter_id);
long_double get_amp_rating_of_light_string(uint32_t light_string_id);
uint32_t get_amp_rating_of_receptacle(uint32_t receptacle_id);
uint32_t get_number_of_breakers_installed_in_load_center(void);
uint32_t get_total_breaker_space_count(void);
uint32_t get_amp_rating_of_load_center(void);
long_double get_total_amp_load_on_load_center(void);
long_double get_total_amp_load_on_breaker_by_breaker_id(uint32_t breaker_id);
long_double get_total_amp_load_on_outlet_by_outlet_va(void* outlet_ptr);
long_double get_total_amp_load_on_outlet_by_outlet_id(uint32_t outlet_id);
long_double get_total_amp_load_on_receptacle_by_receptacle_va(void* receptacle_ptr);
long_double get_total_amp_load_on_receptacle_by_receptacle_id(uint32_t receptacle_id);
long_double get_total_amp_load_on_splitter_by_splitter_va(void* splitter_ptr);
long_double get_total_amp_load_on_splitter_by_splitter_id(uint32_t splitter_id);
long_double get_total_amp_load_on_light_string_by_light_string_va(void* light_string_ptr);
long_double get_total_amp_load_on_light_string_by_light_string_id(uint32_t light_string_id);
long_double get_max_receptacle_amp_load_on_outlet_by_outlet_va(void* outlet_ptr);
long_double get_max_receptacle_amp_load_on_outlet_by_outlet_id(uint32_t outlet_id);
long_double get_max_receptacle_amp_load_on_splitter_by_splitter_va(void* splitter_ptr);
long_double get_max_receptacle_amp_load_on_splitter_by_splitter_id(uint32_t splitter_id);
uint32_t init_electric_model(uint32_t model_id);
uint32_t add_breaker_to_load_center(uint32_t breaker_model_id, uint32_t *new_breaker_id_out);
uint32_t add_outlet_to_breaker(uint32_t outlet_model_id, uint32_t breaker_id, uint32_t *new_outlet_summary_out);
uint32_t add_n_way_splitter_to_receptacle(uint32_t splitter_model_id, uint32_t receptacle_id, uint32_t *new_splitter_summary_out);
uint32_t add_light_string_to_receptacle(uint32_t light_string_model_id, uint32_t receptacle_id, uint32_t *new_light_string_summary_out);

// Function: convert_watts_to_amps
long_double convert_watts_to_amps(float param_1) {
  return (long_double)param_1 / DAT_00014000;
}

// Function: load_center_is_created
_Bool load_center_is_created(void) {
  return e_model != NULL;
}

// Function: load_center_breaker_spaces_are_full
uint32_t load_center_breaker_spaces_are_full(void) {
  return load_center_is_created() && (*(uint32_t*)e_model == *(uint32_t*)(e_model + 4));
}

// Function: get_receptacle_load_type
uint32_t get_receptacle_load_type(void* receptacle_ptr) {
  return *(uint32_t*)((uint8_t*)receptacle_ptr + 4);
}

// Function: receptacle_is_loaded
_Bool receptacle_is_loaded(void* receptacle_ptr) {
  return get_receptacle_load_type(receptacle_ptr) < 2;
}

// Function: search_receptacle_for_receptacle_id
void* search_receptacle_for_receptacle_id(void* receptacle_ptr, int receptacle_id) {
  if (receptacle_id == *(int*)receptacle_ptr) {
    return receptacle_ptr;
  }

  uint32_t type = get_receptacle_load_type(receptacle_ptr);
  if (type == 0) {
    return search_splitter_for_receptacle_id(*(void**)((uint8_t*)receptacle_ptr + 8), receptacle_id);
  } else if (type == 1) {
    return search_light_string_for_receptacle_id(*(void**)((uint8_t*)receptacle_ptr + 8), receptacle_id);
  }
  return NULL;
}

// Function: search_outlet_for_receptacle_id
void* search_outlet_for_receptacle_id(void* outlet_ptr, uint32_t receptacle_id) {
  void* result = search_receptacle_for_receptacle_id((uint8_t*)outlet_ptr + 4, receptacle_id);
  if (result == NULL) {
    result = search_receptacle_for_receptacle_id((uint8_t*)outlet_ptr + 0x14, receptacle_id);
  }
  return result;
}

// Function: search_splitter_for_receptacle_id
void* search_splitter_for_receptacle_id(void* splitter_ptr, uint32_t receptacle_id) {
  uint8_t count = *(uint8_t*)((uint8_t*)splitter_ptr + 6);
  for (uint32_t i = 0; i < count; ++i) {
    void* result = search_receptacle_for_receptacle_id((uint8_t*)splitter_ptr + i * 0x10 + 8, receptacle_id);
    if (result != NULL) {
      return result;
    }
  }
  return NULL;
}

// Function: search_light_string_for_receptacle_id
void* search_light_string_for_receptacle_id(void* light_string_ptr, uint32_t receptacle_id) {
  // Assuming it should return the result of the search, not void.
  return search_receptacle_for_receptacle_id((uint8_t*)light_string_ptr + 0xc, receptacle_id);
}

// Function: search_receptacle_for_splitter_id
void* search_receptacle_for_splitter_id(void* receptacle_ptr, uint32_t splitter_id) {
  uint32_t type = get_receptacle_load_type(receptacle_ptr);
  if (type == 0) {
    return search_splitter_for_splitter_id(*(void**)((uint8_t*)receptacle_ptr + 8), splitter_id);
  } else if (type == 1) {
    return search_light_string_for_splitter_id(*(void**)((uint8_t*)receptacle_ptr + 8), splitter_id);
  }
  return NULL;
}

// Function: search_light_string_for_splitter_id
void* search_light_string_for_splitter_id(void* light_string_ptr, uint32_t splitter_id) {
  // Assuming it should return the result of the search, not void.
  return search_receptacle_for_splitter_id((uint8_t*)light_string_ptr + 0xc, splitter_id);
}

// Function: search_outlet_for_splitter_id
void* search_outlet_for_splitter_id(void* outlet_ptr, uint32_t splitter_id) {
  void* result = search_receptacle_for_splitter_id((uint8_t*)outlet_ptr + 4, splitter_id);
  if (result == NULL) {
    result = search_receptacle_for_splitter_id((uint8_t*)outlet_ptr + 0x14, splitter_id);
  }
  return result;
}

// Function: search_splitter_for_splitter_id
void* search_splitter_for_splitter_id(void* splitter_ptr, int splitter_id) {
  if (splitter_id == *(int*)splitter_ptr) {
    return splitter_ptr;
  }

  uint8_t count = *(uint8_t*)((uint8_t*)splitter_ptr + 6);
  for (uint32_t i = 0; i < count; ++i) {
    // Corrected the address calculation based on consistency with other splitter searches
    void* result = search_receptacle_for_splitter_id((uint8_t*)splitter_ptr + i * 0x10 + 8, splitter_id);
    if (result != NULL) {
      return result;
    }
  }
  return NULL;
}

// Function: search_receptacle_for_light_string_id
void* search_receptacle_for_light_string_id(void* receptacle_ptr, uint32_t light_string_id) {
  uint32_t type = get_receptacle_load_type(receptacle_ptr);
  if (type == 0) {
    return search_splitter_for_light_string_id(*(void**)((uint8_t*)receptacle_ptr + 8), light_string_id);
  } else if (type == 1) {
    return search_light_string_for_light_string_id(*(void**)((uint8_t*)receptacle_ptr + 8), light_string_id);
  }
  return NULL;
}

// Function: search_light_string_for_light_string_id
void* search_light_string_for_light_string_id(void* light_string_ptr, int light_string_id) {
  if (light_string_id == *(int*)light_string_ptr) {
    return light_string_ptr;
  }
  return search_receptacle_for_light_string_id((uint8_t*)light_string_ptr + 0xc, light_string_id);
}

// Function: search_outlet_for_light_string_id
void* search_outlet_for_light_string_id(void* outlet_ptr, uint32_t light_string_id) {
  void* result = search_receptacle_for_light_string_id((uint8_t*)outlet_ptr + 4, light_string_id);
  if (result == NULL) {
    result = search_receptacle_for_light_string_id((uint8_t*)outlet_ptr + 0x14, light_string_id);
  }
  return result;
}

// Function: search_splitter_for_light_string_id
void* search_splitter_for_light_string_id(void* splitter_ptr, uint32_t light_string_id) {
  uint8_t count = *(uint8_t*)((uint8_t*)splitter_ptr + 6);
  for (uint32_t i = 0; i < count; ++i) {
    void* result = search_receptacle_for_light_string_id((uint8_t*)splitter_ptr + i * 0x10 + 8, light_string_id);
    if (result != NULL) {
      return result;
    }
  }
  return NULL;
}

// Function: get_receptacle_by_id_from_breaker_id
void* get_receptacle_by_id_from_breaker_id(uint32_t breaker_id, uint32_t receptacle_id) {
  void* outlet_list = get_outlet_list_on_breaker(breaker_id);
  void* current_node = get_first_node(outlet_list);
  void* list_tail = get_list_tail(outlet_list);

  while (current_node != list_tail) {
    void* receptacle_ptr = search_outlet_for_receptacle_id(*(void**)current_node, receptacle_id);
    if (receptacle_ptr != NULL) {
      return receptacle_ptr;
    }
    current_node = *(void**)((uint8_t*)current_node + sizeof(void*)); // Assuming next pointer is at offset sizeof(void*)
  }
  return NULL;
}

// Function: get_receptacle_by_id
void* get_receptacle_by_id(uint32_t receptacle_id) {
  if (!load_center_is_created()) {
    return NULL;
  }

  uint32_t num_breakers = *(uint8_t*)(e_model + 1);
  for (uint32_t i = 0; i < num_breakers; ++i) {
    void* result = get_receptacle_by_id_from_breaker_id(i, receptacle_id);
    if (result != NULL) {
      return result;
    }
  }
  return NULL;
}

// Function: get_breaker_by_id
void* get_breaker_by_id(int breaker_id) {
  if (!load_center_is_created() || *(uint8_t*)(e_model + 1) == 0) {
    return NULL;
  }
  // Assuming breaker_id is 0-indexed and refers to an array of breakers.
  // Each breaker is 0xc (12) bytes, starting at offset 8 from e_model.
  return e_model + 8 + (uintptr_t)breaker_id * 0xc;
}

// Function: get_outlet_by_id
void* get_outlet_by_id(int outlet_id) {
  uint32_t num_breakers = get_number_of_breakers_installed_in_load_center();
  if (num_breakers == (uint32_t)-1) { // Check for error return
    return NULL;
  }

  for (uint32_t i = 0; i < num_breakers; ++i) {
    void* outlet_list = get_outlet_list_on_breaker(i);
    void* current_node = get_first_node(outlet_list);
    void* list_tail = get_list_tail(outlet_list);

    while (current_node != list_tail) {
      void* current_outlet_ptr = *(void**)current_node;
      if (outlet_id == *(int*)current_outlet_ptr) {
        return current_outlet_ptr;
      }
      current_node = *(void**)((uint8_t*)current_node + sizeof(void*)); // Assuming next pointer
    }
  }
  return NULL;
}

// Function: get_splitter_by_id
void* get_splitter_by_id(uint32_t splitter_id) {
  uint32_t num_breakers = get_number_of_breakers_installed_in_load_center();
  if (num_breakers == (uint32_t)-1) {
    return NULL;
  }

  for (uint32_t i = 0; i < num_breakers; ++i) {
    void* outlet_list = get_outlet_list_on_breaker(i);
    void* current_node = get_first_node(outlet_list);
    void* list_tail = get_list_tail(outlet_list);

    while (current_node != list_tail) {
      void* splitter_ptr = search_outlet_for_splitter_id(*(void**)current_node, splitter_id);
      if (splitter_ptr != NULL) {
        return splitter_ptr;
      }
      current_node = *(void**)((uint8_t*)current_node + sizeof(void*));
    }
  }
  return NULL;
}

// Function: get_light_string_by_id
void* get_light_string_by_id(uint32_t light_string_id) {
  uint32_t num_breakers = get_number_of_breakers_installed_in_load_center();
  if (num_breakers == (uint32_t)-1) {
    return NULL;
  }

  for (uint32_t i = 0; i < num_breakers; ++i) {
    void* outlet_list = get_outlet_list_on_breaker(i);
    void* current_node = get_first_node(outlet_list);
    void* list_tail = get_list_tail(outlet_list);

    while (current_node != list_tail) {
      void* light_string_ptr = search_outlet_for_light_string_id(*(void**)current_node, light_string_id);
      if (light_string_ptr != NULL) {
        return light_string_ptr;
      }
      current_node = *(void**)((uint8_t*)current_node + sizeof(void*));
    }
  }
  return NULL;
}

// Function: breaker_id_is_installed
_Bool breaker_id_is_installed(uint32_t breaker_id) {
  return get_breaker_by_id(breaker_id) != NULL;
}

// Function: get_outlet_list_on_breaker
void* get_outlet_list_on_breaker(uint32_t breaker_id) {
  void* breaker_ptr = get_breaker_by_id(breaker_id);
  if (breaker_ptr == NULL) {
    return NULL; // Or appropriate error handle
  }
  return *(void**)((uint8_t*)breaker_ptr + 8);
}

// Function: get_count_outlets_on_breaker
uint32_t get_count_outlets_on_breaker(uint32_t breaker_id) {
  void* breaker_ptr = get_breaker_by_id(breaker_id);
  if (breaker_ptr == NULL) {
    return 0xffffffd3; // Error code
  }
  return *(uint32_t*)(*(void**)((uint8_t*)breaker_ptr + 8) + 8);
}

// Function: get_amp_rating_of_breaker
uint32_t get_amp_rating_of_breaker(uint32_t breaker_id) {
  void* breaker_ptr = get_breaker_by_id(breaker_id);
  if (breaker_ptr == NULL) {
    return 0xffffffd3; // Error code
  }
  return *(uint8_t*)((uint8_t*)breaker_ptr + 4);
}

// Function: get_amp_rating_of_outlet
uint32_t get_amp_rating_of_outlet(uint32_t outlet_id) {
  void* outlet_ptr = get_outlet_by_id(outlet_id);
  if (outlet_ptr == NULL) {
    return 0xffffffd1; // Error code
  }
  return *(uint8_t*)((uint8_t*)outlet_ptr + 0x24);
}

// Function: get_amp_rating_of_splitter
uint32_t get_amp_rating_of_splitter(uint32_t splitter_id) {
  void* splitter_ptr = get_splitter_by_id(splitter_id);
  if (splitter_ptr == NULL) {
    return 0xffffffd0; // Error code
  }
  return *(uint8_t*)((uint8_t*)splitter_ptr + 4);
}

// Function: get_amp_rating_of_light_string
long_double get_amp_rating_of_light_string(uint32_t light_string_id) {
  void* light_string_ptr = get_light_string_by_id(light_string_id);
  if (light_string_ptr == NULL) {
    return DAT_00014004; // Error/default value
  }
  return get_max_amps_of_light_string();
}

// Function: get_amp_rating_of_receptacle
uint32_t get_amp_rating_of_receptacle(uint32_t receptacle_id) {
  void* receptacle_ptr = get_receptacle_by_id(receptacle_id);
  if (receptacle_ptr == NULL) {
    return 0xffffffd2; // Error code
  }
  return *(uint8_t*)((uint8_t*)receptacle_ptr + 0xc);
}

// Function: get_number_of_breakers_installed_in_load_center
uint32_t get_number_of_breakers_installed_in_load_center(void) {
  if (!load_center_is_created()) {
    return 0xffffffd6; // Error code
  }
  return *(uint8_t*)(e_model + 1);
}

// Function: get_total_breaker_space_count
uint32_t get_total_breaker_space_count(void) {
  if (!load_center_is_created()) {
    return 0xffffffd6; // Error code
  }
  return *(uint32_t*)e_model;
}

// Function: get_amp_rating_of_load_center
uint32_t get_amp_rating_of_load_center(void) {
  if (!load_center_is_created()) {
    return 0xffffffd6; // Error code
  }
  return *(uint32_t*)(e_model + 4);
}

// Function: get_total_amp_load_on_load_center
long_double get_total_amp_load_on_load_center(void) {
  if (!load_center_is_created()) {
    return DAT_00014008; // Error/default value
  }

  long_double total_load = 0.0L;
  uint32_t num_breakers_installed = get_number_of_breakers_installed_in_load_center();
  if (num_breakers_installed == (uint32_t)-1) { // Check for error return
    return num_breakers_installed; // Propagate error
  }

  for (uint32_t i = 0; i < num_breakers_installed; ++i) {
    total_load += get_total_amp_load_on_breaker_by_breaker_id(i);
  }
  return total_load;
}

// Function: get_total_amp_load_on_breaker_by_breaker_id
long_double get_total_amp_load_on_breaker_by_breaker_id(uint32_t breaker_id) {
  if (!breaker_id_is_installed(breaker_id)) {
    return DAT_0001400c; // Error/default value
  }

  long_double total_load = 0.0L;
  void* outlet_list = get_outlet_list_on_breaker(breaker_id);
  void* current_node = get_first_node(outlet_list);
  void* list_tail = get_list_tail(outlet_list);

  while (current_node != list_tail) {
    total_load += get_total_amp_load_on_outlet_by_outlet_va(*(void**)current_node);
    current_node = *(void**)((uint8_t*)current_node + sizeof(void*));
  }
  return total_load;
}

// Function: get_total_amp_load_on_outlet_by_outlet_va
long_double get_total_amp_load_on_outlet_by_outlet_va(void* outlet_ptr) {
  return get_total_amp_load_on_receptacle_by_receptacle_va((uint8_t*)outlet_ptr + 4) +
         get_total_amp_load_on_receptacle_by_receptacle_va((uint8_t*)outlet_ptr + 0x14);
}

// Function: get_total_amp_load_on_outlet_by_outlet_id
long_double get_total_amp_load_on_outlet_by_outlet_id(uint32_t outlet_id) {
  void* outlet_ptr = get_outlet_by_id(outlet_id);
  if (outlet_ptr == NULL) {
    return DAT_00014010; // Error/default value
  }
  return get_total_amp_load_on_outlet_by_outlet_va(outlet_ptr);
}

// Function: get_total_amp_load_on_receptacle_by_receptacle_va
long_double get_total_amp_load_on_receptacle_by_receptacle_va(void* receptacle_ptr) {
  long_double load = 0.0L;
  uint32_t type = get_receptacle_load_type(receptacle_ptr);
  if (type == 0) {
    load = get_total_amp_load_on_splitter_by_splitter_va(*(void**)((uint8_t*)receptacle_ptr + 8));
  } else if (type == 1) {
    load = get_total_amp_load_on_light_string_by_light_string_va(*(void**)((uint8_t*)receptacle_ptr + 8));
  }
  return load;
}

// Function: get_total_amp_load_on_receptacle_by_receptacle_id
long_double get_total_amp_load_on_receptacle_by_receptacle_id(uint32_t receptacle_id) {
  void* receptacle_ptr = get_receptacle_by_id(receptacle_id);
  if (receptacle_ptr == NULL) {
    return DAT_00014014; // Error/default value
  }
  return get_total_amp_load_on_receptacle_by_receptacle_va(receptacle_ptr);
}

// Function: get_total_amp_load_on_splitter_by_splitter_va
long_double get_total_amp_load_on_splitter_by_splitter_va(void* splitter_ptr) {
  long_double total_load = 0.0L;
  uint8_t count = *(uint8_t*)((uint8_t*)splitter_ptr + 6);
  for (uint32_t i = 0; i < count; ++i) {
    total_load += get_total_amp_load_on_receptacle_by_receptacle_va((uint8_t*)splitter_ptr + i * 0x10 + 8);
  }
  return total_load;
}

// Function: get_total_amp_load_on_splitter_by_splitter_id
long_double get_total_amp_load_on_splitter_by_splitter_id(uint32_t splitter_id) {
  void* splitter_ptr = get_splitter_by_id(splitter_id);
  if (splitter_ptr == NULL) {
    return DAT_00014018; // Error/default value
  }
  return get_total_amp_load_on_splitter_by_splitter_va(splitter_ptr);
}

// Function: get_total_amp_load_on_light_string_by_light_string_va
long_double get_total_amp_load_on_light_string_by_light_string_va(void* light_string_ptr) {
  return convert_watts_to_amps(*(float*)((uint8_t*)light_string_ptr + 8)) +
         get_total_amp_load_on_receptacle_by_receptacle_va((uint8_t*)light_string_ptr + 0xc);
}

// Function: get_total_amp_load_on_light_string_by_light_string_id
long_double get_total_amp_load_on_light_string_by_light_string_id(uint32_t light_string_id) {
  void* light_string_ptr = get_light_string_by_id(light_string_id);
  if (light_string_ptr == NULL) {
    return DAT_00014004; // Error/default value
  }
  return get_total_amp_load_on_light_string_by_light_string_va(light_string_ptr);
}

// Function: get_max_receptacle_amp_load_on_outlet_by_outlet_va
long_double get_max_receptacle_amp_load_on_outlet_by_outlet_va(void* outlet_ptr) {
  long_double load1 = get_total_amp_load_on_receptacle_by_receptacle_va((uint8_t*)outlet_ptr + 4);
  long_double load2 = get_total_amp_load_on_receptacle_by_receptacle_va((uint8_t*)outlet_ptr + 0x14);
  return (load1 > load2) ? load1 : load2;
}

// Function: get_max_receptacle_amp_load_on_outlet_by_outlet_id
long_double get_max_receptacle_amp_load_on_outlet_by_outlet_id(uint32_t outlet_id) {
  void* outlet_ptr = get_outlet_by_id(outlet_id);
  if (outlet_ptr == NULL) {
    return DAT_00014010; // Error/default value
  }
  return get_max_receptacle_amp_load_on_outlet_by_outlet_va(outlet_ptr);
}

// Function: get_max_receptacle_amp_load_on_splitter_by_splitter_va
long_double get_max_receptacle_amp_load_on_splitter_by_splitter_va(void* splitter_ptr) {
  long_double max_load = 0.0L;
  uint8_t count = *(uint8_t*)((uint8_t*)splitter_ptr + 6);
  for (uint32_t i = 0; i < count; ++i) {
    long_double current_load = get_total_amp_load_on_receptacle_by_receptacle_va((uint8_t*)splitter_ptr + i * 0x10 + 8);
    if (current_load > max_load) {
      max_load = current_load;
    }
  }
  return max_load;
}

// Function: get_max_receptacle_amp_load_on_splitter_by_splitter_id
long_double get_max_receptacle_amp_load_on_splitter_by_splitter_id(uint32_t splitter_id) {
  void* splitter_ptr = get_splitter_by_id(splitter_id);
  if (splitter_ptr == NULL) {
    return DAT_00014018; // Error/default value
  }
  return get_max_receptacle_amp_load_on_splitter_by_splitter_va(splitter_ptr);
}

// Function: init_electric_model
uint32_t init_electric_model(uint32_t model_id) {
  if (load_center_is_created()) {
    return 0xffffffd7; // Already created error
  }
  e_model = get_new_load_center_by_model_id(model_id);
  if (e_model == NULL) {
    return 0xffffffd4; // Allocation error
  }
  return 0; // Success
}

// Function: add_breaker_to_load_center
uint32_t add_breaker_to_load_center(uint32_t breaker_model_id, uint32_t *new_breaker_id_out) {
  if (!load_center_is_created()) {
    return 0xffffffd6; // Load center not created
  }
  if (load_center_breaker_spaces_are_full()) {
    return 0xffffffd5; // Breaker spaces full
  }

  uint8_t current_breakers_count = *(uint8_t*)(e_model + 1);
  uint8_t *breaker_slot_address = e_model + 8 + (uintptr_t)current_breakers_count * 0xc;

  _Bool status = get_new_breaker_by_model_id(breaker_model_id, breaker_slot_address, current_breakers_count);
  if (!status) { // Assuming get_new_breaker_by_model_id returns true for success, false for failure
    return 0xffffffd4; // Creation error
  }

  *(uint8_t*)(e_model + 1) = current_breakers_count + 1; // Increment count
  *new_breaker_id_out = current_breakers_count; // Assign new breaker's ID
  return 0; // Success
}

// Function: add_outlet_to_breaker
uint32_t add_outlet_to_breaker(uint32_t outlet_model_id, uint32_t breaker_id, uint32_t *new_outlet_summary_out) {
  if (!load_center_is_created()) {
    return 0xffffffd6; // Load center not created
  }

  void* breaker_ptr = get_breaker_by_id(breaker_id);
  if (breaker_ptr == NULL) {
    return 0xffffffd3; // Breaker not found
  }

  void* new_outlet = get_new_outlet_by_model_id(outlet_model_id);
  if (new_outlet == NULL) {
    return 0xffffffd4; // Allocation error
  }

  // Compare breaker amp rating with outlet amp rating
  if (*(uint8_t*)((uint8_t*)breaker_ptr + 4) < *(uint8_t*)((uint8_t*)new_outlet + 9)) {
    deallocate(new_outlet, 0x28); // Assuming 0x28 is size of outlet structure
    return 0xffffffcd; // Rating mismatch error
  }

  // Copy outlet summary data
  new_outlet_summary_out[0] = *(uint32_t*)new_outlet; // ID
  new_outlet_summary_out[1] = *(uint32_t*)((uint8_t*)new_outlet + 4); // Some other data
  new_outlet_summary_out[2] = *(uint32_t*)((uint8_t*)new_outlet + 0x14); // Another data field
  ((uint8_t*)new_outlet_summary_out)[9] = 2; // Set some flag

  uint32_t node_handle = node_create(new_outlet);
  return list_append(get_outlet_list_on_breaker(breaker_id), node_handle);
}

// Function: add_n_way_splitter_to_receptacle
uint32_t add_n_way_splitter_to_receptacle(uint32_t splitter_model_id, uint32_t receptacle_id, uint32_t *new_splitter_summary_out) {
  if (!load_center_is_created()) {
    return 0xffffffd6; // Load center not created
  }

  void* receptacle_ptr = get_receptacle_by_id(receptacle_id);
  if (receptacle_ptr == NULL) {
    return 0xffffffd2; // Receptacle not found
  }
  if (receptacle_is_loaded(receptacle_ptr)) {
    return 0xffffffce; // Receptacle already loaded
  }

  void* new_splitter = get_new_n_way_splitter_by_model_id(splitter_model_id);
  if (new_splitter == NULL) {
    return 0xffffffd4; // Allocation error
  }

  // Link splitter to receptacle
  *(uint32_t*)((uint8_t*)receptacle_ptr + 4) = 0; // Set receptacle load type to splitter (0)
  *(void**)((uint8_t*)receptacle_ptr + 8) = new_splitter; // Link splitter pointer
  *(uint8_t*)((uint8_t*)receptacle_ptr + 0xc) = *(uint8_t*)((uint8_t*)new_splitter + 1); // Copy amp rating

  // Copy splitter summary data
  new_splitter_summary_out[0] = *(uint32_t*)new_splitter; // ID
  ((uint8_t*)new_splitter_summary_out)[9] = *(uint8_t*)((uint8_t*)new_splitter + 6); // Number of splitter ports
  
  uint8_t splitter_port_count = *(uint8_t*)((uint8_t*)new_splitter + 6);
  for (uint32_t i = 0; i < splitter_port_count; ++i) {
    // This access pattern for new_splitter[i * 4 + 2] is highly suspicious for a decompiler output.
    // It's likely meant to access an array of receptacle pointers.
    // Based on search_splitter_for_receptacle_id, receptacles start at offset 8 and are 0x10 bytes apart.
    // The original code literally copies a uint32_t value from an offset calculated using `new_splitter[i * 4 + 2]`.
    // Assuming new_splitter points to a structure and `new_splitter + (i * 4 + 2)` means
    // `(uint8_t*)new_splitter + (i * 4 + 2) * sizeof(uint32_t)` if `new_splitter` is `uint32_t*`.
    // Or, if new_splitter is `uint8_t*`, then `*(uint32_t*)((uint8_t*)new_splitter + (i * 4 + 2))`.
    // Given the context of `param_3[local_10 + 1] = puVar4[local_10 * 4 + 2];`
    // and `puVar4` being `undefined4*`, it implies `puVar4` is `uint32_t*`.
    // So `puVar4[local_10 * 4 + 2]` would be `*(uint32_t*)(new_splitter + (local_10 * 4 + 2))`.
    // This is still inconsistent with the `0x10` step in search functions.
    // We will use the literal interpretation of the original decompiler output here.
    new_splitter_summary_out[i + 1] = ((uint32_t*)new_splitter)[i * 4 + 2];
  }
  return 0; // Success
}

// Function: add_light_string_to_receptacle
uint32_t add_light_string_to_receptacle(uint32_t light_string_model_id, uint32_t receptacle_id, uint32_t *new_light_string_summary_out) {
  if (!load_center_is_created()) {
    return 0xffffffd6; // Load center not created
  }

  void* receptacle_ptr = get_receptacle_by_id(receptacle_id);
  if (receptacle_ptr == NULL) {
    return 0xffffffd2; // Receptacle not found
  }
  if (receptacle_is_loaded(receptacle_ptr)) {
    return 0xffffffce; // Receptacle already loaded
  }

  void* new_light_string = get_new_light_string_by_model_id(light_string_model_id);
  if (new_light_string == NULL) {
    return 0xffffffd4; // Allocation error
  }

  // Link light string to receptacle
  *(uint32_t*)((uint8_t*)receptacle_ptr + 4) = 1; // Set receptacle load type to light string (1)
  *(void**)((uint8_t*)receptacle_ptr + 8) = new_light_string; // Link light string pointer

  // Copy light string summary data
  new_light_string_summary_out[0] = *(uint32_t*)new_light_string; // ID
  ((uint8_t*)new_light_string_summary_out)[9] = 1; // Set some flag/count for light string type
  new_light_string_summary_out[1] = ((uint32_t*)new_light_string)[3]; // Another data field

  return 0; // Success
}