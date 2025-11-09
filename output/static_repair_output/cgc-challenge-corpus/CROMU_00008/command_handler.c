#include <stdio.h>    // For printf, fgets, sscanf
#include <stdlib.h>   // For atoi, exit
#include <ctype.h>    // For isspace, tolower, isalnum
#include <string.h>   // For strcmp, strcspn
#include <setjmp.h>   // For setjmp, jmp_buf

// --- Type Definitions (from decompiler output) ---
typedef unsigned char byte;
typedef unsigned char undefined;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef unsigned int uint;

// --- Forward Declarations for external/missing functions ---
// (These will be dummy implementations for compilation)
extern undefined4 readLine(int param_1, char *buffer, int max_len);
extern void sanitize_string(char *str);
extern int parse_date(char *date_str, uint *packed_date, undefined2 *packed_time);
extern int db_add_record(char *username, char *firstname, char *lastname, uint packed_date, undefined2 packed_time);
extern int db_remove_record(int record_id);
extern void *db_search_index(int record_id); // Returns pointer to record, or NULL
extern int db_update_record(int record_id, char *username, char *firstname, char *lastname, uint packed_date, undefined2 packed_time);
extern uint db_get_record_count(void);
extern void print_record_helper(void *record_ptr); // Takes pointer to record
extern jmp_buf *get_next_exception_frame(void); // Returns a jmp_buf pointer
extern void pop_exception_frame(void);
extern void parse_search_expression(char *expression);
extern void _terminate(void); // Likely exit(0)

// --- Command Table Structure ---
typedef int (*command_handler_t)(char *, int *);

// Structure for a date field, based on update_record printf arguments
// (second, minute, hour, month, day, year_offset)
typedef struct {
    unsigned char sec;
    unsigned char min;
    unsigned char hour;
    unsigned char month;
    unsigned char day;
    unsigned char year_offset; // Year - 1900
} DateFields;

// Structure for a record, based on update_record field offsets
typedef struct {
    int id; // Assuming 4 bytes for an ID at the start
    char first_name[129];
    char last_name[129];
    char user_name[129];
    DateFields birthdate;
    char _padding[3]; // To make total size 400 (0x190) for some systems, or based on actual binary structure
} Record;


// The command table entry structure.
// Size 0x2c (44 bytes)
typedef struct {
    char name[36]; // Command name string
    command_handler_t handler; // Function pointer
    int flags;                 // Flags (e.g., authentication required)
} CommandEntry;

// --- Forward Declarations of functions from the snippet ---
// These are needed before the command_table definition
int stringbeg(byte *param_1, byte *param_2);
void init_user(undefined4 *param_1);
undefined4 parse_command(char *param_1, int *param_2); // Adjusted param_1 type
int command_insert(void);
int command_remove(void);
int command_update(void);
int command_print(void);
int command_find(void);
int command_login(char *param_1, int *param_2); // Adjusted param_1 type
int command_logout(char *param_1, undefined4 *param_2); // Adjusted param_1 type
int command_exit(void);

// --- Command Table Definition ---
CommandEntry command_table[] = {
    {"login", (command_handler_t)command_login, 0}, // login itself doesn't require auth to be called, but sets it.
    {"logout", (command_handler_t)command_logout, 0},
    {"insert", (command_handler_t)command_insert, 1}, // Requires authentication
    {"remove", (command_handler_t)command_remove, 1},
    {"update", (command_handler_t)command_update, 1},
    {"print", (command_handler_t)command_print, 0},
    {"find", (command_handler_t)command_find, 0},
    {"exit", (command_handler_t)command_exit, 0},
    {"", NULL, 0} // Sentinel to mark end of table
};

// Aliases for the specific offsets used in the original code
// These point to the corresponding fields of the first element of the command_table
char *g_cmdTable = command_table[0].name;
command_handler_t *DAT_00017024 = &command_table[0].handler;
int *DAT_00017028 = &command_table[0].flags;


// --- Dummy Implementations for compilation ---
undefined4 readLine(int param_1, char *buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return 0; // Error or EOF
    }
    // Remove trailing newline if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 1; // Success
}
void sanitize_string(char *str) {
    char *read_ptr = str;
    char *write_ptr = str;
    while (*read_ptr) {
        if (isalnum((unsigned char)*read_ptr) || isspace((unsigned char)*read_ptr)) {
            *write_ptr++ = *read_ptr;
        }
        read_ptr++;
    }
    *write_ptr = '\0';
}

