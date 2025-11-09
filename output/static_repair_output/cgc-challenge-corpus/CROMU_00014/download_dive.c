#include <stdio.h>   // For printf, stdin, fgets
#include <stdlib.h>  // For calloc, free, atoi, exit, EXIT_FAILURE
#include <string.h>  // For strncpy, strlen, strcspn
#include <time.h>    // For time_t, localtime, strftime, struct tm
#include <unistd.h>  // For ssize_t (though not explicitly used with fgets, good practice)
#include <stdbool.h> // For bool type and true/false

// --- External Function Declarations ---
// This function is assumed to be defined elsewhere and provides byte data.
// It returns an integer value, or a negative value on error.
extern int receive_bytes();

// --- Helper Functions for Time Conversion ---
// These functions are recreated based on their likely usage in the original snippet.

// time_t2datetime: Converts a Unix timestamp to a formatted datetime string.
// buffer: Output buffer for the formatted string (e.g., "YYYY-MM-DD HH:MM:SS").
// timestamp: The Unix timestamp (seconds since epoch).
void time_t2datetime(char *buffer, int timestamp) {
    time_t t = (time_t)timestamp;
    struct tm *tm_info = localtime(&t);
    if (tm_info) {
        strftime(buffer, 28, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        buffer[0] = '\0'; // Ensure buffer is empty on error
    }
}

// to_date_str: Extracts the date part (YYYY-MM-DD) from a formatted datetime string.
// dest: Destination buffer for the date string.
// src_datetime_str: Source datetime string (e.g., "YYYY-MM-DD HH:MM:SS").
void to_date_str(char *dest, const char *src_datetime_str) {
    if (strlen(src_datetime_str) >= 10) { // YYYY-MM-DD has 10 characters
        strncpy(dest, src_datetime_str, 10);
        dest[10] = '\0'; // Null-terminate the destination string
    } else {
        dest[0] = '\0'; // Ensure destination is empty if source is too short
    }
}

// to_time_str: Extracts the time part (HH:MM:SS) from a formatted datetime string.
// dest: Destination buffer for the time string.
// src_datetime_str: Source datetime string (e.g., "YYYY-MM-DD HH:MM:SS").
void to_time_str(char *dest, const char *src_datetime_str) {
    // HH:MM:SS starts at offset 11 and has 8 characters in "YYYY-MM-DD HH:MM:SS"
    if (strlen(src_datetime_str) >= 19) { // Ensure string is long enough
        strncpy(dest, src_datetime_str + 11, 8);
        dest[8] = '\0'; // Null-terminate the destination string
    } else {
        dest[0] = '\0'; // Ensure destination is empty if source is too short
    }
}

// --- Data Structure Definitions ---
// These structures are derived from the memory access patterns in the original snippet.

// Represents a single data point (sample) during a dive.
typedef struct DiveSample {
    int timestamp;            // Unix timestamp of the sample (Offset 0x00)
    int depth;                // Depth at this timestamp (Offset 0x04)
    struct DiveSample *next_sample; // Pointer to the next sample in the dive (Offset 0x08)
} DiveSample; // Total size: 12 bytes (0xc)

// Represents a single dive record, containing details and a list of samples.
typedef struct DiveRecord {
    char site_name[27];      // Name of the dive site (Offset 0x00, max 26 chars + null)
    char date_str[11];       // Formatted date string (YYYY-MM-DD) (Offset 0x1a, max 10 chars + null)
    char time_str[9];        // Formatted time string (HH:MM:SS) (Offset 0x25, max 8 chars + null)
    int dive_start_timestamp; // Unix timestamp when the dive started (Offset 0x30)
    int max_depth;           // Maximum depth reached during the dive (Offset 0x34)
    int avg_depth;           // Average depth during the dive (Offset 0x38)
    int duration_mins;       // Duration of the dive in minutes (Offset 0x3c)
    int pressure_in;         // Initial tank pressure (psi) (Offset 0x40)
    int pressure_out;        // Final tank pressure (psi) (Offset 0x44)
    int o2_percent;          // Oxygen percentage in tank (Offset 0x48)
    // There might be padding or other unused fields between 0x4C and 0x58
    int total_samples_recorded; // Total number of samples recorded for this dive (Offset 0x58)
    char location[27];       // General location/city of the dive site (Offset 0x5c, max 26 chars + null)
    DiveSample *first_sample; // Pointer to the first sample data point (Offset 0x78)
    struct DiveRecord *next_dive; // Pointer to the next dive record in the log (Offset 0x7c)
} DiveRecord; // Total size: 128 bytes (0x80)

// Represents the overall dive log, holding a list of DiveRecords.
typedef struct DiveLog {
    // Other fields for the DiveLog can be added here if needed
    DiveRecord *first_dive; // Pointer to the first dive record in the log (Offset 0x9c)
} DiveLog;

// --- Main Function: download_dive ---
// This function processes received dive data, calculates statistics,
// and prompts the user for additional dive details.
// log_ptr: A pointer to the DiveLog structure where new dive records will be added.
// Returns 0 on success, -1 on failure.
int download_dive(DiveLog *log_ptr) {
    char datetime_buffer[28]; // Buffer for formatted date/time strings
    char input_buffer[1024];  // General-purpose buffer for user input

    int dive_start_timestamp_received = 0;
    int initial_depth_received = 0;
    int current_sample_timestamp_received = 0;

    int ret_val;

    DiveRecord *current_dive_record = NULL;
    DiveSample *current_dive_sample = NULL;
    DiveSample *new_dive_sample = NULL;

    // 1. Receive initial timestamp for the dive
    ret_val = receive_bytes();
    if (ret_val < 0) {
        printf("Error: Received error code for initial timestamp.\n");
        return -1;
    }
    dive_start_timestamp_received = ret_val;

    // 2. Receive initial depth for the dive
    ret_val = receive_bytes();
    if (ret_val < 0) {
        printf("Error: Received error code or zero sample for initial depth.\n");
        return -1;
    }
    initial_depth_received = ret_val;

    // 3. Allocate a new DiveRecord structure
    DiveRecord *new_dive_record = (DiveRecord *)calloc(1, sizeof(DiveRecord));
    if (new_dive_record == NULL) {
        perror("Error: Failed to allocate DiveRecord");
        exit(EXIT_FAILURE); // Terminate on critical memory allocation failure
    }

    // 4. Link the new DiveRecord into the DiveLog's linked list
    if (log_ptr->first_dive == NULL) {
        log_ptr->first_dive = new_dive_record;
    } else {
        current_dive_record = log_ptr->first_dive;
        while (current_dive_record->next_dive != NULL) {
            current_dive_record = current_dive_record->next_dive;
        }
        current_dive_record->next_dive = new_dive_record;
    }
    current_dive_record = new_dive_record; // Now, current_dive_record points to the newly added record

    // 5. Allocate the first DiveSample for this dive
    current_dive_sample = (DiveSample *)calloc(1, sizeof(DiveSample));
    if (current_dive_sample == NULL) {
        perror("Error: Failed to allocate DiveSample");
        exit(EXIT_FAILURE); // Terminate on critical memory allocation failure
    }
    current_dive_record->first_sample = current_dive_sample;

    // 6. Populate the first DiveSample data
    current_dive_sample->timestamp = dive_start_timestamp_received;
    current_dive_sample->depth = initial_depth_received;

    // 7. Convert the initial timestamp to a formatted datetime string and extract date/time parts
    time_t2datetime(datetime_buffer, dive_start_timestamp_received);
    current_dive_record->dive_start_timestamp = dive_start_timestamp_received;
    to_date_str(current_dive_record->date_str, datetime_buffer);
    to_time_str(current_dive_record->time_str, datetime_buffer);

    // 8. Loop to receive subsequent dive samples
    while (true) {
        // Receive timestamp for the next sample
        ret_val = receive_bytes();
        if (ret_val < 0) {
            printf("Error: Received error code for a sample timestamp.\n");
            return -1; // Error in receiving timestamp
        }
        current_sample_timestamp_received = ret_val;

        if (current_sample_timestamp_received == 0) { // Sentinel value (0 timestamp) to stop receiving samples
            current_dive_sample->next_sample = NULL; // Terminate the linked list of samples
            break; // Exit the sample reception loop
        }

        // Receive depth for the current sample
        ret_val = receive_bytes();
        if (ret_val < 0) {
            printf("Error: Received error code for a sample depth.\n");
            return -1; // Error in receiving depth
        }
        initial_depth_received = ret_val; // Reusing this variable for the current sample's depth

        // Allocate a new DiveSample and link it into the list
        new_dive_sample = (DiveSample *)calloc(1, sizeof(DiveSample));
        if (new_dive_sample == NULL) {
            perror("Error: Failed to allocate DiveSample");
            exit(EXIT_FAILURE); // Terminate on critical memory allocation failure
        }
        current_dive_sample->next_sample = new_dive_sample;
        current_dive_sample = new_dive_sample; // Move to the newly added sample

        // Populate the new DiveSample data
        current_dive_sample->timestamp = current_sample_timestamp_received;
        current_dive_sample->depth = initial_depth_received;
    }

    // --- Post-processing and User Input Section ---
    unsigned int max_depth_computed = 0; // Tracks the maximum depth found in samples
    int total_depth_sum = 0;             // Sum of all depths for average calculation
    int num_samples_processed = 0;       // Count of samples processed

    // Calculate dive duration in seconds, then convert to minutes
    // The last sample's timestamp minus the dive's start timestamp gives total duration
    unsigned int duration_seconds = current_dive_sample->timestamp - current_dive_record->dive_start_timestamp;
    current_dive_record->duration_mins = duration_seconds / 60;

    // Allocate an array to store depth distribution per minute
    // Size is (duration_mins + 1) to cover all minutes from 0 up to duration_mins
    int *depth_distribution = (int *)calloc(current_dive_record->duration_mins + 1, sizeof(int));
    if (depth_distribution == NULL) {
        perror("Error: Failed to allocate depth distribution array");
        exit(EXIT_FAILURE); // Terminate on critical memory allocation failure
    }

    // Iterate through all collected samples to calculate statistics
    for (current_dive_sample = current_dive_record->first_sample; current_dive_sample != NULL;
         current_dive_sample = current_dive_sample->next_sample) {
        // Update max depth
        if (max_depth_computed < (unsigned int)current_dive_sample->depth) {
            max_depth_computed = current_dive_sample->depth;
        }
        // Accumulate total depth
        total_depth_sum += current_dive_sample->depth;
        // Count samples
        num_samples_processed++;

        // Calculate the minute offset for the current sample
        unsigned int sample_minute_offset = (unsigned int)(current_dive_sample->timestamp - current_dive_record->dive_start_timestamp) / 60;
        // Increment count for this minute in the distribution array, if within bounds
        if (sample_minute_offset <= current_dive_record->duration_mins) {
            depth_distribution[sample_minute_offset]++;
        }
    }

    // Assign computed max depth to the dive record
    current_dive_record->max_depth = max_depth_computed;

    // Calculate total samples recorded (based on depth distribution, as per original logic)
    current_dive_record->total_samples_recorded = 0;
    for (unsigned int i = 0; i <= current_dive_record->duration_mins; i++) {
        current_dive_record->total_samples_recorded += depth_distribution[i];
    }
    free(depth_distribution); // Free the dynamically allocated array

    // Calculate average depth, handling division by zero
    if (num_samples_processed > 0) {
        current_dive_record->avg_depth = total_depth_sum / num_samples_processed;
    } else {
        current_dive_record->avg_depth = 0;
    }

    // --- User Interaction: Prompt for and update dive details ---
    // Using fgets for safer input into fixed-size buffers, then removing trailing newline.
    // strcspn finds the first occurrence of a character from the second string in the first.
    // If newline is found, it's replaced with null terminator.

    printf("Dive Site");
    if (current_dive_record->site_name[0] != '\0') {
        printf(" (%s)", current_dive_record->site_name);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(current_dive_record->site_name), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0; // Remove trailing newline
        if (strlen(input_buffer) > 0) {
            strncpy(current_dive_record->site_name, input_buffer, sizeof(current_dive_record->site_name) - 1);
            current_dive_record->site_name[sizeof(current_dive_record->site_name) - 1] = '\0'; // Ensure null-termination
        }
    }

    printf("Date (YYYY-MM-DD)");
    if (current_dive_record->date_str[0] != '\0') {
        printf(" (%s)", current_dive_record->date_str);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(current_dive_record->date_str), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            strncpy(current_dive_record->date_str, input_buffer, sizeof(current_dive_record->date_str) - 1);
            current_dive_record->date_str[sizeof(current_dive_record->date_str) - 1] = '\0';
        }
    }

    printf("Time (HH:MM:SS)");
    if (current_dive_record->time_str[0] != '\0') {
        printf(" (%s)", current_dive_record->time_str);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(current_dive_record->time_str), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            strncpy(current_dive_record->time_str, input_buffer, sizeof(current_dive_record->time_str) - 1);
            current_dive_record->time_str[sizeof(current_dive_record->time_str) - 1] = '\0';
        }
    }

    printf("Location (area/city)");
    if (current_dive_record->location[0] != '\0') {
        printf(" (%s)", current_dive_record->location);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(current_dive_record->location), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            strncpy(current_dive_record->location, input_buffer, sizeof(current_dive_record->location) - 1);
            current_dive_record->location[sizeof(current_dive_record->location) - 1] = '\0';
        }
    }

    printf("Max Depth in ft");
    if (current_dive_record->max_depth != 0) {
        printf(" (%d)", current_dive_record->max_depth);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            current_dive_record->max_depth = atoi(input_buffer);
        }
    }

    printf("Avg Depth in ft");
    if (current_dive_record->avg_depth != 0) {
        printf(" (%d)", current_dive_record->avg_depth);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            current_dive_record->avg_depth = atoi(input_buffer);
        }
    }

    printf("Dive Duration (mins)");
    if (current_dive_record->duration_mins != 0) {
        printf(" (%d)", current_dive_record->duration_mins);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            current_dive_record->duration_mins = atoi(input_buffer);
        }
    }

    printf("O2 Percentage");
    if (current_dive_record->o2_percent != 0) {
        printf(" (%d)", current_dive_record->o2_percent);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            current_dive_record->o2_percent = atoi(input_buffer);
        }
    }

    printf("Pressure In (psi)");
    if (current_dive_record->pressure_in != 0) {
        printf(" (%d)", current_dive_record->pressure_in);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            current_dive_record->pressure_in = atoi(input_buffer);
        }
    }

    printf("Pressure Out (psi)");
    if (current_dive_record->pressure_out != 0) {
        printf(" (%d)", current_dive_record->pressure_out);
    }
    printf("\n> ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            current_dive_record->pressure_out = atoi(input_buffer);
        }
    }

    current_dive_record->next_dive = NULL; // Explicitly null-terminate the list of dive records for the last one

    return 0; // Indicate successful completion
}