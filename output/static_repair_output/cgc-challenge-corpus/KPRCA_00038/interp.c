#include <stdio.h>   // For sprintf
#include <stdlib.h>  // For calloc, malloc, free, strtol
#include <string.h>  // For strdup, strlen, strcmp, strncmp, memcpy, memset, strcpy
#include <stdbool.h> // For bool type
#include <stdint.h>  // For intptr_t (for safe integer-to-pointer conversions)

// --- External/Mocked Function Declarations ---
// These functions are not provided in the snippet, so we declare them as extern.
// Their actual implementations and full signatures might differ based on the original system.

// Dictionary functions (assuming a simple void* based dictionary)
// dict_init takes a void* to the dict storage and a free function for values.
// dict_add/get takes a void* to the dict storage and a key (here, an int, likely a string hash or pointer).
extern int dict_init(void *dict_storage, void (*free_value_func)(int));
extern void *dict_get(void *dict_storage, int key);
extern int dict_add(void *dict_storage, int key, void *value);
extern void dict_free(void *dict_storage);

// I/O functions
extern int io_getc(int stream_handle); // Reads a character from a custom I/O stream
extern int fdprintf(int fd, const char *format, ...); // Prints to a file descriptor, like fprintf
extern int eprintf(int fd, void *format_var, void *vars_array, unsigned int count); // Custom print for variable arrays

// Regular expression functions
extern int regexp_init(void *regexp_storage, const char *pattern);
extern int regexp_match(void *regexp_storage, const char *text);
extern void regexp_free(void *regexp_storage);

// --- Global Data (Inferred from DAT_ addresses) ---
// These are likely static string literals used as keys or default values.
static char DAT_0001b093[] = "";    // Empty string
static char DAT_0001b094[] = "RS";  // Record Separator
static char DAT_0001b099[] = "FS";  // Field Separator
static char DAT_0001b09c[] = "OFS"; // Output Field Separator
static char DAT_0001b090[] = "%d";  // Format string for integer printing
static char DAT_0001b0a0[] = "%s";  // Format string for string printing
static char DAT_0001b134[] = "ORS"; // Output Record Separator

// --- Type Definitions ---

// Enum for variable types
enum VarType {
    VAR_TYPE_EMPTY = 0,
    VAR_TYPE_NUMBER = 1,
    VAR_TYPE_STRING = 2,
    VAR_TYPE_UNKNOWN = 3 // For cases where type is 3, treated as unsupported
};

// Represents a variable (number or string).
// Assumed 32-bit compilation where int and char* are both 4 bytes.
// Total size: 0x20 (32 bytes)
typedef struct Var {
    char _padding_to_14[0x14]; // 20 bytes padding to reach offset 0x14
    int type;                  // Offset 0x14 (4 bytes)
    union {
        int number_val;        // For VAR_TYPE_NUMBER
        char *string_val;      // For VAR_TYPE_STRING
    } value;                   // Offset 0x18 (4 bytes)
    char *number_str_cache;    // Offset 0x1c (4 bytes) - Cached string for numbers
} Var;

// Represents the program's execution context.
// Total size: 0x4C (76 bytes) as implied by memset in program_run.
typedef struct ProgramContext {
    int self_ref_or_id;        // Offset 0x00: param_1 from program_run, likely a reference or ID.
    int io_stream_handle;      // Offset 0x04: Handle for I/O operations.
    char dict_storage[20];     // Offset 0x08: Storage for the dictionary, size 20 bytes.
    char *record_buffer;       // Offset 0x1C: Buffer to hold the current record line.
    char *current_record_ptr;  // Offset 0x20: Pointer to the start of the current record data.
    char **fields_array;       // Offset 0x24: Array of pointers to field strings.
    unsigned int num_fields;   // Offset 0x28: Number of fields in fields_array.
    Var result_var;            // Offset 0x2C: Embedded Var struct for expression results.
                               // Its fields map to ctx+0x40, ctx+0x44, ctx+0x48 respectively.
} ProgramContext;

// Forward declarations
bool eval_expression(ProgramContext *ctx, int *expr_ptr);
int eval_statements(ProgramContext *ctx, int *stmt_list);

// --- Function Implementations ---

// Function: new_number
void *new_number(int value) {
    Var *new_var = calloc(1, sizeof(Var));
    if (new_var == NULL) {
        return NULL;
    }
    new_var->type = VAR_TYPE_NUMBER;
    new_var->value.number_val = value;
    return new_var;
}

// Function: new_string
void *new_string(char *value) {
    if (value == NULL) {
        return NULL;
    }
    Var *new_var = calloc(1, sizeof(Var));
    if (new_var == NULL) {
        return NULL;
    }
    new_var->type = VAR_TYPE_STRING;
    new_var->value.string_val = value; // Takes ownership of the string
    return new_var;
}

// Function: free_var
// This function frees the *contents* of a Var struct but not the Var struct itself.
// This is critical for stack-allocated Var structs (like ctx->result_var or temporaries).
// Callers or the dictionary implementation must free the Var struct if it was dynamically allocated.
void free_var(int var_raw_ptr) {
    Var *var = (Var *)(intptr_t)var_raw_ptr;
    if (var == NULL) {
        return;
    }

    if (var->type == VAR_TYPE_STRING && var->value.string_val != NULL) {
        free(var->value.string_val);
        var->value.string_val = NULL;
    }
    if (var->number_str_cache != NULL) {
        free(var->number_str_cache);
        var->number_str_cache = NULL;
    }
    var->type = VAR_TYPE_EMPTY; // Mark as empty
}

