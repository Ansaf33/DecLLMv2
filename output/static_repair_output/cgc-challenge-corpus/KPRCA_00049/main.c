#include <stdio.h>    // For dprintf, vsnprintf
#include <stdlib.h>   // For malloc, calloc, realloc, free, _Exit
#include <string.h>   // For strlen, strcpy, strncpy, strcat, strcmp, memset, memcpy, strdup, strstr
#include <unistd.h>   // For write, read
#include <ctype.h>    // For isspace
#include <stdarg.h>   // For va_list, va_start, va_end

// Define types from the decompiled code, assuming a 32-bit environment
// where int and pointers are 4 bytes.
typedef char undefined;
typedef unsigned int undefined4;

// Custom exit function to replace _terminate
void _terminate(int __status) {
    _Exit(__status); // Use _Exit for immediate termination without cleanup
}

// Function: exit - Keep only one, replace _terminate
void exit(int __status) {
    _terminate(__status);
}

// Custom fdprintf (file descriptor printf)
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    // Use a dynamically allocated buffer for safety, or a large static one.
    // Given the context (embedded-like), a fixed large stack buffer is common.
    char buffer[4096];
    int ret = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (ret > 0) {
        // Ensure not to write more than buffer size if vsnprintf truncates
        write(fd, buffer, (size_t)ret < sizeof(buffer) ? ret : sizeof(buffer) - 1);
    }
    return ret;
}

// Custom transmit_string
void transmit_string(int fd, const char *str) {
    if (str != NULL) {
        write(fd, str, strlen(str));
    }
}

// Custom read_until
// Reads from `fd` into `buf` until `delim` is found or `max_len` bytes are read.
// Returns the number of bytes read (excluding delimiter), or -1 on error.
int read_until(int fd, char *buf, size_t max_len, char delim) {
    size_t bytes_read = 0;
    while (bytes_read < max_len) {
        ssize_t ret = read(fd, buf + bytes_read, 1);
        if (ret <= 0) { // EOF or error
            if (bytes_read > 0) {
                buf[bytes_read] = '\0';
                return bytes_read;
            }
            return -1;
        }
        if (buf[bytes_read] == delim) {
            buf[bytes_read] = '\0';
            return bytes_read;
        }
        bytes_read++;
    }
    buf[bytes_read] = '\0'; // Ensure null termination
    return bytes_read;
}

// Global variables and messages
const char *PROMPT_MSG = "> ";
const char *ERROR_MSG = "ERROR: Unknown command or invalid arguments\n";
const char *EXIT_CMD = "exit";
const char *NEW_CMD = "new";
const char *APPEND_CMD = "append";
const char *DELETE_CMD = "delete";
const char *READ_CMD = "read";
const char *NOTE_FORMAT = "Note: %s\n";
const char *STATUS_NOTE_EXISTS = "Note already exists\n";
const char *STATUS_NOTE_CREATED = "Note created\n";
const char *STATUS_THUNK_CREATED = "Thunk created and queued\n";
const char *STATUS_NOTE_NOT_FOUND = "Note not found\n";

// Structure definitions
struct Note {
    size_t content_len;
    size_t content_capacity;
    char *content;
    char *name;
};

// Global array for notes
struct Note *note_store[0x200]; // 512 entries

struct Thunk {
    unsigned int count; // Number of arguments stored in args
    char **args;        // Array of argument strings (char*), args[0] is the command
    unsigned int id;    // A numerical ID for the thunk (not used functionally here)
};

struct List {
    struct List *prev;
    struct List *next;
    struct Thunk *data; // Stores a pointer to a Thunk
};

// Global list head for thunks
struct List *thunk_list_head = NULL;

// Function: get_note
struct Note *get_note(const char *name) {
    for (unsigned int i = 0; i < 0x200; ++i) {
        if (note_store[i] != NULL && strcmp(name, note_store[i]->name) == 0) {
            return note_store[i];
        }
    }
    return NULL;
}

// Function: new_note
struct Note *new_note(const char *name, const char *content) {
    if (content == NULL || name == NULL) {
        return NULL;
    }

    if (get_note(name) != NULL) {
        return NULL; // Note with this name already exists
    }

