#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For strlen, memcpy, strcmp, memset
#include <stdbool.h> // For bool

// Simplified custom memory allocation wrappers
// The original `allocate` returned 0 on success, 1 on failure.
// It also often included a 'line number' parameter, which is ignored here.
// The 'alignment' parameter was always 0 and is also ignored.
static int custom_allocate(size_t size, void** dest) {
    *dest = malloc(size);
    if (*dest != NULL) {
        memset(*dest, 0, size); // Equivalent to bzero for new allocations
        return 0; // Success
    }
    return 1; // Failure
}

// The original `deallocate` had a 'size' parameter which is not used by free.
static void custom_deallocate(void* ptr, size_t size_ignored) {
    free(ptr);
}

// Element structure deduced from usage: char* name; int value;
typedef struct Element {
    char* name;
    int value;
} Element;

#define MAX_SET_ELEMENTS 15 // Deduced from `param_1 + 0x18 < 0xf` in add_element_to_set

// Set structure deduced from offsets and usage:
// - `param_1 + 0x8` used for set name/label (char*)
// - `param_1 + 0x18` used for element count (int)
// - `param_1 + 0xc + (index + 4) * 4` used for elements array
//   (this translates to `elements[index]` if array starts at 0x1C)
typedef struct Set {
    // Note: The original code used hardcoded offsets (e.g., param_1 + 0x8, param_1 + 0x18).
    // By defining a struct and using member access, the compiler handles the layout
    // and makes the code portable across architectures (e.g., 32-bit vs 64-bit),
    // as long as the logical relationships (e.g., max elements) are preserved.
    char* name;
    int count;
    Element* elements[MAX_SET_ELEMENTS];
} Set;

// Forward declarations for functions
void free_element(Element* element);
void free_set_array(Set* set);
Element* copy_element(Element* src_element);
int add_element_to_set(Set* set, Element* element);

// Function: print_set
void print_set(Set* set) {
    if (set != NULL) {
        printf("%s = |", set->name);
        for (int i = 0; i < set->count; ++i) {
            Element* element = set->elements[i];
            if (element->value == 0) {
                printf("%s", element->name);
            } else {
                printf("\"%s\"", element->name);
            }
            if (i < set->count - 1) {
                printf(",");
            }
        }
        printf("|\n");
    }
}

// Function: free_element
void free_element(Element* element) {
    if (element != NULL) {
        custom_deallocate(element->name, 0); // Free the name string
        custom_deallocate(element, 0);       // Free the Element struct itself
    }
}

// Function: copy_set
Set* copy_set(Set* src_set) {
    Set* new_set = NULL;
    if (src_set != NULL) {
        if (custom_allocate(sizeof(Set), (void**)&new_set) == 0) { // Allocate Set struct
            if (src_set->name != NULL) {
                size_t name_len = strlen(src_set->name);
                if (custom_allocate(name_len + 1, (void**)&new_set->name) != 0) {
                    custom_deallocate(new_set, 0);
                    return NULL;
                }
                memcpy(new_set->name, src_set->name, name_len + 1);
            }

            for (int i = 0; i < src_set->count; ++i) {
                Element* copied_element = copy_element(src_set->elements[i]);
                if (copied_element == NULL || add_element_to_set(new_set, copied_element) != 0) {
                    if (copied_element != NULL) {
                        free_element(copied_element); // Free element if copied but not added
                    }
                    free_set_array(new_set); // Free partially copied set
                    return NULL;
                }
            }
        }
    }
    return new_set;
}

// Function: free_set_array
void free_set_array(Set* set) {
    if (set != NULL) {
        for (int i = 0; i < set->count; ++i) {
            if (set->elements[i] != NULL) {
                free_element(set->elements[i]);
                set->elements[i] = NULL;
            }
        }
        custom_deallocate(set->name, 0); // Free the set name/label
        custom_deallocate(set, 0);       // Free the Set struct itself
    }
}

// Function: copy_element
Element* copy_element(Element* src_element) {
    Element* new_element = NULL;
    if (src_element != NULL) {
        if (custom_allocate(sizeof(Element), (void**)&new_element) == 0) { // Allocate Element struct
            size_t name_len = strlen(src_element->name);
            if (custom_allocate(name_len + 1, (void**)&new_element->name) == 0) { // Allocate name string
                memcpy(new_element->name, src_element->name, name_len + 1); // Copy with null terminator
                new_element->value = src_element->value;
            } else {
                custom_deallocate(new_element, 0); // Free partially allocated Element
                new_element = NULL;
            }
        }
    }
    return new_element;
}

// Function: element_in_set
bool element_in_set(Set* set, Element* element) {
    if (set != NULL && element != NULL) {
        for (int i = 0; i < set->count; ++i) {
            Element* current_set_element = set->elements[i];
            if (strcmp(element->name, current_set_element->name) == 0 &&
                element->value == current_set_element->value) {
                return true;
            }
        }
    }
    return false;
}

// Function: add_element_to_set
int add_element_to_set(Set* set, Element* element) {
    if (set != NULL && element != NULL && set->count < MAX_SET_ELEMENTS) {
        set->elements[set->count] = element;
        set->count++;
        return 0; // Success (as per original code's return value 0)
    }
    return 1; // Failure (as per original code's return value 1)
}

// Function: create_element
// param_2 in original was void*, checked against 0x0 or 0x1, implying integer values.
Element* create_element(char* name, int value) {
    Element* new_element = NULL;
    if (name != NULL) {
        if (custom_allocate(sizeof(Element), (void**)&new_element) == 0) { // Allocate Element struct
            size_t name_len = strlen(name);
            if (custom_allocate(name_len + 1, (void**)&new_element->name) == 0) { // Allocate name string
                memcpy(new_element->name, name, name_len + 1); // Copy with null terminator
                new_element->value = value;
            } else {
                custom_deallocate(new_element, 0); // Free partially allocated Element
                new_element = NULL;
            }
        }
    }
    return new_element;
}