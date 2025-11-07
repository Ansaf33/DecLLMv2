#include <stdio.h>    // For fprintf, stdout, stderr, va_list, vfprintf
#include <stdlib.h>   // For calloc, free, exit, strtol
#include <string.h>   // For memset, strlen, strncpy, strncmp, memchr
#include <stdbool.h>  // For bool type
#include <ctype.h>    // For isdigit, isspace
#include <stdarg.h>   // For va_list, va_start, va_end

// --- Dummy External Functions/Data (for compilation) ---
// These are placeholders for functions and global variables that would typically
// be provided by the operating system (e.g., system calls for transmit/receive)
// or other source files in a complete project.
extern int transmit(int fd, char *buf, int count, int *bytes_transferred_out);
extern int receive(int fd, char *buf, int count, int *bytes_transferred_out);

// Global data from original snippet, declared as extern
// Values are placeholders for compilation.
extern const char magic_string[];
extern const char junk[];
extern unsigned int MAGIC_JUNK_VALUE; // Equivalent to &DAT_001afd7f
extern float MAX_MATCH_PERCENT;       // Equivalent to (float)DAT_005ccf00 (likely 1.0f)
extern float OCR_THRESHOLD;           // Equivalent to DAT_005ccf08

// Assuming match_objects_base points to an array of CharTemplate structs.
// DAT_005cf004 and match_objects seem to refer to the same base address for templates.
extern const char match_objects_base[]; 

// Custom fdprintf based on original usage.
// Prints to stdout (fd=1) or stderr (fd=2).
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = -1;
    if (fd == 1) { // stdout
        ret = vfprintf(stdout, format, args);
    } else if (fd == 2) { // stderr
        ret = vfprintf(stderr, format, args);
    }
    va_end(args);
    return ret;
}

// --- Type Definitions ---

// Structure representing a 2D view into an image
typedef struct {
    char *data_ptr;   // [0] Pointer to the raw image data
    int view_width;   // [1] Width of the current view
    int view_height;  // [2] Height of the current view
    int image_width;  // [3] Total width of the original image
    int image_height; // [4] Total height of the original image
    int offset_x;     // [5] X-offset of the view within the original image
    int offset_y;     // [6] Y-offset of the view within the original image
} ImageView;

// Structure representing a character template for OCR matching
// Total size 96 bytes (0x60) based on usage (e.g., `i * 0x60` offset for array elements)
typedef struct {
    int width;             // Width of the character template image
    int height;            // Height of the character template image
    int char_value_as_int; // The ASCII value of the character this template represents
    char data[84];         // Pixel data for the character template (96 - (4+4+4) = 84 bytes)
} CharTemplate;

// --- Function Implementations ---

// Function: send_n_bytes
// Sends `count` bytes from `buffer` to `fd`. Handles partial sends.
// Returns total bytes sent on success, -1 on error.
int send_n_bytes(int fd, char *buffer, int count) {
    if (count == 0 || buffer == NULL) {
        return -1; // Invalid arguments
    }

    int total_sent = 0;
    while (total_sent < count) {
        int bytes_transferred_this_call = 0;
        int ret = transmit(fd, buffer + total_sent, count - total_sent, &bytes_transferred_this_call);
        if (ret != 0) { // Transmit error
            return -1;
        }
        if (bytes_transferred_this_call == 0) { // No bytes sent, prevent infinite loop (e.g., blocked or EOF)
            break;
        }
        total_sent += bytes_transferred_this_call;
    }
    return total_sent;
}

// Function: read_n_bytes
// Reads `count` bytes into `buffer` from `fd`. Handles partial reads.
// Returns total bytes read on success, -1 on error.
int read_n_bytes(int fd, char *buffer, int count) {
    if (count == 0 || buffer == NULL) {
        return -1; // Invalid arguments
    }

    int total_read = 0;
    while (total_read < count) {
        int bytes_transferred_this_call = 0;
        int ret = receive(fd, buffer + total_read, count - total_read, &bytes_transferred_this_call);
        if (ret != 0) { // Receive error
            return -1;
        }
        if (bytes_transferred_this_call == 0) { // No bytes received, EOF or no progress
            break;
        }
        total_read += bytes_transferred_this_call;
    }
    return total_read;
}

