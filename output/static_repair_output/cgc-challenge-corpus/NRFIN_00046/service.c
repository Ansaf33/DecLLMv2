#include <stdio.h>    // For debugging, if needed
#include <stdlib.h>   // For exit, random
#include <string.h>   // For memset, memcpy
#include <stdint.h>   // For uint32_t, uint16_t, uint8_t, uintptr_t

// --- Global variables ---
// A session entry is 0xc (12) bytes.
typedef struct Session {
    int32_t id;         // Offset 0, used in find_session, compared with tmp
    uint32_t data_ptr;  // Offset 4, used as base pointer for code/data in do_exec
    uint16_t len;       // Offset 8, used as length for code/data in do_exec, also as 'size' in do_init
    uint16_t padding;   // Offset 10, ensures 12-byte struct size
} Session;

// Max 0x13 sessions (19), so array size 20.
#define MAX_SESSIONS 20
Session sessions[MAX_SESSIONS];

uint8_t rx_buf[0x10000]; // Buffer for receiving data
uint32_t tmp;            // Temporary uint32_t variable
int32_t sessions_num;    // Number of active sessions
uint32_t session_window; // Global window value
void *session_dat;       // Pointer to a block of memory for sessions

// --- External function declarations ---
// The 'type' argument (first int) is inferred from decompiler output patterns.
extern int recv_bytes(int type, void *buf, size_t len);
extern int send_bytes(int type, void *buf, size_t len);
extern int allocate(size_t size, int type, void **ptr);
extern int bytecode_exec(void *code_ptr, uint16_t code_len);

// Helper for _terminate (not standard C)
void _terminate(void) {
    exit(1);
}

// Function: find_session
// Finds a session by its ID.
Session *find_session(int32_t session_id) {
    for (unsigned int i = 0; i < MAX_SESSIONS; ++i) {
        if (session_id == sessions[i].id) {
            return &sessions[i];
        }
    }
    return NULL; // Not found
}

// Function: do_init
// Initializes or finds a session based on received data.
int do_init(Session **new_session_ptr) {
    int status_code = 0;
    uint32_t rx_val;

    memset(rx_buf, 0, sizeof(rx_buf));
    
    // First recv_bytes call, populates rx_buf, returns 0 on success.
    // Type 0 assumed for default buffer operation.
    if (recv_bytes(0, rx_buf, sizeof(rx_buf)) != 0) {
        return 0x20; // Error
    }

    rx_val = *(uint32_t*)rx_buf; // Read first 4 bytes of rx_buf

    if (rx_val == 0xe3e3f046) {
        _terminate(); // Exit if magic number found
    }

    if ((rx_val < 0xe0000000) || (0xefffffff < rx_val)) {
        if (rx_val == 0) { // Special case: rx_buf starts with 0
            if (sessions_num < MAX_SESSIONS) { // 0x13 is 19. MAX_SESSIONS is 20.
                // Assuming find_session(0) is used to find an existing session with ID 0
                // or to signify a "new" session that gets ID 0.
                *new_session_ptr = find_session(0);
                if (*new_session_ptr != NULL) {
                    do {
                        int random_check_val = 0; // Value is never updated in snippet.
                                                  // Likely a decompiler artifact or requires external context.
                        long lVar3 = random();
                        if (lVar3 != 0) {
                            return status_code; // status_code is 0 here
                        }
                        if (random_check_val != 4) { // This condition (0 != 4) is always true.
                                                     // This block executes if random() returns 0.
                            return status_code; // status_code is 0 here
                        }
                    } while (((*new_session_ptr)->id == 0) || (((*new_session_ptr)->id > 0xdfffffff) && ((*new_session_ptr)->id < 0xf0000000)));

                    // send_bytes(4, *new_session_ptr, 1) - type 4, pointer to session, length 1
                    if (send_bytes(4, *new_session_ptr, 1) != 0) {
                        return 0x20; // Error
                    }
                    
                    // recv_bytes(0, &tmp, 4) - type 0, buffer &tmp, length 4
                    if (recv_bytes(0, &tmp, 4) != 0) {
                        return 0x20; // Error
                    }
                    
                    if ((*new_session_ptr)->id == (int32_t)tmp) {
                        // recv_bytes(2, &((*new_session_ptr)->len), sizeof(uint16_t)) - type 2, buffer for len, length 2 bytes
                        if (recv_bytes(2, &((*new_session_ptr)->len), sizeof(uint16_t)) != 0) {
                            return 0x20; // Error
                        }
                        
                        if (((*new_session_ptr)->len & 7) == 0) {
                            if ((*new_session_ptr)->len < 0x201) { // 513
                                session_window = session_window + (uint32_t)(-0x103 - (*new_session_ptr)->len);
                                (*new_session_ptr)->data_ptr = session_window; // Assign to data_ptr
                                sessions_num++;
                                status_code = 0;
                            } else {
                                tmp = 0xe33bf5be;
                                if (send_bytes(4, &tmp, 1) != 0) {
                                    return 0x20; // Error
                                }
                                *new_session_ptr = NULL;
                                status_code = 0;
                            }
                        } else {
                            *new_session_ptr = NULL;
                            status_code = 0;
                        }
                    } else { // (*new_session_ptr)->id != tmp
                        tmp = 0xe2ddaa66;
                        if (send_bytes(4, &tmp, 1) != 0) {
                            return 0x20; // Error
                        }
                        *new_session_ptr = NULL;
                        status_code = 0;
                    }
                } // if (*new_session_ptr != NULL)
            } else { // sessions_num >= MAX_SESSIONS
                tmp = 0xe2ea8684;
                if (send_bytes(4, &tmp, 1) != 0) {
                    return 0x20; // Error
                }
                *new_session_ptr = NULL;
                status_code = 0;
            }
        } else { // rx_val != 0
            *new_session_ptr = find_session(rx_val);
            if (*new_session_ptr == NULL) {
                tmp = 0xe9280dfc;
                if (send_bytes(4, &tmp, 1) != 0) {
                    return 0x20; // Error
                }
                status_code = 0;
            } else {
                if (send_bytes(4, *new_session_ptr, 1) != 0) {
                    return 0x20; // Error
                }
                status_code = 0;
            }
        }
    } else { // (rx_val >= 0xe0000000) && (rx_val <= 0xefffffff)
        tmp = 0xe0deade9;
        if (send_bytes(4, &tmp, 1) != 0) {
            return 0x20; // Error
        }
        *new_session_ptr = NULL;
        status_code = 0;
    }
    return status_code;
}

