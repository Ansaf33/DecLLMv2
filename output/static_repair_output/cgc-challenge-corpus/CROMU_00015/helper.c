#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free, perror
#include <string.h>  // For strlen, strcmp, strncpy, memset
#include <ctype.h>   // For isspace, isalpha
#include <stdint.h>  // For uintptr_t (for 64-bit safe pointer arithmetic)

// --- Stub Implementations for functions not supplied in the snippet ---
// These stubs are designed to match the inferred usage from the provided code
// and ensure compilation.
//
// The 'param_1' argument (type 'int *') is interpreted as an array:
//   param_1[0]: Base address of the character buffer (stored as an int, potentially truncated on 64-bit systems)
//   param_1[1]: End limit/length of the buffer (int)
//   param_1[2]: Current parsing offset (int)

void skipWhiteSpace(int *param_1) {
    if (param_1 == NULL) return;
    uintptr_t buffer_base = (uintptr_t)param_1[0];
    int buffer_limit = param_1[1];
    while (param_1[2] < buffer_limit && isspace((unsigned char)*(char *)(buffer_base + param_1[2]))) {
        param_1[2]++;
    }
}

int incChar(int *param_1) {
    if (param_1 == NULL) return -1;
    int buffer_limit = param_1[1];
    if (param_1[2] < buffer_limit) {
        param_1[2]++;
        return 0; // Success
    }
    return -1; // End of buffer
}

int allocate(size_t size, int flags, char **ptr) {
    *ptr = (char *)malloc(size);
    if (*ptr == NULL) {
        perror("Memory allocation failed");
        return 1; // Failure
    }
    // 'flags' parameter is ignored as per original snippet's usage (0)
    return 0; // Success
}

void deallocate(char *ptr, size_t size) {
    free(ptr);
    // 'size' parameter is ignored as per original snippet's usage
}

// Function: pullNextElementName
char *pullNextElementName(int *param_1) {
    char *result = NULL;
    size_t element_len = 0;
    int temp_int_val; // For return values of incChar, isspace, isalpha

    if (param_1 == NULL) {
        return NULL;
    }

    uintptr_t buffer_base = (uintptr_t)param_1[0];
    int buffer_limit = param_1[1];
    int initial_parse_pos = param_1[2];

    skipWhiteSpace(param_1);

    // Check for opening brace
    if (param_1[2] < buffer_limit && *(char *)(buffer_base + param_1[2]) == '{') {
        temp_int_val = incChar(param_1); // Move past '{'
        if (temp_int_val != -1) { // incChar successful
            skipWhiteSpace(param_1);

            int start_element_name_pos = param_1[2];

            // Loop to find the end of the element name
            for (int current_char_pos = start_element_name_pos; current_char_pos < buffer_limit; current_char_pos++) {
                char current_char = *(char *)(buffer_base + current_char_pos);

                if (current_char == '\0') {
                    printf("!!Null character hit. Improperly formatted element\n");
                    break;
                }

                // Check for '}' or whitespace
                temp_int_val = isspace((unsigned char)current_char);
                if (current_char == '}' || temp_int_val != 0) {
                    element_len = current_char_pos - start_element_name_pos;

                    // Handle empty element name case
                    if (element_len == 0) {
                        printf("!!Empty element name found.\n");
                        break;
                    }

                    temp_int_val = allocate(element_len + 1, 0, &result);
                    if (temp_int_val == 0) { // Allocation successful
                        memset(result, 0, element_len + 1);
                        strncpy(result, (char *)(buffer_base + start_element_name_pos), element_len);
                        param_1[2] = current_char_pos; // Update context position

                        skipWhiteSpace(param_1);

                        // Check for closing brace
                        if (param_1[2] < buffer_limit && *(char *)(buffer_base + param_1[2]) == '}') {
                            incChar(param_1); // Move past '}'
                        } else {
                            printf("!!Improperly formatted element name: expected '}'\n");
                            deallocate(result, element_len + 1);
                            result = NULL;
                        }
                    } else { // Allocation failed
                        result = NULL;
                    }
                    break; // Exit loop after processing element name
                }

                // Element names must be alphabetic
                temp_int_val = isalpha((unsigned char)current_char);
                if (temp_int_val == 0) {
                    printf("!!Non-alphabetic character in element name: %c\n", current_char);
                    break; // Exit loop, result remains NULL
                }
            }
        } else { // incChar failed (unexpected end of buffer after '{')
             printf("!!Unexpected end of buffer after '{'\n");
        }
    } else { // No opening brace found
        if (param_1[2] < buffer_limit) {
            printf("!!Invalid opening element: %c\n", *(char *)(buffer_base + param_1[2]));
        } else {
            printf("!!End of buffer reached, expected '{'\n");
        }
    }

    param_1[2] = initial_parse_pos; // Restore original position as per original snippet
    return result;
}

