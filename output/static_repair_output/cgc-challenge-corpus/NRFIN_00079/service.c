#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // For read/write, exit
#include <stdint.h>   // For uint32_t, uint8_t

// Ghidra type aliases
typedef unsigned int uint;
typedef uint32_t undefined4;
typedef void undefined; // Used for generic pointers
typedef uint8_t byte;
typedef void (*code)(void); // For function pointers

// --- Global Variables ---
void* search_map = NULL; // Array of SearchEntry structs
void* person_list = NULL; // Head of a linked list of Person data blocks
unsigned int total_person_count = 0;

// Command and result codes (assuming 4-byte unsigned integers)
// These will be initialized in gen_result_bufs.
uint32_t OK_CODE;
uint32_t ERR_CODE;

uint32_t ADD_PERSON_CMD;
uint32_t SET_BIOLOGICAL_CHILD_CMD;
uint32_t SET_ADOPTED_CHILD_CMD;
uint32_t SET_UNION_CMD;
uint32_t SET_DECEASED_CMD;
uint32_t SET_SEPARATED_CMD;
uint32_t ARE_RELATED_CMD;
uint32_t DEGREES_CMD;

// --- Mock Functions ---
// _terminate: Exits the program with a status code
void _terminate(unsigned int status) {
    fprintf(stderr, "Program terminated with status: 0x%x\n", status);
    exit((int)status);
}
void _terminate_no_arg(void) {
    fprintf(stderr, "Program terminated.\n");
    exit(1);
}

// recv_all: Reads 'count' bytes into 'buffer'. Returns bytes read or error.
// Mock implementation: Reads from stdin.
int recv_all(void* buffer, size_t count) {
    size_t total_read = 0;
    while (total_read < count) {
        ssize_t bytes_read = read(STDIN_FILENO, (char*)buffer + total_read, count - total_read);
        if (bytes_read <= 0) {
            fprintf(stderr, "recv_all error or EOF\n");
            return -1; // Error or EOF
        }
        total_read += bytes_read;
    }
    return (int)total_read;
}

// send_data: Sends 'count' bytes from 'buffer'. Returns bytes sent or error.
// Mock implementation: Writes to stdout.
int send_data(void* buffer, size_t count, int flags) { // Renamed to send_data to avoid conflict with standard send()
    size_t total_sent = 0;
    while (total_sent < count) {
        ssize_t bytes_sent = write(STDOUT_FILENO, (const char*)buffer + total_sent, count - total_sent);
        if (bytes_sent <= 0) {
            fprintf(stderr, "send_data error\n");
            return -1; // Error
        }
        total_sent += bytes_sent;
    }
    return (int)total_sent;
}

// new_person: Mocks creation of a new person, returns a pointer to the data block.
// Allocates 0x24 bytes as seen in cmd_add_person, and assigns a unique ID.
void* new_person(void) {
    void* person_data = calloc(1, 0x24); // Size 0x24 from cmd_add_person
    if (!person_data) {
        return NULL;
    }
    static int next_id = 1;
    *(int*)person_data = next_id++; // Assign a unique ID at offset 0
    return person_data;
}

// add_person_to_list: Mocks adding a person to the global linked list.
// The list is managed by `person_list` (head pointer) and `0x104` offset for next pointer.
void add_person_to_list(void* new_person_ptr) {
    if (!new_person_ptr) return;
    // The previous head becomes the next element of the new person
    *(void**)((char*)new_person_ptr + 0x104) = person_list;
    // The new person becomes the head of the list
    person_list = new_person_ptr;
}

// get_person_by_id: Mocks retrieving a person's pointer by their ID from the linked list.
void* get_person_by_id(int id, void* list_head) {
    void* current = list_head;
    while (current != NULL) {
        if (*(int*)current == id) {
            return current;
        }
        current = *(void**)((char*)current + 0x104); // Assuming 0x104 is the 'next' pointer offset
    }
    return NULL; // Not found
}

// --- Structure Definitions ---
typedef struct SearchEntry {
    int person_id;
    uint lock_flags;
} SearchEntry;

// --- Helper Macros for Person struct access ---
// These macros interpret a `void* person_ptr` as a pointer to a byte array,
// allowing access to fields at specific byte offsets.
// This mirrors the low-level pointer arithmetic in the original code.

// Access person ID (first 4 bytes)
#define GET_PERSON_ID(person_ptr) (*(int*)(person_ptr))