int parse_date(char *date_str, uint *packed_date, undefined2 *packed_time) {
    int month, day, year, hour, min, sec;
    if (sscanf(date_str, "%d/%d/%d %d:%d:%d", &month, &day, &year, &hour, &min, &sec) == 6) {
        *packed_date = (uint)((sec << 24) | (min << 16) | (hour << 8) | day);
        *packed_time = (undefined2)(((year - 1900) & 0xFF) << 8 | (month & 0xFF));
        return 1; // Success
    }
    return 0; // Error
}

int db_add_record(char *username, char *firstname, char *lastname, uint packed_date, undefined2 packed_time) {
    static int record_count = 0;
    printf("[DB] Adding record for %s (FN:%s LN:%s) Date: %u Time: %u\n", username, firstname, lastname, packed_date, packed_time);
    if (record_count >= 10) return -1; // Database full dummy
    return record_count++; // Return dummy record ID
}

int db_remove_record(int record_id) {
    printf("[DB] Removing record %d\n", record_id);
    return (record_id == 1) ? 1 : 0; // Dummy: only record 1 can be removed
}

void *db_search_index(int record_id) {
    static Record dummy_record = {
        .id = 1,
        .first_name = "John",
        .last_name = "Doe",
        .user_name = "jdoe",
        .birthdate = { .sec = 30, .min = 15, .hour = 10, .month = 7, .day = 20, .year_offset = 120 } // July 20, 2020 10:15:30
    };
    printf("[DB] Searching for record %d\n", record_id);
    return (record_id == 1) ? &dummy_record : NULL; // Dummy: only record 1 exists
}

int db_update_record(int record_id, char *username, char *firstname, char *lastname, uint packed_date, undefined2 packed_time) {
    printf("[DB] Updating record %d - %s (FN:%s LN:%s) Date: %u Time: %u\n", record_id, username, firstname, lastname, packed_date, packed_time);
    return 1; // Dummy success
}

uint db_get_record_count(void) {
    return 5; // Dummy count
}

void print_record_helper(void *record_ptr) {
    Record *rec = (Record *)record_ptr;
    printf("--- Record ID: %d ---\n", rec->id);
    printf("  First Name: %s\n", rec->first_name);
    printf("  Last Name: %s\n", rec->last_name);
    printf("  User Name: %s\n", rec->user_name);
    printf("  Birthdate: %u/%u/%u %u:%u:%u\n",
           (unsigned int)rec->birthdate.month,
           (unsigned int)rec->birthdate.day,
           (unsigned int)rec->birthdate.year_offset + 1900,
           (unsigned int)rec->birthdate.hour,
           (unsigned int)rec->birthdate.min,
           (unsigned int)rec->birthdate.sec);
    printf("---------------------\n");
}

jmp_buf *get_next_exception_frame(void) {
    static jmp_buf env;
    return &env;
}
void pop_exception_frame(void) {
    // In a real system, this would manage a stack of jmp_buf's
    printf("[EXC] Pop exception frame (dummy)\n");
}
void parse_search_expression(char *expression) {
    printf("[SRCH] Parsing search expression: %s (dummy)\n", expression);
    // Example of triggering an error for demonstration:
    // if (strstr(expression, "error") != NULL) {
    //     longjmp(*get_next_exception_frame(), 2); // Syntax error
    // }
}

void _terminate(void) {
    printf("[APP] Terminating application.\n");
    exit(0);
}


// --- Original Functions, now fixed ---

// Function: stringbeg
int stringbeg(byte *param_1, byte *param_2) {
    int pos = 0;
    while (*param_1 != 0 && isspace((unsigned char)*param_1)) {
        param_1++;
        pos++;
    }
    while (*param_1 != 0 && *param_2 != 0) {
        if (tolower((unsigned char)*param_1) != tolower((unsigned char)*param_2)) {
            return -1;
        }
        param_1++;
        param_2++;
        pos++;
    }
    return pos;
}

// Function: init_user
void init_user(undefined4 *param_1) {
    *param_1 = 0;
}

