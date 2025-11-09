#include <stdio.h>   // For printf, fprintf, stderr, FILE, fread
#include <stdlib.h>  // For calloc, free, exit
#include <string.h>  // For strlen, strcpy, memcpy
#include <stddef.h>  // For size_t

// Global variables (mocked or assumed from context)
// txcnt: Current number of bytes in txbuf
int txcnt = 0;
// txbuf: Buffer for transmitting data, flushed when full (0x10 bytes)
char txbuf[16];
// secret_page: A page of data (0x1000 bytes) to be processed
char secret_page[4096];
// user_codes: An array where user-specific code/data might be stored
char user_codes[1000]; // Arbitrary size, needs to be sufficiently large based on uid*gid

// External function prototypes (mocked for compilation)
// transmit: Sends data to a file descriptor. Returns 0 on success, non-zero on error.
extern int transmit(int fd, const void *buf, size_t count, int *error_flag);
// receive: Receives data from a file descriptor. Returns 0 on success, non-zero on error.
extern int receive(int fd, void *buf, size_t count, int *error_flag);

// Dummy implementations for transmit and receive to make the code compilable.
// In a real scenario, these would be linked from a library or actual syscalls.
int transmit(int fd, const void *buf, size_t count, int *error_flag) {
    if (error_flag) *error_flag = 0; // Assume no error by default
    // For demonstration, print to stderr. In a real system, this would write to a socket/file.
    // fprintf(stderr, "TRANSMIT: fd=%d, count=%zu\n", fd, count);
    // (void)buf; // Suppress unused parameter warning
    // Actual write call (e.g., write(fd, buf, count);) would go here.
    return 0; // 0 for success
}

int receive(int fd, void *buf, size_t count, int *error_flag) {
    if (error_flag) *error_flag = 0; // Assume no error by default
    // Simulate reading from stdin for fd=0. For other fds, it would be a file/socket.
    size_t bytes_read = 0;
    if (fd == 0) { // Standard input
        bytes_read = fread(buf, 1, count, stdin);
        if (bytes_read < count && ferror(stdin)) {
            if (error_flag) *error_flag = 1; // Set error flag if fread encountered an error
            return -1; // Indicate a read error
        }
    } else {
        // For other file descriptors, this would involve actual read calls.
        // For now, simulate reading nothing for other fds.
        // (void)buf; // Suppress unused parameter warning
        // (void)count; // Suppress unused parameter warning
    }
    // The `read_n` function expects `bytes_read` to be updated via `error_flag` pointer.
    // This mock function does not directly update `bytes_read` through `error_flag`.
    // Instead, `read_n`'s `bytes_read` variable will capture the return value of `fread` implicitly.
    // To align with the original `transmit/receive` usage, let's pass `bytes_read` through `error_flag` if it's the `local_14` variable.
    // The original `receive` signature takes `&local_14` which is `int*`. So `error_flag` should be `bytes_read_ptr`.
    // Let's adjust the mock `receive` to update `error_flag` with `bytes_read`.
    if (error_flag) {
        *error_flag = bytes_read;
    }
    return 0; // 0 for success
}


// Function prototypes
void send_flush(void);
void poop_page_perfectly(void);
void *xcalloc(size_t num_elements, size_t element_size);
unsigned int read_n(int fd, char *buf, unsigned int count, int *error_flag);
void reverse(char *str);
int read_ascii_octal(const char *str, int max_len, int *error_flag);
char *map_type(int type_char);
char *get_user_code(int uid, int gid);
unsigned int sent_n(int fd, const char *buf, unsigned int count, int *error_flag);
void print_entry(const char *entry_data);
char *initialize(const char *src_data);
int empty_block(const char *block);
void skip_data(int fd, int size_to_skip, int *error_flag);


// Function: send_flush
void send_flush(void) {
    int offset = 0;
    int bytes_transmitted; // Variable to store bytes transmitted by 'transmit'
    while (txcnt > offset) {
        if (transmit(1, txbuf + offset, txcnt - offset, &bytes_transmitted) != 0) {
            // Error during transmit, break loop
            break;
        }
        if (bytes_transmitted == 0) {
            // No bytes transmitted but no error, to prevent infinite loop
            break;
        }
        offset += bytes_transmitted;
    }
    txcnt = 0;
}

