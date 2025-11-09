#include <stdio.h>    // For sprintf, vfprintf, NULL, stderr
#include <stdlib.h>   // For realloc, malloc, free, strtoul
#include <string.h>   // For strlen, strchr, strdup, strcasecmp, memset
#include <ctype.h>    // For isxdigit, isdigit, tolower
#include <stdarg.h>   // For va_list in fdprintf
#include <unistd.h>   // For STDERR_FILENO

// --- Custom Type Definitions ---

// AppContext structure to hold all global state and pointers to context memory
typedef struct {
    void* command_ht_ctx_ptr; // Pointer to raw memory for command hash table
    void* product_ht_ctx_ptr; // Pointer to raw memory for product hash table
    void* silk_recv_ctx_ptr;  // Pointer to raw memory for silk receive context

    int silk_send_handle;      // Handle for silk_send operations
    int total_product_count;   // Counter for products, used by do_list
} AppContext;

// Product structure based on usage in do_sell and do_buy
typedef struct {
    char* name;
    char* description;
    int price;
    int quantity;
} Product;

// --- Function Prototypes (assuming these are external or custom implementations) ---

// silk_recv expects the raw context memory
int silk_recv(void* silk_recv_context_memory, char* buffer, size_t count, unsigned int flags);
// silk_send expects an integer handle
int silk_send(int silk_send_handle, const char* buffer, size_t count);
// ht_init expects the raw context memory
int ht_init(void* ht_context_memory);
int ht_lookup(void* ht_context_memory, const char* key, void** value_out);
int ht_insert(void* ht_context_memory, const char* key, void* value);
void* ht_node_value(void* node);
int ht_delete(void* ht_context_memory, const char* key, void* value);
void* ht_first(void* ht_context_memory);
void* ht_next(void* ht_context_memory, void* current_node);
// silk_init expects the raw context memory
int silk_init(void* silk_context_memory);
// silk_prepare takes the silk_recv_context_memory and returns the silk_send_handle
int silk_prepare(void* silk_context_memory);

// --- Global String Literals (replacing DAT_0001e9xx) ---

const char* const DAT_0001e921 = "\n";
const char* const DAT_0001e98b = "buy";
const char* const DAT_0001e98f = "sell";
const char* const DAT_0001e994 = "list";
const char* const DAT_0001e999 = "quit";

// --- Custom fdprintf for stderr (file descriptor 2) ---
void fdprintf(int fd, const char* format, ...) {
    if (fd == STDERR_FILENO) { // Assuming 2 means stderr
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }
}

// --- Function: read_line ---
char* read_line(void* silk_recv_context_memory) {
    char* buffer = NULL;
    size_t current_capacity = 0;
    size_t bytes_read = 0; // Number of bytes successfully read into buffer
    unsigned int recv_flags = 0x17d62; // Original uVar5

    while (1) {
        if (bytes_read == current_capacity) {
            if (current_capacity >= 0xFFFF) { // Check for overflow before adding 0x400
                free(buffer);
                return NULL;
            }
            size_t new_capacity = current_capacity + 0x400; // Increase by 1024 bytes
            char* new_buffer = (char*)realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
            current_capacity = new_capacity;
        }

        // Read one byte into buffer[bytes_read]
        if (silk_recv(silk_recv_context_memory, buffer + bytes_read, 1, recv_flags) != 0) {
            free(buffer);
            return NULL;
        }

        // Check the character just read
        if (buffer[bytes_read] == '\b') {
            bytes_read++; // Include the '\b' in the count and exit
            break;
        }
        bytes_read++;
    }

    // After loop, bytes_read contains the total length including the final '\b'.
    // Now, null-terminate the string.
    // If current_capacity is exactly bytes_read, we need one more byte for '\0'.
    if (bytes_read == current_capacity) {
        char* new_buffer = (char*)realloc(buffer, current_capacity + 1);
        if (new_buffer == NULL) {
            free(buffer);
            return NULL;
        }
        buffer = new_buffer;
    }
    buffer[bytes_read] = '\0'; // Null-terminate after the '\b'

    return buffer;
}

