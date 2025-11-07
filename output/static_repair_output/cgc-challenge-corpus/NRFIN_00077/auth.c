#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free, rand, srand
#include <string.h>  // For strlen, strcmp, strcpy, memset, strcat, memcpy
#include <stdint.h>  // For uint32_t, uint8_t
#include <time.h>    // For time (to seed rand)
#include <ctype.h>   // For isalnum, tolower (if needed, or custom char generation)

// This code assumes a 32-bit system where pointers are 4 bytes,
// based on the memory access patterns (offsets) in the original snippet.
// If compiled on a 64-bit system, the struct sizes and offsets would be incorrect,
// leading to undefined behavior or crashes.

// Struct for User nodes in a linked list
typedef struct User {
    uint32_t field0;    // @0x00 - Original `*__ptr = 0`
    char *name;         // @0x04 - Original `__ptr[1]`
    char *password;     // @0x08 - Original `__ptr[2]`
    uint32_t field3;    // @0x0c - Original `__ptr[3]`
    uint32_t field4;    // @0x10 - Original `__ptr[4]`
    struct User *next;  // @0x14 - Original `__ptr[5]`
} User; // Total size 24 bytes (0x18)

// Struct for AuthToken nodes in a linked list
// This struct's first field is `token_str` and it's used with `char **` pointers.
typedef struct AuthToken {
    char *token_str;    // @0x00 - Original `*local_10` in authenticateToken/getUserByToken
    // The other fields are not explicitly used by name in token functions, but fill the 0x14 gap
    void *field1;       // @0x04
    void *field2;       // @0x08
    void *field3;       // @0x0c
    void *field4;       // @0x10
    struct AuthToken *next; // @0x14 - Original `local_10[5]`
} AuthToken; // Total size 24 bytes (0x18)

// Struct for Signature Data nodes in a linked list
typedef struct SignatureDataNode {
    char *data;                 // @0x00 - Original `*local_18`
    void *field1;               // @0x04
    void *field2;               // @0x08
    struct SignatureDataNode *next; // @0x0c - Original `local_18[3]`
} SignatureDataNode; // Total size 16 bytes (0x10)

// Struct for Signature Context
typedef struct SignatureContext {
    void *field0;                   // @0x00
    void *field1;                   // @0x04
    // `param_1 + 8` implies this is the head of the subscription list
    SignatureDataNode *subscription_list_head; // @0x08
    char *signature_key;            // @0x0c - Original `*(undefined4 *)(param_1 + 0xc)`
    SignatureDataNode *data_list_head; // @0x10 - Original `*(char ***)(param_1 + 0x10)`
} SignatureContext;


// Helper function: to_bin (converts hex char to int)
// Assumes input is a valid hex character.
static int to_bin(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0; // Should not happen with valid hex input
}

// Helper function: to_hex (converts int nibble to hex char)
// Assumes input is 0-15.
static char to_hex(uint8_t nibble) {
    if (nibble < 10) return nibble + '0';
    return nibble - 10 + 'a'; // Use lowercase hex
}

// Function: authenticateToken
// head is a pointer to the head of a linked list of AuthToken structs
// token_to_auth is the token string to authenticate
uint32_t authenticateToken(AuthToken **head, char *token_to_auth) {
    AuthToken *current = *head;
    while (current != NULL) {
        if (current->token_str != NULL && strcmp(current->token_str, token_to_auth) == 0) {
            return 1; // Token found
        }
        current = current->next; // Move to the next node
    }
    return 0; // Token not found
}

