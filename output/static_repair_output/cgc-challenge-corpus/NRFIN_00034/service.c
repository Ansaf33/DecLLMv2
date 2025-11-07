#include <stddef.h> // For size_t
#include <stdlib.h> // For malloc, realloc, free, calloc
#include <string.h> // For strlen, strcat, strncat, memcpy
#include <stdint.h> // For uintptr_t, uint

// --- Forward Declarations and Type Definitions ---

// Assuming undefined4 is int and undefined is char
typedef unsigned int uint;
typedef int undefined4;
typedef char undefined;

// Placeholder for VFS structure
struct VFS {
    // Members specific to the VFS
    int dummy; // Just a placeholder
};

// Function pointer type for cron jobs
typedef void (*code)(void);

// Global VFS instance and directory IDs/pointers
struct VFS vfs;
uintptr_t crond; // Used as a base address for cron entries or an ID that can be cast
uintptr_t pwd;   // Current working directory ID/pointer

// Placeholder for DAT_00014002 (assuming it's the root path string)
const char DAT_00014002[] = "/";

// External function declarations (minimal for compilation)
// Note: Actual signatures might differ based on the VFS implementation.
int allocate(size_t size, int count, void** ptr);
void deallocate(void* ptr, size_t size);
int lookup_file(struct VFS* vfs_ptr, const char* path, int flags);
int lookup_dir(struct VFS* vfs_ptr, const char* path, int flags);
int read_all(int fd, void* buf, size_t count);
int write_all(int fd, const void* buf, size_t count);
char* get_path_from_dir(struct VFS* vfs_ptr, uintptr_t dir_id);
int create_file(struct VFS* vfs_ptr, const char* path);
int write_file(struct VFS* vfs_ptr, int user_id, const char* path, const void* data, size_t size);
// Assuming read_file takes a pointer to a pointer (for allocated data) and the max size to read.
// It returns 0 on success, non-zero on error. The caller is responsible for freeing *data_buf_ptr.
int read_file(struct VFS* vfs_ptr, int user_id, const char* path, void** data_buf_ptr, size_t max_size);
// Original delete_file takes `int file_entry_id` (local_18 from lookup_file)
int delete_file(struct VFS* vfs_ptr, int user_id, int file_entry_id);
int create_symlink(struct VFS* vfs_ptr, int user_id, const char* link_path, const char* target_path);
void vfs_init(struct VFS* vfs_ptr);
void vfs_destroy(struct VFS* vfs_ptr);
uintptr_t create_dir(struct VFS* vfs_ptr, const char* path);

// For do_cron's internal structure interpretation
// This structure reflects the memory offsets used in the original assembly.
typedef struct CronJobEntry {
    char _padding_0_13[0x14]; // Padding to reach 'type' at 0x14
    int type; // At offset 0x14
    char _padding_18_1b[0x4]; // Padding to reach 'data_size' at 0x1C
    size_t data_size; // At offset 0x1C
    void* data_ptr; // At offset 0x20
    uintptr_t next_raw_ptr_field; // At offset 0x24, this stores the address of the *next* entry's (+0x24) field
} CronJobEntry;

// For do_read's internal structure interpretation of file metadata
// This structure reflects the memory offsets used in the original assembly for file_id.
typedef struct FileEntry {
    char _padding_0_1b[0x1c]; // Padding to reach data_size
    size_t data_size; // At offset 0x1c
    // ... other fields
} FileEntry;

// --- Function Implementations ---

// Function: append_to_path
char * append_to_path(char *param_1, char *param_2) {
  size_t current_len = strlen(param_1);
  // Allocate space for current_len + '/' + param_2 (max 16 chars) + '\0'
  // current_len + 1 (for '/') + 16 (for param_2) + 1 (for '\0') = current_len + 18
  char *new_path = (char *)realloc(param_1, current_len + 18);
  
  if (new_path == NULL) {
    // If realloc fails, param_1 is not freed by realloc, so we must free it.
    free(param_1); 
    return NULL;
  }
  
  strcat(new_path, "/");
  strncat(new_path, param_2, 16);
  new_path[current_len + 1 + 16] = '\0'; // Ensure null termination after strncat
  
  return new_path;
}

