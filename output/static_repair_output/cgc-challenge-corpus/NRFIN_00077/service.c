#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For uint32_t or similar, if specific integer widths are needed.
#include <unistd.h> // For potential use of file descriptors, though transmit_all is a placeholder.

// Placeholder for _terminate, usually exit(1) in critical error paths
void _terminate() {
    fprintf(stderr, "Error: Program terminated unexpectedly.\n");
    exit(1);
}

// External functions (placeholders for compilation)
// In a real scenario, these would be defined in other source files or libraries.
void transmit_all(int fd, const char *buf, size_t len) {
    // Example: write(fd, buf, len);
    // For this exercise, just a placeholder.
    (void)fd; // Suppress unused parameter warning
    (void)buf;
    (void)len;
    // fprintf(stderr, "TRANSMIT (fd %d, len %zu): %s\n", fd, len, buf);
}
void *getMessageById(void *channel_messages, int message_id) { (void)channel_messages; (void)message_id; return NULL; }
void cleanupChannel(void *channel) { (void)channel; }
void *getSubscription(void *subscriptions, const char *channel_id_or_name) { (void)subscriptions; (void)channel_id_or_name; return NULL; }
void *getChannel(void *channels, const char *channel_name) { (void)channels; (void)channel_name; return NULL; }
void *getLastMessage(void *channel_messages) { (void)channel_messages; return NULL; }
void *computeSignature(void *user_data) { (void)user_data; return strdup("DUMMY_SIGNATURE"); } // Returns a char*
void setDeliveryType(void *subscription, const char *type) { (void)subscription; (void)type; }
void *getUserByName(void *users, const char *name) { (void)users; (void)name; return NULL; }
int verifySignature(void *auth_response, void *user_secret) { (void)auth_response; (void)user_secret; return 1; } // Assume valid for now
void *newToken(void *user_data) { // Returns a char** (token string and subscription info)
    (void)user_data;
    char **token_info = (char**)malloc(sizeof(char*) * 2);
    if (token_info) {
        token_info[0] = strdup("DUMMY_TOKEN");
        token_info[1] = strdup("DUMMY_SUB_INFO"); // Placeholder for subscription type
    }
    return token_info;
}
void *newUser(void *users, const char *username, const char *password) { (void)users; (void)username; (void)password; return (void*)1; } // Return non-NULL as a placeholder user_data
void addSubscriptions(void *channels, void *user_subscriptions, const char *username, const char *channel_name) { (void)channels; (void)user_subscriptions; (void)username; (void)channel_name; }
int authenticateToken(void *users, const char *token) { (void)users; (void)token; return 1; } // Assume valid for now
void *getUserByToken(void *users, const char *token) { (void)users; (void)token; return (void*)1; } // Return non-NULL as a placeholder user_data

// Placeholder for DAT_4347c000
// Assuming it's a large array of random bytes for generation purposes.
// For compilation, we'll make it static and provide some dummy data.
static unsigned char DAT_4347c000[4096];

// Helper function to convert a nibble to its hex character
char to_hex(unsigned char nibble) {
    if (nibble < 10) {
        return '0' + nibble;
    }
    return 'a' + (nibble - 10);
}

