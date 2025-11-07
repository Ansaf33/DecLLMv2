#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memset, memcpy
#include <ctype.h>  // For isalpha, isdigit, isspace, isascii
#include <stdint.h> // For uint32_t, uintptr_t

// Type definitions for decompiled code, assuming a 32-bit environment
// where pointers and unsigned ints are 4 bytes.
typedef unsigned int uint;
typedef uint32_t undefined4; // Assuming 4-byte unsigned integer, used for status/return values
typedef unsigned char byte;

// Forward declarations for functions defined later
void *initCVF(void);
void freeCVF(void *param_1);
int bitsNeeded(uint val);
uint receiveWrapper(void *buffer, uint count);
undefined4 parseCVFHeader(int fd, uint *cvf_data);
undefined4 parseCVFName(int fd, int *cvf_data);
undefined4 parseCVFDescription(int fd, int *cvf_data);
uint parseCVFPixelDict(int fd, int *cvf_data, int dict_idx);
undefined4 parseCVFFrame(int fd, int *cvf_data);
undefined4 playVideo(uint *cvf_data);
void renderCVF(int fd);

// Dummy function for receive, as its actual implementation is not provided.
// It simulates reading a single byte and storing it in the buffer.
// For playVideo's specific call, it simulates reading from stdin.
int receive(int fd, void *buf, size_t count, int *bytesRead) {
    if (buf == NULL || bytesRead == NULL || count == 0) {
        return -1; // Error
    }
    // Simulate reading 1 byte.
    if (count > 0) {
        if (fd == 0) { // Assuming fd 0 is stdin for interactive input
            char c = getchar();
            if (c != EOF) {
                *((char*)buf) = c;
                *bytesRead = 1;
                return 0; // Success
            }
        }
        // Fallback for other cases or if getchar fails
        *((char*)buf) = 'A'; // Example data
        *bytesRead = 1;
        return 0; // Success
    }
    return -1; // Error
}

// Dummy function for readBits, as its actual implementation is not provided.
// It simulates reading bits and storing them in the provided uint pointer.
// The values assigned here are based on the context of how they are used in the original code
// to allow the logic to flow through different branches.
int readBits(int fd, int bits_to_read, uint *out_val) {
    if (out_val == NULL || bits_to_read <= 0 || bits_to_read > 32) {
        return 0; // Failure
    }
    *out_val = 1; // Default success value

    if (bits_to_read == 32) { // Magic number
        *out_val = 0x435646; // "CVF"
    } else if (bits_to_read == 16) { // Section type
        static int section_type_idx = 0;
        uint section_types[] = {
            0x1111, // Header
            0x2222, // Name
            0x3333, // Description
            0x4444, // Pixel Dict 0
            0x4445, // Pixel Dict 1
            0x5555, // Frame
            0x5555, // Another Frame
            0xEEEE  // End or unknown marker
        };
        *out_val = section_types[section_type_idx % (sizeof(section_types)/sizeof(section_types[0]))];
        section_type_idx++;
    } else if (bits_to_read == 8) { // Height, Width, Description length, Name length, Pixel dict length, Frame info byte
        *out_val = 10; // Example value for most 8-bit reads
    } else if (bits_to_read == 10) { // Example for max_frames (0x801 needs 11 bits, but original used 0x10=16 bits)
        *out_val = 5;
    } else if (bits_to_read == 11) { // bitsNeeded(0x801-1) = 11
        *out_val = 5;
    } else if (bits_to_read == 1) { // Flags like is_delta_frame, is_compressed
        *out_val = 0; // Default to non-delta, non-compressed for simplicity
    } else if (bits_to_read == 3) { // dict_type, custom_dict_idx
        *out_val = 1; // Default to dict_type 1 (common) or custom_dict_idx 1
    } else if (bits_to_read <= 7) { // Various small bit counts
        *out_val = 1; // General small value
    }

    return 1; // Success
}

// Structure for a CVF Frame, derived from parseCVFFrame usage
typedef struct {
    int height;
    int width;
    char *pixel_data;
} CVF_Frame;

