#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// --- Global definitions and structures ---

// Constants based on decompiled offsets and loop limits
#define MAX_ACCOUNTS 100 // Based on check_account_balance_can_buy param_2 < 100
#define MAX_HOLDINGS_PER_ACCOUNT 32 // Based on 0x1f < local_10 loop limit
#define SYMBOL_LEN 8 // Symbol length (e.g., "AAPL    ")

// Holding structure within an Account
typedef struct {
    char symbol[SYMBOL_LEN]; // Offset 0x0 within Holding struct
    uint32_t quantity;       // Offset 0x8 within Holding struct
} Holding; // Size: 0x8 + 0x4 = 0xc

// Account structure, total size 0x1a4 (420 bytes)
typedef struct {
    float balance; // Offset 0x0 within Account struct
    uint8_t _padding1[0x24 - sizeof(float)]; // Padding to 0x24 (20 bytes)
    Holding holdings[MAX_HOLDINGS_PER_ACCOUNT]; // Offset 0x24 within Account struct
} Account; // Size: 0x24 + 32 * 0xc = 0x24 + 0x180 = 0x1a4.

// Global array for accounts
uint8_t ACCOUNTS_RAW[sizeof(Account) * MAX_ACCOUNTS];
Account* ACCOUNTS_PTR = (Account*)ACCOUNTS_RAW;

// Order structure in ORDERBOOK, total size 0x3c (60 bytes)
#define MAX_ORDERS 1024

typedef struct {
    uint32_t account_id; // 0x0 (Account ID of the order placer)
    float price;         // 0x4
    uint32_t quantity;    // 0x8
    char symbol[SYMBOL_LEN]; // 0xc (8 bytes)
    uint8_t _padding2[0x34 - (0xc + SYMBOL_LEN)]; // Padding to 0x34 (16 bytes)
    uint32_t order_id;    // 0x34
    int order_type;      // 0x38 (0 for buy, 0xff for sell)
} Order; // Size: 0x3c.

// Global array for order book
uint8_t ORDERBOOK_RAW[sizeof(Order) * MAX_ORDERS];
Order* ORDERBOOK_PTR = (Order*)ORDERBOOK_RAW;
uint32_t ORDER_ID = 1; // Global order ID counter

// Unified Transaction Request structure
// This structure attempts to reconcile the various field accesses from the original code,
// assuming a consistent layout for price, quantity, and symbol in request messages.
typedef struct {
    float price;            // Offset 0x0: Used for order price (buy/sell).
    uint32_t quantity;       // Offset 0x4: Used for order quantity.
    char symbol[SYMBOL_LEN]; // Offset 0x8: Used for symbol.
    uint32_t account_id;     // Offset 0x10: Used for seller_account_id in fill_order, or other purposes.
    uint8_t status;          // Offset 0x14: Used for order status (0=filled).
    uint8_t _padding3[3];     // Padding to 0x18
} TransactionRequest; // Size: 0x18

// Generic Response structure
typedef struct {
    uint32_t field1;
    uint32_t field2;
    uint32_t field3;
} GenericResponse;

// Order Fill Message structure
typedef struct {
    uint32_t account_id;
    uint32_t status_code;
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
} OrderFillMessage;

// --- Helper for _terminate ---
// Replaces the decompiled `_terminate` calls with a standard exit.
void _terminate(int error_code) {
    fprintf(stderr, "Termination requested with code: %d\n", error_code);
    exit(error_code);
}

// --- Function: match_holding ---
// Finds a holding for a given symbol in an account.
// param_1: account_idx - The index of the account.
// param_2: req - Pointer to a TransactionRequest containing the symbol.
// Returns a pointer to the holding's quantity if found, otherwise NULL.
uint32_t* match_holding(int account_idx, const TransactionRequest* req) {
    for (int i = 0; i < MAX_HOLDINGS_PER_ACCOUNT; ++i) {
        Holding* current_holding = &ACCOUNTS_PTR[account_idx].holdings[i];
        if (memcmp(req->symbol, current_holding->symbol, SYMBOL_LEN) == 0) {
            return &current_holding->quantity;
        }
    }
    return NULL;
}

