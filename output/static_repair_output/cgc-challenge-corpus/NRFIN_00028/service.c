#include <stdio.h>    // For debugging (printf) and potentially stderr
#include <stdlib.h>   // For calloc, free, exit
#include <string.h>   // For strcpy, strlen, memset, strncmp
#include <unistd.h>   // For ssize_t, potentially recv
#include <sys/socket.h> // For recv

// Ghidra type replacements
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned int undefined4; // Represents a 4-byte untyped value
typedef unsigned char undefined;  // Represents a 1-byte untyped value
typedef int (*code)(void);       // Function pointer type for param_1[2]

// Dummy declarations for external functions to ensure compilation.
// In a real scenario, these would be provided by a library or other source files.
int sendline(void) {
    // Dummy implementation: returns 0 for success
    return 0;
}

void _terminate(void) {
    // Dummy implementation for abnormal termination
    exit(EXIT_FAILURE);
}

// Assuming allocate takes a pointer to a void* to store the allocated address, and the size.
// Returns 0 on success, non-zero on failure.
int allocate(void **ptr_out, size_t size) {
    *ptr_out = malloc(size); // Using malloc as a simple replacement for a custom allocator
    if (*ptr_out == NULL) {
        return 1; // Failure
    }
    memset(*ptr_out, 0, size); // Custom allocators often zero memory, like calloc.
    return 0; // Success
}

// Assuming deallocate takes the pointer to the memory to free, and its size.
// Returns 0 on success, non-zero on failure.
int deallocate(void *ptr, size_t size) {
    // In a real custom allocator, 'size' might be used. For standard free, it's not.
    free(ptr);
    return 0; // Success (no standard way to fail free)
}

int sendall(void) {
    // Dummy implementation: returns 0 for success
    return 0;
}

// Dummy global variables for compilation. Their actual values/contents are unknown.
char *responses[] = {
    "Response 1", "Response 2", "Response 3", "Response 4", "Response 5", "Response 6"
};
char *trusted[] = {
    "Trusted Sig 1", "Trusted Sig 2", "Trusted Sig 3", "Trusted Sig 4", "Trusted Sig 5", "Trusted Sig 6"
};

unsigned int _DAT_4347c00c = 0;
unsigned int _DAT_4347c000 = 0;
unsigned int _DAT_4347c004 = 0;
unsigned int _DAT_4347c008 = 0;
unsigned char DAT_4347c000 = 0;
char DAT_00013127[] = "OK";

// Structure definition for segments, derived from param_1 access patterns
typedef struct Segment {
    struct Segment *next;
    char *data_ptr;
    int (*func_ptr)(void);
} Segment;


// Function: scramble (RC4-like algorithm)
// param_1: input key/data stream buffer
// param_2: output buffer (only 16 bytes are written to repeatedly)
// param_3: length of the input key/data stream to process
void scramble(byte *param_1, byte *param_2, uint param_3) {
    unsigned char S[256];
    unsigned int i;
    unsigned int j;
    unsigned char temp_byte;

    // Initialize S-box (KSA part 1: S[i] = i, then S[i] = 255-i as per Ghidra)
    for (i = 0; i < 256; ++i) {
        S[i] = (unsigned char)(255 - i); // Ghidra's cVar2 = -1; cVar2-- loop
    }

    // Key-scheduling algorithm (KSA part 2)
    j = 0;
    for (i = 0; i < 256; ++i) {
        j = (j + param_1[i % param_3] + S[i]) & 0xff;
        // Swap S[i] and S[j]
        temp_byte = S[i];
        S[i] = S[j];
        S[j] = temp_byte;
    }

    // Pseudo-random generation algorithm (PRGA) and XORing input data
    j = 0; // 'local_14' in Ghidra, used as j index
    unsigned int k = 0; // 'local_18' in Ghidra, used as i index
    for (i = 0; i < param_3; ++i) { // 'local_10' loop counter, up to param_3
        k = (k + 1) & 0xff;
        j = (j + S[k]) & 0xff;

        // Swap S[k] and S[j]
        temp_byte = S[k];
        S[k] = S[j];
        S[j] = temp_byte;

        // XOR keystream byte with input data byte and write to output buffer
        // Note: param_1 is used as input data, and consumed byte by byte.
        // param_2 is an output buffer, but only 16 bytes are affected.
        param_2[k & 0xf] ^= S[(S[j] + S[k]) & 0xff] ^ param_1[i];
    }
}

