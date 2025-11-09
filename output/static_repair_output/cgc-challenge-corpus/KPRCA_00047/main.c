#include <stdio.h>   // For fprintf, NULL, stderr, stdout, va_list, vsnprintf
#include <stdlib.h>  // For calloc, free, exit, strtol
#include <string.h>  // For memchr, memset, strncpy, strlen, strncmp
#include <ctype.h>   // For isdigit, isspace
#include <stdbool.h> // For bool
#include <stdarg.h>  // For va_list in fdprintf

// --- Forward Declarations for External/Mock Functions ---
// Assuming transmit and receive functions take an fd, a buffer, a count, and a pointer
// to store the actual number of bytes transferred. They return 0 on success, -1 on error.
int transmit(int fd, char* buffer, size_t count, size_t* bytes_actual);
int receive(int fd, char* buffer, size_t count, size_t* bytes_actual);

// fdprintf is a custom print function, likely similar to fprintf
// We'll define a simple wrapper for fprintf for compilation.
void fdprintf(int fd, const char* format, ...) {
    va_list args;
    va_start(args, format);
    FILE* stream = (fd == 1) ? stdout : stderr;
    vfprintf(stream, format, args);
    va_end(args);
}

// --- Global Data Declarations ---
// These are placeholders for values that would be defined elsewhere.
// The exact types and values are inferred from usage.

// Define the OCR character template struct
typedef struct {
    unsigned int width;
    unsigned int height;
    char character;
    char pixels[87]; // 96 - sizeof(unsigned int)*2 - sizeof(char) = 96 - 8 - 1 = 87 bytes
} OCRChar; // Total size 0x60 (96 bytes)

// Dummy OCR character templates for compilation
static OCRChar match_objects[27] = {
    {1, 1, 'A', {0}}, // Example: A 1x1 char
    // ... more dummy data if needed for full functionality, for compilation, this is enough
};

static const float DAT_005ccf00 = 1.0f; // A plausible upper bound for match_percent
static const float DAT_005ccf08 = 0.5f; // A plausible threshold for OCR matching

static const char junk[] = "some junk data for checksum";
static const char magic[] = "MAGIC_STRING";

// Assuming DAT_001afd7f is a magic unsigned int value.
// The decompiler output `&DAT_001afd7f` implies it's an address, but it's compared to an `uint`.
// It's likely a constant value used for validation.
static const unsigned int JUNK_MAGIC_VALUE = 0x1afd7f; // Placeholder value

// --- Struct for 2D View ---
typedef struct {
    char* base_ptr;
    int view_width;
    int view_height;
    int image_width;
    int image_height;
    int view_x_offset;
    int view_y_offset;
} ImageView;

// --- Function Implementations ---

// Function: send_n_bytes
int send_n_bytes(int fd, size_t total_bytes, char* buffer) {
    if (total_bytes == 0 || buffer == NULL) {
        return -1;
    }

    size_t bytes_sent_total = 0;
    while (bytes_sent_total < total_bytes) {
        size_t bytes_remaining_in_this_transfer = total_bytes - bytes_sent_total;
        size_t bytes_sent_this_call = 0;
        int ret = transmit(fd, buffer + bytes_sent_total, bytes_remaining_in_this_transfer, &bytes_sent_this_call);

        if (ret != 0) { // transmit returned error
            return -1;
        }
        if (bytes_sent_this_call == 0) { // No progress, break to prevent infinite loop
            break;
        }
        bytes_sent_total += bytes_sent_this_call;
    }
    return bytes_sent_total;
}

// Function: read_n_bytes
int read_n_bytes(int fd, size_t total_bytes, char* buffer) {
    if (total_bytes == 0 || buffer == NULL) {
        return -1;
    }

    size_t bytes_read_total = 0;
    while (bytes_read_total < total_bytes) {
        size_t bytes_remaining_in_this_transfer = total_bytes - bytes_read_total;
        size_t bytes_read_this_call = 0;
        int ret = receive(fd, buffer + bytes_read_total, bytes_remaining_in_this_transfer, &bytes_read_this_call);

        if (ret != 0) { // receive returned error
            return -1;
        }
        if (bytes_read_this_call == 0) { // No progress, break to prevent infinite loop
            break;
        }
        bytes_read_total += bytes_read_this_call;
    }
    return bytes_read_total;
}

