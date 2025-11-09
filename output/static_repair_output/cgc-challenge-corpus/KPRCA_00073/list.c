#include <stdio.h>  // For FILE, fprintf, stdout
#include <stdlib.h> // For calloc, free
#include <string.h> // For strdup (used in main for demonstration, not in list functions)

// Define a Node structure for a singly linked list
typedef struct Node {
    struct Node* next;
    void* data; // Generic pointer for data
} Node;

// Helper function for safe memory allocation.
// Returns NULL on failure.
static void* safe_calloc(size_t num, size_t size) {
    void* ptr = calloc(num, size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
    }
    return ptr;
}

// Function: AllocateAndInitializeListHead
// Creates a new list head and initializes its data.
// head_ptr: A pointer to the head pointer of the list (e.g., &myListHead).
//           If *head_ptr is NULL, this new node becomes the head.
// data: The data to store in the new head node.
// Returns a pointer to the newly created head node, or NULL on failure.
Node* AllocateAndInitializeListHead(Node** head_ptr, void* data) {
    if (head_ptr == NULL) {
        return NULL;
    }

    Node* new_node = (Node*)safe_calloc(1, sizeof(Node));
    if (new_node != NULL) {
        *head_ptr = new_node; // Set the head pointer to the new node
        new_node->data = data;
    }
    return new_node;
}

// Function: FreeList
// Recursively frees all nodes in the list.
// node: The starting node to free (usually the head).
// Note: This function only frees the Node structures themselves.
// It does NOT free the 'data' pointers stored within the nodes.
// The caller is responsible for managing the lifetime and freeing of the data.
void FreeList(Node* node) {
    if (node == NULL) {
        return;
    }

    // Recursively free the rest of the list
    FreeList(node->next);

    // Free the current node structure
    free(node);
}

// Function: LenList
// Calculates the number of nodes in the list.
// head: The head of the list.
// Returns the length of the list.
int LenList(const Node* head) {
    int count = 0;
    while (head != NULL) {
        count++;
        head = head->next;
    }
    return count;
}

// Function: AppendToList
// Appends a new node with the given data to the end of the list.
// head_ptr: A pointer to the head pointer of the list (e.g., &myListHead).
//           This allows modifying the head if the list is initially empty.
// data: The data to store in the new node.
// Returns a pointer to the newly created node, or NULL on failure.
Node* AppendToList(Node** head_ptr, void* data) {
    if (head_ptr == NULL) {
        return NULL;
    }

    Node* new_node = (Node*)safe_calloc(1, sizeof(Node));
    if (new_node == NULL) {
        return NULL;
    }
    new_node->data = data;
    new_node->next = NULL; // New node is always at the end

    if (*head_ptr == NULL) {
        *head_ptr = new_node; // List was empty, new node is the head
    } else {
        Node* current = *head_ptr;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node; // Link new node to the end
    }
    return new_node;
}

// Function: UniqAppendToList
// Appends data to the list only if an identical data pointer is not already present.
// head_ptr: A pointer to the head pointer of the list.
// data: The data pointer to append.
void UniqAppendToList(Node** head_ptr, void* data) {
    if (head_ptr == NULL) {
        return;
    }

    Node* current = *head_ptr;
    while (current != NULL) {
        if (current->data == data) {
            return; // Data pointer already exists, do not append
        }
        current = current->next;
    }

    // If we reached here, data is unique (by pointer value), so append it.
    AppendToList(head_ptr, data);
}

// Function: UniqExtendList
// Extends list1 with unique elements (by pointer value) from list2.
// list1_head: The head of the first list (will be modified).
// list2_head: The head of the second list (read-only).
// Returns the head of the modified list1, or NULL if either input list is NULL.
Node* UniqExtendList(Node* list1_head, const Node* list2_head) {
    // If either list is NULL, return NULL as per the original logic's implied behavior.
    if (list1_head == NULL || list2_head == NULL) {
        return NULL;
    }

    const Node* current_list2 = list2_head;
    while (current_list2 != NULL) {
        Node* current_list1 = list1_head;
        // Search if current_list2->data is already in list1 (by pointer value comparison)
        while (current_list1 != NULL && current_list1->data != current_list2->data) {
            current_list1 = current_list1->next;
        }

        // If current_list1 is NULL, it means the data was not found in list1
        if (current_list1 == NULL) {
            // AppendToList needs a pointer to the head pointer, so we pass &list1_head
            AppendToList(&list1_head, current_list2->data);
        }
        current_list2 = current_list2->next;
    }

    return list1_head; // Return the modified list1
}

// Function: CheckForCycle
// Detects if there's a cycle in the list using Floyd's Tortoise and Hare algorithm.
// head: The head of the list.
// Returns 1 if a cycle is detected, 0 otherwise.
int CheckForCycle(const Node* head) {
    if (head == NULL) {
        return 0; // No list, no cycle
    }

    const Node* tortoise = head;
    const Node* hare = head->next; // Start hare one step ahead

    while (hare != NULL && hare->next != NULL) {
        if (tortoise == hare) {
            return 1; // Cycle detected
        }
        tortoise = tortoise->next;
        hare = hare->next->next;
    }

    return 0; // No cycle
}

