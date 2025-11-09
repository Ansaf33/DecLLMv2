#include <stdio.h>  // For puts, printf, fgets, stdin
#include <stdlib.h> // For malloc, free, exit, atoi
#include <string.h> // For strlen, strcmp, strncat, strncpy, memset, strchr
#include <stdbool.h> // For true/false

// --- Struct Definitions ---

// Forward declarations for mutually recursive structs
struct Message;
struct MessageListHead;

// User structure
struct User {
    char username[16]; // 0x00 - 0x0F
    struct MessageListHead *manager; // 0x10 - 0x13 (pointer to MessageListHead)
    struct User *next; // 0x14 - 0x17
};

// User list head
struct UserListHead {
    struct User *head;
    unsigned int count;
};

// Message structure
struct Message {
    struct Message *next; // 0x00 - 0x03
    unsigned int id;      // 0x04 - 0x07
    unsigned int read;    // 0x08 - 0x0B (0 for unread, 1 for read)
    char content[32];     // 0x0C - 0x2B (total 44 bytes = 0x2C)
};

// Message list head
struct MessageListHead {
    struct Message *head;
    unsigned int count;
};

// --- Custom Function Replacements ---

// Replacement for allocate
// int allocate(size_t size, int flags, void **out_ptr)
// Allocates memory and initializes it to zero. Returns 0 on success, -1 on failure.
int allocate(size_t size, int flags, void **out_ptr) {
    (void)flags; // flags parameter is unused in original snippet's calls
    *out_ptr = malloc(size);
    if (*out_ptr == NULL) {
        return -1; // Indicate failure
    }
    memset(*out_ptr, 0, size); // Initialize allocated memory to zero
    return 0; // Indicate success
}

// Replacement for deallocate
// void deallocate(void *ptr, size_t size)
// Frees allocated memory.
void deallocate(void *ptr, size_t size) {
    (void)size; // size parameter is unused for standard free
    free(ptr);
}

// Replacement for _terminate
// void _terminate(void)
// Exits the program.
void _terminate(void) {
    puts("Terminating program.");
    exit(1); // Exit with an error code
}

// Replacement for receive_until
// int receive_until(char *buffer, int max_len, char delimiter)
// Reads input into buffer until 'delimiter' is found or 'max_len' characters are read.
// max_len is the maximum number of characters to store in the buffer, excluding the null terminator.
// The buffer must be at least max_len + 1 bytes. Returns 0 on success, -1 on failure.
int receive_until(char *buffer, int max_len, char delimiter) {
    if (fgets(buffer, max_len + 1, stdin) == NULL) {
        return -1; // Indicate failure
    }
    // Remove trailing newline or delimiter
    char *found_delimiter = strchr(buffer, delimiter);
    if (found_delimiter != NULL) {
        *found_delimiter = '\0';
    } else {
        // If no delimiter, buffer might be full, clear stdin
        if (strlen(buffer) == max_len) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }
    return 0; // Indicate success
}

// Replacement for itoa (non-standard)
// void my_itoa(char *buf, int val, int buf_size)
// Converts an integer to a string and stores it in buf.
// Uses snprintf for safety, buf_size is the total size of the buffer.
void my_itoa(char *buf, int val, int buf_size) {
    snprintf(buf, buf_size, "%d", val);
}


// --- Original Functions (Refactored) ---

// Function: list_users
// Lists the usernames of all registered users.
void list_users(struct UserListHead *user_list_head) {
  if (user_list_head != NULL) {
    for (struct User *current_user = user_list_head->head; current_user != NULL; current_user = current_user->next) {
      puts(current_user->username);
    }
  }
}

// Function: init_users
// Initializes the global user list head.
// Returns a pointer to the initialized UserListHead on success, NULL on failure.
struct UserListHead *init_users(void) {
  struct UserListHead *user_list_head = NULL;
  if (allocate(sizeof(struct UserListHead), 0, (void **)&user_list_head) == 0) {
    user_list_head->head = NULL;
    user_list_head->count = 0;
  } else {
    puts("[-] Error Failed to allocate user list head\n");
  }
  return user_list_head;
}

// Function: init_manager
// Initializes a message list head for a user.
// Returns a pointer to the initialized MessageListHead on success, NULL on failure.
struct MessageListHead *init_manager(void) {
  struct MessageListHead *message_list_head = NULL;
  if (allocate(sizeof(struct MessageListHead), 0, (void **)&message_list_head) == 0) {
    message_list_head->head = NULL;
    message_list_head->count = 0;
  } else {
    puts("[-] Error Failed to allocate message manager\n");
  }
  return message_list_head;
}

