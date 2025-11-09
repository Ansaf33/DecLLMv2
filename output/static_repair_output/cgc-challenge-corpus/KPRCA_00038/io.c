#include <stdio.h>   // For NULL, potentially for printf in main for testing
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For strlen, memmove
#include <stdbool.h> // For bool
#include <unistd.h>  // For read (assuming 'receive' is 'read')
#include <fcntl.h>   // For open, if testing with actual files

// Define the io_state struct based on parameter access patterns
typedef struct {
    int fd;         // param_1[0]: File descriptor or -1 for byte array/string
    char *buffer;   // param_1[1]: Pointer to the buffer
    int current;    // param_1[2]: Current read/write position within the buffer
    int size;       // param_1[3]: Total valid bytes *currently* in the buffer (for fd) or total length (for bytes/string)
    int mark;       // param_1[4]: Marked position for rewind, -1 (0xffffffff) if no mark
} io_state;

// Forward declarations for functions that call each other
void io_init_fd(io_state *io, int fd_val);
void io_init_bytes(io_state *io, char *data_ptr, int data_len);
void io_init_string(io_state *io, char *str);
void io_mark(io_state *io);
bool io_rewind(io_state *io);
int io_tell(io_state *io);
bool io_seek(io_state *io, int offset);
int io_getc(io_state *io);
void io_ungetc(io_state *io);
int io_peek(io_state *io);
int io_read(void); // This function is a stub in the original, returns -1.

// Function: io_init_fd
void io_init_fd(io_state *io, int fd_val) {
  io->fd = fd_val;
  io->buffer = (char*)malloc(0x2000); // Allocate a 8192-byte buffer
  if (!io->buffer) {
      // Handle malloc failure, e.g., set fd to -1 and return
      io->fd = -1;
      return;
  }
  io->current = 0;
  io->size = 0;
  io->mark = -1; // 0xffffffff
}

// Function: io_init_bytes
void io_init_bytes(io_state *io, char *data_ptr, int data_len) {
  io->fd = -1; // Indicate not a file descriptor
  io->buffer = data_ptr;
  io->size = data_len;
  io->current = 0;
  io->mark = -1; // 0xffffffff
}

// Function: io_init_string
void io_init_string(io_state *io, char *str) {
  io_init_bytes(io, str, strlen(str));
}

// Function: io_mark
void io_mark(io_state *io) {
  // If it's an FD stream and current position is not at the beginning of the buffer,
  // shift the remaining data to the start of the buffer.
  if ((io->fd != -1) && (io->current > 0)) {
    memmove(io->buffer, io->buffer + io->current, io->size - io->current);
    io->size -= io->current;
    io->current = 0;
  }
  io->mark = io->current;
}

// Function: io_rewind
bool io_rewind(io_state *io) {
  if (io->mark != -1) { // Check if a mark has been set
    io->current = io->mark;
    return true;
  }
  return false;
}

// Function: io_tell
int io_tell(io_state *io) {
  return io->current - io->mark;
}

// Function: io_seek
bool io_seek(io_state *io, int offset) {
  int target_pos = offset + io->mark;
  // Ensure target_pos is within valid bounds (0 to io->size)
  if (target_pos >= 0 && target_pos <= io->size) {
    io->current = target_pos;
    return true;
  }
  return false;
}

// Function: io_getc
int io_getc(io_state *io) {
  if (io->current == io->size) { // Buffer exhausted
    if (io->fd == -1) { // Not an FD, no more data to load
      return -1;
    }
    // If buffer is full (0x2000 bytes) and all data has been read,
    // it implies no more space to read into. This might require calling io_mark
    // to shift data, or resizing the buffer, but the original logic returns -1.
    if (io->size == 0x2000) { // Assuming 0x2000 is the buffer's fixed capacity
      return -1;
    }

    // Try to read 1 byte from the file descriptor
    ssize_t bytes_read = read(io->fd, io->buffer + io->size, 1);

    if (bytes_read <= 0) { // Error or EOF
      return -1;
    }
    io->size += bytes_read; // Add the byte read to the buffer's valid size
  }
  // Return the character at the current position and then increment current
  return (int)(unsigned char)io->buffer[io->current++];
}

