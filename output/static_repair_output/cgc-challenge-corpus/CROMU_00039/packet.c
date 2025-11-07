#include <unistd.h>    // For write, ssize_t, STDOUT_FILENO
#include <string.h>    // For strlen, strcpy, memcpy, memset
#include <stdbool.h>   // For bool type
#include <stdio.h>     // For fprintf, stderr, printf (in dummy main)
#include <sys/types.h> // For size_t (often included by others, but good practice)

// Define custom types based on typical decompiled output
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned char undefined;
typedef unsigned int undefined4;

// --- Dummy Implementations for Linux Compilability ---

// A global file descriptor for outgoing packets, used by default if no fd is passed.
// In a real application, this would be a socket FD passed around.
static int g_output_fd = STDOUT_FILENO;

// Dummy receive function
// Mimics read(fd, buf, count) but also updates bytes_received.
ssize_t receive(int fd, void *buf, size_t count, int *bytes_received) {
    fprintf(stderr, "DUMMY: receive(fd=%d, buf=%p, count=%zu)\n", fd, buf, count);
    // Simulate reading some bytes from stdin for testing
    // In a real system, this would block or read from a network socket.
    ssize_t actual_read = read(fd, buf, count);
    if (actual_read > 0 && bytes_received != NULL) {
        *bytes_received = (int)actual_read;
    }
    return actual_read;
}

// Dummy SearchVARS function
int SearchVARS(char *name, char *src_unused, size_t n, undefined4 arg4_unused) {
    fprintf(stderr, "DUMMY: SearchVARS(name='%.*s', n=%zu)\n", (int)n, name, n);
    // Simulate finding a variable and returning a pointer/ID (non-zero for success)
    return (n > 0 && name[0] != '\0') ? 0x1000 : 0; // Dummy non-zero value if name is not empty
}

// Dummy UpdateVARSObject function
int UpdateVARSObject(undefined *name_buf, undefined type_byte, undefined *value_buf, undefined4 arg4_unused) {
    fprintf(stderr, "DUMMY: UpdateVARSObject(name='%.*s', type=0x%x, value='%.*s')\n",
            127, name_buf, type_byte, 127, value_buf);
    // Simulate successful update
    return 0x2000; // Dummy non-zero value
}

// --- Original Functions, Fixed and Optimized ---

uint ReceivePacket(char *param_1) {
    int bytes_read;
    ssize_t rv;
    uint total_read = 0;

    // First loop: read 2 bytes
    for (total_read = 0; total_read != 2; total_read += bytes_read) {
        rv = receive(0, param_1 + total_read, 2 - total_read, &bytes_read);
        if (rv < 0 || bytes_read == 0) {
            return 0;
        }
    }

    if (*param_1 == '\0') {
        // Original logic: `(byte)param_1[1] < 4` or `param_1[1] == -1`
        // Ensure signed char -1 check is performed before unsigned comparison.
        if ((char)param_1[1] == -1 || (byte)param_1[1] < 4) {
            return 0;
        } else {
            byte expected_len = param_1[1] + 2;
            total_read = 0; // Reset total_read for the second read operation
            for (; total_read != expected_len; total_read += bytes_read) {
                rv = receive(0, param_1 + total_read, expected_len - total_read, &bytes_read);
                if (rv < 0 || bytes_read == 0) {
                    return 0;
                }
            }
        }
    } else {
        return 0;
    }
    return total_read;
}

bool SendError(int fd, undefined param_1, undefined param_2) {
    unsigned char packet[5] = {0, 3, 4, param_1, param_2};
    ssize_t bytes_written = write(fd, packet, sizeof(packet));
    return bytes_written == sizeof(packet);
}

uint SendResponse(int fd, undefined param_1, int param_2) {
    // Packet structure:
    // Header: byte 0 (0), byte 1 (3), byte 2 (param_1)
    // Followed by payload data.
    // The total length for comparison (`expected_write_len`) is derived from original `local_10b + 2`.

    struct __attribute__((packed)) {
        undefined header0;
        undefined header1;
        undefined param1_val;
        char payload[256];
    } packet_buffer;

    packet_buffer.header0 = 0;
    packet_buffer.header1 = 3;
    packet_buffer.param1_val = param_1;

    size_t actual_payload_len;   // Actual length of data copied into packet_buffer.payload
    size_t expected_write_len; // The total length expected to be written, used for comparison

    if (*(char *)(param_2 + 0x80) == '\x01') { // Type 1: String
        size_t str_len = strlen((char *)(param_2 + 0x81));
        if (0xfc < str_len) { // Check for buffer overflow (252 bytes max for string + null)
            return SendError(fd, param_1, 1);
        }
        strcpy(packet_buffer.payload, (char *)(param_2 + 0x81));
        actual_payload_len = str_len + 1; // Include null terminator as per original length logic
        expected_write_len = str_len + 4;   // 3 (header) + (str_len + 1) (payload)
    } else if (*(char *)(param_2 + 0x80) == '\x02') { // Type 2: Raw 4 bytes
        memcpy(packet_buffer.payload, (void *)(param_2 + 0x81), 4);
        actual_payload_len = 4;
        // Original logic: `local_10b` was 6. Return check `sVar3 == local_10b + 2` means `sVar3 == 8`.
        // This implies an expected total write of 8 bytes (3 header + 5 payload).
        // However, `memcpy` copies only 4 payload bytes.
        // This discrepancy is preserved: write 7 bytes, but compare against 8.
        expected_write_len = 8;
    } else {
        return SendError(fd, param_1, 4);
    }

    size_t total_bytes_to_write = 3 + actual_payload_len; // 3 header bytes + actual payload length

    ssize_t bytes_written = write(fd, &packet_buffer, total_bytes_to_write);
    return (uint)(bytes_written == expected_write_len);
}

