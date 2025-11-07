#include <stdbool.h> // For bool
#include <stddef.h>  // For size_t
#include <stdint.h>  // For uint32_t, intptr_t

// Type definitions from the original snippet's context
// Assuming 4-byte undefined type for undefined4, char for undefined/byte.
typedef unsigned char undefined;
typedef uint32_t undefined4;
typedef unsigned int uint;
typedef unsigned char byte;

// External function declarations (placeholders)
// These functions are not provided, so they are declared here to make the code compilable.
// Their actual signatures might vary depending on the system/library they belong to.
// The `debug_info` parameters (like 0x110fe) are assumed to be decompilation artifacts
// and are removed or replaced with 0 if they were part of `allocate`.
extern int allocate(size_t size, int flags, void **out_ptr, int debug_info);
extern int deallocate(void *ptr, size_t size);
extern void *node_create(void *data); // Assuming it returns a pointer to the created node
extern int node_destroy(void *node);
extern int list_push(void *list, void *node, void *key_ptr, int debug_info); // Assuming key_ptr is void*
extern void *list_find(void *list, void *key_ptr); // Assuming it returns the node containing the data
extern void *list_remove(void *list, void *key_ptr); // Assuming it returns the removed node
extern int recv_all(void *buf, size_t len);
extern int send(int sockfd, const void *buf, size_t len, int flags);

// Global variables (static to limit scope to this compilation unit)
static unsigned int next_card_id = 1;
static unsigned int next_card_auth_code = 1000;
static unsigned int next_transaction_id = 1;

// Function: transaction_node_contains
bool transaction_node_contains(int *param_1, int *param_2) {
  // *param_1 is treated as a memory address. Offset 8 bytes and dereference as int.
  return *(int *)((char *)*param_1 + 8) != *param_2;
}

// Function: card_node_contains
bool card_node_contains(int **param_1, int *param_2) {
  return **param_1 != *param_2;
}

// Function: card_create
// param_1: out_card_ptr (int**), param_3: initial_balance (int)
// param_2 is unused in the original snippet, removed.
int card_create(int **out_card_ptr, int initial_balance) {
  if (allocate(12, 0, (void **)out_card_ptr, 0) != 0) { // Allocate 12 bytes (for 3 int fields)
    return 3; // Error code for allocation failure
  }

  // Assuming Card structure: [0] ID, [1] Auth Code, [2] Balance
  ((int *)*out_card_ptr)[2] = initial_balance;
  ((int *)*out_card_ptr)[0] = next_card_id;
  ((int *)*out_card_ptr)[1] = next_card_auth_code;

  next_card_id++;
  next_card_auth_code++;
  return 0; // Success
}

// Function: card_add_to_list
// param_1: card_data_ptr (void*), param_2: card_list (void*)
int card_add_to_list(void *card_data_ptr, void *card_list) {
  // The key for list_push is assumed to be the card ID, which is the first int field in card_data_ptr.
  // list_push returns 0 on success, non-zero on failure.
  return list_push(card_list, node_create(card_data_ptr), (void *)(intptr_t)*(int *)card_data_ptr, 0) == 0 ? 0 : 1;
}

// Function: card_create_and_add_to_list
// param_1: out_card_ptr (void**), param_2: card_list (void*), param_3: initial_balance (int)
int card_create_and_add_to_list(void **out_card_ptr, void *card_list, int initial_balance) {
  int result = card_create((int **)out_card_ptr, initial_balance);
  if (result != 0) {
    return result;
  }

  result = card_add_to_list(*out_card_ptr, card_list);
  if (result != 0) {
    return result;
  }
  return 0; // Success
}

// Function: card_get
// param_1: out_card_ptr (void**), param_2: card_id (int), param_3: card_list (void*)
int card_get(void **out_card_ptr, int card_id, void *card_list) {
  void *found_card_node = list_find(card_list, &card_id);
  if (found_card_node == NULL) {
    return 0xf; // Error code 15: Not found
  }
  // Assuming list_find returns a node pointer, and the actual data pointer is the first element of the node.
  *out_card_ptr = *(void **)found_card_node;
  return 0; // Success
}

