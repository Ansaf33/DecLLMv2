#define _GNU_SOURCE // For dprintf

#include <stdarg.h> // For va_list in fdprintf implementation
#include <stdio.h>  // For dprintf (if available) or basic I/O functions
#include <stdlib.h> // For malloc, free, exit
#include <string.h> // For memcpy, strlen (if needed, not explicitly in original)
#include <unistd.h> // For read, write, dprintf

// Type definitions to match original usage (e.g., undefined4 -> unsigned int)
typedef unsigned int uint;
typedef unsigned char byte;

// Message structure inferred from pointer arithmetic and indexing in the original code.
// The fields are named generically as 'fieldX' because their meaning changes based on 'type'.
typedef struct MessageHeader {
    struct MessageHeader *next; // The first field, used for linked list traversal
    unsigned int type;          // Message type (e.g., 0x4f4c4548 for HELO)
    unsigned int field2;        // Can be version/secure_flag (HELO), key_ptr (KEYX), seq_num (DATA), text_ptr (TEXT)
    unsigned int field3;        // Can be ttl (HELO), key_len (KEYX), data_ptr (DATA), text_len (TEXT)
    unsigned int field4;        // Can be options (KEYX), data_len (DATA)
} MessageHeader;

// Global variables
MessageHeader *head = NULL;
MessageHeader *tail = NULL;

// External function declarations (assuming they are provided elsewhere or need to be stubbed for compilation)
// Simple fdprintf implementation using dprintf for compilation.
// If dprintf is not available, a custom implementation using vsnprintf and write would be needed.
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vdprintf(fd, format, args);
    va_end(args);
    return ret;
}

// These functions are assumed to be external and their implementations are not provided.
// Minimal declarations are given to allow compilation.
int read_n(int fd, void *buf, size_t count);
int decode_data(int *key, int *data_payload_info);
MessageHeader *parse_msg(unsigned char *buffer, unsigned int len);

// String literals for fdprintf (from original code's DAT_xxxx addresses)
const char *SECURE_YES_STR = "yes";             // Corresponds to DAT_0001401a
const char *SECURE_NO_STR = "no";               // Corresponds to DAT_00014017
const char *SECURE_MESSAGE_HEADER_STR = "SECURE MESSAGE:\n"; // Corresponds to DAT_00014168
const char *SECURE_MESSAGE_FOOTER_STR = "END SECURE MESSAGE\n"; // Corresponds to DAT_0001416b
const char *SEQ_DUP_FORMAT = "(SEQ #%d DUP)";
const char *SEQ_MISSING_FORMAT = "(SEQ #%d MISSING)";
const char *DECODED_DATA_FORMAT = "%s"; // Assuming decoded data is a string


// Function: sorted
int **sorted(int **param_1) {
    if (param_1 == NULL) {
        return NULL;
    }

    int **sorted_list_head = NULL;
    int **current_unsorted_node = param_1;

    // Iterate through the unsorted list, taking one node at a time to insert into the sorted list.
    while (current_unsorted_node != NULL) {
        int **node_to_insert = current_unsorted_node;
        // Store the pointer to the next unsorted node *before* node_to_insert[1] is potentially modified.
        current_unsorted_node = (int **)node_to_insert[1];

        // Insert node_to_insert into the sorted_list_head
        if (sorted_list_head == NULL || **node_to_insert < **sorted_list_head) {
            // Insert at the beginning of the sorted list
            node_to_insert[1] = (int *)sorted_list_head;
            sorted_list_head = node_to_insert;
        } else {
            // Traverse the sorted list to find the insertion point, preserving original (potentially buggy) logic.
            int **traverse_ptr = sorted_list_head;
            while (traverse_ptr != NULL) {
                // The original condition: if ((local_10[1] == (int *)0x0) || (**local_c < **local_10))
                // This condition determines where to insert. If true, insert after 'traverse_ptr'.
                // This comparison can lead to an unsorted list if **node_to_insert is less than **traverse_ptr
                // but inserted after it. This behavior is preserved as requested.
                if (traverse_ptr[1] == NULL || **node_to_insert < **traverse_ptr) {
                    node_to_insert[1] = (int *)traverse_ptr[1];
                    traverse_ptr[1] = (int *)node_to_insert;
                    break; // Break from this inner loop once inserted
                }
                traverse_ptr = (int **)traverse_ptr[1];
            }
        }
    }
    return sorted_list_head;
}

