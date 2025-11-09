#include <stdio.h>   // For printf
#include <string.h>  // For strcmp, strcpy, memset
#include <stdlib.h>  // For atoi
#include <stdbool.h> // For bool type

// --- Global Definitions and Structures ---

// Assuming these are external functions provided elsewhere in the program
extern int get_user_index(char *username);
extern int receive_until(void *buf, int len, int type);

// Maximum lengths and counts derived from original snippet's offsets
#define MAX_USERNAME_LEN 15        // Max length for user names (e.g., in UserInfo)
#define MAX_MESSAGE_LEN 15         // Max length for message fields (from, to, content)
#define MAX_MESSAGES_PER_USER 20   // 0x14 messages per user
#define MESSAGE_BLOCK_SIZE 0x34    // 52 bytes per message
#define USER_BLOCK_SIZE 0x410      // 1040 bytes per user (20 * 52)
#define MAX_USERS 64               // Estimated max users based on global memory layout

// Message structure based on byte offsets and sizes in the original snippet
struct Message {
    int status;                // 0x00: 0xbeef for active, 0 for empty
    char from[MAX_MESSAGE_LEN];       // 0x04: null-terminated string (length 0xf = 15)
    char to[MAX_MESSAGE_LEN];         // 0x13: null-terminated string (length 0xf = 15)
    char msg_content[MAX_MESSAGE_LEN]; // 0x22: null-terminated string (length 0xf = 15)
    char type;                 // 0x31: 0x01 for draft, 0x00 for inbox
    char padding[1];           // 0x32, 0x33: Padding to make total 0x34 bytes
}; // Total size: 52 bytes (0x34)

// User data structure, primarily an array of messages
struct UserData {
    struct Message messages[MAX_MESSAGES_PER_USER]; // 20 * 52 = 1040 bytes (0x410)
    // Other user-specific data might follow, but are not directly accessed via `globs` in the snippet.
};

// Function pointer type for print_message (used in global_vars)
typedef void (*print_message_func_ptr)(int, int);

// Global structure to encapsulate all global variables accessed via `globs`
// The original code used `globs._10400_4_`, implying a function pointer at a specific offset.
// 0x10400 (66560) is exactly MAX_USERS * USER_BLOCK_SIZE.
// So, the `user_data` array ends just before this offset, and the function pointer follows.
struct Globals {
    struct UserData user_data[MAX_USERS];     // 64 users * 0x410 = 0x10400 bytes
    char filler_before_func_ptr[4];           // Padding/filler bytes
    print_message_func_ptr print_msg_ptr;    // At offset 0x10400 + 4
};

// Define the global instance of our Globals structure
struct Globals global_vars;

// User information structure, derived from `current_user` and `listOfUsers` access patterns.
// Assumes username is at the beginning, and `field_24`/`field_28` are at fixed offsets.
struct UserInfo {
    char name[MAX_USERNAME_LEN];          // 0x00: User's name
    char padding_to_0x24[0x24 - MAX_USERNAME_LEN]; // Filler bytes to reach 0x24
    int field_24;                         // 0x24: An integer flag or counter
    int field_28;                         // 0x28: An integer, possibly message count for this user
    char padding_after_0x28[0x2C - 0x28 - sizeof(int)]; // Total size 0x2C assumed for UserInfo
};

// Global variables, redefined based on usage in the snippet
struct UserInfo *current_user_ptr;       // Pointer to the currently logged-in user's info
struct UserInfo *listOfUsers[MAX_USERS]; // Array of pointers to UserInfo structs
int user_count;                          // Number of active users
int msg_count_login;                     // Global message counter

// --- Dummy Implementations for External Functions ---
// These are placeholders to make the code compilable and runnable.
// In a real application, these would be provided by other modules.

