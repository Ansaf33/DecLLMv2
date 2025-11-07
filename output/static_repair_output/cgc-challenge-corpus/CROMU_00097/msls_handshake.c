#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h> // For va_list, va_start, va_end

// Define custom types based on common decompiler output
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint16_t ushort;
typedef uint32_t uint;

// --- Stub/Placeholder Functions ---

void msls_send_error_msg(uint32_t code, uint32_t sub_code) {
    fprintf(stderr, "ERROR: msls_send_error_msg(0x%x, 0x%x)\n", code, sub_code);
}

void debug_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

// Dummy message structure assumed based on send functions:
// full_msg (0x18 bytes) -> msg_header (0x10 bytes) -> msg_data (variable size)
void msls_send_msg(void *msg) {
    debug_print("msls_send_msg called with message at %p\n", msg);
}

void msls_destroy_msg(void *msg) {
    if (msg) {
        uint16_t *full_msg = (uint16_t *)msg;
        // full_msg[10] (offset 0x14) is a pointer to msg_header (e.g., puVar2)
        uint16_t *msg_header = *(uint16_t **)((char *)full_msg + 10 * sizeof(uint16_t));
        if (msg_header) {
            // msg_header[6] (offset 0xC) is a pointer to msg_data (e.g., puVar3/pvVar3)
            void *msg_data = *(void **)((char *)msg_header + 6 * sizeof(uint16_t));
            if (msg_data) {
                free(msg_data);
            }
            free(msg_header);
        }
        free(full_msg);
    }
}

uint32_t get_random() {
    // Simple pseudo-random number for stubs
    return (uint32_t)rand();
}

// --- Forward Declarations ---
uint32_t *msls_lookup_context(int server_ctx_ptr, uint32_t client_id_to_find);
void destroy_context(uint32_t *conn_ctx_ptr);
uint32_t *msls_get_connection(int server_ctx_ptr, uint32_t client_id);
void msls_send_finish(uint32_t *conn_ctx_ptr);
void msls_send_hello_verify(int server_ctx_ptr, uint32_t client_id);
void msls_send_server_hello(uint32_t *conn_ctx_ptr);
void msls_send_keyx(uint32_t *conn_ctx_ptr);
void msls_send_hello_done(uint32_t *conn_ctx_ptr);


// --- Global Magic Addresses (WARNING: These are hardcoded addresses and will likely cause runtime errors if not properly mapped) ---
// These are likely pointers to global arrays or memory regions.
// For compilation, we'll treat them as uintptr_t and cast to char* for arithmetic.
// In a real application, these should be symbolic addresses or dynamically allocated.
#define MAGIC_ADDR_0x4347c600 ((uintptr_t)0x4347c600)
#define MAGIC_ADDR_0x4347c400 ((uintptr_t)0x4347c400)
#define MAGIC_ADDR_0x4347c000 ((uintptr_t)0x4347c000)
#define MAGIC_ADDR_0x4347c200 ((uintptr_t)0x4347c200)
#define MAGIC_ADDR_0x10d1f080 ((uintptr_t)0x10d1f080)
#define MAGIC_ADDR_0x10d1f000 ((uintptr_t)0x10d1f000)


