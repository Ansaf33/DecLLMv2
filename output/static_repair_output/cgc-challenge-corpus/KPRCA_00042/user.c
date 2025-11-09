#include <stdio.h>    // For fprintf, vfprintf, snprintf, NULL, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO
#include <stdlib.h>   // For malloc, free, strtol
#include <string.h>   // For strcmp, strcspn
#include <unistd.h>   // For write (if fdprintf writes to non-standard FDs)
#include <stdarg.h>   // For va_list in fdprintf

// Type definitions
typedef unsigned int uint;

// Global variables for rng
// Initialized to arbitrary values for demonstration. For actual randomness, these would be seeded.
static uint a = 0xf00dcafe;
static uint b = 0xdeadbeef;
static uint c = 0xc0ffee;
static uint d = 0xbaadface;

// Global variables for admin credentials
static char *admin_username = "admin"; // Default admin username
static char *admin_password = NULL;    // Password is generated at runtime

// --- Helper Functions (replacing custom/unspecified ones) ---

// Custom fdprintf: Replaces a potential custom function, using standard library calls.
// It handles stdout (fd 1), stderr (fd 2), and other FDs using write (after formatting).
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret;

    if (fd == STDOUT_FILENO) { // Standard output
        ret = vfprintf(stdout, format, args);
    } else if (fd == STDERR_FILENO) { // Standard error
        ret = vfprintf(stderr, format, args);
    } else {
        // For other file descriptors, format into a buffer and then write.
        // This assumes messages are not excessively long.
        char buffer[1024]; 
        ret = vsnprintf(buffer, sizeof(buffer), format, args);
        if (ret < 0) { // Error in formatting
            va_end(args);
            return ret;
        }
        // Ensure null terminator is not written if buffer is full and ret == sizeof(buffer)
        size_t len = (size_t)ret < sizeof(buffer) ? (size_t)ret : sizeof(buffer) - 1;
        write(fd, buffer, len); // Use write for generic file descriptors
    }
    va_end(args);
    return ret;
}

// Custom readline: Replaces a potential custom function, using fgets for stdin.
// It removes the trailing newline character.
void readline(int fd, char *buffer, size_t max_len) {
    if (fd == STDIN_FILENO) { // Standard input
        if (fgets(buffer, max_len, stdin) != NULL) {
            // Remove trailing newline character if present
            buffer[strcspn(buffer, "\n")] = 0;
        } else {
            // Handle EOF or error, e.g., clear buffer
            buffer[0] = '\0';
        }
    } else {
        // For other FDs, a more robust read loop would be needed,
        // but for this context, we primarily expect stdin.
        fdprintf(STDERR_FILENO, "[WARNING] readline from non-stdin fd not fully implemented, returning empty string.\n");
        buffer[0] = '\0'; // Ensure buffer is null-terminated
    }
}

// Stubs for movie-related functions (as they were not provided)
void list_all_movies(void) { fdprintf(STDOUT_FILENO, "[STUB] Listing all movies...\n"); }
uint get_num_owned_movies(void) { fdprintf(STDOUT_FILENO, "[STUB] Getting number of owned movies (returning 3 for demonstration).\n"); return 3; }
void list_owned_movies(void) { fdprintf(STDOUT_FILENO, "[STUB] Listing owned movies...\n"); }
void rent_movie(uint movie_id) { fdprintf(STDOUT_FILENO, "[STUB] Renting movie ID: %u\n", movie_id); }
uint get_num_rented_movies(void) { fdprintf(STDOUT_FILENO, "[STUB] Getting number of rented movies (returning 2 for demonstration).\n"); return 2; }
void list_rented_movies(void) { fdprintf(STDOUT_FILENO, "[STUB] Listing rented movies...\n"); }
void return_movie(uint movie_id) { fdprintf(STDOUT_FILENO, "[STUB] Returning movie ID: %u\n", movie_id); }

// --- Original functions (fixed and optimized) ---

// Function: rng
uint rng(void) {
  uint temp_val = a ^ (a << 7);
  a = b;
  b = c;
  c = d;
  d = (temp_val >> 19) ^ temp_val ^ (d >> 8) ^ d;
  return d;
}

