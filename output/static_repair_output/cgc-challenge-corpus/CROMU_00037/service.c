#include <stdio.h>    // For printf, NULL, stdin, fgets
#include <stdlib.h>   // For exit, size_t
#include <string.h>   // For strlen, memcpy, memset, strcspn
#include <stdbool.h>  // For bool type

// --- Global Data / Constants ---
// DAT_000131fb is likely a prompt string, e.g., "> "
const char *DAT_000131fb = "> ";

// String constants identified from original code's offsets
const char *MSG_EXITING = "Exiting...\n";
const char *MSG_INVALID_OPTION = "Invalid option.\n";

// --- Placeholder Functions (Original functions not provided, so mock them) ---

// Assuming 'print' is printf
#define print printf

// Ghidra's 'code' type mapped to a standard C function pointer type
typedef void (*code_ptr)(void);

// Function declarations for functions used before their definitions
void PrintAdminMenu(void);
void SendBroadcastMessage(void);
void PrintNewMessages(void);
void CreateUser(void);
void Login(void);
void ReadMessage(void);
void ListMessages(void);
void DeleteMessage(void);
void Logout(void);

// Placeholder for sort_n_sum
// Assumed to take a buffer and its size, and perform some operation
void sort_n_sum(char *buffer, size_t size) {
    // Mock implementation: do nothing or a simple sum to simulate busy work
    volatile int sum = 0;
    for (size_t i = 0; i < size; ++i) {
        sum += buffer[i];
    }
}

// Placeholder for zero (likely memset to zero)
void zero(char *buffer, size_t size) {
    memset(buffer, 0, size);
}

// Placeholder for read_until
// Reads up to 'max_len' characters into 'buffer' until a newline or EOF.
// Returns 0 on success, -1 on EOF/error.
int read_until(char *buffer, size_t max_len, char terminator) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1; // EOF or error
    }
    // Remove trailing newline if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 0; // Success
}

// Placeholder for _terminate
void _terminate(void) {
    print("Terminating due to error or EOF...\n");
    exit(1);
}

// --- Application State Management ---
// Original code used offsets into a base address (iVar6) for state.
// We model this with a global struct.
struct AppState {
    bool is_logged_in;
    bool is_admin; // Corresponds to *(int *)(iVar6 + 0x2e8f) != -1
    bool exit_program;
};

// Global application state, initialized to logged out and not admin.
struct AppState app_state = {
    .is_logged_in = false,
    .is_admin = false,
    .exit_program = false
};

// --- Function: PrintLoggedOutMenu ---
void PrintLoggedOutMenu(void) {
  print("1) Create User\n");
  print("2) Login\n");
  print("3) Exit\n");
  print(DAT_000131fb);
}

// --- Function: PrintLoggedInMenu ---
void PrintLoggedInMenu(void) {
  print("1) Send Message\n");
  print("2) Read Message\n");
  print("3) List Messages\n");
  print("4) Delete Message\n");
  print("5) Logout\n");
  print("6) Exit\n");
  print(DAT_000131fb);
}

// --- Function: BusyWork ---
void BusyWork(void) {
  char local_buffer[4096]; // Renamed from local_1014 for clarity
  code_ptr func_ptr_to_admin_menu = PrintAdminMenu; // Renamed from local_14
  int counter = 20; // Renamed from local_10 (0x14 in hex)
  
  while (counter > 0) {
    counter--;
    // The original code copies 0x1000 (4096) bytes from the address of PrintAdminMenu.
    // This is unusual and platform-dependent, potentially copying function code or data near it.
    // We replicate this behavior as specified by the original snippet.
    memcpy(local_buffer, (const void *)func_ptr_to_admin_menu, 0x1000);
    sort_n_sum(local_buffer, 0x1000);
  }
}

