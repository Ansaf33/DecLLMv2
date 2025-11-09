#include <stdio.h>   // For puts, printf
#include <stdlib.h>  // For exit
#include <string.h>  // For strcpy, strlen, memset, memcpy

// Forward declarations for list types and data structures
// These types are inferred from the assembly snippet and represent the structure of data
// managed by the list functions and passed between the main program functions.

// Generic List Head for double lists
typedef struct DoubleListHead {
    void *first; // Pointer to the first node in the list
    int count;   // Number of elements in the list
    void *last;  // Pointer to the last node in the list
} DoubleListHead;

// Generic List Node for double lists
// The actual user data is assumed to be embedded within the node, starting at offset 0xc
typedef struct DoubleListNode {
    struct DoubleListNode *prev; // 0x00: Pointer to the previous node
    struct DoubleListNode *next; // 0x04: Pointer to the next node
    // 0x08-0x0b are unknown/padding, assumed to be part of the generic node structure
    char data_start[1]; // Placeholder for the actual data, which starts at offset 0xc
} DoubleListNode;

// Map structure
typedef struct Map {
    char name[32]; // 0x00-0x1f: Name of the map
    DoubleListHead *roads; // 0x20: Pointer to the list of roads in this map
} Map;

// Road data structure (embedded within a DoubleListNode at offset 0xc)
typedef struct RoadData {
    // These offsets are relative to the RoadData struct's start (which is node_base + 0xc)
    char padding_0c_1b[0x10]; // 0x00-0x0f (relative to RoadData) corresponds to 0x0c-0x1b (relative to DoubleListNode)
    DoubleListNode *self_node_ptr; // 0x10 (relative to RoadData) corresponds to 0x1c (relative to DoubleListNode)
    char name[32];                  // 0x14 (relative to RoadData) corresponds to 0x20 (relative to DoubleListNode)
    DoubleListHead *addresses;      // 0x34 (relative to RoadData) corresponds to 0x40 (relative to DoubleListNode)
    DoubleListHead *intersections;  // 0x38 (relative to RoadData) corresponds to 0x44 (relative to DoubleListNode)
    double length;                  // 0x3c (relative to RoadData) corresponds to 0x48 (relative to DoubleListNode)
} RoadData;

// Address data structure (embedded within a DoubleListNode at offset 0xc)
typedef struct AddressData {
    // These offsets are relative to the AddressData struct's start (which is node_base + 0xc)
    int unknown_00; // 0x00: Unknown field
    int unknown_04; // 0x04: Unknown field
    DoubleListNode *self_node_ptr; // 0x08: Pointer to its containing DoubleListNode
    int number;                          // 0x0c: Address number
    char street_name[32];                // 0x10: Street name (up to 0x2f)
    struct RoadData *road_ptr;           // 0x30: Pointer to the RoadData it belongs to
    struct RoadData *intersection_after;  // 0x34: Pointer to the RoadData of the intersection after this address
    double distance_after;               // 0x38: Distance to the intersection after
    struct RoadData *intersection_before; // 0x40: Pointer to the RoadData of the intersection before this address
    double distance_before;              // 0x44: Distance to the intersection before
} AddressData;

// Intersection data structure (embedded within a DoubleListNode at offset 0xc)
typedef struct IntersectionData {
    // These offsets are relative to the IntersectionData struct's start (which is node_base + 0xc)
    char padding_0c_2b[0x20]; // 0x00-0x1f (relative to IntersectionData) corresponds to 0x0c-0x2b (relative to DoubleListNode)
    DoubleListNode *self_node_ptr; // 0x20 (relative to IntersectionData) corresponds to 0x2c (relative to DoubleListNode)
    struct RoadData *road_intersection_ptr; // 0x24 (relative to IntersectionData) corresponds to 0x30 (relative to DoubleListNode) (The road that intersects the current road)
    struct RoadData *road_ptr; // 0x28 (relative to IntersectionData) corresponds to 0x34 (relative to DoubleListNode) (The current road this intersection is on)
    double distance_from_start; // 0x2c (relative to IntersectionData) corresponds to 0x38 (relative to DoubleListNode)
    struct IntersectionData *next_intersection_on_this_road; // 0x34 (relative to IntersectionData) corresponds to 0x40 (relative to DoubleListNode) (Pointer to the *data* of the next intersection on the *same road*)
    double distance_from_end; // 0x38 (relative to IntersectionData) corresponds to 0x44 (relative to DoubleListNode)
} IntersectionData;