// Function: poop_page_perfectly
void poop_page_perfectly(void) {
    for (unsigned int i = 0; i < 4096; i += 4) {
        if (txcnt == 16) { // If txbuf is full (0x10 bytes)
            send_flush();
        }
        // Ensure txcnt is within txbuf bounds before writing
        if (txcnt < sizeof(txbuf)) {
            txbuf[txcnt++] = secret_page[i];
        } else {
            // This case should ideally not be reached if txcnt == 16 triggers flush.
            // But as a safeguard, force flush and try again.
            send_flush();
            if (txcnt < sizeof(txbuf)) {
                txbuf[txcnt++] = secret_page[i];
            } else {
                fprintf(stderr, "Error: txbuf overflow in poop_page_perfectly, cannot write.\n");
                break; // Exit loop to prevent further issues
            }
        }
    }
    send_flush();
}

// Function: xcalloc
void *xcalloc(size_t num_elements, size_t element_size) {
    void *ptr = calloc(num_elements, element_size);
    if (ptr == NULL) {
        int dummy_error = 0; // transmit expects an int* for error_flag
        transmit(2, "calloc failed\n", sizeof("calloc failed\n") - 1, &dummy_error);
        exit(1); // Terminate program on memory allocation failure
    }
    return ptr;
}

// Function: read_n
unsigned int read_n(int fd, char *buf, unsigned int count, int *error_flag) {
    unsigned int total_read = 0;
    int bytes_read_this_call; // Bytes read in the current receive call
    if (error_flag != NULL) {
        *error_flag = 0; // Initialize error_flag to no error
    }
    while (total_read < count) {
        int read_status = receive(fd, buf + total_read, count - total_read, &bytes_read_this_call);
        if (read_status != 0) { // An error occurred during receive
            if (error_flag != NULL) {
                *error_flag = 1; // Set error flag
            }
            return total_read; // Return bytes read so far
        }
        if (bytes_read_this_call == 0) { // End of file or no data available
            return total_read;
        }
        total_read += bytes_read_this_call;
    }
    return total_read;
}

// Function: reverse
void reverse(char *str) {
    size_t len = strlen(str);
    char *temp_str = (char *)xcalloc(len + 1, 1); // Allocate memory for reversed string + null terminator
    // Copy characters from str in reverse order to temp_str
    for (size_t i = 0, j = len - 1; i < len; ++i, --j) {
        temp_str[i] = str[j];
    }
    temp_str[len] = '\0'; // Null-terminate the reversed string
    strcpy(str, temp_str); // Copy the reversed string back to the original buffer
    free(temp_str); // Free the temporary allocated memory
}

// Function: read_ascii_octal
int read_ascii_octal(const char *str, int max_len, int *error_flag) {
    int result = 0;
    // Allocate max_len bytes, including space for null terminator.
    char *temp_buf = (char *)xcalloc(max_len, 1);
    
    // Copy max_len - 1 characters from the input string and ensure null termination.
    // This is crucial for strlen and reverse functions to work correctly.
    memcpy(temp_buf, str, max_len - 1);
    temp_buf[max_len - 1] = '\0';

    reverse(temp_buf); // Reverse the string in temp_buf

    size_t actual_len = strlen(temp_buf); // Get the effective length of the string

    // Iterate through the reversed string to convert octal digits
    for (unsigned int i = 0; i < actual_len; ++i) {
        if (temp_buf[i] < '0' || temp_buf[i] > '7') {
            if (error_flag != NULL) {
                *error_flag = 1; // Indicate a non-octal character error
            }
            free(temp_buf);
            return 0; // Return 0 on error
        }
        int multiplier = 1;
        // Calculate 8^i for the current digit's place value
        for (unsigned int j = 0; j < i; ++j) {
            multiplier <<= 3; // Equivalent to multiplier = multiplier * 8;
        }
        result += (temp_buf[i] - '0') * multiplier;
    }
    free(temp_buf);
    return result;
}

// Function: map_type
char *map_type(int type_char) {
    char *type_name;
    switch (type_char) {
        case 0:
        case '0':
            type_name = "Normal";
            break;
        case '1':
            type_name = "Hard link";
            break;
        case '2':
            type_name = "Symbolic link";
            break;
        case '3':
            type_name = "Character device";
            break;
        case '4':
            type_name = "Block device";
            break;
        case '5':
            type_name = "Directory";
            break;
        case '6':
            type_name = "FIFO";
            break;
        default:
            type_name = "Unknown";
            break;
    }
    return type_name;
}

