#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>      // For read, write, STDIN_FILENO, STDOUT_FILENO
#include <sys/types.h>   // For ssize_t
#include <float.h>       // For FLT_EPSILON

// Global constants derived from DAT_00013020 and DAT_0001301c
const float SQRT_TOLERANCE = FLT_EPSILON; // Tolerance for square root convergence
const float SQRT_DIVISOR = 2.0f;         // Divisor for averaging in square root

// Custom memory management stubs to mimic original behavior
// `allocate` returns 0 on success, non-zero on failure.
// `out_ptr_addr` is a pointer to where the allocated memory's address should be stored.
int allocate(size_t size, int flags, void **out_ptr_addr, unsigned int caller_addr) {
    *out_ptr_addr = malloc(size);
    if (*out_ptr_addr == NULL) {
        // In a real application, proper error logging or handling would be here.
        // fprintf(stderr, "Allocation failed (caller: 0x%x, size: %zu)\n", caller_addr, size);
        return 1; // Non-zero indicates failure
    }
    memset(*out_ptr_addr, 0, size); // Initialize allocated memory to zero
    return 0; // 0 indicates success
}

// `deallocate` frees memory. Returns 0 on success.
int deallocate(void *ptr_to_free) {
    free(ptr_to_free);
    return 0; // 0 indicates success (assuming free always succeeds or handles NULL gracefully)
}

// Stock entry structure (6 bytes total)
typedef struct {
    char id;
    short amount;
    short price; // This field is used as '0' to mark free/inactive entries
} StockEntry;

// Portfolio structure, mapping to main's stack variables based on access patterns
typedef struct {
    short cash_initial;         // Corresponds to main.local_34 (offset 0)
    unsigned short cash_current; // Corresponds to main.local_32 (offset 2)
    unsigned short target_cash;  // Corresponds to main.local_30 (offset 4)
    unsigned short capacity;     // Corresponds to first 2 bytes of main.local_28 (offset 6)
    StockEntry *stocks;         // Corresponds to main.local_24 (offset 8)
    unsigned short num_stocks;   // Corresponds to first 2 bytes of main.local_20 (offset 12)
} Portfolio;

// Function: nextRecord
// Reads 4 bytes into `record_out`. Interprets these as two `short` values
// for decision making. Returns 0 on EOF/zero-value, -1 on specific error condition, 1 on success.
int nextRecord(unsigned int *record_out) {
    ssize_t bytes_read = read(STDIN_FILENO, record_out, sizeof(unsigned int));
    if (bytes_read < 0) {
        exit(2); // Mimics _terminate(2)
    }
    if (bytes_read == 0) { // EOF or no data read, implies record_out is 0
        return 0;
    }

    // Original code reads 4 bytes into a short*, implying it treats the 4 bytes as two shorts.
    // So, we extract two short values from the unsigned int.
    short val1 = (short)(*record_out & 0xFFFF);
    short val2 = (short)((*record_out >> 16) & 0xFFFF);

    if (val1 == 0) {
        return 0;
    }
    if (val1 == -1 || val2 == 0) {
        return -1; // 0xffffffff
    }
    return 1;
}

// Function: squareRoot
// Calculates the square root of a float using an iterative method (Babylonian/binary search hybrid).
// Returns `long double` for precision.
long double squareRoot(float value) {
    if (value < 0.0f) {
        return NAN; // Not a Number for negative inputs
    }
    if (value == 0.0f) {
        return 0.0L;
    }

    float low = 0.0f;
    float high = value;
    float guess = value;
    float prev_guess = -1.0f; // Initialize to ensure the loop runs at least once

    while (fabsf(prev_guess - guess) >= SQRT_TOLERANCE) {
        prev_guess = guess;
        guess = (high + low) / SQRT_DIVISOR;
        if (value < guess * guess) {
            high = guess;
        } else {
            low = guess;
        }
    }
    return (long double)guess;
}