// Function: newRequest
char ** newRequest(char *param_1, char *param_2) {
    char **request_info = NULL;
    char *path_segment = NULL;
    char *input_buffer = NULL;
    char *token = NULL;
    size_t len;

    if (param_2 == NULL || *param_2 != '/') {
        return NULL;
    }

    path_segment = param_2 + 1; // Skip the leading '/'

    request_info = (char **)malloc(sizeof(char *) * 4); // Allocate space for 4 char*
    if (request_info == NULL) {
        return NULL;
    }
    memset(request_info, 0, sizeof(char *) * 4);

    len = strlen(param_1);
    request_info[0] = (char *)malloc(len + 1);
    if (request_info[0] == NULL) {
        free(request_info);
        return NULL;
    }
    strcpy(request_info[0], param_1);

    len = strlen(path_segment);
    request_info[1] = (char *)malloc(len + 1);
    if (request_info[1] == NULL) {
        free(request_info[0]);
        free(request_info);
        return NULL;
    }
    strcpy(request_info[1], path_segment);

    input_buffer = (char *)malloc(0x400); // Max 1024 bytes
    if (input_buffer == NULL) {
        free(request_info[1]);
        free(request_info[0]);
        free(request_info);
        return NULL;
    }
    memset(input_buffer, 0, 0x400);

    if (fgets(input_buffer, 0x3ff, stdin) == NULL) {
        free(input_buffer);
        free(request_info[1]); free(request_info[0]); free(request_info);
        return NULL;
    }

    len = strlen(input_buffer);
    if (len == 0 || input_buffer[0] == '\n') {
        free(input_buffer);
        free(request_info[1]); free(request_info[0]); free(request_info);
        return NULL;
    }
    if (input_buffer[len - 1] == '\n') {
        input_buffer[len - 1] = '\0';
        len--;
    }

    token = strtok(input_buffer, ":");
    if (token == NULL) {
        free(input_buffer);
        free(request_info[1]); free(request_info[0]); free(request_info);
        return NULL;
    }

    len = strlen(token);
    request_info[2] = (char *)malloc(len + 1);
    if (request_info[2] == NULL) {
        free(input_buffer);
        free(request_info[1]); free(request_info[0]); free(request_info);
        return NULL;
    }
    strcpy(request_info[2], token);

    token = strtok(NULL, ":");
    if (token == NULL) {
        free(input_buffer);
        free(request_info[2]); free(request_info[1]); free(request_info[0]); free(request_info);
        return NULL;
    }

    len = strlen(token);
    request_info[3] = (char *)malloc(len + 1);
    if (request_info[3] == NULL) {
        free(input_buffer);
        free(request_info[2]); free(request_info[1]); free(request_info[0]); free(request_info);
        return NULL;
    }
    strcpy(request_info[3], token);

    free(input_buffer);
    return request_info;
}

// Function: parseCommand
char ** parseCommand(char *param_1) {
    size_t len;
    char **request_data = NULL;

    if (param_1 == NULL) return NULL;

    if (strncmp("/subscribe", param_1, len = strlen("/subscribe")) == 0) {
        request_data = newRequest("/subscribe", param_1 + len);
    } else if (strncmp("/out", param_1, len = strlen("/out")) == 0) {
        request_data = newRequest("/out", param_1 + len);
    } else if (strncmp("/auth", param_1, len = strlen("/auth")) == 0) {
        request_data = newRequest("/auth", param_1 + len);
    } else if (strncmp("/token", param_1, len = strlen("/token")) == 0) {
        request_data = newRequest("/token", param_1 + len);
    } else if (strncmp("/in", param_1, len = strlen("/in")) == 0) {
        request_data = newRequest("/in", param_1 + len);
    }
    return request_data;
}

// Function: getCommand
char * getCommand(void) {
    char *input_buffer = (char *)malloc(0x400);
    if (input_buffer == NULL) {
        return NULL;
    }
    memset(input_buffer, 0, 0x400);

    if (fgets(input_buffer, 0x3ff, stdin) == NULL) {
        free(input_buffer);
        return NULL;
    }

    size_t len = strlen(input_buffer);
    if (len == 0) {
        free(input_buffer);
        return NULL;
    }
    if (input_buffer[len - 1] == '\n') {
        input_buffer[len - 1] = '\0';
    }
    return input_buffer;
}

// Function: getRandomString
void * getRandomString(unsigned int length) {
    unsigned char *result_str = (unsigned char *)malloc(length + 1);
    if (result_str == NULL) {
        return NULL;
    }
    memset(result_str, 0, length + 1);

    for (unsigned int i = 0; i < length; i++) {
        unsigned char byte_val = DAT_4347c000[i % sizeof(DAT_4347c000)]; // Use modulo for smaller array
        if ((byte_val & 1) == 0) {
            if ((byte_val & 3) == 0) {
                result_str[i] = (byte_val % 0x1a) + 'A';
            } else {
                result_str[i] = (byte_val % 0x1a) + 'a';
            }
        } else if ((byte_val % 7) == 0) {
            result_str[i] = ' ';
        } else {
            result_str[i] = (byte_val % 10) + '0';
        }
    }
    return result_str;
}

