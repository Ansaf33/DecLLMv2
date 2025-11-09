#include <stdlib.h> // For exit()
#include <stdint.h> // For uint32_t

// External declarations for functions called by main
// These functions are assumed to be defined elsewhere and linked.
extern int allocate();
extern int sendline();
extern int process_next_dgram();
extern void create_resp_pkt();
extern int senddgram();

// Global variables inferred from usage
// `curpkt` is assumed to be a pointer to a 4-byte unsigned integer.
// `DAT_000130b7` is assumed to be a pointer to some arbitrary data.
// `promptc` is assumed to be a function pointer to a function taking no arguments and returning void.
extern uint32_t *curpkt;
extern char *DAT_000130b7;
extern void (*promptc)(void);

int main(void) {
    int result; // Used for storing return values of functions and other integer results
    char *message = NULL; // Used for string literals, replaces pcStack_2c

    // Attempt to allocate resources or enter a special mode.
    // If allocate() returns non-zero, a specific "Clever girl." message is sent,
    // and the program exits based on the success/failure of sendline().
    result = allocate();
    if (result != 0) {
        message = "Clever girl.";
        result = sendline();
        if (result < 0) { // If sendline fails
            exit(6); // Terminate with error code 6
        }
        exit(9); // If sendline succeeds, terminate with code 9
    }

    // Main processing loop:
    // If allocate() returns 0, the program proceeds to process datagrams.
    // It continues calling process_next_dgram() until it returns a negative value (typically -1).
    do {
        result = process_next_dgram();
    } while (result >= 0);

    // After processing datagrams, create and send a response packet.
    create_resp_pkt();
    result = senddgram();
    if (result < 0) { // If senddgram fails
        exit(26); // Terminate with error code 26
    }

    // If all operations complete successfully, the program terminates normally.
    exit(0);

    // Note: The original snippet contained additional code after the final `_terminate()`.
    // In standard C, code following an unconditional `exit()` call is unreachable.
    // This often occurs in decompiled outputs where linear code generation
    // might place unreachable blocks or alternative code paths sequentially.
    // Adhering to structured C and avoiding `goto`, this unreachable code is omitted.
}