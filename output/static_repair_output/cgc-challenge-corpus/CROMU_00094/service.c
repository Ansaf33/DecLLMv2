#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>    // For srand, rand
#include <stdbool.h> // For bool

// --- Global variables ---
int adminPortOffset = 0;

// Dummy global data from decompiler
const char DAT_0001506d[] = ",";  // Likely a separator
const char DAT_0001506f[] = "\n"; // Likely a newline
const char DAT_000150cf[] = "%hu"; // Format string for sprintf

// Dummy array for admin port mapping
unsigned short DAT_4347c000[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99
}; // Enough elements for adminPort values 0-99 (0x1000 is 4096, but this array is small)

// Forward declarations of structs
typedef struct List List;
typedef struct ListNode ListNode;
typedef struct Server Server;
typedef struct Instance Instance;

// --- Type Definitions for internal structures ---
typedef struct ListNode {
    void* data;
    uint32_t _padding[2]; // Padding to match decompiler's `node[3]` for `next` (if sizeof(void*) == sizeof(uint32_t))
    struct ListNode* next;
} ListNode;

typedef struct List {
    int type; // From NewList(type)
    ListNode* head;
} List;

typedef struct Server {
    char* id; // Result of GenerateRandomString
    List* instances; // List of Instance*
} Server;

typedef struct Instance {
    char* id; // Result of GenerateRandomString
    Server* parentServer; // Set by AddInstanceToServer
    uint32_t randomNumber; // Result of GenerateRandomNumber
    uint32_t adminPort; // Result of GetNextAdminPort
    uint32_t _padding[3]; // Total size 0x1c (28 bytes). 4+4+4+4 = 16. 28-16=12. So 3*4 bytes.
} Instance;

List* serverList; // Global list of servers

// --- Dummy function prototypes ---
void InitializeRandomness(void);
char* GenerateRandomString(uint32_t seed);
int GenerateRandomNumber(int min, int max);
List* NewList(int type);
void AddToList(List* list, void* data, int type, ...); // Variadic for the magic number
int ReceiveUntil(void);
void* ParseQuery(void); // Returns a pointer to a Query object
uint32_t GenerateBlankResponse(void); // Returns a response handle/ID
void AddToResponse(uint32_t response_handle, const void* data);
void SendResponse(uint32_t response_handle);
void DestroyQuery(void* query);
void DestroyResponse(uint32_t response_handle);
void _terminate(void); // Maps to exit(1)

// --- Function: GetNextAdminPort ---
int GetNextAdminPort(void) {
  adminPortOffset = (adminPortOffset + 4) % 0x1000;
  return adminPortOffset;
}

// --- Function: CreateServer ---
Server* CreateServer(uint32_t param_1) {
  Server* server = (Server*)calloc(1, sizeof(Server));
  if (server == NULL) {
      perror("calloc failed in CreateServer");
      _terminate();
  }
  server->id = GenerateRandomString(param_1);
  server->instances = NewList(2);
  return server;
}

// --- Function: CreateInstance ---
Instance* CreateInstance(uint32_t param_1) {
  Instance* instance = (Instance*)calloc(1, sizeof(Instance));
  if (instance == NULL) {
      perror("calloc failed in CreateInstance");
      _terminate();
  }
  instance->id = GenerateRandomString(param_1);
  instance->randomNumber = GenerateRandomNumber(0,0xffff);
  instance->adminPort = GetNextAdminPort();
  return instance;
}

// --- Function: AddInstanceToServer ---
void AddInstanceToServer(Server* param_1, Instance* param_2) {
  AddToList(param_1->instances, (void*)param_2, 2, 0x119df); // 0x119df is a magic number
  param_2->parentServer = param_1;
}

