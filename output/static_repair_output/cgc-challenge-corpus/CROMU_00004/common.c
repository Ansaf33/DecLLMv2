#include <string.h> // For strcmp, memset, strncat

// Define a maximum number of entries for the 'l' table for a compilable example.
// In a real application, 0x5a would likely be a defined constant reflecting the table size.
#define MAX_L_ENTRIES 90 // Original 0x5a = 90
#define L_ENTRY_SIZE 16  // Each entry in 'l' is 0x10 (16) bytes

// Dummy global 'l' table for compilation.
// In a real scenario, this would be populated with actual data.
// Each entry is 16 bytes. The string data is assumed to start at offset 1 within each entry.
char l[MAX_L_ENTRIES][L_ENTRY_SIZE];

// Function: FindCode
int FindCode(char *param_1) {
  int idx = 0;
  while (1) {
    if (MAX_L_ENTRIES <= idx) {
      return -1;
    }
    // Compare string starting at offset 1 within the current 16-byte entry
    if (strcmp(&l[idx][1], param_1) == 0) {
      break;
    }
    idx++;
  }
  return idx;
}

// Function: InitLang
// param_1 is treated as a pointer to a structure that can hold pointers at offset 8 and 12 (0xC).
void InitLang(void *param_1, char *param_2) {
  char temp_buf[8]; // Buffer for string operations: 7 characters + null terminator
  int code_idx;

  if (param_1 != NULL) {
    // Clear the buffer
    memset(temp_buf, 0, sizeof(temp_buf));
    // Concatenate param_2, ensuring not to overflow the buffer (leaving space for null terminator)
    strncat(temp_buf, param_2, sizeof(temp_buf) - 1);
    // Concatenate ".", ensuring not to overflow
    strncat(temp_buf, ".", sizeof(temp_buf) - strlen(temp_buf) - 1);

    code_idx = FindCode(temp_buf);
    if (code_idx != -1) {
      // Assign the address of the found 'l' entry to the memory location param_1 + 8
      // Cast param_1 to char* for byte-level pointer arithmetic, then to void** to store a pointer.
      *(void **)((char *)param_1 + 8) = &l[code_idx][0];
      // Recursive call with the newly linked 'l' entry and the modified string
      InitLang(&l[code_idx][0], temp_buf);

      // Reset buffer and prepare for the next search
      memset(temp_buf, 0, sizeof(temp_buf));
      strncat(temp_buf, param_2, sizeof(temp_buf) - 1);
      strncat(temp_buf, "-", sizeof(temp_buf) - strlen(temp_buf) - 1);

      code_idx = FindCode(temp_buf);
      if (code_idx != -1) {
        // Assign the address of the found 'l' entry to the memory location param_1 + 0xc (12)
        *(void **)((char *)param_1 + 0xc) = &l[code_idx][0];
        // Recursive call with the newly linked 'l' entry and the modified string
        InitLang(&l[code_idx][0], temp_buf);
      }
    }
  }
}