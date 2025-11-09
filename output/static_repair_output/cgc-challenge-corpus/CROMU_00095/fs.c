#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Assume sizes are 0x40 for service type strings
#define MAX_SERVICE_TYPE_LEN 0x40
char serviceTypes[5][MAX_SERVICE_TYPE_LEN];

// Define structures based on decompiled offsets and observed member usage
// Tree structure (0xEC bytes)
typedef struct {
    int id;               // Offset 0
    char name[0x80];      // Offset 4
    char service_type[0x40]; // Offset 0x84
    void* files[10];      // Offset 0xC4 (10 * sizeof(void*))
} Tree;

// File structure (0x8C bytes)
typedef struct {
    short id;             // Offset 0
    char name[130];       // Offset 2 (to 0x82)
    char is_open;         // Offset 0x82
    unsigned short data_size; // Offset 0x84
    void* data_ptr;       // Offset 0x88
} File;

// Assume 10 trees, each sizeof(Tree) bytes
#define MAX_TREES 10
char allTrees[MAX_TREES][sizeof(Tree)]; // Using char array to manage raw memory offsets

// Dummy Implementations for External Functions
unsigned int random_in_range(unsigned int min, unsigned int max) {
    if (max == 0) { // Special case for random_in_range(10) observed in original
        return rand() % min;
    }
    if (min > max) { // Ensure min <= max
        unsigned int temp = min;
        min = max;
        max = temp;
    }
    return min + (rand() % (max - min + 1));
}

void populate_random_string(char *buffer, unsigned int length) {
    if (buffer == NULL || length == 0) {
        return;
    }
    for (unsigned int i = 0; i < length - 1; ++i) {
        buffer[i] = 'A' + (rand() % 26);
    }
    buffer[length - 1] = '\0';
}

// Function Prototypes for mutual recursion
void* CreateFile(int tree_ptr_as_int, char *filename, unsigned int flags_param_3, int flags_param_4);
void* CloseFile(void* file_ptr);
unsigned int WriteFile(int file_ptr_as_int, void *src_buffer, unsigned short offset, unsigned short length);
int FindFileByName(int tree_ptr_as_int, char *filename);

// Function: InitializeFileSystem
void InitializeFileSystem(void) {
  char filename_buffer[128];

  for (int i = 0; i < 5; ++i) {
    populate_random_string(serviceTypes[i], random_in_range(0x20, 0x40));
  }

  int tree_name_base_len = random_in_range(0x40, 0x80);

  for (int i = 0; i < 9; ++i) {
    Tree* current_tree = (Tree*)(allTrees + i * sizeof(Tree));
    current_tree->id = rand();
    populate_random_string(current_tree->name, tree_name_base_len - i);
    strcpy(current_tree->service_type, serviceTypes[random_in_range(0, 4)]);

    int num_files_to_create = random_in_range(1, 10);
    int file_name_base_len = random_in_range(0x40, 0x80);

    for (int j = 0; j < num_files_to_create; ++j) {
      populate_random_string(filename_buffer, file_name_base_len - j);
      
      File* file_obj = (File*)CreateFile((int)current_tree, filename_buffer, 0, 0x31);

      if (file_obj != NULL) {
          unsigned int data_len = random_in_range(10, 0x400);
          void* file_data_buffer = calloc(data_len, 1);
          if (file_data_buffer == NULL) continue; // Handle allocation failure

          populate_random_string((char*)file_data_buffer, data_len);
          WriteFile((int)file_obj, file_data_buffer, 0, data_len);
          CloseFile(file_obj);
          
          free(file_data_buffer);
          current_tree->files[j] = file_obj;
      }
    }
  }

  Tree* last_tree = (Tree*)(allTrees + 9 * sizeof(Tree));
  last_tree->id = rand();
  strcpy(last_tree->name, "SOMETREE");
  strcpy(last_tree->service_type, "EYEPSEE");

  File* netstuff_file = (File*)calloc(1, sizeof(File));
  if (netstuff_file != NULL) {
      netstuff_file->id = (short)rand();
      strncpy(netstuff_file->name, "NETSTUFF", sizeof(netstuff_file->name) - 1);
      netstuff_file->name[sizeof(netstuff_file->name) - 1] = '\0';
      last_tree->files[0] = netstuff_file;
  }
}

