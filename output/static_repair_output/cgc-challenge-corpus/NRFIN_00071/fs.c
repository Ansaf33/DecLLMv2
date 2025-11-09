#include <stdio.h>    // For printf, NULL (usually)
#include <stdlib.h>   // For calloc, free, exit
#include <string.h>   // For strlen, strcpy, strcat, strtok, strcmp, strncpy
#include <stdbool.h>  // For bool type

// --- Placeholder Definitions for External Dependencies and Decompiled Types ---

// Decompiler-specific type aliases
typedef unsigned int uint;
// `undefined` and `undefined4` are replaced with standard C types.
// `undefined` is likely `char` or `unsigned char`.
// `undefined4` is likely `int` or `unsigned int` (assuming 4 bytes).
// `code` is a function pointer type, which will be explicitly defined.

// Global root directory pointer
typedef struct DirEntry DirEntry;
typedef struct FileEntry FileEntry;
typedef struct Node Node;
typedef struct List List;

DirEntry* root = NULL;

// Function pointer types for list operations
typedef void (*free_data_func)(void*);
typedef bool (*compare_data_func)(void*, void*);
typedef DirEntry* (*get_dir_func)(DirEntry*, char*); // Specific to get_dir's recursive nature

// --- Data Structures (Mocked) ---

// Generic Node structure for the linked list
struct Node {
    void* data; // Pointer to the actual data (DirEntry*, FileEntry*)
    Node* next;
};

// Generic List structure
struct List {
    Node* head;
    free_data_func free_func; // Function to free data when node/list is destroyed
};

// DirEntry structure based on offsets and `calloc` size (0xa8)
struct DirEntry {
    char name[0x80]; // Name of the directory, max 127 chars + null
    List subdirs;    // Offset 0x80 (sizeof(char[0x80]) == 0x80)
    List files;      // Offset 0x80 + sizeof(List) = 0x80 + 8 = 0x88. Original snippet implies 0x94.
                     // This means DirEntry itself is larger than just name + 2 lists.
                     // Let's adjust padding to match the 0xa8 total size used in calloc.
                     // If List is 8 bytes, then 0x80 + 8 + 8 = 0x90. Padding needed 0xa8 - 0x90 = 0x18.
    char _padding_after_files[0xa8 - (0x80 + sizeof(List) + sizeof(List))]; // Adjust to match 0xa8 total size
};

// FileEntry structure based on `calloc` size (0x480)
struct FileEntry {
    char name[0x80];    // Name of the file, max 127 chars + null
    char content[0x400]; // Content of the file, max 0x3ff chars + null. Offset 0x80.
};

// --- Mock Implementations for List Operations and Other Utilities ---

// Terminate function
void _terminate(int status) {
    fprintf(stderr, "Program terminated with status %d\n", status);
    exit(status);
}

// Global variable for root directory name
const char *DAT_00014000 = "root_dir";

// String equality check
bool streq(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}

// List initialization
void list_init(List* list_ptr, free_data_func func) {
    if (list_ptr == NULL) return;
    list_ptr->head = NULL;
    list_ptr->free_func = func;
}

// Get head node (returns the Node*, not its data)
Node* list_head_node(List* list_ptr) {
    if (list_ptr == NULL) return NULL;
    return list_ptr->head;
}

