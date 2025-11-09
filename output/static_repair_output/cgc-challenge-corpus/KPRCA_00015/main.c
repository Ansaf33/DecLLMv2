#include <unistd.h>     // For read, write, exit
#include <sys/types.h>  // For ssize_t, size_t
#include <string.h>     // For strncpy, memset
#include <stdlib.h>     // For malloc, realloc, free, exit
#include <stdbool.h>    // For bool
#include <stdint.h>     // For uint8_t, uint16_t, uint32_t, uintptr_t

// Decompiler artifacts for types
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef unsigned int uint;

// Global variables (from snippet, making reasonable type guesses and initializations)
uint32_t pkthdr = 0;
uint32_t DAT_00016024 = 0;
uint32_t DAT_00016028 = 0; // Used as `._1_1_`, `(char)`, `._3_1_`
uint32_t uRam0001602c = 0; // Used in CONCAT31
int *card_info_list = NULL; // Head of a linked list of card info
uint32_t transactions_length = 0; // Max capacity of transactions_array
uintptr_t transactions_array = 0; // Base address of transactions array (uintptr_t to store pointer)
uint32_t transactions_idx = 0; // Next available index in transactions array
int next_1 = 0; // Next card ID
uint next_0 = 0; // Next auth code
int *transaction = NULL; // Current transaction pointer
uint32_t ram0x0001602b = 0; // Used in main, seems like a transaction ID or similar.

// File descriptor for network operations (assuming a single one for simplicity)
static int g_network_fd = 1; // Default to stdout for send, stdin for read. Can be changed to a socket.

// Helper to write all bytes
// Returns 0 on success, 1 on error (or partial write)
static int _write_all(int fd, const void *buf, size_t count) {
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t res = write(fd, (const char *)buf + bytes_written, count - bytes_written);
        if (res <= 0) { // Error or EOF
            return 1;
        }
        bytes_written += res;
    }
    return 0;
}

// Helper to read all bytes
// Returns 0 on success, 1 on error (or partial read/EOF before full count)
static int _read_all(int fd, void *buf, size_t count) {
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t res = read(fd, (char *)buf + bytes_read, count - bytes_read);
        if (res <= 0) { // Error or EOF
            return 1;
        }
        bytes_read += res;
    }
    return 0;
}

// Helper for 'allocate' (likely a custom malloc wrapper)
// Returns 0 on success, 1 on error. Stores pointer in `out_ptr`.
static int allocate(size_t size, int flags, void **out_ptr) {
    *out_ptr = malloc(size);
    return (*out_ptr == NULL);
}

// CONCAT31 and CONCAT11 are decompiler artifacts for combining parts of registers/variables.
// Assuming little-endian system for byte ordering.
// CONCAT31((undefined3)uRam0001602c,DAT_00016028._3_1_)
// Interpreted as: (MSB, LSB_3_bytes)
#define CONCAT31_GLUE(msb, lsb3) (((uint32_t)(msb) << 24) | ((uint32_t)(lsb3) & 0x00FFFFFFU))

// CONCAT11(DAT_00016028._1_1_,(char)DAT_00016028)
// Interpreted as: (high_byte, low_byte)
#define CONCAT11_GLUE(hi, lo) (((uint16_t)(hi) << 8) | ((uint16_t)(lo) & 0xFFU))

// Function: send
// Reinterpreting arguments based on observed call patterns:
// __fd_as_buf_ptr is the buffer pointer, __buf_as_size is the size.
// __n_ignored and __flags_ignored are discarded.
// Returns 0 on success, 1 on error.
ssize_t send(int __fd_as_buf_ptr, void *__buf_as_size, size_t __n_ignored, int __flags_ignored) {
    return _write_all(g_network_fd, (void *)(intptr_t)__fd_as_buf_ptr, (size_t)(intptr_t)__buf_as_size);
}