// Function: read_until
// Reads bytes into 'buffer' from 'fd' until 'delimiter' is found or 'max_len' bytes are read.
// Null-terminates at the delimiter's position and returns the number of characters *before* the delimiter.
// Returns -1 on error or if max_len is reached without finding delimiter.
int read_until(int fd, unsigned int max_len, char delimiter, char* buffer) {
    if (max_len == 0 || buffer == NULL) {
        return -1;
    }

    unsigned int total_read = 0;
    while (total_read < max_len) {
        ssize_t bytes_read_this_call = read_n_bytes(fd, 1, buffer + total_read); // Read one byte at a time

        if (bytes_read_this_call < 0) {
            return -1; // Error reading
        }
        if (bytes_read_this_call == 0) {
            return -1; // EOF or no data read when expected
        }

        if (buffer[total_read] == delimiter) {
            buffer[total_read] = '\0'; // Null-terminate at the delimiter position
            return total_read;         // Return the number of bytes read BEFORE the delimiter
        }
        total_read += bytes_read_this_call; // Should be 1
    }
    // If loop finishes, max_len was reached without finding delimiter
    return -1;
}

// Function: read_image
unsigned int read_image(int fd, unsigned int width, unsigned int height, char** image_data_ptr) {
    if (image_data_ptr == NULL) {
        return 0;
    }
    *image_data_ptr = NULL;

    size_t total_image_pixels = (size_t)width * height;
    char* image_data = (char*)calloc(total_image_pixels, 1);
    if (image_data == NULL) {
        fdprintf(2, "DEBUG %s:%d:\tcalloc() failed\n", __FILE__, __LINE__);
        exit(1);
    }

    size_t total_pixels_read = 0;             // Offset into image_data (corresponds to local_10)
    size_t current_line_pixels_accumulated = 0; // Pixels accumulated for the current logical line (corresponds to local_14)
    bool overall_success = true;

    while (total_pixels_read < total_image_pixels) {
        int pixels_read_this_call; // Pixels read by read_until this call (corresponds to local_18)
        char* current_buffer_target;
        unsigned int max_read_limit_for_call;

        if (current_line_pixels_accumulated == 0) { // Starting a new image line or a new segment for a wrapped line
            current_buffer_target = image_data + total_pixels_read;
            max_read_limit_for_call = width + 1; // Allow reading up to `width` characters + potential newline
        } else { // Continuing a wrapped image line
            current_buffer_target = image_data + total_pixels_read + current_line_pixels_accumulated;
            max_read_limit_for_call = (width - current_line_pixels_accumulated) + 1; // Remaining space + potential newline
            if (max_read_limit_for_call == 0) { // Safety check: if current_line_pixels_accumulated already reached width
                pixels_read_this_call = 0; // Treat as no bytes read (error condition)
                goto process_read_result; // Use goto to jump to common error handling
            }
        }

        pixels_read_this_call = read_until(fd, max_read_limit_for_call, '\n', current_buffer_target);

    process_read_result: // Common point for handling pixels_read_this_call
        if (pixels_read_this_call < 0) { // Error reading
            overall_success = false;
            break;
        }
        if (pixels_read_this_call == 0) { // Read 0 bytes, unexpected EOF or empty line
            overall_success = false;
            break;
        }

        // Translate original logic for updating counts, assuming `width` is the target line length
        // Original: `if ((local_14 == 0) || (local_14 + local_18 == param_2 + 1))`
        if (current_line_pixels_accumulated == 0 || (current_line_pixels_accumulated + pixels_read_this_call == width)) {
            // Original: `if ((local_14 == 0) && (param_2 + 1 != local_18))`
            if (current_line_pixels_accumulated == 0 && (width != (unsigned int)pixels_read_this_call)) {
                // Original: `local_14 = local_18 + -1;` (Assuming `pixels_read_this_call` is content length)
                current_line_pixels_accumulated = pixels_read_this_call;
            } else {
                total_pixels_read += width;
                current_line_pixels_accumulated = 0;
            }
        } else {
            // Original: `local_14 = local_14 + local_18 + -1;`
            current_line_pixels_accumulated += pixels_read_this_call;
        }
    }

    if (overall_success && total_pixels_read == total_image_pixels) {
        *image_data_ptr = image_data;
        return height; // Return height on success
    } else {
        free(image_data);
        return 0; // Return 0 on failure
    }
}

