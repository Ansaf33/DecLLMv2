#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For uint8_t, uint16_t, uint32_t
#include <stddef.h> // For size_t

// Forward declaration for FileNode structure
typedef struct FileNode_s FileNode;

// Access macros for FileNode fields based on original offsets and assumed memory layout.
// This assumes 'node' is a (char*) pointer to the allocated 0x60 bytes.
// The name string is stored at the beginning of the allocated 0x60 bytes.
#define NODE_NAME(node)        ((char*)(node))
#define NODE_TYPE(node)        (*(uint8_t*)((char*)(node) + 0x41))
#define NODE_SIZE(node)        (*(uint32_t*)((char*)(node) + 0x44))
#define NODE_ID(node)          (*(uint16_t*)((char*)(node) + 0x48))
#define NODE_PREV(node)        (*(FileNode**)((char*)(node) + 0x4c))
#define NODE_NEXT(node)        (*(FileNode**)((char*)(node) + 0x50))
#define NODE_PARENT(node)      (*(FileNode**)((char*)(node) + 0x54))
#define NODE_CHILDREN(node)    (*(FileNode**)((char*)(node) + 0x58))
#define NODE_DATA(node)        (*(void**)((char*)(node) + 0x5c))

// The actual FileNode structure is implicitly defined by the memory layout and access macros.
// We provide a dummy struct definition to allow `FileNode*` type to be used in C.
struct FileNode_s {
    char _reserved[0x60]; // Represents the 0x60 bytes allocated for a node
};

// Global variables
FileNode *root = NULL;
int numFiles = 0;
static uint16_t next_file_id_counter = 1; // File IDs start from 1, 0 might be reserved/invalid
static const char ROOT_PATH_PREFIX[] = "%25"; // Corresponds to DAT_00016007

// Function: DestroyNode (forward declaration for CreateFile)
void DestroyNode(FileNode *node);

// Function: GetPathDepth (forward declaration for CreateFile)
int GetPathDepth(FileNode *node);

// Function: FindFile (forward declaration for FindFileAbsolute and DeleteFile)
FileNode * FindFile(const char *name, FileNode *parent_node);

// Function: InitializeFileSystem
void * InitializeFileSystem(void) {
  root = (FileNode*)calloc(1, 0x60); // Allocate 0x60 bytes for the root node
  if (root == NULL) {
      perror("Failed to allocate root file system node");
      return NULL;
  }
  memcpy(NODE_NAME(root), ROOT_PATH_PREFIX, strlen(ROOT_PATH_PREFIX) + 1); // Copy "%25" and null terminator
  NODE_TYPE(root) = 0xf7; // Set root type
  numFiles = 1;
  return root;
}

// Function: NextFileID
uint16_t NextFileID(void) {
  uint16_t current_id = next_file_id_counter;
  next_file_id_counter++;
  if (next_file_id_counter == 0) { // Wrap around if overflow (0 often invalid, so reset to 1)
    next_file_id_counter = 1;
  }
  return current_id;
}

