#include <stdbool.h> // For bool
#include <stddef.h>  // For size_t
#include <stdlib.h>  // For exit, malloc, free, rand
#include <stdio.h>   // For snprintf
#include <string.h>  // For strlen, memset

// --- External / Placeholder Function Declarations ---
// Assuming g_sockfd is a global socket file descriptor
int g_sockfd = 1; // Placeholder: standard output for simplicity, or 0 for stdin

// Original _terminate function
void _terminate(int exit_code) {
    exit(exit_code);
}

// These functions are not provided, so declare them as external.
// Their signatures are inferred from usage.
// recv_all: Reads exactly 'len' bytes into 'buf'. Returns 'len' on success, -1 on error.
int recv_all(void *buf, size_t len);

// list_create: Creates a new list structure. Returns a pointer to the list.
void *list_create(void);

// node_create: Creates a new list node, wrapping the provided 'data'. Returns a pointer to the node.
void *node_create(void *data);

// list_push: Pushes a node onto the given list.
void list_push(void *list, void *node);

// recvline: Reads a line from a file descriptor 'fd' into 'buf' up to 'max_len'.
// Returns number of bytes read (excluding null terminator) on success, -1 on error.
ssize_t recvline(int fd, char *buf, size_t max_len);

// send: Standard send function. Using 0 for flags as specific values are unclear.
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

// --- Global Data Structures ---
// haiku_data_t: Represents the actual haiku content.
typedef struct haiku_data {
    unsigned int id;
    unsigned short len; // Length of the haiku_text, excluding null terminator
    char text[];        // Flexible array member for the haiku text
} haiku_data_t;

// haiku_node_t: Represents a node in the linked list of haikus.
typedef struct haiku_node {
    haiku_data_t *data;      // Points to haiku_data_t
    struct haiku_node *next; // Next node in the list
} haiku_node_t;

// list_t: Represents the haiku list itself.
// Offsets are assumed for 32-bit architecture based on decompiler output.
typedef struct list {
    haiku_node_t *head;      // At offset 0
    haiku_node_t *tail;      // At offset 4
    unsigned int count;      // At offset 8
} list_t;

list_t *haiku_list = NULL;
unsigned int next_haiku_id = 1; // Start IDs from 1

// DAT_00013000 is a global string for the easter egg haiku
const char DAT_00013000[] = " from within HIGHCOO\a new Haiku every login\a a warm thought engulfs\a";

// --- Error Codes (from decompiler output) ---
#define ERR_RECV_FAILED         -903  // 0xfffffc71
#define ERR_LIST_NOT_EXISTS     -2222 // 0xfffff752
#define ERR_HAIKU_PTR_NULL      -2345 // 0xfffff6d7
#define ERR_RAND_FAILED         -912  // 0xfffffc70
#define ERR_LIST_EMPTY          -1100 // 0xfffffba9
#define ERR_HAIKU_NOT_FOUND     -3456 // 0xfffff280
#define ERR_RECV_LINE_FAILED    -4568 // -0x11d8
#define ERR_HAIKU_LEN_INVALID   -4567 // 0xffffee29
#define ERR_ALLOC_HAIKU_FAILED  -4568 // 0xffffee28 (Same as RECV_LINE_FAILED)

// Function: recv_uint32
unsigned int recv_uint32(void) {
  unsigned int val = 0;
  if (recv_all(&val, sizeof(val)) != sizeof(val)) {
    _terminate(ERR_RECV_FAILED);
  }
  return val;
}

// Function: recv_uint16
unsigned short recv_uint16(void) {
  unsigned short val = 0;
  if (recv_all(&val, sizeof(val)) != sizeof(val)) {
    _terminate(ERR_RECV_FAILED);
  }
  return val;
}

// Function: haiku_list_exists
bool haiku_list_exists(void) {
  return haiku_list != NULL;
}

// Function: is_haiku_list_empty
bool is_haiku_list_empty(void) {
  // Assumes count is at offset 8 in list_t structure
  return haiku_list_exists() ? (((list_t *)haiku_list)->count == 0) : true;
}

// Function: init_haiku_list
void init_haiku_list(void) {
  if (!haiku_list_exists()) {
    haiku_list = (list_t *)list_create();
  }
}

// Function: get_count
unsigned int get_count(void) {
  // Assumes count is at offset 8 in list_t structure
  return haiku_list_exists() ? ((list_t *)haiku_list)->count : ERR_LIST_NOT_EXISTS;
}

// Function: get_id_from_haiku
unsigned int get_id_from_haiku(const haiku_data_t *haiku_data_ptr) {
  if (haiku_data_ptr == NULL) {
    return ERR_HAIKU_PTR_NULL;
  }
  return haiku_data_ptr->id;
}

// Function: get_next_haiku_id
unsigned int get_next_haiku_id(void) {
  return next_haiku_id++;
}

