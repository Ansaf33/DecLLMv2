#include <stdio.h>   // For printf, scanf, fgets, fflush, getchar, feof, ferror, clearerr
#include <stdlib.h>  // For exit
#include <string.h>  // For strcspn

// --- Global Variables ---
// These are declared as global based on their usage in the original snippet.
int msg_count_login = 0;
int user_count = 0;
int bad_login_count = 0;
char current_user[16] = {0}; // Assuming a max username length of 15 characters + null terminator

// --- Function Prototypes (placeholders for external functions) ---
// These functions are referenced in the provided snippet but not defined.
// Their actual implementations would be in other source files or linked libraries.
// Types and arguments are inferred from their usage in the snippet.
void create_user(void);
void list_users(void);
// Assuming login takes username and password, and returns 1 on success, 0 on failure.
// It is also assumed to internally set the 'current_user' global variable on success.
int login(const char* username, const char* password);
void logout(void);
int get_total_count(void);
// Assuming these functions operate on a specific user, hence the 'user' argument.
int get_draft_count(const char* user);
int get_inbox_count(const char* user);
void print_draft_for_send(const char* user);
// Assuming add_message takes sender, recipient, and message body, returns 1 on success.
int add_message(const char* sender, const char* recipient, const char* message_body);
void list_all_messages(void);
void list_drafts(const char* user);
void list_inbox(const char* user);
void print_draft_for_del(const char* user);

// --- Helper Input Functions ---

// Reads a single non-whitespace character from stdin.
// Skips leading whitespace and consumes the rest of the line up to a newline.
// Returns the character read, or 0 if an error or EOF occurs.
char receive_char(void) {
    char c;
    // Use a space before %c to consume any whitespace characters, including newlines
    // from previous inputs, until a non-whitespace character is found.
    if (scanf(" %c", &c) != 1) {
        // Handle potential EOF or read errors.
        if (feof(stdin) || ferror(stdin)) {
            clearerr(stdin); // Clear error flags
        }
        return 0; // Indicate failure to read a character
    }
    // Clear the remaining input buffer up to the next newline character or EOF.
    // This prevents residual input from affecting subsequent reads.
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    return c;
}

// Reads a line of input into a buffer, up to max_len-1 characters.
// Removes any trailing newline character from the buffer.
// Ensures the buffer is null-terminated.
void receive_string(char* buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        // Handle error or EOF; ensure buffer is empty and null-terminated.
        buffer[0] = '\0';
        return;
    }
    // Remove the trailing newline character if it exists.
    // strcspn finds the length of the initial segment of buffer which consists
    // entirely of characters not in "\n". If a newline is found, its position
    // is returned, and that position is set to null to terminate the string.
    buffer[strcspn(buffer, "\n")] = '\0';
}

// Function: print_A
void print_A(void) {
  printf("******************\n");
  printf("1. Create User\n");
  printf("2. Login\n");
  printf("3. Exit\n");
  printf(": "); // Prompt for input on the same line
  fflush(stdout); // Ensure the prompt is displayed immediately
}

// Function: handle_A_input
// Returns 0 to stay in Menu A, 2 to switch to Menu B (login successful).
int handle_A_input(void) {
  char choice;
  char username[15]; // Buffer for username input
  char password[15]; // Buffer for password input
  
  choice = receive_char(); // Get a single character for menu choice

  if (choice == '1') {
    create_user();
  } else if (choice == '2') {
    list_users();
    printf("Username: ");
    fflush(stdout);
    receive_string(username, sizeof(username)); // Read username into buffer
    
    printf("Password: ");
    fflush(stdout);
    receive_string(password, sizeof(password)); // Read password into buffer
    
    msg_count_login = 0; // Reset message count for a new login session
    if (login(username, password) == 1) {
      // Login successful, assume 'current_user' global is set by 'login' function.
      return 2; // Indicate successful login, transition to Menu B
    }
    
    printf("Bad login.\n");
    bad_login_count++;
    if (bad_login_count > 2) { // If more than 2 bad logins (i.e., 3 or more total)
      create_user(); // Force user creation
      bad_login_count = 0; // Reset bad login counter
    }
  } else if (choice == '3') {
    printf("The end.\n");
    exit(0); // Terminate the program
  } else {
    printf("unknown input: %c\n", choice); // Report unknown input character
  }
  return 0; // Stay in Menu A (default)
}

