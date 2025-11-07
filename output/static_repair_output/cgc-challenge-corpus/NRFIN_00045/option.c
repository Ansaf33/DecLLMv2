#include <stdio.h>   // For fprintf, stderr
#include <stdlib.h>  // For exit
#include <string.h>  // For memcmp, memcpy
#include <stdint.h>  // For uint32_t, uint8_t

// Global constants based on usage
#define MAX_ACCOUNTS 32
#define MAX_HOLDINGS_PER_ACCOUNT 32
#define MAX_ORDERS 1024

// Forward declarations of structs
typedef struct Account Account;
typedef struct AccountHoldingSlot AccountHoldingSlot;
typedef struct Message Message;
typedef struct Order Order;
typedef struct Response Response;
typedef struct OrderFillResponse OrderFillResponse;

// Custom terminate function replacement
// This function is called in error conditions and exits the program.
void _terminate(int error_code) {
    fprintf(stderr, "Termination with error code: %d\n", error_code);
    exit(error_code);
}

// Struct definitions based on memory access patterns inferred from the snippet

// Represents a single holding entry within an account.
// Total size: 4 bytes (quantity) + 8 bytes (symbol) = 0xc bytes.
typedef struct AccountHoldingSlot {
    uint32_t quantity; // At offset 0x0 relative to start of AccountHoldingSlot
    char symbol[8];    // At offset 0x4 relative to start of AccountHoldingSlot
} AccountHoldingSlot;

// Represents an account structure.
// Total size: 0x1a4 bytes.
// Balance is at offset 0x0.
// Holdings array starts at offset 0x24.
typedef struct Account {
    float balance; // Offset 0x0
    char padding[0x24 - sizeof(float)]; // Padding to reach 0x24
    AccountHoldingSlot holdings[MAX_HOLDINGS_PER_ACCOUNT]; // Array starts at offset 0x24
} Account;

// Represents a message or transaction request structure.
// Contains price, quantity, and symbol.
// Total size: 4 bytes (price) + 4 bytes (quantity) + 8 bytes (symbol) = 0x10 bytes.
typedef struct Message {
    float price;      // At offset 0x0
    uint32_t quantity; // At offset 0x4
    char symbol[8];    // At offset 0x8
} Message;

// Represents an order in the order book.
// Total size: 0x3c bytes.
typedef struct Order {
    uint32_t account_id; // At offset 0x0 (ID of the account that created this order)
    float price;      // At offset 0x4
    uint32_t quantity; // At offset 0x8 (remaining quantity for this order)
    char symbol[8];   // At offset 0xc
    char padding1[0x34 - 0x14]; // Padding from 0x14 to 0x34
    uint32_t order_id; // At offset 0x34 (unique ID for the order)
    int order_type;    // At offset 0x38 (e.g., 0 for buy, 0xff for sell)
} Order;

// Generic response structure
typedef struct Response {
    uint32_t status;
    uint32_t field1;
    uint32_t field2;
} Response;

// Specific response structure for order fill messages
typedef struct OrderFillResponse {
    uint32_t status;         // 0x0
    uint32_t fill_status;    // 0x4
    uint32_t transaction_id; // 0x8 (placeholder, not explicitly set by generic_resp original)
    char padding1[0x14 - 0xc]; // Padding to 0x14
    char symbol_buffer[8];   // 0x14 (buffer to hold a symbol)
    uint32_t other_data;     // 0x1c (another data field)
} OrderFillResponse;


// Global arrays and counter
Account ACCOUNTS[MAX_ACCOUNTS];
Order ORDERBOOK[MAX_ORDERS];
uint32_t ORDER_ID = 1; // Starts from 1


// Function: match_holding
// Finds a holding for a given symbol within an account.
// Returns a pointer to the holding's quantity if found, otherwise NULL.
int* match_holding(int account_idx, const Message* msg) {
    if (account_idx >= MAX_ACCOUNTS) {
        return NULL; // Invalid account index
    }
    for (int holding_idx = 0; holding_idx < MAX_HOLDINGS_PER_ACCOUNT; ++holding_idx) {
        // Compare the symbol from the message with the current holding's symbol
        if (memcmp(msg->symbol, ACCOUNTS[account_idx].holdings[holding_idx].symbol, 8) == 0) {
            return (int*)&ACCOUNTS[account_idx].holdings[holding_idx].quantity;
        }
    }
    return NULL; // Holding not found
}

