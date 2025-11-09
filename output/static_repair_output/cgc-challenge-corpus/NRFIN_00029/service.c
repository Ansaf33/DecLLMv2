#include <stdio.h>  // For NULL, size_t, perror
#include <stdlib.h> // For calloc, free
#include <string.h> // For strchr, strncpy, strcmp
#include <unistd.h> // For read, write, ssize_t, STDIN_FILENO, STDOUT_FILENO

// --- Dummy Definitions for Compilation ---
// These would typically be provided by a project's header files or other source files.

// Assuming the structure of a file entry based on observed offsets
typedef struct file_entry {
    char name[16];      // At offset 0x0, assuming for list_files output
    int field_0x10;     // At offset 0x10, used for admin check in write_file
    unsigned int size;  // At offset 0x14, file content size
    void *data;         // At offset 0x18, pointer to file content
    struct file_entry *next; // At offset 0x1c, for linked list traversal
    // Other fields might exist
} file_entry_t;

// Placeholder for a Virtual File System structure
typedef struct vfs_t {
    file_entry_t *root_dir; // Example: pointer to the root directory's file_entry_t
} vfs_t;

// Global VFS instance and special directory handles/IDs
vfs_t vfs;
file_entry_t *pubroot; // Points to the /public/ directory entry (acting as a list head)
int admin;             // An integer identifier for the admin group/user status

// Dummy implementation for read_all (reads until 'count' bytes are read or error/EOF)
ssize_t read_all(int fd, void *buf, size_t count) {
    size_t total_read = 0;
    ssize_t bytes_read;
    char *ptr = (char *)buf;
    while (total_read < count) {
        bytes_read = read(fd, ptr + total_read, count - total_read);
        if (bytes_read <= 0) {
            // Error or EOF
            if (bytes_read < 0) {
                perror("read_all error");
            }
            return bytes_read; // Return -1 for error, 0 for EOF
        }
        total_read += bytes_read;
    }
    return total_read;
}

// Dummy implementation for write_all (writes until 'count' bytes are written or error)
ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    ssize_t bytes_written;
    const char *ptr = (const char *)buf;
    while (total_written < count) {
        bytes_written = write(fd, ptr + total_written, count - total_written);
        if (bytes_written <= 0) {
            // Error
            if (bytes_written < 0) {
                perror("write_all error");
            }
            return bytes_written;
        }
        total_written += bytes_written;
    }
    return total_written;
}

// Dummy utf8_canonicalize function
// Assumes it takes a destination buffer, source string, and max length for the string part (excluding null).
int utf8_canonicalize(char *dest, const char *src, size_t maxlen) {
    strncpy(dest, src, maxlen);
    dest[maxlen] = '\0'; // Ensure null termination
    return 0; // Return 0 for success
}

// Dummy VFS initialization
void vfs_init(vfs_t *vfs_ptr) {
    vfs_ptr->root_dir = NULL;
}

// Dummy create_dir function
file_entry_t *create_dir(vfs_t *vfs_ptr, const char *path) {
    file_entry_t *dir = (file_entry_t *)calloc(1, sizeof(file_entry_t));
    if (dir) {
        strncpy(dir->name, path, sizeof(dir->name) - 1);
        dir->name[sizeof(dir->name) - 1] = '\0';
        // In a real VFS, this would link the directory into the hierarchy.
        // For `pubroot`, it's treated as a list head for public files.
    }
    return dir;
}

