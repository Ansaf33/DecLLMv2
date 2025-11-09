#include <stdio.h>   // For STDOUT_FILENO, if send_bytes is a wrapper for write
#include <stdlib.h>  // For exit or similar termination
#include <unistd.h>  // For STDOUT_FILENO (standard output file descriptor)

// Function prototypes (assuming these are defined elsewhere in the project)
void init_election(void);
void print_menu(int menu_id); // Return value not used in the snippet
int get_choice(unsigned int *choice); // Returns status, updates choice via pointer
int decider(unsigned int choice_byte); // Returns status/decision result
int send_bytes(int fd, const char *buf, size_t count); // Returns bytes written
void _terminate(int exit_code); // Custom termination function

int main(void) {
    unsigned int choice = 0; // Corresponds to local_18 in the original snippet
    int status = 0;          // Corresponds to local_14 in the original snippet

    init_election();

    // The loop continues as long as 'status' is 0. Any non-zero value will terminate it.
    while (status == 0) {
        print_menu(4); // Display the main menu (menu ID 4)
        status = get_choice(&choice); // Get user input and update 'status'

        if (status == -0xf) { // If get_choice indicates a fatal error, break the loop
            break;
        }

        // Decide based on the user's choice, update 'status' with the decision result
        status = decider(choice & 0xff);

        // Handle specific decision outcomes (status 1 or 2 often indicate sub-menus)
        if (status == 1 || status == 2) {
            print_menu(status); // Display a sub-menu based on the current 'status'
            int temp_get_choice_status = get_choice(&choice); // Get another choice, store its status temporarily
            
            if (temp_get_choice_status != -0xf) { // If the second get_choice was not a fatal error
                status = decider(choice & 0xff); // Update 'status' with the result of the new decision
                if (status == 4) { // If this new decision result is 4
                    status = -1; // Override 'status' to -1, indicating an error exit condition
                }
            } else {
                status = temp_get_choice_status; // Propagate the fatal error status from the second get_choice
            }
        } else if (status == 4) { // If the initial decision result was 4
            status = -1; // Override 'status' to -1, indicating an error exit condition
        }
        // If 'status' remains 0, the loop will continue. Otherwise, it will terminate.
    }

    // After the loop, if 'status' is negative (indicating an error condition)
    if (status < 0) {
        // Attempt to send an error message to standard output
        if (send_bytes(STDOUT_FILENO, "Is voting too hard?!\n", 22) != 22) {
            _terminate(0xfffffff7); // If sending fails, terminate with a specific error code
        }
    }

    return 0; // Main function returns 0 for a successful execution path
}