// Function: generateRandomToken
// length is the desired length of the token
void *generateRandomToken(unsigned int length) {
    if (length == 0 || length >= 0x401) { // Max length 1024
        return NULL;
    }

    char *token = (char *)malloc(length + 1);
    if (token == NULL) {
        return NULL;
    }
    memset(token, 0, length + 1);

    for (unsigned int i = 0; i < length; ++i) {
        if (i > 0 && (i % 9) == 0) { // Every 9th character (0-indexed) is a dash
            token[i] = '-';
        } else {
            uint8_t random_byte = rand(); // Get a random byte
            if ((random_byte & 1) == 0) { // Even random_byte
                if ((random_byte & 3) == 0) { // Multiple of 4 (0, 4, 8, ...) -> 'A'-'Z'
                    token[i] = (random_byte % 26) + 'A';
                } else { // Multiple of 2 but not 4 (2, 6, 10, ...) -> 'a'-'z'
                    token[i] = (random_byte % 26) + 'a';
                }
            } else { // Odd random_byte -> '0'-'9'
                token[i] = (random_byte % 10) + '0';
            }
        }
    }
    token[length] = '\0'; // Ensure null termination
    return token;
}

// Function: newUser
// head_ptr is a pointer to the head of the User linked list (User**)
// username is the username string
// password is the password string
User *newUser(User **head_ptr, char *username, char *password) {
    User *new_user = (User *)malloc(sizeof(User)); // Allocate 0x18 (24 bytes)
    if (new_user == NULL) {
        return NULL;
    }

    // Allocate and copy username
    new_user->name = (char *)malloc(strlen(username) + 1);
    if (new_user->name == NULL) {
        free(new_user);
        return NULL;
    }
    strcpy(new_user->name, username);

    // Allocate and copy password
    new_user->password = (char *)malloc(strlen(password) + 1);
    if (new_user->password == NULL) {
        free(new_user->name);
        free(new_user);
        return NULL;
    }
    strcpy(new_user->password, password);

    // Initialize other fields and link to list
    new_user->field0 = 0;
    new_user->field3 = 0;
    new_user->field4 = 0;
    new_user->next = *head_ptr; // Link new user to the current head
    *head_ptr = new_user;       // Update head to be the new user

    return new_user;
}

// Function: getUserByToken
// head is a pointer to the head of a linked list of AuthToken structs
// token_to_find is the token string to search for
AuthToken *getUserByToken(AuthToken **head, char *token_to_find) {
    AuthToken *current = *head;
    while (current != NULL) {
        if (current->token_str != NULL && strcmp(current->token_str, token_to_find) == 0) {
            return current; // Token node found
        }
        current = current->next; // Move to the next node
    }
    return NULL; // Token node not found
}

// Function: getUserByName
// head is the head of the User linked list
// username_to_find is the username string to search for
User *getUserByName(User *head, char *username_to_find) {
    User *current = head;
    while (current != NULL) {
        if (strcmp(current->name, username_to_find) == 0) {
            return current; // User found
        }
        current = current->next; // Move to the next user
    }
    return NULL; // User not found
}

// Function: newToken
// token_ptr_location is a pointer to a location where the new token string should be stored (char**)
// The function generates a random token and stores its pointer in *token_ptr_location
void *newToken(char **token_ptr_location) {
    *token_ptr_location = (char *)generateRandomToken(0x28); // 0x28 = 40 characters
    return token_ptr_location; // Return the address where the token pointer is stored
}

// Function: generateSigningKey
// length is the desired length of the signing key
void *generateSigningKey(unsigned int length) {
    char *key = (char *)malloc(length + 1);
    if (key == NULL) {
        return NULL;
    }
    memset(key, 0, length + 1);

    // Fill the key with random bytes
    for (unsigned int i = 0; i < length; ++i) {
        key[i] = rand() % 256; // Generate a random byte
    }
    key[length] = '\0'; // Ensure null termination (though it's a binary key)
    return key;
}

// Function: reverseSubscriptionList
// head_ptr is a pointer to the head of a SignatureDataNode linked list (SignatureDataNode**)
void reverseSubscriptionList(SignatureDataNode **head_ptr) {
    SignatureDataNode *prev = NULL;
    SignatureDataNode *current = *head_ptr;
    SignatureDataNode *next_node = NULL;

    while (current != NULL) {
        next_node = current->next; // Store next node
        current->next = prev;      // Reverse current node's pointer
        prev = current;            // Move pointers one position ahead
        current = next_node;
    }
    *head_ptr = prev; // Update head pointer to the new first node
}

