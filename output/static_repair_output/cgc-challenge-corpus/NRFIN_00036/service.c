#include <stdio.h>    // For snprintf (if int2str is implemented using it)
#include <stdlib.h>   // For exit
#include <string.h>   // For memset, strlen, strcmp
#include <sys/socket.h> // For recv
#include <sys/types.h>  // For ssize_t
#include <stdint.h>   // For uint32_t

// Simplified placeholder for _terminate
#define _terminate(status) exit(status)

// External function declarations (placeholders as their implementations are not provided)
// The flags/debug_flag arguments (like 0x115d1) are assumed to be custom debug/logging flags,
// not standard recv flags, so they are kept as int.
extern int recvline(int fd, void *buf, size_t maxlen, int debug_flag);
extern int getSize(uint32_t *report_data);
extern int transmit_all(size_t size, const void *buf, int channel);
extern int int2str(int value, size_t buffer_size, char *buffer, int debug_flag);
extern int setMap(int value, char *key, uint32_t **map_handle_ptr);
extern uint32_t getValue(char *key, uint32_t *map_handle);
extern void removeMap(char *key, uint32_t **map_handle_ptr);

// Function: receiveInstruction
void receiveInstruction(uint32_t *param_1) {
  if (recv(0, param_1, 4, 0x115d1) < 0) {
    _terminate(2);
  }
}

// Function: receiveValue
void receiveValue(int *param_1) {
  if (recv(0, param_1, 4, 0x1160e) < 0) {
    _terminate(2);
  }
}

// Function: receiveKey
void receiveKey(char *param_1) {
  memset(param_1, 0, 0x18); // 0x18 is 24 bytes
  if (recvline(0, param_1, 0x17, 0x1164b) < 0) { // 0x17 for maxlen, allowing null termination
    _terminate(2);
  }
}

// Function: sendReport
void sendReport(uint32_t *report_list_head) {
  char report_buffer[0x28]; // Buffer for int2str results (40 bytes)
  int total_sum = 0;
  uint32_t *current_item = report_list_head;
  
  memset(report_buffer, 0, sizeof(report_buffer)); // Initialize buffer

  int num_items = getSize(report_list_head);
  
  while (num_items > 0) {
    total_sum += (int)current_item[1]; // current_item[1] is the value
    
    // Transmit data part (current_item + 2, which is 6 uint32_t's or 24 bytes)
    if (transmit_all(0x18, (void *)(current_item + 2), 1) != 0) {
      _terminate(1);
    }

    // Convert current_item[1] (value) to string and transmit
    if (int2str((int)current_item[1], sizeof(report_buffer), report_buffer, 0x11750) != 0) {
      _terminate(1);
    }
    if (transmit_all(sizeof(report_buffer), report_buffer, 1) != 0) {
      _terminate(1);
    }

    // Clear buffer for next iteration
    memset(report_buffer, 0, sizeof(report_buffer));
    
    current_item = (uint32_t *)*current_item; // Move to next item: *current_item is a pointer
    num_items--;
  }
  
  // Transmit final total_sum
  if (int2str(total_sum, sizeof(report_buffer), report_buffer, 0x117c9) != 0) {
    _terminate(1);
  }
  if (transmit_all(sizeof(report_buffer), report_buffer, 1) != 0) {
    _terminate(1);
  }
}

// Function: main
int main(void) {
  uint32_t instruction; // Instruction code received from receiveInstruction
  char key_buffer[24]; // Buffer for key (0x18 bytes)
  int value_int; // Value received from receiveValue
  uint32_t *map_handle = NULL; // Pointer to the head of the map data structure

  char int_str_buffer[0x28]; // Buffer for int2str results in case 3 (40 bytes)
  memset(int_str_buffer, 0, sizeof(int_str_buffer)); // Initialize once

  do {
    receiveInstruction(&instruction);

    if (instruction == 1) { // Create/Set budget entry
      receiveKey(key_buffer);
      receiveValue(&value_int);
      
      if (value_int < 0) {
        continue; // Restart loop if value is negative
      }
      
      if (setMap(value_int, key_buffer, &map_handle) == 1) { // setMap returns 1 on new creation
        if (transmit_all(0x14, "New budget created!\n", 1) != 0) {
          _terminate(1);
        }
      } else { // No more entries / update existing
        if (transmit_all(0x10, "No more entries\n", 1) != 0) {
          _terminate(1);
        }
      }
    } else if (instruction == 2) { // Deduct from budget
      receiveKey(key_buffer);
      receiveValue(&value_int);
      
      int current_budget = (int)getValue(key_buffer, map_handle);
      current_budget -= value_int;
      
      setMap(current_budget, key_buffer, &map_handle); // Update map with new budget
      
      if (current_budget < 0) {
        size_t key_len = strlen(key_buffer);
        if (transmit_all(key_len, key_buffer, 1) != 0) {
          _terminate(1);
        }
        if (transmit_all(0x11, " is over budget!\n", 1) != 0) {
          _terminate(1);
        }
      }
    } else if (instruction == 3) { // Report budget for key
      receiveKey(key_buffer);
      uint32_t budget_value = getValue(key_buffer, map_handle);
      
      if (int2str((int)budget_value, sizeof(int_str_buffer), int_str_buffer, 0x11a2c) != 0) {
        _terminate(1);
      }
      if (transmit_all(sizeof(int_str_buffer), int_str_buffer, 1) != 0) {
        _terminate(1);
      }
    } else if (instruction == 6) { // Remove key (with "BACON" check)
      receiveKey(key_buffer);
      if (strcmp(key_buffer, "BACON") == 0) {
        removeMap(key_buffer, &map_handle);
      }
    } else if (instruction == 7) { // Send full report
      sendReport(map_handle);
    }
    
    // Exit condition for the main loop
    if (instruction > 7) {
      return 0; // Exit program
    }
  } while (1); // Loop indefinitely until instruction > 7
}