#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> // For ssize_t
#include <stdint.h>    // For uint32_t
#include <math.h>      // For abs (or use stdlib.h for int abs)

// --- Global Constants and Structures ---

// Placeholder values for global constants.
// These would typically be derived from the original binary's data section.
double DAT_000140d8 = 0.5; // Example value, adjust as per original binary
double DAT_000140e0 = 2.0; // Example value, adjust as per original binary

// Language Node structure (e.g., for Morse code trie)
// Assuming a 32-bit architecture for pointer sizes (4 bytes) to match original offsets.
typedef struct LangNode LangNode;
struct LangNode {
    char character;
    char padding[3]; // Padding to align dot_next at offset 4
    LangNode *dot_next;  // At offset 4
    LangNode *dash_next; // At offset 8
};
LangNode *lh_root; // Global head of the language parsing tree

// Bucket structure for symbol analysis
typedef struct {
    short active;       // 0x00 (2 bytes) - 0 if inactive, 1 if active
    char  padding1[2];  // 0x02 (2 bytes) - padding for alignment
    uint32_t value;     // 0x04 (4 bytes) - represents the symbol duration
    uint32_t count;     // 0x08 (4 bytes) - number of times this symbol duration has occurred
    char char1;         // 0x0C (1 byte) - assigned character for 'dot'/'silence'
    char char2;         // 0x0D (1 byte) - assigned character for 'dash'/'sound'
    char padding2[2];   // 0x0E (2 bytes) - padding to make total 16 bytes
} BucketEntry;
BucketEntry Bucket[4]; // Indices 1, 2, 3 are used, so array size 4

// Global WAV buffer for input data
#define WAV_BUFFER_SIZE 0x80000 // Max 524288 bytes (0x7fffa max data size + 0xC header)
char g_wav_buffer[WAV_BUFFER_SIZE];

// --- Forward Declarations for Custom Functions ---
void _terminate(int status);
ssize_t receive(int fd, void *buf, size_t nbytes, int *bytes_read_ptr);
void InitLang(void);

// --- Custom Termination Function ---
void _terminate(int status) {
    exit(status);
}

// --- Custom Low-Level Receive Function (Stub) ---
// This function is assumed to be an external dependency.
// For compilation, a basic stub is provided. In a real system, this would interact
// with hardware or a specific file descriptor.
// Original `receive` returns 0 on success, non-zero on error, and sets `bytes_read_ptr`.
ssize_t receive(int fd, void *buf, size_t nbytes, int *bytes_read_ptr) {
    ssize_t bytes_read = read(fd, buf, nbytes); // Using standard `read` from unistd.h
    if (bytes_read_ptr) {
        *bytes_read_ptr = (int)bytes_read;
    }
    return (bytes_read > 0 || (bytes_read == 0 && nbytes == 0)) ? 0 : -1; // 0 on success, -1 on error
}

// --- Custom Receive Wrapper Function ---
// Renamed from `recv` to `my_recv` to avoid conflict with standard library `recv`.
ssize_t my_recv(int fd, void *buf, size_t n, int *bytes_read_total_ptr) {
    uint32_t total_bytes_read = 0;
    int bytes_read_single;

    while (total_bytes_read < n) {
        // The original logic calls `receive` for 1 byte at a time.
        int result = receive(fd, (char *)buf + total_bytes_read, 1, &bytes_read_single);

        if (result != 0) { // receive returns non-zero on error
            return result;
        }
        if (bytes_read_single != 1) { // Expected to read 1 byte, 0 means EOF or error
            return -1; // Indicate error or unexpected behavior
        }
        total_bytes_read++;
    }
    if (bytes_read_total_ptr) {
        *bytes_read_total_ptr = total_bytes_read;
    }
    return 0; // Success
}