// Function: msls_handle_handshake
void msls_handle_handshake(int server_ctx_ptr, int msls_msg_ptr) {
    if ((server_ctx_ptr != 0) && (msls_msg_ptr != 0)) {
        // Check message length in the msls_msg_ptr structure
        if (*(uint16_t *)((char *)msls_msg_ptr + 0x10) < 0xc) {
            msls_send_error_msg(0xe0, 0xa1);
        } else {
            uint16_t *puVar5 = *(uint16_t **)((char *)msls_msg_ptr + 0x14); // Pointer to message payload
            uint32_t client_id = *(uint32_t *)((char *)msls_msg_ptr + 0xc); // Client ID from msls_msg_ptr

            // Compare client_id from payload with client_id from msls_msg_ptr
            if (*(uint32_t *)((char *)puVar5 + 3 * sizeof(uint16_t)) == client_id) {
                uint32_t *conn_ctx = msls_lookup_context(server_ctx_ptr, client_id); // Lookup connection context
                uint16_t msg_type = *puVar5; // Message type from payload

                if (msg_type == 0x59) { // Client Finished
                    debug_print("received client finished\n");
                    if (conn_ctx == NULL) {
                        msls_send_error_msg(0xe3, 0xa1);
                    } else if ((*(uint8_t *)((char *)conn_ctx + 4) == 0) || // Check flags in connection context
                               (*(uint8_t *)((char *)conn_ctx + 0xe) == 0) ||
                               (*(uint8_t *)((char *)conn_ctx + 0xd) == 0)) {
                        msls_send_error_msg(0xe5, 0xa0);
                    } else if (puVar5[5] == 0x200) { // Check expected length of payload
                        int iVar2 = *(int *)((char *)puVar5 + 1 * sizeof(uint16_t)); // Sequence number from payload
                        *(int *)((char *)conn_ctx + 8) = *(int *)((char *)conn_ctx + 8) + 1; // Increment connection sequence number
                        if (iVar2 == *(int *)((char *)conn_ctx + 8)) {
                            for (int local_18 = 0; local_18 < 0x80; local_18 = local_18 + 1) {
                                // Perform XOR validation using data in connection context
                                if (*(uint32_t *)((char *)puVar5 + (local_18 * 2 + 6) * sizeof(uint16_t)) !=
                                    (*(uint32_t *)((char *)(uintptr_t)conn_ctx[9] + local_18 * 4) ^
                                     *(uint32_t *)((char *)(uintptr_t)conn_ctx[8] + local_18 * 4) ^
                                     *(uint32_t *)((char *)(uintptr_t)conn_ctx[6] + local_18 * 4))) {
                                    msls_send_error_msg(0xe7, 0xa1);
                                    return;
                                }
                            }
                            *(uint8_t *)((char *)conn_ctx + 0xc) = 1; // Set flag
                            *(uint8_t *)((char *)conn_ctx + 4) = 0;   // Clear flag
                        } else {
                            msls_send_error_msg(0xe6, 0xa0); // Sequence number mismatch
                        }
                    } else {
                        debug_print("expected length: %d got length: %d\n", 0x200, puVar5[5]);
                        msls_send_error_msg(0xe0, 0xa1); // Length mismatch
                    }
                } else {
                    if (msg_type < 0x5a) {
                        if (msg_type == 0x58) { // Client Done
                            debug_print("Received Client Done\n");
                            if (conn_ctx == NULL) {
                                msls_send_error_msg(0xe3, 0xa1);
                                return;
                            }
                            if ((*(uint8_t *)((char *)conn_ctx + 4) == 0) || (*(uint8_t *)((char *)conn_ctx + 0xe) == 0)) {
                                msls_send_error_msg(0xe5, 0xa0);
                                return;
                            }
                            if (puVar5[5] != 0) { // Expected payload length is 0
                                debug_print("expected length: %d got length: %d\n", 0, puVar5[5]);
                                msls_send_error_msg(0xe0, 0xa1);
                                return;
                            }
                            int iVar2 = *(int *)((char *)puVar5 + 1 * sizeof(uint16_t)); // Sequence number
                            *(int *)((char *)conn_ctx + 8) = *(int *)((char *)conn_ctx + 8) + 1; // Increment connection sequence number
                            if (iVar2 != *(int *)((char *)conn_ctx + 8)) {
                                msls_send_error_msg(0xe6, 0xa0); // Sequence number mismatch
                                return;
                            }
                            msls_send_finish(conn_ctx); // Send finish message
                            *(uint8_t *)((char *)conn_ctx + 0xd) = 1; // Set flag
                            return;
                        }
                        if (msg_type < 0x59) {
                            if (msg_type == 0x51) { // Client Hello
                                debug_print("Received Client Hello\n");
                                if (puVar5[5] != 0x222) { // Expected payload length
                                    msls_send_error_msg(0xe0, 0xa1);
                                    return;
                                }
                                for (int local_10 = 0; local_10 < 0x80; local_10 = local_10 + 1) {
                                    // Cookie validation logic
                                    if (*(uint32_t *)((char *)puVar5 + (local_10 * 2 + 0xb) * sizeof(uint16_t)) !=
                                        (client_id ^ *(uint32_t *)((char *)*(int *)((char *)server_ctx_ptr + 0x88) + local_10 * 4))) {
                                        debug_print("Cookie failed\n");
                                        msls_send_hello_verify(server_ctx_ptr, client_id);
                                        return;
                                    }
                                }
                                debug_print("Done checking cookie\n");
                                conn_ctx = msls_get_connection(server_ctx_ptr, client_id); // Get or create connection context
                                if (conn_ctx == NULL) {
                                    msls_send_error_msg(0xe3, 0xa2);
                                    *(uint8_t *)((char *)server_ctx_ptr + 1) = 1; // Mark server context as error
                                    return;
                                }
                                // Initialize connection context flags and sequence number
                                *(uint8_t *)((char *)conn_ctx + 0xc) = 0;
                                *(uint8_t *)((char *)conn_ctx + 4) = 1;
                                *(uint8_t *)((char *)conn_ctx + 0xe) = 0;
                                *(uint8_t *)((char *)conn_ctx + 0xd) = 0;
                                *(uint32_t *)((char *)conn_ctx + 8) = 0;
                                uint32_t uVar4 = get_random();
                                *(uint16_t *)((char *)conn_ctx + 0x10) = puVar5[uVar4 % 0xb + 0x10b]; // Set some value based on random and payload
                                msls_send_server_hello(conn_ctx);
                                msls_send_keyx(conn_ctx);
                                msls_send_hello_done(conn_ctx);
                                if (conn_ctx[10] == 0) { // Check if conn_ctx[10] (offset 0x28) is null
                                    return;
                                }
                                free((void*)(uintptr_t)conn_ctx[10]); // Free memory pointed to by conn_ctx[10]
                                conn_ctx[10] = 0; // Clear the pointer
                                return;
                            }
                            if (msg_type == 0x56) { // Client Keyx
                                debug_print("Received Client Keyx\n");
                                if (conn_ctx == NULL) {
                                    msls_send_error_msg(0xe3, 0xa1);
                                    return;
                                }
                                if (*(uint8_t *)((char *)conn_ctx + 4) == 0) { // Check flag
                                    msls_send_error_msg(0xe5, 0xa0);
                                    return;
                                }
                                if (puVar5[5] != 0x400) { // Expected payload length
                                    msls_send_error_msg(0xe0, 0xa1);
                                    return;
                                }
                                int iVar2 = *(int *)((char *)puVar5 + 1 * sizeof(uint16_t)); // Sequence number
                                *(int *)((char *)conn_ctx + 8) = *(int *)((char *)conn_ctx + 8) + 1; // Increment connection sequence number
                                if (iVar2 != *(int *)((char *)conn_ctx + 8)) {
                                    msls_send_error_msg(0xe6, 0xa0); // Sequence number mismatch
                                    return;
                                }
                                uint16_t *key_data_ptr = puVar5 + 6; // Pointer to key data within payload
                                conn_ctx[5] = (uint32_t)(uintptr_t)calloc(1, 0x200); // Allocate memory and store pointer in conn_ctx[5] (offset 0x14)
                                memcpy((void*)(uintptr_t)conn_ctx[5], key_data_ptr, 0x200); // Copy key data

                                *(uint8_t *)((char *)conn_ctx + 0xe) = 1; // Set flag
                                conn_ctx[7] = (uint32_t)(uintptr_t)calloc(1, 0x200); // Allocate memory and store pointer in conn_ctx[7] (offset 0x1c)
                                for (int local_14 = 0; local_14 < 0x80; local_14 = local_14 + 1) {
                                    // Perform XOR operation and store in conn_ctx[7]
                                    *(uint32_t *)((char *)(uintptr_t)conn_ctx[7] + local_14 * 4) =
                                        *(uint32_t *)((char *)key_data_ptr + (local_14 + 0x80) * sizeof(uint16_t) * 2) ^
                                        *(uint32_t *)((char *)(uintptr_t)conn_ctx[6] + local_14 * 4); // conn_ctx[6] (offset 0x18)
                                }
                                return;
                            }
                        }
                    }
                    msls_send_error_msg(0xe4, 0xa2); // Unknown message type
                    *(uint8_t *)((char *)server_ctx_ptr + 1) = 1; // Mark server context as error
                }
            } else {
                debug_print("client_id 0x%x != connection id 0x%x\n",
                            *(uint32_t *)((char *)puVar5 + 3 * sizeof(uint16_t)),
                            client_id);
                msls_send_error_msg(0xe3, 0xa1); // Client ID mismatch
            }
        }
    }
    return;
}

