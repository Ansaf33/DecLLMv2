#include <stdio.h>   // For printf
#include <string.h>  // For strlen, memcpy, strncasecmp, strncmp, strcpy, strsep
#include <stdlib.h>  // For malloc, free, exit

// External declarations for functions/globals not defined in the snippet
extern char *cmd_lbls[];
extern int read_line(char *buffer); // Assuming read_line takes a buffer and returns bytes read or -1 on error.

// Placeholder definitions for format strings from decompiled output
const char *FORMAT_STR_DAT_0001588c = "%s=\n";
const char *FORMAT_STR_DAT_000158ab = "%s\n";
const char *FORMAT_STR_DAT_000158b5 = "%s: ";

// Type definitions for clarity, based on observed memory offsets and sizes (assuming 32-bit pointers/ints)
// The code primarily uses these structs for access, which is safer and more readable than raw pointer arithmetic.

// A structure representing a 'Question' block in memory.
typedef struct QuestionBlock {
    char *name;             // Offset 0x00
    char *prompt_suffix;    // Offset 0x04
    char *answer;           // Offset 0x08
    char is_optional;       // Offset 0x0c (0 = not optional, non-0 = optional)
    char _pad0[7];          // Padding to 0x14
    struct QuestionBlock *next_question_in_list; // Offset 0x14
    int (*validation_func)(char *); // Offset 0x18
} QuestionBlock;

// A structure representing a 'Page' block in memory.
typedef struct PageBlock {
    char *name;             // Offset 0x00
    struct PageBlock *prev_page;        // Offset 0x04
    struct PageBlock *next_page;        // Offset 0x08
    QuestionBlock *questions_list_head; // Offset 0x0c
    char is_complete;       // Offset 0x10 (0 = false, non-0 = true)
    char _pad0[3];          // Padding to 0x14
} PageBlock;

// Forward declaration for AppContext to define CommandHandler
typedef struct AppContext_s AppContext;

// Type for command handler functions
typedef int (*CommandHandler)(AppContext *, char *);

// A structure representing the main 'Application Context' or state.
typedef struct AppContext_s {
    void *_dummy_0x00;      // Offset 0x00 (The base address of the struct itself, often unused explicitly)
    char *app_name;         // Offset 0x04
    char *help_message;     // Offset 0x08
    CommandHandler *command_handlers_array_base; // Offset 0x10 (Base address of an array of CommandHandler function pointers)
    char _pad0[0x28 - 0x14]; // Padding to align current_page at 0x28 (0x14 because command_handlers_array_base is 4 bytes)
    PageBlock *current_page;     // Offset 0x28
    QuestionBlock *current_question; // Offset 0x2c
} AppContext;


// Function: handler_index
int handler_index(char *param_2) {
  char *ptr = param_2;
  char buffer[16];

  // Skip leading asterisks, but only up to 2 characters
  while (*ptr == '*' && (ptr - param_2 < 2)) {
    ptr++;
  }

  for (int idx = 0; idx < 5; idx++) { // Loop up to 4, so idx < 5
    size_t len = strlen(cmd_lbls[idx]);
    if (len >= sizeof(buffer)) { // Prevent buffer overflow if command label is too long
        // Command label too long for buffer, treat as not found
        continue; // Or return -1, depending on desired error handling
    }
    memcpy(buffer, ptr, len);
    buffer[len] = '\0'; // Ensure null termination

    if (strncasecmp(buffer, cmd_lbls[idx], len) == 0) {
      return idx;
    }
  }
  return -1;
}

// Function: is_cmd
unsigned int is_cmd(AppContext *ctx, char *param_2) {
  if (strncmp("**", param_2, 2) == 0) {
    int index = handler_index(param_2);
    return (index != -1); // Returns 1 if command found, 0 otherwise
  }
  return 0;
}