// Function: sendAllPriorityMessages
void sendAllPriorityMessages(void *channel_ptr, void *sub_user_ptr, void *sub_channel_ptr) {
    void *message_ptr = getMessageById(*(void **)((char *)channel_ptr + 0xc), *(int *)((char *)sub_user_ptr + 8) + 1);
    if (message_ptr != NULL) {
        for (; message_ptr != NULL; message_ptr = *(void **)((char *)message_ptr + 0x10)) {
            if (strcmp(*(char **)((char *)message_ptr + 0xc), *(char **)((char *)sub_user_ptr + 4)) == 0) {
                transmit_all(1, *(char **)((char *)message_ptr + 8), strlen(*(char **)((char *)message_ptr + 8)));
                transmit_all(1, "\n", strlen("\n"));
            }
            *(int *)((char *)sub_user_ptr + 8) = *(int *)message_ptr;
            *(int *)((char *)sub_channel_ptr + 8) = *(int *)message_ptr;
        }
        cleanupChannel(channel_ptr);
    }
}

// Function: sendAllMessages
void sendAllMessages(void *channel_ptr, void *sub_user_ptr, void *sub_channel_ptr) {
    void *message_ptr = getMessageById(*(void **)((char *)channel_ptr + 0xc), *(int *)((char *)sub_user_ptr + 8) + 1);
    if (message_ptr != NULL) {
        for (; message_ptr != NULL; message_ptr = *(void **)((char *)message_ptr + 0x10)) {
            transmit_all(1, *(char **)((char *)message_ptr + 8), strlen(*(char **)((char *)message_ptr + 8)));
            transmit_all(1, "\n", strlen("\n"));
            *(int *)((char *)sub_user_ptr + 8) = *(int *)message_ptr;
            *(int *)((char *)sub_channel_ptr + 8) = *(int *)message_ptr;
        }
        cleanupChannel(channel_ptr);
    }
}

// Function: sendLatestMessage
void sendLatestMessage(void *channel_ptr, void *sub_user_ptr, void *sub_channel_ptr) {
    void *message_ptr = getLastMessage(*(void **)((char *)channel_ptr + 0xc));
    if ((message_ptr != NULL) && (*(int *)message_ptr != *(int *)((char *)sub_user_ptr + 8))) {
        transmit_all(1, *(char **)((char *)message_ptr + 8), strlen(*(char **)((char *)message_ptr + 8)));
        transmit_all(1, "\n", strlen("\n"));
        *(int *)((char *)sub_user_ptr + 8) = *(int *)message_ptr;
        *(int *)((char *)sub_channel_ptr + 8) = *(int *)message_ptr;
        cleanupChannel(channel_ptr);
    }
}

// Function: sendNextMessage
void sendNextMessage(void *channel_ptr, void *sub_user_ptr, void *sub_channel_ptr) {
    void *message_ptr = getMessageById(*(void **)((char *)channel_ptr + 0xc), *(int *)((char *)sub_user_ptr + 8) + 1);
    if (message_ptr != NULL) {
        transmit_all(1, *(char **)((char *)message_ptr + 8), strlen(*(char **)((char *)message_ptr + 8)));
        transmit_all(1, "\n", strlen("\n"));
        *(int *)((char *)sub_user_ptr + 8) = *(int *)message_ptr;
        *(int *)((char *)sub_channel_ptr + 8) = *(int *)message_ptr;
        cleanupChannel(channel_ptr);
    }
}

