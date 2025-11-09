#include <stdlib.h> // For calloc, NULL, exit
#include <stdio.h>  // For fprintf

// Assuming uint is unsigned int and undefined4 is unsigned int (4-byte unsigned)
typedef unsigned int uint;
typedef unsigned int undefined4; // Represents a 4-byte undefined type, often unsigned int

// Placeholder for _error function
void _error(int code, const char* file, int line) {
    fprintf(stderr, "ERROR %d: %s:%d\n", code, file, line);
    exit(code);
}

// Minimal declarations for other functions used in the snippet
// These functions are not provided, so their exact behavior is inferred
// based on their usage in the snippet.
// In a real scenario, these would be properly defined.
uint init_list(uint initial_value) {
    // Placeholder: This function is expected to initialize a list and return its head (as a uint).
    // For this exercise, it simply returns the initial value.
    return initial_value;
}

// param_1 (list_head_ptr) is a pointer to the list head (uint *).
// param_2 (value_to_append) is the value to append (uint).
// param_3 (flag) is an integer flag (int).
void append_list(uint *list_head_ptr, uint value_to_append, int flag) {
    // Placeholder: This function is expected to append a value to the list
    // whose head is pointed to by list_head_ptr.
    // For this exercise, we just simulate the call.
    (void)list_head_ptr;       // Avoid unused parameter warning
    (void)value_to_append;     // Avoid unused parameter warning
    (void)flag;                // Avoid unused parameter warning
}


// Function: init_tree
// Allocates and initializes a tree node.
// A tree node is represented as an array of 5 uints:
// [key1, key2, list_head, left_child_ptr_as_uint, right_child_ptr_as_uint]
uint * init_tree(uint param_1, uint param_2, int param_3) {
  uint *newNode = (uint *)calloc(1, 5 * sizeof(uint)); // Allocate for 5 uints (20 bytes)
  if (newNode == NULL) {
    _error(1,
           "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/tree.c"
           ,0x22);
  }
  // Initialize only if keys and list_param are non-zero, as per original logic
  if (((param_1 | param_2) != 0) && (param_3 != 0)) {
    newNode[0] = param_1;       // Key 1
    newNode[1] = param_2;       // Key 2
    newNode[2] = init_list(param_3); // List head for this node, initialized with param_3
  }
  return newNode;
}

// Function: get_tree
// Navigates the tree to find a node matching param_2 and param_3.
// Returns the list head (uint) of the found node, or 0 if not found.
uint get_tree(uint *param_1, uint param_2, uint param_3) {
  if (param_1 == NULL) {
    return 0; // Node not found or empty tree
  }

  // Compare (param_2, param_3) with current node's keys (param_1[0], param_1[1])
  // Go left if param_3 is less than current_node_key2
  // OR if param_3 equals current_node_key2 AND param_2 is less than current_node_key1
  if (param_3 < param_1[1] || (param_3 == param_1[1] && param_2 < param_1[0])) {
    // Recurse left (param_1[3] holds the left child pointer, cast back to uint *)
    return get_tree((uint *)param_1[3], param_2, param_3);
  }
  // Go right if param_3 is greater than current_node_key2
  // OR if param_3 equals current_node_key2 AND param_2 is greater than current_node_key1
  else if (param_3 > param_1[1] || (param_3 == param_1[1] && param_2 > param_1[0])) {
    // Recurse right (param_1[4] holds the right child pointer, cast back to uint *)
    return get_tree((uint *)param_1[4], param_2, param_3);
  }
  else {
    // Keys match: param_3 == param_1[1] && param_2 == param_1[0]
    return param_1[2]; // Node found, return its list head
  }
}

