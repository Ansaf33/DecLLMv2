#include <stdbool.h> // For bool type
#include <stdint.h>  // For uint32_t, uint8_t types
#include <stdio.h>   // For NULL, stdin, stdout, fprintf
#include <string.h>  // For strchr, strlen
#include <unistd.h>  // For read, write, ssize_t

// --- Global Variables ---
// gPCRs: Array of 8 PCRs, each 0x80 (128) bytes.
// Total size: 8 * 128 = 1024 bytes.
uint8_t gPCRs[8 * 0x80];

// gNibToHexChar: Lookup table for converting nibbles (4-bit values) to hex characters.
const char gNibToHexChar[] = "0123456789abcdef";

// --- Mock/Helper Functions ---

// Mock function for fdwait.
// In the original decompiler output, param_1 (a uint) is treated as an address.
// This function is likely a misinterpretation of a system call to check memory validity.
// For compilation, we assume any address >= 0x1000 is "valid" in this context.
// The original `isValidAddr` returns true if `fdwait` returns anything other than 2.
// So, we'll return 0, which signifies "valid".
int fdwait(uint32_t timeout, void *readfds, void *writefds, uint32_t addr, int unknown) {
    (void)timeout; // Suppress unused parameter warnings
    (void)readfds;
    (void)writefds;
    (void)addr;
    (void)unknown;
    return 0; // Mocking a successful (non-2) return
}

// Mock function for receive_until. Reads from 'fd' into 'buffer' until 'terminator_char'
// is encountered or 'max_len' bytes are read.
ssize_t receive_until(int fd, char *buffer, size_t max_len, char terminator_char) {
    ssize_t bytes_read = 0;
    char c;
    // Read up to max_len-1 to leave space for null terminator
    while (bytes_read < max_len - 1 && read(fd, &c, 1) == 1) {
        buffer[bytes_read++] = c;
        if (c == terminator_char) {
            break;
        }
    }
    buffer[bytes_read] = '\0'; // Null-terminate the received string
    return bytes_read;
}

// Mock function for receive_all. Reads exactly 'len' bytes from 'fd' into 'buffer'.
ssize_t receive_all(int fd, char *buffer, size_t len) {
    ssize_t total_read = 0;
    while (total_read < len) {
        ssize_t bytes_read_chunk = read(fd, buffer + total_read, len - total_read);
        if (bytes_read_chunk <= 0) {
            // Error or EOF, return bytes read so far (or -1 if error)
            return bytes_read_chunk == 0 ? total_read : -1;
        }
        total_read += bytes_read_chunk;
    }
    return total_read;
}

// Mock function for transmit_all. Writes exactly 'len' bytes from 'buffer' to 'fd'.
ssize_t transmit_all(int fd, const char *buffer, size_t len) {
    ssize_t total_written = 0;
    while (total_written < len) {
        ssize_t bytes_written_chunk = write(fd, buffer + total_written, len - total_written);
        if (bytes_written_chunk <= 0) {
            // Error, return -1
            return -1;
        }
        total_written += bytes_written_chunk;
    }
    return total_written;
}

// --- Original Functions (fixed) ---

// Function: isValidAddr
bool isValidAddr(uint32_t addr) {
    if (addr < 0x1000) {
        return false;
    }
    // The original `fdwait` call is likely a decompiler artifact for a memory check.
    // We've mocked fdwait to return 0 for valid addresses.
    // The original logic was `iVar1 != 2`, so 0 != 2 is true.
    return fdwait(0xffffffff, NULL, NULL, addr, 0) != 2;
}

// Function: xorHash
// Performs XOR hash operation: dst[i] ^= src[i] for min(0x80, size) bytes.
uint32_t xorHash(const uint8_t *src, uint32_t size, uint8_t *dst) {
    for (uint32_t i = 0; i < 0x80; ++i) {
        if (i < size) {
            dst[i] ^= src[i];
        }
    }
    return 0;
}

// Function: HASH
// Hashes a memory region by XORing 0x80-byte chunks into a hash_buffer.
int HASH(uint32_t start_addr, int total_size, uint8_t *hash_buffer) {
    if (start_addr == 0 || hash_buffer == NULL) {
        return -1;
    }

    // Initialize hash_buffer to all zeros
    for (uint32_t i = 0; i < 0x80; ++i) {
        hash_buffer[i] = 0;
    }

    uint32_t current_offset_in_total_size = 0;
    uint32_t current_addr_to_hash = start_addr;

    while (current_addr_to_hash < (uint32_t)total_size + start_addr) {
        if (!isValidAddr(current_addr_to_hash)) {
            // If address is not valid, align to the next 0x1000 boundary
            current_addr_to_hash = (current_addr_to_hash + 0x1000) & 0xfffff000;
        } else {
            // Perform XOR hash on the current 0x80-byte chunk
            int xor_result = xorHash((const uint8_t*)current_addr_to_hash,
                                     total_size - current_offset_in_total_size,
                                     hash_buffer);
            if (xor_result != 0) {
                return xor_result; // Propagate error from xorHash
            }
            current_addr_to_hash += 0x80;
            current_offset_in_total_size += 0x80;
        }
    }
    return 0; // Success
}

