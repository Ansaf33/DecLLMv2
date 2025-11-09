#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

// Type definitions for clarity, matching original intent
typedef unsigned int uint;
typedef unsigned char byte;

// --- Mock/External Function Declarations ---

// Mock for receive.
// In a real system, this would be `read` from a socket or similar,
// and `bytes_received` would be the actual count.
ssize_t receive(int fd, void *buf, size_t nbytes, int *bytes_received_out) {
    if (fd == 0) { // Assuming fd 0 is stdin or a mock input channel
        // For testing, simulate receiving specific data for ReceivePacket.
        // ReceivePacket first reads 2 bytes, then more.
        // To make ReceivePacket proceed past `*param_1 == '\0'` and `param_1[1]` checks:
        // Set `buf[0] = 0` and `buf[1]` to a value that makes `expected_len` plausible.
        if (nbytes >= 2) {
            ((char*)buf)[0] = 0; // Ensures `*param_1 == '\0'` is true
            ((char*)buf)[1] = 10; // `expected_len` will be 10 + 2 = 12
        }
        // Fill the rest with dummy data
        if (nbytes > 2) {
            memset((char*)buf + 2, 'A', nbytes - 2);
        }
        *bytes_received_out = nbytes; // Simulate full read
        return nbytes;
    }
    *bytes_received_out = 0;
    return -1; // Error or invalid fd
}

// Dummy buffer to simulate the structure expected by SendResponse
// This buffer is used by mock SearchVARS and UpdateVARSObject to return data.
// It must be large enough to accommodate the 0x80 offset + type + data.
static char mock_response_buffer[512];

// SearchVARS: Returns a pointer to a response data structure (as int) or 0 (NULL)
// `name_buffer`: the name to search for (null-terminated string)
// `original_src_ptr`: pointer to the original packet data (for context, potentially redundant)
// `name_len`: length of the name
// `magic_val`: a magic constant (0x112e2)
int SearchVARS(char *name_buffer, char *original_src_ptr, unsigned int name_len, int magic_val) {
    (void)original_src_ptr; // Unused in this mock
    (void)name_len;         // Unused in this mock, assuming name_buffer is null-terminated
    (void)magic_val;        // Unused in this mock

    printf("SearchVARS called for: %s\n", name_buffer);

    // For mock, always "find" something and put it in mock_response_buffer
    memset(mock_response_buffer, 0, sizeof(mock_response_buffer));
    mock_response_buffer[0x80] = 0x01; // Simulate type 1 (string)
    snprintf(mock_response_buffer + 0x81, sizeof(mock_response_buffer) - 0x81, "Value_for_%s", name_buffer);
    return (int)(long)mock_response_buffer; // Return pointer to this buffer
}

// UpdateVARSObject: Returns a pointer to a response data structure (as int) or 0 (NULL)
// `name`: name of the variable to update/create
// `type`: type of the variable's value
// `value`: new value for the variable
// `magic_val`: a magic constant (0x113d5)
int UpdateVARSObject(char *name, char type, char *value, int magic_val) {
    (void)magic_val; // Unused in this mock

    printf("UpdateVARSObject called for: %s, type: %c, value: %s\n", name, type, value);

    // For mock, always "update" successfully and put it in mock_response_buffer
    memset(mock_response_buffer, 0, sizeof(mock_response_buffer));
    mock_response_buffer[0x80] = 0x01; // Simulate type 1 (string)
    snprintf(mock_response_buffer + 0x81, sizeof(mock_response_buffer) - 0x81, "Updated_%s_to_%s", name, value);
    return (int)(long)mock_response_buffer; // Return pointer to this buffer
}

// --- Original Functions (Refactored) ---

unsigned int ReceivePacket(char *param_1) {
    ssize_t bytes_read;
    int actual_bytes_received = 0;
    unsigned int total_received = 0;

    for (total_received = 0; total_received < 2; total_received += actual_bytes_received) {
        bytes_read = receive(0, param_1 + total_received, 2 - total_received, &actual_bytes_received);
        if (bytes_read < 0 || actual_bytes_received == 0) {
            return 0;
        }
    }

    if (*param_1 == '\0') {
        // param_1[1] is a signed char, comparing with -1 is specific.
        // (unsigned char)param_1[1] < 4 is comparing unsigned char.
        // The original code implies these are error conditions.
        if (((unsigned char)param_1[1] < 4) || (param_1[1] == (char)-1)) {
            return 0;
        } else {
            unsigned char expected_len = (unsigned char)param_1[1] + 2;
            for (; total_received < expected_len; total_received += actual_bytes_received) {
                bytes_read = receive(0, param_1 + total_received, expected_len - total_received, &actual_bytes_received);
                if (bytes_read < 0 || actual_bytes_received == 0) {
                    return 0;
                }
            }
        }
    } else {
        return 0;
    }
    return total_received;
}

