#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For calloc, free, exit, EXIT_FAILURE
#include <string.h>  // For strncpy, strlen, memcpy, strrchr, strchr, strcmp
#include <stdint.h>  // For uintptr_t (if needed for fixed-size pointers, but using struct* is safer)

// Assuming a 32-bit system for structure packing and pointer sizes
// to match the original code's fixed offsets (e.g., 0x50 total size, 0x40, 0x44, 0x48, 0x4c for members).
// On a 64-bit system, standard pointers and size_t are 8 bytes, which would change the layout.
// For robust compilation, a specific compilation target (e.g., -m32) might be needed,
// or explicit `__attribute__((packed))` and `uint32_t` for pointer types.
// For this solution, we assume the compiler aligns `struct TreeNode` to match the intent,
// or that the code will be compiled in a 32-bit environment where `sizeof(void*)` and `sizeof(size_t)` are 4 bytes.
struct TreeNode {
    char name[64];          // Offset 0x00
    struct TreeNode* left;  // Offset 0x40 (64 bytes)
    struct TreeNode* right; // Offset 0x44
    size_t page_size;       // Offset 0x48
    void* page_data;        // Offset 0x4C
}; // Total size 0x50 (80 bytes) on a 32-bit system

// Global variables
struct TreeNode* root = NULL;

// Structure to hold initial data, matching the 0x44 byte stride
// 64 bytes for name_buffer + 4 bytes for char* (on 32-bit) = 68 bytes (0x44)
struct InitialNodeSource {
    char name_buffer[64];
    char* page_data_ptr;
} initial_tree_data[20] = { // 0x13 < local_10 implies 0 to 19 (20 entries)
    {"root", "Root page content."},
    {"child1", "Child 1 page content."},
    {"child1.sub1", "Child 1 Sub 1 page content."},
    {"child1.sub2", "Child 1 Sub 2 page content."},
    {"child2", "Child 2 page content."},
    {"child2.sub1", "Child 2 Sub 1 page content."},
    {"child2.sub2", "Child 2 Sub 2 page content."},
    {"child2.sub3", "Child 2 Sub 3 page content."},
    {"child2.sub2.nested", "Nested page content."},
    {"child3", "Child 3 page content."},
    {"child3.sub1", "Child 3 Sub 1 page content."},
    {"child3.sub2", "Child 3 Sub 2 page content."},
    {"child4", "Child 4 page content."},
    {"child4.sub1", "Child 4 Sub 1 page content."},
    {"child4.sub2", "Child 4 Sub 2 page content."},
    {"child5", "Child 5 page content."},
    {"child5.sub1", "Child 5 Sub 1 page content."},
    {"child5.sub2", "Child 5 Sub 2 page content."},
    {"child5.sub3", "Child 5 Sub 3 page content."},
    {"child5.sub3.deep", "Deeply nested page content."}
};

// Forward declarations for functions used before definition
void VerifyPointerOrTerminate(void* ptr, const char* msg);
struct TreeNode* LookupNode(const char* name);
unsigned int InsertNodeInTree(struct TreeNode* node);
void FreeTree(struct TreeNode* node);

// Function: VerifyPointerOrTerminate
void VerifyPointerOrTerminate(void* ptr, const char* msg) {
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate %s\n", msg);
        exit(EXIT_FAILURE);
    }
}

// Function: InitializeTree
unsigned int InitializeTree(void) {
    for (int i = 0; i <= 0x13; i++) { // Loop 0 to 19 (0x13)
        struct TreeNode* newNode = (struct TreeNode*)calloc(1, sizeof(struct TreeNode));
        VerifyPointerOrTerminate(newNode, "TreeNode during initialization");

        // Copy name from initial_tree_data
        strncpy(newNode->name, initial_tree_data[i].name_buffer, sizeof(newNode->name) - 1);
        newNode->name[sizeof(newNode->name) - 1] = '\0'; // Ensure null-termination

        // Get page content source and calculate size
        char* page_content_src = initial_tree_data[i].page_data_ptr;
        newNode->page_size = strlen(page_content_src) + 1; // +1 for null terminator

        newNode->page_data = calloc(1, newNode->page_size);
        VerifyPointerOrTerminate(newNode->page_data, "node->page during initialization");

        memcpy(newNode->page_data, page_content_src, newNode->page_size);

        if (InsertNodeInTree(newNode) != 0) {
            // Error during insertion, free allocated memory for this node
            free(newNode->page_data);
            free(newNode);
            return 0xFFFFFFFF; // Return error code
        }
    }
    return 0; // Success
}