// Function: get_random_idx
int get_random_idx(unsigned int *out_idx) {
  unsigned int count = get_count();
  if ((int)count == ERR_LIST_NOT_EXISTS) {
    return ERR_LIST_NOT_EXISTS;
  }
  if (count == 0) {
    return ERR_LIST_EMPTY;
  }
  
  // The original logic was: if rand() == 0, set index to 0 and return 0. Else return an error.
  // This is highly unusual for a random index function.
  // Sticking to the literal behavior of the snippet for strict adherence to "fixed source code".
  if (rand() == 0) {
    *out_idx = 0; // 0 % count is always 0
    return 0;
  }
  return ERR_RAND_FAILED;
}

// Function: populate_array_with_haiku_ids
int populate_array_with_haiku_ids(unsigned int *id_array, unsigned int max_ids) {
  if (!haiku_list_exists()) {
    return ERR_LIST_NOT_EXISTS;
  }
  
  // Assuming list_t structure: head at offset 0, tail at offset 4
  // The original snippet used haiku_list + 4 for the start of iteration, implying tail.
  // It then used node_ptr + 8 for the next pointer in node.
  // This is inconsistent with other functions using node_ptr + 4 for next.
  // Standardizing to iterate from head and node_ptr->next (which implies offset 4 in haiku_node_t).
  haiku_node_t *current_node = ((list_t *)haiku_list)->head;
  
  if (max_ids == 0 || current_node == NULL) {
    return ERR_LIST_EMPTY;
  }
  
  unsigned int current_idx = 0;
  while (current_idx < max_ids && current_node != NULL) {
    // Assuming haiku ID is the first unsigned int in the haiku_data_t structure
    id_array[current_idx] = get_id_from_haiku(current_node->data);
    current_node = current_node->next; // Assumed next pointer is at offset 4 in haiku_node_t
    current_idx++;
  }
  return 0;
}

// Function: find_haiku_with_id
int find_haiku_with_id(haiku_data_t **out_haiku_data_ptr, unsigned int target_id) {
  int count_result = get_count();
  if (count_result < 1) {
    return (count_result == 0) ? ERR_LIST_EMPTY : ERR_LIST_NOT_EXISTS;
  }
  
  bool found = false;
  // Assuming list_t structure: head at offset 0
  haiku_node_t *current_node = ((list_t *)haiku_list)->head;
  
  while (current_node != NULL) {
    // Assuming haiku ID is the first unsigned int in the haiku_data_t structure
    if (target_id == get_id_from_haiku(current_node->data)) {
      *out_haiku_data_ptr = current_node->data;
      found = true;
      break;
    }
    current_node = current_node->next; // Assumed next pointer is at offset 4 in haiku_node_t
  }
  return found ? 0 : ERR_HAIKU_NOT_FOUND;
}

// Function: send_haiku
void send_haiku(const haiku_data_t *haiku_data) {
  // Assuming 0 for flags
  send(g_sockfd, &haiku_data->id, sizeof(haiku_data->id), 0);
  send(g_sockfd, &haiku_data->len, sizeof(haiku_data->len), 0);
  send(g_sockfd, haiku_data->text, haiku_data->len, 0);
}

// Function: send_easter_egg_haiku
void send_easter_egg_haiku(void) {
  unsigned int id = 0x7a69; // 31337
  const char *text = DAT_00013000;
  unsigned short len = (unsigned short)strlen(text);
  
  // Assuming 0 for flags
  send(g_sockfd, &id, sizeof(id), 0);
  send(g_sockfd, &len, sizeof(len), 0);
  send(g_sockfd, text, len, 0);
}

// Function: send_haiku_id
void send_haiku_id(unsigned int id_to_send) {
  // send_haiku_id(*puVar3); in add_haiku implies it takes an ID
  send(g_sockfd, &id_to_send, sizeof(id_to_send), 0);
}

// Function: add_haiku_to_list
void add_haiku_to_list(haiku_data_t *haiku_data) {
  haiku_node_t *new_node = (haiku_node_t *)node_create(haiku_data);
  list_push(haiku_list, new_node);
}

// Function: recv_haiku_line
int recv_haiku_line(char *buffer, unsigned short max_len) {
  // A buffer of 1360 bytes (0x550) was used in original.
  char temp_line_buffer[1360]; // 0x552 - 2 (for local_572 array initial size)
  
  // Original code zeroed out part of the stack. Using memset for clarity.
  memset(temp_line_buffer, 0, sizeof(temp_line_buffer));
  
  // recvline reads into temp_line_buffer, max_len is for buffer size (including null terminator)
  // Assuming recvline returns actual bytes read, excluding null terminator.
  int bytes_received = recvline(g_sockfd, temp_line_buffer, sizeof(temp_line_buffer) - 1);
  if (bytes_received < 0) {
    return ERR_RECV_LINE_FAILED;
  }
  
  // Original format string "~c\a" is non-standard. Assuming it means "%s".
  // max_len + 1 for snprintf size argument to include null terminator.
  return snprintf(buffer, max_len + 1, "%s", temp_line_buffer);
}

