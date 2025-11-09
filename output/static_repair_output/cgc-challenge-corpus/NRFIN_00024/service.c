#include <stdio.h>    // For fprintf, NULL, stderr
#include <stdlib.h>   // For calloc, free, exit
#include <string.h>   // For memcpy, memcmp
#include <stdint.h>   // For uint8_t, uint16_t, uint32_t
#include <unistd.h>   // For ssize_t, close (and socket functions if fully mocked)
#include <sys/socket.h> // For recv, send (if fully mocked)
#include <stdbool.h>  // For bool type

// --- Type Definitions (based on decompiled structure) ---

// MessageHeader struct (received from network)
// Size: 0x1C (28 bytes)
typedef struct MessageHeader {
    uint8_t padding_0_F[16]; // Bytes 0x00-0x0F. The first 8 bytes of this array
                             // (padding_0_F[0] to padding_0_F[7]) are the "magic bytes".
    uint16_t session_id;     // Offset 0x10
    uint16_t msg_idx;        // Offset 0x12 (index of this message within its session)
    uint16_t msg_count;      // Offset 0x14 (total number of messages expected for this session)
    uint16_t msg_size;       // Offset 0x16 (size of the actual message content)
    void* msg_content_ptr;   // Offset 0x18 (pointer to the allocated buffer for message content)
} MessageHeader;

// Session struct (stored in a linked list by the program)
// Size: 0x14 (20 bytes)
typedef struct Session {
    struct Session* next;       // Offset 0x00
    uint16_t id;                // Offset 0x04 (session ID)
    uint16_t max_msgs;          // Offset 0x06 (total messages expected for this session)
    uint8_t* msg_avail_map;     // Offset 0x08 (ptr to a byte array, size max_msgs, tracks received messages)
    uint32_t total_data_size;   // Offset 0x0C (cumulative size of all msg_content_ptr data for this session)
    void* msg_list_head;        // Offset 0x10 (head of linked list of MessageNodes)
} Session;

// MessageNode struct (linked list of messages within a session)
// Size: 0x08 (8 bytes)
typedef struct MessageNode {
    struct MessageNode* next;   // Offset 0x00
    MessageHeader* msg_header;  // Offset 0x04
} MessageNode;

// --- Global Variables ---
Session* head = NULL;
int outstanding = 0; // Tracks the number of outstanding messages/sessions.
int g_sockfd = -1;   // Global socket file descriptor for mock I/O.

// --- Mock/Helper Functions ---

// Mock function for _terminate
void _terminate(void) {
    fprintf(stderr, "Program terminated.\n");
    exit(1);
}

// Mock function for sendline
// In the original code, this often precedes a _terminate call, implying an error.
int sendline(void) {
    fprintf(stderr, "sendline called (mock error message).\n");
    return -1; // Indicate failure to trigger _terminate in calling code
}

// Mock function for sendall
int sendall(void* buffer, size_t len) {
    fprintf(stderr, "sendall: Sending %zu bytes from %p (mock).\n", len, buffer);
    if (g_sockfd == -1) {
        fprintf(stderr, "sendall: No socket available.\n");
        return -1;
    }
    // In a real scenario, this would be:
    // ssize_t sent_bytes = send(g_sockfd, buffer, len, 0);
    // if (sent_bytes < 0) { perror("sendall error"); return -1; }
    return (int)len; // Simulate success
}

// --- Original Functions (fixed and refactored) ---

// Function: find_session
Session* find_session(uint16_t session_id) {
    Session* current_session = head;
    while (current_session != NULL) {
        if (session_id == current_session->id) {
            return current_session;
        }
        current_session = current_session->next;
    }
    return NULL;
}

// Function: new_session
Session* new_session(MessageHeader* msg_header) {
    Session* new_sess = calloc(1, sizeof(Session)); // Allocate 0x14 bytes for Session
    if (new_sess == NULL) {
        sendline();
        _terminate();
    }

    // Allocate memory for tracking message availability within the session
    new_sess->msg_avail_map = calloc(msg_header->msg_count, sizeof(uint8_t));
    if (new_sess->msg_avail_map == NULL) {
        free(new_sess); // Free the session struct itself
        sendline();
        _terminate();
    }

    new_sess->id = msg_header->session_id;
    new_sess->max_msgs = msg_header->msg_count;
    // Other fields (next, total_data_size, msg_list_head) are zeroed by calloc.
    return new_sess;
}

// Function: add_session
Session* add_session(MessageHeader* msg_header) {
    Session* new_sess = new_session(msg_header);

    if (head == NULL) {
        head = new_sess;
    } else {
        Session* current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_sess;
    }
    return new_sess;
}

