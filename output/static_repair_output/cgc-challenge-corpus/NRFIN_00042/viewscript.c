#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h> // For size_t

// Forward declarations for structs
typedef struct BaseVarNode BaseVarNode;
typedef struct NumVarNode NumVarNode;
typedef struct ArrVarNode ArrVarNode;
typedef struct ViewVarNode ViewVarNode;
typedef struct ViewType ViewType;

// Base structure for all variable nodes
struct BaseVarNode {
    BaseVarNode *next;
    char *name;
    int type;      // 0: Number, 1: Array, 2: View
    int ref_count; // For tracking references to this node, especially for views
};

// Specific structure for Number variables (type 0)
struct NumVarNode {
    BaseVarNode base; // Inherits next, name, type, ref_count
    int value;        // Numeric value at offset 0x10 from node start
}; // Total size BaseVarNode + sizeof(int) = 0x10 + 0x4 = 0x14 on 32-bit

// Specific structure for Array variables (type 1)
struct ArrVarNode {
    BaseVarNode base; // Inherits next, name, type, ref_count
    unsigned int size_bytes; // Total bytes allocated for elements (e.g., count * sizeof(int)) (offset 0x10)
    char elements[]; // Flexible array member for the actual data, starts at offset 0x14
}; // Total size BaseVarNode + sizeof(unsigned int) + flexible array

// Structure for View types, used in `viewtypes` array
struct ViewType {
    const char *name;
    int element_size; // e.g., 1, 2, 4 for byte, short, int views
    int signed_flag;  // 0 for unsigned, 1 for signed
}; // Total size 0xc on 32-bit

// Specific structure for View variables (type 2)
struct ViewVarNode {
    BaseVarNode base; // Inherits next, name, type, ref_count
    BaseVarNode *target_array; // Points to the ArrVarNode being viewed (offset 0x10)
    const ViewType *view_type_ptr; // Points to an entry in the global `viewtypes` array (offset 0x14)
    char padding[4]; // Padding to align byte_size_expr at 0x1c for 32-bit architecture
    char *byte_size_expr; // Optional string for custom byte size calculation (offset 0x1c)
}; // Total size 0x20 on 32-bit

// Global variables (assuming these exist and are initialized elsewhere)
BaseVarNode *global_nspace = NULL; // Head of the linked list of variables
BaseVarNode *DAT_00016084 = NULL;  // Pointer to the last added variable node (tail of list)
int DAT_00016088 = 0;              // Counter for total number of variables

// Global arrays for configuration/dispatch (assuming these are initialized elsewhere)
ViewType viewtypes[6]; // Array of view type definitions
const char *types[3]; // Array of strings for "new" command types ("num", "arr", "view")
int (*constructors[3])(const char *, const char *); // Array of function pointers for constructors
const char *cmds[4]; // Array of strings for commands ("new", "get", "set", "del")
int (*handlers[4])(void); // Array of function pointers for command handlers

// Helper function declarations (to make the code compilable)
static inline int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static inline int str2int(const char *s) {
    return atoi(s);
}

static inline unsigned int str2uint(const char *s) {
    return strtoul(s, NULL, 10);
}

static inline void uint2str(char *buf, size_t buf_len, unsigned int val) {
    snprintf(buf, buf_len, "%u", val);
}

static inline void int2str(char *buf, size_t buf_len, int val) {
    snprintf(buf, buf_len, "%d", val);
}

// External functions, assuming they are defined elsewhere (e.g., in a runtime library)
int sendline(int fd, const char *buf, size_t len);
void _terminate(int status);

// Function prototypes (to avoid implicit declarations)
int delvar(const char *name);
int addvar(BaseVarNode *node_ptr, const char *name, int type);
BaseVarNode *getvar(const char *name);
int calc_bytesize(const char *param_1);
int newnum(const char *name, const char *value_str);
int newarr(const char *name, const char *size_str);
int newview(const char *name, const char *target_name);
int handlenew(void);
int handleget(void);
int handleset(void);
int handledel(void);
int runcmd(char *command_line);
int run_viewscript(char *script);