// TurnList definitions for pathfinding
#define TURN_NODE_SIZE 0x80 // 128 bytes per TurnNode
#define TURN_LIST_CAPACITY 32 // Inferred from 0xff0 / 0x80, assuming 0xff0 is a typo for 0x1000
#define TURN_LIST_DATA_BUFFER_SIZE (TURN_LIST_CAPACITY * TURN_NODE_SIZE) // 0x1000 (4096 bytes)
#define TURN_LIST_COUNT_OFFSET_IN_BUFFER TURN_LIST_DATA_BUFFER_SIZE // The count is stored immediately after the node array

typedef struct TurnNode {
    struct RoadData *current_road_data; // 0x00: Pointer to the RoadData for this turn
    struct TurnNode *prev_turn_node;    // 0x04: Pointer to the previous TurnNode in the path
    char padding[TURN_NODE_SIZE - 8];   // 0x08 to 0x7F: Padding to make node 128 bytes
} TurnNode;


// External function declarations (placeholders for a hypothetical library)
// These functions are not provided in the snippet, so their signatures are inferred from usage.
extern void *allocate(void); // Allocates memory for Map, DoubleListHead, DoubleListNode, TurnList buffer
extern DoubleListHead *create_double_list(void);
extern DoubleListNode *get_last_element_d(DoubleListHead *list);
extern DoubleListNode *insert_double_list_node(DoubleListHead *list, DoubleListNode *prev_node);
extern DoubleListNode *delete_double_list_node(DoubleListHead *list, DoubleListNode *node_to_delete);
extern void *create_single_list(void); // Used in init_turnList, but seems to allocate a buffer

// Helper for CONCAT44 (combining two 32-bit values into a 64-bit value)
static inline unsigned long long CONCAT44(unsigned int high, unsigned int low) {
    return ((unsigned long long)high << 32) | low;
}

// Function: init_map
// Initializes a new map structure.
// param_1: Name of the map (char *map_name)
// Returns: A pointer to the newly initialized Map structure
Map *init_map(char *map_name) {
    Map *new_map = (Map *)allocate();
    if (new_map == NULL) {
        puts("\n**Allocate returned an error.\n");
        exit(1); // Replaced _terminate() with exit(1)
    }

    // Initialize map fields
    strcpy(new_map->name, map_name);
    new_map->name[sizeof(new_map->name) - 1] = '\0'; // Ensure null termination
    new_map->roads = create_double_list();

    return new_map;
}

// Function: add_road
// Adds a new road to a map.
// param_1: Pointer to the Map structure
// param_2: Name of the road (char *road_name)
// param_3: Lower 32 bits of the road length (unsigned int)
// param_4: Upper 32 bits of the road length (unsigned int)
// Returns: A pointer to the RoadData structure of the newly added road
RoadData *add_road(Map *map, char *road_name, unsigned int length_low, unsigned int length_high) {
    DoubleListNode *last_road_node = get_last_element_d(map->roads);
    DoubleListNode *new_road_node = insert_double_list_node(map->roads, last_road_node);

    // Assuming RoadData starts at offset 0xc within the DoubleListNode
    RoadData *new_road_data = (RoadData *)((char *)new_road_node + 0xc);

    // Initialize road data fields
    strcpy(new_road_data->name, road_name);
    new_road_data->name[sizeof(new_road_data->name) - 1] = '\0'; // Ensure null termination
    new_road_data->length = CONCAT44(length_high, length_low);
    new_road_data->addresses = create_double_list();
    new_road_data->intersections = create_double_list();
    new_road_data->self_node_ptr = new_road_node; // Store pointer to its containing node

    return new_road_data;
}

