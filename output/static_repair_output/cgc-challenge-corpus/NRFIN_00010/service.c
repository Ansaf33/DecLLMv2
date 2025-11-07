#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

// Define standard types for the undefined ones
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

// Dummy global list structures
// In a real system, these would be defined in a header
typedef struct List List; // Forward declaration
List *card_list;
List *transaction_list;

// Dummy structures for packet and transaction data for type safety
// These are guesses based on usage. A real system would have these defined.
// Packet structure guess:
// Offset 0-3: some ID/header
// Offset 4-7: socket_fd (int)
// Offset 8: pkt_type (uint8_t)
// Offset 9: op_code (uint8_t)
// ...
typedef struct {
    char data[16]; // Example size, needs to match recv_all(..., 0x10)
} Packet;

// Dummy function declarations to make the code compilable
// Replace 'void *' with actual struct pointers if definitions are available
int transaction_recv_issue(void **data_out);
int card_create_and_add_to_list(void **card_out, List *card_list_ptr, int card_id);
int transaction_create_and_add_to_list(void **trans_out, List *trans_list_ptr, int param_val, int card_id);
void transaction_set_details(void *transaction_obj, void *details_data);
int packet_from_transaction_and_send(void *transaction_obj, int socket_fd);

int transaction_recv_purchase(void **data_out);
void transaction_update_state(void *transaction_obj, int state);
int card_purchase(int card_id, List *card_list_ptr, int amount);

int transaction_recv_recharge(void **data_out);
int card_recharge(int card_id, List *card_list_ptr, int amount);

int transaction_recv_balance(void **data_out);
int card_get_balance(int card_id, List *card_list_ptr, void *balance_out);
void transaction_send_balance(void *balance_data);

int transaction_recv_history(void **data_out);
int transaction_get_history_count_and_send(int card_id, List *trans_list_ptr, int count_param);
int transaction_list_send_last_n(int card_id, List *trans_list_ptr, int n_param);

int transaction_recv_refund(void **data_out);
int transaction_get_and_verify_purchase_id(List *trans_list_ptr, void *data, void *purchase_id_out);
int card_refund(int card_id, List *card_list_ptr, int purchase_id);
int transaction_rm_purchase_details(void *purchase_id_ptr);

int transaction_destroy(List *trans_list_ptr, int transaction_id);

int card_is_valid(int card_id, int auth_code, List *card_list_ptr);
int transaction_init_done(void *transaction_obj, void *packet_data, List *trans_list_ptr);
int transaction_authd(void *transaction_obj, void *packet_data, List *trans_list_ptr);
int transaction_ops_done(void *transaction_obj, void *packet_data, List *trans_list_ptr);

// List management functions
typedef int (*NodeCompareFunc)(void *, void *); // Dummy comparator function type
struct List {
    NodeCompareFunc compare_func;
    // other list fields
};
List *list_create(NodeCompareFunc compare_func);
int transaction_node_contains(void *node_data, void *compare_data);
int card_node_contains(void *node_data, void *compare_data);

// Network functions
int recv_all(void *buffer, size_t len);

// Dummy implementations for compilation
// These functions will simply return 0 (success) or a dummy error code (e.g., 1)
// to allow the provided code to compile and link.
int transaction_recv_issue(void **data_out) { *data_out = malloc(sizeof(int)); if (*data_out) *(int*)*data_out = 100; return 0; }
int card_create_and_add_to_list(void **card_out, List *card_list_ptr, int card_id) { *card_out = malloc(2 * sizeof(int)); if (*card_out) { ((int*)*card_out)[0] = card_id; ((int*)*card_out)[1] = 12345; } return 0; }
int transaction_create_and_add_to_list(void **trans_out, List *trans_list_ptr, int param_val, int card_id) { *trans_out = malloc(4 * sizeof(int)); if (*trans_out) { ((int*)*trans_out)[0] = 1; ((int*)*trans_out)[1] = param_val; ((int*)*trans_out)[2] = card_id; ((int*)*trans_out)[3] = 0; } return 0; }
void transaction_set_details(void *transaction_obj, void *details_data) { /*noop*/ }
int packet_from_transaction_and_send(void *transaction_obj, int socket_fd) { return 0; }