// Function: copy_var
bool copy_var(Var *dest, Var *src) {
    if (dest == NULL || src == NULL) { // Simplified error check
        return false;
    }

    // Free existing string data in dest if it's a string
    if (dest->type == VAR_TYPE_STRING && dest->value.string_val != NULL) {
        free(dest->value.string_val);
        dest->value.string_val = NULL;
    }
    // Free existing number string cache in dest
    if (dest->number_str_cache != NULL) {
        free(dest->number_str_cache);
        dest->number_str_cache = NULL;
    }

    dest->type = src->type;
    if (src->type == VAR_TYPE_NUMBER) {
        dest->value.number_val = src->value.number_val;
    } else if (src->type == VAR_TYPE_STRING) {
        char *new_str = strdup(src->value.string_val);
        if (new_str == NULL) {
            dest->type = VAR_TYPE_EMPTY; // Clear type on allocation failure
            return false;
        }
        dest->value.string_val = new_str;
    } else { // VAR_TYPE_EMPTY or UNKNOWN
        dest->type = VAR_TYPE_EMPTY;
    }
    return true;
}

// Function: move_var
void move_var(Var *dest, Var *src) {
    if (dest == NULL || src == NULL) {
        return;
    }

    // Free destination's existing string data if it's a string
    if (dest->type == VAR_TYPE_STRING && dest->value.string_val != NULL) {
        free(dest->value.string_val);
    }
    // Free destination's existing number string cache
    if (dest->number_str_cache != NULL) {
        free(dest->number_str_cache);
    }

    // Move data from source to destination
    dest->type = src->type;
    dest->value = src->value; // Copy union contents (number_val or string_val pointer)
    dest->number_str_cache = src->number_str_cache; // Move cache pointer

    // Invalidate source (it no longer owns the data)
    src->type = VAR_TYPE_EMPTY;
    src->value.number_val = 0; // Clear value
    src->number_str_cache = NULL; // Clear cache pointer
}

// Function: to_string_buf
void to_string_buf(int number_val, char *buffer) {
    sprintf(buffer, DAT_0001b090, number_val); // Use DAT_0001b090 for "%d"
}

// Function: to_string
char *to_string(int number_val) {
    char temp_buffer[24]; // Buffer for sprintf
    sprintf(temp_buffer, DAT_0001b090, number_val);
    return strdup(temp_buffer); // Return newly allocated string
}

// Function: get_var
Var *get_var(ProgramContext *ctx, int key) {
    Var *var = dict_get(ctx->dict_storage, key);
    if (var == NULL) {
        var = calloc(1, sizeof(Var));
        if (var == NULL) {
            return NULL;
        }
        var->type = VAR_TYPE_EMPTY; // Initialize as empty
        // The original code passed a magic number 0x11f5e, which is removed here.
        if (dict_add(ctx->dict_storage, key, var) == 0) {
            free(var); // Free the Var if dict_add fails
            return NULL;
        }
    }
    return var;
}

// Function: get_string
char *get_string(ProgramContext *ctx, int key) {
    Var *var = get_var(ctx, key);
    if (var == NULL) {
        return NULL;
    }

    if (var->type == VAR_TYPE_NUMBER) {
        if (var->number_str_cache == NULL) {
            var->number_str_cache = to_string(var->value.number_val);
            if (var->number_str_cache == NULL) {
                return NULL;
            }
        }
        return var->number_str_cache;
    } else if (var->type == VAR_TYPE_STRING) {
        return var->value.string_val;
    } else if (var->type == VAR_TYPE_EMPTY) {
        return DAT_0001b093; // Empty string literal
    }
    return NULL; // Unknown type
}

// Function: coerce_number
long coerce_number(ProgramContext *ctx, Var *var) {
    if (var == NULL || var->type == VAR_TYPE_EMPTY) {
        return 0;
    } else if (var->type == VAR_TYPE_NUMBER) {
        return var->value.number_val;
    } else if (var->type == VAR_TYPE_STRING) {
        return strtol(var->value.string_val, NULL, 0);
    }
    return 0; // Unknown type
}

// Function: coerce_bool
bool coerce_bool(ProgramContext *ctx, Var *var) {
    if (var == NULL) {
        return false;
    } else if (var->type == VAR_TYPE_NUMBER) {
        return var->value.number_val != 0;
    } else if (var->type == VAR_TYPE_STRING) {
        return var->value.string_val != NULL && *var->value.string_val != '\0';
    }
    return false; // VAR_TYPE_EMPTY or UNKNOWN
}

// Function: get_number
bool get_number(ProgramContext *ctx, int key, int *out_number) {
    Var *var = get_var(ctx, key);
    if (var == NULL || var->type == VAR_TYPE_UNKNOWN) { // VAR_TYPE_UNKNOWN is type 3
        return false;
    }
    *out_number = coerce_number(ctx, var); // Coerce and store the number
    return true;
}

// Function: read_record
bool read_record(ProgramContext *ctx) {
    char *record_separator = get_string(ctx, (int)(intptr_t)DAT_0001b094);
    if (record_separator == NULL) {
        return false;
    }

    size_t rs_len = strlen(record_separator);
    bool rs_is_newline = (rs_len == 0); // Original code treats empty string as "\n"
    if (rs_is_newline) {
        record_separator = "\n";
        rs_len = 1;
    }

    if (rs_len >= 0x1000) { // Record buffer max size is 0x1000
        return false;
    }

    size_t current_len;
    int c;
    do {
        current_len = 0;
        // Read characters until buffer full or separator found
        while (current_len < 0xFFF) { // Max buffer 0x1000, 0xFFF for safety + null terminator
            c = io_getc(ctx->io_stream_handle);
            if (c < 0) { // EOF or error
                return false;
            }
            ctx->record_buffer[current_len++] = (char)c;

            // Check if record_separator is found
            if (current_len >= rs_len &&
                strncmp(&ctx->record_buffer[current_len - rs_len], record_separator, rs_len) == 0) {
                break; // Separator found
            }
        }
        // If loop finished because buffer is full and separator not found
        if (current_len == 0xFFF && (current_len < rs_len ||
                                      strncmp(&ctx->record_buffer[current_len - rs_len], record_separator, rs_len) != 0)) {
            // Buffer overflow or separator not found within limit
            return false;
        }

        ctx->record_buffer[current_len - rs_len] = '\0'; // Null-terminate before the separator
        ctx->current_record_ptr = ctx->record_buffer; // Set current record pointer

        // If separator was newline and the resulting string is empty, continue reading
        if (rs_is_newline && strlen(ctx->record_buffer) == 0) {
            continue;
        }
        return true;
    } while (true);
}

