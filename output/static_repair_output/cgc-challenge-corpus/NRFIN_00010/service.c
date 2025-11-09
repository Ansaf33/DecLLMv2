#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>     // For send, recv, close
#include <sys/socket.h> // For socket types

// --- Global List Pointers ---
// These are assumed to be initialized elsewhere or by list_create.
// For compilation, we declare them as generic void pointers.
void *transaction_list;
void *card_list;

// --- Placeholder Types and Structures ---
// These are inferred from the decompiled code and are mock-ups for compilation.
// Actual definitions would be in specific header files of the original project.
typedef uint32_t card_id_t;
typedef uint32_t transaction_id_t;
typedef uint32_t amount_t; // For balance, purchase amounts, etc.

// Mock structure for packet data, assuming a 16-byte header
// The offsets (e.g., +8, +9, +10) are based on the original snippet.
typedef struct {
    uint32_t field0;    // Offset 0
    uint32_t socket_fd; // Offset 4 (used as param_2[1] in some calls, assumed to be socket)
    uint8_t pkt_type;   // Offset 8 (bVar1 in dispatch_packet)
    uint8_t op_code;    // Offset 9 (char at param_2 + 9)
    uint8_t error_code_field; // Offset 10 (used in send_error)
    uint8_t reserved[5]; // Remaining bytes to make it 16 bytes
} PacketHeader;

// Mock transaction structure
// Assuming a transaction object holds its ID, and potentially a socket_fd.
typedef struct {
    uint32_t field0;
    uint32_t transaction_id; // Assumed at offset 4 based on some calls
    uint32_t field2;
    uint32_t other_id_or_data; // Assumed at offset 0xc based on process_fin
    // ... other transaction details
} Transaction;


// --- External Function Prototypes (Mocks) ---
// These functions are assumed to exist and perform operations on lists, cards, transactions, and network.
// Their exact signatures are inferred from the decompiled code.
// For simplicity, `int` is used for return values representing success (0) or error (non-zero).

// List operations
int list_create(void* compare_func);
int transaction_node_contains(void* node, void* data); // Placeholder comparison function
int card_node_contains(void* node, void* data);       // Placeholder comparison function

// Network operations (recv_all is likely a wrapper for recv)
ssize_t recv_all(void *buf, size_t len);

// Transaction-related functions
int transaction_recv_issue(void **issue_data_ptr);
int transaction_create_and_add_to_list(void **transaction_handle_ptr, void *list, void *packet_data, card_id_t card_id);
int transaction_set_details(void *transaction_handle, void *details_data);
int transaction_update_state(void *transaction_handle, int state);
int transaction_recv_purchase(void **purchase_data_ptr);
int transaction_recv_recharge(void **recharge_data_ptr);
int transaction_recv_balance(void **balance_data_ptr);
int transaction_send_balance(void *balance_data);
int transaction_recv_history(void **history_data_ptr);
int transaction_get_history_count_and_send(card_id_t card_id, void *list, void *history_request_data);
int transaction_list_send_last_n(card_id_t card_id, void *list, void *history_request_data);
int transaction_recv_refund(void **refund_data_ptr);
int transaction_get_and_verify_purchase_id(void *list, void *refund_data, transaction_id_t *purchase_id_ptr);
int transaction_rm_purchase_details(transaction_id_t *purchase_id_ptr);
int transaction_destroy(void *list, transaction_id_t transaction_id);
int transaction_init_done(void *transaction_handle_ptr, void *packet_data, void *list);
int transaction_authd(void *transaction_handle_ptr, void *packet_data, void *list);
int transaction_ops_done(void *transaction_handle_ptr, void *packet_data, void *list);

// Card-related functions
int card_create_and_add_to_list(void **card_handle_ptr, void *list, void *issue_data);
int card_purchase(card_id_t card_id, void *list, amount_t amount);
int card_recharge(card_id_t card_id, void *list, amount_t amount);
int card_get_balance(card_id_t card_id, void *list, void *balance_data_ptr);
int card_refund(card_id_t card_id, void *list, transaction_id_t purchase_id);
int card_is_valid(card_id_t card_id, uint32_t auth_code, void *list);

