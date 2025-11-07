#include <stdlib.h> // For malloc, free, exit, rand
#include <string.h> // For memcpy
#include <stddef.h> // For size_t
#include <stdint.h> // For uint32_t, uint8_t

// Custom type definitions for clarity and portability
typedef uint32_t undefined4;
typedef uint8_t byte;
typedef uint8_t undefined;

// Global variables
uint8_t g_protocol = 0; // Initialized based on g_protocol == '\0' check
int g_incoming_report_type = -1; // Initialized based on g_incoming_report_type == -1 check
uint8_t g_incoming_report[600]; // Buffer for incoming reports, size inferred from usage

// Forward declarations for external functions (assuming their signatures based on usage)
// Note: These functions are not provided in the snippet and are assumed to exist.
void session_send(uint32_t param_1, size_t size, void *data, uint32_t param_4);
void session_register_userdata(uint32_t param_1, int param_2);
// handle_event's signature: void* (*handler)(uint32_t, int, int*)
void session_register_events(uint32_t param_1, void* (*handler)(uint32_t, int, int*));
void session_register_psm(uint32_t psm_id, void (*handler)(uint32_t));
void session_loop(void *context);

// Forward declaration for main (used in send_data for case 0xb)
int main(void);

// Dummy function for handle_control_connect as it's registered but not defined in the snippet
void handle_control_connect(uint32_t param_1) {
    // Placeholder for actual implementation
}

// Forward declaration for handle_event
void* handle_event(uint32_t param_1, int param_2, int *param_3);

// Function: handle_control_packet (modified to return void* for handle_event's usage)
void* handle_control_packet(uint32_t param_1, uint32_t param_2, uint8_t *param_3) {
    if (param_2 == 0) {
        return NULL; // Early exit if no data
    }

    uint8_t command = *param_3 >> 4;
    uint8_t subcommand = *param_3 & 0xf;

    if ((g_incoming_report_type == -1) || (command == 10)) {
        switch (command) {
            default:
                send_handshake(param_1, 3);
                break;
            case 1:
                if (subcommand == 5) {
                    exit(0);
                }
                break;
            case 4:
                if (subcommand == 1) {
                    send_report(param_1);
                } else if (subcommand == 2) {
                    send_data(param_1, 2, 600, g_incoming_report);
                } else {
                    send_handshake(param_1, 4);
                }
                break;
            case 5:
                if (subcommand == 2) {
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
                    if (param_2 > 1) {
                        size_t bytes_to_copy = param_2 - 1;
                        if (bytes_to_copy > 600) {
                            bytes_to_copy = 600;
                        }
                        memcpy(g_incoming_report, param_3 + 1, bytes_to_copy);
                    }
                    send_handshake(param_1, 0);
                    g_incoming_report_type = -1;
                }
                break;
            case 0xb: // 11
                send_data(param_1, 0, 0x35, (void*)main); // Casting main function pointer to void*
                break;
        }
    } else {
        g_incoming_report_type = -1;
        send_handshake(param_1, 1);
    }
    return NULL; // handle_control_packet original return type was void, but handle_event expects void*.
                 // Returning NULL as a dummy value for compilation.
}

// Function: send_data
void send_data(uint32_t param_1, uint8_t param_2, size_t param_3, void *param_4) {
    uint8_t *data_buffer = (uint8_t *)malloc(param_3 + 1);
    if (!data_buffer) {
        // Handle allocation failure, e.g., exit or return an error.
        // For this snippet, we assume success or ignore failure.
        return;
    }
    data_buffer[0] = param_2 | 0xa0;
    memcpy(data_buffer + 1, param_4, param_3);
    session_send(param_1, param_3 + 1, data_buffer, 0x1150e);
    free(data_buffer);
}

// Function: send_handshake
void send_handshake(uint32_t param_1, uint8_t param_2) {
    // Directly passing the address of param_2, as only one byte is needed.
    session_send(param_1, 1, &param_2, 0x11586);
}

// Function: send_report
void send_report(uint32_t param_1) {
    uint8_t dummy_buffer[804]; // Buffer for data, content not initialized by original code
    rand(); // Original code called random(), result was unused. Using rand() from stdlib.h.
    send_data(param_1, 1, (g_protocol == 0) ? 200 : 700, dummy_buffer);
}

// Function: handle_event
void* handle_event(uint32_t param_1, int param_2, int *param_3) {
    // _GLOBAL_OFFSET_TABLE_ is a linker-defined symbol. For compilation, a dummy is used.
    // The original code implies it's a pointer.
    static void* global_offset_table_dummy = NULL;

    void* result_ptr = &global_offset_table_dummy; // Initial value based on decompiler output

    if (param_2 == 0) {
        // The original logic was `(ppuVar1 = (undefined **)*param_3, ppuVar1 == (undefined **)0x0)`
        // which means assign `*param_3` to ppuVar1, then check if ppuVar1 is 0x0.
        // If it is 0x0, then call handle_control_packet. Otherwise, return the assigned value.
        void* temp_param_ptr = (void*)(intptr_t)*param_3; // Cast int to pointer for comparison

        if (temp_param_ptr == NULL) {
            // param_3[2] is the size (uint32_t), param_3[1] is the data pointer (uint8_t*).
            // Casts are necessary due to `param_3` being `int*`.
            result_ptr = handle_control_packet(param_1, (uint32_t)param_3[2], (uint8_t*)(intptr_t)param_3[1]);
        } else {
            result_ptr = temp_param_ptr;
        }
    }
    return result_ptr;
}

// Function: handle_interrupt_connect
void handle_interrupt_connect(uint32_t param_1) {
    session_register_userdata(param_1, 1);
    session_register_events(param_1, handle_event);
}

// Function: main
int main(void) {
    // The original `puVar1 = &stack0x00000004;` is a decompiler artifact
    // representing a stack address. Using NULL or a dummy context for portability.
    void* session_context = NULL;

    session_register_psm(0x1011, handle_control_connect);
    session_register_psm(0x1013, handle_interrupt_connect);
    session_loop(session_context);
    return 0;
}