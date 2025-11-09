#include <stdio.h>    // For sprintf
#include <string.h>   // For strlen, memcpy, strncpy, strcpy, strdup
#include <stdlib.h>   // For strtol, malloc, exit, free
#include <ctype.h>    // For tolower
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uint32_t

// --- External/Mocked Functions ---
// In a real application, these would come from other source files or libraries.
void modem_output(const char *data, size_t len) {
    printf("%s", data); // For demonstration, print to stdout
}
void modem_init(void *arg) {
    // Mock initialization
}
void modem_loop(void (*rx_callback)(void *, size_t)) {
    // This function would typically block and call rx_callback on incoming data.
    // For this exercise, we'll just return immediately.
}

// Forward declarations for structs
struct Thread;
struct Message;

// Global variables
struct Thread *g_threads = NULL; // Head of the linked list of threads.

// Current state of the BBS system.
// 0: Main menu
// 1: Thread list pagination
// 2: Message list pagination
// 3: Post - Subject input
// 4: Post - Body input
int g_state = 0;

int g_next_id = 1; // Counter for generating unique thread/message IDs.

// Used to store the next thread page pointer or current thread for posting.
struct Thread *g_current_thread_page_ptr = NULL;

// Used to store the next message page pointer.
struct Message *g_current_message_page_ptr = NULL;

// Subject buffer for posting. Original code used address of DAT_00019388 for this.
char g_post_subject_buffer[64];

// --- Struct Definitions ---
// These structures are defined for a standard 64-bit Linux environment.
// The original code's `malloc(0x4c)` (76 bytes) implied a 32-bit environment
// with 4-byte pointers. By using `sizeof()` we ensure correct allocation
// for the target architecture.

// Structure for a BBS thread
struct Thread {
    struct Thread *next_thread;
    struct Message *messages;   // Pointer to the first message in this thread
    uint32_t id;
    char subject[64];           // Subject string (max 63 chars + null terminator)
};

// Structure for a BBS message
struct Message {
    struct Message *next_message;
    uint32_t id;
    char subject[64];           // Subject string (max 63 chars + null terminator)
    char *body;                 // Pointer to the dynamically allocated message body string
};


// --- Function Prototypes ---
void send_string(char *param_1);
void bbs_help(void);
void bbs_menu(void);
struct Thread *find_thread(uint32_t param_1);
struct Message *find_message(uint32_t param_1);
void send_thread(struct Thread *thread_ptr);
void send_message_brief(struct Message *message_ptr);
void send_message(struct Message *message_ptr);
void send_thread_list(struct Thread *param_1);
void send_message_list(struct Message *param_1);
void do_list(char *param_1);
void do_post(char *param_1);
void do_read(char *param_1);
void handle_post(char *param_1);
void handle_list(char *param_1);
void handle_menu(char *param_1);
void bbs_rx(void *param_1, size_t param_2);


// --- Function Definitions ---

// Function: send_string
void send_string(char *param_1) {
  modem_output(param_1, strlen(param_1));
}

// Function: bbs_help
void bbs_help(void) {
  send_string(
             "Available commands:\n\t(L)ist [thread-id]\n\t\tList all threads, or messages in a thread\n\t(P)ost [thread-id]\n\t\tPost a new thread, or a reply to a thread\n\t(R)ead message-id\n\t\tGet message contents\n\t(H)elp\n\t\tThis screen\n"
             );
}

// Function: bbs_menu
void bbs_menu(void) {
  send_string("(L)ist, (H)elp, (P)ost, (R)ead\n");
}

// Function: find_thread
struct Thread *find_thread(uint32_t param_1) {
  for (struct Thread *current_thread = g_threads;
       current_thread != NULL && (param_1 <= current_thread->id);
       current_thread = current_thread->next_thread) {
    if (current_thread->id == param_1) {
      return current_thread;
    }
  }
  return NULL;
}

// Function: find_message
struct Message *find_message(uint32_t param_1) {
  for (struct Thread *current_thread = g_threads; current_thread != NULL; current_thread = current_thread->next_thread) {
    for (struct Message *current_message = current_thread->messages;
         current_message != NULL && (param_1 <= current_message->id);
         current_message = current_message->next_message) {
      if (current_message->id == param_1) {
        return current_message;
      }
    }
  }
  return NULL;
}

// Function: send_thread
void send_thread(struct Thread *thread_ptr) {
  char buffer[104];
  sprintf(buffer, "%08u - %s\n", thread_ptr->id, thread_ptr->subject);
  send_string(buffer);
}

// Function: send_message_brief
void send_message_brief(struct Message *message_ptr) {
  char buffer[104];
  sprintf(buffer, "%08u - %s\n", message_ptr->id, message_ptr->subject);
  send_string(buffer);
}

// Function: send_message
void send_message(struct Message *message_ptr) {
  char buffer[104];
  sprintf(buffer, "%08u - %s\n", message_ptr->id, message_ptr->subject);
  send_string(buffer);
  send_string(message_ptr->body);
}

// Function: send_thread_list
void send_thread_list(struct Thread *param_1) {
  int count = 0;
  for (; count < 0x28 && param_1 != NULL; param_1 = param_1->next_thread, count++) {
    send_thread(param_1);
  }
  if (param_1 == NULL) {
    g_state = 0;
  } else {
    g_state = 1;
    g_current_thread_page_ptr = param_1;
    send_string("(N)ext page, (Q)uit\n");
  }
}

