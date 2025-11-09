#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free, atof, atoi
#include <string.h>  // For strcmp, strlen, strncpy
#include <ctype.h>   // For isspace, isalpha, isdigit, isalnum

// Placeholder for the parser state structure.
// In the original snippet, 'param_1' is treated as an integer but then
// dereferenced as a pointer (e.g., '*(int *)(param_1 + 8)').
// This implies 'param_1' is a pointer to a custom structure, and 'param_1 + 8'
// accesses a member, likely the current parsing position within a buffer.
// We'll define a dummy struct to represent this.
typedef struct ParserState {
    char *data;        // The string buffer being parsed
    size_t length;     // Total length of the data
    int current_pos;   // Current reading position, corresponds to 'param_1 + 8'
} ParserState;

// --- Placeholder functions for memory management and parsing ---
// These are minimal implementations to make the provided code compilable.
// Their actual logic would depend on the system they originated from.

// Memory management
int allocate(size_t size, int flags, void **ptr) {
    (void)flags; // Suppress unused parameter warning for 'flags'
    *ptr = malloc(size);
    return (*ptr != NULL) ? 0 : -1;
}

void deallocate(void *ptr, size_t size) {
    (void)size; // Suppress unused parameter warning for 'size'
    free(ptr);
}

// Parsing functions (return 0 for success/found, -1 for failure/not found, or an index)
// 'state' is assumed to be ParserState*
void skipWhiteSpace(ParserState *state) {
    if (!state || !state->data) return;
    while (state->current_pos < state->length && isspace((unsigned char)state->data[state->current_pos])) {
        state->current_pos++;
    }
}

int atChar(ParserState *state, char c) {
    if (!state || !state->data || state->current_pos >= state->length) {
        return 0; // Not at char or end of data
    }
    return (state->data[state->current_pos] == c) ? 1 : 0;
}

int skipLength(ParserState *state, int len) {
    if (!state || !state->data || (state->current_pos + len) > state->length || (state->current_pos + len) < 0) {
        return -1; // Indicate failure (e.g., out of bounds)
    }
    state->current_pos += len;
    return state->current_pos; // Return new position
}

// Skips alphabetic characters. Returns the new current_pos or -1 on error.
int skipAlpha(ParserState *state) {
    if (!state || !state->data || state->current_pos >= state->length) return -1;
    int original_pos = state->current_pos;
    while (state->current_pos < state->length && isalpha((unsigned char)state->data[state->current_pos])) {
        state->current_pos++;
    }
    return (state->current_pos > original_pos) ? state->current_pos : -1; // -1 if no alpha found
}

// Skips float characters (digits, '.', optional '+', '-'). Returns the new current_pos or -1 on error.
int skipFloat(ParserState *state) {
    if (!state || !state->data || state->current_pos >= state->length) return -1;
    int original_pos = state->current_pos;
    int dot_found = 0;
    int digits_found = 0;

    // Optional sign
    if (state->current_pos < state->length && (state->data[state->current_pos] == '+' || state->data[state->current_pos] == '-')) {
        state->current_pos++;
    }
    // Digits before dot
    while (state->current_pos < state->length && isdigit((unsigned char)state->data[state->current_pos])) {
        state->current_pos++;
        digits_found = 1;
    }
    // Optional dot and digits after dot
    if (state->current_pos < state->length && state->data[state->current_pos] == '.') {
        dot_found = 1;
        state->current_pos++;
        while (state->current_pos < state->length && isdigit((unsigned char)state->data[state->current_pos])) {
            state->current_pos++;
            digits_found = 1;
        }
    }
    // Must have at least one digit if no dot, or at least one digit around the dot
    if (!digits_found && !dot_found) { // No digits and no dot
        return -1;
    }
    if (state->current_pos == original_pos) { // Nothing was skipped
        return -1;
    }
    return state->current_pos;
}

// Skips integer characters (digits, optional sign). Returns the new current_pos or -1 on error.
int skipInt(ParserState *state) {
    if (!state || !state->data || state->current_pos >= state->length) return -1;
    int original_pos = state->current_pos;
    // Optional sign
    if (state->current_pos < state->length && (state->data[state->current_pos] == '+' || state->data[state->current_pos] == '-')) {
        state->current_pos++;
    }
    int digits_found = 0;
    while (state->current_pos < state->length && isdigit((unsigned char)state->data[state->current_pos])) {
        state->current_pos++;
        digits_found = 1;
    }
    return (digits_found) ? state->current_pos : -1; // Must have at least one digit
}