// Function: getMessagesFromChannel
void getMessagesFromChannel(void *channel_ptr, void *user_data_ptr) {
    void *sub_user = getSubscription(*(void **)((char *)user_data_ptr + 0x10), *(char **)channel_ptr);
    if (sub_user == NULL) return;

    void *sub_channel = getSubscription(*(void **)((char *)channel_ptr + 0x10), *(char **)((char *)user_data_ptr + 4));
    if (sub_channel == NULL) return;

    char *delivery_type = *(char **)((char *)sub_user + 4);
    if (delivery_type != NULL) {
        if (strcmp(delivery_type, "guaranteed") == 0) {
            sendAllMessages(channel_ptr, sub_user, sub_channel);
        } else if (strcmp(delivery_type, "latest") == 0) {
            sendLatestMessage(channel_ptr, sub_user, sub_channel);
        } else if (strcmp(delivery_type, "next") == 0) {
            sendNextMessage(channel_ptr, sub_user, sub_channel);
        } else if (strcmp(delivery_type, "high") == 0 ||
                   strcmp(delivery_type, "medium") == 0 ||
                   strcmp(delivery_type, "low") == 0) {
            sendAllPriorityMessages(channel_ptr, sub_user, sub_channel);
        }
    }
}

// Function: getMessagesFromAllChannels
void getMessagesFromAllChannels(void *channels_list_ptr, void *user_data_ptr) {
    void *current_sub = *(void **)((char *)user_data_ptr + 0x10);
    for (; current_sub != NULL; current_sub = *(void **)((char *)current_sub + 0xc)) {
        void *channel = getChannel(channels_list_ptr, *(char **)current_sub);
        if (channel != NULL) {
            getMessagesFromChannel(channel, user_data_ptr);
        }
    }
}

// Function: sendMessage
void sendMessage(void *channels_list_ptr, char **request_info, void *user_data_ptr) {
    if (request_info == NULL || request_info[2] == NULL) return;

    void *channel = getChannel(channels_list_ptr, request_info[1]);
    if (channel == NULL) return;

    void *sub_user = getSubscription(*(void **)((char *)channel + 0x10), *(char **)((char *)user_data_ptr + 4));
    if (sub_user == NULL) return;

    void *sub_channel = getSubscription(*(void **)((char *)user_data_ptr + 0x10), *(char **)((char *)channel + 4));
    if (sub_channel == NULL) return;

    // Assuming a 32-bit Message struct layout: {int id; char* sender; char* content; char* priority; void* next;}
    void *new_message = malloc(20); // 0x14 bytes
    if (new_message == NULL) return;
    memset(new_message, 0, 20);

    char *auth_str_copy = strdup(request_info[2]); // strtok modifies the string
    if (auth_str_copy == NULL) { free(new_message); return; }

    char *content_token = strtok(auth_str_copy, ":");
    char *priority_token = strtok(NULL, ":");

    if (content_token == NULL || priority_token == NULL) {
        free(auth_str_copy); free(new_message); return;
    }

    *(char **)((char *)new_message + 8) = strdup(content_token); // content
    if (*(char **)((char *)new_message + 8) == NULL) { free(auth_str_copy); free(new_message); return; }

    *(char **)((char *)new_message + 12) = strdup(priority_token); // priority
    if (*(char **)((char *)new_message + 12) == NULL) {
        free(*(char **)((char *)new_message + 8)); free(auth_str_copy); free(new_message); return;
    }

    *(char **)((char *)new_message + 4) = strdup(*(char **)((char *)user_data_ptr + 4)); // sender
    if (*(char **)((char *)new_message + 4) == NULL) {
        free(*(char **)((char *)new_message + 12)); free(*(char **)((char *)new_message + 8)); free(auth_str_copy); free(new_message); return;
    }

    void *last_message = getLastMessage(*(void **)((char *)channel + 0xc));
    if (last_message == NULL) {
        *(void **)((char *)channel + 0xc) = new_message;
    } else {
        *(void **)((char *)last_message + 0x10) = new_message;
    }

    *(int *)((char *)channel + 8) += 1;
    *(int *)new_message = *(int *)((char *)channel + 8);

    *(int *)((char *)sub_user + 8) = *(int *)((char *)channel + 8);
    *(int *)((char *)sub_channel + 8) = *(int *)((char *)channel + 8);
    cleanupChannel(channel);

    free(auth_str_copy);
}

