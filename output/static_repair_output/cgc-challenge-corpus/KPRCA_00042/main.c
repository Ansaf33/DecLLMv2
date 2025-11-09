#include <stdio.h>   // For vdprintf (often included via stdio.h on Linux) and fgets, stdin
#include <unistd.h>  // For dprintf (POSIX.1-2008)
#include <stdarg.h>  // For va_list
#include <stdlib.h>  // For atoi

// Placeholder for DAT_00016baf, which is a string literal.
const char DAT_00016baf[] = "Select mode (2=user, 1=admin, 0=debug, -1=exit): ";

// Custom fdprintf function using vdprintf for variable arguments to a file descriptor.
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vdprintf(fd, format, args);
    va_end(args);
    return ret;
}

// Placeholder function for movie initialization.
void init_movies() {
    // Add movie initialization logic here.
}

// Placeholder for run_user_mode.
// Returns 0 to continue the main loop, 1 to signal exit.
// Updates the value pointed to by mode_ptr based on user input.
int run_user_mode(int *mode_ptr) {
    char input_buf[16];
    fdprintf(1, "User mode. Enter new mode (2=user, 1=admin, 0=debug, -1=exit): ");
    if (fgets(input_buf, sizeof(input_buf), stdin)) {
        *mode_ptr = atoi(input_buf);
    }
    // Main loop will check *mode_ptr for -1 to exit.
    return 0;
}

// Placeholder for run_admin_mode.
// Returns 0 to continue the main loop, -1 to signal immediate exit from main loop.
// Updates the value pointed to by mode_ptr based on user input.
int run_admin_mode(int *mode_ptr) {
    char input_buf[16];
    fdprintf(1, "Admin mode. Enter new mode (2=user, 1=admin, 0=debug, -1=exit): ");
    if (fgets(input_buf, sizeof(input_buf), stdin)) {
        *mode_ptr = atoi(input_buf);
    }
    // If admin mode explicitly returns a negative value, the main loop exits.
    // This matches the original logic's 'if (iVar1 < 0) { local_14 = 1; }'.
    if (*mode_ptr == -1) {
        return -1; // Indicate exit from admin mode
    }
    return 0; // Continue
}

// Placeholder for run_debug_mode.
// Updates the value pointed to by mode_ptr based on user input.
void run_debug_mode(int *mode_ptr) {
    char input_buf[16];
    fdprintf(1, "Debug mode. Enter new mode (2=user, 1=admin, 0=debug, -1=exit): ");
    if (fgets(input_buf, sizeof(input_buf), stdin)) {
        *mode_ptr = atoi(input_buf);
    }
    // Main loop will check *mode_ptr for -1 to exit.
}

int main(void) {
  int continue_loop = 0; // Renamed from local_14; 0 to continue, 1 to exit.
  int current_mode = 2;  // Renamed from local_18; Initial mode: 2=user, 1=admin, 0=debug.
  
  // local_10 and iVar1 are removed as they were unused or could be optimized out.
  
  init_movies();
  fdprintf(1,"========= Movie Rental Service v0.1 =========\n");
  fdprintf(1,"   . . . Initializing the inventory . . .\n");
  fdprintf(1,"         (movie info from IMDb.com)\n");
  fdprintf(1,"=============================================\n");
  
  while (continue_loop == 0) {
    if (current_mode == 2) { // User mode
      continue_loop = run_user_mode(&current_mode);
    }
    else if (current_mode == 1) { // Admin mode
      if (run_admin_mode(&current_mode) < 0) {
        continue_loop = 1; // Admin mode returned an error/exit signal.
      }
    }
    else if (current_mode == 0) { // Debug mode
      run_debug_mode(&current_mode);
    }
    
    // Check if any mode function explicitly set current_mode to -1 (exit signal).
    if (current_mode == -1) {
        continue_loop = 1;
    }

    if (continue_loop == 0) {
      fdprintf(1, DAT_00016baf); // DAT_00016baf is a string literal, no & needed.
    }
  }
  
  fdprintf(1,"Bye!\n");
  return 0;
}