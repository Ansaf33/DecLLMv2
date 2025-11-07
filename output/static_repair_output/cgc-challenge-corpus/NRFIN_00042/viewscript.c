#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For uint32_t, int32_t, uint8_t

// --- Utility Functions (based on original disassembly) ---

// Custom streq: returns 1 if strings are equal, 0 otherwise
int streq(const char* s1, const char* s2) {
    if (s1 == NULL || s2 == NULL) {
        return s1 == s2; // Both NULL or one NULL means not equal unless both are NULL
    }
    return strcmp(s1, s2) == 0;
}

// Custom _terminate: uses exit from stdlib.h
void _terminate(int status) {
    exit(status);
}

// Custom sendline: sends data to a file descriptor, typically stdout (fd 1) or stderr (fd 2).
// Assuming it adds a newline character for line-based communication.
int sendline(int fd, const void* buf, size_t len) {
    FILE* stream = NULL;
    if (fd == 1) {
        stream = stdout;
    } else if (fd == 2) {
        stream = stderr;
    } else {
        return -1; // Unsupported file descriptor
    }

    if (fwrite(buf, 1, len, stream) != len) {
        return -1;
    }
    if (fputc('\n', stream) == EOF) { // Add newline
        return -1;
    }
    fflush(stream); // Ensure output is sent immediately
    return (int)(len + 1); // Return bytes written including newline
}

// Custom str2int: converts string to int
int32_t str2int(const char* s) {
    return (int32_t)atoi(s); // For simplicity, using atoi. strtol is more robust.
}

// Custom str2uint: converts string to unsigned int
uint32_t str2uint(const char* s) {
    return (uint32_t)strtoul(s, NULL, 10);
}

// Custom int2str: converts int to string
void int2str(char* buf, size_t buf_size, int32_t val) {
    snprintf(buf, buf_size, "%d", val);
}

// Custom uint2str: converts unsigned int to string
void uint2str(char* buf, size_t buf_size, uint32_t val) {
    snprintf(buf, buf_size, "%u", val);
}

// --- Data Structures ---

// Base Node structure (20 bytes, 0x14)
typedef struct VarNode {
    struct VarNode* next;   // Offset 0x00
    char* name;             // Offset 0x04 (points to dynamically allocated string)
    uint32_t type;          // Offset 0x08 (0: Num, 1: Arr, 2: View)
    uint32_t ref_count;     // Offset 0x0C (number of ViewVars referencing this VarNode)
    // Data specific to type follows from 0x10.
    // The memory allocated for this struct might be larger than 20 bytes depending on type.
} VarNode; // Base size 20 bytes (0x14)

// Type 0: Number (NumVar)
// The value is stored directly in the 4 bytes after ref_count.
typedef struct NumVar {
    VarNode base;
    int32_t value; // Offset 0x10 from start of NumVar
} NumVar; // Total size 20 bytes (0x14)

// Type 1: Array (ArrVar)
// The byte_size of the array is stored after ref_count.
// The actual array data (uint32_t elements) follows immediately after the ArrVar struct.
typedef struct ArrVar {
    VarNode base;
    uint32_t byte_size; // Offset 0x10 from start of ArrVar (total bytes for elements)
    // uint32_t elements[]; // Flexible array member (not explicitly declared as FAM, but memory is allocated this way)
} ArrVar; // Base size 20 bytes (0x14) + elements

// Type 2: View (ViewVar)
// It's 32 bytes total.
typedef struct ViewVar {
    VarNode base;
    VarNode* target_var;    // Offset 0x10 from start of ViewVar (pointer to the VarNode being viewed)
    void* view_type_ptr;    // Offset 0x14 (points into viewtypes array)
    uint32_t _padding;      // Offset 0x18 (padding, based on 0x20 total size and 4-byte alignment)
    char* view_script;      // Offset 0x1C (points to dynamically allocated script string, can be NULL)
} ViewVar; // Total size 32 bytes (0x20)

// ViewType structure (12 bytes, 0xc)
typedef struct ViewType {
    char* name;         // Offset 0
    uint32_t byte_size; // Offset 4 (e.g., 1, 2, 4 for char, short, int)
    uint32_t is_signed; // Offset 8 (0 for unsigned, 1 for signed)
} ViewType; // Total size 12 bytes (0xc)

// --- Global Variables ---

VarNode* global_nspace = NULL; // Head of the linked list of variables
VarNode* DAT_00016084 = NULL;  // Tail of the linked list of variables
int DAT_00016088 = 0;          // Count of variables in the global namespace

