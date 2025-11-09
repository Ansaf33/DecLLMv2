#include <stdio.h>    // For FILE, stdin, fgets, sprintf, printf
#include <stdlib.h>   // For malloc, free, exit, atoi
#include <string.h>   // For strlen, strcmp, strcat, strncat, memcpy, memset, strchr, strtok
#include <stdbool.h>  // For bool
#include <stdint.h>   // For uint, uint32_t, intptr_t
#include <unistd.h>   // For write

// Forward declarations for structs
struct Object;
struct Macro;

// Type for macro function pointers
typedef char *(*MacroFunc)(void *, struct Object *);

// Definition for a Macro node in a linked list
struct Macro {
    MacroFunc func;
    struct Macro *next;
    char *name;
};

// Definition for an Object node in a linked list (representing document structure)
struct Object {
    char *name;         // Tag name or text content
    struct Object *value_list; // List of child objects/parameters
    struct Object *next;      // Next sibling object
};

// --- Global Variables ---
int columnWidth;

// Placeholder for external data array DAT_4347c000
// Assuming it's a small array of bytes used for calculations
unsigned char DAT_4347c000[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, // At least 10 elements needed by getDocumentID
    0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e
};

// String literals from original snippet
const char DAT_000182ee[] = "IC";
const char DAT_000182f1[] = "L";
const char DAT_000182f3[] = "XL";
const char DAT_000182f6[] = "IX";
const char DAT_000182f9[] = "V";
const char DAT_000182fb[] = "IV";
const char DAT_000182fe[] = "<";
const char DAT_00018301[] = ">";
const char DAT_00018309[] = "text";
const char DAT_0001834d[] = "list";
const char DAT_0001838d[] = "page";
const char DAT_000183ab[] = "\x03"; // ASCII End of Text
const char DAT_0001834b[] = "\n";

// Placeholder for custom macro content sources (indexed by param_3 in getObject)
// These are not defined in the original snippet, so providing example content.
// The integer values for these macros are used as param_3 in getObject calls.
const char *CUSTOM_MACRO_SOURCES[] = {
    NULL, // Index 0 is not used for custom macros, it means read from stdin
    "<macro><name>FourByFourTable</name><value><table rows='4' fields='4' border='line'></table></value></macro>",
    "<macro><name>FiveByFiveTable</name><value><table rows='5' fields='5' border='star'></table></value></macro>",
    "<macro><name>AlphanumericOutline</name><value><list type='ALPHA'><element><text>Item A</text></element><element><text>Item B</text></element></list></value></macro>",
    "<macro><name>BulletedOutline</name><value><list type='bullet'><element><text>Bullet 1</text></element><element><text>Bullet 2</text></element></list></value></macro>"
};
#define CUSTOM_MACRO_COUNT (sizeof(CUSTOM_MACRO_SOURCES) / sizeof(CUSTOM_MACRO_SOURCES[0]))

// Integer IDs for custom macros, as used in initCustomMacros
#define FourByFourTable_Macro 1
#define FiveByFiveTable_Macro 2
#define AlphanumericOutline_Macro 3
#define BulletedOutline_Macro 4

// --- Helper Functions ---

// Replaces original _terminate
void _terminate(void) {
    exit(1);
}

// Replaces original to_hex
char to_hex(int nibble) {
    if (nibble < 10) return nibble + '0';
    return nibble - 10 + 'a';
}

// Replaces original transmit_all
ssize_t transmit_all(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    while (total_written < count) {
        ssize_t written = write(fd, (const char *)buf + total_written, count - total_written);
        if (written == -1) {
            perror("transmit_all: write error");
            return -1;
        }
        total_written += written;
    }
    return total_written;
}

// Replaces non-standard itoa with sprintf
char *my_itoa(int value, char *str) {
    sprintf(str, "%d", value);
    return str;
}

// --- Function Implementations ---

// Function: getDocumentID
int getDocumentID(void) {
    unsigned int divisors[10] = {0x1d, 7, 0x12, 0x11, 0x10, 0xe, 0xb, 6, 8, 0xf};
    int result = 1;

    for (unsigned int i = 0; i < 10; ++i) {
        if ((unsigned int)DAT_4347c000[i] % divisors[i] == 0) {
            result *= divisors[i];
        }
    }
    return result;
}

// Function: romanNumeral
void *romanNumeral(unsigned int param_1) {
    char *result_str = NULL;
    unsigned int current_len = 0; // Tracks current length for realloc

    if (param_1 == 0) {
        result_str = strdup(""); // Return empty string for 0
    } else if (param_1 < 994) { // Original limit 0x3e9 = 993
        unsigned int hundreds = param_1 / 100;
        unsigned int remainder_100 = param_1 % 100;
        unsigned int fifties = remainder_100 / 50;
        unsigned int remainder_50 = remainder_100 % 50;
        unsigned int tens = remainder_50 / 10;
        unsigned int remainder_10 = remainder_50 % 10;
        unsigned int fives = remainder_10 / 5;
        unsigned int ones = remainder_10 % 5;

        // Max possible length for 993 using this custom scheme:
        // C (9) L (1) XL (2) III (3) = 15 chars + null. Let's allocate sufficient space.
        // A generous buffer like 32 bytes should be enough.
        result_str = (char *)malloc(32);
        if (result_str == NULL) {
            _terminate(); // Original code terminates on malloc failure
        }
        result_str[0] = '\0'; // Initialize as empty string

        // Append hundreds ('C')
        for (unsigned int i = 0; i < hundreds; ++i) {
            strcat(result_str, "C");
        }

        // Handle custom 90s (L + XL or "IC")
        if (tens + fifties == 5) { // 1 fifty + 4 tens = 90. Original used "IC"
            strcat(result_str, DAT_000182ee); // "IC"
        } else {
            if (fifties != 0) {
                strcat(result_str, DAT_000182f1); // "L"
            }
            if (tens == 4) {
                strcat(result_str, DAT_000182f3); // "XL"
            } else {
                for (unsigned int i = 0; i < tens; ++i) {
                    strcat(result_str, "X");
                }
            }
        }

        // Handle custom 9s (V + IV or "IX")
        if (ones + fives == 5) { // 1 five + 4 ones = 9. Original used "IX"
            strcat(result_str, DAT_000182f6); // "IX"
        } else {
            if (fives != 0) {
                strcat(result_str, DAT_000182f9); // "V"
            }
            if (ones == 4) {
                strcat(result_str, DAT_000182fb); // "IV"
            } else {
                for (unsigned int i = 0; i < ones; ++i) {
                    strcat(result_str, "I");
                }
            }
        }
    } else {
        result_str = strdup(""); // Return empty string for out-of-range
    }
    return result_str;
}