// Function: read_until
// Reads bytes from `fd` into `buffer_base` until `terminator` is found or `max_len` bytes are read.
// Null-terminates the buffer at the `terminator` position.
// Returns total bytes read (including the null-ified terminator) on success, -1 on error or `max_len` reached.
int read_until(int fd, unsigned int max_len, char terminator, char *buffer_base) {
    unsigned int total_read_count = 0;
    
    while (total_read_count < max_len) {
        // Attempt to read one byte into the current position in the buffer
        int bytes_just_read = read_n_bytes(fd, buffer_base + total_read_count, 1);
        
        if (bytes_just_read < 0) { // Error reading
            return -1;
        }
        if (bytes_just_read == 0) { // EOF or no progress
            return -1;
        }
        
        // Check if the byte just read is the terminator.
        if (buffer_base[total_read_count] == terminator) {
            buffer_base[total_read_count] = '\0'; // Null-terminate
            return total_read_count + 1; // Return total bytes read (including the terminator's position)
        }
        total_read_count++; // Increment by 1 as we successfully read 1 byte
    }
    
    // Max length reached without finding the terminator.
    return -1; 
}

// Function: read_image
// Reads image data from `fd` based on `width` and `height`.
// Allocates memory for the image and populates it. Handles newline-terminated lines.
// Returns the actual height read on success, 0 on error.
unsigned int read_image(int fd, unsigned int width, int height, void **image_buffer_ptr) {
    if (image_buffer_ptr == NULL || width == 0 || height == 0) {
        return 0; // Invalid arguments
    }

    *image_buffer_ptr = NULL;
    void *img_buffer = calloc(width * height, 1); // Allocate flattened image buffer
    if (img_buffer == NULL) {
        fdprintf(2, "DEBUG %s:%d:\tcalloc() failed\n", __FILE__, __LINE__);
        exit(1); // Exit on memory allocation failure
    }

    unsigned int total_image_bytes_processed = 0; // Accumulates full rows' worth of data
    int current_row_partial_len = 0; // Tracks bytes read into the current logical row, excluding newlines

    bool success_reading_image = false; 

    // Loop until all expected image bytes (`width * height`) are processed
    while (total_image_bytes_processed < width * height) {
        int bytes_read_in_segment;
        
        // Calculate the remaining capacity in the current logical row for data
        // plus one for the potential newline terminator. This is the `max_len` for `read_until`.
        unsigned int remaining_capacity_in_row = (width - current_row_partial_len) + 1;
        
        bytes_read_in_segment = read_until(fd, remaining_capacity_in_row, '\n', 
                                           (char*)img_buffer + total_image_bytes_processed + current_row_partial_len);
        
        if (bytes_read_in_segment < 0) { // Error reading or max_len reached without terminator
            success_reading_image = false;
            break;
        }
        if (bytes_read_in_segment == 0) { // No bytes read, unexpected EOF or no progress
            success_reading_image = false;
            break;
        }

        // `bytes_read_in_segment` includes the terminator (which `read_until` replaces with '\0').
        // So, the actual data length read in this segment is `bytes_read_in_segment - 1`.
        int actual_data_len_in_segment = bytes_read_in_segment - 1; 

        if (current_row_partial_len == 0) { // This is the first segment for the current logical row
            if (actual_data_len_in_segment == (int)width) { // A full row was read in one go
                total_image_bytes_processed += width; // Advance by a full row's worth of data
                current_row_partial_len = 0; // Reset for the next logical row
            } else { // Partial row read (e.g., input line was shorter than the image width)
                current_row_partial_len = actual_data_len_in_segment; // Accumulate the partial length
            }
        } else { // This is a subsequent segment for the current logical row
            if (current_row_partial_len + actual_data_len_in_segment == (int)width) { // Current row is now complete
                total_image_bytes_processed += width; // Advance by a full row's worth of data
                current_row_partial_len = 0; // Reset for the next logical row
            } else { // Current row is still partial
                current_row_partial_len += actual_data_len_in_segment; // Accumulate more partial length
            }
        }
    }

    // After the loop, check if all expected image bytes were processed.
    if (total_image_bytes_processed == width * height) {
         success_reading_image = true; // All bytes processed implies success if no error occurred earlier
    }

    if (success_reading_image) {
        *image_buffer_ptr = img_buffer;
        return total_image_bytes_processed / width; // Return the actual height (number of full rows)
    } else {
        free(img_buffer); // Free allocated memory on failure
        return 0;
    }
}