    struct Note *new_n = (struct Note *)calloc(1, sizeof(struct Note));
    if (new_n == NULL) {
        fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0x4b);
        exit(1);
    }

    size_t content_len = strlen(content);
    size_t content_capacity;

    if (content_len + 1 < 0x401) { // Allocate at least 0x400 (1024) bytes if content is small
        content_capacity = 0x400;
    } else {
        content_capacity = content_len + 1;
    }
    new_n->content = (char *)calloc(1, content_capacity);

    if (new_n->content == NULL) {
        fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0x59);
        free(new_n);
        exit(1);
    }
    new_n->content_len = content_len;
    new_n->content_capacity = content_capacity;
    strcpy(new_n->content, content);

    size_t name_len = strlen(name);
    new_n->name = (char *)calloc(1, name_len + 1);
    if (new_n->name == NULL) {
        fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0x60);
        free(new_n->content);
        free(new_n);
        exit(1);
    }
    strcpy(new_n->name, name);

    for (unsigned int i = 0; i < 0x200; ++i) {
        if (note_store[i] == NULL) {
            note_store[i] = new_n;
            return new_n;
        }
    }

    // If no space in note_store
    free(new_n->name);
    free(new_n->content);
    free(new_n);
    return NULL;
}

// Function: append_note
struct Note *append_note(struct Note *note, const char *text_to_append) {
    if (note == NULL || text_to_append == NULL) {
        return NULL;
    }

    size_t append_len = strlen(text_to_append);
    size_t required_len = note->content_len + append_len + 1;

    if (note->content_capacity < required_len) {
        size_t new_capacity = note->content_capacity * 2;
        if (new_capacity < required_len) { // Ensure new_capacity is at least required_len
            new_capacity = required_len;
        }
        char *new_content = (char *)realloc(note->content, new_capacity);
        if (new_content == NULL) {
            fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0x7c);
            exit(1);
        }
        note->content = new_content;
        note->content_capacity = new_capacity;
    }

    strcat(note->content, text_to_append);
    note->content_len += append_len;

    return note;
}

// Function: delete_note
undefined4 delete_note(struct Note *note) {
    if (note == NULL) {
        return 0xffffffff; // Error
    }

    for (unsigned int i = 0; i < 0x200; ++i) {
        if (note_store[i] == note) {
            note_store[i] = NULL;
            break;
        }
    }

    if (note->content != NULL) {
        free(note->content);
    }
    if (note->name != NULL) {
        free(note->name);
    }
    free(note);
    return 0; // Success
}

// Function: free_thunk
void free_thunk(struct Thunk *thunk) {
    if (thunk == NULL) return;

    if (thunk->args != NULL) {
        for (unsigned int i = 0; i < thunk->count; ++i) {
            if (thunk->args[i] != NULL) {
                free(thunk->args[i]);
            }
        }
        free(thunk->args);
    }
    free(thunk);
}

// Function: new_thunk
struct Thunk *new_thunk(unsigned int id, unsigned int count, char **args_array) {
    if (count == 0 || count >= 5 || args_array == NULL) { // Original code had `param_2 < 5`
        return NULL;
    }

    struct Thunk *new_t = (struct Thunk *)calloc(1, sizeof(struct Thunk)); // 12 bytes on 32-bit
    if (new_t == NULL) {
        fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0xb4);
        exit(1);
    }

    new_t->id = id;
    new_t->count = count;
    new_t->args = (char **)calloc(count, sizeof(char *)); // Array of char pointers
    if (new_t->args == NULL) {
        fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0xb9);
        free(new_t);
        exit(1);
    }

    for (unsigned int i = 0; i < new_t->count; ++i) {
        if (args_array[i] == NULL) {
            free_thunk(new_t);
            return NULL;
        }
        size_t arg_len = strlen(args_array[i]);
        new_t->args[i] = (char *)calloc(1, arg_len + 1);
        if (new_t->args[i] == NULL) {
            fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0xc1);
            free_thunk(new_t);
            exit(1);
        }
        strcpy(new_t->args[i], args_array[i]);
    }
    return new_t;
}