// Dummy lookup_file function
file_entry_t *lookup_file(vfs_t *vfs_ptr, const char *path) {
    // A real implementation would traverse the VFS.
    // For simplicity, iterate through files linked from `pubroot`.
    file_entry_t *current = pubroot->next;
    while (current != NULL) {
        // Compare the canonicalized path with the file's name
        // (assuming `name` in `file_entry_t` stores the full canonical path)
        // This is a simplification; a real system would parse path components.
        if (strcmp(path, current->name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL; // File not found
}

// Dummy create_file function
file_entry_t *create_file(vfs_t *vfs_ptr) {
    file_entry_t *new_file = (file_entry_t *)calloc(1, sizeof(file_entry_t));
    if (new_file) {
        // Link the new file into the public directory's list (managed by `pubroot`)
        new_file->next = pubroot->next;
        pubroot->next = new_file;
    }
    return new_file;
}

// Dummy delete_file function
void delete_file(vfs_t *vfs_ptr, file_entry_t *file_ptr) {
    if (file_ptr) {
        // In a real system, also remove from the VFS list.
        // For `pubroot` list:
        file_entry_t *current = pubroot;
        while (current->next != NULL && current->next != file_ptr) {
            current = current->next;
        }
        if (current->next == file_ptr) {
            current->next = file_ptr->next;
        }

        // Only free `data` if it was allocated by `calloc` (not admin-assigned pointer)
        // The check `file_ptr->data != (void *)(*(unsigned int *)file_ptr->name)`
        // comes from the `write_file` vulnerability.
        if (file_ptr->data != (void *)(*(unsigned int *)file_ptr->name)) {
            free(file_ptr->data);
        }
        free(file_ptr);
    }
}

// Dummy VFS destruction
void vfs_destroy(vfs_t *vfs_ptr) {
    file_entry_t *current = pubroot->next;
    while (current) {
        file_entry_t *next_file = current->next; // Store next before deleting current
        // The delete_file function also removes from the list, but we need to iterate manually.
        // It also frees data and the file_entry_t struct itself.
        delete_file(vfs_ptr, current);
        current = next_file;
    }
    free(pubroot); // Free the pubroot directory entry itself
    // If 'admin' was also dynamically allocated, free it here.
}

// --- Original Functions (Refactored) ---

// Function: canonicalize_path
// param_1 (dest_path) is expected to be a buffer of at least 100 bytes.
// param_2 (src_path) is expected to be a filename string.
int canonicalize_path(char *dest_path, const char *src_path) {
    if (strchr(src_path, '/') != NULL) {
        return -1; // Path contains '/', which is disallowed
    }

    // Copy "/public/" prefix (8 chars + null terminator)
    strncpy(dest_path, "/public/", 8);
    dest_path[8] = '\0'; // Ensure null termination

    // Append canonicalized source path.
    // Assuming dest_path is 100 bytes, and "/public/" is 8 bytes.
    // Remaining capacity for filename + its null terminator is 100 - 8 - 1 = 91 bytes.
    utf8_canonicalize(dest_path + 8, src_path, 100 - 8 - 1);
    return 0; // Success
}

// Function: read_file
int read_file(void) {
    char filename_buffer[16]; // Buffer for the filename read from stdin
    char path_buffer[100];    // Buffer for the canonicalized full path
    file_entry_t *file_entry_ptr; // Pointer to the found file entry

    // Read filename (16 bytes)
    if (read_all(STDIN_FILENO, filename_buffer, sizeof(filename_buffer)) != sizeof(filename_buffer)) {
        return -1;
    }
    filename_buffer[sizeof(filename_buffer) - 1] = '\0'; // Ensure null termination

    // Canonicalize the path
    if (canonicalize_path(path_buffer, filename_buffer) != 0) {
        return -1;
    }

    // Lookup the file in the VFS
    file_entry_ptr = lookup_file(&vfs, path_buffer);
    if (file_entry_ptr == NULL) {
        return -1; // File not found
    }

    // Write the file content to stdout
    if (write_all(STDOUT_FILENO, file_entry_ptr->data, file_entry_ptr->size) != file_entry_ptr->size) {
        return -1;
    }

    return 0; // Success
}

// Function: write_file
int write_file(void) {
    char filename_buffer[16];     // Buffer for the filename read from stdin
    unsigned int file_size;       // Size of the file content to be written
    char path_buffer[100];        // Buffer for the canonicalized full path
    file_entry_t *new_file_entry; // Pointer to the newly created file entry
    void *data_ptr;               // Pointer to the allocated data buffer

    // Read filename (16 bytes)
    if (read_all(STDIN_FILENO, filename_buffer, sizeof(filename_buffer)) != sizeof(filename_buffer)) {
        return -1;
    }
    filename_buffer[sizeof(filename_buffer) - 1] = '\0'; // Ensure null termination

    // Read file size (4 bytes)
    if (read_all(STDIN_FILENO, &file_size, sizeof(file_size)) != sizeof(file_size)) {
        return -1;
    }

    // Check file size against a limit (0xff9 = 4089 bytes)
    if (file_size >= 0xff9) {
        return -1;
    }

    // Canonicalize the path
    if (canonicalize_path(path_buffer, filename_buffer) != 0) {
        return -1;
    }

    // Check if a file with this canonicalized path already exists
    if (lookup_file(&vfs, path_buffer) != NULL) {
        return -1; // File already exists, cannot overwrite or create new
    }

    // Create a new file entry in the VFS
    new_file_entry = create_file(&vfs);
    if (new_file_entry == NULL) {
        return -1;
    }
    strncpy(new_file_entry->name, path_buffer, sizeof(new_file_entry->name) - 1);
    new_file_entry->name[sizeof(new_file_entry->name) - 1] = '\0';

    new_file_entry->size = file_size;

    // Handle admin vs. non-admin file creation logic
    if (new_file_entry->field_0x10 == admin) {
        // WARNING: This is a significant security vulnerability from the original code.
        // It allows an admin to set the file's data pointer to an arbitrary address
        // derived from the first 4 bytes of the input filename_buffer.
        new_file_entry->data = (void *)(*(unsigned int *)filename_buffer);
    } else {
        // For non-admin, allocate memory for the file content
        data_ptr = calloc(file_size, 1); // Fix: calloc(nmemb, size) should be calloc(file_size, 1)
        if (data_ptr == NULL) {
            delete_file(&vfs, new_file_entry);
            return -1;
        }
        new_file_entry->data = data_ptr;
    }

    // Read the file content from stdin into the allocated buffer
    if (read_all(STDIN_FILENO, new_file_entry->data, new_file_entry->size) != new_file_entry->size) {
        delete_file(&vfs, new_file_entry);
        return -1;
    }

    return 0; // Success
}

// Function: list_files
int list_files(void) {
    // Assuming `pubroot` acts as a list head, and `pubroot->next` points to the first actual file entry.
    file_entry_t *current_file = pubroot->next;

    while (current_file != NULL) {
        // Original code writes 16 bytes of the file entry.
        // Assuming the first 16 bytes of `file_entry_t` contain the filename or a relevant identifier.
        if (write_all(STDOUT_FILENO, current_file->name, sizeof(current_file->name)) != sizeof(current_file->name)) {
            return -1; // Error writing
        }
        current_file = current_file->next; // Move to the next file in the list
    }
    return 0; // Success
}

// Function: main
int main(void) {
    int command_id;
    int operation_result;

    vfs_init(&vfs);
    pubroot = create_dir(&vfs, "/public/"); // Initialize /public/ directory
    admin = create_dir(&vfs, "/admin") ? 1 : 0; // Example: set admin status to 1 if /admin dir is created, 0 otherwise

    while (1) { // Main loop for command processing
        // Read command ID (4 bytes)
        while (read_all(STDIN_FILENO, &command_id, sizeof(command_id)) != sizeof(command_id)) {
            // Keep trying to read if not enough bytes are received.
            // In a production system, this might have a timeout or error handling.
        }

        if (command_id == -1) { // Exit command
            vfs_destroy(&vfs);
            return 0; // Exit program successfully
        }

        // Dispatch commands and store the operation result.
        // Only commands 0, 1, and 2 lead to an operation_result being written.
        // Any other command ID causes the loop to continue without output.
        if (command_id == 0) { // Read file
            operation_result = read_file();
        } else if (command_id == 1) { // Write file
            operation_result = write_file();
        } else if (command_id == 2) { // List files
            operation_result = list_files();
        } else {
            // Unknown command_id: original behavior was to loop without writing status.
            continue;
        }

        // Write the operation result (4 bytes) to stdout
        write_all(STDOUT_FILENO, &operation_result, sizeof(operation_result));
    }
}