#include <stdio.h>    // For printf, fprintf, scanf, getchar
#include <string.h>   // For memset
#include <unistd.h>   // For read, STDIN_FILENO, ssize_t
#include <stdlib.h>   // For srand, rand
#include <time.h>     // For time

// --- Global Variables (renamed for clarity and C-style, initialized to 0) ---
int total_won = 0;
int total_lost = 0;
int eq_solved_correctly = 0;
int eq_solved_incorrectly = 0;
int eq_created_correctly = 0;
int eq_created_incorrectly = 0;
int peri_solved = 0;
int peri_unsolved = 0;
int vol_solved = 0;
int vol_unsolved = 0;
int area_solved = 0;
int area_unsolved = 0;
int game_info_var = 0; // Original: game_info
int unknown_var_1 = 0; // Original: DAT_000190fc
int unknown_var_2 = 0; // Original: DAT_00019100

// --- Placeholder functions (as they were not provided in the snippet) ---

// Reads an integer from stdin for user menu choice
int get_user_answer(void) {
    int choice;
    printf("Enter your choice: ");
    if (scanf("%d", &choice) != 1) {
        // Clear input buffer on invalid input
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return -1; // Indicate error or invalid input
    }
    // Clear remaining input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    return choice;
}

// Seeds the pseudo-random number generator
int seed_prng(void) {
    srand(time(NULL));
    return 1; // Indicate success
}

// Generates a random integer within a specified range (inclusive)
int random_in_range(int min, int max) {
    if (min > max) { // Swap if min is greater than max
        int temp = min;
        min = max;
        max = temp;
    }
    return min + (rand() % (max - min + 1));
}

// Placeholder functions for game logic, returning 1 for success/correct, 0 for failure/incorrect
int generate_equation(void) {
    printf("--- Solving an equation ---\n");
    return random_in_range(0, 1);
}
int prompt_for_equation(void) {
    printf("--- Creating an equation ---\n");
    return random_in_range(0, 1);
}
int request_area(void) {
    printf("--- Requesting area calculation (type 1) ---\n");
    return random_in_range(0, 1);
}
int request_2d_area_dim(void) {
    printf("--- Requesting area calculation (type 2) ---\n");
    return random_in_range(0, 1);
}
int request_perimeter(void) {
    printf("--- Requesting perimeter calculation (type 1) ---\n");
    return random_in_range(0, 1);
}
int request_2d_peri_dim(void) {
    printf("--- Requesting perimeter calculation (type 2) ---\n");
    return random_in_range(0, 1);
}
int request_volume(void) {
    printf("--- Requesting volume calculation (type 1) ---\n");
    return random_in_range(0, 1);
}
int request_3d_dim_vol(void) {
    printf("--- Requesting volume calculation (type 2) ---\n");
    return random_in_range(0, 1);
}

// Function: print_menu
int print_menu(void) {
  printf("\n--- Main Menu ---\n");
  printf("1. Solve Equation\n");
  printf("2. Create Equation\n");
  printf("3. Solve Area\n");
  printf("4. Solve Perimeter\n");
  printf("5. Solve Volume\n");
  printf("6. View Statistics\n");
  printf("7. Exit\n");
  return get_user_answer(); // Directly return the user's choice
}

