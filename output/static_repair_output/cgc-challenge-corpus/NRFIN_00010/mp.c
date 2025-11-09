#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> // For NULL, size_t
#include <stdlib.h> // For malloc, free
#include <sys/socket.h> // For send, recv
#include <unistd.h> // For ssize_t

// Type aliases for decompiled types
typedef uint8_t undefined;
typedef uint32_t undefined4;

// Global variables (assuming they are global based on usage)
static uint32_t next_card_id = 1;
static uint32_t next_card_auth_code = 1;
static uint32_t next_transaction_id = 1;

// --- Dummy / Wrapper Functions for external dependencies ---

// Memory allocation wrapper: returns 0 on success, non-zero on failure
int allocate(size_t size, int flags, void **out_ptr) {
    *out_ptr = malloc(size);
    return (*out_ptr == NULL) ? 3 : 0;
}

// Memory deallocation wrapper: returns 0 on success
int deallocate(void *ptr, size_t size) { // size param is ignored by free
    free(ptr);
    return 0;
}

// Dummy declarations for list/node management functions
// These functions are assumed to exist elsewhere and manage linked lists of generic nodes.
void *node_create(void *data) { return data; } // Simplistic dummy
int list_push(void *list, void *node, void *key_ptr) { return 0; } // Simplistic dummy, 0 for success
void *list_find(void *list, void *key_ptr) { return NULL; } // Simplistic dummy, NULL for not found
void *list_remove(void *list, void *key_ptr) { return NULL; } // Simplistic dummy, NULL for not found/removed
int node_destroy(void *node) { return 0; } // Simplistic dummy, 0 for success

// Wrapper for recv_all: reads 'count' bytes into 'buffer' from 'fd'
// Returns number of bytes received on success, or -1 on error/connection closed.
ssize_t recv_all(int fd, void *buffer, size_t count) {
    ssize_t total_received = 0;
    uint8_t *buf = (uint8_t*)buffer;
    while (total_received < count) {
        ssize_t bytes_received = recv(fd, buf + total_received, count - total_received, 0);
        if (bytes_received <= 0) {
            return bytes_received; // Error or connection closed
        }
        total_received += bytes_received;
    }
    return total_received;
}

// --- Original Functions (fixed) ---

// Function: transaction_node_contains
bool transaction_node_contains(int *param_1, int *param_2) {
  return *(int *)((char *)param_1 + 8) != *param_2;
}

// Function: card_node_contains
bool card_node_contains(int **param_1, int *param_2) {
  return **param_1 != *param_2;
}

// Function: card_create
uint32_t card_create(int **param_1, uint32_t param_2, int param_3) {
  if (allocate(0xc, 0, (void**)param_1) == 0) {
    (*param_1)[2] = param_3; // Balance
    **param_1 = next_card_id++; // Card ID
    (*param_1)[1] = next_card_auth_code++; // Auth Code
    return 0;
  }
  return 3;
}

// Function: card_add_to_list
bool card_add_to_list(void *card_obj, void *card_list) {
  return list_push(card_list, node_create(card_obj), card_obj) != 0;
}

// Function: card_create_and_add_to_list
uint32_t card_create_and_add_to_list(int **param_1, void *param_2, int param_3) {
  if (card_create(param_1, *(uint32_t *)((char *)param_2 + 8), param_3) == 0) {
    if (card_add_to_list(*param_1, param_2) == 0) { // First attempt to add
      return 0;
    }
    return card_add_to_list(*param_1, param_2); // Second attempt, likely an error path in original
  }
  return 3; // card_create failed
}

// Function: card_get
uint32_t card_get(uint32_t *param_1, uint32_t param_2, void *param_3) {
  uint32_t *puVar1 = (uint32_t *)list_find(param_3, &param_2);
  if (puVar1 == NULL) {
    return 0xf;
  }
  *param_1 = *puVar1;
  return 0;
}

// Function: card_is_valid
uint32_t card_is_valid(uint32_t param_1, int param_2, void *param_3) {
  int local_card_ptr = 0;
  if (card_get((uint32_t *)&local_card_ptr, param_1, param_3) == 0) {
    if (param_2 == *(int *)((char *)&local_card_ptr + 4)) {
      return 0;
    }
    return 0x10;
  }
  return 0x10;
}

