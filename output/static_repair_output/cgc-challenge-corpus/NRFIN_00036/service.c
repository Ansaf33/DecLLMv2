#include <stdio.h>    // For printf, perror
#include <stdlib.h>   // For exit, malloc, free
#include <string.h>   // For memset, strlen, strcmp, strncpy
#include <unistd.h>   // For read, write, STDIN_FILENO, STDOUT_FILENO
#include <errno.h>    // For errno
#include <stdint.h>   // For uint32_t

// --- Utility Functions (Stubs for original system calls) ---

// Replaces _terminate(int code)
void _terminate(int code) {
    fprintf(stderr, "Error encountered (code %d), terminating.\n", code);
    exit(code);
}

// A simple stub for recvline, reads from a file descriptor until newline or max_len
ssize_t recvline(int fd, char *buf, size_t max_len) {
    ssize_t total_read = 0;
    char c;
    while (total_read < max_len - 1) { // Leave space for null terminator
        ssize_t bytes_read = read(fd, &c, 1);
        if (bytes_read <= 0) {
            if (bytes_read == 0) return total_read; // EOF
            if (errno == EINTR) continue;
            perror("recvline error");
            return -1; // Error
        }
        buf[total_read++] = c;
        if (c == '\n') {
            break;
        }
    }
    buf[total_read] = '\0'; // Null-terminate the string
    return total_read;
}

// A simple stub for transmit_all, sends data to a file descriptor
int transmit_all(const void *buf, size_t len, int fd) {
    const char *ptr = (const char *)buf;
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t bytes_sent = write(fd, ptr + total_sent, len - total_sent);
        if (bytes_sent <= 0) {
            if (bytes_sent == 0) { // EOF, should not happen for write
                return -1;
            }
            if (errno == EINTR) continue;
            perror("transmit_all error");
            return -1;
        }
        total_sent += bytes_sent;
    }
    return 0; // Success
}

// int2str converts an integer to string
// The original uses a buffer of 0x28 (40) bytes.
int int2str(int value, char *buf, size_t buf_len) {
    int ret = snprintf(buf, buf_len, "%d", value);
    if (ret < 0 || (size_t)ret >= buf_len) {
        return -1; // Error or buffer too small
    }
    return 0; // Success
}

// --- Map Data Structure and Operations ---

// MapNode struct reordered to match sendReport's original access pattern
typedef struct MapNode {
    struct MapNode *next; // Corresponds to param_1[0]
    int value;            // Corresponds to param_1[1]
    char key[24];         // Corresponds to param_1[2], 0x18 bytes
} MapNode;

MapNode *global_map_head = NULL; // Global map for simplicity, matching original's 'local_20'

