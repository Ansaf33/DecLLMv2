#include <string.h> // For strcmp, strncat, memset

// Define the global array 'l'
// Each entry is 16 bytes. The string for comparison starts at offset 1.
// The entries themselves can also be treated as `struct LangConfig` pointers.
#define MAX_LANG_ENTRIES 91 // Matches the original loop limit 0x5a (90, so 0-90 = 91 entries)
char l[MAX_LANG_ENTRIES][16]; // Placeholder for language data

// Define the structure that the first parameter of InitLang points to.
// This structure is inferred from memory access patterns:
// `param_1 + 8` and `param_1 + 0xc` (12) are used to store pointers.
// The `l` array entries are also treated as this structure in recursive calls.
struct LangConfig {
    char _padding[8];    // Placeholder for bytes 0-7
    void* lang_ptr;      // Pointer stored at offset 8
    void* alt_lang_ptr;  // Pointer stored at offset 12 (0xc)
};

// Function: FindCode
int FindCode(char *param_1) {
  int i;
  
  for (i = 0; i < MAX_LANG_ENTRIES; ++i) {
    // Compare string starting at offset 1 of each 16-byte block in 'l'
    if (strcmp(l[i] + 1, param_1) == 0) {
      return i;
    }
  }
  return -1;
}

// Function: InitLang
void InitLang(struct LangConfig *config, char *param_2) {
  // `temp_buffer` needs to be large enough for `param_2` + suffix ('.' or '-') + null terminator.
  // Original `local_17[7]` implies max 6 characters + null.
  // E.g., "en_US" (5 chars) + "." (1 char) + "\0" (1 char) = 7 bytes.
  char temp_buffer[7]; 
  int found_code_idx;

  if (config != NULL) { // Equivalent to `param_1 != 0`
    // Initialize buffer to all zeros to ensure null termination for strncpy/strncat
    memset(temp_buffer, 0, sizeof(temp_buffer));
    
    // Copy `param_2` to `temp_buffer`. `sizeof(temp_buffer) - 1` ensures space for null terminator.
    strncpy(temp_buffer, param_2, sizeof(temp_buffer) - 1);
    temp_buffer[sizeof(temp_buffer) - 1] = '\0'; // Explicitly ensure null termination

    // Append "." to the current string in `temp_buffer`
    // Ensure there's space for the character and a null terminator.
    strncat(temp_buffer, ".", sizeof(temp_buffer) - strlen(temp_buffer) - 1);
    
    found_code_idx = FindCode(temp_buffer);
    if (found_code_idx != -1) {
      // Store the address of the found 16-byte block from `l`
      config->lang_ptr = (void*)l[found_code_idx];
      
      // Recursive call: `l[found_code_idx]` is treated as a `LangConfig` pointer
      InitLang((struct LangConfig *)l[found_code_idx], temp_buffer);
      
      // Re-initialize buffer for the next string operation
      memset(temp_buffer, 0, sizeof(temp_buffer));
      strncpy(temp_buffer, param_2, sizeof(temp_buffer) - 1);
      temp_buffer[sizeof(temp_buffer) - 1] = '\0'; // Ensure null termination

      // Append "-"
      strncat(temp_buffer, "-", sizeof(temp_buffer) - strlen(temp_buffer) - 1);
      
      found_code_idx = FindCode(temp_buffer);
      if (found_code_idx != -1) {
        config->alt_lang_ptr = (void*)l[found_code_idx];
        InitLang((struct LangConfig *)l[found_code_idx], temp_buffer);
      }
    }
  }
}