undefined4 HandleReadRequest(int fd, int param_1, uint param_2) {
    if (param_1 == 0) {
        return 0;
    }

    undefined packet_id = *(undefined *)(param_1 + 3);
    char search_buffer[128];
    memset(search_buffer, 0, sizeof(search_buffer));

    byte data_len = *(byte *)(param_1 + 4);

    // (param_2 & 0xff) - 5 is the expected total length of payload bytes in the packet
    // This assumes 5 bytes for packet header before the data starts:
    // [byte 0][byte 1][byte 2][byte 3 (packet_id)][byte 4 (data_len)][data...]
    if ((uint)data_len == (param_2 & 0xff) - 5) {
        strncpy(search_buffer, (char *)(param_1 + 5), data_len);
        if (data_len < sizeof(search_buffer)) {
            search_buffer[data_len] = '\0'; // Ensure null termination
        } else {
            search_buffer[sizeof(search_buffer) - 1] = '\0';
        }

        int search_result = SearchVARS(search_buffer, (char *)(param_1 + 5), data_len, 0x112e2);
        if (search_result == 0) {
            SendError(fd, packet_id, 4);
            return 0;
        } else {
            SendResponse(fd, packet_id, search_result);
            return 1;
        }
    } else {
        SendError(fd, packet_id, 1);
        return 0;
    }
}

undefined4 HandleWriteRequest(int fd, int param_1, uint param_2) {
    if (param_1 == 0) {
        return 0;
    }

    undefined packet_id = *(undefined *)(param_1 + 3);

    undefined name_buffer[128];
    memset(name_buffer, 0, sizeof(name_buffer));

    byte name_len = *(byte *)(param_1 + 4);

    // The condition `(uint)name_len < (param_2 & 0xff) - 7` checks if `name_len + 7 < param_2`.
    // This implies that the total packet length `param_2` must be at least `name_len + 7`.
    // The `7` accounts for: [byte 0][byte 1][byte 2][packet_id][name_len][type_byte][value_len]
    // i.e., 3 fixed header bytes + packet_id + name_len field + type_byte field + value_len field.
    if ((uint)name_len < (param_2 & 0xff) - 7) {
        if ((char)name_len < 0) { // Check for negative length (though byte is unsigned, original code had this check)
            SendError(fd, packet_id, 1);
            return 0;
        } else {
            memcpy(name_buffer, (void *)(param_1 + 5), name_len);
            if (name_len < sizeof(name_buffer)) {
                name_buffer[name_len] = '\0';
            } else {
                name_buffer[sizeof(name_buffer) - 1] = '\0';
            }

            undefined type_byte = *(undefined *)(param_1 + name_len + 5);

            undefined value_buffer[128];
            memset(value_buffer, 0, sizeof(value_buffer));

            byte value_len = *(byte *)(param_1 + name_len + 6);

            // Calculate offset to value data: 3 (fixed header) + 1 (packet_id) + 1 (name_len field)
            // + name_len (actual name data) + 1 (type_byte field) + 1 (value_len field) = 7 + name_len.
            // `remaining_packet_len_for_value = param_2 - (7 + name_len)`.
            // Original check: `remaining_packet_len_for_value < value_len` (this is an error condition)
            if ((param_2 & 0xff) - (7 + name_len) < value_len) {
                SendError(fd, packet_id, 1);
                return 0;
            } else if ((char)value_len < 0) { // Check for negative length
                SendError(fd, packet_id, 1);
                return 0;
            } else {
                memcpy(value_buffer, (void *)(param_1 + name_len + 7), value_len);
                if (value_len < sizeof(value_buffer)) {
                    value_buffer[value_len] = '\0';
                } else {
                    value_buffer[sizeof(value_buffer) - 1] = '\0';
                }

                int update_result = UpdateVARSObject(name_buffer, type_byte, value_buffer, 0x113d5);
                if (update_result == 0) {
                    SendError(fd, packet_id, 2);
                } else {
                    SendResponse(fd, packet_id, update_result);
                }
                return 1;
            }
        }
    } else {
        SendError(fd, packet_id, 1);
        return 0;
    }
}

