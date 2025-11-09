#include <stdlib.h> // For malloc, free

// Define the function pointer types based on their usage
typedef int (*compare_func)(int, int);
typedef void (*destroy_func)(int);

// Function: sl_insert
int * sl_insert(int *head, int value, compare_func compare) {
  // Original code checks param_2 != 0, implying 0 is not a valid value to insert.
  if (value == 0) {
    return head;
  }

  int *newNode = (int *)malloc(sizeof(int) * 2);
  if (newNode == NULL) {
    return head; // Return original head if allocation fails
  }
  *newNode = value; // Store the value

  // Use an indirect pointer to simplify insertion logic for head, middle, and end cases.
  // 'indirect' will point to the 'next' pointer of the previous node, or to 'head' itself.
  int **indirect = &head;

  // Traverse the list to find the insertion point
  while (*indirect != NULL && compare(**(int **)indirect, *newNode) < 1) {
    indirect = (int **)&(*(int **)indirect)[1]; // Move indirect to point to the next pointer of the current node
  }

  // Insert the new node
  newNode[1] = (int)*indirect; // New node points to the node *indirect currently points to
  *indirect = (int)newNode;    // The pointer *indirect now points to newNode

  return head;
}

// Function: sl_destroy
void sl_destroy(int *head, destroy_func destroy) {
  int *current = head;
  while (current != NULL) {
    int *nextNode = (int *)current[1]; // Store next node before freeing current
    // Original code checks if *local_10 != 0 before calling param_2
    if (*current != 0) {
      destroy(*current); // Call the destroy function on the node's value
    }
    free(current); // Free the current node
    current = nextNode; // Move to the next node
  }
}