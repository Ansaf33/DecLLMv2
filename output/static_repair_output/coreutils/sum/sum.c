#include <stdio.h>    // FILE, fread_unlocked, ferror_unlocked, feof_unlocked, printf, putchar_unlocked, fwrite_unlocked, stdout, snprintf
#include <stdlib.h>   // malloc, free, abort
#include <stdint.h>   // uint, size_t, int, long, unsigned char, unsigned short
#include <errno.h>    // __errno_location, EOVERFLOW
#include <byteswap.h> // __bswap_16
#include <limits.h>   // LONG_MAX (for total_bytes_read overflow check)

// Define custom types from the original snippet to standard C types
// The original snippet used 'undefined4' and 'undefined8' which are likely
// specific to the disassembler output. We map them to standard types.
// 'undefined4' typically means a 4-byte value, 'undefined8' an 8-byte value.
// 'byte' means an unsigned char.
typedef unsigned int undefined4;
typedef unsigned long long undefined8; // Used for pointers/sizes on 64-bit systems
typedef unsigned char byte;
typedef unsigned long ulong; // ulong is often unsigned long

// Global buffer lengths (assuming they are global based on the original snippet's access)
static size_t buffer_length_0 = 4096; // Example size
static size_t buffer_length_1 = 4096; // Example size

// Stack protector related.
// On Linux, __stack_chk_guard and __stack_chk_fail are typically provided by glibc.
// Declaring them as extern allows the linker to find them.
extern unsigned long __stack_chk_guard;
extern void __stack_chk_fail(void);

// Minimal stub for human_readable.
// In a real implementation, this would format a size value (size_val) into buf.
// The last parameter (buf_size) likely indicates the buffer's capacity.
static undefined8 human_readable(undefined8 size_val, char *buf, int param_2, int param_3, int buf_size) {
    snprintf(buf, buf_size, "%llu", size_val); // Format the size into the buffer
    return (undefined8)buf; // Return pointer to the buffer (as undefined8)
}

