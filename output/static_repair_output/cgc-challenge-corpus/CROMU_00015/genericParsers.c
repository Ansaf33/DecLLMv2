#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For malloc, free, atof, atoi
#include <string.h>  // For strcmp, strlen, strncpy
#include <ctype.h>   // For isspace, isalpha, isalnum

// --- Dummy/Placeholder Context and Helper Functions ---
// Assuming param_1 is a pointer to a struct that holds the input string
// and its current parsing position.
typedef struct {
    const char *buffer; // The input string to parse
    int current_pos;    // The current reading position in the buffer
    int buffer_len;     // Length of the buffer, for bounds checking
} Context;

// Dummy allocate function using malloc
// Returns 0 on success, -1 on failure
int allocate(size_t size, int some_flag, void **ptr) {
    *ptr = malloc(size);
    if (*ptr == NULL) {
        fprintf(stderr, "Allocation failed for size %zu\n", size);
        return -1; // Indicate failure
    }
    // Initialize to zero, as the original code often implies NULL or zeroed memory
    memset(*ptr, 0, size);
    (void)some_flag; // Suppress unused parameter warning
    return 0; // Indicate success
}

// Dummy deallocate function using free
void deallocate(void *ptr, size_t size) {
    if (ptr) {
        free(ptr);
    }
    // size is not strictly needed for free, but kept for signature match
    (void)size; // Suppress unused parameter warning
}

// Dummy skipWhiteSpace function
// Returns the new position or -1 on error (e.g., null context)
int skipWhiteSpace(Context *ctx) {
    if (!ctx || !ctx->buffer) return -1; // Error condition
    while (ctx->current_pos < ctx->buffer_len && isspace((unsigned char)ctx->buffer[ctx->current_pos])) {
        ctx->current_pos++;
    }
    return ctx->current_pos; // Return new position
}

// Dummy atChar function
// Returns 1 if char matches, 0 otherwise
int atChar(Context *ctx, char c) {
    if (!ctx || !ctx->buffer || ctx->current_pos >= ctx->buffer_len) return 0; // 0 for false
    return (ctx->buffer[ctx->current_pos] == c); // 1 for true, 0 for false
}

// Dummy skipLength function
// Returns the new position or -1 on error (e.g., out of bounds)
int skipLength(Context *ctx, int len) {
    if (!ctx || !ctx->buffer || ctx->current_pos + len > ctx->buffer_len || ctx->current_pos + len < 0) return -1;
    ctx->current_pos += len;
    return ctx->current_pos; // Return new position
}

// Dummy skipAlpha function
// Returns the end position of the alpha sequence, or -1 if no alpha char found
int skipAlpha(Context *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start_alpha = ctx->current_pos;
    while (ctx->current_pos < ctx->buffer_len && isalpha((unsigned char)ctx->buffer[ctx->current_pos])) {
        ctx->current_pos++;
    }
    if (ctx->current_pos == start_alpha) return -1; // No alpha character found
    return ctx->current_pos; // Return end position
}

// Dummy copyData function
// Returns a dynamically allocated string, or NULL on error
char *copyData(Context *ctx, int start, int end) {
    if (!ctx || !ctx->buffer || start < 0 || end > ctx->buffer_len || start > end) return NULL;
    size_t len = end - start;
    char *new_str = (char *)malloc(len + 1);
    if (!new_str) {
        fprintf(stderr, "Memory allocation failed in copyData\n");
        return NULL;
    }
    strncpy(new_str, ctx->buffer + start, len);
    new_str[len] = '\0';
    return new_str;
}

