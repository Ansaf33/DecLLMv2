#include <stdlib.h> // For malloc, free, NULL
#include <stdio.h>  // For printf, if a default print function is used or for main

// Type definitions for the linked list node and function pointers
typedef struct Node {
    struct Node *next;
    int value;
} Node;

typedef int (*CompareFunc)(int, int);
typedef void (*PrintFunc)(int);

// Function: create_node
Node* create_node(int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        return NULL;
    }
    newNode->next = NULL;
    newNode->value = value;
    return newNode;
}

// Function: insert (prepends a node to the list)
int insert(Node** head, int value) {
    if (value == 0) { // Assuming 0 is an invalid value to store
        return 0;
    }
    Node* newNode = create_node(value);
    if (newNode == NULL) {
        return 0; // Allocation failed
    }
    newNode->next = *head;
    *head = newNode;
    return 1;
}

// Function: insert_in_order (inserts a node while maintaining sorted order)
int insert_in_order(Node** head, int value, CompareFunc compare) {
    if (value == 0) { // Assuming 0 is an invalid value to store
        return 0;
    }

    Node* newNode = create_node(value);
    if (newNode == NULL) {
        return 0; // Allocation failed
    }

    // If list is empty or new node should be the new head
    if (*head == NULL || compare(value, (*head)->value) <= 0) {
        newNode->next = *head;
        *head = newNode;
        return 1;
    }

    Node* current = *head;
    // Iterate to find the insertion point (newNode will be inserted after 'current')
    // Loop continues as long as current->next exists AND value is greater than current->next's value
    while (current->next != NULL && compare(value, current->next->value) > 0) {
        current = current->next;
    }

    // Insert newNode after 'current'
    newNode->next = current->next;
    current->next = newNode;
    return 1;
}

// Function: append (adds a node to the end of the list)
int append(Node** head, int value) {
    if (value == 0) { // Assuming 0 is an invalid value to store
        return 0;
    }
    Node* newNode = create_node(value);
    if (newNode == NULL) {
        return 0; // Allocation failed
    }

    if (*head == NULL) {
        *head = newNode;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    return 1;
}

// Function: popfront (removes the head node, returns its value, 0 if list empty)
int popfront(Node** head) {
    if (*head == NULL) {
        return 0; // List is empty, or 0 is an invalid value
    }
    Node* temp = *head;
    int value = temp->value;
    *head = temp->next;
    free(temp);
    return value;
}

// Function: popback (removes the last node, returns 1 on success, 0 if list empty)
int popback(Node** head) {
    if (*head == NULL) {
        return 0; // List is empty, nothing to pop
    }
    if ((*head)->next == NULL) { // Only one node in the list
        free(*head);
        *head = NULL;
        return 1; // Success
    }
    Node* current = *head;
    Node* previous = NULL;
    while (current->next != NULL) {
        previous = current;
        current = current->next;
    }
    // 'current' is now the last node, 'previous' is the second to last
    previous->next = NULL;
    free(current);
    return 1; // Success
}

// Function: find (returns the value if found, 0 if not found or invalid input)
int find(Node* head, int value, CompareFunc compare) {
    if (head == NULL || value == 0) { // Assuming 0 is an invalid value to search for
        return 0; // List empty or invalid search value
    }
    Node* current = head;
    while (current != NULL) {
        if (compare(value, current->value) == 0) { // Found
            return current->value;
        }
        current = current->next;
    }
    return 0; // Not found
}

// Function: pop (removes a specific node by value, returns its value, 0 if not found)
int pop(Node** head, int value, CompareFunc compare) {
    if (head == NULL || *head == NULL || value == 0) { // Assuming 0 is an invalid value
        return 0; // List empty or invalid search value
    }

    Node* current = *head;
    Node* previous = NULL;

    while (current != NULL) {
        if (compare(value, current->value) == 0) { // Found the node to pop
            int poppedValue = current->value;
            if (previous == NULL) { // Popping the head node
                *head = current->next;
            } else { // Popping a middle or last node
                previous->next = current->next;
            }
            free(current);
            return poppedValue;
        }
        previous = current;
        current = current->next;
    }
    return 0; // Not found
}

// Function: print (iterates through the list and applies a print function to each node's value)
void print(Node* head, PrintFunc printNode) {
    Node* current = head;
    while (current != NULL) {
        printNode(current->value);
        current = current->next;
    }
}