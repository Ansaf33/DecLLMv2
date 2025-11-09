#include <stdio.h>  // For main, potential debugging (not strictly needed for snippet)
#include <stdlib.h> // For rand(), srand(), malloc, free
#include <string.h> // For strlen, strcmp, strncpy, strstr, strncat
#include <stddef.h> // For size_t
#include <time.h>   // For time()

// --- External Function Declarations ---
// These functions are assumed to be defined elsewhere and are essential for the code to link.
// Signatures are inferred from usage in the provided snippet.
extern int transmit_with_term(int fd, const char *buffer, size_t length, int flags);
extern int receive_all(int fd, void *buffer, size_t length, int flags);
extern int recv_until_delim(int fd, char *buffer, size_t max_len);
extern int allocate(size_t size, int flags, int *out_ptr); // out_ptr receives allocated address
extern void deallocate(int ptr, size_t size);
extern int receive_with_term(int fd, char *buffer, size_t length, unsigned int *out_received_bytes);

// --- Global Data Declarations ---
// These are likely constant string literals identified by the decompiler.
const char DAT_00012000[] = "//";         // Assumed for path canonicalization (e.g., removing double slashes)
const char DAT_00012004[] = "$SRC$\x06"; // Marker for source code block start
const char DAT_0001200b[] = "$END$\x06"; // Marker for source code block end

// Global buffer for received data, sized based on 0xFFFF usage in recv_until_delim.
// Max_len 0xFFFF means buffer must be at least 0xFFFF + 1 for null termination.
char buf_recv_tmp[0xFFFF + 1];

// --- Helper Function for strpos (Decompiler Artifact) ---
// The original code used 'strpos', which is not a standard C function.
// It's likely a wrapper for strstr.
char *strpos(const char *haystack, const char *needle) {
    return (char *)strstr(haystack, needle);
}

// Function: random_alpha_lower
// Fills a buffer with 'length' random lowercase alphabetic characters.
// The buffer should be able to hold 'length' characters.
void random_alpha_lower(char *buffer, unsigned int length) {
    for (unsigned int i = 0; i < length; ++i) {
        buffer[i] = (char)(rand() % 26 + 'a');
    }
}

// Function: canonicalize_path
// param_1: destination buffer (char *) - must be large enough.
// param_2: source path (const char *)
// param_3: length of source path (size_t)
// Canonicalizes a path by removing segments identified by DAT_00012000 (e.g., "//").
// Returns 0 on success, 0xf on error (e.g., segment too long).
int canonicalize_path(char *dest, const char *src, size_t src_len) {
    const char *current_segment_start = src;
    char *separator_pos;
    const size_t MAX_SEGMENT_LEN = 256; // Based on 0x101 (257) checks in original code.

    dest[0] = '\0'; // Initialize destination string

    separator_pos = strpos(current_segment_start, DAT_00012000);

    size_t segment_len;
    if (separator_pos == NULL) {
        segment_len = src_len;
    } else {
        segment_len = separator_pos - current_segment_start;
    }

    if (segment_len >= MAX_SEGMENT_LEN) {
        return 0xf; // Error: initial segment too long
    }
    strncat(dest, current_segment_start, segment_len);

    if (separator_pos != NULL) {
        // Loop through subsequent segments
        while (1) {
            current_segment_start = separator_pos + strlen(DAT_00012000); // Skip the separator
            separator_pos = strpos(current_segment_start, DAT_00012000);

            if (separator_pos == NULL) {
                // No more separators, copy the rest of the string
                segment_len = (src + src_len) - current_segment_start;
                if (segment_len >= MAX_SEGMENT_LEN) { // Check length of final segment
                    return 0xf;
                }
                strncat(dest, current_segment_start, segment_len);
                break; // Exit loop
            }

            // Separator found, copy segment up to next separator
            segment_len = separator_pos - current_segment_start;
            if (segment_len >= MAX_SEGMENT_LEN) { // Check segment length
                return 0xf;
            }
            strncat(dest, current_segment_start, segment_len);
        }
    }
    return 0; // Success
}