// Function: do_cron
void do_cron(void) {
    // The original code uses `*(int *)(crond + 0x20)` to get the initial pointer.
    // `crond` is `uintptr_t`. We cast it to `char*` to do byte-level arithmetic.
    uintptr_t current_entry_raw_ptr = *(uintptr_t*)((char*)crond + 0x20);

    while (current_entry_raw_ptr != 0) {
        // Adjust the raw pointer to the actual start of the CronJobEntry struct
        CronJobEntry* current_job = (CronJobEntry*)(current_entry_raw_ptr - 0x24);

        CronJobEntry* job_source = current_job; // Default source for data_size, data_ptr
        int should_execute = 0;

        if (current_job->type == 0) {
            should_execute = 1;
        } else {
            // If type != 0, it tries to lookup a file.
            // lookup_file returns an int (ID), which is then treated as a pointer to a CronJobEntry-like struct.
            int lookup_result_id = lookup_file(&vfs, current_job->data_ptr, 1);
            if (lookup_result_id != 0) {
                // If lookup_file succeeds, use the returned ID/pointer as the source for the job details.
                job_source = (CronJobEntry*)(uintptr_t)lookup_result_id;
                should_execute = 1;
            }
        }

        if (should_execute) {
            void* allocated_mem = NULL;
            // The original code accesses `data_size` and `data_ptr` from `job_source`.
            int alloc_result = allocate(job_source->data_size, 1, &allocated_mem);

            if (alloc_result == 0 && allocated_mem != NULL) {
                memcpy(allocated_mem, job_source->data_ptr, job_source->data_size);
                code func_ptr = (code)allocated_mem;
                func_ptr();
                deallocate(allocated_mem, job_source->data_size);
            }
        }
        // Move to the next entry using the raw pointer stored in the current job.
        current_entry_raw_ptr = current_job->next_raw_ptr_field;
    }
}

// Function: do_cd
undefined4 do_cd(void) {
    char path_segment[17]; // Max 0x10 chars + null terminator
    path_segment[16] = '\0'; // Ensure null termination

    if (read_all(0, path_segment, 16) != 16) {
        return 0xffffffff;
    }

    char* full_path = get_path_from_dir(&vfs, pwd);
    if (full_path == NULL) {
        return 0xffffffff;
    }

    char* new_full_path = append_to_path(full_path, path_segment);
    if (new_full_path == NULL) {
        // append_to_path frees `full_path` on failure
        return 0xffffffff;
    }

    int dir_id = lookup_dir(&vfs, new_full_path, 1);
    if (dir_id != 0) {
        pwd = (uintptr_t)dir_id;
        free(new_full_path);
        return 0;
    }

    free(new_full_path);
    return 0xffffffff;
}

// Function: do_read
undefined4 do_read(void) {
    char path_segment[17]; // Max 0x10 chars + null terminator
    path_segment[16] = '\0'; // Ensure null termination

    if (read_all(0, path_segment, 16) != 16) {
        return 0xffffffff;
    }

    char* full_path = get_path_from_dir(&vfs, pwd);
    if (full_path == NULL) {
        return 0xffffffff;
    }

    char* new_full_path = append_to_path(full_path, path_segment);
    if (new_full_path == NULL) {
        // append_to_path frees `full_path` on failure
        return 0xffffffff;
    }

    int file_id = lookup_file(&vfs, new_full_path, 1);
    if (file_id == 0) {
        free(new_full_path);
        return 0xffffffff;
    }

    // `file_id` (local_18) is treated as a pointer to a FileEntry-like struct to get data_size.
    FileEntry* file_metadata = (FileEntry*)(uintptr_t)file_id;
    size_t file_size = file_metadata->data_size;

    void* read_buffer = NULL;
    // Assuming read_file allocates `read_buffer` and returns 0 on success.
    // The caller is responsible for freeing `read_buffer`.
    if (read_file(&vfs, 1000, new_full_path, &read_buffer, file_size) == 0) {
        if (read_buffer != NULL) {
            write_all(1, read_buffer, file_size);
            free(read_buffer); // Free buffer allocated by read_file
            free(new_full_path);
            return 0;
        }
    }
    
    free(new_full_path);
    return 0xffffffff;
}

// Function: do_write
undefined4 do_write(void) {
    char path_segment[17]; // Max 0x10 chars + null terminator
    path_segment[16] = '\0'; // Ensure null termination

    if (read_all(0, path_segment, 16) != 16) {
        return 0xffffffff;
    }

    uint data_size;
    if (read_all(0, &data_size, sizeof(data_size)) != sizeof(data_size)) {
        return 0xffffffff;
    }

    if (data_size >= 0xff9) { // Check against 0xff9 (4089)
        return 0xffffffff;
    }

    void* data_buffer = calloc(data_size, 1); // calloc(num_elements, element_size)
    if (data_buffer == NULL) {
        return 0xffffffff;
    }

    if (read_all(0, data_buffer, data_size) != data_size) {
        free(data_buffer);
        return 0xffffffff;
    }

    char* full_path = get_path_from_dir(&vfs, pwd);
    if (full_path == NULL) {
        free(data_buffer);
        return 0xffffffff;
    }

    char* new_full_path = append_to_path(full_path, path_segment);
    if (new_full_path == NULL) {
        // append_to_path frees `full_path` on failure
        free(data_buffer);
        return 0xffffffff;
    }

    int created_file_id = create_file(&vfs, new_full_path);
    if (created_file_id != 0) { // create_file returns 0 on error, non-zero ID on success
        if (write_file(&vfs, 1000, new_full_path, data_buffer, data_size) == 0) {
            free(new_full_path);
            free(data_buffer);
            return 0; // Success
        }
    }
    
    free(new_full_path);
    free(data_buffer);
    return 0xffffffff;
}

