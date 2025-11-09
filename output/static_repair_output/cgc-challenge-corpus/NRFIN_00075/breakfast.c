#include <stdint.h>   // For uint32_t, intptr_t, etc.
#include <stddef.h>   // For size_t
#include <stdbool.h>  // For bool (though not explicitly used, good practice)
#include <stdio.h>    // For NULL if not already defined

// Define specific types used in the snippet
typedef uint32_t uint;        // `uint` is used for loop counters and `uVar4`.
typedef uint32_t undefined4;  // `undefined4` is used for `param_1` and return values.
typedef uint8_t byte;         // `byte` is used for `bVar2`.

// Function pointer types for object methods
// For liquids/cereals:
// Method at offset 0x10: takes object pointer, type, index. Returns int.
typedef int (*LiquidCerealConstructMethod)(void* obj, undefined4 type, byte index);
// Method at offset 0x14: takes object pointer. Returns void.
typedef void (*LiquidCerealDestroyMethod)(void* obj);

// For toppings:
// Method at offset 0x14: takes object pointer, type, index. Returns int.
typedef int (*ToppingConstructMethod)(void* obj, undefined4 type, byte index);
// Method at offset 0x18: takes object pointer. Returns void.
typedef void (*ToppingDestroyMethod)(void* obj);

// Global arrays to store object pointers.
// Using void* to store generic object pointers, assuming 32-bit architecture
// where intptr_t and void* are interchangeable in size for address calculations.
#define LIQUIDS_COUNT 5
#define CEREALS_COUNT 7
#define TOPPINGS_COUNT 5

void* liquids_ptrs[LIQUIDS_COUNT] = {NULL};
void* cereals_ptrs[CEREALS_COUNT] = {NULL};
void* toppings_ptrs[TOPPINGS_COUNT] = {NULL};

// Placeholder function declarations (externs)
// These functions are external to the provided snippet and must be defined elsewhere.
// Their signatures are inferred from usage in the snippet.
extern void init_dispenser(void);
extern void send(uint32_t param1, void* param2, size_t param3, int param4);
extern void dispense_bowl(void);
extern void completion_buzzer(void);

extern void dispense_dairy_milk(void);
extern void dispense_dairy_cream(void);
extern void dispense_water(void);
extern void dispense_soy_milk(void);
extern void dispense_almond_milk(void);

extern void dispense_sugar_loops(void);
extern void dispense_maize_flakes(void);
extern void dispense_marshmallow_figuringes(void);
extern void dispense_chocolate_rice_pellets(void);
extern void dispense_oohs_of_oats(void);
extern void dispense_crunchy_puffs(void);
extern void dispense_frutiz_n_nuts(void);

extern void dispense_chocolate_drops(void);
extern void dispense_blueberries(void);
extern void dispense_berry_medley(void);
extern void dispense_sugar_cube(void);
extern void dispense_strawberries(void);

// Constructor functions return object pointers.
extern void* constructor_liquids(void);
extern void* constructor_cereals(void);
extern void* constructor_toppings(undefined4 type, int initial_value);

// Deserializer functions take `uint**` (pointer to a pointer to data buffer) and return object pointers.
extern void* deserialize_liquids(uint** pp_data);
extern void* deserialize_cereals(uint** pp_data);
extern void* deserialize_toppings(uint** pp_data);

extern int check_inherited_types(undefined4 type, uint** pp_data);


// Function: reset_buffers
void reset_buffers(void) {
  uint i;
  
  // Liquids
  for (i = 0; i < LIQUIDS_COUNT; ++i) {
    if (liquids_ptrs[i] != NULL) {
      // Call destructor method at offset 0x14 for liquid object
      LiquidCerealDestroyMethod destroy_fn = (LiquidCerealDestroyMethod)((intptr_t)liquids_ptrs[i] + 0x14);
      destroy_fn(liquids_ptrs[i]);
      liquids_ptrs[i] = NULL;
    }
  }
  // Cereals
  for (i = 0; i < CEREALS_COUNT; ++i) {
    if (cereals_ptrs[i] != NULL) {
      // Call destructor method at offset 0x14 for cereal object
      LiquidCerealDestroyMethod destroy_fn = (LiquidCerealDestroyMethod)((intptr_t)cereals_ptrs[i] + 0x14);
      destroy_fn(cereals_ptrs[i]);
      cereals_ptrs[i] = NULL;
    }
  }
  // Toppings
  for (i = 0; i < TOPPINGS_COUNT; ++i) {
    if (toppings_ptrs[i] != NULL) {
      // Call destructor method at offset 0x18 for topping object
      ToppingDestroyMethod destroy_fn = (ToppingDestroyMethod)((intptr_t)toppings_ptrs[i] + 0x18);
      destroy_fn(toppings_ptrs[i]);
      toppings_ptrs[i] = NULL;
    }
  }
}

// Function: units_of_liquids
int units_of_liquids(void) {
  int count = 0;
  for (uint i = 0; i < LIQUIDS_COUNT; ++i) {
    if (liquids_ptrs[i] != NULL) {
      count++;
    }
  }
  return count;
}

