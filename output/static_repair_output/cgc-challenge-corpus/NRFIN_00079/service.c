#include <stdio.h>   // For fprintf (for debugging _terminate)
#include <stdlib.h>  // For malloc, free, exit, calloc
#include <string.h>  // For memcmp, strcpy
#include <stdint.h>  // For intptr_t

// --- Type Definitions ---
typedef unsigned int uint;
typedef unsigned int undefined4;
typedef char undefined; // Used for raw byte access, can be `void` for opaque pointers

// --- Forward Declarations ---
typedef struct Person Person;

// --- Global Variables (placeholders) ---
// Person structure as derived from memory access patterns, assuming 32-bit pointers.
// This structure is larger than 0x24 bytes, implying that 0x24 in cmd_add_person
// refers to an input buffer size, not the full Person struct size.
struct Person {
    uint id; // 0x00
    // Placeholder for 0x24 bytes (0x28 - 0x4) that are not explicitly accessed in the provided code
    // but fill the gap before `adopted_children` start.
    char unknown_data_0x04_to_0x28[0x24]; // 36 bytes, fills up to offset 0x28

    struct { // Each entry is 8 bytes (uint id_val + Person* ptr)
        uint id_val;
        Person *ptr;
    } adopted_children[10]; // Starts at 0x28, ends at 0x78 (10 * 8 = 80 bytes)

    struct {
        uint id_val;
        Person *ptr;
    } biological_children[10]; // Starts at 0x78, ends at 0xc8 (10 * 8 = 80 bytes)

    struct {
        uint id_val;
        Person *ptr;
    } biological_mother; // Starts at 0xc8, ends at 0xd0 (8 bytes)

    struct {
        uint id_val;
        Person *ptr;
    } biological_father; // Starts at 0xd0, ends at 0xd8 (8 bytes)

    struct {
        uint id_val;
        Person *ptr;
    } adopting_parents[2]; // Starts at 0xd8, ends at 0xe8 (2 * 8 = 16 bytes)

    struct {
        uint id_val;
        Person *ptr;
    } current_spouse; // Starts at 0xe8, ends at 0xf0 (8 bytes)

    struct {
        uint id_val;
        Person *ptr;
    } ex_spouse1; // Starts at 0xf0, ends at 0xf8 (8 bytes)

    struct {
        uint id_val;
        Person *ptr;
    } ex_spouse2; // Starts at 0xf8, ends at 0x100 (8 bytes)

    Person *next_person_in_list; // Starts at 0x100 (4 bytes if 32-bit pointer).
                                 // Total size: 0x100 + sizeof(Person*) = 0x104 (260 bytes).
                                 // This matches the `local_10 + 0x104` access for `next_person_in_list`.
};


// Search map structure
typedef struct {
    int person_id_or_ptr; // Stores Person* cast to int, or Person ID
    uint lock_flags;
} SearchEntry;

// Global variables
unsigned int total_person_count = 0;
Person *person_list = NULL; // Head of a linked list of persons
SearchEntry *search_map = NULL; // Array of SearchEntry

// Command buffers/constants
// Initialized with arbitrary values for compilation. `gen_result_bufs` will overwrite OK/ERR.
char OK[4] = {0};
char ERR[4] = {0};
char ADD_PERSON[4] = {'A', 'D', 'D', 'P'};
char SET_BIOLOGICAL_CHILD[4] = {'S', 'B', 'C', 'H'};
char SET_ADOPTED_CHILD[4] = {'S', 'A', 'C', 'H'};
char SET_UNION[4] = {'S', 'U', 'N', 'I'};
char SET_DECEASED[4] = {'S', 'D', 'E', 'C'};
char SET_SEPARATED[4] = {'S', 'S', 'E', 'P'};
char ARE_RELATED[4] = {'A', 'R', 'E', 'L'};
char DEGREES[4] = {'D', 'E', 'G', 'R'};


// --- Placeholder Functions (to make code compilable) ---
// These functions are not provided in the snippet, so we define minimal versions.

