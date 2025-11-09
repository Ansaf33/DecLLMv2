#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h> // For write/read

// --- Type Definitions ---
// Original types:
// undefined   -> void
// undefined1  -> uint8_t
// undefined4  -> uint32_t
// ulong       -> uint64_t
// uint        -> uint32_t
// byte        -> uint8_t
// ushort      -> uint16_t

// Router structure (0x108 bytes)
typedef struct Router {
    uint32_t as_number; // AS number, 0 indicates unused
    char name[256];     // Router name
    uint32_t ip;        // IP address
} Router;

// Route node structure (0x10 bytes)
typedef struct RouteNode {
    struct RouteNode *child[2]; // Pointers to left (0) and right (1) children
    uint32_t ip_prefix;         // IP prefix for this route node
    uint16_t as_number;         // Associated AS number (0 for intermediate nodes)
    uint8_t flags_and_length;   // Bits 0-2: Flags, Bits 3-7: Prefix length (0-31)
    uint8_t padding;            // Padding to make the struct 16 bytes
} RouteNode;

// --- Global Variables ---
char g_buffer[1024]; // Buffer for readline (0x400 max length + null terminator)
int g_enable = 0;    // Enable mode flag

Router g_routers[0x10000]; // Array of routers, indexed 1 to 0xFFFF

RouteNode g_default_route = {0}; // The root of the routing tree, representing the default route
uint16_t g_default_as = 0;       // The AS for the default route (DAT_00016534)
uint8_t g_default_route_flags = 0; // Flags for the default route (DAT_00016537)

RouteNode **g_route_memory = NULL; // Array of pointers to RouteNode blocks
unsigned int g_route_mem_length = 0; // Number of allocated RouteNode blocks (max 0x400)

// Prompt strings
const char *PROMPT_ENABLE_STR = "router> "; // DAT_000140e0
const char *PROMPT_NORMAL_STR = "user> ";   // DAT_000140e3

// --- Mock/Helper Functions ---
// Mock for transmit. Assumes file descriptor 1 is stdout.
ssize_t transmit(int fd, const void *buf, size_t count, void *unused) {
    return write(fd, buf, count);
}

// Mock for receive. Assumes file descriptor 0 is stdin.
ssize_t receive(int fd, void *buf, size_t count, int *bytes_received) {
    ssize_t res = read(fd, buf, count);
    if (res > 0) {
        *bytes_received = res;
    } else {
        *bytes_received = 0; // No bytes read or error
    }
    return res;
}

// Mock for allocate.
// Original signature: int allocate(size_t size, int param_2, void **out_ptr);
// Returns 0 on success, non-zero on failure.
int allocate(size_t size, int unused_param, void **out_ptr) {
    *out_ptr = malloc(size);
    return (*out_ptr == NULL);
}

// Function: print
void print(char *message) {
  transmit(1, message, strlen(message), NULL);
}

// Function: readline
char *readline(const char *prompt) {
  print((char*)prompt); // Cast to char* for print function
  int bytes_read;
  int i = 0;
  
  do {
    if (receive(0, g_buffer + i, 1, &bytes_read) <= 0 || bytes_read != 1) {
      return NULL; // Error or EOF
    }
    if (g_buffer[i] == '\n') {
      g_buffer[i] = '\0';
      return g_buffer;
    }
    i++;
  } while (i < sizeof(g_buffer) - 1); // Leave space for null terminator
  
  g_buffer[sizeof(g_buffer) - 1] = '\0'; // Ensure null termination if buffer fills
  return NULL; // Buffer full without newline
}