// Packet-related functions
// Assumed destination_info is the socket FD based on usage in `main` and other functions.
int packet_from_transaction_and_send(void *transaction_handle, uint32_t destination_socket_fd);

// For simplicity, let's assume a global socket_fd for error sending in this example.
// In a real application, this would be passed around or part of a connection context.
static int current_socket_fd = -1; // Placeholder for the active connection socket

// Function: send_error
// param_1: The socket file descriptor to send the error on.
// param_2: The error code.
// param_3: The error message string.
void send_error(int socket_fd, uint8_t error_code, const char *message) {
    PacketHeader error_packet;
    memset(&error_packet, 0, sizeof(error_packet)); // Clear the packet
    error_packet.pkt_type = 0xFF; // Assuming an error packet type
    error_packet.op_code = error_code; // Place error code here
    error_packet.error_code_field = error_code; // Also place it at offset 10 as per original

    // Send the fixed-size error packet header (16 bytes)
    send(socket_fd, &error_packet, sizeof(error_packet), 0);

    // If no message provided, use a default one
    if (message == NULL) {
        message = "Unknown Error.";
    }

    // Send the length of the error message
    size_t message_len = strlen(message);
    send(socket_fd, &message_len, sizeof(size_t), 0);

    // Send the error message string itself
    send(socket_fd, message, message_len, 0);
}

// Function: send_error_pkt
// param_1: A pointer to a packet context (e.g., the received PacketHeader).
//          From this context, the socket_fd should be derivable, or use a global.
// param_2: The error code.
void send_error_pkt(void *packet_context, uint8_t error_code) {
    // In a real system, 'packet_context' might be used to determine the target socket.
    // For this example, we use the global 'current_socket_fd'.
    int socket_fd = current_socket_fd; // Assuming current_socket_fd is set by main loop

    switch (error_code) {
        case 1:
            send_error(socket_fd, error_code, "Failed to extend list.");
            break;
        case 2:
            send_error(socket_fd, error_code, "Failed to remove node from list.");
            break;
        case 3:
            send_error(socket_fd, error_code, "Failed to allocate memory.");
            break;
        case 4:
            send_error(socket_fd, error_code, "Failed to deallocate memory.");
            break;
        case 5:
            send_error(socket_fd, error_code, "Insufficient funds for purchase.");
            break;
        case 6:
            send_error(socket_fd, error_code, "Unable to recharge card. Balance near max.");
            break;
        case 7:
            send_error(socket_fd, error_code, "Unable to refund card. Balance near max.");
            break;
        case 8:
            send_error(socket_fd, error_code, "No matching transaction history.");
            break;
        case 9:
            send_error(socket_fd, error_code, "Invalid OP_CODE.");
            break;
        case 10:
            send_error(socket_fd, error_code, "Invalid PKT_TYPE.");
            break;
        case 0xb: // 11
            send_error(socket_fd, error_code, "Invalid VENDOR_TYPE.");
            break;
        case 0xc: // 12
            send_error(socket_fd, error_code, "Transaction has not completed AUTH.");
            break;
        case 0xd: // 13
            send_error(socket_fd, error_code, "Transaction has not completed INIT.");
            break;
        case 0xe: // 14
            send_error(socket_fd, error_code, "Transaction has not completed OPS.");
            break;
        case 0xf: // 15
            send_error(socket_fd, error_code, "Card or transaction not found.");
            break;
        case 0x10: // 16
            send_error(socket_fd, error_code, "Invalid card or auth code.");
            break;
        case 0x11: // 17
            send_error(socket_fd, error_code, "Receive failed.");
            break;
        default:
            send_error(socket_fd, error_code, "Unknown Error.");
            break;
    }
}