// --- Function: add_holding ---
// Adds quantity to an existing holding in an account. Terminates if holding not found.
// param_1: account_idx - The index of the account.
// param_2: req - Pointer to a TransactionRequest containing the quantity and symbol.
// Returns a pointer to the updated holding's quantity.
uint32_t* add_holding(uint32_t account_idx, const TransactionRequest* req) {
    uint32_t* holding_qty_ptr = match_holding(account_idx, req);
    if (holding_qty_ptr == NULL) {
        _terminate(0x4d); // Error code from original
    }
    if (*holding_qty_ptr != (uint32_t)-1) { // -1 might signify an empty slot or special value
        *holding_qty_ptr += req->quantity;
    }
    return holding_qty_ptr;
}

// --- Function: dec_holding ---
// Decrements quantity from an existing holding in an account. Terminates if holding not found.
// param_1: account_idx - The index of the account.
// param_2: req - Pointer to a TransactionRequest containing the quantity and symbol.
// Returns a pointer to the updated holding's quantity.
uint32_t* dec_holding(uint32_t account_idx, const TransactionRequest* req) {
    uint32_t* holding_qty_ptr = match_holding(account_idx, req);
    if (holding_qty_ptr == NULL) {
        _terminate(0x37); // Error code from original
    }
    if (req->quantity < *holding_qty_ptr) {
        *holding_qty_ptr -= req->quantity;
    }
    return holding_qty_ptr;
}

// --- Function: fill_order ---
// Processes the filling of a buy order against a sell order.
// param_1: buyer_account_idx - The account ID of the buyer.
// param_2: sell_order - Pointer to the sell order being filled from the order book.
// param_3: client_buy_req - Pointer to the client's buy request (which will be modified).
// Returns a status code (0, 0xee, 0xff0000).
uint32_t fill_order(int buyer_account_idx, Order* sell_order, TransactionRequest* client_buy_req) {
    uint32_t sell_qty = sell_order->quantity;
    float trade_price = client_buy_req->price;
    float trade_amount = trade_price * (float)sell_qty;

    // Update balances
    ACCOUNTS_PTR[buyer_account_idx].balance -= trade_amount;
    ACCOUNTS_PTR[client_buy_req->account_id].balance += trade_amount; // client_buy_req->account_id is the seller's ID

    // Seller decrements holding
    TransactionRequest seller_dec_req;
    seller_dec_req.quantity = sell_qty;
    memcpy(seller_dec_req.symbol, sell_order->symbol, SYMBOL_LEN);
    dec_holding(client_buy_req->account_id, &seller_dec_req);

    // Buyer increments holding
    TransactionRequest buyer_add_req;
    buyer_add_req.quantity = sell_qty;
    memcpy(buyer_add_req.symbol, sell_order->symbol, SYMBOL_LEN);
    uint32_t* buyer_holding_qty_ptr = add_holding(buyer_account_idx, &buyer_add_req);

    // Update client's buy request
    if (client_buy_req->quantity < sell_qty) {
        _terminate(0x66);
    }
    client_buy_req->quantity -= sell_qty;

    uint32_t return_code;
    if (client_buy_req->quantity == 0) {
        // If client's order is fully filled
        client_buy_req->status = 0; // Set status to 0 (filled)
        client_buy_req->price = 0.0; // Clear price
        return_code = 0xff0000; // Original return value
    } else if (*buyer_holding_qty_ptr == (uint32_t)-1) {
        // If buyer's holding quantity is -1 (special value?)
        return_code = 0xee;
    } else {
        return_code = 0;
    }
    return return_code;
}

// --- Function: match_symbol_internal ---
// Compares symbols embedded within structures by pointer arithmetic.
// param_1: order_price_ptr - Pointer to an Order's price field.
// param_2: client_order_price_ptr - Pointer to a TransactionRequest's price field.
// Returns 0 if symbols match, non-zero otherwise (as per memcmp).
int match_symbol_internal(const float* order_price_ptr, const float* client_order_price_ptr) {
    const char* order_symbol = (const char*)order_price_ptr + (0xc - 0x4); // Offset from Order->price to Order->symbol
    const char* client_symbol = (const char*)client_order_price_ptr + (0x8 - 0x0); // Offset from TransactionRequest->price to TransactionRequest->symbol
    return memcmp(order_symbol, client_symbol, SYMBOL_LEN);
}

