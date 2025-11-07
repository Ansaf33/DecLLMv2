#include <stdio.h>    // For fprintf, stderr
#include <stdlib.h>   // For calloc, free, exit, rand
#include <string.h>   // For strlen, memcpy, strcpy, memcmp, strncpy
#include <unistd.h>   // For ssize_t (send, recv return type)
#include <stdint.h>   // For uint32_t, uintptr_t, uint8_t

// --- Type definitions based on decompiled code patterns ---
typedef uint8_t byte;
typedef uint32_t uint;

// Request structure (size 0x1c)
typedef struct Request {
    uint32_t magic;     // 0x00 (e.g., 0x41414141 for endian check)
    uint32_t version;   // 0x04 (e.g., 0x10000000 for v1, 0x10000001 for v2, etc.)
    uint32_t command;   // 0x08 (e.g., 0x41414141 for retrieve, 0x41414142 for submit)
    uint32_t path_len;  // 0x0c
    uint32_t body_len;  // 0x10
    void *path_ptr;     // 0x14
    void *body_ptr;     // 0x18
} Request;

// Response structure (size 0x10)
typedef struct Response {
    uint32_t magic;     // 0x00 (e.g., 0x41414141 or 0x42424242 for endian check)
    uint32_t status;    // 0x04 (e.g., 0xbaaaaaaa for success, 0xbaaaaaab for internal error)
    uint32_t data_len;  // 0x08
    void *data_ptr;     // 0x0c
} Response;

// File structure (size 0x10)
typedef struct File {
    uint32_t path_len;  // 0x00
    uint32_t body_len;  // 0x04
    void *path_ptr;     // 0x08
    void *body_ptr;     // 0x0c
} File;

// List node structure (size 0xc)
typedef struct ListNode {
    struct ListNode *next; // 0x00
    struct ListNode *prev; // 0x04
    void *data;            // 0x08 (points to Request, File, or other data)
} ListNode;

// Log entry structure (size 0x8)
typedef struct LogEntry {
    uint32_t timestamp; // 0x00 (param_2 from logreq)
    void *request_copy; // 0x04 (pvVar1 from logreq)
} LogEntry;

// --- Global variables and function prototypes (stubs) ---
ListNode *loglist = NULL;
ListNode *files = NULL;
char dolog = 1; // Default to true based on usage
char verified = 0; // Default to false
uint __cookie = 0xdeadbeef; // Placeholder, usually initialized by libc

char *challenges[] = {
    "challenge1",
    "challenge2",
    "challenge3",
    "challenge4"
};

// Placeholder constants from decompilation
const uint32_t REQ_MAGIC_LE = 0x41414141; // Little-endian magic value for request
const uint32_t RES_MAGIC_LE = 0x41414141; // Little-endian magic value for response
const uint32_t RES_MAGIC_BE = 0x42424242; // Big-endian magic value for response
const uint32_t CMD_EXIT = 0x41414146;     // Command to exit
const uint32_t LOG_SUCCESS = 0x4347c000;  // Placeholder address/value for logreq param_2 on success
const uint32_t LOG_FAIL = 0x4347c002;     // Placeholder address/value for logreq param_2 on failure

// Stubs for external functions
void _terminate(void) {
    fprintf(stderr, "Program terminated due to a critical error.\n");
    exit(1);
}

void __stack_cookie_fail(void) {
    fprintf(stderr, "Stack cookie check failed!\n");
    exit(1);
}

ssize_t sendall(int fd, const void *buf, size_t count) {
    // Simplified stub: print to stderr for now.
    // In a real system, this would be a loop calling `send`.
    if (fd == 2) { // Assuming fd 2 is stderr for debug messages
        fprintf(stderr, "%.*s", (int)count, (const char*)buf);
    } else {
        fprintf(stderr, "SENDALL (fd %d, size %zu): %.*s\n", fd, count, (int)count, (const char*)buf);
    }
    return count; // Assume success
}

ssize_t sendline(int fd, const char *msg) {
    size_t len = strlen(msg);
    ssize_t sent = sendall(fd, msg, len);
    if (sent != -1) {
        sendall(fd, "\n", 1);
        return sent + 1;
    }
    return -1;
}