// Function: addRecordToDataset
// Updates statistics (count, mean, sum of squared differences, variance, std dev)
// for a dataset using Welford's online algorithm.
void addRecordToDataset(int *stats_array, unsigned int record_value) {
    // stats_array layout:
    // [0]: record_count
    // [1]: mean_price
    // [2]: sum_sq_diff
    // [3]: variance
    // [4]: std_dev

    float new_val_numerator = (float)(record_value & 0xFFFF);
    float new_val_denominator = (float)(record_value >> 16);
    // Avoid division by zero if denominator is 0
    float new_val_ratio = (new_val_denominator == 0.0f) ? 0.0f : (new_val_numerator / new_val_denominator);

    float old_mean = (float)stats_array[1];
    
    stats_array[0]++; // Increment record_count

    // Update mean
    stats_array[1] = (int)(old_mean + (new_val_ratio - old_mean) / (float)stats_array[0]);

    // Update sum of squared differences (Welford's algorithm)
    stats_array[2] = (int)(((new_val_ratio - (float)stats_array[1]) * (new_val_ratio - old_mean)) + (float)stats_array[2]);

    // Calculate variance (only if count > 0 to avoid division by zero)
    stats_array[3] = (int)((stats_array[0] > 0) ? ((float)stats_array[2] / (float)stats_array[0]) : 0.0f);

    // Calculate standard deviation
    stats_array[4] = (int)squareRoot((float)stats_array[3]);
}

// Function: getNextFreeEntry
// Finds or creates a free StockEntry slot in the portfolio's stock array.
StockEntry *getNextFreeEntry(Portfolio *portfolio) {
    // Search for an existing free entry (marked by price == 0)
    for (unsigned int i = 0; i < portfolio->num_stocks; ++i) {
        if (portfolio->stocks[i].price == 0) {
            return &portfolio->stocks[i];
        }
    }

    // If no existing free entry found, check if there's capacity for a new one
    if (portfolio->num_stocks < portfolio->capacity) {
        StockEntry *new_entry_ptr = &portfolio->stocks[portfolio->num_stocks];
        portfolio->num_stocks++; // Increment count of active/allocated entries
        return new_entry_ptr;
    }

    // No free entry and no capacity, so reallocate a larger array
    unsigned short old_num_stocks = portfolio->num_stocks;
    StockEntry *old_stocks_ptr = portfolio->stocks;

    unsigned short new_capacity = portfolio->capacity * 0x40 + 1; // Increase capacity significantly
    size_t new_size = (size_t)new_capacity * sizeof(StockEntry);
    StockEntry *new_stocks_ptr;

    if (allocate(new_size, 0, (void **)&new_stocks_ptr, 0x111e6) != 0) {
        exit(3); // Mimics _terminate(3) on allocation failure
    }
    
    // Copy existing stock data to the new, larger buffer
    if (old_stocks_ptr != NULL) {
        memcpy(new_stocks_ptr, old_stocks_ptr, old_num_stocks * sizeof(StockEntry));
    }

    // Update portfolio with the new stock array and capacity
    portfolio->stocks = new_stocks_ptr;
    portfolio->capacity = new_capacity;

    // Deallocate the old stock array
    if (old_stocks_ptr != NULL && deallocate(old_stocks_ptr) != 0) {
        exit(4); // Mimics _terminate(4) on deallocation failure
    }

    // Return the first newly available entry (which is at the end of the old data)
    StockEntry *new_entry_ptr = &portfolio->stocks[old_num_stocks];
    portfolio->num_stocks++; // Increment count of active/allocated entries
    return new_entry_ptr;
}

// Function: addStockToPortfolio
// Adds a stock entry to the portfolio. `packed_stock_data` contains ID and amount.
// `stock_price` is the price for this specific entry.
void addStockToPortfolio(unsigned int packed_stock_data, short stock_price, Portfolio *portfolio) {
    StockEntry *entry = getNextFreeEntry(portfolio);
    if (entry == NULL) {
        exit(5); // Custom error code for unexpected entry failure
    }
    
    entry->id = (char)(packed_stock_data & 0xFF);
    entry->amount = (short)((packed_stock_data >> 16) & 0xFFFF);
    entry->price = stock_price;
}