// Function: parse_ip
// Parses an IP string (e.g., "192.168.1.1") into a uint32_t.
// Returns 1 on success, 0 on failure.
uint32_t parse_ip(char *ip_string, uint32_t *out_ip) {
  char *segments[4];
  char *temp_ip_string = strdup(ip_string); // strsep modifies the string
  if (!temp_ip_string) return 0; // Memory allocation failed

  char *current_ptr = temp_ip_string;
  for (int i = 0; i < 4; ++i) {
    segments[i] = strsep(&current_ptr, ".");
    if (segments[i] == NULL) {
      free(temp_ip_string);
      return 0; // Not enough segments
    }
  }

  // Check if there are extra segments after the fourth one
  if (current_ptr != NULL) {
    free(temp_ip_string);
    return 0;
  }

  uint32_t ip_parts[4];
  for (int i = 0; i < 4; ++i) {
    char *endptr;
    unsigned long val = strtoul(segments[i], &endptr, 10);
    if (*endptr != '\0' || val > 255) { // Check for non-numeric characters or out of range
      free(temp_ip_string);
      return 0;
    }
    ip_parts[i] = (uint32_t)val;
  }
  
  *out_ip = (ip_parts[0] << 24) | (ip_parts[1] << 16) | (ip_parts[2] << 8) | ip_parts[3];
  free(temp_ip_string);
  return 1;
}

// Function: valid_router
// Checks if a router entry is valid (AS number is not 0 and within bounds).
uint32_t valid_router(const Router *router) {
  return (router != NULL && router->as_number != 0 && router->as_number <= 0xffff);
}

// Function: get_router
// Retrieves a pointer to a Router struct based on its AS ID.
Router *get_router(uint32_t as_id) {
  if (as_id == 0 || as_id >= 0x10000) return NULL; // Invalid AS ID
  return &g_routers[as_id - 1]; // AS IDs are 1-indexed for g_routers array
}

// Function: mask_to_length
// Converts an IP subnet mask (e.g., 255.255.255.0) to a prefix length (e.g., 24).
// Returns 0 for an invalid mask or 0.0.0.0, 32 for 255.255.255.255.
int mask_to_length(uint32_t mask) {
  if (mask == 0) return 0;
  if (mask == 0xFFFFFFFF) return 32;

  for (int length = 1; length <= 32; ++length) {
      // Check if the mask matches a contiguous block of '1's from the MSB.
      if (mask == ((uint32_t)-1 << (32 - length))) {
          return length;
      }
  }
  return 0; // Not a valid contiguous mask
}

// Function: route_mask
// Calculates the subnet mask corresponding to a route node's prefix length.
uint32_t route_mask(const RouteNode *node) {
  if (node == &g_default_route) {
    return 0; // Default route has a mask of 0
  }
  // The prefix length is stored in bits 3-7 of flags_and_length (0-31)
  uint32_t length = node->flags_and_length >> 3;
  return (uint32_t)-1 << (32 - (length + 1)); // Length is 0-31, so +1 for actual length
}

// Function: route_bit
// Determines the bit used for branching at this node's level in the tree.
uint32_t route_bit(const RouteNode *node) {
  if (node == &g_default_route) {
    return 0x80000000; // For the default route, branch on the MSB (bit 31)
  }
  // Length is 0-31, so the bit to check is (31 - length)
  uint32_t length = node->flags_and_length >> 3;
  return 1U << (31 - (length + 1)); // Branch on the bit *after* the current prefix length
}

// Function: route_child
// Returns the appropriate child node (left or right) based on the IP address.
RouteNode *route_child(const RouteNode *node, uint32_t ip_addr) {
  uint32_t bit_to_check = route_bit(node);
  return ((ip_addr & bit_to_check) == 0) ? node->child[0] : node->child[1];
}