// View Types Array
ViewType viewtypes[] = {
    {"byte", 1, 1},
    {"word", 2, 1},
    {"dword", 4, 1},
    {"ubyte", 1, 0},
    {"uword", 2, 0},
    {"udword", 4, 0}
};

// --- Function Prototypes for Handlers and Constructors ---

// Constructor function pointer type: int (*)(char* name, char* value_str)
typedef int (*constructor_func)(char* name, char* value_str);

// Handler function pointer type: uint32_t (*)(void)
typedef uint32_t (*handler_func)(void);

// Variable type names
char* types[] = {
    "num",
    "arr",
    "view",
    NULL
};

// Command names
char* cmds[] = {
    "new",
    "get",
    "set",
    "del",
    NULL
};

// Forward declarations for functions used in global arrays
int newnum(char* name, char* value_str);
int newarr(char* name, char* value_str);
int newview(char* name, char* value_str);
uint32_t handlenew(void);
uint32_t handleget(void);
uint32_t handleset(void);
uint32_t handledel(void);
uint32_t runcmd(char* command_line); // Declared here for constructors/handlers, defined later
int calc_bytesize(char* script_str); // Declared here, defined later

// Constructor functions array
constructor_func constructors[] = {
    newnum,
    newarr,
    newview,
    NULL
};

// Command handler functions array
handler_func handlers[] = {
    handlenew,
    handleget,
    handleset,
    handledel,
    NULL
};

// --- Core Logic Functions ---

// Function: getvar
VarNode* getvar(char* name) {
    if (DAT_00016088 > 0 && global_nspace != NULL) {
        for (VarNode* current = global_nspace; current != NULL; current = current->next) {
            if (streq(current->name, name)) {
                return current;
            }
        }
    }
    return NULL;
}

// Function: addvar
uint32_t addvar(VarNode* new_var, char* name, uint32_t type) {
    new_var->name = (char*)calloc(strlen(name) + 1, 1);
    if (new_var->name == NULL) {
        return 10; // Memory allocation error
    }
    strcpy(new_var->name, name);

    new_var->type = type;

    if (DAT_00016084 != NULL) {
        DAT_00016084->next = new_var;
    }
    DAT_00016084 = new_var;
    new_var->next = NULL;

    if (global_nspace == NULL) {
        global_nspace = new_var;
    }

    DAT_00016088++;
    return 0;
}

// Function: delvar
uint32_t delvar(char* name) {
    VarNode* prev = NULL;
    VarNode* current = global_nspace;

    while (current != NULL) {
        if (streq(current->name, name)) {
            break;
        }
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        return 9; // Not found
    }

    if (prev == NULL) {
        global_nspace = current->next;
    } else {
        prev->next = current->next;
    }

    if (current == DAT_00016084) {
        DAT_00016084 = prev;
    }

    if (current->type == 2) { // View type
        ViewVar* view_var = (ViewVar*)current;
        if (view_var->target_var != NULL) {
            view_var->target_var->ref_count--;
            if (view_var->target_var->ref_count == 0) {
                free(view_var->target_var->name); // Free target's name
                free(view_var->target_var);        // Free target VarNode
            }
        }
        if (view_var->view_script != NULL) {
            free(view_var->view_script);
        }
    }

    free(current->name); // Free the name of the deleted variable

    // Original code: if ((int)local_10[3] < 1) { free(local_10); }
    // This implies `current->ref_count == 0`.
    if (current->ref_count == 0) {
        free(current);
    } // Potential memory leak if ref_count > 0 but removed from global_nspace

    DAT_00016088--;
    return 0;
}

// Function: calc_bytesize
int calc_bytesize(char* script_str) {
    int total_bytes = 0;
    char* script_copy = (char*)calloc(strlen(script_str) + 1, 1);
    if (script_copy == NULL) {
        return 0;
    }
    strcpy(script_copy, script_str);

    char* token = strtok(script_copy, ",");
    while (token != NULL) {
        if (strlen(token) == 0) {
            total_bytes = 0;
            break;
        }
        VarNode* var = getvar(token);
        if (var == NULL) {
            total_bytes = 0;
            break;
        }
        if (var->type != 0) { // Must be a number type
            total_bytes = 0;
            break;
        }
        total_bytes += ((NumVar*)var)->value;
        token = strtok(NULL, ",");
    }

    free(script_copy);
    return total_bytes;
}

// Function: newnum
int newnum(char* name, char* value_str) {
    NumVar* new_num_var = (NumVar*)calloc(1, sizeof(NumVar));
    if (new_num_var == NULL) {
        return 10; // Memory allocation error
    }

    new_num_var->value = str2int(value_str);
    int ret = addvar((VarNode*)new_num_var, name, 0); // Type 0 for number

    if (ret != 0) {
        if (new_num_var->base.name) free(new_num_var->base.name); // Free allocated name if addvar failed
        free(new_num_var);
    }
    return ret;
}