// Function: get_user_code
char *get_user_code(int uid, int gid) {
    fprintf(stderr, "xxx- %d - %d\n", uid, gid);
    // The calculation (long)uid * gid is for an offset into the user_codes array.
    // Cast to long to prevent potential intermediate overflow if uid or gid are large.
    fprintf(stderr, "xxx- %p\n", (void*)(user_codes + (long)uid * gid));
    return user_codes + (long)uid * gid;
}

// Function: sent_n
unsigned int sent_n(int fd, const char *buf, unsigned int count, int *error_flag) {
    unsigned int total_sent = 0;
    int bytes_written_this_call; // Bytes written in the current transmit call
    if (error_flag != NULL) {
        *error_flag = 0; // Initialize error_flag to no error
    }
    while (total_sent < count) {
        unsigned int bytes_to_send_this_call = count - total_sent;
        if (bytes_to_send_this_call > 64) { // Limit chunk size to 64 bytes
            bytes_to_send_this_call = 64;
        }
        int write_status = transmit(fd, buf + total_sent, bytes_to_send_this_call, &bytes_written_this_call);
        if (write_status != 0) { // An error occurred during transmit
            if (error_flag != NULL) {
                *error_flag = 1; // Set error flag
            }
            return total_sent; // Return bytes sent so far
        }
        if (bytes_written_this_call == 0) { // If transmit writes 0 bytes, break to avoid infinite loop
            break;
        }
        total_sent += bytes_written_this_call;
    }
    return total_sent;
}

// Function: print_entry
void print_entry(const char *entry_data) {
    printf("name:\t\t%s\n", entry_data);
    printf("    mode:\t\t%s\n", entry_data + 0x65);
    int uid_val = read_ascii_octal(entry_data + 0x6d, 8, NULL);
    printf("    uid:\t\t%d\n", uid_val);
    int gid_val = read_ascii_octal(entry_data + 0x75, 8, NULL);
    printf("    gid:\t\t%d\n", gid_val);
    printf("    user_code:\t\t");
    char *user_code_ptr = get_user_code(uid_val, gid_val);
    sent_n(1, user_code_ptr, 4, NULL); // Send 4 bytes of user code to fd 1 (stdout)
    printf("\n");
    int size_val = read_ascii_octal(entry_data + 0x7d, 12, NULL);
    printf("    size:\t\t%d\n", size_val);
    int mtime_val = read_ascii_octal(entry_data + 0x89, 12, NULL);
    printf("    mtime:\t\t%d\n", mtime_val);
    // Cast to unsigned char before int to correctly handle char values as positive integers
    char *type_name = map_type((int)*(unsigned char *)(entry_data + 0x9d));
    printf("    type:\t\t%s\n", type_name);
    printf("    link_name:\t\t%s\n", entry_data + 0x9e);
    printf("    magic:\t\t%s\n", entry_data + 0x103);
    int version_val = read_ascii_octal(entry_data + 0x10a, 2, NULL);
    printf("    version:\t\t%d\n", version_val);
    printf("    owner_name:\t\t%s\n", entry_data + 0x10c);
    printf("    group_name:\t\t%s\n", entry_data + 0x12d);
    int dev_major_val = read_ascii_octal(entry_data + 0x14e, 8, NULL);
    printf("    dev_major:\t\t%d\n", dev_major_val);
    int dev_minor_val = read_ascii_octal(entry_data + 0x156, 8, NULL);
    printf("    dev_minor:\t\t%d\n", dev_minor_val);
    printf("    prefix:\t\t%s\n", entry_data + 0x15e);
}

// Function: initialize
char *initialize(const char *src_data) {
    char *dest_data = (char *)xcalloc(0x1c3, 1); // Allocate 451 bytes for the new entry structure
    // Copy various fields from src_data to dest_data using hardcoded offsets and lengths
    memcpy(dest_data, src_data, 100);
    memcpy(dest_data + 0x65, src_data + 100, 8);
    memcpy(dest_data + 0x6d, src_data + 0x6c, 8);
    memcpy(dest_data + 0x75, src_data + 0x74, 8);
    memcpy(dest_data + 0x7d, src_data + 0x7c, 0xc);
    memcpy(dest_data + 0x89, src_data + 0x88, 0xc);
    memcpy(dest_data + 0x95, src_data + 0x94, 8);
    memcpy(dest_data + 0x9d, src_data + 0x9c, 1);
    memcpy(dest_data + 0x9e, src_data + 0x9d, 100);
    memcpy(dest_data + 0x103, src_data + 0x101, 6);
    memcpy(dest_data + 0x10a, src_data + 0x107, 2);
    memcpy(dest_data + 0x10c, src_data + 0x109, 0x20);
    memcpy(dest_data + 0x12d, src_data + 0x129, 0x20);
    memcpy(dest_data + 0x14e, src_data + 0x149, 8);
    memcpy(dest_data + 0x156, src_data + 0x151, 8);
    memcpy(dest_data + 0x15e, src_data + 0x159, 100);
    return dest_data;
}

