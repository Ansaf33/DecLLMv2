#include <stdint.h> // For uint32_t, uint8_t, int32_t, uint64_t
#include <stdlib.h> // For exit, malloc, free, rand
#include <stdio.h>  // For fprintf (used in _terminate and main)
#include <string.h> // For memcpy (if explicit data copying needed, not strictly in original snippet)
#include <unistd.h> // For ssize_t, close (though not used directly in snippet)
#include <sys/socket.h> // For send function

// --- Global Variables and Type Definitions ---

// Global variables implied by the snippet
uint32_t nt; // Type of list: 0 for int, 1 for double
void *num_list; // Pointer to the main list structure
void *sorted_asc_num_list; // Pointer to the sorted list structure

// Dummy Node and List structures for type safety and clarity
typedef struct Node {
    void *data; // Pointer to the actual data (int32_t*, double*)
    // In a real implementation, this would also include pointers to next/previous nodes,
    // data size, data type, etc.
} Node;

typedef struct List {
    Node *head;
    Node *tail;
    // In a real implementation, this would include list size, comparison function, etc.
} List;

// --- Dummy/Placeholder Implementations for External Functions ---

// Dummy _terminate function: prints an error and exits.
void _terminate(int32_t code) {
    fprintf(stderr, "Terminating with code: %d (0x%x)\n", code, (uint32_t)code);
    exit(code);
}

// Dummy recv_all function: simulates receiving data.
// In a real scenario, this would read from a socket or file descriptor.
// For compilation, it pretends to always succeed.
int32_t recv_all(void *buf, size_t size) {
    // For actual testing, you might fill 'buf' with specific values.
    // For example:
    // if (size == 1) { *(uint8_t*)buf = 0; } // Simulate list type 0
    // else if (size == 4) { *(uint32_t*)buf = 5; } // Simulate count 5
    // else if (size == 8) { *(uint64_t*)buf = 1234567890ULL; } // Simulate double/long
    return (int32_t)size; // Assume all bytes are received successfully
}

// Dummy node_create function: associates data with a new node.
// Returns the data_ptr, allowing chaining with list functions.
void* node_create(void *data_ptr) {
    // In a real system, this would create a Node structure,
    // assign data_ptr to node->data, and return the Node*.
    // For this snippet, it's simplified to return the data_ptr itself.
    return data_ptr;
}

// Dummy list_create_dup function: allocates and initializes a new List structure.
List* list_create_dup() {
    List *new_list = (List*)malloc(sizeof(List));
    if (new_list) {
        new_list->head = NULL;
        new_list->tail = NULL;
    }
    return new_list;
}

// Dummy list_insert_sort function: inserts data into a sorted list.
void list_insert_sort(List *list, void *data_ptr, int32_t (*compare_func)(const void*, const void*)) {
    // Placeholder: In a real implementation, this would insert data_ptr
    // into the list while maintaining sorted order using compare_func.
    (void)list; (void)data_ptr; (void)compare_func; // Suppress unused parameter warnings
}

// Dummy list_append function: appends data to a list.
void list_append(List *list, void *data_ptr) {
    // Placeholder: In a real implementation, this would append data_ptr to the end of the list.
    (void)list; (void)data_ptr; // Suppress unused parameter warnings
}

// Dummy functions for the dispatch tables in do_eval
// These functions simulate operations for integer and double lists.
int32_t dummy_op_0() { /* fprintf(stderr, "Dummy Op 0 called\n"); */ return 0; }
int32_t dummy_op_1() { /* fprintf(stderr, "Dummy Op 1 called\n"); */ return 0; }
int32_t dummy_op_2() { /* fprintf(stderr, "Dummy Op 2 called\n"); */ return 0; }
int32_t dummy_op_3() { /* fprintf(stderr, "Dummy Op 3 called\n"); */ return 0; }
int32_t dummy_op_4() { /* fprintf(stderr, "Dummy Op 4 called\n"); */ return 0; }
int32_t dummy_op_5() { /* fprintf(stderr, "Dummy Op 5 called\n"); */ return 0; }
int32_t dummy_op_6() { /* fprintf(stderr, "Dummy Op 6 called\n"); */ return 0; }
int32_t dummy_op_7() { /* fprintf(stderr, "Dummy Op 7 called\n"); */ return 0; }
int32_t dummy_op_8() { /* fprintf(stderr, "Dummy Op 8 called\n"); */ return 0; }
int32_t dummy_op_9() { /* fprintf(stderr, "Dummy Op 9 called\n"); */ return 0; }
int32_t dummy_op_A() { /* fprintf(stderr, "Dummy Op A called\n"); */ return 0; }
int32_t dummy_op_B() { /* fprintf(stderr, "Dummy Op B called\n"); */ return 0; }

