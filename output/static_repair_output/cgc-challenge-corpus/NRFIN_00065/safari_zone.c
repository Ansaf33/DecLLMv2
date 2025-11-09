#include <stdio.h>    // For printf, fgets, stdin
#include <string.h>   // For strlen, strchr
#include <stdlib.h>   // For strtoul, isspace (for strtou_custom)
#include <stdint.h>   // For uint32_t, uintptr_t
#include <stdbool.h>  // For true

// --- Global variables and types assumed from decompilation ---

// A large enough global char array to hold various game state data.
// Offsets like 0x10002, 65528, 65532 are relative to the start of this array.
#define GAME_STATE_SIZE (0x10002 + 6) // Max index accessed in set_nickname is 0x10002 + 5
char game_state_data[GAME_STATE_SIZE];

// Accessors for game_state_data using macros for clarity and type safety
// _65532_4_ refers to a 4-byte unsigned integer at offset 65532
#define GAME_STATE_FIELD_65532 (*(uint32_t*)(game_state_data + 65532))
// _65528_4_ refers to a 4-byte unsigned integer at offset 65528
#define GAME_STATE_FIELD_65528 (*(uint32_t*)(game_state_data + 65528))
// _10002_char_array refers to a char array starting at offset 0x10002
#define GAME_STATE_POKEMON_SLOTS (game_state_data + 0x10002)

// Type for the "catch" function pointer
typedef void (*CatchFuncPtr)(unsigned int, unsigned int);

// Structure for safari entries, 12 bytes total (assuming 32-bit pointers)
typedef struct {
    const char* name;          // 4 bytes (pointer)
    unsigned char catch_rate;  // 1 byte
    unsigned char escape_rate; // 1 byte
    unsigned char _padding[2]; // 2 bytes for alignment
    CatchFuncPtr on_catch_func; // 4 bytes (pointer)
} SafariEntry;

// Global array of SafariEntry structures
// Assuming a reasonable number of entries for demonstration purposes.
// In a real scenario, this would be populated with actual data.
#define MAX_SAFARI_ENTRIES 10
SafariEntry data[MAX_SAFARI_ENTRIES];

// --- External function declarations (placeholders for functions not in snippet) ---
// These functions are assumed to be defined elsewhere in the project.
extern int fread_until(void* buf, size_t count, size_t max_len, FILE* stream);
extern int strtou_custom(const char* str, int base, unsigned int* result_ptr); // Modified signature for clarity
extern int check_cookie(uint32_t cookie_val);
extern unsigned char get_flag_byte(unsigned int index);

// Placeholder implementations for compilation, should be replaced by actual ones.
int fread_until(void* buf, size_t count, size_t max_len, FILE* stream) {
    if (fgets((char*)buf, count > max_len ? max_len : count, stream) != NULL) {
        char* newline = strchr((char*)buf, '\n');
        if (newline) *newline = '\0';
        return 0; // Success
    }
    return -1; // Error
}

int strtou_custom(const char* str, int base, unsigned int* result_ptr) {
    char* endptr;
    unsigned long val = strtoul(str, &endptr, base);
    if (endptr == str || (*endptr != '\0' && !isspace((unsigned char)*endptr))) { // Check if parsing failed or partial
        return -1; // Error
    }
    if (result_ptr) {
        *result_ptr = (unsigned int)val;
    }
    return 0; // Success
}

int check_cookie(uint32_t cookie_val) {
    return cookie_val != 0; // Example: returns 1 if cookie is non-zero
}

unsigned char get_flag_byte(unsigned int index) {
    return (unsigned char)(index % 255); // Example: returns index modulo 255
}
// --- End of external function declarations/placeholders ---

// Function: set_nickname
// param_1 in original snippet is interpreted as 'value_to_set' based on its usage.
// The string for printf is assumed to be passed as a separate argument.
void set_nickname(const char* nickname, char value_to_set) {
  int i = 0; // Loop counter, replaces local_10

  while (true) {
    if (5 < i) {
      printf("%s was transferred to box 1 in your PC\n", nickname);
      return;
    }
    if (GAME_STATE_POKEMON_SLOTS[i] == '\0') break; // Accessing game_state at offset 0x10002
    i++;
  }
  printf("%s has joined your party\n", nickname);
  GAME_STATE_POKEMON_SLOTS[i] = value_to_set; // Accessing game_state at offset 0x10002
}