// Dummy get_user_index: Searches listOfUsers for a matching username
int get_user_index(char *username) {
    for (int i = 0; i < user_count; ++i) {
        if (listOfUsers[i] != NULL && strcmp(username, listOfUsers[i]->name) == 0) {
            return i;
        }
    }
    return -1; // User not found
}

// Dummy receive_until: Simulates user input from stdin
int receive_until(void *buf, int len, int type) {
    printf("Enter selection (type %d, max %d chars): ", type, len - 1);
    if (fgets((char*)buf, len, stdin) != NULL) {
        // Remove newline character if present
        ((char*)buf)[strcspn((char*)buf, "\n")] = 0;
        return (int)strlen((char*)buf);
    }
    return 0; // No input
}

// --- Function Implementations ---

// Function: print_message
void print_message(int user_idx, int message_idx) {
  struct Message *msg = &global_vars.user_data[user_idx].messages[message_idx];
  
  // Check if the 'from' field is not empty, indicating an active message
  if (strcmp(msg->from, "") != 0) {
    printf("******************\n");
    // Corrected format specifier from '@s' to '%s' and direct field access
    printf("To: %s\nFrom: %s \nMsg: %s\n", msg->to, msg->from, msg->msg_content);
  }
}

// Function: add_message
int add_message(char *from_user_name, char *to_user_name, char *message_content, char message_type) {
  global_vars.print_msg_ptr = print_message; // Initialize function pointer

  // Find the index of the sender (or the user for whom the draft is created)
  int message_owner_idx = -1;
  for (int i = 0; i < user_count; ++i) {
    if (listOfUsers[i] != NULL && strcmp(from_user_name, listOfUsers[i]->name) == 0) {
      message_owner_idx = i;
      break;
    }
  }

  if (message_owner_idx == -1) {
    printf("User does not exist.\n");
    return 0;
  }
  
  // Determine which user's message array to add to based on message type
  int target_user_data_idx = get_user_index((message_type == '\x01') ? to_user_name : from_user_name);

  if (target_user_data_idx == -1) {
    printf("Target user for message data does not exist.\n");
    return 0;
  }

  int message_slot_idx = -1;
  // Special condition from original snippet, potentially for optimized slot allocation
  if ((current_user_ptr->field_24 == 1) && (current_user_ptr->field_24 = 0, 19 < current_user_ptr->field_28)) {
    message_slot_idx = current_user_ptr->field_28; // Use the next available slot based on count
  } else {
    // Find the first empty message slot for the target user
    for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
      if (global_vars.user_data[target_user_data_idx].messages[i].status != 0xbeef) {
        message_slot_idx = i;
        break;
      }
    }
  }

  if (message_slot_idx == -1) {
    printf("No space left for this user.\n");
    return 0;
  }
  
  struct Message *msg = &global_vars.user_data[target_user_data_idx].messages[message_slot_idx];
  msg->status = 0xbeef; // Mark message as active
  strcpy(msg->to, to_user_name);
  strcpy(msg->from, from_user_name);
  strcpy(msg->msg_content, message_content);
  msg->type = message_type;
  
  msg_count_login++; // Increment global message count
  current_user_ptr->field_28++; // Increment current user's message count
  
  return 1;
}

// Function: list_drafts
void list_drafts(char *user_name) {
  bool found_drafts = false;
  int user_idx = get_user_index(user_name);
  
  if (user_idx == -1) {
      printf("User does not exist.\n");
      return;
  }

  for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
    struct Message *msg = &global_vars.user_data[user_idx].messages[i];
    if (msg->status == 0xbeef) { // Check if message slot is active
      // Check if 'to' field matches the user and it's a draft
      if ((strcmp(user_name, msg->to) == 0) && (msg->type == '\x01')) {
        found_drafts = true;
        global_vars.print_msg_ptr(user_idx, i); // Use the function pointer
      }
    }
  }
  
  if (!found_drafts) {
    printf("No drafts for this user.\n");
  }
}

