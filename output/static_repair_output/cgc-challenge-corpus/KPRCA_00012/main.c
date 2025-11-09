#include <stdio.h>   // For printf, perror
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memcpy, strlen
#include <unistd.h>  // For read, write, ssize_t
#include <stdint.h>  // For intptr_t

// --- Mock/Stub Functions (replace with actual implementations if available) ---

// Mock for receive. Original usage implies:
// int receive(int fd, void* buffer, size_t count_to_read, int *actual_bytes_read_out);
// Returns 0 on success, non-zero on error.
static int receive(int fd, void* buffer, size_t count_to_read, int *actual_bytes_read_out) {
    ssize_t bytes_read = read(fd, buffer, count_to_read);
    if (bytes_read == -1) {
        *actual_bytes_read_out = 0;
        perror("receive error");
        return -1; // Error
    }
    *actual_bytes_read_out = (int)bytes_read;
    return 0; // Success
}

// Mock for transmit. Original usage implies:
// int transmit(int fd, const void* buffer, size_t count_to_write, int flags_unused);
// Returns 0 on success, non-zero on error.
static int transmit(int fd, const void* buffer, size_t count_to_write, int flags_unused) {
    ssize_t bytes_written = write(fd, buffer, count_to_write);
    if (bytes_written == -1 || (size_t)bytes_written != count_to_write) {
        perror("transmit error");
        return -1; // Error or partial write
    }
    return 0; // Success
}

// Mock for parse_pkk. Assumes it takes a buffer and its size,
// and returns an integer that represents a pointer to processed data.
// This processed data needs to be freed by free_pkk.
static int parse_pkk(void* buffer, size_t size) {
    // In a real application, this would parse a PKG file format
    // and return a handle or a pointer to processed image data.
    // For this mock, we simulate allocation and return a pointer.
    void* processed_data = malloc(size); // Simulate processing and creating new data
    if (processed_data) {
        memcpy(processed_data, buffer, size); // Copy data (placeholder for actual parsing)
        return (int)(intptr_t)processed_data; // Return pointer cast to int
    }
    return 0; // Error
}

// Mock for output_pkk. Takes an integer handle (a pointer) and returns a newly allocated
// buffer containing the output PKG data, along with its size.
static void* output_pkk(int pkk_handle, size_t *output_size) {
    // Simulate serializing the processed PKG data.
    // For this mock, we'll return a dummy buffer.
    const char* dummy_output = "Mock PKG Output\n";
    size_t len = strlen(dummy_output);
    void* buffer = malloc(len + 1);
    if (buffer) {
        memcpy(buffer, dummy_output, len + 1);
        *output_size = len;
    } else {
        *output_size = 0;
    }
    // In a real scenario, this would use pkk_handle to generate actual output.
    // For now, we ignore pkk_handle for simplicity of the mock.
    (void)pkk_handle; // Suppress unused parameter warning
    return buffer;
}

// Mock for free_pkk. Takes an integer handle (a pointer) and frees the associated memory.
static void free_pkk(int pkk_handle) {
    void* processed_data = (void*)(intptr_t)pkk_handle;
    if (processed_data != NULL) {
        free(processed_data); // Free the memory allocated by parse_pkk
    }
}

// --- Original Functions (fixed) ---

// Function: read_n
// Reads `count` bytes into `buffer` from `fd`, one byte at a time.
// Returns the number of bytes successfully read.
int read_n(int fd, void* buffer, size_t count) {
    size_t bytes_read_total = 0;
    for (size_t i = 0; i < count; ++i) {
        int bytes_received_single;
        if (receive(fd, (char*)buffer + i, 1, &bytes_received_single) != 0 || bytes_received_single == 0) {
            break; // Error or EOF
        }
        bytes_read_total++;
    }
    return (int)bytes_read_total;
}