// Access a 4-byte integer field at a specific byte offset
#define GET_INT_FIELD(person_ptr, offset) (*(int*)((char*)(person_ptr) + (offset)))
#define SET_INT_FIELD(person_ptr, offset, val) (*(int*)((char*)(person_ptr) + (offset)) = (val))

// Access a 4-byte unsigned integer field at a specific byte offset
#define GET_UINT_FIELD(person_ptr, offset) (*(uint*)((char*)(person_ptr) + (offset)))
#define SET_UINT_FIELD(person_ptr, offset, val) (*(uint*)((char*)(person_ptr) + (offset)) = (val))

// Access a pointer field (void*) at a specific byte offset
// This assumes sizeof(void*) is consistent with the original binary's memory layout.
// On 64-bit systems, `void*` is 8 bytes. The original code's `undefined4*` uses 4-byte units.
// Using `void**` for pointers and `sizeof(uint32_t)` for offsets maintains compatibility with 4-byte units for indexing.
#define GET_PTR_FIELD(person_ptr, offset) (*(void**)((char*)(person_ptr) + (offset)))
#define SET_PTR_FIELD(person_ptr, offset, val) (*(void**)((char*)(person_ptr) + (offset)) = (val))

// --- Person Data Block Offsets (byte offsets) ---
// Biological children (10 slots, each 8 bytes: ID + Ptr)
#define BIO_CHILD_ID_OFFSET(i) ((14 + (i)) * 8 + 8)
#define BIO_CHILD_PTR_OFFSET(i) ((14 + (i)) * 8 + 12)

// Adopted children (10 slots, each 8 bytes: ID + Ptr)
#define ADOPT_CHILD_ID_OFFSET(i) ((4 + (i)) * 8 + 8)
#define ADOPT_CHILD_PTR_OFFSET(i) ((4 + (i)) * 8 + 12)

// Biological Mother (ID + Ptr)
#define BIO_MOTHER_ID_OFFSET 200 // 0xc8 hex
#define BIO_MOTHER_PTR_OFFSET 0xcc

// Biological Father (ID + Ptr)
#define BIO_FATHER_ID_OFFSET 0xd0
#define BIO_FATHER_PTR_OFFSET 0xd4

// Adopting Parents (2 slots, each 8 bytes: ID + Ptr)
#define ADOPT_PARENT_ID_OFFSET(i) ((26 + (i)) * 8 + 8)
#define ADOPT_PARENT_PTR_OFFSET(i) ((26 + (i)) * 8 + 12)

// Union Partner (ID + Ptr)
#define UNION_ID_OFFSET (58 * 4) // 0x3a * 4 = 232
#define UNION_PTR_OFFSET (59 * 4) // 0x3b * 4 = 236

// Separated Partners (2 slots, each 8 bytes: ID + Ptr)
#define SEP_PARTNER1_ID_OFFSET (62 * 4) // 0x3e * 4 = 248
#define SEP_PARTNER1_PTR_OFFSET (63 * 4) // 0x3f * 4 = 252

#define SEP_PARTNER2_ID_OFFSET (64 * 4) // 0x40 * 4 = 256
#define SEP_PARTNER2_PTR_OFFSET (65 * 4) // 0x41 * 4 = 260

// Next person in list (for global person_list iteration)
#define NEXT_LIST_PTR_OFFSET 0x104

// --- Degrees of Separation Relationship Pointers (byte offsets) ---
// These are the actual pointers to other Person data blocks, stored at specific offsets.
// The original code used `param_1[OFFSET_IN_UINT_UNITS]`.
// So the byte offset is `OFFSET_IN_UINT_UNITS * sizeof(uint32_t)`.
// And the value at this byte offset is a `void*` (pointer to another Person).
#define REL_PTR_OFS(uint_offset) ((uint_offset) * sizeof(uint32_t))

#define REL_PTR_ADOPTED_CHILD(i) REL_PTR_OFS( (i + 4) * 2 + 3 ) // i=0..9 -> 11, 13, ..., 29
#define REL_PTR_BIO_CHILD(i)     REL_PTR_OFS( (i + 0xe) * 2 + 3 ) // i=0..9 -> 31, 33, ..., 49

