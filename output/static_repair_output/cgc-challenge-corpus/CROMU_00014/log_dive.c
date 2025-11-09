#include <stdio.h>    // For printf, fgets, stdin, NULL, FILE
#include <stdlib.h>   // For calloc, atoi, exit, EXIT_FAILURE
#include <string.h>   // For strncpy, strlen, strncat, memset, strcspn
#include <unistd.h>   // For ssize_t (though not strictly necessary with fgets)

// Placeholder for external function
// Assuming it takes a combined date/time string (e.g., "YYYY-MM-DD:HH:MM:SS")
// and returns an unsigned int timestamp.
extern unsigned int str2datetime(char *datetime_str);

// Global string literals identified from decompiler
static const char *const DAT_000173b7 = "\n";
static const char *const DAT_000173ba = "\nDive Date (YYYY-MM-DD HH:MM:SS): ";
static const char *const DAT_000173bf = "\nAir Temp (C): "; // Likely mislabeled in original decompiler output
static const char *const DAT_000173c4 = ":"; // Used for combining date/time components

// Define the DiveEntry struct based on analysis (128 bytes total, 0x80)
// Offsets are derived from the original decompiler output's `local_10 + 0xXX` accesses.
typedef struct DiveEntry DiveEntry;
struct DiveEntry {
    char site_name[26];              // 0x00-0x19, used with strncpy limit 0x1a
    char location_area[11];          // 0x1a-0x24, used with strncpy limit 0xb (often for date part)
    char date_string[9];             // 0x25-0x2d, used with strncpy limit 9 (often for time part)
    char _pad1[0x30 - (0x25 + 9)];   // 0x2e-0x2f (2 bytes padding to 0x30)
    unsigned int datetime_timestamp; // 0x30-0x33
    int max_depth;                   // 0x34-0x37
    int avg_depth;                   // 0x38-0x3b
    int duration_mins;               // 0x3c-0x3f
    int pressure_in;                 // 0x40-0x43
    int pressure_out;                // 0x44-0x47
    int o2_percentage;               // 0x48-0x4b
    char _pad2[0x5c - (0x48 + 4)];   // 0x4c-0x5b (16 bytes padding to 0x5c)
    char general_location[26];       // 0x5c-0x75, used with strncpy limit 0x1a
    char _pad3[0x7c - (0x5c + 26)];  // 0x76-0x7b (6 bytes padding to 0x7c)
    DiveEntry *next;                 // 0x7c-0x7f (8 bytes for pointer on 64-bit systems)
};

// Define the DiveList struct, `param_1` points to this.
// `param_1 + 0x9c` corresponds to `list_ptr->head_dive`.
typedef struct DiveList {
    char _padding_before_head[0x9c]; // Padding to reach the head_dive pointer
    DiveEntry *head_dive;            // At offset 0x9c
} DiveList;

