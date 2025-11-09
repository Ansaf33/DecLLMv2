#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> // For write

// Define custom types
typedef uint32_t undefined4;

// Forward declarations for functions that reference each other
// and the main context structure.
struct StockExchangeContext;
typedef struct StockExchangeContext StockExchangeContext;

// --- Custom function implementations/placeholders ---

// Placeholder for strhash
unsigned int strhash(const char *s, size_t len) {
    unsigned int hash = 5381;
    for (size_t i = 0; i < len && s[i] != '\0'; ++i) {
        hash = ((hash << 5) + hash) + s[i];
    }
    return hash;
}

// Placeholder for itoa using snprintf
char *my_itoa(int value, char *buffer) {
    sprintf(buffer, "%d", value);
    return buffer;
}

// Placeholder for write_all
ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t result = write(fd, (const char *)buf + bytes_written, count - bytes_written);
        if (result == -1) {
            perror("write_all failed");
            return -1;
        }
        bytes_written += result;
    }
    return bytes_written;
}

// Memory pool simulation
void pool_init(StockExchangeContext *ctx, size_t item_size);
int pool_alloc(StockExchangeContext *ctx);
void pool_free(StockExchangeContext *ctx, int addr);
void pool_destroy(StockExchangeContext *ctx);

// Dummy callback for order completion
void on_complete_dummy(int order_ptr) {
    (void)order_ptr; // Suppress unused parameter warning
    // fprintf(stderr, "Order %d completed.\n", *(int*)(intptr_t)(order_ptr + 8));
}

// Global variable placeholder for _GLOBAL_OFFSET_TABLE_
// The original `on_complete` returns `&_GLOBAL_OFFSET_TABLE_`.
// This usually refers to the Global Offset Table for position-independent code.
// For a simple compilable C code, this is not directly accessible or relevant.
// We make it a dummy pointer.
undefined4 * _GLOBAL_OFFSET_TABLE_;

// Structure to represent the Stock Exchange context
// The fields are arranged to match the offsets used in the original decompiled code.
#define HASH_BUCKET_COUNT 0xfb
struct StockExchangeContext {
    char _padding_0_to_B[0xC]; // Padding to align stock_list_head to 0xC
    int stock_list_head;       // 0xC: Head of global list of all stocks
    int stock_list_tail;       // 0x10: Tail of global list of all stocks
    int order_list_head;       // 0x14: Head of global list of all orders
    int order_list_tail;       // 0x18: Tail of global list of all orders
    // 0x1C: Start of hash buckets. Each bucket is 2 'int's (head and tail pointers for a hash chain).
    int stock_hash_buckets[HASH_BUCKET_COUNT * 2]; // Total size: 0xfb * 8 bytes
    
    // Pool state and next ID counter are placed after the fixed-offset data
    size_t item_size;          // Size of items allocated by pool_alloc
    uint32_t next_id_counter;  // Counter for generating unique order IDs
};

// --- Memory pool implementations using StockExchangeContext ---
void pool_init(StockExchangeContext *ctx, size_t item_size) {
    ctx->item_size = item_size;
    ctx->next_id_counter = 1; // Initialize ID counter
}

int pool_alloc(StockExchangeContext *ctx) {
    void *ptr = malloc(ctx->item_size);
    if (ptr) {
        memset(ptr, 0, ctx->item_size); // Initialize allocated memory to zero
    }
    // Return the address as an int. This is unsafe on 64-bit systems if addresses exceed 2^31-1.
    // Assuming a 32-bit environment or small addresses that fit in an int.
    return (int)(intptr_t)ptr;
}

void pool_free(StockExchangeContext *ctx, int addr) {
    free((void *)(intptr_t)addr);
}

void pool_destroy(StockExchangeContext *ctx) {
    // For this simple malloc-based pool, individual items are freed.
    // No large block to free here.
}


// --- Original functions, with fixes and reduced intermediate variables ---

// Function: get_bucket
// param_1 is StockExchangeContext *
// param_2 is stock_name
int get_bucket(StockExchangeContext *ctx, const char *stock_name) {
    unsigned int hash_idx = strhash(stock_name, 4) % HASH_BUCKET_COUNT;
    // The original calculation `param_1 + (hash_idx + 2) * 8 + 0xc`
    // correctly points to the start of the hash bucket entry within the context structure.
    return (int)(intptr_t)&(ctx->stock_hash_buckets[hash_idx * 2]);
}

