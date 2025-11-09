#include <stdio.h>      // For printf
#include <stdlib.h>     // For malloc, free
#include <string.h>     // For memset, strncpy, strncat
#include <sys/types.h>  // For ssize_t
#include <sys/socket.h> // For recv
#include <unistd.h>     // For close (though not explicitly used in given snippets)

// Define types from the snippet
typedef int undefined4;
typedef char undefined;

// --- Dummy Struct and Function Declarations ---
// These are reconstructed based on usage patterns and offsets in the original code.
// They are necessary to make the provided snippets compilable and logically consistent.

// Structure for a file system entry (can be a file or a directory)
typedef struct FileEntry {
    char name[256]; // Assumed name buffer, adjust size if actual offset 0x100 is different
    // The following members are inferred from offsets like 0x100, 0x104, 0x108
    // Relative to the start of this struct or an enclosing object.
    // For simplicity, we assume they are direct members after the name.
    int is_directory;           // If 1, it's a directory; if 0, it's a file (inferred from 0x100)
    unsigned int data_or_child_count; // File size if file (0x104), or number of children if directory (0x104)
    void *data_ptr;             // Pointer to file data if file (0x108), or array of children if directory (0x108)
    // If data_ptr points to an array of children, it would be `struct FileEntry **children;`
    // For `handle_prnt`, it's `struct FileEntry **children_array;`
} FileEntry;

// Dummy function for bubble_sort
void bubble_sort(FileEntry *entry) {
    // In a real implementation, this would sort the children of the directory entry.
    // printf("DEBUG: Sorting children of %s\n", entry->name);
}

// Dummy function for delete_file
int delete_file(char *filename) {
    // In a real implementation, this would delete the specified file.
    // printf("DEBUG: Attempting to delete file: %s\n", filename);
    return 1; // Simulate success
}

// Dummy function for get_file
FileEntry* get_file(char *filename) {
    // In a real implementation, this would retrieve a file/directory entry from a file system.
    // printf("DEBUG: Attempting to get file: %s\n", filename);
    // Simulate finding a file/directory
    if (strcmp(filename, "dummy_file.txt") == 0) {
        FileEntry *f = (FileEntry*)malloc(sizeof(FileEntry));
        if (f) {
            memset(f, 0, sizeof(FileEntry));
            strncpy(f->name, filename, sizeof(f->name) - 1);
            f->is_directory = 0;
            f->data_or_child_count = 12; // Dummy size
            f->data_ptr = malloc(f->data_or_child_count + 1);
            if (f->data_ptr) strcpy((char*)f->data_ptr, "Hello World!");
        }
        return f;
    } else if (strcmp(filename, "dummy_dir") == 0) {
        FileEntry *d = (FileEntry*)malloc(sizeof(FileEntry));
        if (d) {
            memset(d, 0, sizeof(FileEntry));
            strncpy(d->name, filename, sizeof(d->name) - 1);
            d->is_directory = 1;
            d->data_or_child_count = 0; // No children for dummy dir
            d->data_ptr = NULL;
        }
        return d;
    }
    return NULL;
}

// Dummy function for init_file
FileEntry* init_file() {
    FileEntry *new_entry = (FileEntry*)malloc(sizeof(FileEntry));
    if (new_entry) {
        memset(new_entry, 0, sizeof(FileEntry));
    }
    return new_entry;
}

// Dummy function for set_name
int set_name(FileEntry *entry, char *name) {
    if (!entry || !name) return 0;
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';
    return 1;
}

// Dummy function for set_type
int set_type(FileEntry *entry, int type) {
    if (!entry) return 0;
    entry->is_directory = type;
    return 1;
}

// Dummy function for set_data
int set_data(FileEntry *entry, int data_len, void *data_ptr) {
    if (!entry) return 0;
    if (entry->data_ptr) free(entry->data_ptr); // Free existing data if any
    entry->data_ptr = malloc(data_len + 1); // +1 for null terminator
    if (entry->data_ptr) {
        memcpy(entry->data_ptr, data_ptr, data_len);
        ((char*)entry->data_ptr)[data_len] = '\0'; // Null-terminate
        entry->data_or_child_count = data_len; // Store data length
        return 1;
    }
    return 0;
}

// Dummy function for add_file
int add_file(FileEntry *entry) {
    if (!entry) return 0;
    // In a real implementation, this would add the file/directory to the file system.
    // printf("DEBUG: Adding file/directory: %s (type: %d)\n", entry->name, entry->is_directory);
    return 1; // Simulate success
}

// Dummy function for free_file (to properly clean up FileEntry objects)
void free_file(FileEntry *entry) {
    if (entry) {
        if (entry->data_ptr) {
            free(entry->data_ptr);
            entry->data_ptr = NULL;
        }
        free(entry);
    }
}

