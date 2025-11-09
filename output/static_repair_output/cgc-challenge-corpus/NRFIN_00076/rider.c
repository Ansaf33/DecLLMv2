#include <stdlib.h> // For calloc, free
#include <stdint.h> // For uint32_t

// Forward declaration of Rider struct
typedef struct Rider Rider;

// Function pointer type for rider_health_check
typedef uint32_t (*rider_health_check_func)(Rider*);

// Define the Rider struct based on memory accesses
struct Rider {
    uint32_t field_0;
    uint32_t field_4;
    uint32_t field_8;
    uint32_t field_12;
    uint32_t field_16;
    uint32_t field_20;
    uint32_t field_24;
    rider_health_check_func health_check_fn; // offset 28
    Rider* next; // offset 32 (0x20)
};

// Dummy rider_health_check function
// This function was referenced but not defined in the original snippet.
uint32_t rider_health_check(Rider* rider) {
    (void)rider; // Suppress unused parameter warning
    return 0; // Placeholder return value
}

// Function: rider_new
// Allocates and initializes a new Rider object.
// param_1: Pointer to a Rider* where the newly created Rider object will be stored.
// param_2: An array of uint32_t containing initial values for some fields.
uint32_t rider_new(Rider **param_1, uint32_t *param_2) {
    Rider *new_rider = (Rider *)calloc(1, sizeof(Rider));
    if (new_rider == NULL) {
        return 0xFFFFFFFF; // Return error code on allocation failure
    }

    new_rider->field_0 = param_2[0];
    new_rider->field_4 = param_2[1];
    new_rider->field_8 = param_2[2];
    new_rider->field_12 = new_rider->field_8;
    new_rider->field_24 = param_2[3];
    new_rider->health_check_fn = rider_health_check;

    // Validation check
    if ((new_rider->field_8 == 0) || ((new_rider->field_4 != 0 && (new_rider->field_4 != 1)))) {
        free(new_rider); // Free memory if validation fails
        return 0xFFFFFFFF; // Return error
    } else {
        *param_1 = new_rider; // Store the new rider object pointer
        return 0; // Return success
    }
}

// Function: rider_destroy
// Frees the memory associated with a Rider object and sets its pointer to NULL.
// param_1: Pointer to a Rider* that points to the Rider object to be destroyed.
void rider_destroy(Rider **param_1) {
    free(*param_1);
    *param_1 = NULL;
}

// Function: rider_pop
// Removes the first Rider object from a linked list and returns it.
// param_1: Pointer to the head of the Rider linked list.
Rider* rider_pop(Rider **param_1) {
    Rider* popped_rider = *param_1;
    if (popped_rider != NULL) {
        *param_1 = popped_rider->next; // Update list head to the next rider
        popped_rider->next = NULL;     // Detach the popped rider from the list
    }
    return popped_rider;
}

// Function: rider_destroy_list
// Destroys all Rider objects in a linked list.
// param_1: Pointer to the head of the Rider linked list to be destroyed.
void rider_destroy_list(Rider **param_1) {
    Rider* current_rider;
    while (*param_1 != NULL) { // While the list is not empty
        current_rider = rider_pop(param_1); // Pop the current head, updating the list head
        rider_destroy(&current_rider);       // Destroy the popped rider
    }
    // *param_1 is already NULL by the end of the loop due to rider_pop
}

// Function: rider_append
// Appends a Rider object to the end of a linked list.
// param_1: Pointer to the head of the Rider linked list.
// param_2: The Rider object to be appended.
void rider_append(Rider **param_1, Rider *param_2) {
    Rider** current_ptr = param_1; // Pointer to the 'next' field being examined (starts at list head)

    // Traverse to the last 'next' pointer in the list
    while (*current_ptr != NULL) {
        current_ptr = &(*current_ptr)->next;
    }

    // Append the new rider
    *current_ptr = param_2;
    param_2->next = NULL; // Ensure the appended rider's next pointer is NULL
}

// Function: rider_reset
// Resets specific fields of a Rider object.
// param_1: The Rider object to reset.
void rider_reset(Rider *param_1) {
    param_1->field_8 = param_1->field_12;
    param_1->field_16 = 0;
    param_1->field_20 = 0;
}