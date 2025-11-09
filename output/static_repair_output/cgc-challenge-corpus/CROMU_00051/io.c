#include <stdio.h>    // For printf
#include <string.h>   // For memset, strcpy, strlen
#include <unistd.h>   // For size_t, ssize_t (though not strictly needed if only using size_t)
#include <sys/socket.h> // For socket functions, though 'receive' is custom

// --- External declarations for functions/variables used but not defined in the snippet ---
// The original 'receive' has a custom signature:
// int receive(int sockfd, char *buf, int len, int *bytes_received_count_output);
extern int receive(int sockfd, char *buf, int len, int *bytes_received_count_output);

// Assuming ENV is an integer representing the number of history entries
extern int ENV;
// Assuming DAT_0002a921 is a pointer to the start of a memory block containing history strings,
// where each string is 256 bytes long (0x100).
extern char DAT_0002a921[];

// Assuming these are functions defined elsewhere
void EraseLine(int);
void PrintPrompt(void);
void CursorLeft(int);
// --- End of external declarations ---

// Function: ReadShellPrompt
size_t ReadShellPrompt(int param_1, char *param_2, int param_3) {
  char history_buffer[257]; // Buffer to temporarily store current input when navigating history
  char received_char;       // Stores the character received from the socket
  int bytes_received;       // Stores the number of bytes received by `receive`
  size_t cursor_pos = 0;    // Current position of the cursor within the input buffer
  int history_index = -1;   // Index for history navigation (-1 for no history selected)
  size_t current_len = 0;   // Current length of the input string in `param_2`
  int last_bytes_received = 1; // Tracks bytes received from the last `receive` call before loop exit

  // Initialize history_buffer to all zeros
  memset(history_buffer, 0, sizeof(history_buffer));

  // Main input loop, continues until a break condition is met
  while (1) {
    int receive_status; // Return status of the `receive` function

    // Attempt to receive a single character
    receive_status = receive(param_1, &received_char, 1, &bytes_received);

    // Handle immediate errors or peer closure
    if (receive_status != 0) { // Non-zero indicates an error
      last_bytes_received = 0; // Set flag to indicate error-based exit
      break; // Exit the main loop to handle finalization
    }
    if (bytes_received == 0) { // Peer closed connection
      *param_2 = '\0';         // Ensure buffer is null-terminated
      return 0;                // Immediate return
    }
    last_bytes_received = bytes_received; // Store for finalization logic

    // Check if adding another character would exceed buffer capacity
    if (current_len >= (size_t)param_3 - 1) {
      break; // Exit the main loop due to buffer full
    }

    // Process the received character based on its value
    switch (received_char) {
      case '\x7f': // ASCII DEL (usually Backspace)
        if (cursor_pos == current_len) { // Cursor is at the end of the line
          if (current_len > 0) { // Only delete if there's text
            printf("\b\b\b   \b\b\b"); // Erase last character visually
            current_len--;
            cursor_pos--;
          }
        } else { // Cursor is in the middle of the line
          if (cursor_pos > 0) { // Only delete if cursor is not at the beginning
            size_t k;
            // Shift characters to the left to remove the one before the cursor
            for (k = cursor_pos - 1; k < current_len - 1; k++) {
              param_2[k] = param_2[k + 1];
            }
            current_len--;
            cursor_pos--;
            EraseLine(0);    // Clear current line
            PrintPrompt();   // Reprint the shell prompt
            param_2[current_len] = '\0'; // Null-terminate for correct printing
            printf("%s", param_2);       // Print the modified buffer
            CursorLeft((int)(current_len - cursor_pos)); // Reposition cursor
          }
        }
        break;

      case '\x03': // Ctrl+C (ETX)
        return 0; // Exit immediately

      case '\x05': // Ctrl+E (ENQ - End of Transmission, used for End of Line)
        EraseLine(0);
        PrintPrompt();
        param_2[current_len] = '\0';
        printf("%s", param_2);
        cursor_pos = current_len; // Move cursor to the end
        break;

      case '\x15': // Ctrl+U (NAK - Negative Acknowledge, used for Clear Line)
        EraseLine(0);
        PrintPrompt();
        current_len = 0;
        cursor_pos = 0;
        param_2[0] = '\0'; // Clear the buffer
        break;

      case '\x04': // Ctrl+D (EOT - End of Transmission, used for EOF or exit)
        if (current_len == 0) { // If buffer is empty, treat as 'exit' command
          printf("\b\b  \b\b"); // Clear visual feedback
          printf("exit  ");
          strcpy(param_2, "exit");
          return 1; // Indicate successful 'exit' command
        }
        printf("\b\b  \b\b"); // Clear visual feedback, continue input
        break;

      case '\x1b': // Escape sequence (e.g., for arrow keys)
        // Read the next character, expecting '['
        receive_status = receive(param_1, &received_char, 1, &bytes_received);
        if (receive_status != 0 || bytes_received == 0) {
          *param_2 = '\0';
          return 0;
        }
        if (received_char != '[') { // Not an ANSI escape sequence we handle
          break; // Ignore this sequence and proceed to next input
        }

        // Read the next character, expecting arrow key codes (A, B, C, D)
        receive_status = receive(param_1, &received_char, 1, &bytes_received);
        if (receive_status != 0 || bytes_received == 0) {
          *param_2 = '\0';
          return 0;
        }
        last_bytes_received = bytes_received; // Update for finalization

        if (received_char == 'A') { // Up arrow (History previous)
          if (ENV == 0) { // No history available
            EraseLine(0);
            PrintPrompt();
            param_2[current_len] = '\0';
            printf("%s", param_2);
            cursor_pos = current_len;
          } else {
            if (history_index == -1) { // First time navigating history, save current input
              param_2[current_len] = '\0';
              strcpy(history_buffer, param_2);
            }
            history_index++;
            if (history_index >= ENV) { // Clamp history index to maximum
              history_index = ENV - 1;
            }
            EraseLine(0);
            PrintPrompt();
            char* history_entry = DAT_0002a921 + (size_t)history_index * 256; // Calculate address of history entry
            strcpy(param_2, history_entry);
            cursor_pos = strlen(history_entry);
            current_len = cursor_pos;
            printf("%s", history_entry);
          }
        } else if (received_char == 'B') { // Down arrow (History next)
          if (history_index == -1) { // Not currently navigating history
            EraseLine(0);
            PrintPrompt();
            param_2[current_len] = '\0';
            printf("%s", param_2);
          } else if (history_index == 0) { // Back to the saved current input
            EraseLine(0);
            PrintPrompt();
            strcpy(param_2, history_buffer);
            cursor_pos = strlen(param_2);
            current_len = cursor_pos;
            printf("%s", param_2);
            history_index = -1; // Reset history navigation state
          } else { // Navigate down to an older history entry
            history_index--;
            EraseLine(0);
            PrintPrompt();
            char* history_entry = DAT_0002a921 + (size_t)history_index * 256;
            strcpy(param_2, history_entry);
            cursor_pos = strlen(history_entry);
            current_len = cursor_pos;
            printf("%s", history_entry);
          }
        } else if (received_char == 'C') { // Right arrow (Cursor right)
          if (cursor_pos == current_len) { // Already at the end
            EraseLine(0);
            PrintPrompt();
            param_2[current_len] = '\0';
            printf("%s", param_2);
          } else {
            cursor_pos++;
            EraseLine(0);
            PrintPrompt();
            param_2[current_len] = '\0';
            printf("%s", param_2);
            if (current_len != cursor_pos) { // Only move cursor if not at end
              CursorLeft((int)(current_len - cursor_pos));
            }
          }
        } else if (received_char == 'D') { // Left arrow (Cursor left)
          if (cursor_pos == 0) { // Already at the beginning
            EraseLine(0);
            PrintPrompt();
            param_2[current_len] = '\0';
            printf("%s", param_2);
            CursorLeft((int)current_len); // Move cursor to the beginning (all chars left)
          } else {
            cursor_pos--;
            EraseLine(0);
            PrintPrompt();
            param_2[current_len] = '\0';
            printf("%s", param_2);
            CursorLeft((int)(current_len - cursor_pos));
          }
        }
        break; // End of escape sequence handling

      case '\r': // Carriage Return (CR) - End of line input
        param_2[cursor_pos] = '\0'; // Null-terminate at the cursor position
        break; // Exit the main loop

      case '\n': // Newline (LF) - Often part of CRLF, or standalone
        // Original code expects a following '\r' for CRLF
        receive_status = receive(param_1, &received_char, 1, &bytes_received);
        if (receive_status != 0 || bytes_received == 0) {
          *param_2 = '\0';
          return 0;
        }
        if (received_char != '\r') { // If not followed by CR, treat as error/incomplete
          *param_2 = '\0';
          return 0;
        }
        last_bytes_received = bytes_received; // Update for finalization
        param_2[cursor_pos] = '\0'; // Null-terminate at the cursor position
        break; // Exit the main loop

      default: // Any other regular character
        if (cursor_pos == current_len) { // Append character to the end
          param_2[current_len] = received_char;
          current_len++;
          cursor_pos++;
        } else { // Overwrite character at cursor position (as per original logic)
          param_2[cursor_pos] = received_char;
          cursor_pos++;
        }
        // Redraw the line and reposition the cursor
        EraseLine(0);
        PrintPrompt();
        param_2[current_len] = '\0'; // Null-terminate for printing
        printf("%s", param_2);
        if (current_len != cursor_pos) {
          CursorLeft((int)(current_len - cursor_pos));
        }
        break;
    }
  }

  // Finalization block (equivalent to original LAB_00012641)
  if (last_bytes_received == 0) { // If loop exited due to a receive error
    *param_2 = '\0';             // Ensure buffer is cleared
    current_len = 0;             // Indicate no valid input
  } else {
    param_2[current_len] = '\0'; // Ensure buffer is null-terminated at its actual length
  }
  return current_len; // Return the length of the read string
}

