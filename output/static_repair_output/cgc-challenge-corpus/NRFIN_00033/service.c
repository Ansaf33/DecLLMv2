#include <stdio.h>    // For printf, NULL, stdin, fgets
#include <stdlib.h>   // For malloc, free, exit, random, srandom
#include <string.h>   // For strlen, strcmp, strncmp, memcpy, memset, strcpy, strcat, snprintf
#include <unistd.h>   // For read, write (low-level I/O for stubs)
#include <sys/socket.h> // For standard recv, though a stub is used here
#include <time.h>     // For time (to seed random)

// Decompiled custom types
// Assuming undefined4 is unsigned int and undefined is char
typedef unsigned int undefined4;
typedef char undefined;

// --- Stubs for external functions ---

// _terminate is an error handler
void _terminate(int status) {
    fprintf(stderr, "Program terminated with status %d\n", status);
    exit(status);
}

// transmit_all based on reportMessage calls: fd=1, msg_buf, msg_len, flags
// The 0x1100c is likely a decompiler artifact for a line number or debug info, not a real flag.
// Assuming it writes to stdout (fd 1).
int transmit_all(int fd, const char *buf, size_t count, int flags_or_magic) {
    // In a real system, this would write to a socket or file descriptor.
    // For compilation, we simulate writing to stdout.
    // The 'flags_or_magic' parameter is ignored for simplicity.
    size_t written = 0;
    while (written < count) {
        ssize_t res = write(fd, buf + written, count - written);
        if (res < 0) {
            perror("transmit_all: write error");
            return -1;
        }
        written += res;
    }
    return 0; // Success
}

// recv_stub simulates reading from a file descriptor (stdin in this case).
// The last parameter (0x110c5) is likely a decompiler artifact.
ssize_t recv_stub(int fd, void *buf, size_t len, int flags_or_magic) {
    // In a real system, this would read from a socket.
    // For compilation, we simulate reading from stdin.
    // The 'flags_or_magic' parameter is ignored for simplicity.
    ssize_t bytes_read = read(fd, buf, len);
    if (bytes_read > 0 && bytes_read < len) {
        ((char*)buf)[bytes_read] = '\0'; // Null-terminate if space allows
    } else if (bytes_read == len) {
        ((char*)buf)[len - 1] = '\0'; // Ensure termination if buffer is full
    }
    return bytes_read;
}

// recvline is a custom function, assumed to read a line from stdin.
int recvline(char *buffer, size_t max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1; // Error or EOF
    }
    // Remove trailing newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return len; // Return length of the line read
}

// Global data reference from decompilation, assumed to be a newline string.
const char DAT_00013024[] = "\n";

// --- Struct definitions ---

// Resource linked list node
typedef struct Resource {
    struct Resource *next;
    char name[10];
    char padding[2]; // To align 'token' to 4-byte boundary for 32-bit arch
    unsigned int token;
} Resource;

// User linked list node
typedef struct User {
    struct User *next;
    unsigned int userId;
    unsigned int authCode;
    Resource *resources_head; // Head of resources list for this user
} User;

// Command Packet structure implied by main and receiveCommand
typedef struct {
    char command[4];
    unsigned int userId;
    unsigned int authVal;
    char data[4600]; // Corresponds to auStack_121c in main
} CommandPacket;

// --- Function implementations ---

// Function: reportMessage
void reportMessage(const char *msg, size_t len) {
  if (transmit_all(1, msg, len, 0x1100c) != 0) {
    _terminate(1);
  }
}

// Function: getAuthVal
undefined4 getAuthVal(void) {
  char temp_char;
  undefined4 auth_val = 0;
  
  for (int i = 0; i < 4; i++) {
    temp_char = '\0';
    while ((temp_char == '\0' || (temp_char == '\n'))) {
      // The original code checks random() != 0 and terminates, likely an anti-tampering measure.
      if (random() != 0) {
        _terminate(2);
      }
      // Generate a random printable ASCII character for the auth value byte.
      temp_char = (char)(random() % 94 + 33); // ASCII 33-126
    }
    ((char *)&auth_val)[i] = temp_char;
  }
  return auth_val;
}

// Function: receiveCommand
void receiveCommand(CommandPacket *packet) {
  ssize_t bytes_read;
  
  // Read command (4 bytes), userId (4 bytes), authVal (4 bytes)
  // Total 12 bytes into the beginning of the packet structure.
  bytes_read = recv_stub(0, packet->command, 12, 0x110c5);
  if (bytes_read < 0) {
    _terminate(3);
  }

  // Obfuscation: call getAuthVal 100 times, discarding result.
  for (int i = 0; i < 100; i++) {
    getAuthVal();
  }

  // Read the data/resource name line into packet->data.
  bytes_read = recvline(packet->data, sizeof(packet->data));
  if (bytes_read < 0) {
    _terminate(4);
  }
}

