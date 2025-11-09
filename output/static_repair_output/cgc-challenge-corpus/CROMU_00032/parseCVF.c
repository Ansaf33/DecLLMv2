#include <stdio.h>    // For printf
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For memset, memcpy
#include <ctype.h>    // For isalpha, isdigit, isspace, isascii
#include <unistd.h>   // For read (simulating receive)
#include <errno.h>    // For errno

// --- Type Definitions (replacing ghidra's undefined types) ---
typedef unsigned int uint;
typedef unsigned char byte;
// 'undefined' typically means a single byte, 'char' is suitable.
// 'undefined4' typically means 4 bytes, 'unsigned int' or 'int' is suitable.

// --- Dummy/Simulated Functions ---

// Global buffer and position for simulating readBits
static unsigned char *sim_buffer = NULL;
static size_t sim_buffer_len = 0;
static size_t sim_buffer_pos = 0;
static unsigned int sim_bit_buffer = 0;
static int sim_bits_in_buffer = 0;

// A simple way to provide input for readBits for testing.
// In a real scenario, 'fd' would be a file descriptor or similar.
// For compilation, we'll simulate it with an in-memory buffer.
void set_simulated_input(unsigned char *buffer, size_t len) {
    sim_buffer = buffer;
    sim_buffer_len = len;
    sim_buffer_pos = 0;
    sim_bit_buffer = 0;
    sim_bits_in_buffer = 0;
}

// Dummy implementation for receive.
// In playVideo, it's used to wait for a keypress (fd 0, count 1).
// In receiveWrapper, it's used to read 1 byte at a time into a buffer.
int receive(int fd, char *buf, size_t count, int *bytes_read_ptr) {
    if (fd == 0 && count == 1) { // Simulating stdin read for keypress
        char temp_char;
        // Use unistd.h read for actual stdin interaction
        ssize_t res = read(STDIN_FILENO, &temp_char, 1); 
        if (res == 1) {
            if (buf) {
                *buf = temp_char;
            }
            if (bytes_read_ptr) {
                *bytes_read_ptr = 1;
            }
            return 1; // Success
        }
        if (bytes_read_ptr) {
            *bytes_read_ptr = 0;
        }
        return 0; // Error or EOF
    }
    
    // Simulating reading from the global buffer for receiveWrapper's data
    if (sim_buffer && sim_buffer_pos < sim_buffer_len) {
        if (buf) {
            *buf = sim_buffer[sim_buffer_pos++];
        }
        if (bytes_read_ptr) {
            *bytes_read_ptr = 1;
        }
        return 1; // Success
    }
    if (bytes_read_ptr) {
        *bytes_read_ptr = 0;
    }
    return 0; // End of simulated input or error
}

// Dummy implementation for readBits.
// Reads a specified number of bits from the simulated input buffer.
int readBits(int fd, int num_bits, uint *out_val) {
    (void)fd; // fd is unused in this dummy implementation

    if (!out_val || num_bits <= 0 || num_bits > 32) {
        return 0; // Invalid arguments
    }
    *out_val = 0;

    if (!sim_buffer) {
        return 0; // No simulated input set
    }

    for (int i = 0; i < num_bits; ++i) {
        if (sim_bits_in_buffer == 0) {
            if (sim_buffer_pos >= sim_buffer_len) {
                return 0; // End of input
            }
            sim_bit_buffer = sim_buffer[sim_buffer_pos++];
            sim_bits_in_buffer = 8;
        }

        *out_val = (*out_val << 1) | ((sim_bit_buffer >> (sim_bits_in_buffer - 1)) & 1);
        sim_bits_in_buffer--;
    }
    return 1; // Success
}

// --- Function: receiveWrapper ---
// Reads 'total_size' bytes into the buffer starting at 'dest_address'.
uint receiveWrapper(int dest_address, uint total_size) {
    uint total_received = 0;
    
    if (dest_address == 0) {
        return 0;
    }
    
    char *buffer = (char *)dest_address;

    while (total_received < total_size) {
        int bytes_read_this_call = 0;
        int result = receive(0, buffer + total_received, 1, &bytes_read_this_call);
        
        if (result == 0 || bytes_read_this_call == 0) {
            printf("[ERROR] Read fail\n");
            return 0;
        }
        total_received += bytes_read_this_call;
    }
    
    return total_received;
}

