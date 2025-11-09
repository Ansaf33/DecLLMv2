#include <stdio.h>  // For fprintf, stderr, printf
#include <stdlib.h> // For calloc, free, exit, random, srandom
#include <string.h> // For strcmp, strdup
#include <time.h>   // For time (to seed srandom)

// Define the list node structure
typedef struct List {
    struct List *next;
    struct List *prev;
    void *data; // Use void* for generic data, cast to specific types as needed
} List;

// Function: init_list
// Allocates and initializes a new list node.
List *init_list(void *data) {
    List *newNode = (List *)calloc(1, sizeof(List));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in init_list at %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    // calloc already initializes next and prev to NULL
    return newNode;
}

// Function: len_list (forward declaration as it's used by other functions)
int len_list(List *head);

// Function: append_list
// Appends data to a list. If unique_check is 0, it prevents duplicates (assuming string data).
// This version makes a deep copy of the string data using strdup.
void append_list(List **head_ptr, const char *data, int unique_check) {
    char *data_copy = NULL;
    if (data != NULL) {
        data_copy = strdup(data);
        if (data_copy == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for data copy in append_list at %s:%d\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
    }

    List *newNode = init_list(data_copy);

    if (*head_ptr == NULL) {
        *head_ptr = newNode;
    } else {
        List *current = *head_ptr;
        if (unique_check == 0) {
            for (; current != NULL; current = current->next) {
                // Assuming data is a string for strcmp comparison
                if (current->data != NULL && data_copy != NULL && strcmp(data_copy, (char *)current->data) == 0) {
                    free(newNode->data); // Duplicate found, free the strdup'd data for the new node
                    free(newNode);       // Free the new node itself
                    return;              // Do not add duplicate
                }
            }
            // If loop finishes, no duplicate found, proceed to append.
            // current is now NULL, reset to head to find end of list.
            current = *head_ptr;
        }

        // Find the last node in the list
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
        newNode->prev = current;
    }
}

// Function: concat_list
// Concatenates src_list to dest_list. Returns the head of the modified dest_list.
// This creates new nodes and new copies of data for the appended elements.
List *concat_list(List *dest_list, List *src_list) {
    List *result_head = dest_list;
    List *current_src = src_list;

    while (current_src != NULL) {
        // Append data from src_list to dest_list. '1' means no unique check.
        // append_list will create a new node and strdup the data.
        append_list(&result_head, (char *)current_src->data, 1);
        current_src = current_src->next;
    }
    return result_head;
}

// Function: len_list
// Returns the number of elements in the list.
int len_list(List *head) {
    int count = 0;
    List *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Function: lindex
// Returns the data at a specified index. Supports negative indexing (from end of list).
// Returns NULL if the list is empty or index is out of bounds.
void *lindex(List *head, int index) {
    if (head == NULL) {
        return NULL;
    }

    int list_len = len_list(head);

    // Adjust index for negative values (e.g., -1 means last element)
    if (index < 0) {
        index = list_len + index;
    }

    // Check if the adjusted index is within valid bounds
    if (index < 0 || index >= list_len) {
        return NULL; // Index out of bounds
    }

    List *current = head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return current->data;
}

// Function: copy_list
// Creates a new list containing a slice of the source_list from start_index to end_index.
// This creates new nodes and new copies of string data.
List *copy_list(List *source_list, unsigned int start_index, unsigned int end_index) {
    if (source_list == NULL) {
        return NULL;
    }

    int list_len = len_list(source_list);

    // If end_index is 0 or exceeds list_len, set it to list_len
    if (end_index == 0 || list_len < end_index) {
        end_index = list_len;
    }

    // Validate the range
    if (start_index >= end_index || start_index >= list_len) {
        return NULL; // Invalid range or empty slice
    }

    List *newList = NULL;
    List *current_src = source_list;

    // Advance current_src to the starting position
    for (unsigned int i = 0; i < start_index; i++) {
        current_src = current_src->next;
        // This check is defensive; if start_index < list_len, current_src should not be NULL.
        if (current_src == NULL) {
            return NULL; // Should not happen if start_index is valid
        }
    }

    // Copy elements from start_index up to (but not including) end_index
    for (unsigned int i = start_index; i < end_index; i++) {
        // append_list will create a new node and strdup the data.
        append_list(&newList, (char *)current_src->data, 1); // '1' for no unique check
        current_src = current_src->next;
        if (current_src == NULL && i + 1 < end_index) {
            // Reached end of source list prematurely
            break;
        }
    }
    return newList;
}

// Function: free_list
// Frees all nodes in a list. If free_data_flag is 0, it also frees the data payload of each node.
// Note: 'free_data_flag' convention: 0 means free data, non-zero means don't free data.
void free_list(List *head, int free_data_flag) {
    List *current = head;
    List *next_node;

    while (current != NULL) {
        next_node = current->next; // Store next node before freeing current
        if (free_data_flag == 0 && current->data != NULL) {
            free(current->data); // Free the data payload (e.g., strdup'd strings)
        }
        // The original code `if (local_10[1] != 0) { free((void *)local_10[1]); }`
        // which implies freeing `current->prev`, is incorrect for a standard doubly linked list.
        // `prev` is just a pointer to another node, not separately allocated memory owned by the current node.
        // This line has been removed.
        free(current); // Free the list node itself
        current = next_node;
    }
}

// Function: free_list_of_lists
// Frees a list where each node's data is itself a list.
// 'free_data_depth' determines how many levels deep to free the actual data.
// A depth of 1 means free data of the inner lists. A depth of 0 means don't free any data.
void free_list_of_lists(List *head_of_lists, int free_data_depth) {
    List *current = head_of_lists;
    List *next_list_node;

    while (current != NULL) {
        next_list_node = current->next; // Store next node before freeing current
        if (current->data != NULL && free_data_depth > 0) {
            // Recursively free the inner list.
            // The free_data_flag for the inner list will be (free_data_depth - 1).
            free_list((List *)current->data, free_data_depth - 1);
        }
        // Again, remove incorrect freeing of `current->prev`.
        free(current); // Free the list node itself
        current = next_list_node;
    }
}

// Function: random_element
// Returns a random element's data from the list.
// Returns NULL if the list is empty.
void *random_element(List *head) {
    int list_len = len_list(head);
    if (list_len == 0) {
        return NULL;
    }

    // `random()` returns a long int, typically non-negative.
    // Ensure `srandom(time(NULL))` is called once in main for proper randomness.
    long r_val = random();
    if (r_val < 0) {
        // This condition is defensive; `random()` usually returns non-negative values.
        fprintf(stderr, "Error: random() returned a negative value in random_element at %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    unsigned int index = (unsigned int)(r_val % list_len);
    return lindex(head, index);
}

// Main function to demonstrate usage and test functionalities
int main() {
    // Seed the random number generator once for `random_element`
    srandom(time(NULL));

    List *myList = NULL;

    printf("--- Appending elements ---\n");
    // append_list now handles strdup, so we can pass string literals directly
    append_list(&myList, "apple", 0);  // Unique check, adds "apple"
    append_list(&myList, "banana", 0); // Unique check, adds "banana"
    append_list(&myList, "cherry", 0); // Unique check, adds "cherry"
    append_list(&myList, "apple", 0);  // Unique check, should NOT add "apple" again

    printf("List length: %d\n", len_list(myList)); // Expected: 3 (apple, banana, cherry)

    printf("List elements:\n");
    for (int i = 0; i < len_list(myList); ++i) {
        printf("  [%d]: %s\n", i, (char *)lindex(myList, i));
    }

    // Test non-unique append
    append_list(&myList, "grape", 1); // No unique check, adds "grape"
    printf("List length after non-unique append: %d\n", len_list(myList)); // Expected: 4
    printf("  [3]: %s\n", (char *)lindex(myList, 3)); // Expected: grape

    // Test lindex
    printf("\n--- Testing lindex ---\n");
    printf("Element at index 1: %s\n", (char *)lindex(myList, 1));   // Expected: banana
    printf("Element at index -1: %s\n", (char *)lindex(myList, -1)); // Expected: grape
    printf("Element at index 0: %s\n", (char *)lindex(myList, 0));   // Expected: apple
    printf("Element at index 10 (out of bounds): %s\n", (char *)lindex(myList, 10)); // Expected: (null)

    // Test copy_list
    printf("\n--- Testing copy_list ---\n");
    List *copiedList = copy_list(myList, 1, 3); // Copy "banana", "cherry"
    printf("Copied list length: %d\n", len_list(copiedList)); // Expected: 2
    printf("Copied list elements:\n");
    for (int i = 0; i < len_list(copiedList); ++i) {
        printf("  [%d]: %s\n", i, (char *)lindex(copiedList, i));
    }

    List *fullCopy = copy_list(myList, 0, 0); // Full copy (start 0, end 0 means up to list_len)
    printf("Full copied list length: %d\n", len_list(fullCopy)); // Expected: 4
    printf("Full copied list elements:\n");
    for (int i = 0; i < len_list(fullCopy); ++i) {
        printf("  [%d]: %s\n", i, (char *)lindex(fullCopy, i));
    }

    // Test concat_list
    printf("\n--- Testing concat_list ---\n");
    List *otherList = NULL;
    append_list(&otherList, "date", 0);
    append_list(&otherList, "elderberry", 0);
    printf("Other list length: %d\n", len_list(otherList)); // Expected: 2

    myList = concat_list(myList, otherList); // Append otherList's elements to myList
    printf("Concatenated list length (myList): %d\n", len_list(myList)); // Expected: 4 + 2 = 6
    printf("Concatenated list elements:\n");
    for (int i = 0; i < len_list(myList); ++i) {
        printf("  [%d]: %s\n", i, (char *)lindex(myList, i));
    }

    // Test random_element
    printf("\n--- Testing random_element ---\n");
    printf("Random element: %s\n", (char *)random_element(myList));
    printf("Random element: %s\n", (char *)random_element(myList));
    printf("Random element: %s\n", (char *)random_element(myList));

    // Test nested lists (list of lists)
    printf("\n--- Testing free_list_of_lists ---\n");
    List *nestedList = NULL;
    append_list(&nestedList, "inner_a", 0);
    append_list(&nestedList, "inner_b", 0);

    List *listOfLists = NULL;
    append_list(&listOfLists, "outer_1", 0);
    // Add the nested list as data to the outer list.
    // Note: This makes the outer list node "own" the nested list.
    // The data for "outer_1" (a string) will be freed by free_list(listOfLists, 0).
    // To add a list, we need to bypass strdup and directly set the data.
    List *listNodeForNested = init_list(nestedList); // create a node whose data is nestedList
    if (listOfLists == NULL) {
        listOfLists = listNodeForNested;
    } else {
        List *current = listOfLists;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = listNodeForNested;
        listNodeForNested->prev = current;
    }
    append_list(&listOfLists, "outer_2", 0);

    printf("List of lists length: %d\n", len_list(listOfLists)); // Expected: 3
    printf("Outer list elements:\n");
    for (int i = 0; i < len_list(listOfLists); ++i) {
        void *data = lindex(listOfLists, i);
        if (i == 1) { // The nested list is at index 1
            printf("  [%d]: (Nested List) Length: %d\n", i, len_list((List*)data));
            for (int j = 0; j < len_list((List*)data); ++j) {
                printf("    Inner [%d]: %s\n", j, (char*)lindex((List*)data, j));
            }
        } else {
            printf("  [%d]: %s\n", i, (char *)data);
        }
    }


    // Free all lists
    printf("\n--- Freeing lists ---\n");
    // myList, copiedList, fullCopy, otherList have nodes whose data was strdup'd by append_list.
    // So, free_data_flag should be 0 to free these strdup'd strings.
    free_list(myList, 0);       // Free myList and its data
    free_list(copiedList, 0);   // Free copiedList and its data
    free_list(fullCopy, 0);     // Free fullCopy and its data
    free_list(otherList, 0);    // Free otherList and its data

    // For listOfLists:
    // - "outer_1" and "outer_2" are strings (data should be freed).
    // - The data for the second node is `nestedList` (a `List*`).
    //   `free_data_depth=1` means free the string data *within* the `nestedList`.
    //   `free_list_of_lists` will free the "outer_1" and "outer_2" nodes.
    //   When it encounters the `nestedList` as data, it calls `free_list(nestedList, 0)`.
    //   `free_list(nestedList, 0)` will free "inner_a" and "inner_b" strings and their nodes.
    free_list_of_lists(listOfLists, 1); // Free listOfLists, and its inner lists' data

    printf("All lists freed.\n");

    return 0;
}