// Function: receiveWrapper
uint receiveWrapper(void *buffer, uint count) {
    if (buffer == NULL) {
        return 0;
    }

    uint total_bytes_read = 0;
    int bytes_read_this_iter = 0;
    
    while (total_bytes_read < count) {
        bytes_read_this_iter = 0;
        int iVar1 = receive(0, (char*)buffer + total_bytes_read, 1, &bytes_read_this_iter);
        if (iVar1 != 0) {
            printf("[ERROR] Read fail\n");
            return 0;
        }
        if (bytes_read_this_iter == 0) { // Prevent infinite loop if receive reads 0 bytes
            printf("[ERROR] Receive returned 0 bytes, expected more.\n");
            return 0;
        }
        total_bytes_read += bytes_read_this_iter;
    }
    return total_bytes_read;
}

// Function: freeCVF
void freeCVF(void *param_1) {
    if (param_1 == NULL) {
        return;
    }

    char *base_ptr = (char *)param_1; // Use char* for byte-level pointer arithmetic

    // Free custom pixel dictionaries (8 of them)
    // Dictionaries are stored as `uint size` and `void* data` in 8-byte slots
    // starting at `base_ptr + 0x200`.
    // Size is at offset 0, data pointer at offset 4 within each 8-byte slot.
    for (uint i = 0; i < 8; ++i) {
        void **dict_data_loc = (void **)(base_ptr + 0x200 + i * 8 + 4);
        undefined4 *dict_size_loc = (undefined4 *)(base_ptr + 0x200 + i * 8);

        if (*dict_data_loc != NULL) {
            free(*dict_data_loc);
            *dict_data_loc = NULL;
            *dict_size_loc = 0;
        }
    }

    // Free frames data
    // `base_ptr + 0x24c` is the current frame count (uint)
    // `base_ptr + 0x250` is a pointer to an array of `CVF_Frame*`
    uint frame_count = *(uint *)(base_ptr + 0x24c);
    CVF_Frame **frames_array = *(CVF_Frame ***)(base_ptr + 0x250);

    if (frames_array != NULL) {
        for (uint i = 0; i < frame_count; ++i) {
            CVF_Frame *frame_struct = frames_array[i];
            if (frame_struct != NULL) {
                if (frame_struct->pixel_data != NULL) {
                    free(frame_struct->pixel_data);
                    frame_struct->pixel_data = NULL;
                }
                free(frame_struct);
                frames_array[i] = NULL;
            }
        }
        free(frames_array);
        *(CVF_Frame ***)(base_ptr + 0x250) = NULL;
    }
    free(param_1); // Finally, free the main CVF structure
}

// Function: playVideo
undefined4 playVideo(uint *param_1) { // param_1 is the CVF header structure pointer
    if (param_1 == NULL || ((uintptr_t)param_1[0x94]) == 0) {
        return 0;
    }

    uint height = param_1[0];
    uint width = param_1[1];
    uint frame_count = param_1[0x93];
    CVF_Frame **frames_array = (CVF_Frame **)param_1[0x94];

    int video_buffer_size = height * width;

    printf("--------------------Playing video-------------------\n");
    printf("INFO: Height: %u Width: %u Frames: %u\n", height, width, frame_count);
    printf("INFO: Set your terminal height so that only the '|'s show.\n");
    printf("INFO: This will provide a better viewing experience\n");
    printf("INFO: When ready press a key...\n");

    for (uint i = 0; i < height; ++i) {
        printf("|\n");
    }

    undefined4 user_input_char = 0;
    if (receiveWrapper(&user_input_char, 1) == 0) {
        return 0; // Read fail
    }

    for (uint current_frame_idx = 0; current_frame_idx < frame_count; ++current_frame_idx) {
        CVF_Frame *current_frame_struct = frames_array[current_frame_idx];
        if (current_frame_struct == NULL || current_frame_struct->pixel_data == NULL) {
            printf("[ERROR] Frame %u data is missing.\n", current_frame_idx);
            return 0;
        }

        for (uint pixel_idx = 0; pixel_idx < video_buffer_size; ++pixel_idx) {
            if ((pixel_idx != 0) && (current_frame_struct->width != 0) && (pixel_idx % current_frame_struct->width == 0)) {
                printf("\n");
            }
            printf("%c", current_frame_struct->pixel_data[pixel_idx]);
        }
        printf("\n");
    }
    return 1; // Success
}