// --- Function: FindServer ---
char** FindServer(char *param_1) {
  ListNode* current_node = serverList->head;
  while (current_node != NULL) {
    Server* s = (Server*)current_node->data;
    if (s != NULL) {
      if (strcmp(s->id, param_1) == 0) {
        return (char**)s; // Return Server* cast to char**
      }
    }
    current_node = current_node->next;
  }
  return NULL;
}

// --- Function: FindInstance ---
char** FindInstance(char *param_1) {
  ListNode* current_server_node = serverList->head;
  while (current_server_node != NULL) {
    Server* s = (Server*)current_server_node->data;
    if (s != NULL) {
      ListNode* current_instance_node = s->instances->head;
      while (current_instance_node != NULL) {
        Instance* inst = (Instance*)current_instance_node->data;
        if (inst != NULL) {
          if (strcmp(inst->id, param_1) == 0) {
            return (char**)inst; // Return Instance* cast to char**
          }
        }
        current_instance_node = current_instance_node->next;
      }
    }
    current_server_node = current_server_node->next;
  }
  return NULL;
}

// --- Function: InitializeSimulation ---
void InitializeSimulation(void) {
  adminPortOffset = 0;
  serverList = NewList(1);
  int num_servers = GenerateRandomNumber(10,0x20); // 10 to 32
  int server_seed_base = GenerateRandomNumber(0x22,0x40); // 34 to 64

  for (int i = 0; i < num_servers; ++i) {
    Server* new_server = CreateServer(server_seed_base - i);
    AddToList(serverList, new_server, 1, 0); // No magic number for server list

    int num_instances = GenerateRandomNumber(1,0xf); // 1 to 15
    int instance_seed_base = GenerateRandomNumber(0x20,0x40); // 32 to 64
    for (int j = 0; j < num_instances; ++j) {
      Instance* new_instance = CreateInstance(instance_seed_base - j);
      AddInstanceToServer(new_server, new_instance);
    }
  }
}

// --- Function: QueryOne ---
void QueryOne(void* query_ptr, uint32_t response_handle) { // query_ptr is a Query*, response_handle is response handle
  char instance_name_buffer[64];
  
  printf("Query One\n");
  // Assuming Query structure is { uint8_t type; char* param_string_ptr; }
  // *(char **)(query_ptr + 4) gets the string parameter from the Query object
  char* query_string = *((char**)((uint8_t*)query_ptr + 4));
  strncpy(instance_name_buffer, query_string, sizeof(instance_name_buffer) - 1);
  instance_name_buffer[sizeof(instance_name_buffer) - 1] = '\0';

  Instance* found_instance = (Instance*)FindInstance(instance_name_buffer);
  if (found_instance == NULL) {
    printf("Not found\n");
  } else {
    AddToResponse(response_handle, "instance:");
    AddToResponse(response_handle, found_instance->id);
    AddToResponse(response_handle, DAT_0001506d); // Separator
    AddToResponse(response_handle, DAT_0001506f); // Newline/another separator

    char num_buffer[12];
    sprintf(num_buffer, "%d", found_instance->randomNumber); // Fixed "$d" to "%d"
    AddToResponse(response_handle, num_buffer);
    AddToResponse(response_handle, DAT_0001506d); // Separator

    AddToResponse(response_handle, "server:");
    AddToResponse(response_handle, found_instance->parentServer->id);
    AddToResponse(response_handle, DAT_0001506d); // Separator
  }
}

