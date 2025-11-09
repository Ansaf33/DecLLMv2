#include <stdbool.h> // For bool
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For memset, memcmp
#include <stdint.h>  // For uint16_t, uint32_t, uintptr_t
#include <stdio.h>   // For exit(EXIT_FAILURE)

// --- Type Definitions ---
typedef uint16_t address_t;
typedef uint32_t mail_ptr_t; // Represents a pointer to mail content, cast to uint32_t
typedef uint8_t byte_t;

// Mailbox structure inferred from `*(int *)(param_1 + 4)`
typedef struct mailbox {
    void *mail_list_head; // Head of the list containing mail_ptr_t
} mailbox_t;

// Mail structure inferred from various accesses (e.g., `*(undefined2 *)(iVar3 + 2)`)
// Assuming 0x184 bytes total for mail data, with addresses at the beginning.
typedef struct mail_t {
    address_t sender_address;    // Original `*puVar4` in do_return_to_sender
    address_t recipient_address; // Original `*(undefined2 *)(iVar3 + 2)` in do_sort_mail, `puVar4[1]` in do_return_to_sender
    byte_t content[384];         // 0x184 (388) - sizeof(address_t)*2 (4) = 384 bytes for actual content
} mail_t;

// --- External Function Declarations (Assumed to be provided elsewhere) ---
// These functions are used in the snippet but not defined within it.
// Their signatures are inferred from their usage.

// BST (Binary Search Tree) functions
extern void bst_init(void **tree_root);
extern int bst_search_exists(void *tree_root, address_t address); // Returns 0 if not found, non-zero if found
extern mailbox_t *bst_search_mailbox(void *tree_root, address_t address); // Returns mailbox_t* if found, NULL otherwise
extern void *bst_iter_start(void *tree_root, int order); // Returns pointer to the first element's data (address_t*) or NULL
extern void *bst_iter_next(void *iterator); // Returns pointer to the next element's data (address_t*) or NULL
extern void bst_insert(void **tree_root, address_t address);

// List functions
extern void list_init(void *list_head, int type);
extern int list_length(void *list_head);
extern void list_insert_at_end(void *list_head, mail_ptr_t mail_data);
extern void *list_pop(void *list_head); // Returns pointer to the node that was popped, or NULL
extern void list_destroy_node(void *list_head, void **node_ptr); // Destroys a list node, sets *node_ptr to NULL
extern void *list_head_node(void *list_head); // Returns pointer to the first node, or end_marker if empty
extern void *list_end_marker(void *list_head); // Returns pointer to the end marker
extern void list_insert_sorted(void *list_head, mail_ptr_t mail_data, bool (*compare)(mail_ptr_t, mail_ptr_t), int type);
extern void *list_next_node(void *node_ptr); // Returns pointer to the next node

// I/O functions
// Assuming these are wrappers for system calls. Return bytes read/written, or -1 on error.
extern int recv_bytes(void *buffer, size_t size);
extern int send_bytes(void *buffer, size_t size);

// Stamp-related functions
extern int get_new_stamp(void);
extern int use_stamp(void);
extern void init_stamp_roll(void);

// --- Global Variables (Assumed) ---
void *mailboxes = NULL; // Root of the BST for addresses

// Static mailbox structures
mailbox_t undeliverable_box_struct;
mailbox_t lost_box_struct;
mailbox_t received_box_struct;
mailbox_t sorted_box_struct;

// Pointers to the static mailbox structures
mailbox_t *undeliverable_box = &undeliverable_box_struct;
mailbox_t *lost_box = &lost_box_struct;
mailbox_t *received_box = &received_box_struct;
mailbox_t *sorted_box = &sorted_box_struct;

// Command constants (Assumed these are fixed-size char arrays for `memcmp`)
extern const char CMD_ADD_ADDRESS[];
extern const char CMD_RECEIVE_MAIL[];
extern const char CMD_LIST_ADDRESSES[];
extern const char CMD_SORT_MAIL[];
extern const char CMD_LIST_UNDELIVERABLE_MAIL[];
extern const char CMD_DELIVER_MAIL[];
extern const char CMD_RETURN_TO_SENDER[];
extern const char CMD_LIST_LOST_MAIL[];
extern const char CMD_DESTROY_LOST_MAIL[];
extern const char CMD_BUY_POSTAGE[];
extern const char CMD_QUIT[];

