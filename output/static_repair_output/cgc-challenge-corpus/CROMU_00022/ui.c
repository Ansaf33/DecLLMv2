#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h> // For true/false

// Define standard types for decompiled placeholders
typedef unsigned int uint;
// undefined4 is often a 32-bit integer. Using int for general purpose,
// but unsigned int where it clearly represents an unsigned value like a timestamp or size.
// For this code, int seems appropriate for most undefined4 variables.
// For GetUInt32 return value, unsigned int is more accurate.

// --- Forward Declarations for external functions ---
// These are dummy declarations to make the provided code compilable.
// Their actual implementations would come from other compilation units.
extern unsigned int receive_until_flush(void *buffer, int timeout, unsigned int size, int magic_val);
extern int receive(int fd, void *buffer, unsigned int size, int *bytes_read);
extern int AddDive(void *param_1);
extern void DeleteDive(void *param_1, int dive_node_ptr);
extern void destroy_data(void *param_1);

// Replace _terminate with exit from stdlib.h
#define _terminate(status) exit(status)

// --- Placeholder for string literals ---
// In decompiled code, DAT_xxxx might refer to string literals.
// Assuming these are the prompt strings based on context.
const char *CITY_PROMPT = "City";
const char *DATE_PROMPT = "Date"; // Used for PADI Cert Date and Dive Date
const char *TIME_PROMPT = "Time"; // Used for Dive Time


// Function: GetLongString
unsigned int GetLongString(char *param_1) {
  memset(param_1, 0, 0x40); // 64 bytes
  unsigned int bytes_received = receive_until_flush(param_1, 10, 0x40, 0x115a7);
  if (bytes_received < 0x40) {
    param_1[bytes_received] = 0;
  }
  param_1[0x3f] = 0; // Ensure null termination at buffer end (index 63)
  return bytes_received;
}

// Function: GetShortString
unsigned int GetShortString(char *param_1) {
  memset(param_1, 0, 0x20); // 32 bytes
  unsigned int bytes_received = receive_until_flush(param_1, 10, 0x20, 0x11600);
  if (bytes_received < 0x20) {
    param_1[bytes_received] = 0;
  }
  param_1[0x1f] = 0; // Ensure null termination at buffer end (index 31)
  return bytes_received;
}

// Function: GetChar
char GetChar(void) {
  char buffer[6]; // Sufficient for 1 char + null
  receive_until_flush(buffer, 10, 2, 0x11659); // Read up to 2 bytes (char + newline maybe)
  if (islower((int)buffer[0])) {
    buffer[0] = buffer[0] - 0x20; // Convert to uppercase
  }
  return buffer[0];
}

// Function: GetInt
int GetInt(void) {
  char buffer[32]; // Sufficient for 10 digits + null + padding
  memset(buffer, 0, sizeof(buffer));
  int bytes_received = receive_until_flush(buffer, 10, 10, 0x116a8); // Read up to 10 chars
  buffer[bytes_received] = 0; // Null-terminate at actual end of received data
  
  int result = atoi(buffer);
  if (bytes_received == 0) {
    result = -999999;
  }
  return result;
}

// Function: GetPositiveInt
int GetPositiveInt(void) {
  int value = GetInt();
  if (value == -999999) {
    return -999999;
  } else if (value < 0) {
    return 0;
  }
  return value;
}