// Function: list_inbox
void list_inbox(char *user_name) {
  bool found_messages = false;
  int user_idx = get_user_index(user_name);

  if (user_idx == -1) {
      printf("User does not exist.\n");
      return;
  }
  
  for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
    struct Message *msg = &global_vars.user_data[user_idx].messages[i];
    if (msg->status == 0xbeef) { // Check if message slot is active
      // Check if 'from' field matches the user and it's an inbox message
      if ((strcmp(user_name, msg->from) == 0) && (msg->type == '\0')) {
        found_messages = true;
        global_vars.print_msg_ptr(user_idx, i); // Use the function pointer
      }
    }
  }
  
  if (!found_messages) {
    printf("No messages for this user.\n");
  }
}

// Function: list_all_messages
void list_all_messages(void) {
  bool any_messages_printed = false;
  
  for (int user_idx = 0; user_idx < user_count; ++user_idx) {
    for (int msg_idx = 0; msg_idx < MAX_MESSAGES_PER_USER; ++msg_idx) {
      struct Message *msg = &global_vars.user_data[user_idx].messages[msg_idx];
      // Only print messages that are active and have a non-empty 'from' field
      if (msg->status == 0xbeef && strcmp(msg->from, "") != 0) {
        any_messages_printed = true;
        global_vars.print_msg_ptr(user_idx, msg_idx); // Use the function pointer
      }
    }
  }
  
  if (!any_messages_printed) {
    printf("No messages.\n");
  }
}

// Function: get_draft_count
int get_draft_count(char *user_name) {
  int count = 0;
  int user_idx = get_user_index(user_name);
  
  if (user_idx == -1) {
      return 0; // User not found, no drafts
  }

  for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
    struct Message *msg = &global_vars.user_data[user_idx].messages[i];
    if (msg->status == 0xbeef) { // Check if message slot is active
      // Check if 'to' field matches the user and it's a draft
      if ((strcmp(user_name, msg->to) == 0) && (msg->type == '\x01')) {
        count++;
      }
    }
  }
  return count;
}

// Function: get_inbox_count
int get_inbox_count(char *user_name) {
  int count = 0;
  int user_idx = get_user_index(user_name);
  
  if (user_idx == -1) {
      return 0; // User not found, no inbox messages
  }

  for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
    struct Message *msg = &global_vars.user_data[user_idx].messages[i];
    if (msg->status == 0xbeef) { // Check if message slot is active
      // Check if 'from' field matches the user and it's an inbox message
      if ((strcmp(user_name, msg->from) == 0) && (msg->type == '\0')) {
        count++;
      }
    }
  }
  return count;
}

// Function: get_total_count
int get_total_count(void) {
  int total_messages = 0;
  
  for (int user_idx = 0; user_idx < user_count; ++user_idx) {
    if (listOfUsers[user_idx] != NULL) {
        // Accessing field_28 from UserInfo struct pointed to by listOfUsers[user_idx]
        total_messages += listOfUsers[user_idx]->field_28; 
    }
  }
  return total_messages;
}