// Function: lookup_route
// Searches for a route node in the routing tree.
// Returns the found RouteNode, or NULL if not found.
// If parent_out is not NULL, stores a pointer to the parent node.
RouteNode *lookup_route(uint32_t ip_prefix, int length, RouteNode **parent_out) {
  if (length == 0) {
    if (ip_prefix == 0) {
      if (parent_out != NULL) {
        *parent_out = NULL; // Default route has no parent in the tree
      }
      return &g_default_route;
    }
    return NULL; // Non-zero IP with zero length is invalid unless it's the default route
  }

  RouteNode *current_node = &g_default_route;
  RouteNode *found_node = NULL;
  RouteNode *parent_node = NULL;

  while (current_node != NULL) {
    uint32_t current_node_prefix_length = (current_node == &g_default_route) ? 0 : (current_node->flags_and_length >> 3) + 1;
    uint32_t current_node_mask = (current_node_prefix_length == 0) ? 0 : (uint32_t)-1 << (32 - current_node_prefix_length);

    // Check if current_node matches the target prefix and length
    if (ip_prefix == current_node->ip_prefix && (uint32_t)length == current_node_prefix_length) {
      found_node = current_node;
      break;
    }

    // Check for mismatch in common prefix or if current_node is too specific
    if (((current_node->ip_prefix ^ ip_prefix) & current_node_mask) != 0 || current_node_prefix_length > (uint32_t)length) {
        break; // Mismatch or current node is more specific than target
    }

    // If current_node is a leaf node (indicated by specific flag combination), stop
    // Original: (*(byte *)((int)puVar1 + 0xf) & 0xf8) == 0xf8
    // This checks if bits 3-7 are all 1s, meaning length 31.
    if ((current_node->flags_and_length & 0xF8) == 0xF8) {
        break;
    }
    
    parent_node = current_node;
    current_node = route_child(current_node, ip_prefix);
  }

  if (found_node != NULL && parent_out != NULL) {
    *parent_out = parent_node;
  }
  return found_node;
}

// Function: prompt_edit_router
// Prompts the user to modify a router's name and IP.
void prompt_edit_router(Router *router) {
  printf("AS %u\n", router->as_number);
  printf(" Name: %s\n", router->name);
  printf(" IP: %u.%u.%u.%u\n", (router->ip >> 24) & 0xFF, (router->ip >> 16) & 0xFF,
         (router->ip >> 8) & 0xFF, router->ip & 0xFF);
  
  char *response;
  
  response = readline("Modify name? ");
  if (response != NULL && strcmp(response, "y") == 0) {
    response = readline("New name? ");
    if (response != NULL) {
      strncpy(router->name, response, sizeof(router->name) - 1);
      router->name[sizeof(router->name) - 1] = '\0'; // Ensure null termination
    }
  }
  
  response = readline("Modify IP? ");
  if (response != NULL && strcmp(response, "y") == 0) {
    response = readline("New IP? ");
    if (response != NULL) {
      uint32_t new_ip;
      if (parse_ip(response, &new_ip) != 0) {
        router->ip = new_ip;
      }
    }
  }
  return;
}

// Function: allocate_route
// Allocates a new RouteNode from the custom memory pool.
RouteNode *allocate_route(void) {
  RouteNode *block_ptr = NULL;
  unsigned int block_idx = 0;

  // Find a block with free nodes (block_ptr->as_number stores the index of the first free node)
  for (block_idx = 0; block_idx < g_route_mem_length; ++block_idx) {
    block_ptr = g_route_memory[block_idx];
    if (block_ptr != NULL && block_ptr->as_number != 0) {
      break;
    }
  }

  if (block_ptr == NULL || block_ptr->as_number == 0) { // No free nodes found in existing blocks
    if (g_route_mem_length == 0x400) { // Max number of blocks reached
      return NULL;
    }

    // Allocate a new block of RouteNodes (0x1000 nodes, 0x10 bytes each = 0x10000 bytes)
    RouteNode *new_block;
    if (allocate(0x1000 * sizeof(RouteNode), 0, (void**)&new_block) != 0) {
      return NULL;
    }
    
    // Initialize new block's free list
    // The first node (index 0) in the block acts as a header, storing the head of the free list.
    new_block[0].as_number = 1; // Head of free list is at index 1
    for (uint32_t i = 1; i < 0x1000; ++i) { // Nodes 1 to 0xFFF (4095 nodes)
      RouteNode *node = &new_block[i];
      node->flags_and_length |= 4; // Set free flag (bit 2)
      node->ip_prefix = (i == 0xFFF) ? 0 : i + 1; // Link to the next free node's index
    }
    
    // Add new block to g_route_memory array
    g_route_memory = realloc(g_route_memory, (g_route_mem_length + 1) * sizeof(RouteNode*));
    if (g_route_memory == NULL) {
        free(new_block);
        return NULL;
    }
    g_route_memory[g_route_mem_length] = new_block;
    g_route_mem_length++;
    block_ptr = new_block;
  }

  // Get a node from the free list
  uint16_t node_idx = block_ptr[0].as_number;
  RouteNode *new_node = &block_ptr[node_idx]; // Get node at head of free list
  block_ptr[0].as_number = (uint16_t)new_node->ip_prefix; // Update head to next free node's index
  
  memset(new_node, 0, sizeof(RouteNode)); // Clear the allocated node
  return new_node;
}