// Function: create_user
// Creates a new user with the given username and adds them to the user list.
// Returns the new count of users on success, 0 on failure.
unsigned int create_user(struct UserListHead *user_list_head, const char *username) {
  if (user_list_head == NULL || username == NULL) {
    return 0; // Indicate failure or invalid input
  }

  // Check username length (leaving space for null terminator)
  if (strlen(username) >= sizeof(((struct User*)0)->username)) {
    puts("[-] Error username too long\n");
    return 0;
  }

  // Check if user already exists
  for (struct User *current_user = user_list_head->head; current_user != NULL; current_user = current_user->next) {
    if (strcmp(current_user->username, username) == 0) {
      puts("[-] Error user exists\n");
      return 0;
    }
  }

  struct User *new_user = NULL;
  if (allocate(sizeof(struct User), 0, (void **)&new_user) == 0) {
    strncpy(new_user->username, username, sizeof(new_user->username) - 1);
    new_user->username[sizeof(new_user->username) - 1] = '\0'; // Ensure null termination

    new_user->manager = init_manager(); // Allocate message manager for the user
    if (new_user->manager == NULL) {
        puts("[-] Error Failed to allocate manager for user\n");
        deallocate(new_user, sizeof(struct User)); // Clean up user allocation
        return 0;
    }

    // Add new user to the head of the list
    new_user->next = user_list_head->head;
    user_list_head->head = new_user;
    user_list_head->count++;
    return user_list_head->count; // Return the new count
  } else {
    puts("[-] Error allocating user structure\n");
  }
  return 0;
}

// Function: add_message
// Adds a new message to a user's message list.
// Returns the assigned message ID on success, 0 on failure.
unsigned int add_message(struct MessageListHead *message_list_head, struct Message *new_message) {
  if (new_message == NULL || message_list_head == NULL) {
    return 0; // Indicate failure
  }

  new_message->read = 0; // Mark as unread by default
  new_message->next = NULL; // Ensure next pointer is null

  if (message_list_head->head == NULL) {
    message_list_head->head = new_message;
  } else {
    struct Message *current = message_list_head->head;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = new_message;
  }

  message_list_head->count++;
  new_message->id = message_list_head->count; // Assign a new ID
  return new_message->id;
}

// Function: create_message
// Creates a new message structure with the given content.
// Returns a pointer to the new Message on success, NULL on failure.
struct Message *create_message(const char *message_content) {
  struct Message *new_message = NULL;
  if (message_content == NULL) {
    return NULL;
  }

  // Check message content length (leaving space for null terminator)
  if (strlen(message_content) >= sizeof(((struct Message*)0)->content)) {
    puts("[-] Error Message too long\n");
    return NULL;
  }

  if (allocate(sizeof(struct Message), 0, (void **)&new_message) == 0) {
    // Memory is already zeroed by allocate, including next, id, read fields.
    strncpy(new_message->content, message_content, sizeof(new_message->content) - 1);
    new_message->content[sizeof(new_message->content) - 1] = '\0'; // Ensure null termination
  } else {
    puts("[-] Error Failed to allocate message struct\n");
  }
  return new_message;
}

// Function: delete_message
// Deletes a message by its ID from a user's message list.
void delete_message(struct MessageListHead *message_list_head, unsigned int message_id) {
  if (message_list_head == NULL) {
    return;
  }

  if (message_list_head->count == 0 || message_id == 0) { // No messages or invalid ID
      puts("[-] Message ID not found\n");
      return;
  }

  struct Message *current = message_list_head->head;
  struct Message *prev = NULL;

  while (current != NULL) {
    if (message_id == current->id) {
      if (prev == NULL) { // Deleting the head
        message_list_head->head = current->next;
      } else {
        prev->next = current->next;
      }
      deallocate(current, sizeof(struct Message));
      message_list_head->count--; // Decrement count on successful deletion
      return;
    }
    prev = current;
    current = current->next;
  }
  puts("[-] Message ID not found\n");
}

// Function: read_message
// Reads and displays a message by its ID from a user's message list, marking it as read.
void read_message(struct MessageListHead *message_list_head, unsigned int message_id) {
  if (message_list_head == NULL) {
    return;
  }

  if (message_list_head->count == 0 || message_id == 0) { // No messages or invalid ID
      puts("[-] Message ID not found\n");
      return;
  }

  char output_buffer[256];
  size_t current_len = 0;

  memset(output_buffer, 0, sizeof(output_buffer));
  strncat(output_buffer, "***********************************\n", sizeof(output_buffer) - 1);

  for (struct Message *current = message_list_head->head; current != NULL; current = current->next) {
    if (message_id == current->id) {
      current_len = strlen(output_buffer);
      my_itoa(output_buffer + current_len, message_id, sizeof(output_buffer) - current_len - 1);
      
      strncat(output_buffer, ":  ", sizeof(output_buffer) - strlen(output_buffer) - 1);
      strncat(output_buffer, current->content, sizeof(output_buffer) - strlen(output_buffer) - 1);
      strncat(output_buffer, "\n***********************************\n", sizeof(output_buffer) - strlen(output_buffer) - 1);
      
      puts(output_buffer);
      current->read = 1; // Mark as read
      return;
    }
  }
  puts("[-] Message ID not found\n");
}