// Function: make_2d_view
// Creates and initializes an ImageView structure.
// Returns a pointer to the newly allocated ImageView on success, exits on failure.
ImageView *make_2d_view(char *image_data, int image_width, int image_height, int offset_x, int offset_y) {
    ImageView *view = (ImageView *)calloc(1, sizeof(ImageView));
    if (view == NULL) {
        fdprintf(2, "DEBUG %s:%d:\tcalloc() failed\n", __FILE__, __LINE__);
        exit(1); // Exit on memory allocation failure
    }
    
    if (image_data == NULL) {
        fdprintf(2, "DEBUG %s:%d:\timage_data is NULL\n", __FILE__, __LINE__);
        free(view); // Free the allocated view struct
        exit(1); // Original code exits on this condition
    }

    view->data_ptr = image_data;
    view->image_width = image_width;
    view->image_height = image_height;
    view->offset_x = offset_x;
    view->offset_y = offset_y;
    view->view_width = image_width - offset_x; // Calculate viewable width
    view->view_height = image_height - offset_y; // Calculate viewable height
    
    return view;
}

// Function: index_through_view
// Retrieves a character from the image through a given view at relative coordinates (x, y).
// Stores the character in `out_char`.
// Returns 0 on success, -1 if coordinates are out of view or image bounds.
int index_through_view(unsigned int x, unsigned int y, ImageView *view, char *out_char) {
    // Check if (x,y) is within the view's dimensions
    if (x >= (unsigned int)view->view_width || y >= (unsigned int)view->view_height) {
        return -1; // Out of current view bounds
    }

    // Calculate absolute coordinates in the original image
    unsigned int abs_x = x + view->offset_x;
    unsigned int abs_y = y + view->offset_y;

    // Check if absolute coordinates are within the original image dimensions
    if (abs_x >= (unsigned int)view->image_width || abs_y >= (unsigned int)view->image_height) {
        return -1; // Out of original image bounds
    }

    // Access the character using 2D to 1D array mapping
    *out_char = view->data_ptr[abs_y * view->image_width + abs_x];
    return 0;
}

// Function: parse_dimensions
// Parses a string `input_str` of `str_len` length to extract two integer dimensions (width and height).
// Stores results in `width_out` and `height_out`.
// Returns 0 on success, -1 on parsing error.
int parse_dimensions(char *input_str, int str_len, int *width_out, int *height_out) {
    char *current_ptr = input_str;
    char *end_ptr = input_str + str_len;
    char *next_token_ptr;
    long val;

    // Skip leading whitespace
    while (current_ptr < end_ptr && isspace((unsigned char)*current_ptr)) {
        current_ptr++;
    }

    // Check if there's anything left to parse
    if (current_ptr >= end_ptr || *current_ptr == '\0') {
        return -1;
    }

    // Parse the first number (width)
    if (!isdigit((unsigned char)*current_ptr)) {
        return -1; // Must start with a digit
    }
    val = strtol(current_ptr, &next_token_ptr, 10);
    if (val <= 0 || next_token_ptr == current_ptr) { // strtol failed, parsed 0, or negative
        return -1;
    }
    *width_out = (int)val;
    current_ptr = next_token_ptr;

    // Skip whitespace between numbers
    while (current_ptr < end_ptr && isspace((unsigned char)*current_ptr)) {
        current_ptr++;
    }

    // Check if there's anything left to parse for the second number
    if (current_ptr >= end_ptr || *current_ptr == '\0') {
        return -1; // Missing second dimension
    }

    // Parse the second number (height)
    if (!isdigit((unsigned char)*current_ptr)) {
        return -1; // Must start with a digit
    }
    val = strtol(current_ptr, &next_token_ptr, 10);
    if (val <= 0 || next_token_ptr == current_ptr) { // strtol failed, parsed 0, or negative
        return -1;
    }
    *height_out = (int)val;
    current_ptr = next_token_ptr;

    // Skip trailing whitespace
    while (current_ptr < end_ptr && isspace((unsigned char)*current_ptr)) {
        current_ptr++;
    }

    // Ensure no extra characters after the dimensions
    if (current_ptr < end_ptr && *current_ptr != '\0') {
        return -1;
    }

    return 0; // Success
}