// --- Function: freeCVF ---
// Frees memory associated with a CVF video object.
void freeCVF(void *cvf_obj_ptr) {
    if (cvf_obj_ptr == NULL) {
        return;
    }

    char *cvf_base = (char *)cvf_obj_ptr;
    uint i;

    // Free custom pixel dictionaries (8 of them)
    // Dictionaries are stored starting at offset 0x200 (0x40 * 8).
    // Each entry is 8 bytes: 4 bytes for length, 4 bytes for data pointer.
    // Length at (i*8 + 0x20c), Data pointer at (i*8 + 0x210).
    for (i = 0; i < 8; ++i) {
        char **dict_data_ptr = (char **)(cvf_base + (i * 8) + 0x210);
        if (*dict_data_ptr != NULL) {
            free(*dict_data_ptr);
            *dict_data_ptr = NULL;
            *(uint *)(cvf_base + (i * 8) + 0x20c) = 0; // Reset dictionary length
        }
    }

    // Free frame data
    // Frame count at offset 0x24c. Array of frame pointers at offset 0x250.
    uint *num_frames_ptr = (uint *)(cvf_base + 0x24c);
    char ***frames_array_ptr = (char ***)(cvf_base + 0x250);

    if (*frames_array_ptr != NULL) {
        char **frames_array = *frames_array_ptr;
        uint num_frames = *num_frames_ptr;

        for (i = 0; i < num_frames; ++i) {
            char *frame_obj = frames_array[i];
            if (frame_obj != NULL) {
                // Frame object structure: { uint height, uint width, char* pixel_data }
                // Pixel data pointer is at offset 8 within the frame object.
                char **pixel_data_ptr = (char **)(frame_obj + 8);
                if (*pixel_data_ptr != NULL) {
                    free(*pixel_data_ptr);
                    *pixel_data_ptr = NULL;
                    *(uint *)(frame_obj + 0) = 0; // Reset height
                    *(uint *)(frame_obj + 4) = 0; // Reset width
                }
                free(frame_obj);
                frames_array[i] = NULL;
            }
        }
        free(frames_array);
        *frames_array_ptr = NULL;
    }

    free(cvf_obj_ptr); // Free the main CVF structure
}

// --- Function: playVideo ---
// Plays the video stored in the CVF object.
uint playVideo(uint *cvf_data) {
    uint success = 1;
    
    if (cvf_data == NULL || cvf_data[0x94] == 0) { // Check for valid CVF data and frame array
        return 0;
    }
    
    uint height = cvf_data[0];
    uint width = cvf_data[1];
    uint total_frames_expected = cvf_data[2]; // Total frames from header
    uint frames_loaded = cvf_data[0x93];     // Actual frames loaded
    
    uint frame_pixel_count = height * width;
    
    printf("--------------------Playing video-------------------\n");
    printf("INFO: Height: %u Width: %u Frames: %u\n", height, width, total_frames_expected);
    printf("INFO: Set your terminal height so that only the '|'s show.\n");
    printf("INFO: This will provide a better viewing experience\n");
    printf("INFO: When ready press a key...\n");
    
    // Print vertical lines for terminal height guidance
    for (uint i = 0; i < height; ++i) {
        printf("|\n");
    }
    
    // Wait for user input (keypress)
    char dummy_input_buffer;
    if (receive(0, &dummy_input_buffer, 1, NULL) == 0) {
        success = 0;
    } else {
        char **frames_array = (char **)cvf_data[0x94];
        for (uint current_frame_idx = 0; current_frame_idx < frames_loaded; ++current_frame_idx) {
            char *frame_obj = frames_array[current_frame_idx];
            
            if (frame_obj == NULL) {
                printf("[ERROR] Frame %u is null\n", current_frame_idx);
                return 0;
            }
            
            // Frame object: { uint height, uint width, char* pixel_data }
            uint frame_width_for_linebreak = *(uint *)(frame_obj + 4); // Width of the frame
            char *pixel_data = *(char **)(frame_obj + 8);
            
            for (uint pixel_idx = 0; pixel_idx < frame_pixel_count; ++pixel_idx) {
                if ((pixel_idx != 0) && (pixel_idx % frame_width_for_linebreak == 0)) {
                    printf("\n");
                }
                printf("%c", pixel_data[pixel_idx]);
            }
            printf("\n"); // Newline after each frame
        }
    }
    return success;
}