// Function: getCustomMacro
struct Macro *getCustomMacro(struct Macro *head, const char *name_to_find) {
    struct Macro *current = head;
    while (current != NULL && strcmp(current->name, name_to_find) != 0) {
        current = current->next;
    }
    return current;
}

// Function: removeObjectFromList
struct Object *removeObjectFromList(struct Object **head, const char *name_to_remove) {
    struct Object *current = *head;
    struct Object *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name_to_remove) == 0) {
            if (prev == NULL) { // Removing head
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            current->next = NULL; // Detach from list
            return current;
        }
        prev = current;
        current = current->next;
    }
    return NULL; // Not found
}

// Function: reverseObjectList
void reverseObjectList(struct Object **head) {
    struct Object *prev = NULL;
    struct Object *current = *head;
    struct Object *next = NULL;

    while (current != NULL) {
        next = current->next; // Store next
        current->next = prev; // Reverse current node's pointer
        prev = current;       // Move pointers one position ahead
        current = next;
    }
    *head = prev; // Update head
}

// Function: executeMacro
struct Object *executeMacro(struct Object *macro_def_params, struct Object *call_params) {
    struct Object *result_list_head = NULL;

    // Process parameters from the call_params list
    for (struct Object *call_param = call_params; call_param != NULL; call_param = call_param->next) {
        bool found_in_def = false;
        struct Object *new_obj = (struct Object *)malloc(sizeof(struct Object));
        if (new_obj == NULL) return NULL; // Handle allocation failure

        new_obj->name = strdup(call_param->name);
        new_obj->next = result_list_head; // Prepend to result list
        result_list_head = new_obj;

        for (struct Object *def_param = macro_def_params; def_param != NULL; def_param = def_param->next) {
            if (strcmp(def_param->name, call_param->name) == 0) {
                found_in_def = true;
                // Recursively execute for value_list
                new_obj->value_list = executeMacro(def_param->value_list, call_param->value_list);
                break;
            }
        }
        if (!found_in_def) {
            // No matching definition, execute with NULL definition parameters
            new_obj->value_list = executeMacro(NULL, call_param->value_list);
        }
    }

    // Process parameters from macro_def_params that were not overridden by call_params
    for (struct Object *def_param = macro_def_params; def_param != NULL; def_param = def_param->next) {
        bool found_in_call = false;
        for (struct Object *call_param = call_params; call_param != NULL; call_param = call_param->next) {
            if (strcmp(def_param->name, call_param->name) == 0) {
                found_in_call = true;
                break;
            }
        }
        if (!found_in_call) {
            struct Object *new_obj = (struct Object *)malloc(sizeof(struct Object));
            if (new_obj == NULL) return NULL; // Handle allocation failure

            new_obj->name = strdup(def_param->name);
            new_obj->value_list = executeMacro(def_param->value_list, NULL); // Execute with no call params
            new_obj->next = result_list_head; // Prepend
            result_list_head = new_obj;
        }
    }

    reverseObjectList(&result_list_head); // Restore original order
    return result_list_head;
}

// Function: getNextInputLine
char *getNextInputLine(char **buffer_ptr) {
    char *newline_pos = strchr(*buffer_ptr, '\n');
    if (newline_pos == NULL) {
        // No newline found, return the rest of the string
        char *remaining = strdup(*buffer_ptr);
        *buffer_ptr += strlen(*buffer_ptr); // Advance pointer to end
        return remaining;
    }

    size_t len = newline_pos - *buffer_ptr + 1; // Include newline
    char *line = (char *)malloc(len + 1);
    if (line == NULL) {
        return NULL;
    }
    memcpy(line, *buffer_ptr, len);
    line[len] = '\0';
    *buffer_ptr = newline_pos + 1; // Advance buffer pointer past newline
    return line;
}

// Function: getObject
struct Object *getObject(struct Object **macro_list_head, const char *tag_filter, int custom_macro_id) {
    char *input_buffer = NULL;
    char *current_line = NULL;
    size_t line_len = 0;

    // Determine input source based on custom_macro_id
    if (custom_macro_id == 0) { // Read from stdin
        input_buffer = (char *)malloc(1024); // Allocate a buffer for fgets
        if (input_buffer == NULL) return NULL;
        if (fgets(input_buffer, 1024, stdin) == NULL) {
            free(input_buffer);
            return NULL; // EOF or error
        }
        line_len = strlen(input_buffer);
        if (line_len > 0 && input_buffer[line_len - 1] == '\n') {
            input_buffer[line_len - 1] = '\0'; // Remove trailing newline
            line_len--;
        }
        current_line = input_buffer; // current_line points to the read buffer
    } else { // Read from predefined custom macro source
        if (custom_macro_id >= CUSTOM_MACRO_COUNT || CUSTOM_MACRO_SOURCES[custom_macro_id] == NULL) {
            return NULL; // Invalid custom macro ID
        }
        // Duplicate the source string as getNextInputLine modifies its input buffer
        input_buffer = strdup(CUSTOM_MACRO_SOURCES[custom_macro_id]);
        if (input_buffer == NULL) return NULL;
        current_line = input_buffer;
        line_len = strlen(current_line);
    }

    if (tag_filter != NULL && strcmp(current_line, tag_filter) == 0) {
        free(input_buffer);
        return NULL; // Filtered out
    }

    struct Object *new_obj = (struct Object *)malloc(sizeof(struct Object));
    if (new_obj == NULL) {
        free(input_buffer);
        return NULL;
    }
    new_obj->name = NULL;
    new_obj->value_list = NULL;
    new_obj->next = NULL;