// Function: request_document
// param_1: document_name (char *) - The name of the document to request.
// param_2: recursion_depth (unsigned int) - Current recursion level.
// Requests a document, processes its content for embedded source blocks,
// and recursively requests those.
// Returns 0 on success, or an error code.
int request_document(char *document_name, unsigned int recursion_depth) {
    // Max recursion_depth check (0x3e9 = 999 + 1 = 1000)
    if (recursion_depth >= 0x3e9) {
        return 0x10; // Error: recursion depth exceeded
    }

    size_t name_len = strlen(document_name);
    int ret_code = transmit_with_term(1, document_name, name_len, 0);

    if (ret_code != 0) {
        return ret_code;
    }

    unsigned short doc_size = 0;
    ret_code = receive_all(0, &doc_size, sizeof(doc_size), 0);
    if (ret_code != 0) {
        return ret_code;
    }

    // Check if received name matches requested name
    int recv_name_len = recv_until_delim(0, buf_recv_tmp, 0xFFFF);
    if (recv_name_len < 0 || strcmp(document_name, buf_recv_tmp) != 0) {
        return -1; // Error or name mismatch
    }

    int allocated_doc_ptr_val = 0;
    // Allocate buffer for the document content + 1 for null terminator
    ret_code = allocate(doc_size + 1, 0, &allocated_doc_ptr_val);
    if (ret_code != 0) {
        return ret_code;
    }
    char *allocated_doc_ptr = (char*)allocated_doc_ptr_val;

    unsigned int received_bytes = 0;
    ret_code = receive_with_term(0, allocated_doc_ptr, doc_size, &received_bytes);
    if (ret_code != 0 || doc_size != received_bytes) {
        deallocate(allocated_doc_ptr_val, doc_size + 1);
        return ret_code != 0 ? ret_code : -1; // Error or size mismatch
    }

    // Ensure the received document is null-terminated for string operations
    allocated_doc_ptr[doc_size] = '\0';

    char *current_scan_ptr = allocated_doc_ptr;
    char *src_start_marker;
    char *src_end_marker;

    // Buffer for canonicalized path (original `local_13c` was 63*4 = 252 bytes)
    // MAX_SEGMENT_LEN is 256, so a buffer of 257 (256+1) for null terminator is appropriate.
    char canonical_path_buffer[257];

    // Loop to find and process "$SRC$\x06" ... "$END$\x06" blocks
    while (1) {
        src_start_marker = strpos(current_scan_ptr, DAT_00012004); // Find "$SRC$\x06"

        if (src_start_marker == NULL) {
            break; // No more source blocks found, exit loop
        }

        current_scan_ptr = src_start_marker + strlen(DAT_00012004); // Move past "$SRC$\x06"
        src_end_marker = strpos(current_scan_ptr, DAT_0001200b); // Find "$END$\x06"

        if (src_end_marker == NULL) {
            // Malformed document: found start but no end marker. Stop processing.
            break;
        }

        size_t segment_content_len = src_end_marker - current_scan_ptr;
        // Temporary buffer for the extracted segment.
        // MAX_SEGMENT_LEN is 256, so a 257-byte buffer is safe.
        char temp_segment_buffer[257];

        if (segment_content_len >= sizeof(temp_segment_buffer)) {
             // Segment too large for temp buffer or canonicalization; treat as error.
             ret_code = 0xf; // Error code for path too long
             break;
        }
        strncpy(temp_segment_buffer, current_scan_ptr, segment_content_len);
        temp_segment_buffer[segment_content_len] = '\0'; // Null-terminate the segment

        // Call canonicalize_path with the extracted segment
        ret_code = canonicalize_path(canonical_path_buffer, temp_segment_buffer, segment_content_len);
        if (ret_code != 0) {
            break; // Error in canonicalization
        }

        // Recursive call to request_document with the canonicalized path and incremented depth
        ret_code = request_document(canonical_path_buffer, recursion_depth + 1);
        if (ret_code != 0) {
            break; // Error in recursive call
        }

        current_scan_ptr = src_end_marker + strlen(DAT_0001200b); // Move past "$END$\x06"
    }

    deallocate(allocated_doc_ptr_val, doc_size + 1);
    return ret_code;
}

// Function: main
// Entry point of the program.
int main(void) {
    // Seed the random number generator once at program start.
    srand(time(NULL));

    char initial_document_name[16]; // Buffer for a 15-character name + null terminator.
    
    // Generate 15 random lowercase alphabetic characters.
    random_alpha_lower(initial_document_name, 15);
    initial_document_name[15] = '\0'; // Null-terminate the string.

    // Request the initial document with recursion depth 0.
    request_document(initial_document_name, 0);
    
    return 0; // Indicate successful execution.
}