ssize_t recv(int fd, void *buf, size_t count, int flags) {
    // Simplified stub: simulate receiving data.
    // In a real system, this would call `read`.
    // For now, it just fills with zeros or a placeholder if needed for specific tests.
    fprintf(stderr, "RECV (fd %d, size %zu)\n", fd, count);
    if (count > 0) {
        memset(buf, 0, count); // Simulate receiving zeros
    }
    return count; // Assume success for now
}

uint randint(void) {
    // Simple pseudo-random number generator for stub
    return rand();
}

void list_push_back(ListNode **head, ListNode *node) {
    if (!node) return;
    node->next = NULL;
    node->prev = NULL;

    if (!*head) {
        *head = node;
    } else {
        ListNode *current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = node;
        node->prev = current;
    }
}

void list_remove(ListNode **head, ListNode *node) {
    if (!node) return;

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        *head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }

    // Node data is freed by the caller in this specific decompiled code.
    free(node);
}

// Function prototypes for internal functions
byte do_challenge(void);
Response* internalerror_response(void);
Response* notimplemented_response(void);
void logreq(void *req_param, uint32_t log_type);
int get_file(void *path_ptr, uint32_t path_len); // Returns pointer to File struct, or 0
Response* handle_retrieve(Request *req);
void handle_submit(Request *req);
Response* handle_check(Request *req);
void handle_delete(Request *req);
Response* handle_auth(Request *req);
void handle_v1(Request *req);
void handle_v2(Request *req);
void handle_v3(Request *req);
void handle_v4(Request *req);

// --- Fixed functions ---

// Function: logreq
void logreq(void *req_param, uint32_t log_type) {
    Request *req = (Request*)req_param;
    size_t msg_len;

    if (req == NULL) {
        msg_len = strlen("Passed null req\n");
        sendall(2, "Passed null req\n", msg_len);
        return;
    }

    Request *log_req_copy = (Request*)calloc(1, sizeof(Request));
    if (log_req_copy == NULL) {
        msg_len = strlen("Failed to allocate log\n");
        sendall(2, "Failed to allocate log\n", msg_len);
        return;
    }

    memcpy(log_req_copy, req, sizeof(Request));

    // Allocate and copy path if present
    if (req->path_ptr != NULL && req->path_len != 0) {
        log_req_copy->path_ptr = calloc(1, req->path_len);
        if (log_req_copy->path_ptr == NULL) {
            msg_len = strlen("Failed to allocate path\n");
            sendall(2, "Failed to allocate path\n", msg_len);
            free(log_req_copy);
            return;
        }
        memcpy(log_req_copy->path_ptr, req->path_ptr, req->path_len);
    } else {
        log_req_copy->path_ptr = NULL;
        log_req_copy->path_len = 0;
    }

    // Allocate and copy body if present
    if (req->body_ptr != NULL && req->body_len != 0) {
        log_req_copy->body_ptr = calloc(1, req->body_len);
        if (log_req_copy->body_ptr == NULL) {
            msg_len = strlen("Failed to allocate body\n");
            sendall(2, "Failed to allocate body\n", msg_len);
            if (log_req_copy->path_ptr) free(log_req_copy->path_ptr);
            free(log_req_copy);
            return;
        }
        memcpy(log_req_copy->body_ptr, req->body_ptr, req->body_len);
    } else {
        log_req_copy->body_ptr = NULL;
        log_req_copy->body_len = 0;
    }

    LogEntry *log_entry = (LogEntry*)calloc(1, sizeof(LogEntry));
    if (log_entry == NULL) {
        msg_len = strlen("Failed to allocate log entry\n");
        sendall(2, "Failed to allocate log entry\n", msg_len);
        if (log_req_copy->path_ptr) free(log_req_copy->path_ptr);
        if (log_req_copy->body_ptr) free(log_req_copy->body_ptr);
        free(log_req_copy);
        return;
    }

    log_entry->request_copy = log_req_copy;
    log_entry->timestamp = log_type;

    ListNode *node = (ListNode*)calloc(1, sizeof(ListNode));
    if (node == NULL) {
        msg_len = strlen("Failed to allocate node\n");
        sendall(2, "Failed to allocate node\n", msg_len);
        if (log_req_copy->path_ptr) free(log_req_copy->path_ptr);
        if (log_req_copy->body_ptr) free(log_req_copy->body_ptr);
        free(log_req_copy);
        free(log_entry);
        return;
    }

    node->data = log_entry;
    list_push_back(&loglist, node);
}