// Function: parse_command
undefined4 parse_command(char *param_1, int *param_2) { // Changed param_1 type to char*
    for (int i = 0; ; ++i) {
        // Check for end of command table (handler is NULL)
        if (command_table[i].handler == NULL) {
            printf("Command not found.\n");
            return 0xffffffff;
        }

        // Compare command name
        // The original code `g_cmdTable + local_10 * 0x2c` implies `command_table[i].name`
        int match_len = stringbeg((byte *)param_1, (byte *)command_table[i].name);
        if (match_len > 0) { // Found a match
            // Check authentication requirement
            if ((command_table[i].flags & 1) != 0 && *param_2 == 0) {
                printf("Authentication required. Try login\n");
                return 0xffffffff;
            }
            // Execute command handler
            return command_table[i].handler(param_1 + match_len, param_2);
        }
    }
}

// Function: command_insert
int command_insert(void) { // Changed return type to int for consistency with command_handler_t
    char first_name[129];
    char last_name[129];
    char user_name[129];
    char birthdate_str[129];
    uint packed_date;
    undefined2 packed_time;

    printf("First name: ");
    readLine(0, first_name, sizeof(first_name));
    sanitize_string(first_name);

    printf("Last name: ");
    readLine(0, last_name, sizeof(last_name));
    sanitize_string(last_name);

    printf("User name: ");
    readLine(0, user_name, sizeof(user_name));
    sanitize_string(user_name);

    printf("Birthdate (mm/dd/yy hh:mm:ss): ");
    readLine(0, birthdate_str, sizeof(birthdate_str));

    if (parse_date(birthdate_str, &packed_date, &packed_time) == 0) {
        printf("Date parsing error.\n");
    } else {
        printf("Date is: %d/%d/%d %d:%d:%d\n",
               (unsigned int)(byte)packed_time,     // Month (LSB of packed_time)
               packed_date & 0xff,                  // Day (LSB of packed_date)
               (packed_time >> 8) + 0x76c,          // Year (MSB of packed_time + 1900)
               (packed_date >> 8) & 0xff,           // Hour
               (packed_date >> 0x10) & 0xff,        // Minute
               packed_date >> 0x18);                // Second

        int record_id = db_add_record(user_name, first_name, last_name, packed_date, packed_time);
        if (record_id == -1) {
            printf("Database full.\n");
        } else {
            printf("Data added, record %d\n", record_id);
        }
    }
    return 0;
}

// Function: command_remove
int command_remove(void) { // Changed return type to int
    char record_num_str[129];

    printf("Enter record number to remove: ");
    readLine(0, record_num_str, sizeof(record_num_str));
    int record_num = atoi(record_num_str);

    if (db_remove_record(record_num) == 0) {
        printf("Record not found.\n");
    } else {
        printf("Record %d removed.\n", record_num);
    }
    return 0;
}

// Function: command_update
int command_update(void) { // Changed return type to int
    char record_num_str[129];
    char first_name_buf[129];
    char last_name_buf[129];
    char user_name_buf[129];
    char birthdate_str[129];
    uint packed_date;
    undefined2 packed_time;

    printf("Enter record number to update: ");
    readLine(0, record_num_str, sizeof(record_num_str));
    int record_num = atoi(record_num_str);

    Record *record_ptr = (Record *)db_search_index(record_num);
    if (record_ptr == NULL) {
        printf("Record not found.\n");
    } else {
        printf("Update first name (%s): ", record_ptr->first_name);
        readLine(0, first_name_buf, sizeof(first_name_buf));
        sanitize_string(first_name_buf);

        printf("Update last name (%s): ", record_ptr->last_name);
        readLine(0, last_name_buf, sizeof(last_name_buf));
        sanitize_string(last_name_buf);

        printf("Update user name (%s): ", record_ptr->user_name);
        readLine(0, user_name_buf, sizeof(user_name_buf));
        sanitize_string(user_name_buf);

        printf("Update birthdate (%d/%d/%d %d:%d:%d): ",
               (unsigned int)record_ptr->birthdate.month,
               (unsigned int)record_ptr->birthdate.day,
               (unsigned int)record_ptr->birthdate.year_offset + 0x76c,
               (unsigned int)record_ptr->birthdate.hour,
               (unsigned int)record_ptr->birthdate.min,
               (unsigned int)record_ptr->birthdate.sec);
        readLine(0, birthdate_str, sizeof(birthdate_str));

        if (parse_date(birthdate_str, &packed_date, &packed_time) == 0) {
            printf("Date parsing error.\n");
        } else {
            printf("Date is: %d/%d/%d %d:%d:%d\n",
                   (unsigned int)(byte)packed_time,     // Month (LSB of packed_time)
                   packed_date & 0xff,                  // Day (LSB of packed_date)
                   (packed_time >> 8) + 0x76c,          // Year (MSB of packed_time + 1900)
                   (packed_date >> 8) & 0xff,           // Hour
                   (packed_date >> 0x10) & 0xff,        // Minute
                   packed_date >> 0x18);                // Second

            if (db_update_record(record_num, user_name_buf, first_name_buf, last_name_buf, packed_date, packed_time) == 0) {
                printf("Record update failed.\n");
            } else {
                printf("Record %d updated.\n", record_num);
            }
        }
    }
    return 0;
}

