#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // For sockaddr_in, etc. (though not directly used by custom msghdr)
#include <string.h>     // For memset, memcpy, strcpy
#include <stdlib.h>     // For exit
#include <stdio.h>      // For fprintf (in _terminate)
#include <stdint.h>     // For uint32_t, uintptr_t, ssize_t

// --- Type Definitions (from decompiled 'undefined' types) ---
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned int undefined4; // Represents a 4-byte unsigned integer

// Custom msghdr structure inferred from `recvmsg` usage in the snippet.
// It's not the standard `struct msghdr` from sys/socket.h.
// The snippet implies:
// 1. `recv` reads 8 bytes into an instance of this struct.
// 2. `msg_name_size` (first field) is then interpreted as the size of the data payload.
// 3. `msg_iov_data_ptr` (second field) is then used as the buffer for the data payload.
typedef struct {
    uint32_t msg_name_size;    // Expected to hold the size of the subsequent data payload
    void*    msg_iov_data_ptr; // Expected to hold a pointer to the buffer for the data payload
} msghdr_custom;

// --- Global Variables and Structures (from decompiled `qss` and related accesses) ---

// The `qss` variable in the original snippet is used as a base address for offsets.
// `qss + iVar6 + 0xae623` implies `qss` points to a memory region.
// `iVar6` is deduced to be `0x119dd`.
// The total offset `0x119dd + 0xae623` is where a pointer to `msg_data_struct` is stored.
#define QSS_GLOBAL_BUFFER_PTR_OFFSET (0x119dd + 0xae623)
#define QSS_GLOBAL_BUFFER_SIZE (QSS_GLOBAL_BUFFER_PTR_OFFSET + sizeof(uintptr_t))
static unsigned char qss_mem_buffer[QSS_GLOBAL_BUFFER_SIZE];

// Structure representing the data that `msg_data_ptr` points to.
// Inferred from accesses like `*(int *)(*(int *)(qss + iVar6 + 0xae623) + 4)` etc.
// The `0x178` size is used in memcpy, suggesting `field10` is an array of that size.
typedef struct {
    unsigned int field0; // Accessed as `**(int **)(qss + iVar6 + 0xae623)`
    unsigned int field4; // Accessed as `*(int *)(*(int *)(qss + iVar6 + 0xae623) + 4)`
    byte field8;         // Accessed as `byte at offset +8`
    byte field9;         // Accessed as `byte at offset +9`
    undefined4 fieldC;   // Accessed as `undefined4 at offset +0xc`
    unsigned char field10[0x178]; // Data buffer, accessed from `iVar4 + 0x10`
} msg_data_struct;

// A static instance of the `msg_data_struct` to simulate global context.
static msg_data_struct global_msg_data;
static msg_data_struct* msg_data_ptr = &global_msg_data; // Pointer to the global context struct

// Constructor to initialize the global pointer in `qss_mem_buffer`.
// This simulates the decompiled code storing a pointer to `global_msg_data`
// at a specific offset within a global memory region.
__attribute__((constructor))
static void init_qss_global_pointer(void) {
    // Store the address of `global_msg_data` into `qss_mem_buffer` at the calculated offset.
    *(uintptr_t*)(qss_mem_buffer + QSS_GLOBAL_BUFFER_PTR_OFFSET) = (uintptr_t)msg_data_ptr;
}

// --- Dummy Implementations for external functions ---

// Placeholder for `sendall`.
// `sendall` is expected to send `len` bytes from `buf` to `fd` with `flags`.
static int sendall(int fd, const void *buf, size_t len, int flags) {
    // In a real application, this would loop to ensure all bytes are sent.
    ssize_t bytes_sent = send(fd, buf, len, flags);
    if (bytes_sent == (ssize_t)len) {
        return (int)len; // Success: all bytes sent
    } else if (bytes_sent < 0) {
        return (int)bytes_sent; // Error from send()
    }
    return 0; // Partial send (simplified for this placeholder)
}

// Placeholder for `_terminate`.
static void _terminate(void) {
    fprintf(stderr, "Program terminated due to an unrecoverable error.\n");
    exit(1);
}

