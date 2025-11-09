#include <stdio.h>    // For snprintf, NULL
#include <stdlib.h>   // For exit, rand, srand, strtoul
#include <string.h>   // For strlen, memset, strcmp, strncpy
#include <unistd.h>   // For STDIN_FILENO, STDOUT_FILENO, read, ssize_t
#include <sys/socket.h> // For send (though unistd also has it for generic file descriptors)
#include <time.h>     // For time (to seed rand)
#include <ctype.h>    // For isdigit

// --- Forward Declarations & Global Data ---

// Renamed wrappers to avoid conflict with standard library functions
ssize_t my_send(int __fd, const void *__buf, size_t __n, int __flags);
ssize_t my_recv(int __fd, void *__buf, size_t __n, int __flags);

// Function prototypes for functions defined in the snippet
int prompt_user(const char *prompt_str, int fd_for_recv, void *buffer_for_recv, size_t buffer_size_for_recv);
int send_random_joke(unsigned int *jokes_data_base_ptr);
int send_joke(unsigned int *joke_ptr);
int send_all_jokes(unsigned int *jokes_data_base_ptr);
int do_list(unsigned int *jokes_data_ptr);
int do_add(unsigned int *jokes_data_ptr);
int do_show(unsigned int *jokes_data_ptr);
int do_count(unsigned int *jokes_data_ptr);
int do_menu(void);
int do_help(void);
int do_quit(void);

// Placeholder for external/undefined functions in the original snippet.
// These are simplified to allow compilation and simulate basic behavior.
// In a real application, these would have proper implementations.

// Simulates sending all bytes, returns bytes sent or -1 on error.
ssize_t sendall(int fd, const void *buf, size_t len) {
    ssize_t bytes_sent = send(fd, buf, len, 0);
    if (bytes_sent == -1) {
        perror("sendall error");
    }
    return bytes_sent;
}

// Simulates reading a line, returns bytes read or -1 on error.
// Fills `buf` up to `len` bytes and null-terminates it.
ssize_t recvline(int fd, void *buf, size_t len) {
    ssize_t bytes_read = read(fd, buf, len);
    if (bytes_read > 0) {
        // Remove trailing newline if present and null-terminate
        if (((char*)buf)[bytes_read - 1] == '\n') {
            ((char*)buf)[bytes_read - 1] = '\0';
        } else {
            ((char*)buf)[bytes_read] = '\0'; // Null-terminate
        }
    } else if (bytes_read == 0) {
        // EOF
        if (len > 0) ((char*)buf)[0] = '\0';
    } else {
        perror("recvline error");
    }
    return bytes_read;
}

// Assumed joke structure based on 0x204 bytes (516) found in do_show and main
// A joke entry consists of: unsigned int ID (4 bytes) + char text[512] (512 bytes) = 516 bytes
#define JOKE_TEXT_MAX_LEN 512
#define JOKE_STRUCT_SIZE_IN_BYTES (sizeof(unsigned int) + JOKE_TEXT_MAX_LEN)
#define JOKE_STRUCT_SIZE_IN_UINTS (JOKE_STRUCT_SIZE_IN_BYTES / sizeof(unsigned int)) // 129 unsigned ints per joke

// Global storage for jokes.
// jokes_data_global[0] stores the current count of jokes.
// jokes_data_global[1] onwards stores the joke structures.
// We allocate space for 100 jokes, plus one unsigned int for the count.
unsigned int jokes_data_global[1 + 100 * JOKE_STRUCT_SIZE_IN_UINTS];

// Returns the current count of jokes from the global array.
unsigned int joke_count(void) {
    return jokes_data_global[0];
}

// Inserts a new joke into the global joke array.
// Returns the ID of the new joke on success, -1 if joke limit reached, -2 if joke text is empty.
int insert_joke(unsigned int *jokes_data_ptr, char *new_joke_text) {
    unsigned int current_count = jokes_data_ptr[0];
    const unsigned int MAX_JOKES = 100; // Maximum number of jokes allowed
    
    if (current_count >= MAX_JOKES) {
        return -1; // Joke limit reached
    }
    if (strlen(new_joke_text) == 0) {
        return -2; // Empty joke is considered invalid
    }

    // Calculate pointer to the new joke's ID field within the array
    unsigned int *new_joke_slot_id_ptr = jokes_data_ptr + 1 + current_count * JOKE_STRUCT_SIZE_IN_UINTS;
    // Calculate pointer to the new joke's text field (immediately after the ID)
    char *new_joke_slot_text_ptr = (char*)(new_joke_slot_id_ptr + 1);

    jokes_data_ptr[0]++; // Increment count stored at the beginning of the array
    *new_joke_slot_id_ptr = current_count; // Assign ID (0-indexed)
    strncpy(new_joke_slot_text_ptr, new_joke_text, JOKE_TEXT_MAX_LEN - 1); // Copy text
    new_joke_slot_text_ptr[JOKE_TEXT_MAX_LEN - 1] = '\0'; // Ensure null termination
    
    return current_count; // Return the ID of the newly added joke
}