// --- Function: bitsNeeded ---
// Calculates the number of bits required to represent a given unsigned integer.
int bitsNeeded(uint value) {
    if (value == 0) {
        return 1; // 0 needs 1 bit (to represent 0)
    }
    int count = 0;
    for (; value != 0; value >>= 1) {
        count++;
    }
    return count;
}

// --- Helper structure for dictionaries in parseCVFFrame ---
typedef struct {
    uint count;
    char *chars;
} PixelDict;

// Built-in dictionaries. Using static const for read-only memory.
static const PixelDict builtin_dicts[] = {
    {0, NULL}, // Type 0 is custom
    {2, " ."},
    {4, " .|#"},
    {8, " .|#@$()"},
    {16, " .|#@$()*HOEWM%&"},
    {42, " .|#@$()*HOEWM%&abcdefghijklmnopqrstuvwxyz"},
    {62, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"},
    {95, " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"}
};

// --- Function: parseCVFFrame ---
// Parses a single video frame from the input stream.
uint parseCVFFrame(int fd, uint *cvf_data) {
    if (fd == 0 || cvf_data == NULL || *cvf_data == 0) {
        if (cvf_data != NULL && *cvf_data == 0) {
            printf("[ERROR] Header must be specified prior to frame rendering.\n");
        }
        return 0;
    }

    uint frame_header_byte;
    if (!readBits(fd, 8, &frame_header_byte)) {
        return 0;
    }

    byte is_diff_frame = (frame_header_byte >> 7) & 1; // 1 for diff frame, 0 for full frame
    byte builtin_dict_type = (frame_header_byte >> 3) & 7;
    byte custom_dict_idx = frame_header_byte & 7;

    // "First frame must be full" (i.e., not a diff frame)
    if ((cvf_data[0x93] == 0) && (is_diff_frame != 0)) { // 0x93 is current frame count
        printf("[ERROR] First frame must be a full frame (diff_frame bit must be 0).\n");
        return 0;
    }

    PixelDict current_pixel_dict;
    if (builtin_dict_type == 0) { // Custom Dictionary
        uint *dict_len_ptr = (uint *)((char *)cvf_data + (custom_dict_idx * 8) + 0x20c);
        char **dict_data_ptr = (char **)((char *)cvf_data + (custom_dict_idx * 8) + 0x210);

        if (*dict_len_ptr == 0) {
            printf("[ERROR] Custom Dictionary %u does not exist\n", (uint)custom_dict_idx);
            return 0;
        }
        current_pixel_dict.count = *dict_len_ptr;
        current_pixel_dict.chars = *dict_data_ptr;
    } else { // Built-in Dictionary
        if (builtin_dict_type >= sizeof(builtin_dicts)/sizeof(builtin_dicts[0])) {
             printf("[ERROR] Invalid built-in dictionary type %u\n", (uint)builtin_dict_type);
             return 0;
        }
        current_pixel_dict = builtin_dicts[builtin_dict_type];
    }

    int bits_for_pixel_value = bitsNeeded(current_pixel_dict.count - 1);

    // Allocate new frame object: { height, width, pixel_data_ptr } (12 bytes)
    uint *new_frame_obj = (uint *)malloc(12);
    if (new_frame_obj == NULL) {
        return 0;
    }

    size_t frame_pixel_count = (size_t)cvf_data[1] * cvf_data[0]; // width * height
    new_frame_obj[0] = cvf_data[0]; // Height
    new_frame_obj[1] = cvf_data[1]; // Width

    char *pixel_data_buffer = (char *)malloc(frame_pixel_count);
    new_frame_obj[2] = (uint)pixel_data_buffer; // Store pixel data pointer
    if (pixel_data_buffer == NULL) {
        free(new_frame_obj);
        return 0;
    }

    uint success = 0; // Assume failure until frame is fully processed

    if (is_diff_frame == 0) { // Full Frame
        memset(pixel_data_buffer, 0x20, frame_pixel_count); // Initialize with spaces
        for (uint i = 0; i < frame_pixel_count; ++i) {
            uint pixel_value_read;
            if (!readBits(fd, bits_for_pixel_value, &pixel_value_read)) {
                printf("[ERROR] Failed to read pixel value for full frame\n");
                goto cleanup_frame;
            }
            if (current_pixel_dict.count <= pixel_value_read) {
                printf("[ERROR] Invalid pixel value %u (dict count %u)\n", pixel_value_read, current_pixel_dict.count);
                goto cleanup_frame;
            }
            pixel_data_buffer[i] = current_pixel_dict.chars[pixel_value_read];
        }
        success = 1;
    } else { // Diff Frame
        // Copy data from previous frame
        char **frames_array = (char **)cvf_data[0x94];
        uint prev_frame_idx = cvf_data[0x93] - 1;
        char *prev_frame_obj = frames_array[prev_frame_idx];

        if (prev_frame_obj == NULL) {
            printf("[ERROR] Previous frame is null for diff frame\n");
            goto cleanup_frame;
        }
        char *prev_pixel_data = *(char **)(prev_frame_obj + 8);
        memcpy(pixel_data_buffer, prev_pixel_data, frame_pixel_count);

        // Read num_pixels_to_write
        uint num_pixels_to_write;
        int bits_for_index_count = bitsNeeded(frame_pixel_count); // To represent up to frame_pixel_count
        if (!readBits(fd, bits_for_index_count, &num_pixels_to_write)) {
            printf("[ERROR] Failed to read number of diff pixels\n");
            goto cleanup_frame;
        }

        if (num_pixels_to_write == 0) {
            printf("[ERROR] Empty diff frames not allowed (num_pixels_to_write 0)\n");
            goto cleanup_frame;
        }

        int bits_for_pixel_index = bitsNeeded(frame_pixel_count - 1); // To represent up to max index

        for (uint i = 0; i < num_pixels_to_write; ++i) {
            uint pixel_index;
            uint pixel_value_read;

            if (!readBits(fd, bits_for_pixel_index, &pixel_index)) {
                printf("[ERROR] Failed to read pixel index for diff frame\n");
                goto cleanup_frame;
            }
            if (!readBits(fd, bits_for_pixel_value, &pixel_value_read)) {
                printf("[ERROR] Failed to read pixel value for diff frame\n");
                goto cleanup_frame;
            }

            if (frame_pixel_count <= pixel_index) {
                printf("[ERROR] Index %u out of image bounds %zu\n", pixel_index, frame_pixel_count);
                goto cleanup_frame;
            }
            if (current_pixel_dict.count <= pixel_value_read) {
                printf("[ERROR] Pixel value %u beyond dictionary bounds %u\n", pixel_value_read, current_pixel_dict.count);
                goto cleanup_frame;
            }
            pixel_data_buffer[pixel_index] = current_pixel_dict.chars[pixel_value_read];
        }
        success = 1;
    }

    // If parsing was successful, add the frame to the CVF structure
    if (success) {
        char **frames_array_base = (char **)cvf_data[0x94];
        uint current_frame_count = cvf_data[0x93];
        frames_array_base[current_frame_count] = (char *)new_frame_obj;
        cvf_data[0x93]++; // Increment frame count in header
    } else {
        // Cleanup if an error occurred during frame parsing
cleanup_frame:
        if (pixel_data_buffer) free(pixel_data_buffer);
        if (new_frame_obj) free(new_frame_obj);
        return 0;
    }
    
    return success;
}

// --- Function: parseCVFPixelDict ---
// Parses a custom pixel dictionary from the input stream.
uint parseCVFPixelDict(int fd, uint *cvf_data, int dict_idx) {
    if (fd == 0 || cvf_data == NULL || dict_idx < 0 || dict_idx > 7) {
        return 0;
    }

    // Check if dictionary already exists at cvf_data + (dict_idx * 8) + 0x20c
    uint *existing_dict_len_ptr = (uint *)((char *)cvf_data + (dict_idx * 8) + 0x20c);
    if (*existing_dict_len_ptr != 0) {
        printf("[ERROR] Only one type %d pixel dictionary allowed.\n", dict_idx);
        return 0;
    }

    uint dict_len;
    if (!readBits(fd, 8, &dict_len)) {
        return 0;
    }

    if (dict_len == 0) {
        printf("[ERROR] Zero length pixel dictionary not allowed.\n");
        return 0;
    }

    char *dict_data = (char *)malloc(dict_len + 1); // +1 for null terminator
    if (dict_data == NULL) {
        return 0;
    }
    memset(dict_data, 0, dict_len + 1);

    for (uint i = 0; i < dict_len; ++i) {
        uint char_val;
        if (!readBits(fd, 8, &char_val)) {
            printf("[ERROR] Failed to read custom pixel dictionary character\n");
            free(dict_data);
            return 0;
        }
        dict_data[i] = (char)char_val;
    }

    // Store dictionary in CVF structure
    *existing_dict_len_ptr = dict_len;
    char **dict_data_ptr = (char **)((char *)cvf_data + (dict_idx * 8) + 0x210);
    *dict_data_ptr = dict_data;
    
    return 1;
}

// --- Function: parseCVFDescription ---
// Parses the video description from the input stream.
uint parseCVFDescription(int fd, char *cvf_data) {
    if (fd == 0 || cvf_data == NULL) {
        return 0;
    }

    // Description stored starting at offset 0x10c.
    // Check if description already exists (first char is not null).
    if (cvf_data[0x10c] != '\0') {
        return 0;
    }

    uint desc_len;
    if (!readBits(fd, 8, &desc_len)) {
        return 0;
    }

    if (desc_len >= 0x100) { // Max description length 255
        return 0;
    }

    char *description_buffer = cvf_data + 0x10c;
    for (uint i = 0; i < desc_len; ++i) {
        uint char_val;
        if (!readBits(fd, 8, &char_val)) {
            return 0;
        }
        description_buffer[i] = (char)char_val;
    }
    description_buffer[desc_len] = '\0'; // Null-terminate

    // Validate characters
    for (uint i = 0; i < desc_len; ++i) {
        char c = description_buffer[i];
        if (!isalpha(c) && !isdigit(c) && !isspace(c)) {
            printf("[ERROR] Invalid character in description: '%c'\n", c);
            return 0;
        }
    }
    
    return 1;
}

// --- Function: parseCVFName ---
// Parses the video name from the input stream.
uint parseCVFName(int fd, char *cvf_data) {
    if (fd == 0 || cvf_data == NULL) {
        return 0;
    }

    // Name stored starting at offset 0xc.
    // Check if name already exists (first char is not null).
    if (cvf_data[0xc] != '\0') {
        return 0;
    }

    uint name_len;
    if (!readBits(fd, 8, &name_len)) {
        return 0;
    }

    if (name_len >= 0x100) { // Max name length 255
        return 0;
    }

    char *name_buffer = cvf_data + 0xc;
    for (uint i = 0; i < name_len; ++i) {
        uint char_val;
        if (!readBits(fd, 8, &char_val)) {
            return 0;
        }
        name_buffer[i] = (char)char_val;
    }
    name_buffer[name_len] = '\0'; // Null-terminate

    // Validate characters (ASCII)
    for (uint i = 0; i < name_len; ++i) {
        char c = name_buffer[i];
        if (!isascii(c)) {
            printf("[ERROR] Invalid value in name field: '%c'\n", c);
            return 0;
        }
    }
    
    return 1;
}

// --- Function: parseCVFHeader ---
// Parses the video header (height, width, frame count) from the input stream.
uint parseCVFHeader(int fd, uint *cvf_data) {
    if (fd == 0 || cvf_data == NULL) {
        return 0;
    }

    // Check if header already parsed (height field at cvf_data[0] is non-zero)
    if (cvf_data[0] != 0) {
        printf("[ERROR] Multiple header sections are not permitted.\n");
        return 0;
    }

    // Read Height (8 bits)
    if (!readBits(fd, 8, &cvf_data[0])) { // cvf_data[0] is height
        return 0;
    }
    if (cvf_data[0] == 0) {
        printf("[ERROR] Zero length height is not permitted\n");
        return 0;
    }
    if (cvf_data[0] >= 36) { // Height must be < 36
        printf("[ERROR] Height must be less than 36\n");
        return 0;
    }

    // Read Width (8 bits)
    if (!readBits(fd, 8, &cvf_data[1])) { // cvf_data[1] is width
        return 0;
    }
    if (cvf_data[1] == 0) {
        printf("[ERROR] Zero length width is not permitted\n");
        return 0;
    }
    if (cvf_data[1] >= 129) { // Width must be < 129
        printf("[ERROR] Width must be less than 129\n");
        return 0;
    }

    // Read Frame Count (16 bits)
    if (!readBits(fd, 16, &cvf_data[2])) { // cvf_data[2] is total frames
        return 0;
    }
    if (cvf_data[2] == 0) {
        printf("[ERROR] Zero frame count is not permitted\n");
        return 0;
    }
    if (cvf_data[2] >= 2049) { // Max frame count is 2048
        printf("[ERROR] Maximum frame count is 2048\n");
        return 0;
    }

    // Allocate memory for frame pointers array
    // cvf_data[0x94] will store the pointer to this array.
    void *frames_array = malloc(cvf_data[2] * sizeof(void *));
    cvf_data[0x94] = (uint)frames_array; // Store the pointer (cast to uint)

    if (frames_array == NULL) {
        printf("[ERROR] Failed to allocate memory for frame pointers array\n");
        return 0;
    }
    memset(frames_array, 0, cvf_data[2] * sizeof(void *)); // Initialize to NULL

    return 1;
}

// --- Function: initCVF ---
// Allocates and initializes the main CVF structure.
// The size 0x254 is determined by the maximum offset used (0x250 for frames array pointer, plus 4 bytes).
void *initCVF(void) {
    void *cvf_obj = malloc(0x254);
    if (cvf_obj != NULL) {
        memset(cvf_obj, 0, 0x254);
    }
    return cvf_obj;
}

// --- Function: renderCVF ---
// Main function to parse and render a CVF file.
void renderCVF(int fd) {
    if (fd == 0) {
        return;
    }

    uint magic_val;
    if (!readBits(fd, 0x20, &magic_val)) { // Read 32-bit magic
        return;
    }

    if (magic_val != 0x435646) { // 'CVF' in little-endian ASCII
        printf("[ERROR] Invalid magic: 0x%x (expected 0x435646)\n", magic_val);
        return;
    }

    uint *cvf_object = (uint *)initCVF();
    if (cvf_object == NULL) {
        printf("[ERROR] Failed to initialize CVF object\n");
        return;
    }

    uint section_type;
    int parse_result;

    while (1) {
        section_type = 0;
        if (!readBits(fd, 0x10, &section_type)) { // Read 16-bit section type
            // End of file or read error, proceed to play video and cleanup
            break;
        }

        switch (section_type) {
            case 0x1111: // Header Section
                parse_result = parseCVFHeader(fd, cvf_object);
                break;
            case 0x2222: // Name Section
                parse_result = parseCVFName(fd, (char*)cvf_object);
                break;
            case 0x3333: // Description Section
                parse_result = parseCVFDescription(fd, (char*)cvf_object);
                break;
            case 0x4444: // Pixel Dictionary 0
            case 0x4445: // Pixel Dictionary 1
            case 0x4446: // Pixel Dictionary 2
            case 0x4447: // Pixel Dictionary 3
            case 0x4448: // Pixel Dictionary 4
            case 0x4449: // Pixel Dictionary 5
            case 0x444A: // Pixel Dictionary 6
            case 0x444B: // Pixel Dictionary 7
                parse_result = parseCVFPixelDict(fd, cvf_object, section_type - 0x4444);
                break;
            case 0x5555: // Frame Section
                parse_result = parseCVFFrame(fd, cvf_object);
                break;
            default: // Unknown section type
                printf("[ERROR] Invalid section type: 0x%x\n", section_type);
                parse_result = 0; // Indicate parsing failure
                break;
        }

        if (parse_result == 0) {
            printf("[ERROR] Parsing section 0x%x failed.\n", section_type);
            freeCVF(cvf_object);
            return;
        }
    }

    playVideo(cvf_object);
    freeCVF(cvf_object);
    return;
}

// --- Main Function (for compilation and testing) ---
int main() {
    // Example 1: A simple 10x20 video with 1 full frame, using custom dict 0 = " ."
    // All pixels are index 0 (space).
    unsigned char test_cvf_data_simple[] = {
        // Magic: CVF (0x43 0x56 0x46 0x00) - Assuming little-endian 0x00435646
        0x46, 0x56, 0x43, 0x00, 
        
        // Header Section (0x1111)
        0x11, 0x11,
        // Height (8 bits): 10
        10,
        // Width (8 bits): 20
        20,
        // Frames (16 bits): 1
        0x01, 0x00, // Little endian 1
        
        // Pixel Dictionary 0 Section (0x4444)
        0x44, 0x44,
        // Dict Length (8 bits): 2
        2,
        // Dict Data (2 bytes): " ."
        ' ', '.',
        
        // Frame Section (0x5555) - Frame 0 (Full frame)
        0x55, 0x55,
        // Frame Header Byte: 0x00 (Full frame, custom dict 0)
        0x00,
        // Pixel data for 10x20 = 200 pixels.
        // bitsNeeded(2-1)=1 bit per pixel. So, 200 bits = 25 bytes.
        // All pixels are ' ', so all bits are 0.
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, // 25 bytes
    };

    printf("Simulating CVF rendering (simple full frame)...\n");
    set_simulated_input(test_cvf_data_simple, sizeof(test_cvf_data_simple));
    renderCVF(1); // Pass a non-zero fd to indicate a valid stream

    // Example 2: Diff frame test. 5x5 video, 2 frames.
    // Frame 1: All spaces.
    // Frame 2: Changes some pixels to '#'.
    unsigned char test_cvf_data_diff[] = {
        // Magic: CVF
        0x46, 0x56, 0x43, 0x00, 
        
        // Header (0x1111): H=5, W=5, Frames=2
        0x11, 0x11, 5, 5, 0x02, 0x00,
        
        // Pixel Dictionary 0 (0x4444): Length=3, Data=" #."
        0x44, 0x44, 3, ' ', '#', '.',
        
        // Frame 1 (0x5555) - Full frame
        // Header: 0x00 (Full frame, custom dict 0)
        0x55, 0x55, 0x00,
        // Pixel data for 5x5 = 25 pixels. bitsNeeded(3-1)=2 bits per pixel.
        // 25 pixels * 2 bits = 50 bits = 6 bytes and 2 bits.
        // All pixels are index 0 (' ').
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 7 bytes (56 bits), padding 6 bits
        
        // Frame 2 (0x5555) - Diff frame
        // Header: 0x80 (Diff frame, custom dict 0)
        0x55, 0x55, 0x80,
        // Num diff pixels: 5. bitsNeeded(25)=5 bits for num_diff.
        // Pixel indices: (0,2)=2, (1,1)=6, (1,3)=8, (2,0)=10, (2,2)=12.
        // Each index (5 bits), each value (2 bits). Total 7 bits per diff.
        // Total bits: 5 (num_diff) + 5 * 7 (diffs) = 40 bits = 5 bytes.
        //
        // Bit stream:
        // Num diff: 5 (00101)
        // Diffs: [idx 2, val 1], [idx 6, val 1], [idx 8, val 1], [idx 10, val 1], [idx 12, val 1]
        //        00010 01,      00110 01,      01000 01,      01010 01,       01100 01
        // Bytes:
        // 00101000 (0x28) - num_diff 5, start of idx 2 val 1
        // 10010010 (0x92) - rest of idx 2 val 1, start of idx 6 val 1
        // 10000101 (0x85) - rest of idx 6 val 1, start of idx 8 val 1
        // 01010010 (0x52) - rest of idx 8 val 1, start of idx 10 val 1
        // 11000100 (0xC4) - rest of idx 10 val 1, start of idx 12 val 1
        0x28, 0x92, 0x85, 0x52, 0xC4,
    };
    
    printf("\nSimulating CVF rendering with diff frame...\n");
    set_simulated_input(test_cvf_data_diff, sizeof(test_cvf_data_diff));
    renderCVF(1);

    printf("\nSimulation finished.\n");

    return 0;
}