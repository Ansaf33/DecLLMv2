#include <stdbool.h> // For bool type
#include <stdlib.h>  // For calloc, free, exit
#include <string.h>  // For strncpy, memcpy, strcmp
#include <stdint.h>  // For uint32_t, uintptr_t
#include <stdio.h>   // For fprintf, exit (replacement for _terminate)

// Define constants for buffer sizes
#define NAME_MAX_LEN 127        // Max length for name string (0x7f)
#define DATA_STR_MAX_LEN 127    // Max length for string data (0x7f)
#define DATA_INT_SIZE 4         // Size for integer data (4 bytes)

// Define the structure based on observed offsets and sizes from decompiled code.
// This structure is designed to match a 32-bit memory layout.
// On a 64-bit system, explicit casting with uintptr_t is used for pointer arithmetic
// to ensure correct memory access based on the original 32-bit pointer sizes.
typedef struct VARS_Object {
    char name[NAME_MAX_LEN + 1]; // 0x80 bytes (128) for name (offsets 0x00 - 0x7F)
    char type;                   // 1 byte for type (offset 0x80)
    union {
        char data_str[DATA_STR_MAX_LEN + 1]; // 0x7f bytes (127) for string data (offsets 0x81 - 0xFF)
        uint32_t data_int;                   // 4 bytes for integer data (offsets 0x81 - 0x84)
    } data;
    // Padding to align prev_ptr to offset 0x104.
    // Calculation: 0x104 - (0x80 (name) + 0x1 (type) + 0x7f (data_str)) = 0x104 - 0x100 = 4 bytes.
    char padding[4];
    uint32_t prev_ptr;           // 4 bytes for previous node pointer (offset 0x104)
    uint32_t next_ptr;           // 4 bytes for next node pointer (offset 0x108)
} VARS_Object; // Total size: 0x80 + 0x1 + 0x7f + 0x4 + 0x4 + 0x4 = 0x10c (268 bytes)

// Global variable for the head of the linked list
VARS_Object *pVARS = NULL;

// Function: CreateVARSObject
// Original signature: void * CreateVARSObject(int param_1,char param_2,int param_3)
// Corrected signature based on usage in InitVARS and UpdateVARSObject:
//   - name_src: const char* (used for strncpy)
//   - type_val: char (used for type field)
//   - data_src: const void* (used for strncpy for type 1, memcpy for type 2)
VARS_Object *CreateVARSObject(const char *name_src, char type_val, const void *data_src) {
    if (name_src == NULL || data_src == NULL || (type_val != 1 && type_val != 2)) {
        return NULL;
    }

    VARS_Object *new_obj = calloc(1, sizeof(VARS_Object));
    if (new_obj == NULL) {
        // Original code called _terminate(), replacing with exit(1) for error handling.
        fprintf(stderr, "Error: Memory allocation failed in CreateVARSObject.\n");
        exit(1);
    }

    // Copy name into the object, ensuring null-termination
    strncpy(new_obj->name, name_src, NAME_MAX_LEN);
    new_obj->name[NAME_MAX_LEN] = '\0';

    // Set the object's type
    new_obj->type = type_val;

    // Copy data based on the type
    if (type_val == 1) {
        // For type 1, data_src is treated as a string
        strncpy(new_obj->data.data_str, (const char *)data_src, DATA_STR_MAX_LEN);
        new_obj->data.data_str[DATA_STR_MAX_LEN] = '\0'; // Ensure null-termination
    } else if (type_val == 2) {
        // For type 2, data_src is treated as a pointer to 4 bytes of data (e.g., a uint32_t).
        // If data_src is a string literal (like "CGC CB" from InitVARS), this copies
        // the first 4 bytes of the string's content.
        memcpy(&new_obj->data.data_int, data_src, DATA_INT_SIZE);
    }
    // Initialize linked list pointers to NULL (0 for uint32_t)
    new_obj->prev_ptr = 0;
    new_obj->next_ptr = 0;

    return new_obj;
}

// Function: InitVARS
bool InitVARS(void) {
  // Creates the initial VARS object and sets it as the head of the list.
  // The example uses '1' as type, indicating string data for "CGC CB".
  pVARS = CreateVARSObject("system.name", 1, "CGC CB");
  return pVARS != NULL;
}

// Function: DestroyVARS
// Original return type: undefined4. Corrected to bool for semantic clarity.
bool DestroyVARS(void) {
    if (pVARS == NULL) {
        return false; // No VARS objects to destroy
    }

    VARS_Object *current = pVARS;
    while (current != NULL) {
        // Safely get the next node pointer before freeing the current node.
        // Cast uint32_t to uintptr_t then to VARS_Object* to handle 32-bit pointer values.
        VARS_Object *next_node = (VARS_Object *)(uintptr_t)current->next_ptr;
        free(current);
        current = next_node;
    }
    pVARS = NULL; // Clear the global pointer after all objects are destroyed
    return true;
}

