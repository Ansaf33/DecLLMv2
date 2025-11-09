#include <stdio.h>  // For printf, fgets, stdin
#include <stdlib.h> // For atoi
#include <string.h> // For strcspn, strcpy

// Function: list_dives
// param_1 is expected to be a pointer to a structure that contains a pointer
// to the head of the dive entry linked list at offset 0x9c.
// Each dive entry is a memory block where data is stored at specific offsets.
int list_dives(char *dive_log_ptr) {
    int dive_num = 1;
    // Retrieve the pointer to the first dive entry from dive_log_ptr + 0x9c
    char *current_dive_ptr = *(char **)(dive_log_ptr + 0x9c);

    printf("\n");
    if (current_dive_ptr == NULL) { // Check if the dive log is empty
        printf("Dive Log is empty\n");
        return -1; // Return -1 to indicate an empty log or error
    } else {
        // Print header row with corrected format specifiers
        printf("Dive# %-10s %-8s %-25s %-25s\n", "Date", "Time", "Dive Site", "Location");
        // Iterate through the linked list of dive entries
        for (; current_dive_ptr != NULL; current_dive_ptr = *(char **)(current_dive_ptr + 0x7c)) {
            // Print dive entry details using pointer arithmetic for offsets
            printf("%4d: %-10s %-8s %-25s %-25s\n",
                   dive_num,
                   (char *)(current_dive_ptr + 0x1a), // Date string at offset 0x1a
                   (char *)(current_dive_ptr + 0x25), // Time string at offset 0x25
                   (char *)(current_dive_ptr),        // Dive Site string at offset 0x00 (start of entry)
                   (char *)(current_dive_ptr + 0x5c)  // Location string at offset 0x5c
            );
            dive_num++;
        }
        return 0; // Return 0 to indicate success
    }
}

// Function: print_dives
// param_1 is expected to be a pointer to a structure that contains a pointer
// to the head of the dive entry linked list at offset 0x9c.
int print_dives(char *dive_log_ptr) {
    // First, list all dives. If the log is empty, return early.
    if (list_dives(dive_log_ptr) == -1) {
        return -1; // Propagate the error/empty status from list_dives
    }

    printf("\n");
    printf("Enter Dive # to display: ");

    char input_buffer[100]; // Buffer to store user input
    // Read a line of input from stdin using fgets
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        return 0; // Return 0 if fgets encounters an error or EOF
    }

    // Remove the trailing newline character from the input buffer
    input_buffer[strcspn(input_buffer, "\n")] = 0;

    // Convert the input string to an integer
    int dive_num_to_display = atoi(input_buffer);
    if (dive_num_to_display <= 0) { // Check for invalid (non-positive) dive numbers
        printf("Invalid dive number entered\n");
        return 0;
    }

    // Initialize current_dive_ptr to the head of the list
    char *current_dive_ptr = *(char **)(dive_log_ptr + 0x9c);
    int current_dive_num = 1;

    // Iterate through the list to find the desired dive entry
    for (; (current_dive_num < dive_num_to_display) && (current_dive_ptr != NULL);
           current_dive_ptr = *(char **)(current_dive_ptr + 0x7c)) {
        current_dive_num++;
    }

    // If the desired dive number was found and the pointer is valid
    if ((current_dive_num == dive_num_to_display) && (current_dive_ptr != NULL)) {
        printf("\n");
        printf("          Date: %s\n", (char *)(current_dive_ptr + 0x1a));
        printf("          Time: %s\n", (char *)(current_dive_ptr + 0x25));
        printf("     Dive Site: %s\n", (char *)(current_dive_ptr));
        printf("      Location: %s\n", (char *)(current_dive_ptr + 0x5c));
        printf("     Max Depth: %d\n", *(int *)(current_dive_ptr + 0x34));
        printf("     Avg Depth: %d\n", *(int *)(current_dive_ptr + 0x38));
        printf("      Duration: %d\n", *(int *)(current_dive_ptr + 0x3c));
        printf("    O2 Percent: %d\n", *(int *)(current_dive_ptr + 0x48));
        printf("Start Pressure: %d\n", *(int *)(current_dive_ptr + 0x40));
        printf("  End Pressure: %d\n", *(int *)(current_dive_ptr + 0x44));
        printf("     Bin Count: %d\n", *(int *)(current_dive_ptr + 0x58));
        printf("\n");
    } else {
        printf("Invalid dive number entered\n");
    }

    return 0;
}

int main() {
    // Simulate raw memory blocks for dive entries.
    // Each entry is a char array, with data placed at specific offsets.
    // Size 128 is arbitrary but must be large enough to contain all data and pointers (0x7c + sizeof(char*)).
    char raw_entry1[128] = {0}; // Initialize to zeros for clean start
    char raw_entry2[128] = {0};

    // Populate raw_entry1 with dummy data
    strcpy(raw_entry1 + 0x00, "Blue Hole");         // Dive Site
    strcpy(raw_entry1 + 0x1a, "2023-01-15");        // Date
    strcpy(raw_entry1 + 0x25, "10:30");             // Time
    *(int*)(raw_entry1 + 0x34) = 30;                // Max Depth
    *(int*)(raw_entry1 + 0x38) = 20;                // Avg Depth
    *(int*)(raw_entry1 + 0x3c) = 45;                // Duration
    *(int*)(raw_entry1 + 0x40) = 2000;              // Start Pressure
    *(int*)(raw_entry1 + 0x44) = 500;               // End Pressure
    *(int*)(raw_entry1 + 0x48) = 21;                // O2 Percent
    *(int*)(raw_entry1 + 0x58) = 5;                 // Bin Count
    strcpy(raw_entry1 + 0x5c, "Belize Barrier Reef"); // Location

    // Populate raw_entry2 with dummy data
    strcpy(raw_entry2 + 0x00, "USS Kittiwake");
    strcpy(raw_entry2 + 0x1a, "2023-02-20");
    strcpy(raw_entry2 + 0x25, "14:00");
    *(int*)(raw_entry2 + 0x34) = 22;
    *(int*)(raw_entry2 + 0x38) = 18;
    *(int*)(raw_entry2 + 0x3c) = 50;
    *(int*)(raw_entry2 + 0x40) = 1800;
    *(int*)(raw_entry2 + 0x44) = 600;
    *(int*)(raw_entry2 + 0x48) = 21;
    *(int*)(raw_entry2 + 0x58) = 4;
    strcpy(raw_entry2 + 0x5c, "Grand Cayman");

    // Link the dive entries: raw_entry1 points to raw_entry2
    *(char**)(raw_entry1 + 0x7c) = raw_entry2;
    *(char**)(raw_entry2 + 0x7c) = NULL; // End of the linked list

    // Simulate a raw memory block for the dive log structure.
    // This block contains the pointer to the head of the dive entry list at offset 0x9c.
    char raw_dive_log[128] = {0};
    *(char**)(raw_dive_log + 0x9c) = raw_entry1; // Set the head of the list

    // Test the functions
    printf("--- Listing Dives ---\n");
    list_dives(raw_dive_log);

    printf("\n--- Printing a Specific Dive (Enter 1 or 2 to test) ---\n");
    print_dives(raw_dive_log);

    // Test with an empty log
    printf("\n--- Test with Empty Log ---\n");
    char empty_raw_dive_log[128] = {0}; // All zeros, so 0x9c will be NULL
    list_dives(empty_raw_dive_log);
    print_dives(empty_raw_dive_log);

    return 0;
}