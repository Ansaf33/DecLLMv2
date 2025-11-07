#include <stdio.h> // Required for FILE, stdin, stdout, fflush, fread, fgetc, size_t

// Function: read_until
// Reads characters from stdin into buffer until max_len-1 characters are read,
// EOF is encountered, or the specified delimiter is found.
// The buffer is null-terminated. The delimiter character is consumed but not stored.
void read_until(char *buffer, size_t max_len, char delimiter) {
  fflush(stdout); // Ensure any pending output is displayed before reading input
  size_t i = 0;
  int c;

  while (i < max_len - 1 && (c = fgetc(stdin)) != EOF && c != delimiter) {
    buffer[i++] = (char)c;
  }
  buffer[i] = '\0'; // Null-terminate the string
}

// Function: read_n
// Reads a specified number of bytes from stdin into a buffer.
void read_n(void *buffer, size_t num_bytes) {
  fflush(stdout); // Ensure any pending output is displayed before reading input
  // fread(ptr, size, nmemb, stream)
  // ptr: buffer where data is stored
  // size: size of each item to be read (1 byte for raw byte reading)
  // nmemb: number of items to be read (total num_bytes)
  // stream: input stream (stdin)
  fread(buffer, 1, num_bytes, stdin);
}