int transaction_recv_purchase(void **data_out) { *data_out = malloc(2 * sizeof(int)); if (*data_out) { ((int*)*data_out)[0] = 100; ((int*)*data_out)[1] = 5000; } return 0; }
void transaction_update_state(void *transaction_obj, int state) { /*noop*/ }
int card_purchase(int card_id, List *card_list_ptr, int amount) { return 0; }

int transaction_recv_recharge(void **data_out) { *data_out = malloc(sizeof(int)); if (*data_out) *(int*)*data_out = 2000; return 0; }
int card_recharge(int card_id, List *card_list_ptr, int amount) { return 0; }

int transaction_recv_balance(void **data_out) { *data_out = malloc(sizeof(int)); if (*data_out) *(int*)*data_out = 1001; return 0; }
int card_get_balance(int card_id, List *card_list_ptr, void *balance_out) { if (balance_out) *(int*)balance_out = 12345; return 0; }
void transaction_send_balance(void *balance_data) { /*noop*/ }

int transaction_recv_history(void **data_out) { *data_out = malloc(sizeof(int)); if (*data_out) *(int*)*data_out = 5; return 0; }
int transaction_get_history_count_and_send(int card_id, List *trans_list_ptr, int count_param) { return 0; }
int transaction_list_send_last_n(int card_id, List *trans_list_ptr, int n_param) { return 0; }

int transaction_recv_refund(void **data_out) { *data_out = malloc(sizeof(int)); if (*data_out) *(int*)*data_out = 123; return 0; }
int transaction_get_and_verify_purchase_id(List *trans_list_ptr, void *data, void *purchase_id_out) { if (purchase_id_out) *(int*)purchase_id_out = 456; return 0; }
int card_refund(int card_id, List *card_list_ptr, int purchase_id) { return 0; }
int transaction_rm_purchase_details(void *purchase_id_ptr) { return 0; }

int transaction_destroy(List *trans_list_ptr, int transaction_id) { return 0; }

int card_is_valid(int card_id, int auth_code, List *card_list_ptr) { return 0; }
int transaction_init_done(void *transaction_obj, void *packet_data, List *trans_list_ptr) { return 0; }
int transaction_authd(void *transaction_obj, void *packet_data, List *trans_list_ptr) { return 0; }
int transaction_ops_done(void *transaction_obj, void *packet_data, List *trans_list_ptr) { return 0; }

List *list_create(NodeCompareFunc compare_func) { List *l = (List*)malloc(sizeof(List)); if (l) l->compare_func = compare_func; return l; }
int transaction_node_contains(void *node_data, void *compare_data) { return 0; }
int card_node_contains(void *node_data, void *compare_data) { return 0; }

int recv_all(void *buffer, size_t len) {
    // Dummy implementation: simulate receiving 16 bytes
    // For main loop to continue, it should return len for a few iterations
    static int call_count = 0;
    if (len == 16) {
        char *buf = (char*)buffer;
        memset(buf, 0, len); // Clear buffer

        if (call_count == 0) { // Simulate first packet: issue card (PKT_TYPE 0, OP_CODE 4)
            buf[8] = 0; // PKT_TYPE = 0 (INIT_PKT)
            buf[9] = 4; // OP_CODE = 4 (ISSUE_CARD)
            *(int*)(buf + 4) = 100; // Dummy socket_fd
        } else if (call_count == 1) { // Simulate second packet: make purchase (auth packet)
            buf[8] = 1; // PKT_TYPE = 1 (AUTH_PKT)
            buf[9] = 0; // OP_CODE = 0 (MAKE_PURCHASE)
            *(int*)(buf + 4) = 100; // Dummy socket_fd
        } else if (call_count == 2) { // Simulate third packet: process_op_code (OPS packet)
            buf[8] = 2; // PKT_TYPE = 2 (OPS_PKT)
            buf[9] = 0; // OP_CODE = 0 (MAKE_PURCHASE)
            *(int*)(buf + 4) = 100; // Dummy socket_fd
        } else { // Simulate disconnect or no more data
            return 0;
        }
        call_count++;
        return 16;
    }
    return 0; // Simulate EOF or error
}