// Function: make_2d_view
ImageView* make_2d_view(char* base_ptr, int image_width, int image_height, int view_x_offset, int view_y_offset) {
    ImageView* view = (ImageView*)calloc(1, sizeof(ImageView));
    if (view == NULL) {
        fdprintf(2, "DEBUG %s:%d:\tcalloc() failed\n", __FILE__, __LINE__);
        exit(1);
    }

    view->base_ptr = base_ptr;
    if (view->base_ptr == NULL) { // Original equivalent: `if (*piVar1 == 0)`
        fdprintf(2, "DEBUG %s:%d:\tbase_ptr is NULL\n", __FILE__, __LINE__);
        exit(1);
    }
    view->image_width = image_width;
    view->image_height = image_height;
    view->view_x_offset = view_x_offset;
    view->view_y_offset = view_y_offset;
    view->view_width = image_width - view_x_offset;
    view->view_height = image_height - view_y_offset;

    return view;
}

// Function: index_through_view
int index_through_view(unsigned int x, unsigned int y, ImageView* view, char* output_char) {
    // Check if (x,y) is within the view dimensions
    if (x >= (unsigned int)view->view_width || y >= (unsigned int)view->view_height) {
        return -1;
    }

    // Calculate absolute coordinates in the original image
    unsigned int abs_x = x + view->view_x_offset;
    unsigned int abs_y = y + view->view_y_offset;

    // Check if absolute coordinates are within the original image dimensions
    if (abs_x >= (unsigned int)view->image_width || abs_y >= (unsigned int)view->image_height) {
        return -1;
    }

    // Access the character
    *output_char = view->base_ptr[abs_y * view->image_width + abs_x];
    return 0;
}

// Function: parse_dimensions
int parse_dimensions(char* input_str, size_t str_len, int* width_ptr, int* height_ptr) {
    char* current_ptr = input_str;
    char* end_ptr = input_str + str_len;

    // Skip leading whitespace
    while (current_ptr < end_ptr && isspace((unsigned char)*current_ptr)) {
        current_ptr++;
    }

    // Check if we ran out of string or hit non-digit
    if (current_ptr == end_ptr || !isdigit((unsigned char)*current_ptr)) {
        return -1;
    }

    char* parse_end;
    long width_val = strtol(current_ptr, &parse_end, 10);
    if (width_val == 0 || parse_end == current_ptr) { // 0 is invalid dimension, or no digits parsed
        return -1;
    }
    *width_ptr = (int)width_val;
    current_ptr = parse_end;

    // Skip whitespace after first number
    while (current_ptr < end_ptr && isspace((unsigned char)*current_ptr)) {
        current_ptr++;
    }

    // Check if we ran out of string or hit non-digit before second number
    if (current_ptr == end_ptr || !isdigit((unsigned char)*current_ptr)) {
        return -1;
    }

    long height_val = strtol(current_ptr, &parse_end, 10);
    if (height_val == 0 || parse_end == current_ptr) { // 0 is invalid dimension, or no digits parsed
        return -1;
    }
    *height_ptr = (int)height_val;
    current_ptr = parse_end;

    // Skip trailing whitespace
    while (current_ptr < end_ptr && isspace((unsigned char)*current_ptr)) {
        current_ptr++;
    }

    // If there's any non-whitespace character left, it's an error
    if (current_ptr != end_ptr) {
        return -1;
    }

    return 0;
}

// Function: match_percent
long double match_percent(ImageView* image_view, OCRChar* ocr_char_template) {
    unsigned int match_count = 0;
    unsigned int total_pixels_in_template = ocr_char_template->width * ocr_char_template->height;
    char image_pixel;

    for (unsigned int y = 0; y < ocr_char_template->height; y++) {
        for (unsigned int x = 0; x < ocr_char_template->width; x++) {
            if (index_through_view(x, y, image_view, &image_pixel) == 0) { // If in bounds
                // Template pixel data starts after width (4 bytes), height (4 bytes), character (1 byte)
                char template_pixel = *((char*)ocr_char_template + sizeof(unsigned int) * 2 + sizeof(char) + (y * ocr_char_template->width + x));
                if (template_pixel == image_pixel) {
                    match_count++;
                }
            }
        }
    }

    if (total_pixels_in_template == 0) {
        return 0.0L; // Avoid division by zero
    }
    return (long double)match_count / (long double)total_pixels_in_template;
}