// Function: do_exec
// Executes bytecode for a given session.
int do_exec(Session *exec_session_ptr) {
    int status_code = 0;
    Session *found_session;

    tmp = 0;
    // recv_bytes(0, &tmp, 4) - type 0, buffer &tmp, length 4
    if (recv_bytes(0, &tmp, 4) != 0) {
        return 0x20; // Error
    }

    found_session = find_session(tmp);
    if (found_session == NULL) {
        return 0; // Session not found, return success (0) as in original
    }

    // tmp = (uint)CONCAT12(buf[2], CONCAT11(buf[1], buf[0]));
    // Extracts 3 bytes from the buffer pointed to by exec_session_ptr->data_ptr
    uint8_t *buf_addr = (uint8_t *)(uintptr_t)exec_session_ptr->data_ptr;
    tmp = (uint32_t)buf_addr[2] << 16 | (uint32_t)buf_addr[1] << 8 | (uint32_t)buf_addr[0];

    if (tmp == 0) {
        // Shifts the data at exec_session_ptr->data_ptr right by 8 bits and copies 3 bytes back.
        uint32_t val_at_buf = *(uint32_t*)buf_addr;
        tmp = val_at_buf >> 8;
        memcpy(buf_addr, &tmp, 3);
    }

    // recv_bytes(0, (char*)exec_session_ptr->data_ptr + 3, found_session->len)
    // Type 0, buffer at offset 3, length from found_session.
    if (recv_bytes(0, (char*)(uintptr_t)exec_session_ptr->data_ptr + 3, found_session->len) != 0) {
        return 0x20; // Error
    }

    status_code = bytecode_exec((void*)(uintptr_t)exec_session_ptr->data_ptr, exec_session_ptr->len);
    if (status_code != 0) {
        return status_code; // bytecode_exec failed
    }

    // send_bytes(1, exec_session_ptr, 4) - type 1, pointer to session, length 4
    if (send_bytes(1, exec_session_ptr, 4) != 0) {
        return 0x20; // Error
    }

    // send_bytes(1, (char*)exec_session_ptr->data_ptr + offset, 0x100)
    // The offset calculation is highly ambiguous from the decompiler.
    // Using a simplified offset: exec_session_ptr->len + 3
    if (send_bytes(1, (char*)(uintptr_t)exec_session_ptr->data_ptr + exec_session_ptr->len + 3, 0x100) != 0) {
        return 0x20; // Error
    }

    return status_code; // Should be 0 if everything succeeded
}

// Function: main
int main(void) {
    int status_code = 0;
    int exec_loop_state = 0; // 0 for init phase, 1 for exec phase
    Session *current_session = NULL;
    
    // Decompiler artifact, removed.
    // local_10 = &stack0x00000004; 

    if (allocate(0x3c3c, 0, &session_dat) != 0) {
        return 0x41; // Allocation failed
    }

    session_window = (uintptr_t)session_dat + 0x3c3c; // Calculate session_window address

    while (1) { // Main program loop
        status_code = 0; // Reset status for init phase
        while (exec_loop_state == 0) { // Init phase loop
            current_session = NULL; // Reset current session pointer
            int init_status = do_init(&current_session);
            if (init_status != 0) {
                return init_status; // Exit on init error
            }
            if (current_session != NULL) {
                exec_loop_state = 1; // Transition to exec phase
            }
        }
        
        if (exec_loop_state != 1) { // If state is not 1 after init, something went wrong or exit condition
            break; // Should not happen in normal flow if init was successful
        }

        // do_exec expects Session*
        int exec_status = do_exec(current_session);
        if (exec_status != 0) {
            return exec_status; // Exit on exec error
        }
        exec_loop_state = 0; // Reset state to go back to init phase for next iteration
    }
    
    return status_code; // Should be 0 if loop breaks gracefully
}