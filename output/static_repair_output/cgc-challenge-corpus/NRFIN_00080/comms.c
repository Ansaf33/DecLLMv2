#include <stdlib.h> // For calloc, exit, EXIT_FAILURE
#include <string.h> // For memcpy, memcmp

// Define the Session structure based on the offsets observed in the original code.
// - The 'next' pointer is accessed at offset 0x32 (50 bytes).
// - The total size allocated for a session object is 0x36 (54 bytes),
//   implying a 4-byte pointer for 'next' (0x32 + 4 = 0x36), suggesting a 32-bit environment.
// - The first 0xC (12) bytes of the 'data' are used for comparison in session_remove.
// - Bytes from offset 0xC to 0xC+0xC-1 (12 to 23) of 'data' are used for comparison in session_get_by_username.
// - 0x2e (46) bytes are copied into the 'data' field in session_append.
typedef struct Session {
    char data[0x32]; // 50 bytes for session-specific data
    struct Session *next;   // Pointer to the next session in the list, at offset 0x32
} Session;

// Function: session_append
void session_append(Session **head, const void *data_src) {
    Session *new_session = (Session *)calloc(1, sizeof(Session));
    if (new_session == NULL) {
        exit(EXIT_FAILURE); // Terminate as in original _terminate() behavior
    }

    memcpy(new_session->data, data_src, 0x2e);
    new_session->next = NULL;

    if (*head == NULL) {
        *head = new_session;
    } else {
        Session *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_session;
    }
}

// Function: session_remove
Session *session_remove(Session **head, const void *match_data) {
    if (head == NULL || *head == NULL) {
        return NULL;
    }

    Session *current = *head;
    Session *prev = NULL;

    while (current != NULL) {
        if (memcmp(match_data, current->data, 0xc) == 0) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            current->next = NULL;
            return current;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

// Function: session_get_by_username
Session *session_get_by_username(const Session *head, const void *username_data_container) {
    const Session *current = head;
    while (current != NULL) {
        if (memcmp((const char *)username_data_container + 0xc, current->data + 0xc, 0xc) == 0) {
            return (Session *)current;
        }
        current = current->next;
    }
    return NULL;
}