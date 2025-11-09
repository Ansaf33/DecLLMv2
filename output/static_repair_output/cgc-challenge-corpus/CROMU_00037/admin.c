#include <stdio.h>   // For printf, fgets
#include <stdlib.h>  // For exit, atoi
#include <string.h>  // For strlen, strcmp, strcpy, memset, strncpy

// --- Global variables (placeholders based on decompiled code patterns) ---
// These definitions are necessary to make the code compilable and runnable.
// Their actual values/structure depend on the full original program.

// DAT_00013039: A string printed in PrintAdminMenu
const char *DAT_00013039 = "Enter your choice: ";

// ADMIN_PASSWD: The hardcoded admin password
const char *ADMIN_PASSWD = "supersecretpassword";

// ADMIN_ACCESS: Flag indicating if admin access is currently granted
int ADMIN_ACCESS = 0;

// NUM_USERS: Total number of users supported by the system
#define MAX_SYSTEM_USERS 10
int NUM_USERS = MAX_SYSTEM_USERS;

// User structure memory layout constants, derived from decompiled offsets
#define USER_STRUCT_SIZE            0x32e4 // 13028 bytes per user block
#define USER_ACTIVE_OFFSET          0x0    // Offset for the 'active' status byte
#define USER_MESSAGES_START_OFFSET  0x10   // Offset where messages array starts
#define MESSAGE_BUFFER_SIZE         0x32   // 50 bytes per message buffer (including null terminator)
#define USER_MESSAGE_FLAGS_OFFSET   0x31de // Offset for an array of message flags/statuses
#define USER_MESSAGE_COUNT_OFFSET   0x32e0 // Offset for the message count (int)

// Max messages per user: Calculated based on available space for messages
// from 0x10 to 0x31de. (0x31de - 0x10) / MESSAGE_BUFFER_SIZE = 12750 / 50 = 255.
// This also aligns with a potential '0xff' check for a char-sized count.
#define MAX_MESSAGES_PER_USER       255

// Raw memory block to simulate an array of user structures
// This approach directly maps the memory access patterns from the decompiled code,
// avoiding potential issues with C struct padding.
char USERS_RAW_DATA[MAX_SYSTEM_USERS][USER_STRUCT_SIZE];

// --- Helper macros to access user fields within the raw data block ---
// These macros cast the `char*` base pointer to the appropriate type at the correct offset.
#define GET_USER_BASE(user_idx)                 (USERS_RAW_DATA[user_idx])
#define GET_USER_ACTIVE(user_idx)               (*(char *)(GET_USER_BASE(user_idx) + USER_ACTIVE_OFFSET))
#define GET_USER_MESSAGE_COUNT(user_idx)        (*(int *)(GET_USER_BASE(user_idx) + USER_MESSAGE_COUNT_OFFSET))
#define GET_USER_MESSAGE_BUFFER(user_idx, msg_idx) \
    ((char *)(GET_USER_BASE(user_idx) + USER_MESSAGES_START_OFFSET + (msg_idx) * MESSAGE_BUFFER_SIZE))
#define GET_USER_MESSAGE_FLAG(user_idx, msg_idx) \
    (*(char *)(GET_USER_BASE(user_idx) + USER_MESSAGE_FLAGS_OFFSET + (msg_idx)))

// --- Replacement for custom functions found in the decompiled snippet ---

// Replaces 'print'
void print_str(const char *str) {
    printf("%s", str);
}

// Replaces 'zero' (for buffers)
void zero_buffer(char *buffer, size_t size) {
    memset(buffer, 0, size);
}

// Replaces 'read_until' (reads up to size-1 characters or until newline)
// Returns length read, or -1 on error/EOF. Removes trailing newline.
int read_input(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        return -1; // Error or EOF
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    }
    return len;
}

// Replaces '_terminate'
void terminate_program() {
    exit(1);
}

// Replaces 'strmatch'
int string_match(const char *s1, const char *s2) {
    return (strcmp(s1, s2) == 0);
}

// Replaces 'strcopy'
void string_copy(char *dest, const char *src) {
    strncpy(dest, src, MESSAGE_BUFFER_SIZE - 1); // Ensure buffer overflow protection
    dest[MESSAGE_BUFFER_SIZE - 1] = '\0';        // Null-terminate the destination
}