// Function: verifySignature
// context is a pointer to a SignatureContext struct
// signature_to_verify is the hex-encoded signature string to verify against
uint32_t verifySignature(SignatureContext *context, char *signature_to_verify) {
    // Original code: reverseSubscriptionList(param_1 + 8);
    // This implies context->subscription_list_head is passed by address.
    reverseSubscriptionList(&(context->subscription_list_head));

    SignatureDataNode *current_data = context->subscription_list_head;
    unsigned int signature_idx = 0; // Index for the signature_to_verify string

    while (current_data != NULL) {
        if (current_data->data == NULL) {
            current_data = current_data->next;
            continue;
        }
        size_t data_len = strlen(current_data->data);

        for (unsigned int i = 0; i < data_len; ++i) {
            // Check if signature_to_verify has enough characters (2 hex chars per byte)
            if (signature_to_verify[signature_idx] == '\0' || signature_to_verify[signature_idx + 1] == '\0') {
                return 0; // Signature string too short
            }

            // Decode two hex characters from signature_to_verify into a single byte
            int nibble1 = to_bin(signature_to_verify[signature_idx]);
            int nibble2 = to_bin(signature_to_verify[signature_idx + 1]);
            uint8_t received_signed_byte = (uint8_t)((nibble1 << 4) | nibble2);

            // Ensure key is long enough for the current byte index
            if (context->signature_key == NULL || (signature_idx / 2) >= strlen(context->signature_key)) {
                 return 0; // Key too short or missing
            }

            // Calculate the expected signed byte: original_data_byte XOR key_byte
            uint8_t expected_signed_byte = (uint8_t)current_data->data[i] ^ (uint8_t)context->signature_key[signature_idx / 2];

            if (received_signed_byte != expected_signed_byte) {
                return 0; // Mismatch
            }
            signature_idx += 2; // Advance by 2 for the next hex byte in the signature string
        }
        current_data = current_data->next;
    }
    // If we reach here, all data segments matched the signature.
    // Check if the entire signature_to_verify string was consumed.
    if (signature_to_verify[signature_idx] != '\0') {
        return 0; // Signature has extra characters
    }
    return 1; // Signature verified
}

// Function: computeSignature
// context is a pointer to a SignatureContext struct
void *computeSignature(SignatureContext *context) {
    size_t total_data_len = 0;
    SignatureDataNode *current_data_node = context->data_list_head;

    // Calculate total length of concatenated data
    while (current_data_node != NULL) {
        if (current_data_node->data != NULL) {
            total_data_len += strlen(current_data_node->data);
        }
        current_data_node = current_data_node->next;
    }

    if (total_data_len == 0) {
        return NULL; // No data to sign
    }

    // Generate signing key (if not already done, or regenerate for this signature)
    // The original code assigns to context->signature_key (offset 0xc)
    context->signature_key = (char *)generateSigningKey(total_data_len);
    if (context->signature_key == NULL) {
        return NULL;
    }

    // Allocate buffer for concatenated data (raw bytes)
    uint8_t *concatenated_data = (uint8_t *)malloc(total_data_len + 1);
    if (concatenated_data == NULL) {
        free(context->signature_key);
        context->signature_key = NULL;
        return NULL;
    }
    memset(concatenated_data, 0, total_data_len + 1);

    // Concatenate all data strings
    size_t current_offset = 0;
    current_data_node = context->data_list_head;
    while (current_data_node != NULL) {
        if (current_data_node->data != NULL) {
            size_t len = strlen(current_data_node->data);
            memcpy(concatenated_data + current_offset, current_data_node->data, len);
            current_offset += len;
        }
        current_data_node = current_data_node->next;
    }

    // Apply XOR with signing key
    for (unsigned int i = 0; i < total_data_len; ++i) {
        concatenated_data[i] ^= (uint8_t)context->signature_key[i];
    }

    // Allocate buffer for hex-encoded signature (2 chars per byte + null terminator)
    char *hex_signature = (char *)malloc(total_data_len * 2 + 1);
    if (hex_signature == NULL) {
        free(concatenated_data);
        free(context->signature_key);
        context->signature_key = NULL;
        return NULL;
    }
    memset(hex_signature, 0, total_data_len * 2 + 1);

    // Convert raw signature bytes to hex string
    for (unsigned int i = 0; i < total_data_len; ++i) {
        hex_signature[i * 2] = to_hex(concatenated_data[i] >> 4);
        hex_signature[i * 2 + 1] = to_hex(concatenated_data[i] & 0xF);
    }
    hex_signature[total_data_len * 2] = '\0'; // Null-terminate

    free(concatenated_data); // Free intermediate concatenated data buffer

    return hex_signature;
}


