#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h> // For SHRT_MIN, SHRT_MAX, SCHAR_MIN, SCHAR_MAX

// Using packed structs to match the byte-level access patterns observed in the original code.
// This ensures that member offsets and sizes match the original code's assumptions
// (e.g., param_1 + 1 for month, param_1 + 3 for day, param_1 + 4 for time part).
// __attribute__((packed)) is a GCC/Clang extension for explicit packing.
typedef struct __attribute__((packed)) {
    short year;
    char month;
    char day;
} Date; // Total size 4 bytes

typedef struct __attribute__((packed)) {
    char hour;
    char minute;
} Time; // Total size 2 bytes

typedef struct __attribute__((packed)) {
    Date date; // 4 bytes
    Time time; // 2 bytes
} DateTime; // Total size 6 bytes

// Return type for functions indicating success/failure or comparison results
// 0 for failure, 1 for success.
// For comparison functions: -1 if first < second, 1 if first > second, 0 if equal.
typedef int status_t;

// Function declarations for internal consistency and ordering
status_t is_valid_date(const Date *date_ptr);
status_t is_valid_time(const Time *time_ptr);
status_t set_date(Date *out_date, char month_val, char day_val, short year_val);
status_t set_time(Time *out_time, char hour_val, char minute_val);
status_t set_datetime(DateTime *out_dt, Date in_date, Time in_time);
status_t compare_date(const Date *date1, const Date *date2);
status_t compare_time(const Time *time1, const Time *time2);
status_t compare_datetime(const DateTime *dt1, const DateTime *dt2);
status_t set_duration(DateTime *start_dt_out, Date start_date_in, Time start_time_in,
                      Date end_date_in, Time end_time_in);
status_t durations_overlap(const DateTime *dt1_start, const DateTime *dt1_end,
                         const DateTime *dt2_start, const DateTime *dt2_end);
status_t date_within(const Date *date_start, const Date *date_end, const Date *target_date);
status_t datetime_within(const DateTime *dt_start, const DateTime *dt_end, const DateTime *target_dt);
status_t strtodate(char *date_str, Date *out_date);
status_t strtotime(char *time_str, Time *out_time);
void print_date(const Date *date_ptr);
void get_date_str(char *buffer, size_t buffer_size, const Date *date_ptr);
const char * get_month(const Date *date_ptr);
void print_time(const Time *time_ptr);
void get_time_str(char *buffer, size_t buffer_size, const Time *time_ptr);


// Function: is_valid_date
status_t is_valid_date(const Date *date_ptr) {
    if (date_ptr == NULL) {
        return 0;
    }

    short year = date_ptr->year;
    char month = date_ptr->month;
    char day = date_ptr->day;

    // Initial boundary checks based on original snippet's constants
    // year < 0x7df (2015) || year > 0x7e9 (2025)
    // month < 0x01 (1) || month > 0x0c (12)
    // day < 0x01 (1)
    if (year < 2015 || year > 2025 || month < 1 || month > 12 || day < 1) {
        return 0;
    }

    int days_in_month;

    if (month == 2) { // February
        // Leap year check: divisible by 4, unless divisible by 100 but not by 400
        bool is_leap = (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
        days_in_month = is_leap ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) { // April, June, September, November
        days_in_month = 30;
    } else { // January, March, May, July, August, October, December (31-day months)
        days_in_month = 31;
    }

    if (day > days_in_month) {
        return 0;
    }

    return 1;
}

// Function: is_valid_time
status_t is_valid_time(const Time *time_ptr) {
    if (time_ptr == NULL) {
        return 0;
    }

    char hour = time_ptr->hour;
    char minute = time_ptr->minute;

    // Original: hour < 0x00 (0) || hour > 0x17 (23)
    // minute < 0x00 (0) || minute > 0x3B (59)
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        return 0;
    }
    return 1;
}

// Function: set_date
status_t set_date(Date *out_date, char month_val, char day_val, short year_val) {
    if (out_date == NULL) {
        return 0;
    }

    Date temp_date = { .year = year_val, .month = month_val, .day = day_val };

    if (is_valid_date(&temp_date)) {
        *out_date = temp_date;
        return 1;
    }
    return 0;
}

