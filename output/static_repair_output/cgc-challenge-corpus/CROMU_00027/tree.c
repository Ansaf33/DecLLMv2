#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h> // For size_t

// Define the TreeNode structure based on offsets
// 0x00 - 0x3F: name (char[64])
// 0x40: TreeNode* next_sibling
// 0x44: TreeNode* first_child
// 0x48: size_t page_size
// 0x4C: void* page
typedef struct TreeNode {
    char name[64];
    struct TreeNode *next_sibling; // Offset 0x40
    struct TreeNode *first_child;  // Offset 0x44
    size_t page_size;              // Offset 0x48
    void *page;                    // Offset 0x4C
} TreeNode; // Total size 0x50 bytes

// Global root pointer
TreeNode *root = NULL;

// Dummy global data for compilation. Actual values are unknown.
// InitialInfo is an array of 0x44 byte structures, where the first 0x40 bytes are a string.
char InitialInfo_data[20][0x44];

// Structure to match the 0x44 byte indexing for DAT_00019040
typedef struct InitialPageInfo {
    char *page_content_ptr;
    char dummy_padding[0x44 - sizeof(char*)]; // Ensure struct size is 0x44 bytes
} InitialPageInfo;

InitialPageInfo DAT_00019040[20]; // Array of 20 such structs

// Function prototypes
void VerifyPointerOrTerminate(void *ptr, const char *msg);
TreeNode *LookupNode(const char *name_path);
int InsertNodeInTree(TreeNode *node);
void WalkTree(TreeNode *start_node);
void FreeTree(TreeNode *node);

// Dummy initialization for global data (for compilation purposes)
void init_dummy_globals() {
    for (int i = 0; i < 20; ++i) {
        snprintf(InitialInfo_data[i], 0x40, "node%d", i);
        // For simplicity, let the page content be the same as the node name
        DAT_00019040[i].page_content_ptr = InitialInfo_data[i];
    }
}

// Function: VerifyPointerOrTerminate
void VerifyPointerOrTerminate(void *ptr, const char *msg) {
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed for %s\n", msg);
        exit(EXIT_FAILURE);
    }
}

// Function: InitializeTree
int InitializeTree(void) {
    // Initialize dummy globals for the example to work
    init_dummy_globals();

    for (int i = 0; i <= 0x13; ++i) { // Loop from 0 to 19 (20 times)
        TreeNode *newNode = (TreeNode *)calloc(1, sizeof(TreeNode));
        VerifyPointerOrTerminate(newNode, "TreeNode during initialization");

        // Copy name (full path for now, will be modified by InsertNodeInTree)
        strncpy(newNode->name, InitialInfo_data[i], sizeof(newNode->name) - 1);
        newNode->name[sizeof(newNode->name) - 1] = '\0'; // Ensure null termination

        // Get page content pointer from DAT_00019040
        char *pageContent = DAT_00019040[i].page_content_ptr;
        newNode->page_size = strlen(pageContent) + 1; // +1 for null terminator

        newNode->page = calloc(1, newNode->page_size);
        VerifyPointerOrTerminate(newNode->page, "node->page during initialization");

        memcpy(newNode->page, pageContent, newNode->page_size);

        if (InsertNodeInTree(newNode) != 0) {
            // If InsertNodeInTree fails, free allocated memory for this node and return error
            free(newNode->page);
            free(newNode);
            return -1; // 0xffffffff
        }
    }
    return 0;
}

