#include <stdlib.h> // For calloc, NULL, exit
#include <stdio.h>  // For fprintf, stderr
#include <stdint.h> // For uintptr_t, used for robust pointer-to-integer conversions

// Type definitions based on typical reverse engineering output for 32-bit systems
typedef unsigned int uint;
typedef unsigned int undefined4; // Represents a 4-byte unsigned integer

// Forward declarations for mock functions
void _error(int code, const char* file, int line);
uint init_list(uint data);
int append_list(undefined4 *list_head_ptr, undefined4 data, int flag, uint context);

// Tree node structure based on memory allocation (0x14 bytes for 5 uints)
// Pointers to child nodes are stored as uints. This design is common in
// binaries from 32-bit environments where sizeof(uint) == sizeof(void*) == 4.
// For robust compilation on both 32-bit and 64-bit Linux, we explicitly cast
// between uint and TreeNode* using uintptr_t.
typedef struct TreeNode {
    uint val1;              // Corresponds to *puVar1 or param_1[0]
    uint val2;              // Corresponds to puVar1[1] or param_1[1]
    uint list_ptr;          // Corresponds to puVar1[2] or param_1[2] (likely a pointer to a list head)
    uint left_child_ptr;    // Corresponds to puVar1[3] or param_1[3], interpreted as a pointer
    uint right_child_ptr;   // Corresponds to puVar1[4] or param_1[4], interpreted as a pointer
} TreeNode;

// Mock implementations for missing external functions
// In a real scenario, these would be defined elsewhere or linked.
void _error(int code, const char* file, int line) {
    fprintf(stderr, "Error %d at %s:%d\n", code, file, line);
    exit(code);
}

uint init_list(uint data) {
    // Mock: Returns a non-zero value as a dummy list head, 0 for error.
    // In a real implementation, this would allocate and initialize a list structure.
    (void)data; // Suppress unused parameter warning
    static uint next_list_id = 1; // Simple counter for unique IDs
    return next_list_id++;
}

int append_list(undefined4 *list_head_ptr, undefined4 data, int flag, uint context) {
    // Mock: Simulates appending to a list, just "succeeds".
    // In a real implementation, this would modify the list pointed to by list_head_ptr.
    (void)data; (void)flag; (void)context;
    if (list_head_ptr == NULL) {
        return -1; // Indicate error if list_head_ptr is invalid
    }
    // For demonstration, we just return success.
    return 0;
}

// Function: init_tree
TreeNode * init_tree(uint param_1, uint param_2, int param_3) {
  TreeNode *newNode;
  
  // Allocate space for one TreeNode (sizeof(TreeNode) should be 20 bytes on a 32-bit system)
  newNode = (TreeNode *)calloc(1, sizeof(TreeNode));
  if (newNode == NULL) {
    _error(1,
           "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/cgc-challenge-corpus/KPRCA_00031/src/tree.c"
           ,0x22);
  }
  
  // Initialize node fields only if param_1, param_2, and param_3 are non-zero.
  // Otherwise, fields remain zero due to calloc.
  if (((param_1 | param_2) != 0) && (param_3 != 0)) {
    newNode->val1 = param_1;
    newNode->val2 = param_2;
    newNode->list_ptr = init_list(param_3); // Directly assign the result of init_list
  }
  return newNode;
}

// Function: get_tree
uint get_tree(TreeNode *node, uint param_2, uint param_3) {
  if (node == NULL) {
    return 0; // Return 0 for an empty subtree or if the key is not found
  }
  
  // Lexicographical comparison (param_3, param_2) against (node->val2, node->val1)
  // This determines whether to go left, right, or if a match is found.
  if (param_3 < node->val2 || (param_3 == node->val2 && param_2 < node->val1)) {
    // Go left: cast the stored uint pointer to TreeNode* for the recursive call
    return get_tree((TreeNode *)(uintptr_t)node->left_child_ptr, param_2, param_3);
  }
  else if (param_3 > node->val2 || (param_3 == node->val2 && param_2 > node->val1)) {
    // Go right: cast the stored uint pointer to TreeNode* for the recursive call
    return get_tree((TreeNode *)(uintptr_t)node->right_child_ptr, param_2, param_3);
  }
  else { // Keys match: param_3 == node->val2 && param_2 == node->val1
    return node->list_ptr; // Return the list associated with this node
  }
}

