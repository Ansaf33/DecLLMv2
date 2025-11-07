#include <stdlib.h>  // For calloc, exit
#include <stdint.h>  // For UINT32_MAX, uint32_t (if used directly)
#include <limits.h>  // For UINT_MAX
#include <stdbool.h> // For bool (optional, but good practice)

// --- Global Data Structures ---

// Assuming 4-byte pointers and 4-byte integers for struct member offsets,
// which is common for `undefined4` in decompiled 32-bit code.
// This struct definition is derived from the decompiled code's access patterns
// (e.g., Node_ptr[0], Node_ptr[1], Node_ptr[2], Node_ptr[3], Node_ptr[4])
typedef struct Node {
    int visited;          // offset 0 (0: not visited, 1: visited)
    struct Node *prev_in_spt; // offset 4 (Pointer to previous node in shortest path tree)
    struct Node *next_global; // offset 8 (Pointer to the next node in the global list of all nodes)
    int value;            // offset 12 (Some integer value associated with the node, e.g., ID)
    unsigned int distance; // offset 16 (Distance from source node, initialized to UINT_MAX)
} Node;

// This struct definition is derived from the decompiled code's access patterns
// (e.g., Edge_ptr[0], Edge_ptr[2], Edge_ptr[3], Edge_ptr[4])
// Assuming 4-byte pointers and 4-byte integers.
typedef struct Edge {
    struct Edge *next_global; // offset 0 (Pointer to the next edge in the global list)
    void *padding_or_other_field; // offset 4 (Likely padding or an unused field, corresponding to Edge_ptr[1])
    Node *node_A;                 // offset 8 (One of the nodes connected by this edge, corresponding to Edge_ptr[2])
    Node *node_B;                 // offset 12 (The other node connected by this edge, corresponding to Edge_ptr[3])
    unsigned int weight;          // offset 16 (Weight of the edge, corresponding to Edge_ptr[4])
} Edge;

// --- External Global Variables and Functions (placeholders for compilation) ---
// These would typically be defined in other source files or a header.
extern Node *Nodes; // Head of the global linked list of all nodes
extern Edge *Edges; // Head of the global linked list of all edges
extern unsigned int NumNodes; // Total number of nodes in the graph
extern unsigned int NumEdges; // Total number of edges in the graph

// Placeholder function for FindNode. In a real scenario, this would likely take
// an ID or criteria and return a specific node pointer. For a compilable snippet,
// a simple sequential return is used.
Node *FindNode(void) {
    static Node *current_find_node = NULL;
    if (Nodes == NULL) return NULL;

    if (current_find_node == NULL) {
        current_find_node = Nodes; // First call: return the first node
        return current_find_node;
    } else {
        // Second call: return the next node, then reset for future calls or specific logic.
        // This is highly speculative and depends on actual use case.
        Node *node_to_return = current_find_node->next_global;
        current_find_node = NULL; // Reset for potential next path finding operation
        return node_to_return;
    }
}

// Placeholder function to destroy (free) all allocated nodes.
void DestroyNodes(void) {
    Node *current = Nodes;
    while (current != NULL) {
        Node *next = current->next_global;
        free(current);
        current = next;
    }
    Nodes = NULL; // Ensure the global pointer is reset after freeing
}

// Placeholder function to destroy (free) all allocated edges.
void DestroyEdges(void) {
    Edge *current = Edges;
    while (current != NULL) {
        Edge *next = current->next_global;
        free(current);
        current = next;
    }
    Edges = NULL; // Ensure the global pointer is reset after freeing
}

// --- Fixed Functions ---

// Function: FindMinDistanceNode
// Finds the unvisited node with the minimum distance from the source.
Node *FindMinDistanceNode(void) {
    Node *min_dist_node = NULL;
    unsigned int min_distance = UINT_MAX; // Initialize with the maximum possible unsigned int value

    // Iterate through all nodes in the global list
    for (Node *current_node = Nodes; current_node != NULL; current_node = current_node->next_global) {
        // Check if the current node is unvisited and has a smaller distance
        if (current_node->visited == 0 && current_node->distance < min_distance) {
            min_distance = current_node->distance;
            min_dist_node = current_node;
        }
    }
    return min_dist_node;
}

