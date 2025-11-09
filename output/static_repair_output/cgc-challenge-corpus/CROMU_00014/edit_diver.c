#include <stdio.h>   // For printf, FILE, stdin, getline
#include <string.h>  // For strncpy
#include <stdlib.h>  // For size_t, ssize_t, free

// Define maximum lengths for each field, including space for null terminator
#define MAX_LAST_NAME 21
#define MAX_FIRST_NAME 21
#define MAX_PHONE_NUMBER 16
#define MAX_STREET 30
#define MAX_CITY 20
#define MAX_STATE 3
#define MAX_ZIP_CODE 11
#define MAX_PADI_NUMBER 20
#define MAX_CERT_DATE 11

// Structure to hold diver information
// Offsets are derived from the original code's access patterns
typedef struct {
    char lastName[MAX_LAST_NAME];       // Offset 0x0
    char firstName[MAX_FIRST_NAME];     // Offset 0x15 (21)
    char phoneNumber[MAX_PHONE_NUMBER]; // Offset 0x2a (42)
    char street[MAX_STREET];            // Offset 0x3a (58)
    char city[MAX_CITY];                // Offset 0x58 (88)
    char state[MAX_STATE];              // Offset 0x6c (108)
    char zipCode[MAX_ZIP_CODE];         // Offset 0x6f (111)
    char padiNumber[MAX_PADI_NUMBER];   // Offset 0x7a (122)
    char certDate[MAX_CERT_DATE];       // Offset 0x8e (142)
} Diver;

// Function prototypes
int print_diver_info(Diver *diver);

// Function: edit_diver
int edit_diver(Diver *diver) {
    char *line = NULL; // Buffer for getline, will be allocated by getline
    size_t len = 0;    // Size of the buffer, updated by getline
    ssize_t read_len;  // Number of characters read by getline

    // First Name
    printf("First Name");
    if (diver->firstName[0] != '\0') {
        printf(" (%s)", diver->firstName);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0'; // Remove trailing newline
        }
        strncpy(diver->firstName, line, MAX_FIRST_NAME - 1);
        diver->firstName[MAX_FIRST_NAME - 1] = '\0'; // Ensure null termination
    }

    // Last Name
    printf("Last Name");
    if (diver->lastName[0] != '\0') {
        printf(" (%s)", diver->lastName);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->lastName, line, MAX_LAST_NAME - 1);
        diver->lastName[MAX_LAST_NAME - 1] = '\0';
    }

    // Street
    printf("Street");
    if (diver->street[0] != '\0') {
        printf(" (%s)", diver->street);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->street, line, MAX_STREET - 1);
        diver->street[MAX_STREET - 1] = '\0';
    }

    // City
    printf("City");
    if (diver->city[0] != '\0') {
        printf(" (%s)", diver->city);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->city, line, MAX_CITY - 1);
        diver->city[MAX_CITY - 1] = '\0';
    }

    // State
    printf("State");
    if (diver->state[0] != '\0') {
        printf(" (%s)", diver->state);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->state, line, MAX_STATE - 1);
        diver->state[MAX_STATE - 1] = '\0';
    }

    // Zip Code
    printf("Zip Code");
    if (diver->zipCode[0] != '\0') {
        printf(" (%s)", diver->zipCode);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->zipCode, line, MAX_ZIP_CODE - 1);
        diver->zipCode[MAX_ZIP_CODE - 1] = '\0';
    }

    // Phone Number
    printf("Phone Number");
    if (diver->phoneNumber[0] != '\0') {
        printf(" (%s)", diver->phoneNumber);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->phoneNumber, line, MAX_PHONE_NUMBER - 1);
        diver->phoneNumber[MAX_PHONE_NUMBER - 1] = '\0';
    }

    // PADI Diver Number
    printf("PADI Diver Number");
    if (diver->padiNumber[0] != '\0') {
        printf(" (%s)", diver->padiNumber);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->padiNumber, line, MAX_PADI_NUMBER - 1);
        diver->padiNumber[MAX_PADI_NUMBER - 1] = '\0';
    }

    // PADI Cert Date
    printf("PADI Cert Date");
    if (diver->certDate[0] != '\0') {
        printf(" (%s)", diver->certDate);
    }
    printf(": ");
    read_len = getline(&line, &len, stdin);
    if (read_len != -1) {
        if (read_len > 0 && line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        strncpy(diver->certDate, line, MAX_CERT_DATE - 1);
        diver->certDate[MAX_CERT_DATE - 1] = '\0';
    }

    free(line); // Free the memory allocated by getline
    print_diver_info(diver);
    return 0;
}

// Function: print_diver_info
int print_diver_info(Diver *diver) {
    printf("\n");
    printf("     Name: %s %s\n", diver->firstName, diver->lastName);
    printf("  Address: %s\n", diver->street);
    printf("     City: %s\n", diver->city);
    printf("    State: %s\n", diver->state);
    printf(" Zip Code: %s\n", diver->zipCode);
    printf("    Phone: %s\n", diver->phoneNumber);
    printf(" PADI Num: %s\n", diver->padiNumber);
    printf("Cert Date: %s\n", diver->certDate);
    printf("\n");
    return 0;
}

int main() {
    Diver myDiver = {0}; // Initialize all members to zero (empty strings)

    printf("--- Enter Diver Information ---\n");
    edit_diver(&myDiver);

    printf("\n--- Final Diver Information ---\n");
    print_diver_info(&myDiver);

    return 0;
}