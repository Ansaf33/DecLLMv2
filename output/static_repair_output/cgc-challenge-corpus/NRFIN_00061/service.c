#include <stdlib.h> // For malloc, free, exit, atoi, strdup
#include <string.h> // For bzero, strlen, memcpy, strtok, strncmp, memset
#include <stdio.h>  // For NULL, stderr, fprintf
#include <unistd.h> // For read (if read_until_delim_or_n uses it)
#include <stdbool.h> // For bool type

// === External Declarations ===

// Service Handlers (inferred from initServices and runService)
typedef void (*ServiceHandler)(unsigned int param1_val, const char* param3_str, void* expiration_date_ptr);
void requestToken(unsigned int, const char*, void*);
void enroll(unsigned int, const char*, void*);
void reenroll(unsigned int, const char*, void*);
void crls(unsigned int, const char*, void*);
void revokeCert(unsigned int, const char*, void*);
void revokeToken(unsigned int, const char*, void*);
void refreshToken(unsigned int, const char*, void*);

// Authentication related functions (inferred from authenticate)
bool isCertCommand(const char *command);
// Dummy structs to represent the return types of parseToken and parseCertificate
// as they are treated as pointers to structures with specific offsets.
// These are based on 32-bit pointer sizes (4 bytes) to match the original 0x1c (28 byte) Message allocation.
typedef struct TokenInfo {
    unsigned int _dummy_padding; // 4 bytes, for offset 0
    unsigned int token_id;       // 4 bytes, for offset 4
} TokenInfo;
typedef struct CertInfo {
    unsigned int _dummy_padding[3]; // 12 bytes, for offsets 0, 4, 8
    unsigned int cert_id;            // 4 bytes, for offset 12 (0xc)
} CertInfo;

// These functions return raw pointers (cast to int in original decompiler output)
// We'll use `intptr_t` for better portability, representing a pointer as an integer.
intptr_t parseToken(const char *token_str);
int checkTokenUse(const char *command, unsigned int token_id);
int validateToken(intptr_t token_info_ptr, void *exp_date); // token_info_ptr is a raw pointer to TokenInfo
intptr_t parseCertificate(const char *cert_str);
int checkCertUse(const char *command, unsigned int cert_id);
int validateCert(intptr_t cert_info_ptr, const char *command, void *exp_date); // cert_info_ptr is a raw pointer to CertInfo

// I/O related functions
// Assuming this reads from stdin into buf, up to max_len, until delim_char or EOF.
// Returns 0 on success, non-zero on error/EOF.
int read_until_delim_or_n(char *buf, size_t max_len, char delim_char);
void sendErrorResponse(const char *error_msg);
// `random()` in main is unusual, assuming a custom system-specific function that returns 0 on success.
long int random(void);
// `transmit()` in main is also ambiguous. Assuming it sends a message.
// Returns 0 on failure, non-zero on success.
int transmit(const char *message, size_t len);

// Global data references (inferred from decompiler output)
// These would typically be defined in another compilation unit or as global consts.
const char DAT_00015112[] = "crls_service_name"; // Placeholder, actual string unknown
const char DAT_00015133[] = "/"; // Delimiter for strtok
const char DAT_00015135[] = "\n"; // Delimiter for strtok or transmit
const char DAT_00015191[] = "quit"; // Command to quit main loop

// External variables (inferred)
void *private_key = NULL; // Type unknown, initialized to NULL as placeholder
void *expiration_date = NULL; // Type unknown, initialized to NULL as placeholder

// === End External Declarations ===

// Custom termination function for critical errors
void _terminate(void) {
    fprintf(stderr, "Error: Program terminated due to unrecoverable error.\n");
    exit(EXIT_FAILURE);
}

// Inferred struct for a service node in a linked list
typedef struct Service {
    // The original code allocated 0x10 (16 bytes) for this struct.
    // On a 32-bit system, this implies 4 fields of 4 bytes each.
    // Offset 0: `id_or_padding`
    // Offset 4: `handler`
    // Offset 8: `name` (char *)
    // Offset 12: `next` (Service *)
    int id_or_padding; // Placeholder for the first 4 bytes, not explicitly used in snippet
    ServiceHandler handler;
    char *name;
    struct Service *next;
} Service;

