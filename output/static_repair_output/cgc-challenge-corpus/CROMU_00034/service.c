#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For fixed-width integer types
#include <stddef.h> // For size_t

// Custom types from original snippet, mapped to standard C types
typedef uint8_t byte;
typedef uint16_t uint16_t_alias; // Renamed to avoid clash with stdint.h
typedef uint32_t uint32_t_alias; // Renamed to avoid clash with stdint.h

// Structure representing an entry's metadata, based on `populate_entry`
typedef struct {
    uint8_t field0; // Corresponds to param_7
    uint8_t field1; // Corresponds to param_4 (length, cast to char)
    uint8_t field2; // Corresponds to param_6
    uint8_t field3; // Corresponds to param_5 (required flag)
    char name[26];  // Name string
} EntryData;

// Global variables (undeclared in original snippet, inferred from usage)
#define MAX_CHAPTER_TYPES 8 // Max value of *local_2c (0-7)
#define ENTRIES_PER_CHAPTER 15 // Max value of local_38 (0-14)

EntryData* entry_info_table[MAX_CHAPTER_TYPES * ENTRIES_PER_CHAPTER] = {0};
EntryData* sub_entry_info_list[MAX_CHAPTER_TYPES * ENTRIES_PER_CHAPTER] = {0};

char* titles[MAX_CHAPTER_TYPES] = {0};
char* subtitles[MAX_CHAPTER_TYPES - 1] = {0}; // Max index is 6 for subtitles

uint32_t buffer_length = 0;

uint8_t prev_read_entries[ENTRIES_PER_CHAPTER] = {0};
uint8_t required_entries[ENTRIES_PER_CHAPTER] = {0}; // required_entries[0] stores the count of remaining required entries

// Variables related to win condition and special flags
char successful[5] = {0}; // Assuming it's an array of 5 char flags
uint16_t success_info = 0;
uint16_t _DAT_000197c0 = 0;
uint16_t _DAT_000197c2 = 0;
uint8_t DAT_000197a5 = 0;
uint8_t DAT_000197a6 = 0;
uint16_t DAT_000197be = 0;
uint16_t DAT_000197c4 = 0;
uint16_t DAT_000197c6 = 0;
int DAT_000197a7 = 0; // Initialized to 0, set to -1 or 1 in parse_book
int DAT_000197a8 = 0;

// Dummy external functions (replace with actual implementations if available)
// Assuming receive_bytes reads `count` bytes into `buffer` and returns bytes read.
static size_t receive_bytes(void* buffer, size_t count) {
    // Dummy implementation for compilation
    // In a real scenario, this would read from a socket, file, etc.
    if (buffer && count > 0) {
        memset(buffer, 0xAA, count); // Fill with dummy data
        return count;
    }
    return 0;
}

// Assuming receive_until reads until `max_len` or `terminator` is found, returns bytes read.
static int receive_until(char* buffer, size_t max_len, uint32_t terminator) {
    // Dummy implementation for compilation
    // In a real scenario, this would read from a socket, file, etc.
    if (buffer && max_len > 0) {
        strncpy(buffer, "dummy_name", max_len);
        buffer[max_len - 1] = '\0';
        return (int)strnlen(buffer, max_len);
    }
    return 0;
}

// Function to terminate the program, replaces _terminate()
static void terminate_program(int exit_code) {
    fprintf(stderr, "Program terminated with code %d.\n", exit_code);
    exit(exit_code);
}


// Function: read_short
uint16_t read_short(void) {
  uint16_t received_val = 0;
  if (receive_bytes(&received_val, 2) != 2) {
    return 0;
  }
  return received_val;
}