// --- Fixed Functions ---

// Function: handle_repo
undefined4 handle_repo(int sockfd) { // Assumed sockfd is passed as an argument
  char filename_buffer[256];
  int received_len_val;
  ssize_t bytes_received;

  // Receive the length of the filename
  bytes_received = recv(sockfd, &received_len_val, sizeof(received_len_val), 0x1100f);
  if (bytes_received <= 0) {
    // printf("[ERROR] handle_repo: Failed to recv filename length\n");
    return 0;
  }
  
  size_t filename_len = (size_t)received_len_val;
  if (filename_len == 0 || filename_len >= sizeof(filename_buffer)) {
      // printf("[ERROR] handle_repo: Invalid filename length received (%zu)\n", filename_len);
      return 0;
  }

  memset(filename_buffer, 0, sizeof(filename_buffer));
  
  // Receive the filename itself
  bytes_received = recv(sockfd, filename_buffer, filename_len, 0x1100f);
  if (bytes_received <= 0) {
    // printf("[ERROR] handle_repo: Failed to recv filename\n");
    return 0;
  }
  filename_buffer[bytes_received] = '\0'; // Null-terminate received data

  return delete_file(filename_buffer);
}

// Function: handle_prnt
void handle_prnt(FileEntry *current_dir_entry, char *path_prefix) {
  char child_path_buffer[256];

  if (current_dir_entry != NULL) {
    bubble_sort(current_dir_entry);
    
    unsigned int num_children = current_dir_entry->data_or_child_count;
    // Assuming data_ptr points to an array of FileEntry pointers for children
    FileEntry **children = (FileEntry **)current_dir_entry->data_ptr;

    for (unsigned int i = 0; i < num_children; ++i) {
      FileEntry *child_entry = children[i];
      if (child_entry == NULL) continue;

      printf("%s/%s     ", path_prefix, child_entry->name);

      if (child_entry->is_directory == 0) { // If it's a file
        printf("File     %u\n", child_entry->data_or_child_count); // Print file size
      } else { // If it's a directory
        printf("Dir\n");
        
        memset(child_path_buffer, 0, sizeof(child_path_buffer));
        strncpy(child_path_buffer, path_prefix, sizeof(child_path_buffer) - 1);
        child_path_buffer[sizeof(child_path_buffer) - 1] = '\0'; // Ensure null termination
        
        strncat(child_path_buffer, "/", sizeof(child_path_buffer) - strlen(child_path_buffer) - 1);
        strncat(child_path_buffer, child_entry->name, sizeof(child_path_buffer) - strlen(child_path_buffer) - 1);
        
        handle_prnt(child_entry, child_path_buffer); // Recursive call
      }
    }
  }
}

// Function: handle_recv
undefined4 handle_recv(int sockfd) { // Assumed sockfd is passed as an argument
  char filename_buffer[256];
  int received_len_val;
  ssize_t bytes_received;

  // Receive the length of the filename
  bytes_received = recv(sockfd, &received_len_val, sizeof(received_len_val), 0x11200);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv length\n");
    return 0;
  }
  
  size_t filename_len = (size_t)received_len_val;
  if (filename_len == 0 || filename_len >= sizeof(filename_buffer)) {
      printf("[ERROR] Invalid filename length received\n");
      return 0;
  }

  memset(filename_buffer, 0, sizeof(filename_buffer));
  
  // Receive the filename itself
  bytes_received = recv(sockfd, filename_buffer, filename_len, 0x11200);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv name\n");
    return 0;
  }
  filename_buffer[bytes_received] = '\0'; // Null-terminate received data

  FileEntry *file_entry = get_file(filename_buffer);
  if (file_entry == NULL) {
    printf("[ERROR] RECV failed: File not found or creation failed\n");
    return 0;
  }
  
  if (file_entry->is_directory == 1) {
    printf("[ERROR] %s is a directory\n", file_entry->name);
    free_file(file_entry);
    return 0;
  }
  
  if (file_entry->data_ptr == NULL) {
    printf("[ERROR] File %s has no data or data pointer is null\n", file_entry->name);
    free_file(file_entry);
    return 0;
  }
  
  printf("[DATA] %s\n", (char*)file_entry->data_ptr);
  free_file(file_entry);
  return 1;
}