// Function: readuntil
// Reads bytes into `buffer` from `fd` until `delimiter` is found or `max_count` is reached.
// The delimiter character (if found) is replaced by a null terminator.
// Returns the total number of bytes read (including the replaced delimiter).
int readuntil(int fd, char *buffer, size_t max_count, char delimiter) {
    char *current_ptr = buffer;
    size_t bytes_read_count = 0;
    int bytes_received_single;

    while (bytes_read_count < max_count) {
        if (receive(fd, current_ptr, 1, &bytes_received_single) != 0 || bytes_received_single == 0) {
            break; // Error or EOF
        }
        bytes_read_count++;
        // The original logic advances `local_14` (current_ptr) *after* storing the byte
        // and before checking it. So `current_ptr` always points to the next empty slot.
        // If the current byte (which is `*(current_ptr-1)`) is the delimiter, break.
        if (*current_ptr == delimiter) {
            break;
        }
        current_ptr++;
    }

    if (bytes_read_count > 0) {
        *(current_ptr) = '\0'; // Null-terminate at the position *after* the last read char
    } else {
        *current_ptr = '\0'; // Ensure buffer is null-terminated if nothing was read
    }
    return (int)(current_ptr - buffer);
}

// Define a struct to represent the data parsed by parse_input,
// matching the memory layout indicated by the decompiler output (0x14 bytes total).
// This implies a 32-bit environment or explicit pointer truncation for `int` fields.
typedef struct InputData {
    int total_size;      // offset 0
    int data_len;        // offset 4
    int pkk_handle;      // offset 8 (stores a pointer to processed data)
    short field_C;       // offset 12 (mode: 0x1337 embed, 0x7331 extract)
    short field_E;       // offset 14 (length of message to embed/extract)
    int buffer_ptr_int;  // offset 16 (stores a pointer to the message buffer)
} InputData; // Total size: 4+4+4+2+2+4 = 20 bytes (0x14)

// Function: cleanup_input
void cleanup_input(void *data_ptr) {
    if (data_ptr != NULL) {
        // Free pkk_handle if it's valid
        if (*(int *)((char *)data_ptr + 8) != 0) {
            free_pkk(*(int *)((char *)data_ptr + 8));
        }
        // Free the message buffer if it's valid
        if (*(int *)((char *)data_ptr + 0x10) != 0) {
            free((void*)(intptr_t)*(int *)((char *)data_ptr + 0x10));
        }
        free(data_ptr);
    }
}

// Function: parse_input
// Reads various data fields from standard input to populate an InputData structure.
// Returns a pointer to the allocated InputData structure on success, or NULL on failure.
int * parse_input(void) {
    InputData *data = (InputData *)malloc(sizeof(InputData));
    if (data == NULL) {
        return NULL;
    }

    // Initialize pointers/handles to safe values for cleanup
    data->pkk_handle = 0;
    data->buffer_ptr_int = 0;

    int temp_int_val;    // For reading 4-byte integers
    short temp_short_val; // For reading 2-byte shorts
    size_t bytes_processed_total = 0; // Tracks total bytes read for final checksum

    // Read magic number 1
    if (read_n(0, &temp_int_val, sizeof(int)) != sizeof(int) || temp_int_val != -0x27948b2f) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(int);

    // Read total_size
    if (read_n(0, &temp_int_val, sizeof(int)) != sizeof(int)) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(int);
    data->total_size = temp_int_val;

    // Read magic number 2
    if (read_n(0, &temp_int_val, sizeof(int)) != sizeof(int) || temp_int_val != 0x3259036) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(int);

    // Read data_len (length of the pkk image data)
    if (read_n(0, &temp_int_val, sizeof(int)) != sizeof(int)) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(int);
    data->data_len = temp_int_val;

    // Allocate and read the pkk image data buffer if data_len is valid
    if (data->data_len != 0 && (unsigned int)data->data_len < 0x100000) { // Max size 1MB
        void *pkk_buffer = malloc(data->data_len);
        if (pkk_buffer == NULL) {
            cleanup_input(data);
            return NULL;
        }
        if (read_n(0, pkk_buffer, data->data_len) != data->data_len) {
            free(pkk_buffer); // Free this buffer as parse_pkk wasn't called yet
            cleanup_input(data);
            return NULL;
        }
        data->pkk_handle = parse_pkk(pkk_buffer, data->data_len);
        free(pkk_buffer); // The parse_pkk mock copies, so original buffer can be freed
        if (data->pkk_handle == 0) { // Assuming 0 is an invalid handle
            cleanup_input(data);
            return NULL;
        }
        bytes_processed_total += data->data_len;
    } else if (data->data_len != 0) { // data_len is invalid (too large)
        cleanup_input(data);
        return NULL;
    }

    // Read magic number 3
    if (read_n(0, &temp_int_val, sizeof(int)) != sizeof(int) || temp_int_val != -0x447a58e4) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(int);

    // Read field_C (mode short)
    if (read_n(0, &temp_short_val, sizeof(short)) != sizeof(short)) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(short);
    data->field_C = temp_short_val;

    // Read magic number 4
    if (read_n(0, &temp_int_val, sizeof(int)) != sizeof(int) || temp_int_val != -0x40102217) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(int);

    // Read field_E (message length short)
    if (read_n(0, &temp_short_val, sizeof(short)) != sizeof(short)) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(short);
    data->field_E = temp_short_val;

    // Allocate and read the message buffer if field_E is valid
    if (data->field_E != 0) {
        if (data->field_E > 1000) { // Max message size check
            cleanup_input(data);
            return NULL;
        }
        void *message_buffer = malloc(data->field_E);
        if (message_buffer == NULL) {
            cleanup_input(data);
            return NULL;
        }
        data->buffer_ptr_int = (int)(intptr_t)message_buffer; // Store pointer as int
        if (read_n(0, message_buffer, data->field_E) != data->field_E) {
            cleanup_input(data);
            return NULL;
        }
        bytes_processed_total += data->field_E;
    }

    // Read magic number 5
    if (read_n(0, &temp_int_val, sizeof(int)) != sizeof(int) || temp_int_val != -0x5499f510) {
        cleanup_input(data);
        return NULL;
    }
    bytes_processed_total += sizeof(int);

    // Final check: total bytes read must match total_size field
    if (bytes_processed_total != (unsigned int)data->total_size) {
        cleanup_input(data);
        return NULL;
    }

    return (int *)data; // Return the pointer to the struct
}