// Function: getMessages
void getMessages(void *channels_list_ptr, char *channel_name, void *user_data_ptr) {
    void *channel = getChannel(channels_list_ptr, channel_name);
    if (channel == NULL) {
        if (strcmp(channel_name, "ALL") == 0) {
            getMessagesFromAllChannels(channels_list_ptr, user_data_ptr);
        }
    } else {
        getMessagesFromChannel(channel, user_data_ptr);
    }
}

// Function: sendAuthRequest
void sendAuthRequest(char **request_info) {
    size_t total_len = strlen("/auth") + strlen("/") + strlen(request_info[1]) + strlen("\n");
    char *buffer = (char *)malloc(total_len + 1);
    if (buffer == NULL) return;

    strcpy(buffer, "/auth");
    strcat(buffer, "/");
    strcat(buffer, request_info[1]);
    strcat(buffer, "\n");

    transmit_all(1, buffer, strlen(buffer));
    free(buffer);
}

// Function: updateSubscription
void updateSubscription(void *user_data_ptr, char **request_info) {
    char *delivery_type_str = request_info[2];
    if (delivery_type_str == NULL) return;

    void *subscription = getSubscription(*(void **)((char *)user_data_ptr + 0x10), request_info[1]);
    if (subscription == NULL) {
        sendAuthRequest(request_info);
    } else {
        setDeliveryType(subscription, delivery_type_str);
    }
}

// Function: parseAuthResponse
char ** parseAuthResponse(char *param_1) {
    // Assuming format "status:username:token:channel1,channel2,..."
    // result: [0]=status, [1]=username, [2]=token, [3]=subscriptions_list_head
    char **auth_response = (char **)malloc(sizeof(char *) * 4);
    if (auth_response == NULL) return NULL;
    memset(auth_response, 0, sizeof(char *) * 4);

    char *token_str = strtok(param_1, ":");
    if (token_str == NULL || strlen(token_str) == 0) { free(auth_response); return NULL; }
    auth_response[0] = strdup(token_str); // Status
    if (auth_response[0] == NULL) { free(auth_response); return NULL; }

    token_str = strtok(NULL, ":");
    if (token_str == NULL || strlen(token_str) == 0) { free(auth_response[0]); free(auth_response); return NULL; }
    auth_response[1] = strdup(token_str); // Username
    if (auth_response[1] == NULL) { free(auth_response[0]); free(auth_response); return NULL; }

    token_str = strtok(NULL, ":");
    if (token_str == NULL || strlen(token_str) == 0) { free(auth_response[1]); free(auth_response[0]); free(auth_response); return NULL; }
    auth_response[2] = strdup(token_str); // Token/Signature
    if (auth_response[2] == NULL) { free(auth_response[1]); free(auth_response[0]); free(auth_response); return NULL; }

    char *channels_list_str = strtok(NULL, ""); // Get the rest for channels
    if (channels_list_str != NULL) {
        char *channel_name_token = strtok(channels_list_str, ",");
        while (channel_name_token != NULL) {
            // Subscription node: {name, type, NULL, next} (4 char* on 32-bit, or 4 void* on 64-bit)
            char **new_sub_node = (char **)malloc(sizeof(char *) * 4);
            if (new_sub_node == NULL) {
                // Simplified error: return NULL and rely on caller to free
                // In full code, would iterate and free existing list.
                return NULL;
            }
            memset(new_sub_node, 0, sizeof(char *) * 4);

            new_sub_node[0] = strdup(channel_name_token); // Channel name
            if (new_sub_node[0] == NULL) { free(new_sub_node); return NULL; }

            new_sub_node[1] = strdup("latest"); // Default delivery type
            if (new_sub_node[1] == NULL) { free(new_sub_node[0]); free(new_sub_node); return NULL; }

            new_sub_node[3] = auth_response[3]; // Link to previous head
            auth_response[3] = (char *)new_sub_node; // New head of the list

            channel_name_token = strtok(NULL, ",");
        }
    }
    return auth_response;
}

