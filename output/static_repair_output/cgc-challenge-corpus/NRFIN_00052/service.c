#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h> // For size_t and offsetof
#include <stdint.h> // For uintptr_t

// Forward declarations for structs and functions
typedef struct List List;
typedef struct Instr Instr;

// Dummy List structure for compilation
struct List {
    void *head; // Dummy head, not actually used by stub list functions
    void (*destructor)(void*);
};

// Instruction structure based on read_instr and main usage
struct Instr {
    unsigned int type;    // offset 0
    unsigned int size;    // offset 4
    int data_len;         // offset 8
    void *data_ptr;       // offset 12
};


// Global variables (guessed types based on usage)
static int stats = 0;
static int DAT_00017114 = 0; // payments made / gross revenue
static int DAT_00017110 = 0; // operating costs (partially)
static int DAT_00017104 = 0; // tickets given
static int DAT_0001711a = 0; // active tickets count
static int DAT_00017120 = 0; // outstanding ticket value
static int DAT_0001711c = 0; // ticket payments
static int DAT_00017118 = 0; // tickets removed
static int _DAT_00017108 = 0; // coupons used
static int _DAT_00017124 = 0; // coupon value applied
static int _DAT_0001710c = 0; // unknown commands

static void (*DAT_00017128)(void*) = NULL; // Function pointer, initialized in main
static int* statsp = NULL;         // Pointer to stats, initialized in main

static List *ticketl = NULL; // List of tickets
static List *parkl = NULL;   // List of parked items
static List *couponl = NULL; // List of coupons

// --- Stubs for external functions ---
// Assumed streq behaves like strcmp, returning 0 for equality.
static int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2);
}

// sendall signature derived from usage:
// sendall(int fd, const void* buf, size_t len, unsigned int flags_or_magic)
static int sendall(int fd, const void *buf, size_t len, unsigned int flags_or_magic) {
    (void)fd; (void)buf; (void)flags_or_magic; // Suppress unused warnings
    return (int)len; // Simulate sending, return total bytes sent
}

static void* list_find_node_with_data(List *list, int (*compare_func)(void*, void*), void *data) {
    (void)list; (void)compare_func; (void)data;
    return NULL; // Dummy implementation
}

static void list_insert_at_end(List *list, void *data) {
    (void)list; (void)data;
    // Dummy implementation
}

static void list_remove_node(List *list, void *node) {
    (void)list; (void)node;
    // Dummy implementation
}

static void list_init(List *list, void (*destructor)(void*)) {
    if (list) {
        list->head = NULL;
        list->destructor = destructor;
    }
}

static void list_foreach(List *list, void (*callback)(void*)) {
    (void)list; (void)callback;
    // Dummy implementation
}

static void list_destroy(List *list) {
    if (list) {
        // In a real list, iterate and call destructor for each element if list->destructor is not NULL.
        // For this stub, we just free the list struct itself.
        free(list);
    }
}

static int compare_ticket(void *ticket1, void *ticket2) {
    (void)ticket1; (void)ticket2;
    return 0; // Dummy implementation
}

// fread_until signature derived from usage. 0x24 might be a terminator byte.
static int fread_until(void *buffer, int terminator, size_t max_len, FILE *stream) {
    (void)terminator;
    size_t bytes_read = fread(buffer, 1, max_len -1, stream); // Read up to max_len-1 to leave space for NULL terminator
    ((char*)buffer)[bytes_read] = '\0'; // Ensure null termination
    return (int)bytes_read;
}

static void _terminate(int error_code) {
    fprintf(stderr, "Error %d: Program terminated.\n", error_code);
    exit(error_code);
}
// --- End of Stubs ---

// Function: free_parking
// param_1 is expected to be a pointer to an Instr struct
void free_parking(Instr *param_1) {
  if (param_1) {
    free(param_1->data_ptr); // Free the dynamically allocated data
    free(param_1);           // Free the Instr struct itself
  }
}

// Function: dontfree (changed to a destructor type as used by list_init)
void dontfree(void* data) {
  (void)data; // Do nothing, as the name suggests
}

// Function: compare_coupon
// param_1 is expected to be a pointer to a coupon string
// param_2 is the string to compare against (assuming it's a char*)
int compare_coupon(const char *param_1, const char *param_2) {
  return streq(param_1, param_2);
}

// Function: send_resp
bool send_resp(unsigned int param_1, char *param_2) {
  unsigned int local_data_header; // Represents the first 4 bytes of the message
  char response_buffer[256];      // Buffer for the string part of the message

  // Initialize both parts of the message buffer to zero
  memset(&local_data_header, 0, sizeof(local_data_header) + sizeof(response_buffer));
  
  local_data_header = param_1;
  strcpy(response_buffer, param_2);
  
  // 0x11723 is a magic number used as the 4th argument to sendall.
  size_t total_len = sizeof(local_data_header) + sizeof(response_buffer);
  return sendall(1, &local_data_header, total_len, 0x11723) != (int)total_len;
}