// Simulates program termination with an error code.
void _terminate(uint status) {
    fprintf(stderr, "Program terminated with status: 0x%x\n", status);
    exit((int)status);
}

// Simulates receiving data. Returns `count` for simplicity.
// In a real application, this would read from a socket/stdin.
int recv_all(void *buf, size_t count) {
    // For demonstration, fill with dummy data or read from stdin/socket
    // printf("recv_all: attempting to receive %zu bytes.\n", count); // Debug print
    memset(buf, 0, count); // Clear buffer for safety
    return (int)count; // Simulate successful reception
}

// Simulates sending data. Returns `len` for simplicity.
// In a real application, this would write to a socket/stdout.
int send(const void *buf, size_t len) { // Simplified signature
    // For demonstration, print to stdout
    // printf("send: sending %zu bytes.\n", len); // Debug print
    return (int)len; // Simulate successful send
}

// Placeholder for `new_person`. Returns a pointer to a newly allocated Person struct.
Person *new_person() {
    Person *p = (Person *)calloc(1, sizeof(Person));
    if (!p) {
        _terminate(0xfffffffb); // Out of memory
    }
    // Assign a unique ID (dummy for now)
    static uint next_id = 1;
    p->id = next_id++;
    return p;
}

// Placeholder for `add_person_to_list`. Adds a person to the global `person_list`.
void add_person_to_list(Person *p) {
    p->next_person_in_list = person_list;
    person_list = p;
}

// Placeholder for `get_person_by_id`. Traverses `person_list` to find a person.
Person *get_person_by_id(uint id) {
    Person *current = person_list;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next_person_in_list;
    }
    return NULL; // Not found
}

// Placeholder for `(**(code **)(param_2 + 0x100))();` in `unset_adopted_child`
// This looks like a function pointer call. Assuming it's a specific callback.
// The original code was `(**(code **)(param_2 + 0x100))();`.
// `param_2` is `parent_person`. `0x100` is the offset of `next_person_in_list`.
// This is likely a decompiler artifact where `next_person_in_list` was misinterpreted as a function pointer.
// For now, a generic callback is used.
typedef void (*PersonCallback)(Person *);
void person_callback(Person *p) {
    // printf("Person callback for ID: %u\n", p->id); // Debug print
}


// --- Original Functions (fixed and simplified) ---

// Function: is_search_locked
uint is_search_locked(int param_1, uint param_2) {
    for (uint i = 0; i < total_person_count; ++i) {
        // Accessing search_map as an array of SearchEntry
        if (param_1 == search_map[i].person_id_or_ptr) {
            return param_2 & search_map[i].lock_flags;
        }
    }
    _terminate(0xfffffff2); // Person not found in search_map
    return 0; // Should not be reached
}

