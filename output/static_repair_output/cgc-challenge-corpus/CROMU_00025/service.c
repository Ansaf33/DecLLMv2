#include <stdio.h>      // For printf
#include <string.h>     // For memset, memcpy
#include <stdint.h>     // For uint32_t, uint16_t, uint8_t, uintptr_t
#include <stdlib.h>     // For malloc, free
#include <unistd.h>     // For read, STDIN_FILENO
#include <sys/types.h>  // For ssize_t

// --- Mock Function Declarations and Implementations for External Dependencies ---

// Mock for receive (e.g., from stdin or a file descriptor)
// `fd` is the file descriptor (0 for STDIN_FILENO).
// `buf` is the buffer to read into.
// `len` is the maximum number of bytes to read.
// `bytes_received_out` will store the actual number of bytes read or -1 on error.
// Returns 0 on success, -1 on error.
int receive(int fd, void *buf, size_t len, int *bytes_received_out) {
    ssize_t bytes_read = read(fd, buf, len);
    if (bytes_read == -1) {
        if (bytes_received_out) *bytes_received_out = -1; // Indicate error
        return -1; // Error
    }
    if (bytes_received_out) *bytes_received_out = (int)bytes_read; // Store actual bytes read
    return 0; // Success (0 in original code implies success)
}

// Mock for allocate (e.g., using malloc)
// `size` is the number of bytes to allocate.
// `flags` parameter is ignored for simplicity.
// `out_ptr` will store the allocated address as a uint32_t.
// Returns 0 on success, -1 on error.
int allocate(size_t size, int flags, uint32_t *out_ptr) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        *out_ptr = 0; // Indicate failure
        return -1; // Error
    }
    // Store address. Assuming a 32-bit environment where void* fits into uint32_t.
    // Use uintptr_t for safe conversion, then cast to uint32_t.
    *out_ptr = (uint32_t)(uintptr_t)ptr;
    return 0; // Success
}

// Mock for receive_until used in menu
// Reads characters into buf until max_len, or `terminator`, or EOF.
// Returns number of bytes read (excluding terminator, but including it if max_len reached)
// or 0 on error/EOF before any data.
int receive_until(char *buf, size_t max_len, char terminator) {
    ssize_t total_read = 0;
    char c;
    while (total_read < max_len) {
        ssize_t bytes_read = read(STDIN_FILENO, &c, 1);
        if (bytes_read <= 0) { // EOF or error
            if (total_read == 0) return 0; // No bytes read at all, true error
            break;
        }
        if (c == terminator) {
            break;
        }
        buf[total_read++] = c;
    }
    buf[total_read] = '\0'; // Null-terminate the string
    return (int)total_read;
}

// Mock display functions
// Assume they take a pointer to their specific context struct and return 0 on success, non-zero on failure.
int fpai_display_img(void *ctx) {
    printf("[MOCK] fpai_display_img called.\n");
    return 0;
}
int fpti_display_img(void *ctx) {
    printf("[MOCK] fpti_display_img called.\n");
    return 0;
}
int rpti_display_img(void *ctx) {
    printf("[MOCK] rpti_display_img called.\n");
    return 0;
}
int tbir_display_img(void *ctx) {
    printf("[MOCK] tbir_display_img called.\n");
    return 0;
}
int tpai_display_image(void *ctx) {
    printf("[MOCK] tpai_display_image called.\n");
    return 0;
}

// --- Original Functions with Fixes ---

// Function: readData
int readData(uint32_t *out_data_ptr) {
    uint16_t size_val; // To hold the 2-byte size value
    int bytes_received = 0;
    uint32_t allocated_ptr_val = 0;

    if (out_data_ptr == NULL) {
        return 0;
    }

    // Attempt to receive the 2-byte size
    if (receive(STDIN_FILENO, &size_val, sizeof(size_val), &bytes_received) != 0 || bytes_received != sizeof(size_val)) {
        return 0; // Failed to receive size or incorrect bytes received
    }

    // Check size constraints
    if (size_val == 0 || size_val >= 0x801) { // 0x801 is 2049, as per original logic.
        return 0;
    }

    // Allocate memory
    if (allocate(size_val, 0, &allocated_ptr_val) != 0) {
        return 0; // Failed to allocate memory
    }

    // Receive the actual data
    if (receive(STDIN_FILENO, (void*)(uintptr_t)allocated_ptr_val, size_val, &bytes_received) != 0) {
        // If receive fails, the allocated memory should be freed.
        free((void*)(uintptr_t)allocated_ptr_val);
        return 0;
    }

    // Verify if all expected data was received
    if (bytes_received != size_val) {
        free((void*)(uintptr_t)allocated_ptr_val); // Free on data mismatch
        return 0;
    }

    *out_data_ptr = allocated_ptr_val;
    return bytes_received; // Return actual bytes received (which is size_val)
}

// Struct definitions inferred from selectRenderer's usage
// Assuming a 32-bit environment where sizeof(void*) == 4 bytes
typedef struct {
    uint8_t _padding1[0x10]; // Placeholder for bytes 0x00 to 0x0F
    uint32_t size;           // At offset 0x10
    void* data_ptr;          // At offset 0x14
} ImageInfo;

