#include <stdbool.h> // For bool type
#include <stdint.h>  // For uint8_t, uint16_t, uint32_t
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For memset, memcmp

// --- Placeholder External Function Declarations ---
// These functions are assumed to be defined elsewhere and linked.
// Their signatures are inferred from the provided snippet.

// Binary Search Tree (BST) functions
// Assuming BST stores uint16_t addresses.
typedef void* bst_t; // Abstract type for BST handle
extern bst_t bst_init(void); // Initializes a new BST and returns its handle
extern void* bst_search(bst_t tree_handle, uint16_t key); // Returns pointer to key data if found, NULL otherwise
extern void bst_insert(bst_t tree_handle, uint16_t key_to_insert); // Inserts a key into the BST
extern void* bst_iter_start(bst_t tree_handle, int order); // Returns pointer to the first key (uint16_t) in iteration
extern void* bst_iter_next(bst_t tree_handle); // Returns pointer to the next key (uint16_t) in iteration

// Linked List functions
typedef void* list_t;     // Abstract type for list handle
typedef void* list_node_t; // Abstract type for list node
typedef bool (*list_compare_func_t)(void*, void*); // Function pointer for sorting

extern void list_init(list_t* list_handle_ptr, int param); // Initializes a list handle
extern int list_length(list_t list_handle);
extern void list_insert_at_end(list_t list_handle, void* data);
extern void* list_pop(list_t list_handle); // Returns data from the head, removes node
extern void list_destroy_node(list_t list_handle, list_node_t* node_ptr); // Destroys a specific node (if list_pop returns node, not data)
extern list_node_t list_head_node(list_t list_handle); // Returns the head node
extern list_node_t list_end_marker(list_t list_handle); // Returns an end marker for iteration
extern list_node_t list_next_node(list_node_t node); // Returns the next node
extern void* list_node_data(list_node_t node); // Returns data stored in a node
extern void list_insert_sorted(list_t list_handle, void* data, list_compare_func_t compare_func, int param);

// I/O and Stamp functions
extern int recv_bytes(void* buffer, size_t size);
extern int send_bytes(int flag, const void* buffer, size_t size);
extern void init_stamp_roll(void);
extern int use_stamp(void); // Returns stamp value or -1 on error
extern int get_new_stamp(void); // Returns new stamp value or 0 on error

// --- Custom Data Structures ---

// Mailbox structure inferred from (param_1 + 4) access patterns
typedef struct Mailbox {
    uint32_t id;       // Identifier for the mailbox
    list_t mail_list; // Handle to the list of mail in this mailbox
} Mailbox;

// Mail structure inferred from access patterns (e.g., mail_ptr + 2, mail_ptr + 4)
// Assuming a mail item is a dynamically allocated struct pointer.
#define MAIL_CONTENT_SIZE 400 // 0x184 (total mail size) - 4 bytes (addresses)
typedef struct Mail {
    uint16_t sender_address;
    uint16_t receiver_address;
    uint8_t content[MAIL_CONTENT_SIZE];
} Mail;

// Temporary struct for sorting undeliverable mail
typedef struct {
    uint16_t receiver;
    uint16_t sender;
} MailAddrPair;

// --- Global Variables ---
bst_t mailboxes; // Global BST handle for addresses

// Global Mailbox instances
Mailbox undeliverable_box_inst;
Mailbox lost_box_inst;
Mailbox received_box_inst;
Mailbox sorted_box_inst;

// Command strings (assuming these are global constants)
const char CMD_ADD_ADDRESS[] = "ADD_";
const char CMD_RECEIVE_MAIL[] = "RECV";
const char CMD_LIST_ADDRESSES[] = "LSTA";
const char CMD_SORT_MAIL[] = "SORT";
const char CMD_LIST_UNDELIVERABLE_MAIL[] = "LSTU";
const char CMD_DELIVER_MAIL[] = "DELV";
const char CMD_RETURN_TO_SENDER[] = "RTNS";
const char CMD_LIST_LOST_MAIL[] = "LSTL";
const char CMD_DESTROY_LOST_MAIL[] = "DSTL";
const char CMD_BUY_POSTAGE[] = "BUYP";
const char CMD_QUIT[] = "QUIT";

// --- Helper for _terminate() ---
// Replaces decompiler's _terminate() with standard exit.
void _terminate(int status) {
    exit(status);
}

// --- Function Implementations ---

// Function: address_exists
bool address_exists(uint16_t address) {
  return bst_search(mailboxes, address) != NULL;
}