// Function: print_B
void print_B(void) {
  printf("********%s********\n", current_user); // Display current logged-in user
  printf("1. Create Message\n");
  printf("2. Send Message\n");
  printf("3. List All [%d]\n", get_total_count()); // Display total message count
  printf("4. List Drafts [%d]\n", get_draft_count(current_user)); // Display user's draft count
  printf("5. List Inbox [%d]\n", get_inbox_count(current_user)); // Display user's inbox count
  printf("6. Delete Draft\n");
  printf("7. Logout\n");
  printf("8. Exit\n");
  printf(": ");
  fflush(stdout); // Ensure the prompt is displayed immediately
}

// Function: handle_B_input
// Returns 0 to stay in Menu B, 2 to switch back to Menu A (logout successful).
int handle_B_input(void) {
  char choice;
  char recipient[15];    // Buffer for recipient username input
  char message_body[15]; // Buffer for message body input
  
  choice = receive_char(); // Get a single character for menu choice

  if (choice == '7') { // Logout
    printf("Logging out of user %s\n", current_user);
    logout();
    current_user[0] = '\0'; // Clear the current user on logout
    return 2; // Indicate logout, transition back to Menu A
  } else if (choice == '8') { // Exit
    printf("The end.\n");
    exit(0); // Terminate the program
  } else if (choice == '2') { // Send Message
    if (get_draft_count(current_user) == 0) {
      printf("Must create draft first.\n");
    } else {
      print_draft_for_send(current_user); // Display drafts for sending
    }
  } else if (choice == '1') { // Create Message
    if (msg_count_login >= 3) { // Check if max messages for session reached (limit of 3)
      printf("Reached maximum messages for this session.\n");
    } else {
      printf("Recipient: ");
      fflush(stdout);
      receive_string(recipient, sizeof(recipient)); // Read recipient username
      
      printf("Message: ");
      fflush(stdout);
      receive_string(message_body, sizeof(message_body)); // Read message body
      
      if (add_message(current_user, recipient, message_body) != 1) {
        printf("add_message failed\n");
      }
      msg_count_login++; // Increment message count for the current session
    }
  } else if (choice == '3') { // List All Messages
    if (get_total_count() == 0) {
      printf("No messages.\n");
    } else {
      list_all_messages();
    }
  } else if (choice == '4') { // List Drafts
    list_drafts(current_user);
  } else if (choice == '5') { // List Inbox
    list_inbox(current_user);
  } else if (choice == '6') { // Delete Draft
    if (get_draft_count(current_user) == 0) {
      printf("Must create draft first.\n");
    } else {
      print_draft_for_del(current_user); // Display drafts for deletion
    }
  } else {
    printf("unknown input: %c\n", choice); // Report unknown input character
  }
  return 0; // Stay in Menu B (default)
}

// Function: main
// The entry point of the program.
// Uses a state-based loop to switch between Menu A (pre-login) and Menu B (post-login).
int main(void) {
  // Initialize global counters and current_user state.
  // Although static storage duration variables are zero-initialized by default,
  // explicit initialization is often good practice.
  msg_count_login = 0;
  user_count = 0;
  bad_login_count = 0;
  current_user[0] = '\0'; // Ensure no user is logged in initially

  int menu_state; // 0 for Menu A, 2 for Menu B (based on handle_A_input return)
  
  do {
    print_A(); // Display Menu A options
    menu_state = handle_A_input(); // Process input for Menu A
    
    // If login was successful (menu_state == 2), enter the Menu B loop.
    while (menu_state == 2) {
      print_B(); // Display Menu B options
      menu_state = handle_B_input(); // Process input for Menu B
      // If handle_B_input returns 2 (logout), this inner while loop will terminate,
      // and control will return to the outer do-while loop, effectively going back to Menu A.
    }
  } while(1); // This loop runs indefinitely until 'exit(0)' is called from a handler.
  
  // This return statement will theoretically not be reached as exit(0) is used for termination.
  return 0;
}