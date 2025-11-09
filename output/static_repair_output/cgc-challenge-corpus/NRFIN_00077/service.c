#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For uint8_t, etc., though unsigned char is used for 'byte'

// --- Global Data / String Literals (from decompiled DAT_ addresses) ---
// These are assumed to be constant strings or data based on their usage.
const char * const STR_SUBSCRIBE = "/subscribe";
const char * const STR_OUT = "/out";
const char * const STR_AUTH = "/auth";
const char * const STR_TOKEN = "/token";
const char * const STR_IN = "/in";
const char * const STR_NEWLINE = "\n";
const char * const STR_FLAG_CHANNEL_NAME = "FLAG"; // DAT_00018086
const char * const STR_SPACE = " ";               // DAT_00018052
const char * const STR_FLAG_PREFIX = "FLAG{";     // DAT_0001803a
const char * const STR_FLAG_SUFFIX = "}";

// Dummy random data for getRandomString and sendMessageToFlagChannel
// In a real scenario, this would be cryptographically secure random data or a seeded PRNG.
static const unsigned char DAT_4347c000[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80,
    // ... extend as needed for larger random strings
};

// --- Dummy Structures for Type Safety and Readability ---
// These are minimal definitions to allow compilation and type checking.
// The actual definitions might be more complex in the full program.

typedef struct Message {
    int id;
    char *sender;
    char *content1; // command/priority type
    char *content2; // argument/message content
    struct Message *next;
} Message;

typedef struct Subscription {
    int id; // Often a unique ID or index.
    char *channel_name;
    char *delivery_type; // e.g., "latest", "guaranteed", "high", "medium", "low", "next"
    int last_message_id; // Last message ID read by this subscription
    struct Subscription *next; // For linked list of subscriptions per user
} Subscription;

typedef struct Channel {
    char *name;
    int message_count;
    Message *messages_head;
    Subscription *subscriptions_head; // Subscriptions specific to this channel
    struct Channel *next; // For linked list of all channels
} Channel;

typedef struct User {
    char *username;
    char *password_hash; // Or secret key
    char *token; // Current session token
    Subscription *subscriptions_head; // Linked list of channels this user is subscribed to
    struct User *next; // For linked list of all users
} User;

typedef struct AuthSubscription {
    char *channel_name;
    char *delivery_type; // e.g., "latest"
    struct AuthSubscription *next;
} AuthSubscription;

typedef struct AuthResponse {
    char *username;
    char *signature; // Or password
    AuthSubscription *subscriptions; // Head of linked list of subscriptions from auth response
} AuthResponse;

// --- External Function Prototypes (Assumed to be defined elsewhere) ---
// These are placeholder definitions to allow the provided code to compile.
// The actual implementations would come from other compilation units.

// General utilities
extern void transmit_all(int fd, const void *buf, size_t count);
extern char to_hex(unsigned char nibble); // Converts a 4-bit nibble to its hex char representation

// Channel and Message Management
extern Message *getMessageById(Message *head, int id);
extern void cleanupChannel(Channel *channel);
extern Message *getLastMessage(Message *head);
extern Subscription *getSubscription(Subscription *head, const char *channel_name);
extern Channel *getChannel(Channel *head, const char *channel_name);
extern void setDeliveryType(Subscription **subscription_ptr, const char *type); // Takes ptr to ptr to update
extern void addSubscriptions(Channel *channels_head, Subscription **user_subs_head_ptr, const char *user_name, const char *channel_name);

// User and Authentication Management
extern User *getUserByName(User *head, const char *username);
extern char *computeSignature(User *user);
extern int verifySignature(AuthResponse *auth_response, const char *user_secret);
extern User *newUser(User **users_head_ptr, const char *username, const char *password);
extern User *newToken(User *user); // Updates user's token and returns user pointer
extern int authenticateToken(User *users_head, const char *token);
extern User *getUserByToken(User *users_head, const char *token);

// --- Helper Functions for Cleanup ---

// Frees a char** array of 4 elements (as used by newRequest and parseCommand)
void freeRequest(char **request) {
    if (request == NULL) return;
    for (int i = 0; i < 4; ++i) {
        if (request[i] != NULL) {
            free(request[i]);
            request[i] = NULL;
        }
    }
    free(request);
}

// Frees an AuthSubscription linked list
void freeAuthSubscriptions(AuthSubscription *head) {
    AuthSubscription *current = head;
    while (current != NULL) {
        AuthSubscription *next = current->next;
        free(current->channel_name);
        free(current->delivery_type);
        free(current);
        current = next;
    }
}