// Function: print_draft_for_send
void print_draft_for_send(char *user_name) {
  int user_idx = get_user_index(user_name);
  
  if (user_idx == -1) {
      printf("User does not exist.\n");
      return;
  }

  for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
    struct Message *msg = &global_vars.user_data[user_idx].messages[i];
    if (msg->status == 0xbeef) { // Check if message slot is active
      // Check if 'to' field matches the user and it's a draft
      if ((strcmp(user_name, msg->to) == 0) && (msg->type == '\x01')) {
        printf("%d: To: %s Msg: %s\n", i, msg->to, msg->msg_content);
      }
    }
  }
  
  char input_buffer[10]; // Buffer for user input (e.g., draft index)
  int input_len;
  do {
    input_len = receive_until(input_buffer, sizeof(input_buffer), 3);
  } while (input_len == 0); // Loop until input is received
  
  int selected_msg_idx = atoi(input_buffer); // Convert input to integer
  
  if ((selected_msg_idx < MAX_MESSAGES_PER_USER) && (selected_msg_idx >= 0)) {
    struct Message *selected_draft = &global_vars.user_data[user_idx].messages[selected_msg_idx];
    // Verify the selected message is a valid draft for the current user
    if ((strcmp(user_name, selected_draft->to) == 0) && (selected_draft->type == '\x01')) {
      int recipient_user_idx = get_user_index(selected_draft->to); // Get recipient's user index
      
      if (recipient_user_idx == -1) {
          printf("Recipient user does not exist.\n");
          return;
      }

      int recipient_msg_idx = -1;
      // Find an empty slot in the recipient's message array
      for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
        if (global_vars.user_data[recipient_user_idx].messages[i].status != 0xbeef) {
          recipient_msg_idx = i;
          break;
        }
      }
      
      if (recipient_msg_idx == -1) {
        printf("No space left for this recipient user.\n");
      } else {
        struct Message *recipient_slot = &global_vars.user_data[recipient_user_idx].messages[recipient_msg_idx];
        
        // Copy message content from draft to recipient's inbox slot
        strcpy(recipient_slot->to, selected_draft->to);
        strcpy(recipient_slot->from, selected_draft->from);
        strcpy(recipient_slot->msg_content, selected_draft->msg_content);
        recipient_slot->type = '\0'; // Change type to inbox message
        recipient_slot->status = 0xbeef; // Mark as active
        
        // Clear the sent draft by resetting its fields
        memset(selected_draft->from, 0, MAX_MESSAGE_LEN);
        memset(selected_draft->to, 0, MAX_MESSAGE_LEN);
        memset(selected_draft->msg_content, 0, MAX_MESSAGE_LEN);
        selected_draft->status = 0; // Mark draft slot as empty
      }
    } else {
      printf("Not a valid selection.\n");
    }
  } else {
    printf("Out of range.\n");
  }
}

// Function: print_draft_for_del
void print_draft_for_del(char *user_name) {
  int user_idx = get_user_index(user_name);
  
  if (user_idx == -1) {
      printf("User does not exist.\n");
      return;
  }

  for (int i = 0; i < MAX_MESSAGES_PER_USER; ++i) {
    struct Message *msg = &global_vars.user_data[user_idx].messages[i];
    if (msg->status == 0xbeef) { // Check if message slot is active
      // Check if 'to' field matches the user and it's a draft
      if ((strcmp(user_name, msg->to) == 0) && (msg->type == '\x01')) {
        printf("%d: To: %s Msg: %s\n", i, msg->to, msg->msg_content);
      }
    }
  }
  
  char input_buffer[10]; // Buffer for user input (e.g., draft index)
  int input_len;
  do {
    input_len = receive_until(input_buffer, sizeof(input_buffer), 2);
  } while (input_len == 0); // Loop until input is received
  
  int selected_msg_idx = atoi(input_buffer); // Convert input to integer
  
  if ((selected_msg_idx < MAX_MESSAGES_PER_USER) && (selected_msg_idx >= 0)) {
    struct Message *selected_draft = &global_vars.user_data[user_idx].messages[selected_msg_idx];
    // Verify the selected message is a valid draft for the current user
    if ((strcmp(user_name, selected_draft->to) == 0) && (selected_draft->type == '\x01')) {
      // Clear the draft by resetting its fields
      memset(selected_draft->from, 0, MAX_MESSAGE_LEN);
      memset(selected_draft->to, 0, MAX_MESSAGE_LEN);
      memset(selected_draft->msg_content, 0, MAX_MESSAGE_LEN);
      selected_draft->status = 0; // Mark draft slot as empty
      current_user_ptr->field_28--; // Decrement message count for current user
    } else {
      printf("Not a valid selection.\n");
    }
  } else {
    printf("Out of range.\n");
  }
}