// Function: lookup_stock
int lookup_stock(StockExchangeContext *ctx, char *stock_name) {
    int current_stock_ptr = *(int *)get_bucket(ctx, stock_name);
    while (current_stock_ptr != 0) {
        if (strncmp((char *)(intptr_t)(current_stock_ptr + 4), stock_name, 4) == 0) {
            return current_stock_ptr;
        }
        current_stock_ptr = *(int *)(intptr_t)(current_stock_ptr + 0xc); // Next in hash chain (hash_prev field)
    }
    return 0;
}

// Function: remove_stock
void remove_stock(StockExchangeContext *ctx, int stock_ptr) {
    int *bucket_head_ptr = (int *)get_bucket(ctx, (char *)(intptr_t)(stock_ptr + 4));

    // Unlink from hash chain
    if (*(int *)(intptr_t)(stock_ptr + 0x10) == 0) { // If hash_next is null (this is the tail)
        bucket_head_ptr[1] = *(undefined4 *)(intptr_t)(stock_ptr + 0xc); // Update bucket tail to hash_prev
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(stock_ptr + 0x10) + 0xc) = *(undefined4 *)(intptr_t)(stock_ptr + 0xc);
    }

    if (*(int *)(intptr_t)(stock_ptr + 0xc) == 0) { // If hash_prev is null (this is the head)
        *bucket_head_ptr = *(undefined4 *)(intptr_t)(stock_ptr + 0x10); // Update bucket head to hash_next
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(stock_ptr + 0xc) + 0x10) = *(undefined4 *)(intptr_t)(stock_ptr + 0x10);
    }

    // Unlink from global stock list
    if (*(int *)(intptr_t)(stock_ptr + 0x18) == 0) { // If global_list_next is null (this is the tail)
        ctx->stock_list_tail = *(undefined4 *)(intptr_t)(stock_ptr + 0x14);
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(stock_ptr + 0x18) + 0x14) = *(undefined4 *)(intptr_t)(stock_ptr + 0x14);
    }

    if (*(int *)(intptr_t)(stock_ptr + 0x14) == 0) { // If global_list_prev is null (this is the head)
        ctx->stock_list_head = *(undefined4 *)(intptr_t)(stock_ptr + 0x18);
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(stock_ptr + 0x14) + 0x18) = *(undefined4 *)(intptr_t)(stock_ptr + 0x18);
    }
    pool_free(ctx, stock_ptr);
}

// Function: insert_stock
undefined4 insert_stock(StockExchangeContext *ctx, char *stock_name) {
    int new_stock_ptr = pool_alloc(ctx);
    if (new_stock_ptr == 0) {
        return 0xffffffff;
    }

    // Initialize fields of the new stock entry
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x08) = 0; // order count
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x0c) = 0; // hash_prev
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x10) = 0; // hash_next
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x14) = 0; // global_list_prev
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x18) = 0; // global_list_next
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x1c) = 0; // buy_orders_head
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x20) = 0; // buy_orders_tail
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x24) = 0; // sell_orders_head
    *(undefined4 *)(intptr_t)(new_stock_ptr + 0x28) = 0; // sell_orders_tail

    // Link into global stock list (always insert at head based on original logic)
    if (ctx->stock_list_head == 0 && ctx->stock_list_tail == 0) {
        ctx->stock_list_tail = new_stock_ptr;
        ctx->stock_list_head = new_stock_ptr;
    } else {
        *(int *)(intptr_t)(new_stock_ptr + 0x14) = ctx->stock_list_head;
        *(int *)(intptr_t)(ctx->stock_list_head + 0x18) = new_stock_ptr;
        ctx->stock_list_head = new_stock_ptr;
    }

    // Link into hash bucket list (always insert at head based on original logic)
    int *bucket_head_ptr = (int *)get_bucket(ctx, stock_name);
    if (*bucket_head_ptr == 0 && bucket_head_ptr[1] == 0) {
        bucket_head_ptr[1] = new_stock_ptr;
        *bucket_head_ptr = new_stock_ptr;
    } else {
        *(int *)(intptr_t)(new_stock_ptr + 0xc) = *bucket_head_ptr;
        *(int *)(intptr_t)(*bucket_head_ptr + 0x10) = new_stock_ptr;
        *bucket_head_ptr = new_stock_ptr;
    }

    strcpy((char *)(intptr_t)(new_stock_ptr + 4), stock_name);
    return 0;
}

