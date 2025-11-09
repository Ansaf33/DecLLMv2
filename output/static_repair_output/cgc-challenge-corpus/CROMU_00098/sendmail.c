#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // For srand/rand
#include <unistd.h> // For some system calls if needed, not explicitly here but common for Linux

// --- Struct Definitions ---

// Address Book Entry (originally 0x10 bytes)
typedef struct AddressEntry {
    char *name;
    char *email;
    int flag; // 0 or 1 in add_random_addressbook_entry
    struct AddressEntry *next;
} AddressEntry;

// Message Entry (originally 0x1c bytes)
typedef struct MessageEntry {
    AddressEntry *sender;
    AddressEntry *recipient;
    char *subject;
    char *body;
    size_t body_len;
    struct MessageEntry *next;
} MessageEntry;

// Queue Entry (originally 0x10 bytes)
// Fields are ordered logically. The original snippet's raw offsets
// are assumed to be for a 32-bit system or a specific packing.
// For standard C compilation on Linux (typically 64-bit), fields are
// laid out by the compiler, and `sizeof(QueueEntry)` will be used.
typedef struct QueueEntry {
    char *address_name;
    MessageEntry *messages;
    struct QueueEntry *next;
} QueueEntry;

// Address Book (global structure, originally 8 bytes)
typedef struct AddressBook {
    int count;
    AddressEntry *head;
} AddressBook;

// --- Global Variables ---
AddressBook *abook_global;
QueueEntry *root_queue = NULL; // Initialize to NULL

// --- Placeholder/Helper Functions ---

// Simple PRNG seeding
void seed_prng(long seed) {
    srand((unsigned int)seed);
}

// Simple random_in_range implementation
// Generates a random integer between min_val (inclusive) and max_val (inclusive).
int random_in_range(int min_val, int max_val) {
    if (min_val > max_val) { // Handle swapped min/max
        int temp = min_val;
        min_val = max_val;
        max_val = temp;
    }
    if (max_val < 0) return 0; // Return 0 or handle error for negative range
    if (min_val == max_val) return min_val;
    return rand() % (max_val - min_val + 1) + min_val;
}

// Placeholder for `crackaddr`
void crackaddr(char *addr, char *buffer) {
    strncpy(buffer, addr, 1023); // Assuming buffer size 1024
    buffer[1023] = '\0';
    printf("crackaddr called with: %s\n", addr);
}

// Placeholder for `receive_until`
// Simulates reading user input from stdin.
int receive_until(char *buffer, int max_len, int timeout) {
    printf("> ");
    fflush(stdout);
    if (fgets(buffer, max_len, stdin) != NULL) {
        // Remove trailing newline if present
        buffer[strcspn(buffer, "\n")] = 0;
        return strlen(buffer);
    }
    return 0; // Indicate error or EOF
}

// --- Function Declarations (to resolve circular dependencies) ---
AddressEntry *pick_address(void);
MessageEntry *add_random_message(QueueEntry *q_entry);
char *make_string(const char *param_1);
void *gen_random_string(int length_range_max, size_t element_size); // Returns char* actually

// Function: gen_random_string
// Generates a random string of length between 1 and `length_range_max - 1`.
// `element_size` is typically 1 for char strings.
void *gen_random_string(int length_range_max, size_t element_size) {
    // Ensure string has at least 1 character and is within max bounds
    size_t length = random_in_range(1, length_range_max - 1);
    char *str = (char *)calloc(length + 1, element_size); // +1 for null terminator

    if (str == NULL) {
        perror("Failed to allocate memory for random string");
        return NULL;
    }

    for (size_t i = 0; i < length; ++i) {
        str[i] = (char)random_in_range(0x2e, 0x7e); // ASCII chars from '.' (46) to '~' (126)
    }
    str[length] = '\0'; // Null-terminate the string
    return str;
}

