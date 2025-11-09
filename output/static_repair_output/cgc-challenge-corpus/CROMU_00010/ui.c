#include <stdio.h>   // For printf, puts
#include <string.h>  // For strcpy, strlen, memset
#include <stdlib.h>  // For atof, atoi

// Forward declarations for functions not provided in the snippet.
// These are external functions that must be linked from elsewhere.
// Assuming 'int delimiter' is ASCII value (e.g., 10 for newline).
// 'max_len' is the maximum characters to read (excluding null terminator).
int receive_until(char *buf, int delimiter, int max_len);

// Function parameters for 'map_ptr', 'road_ptr', 'inter_ptr' etc.
// are assumed to be 'void*' as they represent opaque object pointers in the original snippet.
void add_road(void *map_ptr, const char *road_name, double road_length);

// Assuming signature based on usage:
// int add_address(int street_num, const char *resident_name, void *road_ptr,
//                 void *inter1_ptr, double dist1, void *inter2_ptr, double dist2);
// Returns 0 on failure, non-zero on success (likely a pointer to the new address object).
int add_address(int street_num, const char *resident_name, void *road_ptr,
                void *inter1_ptr, double dist1, void *inter2_ptr, double dist2);

// int add_intersection(void *road_ptr_1, void *road_ptr_2, void *prev_inter_ptr,
//                      double dist_to_prev, double dist_to_next);
// Returns 0 on failure, non-zero on success (likely a pointer to the new intersection object).
int add_intersection(void *road_ptr_1, void *road_ptr_2, void *prev_inter_ptr,
                     double dist_to_prev, double dist_to_next);

void delete_intersection(void *road_ptr, void *intersection_ptr);
void print_roads(void *map_ptr); // Assumed param is map_ptr, not road_ptr
void print_intersections(void *road_ptr);
void print_addresses(void *road_ptr);

// Returns a pointer to a route object or NULL.
// 'route_data_ptr' is an opaque pointer, perhaps for internal route management data.
void* get_route(void *map_ptr, void *route_data_ptr, void *start_road_ptr, void *end_road_ptr);


// Function: name_map
void name_map(char *param_1) {
  char name_buf[32]; // Buffer for name input
  
  printf("Name:\n");
  int len = receive_until(name_buf, 10, 0x1f); // 0x1f = 31 chars + null terminator
  if (len < 1) {
    puts("Name not set.");
  } else {
    name_buf[len] = '\0';
    printf("Map %s is now named %s.\n", param_1, name_buf);
    strcpy(param_1, name_buf);
  }
}

// Function: get_positive_float
long double get_positive_float(void) {
  char float_buf[11]; // Buffer for float input (e.g., "123.45\n\0" requires 8 chars + decimal + null)
  
  // receive_until(buffer, delimiter, max_len)
  // The original snippet had `receive_until(local_23,10,10,0x12386);`
  // Assuming delimiter is 10 (newline), max_len is 10.
  float_buf[receive_until(float_buf, 10, 10)] = '\0';
  double val = atof(float_buf);
  return (val > 0.0) ? (long double)val : 0.0L;
}

// Function: get_positive_int
int get_positive_int(void) {
  char int_buf[9]; // Buffer for int input (e.g., "1234567\n\0" requires 8 chars + null)
  
  // The original snippet had `receive_until(local_1d,10,8,0x123e7);`
  // Assuming delimiter is 10 (newline), max_len is 8.
  int_buf[receive_until(int_buf, 10, 8)] = '\0';
  int val = atoi(int_buf);
  return (val < 1) ? 0 : val;
}

// Function: get_char32
void get_char32(char *param_1) {
  char input_buf[32]; // Buffer for char32 input
  
  memset(input_buf, 0, sizeof(input_buf)); // 0x20 = 32 bytes
  // The original snippet had `receive_until(local_30,10,0x1f,uVar2);`
  // Assuming delimiter is 10 (newline), max_len is 0x1f (31 chars).
  input_buf[receive_until(input_buf, 10, 0x1f)] = '\0';
  
  memset(param_1, 0, 0x20); // Clear destination buffer (assuming it's also 32 bytes)
  if (strlen(input_buf) != 0) { // Check if input is not empty
    strcpy(param_1, input_buf);
  }
}