// Skips alphanumeric characters. Returns the new current_pos or -1 on error.
int skipToNonAlphaNum(ParserState *state) {
    if (!state || !state->data || state->current_pos >= state->length) return -1;
    int original_pos = state->current_pos;
    while (state->current_pos < state->length && isalnum((unsigned char)state->data[state->current_pos])) {
        state->current_pos++;
    }
    return (state->current_pos > original_pos) ? state->current_pos : -1; // -1 if no alpha-num found
}

// Copies data from start to end position. Returns new allocated string or NULL.
char *copyData(ParserState *state, int start_pos, int end_pos) {
    if (!state || !state->data || start_pos < 0 || end_pos < start_pos || (size_t)end_pos > state->length) {
        return NULL;
    }
    size_t len = end_pos - start_pos;
    char *buffer = (char *)malloc(len + 1);
    if (buffer) {
        strncpy(buffer, state->data + start_pos, len);
        buffer[len] = '\0';
    }
    return buffer;
}
// --- End of placeholder functions ---

// Function: extractBorder
double * extractBorder(ParserState *state) {
    double *result_border = NULL;
    char *temp_str = NULL;
    int start_pos, end_pos;

    if (!state) {
        return NULL;
    }

    // Allocate memory for 4 doubles (0x20 bytes)
    if (allocate(4 * sizeof(double), 0, (void **)&result_border) != 0) {
        return NULL;
    }

    // Step 1: Locate and skip opening brace '{'
    skipWhiteSpace(state);
    if (atChar(state, '{') == 0) {
        printf("!!Failed to locate opening brace\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip opening brace\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }

    // Step 2: Read element ID "Border"
    skipWhiteSpace(state);
    start_pos = state->current_pos;
    end_pos = skipAlpha(state);
    if (end_pos == -1 || start_pos == end_pos) {
        printf("!!Failed to locate the end of the element id\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }

    temp_str = copyData(state, start_pos, end_pos);
    if (temp_str == NULL) {
        printf("!!Copy from %d to %d failed\n", start_pos, end_pos);
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    if (strcmp(temp_str, "Border") != 0) {
        printf("!!Element id is not \"Border\"\n");
        deallocate(temp_str, strlen(temp_str) + 1);
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL; // Clear pointer after deallocation

    // Step 3: Locate and skip initial closing brace '}'
    skipWhiteSpace(state);
    if (atChar(state, '}') == 0) {
        printf("!!Failed to locate initial closing brace\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip initial closing brace\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }

    // Macro to simplify reading float data, converting, and error handling
    #define READ_FLOAT_AND_STORE(index, locate_err_msg, copy_err_msg) \
        skipWhiteSpace(state); \
        start_pos = state->current_pos; \
        end_pos = skipFloat(state); \
        if (end_pos == -1 || start_pos == end_pos) { \
            printf("!!" locate_err_msg "\n"); \
            deallocate(result_border, 4 * sizeof(double)); \
            return NULL; \
        } \
        temp_str = copyData(state, start_pos, end_pos); \
        if (temp_str == NULL) { \
            printf("!!" copy_err_msg "\n"); \
            deallocate(result_border, 4 * sizeof(double)); \
            return NULL; \
        } \
        result_border[index] = atof(temp_str); \
        deallocate(temp_str, (size_t)(end_pos - start_pos) + 1); \
        temp_str = NULL;

    // Step 4: Read first latitude
    READ_FLOAT_AND_STORE(0, "Failed to locate first lat", "Failed to copy first latitude float");

    // Step 5: Read first longitude
    READ_FLOAT_AND_STORE(1, "Failed to locate first long", "Failed to copy first longitude float");

    // Step 6: Read second latitude
    READ_FLOAT_AND_STORE(2, "Failed to locate second lat", "Failed to copy second latitude float");

    // Step 7: Read second longitude
    READ_FLOAT_AND_STORE(3, "Failed to locate second long", "Failed to copy second longitude float");

    #undef READ_FLOAT_AND_STORE

    // Step 8: Locate and skip final opening brace '{'
    skipWhiteSpace(state);
    if (atChar(state, '{') == 0) {
        printf("!!Failed to locate the final opening brace\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip the final opening brace\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }

    // Step 9: Locate and skip closing mark '#'
    skipWhiteSpace(state);
    if (atChar(state, '#') == 0) {
        printf("!!Failed to locate the closing mark\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    start_pos = skipLength(state, 1); // start_pos now holds the position after '#'
    if (start_pos == -1) {
        printf("!!Failed to skip closing mark\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }

    // Step 10: Read closing element ID "Border"
    end_pos = skipAlpha(state);
    if (end_pos == -1) {
        printf("!!Failed to locate the end of the closing element id\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }

    temp_str = copyData(state, start_pos, end_pos);
    if (temp_str == NULL) {
        printf("!!Copy of closing element id failed\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    if (strcmp(temp_str, "Border") != 0) {
        printf("!!Invalid closing element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    // Step 11: Locate and skip final closing brace '}'
    skipWhiteSpace(state);
    if (atChar(state, '}') == 0) {
        printf("!!Failed to locate final closing brace\n");
        deallocate(result_border, 4 * sizeof(double));
        return NULL;
    }
    skipLength(state, 1); // Advance past '}'

    return result_border;
}

// Function: extractPopulation
int extractPopulation(ParserState *state) {
    int result_population = -1; // Default failure value
    char *temp_str = NULL;
    int start_pos, end_pos;

    if (!state) {
        return -1;
    }

    // Step 1: Locate and skip opening brace '{'
    skipWhiteSpace(state);
    if (atChar(state, '{') == 0) {
        printf("!!Failed to locate opening brace\n");
        return -1;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip opening brace\n");
        return -1;
    }

    // Step 2: Read element ID "Population"
    skipWhiteSpace(state);
    start_pos = state->current_pos;
    end_pos = skipAlpha(state);
    if (end_pos == -1 || start_pos == end_pos) {
        printf("!!Failed to locate the end of the element id\n");
        return -1;
    }

    temp_str = copyData(state, start_pos, end_pos);
    if (temp_str == NULL) {
        printf("!!Copy from %d to %d failed\n", start_pos, end_pos);
        return -1;
    }
    if (strcmp(temp_str, "Population") != 0) {
        printf("!!Element id is not \"Population\"\n");
        deallocate(temp_str, strlen(temp_str) + 1);
        return -1;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    // Step 3: Locate and skip initial closing brace '}'
    skipWhiteSpace(state);
    if (atChar(state, '}') == 0) {
        printf("!!Failed to locate initial closing brace\n");
        return -1;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip initial closing brace\n");
        return -1;
    }

    // Step 4: Read population data (integer)
    skipWhiteSpace(state);
    int population_start_pos = state->current_pos;
    int population_end_pos = skipInt(state);
    if (population_end_pos == -1 || population_start_pos == population_end_pos) {
        printf("!!Failed to locate the end of the population data\n");
        return -1;
    }

    // Step 5: Locate and skip final opening brace '{'
    skipWhiteSpace(state);
    if (atChar(state, '{') == 0) {
        printf("!!Failed to locate the final opening brace\n");
        return -1;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip the final opening brace\n");
        return -1;
    }

    // Step 6: Locate and skip closing mark '#'
    skipWhiteSpace(state);
    if (atChar(state, '#') == 0) {
        printf("!!Failed to locate the closing mark\n");
        return -1;
    }
    start_pos = skipLength(state, 1); // start_pos now holds the position after '#'
    if (start_pos == -1) {
        printf("!!Failed to skip closing mark\n");
        return -1;
    }

    // Step 7: Read closing element ID "Population"
    end_pos = skipAlpha(state);
    if (end_pos == -1) {
        printf("!!Failed to locate the end of the closing element id\n");
        return -1;
    }

    temp_str = copyData(state, start_pos, end_pos);
    if (temp_str == NULL) {
        printf("!!Copy of closing element id failed\n");
        return -1;
    }
    if (strcmp(temp_str, "Population") != 0) {
        printf("!!Invalid closing element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        return -1;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    // Step 8: Locate and skip final closing brace '}'
    skipWhiteSpace(state);
    if (atChar(state, '}') == 0) {
        printf("!!Failed to locate final closing brace\n");
        return -1;
    }
    skipLength(state, 1); // Advance past '}'

    // Final step: copy and convert population data
    temp_str = copyData(state, population_start_pos, population_end_pos);
    if (temp_str == NULL) {
        printf("!!Failed to copy population data\n");
        return -1;
    }
    result_population = atoi(temp_str);
    deallocate(temp_str, strlen(temp_str) + 1);

    return result_population;
}

// Function: extractName
char * extractName(ParserState *state) { // Changed return type from undefined4 to char*
    char *result_name = NULL; // Default failure value
    char *temp_str = NULL;
    int start_pos, end_pos;

    if (!state) {
        return NULL;
    }

    // Step 1: Locate and skip opening brace '{'
    skipWhiteSpace(state);
    if (atChar(state, '{') == 0) {
        printf("!!Failed to locate opening brace\n");
        return NULL;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip opening brace\n");
        return NULL;
    }

    // Step 2: Read element ID "Name"
    skipWhiteSpace(state);
    start_pos = state->current_pos;
    end_pos = skipAlpha(state);
    if (end_pos == -1 || start_pos == end_pos) {
        printf("!!Failed to locate the end of the element id\n");
        return NULL;
    }

    temp_str = copyData(state, start_pos, end_pos);
    if (temp_str == NULL) {
        printf("!!Copy from %d to %d failed\n", start_pos, end_pos);
        return NULL;
    }
    if (strcmp(temp_str, "Name") != 0) {
        printf("!!Element id is not \"Name\"\n");
        deallocate(temp_str, strlen(temp_str) + 1);
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    // Step 3: Locate and skip initial closing brace '}'
    skipWhiteSpace(state);
    if (atChar(state, '}') == 0) {
        printf("!!Failed to locate initial closing brace\n");
        return NULL;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip initial closing brace\n");
        return NULL;
    }

    // Step 4: Read name data (alphanumeric string)
    skipWhiteSpace(state);
    int name_start_pos = state->current_pos;
    int name_end_pos = skipToNonAlphaNum(state);
    if (name_end_pos == -1 || name_start_pos == name_end_pos) {
        printf("!!Failed to locate the end of the name data\n");
        return NULL;
    }

    // Step 5: Locate and skip final opening brace '{'
    skipWhiteSpace(state);
    if (atChar(state, '{') == 0) {
        printf("!!Failed to locate the final opening brace\n");
        return NULL;
    }
    if (skipLength(state, 1) == -1) {
        printf("!!Failed to skip the final opening brace\n");
        return NULL;
    }

    // Step 6: Locate and skip closing mark '#'
    skipWhiteSpace(state);
    if (atChar(state, '#') == 0) {
        printf("!!Failed to locate the closing mark\n");
        return NULL;
    }
    start_pos = skipLength(state, 1); // start_pos now holds the position after '#'
    if (start_pos == -1) {
        printf("!!Failed to skip closing mark\n");
        return NULL;
    }

    // Step 7: Read closing element ID "Name"
    end_pos = skipAlpha(state);
    if (end_pos == -1) {
        printf("!!Failed to locate the end of the closing element id\n");
        return NULL;
    }

    temp_str = copyData(state, start_pos, end_pos);
    if (temp_str == NULL) {
        printf("!!Copy of closing element id failed\n");
        return NULL;
    }
    if (strcmp(temp_str, "Name") != 0) {
        printf("!!Invalid closing element id: %s\n", temp_str);
        deallocate(temp_str, strlen(temp_str) + 1);
        return NULL;
    }
    deallocate(temp_str, strlen(temp_str) + 1);
    temp_str = NULL;

    // Step 8: Locate and skip final closing brace '}'
    skipWhiteSpace(state);
    if (atChar(state, '}') == 0) {
        printf("!!Failed to locate final closing brace\n");
        return NULL;
    }
    skipLength(state, 1); // Advance past '}'

    // Final step: copy name data
    result_name = copyData(state, name_start_pos, name_end_pos);
    if (result_name == NULL) {
        printf("!!Failed to copy name data\n");
    }

    return result_name;
}