#include <stdint.h> // For uint32_t, uint8_t
#include <stddef.h> // For size_t

// --- External Declarations (required for compilation) ---

// Global variables modified by gen_status_codes and accessed by send_status
// Assuming these are uint8_t arrays of size 2 based on the access patterns.
uint8_t STATUS_OK[2];
uint8_t STATUS_ERR[2];
uint8_t STATUS_QUIT[2];

// External data array DAT_4347c000.
// It's accessed up to index local_c + 2 where local_c goes up to 0xffe - 3.
// So, it needs to be at least (0xffe - 3) + 2 + 1 = 0xffe elements long (4094 bytes).
// Providing a dummy definition for compilation. In a real scenario, this would be defined elsewhere.
const uint8_t DAT_4347c000[4096] = {0}; // Example: All zeros, adjust as needed.

// External function declarations
extern int send_bytes(int fd, const void *buf, size_t count, int flags);
extern void _terminate(int status); // Often does not return
extern void setup(void);
extern short process_cmd(void);

// --- Function Implementations from Snippet ---

// Function: gen_status_codes
void gen_status_codes(void) {
  for (uint32_t local_c = 0; local_c < 0xffe; local_c += 3) {
    STATUS_OK[local_c & 1] ^= DAT_4347c000[local_c];
    STATUS_ERR[(local_c + 1) & 1] ^= DAT_4347c000[local_c + 1];
    STATUS_QUIT[local_c & 1] ^= DAT_4347c000[local_c + 2];
  }
}

// Function: send_status
void send_status(const void *param_1) {
  if (send_bytes(1, param_1, 2, 0x17449) != 2) {
    _terminate(0xfffffff7); // 0xfffffff7 is -9 in signed 32-bit integer
  }
}

// Function: main
int main(void) {
  short sVar1;

  gen_status_codes();
  setup();
  while (1) { // Infinite loop (equivalent to while(true))
    while (1) { // Infinite loop
      sVar1 = process_cmd();
      if (sVar1 != 0) {
        break; // Break from inner loop
      }
      send_status(&STATUS_OK);
    }
    if (sVar1 != -1) {
      break; // Break from outer loop
    }
    send_status(&STATUS_ERR);
  }
  send_status(&STATUS_QUIT);
  return 0;
}