// Function: get_file
// Returns pointer to File struct, or 0
int get_file(void *path_ptr, uint path_len) {
    if (path_ptr == NULL) return 0;

    ListNode *current_node = files;
    while (current_node != NULL) {
        File *file = (File*)current_node->data;
        if (file == NULL) {
            current_node = current_node->next;
            continue;
        }

        uint compare_len = (path_len < file->path_len) ? path_len : file->path_len;
        if (memcmp(path_ptr, file->path_ptr, compare_len) == 0) {
            return (int)(uintptr_t)file; // Return the address of the File struct
        }
        current_node = current_node->next;
    }
    return 0; // Not found
}

// Function: add_file
uint32_t add_file(File *file_data) {
    if (file_data == NULL) return 0;

    // Check if file already exists
    if (get_file(file_data->path_ptr, file_data->path_len) != 0) {
        return 0; // File already exists
    }

    ListNode *node = (ListNode*)calloc(1, sizeof(ListNode));
    if (node == NULL) {
        return 0; // Failed to allocate node
    }

    node->data = file_data;
    list_push_back(&files, node);
    return 1; // Success
}

// Function: delete_file
uint32_t delete_file(void *path_ptr, uint path_len) {
    if (path_ptr == NULL) return 0;

    ListNode *current_node = files;
    while (current_node != NULL) {
        File *file = (File*)current_node->data;
        if (file == NULL) {
            current_node = current_node->next;
            continue;
        }

        uint compare_len = (path_len < file->path_len) ? path_len : file->path_len;
        if (memcmp(path_ptr, file->path_ptr, compare_len) == 0) {
            // Found the file, now remove it
            list_remove(&files, current_node);
            // Free the file data itself
            if (file->path_ptr) free(file->path_ptr);
            if (file->body_ptr) free(file->body_ptr);
            free(file);
            return 1; // Success
        }
        current_node = current_node->next;
    }
    return 0; // Not found
}

// Function: genericmsg
Response * genericmsg(uint32_t status_code, char *message, size_t msg_len) {
    size_t sVar2;

    Response *resp = (Response*)calloc(1, sizeof(Response));
    if (resp == NULL) {
        sVar2 = strlen("Failed to allocate resp");
        sendall(2, "Failed to allocate resp", sVar2);
        return NULL;
    }

    resp->data_ptr = calloc(1, msg_len);
    if (resp->data_ptr == NULL) {
        sVar2 = strlen("Failed to allocate resp data");
        sendall(2, "Failed to allocate resp data", sVar2);
        free(resp);
        return NULL;
    }

    strncpy((char*)resp->data_ptr, message, msg_len);
    ((char*)resp->data_ptr)[msg_len - 1] = '\0'; // Ensure null termination

    resp->data_len = strlen((char*)resp->data_ptr) + 1;
    resp->status = status_code;
    resp->magic = RES_MAGIC_LE; // Default magic

    return resp;
}

// Function: internalerror_response
Response* internalerror_response(void) {
    Response* resp = genericmsg(0xbaaaaaab, "If anyone asks, you're my debugger.", 0x24);
    if (resp == NULL) {
        _terminate();
    }
    return resp;
}

// Function: notimplemented_response
Response* notimplemented_response(void) {
    Response* resp = genericmsg(0xbaaaaaad, "You want me to do *two* things?!", 0x21);
    if (resp == NULL) {
        _terminate();
    }
    return resp;
}

// Function: handle_retrieve
Response * handle_retrieve(Request *req) {
    size_t msg_len;
    Response *resp = NULL;
    File *file = NULL;

    if (req == NULL) {
        msg_len = strlen("Passed null req\n");
        sendall(2, "Passed null req\n", msg_len);
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }

    file = (File*)(uintptr_t)get_file(req->path_ptr, req->path_len);
    if (file == NULL) {
        msg_len = strlen("Failed to find file.\n");
        sendall(2, "Failed to find file.\n", msg_len);
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }

    resp = (Response*)calloc(1, sizeof(Response));
    if (resp == NULL) {
        msg_len = strlen("Failed to allocate resp.\n");
        sendall(2, "Failed to allocate resp.\n", msg_len);
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }

    resp->data_ptr = calloc(1, file->body_len);
    if (resp->data_ptr == NULL) {
        msg_len = strlen("Failed to allocate resp data");
        sendall(2, "Failed to allocate resp data", msg_len);
        free(resp); // Free response header
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }

    memcpy(resp->data_ptr, file->body_ptr, file->body_len);
    resp->data_len = file->body_len;
    resp->status = 0xbaaaaaaa;
    resp->magic = RES_MAGIC_LE;

    if (dolog != '\0') logreq(req, LOG_SUCCESS);
    return resp;
}