// Function: free_fields
void free_fields(ProgramContext *ctx) {
    if (ctx->fields_array != NULL) {
        for (unsigned int i = 0; i < ctx->num_fields; ++i) {
            if (ctx->fields_array[i] != NULL) {
                free(ctx->fields_array[i]);
            }
        }
        free(ctx->fields_array);
        ctx->fields_array = NULL;
    }
    ctx->num_fields = 0;
}

// Function: read_fields
bool read_fields(ProgramContext *ctx) {
    char *field_separator = get_string(ctx, (int)(intptr_t)DAT_0001b099);
    if (field_separator == NULL) {
        return false;
    }
    size_t fs_len = strlen(field_separator);

    free_fields(ctx);

    ctx->current_record_ptr = ctx->record_buffer;

    if (*ctx->record_buffer == '\0') { // Empty record, consider it one empty field
        ctx->num_fields = 1;
        ctx->fields_array = calloc(1, sizeof(char *));
        if (ctx->fields_array == NULL) return false;
        ctx->fields_array[0] = strdup("");
        return ctx->fields_array[0] != NULL;
    }

    unsigned int field_count = 0;
    int current_char_idx = 0;
    // First pass to count fields
    while (ctx->record_buffer[current_char_idx] != '\0') {
        if (fs_len == 0) {
            field_count++;
            current_char_idx++;
        } else {
            if (strncmp(&ctx->record_buffer[current_char_idx], field_separator, fs_len) == 0) {
                field_count++;
                current_char_idx += fs_len;
            } else {
                current_char_idx++;
            }
        }
    }
    field_count++; // Account for the last field

    if (field_count >= 0x10000) { // Max fields limit
        return false;
    }

    ctx->num_fields = field_count;
    ctx->fields_array = calloc(ctx->num_fields, sizeof(char *));
    if (ctx->fields_array == NULL) {
        return false;
    }

    unsigned int current_field_idx = 0;
    int field_start_idx = 0;
    current_char_idx = 0;
    // Second pass to extract fields
    while (ctx->record_buffer[current_char_idx] != '\0' && current_field_idx < ctx->num_fields) {
        if (fs_len == 0) {
            char *field_str = malloc(2); // 1 char + null terminator
            if (field_str == NULL) {
                free_fields(ctx);
                return false;
            }
            field_str[0] = ctx->record_buffer[current_char_idx];
            field_str[1] = '\0';
            ctx->fields_array[current_field_idx++] = field_str;
            current_char_idx++;
            field_start_idx = current_char_idx; // For next field
        } else {
            if (strncmp(&ctx->record_buffer[current_char_idx], field_separator, fs_len) == 0) {
                size_t field_len = current_char_idx - field_start_idx;
                char *field_str = malloc(field_len + 1);
                if (field_str == NULL) {
                    free_fields(ctx);
                    return false;
                }
                memcpy(field_str, &ctx->record_buffer[field_start_idx], field_len);
                field_str[field_len] = '\0';
                ctx->fields_array[current_field_idx++] = field_str;

                current_char_idx += fs_len;
                field_start_idx = current_char_idx;
            } else {
                current_char_idx++;
            }
        }
    }

    // Add the last field (or the only field if no separators)
    if (current_field_idx < ctx->num_fields) {
        size_t field_len = current_char_idx - field_start_idx;
        char *field_str = malloc(field_len + 1);
        if (field_str == NULL) {
            free_fields(ctx);
            return false;
        }
        memcpy(field_str, &ctx->record_buffer[field_start_idx], field_len);
        field_str[field_len] = '\0';
        ctx->fields_array[current_field_idx++] = field_str;
    }

    return true;
}

// Function: combine_fields
bool combine_fields(ProgramContext *ctx) {
    char *output_field_separator = get_string(ctx, (int)(intptr_t)DAT_0001b09c);
    if (output_field_separator == NULL) {
        return false;
    }
    size_t ofs_len = strlen(output_field_separator);

    if (ctx->fields_array == NULL || ctx->num_fields == 0) {
        // No fields to combine, result is an empty string
        ctx->record_buffer[0] = '\0';
        ctx->current_record_ptr = ctx->record_buffer;
        return true;
    }

    size_t total_len = 0;
    for (unsigned int i = 0; i < ctx->num_fields; ++i) {
        if (ctx->fields_array[i] != NULL) {
            total_len += strlen(ctx->fields_array[i]);
        }
        if (i < ctx->num_fields - 1) { // Add separator length for all but the last field
            total_len += ofs_len;
        }
    }

    if (total_len >= 0x1000) { // Check against record buffer size (0xFFF for safety + null terminator)
        return false;
    }

    char *current_pos = ctx->record_buffer;
    for (unsigned int i = 0; i < ctx->num_fields; ++i) {
        if (i > 0) {
            memcpy(current_pos, output_field_separator, ofs_len);
            current_pos += ofs_len;
        }
        if (ctx->fields_array[i] != NULL) {
            size_t field_len = strlen(ctx->fields_array[i]);
            memcpy(current_pos, ctx->fields_array[i], field_len);
            current_pos += field_len;
        }
    }
    *current_pos = '\0'; // Null-terminate the combined string
    ctx->current_record_ptr = ctx->record_buffer; // Point to the combined string
    return true;
}

// Function: get_field
char *get_field(ProgramContext *ctx, unsigned int field_idx) {
    // If fields haven't been parsed yet, and it's not field 0, try to parse them.
    if (ctx->fields_array == NULL && field_idx != 0) {
        if (!read_fields(ctx)) {
            return NULL;
        }
    }

    if (field_idx == 0) {
        // Field 0 represents the entire record.
        // If current_record_ptr is not set (meaning fields were modified or not combined),
        // combine them first.
        if (ctx->current_record_ptr == NULL || ctx->current_record_ptr != ctx->record_buffer) {
            if (!combine_fields(ctx)) {
                return NULL;
            }
        }
        return ctx->current_record_ptr;
    } else {
        // For 1-based indexing, adjust to 0-based.
        unsigned int actual_idx = field_idx - 1;
        if (ctx->fields_array == NULL || actual_idx >= ctx->num_fields) {
            return DAT_0001b093; // Return empty string for out-of-bounds field
        }
        char *field_str = ctx->fields_array[actual_idx];
        return (field_str == NULL) ? DAT_0001b093 : field_str;
    }
}

