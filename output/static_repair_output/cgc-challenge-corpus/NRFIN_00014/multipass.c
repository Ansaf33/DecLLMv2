#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For uint32_t, uint8_t, uintptr_t

// Decompiler-specific type mapping
typedef uint8_t byte;
typedef uint32_t undefined4;

// For EVP_PKEY_CTX, assuming it's a pointer to an array of 4-byte values.
typedef undefined4 EVP_PKEY_CTX_STRUCT;

// Forward declarations for external functions (dummy implementations provided below)
void _terminate(void);
int get_account(undefined4 card_id, undefined4 auth_code);
int create_account(undefined4 card_id, undefined4 auth_code, int initial_balance);
int open_txn(int account_ptr, void* packet_ptr);
int add_transaction_log_entry(int account_ptr, void* param_packet);
int get_transaction(int account_ptr, undefined4 txn_id);
int transaction_iterator(int account_ptr); // returns int (likely a pointer to a transaction struct address)
int finalize_transaction(int transaction_ptr);
int remove_transaction_log_entry(int account_ptr, int transaction_ptr);

// Global variables
int NEXT_CARD_ID = 1;
int NEXT_TXN_ID = 1;
int NEXT_AUTH_CODE = 1;
void *RESP_PAD = NULL;
void *RESP_PACKET = NULL;
int ERRNO = 0;
const char *ERROR_MSGS[] = {
    "No error", // 0
    "Memory allocation failed", // 1 (Placeholder)
    "Invalid packet type", // 2 (Placeholder)
    "Allocation failed", // 3
    "Authentication failed", // 4 (Placeholder)
    "Insufficient funds", // 5
    "Amount out of range", // 6
    "Refund amount too large", // 7
    "Invalid transaction", // 8 (Placeholder)
    "Invalid state", // 9
    "Initialization error", // 10
    "Invalid parameter", // 11
    "Transaction open failed", // 12
    "Account not found", // 13 (Placeholder)
    "Transaction not found", // 14 (Placeholder)
    "Account/Transaction not found", // 15 (0xF)
    "Payload error", // 16 (Placeholder)
    "Packet malformed", // 17 (Placeholder)
    "Response payload error" // 18 (0x12)
};

// Dummy implementations for functions not provided in the snippet
void _terminate(void) {
    fprintf(stderr, "Program terminated due to an error. ERRNO: %d (%s)\n", ERRNO, ERROR_MSGS[ERRNO]);
    exit(EXIT_FAILURE);
}

