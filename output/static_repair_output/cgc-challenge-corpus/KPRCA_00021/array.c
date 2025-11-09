#include <stdlib.h> // Required for malloc, free, realloc
#include <stdio.h>  // Required for printf in the main function

// Define a struct to represent the dynamic integer array.
// This provides a clear and type-safe way to manage array metadata,
// replacing the raw 'undefined4 *' pointer and its implicit structure.
typedef struct {
    int length;        // Current number of elements stored in the array
    int capacity;      // Total allocated capacity for elements
    int *data;         // Pointer to the dynamically allocated integer array
    void (*destructor)(int); // Function pointer for custom element destruction (can be NULL)
} IntArray;

// Function: array_create
// Creates a new dynamic integer array.
// initial_capacity: The initial capacity of the array. If less than 1, a default capacity is used.
// destructor_func: A function pointer to be called on each non-zero element when the array is destroyed.
//                  Pass NULL if no custom destruction is needed for elements.
// Returns a pointer to the newly created IntArray on success, or NULL on failure.
IntArray *array_create(int initial_capacity, void (*destructor_func)(int)) {
    IntArray *arr = (IntArray *)malloc(sizeof(IntArray));
    if (arr == NULL) {
        return NULL; // Failed to allocate memory for the array metadata
    }

    // Use a default capacity if the requested initial_capacity is invalid
    if (initial_capacity < 1) {
        initial_capacity = 16; // Default capacity (0x10 from original snippet)
    }

    // Allocate memory for the actual data elements
    arr->data = (int *)malloc(initial_capacity * sizeof(int));
    if (arr->data == NULL) {
        free(arr); // Free the metadata if data allocation fails
        return NULL;
    }

    arr->length = 0; // Initialize length to 0
    arr->capacity = initial_capacity;
    arr->destructor = destructor_func; // Store the provided destructor function
    return arr;
}

// Function: _array_double_size (static helper function)
// Doubles the allocated capacity of the array.
// Returns 0 on success, or -1 on failure (e.g., realloc fails, capacity limit reached).
static int _array_double_size(IntArray *arr) {
    // Check for potential integer overflow before doubling the capacity
    // 0x7FFFFFFF is MAX_INT. Dividing by 2 then sizeof(int) (usually 4) ensures the multiplication
    // arr->capacity * 2 * sizeof(int) won't overflow.
    if (arr->capacity > (0x7FFFFFFF / 2 / sizeof(int))) {
        return -1; // Capacity too large, would overflow or exceed reasonable limits
    }

    int new_capacity = arr->capacity * 2;

    // Check against the original snippet's explicit maximum element capacity (0x20000000 elements)
    if (new_capacity >= 0x20000000) {
        return -1; // Exceeded maximum allowed elements
    }

    // Reallocate memory for the data array with the new capacity
    int *new_data = (int *)realloc(arr->data, new_capacity * sizeof(int));
    if (new_data == NULL) {
        return -1; // Reallocation failed
    }

    arr->data = new_data;     // Update the data pointer
    arr->capacity = new_capacity; // Update the capacity
    return 0;                 // Success
}

// Function: array_append
// Appends an integer value to the end of the array.
// Returns 0 on success, or -1 on failure (e.g., array is NULL, failed to resize).
int array_append(IntArray *arr, int value) {
    if (arr == NULL) {
        return -1; // Error: Array pointer is NULL
    }

    // If the array is full, attempt to double its size
    if (arr->length == arr->capacity) {
        if (_array_double_size(arr) != 0) {
            return -1; // Error: Failed to resize the array
        }
    }

    // Append the value and increment the length
    arr->data[arr->length] = value;
    arr->length++;
    return 0; // Success
}

// Function: array_get
// Retrieves the integer value at a specific index.
// Returns the value at the specified index.
// If the array is NULL or the index is out of bounds, returns 0 (consistent with original behavior).
int array_get(IntArray *arr, unsigned int index) {
    if (arr == NULL || index >= arr->length) {
        return 0; // Return 0 for error/out of bounds, as per original snippet's behavior
    }
    // Return the value at the given index
    return arr->data[index];
}