// Function: handle_submit
void handle_submit(Request *req) {
    size_t msg_len;
    File *new_file = NULL;
    uint32_t add_success = 0;
    Response *handler_resp = NULL; // To catch responses from genericmsg/internalerror_response

    do { // Using a do-while(0) loop to simulate early exit on error, without using actual goto
        if (req == NULL) {
            msg_len = strlen("Passed null req\n");
            sendall(2, "Passed null req\n", msg_len);
            handler_resp = internalerror_response();
            break;
        }

        new_file = (File*)calloc(1, sizeof(File));
        if (new_file == NULL) {
            msg_len = strlen("Failed to alloc file.\n");
            sendall(2, "Failed to alloc file.\n", msg_len);
            handler_resp = internalerror_response();
            break;
        }

        new_file->path_ptr = calloc(1, req->path_len);
        if (new_file->path_ptr == NULL) {
            msg_len = strlen("Failed too alloc path.\n");
            sendall(2, "Failed too alloc path.\n", msg_len);
            free(new_file);
            handler_resp = internalerror_response();
            break;
        }

        new_file->body_ptr = calloc(1, req->body_len);
        if (new_file->body_ptr == NULL) {
            msg_len = strlen("Failed to alloc body.\n");
            sendall(2, "Failed to alloc body.\n", msg_len);
            free(new_file->path_ptr);
            free(new_file);
            handler_resp = internalerror_response();
            break;
        }

        new_file->path_len = req->path_len;
        new_file->body_len = req->body_len;
        memcpy(new_file->path_ptr, req->path_ptr, req->path_len);
        memcpy(new_file->body_ptr, req->body_ptr, req->body_len);

        add_success = add_file(new_file);
        if (add_success != 0) {
            if (dolog != '\0') logreq(req, LOG_SUCCESS);
            handler_resp = genericmsg(0xbaaaaaaa, "Oh, sure. Let the robot do all the work.", 0x29);
            return; // Success, no further logging/error handling for submit
        } else {
            // If add_file fails (e.g., file already exists), free allocated memory
            if (new_file->body_ptr) free(new_file->body_ptr);
            if (new_file->path_ptr) free(new_file->path_ptr);
            free(new_file);
            handler_resp = internalerror_response(); // Or a more specific error
            break;
        }
    } while (0);
    
    // Common error logging and response handling
    if (dolog != '\0') logreq(req, LOG_FAIL);
    // The handler_resp from internalerror_response will be discarded here,
    // as handle_submit is void. handle_request will then generate its own default error.
    if (handler_resp) { /* handler_resp would be freed by handle_request's cleanup */ }
}

// Function: handle_check
Response * handle_check(Request *req) {
    size_t msg_len;
    Response *resp = NULL;
    File *file = NULL;

    if (req == NULL) {
        msg_len = strlen("Passed null req\n");
        sendall(2, "Passed null req\n", msg_len);
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }

    msg_len = strlen("in handle check\n");
    sendall(2, "in handle check\n", msg_len);

    file = (File*)(uintptr_t)get_file(req->path_ptr, req->path_len);
    if (file == NULL) {
        msg_len = strlen("Failed to find file.\n");
        sendall(2, "Failed to find file.\n", msg_len);
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }
    
    resp = (Response*)calloc(1, sizeof(Response));
    if (resp == NULL) {
        msg_len = strlen("Failed to allocate resp.\n");
        sendall(2, "Failed to allocate resp.\n", msg_len);
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }

    resp->data_ptr = calloc(1, sizeof(uint32_t)); // Allocate for body_len (uint32_t)
    if (resp->data_ptr == NULL) {
        msg_len = strlen("Failed to allocate resp data");
        sendall(2, "Failed to allocate resp data", msg_len);
        free(resp);
        if (dolog != '\0') logreq(req, LOG_FAIL);
        return internalerror_response();
    }

    msg_len = strlen("found file\n");
    sendall(2, "found file\n", msg_len);

    memcpy(resp->data_ptr, &(file->body_len), sizeof(uint32_t));
    resp->data_len = sizeof(uint32_t);
    resp->status = 0xbaaaaaaa;
    resp->magic = RES_MAGIC_LE;

    if (dolog != '\0') logreq(req, LOG_SUCCESS);
    return resp;
}

