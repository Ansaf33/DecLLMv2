#include <stdio.h>    // For printf
#include <string.h>   // For strncpy, strcpy, memcpy, strcmp
#include <stdbool.h>  // For bool type
#include <stdint.h>   // For uint32_t, uint8_t, uint16_t
#include <stdlib.h>   // For NULL

// Global constants for record structure and limits
#define MAX_RECORDS 0x40      // 64 records
#define RECORD_SIZE 400       // 400 bytes per record
#define NAME_BUF_SIZE 0x81    // 128 bytes for name + 1 for null terminator (0x80 + 1)

// Conceptual layout of a record in g_ddapDatabase:
// Offset 0x00: uint32_t record_id
// Offset 0x04: char first_name[NAME_BUF_SIZE] (up to 0x80 chars + null)
// Offset 0x85: char last_name[NAME_BUF_SIZE] (up to 0x80 chars + null)
// Offset 0x106: char user_name[NAME_BUF_SIZE] (up to 0x80 chars + null)
// Offset 0x187: uint8_t date_time[6] (hour, minute, second, month, day, year_offset)
// Total size used by data: 4 + NAME_BUF_SIZE + NAME_BUF_SIZE + NAME_BUF_SIZE + 6 = 4 + 129 + 129 + 129 + 6 = 397 bytes.
// The remaining bytes (400 - 397 = 3) are padding.

// Global database storage and record count
char g_ddapDatabase[MAX_RECORDS * RECORD_SIZE];
uint32_t g_recordCount = 0;

// Placeholder for the external date_compare function.
// This function's signature and logic are inferred from the original snippet's usage.
// It compares two packed date/time values.
// - `record_hmsm`: A 32-bit integer packing hour, minute, second, and month.
// - `record_dy`: A 16-bit integer packing day and year_offset.
// Returns 0 if values match, non-zero otherwise.
int date_compare(uint32_t record_hmsm, uint16_t record_dy, uint32_t search_hmsm, uint16_t search_dy) {
    // This is a simplified comparison based on the packed values.
    // A robust implementation would extract individual date components and compare them.
    if (record_hmsm == search_hmsm && record_dy == search_dy) {
        return 0; // Match
    }
    return 1; // No match
}

// Function: init_database
void init_database(void) {
  g_recordCount = 0;
}

// Function: db_add_record
// param_1: user_name, param_2: first_name, param_3: last_name
// date_data: 6-byte array for hour, minute, second, month, day, year_offset
uint32_t db_add_record(char *param_1, char *param_2, char *param_3, const uint8_t date_data[6]) {
  if (g_recordCount < MAX_RECORDS) {
    uint32_t current_index = g_recordCount;
    char *record_ptr = g_ddapDatabase + current_index * RECORD_SIZE;

    *(uint32_t *)(record_ptr) = current_index; // Record ID
    
    strncpy(record_ptr + 4, param_2, NAME_BUF_SIZE - 1); // First name
    record_ptr[4 + NAME_BUF_SIZE - 1] = '\0'; // Ensure null-termination
    
    strncpy(record_ptr + 0x85, param_3, NAME_BUF_SIZE - 1); // Last name
    record_ptr[0x85 + NAME_BUF_SIZE - 1] = '\0'; // Ensure null-termination
    
    strncpy(record_ptr + 0x106, param_1, NAME_BUF_SIZE - 1); // User name
    record_ptr[0x106 + NAME_BUF_SIZE - 1] = '\0'; // Ensure null-termination
    
    memcpy(record_ptr + 0x187, date_data, 6); // Date/Time (hour, minute, second, month, day, year_offset)
    
    g_recordCount++;
    return current_index; // Return the index of the newly added record
  }
  return 0xffffffff; // Error: database full
}

// Function: db_update_record
// param_1: record_index, param_2: user_name, param_3: first_name, param_4: last_name
// date_data: 6-byte array for hour, minute, second, month, day, year_offset
bool db_update_record(uint32_t param_1, char *param_2, char *param_3, char *param_4, const uint8_t date_data[6]) {
  if (param_1 < g_recordCount) {
    char *record_ptr = g_ddapDatabase + param_1 * RECORD_SIZE;

    strncpy(record_ptr + 4, param_3, NAME_BUF_SIZE - 1); // First name
    record_ptr[4 + NAME_BUF_SIZE - 1] = '\0';
    
    strncpy(record_ptr + 0x85, param_4, NAME_BUF_SIZE - 1); // Last name
    record_ptr[0x85 + NAME_BUF_SIZE - 1] = '\0';
    
    strncpy(record_ptr + 0x106, param_2, NAME_BUF_SIZE - 1); // User name
    record_ptr[0x106 + NAME_BUF_SIZE - 1] = '\0';
    
    memcpy(record_ptr + 0x187, date_data, 6); // Date/Time
    return true;
  }
  return false;
}