// Function: newarr
int newarr(char* name, char* size_str) {
    uint32_t num_elements = str2uint(size_str);
    if (num_elements >= 0x200001) { // Max size check (e.g., ~8MB for elements)
        return 0xB; // Size too large
    }

    ArrVar* new_arr_var = (ArrVar*)calloc(1, sizeof(ArrVar) + num_elements * sizeof(uint32_t));
    if (new_arr_var == NULL) {
        return 10; // Memory allocation error
    }

    new_arr_var->byte_size = num_elements * sizeof(uint32_t);
    int ret = addvar((VarNode*)new_arr_var, name, 1); // Type 1 for array

    if (ret != 0) {
        if (new_arr_var->base.name) free(new_arr_var->base.name);
        free(new_arr_var);
    }
    return ret;
}

// Function: newview
int newview(char* name, char* target_name) {
    char* view_type_str = strtok(NULL, " ");
    if (view_type_str == NULL || strlen(view_type_str) == 0) {
        return 6; // Missing view type
    }

    VarNode* target_var = getvar(target_name);
    if (target_var == NULL) {
        return 9; // Target variable not found
    }
    if (target_var->type != 1) { // Target must be an array type
        return 0xC; // Type mismatch (not an array)
    }

    ViewVar* new_view_var = (ViewVar*)calloc(1, sizeof(ViewVar));
    if (new_view_var == NULL) {
        return 10; // Memory allocation error
    }

    ViewType* selected_view_type = NULL;
    for (uint32_t i = 0; i < sizeof(viewtypes) / sizeof(ViewType); i++) {
        if (streq(view_type_str, viewtypes[i].name)) {
            selected_view_type = &viewtypes[i];
            break;
        }
    }

    if (selected_view_type == NULL) {
        free(new_view_var);
        return 4; // Invalid view type string
    }

    new_view_var->target_var = target_var;
    new_view_var->view_type_ptr = selected_view_type;
    new_view_var->target_var->ref_count++; // Increment target's reference count

    int ret = addvar((VarNode*)new_view_var, name, 2); // Type 2 for view

    if (ret != 0) {
        new_view_var->target_var->ref_count--; // Decrement ref count if creation failed
        if (new_view_var->base.name) free(new_view_var->base.name);
        free(new_view_var);
    }
    return ret;
}

// Function: handlenew
uint32_t handlenew(void) {
    char* type_str = strtok(NULL, " ");
    if (type_str == NULL || strlen(type_str) == 0) {
        return 6; // Missing type
    }

    char* name = strtok(NULL, " ");
    if (name == NULL || strlen(name) == 0) {
        return 7; // Missing name
    }

    if (getvar(name) != NULL) {
        return 8; // Variable with this name already exists
    }

    char* value_str = strtok(NULL, " ");
    if (value_str == NULL || strlen(value_str) == 0) {
        return 3; // Missing value/size/target
    }

    uint32_t type_idx = 0;
    while (types[type_idx] != NULL) {
        if (streq(type_str, types[type_idx])) {
            return constructors[type_idx](name, value_str);
        }
        type_idx++;
    }
    return 4; // Invalid type string
}