// Function: set_search_lock
void set_search_lock(int param_1, uint param_2) {
    for (uint i = 0; i < total_person_count; ++i) {
        if (param_1 == search_map[i].person_id_or_ptr) {
            search_map[i].lock_flags |= param_2;
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
        return 0xffffffff; // Error: too many persons
    }

    if (search_map != NULL) {
        free_search_map();
    }

    // Allocate total_person_count * sizeof(SearchEntry) bytes
    search_map = (SearchEntry *)calloc(total_person_count, sizeof(SearchEntry));
    if (search_map == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int i = 0;
    for (Person *current_person = person_list; current_person != NULL; current_person = current_person->next_person_in_list) {
        if (i >= total_person_count) {
             // This condition should ideally not be met if total_person_count is accurate
             // but it's a safeguard.
             _terminate(0xfffffff2); // Mismatch in person count
        }
        search_map[i].person_id_or_ptr = (int)(intptr_t)current_person; // Store person pointer (or ID)
        search_map[i].lock_flags = 0; // Initialize lock flags
        i++;
    }
    return 0; // Success
}

// Function: degrees_of_separation
int degrees_of_separation(Person *param_1, Person *param_2) {
    if (param_1 == NULL || param_2 == NULL) {
        return -1;
    }
    if (param_1 == param_2) {
        return 0;
    }

    int result;

    // Adopted Children (0x10)
    // `param_1[(local_10 + 4) * 2 + 3]` means `param_1->adopted_children[local_10].ptr`
    if (!is_search_locked(param_1->id, 0x10)) {
        set_search_lock(param_1->id, 0x10);
        for (int i = 0; i < 10; ++i) {
            if (param_1->adopted_children[i].ptr != NULL) {
                result = degrees_of_separation(param_1->adopted_children[i].ptr, param_2);
                if (result >= 0) {
                    return result + 1;
                }
            }
        }
    }

    // Biological Children (0x20)
    // `param_1[(local_10 + 0xe) * 2 + 3]` means `param_1->biological_children[local_10].ptr`
    if (!is_search_locked(param_1->id, 0x20)) {
        set_search_lock(param_1->id, 0x20);
        for (int i = 0; i < 10; ++i) {
            if (param_1->biological_children[i].ptr != NULL) {
                result = degrees_of_separation(param_1->biological_children[i].ptr, param_2);
                if (result >= 0) {
                    return result + 1;
                }
            }
        }
    }

    // Biological Mother (2)
    // `param_1[0x33]` means `param_1->biological_mother.ptr`
    if (!is_search_locked(param_1->id, 2)) {
        set_search_lock(param_1->id, 2);
        if (param_1->biological_mother.ptr != NULL) {
            result = degrees_of_separation(param_1->biological_mother.ptr, param_2);
            if (result >= 0) {
                return result + 1;
            }
        }
    }

    // Biological Father (4)
    // `param_1[0x35]` means `param_1->biological_father.ptr`
    if (!is_search_locked(param_1->id, 4)) {
        set_search_lock(param_1->id, 4);
        if (param_1->biological_father.ptr != NULL) {
            result = degrees_of_separation(param_1->biological_father.ptr, param_2);
            if (result >= 0) {
                return result + 1;
            }
        }
    }

    // Current Spouse & Ex-spouses (8)
    // `param_1[0x37]` means `param_1->current_spouse.ptr`
    // `param_1[0x39]` means `param_1->ex_spouse1.ptr`
    // `param_1[0x3b]` means `param_1->ex_spouse2.ptr`
    if (!is_search_locked(param_1->id, 8)) {
        set_search_lock(param_1->id, 8);
        if (param_1->current_spouse.ptr != NULL) {
            result = degrees_of_separation(param_1->current_spouse.ptr, param_2);
            if (result >= 0) {
                return result + 1;
            }
        }
        if (param_1->ex_spouse1.ptr != NULL) {
            result = degrees_of_separation(param_1->ex_spouse1.ptr, param_2);
            if (result >= 0) {
                return result + 1;
            }
        }
        if (param_1->ex_spouse2.ptr != NULL) {
            result = degrees_of_separation(param_1->ex_spouse2.ptr, param_2);
            if (result >= 0) {
                return result + 1;
            }
        }
    }

    // Adopting Parent 1 (0x40)
    // Based on `set_adopting_parent` logic, using adopting_parents array.
    if (!is_search_locked(param_1->id, 0x40)) {
        set_search_lock(param_1->id, 0x40);
        if (param_1->adopting_parents[0].ptr != NULL) {
            result = degrees_of_separation(param_1->adopting_parents[0].ptr, param_2);
            if (result >= 0) {
                return result + 1;
            }
        }
    }

    // Adopting Parent 2 (0x80)
    if (!is_search_locked(param_1->id, 0x80)) {
        set_search_lock(param_1->id, 0x80);
        if (param_1->adopting_parents[1].ptr != NULL) {
            result = degrees_of_separation(param_1->adopting_parents[1].ptr, param_2);
            if (result >= 0) {
                return result + 1;
            }
        }
    }
    return -1;
}

// Function: can_have_more_biological_children
undefined4 can_have_more_biological_children(Person *param_1) {
    for (uint i = 0; i < 10; ++i) {
        if (param_1->biological_children[i].ptr == NULL) {
            return 1;
        }
    }
    return 0;
}

// Function: set_biological_child
undefined4 set_biological_child(Person *child_person, Person *parent_person) {
    for (uint i = 0; i < 10; ++i) {
        if (parent_person->biological_children[i].ptr == NULL) {
            parent_person->biological_children[i].id_val = child_person->id;
            parent_person->biological_children[i].ptr = child_person;
            return 0;
        }
    }
    return 0xffffffff; // No room for more biological children
}

// Function: set_biological_mother
void set_biological_mother(Person *child_person, Person *mother_person) {
    child_person->biological_mother.id_val = mother_person->id;
    child_person->biological_mother.ptr = mother_person;
}

// Function: set_biological_father
void set_biological_father(Person *child_person, Person *father_person) {
    child_person->biological_father.id_val = father_person->id;
    child_person->biological_father.ptr = father_person;
}

// Function: unset_adopted_child
undefined4 unset_adopted_child(Person *child_person, Person *parent_person) {
    for (uint i = 0; i < 10; ++i) {
        if (child_person->id == parent_person->adopted_children[i].id_val) {
            parent_person->adopted_children[i].id_val = 0;
            parent_person->adopted_children[i].ptr = NULL;
            // Original code: `(**(code **)(param_2 + 0x100))();`
            // Replaced with a placeholder callback.
            person_callback(parent_person);
            return 0;
        }
    }
    return 0xffffffff; // Child not found as adopted
}

// Function: can_have_more_adopted_children
undefined4 can_have_more_adopted_children(Person *param_1) {
    for (uint i = 0; i < 10; ++i) {
        if (param_1->adopted_children[i].ptr == NULL) {
            return 1;
        }
    }
    return 0;
}

// Function: set_adopted_child
undefined4 set_adopted_child(Person *child_person, Person *parent_person) {
    for (uint i = 0; i < 10; ++i) {
        if (parent_person->adopted_children[i].ptr == NULL) {
            parent_person->adopted_children[i].id_val = child_person->id;
            parent_person->adopted_children[i].ptr = child_person;
            return 0;
        }
    }
    return 0xffffffff; // No room for more adopted children
}

// Function: set_adopting_parent
void set_adopting_parent(Person *child_person, Person *parent_person, int parent_idx) {
    // parent_idx should be 0 or 1 for adopting_parents array
    if (parent_idx >= 0 && parent_idx < 2) {
        child_person->adopting_parents[parent_idx].id_val = parent_person->id;
        child_person->adopting_parents[parent_idx].ptr = parent_person;
    }
}

// Function: separate_two_persons
undefined4 separate_two_persons(Person *person1, Person *person2) {
    if (person2 == NULL) {
        return 0xffffffff;
    }

    // Set ex-spouses for person1
    if (person1->ex_spouse1.ptr == NULL) {
        person1->ex_spouse1.id_val = person2->id;
        person1->ex_spouse1.ptr = person2;
    } else {
        person1->ex_spouse2.id_val = person2->id;
        person1->ex_spouse2.ptr = person2;
    }

    // Set ex-spouses for person2
    if (person2->ex_spouse1.ptr == NULL) {
        person2->ex_spouse1.id_val = person1->id;
        person2->ex_spouse1.ptr = person1;
    } else {
        person2->ex_spouse2.id_val = person1->id;
        person2->ex_spouse2.ptr = person1;
    }

    // Clear current spouse
    person1->current_spouse.id_val = 0;
    person1->current_spouse.ptr = NULL;
    person2->current_spouse.id_val = 0;
    person2->current_spouse.ptr = NULL;

    return 0;
}

// Function: union_two_persons
int union_two_persons(Person *person1, Person *person2) {
    int result = 0;

    // If person1 is currently married, separate them first
    if (person1->current_spouse.ptr != NULL) {
        result = separate_two_persons(person1, person1->current_spouse.ptr);
        if (result == -1) {
            return result;
        }
    }
    person1->current_spouse.id_val = person2->id;
    person1->current_spouse.ptr = person2;


    // If person2 is currently married, separate them first
    if (person2->current_spouse.ptr != NULL) {
        result = separate_two_persons(person2, person2->current_spouse.ptr);
        if (result == -1) {
            return result;
        }
    }
    person2->current_spouse.id_val = person1->id;
    person2->current_spouse.ptr = person1;

    return result;
}

// Function: find_are_related
undefined4 find_are_related(Person *param_1, Person *param_2) {
    int map_init_result = new_search_map();
    if (map_init_result != -1) {
        int separation_degrees = degrees_of_separation(param_1, param_2);
        free_search_map();
        if (separation_degrees < 0) {
            return 0x14000041; // Not related
        }
        return 0x10000001; // Related
    }
    return 0xffffffff; // Error creating search map
}

// Function: cmd_add_person
undefined4 cmd_add_person(int param_1_size) {
    if (param_1_size != 0x24) {
        return 0xffffffff; // Incorrect size
    }

    // Allocate buffer for input data (0x24 bytes)
    void *input_buffer = calloc(1, 0x24);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 0x24);
    if (bytes_received != 0x24) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    Person *new_p = new_person();
    if (new_p == NULL) { // new_person can call _terminate, but check for NULL anyway
        free(input_buffer);
        return 0xffffffff; // Error creating person
    }

    // The input_buffer likely contains initial data for the new person,
    // which would be copied into `new_p`. This step is missing in the decompiler output.
    // For now, we'll assume `new_person()` handles initialization based on `input_buffer` implicitly,
    // or the 0x24 bytes are just a command payload.
    // However, the decompiler output doesn't use `input_buffer` after `recv_all` except for freeing.

    add_person_to_list(new_p);
    total_person_count++;

    free(input_buffer);
    return 0; // Success
}

// Function: cmd_set_biological_child
int cmd_set_biological_child(int param_1_size) {
    if (param_1_size != 0xc) {
        return -1; // Incorrect size
    }

    // Allocate buffer for input data (0xc bytes)
    uint *input_buffer = (uint *)calloc(1, 0xc);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 0xc);
    if (bytes_received != 0xc) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    // Input buffer assumed to contain: child_id, mother_id, father_id
    uint child_id = input_buffer[0];
    uint mother_id = input_buffer[1];
    uint father_id = input_buffer[2];

    free(input_buffer); // Free buffer early if not needed further

    Person *child_person = get_person_by_id(child_id);
    Person *mother_person = get_person_by_id(mother_id);
    Person *father_person = get_person_by_id(father_id);

    if (child_person == NULL || mother_person == NULL || father_person == NULL) {
        return -1; // One or more persons not found
    }

    if (!can_have_more_biological_children(mother_person)) {
        return -1; // Mother cannot have more biological children
    }
    if (!can_have_more_biological_children(father_person)) {
        return -1; // Father cannot have more biological children
    }

    if (set_biological_child(child_person, mother_person) == 0xffffffff) {
        return -1; // Failed to set child for mother
    }
    set_biological_mother(child_person, mother_person);

    if (set_biological_child(child_person, father_person) == 0xffffffff) {
        return -1; // Failed to set child for father
    }
    set_biological_father(child_person, father_person);

    return 0; // Success
}