// Function: card_purchase
uint32_t card_purchase(uint32_t param_1, void *param_2, uint32_t param_3) {
  int local_card_ptr = 0;
  if (card_get((uint32_t *)&local_card_ptr, param_1, param_2) == 0) {
    if (*(uint32_t *)((char *)&local_card_ptr + 8) < param_3) {
      return 5;
    }
    *(uint32_t *)((char *)&local_card_ptr + 8) -= param_3;
    return 0;
  }
  return 0xf; // card_get failed
}

// Function: card_recharge
uint32_t card_recharge(uint32_t param_1, void *param_2, uint32_t param_3) {
  int local_card_ptr = 0;
  if (card_get((uint32_t *)&local_card_ptr, param_1, param_2) == 0) {
    if (param_3 + *(int *)((char *)&local_card_ptr + 8) < param_3) { // Overflow check
      return 6;
    }
    *(uint32_t *)((char *)&local_card_ptr + 8) = param_3 + *(int *)((char *)&local_card_ptr + 8);
    return 0;
  }
  return 0xf; // card_get failed
}

// Function: card_get_balance
uint32_t card_get_balance(uint32_t param_1, void *param_2, uint32_t *param_3) {
  int local_card_ptr = 0;
  if (card_get((uint32_t *)&local_card_ptr, param_1, param_2) == 0) {
    *param_3 = *(uint32_t *)((char *)&local_card_ptr + 8);
    return 0;
  }
  return 0xf; // card_get failed
}

// Function: card_refund
uint32_t card_refund(int param_1, void *param_2, void *param_3) {
  if (param_1 != *(int *)((char *)param_3 + 4)) {
    return 0xf;
  }
  int local_card_ptr = 0;
  if (card_get((uint32_t *)&local_card_ptr, param_1, param_2) == 0) {
    int refund_amount = *(int *)((char *)param_3 + 0xc); // Pointer to transaction refund details
    if ((uint32_t)(*(int *)((char *)&local_card_ptr + 8) + *(int *)((char *)&refund_amount + 4)) < *(uint32_t *)((char *)&refund_amount + 4)) { // Overflow check
      return 7;
    }
    *(int *)((char *)&local_card_ptr + 8) = *(int *)((char *)&refund_amount + 4) + *(int *)((char *)&local_card_ptr + 8);
    return 0;
  }
  return 0xf; // card_get failed
}

// Function: transaction_create
uint32_t transaction_create(int *param_1, uint32_t param_2, uint32_t *param_3) {
  if (allocate(0x10, 0, (void**)param_1) == 0) { // Allocate 16 bytes for transaction object
    *(undefined *)*param_1 = *(undefined *)((char *)param_3 + 9); // transaction_type
    *(undefined *)((char *)*param_1 + 1) = *(undefined *)((char *)param_3 + 2); // state
    *(uint32_t *)((char *)*param_1 + 4) = *param_3; // card_id
    *(int *)((char *)*param_1 + 8) = next_transaction_id++; // transaction_id
    *(uint32_t *)((char *)*param_1 + 0xc) = 0; // details_ptr
    return 0;
  }
  return 3;
}

// Function: transaction_add_to_list
bool transaction_add_to_list(void *transaction_obj, void *transaction_list) {
  return list_push(transaction_list, node_create(transaction_obj), (char *)transaction_obj + 8) != 0;
}

// Function: transaction_create_and_add_to_list
uint32_t transaction_create_and_add_to_list(int *param_1, void *param_2, uint32_t *param_3, uint32_t param_4) {
  if (transaction_create(param_1, *(uint32_t *)((char *)param_2 + 8), param_3) == 0) {
    *(uint32_t *)((char *)*param_1 + 4) = param_4; // Set card_id again? (original code sets it at offset 4 and 0xc, looks like a typo, assuming 0xc is details)
    if (transaction_add_to_list(*param_1, param_2) == 0) {
      return 0;
    }
    return transaction_add_to_list(*param_1, param_2); // Second attempt, likely error path
  }
  return 3; // transaction_create failed
}