// Function: free_route
// Frees a RouteNode and returns it to the custom memory pool.
void free_route(RouteNode *node_to_free) {
  RouteNode *block_ptr = NULL;
  unsigned int block_idx = 0;

  // Find which block this node belongs to
  while (block_idx < g_route_mem_length) {
    block_ptr = g_route_memory[block_idx];
    if (block_ptr != NULL && (uintptr_t)node_to_free >= (uintptr_t)block_ptr && 
        (uintptr_t)node_to_free < (uintptr_t)block_ptr + 0x1000 * sizeof(RouteNode)) {
      break;
    }
    block_idx++;
  }
  
  if (block_idx == g_route_mem_length) {
    exit(9); // Critical error: node not found in any allocated block
  }

  // Add node back to the free list
  node_to_free->flags_and_length |= 4; // Set free flag (bit 2)
  node_to_free->ip_prefix = block_ptr[0].as_number; // Store old head as next free
  block_ptr[0].as_number = (uint16_t)(((uintptr_t)node_to_free - (uintptr_t)block_ptr) / sizeof(RouteNode)); // Update head to this node's index
  return;
}

// Function: delete_route
// Deletes a route node from the routing tree.
void delete_route(RouteNode *node_to_delete, RouteNode *parent_node) {
  if (node_to_delete == NULL) return;

  if (parent_node == NULL) {
    // If parent_node is not supplied, find it.
    // lookup_route returns the node itself, and puts its parent in the third argument.
    int length = (node_to_delete->flags_and_length >> 3) + 1;
    if (lookup_route(node_to_delete->ip_prefix, length, &parent_node) == NULL) {
      exit(1); // Route not found for deletion, critical error
    }
  }

  if (parent_node == NULL) {
    exit(2); // Parent not found, critical error
  }

  RouteNode **child_ptr_to_update = NULL;
  RouteNode *other_child = NULL;

  if (node_to_delete == parent_node->child[0]) {
    child_ptr_to_update = &parent_node->child[0];
    other_child = parent_node->child[1];
  } else if (node_to_delete == parent_node->child[1]) {
    child_ptr_to_update = &parent_node->child[1];
    other_child = parent_node->child[0];
  } else {
    exit(3); // Node to delete is not a child of parent_node, critical error
  }

  // Mark node_to_delete as not in use (clear AS number and specific flag)
  node_to_delete->as_number = 0;
  node_to_delete->flags_and_length &= ~2; // Clear bit 1 (0xfd)

  if (node_to_delete->child[0] == NULL && node_to_delete->child[1] == NULL) {
    // Case 1: node_to_delete is a leaf node
    // Check if parent node is also a non-route intermediate node that can be collapsed
    if ((parent_node == &g_default_route) || (parent_node->as_number != 0) || ((parent_node->flags_and_length & 2) != 0) || (other_child == NULL)) {
        // Parent is default route, or parent is a route, or parent has no other child
        // Simply remove node_to_delete.
        *child_ptr_to_update = NULL;
        free_route(node_to_delete);
    } else {
        // Parent is an intermediate node (not a route itself) with only one other child (other_child != NULL).
        // Collapse parent by linking grandparent to other_child.
        RouteNode *grandparent_node = NULL;
        int parent_length = (parent_node->flags_and_length >> 3) + 1;
        if (lookup_route(parent_node->ip_prefix, parent_length, &grandparent_node) != NULL && grandparent_node != NULL) {
            if (grandparent_node->child[0] == parent_node) {
              grandparent_node->child[0] = other_child;
            } else {
              grandparent_node->child[1] = other_child;
            }
            free_route(parent_node);
            free_route(node_to_delete);
        } else {
            // If parent_node has no grandparent (e.g., direct child of g_default_route),
            // or lookup_route failed, just remove node_to_delete.
            *child_ptr_to_update = NULL;
            free_route(node_to_delete);
        }
    }
  } else if (node_to_delete->child[0] == NULL) {
    // Case 2: node_to_delete has only a right child
    *child_ptr_to_update = node_to_delete->child[1];
    free_route(node_to_delete);
  } else { // node_to_delete->child[1] == NULL
    // Case 3: node_to_delete has only a left child
    *child_ptr_to_update = node_to_delete->child[0];
    free_route(node_to_delete);
  }
  return;
}