// Function: handle_delete
void handle_delete(Request *req) {
    size_t msg_len;
    uint32_t delete_success = 0;
    Response *handler_resp = NULL; // To catch responses

    do {
        if (req == NULL) {
            msg_len = strlen("Passed null req\n");
            sendall(2, "Passed null req\n", msg_len);
            handler_resp = internalerror_response();
            break;
        }

        delete_success = delete_file(req->path_ptr, req->path_len);
        if (delete_success != 0) {
            if (dolog != '\0') logreq(req, LOG_SUCCESS);
            handler_resp = genericmsg(0xbaaaaaaa, "Hopes: deleted.", 0x10);
            return; // Success
        } else {
            handler_resp = internalerror_response(); // Or a more specific error
            break;
        }
    } while (0);

    if (dolog != '\0') logreq(req, LOG_FAIL);
    if (handler_resp) { /* handler_resp would be freed by handle_request's cleanup */ }
}

// Function: handle_auth
Response * handle_auth(Request *req) {
    size_t msg_len;
    Response *resp = NULL;
    File *file = NULL;
    uint32_t local_cookie_copy = __cookie; // Save cookie for comparison

    do {
        if (do_challenge() == '\0') {
            msg_len = strlen("Failed to auth!\n");
            sendall(2, "Failed to auth!\n", msg_len);
            if (req != NULL && dolog != '\0') logreq(req, LOG_FAIL);
            resp = internalerror_response();
            break;
        }

        if (req == NULL) {
            msg_len = strlen("Passed null req\n");
            sendall(2, "Passed null req\n", msg_len);
            if (dolog != '\0') logreq(req, LOG_FAIL);
            resp = internalerror_response();
            break;
        }

        file = (File*)(uintptr_t)get_file(req->path_ptr, req->path_len);
        if (file == NULL) {
            msg_len = strlen("Failed to find file.\n");
            sendall(2, "Failed to find file.\n", msg_len);
            if (dolog != '\0') logreq(req, LOG_FAIL);
            resp = internalerror_response();
            break;
        }

        resp = (Response*)calloc(1, sizeof(Response));
        if (resp == NULL) {
            msg_len = strlen("Failed to allocate resp.\n");
            sendall(2, "Failed to allocate resp.\n", msg_len);
            if (dolog != '\0') logreq(req, LOG_FAIL);
            resp = internalerror_response();
            break;
        }

        resp->data_ptr = calloc(1, file->body_len);
        if (resp->data_ptr == NULL) {
            msg_len = strlen("Failed to allocate resp data");
            sendall(2, "Failed to allocate resp data", msg_len);
            free(resp);
            if (dolog != '\0') logreq(req, LOG_FAIL);
            resp = internalerror_response();
            break;
        }

        byte *temp_buffer = (byte*)malloc(file->body_len);
        if (temp_buffer == NULL) {
            msg_len = strlen("Failed to allocate temporary buffer\n");
            sendall(2, "Failed to allocate temporary buffer\n", msg_len);
            free(resp->data_ptr);
            free(resp);
            if (dolog != '\0') logreq(req, LOG_FAIL);
            resp = internalerror_response();
            break;
        }
        memcpy(temp_buffer, file->body_ptr, file->body_len);

        byte xor_key = randint();
        for (uint i = 0; i < file->body_len; i++) {
            ((byte*)resp->data_ptr)[i] = temp_buffer[i] ^ xor_key;
        }
        free(temp_buffer);

        resp->data_len = file->body_len;
        resp->status = 0xbaaaaaaa;
        resp->magic = RES_MAGIC_LE;

        if (dolog != '\0') logreq(req, LOG_SUCCESS);
    } while (0);

    // Stack cookie check
    if (local_cookie_copy != __cookie) {
        __stack_cookie_fail();
    }
    return resp;
}