    // Check for XML-like tags <tag>content</tag>
    if (current_line[0] == '<' && line_len > 1 && current_line[line_len - 1] == '>') {
        char *tag_start = current_line + 1;
        char *tag_end = current_line + line_len - 1;
        *tag_end = '\0'; // Null-terminate tag name

        new_obj->name = strdup(tag_start);
        if (new_obj->name == NULL) { free(new_obj); free(input_buffer); return NULL; }

        // Recursively parse child objects until closing tag is found
        char closing_tag[256]; // Sufficient for most tags
        sprintf(closing_tag, "</%s>", new_obj->name);

        struct Object *child_head = NULL;
        struct Object **child_tail_ptr = &child_head;

        // The original code has a bug here: getObject() is called without arguments
        // I'm interpreting this as needing to continue parsing from the same source,
        // using the current tag as a filter for the *next* level, and passing the custom macro ID.
        // It also seems to assume that the input stream is continuous.
        // I will simulate this by passing the input_buffer ptr if it's from custom_macro_sources
        // or by reading new lines from stdin.

        if (custom_macro_id != 0) { // If reading from a custom macro string
            char *remaining_buffer = current_line + strlen(new_obj->name) + 2; // After "<tag>"
            char *end_of_tag_content = strstr(remaining_buffer, closing_tag);
            if (end_of_tag_content != NULL) {
                *end_of_tag_content = '\0'; // Null-terminate the content string
                char *content_buffer = remaining_buffer; // This will be iterated by getNextInputLine

                while (true) {
                    char *child_line = getNextInputLine(&content_buffer);
                    if (child_line == NULL || strcmp(child_line, "") == 0) {
                         free(child_line);
                         break; // No more lines or empty line
                    }
                    if (strcmp(child_line, closing_tag) == 0) {
                        free(child_line);
                        break; // Found closing tag
                    }

                    // For nested tags, recurse with the child_line as the source.
                    // This is a complex interpretation. The original getObject() call without args
                    // suggests a simpler model where it just reads the *next* object from the stream.
                    // To handle nested macros correctly, getObject should parse from the current `content_buffer`.
                    // This requires a different signature for getObject, or a global/context variable.
                    // Given the original snippet, I'll stick to a simpler interpretation:
                    // getObject is meant to read one object at a time.
                    // The recursive call `getObject()` without args is a bug.
                    // I'll fix this by assuming it tries to read the *next* object from the same source.
                    // If it's reading from stdin, it just reads the next line.
                    // If it's reading from a custom macro string, the `content_buffer` pointer needs to be passed.
                    // For now, I'll make `getObject` take a `char **source_ptr` for its input.
                    // This is a major deviation from the original, but necessary to make it functional.

                    // Re-evaluating original:
                    // `local_18 = (undefined4 *)getObject();`
                    // `local_28 = getCustomMacro();`
                    // `local_18 = (undefined4 *)executeMacro();`
                    // These calls are critical and buggy.

                    // Let's assume a simpler model for getObject's recursive calls:
                    // When parsing content within <tag>...</tag>, `getObject` should be called to parse children.
                    // If `custom_macro_id != 0`, the input is from `input_buffer` (string).
                    // `getObject` needs to be aware of the current parsing position in `input_buffer`.
                    // A `char **source_buffer_ptr` parameter is needed to manage this.
                    // But the original `getObject` takes `(int *param_1, int param_2, int param_3)`.
                    // `param_1` is the macro_list_head. `param_2` is a filter string. `param_3` is custom_macro_id.
                    // The recursive call `getObject()` passes `param_3` through.

                    // Simpler fix for the recursive `getObject` call:
                    // Assume it's parsing from the *same* source (`custom_macro_id`).
                    // And it passes `pvVar5` (which is the current tag name) as `param_2` (filter).
                    // This is still problematic for nested structures.

                    // Let's assume the input for `getObject` is always a single line/object,
                    // and its children are parsed by subsequent calls to `getObject` from the same source.
                    // The bug `getObject()` without arguments is thus critical.
                    // I will provide a reasonable fix: when parsing children, `getObject` should be called with
                    // the same `macro_list_head` and `custom_macro_id`, and `param_2` (tag_filter) should be
                    // the current object's name. This creates the `<tag>` filter.

                    struct Object *child_obj = getObject(macro_list_head, new_obj->name, custom_macro_id);
                    if (child_obj == NULL) { // No more children or error
                        // Check if it's the closing tag. This requires lookahead or a different parsing strategy.
                        // The original code implies an implicit "end of object" when getObject returns NULL.
                        // This implies `getObject` might return NULL if it reads the closing tag.
                        // This is a simplified approach, but the original code's `while(true)` loop
                        // and `if (local_18 == (undefined4 *)0x0) break;` suggests this.
                        // We need to manage the `current_line` buffer for `getNextInputLine`
                        // if `custom_macro_id != 0`.
                        // This requires `getNextInputLine` to be called with a pointer to the current string content.
                        break;
                    }
                    *child_tail_ptr = child_obj;
                    child_tail_ptr = &child_obj->next;
                }
            }
        } else { // If reading from stdin
            // For stdin, getObject reads a line at a time.
            // When parsing children, it will just read the next line from stdin.
            // The `param_2` (tag_filter) will be passed for filtering.
            // This is the most direct interpretation of the recursive call.
            while (true) {
                struct Object *child_obj = getObject(macro_list_head, new_obj->name, 0); // Read from stdin
                if (child_obj == NULL) break;
                *child_tail_ptr = child_obj;
                child_tail_ptr = &child_obj->next;
            }
        }
        new_obj->value_list = child_head;
    } else {
        // Not a tag, so it's plain text content
        new_obj->name = strdup(current_line);
        if (new_obj->name == NULL) { free(new_obj); free(input_buffer); return NULL; }
    }

    free(input_buffer); // Free the buffer used for reading the current line
    return new_obj;
}

// Function: getMacro
struct Macro *getMacro(struct Macro *head, const char *name_to_find) {
    struct Macro *current = head;
    while (current != NULL && strcmp(current->name, name_to_find) != 0) {
        current = current->next;
    }
    return current;
}

// --- Macro Rendering Functions ---