// --- ParseCode Function ---
void ParseCode(const char *input_code, char *output_message) {
    int input_idx = 0;
    int output_idx = 0;
    LangNode *current_node = lh_root;

    while (1) {
        if (output_idx >= 0x3ff) { // Max output length 1023 (0x3ff)
            puts("message too long\n");
            _terminate(EXIT_FAILURE);
        }

        char current_char = input_code[input_idx];

        if (current_char == '\0') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            } else if (current_node != lh_root) { // If a symbol was being parsed
                output_message[output_idx++] = current_node->character;
            }
            output_message[output_idx] = '\0'; // Null-terminate
            return;
        }

        if (current_char == '|') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            }
            output_message[output_idx++] = current_node->character;
            current_node = lh_root; // Reset for next symbol
            output_message[output_idx++] = ' '; // Add space
        } else if (current_char == '.') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            }
            current_node = current_node->dot_next;
        } else if (current_char == '-') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            }
            current_node = current_node->dash_next;
        } else if (current_char == ' ') {
            if (current_node == lh_root) { // If at the root and space, just add a space
                output_message[output_idx++] = ' ';
            } else { // If not at root and space, complete current symbol, reset, and add space
                if (current_node == NULL) {
                    puts("unknown character\n");
                    _terminate(EXIT_FAILURE);
                }
                output_message[output_idx++] = current_node->character;
                current_node = lh_root;
                output_message[output_idx++] = ' ';
            }
        } else if (current_char == '\n') {
            if (current_node != lh_root && current_node != NULL) {
                output_message[output_idx++] = current_node->character;
            }
            output_message[output_idx++] = '\n';
            output_message[output_idx] = '\0';
            return;
        } else {
            puts("invalid symbol\n");
            _terminate(EXIT_FAILURE);
        }
        input_idx++;
    }
}

// --- ReadWav Function ---
int *ReadWav(int *wav_header_ptr) {
    int bytes_read_count;

    // Read initial 0xC bytes (custom header for this application)
    if (my_recv(0, wav_header_ptr, 0xc, &bytes_read_count) != 0) {
        puts("Read error\n");
        _terminate(EXIT_FAILURE);
    }

    // Check custom format magic (e.g., ' PCM')
    if (wav_header_ptr[0] != 0x204d4350) {
        puts("Invalid PCM format\n");
        _terminate(EXIT_FAILURE);
    }

    // Check data length (wav_header_ptr[2] is data_size)
    if ((uint32_t)wav_header_ptr[2] > 0x7fffa) {
        puts("Invalid PCM length\n");
        _terminate(EXIT_FAILURE);
    }
    if (wav_header_ptr[2] == 0) {
        puts("Invalid PCM format\n"); // Original message
        _terminate(EXIT_FAILURE);
    }

    // Check bits per sample (wav_header_ptr[1] is total_data_bytes)
    // The condition `((uint)(total_data_bytes * 8) / (uint)data_size != 0x10)`
    // means `(bytes_per_sample * 8) != 16`, so `bytes_per_sample != 2`.
    // This implies it expects non-16-bit samples.
    if (((uint32_t)wav_header_ptr[1] * 8) / (uint32_t)wav_header_ptr[2] != 0x10) {
        puts("Invalid PCM length\n");
        _terminate(EXIT_FAILURE);
    }

    // Read the actual WAV data following the custom header
    for (uint32_t current_data_offset = 0; current_data_offset < (uint32_t)wav_header_ptr[1]; current_data_offset += bytes_read_count) {
        int remaining_bytes = wav_header_ptr[1] - current_data_offset;
        if (my_recv(0, (char *)wav_header_ptr + 0xc + current_data_offset, remaining_bytes, &bytes_read_count) != 0) {
            puts("Read error\n");
            _terminate(EXIT_FAILURE);
        }
        if (bytes_read_count == 0) {
            puts("Read error\n");
            _terminate(EXIT_FAILURE);
        }
    }
    return wav_header_ptr;
}

// --- AssignToBucket Function ---
int AssignToBucket(uint32_t symbol_duration) {
    int bucket_idx = 1;

    // Try to assign to an existing bucket
    while (bucket_idx <= 3) {
        if (Bucket[bucket_idx].active != 0 &&
            DAT_000140d8 * (double)symbol_duration < (double)Bucket[bucket_idx].value &&
            (double)Bucket[bucket_idx].value < DAT_000140e0 * (double)symbol_duration) {
            Bucket[bucket_idx].count++;
            return bucket_idx;
        }
        bucket_idx++;
    }

    // If no existing bucket found, find an empty one
    bucket_idx = 1;
    while (bucket_idx <= 3) {
        if (Bucket[bucket_idx].active == 0) {
            Bucket[bucket_idx].active = 1;
            Bucket[bucket_idx].value = symbol_duration;
            Bucket[bucket_idx].count = 1;
            return bucket_idx;
        }
        bucket_idx++;
    }

    // No bucket found or available
    puts("Couldn\'t find a bucket\n");
    _terminate(EXIT_FAILURE);
    return -1; // Should not be reached
}