// --- Function: main ---
int main(void) {
  char input_buffer[100]; // Buffer for user input
  size_t input_len;

  // The original low-level stack setup and variable assignments
  // (`local_10 = &stack0x00000004; ppuVar2 = &local_10; do { ... } while (...)`)
  // are replaced by standard C variable declarations and initialization of `app_state`.

  while (!app_state.exit_program) {
    zero(input_buffer, sizeof(input_buffer)); // Clear input buffer before each read

    if (app_state.is_logged_in && app_state.is_admin) {
      // Admin state
      PrintAdminMenu();
      if (read_until(input_buffer, sizeof(input_buffer), '\n') == -1) {
        _terminate();
      }
      input_len = strlen(input_buffer);

      if (input_len < 2) {
        switch (input_buffer[0]) {
          case '1':
            SendBroadcastMessage();
            break;
          case '2': // Logout Admin
            app_state.is_admin = false; // Admin logs out to become a regular logged-in user
            print("Admin privileges removed. You are now a regular logged-in user.\n");
            break;
          case '3': // Exit
            print(MSG_EXITING);
            app_state.exit_program = true;
            break;
          default:
            print(MSG_INVALID_OPTION);
            break;
        }
      } else {
        print(MSG_INVALID_OPTION);
      }
    } else if (app_state.is_logged_in) {
      // Logged-in state (not admin)
      PrintNewMessages(); // Appears before menu in original logged-in state logic
      PrintLoggedInMenu();
      if (read_until(input_buffer, sizeof(input_buffer), '\n') == -1) {
        _terminate();
      }
      input_len = strlen(input_buffer);

      if (input_len < 2) {
        // The original code had a jump table based on input_buffer[0] - '1'.
        // This is replaced with a clear switch statement.
        switch (input_buffer[0]) {
          case '1':
            SendBroadcastMessage(); // This might be "Send Message" rather than "Broadcast"
            break;
          case '2':
            ReadMessage();
            break;
          case '3':
            ListMessages();
            break;
          case '4':
            DeleteMessage();
            break;
          case '5': // Logout
            Logout();
            app_state.is_logged_in = false;
            print("Logged out successfully.\n");
            break;
          case '6': // Exit
            print(MSG_EXITING);
            app_state.exit_program = true;
            break;
          default:
            print(MSG_INVALID_OPTION);
            break;
        }
      } else {
        print(MSG_INVALID_OPTION);
      }
    } else {
      // Logged-out state
      PrintLoggedOutMenu();
      if (read_until(input_buffer, sizeof(input_buffer), '\n') == -1) {
        _terminate();
      }
      input_len = strlen(input_buffer);

      if (input_len < 2) {
        switch (input_buffer[0]) {
          case '1':
            CreateUser();
            break;
          case '2': // Login
            Login();
            // Simulate successful login. The original snippet doesn't provide
            // logic for distinguishing regular vs. admin login.
            // For now, assume a regular login.
            app_state.is_logged_in = true;
            print("Logged in successfully.\n");
            break;
          case '3': // Exit
            print(MSG_EXITING);
            app_state.exit_program = true;
            break;
          default:
            print(MSG_INVALID_OPTION);
            break;
        }
      } else {
        print(MSG_INVALID_OPTION);
      }
    }
  }

  return 0; // Successful exit
}

// --- Placeholder function definitions ---

void PrintAdminMenu(void) {
    print("Admin Menu:\n");
    print("1) Send Broadcast Message\n");
    print("2) Logout Admin\n");
    print("3) Exit\n");
    print(DAT_000131fb);
}

void SendBroadcastMessage(void) {
    print("Sending broadcast message...\n");
}

void PrintNewMessages(void) {
    print("No new messages.\n");
}

void CreateUser(void) {
    print("Creating user...\n");
    // Add logic for user creation here.
}

void Login(void) {
    print("Attempting login...\n");
    // Add logic for user authentication here.
    // This function could set `app_state.is_admin = true;` if admin credentials are provided.
    // For this refactor, it just prepares for `app_state.is_logged_in = true;` in main.
}

void ReadMessage(void) {
    print("Reading message...\n");
}

void ListMessages(void) {
    print("Listing messages...\n");
}

void DeleteMessage(void) {
    print("Deleting message...\n");
}

void Logout(void) {
    print("User logging out...\n");
}