// Checks if a string contains only digits. Returns 0 if numeric, -1 otherwise.
int is_numeric(char *str) {
    if (str == NULL || *str == '\0') {
        return -1; // Not numeric (empty or NULL)
    }
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!isdigit((unsigned char)str[i])) {
            return -1; // Contains non-digit characters
        }
    }
    return 0; // All characters are digits
}

// Compares two strings. Returns 0 if equal, non-zero otherwise (like strcmp).
int streq(char *s1, char *s2) {
    return strcmp(s1, s2);
}

// Converts a string to an unsigned 32-bit integer.
unsigned int str2uint32(char *str) {
    return (unsigned int)strtoul(str, NULL, 10);
}

// Loads default jokes into the global joke array.
void load_default_jokes(void) {
    jokes_data_global[0] = 0; // Initialize joke count to 0
    
    char *default_jokes[] = {
        "Chuck Norris doesn't read books. He stares them down until he gets the information he wants.",
        "Time waits for no man. Unless that man is Chuck Norris.",
        "Chuck Norris can slam a revolving door.",
        "Chuck Norris counted to infinity. Twice."
    };
    
    for (int i = 0; i < sizeof(default_jokes) / sizeof(default_jokes[0]); ++i) {
        insert_joke(jokes_data_global, default_jokes[i]);
    }
}

// Terminates the program with a given status code.
void _terminate(int status) {
    exit(status);
}

// --- Functions from Supplied Snippet (Fixed) ---

// Function: send (renamed to my_send to avoid conflict with standard library send)
ssize_t my_send(int __fd, const void *__buf, size_t __n, int __flags) {
  // Original decompiled call was `sendall(1,__fd,__buf,0x117d5);`
  // We assume `sendall` is a helper for sending all bytes and takes (fd, buf, len).
  // The '1' and '0x117d5' are treated as decompiler artifacts or internal flags.
  // We use `__n` as the length argument to `sendall` for consistency with the `send` signature.
  ssize_t bytes_sent = sendall(__fd, __buf, __n);
  if (bytes_sent == -1 || (size_t)bytes_sent != __n) { // Check for error or incomplete send
    _terminate(1);
  }
  return 0; // Original snippet's `send` function returns 0 on success.
}

// Function: recv (renamed to my_recv to avoid conflict with standard library recv)
ssize_t my_recv(int __fd, void *__buf, size_t __n, int __flags) {
  // Original decompiled call was `recvline(0,__fd);`
  // `__buf` and `__n` from `my_recv` were not directly passed to `recvline` in the original snippet.
  // We adjust `recvline`'s signature and usage to fill `__buf` up to `__n` bytes.
  ssize_t bytes_received = recvline(__fd, __buf, __n);
  if (bytes_received < 0) { // recvline returns -1 on error
    const char *error_msg = "\nAre you kidding me? What is that garbage? I thought my instructions were pretty straight forward. Try again.\n";
    send(STDOUT_FILENO, error_msg, strlen(error_msg), 0); // Use standard library `send`
    _terminate(2);
  }
  return 0; // Original snippet's `recv` function returns 0 on success.
}

// Function: prompt_user
// Fixed arguments to provide necessary buffer and size information.
int prompt_user(const char *prompt_str, int fd_for_recv, void *buffer_for_recv, size_t buffer_size_for_recv) {
  send(STDOUT_FILENO, prompt_str, strlen(prompt_str), 0); // Use standard library `send` for prompt
  my_recv(fd_for_recv, buffer_for_recv, buffer_size_for_recv, 0); // Use `my_recv` to read user input
  return 0;
}

// Function: send_random_joke
int send_random_joke(unsigned int *jokes_data_base_ptr) {
  unsigned int total_jokes = joke_count();
  if (total_jokes == 0) {
    // No jokes to send, handle this case gracefully.
    const char *no_jokes_msg = "No jokes available yet. Why don't you add one?\n";
    send(STDOUT_FILENO, no_jokes_msg, strlen(no_jokes_msg), 0);
    return -1;
  }
  
  unsigned int random_index = rand() % total_jokes;
  
  // Calculate the pointer to the selected joke's ID within the global array.
  // `jokes_data_base_ptr + 1` points to the start of actual joke data (after the count).
  // `random_index * JOKE_STRUCT_SIZE_IN_UINTS` moves to the correct joke structure.
  unsigned int *selected_joke_ptr = jokes_data_base_ptr + 1 + random_index * JOKE_STRUCT_SIZE_IN_UINTS;
  send_joke(selected_joke_ptr);
  return 0;
}