// Function: handle_v1
void handle_v1(Request *req) {
    size_t msg_len;
    msg_len = strlen("Got v1 req\n");
    sendall(2, "Got v1 req\n", msg_len);
    Response *handler_resp = NULL; // Capture response if any is generated

    do {
        if (req == NULL) {
            msg_len = strlen("Passed null request\n");
            sendall(2, "Passed null request\n", msg_len);
            handler_resp = internalerror_response();
            break;
        }

        if (req->command == 0x41414141) { // Retrieve
            handler_resp = handle_retrieve(req);
        } else if (req->command == 0x41414142) { // Submit
            handle_submit(req); // This is void, it handles its own error response
        } else {
            if (dolog != '\0') logreq(req, LOG_FAIL);
            handler_resp = notimplemented_response();
        }
    } while (0);
    // If handler_resp is not NULL, it means an error or a specific response was generated.
    // This `Response*` would be freed by the main `handle_request` loop.
    if (handler_resp) { /* free(handler_resp->data_ptr); free(handler_resp); */ }
}

// Function: handle_v2
void handle_v2(Request *req) {
    size_t msg_len;
    msg_len = strlen("Got v2 req\n");
    sendall(2, "Got v2 req\n", msg_len);
    Response *handler_resp = NULL;

    do {
        if (req == NULL) {
            msg_len = strlen("Passed null request\n");
            sendall(2, "Passed null request\n", msg_len);
            handler_resp = internalerror_response();
            break;
        }

        switch (req->command) {
            case 0x41414141: // Retrieve
                handler_resp = handle_retrieve(req);
                break;
            case 0x41414142: // Submit
                handle_submit(req);
                break;
            case 0x41414143: // Check
                handler_resp = handle_check(req);
                break;
            case 0x41414145: // Delete
                handle_delete(req);
                break;
            default:
                if (dolog != '\0') logreq(req, LOG_FAIL);
                handler_resp = notimplemented_response();
                break;
        }
    } while (0);
    if (handler_resp) { /* handler_resp would be freed by handle_request's cleanup */ }
}

// Function: handle_v3
void handle_v3(Request *req) {
    size_t msg_len;
    msg_len = strlen("Got v3 req\n");
    sendall(2, "Got v3 req\n", msg_len);
    Response *handler_resp = NULL;

    do {
        if (req == NULL) {
            msg_len = strlen("Passed null request\n");
            sendall(2, "Passed null request\n", msg_len);
            handler_resp = internalerror_response();
            break;
        }

        switch(req->command) {
            case 0x41414141: // Retrieve
                handler_resp = handle_retrieve(req);
                break;
            case 0x41414142: // Submit
                handle_submit(req);
                break;
            case 0x41414143: // Check
                handler_resp = handle_check(req);
                break;
            case 0x41414144: // Auth
                handler_resp = handle_auth(req);
                break;
            case 0x41414145: // Delete
                handle_delete(req);
                break;
            default:
                if (dolog != '\0') logreq(req, LOG_FAIL);
                handler_resp = notimplemented_response();
                break;
        }
    } while (0);
    if (handler_resp) { /* handler_resp would be freed by handle_request's cleanup */ }
}

// Function: handle_v4
void handle_v4(Request *req) {
    size_t msg_len;
    msg_len = strlen("Got v4 req\n");
    sendall(2, "Got v4 req\n", msg_len);
    Response *handler_resp = NULL;

    do {
        if (dolog != '\0') logreq(req, LOG_FAIL);
        handler_resp = notimplemented_response();
    } while (0);
    if (handler_resp) { /* handler_resp would be freed by handle_request's cleanup */ }
}