// Function: empty_block
int empty_block(const char *block) {
    // Check if all bytes in the first 512 bytes (0x200) of the block are null
    for (unsigned int i = 0; i <= 511; ++i) { // Loop up to and including index 511 (0x1ff)
        if (block[i] != '\0') {
            return 0; // Not empty, found a non-null byte
        }
    }
    return 1; // Block is empty (all null bytes)
}

// Function: skip_data
void skip_data(int fd, int size_to_skip, int *error_flag) {
    char buffer[512]; // Buffer to read chunks of data
    while (size_to_skip > 0 && (error_flag == NULL || *error_flag == 0)) {
        unsigned int bytes_expected_this_chunk = (size_to_skip > 512) ? 512 : size_to_skip;

        int bytes_read_actual_val; // Variable to store bytes read by read_n
        unsigned int bytes_read_actual = read_n(fd, buffer, bytes_expected_this_chunk, &bytes_read_actual_val);

        // read_n sets *error_flag to 1 if an error occurred.
        // It also returns the total bytes read, which might be less than expected.
        if (bytes_read_actual_val != bytes_read_actual && error_flag != NULL) { // If actual bytes read differs from what was reported as successful read in error_flag
            *error_flag = 1; // Indicate a read error
            return;
        }

        if (error_flag != NULL && *error_flag != 0) { // Error during read_n
            return;
        }
        if (bytes_read_actual == 0 && bytes_expected_this_chunk > 0) { // EOF or no data, but expected some
            if (error_flag != NULL) {
                *error_flag = 1; // Set error flag
            }
            return;
        }
        size_to_skip -= bytes_read_actual; // Subtract actual bytes read from total to skip
    }
}

// Function: main
int main(void) {
    char header_block[512]; // Buffer to hold a 512-byte header block
    int error_flag;         // Flag to indicate errors during I/O or parsing
    char *entry_ptr;        // Pointer to a dynamically allocated entry structure
    int empty_block_count = 0; // Counter for consecutive empty blocks

    // Initialize global arrays for compilation purposes if not provided elsewhere.
    // In a real scenario, these would be populated with actual data.
    memset(secret_page, 'A', sizeof(secret_page));
    memset(user_codes, 'B', sizeof(user_codes));

    poop_page_perfectly(); // Perform initial data "pooping"

    while (1) { // Main loop to process archive entries
        // Read a 512-byte header block from standard input (fd 0)
        int bytes_read_val; // Variable to capture bytes read from receive in read_n
        unsigned int bytes_read = read_n(0, header_block, 512, &bytes_read_val);

        // Check if a full 512-byte block was read and no error occurred
        if (bytes_read != 512 || bytes_read_val != (int)bytes_read) { // bytes_read_val is from receive's error_flag
            // If not a full block, or an error occurred, it signifies end of archive or problem.
            return 0;
        }

        // Check if the current block is entirely null bytes
        if (empty_block(header_block) != 0) { // empty_block returns 1 if it's empty
            empty_block_count++;
            if (empty_block_count == 2) { // Two consecutive empty blocks typically mark the end of a tar archive
                return 0; // Exit program
            }
            continue; // Skip to the next block
        }
        
        empty_block_count = 0; // Reset counter if a non-empty block is found

        // Initialize a new entry structure by copying data from the header block
        entry_ptr = initialize(header_block);
        print_entry(entry_ptr); // Print the details of the current entry

        // Read the data size for the current entry from the entry structure itself
        int data_size = read_ascii_octal(entry_ptr + 0x7d, 12, &error_flag);
        if (error_flag != 0) {
            free(entry_ptr); // Free allocated memory before exiting on error
            return 0;
        }

        // Skip the actual file data content based on the data_size
        skip_data(0, data_size, &error_flag);
        if (error_flag != 0) {
            free(entry_ptr); // Free allocated memory before exiting on error
            return 0;
        }
        
        free(entry_ptr); // Free memory allocated for the entry after processing
    }
}