// Function: recv_haiku
haiku_data_t *recv_haiku(unsigned short total_haiku_text_len) {
  // Allocate memory for haiku_data_t structure + actual text + null terminator
  haiku_data_t *new_haiku_data = (haiku_data_t *)malloc(sizeof(haiku_data_t) + total_haiku_text_len + 1);
  if (new_haiku_data == NULL) {
    _terminate(5); // Original used magic number 5 for allocate failure
  }
  
  new_haiku_data->id = get_next_haiku_id();
  new_haiku_data->len = total_haiku_text_len;
  new_haiku_data->text[total_haiku_text_len] = '\0'; // Null-terminate the buffer
  
  char *current_text_pos = new_haiku_data->text;
  unsigned short remaining_len = total_haiku_text_len;
  
  while (remaining_len > 0) {
    // recv_haiku_line writes string into current_text_pos, up to remaining_len bytes.
    // It returns the length of the string written (excluding null terminator).
    int bytes_read_for_line = recv_haiku_line(current_text_pos, remaining_len);
    
    if (bytes_read_for_line < 0) {
      free(new_haiku_data);
      return NULL;
    }
    
    current_text_pos += bytes_read_for_line;
    remaining_len -= bytes_read_for_line;
  }
  return new_haiku_data;
}

// Function: recv_haiku_size
unsigned short recv_haiku_size(void) {
  return recv_uint16();
}

// Function: add_haiku
int add_haiku(void) {
  if (!haiku_list_exists()) {
    init_haiku_list();
  }
  
  unsigned short haiku_text_len = recv_haiku_size();
  if (haiku_text_len == 0 || haiku_text_len > 0xff6) { // 0xff6 is 4086
    return ERR_HAIKU_LEN_INVALID;
  }
  
  haiku_data_t *new_haiku_data = recv_haiku(haiku_text_len);
  if (new_haiku_data == NULL) {
    return ERR_ALLOC_HAIKU_FAILED;
  }
  
  add_haiku_to_list(new_haiku_data);
  send_haiku_id(new_haiku_data->id);
  
  return 0; // Success
}

// Function: get_haiku_by_id
int get_haiku_by_id(void) {
  if (!haiku_list_exists()) {
    return ERR_LIST_NOT_EXISTS;
  }
  
  unsigned int target_id = recv_uint32();
  if (target_id == 0x7a69) { // Easter egg ID
    send_easter_egg_haiku();
    return 0;
  }
  
  haiku_data_t *found_haiku_data = NULL;
  int result = find_haiku_with_id(&found_haiku_data, target_id);
  if (result == 0) {
    send_haiku(found_haiku_data);
  }
  return result;
}

// Function: get_haiku_random
int get_haiku_random(void) {
  if (!haiku_list_exists()) {
    return ERR_LIST_NOT_EXISTS;
  }
  
  unsigned int count = get_count();
  if (count == 0) {
    return ERR_LIST_EMPTY;
  }
  
  // Allocate array to store haiku IDs
  unsigned int *haiku_id_array = (unsigned int *)malloc(count * sizeof(unsigned int));
  if (haiku_id_array == NULL) {
    _terminate(5); // Original used magic number 5 for allocate failure
  }
  
  int result = populate_array_with_haiku_ids(haiku_id_array, count);
  if (result != 0) {
    free(haiku_id_array);
    return result;
  }
  
  unsigned int random_idx = 0;
  result = get_random_idx(&random_idx);
  if (result != 0) {
    free(haiku_id_array);
    return result;
  }
  
  unsigned int target_id = haiku_id_array[random_idx];
  haiku_data_t *found_haiku_data = NULL;
  result = find_haiku_with_id(&found_haiku_data, target_id);
  
  free(haiku_id_array); // Deallocate the temporary array
  
  if (result == 0) {
    send_haiku(found_haiku_data);
  }
  return result;
}

// Function: get_haiku_count
int get_haiku_count(void) {
  if (!haiku_list_exists()) {
    return ERR_LIST_NOT_EXISTS;
  }
  
  unsigned int count_val = get_count();
  send(g_sockfd, &count_val, sizeof(count_val), 0);
  return 0;
}

// Function: get_haiku_ids
int get_haiku_ids(void) {
  if (!haiku_list_exists()) {
    return ERR_LIST_NOT_EXISTS;
  }
  
  unsigned int count = get_count();
  send(g_sockfd, &count, sizeof(count), 0);
  
  // Assuming list_t structure: head at offset 0
  haiku_node_t *current_node = ((list_t *)haiku_list)->head;
  for (; current_node != NULL; current_node = current_node->next) {
    unsigned int id_to_send = get_id_from_haiku(current_node->data);
    send(g_sockfd, &id_to_send, sizeof(id_to_send), 0);
  }
  return 0;
}