// Function: transaction_get
uint32_t transaction_get(uint8_t **param_1, uint32_t param_2, uint8_t param_3, void *param_4) {
  uint8_t **ppbVar1 = (uint8_t **)list_find(param_4, &param_2); // param_2 is transaction_id
  if (ppbVar1 == NULL) {
    return 0xf;
  }
  *param_1 = *ppbVar1;
  if (param_3 == **param_1) { // param_3 is transaction_type
    return 0;
  }
  return 0xf;
}

// Function: transaction_update_state
void transaction_update_state(int *param_1, undefined param_2) {
  *(undefined *)((char *)*param_1 + 1) = param_2; // Update state byte
}

// Function: transaction_set_details
void transaction_set_details(int *param_1, uint32_t param_2) {
  *(uint32_t *)((char *)*param_1 + 0xc) = param_2; // Set details pointer
}

// Function: transaction_get_and_verify_purchase_id
uint32_t transaction_get_and_verify_purchase_id(void *param_1, int *param_2, int *param_3) {
  uint8_t *transaction_obj_ptr;
  if (transaction_get(&transaction_obj_ptr, *(uint32_t *)((char *)param_2 + 4), 0, param_1) == 0) { // Get transaction with type 0 (purchase)
    *param_3 = (int)transaction_obj_ptr; // Store transaction object ptr in param_3
    if (**(int **)((char *)*param_3 + 0xc) == *param_2) { // Compare card ID from details with param_2
      return 0;
    }
    return 0xf;
  }
  return 0xf; // transaction_get failed
}

// Function: transaction_rm_purchase_details
uint32_t transaction_rm_purchase_details(int *param_1) {
  int details_ptr = *(int *)((char *)*param_1 + 0xc); // Get details pointer
  if (deallocate(*(uint32_t *)((char *)&details_ptr + 0x10), *(undefined *)((char *)&details_ptr + 0xd)) == 0) { // Deallocate vendor location string
    if (deallocate((void *)details_ptr, 0x14) == 0) { // Deallocate purchase details struct
      *(uint32_t *)((char *)*param_1 + 0xc) = 0; // Clear details pointer in transaction object
      return 0;
    }
  }
  return 3; // Deallocation failed
}

// Function: transaction_rm_from_list
uint32_t transaction_rm_from_list(uint32_t **removed_item_ptr, void *transaction_list, uint32_t transaction_id_to_remove) {
  *removed_item_ptr = (uint32_t *)list_remove(transaction_list, &transaction_id_to_remove);
  return (*removed_item_ptr == NULL) ? 2 : 0; // Return 2 if not found/removed, 0 if successful
}

// Function: transaction_destroy
uint32_t transaction_destroy(void *transaction_list, uint32_t transaction_id) {
  uint32_t *local_transaction_obj_ptr = NULL;
  if (transaction_rm_from_list(&local_transaction_obj_ptr, transaction_list, transaction_id) == 0) {
    if (deallocate(local_transaction_obj_ptr, 0x10) == 0) { // Deallocate transaction object (16 bytes)
      if (node_destroy(local_transaction_obj_ptr) == 0) {
        return 0;
      }
      return 3; // node_destroy failed
    }
    return 3; // deallocate transaction object failed
  }
  return 0xf; // transaction_rm_from_list failed (transaction not found or removed)
}

// Function: transaction_send_purchase
void transaction_send_purchase(int client_fd, uint32_t details_ptr) {
  uint8_t command_code = 0xE; // Purchase command code
  send(client_fd, &command_code, sizeof(command_code), 0);
  if (*(int *)((char *)&details_ptr + 4) != 0) { // If amount is not zero
    send(client_fd, (char *)&details_ptr + 4, sizeof(uint32_t), 0); // Send amount
    // Assuming transaction_send_vendor_loc sends vendor data
    transaction_send_vendor_loc(client_fd, (char *)&details_ptr + 8);
  }
  // The original function returned `undefined **` which was `&_GLOBAL_OFFSET_TABLE_`.
  // This is a decompilation artifact; actual return value is not used. Changed to void.
}