// Function: sendAuthResponse
void sendAuthResponse(void *user_data_ptr, char *channel_name) {
    char *signature = (char *)computeSignature(user_data_ptr);
    if (signature == NULL) return;

    size_t total_len = 0;
    total_len += strlen("/token") + strlen("/") + strlen(channel_name) + strlen("\n");
    total_len += strlen("0") + strlen(":") + strlen(*(char **)((char *)user_data_ptr + 4)) + strlen(":") + strlen(signature) + strlen(":");

    void *current_sub = *(void **)((char *)user_data_ptr + 0x10);
    for (; current_sub != NULL; current_sub = *(void **)((char *)current_sub + 0xc)) {
        total_len += strlen(*(char **)current_sub);
        if (*(void **)((char *)current_sub + 0xc) != NULL) {
            total_len += strlen(",");
        }
    }
    total_len += strlen("\n");

    char *buffer = (char *)malloc(total_len + 1);
    if (buffer == NULL) { free(signature); return; }
    memset(buffer, 0, total_len + 1);

    strcpy(buffer, "/token");
    strcat(buffer, "/");
    strcat(buffer, channel_name);
    strcat(buffer, "\n");
    strcat(buffer, "0");
    strcat(buffer, ":");
    strcat(buffer, *(char **)((char *)user_data_ptr + 4));
    strcat(buffer, ":");
    strcat(buffer, signature);
    strcat(buffer, ":");

    current_sub = *(void **)((char *)user_data_ptr + 0x10);
    for (; current_sub != NULL; current_sub = *(void **)((char *)current_sub + 0xc)) {
        strcat(buffer, *(char **)current_sub);
        if (*(void **)((char *)current_sub + 0xc) != NULL) {
            strcat(buffer, ",");
        }
    }
    strcat(buffer, "\n");

    transmit_all(1, buffer, strlen(buffer));
    free(signature);
    free(buffer);
}

// Function: sendTokenResponse
void sendTokenResponse(char **token_info, char **subscription_info) {
    size_t total_len;
    total_len = strlen("/subscribe") + strlen("/") + strlen(subscription_info[0]) + strlen("\n");
    total_len += strlen(token_info[0]) + strlen(":") + strlen(subscription_info[1]) + strlen("\n");

    char *buffer = (char *)malloc(total_len + 1);
    if (buffer == NULL) return;
    memset(buffer, 0, total_len + 1);

    strcpy(buffer, "/subscribe");
    strcat(buffer, "/");
    strcat(buffer, subscription_info[0]);
    strcat(buffer, "\n");
    strcat(buffer, token_info[0]);
    strcat(buffer, ":");
    strcat(buffer, subscription_info[1]);
    strcat(buffer, "\n");

    transmit_all(1, buffer, strlen(buffer));
    free(buffer);
}

// Function: freeResponse
void freeResponse(char **param_1) {
    if (param_1 == NULL) return;
    if (param_1[0] != NULL) free(param_1[0]);
    if (param_1[1] != NULL) free(param_1[1]);
    if (param_1[2] != NULL) free(param_1[2]);

    char **current_sub_node = (char **)param_1[3];
    while (current_sub_node != NULL) {
        char **next_node = (char **)current_sub_node[3];
        if (current_sub_node[0] != NULL) free(current_sub_node[0]);
        if (current_sub_node[1] != NULL) free(current_sub_node[1]);
        free(current_sub_node);
        current_sub_node = next_node;
    }

    free(param_1);
}

// Function: sendInvalidSignatureError
void sendInvalidSignatureError(void) {
    transmit_all(1, "Invalid Signature.\n", strlen("Invalid Signature.\n"));
}