#define REL_PTR_SPOUSE          REL_PTR_OFS(0x33)
#define REL_PTR_BIO_MOTHER      REL_PTR_OFS(0x35)
#define REL_PTR_BIO_FATHER      REL_PTR_OFS(0x37)
#define REL_PTR_ADOPTING_PARENT1 REL_PTR_OFS(0x39)
#define REL_PTR_ADOPTING_PARENT2 REL_PTR_OFS(0x3b)
#define REL_PTR_SEP_PARTNER1    REL_PTR_OFS(0x3d)
#define REL_PTR_SEP_PARTNER2    REL_PTR_OFS(0x3f)

// --- Lock Flags for degrees_of_separation ---
#define LOCK_FLAG_ADOPTED_CHILDREN_MASK 0x10
#define LOCK_FLAG_BIO_CHILDREN_MASK     0x20
#define LOCK_FLAG_SPOUSE_MASK           0x02
#define LOCK_FLAG_BIO_MOTHER_MASK       0x04
#define LOCK_FLAG_BIO_FATHER_MASK       0x08
#define LOCK_FLAG_ADOPTING_PARENTS_MASK 0x40
#define LOCK_FLAG_SEPARATED_PARTNERS_MASK 0x80

// --- Function Implementations ---

// Function: is_search_locked
uint is_search_locked(int person_id, uint lock_mask) {
    for (uint i = 0; i < total_person_count; ++i) {
        SearchEntry* current_entry = &((SearchEntry*)search_map)[i];
        if (person_id == current_entry->person_id) {
            return lock_mask & current_entry->lock_flags;
        }
    }
    _terminate(0xfffffff2); // Person not found in search_map
    return 0; // Unreachable
}

// Function: set_search_lock
void set_search_lock(int person_id, uint lock_mask) {
    for (uint i = 0; i < total_person_count; ++i) {
        SearchEntry* current_entry = &((SearchEntry*)search_map)[i];
        if (person_id == current_entry->person_id) {
            current_entry->lock_flags |= lock_mask;
            return;
        }
    }
    _terminate(0xfffffff2); // Person not found in search_map
}

// Function: free_search_map
void free_search_map(void) {
    free(search_map);
    search_map = NULL;
}

// Function: new_search_map
undefined4 new_search_map(void) {
    if (total_person_count >= 0x1fffffff) { // Check for potential overflow
        return 0xffffffff; // Error code
    }

    if (search_map != NULL) {
        free_search_map();
    }

    // Allocate total_person_count * sizeof(SearchEntry) bytes
    search_map = calloc(total_person_count, sizeof(SearchEntry));
    if (search_map == NULL) {
        _terminate(0xfffffffb); // Memory allocation error
    }

    uint i = 0;
    void* current_person = person_list; // person_list is a void* to the head of persons
    while (current_person != NULL) {
        if (i >= total_person_count) { // Should not happen if total_person_count is correct
            break;
        }
        ((SearchEntry*)search_map)[i].person_id = GET_PERSON_ID(current_person);
        ((SearchEntry*)search_map)[i].lock_flags = 0; // Initialize lock flags
        current_person = GET_PTR_FIELD(current_person, NEXT_LIST_PTR_OFFSET);
        i++;
    }
    return 0; // Success
}