// --- Function: main ---
uint32_t main(void) {
  InitializeRandomness();
  InitializeSimulation();

  while (true) {
    int received_bytes;
    do {
      received_bytes = ReceiveUntil();
      if (received_bytes < 0) {
        return 0; // Error, terminate main
      }
    } while (received_bytes == 0); // Keep trying if 0 bytes received

    void* query_ptr = ParseQuery(); // query_ptr is a pointer to a Query object
    uint32_t response_handle = GenerateBlankResponse();
    uint8_t query_type = *(uint8_t*)query_ptr;

    if (query_type == 4) { // Query Admin
      printf("Query Admin\n");

      // Original snippet interpretation implies:
      // (query_ptr + 4) is a pointer to a string.
      // The first character of this string determines a sub-command.
      // The rest of the string (from char 1 onwards) is the parameter (instance name).
      char* admin_command_string_ptr = *(char**)((uint8_t*)query_ptr + 4);
      uint8_t first_char_of_command = admin_command_string_ptr[0];

      if (first_char_of_command != '1') { // Assuming '1' means the valid sub-command
        printf("Invalid Query\n");
        _terminate();
      }

      char instance_name_buffer[64];
      strncpy(instance_name_buffer, admin_command_string_ptr + 1, sizeof(instance_name_buffer) - 1);
      instance_name_buffer[sizeof(instance_name_buffer) - 1] = '\0';

      Instance* found_instance = (Instance*)FindInstance(instance_name_buffer);
      if (found_instance == NULL) {
        printf("Instance Not Found\n");
        _terminate();
      }

      AddToResponse(response_handle, "admin:");
      char port_buffer[12];
      memset(port_buffer, 0, sizeof(port_buffer));
      // DAT_4347c000 is a dummy array.
      // The index `found_instance->adminPort` must be within its bounds.
      unsigned short admin_port_value = 0;
      if (found_instance->adminPort < sizeof(DAT_4347c000)/sizeof(DAT_4347c000[0])) {
          admin_port_value = DAT_4347c000[found_instance->adminPort];
      } else {
          // Handle out-of-bounds access if adminPortOffset grows too large
          admin_port_value = found_instance->adminPort; // Fallback to raw value
      }
      
      sprintf(port_buffer, DAT_000150cf, admin_port_value);
      AddToResponse(response_handle, port_buffer);
      AddToResponse(response_handle, DAT_0001506d); // Separator

      break; // Exit loop after Admin Query to proceed to common cleanup
    }
    else if (query_type == 3) { // Query One
      QueryOne(query_ptr, response_handle);
    }
    else if (query_type == 1) { // Query All
      printf("Query All\n");
      ListNode* current_server_node = serverList->head;
      while (current_server_node != NULL) {
        Server* s = (Server*)current_server_node->data;
        if (s != NULL) {
          AddToResponse(response_handle, "server:");
          AddToResponse(response_handle, s->id);
          AddToResponse(response_handle, DAT_0001506d);
        }
        current_server_node = current_server_node->next;
      }
    }
    else if (query_type == 2) { // Query Server
      printf("Query Server\n");
      // Assuming Query object structure: { uint8_t type; char* param_string_ptr; }
      char* server_name = *(char**)((uint8_t*)query_ptr + 4); // String parameter for FindServer
      Server* found_server = (Server*)FindServer(server_name);
      if (found_server != NULL) {
        ListNode* current_instance_node = found_server->instances->head;
        while (current_instance_node != NULL) {
          Instance* inst = (Instance*)current_instance_node->data;
          if (inst != NULL) {
            AddToResponse(response_handle, "instance:");
            AddToResponse(response_handle, inst->id);
            AddToResponse(response_handle, DAT_0001506d);
          }
          current_instance_node = current_instance_node->next;
        }
      }
    }
    else { // Invalid Query type
      printf("Invalid Query\n");
      _terminate();
    }

    // Common post-query logic
    SendResponse(response_handle);
    DestroyQuery(query_ptr);
    DestroyResponse(response_handle);
  }

  return 0; // Should not be reached if _terminate() is called on error
}

// --- Dummy function implementations ---
static int random_string_counter = 0;
void InitializeRandomness(void) {
    srand(time(NULL));
}

char* GenerateRandomString(uint32_t seed) {
    char* str = (char*)malloc(64); // Allocate enough space
    if (str == NULL) {
        perror("malloc failed in GenerateRandomString");
        _terminate();
    }
    sprintf(str, "String_%u_%d", seed, random_string_counter++);
    return str;
}

