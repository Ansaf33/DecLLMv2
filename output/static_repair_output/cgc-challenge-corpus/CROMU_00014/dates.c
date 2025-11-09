#include <stdio.h>   // For printf, sprintf
#include <stdlib.h>  // For atoi
#include <ctype.h>   // For isspace, isdigit

// Constants for datetime array indices
#define DT_MONTH 0
#define DT_DAY 1
#define DT_YEAR 2
#define DT_HOUR 3
#define DT_MINUTE 4
#define DT_SECOND 5
#define DT_DOY 6 // Day of Year (1-366)

// Constants for time calculations
#define EPOCH_YEAR 1970
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24
#define DAYS_PER_COMMON_YEAR 365
#define DAYS_PER_LEAP_YEAR 366

#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * HOURS_PER_DAY)

// Function: leap_year
// Returns 1 if year is a leap year, 0 otherwise.
int leap_year(unsigned int year) {
    return ((year % 400 == 0) || (year % 100 != 0 && year % 4 == 0));
}

// Function: doy
// Calculates day of year (1-365/366) for a given date.
// year: year, month: month (1-12), day: day of month (1-31)
int doy(unsigned int year, unsigned int month, unsigned int day) {
    // days_before_month[0] is unused, index 1-12 for months
    const int days_before_month[] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    
    int day_of_year = day + days_before_month[month];
    if (month > 2 && leap_year(year)) { // If month is after February and it's a leap year
        day_of_year += 1;
    }
    return day_of_year;
}

// Function: datetime2time_t
// Converts a datetime array (month, day, year, hour, minute, second, doy) to a time_t (seconds since EPOCH_YEAR-01-01 00:00:00).
// dt: pointer to an unsigned int array representing datetime components.
int datetime2time_t(unsigned int *dt) {
    long long total_days = 0; // Use long long to prevent overflow for total_days before multiplying by SECONDS_PER_DAY

    if (dt[DT_YEAR] < EPOCH_YEAR) {
        return -1; // Error: Year before epoch
    }

    for (unsigned int year = EPOCH_YEAR; year < dt[DT_YEAR]; ++year) {
        total_days += DAYS_PER_COMMON_YEAR + leap_year(year);
    }
    
    // Add days for the current year up to the given day
    // dt[DT_DOY] is 1-indexed, subtract 1 for 0-indexed days count
    total_days += dt[DT_DOY] - 1; 

    // Convert total days to seconds and add time of day
    return (int)((total_days * SECONDS_PER_DAY) +
                 (dt[DT_HOUR] * SECONDS_PER_HOUR) +
                 (dt[DT_MINUTE] * SECONDS_PER_MINUTE) +
                 dt[DT_SECOND]);
}

// Function: time_t2datetime
// Converts time_t (seconds since EPOCH_YEAR-01-01 00:00:00) to a datetime array.
// time_val: time_t value, dt: pointer to unsigned int array for datetime components
int time_t2datetime(unsigned int time_val, unsigned int *dt) {
    // days_in_month[0] is unused, index 1-12 for months
    const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Extract time components
    unsigned int seconds_in_day = time_val % SECONDS_PER_DAY;
    dt[DT_SECOND] = seconds_in_day % SECONDS_PER_MINUTE;
    dt[DT_MINUTE] = (seconds_in_day / SECONDS_PER_MINUTE) % MINUTES_PER_HOUR;
    dt[DT_HOUR] = seconds_in_day / SECONDS_PER_HOUR;

    // Extract date components
    unsigned int total_days_since_epoch = time_val / SECONDS_PER_DAY;
    unsigned int current_year = EPOCH_YEAR;
    unsigned int days_passed_in_prev_years = 0;

    // Determine the year
    while (days_passed_in_prev_years + DAYS_PER_COMMON_YEAR + leap_year(current_year) <= total_days_since_epoch) {
        days_passed_in_prev_years += DAYS_PER_COMMON_YEAR + leap_year(current_year);
        current_year++;
    }
    dt[DT_YEAR] = current_year;
    
    // Calculate day of year (1-indexed) for the current year
    unsigned int day_of_year_in_current_year = (total_days_since_epoch - days_passed_in_prev_years) + 1;
    dt[DT_DOY] = day_of_year_in_current_year;

    // Determine month and day
    int is_leap = leap_year(current_year);
    unsigned int month = 1;
    while (month <= 12) {
        int days_in_this_month = days_in_month[month];
        if (month == 2 && is_leap) {
            days_in_this_month++; // Adjust for February in a leap year
        }
        if (day_of_year_in_current_year <= days_in_this_month) {
            break; // Found the month
        }
        day_of_year_in_current_year -= days_in_this_month;
        month++;
    }
    dt[DT_MONTH] = month;
    dt[DT_DAY] = day_of_year_in_current_year; // Remaining days are the day of the month

    return 0;
}

