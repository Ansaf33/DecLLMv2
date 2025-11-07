#include <stdlib.h> // For free
#include <stdint.h> // For uint32_t, intptr_t
#include <stddef.h> // For NULL

// Define undefined4 as uint32_t based on common usage in disassemblies
typedef uint32_t undefined4;

// Global variables (assuming these are declared externally or in the same compilation unit)
// Nodes is an int, treated as a raw address.
int Nodes = 0;
// Edges is an undefined4*, treated as a raw address.
undefined4 *Edges = NULL;
int NumNodes = 0;
int NumEdges = 0;

// Function: AddNode
int AddNode(int param_1) {
    if (param_1 == 0) {
        return 0;
    }

    if (Nodes == 0) {
        Nodes = param_1;
        *(undefined4 *)(intptr_t)(param_1 + 8) = 0; // Set next node pointer to NULL
    } else {
        *(int *)(intptr_t)(param_1 + 8) = Nodes; // New node's next points to current head
        Nodes = param_1; // New node becomes the head
    }
    NumNodes++;
    return param_1;
}

// Function: AddEdge
int * AddEdge(int *param_1) {
    if (param_1 == NULL) {
        return NULL;
    }

    if (Edges == NULL) {
        Edges = (undefined4 *)param_1;
    } else {
        *param_1 = (int)(intptr_t)Edges; // New edge's next points to current head
        Edges = (undefined4 *)param_1; // New edge becomes the head
    }
    NumEdges++;
    return param_1;
}

// Function: RemoveNode
int RemoveNode(int param_1) {
    if (Nodes == 0 || param_1 == 0) {
        return 0;
    }

    if (param_1 == Nodes) {
        Nodes = *(int *)(intptr_t)(param_1 + 8);
        *(undefined4 *)(intptr_t)(param_1 + 8) = 0; // Clear next pointer of the removed node
        return Nodes;
    } else {
        // The original loop iterates through nodes starting from the second node (Nodes->next).
        // It updates the 'next' pointer of the *head* node, not the 'previous' node to param_1.
        // This means it only correctly removes param_1 if param_1 is the second node in the list.
        // If param_1 is further down, this logic is flawed, but is replicated as per instructions.
        for (int currentNodeAddr = *(int *)(intptr_t)(Nodes + 8); currentNodeAddr != 0; currentNodeAddr = *(int *)(intptr_t)(currentNodeAddr + 8)) {
            if (currentNodeAddr == param_1) {
                *(undefined4 *)(intptr_t)(Nodes + 8) = *(undefined4 *)(intptr_t)(currentNodeAddr + 8);
                *(undefined4 *)(intptr_t)(param_1 + 8) = 0; // Clear next pointer of the removed node
                return Nodes;
            }
        }
        return 0; // param_1 not found (or not the head/second node if it's a deeper node)
    }
}

// Function: RemoveEdge
undefined4 * RemoveEdge(undefined4 *param_1) {
    if (Edges == NULL || param_1 == NULL) {
        return NULL;
    }

    if (param_1 == Edges) {
        Edges = (undefined4 *)*param_1;
        *param_1 = 0; // Clear next pointer of the removed edge
        return Edges;
    } else {
        // The original loop iterates through edges starting from the second edge (*Edges).
        // It updates the 'next' pointer of the *head* edge, not the 'previous' edge to param_1.
        // This means it only correctly removes param_1 if param_1 is the second edge in the list.
        // If param_1 is further down, this logic is flawed, but is replicated as per instructions.
        for (undefined4 *currentEdgeAddr = (undefined4 *)*Edges; currentEdgeAddr != NULL; currentEdgeAddr = (undefined4 *)*currentEdgeAddr) {
            if (currentEdgeAddr == param_1) {
                *Edges = *currentEdgeAddr;
                *param_1 = 0; // Clear next pointer of the removed edge
                return Edges;
            }
        }
        return NULL;
    }
}

// Function: FindNode
int FindNode(int param_1) {
    if (Nodes != 0) {
        for (int currentNodeAddr = Nodes; currentNodeAddr != 0; currentNodeAddr = *(int *)(intptr_t)(currentNodeAddr + 8)) {
            if (param_1 == *(int *)(intptr_t)(currentNodeAddr + 0xc)) { // Compare param_1 with ID at offset 0xc
                return currentNodeAddr;
            }
        }
    }
    return 0;
}

// Function: FindEdge
undefined4 * FindEdge(int param_1, int param_2) {
    if (Edges != NULL) {
        for (undefined4 *currentEdgeAddr = Edges; currentEdgeAddr != NULL; currentEdgeAddr = (undefined4 *)*currentEdgeAddr) {
            if ((param_1 == currentEdgeAddr[2]) && (param_2 == currentEdgeAddr[3])) { // IDs at offset 8 and 12
                return currentEdgeAddr;
            }
            if ((param_2 == currentEdgeAddr[2]) && (param_1 == currentEdgeAddr[3])) {
                return currentEdgeAddr;
            }
        }
    }
    return NULL;
}

// Function: DestroyNodes
undefined4 DestroyNodes(void) {
    if (Nodes == 0) {
        return 0;
    }

    int currentNodeAddr = Nodes;
    while (currentNodeAddr != 0) {
        int nextNodeAddr = *(int *)(intptr_t)(currentNodeAddr + 8);
        free((void*)(intptr_t)currentNodeAddr); // Cast to void* for free
        currentNodeAddr = nextNodeAddr;
    }
    Nodes = 0; // Reset head
    NumNodes = 0; // Reset count
    return 1;
}

// Function: DestroyEdges
undefined4 DestroyEdges(void) {
    if (Edges == NULL) {
        return 0;
    }

    undefined4 *currentEdge = Edges;
    while (currentEdge != NULL) {
        undefined4 *nextEdge = (undefined4 *)*currentEdge;
        free(currentEdge);
        currentEdge = nextEdge;
    }
    Edges = NULL;
    NumEdges = 0;
    return 1;
}