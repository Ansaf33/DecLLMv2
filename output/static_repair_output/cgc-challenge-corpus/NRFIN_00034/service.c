#include <stdlib.h> // For realloc, free, calloc, malloc
#include <string.h> // For strlen, strcat, strncat, memcpy
#include <stdio.h>  // For NULL
#include <stdint.h> // For size_t, uintptr_t

// --- Type definitions for decompiled types ---
typedef unsigned char undefined;
typedef unsigned int  undefined4;
typedef void (*code)(void); // Assuming 'code' is a function pointer type

// --- External declarations (placeholders for compilation) ---
// Assume VFS structure - minimal definition
struct VFS {
    int dummy_field; // Placeholder member, actual structure is unknown from snippet
};

// Global instances
struct VFS vfs;
long crond;     // Global variable for cron directory ID/address
long pwd;       // Global variable for current working directory ID/address

// Placeholder for DAT_00014002 (e.g., a constant string literal)
const char DAT_00014002[] = "/etc";

// Dummy VFS functions for compilation
void vfs_init(struct VFS *vfs_ptr) { (void)vfs_ptr; /* Implementation specific */ }
long create_dir(struct VFS *vfs_ptr, const char *path) { (void)vfs_ptr; (void)path; return 1; } // Return a non-zero ID
char *get_path_from_dir(struct VFS *vfs_ptr, long dir_id) { (void)vfs_ptr; (void)dir_id; return strdup("/dummy/path"); } // Must return dynamically allocated string
long lookup_dir(struct VFS *vfs_ptr, const char *path, int permission) { (void)vfs_ptr; (void)path; (void)permission; return 1; }
long lookup_file(struct VFS *vfs_ptr, const char *path, int permission) { (void)vfs_ptr; (void)path; (void)permission; return 1; }
int read_file(struct VFS *vfs_ptr, int user_id, const char *path, void **data_out) {
    (void)vfs_ptr; (void)user_id; (void)path;
    *data_out = malloc(10); // Dummy data allocation
    if (*data_out) {
        memcpy(*data_out, "dummy data", 10);
        return 0; // Success
    }
    return -1; // Failure
}
int create_file(struct VFS *vfs_ptr, const char *path) { (void)vfs_ptr; (void)path; return 1; }
int write_file(struct VFS *vfs_ptr, int user_id, const char *path, const void *data, size_t size) { (void)vfs_ptr; (void)user_id; (void)path; (void)data; (void)size; return 0; }
int create_symlink(struct VFS *vfs_ptr, int user_id, const char *path, const char *target_path) { (void)vfs_ptr; (void)user_id; (void)path; (void)target_path; return 0; }
int delete_file(struct VFS *vfs_ptr, int user_id, long file_id) { (void)vfs_ptr; (void)user_id; (void)file_id; return 0; }
void vfs_destroy(struct VFS *vfs_ptr) { (void)vfs_ptr; /* Implementation specific */ }

// Dummy Custom I/O functions for compilation
int read_all(int fd, void *buf, size_t count) {
    (void)fd; (void)buf; (void)count;
    // Simulate reading input for commands or path segments
    static int command_sequence_idx = 0;
    static const unsigned int commands[] = {0, 1, 2, 3, 4, 0xbad1abe1, 5}; // Sequence of commands, 0xbad1abe1 to continue, 5 to exit
    static const char *dummy_path_segment = "testfile"; // 8 chars
    static unsigned int dummy_data_size = 10;
    static char dummy_file_content[10] = "123456789";

    if (fd == 0) { // Simulate stdin
        if (count == sizeof(unsigned int)) { // Reading command_code or data_size
            if (command_sequence_idx < sizeof(commands)/sizeof(commands[0])) {
                memcpy(buf, &commands[command_sequence_idx], count);
                command_sequence_idx++;
                return (int)count;
            } else {
                unsigned int exit_cmd = 5; // Default exit command if sequence exhausted
                memcpy(buf, &exit_cmd, count);
                return (int)count;
            }
        } else if (count == 16) { // Reading path_segment
            memset(buf, 0, count);
            strncpy(buf, dummy_path_segment, 16);
            return 16;
        } else if (count == dummy_data_size) { // Reading file data or symlink target
            memcpy(buf, dummy_file_content, count);
            return (int)count;
        }
    }
    return 0; // No bytes read
}
int write_all(int fd, const void *buf, size_t count) {
    (void)fd; (void)buf; (void)count;
    // fprintf(stderr, "write_all(fd=%d, count=%zu, data=%u)\n", fd, count, *(unsigned int*)buf);
    return (int)count; // Simulate success
}

