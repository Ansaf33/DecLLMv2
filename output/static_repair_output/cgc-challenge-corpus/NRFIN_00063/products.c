#include <stdbool.h>
#include <string.h>
#include <stdlib.h> // For malloc, free
#include <stdio.h>  // For NULL
#include <stdint.h> // For int16_t, int32_t, uint32_t

// Decompiler-specific types replaced
typedef int int32_t;
typedef uint32_t uint32_t;
typedef int16_t int16_t;

// --- External / Undefined functions ---
// These functions are assumed to be defined elsewhere and handle I/O or list operations.
void _terminate(void);
int recv_bytes(void* buffer, size_t count);
int send_bytes(const void* buffer, size_t count);
// recv_until_delim_n: Reads data into buffer up to max_len. Returns number of bytes read.
// The 'flags' argument (0xffffffed in original) is interpreted as a generic flag or 0 for no special delimiter.
int recv_until_delim_n(void* buffer, size_t max_len, int flags);

// List management functions
// A simplified list structure and API based on usage
typedef struct list_node {
    struct list_node *next;
    void *data;
} list_node_t;

typedef struct {
    list_node_t *head;
    list_node_t *tail;
    size_t length;
    void (*free_data)(void*);
    list_node_t *current_iter_node; // For list_head_node/list_next_node iteration
} list_t;

// Function prototypes for list operations
void list_init(list_t* list, void (*free_data_fn)(void*));
void* list_find_node_with_data(list_t* list, bool (*compare_fn)(void*, void*), const void* data_to_compare);
void list_insert_at_end(list_t* list, void* data);
void list_destroy(list_t* list); // Frees all nodes and their data
void list_remove_node(list_t* list, const void* data_to_remove); // Removes node containing data_to_remove, but doesn't free data
void list_destroy_node(list_t* list, const void* data_to_destroy); // Removes node and frees its data
int list_length(list_t* list);
void* list_head_node(list_t* list); // Returns data of the head node, sets internal iterator
void* list_end_marker(list_t* list); // Returns a marker for end of list (e.g., NULL)
void* list_next_node(list_t* list); // Returns data of next node, advances internal iterator

void load_inventory(list_t* list);

// --- Product structure and price calculation function pointer ---
typedef long double (*price_calc_fn)(void*); // Function pointer for price calculation

// Placeholder price calculation functions (actual implementations would be defined elsewhere)
long double default_onsale_calc(void* product_ptr) {
    // Example: returns product->price
    // This function would typically cast product_ptr to product_t* and access members.
    // For compilation, we need to declare product_t first.
    // For now, a placeholder that returns a dummy value.
    (void)product_ptr; // Suppress unused parameter warning
    return 100.0L;
}
long double onsale_calc_1(void* product_ptr) { (void)product_ptr; return 1.0L; }
long double onsale_calc_2(void* product_ptr) { (void)product_ptr; return 2.0L; }
// Add more placeholder functions up to 100 if needed

// Main product structure based on offsets in original code
typedef struct product {
    char barcode[8];        // Offset 0x0, used for comparison
    char description[128];  // Offset 0x8 (8 + 128 = 136 = 0x88)
    int quantity;           // Offset 0x88
    int price;              // Offset 0x8c
    price_calc_fn calculate_price; // Offset 0x90 (size of func ptr varies, assume 4 bytes for 32-bit arch, 8 for 64-bit)
    int update_serial_val;  // Offset 0x94
} product_t; // Total size 0x98 (152 bytes) if func ptr is 4 bytes

// --- Global variables ---
list_t inv; // Global inventory list
int update_serial = 0; // Global serial counter
int _DAT_4347c000 = 0; // Placeholder for a global constant/base address

// Array of price calculation function pointers
// Assuming 100 possible onsale functions, with default at index 0
price_calc_fn onsale_fn[100] = {
    default_onsale_calc,
    onsale_calc_1,
    onsale_calc_2,
    // ... initialize other functions up to index 99 ...
};

// Command strings (assuming 4-byte commands for memcmp)
const char CMD_BUY[] = "BUY\0";
const char CMD_CHECK[] = "CHK\0";
const char CMD_ADD[] = "ADD\0";
const char CMD_RM[] = "RM\0";
const char CMD_UPDATE[] = "UPD\0";
const char CMD_ONSALE[] = "ONS\0";
const char CMD_NOSALE[] = "NOS\0";
const char CMD_LIST[] = "LST\0";
const char CMD_QUIT[] = "QUIT"; // This one is 4 chars, so it fits
const char BUY_MORE[] = "BUYM";
const char BUY_TERM[] = "BUYT";
const char DESC_TERM = '\n'; // Single character delimiter

