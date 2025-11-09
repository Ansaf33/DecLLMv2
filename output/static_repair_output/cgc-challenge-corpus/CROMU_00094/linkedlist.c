#include <stdio.h>  // For printf
#include <stdlib.h> // For calloc, free
#include <stddef.h> // For size_t (though often included by stdlib.h)

// Define the Node structure for the linked list
// Offsets (assuming 32-bit system where int and pointers are 4 bytes):
// id:    0x00
// value: 0x04
// prev:  0x08
// next:  0x0C
// Total size: 16 bytes (0x10)
typedef struct Node {
    int id;
    int value;
    struct Node* prev;
    struct Node* next;
} Node;

// Define the List structure (header)
// Offsets (assuming 32-bit system where int and pointers are 4 bytes):
// type: 0x00
// head: 0x04
// Total size: 8 bytes
typedef struct List {
    int type;
    Node* head; // Points to the first node in the list
} List;

// Function: NewList
// Creates a new empty list with a specified type.
// Returns a pointer to the new List structure, or NULL on failure.
List *NewList(int type_param) {
    List *newList = (List *)calloc(1, sizeof(List));
    if (newList == NULL) {
        return NULL; // Handle allocation failure
    }
    newList->type = type_param;
    newList->head = NULL; // Initialize head to NULL
    return newList;
}

// Function: AddToList
// Adds a new node with the given id and value to the list.
// The original code implies a constraint: param_3 (value) must match *param_1 (list->type).
// Returns 1 on success, -1 on failure.
int AddToList(List *list, int id, int value) {
    // Check for invalid list pointer or invalid id (id 0 is often reserved/invalid)
    if (list == NULL || id == 0) {
        return -1; // 0xffffffff from original code
    }

    // Preserve the original code's constraint: value must match list type
    if (value != list->type) {
        return -1;
    }

    Node *newNode = (Node *)calloc(1, sizeof(Node));
    if (newNode == NULL) {
        return -1; // Allocation failure
    }
    newNode->id = id;
    newNode->value = value;
    newNode->prev = NULL;
    newNode->next = NULL;

    if (list->head == NULL) {
        // If the list is empty, the new node becomes the head
        list->head = newNode;
    } else {
        // Traverse to the end of the list
        Node *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        // Link the new node to the end
        current->next = newNode;
        newNode->prev = current;
    }
    return 1;
}

// Function: RemoveFromList
// Removes a node with the specified id from the list.
// Returns 1 on success, -1 on failure (e.g., list/id invalid, node not found).
int RemoveFromList(List *list, int id) {
    if (list == NULL || id == 0 || list->head == NULL) {
        return -1;
    }

    Node *nodeToRemove = NULL;

    // Check if the head node is the one to be removed
    if (list->head->id == id) {
        nodeToRemove = list->head;
        list->head = list->head->next; // Move head to the next node
        if (list->head != NULL) {
            list->head->prev = NULL; // New head's prev should be NULL
        }
    } else {
        Node *current = list->head;
        // Iterate through the list to find the node before the one to remove
        while (current->next != NULL && current->next->id != id) {
            current = current->next;
        }

        if (current->next == NULL) {
            return -1; // Node not found
        }

        // Node found, it's current->next
        nodeToRemove = current->next;
        current->next = nodeToRemove->next; // Bypass nodeToRemove
        if (nodeToRemove->next != NULL) {
            nodeToRemove->next->prev = current; // Update prev of the node after the removed one
        }
    }

    if (nodeToRemove != NULL) {
        free(nodeToRemove);
        return 1;
    }
    return -1; // Should not be reached if logic is sound, but for safety
}

// Function: DumpList
// Prints the contents of the list to standard output.
void DumpList(List *list) {
    if (list == NULL || list->head == NULL) {
        printf("Empty List\n");
    } else {
        printf("List Type: %d\n", list->type);
        int index = 0;
        Node *current = list->head;
        while (current != NULL) {
            printf("%d) ID: %d, Value: %d\n", index, current->id, current->value);
            current = current->next;
            index++;
        }
    }
}

int main() {
    // Create a new list of type 100
    List *myList = NewList(100);

    if (myList == NULL) {
        fprintf(stderr, "Failed to create list.\n");
        return 1;
    }

    printf("--- Initial List Dump ---\n");
    DumpList(myList);

    printf("\n--- Adding elements to the list ---\n");
    // Add elements where 'value' matches the list's 'type' (100)
    AddToList(myList, 101, 100);
    AddToList(myList, 102, 100);
    AddToList(myList, 103, 100);
    AddToList(myList, 104, 100);
    
    // This will fail because value (101) does not match list type (100)
    printf("Attempting to add (ID: 105, Value: 101): %s\n", 
           AddToList(myList, 105, 101) == 1 ? "Success" : "Failed (Value mismatch)");
    
    // This will fail because ID (0) is invalid
    printf("Attempting to add (ID: 0, Value: 100): %s\n", 
           AddToList(myList, 0, 100) == 1 ? "Success" : "Failed (Invalid ID)");

    printf("\n--- List Dump After Additions ---\n");
    DumpList(myList);

    printf("\n--- Removing elements from the list ---\n");
    printf("Removing ID 101 (head): %s\n", RemoveFromList(myList, 101) == 1 ? "Success" : "Failed");
    printf("Removing ID 103 (middle): %s\n", RemoveFromList(myList, 103) == 1 ? "Success" : "Failed");
    printf("Attempting to remove non-existent ID 999: %s\n", RemoveFromList(myList, 999) == 1 ? "Success" : "Failed (Not Found)");
    printf("Removing ID 104 (last): %s\n", RemoveFromList(myList, 104) == 1 ? "Success" : "Failed");

    printf("\n--- List Dump After Removals ---\n");
    DumpList(myList);

    printf("\n--- Removing the last remaining element ---\n");
    printf("Removing ID 102: %s\n", RemoveFromList(myList, 102) == 1 ? "Success" : "Failed");

    printf("\n--- List Dump After Last Removal ---\n");
    DumpList(myList);

    // Clean up the list header itself
    // (All nodes should have been freed by RemoveFromList)
    free(myList);
    myList = NULL;

    return 0;
}