// Function: lookup_order
undefined4 lookup_order(StockExchangeContext *ctx, undefined4 order_id) {
    int current_order_ptr = ctx->order_list_head;
    while (current_order_ptr != 0) {
        if (*(undefined4 *)(intptr_t)(current_order_ptr + 8) == order_id) {
            return (undefined4)current_order_ptr;
        }
        current_order_ptr = *(int *)(intptr_t)(current_order_ptr + 0x14);
    }
    return 0;
}

// Function: remove_order
void remove_order(StockExchangeContext *ctx, int order_ptr) {
    int stock_ptr = *(int *)(intptr_t)(order_ptr + 0x28);

    int *stock_order_list_head_ptr;
    if (*(int *)(intptr_t)(order_ptr + 0x24) == 0) { // BUY order
        stock_order_list_head_ptr = (int *)(intptr_t)(stock_ptr + 0x1c); // buy_orders_head
    } else { // SELL order
        stock_order_list_head_ptr = (int *)(intptr_t)(stock_ptr + 0x24); // sell_orders_head
    }

    // Unlink from global order list
    if (*(int *)(intptr_t)(order_ptr + 0x18) == 0) { // If global_list_next is null (this is the tail)
        ctx->order_list_tail = *(undefined4 *)(intptr_t)(order_ptr + 0x14);
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(order_ptr + 0x18) + 0x14) = *(undefined4 *)(intptr_t)(order_ptr + 0x14);
    }
    if (*(int *)(intptr_t)(order_ptr + 0x14) == 0) { // If global_list_prev is null (this is the head)
        ctx->order_list_head = *(undefined4 *)(intptr_t)(order_ptr + 0x18);
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(order_ptr + 0x14) + 0x18) = *(undefined4 *)(intptr_t)(order_ptr + 0x18);
    }

    // Unlink from stock's order list
    if (*(int *)(intptr_t)(order_ptr + 0x20) == 0) { // If order_next is null (this is the tail)
        stock_order_list_head_ptr[1] = *(undefined4 *)(intptr_t)(order_ptr + 0x1c); // Update stock's list tail
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(order_ptr + 0x20) + 0x1c) = *(undefined4 *)(intptr_t)(order_ptr + 0x1c);
    }
    if (*(int *)(intptr_t)(order_ptr + 0x1c) == 0) { // If order_prev is null (this is the head)
        *stock_order_list_head_ptr = *(undefined4 *)(intptr_t)(order_ptr + 0x20); // Update stock's list head
    } else {
        *(undefined4 *)(intptr_t)(*(int *)(intptr_t)(order_ptr + 0x1c) + 0x20) = *(undefined4 *)(intptr_t)(order_ptr + 0x20);
    }

    *(int *)(intptr_t)(stock_ptr + 8) -= 1; // Decrement order count for the parent stock

    if (*(int *)(intptr_t)(stock_ptr + 0x1c) == 0 && *(int *)(intptr_t)(stock_ptr + 0x24) == 0) {
        remove_stock(ctx, stock_ptr);
    }

    if (*(int *)(intptr_t)(order_ptr + 0x10) == 0) { // If quantity is 0, call the completion callback
        void (*on_complete_cb)(int) = (void (*)(int))(intptr_t)*(int *)(intptr_t)(order_ptr + 4);
        if (on_complete_cb) {
            on_complete_cb(order_ptr);
        }
    }
    pool_free(ctx, order_ptr);
}

// Function: order_cmp
int order_cmp(int order_ptr1, int order_ptr2) {
    if (*(int *)(intptr_t)(order_ptr1 + 0xc) == *(int *)(intptr_t)(order_ptr2 + 0xc)) { // Price comparison
        if (*(int *)(intptr_t)(order_ptr1 + 0x24) == 0) { // If order_ptr1 is a BUY order (type 0)
            return *(int *)(intptr_t)(order_ptr2 + 8) - *(int *)(intptr_t)(order_ptr1 + 8); // Compare IDs (descending)
        } else { // SELL order
            return *(int *)(intptr_t)(order_ptr1 + 8) - *(int *)(intptr_t)(order_ptr2 + 8); // Compare IDs (ascending)
        }
    } else {
        return *(int *)(intptr_t)(order_ptr2 + 0xc) - *(int *)(intptr_t)(order_ptr1 + 0xc); // Compare prices (descending)
    }
}