// Function: card_is_valid
// param_1: card_id (int), param_2: auth_code (int), param_3: card_list (void*)
int card_is_valid(int card_id, int auth_code, void *card_list) {
  int *card_data_ptr = NULL;
  int result = card_get((void **)&card_data_ptr, card_id, card_list);
  if (result != 0) {
    return 0x10; // Error code 16: Invalid card
  }
  // Assuming Card structure: [0] ID, [1] Auth Code, [2] Balance
  if (auth_code == card_data_ptr[1]) {
    return 0; // Success
  }
  return 0x10; // Error code 16: Invalid auth code
}

// Function: card_purchase
// param_1: card_id (int), param_2: card_list (void*), param_3: amount (unsigned int)
int card_purchase(int card_id, void *card_list, unsigned int amount) {
  int *card_data_ptr = NULL;
  int result = card_get((void **)&card_data_ptr, card_id, card_list);
  if (result != 0) {
    return result;
  }
  // Assuming Card structure: [0] ID, [1] Auth Code, [2] Balance
  if (((unsigned int *)card_data_ptr)[2] < amount) {
    return 5; // Error code 5: Insufficient funds
  }
  ((unsigned int *)card_data_ptr)[2] -= amount;
  return 0; // Success
}

// Function: card_recharge
// param_1: card_id (int), param_2: card_list (void*), param_3: amount (unsigned int)
int card_recharge(int card_id, void *card_list, unsigned int amount) {
  int *card_data_ptr = NULL;
  int result = card_get((void **)&card_data_ptr, card_id, card_list);
  if (result != 0) {
    return result;
  }
  // Assuming Card structure: [0] ID, [1] Auth Code, [2] Balance
  // Check for unsigned integer overflow
  if (amount + ((unsigned int *)card_data_ptr)[2] < amount) {
    return 6; // Error code 6: Overflow
  }
  ((unsigned int *)card_data_ptr)[2] += amount;
  return 0; // Success
}

// Function: card_get_balance
// param_1: card_id (int), param_2: card_list (void*), param_3: out_balance_ptr (unsigned int*)
int card_get_balance(int card_id, void *card_list, unsigned int *out_balance_ptr) {
  int *card_data_ptr = NULL;
  int result = card_get((void **)&card_data_ptr, card_id, card_list);
  if (result != 0) {
    return result;
  }
  // Assuming Card structure: [0] ID, [1] Auth Code, [2] Balance
  *out_balance_ptr = ((unsigned int *)card_data_ptr)[2];
  return 0; // Success
}

// Function: card_refund
// param_1: card_id (int), param_2: card_list (void*), param_3: transaction_ptr (int*)
int card_refund(int card_id, void *card_list, int *transaction_ptr) {
  int *card_data_ptr = NULL;

  // Assuming Transaction structure: [0] type, [1] state, [2] card_id, [3] transaction_id, [4] details_ptr
  // Check if card_id matches transaction's card_id (transaction_ptr[1])
  if (card_id != transaction_ptr[1]) {
    return 0xf; // Error code 15: Card ID mismatch
  }

  int result = card_get((void **)&card_data_ptr, card_id, card_list);
  if (result != 0) {
    return result;
  }

  // Get transaction details pointer (at offset 0xc, which is index 3 for void**)
  void *transaction_details_ptr = ((void **)transaction_ptr)[3];

  // Assuming RefundDetails structure: [0] some_id, [1] refund_amount (at offset 4)
  unsigned int refund_amount = ((unsigned int *)transaction_details_ptr)[1];

  // Check for overflow
  if (((unsigned int *)card_data_ptr)[2] + refund_amount < refund_amount) {
    return 7; // Error code 7: Overflow on refund
  }

  ((unsigned int *)card_data_ptr)[2] += refund_amount; // Add refund amount to balance
  return 0; // Success
}