// Calculate list length (iterative for simplicity)
uint list_length(List* list_ptr) {
    if (list_ptr == NULL) return 0;
    uint count = 0;
    Node* current = list_ptr->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Insert data at the end of the list
void list_insert_at_end(List* list_ptr, void* data) {
    if (list_ptr == NULL || data == NULL) return;

    Node* new_node = (Node*)calloc(1, sizeof(Node));
    if (new_node == NULL) {
        _terminate(10); // Memory allocation failed for list node
    }
    new_node->data = data;
    new_node->next = NULL;

    if (list_ptr->head == NULL) {
        list_ptr->head = new_node;
    } else {
        Node* current = list_ptr->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// Remove a specific node from the list
void list_remove_node(List* list_ptr, Node* node_to_remove) {
    if (list_ptr == NULL || node_to_remove == NULL) return;

    if (list_ptr->head == node_to_remove) {
        list_ptr->head = node_to_remove->next;
    } else {
        Node* current = list_ptr->head;
        while (current != NULL && current->next != node_to_remove) {
            current = current->next;
        }
        if (current != NULL) {
            current->next = node_to_remove->next;
        }
    }
    // Note: Node's data and node itself should be freed by list_destroy_node later
}

// Destroy a node and its data
void list_destroy_node(List* list_ptr, Node** node_ptr) {
    if (list_ptr == NULL || node_ptr == NULL || *node_ptr == NULL) return;

    if (list_ptr->free_func != NULL) {
        list_ptr->free_func((*node_ptr)->data);
    }
    free(*node_ptr);
    *node_ptr = NULL; // Clear the pointer after freeing
}

// Destroy the entire list, freeing all nodes and their data
void list_destroy(List* list_ptr) {
    if (list_ptr == NULL) return;
    Node* current = list_ptr->head;
    while (current != NULL) {
        Node* next_node = current->next;
        if (list_ptr->free_func != NULL) {
            list_ptr->free_func(current->data);
        }
        free(current);
        current = next_node;
    }
    list_ptr->head = NULL;
}

// Forward declarations for FS functions used in list callbacks
void freedir(void* param_1);
void freefile(void* param_1);
bool get_file(void* node_data, void* search_key);
DirEntry* get_dir(DirEntry* dir_in_list, char* full_path);
void init_fs(void); // Forward declaration

// Recursive list search (used by `get_dir` for sub-paths)
// The `search_func` (which is `get_dir` itself) is responsible for handling recursion logic.
void* list_find_node_with_data_recurse(List* list_ptr, void* (*search_func)(void*, void*), void* search_key) {
    if (list_ptr == NULL) return NULL;

    Node* current = list_ptr->head;
    while (current != NULL) {
        void* result = search_func(current->data, search_key);
        if (result != NULL) {
            return result;
        }
        current = current->next;
    }
    return NULL;
}

// Non-recursive list search (used by `get_file`)
void* list_find_node_with_data(List* list_ptr, compare_data_func compare_func, void* search_key) {
    if (list_ptr == NULL) return NULL;

    Node* current = list_ptr->head;
    while (current != NULL) {
        if (compare_func(current->data, search_key)) {
            return current->data; // Return the data itself, not the node
        }
        current = current->next;
    }
    return NULL;
}

// --- Original Functions (Refactored) ---

// Function: ls_dir
void * ls_dir(char *param_1) {
  size_t path_len = strlen(param_1);
  if (root == NULL) {
    init_fs();
  }

  if (path_len == 0) {
    return NULL;
  }

  DirEntry* target_dir = NULL;
  if ((path_len == 1) && (*param_1 == '#')) {
    target_dir = root;
  } else {
    target_dir = (DirEntry*)list_find_node_with_data_recurse(&root->subdirs, (void* (*)(void*, void*))get_dir, param_1);
    if (target_dir == NULL) {
      return NULL;
    }
  }

  List* files_list = &target_dir->files;
  
  size_t total_len = 0;
  Node* current_node = list_head_node(files_list);
  uint i = 0;
  uint list_len = list_length(files_list);

  for (; i < list_len; ++i) {
    total_len += strlen(((FileEntry*)current_node->data)->name) + 1; // +1 for ':' separator
    current_node = current_node->next;
  }

  char* result_buffer = (char*)calloc(total_len + 1, sizeof(char)); // +1 for final null terminator
  if (result_buffer == NULL) {
    return NULL;
  }

  size_t current_offset = 0;
  current_node = list_head_node(files_list); // Reset for second loop
  for (i = 0; i < list_len; ++i) {
    char* file_name = ((FileEntry*)current_node->data)->name;
    size_t name_len = strlen(file_name);
    
    strcpy(result_buffer + current_offset, file_name);
    current_offset += name_len;
    result_buffer[current_offset] = ':'; // Separator
    current_offset += 1; // Move past separator
    
    current_node = current_node->next;
  }
  
  if (total_len > 0) { // If files were added, overwrite the last ':' with null terminator
    result_buffer[current_offset - 1] = '\0';
  } else { // If no files, ensure it's an empty string
    result_buffer[0] = '\0';
  }

  return result_buffer;
}

// Function: add_dir
int add_dir(char *param_1) {
  if (root == NULL) {
    init_fs();
  }

  size_t path_len = strlen(param_1);
  if (path_len < 2) {
    return 3;
  }
  if (path_len >= 0x80) {
    return 4;
  }

  DirEntry* existing_dir = (DirEntry*)list_find_node_with_data_recurse(&root->subdirs, (void* (*)(void*, void*))get_dir, param_1);
  if (existing_dir != NULL) {
    return 1; // Directory already exists
  }

  char* path_copy = (char*)calloc(path_len + 1, sizeof(char));
  if (path_copy == NULL) {
    return 2;
  }
  strcpy(path_copy, param_1);

  char* last_token_name = NULL;
  char* parent_path_buffer = (char*)calloc(path_len + 1, sizeof(char));
  if (parent_path_buffer == NULL) {
      free(path_copy);
      return 2;
  }
  parent_path_buffer[0] = '\0';

  char* token = strtok(path_copy, "#");
  while (token != NULL) {
      if (last_token_name != NULL) {
          if (parent_path_buffer[0] != '\0') {
              strcat(parent_path_buffer, "#");
          }
          strcat(parent_path_buffer, last_token_name);
      }
      last_token_name = token;
      token = strtok(NULL, "#");
  }

  if (last_token_name == NULL || strlen(last_token_name) < 1) {
    free(path_copy);
    free(parent_path_buffer);
    return 2;
  }

  DirEntry* parent_dir = NULL;
  if (parent_path_buffer[0] == '\0' || (strlen(parent_path_buffer) == 1 && parent_path_buffer[0] == '#')) {
    parent_dir = root;
  } else {
    parent_dir = (DirEntry*)list_find_node_with_data_recurse(&root->subdirs, (void* (*)(void*, void*))get_dir, parent_path_buffer);
    if (parent_dir == NULL) {
      free(path_copy);
      free(parent_path_buffer);
      return 3;
    }
  }
  
  DirEntry* new_dir = (DirEntry*)calloc(1, sizeof(DirEntry));
  if (new_dir == NULL) {
    free(path_copy);
    free(parent_path_buffer);
    return 1;
  }
  
  strcpy(new_dir->name, last_token_name);
  list_init(&new_dir->subdirs, freedir);
  list_init(&new_dir->files, freefile);
  list_insert_at_end(&parent_dir->subdirs, new_dir);

  free(path_copy);
  free(parent_path_buffer);
  return 0;
}

// Function: rm_dir
int rm_dir(char *param_1) {
  if (root == NULL) {
    init_fs();
  }
  if (param_1 == NULL) {
    return 1;
  }

  size_t path_len = strlen(param_1);
  if (path_len >= 0x80) {
    return 2;
  }
  if (path_len < 2) {
    return 7;
  }

  char* path_copy = (char*)calloc(path_len + 1, sizeof(char));
  if (path_copy == NULL) {
    return 3;
  }
  strcpy(path_copy, param_1);

  char* last_token_name = NULL;
  char* parent_path_buffer = (char*)calloc(path_len + 1, sizeof(char));
  if (parent_path_buffer == NULL) {
      free(path_copy);
      return 3;
  }
  parent_path_buffer[0] = '\0';

  char* token = strtok(path_copy, "#");
  while (token != NULL) {
      if (last_token_name != NULL) {
          if (parent_path_buffer[0] != '\0') {
              strcat(parent_path_buffer, "#");
          }
          strcat(parent_path_buffer, last_token_name);
      }
      last_token_name = token;
      token = strtok(NULL, "#");
  }

  DirEntry* parent_dir = NULL;
  if (parent_path_buffer[0] == '\0' || (strlen(parent_path_buffer) == 1 && parent_path_buffer[0] == '#')) {
    parent_dir = root;
  } else {
    parent_dir = (DirEntry*)list_find_node_with_data_recurse(&root->subdirs, (void* (*)(void*, void*))get_dir, parent_path_buffer);
    if (parent_dir == NULL) {
      free(path_copy);
      free(parent_path_buffer);
      return 5;
    }
  }
  
  List* subdirs_list = &parent_dir->subdirs;
  if (subdirs_list == NULL) { // Should not be NULL if parent_dir is valid
    free(path_copy);
    free(parent_path_buffer);
    return 6; 
  }

  Node* current_node = list_head_node(subdirs_list);
  uint i = 0;
  uint list_len = list_length(subdirs_list);

  for (; i < list_len; ++i) {
    DirEntry* dir_entry_to_check = (DirEntry*)current_node->data;
    if (streq(last_token_name, dir_entry_to_check->name)) {
      list_remove_node(subdirs_list, current_node);
      list_destroy_node(subdirs_list, &current_node);
      free(path_copy);
      free(parent_path_buffer);
      return 0;
    }
    current_node = current_node->next;
  }

  free(path_copy);
  free(parent_path_buffer);
  return 1;
}

// Function: add_file
int add_file(char *param_1, char *param_2) {
  if (root == NULL) {
    init_fs();
  }

  size_t path_len = strlen(param_1);
  char* path_copy = (char*)calloc(path_len + 1, sizeof(char));
  if (path_copy == NULL) {
    return 7;
  }
  strcpy(path_copy, param_1);

  char* last_token_name = NULL;
  char* parent_path_buffer = (char*)calloc(path_len + 1, sizeof(char));
  if (parent_path_buffer == NULL) {
      free(path_copy);
      return 7;
  }
  parent_path_buffer[0] = '\0';

  char* token = strtok(path_copy, "#");
  while (token != NULL) {
      if (last_token_name != NULL) {
          if (parent_path_buffer[0] != '\0') {
              strcat(parent_path_buffer, "#");
          }
          strcat(parent_path_buffer, last_token_name);
      }
      last_token_name = token;
      token = strtok(NULL, "#");
  }

  if (last_token_name == NULL || strlen(last_token_name) == 0) {
    free(path_copy);
    free(parent_path_buffer);
    return 2;
  }

  if (strlen(last_token_name) >= 0x80) {
    free(path_copy);
    free(parent_path_buffer);
    return 3;
  }

  if (strlen(param_2) >= 0x3ff) {
    free(path_copy);
    free(parent_path_buffer);
    return 4;
  }

  DirEntry* target_parent_dir = NULL;
  if (parent_path_buffer[0] == '\0' || (strlen(parent_path_buffer) == 1 && parent_path_buffer[0] == '#')) {
    target_parent_dir = root;
  } else {
    target_parent_dir = (DirEntry*)list_find_node_with_data_recurse(&root->subdirs, (void* (*)(void*, void*))get_dir, parent_path_buffer);
    if (target_parent_dir == NULL) {
      free(path_copy);
      free(parent_path_buffer);
      return 1;
    }
  }

  FileEntry* existing_file = (FileEntry*)list_find_node_with_data(&target_parent_dir->files, get_file, last_token_name);
  if (existing_file != NULL) {
    free(path_copy);
    free(parent_path_buffer);
    return 1;
  }

  FileEntry* new_file = (FileEntry*)calloc(1, sizeof(FileEntry));
  if (new_file == NULL) {
    free(path_copy);
    free(parent_path_buffer);
    return 7;
  }
  
  strcpy(new_file->name, last_token_name);
  strcpy(new_file->content, param_2);
  list_insert_at_end(&target_parent_dir->files, new_file);

  free(path_copy);
  free(parent_path_buffer);
  return 0;
}

// Function: rm_file
int rm_file(char *param_1) {
  if (root == NULL) {
    init_fs();
  }
  if (param_1 == NULL) {
    return 1;
  }

  size_t path_len = strlen(param_1);
  if (path_len >= 0x80) {
    return 2;
  }
  if (path_len < 2) {
    return 7;
  }

  char* path_copy = (char*)calloc(path_len + 1, sizeof(char));
  if (path_copy == NULL) {
    return 3;
  }
  strcpy(path_copy, param_1);

  char* last_token_name = NULL;
  char* parent_path_buffer = (char*)calloc(path_len + 1, sizeof(char));
  if (parent_path_buffer == NULL) {
      free(path_copy);
      return 3;
  }
  parent_path_buffer[0] = '\0';

  char* token = strtok(path_copy, "#");
  while (token != NULL) {
      if (last_token_name != NULL) {
          if (parent_path_buffer[0] != '\0') {
              strcat(parent_path_buffer, "#");
          }
          strcat(parent_path_buffer, last_token_name);
      }
      last_token_name = token;
      token = strtok(NULL, "#");
  }

  DirEntry* parent_dir = NULL;
  if (parent_path_buffer[0] == '\0' || (strlen(parent_path_buffer) == 1 && parent_path_buffer[0] == '#')) {
    parent_dir = root;
  } else {
    parent_dir = (DirEntry*)list_find_node_with_data_recurse(&root->subdirs, (void* (*)(void*, void*))get_dir, parent_path_buffer);
    if (parent_dir == NULL) {
      free(path_copy);
      free(parent_path_buffer);
      return 5;
    }
  }
  
  List* files_list = &parent_dir->files;
  if (files_list == NULL) {
    free(path_copy);
    free(parent_path_buffer);
    return 7;
  }

  Node* current_node = list_head_node(files_list);
  uint i = 0;
  uint list_len = list_length(files_list);

  for (; i < list_len; ++i) {
    FileEntry* file_entry_to_check = (FileEntry*)current_node->data;
    if (streq(last_token_name, file_entry_to_check->name)) {
      list_remove_node(files_list, current_node);
      list_destroy_node(files_list, &current_node);
      free(path_copy);
      free(parent_path_buffer);
      return 0;
    }
    current_node = current_node->next;
  }

  free(path_copy);
  free(parent_path_buffer);
  return 1;
}

// Function: readfile
int readfile(char *param_1) {
  if (root == NULL) {
    init_fs();
  }

  size_t path_len = strlen(param_1);
  if (path_len >= 0x80 || path_len < 2) {
    return 0;
  }

  char* path_copy = (char*)calloc(path_len + 1, sizeof(char));
  if (path_copy == NULL) {
    return 0;
  }
  strcpy(path_copy, param_1);

  char* last_token_name = NULL;
  char* parent_path_buffer = (char*)calloc(path_len + 1, sizeof(char));
  if (parent_path_buffer == NULL) {
      free(path_copy);
      return 0;
  }
  parent_path_buffer[0] = '\0';

  char* token = strtok(path_copy, "#");
  while (token != NULL) {
      if (last_token_name != NULL) {
          if (parent_path_buffer[0] != '\0') {
              strcat(parent_path_buffer, "#");
          }
          strcat(parent_path_buffer, last_token_name);
      }
      last_token_name = token;
      token = strtok(NULL, "#");
  }

  if (last_token_name == NULL || strlen(last_token_name) < 1) {
    free(path_copy);
    free(parent_path_buffer);
    return 0;
  }

  DirEntry* parent_dir = NULL;
  if (parent_path_buffer[0] == '\0' || (strlen(parent_path_buffer) == 1 && parent_path_buffer[0] == '#')) {
    parent_dir = root;
  } else {
    parent_dir = (DirEntry*)list_find_node_with_data_recurse(&root->subdirs, (void* (*)(void*, void*))get_dir, parent_path_buffer);
    if (parent_dir == NULL) {
      free(path_copy);
      free(parent_path_buffer);
      return 0;
    }
  }

  FileEntry* target_file = (FileEntry*)list_find_node_with_data(&parent_dir->files, get_file, last_token_name);
  if (target_file == NULL) {
    free(path_copy);
    free(parent_path_buffer);
    return 0;
  }

  free(path_copy);
  free(parent_path_buffer);
  return (int)target_file->content;
}

// Function: init_fs
void init_fs(void) {
  if (root != NULL) {
    _terminate(8);
  }

  root = (DirEntry*)calloc(1, sizeof(DirEntry));
  if (root == NULL) {
    _terminate(9);
  }
  
  strcpy(root->name, DAT_00014000);
  list_init(&root->subdirs, freedir);
  list_init(&root->files, freefile);

  int add_file_status = add_file("#EAR#DME", "This is a sample file! Feel free to add your own!");
  if (add_file_status != 0) {
    _terminate(6);
  }
}

// Function: freedir
void freedir(void *param_1) {
  DirEntry* dir_entry = (DirEntry*)param_1;
  list_destroy(&dir_entry->subdirs);
  list_destroy(&dir_entry->files);
  free(dir_entry);
}

// Function: freefile
void freefile(void *param_1) {
  free(param_1);
}

// Function: get_file
bool get_file(void* node_data, void* search_key) {
  FileEntry* file_entry = (FileEntry*)node_data;
  char* file_name_to_find = (char*)search_key;
  return streq(file_entry->name, file_name_to_find);
}

// Function: get_dir
DirEntry* get_dir(DirEntry* dir_in_list, char* full_path) {
  if (*full_path != '#') {
    return NULL;
  }

  char* path_segment_start = full_path + 1;
  char* next_separator = strchr(path_segment_start, '#');

  char current_segment_name[0x80];
  size_t segment_len;

  if (next_separator != NULL) {
    segment_len = next_separator - path_segment_start;
  } else {
    segment_len = strlen(path_segment_start);
  }

  if (segment_len >= sizeof(current_segment_name)) {
      return NULL;
  }
  strncpy(current_segment_name, path_segment_start, segment_len);
  current_segment_name[segment_len] = '\0';

  if (!streq(dir_in_list->name, current_segment_name)) {
    return NULL;
  }

  if (next_separator == NULL) {
    return dir_in_list;
  } else {
    return (DirEntry*)list_find_node_with_data_recurse(&dir_in_list->subdirs, (void* (*)(void*, void*))get_dir, next_separator);
  }
}