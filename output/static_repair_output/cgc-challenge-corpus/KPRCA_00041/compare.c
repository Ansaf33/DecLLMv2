#include <stdio.h>   // For printf, fprintf, NULL
#include <stdlib.h>  // For malloc, free, strdup
#include <string.h>  // For strlen, strsep
#include <ctype.h>   // For isspace

// Define a structure to hold line comparison data
typedef struct LineCmp {
    char *original_line;
    char *no_ws_line;
    int hash1; // Hash for original_line
    int hash2; // Another hash for original_line
    int hash3; // Hash for no_ws_line
    int hash4; // Another hash for no_ws_line
} LineCmp;

// Define a node for the linked list
typedef struct LineCmpNode {
    LineCmp *data;
    struct LineCmpNode *next;
} LineCmpNode;

// Function: get_lhash
// Computes two simple hashes for a given string and stores them in hash_out1 and hash_out2.
void get_lhash(int *hash_out1, int *hash_out2, const char *str) {
    if (!str) {
        *hash_out1 = 0;
        *hash_out2 = 0;
        return;
    }

    // DJB2 hash
    unsigned long hash_djb2 = 5381;
    int c;
    for (size_t i = 0; (c = str[i]); i++) {
        hash_djb2 = ((hash_djb2 << 5) + hash_djb2) + c; // hash * 33 + c
    }
    *hash_out1 = (int)hash_djb2;

    // SDBM hash
    unsigned long hash_sdbm = 0;
    for (size_t i = 0; (c = str[i]); i++) {
        hash_sdbm = c + (hash_sdbm << 6) + (hash_sdbm << 16) - hash_sdbm;
    }
    *hash_out2 = (int)hash_sdbm;
}

// Function: get_no_ws_line
// Creates a new string with all whitespace characters removed.
// Returns a dynamically allocated string, or NULL on error or if input is NULL.
char *get_no_ws_line(const char *input_line) {
    if (input_line == NULL) {
        return NULL;
    }

    size_t len = strlen(input_line);
    char *result_line = (char *)malloc(len + 1); // Allocate for worst case (no whitespace) + null terminator
    if (result_line == NULL) {
        return NULL; // Malloc failed
    }

    char *dest_ptr = result_line;
    for (size_t i = 0; i < len; ++i) {
        if (!isspace((unsigned char)input_line[i])) {
            *dest_ptr = input_line[i];
            dest_ptr++;
        }
    }
    *dest_ptr = '\0'; // Null-terminate the new string

    return result_line;
}

// Function: count_words
// Counts the number of words in a string (sequences of non-whitespace characters).
// Returns the word count, or 0 if input is NULL.
int count_words(const char *input_line) {
    if (input_line == NULL) {
        return 0;
    }

    int word_count = 0;
    int in_word = 0; // Flag to track if we are currently inside a word

    for (size_t i = 0; input_line[i] != '\0'; ++i) {
        if (isspace((unsigned char)input_line[i])) {
            in_word = 0; // Transition to whitespace, not in a word
        } else {
            if (in_word == 0) {
                word_count++; // Transition from whitespace to non-whitespace, new word
                in_word = 1;
            }
        }
    }
    return word_count;
}

// Function: new_linecmp
// Creates and populates a LineCmp struct.
// Returns a pointer to the LineCmp struct, or NULL on error.
LineCmp *new_linecmp(const char *line) {
    if (line == NULL) {
        return NULL;
    }

    LineCmp *cmp = (LineCmp *)malloc(sizeof(LineCmp));
    if (cmp == NULL) {
        return NULL;
    }

    // Duplicate the original line
    cmp->original_line = strdup(line);
    if (cmp->original_line == NULL) {
        free(cmp);
        return NULL;
    }

    // Get line without whitespace
    cmp->no_ws_line = get_no_ws_line(line);
    if (cmp->no_ws_line == NULL) {
        free(cmp->original_line);
        free(cmp);
        return NULL;
    }

    // Compute hashes for original_line
    get_lhash(&cmp->hash1, &cmp->hash2, cmp->original_line);

    // Compute hashes for no_ws_line
    get_lhash(&cmp->hash3, &cmp->hash4, cmp->no_ws_line);

    return cmp;
}

// Function: free_linecmp_list (forward declaration for use in pre_process)
void free_linecmp_list(LineCmpNode **head);

