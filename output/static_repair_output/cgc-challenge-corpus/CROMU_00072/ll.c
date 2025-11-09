#include <stdlib.h> // For free
#include <stdint.h> // For uintptr_t, uint32_t

// Define undefined4 as uint32_t for clarity and portability,
// as it's a common type in decompiled 32-bit code.
typedef uint32_t undefined4;

// Global variables, initialized to NULL and 0.
// Using void* for generic pointers to handle raw address manipulation
// as seen in the original snippet.
void *Nodes_head = NULL;
void *Edges_head = NULL;
int NumNodes = 0;
int NumEdges = 0;

// Function: AddNode
// param_1 is an integer representing a memory address of a new node.
int AddNode(int param_1) {
    if (param_1 == 0) {
        return 0;
    }

    // Cast the integer address to a char pointer for byte-level arithmetic.
    char *node_ptr = (char *)(uintptr_t)param_1;

    if (Nodes_head == NULL) {
        Nodes_head = node_ptr;
        // The 'next' pointer of a node is assumed to be at offset 8.
        // Set it to NULL as it's the first node.
        *(void **)(node_ptr + 8) = NULL;
    } else {
        // Set the 'next' pointer of the new node to the current head.
        *(void **)(node_ptr + 8) = Nodes_head;
        Nodes_head = node_ptr; // The new node becomes the head of the list.
    }
    NumNodes++;
    return param_1;
}

// Function: AddEdge
// param_1 is a void pointer representing a new edge.
void *AddEdge(void *param_1) {
    if (param_1 == NULL) {
        return NULL;
    }

    if (Edges_head == NULL) {
        Edges_head = param_1;
        // The original snippet doesn't explicitly set the next pointer of the first edge to NULL.
        // It relies on the caller to initialize the memory.
    } else {
        // The 'next' pointer of an edge is assumed to be at offset 0 (the first member).
        // Set the 'next' pointer of the new edge to the current head.
        *(void **)param_1 = Edges_head;
        Edges_head = param_1; // The new edge becomes the head of the list.
    }
    NumEdges++;
    return param_1;
}

// Function: RemoveNode
// param_1 is an integer representing the memory address of the node to remove.
int RemoveNode(int param_1_addr) {
    if (Nodes_head == NULL || param_1_addr == 0) {
        return 0;
    }

    char *param_1_ptr = (char *)(uintptr_t)param_1_addr;

    if (param_1_ptr == Nodes_head) {
        // If the node to remove is the head, update the head.
        Nodes_head = *(void **)(param_1_ptr + 8); // New head is the next node.
        *(void **)(param_1_ptr + 8) = NULL;       // Clear the removed node's next pointer.
    } else {
        // Iterate through the list to find the predecessor of the node to remove.
        char *current = (char *)Nodes_head;
        while (current != NULL) {
            char *next_ptr_of_current = *(char **)(current + 8); // Get current node's next pointer.
            if (next_ptr_of_current == param_1_ptr) {
                // Found the predecessor: 'current' points to 'param_1_ptr'.
                // Bypass 'param_1_ptr' by linking 'current->next' to 'param_1_ptr->next'.
                *(void **)(current + 8) = *(void **)(param_1_ptr + 8);
                *(void **)(param_1_ptr + 8) = NULL; // Clear the removed node's next pointer.
                return (int)(uintptr_t)Nodes_head;  // Return the head (node successfully removed).
            }
            current = next_ptr_of_current; // Move to the next node.
        }
        return 0; // Node not found in the list.
    }
    return (int)(uintptr_t)Nodes_head; // Return the new head (if head was removed) or original head.
}

// Function: RemoveEdge
// param_1 is a void pointer representing the edge to remove.
void *RemoveEdge(void *param_1) {
    if (Edges_head == NULL || param_1 == NULL) {
        return NULL;
    }

    if (param_1 == Edges_head) {
        // If the edge to remove is the head, update the head.
        Edges_head = *(void **)Edges_head; // New head is the next edge.
        *(void **)param_1 = NULL;          // Clear the removed edge's next pointer.
    } else {
        // Iterate through the list to find the predecessor of the edge to remove.
        void *current = Edges_head;
        while (current != NULL) {
            void *next_ptr_of_current = *(void **)current; // Get current edge's next pointer.
            if (next_ptr_of_current == param_1) {
                // Found the predecessor: 'current' points to 'param_1'.
                // Bypass 'param_1' by linking 'current->next' to 'param_1->next'.
                *(void **)current = *(void **)param_1;
                *(void **)param_1 = NULL; // Clear the removed edge's next pointer.
                return Edges_head;         // Return the head (edge successfully removed).
            }
            current = next_ptr_of_current; // Move to the next edge.
        }
        return NULL; // Edge not found in the list.
    }
    return Edges_head; // Return the new head (if head was removed) or original head.
}

// Function: FindNode
// param_1 is an integer representing the ID value to find within a node.
int FindNode(int param_1_value) {
    char *current = (char *)Nodes_head;
    if (current == NULL) {
        return 0;
    }

    // The node's ID is assumed to be at offset 0xc (12 bytes).
    while (current != NULL) {
        if (param_1_value == *(int *)(current + 0xc)) {
            return (int)(uintptr_t)current; // Return the address of the found node.
        }
        // Move to the next node; its pointer is at offset 8.
        current = *(char **)(current + 8);
    }
    return 0; // Node not found.
}

// Function: FindEdge
// param_1 and param_2 are integers representing node IDs connected by an edge.
void *FindEdge(int param_1, int param_2) {
    char *current = (char *)Edges_head;
    if (current == NULL) {
        return NULL;
    }

    // Edge node IDs are assumed to be at offset 8 (first ID) and 12 (second ID).
    while (current != NULL) {
        int node1_id = *(int *)(current + 8);
        int node2_id = *(int *)(current + 12);

        // Check for a match in either direction (param_1-param_2 or param_2-param_1).
        if ((param_1 == node1_id && param_2 == node2_id) ||
            (param_2 == node1_id && param_1 == node2_id)) {
            return current; // Return the address of the found edge.
        }
        // Move to the next edge; its pointer is at offset 0.
        current = *(char **)current;
    }
    return NULL; // Edge not found.
}

// Function: DestroyNodes
// Frees all memory allocated for nodes in the list.
undefined4 DestroyNodes(void) {
    if (Nodes_head == NULL) {
        return 0; // No nodes to destroy.
    }

    char *current = (char *)Nodes_head;
    while (current != NULL) {
        char *next_node = *(char **)(current + 8); // Get the next node's address.
        free(current);                              // Free the current node's memory.
        current = next_node;                        // Move to the next node.
    }
    Nodes_head = NULL; // Reset head pointer.
    NumNodes = 0;      // Reset node count.
    return 1;          // Successfully destroyed nodes.
}

// Function: DestroyEdges
// Frees all memory allocated for edges in the list.
undefined4 DestroyEdges(void) {
    if (Edges_head == NULL) {
        return 0; // No edges to destroy.
    }

    char *current = (char *)Edges_head;
    while (current != NULL) {
        char *next_edge = *(char **)current; // Get the next edge's address (at offset 0).
        free(current);                       // Free the current edge's memory.
        current = next_edge;                 // Move to the next edge.
    }
    Edges_head = NULL; // Reset head pointer.
    NumEdges = 0;      // Reset edge count.
    return 1;          // Successfully destroyed edges.
}