// Function: print_map_info
void print_map_info(void *map_ptr) {
  puts("Map Info");
  
  // Assuming map_ptr + 0x20 points to a RoadList struct, and at offset +4 within that struct is the number of roads.
  void* road_list_ptr = *(void**)((char*)map_ptr + 0x20);
  int num_roads = *(int*)((char*)road_list_ptr + 4);

  // Assuming the map name is at the beginning of the map_ptr object (offset 0)
  printf("Map Name: %s Number of Roads: %d\n", (char*)map_ptr, num_roads);
  puts("Road List:");

  // Assuming RoadList struct at map_ptr + 0x20 has a pointer to the first road node at offset 0.
  // And each road node has a pointer to the next road node at offset +0x10.
  // The actual road data starts at an offset of +0xc from the node pointer.
  void* current_road_node = *(void**)road_list_ptr; // Pointer to the first road node
  if (current_road_node == NULL) {
    puts("End of road list");
  } else {
    while (current_road_node != NULL) {
      void* current_road_data = (char*)current_road_node + 0xc;
      // Road name at current_road_data + 0x14, length at current_road_data + 0x3c (long double)
      printf("%s is %Lf units long\n", (char*)current_road_data + 0x14, *(long double*)((char*)current_road_data + 0x3c));
      puts("IntersectionList:");
      print_intersections(current_road_data); // Pass current road data pointer
      puts("AddressList:");
      print_addresses(current_road_data); // Pass current road data pointer

      // Get pointer to the next road node in the list
      // current_road_node + 0x10 points to a structure containing the next node pointer at offset +4.
      void* next_road_list_entry = *(void**)((char*)current_road_node + 0x10);
      if (next_road_list_entry == NULL || *(int*)((char*)next_road_list_entry + 4) == 0) {
        current_road_node = NULL; // End the loop
      } else {
        current_road_node = *(void**)((char*)next_road_list_entry + 4); // Move to the next road node
      }
    }
    puts("End of road list");
  }
}

// Function: add_new_road
void add_new_road(void *map_ptr) {
  char road_name_buf[32]; // Buffer for road name
  
  // Check road limit (0x32 = 50)
  if (*(int*)((char*)*(void**)((char*)map_ptr + 0x20) + 4) < 0x32) {
    printf("Road Name:\n");
    road_name_buf[receive_until(road_name_buf, 10, 0x1f)] = '\0';
    
    printf("Road Length:\n");
    long double road_length = get_positive_float();
    
    if ((road_length > 0.0L) && (strlen(road_name_buf) != 0)) {
      add_road(map_ptr, road_name_buf, (double)road_length); // Assuming add_road takes a double for length
      return;
    }
    puts("Invalid Road.");
  } else {
    puts("No more roads allowed.(Limit)");
  }
}

// Function: select_road
void* select_road(void *map_ptr) {
  // Assuming map_ptr + 0x20 points to a RoadList struct, and at offset +4 within that struct is the number of roads.
  void* road_list_ptr = *(void**)((char*)map_ptr + 0x20);
  int num_roads = *(int*)((char*)road_list_ptr + 4);

  if (num_roads == 1) {
    puts("Only road selected.");
    // Return the first road's data pointer (first node + 0xc)
    return (char*)*(void**)road_list_ptr + 0xc;
  } else if (num_roads == 0) {
    return NULL;
  } else {
    char choice_buf[3]; // Buffer for user choice
    while (1) { // Loop until a valid selection or exit
      memset(choice_buf, 0, sizeof(choice_buf));
      puts("Select Road by (N)umber, or (L)ist or (0)Exit.");
      printf("choice:\n");
      receive_until(choice_buf, 10, 2); // Read one character
      
      if (choice_buf[0] == 'N') {
        printf("1-%d:\n", num_roads);
        int road_num = get_positive_int();
        if (road_num == 0) {
          puts("Invalid number. (0)");
        } else if (num_roads < road_num) {
          puts("Invalid number. (out of range)");
        } else {
          // Traverse the list to find the Nth road node
          void* current_road_node = *(void**)road_list_ptr; // Start at the first road node
          for (int i = 1; i < road_num; ++i) {
            // current_road_node + 0x1c points to a structure containing the next node pointer at offset +4.
            void* next_road_list_entry = *(void**)((char*)current_road_node + 0x1c);
            current_road_node = *(void**)((char*)next_road_list_entry + 4);
            if (current_road_node == NULL) { // Should not happen if road_num is in range
                return NULL;
            }
          }
          return (char*)current_road_node + 0xc; // Return the data pointer for the selected road
        }
      } else if (choice_buf[0] == '0') {
        return NULL; // Exit
      } else if (choice_buf[0] == 'L') {
        print_roads(map_ptr); // List all roads
      } else {
        puts("Invalid choice.");
      }
    }
  }
}