// Function: queue_msg
void queue_msg(MessageHeader *msg) {
    if (msg != NULL) {
        msg->next = NULL; // Ensure the new message is the end of the list for now
        if (head == NULL) {
            head = msg;
        } else {
            // Original: *tail = (int)param_1; which implies tail points to the 'next' field of the last message
            // With MessageHeader struct, this translates to tail->next = msg;
            tail->next = msg;
        }
        tail = msg;
        fdprintf(1, "QUEUED\n");
    }
}

// Function: clear_queue
void clear_queue(void) {
    MessageHeader *current = head;
    while (current != NULL) {
        // Free data fields based on message type
        if (current->type == 0x5859454b /* KEYX */) {
            if ((void *)current->field2 != NULL) { // field2 holds key data pointer
                free((void *)current->field2);
            }
        } else if (current->type == 0x41544144 /* DATA */) {
            if ((void *)current->field3 != NULL) { // field3 holds data pointer
                free((void *)current->field3);
            }
        } else if (current->type == 0x54584554 /* TEXT */) {
            if ((void *)current->field2 != NULL) { // field2 holds text message pointer
                free((void *)current->field2);
            }
        }
        MessageHeader *next_msg = current->next; // Store next pointer before freeing current
        free(current);
        current = next_msg;
    }
    head = NULL; // Reset head and tail after clearing
    tail = NULL;
    fdprintf(1, "QUEUE CLEARED\n");
}