// Function: match_order
undefined4 match_order(StockExchangeContext *ctx, int stock_ptr, int new_order_ptr) {
    uint32_t matched_qty;
    int current_order_ptr;

    if (*(int *)(intptr_t)(new_order_ptr + 0x24) == 0) { // If new_order is a BUY order (type 0)
        // Iterate through SELL orders (stock_ptr + 0x24 is sell_orders_head, 0x20 is next)
        for (current_order_ptr = *(int *)(intptr_t)(stock_ptr + 0x24); current_order_ptr != 0; current_order_ptr = *(int *)(intptr_t)(current_order_ptr + 0x20)) {
            if (*(uint32_t *)(intptr_t)(current_order_ptr + 0xc) <= *(uint32_t *)(intptr_t)(new_order_ptr + 0xc)) { // If sell price <= buy price
                matched_qty = *(uint32_t *)(intptr_t)(new_order_ptr + 0x10);
                if (*(uint32_t *)(intptr_t)(current_order_ptr + 0x10) < matched_qty) {
                    matched_qty = *(uint32_t *)(intptr_t)(current_order_ptr + 0x10);
                }
                *(uint32_t *)(intptr_t)(new_order_ptr + 0x10) -= matched_qty;
                *(uint32_t *)(intptr_t)(current_order_ptr + 0x10) -= matched_qty;

                if (*(int *)(intptr_t)(current_order_ptr + 0x10) == 0) {
                    remove_order(ctx, current_order_ptr);
                }
                if (*(int *)(intptr_t)(new_order_ptr + 0x10) == 0) {
                    remove_order(ctx, new_order_ptr);
                    break;
                }
            }
        }
    } else { // If new_order is a SELL order (type 1)
        // Iterate through BUY orders (stock_ptr + 0x1c is buy_orders_head, 0x20 is next)
        for (current_order_ptr = *(int *)(intptr_t)(stock_ptr + 0x1c); current_order_ptr != 0; current_order_ptr = *(int *)(intptr_t)(current_order_ptr + 0x20)) {
            if (*(uint32_t *)(intptr_t)(new_order_ptr + 0xc) <= *(uint32_t *)(intptr_t)(current_order_ptr + 0xc)) { // If sell price <= buy price
                matched_qty = *(uint32_t *)(intptr_t)(new_order_ptr + 0x10);
                if (*(uint32_t *)(intptr_t)(current_order_ptr + 0x10) < matched_qty) {
                    matched_qty = *(uint32_t *)(intptr_t)(current_order_ptr + 0x10);
                }
                *(uint32_t *)(intptr_t)(new_order_ptr + 0x10) -= matched_qty;
                *(uint32_t *)(intptr_t)(current_order_ptr + 0x10) -= matched_qty;

                if (*(int *)(intptr_t)(current_order_ptr + 0x10) == 0) {
                    remove_order(ctx, current_order_ptr);
                }
                if (*(int *)(intptr_t)(new_order_ptr + 0x10) == 0) {
                    remove_order(ctx, new_order_ptr);
                    break;
                }
            }
        }
    }
    return (*(int *)(intptr_t)(new_order_ptr + 0x10) == 0) ? 0 : 0xffffffff;
}

// Function: get_next_id
undefined4 get_next_id(StockExchangeContext *ctx, undefined4 order_ptr_as_id) {
    (void)order_ptr_as_id; // Parameter not used as we generate a sequential ID
    return ctx->next_id_counter++;
}