// --- Function: parse_request ---
int parse_request(char* request_line, char** command_out, char** arg_out) {
    char* tab_pos = strchr(request_line, '\t');
    if (tab_pos == NULL) {
        char* bs_pos = strchr(request_line, '\b');
        if (bs_pos == NULL) {
            return 1; // Error: no tab or backspace separator
        } else {
            *bs_pos = '\0';
            *command_out = request_line;
            *arg_out = NULL;
            return 0; // Success with only command
        }
    } else {
        *tab_pos = '\0';
        *command_out = request_line;
        char* bs_pos = strchr(tab_pos + 1, '\b');
        if (bs_pos != NULL) {
            *bs_pos = '\0';
        }
        *arg_out = tab_pos + 1;
        return 0; // Success with command and argument
    }
}

// --- Function: free_product ---
void free_product(void** product_data_ptr) {
    if (product_data_ptr == NULL) return; // Defensive check
    Product* product = (Product*)product_data_ptr;
    free(product->description); // Free the description string
    free(product->name);       // Free the name string
    free(product);             // Free the Product structure itself
}

// --- Function: send_response ---
int send_response(AppContext* ctx, int response_code, const char* message) {
    size_t msg_len = strlen(message);
    int result;

    // 0x40 is an arbitrary buffer for code, tabs, null, etc.
    // 1024 (0x400) is the stack buffer size
    if (msg_len + 0x40 < 1024) {
        char buffer[1024];
        sprintf(buffer, "%d\t%s\b", response_code, message);
        result = silk_send(ctx->silk_send_handle, buffer, strlen(buffer));
    } else {
        char* dynamic_buffer = (char*)malloc(msg_len + 0x40);
        fdprintf(STDERR_FILENO, "WARNING text is too long: ");
        fdprintf(STDERR_FILENO, "%s", message);
        fdprintf(STDERR_FILENO, "%s", DAT_0001e921); // Newline

        if (dynamic_buffer == NULL) {
            result = 1; // Allocation error
        } else {
            sprintf(dynamic_buffer, "%d\t%s\b", response_code, message);
            result = silk_send(ctx->silk_send_handle, dynamic_buffer, strlen(dynamic_buffer));
            free(dynamic_buffer);
        }
    }
    return result;
}

// --- Function: unescape ---
void unescape(char* str) {
    size_t len = strlen(str);
    size_t read_idx = 0;
    size_t write_idx = 0;

    while (read_idx < len) {
        if (str[read_idx] == '%' && read_idx + 2 < len) {
            int h1 = str[read_idx + 1];
            int h2 = str[read_idx + 2];

            if (isxdigit(h1) && isxdigit(h2)) {
                // Convert hex chars to byte value
                int val1 = isdigit(h1) ? h1 - '0' : tolower(h1) - 'a' + 10;
                int val2 = isdigit(h2) ? h2 - '0' : tolower(h2) - 'a' + 10;
                str[write_idx] = (unsigned char)((val1 << 4) | val2);
                read_idx += 2; // Consume the two hex digits
            } else {
                // Not a valid hex escape, copy '%' literally
                str[write_idx] = str[read_idx];
            }
        } else {
            // Not a '%' or not enough chars for escape, copy character literally
            str[write_idx] = str[read_idx];
        }
        read_idx++;
        write_idx++;
    }
    str[write_idx] = '\0'; // Null-terminate the new string
}

// --- Function: do_buy ---
void do_buy(AppContext* ctx, char* product_name) {
    unescape(product_name);

    void* node_value_ptr; // Will point to Product*
    if (ht_lookup(ctx->product_ht_ctx_ptr, product_name, &node_value_ptr) == 0) {
        Product* product = (Product*)node_value_ptr;
        if (product->quantity == 0) {
            send_response(ctx, 0x2002, "Name not found"); // Original code uses this error code for quantity 0
        } else {
            product->quantity--;
            if (product->quantity == 0) {
                ht_delete(ctx->product_ht_ctx_ptr, product_name, product);
                free_product((void**)product); // Free product data and product struct
                ctx->total_product_count--;
            }
            send_response(ctx, 0x1000, "Success");
        }
    } else {
        send_response(ctx, 0x2002, "Name not found");
    }
}