// Function: cmd_add_route
// Adds a new route to the routing table.
void cmd_add_route(char *args) {
  char *ip_str = strsep(&args, " ");
  char *mask_str = strsep(&args, " ");
  char *as_str = strsep(&args, " ");

  uint32_t ip_prefix_val;
  uint32_t mask_val;
  unsigned long as_id_val;

  bool success = false;
  if (ip_str && parse_ip(ip_str, &ip_prefix_val)) {
    if (mask_str && parse_ip(mask_str, &mask_val)) {
      if (as_str) {
        char *endptr;
        as_id_val = strtoul(as_str, &endptr, 10);
        if (*endptr == '\0' && as_id_val < 0x10000) {
          if (as_id_val == 0 || valid_router(get_router(as_id_val))) {
            uint32_t length = mask_to_length(mask_val); // Full length 0-32
            if (length <= 32 && (ip_prefix_val & ~mask_val) == 0) { // Check prefix validity
              if (length == 0) {
                g_default_as = (uint16_t)as_id_val;
                success = true;
              } else {
                uint32_t node_length = length - 1; // Internal representation 0-31 for lengths 1-32
                RouteNode *current_node = &g_default_route;
                RouteNode *allocated_node = NULL; // The node that will become the target route

                while (allocated_node == NULL) { // Loop until the target node is found or created
                  uint32_t bit = route_bit(current_node); // Bit to check for branching
                  RouteNode **child_link;
                  RouteNode *child_node;

                  if ((bit & ip_prefix_val) == 0) {
                    child_link = &current_node->child[0];
                  } else {
                    child_link = &current_node->child[1];
                  }
                  child_node = *child_link;

                  if (child_node == NULL) {
                    // No child, allocate a new node and link it
                    allocated_node = allocate_route();
                    if (allocated_node == NULL) {
                      print("OUT OF MEMORY\n");
                      return;
                    }
                    *child_link = allocated_node;
                    // No break here, continue to set properties after loop
                  } else {
                    // Child exists, check if it's the target or if we need to insert/traverse
                    uint32_t child_node_length = (child_node->flags_and_length >> 3); // 0-31

                    if (node_length == child_node_length) {
                      // Exact match in prefix length, update existing node
                      allocated_node = child_node;
                      // No break here, continue to set properties after loop
                    } else {
                      // Calculate mask for the current common prefix length (node_length)
                      uint32_t current_len_mask = (uint32_t)-1 << (31 - node_length); // mask for length 0-31

                      if (((child_node->ip_prefix & current_len_mask) == (ip_prefix_val & current_len_mask)) &&
                          (node_length < child_node_length)) {
                        // Prefixes match up to target length, but child is more specific.
                        // Insert an intermediate node.
                        RouteNode *intermediate_node = allocate_route();
                        if (intermediate_node == NULL) {
                          print("OUT OF MEMORY\n");
                          return;
                        }
                        *child_link = intermediate_node; // Link intermediate node
                        
                        // Attach child_node to intermediate_node
                        uint32_t bit_at_node_length = 1U << (31 - node_length);
                        if ((bit_at_node_length & child_node->ip_prefix) == 0) {
                          intermediate_node->child[0] = child_node;
                        } else {
                          intermediate_node->child[1] = child_node;
                        }
                        allocated_node = intermediate_node;
                        // No break here, continue to set properties after loop
                      } else {
                        // Prefixes don't match up to target length, or child is less specific.
                        // Need to insert a common ancestor node.
                        RouteNode *common_ancestor_node = allocate_route();
                        if (common_ancestor_node == NULL) {
                          print("OUT OF MEMORY\n");
                          return;
                        }
                        *child_link = common_ancestor_node; // Link common ancestor

                        // Find the most significant bit where ip_prefix_val and child_node->ip_prefix differ
                        uint32_t diff_bits = ip_prefix_val ^ child_node->ip_prefix;
                        int common_prefix_len = 0; // Length of the common ancestor's prefix
                        for (int i = 31; i >= 0; --i) {
                            if ((diff_bits >> i) & 1) { // Found the first differing bit
                                common_prefix_len = 31 - i;
                                break;
                            }
                        }
                        
                        common_ancestor_node->flags_and_length = (uint8_t)(common_prefix_len * 8); // Store common length
                        common_ancestor_node->ip_prefix = ip_prefix_val & ((uint32_t)-1 << (32 - common_prefix_len));

                        // Allocate the new route node for ip_prefix_val
                        allocated_node = allocate_route();
                        if (allocated_node == NULL) {
                          print("OUT OF MEMORY\n");
                          return;
                        }

                        // Attach ip_prefix_val and child_node->ip_prefix to common_ancestor_node
                        uint32_t common_bit_for_children = 1U << (31 - common_prefix_len);
                        if ((ip_prefix_val & common_bit_for_children) == 0) {
                          common_ancestor_node->child[0] = allocated_node;
                          common_ancestor_node->child[1] = child_node;
                        } else {
                          common_ancestor_node->child[0] = child_node;
                          common_ancestor_node->child[1] = allocated_node;
                        }
                        // No break here, continue to set properties after loop
                      }
                    }
                  }
                  // If allocated_node is still NULL, continue traversing.
                  // This happens if child_node exists but is not the target and no insertion happened.
                  if (allocated_node == NULL) {
                    current_node = child_node;
                  } else {
                    break; // Node found or created, exit while loop to set properties
                  }
                }

                // Set properties of the (newly allocated or existing) route node
                allocated_node->ip_prefix = ip_prefix_val;
                allocated_node->as_number = (uint16_t)as_id_val;
                allocated_node->flags_and_length = (uint8_t)(node_length * 8); // Store internal length (0-31)
                success = true;
              }
            }
          }
        }
      }
    }
  }

  if (!success) {
    print("BAD ARGUMENTS\n");
  }
}