int get_account(undefined4 card_id, undefined4 auth_code) {
    (void)card_id; (void)auth_code;
    // Simulate finding an account
    // For simplicity, always return a dummy account_ptr if not 0, else 0 for error
    if (card_id == 0 && auth_code == 0) {
        return 0; // Simulate account not found
    }
    // Dummy account structure simulation
    static uint32_t dummy_account[4] = {0x1000, 0x1001, 1000, 0x1234}; // card_id, auth_code, balance, some_other_field
    return (int)(uintptr_t)dummy_account; // Dummy account pointer
}
int create_account(undefined4 card_id, undefined4 auth_code, int initial_balance) {
    (void)card_id; (void)auth_code; (void)initial_balance;
    return 1; // Simulate success
}
int open_txn(int account_ptr, void* packet_ptr) {
    (void)account_ptr; (void)packet_ptr;
    // Simulate opening a transaction, return a dummy transaction pointer
    // Dummy transaction structure simulation
    static uint32_t dummy_txn[2] = {0x2000, 0x5678}; // txn_id, some_other_field
    return (int)(uintptr_t)dummy_txn;
}
int add_transaction_log_entry(int account_ptr, void* param_packet) {
    (void)account_ptr; (void)param_packet;
    return 1; // Simulate success
}
int get_transaction(int account_ptr, undefined4 txn_id) {
    (void)account_ptr; (void)txn_id;
    // Simulate finding a transaction
    if (txn_id == 0) {
        return 0; // Simulate transaction not found
    }
    // Dummy transaction structure simulation for process_client_refund_op accesses
    // struct transaction { void* data_ptr; size_t data_size; int* account_ptr_in_txn; undefined4 field_10; void* payload_ptr; };
    // This needs to be static to keep the address valid.
    static uint32_t dummy_transaction_data[6]; // Sufficient space for fields
    dummy_transaction_data[0] = 0x3000; // data_ptr (dummy)
    dummy_transaction_data[2] = 50;     // data_size (offset 8, 50 units)
    dummy_transaction_data[3] = (uint32_t)(uintptr_t)get_account(0x1000, 0x1001); // account_ptr_in_txn (offset 12)
    dummy_transaction_data[4] = 0x42;   // field_10 (offset 16)
    dummy_transaction_data[5] = 0x5000; // payload_ptr (offset 20, dummy)
    return (int)(uintptr_t)dummy_transaction_data; // Dummy transaction pointer
}
int transaction_iterator(int account_ptr) {
    static int current_txn_count = 0;
    if (account_ptr != 0) { // Initial call
        current_txn_count = 1; // Start from first dummy transaction
    } else { // Subsequent calls
        current_txn_count++;
    }
    if (current_txn_count <= 3) { // Simulate 3 transactions
        // Return a dummy pointer to a transaction struct.
        // This struct needs to have fields accessed by process_client_history.
        // Let's make it a static array to simulate the structure.
        static uint32_t dummy_txn_iter_data[6]; // Sufficient space for fields
        dummy_txn_iter_data[0] = 0x4000 + current_txn_count; // data_ptr (dummy)
        dummy_txn_iter_data[2] = 20 + current_txn_count * 5; // data_size (offset 8)
        // Simulate account_ptr_in_txn pointing to a dummy account struct
        static uint32_t dummy_account_for_txn[4] = {0x1000, 0x1001, 1000, 0x1234}; // card_id, auth_code, balance, some_other_field
        dummy_txn_iter_data[3] = (uint32_t)(uintptr_t)dummy_account_for_txn; // account_ptr_in_txn (offset 12)
        dummy_txn_iter_data[4] = (uint32_t)(0x40 + current_txn_count); // field_10 (offset 16)
        dummy_txn_iter_data[5] = 0x5000 + current_txn_count; // payload_ptr (offset 20, dummy)
        return (int)(uintptr_t)dummy_txn_iter_data;
    }
    current_txn_count = 0; // Reset for next initial call
    return 0; // End of iteration
}
int finalize_transaction(int transaction_ptr) {
    (void)transaction_ptr;
    return 0; // Simulate success
}
int remove_transaction_log_entry(int account_ptr, int transaction_ptr) {
    (void)account_ptr; (void)transaction_ptr;
    return 0; // Simulate success
}


// Function: check_auth_packet
// Returns account_ptr on success, or calls _terminate() on failure.
int check_auth_packet(undefined4 *param_1) {
    if (*(char *)((byte*)param_1 + 8) != 0x01) {
        _terminate(); // Program exits here
    }
    // Assuming get_account(*param_1, param_1[1]) is the intended call
    return get_account(param_1[0], param_1[1]);
}

// Function: verify_init
int verify_init(EVP_PKEY_CTX_STRUCT *ctx) {
    if (ctx == NULL) {
        return 10;
    } else if (ctx[8] == 0) {
        if (ctx[9] == 0) {
            return 0;
        } else {
            return 9;
        }
    } else {
        return 10;
    }
}

// Function: alloc_new_pkt
void * alloc_new_pkt(int param_1) {
    return calloc(1, param_1 + 16); // 0x10 is 16
}

// Function: register_card_id
int register_card_id(void) {
    return NEXT_CARD_ID++;
}

// Function: register_transaction
int register_transaction(void) {
    return NEXT_TXN_ID++;
}

// Function: register_auth_code
int register_auth_code(void) {
    return NEXT_AUTH_CODE++;
}

// Function: init_resp_structs
void init_resp_structs(void) {
    RESP_PAD = malloc(12); // 0xc is 12 bytes
    RESP_PACKET = alloc_new_pkt(0); // Assuming 0 or a default size if no arg is given

    if (RESP_PACKET == NULL) {
        _terminate();
    }
    if (RESP_PAD == NULL) {
        _terminate();
    }
    return;
}

// Function: clear_gen_pkt
int clear_gen_pkt(int param_1) {
    if (RESP_PACKET != NULL) {
        memset(RESP_PACKET, 0, param_1 + 16);
    }
    return (int)(uintptr_t)RESP_PACKET; // Cast to int as the return type is int
}