// Function: degrees_of_separation
int degrees_of_separation(void* param_1, void* param_2) {
    if (param_1 == NULL || param_2 == NULL) {
        return -1;
    }
    if (param_1 == param_2) {
        return 0;
    }

    int result;

    // Helper macro to check lock, set lock, and recurse
    // The original code checks and sets lock before iterating through children.
    // This implies the lock applies to the *group* of children.
    #define CHECK_AND_RECURSE_GROUP(lock_mask, loop_count, rel_ptr_macro) \
        if (!is_search_locked(GET_PERSON_ID(param_1), lock_mask)) { \
            set_search_lock(GET_PERSON_ID(param_1), lock_mask); \
            for (int i = 0; i < loop_count; ++i) { \
                void* related_person = GET_PTR_FIELD(param_1, rel_ptr_macro(i)); \
                if (related_person != NULL) { \
                    result = degrees_of_separation(related_person, param_2); \
                    if (result >= 0) { \
                        return result + 1; \
                    } \
                } \
            } \
        }

    // Adopted children (10 of them)
    CHECK_AND_RECURSE_GROUP(LOCK_FLAG_ADOPTED_CHILDREN_MASK, 10, REL_PTR_ADOPTED_CHILD);

    // Biological children (10 of them)
    CHECK_AND_RECURSE_GROUP(LOCK_FLAG_BIO_CHILDREN_MASK, 10, REL_PTR_BIO_CHILD);

    // Helper macro for single relationships
    #define CHECK_AND_RECURSE_SINGLE(lock_mask, rel_ptr_offset) \
        if (!is_search_locked(GET_PERSON_ID(param_1), lock_mask)) { \
            set_search_lock(GET_PERSON_ID(param_1), lock_mask); \
            void* related_person = GET_PTR_FIELD(param_1, rel_ptr_offset); \
            if (related_person != NULL) { \
                result = degrees_of_separation(related_person, param_2); \
                if (result >= 0) { \
                    return result + 1; \
                } \
            } \
        }

    // Spouse
    CHECK_AND_RECURSE_SINGLE(LOCK_FLAG_SPOUSE_MASK, REL_PTR_SPOUSE);

    // Biological Mother
    CHECK_AND_RECURSE_SINGLE(LOCK_FLAG_BIO_MOTHER_MASK, REL_PTR_BIO_MOTHER);

    // Biological Father
    CHECK_AND_RECURSE_SINGLE(LOCK_FLAG_BIO_FATHER_MASK, REL_PTR_BIO_FATHER);

    // Adopting Parents (2 of them)
    CHECK_AND_RECURSE_SINGLE(LOCK_FLAG_ADOPTING_PARENTS_MASK, REL_PTR_ADOPTING_PARENT1);
    CHECK_AND_RECURSE_SINGLE(LOCK_FLAG_ADOPTING_PARENTS_MASK, REL_PTR_ADOPTING_PARENT2);

    // Separated Partners (2 of them)
    CHECK_AND_RECURSE_SINGLE(LOCK_FLAG_SEPARATED_PARTNERS_MASK, REL_PTR_SEP_PARTNER1);
    CHECK_AND_RECURSE_SINGLE(LOCK_FLAG_SEPARATED_PARTNERS_MASK, REL_PTR_SEP_PARTNER2);

    return -1; // No path found
    #undef CHECK_AND_RECURSE_GROUP
    #undef CHECK_AND_RECURSE_SINGLE
}

// Function: can_have_more_biological_children
undefined4 can_have_more_biological_children(void* person_ptr) {
    for (uint i = 0; i <= 9; ++i) {
        if (GET_INT_FIELD(person_ptr, BIO_CHILD_ID_OFFSET(i)) == 0) {
            return 1; // Found an empty slot
        }
    }
    return 0; // No empty slots
}

// Function: set_biological_child
undefined4 set_biological_child(void* child_ptr, void* parent_ptr) {
    for (uint i = 0; i <= 9; ++i) {
        if (GET_INT_FIELD(parent_ptr, BIO_CHILD_ID_OFFSET(i)) == 0) {
            SET_INT_FIELD(parent_ptr, BIO_CHILD_ID_OFFSET(i), GET_PERSON_ID(child_ptr));
            SET_PTR_FIELD(parent_ptr, BIO_CHILD_PTR_OFFSET(i), child_ptr);
            return 0; // Success
        }
    }
    return 0xffffffff; // No empty slots
}

// Function: set_biological_mother
void set_biological_mother(void* child_ptr, void* mother_ptr) {
    SET_INT_FIELD(child_ptr, BIO_MOTHER_ID_OFFSET, GET_PERSON_ID(mother_ptr));
    SET_PTR_FIELD(child_ptr, BIO_MOTHER_PTR_OFFSET, mother_ptr);
}

// Function: set_biological_father
void set_biological_father(void* child_ptr, void* father_ptr) {
    SET_INT_FIELD(child_ptr, BIO_FATHER_ID_OFFSET, GET_PERSON_ID(father_ptr));
    SET_PTR_FIELD(child_ptr, BIO_FATHER_PTR_OFFSET, father_ptr);
}

// Function: unset_adopted_child
undefined4 unset_adopted_child(int child_id, void* parent_ptr) {
    for (uint i = 0; i <= 9; ++i) {
        if (child_id == GET_INT_FIELD(parent_ptr, ADOPT_CHILD_ID_OFFSET(i))) {
            SET_INT_FIELD(parent_ptr, ADOPT_CHILD_ID_OFFSET(i), 0);
            SET_PTR_FIELD(parent_ptr, ADOPT_CHILD_PTR_OFFSET(i), NULL);
            // The original code had `(**(code **)(param_2 + 0x100))();`
            // This is a function pointer call at offset 0x100.
            // Its purpose is unclear and it's not defined, so it's omitted.
            return 0; // Success
        }
    }
    return 0xffffffff; // Child not found
}