// Function: io_ungetc
void io_ungetc(io_state *io) {
  // The original logic calculates the effective current position relative to mark,
  // then attempts to seek back by 1.
  // io_tell(io) returns (io->current - io->mark)
  // io_seek(io, (io->current - io->mark) - 1)
  // Inside io_seek, target_pos = ((io->current - io->mark) - 1) + io->mark = io->current - 1.
  // So, this effectively decrements io->current if possible.
  io_seek(io, io_tell(io) - 1);
}

// Function: io_peek
int io_peek(io_state *io) {
  int c = io_getc(io);
  if (c != -1) {
    io_ungetc(io);
  }
  return c;
}

// Function: io_read
// This function is a stub in the original code, returning 0xffffffff (-1).
// It does not take an io_state parameter and performs no actual read operation.
int io_read(void) {
  return -1; // 0xffffffff is -1 in 32-bit signed integer
}

// Helper function to free malloc'd buffer in io_state
void io_free(io_state *io) {
    // Only free the buffer if it was allocated by io_init_fd
    // (indicated by fd != -1 and buffer not NULL)
    if (io->fd != -1 && io->buffer != NULL) {
        free(io->buffer);
        io->buffer = NULL;
    }
    // Reset other fields
    io->fd = -1;
    io->current = 0;
    io->size = 0;
    io->mark = -1;
}

// Main function to make the code compilable and provide an example of usage
int main() {
    io_state my_stream;
    char test_data[] = "Test string for io_init_string.";

    printf("--- Testing io_init_string ---\n");
    io_init_string(&my_stream, test_data);
    printf("Initialized string: '%s'\n", my_stream.buffer);
    printf("Initial state: current=%d, size=%d, mark=%d\n", my_stream.current, my_stream.size, my_stream.mark);

    printf("io_getc: %c\n", io_getc(&my_stream)); // T
    printf("io_peek: %c\n", io_peek(&my_stream)); // e
    printf("io_getc: %c\n", io_getc(&my_stream)); // e
    printf("Current state: current=%d, size=%d, mark=%d\n", my_stream.current, my_stream.size, my_stream.mark);
    printf("io_tell (relative to mark): %d\n", io_tell(&my_stream)); // Should be current - (-1) = current + 1

    printf("--- Testing io_mark and io_rewind ---\n");
    io_mark(&my_stream);
    printf("After io_mark: current=%d, size=%d, mark=%d\n", my_stream.current, my_stream.size, my_stream.mark);
    printf("io_getc: %c\n", io_getc(&my_stream)); // s
    printf("io_getc: %c\n", io_getc(&my_stream)); // t
    printf("io_tell (relative to mark): %d\n", io_tell(&my_stream)); // Should be 2

    if (io_rewind(&my_stream)) {
        printf("After io_rewind: current=%d, size=%d, mark=%d\n", my_stream.current, my_stream.size, my_stream.mark);
        printf("io_getc after rewind: %c\n", io_getc(&my_stream)); // s (should be back to 's')
    }

    printf("--- Testing io_ungetc ---\n");
    printf("io_getc: %c\n", io_getc(&my_stream)); // t
    printf("Current pos before unget: %d\n", my_stream.current);
    io_ungetc(&my_stream);
    printf("Current pos after unget: %d\n", my_stream.current);
    printf("io_getc after unget: %c\n", io_getc(&my_stream)); // t (should be 't' again)

    io_free(&my_stream); // Clean up any allocated resources
    printf("--- Done ---\n");

    // Example with io_init_fd (requires a file to read from)
    io_state file_stream;
    int fd = open("testfile.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Failed to open testfile.txt");
        return 1;
    }
    write(fd, "12345", 5); // Write some data to the file
    lseek(fd, 0, SEEK_SET); // Rewind file descriptor

    printf("\n--- Testing io_init_fd ---\n");
    io_init_fd(&file_stream, fd);
    if (file_stream.fd != -1) {
        printf("Initialized FD stream: fd=%d, current=%d, size=%d, mark=%d\n",
               file_stream.fd, file_stream.current, file_stream.size, file_stream.mark);
        printf("io_getc from file: %c\n", io_getc(&file_stream)); // 1
        printf("io_getc from file: %c\n", io_getc(&file_stream)); // 2
        printf("io_peek from file: %c\n", io_peek(&file_stream)); // 3
        printf("io_getc from file: %c\n", io_getc(&file_stream)); // 3
        io_free(&file_stream);
    } else {
        printf("Failed to initialize FD stream.\n");
    }
    close(fd); // Close the file descriptor

    return 0;
}