// Function: bitsNeeded
int bitsNeeded(uint val) {
    if (val == 0) {
        return 1;
    }
    int bits = 0;
    while (val != 0) {
        val >>= 1;
        bits++;
    }
    return bits;
}

// Function: parseCVFFrame
undefined4 parseCVFFrame(int fd, int *cvf_data) { // cvf_data is the main CVF header structure
    if (fd == 0 || cvf_data == NULL || cvf_data[0] == 0) {
        printf("[ERROR] Header must be specified prior to frame rendering.\n");
        return 0;
    }

    uint header_byte = 0;
    if (readBits(fd, 8, &header_byte) == 0) return 0;

    uint is_delta_frame = (header_byte >> 7) & 1;
    // uint is_compressed = (header_byte >> 6) & 1; // Not used
    uint dict_type = (header_byte >> 3) & 7;
    uint custom_dict_idx = header_byte & 7;

    if (cvf_data[0x93] == 0 && is_delta_frame != 0) { // cvf_data[0x93] is current_frame_count
        printf("[ERROR] First frame must be full\n");
        return 0;
    }

    uint dict_size = 0;
    const char *dict_data = NULL; // Use const char* for string literals or dictionary data

    if (dict_type == 0) { // Custom dictionary
        char *base_ptr = (char *)cvf_data;
        uint *custom_dict_size_ptr = (uint*)(base_ptr + 0x200 + custom_dict_idx * 8);
        char **custom_dict_data_ptr = (char**)(base_ptr + 0x200 + custom_dict_idx * 8 + 4);

        if (*custom_dict_size_ptr == 0) {
            printf("[ERROR] Custom Dictionary %u does not exist\n", custom_dict_idx);
            return 0;
        }
        dict_size = *custom_dict_size_ptr;
        dict_data = *custom_dict_data_ptr;
    } else { // Standard dictionaries
        switch (dict_type) {
            case 1: dict_size = 2; dict_data = " ."; break;
            case 2: dict_size = 4; dict_data = " .|#"; break;
            case 3: dict_size = 8; dict_data = " .|#@$()"; break;
            case 4: dict_size = 0x10; dict_data = " .|#@$()*HOEWM%&"; break;
            case 5: dict_size = 0x2a; dict_data = " .|#@$()*HOEWM%&abcdefghijklmnopqrstuvwxyz"; break;
            case 6: dict_size = 0x3e; dict_data = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; break;
            case 7: dict_size = 0x5f; dict_data = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"; break;
            default:
                printf("[ERROR] Invalid dictionary type %u\n", dict_type);
                return 0;
        }
    }

    int bits_per_pixel = bitsNeeded(dict_size - 1);
    
    CVF_Frame *new_frame = (CVF_Frame *)malloc(sizeof(CVF_Frame));
    if (new_frame == NULL) return 0;

    size_t total_pixels = (size_t)cvf_data[1] * cvf_data[0]; // width * height
    new_frame->height = cvf_data[0];
    new_frame->width = cvf_data[1];
    new_frame->pixel_data = (char *)malloc(total_pixels);

    if (new_frame->pixel_data == NULL) {
        free(new_frame);
        return 0;
    }

    uint num_pixels_to_read = total_pixels;
    int bits_for_index = 0;
    if (is_delta_frame) {
        bits_for_index = bitsNeeded(total_pixels - 1);
        if (readBits(fd, bits_for_index, &num_pixels_to_read) == 0) {
            free(new_frame->pixel_data); free(new_frame); return 0;
        }
        if (num_pixels_to_read == 0) {
            printf("[ERROR] Empty frames not allowed for delta frame\n");
            free(new_frame->pixel_data); free(new_frame); return 0;
        }
    }

    if (!is_delta_frame) {
        memset(new_frame->pixel_data, ' ', total_pixels); // Fill with spaces
        for (uint i = 0; i < num_pixels_to_read; ++i) {
            uint pixel_value_idx = 0;
            if (readBits(fd, bits_per_pixel, &pixel_value_idx) == 0) {
                free(new_frame->pixel_data); free(new_frame); return 0;
            }
            if (dict_size <= pixel_value_idx) {
                printf("[ERROR] Invalid pixel (value %u out of dict bounds %u)\n", pixel_value_idx, dict_size);
                free(new_frame->pixel_data); free(new_frame); return 0;
            }
            new_frame->pixel_data[i] = dict_data[pixel_value_idx];
        }
    } else { // Delta frame
        CVF_Frame **frames_array = (CVF_Frame **)cvf_data[0x94];
        uint prev_frame_idx = cvf_data[0x93] - 1; // Current frame count is 0x93, so previous is 0x93 - 1
        CVF_Frame *prev_frame = frames_array[prev_frame_idx];

        if (prev_frame == NULL || prev_frame->pixel_data == NULL) {
            printf("[ERROR] Previous frame data not available for delta frame.\n");
            free(new_frame->pixel_data); free(new_frame); return 0;
        }
        memcpy(new_frame->pixel_data, prev_frame->pixel_data, total_pixels);

        for (uint i = 0; i < num_pixels_to_read; ++i) {
            uint pixel_index = 0;
            uint pixel_value_idx = 0;

            if (readBits(fd, bits_for_index, &pixel_index) == 0) {
                free(new_frame->pixel_data); free(new_frame); return 0;
            }
            if (readBits(fd, bits_per_pixel, &pixel_value_idx) == 0) {
                free(new_frame->pixel_data); free(new_frame); return 0;
            }
            if (total_pixels <= pixel_index) {
                printf("[ERROR] Index %u out of image bounds %zu\n", pixel_index, total_pixels);
                free(new_frame->pixel_data); free(new_frame); return 0;
            }
            if (dict_size <= pixel_value_idx) {
                printf("[ERROR] Pixel %u beyond dictionary bounds %u\n", pixel_value_idx, dict_size);
                free(new_frame->pixel_data); free(new_frame); return 0;
            }
            new_frame->pixel_data[pixel_index] = dict_data[pixel_value_idx];
        }
    }

    // Padding bits
    uint bits_read_in_frame_data = 0;
    if (!is_delta_frame) {
        bits_read_in_frame_data = num_pixels_to_read * bits_per_pixel;
    } else {
        bits_read_in_frame_data = bits_for_index + (bits_for_index + bits_per_pixel) * num_pixels_to_read;
    }

    uint padding_bits = (-bits_read_in_frame_data) & 7;
    uint dummy_val = 0;
    if (padding_bits != 0 && readBits(fd, padding_bits, &dummy_val) == 0) {
        printf("[ERROR] Failed to read padding bits\n");
        free(new_frame->pixel_data); free(new_frame); return 0;
    }

    // Store the new frame
    CVF_Frame **frames_array_ptr = (CVF_Frame **)cvf_data[0x94];
    frames_array_ptr[cvf_data[0x93]] = new_frame;
    cvf_data[0x93]++; // Increment current_frame_count

    return 1; // Success
}