// Function: cmd_set_adopted_child
int cmd_set_adopted_child(int param_1_size) {
    if (param_1_size != 0xc) {
        return -1; // Incorrect size
    }

    // Allocate buffer for input data (0xc bytes)
    uint *input_buffer = (uint *)calloc(1, 0xc);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 0xc);
    if (bytes_received != 0xc) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    // Input buffer assumed to contain: child_id, parent1_id, parent2_id
    uint child_id = input_buffer[0];
    uint parent1_id = input_buffer[1];
    uint parent2_id = input_buffer[2];

    free(input_buffer);

    Person *child_person = get_person_by_id(child_id);
    Person *parent1_person = get_person_by_id(parent1_id);
    Person *parent2_person = get_person_by_id(parent2_id);

    if (child_person == NULL || (parent1_person == NULL && parent2_person == NULL)) {
        return -1; // Child not found or no valid parents
    }

    if (parent1_person != NULL && !can_have_more_adopted_children(parent1_person)) {
        return -1; // Parent 1 cannot have more adopted children
    }
    if (parent2_person != NULL && !can_have_more_adopted_children(parent2_person)) {
        return -1; // Parent 2 cannot have more adopted children
    }

    int result = 0;

    // Unset previous adopting parents if any
    if (child_person->adopting_parents[0].ptr != NULL) {
        result = unset_adopted_child(child_person, child_person->adopting_parents[0].ptr);
        if (result == -1) return -1;
    }
    if (child_person->adopting_parents[1].ptr != NULL) {
        result = unset_adopted_child(child_person, child_person->adopting_parents[1].ptr);
        if (result == -1) return -1;
    }

    if (parent1_person != NULL) {
        result = set_adopted_child(child_person, parent1_person);
        if (result == -1) return -1;
        set_adopting_parent(child_person, parent1_person, 0); // Parent 0
    }

    if (parent2_person != NULL) {
        result = set_adopted_child(child_person, parent2_person);
        if (result == -1) return -1;
        set_adopting_parent(child_person, parent2_person, 1); // Parent 1
    }

    return result;
}

