#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h> // For read, write
#include <sys/socket.h> // For send flags (MSG_*)
#include <time.h> // For srand, rand

// --- Error codes ---
#define ERROR_HAIKU_LIST_NOT_EXISTS -2174 // 0xfffff752
#define ERROR_HAIKU_LIST_EMPTY -1023    // 0xfffffba9
#define ERROR_RECV_ALL_FAILED -911      // 0xfffffc71
#define ERROR_HAIKU_NODE_NULL -2345     // 0xfffff6d7
#define ERROR_HAIKU_NOT_FOUND -3456     // 0xfffff280
#define ERROR_INVALID_HAIKU_SIZE -4567  // 0xffffee29
#define ERROR_HAIKU_ALLOC_FAILED -4568  // 0xffffee28
#define ERROR_RECVLINE_FAILED -4569     // Derived from -0x11d8
#define EASTER_EGG_ID 0x7a69

// --- Data Structures ---
// Haiku data format: ID (uint32_t), Size (uint16_t), Text (char[])
typedef struct {
    uint32_t id;
    uint16_t size; // Length of the text, excluding null terminator
    char text[];    // Flexible array member for haiku text
} haiku_t;

typedef struct haiku_list_node {
    haiku_t* haiku;
    struct haiku_list_node* next;
} haiku_list_node_t;

typedef struct {
    haiku_list_node_t* head;
    haiku_list_node_t* tail;
    int count;
} haiku_list_t;

// --- Global Variables ---
haiku_list_t* haiku_list = NULL;
int next_haiku_id = 1;
int global_socket_fd = 0; // Default to stdin for receiving, stdout for sending

// --- Dummy Functions (replace with actual implementations if available) ---

// Replaces _terminate
void _terminate(int error_code) {
    fprintf(stderr, "Program terminated with error code: 0x%x\n", error_code);
    exit(EXIT_FAILURE);
}

// Dummy recv_all: Simulates reading exactly 'len' bytes
// In a real scenario, this would loop until all bytes are received or an error occurs.
int recv_all(void* buf, size_t len) {
    ssize_t bytes_read = 0;
    ssize_t total_read = 0;
    while (total_read < len) {
        bytes_read = read(global_socket_fd, (char*)buf + total_read, len - total_read);
        if (bytes_read <= 0) { // EOF or error
            if (bytes_read < 0) perror("recv_all read error");
            return (int)bytes_read; // Return actual bytes read or error
        }
        total_read += bytes_read;
    }
    return (int)total_read;
}

// Dummy send: Uses write to stdout
ssize_t send_dummy(int fd, const void* buf, size_t len, int flags) {
    (void)flags; // flags not used in dummy
    ssize_t bytes_written = write(fd, buf, len);
    if (bytes_written < 0) {
        perror("send_dummy write error");
    }
    return bytes_written;
}
#define send send_dummy // Alias send to send_dummy

// Dummy list_create
haiku_list_t* list_create(void) {
    haiku_list_t* new_list = (haiku_list_t*)malloc(sizeof(haiku_list_t));
    if (new_list) {
        new_list->head = NULL;
        new_list->tail = NULL;
        new_list->count = 0;
    }
    return new_list;
}

// Dummy node_create
haiku_list_node_t* node_create(haiku_t* haiku) {
    haiku_list_node_t* new_node = (haiku_list_node_t*)malloc(sizeof(haiku_list_node_t));
    if (new_node) {
        new_node->haiku = haiku;
        new_node->next = NULL;
    }
    return new_node;
}

// Dummy list_push
void list_push(haiku_list_t* list, haiku_list_node_t* node) {
    if (!list || !node) return;
    if (list->tail) {
        list->tail->next = node;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
    }
    list->count++;
}

// Dummy allocate (for haiku_t and ID arrays)
void* allocate(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        perror("Memory allocation failed");
    }
    return ptr;
}