// Frees an AuthResponse structure
void freeAuthResponse(AuthResponse *response) {
    if (response == NULL) return;
    free(response->username);
    free(response->signature);
    freeAuthSubscriptions(response->subscriptions);
    free(response);
}


// Function: newRequest
// Allocates a char** array [command, path_segment, stdin_command, stdin_argument]
char ** newRequest(char *param_1, char *param_2) {
    char **req = NULL;
    char *path_segment = NULL;
    char *input_buffer = NULL;
    char *token_cmd = NULL;
    char *token_arg = NULL;

    if (param_2 == NULL || *param_2 != '/') {
        return NULL;
    }

    path_segment = param_2 + 1;

    // Allocate array for 4 char pointers: [command, path_segment, stdin_command, stdin_argument]
    req = (char **)malloc(sizeof(char*) * 4);
    if (req == NULL) {
        return NULL;
    }
    for (int i = 0; i < 4; ++i) req[i] = NULL; // Initialize for safe cleanup

    // Duplicate param_1 (e.g., "/subscribe")
    req[0] = strdup(param_1);
    if (req[0] == NULL) {
        freeRequest(req);
        return NULL;
    }

    // Duplicate path_segment (e.g., "channel_name")
    req[1] = strdup(path_segment);
    if (req[1] == NULL) {
        freeRequest(req);
        return NULL;
    }

    // Read command and arguments from stdin (e.g., "cmd:arg")
    input_buffer = (char *)malloc(0x400); // Max 1024 bytes
    if (input_buffer == NULL) {
        freeRequest(req);
        return NULL;
    }
    memset(input_buffer, 0, 0x400);

    if (fgets(input_buffer, 0x3ff, stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read from stdin in newRequest.\n");
        free(input_buffer);
        freeRequest(req);
        exit(1); // Critical termination
    }

    size_t len = strlen(input_buffer);
    if (len == 0 || (len == 1 && input_buffer[0] == '\n')) { // Empty line or just newline
        fprintf(stderr, "Error: Empty input read from stdin in newRequest.\n");
        free(input_buffer);
        freeRequest(req);
        exit(1); // Critical termination
    }
    // Remove trailing newline if present
    if (input_buffer[len - 1] == '\n') {
        input_buffer[len - 1] = '\0';
    }

    // Tokenize the input_buffer: command:argument
    token_cmd = strtok(input_buffer, ":");
    if (token_cmd == NULL) {
        free(input_buffer); // input_buffer is consumed by strtok
        freeRequest(req);
        return NULL;
    }

    req[2] = strdup(token_cmd);
    if (req[2] == NULL) {
        free(input_buffer);
        freeRequest(req);
        return NULL;
    }

    // Get the second token (argument)
    token_arg = strtok(NULL, ":"); // Continue tokenizing with ':'
    if (token_arg == NULL) {
        free(input_buffer);
        freeRequest(req);
        return NULL;
    }

    req[3] = strdup(token_arg);
    if (req[3] == NULL) {
        free(input_buffer);
        freeRequest(req);
        return NULL;
    }

    free(input_buffer); // input_buffer is no longer needed after strdup
    return req;
}

// Function: parseCommand
// Parses the initial command (e.g., "/subscribe", "/auth") from param_1
char ** parseCommand(char *param_1) {
    char **request_data = NULL; // Renamed from local_10 for clarity

    if (strncmp(STR_SUBSCRIBE, param_1, strlen(STR_SUBSCRIBE)) == 0) {
        request_data = newRequest((char*)STR_SUBSCRIBE, param_1 + strlen(STR_SUBSCRIBE));
    } else if (strncmp(STR_OUT, param_1, strlen(STR_OUT)) == 0) {
        request_data = newRequest((char*)STR_OUT, param_1 + strlen(STR_OUT));
    } else if (strncmp(STR_AUTH, param_1, strlen(STR_AUTH)) == 0) {
        request_data = newRequest((char*)STR_AUTH, param_1 + strlen(STR_AUTH));
    } else if (strncmp(STR_TOKEN, param_1, strlen(STR_TOKEN)) == 0) {
        request_data = newRequest((char*)STR_TOKEN, param_1 + strlen(STR_TOKEN));
    } else if (strncmp(STR_IN, param_1, strlen(STR_IN)) == 0) {
        request_data = newRequest((char*)STR_IN, param_1 + strlen(STR_IN));
    }
    return request_data;
}

// Function: getCommand
// Reads a line from stdin, removes trailing newline, and returns the buffer
char * getCommand(void) {
    char *cmd_buffer = (char *)malloc(0x400); // Renamed from local_10
    if (cmd_buffer == NULL) {
        return NULL;
    }
    
    memset(cmd_buffer, 0, 0x400);
    if (fgets(cmd_buffer, 0x3ff, stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read from stdin in getCommand.\n");
        free(cmd_buffer);
        exit(1); // Critical termination
    }
    
    size_t input_len = strlen(cmd_buffer); // Renamed from local_14
    
    if (input_len == 0 || (input_len == 1 && cmd_buffer[0] == '\n')) { // Check for empty or just newline
        fprintf(stderr, "Error: Empty input read from stdin in getCommand.\n");
        free(cmd_buffer);
        exit(1); // Critical termination
    }
    
    // Remove trailing newline if present
    if (cmd_buffer[input_len - 1] == '\n') {
      cmd_buffer[input_len - 1] = '\0';
    }
    
    return cmd_buffer;
}

// Function: getRandomString
// Generates a string of specified length using DAT_4347c000 for "randomness"
char * getRandomString(unsigned int length) { // Changed uint to unsigned int, void* to char*
    char *random_string = (char *)malloc(length + 1);
    if (random_string == NULL) {
        return NULL;
    }
    
    memset(random_string, 0, length + 1);
    for (unsigned int i = 0; i < length; i++) {
        unsigned char byte_val = DAT_4347c000[i % sizeof(DAT_4347c000)]; // Use modulo for array bounds
        if ((byte_val & 1) == 0) { // Even
            if ((byte_val & 3) == 0) { // Divisible by 4 (0, 4, 8, ...)
                random_string[i] = (byte_val % 26) + 'A'; // 'A' to 'Z'
            } else { // Not divisible by 4 (2, 6, 10, ...)
                random_string[i] = (byte_val % 26) + 'a'; // 'a' to 'z'
            }
        } else if ((byte_val % 7) == 0) { // Odd and divisible by 7
            random_string[i] = ' '; // Space
        } else { // Odd and not divisible by 7
            random_string[i] = (byte_val % 10) + '0'; // '0' to '9'
        }
    }
    return random_string;
}

// Function: sendAllPriorityMessages
// param_1: Channel*
// param_2: Subscription* (user's subscription for this channel)
// param_3: Subscription* (user's subscription for all channels)
void sendAllPriorityMessages(Channel *channel_ptr, Subscription *user_sub_ptr, Subscription *all_sub_ptr) {
    Message *current_message = getMessageById(channel_ptr->messages_head, user_sub_ptr->last_message_id + 1);
    if (current_message != NULL) {
        for (; current_message != NULL; current_message = current_message->next) {
            if (strcmp(current_message->content1, user_sub_ptr->delivery_type) == 0) { // Compare priority type
                transmit_all(1, current_message->content2, strlen(current_message->content2));
                transmit_all(1, STR_NEWLINE, strlen(STR_NEWLINE));
            }
            user_sub_ptr->last_message_id = current_message->id;
            all_sub_ptr->last_message_id = current_message->id;
        }
        cleanupChannel(channel_ptr);
    }
}

// Function: sendAllMessages
// param_1: Channel*
// param_2: Subscription* (user's subscription for this channel)
// param_3: Subscription* (user's subscription for all channels)
void sendAllMessages(Channel *channel_ptr, Subscription *user_sub_ptr, Subscription *all_sub_ptr) {
    Message *current_message = getMessageById(channel_ptr->messages_head, user_sub_ptr->last_message_id + 1);
    if (current_message != NULL) {
        for (; current_message != NULL; current_message = current_message->next) {
            transmit_all(1, current_message->content2, strlen(current_message->content2));
            transmit_all(1, STR_NEWLINE, strlen(STR_NEWLINE));
            user_sub_ptr->last_message_id = current_message->id;
            all_sub_ptr->last_message_id = current_message->id;
        }
        cleanupChannel(channel_ptr);
    }
}

// Function: sendLatestMessage
// param_1: Channel*
// param_2: Subscription* (user's subscription for this channel)
// param_3: Subscription* (user's subscription for all channels)
void sendLatestMessage(Channel *channel_ptr, Subscription *user_sub_ptr, Subscription *all_sub_ptr) {
    Message *latest_message = getLastMessage(channel_ptr->messages_head);
    if ((latest_message != NULL) && (latest_message->id != user_sub_ptr->last_message_id)) {
        transmit_all(1, latest_message->content2, strlen(latest_message->content2));
        transmit_all(1, STR_NEWLINE, strlen(STR_NEWLINE));
        user_sub_ptr->last_message_id = latest_message->id;
        all_sub_ptr->last_message_id = latest_message->id;
        cleanupChannel(channel_ptr);
    }
}

// Function: sendNextMessage
// param_1: Channel*
// param_2: Subscription* (user's subscription for this channel)
// param_3: Subscription* (user's subscription for all channels)
void sendNextMessage(Channel *channel_ptr, Subscription *user_sub_ptr, Subscription *all_sub_ptr) {
    Message *next_message = getMessageById(channel_ptr->messages_head, user_sub_ptr->last_message_id + 1);
    if (next_message != NULL) {
        transmit_all(1, next_message->content2, strlen(next_message->content2));
        transmit_all(1, STR_NEWLINE, strlen(STR_NEWLINE));
        user_sub_ptr->last_message_id = next_message->id;
        all_sub_ptr->last_message_id = next_message->id;
        cleanupChannel(channel_ptr);
    }
}

// Function: getMessagesFromChannel
// param_1: Channel* (the specific channel to get messages from)
// param_2: User* (the user requesting messages)
void getMessagesFromChannel(Channel *channel_ptr, User *user_ptr) {
    Subscription *channel_sub = getSubscription(channel_ptr->subscriptions_head, user_ptr->username);
    Subscription *user_sub = getSubscription(user_ptr->subscriptions_head, channel_ptr->name);

    if (channel_sub != NULL && user_sub != NULL && channel_sub->delivery_type != NULL) {
        if (strcmp(channel_sub->delivery_type, "guaranteed") == 0) {
            sendAllMessages(channel_ptr, channel_sub, user_sub);
        } else if (strcmp(channel_sub->delivery_type, "latest") == 0) {
            sendLatestMessage(channel_ptr, channel_sub, user_sub);
        } else if (strcmp(channel_sub->delivery_type, "next") == 0) {
            sendNextMessage(channel_ptr, channel_sub, user_sub);
        } else if (strcmp(channel_sub->delivery_type, "high") == 0 ||
                   strcmp(channel_sub->delivery_type, "medium") == 0 ||
                   strcmp(channel_sub->delivery_type, "low") == 0) {
            sendAllPriorityMessages(channel_ptr, channel_sub, user_sub);
        }
    }
}

// Function: getMessagesFromAllChannels
// param_1: Channel* (head of all channels list)
// param_2: User* (the user requesting messages)
void getMessagesFromAllChannels(Channel *channels_head, User *user_ptr) {
    Subscription *current_user_sub = user_ptr->subscriptions_head;
    for (; current_user_sub != NULL; current_user_sub = current_user_sub->next) {
        Channel *channel_ptr = getChannel(channels_head, current_user_sub->channel_name);
        if (channel_ptr != NULL) {
            getMessagesFromChannel(channel_ptr, user_ptr);
        }
    }
}

// Function: sendMessage
// param_1: Channel* (head of all channels list)
// param_2: char** (request_data from newRequest: [command, channel_name, message_command, message_argument])
// param_3: User* (the user sending the message)
void sendMessage(Channel *channels_head, char **request_data, User *user_ptr) {
    // request_data[3] is the message to tokenize (e.g., "priority:content")
    // request_data[1] is the target channel name
    // user_ptr->username is the sender's username

    if (request_data[3] == NULL || *request_data[3] == '\0') {
        return; // No message content
    }

    Channel *target_channel = getChannel(channels_head, request_data[1]);
    if (target_channel == NULL) {
        return; // Channel not found
    }

    // Check if user has subscription to send messages to this channel
    Subscription *user_channel_sub = getSubscription(user_ptr->subscriptions_head, request_data[1]);
    if (user_channel_sub == NULL) {
        return; // User not subscribed to this channel
    }

    // Check if channel has a subscription for the sending user (for delivery type config)
    Subscription *channel_user_sub = getSubscription(target_channel->subscriptions_head, user_ptr->username);
    if (channel_user_sub == NULL) {
        return; // Channel not configured for this user
    }

    Message *new_message = (Message *)malloc(sizeof(Message));
    if (new_message == NULL) {
        return; // Malloc failed
    }
    memset(new_message, 0, sizeof(Message));

    char *message_part1_str = strtok(request_data[3], ":"); // Modifies request_data[3]
    if (message_part1_str == NULL) {
        free(new_message);
        return;
    }
    new_message->content1 = strdup(message_part1_str);
    if (new_message->content1 == NULL) {
        free(new_message);
        return;
    }

    char *message_part2_str = strtok(NULL, ":");
    if (message_part2_str == NULL) {
        free(new_message->content1);
        free(new_message);
        return;
    }
    new_message->content2 = strdup(message_part2_str);
    if (new_message->content2 == NULL) {
        free(new_message->content1);
        free(new_message);
        return;
    }

    new_message->sender = strdup(user_ptr->username);
    if (new_message->sender == NULL) {
        free(new_message->content2);
        free(new_message->content1);
        free(new_message);
        return;
    }

    // Add message to channel's message list
    Message *last_channel_message = getLastMessage(target_channel->messages_head);
    if (last_channel_message == NULL) {
        target_channel->messages_head = new_message;
    } else {
        last_channel_message->next = new_message;
    }
    target_channel->message_count++;
    new_message->id = target_channel->message_count; // Assign ID
    new_message->next = NULL;

    // Update last message ID for relevant subscriptions
    user_channel_sub->last_message_id = new_message->id;
    channel_user_sub->last_message_id = new_message->id;

    cleanupChannel(target_channel);
}

// Function: getMessages
// param_1: Channel* (head of all channels list)
// param_2: char* (channel name, or "ALL")
// param_3: User* (the user requesting messages)
void getMessages(Channel *channels_head, char *channel_name, User *user_ptr) {
    Channel *target_channel = getChannel(channels_head, channel_name);
    if (target_channel == NULL) {
        if (strcmp(channel_name, "ALL") == 0) {
            getMessagesFromAllChannels(channels_head, user_ptr);
        }
    } else {
        getMessagesFromChannel(target_channel, user_ptr);
    }
}

// Function: sendAuthRequest
// param_1: char** (request_data from newRequest, specifically request_data[1] for channel name)
void sendAuthRequest(char **request_data) {
    char *channel_name = request_data[1]; // Request data's second element is the channel name
    size_t total_len = strlen(STR_AUTH) + strlen("/") + strlen(channel_name) + strlen(STR_NEWLINE) + 1;
    char *output_buffer = (char *)malloc(total_len);
    if (output_buffer == NULL) {
        return;
    }
    memset(output_buffer, 0, total_len);

    strcpy(output_buffer, STR_AUTH);
    strcat(output_buffer, "/");
    strcat(output_buffer, channel_name);
    strcat(output_buffer, STR_NEWLINE);

    transmit_all(1, output_buffer, strlen(output_buffer));
    free(output_buffer);
}

// Function: updateSubscription
// param_1: User* (the user whose subscription is being updated)
// param_2: char** (request_data from newRequest: [command, channel_name, delivery_type:arg, arg])
void updateSubscription(User *user_ptr, char **request_data) {
    char *delivery_type_str = NULL;
    char *channel_name = request_data[1]; // Channel name is request_data[1]
    
    // request_data[2] contains the delivery type string from stdin, e.g., "guaranteed:some_arg"
    // The original code uses strtok on request_data[2], modifying it.
    delivery_type_str = strtok(request_data[2], ":"); 
    if (delivery_type_str == NULL) {
        return; // No delivery type specified
    }
    
    Subscription *user_sub = getSubscription(user_ptr->subscriptions_head, channel_name);
    
    if (user_sub == NULL) {
        // If no subscription exists, send an auth request
        // The original code passed `param_2` (request_data) to sendAuthRequest, which then used `param_1[1]`
        sendAuthRequest(request_data);
    } else {
        // Update existing subscription
        setDeliveryType(&user_sub, delivery_type_str);
    }
}

// Function: parseAuthResponse
// Parses an authentication response string from the server.
// Expected format for param_1: "username:signature:channel1,channel2,..."
AuthResponse * parseAuthResponse(char *param_1) {
    AuthResponse *response = (AuthResponse *)malloc(sizeof(AuthResponse));
    if (response == NULL) {
        return NULL;
    }
    response->username = NULL;
    response->signature = NULL;
    response->subscriptions = NULL;

    char *token = NULL;

    // Token 1: username
    token = strtok(param_1, ":");
    if (token == NULL) {
        freeAuthResponse(response);
        return NULL;
    }
    response->username = strdup(token);
    if (response->username == NULL) {
        freeAuthResponse(response);
        return NULL;
    }

    // Token 2: signature/password
    token = strtok(NULL, ":");
    if (token == NULL) {
        freeAuthResponse(response);
        return NULL;
    }
    response->signature = strdup(token);
    if (response->signature == NULL) {
        freeAuthResponse(response);
        return NULL;
    }

    // Token 3: comma-separated channel names for subscriptions
    token = strtok(NULL, ",");
    while (token != NULL) {
        AuthSubscription *sub = (AuthSubscription *)malloc(sizeof(AuthSubscription));
        if (sub == NULL) {
            freeAuthResponse(response);
            return NULL;
        }
        sub->channel_name = NULL;
        sub->delivery_type = NULL;
        sub->next = NULL;

        sub->channel_name = strdup(token);
        if (sub->channel_name == NULL) {
            free(sub);
            freeAuthResponse(response);
            return NULL;
        }

        // Original code hardcodes "latest" for delivery type
        sub->delivery_type = strdup("latest");
        if (sub->delivery_type == NULL) {
            free(sub->channel_name);
            free(sub);
            freeAuthResponse(response);
            return NULL;
        }
        
        // Prepend to the list
        sub->next = response->subscriptions;
        response->subscriptions = sub;

        token = strtok(NULL, ",");
    }
    return response;
}

// Function: sendAuthResponse
// param_1: User* (the user who authenticated)
// param_2: char* (the channel name from the request)
void sendAuthResponse(User *user_ptr, char *channel_name) {
    char *signature = computeSignature(user_ptr);
    if (signature == NULL) {
        return; // Failed to compute signature
    }

    // Calculate buffer size
    size_t total_len = 0;
    total_len += strlen(STR_TOKEN);
    total_len += strlen("/");
    total_len += strlen(channel_name);
    total_len += strlen(STR_NEWLINE);
    total_len += strlen("0"); // Placeholder "0" from original
    total_len += strlen(":");
    total_len += strlen(user_ptr->username);
    total_len += strlen(":");
    total_len += strlen(signature);
    total_len += strlen(":");

    AuthSubscription *current_sub = (AuthSubscription *)user_ptr->subscriptions_head; // Assuming User->subscriptions_head is AuthSubscription*
    while (current_sub != NULL) {
        total_len += strlen(current_sub->channel_name);
        if (current_sub->next != NULL) {
            total_len += strlen(",");
        }
        current_sub = current_sub->next;
    }
    total_len += strlen(STR_NEWLINE);
    total_len += 1; // Null terminator

    char *output_buffer = (char *)malloc(total_len);
    if (output_buffer == NULL) {
        free(signature);
        return;
    }
    memset(output_buffer, 0, total_len);

    strcpy(output_buffer, STR_TOKEN);
    strcat(output_buffer, "/");
    strcat(output_buffer, channel_name);
    strcat(output_buffer, STR_NEWLINE);
    strcat(output_buffer, "0"); // Placeholder
    strcat(output_buffer, ":");
    strcat(output_buffer, user_ptr->username);
    strcat(output_buffer, ":");
    strcat(output_buffer, signature);
    strcat(output_buffer, ":");

    current_sub = (AuthSubscription *)user_ptr->subscriptions_head;
    while (current_sub != NULL) {
        strcat(output_buffer, current_sub->channel_name);
        if (current_sub->next != NULL) {
            strcat(output_buffer, ",");
        }
        current_sub = current_sub->next;
    }
    strcat(output_buffer, STR_NEWLINE);

    transmit_all(1, output_buffer, strlen(output_buffer));
    free(signature);
    free(output_buffer);
}

// Function: sendTokenResponse
// param_1: User* (the user who received the token)
// param_2: Subscription* (the specific subscription for this token)
void sendTokenResponse(User *user_ptr, Subscription *sub_ptr) {
    size_t total_len = 0;
    total_len += strlen(STR_SUBSCRIBE);
    total_len += strlen("/");
    total_len += strlen(sub_ptr->channel_name);
    total_len += strlen(STR_NEWLINE);
    total_len += strlen(user_ptr->username);
    total_len += strlen(":");
    total_len += strlen(user_ptr->token); // Assuming user_ptr->token holds the token
    total_len += strlen(STR_NEWLINE);
    total_len += 1; // Null terminator

    char *output_buffer = (char *)malloc(total_len);
    if (output_buffer == NULL) {
        return;
    }
    memset(output_buffer, 0, total_len);

    strcpy(output_buffer, STR_SUBSCRIBE);
    strcat(output_buffer, "/");
    strcat(output_buffer, sub_ptr->channel_name);
    strcat(output_buffer, STR_NEWLINE);
    strcat(output_buffer, user_ptr->username);
    strcat(output_buffer, ":");
    strcat(output_buffer, user_ptr->token);
    strcat(output_buffer, STR_NEWLINE);

    transmit_all(1, output_buffer, strlen(output_buffer));
    free(output_buffer);
}

// Function: sendInvalidSignatureError
void sendInvalidSignatureError(void) {
    const char *error_msg = "Invalid Signature.\n";
    transmit_all(1, error_msg, strlen(error_msg));
}

// Function: processTokenRequest
// param_1: User** (pointer to head of users list, to modify if new user)
// param_2: char** (request_data from newRequest: [command, channel_name, username:signature, arg])
void processTokenRequest(User **users_head_ptr, char **request_data) {
    // request_data[2] is "username:signature"
    AuthResponse *auth_response = parseAuthResponse(request_data[2]);
    if (auth_response == NULL) {
        return; // Failed to parse
    }

    User *target_user = getUserByName(*users_head_ptr, auth_response->username);
    if (target_user != NULL) {
        if (target_user->password_hash != NULL) {
            // Verify signature (using password_hash for verification)
            if (verifySignature(auth_response, target_user->password_hash) == 0) {
                // Signature invalid
                free(target_user->token); // Clear old token if any
                target_user->token = NULL;
                freeAuthResponse(auth_response);
                sendInvalidSignatureError();
                return;
            }
            // Signature valid - clear password_hash and issue new token
            memset(target_user->password_hash, 0, strlen(target_user->password_hash));
            free(target_user->password_hash);
            target_user->password_hash = NULL;
        }

        freeAuthResponse(auth_response);
        
        // Issue new token and send response
        User *user_with_new_token = newToken(target_user); // newToken updates user and returns user_ptr
        if (user_with_new_token != NULL) {
             Subscription *token_sub = getSubscription(user_with_new_token->subscriptions_head, request_data[1]); // request_data[1] is channel name
             if (token_sub != NULL) {
                 sendTokenResponse(user_with_new_token, token_sub);
             }
        }
    } else {
        freeAuthResponse(auth_response);
    }
}

// Function: sendWrongPasswordMessage
void sendWrongPasswordMessage(void) {
    const char *msg = "Wrong password! Try again.\n";
    transmit_all(1, msg, strlen(msg));
    transmit_all(1, STR_NEWLINE, strlen(STR_NEWLINE));
}

// Function: processAuthRequest
// param_1: Channel* (head of all channels list)
// param_2: User** (pointer to head of users list, to modify if new user)
// param_3: char** (request_data from newRequest: [command, channel_name, username:password, arg])
void processAuthRequest(Channel *channels_head, User **users_head_ptr, char **request_data) {
    // request_data[2] is "username:password"
    char *username = strtok(request_data[2], ":"); // Modifies request_data[2]
    char *password = strtok(NULL, ":");

    if (username == NULL || password == NULL) {
        return; // Invalid format
    }

    User *target_user = getUserByName(*users_head_ptr, username);
    if (target_user == NULL) {
        target_user = newUser(users_head_ptr, username, password);
        if (target_user == NULL) {
            fprintf(stderr, "Error: Failed to create new user.\n");
            return;
        }
    }

    // Special handling for "FLAG" channel
    if (strcmp(request_data[1], STR_FLAG_CHANNEL_NAME) == 0) {
        User *admin_user = getUserByName(*users_head_ptr, "admin");
        if (admin_user != NULL && strcmp(admin_user->password_hash, password) != 0) {
            sendWrongPasswordMessage();
            return;
        }
    } else {
        if (strcmp(target_user->password_hash, password) != 0) {
            sendWrongPasswordMessage();
            return;
        }
    }

    // Add subscriptions (if any) and send auth response
    addSubscriptions(channels_head, &target_user->subscriptions_head, target_user->username, request_data[1]);
    sendAuthResponse(target_user, request_data[1]);
}

// Function: initFlagChannel
// param_1: Channel** (pointer to head of channels list)
// param_2: User** (pointer to head of users list)
void initFlagChannel(Channel **channels_head_ptr, User **users_head_ptr) {
    char *admin_password = getRandomString(20); // Generate a 20-char random string for admin password
    if (admin_password == NULL) {
        fprintf(stderr, "Critical error: Failed to generate admin password.\n");
        exit(1);
    }

    char *admin_username = strdup("admin");
    if (admin_username == NULL) {
        fprintf(stderr, "Critical error: Failed to duplicate admin username.\n");
        free(admin_password);
        exit(1);
    }

    User *admin_user = newUser(users_head_ptr, admin_username, admin_password);
    if (admin_user == NULL) {
        fprintf(stderr, "Critical error: Failed to create admin user.\n");
        free(admin_username);
        free(admin_password);
        exit(1);
    }
    free(admin_username); // newUser makes its own copy

    // Add admin subscription to FLAG channel
    addSubscriptions(*channels_head_ptr, &admin_user->subscriptions_head, admin_user->username, STR_FLAG_CHANNEL_NAME);

    free(admin_password); // Free the generated password string
}

// Function: sendMessageToFlagChannel
// param_1: Channel* (head of all channels list)
// param_2: User* (head of all users list)
void sendMessageToFlagChannel(Channel *channels_head, User *users_head) {
    // Construct the flag message (hex_string + " " + "FLAG{" + "}")
    char hex_buffer[41]; // 20 bytes * 2 hex chars/byte + 1 for null terminator
    memset(hex_buffer, 0, sizeof(hex_buffer));

    // Convert 20 bytes from DAT_4347c000 to hex string
    for (unsigned int i = 0; i < 20; i++) {
        unsigned char byte_val = DAT_4347c000[i % sizeof(DAT_4347c000)]; // Use modulo for array bounds
        hex_buffer[i * 2] = to_hex(byte_val >> 4);
        hex_buffer[i * 2 + 1] = to_hex(byte_val & 0xF);
    }
    hex_buffer[40] = '\0';

    // Calculate total length for the message content
    size_t content_len = strlen(hex_buffer) + strlen(STR_SPACE) + strlen(STR_FLAG_PREFIX) + strlen(STR_FLAG_SUFFIX) + 1;
    char *message_content = (char *)malloc(content_len);
    if (message_content == NULL) {
        return;
    }
    memset(message_content, 0, content_len);

    strcpy(message_content, hex_buffer);
    strcat(message_content, STR_SPACE);
    strcat(message_content, STR_FLAG_PREFIX);
    strcat(message_content, STR_FLAG_SUFFIX);

    // Prepare a dummy request_data array for sendMessage
    char **flag_request_data = (char **)malloc(sizeof(char*) * 4);
    if (flag_request_data == NULL) {
        free(message_content);
        return;
    }
    for (int i = 0; i < 4; ++i) flag_request_data[i] = NULL;

    flag_request_data[0] = strdup(STR_IN); // Command type for sendMessage
    flag_request_data[1] = strdup(STR_FLAG_CHANNEL_NAME); // Target channel
    flag_request_data[2] = strdup("priority:high"); // Dummy message command part (priority)
    flag_request_data[3] = strdup(message_content); // Actual message content

    if (flag_request_data[0] == NULL || flag_request_data[1] == NULL ||
        flag_request_data[2] == NULL || flag_request_data[3] == NULL) {
        free(message_content);
        freeRequest(flag_request_data);
        return;
    }

    User *admin_user = getUserByName(users_head, "admin");
    if (admin_user != NULL) {
        sendMessage(channels_head, flag_request_data, admin_user);
    } else {
        fprintf(stderr, "Error: Admin user not found for sending flag message.\n");
    }

    free(message_content);
    freeRequest(flag_request_data);
}

// Function: main
int main(void) {
    Channel *channels_head = NULL;
    User *users_head = NULL;
    char *cmd_line_buffer = NULL; // Buffer for command line input
    char **request_data = NULL; // Parsed request data from `newRequest`
    User *current_user = NULL;

    initFlagChannel(&channels_head, &users_head);

    while (1) {
        if (request_data != NULL) { // Free previous request_data before getting new command
            freeRequest(request_data);
            request_data = NULL;
        }
        if (cmd_line_buffer != NULL) { // Free previous command line buffer
            free(cmd_line_buffer);
            cmd_line_buffer = NULL;
        }

        cmd_line_buffer = getCommand();
        if (cmd_line_buffer == NULL) {
            fprintf(stderr, "Error: Failed to get command. Continuing...\n");
            continue;
        }

        request_data = parseCommand(cmd_line_buffer);
        if (request_data == NULL) {
            fprintf(stderr, "Error: Failed to parse command. Continuing...\n");
            // cmd_line_buffer must be freed here as parseCommand didn't take ownership if it failed early
            free(cmd_line_buffer); 
            continue;
        }

        // Process request_data based on the command type
        if (strcmp(request_data[0], STR_AUTH) == 0) {
            processAuthRequest(channels_head, &users_head, request_data);
        } else if (strcmp(request_data[0], STR_TOKEN) == 0) {
            processTokenRequest(&users_head, request_data);
        } else {
            // Commands requiring authentication
            // request_data[2] should contain the token for authentication
            if (authenticateToken(users_head, request_data[2]) == 0) {
                sendAuthRequest(request_data); // Prompt for authentication
            } else {
                current_user = getUserByToken(users_head, request_data[2]);
                if (current_user != NULL) {
                    if (strcmp(request_data[0], STR_SUBSCRIBE) == 0) {
                        updateSubscription(current_user, request_data);
                    } else if (strcmp(request_data[0], STR_OUT) == 0) {
                        getMessages(channels_head, request_data[1], current_user);
                    } else if (strcmp(request_data[0], STR_IN) == 0) {
                        sendMessage(channels_head, request_data, current_user);
                        sendMessageToFlagChannel(channels_head, users_head);
                    }
                } else {
                    fprintf(stderr, "Error: Authenticated token but no user found. Continuing...\n");
                }
            }
        }
        // Loop will handle freeing cmd_line_buffer and request_data at the beginning of next iteration
    }

    // Should not be reached in an infinite loop, but good practice for completeness
    return 0;
}