int GenerateRandomNumber(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    return min + (rand() % (max - min + 1));
}

List* NewList(int type) {
    List* new_list = (List*)calloc(1, sizeof(List));
    if (new_list == NULL) {
        perror("calloc failed in NewList");
        _terminate();
    }
    new_list->type = type;
    new_list->head = NULL;
    return new_list;
}

void AddToList(List* list, void* data, int type, ...) {
    if (list == NULL) {
        fprintf(stderr, "Error: Attempted to add to a NULL list.\n");
        return;
    }
    ListNode* new_node = (ListNode*)calloc(1, sizeof(ListNode));
    if (new_node == NULL) {
        perror("calloc failed in AddToList");
        _terminate();
    }
    new_node->data = data;
    new_node->next = list->head;
    list->head = new_node;
}

// Dummy Query structure for ParseQuery
typedef struct Query {
    uint8_t type; // 1: All, 2: Server, 3: One, 4: Admin
    uint8_t _padding[3]; // Align to 4 bytes
    char* param_string_ptr; // Used by QueryOne, QueryServer, QueryAdmin
} Query;

static int query_state = 0; // For generating different dummy queries

int ReceiveUntil(void) {
    // Simulate receiving some data. Always return 1 for success in this dummy.
    return 1;
}

void* ParseQuery(void) {
    Query* q = (Query*)calloc(1, sizeof(Query));
    if (q == NULL) {
        perror("calloc failed in ParseQuery");
        _terminate();
    }

    // Cycle through query types for testing
    switch (query_state % 5) { // 0,1,2,3,4
        case 0: q->type = 1; printf("Dummy Query: All\n"); break;
        case 1: q->type = 2; q->param_string_ptr = GenerateRandomString(rand() % 100); printf("Dummy Query: Server %s\n", q->param_string_ptr); break;
        case 2: q->type = 3; q->param_string_ptr = GenerateRandomString(rand() % 100); printf("Dummy Query: One %s\n", q->param_string_ptr); break;
        case 3: q->type = 4;
                char* admin_str = (char*)malloc(64);
                if (admin_str == NULL) { perror("malloc failed"); _terminate(); }
                sprintf(admin_str, "1Instance_%u_%d", rand() % 100, random_string_counter++);
                q->param_string_ptr = admin_str;
                printf("Dummy Query: Admin sub 1, instance %s\n", admin_str + 1);
                break;
        case 4: q->type = 99; printf("Dummy Query: Invalid (type 99)\n"); break; // Test invalid query
    }
    query_state++;
    return q;
}

uint32_t GenerateBlankResponse(void) {
    // In a real system, this would allocate/initialize a response object and return a handle.
    // For dummy, just return a fixed value.
    return 0xDEADBEEF;
}

void AddToResponse(uint32_t response_handle, const void* data) {
    // In a real system, this would append data to the response object.
    // For dummy, just print it.
    if (data == NULL) {
        printf("[Response: (NULL)]");
    } else if (data == DAT_0001506d) {
        printf("%s", (char*)data);
    } else if (data == DAT_0001506f) {
        printf("%s", (char*)data);
    } else {
        // Assume it's a string if not special
        printf("[Response: %s]", (char*)data);
    }
}

void SendResponse(uint32_t response_handle) {
    // In a real system, this would send the accumulated response.
    printf("\n--- Sending Response (Handle: 0x%X) ---\n", response_handle);
}

void DestroyQuery(void* query) {
    Query* q = (Query*)query;
    if (q) {
        if (q->param_string_ptr) {
            free(q->param_string_ptr);
        }
        free(q);
    }
}

void DestroyResponse(uint32_t response_handle) {
    // In a real system, this would free resources associated with the response_handle.
    // For dummy, nothing specific to free for a fixed handle.
}

void _terminate(void) {
    printf("Terminating program.\n");
    exit(EXIT_FAILURE);
}