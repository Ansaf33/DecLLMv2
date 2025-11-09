#include <stdlib.h> // For malloc, NULL, free
#include <string.h> // For strcpy, strcmp
#include <stdio.h>  // For printf (used in example main for verification)

// Define structs based on offsets and usage inferred from the original code.
// These structs are designed to match the 0x10 (16-byte) allocation size
// and field access offsets (0, 4, 8, 12) if compiled for a 32-bit system
// where pointers are 4 bytes. Using sizeof() makes it portable.
typedef struct AirportConnection {
    char targetCode[4]; // 3-char code + null terminator
    unsigned int distance;
    unsigned int cost;
    struct AirportConnection *next;
} AirportConnection;

typedef struct AirportNode {
    char code[4]; // 3-char code + null terminator
    AirportConnection *connections;
    struct AirportNode *next;
} AirportNode;

// DAT_4347c000 is assumed to be a global array of bytes used as a data source.
// Its content dictates the number of airports, their codes, and connections.
// A sample array is provided to make the code compilable and demonstrate functionality.
// In a real application, this would come from a specific data source.
static unsigned char DAT_4347c000[] = {
    // Index 0: Determines the total number of airports.
    // (DAT_4347c000[0] & 0xf) + 5; For 0x00, this means (0 & 0xf) + 5 = 5 airports.
    0x00,
    // Airport codes (3 bytes each for generation)
    // Airport 0: AAB (0x00, 0x01, 0x01 -> 'A'+0, 'A'+1, 'A'+1)
    0x00, 0x01, 0x01,
    // Airport 1: BCC (0x01, 0x02, 0x02 -> 'A'+1, 'A'+2, 'A'+2)
    0x01, 0x02, 0x02,
    // Airport 2: DFF (0x03, 0x05, 0x05 -> 'A'+3, 'A'+5, 'A'+5)
    0x03, 0x05, 0x05,
    // Airport 3: GII (0x06, 0x08, 0x08 -> 'A'+6, 'A'+8, 'A'+8)
    0x06, 0x08, 0x08,
    // Airport 4: KLL (0x0A, 0x0B, 0x0B -> 'A'+10, 'A'+11, 'A'+11)
    0x0A, 0x0B, 0x0B,

    // Connection data starts after airport codes.
    // The data_idx will be 1 (for initial count byte) + (5 airports * 3 bytes/code) = 16.
    // Connections for Airport 0 (AAB):
    // Number of connections: (DAT_4347c000[16] % (num_airports >> 1)) + 1
    // (0x01 % (5 >> 1)) + 1 = (0x01 % 2) + 1 = 2 connections
    0x01,
    // Connection 1: target index (0-4), distance, cost
    0x01, // Target Airport 1 (BCC)
    0x10, // Distance 16
    0x20, // Cost 32
    // Connection 2:
    0x02, // Target Airport 2 (DFF)
    0x11, // Distance 17
    0x21, // Cost 33

    // Connections for Airport 1 (BCC):
    // Number of connections: (DAT_4347c000[23] % 2) + 1 = (0x00 % 2) + 1 = 1 connection
    0x00,
    // Connection 1:
    0x00, // Target Airport 0 (AAB)
    0x12, // Distance 18
    0x22, // Cost 34

    // Connections for Airport 2 (DFF):
    // Number of connections: (0x01 % 2) + 1 = 2 connections
    0x01,
    0x03, // Target Airport 3 (GII)
    0x13,
    0x23,
    0x04, // Target Airport 4 (KLL)
    0x14,
    0x24,

    // Connections for Airport 3 (GII):
    // Number of connections: (0x00 % 2) + 1 = 1 connection
    0x00,
    0x02, // Target Airport 2 (DFF)
    0x15,
    0x25,

    // Connections for Airport 4 (KLL):
    // Number of connections: (0x01 % 2) + 1 = 2 connections
    0x01,
    0x00, // Target Airport 0 (AAB)
    0x16,
    0x26,
    0x03, // Target Airport 3 (GII)
    0x17,
    0x27,

    // Additional filler data to ensure enough bytes are available for the loops
    // based on potential future expansions or varied data.
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

// Function: makeAirportCode
// Generates a 3-character airport code from 3 bytes of `data_src`.
// The code is null-terminated and stored in `code_buf`.
void makeAirportCode(const unsigned char *data_src, char *code_buf) {
  for (int i = 0; i < 3; ++i) {
    code_buf[i] = data_src[i] % 0x1a + 'A'; // Map byte value to 'A'-'Z'
  }
  code_buf[3] = '\0'; // Null-terminate the string for safety with strcpy/strcmp
}

// Function: check4Code
// Checks if an airport code already exists in the linked list of AirportNodes.
// Returns -1 if the code is found, 0 otherwise.
int check4Code(const AirportNode *head, const char *code_to_check) {
  for (const AirportNode *current = head; current != NULL; current = current->next) {
    if (strcmp(code_to_check, current->code) == 0) {
      return -1; // Code found
    }
  }
  return 0; // Code not found
}

// Function: check4ConnectionCode
// Checks if a connection to a specific target code already exists
// in the linked list of AirportConnections for a given airport.
// Returns the 0-based index of the connection if found, -1 otherwise.
int check4ConnectionCode(const AirportConnection *head, const char *code_to_check) {
  int count = 0;
  for (const AirportConnection *current = head; current != NULL; current = current->next) {
    if (strcmp(code_to_check, current->targetCode) == 0) {
      return count; // Connection found, return its sequential position
    }
    count++;
  }
  return -1; // Connection not found
}

// Function: findAirportCodebyNumber
// Finds an AirportNode by its numerical index in the linked list.
// Returns a pointer to the AirportNode if found, NULL otherwise.
const AirportNode *findAirportCodebyNumber(const AirportNode *head, unsigned int index) {
  unsigned int current_index = 0;
  for (const AirportNode *current = head; current != NULL; current = current->next) {
    if (current_index == index) {
      return current;
    }
    current_index++;
  }
  return NULL; // Airport not found at the given index
}

// Helper function to free all allocated memory in the airport database
void freeDB(AirportNode **airport_list_head) {
    AirportNode *current_airport = *airport_list_head;
    while (current_airport != NULL) {
        AirportConnection *current_connection = current_airport->connections;
        while (current_connection != NULL) {
            AirportConnection *temp_conn = current_connection;
            current_connection = current_connection->next;
            free(temp_conn);
        }
        AirportNode *temp_airport = current_airport;
        current_airport = current_airport->next;
        free(temp_airport);
    }
    *airport_list_head = NULL; // Ensure the head pointer is nulled out
}

// Function: loadDB
// Loads airport and connection data from DAT_4347c000 into a linked list structure.
// `airport_list_head` is a pointer to the head of the AirportNode list.
// Returns 0 on success, -1 on failure (e.g., memory allocation failure or DB already loaded).
int loadDB(AirportNode **airport_list_head) {
  if (*airport_list_head != NULL) {
    return -1; // Database already loaded
  }

  const unsigned char *data_ptr = DAT_4347c000;
  unsigned int data_idx = 0; // Current index into the global data array
  
  // Determine the total number of airports to create.
  // The original code implies DAT_4347c000 itself (as an address) was used,
  // but given the context of `local_20 = &DAT_4347c000` and `local_20[local_10]`,
  // it's interpreted as accessing the first byte of the data array.
  unsigned int num_airports = (data_ptr[data_idx] & 0xf) + 5;
  data_idx++; // Advance past the count byte

  AirportNode *current_airport_node = NULL;
  AirportNode *last_airport_node = NULL; // Helper to easily append to the main airport list

  // First pass: Create airport nodes and assign their codes
  for (unsigned int i = 0; i < num_airports; ++i) {
    AirportNode *new_airport = (AirportNode *)malloc(sizeof(AirportNode));
    if (new_airport == NULL) {
      freeDB(airport_list_head); // Clean up any previously allocated memory
      return -1; // Memory allocation failed
    }
    new_airport->connections = NULL;
    new_airport->next = NULL;

    if (*airport_list_head == NULL) {
      *airport_list_head = new_airport; // Set the head of the main airport list
    } else {
      last_airport_node->next = new_airport; // Link to the previous node
    }
    current_airport_node = new_airport; // Update current node for code assignment
    last_airport_node = new_airport;    // Update last node for next iteration

    char airport_code_buffer[4]; // Buffer to hold the generated airport code
    int code_check_result;
    do {
      makeAirportCode(&data_ptr[data_idx], airport_code_buffer);
      data_idx += 3; // Advance data pointer for the next code generation
      code_check_result = check4Code(*airport_list_head, airport_code_buffer);
    } while (code_check_result == -1); // Regenerate if the code already exists in the list

    strcpy(current_airport_node->code, airport_code_buffer);
  }

  // Second pass: Create connections for each airport
  current_airport_node = *airport_list_head; // Start from the beginning of the airport list
  while (current_airport_node != NULL) {
    // Determine the number of connections for the current airport.
    // num_airports >> 1 will be at least 2 (since num_airports is minimum 5).
    unsigned int num_connections_for_airport = (data_ptr[data_idx] % (num_airports >> 1)) + 1;
    data_idx++;

    AirportConnection *current_connection_node = NULL;
    AirportConnection *last_connection_node = NULL; // Helper to easily append to the connection list

    for (unsigned int j = 0; j < num_connections_for_airport; ++j) {
      AirportConnection *new_connection = (AirportConnection *)malloc(sizeof(AirportConnection));
      if (new_connection == NULL) {
        freeDB(airport_list_head); // Clean up all allocated memory on failure
        return -1; // Memory allocation failed
      }
      new_connection->next = NULL;

      if (current_airport_node->connections == NULL) {
        current_airport_node->connections = new_connection; // Set the head of this airport's connection list
      } else {
        last_connection_node->next = new_connection; // Link to the previous connection node
      }
      current_connection_node = new_connection; // Update current connection node
      last_connection_node = new_connection;    // Update last connection node for next iteration

      unsigned int target_airport_index;
      const AirportNode *target_airport_ptr;
      int connection_check_result;
      
      do {
        target_airport_index = data_ptr[data_idx] % num_airports;
        data_idx++;
        target_airport_ptr = findAirportCodebyNumber(*airport_list_head, target_airport_index);
        
        if (target_airport_ptr == NULL) {
            // This indicates an issue with the data in DAT_4347c000 (invalid target_airport_index)
            // or a logical error. Clean up and return an error.
            freeDB(airport_list_head);
            return -1; 
        }

        connection_check_result = check4ConnectionCode(current_airport_node->connections, target_airport_ptr->code);

      } while (connection_check_result != -1); // Regenerate if connection already exists for this airport

      strcpy(current_connection_node->targetCode, target_airport_ptr->code);
      current_connection_node->distance = data_ptr[data_idx];
      data_idx++;
      current_connection_node->cost = data_ptr[data_idx];
      data_idx++;
    }
    current_airport_node = current_airport_node->next; // Move to the next airport in the main list
  }

  return 0; // Success
}

// Minimal main function to make the code compilable and runnable,
// and to demonstrate the usage of loadDB and freeDB.
int main() {
    AirportNode *airport_db_head = NULL;
    
    printf("Attempting to load database...\n");
    if (loadDB(&airport_db_head) == 0) {
        printf("Database loaded successfully!\n");
        // Example: Print some data to verify
        AirportNode *current_airport = airport_db_head;
        while (current_airport != NULL) {
            printf("Airport: %s\n", current_airport->code);
            AirportConnection *current_conn = current_airport->connections;
            while (current_conn != NULL) {
                printf("  -> Connects to: %s, Distance: %u, Cost: %u\n", 
                       current_conn->targetCode, current_conn->distance, current_conn->cost);
                current_conn = current_conn->next;
            }
            current_airport = current_airport->next;
        }
    } else {
        printf("Failed to load database.\n");
    }

    // Clean up all allocated memory regardless of load success (if head is not NULL)
    freeDB(&airport_db_head);
    printf("Database memory freed.\n");

    return 0;
}