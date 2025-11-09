#include <stdio.h>   // For printf, stdin, fgets
#include <stdlib.h>  // For malloc, free, atoi
#include <string.h>  // For strlen, strcpy, strncpy, strcmp, strchr, strcspn
#include <unistd.h>  // For getline (often available with stdio.h, but good to include for POSIX compliance)
#include <stdint.h>  // For fixed-width integers, though int is used for cost/duration as per original intent

// Error codes as defined by the original snippet's undefined4 return values
// Interpreted as signed 32-bit integers:
#define ERR_NULL_HEAD       -5 // 0xfffffffb
#define ERR_INVALID_FORMAT  -2 // 0xfffffffe
#define ERR_NOT_FOUND       -7 // 0xfffffff9 (for showAirports if no matching airport found)
#define ERR_DUPLICATE_CODE  -3 // 0xfffffffd
#define ERR_CONN_NOT_FOUND  -4 // 0xfffffffc (used for deleteAirport if target airport node not found)
#define ERR_MALLOC_FAILED   -100 // 0xffffff9c
#define ERR_EMPTY_INPUT     -6 // 0xfffffffa

// Structure definitions inferred from memory access patterns in the original code.
// The original code's offsets (+8 for connections, +0xc for next_airport) strongly
// suggest a 32-bit architecture where pointers are 4 bytes.
// This structure is designed to match that layout, using a reserved field for alignment.
// On a 64-bit system, standard alignment might make `_reserved` unnecessary,
// but including it maintains the original memory map intent for `connections` and `next_airport` offsets.

typedef struct AirportConnection {
    char connected_code[4]; // 3-char code + null terminator
    int cost;
    int duration;
    struct AirportConnection *next_connection;
} AirportConnection; // Total size 16 bytes (4+4+4+4)

typedef struct Airport {
    char code[4]; // 3-char code + null terminator
    int _reserved; // Padding or unknown field to align connections to +8 from start of struct
    AirportConnection *connections;
    struct Airport *next_airport;
} Airport; // Total size 16 bytes (4+4+4+4), matches malloc(0x10) for airport node in original code.

// Forward declaration for check4Code, used by other functions
int check4Code(Airport *head, const char *code_to_check);

// Function: showAirports
// Displays airport information based on a filter string.
// filter_str can be empty (e.g., "SHOW") to show all, or "/CODE" (e.g., "SHOW/JFK") for a specific airport.
int showAirports(Airport *head, char *filter_str) {
    char filter_code[4] = {'\0'};
    int found_airport = 0;

    if (head == NULL) {
        return ERR_NULL_HEAD;
    }

    // Parse filter string for specific airport code (e.g., "SHOW/JFK")
    size_t filter_str_len = strlen(filter_str);
    for (size_t i = 0; i < filter_str_len; ++i) {
        if (filter_str[i] == '/') {
            if (strlen(filter_str + i + 1) != 3) { // Expecting a 3-char code after '/'
                return ERR_INVALID_FORMAT;
            }
            strncpy(filter_code, filter_str + i + 1, 3);
            filter_code[3] = '\0'; // Ensure null-termination
            break;
        }
    }

    Airport *current_airport = head;
    while (current_airport != NULL) {
        // If no filter code or current airport matches the filter code
        if (filter_code[0] == '\0' || strcmp(filter_code, current_airport->code) == 0) {
            found_airport = 1;
            printf("Airport: %s\n", current_airport->code);
            printf("Connections: ");

            AirportConnection *current_conn = current_airport->connections;
            if (current_conn == NULL) {
                printf("None\n\n");
            } else {
                // Print first connection
                printf("%s (%d, %d)", current_conn->connected_code, current_conn->cost, current_conn->duration);
                // Print subsequent connections
                for (current_conn = current_conn->next_connection; current_conn != NULL; current_conn = current_conn->next_connection) {
                    printf(", %s (%d, %d)", current_conn->connected_code, current_conn->cost, current_conn->duration);
                }
                printf("\n\n");
            }
        }
        current_airport = current_airport->next_airport;
    }

    if (found_airport == 0 && filter_code[0] != '\0') {
        return ERR_NOT_FOUND; // Specific airport not found
    }
    return 0; // Success
}