// Function: match_percent
// Calculates the percentage of matching pixels between an `image_view` and a `object_template`.
// Only compares pixels within the `object_template`'s dimensions that are also visible in the `image_view`.
// Returns the match percentage as a long double.
long double match_percent(ImageView *image_view, const CharTemplate *object_template) {
    unsigned int matches = 0;
    unsigned int total_pixels_compared = 0;

    for (unsigned int y = 0; y < (unsigned int)object_template->height; y++) {
        for (unsigned int x = 0; x < (unsigned int)object_template->width; x++) {
            char image_char;
            int ret = index_through_view(x, y, image_view, &image_char);
            if (ret == 0) { // If pixel (x,y) within template is also visible in the image_view
                total_pixels_compared++;
                // Compare with the corresponding pixel in the character template's data
                if (object_template->data[y * object_template->width + x] == image_char) {
                    matches++;
                }
            }
        }
    }
    
    if (total_pixels_compared == 0) {
        return 0.0L; // Avoid division by zero
    }
    return (long double)matches / (long double)total_pixels_compared;
}

// Function: find_matching_object
// Iterates through a predefined set of character templates (`match_objects_base`)
// to find the one that best matches the `image_view`.
// Returns a pointer to the best matching `CharTemplate` if its match percentage
// is above `threshold`, otherwise returns NULL.
CharTemplate *find_matching_object(ImageView *image_view, float threshold) {
    float best_match_percent = 0.0f;
    CharTemplate *best_match_template = NULL;

    // Loop through 27 (0x1b) possible character templates
    for (unsigned int i = 0; i < 27; i++) {
        CharTemplate *current_template = (CharTemplate *)((char *)&match_objects_base + i * 0x60);
        
        // Check if the template's dimensions fit within the original image's dimensions
        // (as per original code's logic, comparing against image_width/height, not view_width/height)
        if ((unsigned int)current_template->width <= (unsigned int)image_view->image_width &&
            (unsigned int)current_template->height <= (unsigned int)image_view->image_height) {
            
            long double current_match_percent_ld = match_percent(image_view, current_template);
            float current_match_percent = (float)current_match_percent_ld;

            // Update best match if current template is better and below the maximum allowed percent (likely 100%)
            if (best_match_percent <= current_match_percent && current_match_percent < MAX_MATCH_PERCENT) {
                best_match_template = current_template;
                best_match_percent = current_match_percent;
            }
        }
    }

    // If a best match was found but its percentage is below the threshold, discard it.
    if (best_match_template != NULL && best_match_percent < threshold) {
        best_match_template = NULL;
    }
    return best_match_template;
}

// Function: perform_ocr
// Performs Optical Character Recognition on the `image_data_ptr` of given `image_width` and `image_height`.
// It scans the image, attempting to match character templates.
// Returns a dynamically allocated string of recognized characters on success, or NULL if no characters are recognized.
char *perform_ocr(char *image_data_ptr, unsigned int image_width, unsigned int image_height, float threshold) {
    char ocr_result_buffer[64]; // Static buffer for recognized characters (max 63 chars + null)
    size_t result_len = 0;
    char *result_string = NULL;

    unsigned int current_x = 0; // Current x-coordinate for scanning
    unsigned int current_y = 0; // Current y-coordinate for scanning

    // Iterate through the image pixels
    while (current_y < image_height && current_x < image_width) {
        // Create a view for the current potential character position
        ImageView *current_char_view = make_2d_view(image_data_ptr, image_width, image_height, current_x, current_y);
        
        CharTemplate *found_template = find_matching_object(current_char_view, threshold);
        
        free(current_char_view); // Free the ImageView struct (not the image data it points to)

        if (found_template == NULL) {
            // No character recognized at the current position. Advance to the next pixel.
            current_x++;
            if (current_x >= image_width) { // If end of row, move to next line
                current_x = 0;
                current_y++;
            }
        } else {
            // Character recognized. Add it to the result buffer.
            if (result_len < sizeof(ocr_result_buffer) - 1) { // Ensure space for null terminator
                ocr_result_buffer[result_len++] = (char)found_template->char_value_as_int;
            } else {
                // Buffer full, stop processing further characters
                break;
            }

            // Advance scanning position by the width of the recognized character
            current_x += found_template->width;
            // If new x-position goes beyond image width, wrap to the next line,
            // advancing y by the height of the recognized character
            if (current_x >= image_width) {
                current_x = 0;
                current_y += found_template->height;
            }
        }
    }

    if (result_len == 0) {
        result_string = NULL; // No characters recognized
    } else {
        result_string = (char *)calloc(result_len + 1, 1); // Allocate memory for result string (+1 for null terminator)
        if (result_string == NULL) {
            fdprintf(2, "DEBUG %s:%d:\tcalloc() failed\n", __FILE__, __LINE__);
            exit(1); // Exit on memory allocation failure
        }
        strncpy(result_string, ocr_result_buffer, result_len);
        result_string[result_len] = '\0'; // Ensure null termination
    }
    return result_string;
}