// Dispatch tables for do_eval
typedef int32_t (*OpCodeFunc)(void);

OpCodeFunc CODE_TABLE_INT[12] = {
    dummy_op_0, dummy_op_1, dummy_op_2, dummy_op_3,
    dummy_op_4, dummy_op_5, dummy_op_6, dummy_op_7,
    dummy_op_8, dummy_op_9, dummy_op_A, dummy_op_B
};

OpCodeFunc CODE_TABLE_DOUBLE[12] = {
    dummy_op_0, dummy_op_1, dummy_op_2, dummy_op_3,
    dummy_op_4, dummy_op_5, dummy_op_6, dummy_op_7,
    dummy_op_8, dummy_op_9, dummy_op_A, dummy_op_B
};

// --- Fixed Functions ---

// Function: get_list_type
// Reads a single byte to determine the list type (int or double).
// Returns 0 on success, or an error code.
int32_t get_list_type(void) {
    uint8_t type_byte;
    if (recv_all(&type_byte, 1) != 1) {
        _terminate(0xfffffff6); // Error: failed to receive list type
    }
    nt = type_byte; // Store list type in global variable
    return (type_byte < 2) ? 0 : 0xffffffe2; // 0 for valid types (0 or 1), error otherwise
}

// Function: get_num_count
// Reads 4 bytes to determine the number of elements in the list.
void get_num_count(uint32_t *count_ptr) {
    if (recv_all(count_ptr, 4) != 4) {
        _terminate(0xfffffff6); // Error: failed to receive element count
    }
}

// Function: get_op_code
// Reads a single byte to determine the operation code for evaluation.
// Returns 0 on success, or an error code.
int32_t get_op_code(uint32_t *op_code_ptr) {
    uint8_t op_byte;
    if (recv_all(&op_byte, 1) != 1) {
        _terminate(0xfffffff6); // Error: failed to receive op code
    }
    *op_code_ptr = op_byte;
    return (op_byte < 0xc) ? 0 : 0xffffffb0; // 0 for valid op codes (<12), error otherwise
}

// Function: read_double_into_node
// Allocates memory for a double, reads it, and creates a node.
// Returns a pointer to the double data, or terminates on error.
void* read_double_into_node(void) {
    double *value_ptr = (double *)malloc(sizeof(double));
    if (value_ptr == NULL) {
        _terminate(5); // Error: memory allocation failed
    }

    if (recv_all(value_ptr, sizeof(double)) != sizeof(double)) {
        free(value_ptr); // Clean up allocated memory before terminating
        _terminate(0xfffffff6); // Error: failed to receive double data
    }

    return node_create(value_ptr); // Create node and return pointer to its data
}

// Function: read_int_into_node
// Allocates memory for an int32_t, reads it, and creates a node.
// Returns a pointer to the int32_t data, or terminates on error.
void* read_int_into_node(void) {
    int32_t *value_ptr = (int32_t *)malloc(sizeof(int32_t));
    if (value_ptr == NULL) {
        _terminate(5); // Error: memory allocation failed
    }

    if (recv_all(value_ptr, sizeof(int32_t)) != sizeof(int32_t)) {
        free(value_ptr); // Clean up allocated memory before terminating
        _terminate(0xfffffff6); // Error: failed to receive int data
    }

    return node_create(value_ptr); // Create node and return pointer to its data
}

