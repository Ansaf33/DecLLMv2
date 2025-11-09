#include <stdio.h>   // For printf, sprintf
#include <string.h>  // For strcmp, strlen
#include <stdlib.h>  // For atoi, exit
#include <stdbool.h> // For bool type

// --- Mock Function Declarations ---
// These functions simulate file operations and program termination based on the original snippet.
// They are provided to make the code compilable. In a real application, these would be
// actual system calls (e.g., open, read, write, close, lseek) or higher-level library functions.

// Simulates opening a file. Returns a file descriptor (int) or -1 on error.
// The original code implies no filename argument for some calls,
// assuming a default file or global context.
int openFile(void);

// Simulates reading from a file until a specific character is found or max_len is reached.
// Returns 1 on success (data read), 0 on EOF, -1 on error.
int readFileUntil(int fd, char* buffer, int max_len, char until_char);

// Simulates reading a fixed size block from a file at an optional offset.
// Returns number of bytes read, or -1 on error.
// The 'origin' parameter would typically be SEEK_SET, SEEK_CUR, or SEEK_END.
int readFile(int fd, void* buffer, size_t size, long offset, int origin);

// Simulates writing a block of data to a file.
// Returns number of bytes written, or -1 on error.
int writeFile(int fd, const void* buffer, size_t size);

// Simulates closing a file. Returns 0 on success, -1 on error.
int closeFile(int fd);

// Simulates creating a new file. Returns a new file descriptor or -1 on error.
int createFile(const char* filename, int mode, int permissions);

// Simulates getting the current file read position (like fseek(fd, 0, SEEK_CUR)).
void fileReadPosition(int fd);

// Simulates seeking to a position relative to current or start/end (like fseek).
// The 'origin' parameter would typically be SEEK_SET, SEEK_CUR, or SEEK_END.
void fileReadPosRelative(int fd, long offset, int origin);

// Simulates program termination.
void _terminate(void);

// --- Global/Static Variable (Decompiler Artifact) ---
// This corresponds to `*pcRam0001018e` in the original snippet, likely a static file descriptor
// or a configuration value that needs to be initialized once.
static int global_config_fd = -1;

// --- Mock Function Definitions ---
// These are minimal implementations to allow compilation. Their behavior is simplified.

int openFile(void) {
    // In a real scenario, this would open a specific file (e.g., "users.db").
    // Returning a dummy positive integer for a successful file descriptor.
    // The original code implies -1 as an error condition.
    return 100; // Simulate success
}

int readFileUntil(int fd, char* buffer, int max_len, char until_char) {
    // Simplified mock: always return 0 (EOF) after a few calls or specific conditions.
    // In a real scenario, this would read from the actual file.
    (void)fd; (void)until_char; // Unused parameters for this simple mock
    if (buffer == NULL || max_len <= 0) return -1;
    buffer[0] = '\0'; // Ensure buffer is empty or null-terminated
    return 0; // Simulate EOF by default
}

int readFile(int fd, void* buffer, size_t size, long offset, int origin) {
    // Simplified mock: always return size (success) and fill buffer with zeros.
    (void)fd; (void)offset; (void)origin; // Unused parameters
    if (buffer == NULL || size == 0) return -1;
    memset(buffer, 0, size);
    return (int)size; // Simulate reading 'size' bytes
}

int writeFile(int fd, const void* buffer, size_t size) {
    // Simplified mock: always return size (success).
    (void)fd; (void)buffer; // Unused parameters
    return (int)size; // Simulate writing 'size' bytes
}

int closeFile(int fd) {
    // Simplified mock: always return 0 (success).
    (void)fd; // Unused parameter
    return 0;
}

int createFile(const char* filename, int mode, int permissions) {
    // Simplified mock: always return a dummy file descriptor.
    (void)filename; (void)mode; (void)permissions; // Unused parameters
    return 200; // Simulate success
}

void fileReadPosition(int fd) {
    // No operation for mock.
    (void)fd; // Unused parameter
}

void fileReadPosRelative(int fd, long offset, int origin) {
    // No operation for mock.
    (void)fd; (void)offset; (void)origin; // Unused parameters
}

void _terminate(void) {
    // The original snippet implies a program exit.
    exit(1);
}

// Function: authenticate
// param_1: username string, param_2: password string
int authenticate(const char* username, const char* password) {
    char user_buffer[25];
    char pass_buffer[25];
    char permissions_buffer[100];
    char line_buffer_200[200];

    int fd = openFile();
    if (fd < 0) {
        printf("Error: Could not open authentication file.\n"); // Original 0x1001a
        _terminate();
    }

    int read_status;

    while (true) {
        // Read username (until ':', max 25 chars)
        read_status = readFileUntil(fd, user_buffer, sizeof(user_buffer), ':');
        if (read_status == 0) { // EOF
            closeFile(fd);
            return -1; // User not found
        }
        // Error handling for readFileUntil can be added here if read_status is -1

        if (strcmp(user_buffer, username) == 0) {
            break; // Username found
        }

        // Username did not match, skip password and permissions line
        readFileUntil(fd, pass_buffer, sizeof(pass_buffer), ':'); // Skip password field
        readFileUntil(fd, line_buffer_200, sizeof(line_buffer_200), '\n'); // Skip rest of the line
    }

    // Username matched, now check password
    readFileUntil(fd, pass_buffer, sizeof(pass_buffer), ':'); // Read password
    if (strcmp(pass_buffer, password) != 0) {
        closeFile(fd);
        return -1; // Incorrect password
    }

    // Password matched, read permissions
    readFileUntil(fd, permissions_buffer, sizeof(permissions_buffer), '\n');
    closeFile(fd);

    return atoi(permissions_buffer); // Convert permissions string to integer
}