// Function: ReadUntilNewline
size_t ReadUntilNewline(int param_1, char *param_2, int param_3) {
  char received_char;       // Stores the character received from the socket
  int bytes_received;       // Stores the number of bytes received by `receive`
  size_t cursor_pos = 0;    // Current write position in the buffer
  size_t current_len = 0;   // Total length of the string in the buffer
  int last_bytes_received = 1; // Tracks bytes received from the last `receive` call before loop exit

  // Main input loop
  while (1) {
    int receive_status; // Return status of the `receive` function

    // Attempt to receive a single character
    receive_status = receive(param_1, &received_char, 1, &bytes_received);

    // Handle immediate errors or peer closure
    if (receive_status != 0) { // Error condition
      last_bytes_received = 0; // Set flag to indicate error-based exit
      break; // Exit the main loop to handle finalization
    }
    if (bytes_received == 0) { // Peer closed connection
      *param_2 = '\0';         // Ensure buffer is null-terminated
      return 0;                // Immediate return
    }
    last_bytes_received = bytes_received; // Store for finalization logic

    // Check if adding another character would exceed buffer capacity
    if (current_len >= (size_t)param_3 - 1) {
      break; // Exit the main loop due to buffer full
    }

    // Process the received character
    switch (received_char) {
      case '\x7f': // Backspace
        if (cursor_pos == 0) { // Cursor at the beginning, cannot backspace
          printf("\b\b  \b\b"); // Visual feedback for invalid backspace
        } else {
          printf("\b\b\b   \b\b\b"); // Erase last character visually
          cursor_pos--;
          current_len--; // Reduce total length
        }
        break;

      case '\x03': // Ctrl+C
        return 0; // Exit immediately

      case '\r': // Carriage Return (CR) - End of line input
        param_2[cursor_pos] = '\0'; // Null-terminate at the cursor position
        break; // Exit the main loop

      case '\n': // Newline (LF) - Often part of CRLF, or standalone
        // Original code expects a following '\r' for CRLF
        receive_status = receive(param_1, &received_char, 1, &bytes_received);
        if (receive_status != 0 || bytes_received == 0) {
          *param_2 = '\0';
          return 0;
        }
        if (received_char != '\r') { // If not followed by CR, treat as error/incomplete
          *param_2 = '\0';
          return 0;
        }
        last_bytes_received = bytes_received; // Update for finalization
        param_2[cursor_pos] = '\0'; // Null-terminate at the cursor position
        break; // Exit the main loop

      default: // Any other regular character
        param_2[cursor_pos] = received_char;
        cursor_pos++;
        if (cursor_pos > current_len) { // If appending, update total length
          current_len = cursor_pos;
        }
        break;
    }
  }

  // Finalization block (equivalent to original LAB_000127dd)
  if (last_bytes_received == 0) { // If loop exited due to a receive error
    *param_2 = '\0';             // Ensure buffer is cleared
    current_len = 0;             // Indicate no valid input
  } else {
    param_2[current_len] = '\0'; // Ensure buffer is null-terminated at its actual length
  }
  return current_len; // Return the length of the read string
}