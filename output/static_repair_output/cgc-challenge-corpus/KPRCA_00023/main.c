#include <stdio.h>    // For printf, fgets
#include <string.h>   // For memset, strlen, strcmp
#include <stdbool.h>  // For bool type

// Define a context structure based on usage of 'af86'
typedef struct {
    void (*initializer)(void);
} AppContext;

// Declare and provide simple stub implementations for external functions
void print_greeting(AppContext *ctx) {
    printf("Welcome to the simple shell!\n");
}

void print_prompt(AppContext *ctx, bool new_session, bool error_state) {
    printf("Shell%s%s> ",
           new_session ? " [NEW]" : "",
           error_state ? " [ERROR]" : "");
}

int read_line(char *buffer) {
    if (fgets(buffer, 4096, stdin) == NULL) {
        return -1; // Error or EOF
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return 0; // Success
}

// handle_line returns:
// < 0: Signal to main loop to break (e.g., "exit" command).
// 0: Continue loop without special state change.
// 1: Set new_session = true.
// 2: Set error_state = true.
// 3: Set new_session = true, error_state = false.
int handle_line(AppContext *ctx, const char *line) {
    if (strcmp(line, "exit") == 0) {
        printf("Exiting shell.\n");
        return -1;
    } else if (strcmp(line, "new") == 0) {
        printf("Starting new session.\n");
        return 1;
    } else if (strcmp(line, "error") == 0) {
        printf("Entering error state.\n");
        return 2;
    } else if (strcmp(line, "reset") == 0) {
        printf("Resetting session.\n");
        return 3;
    } else if (strlen(line) == 0) {
        return 0;
    } else {
        printf("Unknown command: '%s'\n", line);
        return 0;
    }
}

// Placeholder for the initializer function
void app_initializer(void) {
    printf("Application context initialized.\n");
}

// Global context instance, 'af86' is interpreted as a pointer to this.
AppContext g_app_context = {
    .initializer = app_initializer
};

// Function: main
int main(void) {
  char input_buffer[4096];
  int handler_result;
  bool new_session = true;
  bool error_state = false;

  g_app_context.initializer();

  print_greeting(&g_app_context);

  while (true) {
    print_prompt(&g_app_context, new_session, error_state);
    new_session = false;

    memset(input_buffer, 0, sizeof(input_buffer));

    int read_status = read_line(input_buffer);

    if (read_status < 0) {
      break;
    }

    handler_result = handle_line(&g_app_context, input_buffer);

    if (handler_result < 0) {
      break;
    }

    if (handler_result == 1) {
      new_session = true;
    } else if (handler_result == 2) {
      error_state = true;
    } else if (handler_result == 3) {
      new_session = true;
      error_state = false;
    }
  }

  return 0;
}