// Function: elementNameToEnum
int elementNameToEnum(char *param_1) {
    int element_enum = 25; // Default value 0x19
    if (param_1 == NULL) {
        return element_enum;
    }

    size_t len = strlen(param_1);
    int cmp_result;

    switch (len) {
        case 3:
            cmp_result = strcmp(param_1, "Url");
            if (cmp_result == 0) element_enum = 24;
            break;
        case 4:
            cmp_result = strcmp(param_1, "Name");
            if (cmp_result == 0) element_enum = 1;
            else {
                cmp_result = strcmp(param_1, "Mass");
                if (cmp_result == 0) element_enum = 8;
                else {
                    cmp_result = strcmp(param_1, "Area");
                    if (cmp_result == 0) element_enum = 20;
                    else {
                        cmp_result = strcmp(param_1, "Seat");
                        if (cmp_result == 0) element_enum = 19;
                        else {
                            cmp_result = strcmp(param_1, "City");
                            if (cmp_result == 0) element_enum = 22;
                        }
                    }
                }
            }
            break;
        case 5:
            cmp_result = strcmp(param_1, "Mayor");
            if (cmp_result == 0) element_enum = 23;
            break;
        case 6:
            cmp_result = strcmp(param_1, "Planet");
            if (cmp_result == 0) element_enum = 0;
            else {
                cmp_result = strcmp(param_1, "Period");
                if (cmp_result == 0) element_enum = 2;
                else {
                    cmp_result = strcmp(param_1, "Radius");
                    if (cmp_result == 0) element_enum = 6;
                    else {
                        cmp_result = strcmp(param_1, "Border");
                        if (cmp_result == 0) element_enum = 14;
                        else {
                            cmp_result = strcmp(param_1, "County");
                            if (cmp_result == 0) element_enum = 18;
                        }
                    }
                }
            }
            break;
        case 7:
            cmp_result = strcmp(param_1, "ERadius");
            if (cmp_result == 0) element_enum = 7;
            else {
                cmp_result = strcmp(param_1, "Gravity");
                if (cmp_result == 0) element_enum = 9;
                else {
                    cmp_result = strcmp(param_1, "Country");
                    if (cmp_result == 0) element_enum = 11;
                    else {
                        cmp_result = strcmp(param_1, "Capitol");
                        if (cmp_result == 0) element_enum = 12;
                        else {
                            cmp_result = strcmp(param_1, "Founder");
                            if (cmp_result == 0) element_enum = 17;
                            else {
                                cmp_result = strcmp(param_1, "Density");
                                if (cmp_result == 0) element_enum = 21;
                            }
                        }
                    }
                }
            }
            break;
        case 8:
            cmp_result = strcmp(param_1, "Aphelion");
            if (cmp_result == 0) element_enum = 4;
            else {
                cmp_result = strcmp(param_1, "Language");
                if (cmp_result == 0) element_enum = 13;
            }
            break;
        case 9:
            cmp_result = strcmp(param_1, "Territory");
            if (cmp_result == 0) element_enum = 15;
            break;
        case 10:
            cmp_result = strcmp(param_1, "OrbitSpeed");
            if (cmp_result == 0) element_enum = 3;
            else {
                cmp_result = strcmp(param_1, "Perihelion");
                if (cmp_result == 0) element_enum = 5;
                else {
                    cmp_result = strcmp(param_1, "Population");
                    if (cmp_result == 0) element_enum = 10;
                }
            }
            break;
        case 11: // 0xb
            cmp_result = strcmp(param_1, "Established");
            if (cmp_result == 0) element_enum = 16;
            break;
        default:
            // element_enum remains 25
            break;
    }

    if (element_enum == 25) {
        printf("!!Unknown element id: %s\n", param_1);
    }
    return element_enum;
}

