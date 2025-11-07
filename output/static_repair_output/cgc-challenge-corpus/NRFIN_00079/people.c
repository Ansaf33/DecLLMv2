#include <stdlib.h> // For calloc, exit, EXIT_FAILURE
#include <string.h> // For memcpy
#include <stdio.h>  // For fprintf, printf

// A placeholder for the value to be assigned to 'some_relation_value'.
// In the original context, this might be a global variable or a constant.
static const int SHIFT_RELATION_VALUE = 0x1A2B3C4D; // Example value

// Define the Person struct based on byte offsets and total size 0x108 (264 bytes)
typedef struct Person {
    int id;          // Offset 0x00
    short age;       // Offset 0x04
    char _padding0[2]; // Offset 0x06, explicit padding to align subsequent fields
    char name[10];   // Offset 0x08 (0x06 + 2 bytes padding)
    char address[10]; // Offset 0x12 (0x08 + 10 bytes name)
    char phone[10];   // Offset 0x1C (0x12 + 10 bytes address)
    // Pad from current offset (0x1C + 10 = 0x26 = 38) to 0x100 (256)
    char _padding1[0x100 - (sizeof(int) + sizeof(short) + sizeof(char[2]) + sizeof(char[10]) + sizeof(char[10]) + sizeof(char[10]))];
    int some_relation_value; // Offset 0x100 (256)
    struct Person *next; // Offset 0x104 (260)
} Person; // Total size: 264 bytes (0x108)

// Define the input structure, assuming it's tightly packed for name/address/phone
typedef struct InputPersonData {
    int id;
    short age;
    char name[10];
    char address[10];
    char phone[10];
    // Potentially other fields in the input that are not copied
} InputPersonData; // Total size: 4 + 2 + 10 + 10 + 10 = 36 bytes

// Function: new_person
Person *new_person(const InputPersonData *input_data) {
    if (input_data == NULL) {
        return NULL;
    }

    Person *new_person_ptr = (Person *)calloc(1, sizeof(Person));
    if (new_person_ptr == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in new_person.\n");
        exit(EXIT_FAILURE);
    }

    new_person_ptr->id = input_data->id;
    new_person_ptr->age = input_data->age;

    // The original code advances the input pointer by 6 bytes and then copies.
    // This implies that input_data->name starts at offset 6 relative to input_data.
    // And the destination for memcpy is at offset 8 relative to new_person_ptr.
    const char *src_data = (const char *)input_data + 6;

    memcpy(new_person_ptr->name, src_data, 10);
    src_data += 10;
    memcpy(new_person_ptr->address, src_data, 10);
    src_data += 10;
    memcpy(new_person_ptr->phone, src_data, 10);

    // Assign the relation value to the field at offset 0x100
    new_person_ptr->some_relation_value = SHIFT_RELATION_VALUE;
    new_person_ptr->next = NULL; // Initialize next pointer

    return new_person_ptr;
}

// Function: get_person_by_id
Person *get_person_by_id(Person *head, int id_to_find) {
    if (head == NULL || id_to_find == 0) { // Assuming ID 0 is invalid or not found
        return NULL;
    }

    Person *current = head;
    while (current != NULL && current->id != id_to_find) {
        current = current->next;
    }
    return current;
}

// Function: add_person_to_list
void add_person_to_list(Person **head, Person *new_person_to_add) {
    if (new_person_to_add == NULL) {
        return; // Cannot add a NULL person
    }
    new_person_to_add->next = NULL; // Ensure new person is the end of its own chain initially

    if (*head == NULL) {
        *head = new_person_to_add;
    } else {
        Person *current = *head;
        while (current->next != NULL) { // Iterate until the last person
            current = current->next;
        }
        current->next = new_person_to_add; // Add new person to the end
    }
}

// Function: count_people
int count_people(const Person *head) {
    int count = 0;
    const Person *current = head;

    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Main function for demonstration
int main() {
    Person *person_list_head = NULL; // Initialize an empty list

    // Create some input data
    InputPersonData input1 = {101, 30, "Alice", "123 Main St", "555-1234"};
    InputPersonData input2 = {102, 25, "Bob", "456 Oak Ave", "555-5678"};
    InputPersonData input3 = {103, 35, "Charlie", "789 Pine Rd", "555-9012"};

    // Create new Person objects
    Person *p1 = new_person(&input1);
    Person *p2 = new_person(&input2);
    Person *p3 = new_person(&input3);
    Person *p_invalid = new_person(NULL); // Test with NULL input

    if (p_invalid == NULL) {
        printf("new_person handled NULL input correctly.\n");
    }

    // Add people to the list
    add_person_to_list(&person_list_head, p1);
    add_person_to_list(&person_list_head, p2);
    add_person_to_list(&person_list_head, p3);

    printf("Number of people in list: %d\n", count_people(person_list_head));

    // Get a person by ID
    Person *found_person = get_person_by_id(person_list_head, 102);
    if (found_person) {
        printf("Found person with ID %d: Name: %s, Age: %d\n",
               found_person->id, found_person->name, found_person->age);
    } else {
        printf("Person with ID 102 not found.\n");
    }

    found_person = get_person_by_id(person_list_head, 999);
    if (found_person == NULL) {
        printf("Person with ID 999 not found (as expected).\n");
    }

    // Clean up memory (important for linked lists)
    Person *current = person_list_head;
    while (current != NULL) {
        Person *next_person = current->next;
        free(current);
        current = next_person;
    }
    printf("Memory freed.\n");

    return 0;
}