// Function: read_fully
// Reinterpreting arguments: param_1_as_buf_ptr is the buffer pointer, param_2_as_size is the size.
// Returns 0 on success, 1 on error.
undefined4 read_fully(int param_1_as_buf_ptr, uint param_2_as_size) {
    return _read_all(g_network_fd, (void *)(intptr_t)param_1_as_buf_ptr, param_2_as_size);
}

// Function: send_error
void send_error(undefined param_1, char *param_2) {
    struct {
        uint32_t pkthdr_val;
        uint32_t dat_val;
        uint16_t dat_part;
        uint32_t concat_val;
        uint8_t param1_val;
    } header_part;
    
    header_part.pkthdr_val = pkthdr;
    header_part.dat_val = DAT_00016024;
    header_part.dat_part = (uint16_t)DAT_00016028; // Lower 2 bytes of DAT_00016028
    header_part.concat_val = CONCAT31_GLUE((DAT_00016028 >> 24) & 0xFF, uRam0001602c);
    header_part.param1_val = param_1;
    
    send((int)(intptr_t)&header_part, (void *)(intptr_t)0xf, 0, 0); // Send 15 bytes header
    
    uint32_t msg_len_val = 0x40; // 64
    char acStack_5b[64];
    strncpy(acStack_5b, param_2, sizeof(acStack_5b));
    acStack_5b[sizeof(acStack_5b) - 1] = '\0'; // Ensure null termination
    
    send((int)(intptr_t)&msg_len_val, (void *)(intptr_t)sizeof(msg_len_val), 0, 0); // Send message length (4 bytes)
    send((int)(intptr_t)acStack_5b, (void *)(intptr_t)strnlen(acStack_5b, sizeof(acStack_5b)), 0, 0); // Send message
    
    if (transaction != NULL) {
        *(undefined *)((char *)transaction + 10) = param_1; // Update status byte in transaction
    }
    exit(0);
}

// Function: enlarge_transactions_array
undefined4 enlarge_transactions_array(uint param_1) {
    if (transactions_length < param_1) {
        if (param_1 < 0x55556) { // 350006
            void *new_array_ptr;
            if (allocate(0x800000, 0, &new_array_ptr) == 0) { // Allocate 8MB
                transactions_array = (uintptr_t)new_array_ptr;
                transactions_length = 0x55555; // Set new capacity
                return 0; // Success
            }
        }
        return 1; // Error (param_1 too large or allocation failed)
    }
    return 0; // Already large enough
}

// Function: next_card_id
int next_card_id(void) {
    return next_1++;
}

// Function: next_auth_code
uint next_auth_code(void) {
    uint current_next_0 = next_0;
    next_0 = (next_0 << 3 ^ next_0 >> 0x11) * 0x7a69;
    return current_next_0;
}

// Function: lookup_card
int *lookup_card(int param_1) {
    int *current_card = card_info_list;
    while (current_card != NULL) {
        if (param_1 == *current_card) { // Compare card ID
            return current_card;
        }
        current_card = (int *)current_card[3]; // Assuming card_info_list is a linked list where current_card[3] is the next pointer
    }
    return NULL;
}