// Function: EpochToDate
// date_components_out expects an array of 6 integers: [month, day, year, hour, minute, second]
void EpochToDate(int *date_components_out, int epoch_seconds) {
    if (epoch_seconds < 0) {
        date_components_out[0] = 1; date_components_out[1] = 1; date_components_out[2] = 1970;
        date_components_out[3] = 0; date_components_out[4] = 0; date_components_out[5] = 0;
        return;
    }

    int years_passed = 0;
    int remaining_seconds_in_year = epoch_seconds; // Use a more descriptive name
    int current_year_seconds;
    int last_positive_seconds = epoch_seconds;

    // Calculate year
    // Loop until remaining_seconds_in_year becomes negative
    while (remaining_seconds_in_year >= 0) {
        last_positive_seconds = remaining_seconds_in_year; // Store the last positive value
        // The base year 1969 is derived from 0x7b1 in the original code (1969 + years_passed)
        bool is_leap_year_candidate = ((1969 + years_passed) % 4 == 0 && (1969 + years_passed) % 100 != 0) || ((1969 + years_passed) % 400 == 0);
        
        current_year_seconds = is_leap_year_candidate ? 31622400 : 31536000; // 366 days (0x1e28500) or 365 days (0x1e13380) in seconds
        
        remaining_seconds_in_year -= current_year_seconds;
        years_passed++;
    }
    date_components_out[2] = 1969 + years_passed - 1; // Year is 1969 + years_passed - 1 (since loop overshot)
    remaining_seconds_in_year = last_positive_seconds; // Restore the seconds for the current year

    // Calculate month and day
    // Index 0 unused, months 1-12
    int month_days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool is_current_year_leap = (date_components_out[2] % 4 == 0 && date_components_out[2] % 100 != 0) || (date_components_out[2] % 400 == 0);
    if (is_current_year_leap) {
        month_days[2] = 29; // Adjust February for leap year
    }

    int month = 1;
    long seconds_per_day = 86400; // 0x15180
    while (month <= 12) {
        long seconds_in_current_month = (long)month_days[month] * seconds_per_day;
        if (remaining_seconds_in_year < seconds_in_current_month) {
            break;
        }
        remaining_seconds_in_year -= seconds_in_current_month;
        month++;
    }
    date_components_out[0] = month; // Month

    int day = remaining_seconds_in_year / seconds_per_day + 1;
    remaining_seconds_in_year %= seconds_per_day;
    date_components_out[1] = day; // Day

    // Calculate hour, minute, second
    long seconds_per_hour = 3600; // 0xe10
    long seconds_per_minute = 60; // 0x3c

    date_components_out[3] = remaining_seconds_in_year / seconds_per_hour; // Hour
    remaining_seconds_in_year %= seconds_per_hour;
    date_components_out[4] = remaining_seconds_in_year / seconds_per_minute; // Minute
    date_components_out[5] = remaining_seconds_in_year % seconds_per_minute; // Second
}

// Function: MainMenu
void MainMenu(void *data_ptr) {
  char choice = 0;
  while (choice != 'X') {
    puts("");
    puts("C - Change Diver Info");
    puts("L - Log a New Dive");
    puts("D - Download Dive Data");
    puts("E - Edit Dives");
    puts("P - Print Dive Logs");
    puts("R - Remove Dives");
    puts("S - Diver Statistics");
    puts("X - Exit Application");
    printf(":");
    choice = GetChar();
    switch(choice) {
    case 'C':
      ChangeDiverInfo(data_ptr);
      break;
    case 'D':
      DownloadDiveData(data_ptr);
      break;
    case 'E':
      EditDive(data_ptr);
      break;
    case 'L':
      LogNewDive(data_ptr);
      break;
    case 'P':
      PrintDiveLogs(data_ptr);
      break;
    case 'R':
      RemoveDive(data_ptr);
      break;
    case 'S':
      DiverStatistics(data_ptr);
      break;
    case 'X':
      destroy_data(data_ptr);
      break;
    default:
      // Invalid choice, loop again
      break;
    }
  }
}

// Function: SetParam
void SetParam(const char *prompt, char *buffer, int max_len) {
  char input_buffer[64]; // Max buffer size for GetLongString is 64
  
  if (strcmp(buffer, "") == 0) {
    printf("%s: ", prompt);
  } else {
    printf("%s (%s): ", prompt, buffer);
  }
  
  int bytes_read = GetLongString(input_buffer);
  if (bytes_read > 0) {
    // Ensure null termination within the target buffer's max_len
    strncpy(buffer, input_buffer, max_len);
    buffer[max_len] = '\0';
  }
}

// Function: SetInt
void SetInt(const char *prompt, int *value_ptr) {
  if (*value_ptr == 0) {
    printf("%s: ", prompt);
  } else {
    printf("%s (%d): ", prompt, *value_ptr);
  }
  
  int input_val = GetInt();
  if (input_val != -999999) {
    *value_ptr = input_val;
  }
}