// Generic base for display contexts
typedef struct {
    void* data;
    uint32_t size;
} ImageRenderContext_Base;

// Specific contexts with padding as implied by memset sizes
// Using char arrays for padding for portability
typedef struct { ImageRenderContext_Base base; char padding[0x20 - sizeof(ImageRenderContext_Base)]; } DisplayContext_fpai;
typedef struct { ImageRenderContext_Base base; char padding[0x14 - sizeof(ImageRenderContext_Base)]; } DisplayContext_fpti;
typedef struct { ImageRenderContext_Base base; char padding[0x10 - sizeof(ImageRenderContext_Base)]; } DisplayContext_rpti;
typedef struct { ImageRenderContext_Base base; char padding[0x30 - sizeof(ImageRenderContext_Base)]; } DisplayContext_tbir;
typedef struct { ImageRenderContext_Base base; char padding[0x20 - sizeof(ImageRenderContext_Base)]; } DisplayContext_tpai;


// Function: selectRenderer
uint32_t selectRenderer(ImageInfo *img_info) {
    if (img_info == NULL || img_info->data_ptr == NULL || img_info->size < 4) {
        return 0; // Invalid input, return 0 as per original logic for early exits
    }

    uint32_t format_magic;
    // Read the first 4 bytes (magic number) from the data_ptr
    memcpy(&format_magic, img_info->data_ptr, sizeof(format_magic));

    int render_result = -1; // Default to unhandled/error

    // Use if-else if for cleaner flow
    if (format_magic == 0x55d9b6de) {
        DisplayContext_fpai ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.base.data = img_info->data_ptr;
        ctx.base.size = img_info->size;
        render_result = fpai_display_img(&ctx);
    } else if (format_magic == (uint32_t)0x85eecc24) { // Original -0x7a1138dc
        DisplayContext_fpti ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.base.data = img_info->data_ptr;
        ctx.base.size = img_info->size;
        render_result = fpti_display_img(&ctx);
    } else if (format_magic == (uint32_t)0xc35109d3) { // Original -0x3caef62d
        DisplayContext_rpti ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.base.data = img_info->data_ptr;
        ctx.base.size = img_info->size;
        render_result = rpti_display_img(&ctx);
    } else if (format_magic == 0x76dfc4b0) {
        DisplayContext_tbir ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.base.data = img_info->data_ptr;
        ctx.base.size = img_info->size;
        render_result = tbir_display_img(&ctx);
    } else if (format_magic == 0x310f59cb) {
        DisplayContext_tpai ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.base.data = img_info->data_ptr;
        ctx.base.size = img_info->size;
        render_result = tpai_display_image(&ctx);
    } else {
        printf("[ERROR] Unknown Format (0x%X)\n", format_magic);
        return 1; // Unhandled format, return 1 as per original logic
    }

    // Original logic returns 0 on success (iVar2 == 0) and 1 otherwise.
    return (render_result == 0) ? 0 : 1;
}

// Function: menu
uint32_t menu(void) {
    char input_buffer[16]; // Sufficient for a digit and newline
    int bytes_read;
    uint32_t choice_val;

    while (1) { // Loop indefinitely until 'Leave' is chosen
        printf("\n--- Main Menu ---\n");
        printf("1- Upload Image\n");
        printf("2- List Images\n");
        printf("3- Remove Image\n");
        printf("4- Display Image\n");
        printf("5- Leave\n");
        printf("} ");

        // Read a single character for the choice, consuming the rest of the line
        bytes_read = receive_until(input_buffer, sizeof(input_buffer) - 1, '\n');

        if (bytes_read <= 0) { // Check for read failure or empty input
            printf("[ERROR] Failed to read choice. Exiting menu.\n");
            return 0; // Indicate menu failure
        }

        // Convert the first character to an integer choice
        choice_val = input_buffer[0] - '0';

        if (choice_val < 1 || choice_val > 5) {
            printf("[ERROR] Invalid option: %d. Please choose between 1 and 5.\n", choice_val);
            continue; // Continue loop for invalid input
        }

        // Valid choice, now perform action
        switch (choice_val) {
            case 1:
                printf("[INFO] Upload Image functionality not implemented.\n");
                // Example usage: uint32_t data_ptr; if (readData(&data_ptr)) { printf("Data uploaded to 0x%X\n", data_ptr); } else { printf("Upload failed.\n"); }
                break;
            case 2:
                printf("[INFO] List Images functionality not implemented.\n");
                break;
            case 3:
                printf("[INFO] Remove Image functionality not implemented.\n");
                break;
            case 4:
                printf("[INFO] Display Image functionality not implemented.\n");
                // Example usage: ImageInfo img_info = { .data_ptr = (void*)some_ptr, .size = some_size }; selectRenderer(&img_info);
                break;
            case 5:
                printf("[INFO] Leaving menu...\n");
                return 1; // Indicate successful exit from menu (user chose to leave)
            default:
                // This case should ideally not be reached due to the validation loop
                printf("[ERROR] Unexpected menu choice: %d.\n", choice_val);
                break;
        }
    }
    // Should not be reached, but for completeness
    return 0;
}

// Function: main
uint32_t main(void) {
    menu();
    return 0;
}