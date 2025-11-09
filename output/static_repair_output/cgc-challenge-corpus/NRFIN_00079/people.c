#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a placeholder for _terminate.
// In a real application, this might log an error and exit.
void _terminate() {
    fprintf(stderr, "Fatal error: memory allocation failed.\n");
    exit(EXIT_FAILURE);
}

// Dummy global variable to represent `shift_relation`
// Its type is `undefined4` which is likely `int` or `unsigned int`
int shift_relation = 12345; // Placeholder value

// Struct definition based on analysis of new_person and list operations
// Assumes a 32-bit system for pointer size (4 bytes) and alignment behavior.
typedef struct Person {
    int id;               // Offset 0x00 (4 bytes)
    short age;            // Offset 0x04 (2 bytes)
    // 2 bytes of implicit padding for 4-byte alignment before `name` if needed.
    // The original memcpy target `local_10 + 2` means offset 8.
    char name[10];        // Offset 0x08 (10 bytes)
    char address[10];     // Offset 0x12 (10 bytes)
    char phone[10];       // Offset 0x1C (10 bytes)
    char _padding_0[0x100 - (0x1C + 10)]; // Padding to reach 0x100 (0xDA = 218 bytes)
    int some_relation_field; // Offset 0x100 (4 bytes)
    struct Person *next;  // Offset 0x104 (4 bytes, for 32-bit pointer)
} Person; // Total size 0x108 (264 bytes)

// Struct to represent the raw input data format for new_person.
// This matches the sequential data layout implied by the original function's
// pointer arithmetic, without padding or list-specific fields.
typedef struct PersonRawData {
    int id;
    short age;
    char name[10];
    char address[10];
    char phone[10];
} PersonRawData;

// Function: new_person
Person *new_person(const PersonRawData *src_data_ptr) {
    if (src_data_ptr == NULL) {
        return NULL;
    }

    Person *new_p = (Person *)calloc(1, sizeof(Person));
    if (new_p == NULL) {
        _terminate(); // Handle memory allocation failure
    }

    // Copy fields directly from the source raw data
    new_p->id = src_data_ptr->id;
    new_p->age = src_data_ptr->age;
    memcpy(new_p->name, src_data_ptr->name, sizeof(new_p->name));
    memcpy(new_p->address, src_data_ptr->address, sizeof(new_p->address));
    memcpy(new_p->phone, src_data_ptr->phone, sizeof(new_p->phone));

    // Set the specific relation field
    new_p->some_relation_field = shift_relation;
    // new_p->next is already NULL due to calloc

    return new_p;
}

// Function: get_person_by_id
Person *get_person_by_id(Person *list_head, int target_id) {
    // Assuming target_id == 0 is considered an invalid ID based on original code.
    if (list_head == NULL || target_id == 0) {
        return NULL;
    }

    Person *current = list_head;
    while (current != NULL && current->id != target_id) {
        current = current->next;
    }
    return current;
}

// Function: add_person_to_list
void add_person_to_list(Person **list_head_ptr, Person *new_person_node) {
    if (new_person_node == NULL) {
        return; // Cannot add a NULL person
    }

    new_person_node->next = NULL; // Ensure the new node is properly terminated

    if (*list_head_ptr == NULL) { // If the list is empty
        *list_head_ptr = new_person_node;
    } else {
        Person *current = *list_head_ptr;
        while (current->next != NULL) { // Traverse to the last node
            current = current->next;
        }
        current->next = new_person_node; // Append the new node
    }
}

// Function: count_people
int count_people(const Person *list_head) {
    int count = 0;
    const Person *current = list_head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Helper function to free all nodes in the list
void free_person_list(Person *list_head) {
    Person *current = list_head;
    while (current != NULL) {
        Person *next_node = current->next;
        free(current);
        current = next_node;
    }
}

// Main function to demonstrate usage
int main() {
    Person *person_list = NULL; // Head of the linked list

    // Example raw data for persons
    PersonRawData raw_data1 = {101, 30, "Alice", "123 Main", "555-1111"};
    PersonRawData raw_data2 = {102, 25, "Bob", "456 Oak", "555-2222"};
    PersonRawData raw_data3 = {103, 35, "Charlie", "789 Pine", "555-3333"};

    // Create new person nodes
    Person *p1 = new_person(&raw_data1);
    Person *p2 = new_person(&raw_data2);
    Person *p3 = new_person(&raw_data3);
    Person *p4 = new_person(NULL); // Test with NULL input

    if (p4 == NULL) {
        printf("new_person handled NULL input correctly.\n");
    }

    // Add persons to the list
    add_person_to_list(&person_list, p1);
    add_person_to_list(&person_list, p2);
    add_person_to_list(&person_list, p3);
    add_person_to_list(&person_list, NULL); // Test with NULL person

    // Count people
    printf("Number of people in list: %d\n", count_people(person_list)); // Expected: 3

    // Get person by ID
    Person *found_person = get_person_by_id(person_list, 102);
    if (found_person) {
        printf("Found person with ID %d: Name=%s, Age=%d, Address=%s, Phone=%s\n",
               found_person->id, found_person->name, found_person->age,
               found_person->address, found_person->phone);
    } else {
        printf("Person with ID 102 not found.\n");
    }

    found_person = get_person_by_id(person_list, 104);
    if (found_person) {
        printf("Found person with ID 104: Name=%s\n", found_person->name);
    } else {
        printf("Person with ID 104 not found.\n"); // Expected: not found
    }

    found_person = get_person_by_id(person_list, 0); // Test invalid ID
    if (found_person) {
        printf("Found person with ID 0.\n");
    } else {
        printf("Person with ID 0 not found (as expected).\n");
    }

    // Demonstrate iteration
    printf("\nAll people in the list:\n");
    Person *current = person_list;
    while (current != NULL) {
        printf("ID: %d, Name: %s\n", current->id, current->name);
        current = current->next;
    }

    // Clean up
    free_person_list(person_list);
    printf("\nList freed.\n");

    return 0;
}