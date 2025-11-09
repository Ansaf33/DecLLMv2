#include <stdio.h>    // For fprintf, stdout, stderr, vfprintf
#include <string.h>   // For memset, strcmp, strlen, strsep, strcat
#include <stdlib.h>   // For exit, strtol, NULL, malloc
#include <unistd.h>   // For read
#include <stdarg.h>   // For va_list in fdprintf

// --- Mock/External Declarations ---

// Assuming 'bloomy' is a global pointer to a bloomy filter structure
typedef struct bloomy_filter {
    // fields for bloom filter implementation (simplified for mock)
    int size;
    void *hash_func1;
    void *hash_func2;
    void *hash_func3;
} bloomy_filter_t;

bloomy_filter_t *bloomy = NULL;

// Mock values for bloomy_new parameters (likely function pointers in a real scenario)
void *moomoo = (void*)0x1;
void *yooyoo = (void*)0x2;
void *geegee = (void*)0x3;

// Assuming 'depressing_words' is a global array of char pointers
// The original code uses 0x21 (33) as the size.
char *depressing_words[] = {
    "sad", "lonely", "depressed", "unhappy", "gloom", "misery", "sorrow", "pain",
    "despair", "hopeless", "down", "melancholy", "distress", "anguish", "woe",
    "grief", "heartbreak", "dismal", "bleak", "mourn", "weep", "cry", "tears",
    "suffering", "torment", "burden", "dark", "empty", "lost", "broken", "fear",
    "anxiety", "stress" // 33 words
};
const unsigned int DEPRESSING_WORDS_COUNT = sizeof(depressing_words) / sizeof(depressing_words[0]);

// Mock fdprintf function using standard fprintf
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = -1;
    if (fd == 1) { // Assuming fd 1 is stdout
        ret = vfprintf(stdout, format, args);
    } else if (fd == 2) { // Assuming fd 2 is stderr
        ret = vfprintf(stderr, format, args);
    }
    va_end(args);
    return ret;
}

// Mock read_until function
// Reads from stdin until a delimiter or max_len or EOF
int read_until(int fd, char *buffer, size_t max_len, char delimiter) {
    if (buffer == NULL || max_len == 0) return -1;

    size_t bytes_read = 0;
    char c;

    // Assuming fd 0 is stdin for simplicity in this mock
    if (fd != 0) {
        fprintf(stderr, "read_until mock only supports fd 0 (stdin).\n");
        return -1;
    }

    while (bytes_read < max_len - 1 && read(fd, &c, 1) == 1) {
        if (c == delimiter) {
            break;
        }
        buffer[bytes_read++] = c;
    }
    buffer[bytes_read] = '\0';

    // Consume remaining characters on the line if delimiter was found (and not newline itself)
    // This is common in CTF challenges for line-based input.
    if (c != '\n' && delimiter == '\n') { // If we stopped due to max_len, consume until newline
        while (read(fd, &c, 1) == 1 && c != '\n');
    }

    return bytes_read;
}

// Mock bloomy filter functions
bloomy_filter_t *bloomy_new(int size, void *h1, void *h2, void *h3) {
    bloomy_filter_t *bf = (bloomy_filter_t *)malloc(sizeof(bloomy_filter_t));
    if (bf) {
        bf->size = size;
        bf->hash_func1 = h1;
        bf->hash_func2 = h2;
        bf->hash_func3 = h3;
        // In a real bloom filter, initialize bit array here
    }
    return bf;
}

void bloomy_add(bloomy_filter_t *bf, char *word) {
    if (bf && word) {
        // In a real bloom filter, hash the word and set bits
        // For mock, do nothing or print a debug message
        // printf("Mock: Added '%s' to bloom filter.\n", word);
    }
}

int bloomy_check(bloomy_filter_t *bf, char *word) {
    if (bf && word) {
        // In a real bloom filter, hash the word and check bits
        // For mock, let's simulate some behavior:
        // Assume words in depressing_words are in the bloomy filter
        for (unsigned int i = 0; i < DEPRESSING_WORDS_COUNT; ++i) {
            if (strcmp(depressing_words[i], word) == 0) {
                return 1; // "Potentially" in bloomy filter (positive match)
            }
        }
        // Otherwise, assume not in bloomy filter
        return 0; // Not in bloomy filter (negative match)
    }
    return -1; // Error
}

// EVP_PKEY_CTX is from OpenSSL, but it's unused in the provided init function.
// Define a dummy struct to allow compilation.
typedef struct dummy_EVP_PKEY_CTX {} EVP_PKEY_CTX;

// --- Fixed Functions ---