// Function: run_thunk
// This function interprets the thunk data as a command to execute.
void run_thunk(struct List *list_head_ptr) {
    if (list_head_ptr == NULL || list_head_ptr->data == NULL) {
        return;
    }
    struct Thunk *thunk = list_head_ptr->data;
    if (thunk->count == 0 || thunk->args == NULL || thunk->args[0] == NULL) {
        fdprintf(2, "ERROR: Invalid thunk data (ID: %u).\n", thunk->id);
        return;
    }

    const char *command = thunk->args[0];

    if (strcmp(command, APPEND_CMD) == 0) {
        if (thunk->count < 3) { // Expecting "append NAME CONTENT"
            fdprintf(2, "ERROR: Append thunk (ID: %u) requires NAME and CONTENT.\n", thunk->id);
            return;
        }
        char *note_name = thunk->args[1];
        char *content_to_append = thunk->args[2];

        char *modified_content = strdup(content_to_append);
        if (modified_content == NULL) {
            fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0xf6);
            exit(1);
        }

        const char *find_str = "cloud";
        const char *replace_str = "butt ";
        size_t find_len = strlen(find_str);
        size_t replace_len = strlen(replace_str);

        char *current_pos = modified_content;
        while ((current_pos = strstr(current_pos, find_str)) != NULL) {
            memmove(current_pos + replace_len, current_pos + find_len, strlen(current_pos + find_len) + 1);
            memcpy(current_pos, replace_str, replace_len);
            current_pos += replace_len;
        }

        struct Note *note = get_note(note_name);
        if (note != NULL) {
            if (append_note(note, modified_content) == NULL) {
                fdprintf(2, "ERROR: Failed to append to note '%s' via thunk (ID: %u)\n", note_name, thunk->id);
            } else {
                fdprintf(1, "Appended to note '%s' via thunk (ID: %u).\n", note_name, thunk->id);
            }
        } else {
            fdprintf(2, "ERROR: Note '%s' not found for append via thunk (ID: %u).\n", note_name, thunk->id);
        }
        free(modified_content);

    } else if (strcmp(command, DELETE_CMD) == 0) {
        if (thunk->count < 2) { // Expecting "delete NAME"
            fdprintf(2, "ERROR: Delete thunk (ID: %u) requires NAME.\n", thunk->id);
            return;
        }
        char *note_name = thunk->args[1];
        struct Note *note = get_note(note_name);
        if (note != NULL) {
            if (delete_note(note) == 0) { // 0 is success
                fdprintf(1, "Deleted note '%s' via thunk (ID: %u).\n", note_name, thunk->id);
            } else {
                fdprintf(2, "ERROR: Failed to delete note '%s' via thunk (ID: %u).\n", note_name, thunk->id);
            }
        } else {
            fdprintf(2, "ERROR: Note '%s' not found for delete via thunk (ID: %u).\n", note_name, thunk->id);
        }
    } else {
        fdprintf(2, "ERROR: Unknown thunk command: %s (ID: %u)\n", command, thunk->id);
    }
}

// Function: new_list
struct List *new_list(struct List *prev_node, struct List *next_node, struct Thunk *data) {
    struct List *new_l = (struct List *)calloc(1, sizeof(struct List));
    if (new_l == NULL) {
        fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0x120);
        exit(1);
    }
    new_l->prev = prev_node;
    new_l->next = next_node;
    new_l->data = data;
    return new_l;
}

// Function: append_list
undefined4 append_list(struct List **list_head_ptr, struct Thunk *thunk_data) {
    if (list_head_ptr == NULL) return 0xffffffff;

    struct List *new_node = new_list(NULL, NULL, thunk_data);
    if (new_node == NULL) return 0xffffffff; // new_list already calls exit on alloc failure

    if (*list_head_ptr == NULL) {
        *list_head_ptr = new_node;
    } else {
        struct List *current = *list_head_ptr;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        new_node->prev = current;
    }
    return 0;
}

// Function: remove_head_list
// Removes the head of the list, frees its data (thunk), and the list node itself.
struct List **remove_head_list(struct List **list_head_ptr) {
    if (list_head_ptr == NULL || *list_head_ptr == NULL) {
        return list_head_ptr; // Nothing to remove
    }

    struct List *old_head = *list_head_ptr;
    *list_head_ptr = old_head->next;

    if (*list_head_ptr != NULL) {
        (*list_head_ptr)->prev = NULL;
    }

    free_thunk(old_head->data); // Free the thunk data associated with the removed list item
    free(old_head); // Free the list item itself
    return list_head_ptr;
}