// Function: insert_order
undefined4 insert_order(StockExchangeContext *ctx, char *stock_name, int buy_sell_type, undefined4 quantity, undefined4 price) {
    if (strnlen(stock_name, 4) == 0) {
        return 0xffffffff;
    }

    int stock_ptr = lookup_stock(ctx, stock_name);
    if (stock_ptr == 0) {
        if (insert_stock(ctx, stock_name) == 0xffffffff) {
            return 0xffffffff;
        }
        stock_ptr = lookup_stock(ctx, stock_name);
        if (stock_ptr == 0) {
            return 0xffffffff;
        }
    }

    int new_order_ptr = pool_alloc(ctx);
    if (new_order_ptr == 0) {
        return 0xffffffff;
    }

    *(undefined4 *)(intptr_t)(new_order_ptr + 8) = get_next_id(ctx, new_order_ptr);
    *(int *)(intptr_t)(new_order_ptr + 0x24) = buy_sell_type;
    *(undefined4 *)(intptr_t)(new_order_ptr + 0x10) = quantity;
    *(undefined4 *)(intptr_t)(new_order_ptr + 0xc) = price;
    *(int *)(intptr_t)(new_order_ptr + 0x28) = stock_ptr;
    *(void (**)(int))(intptr_t)(new_order_ptr + 4) = on_complete_dummy;

    *(undefined4 *)(intptr_t)(new_order_ptr + 0x18) = 0; // Global list next
    *(undefined4 *)(intptr_t)(new_order_ptr + 0x14) = 0; // Global list prev
    *(undefined4 *)(intptr_t)(new_order_ptr + 0x20) = 0; // Stock's order list next
    *(undefined4 *)(intptr_t)(new_order_ptr + 0x1c) = 0; // Stock's order list prev

    int *stock_order_list_head_ptr;
    int *stock_order_list_tail_ptr;

    if (buy_sell_type == 0) { // BUY order
        stock_order_list_head_ptr = (int *)(intptr_t)(stock_ptr + 0x1c); // buy_orders_head
        stock_order_list_tail_ptr = (int *)(intptr_t)(stock_ptr + 0x20); // buy_orders_tail
    } else { // SELL order
        stock_order_list_head_ptr = (int *)(intptr_t)(stock_ptr + 0x24); // sell_orders_head
        stock_order_list_tail_ptr = (int *)(intptr_t)(stock_ptr + 0x28); // sell_orders_tail
    }

    int current_order_in_stock_list = *stock_order_list_head_ptr;
    int prev_order_in_stock_list = 0;

    while (current_order_in_stock_list != 0 && order_cmp(current_order_in_stock_list, new_order_ptr) < 0) {
        prev_order_in_stock_list = current_order_in_stock_list;
        current_order_in_stock_list = *(int *)(intptr_t)(current_order_in_stock_list + 0x20);
    }

    if (prev_order_in_stock_list == 0) { // Insert at head
        if (*stock_order_list_head_ptr != 0) {
            *(int *)(intptr_t)(*stock_order_list_head_ptr + 0x1c) = new_order_ptr;
        } else {
            *stock_order_list_tail_ptr = new_order_ptr;
        }
        *(int *)(intptr_t)(new_order_ptr + 0x20) = *stock_order_list_head_ptr;
        *stock_order_list_head_ptr = new_order_ptr;
    } else { // Insert between prev and current
        *(int *)(intptr_t)(new_order_ptr + 0x20) = current_order_in_stock_list;
        *(int *)(intptr_t)(new_order_ptr + 0x1c) = prev_order_in_stock_list;
        if (current_order_in_stock_list != 0) {
            *(int *)(intptr_t)(current_order_in_stock_list + 0x1c) = new_order_ptr;
        } else {
            *stock_order_list_tail_ptr = new_order_ptr;
        }
        *(int *)(intptr_t)(prev_order_in_stock_list + 0x20) = new_order_ptr;
    }

    // Insert into global order list (always insert at head based on original logic)
    if (ctx->order_list_head == 0 && ctx->order_list_tail == 0) {
        ctx->order_list_tail = new_order_ptr;
        ctx->order_list_head = new_order_ptr;
    } else {
        *(int *)(intptr_t)(new_order_ptr + 0x14) = ctx->order_list_head;
        *(int *)(intptr_t)(ctx->order_list_head + 0x18) = new_order_ptr;
        ctx->order_list_head = new_order_ptr;
    }

    *(int *)(intptr_t)(stock_ptr + 8) += 1; // Increment order count for stock

    if (match_order(ctx, stock_ptr, new_order_ptr) == 0) {
        return 0;
    } else {
        return *(undefined4 *)(intptr_t)(new_order_ptr + 8);
    }
}