// Function: create_user
// param_1: username string, param_2: password string, param_3: permissions string
unsigned int create_user(const char* username, const char* password, const char* permissions_str) {
    char main_buffer[1024];
    size_t data_len;
    int user_count_val = 0;
    int last_user_id_val = 0;
    char current_username_buffer[25];
    char user_filename_buffer[20];
    char existing_permissions_id_buffer[25];
    int fd_main_db;
    int op_status;
    int fd_user_data_file;
    unsigned int new_user_id;

    fd_main_db = openFile();
    if (fd_main_db < 0) {
        printf("Error: Could not open main user database for creation.\n"); // Original 0x10197
        _terminate();
    }

    // Initialize global_config_fd if it's the first run
    if (global_config_fd == -1) { // Original *pcRam0001018e == -1
        global_config_fd = openFile(); // Open config file for ID management
        if (global_config_fd < 0) {
            printf("Error: Could not open ID configuration file.\n"); // Original 0x101be, 0x10210
            _terminate();
        }
    }

    // Read user count and last assigned user ID from config file
    op_status = readFile(global_config_fd, &user_count_val, sizeof(user_count_val), 0, 0);
    if (op_status == -7) { // Custom error code, possibly indicating empty file
        fileReadPosition(global_config_fd);
    }
    op_status = readFile(global_config_fd, &last_user_id_val, sizeof(last_user_id_val), 0, 0);
    if (op_status == -7) {
        fileReadPosition(global_config_fd);
    }

    // Calculate a new user ID based on original snippet's logic
    new_user_id = (unsigned int)last_user_id_val * user_count_val & 0x7fffffff;
    if (new_user_id == 0) { // Ensure ID is not zero, assign a default starting ID if needed
        new_user_id = 1000;
    }


    while (true) {
        // Read existing username from the main database
        op_status = readFileUntil(fd_main_db, current_username_buffer, sizeof(current_username_buffer), ':');
        if (op_status == 0) { // EOF - no more existing users, new user can be added
            // Format new user entry and write to the main database
            sprintf(main_buffer, "%s:%s:%s:%u\n", username, password, permissions_str, new_user_id); // Original 0x101de
            data_len = strlen(main_buffer);
            op_status = writeFile(fd_main_db, main_buffer, data_len);
            if (op_status < 0) {
                printf("Error: Could not write new user to database.\n"); // Original 0x101e8, 0x10407
                _terminate();
            }
            closeFile(fd_main_db);

            // Create user-specific data file (e.g., user_ID.dat)
            sprintf(user_filename_buffer, "user_%u.dat", new_user_id); // Original 0x10201
            op_status = createFile(user_filename_buffer, 1, 2); // Mode 1, permissions 2 (from snippet)
            if (op_status < 0) {
                printf("Error: Could not create user data file '%s'.\n", user_filename_buffer); // Original 0x10209, 0x1046b
                _terminate();
            }
            fd_user_data_file = openFile(); // Reopen the just created file to write to it
            if (fd_user_data_file < 0) {
                printf("Error: Could not open new user data file '%s' for writing.\n", user_filename_buffer); // Original 0x1020d, 0x104a1
                _terminate();
            }

            // Write initial data to the user-specific file
            size_t zero_value = 0;
            writeFile(fd_user_data_file, &zero_value, sizeof(zero_value)); // Write 4 bytes of 0 (Original 0x104c7)
            writeFile(fd_user_data_file, &zero_value, sizeof(zero_value)); // Write 4 bytes of 0 (Original 0x104dc)

            // Write length of permissions string, then the permissions string itself
            size_t permissions_len = strlen(permissions_str);
            writeFile(fd_user_data_file, &permissions_len, sizeof(permissions_len)); // Write 4 bytes of length (Original 0x10502)
            writeFile(fd_user_data_file, permissions_str, permissions_len); // Write permissions string (Original 0x10518)

            closeFile(fd_user_data_file);
            return new_user_id; // Successfully created user, return new ID
        }

        // Compare with existing username
        if (strcmp(current_username_buffer, username) == 0) {
            // Username already exists
            break; // Exit loop to handle existing username error
        }

        // Username did not match, skip password and read existing user's ID/permissions
        readFileUntil(fd_main_db, main_buffer, sizeof(main_buffer), ':'); // Skip password field
        readFileUntil(fd_main_db, existing_permissions_id_buffer, sizeof(existing_permissions_id_buffer), '\n'); // Read permissions/ID field

        // Check for ID collision (if new_user_id is already taken by an existing user)
        unsigned int existing_user_id = (unsigned int)atoi(existing_permissions_id_buffer);
        if (new_user_id == existing_user_id) {
            new_user_id++; // Increment new_user_id to find a unique one
        }
    }

    // If loop breaks here, it means the username already exists.
    // Rewind file pointer slightly (Original 0x10316: fileReadPosRelative(fd, -4, 1))
    fileReadPosRelative(fd_main_db, -4, 1); // Assuming 1 means SEEK_CUR
    closeFile(fd_main_db);
    return 0xFFFFFFFF; // Indicate failure (username already exists)
}