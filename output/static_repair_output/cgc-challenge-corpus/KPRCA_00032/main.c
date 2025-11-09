#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For 'bool' type

// --- Placeholder for external functions ---
// These functions are assumed to be defined elsewhere and linked.
// Their signatures are inferred from usage in the provided snippet.

// Reads an integer from input.
// Returns -1 on error or EOF.
int read_int() {
    int val;
    if (scanf("%d", &val) == 1) {
        return val;
    }
    // Clear input buffer for subsequent reads if an error occurred
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    return -1; // Indicate error
}

// Reads a string from input. Returns dynamically allocated string. Caller must free.
// Returns NULL on error or EOF.
char* read_string() {
    char buffer[256]; // A reasonable buffer size for input strings
    if (scanf("%255s", buffer) == 1) { // Read up to 255 chars to prevent overflow
        return strdup(buffer); // strdup allocates memory, caller must free
    }
    // Clear input buffer for subsequent reads if an error occurred
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    return NULL; // Indicate error
}

// Writes an integer to output.
void write_int(int val) {
    printf("%d\n", val);
}

// Writes a string to output.
void write_string(const char* s) {
    printf("%s\n", s);
}

// Initializes a feature table. `ftab_buffer` is a fixed-size buffer.
void ftab_init1(void* ftab_buffer) {
    // Example: memset(ftab_buffer, 0, 104);
    (void)ftab_buffer; // Suppress unused parameter warning
}

// Adds a word to the feature table.
void ftab_add(void* ftab_buffer, const char* word) {
    (void)ftab_buffer; // Suppress unused parameter warning
    (void)word;        // Suppress unused parameter warning
    // Placeholder implementation
}

// Initializes an atree. Returns a tree handle (int). Takes feature table buffer.
// Returns 0 on failure.
int atree_init(void* ftab_buffer) {
    (void)ftab_buffer; // Suppress unused parameter warning
    // Placeholder: return a non-zero handle for success
    return 1; // Example: success
}

// Adds a word to the atree. Takes ownership of the string on success.
// Returns 1 on success, 0 on failure.
int atree_add(int tree_handle, char* word) {
    (void)tree_handle; // Suppress unused parameter warning
    (void)word;        // Suppress unused parameter warning
    // Placeholder: Assume success for now.
    // In a real implementation, if it fails, 'word' should be freed by the caller.
    // If it succeeds, 'word' is managed by the tree.
    return 1; // Example: success
}

// Removes a word from the atree. Internally frees the string from the tree.
// Returns 1 on success (word found and removed), 0 on failure (word not found).
int atree_remove(int tree_handle, const char* word) {
    (void)tree_handle; // Suppress unused parameter warning
    (void)word;        // Suppress unused parameter warning
    // Placeholder: Assume success for now.
    // In a real implementation, if a word is removed, the memory for that word
    // (which was taken ownership of by atree_add) should be freed by atree_remove.
    return 1; // Example: success
}

// Queries the atree for exact matches or related words.
// Returns a dynamically allocated, NULL-terminated array of char* pointers.
// The caller must free the returned array itself, but not the strings within it.
// Returns NULL on failure or no results.
char** atree_query(int tree_handle, const char* query_word) {
    (void)tree_handle; // Suppress unused parameter warning
    (void)query_word;  // Suppress unused parameter warning
    // Placeholder: Return a dummy array of strings
    char** results = (char**)malloc(2 * sizeof(char*));
    if (results) {
        results[0] = strdup("example_result");
        results[1] = NULL;
    }
    return results; // Example: returns one result
}

// Queries the atree for subset matches. Similar return behavior to atree_query.
char** atree_query_subset(int tree_handle, const char* query_word) {
    (void)tree_handle; // Suppress unused parameter warning
    (void)query_word;  // Suppress unused parameter warning
    // Placeholder: Return a dummy array of strings
    char** results = (char**)malloc(2 * sizeof(char*));
    if (results) {
        results[0] = strdup("subset_example");
        results[1] = NULL;
    }
    return results; // Example: returns one result
}

// Returns the number of words in the atree.
unsigned int atree_count(int tree_handle) {
    (void)tree_handle; // Suppress unused parameter warning
    // Placeholder:
    return 15; // Example: return 15 words
}

