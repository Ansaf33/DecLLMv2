#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h> // For abs (though stdlib.h provides abs for int/long)

// --- Global variables and types (reconstructed from snippet) ---

// Assuming 32-bit pointers based on offsets 4 and 8 from `LangNode *`
typedef struct LangNode {
    char value;
    char _pad1[3]; // Padding to align next to 4 bytes
    struct LangNode *next; // Offset 4
    struct LangNode *child; // Offset 8
} LangNode; // Total size 12 bytes (1 + 3 + 4 + 4)

LangNode lh; // Global root for language tree

// Bucket structure based on access patterns (0x10 bytes per entry)
typedef struct {
    short field0;    // Offset 0 (e.g., used/unused flag)
    char _pad2[2];
    uint32_t field4; // Offset 4 (e.g., value/timestamp)
    int32_t field8;  // Offset 8 (e.g., count)
    char fieldC;     // Offset 12 (character for bucket)
    char fieldD;     // Offset 13 (alternate character for bucket)
    char _padE[2];
} BucketEntry; // Total size 16 bytes (0x10)

BucketEntry Bucket[4]; // Assuming indices 1, 2, 3 are used. Index 0 might be unused or special.

// Global double constants (placeholder values, adjust as needed)
double DAT_000140d8 = 0.8;
double DAT_000140e0 = 1.2;

// Global buffer for WAV data (1MB as inferred from main)
char global_wav_buffer[0x100000]; // Static storage duration variables are zero-initialized by default

// --- Stubs for missing functions ---

// Placeholder for _terminate
void _terminate(int status) {
    exit(status);
}

// Custom receive function, assuming it wraps read and returns 0 on success, non-zero on error
// and fills bytes_read_ptr with actual bytes read.
// The original snippet implies `receive` returns 0 for success (1 byte read), non-zero for error.
int mock_receive(int fd, void *buf, size_t count, int *bytes_read_ptr) {
    ssize_t ret = read(fd, buf, count);
    if (ret < 0) {
        *bytes_read_ptr = 0;
        return -1; // Indicate error
    }
    *bytes_read_ptr = (int)ret;
    return 0; // Indicate success (0 means no error)
}

// Renamed from original 'recv' to avoid conflict with standard library
ssize_t custom_recv(int fd, void *buf, size_t n, int *out_total_bytes_read) {
    size_t total_received = 0;
    while (total_received < n) {
        int bytes_this_read = 0;
        int receive_status = mock_receive(fd, (char *)buf + total_received, 1, &bytes_this_read);
        
        if (receive_status != 0) { // If mock_receive returned an error status
            return -1; // Propagate error
        }
        
        if (bytes_this_read != 1) { // If `mock_receive` succeeded but didn't read 1 byte (e.g., EOF)
            return -1; // Original code returns -1, implying it must read exactly 1 byte.
        }
        total_received++;
    }
    *out_total_bytes_read = (int)total_received;
    return (ssize_t)total_received; // Return total bytes read (n)
}

// Placeholder for InitLang - initializes the global language tree 'lh'
void InitLang(void) {
    // Example initialization for 'lh'. Actual logic might be more complex.
    lh.value = ' ';
    lh.next = NULL;
    lh.child = NULL;
}

// Helper to initialize Bucket array
void InitBuckets(void) {
    memset(Bucket, 0, sizeof(Bucket));
}

// --- Original functions, fixed ---