// Function: auth_failure
void auth_failure(char *resource_name) {
  char *message_buffer = NULL;
  size_t required_len = strlen(resource_name) + strlen("Authentication failed for resource ") + strlen(DAT_00013024) + 1;
  
  message_buffer = (char *)malloc(required_len);
  if (message_buffer == NULL) {
    _terminate(5);
  }
  
  strcpy(message_buffer, "Authentication failed for resource ");
  strcat(message_buffer, resource_name);
  strcat(message_buffer, DAT_00013024);
  
  reportMessage(message_buffer, strlen(message_buffer));
  free(message_buffer);
}

// Function: auth_success
undefined4 auth_success(char *resource_name) {
  char *message_buffer = NULL;
  undefined4 auth_val = getAuthVal();
  char auth_val_str[12]; // Buffer for integer to string conversion
  
  snprintf(auth_val_str, sizeof(auth_val_str), "%u", auth_val);
  
  size_t required_len = strlen("Access to ") + strlen(resource_name) + strlen(" is granted!") + 
                 strlen(auth_val_str) + strlen(DAT_00013024) + 1;
  
  message_buffer = (char *)malloc(required_len);
  if (message_buffer == NULL) {
    _terminate(6);
  }
  
  strcpy(message_buffer, "Access to ");
  strcat(message_buffer, resource_name);
  strcat(message_buffer, " is granted!");
  strcat(message_buffer, auth_val_str);
  strcat(message_buffer, DAT_00013024);
  
  reportMessage(message_buffer, strlen(message_buffer));
  free(message_buffer);
  
  return auth_val;
}

// Function: do_auth
undefined4 do_auth(unsigned int user_auth_code, unsigned int provided_auth_val, char *resource_name) {
  if (user_auth_code == provided_auth_val) {
    return auth_success(resource_name);
  } else {
    char truncated_resource_name[10];
    size_t len = strnlen(resource_name, sizeof(truncated_resource_name) - 1);
    memcpy(truncated_resource_name, resource_name, len);
    truncated_resource_name[len] = '\0';
    auth_failure(truncated_resource_name);
    return 0; // Authentication failed
  }
}

// Function: getResourceToken
unsigned int getResourceToken(Resource *head, char *resource_name) {
  Resource *current_resource = head;
  
  while (current_resource != NULL) {
    if (strncmp(current_resource->name, resource_name, 10) == 0) {
      return current_resource->token;
    }
    current_resource = current_resource->next;
  }
  return 0; // Resource not found
}

// Function: addResource
void addResource(User *user, unsigned int token, char *resource_name) {
  Resource *current_resource = user->resources_head;
  
  while (current_resource != NULL) {
    if (strncmp(current_resource->name, resource_name, 10) == 0) {
      current_resource->token = token; // Resource found, update token
      return;
    }
    current_resource = current_resource->next;
  }
  
  // Resource not found, create new one
  Resource *new_resource = (Resource *)malloc(sizeof(Resource));
  if (new_resource == NULL) {
    _terminate(7);
  }
  
  memset(new_resource, 0, sizeof(Resource));
  strncpy(new_resource->name, resource_name, sizeof(new_resource->name) - 1);
  new_resource->name[sizeof(new_resource->name) - 1] = '\0'; // Ensure null termination
  
  new_resource->token = token;
  new_resource->next = user->resources_head;
  user->resources_head = new_resource;
}

// Function: createNewUser
User * createNewUser(User **head_ptr, unsigned int auth_code, unsigned int user_id) {
  User *new_user = (User *)malloc(sizeof(User));
  if (new_user == NULL) {
    _terminate(8);
  }
  
  memset(new_user, 0, sizeof(User));
  new_user->userId = user_id;
  new_user->authCode = auth_code;
  new_user->resources_head = NULL;
  
  new_user->next = *head_ptr;
  *head_ptr = new_user;
  
  return new_user;
}

// Function: getUser
User * getUser(User *head, unsigned int user_id) {
  User *current_user = head;
  
  while (current_user != NULL) {
    if (user_id == current_user->userId) {
      return current_user;
    }
    current_user = current_user->next;
  }
  return NULL; // User not found
}