// Function: clear_pad
undefined4 * clear_pad(void) {
    if (RESP_PAD != NULL) {
        ((undefined4*)RESP_PAD)[0] = 0;
        ((undefined4*)RESP_PAD)[1] = 0;
        ((undefined4*)RESP_PAD)[2] = 0;
    }
    return (undefined4*)RESP_PAD;
}

// Function: create_basic_packet_head
undefined4 * create_basic_packet_head(undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4) {
    undefined4 *pkt = (undefined4 *)clear_gen_pkt(param_4);
    if (pkt == NULL) {
        ERRNO = 3;
        return NULL;
    }
    pkt[0] = param_1;
    pkt[1] = param_2;
    pkt[3] = param_3;
    return pkt;
}

// Function: create_basic_response
int * create_basic_response(undefined4 param_1, undefined4 param_2, undefined4 param_3,
                            byte param_4, byte param_5, byte param_6, int param_7) {
    int *packet_head = (int *)(uintptr_t)create_basic_packet_head(param_1, param_2, param_3, param_7);
    if (packet_head == NULL) {
        return NULL;
    }

    int *pad = (int *)clear_pad();
    if (pad == NULL) {
        return NULL;
    }

    // Accessing bytes within the packet_head (which is RESP_PACKET)
    ((byte*)packet_head)[8] = param_4;
    ((byte*)packet_head)[9] = param_5;
    ((byte*)packet_head)[10] = param_6;

    if (param_7 == 0) {
        pad[2] = 0;
    } else {
        pad[2] = (int)((uintptr_t)packet_head + 16); // packet_head + 0x10 (payload start)
    }
    pad[0] = (int)(uintptr_t)packet_head;
    pad[1] = param_7;
    return pad;
}

// Function: generate_new_init_and_init_resp
undefined4 generate_new_init_and_init_resp(int *param_1) {
    int current_txn_id_addr = *param_1; // This int holds the address of the packet
    int initial_balance = param_1[2];

    if (initial_balance == 0) {
        return 0;
    }

    undefined4 card_id = register_card_id();
    undefined4 auth_code = register_auth_code();

    if (create_account(card_id, auth_code, initial_balance) == 0) {
        return 0; // Account creation failed
    }

    int account_ptr = get_account(card_id, auth_code);
    if (account_ptr == 0) {
        return 0; // Get account failed
    }

    open_txn(account_ptr, (void*)(uintptr_t)current_txn_id_addr);

    // Store generated card_id and auth_code into the first two fields of the packet
    *(undefined4 *)( (uintptr_t)current_txn_id_addr ) = card_id;
    *(undefined4 *)( (uintptr_t)current_txn_id_addr + 4) = auth_code;

    if (add_transaction_log_entry(account_ptr, param_1) == 0) {
        return 0; // Log entry failed
    }

    // *(undefined4 *)(current_txn_id_addr + 0xc) implies current_txn_id_addr is a pointer to a structure
    return create_basic_response(card_id, auth_code, *(undefined4 *)( (uintptr_t)current_txn_id_addr + 12), 0, 4, 0, 0);
}

// Function: error_out_and_free_txn
undefined4 error_out_and_free_txn(void *param_1, int param_2) {
    if (param_2 != 0) {
        ERRNO = param_2;
    }
    free(param_1);
    return 0;
}

// Function: get_acct_txn_reg
int * get_acct_txn_reg(undefined4 param_1, undefined4 param_2, undefined4 param_3) {
    int account_ptr = get_account(param_1, param_2);
    if (account_ptr == 0) {
        ERRNO = 0xF; // 15
        return NULL;
    }

    int transaction_ptr = get_transaction(account_ptr, param_3);
    if (transaction_ptr == 0) {
        ERRNO = 0xF; // 15
        return NULL;
    }

    int *reg_pair = (int *)malloc(8); // Allocate space for two ints
    if (reg_pair == NULL) {
        ERRNO = 3;
        return NULL;
    }
    reg_pair[0] = account_ptr;
    reg_pair[1] = transaction_ptr;
    return reg_pair;
}

// Function: calc_payload_sz
undefined4 calc_payload_sz(int param_1) {
    if ((*(byte *)( (uintptr_t)param_1 + 8) == 0x02) && (*(byte *)( (uintptr_t)param_1 + 9) == 0x02)) {
        return 4;
    } else {
        return 0;
    }
}

