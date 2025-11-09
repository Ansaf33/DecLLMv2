#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

// Type definitions from the snippet or inferred
typedef unsigned char byte;
typedef unsigned int undefined; // Using unsigned int as a generic 4-byte undefined type
typedef unsigned int undefined4;
typedef unsigned int uint;

// --- Global Variables ---
// Assume 32-bit pointers based on offsets (0xc, 0x1c, etc.)
// Forward declarations for structs
struct FileNode;
struct LogNode;

struct FileNode *files = NULL;
struct LogNode *loglist = NULL;

char dolog = 1;    // Initially enabled
char verified = 0; // Initially unverified

// Global constants / magic numbers
const unsigned int LOG_SUCCESS_CODE = 0x4347c000;
const unsigned int LOG_FAILURE_CODE = 0x4347c002;
const unsigned int REQ_MAGIC_0 = 0x41414141; // DAT_00083cb0 in handle_request

// Response status codes
const unsigned int RESP_STATUS_SUCCESS = 0xbaaaaaaa;
const unsigned int RESP_STATUS_INTERNAL_ERROR = 0xbaaaaaab;
const unsigned int RESP_STATUS_NOT_IMPLEMENTED = 0xbaaaaaad;

// Challenge related
uint __cookie = 0x127a1b76; // Value from do_challenge for comparison
char *challenges[] = {
    "Challenge 0",
    "Challenge 1",
    "Challenge 2",
    "Challenge 3"
};

// --- Utility Functions ---

// Simple wrapper for write to stderr/stdout
void sendall(int fd, const char *buf, size_t len) {
    ssize_t sent = 0;
    while (sent < len) {
        ssize_t res = write(fd, buf + sent, len - sent);
        if (res < 0) {
            perror("sendall failed");
            exit(1);
        }
        sent += res;
    }
}

// sendline function, inferred to send the buffer followed by a newline
int sendline(int fd, const char *buf, size_t len) {
    sendall(fd, buf, len);
    sendall(fd, "\n", 1);
    return len + 1;
}

// Simple wrapper for read
ssize_t recv_internal(int fd, void *buf, size_t len) {
    ssize_t received = 0;
    while (received < len) {
        ssize_t res = read(fd, buf + received, len - received);
        if (res <= 0) {
            return res; // Error or EOF
        }
        received += res;
    }
    return received;
}

// randint function, using standard rand()
uint randint() {
    return (uint)rand();
}

// Terminate function, replacing _terminate()
void _terminate() {
    fprintf(stderr, "Critical error, terminating.\n");
    exit(1);
}

// Stack cookie fail, replacing __stack_cookie_fail()
void __stack_cookie_fail() {
    fprintf(stderr, "Stack cookie check failed, terminating.\n");
    exit(1);
}

// --- Struct Definitions (inferred from usage) ---

// Request structure (0x1c bytes)
typedef struct Request {
    unsigned int magic_0;   // 0x0
    unsigned int version;   // 0x4
    unsigned int type;      // 0x8
    unsigned int path_len;  // 0xc
    unsigned int body_len;  // 0x10
    char *path_ptr;         // 0x14
    char *body_ptr;         // 0x18
} Request; // Total size 0x1c (7 * 4 bytes = 28 bytes)

// Response structure (0x10 bytes)
typedef struct Response {
    unsigned int magic_0;   // 0x0
    unsigned int status;    // 0x4
    unsigned int data_len;  // 0x8
    char *data_ptr;         // 0xc
} Response; // Total size 0x10 (4 * 4 bytes = 16 bytes)

// FileEntry structure (0x10 bytes)
typedef struct FileEntry {
    unsigned int path_len;  // 0x0
    unsigned int body_len;  // 0x4
    char *path_ptr;         // 0x8
    char *body_ptr;         // 0xc
} FileEntry; // Total size 0x10 (4 * 4 bytes = 16 bytes)