// Function: msls_send_hello_verify
void msls_send_hello_verify(int server_ctx_ptr, uint32_t client_id) {
    uint16_t *full_msg = (uint16_t *)calloc(1, 0x18); // Allocate memory for the full message structure
    uint16_t *msg_header = (uint16_t *)calloc(1, 0x10); // Allocate memory for the message header
    uint16_t *msg_data = (uint16_t *)calloc(1, 0x202); // Allocate memory for the message data (payload)

    *msg_data = 0xff01; // Initialize first word of message data
    for (int local_10 = 0; local_10 < 0x80; local_10 = local_10 + 1) {
        // Populate message data with XORed values from server cookie and client ID
        *(uint32_t *)((char *)msg_data + (local_10 * 2 + 1) * sizeof(uint16_t)) =
            *(uint32_t *)((char *)*(int *)((char *)server_ctx_ptr + 0x88) + local_10 * 4) ^ client_id;
    }

    *msg_header = 0x52; // Message type
    *(uint32_t *)((char *)msg_header + 1 * sizeof(uint16_t)) = 0; // Sequence number (or similar)
    *(uint32_t *)((char *)msg_header + 3 * sizeof(uint16_t)) = client_id; // Client ID
    msg_header[5] = 0x202; // Payload length
    *(uint16_t **)((char *)msg_header + 6 * sizeof(uint16_t)) = msg_data; // Pointer to payload

    *full_msg = 0x21d; // Full message length
    full_msg[1] = 0xff01; // Some protocol version/flag
    *(uint8_t *)((char *)full_msg + 2 * sizeof(uint16_t)) = 3; // Type/version field
    *(uint32_t *)((char *)full_msg + 6 * sizeof(uint16_t)) = client_id; // Client ID
    full_msg[8] = 0x20e; // Length
    *(uint16_t **)((char *)full_msg + 10 * sizeof(uint16_t)) = msg_header; // Pointer to message header

    msls_send_msg(full_msg);
    msls_destroy_msg(full_msg);
}