// --- CheckLimits Function ---
void CheckLimits(int primary_idx, int secondary_idx, int tertiary_idx) {
    uint32_t base_limit_val = Bucket[primary_idx].value * 3;

    // Check variance between primary and secondary buckets
    if (((double)Bucket[secondary_idx].value < DAT_000140d8 * (double)base_limit_val) ||
        (DAT_000140e0 * (double)base_limit_val < (double)Bucket[secondary_idx].value)) {
        puts("Too much variance in symbol times\n");
        _terminate(EXIT_FAILURE);
    }

    // Check variance between primary and tertiary buckets, if tertiary_idx is valid (not 0)
    if (tertiary_idx != 0) {
        base_limit_val = Bucket[primary_idx].value * 7; // Recalculate for tertiary check
        if (((double)Bucket[tertiary_idx].value < DAT_000140d8 * (double)base_limit_val) ||
            (DAT_000140e0 * (double)base_limit_val < (double)Bucket[tertiary_idx].value)) {
            puts("Too much variance in symbol times\n");
            _terminate(EXIT_FAILURE);
        }
    }
}

// --- ValidateBuckets Function ---
void ValidateBuckets(void) {
    int primary_bucket = 0;
    int secondary_bucket = 0;
    int tertiary_bucket = 0;

    // Check if all three buckets (1, 2, 3) are active
    if (Bucket[1].active != 0 && Bucket[2].active != 0 && Bucket[3].active != 0) {
        uint32_t val1 = Bucket[1].value;
        uint32_t val2 = Bucket[2].value;
        uint32_t val3 = Bucket[3].value;

        // Determine the order of buckets based on their 'value'
        if (val1 < val2 && val1 < val3) {
            primary_bucket = 1;
            if (val2 < val3) { secondary_bucket = 2; tertiary_bucket = 3; }
            else { secondary_bucket = 3; tertiary_bucket = 2; }
        } else if (val2 < val1 && val2 < val3) {
            primary_bucket = 2;
            if (val1 < val3) { secondary_bucket = 1; tertiary_bucket = 3; }
            else { secondary_bucket = 3; tertiary_bucket = 1; }
        } else { // val3 is smallest
            primary_bucket = 3;
            if (val1 < val2) { secondary_bucket = 1; tertiary_bucket = 2; }
            else { secondary_bucket = 2; tertiary_bucket = 1; }
        }
    } else {
        // If not all three are active, check if at least two buckets (1 and 2) are active
        if (Bucket[1].active == 0 || Bucket[2].active == 0) {
            puts("Insufficient symbol diversity\n");
            _terminate(EXIT_FAILURE);
        }
        // If Bucket 1 and 2 are active, but 3 is not, or other combinations that lead here
        // The original logic here specifically identifies Bucket 1 and 2 if 3 is inactive.
        uint32_t val1 = Bucket[1].value;
        uint32_t val2 = Bucket[2].value;

        if (val1 < val2) {
            primary_bucket = 1;
            secondary_bucket = 2;
            tertiary_bucket = 0; // Indicate tertiary bucket is not active/used
        } else {
            primary_bucket = 2;
            secondary_bucket = 1;
            tertiary_bucket = 0;
        }
    }

    // Assign characters based on bucket order
    Bucket[primary_bucket].char1 = ' ';
    Bucket[primary_bucket].char2 = ' ';

    Bucket[secondary_bucket].char1 = '-';
    Bucket[secondary_bucket].char2 = '|';

    if (tertiary_bucket != 0) { // Only assign if a tertiary bucket was identified
        Bucket[tertiary_bucket].char1 = '.';
        Bucket[tertiary_bucket].char2 = '\0';
    } else { // If only two buckets, the third symbol might be assigned to a non-existent bucket or ignored.
             // Based on ParseWav, it looks for positive/negative bucket_id, so a char for dot would be needed.
             // If tertiary_bucket is 0, this logic implies no "dot" symbol is assigned.
             // This might be a semantic point where the number of distinct symbols depends on active buckets.
             // For robustness, let's ensure dot symbol is always assignable if only 2 buckets.
             // The original code implies that if `tertiary_bucket` is 0, the corresponding char assignments are skipped.
    }
    
    CheckLimits(primary_bucket, secondary_bucket, tertiary_bucket);
}

