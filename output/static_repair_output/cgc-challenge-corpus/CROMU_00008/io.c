#include <unistd.h> // For read()
#include <ctype.h>  // For isalnum()

// Global variable for last error.
static int g_lasterror = 0;

// Type definition for uint
typedef unsigned int uint;

// Function: readLine
uint readLine(int fd, char *buffer, uint max_len_incl_null) {
  int bytes_read;
  char c;
  uint i; // Loop counter and character count

  for (i = 0; i < max_len_incl_null - 1; ++i) {
    bytes_read = read(fd, &c, 1);
    if (bytes_read <= 0) { // 0 for EOF, -1 for error
      g_lasterror = bytes_read;
      buffer[i] = '\0'; // Null-terminate partially read string
      return 0xffffffff;
    }
    if (c == '\n') {
      break;
    }
    buffer[i] = c;
  }
  buffer[i] = '\0'; // Null-terminate the string
  return i;         // Return number of characters read (string length)
}

// Function: sanitize_string
char *sanitize_string(char *str) {
  char *original_str = str;

  for (; *str != '\0'; ++str) {
    // Check if the character is NOT an alphanumeric character
    if (!isalnum((unsigned char)*str)) {
      *str = '_';
    }
  }
  return original_str;
}