// Function: set_field
bool set_field(ProgramContext *ctx, unsigned int field_idx, char *new_value) {
    if (field_idx >= 0x10001) { // Check max field index
        return false;
    }

    // If fields haven't been parsed yet, and it's not field 0, try to parse them.
    if (ctx->fields_array == NULL && field_idx != 0) {
        if (!read_fields(ctx)) {
            return false;
        }
    }

    if (field_idx == 0) {
        // Setting field 0 means setting the entire record.
        size_t value_len = strlen(new_value);
        if (value_len >= 0x1000) { // Check against record buffer size
            return false;
        }
        strcpy(ctx->record_buffer, new_value);
        free_fields(ctx); // Invalidate existing fields
        ctx->current_record_ptr = ctx->record_buffer; // Point to the new record
    } else {
        // For 1-based indexing, adjust to 0-based.
        unsigned int actual_idx = field_idx - 1;

        // Resize fields_array if necessary
        if (actual_idx >= ctx->num_fields) {
            // Reallocate to hold up to actual_idx + 1 fields
            char **new_fields_array = realloc(ctx->fields_array, (actual_idx + 1) * sizeof(char *));
            if (new_fields_array == NULL) {
                return false;
            }
            ctx->fields_array = new_fields_array;
            // Initialize new elements to NULL
            memset(&ctx->fields_array[ctx->num_fields], 0, (actual_idx + 1 - ctx->num_fields) * sizeof(char *));
            ctx->num_fields = actual_idx + 1;
        }

        // Free old field string if it exists
        if (ctx->fields_array[actual_idx] != NULL) {
            free(ctx->fields_array[actual_idx]);
        }

        char *new_str = strdup(new_value);
        if (new_str == NULL) {
            ctx->fields_array[actual_idx] = NULL; // Mark as NULL on failure
            return false;
        }
        ctx->fields_array[actual_idx] = new_str;
        ctx->current_record_ptr = NULL; // Invalidate combined record, needs re-combination
    }
    return true;
}

// Function: set_result_string
bool set_result_string(ProgramContext *ctx, char *str_val) {
    free_var((int)(intptr_t)&ctx->result_var); // Free previous result's contents
    ctx->result_var.type = VAR_TYPE_STRING;
    ctx->result_var.value.string_val = str_val; // Takes ownership of str_val
    ctx->result_var.number_str_cache = NULL;
    return true;
}

// Function: set_result_number
bool set_result_number(ProgramContext *ctx, int num_val) {
    free_var((int)(intptr_t)&ctx->result_var); // Free previous result's contents
    ctx->result_var.type = VAR_TYPE_NUMBER;
    ctx->result_var.value.number_val = num_val;
    ctx->result_var.number_str_cache = NULL;
    return true;
}

// Function: set_result_var
bool set_result_var(ProgramContext *ctx, Var *src_var) {
    if (src_var == NULL) {
        free_var((int)(intptr_t)&ctx->result_var);
        ctx->result_var.type = VAR_TYPE_EMPTY;
        return true;
    }
    // Original code copies string, so we perform a copy here, not a move.
    free_var((int)(intptr_t)&ctx->result_var); // Free previous result's contents
    ctx->result_var.type = src_var->type;
    ctx->result_var.number_str_cache = NULL; // Result var is fresh, no cache yet

    if (src_var->type == VAR_TYPE_NUMBER) {
        ctx->result_var.value.number_val = src_var->value.number_val;
    } else if (src_var->type == VAR_TYPE_STRING) {
        char *new_str = strdup(src_var->value.string_val);
        if (new_str == NULL) {
            ctx->result_var.type = VAR_TYPE_EMPTY; // Clear type on allocation failure
            return false;
        }
        ctx->result_var.value.string_val = new_str;
    } else { // VAR_TYPE_EMPTY or UNKNOWN
        ctx->result_var.type = VAR_TYPE_EMPTY;
    }
    return true;
}

// Function: assign_result
bool assign_result(ProgramContext *ctx, int *target_expr) {
    int target_type = target_expr[0]; // First element is type
    int target_key = target_expr[3]; // Third element is key/index for assignment

    if (target_type == 4 || target_type == 5) { // Field assignment
        unsigned int field_index;
        if (target_type == 4) { // Literal field index
            field_index = target_key;
        } else { // Field index from a variable (target_key is the key to the var holding the index)
            int num_val;
            if (!get_number(ctx, target_key, &num_val)) {
                return false;
            }
            field_index = num_val;
        }

        char buffer[24]; // Buffer for number to string conversion
        char *str_to_assign = NULL;

        if (ctx->result_var.type == VAR_TYPE_NUMBER) {
            to_string_buf(ctx->result_var.value.number_val, buffer);
            str_to_assign = buffer;
        } else if (ctx->result_var.type == VAR_TYPE_STRING) {
            str_to_assign = ctx->result_var.value.string_val;
        } else { // VAR_TYPE_EMPTY
            str_to_assign = DAT_0001b093; // Assign empty string for empty result
        }

        return set_field(ctx, field_index, str_to_assign);

    } else if (target_type == 6) { // Variable assignment
        Var *target_var = get_var(ctx, target_key);
        if (target_var == NULL) {
            return false;
        }

        // Free contents of target_var before copying
        free_var((int)(intptr_t)target_var);

        target_var->type = ctx->result_var.type;
        target_var->number_str_cache = NULL; // New var, no cache yet

        if (ctx->result_var.type == VAR_TYPE_NUMBER) {
            target_var->value.number_val = ctx->result_var.value.number_val;
        } else if (ctx->result_var.type == VAR_TYPE_STRING) {
            char *new_str = strdup(ctx->result_var.value.string_val);
            if (new_str == NULL) {
                target_var->type = VAR_TYPE_EMPTY; // Mark as empty on allocation failure
                return false;
            }
            target_var->value.string_val = new_str;
        } else {
            target_var->type = VAR_TYPE_EMPTY;
        }
        return true;
    }
    return false; // Unknown target type
}