// Function: can_have_more_adopted_children
undefined4 can_have_more_adopted_children(void* person_ptr) {
    for (uint i = 0; i <= 9; ++i) {
        if (GET_INT_FIELD(person_ptr, ADOPT_CHILD_ID_OFFSET(i)) == 0) {
            return 1; // Found an empty slot
        }
    }
    return 0; // No empty slots
}

// Function: set_adopted_child
undefined4 set_adopted_child(void* child_ptr, void* parent_ptr) {
    for (uint i = 0; i <= 9; ++i) {
        if (GET_INT_FIELD(parent_ptr, ADOPT_CHILD_ID_OFFSET(i)) == 0) {
            SET_INT_FIELD(parent_ptr, ADOPT_CHILD_ID_OFFSET(i), GET_PERSON_ID(child_ptr));
            SET_PTR_FIELD(parent_ptr, ADOPT_CHILD_PTR_OFFSET(i), child_ptr);
            return 0; // Success
        }
    }
    return 0xffffffff; // No empty slots
}

// Function: set_adopting_parent
void set_adopting_parent(void* child_ptr, void* parent_ptr, int parent_index) {
    // parent_index is 0 or 1 for the two adopting parents
    SET_INT_FIELD(child_ptr, ADOPT_PARENT_ID_OFFSET(parent_index), GET_PERSON_ID(parent_ptr));
    SET_PTR_FIELD(child_ptr, ADOPT_PARENT_PTR_OFFSET(parent_index), parent_ptr);
}

// Function: separate_two_persons
undefined4 separate_two_persons(void* person1_ptr, void* person2_ptr) {
    if (person2_ptr == NULL) {
        return 0xffffffff;
    }

    // Set person1's separated partners
    if (GET_INT_FIELD(person1_ptr, SEP_PARTNER1_ID_OFFSET) == 0) {
        SET_INT_FIELD(person1_ptr, SEP_PARTNER1_ID_OFFSET, GET_PERSON_ID(person2_ptr));
        SET_PTR_FIELD(person1_ptr, SEP_PARTNER1_PTR_OFFSET, person2_ptr);
    } else {
        SET_INT_FIELD(person1_ptr, SEP_PARTNER2_ID_OFFSET, GET_PERSON_ID(person2_ptr));
        SET_PTR_FIELD(person1_ptr, SEP_PARTNER2_PTR_OFFSET, person2_ptr);
    }

    // Set person2's separated partners
    if (GET_INT_FIELD(person2_ptr, SEP_PARTNER1_ID_OFFSET) == 0) {
        SET_INT_FIELD(person2_ptr, SEP_PARTNER1_ID_OFFSET, GET_PERSON_ID(person1_ptr));
        SET_PTR_FIELD(person2_ptr, SEP_PARTNER1_PTR_OFFSET, person1_ptr);
    } else {
        SET_INT_FIELD(person2_ptr, SEP_PARTNER2_ID_OFFSET, GET_PERSON_ID(person1_ptr));
        SET_PTR_FIELD(person2_ptr, SEP_PARTNER2_PTR_OFFSET, person1_ptr);
    }

    // Clear union status for both
    SET_INT_FIELD(person1_ptr, UNION_ID_OFFSET, 0);
    SET_PTR_FIELD(person1_ptr, UNION_PTR_OFFSET, NULL);
    SET_INT_FIELD(person2_ptr, UNION_ID_OFFSET, 0);
    SET_PTR_FIELD(person2_ptr, UNION_PTR_OFFSET, NULL);

    return 0;
}