// Function: delvar
int delvar(const char *name_to_delete) {
    BaseVarNode *current = global_nspace;
    BaseVarNode *prev = NULL;

    while (current != NULL) {
        if (streq(current->name, name_to_delete)) {
            if (prev == NULL) {
                global_nspace = current->next;
            } else {
                prev->next = current->next;
            }

            if (current->next == NULL) {
                DAT_00016084 = prev;
            }

            if (current->type == 2) { // View type
                ViewVarNode *view_node = (ViewVarNode *)current;
                if (view_node->target_array != NULL) {
                    view_node->target_array->ref_count--;
                    if (view_node->target_array->ref_count == 0) {
                        free(view_node->target_array);
                    }
                }
                if (view_node->byte_size_expr != NULL) {
                    free(view_node->byte_size_expr);
                }
            }

            if (current->name != NULL) {
                free(current->name);
            }

            free(current);
            DAT_00016088--;
            return 0;
        }
        prev = current;
        current = current->next;
    }
    return 9; // Not found
}

// Function: addvar
int addvar(BaseVarNode *node_ptr, const char *name, int type) {
    node_ptr->name = (char *)calloc(strlen(name) + 1, 1);
    if (node_ptr->name == NULL) {
        return 10; // Memory allocation error
    }
    strcpy(node_ptr->name, name);
    node_ptr->type = type;
    node_ptr->ref_count = 0; // Initialized to 0 by calloc, but explicit is clear

    if (DAT_00016084 != NULL) {
        ((BaseVarNode *)DAT_00016084)->next = node_ptr;
    }
    DAT_00016084 = node_ptr;

    if (global_nspace == NULL) {
        global_nspace = node_ptr;
    }
    node_ptr->next = NULL;

    DAT_00016088++;
    return 0;
}

// Function: getvar
BaseVarNode *getvar(const char *name) {
    if (DAT_00016088 > 0 && global_nspace != NULL) {
        for (BaseVarNode *current = global_nspace; current != NULL; current = current->next) {
            if (streq(current->name, name)) {
                return current;
            }
        }
    }
    return NULL;
}

// Function: calc_bytesize
int calc_bytesize(const char *param_1) {
    int total_bytes = 0;
    char *temp_str = (char *)calloc(strlen(param_1) + 1, 1);
    if (temp_str == NULL) {
        return 0;
    }
    strcpy(temp_str, param_1);

    char *token_context = temp_str;
    char *token;

    while ((token = strtok(token_context, ",")) != NULL) {
        token_context = NULL;

        if (strlen(token) == 0) {
            total_bytes = 0;
            break;
        }

        BaseVarNode *var_node = getvar(token);
        if (var_node == NULL || var_node->type != 0) {
            total_bytes = 0;
            break;
        }
        total_bytes += ((NumVarNode *)var_node)->value;
    }

    free(temp_str);
    return total_bytes;
}

// Function: newnum
int newnum(const char *name, const char *value_str) {
    NumVarNode *node = (NumVarNode *)calloc(1, sizeof(NumVarNode));
    if (node == NULL) {
        return 10; // Memory allocation error
    }

    node->value = str2int(value_str);

    int result = addvar((BaseVarNode *)node, name, 0); // Type 0 for number
    if (result != 0) {
        free(node->base.name);
        free(node);
    }
    return result;
}

// Function: newarr
int newarr(const char *name, const char *size_str) {
    unsigned int element_count = str2uint(size_str);
    if (element_count >= 0x200001) { // Max 2MB array (approx 524288 ints)
        return 11; // Too large
    }

    ArrVarNode *node = (ArrVarNode *)calloc(1, sizeof(BaseVarNode) + sizeof(unsigned int) + (element_count * sizeof(int)));
    if (node == NULL) {
        return 10; // Memory allocation error
    }

    node->size_bytes = element_count * sizeof(int);

    int result = addvar((BaseVarNode *)node, name, 1); // Type 1 for array
    if (result != 0) {
        free(node->base.name);
        free(node);
    }
    return result;
}

// Function: newview
int newview(const char *name, const char *target_name) {
    char *view_type_str = strtok(NULL, " ");
    if (view_type_str == NULL || strlen(view_type_str) == 0) {
        return 6; // Missing view type string
    }

    BaseVarNode *target_node = getvar(target_name);
    if (target_node == NULL) {
        return 9; // Target variable not found
    }
    if (target_node->type != 1) { // Target must be an array (type 1)
        return 12; // Not an array type
    }

    ViewVarNode *node = (ViewVarNode *)calloc(1, sizeof(ViewVarNode));
    if (node == NULL) {
        return 10; // Memory allocation error
    }

    int view_type_idx = -1;
    for (int i = 0; i < 6; i++) {
        if (streq(view_type_str, viewtypes[i].name)) {
            view_type_idx = i;
            break;
        }
    }

    if (view_type_idx == -1) {
        free(node);
        return 4; // Unknown view type
    }

    node->view_type_ptr = &viewtypes[view_type_idx];
    node->target_array = target_node;
    node->target_array->ref_count++; // Increment ref_count of the target array

    int result = addvar((BaseVarNode *)node, name, 2); // Type 2 for view
    if (result != 0) {
        node->target_array->ref_count--; // Decrement if addvar failed
        free(node->base.name);
        free(node);
    }
    return result;
}

