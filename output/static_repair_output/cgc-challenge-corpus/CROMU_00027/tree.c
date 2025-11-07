#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For true/false, though 'while(true)' implies 1

// Assuming 32-bit pointers for 0x50 total size, which is consistent with the snippet's memory accesses.
// If compiled on a 64-bit system, padding might occur or pointer sizes might differ,
// requiring adjustments or explicit packing attributes for the struct.
// For Linux compilable C code, assuming default packing and 32-bit compilation target
// (e.g., with -m32 flag) for exact size match. Otherwise, for 64-bit native, sizes would be:
// name[0x40] (64 bytes)
// left (8 bytes)
// right (8 bytes)
// page_size (8 bytes)
// page (8 bytes)
// Total: 64 + 8 + 8 + 8 + 8 = 96 bytes (0x60).
// Sticking to the 0x50 size implies 32-bit pointers and size_t.
typedef struct TreeNode {
    char name[0x40];           // 0x00 - 0x3F (64 bytes)
    struct TreeNode *left;     // 0x40 - 0x43 (4 bytes) - Used as "next sibling"
    struct TreeNode *right;    // 0x44 - 0x47 (4 bytes) - Used as "first child"
    size_t page_size;          // 0x48 - 0x4B (4 bytes)
    void *page;                // 0x4C - 0x4F (4 bytes)
} TreeNode; // Total Size: 0x50 (80) bytes on a 32-bit system

// Structure for initial data, assuming each entry is 0x44 bytes, consistent with snippet's access pattern.
typedef struct InitialDataBlock {
    char name_prefix[0x40];    // 0x00 - 0x3F (64 bytes)
    char *page_content_ptr;    // 0x40 - 0x43 (4 bytes)
} InitialDataBlock; // Total Size: 0x44 (68) bytes on a 32-bit system

// Global variables
TreeNode *root = NULL;

// Initial data for tree population
// The original code suggests 0x14 (20) entries are processed (0 to 0x13 inclusive).
// Providing example data here.
InitialDataBlock InitialInfo[] = {
    {"nodeA", "Page content for nodeA"},
    {"nodeA.sub1", "Page content for nodeA.sub1"},
    {"nodeB", "Page content for nodeB"},
    {"nodeA.sub2", "Page content for nodeA.sub2"},
    {"nodeB.sub1", "Page content for nodeB.sub1"},
    {"nodeC", "Page content for nodeC"},
    {"nodeA.sub1.leaf", "Page content for nodeA.sub1.leaf"},
    {"nodeD", "Page content for nodeD"},
    {"nodeE", "Page content for nodeE"},
    {"nodeF", "Page content for nodeF"},
    {"nodeG", "Page content for nodeG"},
    {"nodeH", "Page content for nodeH"},
    {"nodeI", "Page content for nodeI"},
    {"nodeJ", "Page content for nodeJ"},
    {"nodeK", "Page content for nodeK"},
    {"nodeL", "Page content for nodeL"},
    {"nodeM", "Page content for nodeM"},
    {"nodeN", "Page content for nodeN"},
    {"nodeO", "Page content for nodeO"},
    {"nodeP", "Page content for nodeP"},
};

// Function prototypes
void VerifyPointerOrTerminate(void *ptr, const char *msg);
TreeNode *LookupNode(const char *nodePath);
int InsertNodeInTree(TreeNode *newNode);
void WalkTree(TreeNode *startNode);
void FreeTree(TreeNode *node);
int PrintTree(const char *nodeName);
int DeleteNode(const char *nodeName);
int InitializeTree(void);

// Function: VerifyPointerOrTerminate
// Checks if a pointer is NULL and terminates the program if it is.
void VerifyPointerOrTerminate(void *ptr, const char *msg) {
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed for %s\n", msg);
        exit(EXIT_FAILURE);
    }
}