// Returns a random word (char*) from the atree. The string is owned by the tree.
// Returns NULL on failure or if tree is empty.
char* atree_random(int tree_handle) {
    (void)tree_handle; // Suppress unused parameter warning
    // Placeholder:
    return strdup("random_word"); // Example: returns a dummy word
}
// --- End of placeholder functions ---


// Function: read_initial_words
int read_initial_words(int *tree_handle_ptr) {
    unsigned int count = read_int();
    if (count >= 0x3fffffff) { // Check for excessive size
        return 0; // Indicate failure
    }

    // Allocate array of char* pointers to hold the words temporarily
    char **words_to_add = (char **)malloc(count * sizeof(char *));
    if (words_to_add == NULL) {
        return 0; // Indicate failure due to memory allocation
    }

    // Read all words and validate them
    for (unsigned int i = 0; i < count; ++i) {
        words_to_add[i] = read_string();
        // If a word is NULL or an empty string, consider it a failure.
        if (words_to_add[i] == NULL || strlen(words_to_add[i]) == 0) {
            // Cleanup: free all strings read so far and the array itself
            for (unsigned int j = 0; j <= i; ++j) {
                free(words_to_add[j]);
            }
            free(words_to_add);
            return 0; // Indicate failure
        }
    }

    char ftab_buffer[104]; // Buffer for feature table initialization
    ftab_init1(ftab_buffer);

    for (unsigned int i = 0; i < count; ++i) {
        ftab_add(ftab_buffer, words_to_add[i]);
    }

    *tree_handle_ptr = atree_init(ftab_buffer); // Initialize the tree and store its handle
    if (*tree_handle_ptr == 0) { // If tree initialization fails
        // Cleanup: free all strings and the array if tree init fails
        for (unsigned int i = 0; i < count; ++i) {
            free(words_to_add[i]);
        }
        free(words_to_add);
        return 0; // Indicate failure
    }

    // Add words to the tree.
    // Based on cmd_add_word, atree_add takes ownership of the string on success.
    for (unsigned int i = 0; i < count; ++i) {
        atree_add(*tree_handle_ptr, words_to_add[i]); // Ownership of words_to_add[i] transferred to the tree
    }
    free(words_to_add); // Free the array of pointers, not the strings themselves

    return 1; // Indicate success
}

// Function: cmd_add_word
void cmd_add_word(int *tree_handle_ptr) {
    char *word = read_string(); // Read the word to add
    if (word == NULL) {
        write_int(1); // Indicate error (e.g., input failure)
    } else {
        if (atree_add(*tree_handle_ptr, word) == 0) { // If adding to tree fails
            write_int(1); // Indicate error
            free(word);   // Caller frees word as atree didn't take ownership
        } else {
            write_int(0); // Indicate success
            // Ownership of 'word' is now with the tree
        }
    }
}

// Function: cmd_remove_word
void cmd_remove_word(int *tree_handle_ptr) {
    char *word = read_string(); // Read the word to remove
    if (word == NULL) {
        write_int(1); // Indicate error
    } else {
        if (atree_remove(*tree_handle_ptr, word) == 0) { // If removing from tree fails (word not found)
            write_int(1); // Indicate error
        } else {
            write_int(0); // Indicate success
            // The string that was *in the tree* (matching 'word') is assumed to be freed by atree_remove internally.
        }
        free(word); // Free the input string read from read_string()
    }
}

// Function: sort_results (Insertion Sort)
// results is an array of char* (string pointers), count is the number of elements.
void sort_results(char **results, int count) {
    for (int i = 1; i < count; ++i) {
        char *current_word = results[i];
        int j = i;
        // Shift elements greater than current_word to the right
        while (j > 0 && strcmp(results[j - 1], current_word) > 0) {
            results[j] = results[j - 1];
            j = j - 1;
        }
        results[j] = current_word; // Insert current_word at its sorted position
    }
}

// Function: cmd_query
void cmd_query(int *tree_handle_ptr, int query_type) {
    char *query_word = read_string(); // Read the query word
    if (query_word == NULL) {
        write_int(1); // Indicate error
        return;
    }

    char **results_array;
    if (query_type == 0) { // Exact query
        results_array = atree_query(*tree_handle_ptr, query_word);
    } else { // Subset query
        results_array = atree_query_subset(*tree_handle_ptr, query_word);
    }
    free(query_word); // Free the input query word

    if (results_array == NULL) {
        write_int(1); // Query failed or no results found
        return;
    }

    int results_count = 0;
    // Count the number of results (array is NULL-terminated)
    for (char **current_result_ptr = results_array; *current_result_ptr != NULL; ++current_result_ptr) {
        results_count++;
    }

    write_int(0); // Indicate success
    write_int(results_count);
    sort_results(results_array, results_count); // Sort the results

    // Print the sorted results
    for (char **current_result_ptr = results_array; *current_result_ptr != NULL; ++current_result_ptr) {
        write_string(*current_result_ptr);
    }
    free(results_array); // Free the array of pointers returned by atree_query, not the strings themselves
}