// Function: process
void process(void) {
    unsigned int processed_count = 0;
    MessageHeader *helo_msg_ptr = NULL; // Points to the HELO message header
    int *key_data_ptr = NULL;           // Points to the KEYX key data
    int **data_msgs_head = NULL;        // Head of the linked list of DATA messages (for sorting)
    int **data_msgs_tail = NULL;        // Tail of the linked list of DATA messages

    MessageHeader *current_msg = head;
    int process_early_termination = 0; // Flag to indicate if processing should terminate early (was LAB_00011f03 goto)

    while (current_msg != NULL) {
        // Original condition for `goto LAB_00011f03`:
        // `(local_24 == (undefined4 *)0x0)` is handled by `current_msg != NULL` loop condition.
        // `(local_18 != (undefined4 *)0x0 && ((int)local_18[1] < local_10))`
        // `local_18` is `helo_msg_ptr`, `local_18[1]` is `helo_msg_ptr->field3` (TTL), `local_10` is `processed_count`.
        if (helo_msg_ptr != NULL && helo_msg_ptr->field3 < processed_count) {
            process_early_termination = 1;
            break; // Exit the main message processing loop
        }

        switch (current_msg->type) {
            case 0x4f4c4548 /* HELO */: {
                if (helo_msg_ptr == NULL) { // Only accept the first HELO message
                    // Version check: first byte of field2
                    if (*((char *)&current_msg->field2) == 'g') {
                        helo_msg_ptr = current_msg; // Store pointer to the HELO message
                        // Secure flag: second byte of field2
                        const char *secure_str = (*((char *)&current_msg->field2 + 1) == '\0') ? SECURE_YES_STR : SECURE_NO_STR;
                        fdprintf(1, "HELO [VERSION %d] [SECURE %s] [TTL %d]\n",
                                 (int)*((char *)&current_msg->field2), secure_str, current_msg->field3);
                    } else {
                        fdprintf(1, "INVALID VERSION\n");
                    }
                }
                break;
            }
            case 0x5859454b /* KEYX */: {
                if (helo_msg_ptr == NULL) { process_early_termination = 1; break; } // HELO MISSING
                // Check secure flag (second byte of HELO's field2)
                if (*((char *)&helo_msg_ptr->field2 + 1) == '\0') {
                    fdprintf(1, "KEYX IN NON-SECURE\n");
                    process_early_termination = 1; break;
                }
                if (key_data_ptr == NULL) { // Only accept the first KEYX message
                    if (current_msg->field3 == 0) { // Key length is 0
                        fdprintf(1, "NO KEY\n");
                        process_early_termination = 1; break;
                    }
                    key_data_ptr = (int *)current_msg->field2; // field2 holds the key data pointer
                    fdprintf(1, "KEYX [OPTION ");
                    unsigned char options = *((unsigned char *)&current_msg->field4); // field4 holds options
                    if ((options & 0xf) == 7) {
                        fdprintf(1, "prepend | ");
                    } else {
                        fdprintf(1, "append | ");
                    }
                    if ((options & 0xf0U) == 0x30) {
                        fdprintf(1, "inverted] ");
                        for (unsigned int i = 0; i < current_msg->field3; ++i) { // field3 is key_len
                            ((unsigned char *)key_data_ptr)[i] = ~((unsigned char *)key_data_ptr)[i];
                        }
                    } else {
                        fdprintf(1, "as-is] ");
                    }
                    fdprintf(1, "[LEN %d]\n", current_msg->field3);
                }
                break;
            }
            case 0x41544144 /* DATA */: {
                if (helo_msg_ptr == NULL) { process_early_termination = 1; break; } // HELO MISSING
                // Check secure flag (second byte of HELO's field2)
                if (*((char *)&helo_msg_ptr->field2 + 1) == '\0') {
                    fdprintf(1, "DATA IN NON-SECURE\n");
                    process_early_termination = 1; break;
                }
                if (key_data_ptr == NULL) {
                    fdprintf(1, "DATA BEFORE KEYX\n");
                    process_early_termination = 1; break;
                }
                fdprintf(1, "DATA [SEQ %d] [LEN %d]\n", current_msg->field2, current_msg->field4);

                // Add to linked list of data messages. Each node is `int**` pointing to `int*` (payload info).
                int **new_data_node = (int **)malloc(sizeof(int *) * 2); // Allocate space for {payload_info_ptr, next_node_ptr}
                if (new_data_node == NULL) { /* handle error */ process_early_termination = -1; break; }

                int *data_payload_info = (int *)malloc(sizeof(int) * 2); // {seq_num, data_ptr}
                if (data_payload_info == NULL) { free(new_data_node); process_early_termination = -1; break; }
                data_payload_info[0] = current_msg->field2; // field2 is sequence number
                data_payload_info[1] = current_msg->field3; // field3 is data pointer

                new_data_node[0] = data_payload_info; // Store the payload info pointer
                new_data_node[1] = NULL;              // Initialize next pointer

                if (data_msgs_head == NULL) {
                    data_msgs_head = new_data_node;
                    data_msgs_tail = new_data_node;
                } else {
                    data_msgs_tail[1] = (int *)new_data_node;
                    data_msgs_tail = new_data_node;
                }
                break;
            }
            case 0x54584554 /* TEXT */: {
                if (helo_msg_ptr == NULL) { process_early_termination = 1; break; } // HELO MISSING
                // Check secure flag (second byte of HELO's field2)
                if (*((char *)&helo_msg_ptr->field2 + 1) != '\0') {
                    fdprintf(1, "TEXT IN SECURE\n");
                    process_early_termination = 1; break;
                }
                if (current_msg->field3 == 0) { // Text length is 0
                    fdprintf(1, "NO TEXT MSG\n");
                    process_early_termination = 1; break;
                }
                fdprintf(1, "TEXT [LEN %d] [MSG %s]\n", current_msg->field3, (char *)current_msg->field2);
                break;
            }
            default:
                // Unknown message type, continue processing or handle error
                break;
        }

        if (process_early_termination != 0) {
            break; // Exit the main message processing loop if an error or early termination condition was met
        }
        current_msg = current_msg->next;
        processed_count++;
    }

    // This block corresponds to LAB_00011f03 in the original code, executed after the main loop.
    if (helo_msg_ptr == NULL) {
        fdprintf(1, "HELO MISSING\n");
    }

    if (data_msgs_head != NULL) {
        int **sorted_data_msgs = sorted(data_msgs_head);
        fdprintf(1, SECURE_MESSAGE_HEADER_STR);
        int expected_seq = 1;
        int last_seq_processed = 0; // Corresponds to local_34 in original
        int **current_sorted_node = sorted_data_msgs;

        while (current_sorted_node != NULL) {
            int current_seq = (*current_sorted_node)[0]; // **local_2c in original

            if (last_seq_processed == current_seq) { // Check for duplicates (original logic)
                fdprintf(1, SEQ_DUP_FORMAT, current_seq);
                current_sorted_node = (int **)current_sorted_node[1];
            } else {
                if (expected_seq == current_seq) {
                    // Call decode_data with key_data_ptr (local_14) and data_payload_info (*local_2c)
                    int decode_result = decode_data(key_data_ptr, *current_sorted_node);
                    if (decode_result != 0) {
                        // Original code had a bug here, printing SECURE_MESSAGE_HEADER_STR.
                        // Corrected to print the data pointer from the payload info.
                        fdprintf(1, DECODED_DATA_FORMAT, (char *)(*current_sorted_node)[1]);
                    }
                    current_sorted_node = (int **)current_sorted_node[1];
                } else {
                    fdprintf(1, SEQ_MISSING_FORMAT, expected_seq);
                }
                last_seq_processed = expected_seq; // Matches original logic for local_34 = local_30
                expected_seq++;
            }
        }
        fdprintf(1, SECURE_MESSAGE_FOOTER_STR);
    }
    fdprintf(1, "PROCESS DONE\n");
    clear_queue(); // Clears messages and resets head/tail
}