// Function: str2datetime
// Parses a date/time string into a datetime array and then converts it to time_t.
// Expected format: "MM/DD/YYYY HH:MM:SS" (time part is optional, default 00:00:00)
// str: input string, dt: pointer to unsigned int array for datetime components
int str2datetime(char *str, unsigned int *dt) {
    const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    char *current_pos = str;
    
    // Skip leading spaces
    while (isspace((unsigned char)*current_pos)) {
        current_pos++;
    }

    // Parse Month
    dt[DT_MONTH] = atoi(current_pos);
    if (dt[DT_MONTH] < 1 || dt[DT_MONTH] > 12) {
        return -1; // Error: Invalid month
    }

    // Find '/'
    while (*current_pos != '/' && *current_pos != '\0') {
        current_pos++;
    }
    if (*current_pos == '\0') return -1; // Error: Missing '/'
    current_pos++;

    // Parse Day
    dt[DT_DAY] = atoi(current_pos);
    if (dt[DT_DAY] < 1) {
        return -1; // Error: Invalid day
    }

    // Find '/'
    while (*current_pos != '/' && *current_pos != '\0') {
        current_pos++;
    }
    if (*current_pos == '\0') return -1; // Error: Missing '/'
    current_pos++;

    // Parse Year
    dt[DT_YEAR] = atoi(current_pos);
    if (dt[DT_YEAR] < EPOCH_YEAR || dt[DT_YEAR] > 2050) { // Original range was 0x7b2 (1970) to 0x802 (2050)
        return -1; // Error: Year out of range
    }

    // Validate Day based on Month and Year
    int max_days = days_in_month[dt[DT_MONTH]];
    if (dt[DT_MONTH] == 2 && leap_year(dt[DT_YEAR])) {
        max_days++; // February in a leap year
    }
    if (dt[DT_DAY] > max_days) {
        return -1; // Error: Day exceeds max for month/year
    }

    // Skip to time part (past year digits and any spaces)
    while (isdigit((unsigned char)*current_pos)) { // Skip year digits
        current_pos++;
    }
    while (isspace((unsigned char)*current_pos)) { // Skip spaces after date
        current_pos++;
    }

    // Initialize time components to 0 in case they are not provided
    dt[DT_HOUR] = 0;
    dt[DT_MINUTE] = 0;
    dt[DT_SECOND] = 0;

    // Check if time part is present (hour digits or a colon)
    if (*current_pos != '\0' && (*current_pos == ':' || isdigit((unsigned char)*current_pos))) { 
        // Parse Hour
        dt[DT_HOUR] = atoi(current_pos);
        if (dt[DT_HOUR] < 0 || dt[DT_HOUR] > 23) {
            return -1; // Error: Invalid hour
        }

        // Find ':' for minutes
        while (*current_pos != ':' && *current_pos != '\0') {
            current_pos++;
        }
        if (*current_pos == '\0') { // No minute, so no second
            dt[DT_DOY] = doy(dt[DT_YEAR], dt[DT_MONTH], dt[DT_DAY]);
            return datetime2time_t(dt);
        }
        current_pos++;

        // Parse Minute
        dt[DT_MINUTE] = atoi(current_pos);
        if (dt[DT_MINUTE] < 0 || dt[DT_MINUTE] > 59) {
            return -1; // Error: Invalid minute
        }

        // Find ':' for seconds
        while (*current_pos != ':' && *current_pos != '\0') {
            current_pos++;
        }
        if (*current_pos == '\0') { // No second
            dt[DT_DOY] = doy(dt[DT_YEAR], dt[DT_MONTH], dt[DT_DAY]);
            return datetime2time_t(dt);
        }
        current_pos++;

        // Parse Second
        dt[DT_SECOND] = atoi(current_pos);
        if (dt[DT_SECOND] < 0 || dt[DT_SECOND] > 59) {
            return -1; // Error: Invalid second
        }
    }

    // Calculate Day of Year
    dt[DT_DOY] = doy(dt[DT_YEAR], dt[DT_MONTH], dt[DT_DAY]);

    // Convert to time_t
    return datetime2time_t(dt);
}

// Function: print_time_t
// Prints a time_t value as a formatted date/time string.
void print_time_t(unsigned int time_val) {
    unsigned int dt[7]; // Array to hold datetime components
    time_t2datetime(time_val, dt);
    printf("%u/%u/%u %02u:%02u:%02u", dt[DT_MONTH], dt[DT_DAY], dt[DT_YEAR],
                                     dt[DT_HOUR], dt[DT_MINUTE], dt[DT_SECOND]);
}

// Function: print_datetime
// Prints a datetime array as a formatted date/time string.
void print_datetime(unsigned int *dt) {
    printf("%u/%u/%u %02u:%02u:%02u", dt[DT_MONTH], dt[DT_DAY], dt[DT_YEAR],
                                     dt[DT_HOUR], dt[DT_MINUTE], dt[DT_SECOND]);
}

// Function: to_date_str
// Formats a date from a datetime array into a string.
// dt: pointer to datetime array, buffer: destination char array
int to_date_str(unsigned int *dt, char *buffer) {
    sprintf(buffer, "%u/%u/%u", dt[DT_MONTH], dt[DT_DAY], dt[DT_YEAR]);
    return 0;
}

// Function: to_time_str
// Formats a time from a datetime array into a string.
// dt: pointer to datetime array, buffer: destination char array
int to_time_str(unsigned int *dt, char *buffer) {
    sprintf(buffer, "%02u:%02u:%02u", dt[DT_HOUR], dt[DT_MINUTE], dt[DT_SECOND]);
    return 0;
}

// Function: diff_between_dates
// Placeholder function, original implementation returns 0.
int diff_between_dates(void) {
  return 0;
}