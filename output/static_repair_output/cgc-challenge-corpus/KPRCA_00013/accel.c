#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   // For pow, round, atof
#include <ctype.h>  // For toupper, isalpha, isdigit
#include <stdbool.h> // For bool type

// Define ROUND as it's not standard C, but round() is in math.h
#define ROUND(x) round(x)

// Cell structure based on usage in the original snippet
typedef struct Cell {
    int id;           // Offset 0: Unique ID for the cell
    int type;         // Offset 4: 0=empty, 1=error, 2=string, 3=string (quoted), 4=number, 5=operator, 6=cell_ref, 7=formula
    char* data_ptr;   // Offset 8: Points to actual string/number data or string representation
    char* formula_ptr; // Offset 12: Points to formula string if type is 7 (points to data_ptr + 1)
} Cell;

// Global variables for the spreadsheet
#define NUM_ROWS 702 // 0x2be rows
#define NUM_COLS 100 // 100 columns per row block
Cell** g_sheet = NULL;
void* _g_sheet = NULL; // Raw memory block for all cells

// Placeholder for DAT_00017082 and other magic strings/constants
const char* DAT_00017082 = "("; // Used in infixtorpn for parenthesis
const double DAT_00017148 = 26.0; // Base for column conversion (A=1, B=2, ..., Z=26)
const char* DAT_00017077 = "0"; // Used in eval_formula to push "0" for empty cells

// Forward declarations for functions
// (These are minimal declarations to allow compilation, actual implementations might be external)
int itoa(int value, char* buffer, int buffer_size);
int ftoa(double value, char* buffer, int buffer_size);
Cell* get_cell(char* cell_id_str); // Takes cell ID as string (e.g., "A1")
int get_rowcol(const char* cell_id_str, char* col_chars, char* row_digits);
int parsearg(char* arg_str);
int valid_cell_id(char* cell_id_str);
char** get_op(char* op_str); // Returns char** to an array of {op_name, op_func_ptr}
int sanitize_formula(char* formula_str, size_t buffer_size);
int is_arg_arithmetic(void* arg);
long double eval_function(char** op_entry, int operand_stack_handle, char* result_buffer, int result_buffer_size);
long double eval_formula(char* formula_str, int* error_flag, int* circular_ref_stack_handle, int current_cell_id);
int infixtorpn(char* infix_expr, size_t buffer_size);


// --- Generic Stack and Queue Implementations ---
// Using simple array-based approach for demonstration.
// In a real system, these would be linked lists or dynamic arrays.
#define MAX_STACK_SIZE 256
#define MAX_QUEUE_SIZE 256

typedef struct {
    char* items[MAX_STACK_SIZE];
    int top;
} GenericStack;

typedef struct {
    char* items[MAX_QUEUE_SIZE];
    int head;
    int tail;
    int count;
} GenericQueue;

// Global stacks/queues, indexed by an int handle
// This is a simplification; a real system would manage these handles dynamically.
GenericStack global_stacks[5]; // Max 5 stacks
GenericQueue global_queues[5]; // Max 5 queues

int init_global_structures() {
    for (int i = 0; i < 5; ++i) {
        global_stacks[i].top = -1;
        global_queues[i].head = 0;
        global_queues[i].tail = 0;
        global_queues[i].count = 0;
    }
    return 0;
}

// Stack operations
int stack_push(int handle, char* item) {
    if (handle < 0 || handle >= 5) return -1;
    GenericStack* s = &global_stacks[handle];
    if (s->top >= MAX_STACK_SIZE - 1) return -1; // Stack overflow
    s->items[++s->top] = item; // Store the pointer
    return 0;
}

char* stack_pop(int handle) {
    if (handle < 0 || handle >= 5) return NULL;
    GenericStack* s = &global_stacks[handle];
    if (s->top < 0) return NULL; // Stack underflow
    return s->items[s->top--];
}

char* stack_peek(int handle) {
    if (handle < 0 || handle >= 5) return NULL;
    GenericStack* s = &global_stacks[handle];
    if (s->top < 0) return NULL;
    return s->items[s->top];
}

void stack_clear(int handle) {
    if (handle < 0 || handle >= 5) return;
    GenericStack* s = &global_stacks[handle];
    while (s->top >= 0) {
        free(s->items[s->top--]); // Free the copied string
    }
}

int stack_is_empty(int handle) {
    if (handle < 0 || handle >= 5) return 1; // Treat invalid handle as empty
    return global_stacks[handle].top < 0;
}

// Helper for circular reference detection
int stack_contains_string(int handle, const char* str) {
    if (handle < 0 || handle >= 5 || str == NULL) return 0;
    GenericStack* s = &global_stacks[handle];
    for (int i = 0; i <= s->top; ++i) {
        if (s->items[i] != NULL && strcmp(s->items[i], str) == 0) {
            return 1; // Found
        }
    }
    return 0; // Not found
}


// Queue operations
int queue_enqueue(int handle, char* item) {
    if (handle < 0 || handle >= 5) return -1;
    GenericQueue* q = &global_queues[handle];
    if (q->count >= MAX_QUEUE_SIZE) return -1; // Queue overflow
    q->items[q->tail] = item;
    q->tail = (q->tail + 1) % MAX_QUEUE_SIZE;
    q->count++;
    return 0;
}