// Function: list_unread_messages
// Lists all unread messages for a user, marking them as read after listing.
void list_unread_messages(struct MessageListHead *message_list_head) {
  if (message_list_head == NULL) {
    return;
  }

  unsigned int unread_count = 0;
  for (struct Message *current = message_list_head->head; current != NULL; current = current->next) {
    if (current->read == 0) {
      unread_count++;
    }
  }

  if (unread_count == 0) {
    puts("No unread messages.\n");
    return;
  }

  puts("Unread messages:\n");
  
  // Calculate required buffer size. Each message takes approx 120 chars.
  // Add some buffer for safety.
  size_t total_buffer_size = unread_count * 128 + 1; // +1 for null terminator

  char *output_buffer = (char *)malloc(total_buffer_size);
  if (output_buffer == NULL) {
    puts("[-] Error: Failed to allocate buffer for unread messages.\n");
    return;
  }
  memset(output_buffer, 0, total_buffer_size); // Initialize buffer

  for (struct Message *current = message_list_head->head; current != NULL; current = current->next) {
    if (current->read == 0) {
      current->read = 1; // Mark as read after listing

      size_t current_len = strlen(output_buffer);
      size_t remaining_size = total_buffer_size - current_len - 1; // Space for string + null terminator

      if (remaining_size == 0) break; // Buffer full

      strncat(output_buffer, "***********************************\n", remaining_size);
      current_len = strlen(output_buffer);
      remaining_size = total_buffer_size - current_len - 1;

      if (remaining_size == 0) break;

      my_itoa(output_buffer + current_len, current->id, remaining_size);
      current_len = strlen(output_buffer);
      remaining_size = total_buffer_size - current_len - 1;

      if (remaining_size == 0) break;

      strncat(output_buffer, ":  ", remaining_size);
      strncat(output_buffer, current->content, remaining_size);
      strncat(output_buffer, "\n***********************************\n", remaining_size);
    }
  }
  puts(output_buffer);
  free(output_buffer);
}

// Function: list_messages
// Lists all messages for a user by calling read_message for each.
void list_messages(struct MessageListHead *message_list_head) {
  if (message_list_head != NULL) {
    for (struct Message *current = message_list_head->head; current != NULL; current = current->next) {
      read_message(message_list_head, current->id);
    }
  }
}

// Function: get_user
// Retrieves a user by their username.
// Returns a pointer to the User struct on success, NULL if not found.
struct User *get_user(struct UserListHead *user_list_head, const char *username) {
  if (user_list_head == NULL || username == NULL) {
    return NULL;
  }

  struct User *current_user = user_list_head->head;
  while (current_user != NULL) {
    if (strcmp(current_user->username, username) == 0) {
      return current_user;
    }
    current_user = current_user->next;
  }
  return NULL; // User not found
}

// Function: send_user_message
// Sends a message to a specified user.
// Returns the message ID on success, 0 on failure.
unsigned int send_user_message(struct UserListHead *user_list_head, const char *target_username, const char *message_content) {
  if (user_list_head == NULL || target_username == NULL || message_content == NULL) {
    return 0;
  }

  struct User *target_user = get_user(user_list_head, target_username);
  if (target_user == NULL) {
    puts("[-] Error invalid user\n");
    return 0;
  }

  struct Message *new_message = create_message(message_content);
  if (new_message == NULL) {
    puts("[-] Failed to create message\n");
    return 0;
  }

  unsigned int message_id = add_message(target_user->manager, new_message);
  if (message_id == 0) {
    puts("[-] Failed to add message\n");
    deallocate(new_message, sizeof(struct Message)); // Clean up message if not added
  }
  return message_id;
}