// --- ParseWav Function ---
void ParseWav(int *wav_data_ptr) {
    char morse_encoded_buffer[1024]; // Stores bucket IDs (positive for silence, negative for sound)
    char intermediate_buffer[1024];  // Stores Morse code symbols ('.', '-', ' ', '|')
    char parsed_output_buffer[1024]; // Stores decoded text

    memset(morse_encoded_buffer, 0, sizeof(morse_encoded_buffer));
    memset(intermediate_buffer, 0, sizeof(intermediate_buffer));
    memset(parsed_output_buffer, 0, sizeof(parsed_output_buffer));

    // Reset Bucket active states
    Bucket[1].active = 0;
    Bucket[2].active = 0;
    Bucket[3].active = 0;

    int morse_buffer_idx = 0;
    int is_silence_state = 1; // 1: silence (abs(sample) <= 9), 0: sound (abs(sample) > 9)
    int is_initial_scan = 1;  // 1: initial scan phase, 0: regular processing phase
    int consecutive_samples = 0;
    int initial_wav_offset = 0xc; // Byte offset from start of wav_data_ptr to current segment start

    short *end_of_samples_ptr = (short *)((char *)wav_data_ptr + wav_data_ptr[1] + 0xc);
    short *current_sample_ptr = (short *)((char *)wav_data_ptr + 0xc);

    while (1) {
        // Exit condition for the entire ParseWav function
        if (current_sample_ptr >= end_of_samples_ptr || morse_buffer_idx >= 0x3fd) {
            // Process the final segment if any
            int current_wav_offset = (char *)current_sample_ptr - (char *)wav_data_ptr;
            uint32_t final_symbol_duration = (uint32_t)((current_wav_offset - initial_wav_offset) / 2);

            if (final_symbol_duration > 0) { // Only process if duration is positive
                char bucket_char = AssignToBucket(final_symbol_duration);
                if (is_silence_state == 0) { // Last state was sound
                    morse_encoded_buffer[morse_buffer_idx++] = -bucket_char;
                } else { // Last state was silence
                    morse_encoded_buffer[morse_buffer_idx++] = bucket_char;
                }
            }

            ValidateBuckets();

            // Translate morse_encoded_buffer to intermediate_buffer
            int intermediate_idx = 0;
            for (int i = 0; i < morse_buffer_idx; i++) {
                char bucket_id = morse_encoded_buffer[i];
                if (bucket_id == '\0') continue;

                int actual_bucket_idx = abs((int)bucket_id);
                if (actual_bucket_idx > 0 && actual_bucket_idx <= 3) {
                    if (bucket_id < 0) { // Negative ID implies 'sound' (dash/separator)
                        if (Bucket[actual_bucket_idx].char2 != '\0') {
                            intermediate_buffer[intermediate_idx++] = Bucket[actual_bucket_idx].char2;
                        }
                    } else { // Positive ID implies 'silence' (dot/space)
                        if (Bucket[actual_bucket_idx].char1 != '\0') {
                            intermediate_buffer[intermediate_idx++] = Bucket[actual_bucket_idx].char1;
                        }
                    }
                }
            }
            intermediate_buffer[intermediate_idx++] = '\n';
            intermediate_buffer[intermediate_idx] = '\0';

            ParseCode(intermediate_buffer, parsed_output_buffer);
            puts(parsed_output_buffer);
            puts("\n");
            return; // Exit ParseWav
        }

        short sample_value = *current_sample_ptr;
        int current_sample_is_sound = (sample_value < -9 || sample_value > 9);
        int current_sample_is_silence = !current_sample_is_sound;

        // Condition for breaking from the inner logic loop in original code
        int break_inner_loop = current_sample_is_sound || (is_silence_state == 1 && is_initial_scan == 0);

        if (break_inner_loop) {
            // This corresponds to the code block after `break` in the original inner `while` loop
            // Condition for the `if` block after the inner loop break:
            // `(-0xb < sample_value && sample_value < 0xb)` (sample_value is within a slightly wider silence range)
            // OR `(is_silence_state == 0 && is_initial_scan == 0)` (current state is sound AND not initial scan)
            if ((sample_value >= -0xb && sample_value <= 0xb) || (is_silence_state == 0 && is_initial_scan == 0)) {
                consecutive_samples = 0;
            } else {
                consecutive_samples++;
                if (consecutive_samples >= 0x32) { // 50 samples threshold
                    if (is_initial_scan == 0) {
                        is_silence_state = 0; // Transition to sound state
                        consecutive_samples = 0;
                        int current_wav_offset = (char *)current_sample_ptr - (char *)wav_data_ptr;
                        uint32_t symbol_duration = (uint32_t)((current_wav_offset - initial_wav_offset - 100U) / 2);
                        initial_wav_offset = current_wav_offset - 100;

                        char bucket_char = AssignToBucket(symbol_duration);
                        morse_encoded_buffer[morse_buffer_idx++] = -bucket_char; // Negative for sound
                    } else { // is_initial_scan == 1: Reset for calibration
                        is_silence_state = 0; // Set state to sound
                        consecutive_samples = 0;
                        is_initial_scan = 0;
                        current_sample_ptr = (short *)((char *)wav_data_ptr + 0xc) -1; // Will be +0xc after loop increment
                        initial_wav_offset = 0xc;
                    }
                }
            }
        } else { // This corresponds to the main body of the original inner `while` loop
            consecutive_samples++;
            if (consecutive_samples >= 0x32) { // 50 samples threshold
                if (is_initial_scan == 0) {
                    is_silence_state = 1; // Transition to silence state
                    consecutive_samples = 0;
                    int current_wav_offset = (char *)current_sample_ptr - (char *)wav_data_ptr;
                    uint32_t symbol_duration = (uint32_t)((current_wav_offset - initial_wav_offset - 100U) / 2);
                    initial_wav_offset = current_wav_offset - 100;

                    char bucket_char = AssignToBucket(symbol_duration);
                    morse_encoded_buffer[morse_buffer_idx++] = bucket_char; // Positive for silence
                } else { // is_initial_scan == 1: Reset for calibration
                    is_silence_state = 1; // Set state to silence
                    consecutive_samples = 0;
                    is_initial_scan = 0;
                    current_sample_ptr = (short *)((char *)wav_data_ptr + 0xc) -1; // Will be +0xc after loop increment
                    initial_wav_offset = 0xc;
                }
            }
        }
        current_sample_ptr++; // Move to the next sample
    }
}

