#include <stdio.h>    // For printf, fprintf, stderr
#include <stdlib.h>   // For exit, malloc, rand, srand
#include <string.h>   // For memcpy, memcmp, strchr, strlen
#include <unistd.h>   // For read, write
#include <time.h>     // For time (to seed rand)
#include <limits.h>   // For UINT_MAX

// Decompiler-specific types mapping to standard C types
typedef unsigned int uint;
typedef int undefined4; // Often used for return codes or generic 4-byte values
typedef char undefined;
typedef unsigned char byte;

// --- Global Variables ---
char buf_recv[1024]; // Max size inferred from recvline call (0x400)
unsigned int num_files;
char* files = NULL; // Dynamically allocated

// The original code used a single byte `local_44` for credits.
// Using a char for `credits_val` to match.
char credits_val = 0;
char* ptr_credits = &credits_val;

// URIs, 0x11 (17) bytes each, including space for null terminator
char uri_tip_buf[17];
char uri_gimme_buf[17];
char uri_smore_buf[17];
char uri_mooch_buf[17];
char uri_auth_buf[17];

char* ptr_uri_tip = uri_tip_buf;
char* ptr_uri_gimme = uri_gimme_buf;
char* ptr_uri_smore = uri_smore_buf;
char* ptr_uri_mooch = uri_mooch_buf;
char* ptr_uri_auth = uri_auth_buf;

// Decompiled string literals/constants
const char DAT_0001302b[] = "\n"; // Used for list_files
const char DAT_0001302d[] = "GET "; // Verb "GET "
const char DAT_0001303e[] = "LIST"; // Verb "LIST"
const char DAT_00013055[] = "SUP "; // Verb "SUP "
const char DAT_00013059[] = "AUTH"; // Verb "AUTH"
const char DAT_0001305e[] = "EXIT"; // Verb "EXIT"
const char DAT_000130bd[] = "\n\n"; // Used in main loop, 2 bytes

// --- Function Stubs (minimal implementations for compilation) ---

// Custom _terminate function to replace decompiler's _terminate calls
void _terminate(int code) {
    fprintf(stderr, "Program terminated with code: %d\n", code);
    if (files != NULL) { // Free allocated memory before exit
        free(files);
        files = NULL;
    }
    exit(code);
}

// Mimics strchr, returns offset or UINT_MAX if not found
uint findchar(const char* buf, char c) {
    const char* found = strchr(buf, c);
    if (found) {
        return (uint)(found - buf);
    }
    return UINT_MAX; // 0xffffffff
}

// Writes 'len' bytes from 'buf' to 'fd'.
int sendallnulltrunc(int fd, const char* buf, size_t len) {
    ssize_t total_sent = 0;
    while (total_sent < len) {
        ssize_t sent = write(fd, buf + total_sent, len - total_sent);
        if (sent < 0) {
            perror("sendallnulltrunc error");
            return -1;
        }
        if (sent == 0) { // EOF or connection closed
            break;
        }
        total_sent += sent;
    }
    return (int)total_sent;
}

// Writes 'len' bytes from 'buf' to 'fd'.
// The 'unknown_param' (uVar2) is ignored as its purpose isn't clear from C context.
int sendall(int fd, const void* buf, size_t len, int unknown_param) {
    (void)unknown_param; // Suppress unused parameter warning
    ssize_t total_sent = 0;
    while (total_sent < len) {
        ssize_t sent = write(fd, (const char*)buf + total_sent, len - total_sent);
        if (sent < 0) {
            perror("sendall error");
            return -1;
        }
        if (sent == 0) {
            break;
        }
        total_sent += sent;
    }
    return (int)total_sent;
}

// Stub for other dispatch functions, returning 0 for success
undefined4 do_tip(void) { return 0; }
undefined4 do_status(void) { return 0; }
undefined4 do_gimme(void) { return 0; }
undefined4 do_list(void) { return 0; }
undefined4 do_smore(void) { return 0; }
undefined4 do_youup(void) { return 0; }
undefined4 do_mooch(void) { return 0; }
undefined4 do_sup(void) { return 0; }
undefined4 do_auth(void) { return 0; }

// Stub for allocate, wraps malloc, returns void*
void* allocate(size_t size) {
    void* mem = malloc(size);
    if (mem == NULL) {
        perror("Memory allocation failed");
        return NULL; // Indicate failure
    }
    return mem;
}

// Stub for transmit, wraps write
int transmit(int fd, const char* buf, size_t len) {
    return sendallnulltrunc(fd, buf, len); // Reusing sendallnulltrunc for simplicity
}