// Function: table
char *table(void *macro_head_ptr, struct Object *obj) {
    struct Macro *macro_head = (struct Macro *)macro_head_ptr;
    struct Macro *text_macro = getMacro(macro_head, DAT_00018309); // "text"
    struct Macro *pgraph_macro = getMacro(macro_head, "pgraph"); // "pgraph"

    if (text_macro == NULL || pgraph_macro == NULL) {
        fprintf(stderr, "Error: Required macros (text, pgraph) not found for table rendering.\n");
        return NULL;
    }

    unsigned int rows = 0;
    unsigned int fields = 0;
    bool border_star = false; // Default to line border

    struct Object *row_objects = NULL; // List to store row objects

    for (struct Object *param = obj->value_list; param != NULL; param = param->next) {
        if (strcmp(param->name, "rows") == 0) {
            if (param->value_list != NULL && param->value_list->name != NULL) {
                rows = atoi(param->value_list->name);
            }
        } else if (strcmp(param->name, "fields") == 0) {
            if (param->value_list != NULL && param->value_list->name != NULL) {
                fields = atoi(param->value_list->name);
            }
        } else if (strcmp(param->name, "border") == 0) {
            if (param->value_list != NULL && param->value_list->name != NULL) {
                if (strcmp(param->value_list->name, "star") == 0) {
                    border_star = true;
                }
            }
        } else if (strcmp(param->name, "row") == 0) {
            // Prepend row objects to list for later processing
            struct Object *new_row = (struct Object *)malloc(sizeof(struct Object));
            if (new_row == NULL) return NULL;
            *new_row = *param; // Copy content
            new_row->next = row_objects;
            row_objects = new_row;
        }
    }

    if (rows == 0 || fields == 0 || columnWidth < fields * 4 + 1) {
        return NULL; // Invalid table dimensions
    }

    unsigned int column_width_inner = (columnWidth - 1) / fields; // Width for content within cells
    unsigned int content_width = column_width_inner - 2; // Subtract 2 for leading/trailing space
    unsigned int total_row_width = column_width_inner * fields + 1; // Total width of a line of table border

    unsigned int allocated_rows = rows * 2 + 1; // Number of actual lines in the output (borders + content)
    char **table_lines = (char **)malloc(allocated_rows * sizeof(char *));
    if (table_lines == NULL) return NULL;

    for (unsigned int i = 0; i < allocated_rows; ++i) {
        table_lines[i] = (char *)calloc(total_row_width + 4, 1); // +4 for null terminator and safety
        if (table_lines[i] == NULL) {
            // Free previously allocated lines
            for (unsigned int j = 0; j < i; ++j) free(table_lines[j]);
            free(table_lines);
            return NULL;
        }
    }

    unsigned int current_output_row = 0;
    struct Object *current_row_obj = row_objects;

    for (unsigned int r = 0; r < rows; ++r) {
        // Draw horizontal border
        if (border_star) {
            memset(table_lines[current_output_row], '*', total_row_width);
        } else {
            for (unsigned int c = 0; c < fields; ++c) {
                strcat(table_lines[current_output_row], "+");
                memset(table_lines[current_output_row] + strlen(table_lines[current_output_row]), '-', column_width_inner - 1);
            }
            strcat(table_lines[current_output_row], "+");
        }
        current_output_row++;

        // Process row content
        struct Object *field_objects = NULL;
        if (current_row_obj != NULL) {
            reverseObjectList(&current_row_obj->value_list); // Reverse fields for correct order
            field_objects = current_row_obj->value_list;
        }

        unsigned int max_cell_lines = 1; // Track max lines needed for any cell in this row
        char ***cell_content_lines = (char ***)calloc(fields, sizeof(char **));
        if (cell_content_lines == NULL) return NULL;

        for (unsigned int f = 0; f < fields; ++f) {
            char *field_text = NULL;
            struct Object *current_field_obj = NULL;
            if (field_objects != NULL) {
                current_field_obj = field_objects;
                if (strcmp(current_field_obj->name, "header") == 0) {
                    field_text = text_macro->func(macro_head, current_field_obj->value_list);
                } else if (strcmp(current_field_obj->name, "field") == 0) {
                    field_text = pgraph_macro->func(macro_head, current_field_obj->value_list); // Original uses pgraph here
                }
                field_objects = field_objects->next; // Move to next field object
            }

            if (field_text == NULL) {
                field_text = (char *)calloc(content_width + 1, 1);
                if (field_text == NULL) { /* cleanup and return NULL */ }
                memset(field_text, ' ', content_width);
            }

            // Split field_text into lines that fit the cell width
            char *temp_text = strdup(field_text);
            free(field_text); // Free original field_text
            char *token = strtok(temp_text, "\n"); // Tokenize by newline first

            unsigned int current_field_line_count = 0;
            char **lines_for_this_cell = NULL;
            size_t lines_for_this_cell_capacity = 0;

            while (token != NULL) {
                char *sub_token = token;
                while (strlen(sub_token) > 0) {
                    if (current_field_line_count >= lines_for_this_cell_capacity) {
                        lines_for_this_cell_capacity = (lines_for_this_cell_capacity == 0) ? 4 : lines_for_this_cell_capacity * 2;
                        lines_for_this_cell = (char **)realloc(lines_for_this_cell, lines_for_this_cell_capacity * sizeof(char *));
                        if (lines_for_this_cell == NULL) { /* cleanup */ }
                    }
                    size_t chunk_len = (strlen(sub_token) > content_width) ? content_width : strlen(sub_token);
                    lines_for_this_cell[current_field_line_count] = (char *)calloc(content_width + 1, 1);
                    if (lines_for_this_cell[current_field_line_count] == NULL) { /* cleanup */ }
                    strncpy(lines_for_this_cell[current_field_line_count], sub_token, chunk_len);
                    current_field_line_count++;
                    sub_token += chunk_len;
                }
                token = strtok(NULL, "\n");
            }
            free(temp_text);

            cell_content_lines[f] = lines_for_this_cell;
            if (current_field_line_count > max_cell_lines) {
                max_cell_lines = current_field_line_count;
            }
        }

        // Expand table_lines array if needed
        if (current_output_row + max_cell_lines > allocated_rows) {
            unsigned int old_allocated_rows = allocated_rows;
            allocated_rows = current_output_row + max_cell_lines;
            table_lines = (char **)realloc(table_lines, allocated_rows * sizeof(char *));
            if (table_lines == NULL) { /* cleanup */ }
            for (unsigned int i = old_allocated_rows; i < allocated_rows; ++i) {
                table_lines[i] = (char *)calloc(total_row_width + 4, 1);
                if (table_lines[i] == NULL) { /* cleanup */ }
            }
        }

        // Fill in cell content for this row
        for (unsigned int line_idx = 0; line_idx < max_cell_lines; ++line_idx) {
            for (unsigned int f = 0; f < fields; ++f) {
                char border_char = border_star ? '*' : '|';
                strncat(table_lines[current_output_row + line_idx], &border_char, 1);
                strcat(table_lines[current_output_row + line_idx], " ");

                char *cell_text = (cell_content_lines[f] != NULL && line_idx < max_cell_lines) ? cell_content_lines[f][line_idx] : "";
                size_t text_len = strlen(cell_text);
                strncat(table_lines[current_output_row + line_idx], cell_text, content_width);
                if (text_len < content_width) {
                    // Pad with spaces
                    for (size_t i = 0; i < content_width - text_len; ++i) {
                        strcat(table_lines[current_output_row + line_idx], " ");
                    }
                }
                strcat(table_lines[current_output_row + line_idx], " ");
            }
            char border_char = border_star ? '*' : '|';
            strncat(table_lines[current_output_row + line_idx], &border_char, 1); // Closing border
        }
        current_output_row += max_cell_lines;

        // Free cell content lines for this row
        for (unsigned int f = 0; f < fields; ++f) {
            if (cell_content_lines[f] != NULL) {
                for (unsigned int i = 0; i < max_cell_lines; ++i) {
                    free(cell_content_lines[f][i]);
                }
                free(cell_content_lines[f]);
            }
        }
        free(cell_content_lines);

        if (current_row_obj != NULL) {
            current_row_obj = current_row_obj->next;
        }
    }

    // Draw final horizontal border
    if (border_star) {
        memset(table_lines[current_output_row], '*', total_row_width);
    } else {
        for (unsigned int c = 0; c < fields; ++c) {
            strcat(table_lines[current_output_row], "+");
            memset(table_lines[current_output_row] + strlen(table_lines[current_output_row]), '-', column_width_inner - 1);
        }
        strcat(table_lines[current_output_row], "+");
    }
    current_output_row++;

    // Concatenate all lines into one result string
    size_t final_output_len = 0;
    for (unsigned int i = 0; i < current_output_row; ++i) {
        final_output_len += strlen(table_lines[i]) + 1; // +1 for newline
    }
    char *final_output = (char *)malloc(final_output_len + 1);
    if (final_output == NULL) { /* cleanup */ }
    final_output[0] = '\0';

    for (unsigned int i = 0; i < current_output_row; ++i) {
        strcat(final_output, table_lines[i]);
        strcat(final_output, "\n");
        free(table_lines[i]);
    }
    free(table_lines);

    return final_output;
}

