#include <stdio.h>    // For printf, puts
#include <stdlib.h>   // For malloc, free, exit
#include <string.h>   // For strcpy, strlen, memcpy, memset, strcmp
#include <stdbool.h>  // For bool
#include <stddef.h>   // For size_t

// Forward declarations of structs
typedef struct FileDataSystem FileDataSystem;
typedef struct Node Node;
typedef struct Perm Perm;
typedef struct User User;
typedef struct Group Group;
typedef struct UserInGroup UserInGroup;
typedef struct FileData FileData;
typedef struct FileChunk FileChunk;

// Forward declarations of functions
void die(const char *message);
void *mallocOrDie(size_t size, const char *errorMessage);
Perm *add_perm(User *user, Group *group, Node *node);
User *_add_user(const char *name, User *user_list_head);
Group *_add_group(const char *name, Group *group_list_head);
UserInGroup *add_user_to_group(User *user, Group *group);
Node *_add_node(char type, User *owner_user, const char *name, Node *parent_node, User *perm_user);
FileChunk *delete_chunk(FileData *file_data, FileChunk *chunk_to_delete);
Perm *delete_perms(Node *node, Perm *perm_to_delete); // Changed return type to Perm* for next perm
void validate_current_perms(Node *node, FileDataSystem *data);
bool is_user_valid(User *user, FileDataSystem *data);
bool is_group_valid(Group *group, FileDataSystem *data);
UserInGroup *is_user_in_group(User *user, Group *group);
UserInGroup *remove_user_from_group(User *user, Group *group); // Changed return type to UserInGroup* for next in list
char *recursive_path(Node *root_node, Node *current_node);
Node *find_node_by_name(const char *name, Node *start_node);
void delete_file(FileData *file_data);
Node *delete_node(Node *node_to_delete, FileDataSystem *data); // Changed return type to Node* for next sibling
void start_UI(FileDataSystem *data); // External declaration

// Data structures
#define NODE_TYPE_DIR 'D'
#define NODE_TYPE_FILE 'F'

struct FileDataSystem {
    Node *root_node;         // 0x00
    Node *current_node;      // 0x04
    User *current_user;      // 0x08
    Group *current_group;    // 0x0C
    User *root_user;         // 0x10
    Group *root_group;       // 0x14
    int unknown_field;       // 0x18
}; // Size 28 bytes (0x1C)

struct Node {
    Node *next_sibling;  // 0x00
    Node *prev_sibling;  // 0x04
    int type_and_padding;       // 0x08 (char type is at byte 0 of this int)
    User *owner_user;           // 0x0C
    Perm *perm_list_head;       // 0x10
    char *name;                 // 0x14
    FileData *file_data;        // 0x18 (FileData* if type 'F')
    Node *parent;               // 0x1C
    Node *first_child;          // 0x20 (if type 'D')
    Node *last_child;           // 0x24 (if type 'D')
}; // Size 44 bytes (0x2C)

struct Perm {
    Perm *prev;  // 0x00
    Perm *next;  // 0x04
    User *user;  // 0x08
    Group *group; // 0x0C
}; // Size 16 bytes (0x10)

struct User {
    User *prev;  // 0x00
    User *next;  // 0x04
    char *name;  // 0x08
}; // Size 12 bytes (0xC)

struct Group {
    int user_count;             // 0x00
    Group *prev;                // 0x04
    Group *next;                // 0x08
    UserInGroup *first_user_in_group; // 0x0C
    char *name;                 // 0x10
}; // Size 20 bytes (0x14)

struct UserInGroup {
    UserInGroup *prev;   // 0x00
    UserInGroup *next;   // 0x04
    User *user_ptr;      // 0x08
}; // Size 12 bytes (0xC)

struct FileData {
    int chunk_count;        // 0x00
    FileChunk *first_chunk; // 0x04
    FileChunk *last_chunk;  // 0x08
}; // Size 12 bytes (0xC)

struct FileChunk {
    void *data;                 // 0x00
    size_t size;                // 0x04
    FileChunk *prev;            // 0x08
    FileChunk *next;            // 0x0C
}; // Size 16 bytes (0x10)


