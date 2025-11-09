#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// --- Stub declarations for external functions ---
// zero: Fills a buffer with zeros
void zero(void *buf, size_t len) {
    memset(buf, 0, len);
}

// print: Prints a null-terminated string to stdout
void print(const char *s) {
    if (s) {
        printf("%s", s);
    }
}

// read_until: Reads input into a buffer until newline or max_len. Returns -1 on error.
int read_until(char *buf, size_t max_len, char delimiter) {
    if (fgets(buf, max_len, stdin) == NULL) {
        return -1; // Error or EOF
    }
    // Remove trailing newline if present
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == delimiter) {
        buf[len - 1] = '\0';
    }
    return 0; // Success
}

// _terminate: Exits the program with an error status
void _terminate(void) {
    perror("Fatal error, terminating");
    exit(EXIT_FAILURE);
}

// strmatch: Compares two strings. Returns non-zero if they match (like `!strcmp`).
int strmatch(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0 ? 1 : 0;
}

// strcopy: Copies a string. Assuming it's like strcpy.
void strcopy(char *dest, const char *src) {
    strcpy(dest, src);
}

// isdigits: Checks if a string contains only digits.
int isdigits(const char *s) {
    if (!s || *s == '\0') return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) {
            return 0;
        }
        s++;
    }
    return 1;
}

// print_uint: Prints an unsigned integer to stdout
void print_uint(unsigned int val) {
    printf("%u", val);
}

// --- Global data and structures ---
#define MESSAGE_SIZE 0x32 // 50 bytes
#define MESSAGE_CONTENT_OFFSET 0x10 // Content starts at 16 bytes into message block
#define USER_BLOCK_SIZE 0x32e4 // 13028 bytes
#define MAX_MESSAGES_PER_USER 255 // From PrintNewMessages loop (0xff)

// Definition of MessageBlock
typedef struct {
    char data[MESSAGE_SIZE]; // Raw message block. Content starts at data[MESSAGE_CONTENT_OFFSET]
} MessageBlock;

// Definition of User structure
typedef struct {
    char user_data[0x10]; // Some initial user data/padding, likely username
    MessageBlock messages[MAX_MESSAGES_PER_USER];
    char read_flags[MAX_MESSAGES_PER_USER]; // Offset 0x31de
    char padding_after_flags[3]; // To align message_count
    int message_count; // Offset 0x32e0
} User;

// Global variables
User USERS[10]; // Assuming 10 users for demonstration
int CURRENT_USER = 0; // Default current user, e.g., index 0
const int NUM_USERS = 10; // Number of users in the system

// Global string literals from decompilation
const char DAT_000130c8[] = "Enter recipient username: ";
const char DAT_000130f9[] = "Enter message ID: ";
const char DAT_0001315d[] = "Message (ID: ";
const char DAT_00013161[] = "):\n";

// Function: SendMessage
void SendMessage(void) {
  char recipient_username[16];
  char message_content[50];
  int user_idx;
  int msg_idx;
  size_t len;

  zero(recipient_username, sizeof(recipient_username));
  print(DAT_000130c8);
  if (read_until(recipient_username, sizeof(recipient_username), '\n') == -1) {
    _terminate();
  }

  len = strlen(recipient_username);
  if (len == 0) {
    return;
  }

  zero(message_content, sizeof(message_content));
  print("Message: ");
  if (read_until(message_content, sizeof(message_content), '\n') == -1) {
    _terminate();
  }

  len = strlen(message_content);
  if (len == 0) {
    return;
  }

  for (user_idx = 0; user_idx < NUM_USERS; user_idx++) {
    if (strmatch(recipient_username, USERS[user_idx].user_data) != 0) {
      break;
    }
  }

  if (user_idx == NUM_USERS) {
    return;
  }

  if (USERS[user_idx].message_count == MAX_MESSAGES_PER_USER) {
    print("[-] Recipient's mailbox is full\n");
  } else {
    msg_idx = USERS[user_idx].message_count + 1;
    strcopy(&USERS[user_idx].messages[msg_idx - 1].data[MESSAGE_CONTENT_OFFSET], message_content);
    USERS[user_idx].read_flags[msg_idx - 1] = 0;
    USERS[user_idx].message_count = msg_idx;
  }
}