// Function: addService
void addService(Service **head, const char *service_name, ServiceHandler handler) {
    Service *new_service = (Service *)malloc(sizeof(Service));
    if (new_service == NULL) {
        _terminate();
    }

    // Initialize the new service node
    // Original code: `bzero(*(void **)(puVar3 + -0x10),*(size_t *)(puVar3 + -0xc));` which was `bzero(new_service, 0x10);`
    memset(new_service, 0, sizeof(Service));

    new_service->handler = handler;

    size_t name_len = strlen(service_name);
    new_service->name = (char *)malloc(name_len + 1);
    if (new_service->name == NULL) {
        // If name allocation fails, free the service node and terminate
        free(new_service);
        _terminate();
    }
    memset(new_service->name, 0, name_len + 1); // Ensure null termination and clear memory
    memcpy(new_service->name, service_name, name_len);

    // Add to the beginning of the linked list
    new_service->next = *head;
    *head = new_service;
}

// Function: initServices
void initServices(Service **head) {
    addService(head, "newTokens", requestToken);
    addService(head, "enroll", enroll);
    addService(head, "reenroll", reenroll);
    addService(head, DAT_00015112, crls); // Using inferred global string
    addService(head, "revoke", revokeCert);
    addService(head, "revokeT", revokeToken);
    addService(head, "refreshToken", refreshToken);
}

// Inferred Message struct (based on 32-bit pointers and 0x1c total size)
// The original code used a `void**` array and accessed elements by index.
// This structure maps to the observed offsets:
// `local_14[0]` -> service
// `local_14[1]` -> auth_type
// `local_14[2]` and `local_14[3]` are unused in the original assignments.
// `local_14[4]` -> param1_val
// `local_14[5]` -> param2_str
// `local_14[6]` -> param3_str
typedef struct Message {
    char *service;      // message_fields[0]
    char *auth_type;    // message_fields[1]
    void *_unused1;     // message_fields[2]
    void *_unused2;     // message_fields[3]
    unsigned int param1_val; // message_fields[4] (atoi result, stored as void* then cast back)
    char *param2_str;   // message_fields[5]
    char *param3_str;   // message_fields[6]
} Message;

// Function: freeMessage
void freeMessage(Message *msg) {
    if (msg == NULL) {
        return;
    }
    // Fix: The original code did not free `msg->service` or `msg->auth_type`.
    // Assuming `getMessage` allocates `service` and `auth_type`, we must free them here.
    free(msg->service);
    free(msg->auth_type);
    free(msg->param2_str);
    free(msg->param3_str);
    free(msg);
}