// Function: populate_entry
void populate_entry(int param_1, int param_2, char *param_3, size_t param_4, uint8_t param_5,
                   uint8_t param_6, uint8_t param_7) {
  EntryData* entry = (EntryData*)malloc(sizeof(EntryData));
  if (entry == NULL) {
      fprintf(stderr, "Memory allocation failed in populate_entry.\n");
      terminate_program(-1);
  }

  // Calculate the linear index for the 2D array
  int index = param_1 * ENTRIES_PER_CHAPTER + param_2;
  if (index >= MAX_CHAPTER_TYPES * ENTRIES_PER_CHAPTER) {
      fprintf(stderr, "Index out of bounds for entry_info_table.\n");
      free(entry);
      terminate_program(-1);
  }

  entry_info_table[index] = entry;

  entry->field0 = param_7;
  entry->field1 = (uint8_t)param_4;
  entry->field2 = param_6;
  entry->field3 = param_5;
  strncpy(entry->name, param_3, sizeof(entry->name) - 1);
  entry->name[sizeof(entry->name) - 1] = '\0'; // Ensure null-termination

  memset(param_3, 0, 0x1a); // Clear the buffer after use
}

// Function: populate_sub_entry
void populate_sub_entry(int param_1, int param_2, char *param_3, size_t param_4, uint8_t param_5,
                       uint8_t param_6, uint8_t param_7) {
  EntryData* entry = (EntryData*)malloc(sizeof(EntryData));
  if (entry == NULL) {
      fprintf(stderr, "Memory allocation failed in populate_sub_entry.\n");
      terminate_program(-1);
  }

  // Calculate the linear index for the 2D array
  int index = param_1 * ENTRIES_PER_CHAPTER + param_2;
  if (index >= MAX_CHAPTER_TYPES * ENTRIES_PER_CHAPTER) {
      fprintf(stderr, "Index out of bounds for sub_entry_info_list.\n");
      free(entry);
      terminate_program(-1);
  }

  sub_entry_info_list[index] = entry;

  entry->field0 = param_7;
  entry->field1 = (uint8_t)param_4;
  entry->field2 = param_6;
  entry->field3 = param_5;
  strncpy(entry->name, param_3, sizeof(entry->name) - 1);
  entry->name[sizeof(entry->name) - 1] = '\0'; // Ensure null-termination

  memset(param_3, 0, 0x1a); // Clear the buffer after use
}

// Function: populateEntryInfo
void populateEntryInfo(void) {
  char entry_name_buffer[26]; // local_2e
  size_t name_len;
  int entry_idx; // local_10
  int chapter_idx; // local_14

  // Initialize titles array
  titles[0] = "Person";
  titles[1] = "Movie";
  titles[2] = "Car";
  titles[3] = "Book";
  titles[4] = "Poem";
  titles[5] = "Job";
  titles[6] = "Subtitles"; // Special case
  titles[7] = "Pet";

  chapter_idx = 0;
  entry_idx = 0;
  strcpy(entry_name_buffer,"first name"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"middle name"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"last name"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"age"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);
  strcpy(entry_name_buffer,"weight"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,1);
  strcpy(entry_name_buffer,"birth country"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"birth state"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"birth city"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"married"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);

  chapter_idx = 1;
  entry_idx = 0;
  strcpy(entry_name_buffer,"title"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"actor"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,1,3);
  strcpy(entry_name_buffer,"director"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,1,3);
  strcpy(entry_name_buffer,"producer"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,1,3);
  strcpy(entry_name_buffer,"year released"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,1);
  strcpy(entry_name_buffer,"plot summary"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);

  chapter_idx = 2;
  entry_idx = 0;
  strcpy(entry_name_buffer,"make"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"model"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"color"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,1,3);
  strcpy(entry_name_buffer,"displacement"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,1);
  strcpy(entry_name_buffer,"displacement units"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"doors"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);

  chapter_idx = 3;
  entry_idx = 0;
  strcpy(entry_name_buffer,"author"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"year"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"summary"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"publisher"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"character"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,1,3);
  strcpy(entry_name_buffer,"made into a movie"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,4);

  chapter_idx = 4;
  entry_idx = 0;
  strcpy(entry_name_buffer,"writer"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"year"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"genre"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"publisher"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"free online"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,4);

  chapter_idx = 5;
  entry_idx = 0;
  strcpy(entry_name_buffer,"title"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"years"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"job category"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"company"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"have a best friend"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);
  strcpy(entry_name_buffer,"salary"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,2);

  chapter_idx = 6; // Special chapter for sub-entries
  entry_idx = 0;
  // These entries are likely placeholders or have dynamic names, original code uses empty strings
  strcpy(entry_name_buffer,""); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,5);
  strcpy(entry_name_buffer,""); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,5);
  strcpy(entry_name_buffer,""); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,5);
  strcpy(entry_name_buffer,""); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,5);
  strcpy(entry_name_buffer,""); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,5);
  strcpy(entry_name_buffer,""); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,5);

  chapter_idx = 7;
  entry_idx = 0;
  strcpy(entry_name_buffer,"name"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"species"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"legs"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"inside only"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);
  strcpy(entry_name_buffer,"age"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"shots updated"); name_len = strlen(entry_name_buffer); populate_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,4);
}