// Function: PrintDiverInfo
void PrintDiverInfo(const char *diver_info_base) {
  puts("");
  printf("     Name: %s %s\n", diver_info_base, diver_info_base + 0x20);
  printf("  Address: %s\n", diver_info_base + 0x40);
  printf("     City: %s\n", diver_info_base + 0x80);
  printf("    State: %s\n", diver_info_base + 0xa0);
  printf(" Zip Code: %s\n", diver_info_base + 0xc0);
  printf("    Phone: %s\n", diver_info_base + 0xe0);
  printf(" PADI Num: %s\n", diver_info_base + 0x100);
  printf("Cert Date: %s\n", diver_info_base + 0x120);
  puts("");
}

// Function: PrintDiveEntry
void PrintDiveEntry(const char *dive_entry_base) {
  puts("");
  printf("          Date: %s\n", dive_entry_base + 0x40);
  printf("          Time: %s\n", dive_entry_base + 0x60);
  printf("     Dive Site: %s\n", dive_entry_base);
  printf("      Location: %s\n", dive_entry_base + 0x80);
  printf("     Max Depth: %d\n", *(const int *)(dive_entry_base + 0xa0));
  printf("     Avg Depth: %d\n", *(const int *)(dive_entry_base + 0xa4));
  printf("      Duration: %d\n", *(const int *)(dive_entry_base + 0xa8));
  printf("    O2 Percent: %d\n", *(const int *)(dive_entry_base + 0xac));
  printf("Start Pressure: %d\n", *(const int *)(dive_entry_base + 0xb0));
  printf("  End Pressure: %d\n", *(const int *)(dive_entry_base + 0xb4));
  printf("     Bin Count: %d\n", *(const int *)(dive_entry_base + 0xb8));
  puts("");
}

// Function: ChangeDiverInfo
void ChangeDiverInfo(char *diver_data_base) {
  puts("");
  SetParam("First Name", diver_data_base, 0x14); // 20 bytes
  SetParam("Last Name", diver_data_base + 0x20, 0x14); // 20 bytes
  SetParam("Street", diver_data_base + 0x40, 0x1d); // 29 bytes
  SetParam(CITY_PROMPT, diver_data_base + 0x80, 0x13); // 19 bytes
  SetParam("State", diver_data_base + 0xa0, 2); // 2 bytes
  SetParam("Zip Code", diver_data_base + 0xc0, 10); // 10 bytes
  SetParam("Phone Number", diver_data_base + 0xe0, 0xf); // 15 bytes
  SetParam("PADI Diver Number", diver_data_base + 0x100, 0x13); // 19 bytes
  SetParam("PADI Cert Date", diver_data_base + 0x120, 10); // 10 bytes
  SanitizeDate(diver_data_base + 0x120);
  PrintDiverInfo(diver_data_base);
}

// Function: LogNewDive
void LogNewDive(void *data_ptr) {
  puts("");
  // Check if dive log has capacity (assuming 0x144 is a dive count, 0x80 is max)
  if (*(int *)((char *)data_ptr + 0x144) < 0x80) {
    int new_dive_ptr = AddDive(data_ptr);
    ChangeDive((char *)new_dive_ptr);
  }
}

// Function: SanitizeDate
void SanitizeDate(char *date_str) {
  char formatted_date[32];
  memset(formatted_date, 0, sizeof(formatted_date));

  int month = 0, day = 0, year = 0;
  int is_valid = 0;

  char *slash1 = strchr(date_str, '/');
  if (slash1 != NULL) {
    char *slash2 = strchr(slash1 + 1, '/');
    if (slash2 != NULL) {
      month = atoi(date_str);
      day = atoi(slash1 + 1);
      year = atoi(slash2 + 1);
    }
  }

  // Validate date components
  if ((year >= 1900 && year <= 2100) && (month >= 1 && month <= 12)) {
    if (month == 2) { // February
      bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
      if ((day >= 1 && day <= (is_leap ? 29 : 28))) {
        is_valid = 1;
      }
    } else if (month == 4 || month == 6 || month == 9 || month == 11) { // 30-day months
      if ((day >= 1 && day <= 30)) {
        is_valid = 1;
      }
    } else { // 31-day months
      if ((day >= 1 && day <= 31)) {
        is_valid = 1;
      }
    }
  }

  if (is_valid) {
    sprintf(formatted_date, "%d/%d/%d", month, day, year);
  } else {
    sprintf(formatted_date, "1/1/1900");
  }
  strcpy(date_str, formatted_date);
}