// Function: send_joke
int send_joke(unsigned int *joke_ptr) {
  // `joke_ptr` points to the `unsigned int id` of the joke.
  // `(char *)(joke_ptr + 1)` points to the `char text[]` part of the joke structure.
  size_t joke_text_len = strlen((char *)(joke_ptr + 1));
  
  // Calculate buffer size for `snprintf`: `ID: joke_text\n\0`
  // Max ID (10 digits) + ": " (2 chars) + joke_text_len + "\n" (1 char) + "\0" (1 char)
  size_t required_buffer_size = 10 + 2 + joke_text_len + 1 + 1;
  
  // Use a Variable Length Array (VLA) for the output buffer.
  // VLAs are a C99 feature, commonly supported by GCC/Clang.
  char output_buffer[required_buffer_size];

  // Format the joke string: "%u: %s\n"
  snprintf(output_buffer, required_buffer_size, "%u: %s\n", *joke_ptr, (char *)(joke_ptr + 1));
  
  send(STDOUT_FILENO, output_buffer, strlen(output_buffer), 0); // Use standard library `send`
  return 0;
}

// Function: send_all_jokes
int send_all_jokes(unsigned int *jokes_data_base_ptr) {
  unsigned int total_jokes = jokes_data_base_ptr[0]; // Get joke count from the first element
  for (unsigned int i = 0; i < total_jokes; ++i) {
    // Calculate pointer to the current joke's ID.
    unsigned int *current_joke_ptr = jokes_data_base_ptr + 1 + i * JOKE_STRUCT_SIZE_IN_UINTS;
    send_joke(current_joke_ptr);
  }
  return 0;
}

// Function: do_list
int do_list(unsigned int *jokes_data_ptr) {
  send_all_jokes(jokes_data_ptr);
  return 0;
}

// Function: do_add
int do_add(unsigned int *jokes_data_ptr) {
  char joke_buffer[JOKE_TEXT_MAX_LEN] = {0}; // Buffer for user input joke
  
  const char *prompt_msg1 = "So, you think you have a good Chuck Norris joke? Give me the joke string already....\n";
  send(STDOUT_FILENO, prompt_msg1, strlen(prompt_msg1), 0);
  
  prompt_user("ADD> ", STDIN_FILENO, joke_buffer, sizeof(joke_buffer) - 1);
  
  int joke_id = insert_joke(jokes_data_ptr, joke_buffer);
  
  if (joke_id == -1) {
    const char *error_msg_limit = "Lordy, lordy, I\'ve had enough Chuck Norris jokes. Go away.\n";
    send(STDOUT_FILENO, error_msg_limit, strlen(error_msg_limit), 0);
  } else if (joke_id < -1) { // e.g., -2 for crap joke from insert_joke
    const char *error_msg_full = "\nThat joke you gave me is crap! For that, you get to start over!\n";
    send(STDOUT_FILENO, error_msg_full, strlen(error_msg_full), 0);
  } else { // joke_id >= 0 (success)
    const char *success_format = "Joke added. Thanks for sharing! Your joke is ID: %u\n";
    // Buffer for the success message with ID
    size_t required_buffer_size = strlen(success_format) + 10 + 1; // Max 10 digits for uint, +1 for null
    char output_buffer[required_buffer_size];
    snprintf(output_buffer, required_buffer_size, success_format, joke_id);
    send(STDOUT_FILENO, output_buffer, strlen(output_buffer), 0);
  }
  return 0;
}