// Function: cmd_set_union
undefined4 cmd_set_union(int param_1_size) {
    if (param_1_size != 8) {
        return 0xffffffff; // Incorrect size
    }

    // Allocate buffer for input data (8 bytes)
    uint *input_buffer = (uint *)calloc(1, 8);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 8);
    if (bytes_received != 8) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    // Input buffer assumed to contain: person1_id, person2_id
    uint person1_id = input_buffer[0];
    uint person2_id = input_buffer[1];

    free(input_buffer);

    Person *person1 = get_person_by_id(person1_id);
    Person *person2 = get_person_by_id(person2_id);

    if (person1 == NULL || person2 == NULL) {
        return 0xffffffff; // One or both persons not found
    }

    return union_two_persons(person1, person2);
}

// Function: cmd_set_deceased
undefined4 cmd_set_deceased(int param_1_size) {
    if (param_1_size != 6) { // 6 bytes: 4 bytes for ID, 2 bytes for status
        return 0xffffffff; // Incorrect size
    }

    // Allocate buffer for input data (6 bytes)
    char *input_buffer = (char *)calloc(1, 6);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 6);
    if (bytes_received != 6) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    // Input buffer assumed to contain: person_id (4 bytes), deceased_status (2 bytes)
    uint person_id = *(uint *)input_buffer;
    unsigned short deceased_status = *(unsigned short *)(input_buffer + 4);

    free(input_buffer);

    Person *person = get_person_by_id(person_id);
    if (person == NULL) {
        return 0xffffffff; // Person not found
    }

    // Assuming `person->unknown_data_0x04_to_0x28` contains the status at offset 2 (0x06).
    // `*(undefined2 *)(local_24 + 6)` means `*(unsigned short *)((char*)person + 6)`.
    // This access is to `person->unknown_data_0x04_to_0x28[2]` given `unknown_data_0x04_to_0x28` starts at offset 4.
    *(unsigned short *)((char *)person + 6) = deceased_status;

    return 0; // Success
}