// Function: WalkTree
// Performs an iterative pre-order traversal using a stack.
// Assumes 'right' is the first child and 'left' is the next sibling.
void WalkTree(struct TreeNode* node) {
    if (node == NULL) return;

    // Stack entry to store (node pointer, depth) pairs
    struct StackEntry {
        struct TreeNode* node;
        int depth;
    };
    // Assuming a max depth of 32 for the stack, 32 * (ptr_size + int_size)
    // On 32-bit: 32 * (4+4) = 256 bytes.
    struct StackEntry stack[32];
    int stack_ptr = 0; // Points to the next available slot

    // Print the starting node at depth 0
    printf("%s\n", node->name);

    // If the starting node has a first child (right pointer), push it onto the stack.
    if (node->right != NULL) {
        stack[stack_ptr].node = node->right;
        stack[stack_ptr].depth = 1; // First child is at depth 1
        stack_ptr++;
    }

    // Main loop: pop, process, then push children/siblings
    while (stack_ptr > 0) {
        stack_ptr--; // Pop an entry
        struct TreeNode* current_node = stack[stack_ptr].node;
        int current_depth = stack[stack_ptr].depth;

        // Print current node with indentation
        for (int i = 0; i < current_depth; i++) {
            printf("    "); // Indentation for depth level
        }
        printf("%s\n", current_node->name);

        // Push children/siblings onto the stack (LIFO order for pre-order traversal)
        // Push next sibling (left pointer) first, so it's processed after the child's subtree.
        if (current_node->left != NULL) {
            stack[stack_ptr].node = current_node->left;
            stack[stack_ptr].depth = current_depth; // Siblings are at the same depth
            stack_ptr++;
        }
        // Then push first child (right pointer) if it exists, so it's processed next.
        if (current_node->right != NULL) {
            stack[stack_ptr].node = current_node->right;
            stack[stack_ptr].depth = current_depth + 1; // Children are one level deeper
            stack_ptr++;
        }
    }
}

// Function: PrintTree
unsigned int PrintTree(const char* node_name) {
    struct TreeNode* target_node;

    if (node_name == NULL || *node_name == '\0') {
        target_node = root;
    } else {
        target_node = LookupNode(node_name);
    }

    if (target_node == NULL) {
        printf("ERROR: Tree not found: %s\n", node_name ? node_name : "(root)");
        return 0xFFFFFFFF;
    } else {
        WalkTree(target_node);
        return 0;
    }
}

// Function: FreeTree (recursive post-order traversal)
void FreeTree(struct TreeNode* node) {
    if (node == NULL) {
        return;
    }
    FreeTree(node->right); // Free children first (first child)
    FreeTree(node->left);  // Then free siblings (next sibling)
    if (node->page_data != NULL) {
        free(node->page_data);
    }
    free(node);
}

