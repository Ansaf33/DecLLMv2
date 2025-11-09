#include <stdlib.h> // For malloc, free, exit, random
#include <string.h> // For memcpy
#include <stdint.h> // For uint8_t, uint32_t, uintptr_t, size_t

// Type definitions from disassembler output
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint32_t undefined4;

// Global variables (mocked or actual)
char g_protocol = '\0'; // Assuming g_protocol is a single byte/char
int g_incoming_report_type = -1; // Assuming g_incoming_report_type is an int
byte g_incoming_report[600]; // Max size based on `if (600 < __n)`

// Mock external functions for compilation
void session_send(uint32_t param_1, size_t param_2, void *param_3, uint32_t param_4) { /* ... */ }
void session_register_userdata(uint32_t param_1, int param_2) { /* ... */ }
void session_register_events(uint32_t param_1, void *handler) { /* ... */ }
void session_register_psm(uint32_t psm, void *handler) { /* ... */ }
void session_loop(void *param_1) { /* ... */ }

// Forward declarations
void send_data(uint32_t param_1, byte param_2, size_t param_3, void *param_4);
void send_handshake(uint32_t param_1, uint8_t param_2);
void send_report(uint32_t param_1);
// handle_control_packet changed to return void* to be compatible with handle_event's assignment.
void* handle_control_packet(uint32_t param_1, uint32_t param_2, byte *param_3);
// handle_event's param_3 type changed to uintptr_t* to safely handle pointer values, and returns void**
void ** handle_event(uint32_t param_1, int param_2, uintptr_t *param_3);
void handle_interrupt_connect(uint32_t param_1);
void handle_control_connect(uint32_t param_1); // Mocked function, defined below
uint32_t main(void); // main returns uint32_t

// Function: send_data
void send_data(uint32_t param_1, byte param_2, size_t param_3, void *param_4) {
  byte *ptr = (byte *)malloc(param_3 + 1);
  if (ptr == NULL) {
      // In a real application, proper error handling (e.g., logging, returning an error code)
      // would be needed here. For this context, we'll just return.
      return; 
  }
  *ptr = param_2 | 0xa0;
  memcpy(ptr + 1, param_4, param_3);
  session_send(param_1, param_3 + 1, ptr, 0x1150e);
  free(ptr);
}

// Function: send_handshake
void send_handshake(uint32_t param_1, uint8_t param_2) {
  uint8_t local_d[1];
  local_d[0] = param_2;
  session_send(param_1, 1, local_d, 0x11586);
}

// Function: send_report
void send_report(uint32_t param_1) {
  random(); // POSIX random, requires <stdlib.h>
  
  size_t report_size = (g_protocol == '\0') ? 200 : 700;
  
  uint8_t local_32c[804]; 
  
  send_data(param_1, 1, report_size, local_32c);
}

// Function: handle_control_packet
void* handle_control_packet(uint32_t param_1, uint32_t param_2, byte *param_3) {
  if (param_2 == 0) {
      return NULL;
  }

  byte packet_type_and_subtype = *param_3;
  byte subtype = packet_type_and_subtype & 0xf;
  byte type = packet_type_and_subtype >> 4;

  if ((g_incoming_report_type == -1) || (type == 10)) {
    switch (type) {
      default:
        send_handshake(param_1, 3);
        break;
      case 1:
        if (subtype == 5) {
          exit(0);
        }
        break;
      case 4:
        if (subtype == 1) {
          send_report(param_1);
        } else if (subtype == 2) {
          send_data(param_1, 2, 600, g_incoming_report);
        } else {
          send_handshake(param_1, 4);
        }
        break;
      case 5:
        if (subtype == 2) {
          g_incoming_report_type = 2;
        } else {
          send_handshake(param_1, 4);
        }
        break;
      case 6:
        send_data(param_1, 0, 1, &g_protocol);
        break;
      case 7:
        g_protocol = *param_3 & 1;
        send_handshake(param_1, 0);
        break;
      case 10:
        if (g_incoming_report_type == -1) {
          send_handshake(param_1, 1);
        } else {
          if (1 < param_2) {
            size_t data_len = param_2 - 1;
            if (600 < data_len) {
              data_len = 600;
            }
            memcpy(g_incoming_report, param_3 + 1, data_len);
          }
          send_handshake(param_1, 0);
          g_incoming_report_type = -1;
        }
        break;
      case 0xb: // 11 in decimal
        send_data(param_1, 0, 0x35, (void *)main);
    }
  } else {
    g_incoming_report_type = -1;
    send_handshake(param_1, 1);
  }
  return NULL; // Return NULL as a placeholder, original was void
}

// Function: handle_event
// param_3 changed to uintptr_t* to safely handle pointer values from the array.
void ** handle_event(uint32_t param_1, int param_2, uintptr_t *param_3) {
  void **returned_ptr = NULL;
  
  // The original disassembler output:
  // ppuVar1 = &_GLOBAL_OFFSET_TABLE_; // This line is dead code as ppuVar1 is immediately overwritten.
  // if ((param_2 == 0) && (ppuVar1 = (undefined **)*param_3, ppuVar1 == (undefined **)0x0))
  // This translates to: if param_2 is 0 AND the pointer at param_3[0] is NULL.
  // The assignment `ppuVar1 = (undefined **)*param_3` sets the default return value
  // if the condition is false. If true, handle_control_packet is called.
  
  if ((param_2 == 0) && ((void *)param_3[0] == NULL)) {
    // Call handle_control_packet, casting the uintptr_t values from param_3
    // to their expected types (uint32_t for param_2, byte* for param_3).
    returned_ptr = (void **)handle_control_packet(param_1, (uint32_t)param_3[2], (byte *)param_3[1]);
  } else {
    // If the condition is false, return the pointer value from param_3[0].
    returned_ptr = (void **)param_3[0];
  }
  return returned_ptr;
}

// Function: handle_interrupt_connect
void handle_interrupt_connect(uint32_t param_1) {
  session_register_userdata(param_1, 1);
  session_register_events(param_1, (void*)handle_event);
}

// Mock function for handle_control_connect, as it's called but not defined in the snippet.
void handle_control_connect(uint32_t param_1) {
  session_register_userdata(param_1, 0); // Placeholder for control connection setup
  // Additional setup for the control connection can go here
}

// Function: main
uint32_t main(void) {
  // `puVar1 = &stack0x00000004;` is a disassembler artifact and not standard C. Removed.
  // `session_loop` likely expects a context pointer, passing NULL for now.
  session_register_psm(0x1011, (void*)handle_control_connect);
  session_register_psm(0x1013, (void*)handle_interrupt_connect);
  session_loop(NULL);
  return 0;
}