// Function: find_matching_object
OCRChar* find_matching_object(ImageView* image_view, float threshold) {
    float best_match_percent = 0.0f;
    OCRChar* best_match_template = NULL;

    for (unsigned int i = 0; i < 27; i++) { // Iterates through 27 possible templates
        // Check if template dimensions fit within the current view dimensions
        if (match_objects[i].height <= (unsigned int)image_view->view_height &&
            match_objects[i].width <= (unsigned int)image_view->view_width) {

            long double current_match_ld = match_percent(image_view, &match_objects[i]);
            float current_match_f = (float)current_match_ld;

            // Original logic: `(local_10 <= fVar1) && (fVar1 < (float)DAT_005ccf00)`
            if (best_match_percent <= current_match_f && current_match_f < DAT_005ccf00) {
                best_match_template = &match_objects[i];
                best_match_percent = current_match_f;
            }
        }
    }

    if (best_match_percent < threshold) {
        best_match_template = NULL;
    }
    return best_match_template;
}

// Function: perform_ocr
char* perform_ocr(char* image_data, unsigned int image_width, unsigned int image_height, float match_threshold) {
    char recognized_chars_buffer[64]; // Max 64 recognized characters
    size_t recognized_count = 0;

    unsigned int current_x = 0;
    unsigned int current_y = 0;

    while (current_y < image_height && current_x < image_width) {
        ImageView* current_view = make_2d_view(image_data, image_width, image_height, current_x, current_y);
        OCRChar* matched_char_template = find_matching_object(current_view, match_threshold);
        free(current_view); // Free the view after use

        if (matched_char_template == NULL) {
            // No character recognized at current position. Move to the next pixel.
            current_x++;
            if (current_x >= image_width) {
                current_x = 0;
                current_y++;
            }
        } else {
            // Character recognized. Add to buffer.
            if (recognized_count < sizeof(recognized_chars_buffer) - 1) { // Leave space for null terminator
                recognized_chars_buffer[recognized_count++] = matched_char_template->character;
            } else {
                // Buffer full. Stop recognizing.
                break;
            }

            // Advance position by the width of the recognized character
            current_x += matched_char_template->width;
            if (current_x >= image_width) {
                current_x = 0;
                current_y += matched_char_template->height;
            }
        }
    }

    char* result_string = NULL;
    if (recognized_count > 0) {
        result_string = (char*)calloc(recognized_count + 1, 1); // +1 for null terminator
        if (result_string == NULL) {
            fdprintf(2, "DEBUG %s:%d:\tcalloc() failed\n", __FILE__, __LINE__);
            exit(1);
        }
        strncpy(result_string, recognized_chars_buffer, recognized_count);
        result_string[recognized_count] = '\0'; // Ensure null-termination
    }
    return result_string;
}

// Function: check_junk
bool check_junk(void) {
    size_t junk_len = strlen(junk);
    unsigned int accumulator = 0;

    for (unsigned int i = 0; i < 16; i++) {
        for (unsigned int j = 0; j < junk_len; j++) {
            if (256 < i + j) { // 0x100
                accumulator ^= (unsigned int)(unsigned char)junk[i + j - 256] << ((j + (i & 0x3f)) & 0x1f);
            }
            if (2048 < i + j) { // 0x800
                accumulator ^= (unsigned int)(unsigned char)(junk[i + j - 2048] | junk[i + j]);
            }
            if (30000 < i + j) {
                accumulator ^= (unsigned int)(unsigned char)(junk[i + j - 30000] | junk[i + j]);
            }
        }
    }
    return accumulator == JUNK_MAGIC_VALUE;
}

