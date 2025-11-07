#include <stdbool.h> // For bool type
#include <stdlib.h>  // For calloc, free, exit
#include <string.h>  // For strncpy, memcpy, strcmp
#include <stdio.h>   // For fprintf

// This code is reverse-engineered from a snippet that implies a 32-bit
// compilation environment where pointers are 4 bytes. The `VARSObject`
// structure is designed to match the original binary's memory layout
// with a total size of 0x10C (268 bytes). If compiled on a 64-bit system,
// pointer sizes (8 bytes) would cause misalignment unless specific
// compiler attributes (e.g., `__attribute__((packed))`) are used,
// which are non-standard.

// Offsets within VARSObject:
// 0x00 - 0x7F: name (128 bytes)
// 0x80: type (1 byte)
// 0x81 - 0x100: data union (max 128 bytes for string, including null)
// 0x101 - 0x103: padding (3 bytes)
// 0x104 - 0x107: prev pointer (4 bytes)
// 0x108 - 0x10B: next pointer (4 bytes)
// Total size: 128 + 1 + 128 + 3 + 4 + 4 = 268 bytes (0x10C)

#define VARS_NAME_SIZE 128          // Size of the name buffer (including null terminator)
#define VARS_STRING_DATA_MAX_LEN 127 // Max characters for string data (excluding null terminator)

// Forward declaration for linked list pointers
typedef struct VARSObject VARSObject;

struct VARSObject {
    char name[VARS_NAME_SIZE];      // 0x00 - 0x7F
    char type;                      // 0x80
    union {
        char s_data[VARS_STRING_DATA_MAX_LEN + 1]; // 0x81 - 0x100 (127 chars + null, total 128 bytes)
        int i_data;                                 // 0x81 - 0x84 (4 bytes)
    } data;
    char _padding[3]; // Explicit padding to align `prev` at 0x104
    VARSObject *prev;               // 0x104
    VARSObject *next;               // 0x108
};

// Global variable for the head of the linked list
VARSObject *pVARS = NULL;

// Helper function for fatal errors (e.g., memory allocation failure)
void _terminate(void) {
    fprintf(stderr, "Fatal error: Memory allocation failed.\n");
    exit(EXIT_FAILURE);
}

// Function: CreateVARSObject
// Allocates and initializes a new VARSObject.
// `name`: The name string for the object.
// `type`: '1' for string data, '2' for integer data.
// `value`: Pointer to the data (char* for string, int* for integer).
VARSObject *CreateVARSObject(const char *name, char type, const void *value) {
  if ((name == NULL) || (value == NULL) || ((type != 1) && (type != 2))) {
    return NULL;
  }
  
  VARSObject *new_obj = (VARSObject *)calloc(1, sizeof(VARSObject));
  if (new_obj == NULL) {
    _terminate(); // Call _terminate on memory allocation failure
  }
  
  strncpy(new_obj->name, name, VARS_NAME_SIZE - 1);
  new_obj->name[VARS_NAME_SIZE - 1] = '\0'; // Ensure null termination

  new_obj->type = type;

  if (type == 1) { // String type
    strncpy(new_obj->data.s_data, (const char *)value, VARS_STRING_DATA_MAX_LEN);
    new_obj->data.s_data[VARS_STRING_DATA_MAX_LEN] = '\0'; // Ensure null termination
  } else if (type == 2) { // Integer type
    memcpy(&(new_obj->data.i_data), value, sizeof(int)); // Copy 4 bytes for integer
  }
  
  // `prev` and `next` pointers are already NULL due to `calloc`
  return new_obj;
}

// Function: InitVARS
// Initializes the global pVARS object with a default entry.
bool InitVARS(void) {
  // Original call: CreateVARSObject("system.name",1,"CGC CB");
  // This implies param_3 is a string when param_2 is 1.
  pVARS = CreateVARSObject("system.name", 1, "CGC CB");
  return pVARS != NULL;
}