// Function: CreateFile
uint32_t CreateFile(const char *file_name, uint8_t file_type, uint32_t file_size, void *initial_data, FileNode *parent_node) {
  size_t name_len = strlen(file_name);
  if (name_len == 0) {
    return 0xffffffff; // Error: empty name
  }

  if (!parent_node || NODE_TYPE(parent_node) != 0xf7) { // If parent is null or not a directory (type 0xf7)
    parent_node = root;
  }

  if (GetPathDepth(parent_node) >= 10) { // Max path depth check
    return 0xfffffffc; // Error: path too deep
  }

  if (file_type >= 0xf8) { // Invalid file type
    return 0xffffffff;
  }

  if (file_size >= 0x801) { // File size too large
    return 0xffffffff;
  }

  if (numFiles >= 0x100) { // Max files exceeded
    return 0xfffffffb;
  }

  if (strcmp(file_name, "upone") == 0) { // "upone" is a reserved name
    return 0xffffffff;
  }

  if (file_type == 0xf7) { // If it's a directory, size should be 0
    file_size = 0;
  }

  FileNode *new_node = (FileNode*)calloc(1, 0x60);
  if (new_node == NULL) {
      perror("Failed to allocate new file node");
      return 0xffffffff; // Error: allocation failed
  }
  
  memcpy(NODE_NAME(new_node), file_name, name_len);
  NODE_NAME(new_node)[name_len] = '\0'; // Ensure null termination
  NODE_TYPE(new_node) = file_type;
  NODE_ID(new_node) = NextFileID();
  numFiles++;

  if (file_size > 0 && initial_data != NULL) {
    void *file_data = calloc(1, file_size);
    if (file_data == NULL) {
        perror("Failed to allocate file data");
        DestroyNode(new_node); // Clean up partially created node
        return 0xffffffff; // Error: allocation failed
    }
    NODE_DATA(new_node) = file_data;
    memcpy(NODE_DATA(new_node), initial_data, file_size);
    NODE_SIZE(new_node) = file_size;
  }

  NODE_PARENT(new_node) = parent_node;
  FileNode *prev_child = NULL;
  FileNode *current_child = NODE_CHILDREN(parent_node);

  while (current_child != NULL) {
    int cmp_result = strcmp(NODE_NAME(new_node), NODE_NAME(current_child));
    if (cmp_result == 0) {
      DestroyNode(new_node); // Duplicate name, destroy new node
      return 0xfffffffd; // Error code for duplicate
    }
    if (cmp_result < 0) { // new_node comes before current_child
      NODE_NEXT(new_node) = current_child;
      NODE_PREV(new_node) = prev_child;

      if (prev_child != NULL) {
        NODE_NEXT(prev_child) = new_node;
      } else { // new_node is the new first child
        NODE_CHILDREN(parent_node) = new_node;
      }
      NODE_PREV(current_child) = new_node;
      return 0; // Success
    }
    prev_child = current_child;
    current_child = NODE_NEXT(current_child);
  }

  // If loop finishes, new_node should be appended to the end or is the first child
  if (prev_child == NULL) { // No children yet, new_node is the first
    NODE_CHILDREN(parent_node) = new_node;
  } else { // Append to the end
    NODE_NEXT(prev_child) = new_node;
    NODE_PREV(new_node) = prev_child;
  }
  return 0; // Success
}

// Function: DestroyNode
void DestroyNode(FileNode *node) {
  if (node != NULL) {
    numFiles--;
    DestroyNode(NODE_CHILDREN(node)); // Recursively destroy children
    DestroyNode(NODE_NEXT(node));     // Recursively destroy siblings
    free(NODE_DATA(node));            // Free file data
    free(node);                       // Free the node itself
  }
}

// Function: FindFile
FileNode * FindFile(const char *name, FileNode *parent_node) {
  if (parent_node == NULL || name == NULL) {
    return NULL;
  }

  FileNode *current_child = NODE_CHILDREN(parent_node);
  while (current_child != NULL) {
    int cmp_result = strcmp(name, NODE_NAME(current_child));
    if (cmp_result == 0) {
      return current_child;
    }
    if (cmp_result < 0) { // Since list is sorted, if we passed it, it's not here
      return NULL;
    }
    current_child = NODE_NEXT(current_child);
  }
  return NULL;
}

// Function: FindFileAbsolute
FileNode * FindFileAbsolute(const char *path) {
  if (path == NULL) {
    return NULL;
  }

  char component_name[65]; // Buffer for path components
  char *path_segment = (char *)path;
  char *next_separator;
  FileNode *current_node = root;

  do {
    if (*path_segment == '\0' || !(next_separator = strchr(path_segment, '%'))) {
      // Last segment or no more separators
      return FindFile(path_segment, current_node);
    }
    
    // Extract component name
    size_t len = next_separator - path_segment;
    if (len >= sizeof(component_name)) { // Component name too long
        return NULL;
    }
    memcpy(component_name, path_segment, len);
    component_name[len] = '\0';

    current_node = FindFile(component_name, current_node);
    path_segment = next_separator + 1; // Move past separator
  } while (current_node != NULL);

  return NULL; // Path component not found
}