// Dummy skipFloat function
// Returns the end position of the float, or -1 if no valid float found
int skipFloat(Context *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start_float = ctx->current_pos;
    int digit_count = 0;

    // Optional sign
    if (ctx->current_pos < ctx->buffer_len && (ctx->buffer[ctx->current_pos] == '-' || ctx->buffer[ctx->current_pos] == '+')) {
        ctx->current_pos++;
    }

    // Digits before decimal
    while (ctx->current_pos < ctx->buffer_len && isdigit((unsigned char)ctx->buffer[ctx->current_pos])) {
        ctx->current_pos++;
        digit_count++;
    }

    // Decimal point and digits after
    if (ctx->current_pos < ctx->buffer_len && ctx->buffer[ctx->current_pos] == '.') {
        ctx->current_pos++;
        while (ctx->current_pos < ctx->buffer_len && isdigit((unsigned char)ctx->buffer[ctx->current_pos])) {
            ctx->current_pos++;
            digit_count++;
        }
    }

    // Exponent (e or E)
    if (ctx->current_pos < ctx->buffer_len && (ctx->buffer[ctx->current_pos] == 'e' || ctx->buffer[ctx->current_pos] == 'E')) {
        ctx->current_pos++;
        // Optional sign for exponent
        if (ctx->current_pos < ctx->buffer_len && (ctx->buffer[ctx->current_pos] == '-' || ctx->buffer[ctx->current_pos] == '+')) {
            ctx->current_pos++;
        }
        // Digits for exponent
        int exp_digit_count = 0;
        while (ctx->current_pos < ctx->buffer_len && isdigit((unsigned char)ctx->buffer[ctx->current_pos])) {
            ctx->current_pos++;
            exp_digit_count++;
        }
        if (exp_digit_count == 0) return -1; // Exponent sign without digits is invalid
    }

    if (ctx->current_pos == start_float || digit_count == 0) return -1; // No actual number found
    return ctx->current_pos; // Return end position
}

// Dummy skipInt function
// Returns the end position of the int, or -1 if no valid int found
int skipInt(Context *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start_int = ctx->current_pos;

    // Optional sign
    if (ctx->current_pos < ctx->buffer_len && (ctx->buffer[ctx->current_pos] == '-' || ctx->buffer[ctx->current_pos] == '+')) {
        ctx->current_pos++;
    }

    // Digits
    int digit_count = 0;
    while (ctx->current_pos < ctx->buffer_len && isdigit((unsigned char)ctx->buffer[ctx->current_pos])) {
        ctx->current_pos++;
        digit_count++;
    }
    if (ctx->current_pos == start_int || digit_count == 0) return -1; // No integer found
    return ctx->current_pos; // Return end position
}

// Dummy skipToNonAlphaNum function
// Returns the end position of the alphanumeric sequence, or -1 if no alphanumeric char found
int skipToNonAlphaNum(Context *ctx) {
    if (!ctx || !ctx->buffer) return -1;
    int start_pos = ctx->current_pos;
    while (ctx->current_pos < ctx->buffer_len && isalnum((unsigned char)ctx->buffer[ctx->current_pos])) {
        ctx->current_pos++;
    }
    if (ctx->current_pos == start_pos) return -1; // No alphanumeric character found
    return ctx->current_pos; // Return end position
}