// Function: die
void die(const char *message) {
  printf("\n******************\n%s\n*****************\n", message);
  exit(1);
}

// Function: mallocOrDie
void * mallocOrDie(size_t size, const char *errorMessage) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    die(errorMessage);
  }
  memset(ptr, 0, size);
  return ptr;
}

// Function: init_data
FileDataSystem * init_data(void) {
  FileDataSystem *data = (FileDataSystem *)mallocOrDie(sizeof(FileDataSystem), "Failed to malloc workingData");
  
  Node *root_node = (Node *)mallocOrDie(sizeof(Node), "Failed to malloc root node");
  data->root_node = root_node;
  
  root_node->next_sibling = NULL;
  root_node->prev_sibling = NULL;
  ((char*)&root_node->type_and_padding)[0] = NODE_TYPE_DIR;
  root_node->owner_user = NULL;
  root_node->perm_list_head = NULL;
  
  char *root_name = (char *)mallocOrDie(5, "Failed to malloc root name");
  strcpy(root_name, "root"); // Assuming "root" is DAT_00018062 and is 4 chars + null
  root_node->name = root_name;
  
  root_node->file_data = NULL;
  root_node->parent = NULL;
  root_node->first_child = NULL;
  root_node->last_child = NULL;

  data->current_node = data->root_node;
  data->unknown_field = 0;

  User *root_user = _add_user("root", NULL); // Assuming DAT_00018062 is "root"
  data->root_user = root_user;
  data->current_user = data->root_user;
  
  Group *root_group = _add_group("root", NULL); // Assuming DAT_00018062 is "root"
  data->root_group = root_group;
  data->current_group = data->root_group;
  
  add_user_to_group(data->root_user, data->root_group);
  
  root_node->perm_list_head = add_perm(data->root_user, NULL, root_node);
  add_perm(NULL, data->root_group, root_node);
  
  return data;
}

// Function: find_perm_by_name
Perm * find_perm_by_name(const char *name, Node *node, FileDataSystem *data) {
  validate_current_perms(node, data);
  Perm *current_perm = node->perm_list_head;
  while(current_perm != NULL) {
    if (current_perm->user != NULL && strcmp(current_perm->user->name, name) == 0) {
      return current_perm;
    }
    if (current_perm->group != NULL && strcmp(current_perm->group->name, name) == 0) {
      return current_perm;
    }
    current_perm = current_perm->next;
  }
  return NULL;
}

// Function: add_perm
Perm * add_perm(User *user, Group *group, Node *node) {
  if ((user == NULL && group == NULL) || (user != NULL && group != NULL)) {
    die("Bad call to add_perm: must specify either user or group, but not both.");
  }
  Perm *new_perm = (Perm *)mallocOrDie(sizeof(Perm), "Failed to malloc Perm");
  new_perm->user = user;
  new_perm->group = group;

  if (node->perm_list_head == NULL) {
    node->perm_list_head = new_perm;
    new_perm->prev = NULL;
    new_perm->next = NULL;
  } else {
    Perm *current_perm = node->perm_list_head;
    Perm *last_perm = NULL;
    while (current_perm != NULL) {
      if ((current_perm->user != NULL && user != NULL && current_perm->user == user) ||
          (current_perm->group != NULL && group != NULL && current_perm->group == group)) {
        free(new_perm); // Perm already exists
        return NULL;
      }
      last_perm = current_perm;
      current_perm = current_perm->next;
    }
    last_perm->next = new_perm;
    new_perm->prev = last_perm;
    new_perm->next = NULL;
  }
  return new_perm;
}

// Function: delete_perms
Perm * delete_perms(Node *node, Perm *perm_to_delete) {
  if (perm_to_delete == NULL) {
    return NULL;
  }
  
  Perm *next_perm = perm_to_delete->next;

  if (perm_to_delete->prev != NULL) {
    perm_to_delete->prev->next = perm_to_delete->next;
  } else {
    node->perm_list_head = perm_to_delete->next;
  }

  if (perm_to_delete->next != NULL) {
    perm_to_delete->next->prev = perm_to_delete->prev;
  }
  
  free(perm_to_delete);
  return next_perm;
}