// Function: ReadMessage
void ReadMessage(void) {
  char msg_id_str[4];
  unsigned int msg_id;
  size_t len;

  zero(msg_id_str, sizeof(msg_id_str));
  print(DAT_000130f9);
  if (read_until(msg_id_str, sizeof(msg_id_str), '\n') == -1) {
    _terminate();
  }

  len = strlen(msg_id_str);
  if (len == 0) {
    return;
  }

  if (isdigits(msg_id_str)) {
    msg_id = atoi(msg_id_str);

    if (msg_id == 0 || USERS[CURRENT_USER].message_count < msg_id) {
      print("[-] Message ID out of range\n");
    } else if (USERS[CURRENT_USER].messages[msg_id - 1].data[MESSAGE_CONTENT_OFFSET] == '\0') {
      print("[-] Message ID not found\n");
    } else {
      print("***********************************\n");
      print(DAT_0001315d);
      print_uint(msg_id);
      print(DAT_00013161);
      print((const char*)&USERS[CURRENT_USER].messages[msg_id - 1].data[MESSAGE_CONTENT_OFFSET]);
      print("***********************************\n");
      USERS[CURRENT_USER].read_flags[msg_id - 1] = 1;
    }
  }
}

// Function: ListMessages
void ListMessages(void) {
  unsigned int msg_id;

  for (msg_id = 1; msg_id <= USERS[CURRENT_USER].message_count; msg_id++) {
    if (USERS[CURRENT_USER].messages[msg_id - 1].data[MESSAGE_CONTENT_OFFSET] != '\0') {
      print("***********************************\n");
      print(DAT_0001315d);
      print_uint(msg_id);
      print(DAT_00013161);
      print((const char*)&USERS[CURRENT_USER].messages[msg_id - 1].data[MESSAGE_CONTENT_OFFSET]);
      print("***********************************\n");
    }
  }
}

// Function: DeleteMessage
void DeleteMessage(void) {
  char msg_id_str[4];
  unsigned int msg_id;
  size_t len;

  zero(msg_id_str, sizeof(msg_id_str));
  print(DAT_000130f9);
  if (read_until(msg_id_str, sizeof(msg_id_str), '\n') == -1) {
    _terminate();
  }

  len = strlen(msg_id_str);
  if (len == 0) {
    return;
  }

  if (isdigits(msg_id_str)) {
    msg_id = atoi(msg_id_str);

    if (msg_id == 0 || USERS[CURRENT_USER].message_count < msg_id) {
      print("[-] Message ID out of range\n");
    } else {
      zero(&USERS[CURRENT_USER].messages[msg_id - 1].data[0], MESSAGE_SIZE);
      USERS[CURRENT_USER].read_flags[msg_id - 1] = 0;
    }
  }
}

// Function: PrintNewMessages
void PrintNewMessages(void) {
  bool unread_found = false;
  unsigned int msg_id;

  for (msg_id = 1; msg_id < MAX_MESSAGES_PER_USER; msg_id++) {
    if ((USERS[CURRENT_USER].messages[msg_id - 1].data[MESSAGE_CONTENT_OFFSET] != '\0') &&
        (USERS[CURRENT_USER].read_flags[msg_id - 1] != 1)) {
      if (!unread_found) {
        print("Unread messages:\n");
        unread_found = true;
      }
      print("***********************************\n");
      print(DAT_0001315d);
      print_uint(msg_id);
      print(DAT_00013161);
      print((const char*)&USERS[CURRENT_USER].messages[msg_id - 1].data[MESSAGE_CONTENT_OFFSET]);
      print("***********************************\n");
      USERS[CURRENT_USER].read_flags[msg_id - 1] = 1;
    }
  }
}