_Bool SendError(char session_id, char error_code) {
    char packet_buf[5];
    packet_buf[0] = 0;
    packet_buf[1] = 3; // Seems to be a fixed length for error packets
    packet_buf[2] = 4; // Command type for error
    packet_buf[3] = session_id;
    packet_buf[4] = error_code;

    ssize_t bytes_written = write(0, packet_buf, sizeof(packet_buf)); // Assuming FD 0 for output
    printf("SendError: Session %d, Error %d. Bytes written: %zd\n", session_id, error_code, bytes_written);
    return bytes_written == sizeof(packet_buf);
}

unsigned int SendResponse(char session_id, const char *response_data_ptr) {
    char full_packet[4 + 256]; // Max size: 1 (cmd) + 1 (len) + 1 (type) + 1 (session_id) + 256 (data)
    size_t content_len = 0;

    full_packet[0] = 0; // Fixed first byte
    full_packet[2] = 3; // Command/type byte for response
    full_packet[3] = session_id; // Session ID

    // `response_data_ptr` is assumed to point to a buffer that contains the type indicator at offset 0x80
    // and the actual content at offset 0x81, as implied by the original disassembler code.
    char type_indicator = response_data_ptr[0x80];
    const char *actual_response_content = response_data_ptr + 0x81;

    if (type_indicator == '\x01') { // Type 1: string
        content_len = strlen(actual_response_content);
        if (content_len > 0xfc) { // Max string length (252 bytes)
            return SendError(session_id, 1);
        }
        memcpy(full_packet + 4, actual_response_content, content_len);
    } else if (type_indicator == '\x02') { // Type 2: fixed 4 bytes
        content_len = 4;
        memcpy(full_packet + 4, actual_response_content, content_len);
    } else {
        return SendError(session_id, 4);
    }

    full_packet[1] = (unsigned char)(content_len + 2); // Length byte: actual data length + 2 (for type and session_id bytes)

    ssize_t bytes_written = write(0, full_packet, 4 + content_len); // Assuming FD 0 for output
    printf("SendResponse: Session %d, Content '%s', Total Bytes written: %zd\n",
           session_id, actual_response_content, bytes_written);
    return (unsigned int)(bytes_written == (ssize_t)(4 + content_len));
}

int HandleReadRequest(const char *packet, unsigned int total_packet_len) {
    if (packet == NULL) {
        return 0;
    }

    char session_id = packet[3];
    char var_name_buffer[128];
    memset(var_name_buffer, 0, sizeof(var_name_buffer));

    unsigned char var_name_len = packet[4];

    // Check if `var_name_len` is consistent with `total_packet_len`
    // `total_packet_len - 5` is the expected length of the variable name if packet is well-formed.
    if (var_name_len != (total_packet_len - 5)) {
        SendError(session_id, 1);
        return 0;
    }
    // Additional check for negative value after implicit cast, or extremely large unsigned value.
    if ((char)var_name_len < 0 || var_name_len >= sizeof(var_name_buffer)) {
        SendError(session_id, 1); // Length too large or invalid
        return 0;
    }

    memcpy(var_name_buffer, packet + 5, var_name_len);
    var_name_buffer[var_name_len] = '\0'; // Ensure null termination

    int var_result_ptr_as_int = SearchVARS(var_name_buffer, (char *)(packet + 5), var_name_len, 0x112e2);
    if (var_result_ptr_as_int == 0) {
        SendError(session_id, 4);
        return 0;
    } else {
        SendResponse(session_id, (const char *)(long)var_result_ptr_as_int);
        return 1;
    }
}

int HandleWriteRequest(const char *packet, unsigned int total_packet_len) {
    if (packet == NULL) {
        return 0;
    }

    char session_id = packet[3];

    char var_name_buffer[128];
    char var_value_buffer[128];
    memset(var_name_buffer, 0, sizeof(var_name_buffer));
    memset(var_value_buffer, 0, sizeof(var_value_buffer));

    unsigned char var_name_len = packet[4];

    // Calculate offsets and remaining lengths
    unsigned int name_start_offset = 5; // offset of var_name
    unsigned int type_start_offset = name_start_offset + var_name_len; // offset of var_type
    unsigned int value_len_start_offset = type_start_offset + 1; // offset of var_value_len
    unsigned int value_start_offset = value_len_start_offset + 1; // offset of var_value

    // Check if var_name_len is valid and fits within the packet
    // Minimum expected packet length up to value_len_start_offset should be `value_len_start_offset`
    if (var_name_len == 0 || (value_len_start_offset >= total_packet_len)) { // `var_name_len` must be at least 1, otherwise `type_start_offset` will be 5, which is `packet[5]`.
        SendError(session_id, 1);
        return 0;
    }
    if ((char)var_name_len < 0 || var_name_len >= sizeof(var_name_buffer)) { // Check for overflow or too large
        SendError(session_id, 1);
        return 0;
    }

    memcpy(var_name_buffer, packet + name_start_offset, var_name_len);
    var_name_buffer[var_name_len] = '\0'; // Null-terminate

    char var_type_byte = packet[type_start_offset];
    unsigned char var_value_len = packet[value_len_start_offset];

    // Check if var_value_len is valid and fits within the packet
    unsigned int expected_total_len = value_start_offset + var_value_len;
    if (expected_total_len > total_packet_len) { // value goes beyond packet boundary
        SendError(session_id, 1);
        return 0;
    }
    if ((char)var_value_len < 0 || var_value_len >= sizeof(var_value_buffer)) { // Check for overflow or too large
        SendError(session_id, 1);
        return 0;
    }

    memcpy(var_value_buffer, packet + value_start_offset, var_value_len);
    var_value_buffer[var_value_len] = '\0'; // Null-terminate

    int update_result_ptr_as_int = UpdateVARSObject(var_name_buffer, var_type_byte, var_value_buffer, 0x113d5);
    if (update_result_ptr_as_int == 0) {
        SendError(session_id, 2);
        return 0;
    } else {
        SendResponse(session_id, (const char *)(long)update_result_ptr_as_int);
        return 1;
    }
}