// Function: SearchVARS
// Original return type: char*. Corrected to VARS_Object* as it returns a pointer to the object.
// param_1: char*. Corrected to const char* as it's a search key.
VARS_Object *SearchVARS(const char *name_to_find) {
    if (pVARS == NULL || name_to_find == NULL) {
        return NULL;
    }

    // Iterate through the linked list to find an object by name.
    for (VARS_Object *current = pVARS; current != NULL; current = (VARS_Object *)(uintptr_t)current->next_ptr) {
        if (strcmp(current->name, name_to_find) == 0) {
            return current; // Object found
        }
    }
    return NULL; // Object not found
}

// Function: InsertVARSObject
// Original return type: undefined4. Corrected to bool.
// param_1: int. Corrected to VARS_Object* as it's a pointer to the object to insert.
bool InsertVARSObject(VARS_Object *new_obj) {
    if (new_obj == NULL) {
        return false;
    }

    // Set new_obj's previous pointer to NULL (0 for uint32_t) as it will be the new head.
    new_obj->prev_ptr = 0;
    // New object points to the current head.
    new_obj->next_ptr = (uint32_t)(uintptr_t)pVARS;

    if (pVARS != NULL) {
        // If there was an old head, its previous pointer now points to the new object.
        pVARS->prev_ptr = (uint32_t)(uintptr_t)new_obj;
    }
    pVARS = new_obj; // Update the global head pointer
    return true;
}

// Function: DeleteVARSObject
// Original return type: undefined4. Corrected to bool.
// param_1: char*. Corrected to const char* as it's the name to find and delete.
bool DeleteVARSObject(const char *name_to_delete) {
    if (pVARS == NULL || name_to_delete == NULL) {
        return false;
    }

    VARS_Object *current = pVARS;
    // Search for the object to delete
    while (current != NULL && strcmp(current->name, name_to_delete) != 0) {
        current = (VARS_Object *)(uintptr_t)current->next_ptr;
    }

    if (current == NULL) {
        return false; // Object not found
    }

    // Get pointers to previous and next nodes
    VARS_Object *prev_node = (VARS_Object *)(uintptr_t)current->prev_ptr;
    VARS_Object *next_node = (VARS_Object *)(uintptr_t)current->next_ptr;

    if (current == pVARS) {
        // If deleting the head node
        pVARS = next_node; // The next node becomes the new head
        if (pVARS != NULL) {
            pVARS->prev_ptr = 0; // New head has no previous node
        }
    } else {
        // If deleting a middle or tail node
        if (prev_node != NULL) {
            prev_node->next_ptr = (uint32_t)(uintptr_t)next_node; // Link previous to next
        }
        if (next_node != NULL) {
            next_node->prev_ptr = (uint32_t)(uintptr_t)prev_node; // Link next to previous
        }
    }
    free(current); // Free the memory of the deleted object
    return true;
}

// Function: UpdateVARSObject
// Original return type: int. Corrected to bool.
// param_1: int. Corrected to const char* (name).
// param_2: char. (type_val).
// param_3: char*. Corrected to const void* (data_src).
bool UpdateVARSObject(const char *name, char type_val, const void *data_src) {
    if (name == NULL || data_src == NULL || (type_val != 1 && type_val != 2)) {
        return false;
    }

    VARS_Object *found_obj = SearchVARS(name);

    if (found_obj == NULL) {
        // Object not found, create and insert a new one.
        // The original code had an extra uVar3 argument, which is a decompiler artifact and removed.
        VARS_Object *new_obj = CreateVARSObject(name, type_val, data_src);
        if (new_obj == NULL) {
            return false; // Creation failed
        }
        return InsertVARSObject(new_obj); // Insert the newly created object
    } else {
        // Object found, update its data if the type matches.
        if (type_val == found_obj->type) {
            if (type_val == 1) {
                // Update string data
                strncpy(found_obj->data.data_str, (const char *)data_src, DATA_STR_MAX_LEN);
                found_obj->data.data_str[DATA_STR_MAX_LEN] = '\0'; // Ensure null-termination
            } else if (type_val == 2) {
                // Update integer data (copy 4 bytes)
                memcpy(&found_obj->data.data_int, data_src, DATA_INT_SIZE);
            }
            return true; // Update successful
        } else {
            // Type mismatch, cannot update
            return false;
        }
    }
}