// Function: delete_session
void delete_session(uint16_t session_id) {
    Session* current_session = head;
    Session* prev_session = NULL;

    while (current_session != NULL) {
        if (session_id == current_session->id) {
            break; // Found the session to delete
        }
        prev_session = current_session;
        current_session = current_session->next;
    }

    if (current_session == NULL) {
        return; // Session not found
    }

    // Unlink the session from the list
    if (prev_session == NULL) { // Deleting the head
        head = current_session->next;
    } else {
        prev_session->next = current_session->next;
    }

    // Free all messages associated with this session
    MessageNode* current_msg_node = (MessageNode*)current_session->msg_list_head;
    while (current_msg_node != NULL) {
        MessageNode* next_msg_node = current_msg_node->next;
        if (current_msg_node->msg_header) {
            free(current_msg_node->msg_header->msg_content_ptr);
            free(current_msg_node->msg_header);
        }
        free(current_msg_node);
        current_msg_node = next_msg_node;
    }

    // Free session's message availability map and the session struct itself
    free(current_session->msg_avail_map);
    free(current_session);

    outstanding--;
}

// Function: add_msg
void* add_msg(MessageHeader* msg_header, Session* session) {
    void* combined_data_buffer = NULL;

    // Check if message index is out of bounds or slot is already taken
    if (msg_header->msg_idx >= session->max_msgs || session->msg_avail_map[msg_header->msg_idx] != 0) {
        free(msg_header->msg_content_ptr);
        free(msg_header);
        return NULL;
    }

    MessageNode* new_msg_node = calloc(1, sizeof(MessageNode)); // Allocate 8 bytes for MessageNode
    if (new_msg_node == NULL) {
        sendline();
        _terminate();
    }
    new_msg_node->msg_header = msg_header;

    // Add message node to the session's message list
    if (session->msg_list_head == NULL) {
        session->msg_list_head = new_msg_node;
    } else {
        MessageNode* current_node = (MessageNode*)session->msg_list_head;
        while (current_node->next != NULL) {
            current_node = current_node->next;
        }
        current_node->next = new_msg_node;
    }

    session->msg_avail_map[msg_header->msg_idx] = 1; // Mark message slot as taken
    session->total_data_size += msg_header->msg_size; // Accumulate total data size

    // Check if all messages for this session have been received
    bool all_msgs_received = true;
    for (uint16_t i = 0; i < session->max_msgs; ++i) {
        if (session->msg_avail_map[i] == 0) {
            all_msgs_received = false;
            break;
        }
    }

    if (all_msgs_received) {
        combined_data_buffer = calloc(session->total_data_size, sizeof(uint8_t));
        if (combined_data_buffer == NULL) {
            sendline();
            _terminate();
        }

        uint32_t current_offset = 0;
        // Iterate through expected message indices (0 to max_msgs-1)
        for (uint16_t i = 0; i < session->max_msgs; ++i) {
            // Find the message node for the current index
            MessageNode* current_node = (MessageNode*)session->msg_list_head;
            while (current_node != NULL) {
                if (current_node->msg_header->msg_idx == i) {
                    // Copy message content to the combined buffer
                    memcpy((uint8_t*)combined_data_buffer + current_offset,
                           current_node->msg_header->msg_content_ptr,
                           current_node->msg_header->msg_size);
                    current_offset += current_node->msg_header->msg_size;
                    break; // Found and copied, move to next index
                }
                current_node = current_node->next;
            }
        }
    }
    return combined_data_buffer;
}

// Function: process_msg
void process_msg(MessageHeader* msg_header) {
    Session* session = find_session(msg_header->session_id);

    if (session == NULL) {
        session = add_session(msg_header); // Add new session if not found
    }

    void* combined_data = add_msg(msg_header, session);

    if (combined_data != NULL) { // All messages for this session have been received and combined
        sendall(combined_data, session->total_data_size);
        free(combined_data);
        delete_session(session->id); // Delete session after processing all its messages
    }
    // If combined_data is NULL, not all messages for this session are received yet,
    // so nothing more to do for this message's processing.
}

