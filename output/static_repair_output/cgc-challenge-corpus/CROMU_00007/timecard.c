#include <stdbool.h> // For bool
#include <ctype.h>   // For isdigit
#include <string.h>  // For memcpy, strncpy, memset, strcmp, strrchr
#include <stdlib.h>  // For atoi, abs, strdup, free
#include <stdio.h>   // For printf, sprintf, fgets, scanf, getchar
#include <math.h>    // For roundf

// --- Global Constants (derived from DAT_xxxx in original snippet) ---
#define MINUTES_PER_HOUR 60.0f
#define ROUNDING_OFFSET 0.5f // Used for rounding in original add_money
#define CENTS_PER_DOLLAR 100.0f
#define STANDARD_WORK_HOURS_PER_WEEK 40.0f
#define TAX_DEDUCTION_MULTIPLIER 0.15f // Example value, adjust as needed
#define OVERTIME_RATE_MULTIPLIER 1.5f

// String literals (derived from DAT_xxxx in original snippet)
const char *KEY_HOURLY_RATE = "hourly_rate"; // DAT_0001406a
const char *VALUE_EXEMPT_TRUE = "true";     // DAT_00014076
const char *KEY_WEEK = "week";             // DAT_0001407a
const char *DELIMITER_NEWLINE = "`\n";     // DAT_00014108

// --- Helper Functions (replacing `print`, `itoa`, `equals`, `receive_until`, `ROUND`) ---

// `print` function: Assumed to be `printf` for strings.
void print(const char *s) {
    printf("%s", s);
}

// `itoa` function: Using `sprintf` for robust integer-to-string conversion.
void itoa(char *buffer, int value) {
    sprintf(buffer, "%d", value);
}

