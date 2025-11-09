#include <stdio.h>    // For printf
#include <string.h>   // For memcpy, memset
#include <stdint.h>   // For uint32_t, uint16_t, uint8_t

// --- Type definitions for decompiled types ---
typedef uint32_t undefined4;
typedef uint8_t undefined;

// --- Forward declarations for external functions ---
// These declarations are assumptions based on their usage in the snippet.
// The actual signatures might vary.
int receive(int fd, void *buf, size_t len, int *bytes_received);
int allocate(size_t size, int flags, uint32_t *out_ptr);
int fpai_display_img(void *img_data);
int fpti_display_img(void *img_data);
int rpti_display_img(void *img_data);
int tbir_display_img(void *img_data);
int tpai_display_image(void *img_data);
int receive_until(char *buffer, char delimiter, size_t max_len);

// Function: readData
int readData(uint32_t *out_buffer_ptr) {
    uint16_t data_size_short = 0; // To correctly read 2-byte size
    int temp_bytes_read_len = 0;
    void *allocated_buffer = NULL;

    if (out_buffer_ptr == NULL) {
        return 0;
    }

    // Receive the size of the data (expected to be 2 bytes)
    // The original code implies a 16-bit size due to the `0x801` limit.
    if (receive(0, &data_size_short, sizeof(uint16_t), &temp_bytes_read_len) != 0 || temp_bytes_read_len != sizeof(uint16_t)) {
        return 0; // Failed to receive size or received an incorrect number of bytes
    }

    int data_size = (int)data_size_short; // Promote to int for subsequent operations

    // Validate the received size
    if (data_size == 0 || data_size >= 0x801) { // Max size 2048
        return 0; // Invalid or excessively large size
    }

    // Allocate memory for the incoming data
    if (allocate(data_size, 0, (uint32_t*)&allocated_buffer) != 0 || allocated_buffer == NULL) {
        return 0; // Allocation failed
    }

    // Receive the actual data into the allocated buffer
    if (receive(0, allocated_buffer, data_size, &temp_bytes_read_len) != 0 || temp_bytes_read_len != data_size) {
        // In a real scenario, `free(allocated_buffer)` would be called here.
        // Omitted as `free` is not part of the snippet.
        return 0; // Failed to receive data or received an incorrect number of bytes
    }

    // On success, return the pointer to the allocated data and the size
    *out_buffer_ptr = (uint32_t)allocated_buffer;
    return data_size;
}

// --- Dummy structs for display functions based on memset sizes and assignments ---
// These structs are inferred from the `memset` calls and subsequent assignments
// to `ptr` and `size` fields, followed by passing their address to `_display_img`.
// The padding ensures the struct's size matches the `memset` length.
typedef struct {
    void *ptr;
    uint32_t size;
    uint8_t _padding[32 - sizeof(void*) - sizeof(uint32_t)]; // 0x20 bytes
} FpaiImgData;

typedef struct {
    void *ptr;
    uint32_t size;
    uint8_t _padding[20 - sizeof(void*) - sizeof(uint32_t)]; // 0x14 bytes
} FptiImgData;

typedef struct {
    void *ptr;
    uint32_t size;
    uint8_t _padding[16 - sizeof(void*) - sizeof(uint32_t)]; // 0x10 bytes
} RptiImgData;

typedef struct {
    void *ptr;
    uint32_t size;
    uint8_t _padding[48 - sizeof(void*) - sizeof(uint32_t)]; // 0x30 bytes
} TbirImgData;

typedef struct {
    void *ptr;
    uint32_t size;
    uint8_t _padding[32 - sizeof(void*) - sizeof(uint32_t)]; // 0x20 bytes
} TpaiImgData;