// Function: get_next_update_serial
int get_next_update_serial(void) {
  return _DAT_4347c000 + update_serial++;
}

// Function: prod_has_bc
bool prod_has_bc(void *param_1, void *param_2) {
  return memcmp(param_1, param_2, 8) == 0;
}

// Function: get_product_by_barcode
product_t* get_product_by_barcode(const void* barcode_data) {
  // list_find_node_with_data returns the data pointer (product_t*) if found, otherwise NULL.
  return (product_t*)list_find_node_with_data(&inv, prod_has_bc, barcode_data);
}

// Function: do_buy
int32_t do_buy(void) {
  char command_buf[4];
  char barcode_buf[8];
  product_t *product_found;
  product_t *new_item;
  long double total_price = 0.0L;
  int bytes_received;

  // Initialize a temporary list for this buy operation (local_40 in original)
  list_t buy_list;
  list_init(&buy_list, free); // Assuming 'free' is appropriate for product_t*

  // First receive the command (BUY_MORE or BUY_TERM)
  bytes_received = recv_bytes(command_buf, sizeof(command_buf));
  if (bytes_received < 0) {
    _terminate();
  }

  while (memcmp(command_buf, BUY_MORE, sizeof(command_buf)) == 0) {
    // Receive barcode
    bytes_received = recv_bytes(barcode_buf, sizeof(barcode_buf));
    if (bytes_received < 0) {
      _terminate();
    }

    product_found = get_product_by_barcode(barcode_buf);
    if (product_found == NULL) {
      list_destroy(&buy_list);
      return -1; // 0xffffffff
    }

    new_item = (product_t*)malloc(sizeof(product_t));
    if (new_item == NULL) {
      _terminate();
    }
    memcpy(new_item, product_found, sizeof(product_t));
    list_insert_at_end(&buy_list, new_item);

    total_price += new_item->calculate_price(new_item);

    // Receive next command
    bytes_received = recv_bytes(command_buf, sizeof(command_buf));
    if (bytes_received < 0) {
      _terminate();
    }
  }

  if (memcmp(command_buf, BUY_TERM, sizeof(command_buf)) == 0) {
    bytes_received = send_bytes(&total_price, sizeof(total_price));
    if (bytes_received != sizeof(total_price)) {
      _terminate();
    }
    list_destroy(&buy_list);
    return 0;
  } else {
    list_destroy(&buy_list);
    return -1; // 0xffffffff
  }
}

// Function: do_check
int32_t do_check(void) {
  char barcode_buf[8];
  product_t *product_found;
  int bytes_received;
  float calculated_price_float; // For sending float price, as per original

  bytes_received = recv_bytes(barcode_buf, sizeof(barcode_buf));
  if (bytes_received < 0) {
    _terminate();
  }

  product_found = get_product_by_barcode(barcode_buf);
  if (product_found == NULL) {
    return -1; // 0xffffffff
  } else {
    bytes_received = send_bytes(&product_found->quantity, sizeof(product_found->quantity));
    if (bytes_received != sizeof(product_found->quantity)) {
      _terminate();
    }

    calculated_price_float = (float)product_found->calculate_price(product_found);
    bytes_received = send_bytes(&calculated_price_float, sizeof(calculated_price_float));
    if (bytes_received != sizeof(calculated_price_float)) {
      _terminate();
    }

    size_t desc_len = strlen(product_found->description);
    if (desc_len != 0) {
      bytes_received = send_bytes(product_found->description, desc_len);
      if (bytes_received != desc_len) {
        _terminate();
      }
    }

    bytes_received = send_bytes(&DESC_TERM, sizeof(DESC_TERM));
    if (bytes_received != sizeof(DESC_TERM)) {
      _terminate();
    }
    return 0;
  }
}

