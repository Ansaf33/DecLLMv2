#include <stdlib.h> // For calloc, free
#include <string.h> // For strcmp, strncpy
#include <stdio.h>  // For printf
#include <stdint.h> // For uint32_t

// The original code implies a fixed-size buffer for string data within linked list nodes,
// and the 'next' pointer for collision resolution is at a fixed offset (0x400).
// This design is unusual but must be respected to match the original logic.
// DATA_LIST_NODE_SIZE (0x400 or 1024 bytes) is the total size of the data_list_node_t struct.
#define DATA_LIST_NODE_SIZE 0x400

// Structure for the linked list of strings at a given BST node (for key collisions)
typedef struct data_list_node {
    // String data buffer. Its size is DATA_LIST_NODE_SIZE minus the size of the 'next' pointer.
    // This allows the 'next' pointer to be effectively at offset DATA_LIST_NODE_SIZE - sizeof(void*)
    // from the start of the struct, matching the original code's 0x400 offset usage.
    char str_data[DATA_LIST_NODE_SIZE - sizeof(struct data_list_node*)];
    struct data_list_node *next; // Pointer to the next string in the collision list.
} data_list_node_t;

// Structure for a node in the Binary Search Tree (BST)
typedef struct node {
    data_list_node_t *data_list_head; // Head of a linked list storing strings that map to this node's key.
    uint32_t key;                      // The numeric key derived from the string.
    uint32_t count;                    // Number of strings stored in data_list_head for this node.
    struct node *left;                 // Pointer to the left child node.
    struct node *right;                // Pointer to the right child node.
} node_t; // Total size 20 bytes (0x14), matching original calloc(1, 0x14)

// Function pointer type for generating a key from a string.
typedef uint32_t (*KeyGenerator)(const char*);

// Global constant for printf format string to fix potential error
static const char *const PRINTF_STRING_FORMAT = "%s\n";