// Log Entry structure (8 bytes)
typedef struct LogEntry {
    unsigned int status_code; // 0x0
    Request *request;         // 0x4 (pointer, 4 bytes)
} LogEntry; // Total size 8 bytes

// Linked List Nodes (assuming 32-bit pointers)
// FileNode structure (0xc bytes)
typedef struct FileNode {
    struct FileNode *next; // 0x0
    unsigned int padding;  // 0x4 (filler to align file_ptr at 0x8)
    FileEntry *file;       // 0x8
} FileNode; // Total size 0xc (3 * 4 bytes = 12 bytes)

// LogNode structure (0xc bytes)
typedef struct LogNode {
    struct LogNode *next; // 0x0
    unsigned int padding; // 0x4 (filler to align entry_ptr at 0x8)
    LogEntry *entry;      // 0x8
} LogNode; // Total size 0xc (3 * 4 bytes = 12 bytes)


// --- Linked List Utility Functions ---

void list_push_back_file(struct FileNode **head, FileNode *newNode) {
    if (!newNode) return;
    newNode->next = NULL;
    if (!*head) {
        *head = newNode;
    } else {
        struct FileNode *current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void list_push_back_log(struct LogNode **head, LogNode *newNode) {
    if (!newNode) return;
    newNode->next = NULL;
    if (!*head) {
        *head = newNode;
    } else {
        struct LogNode *current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void list_remove_file(struct FileNode **head, FileNode *nodeToRemove) {
    if (!*head || !nodeToRemove) return;

    if (*head == nodeToRemove) {
        *head = nodeToRemove->next;
        if (nodeToRemove->file) {
            free(nodeToRemove->file->path_ptr);
            free(nodeToRemove->file->body_ptr);
            free(nodeToRemove->file);
        }
        free(nodeToRemove);
        return;
    }

    struct FileNode *current = *head;
    while (current->next && current->next != nodeToRemove) {
        current = current->next;
    }

    if (current->next == nodeToRemove) {
        current->next = nodeToRemove->next;
        if (nodeToRemove->file) {
            free(nodeToRemove->file->path_ptr);
            free(nodeToRemove->file->body_ptr);
            free(nodeToRemove->file);
        }
        free(nodeToRemove);
    }
}


// --- Function Prototypes for inter-dependencies ---
Response *internalerror_resp(void);
Response *notimplemented_resp(void);
char do_challenge(void);
Response *genericmsg(unsigned int status, const char *msg, size_t msg_len);
FileEntry *get_file(const char *path, unsigned int path_len);
char add_file(FileEntry *file_entry);
char delete_file(const char *path, unsigned int path_len);


// Function: logreq
void logreq(Request *req, undefined4 status_code) {
    LogNode *log_node = NULL;
    LogEntry *log_entry = NULL;
    Request *logged_req_data = NULL;

    if (!req) {
        sendall(2, "Passed null req\n", strlen("Passed null req\n"));
        return;
    }

    logged_req_data = (Request *)calloc(1, sizeof(Request));
    if (!logged_req_data) {
        sendall(2, "Failed to allocate log\n", strlen("Failed to allocate log\n"));
        return;
    }

    memcpy(logged_req_data, req, sizeof(Request));

    if (req->path_ptr && req->path_len != 0) {
        char *path_copy = (char *)calloc(req->path_len, 1);
        if (!path_copy) {
            sendall(2, "Failed to allocate path for log\n", strlen("Failed to allocate path for log\n"));
            free(logged_req_data);
            return;
        }
        memcpy(path_copy, req->path_ptr, req->path_len);
        logged_req_data->path_ptr = path_copy;
    } else {
        logged_req_data->path_ptr = NULL;
        logged_req_data->path_len = 0;
    }

    if (req->body_ptr && req->body_len != 0) {
        char *body_copy = (char *)calloc(req->body_len, 1);
        if (!body_copy) {
            sendall(2, "Failed to allocate body for log\n", strlen("Failed to allocate body for log\n"));
            free(logged_req_data->path_ptr);
            free(logged_req_data);
            return;
        }
        memcpy(body_copy, req->body_ptr, req->body_len);
        logged_req_data->body_ptr = body_copy;
    } else {
        logged_req_data->body_ptr = NULL;
        logged_req_data->body_len = 0;
    }

    log_entry = (LogEntry *)calloc(1, sizeof(LogEntry));
    if (!log_entry) {
        sendall(2, "Failed to allocate log entry\n", strlen("Failed to allocate log entry\n"));
        free(logged_req_data->path_ptr);
        free(logged_req_data->body_ptr);
        free(logged_req_data);
        return;
    }
    log_entry->request = logged_req_data;
    log_entry->status_code = status_code;

    log_node = (LogNode *)calloc(1, sizeof(LogNode));
    if (!log_node) {
        sendall(2, "Failed to allocate log node\n", strlen("Failed to allocate log node\n"));
        free(log_entry->request->path_ptr);
        free(log_entry->request->body_ptr);
        free(log_entry->request);
        free(log_entry);
        return;
    }
    log_node->entry = log_entry;
    list_push_back_log(&loglist, log_node);
}

// Function: get_file
FileEntry *get_file(const char *path, uint path_len) {
    if (!path) return NULL;

    FileNode *current_node = files;
    while (current_node) {
        FileEntry *file_entry = current_node->file;
        if (file_entry && file_entry->path_ptr) {
            uint cmp_len = path_len < file_entry->path_len ? path_len : file_entry->path_len;
            if (memcmp(path, file_entry->path_ptr, cmp_len) == 0 && path_len == file_entry->path_len) {
                return file_entry;
            }
        }
        current_node = current_node->next;
    }
    return NULL;
}

// Function: add_file
char add_file(FileEntry *new_file_entry) {
    if (!new_file_entry) return 0;

    if (get_file(new_file_entry->path_ptr, new_file_entry->path_len) != NULL) {
        return 0; // File already exists
    }

    FileNode *new_node = (FileNode *)calloc(1, sizeof(FileNode));
    if (!new_node) {
        return 0; // Failed to allocate node
    }

    new_node->file = new_file_entry;
    list_push_back_file(&files, new_node);
    return 1;
}

// Function: delete_file
char delete_file(const char *path, uint path_len) {
    if (!path) return 0;

    FileNode *current_node = files;
    FileNode *prev_node = NULL;

    while (current_node) {
        FileEntry *file_entry = current_node->file;
        if (file_entry && file_entry->path_ptr) {
            uint cmp_len = path_len < file_entry->path_len ? path_len : file_entry->path_len;
            if (memcmp(path, file_entry->path_ptr, cmp_len) == 0 && path_len == file_entry->path_len) {
                if (prev_node) {
                    prev_node->next = current_node->next;
                } else {
                    files = current_node->next;
                }
                free(file_entry->path_ptr);
                free(file_entry->body_ptr);
                free(file_entry);
                free(current_node);
                return 1;
            }
        }
        prev_node = current_node;
        current_node = current_node->next;
    }
    return 0;
}

// Function: genericmsg
Response *genericmsg(undefined4 status, const char *msg, size_t msg_len) {
    Response *resp = (Response *)calloc(1, sizeof(Response));
    if (!resp) {
        sendall(2, "Failed to allocate resp\n", strlen("Failed to allocate resp\n"));
        return NULL;
    }

    char *data = (char *)calloc(msg_len, 1);
    if (!data) {
        sendall(2, "Failed to allocate resp data\n", strlen("Failed to allocate resp data\n"));
        free(resp);
        return NULL;
    }

    strcpy(data, msg);
    resp->data_ptr = data;
    resp->data_len = strlen(data) + 1;
    resp->status = status;
    resp->magic_0 = REQ_MAGIC_0;

    return resp;
}

// Function: internalerror (returns Response*)
Response *internalerror_resp(void) {
    return genericmsg(RESP_STATUS_INTERNAL_ERROR, "If anyone asks, you're my debugger.", 0x24);
}

// Function: notimplemented (returns Response*)
Response *notimplemented_resp(void) {
    return genericmsg(RESP_STATUS_NOT_IMPLEMENTED, "You want me to do *two* things?!", 0x21);
}

// Function: handle_retrieve
Response *handle_retrieve(Request *req) {
    Response *resp = NULL;

    if (!req) {
        sendall(2, "Passed null req\n", strlen("Passed null req\n"));
        return internalerror_resp();
    }

    resp = (Response *)calloc(1, sizeof(Response));
    if (!resp) {
        sendall(2, "Failed to allocate resp.\n", strlen("Failed to allocate resp.\n"));
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    FileEntry *found_file = get_file(req->path_ptr, req->path_len);
    if (!found_file) {
        sendall(2, "Failed to find file.\n", strlen("Failed to find file.\n"));
        free(resp);
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    char *data_copy = (char *)calloc(found_file->body_len, 1);
    if (!data_copy) {
        sendall(2, "Failed to allocate resp data\n", strlen("Failed to allocate resp data\n"));
        free(resp);
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    memcpy(data_copy, found_file->body_ptr, found_file->body_len);
    resp->data_ptr = data_copy;
    resp->data_len = found_file->body_len;
    resp->status = RESP_STATUS_SUCCESS;
    resp->magic_0 = REQ_MAGIC_0;

    if (dolog) {
        logreq(req, LOG_SUCCESS_CODE);
    }
    return resp;
}

// Function: handle_submit
Response *handle_submit(Request *req) {
    Response *response_to_send = NULL;
    FileEntry *new_file_entry = NULL;
    char *path_copy = NULL;
    char *body_copy = NULL;

    if (!req) {
        sendall(2, "Passed null req\n", strlen("Passed null req\n"));
        return internalerror_resp();
    }

    new_file_entry = (FileEntry *)calloc(1, sizeof(FileEntry));
    if (!new_file_entry) {
        sendall(2, "Failed to alloc file.\n", strlen("Failed to alloc file.\n"));
        return internalerror_resp();
    }

    path_copy = (char *)calloc(req->path_len, 1);
    if (!path_copy) {
        sendall(2, "Failed to alloc path.\n", strlen("Failed to alloc path.\n"));
        free(new_file_entry);
        return internalerror_resp();
    }
    memcpy(path_copy, req->path_ptr, req->path_len);
    new_file_entry->path_ptr = path_copy;
    new_file_entry->path_len = req->path_len;

    body_copy = (char *)calloc(req->body_len, 1);
    if (!body_copy) {
        sendall(2, "Failed to alloc body.\n", strlen("Failed to alloc body.\n"));
        free(path_copy);
        free(new_file_entry);
        return internalerror_resp();
    }
    memcpy(body_copy, req->body_ptr, req->body_len);
    new_file_entry->body_ptr = body_copy;
    new_file_entry->body_len = req->body_len;

    if (add_file(new_file_entry)) {
        if (dolog) {
            logreq(req, LOG_SUCCESS_CODE);
        }
        response_to_send = genericmsg(RESP_STATUS_SUCCESS, "Oh, sure. Let the robot do all the work.", 0x29);
    } else {
        sendall(2, "Failed to add file (might already exist).\n", strlen("Failed to add file (might already exist).\n"));
        free(new_file_entry->path_ptr);
        free(new_file_entry->body_ptr);
        free(new_file_entry);
        response_to_send = internalerror_resp();
    }

    if (dolog && !response_to_send) {
        logreq(req, LOG_FAILURE_CODE);
    }
    return response_to_send;
}

// Function: handle_check
Response *handle_check(Request *req) {
    Response *resp = NULL;

    if (!req) {
        sendall(2, "Passed null req\n", strlen("Passed null req\n"));
        return internalerror_resp();
    }

    resp = (Response *)calloc(1, sizeof(Response));
    if (!resp) {
        sendall(2, "Failed to allocate resp.\n", strlen("Failed to allocate resp.\n"));
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    sendall(2, "in handle check\n", strlen("in handle check\n"));

    FileEntry *found_file = get_file(req->path_ptr, req->path_len);
    if (!found_file) {
        sendall(2, "Failed to find file.\n", strlen("Failed to find file.\n"));
        free(resp);
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    unsigned int *data_copy = (unsigned int *)calloc(1, sizeof(unsigned int));
    if (!data_copy) {
        sendall(2, "Failed to allocate resp data\n", strlen("Failed to allocate resp data\n"));
        free(resp);
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    sendall(2, "found file\n", strlen("found file\n"));

    *data_copy = found_file->body_len;
    resp->data_ptr = (char *)data_copy;
    resp->data_len = sizeof(unsigned int);
    resp->status = RESP_STATUS_SUCCESS;
    resp->magic_0 = REQ_MAGIC_0;

    if (dolog) {
        logreq(req, LOG_SUCCESS_CODE);
    }
    return resp;
}

// Function: handle_delete
Response *handle_delete(Request *req) {
    Response *response_to_send = NULL;

    if (!req) {
        sendall(2, "Passed null req\n", strlen("Passed null req\n"));
        return internalerror_resp();
    }

    if (delete_file(req->path_ptr, req->path_len)) {
        if (dolog) {
            logreq(req, LOG_SUCCESS_CODE);
        }
        response_to_send = genericmsg(RESP_STATUS_SUCCESS, "Hopes: deleted.", 0x10);
    } else {
        sendall(2, "Failed to delete file (might not exist).\n", strlen("Failed to delete file (might not exist).\n"));
        response_to_send = internalerror_resp();
    }

    if (dolog && !response_to_send) {
        logreq(req, LOG_FAILURE_CODE);
    }
    return response_to_send;
}

// Function: handle_auth
Response *handle_auth(Request *req) {
    Response *resp = NULL;
    uint original_cookie = __cookie;

    char challenge_success = do_challenge();

    if (!challenge_success) {
        sendall(2, "Failed to auth!\n", strlen("Failed to auth!\n"));
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    resp = (Response *)calloc(1, sizeof(Response));
    if (!resp) {
        sendall(2, "Failed to allocate resp.\n", strlen("Failed to allocate resp.\n"));
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    if (!req) {
        sendall(2, "Passed null req\n", strlen("Passed null req\n"));
        free(resp);
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    FileEntry *found_file = get_file(req->path_ptr, req->path_len);
    if (!found_file) {
        sendall(2, "Failed to find file.\n", strlen("Failed to find file.\n"));
        free(resp);
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    char *data_buffer = (char *)calloc(found_file->body_len, 1);
    if (!data_buffer) {
        sendall(2, "Failed to allocate resp data\n", strlen("Failed to allocate resp data\n"));
        free(resp);
        if (dolog) logreq(req, LOG_FAILURE_CODE);
        return internalerror_resp();
    }

    memcpy(data_buffer, found_file->body_ptr, found_file->body_len);

    byte xor_key = randint();
    for (uint i = 0; i < found_file->body_len; i++) {
        data_buffer[i] ^= xor_key;
    }

    resp->data_ptr = data_buffer;
    resp->data_len = found_file->body_len;
    resp->status = RESP_STATUS_SUCCESS;
    resp->magic_0 = REQ_MAGIC_0;

    if (dolog) {
        logreq(req, LOG_SUCCESS_CODE);
    }

    if (original_cookie != __cookie) {
        __stack_cookie_fail();
    }
    return resp;
}

// Function: handle_v1
Response *handle_v1(Request *req) {
    sendall(2, "Got v1 req\n", strlen("Got v1 req\n"));

    if (!req) {
        sendall(2, "Passed null request\n", strlen("Passed null request\n"));
        return internalerror_resp();
    } else if (req->type == 0x41414141) { // RETRIEVE
        return handle_retrieve(req);
    } else if (req->type == 0x41414142) { // SUBMIT
        return handle_submit(req);
    } else {
        if (dolog) {
            logreq(req, LOG_FAILURE_CODE);
        }
        return notimplemented_resp();
    }
}

// Function: handle_v2
Response *handle_v2(Request *req) {
    sendall(2, "Got v2 req\n", strlen("Got v2 req\n"));

    if (!req) {
        sendall(2, "Passed null request\n", strlen("Passed null request\n"));
        return internalerror_resp();
    } else {
        uint uVar1 = req->type;
        if (uVar1 == 0x41414145) { // DELETE
            return handle_delete(req);
        } else if (uVar1 == 0x41414143) { // CHECK
            return handle_check(req);
        } else if (uVar1 == 0x41414141) { // RETRIEVE
            return handle_retrieve(req);
        } else if (uVar1 == 0x41414142) { // SUBMIT
            return handle_submit(req);
        } else {
            if (dolog) {
                logreq(req, LOG_FAILURE_CODE);
            }
            return notimplemented_resp();
        }
    }
}

// Function: handle_v3
Response *handle_v3(Request *req) {
    sendall(2, "Got v3 req\n", strlen("Got v3 req\n"));

    if (!req) {
        sendall(2, "Passed null request\n", strlen("Passed null request\n"));
        return internalerror_resp();
    } else {
        switch (req->type) {
            case 0x41414141: // RETRIEVE
                return handle_retrieve(req);
            case 0x41414142: // SUBMIT
                return handle_submit(req);
            case 0x41414143: // CHECK
                return handle_check(req);
            case 0x41414144: // AUTH
                return handle_auth(req);
            case 0x41414145: // DELETE
                return handle_delete(req);
            default:
                if (dolog) {
                    logreq(req, LOG_FAILURE_CODE);
                }
                return notimplemented_resp();
        }
    }
}

// Function: handle_v4
Response *handle_v4(Request *req) {
    sendall(2, "Got v4 req\n", strlen("Got v4 req\n"));

    if (!req) {
        sendall(2, "Passed null request\n", strlen("Passed null request\n"));
        return internalerror_resp();
    } else {
        if (dolog) {
            logreq(req, LOG_FAILURE_CODE);
        }
        return notimplemented_resp();
    }
}

// Function: do_challenge
char do_challenge(void) {
    uint random_idx = randint() & 3;
    char *challenge_str = challenges[random_idx];
    size_t challenge_len = strlen(challenge_str);

    char *encoded_buf = (char *)calloc(challenge_len + 1, 1);
    char *decoded_buf = (char *)calloc(challenge_len + 1, 1);
    if (!encoded_buf || !decoded_buf) {
        sendall(2, "Failed to allocate encode/decode buffers.\n", strlen("Failed to allocate encode/decode buffers.\n"));
        free(encoded_buf);
        free(decoded_buf);
        return 0;
    }

    byte xor_key_str = randint();
    uint xor_key_int = randint();
    uint xor_key_int_encoded = xor_key_int;

    for (uint i = 0; i < challenge_len; i++) {
        encoded_buf[i] = challenge_str[i] ^ xor_key_str;
    }

    for (uint i = 0; i < sizeof(uint); i++) {
        ((byte *)&xor_key_int_encoded)[i] ^= xor_key_str;
    }

    char len_str[16];
    snprintf(len_str, sizeof(len_str), "%zu", challenge_len);
    sendline(1, "len: ", strlen("len: "));
    sendline(1, len_str, strlen(len_str));

    sendall(1, "encoded: ", strlen("encoded: "));
    sendall(1, encoded_buf, challenge_len);
    sendline(1, "", 0); // Newline after encoded data

    char token_str[16];
    snprintf(token_str, sizeof(token_str), "%u", xor_key_int_encoded);
    sendline(1, "token: ", strlen("token: "));
    sendline(1, token_str, strlen(token_str));


    sendline(1, "grab decoded", strlen("grab decoded"));
    ssize_t bytes_read_decoded_str = recv_internal(0, decoded_buf, challenge_len);
    if (bytes_read_decoded_str <= 0) {
        sendline(1, "Please? Thanks? Sorry?", strlen("Please? Thanks? Sorry?"));
        free(encoded_buf);
        free(decoded_buf);
        _terminate();
    }
    decoded_buf[bytes_read_decoded_str] = '\0';

    sendline(1, "grabbed,now dtok", strlen("grabbed,now dtok"));
    uint received_token;
    ssize_t bytes_read_token = recv_internal(0, &received_token, sizeof(uint));
    if (bytes_read_token <= 0) {
        sendline(1, "Please? Thanks? Sorry?", strlen("Please? Thanks? Sorry?"));
        free(encoded_buf);
        free(decoded_buf);
        _terminate();
    }

    sendline(1, "grabbed all", strlen("grabbed all"));

    char success = 0;
    if ((memcmp(challenge_str, decoded_buf, challenge_len) == 0) && (challenge_len == strlen(decoded_buf))) {
        if (xor_key_int == (received_token ^ __cookie)) {
            success = 1;
        }
    }

    free(encoded_buf);
    free(decoded_buf);
    return success;
}

// Function: handle_request
unsigned int handle_request(void) {
    Request req_buffer;
    Response *response_to_send = NULL;
    char *path_data = NULL;
    char *body_data = NULL;

    memset(&req_buffer, 0, sizeof(Request));
    dolog = 1;

    ssize_t bytes_received = recv_internal(0, &req_buffer, sizeof(Request));
    if (bytes_received != sizeof(Request)) {
        sendall(2, "Failed to recv request.\n", strlen("Failed to recv request.\n"));
        _terminate();
    }

    if (req_buffer.magic_0 != REQ_MAGIC_0) {
        req_buffer.magic_0 = __builtin_bswap32(req_buffer.magic_0);
        req_buffer.version = __builtin_bswap32(req_buffer.version);
        req_buffer.type = __builtin_bswap32(req_buffer.type);
        req_buffer.path_len = __builtin_bswap32(req_buffer.path_len);
        req_buffer.body_len = __builtin_bswap32(req_buffer.body_len);
    }

    if (req_buffer.type == 0x41414146) {
        sendall(1, "Received terminate request.\n", strlen("Received terminate request.\n"));
        return 0;
    }

    if (req_buffer.path_len > 0x100 || req_buffer.body_len > 0x200000) {
        response_to_send = genericmsg(RESP_STATUS_INTERNAL_ERROR, "Invalid request lengths.", strlen("Invalid request lengths."));
        if (dolog) logreq(&req_buffer, LOG_FAILURE_CODE);
        goto end_request_processing;
    }

    if (!verified) {
        if (!do_challenge()) {
            if (dolog) logreq(&req_buffer, LOG_FAILURE_CODE);
            return 0;
        }
        verified = 1;
    }

    if (req_buffer.path_len != 0) {
        path_data = (char *)calloc(req_buffer.path_len, 1);
        if (!path_data) {
            sendall(2, "Failed to allocate path.\n", strlen("Failed to allocate path.\n"));
            _terminate();
        }
        bytes_received = recv_internal(0, path_data, req_buffer.path_len);
        if (bytes_received <= 0) {
            sendline(1, "Please? Thanks? Sorry?", strlen("Please? Thanks? Sorry?"));
            free(path_data);
            _terminate();
        }
        req_buffer.path_ptr = path_data;
    }

    if (req_buffer.body_len != 0) {
        body_data = (char *)calloc(req_buffer.body_len, 1);
        if (!body_data) {
            sendall(2, "Failed to allocate body.\n", strlen("Failed to allocate body.\n"));
            free(path_data);
            _terminate();
        }
        bytes_received = recv_internal(0, body_data, req_buffer.body_len);
        if (bytes_received <= 0) {
            sendline(1, "Please? Thanks? Sorry?", strlen("Please? Thanks? Sorry?"));
            free(path_data);
            free(body_data);
            _terminate();
        }
        req_buffer.body_ptr = body_data;
    }

    switch (req_buffer.version) {
        case 0x10000000:
            response_to_send = handle_v1(&req_buffer);
            break;
        case 0x10000001:
            response_to_send = handle_v2(&req_buffer);
            break;
        case 0x10000002:
            response_to_send = handle_v3(&req_buffer);
            break;
        case 0x10000003:
            response_to_send = handle_v4(&req_buffer);
            break;
        default:
            if (dolog) {
                logreq(&req_buffer, LOG_FAILURE_CODE);
            }
            response_to_send = notimplemented_resp();
            break;
    }

end_request_processing:
    if (response_to_send) {
        unsigned int response_magic_0 = response_to_send->magic_0;
        unsigned int response_status = response_to_send->status;
        unsigned int response_data_len = response_to_send->data_len;

        if (req_buffer.magic_0 != REQ_MAGIC_0) {
            response_magic_0 = __builtin_bswap32(response_magic_0);
            response_status = __builtin_bswap32(response_status);
            response_data_len = __builtin_bswap32(response_data_len);
        }

        Response response_header = {
            .magic_0 = response_magic_0,
            .status = response_status,
            .data_len = response_data_len,
            .data_ptr = NULL
        };

        sendall(1, (char *)&response_header, sizeof(Response) - sizeof(char*));
        sendall(1, response_to_send->data_ptr, response_to_send->data_len);

        free(response_to_send->data_ptr);
        free(response_to_send);
    }

    free(path_data);
    free(body_data);

    return 1;
}

// Function: print_stats
void print_stats(void) {
    LogNode *current_node = loglist;
    sendall(1, "--- Log Stats ---\n", strlen("--- Log Stats ---\n"));

    while (current_node) {
        LogEntry *entry = current_node->entry;
        if (entry && entry->request) {
            Request *req = entry->request;
            char log_line[256];
            snprintf(log_line, sizeof(log_line), "Status: 0x%x, Version: 0x%x, Type: 0x%x\n",
                     entry->status_code, req->version, req->type);
            sendall(1, log_line, strlen(log_line));

            if (req->path_ptr && req->path_len != 0) {
                sendall(1, "  Path: ", strlen("  Path: "));
                sendall(1, req->path_ptr, req->path_len);
                sendall(1, "\n", 1);
            }
            if (req->body_ptr && req->body_len != 0) {
                sendall(1, "  Body: ", strlen("  Body: "));
                sendall(1, req->body_ptr, req->body_len);
                sendall(1, "\n", 1);
            }
        }
        current_node = current_node->next;
    }
    sendall(1, "-----------------\n", strlen("-----------------\n"));
}

// Main function (for compilation and demonstration)
int main() {
    srand(time(NULL));

    while (handle_request()) {
        // Continue handling requests
    }

    print_stats();

    while (files) {
        FileNode *next = files->next;
        if (files->file) {
            free(files->file->path_ptr);
            free(files->file->body_ptr);
            free(files->file);
        }
        free(files);
        files = next;
    }

    while (loglist) {
        LogNode *next = loglist->next;
        if (loglist->entry) {
            if (loglist->entry->request) {
                free(loglist->entry->request->path_ptr);
                free(loglist->entry->request->body_ptr);
                free(loglist->entry->request);
            }
            free(loglist->entry);
        }
        free(loglist);
        loglist = next;
    }

    return 0;
}