// Function: extractBorder
double *extractBorder(Context *ctx) {
    double *border_data = NULL;
    char *temp_str = NULL;
    int start_idx, end_idx;

    if (!ctx) {
        return NULL;
    }

    // Allocate space for 4 doubles (sizeof(double) * 4 = 32 bytes)
    if (allocate(sizeof(double) * 4, 0, (void **)&border_data) != 0) {
        return NULL; // Allocation failed
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '{')) {
        printf("!!Failed to locate opening brace\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip opening brace\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }

    skipWhiteSpace(ctx);
    start_idx = ctx->current_pos;
    end_idx = skipAlpha(ctx);
    if (end_idx == -1) {
        printf("!!Failed to locate the end of the element id\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    
    temp_str = copyData(ctx, start_idx, end_idx);
    if (temp_str == NULL) {
        printf("!!Copy from %d to %d failed\n", start_idx, end_idx);
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    if (strcmp(temp_str, "Border") != 0) {
        printf("!!Element id is not \"Border\"\n");
        deallocate(temp_str, strlen(temp_str) + 1);
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL; // Clear pointer after deallocation

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '}')) {
        printf("!!Failed to locate initial closing brace\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip initial closing brace\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }

    // Parse 4 doubles
    const char* float_names[] = {"first lat", "first long", "second lat", "second long"};
    for (int i = 0; i < 4; ++i) {
        skipWhiteSpace(ctx);
        start_idx = ctx->current_pos;
        end_idx = skipFloat(ctx);
        
        if (start_idx == end_idx) { // No characters were skipped for the float
            printf("!!Failed to locate %s\n", float_names[i]);
            deallocate(border_data, sizeof(double) * 4);
            return NULL;
        }
        if (end_idx == -1) { // skipFloat returned an error
            printf("!!Failed to locate the end of the %s float\n", float_names[i]);
            deallocate(border_data, sizeof(double) * 4);
            return NULL;
        }

        temp_str = copyData(ctx, start_idx, end_idx);
        if (temp_str == NULL) {
            printf("!!Failed to copy %s float\n", float_names[i]);
            deallocate(border_data, sizeof(double) * 4);
            return NULL;
        }
        border_data[i] = atof(temp_str);
        deallocate(temp_str, (end_idx - start_idx) + 1); // Deallocate copied string
        temp_str = NULL;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '{')) {
        printf("!!Failed to locate the final opening brace\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip the final opening brace\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '#')) {
        printf("!!Failed to locate the closing mark\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) { // This advances past '#'
        printf("!!Failed to skip closing mark\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }

    start_idx = ctx->current_pos; // After '#'
    end_idx = skipAlpha(ctx);
    if (end_idx == -1) {
        printf("!!Failed to locate the end of the closing element id\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }

    temp_str = copyData(ctx, start_idx, end_idx);
    if (temp_str == NULL) {
        printf("!!Failed to copy closing element id\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    if (strcmp(temp_str, "Border") != 0) {
        printf("!!Invalid closing element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '}')) {
        printf("!!Failed to locate final closing brace\n");
        deallocate(border_data, sizeof(double) * 4);
        return NULL;
    }
    skipLength(ctx, 1); // Skip final closing brace

    return border_data;
}

// Function: extractPopulation
int extractPopulation(Context *ctx) {
    char *temp_str = NULL;
    int start_idx, end_idx;
    int population_start_idx; // Variable for the start of population data

    if (!ctx) {
        return -1;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '{')) {
        printf("!!Failed to locate opening brace\n");
        return -1;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip opening brace\n");
        return -1;
    }

    skipWhiteSpace(ctx);
    start_idx = ctx->current_pos;
    end_idx = skipAlpha(ctx);
    if (end_idx == -1) {
        printf("!!Failed to locate the end of the element id\n");
        return -1;
    }

    temp_str = copyData(ctx, start_idx, end_idx);
    if (temp_str == NULL) {
        printf("!!Copy from %d to %d failed\n", start_idx, end_idx);
        return -1;
    }
    if (strcmp(temp_str, "Population") != 0) {
        printf("!!Element id is not \"Population\"\n");
        deallocate(temp_str, strlen(temp_str) + 1);
        return -1;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '}')) {
        printf("!!Failed to locate initial closing brace\n");
        return -1;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip initial closing brace\n");
        return -1;
    }

    skipWhiteSpace(ctx);
    population_start_idx = ctx->current_pos; // Store start for population data
    end_idx = skipInt(ctx); // `end_idx` now holds the end of the integer data
    if (end_idx == -1) {
        printf("!!Failed to locate the end of the population data\n");
        return -1;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '{')) {
        printf("!!Failed to locate the final opening brace\n");
        return -1;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip the final opening brace\n");
        return -1;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '#')) {
        printf("!!Failed to locate the closing mark\n");
        return -1;
    }
    if (skipLength(ctx, 1) == -1) { // This advances past '#'
        printf("!!Failed to skip closing mark\n");
        return -1;
    }

    start_idx = ctx->current_pos; // After '#'
    int closing_id_end_idx = skipAlpha(ctx); // Use a distinct variable name for the end of closing ID
    if (closing_id_end_idx == -1) {
        printf("!!Failed to locate the end of the closing element id\n");
        return -1;
    }

    temp_str = copyData(ctx, start_idx, closing_id_end_idx);
    if (temp_str == NULL) {
        printf("!!Failed to copy closing element id\n");
        return -1;
    }
    if (strcmp(temp_str, "Population") != 0) {
        printf("!!Invalid closing element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        return -1;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '}')) {
        printf("!!Failed to locate final closing brace\n");
        return -1;
    }
    skipLength(ctx, 1); // Skip final closing brace

    // Now, copy and convert the actual population data
    // The `end_idx` here is the one from `skipInt`
    temp_str = copyData(ctx, population_start_idx, end_idx); 
    if (temp_str == NULL) {
        printf("!!Failed to copy population data\n");
        return -1;
    }
    int population_data = atoi(temp_str);
    deallocate(temp_str, strlen(temp_str) + 1);
    return population_data;
}

// Function: extractName
char *extractName(Context *ctx) { // Changed return type to char*
    char *name_data = NULL;
    char *temp_str = NULL;
    int start_idx, end_idx;
    int name_data_start_idx; // Variable for the start of name data

    if (!ctx) {
        return NULL;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '{')) {
        printf("!!Failed to locate opening brace\n");
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip opening brace\n");
        return NULL;
    }

    skipWhiteSpace(ctx);
    start_idx = ctx->current_pos;
    end_idx = skipAlpha(ctx);
    if (end_idx == -1) {
        printf("!!Failed to locate the end of the element id\n");
        return NULL;
    }

    temp_str = copyData(ctx, start_idx, end_idx);
    if (temp_str == NULL) {
        printf("!!Copy from %d to %d failed\n", start_idx, end_idx);
        return NULL;
    }
    if (strcmp(temp_str, "Name") != 0) {
        printf("!!Element id is not \"Name\"\n");
        deallocate(temp_str, strlen(temp_str) + 1);
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '}')) {
        printf("!!Failed to locate initial closing brace\n");
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip initial closing brace\n");
        return NULL;
    }

    skipWhiteSpace(ctx);
    name_data_start_idx = ctx->current_pos; // Store start for name data
    end_idx = skipToNonAlphaNum(ctx); // `end_idx` now holds the end of the name data
    if (end_idx == -1) {
        printf("!!Failed to locate the end of the name data\n");
        return NULL;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '{')) {
        printf("!!Failed to locate the final opening brace\n");
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) {
        printf("!!Failed to skip the final opening brace\n");
        return NULL;
    }

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '#')) {
        printf("!!Failed to locate the closing mark\n");
        return NULL;
    }
    if (skipLength(ctx, 1) == -1) { // This advances past '#'
        printf("!!Failed to skip closing mark\n");
        return NULL;
    }

    start_idx = ctx->current_pos; // After '#'
    int closing_id_end_idx = skipAlpha(ctx);
    if (closing_id_end_idx == -1) {
        printf("!!Failed to locate the end of the closing element id\n");
        return NULL;
    }

    temp_str = copyData(ctx, start_idx, closing_id_end_idx);
    if (temp_str == NULL) {
        printf("!!Failed to copy closing element id\n");
        return NULL;
    }
    if (strcmp(temp_str, "Name") != 0) {
        printf("!!Invalid closing element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    if (skipWhiteSpace(ctx) == -1 || !atChar(ctx, '}')) {
        printf("!!Failed to locate final closing brace\n");
        return NULL;
    }
    skipLength(ctx, 1); // Skip final closing brace

    // Now, copy the actual name data
    // The `end_idx` here is the one from `skipToNonAlphaNum`
    name_data = copyData(ctx, name_data_start_idx, end_idx); 
    if (name_data == NULL) {
        printf("!!Failed to copy name data\n");
        return NULL;
    }
    return name_data;
}