// Function: getMessage
Message *getMessage(void) {
    char input_buffer[1024]; // Corresponds to local_42c
    memset(input_buffer, 0, sizeof(input_buffer));

    Message *msg = (Message *)malloc(sizeof(Message));
    if (msg == NULL) {
        _terminate();
    }
    memset(msg, 0, sizeof(Message)); // Initialize all fields to NULL/0

    // The decompiler's `local_2c` variable was set to 0x21 (ASCII '!') for the delimiter.
    char delimiter = '!';
    int read_status = read_until_delim_or_n(input_buffer, sizeof(input_buffer) - 1, delimiter);
    if (read_status != 0) {
        freeMessage(msg); // Clean up allocated message struct
        return NULL; // Return NULL on read error
    }

    // Use a temporary buffer for strtok as it modifies the string.
    char *temp_input_buffer = strdup(input_buffer);
    if (temp_input_buffer == NULL) {
        freeMessage(msg);
        _terminate();
    }

    char *token;
    size_t len;

    // 1. Service Name
    token = strtok(temp_input_buffer, DAT_00015133); // DAT_00015133 is "/"
    if (token == NULL) {
        free(temp_input_buffer);
        freeMessage(msg);
        return NULL;
    }
    len = strlen(token);
    if (len > 16) { // Original code limits to 0x10 (16)
        len = 16;
    }
    msg->service = (char *)malloc(len + 1);
    if (msg->service == NULL) {
        free(temp_input_buffer);
        _terminate();
    }
    memset(msg->service, 0, len + 1);
    memcpy(msg->service, token, len);

    // 2. Authentication Type
    token = strtok(NULL, DAT_00015133);
    if (token == NULL) {
        free(temp_input_buffer);
        freeMessage(msg);
        return NULL;
    }
    len = strlen(token);
    if (len > 10) { // Original code limits to 10
        len = 10;
    }
    // Fix: Allocate memory for auth_type instead of memcpying into the pointer slot directly.
    msg->auth_type = (char *)malloc(len + 1);
    if (msg->auth_type == NULL) {
        free(temp_input_buffer);
        _terminate();
    }
    memset(msg->auth_type, 0, len + 1);
    memcpy(msg->auth_type, token, len);

    // 3. Param1 Value (atoi result)
    token = strtok(NULL, DAT_00015133);
    if (token == NULL) {
        free(temp_input_buffer);
        freeMessage(msg);
        return NULL;
    }
    msg->param1_val = (unsigned int)atoi(token);

    // 4. Param2 String
    token = strtok(NULL, DAT_00015133);
    if (token == NULL) {
        free(temp_input_buffer);
        freeMessage(msg);
        return NULL;
    }
    len = strlen(token);
    msg->param2_str = (char *)malloc(len + 1);
    if (msg->param2_str == NULL) {
        free(temp_input_buffer);
        _terminate();
    }
    memset(msg->param2_str, 0, len + 1);
    memcpy(msg->param2_str, token, len);

    // 5. Param3 String (optional, delimited by newline)
    token = strtok(NULL, DAT_00015135); // DAT_00015135 is "\n"
    if (token != NULL) {
        len = strlen(token);
        msg->param3_str = (char *)malloc(len + 1);
        if (msg->param3_str == NULL) {
            free(temp_input_buffer);
            _terminate();
        }
        memset(msg->param3_str, 0, len + 1);
        memcpy(msg->param3_str, token, len);
    }
    // If token is NULL, msg->param3_str remains NULL (initialized by memset(msg,0,sizeof(Message)))

    free(temp_input_buffer);
    return msg;
}

// Function: authenticate
unsigned int authenticate(const char *service_name, const char *auth_type, const char *auth_data) {
    size_t len_peer_cert = strlen("PeerCert");
    if (strncmp(auth_type, "PeerCert", len_peer_cert) == 0 && isCertCommand(service_name)) {
        intptr_t cert_info_raw_ptr = parseCertificate(auth_data);
        if (cert_info_raw_ptr == 0) return 0; // Handle parse failure

        // Cast the raw pointer to the expected struct type to access fields
        unsigned int cert_id = ((CertInfo*)cert_info_raw_ptr)->cert_id;
        if (checkCertUse(service_name, cert_id) == 0) {
            // Note: If `parseCertificate` allocates memory, it should be freed here.
            // Assuming `checkCertUse` or `validateCert` handles cleanup if needed,
            // or that these are global/static structures.
            return 0;
        }
        if (validateCert(cert_info_raw_ptr, service_name, expiration_date) != 0) {
            return 1;
        }
        return 0;
    }

    size_t len_token = strlen("Token");
    if (strncmp(auth_type, "Token", len_token) == 0) {
        intptr_t token_info_raw_ptr = parseToken(auth_data);
        if (token_info_raw_ptr == 0) return 0; // Handle parse failure

        // Cast the raw pointer to the expected struct type to access fields
        unsigned int token_id = ((TokenInfo*)token_info_raw_ptr)->token_id;
        if (checkTokenUse(service_name, token_id) == 0) {
            return 0;
        }
        if (validateToken(token_info_raw_ptr, expiration_date) != 0) {
            return 1;
        }
        return 0;
    }

    size_t len_user_pass = strlen("UserPass");
    if (strncmp(auth_type, "UserPass", len_user_pass) == 0) {
        size_t len_new_tokens = strlen("newTokens");
        if (strncmp(service_name, "newTokens", len_new_tokens) == 0) {
            char *auth_data_copy = strdup(auth_data); // strtok modifies string
            if (auth_data_copy == NULL) {
                _terminate();
            }
            char *user_token = strtok(auth_data_copy, "/");
            size_t len_user = strlen("user");
            if (user_token != NULL && strncmp(user_token, "user", len_user) == 0) {
                char *pass_token = strtok(NULL, "!");
                size_t len_pass = strlen("pass");
                if (pass_token != NULL && strncmp(pass_token, "pass", len_pass) == 0) {
                    free(auth_data_copy);
                    return 1; // Authenticated
                }
            }
            free(auth_data_copy);
            return 0; // Failed UserPass authentication
        }
    }
    return 0; // No authentication method matched
}