// Function: check_junk
// Performs a series of bitwise operations on a global `junk` array
// and compares the final calculated value against a magic constant.
// This is likely an anti-tampering or integrity check.
// Returns true if the calculated value matches the magic constant, false otherwise.
bool check_junk(void) {
    size_t junk_len = strlen(junk);
    unsigned int calculated_junk_val = 0; 

    // The logic below is highly specific to the original binary's architecture and compiler.
    // It's preserved as-is, assuming its correctness for the intended environment.
    for (unsigned int i = 0; i < 0x10; i++) {
        for (unsigned int j = 0; j < junk_len; j++) {
            if (0x100 < i + j) {
                calculated_junk_val ^= ((unsigned int)(unsigned char)junk[i + j - 0x100] << ((j + (i & 0x3f)) & 0x1f));
            }
            if (0x800 < i + j) {
                calculated_junk_val ^= ((unsigned int)(unsigned char)(junk[i + j - 0x800] | junk[i + j]));
            }
            if (30000 < i + j) {
                calculated_junk_val ^= ((unsigned int)(unsigned char)(junk[i + j - 30000] | junk[i + j]));
            }
        }
    }
    // Compare the final calculated value with the magic constant.
    return calculated_junk_val == MAGIC_JUNK_VALUE;
}

// Function: main
// Entry point of the program. Handles user interaction, image reading, and OCR.
int main(void) {
    // Assuming file descriptor 0 is stdin, 1 is stdout, 2 is stderr
    int stdin_fd = 0; 
    int width, height;
    void *image_buffer = NULL; // Pointer to the allocated image data
    char *ocr_result = NULL;   // Pointer to the dynamically allocated OCR result string

    // Perform initial integrity check
    if (!check_junk()) {
        fdprintf(2, "DEBUG %s:%d:\tbad junk\n", __FILE__, __LINE__);
        exit(1);
    }

    fdprintf(1, "Welcome to the super awesome OCR engine! Enter your input to have it OCR'd\n");

    char input_line[32]; // Buffer to read input lines (max 31 chars + null)
    
    // Read the first line (expected to be a magic string)
    memset(input_line, 0, sizeof(input_line));
    int bytes_read_line = read_until(stdin_fd, sizeof(input_line) - 1, '\n', input_line); 
    
    if (bytes_read_line < 0) { // Error or max_len reached without terminator
        fdprintf(2, "DEBUG %s:%d:\tbad line\n", __FILE__, __LINE__);
        exit(1);
    }

    // Compare the read line with the magic string
    if (strlen(input_line) == strlen(magic_string)) { 
        if (strncmp(input_line, magic_string, strlen(input_line)) == 0) {
            // Magic string matched. Read next line for image dimensions.
            memset(input_line, 0, sizeof(input_line));
            bytes_read_line = read_until(stdin_fd, sizeof(input_line) - 1, '\n', input_line);
            if (bytes_read_line < 0) {
                fdprintf(2, "DEBUG %s:%d:\tbad line\n", __FILE__, __LINE__);
                exit(1);
            }

            // Parse width and height from the input line
            if (parse_dimensions(input_line, strlen(input_line), &width, &height) != 0) {
                fdprintf(2, "DEBUG %s:%d:\tbad dimensions\n", __FILE__, __LINE__);
                exit(1);
            }
            
            // Read the image data
            unsigned int actual_height_read = read_image(stdin_fd, (unsigned int)width, height, &image_buffer);
            
            // Check if image was read successfully and buffer is valid
            if (actual_height_read == (unsigned int)height && image_buffer != NULL) {
                // Perform OCR on the read image
                ocr_result = perform_ocr((char*)image_buffer, (unsigned int)width, (unsigned int)height, OCR_THRESHOLD); 
                if (ocr_result == NULL) {
                    fdprintf(1, "No characters recognized\n");
                } else {
                    fdprintf(1, "Result: %s\n", ocr_result);
                    free(ocr_result); // Free the allocated OCR result string
                }
                free(image_buffer); // Free the allocated image buffer
                return 0; // Successful execution
            } else {
                fdprintf(2, "DEBUG %s:%d:\tbad image\n", __FILE__, __LINE__);
                free(image_buffer); // Free image buffer if it was partially allocated
                exit(1);
            }
        }
    }
    // If magic string did not match or other errors occurred during initial checks
    fdprintf(2, "DEBUG %s:%d:\tbad magic\n", __FILE__, __LINE__);
    exit(1);
}