// Function: glitch_items
// param_1 was unused in the original snippet and has been removed.
// param_2 is interpreted as func_ptr_offset.
void glitch_items(unsigned int func_ptr_offset) {
  char input_buffer[40]; // Replaces local_38 to local_14 (0x28 bytes)
  unsigned int parsed_value; // Replaces local_3c

  printf("BZZZT!\n");

  // Read input, check for errors, and parse it
  if (fread_until(input_buffer, sizeof(input_buffer), sizeof(input_buffer) - 1, stdin) != -1 &&
      strlen(input_buffer) != 0 &&
      strtou_custom(input_buffer, 0x10, &parsed_value) != -1) {

    // Construct the function pointer by ORing with 0xf0000000
    void (*target_func)(void) = (void (*)(void))((uintptr_t)func_ptr_offset | 0xf0000000);
    target_func(); // Call the glitched function
  }
}

// Function: do_safari_zone
unsigned int do_safari_zone(void) {
  char input_buffer[40]; // Replaces local_48 to local_24 (0x28 bytes)
  unsigned int choice; // Replaces local_4c
  unsigned int parsed_val_for_nickname_func; // Replaces local_50

  const int max_encounters = 8; // Replaces local_18
  unsigned int current_encounter_idx = GAME_STATE_FIELD_65532 + 1; // Replaces local_1c

  printf("Welcome to the Safari Zone!\n");

  if (check_cookie(GAME_STATE_FIELD_65528) == 0) {
    return 0xffffffff; // Error: cookie check failed
  }
  else if ((unsigned int)(max_encounters - 1) < current_encounter_idx) { // Ensure correct comparison type
    printf("Nothing happened...\n");
    return 0; // No encounter
  }
  else {
    // Select the current safari entry from the global 'data' array
    SafariEntry* current_entry = &data[current_encounter_idx]; // Replaces local_20

    // Get initial catch and escape rates from the entry
    unsigned char current_catch_rate = current_entry->catch_rate; // Replaces local_11
    unsigned char current_escape_rate = current_entry->escape_rate; // Replaces local_12

    unsigned int turn_count = 0; // Replaces local_10

    // Update game state field once at the beginning of the encounter logic
    GAME_STATE_FIELD_65532 = (get_flag_byte(current_encounter_idx) & 0xff) % (max_encounters - 2);

    printf("A wild %s has appeared!\n", current_entry->name);

    while (true) { // Main loop for player actions during an encounter
      printf("What to do?\n1. Ball\n2. Rock\n3. Bait\n4. Run\n\n");

      // Read user choice, check for errors, and parse it
      if (fread_until(input_buffer, sizeof(input_buffer), sizeof(input_buffer) - 1, stdin) == -1 ||
          strlen(input_buffer) == 0 ||
          strtou_custom(input_buffer, 0x10, &choice) == -1) {
        return 0xffffffff; // Error: invalid input
      }

      // Check if Pokemon got away (this check happens before processing the current turn's action)
      if (turn_count > 10 || (turn_count > 0 && get_flag_byte(turn_count) <= current_escape_rate)) {
        printf("%s got away :(\n", current_entry->name);
        return 0; // Pokemon escaped
      }

      unsigned int next_turn_count = turn_count + 1; // Used for flag byte and next iteration's turn count

      switch (choice) {
        case 1: // Ball
          turn_count += 2; // Increment turn_count by 2 for a 'Ball' action
          if (get_flag_byte(next_turn_count) <= current_catch_rate) {
            printf("Congratulations, you\'ve caught %s!\nPlease enter a nickname:\n", current_entry->name);
            do { // Loop until valid nickname input is received
              if (fread_until(input_buffer, sizeof(input_buffer), sizeof(input_buffer) - 1, stdin) == -1) {
                return 0xffffffff; // Error: input read failed
              }
            } while (strlen(input_buffer) == 0 || strtou_custom(input_buffer, 0x10, &parsed_val_for_nickname_func) == -1);

            // Call the associated catch function with encounter index and parsed nickname value
            current_entry->on_catch_func(current_encounter_idx, parsed_val_for_nickname_func);
            return 0; // Pokemon caught, encounter ends
          }
          printf("Darn! Almost had it!\n");
          break;

        case 2: // Rock
          turn_count = next_turn_count; // Increment turn_count by 1
          current_catch_rate <<= 1;     // Double catch rate
          current_escape_rate <<= 1;    // Double escape rate
          break;

        case 3: // Bait
          turn_count = next_turn_count; // Increment turn_count by 1
          current_catch_rate >>= 1;     // Halve catch rate
          current_escape_rate >>= 1;    // Halve escape rate
          break;

        case 4: // Run
          printf("Got away safely!\n");
          return 0; // Player ran away, encounter ends
        
        default:
          printf("Invalid choice. Try again.\n");
          // No turn_count increment for invalid choices, loop continues
          continue; 
      }
    }
  }
  return 0; // Should be unreachable if all paths handled, but matches decompiler's final return.
}