// Function: dup_double_node
// Duplicates a double value from an existing node's data.
// param_1 is a pointer to the void* that holds the actual double data.
// Returns a pointer to the duplicated double data, or terminates on error.
void* dup_double_node(void *param_1) {
    // Dereference param_1 to get the pointer to the source double data
    double *source_data_ptr = *(double**)param_1;
    
    double *new_data_ptr = (double *)malloc(sizeof(double));
    if (new_data_ptr == NULL) {
        _terminate(5); // Error: memory allocation failed
    }

    *new_data_ptr = *source_data_ptr; // Copy the double value

    return node_create(new_data_ptr); // Create node for the duplicated data
}

// Function: dup_int_node
// Duplicates an int32_t value from an existing node's data.
// param_1 is a pointer to the void* that holds the actual int32_t data.
// Returns a pointer to the duplicated int32_t data, or terminates on error.
void* dup_int_node(void *param_1) {
    // Dereference param_1 to get the pointer to the source int32_t data
    int32_t *source_data_ptr = *(int32_t**)param_1;

    int32_t *new_data_ptr = (int32_t *)malloc(sizeof(int32_t));
    if (new_data_ptr == NULL) {
        _terminate(5); // Error: memory allocation failed
    }

    *new_data_ptr = *source_data_ptr; // Copy the int32_t value

    return node_create(new_data_ptr); // Create node for the duplicated data
}

// Function: int32_node_compare
// Compares two int32_t values, returning -1, 0, or 1.
// param_1, param_2 are pointers to void* which in turn point to int32_t values.
int32_t int32_node_compare(const void *param_1, const void *param_2) {
    const int32_t *val1 = *(const int32_t**)param_1;
    const int32_t *val2 = *(const int32_t**)param_2;

    if (*val1 < *val2) {
        return -1; // Equivalent to 0xffffffff
    }
    if (*val1 > *val2) {
        return 1;
    }
    return 0;
}

// Function: dbl64_node_compare
// Compares two double values, returning -1, 0, or 1.
// param_1, param_2 are pointers to void* which in turn point to double values.
int32_t dbl64_node_compare(const void *param_1, const void *param_2) {
    const double *val1 = *(const double**)param_1;
    const double *val2 = *(const double**)param_2;

    if (*val1 < *val2) {
        return -1; // Equivalent to 0xffffffff
    }
    if (*val1 > *val2) {
        return 1;
    }
    return 0;
}

// Function: load_list
// Populates two lists (one main, one sorted) with data based on list type and count.
// Returns 0 on success, or an error code.
int32_t load_list(int32_t num_elements) {
    num_list = list_create_dup();
    if (num_list == NULL) {
        return 5; // Error: failed to create main list
    }
    
    sorted_asc_num_list = list_create_dup();
    if (sorted_asc_num_list == NULL) {
        return 5; // Error: failed to create sorted list
    }
    
    for (int32_t i = 0; i < num_elements; ++i) {
        void *read_data_ptr; // Pointer to data read from input
        void *dup_data_ptr;   // Pointer to duplicated data

        if (nt == 0) { // Integer list type
            read_data_ptr = read_int_into_node();
            // read_int_into_node terminates on error, so NULL check here is defensive
            if (read_data_ptr == NULL) return 5; 

            dup_data_ptr = dup_int_node(&read_data_ptr); // Pass address of pointer
            // dup_int_node terminates on error, so NULL check here is defensive
            if (dup_data_ptr == NULL) return 5;
            
            list_insert_sort((List*)sorted_asc_num_list, read_data_ptr, int32_node_compare);
            list_append((List*)num_list, dup_data_ptr);
        } else if (nt == 1) { // Double list type
            read_data_ptr = read_double_into_node();
            // read_double_into_node terminates on error, so NULL check here is defensive
            if (read_data_ptr == NULL) return 5;

            dup_data_ptr = dup_double_node(&read_data_ptr); // Pass address of pointer
            // dup_double_node terminates on error, so NULL check here is defensive
            if (dup_data_ptr == NULL) return 5;

            list_insert_sort((List*)sorted_asc_num_list, read_data_ptr, dbl64_node_compare);
            list_append((List*)num_list, dup_data_ptr);
        } else {
            return 0xffffffff; // Error: unknown list type (should have been caught by get_list_type)
        }
    }
    return 0; // Success
}

