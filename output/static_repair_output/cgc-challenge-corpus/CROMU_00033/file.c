#include <stdio.h>   // For printf, fprintf, stderr
#include <stdlib.h>  // For malloc, free, realloc, exit
#include <string.h>  // For strlen, memcpy, memset, strcmp
#include <stdbool.h> // For bool type (true/false)

// Define the File structure and constants
#define MAX_NAME_LEN 256
#define MAX_DATA_SIZE 1024 // Corresponds to 0x400 in the original snippet

// File structure to represent both files and directories
typedef struct File {
    char name[MAX_NAME_LEN]; // Name of the file or directory
    int type; // 0 for file, 1 for directory

    // Use a union to manage data specific to files or sub-files for directories.
    // This correctly models the memory layout implied by the original code's offsets.
    union {
        struct {
            int data_size; // Size of the file's data (for type 0)
            void *data;    // Pointer to the file's data (for type 0)
        } file_data;
        struct {
            unsigned int sub_file_count; // Number of sub-files/directories (for type 1)
            struct File **sub_files;     // Array of pointers to sub-files/directories (for type 1)
        } dir_data;
    };
} File;

// Global root directory pointer
File *root;

// Forward declarations for functions
File *init_file(void);
size_t set_name(File *file, const char *name);
int set_type(File *file, int type_val);
int fixup_dir_length(File *dir);
int does_sub_file_exist(File *dir, const char *filename);
File *retrieve_sub(File *dir, const char *filename);
void free_file(File *file);
int bubble_sort(File *dir);
int find_next_slash(const char *path, unsigned int start_idx, size_t path_len);


// Function to initialize the file system, creating the root directory
void init_filesystem() {
    root = init_file();
    if (root == NULL) {
        fprintf(stderr, "Failed to initialize root directory!\n");
        exit(EXIT_FAILURE);
    }
    set_name(root, "/");
    set_type(root, 1); // Mark root as a directory
}

// Function to recursively clean up the entire file system tree
void cleanup_filesystem(File *node) {
    if (node == NULL) return;

    if (node->type == 1) { // If it's a directory
        for (unsigned int i = 0; i < node->dir_data.sub_file_count; ++i) {
            if (node->dir_data.sub_files[i] != NULL) {
                cleanup_filesystem(node->dir_data.sub_files[i]); // Recursively free children
            }
        }
        // After all children are freed, free the array of pointers itself
        if (node->dir_data.sub_files != NULL) {
            free(node->dir_data.sub_files);
        }
    } else { // If it's a file
        if (node->file_data.data != NULL) {
            free(node->file_data.data);
        }
    }
    free(node); // Free the File struct itself
}

// Function: bubble_sort (renamed from bubble_sort with appropriate types)
int bubble_sort(File *dir) {
    if (dir == NULL || dir->type != 1 || dir->dir_data.sub_files == NULL || dir->dir_data.sub_file_count == 0) {
        return 0; // Not a valid directory or is empty
    }

    File **sub_files = dir->dir_data.sub_files;
    unsigned int count = dir->dir_data.sub_file_count;
    unsigned int non_null_count = 0;

    // First pass: Count non-NULL file entries
    for (unsigned int i = 0; i < count; ++i) {
        if (sub_files[i] != NULL) {
            non_null_count++;
        }
    }

    // Second pass: Compact non-NULL file entries to the beginning of the array
    if (non_null_count < count) {
        unsigned int current_write_idx = 0;
        for (unsigned int i = 0; i < count; ++i) {
            if (sub_files[i] != NULL) {
                if (current_write_idx != i) { // Only copy if not already in place
                    sub_files[current_write_idx] = sub_files[i];
                }
                current_write_idx++;
            }
        }
        // Fill the remaining slots with NULL
        for (; current_write_idx < count; ++current_write_idx) {
            sub_files[current_write_idx] = NULL;
        }
        dir->dir_data.sub_file_count = non_null_count; // Update the directory's actual count
    }

    // Third pass: Perform bubble sort on the compacted list of sub-files
    count = dir->dir_data.sub_file_count; // Use the updated count for sorting
    for (unsigned int i = 0; i < count; ++i) {
        if (sub_files[i] == NULL) continue; // Should not happen after compaction, but for safety

        for (unsigned int j = i + 1; j < count; ++j) {
            if (sub_files[j] == NULL) continue; // Should not happen after compaction

            // Compare names and swap if they are in the wrong order
            if (strcmp(sub_files[i]->name, sub_files[j]->name) > 0) {
                File *temp = sub_files[j];
                sub_files[j] = sub_files[i];
                sub_files[i] = temp;
            }
        }
    }
    return 1;
}