// Function: set_time
status_t set_time(Time *out_time, char hour_val, char minute_val) {
    if (out_time == NULL) {
        return 0;
    }

    Time temp_time = { .hour = hour_val, .minute = minute_val };

    if (is_valid_time(&temp_time)) {
        *out_time = temp_time;
        return 1;
    }
    return 0;
}

// Function: set_datetime
status_t set_datetime(DateTime *out_dt, Date in_date, Time in_time) {
    if (out_dt == NULL) {
        return 0;
    }

    if (is_valid_date(&in_date) && is_valid_time(&in_time)) {
        out_dt->date = in_date;
        out_dt->time = in_time;
        return 1;
    }
    return 0;
}

// Function: compare_date
status_t compare_date(const Date *date1, const Date *date2) {
    if (date1 == NULL || date2 == NULL) {
        return -1; // Indicate error or invalid input, original used 0xFFFFFFFF
    }

    if (date1->year < date2->year) return -1;
    if (date1->year > date2->year) return 1;

    if (date1->month < date2->month) return -1;
    if (date1->month > date2->month) return 1;

    if (date1->day < date2->day) return -1;
    if (date1->day > date2->day) return 1;

    return 0; // Dates are equal
}

// Function: compare_time
status_t compare_time(const Time *time1, const Time *time2) {
    if (time1 == NULL || time2 == NULL) {
        return -1; // Indicate error or invalid input, original used 0xFFFFFFFF
    }

    if (time1->hour < time2->hour) return -1;
    if (time1->hour > time2->hour) return 1;

    if (time1->minute < time2->minute) return -1;
    if (time1->minute > time2->minute) return 1;

    return 0; // Times are equal
}

// Function: compare_datetime
status_t compare_datetime(const DateTime *dt1, const DateTime *dt2) {
    if (dt1 == NULL || dt2 == NULL) {
        return -1; // Indicate error or invalid input
    }
    
    status_t date_cmp = compare_date(&dt1->date, &dt2->date);
    if (date_cmp != 0) {
        return date_cmp;
    }
    return compare_time(&dt1->time, &dt2->time);
}

// Function: set_duration
// `start_dt_out` points to the memory location for the start DateTime.
// `(DateTime*)((char*)start_dt_out + sizeof(DateTime))` points to the memory location for the end DateTime.
status_t set_duration(DateTime *start_dt_out, Date start_date_in, Time start_time_in,
                      Date end_date_in, Time end_time_in) {
    if (start_dt_out == NULL) {
        return 0;
    }

    // Check validity of start and end datetime components
    if (!is_valid_date(&start_date_in) || !is_valid_time(&start_time_in) ||
        !is_valid_date(&end_date_in) || !is_valid_time(&end_time_in)) {
        return 0;
    }

    DateTime temp_start_dt;
    DateTime temp_end_dt;

    temp_start_dt.date = start_date_in;
    temp_start_dt.time = start_time_in;
    temp_end_dt.date = end_date_in;
    temp_end_dt.time = end_time_in;

    // A duration is valid if start_datetime <= end_datetime.
    // The original code used `compare_date` and returned 0 (failure) if dates were equal,
    // implying start date must be strictly less than end date.
    // Here, we use `compare_datetime` for a more complete check, allowing same-day durations.
    if (compare_datetime(&temp_start_dt, &temp_end_dt) > 0) { // If start_dt > end_dt
        return 0; // Invalid duration
    }

    // Set the start datetime
    if (!set_datetime(start_dt_out, start_date_in, start_time_in)) {
        return 0;
    }
    // Set the end datetime. Original `param_1 + 6` implies `start_dt_out + 1` if DateTime is 6 bytes.
    if (!set_datetime((DateTime*)((char*)start_dt_out + sizeof(DateTime)), end_date_in, end_time_in)) {
        return 0;
    }
    return 1;
}