// Function: DeleteFile
uint32_t DeleteFile(const char *file_name, FileNode *parent_node) {
  FileNode *file_to_delete = FindFile(file_name, parent_node);
  if (file_to_delete == NULL) {
    return 0xfffffffe; // Error: file not found
  }
  if (file_to_delete == root) {
    return 0xfffffffc; // Error: cannot delete root
  }

  // Unlink from parent's children list
  if (file_to_delete == NODE_CHILDREN(parent_node)) {
    NODE_CHILDREN(parent_node) = NODE_NEXT(file_to_delete);
  }

  // Unlink from sibling list
  if (NODE_PREV(file_to_delete) != NULL) {
    NODE_NEXT(NODE_PREV(file_to_delete)) = NODE_NEXT(file_to_delete);
  }
  if (NODE_NEXT(file_to_delete) != NULL) {
    NODE_PREV(NODE_NEXT(file_to_delete)) = NODE_PREV(file_to_delete);
  }

  // Clear pointers in the node being deleted
  NODE_PREV(file_to_delete) = NULL;
  NODE_NEXT(file_to_delete) = NULL;

  DestroyNode(file_to_delete);
  return 0; // Success
}

// Function: GetFileType
uint8_t GetFileType(FileNode *node) {
  if (node == NULL) {
    return 0;
  }
  return NODE_TYPE(node);
}

// Function: GetFileSize
uint32_t GetFileSize(FileNode *node) {
  if (node == NULL) {
    return 0;
  }
  return NODE_SIZE(node);
}

// Function: GetFileID
uint16_t GetFileID(FileNode *node) {
  if (node == NULL) {
    return 0;
  }
  return NODE_ID(node);
}

// Function: GetFilePath
char * GetFilePath(FileNode *node) {
  if (node == NULL) {
    return NULL;
  }
  if (node == root) {
    return NULL; // Root has no path components leading to it
  }

  int total_len = 0;
  FileNode *current_node = node;

  // Calculate total length needed for the path string
  while (current_node != root) {
    total_len += strlen(NODE_NAME(current_node)) + 1; // +1 for the '%' separator
    current_node = NODE_PARENT(current_node);
  }
  total_len += strlen(ROOT_PATH_PREFIX); // For the initial "%25"
  total_len += 1; // For the null terminator

  char *path_buffer = (char*)calloc(total_len, 1);
  if (path_buffer == NULL) {
      perror("Failed to allocate path buffer");
      return NULL;
  }

  char *current_pos = path_buffer + total_len - 1;
  *current_pos = '\0'; // Null-terminate the string
  current_pos--;

  current_node = node;
  while (current_node != root) {
    size_t len = strlen(NODE_NAME(current_node));
    current_pos -= len;
    memcpy(current_pos, NODE_NAME(current_node), len);
    current_pos--;
    *current_pos = '%'; // Add the separator
    current_node = NODE_PARENT(current_node);
  }
  memcpy(path_buffer, ROOT_PATH_PREFIX, strlen(ROOT_PATH_PREFIX)); // Copy the root prefix

  return path_buffer;
}

// Function: GetPathDepth
int GetPathDepth(FileNode *node) {
  int depth = 0;
  FileNode *current_node = node;
  while (current_node != NULL && current_node != root) {
    depth++;
    current_node = NODE_PARENT(current_node);
  }
  return depth;
}

// Function: ReadFile
void * ReadFile(FileNode *node) {
  if (node == NULL || NODE_SIZE(node) == 0) {
    return NULL;
  }

  void *read_buffer = calloc(1, NODE_SIZE(node) + 1); // +1 for potential null termination if needed
  if (read_buffer == NULL) {
      perror("Failed to allocate read buffer");
      return NULL;
  }
  memcpy(read_buffer, NODE_DATA(node), NODE_SIZE(node));
  // The original code doesn't null-terminate, but calloc ensures it's zeroed.
  // ((char*)read_buffer)[NODE_SIZE(node)] = '\0'; // Optional: for text data

  return read_buffer;
}

// Function: GetFileName
const char * GetFileName(FileNode *node) {
  if (node == NULL) {
      return NULL;
  }
  return NODE_NAME(node);
}

// Function: GetParent
FileNode * GetParent(FileNode *node) {
  if (node == NULL || node == root) {
    return root;
  }
  return NODE_PARENT(node);
}