// Function: FindFileByName
int FindFileByName(int tree_ptr_as_int, char *filename) {
  Tree* tree = (Tree*)tree_ptr_as_int;

  for (int i = 0; i < 10; ++i) {
    File* file_obj = (File*)tree->files[i];
    
    if (file_obj != NULL && strcmp(file_obj->name, filename) == 0) {
        return (int)file_obj;
    }
  }
  return 0;
}

// Function: CreateFile
void* CreateFile(int tree_ptr_as_int, char *filename, unsigned int flags_param_3, int flags_param_4) {
  Tree* tree = (Tree*)tree_ptr_as_int;
  File* existing_file = (File*)FindFileByName(tree_ptr_as_int, filename);

  if (existing_file == NULL) {
    if (flags_param_4 == 0x31) {
      for (int i = 0; i < 10; ++i) {
        if (tree->files[i] == NULL) {
          File* new_file = (File*)calloc(1, sizeof(File));
          if (new_file == NULL) return NULL;

          new_file->id = (short)rand();
          strncpy(new_file->name, filename, sizeof(new_file->name) - 1);
          new_file->name[sizeof(new_file->name) - 1] = '\0';
          new_file->is_open = 1;

          tree->files[i] = new_file;
          return new_file;
        }
      }
      return NULL;
    }
    return NULL;
  } else {
    if (existing_file->is_open == 1) {
      return NULL;
    } else {
      existing_file->is_open = 1;
      return existing_file;
    }
  }
}

// Function: CloseFile
void* CloseFile(void* file_ptr) {
  if (file_ptr != NULL) {
    ((File*)file_ptr)->is_open = 0;
    return file_ptr;
  }
  return NULL;
}

// Function: ReadFile
unsigned int ReadFile(void *buffer, int file_ptr_as_int, unsigned short offset, unsigned short length) {
  File* file = (File*)file_ptr_as_int;

  if (buffer == NULL || file == NULL || length == 0) {
    return 0xFFFFFFFF;
  }
  
  if (file->data_size < offset) {
    return 0xFFFFFFFF;
  }

  unsigned short bytes_to_read = length;
  if (file->data_size < (unsigned int)offset + length) {
    bytes_to_read = file->data_size - offset;
  }

  if (bytes_to_read == 0) {
      return 0;
  }

  memcpy(buffer, (char*)file->data_ptr + offset, bytes_to_read);
  return 0;
}

// Function: WriteFile
unsigned int WriteFile(int file_ptr_as_int, void *src_buffer, unsigned short offset, unsigned short length) {
  File* file = (File*)file_ptr_as_int;

  if (file == NULL || src_buffer == NULL || length == 0) {
    return 0xFFFFFFFF;
  }

  if ((unsigned int)offset + length > 0x400) {
    return 0xFFFFFFFF;
  }

  void* new_data_buffer = calloc((unsigned int)offset + length, 1);
  if (new_data_buffer == NULL) return 0xFFFFFFFF;

  void* old_data_buffer = file->data_ptr;
  
  if (offset != 0 && old_data_buffer != NULL) {
    unsigned short bytes_to_copy_before_offset = (file->data_size < offset) ? file->data_size : offset;
    memcpy(new_data_buffer, old_data_buffer, bytes_to_copy_before_offset);
  }

  memcpy((char*)new_data_buffer + offset, src_buffer, length);
  
  file->data_ptr = new_data_buffer;
  file->data_size = offset + length;

  free(old_data_buffer);
  return 0;
}

// Function: FindTreeByPath
void* FindTreeByPath(unsigned int unused_param, char *tree_name, char *service_type) {
  for (int i = 0; i < MAX_TREES; ++i) {
    Tree* current_tree = (Tree*)(allTrees + i * sizeof(Tree));

    if (strcmp(current_tree->name, tree_name) == 0 &&
        strcmp(current_tree->service_type, service_type) == 0) {
      return current_tree;
    }
  }
  return NULL;
}

// Main function for Linux compilation
int main() {
    srand(time(NULL));
    InitializeFileSystem();
    return 0;
}