// Function: remove_sub_file (renamed from remove_sub_file with appropriate types)
int remove_sub_file(File *dir, const char *filename) {
    if (dir == NULL || filename == NULL || dir->type != 1 || dir->dir_data.sub_files == NULL) {
        return 0;
    }

    for (unsigned int i = 0; i < dir->dir_data.sub_file_count; ++i) {
        File *sub_file = dir->dir_data.sub_files[i];
        if (sub_file != NULL && strcmp(sub_file->name, filename) == 0) {
            if (sub_file->type == 1) { // Cannot delete a directory directly
                printf("[ERROR] Cannot delete a directory\n");
                return 0;
            }
            // It's a file, so free its resources and mark its slot as NULL
            free_file(sub_file);
            dir->dir_data.sub_files[i] = NULL; // Mark this slot as empty
            // The bubble_sort function will handle compaction and updating sub_file_count later.
            return 1;
        }
    }
    return 0; // File not found
}

// Function: delete_file (renamed from delete_file with appropriate types)
int delete_file(const char *path) {
    if (path == NULL) {
        return 0;
    }

    if (*path == '/') { // Absolute path
        unsigned int path_start_idx = 1; // Start after the leading slash
        size_t path_len = strlen(path);
        File *current_dir = root;
        int next_slash_idx = 0;
        char path_segment[MAX_NAME_LEN];

        if (path_len >= MAX_NAME_LEN) {
            printf("[ERROR] Path name too long\n");
            return 0;
        }

        while (true) {
            next_slash_idx = find_next_slash(path, path_start_idx, path_len);
            memset(path_segment, 0, MAX_NAME_LEN);

            if (next_slash_idx == -1) { // This is the last segment (the file/dir to delete)
                size_t segment_len = path_len - path_start_idx;
                if (segment_len >= MAX_NAME_LEN) {
                    printf("[ERROR] Size calculation failed for last segment\n");
                    return 0;
                }
                memcpy(path_segment, path + path_start_idx, segment_len);
                path_segment[segment_len] = '\0'; // Null-terminate the segment

                if (does_sub_file_exist(current_dir, path_segment) == 0) {
                    printf("[ERROR] Could not locate %s\n", path);
                    return 0;
                }
                if (remove_sub_file(current_dir, path_segment) != 0) {
                    printf("[INFO] %s removed\n", path_segment);
                    bubble_sort(current_dir); // Sort and compact after removal
                    return 1;
                }
                return 0; // Failed to remove
            }

            // This is an intermediate directory
            size_t segment_len = next_slash_idx - path_start_idx;
            if (segment_len >= MAX_NAME_LEN) {
                printf("[ERROR] Size calculation failed for intermediate segment\n");
                return 0;
            }
            memcpy(path_segment, path + path_start_idx, segment_len);
            path_segment[segment_len] = '\0'; // Null-terminate the segment

            current_dir = retrieve_sub(current_dir, path_segment);
            if (current_dir == NULL) {
                printf("[ERROR] Failed to locate directory %s\n", path_segment);
                return 0;
            }
            if (current_dir->type != 1) { // Ensure it's a directory
                printf("[ERROR] %s is not a directory\n", path_segment);
                return 0;
            }
            path_start_idx = next_slash_idx + 1; // Move past the current slash
        }
    } else { // Relative path (from root)
        if (does_sub_file_exist(root, path) == 0) {
            printf("[ERROR] Could not locate %s\n", path);
            return 0;
        }
        if (remove_sub_file(root, path) != 0) {
            printf("[INFO] %s removed\n", path);
            bubble_sort(root); // Sort and compact after removal
            return 1;
        }
        return 0; // Failed to remove
    }
    return 0; // Should ideally not be reached
}

// Function: retrieve_sub (renamed from retrieve_sub with appropriate types)
File *retrieve_sub(File *dir, const char *filename) {
    if (dir == NULL || filename == NULL || dir->type != 1 || dir->dir_data.sub_files == NULL) {
        return NULL;
    }

    for (unsigned int i = 0; i < dir->dir_data.sub_file_count; ++i) {
        File *sub_file = dir->dir_data.sub_files[i];
        if (sub_file != NULL && strcmp(sub_file->name, filename) == 0) {
            return sub_file;
        }
    }
    return NULL; // Sub-file not found
}