// Function: compare_value
int compare_value(Var *var1, Var *var2) {
    // Handle NULL vars as empty
    if (var1 == NULL) var1 = (Var*)(intptr_t)VAR_TYPE_EMPTY; // Mock empty var
    if (var2 == NULL) var2 = (Var*)(intptr_t)VAR_TYPE_EMPTY; // Mock empty var

    char buf1[24], buf2[24]; // Buffers for number to string conversion
    char *s1_ptr, *s2_ptr;

    // Determine string representations for comparison
    if (var1->type == VAR_TYPE_STRING) {
        s1_ptr = var1->value.string_val;
    } else if (var1->type == VAR_TYPE_NUMBER) {
        to_string_buf(var1->value.number_val, buf1);
        s1_ptr = buf1;
    } else { // VAR_TYPE_EMPTY
        s1_ptr = DAT_0001b093;
    }

    if (var2->type == VAR_TYPE_STRING) {
        s2_ptr = var2->value.string_val;
    } else if (var2->type == VAR_TYPE_NUMBER) {
        to_string_buf(var2->value.number_val, buf2);
        s2_ptr = buf2;
    } else { // VAR_TYPE_EMPTY
        s2_ptr = DAT_0001b093;
    }
    
    // If both are numbers, compare numerically for efficiency
    if (var1->type == VAR_TYPE_NUMBER && var2->type == VAR_TYPE_NUMBER) {
        return var1->value.number_val - var2->value.number_val;
    }
    
    // Otherwise, compare as strings
    return strcmp(s1_ptr, s2_ptr);
}

// Function: do_concat
bool do_concat(ProgramContext *ctx, int *expr1, int *expr2) {
    Var temp_var1, temp_var2; // Stack-allocated temporary vars
    memset(&temp_var1, 0, sizeof(Var));
    memset(&temp_var2, 0, sizeof(Var));

    bool success = false;
    char *s1 = NULL, *s2 = NULL;
    char buf1[24], buf2[24]; // Buffers for number to string conversion

    if (!eval_expression(ctx, expr1)) {
        goto cleanup;
    }
    move_var(&temp_var1, &ctx->result_var); // Move result to temp_var1

    if (!eval_expression(ctx, expr2)) {
        goto cleanup;
    }
    move_var(&temp_var2, &ctx->result_var); // Move result to temp_var2

    // Get string representations for temp_var1
    if (temp_var1.type == VAR_TYPE_STRING) {
        s1 = temp_var1.value.string_val;
    } else if (temp_var1.type == VAR_TYPE_NUMBER) {
        to_string_buf(temp_var1.value.number_val, buf1);
        s1 = buf1;
    } else { // VAR_TYPE_EMPTY
        s1 = DAT_0001b093;
    }

    // Get string representations for temp_var2
    if (temp_var2.type == VAR_TYPE_STRING) {
        s2 = temp_var2.value.string_val;
    } else if (temp_var2.type == VAR_TYPE_NUMBER) {
        to_string_buf(temp_var2.value.number_val, buf2);
        s2 = buf2;
    } else { // VAR_TYPE_EMPTY
        s2 = DAT_0001b093;
    }

    // Should not happen with current logic, but defensive check
    if (s1 == NULL || s2 == NULL) {
        goto cleanup;
    }

    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    size_t total_len = len1 + len2 + 1; // +1 for null terminator

    char *combined_str = malloc(total_len);
    if (combined_str == NULL) {
        goto cleanup;
    }

    strcpy(combined_str, s1);
    strcat(combined_str, s2);

    success = set_result_string(ctx, combined_str); // set_result_string takes ownership of combined_str

cleanup:
    free_var((int)(intptr_t)&temp_var1); // Free contents of stack variable
    free_var((int)(intptr_t)&temp_var2); // Free contents of stack variable
    return success;
}

// Function: do_match
bool do_match(ProgramContext *ctx, int *target_expr, int *pattern_expr) {
    char *text_to_match = NULL;
    char *pattern_str = NULL;
    char text_buf[24], pattern_buf[24]; // Buffers for number to string conversion

    // Determine text_to_match
    if (target_expr == NULL) { // No target expression, use field 0 (entire record)
        text_to_match = get_field(ctx, 0);
    } else {
        if (!eval_expression(ctx, target_expr)) return false;
        if (ctx->result_var.type == VAR_TYPE_STRING) {
            text_to_match = ctx->result_var.value.string_val;
        } else if (ctx->result_var.type == VAR_TYPE_NUMBER) {
            to_string_buf(ctx->result_var.value.number_val, text_buf);
            text_to_match = text_buf;
        }
    }

    if (text_to_match == NULL) {
        return set_result_number(ctx, 0); // No text, no match
    }

    // Determine pattern_str
    if (pattern_expr[0] == 3) { // Literal string pattern (expr_type 3 is a direct string value)
        pattern_str = (char *)(intptr_t)pattern_expr[3];
    } else {
        if (!eval_expression(ctx, pattern_expr)) return false;
        if (ctx->result_var.type == VAR_TYPE_STRING) {
            pattern_str = ctx->result_var.value.string_val;
        } else if (ctx->result_var.type == VAR_TYPE_NUMBER) {
            to_string_buf(ctx->result_var.value.number_val, pattern_buf);
            pattern_str = pattern_buf;
        }
    }

    if (pattern_str == NULL) {
        return set_result_number(ctx, 0); // No pattern, no match
    }

    char regexp_storage[28]; // Size 28 from decompiler local_5c
    if (regexp_init(regexp_storage, pattern_str) == 0) {
        return set_result_number(ctx, 0); // Regex init failed
    }

    int match_result = regexp_match(regexp_storage, text_to_match);
    regexp_free(regexp_storage); // Free regex resources

    return set_result_number(ctx, match_result != 0);
}