// Function: EXTEND
// Extends a PCR by hashing a memory region and XORing it with the PCR.
uint32_t EXTEND(int pcr_index, uint32_t start_addr, uint32_t end_addr) {
    if (start_addr < end_addr) {
        uint8_t local_hash_buffer[0x80];
        uint32_t size_to_hash = end_addr - start_addr;

        HASH(start_addr, size_to_hash, local_hash_buffer);
        // XOR the computed hash with the specified PCR
        xorHash(local_hash_buffer, 0x80, gPCRs + pcr_index * 0x80);
        return 0; // Success
    } else {
        return 0xffffffff; // Error: Invalid address range
    }
}

// Function: INIT
// Initializes a given buffer with a specified fill_byte.
uint32_t INIT(uint8_t *buffer, uint8_t fill_byte) {
    if (buffer == NULL) {
        return 0xffffffff; // Error: Null buffer
    } else {
        for (uint32_t i = 0; i < 0x80; ++i) {
            buffer[i] = fill_byte;
        }
        return 0; // Success
    }
}

// Function: ZERO
// Zeros out a specific PCR.
uint32_t ZERO(uint32_t pcr_index) {
    if (pcr_index < 8) { // Assuming 8 PCRs (0-7)
        return INIT(gPCRs + pcr_index * 0x80, 0);
    } else {
        return 0xffffffff; // Error: Invalid PCR index
    }
}

// Function: RESET
// Resets all PCRs, initializing each PCR[i] with the value 'i'.
int RESET(void) {
    for (uint32_t i = 0; i < 8; ++i) { // Iterate through 8 PCRs
        int init_result = INIT(gPCRs + i * 0x80, (uint8_t)i);
        if (init_result != 0) {
            return init_result; // Propagate error from INIT
        }
    }
    return 0; // Success
}

// Function: hexStrToUint32
// Converts a hexadecimal string to a uint32_t value.
// Returns 0 if an invalid character is encountered, or the converted value.
uint32_t hexStrToUint32(const char *hex_str) {
    uint32_t value = 0;
    for (int i = 0; hex_str[i] != '\0'; ++i) {
        uint32_t digit;
        char c = hex_str[i];
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else {
            return 0; // Invalid character, return 0 as error/default
        }
        value = (value << 4) | (digit & 0xf);
    }
    return value;
}

// Function: hashToHexStr
// Converts a 0x80-byte hash buffer into a null-terminated hexadecimal string.
// The output string will be 0x80 * 2 characters long, plus a null terminator.
void hashToHexStr(const uint8_t *hash_buffer, char *hex_str_buffer) {
    for (uint32_t i = 0; i < 0x80; ++i) {
        uint8_t byte_val = hash_buffer[i];
        hex_str_buffer[i * 2] = gNibToHexChar[(byte_val >> 4) & 0xf];
        hex_str_buffer[i * 2 + 1] = gNibToHexChar[byte_val & 0xf];
    }
    hex_str_buffer[0x80 * 2] = '\0'; // Null-terminate the string
}

// Function: findChar
// Finds the first occurrence of 'target_char' in 'str'.
// Returns the index of the character, or -1 if not found.
int findChar(const char *str, char target_char) {
    int i = 0;
    while (str[i] != '\0' && str[i] != target_char) {
        i++;
    }
    if (str[i] == '\0') {
        return -1; // Character not found
    }
    return i; // Index where character was found
}