// Function: SanitizeTime
void SanitizeTime(char *time_str) {
  char formatted_time[16];
  memset(formatted_time, 0, sizeof(formatted_time));

  int hour = 0, minute = 0, second = 0;
  int is_valid = 0;

  char *colon1 = strchr(time_str, ':');
  if (colon1 != NULL) {
    char *colon2 = strchr(colon1 + 1, ':');
    if (colon2 != NULL) {
      hour = atoi(time_str);
      minute = atoi(colon1 + 1);
      second = atoi(colon2 + 1);
    }
  }

  if ((hour >= 0 && hour < 24) && (minute >= 0 && minute < 60) && (second >= 0 && second < 60)) {
    is_valid = 1;
  }
  
  if (is_valid) {
    sprintf(formatted_time, "%02d:%02d:%02d", hour, minute, second);
  } else {
    sprintf(formatted_time, "00:00:00");
  }
  strncpy(time_str, formatted_time, sizeof(formatted_time) - 1);
  formatted_time[sizeof(formatted_time) - 1] = '\0';
}

// Function: SanitizeDiveEntry
void SanitizeDiveEntry(char *dive_entry_base) {
  SanitizeDate(dive_entry_base + 0x40);
  SanitizeTime(dive_entry_base + 0x60);
}

// Function: ChangeDive
void ChangeDive(char *dive_entry_base) {
  SetParam("Dive Site", dive_entry_base, 0x19); // 25 bytes
  SetParam(DATE_PROMPT, dive_entry_base + 0x40, 10); // 10 bytes
  SetParam(TIME_PROMPT, dive_entry_base + 0x60, 0x20); // 32 bytes (seems large for time, but original used 0x20)
  SetParam("Location (area/city)", dive_entry_base + 0x80, 0x19); // 25 bytes
  SetInt("Max Depth in ft", (int *)(dive_entry_base + 0xa0));
  SetInt("Avg Depth in ft", (int *)(dive_entry_base + 0xa4));
  SetInt("Dive Duration (mins)", (int *)(dive_entry_base + 0xa8));
  SetInt("O2 Percentage", (int *)(dive_entry_base + 0xac));
  SetInt("Pressure In (psi)", (int *)(dive_entry_base + 0xb0));
  SetInt("Pressure Out (psi)", (int *)(dive_entry_base + 0xb4));
  // Bin Count (0xb8) is typically set by DownloadDiveData, not directly changed here.
  SanitizeDiveEntry(dive_entry_base);
}

// Function: GetUInt32
unsigned int GetUInt32(void) {
  unsigned int value = 0;
  int bytes_read = 0;
  int result = receive(0, &value, 4, &bytes_read);
  if ((result != 0) || (bytes_read == 0)) {
    _terminate(EXIT_FAILURE);
  }
  return value;
}

// Function: DownloadDiveData
void DownloadDiveData(void *data_ptr) {
  int dive_entry_ptr = AddDive(data_ptr);
  
  unsigned int min_timestamp = 0;
  unsigned int total_depth = 0;
  unsigned int max_timestamp = 0;
  unsigned int max_depth = 0;
  unsigned int bin_count = 0;

  puts("");
  unsigned int current_timestamp = GetUInt32();
  unsigned int current_depth = GetUInt32();

  // Initialize with first entry
  min_timestamp = current_timestamp;
  max_timestamp = current_timestamp;
  max_depth = current_depth;
  total_depth = current_depth;
  bin_count = 1;

  while (current_timestamp != 0 && bin_count < 100000) {
    current_timestamp = GetUInt32();
    if (current_timestamp != 0) {
      current_depth = GetUInt32();
      if (current_depth > 3000) { // Max depth clamp
        current_depth = 3000;
      }
      bin_count++;
      total_depth += current_depth;
      
      if (current_timestamp < min_timestamp) {
        min_timestamp = current_timestamp;
      }
      if (current_timestamp > max_timestamp) {
        max_timestamp = current_timestamp;
      }
      if (current_depth > max_depth) {
        max_depth = current_depth;
      }
    }
  }
  
  int date_time_components[6]; // [month, day, year, hour, minute, second]
  EpochToDate(date_time_components, min_timestamp);

  *(unsigned int *)((char *)dive_entry_ptr + 0xa0) = max_depth; // Max Depth
  *(unsigned int *)((char *)dive_entry_ptr + 0xa8) = (max_timestamp - min_timestamp) / 60; // Duration in minutes
  *(unsigned int *)((char *)dive_entry_ptr + 0xa4) = total_depth / bin_count; // Avg Depth
  *(unsigned int *)((char *)dive_entry_ptr + 0xb8) = bin_count; // Bin Count

  sprintf((char *)dive_entry_ptr + 0x40, "%d/%d/%d", date_time_components[0], date_time_components[1], date_time_components[2]);
  sprintf((char *)dive_entry_ptr + 0x60, "%02d:%02d:%02d", date_time_components[3], date_time_components[4], date_time_components[5]);
  
  ChangeDive((char *)dive_entry_ptr);
}