// Function: ins_tree
undefined4 ins_tree(TreeNode *node, uint param_2, uint param_3, undefined4 param_4) {
  if (node == NULL) {
    // This case usually indicates an attempt to insert into a non-existent tree root,
    // or an error in tree traversal logic.
    return 0xffffffff; // Error: Cannot insert into a NULL tree
  }

  uint *target_child_ptr_addr; // Pointer to either node->left_child_ptr or node->right_child_ptr
  
  // Lexicographical comparison (param_3, param_2) against (node->val2, node->val1)
  if (param_3 < node->val2 || (param_3 == node->val2 && param_2 < node->val1)) {
    target_child_ptr_addr = &(node->left_child_ptr);
  }
  else if (param_3 > node->val2 || (param_3 == node->val2 && param_2 > node->val1)) {
    target_child_ptr_addr = &(node->right_child_ptr);
  }
  else { // Keys match: param_3 == node->val2 && param_2 == node->val1
    // If keys match, append param_4 to the list associated with this node
    return append_list(&node->list_ptr, param_4, 0, 0x12c19);
  }

  // If the target child pointer is NULL (0), create a new node
  if (*target_child_ptr_addr == 0) {
    TreeNode *new_child = init_tree(param_2, param_3, param_4);
    *target_child_ptr_addr = (uint)(uintptr_t)new_child; // Store the new child's address as a uint
    // Return 0 on success, 0xffffffff on error (if init_tree failed to allocate)
    return (new_child == NULL) ? 0xffffffff : 0;
  } else {
    // Otherwise, recursively call ins_tree on the existing child
    return ins_tree((TreeNode *)(uintptr_t)*target_child_ptr_addr, param_2, param_3, param_4);
  }
}

// Function: num_nodes
int num_nodes(TreeNode *node) {
  if (node == NULL) {
    return 0; // An empty subtree has 0 nodes
  }
  // Recursively count nodes: 1 for current node + nodes in left subtree + nodes in right subtree
  return 1 + num_nodes((TreeNode *)(uintptr_t)node->left_child_ptr) + 
             num_nodes((TreeNode *)(uintptr_t)node->right_child_ptr);
}

// Function: _tree_to_list (recursive helper)
undefined4 _tree_to_list(TreeNode *node, undefined4 *list_head_ptr) {
  if (node == NULL) {
    return 0; // Success for an empty subtree
  }
  
  // Append current node's list_ptr to the main list being built
  int ret = append_list(list_head_ptr, node->list_ptr, 1, 0x12d3b);
  if (ret < 0) {
    return 0xffffffff; // Propagate error
  }

  // Recurse left subtree
  ret = _tree_to_list((TreeNode *)(uintptr_t)node->left_child_ptr, list_head_ptr);
  if (ret < 0) {
    return 0xffffffff; // Propagate error
  }

  // Recurse right subtree
  ret = _tree_to_list((TreeNode *)(uintptr_t)node->right_child_ptr, list_head_ptr);
  if (ret < 0) {
    return 0xffffffff; // Propagate error
  }
  
  return 0; // Success
}

// Function: tree_to_list (main entry point)
undefined4 tree_to_list(TreeNode *node) {
  if (node == NULL) {
    return 0; // An empty tree produces an empty list, represented by 0
  }
  
  undefined4 list_head; // This variable will hold the head of the newly created list
  
  // Initialize the new list with the list_ptr from the current node
  list_head = init_list(node->list_ptr);
  if (list_head == 0) { // Assuming 0 indicates an error or invalid list head from init_list
    return 0; // Return 0 on error
  }

  // Recursively traverse the left subtree and append its elements to the list
  int ret = _tree_to_list((TreeNode *)(uintptr_t)node->left_child_ptr, &list_head); // Pass address of list_head
  if (ret < 0) {
    // In a real application, proper error handling (e.g., freeing partially built list) would be needed
    return 0; // Return 0 on error
  }

  // Recursively traverse the right subtree and append its elements to the list
  ret = _tree_to_list((TreeNode *)(uintptr_t)node->right_child_ptr, &list_head); // Pass address of list_head
  if (ret < 0) {
    // In a real application, proper error handling would be needed
    return 0; // Return 0 on error
  }
  
  return list_head; // Return the head of the newly created list
}