// Function: transaction_create
// param_1: out_transaction_ptr (int**), param_3: transaction_template_ptr (void*)
// param_2 is unused in the original snippet, removed.
int transaction_create(int **out_transaction_ptr, void *transaction_template_ptr) {
  if (allocate(16, 0, (void **)out_transaction_ptr, 0) != 0) { // Allocate 16 bytes (0x10)
    return 3; // Error code for allocation failure
  }

  // Assuming TransactionTemplate structure for param_3:
  // type (byte at offset 9), state (byte at offset 2), card_id (uint32_t at offset 0)
  // Assuming Transaction structure for *out_transaction_ptr:
  // [0] type (byte), [1] state (byte), [4] card_id (uint32_t), [8] transaction_id (int), [0xc] details_ptr (void*)

  ((char *)*out_transaction_ptr)[0] = ((char *)transaction_template_ptr)[9]; // Type
  ((char *)*out_transaction_ptr)[1] = ((char *)transaction_template_ptr)[2]; // State
  ((uint32_t *)*out_transaction_ptr)[1] = *(uint32_t *)transaction_template_ptr; // Card ID (at offset 4)
  ((int *)*out_transaction_ptr)[2] = next_transaction_id;                       // Transaction ID (at offset 8)
  ((void **)*out_transaction_ptr)[3] = NULL;                                   // Details pointer (at offset 0xc)

  next_transaction_id++;
  return 0; // Success
}

// Function: transaction_add_to_list
// param_1: transaction_data_ptr (void*), param_2: transaction_list (void*)
int transaction_add_to_list(void *transaction_data_ptr, void *transaction_list) {
  // The key for list_push is assumed to be the transaction ID, which is the int field at offset 8.
  int transaction_id = ((int *)transaction_data_ptr)[2]; // Offset 8 (index 2 for int*)
  return list_push(transaction_list, node_create(transaction_data_ptr), (void *)(intptr_t)transaction_id, 0) == 0 ? 0 : 1;
}

// Function: transaction_create_and_add_to_list
// param_1: out_transaction_ptr (int**), param_2: transaction_list (void*)
// param_3: transaction_template_ptr (void*), param_4: actual_card_id (int)
int transaction_create_and_add_to_list(int **out_transaction_ptr, void *transaction_list, void *transaction_template_ptr, int actual_card_id) {
  int result = transaction_create(out_transaction_ptr, transaction_template_ptr);
  if (result != 0) {
    return result;
  }

  // Overwrite the card_id with the actual_card_id (at offset 4)
  ((uint32_t *)*out_transaction_ptr)[1] = actual_card_id;

  result = transaction_add_to_list(*out_transaction_ptr, transaction_list);
  if (result != 0) {
    return result;
  }
  return 0; // Success
}

// Function: transaction_get
// param_1: out_transaction_ptr (byte**), param_2: transaction_id (int)
// param_3: transaction_type (unsigned int), param_4: transaction_list (void*)
int transaction_get(byte **out_transaction_ptr, int transaction_id, unsigned int transaction_type, void *transaction_list) {
  void *found_transaction_node = list_find(transaction_list, &transaction_id);
  if (found_transaction_node == NULL) {
    return 0xf; // Error code 15: Not found
  }

  // Assuming list_find returns a node pointer, and the actual data pointer is the first element of the node.
  *out_transaction_ptr = *(byte **)found_transaction_node;

  // Check transaction type (byte at offset 0)
  if (transaction_type == (unsigned int)((char *)*out_transaction_ptr)[0]) {
    return 0; // Success
  }
  return 0xf; // Error code 15: Type mismatch
}

// Function: transaction_update_state
// param_1: transaction_ptr (int*), param_2: new_state (unsigned char)
void transaction_update_state(int *transaction_ptr, unsigned char new_state) {
  // Transaction state is byte at offset 1
  ((char *)*transaction_ptr)[1] = new_state;
}

// Function: transaction_set_details
// param_1: transaction_ptr (int*), param_2: details_ptr (void*)
void transaction_set_details(int *transaction_ptr, void *details_ptr) {
  // Details pointer is at offset 0xc (index 3 for void**)
  ((void **)*transaction_ptr)[3] = details_ptr;
}