// Function: get_payload
int get_payload(int param_1) {
    return param_1 + 16; // 0x10 is 16
}

// Function: process_client_history
int * process_client_history(int **param_1, int *param_2) {
    // Assuming param_1 points to an array of pointers:
    // param_1[0] -> int* (packet header: card_id, auth_code, txn_id, etc.)
    // param_1[1] -> int* (unused in this function)
    // param_1[2] -> uint* (max history count to retrieve)

    int *packet_head = *param_1;
    uint32_t max_history_count_limit = *(uint32_t *)param_1[2]; // Access the value pointed to by param_1[2]

    undefined4 card_id = (undefined4)packet_head[0];
    undefined4 auth_code = (undefined4)packet_head[1];
    undefined4 txn_id = (undefined4)packet_head[3]; // Assuming packet_head[2] is balance or unused

    undefined4 account_ptr_val = get_account(card_id, auth_code);
    int log_entry_ptr = add_transaction_log_entry(account_ptr_val, param_1);

    int *acct_txn_reg = get_acct_txn_reg(card_id, auth_code, txn_id);
    if (acct_txn_reg == NULL) {
        return NULL;
    }

    uint32_t history_count = 0;
    int current_balance_sum = 0;

    // First pass to calculate total size and history count
    int transaction_iter_ptr = transaction_iterator(acct_txn_reg[0]); // Start iteration with account_ptr
    while (transaction_iter_ptr != 0) {
        // Assume transaction_iter_ptr points to a struct where field at offset 0xc is a pointer to the account struct
        int *txn_account_ptr = *(int **)((uintptr_t)transaction_iter_ptr + 12);
        if (card_id == (undefined4)txn_account_ptr[0]) { // Check if transaction belongs to this card_id
            current_balance_sum += *(int *)((uintptr_t)transaction_iter_ptr + 8); // Data size (transaction amount)
            if (history_count >= max_history_count_limit) {
                break; // Max history count reached
            }
            history_count++;
        }
        transaction_iter_ptr = transaction_iterator(0); // Next iteration
    }

    // Allocate response packet based on calculated size
    int *response_pad = create_basic_response(card_id, auth_code, txn_id, 2, 3, 0, history_count * 12 + current_balance_sum + 4);
    if (response_pad == NULL) {
        free(acct_txn_reg);
        return NULL;
    }

    // Set specific byte in response packet (packet_head is at response_pad[0])
    ((byte*)( (uintptr_t)response_pad[0] ))[9] = 3;

    // Get payload pointer for the response packet (payload pointer is at response_pad[2])
    uint32_t *response_payload_ptr = (uint32_t *)(uintptr_t)response_pad[2];

    // Update log entry (assuming log_entry_ptr is a pointer to a structure)
    *(undefined4 *)((uintptr_t)log_entry_ptr + 16) = 2;

    uint32_t current_history_index = 0;
    int current_payload_offset = 0;
    uint32_t *current_payload_write_ptr = response_payload_ptr + 1; // Skip first 4 bytes for history_count

    // Second pass to copy history data into the response payload
    transaction_iter_ptr = transaction_iterator(acct_txn_reg[0]); // Reset iterator
    while (transaction_iter_ptr != 0) {
        int *txn_account_ptr = *(int **)((uintptr_t)transaction_iter_ptr + 12);
        if (card_id == (undefined4)txn_account_ptr[0]) {
            if (current_history_index >= max_history_count_limit) {
                break; // Max history count reached
            }

            // Copy transaction details to response payload
            ((byte*)current_payload_write_ptr)[0] = *(byte *)((uintptr_t)txn_account_ptr + 9);
            ((byte*)current_payload_write_ptr)[1] = *(char *)((uintptr_t)transaction_iter_ptr + 16);
            ((byte*)current_payload_write_ptr)[2] = *(byte *)((uintptr_t)txn_account_ptr + 10);

            current_payload_write_ptr[1] = (uint32_t)txn_account_ptr[0]; // Card ID
            current_payload_write_ptr[2] = *(uint32_t *)((uintptr_t)txn_account_ptr + 12); // Auth Code or similar

            size_t transaction_data_size = *(size_t *)((uintptr_t)transaction_iter_ptr + 8);
            memcpy(current_payload_write_ptr + 3, *(void **)((uintptr_t)transaction_iter_ptr + 20), transaction_data_size);

            current_history_index++;
            int entry_total_size = transaction_data_size + 12;
            current_payload_offset += entry_total_size;
            current_payload_write_ptr = (uint32_t *)((uintptr_t)current_payload_write_ptr + entry_total_size);
        }
        transaction_iter_ptr = transaction_iterator(0); // Next iteration
    }

    *param_2 = current_payload_offset + 4; // Output total payload size (including history_count field)
    response_pad[1] = current_payload_offset + 4; // Update response packet size field
    response_payload_ptr[0] = history_count; // Store total history count at the beginning of payload

    finalize_transaction(log_entry_ptr); // Finalize the log entry
    free(acct_txn_reg);
    return response_pad;
}

