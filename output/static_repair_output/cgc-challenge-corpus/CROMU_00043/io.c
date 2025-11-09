#include <stdio.h>    // For fwrite (used in dummy transmit), stdout
#include <stdlib.h>   // For exit, EXIT_FAILURE
#include <string.h>   // For strlen, memset
#include <stdarg.h>   // For va_list, va_start, va_end, va_arg
#include <limits.h>   // For INT_MIN

// --- Type Definitions ---
typedef unsigned char byte;
typedef unsigned int uint;

// --- Forward Declarations ---
void _terminate(void);
int receive(int fd, void *buf, size_t len, int *bytes_received_out);
int transmit(int fd, const void *buf, size_t count, int *bytes_written_out);
void TransmitBytes(const char *buf, size_t len);
char itoh(byte value, int high_nibble);
void vTransmitFormattedBytes(const char *format, va_list args);
void TransmitFormattedBytes(const char *format, ...);
void FailAndTerminate(const char *message);

// --- Dummy/Placeholder Implementations for System Calls ---

// Function: _terminate
// Terminates the program with an error status.
void _terminate(void) {
    exit(EXIT_FAILURE);
}

// Function: receive
// Placeholder for a system-level receive function.
// Simulates receiving data. For a real system, this would interact with hardware or OS.
// Returns 0 on success, non-zero on error.
int receive(int fd, void *buf, size_t len, int *bytes_received_out) {
    (void)fd; // Unused parameter
    (void)buf; // Unused parameter
    if (bytes_received_out) {
        // Simulate receiving some bytes, e.g., half of requested, or 0 if len is small.
        *bytes_received_out = (len > 0) ? (len / 2 + 1) : 0;
        if (*bytes_received_out > len) *bytes_received_out = len;
    }
    return 0; // Simulate success
}

// Function: transmit
// Placeholder for a system-level transmit function.
// For Linux compilation, uses `fwrite` to stdout.
// Returns 0 on success, non-zero on error.
int transmit(int fd, const void *buf, size_t count, int *bytes_written_out) {
    size_t written_count = fwrite(buf, 1, count, (fd == 1) ? stdout : stderr);
    if (bytes_written_out) {
        *bytes_written_out = (int)written_count;
    }
    return 0; // Simulate success
}

// --- Original Functions (Fixed) ---

// Function: FailAndTerminate
// Prints an error message and terminates the program.
void FailAndTerminate(const char *message) {
    TransmitFormattedBytes("ERROR: +s\n", message);
    _terminate();
}

// Function: ReceiveBytes
// Continuously receives bytes into a buffer until the specified length is met.
// Calls FailAndTerminate on receive errors or zero bytes received.
void ReceiveBytes(void *buf, size_t len) {
    int bytes_received_this_call;
    while (len > 0) {
        if (receive(0, buf, len, &bytes_received_this_call) != 0) {
            FailAndTerminate("failed during receive");
        }
        if (bytes_received_this_call == 0) {
            FailAndTerminate("zero bytes received");
        }
        buf = (char *)buf + bytes_received_this_call;
        len -= bytes_received_this_call;
    }
}

// Function: itoh
// Converts a nibble of a byte to its hexadecimal character representation.
// `high_nibble` is 1 to convert the high nibble, 0 for the low nibble.
char itoh(byte value, int high_nibble) {
    byte nibble = value;
    if (high_nibble) {
        nibble >>= 4;
    }
    nibble &= 0xF;

    if (nibble < 10) {
        return nibble + '0';
    } else {
        return nibble - 10 + 'A';
    }
}

// Function: HexDump
// Prints a hexadecimal dump of the given data to the output.
void HexDump(const void *data, uint len) {
    const unsigned char *buf = (const unsigned char *)data;
    char line_buf[50]; // Max: 16*3 chars + '\n' + '\0' = 48 + 1 + 1 = 50
    uint offset = 0;

    while (offset < len) {
        memset(line_buf, 0, sizeof(line_buf));
        int current_line_idx = 0;
        uint bytes_in_this_line = 0;

        for (; bytes_in_this_line < 16 && (offset + bytes_in_this_line < len);
             bytes_in_this_line++) {
            byte current_byte = buf[offset + bytes_in_this_line];
            line_buf[current_line_idx++] = itoh(current_byte, 1);
            line_buf[current_line_idx++] = itoh(current_byte, 0);
            line_buf[current_line_idx++] = ' '; // Always add space after a byte's hex representation
        }
        // Remove trailing space if it's the last character before newline
        if (current_line_idx > 0 && line_buf[current_line_idx - 1] == ' ') {
            current_line_idx--;
        }
        line_buf[current_line_idx++] = '\n';
        TransmitBytes(line_buf, strlen(line_buf));
        offset += bytes_in_this_line;
    }
}

// Function: TransmitBytes
// Transmits a buffer of bytes. Handles partial writes.
// Calls _terminate on transmit errors or zero bytes transmitted (stall).
void TransmitBytes(const char *buf, size_t len) {
    int bytes_transmitted_this_call;
    while (len > 0) {
        if (transmit(1, buf, len, &bytes_transmitted_this_call) != 0) { // Assume fd=1 for stdout
            _terminate();
        }
        if (bytes_transmitted_this_call == 0) {
            _terminate(); // Transmit reported no error but wrote 0 bytes, likely a stall.
        }
        buf += bytes_transmitted_this_call;
        len -= bytes_transmitted_this_call;
    }
}

