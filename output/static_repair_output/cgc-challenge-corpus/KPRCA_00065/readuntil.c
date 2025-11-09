#include <stdio.h>   // For fflush, fread, fgetc, FILE, stdout, stdin, EOF
#include <stddef.h>  // For size_t

// Function: read_until
// Reads from stdin into buffer until the specified delimiter, EOF, or max_len-1 characters are read.
// The buffer is always null-terminated if max_len > 0.
void read_until(char *buffer, size_t max_len, char delimiter) {
  fflush(stdout);

  if (max_len == 0) {
    return; // Cannot store anything if max_len is 0
  }

  size_t i = 0;
  int c;

  // Read characters one by one until buffer is almost full (leaving space for null terminator),
  // or EOF is encountered, or the delimiter character is read.
  while (i < max_len - 1 && (c = fgetc(stdin)) != EOF && c != delimiter) {
    buffer[i++] = (char)c;
  }

  // Null-terminate the buffer.
  buffer[i] = '\0';
}

// Function: read_n
// Reads exactly num_bytes from stdin into the provided buffer.
void read_n(void *buffer, size_t num_bytes) {
  fflush(stdout);

  // Use fread to read num_bytes bytes into the buffer from stdin.
  // We read blocks of 1 byte, num_bytes times.
  fread(buffer, 1, num_bytes, stdin);
}