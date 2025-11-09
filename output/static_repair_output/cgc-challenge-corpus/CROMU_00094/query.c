#include <stdlib.h> // For calloc, free, NULL
#include <string.h> // For strlen, strcpy

// Define the Query struct based on the original logic.
// This structure is designed to hold a character 'type',
// a 'length' (short integer), and a dynamically allocated
// string 'data'.
// Its size will vary depending on the architecture (32-bit vs. 64-bit)
// due to the size of the char* pointer and alignment.
typedef struct Query {
    char type;
    short length;
    char *data;
} Query;

// Function: DestroyQuery
// Frees the memory allocated for a Query object and its internal data string.
// Returns 0 upon completion.
int DestroyQuery(void *param_1) {
    if (param_1 != NULL) {
        Query *query = (Query *)param_1;
        // Free the data string if it was allocated
        if (query->data != NULL) {
            free(query->data);
        }
        // Free the Query object itself
        free(query);
    }
    return 0;
}

// Function: ParseQuery
// Parses an input string (param_1) to create and populate a new Query object.
// The first character of param_1 becomes the 'type'.
// The rest of param_1 (from the second character onwards) becomes the 'data' string.
// Returns a pointer to the newly created Query object, or NULL on error or if param_1 is NULL.
Query *ParseQuery(const char *param_1) {
    if (param_1 == NULL) {
        return NULL;
    }

    // Allocate memory for one Query object.
    // Using sizeof(Query) ensures portability across different architectures.
    Query *query = (Query *)calloc(1, sizeof(Query));
    if (query == NULL) {
        return NULL; // Allocation failed
    }

    // Set the 'type' of the query from the first character of the input string.
    query->type = param_1[0];

    // Calculate the length of the data string (from the second character of param_1).
    // The result of strlen (size_t) is cast to short, which might truncate for very long strings.
    query->length = (short)strlen(param_1 + 1);

    // Allocate memory for the 'data' string, including space for the null terminator.
    // sizeof(char) is typically 1, so calloc(count, 1) is equivalent to calloc(count, sizeof(char)).
    query->data = (char *)calloc(query->length + 1, sizeof(char));
    if (query->data == NULL) {
        free(query); // Clean up the partially allocated Query object
        return NULL; // Allocation failed
    }

    // Copy the data string from param_1 (starting from the second character).
    strcpy(query->data, param_1 + 1);

    return query;
}