// --- Helper Functions ---
// Replaces the decompiled `_terminate()` calls with a standard exit.
void _terminate(int error_code) {
    exit(error_code);
}

// --- Function Implementations (Fixed from original snippet) ---

// Function: address_exists
bool address_exists(address_t address) {
  return bst_search_exists(mailboxes, address) != 0;
}

// Function: get_mailbox_for_address
mailbox_t *get_mailbox_for_address(address_t address) {
  return bst_search_mailbox(mailboxes, address);
}

// Function: get_undeliverable_mailbox
mailbox_t *get_undeliverable_mailbox(void) {
  return undeliverable_box;
}

// Function: get_lost_mailbox
mailbox_t *get_lost_mailbox(void) {
  return lost_box;
}

// Function: get_received_mailbox
mailbox_t *get_received_mailbox(void) {
  return received_box;
}

// Function: get_sorted_mailbox
mailbox_t *get_sorted_mailbox(void) {
  return sorted_box;
}

// Function: delete_mail
void delete_mail(void *mail_data_ptr) {
  free(mail_data_ptr);
}

// Function: is_mailbox_empty
bool is_mailbox_empty(mailbox_t *mailbox) {
  return mailbox->mail_list_head == NULL || list_length(mailbox->mail_list_head) == 0;
}

// Function: put_mail_in_mailbox
void put_mail_in_mailbox(mail_ptr_t mail_data, mailbox_t *mailbox) {
  if (mailbox->mail_list_head == NULL) {
    void *new_list_head = malloc(sizeof(void*) * 5); // Assuming 0x14 bytes for list head structure
    if (new_list_head == NULL) {
      _terminate(EXIT_FAILURE);
    }
    mailbox->mail_list_head = new_list_head;
    list_init(mailbox->mail_list_head, 0); // Assuming 0 is a default type for mailboxes
  }
  list_insert_at_end(mailbox->mail_list_head, mail_data);
}

// Function: pop_mail_from_mailbox
mail_ptr_t pop_mail_from_mailbox(mailbox_t *mailbox) {
  void *node_ptr = list_pop(mailbox->mail_list_head);
  if (node_ptr == NULL) {
    return 0; // Assuming 0 is NULL/invalid mail_ptr_t
  } else {
    mail_ptr_t mail_data = *(mail_ptr_t *)node_ptr;
    list_destroy_node(mailbox->mail_list_head, &node_ptr);
    return mail_data;
  }
}

// Function: do_receive_mail
uint32_t do_receive_mail(void) {
  // The first `recv_bytes` in original code is unclear, assuming it reads a header.
  // The mail data size (0x184 = 388 bytes) is inferred from `malloc(0x184)`.
  // Assuming a small header is read first, then a stamp is used, then mail content.
  byte_t header_buffer[3]; // Inferred from original stack usage for `local_13`
  if (recv_bytes(header_buffer, sizeof(header_buffer)) < 0) {
    _terminate(EXIT_FAILURE);
  }

  if (use_stamp() == -1) {
    return 0xffffffff; // Error: stamp not usable
  }

  mail_t *mail_data = (mail_t *)malloc(sizeof(mail_t));
  if (mail_data == NULL) {
    _terminate(EXIT_FAILURE);
  }

  // Read the full mail content (excluding what might have been in header_buffer)
  // Assuming `sizeof(mail_t)` is 0x184 (388 bytes)
  if (recv_bytes(mail_data, sizeof(mail_t)) < 0) {
    free(mail_data);
    _terminate(EXIT_FAILURE);
  }

  put_mail_in_mailbox((mail_ptr_t)(uintptr_t)mail_data, get_received_mailbox());
  return 0; // Success
}

// Function: do_list_addresses
uint32_t do_list_addresses(void) {
  void *bst_iterator = bst_iter_start(mailboxes, 1); // 1 for in-order traversal

  if (bst_iterator == NULL) {
    return 0xffffffff; // No addresses or error
  }

  address_t current_address;
  while (bst_iterator != NULL) {
    current_address = *(address_t *)bst_iterator;
    if (current_address > 9) { // Original `9 < local_12`
      if (send_bytes(&current_address, sizeof(address_t)) != sizeof(address_t)) {
        _terminate(EXIT_FAILURE);
      }
    }
    bst_iterator = bst_iter_next(bst_iterator);
  }
  return 0; // Success
}