// Function: page_complete
int page_complete(AppContext *ctx) {
  int all_required_answered = 1;
  QuestionBlock *current_q_block = ctx->current_page->questions_list_head;

  while (current_q_block != NULL) {
    // Check if the question is NOT optional (is_optional == 0) AND has NO answer (answer == NULL)
    if ((current_q_block->is_optional == '\0') && (current_q_block->answer == NULL)) {
      all_required_answered = 0; // Found an unanswered required question
      break;
    }
    current_q_block = current_q_block->next_question_in_list;
  }

  // Page is complete if all required questions are answered AND
  // the current question pointer (ctx->current_question) is NULL,
  // indicating all questions on the page have been processed or the page is empty.
  return (all_required_answered && (ctx->current_question == NULL));
}

// Function: print_title
void print_title(AppContext *ctx) {
  if (strncmp(ctx->current_page->name, "Fin", 3) == 0) {
    printf("\nFinal Questions\n");
  } else {
    printf("\n%s Form\n", ctx->current_page->name);
  }
}

// Function: handle_cmd
int handle_cmd(AppContext *ctx, char *param_2) {
  size_t len = strlen(param_2);
  char *input_copy = (char *)malloc(len + 1);
  if (input_copy == NULL) {
    return -1;
  }
  strcpy(input_copy, param_2);

  char *temp_input_ptr = input_copy; // strsep modifies its first argument
  char *command_token = strsep(&temp_input_ptr, " ");

  int handler_idx = handler_index(command_token);
  int result = handler_idx; // Default to handler_idx for return if no handler found or error

  if (handler_idx >= 0) {
    // Call the command handler function.
    // The command_handlers_array_base is assumed to be at offset 0x10 within AppContext.
    // `((CommandHandler*)((char*)ctx + 0x10))` gets the array base.
    // `[handler_idx]` accesses the specific handler.
    CommandHandler *handlers = (CommandHandler *)((char *)ctx + 0x10);
    result = handlers[handler_idx](ctx, temp_input_ptr);
  }

  free(input_copy); // Free the malloc'd copy of the input string
  return result;
}

// Function: handle_answer
int handle_answer(AppContext *ctx, char *param_2) {
  QuestionBlock *current_q_block = ctx->current_question;

  // Call the validation function for the current question
  if (current_q_block->validation_func(param_2) == 0) {
    return 1; // Validation failed
  } else {
    // If there's an existing answer, free it first
    if (current_q_block->answer != NULL) {
      free(current_q_block->answer);
    }

    size_t len = strlen(param_2);
    char *new_answer = (char *)malloc(len + 1);
    if (new_answer == NULL) {
      return -1; // Memory allocation failed
    }
    strcpy(new_answer, param_2);
    current_q_block->answer = new_answer;
    return 0; // Success
  }
}

// Function: next_question
int next_question(AppContext *ctx) {
  if (ctx->current_question == NULL) {
    return -1; // No current question
  } else {
    if (ctx->current_question->next_question_in_list == NULL) {
      ctx->current_page->is_complete = 1; // Mark current page as complete
    }
    ctx->current_question = ctx->current_question->next_question_in_list;
    return 0; // Success
  }
}

// Function: print_page
void print_page(AppContext *ctx) {
  // Print page title based on name
  if (strncmp(ctx->current_page->name, "Edu", 3) == 0) {
    printf("\n*********Highest %s:*********\n", ctx->current_page->name);
  } else if (strncmp(ctx->current_page->name, "Emp", 3) == 0) {
    printf("\n*********Most Recent Employer:*********\n");
  } else if (strncmp(ctx->current_page->name, "Fin", 3) == 0) {
    printf("\n*********Final Screening:*********\n");
  } else {
    printf("\n*********%s:*********\n", ctx->current_page->name);
  }

  // Iterate through questions and print their status
  QuestionBlock *current_q_block = ctx->current_page->questions_list_head;
  for (; current_q_block != NULL; current_q_block = current_q_block->next_question_in_list) {
    if (current_q_block->answer == NULL) {
      printf(FORMAT_STR_DAT_0001588c, current_q_block->name);
    } else {
      printf("%s=%s\n", current_q_block->name, current_q_block->answer);
    }
  }
}