// Function: msls_send_server_hello
void msls_send_server_hello(uint32_t *conn_ctx_ptr) {
    uint16_t *full_msg = (uint16_t *)calloc(1, 0x18);
    uint16_t *msg_header = (uint16_t *)calloc(1, 0x10);
    uint16_t *msg_data = (uint16_t *)calloc(1, 8); // 8 bytes for payload

    *msg_data = 0xff01;
    *(uint32_t *)((char *)msg_data + 1 * sizeof(uint16_t)) = get_random(); // Random value
    msg_data[3] = *(uint16_t *)((char *)conn_ctx_ptr + 4 * sizeof(uint32_t)); // Value from connection context

    *msg_header = 0x53; // Message type
    conn_ctx_ptr[2] = conn_ctx_ptr[2] + 1; // Increment sequence number in connection context
    *(uint32_t *)((char *)msg_header + 1 * sizeof(uint16_t)) = conn_ctx_ptr[2]; // Sequence number from context
    *(uint32_t *)((char *)msg_header + 3 * sizeof(uint16_t)) = *conn_ctx_ptr; // Client ID from context
    msg_header[5] = 8; // Payload length
    *(uint16_t **)((char *)msg_header + 6 * sizeof(uint16_t)) = msg_data; // Pointer to payload

    *full_msg = 0x23; // Full message length
    full_msg[1] = 0xff01;
    *(uint8_t *)((char *)full_msg + 2 * sizeof(uint16_t)) = 3;
    *(uint32_t *)((char *)full_msg + 6 * sizeof(uint16_t)) = *conn_ctx_ptr;
    full_msg[8] = 0x14;
    *(uint16_t **)((char *)full_msg + 10 * sizeof(uint16_t)) = msg_header;

    msls_send_msg(full_msg);
    msls_destroy_msg(full_msg);
}