// Function: do_create
// Initializes lists by getting list type, element count, and loading data.
// Returns 0 on success, or an error code.
int32_t do_create(void) {
    if (num_list != NULL) { // Check if lists are already initialized
        return -9; // Error: lists already exist
    }
    
    int32_t status = get_list_type();
    if (status < 0) { // Check for negative error codes from get_list_type
        return status;
    }
    
    uint32_t num_count;
    get_num_count(&num_count);
    
    if (num_count >= 0x3e9) { // 0x3e9 is 1001 decimal; check for count exceeding limit
        return -0x46; // Error: element count too high
    }
    
    return load_list(num_count); // Load the actual list data
}

// Function: do_eval
// Evaluates an operation based on the list type and an operation code.
// Returns the result of the operation, or an error code.
int32_t do_eval(void) {
    if (num_list == NULL) {
        return -10; // Error: lists not initialized
    }
    
    uint32_t op_code;
    int32_t status = get_op_code(&op_code);
    if (status < 0) { // Check for negative error codes from get_op_code
        return status;
    }
    
    if (op_code >= 0xc) { // 0xc is 12 decimal; check for invalid op code
        return -0x65; // Error: invalid operation code
    }

    if (nt == 0) { // Integer list type
        return CODE_TABLE_INT[op_code](); // Call function from integer dispatch table
    } else if (nt == 1) { // Double list type
        return CODE_TABLE_DOUBLE[op_code](); // Call function from double dispatch table
    } else {
        return -0x7a; // Error: unknown list type for evaluation
    }
}

// Function: do_nonce
// Performs a nonce challenge-response exchange.
// Returns 0 on success, or an error code.
int32_t do_nonce(void) {
    // The original code `if (rand() == 0)` implies success if rand() returns 0.
    // This is an unusual condition for a typical PRNG. Assuming it's a specific trigger.
    if (rand() == 0) { // Condition for successful challenge generation
        // Generate a 64-bit challenge value
        uint64_t challenge_val = ((uint64_t)rand() << 32) | (uint32_t)rand();

        // Send the challenge value.
        // Assuming '1' is a dummy socket file descriptor and '0' for flags.
        int sockfd = 1; 
        int flags = 0;
        
        // In a real system, a failed send would typically be handled (e.g., _terminate).
        // The original snippet does not explicitly _terminate on send failure.
        send(sockfd, &challenge_val, sizeof(challenge_val), flags); // Check return value if error handling needed

        // Receive the 64-bit response value
        uint64_t response_val;
        if (recv_all(&response_val, sizeof(response_val)) != sizeof(response_val)) {
            _terminate(0xfffffff6); // Error: failed to receive nonce response
        }
        
        // Compare the sent challenge with the received response
        if (challenge_val == response_val) {
            return 0; // Success: challenge matches response
        } else {
            return 0xffffffd4; // Error: nonce mismatch
        }
    } else {
        return 0xffffffd3; // Error: challenge generation failed (rand() != 0)
    }
}

// --- Main function for compilation and basic testing ---
int main() {
    // Initialize global lists to NULL and nt to a default value
    num_list = NULL;
    sorted_asc_num_list = NULL;
    nt = 0; // Default list type to int

    printf("--- Running do_create ---\n");
    int32_t create_result = do_create();
    printf("do_create result: %d (0x%x)\n", create_result, (uint32_t)create_result);

    printf("\n--- Running do_eval ---\n");
    // For do_eval to work, 'nt' must be set (done by do_create) and
    // 'recv_all' for op_code must be mocked to return a valid code.
    // Assuming 'recv_all' is mocked to return 0 for op_code.
    int32_t eval_result = do_eval();
    printf("do_eval result: %d (0x%x)\n", eval_result, (uint32_t)eval_result);

    printf("\n--- Running do_nonce ---\n");
    // The `rand() == 0` condition in `do_nonce` is very unlikely to be true.
    // This call will likely return 0xffffffd3.
    int32_t nonce_result = do_nonce();
    printf("do_nonce result: %d (0x%x)\n", nonce_result, (uint32_t)nonce_result);

    // Clean up dummy lists if they were allocated
    // (Note: individual node data needs to be freed too in a real app)
    if (num_list) free(num_list);
    if (sorted_asc_num_list) free(sorted_asc_num_list);

    return 0;
}