// Function: select_address
void* select_address(void *road_ptr) {
  // Assuming road_ptr + 0x34 points to an AddressList struct, and at offset +4 within that struct is the number of addresses.
  void* address_list_ptr = *(void**)((char*)road_ptr + 0x34);
  int num_addresses = *(int*)((char*)address_list_ptr + 4);

  if (num_addresses == 1) {
    puts("Only address on road was selected");
    // Return the first address's data pointer (first node + 0xc)
    return (char*)*(void**)address_list_ptr + 0xc;
  } else if (num_addresses == 0) {
    return NULL;
  } else {
    char choice_buf[3]; // Buffer for user choice
    while (1) { // Loop until a valid selection or exit
      memset(choice_buf, 0, sizeof(choice_buf));
      puts("Select address (N)umber (L)ist or (0)Exit: ");
      receive_until(choice_buf, 10, 2); // Read one character
      
      if (choice_buf[0] == 'N') {
        printf("1-%d:\n", num_addresses);
        int address_num = get_positive_int();
        if (address_num == 0) {
          puts("Invalid number. (0)");
          return NULL;
        }
        if (address_num <= num_addresses) {
          // Traverse the list to find the Nth address node
          void* current_address_node = *(void**)address_list_ptr; // Start at the first address node
          for (int i = 1; i < address_num; ++i) {
            // current_address_node + 0x14 points to a structure containing the next node pointer at offset +4.
            void* next_address_list_entry = *(void**)((char*)current_address_node + 0x14);
            current_address_node = *(void**)((char*)next_address_list_entry + 4);
            if (current_address_node == NULL) { // Should not happen if address_num is in range
                return NULL;
            }
          }
          return (char*)current_address_node + 0xc; // Return the data pointer for the selected address
        }
        puts("Invalid number. (out of range)");
        return NULL;
      } else if (choice_buf[0] == '0') {
        return NULL; // Exit
      } else if (choice_buf[0] == 'L') {
        print_addresses(road_ptr); // List all addresses for this road
      } else {
        puts("Invalid choice.");
      }
    }
  }
}

// Function: select_intersection
void* select_intersection(void *road_ptr) {
  // Assuming road_ptr + 0x38 points to an IntersectionList struct, and at offset +4 within that struct is the number of intersections.
  void* intersection_list_ptr = *(void**)((char*)road_ptr + 0x38);
  int num_intersections = *(int*)((char*)intersection_list_ptr + 4);

  if (num_intersections == 0) {
    puts("This road has no intersections to choose.");
    return NULL;
  } else {
    char choice_buf[3]; // Buffer for user choice
    while (1) { // Loop until a valid selection or exit
      memset(choice_buf, 0, sizeof(choice_buf));
      puts("Select intersection by (N)umber, (L)ist, or set to N(O)TDEFINED.");
      printf("choice:\n");
      receive_until(choice_buf, 10, 2); // Read one character
      
      if (choice_buf[0] == 'O') { // NOTDEFINED
        return NULL;
      } else if (choice_buf[0] == 'L') {
        print_intersections(road_ptr); // List all intersections for this road
      } else if (choice_buf[0] == 'N') {
        printf("1-%d:\n", num_intersections);
        int intersection_num = get_positive_int();
        if (intersection_num == 0) {
          puts("Invalid number.");
          return NULL;
        }
        if (intersection_num > num_intersections) {
          puts("Invalid number.");
          return NULL;
        }
        // Traverse the list to find the Nth intersection node
        void* current_intersection_node = *(void**)intersection_list_ptr; // Start at the first intersection node
        for (int i = 1; i < intersection_num; ++i) {
          // current_intersection_node + 0x2c points to a structure containing the next node pointer at offset +4.
          void* next_intersection_list_entry = *(void**)((char*)current_intersection_node + 0x2c);
          current_intersection_node = *(void**)((char*)next_intersection_list_entry + 4);
          if (current_intersection_node == NULL) { // Should not happen if intersection_num is in range
              return NULL;
          }
        }
        return (char*)current_intersection_node + 0xc; // Return the data pointer for the selected intersection
      } else {
        puts("Invalid choice.");
      }
    }
  }
}