// Function: cmd_delete_route
// Deletes a route from the routing table.
void cmd_delete_route(char *args) {
  char *ip_str = strsep(&args, " ");
  char *mask_str = strsep(&args, " ");

  uint32_t ip_val;
  uint32_t mask_val;
  
  bool success = false;
  if (ip_str && parse_ip(ip_str, &ip_val)) {
    if (mask_str && parse_ip(mask_str, &mask_val)) {
      int length = mask_to_length(mask_val);
      if (length <= 32) {
        if (length == 0) {
          if (ip_val == 0) { // Deleting default route (0.0.0.0/0)
            g_default_as = 0;
            success = true;
          }
        } else {
          RouteNode *parent_node = NULL;
          RouteNode *node_to_delete = lookup_route(ip_val, length, &parent_node);
          
          if (node_to_delete != NULL && (node_to_delete->as_number != 0 || (node_to_delete->flags_and_length & 2) != 0)) {
            // Only delete if it's an actual route (has an AS number or "in-use" flag)
            delete_route(node_to_delete, parent_node);
            success = true;
          }
        }
      }
    }
  }

  if (!success) {
    print("ROUTE NOT FOUND\n"); // Original message for failed deletion
  }
}

// Function: cmd_query_route
// Queries the routing table for the next hop of a given IP address.
void cmd_query_route(char *args) {
  char *ip_str = strsep(&args, " ");
  uint32_t ip_to_query;

  if (ip_str == NULL || parse_ip(ip_str, &ip_to_query) == 0) {
    print("BAD ARGUMENTS\n");
    return;
  }

  uint32_t next_hop_as = 0;
  RouteNode *current_node = &g_default_route;

  while (current_node != NULL) {
    uint32_t current_node_prefix_length;
    uint32_t current_node_mask;

    if (current_node == &g_default_route) {
      current_node_prefix_length = 0;
      current_node_mask = 0; // Default route matches everything
    } else {
      current_node_prefix_length = (current_node->flags_and_length >> 3) + 1;
      current_node_mask = (uint32_t)-1 << (32 - current_node_prefix_length);
    }
    
    // Check if the current node's prefix matches the queried IP
    if ((current_node->ip_prefix & current_node_mask) != (ip_to_query & current_node_mask)) {
      break; // Mismatch in common prefix, stop traversal
    }

    // If current node is an actual route, update next_hop_as
    // Original: (*(short *)(local_24 + 3) != 0) || ((*(byte *)((int)local_24 + 0xf) & 2) != 0)
    // This checks if as_number is non-zero OR if bit 1 of flags_and_length is set (in-use flag)
    if (current_node->as_number != 0 || (current_node->flags_and_length & 2) != 0) {
      next_hop_as = current_node->as_number;
    }

    // If current node is a leaf node (indicated by specific flag combination), stop traversal
    if ((current_node->flags_and_length & 0xF8) == 0xF8) {
        break;
    }

    // Move to the next child
    current_node = route_child(current_node, ip_to_query);
  }

  printf("Next hop for %u.%u.%u.%u is ", (ip_to_query >> 24) & 0xFF, (ip_to_query >> 16) & 0xFF,
         (ip_to_query >> 8) & 0xFF, ip_to_query & 0xFF);
  
  if (next_hop_as == 0) {
    printf("BLACKHOLE.\n");
  } else {
    Router *router = get_router(next_hop_as);
    if (router == NULL || !valid_router(router)) {
        printf("UNKNOWN ROUTER (AS %u).\n", next_hop_as);
    } else {
        printf("%u.%u.%u.%u, AS %u (%s)\n", (router->ip >> 24) & 0xFF,
               (router->ip >> 16) & 0xFF, (router->ip >> 8) & 0xFF,
               router->ip & 0xFF, router->as_number, router->name);
    }
  }
}