// Function: parseCVFPixelDict
uint parseCVFPixelDict(int fd, int *cvf_data, int dict_idx) { // cvf_data is main CVF header structure
    if (fd == 0 || cvf_data == NULL || dict_idx < 0 || dict_idx > 7) {
        return 0;
    }

    char *base_ptr = (char *)cvf_data;
    // Dictionaries are stored as `uint size` and `void* data` in 8-byte slots
    // starting at `base_ptr + 0x200`.
    // Size is at offset 0, data pointer at offset 4 within each 8-byte slot.
    uint *dict_size_loc = (uint *)(base_ptr + 0x200 + dict_idx * 8);
    void **dict_data_loc = (void **)(base_ptr + 0x200 + dict_idx * 8 + 4);

    if (*dict_size_loc != 0) { // Check if dictionary already exists
        printf("[ERROR] Only one type %d pixel dictionary allowed.\n", dict_idx);
        return 0;
    }

    uint dict_length = 0;
    if (readBits(fd, 8, &dict_length) == 0) return 0;
    if (dict_length == 0) {
        printf("[ERROR] Zero length pixel dictionary not allowed.\n");
        return 0;
    }

    char *dict_buffer = (char *)malloc(dict_length + 1);
    if (dict_buffer == NULL) return 0;
    memset(dict_buffer, 0, dict_length + 1);

    uint char_val = 0;
    for (uint i = 0; i < dict_length; ++i) {
        if (readBits(fd, 8, &char_val) == 0) {
            printf("[ERROR] Failed to read custom pixel dictionary character\n");
            free(dict_buffer);
            return 0;
        }
        dict_buffer[i] = (char)char_val;
    }

    *dict_size_loc = dict_length;
    *dict_data_loc = dict_buffer;

    return 1; // Success
}