// Function: selectRenderer
undefined4 selectRenderer(void *param_1_void_ptr) {
    if (param_1_void_ptr == NULL) {
        return 1; // Indicate error
    }

    // Cast param_1 to a byte pointer for offset arithmetic
    uint8_t *param_bytes = (uint8_t *)param_1_void_ptr;

    // Extract image_data_ptr and image_data_size from the structure pointed to by param_1
    void *image_data_ptr = *(void **)(param_bytes + 0x14);
    uint32_t image_data_size = *(uint32_t *)(param_bytes + 0x10);

    if (image_data_ptr == NULL || image_data_size < sizeof(uint32_t)) { // Minimum size for magic number
        return 1; // Indicate error
    }

    // Read the magic number (first 4 bytes of image data)
    uint32_t magic_number;
    memcpy(&magic_number, image_data_ptr, sizeof(uint32_t));

    int display_result; // 0 for success, non-zero for failure from display functions

    // Dispatch based on magic number
    if (magic_number == 0x55d9b6de) { // FPAI
        FpaiImgData img_data_struct;
        memset(&img_data_struct, 0, sizeof(FpaiImgData));
        img_data_struct.ptr = image_data_ptr;
        img_data_struct.size = image_data_size;
        display_result = fpai_display_img(&img_data_struct);
        return (display_result == 0) ? 0 : 1; // Return 0 on display success, 1 otherwise
    } else if (magic_number == (uint32_t)0x85eecc24) { // FPTI (-0x7a1138dc as uint32_t)
        FptiImgData img_data_struct;
        memset(&img_data_struct, 0, sizeof(FptiImgData));
        img_data_struct.ptr = image_data_ptr;
        img_data_struct.size = image_data_size;
        display_result = fpti_display_img(&img_data_struct);
        return (display_result == 0) ? 0 : 1;
    } else if (magic_number == (uint32_t)0xc35109d3) { // RPTI (-0x3caef62d as uint32_t)
        RptiImgData img_data_struct;
        memset(&img_data_struct, 0, sizeof(RptiImgData));
        img_data_struct.ptr = image_data_ptr;
        img_data_struct.size = image_data_size;
        display_result = rpti_display_img(&img_data_struct);
        return (display_result == 0) ? 0 : 1;
    } else if (magic_number == 0x76dfc4b0) { // TBIR
        TbirImgData img_data_struct;
        memset(&img_data_struct, 0, sizeof(TbirImgData));
        img_data_struct.ptr = image_data_ptr;
        img_data_struct.size = image_data_size;
        display_result = tbir_display_img(&img_data_struct);
        return (display_result == 0) ? 0 : 1;
    } else if (magic_number == 0x310f59cb) { // TPAI
        TpaiImgData img_data_struct;
        memset(&img_data_struct, 0, sizeof(TpaiImgData));
        img_data_struct.ptr = image_data_ptr;
        img_data_struct.size = image_data_size;
        display_result = tpai_display_image(&img_data_struct);
        return (display_result == 0) ? 0 : 1;
    } else {
        printf("[ERROR] Unknown Format\n");
        return 1; // Unknown format is an error
    }
}

// Function: menu
undefined4 menu(void) {
    char input_buffer[4]; // Buffer for user input (e.g., "1\n\0")
    int choice_val;
    int bytes_read;

    while (1) {
        printf("1- Upload Image\n");
        printf("2- List Images\n");
        printf("3- Remove Image\n");
        printf("4- Display Image\n");
        printf("5- Leave\n");
        printf("} ");

        // Clear buffer before reading
        memset(input_buffer, 0, sizeof(input_buffer));
        // Read up to `sizeof(input_buffer) - 1` characters to leave space for null terminator
        bytes_read = receive_until(input_buffer, '\n', sizeof(input_buffer) - 1);

        if (bytes_read <= 0) {
            printf("[ERROR] Failed to read choice\n");
            return 0; // Indicate failure to read input
        }

        // Convert the first character of input to an integer choice
        choice_val = input_buffer[0] - '0';

        if (choice_val >= 1 && choice_val <= 5) {
            // Valid choice, break from loop to handle action
            break;
        }

        printf("[ERROR] Invalid option: %d\n", choice_val);
    }

    // The original snippet ends abruptly after the loop break.
    // This switch statement is a placeholder for the menu's actions.
    switch (choice_val) {
        case 1:
            printf("Action: Upload Image\n");
            // Call function to handle image upload
            break;
        case 2:
            printf("Action: List Images\n");
            // Call function to list images
            break;
        case 3:
            printf("Action: Remove Image\n");
            // Call function to remove an image
            break;
        case 4:
            printf("Action: Display Image\n");
            // Call function to display an image (e.g., selectRenderer)
            break;
        case 5:
            printf("Action: Leaving menu.\n");
            return 1; // User chose to leave, indicate successful menu interaction
    }

    // If an action was performed (other than 'Leave'), return 1 to indicate success.
    return 1;
}

// Function: main
undefined4 main(void) {
    menu();
    return 0;
}