// Function: add_holding
// Adds or updates the quantity of a holding for a specific account and symbol.
// If the holding's quantity is -1 (special value), it is not updated.
// Returns a pointer to the updated holding's quantity.
int* add_holding(int account_idx, const Message* msg) {
    int* holding_qty_ptr = match_holding(account_idx, msg);
    if (holding_qty_ptr == NULL) {
        // In a real system, an empty slot would be found and initialized here.
        // For now, based on original code, it terminates if not found.
        _terminate(0x4d);
    }
    if (*holding_qty_ptr != -1) { // Assuming -1 means "not initialized" or "empty"
        *holding_qty_ptr += msg->quantity;
    }
    return holding_qty_ptr;
}

// Function: dec_holding
// Decrements the quantity of a holding for a specific account and symbol.
// Only decrements if the requested quantity is less than the current holding.
// Returns a pointer to the updated holding's quantity.
uint32_t* dec_holding(int account_idx, const Message* msg) {
    uint32_t* holding_qty_ptr = (uint32_t*)match_holding(account_idx, msg);
    if (holding_qty_ptr == NULL) {
        _terminate(0x37);
    }
    if (msg->quantity < *holding_qty_ptr) { // Original code used strict less than.
        *holding_qty_ptr -= msg->quantity;
    }
    return holding_qty_ptr;
}

// Function: generic_resp
// Fills a generic response structure.
void generic_resp(Response* resp, uint32_t field1_val, uint32_t field2_val) {
    resp->status = 1;
    resp->field1 = field1_val;
    resp->field2 = field2_val;
}

// Function: fill_order
// Fills a portion or all of an existing order (e.g., a sell order being bought).
// Updates account balances, holdings, and the order's remaining quantity.
// Returns a status code indicating the outcome of the fill.
uint32_t fill_order(int seller_account_idx, const Message* trade_msg, Order* order_to_fill) {
    float transaction_value = order_to_fill->price * (float)trade_msg->quantity;

    // Update balances
    ACCOUNTS[seller_account_idx].balance -= transaction_value;
    ACCOUNTS[order_to_fill->account_id].balance += transaction_value; // order_to_fill->account_id is the buyer's ID

    // Seller's holding of the sold item decreases
    dec_holding(seller_account_idx, trade_msg);

    // Buyer's holding of the bought item increases
    int* buyer_holding_qty_ptr = add_holding(order_to_fill->account_id, trade_msg);

    // Check if the trade quantity exceeds the order's remaining quantity
    if (trade_msg->quantity > order_to_fill->quantity) { // Original used (uint) casts; assuming uint32_t comparison is intended
        _terminate(0x66); // Error: trying to fill more than available
    }

    order_to_fill->quantity -= trade_msg->quantity;

    if (order_to_fill->quantity == 0) {
        order_to_fill->order_type = 0; // Mark order as filled/inactive (original set `status` to 0, `price` to 0.0)
        order_to_fill->price = 0.0f; // Clear price when order is fully filled
        return 0xff0000; // Fully filled status
    } else if (*buyer_holding_qty_ptr == -1) {
        return 0xee; // Partially filled, but buyer's holding was special (-1)
    } else {
        return 0; // Partially filled, normal status
    }
}

// Function: match_symbol
// Compares symbols from two message/order-like structures.
// Arguments are pointers to the 'price' field, and symbols are assumed to be at offset +8 from there.
// (This is based on the original decompiler output's peculiar argument passing for `match_symbol`).
int match_symbol(const float* ptr1_price, const float* ptr2_price) {
    // Assuming symbol is at offset +8 from the price field in both structures
    return memcmp((const char*)ptr1_price + 8, (const char*)ptr2_price + 8, 8);
}