// Stub for recvline, reads until newline or max_len, returns bytes read
int recvline(int fd, char* buf, size_t max_len) {
    size_t i = 0;
    char c;
    while (i < max_len - 1) { // Leave space for null terminator
        ssize_t bytes_read = read(fd, &c, 1);
        if (bytes_read <= 0) { // EOF or error
            if (bytes_read < 0) {
                perror("recvline error");
            }
            break;
        }
        buf[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    buf[i] = '\0'; // Null-terminate the string
    return (int)i;
}

// --- Fixed and Refactored Functions ---

// Function: copy_uri
undefined4 copy_uri(void *param_1) {
    uint space_idx = findchar(buf_recv, ' ');
    if ((space_idx == UINT_MAX) || (0x3f0 < space_idx)) { // If space not found or too far
        if (sendallnulltrunc(1, "invalid request\n", 0x11) < 1) {
            _terminate(4);
        }
        return 0x3f3; // Error code
    } else {
        // Copy 0x11 bytes (17 bytes) from buf_recv after the space
        // This includes space for a potential null terminator if the source string is shorter
        memcpy(param_1, buf_recv + space_idx + 1, 0x11);
        // Ensure null termination if the copied data is treated as a string
        ((char*)param_1)[0x10] = '\0'; // Set the last byte to null
        return 0; // Success
    }
}

// Function: check_plebian
undefined4 check_plebian(void) {
    if (*ptr_credits == '\0') { // Check if credits are zero
        return 0x3f1; // Error code
    } else {
        *ptr_credits = *ptr_credits - 1; // Decrement credits
        return 0; // Success
    }
}

// Function: get_file_contents
char* get_file_contents(void *param_1) {
    for (uint i = 0; i < num_files; ++i) {
        // Compare 0x10 bytes (16 bytes) of param_1 with file name
        if (memcmp(param_1, (void *)(files + i * 0x20), 0x10) == 0) {
            // Return pointer to the file content (offset by 0x10 from file start)
            return files + i * 0x20 + 0x10;
        }
    }
    // File not found
    if (sendallnulltrunc(1, "requested file not found\n", 0x1a) < 1) {
        _terminate(4);
    }
    return NULL; // Indicate not found
}

// Function: dump_file
undefined4 dump_file(void* param_1) {
    // 0x111a4 is a magic constant, perhaps an ID or debug info. Ignored by stub.
    char* file_content_ptr = get_file_contents(param_1);
    if (file_content_ptr == NULL) { // Using NULL for not found
        return 0x3f2; // Error code
    } else {
        // Dump 0x10 bytes (16 bytes) of content
        if (sendall(1, file_content_ptr, 0x10, 0x111a4) < 1) {
            _terminate(4);
        }
        return 0; // Success
    }
}

// Function: head_file
undefined4 head_file(void* param_1) {
    // 0x11216 is a magic constant, perhaps an ID or debug info. Ignored by stub.
    char* file_content_ptr = get_file_contents(param_1);
    if (file_content_ptr == NULL) { // Using NULL for not found
        return 0x3f2; // Error code
    } else {
        // Dump 4 bytes of content (head)
        if (sendall(1, file_content_ptr, 4, 0x11216) < 1) {
            _terminate(4);
        }
        return 0; // Success
    }
}

// Function: list_files
undefined4 list_files(void) {
    for (uint i = 0; i < num_files; ++i) {
        // Send file name (0x10 bytes)
        if (sendall(1, files + i * 0x20, 0x10, 0x112c9) < 1) {
            _terminate(4);
        }
        // Send newline
        if (sendall(1, DAT_0001302b, 1, 0x112f2) < 1) {
            _terminate(4);
        }
    }
    return 0; // Success
}

// Function: dispatch_verb
int dispatch_verb(void) {
    int status_code = 0;

    // "GET "
    if (memcmp(buf_recv, DAT_0001302d, strlen(DAT_0001302d)) == 0) {
        status_code = check_plebian();
        if (status_code == 0) {
            status_code = copy_uri(ptr_uri_tip);
            if (status_code == 0) {
                status_code = do_tip();
            }
        }
    }
    // "STATUS"
    else if (memcmp(buf_recv, "STATUS", strlen("STATUS")) == 0) {
        status_code = check_plebian();
        if (status_code == 0) {
            status_code = do_status();
        }
    }
    // "GIMME"
    else if (memcmp(buf_recv, "GIMME", strlen("GIMME")) == 0) {
        status_code = check_plebian();
        if (status_code == 0) {
            status_code = copy_uri(ptr_uri_gimme);
            if (status_code == 0) {
                status_code = do_gimme();
            }
        }
    }
    // "LIST"
    else if (memcmp(buf_recv, DAT_0001303e, strlen(DAT_0001303e)) == 0) {
        status_code = check_plebian();
        if (status_code == 0) {
            status_code = do_list();
        }
    }
    // "SMORE"
    else if (memcmp(buf_recv, "SMORE", strlen("SMORE")) == 0) {
        status_code = copy_uri(ptr_uri_smore);
        if (status_code == 0) {
            status_code = do_smore();
        }
    }
    // "YOUUP"
    else if (memcmp(buf_recv, "YOUUP", strlen("YOUUP")) == 0) {
        status_code = do_youup();
    }
    // "MOOCH"
    else if (memcmp(buf_recv, "MOOCH", strlen("MOOCH")) == 0) {
        status_code = copy_uri(ptr_uri_mooch);
        if (status_code == 0) {
            status_code = do_mooch();
        }
    }
    // "SUP "
    else if (memcmp(buf_recv, DAT_00013055, strlen(DAT_00013055)) == 0) {
        status_code = do_sup();
    }
    // "AUTH"
    else if (memcmp(buf_recv, DAT_00013059, strlen(DAT_00013059)) == 0) {
        status_code = copy_uri(ptr_uri_auth);
        if (status_code == 0) {
            status_code = do_auth();
        }
    }
    // "EXIT"
    else if (memcmp(buf_recv, DAT_0001305e, strlen(DAT_0001305e)) == 0) {
        _terminate(0); // Graceful exit
    }
    // Invalid verb
    else {
        if (sendallnulltrunc(1, "invalid verb\n", strlen("invalid verb\n")) < 1) {
            _terminate(4);
        }
        status_code = 0x3f0; // Error code
    }
    return status_code;
}

// Function: init_content
int init_content(void) {
    byte random_bytes_buf[32]; // Enough for 2x16 bytes

    // Simulate random() filling a buffer. The original loops with local_3c are removed.
    for (int i = 0; i < 16; ++i) {
        random_bytes_buf[i] = (byte)rand();
    }

    num_files = random_bytes_buf[0];
    if (num_files == 0) {
        num_files = 1;
    }

    // Allocate memory for files: num_files * 0x20 bytes (32 bytes per file)
    files = (char*)allocate((size_t)num_files * 0x20);
    if (files == NULL) { // Check if allocate failed
        _terminate(6); // Custom error code for allocation failure
    }

    for (uint i = 0; i < num_files; ++i) {
        // Fill random bytes for file name and content
        for (int k = 0; k < 32; ++k) { // Fill 32 bytes for current file
            random_bytes_buf[k] = (byte)rand();
        }
        // Fill file name (first 0x10 bytes)
        for (uint j = 0; j < 0x10; ++j) {
            files[j + i * 0x20] = random_bytes_buf[j] % 0x1a + 'a'; // 'a' to 'z'
        }
        // Fill file content (next 0x10 bytes)
        for (uint j = 0; j < 0x10; ++j) {
            files[j + i * 0x20 + 0x10] = random_bytes_buf[j + 0x10] % 10 + '0'; // '0' to '9'
        }
    }
    return 0; // Success
}

// Function: main
void main(void) {
    srand(time(NULL)); // Seed random number generator

    if (init_content() != 0) {
        // If init_content failed, it already calls _terminate(6)
        // This block is for other potential init_content failures (e.g., return non-zero but not terminate)
        transmit(1, "fatal error during init_content()\n", strlen("fatal error during init_content()\n"));
        _terminate(0x924); // Custom error code
    }

    while (1) {
        // Read client request line from stdin (fd 0)
        if (recvline(0, buf_recv, sizeof(buf_recv)) < 0) {
            _terminate(5); // Error code for recvline failure
        }

        int status_code = dispatch_verb();

        if (status_code == 0x3f1) { // Plebian (not enough credits) error
            if (sendallnulltrunc(1, "you\'re too poor to use this verb :(\n", strlen("you\'re too poor to use this verb :(\n")) < 1) {
                _terminate(4);
            }
        }
        // Send a newline (or two newlines based on DAT_000130bd length)
        if (sendallnulltrunc(1, DAT_000130bd, strlen(DAT_000130bd)) < 1) {
            _terminate(4);
        }
    }
}