// Function: transaction_get_and_verify_purchase_id
// param_1: transaction_list (void*), param_2: purchase_request_data (int*)
// param_3: out_transaction_ptr (int**)
int transaction_get_and_verify_purchase_id(void *transaction_list, int *purchase_request_data, int **out_transaction_ptr) {
  // purchase_request_data[1] is transaction_id, type is 0 (purchase)
  int result = transaction_get((byte **)out_transaction_ptr, purchase_request_data[1], 0, transaction_list);
  if (result != 0) {
    return result;
  }

  // Get transaction details pointer (at offset 0xc, which is index 3 for void**)
  void *transaction_details_ptr = ((void **)*out_transaction_ptr)[3];

  // Assuming PurchaseDetails structure, first int field is purchase_id.
  if (*(int *)transaction_details_ptr == purchase_request_data[0]) {
    return 0; // Success
  }
  return 0xf; // Error code 15: Purchase ID mismatch
}

// Function: transaction_rm_purchase_details
// param_1: transaction_ptr (int*)
int transaction_rm_purchase_details(int *transaction_ptr) {
  // Get details pointer (at offset 0xc, which is index 3 for void**)
  void *details_ptr = ((void **)*transaction_ptr)[3];

  // Assuming PurchaseDetails structure:
  // vendor_location_ptr at offset 0x10 (index 4 for void**)
  // vendor_location_len at offset 0xd (index 0xd for char*)
  void *vendor_location_ptr = ((void **)details_ptr)[4];
  size_t vendor_location_len = ((char *)details_ptr)[0xd];

  if (deallocate(vendor_location_ptr, vendor_location_len) != 0) {
    return 3; // Error: Deallocation of vendor location failed
  }

  if (deallocate(details_ptr, 20) != 0) { // Deallocate details_ptr itself (size 0x14 = 20 bytes)
    return 3; // Error: Deallocation of details failed
  }

  // Set details pointer in transaction to NULL
  ((void **)*transaction_ptr)[3] = NULL;
  return 0; // Success
}

// Function: transaction_rm_from_list
// param_1: out_removed_node_ptr (void**), param_2: transaction_list (void*)
// param_3: transaction_id (int) - added to function signature for clarity
int transaction_rm_from_list(void **out_removed_node_ptr, void *transaction_list, int transaction_id) {
  void *removed_node = list_remove(transaction_list, &transaction_id);
  *out_removed_node_ptr = removed_node;

  if (removed_node == NULL) {
    return 2; // Error code 2: Not found or removal failed
  }
  return 0; // Success
}

// Function: transaction_destroy
// param_1: transaction_list (void*), param_2: transaction_id (int)
int transaction_destroy(void *transaction_list, int transaction_id) {
  void *removed_node = NULL;
  void *transaction_data_ptr = NULL;

  int result = transaction_rm_from_list(&removed_node, transaction_list, transaction_id);
  if (result != 0) {
    return result;
  }

  // Get transaction data pointer from the node (assuming data is first element of node)
  transaction_data_ptr = *(void **)removed_node;

  if (deallocate(transaction_data_ptr, 16) != 0) { // Deallocate transaction data (size 0x10 = 16 bytes)
    return 3; // Error: Deallocation of transaction data failed
  }

  if (node_destroy(removed_node) != 0) { // Destroy the node itself
    return 3; // Error: Node destruction failed
  }
  return 0; // Success
}