// Dummy Memory management (allocate/deallocate)
int allocate(size_t size, int count, void **mem_ptr_out) {
    *mem_ptr_out = malloc(size * count);
    return (*mem_ptr_out == NULL) ? -1 : 0;
}
void deallocate(void *mem_ptr, size_t size) {
    (void)size; // Size argument is ignored in free, but kept for signature consistency
    free(mem_ptr);
}

// --- CronEntry structure based on observed offsets ---
// This is an educated guess based on the decompiled code's memory access patterns.
// If the actual structure is different, this needs to be adjusted.
typedef struct CronEntry {
    char _padding_0_13[0x14]; // Padding to reach offset 0x14
    int is_file_path;         // Offset 0x14: 0 for direct code, 1 for file path
    char _padding_18_1b[0x1c - (0x14 + sizeof(int))]; // Padding to reach offset 0x1c
    size_t size;              // Offset 0x1c: size of code/file path
    void *data;               // Offset 0x20: pointer to code or file path string
    long next_entry_offset;   // Offset 0x24: value that needs -0x24 to become next CronEntry*
} CronEntry;

// Function: append_to_path
char * append_to_path(char *param_1,char *param_2) {
  size_t len_param1 = strlen(param_1);
  // Allocate space for original path, '/', max 16 chars of param_2, and null terminator.
  // This calculates to len_param1 + 1 (for '/') + 16 (max param_2) + 1 (for '\0').
  char *new_path = (char *)realloc(param_1, len_param1 + 1 + 16 + 1);
  if (new_path == NULL) {
    free(param_1); // As per original code's logic on realloc failure
    return NULL;
  }
  strcat(new_path, "/");
  strncat(new_path, param_2, 16);
  new_path[len_param1 + 1 + 16] = '\0'; // Explicit null termination
  return new_path;
}

// Function: do_cron
void do_cron(void) {
  // Dummy structure for `crond` to allow `*(long *)(crond + 0x20)` to compile.
  // In a real system, `crond` would point to a valid structure containing cron entry pointers.
  static struct {
      char _padding_0_1f[0x20];
      long first_entry_offset_val; // At offset 0x20
  } dummy_crond_struct = { .first_entry_offset_val = 0 }; // Initialize to no entries
  crond = (long)&dummy_crond_struct; // Make `crond` point to the dummy struct

  long first_entry_val = *(long *)(crond + 0x20);
  CronEntry *current_entry = NULL;

  if (first_entry_val != 0) {
    current_entry = (CronEntry *)(first_entry_val - 0x24);
  }

  while (current_entry != NULL) {
    int should_process_entry = 1;
    void *allocated_mem = NULL;
    code func_ptr = NULL; // Use 'code' typedef for function pointers

    CronEntry *entry_data_source = current_entry;

    if (current_entry->is_file_path != 0) { // If it's a file path, look it up first
      long looked_up_ptr_val = lookup_file(&vfs, (const char *)current_entry->data, 1);
      if (looked_up_ptr_val == 0) {
        should_process_entry = 0; // File not found, skip execution of this entry
      } else {
        entry_data_source = (CronEntry *)looked_up_ptr_val; // Use the looked-up entry's metadata
      }
    }

    if (should_process_entry) {
      if (allocate(entry_data_source->size, 1, &allocated_mem) == 0) {
        memcpy(allocated_mem, entry_data_source->data, entry_data_source->size);
        func_ptr = (code)allocated_mem; // Cast to function pointer type
        func_ptr(); // Execute the code
        deallocate(allocated_mem, entry_data_source->size);
      }
    }

    // Move to the next entry in the cron list
    long next_entry_val = current_entry->next_entry_offset;
    if (next_entry_val == 0) {
      current_entry = NULL; // End of list
    } else {
      current_entry = (CronEntry *)(next_entry_val - 0x24); // Calculate address of next entry
    }
  }
}