// Function: print_statistics
void print_statistics(void) {
  char name_buffer[36]; // Buffer for user name input
  
  printf("\n--- Game Statistics ---\n");
  printf("Total won: %d\n", total_won);
  printf("Total lost: %d\n", total_lost);
  printf("Equations solved correctly: %d\n", eq_solved_correctly);
  printf("Equations solved incorrectly: %d\n", eq_solved_incorrectly);
  printf("Equations created correctly: %d\n", eq_created_correctly);
  printf("Equations created incorrectly: %d\n", eq_created_incorrectly);
  printf("Solved perimeter problems: %d\n", peri_solved);
  printf("Unsolved perimeter problems: %d\n", peri_unsolved);
  printf("Solved volume problems: %d\n", vol_solved);
  printf("Unsolved volume problems: %d\n", vol_unsolved);
  printf("Solved area problems: %d\n", area_solved);
  printf("Unsolved area problems: %d\n", area_unsolved);

  // Check for achievements and prompt for name
  if (eq_created_correctly == 20) {
    printf("Wow, you created 20 equations! What\'s your name?: ");
    memset(name_buffer, 0, sizeof(name_buffer)); // Clear buffer
    // Read user input, preventing buffer overflow by limiting size
    ssize_t bytes_read = read(STDIN_FILENO, name_buffer, sizeof(name_buffer) - 1);
    if (bytes_read > 0) {
        name_buffer[bytes_read] = '\0'; // Null-terminate the string
        // Remove trailing newline if present from read
        if (name_buffer[bytes_read - 1] == '\n') {
            name_buffer[bytes_read - 1] = '\0';
        }
    } else {
        name_buffer[0] = '\0'; // Ensure it's an empty string on error/no input
    }
    printf("Good job, %s!\n", name_buffer);
  }
  if (eq_solved_correctly == 20) {
    printf("Wow, you solved 20 equations! What\'s your name?: ");
    memset(name_buffer, 0, sizeof(name_buffer));
    ssize_t bytes_read = read(STDIN_FILENO, name_buffer, sizeof(name_buffer) - 1);
    if (bytes_read > 0) {
        name_buffer[bytes_read] = '\0';
        if (name_buffer[bytes_read - 1] == '\n') {
            name_buffer[bytes_read - 1] = '\0';
        }
    } else {
        name_buffer[0] = '\0';
    }
    printf("Good job, %s!\n", name_buffer);
  }
  if (area_solved == 20) {
    printf("Wow, you solved 20 area problems! What\'s your name?: ");
    memset(name_buffer, 0, sizeof(name_buffer));
    ssize_t bytes_read = read(STDIN_FILENO, name_buffer, sizeof(name_buffer) - 1);
    if (bytes_read > 0) {
        name_buffer[bytes_read] = '\0';
        if (name_buffer[bytes_read - 1] == '\n') {
            name_buffer[bytes_read - 1] = '\0';
        }
    } else {
        name_buffer[0] = '\0';
    }
    printf("Good job, %s!\n", name_buffer);
  }
  printf("-------------------------\n");
}

// Function: main
int main(void) {
  // All global variables are already initialized to 0 at declaration.
  // The original loop and explicit assignments are redundant here.

  if (seed_prng() != 1) {
    fprintf(stderr, "Failed to seed PRNG. Exiting.\n");
    return 1;
  }
  
  int choice;
  int result;
  do {
    choice = print_menu();
    switch(choice) {
    case 1:
      result = generate_equation();
      if (result == 1) {
        eq_solved_correctly++;
      } else {
        eq_solved_incorrectly++;
      }
      break;
    case 2:
      result = prompt_for_equation();
      if (result == 1) {
        eq_created_correctly++;
      } else {
        eq_created_incorrectly++;
      }
      break;
    case 3:
      result = random_in_range(0,1);
      if (result == 0) { // If random_in_range returns 0
        result = request_area();
      } else { // If random_in_range returns 1
        result = request_2d_area_dim();
      }
      if (result == 1) {
        area_solved++;
      } else {
        area_unsolved++;
      }
      break;
    case 4:
      // 0x2a4 is 676, 0x2a5 is 677
      result = random_in_range(0x2a4, 0x2a5);
      if (result == 0x2a4) { // If random_in_range returns 676
        result = request_perimeter();
      } else { // If random_in_range returns 677
        result = request_2d_peri_dim();
      }
      if (result == 1) {
        peri_solved++;
      } else {
        peri_unsolved++;
      }
      break;
    case 5:
      // 0x2c is 44, 0x2d is 45
      result = random_in_range(0x2c, 0x2d);
      if (result == 0x2d) { // If random_in_range returns 45
        result = request_volume();
      } else { // If random_in_range returns 44
        result = request_3d_dim_vol();
      }
      if (result == 1) {
        vol_solved++;
      } else {
        vol_unsolved++;
      }
      break;
    case 6:
      print_statistics();
      break;
    case 7:
      printf("Exiting program. Goodbye!\n");
      return 0; // Exit the program
    default:
      printf("Invalid input. Please enter a number between 1 and 7.\n");
      // Loop continues, prompting for input again
      break;
    }
  } while(1); // Loop indefinitely until case 7 or default exit
}