// Function: do_challenge
byte do_challenge(void) {
    uint rand_val;
    size_t msg_len;
    ssize_t bytes_received;
    char *challenge_str;
    byte xor_key_seed, xor_key_final;
    uint32_t challenge_num_original;
    size_t challenge_len;
    void *encoded_data = NULL;
    void *decoded_data_recv = NULL;
    uint32_t received_num;
    byte result = 0; // Default to failure

    rand_val = randint();
    challenge_str = challenges[rand_val % (sizeof(challenges) / sizeof(challenges[0]))]; // Use modulo for array bounds
    
    xor_key_seed = randint();
    xor_key_final = randint();
    xor_key_final ^= xor_key_seed; // This is the actual key used for encoding/decoding string

    challenge_num_original = randint(); // This is the number to be sent and received XORed

    challenge_len = strlen(challenge_str);

    encoded_data = calloc(1, challenge_len + 1); // +1 for null terminator if needed
    decoded_data_recv = calloc(1, challenge_len + 1);
    
    if (encoded_data == NULL || decoded_data_recv == NULL) {
        msg_len = strlen("Failed to allocate encode/decode buffers.\n");
        sendall(2, "Failed to allocate encode/decode buffers.\n", msg_len);
        if (encoded_data) free(encoded_data);
        if (decoded_data_recv) free(decoded_data_recv);
        return 0; // Failure
    }

    // Encode challenge string
    for (uint i = 0; i < challenge_len; i++) {
        ((byte*)encoded_data)[i] = ((byte*)challenge_str)[i] ^ xor_key_final;
    }
    ((byte*)encoded_data)[challenge_len] = '\0';

    // Encode challenge number
    uint32_t encoded_challenge_num = challenge_num_original;
    // XOR each byte of the number
    for (uint i = 0; i < sizeof(uint32_t); i++) {
        ((byte*)&encoded_challenge_num)[i] ^= xor_key_final;
    }

    // Send challenge length
    if (sendall(1, &challenge_len, sizeof(size_t)) < 0) {
        _terminate();
    }
    // Send encoded string
    if (sendall(1, encoded_data, challenge_len) < 0) {
        _terminate();
    }
    // Send encoded number
    if (sendall(1, &encoded_challenge_num, sizeof(uint32_t)) < 0) {
        _terminate();
    }

    // Prompt for decoded string
    msg_len = strlen("grab decoded\n");
    sendall(2, "grab decoded\n", msg_len);
    
    // Receive decoded string
    bytes_received = recv(0, decoded_data_recv, challenge_len, 0);
    if (bytes_received < 0) {
        sendline(1, "Please? Thanks? Sorry?");
        _terminate();
    }
    ((byte*)decoded_data_recv)[challenge_len] = '\0'; // Ensure null termination

    // Prompt for decoded number
    msg_len = strlen("grabbed,now dtok\n");
    sendall(2, "grabbed,now dtok\n", msg_len);

    bytes_received = recv(0, &received_num, sizeof(uint32_t), 0);
    if (bytes_received < 0) {
        sendline(1, "Please? Thanks? Sorry?");
        _terminate();
    }

    msg_len = strlen("grabbed all\n");
    sendall(2, "grabbed all\n", msg_len);

    // Compare received decoded string and number
    if (memcmp(challenge_str, decoded_data_recv, challenge_len) == 0 &&
        challenge_num_original == (received_num ^ 0x127a1b76)) {
        result = 1; // Success
    }

    if (encoded_data) free(encoded_data);
    if (decoded_data_recv) free(decoded_data_recv);

    return result;
}