// Function: issue_card
int issue_card(void *packet_data) {
    int result;
    void *issue_data = NULL;
    void *new_card_handle = NULL;
    void *new_transaction_handle = NULL;

    result = transaction_recv_issue(&issue_data);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data points to a PacketHeader, and its field0 is the card_id
    result = card_create_and_add_to_list(&new_card_handle, card_list, issue_data);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data field0 is card_id and *new_card_handle is another card_id
    // This looks like a decompiler artifact. Let's assume the transaction creation needs
    // the original packet data and the newly created card handle.
    result = transaction_create_and_add_to_list(&new_transaction_handle, transaction_list, packet_data, (card_id_t)(uintptr_t)new_card_handle);
    if (result != 0) {
        return result;
    }

    transaction_set_details(new_transaction_handle, issue_data);
    // Assuming packet_data is PacketHeader*, and its socket_fd is used.
    result = packet_from_transaction_and_send(new_transaction_handle, ((PacketHeader*)packet_data)->socket_fd);
    // The original code has a redundant call if it fails. We just return the error.
    return result;
}

// Function: make_purchase
int make_purchase(void *transaction_handle_ptr, void *packet_data) {
    int result;
    void *purchase_data = NULL; // This will point to the purchase details received

    result = transaction_recv_purchase(&purchase_data);
    if (result != 0) {
        return result;
    }

    transaction_set_details(transaction_handle_ptr, purchase_data);
    transaction_update_state(transaction_handle_ptr, 2); // Assuming state 2 is 'processing'

    // Assuming packet_data is PacketHeader*, and its field0 is card_id.
    // purchase_data is assumed to be a pointer, and we access data at offset 4.
    // This is often ((uint32_t*)purchase_data)[1]
    result = card_purchase( ((PacketHeader*)packet_data)->field0, card_list, *(amount_t*)((uintptr_t)purchase_data + 4));
    if (result != 0) {
        return result;
    }

    // Assuming packet_data is PacketHeader*, and its socket_fd is used.
    result = packet_from_transaction_and_send(transaction_handle_ptr, ((PacketHeader*)packet_data)->socket_fd);
    return result;
}

// Function: recharge_card
int recharge_card(void *transaction_handle_ptr, void *packet_data) {
    int result;
    void *recharge_data = NULL; // This will point to the recharge details received

    result = transaction_recv_recharge(&recharge_data);
    if (result != 0) {
        return result;
    }

    transaction_set_details(transaction_handle_ptr, recharge_data);
    transaction_update_state(transaction_handle_ptr, 2); // Assuming state 2 is 'processing'

    // Assuming packet_data is PacketHeader*, and its field0 is card_id.
    // *recharge_data is assumed to be the amount.
    result = card_recharge(((PacketHeader*)packet_data)->field0, card_list, (amount_t)(uintptr_t)recharge_data);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data is PacketHeader*, and its socket_fd is used.
    result = packet_from_transaction_and_send(transaction_handle_ptr, ((PacketHeader*)packet_data)->socket_fd);
    return result;
}

// Function: get_card_balance
int get_card_balance(void *transaction_handle_ptr, void *packet_data) {
    int result;
    void *balance_data = NULL; // This will point to the balance data received/calculated

    result = transaction_recv_balance(&balance_data);
    if (result != 0) {
        return result;
    }

    transaction_set_details(transaction_handle_ptr, balance_data);
    transaction_update_state(transaction_handle_ptr, 2); // Assuming state 2 is 'processing'

    // Assuming packet_data is PacketHeader*, and its field0 is card_id.
    result = card_get_balance(((PacketHeader*)packet_data)->field0, card_list, balance_data);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data is PacketHeader*, and its socket_fd is used.
    result = packet_from_transaction_and_send(transaction_handle_ptr, ((PacketHeader*)packet_data)->socket_fd);
    if (result != 0) {
        return result;
    }

    transaction_send_balance(balance_data);
    return 0; // Success
}