// Function: run_seg
undefined4 run_seg(Segment *param_1) {
    void *str_buffer1 = calloc(1, 0x10); // Buffer for short strings
    if (str_buffer1 == NULL) {
        sendline();
        _terminate();
    }

    void *str_buffer2 = calloc(1, 0x70); // Buffer for longer strings
    if (str_buffer2 == NULL) {
        free(str_buffer1); // Clean up previous allocation
        sendline();
        _terminate();
    }

    Segment *current_seg = param_1;
    while (current_seg != NULL) {
        strcpy(str_buffer1, current_seg->data_ptr + 4);
        strcpy(str_buffer2, current_seg->data_ptr + 0x14);

        strlen(str_buffer1); // Result unused, likely for a debug/logging call
        if (sendline() < 0) {
            _terminate();
        }

        strlen(str_buffer2); // Result unused
        if (sendline() < 0) {
            _terminate();
        }

        int result_code = current_seg->func_ptr();
        strlen(responses[result_code - 1]); // Result unused
        if (sendline() < 0) {
            _terminate();
        }
        current_seg = current_seg->next;
    }

    free(str_buffer1);
    free(str_buffer2);
    return 0;
}

// Function: load_seg
undefined4 load_seg(Segment *param_1) {
    Segment *current_seg = param_1;
    while (current_seg != NULL) {
        void *allocated_func_mem = NULL;
        // Allocate 0x1000 bytes for the function code
        if (allocate(&allocated_func_mem, 0x1000) != 0) {
            sendline(); // Error message "Clever girl."
            _terminate();
        }
        current_seg->func_ptr = (int (*)(void))allocated_func_mem;

        // Copy function code from data_ptr + 0x84 into the allocated memory
        memcpy(current_seg->func_ptr, current_seg->data_ptr + 0x84, 0x1000);
        current_seg = current_seg->next;
    }
    return 0;
}

// Function: sanitycheck
// param_1: points to the data_ptr member of a Segment struct
undefined4 sanitycheck(char *param_1) {
    size_t len1 = strlen(param_1 + 4);
    if (len1 != 0) {
        size_t len2 = strlen(param_1 + 0x14);
        if (len2 != 0) {
            return 0; // Both strings are non-empty
        }
    }
    return 1; // One or both strings are empty
}

// Function: validate_seg
// Iterates through segments, scrambles part of their data, and compares with trusted signatures.
// Also performs a sanity check on specific segments.
int validate_seg(Segment *param_1) {
    Segment *current_seg = param_1;
    int strncmp_result = 0; // Stores result of strncmp
    int segment_count = 0;  // Tracks how many segments have been processed

    while (1) { // Loop through segments until explicit break or return
        // This condition is hit on the third segment (segment_count == 2) and beyond.
        // Original: if (1 < local_18)
        if (segment_count > 1) {
            segment_count++; // Increment local_18 again
            return sanitycheck(current_seg->data_ptr); // Return sanitycheck result for this segment
        }
        segment_count++; // Increment local_18

        char scrambled_data[17];
        memset(scrambled_data, 0, sizeof(scrambled_data));

        // Scramble 0x1000 bytes from data_ptr + 0x84, outputting to scrambled_data (first 16 bytes)
        scramble((byte *)(current_seg->data_ptr + 0x84), (byte *)scrambled_data, 0x1000);

        int trusted_idx = 0;
        strncmp_result = 1; // Assume mismatch until a trusted signature is found
        while (trusted_idx < 6) { // Loop through trusted signatures
            if (strncmp(scrambled_data, trusted[trusted_idx], 0x10) == 0) {
                strncmp_result = 0; // Match found
                break;
            }
            trusted_idx++;
        }

        if (strncmp_result != 0) { // If no match found for current segment, stop and report failure
            break;
        }

        current_seg = current_seg->next;
        if (current_seg == NULL) { // If end of segment list is reached, all validated
            return 0;
        }
    }
    return strncmp_result; // Return the non-zero result from strncmp (failure code)
}