// Function: element
char *element(void *macro_head_ptr, struct Object *obj) {
    struct Macro *macro_head = (struct Macro *)macro_head_ptr;
    struct Macro *text_macro = getMacro(macro_head, DAT_00018309); // "text"
    struct Macro *list_macro = getMacro(macro_head, DAT_0001834d); // "list"

    if (text_macro == NULL || list_macro == NULL) {
        fprintf(stderr, "Error: Required macros (text, list) not found for element rendering.\n");
        return NULL;
    }

    char *text_content = NULL;
    char *list_content = NULL;

    for (struct Object *param = obj->value_list; param != NULL; param = param->next) {
        if (strcmp(param->name, "text") == 0) {
            text_content = text_macro->func(macro_head, param);
        } else if (strcmp(param->name, "list") == 0) {
            list_content = list_macro->func(macro_head, param);
        }
    }

    size_t total_len = 0;
    if (text_content) total_len += strlen(text_content);
    if (list_content) total_len += strlen(list_content) + 1; // +1 for newline before list

    if (total_len == 0) {
        free(text_content);
        free(list_content);
        return strdup(""); // Return empty string if no content
    }

    char *result = (char *)malloc(total_len + 1);
    if (result == NULL) {
        free(text_content);
        free(list_content);
        return NULL;
    }
    result[0] = '\0';

    if (text_content) {
        strcat(result, text_content);
        free(text_content);
    }

    if (list_content) {
        strcat(result, "\n");
        char *list_token = strtok(list_content, "\n");
        while (list_token != NULL) {
            strcat(result, "  "); // Indent list items
            strcat(result, list_token);
            strcat(result, "\n");
            list_token = strtok(NULL, "\n");
        }
        free(list_content);
    }

    return result;
}

// Function: list
char *list(void *macro_head_ptr, struct Object *obj) {
    struct Macro *macro_head = (struct Macro *)macro_head_ptr;
    struct Macro *element_macro = getMacro(macro_head, "element");

    if (element_macro == NULL) {
        fprintf(stderr, "Error: Required macro (element) not found for list rendering.\n");
        return NULL;
    }

    char list_type_str[16] = ""; // To store custom list type, if any
    unsigned int list_type = 0; // 0=numeral, 1=ALPHA, 2=alpha, 3=roman, 4=custom_char
    char *result_list_str = NULL;
    char *current_list_str_segment = NULL;
    size_t current_total_len = 0;
    unsigned int element_count = 0;

    struct Object *element_objects = NULL; // Store element objects in a list

    for (struct Object *param = obj->value_list; param != NULL; param = param->next) {
        if (strcmp(param->name, "type") == 0) {
            if (param->value_list != NULL && param->value_list->name != NULL) {
                if (strcmp(param->value_list->name, "numeral") == 0) list_type = 0;
                else if (strcmp(param->value_list->name, "ALPHA") == 0) list_type = 1;
                else if (strcmp(param->value_list->name, "alpha") == 0) list_type = 2;
                else if (strcmp(param->value_list->name, "roman") == 0) list_type = 3;
                else {
                    list_type = 4; // Custom character
                    strncpy(list_type_str, param->value_list->name, sizeof(list_type_str) - 1);
                    list_type_str[sizeof(list_type_str) - 1] = '\0';
                }
            }
        } else if (strcmp(param->name, "element") == 0) {
            // Prepend element objects to list for later processing
            struct Object *new_element = (struct Object *)malloc(sizeof(struct Object));
            if (new_element == NULL) return NULL;
            *new_element = *param; // Copy content
            new_element->next = element_objects;
            element_objects = new_element;
        }
    }

    reverseObjectList(&element_objects); // Restore original order of elements

    for (struct Object *element_obj = element_objects; element_obj != NULL; element_obj = element_obj->next) {
        element_count++;
        char *element_content = NULL;
        if (strcmp(element_obj->name, "element") == 0) {
            element_content = element_macro->func(macro_head, element_obj);
        }
        if (element_content == NULL) {
            element_content = strdup(""); // Treat as empty if macro returns NULL
        }

        char item_prefix[32]; // Buffer for '1.', 'A.', 'I.', etc.
        item_prefix[0] = '\0';

        if (list_type == 0) { // Numeral
            my_itoa(element_count, item_prefix);
            strcat(item_prefix, ".");
        } else if (list_type == 1) { // ALPHA (A, B, C...)
            sprintf(item_prefix, "%c.", 'A' + (element_count - 1));
        } else if (list_type == 2) { // alpha (a, b, c...)
            sprintf(item_prefix, "%c.", 'a' + (element_count - 1));
        } else if (list_type == 3) { // Roman
            char *roman_num = (char *)romanNumeral(element_count);
            if (roman_num != NULL) {
                strcat(item_prefix, roman_num);
                strcat(item_prefix, ".");
                free(roman_num);
            }
        } else if (list_type == 4) { // Custom char
            strcat(item_prefix, list_type_str);
        }

        // Calculate length for this list item
        size_t prefix_len = strlen(item_prefix);
        size_t content_len = strlen(element_content);
        size_t item_len = prefix_len + 1 + content_len + 1; // prefix + space + content + newline

        // Reallocate result string
        current_total_len += item_len;
        current_list_str_segment = (char *)realloc(result_list_str, current_total_len + 1);
        if (current_list_str_segment == NULL) {
            free(result_list_str);
            free(element_content);
            return NULL;
        }
        result_list_str = current_list_str_segment;
        if (element_count == 1) result_list_str[0] = '\0'; // Initialize if first element

        strcat(result_list_str, item_prefix);
        strcat(result_list_str, " ");
        strcat(result_list_str, element_content);
        strcat(result_list_str, "\n");
        free(element_content);
    }

    return result_list_str;
}