// Function: ParseCode
void ParseCode(const char *input_code, char *output_message) {
    LangNode *current_node = &lh;
    int input_idx = 0;
    int output_idx = 0;

    while (1) {
        if (output_idx >= 1023) { // 0x3ff is 1023, so >= 1023 means 1024th char is too long
            puts("message too long\n");
            _terminate(EXIT_FAILURE);
        }

        char current_char = input_code[input_idx];

        if (current_char == '\0') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            }
            output_message[output_idx++] = current_node->value;
            output_message[output_idx] = '\0'; // Null-terminate the output message
            return;
        }

        // Handle characters and update current_node / output_message
        if (current_char == '|') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            }
            output_message[output_idx++] = current_node->value;
            current_node = &lh; // Reset to root
        } else if (current_char == '.') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            }
            current_node = current_node->next; // Move to next sibling
        } else if (current_char == '-') {
            if (current_node == NULL) {
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            }
            current_node = current_node->child; // Move to child
        } else if (current_char == ' ') {
            if (current_node == &lh) { // If at root
                output_message[output_idx++] = ' ';
            } else if (current_node == NULL) { // If null
                puts("unknown character\n");
                _terminate(EXIT_FAILURE);
            } else { // If not root, not null
                output_message[output_idx++] = current_node->value;
                current_node = &lh; // Reset to root
                output_message[output_idx++] = ' '; // Add space after symbol
            }
        } else if (current_char == '\n') {
            // Consume newline, do nothing else
        } else {
            puts("invalid symbol\n");
            _terminate(EXIT_FAILURE);
        }
        input_idx++;
    }
}

// Function: ReadWav
// wav_buffer points to the start of the buffer for the WAV data.
// Assuming the first 12 bytes contain format (uint32_t), data_size (uint32_t), total_length (uint32_t).
int *ReadWav(int *wav_buffer) {
    int bytes_read_total = 0; 
    ssize_t sVar1 = custom_recv(0, wav_buffer, 0xc, &bytes_read_total); // Read first 12 bytes (header)

    if (sVar1 != 0xc) { // custom_recv returns total bytes read (expected 0xc), or -1 on error.
        puts("Read error\n");
        _terminate(EXIT_FAILURE);
    }

    if (((uint32_t*)wav_buffer)[0] != 0x204d4350) { // Check format (PCM )
        puts("Invalid PCM format\n");
        _terminate(EXIT_FAILURE);
    }

    if (((uint32_t*)wav_buffer)[2] > 0x7fffa) { // Check total length limit
        puts("Invalid PCM length\n");
        _terminate(EXIT_FAILURE);
    }

    if (((uint32_t*)wav_buffer)[2] == 0) { // Check total length not zero
        puts("Invalid PCM format\n"); // Original message is "Invalid PCM format"
        _terminate(EXIT_FAILURE);
    }
    
    // This calculation is (data_size * 8) / total_length == 16
    // Which means data_size (bytes) / total_length (samples) == 2 (bytes/sample for 16-bit PCM)
    if ((((uint32_t*)wav_buffer)[1] * 8) / ((uint32_t*)wav_buffer)[2] != 0x10) {
        puts("Invalid PCM length\n");
        _terminate(EXIT_FAILURE);
    }

    // Loop to read remaining data (wav_buffer[1] is data_size in bytes)
    uint32_t data_size = ((uint32_t*)wav_buffer)[1];
    for (int i = 0; i < data_size; ) { 
        int remaining_bytes = data_size - i;
        int bytes_read_this_iter = 0;

        int status = mock_receive(0, (char *)wav_buffer + 0xc + i, remaining_bytes, &bytes_read_this_iter);

        if (status != 0) { // If mock_receive returned an error status
            puts("Read error\n");
            _terminate(EXIT_FAILURE);
        }
        if (bytes_read_this_iter == 0) { // If mock_receive reported 0 bytes read (EOF)
            puts("Read error\n");
            _terminate(EXIT_FAILURE);
        }
        i += bytes_read_this_iter; // Increment loop counter by actual bytes read
    }
    return wav_buffer;
}

