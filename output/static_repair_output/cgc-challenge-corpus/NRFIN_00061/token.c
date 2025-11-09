#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For fixed-width integers if needed, but unsigned int is fine for uint

// --- Global Variables ---
// DAT_4347c000: An array of bytes, likely representing some raw data or configuration.
// Initialized with example values.
unsigned char DAT_4347c000[200] = {
    0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f, 0x7a, 0x8b, 0x9c, 0xad,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa
};
unsigned int flag_index = 0; // Index for DAT_4347c000

// TokenEntry structure for the linked list `tokenStore`.
// Assumed to be compatible with original decompiler's offsets (e.g., char* is 4 bytes on 32-bit).
typedef struct TokenEntry {
    int token_id;
    char *name;
    int expiration;
    struct TokenEntry *next;
} TokenEntry;

TokenEntry *tokenStore = NULL; // Head of the linked list of tokens

// --- Stub Functions for External Dependencies ---

// _terminate: Exits the program due to a fatal error.
void _terminate() {
    fprintf(stderr, "Program terminated due to a fatal error.\n");
    exit(1);
}

// sendErrorResponse: Sends an error message (e.g., to client or log).
void sendErrorResponse(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

// transmit_all: Placeholder for a function that sends data (e.g., over a network).
// Assumed to operate on implicitly available data or context in the original code.
// Returns 0 on success, non-zero on failure.
int transmit_all() {
    // In a real application, this would handle network transmission.
    // For this compilation, it's a simple stub.
    return 0; // Simulate success
}

// initializeAttributes: Sets up a parsing context for a raw token string/data.
// `raw_token_data_handle` could be a file descriptor, a pointer to a string, etc.
// For the stub, it sets mock values for subsequent attribute retrieval.
static char _mock_token_name_attr[64];
static int _mock_token_id_attr;
static int _mock_token_expiration_attr;

void initializeAttributes(int raw_token_data_handle) {
    // Mock implementation: sets different values based on the handle.
    if (raw_token_data_handle == 1) { // Example: mock data for handle 1
        strcpy(_mock_token_name_attr, "user_alpha");
        _mock_token_id_attr = 10001;
        _mock_token_expiration_attr = 20000000; // A future expiration
    } else if (raw_token_data_handle == 2) { // Example: mock data for handle 2
        strcpy(_mock_token_name_attr, "user_beta");
        _mock_token_id_attr = 10002;
        _mock_token_expiration_attr = 5000000;  // An expired token
    } else { // Default mock data
        strcpy(_mock_token_name_attr, "default_user");
        _mock_token_id_attr = 12345;
        _mock_token_expiration_attr = 98765;
    }
}

// getStringAttribute: Retrieves a string attribute from the current parsing context.
// `buffer` will point to newly allocated memory containing the string.
void getStringAttribute(char **buffer, const char *attribute_name) {
    // Mock implementation: allocates memory and copies a mock string.
    size_t len = strlen(_mock_token_name_attr);
    *buffer = (char *)malloc(len + 1);
    if (*buffer == NULL) {
        fprintf(stderr, "Memory allocation failed for string attribute.\n");
        _terminate();
    }
    strcpy(*buffer, _mock_token_name_attr);
}

// getIntegerAttribute: Retrieves an integer attribute from the current parsing context.
void getIntegerAttribute(int *buffer, const char *attribute_name) {
    // Mock implementation: returns mock integer values based on attribute_name.
    if (strcmp(attribute_name, "Token") == 0) {
        *buffer = _mock_token_id_attr;
    } else if (strcmp(attribute_name, "Expiration") == 0) {
        *buffer = _mock_token_expiration_attr;
    } else {
        *buffer = 0; // Default or error value
    }
}

// --- Original Functions (Fixed and Reduced) ---

// Function: getToken
unsigned int getToken(void) {
    unsigned int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    unsigned int result = 1;

    for (int i = 0; i < 10; i++) {
        if ((unsigned int)DAT_4347c000[flag_index + i] % primes[i] == 0) {
            result *= primes[i];
        }
    }
    flag_index += 10;

    while (result < 10000000) {
        result *= 10;
    }
    while (result >= 100000000) { // Condition 99999999 < local_c means local_c >= 100000000
        result /= 10;
    }
    return result;
}

// Function: isTokenCommand
int isTokenCommand(const char *param_1) {
    if (strncmp(param_1, "newTokens", strlen("newTokens")) == 0) {
        return 1;
    }
    if (strncmp(param_1, "refreshToken", strlen("refreshToken")) == 0) {
        return 1;
    }
    if (strncmp(param_1, "revokeT", strlen("revokeT")) == 0) {
        return 1;
    }
    return 0;
}

// Function: calculateTokenSize
// Calculates the size needed for the token string based on the format "Token=%u;Expiration=%u;%s=%s?".
// It takes a TokenEntry pointer to access its fields.
int calculateTokenSize(const TokenEntry *param_1) {
    char temp_buffer[20]; // Buffer to hold string representation of integers
    size_t len_expiration_str;
    size_t len_token_id_str;
    size_t len_name_str = strlen(param_1->name);

    // Get length of expiration as string
    memset(temp_buffer, 0, sizeof(temp_buffer));
    sprintf(temp_buffer, "%u", (unsigned int)param_1->expiration);
    len_expiration_str = strlen(temp_buffer);

    // Get length of token_id as string
    memset(temp_buffer, 0, sizeof(temp_buffer));
    sprintf(temp_buffer, "%u", (unsigned int)param_1->token_id);
    len_token_id_str = strlen(temp_buffer);

    // Calculate total length based on format "Token=%u;Expiration=%u;%s=%s?"
    // strlen("Token=") + len_expiration_str +
    // strlen(";Expiration=") + len_token_id_str +
    // strlen(";") + len_name_str +
    // strlen("=") + len_name_str +
    // strlen("?") + 1 (for null terminator)
    return strlen("Token=") + len_expiration_str +
           strlen(";Expiration=") + len_token_id_str +
           strlen(";") + len_name_str +
           strlen("=") + len_name_str +
           strlen("?") + 1; // +1 for the null terminator
}

// Function: sendToken
void sendToken(int param_1, const TokenEntry *param_2) {
    void *token_str_buffer;
    size_t token_str_len;
    void *token_data_buffer;
    unsigned char *data_ptr;
    
    token_str_len = calculateTokenSize(param_2);

    token_str_buffer = malloc(token_str_len);
    if (token_str_buffer == NULL) {
        _terminate();
    }
    memset(token_str_buffer, 0, token_str_len);

    token_data_buffer = malloc(0x40); // Hardcoded size 0x40 from original
    if (token_data_buffer == NULL) {
        free(token_str_buffer); // Clean up before terminating
        _terminate();
    }
    memset(token_data_buffer, 0, 0x40);

    data_ptr = DAT_4347c000;
    for (unsigned int i = 0; i < 10; i++) {
        sprintf((char *)token_data_buffer + i * 4, "%d", (unsigned int)*data_ptr);
        data_ptr++;
    }

    // Construct the final token string
    sprintf((char *)token_str_buffer, "Token=%u;Expiration=%u;%s=%s?",
            (unsigned int)param_2->expiration, // First %u
            (unsigned int)param_2->token_id,   // Second %u
            param_2->name,                     // First %s
            param_2->name);                    // Second %s

    if (transmit_all() != 0) { // Assumes transmit_all uses global context or param_1
        free(token_str_buffer);
        free(token_data_buffer);
        _terminate();
    }

    free(token_str_buffer);
    free(token_data_buffer);
    return;
}

// Function: findToken
// Finds a token in the linked list based on its expiration value matching the input token's expiration.
// Returns a pointer to the found TokenEntry, or NULL if not found.
TokenEntry *findToken(const TokenEntry *param_1) {
    TokenEntry *current = tokenStore;

    while (current != NULL) {
        if (current->expiration == param_1->expiration) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: removeToken
// Removes a specific TokenEntry from the linked list `tokenStore`.
// `token_to_remove` must be a pointer to an actual node *within* the `tokenStore` list.
// Returns 1 on success, 0 if the token was not found in the list.
int removeToken(TokenEntry *token_to_remove) {
    if (token_to_remove == NULL) {
        return 0; // Cannot remove a NULL token
    }

    TokenEntry *current = tokenStore;
    TokenEntry *previous = NULL;

    // Search for the token_to_remove by its memory address
    while (current != NULL && current != token_to_remove) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        return 0; // Token not found in the list
    }

    // Token found (current points to it)
    if (previous != NULL) {
        previous->next = current->next;
    } else {
        // Removing the head of the list
        tokenStore = current->next;
    }
    
    free(current->name); // Free the name string if it was malloc'd by getStringAttribute
    free(current);       // Free the TokenEntry struct itself
    return 1;
}

// Function: validateToken
// Validates an incoming token against the store and current time.
// `token_info` is the parsed token from the request.
// `current_time` is a pointer to an unsigned int representing the current time.
// Returns 1 if valid and not expired, 0 otherwise.
int validateToken(const TokenEntry *token_info, unsigned int *current_time) {
    // Basic validation of the incoming token's fields
    if (token_info->token_id == 0 || token_info->name == NULL || token_info->expiration == 0) {
        sendErrorResponse("Invalid Token");
        return 0;
    }

    TokenEntry *found_token = findToken(token_info);
    if (found_token == NULL) {
        sendErrorResponse("Invalid Token");
        return 0;
    }

    // Compare current time with the found token's expiration
    if (*current_time < (unsigned int)found_token->expiration) {
        return 1; // Token is valid and not expired
    } else {
        removeToken(found_token); // Remove the expired token from the store
        sendErrorResponse("Expired Token");
        return 0;
    }
}

// Function: parseToken
// Parses raw token data (identified by `raw_token_data_handle`) into a new TokenEntry structure.
// Returns a pointer to the newly created TokenEntry, or NULL on failure.
TokenEntry *parseToken(int raw_token_data_handle) {
    TokenEntry *new_token = (TokenEntry *)malloc(sizeof(TokenEntry));
    if (new_token == NULL) {
        _terminate();
    }
    memset(new_token, 0, sizeof(TokenEntry));

    initializeAttributes(raw_token_data_handle);

    new_token->name = NULL; // Initialize to NULL before `getStringAttribute` allocates
    getStringAttribute(&new_token->name, "Name"); // Assuming "Name" is the attribute key for the string.
    if (new_token->name == NULL) { // Check if string attribute was successfully retrieved/allocated
        free(new_token);
        return NULL;
    }

    getIntegerAttribute(&new_token->token_id, "Token");
    getIntegerAttribute(&new_token->expiration, "Expiration");

    return new_token;
}

// Function: refreshToken
// Refreshes an existing token's expiration time.
// `param_1` is connection_id, `param_2` is raw_token_data_handle, `param_3` is current_time/counter.
void refreshToken(int param_1, int param_2, unsigned int *param_3) {
    TokenEntry *parsed_token = parseToken(param_2);
    if (parsed_token == NULL) {
        sendErrorResponse("Invalid Token Format");
        return;
    }

    TokenEntry *found_token = findToken(parsed_token);
    if (found_token == NULL) {
        sendErrorResponse("Invalid Token");
    } else {
        // Check if token is still valid before refreshing
        if (*param_3 < (unsigned int)found_token->expiration) {
            // Token is still valid, refresh it
            (*param_3)++; // Increment global counter/current_time
            found_token->expiration = *param_3 + 100; // Set new expiration
            sendToken(param_1, found_token); // Send the updated token
        } else {
            // Token has expired, remove it
            removeToken(found_token);
            sendErrorResponse("Expired Token");
        }
    }

    // Always free the newly parsed token, as it was only used for lookup.
    free(parsed_token->name);
    free(parsed_token);
    return;
}

// Function: requestToken
// Requests a new token, generates its ID and expiration, and adds it to the store.
// `param_1` is connection_id, `param_2` is raw_token_data_handle, `param_3` is current_time/counter.
void requestToken(int param_1, int param_2, int *param_3) {
    TokenEntry *new_token = parseToken(param_2);
    if (new_token == NULL) {
        sendErrorResponse("Invalid Token Format");
        return;
    }

    new_token->expiration = getToken(); // Generate a new token ID/expiration for this field

    (*param_3)++; // Increment global counter/current_time
    new_token->token_id = *param_3 + 100; // Set token_id based on counter

    sendToken(param_1, new_token);

    // Add the new token to the head of the linked list
    new_token->next = tokenStore;
    tokenStore = new_token;
    return;
}

// Function: checkTokenUse
// Checks if a given `command` is permitted based on a string of `token_uses_str`.
// `token_uses_str` is a colon-separated list of allowed uses.
// Returns 1 if permitted, 0 otherwise.
int checkTokenUse(const char *command, char *token_uses_str) {
    if (token_uses_str == NULL) {
        return 0;
    }

    char *current_use_token = strtok(token_uses_str, ":");
    while (current_use_token != NULL) {
        // Check if command matches an allowed use
        if (strncmp(command, current_use_token, strlen(command)) == 0) {
            return 1;
        }
        // Original code's peculiar logic: if the command itself is "revokeT", always permit.
        // This might be a security flaw or intended behavior. Preserving original logic.
        if (strncmp("revokeT", command, strlen("revokeT")) == 0) {
            return 1;
        }
        current_use_token = strtok(NULL, ":");
    }
    return 0;
}

// Function: revokeToken
// Revokes an existing token by removing it from the store.
// `param_1` is connection_id, `param_2` is raw_token_data_handle.
void revokeToken(int param_1, int param_2) {
    TokenEntry *parsed_token = parseToken(param_2);
    if (parsed_token == NULL) {
        sendErrorResponse("Invalid Token Format");
        return;
    }

    TokenEntry *found_token = findToken(parsed_token);
    if (found_token == NULL) {
        sendErrorResponse("Invalid Token");
    } else {
        removeToken(found_token); // Remove the found token from the store
        sendErrorResponse("Token Revoked");
    }

    // Always free the newly parsed token, as it was only used for lookup.
    free(parsed_token->name);
    free(parsed_token);
    return;
}

// --- Main function for demonstration/testing (not part of original snippet) ---
int main() {
    printf("Starting token system simulation.\n");

    unsigned int current_system_time = 1000000; // A mock current time/counter

    // Test requestToken
    printf("\n--- Requesting a new token ---\n");
    requestToken(1, 1, (int*)&current_system_time); // Handle 1 for user_alpha
    printf("TokenStore head: %p\n", (void*)tokenStore);
    if (tokenStore) {
        printf("New Token: ID=%d, Name=%s, Expiration=%d\n", tokenStore->token_id, tokenStore->name, tokenStore->expiration);
    }

    // Test refreshToken
    printf("\n--- Refreshing token ---\n");
    // Simulate current time still less than expiration, so it should refresh
    refreshToken(1, 1, &current_system_time);
    if (tokenStore) {
        printf("Refreshed Token: ID=%d, Name=%s, Expiration=%d\n", tokenStore->token_id, tokenStore->name, tokenStore->expiration);
    }

    // Test validateToken (valid case)
    printf("\n--- Validating token (valid case) ---\n");
    unsigned int validation_time = current_system_time + 50; // Still within expiration
    if (validateToken(tokenStore, &validation_time)) {
        printf("Token is valid.\n");
    } else {
        printf("Token is invalid or expired.\n");
    }

    // Test validateToken (expired case)
    printf("\n--- Validating token (expired case) ---\n");
    // Advance time past the token's expiration
    unsigned int expired_validation_time = (tokenStore != NULL ? tokenStore->expiration + 1 : 0);
    if (tokenStore != NULL && validateToken(tokenStore, &expired_validation_time)) {
        printf("Token is valid.\n");
    } else {
        printf("Token is invalid or expired.\n");
    }
    printf("TokenStore head after expiration check: %p\n", (void*)tokenStore); // Should be NULL if expired token was removed

    // Test revokeToken
    printf("\n--- Requesting another token for revocation test ---\n");
    requestToken(1, 2, (int*)&current_system_time); // Handle 2 for user_beta
    printf("TokenStore head: %p\n", (void*)tokenStore);
    if (tokenStore) {
        printf("New Token for revoke: ID=%d, Name=%s, Expiration=%d\n", tokenStore->token_id, tokenStore->name, tokenStore->expiration);
    }

    printf("\n--- Revoking token ---\n");
    revokeToken(1, 2); // Use handle 2 to identify token for revocation
    printf("TokenStore head after revocation: %p\n", (void*)tokenStore); // Should be NULL

    // Test checkTokenUse
    printf("\n--- Checking token use ---\n");
    char uses_str[] = "newTokens:refreshToken:revokeT";
    printf("Command 'newTokens' allowed: %d\n", checkTokenUse("newTokens", uses_str)); // Should be 1
    char uses_str2[] = "newTokens:refreshToken:revokeT";
    printf("Command 'unknown' allowed: %d\n", checkTokenUse("unknown", uses_str2));     // Should be 0
    char uses_str3[] = "newTokens:refreshToken:revokeT";
    printf("Command 'revokeT' allowed: %d\n", checkTokenUse("revokeT", uses_str3));     // Should be 1 due to special handling

    printf("\nSimulation finished.\n");
    return 0;
}