// Function: get_mailbox_for_address
Mailbox* get_mailbox_for_address(uint16_t address) {
  // Original function returned void after calling bst_search.
  // However, it is used in do_deliver_mail as a Mailbox* destination.
  // This is an inferred behavior: if the address exists, mail is delivered to the received_box.
  // A more complete system would have per-user mailboxes.
  if (bst_search(mailboxes, address) != NULL) {
    return &received_box_inst;
  }
  return NULL; // Address not found, cannot deliver
}

// Function: get_undeliverable_mailbox
Mailbox* get_undeliverable_mailbox(void) {
  return &undeliverable_box_inst;
}

// Function: get_lost_mailbox
Mailbox* get_lost_mailbox(void) {
  return &lost_box_inst;
}

// Function: get_received_mailbox
Mailbox* get_received_mailbox(void) {
  return &received_box_inst;
}

// Function: get_sorted_mailbox
Mailbox* get_sorted_mailbox(void) {
  return &sorted_box_inst;
}

// Function: delete_mail
void delete_mail(void *mail_ptr) {
  free(mail_ptr);
}

// Function: is_mailbox_empty
bool is_mailbox_empty(Mailbox* mailbox_ptr) {
  return mailbox_ptr->mail_list == NULL || list_length(mailbox_ptr->mail_list) == 0;
}

// Function: put_mail_in_mailbox
void put_mail_in_mailbox(void* mail_ptr, Mailbox* mailbox_ptr) {
  if (mailbox_ptr->mail_list == NULL) {
    list_init(&mailbox_ptr->mail_list, 0); // Initialize list if not already
  }
  list_insert_at_end(mailbox_ptr->mail_list, mail_ptr);
}

// Function: pop_mail_from_mailbox
void* pop_mail_from_mailbox(Mailbox* mailbox_ptr) {
  if (is_mailbox_empty(mailbox_ptr)) {
    return NULL;
  }
  // Assuming list_pop returns the data pointer directly and handles node destruction
  return list_pop(mailbox_ptr->mail_list);
}

// Function: do_receive_mail
uint32_t do_receive_mail(void) {
  int bytes_received;
  int stamp_status;
  Mail* new_mail;

  bytes_received = recv_bytes(NULL, 0); // Assuming recv_bytes without args implies a setup phase or global buffer
  if (bytes_received < 0) {
    _terminate(0xfffffff5); // Error code from decompiler
  }

  stamp_status = use_stamp();
  if (stamp_status == -1) {
    return 0xffffffff; // Error
  }

  new_mail = (Mail*)malloc(sizeof(Mail)); // Allocate space for a new mail item (0x184 bytes)
  if (new_mail == NULL) {
    _terminate(0xfffffff9); // Memory allocation error
  }

  // Assuming the first recv_bytes prepared for a subsequent read into `new_mail`
  // The size 0x184 (404 bytes) is inferred from the original malloc.
  bytes_received = recv_bytes(new_mail, sizeof(Mail));
  if (bytes_received < 0) {
    free(new_mail); // Clean up allocated memory
    _terminate(0xfffffff5); // Error code from decompiler
  }

  put_mail_in_mailbox(new_mail, get_received_mailbox());
  return 0; // Success
}

// Function: do_list_addresses
uint32_t do_list_addresses(void) {
  uint16_t* current_address_ptr;
  int bytes_sent;

  current_address_ptr = (uint16_t*)bst_iter_start(mailboxes, 1); // Start iteration (in-order)

  if (current_address_ptr == NULL) {
    return 0xffffffff; // No addresses found
  }

  while (current_address_ptr != NULL) {
    // Send the address (assuming 2 bytes for uint16_t)
    bytes_sent = send_bytes(1, current_address_ptr, sizeof(uint16_t));
    if (bytes_sent != sizeof(uint16_t)) {
      _terminate(0xfffffff7); // Send error
    }
    current_address_ptr = (uint16_t*)bst_iter_next(mailboxes); // Get next address
  }
  return 0; // Success
}

// Function: do_add_address
uint32_t do_add_address(void) {
  uint16_t new_address = 0;
  int bytes_received;

  // Receive the address (assuming 2 bytes for uint16_t)
  bytes_received = recv_bytes(&new_address, sizeof(uint16_t));
  if (bytes_received < 0) {
    _terminate(0xfffffff5); // Receive error
  }

  // Check if address is valid (e.g., > 9) and doesn't already exist
  if (new_address > 9 && !address_exists(new_address)) {
    bst_insert(mailboxes, new_address);
    return 0; // Success
  }
  return 0xffffffff; // Invalid address or already exists
}