// Function: msls_send_keyx
void msls_send_keyx(uint32_t *conn_ctx_ptr) {
    uint16_t *full_msg = (uint16_t *)calloc(1, 0x18);
    uint16_t *msg_header = (uint16_t *)calloc(1, 0x10);
    
    // Allocate memory for data and store pointer in conn_ctx_ptr[6] (offset 0x18)
    conn_ctx_ptr[6] = (uint32_t)(uintptr_t)calloc(1, 0x200);

    void *pvVar3 = calloc(1, 0x200); // Allocate memory for message payload data

    for (int local_10 = 0; local_10 < 0x80; local_10 = local_10 + 1) {
        // Perform XOR operation using magic addresses and store in conn_ctx_ptr[6]
        *(uint32_t *)((char *)(uintptr_t)conn_ctx_ptr[6] + local_10 * 4) =
            *(uint32_t *)((char *)MAGIC_ADDR_0x4347c600 + local_10 * 4) ^
            *(uint32_t *)((char *)MAGIC_ADDR_0x4347c400 + local_10 * 4);
        // Copy data from conn_ctx_ptr[6] to pvVar3
        *(uint32_t *)((char *)pvVar3 + local_10 * 4) =
            *(uint32_t *)((char *)(uintptr_t)conn_ctx_ptr[6] + local_10 * 4);
    }

    *msg_header = 0x55; // Message type
    conn_ctx_ptr[2] = conn_ctx_ptr[2] + 1; // Increment sequence number in connection context
    *(uint32_t *)((char *)msg_header + 1 * sizeof(uint16_t)) = conn_ctx_ptr[2]; // Sequence number
    *(uint32_t *)((char *)msg_header + 3 * sizeof(uint16_t)) = *conn_ctx_ptr; // Client ID
    msg_header[5] = 0x200; // Payload length
    *(void **)((char *)msg_header + 6 * sizeof(uint16_t)) = pvVar3; // Pointer to payload

    *full_msg = 0x21b; // Full message length
    full_msg[1] = 0xff01;
    *(uint8_t *)((char *)full_msg + 2 * sizeof(uint16_t)) = 3;
    *(uint32_t *)((char *)full_msg + 6 * sizeof(uint16_t)) = *conn_ctx_ptr;
    full_msg[8] = 0x20c;
    *(uint16_t **)((char *)full_msg + 10 * sizeof(uint16_t)) = msg_header;

    msls_send_msg(full_msg);
    msls_destroy_msg(full_msg);
}

// Function: msls_send_hello_done
void msls_send_hello_done(uint32_t *conn_ctx_ptr) {
    uint16_t *full_msg = (uint16_t *)calloc(1, 0x18);
    uint16_t *msg_header = (uint16_t *)calloc(1, 0x10);

    *msg_header = 0x57; // Message type
    conn_ctx_ptr[2] = conn_ctx_ptr[2] + 1; // Increment sequence number in connection context
    *(uint32_t *)((char *)msg_header + 1 * sizeof(uint16_t)) = conn_ctx_ptr[2]; // Sequence number
    *(uint32_t *)((char *)msg_header + 3 * sizeof(uint16_t)) = *conn_ctx_ptr; // Client ID
    msg_header[5] = 0; // Payload length (0 for hello done)
    *(uint32_t *)((char *)msg_header + 6 * sizeof(uint16_t)) = 0; // No payload data

    *full_msg = 0x1b; // Full message length
    full_msg[1] = 0xff01;
    *(uint8_t *)((char *)full_msg + 2 * sizeof(uint16_t)) = 3;
    *(uint32_t *)((char *)full_msg + 6 * sizeof(uint16_t)) = *conn_ctx_ptr;
    full_msg[8] = 0xc;
    *(uint16_t **)((char *)full_msg + 10 * sizeof(uint16_t)) = msg_header;

    msls_send_msg(full_msg);
    msls_destroy_msg(full_msg);
}