// Function: validate_current_perms
void validate_current_perms(Node *node, FileDataSystem *data) {
  Perm *current_perm = node->perm_list_head;
  if (current_perm == NULL) {
    return;
  }

  // Check for invalid root perm, this looks like a debugging check
  if (current_perm->user == NULL && current_perm->group == NULL) {
    printf("Bad perm %s\n", node->name);
    // Original code doesn't delete this specific bad perm, just prints.
    // If this is the only perm and it's bad, the list becomes empty.
  }

  while (current_perm != NULL) {
    bool is_valid = true;
    if (current_perm->user != NULL) {
      if (!is_user_valid(current_perm->user, data)) {
        is_valid = false;
      }
    }
    if (current_perm->group != NULL) {
      if (!is_group_valid(current_perm->group, data)) {
        is_valid = false;
      }
    }
    
    if (!is_valid) {
      current_perm = delete_perms(node, current_perm); // delete_perms returns the next item
    } else {
      current_perm = current_perm->next;
    }
  }
  return;
}

// Function: _add_node
Node * _add_node(char type, User *owner_user, const char *name, Node *parent_node, User *perm_user) {
  Node *new_node = (Node *)mallocOrDie(sizeof(Node), "Failed to malloc node");
  
  new_node->next_sibling = NULL;
  new_node->prev_sibling = NULL;
  ((char*)&new_node->type_and_padding)[0] = type;
  new_node->owner_user = owner_user;
  
  new_node->perm_list_head = add_perm(perm_user, NULL, new_node); // Assuming perm_user is for initial permissions
  
  size_t name_len = strlen(name);
  char *node_name = (char *)mallocOrDie(name_len + 1, "Failed to malloc name");
  strcpy(node_name, name);
  new_node->name = node_name;
  
  new_node->file_data = NULL; // Initialized to NULL, will be set for files
  new_node->parent = parent_node;
  new_node->first_child = NULL;
  new_node->last_child = NULL;
  
  if (parent_node->first_child == NULL) {
    parent_node->first_child = new_node;
    parent_node->last_child = new_node;
  } else {
    parent_node->last_child->next_sibling = new_node;
    new_node->prev_sibling = parent_node->last_child;
    parent_node->last_child = new_node;
  }
  return new_node;
}

// Function: get_file_size
int get_file_size(FileData *file_data) {
  int total_size = 0;
  FileChunk *current_chunk = file_data->first_chunk;
  while (current_chunk != NULL) {
    total_size += current_chunk->size;
    current_chunk = current_chunk->next;
  }
  return total_size;
}

// Function: delete_file_bytes
void delete_file_bytes(FileData *file_data, unsigned int bytes_to_delete) {
  FileChunk *current_chunk = file_data->first_chunk;
  FileChunk *target_chunk = NULL;
  unsigned int current_total_size = 0;
  int remaining_bytes_in_target = 0;
  bool found_target = false;

  // Find the chunk where deletion starts and mark subsequent chunks for deletion
  while (current_chunk != NULL) {
    current_total_size += current_chunk->size;
    if (!found_target && bytes_to_delete <= current_total_size) {
      target_chunk = current_chunk;
      found_target = true;
      remaining_bytes_in_target = current_total_size - bytes_to_delete;
    }

    if (found_target && current_chunk != target_chunk) {
      // Delete chunks after the target_chunk
      current_chunk = delete_chunk(file_data, current_chunk);
    } else {
      current_chunk = current_chunk->next;
    }
  }

  if (target_chunk == NULL) {
      // All bytes_to_delete exceeds file size, delete all chunks
      current_chunk = file_data->first_chunk;
      while(current_chunk != NULL) {
          current_chunk = delete_chunk(file_data, current_chunk);
      }
      return;
  }

  // Adjust the target_chunk
  size_t new_size = target_chunk->size - remaining_bytes_in_target;
  void *new_data = mallocOrDie(new_size, "Failed to reallocate filechunk data");
  memcpy(new_data, target_chunk->data, new_size);
  
  free(target_chunk->data);
  target_chunk->data = new_data;
  target_chunk->size = new_size;
}