// Function: handle_ndir
undefined4 handle_ndir(int sockfd) { // Assumed sockfd is passed as an argument
  char dirname_buffer[256];
  int received_len_val;
  ssize_t bytes_received;
  
  // Receive the length of the directory name
  bytes_received = recv(sockfd, &received_len_val, sizeof(received_len_val), 0x1133c);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv length\n");
    return 0;
  }
  
  size_t dirname_len = (size_t)received_len_val;
  if (dirname_len == 0 || dirname_len >= sizeof(dirname_buffer)) {
      printf("[ERROR] Invalid directory name length received\n");
      return 0;
  }

  memset(dirname_buffer, 0, sizeof(dirname_buffer));
  
  // Receive the directory name itself
  bytes_received = recv(sockfd, dirname_buffer, dirname_len, 0x1133c);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv name\n");
    return 0;
  }
  dirname_buffer[bytes_received] = '\0'; // Null-terminate received data

  FileEntry *new_dir_entry = init_file();
  if (new_dir_entry == NULL) {
    printf("[ERROR] Failed to initialize a new directory\n");
    return 0;
  }
  
  if (set_name(new_dir_entry, dirname_buffer) == 0) {
    printf("[ERROR] Failed to set directory name\n");
    free_file(new_dir_entry);
    return 0;
  }
  
  if (set_type(new_dir_entry, 1) == 0) { // 1 for directory
    printf("[ERROR] Failed to set type\n");
    free_file(new_dir_entry);
    return 0;
  }
  
  if (add_file(new_dir_entry) == 0) {
    printf("[ERROR] NDIR failed\n");
    free_file(new_dir_entry);
    return 0;
  }
  
  printf("[INFO] Added new directory: %s\n", new_dir_entry->name);
  return 1;
}

// Function: handle_send
undefined4 handle_send(int sockfd) { // Assumed sockfd is passed as an argument
  char filename_buffer[256];
  int filename_len_val;
  int file_data_len_val;
  ssize_t bytes_received;
  void *file_data_ptr = NULL;
  FileEntry *new_file_entry = NULL;
  
  // Receive the length of the filename
  bytes_received = recv(sockfd, &filename_len_val, sizeof(filename_len_val), 0x114dc);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv filename length\n");
    return 0;
  }
  
  size_t filename_len = (size_t)filename_len_val;
  if (filename_len == 0 || filename_len >= sizeof(filename_buffer)) {
      printf("[ERROR] Invalid filename length received\n");
      return 0;
  }

  memset(filename_buffer, 0, sizeof(filename_buffer));
  
  // Receive the filename itself
  bytes_received = recv(sockfd, filename_buffer, filename_len, 0x114dc);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv name\n");
    return 0;
  }
  filename_buffer[bytes_received] = '\0'; // Null-terminate received data

  // Receive the length of the file data
  bytes_received = recv(sockfd, &file_data_len_val, sizeof(file_data_len_val), 0x114dc);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv file length\n");
    return 0;
  }

  // Check file data length constraints
  if (file_data_len_val <= 0 || file_data_len_val > 0x400) { // Max length is 0x400 (1024 bytes)
    printf("[ERROR] Maximum file length is 0x400, received %d\n", file_data_len_val);
    return 0;
  }
  
  // Allocate buffer for file data
  file_data_ptr = malloc(file_data_len_val + 1); // +1 for null terminator
  if (file_data_ptr == NULL) {
    printf("[ERROR] malloc failed for file data\n");
    return 0;
  }
  memset(file_data_ptr, 0, file_data_len_val + 1);
  
  // Receive the file data itself
  bytes_received = recv(sockfd, file_data_ptr, file_data_len_val, 0x114dc);
  if (bytes_received <= 0) {
    printf("[ERROR] Failed to recv data\n");
    free(file_data_ptr);
    return 0;
  }
  ((char*)file_data_ptr)[bytes_received] = '\0'; // Null-terminate received data

  new_file_entry = init_file();
  if (new_file_entry == NULL) {
    printf("[ERROR] Failed to initialize a new file\n");
    free(file_data_ptr);
    return 0;
  }
  
  if (set_name(new_file_entry, filename_buffer) == 0) {
    printf("[ERROR] Failed to set file name\n");
    free_file(new_file_entry); // Use free_file to clean up
    free(file_data_ptr);
    return 0;
  }
  
  if (set_type(new_file_entry, 0) == 0) { // 0 for file
    printf("[ERROR] Failed to set type\n");
    free_file(new_file_entry);
    free(file_data_ptr);
    return 0;
  }
  
  if (set_data(new_file_entry, file_data_len_val, file_data_ptr) == 0) {
    printf("[ERROR] Failed to set data\n");
    free_file(new_file_entry);
    free(file_data_ptr);
    return 0;
  }
  
  free(file_data_ptr); // Data has been copied to new_file_entry, so free the temporary buffer

  if (add_file(new_file_entry) == 0) {
    printf("[ERROR] SEND failed\n");
    free_file(new_file_entry);
    return 0;
  }
  
  printf("[INFO] File received: %s\n", filename_buffer);
  return 1;
}