// Function: populateSubEntryInfo
void populateSubEntryInfo(void) {
  char entry_name_buffer[26]; // local_2e
  size_t name_len;
  int entry_idx; // local_10
  int chapter_idx; // local_14

  // Initialize subtitles array
  subtitles[0] = "Sporting Good"; // chapter 0
  subtitles[1] = "Weapon";        // chapter 1
  subtitles[2] = "Collectible";   // chapter 2
  subtitles[3] = "Boat";          // chapter 3
  subtitles[4] = "Art";           // chapter 4
  subtitles[5] = "Exercise";      // chapter 5
  subtitles[6] = "Team";          // chapter 6

  chapter_idx = 0;
  entry_idx = 0;
  strcpy(entry_name_buffer,"make"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"model"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"league"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"length"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"length units"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"years experience"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);
  strcpy(entry_name_buffer,"injury"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);

  chapter_idx = 1;
  entry_idx = 0;
  strcpy(entry_name_buffer,"make"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"model"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"value"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,1);
  strcpy(entry_name_buffer,"style"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"blade length"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);
  strcpy(entry_name_buffer,"length units"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"comes with sheath"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);
  strcpy(entry_name_buffer,"injury"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,4);

  chapter_idx = 2;
  entry_idx = 0;
  strcpy(entry_name_buffer,"name"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"value"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,1);
  strcpy(entry_name_buffer,"seller"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"mint"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,4);
  strcpy(entry_name_buffer,"count"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);

  chapter_idx = 3;
  entry_idx = 0;
  strcpy(entry_name_buffer,"make"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"model"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"length"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"style"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"initial stability"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"years experience"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"highest class"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"touring"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);
  strcpy(entry_name_buffer,"surfing"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);
  strcpy(entry_name_buffer,"tricking"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);
  strcpy(entry_name_buffer,"injury"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);

  chapter_idx = 4;
  entry_idx = 0;
  strcpy(entry_name_buffer,"name"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"seller"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,3);
  strcpy(entry_name_buffer,"value"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,2);
  strcpy(entry_name_buffer,"mint"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,4);
  strcpy(entry_name_buffer,"count"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);

  chapter_idx = 5;
  entry_idx = 0;
  strcpy(entry_name_buffer,"name"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"max weight"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,1);
  strcpy(entry_name_buffer,"reps"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,0);
  strcpy(entry_name_buffer,"sets"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);
  strcpy(entry_name_buffer,"injury"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);

  chapter_idx = 6;
  entry_idx = 0;
  strcpy(entry_name_buffer,"name"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,3);
  strcpy(entry_name_buffer,"position"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,1,3);
  strcpy(entry_name_buffer,"years experience"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,1,0,0);
  strcpy(entry_name_buffer,"injury"); name_len = strlen(entry_name_buffer); populate_sub_entry(chapter_idx,entry_idx++,entry_name_buffer,name_len,0,0,4);
}

