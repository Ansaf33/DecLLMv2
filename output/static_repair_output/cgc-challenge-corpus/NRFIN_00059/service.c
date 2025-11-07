#include <stdbool.h> // For 'true'
#include <stdio.h>   // For printf in placeholder functions
#include <stdlib.h>  // For exit, though not strictly required by the snippet, good for main

// Define a simple Status struct to represent status messages
typedef struct Status {
    const char* message;
} Status;

// Define status constants
const Status STATUS_OK = {"OK"};
const Status STATUS_ERR = {"ERROR"};
const Status STATUS_QUIT = {"QUIT"};

// Placeholder for setup function
void setup(void) {
    printf("System setup complete.\n");
}

// Placeholder for process_cmd function
// This function simulates different return values to test the main loop's logic.
// It will return 0 twice, then -1 once, then 1 once.
static int cmd_call_count = 0;
short process_cmd(void) {
    cmd_call_count++;
    if (cmd_call_count <= 2) { // Returns 0 for the first two calls
        printf("  [process_cmd] Call %d: Returning 0 (continue inner loop)\n", cmd_call_count);
        return 0;
    } else if (cmd_call_count == 3) { // Returns -1 for the third call
        printf("  [process_cmd] Call %d: Returning -1 (break inner, trigger outer ERR)\n", cmd_call_count);
        return -1;
    } else { // Returns 1 for the fourth call (or any subsequent calls)
        printf("  [process_cmd] Call %d: Returning 1 (break inner, trigger outer QUIT)\n", cmd_call_count);
        return 1;
    }
}

// Placeholder for send_status function
void send_status(const Status* status) {
    printf("  [send_status] Sending status: %s\n", status->message);
}

// Function: main
int main(void) {
  short sVar1; // Variable is necessary to store the result of process_cmd across checks

  setup();
  while(true) { // Outer loop
    while(true) { // Inner loop
      sVar1 = process_cmd();
      if (sVar1 != 0) { // If process_cmd returns non-zero, break the inner loop
        break;
      }
      send_status(&STATUS_OK); // If process_cmd returns 0, send OK and continue inner loop
    }
    // At this point, sVar1 holds the value that caused the inner loop to break.
    if (sVar1 != -1) { // If sVar1 is not -1 (e.g., 1, 2, etc.), break the outer loop
      break;
    }
    send_status(&STATUS_ERR); // If sVar1 was -1, send ERR and continue the outer loop
  }
  send_status(&STATUS_QUIT); // After the outer loop breaks, send QUIT
  return 0;
}