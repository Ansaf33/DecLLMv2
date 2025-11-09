#include <string.h> // For memset
#include <ctype.h>  // For isalpha, toupper

// Function: ftab_init1
void ftab_init1(void *param_1) {
  memset(param_1, 0, 0x68);
}

// Function: ftab_add
void ftab_add(int *freq_table, const char *text) {
  for (int i = 0; text[i] != '\0'; ++i) {
    char c = text[i];
    if (isalpha(c)) {
      char upper_c = toupper(c);
      // 'A' is 65 in ASCII. (upper_c - 'A') gives 0 for 'A', 1 for 'B', etc.
      freq_table[upper_c - 'A']++;
    }
  }
}

// Function: ftab_init
void ftab_init(void *param_1, const char *param_2) {
  ftab_init1(param_1);
  ftab_add((int *)param_1, param_2);
}