// Function: do_sort_mail
uint32_t do_sort_mail(void) {
  Mail* mail_ptr;

  if (is_mailbox_empty(get_received_mailbox())) {
    return 0xffffffff; // Received mailbox is empty
  }

  while (true) {
    mail_ptr = (Mail*)pop_mail_from_mailbox(get_received_mailbox());
    if (mail_ptr == NULL) break; // No more mail

    if (address_exists(mail_ptr->receiver_address)) {
      put_mail_in_mailbox(mail_ptr, get_sorted_mailbox());
    } else {
      put_mail_in_mailbox(mail_ptr, get_undeliverable_mailbox());
    }
  }
  return 0; // Success
}

// Function: udm_sort (Undeliverable Mail Sort)
bool udm_sort(uint32_t param_1_raw, uint32_t param_2_raw) {
  // Cast raw uint32_t to MailAddrPair*
  MailAddrPair* mail1 = (MailAddrPair*)param_1_raw;
  MailAddrPair* mail2 = (MailAddrPair*)param_2_raw;

  // Sort by receiver address (descending), then sender address (descending)
  if (mail1->receiver != mail2->receiver) {
    return mail2->receiver <= mail1->receiver; // Descending order
  }
  return mail2->sender <= mail1->sender; // Descending order
}

// Function: do_list_undeliverable_mail
uint32_t do_list_undeliverable_mail(void) {
  if (is_mailbox_empty(get_undeliverable_mailbox())) {
    return 0xffffffff; // Undeliverable mailbox is empty
  }

  list_t sorted_addr_list;
  list_init(&sorted_addr_list, 0);

  // Iterate through undeliverable mail, extract addresses, and insert into sorted list
  list_node_t current_node = list_head_node(get_undeliverable_mailbox()->mail_list);
  list_node_t end_marker = list_end_marker(get_undeliverable_mailbox()->mail_list);

  while (current_node != end_marker) {
    Mail* mail_ptr = (Mail*)list_node_data(current_node);
    MailAddrPair* addr_pair = (MailAddrPair*)malloc(sizeof(MailAddrPair));
    if (addr_pair == NULL) {
        // Handle error: free resources and terminate
        // (Simplified error handling for brevity, real code needs proper cleanup)
        _terminate(0xfffffff9);
    }
    addr_pair->receiver = mail_ptr->receiver_address;
    addr_pair->sender = mail_ptr->sender_address;
    // Store the pointer to the allocated MailAddrPair
    list_insert_sorted(sorted_addr_list, (void*)addr_pair, udm_sort, 0);
    current_node = list_next_node(current_node);
  }

  // Pop from sorted list and send addresses
  MailAddrPair* current_addr_pair;
  while ((current_addr_pair = (MailAddrPair*)list_pop(sorted_addr_list)) != NULL) {
    int bytes_sent;

    // Send receiver address
    bytes_sent = send_bytes(1, &current_addr_pair->receiver, sizeof(uint16_t));
    if (bytes_sent != sizeof(uint16_t)) {
      free(current_addr_pair);
      _terminate(0xfffffff7); // Send error
    }

    // Send sender address
    bytes_sent = send_bytes(1, &current_addr_pair->sender, sizeof(uint16_t));
    if (bytes_sent != sizeof(uint16_t)) {
      free(current_addr_pair);
      _terminate(0xfffffff7); // Send error
    }
    free(current_addr_pair); // Free the allocated MailAddrPair
  }
  // The sorted list itself might need to be destroyed if list_init allocated it.
  // For simplicity, assuming list_pop and free handle node cleanup.
  return 0; // Success
}

// Function: do_deliver_mail
uint32_t do_deliver_mail(void) {
  Mail* mail_ptr;
  Mailbox* destination_mailbox;

  if (is_mailbox_empty(get_sorted_mailbox())) {
    return 0xffffffff; // Sorted mailbox is empty
  }

  while (true) {
    mail_ptr = (Mail*)pop_mail_from_mailbox(get_sorted_mailbox());
    if (mail_ptr == NULL) break; // No more mail

    destination_mailbox = get_mailbox_for_address(mail_ptr->receiver_address);
    if (destination_mailbox != NULL) {
      put_mail_in_mailbox(mail_ptr, destination_mailbox);
    } else {
        // If receiver address is not found (e.g., deleted), move to undeliverable
        put_mail_in_mailbox(mail_ptr, get_undeliverable_mailbox());
    }
  }
  return 0; // Success
}