// Function: DeleteNode
unsigned int DeleteNode(const char* node_name) {
    struct TreeNode* target_node = LookupNode(node_name);
    if (target_node == NULL) {
        printf("ERROR: Could not locate node for deletion\n");
        return 0xFFFFFFFF;
    }

    char parent_path_buffer[sizeof(target_node->name)];
    strncpy(parent_path_buffer, node_name, sizeof(parent_path_buffer) - 1);
    parent_path_buffer[sizeof(parent_path_buffer) - 1] = '\0';

    char* last_dot = strrchr(parent_path_buffer, '.');
    struct TreeNode* parent_node;

    if (last_dot == NULL) {
        parent_node = root; // If no dot, parent is the dummy root node
    } else {
        *last_dot = '\0'; // Null-terminate to get parent's name
        parent_node = LookupNode(parent_path_buffer);
    }

    if (parent_node == NULL) {
        printf("ERROR: Could not locate parent node for deletion\n");
        return 0xFFFFFFFF;
    }

    // Unlink `target_node` from its `parent_node`
    if (target_node == parent_node->right) {
        // `target_node` is the first child of `parent_node`
        parent_node->right = target_node->left; // Make its sibling the new first child
        target_node->left = NULL; // Detach target_node's sibling chain
    } else {
        // `target_node` is a sibling, not the first child.
        // Traverse siblings (using `left` pointers) to find the node whose `left` pointer points to `target_node`.
        struct TreeNode* current_sibling = parent_node->right;
        while (current_sibling != NULL && current_sibling->left != target_node) {
            current_sibling = current_sibling->left;
        }

        if (current_sibling == NULL) {
            printf("ERROR: Could not locate node for deletion (sibling not found)\n");
            return 0xFFFFFFFF;
        }

        // `current_sibling` is the node *before* `target_node` in the sibling list.
        current_sibling->left = target_node->left; // Unlink target_node
        target_node->left = NULL; // Detach target_node's sibling chain
    }

    FreeTree(target_node); // Free the unlinked node and its entire subtree
    return 0;
}

// Function: LookupNode
// Searches for a node by its full path (e.g., "root.child1.sub1")
// Assumes 'right' is the first child and 'left' is the next sibling.
struct TreeNode* LookupNode(const char* search_name) {
    if (root == NULL || search_name == NULL || *search_name == '\0') {
        return NULL;
    }

    char name_buffer[sizeof(root->name)]; // Use buffer for tokenizing search_name
    strncpy(name_buffer, search_name, sizeof(name_buffer) - 1);
    name_buffer[sizeof(name_buffer) - 1] = '\0';

    char* current_segment = name_buffer;
    char* next_dot = strchr(name_buffer, '.');
    if (next_dot != NULL) {
        *next_dot = '\0'; // Null-terminate the current segment
    }

    // Start search from the first child of the dummy root node
    struct TreeNode* current_node = root->right;

    while (1) {
        // Traverse siblings (using left pointers) to find the current segment
        while (current_node != NULL) {
            if (strcmp(current_node->name, current_segment) == 0) {
                break; // Found the current segment
            }
            current_node = current_node->left; // Move to next sibling
        }

        if (current_node == NULL) {
            return NULL; // Current segment not found in this level
        }

        if (next_dot == NULL) {
            return current_node; // This is the last segment, node found
        }

        // Move to the next segment and descend to children
        current_segment = next_dot + 1;
        next_dot = strchr(current_segment, '.');
        if (next_dot != NULL) {
            *next_dot = '\0'; // Null-terminate the next segment
        }
        current_node = current_node->right; // Descend to the first child
    }
}