// Function: db_remove_record
// param_1: record_index to remove
int db_remove_record(uint32_t param_1) {
  if (param_1 < g_recordCount) {
    g_recordCount--;
    // Shift records down to fill the gap
    for (uint32_t i = param_1; i < g_recordCount; i++) {
      char *dest_ptr = g_ddapDatabase + i * RECORD_SIZE;
      char *src_ptr = g_ddapDatabase + (i + 1) * RECORD_SIZE;

      *(uint32_t *)(dest_ptr) = i; // Update record ID for the shifted record
      
      strcpy(dest_ptr + 4, src_ptr + 4); // First name
      strcpy(dest_ptr + 0x85, src_ptr + 0x85); // Last name
      strcpy(dest_ptr + 0x106, src_ptr + 0x106); // User name
      memcpy(dest_ptr + 0x187, src_ptr + 0x187, 6); // Date/Time
    }
    return 1; // Success
  }
  return 0; // Failure
}

// Function: db_get_record_count
uint32_t db_get_record_count(void) {
  return g_recordCount;
}

// Function: db_search_first_name
// param_1: first name to search for
char * db_search_first_name(char *param_1) {
  for (uint32_t i = 0; i < g_recordCount; i++) {
    if (strcmp(param_1, g_ddapDatabase + i * RECORD_SIZE + 4) == 0) {
      return g_ddapDatabase + i * RECORD_SIZE; // Return pointer to the start of the record
    }
  }
  return NULL; // Not found
}

// Function: db_search_last_name
// param_1: last name to search for
char * db_search_last_name(char *param_1) {
  for (uint32_t i = 0; i < g_recordCount; i++) {
    if (strcmp(param_1, g_ddapDatabase + i * RECORD_SIZE + 0x85) == 0) {
      return g_ddapDatabase + i * RECORD_SIZE;
    }
  }
  return NULL;
}

// Function: db_search_user_name
// param_1: user name to search for
char * db_search_user_name(char *param_1) {
  for (uint32_t i = 0; i < g_recordCount; i++) {
    if (strcmp(param_1, g_ddapDatabase + i * RECORD_SIZE + 0x106) == 0) {
      return g_ddapDatabase + i * RECORD_SIZE;
    }
  }
  return NULL;
}

// Function: db_search_birth_date
// param_1: packed hour, minute, second, month (uint32_t)
// param_2: packed day, year_offset (uint16_t)
char * db_search_birth_date(uint32_t param_1, uint16_t param_2) {
  for (uint32_t i = 0; i < g_recordCount; i++) {
    char *record_ptr = g_ddapDatabase + i * RECORD_SIZE;
    // Extract packed date components from the record
    uint32_t record_hmsm = *(uint32_t *)(record_ptr + 0x187);
    uint16_t record_dy = *(uint16_t *)(record_ptr + 0x18b);

    if (date_compare(record_hmsm, record_dy, param_1, param_2) == 0) {
      return record_ptr;
    }
  }
  return NULL;
}

// Function: db_search_index
// param_1: record_index
char * db_search_index(uint32_t param_1) {
  if (param_1 < g_recordCount) {
    return g_ddapDatabase + param_1 * RECORD_SIZE;
  }
  return NULL;
}

// Function: print_record_helper
// record_ptr: pointer to the start of a record
void print_record_helper(char *record_ptr) {
  if (record_ptr != NULL) {
    // Record ID is at offset 0
    uint32_t record_id = *(uint32_t *)(record_ptr);
    // First name is at offset 4
    char *first_name = record_ptr + 4;
    // Last name is at offset 0x85 (133)
    char *last_name = record_ptr + 0x85;
    // User name is at offset 0x106 (262)
    char *user_name = record_ptr + 0x106;
    // Date/Time components from offset 0x187 (391)
    // Layout: [hour, minute, second, month, day, year_offset] (6 bytes)
    uint8_t hour = record_ptr[0x187];
    uint8_t minute = record_ptr[0x188];
    uint8_t second = record_ptr[0x189];
    uint8_t month = record_ptr[0x18a];
    uint8_t day = record_ptr[0x18b];
    uint8_t year_offset = record_ptr[0x18c];

    printf("Record [%u]\n", record_id);
    printf("First name: %s\n", first_name);
    printf("Last name: %s\n", last_name);
    printf("User name: %s\n", user_name);
    printf("Birthdate: %u/%u/%u %u:%u:%u\n",
           (unsigned int)month,
           (unsigned int)day,
           (unsigned int)(year_offset + 1900), // Original code used + 0x76c (1900 decimal)
           (unsigned int)hour,
           (unsigned int)minute,
           (unsigned int)second);
  }
}