// Function: handle_issue
bool handle_issue(void) {
    undefined4 *new_card_info = (undefined4 *)malloc(0x10); // 16 bytes for card_info
    
    if (new_card_info == NULL) {
        send_error(3, "Failed to allocate memory.");
        return true; // Error
    }
    
    *new_card_info = next_card_id();
    new_card_info[1] = next_auth_code();
    new_card_info[2] = *(undefined4 *)((char *)transaction + 5 * sizeof(undefined4)); // transaction[5]
    new_card_info[3] = (undefined4)(intptr_t)card_info_list; // Link to existing list
    card_info_list = (int *)new_card_info;
    
    struct {
        uint32_t card_id;
        uint32_t auth_code;
        uint8_t flags_0;
        uint8_t flags_1;
        uint8_t flags_2;
        uint32_t transaction_id;
    } issue_response;
    
    memset(&issue_response, 0, sizeof(issue_response));
    issue_response.card_id = *new_card_info;
    issue_response.auth_code = new_card_info[1];
    issue_response.flags_1 = 4; // Specific magic number
    issue_response.transaction_id = *(uint32_t *)((char *)transaction + 3 * sizeof(undefined4)); // transaction[3]
    
    *(uint32_t *)((char *)transaction + 4 * sizeof(undefined4)) = *new_card_info; // transaction[4] = card_id
    *(uint32_t *)transaction = new_card_info[1]; // transaction[0] = auth_code
    
    send((int)(intptr_t)&issue_response, (void *)(intptr_t)0xf, 0, 0); // Send 15 bytes response
    
    return false; // Success
}

// Function: read_data
undefined4 read_data(int param_1) {
    size_t data_size;
    
    switch ((DAT_00016028 >> 8) & 0xFF) { // DAT_00016028._1_1_
        case 0: data_size = 0xe; break;
        case 1: data_size = 10; break;
        case 2: data_size = 0; break;
        case 3:
        case 4: data_size = 4; break;
        case 5: data_size = 8; break;
        default: return 1; // Error
    }
    
    void *data_buf = NULL;
    *(void **)((char *)param_1 + 0x14) = NULL; // Initialize transaction->data_ptr to NULL
    
    bool error_flag = false;
    
    if (data_size != 0) {
        data_buf = malloc(data_size);
        *(void **)((char *)param_1 + 0x14) = data_buf;
        
        if (data_buf == NULL || read_fully((int)(intptr_t)data_buf, data_size) != 0) {
            error_flag = true;
        }
    }
    
    if (!error_flag) {
        uint additional_size = 0;
        if (((DAT_00016028 >> 8) & 0xFF) == 0) {
            additional_size = (uint)*(uint8_t *)((char *)data_buf + 0xd);
        } else if (((DAT_00016028 >> 8) & 0xFF) == 1) {
            additional_size = (uint)*(uint8_t *)((char *)data_buf + 9);
        }
        
        if (additional_size != 0) {
            void *realloc_buf = realloc(data_buf, data_size + additional_size);
            if (realloc_buf != NULL) {
                data_buf = realloc_buf;
                *(void **)((char *)param_1 + 0x14) = data_buf; // Update pointer in transaction struct
                if (read_fully((int)(intptr_t)((char *)data_buf + data_size), additional_size) != 0) {
                    error_flag = true;
                }
            } else {
                error_flag = true;
            }
        }
    }
    
    if (error_flag) {
        free(*(void **)((char *)param_1 + 0x14));
        *(void **)((char *)param_1 + 0x14) = NULL;
        return 1; // Error
    }
    
    return 0; // Success
}

// Function: new_transaction
undefined4 *new_transaction(void) {
    if (enlarge_transactions_array(transactions_idx + 10) != 0) {
        return NULL; // Error
    }
    
    undefined4 *new_tx = (undefined4 *)((char *)transactions_array + transactions_idx * 0x18);
    transactions_idx++;
    
    memset(new_tx, 0, 0x18); // Size of transaction struct is 24 bytes (0x18)
    
    *(uint32_t *)((char *)new_tx + 12) = (uint32_t)(intptr_t)new_tx; // new_tx[3] = pointer to itself
    
    *(uint8_t *)((char *)new_tx + 8) = (DAT_00016028 >> 8) & 0xFF; // new_tx + 8 (op_code)
    *(uint8_t *)((char *)new_tx + 9) = (uint8_t)DAT_00016028;     // new_tx + 9 (pkt_type)
    *(uint8_t *)((char *)new_tx + 10) = 0;                        // new_tx + 10 (status)
    
    *(uint32_t *)((char *)new_tx + 16) = pkthdr; // new_tx[4] (auth_code related?)
    *(uint32_t *)new_tx = DAT_00016024;          // new_tx[0] (card_id related?)
    
    return new_tx;
}