// Function: handle_request
uint32_t handle_request(void) {
    ssize_t bytes_received;
    Request req = {0};
    Response *resp = NULL;
    uint32_t local_cookie_copy = __cookie; // Save cookie for stack protection
    char needs_byte_swap = 0;
    size_t msg_len;
    uint32_t return_status = 1; // Default to success

    dolog = 1; // Set dolog at the beginning of the function

    do { // Outer do-while(0) for structured early exit instead of goto
        // Read initial request header (sizeof(Request) bytes)
        bytes_received = recv(0, &req, sizeof(Request), 0);
        if (bytes_received != sizeof(Request)) {
            msg_len = strlen("Failed to recv request.\n");
            sendall(2, "Failed to recv request.\n", msg_len);
            _terminate(); // Critical error, terminate
        }

        // Endianness check and swap if needed
        if (req.magic != REQ_MAGIC_LE) {
            needs_byte_swap = 1;
            req.version = __builtin_bswap32(req.version);
            req.command = __builtin_bswap32(req.command);
            req.path_len = __builtin_bswap32(req.path_len);
            req.body_len = __builtin_bswap32(req.body_len);
        }

        // Handle special "exit" command
        if (req.command == CMD_EXIT) {
            sendall(1, "4", 1); // Send a single byte '4'
            return_status = 0; // Indicate program exit
            break; // Exit do-while
        }

        // Check bounds for path_len and body_len
        if (req.path_len >= 0x101 || req.body_len >= 0x200001) {
            resp = internalerror_response();
            break; // Exit do-while
        }

        // Challenge verification if not already verified
        if (verified == '\0') {
            if (do_challenge() == '\0') {
                if (dolog != '\0') logreq(&req, LOG_FAIL);
                resp = internalerror_response();
                break; // Exit do-while
            }
            verified = '\x01';
        }

        // Read path data if present
        if (req.path_len != 0) {
            req.path_ptr = calloc(1, req.path_len);
            if (req.path_ptr == NULL) {
                msg_len = strlen("Failed to allocate path.\n");
                sendall(2, "Failed to allocate path.\n", msg_len);
                _terminate(); // Critical error, terminate
            }
            bytes_received = recv(0, req.path_ptr, req.path_len, 0);
            if (bytes_received < 0) {
                sendline(1, "Please? Thanks? Sorry?");
                _terminate(); // Critical error, terminate
            }
        }

        // Read body data if present
        if (req.body_len != 0) {
            req.body_ptr = calloc(1, req.body_len);
            if (req.body_ptr == NULL) {
                msg_len = strlen("Failed to allocate body.\n");
                sendall(2, "Failed to allocate body.\n", msg_len);
                _terminate(); // Critical error, terminate
            }
            bytes_received = recv(0, req.body_ptr, req.body_len, 0);
            if (bytes_received < 0) {
                sendline(1, "Please? Thanks? Sorry?");
                _terminate(); // Critical error, terminate
            }
        }

        // Dispatch based on version.
        // Handlers that return Response* will set `resp`.
        // Handlers that return void (like submit, delete, and v1-v4) might call `genericmsg`
        // internally, but their return value won't be captured directly by `resp`.
        // If `resp` is still NULL after dispatch, `internalerror_response` will be called.
        switch (req.version) {
            case 0x10000000: // v1
                handle_v1(&req);
                break;
            case 0x10000001: // v2
                handle_v2(&req);
                break;
            case 0x10000002: // v3
                handle_v3(&req);
                break;
            case 0x10000003: // v4
                handle_v4(&req);
                break;
            default:
                if (dolog != '\0') logreq(&req, LOG_FAIL);
                resp = notimplemented_response();
                break;
        }

    } while (0); // End of outer do-while(0)

    // Stack cookie check
    if (local_cookie_copy != __cookie) {
        __stack_cookie_fail();
    }

    // If a non-exit return status is set, proceed with response and cleanup
    if (return_status != 0) {
        // If no specific response was generated by handlers, default to internal error
        if (resp == NULL) {
            resp = internalerror_response();
        }
        
        // Adjust response magic based on original request endianness
        if (needs_byte_swap) {
            resp->magic = RES_MAGIC_BE; // Set big-endian magic for response
            resp->status = __builtin_bswap32(resp->status);
            resp->data_len = __builtin_bswap32(resp->data_len);
        } else {
            resp->magic = RES_MAGIC_LE; // Set little-endian magic for response
        }
        
        // Send response header (first 0xc bytes of Response)
        if (sendall(1, resp, sizeof(Response) - sizeof(void*)) < 0) {
            _terminate();
        }

        // Send response data
        if (resp->data_len > 0 && resp->data_ptr != NULL) {
            if (sendall(1, resp->data_ptr, resp->data_len) < 0) {
                _terminate();
            }
        }

        // Free allocated request data
        if (req.path_ptr != NULL) {
            msg_len = strlen("freeing path\n");
            sendall(2, "freeing path\n", msg_len);
            free(req.path_ptr);
        }
        if (req.body_ptr != NULL) {
            msg_len = strlen("freeing body\n");
            sendall(2, "freeing body\n", msg_len);
            free(req.body_ptr);
        }

        // Free allocated response data
        if (resp->data_ptr != NULL) {
            msg_len = strlen("freeing resp data\n");
            sendall(2, "freeing resp data\n", msg_len);
            free(resp->data_ptr);
        }
        if (resp != NULL) {
            msg_len = strlen("freeing resp\n");
            sendall(2, "freeing resp\n", msg_len);
            free(resp);
        }
    }

    return return_status;
}

// Function: print_stats
void print_stats(void) {
    ListNode *current_node = loglist;
    size_t msg_len;

    while (current_node != NULL) {
        LogEntry *entry = (LogEntry*)current_node->data;
        if (entry == NULL) {
            current_node = current_node->next;
            continue;
        }
        Request *logged_req = (Request*)entry->request_copy;

        if (logged_req != NULL && logged_req->path_ptr != NULL && logged_req->path_len != 0) {
            // Send path_len (4 bytes)
            if (sendall(1, &(logged_req->path_len), sizeof(uint32_t)) < 0) {
                _terminate();
            }
            // Send path data
            if (sendall(1, logged_req->path_ptr, logged_req->path_len) < 0) {
                _terminate();
            }
        }
        current_node = current_node->next;
    }
}