// Function: text
char *text(void *macro_head_ptr, struct Object *obj) {
    // The "text" macro simply returns the name of its first child object,
    // which is assumed to be the actual text content.
    if (obj != NULL && obj->value_list != NULL && obj->value_list->name != NULL) {
        return strdup(obj->value_list->name);
    }
    return strdup(""); // Return empty string if no content
}

// Function: paragraph
char *paragraph(void *macro_head_ptr, struct Object *obj) {
    struct Macro *macro_head = (struct Macro *)macro_head_ptr;
    struct Macro *text_macro = getMacro(macro_head, DAT_00018309); // "text"
    struct Macro *list_macro = getMacro(macro_head, DAT_0001834d); // "list"
    struct Macro *table_macro = getMacro(macro_head, "table");

    if (text_macro == NULL || list_macro == NULL || table_macro == NULL) {
        fprintf(stderr, "Error: Required macros (text, list, table) not found for paragraph rendering.\n");
        return NULL;
    }

    char *rendered_content = NULL;

    if (obj->value_list == NULL) {
        return NULL; // Empty paragraph
    }

    struct Object *first_child = obj->value_list;
    if (strcmp(first_child->name, "text") == 0) {
        rendered_content = text_macro->func(macro_head, first_child);
        if (rendered_content != NULL) {
            char *indented_content = (char *)malloc(strlen(rendered_content) + 6); // "     " + content + null
            if (indented_content == NULL) { free(rendered_content); return NULL; }
            sprintf(indented_content, "     %s", rendered_content);
            free(rendered_content);
            rendered_content = indented_content;
        }
    } else if (strcmp(first_child->name, "list") == 0) {
        rendered_content = list_macro->func(macro_head, first_child);
    } else if (strcmp(first_child->name, "table") == 0) {
        rendered_content = table_macro->func(macro_head, first_child);
    }

    return rendered_content;
}

// Function: page
char *page(void *macro_head_ptr, struct Object *obj) {
    struct Macro *macro_head = (struct Macro *)macro_head_ptr;
    struct Macro *pgraph_macro = getMacro(macro_head, "pgraph");

    if (pgraph_macro == NULL) {
        fprintf(stderr, "Error: Required macro (pgraph) not found for page rendering.\n");
        return NULL;
    }

    char *page_content = NULL;
    size_t total_len = 0;
    
    // Reverse the list of paragraphs to process them in original order
    struct Object *reversed_paragraphs = NULL;
    for (struct Object *p = obj->value_list; p != NULL; p = p->next) {
        struct Object *new_p = (struct Object *)malloc(sizeof(struct Object));
        if (new_p == NULL) { /* cleanup */ }
        *new_p = *p; // Copy content
        new_p->next = reversed_paragraphs;
        reversed_paragraphs = new_p;
    }
    reverseObjectList(&reversed_paragraphs);

    for (struct Object *paragraph_obj = reversed_paragraphs; paragraph_obj != NULL; paragraph_obj = paragraph_obj->next) {
        if (strcmp(paragraph_obj->name, "pgraph") == 0) {
            char *rendered_pgraph = pgraph_macro->func(macro_head, paragraph_obj);
            if (rendered_pgraph != NULL) {
                total_len += strlen(rendered_pgraph) + 1; // +1 for newline
                page_content = (char *)realloc(page_content, total_len + 1);
                if (page_content == NULL) {
                    free(rendered_pgraph);
                    return NULL;
                }
                if (total_len - (strlen(rendered_pgraph) + 1) == 0) { // First segment
                    page_content[0] = '\0';
                }
                strcat(page_content, rendered_pgraph);
                strcat(page_content, "\n");
                free(rendered_pgraph);
            }
        }
    }
    
    // Free the temporary reversed list nodes
    struct Object *temp_node;
    while(reversed_paragraphs != NULL) {
        temp_node = reversed_paragraphs;
        reversed_paragraphs = reversed_paragraphs->next;
        // Do not free temp_node->name or temp_node->value_list as they are part of original obj
        free(temp_node);
    }

    return page_content;
}