// Function: array_length
// Returns the current number of elements in the array.
// Returns 0 if the array is NULL (consistent with original behavior).
int array_length(IntArray *arr) {
    if (arr == NULL) {
        return 0; // Return 0 for a NULL array
    }
    // Return the current length of the array
    return arr->length;
}

// Function: array_destroy
// Frees all memory associated with the array.
// If a destructor function was provided during creation, it is called for each
// non-zero element in the array before freeing the data.
void array_destroy(IntArray *arr) {
    if (arr == NULL) {
        return; // Nothing to destroy if the array pointer is NULL
    }

    if (arr->data != NULL) {
        // If a destructor function is set, call it for each non-zero element
        if (arr->destructor != NULL) {
            for (int i = 0; i < arr->length; i++) {
                // The original snippet only called the destructor for non-zero elements
                if (arr->data[i] != 0) {
                    arr->destructor(arr->data[i]);
                }
            }
        }
        free(arr->data); // Free the memory allocated for the data elements
    }
    free(arr); // Free the memory allocated for the IntArray metadata struct itself
}

// --- Example Usage (main function and a sample destructor) ---

// A sample destructor function for elements.
// This function will be called for each non-zero element when the array is destroyed.
void my_element_destructor(int value) {
    printf("  [Destructor] Destroying element with value: %d\n", value);
}

int main() {
    printf("--- Testing array_create and array_append ---\n");

    // Create an array with an initial capacity of 5 and a custom destructor
    IntArray *my_array = array_create(5, my_element_destructor);
    if (my_array == NULL) {
        fprintf(stderr, "Error: Failed to create array.\n");
        return 1;
    }

    printf("Initial array length: %d, capacity: %d\n", array_length(my_array), my_array->capacity);

    // Append 10 elements to demonstrate resizing
    for (int i = 1; i <= 10; i++) {
        if (array_append(my_array, i * 10) != 0) {
            fprintf(stderr, "Error: Failed to append %d.\n", i * 10);
            array_destroy(my_array); // Clean up before exiting
            return 1;
        }
        printf("Appended %d. Current length: %d, capacity: %d\n", i * 10, array_length(my_array), my_array->capacity);
    }

    printf("\n--- Testing array_get and array_length ---\n");
    printf("Final array length: %d\n", array_length(my_array));
    // Retrieve and print elements, including out-of-bounds access
    for (unsigned int i = 0; i < array_length(my_array) + 2; i++) {
        int value = array_get(my_array, i);
        if (i < array_length(my_array)) {
            printf("Element at index %u: %d\n", i, value);
        } else {
            printf("Element at index %u (out of bounds): %d (expected 0)\n", i, value);
        }
    }

    printf("\n--- Testing array_destroy ---\n");
    // Modify an element to 0 to test the destructor's original behavior
    // (it only calls the destructor for non-zero elements).
    if (my_array->length > 0) {
        my_array->data[0] = 0;
        printf("Set array[0] to 0 to test destructor behavior (no destructor call expected for this element).\n");
    }
    array_destroy(my_array); // Destroy the array, triggering destructor calls
    printf("Array destroyed.\n");

    printf("\n--- Testing array_create without a destructor ---\n");
    // Create an array without a custom destructor
    IntArray *no_destructor_array = array_create(3, NULL);
    if (no_destructor_array == NULL) {
        fprintf(stderr, "Error: Failed to create array without destructor.\n");
        return 1;
    }
    array_append(no_destructor_array, 100);
    array_append(no_destructor_array, 200);
    printf("Array without destructor created. Length: %d\n", array_length(no_destructor_array));
    array_destroy(no_destructor_array); // Destroy this array (no destructor messages expected)
    printf("Array without destructor destroyed (no element destruction messages expected).\n");

    return 0;
}