// Function: cmd_add_router
// Adds a new router entry.
void cmd_add_router(char *args) {
  char *as_str = strsep(&args, " ");
  unsigned long as_id;

  bool success = false;
  if (as_str) {
    char *endptr;
    as_id = strtoul(as_str, &endptr, 10);
    if (*endptr == '\0' && as_id != 0 && as_id < 0x10000) {
      Router *router = get_router(as_id);
      if (router != NULL && !valid_router(router)) { // Check if AS is not already in use
        memset(router, 0, sizeof(Router));
        router->as_number = (uint32_t)as_id;
        prompt_edit_router(router);
        success = true;
      }
    }
  }

  if (!success) {
    print("BAD ARGUMENTS\n");
  }
}

// Function: cmd_delete_router
// Deletes a router entry and all associated routes.
void cmd_delete_router(char *args) {
  char *as_str = strsep(&args, " ");
  unsigned long as_id;

  bool success = false;
  if (as_str) {
    char *endptr;
    as_id = strtoul(as_str, &endptr, 10);
    if (*endptr == '\0' && as_id != 0 && as_id < 0x10000) {
      Router *router_to_delete = get_router(as_id);
      if (router_to_delete != NULL && valid_router(router_to_delete)) {
        // Iterate through all allocated RouteNode blocks and nodes
        for (unsigned int block_idx = 0; block_idx < g_route_mem_length; ++block_idx) {
          RouteNode *block_start = g_route_memory[block_idx];
          for (uint32_t node_idx = 1; node_idx < 0x1000; ++node_idx) { // Skip block header node (index 0)
            RouteNode *node = &block_start[node_idx];
            // Check if node is in use (not free, bit 2 clear) and points to this AS
            if (!((node->flags_and_length & 4) != 0) && (as_id == node->as_number)) {
              delete_route(node, NULL); // Pass NULL for parent, delete_route will find it
            }
          }
        }
        
        // If this was the default AS, clear it
        if (as_id == g_default_as) {
          g_default_as = 0;
          g_default_route_flags &= ~2; // Clear default route's "in-use" flag
        }
        router_to_delete->as_number = 0; // Mark router as not valid
        success = true;
      }
    }
  }

  if (!success) {
    print("BAD ARGUMENTS\n");
  }
}