// Function: issue_card
int issue_card(int param_val) {
  void *recv_data = NULL;
  int result = transaction_recv_issue(&recv_data);
  if (result != 0) {
    return result;
  }

  void *new_card = NULL;
  result = card_create_and_add_to_list(&new_card, card_list, *(int*)recv_data);
  if (result != 0) {
    free(recv_data); // Clean up allocated data
    return result;
  }

  void *new_transaction = NULL;
  result = transaction_create_and_add_to_list(&new_transaction, transaction_list, param_val, *(int*)new_card);
  if (result != 0) {
    free(recv_data);
    free(new_card); // Clean up allocated data
    return result;
  }

  transaction_set_details(new_transaction, recv_data);
  result = packet_from_transaction_and_send(new_transaction, ((int*)new_card)[1]);
  
  // Clean up dynamically allocated memory
  free(recv_data);
  free(new_card);
  // new_transaction might be owned by transaction_list, so not freed here.
  return result;
}

// Function: make_purchase
int make_purchase(void *transaction_obj, void *packet_data) {
  void *recv_data = NULL;
  int result = transaction_recv_purchase(&recv_data);
  if (result != 0) {
    return result;
  }

  transaction_set_details(transaction_obj, recv_data);
  transaction_update_state(transaction_obj, 2);
  
  result = card_purchase(*(int*)packet_data, card_list, *(int*)((char*)recv_data + 4));
  if (result != 0) {
    free(recv_data);
    return result;
  }

  result = packet_from_transaction_and_send(*(void**)transaction_obj, ((int*)packet_data)[1]);
  
  free(recv_data);
  return result;
}

// Function: recharge_card
int recharge_card(void *transaction_obj, void *packet_data) {
  void *recv_data = NULL;
  int result = transaction_recv_recharge(&recv_data);
  if (result != 0) {
    return result;
  }

  transaction_set_details(transaction_obj, recv_data);
  transaction_update_state(transaction_obj, 2);
  
  result = card_recharge(*(int*)packet_data, card_list, *(int*)recv_data);
  if (result != 0) {
    free(recv_data);
    return result;
  }

  result = packet_from_transaction_and_send(*(void**)transaction_obj, ((int*)packet_data)[1]);
  
  free(recv_data);
  return result;
}

// Function: get_card_balance
int get_card_balance(void *transaction_obj, void *packet_data) {
  int balance_data = 0; // Use an int for balance, as it's passed by address
  int result = transaction_recv_balance(&balance_data); // Assuming it takes a pointer to int
  if (result != 0) {
    return result;
  }

  transaction_set_details(transaction_obj, &balance_data); // Pass address of int
  transaction_update_state(transaction_obj, 2);
  
  result = card_get_balance(*(int*)packet_data, card_list, &balance_data); // Pass address of int
  if (result != 0) {
    return result;
  }

  result = packet_from_transaction_and_send(*(void**)transaction_obj, ((int*)packet_data)[1]);
  if (result != 0) {
    return result;
  }
  
  transaction_send_balance(&balance_data); // Pass address of int
  return 0; // If packet_from_transaction_and_send succeeded, return 0
}

// Function: get_transaction_history
int get_transaction_history(void *transaction_obj, void *packet_data) {
  void *recv_data = NULL;
  int result = transaction_recv_history(&recv_data);
  if (result != 0) {
    return result;
  }

  transaction_set_details(transaction_obj, recv_data);
  transaction_update_state(transaction_obj, 2);
  
  result = packet_from_transaction_and_send(*(void**)transaction_obj, ((int*)packet_data)[1]);
  if (result != 0) {
    free(recv_data);
    return result;
  }

  result = transaction_get_history_count_and_send(*(int*)packet_data, transaction_list, *(int*)recv_data);
  if (result != 0) {
    free(recv_data);
    return result;
  }

  result = transaction_list_send_last_n(*(int*)packet_data, transaction_list, *(int*)recv_data);
  
  free(recv_data);
  return result;
}