// Function: do_add
int32_t do_add(void) {
  product_t *new_product;
  char barcode_buf[8];
  int bytes_received;
  int description_len;

  new_product = (product_t*)malloc(sizeof(product_t));
  if (new_product == NULL) {
    _terminate();
  }

  // Set default onsale function and initialize update serial
  new_product->calculate_price = onsale_fn[0];
  new_product->update_serial_val = 0;

  bytes_received = recv_bytes(barcode_buf, sizeof(barcode_buf));
  if (bytes_received < 0) {
    _terminate();
  }
  memcpy(new_product->barcode, barcode_buf, sizeof(new_product->barcode));

  // Check if product with this barcode already exists
  if (get_product_by_barcode(new_product->barcode) != NULL) {
    free(new_product);
    return -1; // 0xffffffff
  } else {
    bytes_received = recv_bytes(&new_product->quantity, sizeof(new_product->quantity));
    if (bytes_received < 0) {
      _terminate();
    }

    bytes_received = recv_bytes(&new_product->price, sizeof(new_product->price));
    if (bytes_received < 0) {
      _terminate();
    }

    // Recv description until delimiter or max length
    // Original: max_len 0x98 (152), flags 0xffffffed.
    // For product description, buffer is new_product->description, max_len is 128 (sizeof - 1 for null).
    description_len = recv_until_delim_n(new_product->description, sizeof(new_product->description) - 1, 0);
    if (description_len < 1) { // Original checks < 1, implies at least 1 byte must be received
      _terminate();
    }
    new_product->description[description_len] = '\0'; // Null-terminate received description

    list_insert_at_end(&inv, new_product);
    return 0;
  }
}

// Function: do_rm
int32_t do_rm(void) {
  char barcode_buf[8];
  product_t *product_to_remove;
  int bytes_received;

  bytes_received = recv_bytes(barcode_buf, sizeof(barcode_buf));
  if (bytes_received < 0) {
    _terminate();
  }

  product_to_remove = get_product_by_barcode(barcode_buf);
  if (product_to_remove == NULL) {
    return -1; // 0xffffffff
  } else {
    list_remove_node(&inv, product_to_remove); // Remove the node from the list
    list_destroy_node(&inv, product_to_remove); // Free the product data and the node itself
    return 0;
  }
}

// Function: do_update
int32_t do_update(void) {
  char barcode_buf[8];
  product_t *product_to_update;
  char description_buf[128]; // Max description size
  int bytes_received;
  int description_len;

  // Initialize description buffer to zeros (local_a0 in original)
  memset(description_buf, 0, sizeof(description_buf));

  bytes_received = recv_bytes(barcode_buf, sizeof(barcode_buf));
  if (bytes_received < 0) {
    _terminate();
  }

  product_to_update = get_product_by_barcode(barcode_buf);
  if (product_to_update == NULL) {
    return -1; // 0xffffffff
  } else {
    bytes_received = recv_bytes(&product_to_update->quantity, sizeof(product_to_update->quantity));
    if (bytes_received < 0) {
      _terminate();
    }

    bytes_received = recv_bytes(&product_to_update->price, sizeof(product_to_update->price));
    if (bytes_received < 0) {
      _terminate();
    }

    // Recv description
    description_len = recv_until_delim_n(description_buf, sizeof(description_buf) - 1, 0);
    if (description_len < 1) {
      _terminate();
    }
    description_buf[description_len] = '\0'; // Null-terminate

    // Copy new description to product
    memcpy(product_to_update->description, description_buf, sizeof(product_to_update->description));

    // Update serial
    product_to_update->update_serial_val = get_next_update_serial();
    return 0;
  }
}

// Function: do_onsale
int32_t do_onsale(void) {
  char barcode_buf[8];
  product_t *product_to_update;
  uint32_t onsale_idx;
  int bytes_received;

  bytes_received = recv_bytes(barcode_buf, sizeof(barcode_buf));
  if (bytes_received < 0) {
    _terminate();
  }

  product_to_update = get_product_by_barcode(barcode_buf);
  if (product_to_update == NULL) {
    return -1; // 0xffffffff
  } else {
    bytes_received = recv_bytes(&onsale_idx, sizeof(onsale_idx));
    if (bytes_received < 0) {
      _terminate();
    }

    if (onsale_idx < 100) { // Check bounds for onsale_fn array
      product_to_update->calculate_price = onsale_fn[onsale_idx];
      return 0;
    } else {
      return -1; // 0xffffffff
    }
  }
}

