#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

// Forward declarations for structs
typedef struct SubscriptionNode SubscriptionNode;
typedef struct User User;

// Structure for a subscription node in a linked list
struct SubscriptionNode {
    char *data; // The actual subscription data (e.g., a string)
    void *field1; // Placeholder for unknown field at index 1
    void *field2; // Placeholder for unknown field at index 2
    SubscriptionNode *next; // Pointer to the next subscription node (at index 3)
};

// Structure for a user node in a linked list
struct User {
    char *token;             // At logical index 0
    char *name;              // At logical index 1
    char *password;          // At logical index 2
    SubscriptionNode *subscription_list; // At logical index 3
    void *signing_key;       // At logical index 4 (raw bytes)
    User *next;              // At logical index 5
};

// Global flag to ensure random is seeded only once
static bool random_seeded = false;

// Helper function to convert a nibble (4-bit value) to a hex character
char to_hex(uint8_t nibble) {
    if (nibble < 10) {
        return nibble + '0';
    }
    return nibble - 10 + 'A';
}

// Helper function to convert a hex character to its nibble (4-bit value)
uint8_t to_bin(char hex_char) {
    if (hex_char >= '0' && hex_char <= '9') {
        return hex_char - '0';
    }
    if (hex_char >= 'a' && hex_char <= 'f') {
        return hex_char - 'a' + 10;
    }
    if (hex_char >= 'A' && hex_char <= 'F') {
        return hex_char - 'A' + 10;
    }
    return 0; // Invalid hex character
}

// Function: authenticateToken
int authenticateToken(User *head, char *token_to_auth) {
    User *current = head;
    while (current != NULL) {
        if (current->token != NULL && strcmp(current->token, token_to_auth) == 0) {
            return 1; // Token found
        }
        current = current->next;
    }
    return 0; // Token not found
}

// Function: generateRandomToken
char *generateRandomToken(unsigned int length) {
    if (!random_seeded) {
        srandom(time(NULL));
        random_seeded = true;
    }

    if (length == 0 || length > 1024) { // Max length 0x400 (1024) + 1 null terminator
        return NULL;
    }

    char *token = (char *)malloc(length + 1);
    if (token == NULL) {
        return NULL;
    }
    memset(token, 0, length + 1);

    for (unsigned int i = 0; i < length; ++i) {
        // Original code implies a hyphen every 9th char (0-indexed 8th, 17th etc.)
        if ((i % 9) == 8) {
            token[i] = '-';
        } else {
            // Generate random alphanumeric character
            long rand_val = random();
            if ((rand_val % 3) == 0) { // Digits (0-9)
                token[i] = (rand_val % 10) + '0';
            } else if ((rand_val % 3) == 1) { // Uppercase letters (A-Z)
                token[i] = (rand_val % 26) + 'A';
            } else { // Lowercase letters (a-z)
                token[i] = (rand_val % 26) + 'a';
            }
        }
    }
    return token;
}

// Function: newUser
User *newUser(User **head_ptr, char *name, char *password) {
    if (head_ptr == NULL || name == NULL || password == NULL) {
        return NULL;
    }

    User *new_user = (User *)malloc(sizeof(User));
    if (new_user == NULL) {
        return NULL;
    }
    memset(new_user, 0, sizeof(User)); // Initialize all fields to 0/NULL

    new_user->name = (char *)malloc(strlen(name) + 1);
    if (new_user->name == NULL) {
        free(new_user);
        return NULL;
    }
    strcpy(new_user->name, name);

    new_user->password = (char *)malloc(strlen(password) + 1);
    if (new_user->password == NULL) {
        free(new_user->name);
        free(new_user);
        return NULL;
    }
    strcpy(new_user->password, password);

    // Other fields are initialized to NULL by memset
    // new_user->token = NULL;
    // new_user->subscription_list = NULL;
    // new_user->signing_key = NULL;

    // Prepend new user to the list
    new_user->next = *head_ptr;
    *head_ptr = new_user;

    return new_user;
}