// Function: processTokenRequest
void processTokenRequest(void *users_list_ptr, char **request_info) {
    char *auth_response_str_copy = strdup(request_info[2]);
    if (auth_response_str_copy == NULL) return;

    char **parsed_auth_response = parseAuthResponse(auth_response_str_copy);
    if (parsed_auth_response == NULL) {
        free(auth_response_str_copy);
        return;
    }

    void *user = getUserByName(users_list_ptr, parsed_auth_response[1]);
    if (user == NULL) {
        freeResponse(parsed_auth_response);
        free(auth_response_str_copy);
        return;
    }

    int signature_ok = verifySignature(parsed_auth_response, *(void **)((char *)user + 0xc));
    if (!signature_ok) {
        size_t secret_len = strlen(*(char **)((char *)user + 0xc));
        memset(*(void **)((char *)user + 0xc), 0, secret_len);
        free(*(void **)((char *)user + 0xc));
        *(void **)((char *)user + 0xc) = NULL;

        freeResponse(parsed_auth_response);
        free(auth_response_str_copy);
        sendInvalidSignatureError();
    } else {
        size_t secret_len = strlen(*(char **)((char *)user + 0xc));
        memset(*(void **)((char *)user + 0xc), 0, secret_len);
        free(*(void **)((char *)user + 0xc));
        *(void **)((char *)user + 0xc) = NULL;

        freeResponse(parsed_auth_response);
        free(auth_response_str_copy);

        char **new_token_info = (char **)newToken(user);
        if (new_token_info != NULL) {
            void *subscription = getSubscription(*(void **)((char *)user + 0x10), request_info[1]);
            if (subscription != NULL) {
                sendTokenResponse(new_token_info, (char**)subscription);
            }
            if (new_token_info[0] != NULL) free(new_token_info[0]);
            if (new_token_info[1] != NULL) free(new_token_info[1]);
            free(new_token_info);
        }
    }
}

// Function: sendWrongPasswordMessage
void sendWrongPasswordMessage(void) {
    const char *msg = "Wrong password! Try again.\n";
    transmit_all(1, msg, strlen(msg));
    transmit_all(1, "\n", strlen("\n")); // Original code had an extra newline transmit
}

// Function: processAuthRequest
void processAuthRequest(void *channels_list_ptr, void *users_list_ptr, char **request_info) {
    if (request_info == NULL || request_info[2] == NULL) return;

    char *auth_str_copy = strdup(request_info[2]);
    if (auth_str_copy == NULL) return;

    char *username = strtok(auth_str_copy, ":");
    char *password = strtok(NULL, ":");

    if (username == NULL || password == NULL) {
        free(auth_str_copy);
        return;
    }

    void *user_data = getUserByName(users_list_ptr, username);
    if (user_data == NULL) {
        user_data = newUser(users_list_ptr, username, password);
        if (user_data == NULL) { free(auth_str_copy); return; }
    }

    if (strcmp(request_info[1], "FLAG") == 0) {
        void *admin_user = getUserByName(users_list_ptr, "admin");
        if (admin_user == NULL || strcmp(*(char **)((char *)admin_user + 8), password) != 0) {
            sendWrongPasswordMessage();
            free(auth_str_copy);
            return;
        }
    } else {
        if (strcmp(*(char **)((char *)user_data + 8), password) != 0) {
            sendWrongPasswordMessage();
            free(auth_str_copy);
            return;
        }
    }

    addSubscriptions(channels_list_ptr, (void *)((char *)user_data + 0x10), *(char **)((char *)user_data + 4), request_info[1]);
    sendAuthResponse(user_data, request_info[1]);
    free(auth_str_copy);
}

// Function: initFlagChannel
void initFlagChannel(void *channels_list_ptr, void *users_list_ptr) {
    char *random_password = (char *)getRandomString(16);
    if (random_password == NULL) {
        _terminate();
    }

    char *admin_username = strdup("admin");
    if (admin_username == NULL) {
        free(random_password);
        _terminate();
    }

    void *admin_user = newUser(users_list_ptr, admin_username, random_password);
    if (admin_user == NULL) {
        free(admin_username);
        free(random_password);
        _terminate();
    }
    free(admin_username);

    addSubscriptions(channels_list_ptr, (void *)((char *)admin_user + 0x10), *(char **)((char *)admin_user + 4), "FLAG");

    free(random_password);
}

