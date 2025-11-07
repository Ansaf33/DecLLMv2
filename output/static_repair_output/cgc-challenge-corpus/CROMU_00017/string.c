#include <stdbool.h> // For bool in itos
#include <stdio.h>   // For printf in main
#include <string.h>  // For strlen in main (for testing)

// Function: get_str_end
// Finds the end of a string or the first newline, null-terminating at the newline if found.
int get_str_end(char *s) {
  int len = 0;
  while (s[len] != '\0') {
    if (s[len] == '\n') {
      s[len] = '\0'; // Null-terminate at newline
      return len;
    }
    len++;
  }
  return len; // Return length up to null terminator
}

// Function: reverse
// Reverses a string in place.
void reverse(char *s, int len) {
  int start = 0;
  int end = len - 1;
  char temp;

  while (start < end) {
    temp = s[end];
    s[end] = s[start];
    s[start] = temp;
    start++;
    end--;
  }
}

// Function: itos
// Converts an integer to a string. Returns 1 on success.
int itos(int value, char *buffer) {
  if (value == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return 1;
  }

  bool is_negative = false;
  if (value < 0) {
    is_negative = true;
    value = -value;
  }

  int i = 0;
  while (value != 0) {
    buffer[i++] = (value % 10) + '0';
    value /= 10;
  }

  if (is_negative) {
    buffer[i++] = '-';
  }

  reverse(buffer, i);
  buffer[i] = '\0';
  return 1;
}

// Function: stoi
// Converts a string to an integer. Stores the length of the parsed number in length_ptr.
int stoi(const char *s, int *length_ptr) {
  int len = 0;
  // Loop to find the end of the number string (digits '0' through '9')
  while (s[len] >= '0' && s[len] <= '9') {
    len++;
  }
  *length_ptr = len; // Store the length

  int result = 0;
  for (int i = 0; i < len; i++) {
    result = (result * 10) + (s[i] - '0');
  }
  return result;
}

// Main function to demonstrate usage
int main() {
    char buffer[64];
    int num_val;
    int len_parsed;

    printf("--- Testing get_str_end ---\n");
    char test_str1[] = "Hello World\nThis is a test";
    printf("Original: \"%s\"\n", test_str1);
    int end_pos = get_str_end(test_str1);
    printf("After get_str_end (len %d): \"%s\"\n", end_pos, test_str1);

    char test_str2[] = "Another string without newline";
    printf("Original: \"%s\"\n", test_str2);
    end_pos = get_str_end(test_str2);
    printf("After get_str_end (len %d): \"%s\"\n", end_pos, test_str2);

    printf("\n--- Testing reverse ---\n");
    char rev_str1[] = "abcdefg";
    printf("Original: \"%s\"\n", rev_str1);
    reverse(rev_str1, strlen(rev_str1));
    printf("Reversed: \"%s\"\n", rev_str1);

    char rev_str2[] = "hello";
    printf("Original: \"%s\"\n", rev_str2);
    reverse(rev_str2, strlen(rev_str2));
    printf("Reversed: \"%s\"\n", rev_str2);

    printf("\n--- Testing itos ---\n");
    itos(12345, buffer);
    printf("itos(12345): \"%s\"\n", buffer);
    itos(-6789, buffer);
    printf("itos(-6789): \"%s\"\n", buffer);
    itos(0, buffer);
    printf("itos(0): \"%s\"\n", buffer);
    itos(2147483647, buffer); // INT_MAX
    printf("itos(2147483647): \"%s\"\n", buffer);
    itos(-2147483648, buffer); // INT_MIN
    printf("itos(-2147483648): \"%s\"\n", buffer);

    printf("\n--- Testing stoi ---\n");
    char num_str1[] = "12345abc";
    num_val = stoi(num_str1, &len_parsed);
    printf("stoi(\"%s\"): Value=%d, Parsed Length=%d\n", num_str1, num_val, len_parsed);

    char num_str2[] = "0";
    num_val = stoi(num_str2, &len_parsed);
    printf("stoi(\"%s\"): Value=%d, Parsed Length=%d\n", num_str2, num_val, len_parsed);

    char num_str3[] = "9876543210"; // Potentially overflows 32-bit int
    num_val = stoi(num_str3, &len_parsed);
    printf("stoi(\"%s\"): Value=%d, Parsed Length=%d\n", num_str3, num_val, len_parsed);

    char num_str4[] = "no_numbers_here";
    num_val = stoi(num_str4, &len_parsed);
    printf("stoi(\"%s\"): Value=%d, Parsed Length=%d\n", num_str4, num_val, len_parsed);

    return 0;
}