// Function: union_two_persons
int union_two_persons(void* person1_ptr, void* person2_ptr) {
    int result = 0;

    // Separate person1 from existing union if any
    if (GET_INT_FIELD(person1_ptr, UNION_ID_OFFSET) != 0) {
        result = separate_two_persons(person1_ptr, GET_PTR_FIELD(person1_ptr, UNION_PTR_OFFSET));
        if (result != 0) {
            return result; // Error during separation
        }
    }

    // Establish new union for person1
    SET_INT_FIELD(person1_ptr, UNION_ID_OFFSET, GET_PERSON_ID(person2_ptr));
    SET_PTR_FIELD(person1_ptr, UNION_PTR_OFFSET, person2_ptr);

    // Separate person2 from existing union if any
    if (GET_INT_FIELD(person2_ptr, UNION_ID_OFFSET) != 0) {
        result = separate_two_persons(person2_ptr, GET_PTR_FIELD(person2_ptr, UNION_PTR_OFFSET));
        if (result != 0) {
            return result; // Error during separation
        }
    }

    // Establish new union for person2
    SET_INT_FIELD(person2_ptr, UNION_ID_OFFSET, GET_PERSON_ID(person1_ptr));
    SET_PTR_FIELD(person2_ptr, UNION_PTR_OFFSET, person1_ptr);

    return result;
}

// Function: find_are_related
undefined4 find_are_related(void* param_1, void* param_2) {
    int map_init_result = new_search_map();
    if (map_init_result != 0) {
        return 0xffffffff; // Error initializing search map
    }

    int separation_degrees = degrees_of_separation(param_1, param_2);
    free_search_map();

    if (separation_degrees < 0) {
        return 0x14000041; // Not related
    }
    return 0x10000001; // Related
}

// Function: cmd_add_person
undefined4 cmd_add_person(int param_1_size) {
    if (param_1_size != 0x24) {
        return 0xffffffff; // Invalid size
    }

    void* client_data_buffer = calloc(1, 0x24);
    if (client_data_buffer == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(client_data_buffer, 0x24) != 0x24) {
        free(client_data_buffer);
        _terminate(0xfffffff7);
    }

    void* actual_new_person_obj = new_person(); // Allocates and assigns ID.
    if (actual_new_person_obj == NULL) {
        free(client_data_buffer);
        return 0xffffffff;
    }

    // Overwrite the attributes of the new person with client-provided data.
    // This will overwrite the ID generated by `new_person()` if client_data_buffer starts with an ID.
    memcpy(actual_new_person_obj, client_data_buffer, 0x24);

    add_person_to_list(actual_new_person_obj);
    total_person_count++;
    
    free(client_data_buffer); // Free the temporary buffer
    return 0;
}

// Function: cmd_set_biological_child
int cmd_set_biological_child(int param_1_size) {
    if (param_1_size != 0xc) {
        return -1;
    }

    unsigned int* input_data = (unsigned int*)calloc(3, sizeof(unsigned int)); // 0xc bytes = 3 uints
    if (input_data == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(input_data, 0xc) != 0xc) {
        free(input_data);
        _terminate(0xfffffff7);
    }

    void* child_person = get_person_by_id(input_data[0], person_list);
    void* mother_person = get_person_by_id(input_data[1], person_list);
    void* father_person = get_person_by_id(input_data[2], person_list);

    int result = -1;
    if (child_person != NULL && mother_person != NULL && father_person != NULL) {
        if (can_have_more_biological_children(mother_person) &&
            can_have_more_biological_children(father_person)) {

            if (set_biological_child(child_person, mother_person) == 0) {
                set_biological_mother(child_person, mother_person);
                if (set_biological_child(child_person, father_person) == 0) {
                    set_biological_father(child_person, father_person);
                    result = 0;
                }
            }
        }
    }

    free(input_data);
    return result;
}

// Function: cmd_set_adopted_child
int cmd_set_adopted_child(int param_1_size) {
    if (param_1_size != 0xc) {
        return -1;
    }

    unsigned int* input_data = (unsigned int*)calloc(3, sizeof(unsigned int));
    if (input_data == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(input_data, 0xc) != 0xc) {
        free(input_data);
        _terminate(0xfffffff7);
    }

    void* child_person = get_person_by_id(input_data[0], person_list);
    void* parent1_person = get_person_by_id(input_data[1], person_list);
    void* parent2_person = get_person_by_id(input_data[2], person_list);

    int result = -1;

    if (child_person == NULL) {
        goto end_func;
    }

    if (parent1_person != NULL && !can_have_more_adopted_children(parent1_person)) {
        goto end_func;
    }
    if (parent2_person != NULL && !can_have_more_adopted_children(parent2_person)) {
        goto end_func;
    }

    // Unset existing adopted parents if any
    if (GET_INT_FIELD(child_person, ADOPT_PARENT_ID_OFFSET(0)) != 0) {
        if (unset_adopted_child(GET_INT_FIELD(child_person, ADOPT_PARENT_ID_OFFSET(0)), child_person) == 0xffffffff) {
            goto end_func;
        }
    }
    if (GET_INT_FIELD(child_person, ADOPT_PARENT_ID_OFFSET(1)) != 0) {
        if (unset_adopted_child(GET_INT_FIELD(child_person, ADOPT_PARENT_ID_OFFSET(1)), child_person) == 0xffffffff) {
            goto end_func;
        }
    }

    result = 0; // Assume success initially

    if (parent1_person != NULL) {
        if (set_adopted_child(child_person, parent1_person) == 0xffffffff) {
            result = -1;
        } else {
            set_adopting_parent(child_person, parent1_person, 0); // parent_index 0
        }
    }

    if (result == 0 && parent2_person != NULL) {
        if (set_adopted_child(child_person, parent2_person) == 0xffffffff) {
            result = -1;
        } else {
            set_adopting_parent(child_person, parent2_person, 1); // parent_index 1
        }
    }

end_func:
    free(input_data);
    return result;
}