// Function: embed_text
// Embeds `text_buffer` of `text_len` into the image data pointed to by `data->pkk_handle`.
// Returns 0 on success, -1 on error.
int embed_text(InputData *data, void *text_buffer, unsigned short text_len) {
    if (data == NULL || text_buffer == NULL || data->pkk_handle == 0) {
        return -1;
    }

    unsigned char *pkk_data_ptr = (unsigned char *)(intptr_t)data->pkk_handle;
    unsigned int required_size = text_len + 10; // Header (4) + Length (2) + Footer (4) = 10 bytes

    // Check if there's enough space in the image data to embed.
    // Each byte of pkk_data_ptr can store 1 bit. So `data->data_len` bytes can store `data->data_len` bits.
    // To store `required_size` bytes (which is `required_size * 8` bits), we need `required_size * 8` bytes in `pkk_data_ptr`.
    if ((unsigned int)data->data_len < required_size * 8) {
        return -1;
    }

    // Allocate a temporary buffer for the message + header/footer
    unsigned char *embedded_msg_buffer = (unsigned char *)malloc(required_size);
    if (embedded_msg_buffer == NULL) {
        return -1;
    }

    // Populate embedded message buffer
    *(unsigned int *)embedded_msg_buffer = 0xb58333c6; // Magic header
    *(unsigned short *)(embedded_msg_buffer + 4) = text_len; // Message length
    memcpy(embedded_msg_buffer + 6, text_buffer, text_len); // Actual message content
    *(unsigned int *)(embedded_msg_buffer + 6 + text_len) = 0x507a018; // Magic footer

    // Embed bit by bit into the LSB of each byte of the image data
    for (size_t i = 0; i < required_size; ++i) {
        unsigned char current_secret_byte = embedded_msg_buffer[i];
        for (int bit_pos = 7; bit_pos >= 0; --bit_pos) { // Iterate from MSB (bit 7) to LSB (bit 0)
            int secret_bit = (current_secret_byte >> bit_pos) & 1;
            if ((*pkk_data_ptr & 1) != secret_bit) { // If LSB of pkk_data byte is different from secret_bit
                if (secret_bit == 0) { // If secret_bit is 0, set LSB of pkk_data to 0
                    *pkk_data_ptr &= 0xfe;
                } else { // If secret_bit is 1, set LSB of pkk_data to 1
                    *pkk_data_ptr |= 1;
                }
            }
            pkk_data_ptr++; // Move to next byte in image data
        }
    }

    free(embedded_msg_buffer); // Free the temporary buffer
    return 0; // Success
}