// Function: handleget
uint32_t handleget(void) {
    char var_value_str_buffer[12]; // Buffer for int/uint to string conversion

    char* name = strtok(NULL, " ");
    if (name == NULL || strlen(name) == 0) {
        return 7; // Missing variable name
    }

    VarNode* var = getvar(name);
    if (var == NULL) {
        return 9; // Variable not found
    }

    if (var->type == 2) { // View type
        ViewVar* view_var = (ViewVar*)var;
        char* index_str = strtok(NULL, " ");
        if (index_str == NULL) {
            return 3; // Missing index for view
        }
        uint32_t index = str2uint(index_str);

        ViewType* view_type = (ViewType*)view_var->view_type_ptr;
        uint32_t offset = index * view_type->byte_size;

        if (view_var->view_script == NULL) {
            ArrVar* target_arr = (ArrVar*)view_var->target_var;
            if (target_arr->byte_size < offset + view_type->byte_size) {
                return 0xD; // Out of bounds
            }
        } else {
            uint32_t scripted_byte_size = calc_bytesize(view_var->view_script);
            if (scripted_byte_size <= offset + view_type->byte_size) {
                return 0xD; // Out of bounds
            }
        }

        uint32_t raw_value = 0;
        char* array_data_ptr = (char*)view_var->target_var + sizeof(ArrVar);

        for (uint32_t i = 0; i < view_type->byte_size; i++) {
            raw_value |= (uint32_t)*(uint8_t*)(array_data_ptr + offset + i) << ((i << 3) & 0x1f);
        }

        if (view_type->is_signed == 0) { // Unsigned
            uint2str(var_value_str_buffer, sizeof(var_value_str_buffer), raw_value);
        } else { // Signed
            if (view_type->byte_size == 4) {
                int2str(var_value_str_buffer, sizeof(var_value_str_buffer), (int32_t)raw_value);
            } else if (view_type->byte_size == 2) {
                int2str(var_value_str_buffer, sizeof(var_value_str_buffer), (int32_t)(int16_t)raw_value);
            } else if (view_type->byte_size == 1) {
                int2str(var_value_str_buffer, sizeof(var_value_str_buffer), (int32_t)(int8_t)raw_value);
            } else {
                uint2str(var_value_str_buffer, sizeof(var_value_str_buffer), raw_value); // Fallback
            }
        }
    } else if (var->type == 0) { // Num type
        int2str(var_value_str_buffer, sizeof(var_value_str_buffer), ((NumVar*)var)->value);
    } else if (var->type == 1) { // Arr type
        ArrVar* arr_var = (ArrVar*)var;
        char* index_str = strtok(NULL, " ");
        if (index_str == NULL) {
            return 3; // Missing index for array
        }
        uint32_t index = str2uint(index_str);

        if (arr_var->byte_size / sizeof(uint32_t) <= index) {
            return 0xD; // Out of bounds
        }
        uint32_t* elements = (uint32_t*)((char*)arr_var + sizeof(ArrVar));
        int2str(var_value_str_buffer, sizeof(var_value_str_buffer), elements[index]);
    } else {
        return 4; // Unknown type
    }

    int bytes_sent = sendline(1, var_value_str_buffer, strlen(var_value_str_buffer));
    if (bytes_sent < 0) {
        _terminate(6);
    }
    return 0;
}

// Function: handleset
uint32_t handleset(void) {
    char* name = strtok(NULL, " ");
    if (name == NULL || strlen(name) == 0) {
        return 7; // Missing variable name
    }

    VarNode* var = getvar(name);
    if (var == NULL) {
        return 9; // Variable not found
    }

    char* value_or_index_str = strtok(NULL, " ");
    if (value_or_index_str == NULL) {
        return 3; // Missing value/index
    }

    if (var->type == 2) { // View type
        ViewVar* view_var = (ViewVar*)var;
        char* field_name = value_or_index_str; // "byteSize"
        char* script_str = strtok(NULL, " ");
        if (script_str == NULL) {
            return 3; // Missing script string
        }

        if (!streq(field_name, "byteSize")) {
            return 9; // Invalid field name for view
        }

        char* new_script = (char*)calloc(strlen(script_str) + 1, 1);
        if (new_script == NULL) {
            return 10; // Memory allocation error
        }
        strcpy(new_script, script_str);

        uint32_t required_byte_size = calc_bytesize(new_script);
        ArrVar* target_arr = (ArrVar*)view_var->target_var;
        if (target_arr->byte_size < required_byte_size) {
            free(new_script);
            return 0xB; // Scripted size exceeds target array capacity
        }

        if (view_var->view_script != NULL) {
            free(view_var->view_script);
        }
        view_var->view_script = new_script;
        return 0;
    } else if (var->type == 0) { // Num type
        ((NumVar*)var)->value = str2int(value_or_index_str);
        return 0;
    } else if (var->type == 1) { // Arr type
        ArrVar* arr_var = (ArrVar*)var;
        uint32_t index = str2uint(value_or_index_str);
        if (arr_var->byte_size / sizeof(uint32_t) <= index) {
            return 0xD; // Out of bounds
        }

        char* value_str = strtok(NULL, " ");
        if (value_str == NULL) {
            return 3; // Missing value for array element
        }
        int32_t value = str2int(value_str);

        uint32_t* elements = (uint32_t*)((char*)arr_var + sizeof(ArrVar));
        elements[index] = value;
        return 0;
    } else {
        return 4; // Unknown type
    }
}

// Function: handledel
uint32_t handledel(void) {
    char* name = strtok(NULL, " ");
    if (name == NULL || strlen(name) == 0) {
        return 7; // Missing variable name
    }
    return delvar(name);
}

// Function: runcmd
uint32_t runcmd(char* command_line) {
    char* cmd_token = strtok(command_line, " ");
    if (cmd_token == NULL || strlen(cmd_token) == 0) {
        return 1; // Empty command
    }

    uint32_t cmd_idx = 0;
    while (cmds[cmd_idx] != NULL) {
        if (streq(cmd_token, cmds[cmd_idx])) {
            return handlers[cmd_idx]();
        }
        cmd_idx++;
    }
    return 5; // Unknown command
}