// --- Original Functions with Fixes ---

// Function: custom_recvmsg (renamed from `recvmsg` to avoid conflict with standard library)
// This function implements a custom message reception logic.
ssize_t custom_recvmsg(int fd, msghdr_custom *message, int flags) {
    ssize_t bytes_received;
    uint32_t data_payload_size;

    // Original: `if ((uint)__flags < 8)` then return `0xfffffffd` (-3)
    // This could be a check for minimum acceptable flags or minimum expected size.
    if ((uint)flags < 8) {
        return -3; // Custom error code: invalid flags
    }

    // First `recv` call: reads 8 bytes into the `msghdr_custom` structure.
    bytes_received = recv(fd, message, 8, 0x118d3); // `0x118d3` is a magic flag from the original snippet

    // Original: `if (__n == (void *)0x8)`
    // Check if exactly 8 bytes (the size of our custom header) were received.
    if (bytes_received == 8) {
        // Original: `__n = __message->msg_name;`
        // Interprets the `msg_name_size` field of the custom header as the size of the actual data payload.
        data_payload_size = message->msg_name_size;

        // Original: `if (((uint)__flags < __n) || (__n < in_stack_00000010))`
        // The `in_stack_00000010` part is a decompilation artifact and removed.
        // `(uint)flags < data_payload_size` implies `flags` might specify a maximum allowed size.
        // Also, `data_payload_size` should be positive.
        if (data_payload_size == 0 || (uint)data_payload_size > (uint)flags) {
            return -1; // Custom error code: invalid data payload size
        }

        // Second `recv` call: reads the actual data payload into the buffer
        // pointed to by `message->msg_iov_data_ptr`.
        bytes_received = recv(fd, message->msg_iov_data_ptr, (size_t)data_payload_size, 0x118d3);

        // Original: `if (pvVar1 != __n)`
        // Check if the number of bytes received for the payload matches the expected `data_payload_size`.
        if (bytes_received != (ssize_t)data_payload_size) {
            return -2; // Custom error code: payload read mismatch
        }
        // If both header and payload reads are successful, return the size of the data payload.
        return (ssize_t)data_payload_size;
    } else if (bytes_received < 0) {
        return bytes_received; // Return actual `recv` error
    } else {
        return -4; // Custom error code: header read failed or incomplete
    }
}

// Function: senddgram
// `param_1`: Socket file descriptor.
// `param_2`: Pointer to a buffer, structured similarly to `msghdr_custom` for header and data.
int senddgram(int param_1, unsigned int *param_2) {
    int flags = 0x11972; // Magic flag from the original snippet

    // First `sendall`: sends the first 8 bytes (header-like) from `param_2`.
    int send_result = sendall(param_1, param_2, 8, flags);
    if (send_result == 8) {
        // Second `sendall`: sends the data payload.
        // `param_2 + 2` offsets the pointer by `2 * sizeof(unsigned int)` (8 bytes),
        // effectively pointing past the initial 8-byte header.
        // `*param_2` dereferences the first `unsigned int` in `param_2`, which is
        // interpreted as the length of the data payload.
        send_result = sendall(param_1, param_2 + 2, *param_2, flags);
    }
    return send_result;
}

// Function: create_resp_pkt
// `param_1`: Pointer to a buffer where the response packet will be constructed.
// `param_2`: String to be copied into the response packet.
void create_resp_pkt(unsigned int *param_1, char *param_2) {
    memset(param_1, 0, 0x17808);        // Clear a large buffer (0x17808 bytes)
    strcpy((char *)(param_1 + 2), param_2); // Copy string to offset 8 bytes into the buffer
    param_1[1] = 3;                     // Set a field at offset 4 bytes
    param_1[0] = 0x100;                 // Set a field at offset 0 bytes
    return;
}