// Function: lookup_transaction
uint lookup_transaction(uint param_1) {
    // param_1 is treated as an address.
    // Check if param_1 is within the bounds of transactions_array.
    if ((param_1 < transactions_array) || (transactions_array + transactions_idx * 0x18 <= param_1)) {
        return 0; // Not found (or invalid address)
    }
    return param_1;
}

// Function: handle_purchase
bool handle_purchase(void) {
    int *data_ptr = *(int **)((char *)transaction + 0x14); // transaction[5]
    int *card = lookup_card(*(int *)((char *)transaction + 0x10)); // transaction[4]
    
    // Assuming card[2] is balance, data_ptr[1] is purchase amount
    if (*(uint *)((char *)card + 8) < *(uint *)((char *)data_ptr + 4)) {
        send_error(5, "Insuffient funds for purchase.");
        return true; // Error
    }
    
    *(int *)((char *)card + 8) -= *(int *)((char *)data_ptr + 4); // card[2] -= data_ptr[1]
    
    *(uint8_t *)((char *)transaction + 9) = 2;  // transaction + 9 (pkt_type) = 2 (completed)
    *(uint8_t *)((char *)transaction + 10) = 0; // transaction + 10 (status) = 0 (success)
    *(uint32_t *)((char *)transaction + 16) = *(uint32_t *)((char *)data_ptr + 8); // transaction[4] = data_ptr[2]
    
    struct {
        uint32_t pkthdr_val;
        uint32_t dat_val;
        uint16_t dat_part;
        uint32_t concat_val;
        uint8_t zero_byte;
    } response_header;
    
    response_header.pkthdr_val = pkthdr;
    response_header.dat_val = DAT_00016024;
    response_header.dat_part = (uint16_t)DAT_00016028;
    response_header.concat_val = CONCAT31_GLUE((DAT_00016028 >> 24) & 0xFF, uRam0001602c);
    response_header.zero_byte = 0;
    
    send((int)(intptr_t)&response_header, (void *)(intptr_t)0xf, 0, 0);
    
    return false; // Success
}

// Function: handle_recharge
bool handle_recharge(void) {
    uint *data_ptr = *(uint **)((char *)transaction + 0x14); // transaction[5]
    int *card = lookup_card(*(int *)((char *)transaction + 0x10)); // transaction[4]
    
    // Assuming card[2] is balance, data_ptr[0] is recharge amount
    if (~(*(uint *)((char *)card + 8)) < *data_ptr) { // Check for overflow
        send_error(6, "Unable to recharge card. Balance near max.");
        return true; // Error
    }
    
    *(uint *)((char *)card + 8) += *data_ptr; // card[2] += data_ptr[0]
    
    *(uint8_t *)((char *)transaction + 9) = 2;  // transaction + 9 (pkt_type) = 2 (completed)
    *(uint8_t *)((char *)transaction + 10) = 0; // transaction + 10 (status) = 0 (success)
    *(uint32_t *)((char *)transaction + 16) = data_ptr[1]; // transaction[4] = data_ptr[1]
    
    struct {
        uint32_t pkthdr_val;
        uint32_t dat_val;
        uint16_t dat_part;
        uint32_t concat_val;
        uint8_t zero_byte;
    } response_header;
    
    response_header.pkthdr_val = pkthdr;
    response_header.dat_val = DAT_00016024;
    response_header.dat_part = (uint16_t)DAT_00016028;
    response_header.concat_val = CONCAT31_GLUE((DAT_00016028 >> 24) & 0xFF, uRam0001602c);
    response_header.zero_byte = 0;
    
    send((int)(intptr_t)&response_header, (void *)(intptr_t)0xf, 0, 0);
    
    return false; // Success
}