// Function: log_dives
int log_dives(DiveList *list_ptr) {
    DiveEntry *current_entry;
    char input_buffer_a[1024]; // Used as local_414
    char input_buffer_b[1024]; // Used as local_814

    // Handle the linked list traversal/creation
    if (list_ptr->head_dive == NULL) {
        current_entry = (DiveEntry *)calloc(1, sizeof(DiveEntry));
        if (current_entry == NULL) {
            fprintf(stderr, "Error: calloc failed for new dive entry.\n");
            exit(EXIT_FAILURE);
        }
        list_ptr->head_dive = current_entry;
    } else {
        DiveEntry *temp_entry = list_ptr->head_dive;
        while (temp_entry->next != NULL) {
            temp_entry = temp_entry->next;
        }
        current_entry = (DiveEntry *)calloc(1, sizeof(DiveEntry));
        if (current_entry == NULL) {
            fprintf(stderr, "Error: calloc failed for new dive entry.\n");
            exit(EXIT_FAILURE);
        }
        temp_entry->next = current_entry;
    }

    // ************* Dive Site *************
    printf("Dive Site");
    if (current_entry->site_name[0] != '\0') {
        printf(" (%s)", current_entry->site_name);
    }
    printf(DAT_000173b7); // Newline
    
    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0; // Remove trailing newline
        size_t input_len = strlen(input_buffer_a);
        size_t copy_len = (input_len < sizeof(current_entry->site_name) - 1) ? input_len : (sizeof(current_entry->site_name) - 1);
        strncpy(current_entry->site_name, input_buffer_a, copy_len);
        current_entry->site_name[copy_len] = '\0'; // Ensure null termination
    }

    // ************* Dive Date (YYYY-MM-DD) *************
    // This part prints "Dive Date (YYYY-MM-DD HH:MM:SS): " but the input
    // is limited to 0xb (11) characters and stored in `location_area`.
    // Re-interpreting to fit the explicit limits.
    printf(DAT_000173ba); // Original prompt from decompiler
    if (current_entry->location_area[0] != '\0') {
        printf(" (%s)", current_entry->location_area);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        size_t input_len = strlen(input_buffer_a);
        size_t copy_len = (input_len < sizeof(current_entry->location_area) - 1) ? input_len : (sizeof(current_entry->location_area) - 1);
        strncpy(current_entry->location_area, input_buffer_a, copy_len);
        current_entry->location_area[copy_len] = '\0';
    }

    // ************* Dive Time (HH:MM:SS) *************
    // This part prints "Air Temp (C): " but the input is limited to 9 characters
    // and stored in `date_string`. This likely serves as the time component.
    printf(DAT_000173bf); // Original prompt from decompiler
    if (current_entry->date_string[0] != '\0') {
        printf(" (%s)", current_entry->date_string);
    }
    printf(DAT_000173b7);

    memset(input_buffer_b, 0, sizeof(input_buffer_b)); // Using buffer_b for this input
    if (fgets(input_buffer_b, sizeof(input_buffer_b), stdin) != NULL) {
        input_buffer_b[strcspn(input_buffer_b, "\n")] = 0;
        size_t input_len = strlen(input_buffer_b);
        size_t copy_len = (input_len < sizeof(current_entry->date_string) - 1) ? input_len : (sizeof(current_entry->date_string) - 1);
        strncpy(current_entry->date_string, input_buffer_b, copy_len);
        current_entry->date_string[copy_len] = '\0';
    }

    // Combine date (from location_area) and time (from date_string) and call str2datetime
    // The decompiler output concatenates with ":"
    strncpy(input_buffer_a, current_entry->location_area, sizeof(input_buffer_a) - 1);
    input_buffer_a[sizeof(input_buffer_a) - 1] = '\0';
    strncat(input_buffer_a, DAT_000173c4, sizeof(input_buffer_a) - strlen(input_buffer_a) - 1); // Append ":"
    strncat(input_buffer_a, current_entry->date_string, sizeof(input_buffer_a) - strlen(input_buffer_a) - 1); // Append time string
    current_entry->datetime_timestamp = str2datetime(input_buffer_a);

    // ************* Location (area/city) *************
    printf("Location (area/city)");
    if (current_entry->general_location[0] != '\0') {
        printf(" (%s)", current_entry->general_location);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        size_t input_len = strlen(input_buffer_a);
        size_t copy_len = (input_len < sizeof(current_entry->general_location) - 1) ? input_len : (sizeof(current_entry->general_location) - 1);
        strncpy(current_entry->general_location, input_buffer_a, copy_len);
        current_entry->general_location[copy_len] = '\0';
    }

    // ************* Max Depth in ft *************
    printf("Max Depth in ft");
    if (current_entry->max_depth != 0) {
        printf(" (%d)", current_entry->max_depth);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        current_entry->max_depth = atoi(input_buffer_a);
    }

    // ************* Avg Depth in ft *************
    printf("Avg Depth in ft");
    if (current_entry->avg_depth != 0) {
        printf(" (%d)", current_entry->avg_depth);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        current_entry->avg_depth = atoi(input_buffer_a);
    }

    // ************* Dive Duration (mins) *************
    printf("Dive Duration (mins)");
    if (current_entry->duration_mins != 0) {
        printf(" (%d)", current_entry->duration_mins);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        current_entry->duration_mins = atoi(input_buffer_a);
    }

    // ************* O2 Percentage *************
    printf("O2 Percentage");
    if (current_entry->o2_percentage != 0) {
        printf(" (%d)", current_entry->o2_percentage);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        current_entry->o2_percentage = atoi(input_buffer_a);
    }

    // ************* Pressure In (psi) *************
    printf("Pressure In (psi)");
    if (current_entry->pressure_in != 0) {
        printf(" (%d)", current_entry->pressure_in);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        current_entry->pressure_in = atoi(input_buffer_a);
    }

    // ************* Pressure Out (psi) *************
    printf("Pressure Out (psi)");
    if (current_entry->pressure_out != 0) {
        printf(" (%d)", current_entry->pressure_out);
    }
    printf(DAT_000173b7);

    memset(input_buffer_a, 0, sizeof(input_buffer_a));
    if (fgets(input_buffer_a, sizeof(input_buffer_a), stdin) != NULL) {
        input_buffer_a[strcspn(input_buffer_a, "\n")] = 0;
        current_entry->pressure_out = atoi(input_buffer_a);
    }

    current_entry->next = NULL; // Mark the end of the list for this new entry

    return 0;
}