// Function: parseCVFDescription
undefined4 parseCVFDescription(int fd, int *cvf_data) { // cvf_data is main CVF header structure
    if (fd == 0 || cvf_data == NULL) {
        return 0;
    }

    char *description_buffer = (char *)cvf_data + 0x10c;
    if (description_buffer[0] != '\0') { // Check if description already exists
        printf("[ERROR] Multiple description sections are not permitted.\n");
        return 0;
    }

    uint description_length = 0;
    if (readBits(fd, 8, &description_length) == 0) return 0;
    if (description_length >= 0x100) { // Max length is 255 for 8-bit length field
        printf("[ERROR] Description length %u exceeds maximum 255\n", description_length);
        return 0;
    }

    uint char_val = 0;
    for (uint i = 0; i < description_length; ++i) {
        if (readBits(fd, 8, &char_val) == 0) return 0;
        description_buffer[i] = (char)char_val;
    }
    description_buffer[description_length] = '\0'; // Null-terminate

    for (uint i = 0; i < description_length; ++i) {
        char c = description_buffer[i];
        if (!isalpha(c) && !isdigit(c) && !isspace(c)) {
            printf("[ERROR] Invalid character in description: '%c'\n", c);
            return 0;
        }
    }
    return 1; // Success
}

// Function: parseCVFName
undefined4 parseCVFName(int fd, int *cvf_data) { // cvf_data is main CVF header structure
    if (fd == 0 || cvf_data == NULL) {
        return 0;
    }

    char *name_buffer = (char *)cvf_data + 0xc;
    if (name_buffer[0] != '\0') { // Check if name already exists
        printf("[ERROR] Multiple name sections are not permitted.\n");
        return 0;
    }

    uint name_length = 0;
    if (readBits(fd, 8, &name_length) == 0) return 0;
    if (name_length >= 0x100) { // Max length is 255 for 8-bit length field
        printf("[ERROR] Name length %u exceeds maximum 255\n", name_length);
        return 0;
    }

    uint char_val = 0;
    for (uint i = 0; i < name_length; ++i) {
        if (readBits(fd, 8, &char_val) == 0) return 0;
        name_buffer[i] = (char)char_val;
    }
    name_buffer[name_length] = '\0'; // Null-terminate

    for (uint i = 0; i < name_length; ++i) {
        if (!isascii(name_buffer[i])) {
            printf("[ERROR] Invalid value in name field: '%c'\n", name_buffer[i]);
            return 0;
        }
    }
    return 1; // Success
}

// Function: parseCVFHeader
undefined4 parseCVFHeader(int fd, uint *cvf_data) { // cvf_data is main CVF header structure
    if (fd == 0 || cvf_data == NULL) {
        return 0;
    }
    if (cvf_data[0] != 0) { // Check if header already exists (height is 0 initially)
        printf("[ERROR] Multiple header sections are not permitted.\n");
        return 0;
    }

    uint height = 0;
    if (readBits(fd, 8, &height) == 0) return 0;
    if (height == 0) { printf("[ERROR] Zero length height is not permitted\n"); return 0; }
    if (height >= 0x24) { printf("[ERROR] Height must be less than 36 (0x24), got %u\n", height); return 0; }
    cvf_data[0] = height;

    uint width = 0;
    if (readBits(fd, 8, &width) == 0) return 0;
    if (width == 0) { printf("[ERROR] Zero length width is not permitted\n"); return 0; }
    if (width >= 0x81) { printf("[ERROR] Width must be less than 129 (0x81), got %u\n", width); return 0; }
    cvf_data[1] = width;

    uint max_frames = 0;
    if (readBits(fd, 0x10, &max_frames) == 0) return 0; // Read 16 bits
    if (max_frames == 0) { printf("[ERROR] Zero frame count is not permitted\n"); return 0; }
    if (max_frames >= 0x801) { printf("[ERROR] Maximum frame count is 2048 (0x800), got %u\n", max_frames); return 0; }
    cvf_data[2] = max_frames; // Store max_frames

    // Allocate array of pointers to frames
    // cvf_data[0x94] is the offset for the frames array pointer
    CVF_Frame **frames_array = (CVF_Frame **)malloc(max_frames * sizeof(CVF_Frame*));
    if (frames_array == NULL) {
        return 0;
    }
    memset(frames_array, 0, max_frames * sizeof(CVF_Frame*));
    cvf_data[0x94] = (uintptr_t)frames_array; // Store pointer to frames array

    // cvf_data[0x93] (current_frame_count) is initialized to 0 by memset in initCVF.
    return 1; // Success
}