// Function: msls_send_finish
void msls_send_finish(uint32_t *conn_ctx_ptr) {
    uint16_t *full_msg = (uint16_t *)calloc(1, 0x18);
    uint16_t *msg_header = (uint16_t *)calloc(1, 0x10);

    // Allocate memory for data and store pointer in conn_ctx_ptr[8] (offset 0x20)
    conn_ctx_ptr[8] = (uint32_t)(uintptr_t)calloc(1, 0x200);
    void *pvVar3 = calloc(1, 0x200); // Allocate memory for message payload data

    for (int local_10 = 0; local_10 < 0x80; local_10 = local_10 + 1) {
        // Perform XOR operation using magic addresses and store in conn_ctx_ptr[8]
        *(uint32_t *)((char *)(uintptr_t)conn_ctx_ptr[8] + local_10 * 4) =
            *(uint32_t *)((char *)MAGIC_ADDR_0x4347c400 + local_10 * 4) ^
            *(uint32_t *)((char *)MAGIC_ADDR_0x4347c200 + local_10 * 4);
    }

    for (int local_14 = 0; local_14 < 0x80; local_14 = local_14 + 1) {
        // Perform XOR operations using data from various parts of connection context and store in pvVar3
        *(uint32_t *)((char *)pvVar3 + local_14 * 4) =
            *(uint32_t *)((char *)(uintptr_t)conn_ctx_ptr[8] + local_14 * 4) ^
            *(uint32_t *)((char *)(uintptr_t)conn_ctx_ptr[7] + local_14 * 4) ^
            *(uint32_t *)((char *)(uintptr_t)conn_ctx_ptr[5] + local_14 * 4) ^
            *(uint32_t *)((char *)(uintptr_t)conn_ctx_ptr[9] + local_14 * 4);
    }

    *msg_header = 0x59; // Message type
    conn_ctx_ptr[2] = conn_ctx_ptr[2] + 1; // Increment sequence number in connection context
    *(uint32_t *)((char *)msg_header + 1 * sizeof(uint16_t)) = conn_ctx_ptr[2]; // Sequence number
    *(uint32_t *)((char *)msg_header + 3 * sizeof(uint16_t)) = *conn_ctx_ptr; // Client ID
    msg_header[5] = 0x200; // Payload length
    *(void **)((char *)msg_header + 6 * sizeof(uint16_t)) = pvVar3; // Pointer to payload

    *full_msg = 0x21b; // Full message length
    full_msg[1] = 0xff01;
    *(uint8_t *)((char *)full_msg + 2 * sizeof(uint16_t)) = 3;
    *(uint32_t *)((char *)full_msg + 6 * sizeof(uint16_t)) = *conn_ctx_ptr;
    full_msg[8] = 0x20c;
    *(uint16_t **)((char *)full_msg + 10 * sizeof(uint16_t)) = msg_header;

    msls_send_msg(full_msg);
    msls_destroy_msg(full_msg);
}

// Function: msls_get_connection
uint32_t *msls_get_connection(int server_ctx_ptr, uint32_t client_id) {
    uint32_t *conn_ctx = NULL;

    if (server_ctx_ptr == 0) {
        return NULL;
    }

    conn_ctx = msls_lookup_context(server_ctx_ptr, client_id); // Try to find existing connection
    if (conn_ctx == NULL) {
        // If no existing connection, try to create one
        for (int local_10 = 0; local_10 < 0x20; local_10 = local_10 + 1) {
            // Check for an empty slot in the server's connection array (offset 8)
            if (*(uint32_t *)((char *)server_ctx_ptr + 8 + local_10 * 4) == 0) {
                conn_ctx = (uint32_t *)calloc(1, 0x2c); // Allocate 0x2c bytes for connection context
                debug_print("Created connection at %p\n", conn_ctx);
                *conn_ctx = client_id; // Store client ID as first word of context
                *(uint32_t **)((char *)server_ctx_ptr + 8 + local_10 * 4) = conn_ctx; // Store context pointer in server's array

                uint32_t *dest_magic_a = (uint32_t *)calloc(1, 0x200); // Allocate memory for magic data A
                uint32_t *dest_magic_b = (uint32_t *)calloc(1, 0x200); // Allocate memory for magic data B
                memcpy(dest_magic_a, (void *)MAGIC_ADDR_0x4347c000, 0x200); // Copy from magic address
                memcpy(dest_magic_b, (void *)MAGIC_ADDR_0x4347c200, 0x200); // Copy from magic address

                for (int local_14 = 0; local_14 < 0x80; local_14 = local_14 + 1) {
                    // Perform XOR operations on magic data A
                    dest_magic_a[local_14] =
                        dest_magic_b[local_14] ^
                        dest_magic_a[local_14] ^
                        client_id;
                }
                debug_print("leaving some magic at: %p\n", dest_magic_b);
                conn_ctx[10] = (uint32_t)(uintptr_t)dest_magic_b; // Store pointer in conn_ctx[10] (offset 0x28)
                conn_ctx[9] = (uint32_t)(uintptr_t)dest_magic_a; // Store pointer in conn_ctx[9] (offset 0x24)
                return conn_ctx;
            }
        }
        conn_ctx = NULL; // No empty slot found
    }
    return conn_ctx;
}