// Function: add_new_address
void add_new_address(void *map_ptr) {
  char resident_name_buf[32]; // Buffer for resident name
  
  printf("Street Number: \n");
  int street_num = get_positive_int();
  if (street_num == 0) {
    puts("Invalid number.");
    return;
  }
  
  printf("Resident: \n");
  get_char32(resident_name_buf);
  if (strlen(resident_name_buf) == 0) {
    puts("Invalid resident.");
    return;
  }
  
  void* road_ptr = select_road(map_ptr);
  if (road_ptr == NULL) {
    puts("Invalid road.");
    return;
  }
  
  // Check address limit (0x32 = 50) on the selected road
  if (*(int*)((char*)*(void**)((char*)road_ptr + 0x34) + 4) >= 0x32) {
    puts("Invalid road.(Limit)");
    return;
  }
  
  void* inter1_ptr = select_intersection(road_ptr);
  long double dist1 = 0.0L;
  if (inter1_ptr != NULL) {
    puts("Distance to intersection: ");
    dist1 = get_positive_float();
  }
  
  void* inter2_ptr = select_intersection(road_ptr);
  long double dist2 = 0.0L;
  if (inter2_ptr != NULL) {
    puts("Distance to intersection: ");
    dist2 = get_positive_float();
  }
  
  // Assuming add_address takes doubles for distances and returns 0 on failure.
  if (add_address(street_num, resident_name_buf, road_ptr, inter1_ptr, (double)dist1, inter2_ptr, (double)dist2) == 0) {
    puts("Address not Added");
  }
}

// Function: add_new_intersection
void add_new_intersection(void *map_ptr) {
  void* road1_ptr = select_road(map_ptr);
  void* road2_ptr = select_road(map_ptr);
  
  long double dist_road1_prev = 0.0L;
  long double dist_road1_next = 0.0L;
  long double dist_road2_prev = 0.0L;
  long double dist_road2_next = 0.0L;
  
  if (road1_ptr == NULL || road2_ptr == NULL || road1_ptr == road2_ptr) {
    puts("Invalid intersection.");
    return;
  }
  
  // Check intersection limits (0x32 = 50) on both roads
  if (*(int*)((char*)*(void**)((char*)road1_ptr + 0x38) + 4) >= 0x32 ||
      *(int*)((char*)*(void**)((char*)road2_ptr + 0x38) + 4) >= 0x32) {
    puts("Invalid intersection. (Limit)");
    return;
  }
  
  puts("Prev intersection on road 1.");
  void* prev_inter1_ptr = select_intersection(road1_ptr);
  if (prev_inter1_ptr == NULL) {
    // If no previous intersection, check if there's any intersection at all to ask for 'distance to next'
    // road1_ptr + 0x38 is pointer to the IntersectionList struct. Dereference it to get the head node.
    if (*(void**)((char*)road1_ptr + 0x38) != NULL) {
      puts("Distance to next");
      dist_road1_next = get_positive_float();
    }
  } else {
    puts("Distance to prev");
    dist_road1_prev = get_positive_float();
    // Check if there's a next intersection after prev_inter1_ptr to ask for 'distance to next'
    // prev_inter1_ptr points to the intersection data (+0xc from node). So prev_inter1_ptr - 0xc is the node pointer.
    // Node pointer + 0x20 points to a structure containing the next node pointer at offset +4.
    if (*(void**)((char*)prev_inter1_ptr - 0xc + 0x20) != NULL) {
      puts("Distance to next");
      dist_road1_next = get_positive_float();
    }
  }
  
  // Assuming add_intersection takes doubles for distances and returns 0 on failure.
  // The return value is used as a pointer for delete_intersection, so it must be convertible.
  void* new_inter1_ptr = (void*)(long)add_intersection(road1_ptr, road2_ptr, prev_inter1_ptr,
                                                        (double)dist_road1_prev, (double)dist_road1_next);
  if (new_inter1_ptr == NULL) {
    puts("Intersection not added.");
    return;
  }
  
  puts("Prev intersection on road 2.");
  void* prev_inter2_ptr = select_intersection(road2_ptr);
  if (prev_inter2_ptr == NULL) {
    if (*(void**)((char*)road2_ptr + 0x38) != NULL) {
      puts("Distance to next");
      dist_road2_next = get_positive_float();
    }
  } else {
    puts("Distance to prev");
    dist_road2_prev = get_positive_float();
    if (*(void**)((char*)prev_inter2_ptr - 0xc + 0x20) != NULL) {
      puts("Distance to next");
      dist_road2_next = get_positive_float();
    }
  }
  
  void* new_inter2_ptr = (void*)(long)add_intersection(road2_ptr, road1_ptr, prev_inter2_ptr,
                                                        (double)dist_road2_prev, (double)dist_road2_next);
  if (new_inter2_ptr == NULL) {
    puts("Intersection not added.");
    delete_intersection(road1_ptr, new_inter1_ptr); // Rollback the first intersection
  }
}