// Function: refund_purchase
int refund_purchase(void *transaction_obj, void *packet_data) {
  int recv_data_val = 0; // Assuming single int value
  int result = transaction_recv_refund(&recv_data_val);
  if (result != 0) {
    return result;
  }

  transaction_set_details(transaction_obj, &recv_data_val);
  transaction_update_state(transaction_obj, 2);
  
  int purchase_id = 0;
  result = transaction_get_and_verify_purchase_id(transaction_list, &recv_data_val, &purchase_id);
  if (result != 0) {
    return result;
  }

  result = card_refund(*(int*)packet_data, card_list, purchase_id);
  if (result != 0) {
    return result;
  }

  result = transaction_rm_purchase_details(&purchase_id);
  if (result != 0) {
    return result;
  }

  result = packet_from_transaction_and_send(*(void**)transaction_obj, ((int*)packet_data)[1]);
  return result;
}

// Function: process_op_code
int process_op_code(void *transaction_obj, void *packet_data) {
  int result;
  uint8_t op_code = ((uint8_t*)packet_data)[9];

  switch(op_code) {
  case 0:
    result = make_purchase(transaction_obj, packet_data);
    break;
  case 1:
    result = recharge_card(transaction_obj, packet_data);
    break;
  case 2:
    result = get_card_balance(transaction_obj, packet_data);
    break;
  case 3:
    result = get_transaction_history(transaction_obj, packet_data);
    break;
  case 5:
    result = refund_purchase(transaction_obj, packet_data);
    break;
  default:
    result = 9; // Invalid OP_CODE
    break;
  }
  return result;
}

// Function: process_fin
int process_fin(void **transaction_obj_ptr, void *packet_data) {
  int result = 0;
  uint8_t op_code = ((uint8_t*)packet_data)[9];

  // Original condition: if (4 < *(byte *)(param_2 + 9)) && (*(byte *)(param_2 + 9) != 5)
  // Which means op_code > 4 AND op_code != 5. This implies op_code is 6, 7, ...
  if (op_code > 4 && op_code != 5) {
    return 9; // Invalid OP_CODE
  }
  
  // If op_code is 5 (refund), destroy transaction
  if (op_code == 5) {
    void* current_transaction = *transaction_obj_ptr;
    // Assuming *(int *)(*param_1 + 0xc) is a pointer to another structure
    // and then *(undefined4 *)(iVar3 + 4) is an ID within that structure.
    // This is complex and highly dependent on actual struct definitions.
    // Making a best guess based on common patterns.
    void* ptr_at_offset_c = *(void**)((char*)current_transaction + 0xC);
    int destroy_id = *(int*)((char*)ptr_at_offset_c + 4);
    
    result = transaction_destroy(transaction_list, destroy_id);
    if (result != 0) {
      return result;
    }
  }

  transaction_update_state(*transaction_obj_ptr, 3);
  result = packet_from_transaction_and_send(*transaction_obj_ptr, ((int*)packet_data)[1]);
  return result;
}

// Function: send_error
void send_error(int socket_fd, uint8_t error_code, const char *error_msg) {
  char error_pkt_header[16]; // Assume a 16-byte fixed-size header
  memset(error_pkt_header, 0, sizeof(error_pkt_header));

  // Set packet type (e.g., to indicate error) - assuming offset 8 for pkt_type
  error_pkt_header[8] = 0xFF; // Example: 0xFF for error packet type
  // Set the error code at the specified offset (offset 10 in original)
  error_pkt_header[10] = error_code; 

  // Send the 16-byte header
  send(socket_fd, error_pkt_header, sizeof(error_pkt_header), 0);

  const char *effective_error_msg = (error_msg == NULL) ? "Unknown Error." : error_msg;
  size_t msg_len = strlen(effective_error_msg);

  // Send the length of the error message
  send(socket_fd, &msg_len, sizeof(size_t), 0);

  // Send the error message itself
  send(socket_fd, effective_error_msg, msg_len, 0);
  return;
}