// --- Function: get_current_ask ---
// Finds the lowest sell price for a given symbol.
// param_1: symbol - Pointer to the 8-byte symbol string.
// Returns the lowest ask price as a long double.
long double get_current_ask(const char* symbol) {
    float min_price = 0.0;
    uint32_t min_order_id = 0; // Used to break ties (older order wins)

    for (int i = 0; i < MAX_ORDERS; ++i) {
        Order* current_order = &ORDERBOOK_PTR[i];

        if (current_order->order_type == 0xff && current_order->quantity != 0) { // Active sell order
            if (memcmp(current_order->symbol, symbol, SYMBOL_LEN) == 0) { // Symbols match
                if (min_order_id == 0 || current_order->order_id < min_order_id) { // Find oldest order
                    min_order_id = current_order->order_id;
                    min_price = current_order->price;
                }
            }
        }
    }
    return (long double)min_price;
}

// --- Function: find_sell_order ---
// Finds the best (oldest, matching price/quantity) sell order for a client's buy request.
// param_1: client_buy_req - Pointer to the client's buy request.
// Returns a pointer to the best matching sell Order, or NULL if none found.
Order* find_sell_order(const TransactionRequest* client_buy_req) {
    uint32_t best_order_id = 0;
    Order* best_sell_order = NULL;

    for (int i = 0; i < MAX_ORDERS; ++i) {
        Order* current_order = &ORDERBOOK_PTR[i];

        if (current_order->order_type == 0xff && current_order->quantity > 0) { // Active sell order
            if (match_symbol_internal(&current_order->price, &client_buy_req->price) == 0) { // Symbols match
                if (current_order->price <= client_buy_req->price) { // Sell price <= buy price
                    if (client_buy_req->quantity <= current_order->quantity) { // Sell order has enough quantity
                        if (best_order_id == 0 || current_order->order_id < best_order_id) { // Find oldest
                            best_order_id = current_order->order_id;
                            best_sell_order = current_order;
                        }
                    }
                }
            }
        }
    }
    return best_sell_order;
}

// --- Function: add_to_order_book ---
// Adds a new order (buy or sell) to the order book.
// param_1: client_order_req - Pointer to the client's order request.
// param_2: account_idx - The account ID placing the order.
// param_3: order_type - Type of order (0 for buy, 0xff for sell).
// Returns a status code (0x99 or 0xffffff).
uint32_t add_to_order_book(const TransactionRequest* client_order_req, uint32_t account_idx, int order_type) {
    for (int i = 0; i < MAX_ORDERS; ++i) {
        // Find an empty slot (symbol[0] == '\0' indicates empty)
        if (ORDERBOOK_PTR[i].symbol[0] == '\0') {
            Order* new_order = &ORDERBOOK_PTR[i];

            new_order->price = client_order_req->price;
            new_order->quantity = client_order_req->quantity;
            memcpy(new_order->symbol, client_order_req->symbol, SYMBOL_LEN);
            new_order->account_id = account_idx;

            if (order_type != 0 && order_type != 0xff) {
                _terminate(99);
            }
            new_order->order_type = order_type;
            new_order->order_id = ORDER_ID++;
            return 0x99; // Original return value
        }
    }
    return 0xffffff; // No empty slot
}

// --- Function: check_account_balance_can_buy ---
// Checks if an account has sufficient balance to place a buy order.
// param_1: client_buy_req - Pointer to the client's buy request.
// param_2: account_idx - The account ID.
// Returns 0 if balance is sufficient, 0x55 if insufficient, 0xf0 if invalid account.
uint32_t check_account_balance_can_buy(const TransactionRequest* client_buy_req, uint32_t account_idx) {
    if (account_idx >= MAX_ACCOUNTS) {
        return 0xf0;
    }
    float required_amount = client_buy_req->price * (float)client_buy_req->quantity;
    if (ACCOUNTS_PTR[account_idx].balance <= required_amount) {
        return 0x55; // Not enough balance
    }
    return 0; // Can buy
}

