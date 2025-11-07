#include <stdio.h>    // For printf, fprintf, getchar, NULL
#include <string.h>   // For strlen, memset, strncpy, strcmp
#include <stdlib.h>   // For atoi, exit
#include <stdbool.h>  // For bool type
#include <ctype.h>    // For isdigit

// --- Global Constants and Data Structures ---

// Define sizes and offsets based on decompiled analysis
#define USER_BLOCK_SIZE            0x32e4 // 13028 bytes per user block
#define MAX_USERNAME_LENGTH        16     // Size of local_24, used for username
#define MESSAGE_BODY_LENGTH        50     // 0x32, size of local_56, and message content
#define MSG_CONTENT_ARRAY_OFFSET   0x10   // Offset where messages array starts within a user block
#define MSG_READ_FLAG_ARRAY_OFFSET 0x31de // Offset where read flags array starts within a user block
#define MSG_COUNT_OFFSET           0x32e0 // Offset where message_count (int) is stored within a user block

// Calculate MAX_MESSAGES_PER_USER based on allocated space
// (Space for messages) / (Message body length)
// (MSG_READ_FLAG_ARRAY_OFFSET - MSG_CONTENT_ARRAY_OFFSET) / MESSAGE_BODY_LENGTH
// (0x31de - 0x10) / 0x32 = 12750 / 50 = 255
#define MAX_MESSAGES_PER_USER      255

#define NUM_USERS                  5 // Arbitrary number of users, can be adjusted for a full system

// Global user data storage (assuming a flat memory model as suggested by offsets)
char USERS[NUM_USERS][USER_BLOCK_SIZE];
int CURRENT_USER = 0; // Index of the currently logged-in user

// Decompiled string literals
const char DAT_000130c8[] = "Enter recipient username: ";
const char DAT_000130f9[] = "Enter message ID: ";
const char DAT_0001315d[] = "\nMessage: ";
const char DAT_00013161[] = "\n-----------------------------------\n";

// --- Helper Function Implementations (Mimicking original behavior) ---

// Fills a buffer with zeros
static void zero_buffer(char *buf, size_t len) {
    memset(buf, 0, len);
}

// Prints a string to stdout
static void print(const char *str) {
    fputs(str, stdout);
}

// Prints an unsigned integer to stdout
static void print_uint(unsigned int val) {
    printf("%u", val);
}

// Reads input until a newline or max_len is reached. Returns number of bytes read or -1 on error.
static int read_until_newline(char *buf, size_t max_len) {
    if (fgets(buf, max_len, stdin) == NULL) {
        return -1; // Error or EOF
    }
    // Remove trailing newline if present
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
        len--;
    }
    return (int)len;
}

// Terminates the program
static void _terminate(void) {
    exit(1);
}

// Custom string match function (case-sensitive, returns non-zero if match)
// Compares the beginning of the user block (assumed to be username) with a supplied string.
static int strmatch(const char *s1_user_block_start, const char *s2_username_input) {
    return strncmp(s1_user_block_start, s2_username_input, MAX_USERNAME_LENGTH) == 0;
}

// Custom string copy function with length limit
static void strcopy_limited(char *dest, const char *src, size_t max_len) {
    strncpy(dest, src, max_len);
    dest[max_len - 1] = '\0'; // Ensure null-termination
}