// Function: WalkTree
#define MAX_STACK_DEPTH 32 // Max depth of tree to prevent stack overflow for iterative traversal
void WalkTree(TreeNode *start_node) {
    if (start_node == NULL) {
        return;
    }

    struct StackFrame {
        TreeNode *node;
        int depth;
    } stack_frames[MAX_STACK_DEPTH];
    int stack_idx = 0; // Stack pointer

    // The original code prints the starting node's name first (at depth 0).
    // It then pushes its first_child (if any) to be processed by the loop.
    printf("%s\n", start_node->name);

    if (start_node->first_child != NULL) {
        if (stack_idx >= MAX_STACK_DEPTH) {
            fprintf(stderr, "ERROR: WalkTree stack overflow for initial child push\n");
            return;
        }
        stack_frames[stack_idx].node = start_node->first_child;
        stack_frames[stack_idx].depth = 1; // Child of root is at depth 1
        stack_idx++;
    }

    while (stack_idx > 0) {
        // Pop the current node and its depth
        stack_idx--;
        TreeNode *current_node = stack_frames[stack_idx].node;
        int current_depth = stack_frames[stack_idx].depth;

        // Print indentation
        for (int i = 0; i < current_depth; ++i) {
            printf("    ");
        }
        printf("%s\n", current_node->name);

        // Push children and siblings onto the stack for later processing (LIFO).
        // To process children before siblings (standard pre-order), push sibling first, then child.
        // This ensures child is at the top of the stack and processed next.
        if (current_node->next_sibling != NULL) {
            if (stack_idx >= MAX_STACK_DEPTH) {
                fprintf(stderr, "ERROR: WalkTree stack overflow pushing sibling\n");
                return;
            }
            stack_frames[stack_idx].node = current_node->next_sibling;
            stack_frames[stack_idx].depth = current_depth; // Sibling is at the same depth
            stack_idx++;
        }

        if (current_node->first_child != NULL) {
            if (stack_idx >= MAX_STACK_DEPTH) {
                fprintf(stderr, "ERROR: WalkTree stack overflow pushing child\n");
                return;
            }
            stack_frames[stack_idx].node = current_node->first_child;
            stack_frames[stack_idx].depth = current_depth + 1;
            stack_idx++;
        }
    }
}

// Function: PrintTree
int PrintTree(const char *name_path) {
    TreeNode *node_to_print;

    if (name_path == NULL || *name_path == '\0') {
        node_to_print = root;
    } else {
        node_to_print = LookupNode(name_path);
    }

    if (node_to_print == NULL) {
        printf("ERROR: Tree not found: %s\n", name_path);
        return -1; // 0xffffffff
    } else {
        WalkTree(node_to_print);
        return 0;
    }
}

// Function: FreeTree
void FreeTree(TreeNode *node) {
    if (node == NULL) {
        return;
    }
    // Recursively free first_child subtree
    FreeTree(node->first_child);
    // Recursively free next_sibling subtree
    FreeTree(node->next_sibling);
    // Free the page data if it exists
    if (node->page != NULL) {
        free(node->page);
    }
    // Free the node itself
    free(node);
}

// Function: DeleteNode
int DeleteNode(const char *name_path) {
    TreeNode *node_to_delete = LookupNode(name_path);
    if (node_to_delete == NULL) {
        printf("ERROR: Could not locate node for deletion\n");
        return -1;
    }

    char parent_path_buffer[sizeof(((TreeNode*)0)->name)];
    strncpy(parent_path_buffer, name_path, sizeof(parent_path_buffer) - 1);
    parent_path_buffer[sizeof(parent_path_buffer) - 1] = '\0';

    char *last_dot = strrchr(parent_path_buffer, '.');
    TreeNode *parent_node;

    if (last_dot == NULL) {
        // Node to delete is a top-level child of the root (e.g., "node1" where root is ".")
        parent_node = root;
    } else {
        *last_dot = '\0'; // Null-terminate to get the parent's path
        parent_node = LookupNode(parent_path_buffer);
    }

    if (parent_node == NULL) {
        printf("ERROR: Could not locate parent node for deletion\n");
        return -1;
    }

    // Now, find `node_to_delete` in `parent_node`'s children list and remove it.
    // Case 1: `node_to_delete` is the first child of `parent_node`
    if (parent_node->first_child == node_to_delete) {
        parent_node->first_child = node_to_delete->next_sibling;
        node_to_delete->next_sibling = NULL; // Isolate the node to delete
    } else {
        // Case 2: `node_to_delete` is a sibling, not the first child
        // Iterate through siblings to find the one *before* node_to_delete
        TreeNode *current_sibling = parent_node->first_child;
        while (current_sibling != NULL && current_sibling->next_sibling != node_to_delete) {
            current_sibling = current_sibling->next_sibling;
        }

        if (current_sibling == NULL) {
            // This means node_to_delete was not found in the sibling list
            // (e.g., it's not a child of parent_node, or logic error)
            printf("ERROR: Could not locate node for deletion within parent's children list\n");
            return -1;
        }

        // current_sibling is the node *before* node_to_delete in the sibling list
        current_sibling->next_sibling = node_to_delete->next_sibling;
        node_to_delete->next_sibling = NULL; // Isolate the node to delete
    }

    FreeTree(node_to_delete); // Free the isolated node and its children
    return 0;
}