// Function: handlenew
int handlenew(void) {
    char *type_str = strtok(NULL, " ");
    if (type_str == NULL || strlen(type_str) == 0) {
        return 6; // Missing type string
    }

    char *name_str = strtok(NULL, " ");
    if (name_str == NULL) {
        return 7; // Missing name string
    }

    if (getvar(name_str) != NULL) {
        return 8; // Variable already exists
    }

    char *value_or_size_str = strtok(NULL, " ");
    if (value_or_size_str == NULL || strlen(value_or_size_str) == 0) {
        return 3; // Missing value/size string
    }

    int type_idx = -1;
    for (int i = 0; i < 3; i++) {
        if (streq(type_str, types[i])) {
            type_idx = i;
            break;
        }
    }

    if (type_idx == -1) {
        return 4; // Unknown type
    }

    return constructors[type_idx](name_str, value_or_size_str);
}

// Function: handleget
int handleget(void) {
    char output_buf[0xC]; // Buffer for string conversion
    memset(output_buf, 0, sizeof(output_buf));

    char *var_name = strtok(NULL, " ");
    if (var_name == NULL || strlen(var_name) == 0) {
        return 7; // Missing variable name
    }

    BaseVarNode *node = getvar(var_name);
    if (node == NULL) {
        return 9; // Variable not found
    }

    if (node->type == 2) { // View type
        ViewVarNode *view_node = (ViewVarNode *)node;
        char *index_str = strtok(NULL, " ");
        if (index_str == NULL) {
            return 3; // Missing index for view
        }
        unsigned int index = str2uint(index_str);

        unsigned int offset = index * view_node->view_type_ptr->element_size;
        ArrVarNode *target_arr_node = (ArrVarNode *)view_node->target_array;

        // Check bounds
        if (view_node->byte_size_expr == NULL) {
            if (target_arr_node->size_bytes < (view_node->view_type_ptr->element_size + offset)) {
                return 13; // Out of bounds
            }
        } else {
            unsigned int calculated_size = calc_bytesize(view_node->byte_size_expr);
            if (calculated_size <= (view_node->view_type_ptr->element_size + offset)) {
                return 13; // Out of bounds
            }
        }

        unsigned int value = 0;
        char *array_elements_ptr = target_arr_node->elements;
        for (unsigned int i = 0; i < view_node->view_type_ptr->element_size; i++) {
            value |= (unsigned int)*(unsigned char *)(array_elements_ptr + offset + i) << ((i * 8) & 0x1f);
        }

        if (view_node->view_type_ptr->signed_flag == 0) { // Unsigned
            uint2str(output_buf, sizeof(output_buf), value);
        } else { // Signed
            if (view_node->view_type_ptr->element_size == 4) {
                int2str(output_buf, sizeof(output_buf), (int)value);
            } else if (view_node->view_type_ptr->element_size == 2) {
                int2str(output_buf, sizeof(output_buf), (short)value);
            } else if (view_node->view_type_ptr->element_size == 1) {
                int2str(output_buf, sizeof(output_buf), (char)value);
            } else {
                uint2str(output_buf, sizeof(output_buf), value); // Fallback to unsigned
            }
        }
    } else if (node->type == 0) { // Number type
        int2str(output_buf, sizeof(output_buf), ((NumVarNode *)node)->value);
    } else if (node->type == 1) { // Array type
        ArrVarNode *arr_node = (ArrVarNode *)node;
        char *index_str = strtok(NULL, " ");
        if (index_str == NULL) {
            return 3; // Missing index for array
        }
        unsigned int index = str2uint(index_str);

        if (arr_node->size_bytes / sizeof(int) <= index) {
            return 13; // Out of bounds
        }
        int2str(output_buf, sizeof(output_buf), ((int *)arr_node->elements)[index]);
    } else {
        return 4; // Unknown variable type
    }

    if (sendline(1, output_buf, strlen(output_buf)) < 0) {
        _terminate(6);
    }
    return 0;
}