// Function: command_print
int command_print(void) { // Changed return type to int
    char record_selection_str[129];

    printf("Enter record number to print (or \"all\" for all records): ");
    readLine(0, record_selection_str, sizeof(record_selection_str));

    if (strcmp(record_selection_str, "all") == 0) {
        uint record_count = db_get_record_count();
        printf("Printing all %u records.\n", record_count);
        for (uint i = 0; i < record_count; ++i) {
            void *record_ptr = db_search_index(i); // Assuming db_search_index can take 0-based index
            if (record_ptr != NULL) {
                print_record_helper(record_ptr);
            }
        }
    } else {
        int record_num = atoi(record_selection_str);
        void *record_ptr = db_search_index(record_num);
        if (record_ptr == NULL) {
            printf("Record not found.\n");
        } else {
            print_record_helper(record_ptr);
        }
    }
    return 0;
}

// Function: command_find
int command_find(void) { // Changed return type to int
    if (db_get_record_count() == 0) {
        printf("Database empty. No records to find.\n");
    } else {
        char search_expression[129];
        printf(
            "Enter search express (firstname or fn, lastname or ln, username or un, birthdate or bd, operators ==, !=, >, <, AND and OR):\n"
        );
        readLine(0, search_expression, sizeof(search_expression));

        jmp_buf *env = get_next_exception_frame();
        int jmp_val = setjmp(*env); // setjmp takes jmp_buf by value

        if (jmp_val == 3) {
            printf("Parser overflow\n");
        } else if (jmp_val == 2) { // Simplified nested if-else based on distinct values
            printf("Syntax error\n");
        } else if (jmp_val == 1) {
            printf("Invalid token\n");
        } else if (jmp_val == 0) { // Only call parse_search_expression if no error
            parse_search_expression(search_expression);
        }
        pop_exception_frame();
    }
    return 0;
}

// Function: command_login
int command_login(char *param_1, int *param_2) { // Changed param_1 type to char*, return to int
    if (*param_2 == 1) {
        printf("You are already logged in.\n");
    } else {
        *param_2 = 1;
        printf("You logged in.\n");
    }
    return 0;
}

// Function: command_logout
int command_logout(char *param_1, undefined4 *param_2) { // Changed param_1 type to char*, return to int
    *param_2 = 0;
    printf("You logged out.\n");
    return 0;
}

// Function: command_exit
int command_exit(void) { // Changed return type to int
    printf("Disconnecting.\n");
    _terminate();
    return 0; // _terminate should exit, so this return is unreachable.
}

// Main function for compilation
int main() {
    printf("Welcome to the database management system.\n");
    int auth_status = 0; // 0 = logged out, 1 = logged in
    char command_line[256];

    init_user((undefined4 *)&auth_status); // Initialize auth_status (sets to 0)

    while (1) {
        printf("\n> ");
        if (readLine(0, command_line, sizeof(command_line)) == 0) {
            printf("Error reading command or EOF.\n");
            break;
        }

        // Check for empty command
        char *trimmed_command = command_line;
        while (*trimmed_command != '\0' && isspace((unsigned char)*trimmed_command)) {
            trimmed_command++;
        }
        if (*trimmed_command == '\0') {
            continue; // Skip empty commands
        }

        parse_command(trimmed_command, &auth_status);
    }

    return 0;
}