// Function: LookupNode
TreeNode *LookupNode(const char *name_path) {
    if (name_path == NULL || root == NULL) {
        return NULL;
    }

    // Handle the special case of looking up the root node itself
    if (strcmp(name_path, root->name) == 0) {
        return root;
    }

    char path_buffer[sizeof(((TreeNode*)0)->name)];
    strncpy(path_buffer, name_path, sizeof(path_buffer) - 1);
    path_buffer[sizeof(path_buffer) - 1] = '\0';

    char *segment_start = path_buffer;
    char *next_dot;
    TreeNode *current_node_list_head = root->first_child; // Start search from root's first child

    while (1) {
        next_dot = strchr(segment_start, '.');
        if (next_dot != NULL) {
            *next_dot = '\0'; // Null-terminate the current segment
        }

        TreeNode *found_segment_node = NULL;
        // Search for the current segment in the current_node_list_head's sibling list
        TreeNode *traverse_node = current_node_list_head;
        while (traverse_node != NULL) {
            if (strcmp(traverse_node->name, segment_start) == 0) {
                found_segment_node = traverse_node;
                break;
            }
            traverse_node = traverse_node->next_sibling;
        }

        if (found_segment_node == NULL) {
            return NULL; // Segment not found
        }

        if (next_dot == NULL) {
            return found_segment_node; // This was the last segment, return the found node
        }

        // Move to the child list of the found segment node for the next segment
        current_node_list_head = found_segment_node->first_child;
        segment_start = next_dot + 1; // Start of the next segment
    }
}

// Function: InsertNodeInTree
int InsertNodeInTree(TreeNode *newNode) {
    if (newNode == NULL) {
        return -1;
    }

    // Ensure root exists
    if (root == NULL) {
        root = (TreeNode *)calloc(1, sizeof(TreeNode));
        VerifyPointerOrTerminate(root, "root TreeNode during insert");
        strncpy(root->name, ".", sizeof(root->name) - 1); // Root name is typically "."
        root->name[sizeof(root->name) - 1] = '\0';
    }

    // Check if node with this full path already exists
    if (LookupNode(newNode->name) != NULL) {
        printf("ERROR: node already exists: %s\n", newNode->name);
        return -1;
    }

    if (newNode->name[0] == '\0') {
        printf("ERROR: Name cannot be blank\n");
        return -1;
    }

    char full_path_buffer[sizeof(newNode->name)];
    strncpy(full_path_buffer, newNode->name, sizeof(full_path_buffer) - 1);
    full_path_buffer[sizeof(full_path_buffer) - 1] = '\0';

    char *last_dot = strrchr(full_path_buffer, '.');
    TreeNode *parent_node;
    TreeNode **target_list_head_ptr; // Pointer to the head of the sibling list where newNode should be inserted

    if (last_dot == NULL) {
        // Node is a top-level child of the root node
        parent_node = root;
        target_list_head_ptr = &root->first_child;
    } else {
        *last_dot = '\0'; // Null-terminate to get parent's path
        parent_node = LookupNode(full_path_buffer);

        if (parent_node == NULL) {
            printf("ERROR: Parent node doesn't exist: %s\n", full_path_buffer);
            return -1;
        }
        target_list_head_ptr = &parent_node->first_child;
    }

    // Modify the new node's name to be just the segment name.
    // This is a destructive modification of `newNode->name`.
    strncpy(newNode->name, (last_dot == NULL) ? full_path_buffer : last_dot + 1, sizeof(newNode->name) - 1);
    newNode->name[sizeof(newNode->name) - 1] = '\0';

    // Insert into the sibling list
    if (*target_list_head_ptr == NULL) {
        // No children/siblings yet, new node becomes the first child
        *target_list_head_ptr = newNode;
    } else {
        // Append to the end of the sibling list
        TreeNode *current = *target_list_head_ptr;
        while (current->next_sibling != NULL) {
            current = current->next_sibling;
        }
        current->next_sibling = newNode;
    }

    return 0;
}