// --- Initialization for USERS_RAW_DATA ---
void init_users() {
    for (int i = 0; i < NUM_USERS; ++i) {
        // Zero out the entire memory block for the user
        memset(GET_USER_BASE(i), 0, USER_STRUCT_SIZE);
        GET_USER_ACTIVE(i) = (i % 2 == 0) ? 1 : 0; // Set some users as active
        GET_USER_MESSAGE_COUNT(i) = 0;             // Initialize message count
    }
    // Ensure at least one user is active and has an empty mailbox for testing
    if (NUM_USERS > 0) {
        GET_USER_ACTIVE(0) = 1;
        GET_USER_MESSAGE_COUNT(0) = 0;
    }
    // Simulate a full mailbox for a specific user for testing
    if (NUM_USERS > 1) {
        GET_USER_ACTIVE(1) = 1;
        GET_USER_MESSAGE_COUNT(1) = MAX_MESSAGES_PER_USER;
    }
}

// --- Fixed functions ---

// Function: PrintAdminMenu
void PrintAdminMenu(void) {
  print_str("1) Send Broadcast Message\n");
  print_str("2) Leave Admin Mode\n");
  print_str("3) Exit\n");
  print_str(DAT_00013039);
}

// Function: AdminLogin
void AdminLogin(void) {
  char password_buffer[104];
  
  zero_buffer(password_buffer, sizeof(password_buffer));
  print_str("Admin Password: ");
  
  if (read_input(password_buffer, sizeof(password_buffer)) == -1) {
    terminate_program();
  }

  if (string_match(password_buffer, ADMIN_PASSWD) == 0) { // 0 means not a match
    print_str("[-] Admin access denied\n");
    ADMIN_ACCESS = 0;
  } else {
    ADMIN_ACCESS = 1;
    print_str("[-] Admin access granted\n");
  }
}

// Function: SendBroadcastMessage
void SendBroadcastMessage(void) {
  char message_buffer[1024];
  
  zero_buffer(message_buffer, sizeof(message_buffer));
  print_str("Message: ");
  
  if (read_input(message_buffer, sizeof(message_buffer)) == -1) {
    terminate_program();
  }

  if (strlen(message_buffer) != 0) {
    for (int i = 0; i < NUM_USERS; i++) {
      if (GET_USER_ACTIVE(i) != '\0') { // Check if user is active
        // The original code checked for '0xff'. Assuming this implies mailbox full.
        // Using >= MAX_MESSAGES_PER_USER is a more robust check for a count.
        if (GET_USER_MESSAGE_COUNT(i) >= MAX_MESSAGES_PER_USER) {
          print_str("[*] Unable to send message to ");
          printf("User %d", i); // Placeholder for user identification
          print_str(". Their mailbox is full.\n");
        } else {
          int next_message_idx = GET_USER_MESSAGE_COUNT(i); // Use current count as index for new message
          
          string_copy(GET_USER_MESSAGE_BUFFER(i, next_message_idx), message_buffer);
          
          GET_USER_MESSAGE_FLAG(i, next_message_idx) = 0; // Set a flag for the new message
          
          GET_USER_MESSAGE_COUNT(i)++; // Increment message count
        }
      }
    }
  }
}

// --- Main function to demonstrate usage ---
int main() {
    init_users(); // Initialize user data

    char choice_buffer[10];

    while (1) {
        if (!ADMIN_ACCESS) {
            print_str("\n--- Login to Admin Mode ---\n");
            AdminLogin();
            if (!ADMIN_ACCESS) {
                print_str("Login failed. Exiting.\n");
                break;
            }
        }

        PrintAdminMenu();
        if (read_input(choice_buffer, sizeof(choice_buffer)) == -1) {
            print_str("Error reading choice. Exiting.\n");
            break;
        }
        int choice = atoi(choice_buffer);

        switch (choice) {
            case 1:
                SendBroadcastMessage();
                break;
            case 2:
                ADMIN_ACCESS = 0;
                print_str("Left Admin Mode.\n");
                break;
            case 3:
                print_str("Exiting.\n");
                return 0;
            default:
                print_str("Invalid choice.\n");
                break;
        }
    }

    return 0;
}