// Temporary structure for storing connection data before linking to the main list
typedef struct TempConnection {
    char code[4];
    int cost;
    int duration;
} TempConnection;

// Function: addAirport
// Adds a new airport and its connections to the list.
// input_str can be empty (for interactive input) or formatted (e.g., "/JFK/LHR/100/60").
int addAirport(Airport **head_ref, char *input_str) {
    int slash_count = 0;
    size_t i;
    char *line_buffer = NULL; // Used for getline dynamic buffer
    size_t line_buffer_len = 0;
    char new_airport_code[4];
    TempConnection temp_connections[39]; // Max 39 connections, as implied by original aiStack_e4 size
    int num_temp_connections = 0;

    // Count slashes to determine input format (interactive vs. string-based)
    for (i = 0; input_str[i] != '\0'; ++i) {
        if (input_str[i] == '/') {
            slash_count++;
        }
    }

    if (slash_count == 0) { // Interactive input (e.g., "ADD")
        printf("Enter airport code:\n");
        if (getline(&line_buffer, &line_buffer_len, stdin) == -1) {
            fprintf(stderr, "Error reading line.\n");
            return ERR_INVALID_FORMAT;
        }
        // Expecting 3 chars + newline, so total length >= 4. Or just a newline if empty.
        if (strlen(line_buffer) < 4 || line_buffer[0] == '\n') {
            free(line_buffer);
            return ERR_INVALID_FORMAT;
        }
        strncpy(new_airport_code, line_buffer, 3);
        new_airport_code[3] = '\0'; // Ensure null-termination
        free(line_buffer);
        line_buffer = NULL;
        line_buffer_len = 0;

        if (check4Code(*head_ref, new_airport_code) != 0) {
            printf("Duplicate code detected\n");
            return ERR_DUPLICATE_CODE;
        }

        while (num_temp_connections < 39) { // Limit number of connections
            printf("Enter airport code for connection (or empty to finish):\n");
            if (getline(&line_buffer, &line_buffer_len, stdin) == -1) {
                 fprintf(stderr, "Error reading line.\n");
                 return ERR_INVALID_FORMAT;
            }
            if (line_buffer[0] == '\n' || line_buffer[0] == '\0') { // Empty line signifies end of connections
                free(line_buffer);
                line_buffer = NULL;
                line_buffer_len = 0;
                break;
            }
            if (strlen(line_buffer) < 4) { // Expecting 3 chars + newline
                printf("Invalid connection code length.\n");
                free(line_buffer);
                line_buffer = NULL;
                line_buffer_len = 0;
                continue;
            }

            char conn_code[4];
            strncpy(conn_code, line_buffer, 3);
            conn_code[3] = '\0'; // Ensure null-termination
            free(line_buffer);
            line_buffer = NULL;
            line_buffer_len = 0;

            if (strcmp(new_airport_code, conn_code) == 0) {
                printf("Illegal connection value (cannot connect to self)\n");
            } else if (check4Code(*head_ref, conn_code) == 0) {
                printf("Connecting airport not found\n");
            } else {
                int duplicate_conn = 0;
                for (int j = 0; j < num_temp_connections; ++j) {
                    if (strcmp(conn_code, temp_connections[j].code) == 0) {
                        duplicate_conn = 1;
                        printf("Duplicate connection\n");
                        break;
                    }
                }
                if (!duplicate_conn) {
                    strncpy(temp_connections[num_temp_connections].code, conn_code, 3);
                    temp_connections[num_temp_connections].code[3] = '\0';

                    printf("Connection cost:\n");
                    if (getline(&line_buffer, &line_buffer_len, stdin) == -1) {
                        fprintf(stderr, "Error reading line.\n");
                        return ERR_INVALID_FORMAT;
                    }
                    temp_connections[num_temp_connections].cost = atoi(line_buffer);
                    free(line_buffer);
                    line_buffer = NULL;
                    line_buffer_len = 0;

                    printf("Connection duration:\n");
                    if (getline(&line_buffer, &line_buffer_len, stdin) == -1) {
                        fprintf(stderr, "Error reading line.\n");
                        return ERR_INVALID_FORMAT;
                    }
                    temp_connections[num_temp_connections].duration = atoi(line_buffer);
                    free(line_buffer);
                    line_buffer = NULL;
                    line_buffer_len = 0;

                    num_temp_connections++;
                }
            }
        }
    } else { // String-based input (e.g., "ADD/JFK/LHR/100/60/CDG/200/120")
        // Format: /CODE/CONN_CODE/COST/DURATION(/CONN_CODE/COST/DURATION...)
        // Expected slashes: 1 for airport code, then 3 for each connection (code/cost/duration)
        if (slash_count < 1 || (slash_count - 1) % 3 != 0) {
            return ERR_INVALID_FORMAT;
        }

        i = 0;
        // Skip initial '/'
        while (input_str[i] != '/' && input_str[i] != '\0') i++;
        if (input_str[i] == '/') i++; // Move past the first slash

        // Read new airport code
        strncpy(new_airport_code, input_str + i, 3);
        new_airport_code[3] = '\0';
        i += 3;

        if (check4Code(*head_ref, new_airport_code) != 0) {
            return ERR_DUPLICATE_CODE;
        }

        while (input_str[i] != '\0') {
            if (num_temp_connections >= 39) {
                // Too many connections, stop processing further connections
                break;
            }

            if (input_str[i] != '/') {
                return ERR_INVALID_FORMAT;
            }
            i++; // Move past '/'

            // Read connection code
            strncpy(temp_connections[num_temp_connections].code, input_str + i, 3);
            temp_connections[num_temp_connections].code[3] = '\0';
            i += 3;

            if (check4Code(*head_ref, temp_connections[num_temp_connections].code) == 0) {
                return ERR_CONN_NOT_FOUND;
            }

            // Check for duplicate connection in current temporary list
            for (int j = 0; j < num_temp_connections; ++j) {
                if (strcmp(temp_connections[num_temp_connections].code, temp_connections[j].code) == 0) {
                    return ERR_DUPLICATE_CODE;
                }
            }
            
            if (input_str[i] != '/') {
                return ERR_INVALID_FORMAT;
            }
            i++; // Move past '/'

            // Read connection cost
            char *next_slash = strchr(input_str + i, '/');
            char temp_val[32]; // Buffer for atoi conversion

            if (next_slash == NULL) { // Malformed, expected another slash for duration
                 return ERR_INVALID_FORMAT;
            }
            
            size_t val_len = next_slash - (input_str + i);
            if (val_len >= sizeof(temp_val)) return ERR_INVALID_FORMAT;
            strncpy(temp_val, input_str + i, val_len);
            temp_val[val_len] = '\0';
            temp_connections[num_temp_connections].cost = atoi(temp_val);
            i += val_len + 1; // Move past value and '/'

            // Read connection duration
            char *end_of_str = strchr(input_str + i, '\0');
            next_slash = strchr(input_str + i, '/'); // Look for next connection's slash
            char *end_of_val = (next_slash != NULL) ? next_slash : end_of_str;

            val_len = end_of_val - (input_str + i);
            if (val_len >= sizeof(temp_val)) return ERR_INVALID_FORMAT;
            strncpy(temp_val, input_str + i, val_len);
            temp_val[val_len] = '\0';
            
            temp_connections[num_temp_connections].duration = atoi(temp_val);
            
            i += val_len;
            if (input_str[i] == '/') { // If there's another slash, it means more connections
                i++;
            }
            num_temp_connections++;
        }
    }

    // Allocate new airport node
    Airport *new_airport = (Airport *)malloc(sizeof(Airport));
    if (new_airport == NULL) {
        return ERR_MALLOC_FAILED;
    }
    strncpy(new_airport->code, new_airport_code, 3);
    new_airport->code[3] = '\0';
    new_airport->_reserved = 0; // Initialize reserved field
    new_airport->connections = NULL;
    new_airport->next_airport = NULL;

    // Add new airport to the linked list
    if (*head_ref == NULL) {
        *head_ref = new_airport;
    } else {
        Airport *current = *head_ref;
        while (current->next_airport != NULL) {
            current = current->next_airport;
        }
        current->next_airport = new_airport;
    }

    // Add connections from temporary storage to the new airport's connection list
    AirportConnection *last_conn = NULL;
    for (int j = 0; j < num_temp_connections; ++j) {
        AirportConnection *new_conn = (AirportConnection *)malloc(sizeof(AirportConnection));
        if (new_conn == NULL) {
            // Memory allocation failed for a connection.
            // Clean up connections already allocated for this airport and return error.
            AirportConnection *conn_to_free = new_airport->connections;
            while(conn_to_free != NULL) {
                AirportConnection *next = conn_to_free->next_connection;
                free(conn_to_free);
                conn_to_free = next;
            }
            // Note: The airport node itself is already linked.
            // A more robust solution might involve unlinking it or requiring
            // the caller to handle partial additions. For simplicity,
            // matching original behavior, we return an error.
            return ERR_MALLOC_FAILED;
        }
        strncpy(new_conn->connected_code, temp_connections[j].code, 3);
        new_conn->connected_code[3] = '\0';
        new_conn->cost = temp_connections[j].cost;
        new_conn->duration = temp_connections[j].duration;
        new_conn->next_connection = NULL;

        if (new_airport->connections == NULL) {
            new_airport->connections = new_conn;
        } else {
            last_conn->next_connection = new_conn;
        }
        last_conn = new_conn;
    }

    return 0; // Success
}