// Function: cmd_set_separated
undefined4 cmd_set_separated(int param_1_size) {
    if (param_1_size != 8) {
        return 0xffffffff; // Incorrect size
    }

    // Allocate buffer for input data (8 bytes)
    uint *input_buffer = (uint *)calloc(1, 8);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 8);
    if (bytes_received != 8) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    // Input buffer assumed to contain: person1_id, person2_id
    uint person1_id = input_buffer[0];
    uint person2_id = input_buffer[1];

    free(input_buffer);

    Person *person1 = get_person_by_id(person1_id);
    Person *person2 = get_person_by_id(person2_id);

    if (person1 == NULL || person2 == NULL) {
        return 0xffffffff; // One or both persons not found
    }

    return separate_two_persons(person1, person2);
}

// Function: cmd_are_related
int cmd_are_related(int param_1_size) {
    if (param_1_size != 8) {
        return -1; // Incorrect size
    }

    // Allocate buffer for input data (8 bytes)
    uint *input_buffer = (uint *)calloc(1, 8);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 8);
    if (bytes_received != 8) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    // Input buffer assumed to contain: person1_id, person2_id
    uint person1_id = input_buffer[0];
    uint person2_id = input_buffer[1];

    free(input_buffer);

    Person *person1 = get_person_by_id(person1_id);
    Person *person2 = get_person_by_id(person2_id);

    if (person1 == NULL || person2 == NULL) {
        return -1; // One or both persons not found
    }

    undefined4 result_code = find_are_related(person1, person2);
    if (result_code == 0x10000001 || result_code == 0x14000041) {
        // Send the result_code back
        send(&result_code, sizeof(result_code));
        return 0; // Success
    }

    return -1; // Error in finding relation
}