// Function: getEntryInfo
uint32_t getEntryInfo(uint32_t chapter_type, uint32_t entry_id, char is_sub_entry, EntryData* out_data) {
  EntryData** target_table = is_sub_entry ? sub_entry_info_list : entry_info_table;
  int index = (chapter_type & 0xffff) * ENTRIES_PER_CHAPTER + (entry_id & 0xff);

  if (index >= MAX_CHAPTER_TYPES * ENTRIES_PER_CHAPTER || target_table[index] == NULL) {
      fprintf(stderr, "Error: Attempted to access non-existent entry info at index %d (chapter %u, entry %u, sub_entry %d).\n",
              index, chapter_type, entry_id, is_sub_entry);
      // Populate with dummy data or return error code
      if (out_data) {
          memset(out_data, 0, sizeof(EntryData));
          strcpy(out_data->name, "ERROR");
      }
      return 0; // Indicate failure
  }

  if (out_data) {
    memcpy(out_data, target_table[index], sizeof(EntryData));
  }

  // Original code checks these fields and prints an error, but doesn't terminate or return error.
  // This implies it's a warning or debug check.
  if ((1 < out_data->field2) || (1 < out_data->field3)) {
    printf("WRONG FORMAT in getEntryInfo\n");
  }
  return 1; // Indicate success
}

// Function: verify_entry
uint32_t verify_entry(byte *param_1) {
  if (param_1 == NULL) {
    return 1; // Null pointer
  }
  if (*param_1 >= ENTRIES_PER_CHAPTER) { // Entry ID too high
    return 2;
  }
  if (param_1[1] >= 6) { // Entry type (field1) too high
    return 3;
  }
  // Check for string/4-byte data types, ensure offset + size doesn't exceed buffer_length
  if (((param_1[1] == 3) || (param_1[1] == 2)) &&
      (buffer_length <= (uint32_t)*(uint16_t *)(param_1 + 3))) {
    return 4; // Data offset too high
  }
  if (param_1[2] >= 0x1b) { // String length too long (0x1a is max for local_2e buffer)
    return 5;
  }
  if ((uint32_t)*(uint16_t *)(param_1 + 5) >= buffer_length) { // Next entry offset too high
    return 6;
  }
  return 0; // Success
}

// Function: verify_chapter
uint32_t verify_chapter(byte *param_1) {
  if (param_1 == NULL) {
    return 1; // Null pointer
  }
  if (*param_1 >= MAX_CHAPTER_TYPES) { // Chapter ID too high
    return 2;
  }
  if (param_1[1] >= ENTRIES_PER_CHAPTER) { // Entry count for chapter too high
    return 3;
  }
  if ((uint32_t)*(uint16_t *)(param_1 + 2) >= buffer_length) { // Entries offset too high
    return 4;
  }
  if ((uint32_t)*(uint16_t *)(param_1 + 4) >= buffer_length) { // Next chapter offset too high
    return 5;
  }
  return 0; // Success
}