// Function: print_roads
// Prints details of all roads in a map.
// param_1: Pointer to the Map structure
void print_roads(Map *map) {
    RoadData *current_road_data = NULL;
    if (map->roads->first != NULL) {
        // RoadData starts at offset 0xc from the DoubleListNode
        current_road_data = (RoadData *)((char *)map->roads->first + 0xc);
    }

    printf("The map %s contains %d roads.\n", map->name, map->roads->count);

    int road_count = 1;
    while (current_road_data != NULL) {
        printf("(%d) %s is %lf units long\n", road_count, current_road_data->name, current_road_data->length);
        road_count++;

        // Move to the next road: get the next node from self_node_ptr, then add 0xc for RoadData
        DoubleListNode *next_node = current_road_data->self_node_ptr->next;
        if (next_node == NULL) {
            current_road_data = NULL;
        } else {
            current_road_data = (RoadData *)((char *)next_node + 0xc);
        }
    }
}

// Function: add_address
// Adds a new address to a road.
// param_1: Address number
// param_2: Street name (char *street_name)
// param_3: Pointer to the RoadData structure
// param_4 to param_9: Intersection data (pointers to RoadData and distances)
// Returns: A pointer to the newly added AddressData structure
AddressData *add_address(int address_number, char *street_name, RoadData *road,
                         RoadData *intersection_after_road, unsigned int dist_after_low,
                         unsigned int dist_after_high, RoadData *intersection_before_road,
                         unsigned int dist_before_low, unsigned int dist_before_high) {
    AddressData stack_address_data; // Use a stack variable to build the data
    memset(&stack_address_data, 0, sizeof(AddressData)); // bzero(local_1c, 0x4c)

    DoubleListNode *prev_address_node = NULL;
    AddressData *current_address_data = NULL;
    if (road->addresses->first != NULL) {
        current_address_data = (AddressData *)((char *)road->addresses->first + 0xc);
    }

    // Find the correct insertion point (maintaining sorted order by address_number)
    while (current_address_data != NULL) {
        if (address_number < current_address_data->number) {
            current_address_data = NULL; // Found insertion point, break loop
        } else {
            prev_address_node = current_address_data->self_node_ptr;
            DoubleListNode *next_node = current_address_data->self_node_ptr->next;
            if (next_node == NULL) {
                current_address_data = NULL;
            } else {
                current_address_data = (AddressData *)((char *)next_node + 0xc);
            }
        }
    }

    DoubleListNode *new_address_node = insert_double_list_node(road->addresses, prev_address_node);
    if (new_address_node == NULL) {
        puts("**insert_double_list_node failed");
        return NULL;
    }

    // Populate the stack AddressData
    stack_address_data.number = address_number;
    strcpy(stack_address_data.street_name, street_name);
    stack_address_data.street_name[sizeof(stack_address_data.street_name) - 1] = '\0'; // Ensure null termination
    stack_address_data.road_ptr = road;
    stack_address_data.intersection_after = intersection_after_road;
    stack_address_data.distance_after = CONCAT44(dist_after_high, dist_after_low);
    stack_address_data.intersection_before = intersection_before_road;
    stack_address_data.distance_before = CONCAT44(dist_before_high, dist_before_low);
    stack_address_data.self_node_ptr = new_address_node;

    // Copy the prepared data into the newly created node's data section
    AddressData *new_address_data = (AddressData *)((char *)new_address_node + 0xc);
    memcpy(new_address_data, &stack_address_data, sizeof(AddressData));

    return new_address_data;
}