// Function: durations_overlap
// Checks if two durations (dt1_start, dt1_end) and (dt2_start, dt2_end) overlap.
status_t durations_overlap(const DateTime *dt1_start, const DateTime *dt1_end,
                         const DateTime *dt2_start, const DateTime *dt2_end) {
    // Check for invalid input pointers
    if (dt1_start == NULL || dt1_end == NULL || dt2_start == NULL || dt2_end == NULL) {
        return 0; // Cannot determine overlap with invalid durations
    }

    // Original logic:
    // (dt1_start < dt2_start && dt1_end > dt2_start) || (dt2_start < dt1_end && dt2_end > dt1_start)
    // This translates to:
    // (duration1 starts before duration2 AND duration1 ends after duration2 starts)
    // OR
    // (duration2 starts before duration1 ends AND duration2 ends after duration1 starts)

    if ((compare_datetime(dt1_start, dt2_start) == -1) && (compare_datetime(dt1_end, dt2_start) == 1)) {
        return 1;
    }
    if ((compare_datetime(dt2_start, dt1_end) == -1) && (compare_datetime(dt2_end, dt1_start) == 1)) {
        return 1;
    }
    return 0;
}

// Function: date_within
// Checks if a `target_date` is within a range [`date_start`, `date_end`] (inclusive).
status_t date_within(const Date *date_start, const Date *date_end, const Date *target_date) {
    if (date_start == NULL || date_end == NULL || target_date == NULL) {
        return 0;
    }

    status_t cmp_start = compare_date(date_start, target_date);
    status_t cmp_end = compare_date(date_end, target_date);

    // (date_start <= target_date) && (target_date <= date_end)
    if ((cmp_start <= 0) && (cmp_end >= 0)) {
        return 1;
    }
    return 0;
}

// Function: datetime_within
// Checks if a `target_dt` is within a range (`dt_start`, `dt_end`) (exclusive).
status_t datetime_within(const DateTime *dt_start, const DateTime *dt_end, const DateTime *target_dt) {
    if (dt_start == NULL || dt_end == NULL || target_dt == NULL) {
        return 0;
    }

    status_t cmp_start = compare_datetime(dt_start, target_dt);
    status_t cmp_end = compare_datetime(dt_end, target_dt);

    // (dt_start < target_dt) && (target_dt < dt_end)
    if ((cmp_start == -1) && (cmp_end == 1)) {
        return 1;
    }
    return 0;
}

// Function: strtodate
// Parses a date string in "YYYY-MM-DD" format into a Date struct.
status_t strtodate(char *date_str, Date *out_date) {
    if (date_str == NULL || out_date == NULL) {
        return 0;
    }

    // `strsep` modifies the string, so work on a copy
    char *date_str_copy = strdup(date_str);
    if (date_str_copy == NULL) {
        return 0; // Memory allocation failed
    }

    // Assuming YYYY-MM-DD format
    char *token_year = strsep(&date_str_copy, "-");
    char *token_month = strsep(&date_str_copy, "-");
    char *token_day = strsep(&date_str_copy, "-");

    if (token_year == NULL || token_month == NULL || token_day == NULL ||
        strlen(token_year) == 0 || strlen(token_month) == 0 || strlen(token_day) == 0) {
        free(date_str_copy);
        return 0;
    }

    char *endptr;
    long year_long = strtol(token_year, &endptr, 10);
    if (*endptr != '\0' || endptr == token_year) { free(date_str_copy); return 0; }
    
    long month_long = strtol(token_month, &endptr, 10);
    if (*endptr != '\0' || endptr == token_month) { free(date_str_copy); return 0; }

    long day_long = strtol(token_day, &endptr, 10);
    if (*endptr != '\0' || endptr == token_day) { free(date_str_copy); return 0; }

    // Basic range check for casting safety, `is_valid_date` will perform full validation
    if (year_long < SHRT_MIN || year_long > SHRT_MAX ||
        month_long < SCHAR_MIN || month_long > SCHAR_MAX ||
        day_long < SCHAR_MIN || day_long > SCHAR_MAX) {
        free(date_str_copy);
        return 0;
    }

    Date temp_date = {
        .year = (short)year_long,
        .month = (char)month_long,
        .day = (char)day_long
    };
    
    free(date_str_copy);

    if (is_valid_date(&temp_date)) {
        *out_date = temp_date;
        return 1;
    }
    return 0;
}