// Function: run_viewscript
int run_viewscript(char* script_content) {
    if (strlen(script_content) >= 0x200001) { // Max script size check (~2MB)
        return 0xB;
    }

    char* script_copy = (char*)calloc(strlen(script_content) + 1, 1);
    if (script_copy == NULL) {
        return 10; // Allocation error
    }
    strcpy(script_copy, script_content);

    int num_lines = 0;
    for (char* p = script_copy; *p != '\0'; p++) {
        if (*p == '\n') {
            num_lines++;
        }
    }
    if (strlen(script_copy) > 0 && script_copy[strlen(script_copy) - 1] != '\n') {
        num_lines++; // Count last line if not ending with newline
    }

    if (num_lines == 0) {
        free(script_copy);
        return 1; // Empty script or no commands
    }

    char** lines = (char**)calloc(num_lines, sizeof(char*));
    if (lines == NULL) {
        free(script_copy);
        return 10; // Memory allocation error
    }

    char* line = strtok(script_copy, "\n");
    int current_line_idx = 0;
    while (line != NULL && current_line_idx < num_lines) {
        lines[current_line_idx++] = line;
        line = strtok(NULL, "\n");
    }

    for (int i = 0; i < num_lines; i++) {
        int ret = runcmd(lines[i]);
        if (ret != 0) {
            free(lines);
            free(script_copy);
            return ret;
        }
    }

    free(lines);
    free(script_copy);

    int bytes_sent = sendline(1, "Done.", 5); // Original length 5
    if (bytes_sent < 0) {
        _terminate(6);
    }
    return 0;
}

// Function: main
int main(void) {
    char input_buffer[1024];
    int ret_code = 0;

    sendline(1, "Welcome! Type 'help' or commands.", strlen("Welcome! Type 'help' or commands."));

    while (1) {
        sendline(1, "> ", 2); // Prompt
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            sendline(1, "Exiting...", strlen("Exiting..."));
            break;
        }

        input_buffer[strcspn(input_buffer, "\n")] = 0; // Remove trailing newline

        if (strlen(input_buffer) == 0) {
            continue; // Empty line
        }

        if (streq(input_buffer, "exit")) {
            sendline(1, "Exiting...", strlen("Exiting..."));
            break;
        }
        if (streq(input_buffer, "help")) {
            sendline(1, "Commands: new <type> <name> <value/size/target_name> [view_type/script]", strlen("Commands: new <type> <name> <value/size/target_name> [view_type/script]"));
            sendline(1, "          get <name> [index]", strlen("          get <name> [index]"));
            sendline(1, "          set <name> <value/index> [value_for_array]", strlen("          set <name> <value/index> [value_for_array]"));
            sendline(1, "          del <name>", strlen("          del <name>"));
            sendline(1, "          run_script <script_string_literal>", strlen("          run_script <script_string_literal>"));
            sendline(1, "          exit", strlen("          exit"));
            continue;
        }

        if (strncmp(input_buffer, "run_script ", strlen("run_script ")) == 0) {
            char* script_arg = input_buffer + strlen("run_script ");
            ret_code = run_viewscript(script_arg);
            if (ret_code != 0) {
                char error_msg[64];
                snprintf(error_msg, sizeof(error_msg), "Script error: %d", ret_code);
                sendline(2, error_msg, strlen(error_msg));
            }
        } else {
            char cmd_copy[sizeof(input_buffer)];
            strncpy(cmd_copy, input_buffer, sizeof(cmd_copy) - 1);
            cmd_copy[sizeof(cmd_copy) - 1] = '\0';

            ret_code = runcmd(cmd_copy);
            if (ret_code != 0) {
                char error_msg[64];
                snprintf(error_msg, sizeof(error_msg), "Error: %d", ret_code);
                sendline(2, error_msg, strlen(error_msg));
            }
        }
    }

    // Cleanup global_nspace before exiting
    VarNode* current = global_nspace;
    while (current != NULL) {
        VarNode* next = current->next;
        if (current->name) free(current->name);
        if (current->type == 2) {
            ViewVar* view_var = (ViewVar*)current;
            if (view_var->view_script) free(view_var->view_script);
            // The target_var is handled by delvar's ref_count logic.
            // If the target_var's ref_count is still > 0, it won't be freed here.
            // This reflects the original code's potential memory management quirks.
        }
        free(current);
        current = next;
    }

    return 0;
}