// Function: parse_book
uint32_t parse_book(uint32_t param_1, uint32_t_alias param_2_unused, uint16_t_alias param_3) {
  uint32_t current_offset = param_3;
  uint8_t is_sub_chapter_mode = 0;
  uint32_t next_chapter6_main_chapter_offset = 0; // Stores the 'next main chapter' offset when processing chapter 6's sub-chapters

  // DAT_000197a7 is initialized to 0 globally, but in original code it can be -1.
  // This likely depends on how it's used elsewhere. For now, matching original logic.
  DAT_000197a7 = 0;
  DAT_000197a8 = 0;

  while (1) { // Replaced do-while(true) with an explicit infinite loop and internal breaks/returns
    byte* chapter_header = (byte*)(param_1 + current_offset);
    uint32_t verify_res;

    // Handle special chapter type 6 (Subtitles) which initiates a sub-chapter sequence
    if (*chapter_header == 6 && !is_sub_chapter_mode) {
      printf("Title: %s\n", titles[(uint32_t)*chapter_header]); // Original uses titles array for chapter 6 title
      printf("Entry Count: 0x%x\n", chapter_header[1]);

      is_sub_chapter_mode = 1;
      // The first sub-chapter starts at chapter_header + 2
      current_offset = *(uint16_t*)(chapter_header + 2);
      // The chapter_header + 4 field stores the offset to the NEXT MAIN chapter *after* this sub-chapter sequence is done.
      next_chapter6_main_chapter_offset = *(uint16_t*)(chapter_header + 4);
      continue; // Restart loop to process the new current_offset as a sub-chapter
    }

    // Determine which tables to use based on mode
    char** current_titles_array = is_sub_chapter_mode ? subtitles : titles;
    EntryData** current_entry_table = is_sub_chapter_mode ? sub_entry_info_list : entry_info_table;

    if (!is_sub_chapter_mode) {
      verify_res = verify_chapter(chapter_header);
      if (verify_res != 0) {
        printf("Malformed chapter found! %u Exiting.\n", verify_res);
        terminate_program(-2);
      }
    }

    // Print chapter title and entry count
    if ((uint32_t)*chapter_header >= (is_sub_chapter_mode ? (MAX_CHAPTER_TYPES - 1) : MAX_CHAPTER_TYPES)) {
        printf("Error: Chapter ID %u out of bounds for titles array.\n", (uint32_t)*chapter_header);
        terminate_program(-1);
    }
    printf("Title: %s\n", current_titles_array[(uint32_t)*chapter_header]);

    if (chapter_header[1] >= ENTRIES_PER_CHAPTER) { // Check against 0xf (15)
      printf("ERROR: Too many entries: %u\n", chapter_header[1]);
      terminate_program(-1);
    }
    printf("Entry Count: 0x%x\n", chapter_header[1]);

    uint16_t entries_start_offset = *(uint16_t*)(chapter_header + 2);
    uint16_t next_chapter_in_sequence_offset = *(uint16_t*)(chapter_header + 4); // Offset to the next chapter in the current main/sub sequence.

    if (entries_start_offset == 0 || chapter_header[1] == 0) {
      printf("NO entries found\n");
    } else {
      // --- Process Entries for the current chapter ---
      byte* current_entry_ptr = (byte*)(param_1 + entries_start_offset);
      int is_first_entry_in_chapter = 1;
      int prev_read_entries_idx = 0;

      verify_res = verify_entry(current_entry_ptr);
      if (verify_res != 0) {
        printf("Malformed entry found! %u Exiting.\n", verify_res);
        terminate_program(-1);
      }

      while (current_entry_ptr != NULL) {
        EntryData entry_metadata;
        getEntryInfo(*chapter_header, *current_entry_ptr, is_sub_chapter_mode, &entry_metadata);

        if (is_first_entry_in_chapter) {
          memset(prev_read_entries, 0xFF, sizeof(prev_read_entries));
          memset(required_entries, 0xFF, sizeof(required_entries));
          uint8_t temp_required_count = 0;
          for (int i = 0; i < ENTRIES_PER_CHAPTER; i++) {
            if (current_entry_table[(uint32_t)*chapter_header * ENTRIES_PER_CHAPTER + i] != NULL) {
              EntryData* info = current_entry_table[(uint32_t)*chapter_header * ENTRIES_PER_CHAPTER + i];
              if (info->field3 == 1) { // If required
                required_entries[temp_required_count + 1] = (uint8_t)i; // Store from index 1
                temp_required_count++;
              }
            }
          }
          required_entries[0] = temp_required_count; // Store count at index 0
          is_first_entry_in_chapter = 0;
          prev_read_entries_idx = 0;
        }

        // Mark required entries as found
        for (int i = 1; i <= required_entries[0]; i++) { // Iterate from index 1 up to count
          if (*current_entry_ptr == required_entries[i]) {
            required_entries[i] = 0x46; // Mark as found (original behavior, 0x46 is used as a sentinel)
            required_entries[0]--;
            break; // Found and marked, break inner loop for this entry
          }
        }

        // Check for duplicate entries
        for (int i = 0; i < prev_read_entries_idx; i++) {
          if (*current_entry_ptr == prev_read_entries[i]) {
            printf("Duplicate entry found! Exiting...\n");
            terminate_program(-1);
          }
        }
        prev_read_entries[prev_read_entries_idx++] = *current_entry_ptr;

        // Format check
        if (entry_metadata.field1 != current_entry_ptr[1]) {
          printf("WRONG FORMAT: %u vs %u\n", entry_metadata.field1, current_entry_ptr[1]);
          terminate_program(-2);
        }

        // Special global flag updates
        if (*chapter_header == 7 && *current_entry_ptr == 0) {
          if (DAT_000197a7 == -1) DAT_000197a7 = 1;
          else if (DAT_000197a7 == 0) DAT_000197a7 = -1;
        }
        if (*chapter_header == 5 && *current_entry_ptr == 1 && *(uint16_t*)(current_entry_ptr + 3) > 0x14) {
          DAT_000197a8 = 1;
        }

        char entry_name_buf[sizeof(entry_metadata.name)];
        EntryData* info = current_entry_table[(uint32_t)*chapter_header * ENTRIES_PER_CHAPTER + *current_entry_ptr];
        if (info) {
          strncpy(entry_name_buf, info->name, sizeof(entry_name_buf) - 1);
          entry_name_buf[sizeof(entry_name_buf) - 1] = '\0';
        } else {
          strcpy(entry_name_buf, "UNKNOWN");
        }

        // Print entry value based on type
        if (entry_metadata.field0 == 3) { // String
          char entry_value_buf[25];
          uint16_t str_offset = *(uint16_t*)(current_entry_ptr + 3);
          uint8_t str_len = current_entry_ptr[2];
          if (str_offset + str_len > buffer_length) { // Prevent out-of-bounds read
            printf("String read out of bounds. Exiting.\n");
            terminate_program(-1);
          }
          memcpy(entry_value_buf, (void*)(param_1 + str_offset), str_len);
          if (str_len < sizeof(entry_value_buf)) {
            entry_value_buf[str_len] = 0;
          } else {
            entry_value_buf[sizeof(entry_value_buf) - 1] = 0;
          }
          printf("%s: %s\n", entry_name_buf, entry_value_buf);
        } else if (entry_metadata.field0 == 2) { // 4-byte value (uint32_t)
          uint16_t val_offset = *(uint16_t*)(current_entry_ptr + 3);
          if (val_offset + 4 > buffer_length) { // Prevent out-of-bounds read
            printf("Integer read out of bounds. Exiting.\n");
            terminate_program(-1);
          }
          uint32_t val = *(uint32_t*)(param_1 + val_offset);
          printf("%s: 0x%x\n", entry_name_buf, val);
        } else if (entry_metadata.field0 == 4) { // Boolean (short)
          int16_t val = *(int16_t*)(current_entry_ptr + 3);
          if (val == 1) {
            printf("%s: true\n", entry_name_buf);
          } else if (val == 0) {
            printf("%s: false\n", entry_name_buf);
          } else {
            printf("%s!!: 0x%x\n", entry_name_buf, val);
          }
        } else { // Other 2-byte value (ushort)
          uint16_t val = *(uint16_t*)(current_entry_ptr + 3);
          printf("%s: 0x%x\n", entry_name_buf, val);
        }

        // More special global flag updates for chapter 0
        if (*chapter_header == 0) {
          if (*current_entry_ptr == 8) {
            if (successful[0] == 1) { // Assuming successful[0] is the main flag
              _DAT_000197c0 = *(uint16_t*)(current_entry_ptr + 3);
              _DAT_000197c2 = current_entry_ptr[2];
              DAT_000197a5 = 1;
            } else {
              success_info = *(uint16_t*)(current_entry_ptr + 3);
              DAT_000197be = current_entry_ptr[2];
              successful[0] = 1;
            }
          } else if (*current_entry_ptr == 3 && *(uint16_t*)(current_entry_ptr + 3) < 0xb) {
            DAT_000197a6 = 1;
            DAT_000197c4 = *(uint16_t*)(current_entry_ptr + 3);
            DAT_000197c6 = current_entry_ptr[2];
          }
        }

        uint16_t next_entry_offset = *(uint16_t*)(current_entry_ptr + 5);
        if (next_entry_offset == 0) {
          current_entry_ptr = NULL; // No more entries in this chapter
        } else {
          current_entry_ptr = (byte*)(param_1 + next_entry_offset);
          verify_res = verify_entry(current_entry_ptr);
          if (verify_res != 0) {
            printf("Malformed entry found! %u %u Exiting..\n", verify_res, current_entry_ptr[1]);
            terminate_program(-1);
          }
        }
      } // End of inner while loop for entries

      if (required_entries[0] != 0) { // Check if all required entries were found
        printf("DIDN'T SEND ME ALL THE REQUIRED ENTRIES! %u  ", required_entries[0]);
        return 0xffffffff;
      }
    } // End of else (has entries)

    // Decide what to process next
    if (is_sub_chapter_mode) {
      if (next_chapter_in_sequence_offset != 0) {
        current_offset = next_chapter_in_sequence_offset; // Move to next sub-chapter in the sequence
        // Stay in sub-chapter mode
      } else {
        // End of sub-chapters sequence, switch back to main chapter mode
        is_sub_chapter_mode = 0;
        if (next_chapter6_main_chapter_offset != 0) {
          current_offset = next_chapter6_main_chapter_offset; // Go to the main chapter that follows the chapter 6 sequence
          next_chapter6_main_chapter_offset = 0; // Reset
        } else {
          // This means no more chapters at all
          return 1;
        }
      }
    } else { // In main chapter mode
      if (next_chapter_in_sequence_offset != 0) {
        current_offset = next_chapter_in_sequence_offset; // Move to next main chapter
      } else {
        // No more main chapters
        return 1;
      }
    }
  } // End of outer while loop
}