// Function: process_client_balance
undefined4 * process_client_balance(int *param_1) {
    // param_1 is `int*`. `*param_1` is the address of the packet.
    undefined4 *packet_head = (undefined4 *)( (uintptr_t)*param_1 );
    undefined4 card_id = packet_head[0];
    undefined4 auth_code = packet_head[1];
    undefined4 txn_id = packet_head[3];

    int *acct_txn_reg = get_acct_txn_reg(card_id, auth_code, txn_id);
    if (acct_txn_reg == NULL) {
        return NULL;
    }

    undefined4 *response_pad = (undefined4 *)create_basic_response(card_id, auth_code, txn_id, 2, 2, 0, 4); // Payload size 4 for balance
    if (response_pad == NULL) {
        free(acct_txn_reg);
        return NULL;
    }

    // response_pad[0] is the packet head address
    undefined4 *response_payload = (undefined4 *)(uintptr_t)get_payload((int)(uintptr_t)response_pad[0]);
    if (response_payload == NULL) {
        ERRNO = 0x12; // 18
        free(acct_txn_reg);
        return NULL;
    }

    // *(undefined4 *)(acct_txn_reg[0] + 0xc) is the balance field of the account structure.
    *response_payload = *(undefined4 *)( (uintptr_t)acct_txn_reg[0] + 12);
    add_transaction_log_entry(acct_txn_reg[0], response_pad); // Log entry for account, response pad
    free(acct_txn_reg);
    return response_pad;
}

// Function: process_client_refund_op
undefined4 process_client_refund_op(int *param_1) {
    // param_1: [packet_ptr, unused, refund_details_ptr]
    undefined4 *packet_head = (undefined4 *)( (uintptr_t)*param_1 );
    int *refund_details = (int *)param_1[2]; // Likely struct { int account_id; undefined4 transaction_id; }

    undefined4 card_id = packet_head[0];
    undefined4 auth_code = packet_head[1];
    undefined4 txn_id = packet_head[3];

    int *acct_txn_reg = get_acct_txn_reg(card_id, auth_code, txn_id);
    if (acct_txn_reg == NULL) {
        return 0;
    }

    // refund_details[1] is the transaction ID to refund
    int transaction_to_refund_ptr = get_transaction(acct_txn_reg[0], (undefined4)refund_details[1]);
    if (transaction_to_refund_ptr == 0) {
        ERRNO = 0xF; // 15
        free(acct_txn_reg);
        return 0;
    }

    // Check if the account ID in the transaction matches the one in refund_details[0]
    // *(int **)(transaction_to_refund_ptr + 0x14) is a pointer to the account struct associated with the transaction
    int *txn_associated_account_ptr = *(int **)((uintptr_t)transaction_to_refund_ptr + 20); // 0x14 is 20
    if (txn_associated_account_ptr[0] != refund_details[0]) { // Compare account IDs
        ERRNO = 0xF; // Transaction does not belong to the specified account
        free(acct_txn_reg);
        return 0;
    }

    // Refund amount is `txn_associated_account_ptr[1]`
    // Current balance is `*(uint32_t *)(acct_txn_reg[0] + 0xc)`
    uint32_t current_balance = *(uint32_t *)((uintptr_t)acct_txn_reg[0] + 12);
    uint32_t refund_amount = (uint32_t)txn_associated_account_ptr[1];
    uint32_t new_balance = current_balance + refund_amount;

    if (new_balance < current_balance) { // Check for overflow (shouldn't happen with refund)
        ERRNO = 7; // Refund amount too large (or overflow)
        free(acct_txn_reg);
        return 0;
    }

    *(uint32_t *)((uintptr_t)acct_txn_reg[0] + 12) = new_balance; // Update balance
    add_transaction_log_entry(acct_txn_reg[0], param_1);
    remove_transaction_log_entry(acct_txn_reg[0], transaction_to_refund_ptr);
    free(acct_txn_reg);

    return create_basic_response(card_id, auth_code, txn_id, 2, 5, 0, 0);
}