// Function: do_add_address
uint32_t do_add_address(void) {
  address_t new_address = 0;
  // Assuming `recv_bytes` reads 2 bytes (sizeof(address_t)) into `new_address`
  if (recv_bytes(&new_address, sizeof(address_t)) < 0) {
    _terminate(EXIT_FAILURE);
  }

  if (new_address > 9) { // Original `9 < local_e[0]`
    if (!address_exists(new_address)) {
      bst_insert(&mailboxes, new_address);
      return 0; // Success
    }
  }
  return 0xffffffff; // Address invalid or already exists
}

// Function: do_sort_mail
uint32_t do_sort_mail(void) {
  mailbox_t *received_mb = get_received_mailbox();
  if (is_mailbox_empty(received_mb)) {
    return 0xffffffff;
  }

  mail_ptr_t current_mail_ptr;
  while ((current_mail_ptr = pop_mail_from_mailbox(received_mb)) != 0) {
    mail_t *mail = (mail_t *)(uintptr_t)current_mail_ptr;
    if (address_exists(mail->recipient_address)) {
      put_mail_in_mailbox(current_mail_ptr, get_sorted_mailbox());
    } else {
      put_mail_in_mailbox(current_mail_ptr, get_undeliverable_mailbox());
    }
  }
  return 0; // Success
}

// Function: udm_sort (Undeliverable Mail Sort)
// Compares two 32-bit values, where each value contains two 16-bit addresses.
bool udm_sort(uint32_t val1, uint32_t val2) {
  return val2 <= val1; // Sorts in descending order based on combined address
}

// Function: do_list_undeliverable_mail
uint32_t do_list_undeliverable_mail(void) {
  mailbox_t *undeliverable_mb = get_undeliverable_mailbox();
  if (is_mailbox_empty(undeliverable_mb)) {
    return 0xffffffff;
  }

  void *sorted_list_head = malloc(sizeof(void*) * 5); // Assuming 0x14 bytes for list head
  if (sorted_list_head == NULL) {
      _terminate(EXIT_FAILURE);
  }
  list_init(sorted_list_head, 0);

  void *current_node = list_head_node(undeliverable_mb->mail_list_head);
  void *end_marker = list_end_marker(undeliverable_mb->mail_list_head);

  uint32_t combined_address;
  for (; current_node != end_marker; current_node = list_next_node(current_node)) {
    mail_t *mail = (mail_t *)(uintptr_t)(*(mail_ptr_t *)current_node);
    ((address_t*)&combined_address)[0] = mail->recipient_address;
    ((address_t*)&combined_address)[1] = mail->sender_address;
    list_insert_sorted(sorted_list_head, combined_address, udm_sort, 0);
  }

  void *sorted_node_ptr;
  while ((sorted_node_ptr = list_pop(sorted_list_head)) != NULL) {
    combined_address = *(uint32_t *)sorted_node_ptr;
    address_t recipient_addr = ((address_t*)&combined_address)[0];
    address_t sender_addr = ((address_t*)&combined_address)[1];

    if (send_bytes(&recipient_addr, sizeof(address_t)) != sizeof(address_t)) {
      _terminate(EXIT_FAILURE);
    }
    if (send_bytes(&sender_addr, sizeof(address_t)) != sizeof(address_t)) {
      _terminate(EXIT_FAILURE);
    }
    list_destroy_node(sorted_list_head, &sorted_node_ptr);
  }

  free(sorted_list_head); // Free the temporary list head
  return 0; // Success
}