// Function: bsd_sum_stream
undefined4 bsd_sum_stream(FILE *param_1, unsigned int *param_2, long *param_3) {
    long stack_chk_guard_val = __stack_chk_guard; // Store stack canary
    int ret_val = -1; // Initialize to error (failure)
    unsigned int current_sum = 0;
    long total_bytes_read = 0;

    char *buffer = (char *)malloc(buffer_length_1);
    if (buffer == NULL) {
        return -1; // Memory allocation failed
    }

    int file_ended = 0; // Flag to indicate if EOF or an error occurred during read

    // Main loop to read and process chunks
    while (!file_ended) {
        size_t bytes_read_in_chunk = 0;
        // Inner loop to fill the buffer completely or until EOF/error
        while (bytes_read_in_chunk < buffer_length_1) {
            size_t s_read = fread_unlocked(buffer + bytes_read_in_chunk, 1, buffer_length_1 - bytes_read_in_chunk, param_1);
            bytes_read_in_chunk += s_read;

            if (s_read == 0) { // EOF or error occurred during read
                if (ferror_unlocked(param_1)) {
                    ret_val = -1; // Set error code
                }
                file_ended = 1; // Mark file as ended (either EOF or error)
                break; // Break from inner loop to process any data that was read
            }
        }

        // Process the data read in this chunk (even if partial due to EOF)
        if (bytes_read_in_chunk > 0) {
            for (size_t i = 0; i < bytes_read_in_chunk; ++i) {
                // BSD sum algorithm: 16-bit right rotate + byte, then mask to 16 bits
                current_sum = (current_sum & 1) * 0x8000 + (current_sum >> 1) + (unsigned char)buffer[i];
                current_sum &= 0xffff; // Ensure sum stays within 16-bit range
            }

            // Check for total_bytes_read overflow
            // LONG_MAX is from <limits.h>
            if (total_bytes_read > LONG_MAX - bytes_read_in_chunk) {
                *__errno_location() = EOVERFLOW;
                ret_val = -1;
                file_ended = 1; // Mark file as ended due to error
            } else {
                total_bytes_read += bytes_read_in_chunk;
            }
        }

        // If an error occurred or EOF was reached, and it's handled, exit the main loop.
        if (file_ended && ret_val == -1) {
            break; // Exit outer loop due to error
        }
        if (bytes_read_in_chunk < buffer_length_1 && file_ended) {
            // This condition is met after processing the last partial chunk (or empty if file was empty).
            // At this point, all data has been read and processed (unless ret_val is already -1).
            break; // Exit the main reading loop
        }
    }

    // If we reached here without an error, it's a success
    if (ret_val != -1) { // Check if no error occurred previously
        *param_2 = current_sum;
        *param_3 = total_bytes_read;
        ret_val = 0; // Success
    }

    free(buffer); // Free the allocated buffer

    if (stack_chk_guard_val != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return ret_val;
}

// Function: sysv_sum_stream
undefined4 sysv_sum_stream(FILE *param_1, int *param_2, long *param_3) {
    long stack_chk_guard_val = __stack_chk_guard; // Store stack canary
    int ret_val = -1; // Initialize to error (failure)
    unsigned int current_sum = 0;
    long total_bytes_read = 0;

    char *buffer = (char *)malloc(buffer_length_0);
    if (buffer == NULL) {
        return -1; // Memory allocation failed
    }

    int file_ended = 0; // Flag to indicate if EOF or an error occurred during read

    // Main loop to read and process chunks
    while (!file_ended) {
        size_t bytes_read_in_chunk = 0;
        // Inner loop to fill the buffer completely or until EOF/error
        while (bytes_read_in_chunk < buffer_length_0) {
            size_t s_read = fread_unlocked(buffer + bytes_read_in_chunk, 1, buffer_length_0 - bytes_read_in_chunk, param_1);
            bytes_read_in_chunk += s_read;

            if (s_read == 0) { // EOF or error occurred during read
                if (ferror_unlocked(param_1)) {
                    ret_val = -1; // Set error code
                }
                file_ended = 1; // Mark file as ended (either EOF or error)
                break; // Break from inner loop to process any data that was read
            }
        }

        // Process the data read in this chunk (even if partial due to EOF)
        if (bytes_read_in_chunk > 0) {
            for (size_t i = 0; i < bytes_read_in_chunk; ++i) {
                current_sum += (unsigned char)buffer[i];
            }

            // Check for total_bytes_read overflow
            if (total_bytes_read > LONG_MAX - bytes_read_in_chunk) {
                *__errno_location() = EOVERFLOW;
                ret_val = -1;
                file_ended = 1; // Mark file as ended due to error
            } else {
                total_bytes_read += bytes_read_in_chunk;
            }
        }

        // If an error occurred or EOF was reached, and it's handled, exit the main loop.
        if (file_ended && ret_val == -1) {
            break; // Exit outer loop due to error
        }
        if (bytes_read_in_chunk < buffer_length_0 && file_ended) {
            // This condition is met after processing the last partial chunk (or empty if file was empty).
            // At this point, all data has been read and processed (unless ret_val is already -1).
            break; // Exit the main reading loop
        }
    }

    // If we reached here without an error, it's a success
    if (ret_val != -1) { // Check if no error occurred previously
        // SysV sum algorithm: fold 32-bit sum into 16 bits
        unsigned int folded_sum = (current_sum & 0xffff) + (current_sum >> 0x10);
        *param_2 = (folded_sum & 0xffff) + (folded_sum >> 0x10); // Fold again
        *param_3 = total_bytes_read;
        ret_val = 0; // Success
    }

    free(buffer); // Free the allocated buffer

    if (stack_chk_guard_val != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return ret_val;
}

// Function: output_bsd
void output_bsd(undefined8 filename_ptr, undefined8 unused_param_2, unsigned int *sum_ptr, char binary_output_flag, undefined8 unused_param_5,
               byte separator_char, char show_filename_flag, undefined8 file_size_val) {
    long stack_chk_guard_val = __stack_chk_guard; // Store stack canary
    char human_readable_buf[664]; // Using a fixed size as per original
    unsigned short sum_output;

    if (binary_output_flag == '\0') { // If not binary output
        human_readable(file_size_val, human_readable_buf, 0, 1, sizeof(human_readable_buf));
        printf("%05u %5s", *sum_ptr, human_readable_buf); // Changed %d to %u for unsigned int
        if (show_filename_flag != '\0') {
            printf(" %s", (char*)filename_ptr); // Assuming filename_ptr is a char* for filename
        }
        putchar_unlocked(separator_char);
    } else { // Binary output
        sum_output = (unsigned short)*sum_ptr;
        sum_output = __bswap_16(sum_output); // Byte swap for big-endian output
        fwrite_unlocked(&sum_output, 1, 2, stdout); // Changed _stdout to stdout
    }

    if (stack_chk_guard_val != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return;
}

// Function: output_sysv
void output_sysv(undefined8 filename_ptr, undefined8 unused_param_2, unsigned int *sum_ptr, char binary_output_flag, undefined8 unused_param_5,
                byte separator_char, char show_filename_flag, undefined8 file_size_val) {
    long stack_chk_guard_val = __stack_chk_guard; // Store stack canary
    char human_readable_buf[664]; // Using a fixed size as per original
    unsigned short sum_output;

    if (binary_output_flag == '\0') { // If not binary output
        human_readable(file_size_val, human_readable_buf, 0, 1, sizeof(human_readable_buf));
        printf("%u %s", *sum_ptr, human_readable_buf); // Changed %d to %u for unsigned int
        if (show_filename_flag != '\0') {
            printf(" %s", (char*)filename_ptr); // Assuming filename_ptr is a char* for filename
        }
        putchar_unlocked(separator_char);
    } else { // Binary output
        sum_output = (unsigned short)*sum_ptr;
        sum_output = __bswap_16(sum_output); // Byte swap for big-endian output
        fwrite_unlocked(&sum_output, 1, 2, stdout); // Changed _stdout to stdout
    }

    if (stack_chk_guard_val != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return;
}