// Minimal main function for compilation and testing
int main() {
    // Test buffers (char arrays)
    char test_buffer1[] = "{Planet}";
    char test_buffer2[] = "{ Name }"; // With spaces
    char test_buffer3[] = "{InvalidName}"; // No closing brace after name
    char test_buffer4[] = "{ValidName } ExtraData"; // Name followed by space then '}' then more data
    char test_buffer5[] = "{NoClosingBrace"; // Missing '}' entirely
    char test_buffer6[] = "NotAnElement"; // Doesn't start with '{'
    char test_buffer7[] = "{P1an3t}"; // Contains non-alphabetic char
    char test_buffer8[] = "{  }"; // Empty name after whitespace
    char test_buffer9[] = "{TooMany}"; // Valid name, but no matching enum
    char test_buffer10[] = "{Url}";


    // Parsing context for each buffer.
    // param_1[0] stores the buffer address, param_1[1] the length, param_1[2] the offset.
    // Note: Storing a pointer (char*) into an int (param_1[0]) can lead to truncation
    // and warnings on 64-bit systems if int is 32-bit.
    // A robust solution would use uintptr_t* for param_1, but the original snippet uses int*.
    int ctx1[] = {(int)(uintptr_t)test_buffer1, (int)(sizeof(test_buffer1) - 1), 0};
    int ctx2[] = {(int)(uintptr_t)test_buffer2, (int)(sizeof(test_buffer2) - 1), 0};
    int ctx3[] = {(int)(uintptr_t)test_buffer3, (int)(sizeof(test_buffer3) - 1), 0};
    int ctx4[] = {(int)(uintptr_t)test_buffer4, (int)(sizeof(test_buffer4) - 1), 0};
    int ctx5[] = {(int)(uintptr_t)test_buffer5, (int)(sizeof(test_buffer5) - 1), 0};
    int ctx6[] = {(int)(uintptr_t)test_buffer6, (int)(sizeof(test_buffer6) - 1), 0};
    int ctx7[] = {(int)(uintptr_t)test_buffer7, (int)(sizeof(test_buffer7) - 1), 0};
    int ctx8[] = {(int)(uintptr_t)test_buffer8, (int)(sizeof(test_buffer8) - 1), 0};
    int ctx9[] = {(int)(uintptr_t)test_buffer9, (int)(sizeof(test_buffer9) - 1), 0};
    int ctx10[] = {(int)(uintptr_t)test_buffer10, (int)(sizeof(test_buffer10) - 1), 0};

    char *name;

    printf("--- Testing pullNextElementName ---\n");

    name = pullNextElementName(ctx1);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer1, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx2);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer2, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx3);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer3, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx4);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer4, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx5);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer5, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx6);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer6, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx7);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer7, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);
    
    name = pullNextElementName(ctx8);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer8, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx9);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer9, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    name = pullNextElementName(ctx10);
    printf("Buffer: \"%s\", Result: \"%s\", Enum: %d\n", test_buffer10, name ? name : "NULL", name ? elementNameToEnum(name) : -1);
    deallocate(name, 0);

    printf("\n--- Testing elementNameToEnum (standalone) ---\n");
    printf("Name: \"Planet\", Enum: %d\n", elementNameToEnum("Planet"));
    printf("Name: \"Name\", Enum: %d\n", elementNameToEnum("Name"));
    printf("Name: \"Url\", Enum: %d\n", elementNameToEnum("Url"));
    printf("Name: \"Unknown\", Enum: %d\n", elementNameToEnum("Unknown"));
    printf("Name: NULL, Enum: %d\n", elementNameToEnum(NULL));

    return 0;
}