// Function: process_client_recharge_op
undefined4 process_client_recharge_op(int *param_1) {
    // param_1: [packet_ptr, unused, recharge_details_ptr]
    undefined4 *packet_head = (undefined4 *)( (uintptr_t)*param_1 );
    int *recharge_details = (int *)param_1[2]; // Likely struct { int amount; byte flags[...]; }

    undefined4 card_id = packet_head[0];
    undefined4 auth_code = packet_head[1];
    undefined4 txn_id = packet_head[3];

    int *acct_txn_reg = get_acct_txn_reg(card_id, auth_code, txn_id);
    if (acct_txn_reg == NULL) {
        return 0;
    }

    // *(byte *)(recharge_details + 2) implies `((byte*)recharge_details)[8]` (if recharge_details is int*)
    if (*(byte *)((uintptr_t)recharge_details + 8) < 7) { // Check a byte field (e.g., flags)
        uint32_t current_balance = *(uint32_t *)((uintptr_t)acct_txn_reg[0] + 12);
        uint32_t recharge_amount = (uint32_t)recharge_details[0]; // *recharge_details is recharge amount

        uint32_t new_balance = current_balance + recharge_amount;

        if (new_balance < current_balance) { // Check for overflow
            return error_out_and_free_txn(acct_txn_reg, 6); // Amount out of range
        } else {
            *(uint32_t *)((uintptr_t)acct_txn_reg[0] + 12) = new_balance; // Update balance
            add_transaction_log_entry(acct_txn_reg[0], param_1);
            free(acct_txn_reg);
            return create_basic_response(card_id, auth_code, txn_id, 2, 1, 0, 0);
        }
    } else {
        return error_out_and_free_txn(acct_txn_reg, 0xB); // Invalid parameter
    }
}

// Function: generate_error
undefined4 * generate_error(undefined4 *param_1) {
    // param_1: packet_head
    undefined4 card_id = param_1[0];
    undefined4 auth_code = param_1[1];
    undefined4 txn_id = param_1[3];
    byte packet_type = *(byte *)((uintptr_t)param_1 + 8);
    byte sub_type = *(byte *)((uintptr_t)param_1 + 9);

    // Check for allocation error before attempting to generate an error response
    if (ERRNO == 3) {
        _terminate(); // Cannot allocate error response, critical failure
    }

    const char *error_msg = ERROR_MSGS[ERRNO];
    size_t error_msg_len = strlen(error_msg);
    int payload_size = error_msg_len + 4; // 4 bytes for length prefix

    undefined4 *response_pad = (undefined4 *)create_basic_response(
        card_id,
        auth_code,
        txn_id,
        packet_type,
        sub_type,
        (byte)ERRNO,
        payload_size
    );

    if (response_pad == NULL) {
        return NULL; // Failed to create error response packet
    }

    // response_pad[0] is the packet head address
    undefined4 *response_payload = (undefined4 *)(uintptr_t)get_payload((int)(uintptr_t)response_pad[0]);
    if (response_payload == NULL) {
        return NULL;
    }

    *response_payload = (undefined4)error_msg_len; // Store length of error message
    memcpy(response_payload + 1, error_msg, error_msg_len); // Copy error message

    return response_pad;
}

