#include <sys/socket.h> // For recv, send, ssize_t
#include <sys/types.h>  // For ssize_t
#include <stddef.h>     // For size_t
#include <stdint.h>     // For uint32_t, int32_t, uint8_t, uint16_t, uintptr_t
#include <string.h>     // For memset, strcpy, strncpy, memcpy
#include <unistd.h>     // For ssize_t
#include <stdlib.h>     // For exit
#include <errno.h>      // For errno (though not explicitly used for specific error checks)

// --- Custom types and structures based on decompiled code ---

// Custom structure for custom_recvmsg and senddgram header.
// Assumes 32-bit architecture where int32_t and void* are 4 bytes, making it 8 bytes total.
typedef struct {
    int32_t msg_data_len; // Interpreted from 'msg_name' in original recvmsg wrapper.
                          // Also used as the length field for senddgram.
    void *msg_data_ptr;   // Interpreted from '&__message->msg_iov' in original recvmsg wrapper.
                          // In senddgram, this field is not used as a pointer for the second `sendall` call,
                          // but its value (the second 4 bytes of the header) is part of the 8-byte header sent.
} custom_msghdr;

// Constants for buffer sizes and magic values
#define PACKET_FRAGMENT_SIZE 0x180 // 384 bytes, used for max_data_len in custom_recvmsg and validation
#define FRAGMENT_COPY_SIZE   0x178 // 376 bytes, used for memcpy of individual fragments
#define MAX_FRAGMENTS        256   // Implied by total buffer size / fragment size (approx 96256 / 376)

// The overall packet buffer size as determined from memset(..., 0x17808)
#define ACTUAL_PACKET_BUFFER_SIZE 0x17808 // 96264 bytes

// Structure for the packet buffer, accessed via global_packet_buffer_ptr.
// This struct is designed to be compatible with custom_msghdr for its first 8 bytes.
// The layout is inferred from access patterns in recvdgram.
typedef struct {
    custom_msghdr header;        // First 8 bytes, compatible with custom_msghdr.
                                 // Original `*param_1` and `param_1[1]` access these.
    uint8_t fragment_idx;        // *(byte *)(current_packet_buffer + 8)
    uint8_t total_fragments_m1;  // *(byte *)(current_packet_buffer + 9) (total_fragments - 1)
    uint16_t _padding;           // Padding to align to 0xc or 0x10. Makes 8 bytes after header.
    uint32_t fieldC_data;        // *(undefined4 *)(current_packet_buffer + 0xc)
    char data_payload[ACTUAL_PACKET_BUFFER_SIZE - sizeof(custom_msghdr) - 0x8]; // Data starts at 0x10 (8+8)
} PacketBuffer;

// Placeholder for the global_packet_buffer_ptr.
// It's assumed to point to a statically allocated buffer for compilation.
static PacketBuffer *global_packet_buffer_ptr;

// --- Helper function declarations ---
static ssize_t sendall(int fd, const void *buf, size_t len, int flags);
static void _terminate(int code); // Placeholder for _terminate

// --- Fixed functions ---

// Function: custom_recvmsg (renamed from recvmsg to avoid conflict with standard library)
ssize_t custom_recvmsg(int fd, custom_msghdr *message, int max_data_len) {
    const int custom_recv_flags = 0x118d3;
    ssize_t bytes_received;

    // 1. Initial check: If max_data_len (original __flags) is less than the header size (8 bytes), return error.
    if ((unsigned int)max_data_len < sizeof(custom_msghdr)) {
        return -3; // Corresponds to 0xfffffffd in original
    }

    // 2. Receive the header (custom_msghdr) which contains the data length and a pointer.
    bytes_received = recv(fd, message, sizeof(custom_msghdr), custom_recv_flags);

    if (bytes_received != sizeof(custom_msghdr)) {
        return bytes_received < 0 ? bytes_received : -1; // Propagate system error or return generic error
    }

    // 3. Validate the data length read from the header.
    // Original: `((uint)__flags < __n)` means `max_data_len < message->msg_data_len` (data_len too large).
    if (message->msg_data_len < 0 || message->msg_data_len > max_data_len) {
        return -1; // Corresponds to 0xffffffff in original
    }

    // 4. Receive the actual data into the buffer pointed to by msg_data_ptr.
    bytes_received = recv(fd, message->msg_data_ptr, (size_t)message->msg_data_len, custom_recv_flags);

    if (bytes_received != message->msg_data_len) {
        return bytes_received < 0 ? bytes_received : -2; // Propagate system error or return specific error
    }

    return message->msg_data_len; // Success: return number of data bytes read.
}

