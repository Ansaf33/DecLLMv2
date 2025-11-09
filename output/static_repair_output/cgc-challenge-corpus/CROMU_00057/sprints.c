#include <stdlib.h> // For calloc, free, exit
#include <string.h> // For strlen, strncpy
#include <stdio.h>  // For stderr, fprintf

// Helper function to replace _terminate()
static void terminate_program(void) {
    fprintf(stderr, "Error: Program terminated due to memory allocation failure or unexpected condition.\n");
    exit(EXIT_FAILURE);
}

// Assuming 64-bit system: sizeof(short)=2, sizeof(int)=4, sizeof(void*)=8.
// The original code's pointer arithmetic and size (0x18 = 24 bytes)
// suggest a 32-bit system where pointers are 4 bytes, or specific packing.
// For robust 64-bit Linux compilation, pointers are 8 bytes.
// This requires adjusting the struct sizes to correctly accommodate pointers.

// Represents an individual sprint node in a doubly linked list.
// Original accesses:
// - `*local_14`: `id`
// - `*(void **)(local_14 + 2)`: `name` (char*)
// - `local_14[4]`, `local_14[5]`: `data_field_1`, `data_field_2` (shorts)
// - `*(int *)(local_10 + 0x10)`: `prev` (Sprint*)
// - `*(int *)(local_10 + 0x14)`: `next` (Sprint*)
// Structure adjusted for 64-bit pointers.
typedef struct Sprint {
    short id;           // Offset 0x00 (2 bytes)
    short _pad0;        // Offset 0x02 (2 bytes padding for 4-byte alignment of name)
    char *name;         // Offset 0x04 (8 bytes for 64-bit char* pointer)
    short data_field_1; // Offset 0x0C (2 bytes, original local_14[4])
    short data_field_2; // Offset 0x0E (2 bytes, original local_14[5])
    struct Sprint *prev; // Offset 0x10 (8 bytes for 64-bit Sprint* pointer)
    struct Sprint *next; // Offset 0x18 (8 bytes for 64-bit Sprint* pointer)
} Sprint; // Total size: 2+2+8+2+2+8+8 = 32 bytes

// Represents an entry in the main list, which can have a sub-list of Sprints.
// Original accesses:
// - `*param_1`: `id`
// - `*(int *)(param_1 + 6)`: `sprint_head` (Sprint*)
// - `*(short **)(param_1 + 8)`: `next` (ListEntry*)
// Structure adjusted for 64-bit pointers.
typedef struct ListEntry {
    short id;               // Offset 0x00 (2 bytes)
    char _pad0[10];         // Offset 0x02 (10 bytes padding to align sprint_head to 8-byte boundary)
    struct Sprint *sprint_head; // Offset 0x0C (8 bytes for 64-bit Sprint* pointer)
    struct ListEntry *next;     // Offset 0x14 (8 bytes for 64-bit ListEntry* pointer)
} ListEntry; // Total size: 2+10+8+8 = 28 bytes

// Represents the data used to create/delete a sprint.
// Original accesses:
// - `*param_2`: `search_id`
// - `param_2[1]`: `sprint_id`
// - `param_2 + 2`: `name_ptr` (char*) (implies `*(char**)((char*)param_2 + 4)`)
typedef struct SprintData {
    short search_id; // Offset 0x00 (2 bytes)
    short sprint_id; // Offset 0x02 (2 bytes)
    char *name_ptr;  // Offset 0x04 (8 bytes for 64-bit char* pointer)
} SprintData; // Total size: 2+2+8 = 12 bytes

// Function: create_sprint
int create_sprint(ListEntry *list_head, SprintData *sprint_data) {
    // Find the ListEntry matching sprint_data->search_id
    for (; list_head != NULL && list_head->id != sprint_data->search_id; list_head = list_head->next) {
    }

    if (list_head == NULL) {
        return -1; // ListEntry not found
    }

    // Allocate memory for the new Sprint node
    Sprint *new_sprint = (Sprint *)calloc(1, sizeof(Sprint));
    if (new_sprint == NULL) {
        terminate_program(); // Handle allocation failure
    }

    new_sprint->id = sprint_data->sprint_id;

    // Allocate memory for the sprint's name and copy it
    size_t name_len = strlen(sprint_data->name_ptr);
    new_sprint->name = (char *)calloc(name_len + 1, sizeof(char));
    if (new_sprint->name == NULL) {
        free(new_sprint); // Free the Sprint node before terminating
        terminate_program();
    }
    strncpy(new_sprint->name, sprint_data->name_ptr, name_len);
    new_sprint->name[name_len] = '\0'; // Ensure null-termination

    // Initialize other fields
    new_sprint->data_field_1 = 0; // Corresponds to local_14[4] = 0
    new_sprint->data_field_2 = 0; // Corresponds to local_14[5] = 0
    new_sprint->prev = NULL;      // Corresponds to *(undefined4 *)(local_14 + 8) = 0
    new_sprint->next = NULL;      // Corresponds to *(undefined4 *)(local_14 + 10) = 0

    // Add the new sprint to the list_head's sub-list
    if (list_head->sprint_head == NULL) {
        list_head->sprint_head = new_sprint;
    } else {
        Sprint *current_sprint = list_head->sprint_head;
        // Find the last sprint in the sub-list
        for (; current_sprint->next != NULL; current_sprint = current_sprint->next) {
        }
        current_sprint->next = new_sprint;
        new_sprint->prev = current_sprint;
    }

    return 0;
}

// Function: delete_sprint
int delete_sprint(ListEntry *list_head, SprintData *sprint_data) {
    // Find the ListEntry matching sprint_data->search_id
    for (; list_head != NULL && list_head->id != sprint_data->search_id; list_head = list_head->next) {
    }

    if (list_head == NULL) {
        return -1; // ListEntry not found
    }

    // Check if the sub-list is empty
    if (list_head->sprint_head == NULL) {
        return -1; // No sprints to delete
    }

    // Check if the head of the sub-list matches sprint_data->sprint_id
    if (list_head->sprint_head->id == sprint_data->sprint_id) {
        Sprint *sprint_to_delete = list_head->sprint_head;
        // If there's a next sprint, make it the new head
        if (sprint_to_delete->next != NULL) {
            sprint_to_delete->next->prev = NULL; // Unlink from deleted head
        }
        list_head->sprint_head = sprint_to_delete->next;

        // Free resources and the sprint node
        if (sprint_to_delete->name != NULL) {
            free(sprint_to_delete->name);
        }
        free(sprint_to_delete);
        return 0;
    } else {
        // Search for the sprint in the rest of the sub-list
        Sprint *prev_sprint = list_head->sprint_head;
        Sprint *current_sprint = list_head->sprint_head->next;

        for (; current_sprint != NULL && current_sprint->id != sprint_data->sprint_id;
             prev_sprint = current_sprint, current_sprint = current_sprint->next) {
        }

        if (current_sprint == NULL) {
            return -1; // Sprint not found
        }

        // Relink the list
        prev_sprint->next = current_sprint->next;
        if (current_sprint->next != NULL) {
            current_sprint->next->prev = prev_sprint;
        }

        // Free resources and the sprint node
        if (current_sprint->name != NULL) {
            free(current_sprint->name);
        }
        free(current_sprint);
        return 0;
    }
}