// Function: send_message_list
void send_message_list(struct Message *param_1) {
  int count = 0;
  for (; count < 0x28 && param_1 != NULL; param_1 = param_1->next_message, count++) {
    send_message_brief(param_1);
  }
  if (param_1 == NULL) {
    g_state = 0;
  } else {
    g_state = 2;
    g_current_message_page_ptr = param_1;
    send_string("(N)ext page, (Q)uit\n");
  }
}

// Function: do_list
void do_list(char *param_1) {
  if (param_1[1] == ' ') {
    long thread_id = strtol(param_1 + 2, NULL, 10);
    struct Thread *found_thread = find_thread(thread_id);
    if (found_thread == NULL) {
      send_string("Thread ID not found.\n");
    } else {
      send_message_list(found_thread->messages);
    }
  } else {
    send_thread_list(g_threads);
  }
}

// Function: do_post
void do_post(char *param_1) {
  g_current_thread_page_ptr = NULL; // Reset for new thread by default
  if (param_1[1] == ' ') {
    long thread_id = strtol(param_1 + 2, NULL, 10);
    struct Thread *found_thread = find_thread(thread_id);
    if (found_thread == NULL) {
      send_string("Thread ID not found.\n");
      return;
    }
    g_current_thread_page_ptr = found_thread; // Reply to existing thread
  }
  send_string("Subject?\n");
  g_state = 3;
}

// Function: do_read
void do_read(char *param_1) {
  if (param_1[1] == ' ') {
    long message_id = strtol(param_1 + 2, NULL, 10);
    struct Message *found_message = find_message(message_id);
    if (found_message == NULL) {
      send_string("Message ID not found.\n");
    } else {
      send_message(found_message);
    }
  } else {
    send_string("Missing required argument.\n");
  }
}

// Function: handle_post
void handle_post(char *param_1) {
  if (g_state == 3) { // Inputting subject
    strncpy(g_post_subject_buffer, param_1, sizeof(g_post_subject_buffer) - 1);
    g_post_subject_buffer[sizeof(g_post_subject_buffer) - 1] = '\0';
    send_string("Body?\n");
    g_state = 4;
  } else if (g_state == 4) { // Inputting body
    struct Thread *target_thread = g_current_thread_page_ptr;

    if (target_thread == NULL) { // New thread
      target_thread = (struct Thread *)malloc(sizeof(struct Thread));
      if (target_thread == NULL) {
        g_state = 0; // Out of memory
        return;
      }
      strncpy(target_thread->subject, g_post_subject_buffer, sizeof(target_thread->subject) - 1);
      target_thread->subject[sizeof(target_thread->subject) - 1] = '\0';

      target_thread->id = g_next_id++;
      target_thread->next_thread = g_threads;
      target_thread->messages = NULL;
      g_threads = target_thread;
    }

    struct Message *new_message = (struct Message *)malloc(sizeof(struct Message));
    if (new_message != NULL) {
      new_message->id = g_next_id++;
      strncpy(new_message->subject, g_post_subject_buffer, sizeof(new_message->subject) - 1);
      new_message->subject[sizeof(new_message->subject) - 1] = '\0';
      new_message->body = strdup(param_1); // Duplicate the body string

      if (new_message->body != NULL) {
        new_message->next_message = target_thread->messages;
        target_thread->messages = new_message;
      } else {
        free(new_message); // strdup failed, free the message struct
      }
    }
    g_state = 0; // Return to main menu
  }
}

// Function: handle_list
void handle_list(char *param_1) {
  if (tolower((unsigned char)*param_1) == 'q') {
    g_state = 0;
  } else if (tolower((unsigned char)*param_1) == 'n') {
    if (g_state == 1) { // Paginating threads
      send_thread_list(g_current_thread_page_ptr);
    } else { // Paginating messages
      send_message_list(g_current_message_page_ptr);
    }
  } else {
    send_string("Bad input.\n");
  }
}

// Function: handle_menu
void handle_menu(char *param_1) {
  // Check for multi-character commands or commands followed by non-space
  if (param_1[1] != '\0' && param_1[1] != ' ') {
    send_string("Bad input. Unknown command.\n");
    return;
  }

  switch (tolower((unsigned char)*param_1)) {
    case '?':
    case 'h':
      bbs_help();
      break;
    case 'l':
      do_list(param_1);
      break;
    case 'p':
      do_post(param_1);
      break;
    case 'q':
      exit(0);
    case 'r':
      do_read(param_1);
      break;
    default:
      send_string("Bad input. Unknown command.\n");
      break;
  }
}

// Function: bbs_rx
void bbs_rx(void *param_1, size_t param_2) {
  char buffer[105]; // Max 104 chars + null terminator
  memcpy(buffer, param_1, param_2);
  buffer[param_2] = '\0';

  if (strlen(buffer) != 0) {
    if (g_state == 0) {
      handle_menu(buffer);
    } else if (g_state == 1 || g_state == 2) {
      handle_list(buffer);
    } else if (g_state == 3 || g_state == 4) {
      handle_post(buffer);
    }
    // After any state change, if we're back to main menu, print it.
    if (g_state == 0) {
      bbs_menu();
    }
  }
}

// Function: main
int main(void) {
  modem_init(NULL); // Original passes &stack0x00000004, likely a dummy arg.
  bbs_menu();
  modem_loop(bbs_rx);
  return 0;
}