// Function: add_random_addressbook_entry
AddressEntry *add_random_addressbook_entry(void) {
    AddressEntry *new_entry = (AddressEntry *)calloc(1, sizeof(AddressEntry));
    if (new_entry == NULL) {
        perror("Failed to allocate memory for address book entry");
        return NULL;
    }

    new_entry->name = (char *)gen_random_string(5, 1); // Max length 4
    new_entry->email = (char *)gen_random_string(5, 1); // Max length 4
    new_entry->flag = random_in_range(0, 1);

    if (abook_global->head == NULL) {
        abook_global->head = new_entry;
    } else {
        AddressEntry *current = abook_global->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_entry;
    }
    abook_global->count++;
    return new_entry;
}

// Function: initialize_address_book
void initialize_address_book(void) {
    abook_global->count = 0;
    abook_global->head = NULL;
    for (int i = 0; i < 0x14; ++i) { // 20 entries
        add_random_addressbook_entry();
    }
}

// Function: pick_address
AddressEntry *pick_address(void) {
    if (abook_global->count == 0 || abook_global->head == NULL) {
        return NULL; // No addresses in the book
    }

    int chosen_index = random_in_range(0, abook_global->count - 1);
    AddressEntry *current = abook_global->head;
    for (int i = 0; i < chosen_index; ++i) {
        current = current->next;
    }
    return current;
}