// Function: buyStock
// Buys a stock. `transaction_value` is interpreted as the 'amount' that is also the 'value'
// for cash updates in this simplified model.
void buyStock(char stock_id, unsigned short transaction_value, Portfolio *portfolio) {
    if (transaction_value <= portfolio->cash_current) {
        // Pack stock_id (lower 8 bits) and amount (upper 16 bits) into a single unsigned int
        unsigned int packed_stock_info = ((unsigned int)transaction_value << 16) | (unsigned int)stock_id;
        
        // The original code passed `sVar1` (transaction_value) as the 'price' to addStockToPortfolio.
        // This implies `transaction_value` serves as both the amount and the price in this context.
        addStockToPortfolio(packed_stock_info, (short)transaction_value, portfolio);
        portfolio->cash_current -= transaction_value;
    }
}

// Function: findMostExpensiveStock
// Finds the most expensive stock of a given ID in the portfolio.
StockEntry *findMostExpensiveStock(char stock_id, Portfolio *portfolio) {
    StockEntry *most_expensive_stock_ptr = NULL;
    unsigned short max_price = 0;

    for (unsigned int i = 0; i < portfolio->num_stocks; ++i) {
        StockEntry *current_entry = &portfolio->stocks[i];
        // Check if the entry is active (price != 0), matches the ID, and is more expensive
        if (current_entry->price != 0 && current_entry->id == stock_id && max_price < current_entry->price) {
            max_price = current_entry->price;
            most_expensive_stock_ptr = current_entry;
        }
    }
    return most_expensive_stock_ptr;
}

// Function: sellStock
// Sells the most expensive stock of a given ID. `transaction_value` is added to cash_current.
void sellStock(char stock_id, short transaction_value, Portfolio *portfolio) {
    // Original code checks param_3[3] != 0, which is portfolio->capacity != 0.
    // This is always true after initPortfolio. Assuming it's a general validity check.
    if (portfolio->capacity != 0) {
        StockEntry *entry_to_sell = findMostExpensiveStock(stock_id, portfolio);
        if (entry_to_sell != NULL) {
            portfolio->cash_current += transaction_value; // Add transaction_value to cash_current
            // Mark the entry as free/inactive
            entry_to_sell->id = 0;
            entry_to_sell->amount = 0;
            entry_to_sell->price = 0;
        }
    }
}

// Function: tradeStocks
// Implements a simple trading strategy based on a record ratio and thresholds.
// Unused parameters from original signature are omitted.
void tradeStocks(unsigned int current_record, float threshold_a, float threshold_b, Portfolio *portfolio) {
    float record_ratio_numerator = (float)(current_record & 0xFFFF);
    float record_ratio_denominator = (float)(current_record >> 16);
    float record_ratio = (record_ratio_denominator == 0.0f) ? 0.0f : (record_ratio_numerator / record_ratio_denominator);

    // If (threshold_b + threshold_a < record_ratio)
    if (threshold_b + threshold_a < record_ratio) {
        // Sell 'X' stock, transaction value is lower 16 bits of record
        sellStock('X', (short)(current_record & 0xFFFF), portfolio);
        // Buy 'Y' stock, transaction value is upper 16 bits of record
        buyStock('Y', (unsigned short)(current_record >> 16), portfolio);
    }
    // If (record_ratio < threshold_a - threshold_b)
    if (record_ratio < threshold_a - threshold_b) {
        // Sell 'Y' stock, transaction value is upper 16 bits of record
        sellStock('Y', (short)(current_record >> 16), portfolio);
        // Buy 'X' stock, transaction value is lower 16 bits of record
        buyStock('X', (unsigned short)(current_record & 0xFFFF), portfolio);
    }
}

// Function: sendGoalNotification
// Sends a notification message to standard output.
void sendGoalNotification(void) {
    const char *message = "You doubled your money!\n";
    size_t len = strlen(message);
    if (write(STDOUT_FILENO, message, len) != (ssize_t)len) {
        exit(1); // Mimics _terminate(1) on write failure
    }
}

// Function: sellAllStock
// Sells all active stocks in the portfolio.
void sellAllStock(Portfolio *portfolio) {
    for (unsigned int i = 0; i < portfolio->num_stocks; ++i) {
        StockEntry *current_entry = &portfolio->stocks[i];
        if (current_entry->price != 0) { // If stock is active
            // Sell stock, using its current amount as the transaction value
            sellStock(current_entry->id, current_entry->amount, portfolio);
        }
    }
}

