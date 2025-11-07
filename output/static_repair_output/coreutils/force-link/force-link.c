#include <stdlib.h> // For malloc, free, size_t
#include <string.h> // For mempcpy, strrchr
#include <errno.h>  // For __errno_location, EEXIST
#include <unistd.h> // For linkat, symlinkat, unlinkat, renameat
#include <stdint.h> // For uint64_t, uint8_t

// --- External declarations (guessed signatures based on usage) ---
// last_component: Assumed to return a pointer within the input string
extern const char *last_component(const char *path);

// try_tempname_len: Assumed signature for creating a temp file and performing an action
// template_path: path template for temporary file (e.g., "/tmp/fooXXXXXX")
// suffix_len: length of suffix to add (e.g., "XXXXXX")
// arg: context passed to the action function
// action: callback function to perform operation with the temporary path
// arg_size: size of the context argument
extern int try_tempname_len(char *template_path, int suffix_len, void *arg,
                            void (*action)(char *, void *), size_t arg_size);

// simple_pattern: Assumed to be an 8-byte value
extern uint64_t simple_pattern;

// DAT_001004c0: Assumed to be a single byte value
extern unsigned char DAT_001004c0;

// __errno_location: Standard GNU C library function to get a pointer to errno
extern int *__errno_location(void);

// __stack_chk_fail: Standard stack smashing protector failure handler
extern void __stack_chk_fail(void) __attribute__((noreturn));

// GCC intrinsic for reading from FS segment register on x86-64.
// This is typically used for accessing the stack canary in TLS (Thread Local Storage)
// pointed to by the FS segment register, at offset 0x28.
#if defined(__GNUC__) && defined(__x86_64__)
#define READ_FS_QWORD(offset) __builtin_read_fs_qword(offset)
#else
// Fallback for non-GCC or non-x86_64 environments.
// This will not provide actual stack canary protection, but allows compilation.
#define READ_FS_QWORD(offset) 0xdeadbeefdeadbeefL // Dummy value for compilation
#endif

// Function: samedir_template
void *samedir_template(const char *path, void *buffer_for_small_alloc) {
    const char *component_end_ptr = last_component(path);
    // Calculate required size: length of path component + 8 bytes for simple_pattern + 1 byte for DAT_001004c0
    size_t total_size = (size_t)(component_end_ptr - path) + 9;

    void *result_buffer = buffer_for_small_alloc;
    // If the required size exceeds the small buffer capacity (256 bytes, as 0x101 is 257)
    if (total_size > 256) {
        result_buffer = malloc(total_size);
        if (result_buffer == NULL) {
            return NULL; // malloc failed
        }
    }

    // Copy the path component
    uint8_t *current_ptr = (uint8_t *)mempcpy(result_buffer, path, component_end_ptr - path);
    
    // Append simple_pattern (8 bytes)
    *(uint64_t *)current_ptr = simple_pattern;
    current_ptr += sizeof(uint64_t);
    
    // Append DAT_001004c0 (1 byte)
    *current_ptr = DAT_001004c0;

    return result_buffer;
}

// Context structure for try_link to pass multiple parameters to the callback
struct LinkContext {
    int olddirfd;
    char *oldpath;
    int newdirfd;
    int flags;
};

// Function: try_link - Callback for try_tempname_len to perform the link operation
void try_link(char *temp_path, void *arg_ctx) {
    struct LinkContext *ctx = (struct LinkContext *)arg_ctx;
    linkat(ctx->olddirfd, ctx->oldpath, ctx->newdirfd, temp_path, ctx->flags);
}

// Function: force_linkat
int force_linkat(int olddirfd, char *oldpath, int newdirfd, char *newpath, int flags,
                 char retry_on_eexist, int initial_errno) {
    long stack_canary = READ_FS_QWORD(0x28); // Read stack canary from FS:0x28

    int result_errno = initial_errno;
    if (result_errno < 0) { // If initial_errno indicates an operation hasn't been attempted yet
        if (linkat(olddirfd, oldpath, newdirfd, newpath, flags) == 0) {
            result_errno = 0; // Success
        } else {
            result_errno = *__errno_location();
        }
    }

    // If retry is enabled and the error was EEXIST (file already exists)
    if (retry_on_eexist == '\x01' && result_errno == EEXIST) {
        char temp_buffer[264]; // Local buffer for small path templates (corresponds to local_118)
        char *temp_old_path_template = (char *)samedir_template(newpath, temp_buffer);

        if (temp_old_path_template == NULL) {
            result_errno = *__errno_location(); // Error during template creation
        } else {
            struct LinkContext link_ctx = {
                .olddirfd = olddirfd,
                .oldpath = oldpath,
                .newdirfd = newdirfd,
                .flags = flags
            };

            // Try to create a temporary link name and perform the link operation
            int try_temp_result = try_tempname_len(temp_old_path_template, 0, &link_ctx, try_link, sizeof(link_ctx));

            if (try_temp_result == 0) { // Temporary link created successfully
                // Attempt to atomically rename the temporary link to the final destination
                if (renameat(newdirfd, temp_old_path_template, newdirfd, newpath) == 0) {
                    result_errno = 0; // Rename successful
                } else {
                    result_errno = *__errno_location(); // Rename failed
                }
                // Clean up the temporary link regardless of rename success
                unlinkat(newdirfd, temp_old_path_template, 0);
            } else {
                result_errno = *__errno_location(); // Error during temporary name creation/link
            }

            // Free dynamically allocated memory if it wasn't the local buffer
            if (temp_old_path_template != temp_buffer) {
                free(temp_old_path_template);
            }
        }
    }

    // Stack canary check
    if (stack_canary != READ_FS_QWORD(0x28)) {
        __stack_chk_fail();
    }
    return result_errno;
}