// Function: do_ln
undefined4 do_ln(void) {
  char path_segment[17]; // Max 0x10 chars + null terminator
  path_segment[16] = '\0'; // Ensure null termination

  if (read_all(0, path_segment, 16) != 16) {
    return 0xffffffff;
  }

  uint target_path_size;
  if (read_all(0, &target_path_size, sizeof(target_path_size)) != sizeof(target_path_size)) {
    return 0xffffffff;
  }

  if (target_path_size >= 0xff9) { // Check against 0xff9 (4089)
    return 0xffffffff;
  }

  // Allocate for target path + null terminator
  char* target_path_buffer = (char*)calloc(target_path_size + 1, 1);
  if (target_path_buffer == NULL) {
    return 0xffffffff;
  }

  if (read_all(0, target_path_buffer, target_path_size) != target_path_size) {
    free(target_path_buffer);
    return 0xffffffff;
  }
  target_path_buffer[target_path_size] = '\0'; // Ensure null termination

  char* full_link_path = get_path_from_dir(&vfs, pwd);
  if (full_link_path == NULL) {
    free(target_path_buffer);
    return 0xffffffff;
  }

  char* new_full_link_path = append_to_path(full_link_path, path_segment);
  if (new_full_link_path == NULL) {
    // append_to_path frees `full_link_path` on failure
    free(target_path_buffer);
    return 0xffffffff;
  }

  if (create_symlink(&vfs, 1000, new_full_link_path, target_path_buffer) == 0) {
    free(new_full_link_path);
    free(target_path_buffer);
    return 0; // Success
  }
  
  free(new_full_link_path);
  free(target_path_buffer);
  return 0xffffffff;
}

// Function: do_rm
undefined4 do_rm(void) {
    char path_segment[17]; // Max 0x10 chars + null terminator
    path_segment[16] = '\0'; // Ensure null termination

    if (read_all(0, path_segment, 16) != 16) {
        return 0xffffffff;
    }

    char* full_path = get_path_from_dir(&vfs, pwd);
    if (full_path == NULL) {
        return 0xffffffff;
    }

    char* new_full_path = append_to_path(full_path, path_segment);
    if (new_full_path == NULL) {
        // append_to_path frees `full_path` on failure
        return 0xffffffff;
    }

    int file_entry_id = lookup_file(&vfs, new_full_path, 1);
    if (file_entry_id == 0) {
        free(new_full_path);
        return 0xffffffff;
    }

    if (delete_file(&vfs, 1000, file_entry_id) == 0) {
        free(new_full_path);
        return 0; // Success
    }
    
    free(new_full_path);
    return 0xffffffff;
}

// Function: main
undefined4 main(void) {
    vfs_init(&vfs);
    create_dir(&vfs, DAT_00014002); // DAT_00014002 decays to const char*
    crond = create_dir(&vfs, "/etc/crond");
    create_dir(&vfs, "/home");
    pwd = create_dir(&vfs, "/home/user");
    // Assuming pwd is a base address for a struct, and 0x10 is an offset.
    // This looks like setting a user ID or permissions for the /home/user directory.
    *(undefined4 *)((char*)pwd + 0x10) = 1000; 

    int running = 1;
    while (running) {
        do_cron();
        
        uint command_code;
        int bytes_read = 0;
        int total_bytes_read = 0;
        char *ptr = (char*)&command_code;

        // Loop to ensure exactly 4 bytes are read for the command code
        while (total_bytes_read < sizeof(command_code)) {
            bytes_read = read_all(0, ptr + total_bytes_read, sizeof(command_code) - total_bytes_read);
            if (bytes_read <= 0) { // EOF or error
                running = 0; // Exit main loop
                break;
            }
            total_bytes_read += bytes_read;
        }
        if (!running) {
            break; // Exit if read failed
        }

        undefined4 result = 0xffffffff; // Default error
        switch(command_code) {
            case 0:
                result = do_cd();
                break;
            case 1:
                result = do_read();
                break;
            case 2:
                result = do_write();
                break;
            case 3:
                result = do_ln();
                break;
            case 4:
                result = do_rm();
                break;
            case 0xbad1abe1:
                // Original code effectively `continue`s the main loop.
                continue;
            default:
                running = 0; // Exit main loop
                break;
        }

        if (running) { // Only write result if we are not exiting
            write_all(1, &result, sizeof(result));
        }
    }

    vfs_destroy(&vfs);
    return 0;
}