// Function: transaction_get_history_count_and_send
// param_1: card_id (int), param_2: transaction_list_head_node (void*)
// param_3: max_count (unsigned int)
int transaction_get_history_count_and_send(int card_id, void *transaction_list_head_node, unsigned int max_count) {
  // Assuming list node structure: struct Node { void* data_ptr; void* prev_ptr; void* next_ptr; };
  // Pointers at offset 0 (data), 4 (prev), 8 (next)
  if (((void **)transaction_list_head_node)[2] == NULL) { // Check if list is empty (next pointer is NULL)
    return 8; // Error code 8: Empty list
  }

  unsigned int history_count = 0;
  void *current_node_ptr = ((void **)transaction_list_head_node)[2]; // Start from head->next

  // Iterate through the circular list until back to head or max_count reached
  while (current_node_ptr != transaction_list_head_node && history_count < max_count) {
    void *transaction_data_ptr = *(void **)current_node_ptr; // Get data_ptr from current node

    // Check if card_id matches transaction's card_id (at offset 4, index 1 for int*)
    if (card_id == ((int *)transaction_data_ptr)[1]) {
      history_count++;
    }
    current_node_ptr = ((void **)current_node_ptr)[2]; // Move to next node (node->next)
  }

  // Assuming transaction_send_history takes the count as an integer.
  transaction_send_history((void *)(intptr_t)history_count); // Cast to void* to match signature
  return 0; // Success
}

// Function: transaction_list_send_last_n
// param_1: card_id (int), param_2: transaction_list_head_node (void*)
// param_3: max_count (unsigned int)
int transaction_list_send_last_n(int card_id, void *transaction_list_head_node, unsigned int max_count) {
  if (((void **)transaction_list_head_node)[2] == NULL) { // Check if list is empty
    return 8; // Error code 8: Empty list
  }

  unsigned int count = 0;
  void *current_node_ptr = ((void **)transaction_list_head_node)[2]; // Start from head->next

  while (current_node_ptr != transaction_list_head_node && count < max_count) {
    void *transaction_data_ptr = *(void **)current_node_ptr; // Get data_ptr from current node

    // Check if card_id matches transaction's card_id (at offset 4, index 1 for int*)
    if (card_id == ((int *)transaction_data_ptr)[1]) {
      // Send transaction header (first 12 bytes = 0xc)
      // Assuming 0 as a placeholder for sockfd and flags
      send(0, transaction_data_ptr, 12, 0);
      transaction_send_details(transaction_data_ptr); // Send details
      count++;
    }
    current_node_ptr = ((void **)current_node_ptr)[2]; // Move to next node
  }
  return 0; // Success
}

// Function: transaction_send_details
// param_1: transaction_data_ptr (void*)
int transaction_send_details(void *transaction_data_ptr) {
  // Transaction type is byte at offset 0
  switch (((char *)transaction_data_ptr)[0]) {
  case 0: // Purchase
    transaction_send_purchase(((void **)transaction_data_ptr)[3]); // Details ptr at offset 0xc
    break;
  case 1: // Recharge
    transaction_send_recharge(((void **)transaction_data_ptr)[3]);
    break;
  case 2: // Balance
    transaction_send_balance(((void **)transaction_data_ptr)[3]);
    break;
  case 3: // History
    transaction_send_history(((void **)transaction_data_ptr)[3]);
    break;
  case 4: // Issue
    transaction_send_issue(((void **)transaction_data_ptr)[3]);
    break;
  case 5: // Refund
    transaction_send_refund(((void **)transaction_data_ptr)[3]);
    break;
  default:
    return 9; // Error code 9: Unknown transaction type
  }
  return 0; // Success
}

// Function: transaction_send_purchase
// param_1: purchase_details_ptr (void*)
void transaction_send_purchase(void *purchase_details_ptr) {
  // Assuming PurchaseDetails structure: amount at offset 4 (index 1 for int*)
  if (((int *)purchase_details_ptr)[1] != 0) { // If amount is not zero
    // Send 14 bytes (0xe) of purchase details
    // Assuming 0 as a placeholder for sockfd and flags
    send(0, purchase_details_ptr, 14, 0);
    // Vendor info (vendor_id, vendor_location_len, vendor_location_ptr) starts at offset 8.
    transaction_send_vendor_loc(((char *)purchase_details_ptr + 8));
  }
}

// Function: transaction_send_recharge
// param_1: recharge_details_ptr (void*)
void transaction_send_recharge(void *recharge_details_ptr) {
  // Assuming RechargeDetails structure: amount at offset 0 (index 0 for int*)
  if (*(int *)recharge_details_ptr != 0) { // If amount is not zero
    // Send 10 bytes (0xa) of recharge details
    // Assuming 0 as a placeholder for sockfd and flags
    send(0, recharge_details_ptr, 10, 0);
    // Vendor info (vendor_id, vendor_location_len, vendor_location_ptr) starts at offset 4.
    transaction_send_vendor_loc(((char *)recharge_details_ptr + 4));
  }
}