// Placeholder for str_token / make_key_from_name
// A simple DJB2 hash function for string to key conversion.
uint32_t str_to_key(const char *str) {
    if (!str) return 0;
    uint32_t hash = 5381;
    for (int c = *str; c; c = *++str) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Function: insert_node
// Inserts a new string into the BST. If the key exists, adds to collision list.
// Returns 0 on success, -1 on failure.
int insert_node(node_t **root, const char *str_data, KeyGenerator key_gen_func) {
    if (!str_data) return -1;

    uint32_t key = key_gen_func(str_data);
    if (key == 0) return -1; // Assuming 0 is an invalid key

    // If the tree is empty, create the root node
    if (!*root) {
        *root = calloc(1, sizeof(node_t));
        if (!*root) return -1;
        (*root)->key = key;

        data_list_node_t *new_data_node = calloc(1, DATA_LIST_NODE_SIZE);
        if (!new_data_node) {
            free(*root); // Clean up partially allocated node
            *root = NULL;
            return -1;
        }
        strncpy(new_data_node->str_data, str_data, sizeof(new_data_node->str_data) - 1);
        new_data_node->str_data[sizeof(new_data_node->str_data) - 1] = '\0';
        (*root)->data_list_head = new_data_node;
        (*root)->count = 1;
        return 0;
    }

    // Traverse the tree to find the insertion point
    node_t *current_node = *root;
    while (1) {
        if (key < current_node->key) { // Go left
            if (!current_node->left) {
                current_node->left = calloc(1, sizeof(node_t));
                if (!current_node->left) return -1;
                current_node->left->key = key;

                data_list_node_t *new_data_node = calloc(1, DATA_LIST_NODE_SIZE);
                if (!new_data_node) {
                    free(current_node->left);
                    current_node->left = NULL;
                    return -1;
                }
                strncpy(new_data_node->str_data, str_data, sizeof(new_data_node->str_data) - 1);
                new_data_node->str_data[sizeof(new_data_node->str_data) - 1] = '\0';
                current_node->left->data_list_head = new_data_node;
                current_node->left->count = 1;
                return 0;
            }
            current_node = current_node->left;
        } else if (key > current_node->key) { // Go right
            if (!current_node->right) {
                current_node->right = calloc(1, sizeof(node_t));
                if (!current_node->right) return -1;
                current_node->right->key = key;

                data_list_node_t *new_data_node = calloc(1, DATA_LIST_NODE_SIZE);
                if (!new_data_node) {
                    free(current_node->right);
                    current_node->right = NULL;
                    return -1;
                }
                strncpy(new_data_node->str_data, str_data, sizeof(new_data_node->str_data) - 1);
                new_data_node->str_data[sizeof(new_data_node->str_data) - 1] = '\0';
                current_node->right->data_list_head = new_data_node;
                current_node->right->count = 1;
                return 0;
            }
            current_node = current_node->right;
        } else { // Key matches, handle collision by adding to linked list
            data_list_node_t *data_node_ptr = current_node->data_list_head;
            data_list_node_t *prev_data_node = NULL;

            // Check if string already exists in the collision list
            while (data_node_ptr) {
                if (strcmp(data_node_ptr->str_data, str_data) == 0) return 0; // String already present
                prev_data_node = data_node_ptr;
                data_node_ptr = data_node_ptr->next;
            }

            // String not found, add it to the end of the collision list
            data_list_node_t *new_data_node = calloc(1, DATA_LIST_NODE_SIZE);
            if (!new_data_node) return -1;
            strncpy(new_data_node->str_data, str_data, sizeof(new_data_node->str_data) - 1);
            new_data_node->str_data[sizeof(new_data_node->str_data) - 1] = '\0';
            
            if (prev_data_node) {
                prev_data_node->next = new_data_node;
            } else { // This case should ideally not be reached if key matches and data_list_head exists
                current_node->data_list_head = new_data_node;
            }
            current_node->count++;
            return 0;
        }
    }
}

// Function: delete_node
// Deletes a specific string from the BST. If it's the last string for a key, the BST node is deleted.
// Returns 0 on success, -1 on failure (e.g., node not found)
int delete_node(node_t **root, const char *str_data_to_delete, uint32_t key_to_delete) {
    if (!*root) return 0; // Tree is empty, nothing to delete

    node_t *current_node = *root;
    node_t *parent_node = NULL; // Keep track of parent for linking
    
    // Find the node with the matching key
    while (current_node && current_node->key != key_to_delete) {
        parent_node = current_node;
        if (key_to_delete < current_node->key) {
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }
    }

    if (!current_node) return 0; // Key not found

    // Found the node (current_node) with the key_to_delete.
    // Now, handle the string deletion within its collision list.
    data_list_node_t *data_node_ptr = current_node->data_list_head;
    data_list_node_t *prev_data_node = NULL;

    while (data_node_ptr) {
        if (strcmp(data_node_ptr->str_data, str_data_to_delete) == 0) {
            // Found the string to delete
            if (prev_data_node) {
                prev_data_node->next = data_node_ptr->next;
            } else {
                current_node->data_list_head = data_node_ptr->next;
            }
            free(data_node_ptr);
            current_node->count--;

            // If no more strings in this node's collision list, delete the BST node itself
            if (current_node->count == 0) {
                // Case 1: Node has no children (leaf node)
                if (!current_node->left && !current_node->right) {
                    if (!parent_node) { // Deleting root node
                        free(current_node);
                        *root = NULL;
                    } else if (parent_node->left == current_node) {
                        parent_node->left = NULL;
                        free(current_node);
                    } else {
                        parent_node->right = NULL;
                        free(current_node);
                    }
                }
                // Case 2: Node has one child
                else if (!current_node->left) { // Only right child
                    if (!parent_node) { // Deleting root
                        *root = current_node->right;
                        free(current_node);
                    } else if (parent_node->left == current_node) {
                        parent_node->left = current_node->right;
                        free(current_node);
                    } else {
                        parent_node->right = current_node->right;
                        free(current_node);
                    }
                } else if (!current_node->right) { // Only left child
                    if (!parent_node) { // Deleting root
                        *root = current_node->left;
                        free(current_node);
                    } else if (parent_node->left == current_node) {
                        parent_node->left = current_node->left;
                        free(current_node);
                    } else {
                        parent_node->right = current_node->left;
                        free(current_node);
                    }
                }
                // Case 3: Node has two children
                else {
                    // Find the in-order successor (smallest in the right subtree)
                    node_t *successor_parent = current_node;
                    node_t *successor = current_node->right;
                    while (successor->left) {
                        successor_parent = successor;
                        successor = successor->left;
                    }

                    // Copy successor's data to current_node
                    current_node->key = successor->key;
                    current_node->data_list_head = successor->data_list_head; // Transfer head and count
                    current_node->count = successor->count;

                    // Delete the successor from its original position
                    if (successor_parent->left == successor) {
                        successor_parent->left = successor->right;
                    } else { // successor_parent->right == successor
                        successor_parent->right = successor->right;
                    }
                    // Successor's data_list_head was transferred, so don't free it here.
                    // Set it to NULL to prevent double free.
                    successor->data_list_head = NULL; 
                    free(successor);
                }
            }
            return 0; // Successfully deleted string and possibly node
        }
        prev_data_node = data_node_ptr;
        data_node_ptr = data_node_ptr->next;
    }
    return 0; // String not found in the collision list, nothing deleted.
}

// Function: find_node_by_key
// Returns a pointer to the head of the data list (data_list_node_t*) if key found, otherwise NULL.
data_list_node_t *find_node_by_key(node_t *root, uint32_t search_key) {
    node_t *current_node = root;
    while (current_node) {
        if (search_key == current_node->key) {
            return current_node->data_list_head;
        }
        if (search_key < current_node->key) {
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }
    }
    return NULL;
}

// Function: walk_tree (in-order traversal)
// Prints all strings in the tree in sorted order by key.
// Returns 0 on success.
int walk_tree(node_t *node) {
    if (!node) return 0;

    if (node->left) {
        walk_tree(node->left);
    }

    // Print all strings in the collision list for this node
    data_list_node_t *data_node_ptr = node->data_list_head;
    while (data_node_ptr) {
        printf(PRINTF_STRING_FORMAT, data_node_ptr->str_data);
        data_node_ptr = data_node_ptr->next;
    }

    if (node->right) {
        walk_tree(node->right);
    }
    return 0;
}

// Function: make_key_from_name (alias for str_to_key)
uint32_t make_key_from_name(const char *name) {
    return str_to_key(name);
}

// Helper function: count_nodes (recursive helper for count_bst_stats)
// Counts the number of nodes in a subtree.
static int count_nodes(node_t *node, int *count_ptr) {
    if (!node) return 0;
    if (node->left) {
        count_nodes(node->left, count_ptr);
    }
    (*count_ptr)++; // Count this node
    if (node->right) {
        count_nodes(node->right, count_ptr);
    }
    return 0;
}

// Function: count_bst_stats
// Calculates statistics about the BST.
// param_2 points to an array of uint32_t: [total_nodes, left_imbalance_percent, right_imbalance_percent]
// Returns 0 on success, -1 on failure.
int count_bst_stats(node_t *root, uint32_t *stats_array) {
    if (!root || !stats_array) {
        return -1;
    }

    int left_subtree_nodes = 0;
    int right_subtree_nodes = 0;

    if (root->left) {
        count_nodes(root->left, &left_subtree_nodes);
    }
    if (root->right) {
        count_nodes(root->right, &right_subtree_nodes);
    }

    uint32_t total_nodes = left_subtree_nodes + right_subtree_nodes + 1; // +1 for the root itself
    stats_array[0] = total_nodes;

    if (total_nodes < 2) { // If only root or empty, no imbalance
        stats_array[1] = 0;
        stats_array[2] = 0;
    } else {
        // Imbalance is calculated as percentage of nodes in left/right subtrees relative to (total_nodes - 1)
        stats_array[1] = (uint32_t)((double)left_subtree_nodes * 100.0 / (total_nodes - 1));
        stats_array[2] = (uint32_t)((double)right_subtree_nodes * 100.0 / (total_nodes - 1));
    }
    return 0;
}

// Main function for compilation and basic demonstration
int main() {
    node_t *root = NULL;
    uint32_t stats[3]; // total_nodes, left_imbalance_percent, right_imbalance_percent

    // Example usage:
    insert_node(&root, "apple", str_to_key);
    insert_node(&root, "banana", str_to_key);
    insert_node(&root, "cherry", str_to_key);
    insert_node(&root, "date", str_to_key);
    insert_node(&root, "banana", str_to_key); // Duplicate string, should be added to collision list

    printf("Tree content (in-order):\n");
    walk_tree(root);

    printf("\nFinding 'banana' (key %u):\n", make_key_from_name("banana"));
    data_list_node_t *found = find_node_by_key(root, make_key_from_name("banana"));
    if (found) {
        printf("Found: %s\n", found->str_data);
        while(found->next) { // Print collision list items
            found = found->next;
            printf("  (collision) %s\n", found->str_data);
        }
    } else {
        printf("Not found.\n");
    }

    printf("\nBST Stats:\n");
    if (count_bst_stats(root, stats) == 0) {
        printf("Total nodes: %u\n", stats[0]);
        printf("Left imbalance: %u%%\n", stats[1]);
        printf("Right imbalance: %u%%\n", stats[2]);
    }

    printf("\nDeleting 'banana' (one instance)...\n");
    delete_node(&root, "banana", make_key_from_name("banana"));
    printf("Tree content after deleting 'banana':\n");
    walk_tree(root);

    printf("\nDeleting 'date'...\n");
    delete_node(&root, "date", make_key_from_name("date"));
    printf("Tree content after deleting 'date':\n");
    walk_tree(root);

    // Clean up the entire tree
    void destroy_tree(node_t *node) {
        if (!node) return;
        destroy_tree(node->left);
        destroy_tree(node->right);
        data_list_node_t *current_data = node->data_list_head;
        while (current_data) {
            data_list_node_t *temp = current_data;
            current_data = current_data->next;
            free(temp);
        }
        free(node);
    }
    destroy_tree(root);
    root = NULL; // Ensure root is NULL after destruction

    return 0;
}