// Function: cmd_degrees_of_separation
int cmd_degrees_of_separation(int param_1_size) {
    if (param_1_size != 8) {
        return -1; // Incorrect size
    }

    // Allocate buffer for input data (8 bytes)
    uint *input_buffer = (uint *)calloc(1, 8);
    if (input_buffer == NULL) {
        _terminate(0xfffffffb); // Out of memory
    }

    int bytes_received = recv_all(input_buffer, 8);
    if (bytes_received != 8) {
        free(input_buffer);
        _terminate(0xfffffff7); // Receive error
    }

    // Input buffer assumed to contain: person1_id, person2_id
    uint person1_id = input_buffer[0];
    uint person2_id = input_buffer[1];

    free(input_buffer);

    Person *person1 = get_person_by_id(person1_id);
    Person *person2 = get_person_by_id(person2_id);

    if (person1 == NULL || person2 == NULL) {
        return -1; // One or both persons not found
    }

    int result = -1;
    if (new_search_map() != -1) {
        result = degrees_of_separation(person1, person2);
        free_search_map();
        // Send the result back
        send(&result, sizeof(result));
        return 0; // Success
    }

    return -1; // Error creating search map
}

// Function: gen_result_bufs
void gen_result_bufs(void) {
    // The original code was obfuscated. Assuming the intent is to initialize OK and ERR.
    // The values are set here directly to "OK" and "ERR".
    strcpy(OK, "OK");
    strcpy(ERR, "ERR");
}

// Function: main
undefined4 main(void) {
    char command_buffer[8]; // To receive 8 bytes command (4 bytes for command ID, 4 for size)

    gen_result_bufs();

    while (1) {
        int bytes_received = recv_all(command_buffer, 8);
        if (bytes_received != 8) {
            _terminate(0xfffffff7); // Receive error
        }

        int result_status = 0; // Default success

        // Extract the size parameter from the command buffer
        int command_size_param = *(int*)(command_buffer + 4); 

        if (memcmp(command_buffer, ADD_PERSON, 4) == 0) {
            result_status = cmd_add_person(command_size_param);
        } else if (memcmp(command_buffer, SET_BIOLOGICAL_CHILD, 4) == 0) {
            result_status = cmd_set_biological_child(command_size_param);
        } else if (memcmp(command_buffer, SET_ADOPTED_CHILD, 4) == 0) {
            result_status = cmd_set_adopted_child(command_size_param);
        } else if (memcmp(command_buffer, SET_UNION, 4) == 0) {
            result_status = cmd_set_union(command_size_param);
        } else if (memcmp(command_buffer, SET_DECEASED, 4) == 0) {
            result_status = cmd_set_deceased(command_size_param);
        } else if (memcmp(command_buffer, SET_SEPARATED, 4) == 0) {
            result_status = cmd_set_separated(command_size_param);
        } else if (memcmp(command_buffer, ARE_RELATED, 4) == 0) {
            result_status = cmd_are_related(command_size_param);
        } else if (memcmp(command_buffer, DEGREES, 4) == 0) {
            result_status = cmd_degrees_of_separation(command_size_param);
        } else {
            result_status = -1; // Unknown command
        }

        if (result_status != 0) {
            send(ERR, sizeof(ERR)); // Send error message
            break; // Exit loop on error
        }
        send(OK, sizeof(OK)); // Send success message
    }
    return 0; // Main function returns 0
}