// --- InitLang Function ---
// Placeholder for language structure initialization (e.g., Morse code trie)
void InitLang(void) {
    // Allocate root node
    lh_root = (LangNode *)malloc(sizeof(LangNode));
    if (lh_root == NULL) {
        perror("Failed to allocate LangNode");
        exit(EXIT_FAILURE);
    }
    lh_root->character = '\0'; // Root doesn't represent a character
    lh_root->dot_next = NULL;
    lh_root->dash_next = NULL;

    // Example: Node for 'A' (. -)
    LangNode *node_dot = (LangNode *)malloc(sizeof(LangNode));
    if (node_dot == NULL) { perror("malloc failed"); exit(EXIT_FAILURE); }
    node_dot->character = '\0';
    node_dot->dot_next = NULL;
    node_dot->dash_next = NULL;
    lh_root->dot_next = node_dot;

    LangNode *node_dash_from_dot = (LangNode *)malloc(sizeof(LangNode));
    if (node_dash_from_dot == NULL) { perror("malloc failed"); exit(EXIT_FAILURE); }
    node_dash_from_dot->character = 'A'; // 'A' is ._
    node_dash_from_dot->dot_next = NULL;
    node_dash_from_dot->dash_next = NULL;
    node_dot->dash_next = node_dash_from_dot;

    // Example: Node for 'T' (-)
    LangNode *node_dash = (LangNode *)malloc(sizeof(LangNode));
    if (node_dash == NULL) { perror("malloc failed"); exit(EXIT_FAILURE); }
    node_dash->character = 'T'; // 'T' is -
    node_dash->dot_next = NULL;
    node_dash->dash_next = NULL;
    lh_root->dash_next = node_dash;

    // In a full implementation, a complete Morse code trie would be built here.
    // For this exercise, this minimal setup allows ParseCode to demonstrate traversal.
}

// --- Main Function ---
int main(void) {
    // The original main's stack setup and `bzero` loops are Ghidra artifacts
    // and are removed as they are not standard C and likely environment-specific.

    InitLang(); // Initialize the language parsing structure

    // Read WAV data into the global buffer.
    // The original code implicitly passes a stack address, which we map to g_wav_buffer.
    ReadWav((int *)g_wav_buffer);

    // Parse the WAV data from the global buffer.
    ParseWav((int *)g_wav_buffer);

    // Free the allocated LangNodes to prevent memory leaks.
    // A proper cleanup function would traverse the trie. For this example,
    // we just free the few nodes we explicitly allocated.
    if (lh_root) {
        if (lh_root->dot_next) {
            if (lh_root->dot_next->dash_next) free(lh_root->dot_next->dash_next);
            free(lh_root->dot_next);
        }
        if (lh_root->dash_next) free(lh_root->dash_next);
        free(lh_root);
    }

    return 0;
}