// Function: show_route
void show_route(void *map_ptr, void *route_data_ptr) {
  puts("from address ");
  void* start_road = select_road(map_ptr);
  if (start_road == NULL) return;

  void* start_address = select_address(start_road);
  if (start_address == NULL) return;

  puts("to address ");
  void* end_road = select_road(map_ptr);
  if (end_road == NULL) return;

  void* end_address = select_address(end_road);
  if (end_address == NULL) return;

  if (start_road == end_road) {
    puts("Address is on the same road.");
  } else {
    void* route_result = get_route(map_ptr, route_data_ptr, start_road, end_road);
    if (route_result == NULL) {
      puts("No valid route.\nCan't get there from here");
      puts("#############################");
    } else if (*(int*)((char*)route_result + 0xff4) == 0) { // Check route validity / size
      puts("**bad route");
    } else {
      // Re-interpreting the route structure based on the original snippet's access patterns.
      // Assuming `get_route` returns a pointer to the first "route step" node.
      // Each "route step" node is a `void**` (or `int*`) where:
      // `[0]` points to the actual road object data for that step.
      // `[1]` points to the *next* "route step" node, or NULL if it's the last step.
      // The original `piVar4` re-assignment is ignored as it seems to point to the last element,
      // which is unsuitable for starting the print loop.

      void** current_route_node = (void**)route_result; // Points to the first route step node
      
      // The very first road in the route is `current_route_node[0]`.
      // The name is at `current_route_node[0] + 0x14`.
      printf("Starting on %s.\n", (char*)current_route_node[0] + 0x14);
      
      // Now iterate through the *subsequent* steps in the route.
      // The next route node is `current_route_node[1]`.
      current_route_node = (void**)current_route_node[1];
      
      while (current_route_node != NULL) {
        // Each step's road is `current_route_node[0]`.
        // The name is at `current_route_node[0] + 0x14`.
        // The original prints the same string twice, which is likely a copy-paste error.
        printf("head towards %s then turn on to %s\n", (char*)current_route_node[0] + 0x14, (char*)current_route_node[0] + 0x14);
        current_route_node = (void**)current_route_node[1]; // Move to the next route node
      }
      puts("You have arrived at your destination.");
      puts("#############################");
    }
  }
}

// Function: prompt_loop
void prompt_loop(void *map_ptr, void *route_data_ptr) {
  char choice_buf[3]; // Buffer for user choice
  
  while (1) { // Loop indefinitely until '0' is entered
    memset(choice_buf, 0, sizeof(choice_buf));
    printf("choice:\n");
    receive_until(choice_buf, 10, 2); // Read one character
    
    switch(choice_buf[0]) {
    case '0':
      puts("Exiting.");
      return; // Exit the function
    case 'A':
      // Assuming map_ptr itself is a char* to the map name, or the map name is at offset 0.
      name_map((char*)map_ptr);
      break;
    case 'B':
      add_new_road(map_ptr);
      break;
    case 'C':
      add_new_address(map_ptr);
      break;
    case 'D':
      add_new_intersection(map_ptr);
      break;
    case 'E':
      print_map_info(map_ptr);
      break;
    case 'I':
      show_route(map_ptr, route_data_ptr);
      break;
    default:
      puts("Invalid choice.");
      break;
    }
  }
}