// Function: add_file_chunk
FileChunk ** add_file_chunk(void *data_buffer, FileData *file_data, size_t size) {
  FileChunk *new_chunk = (FileChunk *)mallocOrDie(sizeof(FileChunk), "Failed to malloc filechunk");
  void *chunk_data = mallocOrDie(size, "Failed to malloc chunk data");
  memcpy(chunk_data, data_buffer, size);
  
  new_chunk->data = chunk_data;
  new_chunk->size = size;
  new_chunk->next = NULL;

  if (file_data->last_chunk == NULL) { // First chunk
    file_data->first_chunk = new_chunk;
    file_data->last_chunk = new_chunk;
    new_chunk->prev = NULL;
    file_data->chunk_count = 1;
  } else {
    file_data->last_chunk->next = new_chunk;
    new_chunk->prev = file_data->last_chunk;
    file_data->last_chunk = new_chunk;
    file_data->chunk_count++;
  }
  return (FileChunk**)&new_chunk; // Return a pointer to the chunk for consistency with original return type
}

// Function: add_file
Node * add_file(Node *parent_node, User *owner_user, size_t file_size, const char *file_name,
               void *file_content, User *perm_user) {
  FileData *new_file_data = (FileData *)mallocOrDie(sizeof(FileData), "Failed to malloc file data");
  
  if (file_size > 0) {
    add_file_chunk(file_content, new_file_data, file_size);
  }
  
  Node *new_file_node = _add_node(NODE_TYPE_FILE, owner_user, file_name, parent_node, perm_user);
  new_file_node->file_data = new_file_data;
  return new_file_node;
}

// Function: delete_chunk
FileChunk * delete_chunk(FileData *file_data, FileChunk *chunk_to_delete) {
  if (chunk_to_delete == NULL) {
    puts("delete_chunk called with NULL");
    return NULL;
  }
  
  FileChunk *next_chunk = chunk_to_delete->next;

  if (chunk_to_delete->prev == NULL) { // First chunk
    file_data->first_chunk = chunk_to_delete->next;
    if (chunk_to_delete->next != NULL) {
      chunk_to_delete->next->prev = NULL;
    } else {
      file_data->last_chunk = NULL; // Only chunk, list is now empty
    }
  } else { // Not the first chunk
    chunk_to_delete->prev->next = chunk_to_delete->next;
    if (chunk_to_delete->next != NULL) {
      chunk_to_delete->next->prev = chunk_to_delete->prev;
    } else {
      file_data->last_chunk = chunk_to_delete->prev; // Last chunk
    }
  }
  
  file_data->chunk_count--;
  free(chunk_to_delete->data);
  free(chunk_to_delete);
  
  return next_chunk;
}

// Function: delete_file
void delete_file(FileData *file_data) {
  FileChunk *current_chunk = file_data->first_chunk;
  while (current_chunk != NULL) {
    current_chunk = delete_chunk(file_data, current_chunk);
  }
  free(file_data);
}

// Function: delete_node
Node * delete_node(Node *node_to_delete, FileDataSystem *data) {
  if (node_to_delete == data->root_node) {
    puts("Can not delete root directory");
    return NULL;
  }
  
  Node *next_node_to_return = node_to_delete->next_sibling;

  if (((char*)&node_to_delete->type_and_padding)[0] == NODE_TYPE_FILE) {
    delete_file(node_to_delete->file_data);
  }
  
  if (((char*)&node_to_delete->type_and_padding)[0] == NODE_TYPE_DIR) {
    Node *current_child = node_to_delete->first_child;
    while (current_child != NULL) {
      current_child = delete_node(current_child, data); // Recursively delete children
    }
  }

  // Unlink node from its parent's child list
  if (node_to_delete->prev_sibling == NULL) { // Is first child
    if (node_to_delete->parent != NULL) {
        node_to_delete->parent->first_child = node_to_delete->next_sibling;
    }
  } else {
    node_to_delete->prev_sibling->next_sibling = node_to_delete->next_sibling;
  }

  if (node_to_delete->next_sibling == NULL) { // Is last child
    if (node_to_delete->parent != NULL) {
        node_to_delete->parent->last_child = node_to_delete->prev_sibling;
    }
  } else {
    node_to_delete->next_sibling->prev_sibling = node_to_delete->prev_sibling;
  }

  // Delete permissions
  Perm *current_perm = node_to_delete->perm_list_head;
  while (current_perm != NULL) {
    current_perm = delete_perms(node_to_delete, current_perm);
  }
  
  free(node_to_delete->name);
  free(node_to_delete);
  
  return next_node_to_return;
}