// Function: transaction_send_balance
// param_1: balance_details_ptr (void*)
void transaction_send_balance(void *balance_details_ptr) {
  // Send 4 bytes (0x4) of balance details
  // Assuming 0 as a placeholder for sockfd and flags
  send(0, balance_details_ptr, 4, 0);
}

// Function: transaction_send_history
// param_1: history_details_ptr (void*) - could be a pointer to count or history data
// Given usage in `transaction_get_history_count_and_send` it takes an int (count).
// Given usage in `transaction_send_details` it takes a pointer to details.
// Assuming it sends 4 bytes of data pointed to by param_1.
void transaction_send_history(void *history_details_ptr) {
  send(0, history_details_ptr, 4, 0); // Assuming 0 as a placeholder for sockfd and flags
}

// Function: transaction_send_issue
// param_1: issue_details_ptr (void*)
void transaction_send_issue(void *issue_details_ptr) {
  send(0, issue_details_ptr, 4, 0); // Assuming 0 as a placeholder for sockfd and flags
}

// Function: transaction_send_refund
// param_1: refund_details_ptr (void*)
void transaction_send_refund(void *refund_details_ptr) {
  send(0, refund_details_ptr, 8, 0); // Assuming 0 as a placeholder for sockfd and flags
}

// Function: transaction_send_vendor_loc
// param_1: vendor_info_ptr (void*) - pointer to vendor_id field within details structure
void transaction_send_vendor_loc(void *vendor_info_ptr) {
  // Assuming the layout where vendor_info_ptr points to `vendor_id`.
  // `vendor_location_len` is at `vendor_info_ptr + 5`.
  // `vendor_location_ptr` is at `vendor_info_ptr + 8`.
  unsigned char vendor_location_len = ((char *)vendor_info_ptr)[5];
  void *vendor_location_ptr = ((void **)vendor_info_ptr)[2]; // Offset 8 (index 2 for void**)

  // Send the actual vendor location data.
  // Assuming 0 as a placeholder for sockfd and flags.
  send(0, vendor_location_ptr, vendor_location_len, 0);
}

// Function: transaction_recv_issue
// param_1: out_issue_details_ptr (void**)
int transaction_recv_issue(void **out_issue_details_ptr) {
  if (allocate(4, 0, out_issue_details_ptr, 0) != 0) {
    return 3; // Error: Allocation failed
  }
  if (recv_all(*out_issue_details_ptr, 4) != 4) {
    return 0x11; // Error code 17: Incomplete read
  }
  return 0; // Success
}

// Function: transaction_recv_purchase
// param_1: out_purchase_details_ptr (int**)
int transaction_recv_purchase(int **out_purchase_details_ptr) {
  if (allocate(20, 0, (void **)out_purchase_details_ptr, 0) != 0) { // Allocate 20 bytes (0x14) for details struct
    return 3; // Error: Allocation failed
  }

  if (recv_all(*out_purchase_details_ptr, 14) != 14) { // Receive first 14 bytes (0xe)
    return 0x11; // Error: Incomplete read
  }

  // Get vendor_location_len (byte at offset 0xd)
  unsigned int vendor_location_len = ((char *)*out_purchase_details_ptr)[0xd];
  if (vendor_location_len == 0) {
    return 0x11; // Error: Zero length location
  }

  // Allocate buffer for vendor_location_ptr (at offset 0x10)
  void **vendor_location_ptr_addr = (void **)((char *)*out_purchase_details_ptr + 0x10);
  if (allocate(vendor_location_len, 0, vendor_location_ptr_addr, 0) != 0) {
    return 3; // Error: Allocation failed for vendor location
  }

  if (recv_all(*vendor_location_ptr_addr, vendor_location_len) != vendor_location_len) {
    return 0x11; // Error: Incomplete read for vendor location
  }
  return 0; // Success
}