// Function: PrintList
// Prints the list content to the specified file stream.
// stream: The file stream to print to (e.g., stdout, stderr).
// head: The head of the list.
// Assumes data pointers point to null-terminated strings for "%s" formatting.
// If data is not a string, using "%s" will lead to undefined behavior/crash.
void PrintList(FILE* stream, const Node* head) {
    if (head == NULL) {
        fprintf(stream, "(empty list)\n");
        return;
    }
    const Node* current = head;
    while (current != NULL) {
        // Use %p for pointer addresses (Node address and data address) for portability.
        // Use %s and cast to (char*) for string data, as implied by original format.
        fprintf(stream, "%p:%s", (void*)current, (char*)current->data);

        if (current->next == NULL) {
            fprintf(stream, "\n"); // Last element
        } else {
            fprintf(stream, " "); // Not last element
        }
        current = current->next;
    }
}

// Main function to demonstrate the list operations
int main() {
    Node* myList = NULL;
    Node* anotherList = NULL;

    // Data for the lists. Using strdup to ensure distinct pointers for the data,
    // which aligns with the pointer-value comparison logic of UniqAppendToList/UniqExtendList.
    // These strings must be freed manually at the end.
    char* s_apple = strdup("apple");
    char* s_banana = strdup("banana");
    char* s_cherry = strdup("cherry");
    char* s_date = strdup("date");
    char* s_grape = strdup("grape");
    char* s_fig = strdup("fig");
    char* s_orange = strdup("orange");

    fprintf(stdout, "--- AllocateAndInitializeListHead & AppendToList ---\n");
    // Allocate and initialize the head
    AllocateAndInitializeListHead(&myList, s_apple);
    AppendToList(&myList, s_banana);
    AppendToList(&myList, s_cherry);
    AppendToList(&myList, s_date);

    fprintf(stdout, "MyList (length %d): ", LenList(myList));
    PrintList(stdout, myList);

    fprintf(stdout, "\n--- UniqAppendToList ---\n");
    UniqAppendToList(&myList, s_grape); // s_grape is new by pointer value, append
    fprintf(stdout, "MyList after adding 'grape': ");
    PrintList(stdout, myList);

    UniqAppendToList(&myList, s_apple); // s_apple pointer already exists, should NOT append
    fprintf(stdout, "MyList after trying to add existing 'apple' ptr: ");
    PrintList(stdout, myList);

    UniqAppendToList(&myList, s_orange); // s_orange is new by pointer value, append
    fprintf(stdout, "MyList after adding 'orange': ");
    PrintList(stdout, myList);

    fprintf(stdout, "\n--- CheckForCycle ---\n");
    fprintf(stdout, "MyList has cycle? %d\n", CheckForCycle(myList));

    // Create a cycle for demonstration (e.g., last node points to second node)
    Node* current = myList;
    Node* second_node = NULL;
    if (current != NULL) {
        second_node = current->next; // Get the second node
        while (current->next != NULL) {
            current = current->next; // Traverse to the last node
        }
        current->next = second_node; // Create a cycle: last node points to second
    }
    fprintf(stdout, "MyList (with cycle) has cycle? %d\n", CheckForCycle(myList));
    // To avoid infinite loop on PrintList/FreeList, break the cycle before further operations.
    if (current != NULL) {
        current->next = NULL; // Break the cycle
    }

    fprintf(stdout, "\n--- UniqExtendList ---\n");
    // Create another list using some existing and some new data pointers
    AppendToList(&anotherList, s_banana); // s_banana is also in myList (shared pointer)
    AppendToList(&anotherList, s_fig);    // s_fig is new
    AppendToList(&anotherList, s_apple);  // s_apple is also in myList (shared pointer)

    fprintf(stdout, "AnotherList: ");
    PrintList(stdout, anotherList);

    // Extend myList with unique elements (by pointer value) from anotherList
    Node* extendedList = UniqExtendList(myList, anotherList);
    fprintf(stdout, "MyList after UniqExtendList with AnotherList: ");
    PrintList(stdout, extendedList);

    fprintf(stdout, "\n--- FreeList and Data Cleanup ---\n");
    // Free the lists (nodes only)
    FreeList(anotherList);
    fprintf(stdout, "AnotherList freed. Length: %d\n", LenList(anotherList)); // LenList might still show non-zero if anotherList wasn't NULL.
                                                                           // The memory it pointed to is freed, but the local pointer 'anotherList' itself is unchanged.

    FreeList(myList); // myList was modified by UniqExtendList, now contains more nodes.
    fprintf(stdout, "MyList freed. Length: %d\n", LenList(myList)); // Same as above.

    // Free the data pointers that were strdup'd
    free(s_apple);
    free(s_banana);
    free(s_cherry);
    free(s_date);
    free(s_grape);
    free(s_fig);
    free(s_orange);

    return 0;
}