// Function: add_directory
Node * add_directory(User *owner_user, const char *name, Node *parent_node, User *perm_user) {
  Node *new_dir_node = _add_node(NODE_TYPE_DIR, owner_user, name, parent_node, perm_user);
  new_dir_node->first_child = NULL;
  new_dir_node->last_child = NULL;
  return new_dir_node;
}

// Function: _add_user
User * _add_user(const char *name, User *user_list_head) {
  User *new_user = (User *)mallocOrDie(sizeof(User), "Failed to allocate user");
  
  size_t name_len = strlen(name);
  char *user_name = (char *)mallocOrDie(name_len + 1, "Failed to allocate username");
  strcpy(user_name, name);
  new_user->name = user_name;

  if (user_list_head == NULL) { // First user in list (root user)
    new_user->prev = NULL;
    new_user->next = NULL;
  } else {
    User *current_user = user_list_head;
    User *last_user = NULL;
    while (current_user != NULL) {
      last_user = current_user;
      current_user = current_user->next;
    }
    last_user->next = new_user;
    new_user->prev = last_user;
    new_user->next = NULL;
  }
  return new_user;
}

// Function: add_user
void add_user(const char *name, FileDataSystem *data) {
  _add_user(name, data->root_user);
}

// Function: is_user_in_group
UserInGroup * is_user_in_group(User *user, Group *group) {
  UserInGroup *current_user_in_group = group->first_user_in_group;
  while (current_user_in_group != NULL) {
    if (current_user_in_group->user_ptr == user) {
      return current_user_in_group;
    }
    current_user_in_group = current_user_in_group->next;
  }
  return NULL;
}

// Function: is_user_valid
bool is_user_valid(User *user, FileDataSystem *data) {
  User *current_user = data->root_user;
  while (current_user != NULL) {
    if (current_user == user) {
      return true;
    }
    current_user = current_user->next;
  }
  return false;
}

// Function: is_group_valid
bool is_group_valid(Group *group, FileDataSystem *data) {
  Group *current_group = data->root_group;
  while (current_group != NULL) {
    if (current_group == group) {
      return true;
    }
    current_group = current_group->next;
  }
  return false;
}

// Function: remove_user_from_group
UserInGroup * remove_user_from_group(User *user, Group *group) {
  UserInGroup *user_in_group_to_delete = is_user_in_group(user, group);
  if (user_in_group_to_delete == NULL) {
    return NULL; // User not found in group
  }
  
  UserInGroup *next_user_in_group = user_in_group_to_delete->next;

  if (user_in_group_to_delete->prev == NULL) { // First user in group
    group->first_user_in_group = user_in_group_to_delete->next;
    if (user_in_group_to_delete->next != NULL) {
      user_in_group_to_delete->next->prev = NULL;
    }
  } else { // Not the first user in group
    user_in_group_to_delete->prev->next = user_in_group_to_delete->next;
    if (user_in_group_to_delete->next != NULL) {
      user_in_group_to_delete->next->prev = user_in_group_to_delete->prev;
    }
  }
  
  group->user_count--;
  free(user_in_group_to_delete);
  
  return next_user_in_group;
}