// Function: transaction_recv_recharge
// param_1: out_recharge_details_ptr (int**)
int transaction_recv_recharge(int **out_recharge_details_ptr) {
  if (allocate(16, 0, (void **)out_recharge_details_ptr, 0) != 0) { // Allocate 16 bytes (0x10) for details struct
    return 3; // Error: Allocation failed
  }

  if (recv_all(*out_recharge_details_ptr, 10) != 10) { // Receive first 10 bytes (0xa)
    return 0x11; // Error: Incomplete read
  }

  // Get vendor_location_len (byte at offset 9)
  unsigned int vendor_location_len = ((char *)*out_recharge_details_ptr)[9];
  if (vendor_location_len == 0) {
    return 0x11; // Error: Zero length location
  }

  // Allocate buffer for vendor_location_ptr (at offset 0xc)
  void **vendor_location_ptr_addr = (void **)((char *)*out_recharge_details_ptr + 0xc);
  if (allocate(vendor_location_len, 0, vendor_location_ptr_addr, 0) != 0) {
    return 3; // Error: Allocation failed for vendor location
  }

  if (recv_all(*vendor_location_ptr_addr, vendor_location_len) != vendor_location_len) {
    return 0x11; // Error: Incomplete read for vendor location
  }
  return 0; // Success
}

// Function: transaction_recv_balance
// param_1: out_balance_details_ptr (void**)
int transaction_recv_balance(void **out_balance_details_ptr) {
  if (allocate(4, 0, out_balance_details_ptr, 0) != 0) {
    return 3; // Error: Allocation failed
  }
  // No recv_all in original snippet, implies buffer is allocated but not filled immediately.
  return 0; // Success
}

// Function: transaction_recv_history
// param_1: out_history_details_ptr (void**)
int transaction_recv_history(void **out_history_details_ptr) {
  if (allocate(4, 0, out_history_details_ptr, 0) != 0) {
    return 3; // Error: Allocation failed
  }
  if (recv_all(*out_history_details_ptr, 4) != 4) {
    return 0x11; // Error: Incomplete read
  }
  return 0; // Success
}

// Function: transaction_recv_refund
// param_1: out_refund_details_ptr (void**)
int transaction_recv_refund(void **out_refund_details_ptr) {
  if (allocate(8, 0, out_refund_details_ptr, 0) != 0) {
    return 3; // Error: Allocation failed
  }
  if (recv_all(*out_refund_details_ptr, 8) != 8) {
    return 0x11; // Error: Incomplete read
  }
  return 0; // Success
}

// Function: transaction_init_done
// param_1: out_transaction_ptr (int**), param_2: transaction_request_packet (int*)
// param_3: transaction_list (void*)
int transaction_init_done(int **out_transaction_ptr, int *transaction_request_packet, void *transaction_list) {
  // Extract transaction_id (param_2[3]) and transaction_type (byte at param_2 + 9)
  int transaction_id = transaction_request_packet[3];
  unsigned char transaction_type = ((char *)transaction_request_packet)[9];

  int result = transaction_get((byte **)out_transaction_ptr, transaction_id, transaction_type, transaction_list);
  if (result != 0) {
    return result;
  }

  // Check if card_id from request (param_2[0]) matches transaction's card_id (at offset 4, index 1 for int*)
  if (transaction_request_packet[0] != ((int *)*out_transaction_ptr)[1]) {
    return 0x10; // Error code 16: Card ID mismatch
  }

  // Check transaction state (byte at offset 1). '\0' means initial state.
  if (((char *)*out_transaction_ptr)[1] == '\0') {
    return 0; // Success
  }
  return 0xd; // Error code 13: Not in initial state
}