// Function: transaction_send_recharge
void transaction_send_recharge(int client_fd, int *param_1) {
  uint8_t command_code = 0xA; // Recharge command code
  send(client_fd, &command_code, sizeof(command_code), 0);
  if (*param_1 != 0) { // If recharge amount is not zero
    send(client_fd, param_1, sizeof(uint32_t), 0); // Send recharge amount
    // Assuming transaction_send_vendor_loc sends vendor data
    transaction_send_vendor_loc(client_fd, (char *)param_1 + 1);
  }
  // The original function returned `undefined **` which was `&_GLOBAL_OFFSET_TABLE_`.
  // This is a decompilation artifact; actual return value is not used. Changed to void.
}

// Function: transaction_send_balance
void transaction_send_balance(int client_fd, int param_1) {
  uint8_t command_code = 0x4; // Balance command code
  send(client_fd, &command_code, sizeof(command_code), 0);
  send(client_fd, &param_1, sizeof(uint32_t), 0); // Send balance value
}

// Function: transaction_send_history
void transaction_send_history(int client_fd, int param_1) {
  uint8_t command_code = 0x4; // History command code
  send(client_fd, &command_code, sizeof(command_code), 0);
  send(client_fd, &param_1, sizeof(uint32_t), 0); // Send history count
}

// Function: transaction_send_issue
void transaction_send_issue(int client_fd, int param_1) {
  uint8_t command_code = 0x4; // Issue command code
  send(client_fd, &command_code, sizeof(command_code), 0);
  send(client_fd, &param_1, sizeof(uint32_t), 0); // Send issue details (e.g., card ID)
}

// Function: transaction_send_refund
void transaction_send_refund(int client_fd, int param_1) {
  uint8_t command_code = 0x8; // Refund command code
  send(client_fd, &command_code, sizeof(command_code), 0);
  send(client_fd, &param_1, sizeof(uint32_t), 0); // Send refund details
}

// Function: transaction_send_vendor_loc
void transaction_send_vendor_loc(int client_fd, void *param_1) {
  // param_1 is expected to be a pointer to vendor location data
  // *(int *)(param_1 + 8) is likely the length of the string
  // (uint)*(byte *)(param_1 + 5) is likely the vendor location string itself (or a pointer to it)
  uint8_t loc_len = *(uint8_t *)((char *)param_1 + 5);
  void *loc_data = *(void **)((char *)param_1 + 8); // Assuming this is a pointer to the location string
  send(client_fd, loc_data, loc_len, 0);
}

// Function: transaction_send_details
uint32_t transaction_send_details(int client_fd, undefined *param_1) {
  switch(*param_1) { // *param_1 is transaction type
  case 0: // Purchase
    transaction_send_purchase(client_fd, *(uint32_t *)((char *)param_1 + 0xc));
    break;
  case 1: // Recharge
    transaction_send_recharge(client_fd, (int *)(*(uint32_t *)((char *)param_1 + 0xc)));
    break;
  case 2: // Balance
    transaction_send_balance(client_fd, *(uint32_t *)((char *)param_1 + 0xc));
    break;
  case 3: // History
    transaction_send_history(client_fd, *(uint32_t *)((char *)param_1 + 0xc));
    break;
  case 4: // Issue
    transaction_send_issue(client_fd, *(uint32_t *)((char *)param_1 + 0xc));
    break;
  case 5: // Refund
    transaction_send_refund(client_fd, *(uint32_t *)((char *)param_1 + 0xc));
    break;
  default:
    return 9;
  }
  return 0;
}

// Function: transaction_get_history_count_and_send
uint32_t transaction_get_history_count_and_send(int client_fd, int param_1, int **param_2, uint32_t param_3) {
  if (param_2[2] == NULL) {
    return 8;
  }
  uint32_t local_history_count = 0;
  int *local_current_node = (int *)param_2[1][2]; // Start from head of list
  while ((local_current_node != *param_2) && (local_history_count < param_3)) {
    if (param_1 == *(int *)((char *)local_current_node + 4)) { // Compare card ID
      local_history_count++;
    }
    local_current_node = (int *)local_current_node[2]; // Move to next node
  }
  transaction_send_history(client_fd, local_history_count);
  return 0;
}