// Function: remove_user
User * remove_user(User *user_to_delete, FileDataSystem *data) {
  if (user_to_delete == NULL) {
    puts("Bad call: remove_user with NULL user");
    return NULL;
  }
  if (user_to_delete == data->root_user) {
    puts("Can not delete root user");
    return NULL;
  }
  
  User *next_user_to_return = user_to_delete->next;

  // Remove user from all groups
  Group *current_group = data->root_group;
  while (current_group != NULL) {
    remove_user_from_group(user_to_delete, current_group);
    current_group = current_group->next;
  }

  // Unlink user from the global user list
  if (user_to_delete->prev != NULL) {
    user_to_delete->prev->next = user_to_delete->next;
  } else {
    // If user_to_delete is the first user, update root_user (this should not happen for non-root)
    // This case should be handled by `user_to_delete == data->root_user` above.
    // If root user is somehow deleted, this would make the user list invalid.
    // However, the original code doesn't adjust data->root_user for this case, implying root_user is never removed.
  }
  if (user_to_delete->next != NULL) {
    user_to_delete->next->prev = user_to_delete->prev;
  }
  
  free(user_to_delete->name);
  free(user_to_delete);
  
  return next_user_to_return;
}

// Function: remove_group
Group * remove_group(Group *group_to_delete, FileDataSystem *data) {
  if (group_to_delete == NULL) {
    puts("Bad call to remove_group with NULL group");
    return NULL;
  }
  if (group_to_delete == data->root_group) {
    puts("Unable to delete group root");
    return NULL;
  }
  
  Group *next_group_to_return = group_to_delete->next;

  // Remove all users from this group
  UserInGroup *current_user_in_group = group_to_delete->first_user_in_group;
  while (current_user_in_group != NULL) {
    current_user_in_group = remove_user_from_group(current_user_in_group->user_ptr, group_to_delete);
  }

  // Unlink group from the global group list
  if (group_to_delete->prev != NULL) {
    group_to_delete->prev->next = group_to_delete->next;
  } else {
    // If group_to_delete is the first group, update root_group (this should not happen for non-root)
    // Similar to remove_user, assuming root_group is never removed.
  }
  if (group_to_delete->next != NULL) {
    group_to_delete->next->prev = group_to_delete->prev;
  }
  
  free(group_to_delete->name);
  free(group_to_delete);
  
  return next_group_to_return;
}

// Function: find_user_by_name
User * find_user_by_name(const char *name, FileDataSystem *data) {
  User *current_user = data->root_user;
  while (current_user != NULL) {
    if (strcmp(name, current_user->name) == 0) {
      return current_user;
    }
    current_user = current_user->next;
  }
  return NULL;
}

// Function: find_user_by_number
User * find_user_by_number(int user_number, FileDataSystem *data) {
  int current_number = 0;
  User *current_user = data->root_user;
  while (current_user != NULL) {
    if (current_number == user_number) {
      return current_user;
    }
    current_number++;
    current_user = current_user->next;
  }
  return NULL;
}

// Function: _add_group
Group * _add_group(const char *name, Group *group_list_head) {
  Group *new_group = (Group *)mallocOrDie(sizeof(Group), "Failed to allocate group");
  
  size_t name_len = strlen(name);
  char *group_name = (char *)mallocOrDie(name_len + 1, "Failed to allocate groupName");
  strcpy(group_name, name);
  new_group->name = group_name;
  
  new_group->user_count = 0;
  new_group->first_user_in_group = NULL;
  new_group->next = NULL; // Initialize next to NULL
  
  if (group_list_head != NULL) {
    Group *current_group = group_list_head;
    Group *last_group = NULL;
    while (current_group != NULL) {
      last_group = current_group;
      current_group = current_group->next;
    }
    last_group->next = new_group;
    new_group->prev = last_group;
  } else {
    new_group->prev = NULL; // First group in list (root group)
  }
  return new_group;
}

// Function: add_group
void add_group(const char *name, FileDataSystem *data) {
  _add_group(name, data->root_group);
}

// Function: add_user_to_group
UserInGroup * add_user_to_group(User *user, Group *group) {
  // Check if user is already in group
  if (is_user_in_group(user, group) != NULL) {
      return NULL; // User already in group
  }

  UserInGroup *new_user_in_group = (UserInGroup *)mallocOrDie(sizeof(UserInGroup), "Failed to allocate newUserInGroup");
  new_user_in_group->user_ptr = user;
  new_user_in_group->next = NULL;

  if (group->first_user_in_group == NULL) {
    group->first_user_in_group = new_user_in_group;
    new_user_in_group->prev = NULL;
  } else {
    UserInGroup *current_user_in_group = group->first_user_in_group;
    UserInGroup *last_user_in_group = NULL;
    while (current_user_in_group != NULL) {
      last_user_in_group = current_user_in_group;
      current_user_in_group = current_user_in_group->next;
    }
    last_user_in_group->next = new_user_in_group;
    new_user_in_group->prev = last_user_in_group;
  }
  group->user_count++;
  return new_user_in_group;
}