// Function: random_password
char *random_password(void) {
  char *password_buffer = (char *)malloc(8 * sizeof(char)); // Allocate 7 chars + null terminator
  if (password_buffer == NULL) {
    return NULL;
  }
  
  password_buffer[7] = '\0'; // Null-terminate the string
  for (int i = 0; i < 7; ++i) {
    uint r_main = rng(); // First random number to determine type
    
    if (r_main % 5 == 0) { // Condition `uVar2 == (uVar2 / 5) * 5` is equivalent to `uVar2 % 5 == 0`
      uint r_digit = rng(); // Second random number for the character itself
      // Original calculation: ((char)uVar2 - ((char)(uVar2 / 0x11 << 4) + (char)(uVar2 / 0x11))) + '0'
      // This simplifies to (uVar2 % 17) + '0'. This generates characters from '0' to 'P' (ASCII 48-64).
      password_buffer[i] = (r_digit % 17) + '0'; 
    } else {
      uint r_case_decider = rng(); // Second random number to decide case
      if ((r_case_decider & 1) == 0) { // Even: lowercase
        uint r_lower = rng(); // Third random number for lowercase character
        // Original calculation: (char)uVar2 + (char)(uVar2 / 0x1a) * -0x1a + 'a'
        // This simplifies to (uVar2 % 26) + 'a'. Generates 'a' through 'z'.
        password_buffer[i] = (r_lower % 26) + 'a';
      } else { // Odd: uppercase
        uint r_upper = rng(); // Third random number for uppercase character
        // Original calculation: (char)uVar2 + (char)(uVar2 / 0x1a) * -0x1a + 'A'
        // This simplifies to (uVar2 % 26) + 'A'. Generates 'A' through 'Z'.
        password_buffer[i] = (r_upper % 26) + 'A';
      }
    }
  }
  return password_buffer;
}

// Function: list_movies
void list_movies(void) {
  list_all_movies();
} // Return statement is implicit for void functions

// Function: user_rent_movie
void user_rent_movie(void) {
  char input_buffer[1024];
  uint num_owned_movies;
  uint movie_choice = 0; // Initialize to 0 to ensure loop entry
  
  num_owned_movies = get_num_owned_movies();
  list_owned_movies();
  if (num_owned_movies == 0) {
    fdprintf(STDOUT_FILENO,"[ERROR] Movie list is empty. Please try again later.\n");
  } else {
    while ((movie_choice == 0) || (movie_choice > num_owned_movies)) { // Loop until valid ID
      fdprintf(STDOUT_FILENO,"Enter movie id: ");
      readline(STDIN_FILENO, input_buffer, sizeof(input_buffer));
      movie_choice = strtol(input_buffer, NULL, 10);
      if ((movie_choice == 0) || (movie_choice > num_owned_movies)) {
        fdprintf(STDOUT_FILENO,"[ERROR] Invalid movie id. Try again.\n");
      }
    }
    rent_movie(movie_choice);
  }
}

// Function: user_return_movie
void user_return_movie(void) {
  char input_buffer[1024];
  uint num_rented_movies;
  uint movie_choice = 0; // Initialize to 0 to ensure loop entry
  
  num_rented_movies = get_num_rented_movies();
  list_rented_movies();
  if (num_rented_movies == 0) {
    fdprintf(STDOUT_FILENO,"[ERROR] All the movies are in our inventory.\n");
  } else {
    while ((movie_choice == 0) || (movie_choice > num_rented_movies)) { // Loop until valid ID
      fdprintf(STDOUT_FILENO,"Enter movie id: ");
      readline(STDIN_FILENO, input_buffer, sizeof(input_buffer));
      movie_choice = strtol(input_buffer, NULL, 10);
      if ((movie_choice == 0) || (movie_choice > num_rented_movies)) {
        fdprintf(STDOUT_FILENO,"[ERROR] Invalid movie id. Try again.\n");
      }
    }
    return_movie(movie_choice);
  }
}