// Function: get_current_ask
// Finds the lowest ask price (sell order price) for a given symbol.
// Returns 0.0 if no matching sell order is found.
long double get_current_ask(const Message* msg) {
    float min_price = 0.0f;
    uint32_t min_order_id = 0; // 0 for initial state, any real order_id will be positive

    for (int i = 0; i < MAX_ORDERS; ++i) {
        Order* order = &ORDERBOOK[i];
        if (order->order_type == 0xff) { // Check if it's a SELL order
            // Compare order symbol with message symbol
            if (memcmp(order->symbol, msg->symbol, 8) == 0) {
                if (order->quantity != 0) { // Check if order has remaining quantity
                    // Check for lowest order_id (oldest order) or first valid match
                    if (order->order_id < min_order_id || min_order_id == 0) {
                        min_order_id = order->order_id;
                        min_price = order->price;
                    }
                }
            }
        }
    }
    return (long double)min_price;
}

// Function: find_sell_order
// Finds the best (oldest, matching price, sufficient quantity) sell order for a given buy message.
// Returns a pointer to the best Order if found, otherwise NULL.
Order* find_sell_order(const Message* buy_msg) {
    uint32_t best_order_id = 0;
    Order* best_order_ptr = NULL;

    for (int i = 0; i < MAX_ORDERS; ++i) {
        Order* current_order = &ORDERBOOK[i];
        // Check if order slot is active (symbol not empty) and is a sell order
        if (current_order->symbol[0] != '\0' && current_order->order_type == 0xff) {
            // Check if symbols match
            if (memcmp(current_order->symbol, buy_msg->symbol, 8) == 0) {
                // Check if order price is less than or equal to buyer's offered price
                if (current_order->price <= buy_msg->price) {
                    // Check if order has quantity and buyer's quantity <= order's quantity
                    if (current_order->quantity != 0 && buy_msg->quantity <= current_order->quantity) {
                        // Found a matching sell order
                        if (best_order_ptr == NULL || current_order->order_id < best_order_id) {
                            // This is the first matching order or an older (better) one
                            best_order_id = current_order->order_id;
                            best_order_ptr = current_order;
                        }
                    }
                }
            }
        }
    }
    return best_order_ptr;
}

// Function: add_to_order_book
// Adds a new order to the global order book in the first available empty slot.
// Returns 0x99 on success, 0xffffff if the order book is full.
uint32_t add_to_order_book(const Message* msg, uint32_t account_id, int order_type) {
    int empty_slot_idx = -1;
    for (int i = 0; i < MAX_ORDERS; ++i) {
        if (ORDERBOOK[i].symbol[0] == '\0') { // Find empty slot (symbol[0] == '\0' indicates empty)
            empty_slot_idx = i;
            break;
        }
    }

    if (empty_slot_idx == -1) {
        return 0xffffff; // Order book full
    }

    Order* new_order_slot = &ORDERBOOK[empty_slot_idx];

    // Copy message details to the order slot
    memcpy(new_order_slot->symbol, msg->symbol, 8);
    new_order_slot->quantity = msg->quantity;
    new_order_slot->price = msg->price;
    new_order_slot->account_id = account_id;

    if (order_type != 0 && order_type != 0xff) {
        _terminate(99); // Invalid order type
    }
    new_order_slot->order_type = order_type;
    new_order_slot->order_id = ORDER_ID;
    ORDER_ID++; // Increment global order ID

    return 0x99; // Success status
}

// Function: check_account_balance_can_buy
// Checks if an account has sufficient balance to make a purchase.
// Returns 0 if balance is sufficient, 0x55 if not enough balance, 0xf0 for invalid account.
uint32_t check_account_balance_can_buy(const Message* buy_msg, uint32_t account_idx) {
    if (account_idx >= MAX_ACCOUNTS) {
        return 0xf0; // Invalid account
    }

    float required_funds = buy_msg->price * (float)buy_msg->quantity;
    // Original code used `<=`, meaning if balance is exactly equal to required funds, it's considered "not enough".
    // Keeping this behavior.
    if (ACCOUNTS[account_idx].balance <= required_funds) {
        return 0x55; // Not enough balance
    } else {
        return 0; // Enough balance
    }
}