// Function: find_group_by_number
Group * find_group_by_number(int group_number, FileDataSystem *data) {
  int current_number = 0;
  Group *current_group = data->root_group;
  while (current_group != NULL) {
    if (current_number == group_number) {
      return current_group;
    }
    current_number++;
    current_group = current_group->next;
  }
  return NULL;
}

// Function: find_group_by_name
Group * find_group_by_name(const char *name, FileDataSystem *data) {
  Group *current_group = data->root_group;
  while (current_group != NULL) {
    if (strcmp(name, current_group->name) == 0) {
      return current_group;
    }
    current_group = current_group->next;
  }
  return NULL;
}

// Function: recursive_path
char * recursive_path(Node *root_node, Node *current_node) {
  if (current_node == root_node) {
    size_t name_len = strlen(current_node->name);
    char *path = (char *)mallocOrDie(name_len + 2, "Failed to allocate endName"); // +2 for '/' and null
    strcpy(path, current_node->name);
    return path;
  } else if (current_node->parent == NULL) {
    return NULL; // Should not happen for a node under root, but handle it
  } else {
    char *parent_path = recursive_path(root_node, current_node->parent);
    if (parent_path == NULL) {
      return NULL;
    } else {
      size_t parent_path_len = strlen(parent_path);
      size_t current_name_len = strlen(current_node->name);
      char *full_path = (char *)mallocOrDie(parent_path_len + current_name_len + 2, "Failed to allocate retpath");
      strcpy(full_path, parent_path);
      strcat(full_path, "/");
      strcat(full_path, current_node->name);
      free(parent_path);
      return full_path;
    }
  }
}

// Function: str_of_path
void str_of_path(char *dest_buffer, FileDataSystem *data, Node *target_node) {
  char *path_str = recursive_path(data->root_node, target_node);
  if (path_str != NULL) {
    strcpy(dest_buffer, path_str);
    free(path_str);
  } else {
    dest_buffer[0] = '\0'; // Ensure buffer is empty if path generation fails
  }
}

// Function: find_node_by_name
Node * find_node_by_name(const char *name, Node *start_node) {
  Node *current_node = start_node;
  while (current_node != NULL) {
    if (strcmp(name, current_node->name) == 0) {
      return current_node;
    }
    current_node = current_node->next_sibling;
  }
  return NULL;
}

// Function: find_directory_by_name
Node * find_directory_by_name(const char *name, Node *current_parent_node) {
  Node *current_node = find_node_by_name(name, current_parent_node->first_child);
  while (current_node != NULL) {
    if (((char*)&current_node->type_and_padding)[0] == NODE_TYPE_DIR) {
      return current_node;
    }
    current_node = find_node_by_name(name, current_node->next_sibling);
  }
  return NULL;
}

// Function: find_file_by_name
FileData * find_file_by_name(const char *name, Node *current_parent_node) {
  Node *current_node = find_node_by_name(name, current_parent_node->first_child);
  while (current_node != NULL) {
    if (((char*)&current_node->type_and_padding)[0] == NODE_TYPE_FILE) {
      return current_node->file_data;
    }
    current_node = find_node_by_name(name, current_node->next_sibling);
  }
  return NULL;
}

// Function: find_file_node_by_name
Node * find_file_node_by_name(const char *name, Node *current_parent_node) {
  Node *current_node = find_node_by_name(name, current_parent_node->first_child);
  while (current_node != NULL) {
    if (((char*)&current_node->type_and_padding)[0] == NODE_TYPE_FILE) {
      return current_node;
    }
    current_node = find_node_by_name(name, current_node->next_sibling);
  }
  return NULL;
}

// Function: main
int main(void) {
  FileDataSystem *data = init_data();
  start_UI(data);
  return 0;
}