// Function: prompt_next
void prompt_next(void) {
  printf("\nType **next to continue\n");
}

// Function: next_page
int next_page(AppContext *ctx) {
  if (ctx->current_page == NULL) {
    return -1; // No current page
  } else if (ctx->current_page->next_page == NULL) {
    return 1; // No next page (end of form)
  } else {
    ctx->current_page = ctx->current_page->next_page;
    ctx->current_question = ctx->current_page->questions_list_head; // Reset current question for new page
    return 0; // Success
  }
}

// Function: print_next_title
void print_next_title(AppContext *ctx) {
  if (ctx->current_page->next_page == NULL) {
    printf(FORMAT_STR_DAT_000158ab, ctx->app_name);
  } else {
    PageBlock *next_page_block = ctx->current_page->next_page;
    if (strncmp(next_page_block->name, "Fin", 3) == 0) {
      printf("\nFinal Questions\n");
    } else {
      printf("\n%s Form\n", next_page_block->name);
    }
  }
}

// Function: prompt_q
void prompt_q(QuestionBlock *q_block) {
  if (q_block->prompt_suffix == NULL) {
    printf(FORMAT_STR_DAT_000158b5, q_block->name);
  } else {
    printf("%s%s: ", q_block->name, q_block->prompt_suffix);
  }
}

// Function: handle_next
int handle_next(AppContext *ctx) {
  if (ctx->current_page == NULL) {
    return 1; // No current page, implies end or error
  } else if (ctx->current_page->is_complete == '\0') { // Check if page is NOT complete
    print_next_title(ctx);
    printf("You must complete the previous page before proceeding to this page\n");
    return 1; // Page not complete, cannot proceed
  } else {
    int next_page_result = next_page(ctx);
    if (next_page_result < 0) {
      return -1; // Error during next_page
    } else if (next_page_result == 0) { // Successfully moved to next page
      // Skip already answered questions on the new page
      while (ctx->current_question != NULL &&
             ctx->current_question->next_question_in_list != NULL &&
             ctx->current_question->answer != NULL) {
        ctx->current_question = ctx->current_question->next_question_in_list;
      }
      return 1; // Indicate successful navigation to next page, ready for prompt
    } else { // next_page_result == 1, meaning no next page (end of form)
      return 2; // Indicate end of form
    }
  }
}

// Function: handle_exit
void handle_exit(void) {
  printf("Thank you!\n");
  exit(0);
}

// Function: handle_help
int handle_help(AppContext *ctx) {
  printf(FORMAT_STR_DAT_000158ab, ctx->help_message);
  return 1; // Assuming 1 indicates success or handled
}

// Function: handle_prev
int handle_prev(AppContext *ctx) {
  if (ctx->current_page == NULL) {
    return -1; // No current page
  } else if (ctx->current_page->prev_page == NULL) {
    print_title(ctx); // Already on the first page, just print title
    return 0; // Indicate staying on current (first) page
  } else {
    ctx->current_page = ctx->current_page->prev_page;
    ctx->current_question = ctx->current_page->questions_list_head; // Reset current question for new page
    return 1; // Indicate successful navigation to previous page
  }
}