// Function: transaction_list_send_last_n
uint32_t transaction_list_send_last_n(int client_fd, int param_1, int **param_2, uint32_t param_3) {
  if (param_2[2] == NULL) {
    return 8;
  }
  uint32_t local_count = 0;
  int *local_current_node = (int *)param_2[1][2]; // Start from head of list
  while ((local_current_node != *param_2) && (local_count < param_3)) {
    if (param_1 == *(int *)((char *)local_current_node + 4)) { // Compare card ID
      uint8_t msg_type_code = 0xC; // Assuming 0xC is a message type for sending transaction
      send(client_fd, &msg_type_code, sizeof(msg_type_code), 0);
      transaction_send_details(client_fd, (undefined *)local_current_node);
      local_count++;
    }
    local_current_node = (int *)local_current_node[2]; // Move to next node
  }
  return 0;
}


// Function: transaction_recv_issue
uint32_t transaction_recv_issue(uint32_t *param_1, int client_fd) {
  if (allocate(4, 0, (void**)param_1) == 0) { // Allocate 4 bytes for issue details (e.g., card ID)
    if (recv_all(client_fd, *param_1, 4) == 4) {
      return 0;
    }
    return 0x11; // Receive error
  }
  return 3; // Allocation failed
}

// Function: transaction_recv_purchase
uint32_t transaction_recv_purchase(int *param_1, int client_fd) {
  if (allocate(0x14, 0, (void**)param_1) == 0) { // Allocate 20 bytes for purchase details
    if (recv_all(client_fd, *param_1, 0xe) == 0xe) { // Receive 14 bytes (fixed part of purchase details)
      uint32_t vendor_loc_len = (uint32_t)*(uint8_t *)((char *)*param_1 + 0xd); // Vendor location length
      if (vendor_loc_len == 0) {
        return 0x11; // Invalid length
      }
      if (allocate(vendor_loc_len, 0, (void**)((char *)*param_1 + 0x10)) == 0) { // Allocate for vendor location string
        if (recv_all(client_fd, *(uint32_t *)((char *)*param_1 + 0x10), vendor_loc_len) == vendor_loc_len) {
          return 0;
        }
        return 0x11; // Receive error for vendor location
      }
      return 3; // Allocation failed for vendor location
    }
    return 0x11; // Receive error for purchase details
  }
  return 3; // Allocation failed for purchase details
}

// Function: transaction_recv_recharge
uint32_t transaction_recv_recharge(int *param_1, int client_fd) {
  if (allocate(0x10, 0, (void**)param_1) == 0) { // Allocate 16 bytes for recharge details
    if (recv_all(client_fd, *param_1, 10) == 10) { // Receive 10 bytes (fixed part of recharge details)
      uint32_t vendor_loc_len = (uint32_t)*(uint8_t *)((char *)*param_1 + 9); // Vendor location length
      if (vendor_loc_len == 0) {
        return 0x11; // Invalid length
      }
      if (allocate(vendor_loc_len, 0, (void**)((char *)*param_1 + 0xc)) == 0) { // Allocate for vendor location string
        if (recv_all(client_fd, *(uint32_t *)((char *)*param_1 + 0xc), vendor_loc_len) == vendor_loc_len) {
          return 0;
        }
        return 0x11; // Receive error for vendor location
      }
      return 3; // Allocation failed for vendor location
    }
    return 0x11; // Receive error for recharge details
  }
  return 3; // Allocation failed for recharge details
}

// Function: transaction_recv_balance
uint32_t transaction_recv_balance(uint32_t *param_1) {
  // Balance recv only allocates, actual balance value is typically sent back by server
  return (allocate(4, 0, (void**)param_1) == 0) ? 0 : 3;
}

// Function: transaction_recv_history
uint32_t transaction_recv_history(uint32_t *param_1, int client_fd) {
  if (allocate(4, 0, (void**)param_1) == 0) { // Allocate 4 bytes for history count
    if (recv_all(client_fd, *param_1, 4) == 4) {
      return 0;
    }
    return 0x11; // Receive error
  }
  return 3; // Allocation failed
}

// Function: transaction_recv_refund
uint32_t transaction_recv_refund(uint32_t *param_1, int client_fd) {
  if (allocate(8, 0, (void**)param_1) == 0) { // Allocate 8 bytes for refund details (e.g., transaction ID, amount)
    if (recv_all(client_fd, *param_1, 8) == 8) {
      return 0;
    }
    return 0x11; // Receive error
  }
  return 3; // Allocation failed
}