// Main function for demonstration and testing
int main() {
    init_database();

    printf("Database initialized. Record count: %u\n", db_get_record_count());

    // Example date/time data: {hour, minute, second, month, day, year_offset}
    // year_offset is relative to 1900 (e.g., 23 for 2023)
    uint8_t date1[6] = {10, 0, 0, 5, 15, 23}; // 10:00:00 05/15/2023
    uint8_t date2[6] = {14, 30, 15, 12, 25, 22}; // 14:30:15 12/25/2022
    uint8_t date3[6] = {9, 45, 0, 1, 1, 24}; // 09:45:00 01/01/2024

    uint32_t idx1 = db_add_record("jdoe", "John", "Doe", date1);
    if (idx1 != 0xffffffff) printf("Added record at index %u\n", idx1);
    uint32_t idx2 = db_add_record("asmith", "Alice", "Smith", date2);
    if (idx2 != 0xffffffff) printf("Added record at index %u\n", idx2);
    uint32_t idx3 = db_add_record("bob_b", "Bob", "Builder", date3);
    if (idx3 != 0xffffffff) printf("Added record at index %u\n", idx3);

    printf("Current record count: %u\n", db_get_record_count());

    printf("\n--- All Records ---\n");
    for (uint32_t i = 0; i < db_get_record_count(); ++i) {
        print_record_helper(db_search_index(i));
    }

    printf("\n--- Search by Name ---\n");
    char *record = db_search_first_name("Alice");
    if (record) {
        printf("Found Alice:\n");
        print_record_helper(record);
    } else {
        printf("Alice not found.\n");
    }

    record = db_search_user_name("jdoe");
    if (record) {
        printf("Found jdoe:\n");
        print_record_helper(record);
    } else {
        printf("jdoe not found.\n");
    }

    printf("\n--- Search by Birth Date ---\n");
    // Pack target date components for search:
    // (hour, min, sec, month) into uint32_t
    // (day, year_offset) into uint16_t
    // Assuming little-endian packing as per decompiler's interpretation
    uint32_t search_hmsm = (uint32_t)10 | ((uint32_t)0 << 8) | ((uint32_t)0 << 16) | ((uint32_t)5 << 24); // 10:00:00 05
    uint16_t search_dy = (uint16_t)15 | ((uint16_t)23 << 8); // 15/23 (day/year_offset)
    
    record = db_search_birth_date(search_hmsm, search_dy);
    if (record) {
        printf("Found record with birth date 05/15/2023 10:00:00:\n");
        print_record_helper(record);
    } else {
        printf("Record with birth date 05/15/2023 10:00:00 not found.\n");
    }

    printf("\n--- Update Record ---\n");
    uint8_t updated_date[6] = {11, 11, 11, 11, 11, 21}; // 11:11:11 11/11/2021
    if (db_update_record(idx1, "john_d_new", "Johnny", "Doefield", updated_date)) {
        printf("Updated record at index %u:\n", idx1);
        print_record_helper(db_search_index(idx1));
    } else {
        printf("Failed to update record at index %u.\n", idx1);
    }

    printf("\n--- Remove Record ---\n");
    if (db_remove_record(idx2)) { // Remove Alice Smith (at original index 1)
        printf("Removed record at original index %u.\n", idx2);
    } else {
        printf("Failed to remove record at index %u.\n", idx2);
    }
    printf("Record count after removal: %u\n", db_get_record_count());

    printf("\n--- Records after removal and shift ---\n");
    for (uint32_t i = 0; i < db_get_record_count(); ++i) {
        print_record_helper(db_search_index(i));
    }
    
    // Verify that the record originally at idx3 (Bob Builder) is now at index 1
    // and its internal ID has been updated to 1.
    char *shifted_record = db_search_index(1);
    if (shifted_record) {
        printf("\nRecord at index 1 (originally index %u, now shifted):\n", idx3);
        print_record_helper(shifted_record);
    }

    return 0;
}