// Function: find_next_slash (renamed from find_next_slash with appropriate types)
int find_next_slash(const char *path, unsigned int start_idx, size_t path_len) {
    if (path == NULL) {
        return -1;
    }
    for (unsigned int i = start_idx; i < path_len; ++i) {
        if (path[i] == '/') {
            return i;
        }
    }
    return -1; // No slash found
}

// Function: fixup_dir_length (renamed from fixup_dir_length with appropriate types)
int fixup_dir_length(File *dir) {
    if (dir == NULL || dir->type != 1) {
        return 0;
    }

    unsigned int old_count = dir->dir_data.sub_file_count;
    unsigned int new_count = old_count + 1;

    // Use realloc to resize the array of sub-file pointers.
    // realloc(NULL, size) behaves like malloc(size) for initial allocation.
    File **new_sub_files = (File **)realloc(dir->dir_data.sub_files, new_count * sizeof(File *));
    if (new_sub_files == NULL) {
        printf("[ERROR] Failed to reallocate memory for sub_files\n");
        return 0;
    }

    dir->dir_data.sub_files = new_sub_files;
    dir->dir_data.sub_file_count = new_count;
    dir->dir_data.sub_files[old_count] = NULL; // Initialize the newly added slot to NULL

    return 1;
}

// Function: get_file (renamed from get_file with appropriate types)
File *get_file(const char *path) {
    if (path == NULL) {
        return NULL;
    }

    if (*path == '/') { // Absolute path
        unsigned int path_start_idx = 1;
        size_t path_len = strlen(path);
        File *current_dir = root;
        int next_slash_idx = 0;
        char path_segment[MAX_NAME_LEN];

        if (path_len >= MAX_NAME_LEN) {
             printf("[ERROR] Path name too long\n");
             return NULL;
        }

        while (true) {
            next_slash_idx = find_next_slash(path, path_start_idx, path_len);
            memset(path_segment, 0, MAX_NAME_LEN);

            if (next_slash_idx == -1) { // This is the last segment (the target file/dir)
                size_t segment_len = path_len - path_start_idx;
                if (segment_len >= MAX_NAME_LEN) {
                    printf("[ERROR] Size calculation failed for last segment\n");
                    return NULL;
                }
                memcpy(path_segment, path + path_start_idx, segment_len);
                path_segment[segment_len] = '\0'; // Null-terminate
                return retrieve_sub(current_dir, path_segment);
            }

            // This is an intermediate directory
            size_t segment_len = next_slash_idx - path_start_idx;
            if (segment_len >= MAX_NAME_LEN) {
                printf("[ERROR] Size calculation failed for intermediate segment\n");
                return NULL;
            }
            memcpy(path_segment, path + path_start_idx, segment_len);
            path_segment[segment_len] = '\0'; // Null-terminate

            current_dir = retrieve_sub(current_dir, path_segment);
            if (current_dir == NULL) {
                return NULL; // Directory not found along the path
            }
            if (current_dir->type != 1) { // Ensure it's a directory
                printf("[ERROR] %s is not a directory\n", path_segment);
                return NULL;
            }
            path_start_idx = next_slash_idx + 1;
        }
    } else { // Relative path (from root)
        return retrieve_sub(root, path);
    }
}

// Function: does_sub_file_exist (renamed from does_sub_file_exist with appropriate types)
int does_sub_file_exist(File *dir, const char *filename) {
    if (dir == NULL || filename == NULL || dir->type != 1) {
        return 0;
    }
    // Leverage retrieve_sub to check for existence
    return retrieve_sub(dir, filename) != NULL;
}

// Function: add_file_to_dir (renamed from add_file_to_dir with appropriate types)
int add_file_to_dir(File *dir, File *new_file) {
    if (dir == NULL || new_file == NULL || dir->type != 1) {
        return 0;
    }

    if (fixup_dir_length(dir) == 0) { // Expand directory capacity
        return 0;
    }
    // Add the new file to the last available slot
    dir->dir_data.sub_files[dir->dir_data.sub_file_count - 1] = new_file;
    bubble_sort(dir); // Sort the directory entries after adding
    return 1;
}