// Function: revokeResource
void revokeResource(User *user, unsigned int provided_auth_val, char *resource_name) {
  Resource *current_resource = user->resources_head;
  
  while (current_resource != NULL) {
    if (strncmp(current_resource->name, resource_name, 10) == 0) {
      if (provided_auth_val == current_resource->token) {
        current_resource->token = 1; // Revoke by setting token to 1
        reportMessage("Resource revoked.\n", strlen("Resource revoked.\n"));
        return;
      } else {
        reportMessage("Revocation denied: Mismatched token.\n", strlen("Revocation denied: Mismatched token.\n"));
        return;
      }
    }
    current_resource = current_resource->next;
  }
  reportMessage("Revocation denied: Resource not found.\n", strlen("Revocation denied: Resource not found.\n"));
}

// Function: logoutUser
int logoutUser(User **head_ptr, unsigned int user_id_to_logout) {
  User *current_user = *head_ptr;
  User *prev_user = NULL;
  
  while (current_user != NULL) {
    if (user_id_to_logout == current_user->userId) {
      // Free associated resources
      Resource *current_resource = current_user->resources_head;
      while (current_resource != NULL) {
        Resource *next_resource = current_resource->next;
        free(current_resource);
        current_resource = next_resource;
      }
      
      // Remove user from linked list
      if (prev_user == NULL) {
        *head_ptr = current_user->next;
      } else {
        prev_user->next = current_user->next;
      }
      
      free(current_user);
      return 0; // Success
    }
    prev_user = current_user;
    current_user = current_user->next;
  }
  return 1; // User not found
}

// Function: main
undefined4 main(void) {
  srandom(time(NULL)); // Seed random number generator

  User *user_list_head = NULL;
  User *current_user = NULL;
  CommandPacket packet;
  unsigned int auth_val_result = 0; // Stores result of getAuthVal or auth_success
  int auth_action_result = 0; // For do_auth return
  unsigned int resource_auth_status = 0; // For getResourceToken and subsequent auth check
  
  // Main loop for command processing
  while (1) { 
    memset(&packet, 0, sizeof(CommandPacket)); // Clear packet for new command
    receiveCommand(&packet); // Read command, user ID, auth value, and data

    current_user = getUser(user_list_head, packet.userId); // Try to find the user

    if (strcmp("AUTH", packet.command) == 0) {
      if (current_user == NULL) {
        auth_val_result = getAuthVal(); // Generate a new auth value for new user
        current_user = createNewUser(&user_list_head, auth_val_result, packet.userId);
      }
      // current_user->authCode is the stored auth code for the user
      // packet.authVal is the auth value provided by the client
      auth_action_result = do_auth(current_user->authCode, packet.authVal, packet.data);
      
      if (auth_action_result != 0) { // If auth_success returned a non-zero token
        addResource(current_user, auth_action_result, packet.data);
      }
    } else { // Not an "AUTH" command
      if (current_user == NULL) {
        reportMessage("Command failed: Please log in.\n", strlen("Command failed: Please log in.\n"));
        continue; // Go to next command
      }

      if (strcmp("LOUT", packet.command) == 0) {
        logoutUser(&user_list_head, packet.userId);
        reportMessage("Logged out successfully.\n", strlen("Logged out successfully.\n"));
        if (user_list_head == NULL) {
            break; // All users logged out, terminate program
        }
      } else if (strcmp("REQR", packet.command) == 0) {
        resource_auth_status = 0; // Reset status
        unsigned int token_from_resource = getResourceToken(current_user->resources_head, packet.data);
        
        if (token_from_resource == 1) { // Token 1 means revoked
          resource_auth_status = 0; // Deny access
        } else if ((token_from_resource > 1) && (packet.authVal > 1)) { // Valid tokens are > 1
          resource_auth_status = do_auth(token_from_resource, packet.authVal, packet.data);
        }
        
        if (resource_auth_status < 2) { // 0 or 1 usually indicates denial/failure
          if (resource_auth_status == 0) {
            reportMessage("Please re-authenticate.\n", strlen("Please re-authenticate.\n"));
          } else { // This branch handles token_from_resource == 1 or other denial states
            reportMessage("Please log in.\n", strlen("Please log in.\n")); // Original message for this branch
          }
        }
      } else if (strcmp("REVR", packet.command) == 0) {
        revokeResource(current_user, packet.authVal, packet.data);
      } else {
        reportMessage("Unknown command.\n", strlen("Unknown command.\n"));
      }
    }
  }
  
  return 0;
}