// Function: recv_msg
MessageHeader* recv_msg(void) {
    MessageHeader* msg_header = calloc(1, sizeof(MessageHeader)); // Allocate 0x1C bytes for MessageHeader
    if (msg_header == NULL) {
        sendline();
        _terminate();
    }

    // Read the entire MessageHeader struct (0x1C bytes)
    ssize_t bytes_received = recv(g_sockfd, msg_header, sizeof(MessageHeader), 0);
    if (bytes_received < 0) {
        fprintf(stderr, "recv_msg: Error receiving message header.\n");
        free(msg_header);
        sendline();
        _terminate();
    }
    if (bytes_received < sizeof(MessageHeader)) {
        fprintf(stderr, "recv_msg: Incomplete message header received (%zd/%zu bytes).\n", bytes_received, sizeof(MessageHeader));
        free(msg_header);
        sendline();
        _terminate();
    }

    // Validate msg_size (must be between 1 and 256)
    if (msg_header->msg_size == 0 || msg_header->msg_size > 0x100) {
        fprintf(stderr, "recv_msg: Invalid message size %hu.\n", msg_header->msg_size);
        free(msg_header);
        sendline();
        _terminate();
    }

    // Allocate buffer for message content
    msg_header->msg_content_ptr = calloc(msg_header->msg_size, sizeof(uint8_t));
    if (msg_header->msg_content_ptr == NULL) {
        free(msg_header); // Free the header too
        sendline();
        _terminate();
    }

    // Read message content
    bytes_received = recv(g_sockfd, msg_header->msg_content_ptr, msg_header->msg_size, 0);
    if (bytes_received < 0) {
        fprintf(stderr, "recv_msg: Error receiving message content.\n");
        free(msg_header->msg_content_ptr);
        free(msg_header);
        sendline();
        _terminate();
    }
    if (bytes_received < msg_header->msg_size) {
        fprintf(stderr, "recv_msg: Incomplete message content received (%zd/%hu bytes).\n", bytes_received, msg_header->msg_size);
        free(msg_header->msg_content_ptr);
        free(msg_header);
        sendline();
        _terminate();
    }

    return msg_header;
}

// Function: check_number
uint32_t check_number(const uint8_t* param_1, const uint8_t* param_2) {
    // The original code checks nibble by nibble, which is equivalent to byte by byte comparison
    // for 8 bytes.
    return memcmp(param_1, param_2, 8) == 0 ? 1 : 0;
}

// Function: main
int main(void) {
    // Mock socket setup (for compilation; actual implementation would be more complex)
    // In a real scenario, g_sockfd would be initialized via socket(), connect()/accept().
    // For this exercise, we assign a dummy value and assume recv/send are mocked.
    g_sockfd = 3; // A dummy file descriptor value

    // The magic number to check against the first 8 bytes of the received message.
    // The original code initializes `local_1c = 0x73317331;` and `local_18 = 0x3000000;`.
    // When compared using `&local_1c`, it creates an 8-byte sequence.
    // Assuming little-endian system:
    // 0x73317331 -> bytes {0x31, 0x73, 0x31, 0x73} (s1s1)
    // 0x03000000 -> bytes {0x00, 0x00, 0x00, 0x03}
    uint32_t magic_num_val1 = 0x73317331;
    uint32_t magic_num_val2 = 0x03000000;
    uint8_t magic_number[8];
    memcpy(magic_number, &magic_num_val1, 4);
    memcpy(magic_number + 4, &magic_num_val2, 4);

    MessageHeader* current_msg_header = recv_msg();
    if (current_msg_header == NULL) {
        // recv_msg already handles termination on error.
        // This path should only be reached if recv_msg returns NULL without terminating.
        return 0;
    }

    // Initialize outstanding count from the high nibble of the 8th byte (index 7)
    // of the received message header's padding_0_F array.
    outstanding = (uint32_t)(current_msg_header->padding_0_F[7] >> 4);

    while (true) {
        if (check_number(current_msg_header->padding_0_F, magic_number) != 0) {
            process_msg(current_msg_header);
        } else {
            // If magic number doesn't match, free the message and discard it.
            free(current_msg_header->msg_content_ptr);
            free(current_msg_header);
        }

        if (outstanding == 0) {
            break; // Exit loop if no outstanding messages
        }

        current_msg_header = recv_msg();
        if (current_msg_header == NULL) {
            // If recv_msg returns NULL, it means a fatal error occurred and it already terminated.
            return 0;
        }
    }

    // Optional: Cleanup any remaining sessions. Not in original decompiled code.
    // while (head != NULL) {
    //     Session* temp_session = head;
    //     head = head->next;
    //     // Free messages, message headers, msg_avail_map, and session struct itself.
    //     MessageNode* node = (MessageNode*)temp_session->msg_list_head;
    //     while (node != NULL) {
    //         MessageNode* next_node = node->next;
    //         if (node->msg_header) {
    //             free(node->msg_header->msg_content_ptr);
    //             free(node->msg_header);
    //         }
    //         free(node);
    //         node = next_node;
    //     }
    //     free(temp_session->msg_avail_map);
    //     free(temp_session);
    // }

    // Close the mock socket if it was opened
    // if (g_sockfd != -1) {
    //     close(g_sockfd);
    // }

    return 0;
}