#include <stdio.h>   // For NULL, stderr, fprintf
#include <stdlib.h>  // For malloc, calloc, free
#include <string.h>  // For strchr, strncpy, strlen, strncmp
#include <unistd.h>  // For STDIN_FILENO, STDOUT_FILENO, ssize_t, read, write
#include <stdint.h>  // For uint32_t, uintptr_t

// --- Dummy VFS Structures and Functions (to make code compilable) ---
// These are simplified to match the access patterns of the original snippet.
// In a real VFS, these would be more robust and complex.

// File entry structure. Also used for directory entries.
// The offsets are chosen to match the original decompiled code's access patterns.
typedef struct file_entry {
    char name[16];          // Offset 0x00: File name (or directory name)
    char _padding_0x10[0x10 - 0x0C]; // Padding to align owner_id if needed
    int owner_id;           // Offset 0x10: Owner ID (used for admin check)
    size_t size;            // Offset 0x14: Size of file data
    void *data;             // Offset 0x18: Pointer to file data
    struct file_entry *next; // Offset 0x1C: Next file/directory in a linked list
} file_entry_t;

// VFS structure (simplified: a linked list of root directories)
typedef struct {
    file_entry_t *root_dirs_head; // Head of a linked list of top-level directories
} vfs_t;

// Global VFS instance and special directory pointers/IDs
vfs_t vfs;
file_entry_t *pubroot = NULL; // Pointer to the "/public/" directory entry
file_entry_t *admin_dir = NULL; // Pointer to the "/admin" directory entry
int ADMIN_UID = 1; // Arbitrary admin user ID for owner_id checks

// Dummy function implementations for VFS operations
void vfs_init(vfs_t *fs) {
    fs->root_dirs_head = NULL;
}

// Helper to free all files within a directory entry's linked list
static void free_file_list(file_entry_t *head) {
    file_entry_t *current = head;
    while (current) {
        file_entry_t *next_file = current->next;
        // Only free data if it was dynamically allocated and not the special admin case
        if (current->owner_id != ADMIN_UID || current->data != (void*)(uintptr_t)current->name) {
            free(current->data);
        }
        free(current);
        current = next_file;
    }
}

void vfs_destroy(vfs_t *fs) {
    file_entry_t *current_dir = fs->root_dirs_head;
    while (current_dir) {
        file_entry_t *next_dir = current_dir->next; // Save next directory before freeing current
        // Free files linked from this directory entry's 'data' or 'next'
        // Based on `list_files`, files are linked via `dir_entry->next`.
        free_file_list(current_dir->next); // Free files within this directory

        // Free the directory entry itself
        free(current_dir);
        current_dir = next_dir;
    }
    fs->root_dirs_head = NULL;
}

// Creates a directory entry (which is a file_entry_t) and adds it to the VFS root_dirs_head
file_entry_t *create_dir(vfs_t *fs, const char *path) {
    file_entry_t *new_dir = (file_entry_t *)calloc(1, sizeof(file_entry_t));
    if (!new_dir) return NULL;
    strncpy(new_dir->name, path, sizeof(new_dir->name) - 1);
    new_dir->name[sizeof(new_dir->name) - 1] = '\0';
    new_dir->owner_id = ADMIN_UID; // Directories might be default owned by admin

    // Link this new directory into the VFS's global list of root directories
    new_dir->next = fs->root_dirs_head;
    fs->root_dirs_head = new_dir;
    return new_dir;
}

// Looks up a file or directory by its canonical path.
// Returns a pointer to the file_entry_t if found, NULL otherwise.
file_entry_t *lookup_file(vfs_t *fs, const char *path) {
    if (path[0] != '/') return NULL; // Must be an absolute path

    char dir_name[sizeof(((file_entry_t*)0)->name)];
    char file_name[sizeof(((file_entry_t*)0)->name)];
    file_name[0] = '\0'; // Initialize to empty

    const char *first_slash = strchr(path + 1, '/'); // Find the slash after the root directory name

    // Extract directory name
    size_t dir_name_len;
    if (first_slash) {
        dir_name_len = first_slash - path;
    } else {
        dir_name_len = strlen(path); // Path refers to a directory directly, e.g., "/public/"
    }
    if (dir_name_len == 0 || dir_name_len >= sizeof(dir_name)) return NULL;
    strncpy(dir_name, path, dir_name_len);
    dir_name[dir_name_len] = '\0';

    // Find the target directory
    file_entry_t *target_dir = NULL;
    file_entry_t *current_dir_search = fs->root_dirs_head;
    while (current_dir_search) {
        if (strncmp(current_dir_search->name, dir_name, sizeof(dir_name)) == 0) {
            target_dir = current_dir_search;
            break;
        }
        current_dir_search = current_dir_search->next;
    }
    if (!target_dir) return NULL;

    // If path refers to a directory itself, return it
    if (!first_slash) {
        return target_dir;
    }

    // Extract file name
    const char *file_name_start = first_slash + 1;
    strncpy(file_name, file_name_start, sizeof(file_name) - 1);
    file_name[sizeof(file_name) - 1] = '\0';

    // Search for the file within this directory's linked list of files
    // Files are linked using the directory's `next` pointer as the head of the file list.
    file_entry_t *current_file = target_dir->next;
    while (current_file) {
        if (strncmp(current_file->name, file_name, sizeof(file_name)) == 0) {
            return current_file;
        }
        current_file = current_file->next;
    }
    return NULL;
}