// Function: FindSpt (Find Shortest Path Tree / Path)
// Implements Dijkstra's algorithm to find the shortest path between two nodes
// and returns an array of node values representing the path.
// `param_1` and `param_2` are unused in the original snippet; their `undefined4` type
// is replaced with `void*` for Linux compilability.
// `param_3` (original `byte *`) is replaced with `unsigned char *path_length_out`
// to store the length of the found path.
void *FindSpt(void *param_1, void *param_2, unsigned char *path_length_out) {
    void *path_array = NULL; // Pointer to the array that will store the path's node values

    // Handle initial invalid input conditions
    if (Nodes == NULL || path_length_out == NULL) {
        return NULL;
    }

    // --- Initialization Phase ---
    // Reset all nodes for a new Dijkstra run: distance to infinity, visited to false, prev_in_spt to NULL
    for (Node *current_node_loop = Nodes; current_node_loop != NULL; current_node_loop = current_node_loop->next_global) {
        current_node_loop->distance = UINT_MAX; // Represents infinity
        current_node_loop->visited = 0;          // 0 for not visited
        current_node_loop->prev_in_spt = NULL;   // No predecessor yet
    }

    // Identify the end (destination) and start (source) nodes using FindNode.
    // The order of calls to FindNode is inferred from the original snippet's variable usage.
    Node *end_node = FindNode(); // Assumed to return the destination node
    if (end_node == NULL) {
        return NULL;
    }

    Node *start_node = FindNode(); // Assumed to return the source node
    if (start_node == NULL) {
        return NULL;
    }

    // Set the distance of the start node to 0
    start_node->distance = 0;

    // --- Dijkstra's Algorithm Main Loop ---
    // The loop runs up to NumNodes times, effectively processing each node once.
    for (unsigned int i = 0; i < NumNodes; ++i) {
        // Find the unvisited node with the minimum distance
        Node *current_node_dijkstra = FindMinDistanceNode();
        if (current_node_dijkstra == NULL) {
            // No reachable unvisited nodes left; path not found or graph disconnected
            return NULL;
        }

        // Mark the current node as visited
        current_node_dijkstra->visited = 1; // 1 for visited

        // If the end node has been reached, the shortest path is found, so break
        if (current_node_dijkstra == end_node) {
            break;
        }

        // --- Edge Relaxation ---
        // Iterate through all edges to update distances of neighbors
        for (Edge *current_edge = Edges; current_edge != NULL; current_edge = current_edge->next_global) {
            // Check if current_node_dijkstra is one of the nodes connected by this edge
            // Relaxation for node_B if current_node_dijkstra is node_A
            if (current_node_dijkstra == current_edge->node_A) {
                Node *neighbor_node = current_edge->node_B;
                unsigned int new_distance = current_node_dijkstra->distance + current_edge->weight;
                if (new_distance < neighbor_node->distance) {
                    neighbor_node->distance = new_distance;
                    neighbor_node->prev_in_spt = current_node_dijkstra;
                }
            }
            // Relaxation for node_A if current_node_dijkstra is node_B (for undirected graphs)
            if (current_node_dijkstra == current_edge->node_B) {
                Node *neighbor_node = current_edge->node_A;
                unsigned int new_distance = current_node_dijkstra->distance + current_edge->weight;
                if (new_distance < neighbor_node->distance) {
                    neighbor_node->distance = new_distance;
                    neighbor_node->prev_in_spt = current_node_dijkstra;
                }
            }
        }
    }

    // --- Path Reconstruction Phase ---
    unsigned char path_length = 0;
    Node *current_node_path = end_node;

    // Traverse backward from the end_node to the start_node to determine the path length
    // This loop counts the number of nodes in the path.
    while (current_node_path != NULL && current_node_path != start_node) {
        path_length++;
        current_node_path = current_node_path->prev_in_spt;
    }

    // If current_node_path is NULL before reaching start_node, it means the path was not found
    if (current_node_path == NULL) {
        return NULL;
    }
    // Add the start_node itself to the path length
    path_length++;

    *path_length_out = path_length; // Store the calculated path length in the output parameter

    // Allocate memory for the path array (an array of node values/IDs)
    // The original code used `(uint)*param_3 << 2` which is `path_length * 4`,
    // implying `sizeof(int)` is 4 bytes. `calloc` takes `num_elements` and `size_of_each_element`.
    path_array = calloc(path_length, sizeof(int));
    if (path_array == NULL) {
        // Memory allocation failed; terminate the program or handle the error appropriately
        exit(EXIT_FAILURE);
    }

    // Fill the path array by traversing backward from the end_node to the start_node
    current_node_path = end_node;
    unsigned int path_idx = path_length - 1; // Start filling from the last index of the array

    while (current_node_path != NULL && path_idx < path_length) {
        ((int *)path_array)[path_idx] = current_node_path->value; // Store the node's value
        if (current_node_path == start_node) {
            break; // Reached the start node, path construction is complete
        }
        current_node_path = current_node_path->prev_in_spt;
        if (current_node_path == NULL && path_idx > 0) {
            // Path became disconnected unexpectedly (should not happen if previous checks passed)
            free(path_array);
            return NULL;
        }
        path_idx--; // Move to the previous index for the next node in the path
    }

    // --- Cleanup Phase ---
    // Free all dynamically allocated nodes and edges, and reset global counters
    DestroyNodes();
    DestroyEdges();
    NumNodes = 0;
    NumEdges = 0;

    return path_array; // Return the dynamically allocated array containing the path
}