// Function: add_file (renamed from add_file with appropriate types)
int add_file(const char *path) {
    if (path == NULL) {
        return 0;
    }

    if (*path == '/') { // Absolute path
        if (strlen(path) == 1) { // Cannot add the root directory itself
            printf("[ERROR] You cannot add '/'\n");
            return 0;
        }

        unsigned int path_start_idx = 1;
        size_t path_len = strlen(path);
        File *current_dir = root;
        int next_slash_idx = 0;
        char path_segment[MAX_NAME_LEN];

        if (path_len >= MAX_NAME_LEN) {
             printf("[ERROR] Path name too long\n");
             return 0;
        }

        while (true) {
            next_slash_idx = find_next_slash(path, path_start_idx, path_len);
            memset(path_segment, 0, MAX_NAME_LEN);

            if (next_slash_idx == -1) { // This is the last segment (the file/dir to add)
                size_t segment_len = path_len - path_start_idx;
                if (segment_len >= MAX_NAME_LEN) {
                    printf("[ERROR] Size calculation failed for last segment\n");
                    return 0;
                }
                memcpy(path_segment, path + path_start_idx, segment_len);
                path_segment[segment_len] = '\0'; // Null-terminate

                if (does_sub_file_exist(current_dir, path_segment) == 1) {
                    printf("[ERROR] File '%s' already exists\n", path_segment);
                    return 0;
                }

                File *new_node = init_file(); // Create a new File object
                if (new_node == NULL) {
                    printf("[ERROR] Failed to allocate new file/directory\n");
                    return 0;
                }
                if (set_name(new_node, path_segment) == 0) { // Set its name
                    printf("[ERROR] Failed to set name for new file/directory '%s'\n", path_segment);
                    free_file(new_node);
                    return 0;
                }
                // By default, init_file sets type to 0 (file). If a directory is intended,
                // set_type must be called on the returned File* after add_file.

                if (add_file_to_dir(current_dir, new_node) == 0) {
                    printf("[ERROR] Failed to add file to %s\n", current_dir->name);
                    free_file(new_node);
                    return 0;
                }
                return 1;
            }

            // This is an intermediate directory
            size_t segment_len = next_slash_idx - path_start_idx;
            if (segment_len >= MAX_NAME_LEN) {
                printf("[ERROR] Size calculation failed for intermediate segment\n");
                return 0;
            }
            memcpy(path_segment, path + path_start_idx, segment_len);
            path_segment[segment_len] = '\0'; // Null-terminate

            File *next_dir = retrieve_sub(current_dir, path_segment);
            if (next_dir == NULL) {
                printf("[ERROR] Directory '%s' does not exist.\n", path_segment);
                return 0;
            }
            if (next_dir->type != 1) { // Ensure it's a directory
                printf("[ERROR] '%s' is not a directory\n", path_segment);
                return 0;
            }
            path_start_idx = next_slash_idx + 1;
            current_dir = next_dir;
        }
    } else { // Relative path (from root)
        if (does_sub_file_exist(root, path) == 1) {
            printf("[ERROR] File '%s' already exists\n", path);
            return 0;
        }

        File *new_node = init_file();
        if (new_node == NULL) {
            printf("[ERROR] Failed to allocate new file/directory\n");
            return 0;
        }
        if (set_name(new_node, path) == 0) {
            printf("[ERROR] Failed to set name for new file/directory '%s'\n", path);
            free_file(new_node);
            return 0;
        }

        if (add_file_to_dir(root, new_node) == 0) {
            printf("[ERROR] Failed to add file to root\n");
            free_file(new_node);
            return 0;
        }
        return 1;
    }
}

// Function: free_file (renamed from free_file with appropriate types)
// This function frees resources specific to a single File struct, not recursively.
void free_file(File *file) {
    if (file != NULL) {
        if (file->type == 0) { // If it's a file, free its data
            if (file->file_data.data != NULL) {
                free(file->file_data.data);
            }
        } else { // If it's a directory, free the array of sub-file pointers
            // Note: This does NOT free the child File objects themselves.
            // Full recursive cleanup is handled by cleanup_filesystem.
            if (file->dir_data.sub_files != NULL) {
                free(file->dir_data.sub_files);
            }
        }
        free(file); // Finally, free the File struct itself
    }
}

// Function: init_file (renamed from init_file with appropriate types)
File *init_file(void) {
    File *new_file = (File *)malloc(sizeof(File));
    if (new_file != NULL) {
        memset(new_file, 0, sizeof(File));
        // Default values: name is empty, type is 0 (regular file),
        // data_size/sub_file_count is 0, data/sub_files pointer is NULL.
    }
    return new_file;
}