// Function: units_of_cereals
int units_of_cereals(void) {
  int count = 0;
  for (uint i = 0; i < CEREALS_COUNT; ++i) {
    if (cereals_ptrs[i] != NULL) {
      count++;
    }
  }
  return count;
}

// Function: units_of_toppings
int units_of_toppings(void) {
  int count = 0;
  for (uint i = 0; i < TOPPINGS_COUNT; ++i) {
    if (toppings_ptrs[i] != NULL) {
      count++;
    }
  }
  return count;
}

// Function: dispense_liquid
void dispense_liquid(undefined4 param_1) {
  switch(param_1) {
  case 0: dispense_dairy_milk(); break;
  case 1: dispense_dairy_cream(); break;
  case 2: dispense_water(); break;
  case 3: dispense_soy_milk(); break;
  case 4: dispense_almond_milk(); break;
  }
}

// Function: dispense_cereal
void dispense_cereal(undefined4 param_1) {
  switch(param_1) {
  case 0: dispense_sugar_loops(); break;
  case 1: dispense_maize_flakes(); break;
  case 2: dispense_marshmallow_figuringes(); break;
  case 3: dispense_chocolate_rice_pellets(); break;
  case 4: dispense_oohs_of_oats(); break;
  case 5: dispense_crunchy_puffs(); break;
  case 6: dispense_frutiz_n_nuts(); break;
  }
}

// Function: dispense_toppings
void dispense_toppings(undefined4 param_1) {
  switch(param_1) {
  case 0: dispense_chocolate_drops(); break;
  case 1: dispense_blueberries(); break;
  case 2: dispense_berry_medley(); break;
  case 3: dispense_sugar_cube(); break;
  case 4: dispense_strawberries(); break;
  }
}

// Function: dispense_breakfast
void dispense_breakfast(void) {
  size_t in_stack_ffffffdc = 0; // Placeholder, as its value is not provided
  int units_count;
  uint i;
  
  init_dispenser();
  send(0x14000, (void *)0x4, in_stack_ffffffdc, 0x11894);
  dispense_bowl();
  
  units_count = units_of_cereals();
  if (units_count != 0) {
    send(0x14004, (void *)0x4, in_stack_ffffffdc, 0x118b2);
    for (i = 0; i < CEREALS_COUNT; ++i) {
      if (cereals_ptrs[i] != NULL) {
        dispense_cereal(i);
      }
    }
  }
  
  units_count = units_of_liquids();
  if (units_count != 0) {
    send(0x14008, (void *)0x4, in_stack_ffffffdc, 0x118fe);
    for (i = 0; i < LIQUIDS_COUNT; ++i) {
      if (liquids_ptrs[i] != NULL) {
        dispense_liquid(i);
      }
    }
  }
  
  units_count = units_of_toppings();
  if (units_count != 0) {
    send(0x1400c, (void *)0x4, in_stack_ffffffdc, 0x1194a);
    for (i = 0; i < TOPPINGS_COUNT; ++i) {
      if (toppings_ptrs[i] != NULL) {
        dispense_toppings(i);
      }
    }
  }
  
  send(0x14010, (void *)0x4, in_stack_ffffffdc, 0x1194a);
  completion_buzzer();
}