// Function: cmd_set_union
undefined4 cmd_set_union(int param_1_size) {
    if (param_1_size != 8) {
        return 0xffffffff;
    }

    unsigned int* input_data = (unsigned int*)calloc(2, sizeof(unsigned int)); // 8 bytes = 2 uints
    if (input_data == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(input_data, 8) != 8) {
        free(input_data);
        _terminate(0xfffffff7);
    }

    void* person1 = get_person_by_id(input_data[0], person_list);
    void* person2 = get_person_by_id(input_data[1], person_list);

    undefined4 result = 0;
    if (person1 == NULL || person2 == NULL) {
        result = 0xffffffff;
    } else {
        result = union_two_persons(person1, person2);
    }

    free(input_data);
    return result;
}

// Function: cmd_set_deceased
undefined4 cmd_set_deceased(int param_1_size) {
    if (param_1_size != 6) { // 6 bytes: 4 for ID, 2 for status
        return 0xffffffff;
    }

    // Allocate 6 bytes for input: first 4 bytes for person ID, next 2 bytes for status
    void* input_data_buffer = calloc(1, 6);
    if (input_data_buffer == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(input_data_buffer, 6) != 6) {
        free(input_data_buffer);
        _terminate(0xfffffff7);
    }

    int person_id = *(int*)input_data_buffer;
    short deceased_status = *(short*)((char*)input_data_buffer + 4); // Access 2-byte short at offset 4

    void* person_ptr = get_person_by_id(person_id, person_list);

    undefined4 result = 0;
    if (person_ptr == NULL) {
        result = 0xffffffff;
    } else {
        // Assuming a `short` status field at byte offset 6 in the Person struct.
        *(short*)((char*)person_ptr + 6) = deceased_status;
    }

    free(input_data_buffer);
    return result;
}

// Function: cmd_set_separated
undefined4 cmd_set_separated(int param_1_size) {
    if (param_1_size != 8) {
        return 0xffffffff;
    }

    unsigned int* input_data = (unsigned int*)calloc(2, sizeof(unsigned int)); // 8 bytes = 2 uints
    if (input_data == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(input_data, 8) != 8) {
        free(input_data);
        _terminate(0xfffffff7);
    }

    void* person1 = get_person_by_id(input_data[0], person_list);
    void* person2 = get_person_by_id(input_data[1], person_list);

    undefined4 result = 0;
    if (person1 == NULL || person2 == NULL) {
        result = 0xffffffff;
    } else {
        result = separate_two_persons(person1, person2);
    }

    free(input_data);
    return result;
}

// Function: cmd_are_related
int cmd_are_related(int param_1_size) {
    if (param_1_size != 8) {
        return -1;
    }

    unsigned int* input_data = (unsigned int*)calloc(2, sizeof(unsigned int));
    if (input_data == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(input_data, 8) != 8) {
        free(input_data);
        _terminate(0xfffffff7);
    }

    void* person1 = get_person_by_id(input_data[0], person_list);
    void* person2 = get_person_by_id(input_data[1], person_list);

    int result_code = -1;
    if (person1 == NULL || person2 == NULL) {
        result_code = -1;
    } else {
        // find_are_related returns 0x10000001 (related) or 0x14000041 (not related) or 0xffffffff (error)
        undefined4 find_result = find_are_related(person1, person2);
        if (find_result == 0x10000001 || find_result == 0x14000041) {
            send_data(&find_result, sizeof(find_result), 0);
            result_code = 0; // Success for command execution
        } else {
            result_code = -1; // Error in find_are_related
        }
    }

    free(input_data);
    return result_code;
}