// Function: handle_balance
bool handle_balance(void) {
    int *card = lookup_card(*(int *)((char *)transaction + 0x10)); // transaction[4]
    
    void *balance_ptr = malloc(sizeof(uint32_t)); // Allocate 4 bytes for balance
    *(void **)((char *)transaction + 0x14) = balance_ptr; // transaction[5] = balance_ptr
    
    if (balance_ptr == NULL) {
        send_error(3, "Failed to allocate memory.");
        return true; // Error
    }
    
    struct {
        uint32_t pkthdr_val;
        uint32_t dat_val;
        uint16_t dat_part;
        uint32_t concat_val;
        uint8_t zero_byte;
    } response_header;
    
    response_header.pkthdr_val = pkthdr;
    response_header.dat_val = DAT_00016024;
    response_header.dat_part = (uint16_t)DAT_00016028;
    response_header.concat_val = CONCAT31_GLUE((DAT_00016028 >> 24) & 0xFF, uRam0001602c);
    response_header.zero_byte = 0;
    
    send((int)(intptr_t)&response_header, (void *)(intptr_t)0xf, 0, 0);
    
    memset(balance_ptr, 0, sizeof(uint32_t));
    *(uint32_t *)balance_ptr = *(uint32_t *)((char *)card + 8); // card[2] (balance)
    
    send((int)(intptr_t)balance_ptr, (void *)(intptr_t)sizeof(uint32_t), 0, 0);
    
    *(uint8_t *)((char *)transaction + 9) = 2;  // transaction + 9 (pkt_type) = 2 (completed)
    *(uint8_t *)((char *)transaction + 10) = 0; // transaction + 10 (status) = 0 (success)
    
    return false; // Success
}

// Function: handle_history
undefined4 handle_history(void) {
    uint *requested_count_ptr = *(uint **)((char *)transaction + 0x14); // transaction[5]
    uint matching_tx_count = 0;
    
    for (uint i = 0; i < transactions_idx; ++i) {
        if ((*(uint8_t *)((char *)transactions_array + i * 0x18 + 9) == 3) && // pkt_type == purchase
            (*(int *)((char *)transactions_array + i * 0x18 + 0x10) == *(int *)((char *)transaction + 0x10))) {
            matching_tx_count++;
        }
    }
    
    if (matching_tx_count < *requested_count_ptr) {
        *requested_count_ptr = matching_tx_count;
    }
    
    if (matching_tx_count == 0) {
        send_error(8, "No matching transaction history.");
        return 1; // Error
    }
    
    struct {
        uint32_t pkthdr_val;
        uint32_t dat_val;
        uint16_t dat_part;
        uint32_t concat_val;
        uint8_t zero_byte;
    } response_header;
    
    response_header.pkthdr_val = pkthdr;
    response_header.dat_val = DAT_00016024;
    response_header.dat_part = (uint16_t)DAT_00016028;
    response_header.concat_val = CONCAT31_GLUE((DAT_00016028 >> 24) & 0xFF, uRam0001602c);
    response_header.zero_byte = 0;
    
    send((int)(intptr_t)&response_header, (void *)(intptr_t)0xf, 0, 0);
    send((int)(intptr_t)requested_count_ptr, (void *)(intptr_t)sizeof(uint32_t), 0, 0);
    
    uint current_tx_idx = transactions_idx;
    uint sent_count = 0;
    while (current_tx_idx > 0 && sent_count < *requested_count_ptr) {
        current_tx_idx--;
        
        if ((*(uint8_t *)((char *)transactions_array + current_tx_idx * 0x18 + 9) == 3) &&
            (*(int *)((char *)transactions_array + current_tx_idx * 0x18 + 0x10) == *(int *)((char *)transaction + 0x10))) {
            
            struct {
                uint8_t field_8;
                uint8_t field_9;
                uint8_t field_10;
                uint32_t field_16;
                uint32_t field_12;
            } history_entry;
            
            history_entry.field_8 = *(uint8_t *)((char *)transactions_array + current_tx_idx * 0x18 + 8);
            history_entry.field_9 = *(uint8_t *)((char *)transactions_array + current_tx_idx * 0x18 + 9);
            history_entry.field_10 = *(uint8_t *)((char *)transactions_array + current_tx_idx * 0x18 + 10);
            history_entry.field_16 = *(uint32_t *)((char *)transactions_array + current_tx_idx * 0x18 + 0x10);
            history_entry.field_12 = *(uint32_t *)((char *)transactions_array + current_tx_idx * 0x18 + 0xc);
            
            send((int)(intptr_t)&history_entry, (void *)(intptr_t)sizeof(history_entry), 0, 0);
            sent_count++;
        }
    }
    
    *(uint8_t *)((char *)transaction + 9) = 2;  // transaction + 9 (pkt_type) = 2 (completed)
    *(uint8_t *)((char *)transaction + 10) = 0; // transaction + 10 (status) = 0 (success)
    
    return 0; // Success
}