// Dummy deallocate
void deallocate(void* ptr) {
    free(ptr);
}

// Dummy recvline: reads until newline or max_len-1, null-terminates.
// Returns length of string (excluding null terminator) or -1 on error.
ssize_t recvline(int fd, char* buffer, size_t max_len) {
    if (max_len == 0) return 0;
    size_t i = 0;
    char c;
    while (i < max_len - 1) {
        ssize_t bytes_read = read(fd, &c, 1);
        if (bytes_read <= 0) { // EOF or error
            if (bytes_read < 0) perror("recvline read error");
            if (i == 0) return bytes_read; // Error or EOF on first char
            break; // EOF after some chars read, break and null-terminate
        }
        if (c == '\n') {
            break;
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i;
}

// Dummy global string for easter egg
char DAT_00013000[] = " from within HIGHCOO\nnew Haiku every login\na warm thought engulfs\a";

// --- Original functions, refactored ---

uint32_t recv_uint32(void) {
    uint32_t val = 0;
    if (recv_all(&val, sizeof(val)) != sizeof(val)) {
        _terminate(ERROR_RECV_ALL_FAILED);
    }
    return val;
}

uint16_t recv_uint16(void) {
    uint16_t val = 0;
    if (recv_all(&val, sizeof(val)) != sizeof(val)) {
        _terminate(ERROR_RECV_ALL_FAILED);
    }
    return val;
}

bool haiku_list_exists(void) {
    return haiku_list != NULL;
}

bool is_haiku_list_empty(void) {
    return haiku_list_exists() && haiku_list->count == 0;
}

void init_haiku_list(void) {
    if (!haiku_list_exists()) {
        haiku_list = list_create();
        if (!haiku_list) {
            _terminate(ERROR_HAIKU_ALLOC_FAILED); // Or a specific list error
        }
    }
}

int get_count(void) {
    if (haiku_list_exists()) {
        return haiku_list->count;
    }
    return ERROR_HAIKU_LIST_NOT_EXISTS;
}

int get_id_from_haiku(const haiku_t *haiku) {
    if (haiku == NULL) {
        return ERROR_HAIKU_NODE_NULL;
    }
    return haiku->id;
}

int get_next_haiku_id(void) {
    return next_haiku_id++;
}

int get_random_idx(unsigned int *random_idx) {
    int count = get_count();
    if (count < 0) { // Error from get_count
        return count;
    }
    if (count == 0) {
        return ERROR_HAIKU_LIST_EMPTY;
    }
    *random_idx = rand() % count;
    return 0; // Success
}

int populate_array_with_haiku_ids(uint32_t *id_array, int max_count) {
    if (!haiku_list_exists()) {
        return ERROR_HAIKU_LIST_NOT_EXISTS;
    }
    if (max_count == 0 || haiku_list->head == NULL) {
        return ERROR_HAIKU_LIST_EMPTY;
    }

    haiku_list_node_t *current_node = haiku_list->head;
    int current_idx = max_count - 1; // Fill from end to beginning as implied by original code
    while (current_node != NULL && current_idx >= 0) {
        int id = get_id_from_haiku(current_node->haiku);
        if (id < 0) return id; // Handle error from get_id_from_haiku
        id_array[current_idx--] = id;
        current_node = current_node->next;
    }
    return 0;
}

int find_haiku_with_id(haiku_t **found_haiku_ptr, uint32_t target_id) {
    int count = get_count();
    if (count < 0) {
        return count; // Error from get_count (list not exists)
    }
    if (count == 0) {
        return ERROR_HAIKU_LIST_EMPTY;
    }

    haiku_list_node_t *current_node = haiku_list->head;
    while (current_node != NULL) {
        if (target_id == current_node->haiku->id) {
            *found_haiku_ptr = current_node->haiku;
            return 0; // Found
        }
        current_node = current_node->next;
    }
    return ERROR_HAIKU_NOT_FOUND;
}

void send_haiku(const haiku_t *haiku) {
    if (!haiku) return; // Should not happen if find_haiku_with_id was successful
    send(global_socket_fd, &haiku->id, sizeof(haiku->id), 0);
    send(global_socket_fd, &haiku->size, sizeof(haiku->size), 0);
    send(global_socket_fd, haiku->text, haiku->size, 0);
}

void send_easter_egg_haiku(void) {
    uint32_t id = EASTER_EGG_ID;
    size_t text_len = strlen(DAT_00013000);
    uint16_t size = (uint16_t)text_len; // Truncate if > 65535, but unlikely for a haiku

    send(global_socket_fd, &id, sizeof(id), 0);
    send(global_socket_fd, &size, sizeof(size), 0);
    send(global_socket_fd, DAT_00013000, size, 0);
}

void send_haiku_id(uint32_t haiku_id) {
    send(global_socket_fd, &haiku_id, sizeof(haiku_id), 0);
}

int add_haiku_to_list(haiku_t *haiku) {
    haiku_list_node_t* new_node = node_create(haiku);
    if (!new_node) {
        deallocate(haiku); // Free haiku data if node creation fails
        return ERROR_HAIKU_ALLOC_FAILED;
    }
    list_push(haiku_list, new_node);
    return 0; // Success
}

// Reads a line from global_socket_fd into a temporary buffer, then copies it to `buffer`.
// `buffer` is the destination in the haiku_t struct.
// `max_len_short` is the available space in `buffer` (including null terminator).
// Returns actual bytes copied (excluding null terminator), or error.
short recv_haiku_line(char *buffer, short max_len_short) {
    char temp_line_buffer[0x552]; // Max line length from original code (1362 bytes)
    size_t max_len = (size_t)max_len_short;
    
    ssize_t bytes_read = recvline(global_socket_fd, temp_line_buffer, sizeof(temp_line_buffer));
    if (bytes_read < 0) {
        return (short)ERROR_RECVLINE_FAILED;
    }
    
    // Original snprintf format "~c\a" is ambiguous. Assuming it's meant to copy the string.
    // The original `snprintf` also implicitly adds a null terminator.
    int snprintf_res = snprintf(buffer, max_len, "%s", temp_line_buffer);
    if (snprintf_res < 0) {
        return (short)ERROR_RECVLINE_FAILED; // snprintf error
    }
    
    short copied_len = (short)snprintf_res;
    if (copied_len >= max_len) {
        // String was truncated or exactly filled buffer.
        // Ensure null termination and return `max_len - 1` as the actual copied length.
        copied_len = max_len_short - 1;
        buffer[copied_len] = '\0';
    }
    
    return copied_len;
}

haiku_t * recv_haiku(uint16_t expected_size) {
    haiku_t *new_haiku = (haiku_t *)allocate(sizeof(haiku_t) + expected_size + 1);
    if (!new_haiku) {
        return NULL;
    }

    new_haiku->id = get_next_haiku_id();
    new_haiku->size = expected_size;

    char *text_write_ptr = new_haiku->text;
    uint16_t remaining_size = expected_size;

    while (remaining_size > 0) {
        // `recv_haiku_line` reads a line into its internal buffer, then copies it to `text_write_ptr`.
        // The `max_len_short` argument to `recv_haiku_line` is `remaining_size + 1` (for null terminator).
        short bytes_copied_this_line = recv_haiku_line(text_write_ptr, (short)remaining_size + 1);

        if (bytes_copied_this_line < 0) {
            deallocate(new_haiku);
            return NULL; // Error reading line
        }
        
        // If no bytes were copied but remaining_size > 0, it means an empty line was read.
        // To prevent an infinite loop if `recv_haiku_line` makes no progress, break.
        if (bytes_copied_this_line == 0) {
            break;
        }

        // Advance pointer and reduce remaining size
        text_write_ptr += bytes_copied_this_line;
        remaining_size -= bytes_copied_this_line;
    }
    *text_write_ptr = '\0'; // Ensure final null termination
    
    return new_haiku;
}

uint16_t recv_haiku_size(void) {
    return recv_uint16();
}

int add_haiku(void) {
    if (!haiku_list_exists()) {
        init_haiku_list();
    }

    uint16_t haiku_size = recv_haiku_size();
    if (haiku_size == 0 || haiku_size > 0xff6) { // 0xff6 is 2038
        return ERROR_INVALID_HAIKU_SIZE;
    }

    haiku_t *new_haiku = recv_haiku(haiku_size);
    if (new_haiku == NULL) {
        return ERROR_HAIKU_ALLOC_FAILED; // Or error from recv_haiku
    }

    int result = add_haiku_to_list(new_haiku);
    if (result == 0) {
        send_haiku_id(new_haiku->id);
    } else {
        deallocate(new_haiku); // Free haiku data if adding to list failed
    }
    return result;
}

int get_haiku_by_id(void) {
    if (!haiku_list_exists()) {
        return ERROR_HAIKU_LIST_NOT_EXISTS;
    }

    uint32_t target_id = recv_uint32();
    if (target_id == EASTER_EGG_ID) {
        send_easter_egg_haiku();
        return 0; // Success for easter egg
    }

    haiku_t *found_haiku = NULL;
    int result = find_haiku_with_id(&found_haiku, target_id);
    if (result == 0) {
        send_haiku(found_haiku);
    }
    return result;
}

int get_haiku_random(void) {
    if (!haiku_list_exists()) {
        return ERROR_HAIKU_LIST_NOT_EXISTS;
    }

    int count = get_count();
    if (count == 0) {
        return ERROR_HAIKU_LIST_EMPTY;
    }

    size_t array_size = count * sizeof(uint32_t);
    uint32_t *haiku_ids = (uint32_t *)allocate(array_size);
    if (!haiku_ids) {
        _terminate(ERROR_HAIKU_ALLOC_FAILED); // Original called _terminate directly
    }

    int result = populate_array_with_haiku_ids(haiku_ids, count);
    if (result != 0) {
        deallocate(haiku_ids);
        return result;
    }

    unsigned int random_idx;
    result = get_random_idx(&random_idx);
    if (result != 0) {
        deallocate(haiku_ids);
        return result;
    }

    uint32_t target_id = haiku_ids[random_idx];
    haiku_t *found_haiku = NULL;
    result = find_haiku_with_id(&found_haiku, target_id);
    
    deallocate(haiku_ids); // Free the temporary array

    if (result == 0) {
        send_haiku(found_haiku);
    }
    return result;
}

int get_haiku_count(void) {
    if (!haiku_list_exists()) {
        return ERROR_HAIKU_LIST_NOT_EXISTS;
    }
    uint32_t count_val = get_count();
    send(global_socket_fd, &count_val, sizeof(count_val), 0);
    return 0; // Success
}

int get_haiku_ids(void) {
    if (!haiku_list_exists()) {
        return ERROR_HAIKU_LIST_NOT_EXISTS;
    }

    uint32_t count = get_count();
    send(global_socket_fd, &count, sizeof(count), 0);

    haiku_list_node_t *current_node = haiku_list->head;
    while (current_node != NULL) {
        uint32_t id = get_id_from_haiku(current_node->haiku);
        if (id < 0) return id; // Handle error
        send(global_socket_fd, &id, sizeof(id), 0);
        current_node = current_node->next;
    }
    return 0; // Success
}

// Dummy main function to make it compilable
int main() {
    srand(time(NULL)); // Seed random number generator

    // For testing, you might want to redirect stdin/stdout
    // global_socket_fd = STDOUT_FILENO; // For sending to console
    // global_socket_fd = STDIN_FILENO;  // For receiving from console

    return 0;
}