// Function: ins_tree
// Inserts a new node or appends to an existing node's list.
// Returns 0 on success, 0xffffffff on error (e.g., param_1 is NULL).
undefined4 ins_tree(uint *param_1, uint param_2, uint param_3, undefined4 param_4) {
  if (param_1 == NULL) {
    return 0xffffffff; // Cannot insert into a NULL tree/subtree
  }

  // Pointer to the child link (either param_1[3] or param_1[4]) that we might need to modify
  uint **target_child_ptr;

  // Compare (param_2, param_3) with current node's keys (param_1[0], param_1[1])
  // Determine if we need to go left or right
  if (param_3 < param_1[1] || (param_3 == param_1[1] && param_2 < param_1[0])) {
    target_child_ptr = (uint **)&param_1[3]; // Point to where the left child pointer is stored
  }
  else if (param_3 > param_1[1] || (param_3 == param_1[1] && param_2 > param_1[0])) {
    target_child_ptr = (uint **)&param_1[4]; // Point to where the right child pointer is stored
  }
  else {
    // Keys match: param_3 == param_1[1] && param_2 == param_1[0]
    // Node found, append param_4 to its list (param_1[2])
    append_list(&param_1[2], param_4, 0);
    return 0; // Success
  }

  // If we decided to go left or right, check if the target child exists
  if (*target_child_ptr == NULL) { // If the child pointer is NULL, create a new node
    // Store the new node's pointer (returned as uint) into the target child slot
    *target_child_ptr = (uint)init_tree(param_2, param_3, param_4);
    return 0; // Success
  } else {
    // Otherwise, recurse into the existing child subtree
    return ins_tree((uint *)*target_child_ptr, param_2, param_3, param_4);
  }
}

// Function: num_nodes
// Counts the number of nodes in the tree.
int num_nodes(uint *param_1) { // param_1 is the root of the subtree
  if (param_1 == NULL) {
    return 0; // Base case: empty subtree has 0 nodes
  }
  // Sum of nodes in left subtree + nodes in right subtree + 1 (for the current node)
  return num_nodes((uint *)param_1[3]) + num_nodes((uint *)param_1[4]) + 1;
}

// Function: _tree_to_list
// Helper function to convert a tree (or subtree) into a linear list.
// Appends the list head of each node to a combined list.
// Returns 0 on success, 0xffffffff on error.
undefined4 _tree_to_list(uint *param_1, uint *param_2) { // param_1 is tree node, param_2 is pointer to combined list head
  if (param_1 == NULL) {
    return 0; // Base case: empty subtree, no elements to add
  }

  // Append the current node's list head (param_1[2]) to the combined list (pointed to by param_2)
  append_list(param_2, param_1[2], 1);

  undefined4 res;
  // Recurse for the left child
  res = _tree_to_list((uint *)param_1[3], param_2);
  if (res < 0) { // Check for error (0xffffffff interpreted as -1 if undefined4 is signed)
    return 0xffffffff;
  }
  // Recurse for the right child
  res = _tree_to_list((uint *)param_1[4], param_2);
  if (res < 0) { // Check for error
    return 0xffffffff;
  }
  return 0; // Success
}

// Function: tree_to_list
// Converts an entire tree into a single linear list.
// Returns the head of the new combined list (uint), or 0 on error/empty tree.
undefined4 tree_to_list(uint *param_1) { // param_1 is the root of the tree
  if (param_1 == NULL) {
    return 0; // Empty tree, return empty list head (0)
  }

  uint current_list_head; // This variable will hold the head of the new combined list
  // Initialize the combined list with the list head from the root node (param_1[2])
  current_list_head = init_list(param_1[2]);

  // Recursively add nodes from the left subtree to the combined list
  if (_tree_to_list((uint *)param_1[3], &current_list_head) < 0) {
    return 0; // Error in left subtree processing, return 0 (empty list on error)
  }

  // Recursively add nodes from the right subtree to the combined list
  if (_tree_to_list((uint *)param_1[4], &current_list_head) < 0) {
    return 0; // Error in right subtree processing, return 0 (empty list on error)
  }

  return current_list_head; // Return the final combined list head
}