// Function: handle_refund
undefined4 handle_refund(void) {
    int *data_ptr = *(int **)((char *)transaction + 0x14); // transaction[5]
    int *card = lookup_card(*(uint32_t *)((char *)transaction + 0x10)); // transaction[4]
    uint32_t transaction_id_to_refund = *(uint32_t *)data_ptr; // data_ptr[0]
    
    uint32_t refund_tx_addr = lookup_transaction(transaction_id_to_refund);
    
    if (refund_tx_addr == 0 ||
        *(uint8_t *)((char *)refund_tx_addr + 8) != 0 || // field_8
        *(uint8_t *)((char *)refund_tx_addr + 9) != 3 || // field_9 (pkt_type = purchase)
        *(uint8_t *)((char *)refund_tx_addr + 10) != 0) { // field_10 (status = success)
        send_error(0x12, "Unknown Error."); // 18 in decimal
        return 1; // Error
    }
    
    int *refunded_tx_data_ptr = *(int **)((char *)refund_tx_addr + 0x14); // refund_tx_addr[5]
    
    if (*refunded_tx_data_ptr != data_ptr[0]) { // Check if card ID matches
        send_error(0x12, "Unknown Error.");
        return 1; // Error
    }
    
    uint32_t refund_amount = (uint32_t)refunded_tx_data_ptr[1]; // refunded_tx_data_ptr[1]
    
    if (~(*(uint32_t *)((char *)card + 8)) < refund_amount) { // Check for overflow
        send_error(7, "Unable to refund card. Balance near max.");
        return 1; // Error
    }
    
    *(uint32_t *)((char *)card + 8) += refund_amount; // card[2] += refund_amount
    
    *(uint8_t *)((char *)transaction + 9) = 2;  // transaction + 9 (pkt_type) = 2 (completed)
    *(uint8_t *)((char *)transaction + 10) = 0; // transaction + 10 (status) = 0 (success)
    *(uint32_t *)((char *)transaction + 16) = (uint32_t)refunded_tx_data_ptr[2]; // transaction[4] = refunded_tx_data_ptr[2]
    
    *(uint8_t *)((char *)refund_tx_addr + 9) = 4; // Set refunded transaction pkt_type to 4 (refunded)
    
    struct {
        uint32_t pkthdr_val;
        uint32_t dat_val;
        uint16_t dat_part;
        uint32_t concat_val;
        uint8_t zero_byte;
    } response_header;
    
    response_header.pkthdr_val = pkthdr;
    response_header.dat_val = DAT_00016024;
    response_header.dat_part = (uint16_t)DAT_00016028;
    response_header.concat_val = CONCAT31_GLUE((DAT_00016028 >> 24) & 0xFF, uRam0001602c);
    response_header.zero_byte = 0;
    
    send((int)(intptr_t)&response_header, (void *)(intptr_t)0xf, 0, 0);
    
    return 0; // Success
}

