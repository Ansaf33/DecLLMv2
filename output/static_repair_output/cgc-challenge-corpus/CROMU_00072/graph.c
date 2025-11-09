#include <stdint.h> // For uint32_t, uint8_t
#include <stdlib.h> // For calloc, exit, size_t
#include <limits.h> // For UINT_MAX (for unsigned int)

// Define Node and Edge structures based on decompiled access patterns
// This assumes a 32-bit architecture where pointers and integers are 4 bytes.
// The array-like access in the original code (e.g., local_8[4]) strongly suggests
// 4-byte increments for members.

typedef struct Node Node;
typedef struct Edge Edge;

struct Node {
  int visited;           // 0x00: 0 = unvisited, 1 = visited
  Node* prev_in_path;    // 0x04: Pointer to previous node in shortest path
  Node* next_in_list;    // 0x08: Pointer to next node in the global Nodes list
  uint32_t id;           // 0x0C: Node identifier
  uint32_t distance;     // 0x10: Shortest distance from start node
};

struct Edge {
  Edge* next_in_list;    // 0x00: Pointer to next edge in the global Edges list
  Node* src;             // 0x04: Source node
  Node* dest;            // 0x08: Destination node
  uint32_t weight;       // 0x0C: Weight of the edge
};

// Global variables (must be declared as they are used in the functions)
Node* Nodes = NULL;
Edge* Edges = NULL;
uint32_t NumNodes = 0;
uint32_t NumEdges = 0;

// Placeholder for DestroyNodes function
void DestroyNodes() {
    Node* current = Nodes;
    while (current != NULL) {
        Node* next = current->next_in_list;
        free(current);
        current = next;
    }
    Nodes = NULL;
}

// Placeholder for DestroyEdges function
void DestroyEdges() {
    Edge* current = Edges;
    while (current != NULL) {
        Edge* next = current->next_in_list;
        free(current);
        current = next;
    }
    Edges = NULL;
}

// Placeholder for FindNode function (missing from snippet, inferred from usage)
// Assumes it finds a node by its ID.
Node* FindNode(uint32_t nodeId) {
    Node* current = Nodes;
    while (current != NULL) {
        if (current->id == nodeId) {
            return current;
        }
        current = current->next_in_list;
    }
    return NULL; // Node not found
}

// Function: FindMinDistanceNode
Node* FindMinDistanceNode(void) {
  uint32_t min_distance = UINT_MAX; // Initialize with maximum possible unsigned int value
  Node* min_node = NULL;
  
  for (Node* current_node = Nodes; current_node != NULL; current_node = current_node->next_in_list) {
    // Check if the node is unvisited and has a smaller distance
    if ((current_node->distance < min_distance) && (current_node->visited == 0)) {
      min_distance = current_node->distance;
      min_node = current_node;
    }
  }
  return min_node;
}

// Function: FindSpt (Find Shortest Path Tree or Shortest Path)
void* FindSpt(uint32_t start_node_id, uint32_t end_node_id, uint8_t* path_length_out) {
  uint32_t* path_array = NULL; // Array to store node IDs of the shortest path
  
  if ((Nodes == NULL) || (path_length_out == NULL)) {
    return NULL;
  }
  
  // Initialize all nodes: distance to infinity, unvisited, no predecessor
  for (Node* current_node_in_list = Nodes; current_node_in_list != NULL; current_node_in_list = current_node_in_list->next_in_list) {
    current_node_in_list->distance = UINT_MAX;
    current_node_in_list->visited = 0;
    current_node_in_list->prev_in_path = NULL;
  }
  
  // Find the start and end nodes based on their IDs
  Node* end_node = FindNode(end_node_id);
  if (end_node == NULL) {
    return NULL;
  }
  
  Node* start_node = FindNode(start_node_id);
  if (start_node == NULL) {
    return NULL;
  }
  
  // Set distance of the start node to 0
  start_node->distance = 0;
  
  // Main Dijkstra's loop: iterate NumNodes times to find shortest paths
  for (uint32_t i = 0; i < NumNodes; ++i) {
    Node* current_min_dist_node = FindMinDistanceNode();
    
    // If no unvisited node can be reached, a path might not exist
    if (current_min_dist_node == NULL) {
      break; // No more reachable unvisited nodes
    }
    
    current_min_dist_node->visited = 1; // Mark as visited
    
    // If the end node is visited, we've found its shortest path
    if (current_min_dist_node == end_node) {
      break;
    }
    
    // Iterate through all edges to update neighbors' distances
    for (Edge* current_edge = Edges; current_edge != NULL; current_edge = current_edge->next_in_list) {
      // Check if current_min_dist_node is the source of the edge
      if (current_min_dist_node == current_edge->src) {
        Node* neighbor = current_edge->dest;
        // If neighbor is unvisited and a shorter path is found
        if (neighbor->visited == 0 && current_min_dist_node->distance != UINT_MAX &&
            (current_min_dist_node->distance + current_edge->weight < neighbor->distance)) {
          neighbor->distance = current_min_dist_node->distance + current_edge->weight;
          neighbor->prev_in_path = current_min_dist_node;
        }
      }
      // Check if current_min_dist_node is the destination of the edge (for undirected graphs)
      if (current_min_dist_node == current_edge->dest) {
        Node* neighbor = current_edge->src;
        // If neighbor is unvisited and a shorter path is found
        if (neighbor->visited == 0 && current_min_dist_node->distance != UINT_MAX &&
            (current_min_dist_node->distance + current_edge->weight < neighbor->distance)) {
          neighbor->distance = current_min_dist_node->distance + current_edge->weight;
          neighbor->prev_in_path = current_min_dist_node;
        }
      }
    }
  }
  
  // Path reconstruction: traverse back from end_node to start_node
  *path_length_out = 0;
  Node* current_path_node = end_node;
  
  // Count path length
  while (current_path_node != NULL && current_path_node != start_node) {
      if (current_path_node->prev_in_path == NULL) {
          *path_length_out = 0; // No path found
          return NULL;
      }
      (*path_length_out)++;
      current_path_node = current_path_node->prev_in_path;
  }

  // If start node was reached, include it in the count
  if (current_path_node == start_node) {
      (*path_length_out)++;
  } else {
      *path_length_out = 0; // Start node not reachable from end node
      return NULL;
  }

  // Allocate memory for the path array (array of node IDs)
  path_array = calloc(*path_length_out, sizeof(uint32_t));
  if (path_array == NULL) {
    exit(1); // Memory allocation failed
  }
  
  // Fill the path array in reverse order, then it will be correct when read from index 0
  uint32_t path_idx = *path_length_out - 1;
  current_path_node = end_node;

  while (current_path_node != NULL && current_path_node != start_node) {
      path_array[path_idx--] = current_path_node->id;
      if (current_path_node->prev_in_path == NULL) { // Path integrity check during fill
          free(path_array);
          *path_length_out = 0;
          return NULL;
      }
      current_path_node = current_path_node->prev_in_path;
  }
  path_array[path_idx] = start_node->id; // Add the start node to the array
  
  // Clean up global graph structures as per original code's behavior
  DestroyNodes();
  DestroyEdges();
  NumNodes = 0;
  NumEdges = 0;
  
  return path_array; // Returns a void* pointer to an array of uint32_t (node IDs)
}