// Function: gen_order_fill_msg
// Generates an order fill response message.
// (Note: The original function had complex stack manipulation and implied arguments.
// This refactored version makes reasonable assumptions for its purpose.)
uint32_t gen_order_fill_msg(OrderFillResponse* resp, uint32_t fill_status, char* dest_symbol_ptr, uint32_t other_data_val) {
    // Initialize generic fields in the response
    resp->status = 1;
    resp->fill_status = fill_status;
    // resp->transaction_id is not explicitly set by original generic_resp, leaving as default or assuming prior setup

    if (fill_status != 0 && fill_status != 0xff) { // 0 or 0xff are considered valid fill statuses
        _terminate(99); // Invalid fill status
    }

    // Copy symbol from the response's internal buffer to the provided destination pointer
    memcpy(dest_symbol_ptr, resp->symbol_buffer, 8);
    
    // Set an additional data field in the response
    resp->other_data = other_data_val;

    return 8; // Returns 8, possibly indicating the size of the copied data or a success code
}

// Function: check_account_holding_in_qty_sell
// Checks if an account has sufficient quantity of a holding to sell.
// Returns 0 if holding is sufficient, 0x66 if not enough, 0x88 if holding not found.
uint32_t check_account_holding_in_qty_sell(const Message* sell_msg, int account_idx) {
    uint32_t* holding_qty_ptr = dec_holding(account_idx, sell_msg); // match_holding is called internally by dec_holding
    if (holding_qty_ptr == NULL) { // If holding not found by match_holding
        return 0x88; // Holding not found
    }

    // Check if the message quantity is less than or equal to the current holding quantity
    // (dec_holding already checks if msg->quantity < *holding_qty_ptr for decrement,
    // this check is for overall sufficiency to sell).
    if (sell_msg->quantity <= *holding_qty_ptr) {
        return 0; // Enough holding
    }
    return 0x66; // Not enough holding
}

// Function: fill_buy_order
// Attempts to fill a buy order by finding and processing a matching sell order.
// Returns a status code from fill_order, or 0x88 if no sell order is found.
uint32_t fill_buy_order(uint32_t buyer_account_idx, Order* sell_order, const Message* buy_msg) {
    if (sell_order == NULL) {
        return 0x88; // No matching sell order found
    }
    // `fill_order` expects `seller_account_idx`, `trade_msg`, `order_to_fill`.
    // The `sell_order->account_id` is the seller's account.
    // `buy_msg` is the trade message.
    // `sell_order` is the order to fill.
    return fill_order(sell_order->account_id, buy_msg, sell_order);
}

// Function: run_option_transaction
// Main function to run either a buy or sell transaction.
// Orchestrates checks and order book operations.
// Returns 0 on success, or an error code on failure.
uint32_t run_option_transaction(uint32_t account_idx, Message* transaction_msg, int transaction_type) {
    uint32_t result = 0; // Default success

    if (transaction_type == 0) { // Buy transaction
        result = check_account_balance_can_buy(transaction_msg, account_idx);
        if (result == 0) { // If balance is sufficient
            Order* sell_order = find_sell_order(transaction_msg);
            result = fill_buy_order(account_idx, sell_order, transaction_msg);
            // Convert fill_buy_order's specific success codes (0, 0xff0000, 0xee) to generic 0 for success
            if (result == 0 || result == 0xff0000 || result == 0xee) {
                result = 0;
            }
        }
    } else if (transaction_type == 0xff) { // Sell transaction
        result = check_account_holding_in_qty_sell(transaction_msg, account_idx);
        if (result == 0) { // If holding is sufficient
            // `add_to_order_book` returns 0x99 on success, 0xffffff on failure.
            result = add_to_order_book(transaction_msg, account_idx, 0xff);
            if (result == 0x99) {
                result = 0; // Convert 0x99 success to generic 0
            }
        }
    } else {
        result = 0xf1; // Invalid transaction type
    }
    return result;
}