// Function: process_ops
undefined4 process_ops(void) {
    switch ((DAT_00016028 >> 8) & 0xFF) { // DAT_00016028._1_1_
        case 0: return handle_purchase();
        case 1: return handle_recharge();
        case 2: return handle_balance();
        case 3: return handle_history();
        case 5: return handle_refund();
        default:
            send_error(9, "Invalid OP_CODE.");
            return 1; // Error (send_error calls exit)
    }
}

// Function: main
undefined4 main(void) {
    int *current_tx_ptr = NULL; // Acts as local_14 in original, pointer to transaction + 3
    
    enlarge_transactions_array(10); // Initialize transaction array capacity
    
    while (true) {
        if (read_fully((int)(intptr_t)&pkthdr, 0xf) != 0) { // Read 15 bytes into pkthdr and related globals
            return 0; // Exit on read error
        }
        
        transaction = NULL; // Reset current transaction for each new packet
        
        // DAT_00016028._1_1_ is the second byte of DAT_00016028 (OP_CODE)
        // (char)DAT_00016028 is the first byte of DAT_00016028 (PKT_TYPE)
        
        if (current_tx_ptr == NULL || *current_tx_ptr == (int)ram0x0001602b) {
            int *found_tx = (int *)lookup_transaction(ram0x0001602b);
            
            if (((DAT_00016028 >> 8) & 0xFF) == 4) { // OP_CODE == 4 (INIT_CARD)
                if ((DAT_00016028 & 0xFF) == 0) { // PKT_TYPE == 0 (INIT)
                    transaction = new_transaction();
                    if (transaction == NULL) {
                        send_error(3, "Failed to allocate memory."); // Exits
                    } else if (read_data((int)(intptr_t)transaction) != 0) {
                        send_error(0x12, "Unknown Error."); // Exits
                    } else if (handle_issue() == 0) { // handle_issue returns bool (false on success)
                        current_tx_ptr = (int *)((char *)transaction + 12); // transaction[3] is pointer to itself
                    }
                } else if (((DAT_00016028 & 0xFF) == 3) && (found_tx != NULL)) { // PKT_TYPE == 3 (FINALIZE)
                    current_tx_ptr = NULL; // Reset for next transaction
                    if (*(uint32_t *)((char *)found_tx + 16) != pkthdr || *(uint32_t *)found_tx != DAT_00016024) {
                        transaction = found_tx;
                        send_error(0xd, "Transaction has not completed INIT."); // Exits
                    }
                    
                    struct {
                        uint32_t pkthdr_val;
                        uint32_t dat_val;
                        uint16_t dat_part;
                        uint32_t concat_val;
                        uint8_t zero_byte;
                    } finalize_response;
                    
                    finalize_response.pkthdr_val = pkthdr;
                    finalize_response.dat_val = DAT_00016024;
                    finalize_response.dat_part = CONCAT11_GLUE(((DAT_00016028 >> 8) & 0xFF), (DAT_00016028 & 0xFF));
                    finalize_response.concat_val = ram0x0001602b;
                    finalize_response.zero_byte = 0;
                    
                    transaction = found_tx;
                    send((int)(intptr_t)&finalize_response, (void *)(intptr_t)0xf, 0, 0);
                    *(uint8_t *)((char *)transaction + 9) = 3; // transaction + 9 (pkt_type) = 3 (finalized)
                } else {
                    send_error(0xd, "Transaction has not completed INIT."); // Exits
                }
            } else if (found_tx == NULL) { // No existing transaction found
                if ((DAT_00016028 & 0xFF) == 1) { // PKT_TYPE == 1 (AUTH)
                    int *card = lookup_card(pkthdr);
                    if (card == NULL || *(int *)((char *)card + 16) != (int)DAT_00016024) { // card[4]
                        send_error(0x10, "Invalid card or auth code."); // Exits
                    }
                    
                    transaction = new_transaction();
                    
                    struct {
                        uint32_t pkthdr_val;
                        uint32_t dat_val;
                        uint16_t dat_part;
                        uint32_t concat_val;
                        uint8_t zero_byte;
                    } auth_response;
                    
                    auth_response.pkthdr_val = pkthdr;
                    auth_response.dat_val = DAT_00016024;
                    auth_response.dat_part = CONCAT11_GLUE(((DAT_00016028 >> 8) & 0xFF), (DAT_00016028 & 0xFF));
                    auth_response.concat_val = (uint32_t)(intptr_t)((char *)transaction + 12); // transaction[3]
                    auth_response.zero_byte = 0;
                    
                    send((int)(intptr_t)&auth_response, (void *)(intptr_t)0xf, 0, 0);
                    current_tx_ptr = (int *)((char *)transaction + 12); // transaction[3]
                } else {
                    send_error(0xc, "Transaction has not completed AUTH."); // Exits
                }
            } else { // Existing transaction found (found_tx != NULL)
                if (((DAT_00016028 >> 8) & 0xFF) != *(uint8_t *)((char *)found_tx + 8) || // OP_CODE
                    pkthdr != *(uint32_t *)((char *)found_tx + 16) || // pkthdr (found_tx[4])
                    DAT_00016024 != *(uint32_t *)found_tx) { // DAT_00016024 (found_tx[0])
                    send_error(0x12, "Unknown Error."); // Exits
                }
                
                if ((DAT_00016028 & 0xFF) != 2) { // PKT_TYPE != 2 (OPS)
                    if ((DAT_00016028 & 0xFF) != 3) { // PKT_TYPE != 3 (FINALIZE)
                        transaction = found_tx;
                        send_error(10, "Invalid PKT_TYPE."); // Exits
                    }
                    
                    current_tx_ptr = NULL; // Reset for next transaction
                    if (*(uint8_t *)((char *)found_tx + 9) != 2) { // transaction + 9 (pkt_type) != 2 (completed)
                        transaction = found_tx;
                        send_error(0xe, "Transaction has not completed OPS."); // Exits
                    }
                    
                    transaction = found_tx;
                    *(uint8_t *)((char *)transaction + 9) = 3; // transaction + 9 (pkt_type) = 3 (finalized)
                    
                    struct {
                        uint32_t pkthdr_val;
                        uint32_t dat_val;
                        uint16_t dat_part;
                        uint32_t concat_val;
                        uint8_t zero_byte;
                    } finalize_response_ops;
                    
                    finalize_response_ops.pkthdr_val = pkthdr;
                    finalize_response_ops.dat_val = DAT_00016024;
                    finalize_response_ops.dat_part = CONCAT11_GLUE(((DAT_00016028 >> 8) & 0xFF), (DAT_00016028 & 0xFF));
                    finalize_response_ops.concat_val = ram0x0001602b;
                    finalize_response_ops.zero_byte = 0;
                    
                    send((int)(intptr_t)&finalize_response_ops, (void *)(intptr_t)0xf, 0, 0);
                } else { // PKT_TYPE == 2 (OPS)
                    if (*(uint8_t *)((char *)found_tx + 9) != 1) { // transaction + 9 (pkt_type) != 1 (authenticated)
                        transaction = found_tx;
                        send_error(0xc, "Transaction has not completed AUTH."); // Exits
                    }
                    
                    transaction = found_tx;
                    if (read_data((int)(intptr_t)transaction) != 0) {
                        send_error(0x12, "Unknown Error."); // Exits
                    }
                    process_ops(); // process_ops returns 0 for success, 1 for error. It may call send_error which exits.
                }
            }
        } else { // current_tx_ptr is valid but does not match ram0x0001602b
            send_error(0xf, "Card or transaction not found."); // Exits
        }
    }
}