// --- Function: generic_resp ---
// Fills a generic response structure.
// param_1: resp - Pointer to the GenericResponse structure.
// param_2: val2 - Value for field2.
// param_3: val3 - Value for field3.
void generic_resp(GenericResponse* resp, uint32_t val2, uint32_t val3) {
    resp->field1 = 1;
    resp->field2 = val2;
    resp->field3 = val3;
}

// --- Function: gen_order_fill_msg ---
// Generates an order fill message. The original had complex stack manipulation;
// this version simplifies it to fill a message structure.
// param_1: msg - Pointer to the OrderFillMessage structure to fill.
// param_2: account_id - Account ID involved.
// param_3: status_code - Status of the fill.
// param_4: val3 - Generic value 3.
// param_5: val4 - Generic value 4.
// param_6: val5 - Generic value 5.
// Returns the size of the message structure.
uint32_t gen_order_fill_msg(OrderFillMessage* msg, uint32_t account_id, uint32_t status_code, uint32_t val3, uint32_t val4, uint32_t val5) {
    GenericResponse resp_internal;
    generic_resp(&resp_internal, status_code, val5);

    if (status_code != 0 && status_code != 0xff) {
        _terminate(99);
    }

    msg->account_id = account_id;
    msg->status_code = status_code;
    msg->value1 = val3;
    msg->value2 = val4;
    msg->value3 = val5;

    return sizeof(OrderFillMessage);
}

// --- Function: check_account_holding_in_qty_sell ---
// Checks if an account has sufficient quantity of a holding to place a sell order.
// param_1: sell_req - Pointer to the sell request.
// param_2: account_idx - The account ID.
// Returns 0 if sufficient, 0x66 if insufficient, 0x88 if holding not found.
uint32_t check_account_holding_in_qty_sell(const TransactionRequest* sell_req, uint32_t account_idx) {
    for (int i = 0; i < MAX_HOLDINGS_PER_ACCOUNT; ++i) {
        Holding* current_holding = &ACCOUNTS_PTR[account_idx].holdings[i];
        if (memcmp(current_holding->symbol, sell_req->symbol, SYMBOL_LEN) == 0) {
            if (sell_req->quantity <= current_holding->quantity) {
                return 0; // Enough quantity
            } else {
                return 0x66; // Not enough quantity
            }
        }
    }
    return 0x88; // Holding not found
}

// --- Function: fill_buy_order ---
// Initiates the process to fill a buy order.
// param_1: buyer_account_idx - The account ID of the buyer.
// param_2: matched_sell_order - Pointer to the matched sell order from the order book.
// param_3: client_buy_req - Pointer to the client's buy request.
// Returns a status code from fill_order (0, 0x88, 0xee, 0xff0000).
uint32_t fill_buy_order(uint32_t buyer_account_idx, Order* matched_sell_order, TransactionRequest* client_buy_req) {
    if (matched_sell_order == NULL) {
        return 0x88; // No sell order found
    }
    return fill_order(buyer_account_idx, matched_sell_order, client_buy_req);
}

// --- Function: run_option_transaction ---
// Main entry point for running a buy or sell transaction.
// param_1: account_idx - The account ID initiating the transaction.
// param_2: order_req_ptr - Pointer to the transaction request (TransactionRequest*).
// param_3: transaction_type - Type of transaction (0 for buy, 0xff for sell).
// Returns a status code.
int run_option_transaction(uint32_t account_idx, TransactionRequest* order_req_ptr, int transaction_type) {
    uint32_t status_code = 0;

    if (transaction_type == 0) { // Buy order
        status_code = check_account_balance_can_buy(order_req_ptr, account_idx);
        if (status_code == 0) {
            Order* matched_sell_order = find_sell_order(order_req_ptr);
            status_code = fill_buy_order(account_idx, matched_sell_order, order_req_ptr);
        }
    } else if (transaction_type == 0xff) { // Sell order
        status_code = check_account_holding_in_qty_sell(order_req_ptr, account_idx);
        if (status_code == 0) {
            status_code = add_to_order_book(order_req_ptr, account_idx, 0xff);
        }
    } else {
        status_code = 0xf1; // Invalid transaction type
    }
    return status_code;
}