// Function: vTransmitFormattedBytes
// Formats and transmits bytes based on a format string and a va_list of arguments.
// Custom format specifiers: +x (hex), +u (unsigned int), +s (string), +d (signed int),
// +! (write value to address), ++ (literal '+').
void vTransmitFormattedBytes(const char *format, va_list args) {
    if (!format) {
        return;
    }

    const char *current_segment_start;
    char temp_buf[32]; // Sufficient for int-to-string conversions

    while (*format != '\0') {
        current_segment_start = format;
        while (*format != '+' && *format != '\0') {
            format++;
        }

        if (format > current_segment_start) {
            TransmitBytes(current_segment_start, format - current_segment_start);
        }

        if (*format == '+') {
            char specifier = format[1];
            format += 2; // Advance past '+X'

            switch (specifier) {
                case 'x': { // Hexadecimal output (uint)
                    uint val = va_arg(args, uint);
                    temp_buf[0] = '0';
                    temp_buf[1] = 'x';
                    for (int i = 9; i >= 2; --i) { // 8 hex digits + "0x"
                        temp_buf[i] = itoh((byte)(val & 0xF), 0);
                        val >>= 4;
                    }
                    TransmitBytes(temp_buf, 10); // "0x" + 8 hex digits
                    break;
                }
                case 'u': { // Unsigned integer output (uint)
                    uint val = va_arg(args, uint);
                    int i = sizeof(temp_buf) - 1;
                    temp_buf[i--] = '\0';

                    if (val == 0) {
                        temp_buf[i--] = '0';
                    } else {
                        while (val > 0) {
                            temp_buf[i--] = (val % 10) + '0';
                            val /= 10;
                        }
                    }
                    TransmitBytes(&temp_buf[i + 1], sizeof(temp_buf) - 1 - (i + 1));
                    break;
                }
                case 's': { // String output (const char *)
                    const char *s = va_arg(args, const char *);
                    if (s) {
                        TransmitBytes(s, strlen(s));
                    }
                    break;
                }
                case 'd': { // Signed integer output (int)
                    int val = va_arg(args, int);
                    int i = sizeof(temp_buf) - 1;
                    temp_buf[i--] = '\0';
                    int is_negative = 0;

                    if (val < 0) {
                        is_negative = 1;
                        // Use negative numbers for modulo and division to handle INT_MIN correctly
                    }
                    
                    if (val == 0) {
                        temp_buf[i--] = '0';
                    } else {
                        while (val != 0) {
                            temp_buf[i--] = '0' - (val % 10);
                            val /= 10;
                        }
                    }

                    if (is_negative) {
                        temp_buf[i--] = '-';
                    }
                    TransmitBytes(&temp_buf[i + 1], sizeof(temp_buf) - 1 - (i + 1));
                    break;
                }
                case '!': { // Special: Write value to address (void *addr, int value)
                    // WARNING: This operation is inherently dangerous and non-portable.
                    // It allows writing an integer value to an arbitrary memory address.
                    // Use with extreme caution.
                    void *addr = va_arg(args, void *);
                    int value = va_arg(args, int);
                    *(int *)addr = value;
                    break;
                }
                case '+': { // Literal '+'
                    TransmitBytes("+", 1);
                    break;
                }
                default:
                    // Unknown specifier, stop processing the format string.
                    return;
            }
        }
    }
}

// Function: TransmitFormattedBytes
// Variadic wrapper for vTransmitFormattedBytes.
void TransmitFormattedBytes(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vTransmitFormattedBytes(format, args);
    va_end(args);
}

// --- Main Function (Example) ---

int main() {
    TransmitFormattedBytes("Hello, +s!\n", "World");
    TransmitFormattedBytes("Value: +d, Unsigned: +u, Hex: +x\n", -12345, 67890U, 0xDEADBEEF);
    TransmitFormattedBytes("Another hex value: +x\n", 0x123);
    TransmitFormattedBytes("Test negative zero: +d\n", 0); // Should print 0
    TransmitFormattedBytes("Test INT_MIN: +d\n", INT_MIN);

    char buffer[32];
    memset(buffer, 0xAA, sizeof(buffer));
    TransmitFormattedBytes("Hex dump of buffer (32 bytes of 0xAA):\n");
    HexDump(buffer, sizeof(buffer));

    memset(buffer, 0x12, 5);
    memset(buffer + 5, 0x34, 10);
    memset(buffer + 15, 0x56, 7);
    TransmitFormattedBytes("Hex dump of mixed buffer (22 bytes):\n");
    HexDump(buffer, 22);


    // Example of +! (dangerous memory write)
    int my_var = 0;
    TransmitFormattedBytes("Before +!, my_var is: +d\n", my_var);
    TransmitFormattedBytes("Setting my_var to 10 using +!: +!\n", &my_var, 10);
    TransmitFormattedBytes("After +!, my_var is: +d\n", my_var); // Should print 10

    // ReceiveBytes example (will use dummy receive)
    TransmitFormattedBytes("Attempting to receive 10 bytes...\n");
    char receive_buffer[10];
    ReceiveBytes(receive_buffer, sizeof(receive_buffer));
    TransmitFormattedBytes("Received 10 bytes (dummy data).\n");

    // Uncomment the line below to test program termination on error
    // FailAndTerminate("This is a test termination message.");

    return 0;
}