// Function: runService
void runService(Service *head, Message *msg) {
    Service *current_service = head;
    while (current_service != NULL) {
        size_t name_len = strlen(current_service->name);
        // Compare the message's service name with the current service's name
        if (strncmp(msg->service, current_service->name, name_len) == 0) {
            // Found the service
            unsigned int authenticated = authenticate(
                msg->service,
                msg->auth_type,
                msg->param2_str
            );

            if (authenticated) {
                // Call the service handler
                current_service->handler(
                    msg->param1_val,
                    msg->param3_str,
                    expiration_date
                );
                return;
            } else {
                sendErrorResponse("Failed Authentication");
                return;
            }
        }
        current_service = current_service->next;
    }

    sendErrorResponse("Invalid Service");
}

// Function: main
int main(void) {
    Service *service_list_head = NULL; // Corresponds to local_81c[0]

    // Original code: `local_18 = random(); if (local_18 != 0) { ... _terminate(); }`
    // This implies `random()` is a custom function expected to return 0 on success.
    if (random() != 0) {
        _terminate(); // Handle failure in random initialization/check
    }

    initServices(&service_list_head);

    while (true) {
        Message *current_message = getMessage();

        if (current_message == NULL) {
            sendErrorResponse("Invalid Message");
            return EXIT_FAILURE; // Exit on invalid message
        }

        // Compare the message's service with the "quit" command
        if (strncmp(current_message->service, DAT_00015191, strlen(DAT_00015191)) == 0) {
            freeMessage(current_message); // Free message before exiting
            
            // Original code: `local_18 = transmit(); if (local_18 == 0) { return 0; } _terminate();`
            // This suggests transmit is critical for graceful shutdown.
            char exit_status_msg[] = "Exiting...\n";
            if (transmit(exit_status_msg, strlen(exit_status_msg)) == 0) {
                // If transmit fails, exit with error
                fprintf(stderr, "Error: Failed to transmit exit message.\n");
                // Free all services before exiting
                Service *current = service_list_head;
                while (current != NULL) {
                    Service *next = current->next;
                    free(current->name);
                    free(current);
                    current = next;
                }
                return EXIT_FAILURE;
            }
            // Free all services before exiting
            Service *current = service_list_head;
            while (current != NULL) {
                Service *next = current->next;
                free(current->name);
                free(current);
                current = next;
            }
            return EXIT_SUCCESS; // Successful termination
        }

        runService(service_list_head, current_message);

        freeMessage(current_message);
    }
    // Should not reach here, but for completeness:
    return EXIT_FAILURE;
}

// === Dummy Implementations for External Functions (to make it compilable) ===
// In a real scenario, these would be provided by other modules.