// Function: eval_expression
bool eval_expression(ProgramContext *ctx, int *expr_ptr) {
    if (expr_ptr == NULL) return false;

    int expr_type = expr_ptr[0];
    bool success = false;
    Var temp_var_op1; // For binary operations, stack-allocated
    memset(&temp_var_op1, 0, sizeof(Var));

    switch (expr_type) {
        case 1: { // Literal string
            char *str_val = strdup((char *)(intptr_t)expr_ptr[3]);
            if (str_val == NULL) return false;
            success = set_result_string(ctx, str_val);
            break;
        }
        case 2: { // Literal number
            success = set_result_number(ctx, expr_ptr[3]);
            break;
        }
        case 3: { // Match operator (pattern only, implicitly matches field 0)
            success = do_match(ctx, NULL, expr_ptr);
            break;
        }
        case 4: { // Field by literal index
            char *field_str = get_field(ctx, expr_ptr[3]);
            if (field_str == NULL) return false;
            char *new_str = strdup(field_str);
            if (new_str == NULL) return false;
            success = set_result_string(ctx, new_str);
            break;
        }
        case 5: { // Field by variable index
            int field_idx;
            if (!get_number(ctx, expr_ptr[3], &field_idx)) return false;
            char *field_str = get_field(ctx, field_idx);
            if (field_str == NULL) return false;
            char *new_str = strdup(field_str);
            if (new_str == NULL) return false;
            success = set_result_string(ctx, new_str);
            break;
        }
        case 6: { // Variable lookup
            Var *var = get_var(ctx, expr_ptr[3]);
            if (var == NULL) return false;
            success = set_result_var(ctx, var); // Copies the var's content
            break;
        }
        case 7: { // Assignment expression (e.g., a = b)
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[4])) return false;
            success = assign_result(ctx, (int *)(intptr_t)expr_ptr[3]);
            break;
        }
        case 8:  case 9:  case 10: case 0xB: case 0xC: // Assignment arithmetic (+=, -=, *=, /=, %=)
        case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: { // Pure arithmetic (+, -, *, /, %)
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[3])) goto cleanup;
            int op1_num = coerce_number(ctx, &ctx->result_var);
            move_var(&temp_var_op1, &ctx->result_var); // Save first operand

            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[4])) goto cleanup;
            int op2_num = coerce_number(ctx, &ctx->result_var);

            int result_num = 0;
            switch (expr_type) {
                case 8: case 0x19: result_num = op1_num + op2_num; break;
                case 9: case 0x1A: result_num = op1_num - op2_num; break;
                case 10: case 0x1B: result_num = op1_num * op2_num; break;
                case 0xB: case 0x1C:
                    if (op2_num == 0) goto cleanup; // Division by zero
                    result_num = op1_num / op2_num;
                    break;
                case 0xC: case 0x1D:
                    if (op2_num == 0) goto cleanup; // Modulo by zero
                    result_num = op1_num % op2_num;
                    break;
            }
            if (!set_result_number(ctx, result_num)) goto cleanup;

            if (expr_type >= 8 && expr_type <= 0xC) { // If it was an assignment arithmetic op
                if (!assign_result(ctx, (int *)(intptr_t)expr_ptr[3])) goto cleanup;
            }
            success = true;
            break;
        }
        case 0xD: { // Ternary operator (cond ? true_expr : false_expr)
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[3])) return false;
            if (coerce_bool(ctx, &ctx->result_var)) {
                success = eval_expression(ctx, (int *)(intptr_t)expr_ptr[4]);
            } else {
                success = eval_expression(ctx, (int *)(intptr_t)expr_ptr[5]);
            }
            break;
        }
        case 0xE: { // Logical OR (||)
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[3])) return false;
            if (coerce_bool(ctx, &ctx->result_var)) {
                success = set_result_number(ctx, 1); // Short-circuit true
            } else {
                if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[4])) return false;
                success = set_result_number(ctx, coerce_bool(ctx, &ctx->result_var));
            }
            break;
        }
        case 0xF: { // Logical AND (&&)
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[3])) return false;
            if (!coerce_bool(ctx, &ctx->result_var)) {
                success = set_result_number(ctx, 0); // Short-circuit false
            } else {
                if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[4])) return false;
                success = set_result_number(ctx, coerce_bool(ctx, &ctx->result_var));
            }
            break;
        }
        case 0x10: case 0x11: { // Match (~) or No Match (!~)
            if (!do_match(ctx, (int *)(intptr_t)expr_ptr[3], (int *)(intptr_t)expr_ptr[4])) return false;
            if (expr_type == 0x11) { // No Match (!~), invert result
                ctx->result_var.value.number_val = (ctx->result_var.value.number_val == 0);
            }
            success = true;
            break;
        }
        case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: { // Comparison operators
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[3])) goto cleanup;
            move_var(&temp_var_op1, &ctx->result_var); // Save first operand

            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[4])) goto cleanup;
            // Second operand is in ctx->result_var

            int cmp_result = compare_value(&temp_var_op1, &ctx->result_var);
            bool condition_met = false;
            switch (expr_type) {
                case 0x12: condition_met = (cmp_result < 0); break; // <
                case 0x13: condition_met = (cmp_result > 0); break; // >
                case 0x14: condition_met = (cmp_result <= 0); break; // <=
                case 0x15: condition_met = (cmp_result >= 0); break; // >=
                case 0x16: condition_met = (cmp_result == 0); break; // ==
                case 0x17: condition_met = (cmp_result != 0); break; // !=
            }
            if (!set_result_number(ctx, condition_met)) goto cleanup;
            success = true;
            break;
        }
        case 0x18: { // Concatenation
            success = do_concat(ctx, (int *)(intptr_t)expr_ptr[3], (int *)(intptr_t)expr_ptr[4]);
            break;
        }
        case 0x1E: case 0x1F: { // Unary minus (-) or Logical NOT (!)
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[3])) return false;
            int result_num;
            if (expr_type == 0x1E) { // Unary minus
                result_num = -coerce_number(ctx, &ctx->result_var);
            } else { // Logical NOT
                result_num = !coerce_bool(ctx, &ctx->result_var);
            }
            success = set_result_number(ctx, result_num);
            break;
        }
        case 0x20: case 0x21: case 0x22: case 0x23: { // Pre/Post increment/decrement
            if (!eval_expression(ctx, (int *)(intptr_t)expr_ptr[3])) goto cleanup;
            move_var(&temp_var_op1, &ctx->result_var); // Save original value

            int original_num = coerce_number(ctx, &temp_var_op1);
            int new_num;
            if (expr_type == 0x20 || expr_type == 0x21) { // Increment (++, post or pre)
                new_num = original_num + 1;
            } else { // Decrement (--, post or pre)
                new_num = original_num - 1;
            }

            if (!set_result_number(ctx, new_num)) goto cleanup;
            if (!assign_result(ctx, (int *)(intptr_t)expr_ptr[3])) goto cleanup;

            if (expr_type == 0x21 || expr_type == 0x23) { // Post-increment/decrement, result is original value
                move_var(&ctx->result_var, &temp_var_op1); // Restore original value to result
            }
            success = true;
            break;
        }
        default:
            success = false;
            break;
    }