// Function: handle_loggedin
// Handles the menu options for a logged-in user.
void handle_loggedin(struct UserListHead *user_list_head, struct User *logged_in_user) {
  if (user_list_head == NULL || logged_in_user == NULL) {
    return;
  }

  char choice_str[4]; // Buffer for single digit input + newline + null terminator
  char target_username_buffer[sizeof(((struct User*)0)->username)]; // Max 15 chars + null
  char message_content_buffer[sizeof(((struct Message*)0)->content)]; // Max 31 chars + null
  unsigned int choice;
  unsigned int message_id;

  while (true) {
    list_unread_messages(logged_in_user->manager);

    puts("1) Send Message");
    puts("2) Read Message");
    puts("3) List Messages");
    puts("4) Delete Message");
    puts("5) Logout");
    puts("6) Exit");
    puts("Enter choice: ");

    if (receive_until(choice_str, 1, '\n') == -1) { // Read 1 char + newline
      puts("[-] Receive failed\n");
      _terminate();
    }
    
    choice = atoi(choice_str);

    if (choice < 1 || choice > 6) {
      puts("[-] Invalid choice\n");
      continue;
    }

    switch (choice) {
      case 1: // Send Message
        memset(target_username_buffer, 0, sizeof(target_username_buffer));
        memset(message_content_buffer, 0, sizeof(message_content_buffer));
        
        puts("Recipient username: ");
        if (receive_until(target_username_buffer, sizeof(target_username_buffer) - 1, '\n') == -1) {
            puts("[-] Receive failed\n"); continue;
        }

        puts("Message: ");
        if (receive_until(message_content_buffer, sizeof(message_content_buffer) - 1, '\n') == -1) {
            puts("[-] Receive failed\n"); continue;
        }
        send_user_message(user_list_head, target_username_buffer, message_content_buffer);
        break;

      case 2: // Read Message
        puts("Enter message ID: ");
        // Re-use message_content_buffer for ID input (numeric string)
        memset(message_content_buffer, 0, sizeof(message_content_buffer));
        if (receive_until(message_content_buffer, sizeof(message_content_buffer) - 1, '\n') == -1) {
            puts("[-] Receive failed\n"); continue;
        }
        message_id = atoi(message_content_buffer);
        read_message(logged_in_user->manager, message_id);
        break;

      case 3: // List Messages
        list_messages(logged_in_user->manager);
        break;

      case 4: // Delete Message
        puts("Enter message ID: ");
        // Re-use message_content_buffer for ID input (numeric string)
        memset(message_content_buffer, 0, sizeof(message_content_buffer));
        if (receive_until(message_content_buffer, sizeof(message_content_buffer) - 1, '\n') == -1) {
            puts("[-] Receive failed\n"); continue;
        }
        message_id = atoi(message_content_buffer);
        delete_message(logged_in_user->manager, message_id);
        break;

      case 5: // Logout
        puts("Logging out...\n");
        return; // Exit handle_loggedin

      case 6: // Exit
        puts("Exiting...\n");
        _terminate();
        // Unreachable
        break;
    }
  }
}

// Function: handle_menu
// Displays the main menu and handles user choices (create user, login, exit).
void handle_menu(void) {
  char username_buffer[sizeof(((struct User*)0)->username)]; // Max 15 chars + null
  char choice_str[4]; // Buffer for single digit input + newline + null terminator
  unsigned int choice;
  
  struct UserListHead *user_list_head = init_users();
  if (user_list_head == NULL) {
    puts("[-] Failed to initialize user system. Exiting.\n");
    _terminate();
  }

  while (true) {
    puts("1) Create User");
    puts("2) Login");
    puts("3) Exit");
    puts("Enter choice: ");

    memset(choice_str, 0, sizeof(choice_str)); 
    if (receive_until(choice_str, 1, '\n') == -1) { // Read 1 char + newline
      puts("[-] Receive Failed\n");
      _terminate();
    }
    
    choice = atoi(choice_str);

    if (choice < 1 || choice > 3) {
      puts("[-] Invalid choice\n");
      continue;
    }

    switch (choice) {
      case 1: // Create User
        puts("username: ");
        memset(username_buffer, 0, sizeof(username_buffer));
        if (receive_until(username_buffer, sizeof(username_buffer) - 1, '\n') == -1) {
            puts("[-] Receive failed\n"); continue;
        }
        create_user(user_list_head, username_buffer);
        break;

      case 2: // Login
        puts("username: ");
        memset(username_buffer, 0, sizeof(username_buffer));
        if (receive_until(username_buffer, sizeof(username_buffer) - 1, '\n') == -1) {
            puts("[-] Receive failed\n"); continue;
        }
        struct User *logged_in_user = get_user(user_list_head, username_buffer);
        if (logged_in_user == NULL) {
          puts("Login Failed\n");
        } else {
          puts("Login Success\n");
          handle_loggedin(user_list_head, logged_in_user);
        }
        break;

      case 3: // Exit
        puts("Exiting...\n");
        // TODO: Implement proper memory deallocation for all users, managers, and messages here.
        // For this specific request, we'll just exit.
        _terminate();
        // Unreachable
        break;
    }
  }
}

// Function: main
// Entry point of the program.
int main(void) {
  handle_menu();
  return 0;
}