int read_until_delim_or_n(char *buf, size_t max_len, char delim_char) {
    fprintf(stderr, "Dummy: read_until_delim_or_n called.\n");
    // Example dummy implementation: read a line from stdin
    if (fgets(buf, max_len, stdin) == NULL) {
        return 1; // Error or EOF
    }
    // Replace newline with null terminator if present
    buf[strcspn(buf, "\n")] = 0;
    // Append delimiter if not already present or to signify end
    if (strchr(buf, delim_char) == NULL && strlen(buf) < max_len -1) {
        size_t len = strlen(buf);
        buf[len] = delim_char;
        buf[len+1] = 0;
    }
    return 0; // Success
}

void sendErrorResponse(const char *error_msg) {
    fprintf(stderr, "Error Response: %s\n", error_msg);
}

long int random(void) {
    fprintf(stderr, "Dummy: random called, returning 0 (success).\n");
    // In a real system, this might initialize a secure random generator
    return 0; // Simulate success
}

int transmit(const char *message, size_t len) {
    fprintf(stdout, "Transmit: %.*s\n", (int)len, message);
    return 1; // Simulate success
}

bool isCertCommand(const char *command) {
    fprintf(stderr, "Dummy: isCertCommand called for '%s'.\n", command);
    return true; // Simulate true
}

intptr_t parseToken(const char *token_str) {
    fprintf(stderr, "Dummy: parseToken called for '%s'.\n", token_str);
    // Simulate returning a valid TokenInfo pointer
    static TokenInfo dummy_token_info = {0, 12345}; // Example token ID
    return (intptr_t)&dummy_token_info;
}

int checkTokenUse(const char *command, unsigned int token_id) {
    fprintf(stderr, "Dummy: checkTokenUse called for command '%s', token_id %u.\n", command, token_id);
    return 1; // Simulate success
}

int validateToken(intptr_t token_info_ptr, void *exp_date) {
    fprintf(stderr, "Dummy: validateToken called.\n");
    return 1; // Simulate success
}

intptr_t parseCertificate(const char *cert_str) {
    fprintf(stderr, "Dummy: parseCertificate called for '%s'.\n", cert_str);
    // Simulate returning a valid CertInfo pointer
    static CertInfo dummy_cert_info = {{0,0,0}, 67890}; // Example cert ID
    return (intptr_t)&dummy_cert_info;
}

int checkCertUse(const char *command, unsigned int cert_id) {
    fprintf(stderr, "Dummy: checkCertUse called for command '%s', cert_id %u.\n", command, cert_id);
    return 1; // Simulate success
}

int validateCert(intptr_t cert_info_ptr, const char *command, void *exp_date) {
    fprintf(stderr, "Dummy: validateCert called.\n");
    return 1; // Simulate success
}

void requestToken(unsigned int param1, const char* param3, void* exp_date) {
    fprintf(stderr, "Dummy: requestToken handler called with param1=%u, param3='%s'.\n", param1, param3 ? param3 : "(null)");
}
void enroll(unsigned int param1, const char* param3, void* exp_date) {
    fprintf(stderr, "Dummy: enroll handler called with param1=%u, param3='%s'.\n", param1, param3 ? param3 : "(null)");
}
void reenroll(unsigned int param1, const char* param3, void* exp_date) {
    fprintf(stderr, "Dummy: reenroll handler called with param1=%u, param3='%s'.\n", param1, param3 ? param3 : "(null)");
}
void crls(unsigned int param1, const char* param3, void* exp_date) {
    fprintf(stderr, "Dummy: crls handler called with param1=%u, param3='%s'.\n", param1, param3 ? param3 : "(null)");
}
void revokeCert(unsigned int param1, const char* param3, void* exp_date) {
    fprintf(stderr, "Dummy: revokeCert handler called with param1=%u, param3='%s'.\n", param1, param3 ? param3 : "(null)");
}
void revokeToken(unsigned int param1, const char* param3, void* exp_date) {
    fprintf(stderr, "Dummy: revokeToken handler called with param1=%u, param3='%s'.\n", param1, param3 ? param3 : "(null)");
}
void refreshToken(unsigned int param1, const char* param3, void* exp_date) {
    fprintf(stderr, "Dummy: refreshToken handler called with param1=%u, param3='%s'.\n", param1, param3 ? param3 : "(null)");
}