// Context structure for try_symlink to pass multiple parameters to the callback
struct SymlinkContext {
    char *link_target;
    int newdirfd;
};

// Function: try_symlink - Callback for try_tempname_len to perform the symlink operation
void try_symlink(char *temp_path, void *arg_ctx) {
    struct SymlinkContext *ctx = (struct SymlinkContext *)arg_ctx;
    symlinkat(ctx->link_target, ctx->newdirfd, temp_path);
}

// Function: force_symlinkat
int force_symlinkat(char *link_target, int newdirfd, char *newpath, char retry_on_eexist, int initial_errno) {
    long stack_canary = READ_FS_QWORD(0x28); // Read stack canary from FS:0x28

    int result_errno = initial_errno;
    if (result_errno < 0) { // If initial_errno indicates an operation hasn't been attempted yet
        if (symlinkat(link_target, newdirfd, newpath) == 0) {
            result_errno = 0; // Success
        } else {
            result_errno = *__errno_location();
        }
    }

    // If retry is enabled and the error was EEXIST (file already exists)
    if (retry_on_eexist == '\x01' && result_errno == EEXIST) {
        char temp_buffer[264]; // Local buffer for small path templates (corresponds to local_118)
        char *temp_old_path_template = (char *)samedir_template(newpath, temp_buffer);

        if (temp_old_path_template == NULL) {
            result_errno = *__errno_location(); // Error during template creation
        } else {
            struct SymlinkContext symlink_ctx = {
                .link_target = link_target,
                .newdirfd = newdirfd
            };

            // Try to create a temporary symlink name and perform the symlink operation
            int try_temp_result = try_tempname_len(temp_old_path_template, 0, &symlink_ctx, try_symlink, sizeof(symlink_ctx));

            if (try_temp_result == 0) { // Temporary symlink created successfully
                // Attempt to atomically rename the temporary symlink to the final destination
                if (renameat(newdirfd, temp_old_path_template, newdirfd, newpath) == 0) {
                    result_errno = 0; // Rename successful
                } else {
                    result_errno = *__errno_location(); // Rename failed
                }
                // Clean up the temporary symlink regardless of rename success
                unlinkat(newdirfd, temp_old_path_template, 0);
            } else {
                result_errno = *__errno_location(); // Error during temporary name creation/symlink
            }

            // Free dynamically allocated memory if it wasn't the local buffer
            if (temp_old_path_template != temp_buffer) {
                free(temp_old_path_template);
            }
        }
    }

    // Stack canary check
    if (stack_canary != READ_FS_QWORD(0x28)) {
        __stack_chk_fail();
    }
    return result_errno;
}

// --- Minimal main function for compilation ---
// This main function is a placeholder to ensure the provided functions compile.
// It uses dummy values and does not represent a functional program.
int main() {
    // Dummy external variables and functions for compilation.
    // In a real application, these would be properly defined or linked.
    static uint64_t simple_pattern = 0x123456789ABCDEF0ULL;
    static unsigned char DAT_001004c0 = '\0'; // A null terminator often

    // Dummy implementation for last_component
    // Finds the last '/' or returns the start of the string if none.
    const char *last_component(const char *path) {
        const char *last_slash = strrchr(path, '/');
        return last_slash ? (last_slash + 1) : path;
    }

    // Dummy implementation for try_tempname_len
    // This is a very simplified mock. A real implementation would involve mktemp/mkstemp logic.
    int try_tempname_len(char *template_path, int suffix_len, void *arg,
                         void (*action)(char *, void *), size_t arg_size) {
        // In a real scenario, template_path would be modified to a unique name.
        // For this dummy, we just call the action with the template.
        (void)suffix_len; // Unused
        (void)arg_size;   // Unused
        action(template_path, arg);
        return 0; // Simulate success
    }

    // Test samedir_template
    const char *dummy_path = "/path/to/my/file.txt";
    char dummy_buffer[264]; // Matches the size in force_linkat/force_symlinkat
    void *samedir_result = samedir_template(dummy_path, dummy_buffer);
    if (samedir_result != NULL && samedir_result != dummy_buffer) {
        free(samedir_result);
    }
    (void)samedir_result; // Suppress unused variable warning

    // Test force_linkat
    // Using dummy file descriptors (0 for stdin, usually not what you want for linkat)
    // and dummy paths. This will likely fail at runtime without real files/permissions.
    int link_result = force_linkat(0, "/tmp/old_link_target", 0, "/tmp/new_link_path", 0, 1, -1);
    (void)link_result; // Suppress unused variable warning

    // Test force_symlinkat
    int symlink_result = force_symlinkat("/tmp/original_target", 0, "/tmp/new_symlink", 1, -1);
    (void)symlink_result; // Suppress unused variable warning

    return 0;
}