// Function: admin_login
int admin_login(void) {
  char input_buffer[1028]; // Buffer size as in original snippet (0x400 + some margin)
  
  fdprintf(STDOUT_FILENO,"username: ");
  readline(STDIN_FILENO, input_buffer, sizeof(input_buffer));
  if (strcmp(input_buffer, admin_username) == 0) {
    fdprintf(STDOUT_FILENO,"password: ");
    readline(STDIN_FILENO, input_buffer, sizeof(input_buffer));
    if (strcmp(input_buffer, admin_password) == 0) {
      return 0; // Success
    } else {
      fdprintf(STDOUT_FILENO,"[ERROR] Permission Denied: Wrong credentials\n");
      return -1; // Represents 0xffffffff
    }
  } else {
    fdprintf(STDOUT_FILENO,"[ERROR] Permission Denied: Wrong credentials\n");
    return -1;
  }
}

// Function: run_user_mode
// param_1 (undefined4 *) becomes admin_mode_flag (int *)
int run_user_mode(int *admin_mode_flag) {
  char input_buffer[1024]; // Buffer size 0x400
  
  if (admin_password == NULL) { // Check if password is not yet generated
    admin_password = random_password();
    if (admin_password == NULL) {
        fdprintf(STDERR_FILENO, "[ERROR] Failed to generate admin password. Exiting.\n");
        return 1; // Critical error, signal to exit
    }
    fdprintf(STDOUT_FILENO, "[INFO] Admin password generated: %s (for demonstration purposes)\n", admin_password);
  }
  
  fdprintf(STDOUT_FILENO,"1. List movies\n");
  fdprintf(STDOUT_FILENO,"2. Rent movie\n");
  fdprintf(STDOUT_FILENO,"3. Return movie\n");
  fdprintf(STDOUT_FILENO,"4. Admin mode\n");
  fdprintf(STDOUT_FILENO,"5. Exit\n\n");
  fdprintf(STDOUT_FILENO,"Choice: ");
  readline(STDIN_FILENO, input_buffer, sizeof(input_buffer));
  long choice = strtol(input_buffer, NULL, 10); // Directly use strtol result
  
  switch(choice) {
  default:
    fdprintf(STDOUT_FILENO,"[ERROR] Invalid menu. Please select again.\n");
    break;
  case 1:
    list_movies();
    break;
  case 2:
    user_rent_movie();
    break;
  case 3:
    user_return_movie();
    break;
  case 4:
    if (admin_login() == 0) { // Call admin_login directly in condition
      *admin_mode_flag = 1; // Set the flag if login is successful
    }
    break;
  case 5:
    return 1; // Signal to the main loop to exit the program
  }
  return 0; // Signal to the main loop to continue
}

// Main function to create a compilable and runnable program
int main() {
    int admin_mode_active = 0; // Flag to track if admin mode is currently active
    int exit_program = 0;      // Flag to signal program termination

    // Main program loop
    while (!exit_program) {
        if (admin_mode_active) {
            fdprintf(STDOUT_FILENO, "\n--- Admin Mode Menu ---\n");
            fdprintf(STDOUT_FILENO, "1. Admin Specific Option A (Stub)\n");
            fdprintf(STDOUT_FILENO, "2. Admin Specific Option B (Stub)\n");
            fdprintf(STDOUT_FILENO, "3. Exit Admin Mode\n");
            fdprintf(STDOUT_FILENO, "Choice: ");
            char admin_choice_buffer[64];
            readline(STDIN_FILENO, admin_choice_buffer, sizeof(admin_choice_buffer));
            long admin_choice = strtol(admin_choice_buffer, NULL, 10);

            switch (admin_choice) {
                case 1: fdprintf(STDOUT_FILENO, "[STUB] Admin Option A selected.\n"); break;
                case 2: fdprintf(STDOUT_FILENO, "[STUB] Admin Option B selected.\n"); break;
                case 3: admin_mode_active = 0; fdprintf(STDOUT_FILENO, "Exiting Admin Mode.\n"); break;
                default: fdprintf(STDOUT_FILENO, "[ERROR] Invalid admin menu choice. Please try again.\n"); break;
            }
        } else {
            // Run user mode, which might activate admin mode or request exit
            exit_program = run_user_mode(&admin_mode_active);
        }
    }

    // Cleanup: Free dynamically allocated admin password if it was set
    if (admin_password != NULL) {
        free(admin_password);
        admin_password = NULL;
    }

    fdprintf(STDOUT_FILENO, "Exiting program. Goodbye!\n");
    return 0;
}