// Function: main
int main(void) {
    if (!check_junk()) {
        fdprintf(2, "DEBUG %s:%d:\tbad junk\n", __FILE__, __LINE__);
        exit(1);
    }
    fdprintf(1, "Welcome to the super awesome OCR engine! Enter your input to have it OCR'd\n");

    int input_fd = 0; // Standard input
    char input_buffer[32];

    // Read magic string
    memset(input_buffer, 0, sizeof(input_buffer));
    int bytes_read_magic = read_until(input_fd, sizeof(input_buffer), '\n', input_buffer);
    if (bytes_read_magic < 0) {
        fdprintf(2, "DEBUG %s:%d:\tbad line (magic)\n", __FILE__, __LINE__);
        exit(1);
    }

    size_t magic_input_len = strlen(input_buffer);
    size_t expected_magic_len = strlen(magic);

    if (magic_input_len == expected_magic_len && strncmp(input_buffer, magic, magic_input_len) == 0) {
        // Read dimensions
        memset(input_buffer, 0, sizeof(input_buffer));
        int bytes_read_dims = read_until(input_fd, sizeof(input_buffer), '\n', input_buffer);
        if (bytes_read_dims < 0) {
            fdprintf(2, "DEBUG %s:%d:\tbad line (dimensions)\n", __FILE__, __LINE__);
            exit(1);
        }

        int image_width, image_height;
        if (parse_dimensions(input_buffer, strlen(input_buffer), &image_width, &image_height) != 0) {
            fdprintf(2, "DEBUG %s:%d:\tbad dimensions\n", __FILE__, __LINE__);
            exit(1);
        }

        char* image_data = NULL;
        unsigned int actual_height_read = read_image(input_fd, image_width, image_height, &image_data);

        if (actual_height_read == (unsigned int)image_height && image_data != NULL) {
            char* ocr_result = perform_ocr(image_data, image_width, image_height, DAT_005ccf08);
            if (ocr_result == NULL) {
                fdprintf(1, "No characters recognized\n");
            } else {
                fdprintf(1, "Result: %s\n", ocr_result);
                free(ocr_result); // Free allocated result string
            }
            free(image_data); // Free allocated image data
            return 0;
        } else {
            fdprintf(2, "DEBUG %s:%d:\tbad image\n", __FILE__, __LINE__);
            // image_data would have been freed by read_image on failure
            exit(1);
        }
    } else {
        fdprintf(2, "DEBUG %s:%d:\tbad magic\n", __FILE__, __LINE__);
        exit(1);
    }
    return 0; // Should not be reached if exit() is called on errors
}

// --- Dummy Implementations for `transmit` and `receive` for Compilation ---
// In a real scenario, these would be actual system calls (e.g., `write` and `read`)
// or wrappers around them. Here, they just simulate I/O.
#include <errno.h> // For simulating errors

int transmit(int fd, char* buffer, size_t count, size_t* bytes_actual) {
    // Simulate writing to a file descriptor
    // For simplicity, this mock just "succeeds" by writing all bytes.
    // In a real system, this would involve `write(fd, buffer, count)`.
    if (fd < 0 || buffer == NULL || bytes_actual == NULL) {
        // Invalid arguments
        errno = EBADF; // Example error
        return -1;
    }
    *bytes_actual = count; // Assume all bytes are sent
    // fprintf(stderr, "MOCK TRANSMIT: fd=%d, count=%zu, actual=%zu\n", fd, count, *bytes_actual);
    return 0; // Success
}

int receive(int fd, char* buffer, size_t count, size_t* bytes_actual) {
    // Simulate reading from a file descriptor
    // For simplicity, this mock just "succeeds" by reading 1 byte if count > 0.
    // In a real system, this would involve `read(fd, buffer, count)`.
    if (fd < 0 || buffer == NULL || bytes_actual == NULL) {
        // Invalid arguments
        errno = EBADF; // Example error
        return -1;
    }

    if (count == 0) {
        *bytes_actual = 0;
        return 0;
    }

    // For stdin (fd 0), read from actual stdin
    if (fd == 0) {
        int c = getchar();
        if (c == EOF) {
            *bytes_actual = 0; // EOF
            return 0;
        }
        buffer[0] = (char)c;
        *bytes_actual = 1;
        return 0;
    }
    
    // For other FDs, just simulate reading
    // This part is a simplified mock; a robust mock would need more logic.
    buffer[0] = 'X'; // Dummy byte
    *bytes_actual = 1; // Read one byte
    // fprintf(stderr, "MOCK RECEIVE: fd=%d, count=%zu, actual=%zu\n", fd, count, *bytes_actual);
    return 0; // Success
}