// Function: strtotime
// Parses a time string in "HH:MM" format into a Time struct.
status_t strtotime(char *time_str, Time *out_time) {
    if (time_str == NULL || out_time == NULL) {
        return 0;
    }

    char *time_str_copy = strdup(time_str);
    if (time_str_copy == NULL) {
        return 0; // Memory allocation failed
    }

    // The original snippet's `strsep` for minutes used '-', but `print_time`/`get_time_str` use ':'
    // Assuming "HH:MM" format based on the print functions.
    char *token_hour = strsep(&time_str_copy, ":");
    char *token_minute = strsep(&time_str_copy, ":"); 

    if (token_hour == NULL || token_minute == NULL ||
        strlen(token_hour) == 0 || strlen(token_minute) == 0) {
        free(time_str_copy);
        return 0;
    }

    char *endptr;
    long hour_long = strtol(token_hour, &endptr, 10);
    if (*endptr != '\0' || endptr == token_hour) { // Check for non-numeric or empty string
        free(time_str_copy);
        return 0;
    }

    long minute_long = strtol(token_minute, &endptr, 10);
    if (*endptr != '\0' || endptr == token_minute) { // Check for non-numeric or empty string
        free(time_str_copy);
        return 0;
    }

    // Basic range check for casting safety, `is_valid_time` will perform full validation
    if (hour_long < SCHAR_MIN || hour_long > SCHAR_MAX ||
        minute_long < SCHAR_MIN || minute_long > SCHAR_MAX) {
        free(time_str_copy);
        return 0;
    }

    Time temp_time = {
        .hour = (char)hour_long,
        .minute = (char)minute_long
    };

    free(time_str_copy);

    if (is_valid_time(&temp_time)) {
        *out_time = temp_time;
        return 1;
    }
    return 0;
}

// Function: print_date
// Prints date in "MM-DD-YYYY" format (matches original output order).
void print_date(const Date *date_ptr) {
    if (date_ptr == NULL) {
        printf("Invalid Date\n");
        return;
    }
    printf("%d-%d-%d", (int)date_ptr->month, (int)date_ptr->day, (int)date_ptr->year);
}

// Function: get_date_str
// Formats date into a string buffer in "MM-DD-YYYY" format.
void get_date_str(char *buffer, size_t buffer_size, const Date *date_ptr) {
    if (buffer == NULL || buffer_size == 0) {
        return; // No buffer or zero size, cannot write
    }
    if (date_ptr == NULL) {
        strncpy(buffer, "Invalid Date", buffer_size);
        buffer[buffer_size - 1] = '\0';
        return;
    }
    snprintf(buffer, buffer_size, "%d-%d-%d", (int)date_ptr->month, (int)date_ptr->day, (int)date_ptr->year);
}

// Function: get_month
// Returns the month name for a given Date.
const char * get_month(const Date *date_ptr) {
    if (date_ptr == NULL) {
        return "Undefined Month";
    }
    char month_val = date_ptr->month;
    switch(month_val) {
        case 1:  return "January";
        case 2:  return "February";
        case 3:  return "March";
        case 4:  return "April";
        case 5:  return "May";
        case 6:  return "June";
        case 7:  return "July";
        case 8:  return "August";
        case 9:  return "September";
        case 10: return "October";
        case 11: return "November";
        case 12: return "December";
        default: return "Undefined Month";
    }
}

// Function: print_time
// Prints time in "HH:MM" format with leading zero for single-digit minutes.
void print_time(const Time *time_ptr) {
    if (time_ptr == NULL) {
        printf("Invalid Time\n");
        return;
    }
    printf("%d:%02d", (int)time_ptr->hour, (int)time_ptr->minute);
}

// Function: get_time_str
// Formats time into a string buffer in "HH:MM" format with leading zero for single-digit minutes.
void get_time_str(char *buffer, size_t buffer_size, const Time *time_ptr) {
    if (buffer == NULL || buffer_size == 0) {
        return; // No buffer or zero size, cannot write
    }
    if (time_ptr == NULL) {
        strncpy(buffer, "Invalid Time", buffer_size);
        buffer[buffer_size - 1] = '\0';
        return;
    }
    snprintf(buffer, buffer_size, "%d:%02d", (int)time_ptr->hour, (int)time_ptr->minute);
}