// Function: main
int main(void) {
    // Buffer for receiving commands (e.g., "E 0 1000 2000")
    // Needs to be large enough for command char + space + args + spaces + null terminator.
    // 0x80 (128) max bytes for `receive_until` + 1 byte for command_buffer[0] + 1 for null = 130.
    // Using 256 for safety.
    char command_buffer[256];

    // Buffer for hexadecimal output (0x80 bytes hash -> 256 hex chars + 1 newline + 1 null)
    char hex_str_output[258]; // 256 chars + '\n' + '\0'

    // Local buffer for HASH/xorHash operations
    uint8_t local_hash_buffer[0x80];

    // Buffer for data received via `receive_all`
    uint8_t receive_buffer[0x80];

    // Variables for parsing and command execution
    uint32_t pcr_index_val;
    uint32_t addr_val;
    uint32_t size_param;
    uint32_t end_addr_calc;
    uint32_t bytes_received_len;
    uint32_t current_hash_offset;
    ssize_t actual_receive_len;

    while (true) { // Main command processing loop
        // Receive command line from stdin (file descriptor 0)
        // Reads into command_buffer + 1, leaving command_buffer[0] unused as per original.
        // Terminates on newline (10) or after 0x80 bytes.
        bytes_received_len = receive_until(0, command_buffer + 1, 0x80, '\n');
        command_buffer[bytes_received_len + 1] = '\0'; // Ensure null termination for parsing

        char command_char = command_buffer[1]; // The actual command character (e.g., 'E', 'H', 'P', 'R', 'Z')

        // Check for empty command or I/O error
        if (bytes_received_len == 0 || command_char == '\0') {
            continue; // Keep looping if nothing received or just a newline
        }
        if (command_char < 0 || command_char > 0x5a || command_char < 0x45) {
            // Original code checks for iVar1 < 0, 0x5a < iVar1, iVar1 < 0x45
            // This filters for commands between 'E' (0x45) and 'Z' (0x5a) inclusive.
            return 0; // Exit on invalid command character
        }

        // Pointers for parsing arguments
        char *cmd_line = command_buffer + 1; // Start of the actual command string (e.g., "E 0 1000 2000")
        char *token_ptr = cmd_line;
        char *end_token;

        switch (command_char) {
            case 'E': // EXTEND command: E <pcr_idx> <start_addr> <size>
                // Skip 'E'
                end_token = strchr(token_ptr, ' ');
                if (!end_token) return 0; // Malformed command
                token_ptr = end_token + 1; // Points to start of first argument (pcr_idx)

                // Parse pcr_index_val
                end_token = strchr(token_ptr, ' ');
                if (!end_token) return 0; // Malformed command
                *end_token = '\0'; // Null-terminate for hexStrToUint32
                pcr_index_val = hexStrToUint32(token_ptr);
                token_ptr = end_token + 1; // Points to start of second argument (start_addr)

                // Parse addr_val (start_addr)
                end_token = strchr(token_ptr, ' ');
                if (!end_token) return 0; // Malformed command
                *end_token = '\0'; // Null-terminate for hexStrToUint32
                addr_val = hexStrToUint32(token_ptr);
                token_ptr = end_token + 1; // Points to start of third argument (size)

                // Parse size_param
                size_param = hexStrToUint32(token_ptr);

                end_addr_calc = addr_val + size_param;
                if (end_addr_calc <= addr_val) { // Check for overflow or invalid range (size 0)
                    return 0;
                }
                EXTEND((int)pcr_index_val, addr_val, end_addr_calc);
                break;

            case 'H': // HASH command: H <size_to_hash>
            case 'P': // PRINT PCR command: P <pcr_idx>
            case 'Z': // ZERO PCR command: Z <pcr_idx>
                // Skip command char ('H', 'P', or 'Z')
                end_token = strchr(token_ptr, ' ');
                if (!end_token) return 0; // Malformed command
                token_ptr = end_token + 1; // Points to the single argument

                pcr_index_val = hexStrToUint32(token_ptr); // This variable name is reused for the argument value

                if (command_char == 'H') {
                    if (pcr_index_val == 0xffffffff) return 0; // Invalid hex string for size

                    INIT(local_hash_buffer, 0); // Initialize local hash buffer to zeros

                    current_hash_offset = 0;
                    while (current_hash_offset < pcr_index_val) {
                        size_param = pcr_index_val - current_hash_offset; // Remaining bytes to hash
                        if (size_param > 0x80) { // Limit chunk size to 0x80 bytes
                            size_param = 0x80;
                        }
                        actual_receive_len = receive_all(0, (char*)receive_buffer, size_param);
                        if (actual_receive_len <= 0) { // Error or EOF during receive
                            return 0;
                        }
                        xorHash(receive_buffer, actual_receive_len, local_hash_buffer);
                        current_hash_offset += actual_receive_len;
                    }
                    hashToHexStr(local_hash_buffer, hex_str_output);
                    hex_str_output[256] = '\n'; // Add newline character
                    transmit_all(1, hex_str_output, 257); // Transmit 256 hex chars + newline
                } else if (command_char == 'P') {
                    if (pcr_index_val > 7) return 0; // Invalid PCR index
                    hashToHexStr(gPCRs + pcr_index_val * 0x80, hex_str_output);
                    hex_str_output[256] = '\n'; // Add newline character
                    transmit_all(1, hex_str_output, 257); // Transmit 256 hex chars + newline
                } else { // command_char == 'Z'
                    ZERO(pcr_index_val); // Zero out the specified PCR
                }
                break; // Continue the main loop

            case 'R': // RESET command: R
                RESET();
                break; // Continue the main loop

            default:
                return 0; // Unknown command
        }
    }
    return 0; // Should not be reached in an infinite loop unless an explicit exit is added.
}