// Function: init
void init(void* ctx_unused) { // EVP_PKEY_CTX changed to void* and unused
  populateEntryInfo();
  populateSubEntryInfo();
  // The original code returns iVar1, which is the result of populateSubEntryInfo.
  // Since populateSubEntryInfo is now void, init should also be void or return a fixed value.
  // Given no error handling, assuming success.
}

// Function: test_win
void test_win(void) {
  char family_name_buffer[10]; // local_26
  int successful_count = 0; // local_10

  for (int i = 0; i < 5; i++) { // successful array has 5 elements
    if (successful[i] == 1) {
      successful_count++;
    }
  }

  if (successful_count == 5) {
    printf("What is your family's last name?\n");
    int bytes_read = receive_until(family_name_buffer, sizeof(family_name_buffer) - 1, 0x19); // 0x19 is the terminator
    family_name_buffer[bytes_read] = '\0'; // Ensure null termination
    printf("Congratulations, %s family! You have a great diary!\n", family_name_buffer);
  }
}

// Function: main
uint32_t main(void) {
  uint16_t received_length;
  byte *buffer_ptr;
  uint32_t bytes_received;

  init(NULL); // Pass NULL as ctx is unused
  memset(successful, 0, sizeof(successful));
  memset(&success_info, 0, 0x14); // Clears success_info and subsequent globals

  printf("Welcome to the diary logger!\n");

  received_length = read_short();
  buffer_length = received_length;

  if (received_length == 0) {
    printf("Buffer length is too small.\n");
    terminate_program(-1);
  }

  buffer_ptr = (byte*)malloc(received_length);
  if (buffer_ptr == NULL) {
    printf("Failed to allocate enough space for this buffer.\n");
    terminate_program(-3);
  }

  bytes_received = receive_bytes(buffer_ptr, received_length);
  if (bytes_received != received_length) {
    printf("Buffer received is not correct size.\n");
    free(buffer_ptr);
    terminate_program(-3);
  }

  if (received_length < 8) { // Minimum size for a chapter header
    printf("Buffer received is not correct size (too short for header).\n");
    free(buffer_ptr);
    terminate_program(-3);
  }

  printf("Length: %u\n", received_length);
  printf("Magic number: 0x%x\n", *(uint16_t*)buffer_ptr);
  printf("Future: 0x%x\n", *(uint16_t*)(buffer_ptr + 2)); // puVar2[1]
  printf("Offset: 0x%x\n", *(uint16_t*)(buffer_ptr + 4)); // *(undefined4 *)(puVar2 + 2) is actually *(uint16_t*)(buffer_ptr + 4)

  int parse_result = parse_book((uint32_t)buffer_ptr, 0, *(uint16_t*)(buffer_ptr + 4)); // Pass offset as uint16_t
  if (parse_result == -1) { // 0xffffffff
    printf("bad book.\n");
    free(buffer_ptr);
    return 0xffffffff;
  } else {
    test_win();
    free(buffer_ptr);
    printf("good book.\n");
    return 0;
  }
}