// Function: get_transaction_history
int get_transaction_history(void *transaction_handle_ptr, void *packet_data) {
    int result;
    void *history_data = NULL; // This will point to the history request data

    result = transaction_recv_history(&history_data);
    if (result != 0) {
        return result;
    }

    transaction_set_details(transaction_handle_ptr, history_data);
    transaction_update_state(transaction_handle_ptr, 2); // Assuming state 2 is 'processing'

    // Assuming packet_data is PacketHeader*, and its field0 is card_id.
    result = packet_from_transaction_and_send(transaction_handle_ptr, ((PacketHeader*)packet_data)->socket_fd);
    if (result != 0) {
        return result;
    }

    // *history_data is assumed to be the card_id for history lookup
    result = transaction_get_history_count_and_send(((PacketHeader*)packet_data)->field0, transaction_list, history_data);
    if (result != 0) {
        return result;
    }

    result = transaction_list_send_last_n(((PacketHeader*)packet_data)->field0, transaction_list, history_data);
    return result;
}

// Function: refund_purchase
int refund_purchase(void *transaction_handle_ptr, void *packet_data) {
    int result;
    void *refund_data = NULL; // This will point to the refund request data
    transaction_id_t purchase_id = 0; // The ID of the purchase to refund

    result = transaction_recv_refund(&refund_data);
    if (result != 0) {
        return result;
    }

    transaction_set_details(transaction_handle_ptr, refund_data);
    transaction_update_state(transaction_handle_ptr, 2); // Assuming state 2 is 'processing'

    result = transaction_get_and_verify_purchase_id(transaction_list, refund_data, &purchase_id);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data is PacketHeader*, and its field0 is card_id.
    result = card_refund(((PacketHeader*)packet_data)->field0, card_list, purchase_id);
    if (result != 0) {
        return result;
    }

    result = transaction_rm_purchase_details(&purchase_id);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data is PacketHeader*, and its socket_fd is used.
    result = packet_from_transaction_and_send(transaction_handle_ptr, ((PacketHeader*)packet_data)->socket_fd);
    return result;
}

// Function: process_op_code
// param_1: A pointer to the current transaction handle (e.g., Transaction*).
// param_2: A pointer to the received packet data (e.g., PacketHeader*).
int process_op_code(void *transaction_handle_ptr, void *packet_data) {
    uint8_t op_code = ((PacketHeader*)packet_data)->op_code;
    int result = 0; // Default success

    switch (op_code) {
        case 0: // Make Purchase
            result = make_purchase(transaction_handle_ptr, packet_data);
            break;
        case 1: // Recharge Card
            result = recharge_card(transaction_handle_ptr, packet_data);
            break;
        case 2: // Get Card Balance
            result = get_card_balance(transaction_handle_ptr, packet_data);
            break;
        case 3: // Get Transaction History
            result = get_transaction_history(transaction_handle_ptr, packet_data);
            break;
        case 5: // Refund Purchase
            result = refund_purchase(transaction_handle_ptr, packet_data);
            break;
        default:
            result = 9; // Invalid OP_CODE
            break;
    }
    return result;
}

// Function: process_fin
// param_1: A pointer to the transaction handle (e.g., Transaction**).
// param_2: A pointer to the received packet data (e.g., PacketHeader*).
int process_fin(void **transaction_handle_ptr_addr, void *packet_data) {
    int result = 0;
    uint8_t op_code = ((PacketHeader*)packet_data)->op_code;
    void *transaction_handle = *transaction_handle_ptr_addr; // Dereference the pointer to get the actual handle

    // If OP_CODE > 4 (i.e., 5 or greater, but only 5 is handled explicitly)
    if (op_code > 4) {
        if (op_code != 5) {
            return 9; // Invalid OP_CODE
        }
        // This block handles OP_CODE 5 (Refund Purchase) after ops_completed.
        // It destroys a transaction. This logic is a bit unusual if refund_purchase already happened.
        // Assuming `transaction_handle` points to a Transaction struct, and its transaction_id is at offset 0xc (index 3 for uint32_t).
        transaction_id_t transaction_id = ((Transaction*)transaction_handle)->other_id_or_data;
        result = transaction_destroy(transaction_list, transaction_id);
        if (result != 0) {
            return result;
        }
    }

    // This part executes if OP_CODE <= 4 OR if OP_CODE == 5 and transaction_destroy succeeded.
    transaction_update_state(transaction_handle, 3); // Assuming state 3 is 'finalized'

    // Assuming packet_data is PacketHeader*, and its socket_fd is used.
    result = packet_from_transaction_and_send(transaction_handle, ((PacketHeader*)packet_data)->socket_fd);
    return result;
}