// Function: do_return_to_sender
uint32_t do_return_to_sender(void) {
  Mail* mail_ptr;

  if (is_mailbox_empty(get_undeliverable_mailbox())) {
    return 0xffffffff; // Undeliverable mailbox is empty
  }

  while (true) {
    mail_ptr = (Mail*)pop_mail_from_mailbox(get_undeliverable_mailbox());
    if (mail_ptr == NULL) break; // No more mail

    if (address_exists(mail_ptr->sender_address)) {
      // Set receiver address to sender address
      mail_ptr->receiver_address = mail_ptr->sender_address;
      put_mail_in_mailbox(mail_ptr, get_received_mailbox());
    } else {
      put_mail_in_mailbox(mail_ptr, get_lost_mailbox());
    }
  }
  return 0; // Success
}

// Function: hash_mail
void hash_mail(Mail* mail_ptr, uint8_t* hash_buffer) {
  // Assuming mail content starts after sender/receiver addresses (4 bytes offset)
  // and is 0x180 (384) bytes long, split into two parts for hashing.
  // The original 0x80 and 0x100 loops suggest 128 and 256 bytes.
  // total 0x180 bytes. Mail struct has 400 bytes, so it fits.
  int i;
  uint8_t* mail_data_start = (uint8_t*)mail_ptr + sizeof(uint16_t) * 2; // Offset 4

  for (i = 0; i < 0x80; ++i) { // Hash first 128 bytes of content
    hash_buffer[i % 4] += mail_data_start[i];
  }
  for (i = 0; i < 0x100; ++i) { // Hash next 256 bytes of content (total 384 bytes)
    hash_buffer[4 + (i % 4)] += mail_data_start[0x80 + i];
  }
}

// Function: lm_sort (Lost Mail Sort)
bool lm_sort(uint32_t mail1_ptr_raw, uint32_t mail2_ptr_raw) {
  Mail* mail1 = (Mail*)mail1_ptr_raw;
  Mail* mail2 = (Mail*)mail2_ptr_raw;
  uint8_t hash1[8] = {0};
  uint8_t hash2[8] = {0};

  hash_mail(mail1, hash1);
  hash_mail(mail2, hash2);

  // Return true if hash1 is "not less than" hash2 (i.e., hash1 >= hash2)
  return memcmp(hash1, hash2, 8) != -1;
}

// Function: do_list_lost_mail
uint32_t do_list_lost_mail(void) {
  if (is_mailbox_empty(get_lost_mailbox())) {
    return 0xffffffff; // Lost mailbox is empty
  }

  list_t sorted_mail_list;
  list_init(&sorted_mail_list, 0);

  // Iterate through lost mail and insert into sorted list
  list_node_t current_node = list_head_node(get_lost_mailbox()->mail_list);
  list_node_t end_marker = list_end_marker(get_lost_mailbox()->mail_list);

  while (current_node != end_marker) {
    Mail* mail_ptr = (Mail*)list_node_data(current_node);
    list_insert_sorted(sorted_mail_list, (void*)mail_ptr, lm_sort, 0);
    current_node = list_next_node(current_node);
  }

  // Pop from sorted list, hash, and send hashes
  Mail* current_mail_ptr;
  uint8_t mail_hash[8];
  while ((current_mail_ptr = (Mail*)list_pop(sorted_mail_list)) != NULL) {
    memset(mail_hash, 0, sizeof(mail_hash));
    hash_mail(current_mail_ptr, mail_hash);

    int bytes_sent = send_bytes(1, mail_hash, sizeof(mail_hash));
    if (bytes_sent != sizeof(mail_hash)) {
      _terminate(0xfffffff7); // Send error
    }
    // Note: The mail items themselves are not freed here, as they still reside
    // in the original lost_box_inst list until do_destroy_lost_mail is called.
    // If list_pop from sorted_mail_list duplicated the mail, then it should be freed.
    // Assuming it passes pointers to existing mail.
  }
  return 0; // Success
}