// Function: do_show
int do_show(unsigned int *jokes_data_ptr) {
  char input_buffer[32] = {0}; // Buffer for user input (ID or "RANDOM")
                               // Original used 0xb (11 bytes), 32 is a safer size.
  
  const char *prompt_msg = "Give me the ID of the joke you want to read. Or better yet, enter RANDOM and I\'ll choose one for you.\n";
  send(STDOUT_FILENO, prompt_msg, strlen(prompt_msg), 0);
  
  while (1) { // Loop until a valid action is performed or program exits
    memset(input_buffer, 0, sizeof(input_buffer));
    prompt_user("SHOW> ", STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1);
    
    // Check if input is "RANDOM"
    if (streq(input_buffer, "RANDOM") == 0) { // strcmp returns 0 if equal
      send_random_joke(jokes_data_ptr);
      return 0; // Exit do_show after sending random joke
    }
    
    // Check if input is numeric
    if (is_numeric(input_buffer) == 0) { // is_numeric returns 0 if all digits
      unsigned int joke_id = str2uint32(input_buffer);
      
      if (joke_id == 0x539) { // Magic number 1337
        const char *secret_msg = "Congratulations, you are the only person in the past, present or future to beat Chuck Norris.\n";
        send(STDOUT_FILENO, secret_msg, strlen(secret_msg), 0);
        return 0; // Exit do_show after secret message
      }
      
      unsigned int total_jokes = joke_count();
      if (joke_id < total_jokes) {
        // Calculate pointer to the selected joke's ID.
        unsigned int *selected_joke_ptr = jokes_data_ptr + 1 + joke_id * JOKE_STRUCT_SIZE_IN_UINTS;
        send_joke(selected_joke_ptr);
        return 0; // Exit do_show after sending specific joke
      }
    }
    
    // If input was not "RANDOM" and not a valid numeric ID, print error and retry
    const char *bogus_id_msg = "\nWhat do you take me for? That ID is bogus! Try again.\n";
    send(STDOUT_FILENO, bogus_id_msg, strlen(bogus_id_msg), 0);
  }
}

// Function: do_count
int do_count(unsigned int *jokes_data_ptr) {
  unsigned int count = joke_count(); // Get the current joke count
  
  const char *format_str = "Current CNMP joke count is %u. Enjoy!\n";
  // Buffer for the formatted message
  size_t required_buffer_size = strlen(format_str) + 10 + 1; // Max 10 digits for uint, +1 for null
  char output_buffer[required_buffer_size];
  
  snprintf(output_buffer, required_buffer_size, format_str, count);
  send(STDOUT_FILENO, output_buffer, strlen(output_buffer), 0);
  return 0;
}

// Function: do_menu
int do_menu(void) {
  const char *menu_msg = "Enter one of these commands:\n  LIST  - list all jokes\n  ADD   - upload your own joke\n  COUNT - find out how many jokes we have to offer you\n  SHOW  - show a joke\n  MENU  - display this menu\n  HELP  - you gotta be kididin\' me!\n  QUIT  - take off eh?\n";
  send(STDOUT_FILENO, menu_msg, strlen(menu_msg), 0);
  return 0;
}

// Function: do_help
int do_help(void) {
  const char *help_msg = "Is our menu so extensive that you cannot remember all of the commands? Here, let me jog all 6 of your neurons:\n";
  send(STDOUT_FILENO, help_msg, strlen(help_msg), 0);
  do_menu(); // Display the menu as part of help
  return 0;
}

// Function: do_quit
int do_quit(void) {
  const char *quit_msg = "Thanks for the laughs. Come back soon!\n";
  send(STDOUT_FILENO, quit_msg, strlen(quit_msg), 0);
  return 0; // The original snippet returns 0. `main` will handle program termination.
}

// Function: main
int main(void) {
  // Seed the random number generator using current time
  srand(time(NULL));

  // Initialize the global joke storage and load default jokes
  load_default_jokes();
  
  const char *welcome_msg = "Welcome to the Chuck Norris Joke Server!\n";
  send(STDOUT_FILENO, welcome_msg, strlen(welcome_msg), 0);
  
  do_menu(); // Display the initial menu of commands
  
  char input_command[32]; // Buffer to store user's command input
  
  while (1) { // Main command processing loop
    memset(input_command, 0, sizeof(input_command)); // Clear the input buffer
    prompt_user("CMD> ", STDIN_FILENO, input_command, sizeof(input_command) - 1);
    
    // Process user commands
    if (streq(input_command, "LIST") == 0) {
      do_list(jokes_data_global);
    } else if (streq(input_command, "ADD") == 0) {
      do_add(jokes_data_global);
    } else if (streq(input_command, "COUNT") == 0) {
      do_count(jokes_data_global);
    } else if (streq(input_command, "SHOW") == 0) {
      do_show(jokes_data_global);
    } else if (streq(input_command, "MENU") == 0) {
      do_menu();
    } else if (streq(input_command, "HELP") == 0) {
      do_help();
    } else if (streq(input_command, "QUIT") == 0) {
      do_quit();
      break; // Exit the main loop, leading to program termination
    } else {
      const char *unknown_cmd_msg = "Unknown command. Type 'MENU' for options.\n";
      send(STDOUT_FILENO, unknown_cmd_msg, strlen(unknown_cmd_msg), 0);
    }
  }
  
  return 0; // Indicate successful program execution
}