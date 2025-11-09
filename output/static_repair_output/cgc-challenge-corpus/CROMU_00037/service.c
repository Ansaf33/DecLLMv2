#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

const char *DAT_000131fb = "Enter your choice: ";
const char *MSG_INVALID_OPTION = "Invalid option.\n";
const char *MSG_EXITING = "Exiting...\n";

int g_isAdmin = 0;
int g_isLoggedIn = -1;

void print(const char *s) {
    printf("%s", s);
}

void memcopy(void *dest, const void *src, size_t n) {
    memcpy(dest, src, n);
}

void sort_n_sum(void *data, size_t size) {
}

void zero() {
}

int read_until(char *buffer, int size, char delimiter) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        return 0;
    }
    return -1;
}

void _terminate() {
    exit(0);
}

void PrintAdminMenu(void); // Forward declaration for BusyWork

void SendBroadcastMessage(void) {
  print("Sending broadcast message...\n");
}

void PrintNewMessages(void) {
  print("--- New Messages ---\n");
  print("You have no new messages.\n");
  return;
}

void CreateUser(void) {
  print("Creating user...\n");
  print("User created successfully!\n");
}

void Login(void) {
  print("Logging in...\n");
  g_isLoggedIn = 0;
  static bool first_login_as_admin = true;
  if (first_login_as_admin) {
      g_isAdmin = 1;
      first_login_as_admin = false;
      print("Logged in successfully as admin!\n");
  } else {
      print("Logged in successfully!\n");
  }
}

void Logout(void) {
  print("Logging out...\n");
  g_isLoggedIn = -1;
  g_isAdmin = 0;
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

// Function: PrintLoggedOutMenu
void PrintLoggedOutMenu(void) {
  print("1) Create User\n");
  print("2) Login\n");
  print("3) Exit\n");
  print(DAT_000131fb);
  return;
}

// Function: PrintLoggedInMenu
void PrintLoggedInMenu(void) {
  print("1) Send Message\n");
  print("2) Read Message\n");
  print("3) List Messages\n");
  print("4) Delete Message\n");
  print("5) Logout\n");
  print("6) Exit\n");
  print(DAT_000131fb);
  return;
}

// Function: PrintAdminMenu
void PrintAdminMenu(void) {
  print("--- Admin Menu ---\n");
  print("1) Send Broadcast Message\n");
  print("2) Revoke Admin Status\n");
  print("3) Exit Program\n");
  print(DAT_000131fb);
  return;
}

typedef void (*menu_action_func)(void);
menu_action_func logged_in_menu_actions[] = {
    SendBroadcastMessage,
    ReadMessage,
    ListMessages,
    DeleteMessage,
    Logout,
    _terminate
};
const int LOGGED_IN_MENU_OPTIONS_COUNT = sizeof(logged_in_menu_actions) / sizeof(logged_in_menu_actions[0]);

// Function: BusyWork
void BusyWork(void) {
  unsigned char local_1014[4096];
  void (*local_14)(void);
  int local_10;
  
  local_14 = PrintAdminMenu;
  local_10 = 20;
  while (local_10 != 0) {
    local_10 = local_10 - 1;
    memcpy(local_1014, (void*)local_14, 0x1000);
    sort_n_sum(local_1014, 0x1000);
  }
  return;
}

// Function: main
int main(void) {
    bool running = true;
    char input_buffer[100];
    int read_status;

    BusyWork();

    while (running) {
        zero();

        if (g_isAdmin != 0) {
            PrintAdminMenu();
            read_status = read_until(input_buffer, sizeof(input_buffer), '\n');
            if (read_status == -1) {
                _terminate();
            }

            if (strlen(input_buffer) < 2) {
                char choice = input_buffer[0];
                if (choice == '1') {
                    SendBroadcastMessage();
                } else if (choice == '2') {
                    g_isAdmin = 0;
                    print("Admin status revoked.\n");
                } else if (choice == '3') {
                    print(MSG_EXITING);
                    running = false;
                } else {
                    print(MSG_INVALID_OPTION);
                }
            } else {
                print(MSG_INVALID_OPTION);
            }
        } else if (g_isLoggedIn != -1) {
            PrintNewMessages();
            PrintLoggedInMenu();
            read_status = read_until(input_buffer, sizeof(input_buffer), '\n');
            if (read_status == -1) {
                _terminate();
            }

            if (strlen(input_buffer) < 2) {
                char choice = input_buffer[0];
                int choice_idx = choice - '1';

                if (choice_idx >= 0 && choice_idx < LOGGED_IN_MENU_OPTIONS_COUNT) {
                    logged_in_menu_actions[choice_idx]();
                    if (choice == '5') {
                        g_isLoggedIn = -1;
                        g_isAdmin = 0;
                    } else if (choice == '6') {
                        running = false;
                    }
                } else {
                    print(MSG_INVALID_OPTION);
                }
            } else {
                print(MSG_INVALID_OPTION);
            }
        } else {
            PrintLoggedOutMenu();
            read_status = read_until(input_buffer, sizeof(input_buffer), '\n');
            if (read_status == -1) {
                _terminate();
            }

            if (strlen(input_buffer) < 2) {
                char choice = input_buffer[0];
                if (choice == '1') {
                    CreateUser();
                } else if (choice == '2') {
                    Login();
                } else if (choice == '3') {
                    print(MSG_EXITING);
                    running = false;
                } else {
                    print(MSG_INVALID_OPTION);
                }
            } else {
                print(MSG_INVALID_OPTION);
            }
        }
    }
    return 0;
}