// Function: InitializeTree
// Initializes the tree with predefined data.
int InitializeTree(void) {
    for (int i = 0; i <= 0x13; ++i) { // Loop from 0 to 19 (inclusive)
        TreeNode *newNode = (TreeNode *)calloc(1, sizeof(TreeNode));
        VerifyPointerOrTerminate(newNode, "TreeNode during initialization");

        // Copy name, ensuring null termination
        strncpy(newNode->name, InitialInfo[i].name_prefix, sizeof(newNode->name) - 1);
        newNode->name[sizeof(newNode->name) - 1] = '\0';

        // Allocate and copy page content
        size_t content_len = strlen(InitialInfo[i].page_content_ptr);
        newNode->page_size = content_len + 1; // +1 for null terminator
        newNode->page = calloc(1, newNode->page_size);
        VerifyPointerOrTerminate(newNode->page, "node->page during initialization");
        memcpy(newNode->page, InitialInfo[i].page_content_ptr, newNode->page_size);

        if (InsertNodeInTree(newNode) != 0) {
            // Error occurred during insertion, clean up allocated node
            free(newNode->page);
            free(newNode);
            return -1; // Return -1 for error (0xffffffff)
        }
    }
    return 0; // Return 0 for success
}

// Function: WalkTree
// Performs a custom iterative tree traversal, printing nodes with indentation.
void WalkTree(TreeNode *startNode) {
    if (!startNode) return;

    // Stack to store (node pointer, level) pairs for iterative traversal
    struct StackItem {
        TreeNode *node;
        int level;
    };
    // Max stack depth of 32 pairs (64 ints in original). Using 64 items for (node, level) pairs.
    struct StackItem stack[64];
    int stack_top = 0; // Index of the next available slot in the stack

    TreeNode *currentNode = startNode;
    int currentLevel = 0;

    // Initial processing of the starting node (root of the current walk)
    printf("%s\n", currentNode->name);

    // If the starting node has a right child, push it onto the stack for later processing.
    // This is part of the custom traversal order.
    if (currentNode->right) {
        stack[stack_top].node = currentNode->right;
        stack[stack_top].level = currentLevel + 1;
        stack_top++;
    }

    // Main loop for iterative traversal
    while (stack_top > 0) {
        // Pop node and its corresponding level from the stack
        stack_top--;
        currentNode = stack[stack_top].node;
        currentLevel = stack[stack_top].level;

        // Print current node with appropriate indentation
        for (int i = 0; i < currentLevel; ++i) {
            printf("    ");
        }
        printf("%s\n", currentNode->name);

        // Push children onto the stack for subsequent processing.
        // Order of pushing: right child then left child.
        // Due to LIFO nature of stack, the left child (pushed last) will be processed first.
        // This replicates the original traversal logic.
        if (currentNode->right) {
            stack[stack_top].node = currentNode->right;
            stack[stack_top].level = currentLevel + 1; // Right child is one level deeper
            stack_top++;
        }
        if (currentNode->left) {
            stack[stack_top].node = currentNode->left;
            stack[stack_top].level = currentLevel; // Left child is at the same level (sibling)
            stack_top++;
        }
    }
}

// Function: PrintTree
// Locates a node by name and then calls WalkTree to print its subtree.
int PrintTree(const char *nodeName) {
    TreeNode *targetNode;

    // If nodeName is NULL or empty, start walk from the global root.
    if (nodeName == NULL || *nodeName == '\0') {
        targetNode = root;
    } else {
        targetNode = LookupNode(nodeName);
    }

    if (targetNode == NULL) {
        printf("ERROR: Tree not found: %s\n", nodeName ? nodeName : "(root)");
        return -1; // Return -1 for error
    } else {
        WalkTree(targetNode);
        return 0; // Return 0 for success
    }
}

// Function: FreeTree
// Recursively frees all memory associated with a node and its children.
void FreeTree(TreeNode *node) {
    if (node == NULL) return; // Base case for recursion

    // Recursively free the right child (first child)
    if (node->right != NULL) {
        FreeTree(node->right);
    }
    // Recursively free the left child (next sibling)
    if (node->left != NULL) {
        FreeTree(node->left);
    }
    // Free page content if it was allocated
    if (node->page != NULL) {
        free(node->page);
    }
    // Finally, free the node itself
    free(node);
}