// Function: renderCVF
void renderCVF(int fd) {
    if (fd == 0) {
        return;
    }

    uint magic = 0;
    if (readBits(fd, 0x20, &magic) == 0) {
        return;
    }

    if (magic != 0x435646) { // "CVF" magic
        printf("[ERROR] Invalid magic: 0x%x\n", magic);
        return;
    }

    void *cvf_data = initCVF();
    if (cvf_data == NULL) {
        printf("[ERROR] Failed to initialize CVF structure.\n");
        return;
    }

    uint section_type = 0;
    int success = 1; // Flag to control the loop

    while (success) {
        if (readBits(fd, 0x10, &section_type) == 0) { // Read section type
            break; // End of file or read error, exit loop
        }

        switch (section_type) {
            case 0x1111: // Header
                success = parseCVFHeader(fd, (uint *)cvf_data);
                break;
            case 0x2222: // Name
                success = parseCVFName(fd, (int *)cvf_data);
                break;
            case 0x3333: // Description
                success = parseCVFDescription(fd, (int *)cvf_data);
                break;
            case 0x4444: // Custom Pixel Dictionary 0
            case 0x4445: // Custom Pixel Dictionary 1
            case 0x4446: // Custom Pixel Dictionary 2
            case 0x4447: // Custom Pixel Dictionary 3
            case 0x4448: // Custom Pixel Dictionary 4
            case 0x4449: // Custom Pixel Dictionary 5
            case 0x444A: // Custom Pixel Dictionary 6
            case 0x444B: // Custom Pixel Dictionary 7
                success = parseCVFPixelDict(fd, (int *)cvf_data, section_type - 0x4444);
                break;
            case 0x5555: // Frame
                success = parseCVFFrame(fd, (int *)cvf_data);
                break;
            default:
                printf("[ERROR] Invalid section type: 0x%x\n", section_type);
                success = 0; // Unrecognized section, stop processing
                break;
        }

        if (!success) {
            printf("[ERROR] Parsing section 0x%x failed.\n", section_type);
            break; // Exit loop on parsing error
        }
    }

    // After loop, if parsing was successful (or gracefully ended), play video
    // The condition `section_type == 0` handles the case where `readBits` failed
    // on the very first attempt to read a section type, meaning an empty or malformed file.
    if (success || (section_type == 0 && (uintptr_t)((uint*)cvf_data)[0x94] != 0)) { // Only play if frames array was initialized
        playVideo((uint *)cvf_data);
    }
    freeCVF(cvf_data);
}

// Function: initCVF
void *initCVF(void) {
    void *cvf_data = malloc(0x254); // Assuming 0x254 bytes is the size of the CVF structure
    if (cvf_data != NULL) {
        memset(cvf_data, 0, 0x254);
    }
    return cvf_data;
}

// Main function to demonstrate the code
int main() {
    // In a real scenario, you would open a file:
    // int fd = open("video.cvf", O_RDONLY);
    // if (fd == -1) { perror("Failed to open file"); return 1; }
    // renderCVF(fd);
    // close(fd);

    // For this dummy implementation, we pass a non-zero integer as a dummy file descriptor.
    // The dummy `readBits` will simulate data reading.
    // The `receive` function (used by receiveWrapper for playVideo's "press a key" prompt)
    // will read from stdin (fd 0).
    renderCVF(1); // Using 1 as a dummy file descriptor
    return 0;
}