// Function: len_list (unused but kept for completeness)
int len_list(struct List *list_head) {
    int count = 0;
    struct List *current = list_head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Function: prompt
void prompt(int fd) {
  transmit_string(fd, PROMPT_MSG);
}

// Function: yell
void yell(int fd) {
  transmit_string(fd, ERROR_MSG);
}

// Function: nth_word
// Extracts the word at `index` (0-based) from `full_string`.
// If `full_string_flag` is 1, returns the rest of the string from `index` onwards.
char *nth_word(int index, char *full_string, int full_string_flag) {
    if (full_string == NULL) return NULL;

    char *token_start = full_string;
    char *result = NULL;

    for (int i = 0; i <= index; ++i) {
        // Skip leading whitespace
        while (*token_start != '\0' && isspace((unsigned char)*token_start)) {
            token_start++;
        }
        if (*token_start == '\0') { // End of string
            return NULL;
        }

        if (i == index) {
            if (full_string_flag) {
                result = strdup(token_start);
            } else {
                char *space_pos = strchr(token_start, ' ');
                if (space_pos == NULL) {
                    result = strdup(token_start);
                } else {
                    size_t len = space_pos - token_start;
                    result = (char *)calloc(1, len + 1);
                    if (result == NULL) {
                        fdprintf(2,"ERROR %s:%d:\tbad alloc\n","src/main.c",0x188);
                        exit(1);
                    }
                    strncpy(result, token_start, len);
                    result[len] = '\0';
                }
            }
            return result;
        } else {
            // Find the end of the current word
            char *word_end = token_start;
            while (*word_end != '\0' && !isspace((unsigned char)*word_end)) {
                word_end++;
            }
            // Move token_start past the current word for the next iteration
            token_start = word_end;
        }
    }
    return NULL;
}

// Function: make_argv
// Parses `command_line` to extract `num_args` arguments *after* the initial command word.
// Returns `char**` (array of strings for the arguments), or `NULL` on failure.
char **make_argv(const char *command_line, size_t num_args) {
    if (command_line == NULL) {
        return NULL;
    }

    char **argv_array = (char **)calloc(num_args, sizeof(char*));
    if (argv_array == NULL) {
        fdprintf(2, "ERROR %s:%d:\tbad alloc\n", "src/main.c", 0x188);
        exit(1);
    }

    // Find the starting position for arguments (after the first word/command)
    char *arg_parse_start = strchr(command_line, ' ');
    if (arg_parse_start == NULL) { // No space after command implies no arguments
        if (num_args > 0) { // Expected arguments but none found
            free(argv_array);
            return NULL;
        }
        return argv_array; // Return empty array if no args expected
    }

    // Skip any additional whitespace after the first word
    while (*arg_parse_start != '\0' && isspace((unsigned char)*arg_parse_start)) {
        arg_parse_start++;
    }

    int success = 1;
    for (size_t i = 0; i < num_args; ++i) {
        int full_string = (i == num_args - 1) ? 1 : 0; // Last arg gets the rest of the string
        argv_array[i] = nth_word(0, arg_parse_start, full_string); // Get the first word from current position
        if (argv_array[i] == NULL) {
            success = 0;
            break;
        }

        if (!full_string) {
            char *space_after_arg = strchr(arg_parse_start, ' ');
            if (space_after_arg != NULL) {
                arg_parse_start = space_after_arg;
                while (*arg_parse_start != '\0' && isspace((unsigned char)*arg_parse_start)) {
                    arg_parse_start++;
                }
            } else if (i < num_args - 1) { // Expected more args but no more spaces
                success = 0;
                break;
            }
        }
    }

    if (!success) {
        // Use free_argv to clean up already allocated strings
        for (size_t i = 0; i < num_args; ++i) {
            if (argv_array[i] != NULL) {
                free(argv_array[i]);
            }
        }
        free(argv_array);
        return NULL;
    }
    return argv_array;
}

// Function: print_argv (placeholder, original was empty)
void print_argv(char **argv, unsigned int count) {
    // Original function was empty. Can be used for debugging.
}

// Function: free_argv
void free_argv(char **argv, unsigned int count) {
    if (argv != NULL && count != 0) {
        for (unsigned int i = 0; i < count; ++i) {
            if (argv[i] != NULL) {
                free(argv[i]);
            }
        }
        free(argv);
    }
}

// Function: main
undefined4 main(void) {
    char cmd_buf[0x4000]; // Buffer for command line input
    char *command_word = NULL;
    char **argv_ptr = NULL;
    struct Note *note_result = NULL;
    struct Thunk *thunk_result = NULL;
    const char *status_message = NULL;

    // Main command loop
    while (1) {
        status_message = NULL; // Reset status message for each iteration

        // Process one pending thunk from the list head if any
        if (thunk_list_head != NULL) {
            run_thunk(thunk_list_head);
            remove_head_list(&thunk_list_head); // Remove after running
        }

        prompt(1); // Prompt on stdout

        // Clear buffer and read input
        memset(cmd_buf, 0, sizeof(cmd_buf));
        int bytes_read = read_until(0, cmd_buf, sizeof(cmd_buf) - 1, '\n'); // Read from stdin
        if (bytes_read < 0) {
            break; // EOF or error, exit main loop
        }
        if (bytes_read == 0) { // Empty input, continue loop
            continue;
        }

        command_word = nth_word(0, cmd_buf, 0); // Get the first word (command)
        if (command_word == NULL) {
            yell(1); // Unknown command or empty input after trimming
            // free(NULL) is safe, so no explicit check needed before free(command_word) below
        }
        else if (strcmp(command_word, EXIT_CMD) == 0) {
            free(command_word);
            command_word = NULL;
            break; // Exit the main loop
        }
        else if (strcmp(command_word, NEW_CMD) == 0) {
            // new NAME CONTENT
            argv_ptr = make_argv(cmd_buf, 2); // Expecting 2 args after "new"
            if (argv_ptr != NULL) {
                note_result = new_note(argv_ptr[0], argv_ptr[1]);
                status_message = (note_result == NULL) ? STATUS_NOTE_EXISTS : STATUS_NOTE_CREATED;
                free_argv(argv_ptr, 2);
                argv_ptr = NULL;
            } else {
                yell(1);
            }
        }
        else if (strcmp(command_word, APPEND_CMD) == 0) {
            // append NAME CONTENT
            argv_ptr = make_argv(cmd_buf, 2);
            if (argv_ptr != NULL) {
                char *full_thunk_argv[3]; // Allocate space for command + 2 args
                full_thunk_argv[0] = (char*)APPEND_CMD; // Command itself as first arg
                full_thunk_argv[1] = argv_ptr[0];
                full_thunk_argv[2] = argv_ptr[1];
                thunk_result = new_thunk(0, 3, full_thunk_argv); // ID 0, 3 elements
                if (thunk_result != NULL) {
                    append_list(&thunk_list_head, thunk_result);
                    status_message = STATUS_THUNK_CREATED;
                } else {
                    yell(1); // Thunk creation failed
                }
                free_argv(argv_ptr, 2);
                argv_ptr = NULL;
            } else {
                yell(1);
            }
        }
        else if (strcmp(command_word, DELETE_CMD) == 0) {
            // delete NAME
            argv_ptr = make_argv(cmd_buf, 1);
            if (argv_ptr != NULL) {
                char *full_thunk_argv[2]; // Allocate space for command + 1 arg
                full_thunk_argv[0] = (char*)DELETE_CMD;
                full_thunk_argv[1] = argv_ptr[0];
                thunk_result = new_thunk(0, 2, full_thunk_argv); // ID 0, 2 elements
                if (thunk_result != NULL) {
                    append_list(&thunk_list_head, thunk_result);
                    status_message = STATUS_THUNK_CREATED;
                } else {
                    yell(1); // Thunk creation failed
                }
                free_argv(argv_ptr, 1);
                argv_ptr = NULL;
            } else {
                yell(1);
            }
        }
        else if (strcmp(command_word, READ_CMD) == 0) {
            // read NAME
            argv_ptr = make_argv(cmd_buf, 1);
            if (argv_ptr != NULL) {
                note_result = get_note(argv_ptr[0]);
                if (note_result == NULL) {
                    status_message = STATUS_NOTE_NOT_FOUND;
                } else {
                    fdprintf(1, NOTE_FORMAT, note_result->content);
                }
                free_argv(argv_ptr, 1);
                argv_ptr = NULL;
            } else {
                yell(1);
            }
        }
        else {
            yell(1); // Unknown command
        }

        // Display status message if any
        if (status_message != NULL) {
            transmit_string(1, status_message);
        }

        // Free command_word if it was allocated
        if (command_word != NULL) {
            free(command_word);
            command_word = NULL;
        }
    }

    // Cleanup: Free any remaining thunks in the list
    while (thunk_list_head != NULL) {
        remove_head_list(&thunk_list_head); // This frees the thunk data and the list node
    }
    // Cleanup: Free all notes
    for (unsigned int i = 0; i < 0x200; ++i) {
        if (note_store[i] != NULL) {
            delete_note(note_store[i]); // This frees the note struct and its contents
        }
    }

    exit(0); // Clean exit
}