// Function: senddgram
int senddgram(int fd, custom_msghdr *message) {
    const int custom_send_flags = 0x11972;
    int ret;

    // 1. Send the header part (8 bytes, i.e., sizeof(custom_msghdr)).
    ret = sendall(fd, message, sizeof(custom_msghdr), custom_send_flags);

    if (ret == (int)sizeof(custom_msghdr)) { // If header sent successfully
        // 2. Send the actual data. The data starts immediately after the 8-byte header.
        // The length of the data is given by message->msg_data_len.
        ret = sendall(fd, (char*)message + sizeof(custom_msghdr), (size_t)message->msg_data_len, custom_send_flags);
    }
    return ret;
}

// Function: create_resp_pkt
void create_resp_pkt(PacketBuffer *packet_buf, const char *message_str) {
    memset(packet_buf, 0, ACTUAL_PACKET_BUFFER_SIZE);
    
    // Original: *param_1 = 0x100; (where param_1 is `packet_buf`)
    packet_buf->header.msg_data_len = 0x100; 
    
    // Original: param_1[1] = 3; This sets the second 4-byte word of `packet_buf`.
    // In `custom_msghdr`, this is the `msg_data_ptr` field. It's used as a value here.
    ((uint32_t*)&packet_buf->header)[1] = 3; 
    
    // Original: strcpy((char *)(param_1 + 2),param_2);
    // This copies the message string to `(char*)packet_buf + 8`.
    // This location is immediately after the `custom_msghdr` part of `PacketBuffer`.
    strncpy((char*)packet_buf + sizeof(custom_msghdr), message_str, 
            ACTUAL_PACKET_BUFFER_SIZE - sizeof(custom_msghdr) - 1);
    ((char*)packet_buf + sizeof(custom_msghdr))[ACTUAL_PACKET_BUFFER_SIZE - sizeof(custom_msghdr) - 1] = '\0';
}