// Function: do_deliver_mail
uint32_t do_deliver_mail(void) {
  mailbox_t *sorted_mb = get_sorted_mailbox();
  if (is_mailbox_empty(sorted_mb)) {
    return 0xffffffff;
  }

  mail_ptr_t current_mail_ptr;
  while ((current_mail_ptr = pop_mail_from_mailbox(sorted_mb)) != 0) {
    mail_t *mail = (mail_t *)(uintptr_t)current_mail_ptr;
    mailbox_t *target_mb = get_mailbox_for_address(mail->recipient_address);
    // Assuming get_mailbox_for_address returns a valid mailbox_t* for existing addresses.
    // Original code doesn't handle NULL target_mb.
    if (target_mb != NULL) {
        put_mail_in_mailbox(current_mail_ptr, target_mb);
    } else {
        // If recipient address not found, put it in undeliverable?
        // Original code behavior is unclear for this edge case.
        // For now, it implicitly proceeds, which might lead to a crash if target_mb is NULL.
        // Or, it's assumed target_mb is never NULL if address_exists for the sorted mail.
        // Assuming it means the mail is lost.
        put_mail_in_mailbox(current_mail_ptr, get_undeliverable_mailbox());
    }
  }
  return 0; // Success
}

// Function: do_return_to_sender
uint32_t do_return_to_sender(void) {
  mailbox_t *undeliverable_mb = get_undeliverable_mailbox();
  if (is_mailbox_empty(undeliverable_mb)) {
    return 0xffffffff;
  }

  mail_ptr_t current_mail_ptr;
  while ((current_mail_ptr = pop_mail_from_mailbox(undeliverable_mb)) != 0) {
    mail_t *mail = (mail_t *)(uintptr_t)current_mail_ptr;
    if (!address_exists(mail->sender_address)) {
      put_mail_in_mailbox(current_mail_ptr, get_lost_mailbox());
    } else {
      // Swap recipient and sender to return to sender
      mail->recipient_address = mail->sender_address;
      put_mail_in_mailbox(current_mail_ptr, get_received_mailbox());
    }
  }
  return 0; // Success
}

// Function: hash_mail
void hash_mail(mail_ptr_t mail_data_ptr, uint8_t *hash_buffer) {
  char *mail_content = (char *)(uintptr_t)mail_data_ptr;

  // First 128 bytes of content (offset 4 to 4+128-1)
  for (int i = 0; i < 0x80; ++i) {
    hash_buffer[i % 4] += mail_content[i + 4]; // Start hashing from content[4]
  }
  // Next 256 bytes of content (offset 4+128 to 4+128+256-1)
  for (int i = 0; i < 0x100; ++i) {
    hash_buffer[4 + (i % 4)] += mail_content[i + 0x84]; // 0x84 = 4 + 0x80
  }
}

// Function: lm_sort (Lost Mail Sort)
bool lm_sort(mail_ptr_t mail1_ptr, mail_ptr_t mail2_ptr) {
  uint8_t hash1[8] = {0}; // Initialize to zero
  uint8_t hash2[8] = {0}; // Initialize to zero

  hash_mail(mail1_ptr, hash1);
  hash_mail(mail2_ptr, hash2);

  // `memcmp` returns 0 if equal, <0 if first is less, >0 if first is greater.
  // `iVar1 != -1` means it returns true if `iVar1` is 0 or positive.
  // This sorts in descending lexicographical order (greater or equal).
  return memcmp(hash1, hash2, sizeof(hash1)) != -1;
}

// Function: do_list_lost_mail
uint32_t do_list_lost_mail(void) {
  mailbox_t *lost_mb = get_lost_mailbox();
  if (is_mailbox_empty(lost_mb)) {
    return 0xffffffff;
  }

  void *sorted_list_head = malloc(sizeof(void*) * 5); // Assuming 0x14 bytes for list head
  if (sorted_list_head == NULL) {
      _terminate(EXIT_FAILURE);
  }
  list_init(sorted_list_head, 0);

  void *current_node = list_head_node(lost_mb->mail_list_head);
  void *end_marker = list_end_marker(lost_mb->mail_list_head);

  mail_ptr_t current_mail_ptr;
  for (; current_node != end_marker; current_node = list_next_node(current_node)) {
    current_mail_ptr = *(mail_ptr_t *)current_node;
    list_insert_sorted(sorted_list_head, current_mail_ptr, lm_sort, 0);
  }

  uint8_t mail_hash[8];
  void *sorted_node_ptr;
  while ((sorted_node_ptr = list_pop(sorted_list_head)) != NULL) {
    current_mail_ptr = *(mail_ptr_t *)sorted_node_ptr;

    memset(mail_hash, 0, sizeof(mail_hash));
    hash_mail(current_mail_ptr, mail_hash);

    if (send_bytes(mail_hash, sizeof(mail_hash)) != sizeof(mail_hash)) {
      _terminate(EXIT_FAILURE);
    }
    list_destroy_node(sorted_list_head, &sorted_node_ptr);
  }

  free(sorted_list_head); // Free the temporary list head
  return 0; // Success
}