// Function: print_addresses
// Prints details of all addresses on a road.
// param_1: Pointer to the RoadData structure
void print_addresses(RoadData *road) {
    AddressData *current_address_data = NULL;
    if (road->addresses->first != NULL) {
        current_address_data = (AddressData *)((char *)road->addresses->first + 0xc);
    }

    printf("The road %s contains %d addresses.\n", road->name, road->addresses->count);

    int address_count = 1;
    while (current_address_data != NULL) {
        printf("(%d)\t%s\n\t%d %s\n", address_count, current_address_data->street_name,
               current_address_data->number, road->name);
        address_count++;

        // Move to the next address
        DoubleListNode *next_node = current_address_data->self_node_ptr->next;
        if (next_node == NULL) {
            current_address_data = NULL;
        } else {
            current_address_data = (AddressData *)((char *)next_node + 0xc);
        }
    }
}

// Function: add_intersection
// Adds a new intersection to a road.
// param_1: Pointer to the RoadData structure (current road)
// param_2: Pointer to the RoadData structure (intersecting road)
// param_3: Pointer to the IntersectionData structure (previous intersection on this road for ordering)
// param_4 to param_7: Distance components
// Returns: A pointer to the newly added IntersectionData structure
IntersectionData *add_intersection(RoadData *current_road, RoadData *intersecting_road,
                                   IntersectionData *prev_intersection_data, unsigned int dist_from_start_low,
                                   unsigned int dist_from_start_high, unsigned int dist_from_end_low,
                                   unsigned int dist_from_end_high) {
    DoubleListNode *prev_node_for_insert = NULL;
    if (prev_intersection_data != NULL) {
        prev_node_for_insert = prev_intersection_data->self_node_ptr;
    }

    DoubleListNode *new_intersection_node = insert_double_list_node(current_road->intersections, prev_node_for_insert);
    if (new_intersection_node == NULL) {
        puts("Failed to insert node");
        return NULL;
    }

    // IntersectionData starts at offset 0xc within the DoubleListNode
    IntersectionData *new_intersection_data = (IntersectionData *)((char *)new_intersection_node + 0xc);

    // Update pointers for surrounding intersections if they exist
    if (prev_intersection_data != NULL) {
        prev_intersection_data->next_intersection_on_this_road = new_intersection_data;
    }
    DoubleListNode *next_node = new_intersection_node->next;
    if (next_node != NULL) {
        IntersectionData *next_intersection_data = (IntersectionData *)((char *)next_node + 0xc);
        // This line is complex in the original: `*(int *)(*(int *)(iVar1 + 4) + 0x34) = iVar2;`
        // It's setting `next_node->data.next_intersection_on_this_road = new_intersection_data;`
        // This assumes `0x34` is the offset of `next_intersection_on_this_road` within `IntersectionData`.
        // My struct definition has it at `0x34` relative to `IntersectionData` start.
        next_intersection_data->next_intersection_on_this_road = new_intersection_data;
    }

    // Initialize new intersection data fields
    new_intersection_data->road_ptr = current_road;
    new_intersection_data->next_intersection_on_this_road = NULL; // Will be set if another intersection is added after this one
    new_intersection_data->self_node_ptr = new_intersection_node;
    new_intersection_data->road_intersection_ptr = intersecting_road;
    new_intersection_data->distance_from_start = CONCAT44(dist_from_start_high, dist_from_start_low);
    new_intersection_data->distance_from_end = CONCAT44(dist_from_end_high, dist_from_end_low);

    return new_intersection_data;
}