// Creates and returns an unlinked file_entry_t. `write_file` is responsible for linking it.
file_entry_t *create_file(vfs_t *fs) {
    file_entry_t *new_file = (file_entry_t *)calloc(1, sizeof(file_entry_t));
    if (!new_file) return NULL;
    new_file->owner_id = 0; // Default owner ID
    // Note: The new file is NOT linked into any directory list here.
    // `write_file` will handle setting its name, data, and linking it.
    return new_file;
}

// Deletes a specific file_entry_t from its directory's list and frees its memory.
void delete_file(vfs_t *fs, file_entry_t *file_to_delete) {
    if (!file_to_delete) return;

    // Iterate through root directories to find which one contains the file
    file_entry_t *current_dir = fs->root_dirs_head;
    while (current_dir) {
        // Files are linked off the directory's `next` pointer
        file_entry_t **current_file_ptr = &current_dir->next;
        while (*current_file_ptr) {
            if (*current_file_ptr == file_to_delete) {
                *current_file_ptr = file_to_delete->next; // Unlink the file
                // Free data if it was dynamically allocated and not the admin special case
                if (file_to_delete->owner_id != ADMIN_UID || file_to_delete->data != (void*)(uintptr_t)file_to_delete->name) {
                    free(file_to_delete->data);
                }
                free(file_to_delete); // Free the file entry itself
                return;
            }
            current_file_ptr = &(*current_file_ptr)->next;
        }
        current_dir = current_dir->next; // Move to the next root directory
    }
}

// Placeholder for UTF-8 canonicalization
void utf8_canonicalize(char *dest, const char *src, size_t max_len) {
    // In a real system, this would normalize UTF-8 paths.
    // For this exercise, it's a simple string copy with length limit.
    strncpy(dest, src, max_len - 1);
    dest[max_len - 1] = '\0';
}

// Wrapper for `read` to ensure all `count` bytes are read.
ssize_t read_all(int fd, void *buf, size_t count) {
    size_t total_read = 0;
    while (total_read < count) {
        ssize_t bytes_read = read(fd, (char *)buf + total_read, count - total_read);
        if (bytes_read == 0) return total_read; // EOF
        if (bytes_read == -1) return -1; // Error
        total_read += bytes_read;
    }
    return total_read;
}

// Wrapper for `write` to ensure all `count` bytes are written.
ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    while (total_written < count) {
        ssize_t bytes_written = write(fd, (const char *)buf + total_written, count - total_written);
        if (bytes_written == -1) return -1; // Error
        total_written += bytes_written;
    }
    return total_written;
}

// --- End Dummy VFS Structures and Functions ---

// Function: canonicalize_path
int canonicalize_path(char *dest_path, const char *input_path) {
    if (strchr(input_path, '/') == NULL) {
        strncpy(dest_path, "/public/", 9); // "/public/" + null terminator = 9 bytes
        dest_path[8] = '\0'; // Ensure null termination
        // Remaining space for utf8_canonicalize is 100 - 8 = 92 bytes.
        utf8_canonicalize(dest_path + 8, input_path, 100 - 8);
        return 0; // Success
    }
    return -1; // Failure: input_path contains a '/'
}

// Function: read_file
int read_file(void) {
    char filename_buf[sizeof(((file_entry_t*)0)->name) + 1]; // 16 + 1 for null terminator
    char canonical_path_buf[100];
    
    if (read_all(STDIN_FILENO, filename_buf, sizeof(filename_buf) - 1) != (sizeof(filename_buf) - 1)) {
        return -1;
    }
    filename_buf[sizeof(filename_buf) - 1] = '\0'; // Null terminate

    if (canonicalize_path(canonical_path_buf, filename_buf) != 0) {
        return -1;
    }

    file_entry_t *file_entry = lookup_file(&vfs, canonical_path_buf);
    if (file_entry == NULL) {
        return -1; // File not found
    }

    // Write file content to stdout
    if (write_all(STDOUT_FILENO, file_entry->data, file_entry->size) != file_entry->size) {
        return -1; // Failed to write all bytes
    }

    return 0; // Success
}