// Function: do_cd
undefined4 do_cd(void) {
  char path_segment[17]; // 16 chars + null terminator
  undefined4 result = 0xFFFFFFFF; // Initialize to error (0xFFFFFFFF)

  if (read_all(0, path_segment, 16) == 16) {
    path_segment[16] = '\0'; // Ensure null termination
    char *current_path = get_path_from_dir(&vfs, pwd);

    if (current_path != NULL) {
      char *new_full_path = append_to_path(current_path, path_segment);
      // `current_path` is either freed by `append_to_path` on realloc failure,
      // or its memory is incorporated into `new_full_path` on success.
      // So, no separate `free(current_path)` is needed here.
      if (new_full_path != NULL) {
        long dir_id = lookup_dir(&vfs, new_full_path, 1);
        if (dir_id != 0) {
          pwd = dir_id;
          result = 0; // Success
        }
        free(new_full_path); // Always free the dynamically allocated full path
      }
    }
  }
  return result;
}

// Function: do_read
undefined4 do_read(void) {
  char path_segment[17]; // 16 chars + null terminator
  undefined4 result = 0xFFFFFFFF; // Initialize to error
  void *file_data = NULL; // Pointer to store data read from file
  size_t file_size = 0;   // Size of the file

  if (read_all(0, path_segment, 16) == 16) {
    path_segment[16] = '\0'; // Ensure null termination
    char *current_path = get_path_from_dir(&vfs, pwd);

    if (current_path != NULL) {
      char *full_path = append_to_path(current_path, path_segment);
      if (full_path != NULL) {
        long file_metadata_ptr = lookup_file(&vfs, full_path, 1);
        if (file_metadata_ptr != 0) {
          // Assuming `file_metadata_ptr` points to a structure where offset 0x1c holds the file size.
          file_size = *(undefined4 *)(file_metadata_ptr + 0x1c);

          if (read_file(&vfs, 1000, full_path, &file_data) == 0) { // Passing &file_data as output
            write_all(1, file_data, file_size);
            result = 0; // Success
          }
          if (file_data != NULL) { // Assuming `read_file` allocates `file_data`
              free(file_data);
          }
        }
        free(full_path); // Always free the dynamically allocated full path
      }
    }
  }
  return result;
}

// Function: do_write
undefined4 do_write(void) {
  char path_segment[17]; // 16 chars + null terminator
  unsigned int data_size;
  void *data_buffer = NULL;
  undefined4 result = 0xFFFFFFFF; // Initialize to error

  if (read_all(0, path_segment, 16) == 16) {
    path_segment[16] = '\0'; // Ensure null termination

    if (read_all(0, &data_size, sizeof(data_size)) == (int)sizeof(data_size)) {
      if (data_size < 0xff9) { // Max size check (0xff9 = 4089 bytes)
        data_buffer = calloc(1, data_size); // Corrected calloc usage: 1 element of `data_size` bytes
        if (data_buffer != NULL) {
          if (read_all(0, data_buffer, data_size) == (int)data_size) { // Cast `data_size` to int for comparison
            char *current_path = get_path_from_dir(&vfs, pwd);
            if (current_path != NULL) {
              char *full_path = append_to_path(current_path, path_segment);
              if (full_path != NULL) {
                long file_id = create_file(&vfs, full_path);
                if (file_id != 0) { // Check if file creation succeeded
                  if (write_file(&vfs, 1000, full_path, data_buffer, data_size) == 0) {
                    result = 0; // Success
                  }
                }
                free(full_path);
              }
            }
          }
          free(data_buffer); // Always free data buffer
        }
      }
    }
  }
  return result;
}