// Function: auth_new_transaction
// param_1: A pointer to the received packet data (e.g., PacketHeader*).
int auth_new_transaction(void *packet_data) {
    int result;
    void *new_transaction_handle = NULL;

    // Assuming packet_data is PacketHeader*, and its field0 is card_id, socket_fd is auth_code
    result = card_is_valid(((PacketHeader*)packet_data)->field0, ((PacketHeader*)packet_data)->socket_fd, card_list);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data field0 is card_id
    result = transaction_create_and_add_to_list(&new_transaction_handle, transaction_list, packet_data, ((PacketHeader*)packet_data)->field0);
    if (result != 0) {
        return result;
    }

    // Assuming packet_data is PacketHeader*, and its socket_fd is used.
    result = packet_from_transaction_and_send(new_transaction_handle, ((PacketHeader*)packet_data)->socket_fd);
    return result;
}

// Function: init_completed
// param_1: A pointer to the transaction handle (e.g., Transaction**).
// param_2: A pointer to the received packet data (e.g., PacketHeader*).
int init_completed(void **transaction_handle_ptr_addr, void *packet_data) {
    int result;
    void *transaction_handle = *transaction_handle_ptr_addr; // Dereference to get the handle

    // Assuming packet_data is PacketHeader*, and its field0 is card_id, socket_fd is auth_code
    result = card_is_valid(((PacketHeader*)packet_data)->field0, ((PacketHeader*)packet_data)->socket_fd, card_list);
    if (result != 0) {
        return result;
    }

    result = transaction_init_done(transaction_handle, packet_data, transaction_list);
    return result;
}

// Function: auth_completed
// param_1: A pointer to the transaction handle (e.g., Transaction**).
// param_2: A pointer to the received packet data (e.g., PacketHeader*).
int auth_completed(void **transaction_handle_ptr_addr, void *packet_data) {
    int result;
    void *transaction_handle = *transaction_handle_ptr_addr; // Dereference to get the handle

    // Assuming packet_data is PacketHeader*, and its field0 is card_id, socket_fd is auth_code
    result = card_is_valid(((PacketHeader*)packet_data)->field0, ((PacketHeader*)packet_data)->socket_fd, card_list);
    if (result != 0) {
        return result;
    }

    result = transaction_authd(transaction_handle, packet_data, transaction_list);
    return result;
}

// Function: ops_completed
// param_1: A pointer to the transaction handle (e.g., Transaction**).
// param_2: A pointer to the received packet data (e.g., PacketHeader*).
int ops_completed(void **transaction_handle_ptr_addr, void *packet_data) {
    int result;
    void *transaction_handle = *transaction_handle_ptr_addr; // Dereference to get the handle

    // Assuming packet_data is PacketHeader*, and its field0 is card_id, socket_fd is auth_code
    result = card_is_valid(((PacketHeader*)packet_data)->field0, ((PacketHeader*)packet_data)->socket_fd, card_list);
    if (result != 0) {
        return result;
    }

    result = transaction_ops_done(transaction_handle, packet_data, transaction_list);
    return result;
}