int HandlePacket(const char *packet) {
    if (packet == NULL) {
        return 0;
    }

    // Packet structure:
    // [0] some_fixed_byte (ignored by HandlePacket)
    // [1] length_of_payload_plus_two_bytes (this is `total_packet_len - 2`)
    // [2] packet_type (0x01 for read, 0x02 for write)
    // [3] session_id

    unsigned char total_packet_len = (unsigned char)packet[1] + 2;

    if (total_packet_len < 4) { // Minimum packet length (byte0, len_byte, type_byte, session_id)
        return 0;
    }

    char packet_type = packet[2];

    if (packet_type == '\x01') {
        HandleReadRequest(packet, total_packet_len);
    } else if (packet_type == '\x02') {
        HandleWriteRequest(packet, total_packet_len);
    } else {
        SendError(packet[3], 3); // Unknown packet type error, packet[3] is session_id
    }
    return 1; // Return 1 for handled, regardless of sub-function's success/failure
}

// Minimal main function
int main() {
    printf("--- Starting mock packet processing ---\n");

    // Example Read Request:
    // Packet format: [0] [total_len-2] [type] [session_id] [name_len] [name...]
    // Name: "test_var", len = 8.
    // `name_len = 8`. `total_packet_len - 5 = 8`. So `total_packet_len = 13`.
    // `packet[1]` = `total_packet_len - 2 = 11`.
    // Packet content: 0x00, 0x0B, 0x01, 0x64, 0x08, 't', 'e', 's', 't', '_', 'v', 'a', 'r'
    char read_request_packet[] = {
        0x00, // Ignored by HandlePacket
        11,   // total_len - 2 = 13 - 2 = 11
        0x01, // Type: Read Request
        100,  // Session ID
        8,    // Name length
        't', 'e', 's', 't', '_', 'v', 'a', 'r' // Name (8 bytes)
    };
    printf("\n--- Sending mock Read Request ---\n");
    int handle_result_read = HandlePacket(read_request_packet);
    printf("HandlePacket result for Read Request: %d\n", handle_result_read);

    // Example Write Request:
    // Packet format: [0] [total_len-2] [type] [session_id] [name_len] [name...] [value_type] [value_len] [value...]
    // Name: "new_var", len=7. Value type: 'S'. Value: "hello", len=5.
    // `name_len = 7`. `value_len = 5`.
    // `value_start_offset = 5 (header) + 7 (name_len) + 1 (type) + 1 (value_len) = 14`.
    // `expected_total_len = value_start_offset + value_len = 14 + 5 = 19`.
    // `total_packet_len = 19`.
    // `packet[1]` = `total_packet_len - 2 = 17`.
    // Packet content: 0x00, 0x11, 0x02, 0x65, 0x07, 'n', 'e', 'w', '_', 'v', 'a', 'r', 'S', 0x05, 'h', 'e', 'l', 'l', 'o'
    char write_request_packet[] = {
        0x00, // Ignored
        17,   // total_len - 2 = 19 - 2 = 17
        0x02, // Type: Write Request
        101,  // Session ID
        7,    // Name length
        'n', 'e', 'w', '_', 'v', 'a', 'r', // Name (7 bytes)
        'S',  // Value type (1 byte)
        5,    // Value length (1 byte)
        'h', 'e', 'l', 'l', 'o' // Value (5 bytes)
    };
    printf("\n--- Sending mock Write Request ---\n");
    int handle_result_write = HandlePacket(write_request_packet);
    printf("HandlePacket result for Write Request: %d\n", handle_result_write);

    // Test ReceivePacket separately
    char packet_recv_buf[128];
    memset(packet_recv_buf, 0, sizeof(packet_recv_buf));
    printf("\n--- Testing ReceivePacket ---\n");
    // The mock `receive` is configured to make `ReceivePacket` succeed with `total_received = 12`.
    unsigned int received_len = ReceivePacket(packet_recv_buf);
    printf("ReceivePacket result: %u\n", received_len);
    if (received_len > 0) {
        printf("Received packet content (first few bytes): 0x%02x 0x%02x 0x%02x 0x%02x ...\n",
               (unsigned char)packet_recv_buf[0], (unsigned char)packet_recv_buf[1],
               (unsigned char)packet_recv_buf[2], (unsigned char)packet_recv_buf[3]);
    }

    printf("\n--- Mock packet processing finished ---\n");

    return 0;
}