// Function: do_pay
// param_1 is expected to be a pointer to an integer (e.g., amount)
void do_pay(int *param_1) {
  stats++;
  // Assuming param_1 points directly to the integer value
  DAT_00017114 += *param_1;
  DAT_00017110 += ((unsigned int)*param_1 >> 1);
}

// Function: do_add_ticket
// param_1 is expected to be a pointer to a ticket structure/data
void do_add_ticket(void *param_1) {
  DAT_00017104++;
  DAT_00017110++;
  
  if (list_find_node_with_data(ticketl, compare_ticket, param_1) == NULL) {
    list_insert_at_end(ticketl, param_1);
    DAT_0001711a++;
    // Assuming param_1 points to a struct where the value is at offset 4.
    DAT_00017120 += *(int *)((char *)param_1 + 4);
  }
}

// Function: do_pay_ticket
// param_1 is expected to be int* where param_1[1] is the amount
void do_pay_ticket(int *param_1) {
  DAT_0001711c += param_1[1];
  DAT_00017114 += param_1[1];
  DAT_00017120 -= param_1[1];
  
  // Directly use the result of list_find_node_with_data
  if (((int **)list_find_node_with_data(ticketl, compare_ticket, param_1) != NULL) &&
      ((**((int **)list_find_node_with_data(ticketl, compare_ticket, param_1))) == *param_1) &&
      (((unsigned int)(*(((int **)list_find_node_with_data(ticketl, compare_ticket, param_1)))[1]) <= (unsigned int)param_1[1]))) {
    DAT_00017118++;
    DAT_0001711a--;
    list_remove_node(ticketl, ((int **)list_find_node_with_data(ticketl, compare_ticket, param_1)));
  }
}

// Function: do_coupon
// param_1 is expected to be int* where *param_1 is a value, and param_1+1 points to coupon string
void do_coupon(int *param_1) {
  _DAT_00017108++;
  DAT_00017114 += *param_1;
  
  // The `param_1 + 1` is passed as the coupon string to compare_coupon.
  // Cast `compare_coupon` to match `int (*)(void*, void*)` for `list_find_node_with_data`.
  if (((int **)list_find_node_with_data(couponl, (int (*)(void*,void*))compare_coupon, (char*)(param_1 + 1)) != NULL)) {
    _DAT_00017124 -= **((int **)list_find_node_with_data(couponl, (int (*)(void*,void*))compare_coupon, (char*)(param_1 + 1)));
    DAT_00017110 += **((int **)list_find_node_with_data(couponl, (int (*)(void*,void*))compare_coupon, (char*)(param_1 + 1)));
  }
}

// Function: do_unknown
void do_unknown(void) {
  _DAT_0001710c++;
}

// Function: add_to_stats
// param_1 is expected to be a pointer to an Instr struct
void add_to_stats(void *param_1) {
  Instr *instr = (Instr *)param_1; // Cast param_1 to Instr*
  unsigned int command_type = instr->type;

  if (command_type == 3) {
    do_coupon((int *)instr->data_ptr);
  } else if (command_type < 4) {
    if (command_type == 2) {
      do_pay_ticket((int *)instr->data_ptr);
    } else if (command_type < 3) {
      if (command_type == 0) {
        do_pay((int *)instr->data_ptr);
      } else if (command_type == 1) {
        do_add_ticket(instr->data_ptr);
      }
    }
  } else {
    do_unknown();
  }
}

// Function: do_stats
// param_1 is expected to be an array of unsigned int for statistics
void do_stats(unsigned int *param_1) {
  unsigned int local_data_header; // Represents the first 4 bytes of the message
  char stats_buffer[268];         // Buffer for the statistics string

  // Initialize both parts of the message buffer to zero
  memset(&local_data_header, 0, sizeof(local_data_header) + sizeof(stats_buffer));
  
  memset(param_1, 0, 0x2c); // Clear 44 bytes of the stats array
  param_1[10] = 0;          // Placeholder for original `param_1[10] = do_stats;`

  // Allocate new List object for ticketl before use
  ticketl = (List*)malloc(sizeof(List));
  if (!ticketl) _terminate(1);
  list_init(ticketl, dontfree);

  list_foreach(parkl, (void (*)(void*))add_to_stats);
  list_destroy(ticketl);
  ticketl = NULL; // Mark as destroyed

  local_data_header = 2; // Message type 2
  // sprintf format string uses `@i` which is not standard C. Changed to `%d`.
  sprintf(stats_buffer,
          "\nPayments made: %d\nTickets given: %d\nCoupons used: %d\nGross revenue: %d\nOperating costs: %d\nNet revenue: %d\n\n",
          param_1[0], param_1[1], param_1[2], param_1[5], param_1[4], param_1[5] - param_1[4]);
  
  size_t total_len = sizeof(local_data_header) + sizeof(stats_buffer);
  if (sendall(1, &local_data_header, total_len, 0) != (int)total_len) {
    _terminate(0x23);
  }
}