// Function: recv_seg
// Receives segments from a source (STDIN_FILENO = 0) and builds a linked list of Segment structs.
Segment *recv_seg(void) {
    uint num_segments = 0;
    ssize_t bytes_received = recv(STDIN_FILENO, &num_segments, sizeof(num_segments), 0);
    if (bytes_received < 0) {
        sendline();
        _terminate();
    }

    if (num_segments == 0 || num_segments > 0x1000) { // Max 4096 segments
        sendline(); // Error "Only the sigs deal in absolutes."
        _terminate();
    }

    Segment *head = NULL;
    Segment *tail = NULL;

    for (uint i = 0; i < num_segments; ++i) {
        char *data_buffer = calloc(1, 0x1084); // Allocate buffer for segment data
        if (data_buffer == NULL) {
            sendline(); // Error "Only the sigs deal in absolutes."
            _terminate();
        }

        Segment *new_segment = calloc(1, sizeof(Segment)); // Allocate Segment struct
        if (new_segment == NULL) {
            free(data_buffer); // Clean up data_buffer if segment struct allocation fails
            sendline(); // Error "Only the sigs deal in absolutes."
            _terminate();
        }

        new_segment->data_ptr = data_buffer;

        bytes_received = recv(STDIN_FILENO, new_segment->data_ptr, 0x1084, 0);
        if (bytes_received < 0) {
            free(data_buffer);
            free(new_segment);
            sendline(); // Error "You didn\'t say the magic word."
            _terminate();
        }

        // Check the value at the start of the data buffer.
        // It expects the first 4 bytes to be 0x1000 (4096).
        if (*(unsigned int *)new_segment->data_ptr != 0x1000) {
            free(data_buffer);
            free(new_segment);
            sendline(); // Error "Only the sigs deal in absolutes."
            _terminate();
        }

        // Null-terminate specific offsets within the data_ptr buffer, likely for embedded strings.
        new_segment->data_ptr[0x13] = 0;
        new_segment->data_ptr[0x83] = 0;

        // Add new segment to the linked list
        if (head == NULL) {
            head = new_segment;
        } else {
            tail->next = new_segment;
        }
        tail = new_segment;
    }

    if (head == NULL) { // If no segments were received despite num_segments > 0
        sendline(); // Error "Only the sigs deal in absolutes."
        _terminate();
    }
    return head;
}

// Function: main
undefined4 main(void) {
    unsigned int return_code = 0; // Default success code

    // Initial handshake/setup process
    // The specific calculation of local_24 and its use with sendall is unclear
    // in the decompiled snippet, so it's simplified to a direct call.
    if (sendall() < 0) {
        _terminate();
    }

    Segment *segment_list = recv_seg(); // Receive segments and build the list

    if (validate_seg(segment_list) == 0) { // Validate all segments
        if (load_seg(segment_list) == 0) {   // Load validated segments
            run_seg(segment_list);           // Execute segments
        } else {
            return_code = 2; // Load failed
        }
    } else {
        return_code = 1; // Validation failed
    }

    // Cleanup: Iterate through the linked list and free all allocated memory
    Segment *current_seg = segment_list;
    while (current_seg != NULL) {
        Segment *next_seg = current_seg->next; // Store next segment pointer before freeing current

        free(current_seg->data_ptr); // Free the data buffer
        
        // Deallocate the memory holding the function code
        if (deallocate(current_seg->func_ptr, 0x1000) != 0) { // Assuming 0x1000 was the allocation size
            sendline(); // Error "Clever girl."
            _terminate();
        }
        
        free(current_seg); // Free the Segment struct itself

        current_seg = next_seg;
    }

    sendline(); // Send final "OK" message
    return return_code;
}