// Function: do_ln
undefined4 do_ln(void) {
  char path_segment[17]; // 16 chars + null terminator
  unsigned int target_path_len;
  char *target_path_buffer = NULL;
  undefined4 result = 0xFFFFFFFF; // Initialize to error

  if (read_all(0, path_segment, 16) == 16) {
    path_segment[16] = '\0'; // Ensure null termination

    if (read_all(0, &target_path_len, sizeof(target_path_len)) == (int)sizeof(target_path_len)) {
      if (target_path_len < 0xff9) { // Max length check
        target_path_buffer = (char *)calloc(target_path_len + 1, 1); // +1 for null terminator
        if (target_path_buffer != NULL) {
          if (read_all(0, target_path_buffer, target_path_len) == (int)target_path_len) { // Cast for comparison
            target_path_buffer[target_path_len] = '\0'; // Ensure null termination

            char *current_path = get_path_from_dir(&vfs, pwd);
            if (current_path != NULL) {
              char *symlink_path = append_to_path(current_path, path_segment);
              if (symlink_path != NULL) {
                if (create_symlink(&vfs, 1000, symlink_path, target_path_buffer) == 0) {
                  result = 0; // Success
                }
                free(symlink_path);
              }
            }
          }
          free(target_path_buffer); // Always free target path buffer
        }
      }
    }
  }
  return result;
}

// Function: do_rm
undefined4 do_rm(void) {
  char path_segment[17]; // 16 chars + null terminator
  undefined4 result = 0xFFFFFFFF; // Initialize to error

  if (read_all(0, path_segment, 16) == 16) {
    path_segment[16] = '\0'; // Ensure null termination
    char *current_path = get_path_from_dir(&vfs, pwd);

    if (current_path != NULL) {
      char *full_path = append_to_path(current_path, path_segment);
      if (full_path != NULL) {
        long file_id_to_delete = lookup_file(&vfs, full_path, 1);
        if (file_id_to_delete != 0) { // Check if file exists
          if (delete_file(&vfs, 1000, file_id_to_delete) == 0) {
            result = 0; // Success
          }
        }
        free(full_path); // Always free the dynamically allocated full path
      }
    }
  }
  return result;
}

// Function: main
undefined4 main(void) {
  vfs_init(&vfs);
  create_dir(&vfs, DAT_00014002); // Typically "/etc"
  crond = create_dir(&vfs, "/etc/crond");
  create_dir(&vfs, "/home");
  pwd = create_dir(&vfs, "/home/user");

  // Dummy structure for `pwd` to allow `*(undefined4 *)(pwd + 0x10)` to compile.
  // In a real system, `pwd` would point to a valid structure.
  static struct {
      char _padding_0_f[0x10]; // Padding to reach offset 0x10
      undefined4 owner_id;     // At offset 0x10
  } dummy_pwd_struct;
  pwd = (long)&dummy_pwd_struct; // Make `pwd` point to the dummy struct
  *(undefined4 *)(pwd + 0x10) = 1000; // Set owner/permissions for /home/user

  undefined4 command_code;
  undefined4 op_result;

  int running = 1;
  while (running) {
    do_cron();

    int bytes_read;
    // Loop until 4 bytes (size of command_code) are successfully read
    do {
      bytes_read = read_all(0, &command_code, sizeof(command_code));
    } while (bytes_read != (int)sizeof(command_code)); // Cast for comparison

    if (command_code == 0xbad1abe1) {
      // Equivalent to original `goto switchD_00011802_caseD_5;`
      continue;
    }

    // Default result is error (0xFFFFFFFF) if no operation matches or succeeds.
    op_result = 0xFFFFFFFF;

    switch (command_code) {
      case 0:
        op_result = do_cd();
        break;
      case 1:
        op_result = do_read();
        break;
      case 2:
        op_result = do_write();
        break;
      case 3:
        op_result = do_ln();
        break;
      case 4:
        op_result = do_rm();
        break;
      default:
        // Equivalent to original `goto switchD_00011802_caseD_6;`
        running = 0; // Set flag to exit loop
        break;
    }

    if (running) { // Only write result if we are not exiting the program
      write_all(1, &op_result, sizeof(op_result));
    }
  }

  vfs_destroy(&vfs);
  return 0;
}