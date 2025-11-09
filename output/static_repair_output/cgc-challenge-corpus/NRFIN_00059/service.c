#include <stdbool.h> // Required for 'true'

// Forward declarations for functions used in main
void setup(void);
short process_cmd(void);
void send_status(const int* status);

// Forward declarations for constants used in main
// Assuming these are integer constants defined elsewhere
extern const int STATUS_OK;
extern const int STATUS_ERR;
extern const int STATUS_QUIT;

// Function: main
int main(void) {
  short sVar1; // This variable is necessary to store the result of process_cmd()
               // and cannot be eliminated without altering the logic.

  setup();
  while(true) {
    while(true) {
      sVar1 = process_cmd();
      if (sVar1 != 0) {
        break; // Breaks out of the inner loop
      }
      send_status(&STATUS_OK);
    }
    if (sVar1 != -1) {
      break; // Breaks out of the outer loop
    }
    send_status(&STATUS_ERR);
  }
  send_status(&STATUS_QUIT);
  return 0;
}