// Function: DeleteNode
// Deletes a node from the tree and frees its associated memory.
int DeleteNode(const char *nodeName) {
    char parentPathBuffer[sizeof(((TreeNode*)0)->name)]; // Buffer for parent's full path
    TreeNode *nodeToDelete = LookupNode(nodeName);

    if (nodeToDelete == NULL) {
        printf("ERROR: Could not locate node '%s' for deletion\n", nodeName);
        return -1;
    }

    TreeNode *parentNode;
    char *dot_pos = strrchr(nodeName, '.');

    if (dot_pos == NULL) { // Node is a direct child of the global root
        parentNode = root;
    } else { // Node has a named parent
        // Copy parent path to buffer, null-terminate it
        strncpy(parentPathBuffer, nodeName, dot_pos - nodeName);
        parentPathBuffer[dot_pos - nodeName] = '\0';

        parentNode = LookupNode(parentPathBuffer);
    }

    if (parentNode == NULL) {
        printf("ERROR: Parent node not found for '%s' during deletion\n", nodeName);
        return -1;
    }

    // This section implements the "first child, next sibling" deletion logic.
    // `parentNode->right` points to the head of the children's sibling list.
    // Each child's `left` pointer points to its next sibling.
    if (nodeToDelete == parentNode->right) {
        // nodeToDelete is the first child of parentNode
        parentNode->right = nodeToDelete->left; // Parent's first child becomes nodeToDelete's sibling
        nodeToDelete->left = NULL; // Disconnect nodeToDelete from its siblings
    } else {
        // nodeToDelete is a sibling, not the first child.
        // Traverse the sibling list starting from parentNode->right to find the preceding sibling.
        TreeNode *currentSibling = parentNode->right;
        while (currentSibling != NULL && currentSibling->left != nodeToDelete) {
            currentSibling = currentSibling->left;
        }

        if (currentSibling == NULL) {
            // This case indicates an inconsistency (nodeToDelete not found in sibling list),
            // which should ideally not happen if LookupNode successfully found nodeToDelete.
            printf("ERROR: Could not locate node '%s' in parent's children list for deletion\n", nodeName);
            return -1;
        }

        // currentSibling->left points to nodeToDelete.
        // Link currentSibling to nodeToDelete's next sibling, effectively skipping nodeToDelete.
        currentSibling->left = nodeToDelete->left;
        nodeToDelete->left = NULL; // Disconnect nodeToDelete from its siblings
    }

    FreeTree(nodeToDelete); // Free the node and its subtree
    return 0;
}

// Function: LookupNode
// Locates a node in the tree given its full path (e.g., "nodeA.sub1").
TreeNode *LookupNode(const char *nodePath) {
    char pathBuffer[sizeof(((TreeNode*)0)->name)]; // Buffer for mutable path segments
    strncpy(pathBuffer, nodePath, sizeof(pathBuffer) - 1);
    pathBuffer[sizeof(pathBuffer) - 1] = '\0'; // Ensure null termination

    char *currentSegment = pathBuffer;
    char *dot_pos = strchr(pathBuffer, '.');

    // Handle the special case for the root node itself (named ".")
    if (strcmp(nodePath, ".") == 0 || strcmp(nodePath, "") == 0) {
        return root;
    }

    // The lookup starts from the global root's first child (`root->right`).
    // This implies the global `root` node itself is a special placeholder for its direct children.
    TreeNode *currentNode = root->right; // Start search from the first child of the global root.

    while (true) {
        if (dot_pos != NULL) {
            *dot_pos = '\0'; // Temporarily null-terminate the current segment for strcmp
        }

        TreeNode *segmentNode = NULL;
        TreeNode *tempNode = currentNode; // Iterate through siblings at the current level

        // Search for the current segment in the sibling list
        while (tempNode != NULL) {
            if (strcmp(tempNode->name, currentSegment) == 0) {
                segmentNode = tempNode; // Found the segment node
                break;
            }
            tempNode = tempNode->left; // Move to the next sibling
        }

        if (segmentNode == NULL) {
            return NULL; // Current segment not found at this level
        }

        if (dot_pos == NULL) {
            return segmentNode; // No more segments, this is the target node
        }

        // Prepare for the next segment: move to the segment after the dot,
        // find the next dot, and set the current node to search in its children.
        currentSegment = dot_pos + 1;
        dot_pos = strchr(currentSegment, '.');
        currentNode = segmentNode->right; // Search in the children of the found segment node
    }
}