// Function: cmd_edit_router
// Edits an existing router entry.
void cmd_edit_router(char *args) {
  char *as_str = strsep(&args, " ");
  unsigned long as_id;

  bool success = false;
  if (as_str) {
    char *endptr;
    as_id = strtoul(as_str, &endptr, 10);
    if (*endptr == '\0' && as_id < 0x10000) { // Original code allowed AS 0 here
      Router *router = get_router(as_id);
      if (router != NULL && valid_router(router)) {
        prompt_edit_router(router);
        success = true;
      }
    }
  }

  if (!success) {
    print("BAD ARGUMENTS\n");
  }
}

// Function: cmd_list_router
// Lists all configured routers.
void cmd_list_router(char *args) { // args is unused, but kept for consistency
  for (uint32_t i = 1; i < 0x10000; ++i) {
    Router *router = get_router(i);
    if (router != NULL && valid_router(router)) {
      printf("AS %u\n", router->as_number);
      printf(" Name: %s\n", router->name);
      printf(" IP: %u.%u.%u.%u\n", (router->ip >> 24) & 0xFF, (router->ip >> 16) & 0xFF,
             (router->ip >> 8) & 0xFF, router->ip & 0xFF);
    }
  }
}

// Function: cmd_enable_mode
// Enables privileged mode if the correct password is provided.
void cmd_enable_mode(char *args) {
  char *password = strsep(&args, " ");
  if (password == NULL) {
    print("BAD ARGUMENTS\n");
  } else {
    if (strcmp(password, "racoiaws") == 0) {
      g_enable = 1;
    } else {
      print("BAD PASSWORD\n");
    }
  }
}

// Function: main
int main(void) {
  // Allocate initial array for route node block pointers
  if (allocate(0x400 * sizeof(RouteNode*), 0, (void**)&g_route_memory) != 0) {
    print("Failed to allocate route memory management array\n");
    return 1;
  }
  // Initialize route memory length to 0, as no blocks are allocated yet.
  g_route_mem_length = 0;

  bool running = true;
  while (running) {
    const char *prompt_str = g_enable ? PROMPT_ENABLE_STR : PROMPT_NORMAL_STR;
    char *line = readline(prompt_str);

    if (line == NULL) {
      // EOF or error, exit loop
      running = false;
      continue;
    }

    char *cmd_line_ptr = line; // strsep modifies this pointer
    char *command = strsep(&cmd_line_ptr, " ");

    if (command == NULL || command[0] == '\0') {
      continue; // Empty command
    }

    // Dispatch commands based on mode
    if (g_enable) {
      if (strcmp(command, "add") == 0) {
        cmd_add_router(cmd_line_ptr);
      } else if (strcmp(command, "delete") == 0) {
        cmd_delete_router(cmd_line_ptr);
      } else if (strcmp(command, "edit") == 0) {
        cmd_edit_router(cmd_line_ptr);
      } else if (strcmp(command, "list") == 0) {
        cmd_list_router(cmd_line_ptr);
      } else if (strcmp(command, "quit") == 0) {
        g_enable = 0; // Exit enable mode, return to normal user mode
      } else {
        print("UNKNOWN COMMAND\n");
      }
    } else { // Normal user mode
      if (strcmp(command, "add") == 0) {
        cmd_add_route(cmd_line_ptr);
      } else if (strcmp(command, "delete") == 0) {
        cmd_delete_route(cmd_line_ptr);
      } else if (strcmp(command, "query") == 0) {
        cmd_query_route(cmd_line_ptr);
      } else if (strcmp(command, "enable") == 0) {
        cmd_enable_mode(cmd_line_ptr);
      } else if (strcmp(command, "quit") == 0) {
        running = false; // Exit program
      } else {
        print("UNKNOWN COMMAND\n");
      }
    }
  }

  print("GOOD-BYE\n");

  // Free all allocated memory blocks
  if (g_route_memory) {
      for (unsigned int i = 0; i < g_route_mem_length; ++i) {
          free(g_route_memory[i]);
      }
      free(g_route_memory);
  }
  
  return 0;
}