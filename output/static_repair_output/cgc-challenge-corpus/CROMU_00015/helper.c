#include <stdio.h>    // For printf, fprintf
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For strlen, strncpy, strcmp, memset
#include <ctype.h>    // For isspace, isalpha

// Define a structure for the parser state for better readability
// Original: param_1[0] = buffer_ptr, param_1[1] = buffer_len, param_1[2] = current_pos
typedef struct {
    char *buffer;
    int length;
    int current_pos;
} ParserState;

// --- STUB FUNCTIONS (to make the provided code compilable) ---
// In a real scenario, these would be provided by other modules.

// Simulates allocation, returns 0 on success, non-zero on failure.
int allocate(size_t size, int some_flag, char **ptr) {
    (void)some_flag; // Unused parameter
    *ptr = (char *)malloc(size);
    if (*ptr == NULL) {
        fprintf(stderr, "Allocation failed for size %zu\n", size);
        return -1; // Indicate failure
    }
    return 0; // Indicate success
}

// Simulates deallocation
void deallocate(char *ptr, size_t size) {
    (void)size; // Unused parameter, size is often redundant for free
    if (ptr != NULL) {
        free(ptr);
    }
}

// Skips whitespace characters in the buffer
void skipWhiteSpace(ParserState *state) {
    if (state == NULL || state->buffer == NULL) {
        return;
    }
    while (state->current_pos < state->length &&
           isspace((unsigned char)state->buffer[state->current_pos])) {
        state->current_pos++;
    }
}

// Increments the current position, returns -1 if out of bounds
int incChar(ParserState *state) {
    if (state == NULL || state->buffer == NULL) {
        return -1;
    }
    if (state->current_pos < state->length) {
        state->current_pos++;
        return 0; // Success
    }
    return -1; // Failure (out of bounds)
}

// --- ORIGINAL FUNCTIONS, REFACTORED ---

// Function: pullNextElementName
char *pullNextElementName(ParserState *state) {
    char *result = NULL;
    int original_pos; 
    size_t element_length = 0;

    if (state == NULL || state->buffer == NULL) {
        return NULL;
    }

    original_pos = state->current_pos;

    skipWhiteSpace(state);

    if (state->current_pos >= state->length || state->buffer[state->current_pos] != '{') {
        printf("!!Invalid opening element: %c at position %d\n", 
               (state->current_pos < state->length ? state->buffer[state->current_pos] : '?'), 
               state->current_pos);
        // Restore position at the end, as per original snippet's logic
    } else if (incChar(state) == -1) { // Increment past '{'
        // Restore position at the end
    } else {
        skipWhiteSpace(state);

        int start_pos = state->current_pos;

        while (state->current_pos < state->length) {
            char current_char = state->buffer[state->current_pos];

            if (current_char == '\0') {
                printf("!!Null character hit. Improperly formatted element\n");
                break;
            }

            if (current_char == '}' || isspace((unsigned char)current_char)) {
                element_length = state->current_pos - start_pos;

                if (element_length == 0) {
                    printf("!!Empty element name found.\n");
                    break;
                }

                if (allocate(element_length + 1, 0, &result) != 0) {
                    result = NULL; // Allocation failed
                    break;
                }
                
                memset(result, 0, element_length + 1);
                strncpy(result, state->buffer + start_pos, element_length);

                // state->current_pos is already at the position after the name
                skipWhiteSpace(state);

                if (state->current_pos >= state->length || state->buffer[state->current_pos] != '}') {
                    printf("!!Improperly formatted element name: missing closing '}'\n");
                    deallocate(result, element_length + 1);
                    result = NULL;
                } else {
                    incChar(state); // Advance past '}'
                }
                break; // Exit loop after processing element name
            }

            if (!isalpha((unsigned char)current_char)) {
                printf("!!Invalid character '%c' in element name at position %d\n", current_char, state->current_pos);
                break;
            }
            
            state->current_pos++;
        }
    }

    // Restore the original position regardless of success or failure, as per original snippet.
    state->current_pos = original_pos;
    
    return result;
}

// Function: elementNameToEnum
int elementNameToEnum(char *element_name) {
    int enum_value = 0x19; // Default unknown value

    if (element_name == NULL) {
        return enum_value;
    }

    size_t len = strlen(element_name);

    switch (len) {
        case 3:
            if (strcmp(element_name, "Url") == 0) {
                enum_value = 0x18;
            }
            break;
        case 4:
            if (strcmp(element_name, "Name") == 0) {
                enum_value = 1;
            } else if (strcmp(element_name, "Mass") == 0) {
                enum_value = 8;
            } else if (strcmp(element_name, "Area") == 0) {
                enum_value = 0x14;
            } else if (strcmp(element_name, "Seat") == 0) {
                enum_value = 0x13;
            } else if (strcmp(element_name, "City") == 0) {
                enum_value = 0x16;
            }
            break;
        case 5:
            if (strcmp(element_name, "Mayor") == 0) {
                enum_value = 0x17;
            }
            break;
        case 6:
            if (strcmp(element_name, "Planet") == 0) {
                enum_value = 0;
            } else if (strcmp(element_name, "Period") == 0) {
                enum_value = 2;
            } else if (strcmp(element_name, "Radius") == 0) {
                enum_value = 6;
            } else if (strcmp(element_name, "Border") == 0) {
                enum_value = 0xe;
            } else if (strcmp(element_name, "County") == 0) {
                enum_value = 0x12;
            }
            break;
        case 7:
            if (strcmp(element_name, "ERadius") == 0) {
                enum_value = 7;
            } else if (strcmp(element_name, "Gravity") == 0) {
                enum_value = 9;
            } else if (strcmp(element_name, "Country") == 0) {
                enum_value = 0xb;
            } else if (strcmp(element_name, "Capitol") == 0) {
                enum_value = 0xc;
            } else if (strcmp(element_name, "Founder") == 0) {
                enum_value = 0x11;
            } else if (strcmp(element_name, "Density") == 0) {
                enum_value = 0x15;
            }
            break;
        case 8:
            if (strcmp(element_name, "Aphelion") == 0) {
                enum_value = 4;
            } else if (strcmp(element_name, "Language") == 0) {
                enum_value = 0xd;
            }
            break;
        case 9:
            if (strcmp(element_name, "Territory") == 0) {
                enum_value = 0xf;
            }
            break;
        case 10:
            if (strcmp(element_name, "OrbitSpeed") == 0) {
                enum_value = 3;
            } else if (strcmp(element_name, "Perihelion") == 0) {
                enum_value = 5;
            } else if (strcmp(element_name, "Population") == 0) {
                enum_value = 10;
            }
            break;
        case 11: // 0xb is 11 in decimal
            if (strcmp(element_name, "Established") == 0) {
                enum_value = 0x10;
            }
            break;
        default:
            break;
    }

    if (enum_value == 0x19) {
        printf("!!Unknown element id: %s\n", element_name);
    }
    return enum_value;
}