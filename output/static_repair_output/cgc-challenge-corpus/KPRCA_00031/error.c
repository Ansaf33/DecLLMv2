#include <stdio.h>  // Required for fprintf and stderr
#include <stdlib.h> // Required for exit

// Function: _error
void _error(int error_code, const char *file, int line) {
  switch(error_code) {
  default:
    fprintf(stderr, "%s:%d - unknown error\n", file, line);
    break;
  case 1:
    fprintf(stderr, "%s:%d - allocation error\n", file, line);
    break;
  case 2:
    fprintf(stderr, "%s:%d - something broke with the tree\n", file, line);
    break;
  case 3:
    fprintf(stderr, "%s:%d - something broke with the list\n", file, line);
    break;
  case 4:
    fprintf(stderr, "%s:%d - something broke with commands\n", file, line);
    break;
  case 5:
    fprintf(stderr, "%s:%d - random failed, getting out of here.\n", file, line);
    break;
  }
  exit(1);
}