cleanup:
    free_var((int)(intptr_t)&temp_var_op1); // Free contents of stack variable
    return success;
}

// Function: do_print
bool do_print(ProgramContext *ctx, int *print_stmt_ptr) {
    int *format_expr = (int *)(intptr_t)print_stmt_ptr[2]; // print_stmt_ptr[2] is format expression
    int *args_list = (int *)(intptr_t)print_stmt_ptr[3];   // print_stmt_ptr[3] is arguments list

    bool success = false;

    if (format_expr == NULL) { // Standard print with default separators
        int *current_arg_ptr = args_list;
        while (current_arg_ptr != NULL) {
            if (!eval_expression(ctx, current_arg_ptr)) return false;

            if (ctx->result_var.type == VAR_TYPE_STRING) {
                fdprintf(1, DAT_0001b0a0, ctx->result_var.value.string_val);
            } else if (ctx->result_var.type == VAR_TYPE_NUMBER) {
                fdprintf(1, DAT_0001b090, ctx->result_var.value.number_val);
            } else { // VAR_TYPE_EMPTY
                // Do nothing for empty, or print empty string
            }

            current_arg_ptr = (int *)(intptr_t)current_arg_ptr[1]; // Next argument in linked list
            if (current_arg_ptr != NULL) { // Print OFS if not last argument
                char *ofs = get_string(ctx, (int)(intptr_t)DAT_0001b09c);
                if (ofs == NULL) ofs = DAT_0001b093; // Default to empty if OFS not found
                fdprintf(1, DAT_0001b0a0, ofs);
            }
        }

        if (args_list == NULL) { // No arguments, print field 0
            char *field0 = get_field(ctx, 0);
            if (field0 == NULL) field0 = DAT_0001b093;
            fdprintf(1, DAT_0001b0a0, field0);
        }

        char *ors = get_string(ctx, (int)(intptr_t)DAT_0001b134); // Output Record Separator
        if (ors == NULL) ors = DAT_0001b093; // Default to empty if ORS not found
        fdprintf(1, DAT_0001b0a0, ors);
        success = true;
    } else { // Print with custom format
        if (!eval_expression(ctx, format_expr)) return false;

        if (ctx->result_var.type == VAR_TYPE_NUMBER) { // Format is a number, print as int
            fdprintf(1, DAT_0001b090, ctx->result_var.value.number_val);
            success = true;
        } else if (ctx->result_var.type == VAR_TYPE_STRING) { // Format is a string, use eprintf
            Var format_var_copy;
            memset(&format_var_copy, 0, sizeof(Var));
            move_var(&format_var_copy, &ctx->result_var); // Save format string

            unsigned int arg_count = 0;
            for (int *arg_ptr = args_list; arg_ptr != NULL; arg_ptr = (int *)(intptr_t)arg_ptr[1]) {
                arg_count++;
            }

            Var *arg_vars = calloc(arg_count, sizeof(Var));
            if (arg_vars == NULL) {
                free_var((int)(intptr_t)&format_var_copy);
                return false;
            }

            unsigned int current_arg_idx = 0;
            for (int *arg_ptr = args_list; arg_ptr != NULL; arg_ptr = (int *)(intptr_t)arg_ptr[1]) {
                if (!eval_expression(ctx, arg_ptr)) {
                    // Free all allocated arg_vars before returning
                    for (unsigned int i = 0; i < current_arg_idx; ++i) {
                        free_var((int)(intptr_t)&arg_vars[i]);
                    }
                    free(arg_vars);
                    free_var((int)(intptr_t)&format_var_copy);
                    return false;
                }
                move_var(&arg_vars[current_arg_idx++], &ctx->result_var); // Move result to arg array
            }

            // Call eprintf. It expects the format string from the value field of the format Var.
            int print_status = eprintf(1, &format_var_copy.value, arg_vars, arg_count);
            success = (print_status >= 0); // Check if print was successful

            // Free arg_vars
            for (unsigned int i = 0; i < arg_count; ++i) {
                free_var((int)(intptr_t)&arg_vars[i]);
            }
            free(arg_vars);
            free_var((int)(intptr_t)&format_var_copy); // Free format var
        } else { // VAR_TYPE_EMPTY
            success = true; // Nothing to print explicitly
        }
    }
    return success;
}