// Function: InsertNodeInTree
// Inserts a new node into the tree.
int InsertNodeInTree(TreeNode *newNode) {
    // Ensure the global root node exists. If not, create it.
    if (root == NULL) {
        root = (TreeNode *)calloc(1, sizeof(TreeNode));
        VerifyPointerOrTerminate(root, "root TreeNode during insert");
        strncpy(root->name, ".", sizeof(root->name) - 1); // Root node is typically named "."
        root->name[sizeof(root->name) - 1] = '\0';
    }

    // Check if a node with the same full path already exists.
    if (LookupNode(newNode->name) != NULL) {
        printf("ERROR: node '%s' already exists\n", newNode->name);
        return -1;
    }

    // Check if the node name itself is blank.
    if (newNode->name[0] == '\0') {
        printf("ERROR: Name cannot be blank\n");
        return -1;
    }

    char parentPathBuffer[sizeof(newNode->name)]; // Buffer to hold the parent's full path
    char *dot_pos = strrchr(newNode->name, '.'); // Find the last '.' to separate parent from segment name
    TreeNode *parentNode;

    if (dot_pos == NULL) { // No '.' found, so newNode is a direct child of the global root.
        parentNode = root;
    } else { // '.' found, so newNode has a named parent.
        // Copy the parent's full path into parentPathBuffer and null-terminate it.
        strncpy(parentPathBuffer, newNode->name, dot_pos - newNode->name);
        parentPathBuffer[dot_pos - newNode->name] = '\0';

        // Crucial step: Overwrite newNode's name with just its segment name (after the last dot).
        // This is how names are stored internally in the nodes.
        strncpy(newNode->name, dot_pos + 1, sizeof(newNode->name) - 1);
        newNode->name[sizeof(newNode->name) - 1] = '\0';

        parentNode = LookupNode(parentPathBuffer);
        if (parentNode == NULL) {
            printf("ERROR: Parent node doesn't exist: %s\n", parentPathBuffer);
            return -1;
        }
    }

    // Insert newNode into the parent's children list.
    // `parentNode->right` points to the first child. `node->left` points to the next sibling.
    if (parentNode->right == NULL) {
        // Parent has no children, newNode becomes the first child.
        parentNode->right = newNode;
    } else {
        // Parent already has children, find the last sibling in the list.
        TreeNode *currentSibling = parentNode->right;
        while (currentSibling->left != NULL) {
            currentSibling = currentSibling->left;
        }
        currentSibling->left = newNode; // New node becomes the last sibling.
    }

    return 0; // Return 0 for success
}

// Function: main (example usage)
// A simple main function to demonstrate the tree operations.
int main() {
    printf("Initializing tree...\n");
    if (InitializeTree() != 0) {
        fprintf(stderr, "Tree initialization failed!\n");
        return EXIT_FAILURE;
    }
    printf("Tree initialized successfully.\n\n");

    printf("Printing entire tree:\n");
    PrintTree(""); // Print from the root (named ".")

    printf("\nPrinting subtree for 'nodeA':\n");
    PrintTree("nodeA");

    printf("\nPrinting subtree for 'nodeB.sub1':\n");
    PrintTree("nodeB.sub1");

    // Example of inserting a new node
    printf("\nInserting 'nodeB.sub2'...\n");
    TreeNode *newNode = (TreeNode *)calloc(1, sizeof(TreeNode));
    VerifyPointerOrTerminate(newNode, "new TreeNode for insertion");
    strncpy(newNode->name, "nodeB.sub2", sizeof(newNode->name) - 1);
    newNode->name[sizeof(newNode->name) - 1] = '\0';
    newNode->page_size = strlen("Page content for nodeB.sub2") + 1;
    newNode->page = calloc(1, newNode->page_size);
    VerifyPointerOrTerminate(newNode->page, "new node page for insertion");
    memcpy(newNode->page, "Page content for nodeB.sub2", newNode->page_size);

    if (InsertNodeInTree(newNode) == 0) {
        printf("Node 'nodeB.sub2' inserted.\n");
        printf("\nPrinting subtree for 'nodeB':\n");
        PrintTree("nodeB");
    } else {
        fprintf(stderr, "Failed to insert 'nodeB.sub2'.\n");
        free(newNode->page);
        free(newNode);
    }

    // Example of deleting a node
    printf("\nDeleting 'nodeA.sub2'...\n");
    if (DeleteNode("nodeA.sub2") == 0) {
        printf("Node 'nodeA.sub2' deleted.\n");
        printf("\nPrinting subtree for 'nodeA' after deletion:\n");
        PrintTree("nodeA");
    } else {
        fprintf(stderr, "Failed to delete 'nodeA.sub2'.\n");
    }

    // Attempt to delete a non-existent node
    printf("\nAttempting to delete 'nonExistentNode'...\n");
    if (DeleteNode("nonExistentNode") != 0) {
        printf("Error as expected: 'nonExistentNode' not found.\n");
    }

    printf("\nFreeing entire tree...\n");
    FreeTree(root);
    root = NULL; // Important to nullify the global root after freeing
    printf("Tree freed.\n");

    return EXIT_SUCCESS;
}