// Function: transaction_init_done
uint32_t transaction_init_done(int *param_1, int *param_2, void *param_3) {
  uint8_t *transaction_obj_ptr;
  if (transaction_get(&transaction_obj_ptr, *(uint32_t *)((char *)param_2 + 0xc), *(undefined *)((char *)param_2 + 9), param_3) == 0) {
    *param_1 = (int)transaction_obj_ptr; // Store transaction object ptr in param_1
    if (*param_2 == *(int *)((char *)*param_1 + 4)) { // Compare card ID
      if (*(char *)((char *)*param_1 + 1) == '\0') { // Check if state is initial (0)
        return 0;
      }
      return 0xd; // Not in initial state
    }
    return 0x10; // Card ID mismatch
  }
  return 0xf; // transaction_get failed
}

// Function: transaction_authd
uint32_t transaction_authd(int *param_1, int *param_2, void *param_3) {
  uint8_t *transaction_obj_ptr;
  if (transaction_get(&transaction_obj_ptr, *(uint32_t *)((char *)param_2 + 0xc), *(undefined *)((char *)param_2 + 9), param_3) == 0) {
    *param_1 = (int)transaction_obj_ptr; // Store transaction object ptr in param_1
    if (*param_2 == *(int *)((char *)*param_1 + 4)) { // Compare card ID
      if (*(char *)((char *)*param_1 + 1) == '\x01') { // Check if state is authorized (1)
        return 0;
      }
      return 0xc; // Not in authorized state
    }
    return 0x10; // Card ID mismatch
  }
  return 0xf; // transaction_get failed
}

// Function: transaction_ops_done
uint32_t transaction_ops_done(int *param_1, int *param_2, void *param_3) {
  uint8_t *transaction_obj_ptr;
  if (transaction_get(&transaction_obj_ptr, *(uint32_t *)((char *)param_2 + 0xc), *(undefined *)((char *)param_2 + 9), param_3) == 0) {
    *param_1 = (int)transaction_obj_ptr; // Store transaction object ptr in param_1
    if (*param_2 == *(int *)((char *)*param_1 + 4)) { // Compare card ID
      if (*(char *)((char *)*param_1 + 1) == '\x02') { // Check if state is operations done (2)
        return 0;
      }
      return 0xe; // Not in operations done state
    }
    return 0x10; // Card ID mismatch
  }
  return 0xf; // transaction_get failed
}

// Function: packet_from_transaction
uint32_t packet_from_transaction(undefined *param_1, int *param_2, uint32_t param_3) {
  if (allocate(0x10, 0, (void**)param_2) == 0) { // Allocate 16 bytes for packet
    *(uint32_t *)*param_2 = *(uint32_t *)((char *)param_1 + 4); // card_id
    *(uint32_t *)((char *)*param_2 + 4) = param_3; // client_fd
    *(undefined *)((char *)*param_2 + 8) = param_1[1]; // state
    *(undefined *)((char *)*param_2 + 9) = *param_1; // transaction_type
    *(undefined *)((char *)*param_2 + 10) = param_1[2]; // unknown byte
    *(uint32_t *)((char *)*param_2 + 0xc) = *(uint32_t *)((char *)param_1 + 8); // transaction_id
    return 0;
  }
  return 3; // Allocation failed
}

// Function: packet_from_transaction_and_send
uint32_t packet_from_transaction_and_send(undefined *param_1, size_t param_2) {
  int local_packet_ptr = 0;
  if (packet_from_transaction(param_1, &local_packet_ptr, 0) == 0) { // Create packet, client_fd is 0 for now
    // Assuming *(uint32_t*)local_packet_ptr + 4 is the client_fd
    int client_fd = *(uint32_t *)((char *)&local_packet_ptr + 4);
    send(client_fd, (void *)local_packet_ptr, 0x10, 0); // Send the 16-byte packet
    return deallocate((void *)local_packet_ptr, 0x10); // Deallocate packet
  }
  return 3; // packet_from_transaction failed
}