// Function: process_client_purchase_op
undefined4 process_client_purchase_op(int *param_1) {
    // param_1: [packet_ptr, unused, purchase_details_ptr]
    undefined4 *packet_head = (undefined4 *)( (uintptr_t)*param_1 );
    int *purchase_details = (int *)param_1[2]; // Likely struct { int txn_id; int amount; byte flags[...]; }

    undefined4 card_id = packet_head[0];
    undefined4 auth_code = packet_head[1];
    undefined4 txn_id = packet_head[3];

    int *acct_txn_reg = get_acct_txn_reg(card_id, auth_code, txn_id);
    if (acct_txn_reg == NULL) {
        return 0;
    }

    // *(byte *)(purchase_details + 0xc) implies `((byte*)purchase_details)[12]`
    if (*(byte *)((uintptr_t)purchase_details + 12) < 7) { // Check a byte field (e.g., flags)
        uint32_t current_balance = *(uint32_t *)((uintptr_t)acct_txn_reg[0] + 12);
        uint32_t purchase_amount = *(uint32_t *)((uintptr_t)purchase_details + 4); // *(uint *)(purchase_details + 4)

        if (current_balance < purchase_amount) {
            return error_out_and_free_txn(acct_txn_reg, 5); // Insufficient funds
        } else {
            *(uint32_t *)((uintptr_t)acct_txn_reg[0] + 12) = current_balance - purchase_amount; // Update balance
            add_transaction_log_entry(acct_txn_reg[0], param_1);
            free(acct_txn_reg);
            return create_basic_response(card_id, auth_code, txn_id, 2, 0, 0, 0); // Type 2, Subtype 0 for purchase success
        }
    } else {
        return error_out_and_free_txn(acct_txn_reg, 0xB); // Invalid parameter
    }
}

// Function: create_auth_resp
void create_auth_resp(undefined4 param_1, undefined4 param_2, undefined4 param_3) {
    // create_basic_response already handles packet head creation and setting bytes.
    // The byte at offset 8 is set to 1.
    create_basic_response(param_1, param_2, param_3, 1, 0, 0, 0);
    return;
}

// Function: process_client_auth
int * process_client_auth(int *param_1) {
    // param_1 is `int*`. `*param_1` is the address of the packet.
    undefined4 *packet_head = (undefined4 *)( (uintptr_t)*param_1 );
    undefined4 card_id = packet_head[0];
    undefined4 auth_code = packet_head[1];

    int account_ptr = check_auth_packet(packet_head); // Now returns int (account_ptr)
    if (account_ptr == 0) {
        // _terminate() would have been called inside check_auth_packet if it was a critical error.
        // If it returns 0, it means get_account failed.
        ERRNO = 0xF; // Account not found
        return NULL;
    }

    int transaction_ptr = open_txn(account_ptr, packet_head);
    if (transaction_ptr == 0) {
        ERRNO = 0xC; // 12, Transaction open failed
        return NULL;
    }

    undefined4 new_txn_id = *(undefined4 *)((uintptr_t)transaction_ptr + 4); // Get txn_id from transaction struct
    packet_head[3] = new_txn_id; // Update original packet with new transaction ID

    int *response_pad = create_basic_response(card_id, auth_code, new_txn_id, 1, 0, 0, 0); // Type 1, Subtype 0 for auth response
    if (response_pad == NULL) {
        return NULL;
    }

    // Set a byte in the response packet (which is pointed to by response_pad[0])
    // This copies a byte from the original request packet to the response packet.
    ((byte*)( (uintptr_t)response_pad[0] ))[9] = *(byte *)((uintptr_t)packet_head + 9); // Copy sub_type from request
    return response_pad;
}

// Function: process_client_fin
undefined4 process_client_fin(int *param_1) {
    // param_1: [packet_ptr, ...]
    undefined4 *packet_head = (undefined4 *)( (uintptr_t)*param_1 );
    undefined4 card_id = packet_head[0];
    undefined4 auth_code = packet_head[1];
    undefined4 txn_id = packet_head[3];

    int account_ptr = get_account(card_id, auth_code);
    if (account_ptr == 0) {
        ERRNO = 0xF; // 15, Account not found
        return 0;
    }

    int transaction_ptr = get_transaction(account_ptr, txn_id);
    if (transaction_ptr == 0) {
        // ERRNO might be set by get_transaction, or already 0xF.
        return 0;
    }

    if (finalize_transaction(transaction_ptr) == 0) {
        // If finalize_transaction returns 0, it means success.
        // Create a basic response with type 3, subtype from original packet.
        return create_basic_response(card_id, auth_code, txn_id, 3, *(byte *)((uintptr_t)packet_head + 9), 0, 0);
    } else {
        // finalize_transaction failed
        ERRNO = 8; // Placeholder for transaction finalize error
        return 0;
    }
}