// --- Function: do_sell ---
void do_sell(AppContext* ctx, char* request_string) {
    char* name_str = request_string;
    char* desc_str = NULL;
    int price = 0;
    int quantity = 0;

    char* semicolon1 = strchr(name_str, ';');
    if (semicolon1 == NULL) {
        send_response(ctx, 0x2003, "Invalid request");
        return;
    }
    *semicolon1 = '\0';
    desc_str = semicolon1 + 1;

    char* semicolon2 = strchr(desc_str, ';');
    if (semicolon2 == NULL) {
        send_response(ctx, 0x2003, "Invalid request");
        return;
    }
    *semicolon2 = '\0';
    char* price_str = semicolon2 + 1;

    char* semicolon3 = strchr(price_str, ';');
    if (semicolon3 == NULL) {
        send_response(ctx, 0x2003, "Invalid request");
        return;
    }
    *semicolon3 = '\0';
    char* quantity_str = semicolon3 + 1;

    price = (int)strtoul(price_str, NULL, 10);
    quantity = (int)strtoul(quantity_str, NULL, 10);

    unescape(desc_str);
    unescape(name_str);

    void* node_value_ptr;
    if (ht_lookup(ctx->product_ht_ctx_ptr, name_str, &node_value_ptr) != 0) { // Product not found, create new
        Product* new_product = (Product*)malloc(sizeof(Product));
        if (new_product == NULL) {
            send_response(ctx, 0xA000, "Internal error");
            return;
        }

        new_product->name = strdup(name_str);
        new_product->description = strdup(desc_str);
        new_product->price = price;
        new_product->quantity = quantity;

        if (new_product->name == NULL || new_product->description == NULL) {
            free_product((void**)new_product);
            send_response(ctx, 0xA000, "Internal error");
            return;
        }

        if (ht_insert(ctx->product_ht_ctx_ptr, new_product->name, new_product) == 0) {
            send_response(ctx, 0x1000, "Success");
            ctx->total_product_count++;
            return;
        } else {
            free_product((void**)new_product);
            send_response(ctx, 0xA000, "Internal error");
            return;
        }
    } else { // Product found, update existing
        Product* existing_product = (Product*)node_value_ptr;
        if (strcasecmp(existing_product->description, desc_str) != 0) {
            send_response(ctx, 0x2001, "Name already in-use");
            return;
        }
        existing_product->price = price;
        existing_product->quantity += quantity;
        send_response(ctx, 0x1001, "Record updated");
        return;
    }
}

// --- Function: do_list ---
int do_list(AppContext* ctx) {
    char count_buffer[64];
    sprintf(count_buffer, "%d", ctx->total_product_count);

    if (send_response(ctx, 0x1000, count_buffer) != 0) {
        return 1; // Error sending initial count
    }

    char* dynamic_list_buffer = NULL;
    size_t current_buffer_capacity = 0;
    void* ht_node = NULL; // Use void* for hash table node handle

    for (ht_node = ht_first(ctx->product_ht_ctx_ptr); ht_node != NULL; ht_node = ht_next(ctx->product_ht_ctx_ptr, ht_node)) {
        Product* product = (Product*)ht_node_value(ht_node);
        size_t name_len = strlen(product->name);
        size_t desc_len = strlen(product->description);
        // +64 for ";", ";", price, ";", quantity (converted to string), null terminator, and some margin
        size_t required_len = name_len + desc_len + 64; 

        if (current_buffer_capacity < required_len) {
            char* new_buffer = (char*)realloc(dynamic_list_buffer, required_len);
            if (new_buffer == NULL) {
                free(dynamic_list_buffer);
                send_response(ctx, 0xA000, "Internal error"); // Realloc failed
                return 1;
            }
            dynamic_list_buffer = new_buffer;
            current_buffer_capacity = required_len;
        }

        sprintf(dynamic_list_buffer, "%s;%s;%d;%d", product->name, product->description, product->price, product->quantity);
        if (send_response(ctx, 0x1002, dynamic_list_buffer) != 0) {
            free(dynamic_list_buffer);
            send_response(ctx, 0xA000, "Internal error"); // Send response failed
            return 1;
        }
    }

    free(dynamic_list_buffer);
    return 0; // Success
}