// Function: deleteAirport
// Deletes an airport node and all connections to/from it.
// input_str can be empty (for interactive input) or formatted (e.g., "/JFK").
int deleteAirport(Airport **head_ref, char *input_str) {
    char target_code[4] = {'\0'};
    char *line_buffer = NULL;
    size_t line_buffer_len = 0;

    if (*head_ref == NULL) {
        return ERR_NULL_HEAD;
    }

    // Parse input_str for target code
    size_t input_len = strlen(input_str);
    for (size_t i = 0; i < input_len; ++i) {
        if (input_str[i] == '/') {
            if (strlen(input_str + i + 1) != 3) {
                return ERR_INVALID_FORMAT;
            }
            strncpy(target_code, input_str + i + 1, 3);
            target_code[3] = '\0';
            break;
        }
    }

    if (target_code[0] == '\0') { // If no code in input_str, prompt interactively
        printf("Enter airport code for deletion:\n");
        if (getline(&line_buffer, &line_buffer_len, stdin) == -1) {
            fprintf(stderr, "Error reading line.\n");
            return ERR_INVALID_FORMAT;
        }
        if (line_buffer[0] == '\n' || line_buffer[0] == '\0') { // Empty input
            free(line_buffer);
            return ERR_EMPTY_INPUT;
        }
        if (strlen(line_buffer) < 4) { // Expecting 3 chars + newline
            free(line_buffer);
            return ERR_INVALID_FORMAT;
        }
        strncpy(target_code, line_buffer, 3);
        target_code[3] = '\0';
        free(line_buffer);
        line_buffer = NULL;
        line_buffer_len = 0;
    }

    // --- Phase 1: Delete the airport node itself if it matches target_code ---
    Airport *current_airport = *head_ref;
    Airport *prev_airport = NULL;
    int airport_deleted = 0;

    while (current_airport != NULL) {
        if (strcmp(current_airport->code, target_code) == 0) {
            // Found the airport to delete
            if (prev_airport == NULL) { // It's the head of the list
                *head_ref = current_airport->next_airport;
            } else {
                prev_airport->next_airport = current_airport->next_airport;
            }

            // Free all connections of the deleted airport
            AirportConnection *conn_to_free = current_airport->connections;
            while (conn_to_free != NULL) {
                AirportConnection *next_conn = conn_to_free->next_connection;
                free(conn_to_free);
                conn_to_free = next_conn;
            }
            free(current_airport);
            airport_deleted = 1;
            break; // Airport node deleted, exit loop
        }
        prev_airport = current_airport;
        current_airport = current_airport->next_airport;
    }

    // According to the original code's error flow, if the target airport node itself
    // is not found and deleted in the first phase, it returns 0xfffffffc (ERR_CONN_NOT_FOUND).
    if (!airport_deleted) {
        return ERR_CONN_NOT_FOUND;
    }

    // --- Phase 2: Iterate through all remaining airports to delete connections *to* the target_code ---
    // Start from the (potentially new) head of the airport list
    current_airport = *head_ref; 
    while (current_airport != NULL) {
        if (current_airport->connections == NULL) {
            current_airport = current_airport->next_airport;
            continue;
        }

        // Handle deletion of connections from this airport to the target_code
        // This loop correctly handles multiple consecutive matches or a match at the head of connections
        AirportConnection *prev_conn = NULL;
        AirportConnection *current_conn = current_airport->connections;

        while (current_conn != NULL) {
            if (strcmp(current_conn->connected_code, target_code) == 0) {
                // Found a connection to delete
                if (prev_conn == NULL) { // First connection in the list
                    current_airport->connections = current_conn->next_connection;
                } else {
                    prev_conn->next_connection = current_conn->next_connection;
                }
                AirportConnection *conn_to_free = current_conn;
                current_conn = current_conn->next_connection; // Move to next connection (which might be the new head)
                free(conn_to_free);
            } else {
                prev_conn = current_conn;
                current_conn = current_conn->next_connection;
            }
        }
        current_airport = current_airport->next_airport;
    }

    return 0; // Success
}