char* queue_dequeue(int handle) {
    if (handle < 0 || handle >= 5) return NULL;
    GenericQueue* q = &global_queues[handle];
    if (q->count <= 0) return NULL; // Queue underflow
    char* item = q->items[q->head];
    q->head = (q->head + 1) % MAX_QUEUE_SIZE;
    q->count--;
    return item;
}

void queue_clear(int handle) {
    if (handle < 0 || handle >= 5) return;
    GenericQueue* q = &global_queues[handle];
    while (q->count > 0) {
        free(queue_dequeue(handle)); // Free the copied string
    }
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

int queue_is_empty(int handle) {
    if (handle < 0 || handle >= 5) return 1;
    return global_queues[handle].count <= 0;
}


// Helper wrappers for the specific API used in the original code
void* pop_copy(int stack_handle) {
    return stack_pop(stack_handle); // Caller is responsible for freeing the returned char*
}

int push_copy(int* stack_handle_ptr, void* data, size_t data_size) {
    char* copy = (char*)malloc(data_size);
    if (!copy) return -1;
    memcpy(copy, data, data_size);
    return stack_push(*stack_handle_ptr, copy);
}

int push(int* stack_handle_ptr, void* data) {
    // Assuming push also makes a copy, as `clear_stack` frees items.
    // If `data` is NULL, `strlen` would be an error.
    size_t data_len = (data != NULL) ? strlen((char*)data) + 1 : 0;
    return push_copy(stack_handle_ptr, data, data_len);
}

void clear_stack(int* stack_handle_ptr) {
    stack_clear(*stack_handle_ptr);
}

void* peek_top(int stack_handle) {
    return stack_peek(stack_handle);
}

void* dequeue_copy(int* queue_handle_ptr) {
    return queue_dequeue(*queue_handle_ptr); // Caller is responsible for freeing the returned char*
}

int enqueue_copy(int* queue_handle_ptr, void* data, size_t data_size) {
    char* copy = (char*)malloc(data_size);
    if (!copy) return -1;
    memcpy(copy, data, data_size);
    return queue_enqueue(*queue_handle_ptr, copy);
}

int enqueue(int* queue_handle_ptr, void* data) {
    // Assuming enqueue also makes a copy.
    size_t data_len = (data != NULL) ? strlen((char*)data) + 1 : 0;
    return enqueue_copy(queue_handle_ptr, data, data_len);
}

void clear_queue(int* queue_handle_ptr) {
    queue_clear(*queue_handle_ptr);
}


// --- Utility Functions (Custom itoa/ftoa, etc.) ---

// Custom itoa (integer to ASCII) using snprintf for safety
int itoa(int value, char* buffer, int buffer_size) {
    if (!buffer || buffer_size <= 0) return -1;
    int len = snprintf(buffer, buffer_size, "%d", value);
    if (len < 0 || len >= buffer_size) {
        return -1; // Error or buffer too small
    }
    return 0; // Success
}

// Custom ftoa (float to ASCII) using snprintf for safety
int ftoa(double value, char* buffer, int buffer_size) {
    if (!buffer || buffer_size <= 0) return -1;
    int len = snprintf(buffer, buffer_size, "%f", value);
    if (len < 0 || len >= buffer_size) {
        return -1; // Error or buffer too small
    }
    return 0; // Success
}

// Placeholder for operator functions
// These functions would pop arguments from the stack_handle, perform operation,
// and push result back (or rely on eval_function to handle pushing).
// They also set an error_flag if an issue occurs (e.g., division by zero).
long double op_add(int* stack_handle, int* error_flag) {
    char* b_str = (char*)pop_copy(*stack_handle);
    char* a_str = (char*)pop_copy(*stack_handle);
    if (!a_str || !b_str) { *error_flag = 1; free(a_str); free(b_str); return 0.0L; }
    long double result = atof(a_str) + atof(b_str);
    free(a_str); free(b_str);
    return result;
}
long double op_sub(int* stack_handle, int* error_flag) {
    char* b_str = (char*)pop_copy(*stack_handle);
    char* a_str = (char*)pop_copy(*stack_handle);
    if (!a_str || !b_str) { *error_flag = 1; free(a_str); free(b_str); return 0.0L; }
    long double result = atof(a_str) - atof(b_str);
    free(a_str); free(b_str);
    return result;
}
long double op_mul(int* stack_handle, int* error_flag) {
    char* b_str = (char*)pop_copy(*stack_handle);
    char* a_str = (char*)pop_copy(*stack_handle);
    if (!a_str || !b_str) { *error_flag = 1; free(a_str); free(b_str); return 0.0L; }
    long double result = atof(a_str) * atof(b_str);
    free(a_str); free(b_str);
    return result;
}
long double op_div(int* stack_handle, int* error_flag) {
    char* b_str = (char*)pop_copy(*stack_handle);
    char* a_str = (char*)pop_copy(*stack_handle);
    if (!a_str || !b_str) { *error_flag = 1; free(a_str); free(b_str); return 0.0L; }
    long double divisor = atof(b_str);
    if (fabs(divisor) < 1e-9) { *error_flag = 1; free(a_str); free(b_str); return 0.0L; } // Division by zero
    long double result = atof(a_str) / divisor;
    free(a_str); free(b_str);
    return result;
}

// Global array of operators, used by get_op
char* operators[] = {
    "+", (char*)op_add,
    "-", (char*)op_sub,
    "*", (char*)op_mul,
    "/", (char*)op_div,
    NULL // Sentinel
};

// --- Core Spreadsheet Functions ---

// Function: init_sheet
int init_sheet(void) {
    _g_sheet = calloc(NUM_ROWS * NUM_COLS, sizeof(Cell));
    if (_g_sheet == NULL) {
        return -1;
    }

    g_sheet = (Cell**)malloc(NUM_ROWS * sizeof(Cell*));
    if (g_sheet == NULL) {
        free(_g_sheet);
        _g_sheet = NULL;
        return -1;
    }

    int current_id = 0;
    for (unsigned int i = 0; i < NUM_ROWS; ++i) {
        g_sheet[i] = (Cell*)((char*)_g_sheet + i * NUM_COLS * sizeof(Cell)); // Each row block holds NUM_COLS cells
        for (unsigned int j = 0; j < NUM_COLS; ++j) {
            g_sheet[i][j].id = current_id++;
            // type, data_ptr, formula_ptr are already zeroed by calloc
        }
    }
    return 0;
}

// Function: get_rowcol
// Parses a cell ID string (e.g., "A1") into column characters and row digits.
// Returns 0 on success, -1 on error.
int get_rowcol(const char* cell_id_str, char* col_chars, char* row_digits) {
    if (!cell_id_str || !col_chars || !row_digits) return -1;

    int i = 0;
    int j = 0;
    while (cell_id_str[i] != '\0' && isalpha(cell_id_str[i])) {
        col_chars[i] = toupper(cell_id_str[i]);
        i++;
    }
    col_chars[i] = '\0';

    if (i == 0) return -1; // No column characters

    while (cell_id_str[i] != '\0' && isdigit(cell_id_str[i])) {
        row_digits[j] = cell_id_str[i];
        i++;
        j++;
    }
    row_digits[j] = '\0';

    if (j == 0) return -1; // No row digits

    if (cell_id_str[i] != '\0') return -1; // Extra characters after cell ID

    return 0; // Success
}

// Function: get_cell
// Retrieves a Cell pointer given a cell ID string (e.g., "A1").
Cell* get_cell(char* cell_id_str) {
    char col_chars[32];
    char row_digits[32];

    if (get_rowcol(cell_id_str, col_chars, row_digits) == -1) {
        return NULL; // Invalid cell ID string
    }

    int col_idx_0based = 0;
    size_t col_len = strlen(col_chars);
    for (int i = 0; i < (int)col_len; ++i) {
        // (char - 'A' + 1) for 1-indexed column value (A=1, B=2...)
        col_idx_0based = (int)ROUND((double)col_idx_0based + (double)(toupper(col_chars[i]) - 'A' + 1) * pow(DAT_00017148, (double)(col_len - i - 1)));
    }
    col_idx_0based -= 1; // Convert to 0-indexed (A=0, B=1...)

    long row_idx_1based = strtol(row_digits, NULL, 10);
    if (row_idx_1based <= 0 || row_idx_1based > NUM_COLS) { // Rows usually 1-indexed up to NUM_COLS
        return NULL;
    }
    int row_idx_0based = (int)row_idx_1based - 1; // Convert to 0-indexed

    if (col_idx_0based < 0 || col_idx_0based >= NUM_ROWS) { // NUM_ROWS is 702
        return NULL;
    }

    return &g_sheet[col_idx_0based][row_idx_0based];
}

// Function: show_cell
// Displays the content of a cell based on display_type.
char * show_cell(char* cell_id_str, int display_type, char *buffer, int buffer_size) {
    Cell *cell = get_cell(cell_id_str);
    if (cell == NULL) {
        return NULL;
    }

    if (cell->type == 0) { // Empty
        return "";
    }
    if (cell->type == 1) { // Error
        return "!VALUE";
    }

    if (display_type == 0) { // Display evaluated value
        if (cell->type == 7) { // Formula
            int error_flag = 0;
            int circular_ref_stack_handle = 4; // Placeholder for circular reference detection stack
            init_global_structures(); // Ensure stacks/queues are clear for new evaluation context
            long double evaluated_value = eval_formula(cell->formula_ptr, &error_flag, &circular_ref_stack_handle, cell->id);

            if (error_flag == 0) {
                ftoa(evaluated_value, buffer, buffer_size);
                return buffer;
            } else {
                return "!FORMULA: CIRREF/STR/DIV0";
            }
        } else { // Not a formula, display data_ptr content directly
            return cell->data_ptr;
        }
    } else { // display_type != 0, display raw content (e.g., formula string)
        return cell->data_ptr;
    }
}

// Function: set_cell
// Sets the content of a cell.
int set_cell(char* cell_id_str, char *input_str, unsigned int max_len) {
    if (input_str == NULL) {
        return -1;
    }
    size_t input_len = strlen(input_str);
    if (input_len == 0 || max_len <= input_len) { // Check for empty string or buffer overflow
        return -1;
    }

    Cell *cell = get_cell(cell_id_str);
    if (cell == NULL) {
        return -1;
    }

    // Clear existing data if any
    if (cell->type != 0) { // If not empty
        free(cell->data_ptr);
        cell->data_ptr = NULL;
        cell->type = 0;
        cell->formula_ptr = NULL; // Clear formula pointer too
    }

    // Allocate memory for the new string and copy it
    cell->data_ptr = (char*)malloc(input_len + 1);
    if (cell->data_ptr == NULL) {
        return -1;
    }
    strcpy(cell->data_ptr, input_str);

    // Determine cell type
    if (input_len >= 2 && input_str[0] == '=') {
        cell->formula_ptr = cell->data_ptr + 1; // Formula starts after '='
        cell->type = 7; // Formula type
    } else {
        int arg_type = parsearg(cell->data_ptr);
        cell->type = arg_type;
        if (cell->type != 4) { // If not a number, treat as generic string
            cell->type = 2; // String type
        }
    }
    return 0;
}

// Function: clear_cell
// Clears the content of a cell.
int clear_cell(char* cell_id_str) {
    Cell *cell = get_cell(cell_id_str);
    if (cell == NULL) {
        return -1;
    }

    if (cell->type != 0) { // If not already empty
        free(cell->data_ptr);
        cell->data_ptr = NULL;
        cell->type = 0;
        cell->formula_ptr = NULL;
    }
    return 0;
}

// Function: print_assigned_cells
// Prints all non-empty cells and their contents.
void print_assigned_cells(void) {
    char row_str[4]; // For row number, up to 3 digits + null
    char col_chars[3]; // For column letters, e.g., "AA" + null

    for (unsigned int col_idx = 0; col_idx < NUM_ROWS; ++col_idx) { // Iterate through column blocks
        for (unsigned int row_idx = 0; row_idx < NUM_COLS; ++row_idx) { // Iterate through rows within a block
            Cell *cell = &g_sheet[col_idx][row_idx];
            if (cell->type != 0) { // If cell is not empty
                unsigned int cell_id = cell->id;
                
                // Convert row number (0-99)
                itoa(cell_id % NUM_COLS + 1, row_str, sizeof(row_str)); // Add 1 for 1-indexed row display

                // Convert column letters (A, B, ..., Z, AA, AB, ...)
                unsigned int col_val_0based = cell_id / NUM_COLS; // This is the 0-indexed column block

                if (col_val_0based < 26) { // Single letter column (A-Z)
                    col_chars[0] = (char)(col_val_0based + 'A');
                    col_chars[1] = '\0';
                } else { // Two-letter column (AA, AB, ...)
                    col_chars[0] = (char)((col_val_0based / 26 - 1) + 'A');
                    col_chars[1] = (char)((col_val_0based % 26) + 'A');
                    col_chars[2] = '\0';
                }
                
                // Print cell coordinate and content
                printf("%s%s=%s\n", col_chars, row_str, cell->data_ptr);
            }
        }
    }
}

// Function: get_op
// Retrieves an operator entry (name and function pointer) given its name.
char ** get_op(char *op_name) {
    if (op_name == NULL) {
        return NULL;
    }

    size_t len = strlen(op_name);
    char *normalized_op = (char *)malloc(len + 1);
    if (normalized_op == NULL) {
        return NULL;
    }
    strcpy(normalized_op, op_name);

    for (unsigned int i = 0; i < len; ++i) {
        normalized_op[i] = (char)toupper((int)normalized_op[i]);
    }

    char **op_entry = operators; // Global array of {name, func_ptr}
    while (*op_entry != NULL) {
        if (strcmp(*op_entry, normalized_op) == 0) {
            free(normalized_op);
            return op_entry; // Found
        }
        op_entry += 2; // Move to next operator pair
    }

    free(normalized_op);
    return NULL; // Not found
}

// Function: parsearg
// Parses an argument string to determine its type (number, cell ref, operator, string, error).
int parsearg(char *arg_str) {
    if (arg_str == NULL) {
        return 1; // Empty/invalid argument
    }

    char *current_char = arg_str;
    int digit_count = 0;
    int dot_count = 0;
    int minus_count = 0;
    int quote_count = 0;
    int other_char_count = 0;

    for (; *current_char != '\0'; ++current_char) {
        if (*current_char >= '0' && *current_char <= '9') {
            digit_count++;
        } else if (*current_char == '-') {
            minus_count++;
            digit_count++; // Treat minus as part of number for length count
        } else if (*current_char == '.') {
            dot_count++;
            digit_count++; // Treat dot as part of number for length count
        } else if (*current_char == '\"') {
            quote_count++;
        } else {
            other_char_count++;
        }
    }

    if (quote_count == 0) {
        if (other_char_count == 0) {
            // Numeric or empty
            if (digit_count == 0) {
                return 1; // Empty/invalid argument
            } else if (dot_count < 2 && minus_count < 2) {
                // Valid number format checks (simplified from original)
                // Original logic was very specific about position of '.' and '-'
                // For now, assume a simple integer or float if it only contains digits, dots, minuses.
                // A more robust parser would use strtod.
                if (minus_count > 0 && arg_str[0] != '-') return 2; // Minus not at start
                if (dot_count > 0 && (arg_str[0] == '.' || arg_str[strlen(arg_str)-1] == '.')) return 2; // Dot not at start/end
                return 4; // Number (integer or float)
            } else {
                return 2; // Error (too many dots or minuses)
            }
        } else {
            // Contains other chars, might be cell ID or operator
            if (dot_count < 2 && minus_count < 2) { // Still apply some numeric checks
                if (valid_cell_id(arg_str)) {
                    return 6; // Cell reference
                } else if (get_op(arg_str) != NULL) {
                    return 5; // Operator (e.g., "SUM")
                } else {
                    return 2; // Error (invalid cell ID or operator)
                }
            } else {
                return 2; // Error
            }
        }
    } else {
        // String literal (quoted)
        // Check if it starts and ends with a quote, and has exactly two quotes.
        if (quote_count == 2 && arg_str[0] == '\"' && arg_str[strlen(arg_str) - 1] == '\"') {
            return 3; // String literal (quoted)
        } else {
            return 2; // Error (malformed quotes)
        }
    }
}

// Function: valid_cell_id
// Checks if a string is a valid cell ID (e.g., "A1", "AB100").
int valid_cell_id(char* cell_id_str) {
    char col_chars[32]; // Max reasonable column name length
    char row_digits[32]; // Max reasonable row number length
    return (get_rowcol(cell_id_str, col_chars, row_digits) == 0);
}

// Function: is_arg_arithmetic
// Checks if the given operator string represents an arithmetic operator.
int is_arg_arithmetic(void* param_1) {
    char* op_str = (char*)param_1;
    if (strcmp(op_str, "+") == 0 ||
        strcmp(op_str, "-") == 0 ||
        strcmp(op_str, "*") == 0 ||
        strcmp(op_str, "/") == 0) {
        return 1;
    }
    return 0;
}

// Function: eval_function
// Evaluates a function or operator given its entry and an operand stack.
long double eval_function(char** op_entry, int operand_stack_handle_in, char* result_buffer, int result_buffer_size) {
    long double result_val = 0.0L;
    int error_flag_func = 0;
    int temp_operand_stack_handle = 3; // Use a new temporary stack handle for this function's arguments

    if (op_entry == NULL || op_entry[0] == NULL || op_entry[1] == NULL || result_buffer == NULL || result_buffer_size < 3) {
        error_flag_func = 1;
        goto cleanup_eval_function;
    }

    char* op_str = op_entry[0];
    unsigned int num_args_expected;

    if (is_arg_arithmetic(op_str)) {
        num_args_expected = 2; // Arithmetic ops generally take 2 args
    } else {
        // For other functions, the first argument on the stack might be the count of actual arguments.
        char* num_args_str = (char*)pop_copy(operand_stack_handle_in);
        if (num_args_str == NULL) {
            error_flag_func = 1;
            goto cleanup_eval_function;
        }
        num_args_expected = strtol(num_args_str, NULL, 10);
        free(num_args_str);
    }

    for (unsigned int i = 0; i < num_args_expected; ++i) {
        char* arg_str = (char*)pop_copy(operand_stack_handle_in);
        if (arg_str == NULL) {
            error_flag_func = 1;
            goto cleanup_eval_function;
        }

        int arg_type = parsearg(arg_str);
        if (arg_type == 5) { // If argument is an operator/function call itself
            char** nested_op_entry = get_op(arg_str);
            free(arg_str);
            if (nested_op_entry == NULL) {
                 error_flag_func = 1;
                 goto cleanup_eval_function;
            }
            char temp_nested_result_buffer[256]; // Temp buffer for nested results
            long double nested_result = eval_function(nested_op_entry, operand_stack_handle_in, temp_nested_result_buffer, sizeof(temp_nested_result_buffer));
            if (error_flag_func != 0) { // Check if nested call set an error
                goto cleanup_eval_function;
            }
            ftoa(nested_result, temp_nested_result_buffer, sizeof(temp_nested_result_buffer));
            if (push_copy(&temp_operand_stack_handle, temp_nested_result_buffer, strlen(temp_nested_result_buffer) + 1) == -1) {
                error_flag_func = 1;
                goto cleanup_eval_function;
            }
        } else {
            if (push(&temp_operand_stack_handle, arg_str) == -1) { // Push a copy of arg_str
                free(arg_str);
                error_flag_func = 1;
                goto cleanup_eval_function;
            }
            free(arg_str); // Free the original arg_str copy
        }
    }

    // Call the actual operator/function
    result_val = ((long double (*)(int*, int*))op_entry[1])(&temp_operand_stack_handle, &error_flag_func);

    if (error_flag_func == 0) {
        if (ftoa(result_val, result_buffer, result_buffer_size) == -1) {
            error_flag_func = 1; // ftoa failed
        }
    }

cleanup_eval_function:
    clear_stack(&temp_operand_stack_handle); // Clean up temporary stack
    if (error_flag_func != 0) {
        // Propagate error
        return 0.0L;
    }
    return result_val;
}

// Function: eval_formula
// Evaluates a formula string in RPN, handling cell references, functions, and circular references.
long double eval_formula(char* formula_str, int* error_flag, int* circular_ref_stack_handle, int current_cell_id) {
    long double final_result_value = 0.0L;
    *error_flag = 0; // Initialize error flag

    // Use specific global stack/queue handles for this function instance
    int rpn_queue_handle = 0; // Output queue from infixtorpn
    int operand_stack_handle = 1; // Operand stack for RPN evaluation
    int temp_buffer_size = 256; // Max size for ftoa buffers

    char current_cell_id_str[temp_buffer_size];
    char temp_eval_buffer[temp_buffer_size];

    // Convert current_cell_id to string for circular reference detection
    if (itoa(current_cell_id, current_cell_id_str, sizeof(current_cell_id_str)) == -1) {
        *error_flag = 1;
        goto cleanup_eval_formula;
    }
    // Push current cell ID onto circular reference stack
    if (push_copy(circular_ref_stack_handle, current_cell_id_str, strlen(current_cell_id_str) + 1) == -1) {
        *error_flag = 1;
        goto cleanup_eval_formula;
    }

    // Convert infix formula to RPN
    rpn_queue_handle = infixtorpn(formula_str, strlen(formula_str) + 1); // infixtorpn returns a handle
    if (*error_flag != 0 || queue_is_empty(rpn_queue_handle)) { // infixtorpn failed or empty result
        *error_flag = 1;
        goto cleanup_eval_formula;
    }

    // Evaluate RPN expression
    while (!queue_is_empty(rpn_queue_handle)) {
        char* token = (char*)dequeue_copy(rpn_queue_handle);
        if (token == NULL) {
            *error_flag = 1;
            goto cleanup_eval_formula;
        }

        int arg_type = parsearg(token);

        if (arg_type == 6) { // Cell reference
            Cell* target_cell = get_cell(token);
            if (target_cell == NULL) {
                *error_flag = 1;
                free(token);
                goto cleanup_eval_formula;
            }

            // Check for circular reference
            char target_cell_id_str[temp_buffer_size];
            itoa(target_cell->id, target_cell_id_str, sizeof(target_cell_id_str));
            if (stack_contains_string(*circular_ref_stack_handle, target_cell_id_str)) {
                *error_flag = 1; // Circular reference detected
                free(token);
                goto cleanup_eval_formula;
            }

            if (target_cell->type == 0) { // Empty cell
                if (push_copy(&operand_stack_handle, (void*)DAT_00017077, strlen(DAT_00017077) + 1) == -1) { // Push "0"
                    *error_flag = 1;
                    free(token);
                    goto cleanup_eval_formula;
                }
            } else if (target_cell->type == 4) { // Number type, data_ptr is string representation
                if (push_copy(&operand_stack_handle, target_cell->data_ptr, strlen(target_cell->data_ptr) + 1) == -1) {
                    *error_flag = 1;
                    free(token);
                    goto cleanup_eval_formula;
                }
            } else if (target_cell->type == 7) { // Formula cell
                long double evaluated_nested_formula = eval_formula(target_cell->formula_ptr, error_flag, circular_ref_stack_handle, target_cell->id);
                if (*error_flag != 0) {
                    free(token);
                    goto cleanup_eval_formula;
                }
                ftoa(evaluated_nested_formula, temp_eval_buffer, sizeof(temp_eval_buffer));
                if (push_copy(&operand_stack_handle, temp_eval_buffer, strlen(temp_eval_buffer) + 1) == -1) {
                    *error_flag = 1;
                    free(token);
                    goto cleanup_eval_formula;
                }
            } else { // Other types (string literal, error, etc.) - not evaluable as number in arithmetic context
                *error_flag = 1;
                free(token);
                goto cleanup_eval_formula;
            }
        } else if (arg_type == 4) { // Numeric literal
            if (push(&operand_stack_handle, token) == -1) { // Push a copy of token
                *error_flag = 1;
                free(token);
                goto cleanup_eval_formula;
            }
        } else if (arg_type == 5) { // Operator/Function
            char** op_entry = get_op(token);
            if (op_entry == NULL) {
                *error_flag = 1;
                free(token);
                goto cleanup_eval_formula;
            }
            // Evaluate the function/operator using arguments from operand_stack
            eval_function(op_entry, operand_stack_handle, temp_eval_buffer, sizeof(temp_eval_buffer));
            if (*error_flag != 0) { // eval_function sets error_flag if issue
                free(token);
                goto cleanup_eval_formula;
            }
            if (push_copy(&operand_stack_handle, temp_eval_buffer, strlen(temp_eval_buffer) + 1) == -1) {
                *error_flag = 1;
                free(token);
                goto cleanup_eval_formula;
            }
        } else { // Other argument types (string literal, malformed, etc.) -> error in formula
            *error_flag = 1;
            free(token);
            goto cleanup_eval_formula;
        }
        free(token); // Free token after use
    }

    char* final_result_str = (char*)pop_copy(&operand_stack_handle);
    if (final_result_str == NULL || !stack_is_empty(operand_stack_handle)) { // Stack should contain only one result
        *error_flag = 1;
        free(final_result_str);
        goto cleanup_eval_formula;
    }
    final_result_value = atof(final_result_str);
    free(final_result_str);

cleanup_eval_formula:
    clear_queue(&rpn_queue_handle);
    clear_stack(&operand_stack_handle);
    // Pop current cell ID from circular reference stack (if pushed)
    void* popped_id = pop_copy(circular_ref_stack_handle);
    free(popped_id); // Free the string

    return final_result_value;
}

// Function: sanitize_formula
// Placeholder for formula sanitization logic.
int sanitize_formula(char* formula_str, size_t buffer_size) {
    // Implement checks for balanced parentheses, valid characters, etc.
    // For now, assume formula is valid.
    (void)formula_str;
    (void)buffer_size;
    return 0; // Success
}

// Function: infixtorpn
// Converts an infix formula string to Reverse Polish Notation (RPN).
int infixtorpn(char* infix_expr, size_t buffer_size) {
    int rpn_output_queue_handle = 0; // Use handle 0 for RPN output
    int operator_stack_handle = 2; // Use handle 2 for operator stack
    int error_flag_rpn = 0;

    char* formula_copy = (char*)malloc(buffer_size);
    if (formula_copy == NULL) {
        error_flag_rpn = 1;
        goto cleanup_infixtorpn;
    }
    memcpy(formula_copy, infix_expr, buffer_size);

    if (sanitize_formula(formula_copy, buffer_size) == -1) {
        error_flag_rpn = 1;
        goto cleanup_infixtorpn;
    }

    char* delimiters = "():,+-*/";
    char current_op_str[2]; // For single char operators
    current_op_str[0] = '\0';
    current_op_str[1] = '\0';

    // This array stores argument counts for functions.
    int* func_arg_counts_array = NULL;
    int func_arg_counts_idx = -1; // Top of the func_arg_counts_array
    int func_arg_counts_capacity = 16; // Initial capacity

    func_arg_counts_array = (int*)malloc(func_arg_counts_capacity * sizeof(int));
    if (func_arg_counts_array == NULL) {
        error_flag_rpn = 1;
        goto cleanup_infixtorpn;
    }

    char* read_ptr = formula_copy;
    char* token_start_ptr = formula_copy;
    char prev_char = '\0'; // Previous character processed

    for (unsigned int current_idx = 0; current_idx < buffer_size && *read_ptr != '\0'; ++current_idx) {
        char current_char = *read_ptr;
        char* delimiter_found = strchr(delimiters, (int)current_char);

        if (delimiter_found == NULL) { // Not a delimiter
            prev_char = current_char;
            read_ptr++;
        } else { // It's a delimiter
            // Handle unary minus
            bool is_unary_minus = (current_char == '-') &&
                                  (current_idx > 0) &&
                                  (prev_char == '(' || strchr("+-*/,", prev_char) != NULL);

            if (is_unary_minus) {
                // Treat unary minus as part of the number, not an operator for RPN conversion
                // A more robust parser would convert it to a special UNARY_MINUS token.
                // For now, just advance and continue.
                prev_char = current_char;
                read_ptr++;
                continue;
            }

            // End of a token (operand or multi-char operator)
            current_op_str[0] = current_char;
            current_op_str[1] = '\0';
            
            *read_ptr = '\0'; // Null-terminate the current token
            
            int arg_type = parsearg(token_start_ptr);

            if (arg_type == 6 || arg_type == 4) { // Cell reference or Number literal
                if (enqueue_copy(&rpn_output_queue_handle, token_start_ptr, strlen(token_start_ptr) + 1) == -1) {
                    error_flag_rpn = 1;
                    goto cleanup_infixtorpn;
                }
            } else if (arg_type == 5) { // Function/Operator (like SUM, AVG)
                if (func_arg_counts_idx == func_arg_counts_capacity - 1) {
                    func_arg_counts_capacity *= 2;
                    int* new_array = (int*)realloc(func_arg_counts_array, func_arg_counts_capacity * sizeof(int));
                    if (new_array == NULL) {
                        error_flag_rpn = 1;
                        goto cleanup_infixtorpn;
                    }
                    func_arg_counts_array = new_array;
                }
                func_arg_counts_array[++func_arg_counts_idx] = 0; // Initialize arg count for new function

                if (push_copy(&operator_stack_handle, token_start_ptr, strlen(token_start_ptr) + 1) == -1) {
                    error_flag_rpn = 1;
                    goto cleanup_infixtorpn;
                }
            } else if (arg_type == 1) { // Empty token, possibly due to multiple delimiters. Ignore.
                // No action, token_start_ptr will be advanced.
            } else { // Error in token parsing
                error_flag_rpn = 1;
                goto cleanup_infixtorpn;
            }
            
            token_start_ptr = read_ptr + 1; // Start of next token

            // Handle the current delimiter
            switch (current_char) {
                case '(':
                    if (push_copy(&operator_stack_handle, (void*)DAT_00017082, strlen(DAT_00017082) + 1) == -1) {
                        error_flag_rpn = 1;
                        goto cleanup_infixtorpn;
                    }
                    break;
                case ')':
                    while (!stack_is_empty(operator_stack_handle)) {
                        char* op_top = (char*)peek_top(operator_stack_handle);
                        if (strcmp(op_top, "(") == 0) {
                            void* popped_paren = pop_copy(operator_stack_handle);
                            free(popped_paren); // Discard '('
                            break;
                        }
                        char* popped_op = (char*)pop_copy(operator_stack_handle);
                        if (enqueue(&rpn_output_queue_handle, popped_op) == -1) {
                            error_flag_rpn = 1;
                            free(popped_op);
                            goto cleanup_infixtorpn;
                        }
                        free(popped_op);
                    }
                    if (stack_is_empty(operator_stack_handle)) { // Mismatched parenthesis
                        error_flag_rpn = 1;
                        goto cleanup_infixtorpn;
                    }
                    // After ')' check if top of stack is a function, if so, push its arg count
                    if (!stack_is_empty(operator_stack_handle)) {
                        char* op_top = (char*)peek_top(operator_stack_handle);
                        if (parsearg(op_top) == 5 && func_arg_counts_idx >= 0) { // It's a function
                            char arg_count_str[16];
                            if (itoa(func_arg_counts_array[func_arg_counts_idx] + 1, arg_count_str, sizeof(arg_count_str)) == -1) {
                                error_flag_rpn = 1;
                                goto cleanup_infixtorpn;
                            }
                            if (enqueue_copy(&rpn_output_queue_handle, arg_count_str, strlen(arg_count_str) + 1) == -1) {
                                error_flag_rpn = 1;
                                goto cleanup_infixtorpn;
                            }
                            char* func_name = (char*)pop_copy(operator_stack_handle);
                            if (enqueue(&rpn_output_queue_handle, func_name) == -1) {
                                error_flag_rpn = 1;
                                free(func_name);
                                goto cleanup_infixtorpn;
                            }
                            free(func_name);
                            func_arg_counts_idx--; // Pop function from arg count tracking
                        }
                    }
                    break;
                case '*':
                case '/':
                    while (!stack_is_empty(operator_stack_handle)) {
                        char* op_top = (char*)peek_top(operator_stack_handle);
                        // Higher precedence operators: *, /
                        if (strcmp(op_top, "*") == 0 || strcmp(op_top, "/") == 0) {
                            char* popped_op = (char*)pop_copy(operator_stack_handle);
                            if (enqueue(&rpn_output_queue_handle, popped_op) == -1) {
                                error_flag_rpn = 1;
                                free(popped_op);
                                goto cleanup_infixtorpn;
                            }
                            free(popped_op);
                        } else {
                            break;
                        }
                    }
                    if (push_copy(&operator_stack_handle, current_op_str, strlen(current_op_str) + 1) == -1) {
                        error_flag_rpn = 1;
                        goto cleanup_infixtorpn;
                    }
                    break;
                case '+':
                case '-':
                    while (!stack_is_empty(operator_stack_handle)) {
                        char* op_top = (char*)peek_top(operator_stack_handle);
                        // Equal or higher precedence operators: *, /, +, -
                        if (strcmp(op_top, "*") == 0 || strcmp(op_top, "/") == 0 ||
                            strcmp(op_top, "+") == 0 || strcmp(op_top, "-") == 0) {
                            char* popped_op = (char*)pop_copy(operator_stack_handle);
                            if (enqueue(&rpn_output_queue_handle, popped_op) == -1) {
                                error_flag_rpn = 1;
                                free(popped_op);
                                goto cleanup_infixtorpn;
                            }
                            free(popped_op);
                        } else {
                            break;
                        }
                    }
                    if (push_copy(&operator_stack_handle, current_op_str, strlen(current_op_str) + 1) == -1) {
                        error_flag_rpn = 1;
                        goto cleanup_infixtorpn;
                    }
                    break;
                case ',':
                    while (!stack_is_empty(operator_stack_handle)) {
                        char* op_top = (char*)peek_top(operator_stack_handle);
                        if (strcmp(op_top, "(") == 0) {
                            // If a function argument separator, increment arg count for current function
                            if (func_arg_counts_idx >= 0) {
                                func_arg_counts_array[func_arg_counts_idx]++;
                            }
                            break; // Stop popping at '('
                        }
                        char* popped_op = (char*)pop_copy(operator_stack_handle);
                        if (enqueue(&rpn_output_queue_handle, popped_op) == -1) {
                            error_flag_rpn = 1;
                            free(popped_op);
                            goto cleanup_infixtorpn;
                        }
                        free(popped_op);
                    }
                    if (stack_is_empty(operator_stack_handle)) { // Mismatched parenthesis or comma outside function
                        error_flag_rpn = 1;
                        goto cleanup_infixtorpn;
                    }
                    break;
            }
            prev_char = current_char;
            read_ptr++;
        }
    }

    // After loop, push any remaining operators from stack to queue
    while (!stack_is_empty(operator_stack_handle)) {
        char* op_top = (char*)peek_top(operator_stack_handle);
        if (strcmp(op_top, "(") == 0 || strcmp(op_top, ")") == 0) { // Mismatched parenthesis
            error_flag_rpn = 1;
            break;
        }
        char* popped_op = (char*)pop_copy(operator_stack_handle);
        if (enqueue(&rpn_output_queue_handle, popped_op) == -1) {
            error_flag_rpn = 1;
            free(popped_op);
            goto cleanup_infixtorpn;
        }
        free(popped_op);
    }

cleanup_infixtorpn:
    free(formula_copy);
    free(func_arg_counts_array);
    clear_stack(&operator_stack_handle);

    if (error_flag_rpn != 0) {
        clear_queue(&rpn_output_queue_handle); // Clear output if error
        return -1; // Indicate error
    }
    return rpn_output_queue_handle; // Return handle to RPN queue
}