// Function: msls_lookup_context
uint32_t *msls_lookup_context(int server_ctx_ptr, uint32_t client_id_to_find) {
    if (server_ctx_ptr != 0) {
        for (int local_8 = 0; local_8 < 0x20; local_8 = local_8 + 1) {
            // Access connection context pointer from server_ctx_ptr's array (offset 8)
            uint32_t *conn_ctx_slot = *(uint32_t **)((char *)server_ctx_ptr + 8 + local_8 * 4);
            // If slot is not null and the client ID matches, return the context
            if ((conn_ctx_slot != NULL) && (client_id_to_find == *conn_ctx_slot)) {
                return conn_ctx_slot;
            }
        }
    }
    return NULL;
}

// Function: msls_destroy_connection
uint8_t **msls_destroy_connection(uint8_t **server_ctx_ptr, uint32_t client_id_to_destroy) {
    if (server_ctx_ptr != NULL) {
        for (int local_10 = 0; local_10 < 0x20; local_10 = local_10 + 1) {
            // Access connection context pointer from server_ctx_ptr's array (offset based on sizeof(uint8_t*))
            uint32_t *conn_ctx_slot = *(uint32_t **)((char *)server_ctx_ptr + (local_10 + 2) * sizeof(uint32_t *));
            // If slot is not null and the client ID matches, destroy the context
            if ((conn_ctx_slot != NULL) && (client_id_to_destroy == *conn_ctx_slot)) {
                destroy_context(conn_ctx_slot);
                // Clear the slot in the server's array
                *(uint32_t **)((char *)server_ctx_ptr + (local_10 + 2) * sizeof(uint32_t *)) = NULL;
                return server_ctx_ptr;
            }
        }
    }
    return server_ctx_ptr;
}

// Function: msls_set_cookie
void msls_set_cookie(int server_ctx_ptr) {
    void *pvVar1 = calloc(1, 0x200); // Allocate memory for cookie data
    *(void **)((char *)server_ctx_ptr + 0x88) = pvVar1; // Store pointer in server_ctx_ptr (offset 0x88)

    for (int local_10 = 0; local_10 < 0x80; local_10 = local_10 + 1) {
        // Populate cookie data by XORing values from magic addresses
        *(uint32_t *)((char *)*(int *)((char *)server_ctx_ptr + 0x88) + local_10 * 4) =
            *(uint32_t *)((char *)MAGIC_ADDR_0x10d1f080 + local_10 * 4) ^
            *(uint32_t *)((char *)MAGIC_ADDR_0x10d1f000 + local_10 * 4);
    }
    return;
}

// Function: destroy_context
void destroy_context(uint32_t *conn_ctx_ptr) {
    if (conn_ctx_ptr != NULL) {
        // Free allocated memory pointed to by various indices (offsets) within the connection context
        free((void*)(uintptr_t)conn_ctx_ptr[9]); // Offset 0x24 (9 * sizeof(uint32_t))
        free((void*)(uintptr_t)conn_ctx_ptr[5]); // Offset 0x14 (5 * sizeof(uint32_t))
        free((void*)(uintptr_t)conn_ctx_ptr[6]); // Offset 0x18 (6 * sizeof(uint32_t))
        free((void*)(uintptr_t)conn_ctx_ptr[8]); // Offset 0x20 (8 * sizeof(uint32_t))
        free((void*)(uintptr_t)conn_ctx_ptr[7]); // Offset 0x1c (7 * sizeof(uint32_t))
        free(conn_ctx_ptr); // Finally, free the connection context structure itself
    }
    return;
}