// Function: do_destroy_lost_mail
uint32_t do_destroy_lost_mail(void) {
  mailbox_t *lost_mb = get_lost_mailbox();
  if (is_mailbox_empty(lost_mb)) {
    return 0xffffffff;
  }

  void *node_ptr;
  while ((node_ptr = list_pop(lost_mb->mail_list_head)) != NULL) {
    mail_ptr_t mail_data = *(mail_ptr_t *)node_ptr;
    delete_mail((void *)(uintptr_t)mail_data);
    list_destroy_node(lost_mb->mail_list_head, &node_ptr);
  }

  if (list_length(lost_mb->mail_list_head) == 0) {
    return 0; // Success
  } else {
    return 0xffffffff; // Error: list not empty after destruction attempt
  }
}

// Function: do_buy_postage
uint32_t do_buy_postage(void) {
  int stamp_id = get_new_stamp();
  if (stamp_id == 0) {
    return 0xffffffff; // Error: failed to get new stamp
  }

  byte_t postage_data[3];
  postage_data[0] = 1; // Assuming '1' is a type code for postage
  ((uint16_t*)&postage_data[1])[0] = (uint16_t)stamp_id; // Stamp ID occupies next 2 bytes

  if (send_bytes(postage_data, sizeof(postage_data)) != sizeof(postage_data)) {
    _terminate(EXIT_FAILURE);
  }
  return 0; // Success
}

// Function: setup
void setup(void) {
  init_stamp_roll();
  bst_init(&mailboxes);

  // Initialize the mail_list_head for each static mailbox structure to NULL
  undeliverable_box_struct.mail_list_head = NULL;
  lost_box_struct.mail_list_head = NULL;
  received_box_struct.mail_list_head = NULL;
  sorted_box_struct.mail_list_head = NULL;
}

// Function: process_cmd
uint16_t process_cmd(void) {
  char command_buf[4];
  uint16_t result = 0xffff; // Default error value

  if (recv_bytes(command_buf, sizeof(command_buf)) < 0) {
    _terminate(EXIT_FAILURE);
  }

  if (memcmp(command_buf, CMD_ADD_ADDRESS, sizeof(command_buf)) == 0) {
    result = do_add_address();
  } else if (memcmp(command_buf, CMD_RECEIVE_MAIL, sizeof(command_buf)) == 0) {
    result = do_receive_mail();
  } else if (memcmp(command_buf, CMD_LIST_ADDRESSES, sizeof(command_buf)) == 0) {
    result = do_list_addresses();
  } else if (memcmp(command_buf, CMD_SORT_MAIL, sizeof(command_buf)) == 0) {
    result = do_sort_mail();
  } else if (memcmp(command_buf, CMD_LIST_UNDELIVERABLE_MAIL, sizeof(command_buf)) == 0) {
    result = do_list_undeliverable_mail();
  } else if (memcmp(command_buf, CMD_DELIVER_MAIL, sizeof(command_buf)) == 0) {
    result = do_deliver_mail();
  } else if (memcmp(command_buf, CMD_RETURN_TO_SENDER, sizeof(command_buf)) == 0) {
    result = do_return_to_sender();
  } else if (memcmp(command_buf, CMD_LIST_LOST_MAIL, sizeof(command_buf)) == 0) {
    result = do_list_lost_mail();
  } else if (memcmp(command_buf, CMD_DESTROY_LOST_MAIL, sizeof(command_buf)) == 0) {
    result = do_destroy_lost_mail();
  } else if (memcmp(command_buf, CMD_BUY_POSTAGE, sizeof(command_buf)) == 0) {
    result = do_buy_postage();
  } else if (memcmp(command_buf, CMD_QUIT, sizeof(command_buf)) == 0) {
    result = 0xfffe; // Special quit code
  }
  return result;
}

// Function: send_status
void send_status(uint16_t status_code) {
  if (send_bytes(&status_code, sizeof(uint16_t)) != sizeof(uint16_t)) {
    _terminate(EXIT_FAILURE);
  }
}