// Function: process_plain_input
undefined4 process_plain_input(const void* param_1) {
  char* current_data_byte_ptr;
  char* end_data_byte_ptr;
  void* obj_ptr;
  int result = 0;
  byte item_index;
  uint item_type;
  
  // param_1 + 6 is the start of the data buffer
  current_data_byte_ptr = (char*)((intptr_t)param_1 + 6);
  // param_1 + 4 contains the length (ushort) of the data buffer
  end_data_byte_ptr = (char*)((intptr_t)current_data_byte_ptr + *(uint16_t*)((intptr_t)param_1 + 4));
  
  reset_buffers();
  
  while (true) {
    if (end_data_byte_ptr <= current_data_byte_ptr) {
      dispense_breakfast();
      return 0;
    }
    
    // Read item_type (4 bytes)
    item_type = *(uint*)current_data_byte_ptr;
    // Read item_index (1 byte) immediately following the item_type
    item_index = *(byte*)(current_data_byte_ptr + sizeof(uint));
    
    // Advance pointer by 5 bytes (4 for uint type + 1 for byte index)
    current_data_byte_ptr += 5; 
    
    if (item_type == 2) { // Toppings
      if (item_index >= TOPPINGS_COUNT) {
        return 0xffffffff;
      }
      if (toppings_ptrs[item_index] != NULL) {
        return 0xffffffff;
      }
      
      obj_ptr = constructor_toppings(2, result);
      ToppingConstructMethod construct_fn = (ToppingConstructMethod)((intptr_t)obj_ptr + 0x14);
      result = construct_fn(obj_ptr, item_type, item_index);
      
      if (result < 0) {
        ToppingDestroyMethod destroy_fn = (ToppingDestroyMethod)((intptr_t)obj_ptr + 0x18);
        destroy_fn(obj_ptr);
        obj_ptr = NULL;
      }
      toppings_ptrs[item_index] = obj_ptr;
    } else if (item_type < 2) { // Liquids (0) or Cereals (1)
      if (item_type == 0) { // Liquids
        if (item_index >= LIQUIDS_COUNT) {
          return 0xffffffff;
        }
        if (liquids_ptrs[item_index] != NULL) {
          return 0xffffffff;
        }
        
        obj_ptr = constructor_liquids();
        LiquidCerealConstructMethod construct_fn = (LiquidCerealConstructMethod)((intptr_t)obj_ptr + 0x10);
        result = construct_fn(obj_ptr, item_type, item_index);
        
        if (result < 0) {
          LiquidCerealDestroyMethod destroy_fn = (LiquidCerealDestroyMethod)((intptr_t)obj_ptr + 0x14);
          destroy_fn(obj_ptr);
          obj_ptr = NULL;
        }
        liquids_ptrs[item_index] = obj_ptr;
      } else { // item_type == 1, Cereals
        if (item_index >= CEREALS_COUNT) {
          return 0xffffffff;
        }
        if (cereals_ptrs[item_index] != NULL) {
          return 0xffffffff;
        }
        
        obj_ptr = constructor_cereals();
        LiquidCerealConstructMethod construct_fn = (LiquidCerealConstructMethod)((intptr_t)obj_ptr + 0x10);
        result = construct_fn(obj_ptr, item_type, item_index);
        
        if (result < 0) {
          LiquidCerealDestroyMethod destroy_fn = (LiquidCerealDestroyMethod)((intptr_t)obj_ptr + 0x14);
          destroy_fn(obj_ptr);
          obj_ptr = NULL;
        }
        cereals_ptrs[item_index] = obj_ptr;
      }
    } else { // 2 < item_type -> Invalid type
      return 0xffffffff;
    }
  }
}

// Function: process_serialized_input
undefined4 process_serialized_input(const void* param_1) {
  uint* current_data_ptr;
  uint* end_data_ptr;
  void* obj_ptr;
  uint item_type;
  int check_result;
  
  // param_1 + 6 is the start of the data buffer
  current_data_ptr = (uint*)((intptr_t)param_1 + 6);
  // Calculate end_data_ptr by adding byte offset to the base address (param_1)
  end_data_ptr = (uint*)((char*)((intptr_t)param_1 + 6) + *(uint16_t*)((intptr_t)param_1 + 4));
  
  reset_buffers();
  
  while (true) {
    if (end_data_ptr <= current_data_ptr) {
      dispense_breakfast();
      return 0;
    }
    
    item_type = *current_data_ptr;
    
    if (item_type == 2) { // Toppings
      obj_ptr = deserialize_toppings(&current_data_ptr); 
      if (obj_ptr == NULL) {
        return 0xffffffff;
      }
      
      // The index is at offset 0x10 within the topping object
      byte topping_idx = *(byte*)((intptr_t)obj_ptr + 0x10);
      if (topping_idx >= TOPPINGS_COUNT || toppings_ptrs[topping_idx] != NULL) {
        ToppingDestroyMethod destroy_fn = (ToppingDestroyMethod)((intptr_t)obj_ptr + 0x18);
        destroy_fn(obj_ptr);
        return 0xffffffff;
      }
      toppings_ptrs[topping_idx] = obj_ptr;
    } else if (item_type < 3) { // Liquids (0) or Cereals (1)
      if (item_type == 0) { // Liquids
        obj_ptr = deserialize_liquids(&current_data_ptr);
        if (obj_ptr == NULL) {
          return 0xffffffff;
        }
        
        // The index is at offset 0x0c within the liquid object
        byte liquid_idx = *(byte*)((intptr_t)obj_ptr + 0x0c);
        if (liquid_idx >= LIQUIDS_COUNT || liquids_ptrs[liquid_idx] != NULL) {
          LiquidCerealDestroyMethod destroy_fn = (LiquidCerealDestroyMethod)((intptr_t)obj_ptr + 0x14);
          destroy_fn(obj_ptr);
          return 0xffffffff;
        }
        liquids_ptrs[liquid_idx] = obj_ptr;
      } else { // item_type == 1, Cereals
        obj_ptr = deserialize_cereals(&current_data_ptr);
        if (obj_ptr == NULL) {
          return 0xffffffff;
        }
        
        // The index is at offset 0x0c within the cereal object
        byte cereal_idx = *(byte*)((intptr_t)obj_ptr + 0x0c);
        if (cereal_idx >= CEREALS_COUNT || cereals_ptrs[cereal_idx] != NULL) {
          LiquidCerealDestroyMethod destroy_fn = (LiquidCerealDestroyMethod)((intptr_t)obj_ptr + 0x14);
          destroy_fn(obj_ptr);
          return 0xffffffff;
        }
        cereals_ptrs[cereal_idx] = obj_ptr;
      }
    } else { // item_type >= 3
      check_result = check_inherited_types(item_type, &current_data_ptr);
      if (check_result == -1) {
        return 0xffffffff;
      }
    }
  }
}