// Checks if a string consists only of digits
static int isdigits(const char *str) {
    if (*str == '\0') return 0; // Empty string is not all digits
    while (*str != '\0') {
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

// --- Main Functions (Refactored from snippet) ---

void SendMessage(void) {
  char recipient_username[MAX_USERNAME_LENGTH]; // Corresponds to local_24
  char message_content[MESSAGE_BODY_LENGTH];     // Corresponds to local_56
  int recipient_user_idx;                        // Corresponds to local_10
  int next_message_id;                           // Corresponds to local_14
  
  // Get recipient username
  zero_buffer(recipient_username, sizeof(recipient_username));
  print(DAT_000130c8); // "Enter recipient username: "
  if (read_until_newline(recipient_username, sizeof(recipient_username)) == -1) {
    _terminate();
  }
  
  if (strlen(recipient_username) == 0) {
      return; // No recipient username entered
  }

  // Get message content
  zero_buffer(message_content, sizeof(message_content));
  print("Message: ");
  if (read_until_newline(message_content, sizeof(message_content)) == -1) {
    _terminate();
  }

  if (strlen(message_content) == 0) {
      return; // No message content entered
  }

  // Find recipient user
  for (recipient_user_idx = 0; recipient_user_idx < NUM_USERS; recipient_user_idx++) {
    // strmatch compares recipient_username with the username field of each user block
    if (strmatch(USERS[recipient_user_idx], recipient_username)) {
      break; // User found
    }
  }

  if (recipient_user_idx != NUM_USERS) { // User found
    int *recipient_message_count = (int *)(USERS[recipient_user_idx] + MSG_COUNT_OFFSET);
    
    if (*recipient_message_count == MAX_MESSAGES_PER_USER) { // Mailbox full
      print("[-] Recipient's mailbox is full\n");
    } else {
      next_message_id = *recipient_message_count + 1; // Message IDs typically start from 1
      
      // Copy message content into the correct slot
      char *message_dest = USERS[recipient_user_idx] + MSG_CONTENT_ARRAY_OFFSET + next_message_id * MESSAGE_BODY_LENGTH;
      strcopy_limited(message_dest, message_content, MESSAGE_BODY_LENGTH);
      
      // Mark message as unread (0)
      char *read_flag_ptr = USERS[recipient_user_idx] + MSG_READ_FLAG_ARRAY_OFFSET + next_message_id;
      *read_flag_ptr = 0; // Unread
      
      // Increment message count for the recipient
      *recipient_message_count = next_message_id;
    }
  } else {
      print("[-] Recipient user not found\n");
  }
  return;
}

void ReadMessage(void) {
  char msg_id_str[4]; // Corresponds to local_14
  unsigned int message_id;  // Corresponds to local_10
  
  zero_buffer(msg_id_str, sizeof(msg_id_str));
  print(DAT_000130f9); // "Enter message ID: "
  if (read_until_newline(msg_id_str, sizeof(msg_id_str)) == -1) {
    _terminate();
  }
  
  if (strlen(msg_id_str) == 0 || !isdigits(msg_id_str)) {
      print("[-] Invalid message ID\n");
      return;
  }

  message_id = atoi(msg_id_str);
  
  int *current_user_message_count = (int *)(USERS[CURRENT_USER] + MSG_COUNT_OFFSET);

  if (*current_user_message_count < message_id || message_id == 0) { // Message IDs start from 1
    print("[-] Message ID out of range\n");
  } else {
    char *message_content_ptr = USERS[CURRENT_USER] + MSG_CONTENT_ARRAY_OFFSET + message_id * MESSAGE_BODY_LENGTH;
    
    if (*message_content_ptr == '\0') { // Check if message slot is empty/deleted
      print("[-] Message ID not found\n");
    } else {
      print("***********************************\n");
      print_uint(message_id);
      print(DAT_0001315d); // "\nMessage: "
      print(message_content_ptr);
      print(DAT_00013161); // "\n-----------------------------------\n"
      print("***********************************\n");
      
      // Mark message as read (1)
      char *read_flag_ptr = USERS[CURRENT_USER] + MSG_READ_FLAG_ARRAY_OFFSET + message_id;
      *read_flag_ptr = 1;
    }
  }
  return;
}

void ListMessages(void) {
  unsigned int message_id; // Corresponds to local_10
  int *current_user_message_count = (int *)(USERS[CURRENT_USER] + MSG_COUNT_OFFSET);
  
  for (message_id = 1; message_id <= *current_user_message_count; message_id++) {
    char *message_content_ptr = USERS[CURRENT_USER] + MSG_CONTENT_ARRAY_OFFSET + message_id * MESSAGE_BODY_LENGTH;
    if (*message_content_ptr != '\0') { // Check if message slot is not empty/deleted
      print("***********************************\n");
      print_uint(message_id);
      print(DAT_0001315d); // "\nMessage: "
      print(message_content_ptr);
      print(DAT_00013161); // "\n-----------------------------------\n"
      print("***********************************\n");
    }
  }
  return;
}

void DeleteMessage(void) {
  char msg_id_str[4]; // Corresponds to local_14
  unsigned int message_id;  // Corresponds to local_10
  
  zero_buffer(msg_id_str, sizeof(msg_id_str));
  print(DAT_000130f9); // "Enter message ID: "
  if (read_until_newline(msg_id_str, sizeof(msg_id_str)) == -1) {
    _terminate();
  }

  if (strlen(msg_id_str) == 0 || !isdigits(msg_id_str)) {
      print("[-] Invalid message ID\n");
      return;
  }

  message_id = atoi(msg_id_str);
  
  int *current_user_message_count = (int *)(USERS[CURRENT_USER] + MSG_COUNT_OFFSET);

  if (*current_user_message_count < message_id || message_id == 0) { // Message IDs start from 1
    print("[-] Message ID out of range\n");
  } else {
    // Zero out the message content to "delete" it
    char *message_content_ptr = USERS[CURRENT_USER] + MSG_CONTENT_ARRAY_OFFSET + message_id * MESSAGE_BODY_LENGTH;
    zero_buffer(message_content_ptr, MESSAGE_BODY_LENGTH);
    
    // Optionally, also mark the read flag as deleted (e.g., 0xFF)
    // char *read_flag_ptr = USERS[CURRENT_USER] + MSG_READ_FLAG_ARRAY_OFFSET + message_id;
    // *read_flag_ptr = 0xFF; // Mark as deleted
  }
  return;
}

void PrintNewMessages(void) {
  bool first_unread_message_printed = true; // Corresponds to bVar1
  unsigned int message_id;                   // Corresponds to local_10
  int *current_user_message_count = (int *)(USERS[CURRENT_USER] + MSG_COUNT_OFFSET);
  
  // The original loop condition `local_10 < 0xff` (255) means IDs 1 to 254.
  // Using MAX_MESSAGES_PER_USER (255) ensures all possible message slots are checked.
  for (message_id = 1; message_id <= MAX_MESSAGES_PER_USER; message_id++) {
    // Optimize: stop if message_id exceeds the actual number of messages for the current user
    if (message_id > *current_user_message_count) {
        break;
    }

    char *message_content_ptr = USERS[CURRENT_USER] + MSG_CONTENT_ARRAY_OFFSET + message_id * MESSAGE_BODY_LENGTH;
    char *read_flag_ptr = USERS[CURRENT_USER] + MSG_READ_FLAG_ARRAY_OFFSET + message_id;

    // Check if message exists (content is not empty) AND is unread (flag is not 1)
    if ((*message_content_ptr != '\0') && (*read_flag_ptr != 1)) {
      if (first_unread_message_printed) {
        print("Unread messages:\n");
        first_unread_message_printed = false;
      }
      print("***********************************\n");
      print_uint(message_id);
      print(DAT_0001315d); // "\nMessage: "
      print(message_content_ptr);
      print(DAT_00013161); // "\n-----------------------------------\n"
      print("***********************************\n");
      
      // Mark message as read (1) after printing
      *read_flag_ptr = 1;
    }
  }
  return;
}