// Function: write_file
int write_file(void) {
    char filename_buf[sizeof(((file_entry_t*)0)->name) + 1];
    uint32_t file_size;
    char canonical_path_buf[100];
    file_entry_t *file_entry = NULL;

    if (read_all(STDIN_FILENO, filename_buf, sizeof(filename_buf) - 1) != (sizeof(filename_buf) - 1)) {
        return -1;
    }
    filename_buf[sizeof(filename_buf) - 1] = '\0'; // Null terminate

    if (read_all(STDIN_FILENO, &file_size, sizeof(file_size)) != sizeof(file_size)) {
        return -1;
    }

    // Max size check (0xff9 = 4089)
    if (file_size >= 4089) {
        return -1;
    }

    if (canonicalize_path(canonical_path_buf, filename_buf) != 0) {
        return -1;
    }

    // Check if file already exists
    if (lookup_file(&vfs, canonical_path_buf) != NULL) {
        return -1; // File already exists
    }

    file_entry = create_file(&vfs);
    if (file_entry == NULL) {
        return -1;
    }

    // Set file properties
    strncpy(file_entry->name, filename_buf, sizeof(file_entry->name) - 1);
    file_entry->name[sizeof(file_entry->name) - 1] = '\0';
    file_entry->size = file_size;

    // Special handling for admin-owned files (original code vulnerability)
    // If owner is admin, data pointer is set to filename_buf (a local stack variable).
    // This is a use-after-free vulnerability. Replicating original logic.
    if (file_entry->owner_id == ADMIN_UID) { // Assuming ADMIN_UID is the actual ID.
        file_entry->data = (void *)(uintptr_t)filename_buf; // Points to stack variable
    } else {
        file_entry->data = calloc(file_size, 1); // Corrected calloc usage: allocate file_size bytes
        if (file_entry->data == NULL) {
            delete_file(&vfs, file_entry);
            return -1;
        }
    }

    // Read file content from stdin
    if (read_all(STDIN_FILENO, file_entry->data, file_entry->size) != file_entry->size) {
        delete_file(&vfs, file_entry); // Clean up on failure
        return -1;
    }

    // Link the new file into the correct directory's list
    char dir_name_for_linking[sizeof(((file_entry_t*)0)->name)];
    const char *first_slash_for_linking = strchr(canonical_path_buf + 1, '/');
    if (!first_slash_for_linking) {
        delete_file(&vfs, file_entry);
        return -1; // Canonical path for a file should have a slash after dir.
    }
    size_t dir_name_len_for_linking = first_slash_for_linking - canonical_path_buf;
    if (dir_name_len_for_linking >= sizeof(dir_name_for_linking)) {
        delete_file(&vfs, file_entry);
        return -1;
    }
    strncpy(dir_name_for_linking, canonical_path_buf, dir_name_len_for_linking);
    dir_name_for_linking[dir_name_len_for_linking] = '\0';

    file_entry_t *target_dir_for_linking = NULL;
    file_entry_t *current_dir_search = vfs.root_dirs_head;
    while (current_dir_search) {
        if (strncmp(current_dir_search->name, dir_name_for_linking, sizeof(dir_name_for_linking)) == 0) {
            target_dir_for_linking = current_dir_search;
            break;
        }
        current_dir_search = current_dir_search->next;
    }

    if (!target_dir_for_linking) {
        delete_file(&vfs, file_entry);
        return -1;
    }

    // Link the new file into the target directory's list (via target_dir->next)
    file_entry->next = target_dir_for_linking->next;
    target_dir_for_linking->next = file_entry;

    return 0; // Success
}

// Function: list_files
int list_files(void) {
    // pubroot is a file_entry_t* acting as a directory.
    // Its `next` pointer points to the head of the file list within "/public/".
    file_entry_t *current_entry = pubroot->next;

    while (current_entry != NULL) {
        // Write 16 bytes of the file entry (presumably the filename)
        if (write_all(STDOUT_FILENO, current_entry->name, sizeof(current_entry->name)) != sizeof(current_entry->name)) {
            return -1; // Failure
        }
        current_entry = current_entry->next; // Move to next entry
    }
    return 0; // Success
}

// Function: main
int main(void) {
    int command_code;
    int result;

    vfs_init(&vfs);
    pubroot = create_dir(&vfs, "/public/");
    admin_dir = create_dir(&vfs, "/admin"); // Renamed from 'admin' to avoid ambiguity with ADMIN_UID

    if (pubroot == NULL || admin_dir == NULL) {
        fprintf(stderr, "Failed to initialize VFS directories.\n");
        vfs_destroy(&vfs);
        return 1; // Indicate failure
    }

    while (1) {
        ssize_t bytes_read = read_all(STDIN_FILENO, &command_code, sizeof(command_code));
        if (bytes_read != sizeof(command_code)) {
            // If not exactly 4 bytes, assume EOF or error and terminate.
            break;
        }

        if (command_code == -1) { // Exit command
            break;
        }

        result = -1; // Default to failure for unhandled commands

        if (command_code == 0) { // Read file
            result = read_file();
        } else if (command_code == 1) { // Write file
            result = write_file();
        } else if (command_code == 2) { // List files
            result = list_files();
        }
        // For any other command_code, `result` remains -1.

        write_all(STDOUT_FILENO, &result, sizeof(result));
    }

    vfs_destroy(&vfs);
    return 0;
}