// Function: InsertNodeInTree
// Inserts an already allocated and initialized TreeNode into the tree.
// Assumes 'right' is the first child and 'left' is the next sibling.
unsigned int InsertNodeInTree(struct TreeNode* new_node) {
    if (new_node == NULL || new_node->name[0] == '\0') {
        printf("ERROR: Name cannot be blank or node is NULL\n");
        return 0xFFFFFFFF;
    }

    // Ensure the dummy root node exists
    if (root == NULL) {
        root = (struct TreeNode*)calloc(1, sizeof(struct TreeNode));
        VerifyPointerOrTerminate(root, "root TreeNode during insert");
        root->name[0] = '.'; // Dummy root node name
        root->name[1] = '\0';
    }

    // Check if a node with the same full path already exists
    if (LookupNode(new_node->name) != NULL) {
        printf("ERROR: Node already exists: %s\n", new_node->name);
        return 0xFFFFFFFF;
    }

    char parent_path_buffer[sizeof(new_node->name)];
    strncpy(parent_path_buffer, new_node->name, sizeof(parent_path_buffer) - 1);
    parent_path_buffer[sizeof(parent_path_buffer) - 1] = '\0';

    char* last_dot = strrchr(parent_path_buffer, '.');
    struct TreeNode* parent_node;
    char* node_segment_name; // The name segment for the new node itself

    if (last_dot == NULL) {
        // Node is a top-level child of the dummy root
        parent_node = root;
        node_segment_name = new_node->name; // Full name is the segment name
    } else {
        // Node has a parent, extract parent's full path
        *last_dot = '\0'; // Null-terminate to get parent's path
        node_segment_name = last_dot + 1; // This is the segment for the new node
        parent_node = LookupNode(parent_path_buffer);
    }

    if (parent_node == NULL) {
        printf("ERROR: Parent node doesn't exist: %s\n", parent_path_buffer);
        return 0xFFFFFFFF;
    }

    // Update the `name` field of `new_node` to store only its segment name, not the full path.
    char temp_segment_name_buffer[sizeof(new_node->name)];
    strncpy(temp_segment_name_buffer, node_segment_name, sizeof(temp_segment_name_buffer) - 1);
    temp_segment_name_buffer[sizeof(temp_segment_name_buffer) - 1] = '\0';
    strncpy(new_node->name, temp_segment_name_buffer, sizeof(new_node->name) - 1);
    new_node->name[sizeof(new_node->name) - 1] = '\0';

    // Insert `new_node` as a child of `parent_node`
    if (parent_node->right == NULL) {
        // Parent has no children, `new_node` becomes the first child (parent_node->right)
        parent_node->right = new_node;
    } else {
        // Parent already has children. Traverse siblings (using `left` pointers)
        // to find the last sibling, and append `new_node` there.
        struct TreeNode* current_sibling = parent_node->right;
        while (current_sibling->left != NULL) {
            current_sibling = current_sibling->left;
        }
        current_sibling->left = new_node;
    }

    return 0; // Success
}

/*
// Example main function for testing (not part of the requested output)
int main() {
    printf("Initializing tree...\n");
    if (InitializeTree() != 0) {
        fprintf(stderr, "Tree initialization failed.\n");
        return EXIT_FAILURE;
    }
    printf("Tree initialized successfully.\n");

    printf("\nPrinting entire tree (from root):\n");
    PrintTree(NULL);

    printf("\nPrinting subtree for 'child2':\n");
    PrintTree("child2");

    printf("\nPrinting subtree for 'child1.sub2':\n");
    PrintTree("child1.sub2");

    printf("\nAttempting to insert an existing node 'child1.sub1':\n");
    struct TreeNode* existing_node_mock = (struct TreeNode*)calloc(1, sizeof(struct TreeNode));
    strncpy(existing_node_mock->name, "child1.sub1", sizeof(existing_node_mock->name) - 1);
    InsertNodeInTree(existing_node_mock);
    free(existing_node_mock); // Free mock node as it wasn't inserted

    printf("\nAttempting to insert a node with non-existent parent 'nonexistent.newchild':\n");
    struct TreeNode* bad_node_mock = (struct TreeNode*)calloc(1, sizeof(struct TreeNode));
    strncpy(bad_node_mock->name, "nonexistent.newchild", sizeof(bad_node_mock->name) - 1);
    InsertNodeInTree(bad_node_mock);
    free(bad_node_mock);

    printf("\nDeleting node 'child2.sub2.nested':\n");
    if (DeleteNode("child2.sub2.nested") == 0) {
        printf("Deletion successful. Tree after deletion:\n");
        PrintTree(NULL);
    } else {
        printf("Deletion failed.\n");
    }

    printf("\nDeleting node 'child1.sub2':\n");
    if (DeleteNode("child1.sub2") == 0) {
        printf("Deletion successful. Tree after deletion:\n");
        PrintTree(NULL);
    } else {
        printf("Deletion failed.\n");
    }

    printf("\nDeleting node 'child1':\n");
    if (DeleteNode("child1") == 0) {
        printf("Deletion successful. Tree after deletion:\n");
        PrintTree(NULL);
    } else {
        printf("Deletion failed.\n");
    }

    printf("\nFreeing entire tree...\n");
    FreeTree(root);
    root = NULL;
    printf("Tree freed.\n");

    return 0;
}
*/