// Function: do_destroy_lost_mail
uint32_t do_destroy_lost_mail(void) {
  Mail* mail_ptr;
  list_t lost_mail_list_handle = get_lost_mailbox()->mail_list;

  if (is_mailbox_empty(get_lost_mailbox())) {
    return 0xffffffff; // Lost mailbox is empty
  }

  while ((mail_ptr = (Mail*)pop_mail_from_mailbox(get_lost_mailbox())) != NULL) {
    delete_mail(mail_ptr); // Free the mail item
    // Assuming list_pop also internally calls list_destroy_node for its own node.
    // If not, and list_pop returns the *node*, then list_destroy_node needs to be called.
    // The original code implies list_destroy_node is called after pop:
    // list_destroy_node(local_10,&local_18); (local_10 is list_handle, local_18 is node_ptr)
    // This suggests list_pop might return the node.
    // For simplicity, assuming pop_mail_from_mailbox handles node destruction internally.
  }

  // After popping all elements, the list should be empty.
  if (list_length(lost_mail_list_handle) == 0) {
    return 0; // Success
  }
  return 0xffffffff; // List not empty after destruction (error)
}

// Function: do_buy_postage
uint32_t do_buy_postage(void) {
  uint32_t new_stamp_value = get_new_stamp();

  if (new_stamp_value == 0) {
    return 0xffffffff; // Failed to get a new stamp
  }

  int bytes_sent = send_bytes(1, &new_stamp_value, sizeof(uint32_t)); // Send stamp value (3 bytes implied by original)
  if (bytes_sent != 3) { // Original code checked for 3 bytes sent
    _terminate(0xfffffff7); // Send error
  }
  return 0; // Success
}

// Function: setup
void setup(void) {
  init_stamp_roll();
  mailboxes = bst_init(); // Initialize global BST
  
  // Initialize global Mailbox instances
  undeliverable_box_inst.id = 0;
  undeliverable_box_inst.mail_list = NULL;
  lost_box_inst.id = 1;
  lost_box_inst.mail_list = NULL;
  received_box_inst.id = 2;
  received_box_inst.mail_list = NULL;
  sorted_box_inst.id = 3;
  sorted_box_inst.mail_list = NULL;
}

// Function: process_cmd
uint16_t process_cmd(void) {
  char cmd_buffer[4]; // Commands are 4 bytes long
  int bytes_received;

  bytes_received = recv_bytes(cmd_buffer, sizeof(cmd_buffer));
  if (bytes_received < 0) {
    _terminate(0xfffffff5); // Receive error
  }

  if (memcmp(cmd_buffer, CMD_ADD_ADDRESS, sizeof(cmd_buffer)) == 0) {
    return do_add_address();
  } else if (memcmp(cmd_buffer, CMD_RECEIVE_MAIL, sizeof(cmd_buffer)) == 0) {
    return do_receive_mail();
  } else if (memcmp(cmd_buffer, CMD_LIST_ADDRESSES, sizeof(cmd_buffer)) == 0) {
    return do_list_addresses();
  } else if (memcmp(cmd_buffer, CMD_SORT_MAIL, sizeof(cmd_buffer)) == 0) {
    return do_sort_mail();
  } else if (memcmp(cmd_buffer, CMD_LIST_UNDELIVERABLE_MAIL, sizeof(cmd_buffer)) == 0) {
    return do_list_undeliverable_mail();
  } else if (memcmp(cmd_buffer, CMD_DELIVER_MAIL, sizeof(cmd_buffer)) == 0) {
    return do_deliver_mail();
  } else if (memcmp(cmd_buffer, CMD_RETURN_TO_SENDER, sizeof(cmd_buffer)) == 0) {
    return do_return_to_sender();
  } else if (memcmp(cmd_buffer, CMD_LIST_LOST_MAIL, sizeof(cmd_buffer)) == 0) {
    return do_list_lost_mail();
  } else if (memcmp(cmd_buffer, CMD_DESTROY_LOST_MAIL, sizeof(cmd_buffer)) == 0) {
    return do_destroy_lost_mail();
  } else if (memcmp(cmd_buffer, CMD_BUY_POSTAGE, sizeof(cmd_buffer)) == 0) {
    return do_buy_postage();
  } else if (memcmp(cmd_buffer, CMD_QUIT, sizeof(cmd_buffer)) == 0) {
    return 0xfffe; // Special status for quit
  } else {
    return 0xffff; // Unknown command
  }
}

// Function: send_status
void send_status(uint16_t status_code) {
  int bytes_sent = send_bytes(1, &status_code, sizeof(uint16_t));
  if (bytes_sent != sizeof(uint16_t)) {
    _terminate(0xfffffff7); // Send error
  }
}