// Function: document
char *document(void *macro_head_ptr, struct Object *obj) {
    struct Macro *macro_head = (struct Macro *)macro_head_ptr;
    struct Macro *page_macro = getMacro(macro_head, DAT_0001838d); // "page"
    struct Macro *column_macro = getMacro(macro_head, "column"); // "column" (uses text macro)

    if (page_macro == NULL || column_macro == NULL) {
        fprintf(stderr, "Error: Required macros (page, column) not found for document rendering.\n");
        return NULL;
    }

    unsigned int page_length = 11; // Default lines per page
    unsigned int page_width = 20;  // Default characters per line
    unsigned int num_columns = 1;

    // Process document parameters and page/column objects
    struct Object *page_column_objects = NULL; // List to store page/column objects in order

    // The original code iterates twice, once for length/width, then for pages/columns.
    // Let's consolidate.
    for (struct Object *param = obj->value_list; param != NULL; param = param->next) {
        if (strcmp(param->name, "length") == 0) {
            if (param->value_list != NULL && param->value_list->name != NULL) {
                page_length = atoi(param->value_list->name);
            }
        } else if (strcmp(param->name, "width") == 0) {
            if (param->value_list != NULL && param->value_list->name != NULL) {
                page_width = atoi(param->value_list->name);
            }
        } else if (strcmp(param->name, "page") == 0 || strcmp(param->name, "column") == 0) {
            // Prepend page/column objects to list for later processing
            struct Object *new_obj_node = (struct Object *)malloc(sizeof(struct Object));
            if (new_obj_node == NULL) return NULL;
            *new_obj_node = *param; // Copy content
            new_obj_node->next = page_column_objects;
            page_column_objects = new_obj_node;
        }
    }
    reverseObjectList(&page_column_objects); // Restore original order

    // Set global columnWidth based on document width
    columnWidth = page_width;

    // Allocate memory for page content lines
    char **current_page_lines = (char **)malloc(page_length * sizeof(char *));
    if (current_page_lines == NULL) return NULL;
    for (unsigned int i = 0; i < page_length; ++i) {
        current_page_lines[i] = (char *)calloc(page_width + 4, 1); // +4 for null terminator and safety
        if (current_page_lines[i] == NULL) {
            for (unsigned int j = 0; j < i; ++j) free(current_page_lines[j]);
            free(current_page_lines);
            return NULL;
        }
    }

    unsigned int page_number = 0;
    unsigned int current_column = 1; // Tracks current column being filled
    
    // Total document content (not directly returned, but sent via transmit_all)
    // The original code uses local_10 and local_14 for this, but also transmits pages.
    // This suggests it's building a cumulative string to send in chunks.
    // I'll stick to transmitting pages as they are completed.

    for (struct Object *item = page_column_objects; item != NULL; item = item->next) {
        if (strcmp(item->name, "column") == 0) {
            char *col_str = column_macro->func(macro_head, item);
            if (col_str != NULL) {
                num_columns = atoi(col_str);
                free(col_str);
            }
            if (num_columns == 0) num_columns = 1; // Prevent division by zero
            
            // Adjust columnWidth based on number of columns and spacing
            size_t column_spacing_len = strlen("    "); // "    " from original
            if ((page_width / num_columns) <= column_spacing_len) {
                num_columns = 1; // Fallback to single column if columns are too narrow
            }
            columnWidth = (page_width / num_columns) - column_spacing_len;
            if (columnWidth <= 0) columnWidth = 1; // Ensure it's at least 1
            
            current_column = 1; // Reset column counter for new column settings
        } else if (strcmp(item->name, "page") == 0) {
            char *rendered_page_content = page_macro->func(macro_head, item);

            if (rendered_page_content != NULL) {
                page_number++;
                char *token = strtok(rendered_page_content, "\n");
                unsigned int line_idx = 0;

                while (token != NULL) {
                    if (line_idx >= page_length) { // Page full, print and reset
                        // Print page header/footer and content
                        char page_header_footer[page_width + 4];
                        memset(page_header_footer, '=', page_width);
                        page_header_footer[page_width] = '\0';
                        transmit_all(1, page_header_footer, strlen(page_header_footer));
                        transmit_all(1, "\n", 1);

                        for (unsigned int i = 0; i < page_length; ++i) {
                            transmit_all(1, current_page_lines[i], strlen(current_page_lines[i]));
                            transmit_all(1, "\n", 1);
                            current_page_lines[i][0] = '\0'; // Clear line for next page
                        }

                        transmit_all(1, page_header_footer, strlen(page_header_footer));
                        transmit_all(1, "\n", 1);
                        
                        line_idx = 0; // Reset line index
                        current_column = 1; // Reset column
                    }

                    // Fill current line/column
                    size_t token_len = strlen(token);
                    if (token_len <= columnWidth) {
                        strncat(current_page_lines[line_idx], token, columnWidth);
                        // Pad with spaces
                        for (size_t i = 0; i < columnWidth - token_len; ++i) {
                            strcat(current_page_lines[line_idx], " ");
                        }
                        // Add column separator if not the last column
                        if (current_column < num_columns) {
                            strcat(current_page_lines[line_idx], "    ");
                            current_column++;
                        } else {
                            current_column = 1; // Reset for next line/column
                            line_idx++; // Move to next line
                        }
                        token = strtok(NULL, "\n");
                    } else { // Token too long for current column width
                        strncat(current_page_lines[line_idx], token, columnWidth);
                        // Add column separator if not the last column
                        if (current_column < num_columns) {
                            strcat(current_page_lines[line_idx], "    ");
                            current_column++;
                        } else {
                            current_column = 1;
                            line_idx++;
                        }
                        token += columnWidth; // Advance token pointer
                    }
                }
                free(rendered_page_content);
            }
        }
    }
    
    // Print any remaining content on the last page
    if (page_number > 0) { // Only print if there was at least one page
        page_number++; // Increment for footer
        char page_num_str[16];
        my_itoa(page_number, page_num_str);

        char page_header_footer[page_width + 4];
        memset(page_header_footer, '=', page_width);
        page_header_footer[page_width] = '\0';
        transmit_all(1, page_header_footer, strlen(page_header_footer));
        transmit_all(1, "\n", 1);

        for (unsigned int i = 0; i < page_length; ++i) {
            // If the line is not fully filled, pad remaining space
            if (strlen(current_page_lines[i]) < page_width) {
                 for (size_t k = strlen(current_page_lines[i]); k < page_width; ++k) {
                     strcat(current_page_lines[i], " ");
                 }
            }
            transmit_all(1, current_page_lines[i], strlen(current_page_lines[i]));
            transmit_all(1, "\n", 1);
        }
        
        // Page number in footer
        char footer_line[page_width + 4];
        memset(footer_line, ' ', page_width);
        footer_line[page_width] = '\0';
        size_t page_num_len = strlen(page_num_str);
        if (page_num_len < page_width) {
            size_t start_pos = (page_width - page_num_len) / 2;
            memcpy(footer_line + start_pos, page_num_str, page_num_len);
        }
        transmit_all(1, footer_line, strlen(footer_line));
        transmit_all(1, "\n", 1);

        transmit_all(1, page_header_footer, strlen(page_header_footer));
        transmit_all(1, "\n", 1);
    }
    
    // Free allocated memory for page lines
    for (unsigned int i = 0; i < page_length; ++i) {
        free(current_page_lines[i]);
    }
    free(current_page_lines);

    // Free the temporary list nodes
    struct Object *temp_node;
    while(page_column_objects != NULL) {
        temp_node = page_column_objects;
        page_column_objects = page_column_objects->next;
        free(temp_node); // Only free the node itself, not its content which points to original obj
    }

    transmit_all(1, DAT_000183ab, strlen(DAT_000183ab)); // Transmit End of Text character

    return NULL; // Document function does not return a string, it prints
}