// Function: print_intersections
// Prints details of all intersections on a road.
// param_1: Pointer to the RoadData structure
void print_intersections(RoadData *road) {
    IntersectionData *current_intersection_data = NULL;
    if (road->intersections->first != NULL) {
        current_intersection_data = (IntersectionData *)((char *)road->intersections->first + 0xc);
    }

    printf("The road %s contains %d intersections.\n", road->name, road->intersections->count);

    int intersection_count = 1;
    while (current_intersection_data != NULL) {
        printf("(%d)\t%s intersects %s\n", intersection_count, road->name,
               current_intersection_data->road_intersection_ptr->name);
        intersection_count++;

        // Check and print distances if available
        if (current_intersection_data->distance_from_start != 0) { // Assuming 0 means not set or irrelevant
            printf("\t\t%lf units after the intersection of %s\n", current_intersection_data->distance_from_start,
                   current_intersection_data->next_intersection_on_this_road ?
                   current_intersection_data->next_intersection_on_this_road->road_ptr->name : "N/A");
        }
        if (current_intersection_data->distance_from_end != 0) { // Assuming 0 means not set or irrelevant
            printf("\t\t%lf units before the intersection of %s\n", current_intersection_data->distance_from_end,
                   current_intersection_data->next_intersection_on_this_road ?
                   current_intersection_data->next_intersection_on_this_road->road_ptr->name : "N/A");
        }

        // Move to the next intersection
        DoubleListNode *next_node = current_intersection_data->self_node_ptr->next;
        if (next_node == NULL) {
            current_intersection_data = NULL;
        } else {
            current_intersection_data = (IntersectionData *)((char *)next_node + 0xc);
        }
    }
}

// Function: find_pintersection
// Finds a specific intersection on a road.
// param_1: Pointer to the RoadData structure
// param_2: Pointer to the RoadData structure of the target intersecting road
// param_3: Pointer to an IntersectionData to start searching from (or NULL for beginning)
// Returns: A pointer to the found IntersectionData, or NULL if not found
IntersectionData *find_pintersection(RoadData *road, RoadData *target_intersecting_road,
                                     IntersectionData *start_search_from) {
    if (road->intersections->count == 0) {
        puts("intersection list is empty");
        return NULL;
    }

    IntersectionData *current_intersection_data = start_search_from;
    if (current_intersection_data == NULL) {
        current_intersection_data = (IntersectionData *)((char *)road->intersections->first + 0xc);
    }

    while (current_intersection_data != NULL) {
        if (target_intersecting_road == current_intersection_data->road_ptr ||
            target_intersecting_road == current_intersection_data->road_intersection_ptr) {
            return current_intersection_data;
        }

        DoubleListNode *next_node = current_intersection_data->self_node_ptr->next;
        if (next_node == NULL) {
            current_intersection_data = NULL;
        } else {
            current_intersection_data = (IntersectionData *)((char *)next_node + 0xc);
        }
    }
    return NULL;
}

// Function: find_pintersection_addresses
// Finds an address related to a specific intersection on a road.
// param_1: Pointer to the RoadData structure
// param_2: Pointer to the RoadData structure of the target intersection road
// param_3: Pointer to an AddressData to start searching from (or NULL for beginning)
// Returns: A pointer to the found AddressData, or NULL if not found
AddressData *find_pintersection_addresses(RoadData *road, RoadData *target_intersection_road,
                                          AddressData *start_search_from) {
    AddressData *current_address_data = start_search_from;
    if (current_address_data == NULL) {
        if (road->addresses->count == 0) {
            puts("addresss list is empty");
            return NULL;
        }
        current_address_data = (AddressData *)((char *)road->addresses->first + 0xc);
    }

    while (current_address_data != NULL) {
        if (target_intersection_road == current_address_data->intersection_after ||
            target_intersection_road == current_address_data->intersection_before) {
            return current_address_data;
        }

        DoubleListNode *next_node = current_address_data->self_node_ptr->next;
        if (next_node == NULL) {
            current_address_data = NULL;
        } else {
            current_address_data = (AddressData *)((char *)next_node + 0xc);
        }
    }
    return NULL;
}