// Function: updateStockValues
// Updates the 'amount' of 'X' and 'Y' stocks based on the current record value.
// `cash_current` is updated based on old and new amounts.
void updateStockValues(unsigned int current_record, Portfolio *portfolio) {
    for (unsigned int i = 0; i < portfolio->num_stocks; ++i) {
        StockEntry *entry = &portfolio->stocks[i];
        if (entry->price != 0) { // If stock is active
            portfolio->cash_current -= entry->amount; // Subtract old 'amount' from cash_current

            if (entry->id == 'X') {
                entry->amount = (short)(current_record & 0xFFFF);
            }
            if (entry->id == 'Y') {
                entry->amount = (short)(current_record >> 16);
            }
            portfolio->cash_current += entry->amount; // Add new 'amount' to cash_current
        }
    }
}

// Function: initPortfolio
// Initializes a Portfolio structure with an initial cash value and allocates stock storage.
void initPortfolio(Portfolio *portfolio, unsigned short initial_cash_value) {
    portfolio->cash_initial = initial_cash_value;
    portfolio->cash_current = portfolio->cash_initial;
    portfolio->target_cash = portfolio->cash_current * 2;
    portfolio->capacity = 0x10; // Initial capacity for stock entries

    // Allocate initial memory for stock entries
    if (allocate((size_t)portfolio->capacity * sizeof(StockEntry), 0, (void **)&portfolio->stocks, 0x1178b) != 0) {
        exit(3); // Mimics _terminate(3) on allocation failure
    }
}

// Function: main
// Main entry point of the program. Processes records, manages portfolio, and performs trades.
int main(void) {
    unsigned int current_record_value;
    Portfolio portfolio;
    int stats[5]; // Stores: [0]record_count, [1]mean, [2]sum_sq_diff, [3]variance, [4]std_dev

    // Initialize stats array to zero
    memset(stats, 0, sizeof(stats));

    // Initialize portfolio struct fields to zero
    memset(&portfolio, 0, sizeof(portfolio));

    // Trade parameters, representing `main.local_24` and `main.local_18` in original code.
    // These are initialized to 0.0f, as their initial values are not set in the original snippet.
    float trade_threshold_low = 0.0f;
    float trade_threshold_high = 0.0f;

    while (1) {
        int record_status = nextRecord(&current_record_value);

        if (record_status == -1) {
            // End of input or critical error from nextRecord, terminate program
            if (portfolio.stocks != NULL) {
                deallocate(portfolio.stocks);
                portfolio.stocks = NULL;
            }
            return 0;
        }

        if (record_status == 0) {
            // A record_status of 0 implies an initialization or reset signal
            // If portfolio already has allocated stocks, deallocate them first
            if (portfolio.stocks != NULL) {
                deallocate(portfolio.stocks);
                portfolio.stocks = NULL;
            }
            // Initialize/reset the portfolio with the initial cash value from the record
            initPortfolio(&portfolio, (unsigned short)(current_record_value >> 16));
            continue; // Continue to the next record
        }
        
        // If record_status is 1 (valid record)
        // Check if enough records have been processed to start trading
        if (stats[0] > 100) {
            updateStockValues(current_record_value, &portfolio);
            
            // Check for goal achievement
            if (portfolio.target_cash <= portfolio.cash_current) {
                sellAllStock(&portfolio);
                sendGoalNotification();
            }

            // Check for zero cash_current condition
            if (portfolio.cash_current == 0) {
                if (portfolio.target_cash == 0) {
                    // If both cash_current and target_cash are zero, exit
                    if (portfolio.stocks != NULL) {
                        deallocate(portfolio.stocks);
                        portfolio.stocks = NULL;
                    }
                    return 0;
                }
                sellAllStock(&portfolio); // If cash_current is zero but target is not, sell all stocks
            }
            
            // Perform stock trades based on current record and thresholds
            tradeStocks(current_record_value, trade_threshold_low, trade_threshold_high, &portfolio);
        }
        // Add the current record to the dataset for statistics calculation
        addRecordToDataset(stats, current_record_value);
    }
}