// Function: submit_lyrics
// Original return type undefined4, assuming int (0 for failure, 1 for success)
int submit_lyrics(char *lyrics_buffer, size_t buffer_size) {
    char line_buffer[128];
    char *current_lyrics_ptr = lyrics_buffer;
    size_t remaining_buffer_size = buffer_size;
    int first_word_on_line; // Flag to track if it's the first word on the current line

    memset(lyrics_buffer, 0, buffer_size); // Clear the buffer

    do {
        first_word_on_line = 1; // Reset for each new line
        // Delimiter 10 is ASCII for newline ('\n')
        int read_len = read_until(0, line_buffer, sizeof(line_buffer), '\n');
        if (read_len < 0) {
            if (remaining_buffer_size > 0) {
                *current_lyrics_ptr = '\0'; // Ensure buffer is null-terminated even on error
            }
            return 0; // Read error
        }

        if (strcmp(line_buffer, "EOF") == 0) {
            if (remaining_buffer_size > 0) {
                *current_lyrics_ptr = '\0'; // Null-terminate the string
            }
            return 1; // Success
        }

        char *token_context = line_buffer;
        char *token;

        // Loop for words in a line
        while ((token = strsep(&token_context, " ")) != NULL) {
            if (*token == '\0') { // Handle multiple spaces resulting in empty tokens
                continue;
            }

            size_t token_len = strlen(token);
            size_t content_to_append_len = token_len; // Length of the word itself
            if (!first_word_on_line) {
                content_to_append_len += 1; // Account for the leading space
            }

            // Check if there's enough space for the content + null terminator
            if (remaining_buffer_size < content_to_append_len + 1) {
                if (remaining_buffer_size > 0) {
                    *current_lyrics_ptr = '\0'; // Ensure termination
                }
                return 1; // Buffer full, return what we have
            }

            int bloomy_check_result = bloomy_check(bloomy, token);

            if (bloomy_check_result == 1) { // Word is potentially in bloomy filter
                for (unsigned int i = 0; i < DEPRESSING_WORDS_COUNT; ++i) {
                    if (strcmp(depressing_words[i], token) == 0) {
                        fdprintf(1, "depressing word detected.\n");
                        if (remaining_buffer_size > 0) {
                            *current_lyrics_ptr = '\0'; // Ensure buffer is null-terminated even on error
                        }
                        return 0; // Error: detected depressing word
                    }
                }
                // If it passed the depressing words check, it's a valid word to append
            } else if (bloomy_check_result < 0) {
                fdprintf(1, "bloomy_check error.\n");
                if (remaining_buffer_size > 0) {
                    *current_lyrics_ptr = '\0'; // Ensure buffer is null-terminated even on error
                }
                return 0; // bloomy_check error
            }
            // If bloomy_check_result is 0, the word is not in the bloomy filter, so it's safe to append.

            // Append word
            if (!first_word_on_line) {
                strcat(current_lyrics_ptr, " ");
                current_lyrics_ptr++;
                remaining_buffer_size--;
            }
            strcat(current_lyrics_ptr, token);
            current_lyrics_ptr += token_len;
            remaining_buffer_size -= token_len;

            first_word_on_line = 0; // Not the first word anymore
        }

        // After processing all words on a line, add a newline if space allows
        // Need space for '\n' and '\0' (if this is the last character).
        if (remaining_buffer_size < 2) { // Not enough space for "\n\0"
            if (remaining_buffer_size > 0) {
                *current_lyrics_ptr = '\0'; // Ensure termination
            }
            return 1; // Buffer full
        }
        strcat(current_lyrics_ptr, "\n");
        current_lyrics_ptr++;
        remaining_buffer_size--;

    } while (1); // Loop indefinitely until an explicit return
}

// Function: print_menu
void print_menu(void) {
    fdprintf(1, "1. Submit lyrics\n");
    fdprintf(1, "2. View lyrics\n");
    fdprintf(1, "-3. Quit\n");
}

// Function: init
// Original return type undefined4, assuming int. ctx is unused.
int init(EVP_PKEY_CTX *ctx_unused) {
    bloomy = bloomy_new(0x80, moomoo, yooyoo, geegee);
    if (bloomy == NULL) {
        exit(-1); // WARNING: Subroutine does not return
    }
    for (unsigned int i = 0; i < DEPRESSING_WORDS_COUNT; ++i) {
        bloomy_add(bloomy, depressing_words[i]);
    }
    return DEPRESSING_WORDS_COUNT; // Original returns local_10 which is the loop counter
}

// Function: main
// Original return type undefined4, assuming int
int main(void) {
    char lyrics_buffer[2048];
    char input_buffer[8]; // For menu choice
    char lyrics_submitted = 0; // Flag: 0 = no lyrics, 1 = lyrics submitted

    fdprintf(1, "~=~=~=~= Bloomy Sunday =~=~=~=~\n");
    init(NULL); // Pass NULL as ctx is unused

    do {
        print_menu();
        fdprintf(1, "Enter your choice: "); // Corresponds to DAT_00014185

        // Read choice (delimiter 10 is newline)
        int read_len = read_until(0, input_buffer, sizeof(input_buffer), '\n');
        if (read_len < 0) {
            fdprintf(1, "fail\n");
            return 1; // Return non-zero for error
        }

        long choice = strtol(input_buffer, NULL, 10); // char **0x0 is NULL

        if (choice == 2) { // View lyrics
            if (lyrics_submitted == 0) {
                fdprintf(1, "not good.\n");
            } else {
                fdprintf(1, "Your lyrics:\n%s\n", lyrics_buffer); // Corresponds to DAT_00014193
            }
        } else if (choice == -3) { // Quit
            fdprintf(1, "Don't let the sadness grow.\n\n");
            exit(0);
        } else if (choice == 1) { // Submit lyrics
            lyrics_submitted = submit_lyrics(lyrics_buffer, sizeof(lyrics_buffer));
            // No goto needed, execution continues to the end of the loop iteration
        } else { // Invalid choice
            fdprintf(1, "Nope!\n");
        }
        // This part replaces the goto and the final fdprintf.
        // The original `fdprintf(1,&DAT_00014134,puVar3);` might have been a generic prompt.
        // Simplified to a clear prompt.
        fdprintf(1, "Press Enter to continue...\n");
    } while (1); // Loop indefinitely until exit(0)
    
    return 0; // Should not be reached in normal operation due to exit(0)
}