// Function: delete_intersection
// Deletes an intersection from a road and updates related pointers.
// param_1: Pointer to the RoadData structure (current road)
// param_2: Pointer to the RoadData structure of the intersecting road to delete
// Returns: A pointer to the deleted IntersectionData, or NULL if not found/failed
IntersectionData *delete_intersection(RoadData *current_road, RoadData *target_intersecting_road) {
    // Iterate and update related intersections
    for (IntersectionData *int_data = find_pintersection(current_road, target_intersecting_road, NULL);
         int_data != NULL;
         int_data = find_pintersection(current_road, target_intersecting_road, int_data)) { // The 4th param 0x11e91 is likely a stack cookie or irrelevant
        if (target_intersecting_road == int_data->road_ptr) { // This intersection's current road is the target
            if (target_intersecting_road->intersections->first == NULL) {
                int_data->road_ptr = NULL;
            } else {
                int_data->road_ptr = (RoadData *)((char *)target_intersecting_road->intersections->first + 0xc);
            }
        } else { // This intersection's intersecting road is the target
            if (target_intersecting_road->intersections->first == NULL) {
                int_data->road_intersection_ptr = NULL;
            } else {
                int_data->road_intersection_ptr = (RoadData *)((char *)target_intersecting_road->intersections->first + 0xc);
            }
        }
    }

    // Iterate and update related addresses
    for (AddressData *addr_data = find_pintersection_addresses(current_road, target_intersecting_road, NULL);
         addr_data != NULL;
         addr_data = find_pintersection_addresses(current_road, target_intersecting_road, addr_data)) {
        if (target_intersecting_road == addr_data->intersection_before) {
            if (target_intersecting_road->intersections->first == NULL) {
                addr_data->intersection_before = NULL;
            } else {
                addr_data->intersection_before = (RoadData *)((char *)target_intersecting_road->intersections->first + 0xc);
            }
        } else { // It's intersection_after
            if (target_intersecting_road->intersections->first == NULL) {
                addr_data->intersection_after = NULL;
            } else {
                addr_data->intersection_after = (RoadData *)((char *)target_intersecting_road->intersections->first + 0xc);
            }
        }
    }

    // Finally, delete the intersection node itself
    DoubleListNode *node_to_delete = target_intersecting_road->intersections->first; // Assuming it's the first one
    DoubleListNode *deleted_node = delete_double_list_node(current_road->intersections, node_to_delete);

    if (deleted_node == NULL) {
        return NULL;
    } else {
        return (IntersectionData *)((char *)deleted_node + 0xc);
    }
}

// Function: is_in_turn_list
// Checks if a road is already in the turn list (pathfinding list).
// param_1: Pointer to the turn list buffer (void *turn_list_buffer)
// param_2: Pointer to the RoadData to check
// Returns: 1 if found, 0 otherwise
int is_in_turn_list(void *turn_list_buffer, RoadData *road_to_check) {
    TurnNode *nodes_array = (TurnNode *)turn_list_buffer;
    int *count_ptr = (int *)((char *)turn_list_buffer + TURN_LIST_COUNT_OFFSET_IN_BUFFER);
    int current_count = *count_ptr;

    for (int i = 0; i < current_count; ++i) {
        if (road_to_check == nodes_array[i].current_road_data) {
            return 1;
        }
    }
    return 0;
}

// Function: init_turnList
// Initializes the turn list (pathfinding buffer).
// Returns: A pointer to the allocated turn list buffer
void *init_turnList(void) {
    // create_single_list and insert_single_list_node calls in original
    // seem to be a placeholder for allocating a contiguous buffer.
    // We'll allocate a buffer of size TURN_LIST_DATA_BUFFER_SIZE + sizeof(int)
    // to hold the nodes and the count.
    void *turn_list_buffer = allocate(); // Assume allocate can handle the size
    if (turn_list_buffer == NULL) {
        puts("Failed to allocate turn list buffer.");
        exit(1);
    }
    // Initialize count to 0 (done by get_route) and zero out the data (done by get_route)
    return turn_list_buffer;
}