// Function: cmd_play_game
void cmd_play_game(int *tree_handle_ptr) {
    unsigned int tree_size = atree_count(*tree_handle_ptr);
    if (tree_size < 10) {
        write_int(1); // Not enough words in the tree to play the game
        return;
    }

    char *random_words[10]; // Stack-allocated array to store 10 unique random word pointers
    // Select 10 unique random words from the tree
    for (int i = 0; i < 10; ++i) {
        char *word;
        bool is_duplicate;
        do {
            word = atree_random(*tree_handle_ptr); // Get a random word (char* pointer from the tree)
            is_duplicate = false;
            // Check for duplicates among already selected words
            for (int j = 0; j < i; ++j) {
                if (random_words[j] == word) { // Compare pointers for uniqueness
                    is_duplicate = true;
                    break;
                }
            }
        } while (word == NULL || is_duplicate); // Retry if NULL or duplicate word is returned

        random_words[i] = word; // Store the unique random word pointer
    }

    // Game loop for each of the 10 selected words
    for (int i = 0; i < 10; ++i) {
        char *current_random_word = random_words[i];
        
        // Query for related words for the current random word
        char **related_words = atree_query(*tree_handle_ptr, current_random_word);
        if (related_words == NULL) {
            write_int(1); // Error querying related words
            return; // Exit game due to query failure
        }

        int related_word_count = 0;
        for (char **ptr = related_words; *ptr != NULL; ++ptr) {
            related_word_count++;
        }

        int guesses_needed = related_word_count / 2;
        if (guesses_needed == 0) { // Ensure at least one guess is needed
            guesses_needed = 1;
        }

        write_string(current_random_word); // Display the word to guess related terms for
        write_int(guesses_needed);          // Display how many guesses are needed

        int correct_guesses = 0;
        while (correct_guesses < guesses_needed) {
            char *guess_input = read_string(); // Read user's guess
            if (guess_input == NULL) {
                // User input error or EOF. Clean up and exit game.
                free(related_words);
                return;
            }

            bool found = false;
            // Iterate through related words to check if the guess is correct
            for (int j = 0; j < related_word_count; ++j) {
                // Check if word hasn't been guessed already (marked NULL) and matches the input
                if (related_words[j] != NULL && strcasecmp(guess_input, related_words[j]) == 0) {
                    correct_guesses++;
                    related_words[j] = NULL; // Mark this related word as guessed
                    write_int(correct_guesses); // Inform user of current correct guess count
                    found = true;
                    break;
                }
            }
            free(guess_input); // Free the input string received from read_string()
        }
        free(related_words); // Free the array of pointers returned by atree_query
    }
    write_int(0); // Game finished successfully
}

// Function: main
int main(void) {
    int tree_handle = 0; // Initialize tree handle to 0

    // Loop until initial words are successfully read and the tree is initialized
    while (read_initial_words(&tree_handle) == 0) {
        write_int(1); // Indicate failure to initialize, prompt retry
    }
    write_int(0); // Indicate successful initialization

    // Main command loop
    while (true) {
        int command = read_int();
        switch (command) {
            case 1:
                cmd_add_word(&tree_handle);
                break;
            case 2:
                cmd_remove_word(&tree_handle);
                break;
            case 3:
                cmd_query(&tree_handle, 0); // Exact query
                break;
            case 4:
                cmd_query(&tree_handle, 1); // Subset query
                break;
            case 5:
                cmd_play_game(&tree_handle);
                break;
            case 0xbad1abe1: // A specific error/diagnostic command
                write_int(1);
                break;
            default: // Exit on any other command
                // Assuming tree destruction is handled by a specific command or system exit.
                // If an atree_destroy function existed, it would be called here:
                // atree_destroy(tree_handle);
                write_int(0); // Indicate graceful exit
                return 0;
        }
    }
}