undefined4 HandlePacket(int fd, int param_1) {
    if (param_1 == 0) {
        return 0;
    }

    byte packet_len_indicator = *(byte *)(param_1 + 1);
    uint total_packet_len = packet_len_indicator + 2; // Original `bVar1` was `*(char*)(param_1+1) + 2`

    if (total_packet_len < 4) { // Minimum packet size check
        return 0;
    } else {
        if (*(char *)(param_1 + 2) == '\x01') { // Read Request
            HandleReadRequest(fd, param_1, total_packet_len);
        } else if (*(char *)(param_1 + 2) == '\x02') { // Write Request
            HandleWriteRequest(fd, param_1, total_packet_len);
        } else { // Unknown Request Type
            SendError(fd, *(undefined *)(param_1 + 3), 3);
        }
        return 1;
    }
}

// Dummy main function to demonstrate compilability and basic usage
int main() {
    char test_packet_buffer[512]; // A buffer to simulate received packets

    printf("--- Test ReceivePacket (failure) ---\n");
    memset(test_packet_buffer, 0, sizeof(test_packet_buffer));
    uint received = ReceivePacket(test_packet_buffer);
    printf("ReceivePacket returned: %u (expected 0 if no input)\n", received);

    printf("\n--- Test HandlePacket (Read Request) ---\n");
    memset(test_packet_buffer, 0, sizeof(test_packet_buffer));
    // Simulate a packet: [0][len_indicator=7][type=1][id=0xAA][name_len=4][name="test\0"]
    test_packet_buffer[0] = 0;
    test_packet_buffer[1] = 7; // Packet length indicator for total_packet_len = 7 + 2 = 9
    test_packet_buffer[2] = 0x01; // Read request type
    test_packet_buffer[3] = 0xAA; // Packet ID
    test_packet_buffer[4] = 4; // Length of name "test"
    strcpy(test_packet_buffer + 5, "test"); // Name "test" (occupies bytes 5,6,7,8)
    // Total bytes: 0(byte0) + 1(len_indicator) + 1(type) + 1(id) + 1(name_len) + 4(name) = 9 bytes.
    // In HandleReadRequest, param_2 (total_packet_len) is 9.
    // data_len is 4. `(param_2 & 0xff) - 5` is `9 - 5 = 4`. Condition `4 == 4` is true.

    undefined4 handle_result = HandlePacket(g_output_fd, (int)test_packet_buffer);
    printf("HandlePacket (Read Request) returned: %u (expected 1)\n", handle_result);

    printf("\n--- Test HandlePacket (Write Request) ---\n");
    memset(test_packet_buffer, 0, sizeof(test_packet_buffer));
    // Packet: [0][len_indicator=12][type=2][id=0xBB][name_len=3][name="key\0"][type_val=0][value_len=4][value="data\0"]
    test_packet_buffer[0] = 0;
    test_packet_buffer[1] = 12; // total_packet_len = 12 + 2 = 14
    test_packet_buffer[2] = 0x02; // Write request type
    test_packet_buffer[3] = 0xBB; // Packet ID
    test_packet_buffer[4] = 3; // name_len for "key"
    strcpy(test_packet_buffer + 5, "key"); // Name "key" (bytes 5,6,7)
    test_packet_buffer[5+3] = 0x00; // Type byte for value (e.g., string type) (byte 8)
    test_packet_buffer[5+3+1] = 4; // value_len for "data" (byte 9)
    strcpy(test_packet_buffer + 5+3+1+1, "data"); // Value "data" (bytes 10,11,12,13)
    // Total bytes: 0(byte0) + 1(len_indicator) + 1(type) + 1(id) + 1(name_len) + 3(name) + 1(type_val) + 1(value_len) + 4(value) = 14 bytes.
    // In HandleWriteRequest, param_2 (total_packet_len) is 14.
    // name_len is 3. Condition `(uint)name_len < (param_2 & 0xff) - 7` -> `3 < (14 - 7)` -> `3 < 7` is true.
    // value_len is 4. Condition `(param_2 & 0xff) - (7 + name_len) < value_len` -> `14 - (7 + 3) < 4` -> `14 - 10 < 4` -> `4 < 4` is false.

    handle_result = HandlePacket(g_output_fd, (int)test_packet_buffer);
    printf("HandlePacket (Write Request) returned: %u (expected 1)\n", handle_result);

    printf("\n--- Test HandlePacket (Unknown Request) ---\n");
    memset(test_packet_buffer, 0, sizeof(test_packet_buffer));
    test_packet_buffer[0] = 0;
    test_packet_buffer[1] = 2; // total_packet_len = 2 + 2 = 4
    test_packet_buffer[2] = 0x03; // Unknown request type
    test_packet_buffer[3] = 0xCC; // Packet ID

    handle_result = HandlePacket(g_output_fd, (int)test_packet_buffer);
    printf("HandlePacket (Unknown Request) returned: %u (expected 1)\n", handle_result);

    return 0;
}