// Function: push_to_turn_list
// Adds a new turn (road) to the turn list.
// param_1: Pointer to the turn list buffer (void *turn_list_buffer)
// param_2: Pointer to the RoadData for the current turn
// param_3: Pointer to the previous TurnNode in the path
// Returns: A pointer to the newly added TurnNode, or NULL if list is full
TurnNode *push_to_turn_list(void *turn_list_buffer, RoadData *current_road_data, TurnNode *prev_turn_node) {
    int *count_ptr = (int *)((char *)turn_list_buffer + TURN_LIST_COUNT_OFFSET_IN_BUFFER);

    if (*count_ptr < TURN_LIST_CAPACITY) {
        TurnNode *new_turn_node = (TurnNode *)((char *)turn_list_buffer + (*count_ptr * TURN_NODE_SIZE));
        memset(new_turn_node, 0, TURN_NODE_SIZE); // bzero(__s, 0x80)
        new_turn_node->current_road_data = current_road_data;
        new_turn_node->prev_turn_node = prev_turn_node;
        (*count_ptr)++;
        return new_turn_node;
    } else {
        return NULL; // List is full
    }
}

// Function: get_route
// Finds a route between a start and end road using a breadth-first search.
// param_1: Pointer to the starting RoadData
// param_2: Pointer to a void* that will hold the allocated turn list buffer
// param_3: Pointer to the ending RoadData
// Returns: The turn list buffer if a route is found, or NULL otherwise
void **get_route(RoadData *start_road, void **turn_list_buffer_ptr, RoadData *end_road) {
    void *turn_list_buffer = *turn_list_buffer_ptr;
    int *count_ptr = (int *)((char *)turn_list_buffer + TURN_LIST_COUNT_OFFSET_IN_BUFFER);

    // Initialize turn list: clear buffer and reset count
    *count_ptr = 0;
    memset(turn_list_buffer, 0, TURN_LIST_DATA_BUFFER_SIZE); // bzero(*param_2, 0xff0) -> assuming 0xff0 means TURN_LIST_DATA_BUFFER_SIZE

    // Push the starting road to the turn list
    TurnNode *initial_turn_node = push_to_turn_list(turn_list_buffer, start_road, NULL);
    if (initial_turn_node == NULL) {
        return NULL;
    }

    int current_node_index = 0;
    while (current_node_index < *count_ptr) { // Iterate through the queue of nodes
        TurnNode *current_turn_node = (TurnNode *)((char *)turn_list_buffer + (current_node_index * TURN_NODE_SIZE));
        RoadData *current_road = current_turn_node->current_road_data;

        if (current_road == NULL) {
            return NULL; // Should not happen if push_to_turn_list handles NULLs
        }

        // If current road is the destination, we found a path
        if (current_road == end_road) {
            return turn_list_buffer_ptr;
        }

        // Iterate through intersections of the current road
        IntersectionData *current_intersection_data = NULL;
        if (current_road->intersections->first != NULL) {
            current_intersection_data = (IntersectionData *)((char *)current_road->intersections->first + 0xc);
        }

        while (current_intersection_data != NULL) {
            RoadData *intersecting_road = current_intersection_data->road_intersection_ptr;
            if (intersecting_road != NULL && !is_in_turn_list(turn_list_buffer, intersecting_road)) {
                TurnNode *new_turn_node = push_to_turn_list(turn_list_buffer, intersecting_road, current_turn_node);
                if (new_turn_node == NULL) {
                    return NULL; // Ran out of space in turn list
                }
                if (intersecting_road == end_road) {
                    return turn_list_buffer_ptr;
                }
            }

            // Move to the next intersection on the current road
            DoubleListNode *next_intersection_node = current_intersection_data->self_node_ptr->next;
            if (next_intersection_node == NULL) {
                current_intersection_data = NULL;
            } else {
                current_intersection_data = (IntersectionData *)((char *)next_intersection_node + 0xc);
            }
        }
        current_node_index++; // Move to the next node in the queue
    }

    return NULL; // No route found
}