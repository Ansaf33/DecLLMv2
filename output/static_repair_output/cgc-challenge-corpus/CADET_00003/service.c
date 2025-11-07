#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Assume these external functions exist with these signatures.
// The original code used undefined types and stack manipulation,
// so these are the most plausible C-style declarations.
int transmit_all(int channel, const char* buffer, int len);
int receive_delim(int channel, char* buffer, int max_len, char delimiter);
// void _terminate(void); // Replaced with exit(1) for error conditions.

int check(void) {
  char buffer[64];
  int is_palindrome = 1;
  int iVar1;
  int len_minus_one = -1;
  size_t i;
  int effective_end_idx;

  memset(buffer, '\0', sizeof(buffer));

  iVar1 = receive_delim(0, buffer, sizeof(buffer) - 1, '\n');

  if (iVar1 == 0) {
    for (i = 0; buffer[i] != '\0'; i++) {
      len_minus_one++;
    }

    effective_end_idx = len_minus_one;
    if (len_minus_one % 2 == 1) {
      effective_end_idx = len_minus_one - 1;
    }

    for (i = 0; (int)i <= effective_end_idx / 2; i++) {
      if (buffer[i] != buffer[len_minus_one - i - 1]) {
        is_palindrome = 0;
        break;
      }
    }

    if (buffer[0] == '^') {
        int transmit_result = transmit_all(1, "\n\nEASTER EGG!\n\n", 15);
        if (transmit_result != 0) {
            exit(1);
        }
    }
  } else {
    is_palindrome = -1;
  }
  return is_palindrome;
}

int main(void) {
  int iVar1;

  iVar1 = transmit_all(1, "\nWelcome to Palindrome Finder\n\n", 31);
  if (iVar1 != 0) {
    exit(1);
  }

  while (1) {
    iVar1 = transmit_all(1, "\tPlease enter a possible palindrome: ", 37);
    if (iVar1 != 0) {
      exit(1);
    }

    iVar1 = check();

    if (iVar1 == -1) {
      return 0;
    } else if (iVar1 == 0) {
      iVar1 = transmit_all(1, "\t\tNope, that\'s not a palindrome\n\n", 33);
      if (iVar1 != 0) {
        exit(1);
      }
    } else {
      iVar1 = transmit_all(1, "\t\tYes, that\'s a palindrome!\n\n", 29);
      if (iVar1 != 0) {
        exit(1);
      }
    }
  }
  return 0;
}