// Function: recover_byte
// Recovers one byte by extracting 8 LSBs from sequential bytes in `pkk_data_ptr`.
// `pkk_data_ptr` is updated to point to the next byte after extraction.
unsigned char recover_byte(unsigned char **pkk_data_ptr) {
    unsigned char recovered_byte = 0;
    for (int i = 0; i < 8; ++i) {
        recovered_byte = (recovered_byte << 1) | (**pkk_data_ptr & 1);
        (*pkk_data_ptr)++;
    }
    return recovered_byte;
}

// Function: extract_text
// Extracts a hidden message from the image data pointed to by `data->pkk_handle`
// and stores it in `output_buffer`.
// Returns 0 on success, -1 on error.
int extract_text(InputData *data, char *output_buffer) {
    if (data == NULL || output_buffer == NULL || data->pkk_handle == 0) {
        return -1;
    }

    unsigned char *current_pkk_data_ptr = (unsigned char *)(intptr_t)data->pkk_handle;
    unsigned int magic_val = 0;
    unsigned short message_len = 0;
    unsigned char temp_byte;

    // Recover magic header (4 bytes, little-endian)
    for (int i = 0; i < 4; ++i) {
        temp_byte = recover_byte(&current_pkk_data_ptr);
        magic_val |= (unsigned int)temp_byte << (i * 8);
    }

    if (magic_val != 0xb58333c6) {
        return -1; // Header mismatch
    }

    // Recover message length (2 bytes, little-endian)
    for (int i = 0; i < 2; ++i) {
        temp_byte = recover_byte(&current_pkk_data_ptr);
        message_len |= (unsigned short)temp_byte << (i * 8);
    }

    // Recover actual message
    for (int i = 0; i < message_len; ++i) {
        temp_byte = recover_byte(&current_pkk_data_ptr);
        output_buffer[i] = (char)temp_byte;
    }
    output_buffer[message_len] = '\0'; // Null-terminate the extracted string

    // Recover magic footer (4 bytes, little-endian)
    magic_val = 0; // Reset for footer
    for (int i = 0; i < 4; ++i) {
        temp_byte = recover_byte(&current_pkk_data_ptr);
        magic_val |= (unsigned int)temp_byte << (i * 8);
    }

    if (magic_val == 0x507a018) {
        return 0; // Success
    } else {
        return -1; // Footer mismatch
    }
}

// Function: main
int main(void) {
    InputData *data = parse_input();
    if (data == NULL) {
        printf("[ERROR] Failed to parse input.\n");
        return 1; // Return non-zero for error
    }

    int result = 0; // Default success

    // Check the mode specified by field_C
    if (data->field_C == 0x1337) { // Embed mode
        // embed_text(InputData *data_ptr, void *text_buffer, unsigned short text_len)
        if (embed_text(data, (void *)(intptr_t)data->buffer_ptr_int, data->field_E) == 0) {
            void *output_buffer;
            size_t output_size;
            output_buffer = output_pkk(data->pkk_handle, &output_size);
            if (output_buffer != NULL) {
                transmit(1, output_buffer, output_size, 0); // fd 1 for stdout
                free(output_buffer);
            } else {
                printf("[ERROR] Failed to generate output PKG.\n");
                result = 1;
            }
        } else {
            printf("[ERROR] Failed to embed your message.\n");
            result = 1;
        }
    } else if (data->field_C == 0x7331) { // Extract mode
        char secret_text_buffer[1000]; // Max 1000 bytes as per original logic
        // extract_text(InputData *data_ptr, char *output_buffer)
        if (extract_text(data, secret_text_buffer) == 0) {
            printf("Secret Text: %s\n", secret_text_buffer);
        } else {
            printf("[ERROR] Failed to extract the message.\n");
            result = 1;
        }
    } else {
        printf("[ERROR] Invalid mode.\n");
        result = 1;
    }

    cleanup_input(data);
    return result;
}