// Function: quit
void quit(void) {
    fdprintf(1, "QUIT\n");
    exit(0); // WARNING: Subroutine does not return
}

// Function: main
int main(void) {
    unsigned int total_msg_len; // Corresponds to local_18
    // Buffer to hold the message type (4 bytes) and its payload.
    // Max message length is 0x800.
    unsigned char msg_buffer[0x800]; 

    while (1) {
        int bytes_read = read_n(0, &total_msg_len, 4); // Read total length of the upcoming message
        if (bytes_read < 1) {
            return 0xffffffff;
        }
        // total_msg_len includes the 4 bytes for message type.
        // The original code checks if 0x800 < local_18 or local_18 < 4.
        if (total_msg_len > 0x800 || total_msg_len < 4) {
            return 0xffffffff;
        }

        // Read the message type and payload into msg_buffer
        // original: read_n(0, &local_818, local_18) where local_818 is stack variable at start of buffer.
        // This reads total_msg_len bytes starting at &msg_type, followed by payload.
        bytes_read = read_n(0, msg_buffer, total_msg_len); 
        if (bytes_read < 1) {
            return 0xffffffff;
        }

        int msg_type = *(int *)msg_buffer; // Extract message type from the beginning of the buffer

        if (msg_type == 2) {
            quit();
        } else if (msg_type == 0) {
            // Pass the payload part of the buffer (after msg_type) and its length
            MessageHeader *new_msg = parse_msg(msg_buffer + 4, total_msg_len - 4);
            if (new_msg == NULL) {
                fdprintf(1, "FAILED TO QUEUE\n");
            } else {
                queue_msg(new_msg);
            }
        } else if (msg_type == 1) {
            process();
        } else {
            return 0xffffffff; // Unknown message type
        }
    }
    return 0; // Should not be reached in this infinite loop unless exit is called.
}