// Function: handle_update
int handle_update(AppContext *ctx, char *param_2) {
  if (ctx->current_page->is_complete == '\0') { // Check if page is NOT complete
    printf("Cannot update field until all fields are inputted\n");
    return 1;
  } else if (param_2 == NULL) {
    return 1;
  } else {
    QuestionBlock *target_q_block = NULL;
    // Iterate through questions on the current page to find the target field
    QuestionBlock *current_q_iter = ctx->current_page->questions_list_head;
    for (; current_q_iter != NULL; current_q_iter = current_q_iter->next_question_in_list) {
      size_t name_len = strlen(current_q_iter->name);
      // strncmp for partial match, assuming param_2 is just the field name
      if (strncmp(current_q_iter->name, param_2, name_len) == 0 && strlen(param_2) == name_len) {
        target_q_block = current_q_iter;
        break;
      }
    }

    if (target_q_block == NULL) {
      printf("Could not find specified field\n");
      printf("Update Unsuccessful\n");
      return 1;
    } else {
      char *input_buffer = (char *)malloc(4096); // Allocate buffer for user input
      if (input_buffer == NULL) {
        return -1; // Memory allocation failed
      }

      prompt_q(target_q_block); // Prompt user for new value
      int bytes_read = read_line(input_buffer);
      if (bytes_read < 0) {
        free(input_buffer);
        return -1; // Error reading input
      }

      size_t input_len = strlen(input_buffer);
      
      // If input is empty: if question IS optional, clear the answer.
      // If question is NOT optional, it's an error.
      if (input_len == 0 && target_q_block->is_optional != '\0') { // Input is empty AND question IS optional
          if (target_q_block->answer != NULL) {
              free(target_q_block->answer);
          }
          target_q_block->answer = NULL; // Clear the answer
          free(input_buffer); // Free the temporary input buffer
          return 1; // Success (optional field cleared)
      } else if (input_len == 0 && target_q_block->is_optional == '\0') { // Input is empty AND question is NOT optional
          free(input_buffer);
          printf("Required field cannot be empty.\n");
          printf("Update Unsuccessful\n");
          return 1; // Indicate failure
      } else { // Input is not empty
        if (target_q_block->validation_func(input_buffer) == 0) { // Validation failed
          free(input_buffer);
          printf("Bad input.\n");
          printf("Update Unsuccessful\n");
          return 1; // Indicate failure
        } else {
          // Validation passed, update the answer
          if (target_q_block->answer != NULL) {
            free(target_q_block->answer);
          }
          target_q_block->answer = input_buffer; // Transfer ownership of malloc'd buffer
          return 1; // Success
        }
      }
    }
  }
}

// Function: print_prompt
void print_prompt(AppContext *ctx, int should_print_title) {
  if (ctx->current_page == NULL) { // No current page
    printf(FORMAT_STR_DAT_000158ab, ctx->app_name);
  } else {
    if (should_print_title != 0) {
      print_title(ctx);
    }
    if (ctx->current_page->is_complete == '\0') { // Check if page is NOT complete
      prompt_q(ctx->current_question);
    } else {
      print_page(ctx);
      prompt_next();
    }
  }
}

// Function: handle_line
int handle_line(AppContext *ctx, char *param_2) {
  if (is_cmd(ctx, param_2) == 0) { // Not a command, treat as an answer
    // Check if current page is NOT complete AND there's a current question
    if (ctx->current_page->is_complete == '\0' && ctx->current_question != NULL) {
      size_t len = strlen(param_2);
      if (len == 0) {
        // Original logic: if input is empty AND question is NOT optional, return 0 (success).
        // This allows skipping required questions with empty input. This is likely a bug.
        // Preserving original behavior for now.
        if (ctx->current_question->is_optional == '\0') { // Question is NOT optional
          // Treat as success and proceed to next question, even if empty for a required field.
          // A more robust implementation would return an error here.
          // Fall through to next_question
        } else {
          // If input is empty and optional, just move to next question (no answer to store).
          // Fall through to next_question
        }
      } else {
        int answer_result = handle_answer(ctx, param_2);
        if (answer_result < 0) {
          return -1; // Error handling answer (e.g., malloc failed)
        }
        if (answer_result != 0) { // handle_answer returns 1 on validation fail
          return 0; // Validation failed, but continue (re-prompt on current question)
        }
      }
      // If answer was handled successfully or skipped (empty for required/optional), move to next question
      int next_q_result = next_question(ctx);
      if (next_q_result < 0) {
        return -1; // Error moving to next question
      } else {
        return 0; // Success
      }
    } else { // Page is complete or no current question, but input is not a command
      return 1; // Indicate that input was not processed (e.g., prompt for next command)
    }
  } else { // Input is a command
    return handle_cmd(ctx, param_2);
  }
}

// Function: print_greeting
void print_greeting(AppContext *ctx) {
  printf(FORMAT_STR_DAT_000158ab, ctx->app_name);
}