// Function: order_to_str
void order_to_str(int order_ptr, char *buffer) {
    char temp_buf[20];

    buffer[0] = '\0';

    my_itoa(*(undefined4 *)(intptr_t)(order_ptr + 8), temp_buf);
    strcpy(buffer, temp_buf);

    if (*(int *)(intptr_t)(order_ptr + 0x24) == 0) { // BUY order
        strcat(buffer, "\tBUY\t");
        my_itoa(*(undefined4 *)(intptr_t)(order_ptr + 0x10), temp_buf); // Quantity
        strcat(buffer, temp_buf);
        strcat(buffer, "\t");
        my_itoa(*(undefined4 *)(intptr_t)(order_ptr + 0xc), temp_buf); // Price
        strcat(buffer, temp_buf);
        strcat(buffer, "\t\t\t\n");
    } else { // SELL order
        strcat(buffer, "\t\t\t");
        my_itoa(*(undefined4 *)(intptr_t)(order_ptr + 0xc), temp_buf); // Price
        strcat(buffer, temp_buf);
        strcat(buffer, "\t");
        my_itoa(*(undefined4 *)(intptr_t)(order_ptr + 0x10), temp_buf); // Quantity
        strcat(buffer, temp_buf);
        strcat(buffer, "\tSELL\n");
    }
}

// Function: cmd_list_stocks
undefined4 cmd_list_stocks(StockExchangeContext *ctx) {
    // Cleanup empty stocks first
    int current_stock_ptr = ctx->stock_list_head;
    int next_stock_ptr;
    int prev_stock_ptr = 0;

    while (current_stock_ptr != 0) {
        next_stock_ptr = *(int *)(intptr_t)(current_stock_ptr + 0x14); // Next in global list (0x14 is next)

        if (*(int *)(intptr_t)(current_stock_ptr + 0x1c) == 0 && *(int *)(intptr_t)(current_stock_ptr + 0x24) == 0) {
            remove_stock(ctx, current_stock_ptr);
        } else {
            if (*(int *)(intptr_t)(current_stock_ptr + 0x18) != prev_stock_ptr) {
                return 0xffffffff; // List integrity check failed
            }
            prev_stock_ptr = current_stock_ptr;
        }
        current_stock_ptr = next_stock_ptr;
    }

    char stock_name_buf[5];
    size_t bytes_to_write;
    for (current_stock_ptr = ctx->stock_list_head; current_stock_ptr != 0; current_stock_ptr = *(int *)(intptr_t)(current_stock_ptr + 0x14)) {
        size_t name_len = strnlen((char *)(intptr_t)(current_stock_ptr + 4), 4);
        strncpy(stock_name_buf, (char *)(intptr_t)(current_stock_ptr + 4), name_len);
        stock_name_buf[name_len] = '\n';
        bytes_to_write = name_len + 1;

        if (write_all(1, stock_name_buf, bytes_to_write) != (ssize_t)bytes_to_write) {
            return 0xffffffff;
        }
    }
    return 0;
}

// Function: cmd_list_orders
undefined4 cmd_list_orders(StockExchangeContext *ctx, char *stock_name) {
    char output_buf[200];
    size_t bytes_to_write;

    int stock_ptr = lookup_stock(ctx, stock_name);
    if (stock_ptr == 0) {
        return 0xffffffff;
    }

    strcpy(output_buf, "Order book for ");
    strncat(output_buf, (char *)(intptr_t)(stock_ptr + 4), 4);
    strcat(output_buf, "\nID\tSIDE\tQTY\tPRICE\tQTY\tSIDE\n");
    bytes_to_write = strlen(output_buf);

    if (write_all(1, output_buf, bytes_to_write) != (ssize_t)bytes_to_write) {
        return 0xffffffff;
    }

    // List SELL orders (stock_ptr + 0x24 is sell_orders_head, 0x20 is next)
    for (int order_ptr = *(int *)(intptr_t)(stock_ptr + 0x24); order_ptr != 0; order_ptr = *(int *)(intptr_t)(order_ptr + 0x20)) {
        order_to_str(order_ptr, output_buf);
        bytes_to_write = strlen(output_buf);
        if (write_all(1, output_buf, bytes_to_write) != (ssize_t)bytes_to_write) {
            return 0xffffffff;
        }
    }

    // List BUY orders (stock_ptr + 0x1c is buy_orders_head, 0x20 is next)
    for (int order_ptr = *(int *)(intptr_t)(stock_ptr + 0x1c); order_ptr != 0; order_ptr = *(int *)(intptr_t)(order_ptr + 0x20)) {
        order_to_str(order_ptr, output_buf);
        bytes_to_write = strlen(output_buf);
        if (write_all(1, output_buf, bytes_to_write) != (ssize_t)bytes_to_write) {
            return 0xffffffff;
        }
    }
    return 0;
}