// Function: eval_statements
int eval_statements(ProgramContext *ctx, int *stmt_list) {
    int *current_stmt = stmt_list;
    while (current_stmt != NULL) {
        int stmt_type = current_stmt[0];
        int result_code = 1; // 1 for continue, 2 for break, 3 for next, 4 for exit, 5 for return, 0 for error

        switch (stmt_type) {
            case 1: { // If statement (IF cond THEN body)
                if (!eval_expression(ctx, (int *)(intptr_t)current_stmt[2])) return 0;
                if (coerce_bool(ctx, &ctx->result_var)) {
                    result_code = eval_statements(ctx, (int *)(intptr_t)current_stmt[3]);
                    if (result_code != 1) return result_code;
                }
                break;
            }
            case 2: { // While statement (WHILE cond DO body)
                while (true) {
                    if (!eval_expression(ctx, (int *)(intptr_t)current_stmt[3])) return 0; // Evaluate condition
                    if (!coerce_bool(ctx, &ctx->result_var)) break; // If false, exit loop

                    result_code = eval_statements(ctx, (int *)(intptr_t)current_stmt[4]); // Execute body
                    if (result_code == 3) continue; // NEXT (skip to next iteration)
                    if (result_code == 2 || result_code == 4 || result_code == 5) return result_code; // BREAK, EXIT, RETURN
                    if (result_code == 0) return 0; // Error
                }
                break;
            }
            case 3: { // For statement (FOR init; cond; update; body)
                if (!eval_expression(ctx, (int *)(intptr_t)current_stmt[2])) return 0; // Init expression
                if (!eval_expression(ctx, (int *)(intptr_t)current_stmt[3])) return 0; // Initial condition evaluation

                while (coerce_bool(ctx, &ctx->result_var)) {
                    result_code = eval_statements(ctx, (int *)(intptr_t)current_stmt[5]); // Body
                    if (result_code == 3) { // NEXT, continue to update and re-evaluate condition
                        // No explicit action needed, loop will naturally proceed
                    } else if (result_code == 2 || result_code == 4 || result_code == 5) { // BREAK, EXIT, RETURN
                        break;
                    } else if (result_code == 0) { // Error
                        return 0;
                    }

                    if (!eval_expression(ctx, (int *)(intptr_t)current_stmt[4])) return 0; // Update expression
                    if (!eval_expression(ctx, (int *)(intptr_t)current_stmt[3])) return 0; // Re-evaluate condition
                }
                break;
            }
            case 5: return 2; // BREAK
            case 6: return 3; // NEXT
            case 7: return 4; // EXIT
            case 8: return 5; // RETURN (custom, not standard awk)
            case 9: { // PRINT statement
                if (!do_print(ctx, current_stmt)) return 0;
                break;
            }
            case 10: { // Expression statement (evaluate and discard result)
                if (!eval_expression(ctx, (int *)(intptr_t)current_stmt[2])) return 0;
                break;
            }
            // default: fallthrough for unknown statement types, or error
        }
        current_stmt = (int *)(intptr_t)current_stmt[1]; // Next statement in linked list
    }
    return 1; // Success, continue
}

// Function: program_run
bool program_run(int io_stream_fd, int *program_ast) {
    ProgramContext ctx;
    memset(&ctx, 0, sizeof(ProgramContext)); // Initialize all fields to 0
    ctx.self_ref_or_id = io_stream_fd; // Store initial_id (might be self_ref or other context ID)
    ctx.io_stream_handle = io_stream_fd; // Assuming initial_id is the stream handle

    if (dict_init(ctx.dict_storage, free_var) == 0) {
        return false;
    }

    bool overall_success = false;
    ctx.record_buffer = malloc(0x1000); // Allocate 4KB record buffer
    if (ctx.record_buffer == NULL) goto cleanup;

    // Initialize default variables for RS, FS, OFS, ORS
    struct { int key_ptr; const char *default_val; } defaults[] = {
        {(int)(intptr_t)DAT_0001b094, "\n"},
        {(int)(intptr_t)DAT_0001b134, "\n"},
        {(int)(intptr_t)DAT_0001b099, " "},
        {(int)(intptr_t)DAT_0001b09c, " "}
    };

    for (int i = 0; i < sizeof(defaults) / sizeof(defaults[0]); ++i) {
        char *str_val = strdup(defaults[i].default_val);
        if (str_val == NULL) goto cleanup;
        Var *var_obj = new_string(str_val);
        if (var_obj == NULL) {
            free(str_val); // new_string failed, free strdup'd string
            goto cleanup;
        }
        if (dict_add(ctx.dict_storage, defaults[i].key_ptr, var_obj) == 0) {
            free_var((int)(intptr_t)var_obj); // dict_add failed, free the Var's contents
            free(var_obj); // And free the Var struct itself as it was malloc'd by new_string
            goto cleanup;
        }
    }

    // Execute BEGIN blocks
    int *current_block = program_ast;
    while (current_block != NULL) {
        if (current_block[0] == -2) { // -2 is BEGIN block type
            if (eval_statements(&ctx, (int *)(intptr_t)current_block[1]) == 0) goto cleanup; // Error
        }
        current_block = (int *)(intptr_t)current_block[2]; // Next block in AST list
    }

    // Main processing loop
    while (read_record(&ctx)) {
        current_block = program_ast;
        while (current_block != NULL) {
            if (current_block[0] != 0 && current_block[0] != -2 && current_block[0] != -3) { // Normal blocks
                int *condition_expr = (int *)(intptr_t)current_block[0];
                int *body_stmt = (int *)(intptr_t)current_block[1];

                bool condition_met = true;
                if (condition_expr != NULL) { // If there's a condition
                    if (!eval_expression(&ctx, condition_expr)) goto cleanup; // Error in condition
                    condition_met = coerce_bool(&ctx, &ctx.result_var);
                }

                if (condition_met) {
                    int stmt_result = eval_statements(&ctx, body_stmt);
                    if (stmt_result == 4) goto end_program_loop; // EXIT (statement code 4)
                    if (stmt_result == 0) goto cleanup; // Error
                }
            }
            current_block = (int *)(intptr_t)current_block[2];
        }
        free_fields(&ctx); // Free fields after each record
    }

end_program_loop:
    // Execute END blocks
    current_block = program_ast;
    while (current_block != NULL) {
        if (current_block[0] == -3) { // -3 is END block type
            if (eval_statements(&ctx, (int *)(intptr_t)current_block[1]) == 0) goto cleanup; // Error
        }
        current_block = (int *)(intptr_t)current_block[2];
    }

    overall_success = true;

cleanup:
    free_fields(&ctx); // Free any remaining fields
    if (ctx.record_buffer != NULL) {
        free(ctx.record_buffer);
    }
    dict_free(ctx.dict_storage);
    free_var((int)(intptr_t)&ctx.result_var); // Free any remaining result var contents
    return overall_success;
}