// Function: do_nosale
int32_t do_nosale(void) {
  char barcode_buf[8];
  product_t *product_to_update;
  int bytes_received;

  bytes_received = recv_bytes(barcode_buf, sizeof(barcode_buf));
  if (bytes_received < 0) {
    _terminate();
  }

  product_to_update = get_product_by_barcode(barcode_buf);
  if (product_to_update == NULL) {
    return -1; // 0xffffffff
  } else {
    product_to_update->calculate_price = onsale_fn[0]; // Reset to default
    return 0;
  }
}

// Function: do_list
int32_t do_list(void) {
  uint32_t list_flags; // Flags for what product info to send
  product_t *current_product;
  int bytes_received;
  float calculated_price_float;

  bytes_received = recv_bytes(&list_flags, sizeof(list_flags));
  if (bytes_received < 0) {
    _terminate();
  }

  if (list_length(&inv) == 0) {
    return -1; // 0xffffffff
  } else {
    current_product = (product_t*)list_head_node(&inv); // Get data of the first product
    void* end_marker = list_end_marker(&inv); // Marker for end of list

    while (current_product != NULL && current_product != end_marker) {
      // Send barcode (always sent as per original interpretation)
      bytes_received = send_bytes(current_product->barcode, sizeof(current_product->barcode));
      if (bytes_received != sizeof(current_product->barcode)) {
        _terminate();
      }

      if ((list_flags & 1) == 0) { // If bit 0 is NOT set, send quantity
        bytes_received = send_bytes(&current_product->quantity, sizeof(current_product->quantity));
        if (bytes_received != sizeof(current_product->quantity)) {
          _terminate();
        }
      }

      if ((list_flags & 0x100) != 0) { // If bit 8 is set, send raw price
        bytes_received = send_bytes(&current_product->price, sizeof(current_product->price));
        if (bytes_received != sizeof(current_product->price)) {
          _terminate();
        }
      }

      if ((list_flags & 0x10000) == 0) { // If bit 16 is NOT set, send calculated price
        calculated_price_float = (float)current_product->calculate_price(current_product);
        bytes_received = send_bytes(&calculated_price_float, sizeof(calculated_price_float));
        if (bytes_received != sizeof(calculated_price_float)) {
          _terminate();
        }
      }

      if ((list_flags & 0x1000000) != 0) { // If bit 24 is set, send description
        size_t desc_len = strlen(current_product->description);
        if (desc_len != 0) {
          bytes_received = send_bytes(current_product->description, desc_len);
          if (bytes_received != desc_len) {
            _terminate();
          }
        }
        bytes_received = send_bytes(&DESC_TERM, sizeof(DESC_TERM));
        if (bytes_received != sizeof(DESC_TERM)) {
          _terminate();
        }
      }
      current_product = (product_t*)list_next_node(&inv); // Get data of the next product
    }
    return 0;
  }
}

// Function: setup
void setup(void) {
  list_init(&inv, free); // Initialize global inventory list with a free function
  load_inventory(&inv);
  return;
}

// Function: process_cmd
int16_t process_cmd(void) {
  char command_buf[4];
  int bytes_received;

  bytes_received = recv_bytes(command_buf, sizeof(command_buf));
  if (bytes_received < 0) {
    _terminate();
  }

  if (memcmp(command_buf, CMD_BUY, sizeof(command_buf)) == 0) {
    return do_buy();
  } else if (memcmp(command_buf, CMD_CHECK, sizeof(command_buf)) == 0) {
    return do_check();
  } else if (memcmp(command_buf, CMD_ADD, sizeof(command_buf)) == 0) {
    return do_add();
  } else if (memcmp(command_buf, CMD_RM, sizeof(command_buf)) == 0) {
    return do_rm();
  } else if (memcmp(command_buf, CMD_UPDATE, sizeof(command_buf)) == 0) {
    return do_update();
  } else if (memcmp(command_buf, CMD_ONSALE, sizeof(command_buf)) == 0) {
    return do_onsale();
  } else if (memcmp(command_buf, CMD_NOSALE, sizeof(command_buf)) == 0) {
    return do_nosale();
  } else if (memcmp(command_buf, CMD_LIST, sizeof(command_buf)) == 0) {
    return do_list();
  } else if (memcmp(command_buf, CMD_QUIT, sizeof(command_buf)) == 0) {
    return 0xfffe; // Special return for quit (as per original undefined2 return)
  } else {
    return 0xffff; // Unknown command (as per original undefined2 return)
  }
}