// Function: AssignToBucket
int AssignToBucket(uint32_t param_1) {
    int bucket_idx = 1;

    // Try to assign to an existing bucket
    while (bucket_idx <= 3) {
        if (Bucket[bucket_idx].field0 != 0 &&
            DAT_000140d8 * (double)param_1 < (double)Bucket[bucket_idx].field4 &&
            (double)Bucket[bucket_idx].field4 < DAT_000140e0 * (double)param_1) {
            
            Bucket[bucket_idx].field8++; // Increment count
            return bucket_idx;
        }
        bucket_idx++;
    }

    // If no existing bucket found, find an empty one
    bucket_idx = 1;
    while (bucket_idx <= 3) {
        if (Bucket[bucket_idx].field0 == 0) { // Check if bucket is empty
            Bucket[bucket_idx].field0 = 1; // Mark as used
            Bucket[bucket_idx].field4 = param_1; // Store value
            Bucket[bucket_idx].field8 = 1; // Initialize count
            return bucket_idx;
        }
        bucket_idx++;
    }

    // If all buckets are full
    puts("Couldn\'t find a bucket\n");
    _terminate(6);
    return -1; // Should not be reached
}

// Function: CheckLimits
void CheckLimits(int bucket_idx1, int bucket_idx2, int bucket_idx3) {
    // Check variance between bucket 1 and 2 (bucket_idx1 and bucket_idx2)
    uint32_t val1_times_3 = Bucket[bucket_idx1].field4 * 3;
    if ((double)Bucket[bucket_idx2].field4 < DAT_000140d8 * (double)val1_times_3 ||
        DAT_000140e0 * (double)val1_times_3 < (double)Bucket[bucket_idx2].field4) {
        puts("Too much variance in symbol times\n");
        _terminate(EXIT_FAILURE);
    }

    // Check variance between bucket 1 and 3 (bucket_idx1 and bucket_idx3), if bucket_idx3 is valid
    if (bucket_idx3 != 0) { // bucket_idx3 == 0 means it's not used, or invalid bucket index
        uint32_t val1_times_7 = Bucket[bucket_idx1].field4 * 7;
        if ((double)Bucket[bucket_idx3].field4 < DAT_000140d8 * (double)val1_times_7 ||
            DAT_000140e0 * (double)val1_times_7 < (double)Bucket[bucket_idx3].field4) {
            puts("Too much variance in symbol times\n");
            _terminate(EXIT_FAILURE);
        }
    }
}

// Function: ValidateBuckets
void ValidateBuckets(void) {
    int smallest_idx = 0, middle_idx = 0, largest_idx = 0;

    int used_buckets[3];
    int num_used = 0;
    for (int i = 1; i <= 3; ++i) {
        if (Bucket[i].field0 != 0) { // If bucket is marked as used
            used_buckets[num_used++] = i;
        }
    }

    if (num_used < 2) { // Need at least two buckets for diversity
        puts("Insufficient symbol diversity\n");
        _terminate(EXIT_FAILURE);
    }

    if (num_used == 2) {
        // Sort the two used buckets
        if (Bucket[used_buckets[0]].field4 < Bucket[used_buckets[1]].field4) {
            smallest_idx = used_buckets[0];
            middle_idx = used_buckets[1];
        } else {
            smallest_idx = used_buckets[1];
            middle_idx = used_buckets[0];
        }
        largest_idx = 0; // No third bucket, indicates not used in CheckLimits
    } else { // num_used == 3
        int b1 = used_buckets[0];
        int b2 = used_buckets[1];
        int b3 = used_buckets[2];

        uint32_t v1 = Bucket[b1].field4;
        uint32_t v2 = Bucket[b2].field4;
        uint32_t v3 = Bucket[b3].field4;

        // Sort three buckets by their field4 values
        if (v1 <= v2 && v1 <= v3) {
            smallest_idx = b1;
            if (v2 <= v3) { middle_idx = b2; largest_idx = b3; }
            else { middle_idx = b3; largest_idx = b2; }
        } else if (v2 <= v1 && v2 <= v3) {
            smallest_idx = b2;
            if (v1 <= v3) { middle_idx = b1; largest_idx = b3; }
            else { middle_idx = b3; largest_idx = b1; }
        } else {
            smallest_idx = b3;
            if (v1 <= v2) { middle_idx = b1; largest_idx = b2; }
            else { middle_idx = b2; largest_idx = b1; }
        }
    }

    // Assign characters to buckets based on sorted order
    Bucket[smallest_idx].fieldC = ' ';
    Bucket[smallest_idx].fieldD = ' ';
    Bucket[middle_idx].fieldC = '-';
    Bucket[middle_idx].fieldD = '|';
    Bucket[largest_idx].fieldC = '.';
    Bucket[largest_idx].fieldD = '\0';

    CheckLimits(smallest_idx, middle_idx, largest_idx);
}