// --- Main function for demonstration ---
int main() {
    srand(time(NULL)); // Seed the random number generator once

    printf("--- User Management ---\n");
    User *user_list_head = NULL;
    User *u1 = newUser(&user_list_head, "alice", "pass123");
    User *u2 = newUser(&user_list_head, "bob", "securepwd");
    User *u3 = newUser(&user_list_head, "charlie", "secret");

    if (u1) printf("User 'alice' created.\n");
    if (u2) printf("User 'bob' created.\n");
    if (u3) printf("User 'charlie' created.\n");

    User *found_user = getUserByName(user_list_head, "bob");
    if (found_user) {
        printf("Found user by name: %s (password: %s)\n", found_user->name, found_user->password);
    } else {
        printf("User 'bob' not found.\n");
    }

    found_user = getUserByName(user_list_head, "david");
    if (found_user) {
        printf("Found user by name: %s\n", found_user->name);
    } else {
        printf("User 'david' not found.\n");
    }

    printf("\n--- Token Management ---\n");
    AuthToken *token_list_head = NULL;
    char *token1_str = NULL;
    newToken(&token1_str); // token1_str now points to a malloc'd string
    if (token1_str) {
        printf("Generated token 1: %s\n", token1_str);
        AuthToken *new_token_node = (AuthToken *)malloc(sizeof(AuthToken));
        if (new_token_node) {
            new_token_node->token_str = token1_str;
            new_token_node->field1 = NULL; new_token_node->field2 = NULL;
            new_token_node->field3 = NULL; new_token_node->field4 = NULL;
            new_token_node->next = token_list_head;
            token_list_head = new_token_node;
        } else {
            free(token1_str); // If node allocation fails, free token string
            token1_str = NULL;
        }
    }

    char *token2_str = NULL;
    newToken(&token2_str); // token2_str now points to a malloc'd string
    if (token2_str) {
        printf("Generated token 2: %s\n", token2_str);
        AuthToken *new_token_node = (AuthToken *)malloc(sizeof(AuthToken));
        if (new_token_node) {
            new_token_node->token_str = token2_str;
            new_token_node->field1 = NULL; new_token_node->field2 = NULL;
            new_token_node->field3 = NULL; new_token_node->field4 = NULL;
            new_token_node->next = token_list_head;
            token_list_head = new_token_node;
        } else {
            free(token2_str); // If node allocation fails, free token string
            token2_str = NULL;
        }
    }

    uint32_t auth_result = authenticateToken(&token_list_head, token1_str);
    printf("Authenticating token 1 (%s): %s\n", token1_str, auth_result ? "Success" : "Failed");

    auth_result = authenticateToken(&token_list_head, "INVALIDTOKEN");
    printf("Authenticating 'INVALIDTOKEN': %s\n", auth_result ? "Success" : "Failed");

    AuthToken *found_token_node = getUserByToken(&token_list_head, token2_str);
    if (found_token_node) {
        printf("Found token node for token 2: %s\n", found_token_node->token_str);
    } else {
        printf("Token 2 node not found.\n");
    }

    printf("\n--- Signature Management ---\n");
    SignatureContext sig_ctx = {0}; // Initialize all fields to 0/NULL

    // Add some data to be signed. Note: data is added to the head, so order will be reversed in list.
    SignatureDataNode *data1 = (SignatureDataNode *)malloc(sizeof(SignatureDataNode));
    if (data1) {
        data1->data = strdup("Hello"); // strdup allocates memory
        data1->field1 = NULL; data1->field2 = NULL;
        data1->next = sig_ctx.data_list_head;
        sig_ctx.data_list_head = data1;
    }
    SignatureDataNode *data2 = (SignatureDataNode *)malloc(sizeof(SignatureDataNode));
    if (data2) {
        data2->data = strdup("World"); // strdup allocates memory
        data2->field1 = NULL; data2->field2 = NULL;
        data2->next = sig_ctx.data_list_head;
        sig_ctx.data_list_head = data2;
    }

    printf("Original data list order (for signature, from head): ");
    SignatureDataNode *current_sig_data = sig_ctx.data_list_head;
    while(current_sig_data) {
        printf("%s ", current_sig_data->data);
        current_sig_data = current_sig_data->next;
    }
    printf("\n");

    char *signature = (char *)computeSignature(&sig_ctx);
    if (signature) {
        printf("Computed signature: %s\n", signature);
        printf("Signing key (hex): ");
        if (sig_ctx.signature_key) {
            for(size_t i = 0; i < strlen(sig_ctx.signature_key); ++i) {
                printf("%02x", (uint8_t)sig_ctx.signature_key[i]);
            }
        }
        printf("\n");

        // For verification, the context's subscription_list_head is used, which is then reversed.
        // Let's copy the data_list_head to subscription_list_head for this example.
        // In a real scenario, these lists would be managed separately or the context would be pre-populated.
        // For demonstration, we'll manually set it up, assuming data_list_head is the "input" for signature calculation
        // and subscription_list_head is the "input" for verification (which gets reversed).
        // To simplify, we'll just use data_list_head for both and `reverseSubscriptionList` will operate on it.
        // The original code passed `param_1 + 8` (context->subscription_list_head) to reverseSubscriptionList,
        // so we populate it similarly for a valid test.
        // Let's assume the signature is computed on `data_list_head` and `verifySignature` checks against `subscription_list_head`.
        // To make the example work, let's copy `data_list_head` to `subscription_list_head`.
        // This is a simplification for the main function, not part of the original logic.
        sig_ctx.subscription_list_head = sig_ctx.data_list_head;

        uint32_t verify_result = verifySignature(&sig_ctx, signature);
        printf("Verifying signature: %s\n", verify_result ? "Success" : "Failed");

        // Test with a tampered signature
        char *tampered_signature = strdup(signature);
        if (tampered_signature && strlen(tampered_signature) > 0) {
            tampered_signature[0] = (tampered_signature[0] == 'a') ? 'b' : 'a'; // Tamper first hex char
            printf("Verifying tampered signature (%s): %s\n", tampered_signature, verifySignature(&sig_ctx, tampered_signature) ? "Success" : "Failed");
            free(tampered_signature);
        }
        
        free(signature);
    } else {
        printf("Failed to compute signature.\n");
    }

    printf("\n--- Cleanup ---\n");
    // Free User list
    User *current_user = user_list_head;
    while (current_user != NULL) {
        User *temp = current_user;
        current_user = current_user->next;
        free(temp->name);
        free(temp->password);
        free(temp);
    }

    // Free AuthToken list
    AuthToken *current_token = token_list_head;
    while (current_token != NULL) {
        AuthToken *temp = current_token;
        current_token = current_token->next;
        free(temp->token_str); // Token string was malloc'd by generateRandomToken
        free(temp); // The AuthToken node itself
    }

    // Free SignatureContext data
    // Note: sig_ctx.subscription_list_head and sig_ctx.data_list_head might point to the same list if setup that way for demo.
    // Ensure we only free nodes once.
    SignatureDataNode *current_sig_node = sig_ctx.data_list_head; // Assume data_list_head is the canonical list
    while (current_sig_node != NULL) {
        SignatureDataNode *temp = current_sig_node;
        current_sig_node = current_sig_node->next;
        free(temp->data);
        free(temp);
    }
    if (sig_ctx.signature_key) {
        free(sig_ctx.signature_key);
    }

    return 0;
}