// Function: recvdgram
undefined4 recvdgram(void) {
    // Local variables, simplified from decompiled stack offsets
    ushort remaining_chunks = 0;      // `local_e`: Number of chunks yet to receive
    ushort total_expected_chunks = 0; // `local_10`: Total number of chunks expected
    ushort received_chunks_count = 0; // `local_12`: Number of chunks received so far
    undefined4 final_field4_value = 5; // `local_18`: Value to update `msg_data_ptr->field4` with

    // Large stack buffer for storing received data chunks.
    // `0x17800` bytes total, divided into `0x178` byte chunks.
    // `0x17800 / 0x178 = 256` chunks.
    unsigned char received_data_buffer[256 * 0x178]; // `auStack_1781c`

    // Initialize `received_data_buffer` to zeros.
    memset(received_data_buffer, 0, sizeof(received_data_buffer));

    // Initialize `msg_data_ptr`'s structure to zeros.
    // `0x17808` is the size used in the original `memset` call.
    memset(msg_data_ptr, 0, 0x17808);

    // Loop condition: Continue if there are `remaining_chunks` to receive,
    // OR if it's the very first iteration (`received_chunks_count == 0`).
    while (remaining_chunks != 0 || received_chunks_count == 0) {
        msghdr_custom msg_header;
        // The `msg_iov_data_ptr` in our custom header points to the `msg_data_ptr`
        // which will receive the packet data (after the initial 8 bytes).
        msg_header.msg_iov_data_ptr = (void*)msg_data_ptr;

        // Call the custom `recvmsg` function.
        // Assuming socket FD is 0 and flags are 0x180.
        ssize_t recv_result = custom_recvmsg(0, &msg_header, 0x180);

        if (recv_result < 0) {
            // Handle error during receive.
            // Create a response packet using `msg_data_ptr` as the buffer.
            create_resp_pkt((unsigned int*)msg_data_ptr, "Error response");
            // Send the response packet.
            int send_result = senddgram(0, (unsigned int*)msg_data_ptr);

            if (send_result < 0) {
                _terminate(); // Unrecoverable error, terminate.
            }
            _terminate(); // Terminate after sending error response.
        }

        // Validate received packet data based on `msg_data_ptr` fields.
        if (msg_data_ptr->field4 != 4 && remaining_chunks == 0) {
            return 0; // Specific success condition for initial packet if `field4` is not 4
        }
        if (msg_data_ptr->field4 != 4) {
            return -1; // Error: `field4` mismatch
        }
        if (msg_data_ptr->field0 != 0x180) {
            return -2; // Error: `field0` mismatch
        }

        byte current_chunk_index = msg_data_ptr->field8;

        if (received_chunks_count != current_chunk_index) {
            return -3; // Error: Chunk index mismatch
        }

        if (current_chunk_index == 0) { // This is the first chunk of a multi-chunk message
            total_expected_chunks = msg_data_ptr->field9 + 1; // Calculate total chunks

            if (total_expected_chunks < 2) {
                return -4; // Error: Less than 2 chunks expected (invalid)
            }
            final_field4_value = msg_data_ptr->fieldC; // Update `final_field4_value`
            remaining_chunks = total_expected_chunks;   // Initialize remaining chunks
        }

        if (remaining_chunks == 0) {
            return -5; // Error: No chunks expected but loop continues (logic error)
        }
        if (total_expected_chunks != (ushort)(msg_data_ptr->field9 + 1)) {
            return -6; // Error: Total expected chunks mismatch
        }

        // Copy the data from the current packet (`msg_data_ptr->field10`)
        // into the appropriate slot in the `received_data_buffer`.
        memcpy(received_data_buffer + (uint)current_chunk_index * 0x178,
               msg_data_ptr->field10, 0x178);

        remaining_chunks--;   // Decrement count of chunks remaining
        received_chunks_count++; // Increment count of chunks received
    } // End of while loop

    // After all chunks are received:
    msg_data_ptr->field4 = final_field4_value; // Update `field4` with the accumulated value

    // Copy the entire `received_data_buffer` into `msg_data_ptr` starting at `field8`.
    memcpy(&(msg_data_ptr->field8), received_data_buffer, sizeof(received_data_buffer));

    // Calculate and update `field0` based on `total_expected_chunks`.
    msg_data_ptr->field0 = (uint)total_expected_chunks * 0x100 + 0x80;

    return 0; // Success
}