// Function: lookup_name
AddressEntry *lookup_name(const char *name) {
    AddressEntry *current = abook_global->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: add_random_queue
QueueEntry *add_random_queue(void) {
    QueueEntry *new_queue = (QueueEntry *)calloc(1, sizeof(QueueEntry));
    if (new_queue == NULL) {
        perror("Failed to allocate memory for queue entry");
        return NULL;
    }

    AddressEntry *addr_entry = pick_address();
    if (addr_entry == NULL) {
        fprintf(stderr, "Warning: No addresses available to create queue.\n");
        free(new_queue);
        return NULL;
    }
    new_queue->address_name = make_string(addr_entry->name); // Store a copy of the name

    // The original code implies add_random_message is called with the queue pointer
    // and returns the first message, which is then assigned to the queue's message list.
    new_queue->messages = add_random_message(new_queue);

    if (root_queue == NULL) {
        root_queue = new_queue;
    } else {
        QueueEntry *current = root_queue;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_queue;
    }
    return new_queue;
}

// Function: add_random_message
MessageEntry *add_random_message(QueueEntry *q_entry) {
    MessageEntry *new_message = (MessageEntry *)calloc(1, sizeof(MessageEntry));
    if (new_message == NULL) {
        perror("Failed to allocate memory for message entry");
        return NULL;
    }

    new_message->sender = pick_address();
    new_message->recipient = pick_address();
    new_message->subject = (char *)gen_random_string(5, 1); // Max length 4
    new_message->body = (char *)gen_random_string(0x100, 1); // Max length 255
    new_message->body_len = strlen(new_message->body);

    // Link the new message to the queue's message list
    if (q_entry->messages == NULL) {
        q_entry->messages = new_message;
    } else {
        MessageEntry *current = q_entry->messages;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_message;
    }
    return new_message;
}

// Function: initialize_mail_queues
void initialize_mail_queues(void) {
    root_queue = NULL; // Ensure it's clear before starting
    if (add_random_queue() == NULL) { // First queue
        fprintf(stderr, "Failed to initialize first mail queue.\n");
        return;
    }
    for (int i = 0; i < 0x20; ++i) { // 32 more queues
        add_random_queue();
    }
}

// Function: print_address_book
void print_address_book(void) {
    printf("Address Book:\n");
    AddressEntry *current = abook_global->head;
    int i = 1;
    while (current != NULL) {
        printf("%d) %s@%s [%d]\n", i, current->name, current->email, current->flag);
        current = current->next;
        i++;
    }
}

// Function: make_string
char *make_string(const char *src) {
    if (src == NULL) {
        return NULL;
    }
    char *dest = (char *)calloc(strlen(src) + 1, 1);
    if (dest == NULL) {
        perror("Failed to allocate memory for string copy");
        return NULL;
    }
    strcpy(dest, src);
    return dest;
}

// Function: locate_queue
QueueEntry *locate_queue(const char *address_name) {
    QueueEntry *current = root_queue;
    while (current != NULL) {
        if (strcmp(current->address_name, address_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: sendmail_post
void sendmail_post(char *param_1) {
    char *sender_start = strstr(param_1, "sender:");
    if (sender_start == NULL) return;
    sender_start += 7;

    char *recipient_start = strstr(param_1, "recipient:");
    if (recipient_start == NULL) return;
    recipient_start += 10;

    char *body_start = strstr(param_1, "body:");
    if (body_start == NULL) return;
    body_start += 5;

    char *subject_start = strstr(param_1, "subject:");
    if (subject_start == NULL) return;
    subject_start += 8;

    // Null-terminate the fields by finding the next '!' or end of string
    char *delimiter;
    if ((delimiter = strstr(sender_start, "!")) != NULL) *delimiter = '\0';
    if ((delimiter = strstr(recipient_start, "!")) != NULL) *delimiter = '\0';
    if ((delimiter = strstr(body_start, "!")) != NULL) *delimiter = '\0';
    if ((delimiter = strstr(subject_start, "!")) != NULL) *delimiter = '\0';

    MessageEntry *new_message = (MessageEntry *)calloc(1, sizeof(MessageEntry));
    if (new_message == NULL) {
        perror("Failed to allocate memory for new message");
        return;
    }

    new_message->sender = lookup_name(sender_start);

    if (*recipient_start == '+') {
        char addr_buffer[1024];
        crackaddr(recipient_start, addr_buffer);
        printf("addr:%s\n", addr_buffer);
        // Original code implies special handling for '+' addresses, not adding to queue.
        free(new_message); // Message not fully formed or handled
        return;
    } else {
        new_message->recipient = lookup_name(recipient_start);
    }

    if (new_message->recipient == NULL || new_message->sender == NULL) {
        fprintf(stderr, "Sender or Recipient not found in address book.\n");
        free(new_message);
        return;
    }

    new_message->body = make_string(body_start);
    new_message->body_len = strlen(body_start);
    new_message->subject = make_string(subject_start);

    QueueEntry *target_queue = locate_queue(new_message->sender->name);
    if (target_queue == NULL) {
        target_queue = (QueueEntry *)calloc(1, sizeof(QueueEntry));
        if (target_queue == NULL) {
            perror("Failed to allocate memory for new queue");
            free(new_message->body);
            free(new_message->subject);
            free(new_message);
            return;
        }
        target_queue->address_name = make_string(new_message->sender->name);
        target_queue->messages = new_message;

        if (root_queue == NULL) {
            root_queue = target_queue;
        } else {
            QueueEntry *current = root_queue;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = target_queue;
        }
    } else {
        if (target_queue->messages == NULL) {
            target_queue->messages = new_message;
        } else {
            MessageEntry *current_message = target_queue->messages;
            while (current_message->next != NULL) {
                current_message = current_message->next;
            }
            current_message->next = new_message;
        }
    }
    printf("Message Received\n");
}

// Function: list_queue
void list_queue(const char *param_1) {
    QueueEntry *q_entry = locate_queue(param_1);
    if (q_entry == NULL) {
        printf("Mail Queue Not Found\n");
    } else {
        int i = 0;
        MessageEntry *current_message = q_entry->messages;
        while (current_message != NULL) {
            if (current_message->sender == NULL || current_message->recipient == NULL) {
                printf("%d) Malformed Message\n", i);
            } else {
                printf("%d) From:%s To: %s SUBJECT:%s\n", i,
                       current_message->sender->name,
                       current_message->recipient->name,
                       current_message->subject);
            }
            current_message = current_message->next;
            i++;
        }
    }
}

// Function: list_all_queues
void list_all_queues(void) {
    QueueEntry *current_queue = root_queue;
    while (current_queue != NULL) {
        printf("QUEUE: %s\n", current_queue->address_name);
        int i = 0;
        MessageEntry *current_message = current_queue->messages;
        while (current_message != NULL) {
            if (current_message->sender == NULL || current_message->recipient == NULL) {
                printf("%d) Malformed Message\n", i);
            } else {
                printf("%d) From:%s To: %s SUBJECT:%s\n", i,
                       current_message->sender->name,
                       current_message->recipient->name,
                       current_message->subject);
            }
            current_message = current_message->next;
            i++;
        }
        printf("\n");
        current_queue = current_queue->next;
    }
}

// Function: read_message
void read_message(char *param_1) {
    char *queue_name = param_1;
    char *msg_index_str = param_1;

    // Find the space or null terminator to separate queue name and message index
    while (*msg_index_str != ' ' && *msg_index_str != '\0') {
        msg_index_str++;
    }

    if (*msg_index_str == ' ') {
        *msg_index_str = '\0'; // Null-terminate queue name
        msg_index_str++;      // Move past space to message index
    } else {
        printf("Invalid READ command format. Usage: READ <queue_name> <message_index>\n");
        return;
    }

    int message_index = atoi(msg_index_str);
    if (message_index < 0) {
        printf("Invalid message index.\n");
        return;
    }

    QueueEntry *q_entry = locate_queue(queue_name);
    if (q_entry != NULL) {
        MessageEntry *current_message = q_entry->messages;
        for (int i = 0; i < message_index && current_message != NULL; ++i) {
            current_message = current_message->next;
        }

        if (current_message != NULL) {
            printf("%d) Subject:%s Body:%s\n", message_index,
                   current_message->subject,
                   current_message->body);
        } else {
            printf("Message not found at index %d in queue %s.\n", message_index, queue_name);
        }
    } else {
        printf("Mail Queue Not Found\n");
    }
}

// Function: main
int main(void) {
    // Seed PRNG with current time
    seed_prng(time(NULL));

    abook_global = (AddressBook *)calloc(1, sizeof(AddressBook));
    if (abook_global == NULL) {
        perror("Failed to allocate memory for address book");
        return 1;
    }

    initialize_address_book();
    initialize_mail_queues();

    char input_buffer[2501]; // Buffer for user input
    printf("sendmail:\n");

    while (1) {
        int bytes_read = receive_until(input_buffer, sizeof(input_buffer) - 1, 10);
        if (bytes_read < 1) {
            break; // Exit on empty input or error
        }

        char *command_start = input_buffer;
        char *arg_start = input_buffer;

        // Find the space to separate command and argument
        while (*arg_start != ' ' && *arg_start != '\0') {
            arg_start++;
        }

        if (*arg_start == ' ') {
            *arg_start = '\0'; // Null-terminate command
            arg_start++;      // Move past space to argument
        } else {
            // No argument, arg_start points to null terminator
        }

        if (strcmp(command_start, "LIST") == 0) {
            list_queue(arg_start);
        } else if (strcmp(command_start, "LISTALL") == 0) {
            list_all_queues();
        } else if (strcmp(command_start, "POST") == 0) {
            sendmail_post(arg_start);
        } else if (strcmp(command_start, "READ") == 0) {
            read_message(arg_start);
        } else if (strcmp(command_start, "ADDRESSBOOK") == 0) {
            print_address_book();
        } else if (strcmp(command_start, "QUIT") == 0) {
            break; // Exit loop
        } else {
            printf("Invalid Command!\n");
        }
    }

    printf("Goodbye.\n");

    // TODO: Implement proper memory cleanup (freeing linked lists, strings)
    // This is beyond the scope of "fix the snippet" but good practice for a complete program.

    return 0;
}