// getSize function for the MapNode list
int getSize(MapNode *head) {
    int count = 0;
    MapNode *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// setMap creates/updates a key-value pair in the map
// Returns 1 if a new entry was created, 0 if an existing one was updated.
int setMap(char *key_str, int val, MapNode **map_head_ptr) {
    MapNode *current = *map_head_ptr;
    MapNode *prev = NULL;

    // Check if key already exists
    while (current != NULL) {
        if (strcmp(current->key, key_str) == 0) {
            current->value = val; // Update existing value
            return 0; // Key updated
        }
        prev = current;
        current = current->next;
    }

    // Key not found, create new node
    MapNode *newNode = (MapNode *)malloc(sizeof(MapNode));
    if (newNode == NULL) {
        perror("Failed to allocate map node");
        _terminate(1);
    }
    strncpy(newNode->key, key_str, sizeof(newNode->key) - 1);
    newNode->key[sizeof(newNode->key) - 1] = '\0'; // Ensure null-termination
    newNode->value = val;
    newNode->next = NULL;

    if (prev == NULL) { // New head node
        *map_head_ptr = newNode;
    } else {
        prev->next = newNode;
    }
    return 1; // New key created
}

// getValue retrieves the value associated with a key
// Returns 0 if key not found (or actual value 0). Consider error handling for not found.
int getValue(char *key_str, MapNode *map_head) {
    MapNode *current = map_head;
    while (current != NULL) {
        if (strcmp(current->key, key_str) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return 0; // Key not found, returning 0
}

// removeMap removes a key-value pair from the map
void removeMap(char *key_str, MapNode **map_head_ptr) {
    MapNode *current = *map_head_ptr;
    MapNode *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->key, key_str) == 0) {
            if (prev == NULL) { // Node to remove is the head
                *map_head_ptr = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// --- Original Functions Refactored ---

// Function: receiveInstruction
void receiveInstruction(uint32_t *instruction) {
    ssize_t bytes_read = read(STDIN_FILENO, instruction, sizeof(uint32_t));
    if (bytes_read < (ssize_t)sizeof(uint32_t)) {
        _terminate(2);
    }
}

// Function: receiveValue
void receiveValue(int *value) {
    ssize_t bytes_read = read(STDIN_FILENO, value, sizeof(int));
    if (bytes_read < (ssize_t)sizeof(int)) {
        _terminate(2);
    }
}

// Function: receiveKey
void receiveKey(char *key_buffer) {
    memset(key_buffer, 0, 0x18); // Clear the buffer as in the original snippet
    ssize_t bytes_read = recvline(STDIN_FILENO, key_buffer, 0x17); // Read up to 0x17 chars
    if (bytes_read < 0) {
        _terminate(2);
    }
    key_buffer[0x17] = '\0'; // Ensure null termination at max length
    // Remove trailing newline if present
    size_t len = strlen(key_buffer);
    if (len > 0 && key_buffer[len - 1] == '\n') {
        key_buffer[len - 1] = '\0';
    }
}

// Function: sendReport
void sendReport(MapNode *head) {
    char str_buffer[40]; // Corresponds to local_44 (0x28 bytes)
    int total_sum = 0;
    MapNode *current = head;

    while (current != NULL) {
        total_sum += current->value;

        // Transmit current->key (0x18 bytes)
        if (transmit_all(current->key, sizeof(current->key), STDOUT_FILENO) != 0) {
            _terminate(1);
        }

        // Convert current->value to string and transmit
        if (int2str(current->value, str_buffer, sizeof(str_buffer)) != 0) {
            _terminate(6); // Original uses 6 for int2str error
        }
        if (transmit_all(str_buffer, sizeof(str_buffer), STDOUT_FILENO) != 0) {
            _terminate(1);
        }

        // Clear the buffer
        memset(str_buffer, 0, sizeof(str_buffer));
        current = current->next;
    }

    // After loop, convert total_sum to string and transmit
    if (int2str(total_sum, str_buffer, sizeof(str_buffer)) != 0) {
        _terminate(6);
    }
    if (transmit_all(str_buffer, sizeof(str_buffer), STDOUT_FILENO) != 0) {
        _terminate(1);
    }
}

// Function: main
int main(void) {
    uint32_t instruction;
    char key_buffer[24]; // Corresponds to local_3c
    int value;           // Corresponds to local_40
    char str_buffer[40]; // For int2str in case 3, corresponds to local_68 to local_44

    // global_map_head is implicitly initialized to NULL

    while (1) { // Replaces the do-while(true) loop with goto
        receiveInstruction(&instruction);

        if (instruction == 1) { // Set map entry
            receiveKey(key_buffer);
            receiveValue(&value);
            if (value < 0) {
                continue; // Original goto LAB_0001182f
            }
            int new_entry = setMap(key_buffer, value, &global_map_head);
            if (new_entry == 1) {
                if (transmit_all("New budget created!\n", 20, STDOUT_FILENO) != 0) { // 0x14 bytes
                    _terminate(1);
                }
            } else {
                if (transmit_all("No more entries\n", 16, STDOUT_FILENO) != 0) { // 0x10 bytes
                    _terminate(1);
                }
            }
        } else if (instruction == 2) { // Update map entry (subtract value)
            receiveKey(key_buffer);
            receiveValue(&value);
            int current_map_value = getValue(key_buffer, global_map_head);
            current_map_value -= value;
            setMap(key_buffer, current_map_value, &global_map_head); // Update value

            if (current_map_value < 0) {
                if (transmit_all(key_buffer, strlen(key_buffer), STDOUT_FILENO) != 0) {
                    _terminate(1);
                }
                if (transmit_all(" is over budget!\n", 17, STDOUT_FILENO) != 0) { // 0x11 bytes
                    _terminate(1);
                }
            }
        } else if (instruction == 3) { // Get value and transmit
            memset(str_buffer, 0, sizeof(str_buffer)); // Clear buffer
            receiveKey(key_buffer);
            int retrieved_value = getValue(key_buffer, global_map_head);
            if (int2str(retrieved_value, str_buffer, sizeof(str_buffer)) != 0) {
                _terminate(6);
            }
            if (transmit_all(str_buffer, sizeof(str_buffer), STDOUT_FILENO) != 0) {
                _terminate(1);
            }
        } else if (instruction == 6) { // Remove map entry if key is "BACON"
            receiveKey(key_buffer);
            if (strcmp(key_buffer, "BACON") == 0) {
                removeMap(key_buffer, &global_map_head);
            }
        } else if (instruction == 7) { // Send report
            sendReport(global_map_head);
        } else if (instruction > 7) { // Exit condition
            break; // Exit the loop
        }
    }

    // Clean up allocated memory before exiting
    MapNode *current = global_map_head;
    while (current != NULL) {
        MapNode *next = current->next;
        free(current);
        current = next;
    }

    return 0;
}