// Function: recvdgram
uint32_t recvdgram(void) {
    // Stack variables from original snippet, adjusted types and sizes
    char fragment_assembly_buffer[MAX_FRAGMENTS * FRAGMENT_COPY_SIZE]; // 96256 bytes (0x17800)
    
    uint32_t total_fragments_expected = 0; // local_10
    uint32_t current_fragment_idx = 0;   // local_12
    uint32_t fragments_remaining = 0;    // local_e
    uint32_t final_fieldC_val = 5;       // local_18, initialized to 5

    // Allocate global_packet_buffer_ptr if not already done.
    // For this example, use a static instance to ensure it's a valid memory location.
    static PacketBuffer s_packet_buffer_instance;
    global_packet_buffer_ptr = &s_packet_buffer_instance;

    // Initialize buffers
    memset(fragment_assembly_buffer, 0, sizeof(fragment_assembly_buffer));
    memset(global_packet_buffer_ptr, 0, ACTUAL_PACKET_BUFFER_SIZE);

    do {
        // Set up `msg_data_ptr` within the `global_packet_buffer_ptr->header`.
        // This pointer tells `custom_recvmsg` where to write the actual data payload.
        global_packet_buffer_ptr->header.msg_data_ptr = global_packet_buffer_ptr->data_payload;
        
        // Receive a fragment. The header will be written to `global_packet_buffer_ptr->header`,
        // and the data payload will be written to `global_packet_buffer_ptr->data_payload`.
        // The `max_data_len` for `custom_recvmsg` is `PACKET_FRAGMENT_SIZE` (0x180 = 384 bytes).
        // This is the expected maximum size of each data payload.
        ssize_t bytes_received = custom_recvmsg(0, &global_packet_buffer_ptr->header, PACKET_FRAGMENT_SIZE);

        if (bytes_received < 0) {
            // Error in receiving. Create and send a response packet, then terminate.
            create_resp_pkt(global_packet_buffer_ptr, "Recv Error");
            
            // Send the response packet.
            // `senddgram` expects `custom_msghdr *`.
            // `global_packet_buffer_ptr->header` holds the necessary fields for sending.
            int send_result = senddgram(1, &global_packet_buffer_ptr->header);

            if (send_result < 0) {
                _terminate(0x1a); // senddgram failed
            }
            _terminate(0x18); // Terminate regardless of senddgram success after recvmsg error
        }

        // Validate packet buffer fields after receiving
        // Original: `*(int *)(*(int *)(qss + iVar6 + 0xae623) + 4) != 4`
        // This maps to `global_packet_buffer_ptr->fieldC_data` in this structure.
        if (global_packet_buffer_ptr->fieldC_data != 4 && fragments_remaining == 0) {
            return 0; // Special early exit condition
        }
        if (global_packet_buffer_ptr->fieldC_data != 4) {
            return -1; // Corresponds to 0xffffffff
        }
        // Original: `**(int **)(qss + iVar6 + 0xae623) != 0x180`
        // This maps to `global_packet_buffer_ptr->header.msg_data_len`.
        if (global_packet_buffer_ptr->header.msg_data_len != PACKET_FRAGMENT_SIZE) {
            return -2; // Corresponds to 0xfffffffe
        }

        uint8_t received_fragment_idx = global_packet_buffer_ptr->fragment_idx;
        if (current_fragment_idx != received_fragment_idx) {
            return -3; // Corresponds to 0xfffffffd
        }

        if (received_fragment_idx == 0) {
            // First fragment received, initialize total_fragments and final_fieldC_val
            total_fragments_expected = global_packet_buffer_ptr->total_fragments_m1 + 1;
            if (total_fragments_expected < 2) {
                return -4; // Corresponds to 0xfffffffc
            }
            final_fieldC_val = global_packet_buffer_ptr->fieldC_data; // Original: local_18 = *(undefined4 *)(iVar4 + 0xc)
            fragments_remaining = total_fragments_expected;
        }

        if (fragments_remaining == 0) { // Should not happen after first fragment processing, but check
            return -5; // Corresponds to 0xfffffffb
        }

        // Verify total_fragments_expected against current packet's total_fragments_m1
        if (total_fragments_expected != (uint32_t)(global_packet_buffer_ptr->total_fragments_m1 + 1)) {
            return -6; // Corresponds to 0xfffffffa
        }

        // Copy fragment data from `data_payload` into the `fragment_assembly_buffer`.
        memcpy(fragment_assembly_buffer + (uint32_t)received_fragment_idx * FRAGMENT_COPY_SIZE,
               global_packet_buffer_ptr->data_payload, FRAGMENT_COPY_SIZE);

        fragments_remaining--;
        current_fragment_idx++;

    } while (fragments_remaining != 0); // Loop until all fragments received

    // After loop, all fragments assembled. Update global_packet_buffer_ptr with final values.
    global_packet_buffer_ptr->fieldC_data = final_fieldC_val; // Update field C
    
    // Copy assembled data into the data_payload section of the PacketBuffer
    memcpy(global_packet_buffer_ptr->data_payload, fragment_assembly_buffer, sizeof(fragment_assembly_buffer));
    
    // Update header.msg_data_len for the final assembled packet
    global_packet_buffer_ptr->header.msg_data_len = (total_fragments_expected * 0x100) + 0x80;

    return 0; // Success
}

// --- Helper function implementations ---

// A minimal implementation of sendall. In a real scenario, this would loop until all bytes are sent.
static ssize_t sendall(int fd, const void *buf, size_t len, int flags) {
    size_t total_sent = 0;
    ssize_t bytes_sent;
    while (total_sent < len) {
        bytes_sent = send(fd, (const char*)buf + total_sent, len - total_sent, flags);
        if (bytes_sent == -1) {
            // Handle error, e.g., check errno for specific error codes
            return -1; // Or propagate specific error
        }
        total_sent += bytes_sent;
    }
    return total_sent;
}

// Placeholder for _terminate
static void _terminate(int code) {
    exit(code); // Use stdlib's exit
}