// Function: print_cmd
// param_1 is expected to be a pointer to an Instr struct
void print_cmd(void *param_1) {
  Instr *instr = (Instr *)param_1;
  unsigned int local_data_header; // Represents the first 4 bytes of the message
  char log_buffer[256];           // Buffer for the log message string
  
  // Initialize both parts of the message buffer to zero
  memset(&local_data_header, 0, sizeof(local_data_header) + sizeof(log_buffer));

  local_data_header = 1; // Message type 1
  // sprintf format string uses `@i` which is not standard C. Changed to `%d`.
  sprintf(log_buffer, "LogMessage - Type:%d, Size:%d", instr->type, instr->size);
  
  size_t total_len = sizeof(local_data_header) + sizeof(log_buffer);
  if (sendall(1, &local_data_header, total_len, 0) != (int)total_len) {
    _terminate(0x22);
  }
}

// Function: read_instr
void *read_instr(void) {
  Instr *instr_ptr = (Instr *)calloc(1, sizeof(Instr));
  if (instr_ptr == NULL) {
    return NULL;
  }

  // Read the header (type, size, data_len) which is 12 bytes (offsetof(Instr, data_ptr))
  size_t bytes_read = fread(instr_ptr, 1, offsetof(Instr, data_ptr), stdin);
  if (bytes_read == offsetof(Instr, data_ptr)) {
    if (instr_ptr->data_len != -1) {
      instr_ptr->data_ptr = malloc(instr_ptr->data_len + 1); // +1 for null terminator
      if (instr_ptr->data_ptr != NULL) {
        int actual_read_len = fread_until(instr_ptr->data_ptr, 0x24, instr_ptr->data_len + 1, stdin);
        if (actual_read_len != instr_ptr->data_len) {
          free(instr_ptr->data_ptr);
          free(instr_ptr);
          return NULL;
        }
        return instr_ptr;
      }
    }
    // If data_len is -1 or data_ptr allocation failed
    free(instr_ptr);
  } else {
    // If initial fread failed
    free(instr_ptr);
  }
  return NULL;
}

// Function: main
int main(void) {
  // Allocate memory for global List structures themselves
  parkl = (List*)malloc(sizeof(List));
  if (!parkl) _terminate(1);
  couponl = (List*)malloc(sizeof(List));
  if (!couponl) { free(parkl); _terminate(1); } // Cleanup if couponl allocation fails

  DAT_00017128 = do_stats; // Assign function pointer
  statsp = &stats;         // Assign address of global stats

  list_init(parkl, (void (*)(void*))free_parking); // Cast free_parking to generic destructor
  list_init(couponl, dontfree);

  if (send_resp(0, "INITSTART") != 0) {
    return 1;
  }

  int main_ret_val = 0;
  Instr *instr_ptr = NULL;
  bool running = true;

  while (running) {
    instr_ptr = (Instr *)read_instr();
    if (instr_ptr == NULL) {
      running = false; // Exit loop if no instruction is read
    } else {
      switch (instr_ptr->type) {
        case 0: // Add to parkl
          if (7 < instr_ptr->size) {
            list_insert_at_end(parkl, instr_ptr); // Store the entire Instr struct pointer
          } else {
              if (instr_ptr->data_ptr) free(instr_ptr->data_ptr);
              free(instr_ptr);
          }
          break;
        case 1: // Add to couponl
          if (0x83 < instr_ptr->data_len) {
            ((char*)instr_ptr->data_ptr)[0x83] = 0; // Ensure null termination
            list_insert_at_end(couponl, instr_ptr->data_ptr); // Store the data_ptr (coupon string)
            free(instr_ptr); // Free the Instr struct, data_ptr is now owned by couponl (and not freed by it)
          } else {
              if (instr_ptr->data_ptr) free(instr_ptr->data_ptr);
              free(instr_ptr);
          }
          break;
        case 2: // Do stats
          ((void (*)(void*))DAT_00017128)(statsp); // Call do_stats
          if (instr_ptr->data_ptr) free(instr_ptr->data_ptr);
          free(instr_ptr);
          break;
        case 3: // Print command
          list_foreach(parkl, (void (*)(void*))print_cmd);
          if (instr_ptr->data_ptr) free(instr_ptr->data_ptr);
          free(instr_ptr);
          break;
        case 4: // Terminate
          if (instr_ptr->data_ptr) free(instr_ptr->data_ptr);
          free(instr_ptr);
          running = false; // Set flag to exit loop
          break;
        default: // Handle unknown instruction types by freeing them
          if (instr_ptr->data_ptr) free(instr_ptr->data_ptr);
          free(instr_ptr);
          break;
      }
    }
  }

  // Cleanup block, executed after the loop terminates
  ((void (*)(void*))DAT_00017128)(statsp); // Call do_stats one last time
  list_destroy(parkl);
  parkl = NULL; // Mark as destroyed
  list_destroy(couponl);
  couponl = NULL; // Mark as destroyed

  if (send_resp(3, "done.") != 0) {
    main_ret_val = 1;
  }
  return main_ret_val;
}