// Function: transaction_authd
// param_1: out_transaction_ptr (int**), param_2: transaction_request_packet (int*)
// param_3: transaction_list (void*)
int transaction_authd(int **out_transaction_ptr, int *transaction_request_packet, void *transaction_list) {
  int transaction_id = transaction_request_packet[3];
  unsigned char transaction_type = ((char *)transaction_request_packet)[9];

  int result = transaction_get((byte **)out_transaction_ptr, transaction_id, transaction_type, transaction_list);
  if (result != 0) {
    return result;
  }

  if (transaction_request_packet[0] != ((int *)*out_transaction_ptr)[1]) {
    return 0x10; // Error code 16: Card ID mismatch
  }

  // Check transaction state (byte at offset 1). '\x01' means authorized state.
  if (((char *)*out_transaction_ptr)[1] == '\x01') {
    return 0; // Success
  }
  return 0xc; // Error code 12: Not in authorized state
}

// Function: transaction_ops_done
// param_1: out_transaction_ptr (int**), param_2: transaction_request_packet (int*)
// param_3: transaction_list (void*)
int transaction_ops_done(int **out_transaction_ptr, int *transaction_request_packet, void *transaction_list) {
  int transaction_id = transaction_request_packet[3];
  unsigned char transaction_type = ((char *)transaction_request_packet)[9];

  int result = transaction_get((byte **)out_transaction_ptr, transaction_id, transaction_type, transaction_list);
  if (result != 0) {
    return result;
  }

  if (transaction_request_packet[0] != ((int *)*out_transaction_ptr)[1]) {
    return 0x10; // Error code 16: Card ID mismatch
  }

  // Check transaction state (byte at offset 1). '\x02' means operations done state.
  if (((char *)*out_transaction_ptr)[1] == '\x02') {
    return 0; // Success
  }
  return 0xe; // Error code 14: Not in operations done state
}

// Function: packet_from_transaction
// param_1: transaction_data_ptr (void*), param_2: out_packet_ptr (int**)
// param_3: value_for_packet_field_4 (unsigned int) - value to be placed at offset 4 in the packet
int packet_from_transaction(void *transaction_data_ptr, int **out_packet_ptr, unsigned int param_3_value) {
  if (allocate(16, 0, (void **)out_packet_ptr, 0) != 0) { // Allocate 16 bytes (0x10) for packet
    return 3; // Error: Allocation failed
  }

  // Assuming Packet structure for *out_packet_ptr:
  // [0] card_id (uint32_t), [4] param_3_value (uint32_t), [8] transaction_state (byte)
  // [9] transaction_type (byte), [10] transaction_detail_size (byte), [0xc] transaction_id (uint32_t)

  ((uint32_t *)*out_packet_ptr)[0] = ((uint32_t *)transaction_data_ptr)[1]; // Card ID from transaction (offset 4)
  ((uint32_t *)*out_packet_ptr)[1] = param_3_value;                         // Value provided as param_3_value (offset 4)
  ((char *)*out_packet_ptr)[8] = ((char *)transaction_data_ptr)[1];         // Transaction state (offset 1)
  ((char *)*out_packet_ptr)[9] = ((char *)transaction_data_ptr)[0];         // Transaction type (offset 0)
  ((char *)*out_packet_ptr)[10] = ((char *)transaction_data_ptr)[2];        // Transaction detail size (offset 2)
  ((uint32_t *)*out_packet_ptr)[3] = ((uint32_t *)transaction_data_ptr)[2]; // Transaction ID (offset 8)
  return 0; // Success
}

// Function: packet_from_transaction_and_send
// param_1: transaction_data_ptr (void*)
// Original param_2 (size_t) was inconsistent with `packet_from_transaction` and removed.
int packet_from_transaction_and_send(void *transaction_data_ptr) {
  int *packet_ptr = NULL;

  // Call packet_from_transaction with 0 for the third parameter (param_3_value)
  int result = packet_from_transaction(transaction_data_ptr, &packet_ptr, 0);
  if (result != 0) {
    return result;
  }

  // Send the allocated packet. The packet size is 16 bytes (0x10).
  // Assuming 0 as a placeholder for sockfd and flags.
  send(0, packet_ptr, 16, 0);

  if (deallocate(packet_ptr, 16) != 0) {
    return 3; // Error: Deallocation failed
  }
  return 0; // Success
}