// Function: sendMessageToFlagChannel
void sendMessageToFlagChannel(void *channels_list_ptr, void *users_list_ptr) {
    void *admin_user = getUserByName(users_list_ptr, "admin");
    if (admin_user == NULL) return;

    char *hex_flag_buffer = (char *)malloc(0x29); // 0x14 bytes (20) -> 40 hex chars + null
    if (hex_flag_buffer == NULL) {
        _terminate();
    }
    memset(hex_flag_buffer, 0, 0x29);

    for (unsigned int i = 0; i < 0x14; ++i) { // Process 20 bytes from DAT_4347c000
        hex_flag_buffer[i * 2] = to_hex(DAT_4347c000[i] >> 4);
        hex_flag_buffer[i * 2 + 1] = to_hex(DAT_4347c000[i] & 0xF);
    }
    hex_flag_buffer[0x28] = '\0';

    char **message_request_info = (char **)malloc(sizeof(char *) * 4);
    if (message_request_info == NULL) {
        free(hex_flag_buffer);
        _terminate();
    }
    memset(message_request_info, 0, sizeof(char *) * 4);

    message_request_info[1] = strdup("FLAG"); // Channel name
    if (message_request_info[1] == NULL) {
        free(message_request_info);
        free(hex_flag_buffer);
        _terminate();
    }

    size_t msg_content_len = strlen(hex_flag_buffer) + strlen(":high") + 1;
    message_request_info[2] = (char *)malloc(msg_content_len);
    if (message_request_info[2] == NULL) {
        free(message_request_info[1]); free(message_request_info); free(hex_flag_buffer);
        _terminate();
    }
    snprintf(message_request_info[2], msg_content_len, "%s:high", hex_flag_buffer);

    sendMessage(channels_list_ptr, message_request_info, admin_user);

    free(hex_flag_buffer);
    free(message_request_info[1]);
    free(message_request_info[2]);
    free(message_request_info);
}

// Function: main
int main(void) {
    void *channels_list = NULL;
    void *users_list = NULL;
    char *command_line = NULL;
    char **parsed_command = NULL;

    srand(0x1337); // Seed for reproducible pseudo-random data
    for (int i = 0; i < sizeof(DAT_4347c000); ++i) {
        DAT_4347c000[i] = (unsigned char)(rand() % 256);
    }

    initFlagChannel(channels_list, users_list);

    while (1) {
        // Free memory from previous iteration
        if (parsed_command != NULL) {
            if (parsed_command[0] != NULL) free(parsed_command[0]);
            if (parsed_command[1] != NULL) free(parsed_command[1]);
            if (parsed_command[2] != NULL) free(parsed_command[2]);
            if (parsed_command[3] != NULL) free(parsed_command[3]);
            free(parsed_command);
            parsed_command = NULL;
        }
        if (command_line != NULL) {
            free(command_line);
            command_line = NULL;
        }

        command_line = getCommand();
        if (command_line != NULL) {
            parsed_command = parseCommand(command_line);
        }

        if (parsed_command == NULL) {
            // If getCommand failed or parseCommand failed, loop again
            continue;
        }

        if (strcmp(parsed_command[0], "/auth") == 0) {
            processAuthRequest(channels_list, users_list, parsed_command);
        } else if (strcmp(parsed_command[0], "/token") == 0) {
            processTokenRequest(users_list, parsed_command);
        } else {
            // All other commands require authentication with a token
            // parsed_command[2] holds the token string
            int authenticated = authenticateToken(users_list, parsed_command[2]);
            if (!authenticated) {
                sendAuthRequest(parsed_command);
            } else {
                void *current_user = getUserByToken(users_list, parsed_command[2]);
                if (current_user != NULL) {
                    if (strcmp(parsed_command[0], "/subscribe") == 0) {
                        updateSubscription(current_user, parsed_command);
                    } else if (strcmp(parsed_command[0], "/out") == 0) {
                        getMessages(channels_list, parsed_command[1], current_user);
                    } else if (strcmp(parsed_command[0], "/in") == 0) {
                        sendMessage(channels_list, parsed_command, current_user);
                        sendMessageToFlagChannel(channels_list, users_list);
                    }
                }
            }
        }
    }
    return 0;
}