// Function: DestroyVARS
// Frees all VARSObjects in the linked list and resets pVARS.
bool DestroyVARS(void) {
  if (pVARS == NULL) {
    return false; // No VARS object to destroy
  }
  
  VARSObject *current_obj = pVARS;
  while (current_obj != NULL) {
    VARSObject *next_obj = current_obj->next; // Save next pointer before freeing current
    free(current_obj);
    current_obj = next_obj;
  }
  pVARS = NULL; // Clear the global pointer after destroying all objects
  return true;
}

// Function: SearchVARS
// Searches for a VARSObject by its name.
// Returns a pointer to the found object, or NULL if not found.
VARSObject *SearchVARS(const char *name) {
  if ((pVARS == NULL) || (name == NULL)) {
    return NULL;
  }
  
  VARSObject *current_obj = pVARS;
  while (current_obj != NULL) {
    if (strcmp(current_obj->name, name) == 0) {
      return current_obj;
    }
    current_obj = current_obj->next;
  }
  return NULL;
}

// Function: InsertVARSObject
// Inserts a new VARSObject at the head of the linked list.
// Note: This function expects pVARS to be non-NULL (i.e., the list already
// has at least one element). If pVARS is NULL, it returns false,
// consistent with the original binary's logic.
bool InsertVARSObject(VARSObject *obj_to_insert) {
  if ((pVARS == NULL) || (obj_to_insert == NULL)) {
    return false;
  }
  
  obj_to_insert->next = pVARS;
  pVARS->prev = obj_to_insert;
  pVARS = obj_to_insert; // New object becomes the head
  
  return true;
}

// Function: DeleteVARSObject
// Deletes a VARSObject by its name from the linked list.
bool DeleteVARSObject(const char *name_to_delete) {
  if ((pVARS == NULL) || (name_to_delete == NULL)) {
    return false;
  }
  
  VARSObject *obj_to_delete = pVARS;
  
  // Find the object to delete
  while (obj_to_delete != NULL && strcmp(obj_to_delete->name, name_to_delete) != 0) {
    obj_to_delete = obj_to_delete->next;
  }

  if (obj_to_delete == NULL) {
    return false; // Object not found
  }

  // Found the object, now delete it
  if (obj_to_delete == pVARS) {
    // Deleting the head of the list
    pVARS = obj_to_delete->next;
    if (pVARS != NULL) { // If there's a new head, its prev should be NULL
      pVARS->prev = NULL;
    }
  } else {
    // Deleting an intermediate or tail object
    obj_to_delete->prev->next = obj_to_delete->next;
    if (obj_to_delete->next != NULL) {
      obj_to_delete->next->prev = obj_to_delete->prev;
    }
  }
  
  free(obj_to_delete);
  return true;
}

// Function: UpdateVARSObject
// Updates an existing VARSObject or creates a new one if not found.
// `name`: The name of the object to update or create.
// `type`: '1' for string data, '2' for integer data.
// `value`: Pointer to the new data.
bool UpdateVARSObject(const char *name, char type, const void *value) {
  if ((name == NULL) || (value == NULL) || ((type != 1) && (type != 2))) {
    return false;
  }
  
  VARSObject *found_obj = SearchVARS(name);
  
  if (found_obj == NULL) {
    // Object not found, create and attempt to insert it
    VARSObject *new_obj = CreateVARSObject(name, type, value);
    if (new_obj == NULL) {
      return false; // Creation failed (e.g., due to invalid input)
    }
    
    // As per original logic, InsertVARSObject returns false if pVARS is NULL.
    // This means UpdateVARSObject cannot add the very first element if the list is empty.
    // InitVARS is responsible for the initial element.
    if (!InsertVARSObject(new_obj)) {
        free(new_obj); // Clean up if insertion failed
        return false;
    }
  } else if (type == found_obj->type) {
    // Object found and type matches, update its value
    if (type == 1) { // String type
      strncpy(found_obj->data.s_data, (const char *)value, VARS_STRING_DATA_MAX_LEN);
      found_obj->data.s_data[VARS_STRING_DATA_MAX_LEN] = '\0'; // Ensure null termination
    } else if (type == 2) { // Integer type
      memcpy(&(found_obj->data.i_data), value, sizeof(int));
    }
  } else {
    // Object found but type does not match, cannot update
    return false;
  }
  
  return true;
}