// Function: check4Code
// Scans the list of airports for a given airport code.
// Returns 1 if code is found, 0 otherwise.
int check4Code(Airport *head, const char *code_to_check) {
    Airport *current = head;
    while (current != NULL) {
        if (strcmp(current->code, code_to_check) == 0) {
            return 1; // Found
        }
        current = current->next_airport;
    }
    return 0; // Not found
}

// Example main function for testing the airport management system.
int main() {
    Airport *airport_list_head = NULL;
    char command_buffer[256]; // Buffer to read user commands

    printf("Airport Management System\n");
    printf("Commands:\n");
    printf("  ADD               (interactive input for new airport and connections)\n");
    printf("  ADD/CODE/CONN/COST/DUR... (string input, e.g., ADD/JFK/LHR/100/60)\n");
    printf("  DELETE            (interactive input for airport code to delete)\n");
    printf("  DELETE/CODE       (string input, e.g., DELETE/JFK)\n");
    printf("  SHOW              (show all airports)\n");
    printf("  SHOW/CODE         (show specific airport, e.g., SHOW/JFK)\n");
    printf("  EXIT              (terminate program)\n");

    while (1) {
        printf("\n> ");
        if (fgets(command_buffer, sizeof(command_buffer), stdin) == NULL) {
            // EOF or error reading input
            break;
        }
        // Remove trailing newline character
        command_buffer[strcspn(command_buffer, "\n")] = 0;

        if (strncmp(command_buffer, "ADD", 3) == 0) {
            int result = addAirport(&airport_list_head, command_buffer + 3);
            if (result == 0) {
                printf("Airport added successfully.\n");
            } else {
                printf("Error adding airport: %d\n", result);
            }
        } else if (strncmp(command_buffer, "DELETE", 6) == 0) {
            int result = deleteAirport(&airport_list_head, command_buffer + 6);
            if (result == 0) {
                printf("Airport deleted successfully.\n");
            } else {
                printf("Error deleting airport: %d\n", result);
            }
        } else if (strncmp(command_buffer, "SHOW", 4) == 0) {
            int result = showAirports(airport_list_head, command_buffer + 4);
            if (result != 0) {
                printf("Error showing airports: %d\n", result);
            }
        } else if (strcmp(command_buffer, "EXIT") == 0) {
            break; // Exit the main loop
        } else {
            printf("Unknown command.\n");
        }
    }

    // Cleanup: Free all allocated memory before exiting
    Airport *current_airport = airport_list_head;
    while (current_airport != NULL) {
        AirportConnection *current_conn = current_airport->connections;
        while (current_conn != NULL) {
            AirportConnection *next_conn = current_conn->next_connection;
            free(current_conn);
            current_conn = next_conn;
        }
        Airport *next_airport = current_airport->next_airport;
        free(current_airport);
        current_airport = next_airport;
    }

    return 0;
}