// Function: ParseWav
void ParseWav(int *wav_data_ptr) {
    char bucket_ids_buf[1024];     // Stores assigned bucket IDs (local_440)
    char parsed_symbols_buf[1024];  // Stores symbols translated from bucket IDs (local_840)
    char output_message_buf[1024];  // Output of ParseCode (local_c40)

    memset(bucket_ids_buf, 0, sizeof(bucket_ids_buf));
    memset(parsed_symbols_buf, 0, sizeof(parsed_symbols_buf));
    InitBuckets(); // Ensure Bucket array is cleared/initialized

    int bucket_ids_idx = 0; // Current index for bucket_ids_buf (local_20)
    short *current_sample_ptr = (short *)((char *)wav_data_ptr + 0xc); // Pointer to current sample (local_10)
    // Pointer to end of sample data (wav_data_ptr[1] is data_size in bytes)
    short *end_sample_ptr = (short *)((char *)wav_data_ptr + ((uint32_t*)wav_data_ptr)[1] + 0xc); 

    int prev_offset_bytes = 0xc; // Byte offset of the start of the current segment (local_14)
    int in_quiet_state = 1;      // State flag (local_18): 1=looking for active segment, 0=looking for quiet segment
    int consecutive_count = 0;   // Count of consecutive samples matching current state (local_1c)
    int is_initial_phase = 1;    // Phase flag (local_28): 1=initial calibration, 0=normal processing

    while (current_sample_ptr < end_sample_ptr) {
        if (bucket_ids_idx >= 1023) { // 0x3ff is 1023. If 1023 elements collected, break.
            break; // Process collected symbols if buffer full
        }

        int current_offset_bytes = (int)((char *)current_sample_ptr - (char *)wav_data_ptr);
        short current_sample_value = *current_sample_ptr;
        int is_active_sample = (abs(current_sample_value) > 9); // True if sample is "active" (loud)

        int restart_loop_due_to_reset = 0;

        if (in_quiet_state == 1) { // Currently in a quiet segment, looking for active (symbol start)
            if (is_active_sample) { // Found an active sample
                consecutive_count++;
                if (consecutive_count >= 50) { // 50 consecutive active samples
                    uint32_t duration_bytes = current_offset_bytes - prev_offset_bytes - 100;
                    bucket_ids_buf[bucket_ids_idx++] = (char)(-AssignToBucket(duration_bytes / 2)); // Negative for active segment
                    prev_offset_bytes = current_offset_bytes - 100;
                    in_quiet_state = 0; // Transition to active state (now looking for quiet)
                    consecutive_count = 0;
                }
            } else { // Found a quiet sample
                consecutive_count = 0; // Reset count if quiet sample breaks active streak
            }
        } else { // in_quiet_state == 0: Currently in an active segment, looking for quiet (symbol end)
            if (!is_active_sample) { // Found a quiet sample
                consecutive_count++;
                if (consecutive_count >= 50) { // 50 consecutive quiet samples
                    uint32_t duration_bytes = current_offset_bytes - prev_offset_bytes - 100;
                    if (is_initial_phase == 0) { // Normal processing phase
                        bucket_ids_buf[bucket_ids_idx++] = (char)AssignToBucket(duration_bytes / 2); // Positive for quiet segment
                        prev_offset_bytes = current_offset_bytes - 100;
                    } else { // is_initial_phase == 1: Initial calibration phase
                        is_initial_phase = 0; // End initial calibration phase
                        current_sample_ptr = (short *)((char *)wav_data_ptr + 0xc); // Reset sample pointer to start of data
                        prev_offset_bytes = 0xc; // Reset previous offset
                        consecutive_count = 0; // Reset consecutive count
                        in_quiet_state = 1; // Transition to quiet state (now looking for active)
                        restart_loop_due_to_reset = 1; // Flag to restart loop iteration
                    }
                    if (!restart_loop_due_to_reset) { // Only transition if not restarting the loop
                        in_quiet_state = 1; // Transition to quiet state (now looking for active)
                        consecutive_count = 0;
                    }
                }
            } else { // Found an active sample
                consecutive_count = 0; // Reset count if active sample breaks quiet streak
            }
        }

        if (restart_loop_due_to_reset) {
            continue; // Restart the main loop from beginning with reset pointer
        }
        current_sample_ptr++; // Move to next sample
    }

    // After iterating through all samples or filling bucket_ids_buf:
    // Calculate duration of final segment if any remaining data needs to be processed.
    if (bucket_ids_idx < 1023) { // If output buffer not full
        int final_offset_bytes = (int)((char *)end_sample_ptr - (char *)wav_data_ptr);
        uint32_t final_duration_bytes = (uint32_t)((final_offset_bytes - prev_offset_bytes) / 2);
        
        // Only assign if it's a meaningful duration and not the initial calibration phase that just ended.
        if (final_duration_bytes > 0 && is_initial_phase == 0) {
            if (in_quiet_state == 0) { // If the last segment was active
                bucket_ids_buf[bucket_ids_idx++] = (char)(-AssignToBucket(final_duration_bytes));
            } else { // If the last segment was quiet
                bucket_ids_buf[bucket_ids_idx++] = (char)AssignToBucket(final_duration_bytes);
            }
        }
    }
    
    ValidateBuckets(); // Final validation of buckets

    // Translate bucket IDs to symbols
    int parsed_symbols_idx = 0;
    for (int i = 0; i < bucket_ids_idx; i++) {
        if (parsed_symbols_idx >= 1023) break; // Ensure output buffer doesn't overflow

        int bucket_id = bucket_ids_buf[i];
        if (bucket_id == 0) continue; // Skip unused/invalid bucket IDs

        if (bucket_id < 0) { // Negative bucket ID (active segment)
            bucket_id = -bucket_id;
            if (Bucket[bucket_id].fieldD != '\0') { // Use fieldD for negative
                parsed_symbols_buf[parsed_symbols_idx++] = Bucket[bucket_id].fieldD;
            }
        } else { // Positive bucket ID (quiet segment)
            if (Bucket[bucket_id].fieldC != '\0') { // Use fieldC for positive
                parsed_symbols_buf[parsed_symbols_idx++] = Bucket[bucket_id].fieldC;
            }
        }
    }
    parsed_symbols_buf[parsed_symbols_idx++] = '\n'; // Add newline
    parsed_symbols_buf[parsed_symbols_idx] = '\0'; // Null-terminate

    ParseCode(parsed_symbols_buf, output_message_buf);
    puts(output_message_buf);
    puts("\n");
}

// Function: main
int main(void) {
    // Global variables are zero-initialized by default for static storage duration.
    // The original stack manipulation and bzero for 0x100000 bytes at a fixed
    // stack address is replaced by using the global_wav_buffer.

    InitLang(); // Initialize language tree (lh)
    ReadWav((int *)global_wav_buffer); // Read WAV data into the buffer
    ParseWav((int *)global_wav_buffer); // Parse WAV data

    return 0;
}