// Function: SelectDive
// Returns the pointer to the dive node (e.g., linked list node) or 0 if invalid
int SelectDive(void *data_ptr, const char *prompt) {
  // Assuming a max of 128 dives based on original array size 129 (128 + 1 for input)
  int dive_node_pointers[128]; 
  int current_node = *(int *)((char *)data_ptr + 0x140); // Head of dive list (assuming)
  unsigned int count = 0;
  int selected_dive_num = 0;
  
  if (current_node == 0) {
    printf("\n\nDive Log is empty\n");
    return 0;
  } else {
    printf("\n\nDive# Date       Time     Dive Site                 Location                 \n");
    while (current_node != 0 && count < 128) {
      int dive_entry_ptr = *(int *)((char *)current_node + 4); // Pointer to actual dive data
      dive_node_pointers[count] = current_node;
      count++;
      printf("%4u: %-10s %-8s", count, (char *)dive_entry_ptr + 0x40, (char *)dive_entry_ptr + 0x60);
      printf(" %-25s %-25s\n", (char *)dive_entry_ptr, (char *)dive_entry_ptr + 0x80);
      current_node = *(int *)((char *)current_node + 8); // Next node in list
    }
    printf("%s", prompt);
    selected_dive_num = GetPositiveInt();
    
    if (selected_dive_num < 1 || count < (unsigned int)selected_dive_num) {
      printf("Invalid dive number entered\n");
      return 0;
    } else {
      return dive_node_pointers[selected_dive_num - 1];
    }
  }
}

// Function: EditDive
void EditDive(void *data_ptr) {
  int selected_dive_node = SelectDive(data_ptr, "\nEnter Dive # to edit: ");
  if (selected_dive_node != 0) {
    puts("");
    ChangeDive((char *)(*(int *)((char *)selected_dive_node + 4))); // Pass pointer to actual dive data
  }
}

// Function: PrintDiveLogs
void PrintDiveLogs(void *data_ptr) {
  int selected_dive_node = SelectDive(data_ptr, "\nEnter Dive # to display: ");
  if (selected_dive_node != 0) {
    PrintDiveEntry((char *)(*(int *)((char *)selected_dive_node + 4))); // Pass pointer to actual dive data
  }
}

// Function: RemoveDive
void RemoveDive(void *data_ptr) {
  int selected_dive_node = SelectDive(data_ptr, "\nEnter Dive # to delete or blank to abort: ");
  if (selected_dive_node != 0) {
    DeleteDive(data_ptr, selected_dive_node);
  }
}

// Function: DiverStatistics
void DiverStatistics(void *data_ptr) {
  puts("");
  int current_node = *(int *)((char *)data_ptr + 0x140); // Head of dive list
  int total_dives = 0;
  int total_max_depth = 0;
  int total_dive_length = 0;
  
  if (current_node == 0) {
    printf("\nDive Log is empty\n");
  } else {
    for (; current_node != 0; current_node = *(int *)((char *)current_node + 8)) {
      total_dives++;
      // Access dive entry data through the pointer stored in the node (+4 offset)
      total_max_depth += *(int *)((char *)(*(int *)((char *)current_node + 4)) + 0xa0); // Max depth
      total_dive_length += *(int *)((char *)(*(int *)((char *)current_node + 4)) + 0xa8); // Duration
    }
    printf("\nDives logged: %d\n", total_dives);
    if (total_dives > 0) {
        printf("Average Max Depth: %d\n", total_max_depth / total_dives);
        printf("Average Dive Length: %d\n", total_dive_length / total_dives);
    } else {
        printf("No dives to calculate statistics.\n");
    }
  }
}