// Function: handleset
int handleset(void) {
    char *var_name = strtok(NULL, " ");
    if (var_name == NULL || strlen(var_name) == 0) {
        return 7; // Missing variable name
    }

    BaseVarNode *node = getvar(var_name);
    if (node == NULL) {
        return 9; // Variable not found
    }

    char *field_or_value_str = strtok(NULL, " ");
    if (field_or_value_str == NULL) {
        return 3; // Missing field/value
    }

    if (node->type == 2) { // View type
        ViewVarNode *view_node = (ViewVarNode *)node;
        char *byte_size_expr_str = strtok(NULL, " ");
        if (byte_size_expr_str == NULL) {
            return 3; // Missing byte_size expression for view set
        }

        if (!streq(field_or_value_str, "byteSize")) {
            return 9; // Invalid field for view
        }

        char *temp_byte_size_expr = (char *)calloc(strlen(byte_size_expr_str) + 1, 1);
        if (temp_byte_size_expr == NULL) {
            return 10; // Memory allocation error
        }
        strcpy(temp_byte_size_expr, byte_size_expr_str);

        unsigned int calculated_size = calc_bytesize(temp_byte_size_expr);
        ArrVarNode *target_arr_node = (ArrVarNode *)view_node->target_array;

        if (target_arr_node->size_bytes < calculated_size) {
            free(temp_byte_size_expr);
            return 11; // Byte size too large
        }

        if (view_node->byte_size_expr != NULL) {
            free(view_node->byte_size_expr);
        }
        view_node->byte_size_expr = temp_byte_size_expr;
        return 0; // Success
    } else if (node->type == 0) { // Number type
        ((NumVarNode *)node)->value = str2int(field_or_value_str);
        return 0; // Success
    } else if (node->type == 1) { // Array type
        ArrVarNode *arr_node = (ArrVarNode *)node;
        unsigned int index = str2uint(field_or_value_str); // This is the index

        if (arr_node->size_bytes / sizeof(int) <= index) {
            return 13; // Index out of bounds
        }

        char *value_str = strtok(NULL, " "); // Get the value to set
        if (value_str == NULL) {
            return 3; // Missing value to set
        }

        ((int *)arr_node->elements)[index] = str2int(value_str);
        return 0; // Success
    } else {
        return 4; // Unknown variable type
    }
}

// Function: handledel
int handledel(void) {
    char *var_name = strtok(NULL, " ");
    if (var_name == NULL || strlen(var_name) == 0) {
        return 7; // Missing variable name
    }
    return delvar(var_name);
}

// Function: runcmd
int runcmd(char *command_line) {
    char *cmd_token = strtok(command_line, " ");
    if (cmd_token == NULL || strlen(cmd_token) == 0) {
        return 1; // Empty command
    }

    int cmd_idx = -1;
    for (int i = 0; i < 4; i++) {
        if (streq(cmd_token, cmds[i])) {
            cmd_idx = i;
            break;
        }
    }

    if (cmd_idx == -1) {
        return 5; // Unknown command
    }

    return handlers[cmd_idx]();
}

// Function: run_viewscript
int run_viewscript(char *script) {
    size_t script_len = strlen(script);
    if (script_len >= 0x200001) {
        return 11; // Script too large
    }

    int line_count = 0;
    for (char *p = script; *p != '\0'; p++) {
        if (*p == '\n') {
            line_count++;
        }
    }

    // Original logic: if no newlines, it's an error.
    if (line_count == 0) {
        return 1; // No lines found (or no newlines)
    }

    char **lines = (char **)calloc(line_count + 1, sizeof(char *));
    if (lines == NULL) {
        return 10; // Memory allocation error
    }

    char *line_token = strtok(script, "\n");
    for (int i = 0; i < line_count && line_token != NULL; i++) {
        lines[i] = line_token;
        line_token = strtok(NULL, "\n");
    }
    lines[line_count] = NULL; // Null-terminate the array of pointers

    int result = 0;
    for (int i = 0; i < line_count; i++) {
        result = runcmd(lines[i]);
        if (result != 0) {
            free(lines);
            return result;
        }
    }

    free(lines);

    if (sendline(1, "Done.", 5) < 0) {
        _terminate(6);
    }
    return 0;
}