// Function: pre_process
// Parses an input string, creating a linked list of LineCmpNodes.
// `input_buffer`: The string to process.
// `total_words`: Pointer to an integer to accumulate total word count.
// `total_lines`: Pointer to an integer to accumulate total line count.
// Returns the head of the linked list, or NULL on error or if no valid lines were found.
LineCmpNode *pre_process(const char *input_buffer, int *total_words, int *total_lines) {
    if (input_buffer == NULL || total_words == NULL || total_lines == NULL) {
        return NULL;
    }

    // Make a mutable copy of the input string for strsep
    char *input_buffer_copy = strdup(input_buffer);
    if (input_buffer_copy == NULL) {
        return NULL;
    }

    char *current_line_ptr = input_buffer_copy; // Pointer that strsep will modify
    char *token_line;
    LineCmpNode *head = NULL;
    LineCmpNode *tail = NULL; // Keep track of the tail to append easily

    *total_words = 0;
    *total_lines = 0;

    while ((token_line = strsep(&current_line_ptr, "\n")) != NULL) {
        // Check if the line contains any non-whitespace characters
        int has_content = 0;
        for (size_t k = 0; token_line[k] != '\0'; ++k) {
            if (!isspace((unsigned char)token_line[k])) {
                has_content = 1;
                break;
            }
        }
        if (!has_content) {
            continue; // Skip this line if it's empty or only whitespace
        }

        (*total_lines)++; // Increment line count for non-empty lines

        LineCmp *cmp_data = new_linecmp(token_line);
        if (cmp_data == NULL) {
            // Error creating LineCmp data, free what's been allocated and return NULL
            free_linecmp_list(&head);
            free(input_buffer_copy);
            return NULL;
        }

        LineCmpNode *new_node = (LineCmpNode *)malloc(sizeof(LineCmpNode));
        if (new_node == NULL) {
            // Malloc failed for node, free LineCmp data and list, then return NULL
            free(cmp_data->original_line);
            free(cmp_data->no_ws_line);
            free(cmp_data);
            free_linecmp_list(&head);
            free(input_buffer_copy);
            return NULL;
        }

        new_node->data = cmp_data;
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }

        // Update total word count
        *total_words += count_words(token_line);
    }

    free(input_buffer_copy); // Free the duplicated input buffer
    return head;
}

// Function: free_linecmp_list
// Frees all memory associated with a LineCmpNode linked list.
void free_linecmp_list(LineCmpNode **head) {
    if (head == NULL || *head == NULL) {
        return;
    }

    LineCmpNode *current = *head;
    LineCmpNode *next_node;

    while (current != NULL) {
        next_node = current->next; // Save next node before freeing current

        if (current->data != NULL) {
            // Free the strings within the LineCmp struct
            free(current->data->original_line); // strdup was used, so free
            free(current->data->no_ws_line);    // malloc was used, so free
            // Free the LineCmp struct itself
            free(current->data);
        }
        // Free the LineCmpNode
        free(current);

        current = next_node;
    }

    *head = NULL; // Set the head pointer to NULL after freeing the list
}

int main() {
    const char *input = "  Hello World  \nThis is a test.\n\nAnother line.\n";
    LineCmpNode *list_head = NULL;
    int total_words = 0;
    int total_lines = 0;

    printf("Processing input:\n---\n%s---\n", input);

    list_head = pre_process(input, &total_words, &total_lines);

    if (list_head == NULL) {
        fprintf(stderr, "Error during pre-processing or no valid lines found.\n");
        return 1;
    }

    printf("\nPre-processing complete.\n");
    printf("Total words: %d\n", total_words);
    printf("Total lines: %d\n", total_lines);
    printf("\n--- LineCmp List ---\n");

    LineCmpNode *current = list_head;
    int line_num = 1;
    while (current != NULL) {
        if (current->data != NULL) {
            printf("Line %d:\n", line_num++);
            printf("  Original: \"%s\"\n", current->data->original_line);
            printf("  No WS:    \"%s\"\n", current->data->no_ws_line);
            printf("  Hashes (Orig): %d, %d\n", current->data->hash1, current->data->hash2);
            printf("  Hashes (No WS): %d, %d\n", current->data->hash3, current->data->hash4);
        }
        current = current->next;
    }

    free_linecmp_list(&list_head);
    printf("\nList freed.\n");

    // Test with an empty string or string with only whitespace
    const char *empty_input = "   \t\n\n";
    total_words = 0;
    total_lines = 0;
    printf("\nProcessing empty/whitespace input:\n---\n%s---\n", empty_input);
    list_head = pre_process(empty_input, &total_words, &total_lines);
    if (list_head == NULL) {
        printf("Correctly handled empty/whitespace input: No list created.\n");
        printf("Total words: %d, Total lines: %d\n", total_words, total_lines);
    } else {
        fprintf(stderr, "Error: List created for empty/whitespace input.\n");
        free_linecmp_list(&list_head); // Clean up if unexpected
    }

    return 0;
}