// --- Function: do_quit ---
int do_quit(AppContext* ctx) {
    send_response(ctx, 0x1000, "Success");
    return 1; // Return 1 to signal main loop to terminate
}

// --- Function: main ---
int main(void) {
    // Raw memory for hash table contexts and silk receive context (on stack)
    char command_ht_mem[16];
    char product_ht_mem[16];
    char silk_recv_mem[4];

    AppContext app_ctx;
    memset(&app_ctx, 0, sizeof(AppContext)); // Initialize AppContext

    // Assign pointers to the raw memory blocks
    app_ctx.command_ht_ctx_ptr = command_ht_mem;
    app_ctx.product_ht_ctx_ptr = product_ht_mem;
    app_ctx.silk_recv_ctx_ptr = silk_recv_mem;

    // Initialize hash tables
    if (ht_init(app_ctx.product_ht_ctx_ptr) != 0) return 1;
    if (ht_init(app_ctx.command_ht_ctx_ptr) != 0) return 1;

    // Populate command hash table with function pointers
    if (ht_insert(app_ctx.command_ht_ctx_ptr, DAT_0001e98b, (void*)do_buy) != 0) return 1;
    if (ht_insert(app_ctx.command_ht_ctx_ptr, DAT_0001e98f, (void*)do_sell) != 0) return 1;
    if (ht_insert(app_ctx.command_ht_ctx_ptr, DAT_0001e994, (void*)do_list) != 0) return 1;
    if (ht_insert(app_ctx.command_ht_ctx_ptr, DAT_0001e999, (void*)do_quit) != 0) return 1;

    // Initialize silk communication
    if (silk_init(app_ctx.silk_recv_ctx_ptr) != 0) return 1;
    app_ctx.silk_send_handle = silk_prepare(app_ctx.silk_recv_ctx_ptr);
    if (app_ctx.silk_send_handle == 0) return 1; // Assuming 0 means error for silk_prepare return

    // Main loop
    char* line_buffer;
    int loop_result = 0; // 0 to continue, 1 to quit

    while ((line_buffer = read_line(app_ctx.silk_recv_ctx_ptr)) != NULL) {
        char* command_str = NULL;
        char* arg_str = NULL;

        if (parse_request(line_buffer, &command_str, &arg_str) == 0) {
            void* command_func_ptr; // Will point to handler function pointer
            if (ht_lookup(app_ctx.command_ht_ctx_ptr, command_str, &command_func_ptr) == 0) {
                // Call the appropriate handler function
                // Handlers take AppContext* and char* arg_str
                int (*handler)(AppContext*, char*) = (int (*)(AppContext*, char*))command_func_ptr;
                loop_result = handler(&app_ctx, arg_str);
            } else {
                send_response(&app_ctx, 0x2000, "Unknown command"); // Custom error for unknown command
            }
        } else {
            send_response(&app_ctx, 0x2003, "Invalid request format"); // Custom error for parse_request failure
        }

        free(line_buffer);
        if (loop_result != 0) { // If handler indicates to quit (do_quit returns 1)
            break;
        }
    }

    // Cleanup (e.g., free hash tables, close silk connection) - not in original snippet, so omitted specific calls.
    // In a real application, you'd iterate and free all products, then destroy hash tables.

    return 0; // Success
}