// Function: send_error_pkt
void send_error_pkt(void *packet_data, uint8_t error_code) {
  // Assuming socket_fd is at offset 4 in the packet_data
  int socket_fd = *(int*)((char*)packet_data + 4);
  
  switch(error_code) {
  default:
    send_error(socket_fd, error_code,"Unknown Error.");
    break;
  case 1:
    send_error(socket_fd, error_code,"Failed to extend list.");
    break;
  case 2:
    send_error(socket_fd, error_code,"Failed to remove node from list.");
    break;
  case 3:
    send_error(socket_fd, error_code,"Failed to allocate memory.");
    break;
  case 4:
    send_error(socket_fd, error_code,"Failed to deallocate memory.");
    break;
  case 5:
    send_error(socket_fd, error_code,"Insuffient funds for purchase.");
    break;
  case 6:
    send_error(socket_fd, error_code,"Unable to recharge card. Balance near max.");
    break;
  case 7:
    send_error(socket_fd, error_code,"Unable to refund card. Balance near max.");
    break;
  case 8:
    send_error(socket_fd, error_code,"No matching transaction history.");
    break;
  case 9:
    send_error(socket_fd, error_code,"Invalid OP_CODE.");
    break;
  case 10:
    send_error(socket_fd, error_code,"Invalid PKT_TYPE.");
    break;
  case 0xb: // 11
    send_error(socket_fd, error_code,"Invalid VENDOR_TYPE.");
    break;
  case 0xc: // 12
    send_error(socket_fd, error_code,"Transaction has not completed AUTH.");
    break;
  case 0xd: // 13
    send_error(socket_fd, error_code,"Transaction has not completed INIT.");
    break;
  case 0xe: // 14
    send_error(socket_fd, error_code,"Transaction has not completed OPS.");
    break;
  case 0xf: // 15
    send_error(socket_fd, error_code,"Card or transaction not found.");
    break;
  case 0x10: // 16
    send_error(socket_fd, error_code,"Invalid card or auth code.");
    break;
  case 0x11: // 17
    send_error(socket_fd, error_code,"Receive failed.");
  }
  return;
}

// Function: auth_new_transaction
int auth_new_transaction(void *packet_data) {
  // Assuming *param_1 is card_id, param_1[1] is auth_code/socket_fd
  int card_id = *(int*)packet_data;
  int auth_code = ((int*)packet_data)[1]; // This might be the socket_fd in this context

  int result = card_is_valid(card_id, auth_code, card_list);
  if (result != 0) {
    return result;
  }

  void *new_transaction = NULL;
  result = transaction_create_and_add_to_list(&new_transaction, transaction_list, card_id, card_id); // param_1 used twice in original
  if (result != 0) {
    return result;
  }

  result = packet_from_transaction_and_send(new_transaction, auth_code); // Using auth_code as socket_fd here
  return result;
}

// Function: init_completed
int init_completed(void *transaction_obj, void *packet_data) {
  int card_id = *(int*)packet_data;
  int auth_code = ((int*)packet_data)[1]; // Or socket_fd

  int result = card_is_valid(card_id, auth_code, card_list);
  if (result != 0) {
    return result;
  }
  
  result = transaction_init_done(transaction_obj, packet_data, transaction_list);
  return result;
}

// Function: auth_completed
int auth_completed(void *transaction_obj, void *packet_data) {
  int card_id = *(int*)packet_data;
  int auth_code = ((int*)packet_data)[1]; // Or socket_fd

  int result = card_is_valid(card_id, auth_code, card_list);
  if (result != 0) {
    return result;
  }
  
  result = transaction_authd(transaction_obj, packet_data, transaction_list);
  return result;
}

// Function: ops_completed
int ops_completed(void *transaction_obj, void *packet_data) {
  int card_id = *(int*)packet_data;
  int auth_code = ((int*)packet_data)[1]; // Or socket_fd

  int result = card_is_valid(card_id, auth_code, card_list);
  if (result != 0) {
    return result;
  }
  
  result = transaction_ops_done(transaction_obj, packet_data, transaction_list);
  return result;
}