// `equals` function: Using `strcmp` for string comparison.
int equals(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

// `receive_until` function: Dummy implementation for standard input.
// In a real application, `fd` would be a file descriptor.
// Returns 0 on success, -1 on error. Fills buffer up to `max_len`, sets `*read_len`.
int receive_until(char *buffer, int fd, int max_len, int *read_len) {
    if (fgets(buffer, max_len, stdin) != NULL) {
        *read_len = strlen(buffer);
        // Remove trailing newline if present
        if (*read_len > 0 && buffer[*read_len - 1] == '\n') {
            buffer[*read_len - 1] = '\0';
            (*read_len)--;
        }
        return 0; // Success
    }
    *read_len = 0;
    return -1; // Error or EOF
}

// `ROUND` macro: Using `roundf` from `math.h` for float rounding.
#define ROUND(x) roundf(x)

// --- Structure Definitions ---

typedef struct {
    int hours;
    int minutes;
} TimeRecord;

typedef struct {
    int dollars;
    int cents;
} MoneyRecord;

// Forward declarations for function pointers
typedef void (*OvertimeCalcFunc)(int *pay_dollars_cents, int *hourly_rate_dollars_cents, int *overtime_time_hours_minutes);
void exempt_overtime(int *pay_dollars_cents, int *hourly_rate_dollars_cents, int *overtime_time_hours_minutes);
void nonexempt_overtime(int *pay_dollars_cents, int *hourly_rate_dollars_cents, int *overtime_time_hours_minutes);

typedef struct {
    TimeRecord standard_time; // Offset 0x50
    TimeRecord overtime_time; // Offset 0x58
    MoneyRecord standard_pay; // Offset 0x60
    MoneyRecord overtime_pay; // Offset 0x68
    // Note: original code refers to 0x70, 0x74 as undefined4 which are initialized to 0.
    // They are not used elsewhere in the provided snippets.
    char net_pay_str[12]; // Offset 0x78 (size 0xc from memset in initialize_employee and mtoa usage)
    OvertimeCalcFunc calculate_overtime; // Offset 0x84
} WeeklyData; // Total size 0x38 (56 bytes)

typedef struct Employee {
    char name[64];          // Offset 0x0, size 0x40 (from strncpy(param_1,param_3,0x40))
    int employee_id;        // Offset 0x40
    MoneyRecord hourly_rate; // Offset 0x44 (dollars), 0x48 (cents)
    int is_exempt;          // Offset 0x4c (0 for non-exempt, 1 for exempt)
    WeeklyData weekly_data[52]; // Offset 0x50 (52 weeks * 0x38 bytes/week)
} Employee; // Total size approx. 0xC00 bytes (3072 bytes)


// --- Function Declarations ---
void atom(int *param_1, char *param_2);
void mtoa(char *param_1, int *param_2);
void atoh(int *param_1, const char *param_2);
void htoa(char *param_1, int *param_2);
void initialize_employee(Employee *emp);
void add_time(TimeRecord *tr, int hours_to_add, int minutes_to_add);
void round_minutes(TimeRecord *tr);
void add_money(MoneyRecord *mr, float amount);
void add_pay(MoneyRecord *target_pay, MoneyRecord *hourly_rate, TimeRecord *time_record);
void log_hours(TimeRecord *target_standard_time, const char *time_str);
void log_overtime_hours(TimeRecord *target_overtime_time, const char *time_str);
void calculate_standardpay(MoneyRecord *standard_pay_target, MoneyRecord *hourly_rate, TimeRecord *standard_time_record);
void calculate_totalpay(Employee *emp, int week_idx);
// exempt_overtime and nonexempt_overtime are declared via typedef above
int get_key_value(char *buffer, int fd, char **key_ptr, char **value_ptr);
void process_key_value(Employee *emp, const char *key, const char *value, int *current_week_idx);
void merge_employee_records(Employee *target_emp, Employee *source_emp);
void* process_query(int query_type, Employee *employee_records, Employee *query_emp_template, int specific_week_idx);
void output_paycheck(Employee *emp, int week_idx);
long double c_standardpay(int rate_dollars, int rate_cents, int hours, int minutes);
long double c_overtimepay(int rate_dollars, int rate_cents, int hours, int minutes, int is_exempt_status);
long double c_netpay(int std_hours, int std_minutes, int ot_hours, int ot_minutes, int rate_dollars, int rate_cents, int is_exempt_status);


// --- Function Implementations ---

// Function: atom
// Converts a string to a monetary value (dollars and cents).
// param_1: int array of size 2, param_1[0] = dollars, param_1[1] = cents
// param_2: input string, e.g., "123.45" or "-123"
void atom(int *param_1, char *param_2) {
    bool is_negative = false;
    int i = 0;

    param_1[0] = 0; // Dollars
    param_1[1] = 0; // Cents

    if (*param_2 == '-') {
        is_negative = true;
        i = 1;
    }

    while (isdigit((unsigned char)param_2[i])) {
        param_1[0] = param_1[0] * 10 + (param_2[i] - '0');
        i++;
    }

    if (param_2[i] == '.') {
        i++;
        if (isdigit((unsigned char)param_2[i])) {
            param_1[1] = (param_2[i] - '0') * 10;
            i++;
            if (isdigit((unsigned char)param_2[i])) {
                param_1[1] += (param_2[i] - '0');
            }
        }
    }

    if (is_negative) {
        param_1[0] = -param_1[0];
        param_1[1] = -param_1[1];
    }
}

// Function: mtoa
// Converts a monetary value (dollars and cents) to a string.
// param_1: output buffer (char *)
// param_2: int array of size 2, param_2[0] = dollars, param_2[1] = cents
void mtoa(char *param_1, int *param_2) {
    int dollars = param_2[0];
    int cents = param_2[1];
    bool is_negative = false;

    if (dollars < 0 || cents < 0) {
        is_negative = true;
        dollars = abs(dollars);
        cents = abs(cents);
    }
    
    if (is_negative) {
        sprintf(param_1, "-$%d.%02d", dollars, cents);
    } else {
        sprintf(param_1, "$%d.%02d", dollars, cents);
    }
}

// Function: atoh
// Converts a string to a time value (hours and minutes).
// param_1: int array of size 2, param_1[0] = hours, param_1[1] = minutes
// param_2: input string, e.g., "8h30m"
void atoh(int *param_1, const char *param_2) {
    int i = 0;
    int temp_val = 0;

    param_1[0] = 0; // Hours
    param_1[1] = 0; // Minutes

    while (isdigit((unsigned char)param_2[i])) {
        temp_val = temp_val * 10 + (param_2[i] - '0');
        i++;
    }
    if (param_2[i] == 'h') {
        param_1[0] = temp_val;
        i++;
    }

    temp_val = 0;
    while (isdigit((unsigned char)param_2[i])) {
        temp_val = temp_val * 10 + (param_2[i] - '0');
        i++;
    }
    if (param_2[i] == 'm') {
        param_1[1] = temp_val;
    }

    if (param_1[0] > 24) {
        param_1[0] = 24;
    }
    if (param_1[1] > 59) {
        param_1[1] = 59;
    }
}

// Function: htoa
// Converts a time value (hours and minutes) to a string.
// param_1: output buffer (char *)
// param_2: int array of size 2, param_2[0] = hours, param_2[1] = minutes
void htoa(char *param_1, int *param_2) {
    int hours = param_2[0];
    int minutes = param_2[1];
    
    if (minutes >= (int)MINUTES_PER_HOUR) {
        *param_1 = '\0'; // Return empty string if minutes are invalid
        return;
    }

    sprintf(param_1, "%dh%02dm", hours, minutes);
}

// Function: initialize_employee
void initialize_employee(Employee *emp) {
    memset(emp, 0, sizeof(Employee));

    emp->employee_id = -1;
    emp->hourly_rate.dollars = 0;
    emp->hourly_rate.cents = 0;
    emp->is_exempt = 0; // Default to non-exempt

    for (int i = 0; i < 52; i++) {
        // Default to exempt_overtime, will be updated to nonexempt_overtime if is_exempt is 0
        emp->weekly_data[i].calculate_overtime = exempt_overtime; 
    }
}

// Function: add_time
// Adds hours and minutes to a TimeRecord.
void add_time(TimeRecord *tr, int hours_to_add, int minutes_to_add) {
    tr->minutes += minutes_to_add;
    tr->hours += hours_to_add + (tr->minutes / (int)MINUTES_PER_HOUR);
    tr->minutes %= (int)MINUTES_PER_HOUR;
}

// Function: round_minutes
// Rounds minutes in a TimeRecord to the nearest 15-minute increment.
void round_minutes(TimeRecord *tr) {
    if (tr->minutes < 8) {
        tr->minutes = 0;
    } else if (tr->minutes < 23) { // 0x17 = 23
        tr->minutes = 15; // 0xf
    } else if (tr->minutes < 38) { // 0x26 = 38
        tr->minutes = 30; // 0x1e
    } else if (tr->minutes < 53) { // 0x35 = 53
        tr->minutes = 45; // 0x2d
    } else {
        tr->hours++;
        tr->minutes = 0;
    }
}

// Function: add_money
// Adds a float amount to a MoneyRecord (dollars and cents).
// The float amount represents total dollars (e.g., 123.45).
void add_money(MoneyRecord *mr, float amount) {
    float current_total = (float)mr->dollars + (float)mr->cents / CENTS_PER_DOLLAR;
    float new_total = current_total + amount;

    // The original code `local_8 = param_2 - (float)DAT_00014118;` or `local_8 = (float)DAT_00014118 + param_2;`
    // and then `ROUND(local_8)` effectively adds 0.5 for positive numbers and subtracts 0.5 for negative numbers before rounding.
    // This is a common way to implement round-half-up or round-half-away-from-zero.
    float rounded_total = (new_total >= 0.0f) ? ROUND(new_total + ROUNDING_OFFSET) : ROUND(new_total - ROUNDING_OFFSET);

    mr->dollars = (int)rounded_total;
    mr->cents = (int)ROUND((rounded_total - (float)mr->dollars) * CENTS_PER_DOLLAR);
    
    // Ensure cents have the same sign as dollars for consistency
    if (mr->dollars < 0 && mr->cents > 0) {
        mr->cents = -mr->cents;
    } else if (mr->dollars > 0 && mr->cents < 0) {
        mr->cents = abs(mr->cents);
    }
}

// Function: add_pay
// Calculates pay based on hourly rate and time, then adds it to a target MoneyRecord.
void add_pay(MoneyRecord *target_pay, MoneyRecord *hourly_rate, TimeRecord *time_record) {
    float rate_per_hour = (float)hourly_rate->dollars + (float)hourly_rate->cents / CENTS_PER_DOLLAR;
    float hours_worked = (float)time_record->hours + (float)time_record->minutes / MINUTES_PER_HOUR;
    
    float calculated_pay_amount = rate_per_hour * hours_worked;
    
    add_money(target_pay, calculated_pay_amount);
}

// Function: log_hours
// Logs standard hours for a given day.
void log_hours(TimeRecord *target_standard_time, const char *time_str) {
    TimeRecord parsed_time;
    atoh(&parsed_time.hours, time_str);

    round_minutes(&parsed_time);

    if (parsed_time.hours < 8) {
        add_time(target_standard_time, parsed_time.hours, parsed_time.minutes);
    } else {
        add_time(target_standard_time, 8, 0); // 8 standard hours
        // The next TimeRecord in memory is overtime_time
        add_time(target_standard_time + 1, parsed_time.hours - 8, parsed_time.minutes);
    }
}

// Function: log_overtime_hours
// Logs overtime hours for a given day.
void log_overtime_hours(TimeRecord *target_overtime_time, const char *time_str) {
    TimeRecord parsed_time;
    atoh(&parsed_time.hours, time_str);

    round_minutes(&parsed_time);
    add_time(target_overtime_time, parsed_time.hours, parsed_time.minutes);
}

// Function: calculate_standardpay
void calculate_standardpay(MoneyRecord *standard_pay_target, MoneyRecord *hourly_rate, TimeRecord *standard_time_record) {
    standard_pay_target->dollars = 0;
    standard_pay_target->cents = 0;
    add_pay(standard_pay_target, hourly_rate, standard_time_record);
}

// Function: calculate_totalpay
void calculate_totalpay(Employee *emp, int week_idx) {
    MoneyRecord net_pay_temp = {0, 0};
    
    add_money(&net_pay_temp, (float)emp->weekly_data[week_idx].standard_pay.dollars + 
                               (float)emp->weekly_data[week_idx].standard_pay.cents / CENTS_PER_DOLLAR);
    
    add_money(&net_pay_temp, (float)emp->weekly_data[week_idx].overtime_pay.dollars + 
                               (float)emp->weekly_data[week_idx].overtime_pay.cents / CENTS_PER_DOLLAR);
    
    float current_total = (float)net_pay_temp.dollars + (float)net_pay_temp.cents / CENTS_PER_DOLLAR;
    add_money(&net_pay_temp, -(current_total * TAX_DEDUCTION_MULTIPLIER));

    mtoa(emp->weekly_data[week_idx].net_pay_str, &net_pay_temp.dollars);
}

// Function: exempt_overtime
// Calculates overtime pay for exempt employees (paid at standard rate).
// param_1: int array for pay_dollars_cents (dollars at index 0, cents at index 1)
// param_2: int array for hourly_rate_dollars_cents
// param_3: int array for overtime_time_hours_minutes
void exempt_overtime(int *pay_dollars_cents, int *hourly_rate_dollars_cents, int *overtime_time_hours_minutes) {
    MoneyRecord pay_target = {0,0};
    MoneyRecord hourly_rate = {hourly_rate_dollars_cents[0], hourly_rate_dollars_cents[1]};
    TimeRecord overtime_time = {overtime_time_hours_minutes[0], overtime_time_hours_minutes[1]};
    
    add_pay(&pay_target, &hourly_rate, &overtime_time);
    
    pay_dollars_cents[0] = pay_target.dollars;
    pay_dollars_cents[1] = pay_target.cents;
}

// Function: nonexempt_overtime
// Calculates overtime pay for non-exempt employees (paid at 1.5x standard rate).
// param_1: int array for pay_dollars_cents
// param_2: int array for hourly_rate_dollars_cents
// param_3: int array for overtime_time_hours_minutes
void nonexempt_overtime(int *pay_dollars_cents, int *hourly_rate_dollars_cents, int *overtime_time_hours_minutes) {
    MoneyRecord pay_target = {0,0};
    MoneyRecord hourly_rate = {hourly_rate_dollars_cents[0], hourly_rate_dollars_cents[1]};
    TimeRecord overtime_time = {overtime_time_hours_minutes[0], overtime_time_hours_minutes[1]};
    
    float rate_per_hour = (float)hourly_rate.dollars + (float)hourly_rate.cents / CENTS_PER_DOLLAR;
    float hours_worked = (float)overtime_time.hours + (float)overtime_time.minutes / MINUTES_PER_HOUR;
    
    float calculated_pay_amount = rate_per_hour * hours_worked * OVERTIME_RATE_MULTIPLIER;
    
    add_money(&pay_target, calculated_pay_amount);

    pay_dollars_cents[0] = pay_target.dollars;
    pay_dollars_cents[1] = pay_target.cents;
}

// Function: get_key_value
// Reads a line, parses it into a key and value, handles backticks.
// param_1: buffer to store the entire line (max 80 chars including null terminator)
// param_2: file descriptor for input (unused in this dummy implementation)
// param_3: char** to store pointer to the key within param_1
// param_4: char** to store pointer to the value within param_1
// Returns:
//   -1: Read error or no data
//    1: Empty line (or just a newline)
//    2: Successfully parsed key-value pair
//    3: Malformed input (e.g., no backtick, or no '-' separator in a backticked line)
int get_key_value(char *buffer, int fd, char **key_ptr, char **value_ptr) {
    char local_buffer[96]; // Original 0x60 = 96 bytes
    int read_len = 0;

    if (receive_until(local_buffer, fd, sizeof(local_buffer), &read_len) != 0) {
        return -1; // Read error or EOF
    }

    if (read_len == 0 || local_buffer[0] == '\0') {
        return 1; // Empty line
    }

    strncpy(buffer, local_buffer, 79); // Copy to output buffer (max 80 chars)
    buffer[79] = '\0'; // Ensure null termination

    char *backtick_end = strrchr(buffer, '`');
    if (backtick_end != NULL && backtick_end > buffer) {
        *backtick_end = '\0'; // Null-terminate the string at the backtick

        char *dash_separator = strrchr(buffer, '-');
        if (dash_separator != NULL && dash_separator > buffer) {
            *dash_separator = '\0'; // Null-terminate the key part
            *key_ptr = buffer;
            *value_ptr = dash_separator + 1;
            return 2; // Successfully parsed key-value
        }
    }
    
    return 3; // Malformed input
}

// Function: process_key_value
void process_key_value(Employee *emp, const char *key, const char *value, int *current_week_idx) {
    if (equals(key, "employee_id")) {
        emp->employee_id = atoi(value);
    }
    else if (equals(key, "employee_name")) {
        strncpy(emp->name, value, sizeof(emp->name) - 1);
        emp->name[sizeof(emp->name) - 1] = '\0';
    }
    else if (equals(key, KEY_HOURLY_RATE)) {
        atom(&emp->hourly_rate.dollars, (char*)value);
        if (emp->hourly_rate.dollars > 74) { // Original: 0x4a < dollars
            emp->hourly_rate.dollars = 75;  // Original: dollars = 0x4b
            emp->hourly_rate.cents = 0;
        }
    }
    else if (equals(key, "exempt")) {
        emp->is_exempt = equals(value, VALUE_EXEMPT_TRUE) ? 1 : 0;
    }
    else if (equals(key, KEY_WEEK)) {
        int week = atoi(value);
        if (week < 0 || week >= 52) { // Original: 0x33 = 51 for max index
            *current_week_idx = 0;
        } else {
            *current_week_idx = week;
        }
    }
    else if (equals(key, "monday") || equals(key, "tuesday") ||
             equals(key, "wednesday") || equals(key, "thursday") ||
             equals(key, "friday")) {
        log_hours(&emp->weekly_data[*current_week_idx].standard_time, value);
    }
    else if (equals(key, "saturday") || equals(key, "sunday")) {
        log_overtime_hours(&emp->weekly_data[*current_week_idx].overtime_time, value);
    }
    // Other keys are ignored
}

// Function: merge_employee_records
void merge_employee_records(Employee *target_emp, Employee *source_emp) {
    if (target_emp->employee_id == -1 && source_emp->name[0] != '\0') {
        strncpy(target_emp->name, source_emp->name, sizeof(target_emp->name) - 1);
        target_emp->name[sizeof(target_emp->name) - 1] = '\0';
        target_emp->employee_id = source_emp->employee_id;
        target_emp->hourly_rate = source_emp->hourly_rate;
        target_emp->is_exempt = source_emp->is_exempt;
    }
    else if (target_emp->employee_id != -1) {
        for (int i = 0; i < 52; i++) {
            add_time(&target_emp->weekly_data[i].standard_time,
                     source_emp->weekly_data[i].standard_time.hours,
                     source_emp->weekly_data[i].standard_time.minutes);
            add_time(&target_emp->weekly_data[i].overtime_time,
                     source_emp->weekly_data[i].overtime_time.hours,
                     source_emp->weekly_data[i].overtime_time.minutes);
            
            if (target_emp->is_exempt == 0) {
                target_emp->weekly_data[i].calculate_overtime = nonexempt_overtime;
            } else {
                target_emp->weekly_data[i].calculate_overtime = exempt_overtime;
            }

            calculate_standardpay(&target_emp->weekly_data[i].standard_pay,
                                  &target_emp->hourly_rate,
                                  &target_emp->weekly_data[i].standard_time);
            
            target_emp->weekly_data[i].calculate_overtime(
                &target_emp->weekly_data[i].overtime_pay.dollars,
                &target_emp->hourly_rate.dollars,
                &target_emp->weekly_data[i].overtime_time.hours
            );
        }
    }
}

// Function: process_query
// param_1: query_type (1: all weeks for specific emp, 2: all weeks for all emp, 3: specific week for specific emp, 4: specific week for all emp)
// param_2: array of Employee records (Employee employee_records[MAX_EMPLOYEES])
// param_3: template Employee record (for specific employee_id)
// param_4: specific_week_idx
void* process_query(int query_type, Employee *employee_records, Employee *query_emp_template, int specific_week_idx) {
    // The original code returned `undefined **` and initialized `ppuVar1 = &_GLOBAL_OFFSET_TABLE_`.
    // This is a decompilation artifact. As the function's purpose is to output, we return NULL.
    const int MAX_EMPLOYEES = 50; // Original 0x32 = 50

    switch (query_type) {
        case 1: // Query: All weeks for a specific employee
            if (query_emp_template->employee_id != -1 &&
                query_emp_template->employee_id < MAX_EMPLOYEES &&
                employee_records[query_emp_template->employee_id].employee_id != -1) {
                for (int week_idx = 0; week_idx < 52; week_idx++) {
                    output_paycheck(&employee_records[query_emp_template->employee_id], week_idx);
                }
            } else {
                fprintf(stderr, "Error: Employee not found for query type 1.\n");
            }
            break;
        case 2: // Query: All weeks for all employees
            for (int emp_idx = 0; emp_idx < MAX_EMPLOYEES; emp_idx++) {
                if (employee_records[emp_idx].employee_id != -1) {
                    for (int week_idx = 0; week_idx < 52; week_idx++) {
                        output_paycheck(&employee_records[emp_idx], week_idx);
                    }
                }
            }
            break;
        case 3: // Query: Specific week for a specific employee
            if (query_emp_template->employee_id != -1 &&
                query_emp_template->employee_id < MAX_EMPLOYEES &&
                employee_records[query_emp_template->employee_id].employee_id != -1 &&
                specific_week_idx >= 0 && specific_week_idx < 52) {
                output_paycheck(&employee_records[query_emp_template->employee_id], specific_week_idx);
            } else {
                fprintf(stderr, "Error: Employee or week not found for query type 3.\n");
            }
            break;
        case 4: // Query: Specific week for all employees
            if (specific_week_idx >= 0 && specific_week_idx < 52) {
                for (int emp_idx = 0; emp_idx < MAX_EMPLOYEES; emp_idx++) {
                    if (employee_records[emp_idx].employee_id != -1) {
                        output_paycheck(&employee_records[emp_idx], specific_week_idx);
                    }
                }
            } else {
                fprintf(stderr, "Error: Invalid week index for query type 4.\n");
            }
            break;
        default:
            fprintf(stderr, "Error: Unknown query type %d.\n", query_type);
            break;
    }
    return NULL;
}

// Function: output_paycheck
void output_paycheck(Employee *emp, int week_idx) {
    char buffer[64];

    print("employee-");
    print(emp->name);
    print(DELIMITER_NEWLINE); // Original: `print(&DAT_00014108);` after employee name
    
    print("`week-");
    itoa(buffer, week_idx);
    print(buffer);
    print("`standardtime-");
    htoa(buffer, &emp->weekly_data[week_idx].standard_time.hours);
    print(buffer);
    print("`overtime-");
    htoa(buffer, &emp->weekly_data[week_idx].overtime_time.hours);
    print(buffer);
    print("`standardpay-");
    mtoa(buffer, &emp->weekly_data[week_idx].standard_pay.dollars);
    print(buffer);
    print("`overtimepay-");
    mtoa(buffer, &emp->weekly_data[week_idx].overtime_pay.dollars);
    print(buffer);
    print("`netpay-");
    calculate_totalpay(emp, week_idx);
    print(emp->weekly_data[week_idx].net_pay_str);
    print(DELIMITER_NEWLINE);
}

// Function: c_standardpay (Calculates standard pay as a long double, likely for external verification)
long double c_standardpay(int rate_dollars, int rate_cents, int hours, int minutes) {
    MoneyRecord total_pay = {0, 0};
    MoneyRecord hourly_rate = {rate_dollars, rate_cents};
    TimeRecord time_worked = {hours, minutes};

    add_pay(&total_pay, &hourly_rate, &time_worked);

    return (long double)((float)total_pay.dollars + (float)total_pay.cents / CENTS_PER_DOLLAR);
}

// Function: c_overtimepay (Calculates overtime pay as a long double)
long double c_overtimepay(int rate_dollars, int rate_cents, int hours, int minutes, int is_exempt_status) {
    MoneyRecord total_pay = {0, 0};
    MoneyRecord hourly_rate = {rate_dollars, rate_cents};
    TimeRecord overtime_time = {hours, minutes};

    if (is_exempt_status == 0) { // Non-exempt
        nonexempt_overtime(&total_pay.dollars, &hourly_rate.dollars, &overtime_time.hours);
    } else { // Exempt
        exempt_overtime(&total_pay.dollars, &hourly_rate.dollars, &overtime_time.hours);
    }
    
    return (long double)((float)total_pay.dollars + (float)total_pay.cents / CENTS_PER_DOLLAR);
}

// Function: c_netpay (Calculates net pay as a long double)
long double c_netpay(int std_hours, int std_minutes, int ot_hours, int ot_minutes, int rate_dollars, int rate_cents, int is_exempt_status) {
    MoneyRecord std_pay_calc = {0,0};
    MoneyRecord ot_pay_calc = {0,0};
    MoneyRecord hourly_rate = {rate_dollars, rate_cents};
    TimeRecord standard_time = {std_hours, std_minutes};
    TimeRecord overtime_time = {ot_hours, ot_minutes};

    calculate_standardpay(&std_pay_calc, &hourly_rate, &standard_time);
    
    if (is_exempt_status == 0) {
        nonexempt_overtime(&ot_pay_calc.dollars, &hourly_rate.dollars, &overtime_time.hours);
    } else {
        exempt_overtime(&ot_pay_calc.dollars, &hourly_rate.dollars, &overtime_time.hours);
    }

    MoneyRecord final_net_pay = {0,0};
    add_money(&final_net_pay, (float)std_pay_calc.dollars + (float)std_pay_calc.cents / CENTS_PER_DOLLAR);
    add_money(&final_net_pay, (float)ot_pay_calc.dollars + (float)ot_pay_calc.cents / CENTS_PER_DOLLAR);
    
    float current_total = (float)final_net_pay.dollars + (float)final_net_pay.cents / CENTS_PER_DOLLAR;
    add_money(&final_net_pay, -(current_total * TAX_DEDUCTION_MULTIPLIER));

    return (long double)((float)final_net_pay.dollars + (float)final_net_pay.cents / CENTS_PER_DOLLAR);
}

// Main function to demonstrate usage
int main() {
    const int MAX_EMPLOYEES = 50;
    Employee employees[MAX_EMPLOYEES];
    Employee temp_employee;

    for (int i = 0; i < MAX_EMPLOYEES; i++) {
        initialize_employee(&employees[i]);
    }
    initialize_employee(&temp_employee);

    printf("Welcome to the Payroll System!\n");
    printf("Enter employee data (e.g., 'employee_name-John Doe`' or 'hourly_rate-15.50`'):\n");
    printf("Commands: 'process', 'query', 'exit'\n");

    char line_buffer[80];
    char *key = NULL;
    char *value = NULL;
    int current_week = 0;

    while (true) {
        printf("> ");
        if (fgets(line_buffer, sizeof(line_buffer), stdin) == NULL) {
            break; // EOF or error
        }
        size_t len = strlen(line_buffer);
        if (len > 0 && line_buffer[len - 1] == '\n') {
            line_buffer[len - 1] = '\0';
        }

        if (strcmp(line_buffer, "exit") == 0) {
            break;
        }
        
        if (strcmp(line_buffer, "process") == 0) {
            printf("Processing records...\n");
            if (temp_employee.employee_id != -1) {
                int emp_idx = temp_employee.employee_id % MAX_EMPLOYEES;
                if (employees[emp_idx].employee_id == -1 || employees[emp_idx].employee_id == temp_employee.employee_id) {
                    merge_employee_records(&employees[emp_idx], &temp_employee);
                    printf("Employee '%s' (ID: %d) processed.\n", employees[emp_idx].name, employees[emp_idx].employee_id);
                } else {
                    fprintf(stderr, "Error: Employee ID %d slot already taken by employee %d.\n", temp_employee.employee_id, employees[emp_idx].employee_id);
                }
            } else {
                fprintf(stderr, "Error: No employee ID in temporary record to process.\n");
            }
            initialize_employee(&temp_employee); // Reset temp for next input
            current_week = 0;
            continue;
        }

        if (strcmp(line_buffer, "query") == 0) {
            printf("Enter query type (1-4) and optional week index:\n");
            printf("  1: All weeks for specific employee (ID from temp_employee)\n");
            printf("  2: All weeks for all employees\n");
            printf("  3: Specific week for specific employee (ID from temp_employee, week index from next input)\n");
            printf("  4: Specific week for all employees (week index from next input)\n");
            printf("Query format: <type> [week_idx] (e.g., '3 10')\n");
            
            int query_type_val;
            int query_week_val = -1; // Default to -1 indicating no specific week for types that don't need it

            // Read query type and optional week index
            char query_input_buffer[32];
            if (fgets(query_input_buffer, sizeof(query_input_buffer), stdin) == NULL) {
                fprintf(stderr, "Error: Failed to read query input.\n");
                continue;
            }
            
            int scanned_items = sscanf(query_input_buffer, "%d %d", &query_type_val, &query_week_val);
            if (scanned_items < 1) {
                fprintf(stderr, "Invalid query input format.\n");
                continue;
            }

            printf("Executing query...\n");
            // Pass temp_employee (which might contain a specific employee_id from previous input)
            process_query(query_type_val, employees, &temp_employee, query_week_val);
            initialize_employee(&temp_employee); // Reset temp for next input
            current_week = 0;
            continue;
        }

        char *line_copy = strdup(line_buffer); // Duplicate to allow modification by get_key_value
        if (line_copy == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            break;
        }
        
        int parse_status = get_key_value(line_copy, 0, &key, &value);

        if (parse_status == 2) {
            process_key_value(&temp_employee, key, value, &current_week);
        } else if (parse_status == 1) {
            // Empty line, ignore
        } else {
            fprintf(stderr, "Malformed input: '%s'\n", line_buffer);
        }
        free(line_copy);
    }

    printf("Exiting Payroll System.\n");

    return 0;
}