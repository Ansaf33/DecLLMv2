#include <ctype.h>  // For isspace, isdigit
#include <string.h> // Not strictly needed after removing memcpy, but often useful. Can be removed.

// Define the structure for date and time based on parse_date's memcpy
// The order of members reflects the byte order copied by the original memcpy:
// hour, minute, second, month, day, year.
typedef struct {
    unsigned char hour;   // Corresponds to local_16 in original snippet
    unsigned char minute; // Corresponds to local_15
    unsigned char second; // Corresponds to local_14
    unsigned char month;  // Corresponds to local_13
    unsigned char day;    // Corresponds to local_12
    unsigned char year;   // Corresponds to local_11 (2-digit year, possibly century-adjusted)
} date_time_t;

// Function: date_compare
// This function is re-interpreted to compare two date_time_t structures.
// The original `undefined4` parameters and their byte accesses (`._X_Y_` notation)
// are likely disassembler artifacts and not standard C.
// Returns -1 if d1 < d2, 1 if d1 > d2, 0 if d1 == d2.
int date_compare(const date_time_t *d1, const date_time_t *d2) {
    if (d1->year < d2->year) return -1;
    if (d1->year > d2->year) return 1;

    if (d1->month < d2->month) return -1;
    if (d1->month > d2->month) return 1;

    if (d1->day < d2->day) return -1;
    if (d1->day > d2->day) return 1;

    if (d1->hour < d2->hour) return -1;
    if (d1->hour > d2->hour) return 1;

    if (d1->minute < d2->minute) return -1;
    if (d1->minute > d2->minute) return 1;

    if (d1->second < d2->second) return -1;
    if (d1->second > d2->second) return 1;

    return 0; // Dates and times are equal
}

// Function: parse_date
// param_1: input string (e.g., "12/31/99 23:59:59")
// param_2: pointer to a date_time_t structure to fill with parsed values.
// Returns the number of characters consumed from param_1 on success, 0 on failure.
int parse_date(char *param_1, date_time_t *param_2) {
    unsigned char month_val;
    unsigned char day_val;
    unsigned char year_val;
    unsigned char hour_val;
    unsigned char minute_val;
    unsigned char second_val;
    char *original_param_1 = param_1; // Store original pointer for return value calculation

    if (param_1 == NULL) {
        return 0;
    }

    // Skip leading whitespace
    while (isspace((int)*param_1)) {
        param_1++;
    }

    // Parse Month (MM)
    if (!isdigit((int)*param_1)) return 0;
    month_val = *param_1 - '0';
    param_1++;
    if (isdigit((int)*param_1)) {
        month_val = (month_val * 10) + (*param_1 - '0');
        param_1++;
    }
    if (*param_1 != '/') return 0;
    param_1++; // Skip '/'
    if ((month_val == 0) || (month_val > 12)) return 0;

    // Parse Day (DD)
    if (!isdigit((int)*param_1)) return 0;
    day_val = *param_1 - '0';
    param_1++;
    if (isdigit((int)*param_1)) {
        day_val = (day_val * 10) + (*param_1 - '0');
        param_1++;
    }
    if (*param_1 != '/') return 0;
    param_1++; // Skip '/'
    if ((day_val == 0) || (day_val > 31)) return 0;

    // Parse Year (YY)
    if (!isdigit((int)*param_1)) return 0;
    year_val = *param_1 - '0';
    param_1++; // param_1 now points to char after first year digit

    if (isdigit((int)*param_1)) { // Check for second digit
        year_val = (year_val * 10) + (*param_1 - '0');
        param_1++; // param_1 now points to char after second year digit
    } else {
        // If no second digit, the char param_1 is pointing to (after the first digit)
        // must be a space, as per original logic's `isspace` check.
        if (!isspace((int)*param_1)) return 0;
        // param_1 is already at the space, no need to advance it.
    }

    // Century adjustment for 2-digit year (e.g., 00-27 -> 2000-2027, 28-99 -> 1928-1999)
    if (year_val < 28) {
        year_val += 100; // Represents 20xx
    }

    // Skip any additional spaces between date and time
    while (isspace((int)*param_1)) {
        param_1++;
    }

    // Parse Hour (HH) - Time part is mandatory if parsing proceeds here
    if (!isdigit((int)*param_1)) return 0;
    hour_val = *param_1 - '0';
    param_1++;
    if (isdigit((int)*param_1)) {
        hour_val = (hour_val * 10) + (*param_1 - '0');
        param_1++;
    }
    if (*param_1 != ':') return 0;
    param_1++; // Skip ':'
    if (hour_val >= 24) return 0;

    // Parse Minute (MM)
    if (!isdigit((int)*param_1)) return 0;
    minute_val = *param_1 - '0';
    param_1++;
    if (isdigit((int)*param_1)) {
        minute_val = (minute_val * 10) + (*param_1 - '0');
        param_1++;
    }
    if (*param_1 != ':') return 0;
    param_1++; // Skip ':'
    if (minute_val >= 60) return 0;

    // Parse Second (SS)
    if (!isdigit((int)*param_1)) return 0;
    second_val = *param_1 - '0';
    param_1++;
    if (isdigit((int)*param_1)) {
        second_val = (second_val * 10) + (*param_1 - '0');
        param_1++;
    }
    if (second_val >= 60) return 0;

    // All parsing successful. Populate the struct.
    param_2->hour = hour_val;
    param_2->minute = minute_val;
    param_2->second = second_val;
    param_2->month = month_val;
    param_2->day = day_val;
    param_2->year = year_val;

    // Return the number of characters consumed from the input string.
    return (int)(param_1 - original_param_1);
}