// Function: dispatch_packet
int dispatch_packet(void *packet_data) {
  uint8_t pkt_type = ((uint8_t*)packet_data)[8];
  uint8_t op_code = ((uint8_t*)packet_data)[9];
  int result = 0;
  void *transaction_obj = NULL; // Used for local_18 and local_14

  switch (pkt_type) {
    case 0: // INIT_PKT
      if (op_code == 4) { // OP_CODE_ISSUE_CARD
        result = issue_card(*(int*)packet_data); // param_1 for issue_card is an int
      } else {
        result = 9; // Invalid OP_CODE for INIT_PKT
      }
      break;
    case 1: // AUTH_PKT
      // Check for valid OP_CODES for AUTH_PKT
      if (!((op_code == 0) || (op_code == 1) || (op_code == 2) || (op_code == 3) || (op_code == 5))) {
        result = 9; // Invalid OP_CODE
      } else {
        result = auth_new_transaction(packet_data);
      }
      break;
    case 2: // OPS_PKT
      // Check for valid OP_CODES for OPS_PKT
      if (!((op_code == 0) || (op_code == 1) || (op_code == 2) || (op_code == 3) || (op_code == 5))) {
        result = 9; // Invalid OP_CODE
      } else {
        // transaction_obj is initialized to NULL, then passed by address to auth_completed.
        // auth_completed should populate transaction_obj if successful.
        result = auth_completed(&transaction_obj, packet_data);
        if (result == 0) {
          result = process_op_code(transaction_obj, packet_data);
        }
      }
      break;
    case 3: // FIN_PKT
      // Check for valid OP_CODES for FIN_PKT
      if (!((op_code == 0) || (op_code == 1) || (op_code == 2) || (op_code == 3) || (op_code == 4) || (op_code == 5))) {
        result = 9; // Invalid OP_CODE
      } else {
        // transaction_obj is initialized to NULL, then passed by address to init_completed/ops_completed.
        // These functions should populate transaction_obj if successful.
        if (op_code == 4) { // OP_CODE_INIT_DONE
          result = init_completed(&transaction_obj, packet_data);
        } else { // Other OP_CODES for FIN_PKT
          result = ops_completed(&transaction_obj, packet_data);
        }
        
        if (result == 0) {
          result = process_fin(&transaction_obj, packet_data);
        }
      }
      break;
    default:
      result = 10; // Invalid PKT_TYPE
      break;
  }
  
  return result;
}

// Function: main
int main(void) {
  Packet recv_packet_buffer; // Use the Packet struct
  void *packet_ptr = NULL;
  int error_code = 0;
  
  transaction_list = list_create(transaction_node_contains);
  if (transaction_list == NULL) {
    error_code = 3; // Failed to allocate memory
  } else {
    card_list = list_create(card_node_contains);
    if (card_list == NULL) {
      error_code = 3; // Failed to allocate memory
      // In a real scenario, you'd call a list_destroy(transaction_list) here.
    } else {
      while (1) {
        packet_ptr = &recv_packet_buffer; // Point to the buffer for receiving
        int bytes_received = recv_all(packet_ptr, sizeof(Packet));
        
        if (bytes_received != sizeof(Packet)) {
          error_code = 0x11; // Receive failed (or EOF)
          if (bytes_received == 0) { // Clean shutdown, no error to report
              error_code = 0;
              packet_ptr = NULL; // No packet to send error for
          }
          break; // Exit loop on error or EOF
        }
        
        error_code = dispatch_packet(packet_ptr);
        if (error_code != 0) {
          break; // Exit loop on dispatch error
        }
        // If dispatch was successful, reset error_code for the next iteration
        error_code = 0;
        packet_ptr = NULL; // Clear packet_ptr so error is not sent for a successful packet
      }
    }
  }

  if (error_code != 0 && packet_ptr != NULL) {
    send_error_pkt(packet_ptr, (uint8_t)error_code);
  }

  // In a real scenario, you'd call list_destroy(transaction_list) and list_destroy(card_list) here.
  
  return error_code;
}