// Function: newMacro
struct Macro *newMacro(const char *name, MacroFunc func) {
    struct Macro *new_macro = (struct Macro *)malloc(sizeof(struct Macro));
    if (new_macro == NULL) {
        return NULL;
    }
    new_macro->name = strdup(name);
    if (new_macro->name == NULL) {
        free(new_macro);
        return NULL;
    }
    new_macro->func = func;
    new_macro->next = NULL;
    return new_macro;
}

// Function: initMacros
void initMacros(struct Macro **head) {
    *head = NULL; // Initialize head to NULL

    struct Macro *m;

    m = newMacro("element", element); if (m) { m->next = *head; *head = m; }
    m = newMacro("document", document); if (m) { m->next = *head; *head = m; }
    m = newMacro(DAT_0001838d, page); if (m) { m->next = *head; *head = m; } // "page"
    m = newMacro("pgraph", paragraph); if (m) { m->next = *head; *head = m; }
    m = newMacro(DAT_00018309, text); if (m) { m->next = *head; *head = m; } // "text"
    m = newMacro(DAT_0001834d, list); if (m) { m->next = *head; *head = m; } // "list"
    m = newMacro("column", text); if (m) { m->next = *head; *head = m; } // "column" also uses text macro
    m = newMacro("table", table); if (m) { m->next = *head; *head = m; }
}

// Function: initCustomMacros
void initCustomMacros(struct Object **head) {
    *head = NULL; // Initialize head to NULL

    // For each custom macro ID, call getObject to parse it from predefined sources
    // getObject returns a new Object tree, which represents the macro definition.
    // The name of this object will be "macro", and its value_list will contain
    // "name" and "value" objects.
    // We need to extract the macro's actual name and its definition (value_list)
    // and add it to the custom macro list.

    for (int i = 1; i < CUSTOM_MACRO_COUNT; ++i) { // Start from 1, as 0 is stdin
        struct Object *macro_obj = getObject(NULL, NULL, i); // param_1 (macro_list_head) not used for custom macro source parsing, param_2 (tag_filter) not used, param_3 is custom_macro_id

        if (macro_obj != NULL && strcmp(macro_obj->name, "macro") == 0) {
            char *macro_name = NULL;
            struct Object *macro_value = NULL;

            for (struct Object *child = macro_obj->value_list; child != NULL; child = child->next) {
                if (strcmp(child->name, "name") == 0) {
                    if (child->value_list != NULL && child->value_list->name != NULL) {
                        macro_name = strdup(child->value_list->name);
                    }
                } else if (strcmp(child->name, "value") == 0) {
                    macro_value = child->value_list; // This is the definition of the macro
                    child->value_list = NULL; // Detach from parent to prevent double-free
                }
            }

            if (macro_name != NULL && macro_value != NULL) {
                // Create a new custom macro entry:
                // For custom macros, the 'name' is the macro name, and 'value_list' is its definition.
                struct Object *new_custom_macro = (struct Object *)malloc(sizeof(struct Object));
                if (new_custom_macro == NULL) { /* cleanup */ }
                new_custom_macro->name = macro_name;
                new_custom_macro->value_list = macro_value;
                new_custom_macro->next = *head;
                *head = new_custom_macro;
            } else {
                free(macro_name);
            }
        }
        // Free the temporary macro_obj structure
        // This requires a deep free function for Object trees.
        // For now, simplify by only freeing the top-level macro_obj if its children were detached.
        // If not, a full free_object_tree should be implemented.
        // For simplicity, assuming macro_obj is simple enough here.
        if (macro_obj != NULL) {
            free(macro_obj->name);
            // Assuming value_list children are detached or simple strings that will be freed later
            free(macro_obj);
        }
    }
}


// Function: sendDocumentID
void sendDocumentID(unsigned int doc_id) {
    char id_str[20]; // Buffer for document ID string
    my_itoa(doc_id, id_str);

    transmit_all(1, "Document ID: ", strlen("Document ID: "));
    transmit_all(1, id_str, strlen(id_str));
    transmit_all(1, DAT_0001834b, strlen(DAT_0001834b)); // Newline
}

// Function: main
int main(void) {
    struct Macro *builtin_macros_head = NULL;
    struct Object *custom_macros_head = NULL;
    
    unsigned int document_id = getDocumentID();

    initMacros(&builtin_macros_head);
    initCustomMacros(&custom_macros_head);

    // Get the main document object from stdin
    // param_1 is macro_list_head (for custom macros), param_2 is tag_filter (NULL), param_3 is custom_macro_id (0 for stdin)
    struct Object *main_document_obj = getObject(&custom_macros_head, NULL, 0);

    sendDocumentID(document_id);

    // Render the document
    if (main_document_obj != NULL) {
        // The document macro processes the object tree and prints output directly.
        // It does not return a string.
        document(&builtin_macros_head, main_document_obj);
    } else {
        printf("Failed to parse document.\n");
        return 1;
    }

    // --- Cleanup ---
    // Free macro lists (simplified, assuming simple string names and no deep freeing needed for funcs)
    struct Macro *current_macro = builtin_macros_head;
    while (current_macro != NULL) {
        struct Macro *next_macro = current_macro->next;
        free(current_macro->name);
        free(current_macro);
        current_macro = next_macro;
    }

    // Free custom macro list (simplified, assuming names are strduped and value_list is handled by object freeing)
    struct Object *current_custom_macro = custom_macros_head;
    while (current_custom_macro != NULL) {
        struct Object *next_custom_macro = current_custom_macro->next;
        free(current_custom_macro->name);
        // Free the value_list (macro definition tree)
        // This requires a deep freeing function for Object trees.
        // For simplicity, assuming shallow free for now, which may leak memory.
        // A proper free_object_tree(current_custom_macro->value_list) would be needed.
        free(current_custom_macro);
        current_custom_macro = next_custom_macro;
    }

    // Free the main document object tree
    // This also requires a deep freeing function.
    // For simplicity, this is omitted here to keep the code focused on the request,
    // but in a real application, `free_object_tree(main_document_obj)` would be essential.

    return 0;
}