// Function: cmd_place_order
void cmd_place_order(StockExchangeContext *ctx, char *stock_name, int buy_sell_type, undefined4 quantity, undefined4 price) {
    insert_order(ctx, stock_name, buy_sell_type != 0, quantity, price);
}

// Function: cmd_check_order
undefined4 cmd_check_order(StockExchangeContext *ctx, undefined4 order_id) {
    char output_buf[200];
    size_t bytes_to_write;

    int order_ptr = lookup_order(ctx, order_id);
    if (order_ptr == 0) {
        return 0xffffffff;
    }

    order_to_str(order_ptr, output_buf);
    bytes_to_write = strlen(output_buf);
    if (write_all(1, output_buf, bytes_to_write) != (ssize_t)bytes_to_write) {
        return 0xffffffff;
    }
    return 0;
}

// Function: cmd_cancel_order
undefined4 cmd_cancel_order(StockExchangeContext *ctx, int order_id) {
    int order_ptr = lookup_order(ctx, order_id);
    if (order_ptr == 0) {
        return 0xffffffff;
    } else {
        remove_order(ctx, order_ptr);
        return 0;
    }
}

// Function: stock_init
void stock_init(StockExchangeContext *ctx) {
    pool_init(ctx, 0x2c); // Initialize pool with item size 0x2c (44 bytes)

    ctx->stock_list_head = 0;
    ctx->stock_list_tail = 0;
    ctx->order_list_head = 0;
    ctx->order_list_tail = 0;

    for (uint32_t i = 0; i < HASH_BUCKET_COUNT * 2; ++i) {
        ctx->stock_hash_buckets[i] = 0;
    }
}

// Function: stock_destroy
void stock_destroy(StockExchangeContext *ctx) {
    int current_order_ptr = ctx->order_list_head;
    int next_order_ptr;

    while (current_order_ptr != 0) {
        next_order_ptr = *(int *)(intptr_t)(current_order_ptr + 0x14); // Next in global order list
        remove_order(ctx, current_order_ptr); // remove_order also frees the memory
        current_order_ptr = next_order_ptr;
    }
    pool_destroy(ctx);
}

// Main function to make it a compilable program
int main() {
    // Allocate and initialize the StockExchangeContext
    StockExchangeContext *ctx = (StockExchangeContext *)malloc(sizeof(StockExchangeContext));
    if (!ctx) {
        perror("Failed to allocate StockExchangeContext");
        return 1;
    }
    memset(ctx, 0, sizeof(StockExchangeContext)); // Ensure all members are zero-initialized

    stock_init(ctx);

    printf("--- Inserting Stocks ---\n");
    insert_stock(ctx, "AAPL");
    insert_stock(ctx, "GOOG");
    insert_stock(ctx, "TSLA");
    printf("Stocks after insertion:\n");
    cmd_list_stocks(ctx);

    printf("\n--- Placing Orders ---\n");
    undefined4 order_id_1 = insert_order(ctx, "AAPL", 0, 10, 150); // Buy 10 AAPL @ 150
    undefined4 order_id_2 = insert_order(ctx, "AAPL", 1, 5, 155);  // Sell 5 AAPL @ 155
    undefined4 order_id_3 = insert_order(ctx, "GOOG", 0, 20, 1000); // Buy 20 GOOG @ 1000
    undefined4 order_id_4 = insert_order(ctx, "AAPL", 0, 15, 152); // Buy 15 AAPL @ 152
    undefined4 order_id_5 = insert_order(ctx, "AAPL", 1, 10, 150); // Sell 10 AAPL @ 150 (should match existing buy order)
    undefined4 order_id_6 = insert_order(ctx, "TSLA", 0, 5, 700); // Buy 5 TSLA @ 700

    printf("Orders for AAPL after placement:\n");
    cmd_list_orders(ctx, "AAPL");
    printf("Orders for GOOG after placement:\n");
    cmd_list_orders(ctx, "GOOG");

    printf("\n--- Checking Order ID %u ---\n", order_id_1);
    cmd_check_order(ctx, order_id_1);

    printf("\n--- Cancelling Order ID %u ---\n", order_id_3);
    cmd_cancel_order(ctx, order_id_3);
    printf("Orders for GOOG after cancel:\n");
    cmd_list_orders(ctx, "GOOG");
    
    printf("\n--- Final Orders for AAPL ---\n");
    cmd_list_orders(ctx, "AAPL");

    stock_destroy(ctx);
    free(ctx); // Free the context itself
    return 0;
}