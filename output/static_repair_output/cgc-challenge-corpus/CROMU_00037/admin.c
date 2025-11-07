#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h> // For offsetof, though not strictly used in final code

// Global variables (guessed based on usage and common patterns)
const char* ADMIN_PASSWD = "supersecretpassword"; // Placeholder
int ADMIN_ACCESS = 0; // 0 for false, 1 for true
const char* DAT_00013039 = "4) Log out\n"; // Placeholder for a string literal
int NUM_USERS = 5; // Placeholder for number of users

// User structure definition based on observed access patterns and logical consistency.
// The original offsets from the decompiler were contradictory, so this prioritizes
// logical correctness for compilation over strict byte-for-byte offset matching.
#define MAX_USERNAME_LEN 15 // To fit within the first 0x10 bytes with null terminator
#define MESSAGE_SLOT_SIZE 50 // 0x32 bytes as observed in original code
#define MAX_MESSAGES_PER_USER 255 // Max messages, allowing indices 0-254.
                                  // The 0xff check for full capacity (255) is interpreted
                                  // as `message_count == MAX_MESSAGES_PER_USER`.

typedef struct User {
    char username[MAX_USERNAME_LEN + 1]; // At offset 0. username[0] implicitly acts as active status.
    char _padding_to_messages[16 - (MAX_USERNAME_LEN + 1)]; // Padding to reach offset 0x10
    char messages[MAX_MESSAGES_PER_USER][MESSAGE_SLOT_SIZE]; // Message storage
    char message_flags[MAX_MESSAGES_PER_USER]; // Flags for each message (e.g., read/unread)
    int message_count; // Number of messages currently stored (0 to MAX_MESSAGES_PER_USER)
} User;

User USERS[5]; // Array of users

// Custom function implementations to replace decompiler's artifacts
void print_str(const char* s) {
    printf("%s", s);
}

void zero_buffer(char* buffer, size_t size) {
    memset(buffer, 0, size);
}

int read_until_newline(char* buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1; // Error or EOF
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return len; // Return length read (including null terminator if newline removed)
}

void terminate_program() {
    exit(1);
}

int string_match(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0 ? 1 : 0; // 0 for no match, 1 for match
}

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
  
  // The original decompiler used 100 as max_len, but buffer is 104.
  // Using sizeof(password_buffer) - 1 for fgets to prevent buffer overflow
  // and ensure space for null terminator.
  if (read_until_newline(password_buffer, sizeof(password_buffer) - 1) == -1) {
    terminate_program();
  }
  
  if (string_match(password_buffer, ADMIN_PASSWD) == 0) {
    print_str("[-] Admin access denied\n");
    ADMIN_ACCESS = 0;
  }
  else {
    ADMIN_ACCESS = 1;
    print_str("[-] Admin access granted\n");
  }
}

// Function: SendBroadcastMessage
void SendBroadcastMessage(void) {
  char message_buffer[1024];
  
  zero_buffer(message_buffer, sizeof(message_buffer));
  print_str("Message: ");
  
  // Max length for read_until_newline should be `sizeof(buffer) - 1`
  if (read_until_newline(message_buffer, sizeof(message_buffer) - 1) == -1) {
    terminate_program();
  }
  
  if (strlen(message_buffer) != 0) {
    for (int i = 0; i < NUM_USERS; i++) {
      // Check if user is active (username[0] != '\0')
      if (USERS[i].username[0] != '\0') {
        // Check if mailbox is full. Interpreting 0xff as MAX_MESSAGES_PER_USER.
        if (USERS[i].message_count == MAX_MESSAGES_PER_USER) {
          print_str("[*] Unable to send message to ");
          printf("%s", USERS[i].username); // Print user's name
          print_str(". Their mailbox is full.\n");
        }
        else {
          int next_message_idx = USERS[i].message_count; // Use current count as next available index (0-indexed)
          
          // Ensure we don't exceed message buffer capacity.
          if (next_message_idx < MAX_MESSAGES_PER_USER) {
            strncpy(USERS[i].messages[next_message_idx], message_buffer, MESSAGE_SLOT_SIZE - 1);
            USERS[i].messages[next_message_idx][MESSAGE_SLOT_SIZE - 1] = '\0'; // Ensure null termination
            USERS[i].message_flags[next_message_idx] = 0; // Mark message as unread (or similar)
            USERS[i].message_count++; // Increment message count for the next message
          } else {
             // This case should ideally not be reachable if message_count == MAX_MESSAGES_PER_USER is checked correctly.
             print_str("[*] User's mailbox is unexpectedly full (index out of bounds).\n");
          }
        }
      }
    }
  }
}

// Main function to make the code compilable and runnable
int main() {
    // Initialize some dummy users for testing
    strncpy(USERS[0].username, "Alice", MAX_USERNAME_LEN);
    USERS[0].username[MAX_USERNAME_LEN] = '\0';
    USERS[0].message_count = 0;

    strncpy(USERS[1].username, "Bob", MAX_USERNAME_LEN);
    USERS[1].username[MAX_USERNAME_LEN] = '\0';
    USERS[1].message_count = MAX_MESSAGES_PER_USER; // Bob has a full mailbox

    strncpy(USERS[2].username, "Charlie", MAX_USERNAME_LEN);
    USERS[2].username[MAX_USERNAME_LEN] = '\0';
    USERS[2].message_count = 0;

    USERS[3].username[0] = '\0'; // Inactive user
    USERS[4].username[0] = '\0'; // Inactive user

    print_str("--- Admin Login Test ---\n");
    AdminLogin();

    if (ADMIN_ACCESS) {
        print_str("\n--- Admin Menu ---\n");
        PrintAdminMenu();
        print_str("\n--- Send Broadcast Message Test ---\n");
        SendBroadcastMessage();
        print_str("\nBroadcast message sent (or attempted).\n");
    } else {
        print_str("\nAdmin access denied. Cannot perform admin functions.\n");
    }

    return 0;
}