// Function: set_name (renamed from set_name with appropriate types)
size_t set_name(File *file, const char *name) {
    if (file == NULL || name == NULL) {
        return 0;
    }
    size_t name_len = strlen(name);
    if (name_len < MAX_NAME_LEN) {
        memcpy(file->name, name, name_len);
        file->name[name_len] = '\0'; // Ensure null termination
        return name_len;
    }
    return 0; // Name too long
}

// Function: set_type (renamed from set_type with appropriate types)
int set_type(File *file, int type_val) {
    if (file == NULL) {
        return 0;
    }
    if (type_val == 0 || type_val == 1) { // 0 for file, 1 for directory
        file->type = type_val;
        return 1;
    }
    return 0; // Invalid type value
}

// Function: set_data (renamed from set_data with appropriate types)
int set_data(File *file, int size, const void *data) {
    if (file == NULL || data == NULL) {
        return 0;
    }
    if (file->type != 0) { // Data can only be set for regular files
        printf("[ERROR] Cannot set data for a directory\n");
        return 0;
    }
    if (size < 0 || size > MAX_DATA_SIZE) { // Validate data size
        printf("[ERROR] Invalid data size: %d (max %d)\n", size, MAX_DATA_SIZE);
        return 0;
    }

    // Free any existing data before allocating new data
    if (file->file_data.data != NULL) {
        free(file->file_data.data);
    }

    // Allocate memory for the new data (+1 for null terminator, common for text data)
    void *new_data = malloc(size + 1);
    if (new_data == NULL) {
        printf("[ERROR] Failed to allocate memory for file data\n");
        return 0;
    }
    memcpy(new_data, data, size);
    ((char*)new_data)[size] = '\0'; // Null-terminate for safety/convenience

    file->file_data.data_size = size;
    file->file_data.data = new_data;
    return 1;
}

// Example main function to demonstrate functionality
int main() {
    init_filesystem();
    printf("Filesystem initialized. Root name: %s, type: %d\n\n", root->name, root->type);

    printf("--- Adding Files and Directories ---\n");
    add_file("/file1.txt");
    File *f1 = get_file("/file1.txt");
    if (f1) {
        set_data(f1, strlen("Hello World"), "Hello World");
        printf("[INFO] Added /file1.txt with data: '%s'\n", (char*)f1->file_data.data);
    }

    add_file("/dir1");
    File *dir1 = get_file("/dir1");
    if (dir1) {
        set_type(dir1, 1); // Make it a directory
        printf("[INFO] Added /dir1 as a directory\n");
    }

    add_file("/dir1/file2.txt");
    File *f2 = get_file("/dir1/file2.txt");
    if (f2) {
        set_data(f2, strlen("Inside dir1"), "Inside dir1");
        printf("[INFO] Added /dir1/file2.txt with data: '%s'\n", (char*)f2->file_data.data);
    }

    add_file("/dir1/dir2");
    File *dir2 = get_file("/dir1/dir2");
    if (dir2) {
        set_type(dir2, 1); // Make it a directory
        printf("[INFO] Added /dir1/dir2 as a directory\n");
    }

    add_file("/fileA.txt"); // For sorting test
    add_file("/fileB.txt"); // For sorting test
    add_file("/fileC.txt"); // For sorting test

    printf("\n--- Operations and Error Cases ---\n");

    printf("Attempting to delete a directory directly (/dir1):\n");
    delete_file("/dir1"); // Should fail

    printf("Attempting to delete /file1.txt:\n");
    if (delete_file("/file1.txt")) {
        printf("[INFO] Successfully deleted /file1.txt\n");
    } else {
        printf("[ERROR] Failed to delete /file1.txt\n");
    }
    printf("Checking if /file1.txt exists: %s\n", get_file("/file1.txt") ? "Yes" : "No");

    printf("Attempting to add an existing file (/fileA.txt):\n");
    add_file("/fileA.txt"); // Should print error

    printf("Attempting to delete a non-existent file (/nonexistent.txt):\n");
    delete_file("/nonexistent.txt");

    printf("Attempting to get a file from a non-directory path (e.g., /fileA.txt/somefile):\n");
    get_file("/fileA.txt/somefile");

    printf("\n--- Filesystem Cleanup ---\n");
    cleanup_filesystem(root);
    printf("Filesystem cleaned up.\n");

    return 0;
}