// Function: getUserByToken
User *getUserByToken(User *head, char *token) {
    User *current = head;
    while (current != NULL) {
        if (current->token != NULL && strcmp(current->token, token) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: getUserByName
User *getUserByName(User *head, char *name) {
    User *current = head;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function: newToken
char **newToken(char **token_ptr) {
    if (token_ptr == NULL) {
        return NULL;
    }
    
    // Free existing token if any
    if (*token_ptr != NULL) {
        free(*token_ptr);
        *token_ptr = NULL;
    }

    *token_ptr = generateRandomToken(40); // 0x28 = 40
    return token_ptr;
}

// Function: generateSigningKey
void *generateSigningKey(unsigned int length) {
    if (!random_seeded) {
        srandom(time(NULL));
        random_seeded = true;
    }

    if (length == 0) {
        return NULL;
    }

    uint8_t *key = (uint8_t *)malloc(length + 1); // +1 for null terminator, though it's raw bytes
    if (key == NULL) {
        return NULL;
    }
    memset(key, 0, length + 1);

    for (unsigned int i = 0; i < length; ++i) {
        key[i] = random() % 256; // Fill with random bytes
    }
    return key;
}

// Function: reverseSubscriptionList
void reverseSubscriptionList(SubscriptionNode **head_ptr) {
    if (head_ptr == NULL || *head_ptr == NULL) {
        return;
    }

    SubscriptionNode *prev = NULL;
    SubscriptionNode *current = *head_ptr;
    SubscriptionNode *next_node = NULL;

    while (current != NULL) {
        next_node = current->next; // Store next node
        current->next = prev;      // Reverse current node's pointer
        prev = current;            // Move prev to current
        current = next_node;       // Move current to next
    }
    *head_ptr = prev; // Update head of the list
}

// Function: computeSignature
char *computeSignature(User *user) {
    if (user == NULL || user->subscription_list == NULL) {
        return NULL;
    }

    unsigned int total_data_len = 0;
    for (SubscriptionNode *current_sub = user->subscription_list; current_sub != NULL; current_sub = current_sub->next) {
        if (current_sub->data != NULL) {
            total_data_len += strlen(current_sub->data);
        }
    }

    if (total_data_len == 0) {
        return NULL; // No data to sign
    }

    // Re-generate signing key (original code logic)
    if (user->signing_key != NULL) {
        free(user->signing_key);
        user->signing_key = NULL;
    }
    user->signing_key = generateSigningKey(total_data_len);
    if (user->signing_key == NULL) {
        return NULL;
    }

    uint8_t *payload_buffer = (uint8_t *)malloc(total_data_len + 1);
    if (payload_buffer == NULL) {
        free(user->signing_key);
        user->signing_key = NULL;
        return NULL;
    }
    memset(payload_buffer, 0, total_data_len + 1);

    // Concatenate all subscription data into payload_buffer
    char *current_payload_pos = (char *)payload_buffer;
    for (SubscriptionNode *current_sub = user->subscription_list; current_sub != NULL; current_sub = current_sub->next) {
        if (current_sub->data != NULL) {
            size_t len = strlen(current_sub->data);
            memcpy(current_payload_pos, current_sub->data, len);
            current_payload_pos += len;
        }
    }

    // XOR payload with signing key
    for (unsigned int i = 0; i < total_data_len; ++i) {
        payload_buffer[i] ^= ((uint8_t *)user->signing_key)[i];
    }

    // Convert XORed payload to hex string
    char *hex_signature = (char *)malloc(total_data_len * 2 + 1); // Each byte -> 2 hex chars + null terminator
    if (hex_signature == NULL) {
        free(payload_buffer);
        free(user->signing_key);
        user->signing_key = NULL;
        return NULL;
    }
    memset(hex_signature, 0, total_data_len * 2 + 1);

    for (unsigned int i = 0; i < total_data_len; ++i) {
        hex_signature[i * 2] = to_hex(payload_buffer[i] >> 4);
        hex_signature[i * 2 + 1] = to_hex(payload_buffer[i] & 0xF);
    }

    free(payload_buffer);
    return hex_signature;
}

// Function: verifySignature
int verifySignature(User *user, char *signature) {
    if (user == NULL || user->subscription_list == NULL || user->signing_key == NULL || signature == NULL) {
        return 0; // Invalid input
    }

    unsigned int total_data_len = 0;
    for (SubscriptionNode *current_sub = user->subscription_list; current_sub != NULL; current_sub = current_sub->next) {
        if (current_sub->data != NULL) {
            total_data_len += strlen(current_sub->data);
        }
    }

    if (total_data_len == 0) {
        // If there's no data to sign, and the signature is empty, it's considered valid
        return (strlen(signature) == 0);
    }

    // Check signature length (2 hex chars per byte)
    if (strlen(signature) != total_data_len * 2) {
        return 0; // Signature length mismatch
    }

    uint8_t *payload_buffer = (uint8_t *)malloc(total_data_len + 1);
    if (payload_buffer == NULL) {
        return 0;
    }
    memset(payload_buffer, 0, total_data_len + 1);

    // Concatenate all subscription data into payload_buffer
    char *current_payload_pos = (char *)payload_buffer;
    for (SubscriptionNode *current_sub = user->subscription_list; current_sub != NULL; current_sub = current_sub->next) {
        if (current_sub->data != NULL) {
            size_t len = strlen(current_sub->data);
            memcpy(current_payload_pos, current_sub->data, len);
            current_payload_pos += len;
        }
    }

    // Decode signature hex string into bytes
    uint8_t *decoded_signature = (uint8_t *)malloc(total_data_len + 1);
    if (decoded_signature == NULL) {
        free(payload_buffer);
        return 0;
    }
    memset(decoded_signature, 0, total_data_len + 1);

    for (unsigned int i = 0; i < total_data_len; ++i) {
        uint8_t high_nibble = to_bin(signature[i * 2]);
        uint8_t low_nibble = to_bin(signature[i * 2 + 1]);
        decoded_signature[i] = (high_nibble << 4) | low_nibble;
    }

    // Compare original data XORed with key, to decoded signature
    int result = 1; // Assume valid
    for (unsigned int i = 0; i < total_data_len; ++i) {
        if ((payload_buffer[i] ^ ((uint8_t *)user->signing_key)[i]) != decoded_signature[i]) {
            result = 0; // Mismatch
            break;
        }
    }

    free(payload_buffer);
    free(decoded_signature);
    return result;
}

// Main function for demonstration and compilation
int main() {
    // Seed random only once
    srandom(time(NULL));
    random_seeded = true;

    User *user_list_head = NULL;
    
    // Test newUser
    User *user1 = newUser(&user_list_head, "alice", "pass123");
    if (user1) {
        printf("User 1 created: %s\n", user1->name);
    }

    User *user2 = newUser(&user_list_head, "bob", "securepwd");
    if (user2) {
        printf("User 2 created: %s\n", user2->name);
    }

    // Test newToken
    newToken(&user1->token);
    if (user1->token) {
        printf("User 1 token: %s\n", user1->token);
    }

    // Test authenticateToken
    if (authenticateToken(user_list_head, user1->token)) {
        printf("Token authenticated for %s\n", user1->name);
    } else {
        printf("Token authentication failed for %s\n", user1->name);
    }

    // Add subscriptions to user1
    SubscriptionNode *sub1 = (SubscriptionNode *)malloc(sizeof(SubscriptionNode));
    sub1->data = strdup("premium");
    sub1->field1 = NULL;
    sub1->field2 = NULL;
    sub1->next = NULL;

    SubscriptionNode *sub2 = (SubscriptionNode *)malloc(sizeof(SubscriptionNode));
    sub2->data = strdup("newsletter");
    sub2->field1 = NULL;
    sub2->field2 = NULL;
    sub2->next = sub1; // Prepend to list

    user1->subscription_list = sub2;

    printf("User 1 subscriptions before reverse: %s, %s\n", user1->subscription_list->data, user1->subscription_list->next->data);
    reverseSubscriptionList(&user1->subscription_list);
    printf("User 1 subscriptions after reverse: %s, %s\n", user1->subscription_list->data, user1->subscription_list->next->data);

    // Test computeSignature
    char *signature = computeSignature(user1);
    if (signature) {
        printf("Computed signature for %s: %s\n", user1->name, signature);
    }

    // Test verifySignature
    if (signature && verifySignature(user1, signature)) {
        printf("Signature verified for %s\n", user1->name);
    } else {
        printf("Signature verification FAILED for %s\n", user1->name);
    }

    // --- Cleanup (simplified for example) ---
    // Free subscription list
    SubscriptionNode *current_sub = user1->subscription_list;
    while (current_sub != NULL) {
        SubscriptionNode *temp = current_sub;
        current_sub = current_sub->next;
        free(temp->data);
        free(temp);
    }
    user1->subscription_list = NULL;

    // Free user1
    if (user1->token) free(user1->token);
    if (user1->name) free(user1->name);
    if (user1->password) free(user1->password);
    if (user1->signing_key) free(user1->signing_key);
    free(user1);
    user1 = NULL; // Prevent double free issues in example if user_list_head was used

    // Free user2
    if (user2->name) free(user2->name);
    if (user2->password) free(user2->password);
    free(user2);
    user2 = NULL;

    if (signature) free(signature);
    signature = NULL;

    user_list_head = NULL; // All users freed

    return 0;
}