// Function: cmd_degrees_of_separation
int cmd_degrees_of_separation(int param_1_size) {
    if (param_1_size != 8) {
        return -1;
    }

    unsigned int* input_data = (unsigned int*)calloc(2, sizeof(unsigned int));
    if (input_data == NULL) {
        _terminate_no_arg();
    }

    if (recv_all(input_data, 8) != 8) {
        free(input_data);
        _terminate(0xfffffff7);
    }

    void* person1 = get_person_by_id(input_data[0], person_list);
    void* person2 = get_person_by_id(input_data[1], person_list);

    int result_code = -1;
    if (person1 == NULL || person2 == NULL) {
        result_code = -1;
    } else {
        int map_init_result = new_search_map();
        if (map_init_result != 0) { // 0 on success, -1 (0xffffffff) on error
            result_code = -1; // Error initializing search map
        } else {
            int degrees = degrees_of_separation(person1, person2);
            free_search_map();
            
            // Send the degrees value (an integer)
            send_data(&degrees, sizeof(degrees), 0);
            result_code = 0; // Success for command execution
        }
    }

    free(input_data);
    return result_code;
}

// Function: gen_result_bufs
void gen_result_bufs(void) {
    // The original code XORs specific memory regions to generate these values.
    // Without the original memory contents, we'll hardcode typical values or
    // common "magic" values that might represent these commands/results.
    // The loop iterates 0xfff times, xoring bytes of OK and ERR.
    // Given they are command/result codes, direct assignment is the most practical.

    // Commands (example 4-byte ASCII representations or arbitrary values)
    ADD_PERSON_CMD = 0x41444450;         // 'ADDP'
    SET_BIOLOGICAL_CHILD_CMD = 0x5342494F; // 'SBIO'
    SET_ADOPTED_CHILD_CMD = 0x53414450;    // 'SADP'
    SET_UNION_CMD = 0x53554E49;          // 'SUNI'
    SET_DECEASED_CMD = 0x53444543;        // 'SDEC'
    SET_SEPARATED_CMD = 0x53534550;      // 'SSEP'
    ARE_RELATED_CMD = 0x41524552;        // 'ARER'
    DEGREES_CMD = 0x44454752;            // 'DEGR'

    // Result codes
    OK_CODE = 0x00000000;
    ERR_CODE = 0xFFFFFFFF;
}

// Function: main
undefined4 main(void) {
    char command_buffer[8]; // Enough for 4-byte command ID + 4-byte payload size
    int cmd_result; // Stores the result of command execution

    gen_result_bufs(); // Initialize command/result codes

    while (1) {
        if (recv_all(command_buffer, 8) != 8) {
            _terminate(0xfffffff7); // Receive error
        }

        uint32_t command_id = *(uint32_t*)command_buffer;
        int payload_size = *(int*)(command_buffer + 4); // Assuming next 4 bytes is payload size

        cmd_result = -1; // Default to error

        if (command_id == ADD_PERSON_CMD) {
            cmd_result = cmd_add_person(payload_size);
        } else if (command_id == SET_BIOLOGICAL_CHILD_CMD) {
            cmd_result = cmd_set_biological_child(payload_size);
        } else if (command_id == SET_ADOPTED_CHILD_CMD) {
            cmd_result = cmd_set_adopted_child(payload_size);
        } else if (command_id == SET_UNION_CMD) {
            cmd_result = cmd_set_union(payload_size);
        } else if (command_id == SET_DECEASED_CMD) {
            cmd_result = cmd_set_deceased(payload_size);
        } else if (command_id == SET_SEPARATED_CMD) {
            cmd_result = cmd_set_separated(payload_size);
        } else if (command_id == ARE_RELATED_CMD) {
            cmd_result = cmd_are_related(payload_size);
        } else if (command_id == DEGREES_CMD) {
            cmd_result = cmd_degrees_of_separation(payload_size);
        }

        if (cmd_result == 0) {
            send_data(&OK_CODE, sizeof(OK_CODE), 0);
        } else {
            send_data(&ERR_CODE, sizeof(ERR_CODE), 0);
            break; // Exit loop on command error, as per original logic
        }
    }
    return 0; // Main function always returns 0
}