// Function: dispatch_packet
// param_1: A pointer to the received packet data (e.g., PacketHeader*).
int dispatch_packet(void *packet_data) {
    int result = 0;
    PacketHeader *pkt = (PacketHeader *)packet_data;
    uint8_t pkt_type = pkt->pkt_type;
    uint8_t op_code = pkt->op_code;
    void *current_transaction_handle = NULL; // Used for PKT_TYPE 2 and 3

    if (pkt_type == 3) { // PKT_TYPE == FIN (Finalize transaction)
        // Check if OP_CODE is one of 0, 1, 2, 3, 4, 5
        if (!((op_code == 0) || (op_code == 1) || (op_code == 2) ||
              (op_code == 3) || (op_code == 4) || (op_code == 5))) {
            return 9; // Invalid OP_CODE
        }

        // The original code uses `local_18 = 0;` and then passes `&local_18`.
        // This implies init_completed/ops_completed might *retrieve* or *create* the transaction handle.
        // Let's assume they retrieve it based on the packet data and store it in `current_transaction_handle`.
        if (op_code == 4) { // OP_CODE == INIT (Init completed)
            result = init_completed(&current_transaction_handle, packet_data);
            if (result != 0) {
                return result;
            }
        } else { // OP_CODE == AUTH, OPS, etc. (Other operations completed)
            result = ops_completed(&current_transaction_handle, packet_data);
            if (result != 0) {
                return result;
            }
        }
        result = process_fin(&current_transaction_handle, packet_data);
        return result;
    }

    if (pkt_type == 2) { // PKT_TYPE == AUTH (Authenticate transaction)
        // Check if OP_CODE is one of 0, 1, 2, 3, 5
        if (!((op_code == 0) || (op_code == 1) || (op_code == 2) ||
              (op_code == 3) || (op_code == 5))) {
            return 9; // Invalid OP_CODE
        }

        // `auth_completed` retrieves/creates the transaction handle.
        result = auth_completed(&current_transaction_handle, packet_data);
        if (result != 0) {
            return result;
        }
        result = process_op_code(current_transaction_handle, packet_data);
        return result;
    }

    if (pkt_type == 0) { // PKT_TYPE == ISSUE (Issue new card)
        if (op_code == 4) { // OP_CODE == INIT (Initialize card issuance)
            result = issue_card(packet_data);
            return result;
        }
        return 9; // Invalid OP_CODE for PKT_TYPE 0
    }

    if (pkt_type == 1) { // PKT_TYPE == INIT (Initialize transaction)
        // Check if OP_CODE is one of 0, 1, 2, 3, 5
        if (!((op_code == 0) || (op_code == 1) || (op_code == 2) ||
              (op_code == 3) || (op_code == 5))) {
            return 9; // Invalid OP_CODE
        }
        result = auth_new_transaction(packet_data);
        return result;
    }

    return 10; // Invalid PKT_TYPE
}

// Function: main
int main(void) {
    PacketHeader received_packet_buffer; // Buffer to receive incoming packets
    void *packet_ptr = NULL;            // Pointer to the received packet buffer
    int operation_result = 0;           // Stores the result of operations, 0 for success

    // Initialize global lists
    transaction_list = list_create(transaction_node_contains);
    if (transaction_list == NULL) {
        operation_result = 3; // Failed to allocate memory
    } else {
        card_list = list_create(card_node_contains);
        if (card_list == NULL) {
            operation_result = 3; // Failed to allocate memory
        } else {
            // Main processing loop
            while (operation_result == 0) {
                packet_ptr = NULL; // Reset packet_ptr for each iteration

                // Receive a fixed-size packet header (16 bytes)
                ssize_t bytes_received = recv_all(&received_packet_buffer, sizeof(received_packet_buffer));

                if (bytes_received != sizeof(received_packet_buffer)) {
                    operation_result = 0x11; // Error code for receive failure
                    // We break here, as we can't process an incomplete packet.
                    // packet_ptr remains NULL, so no error packet is sent based on it.
                    break;
                }

                packet_ptr = &received_packet_buffer; // Point to the received data
                current_socket_fd = received_packet_buffer.socket_fd; // Assume socket_fd is in the packet header

                // Dispatch the packet for processing
                operation_result = dispatch_packet(packet_ptr);

                // If dispatch_packet was successful (operation_result == 0), the loop continues.
                // If dispatch_packet returned an error, operation_result is non-zero, and the loop terminates.
            }
        }
    }

    // After the loop, if an error occurred and a packet was successfully received (or at least buffer was ready)
    if (operation_result != 0 && packet_ptr != NULL) {
        // Send an error response packet using the determined socket_fd
        send_error_pkt(packet_ptr, operation_result & 0xff);
    }

    // Return the final result code
    return operation_result;
}