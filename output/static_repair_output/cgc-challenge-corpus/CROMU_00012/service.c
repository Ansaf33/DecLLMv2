#include <stdio.h>    // For STDIN_FILENO (if `recv` is used for stdin)
#include <string.h>   // For memset, memcpy
#include <stdlib.h>   // For exit
#include <stdint.h>   // For fixed-width integers (uint8_t, uint16_t, uint32_t, int16_t, int32_t)
#include <sys/socket.h> // For recv (assuming network socket operations)
#include <unistd.h>   // For ssize_t, STDIN_FILENO
#include <stdbool.h>  // For bool type

// --- Type Definitions for Decompiled Code Clarity ---
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef int16_t short_t;
typedef int32_t int_t;

// --- Placeholder for Global Data ---
// Assuming DAT_00014044 is a constant string literal.
const char DAT_00014044[] = "ALARM_MSG!";

// --- Placeholder for Decompiler Artifact Functions ---
// validatePtr: Assumed to be a no-op or debug assertion, removed for production code.
#define validatePtr(...) (void)0

// validateRet: Assumed to be a no-op or debug assertion, removed for production code.
#define validateRet() (void)0

// validateStatus: Assumed to be a function that returns an integer status.
// Original `iVar2 = validatePtr();` suggests it returns an int and affects control flow.
// For compilation, it's assumed to always return success (1).
int validateStatus(void) {
    // In a real system, this might check system health, memory, etc.
    // For this exercise, assume it always passes.
    return 1;
}

// transmit_all: Placeholder for a function that sends data, likely over a socket.
// Signature derived from `serviceLoop` and `bookKeeping` calls.
void transmit_all(const void *buf, size_t len, int flags) {
    (void)buf;    // Suppress unused parameter warnings
    (void)len;    // Suppress unused parameter warnings
    (void)flags;  // Suppress unused parameter warnings
    // In a real scenario, this would involve socket `send` calls or similar I/O.
    // Example: send(socket_fd, buf, len, flags);
}

// _terminate: Placeholder for a function that exits the program.
void _terminate(void) {
    exit(1);
}

// --- Function Declarations (Forward) ---
int recvUntil(int sockfd, char *buffer, size_t max_len, char delimiter);
void bookKeeping(byte *car_data);
void raiseAlarm(void);
void handleShift(byte *car_data, char shift_direction);
void updateBTMetadata(void *dest_struct, void *src_struct);
void handleTrunk(void *car_data_ptr, char *message_data);
void initCar(void *car_data_ptr);
void serviceLoop(void); // Declared here for main

// --- Function Definitions ---

// Function: recvUntil
int recvUntil(int sockfd, char *buffer, size_t max_len, char delimiter) {
    char current_char;
    ssize_t bytes_read;
    int count = 0; // Renamed local_10

    while (count < max_len) {
        bytes_read = recv(sockfd, &current_char, 1, 0);
        if (bytes_read <= 0) { // Error or connection closed
            return (bytes_read == 0) ? count : -1;
        }
        if (delimiter == current_char) {
            break;
        }
        buffer[count++] = current_char; // Write and increment count
    }
    validateRet();
    return count;
}

// Function: bookKeeping
void bookKeeping(byte *car_data) { // Renamed param_1 to car_data
    validatePtr(car_data);

    if (6000 < *(ushort *)(car_data + 3)) {
        transmit_all(DAT_00014044, sizeof(DAT_00014044) - 1, 1); // -1 for null terminator
        _terminate();
    }

    if (car_data[8] != 0x10) {
        int_t penalty = (*(ushort *)(car_data + 3) >> 2) / 0x19 + (uint)car_data[0] +
                      *(int_t *)(car_data + 0xe) / 100;
        *(short_t *)(car_data + 1) -= (short_t)penalty;

        if (car_data[10] == (char)0xf0) {
            *(short_t *)(car_data + 1) -= 5;
        }
        if (car_data[0xb] == (char)0xd0) {
            *(short_t *)(car_data + 1) -= 2;
        }

        // Original condition `((param_1[5] != 0xff) || (param_1[5] != 1))` is always true.
        // Assuming it was intended as `(param_1[5] != 0xff && param_1[5] != 1)`.
        if ((car_data[5] != (char)0xff) && (car_data[5] != 1) && (0x14 < car_data[6])) {
            car_data[0]++;
            *(ushort *)(car_data + 3) += 0x19;
        }
    }
    validateRet();
}

// Function: raiseAlarm
void raiseAlarm(void) {
    char buffer[5]; // Renamed local_d
    // Fix: memcpy size should not exceed buffer size
    memcpy(buffer, "AAAAAAAAAAAAAAAA", sizeof(buffer)); // Copies 5 bytes
    validateRet();
}

// Function: handleShift
void handleShift(byte *car_data, char shift_direction) { // Renamed param_1, param_2
    validatePtr(car_data);

    if (shift_direction == 'r') {
        if ((car_data[8] < 0x60) && (*(ushort *)(car_data + 3) > 500)) {
            if (car_data[8] < 0x11) {
                car_data[8] += 0x10;
            } else {
                // Simplified condition from complex SBORROW4 logic
                uint val1 = (uint)car_data[0];
                uint val2 = (uint)(car_data[8] >> 4) * 10;
                if (val1 == val2) {
                    validateRet();
                    return;
                }
                car_data[8] += 0x10;
            }

            if (*(ushort *)(car_data + 3) < 0x2ee) {
                *(ushort *)(car_data + 3) = 0;
            } else {
                *(short_t *)(car_data + 3) -= 0x2ee;
            }
        }
    } else { // Implicit 'l' for left shift or other char
        if (car_data[8] > 0x10) {
            car_data[8] -= 0x10;
            *(short_t *)(car_data + 3) += 0x2ee;
        }
    }
    validateRet();
}

// Function: updateBTMetadata
void updateBTMetadata(void *dest_struct, void *src_struct) { // Renamed param_1, param_2
    validatePtr(dest_struct);
    validatePtr(src_struct);
    byte *d = (byte *)dest_struct;
    byte *s = (byte *)src_struct;
    size_t len = (size_t)s[1]; // Length is at offset 1 of src_struct
    memcpy(d + 0x12, s + 2, len); // Data starts at offset 2 of src_struct
    validateRet();
}

// Function: handleTrunk
void handleTrunk(void *car_data_ptr, char *message_data) { // Renamed param_1, param_2
    validatePtr(car_data_ptr);
    validatePtr(message_data);
    int_t *car_e_ptr = (int_t *)((byte *)car_data_ptr + 0xe); // Access car_data[0xe] as int
    int8_t msg_val = message_data[1]; // Value is at offset 1 of message_data

    if (message_data[0] == (char)0x90) { // -0x70
        *car_e_ptr += msg_val;
    } else if (*car_e_ptr < msg_val) {
        *car_e_ptr = 0;
    } else {
        *car_e_ptr -= msg_val;
    }
    validateRet();
}

// Function: initCar
void initCar(void *car_data_ptr) { // Renamed param_1
    validatePtr(car_data_ptr);
    byte *cd = (byte *)car_data_ptr;
    cd[8] = 0x10;
    cd[7] = 0x10;
    cd[0xc] = (char)0xa0;
    cd[0xb] = (char)0xc0;
    cd[10] = (char)0xe0;
    cd[0xd] = (char)0xf9;
    cd[9] = 0x75;
    *(ushort *)(cd + 1) = 0xffff;
    validateRet();
}

// Function: main
int main(void) {
    serviceLoop();
    validateRet();
    return 0;
}

// Function: serviceLoop
void serviceLoop(void) {
    char input_buffer[256]; // Corresponds to local_154
    byte car_data[52];      // Corresponds to local_54 (and local_20 points to it)
    int bytes_read_count;   // Corresponds to local_18

    validatePtr(car_data);
    memset(car_data, 0, sizeof(car_data)); // 0x32 (50) was hardcoded, using sizeof for safety
    initCar(car_data);

    while (true) { // Outer infinite service loop
        memset(input_buffer, 0, sizeof(input_buffer)); // 0x100 (256) was hardcoded, using sizeof for safety

        // Call to recvUntil inferred from stack setup in original code:
        // recvUntil(sockfd=0 (STDIN_FILENO), buffer=local_154, max_len=0xff, delimiter=10 ('\n'))
        bytes_read_count = recvUntil(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n');
        if (bytes_read_count < 1) {
            _terminate();
        }

        bookKeeping(car_data);

        int i = 0; // Loop counter for processing input_buffer
        while (i < bytes_read_count) {
            char current_char = input_buffer[i];
            char *current_input_ptr = &input_buffer[i]; // Pointer to current char, used for multi-byte commands

            if (current_char == '0') {
                validatePtr(car_data);
                if ((car_data[7] == 0x10) || ((car_data[7] == ' ') && (car_data[8] == 0x10))) {
                    car_data[7] = '0';
                }
                i++; // Consume one character
            } else if (current_char < '1') { // Handles ' ', '\x10', and various negative char codes
                if (current_char == ' ') {
                    validatePtr(car_data);
                    if ((car_data[7] == 0x10) || (car_data[7] == '0')) {
                        car_data[7] = ' ';
                    }
                    i++;
                } else if (current_char == 0x10) { // ASCII 16
                    validatePtr(car_data);
                    if (car_data[8] == 0x10) {
                        car_data[7] = 0x10;
                    } else {
                        *(ushort *)(car_data + 3) = 9999;
                    }
                    i++;
                } else if (current_char < 0x11 && current_char > -9) { // Range between -8 and 16 (exclusive)
                    if (current_char == (char)0xF8) { // -8
                        validatePtr(car_data);
                        car_data[0xd] = *current_input_ptr;
                    }
                    i++;
                } else if (current_char == (char)0xDD) { // -0x23
                    validatePtr(car_data);
                    transmit_all(car_data, 0x32, 1); // 0x32 = 50 bytes
                    _terminate(); // Exits the program, no further processing
                } else if (current_char < (char)0xDE) { // Less than -0x22 (0xDE)
                    if (current_char == (char)0xB0) { // -0x50
                        validatePtr(car_data);
                        if (car_data[0xd] == (char)0xF8) { // -8
                            raiseAlarm();
                        }
                        // Original flow merges with -0x60 logic after specific check
                        validatePtr(car_data);
                        if (car_data[8] == 0x10) {
                            car_data[0xc] = *current_input_ptr;
                        }
                        i++;
                    } else if (current_char == (char)0xA0) { // -0x60
                        // Original flow jumps to the same logic as -0x50 (after raiseAlarm check)
                        validatePtr(car_data);
                        if (car_data[8] == 0x10) {
                            car_data[0xc] = *current_input_ptr;
                        }
                        i++;
                    } else if (current_char < (char)0xB1) { // Less than -0x4F (0xB1)
                        if ((current_char == (char)0x80) || (current_char == (char)0x90)) { // -0x80 or -0x70
                            validatePtr(car_data);
                            if ((car_data[8] == 0x10) && (car_data[0xc] == (char)0xB0)) { // -0x50
                                handleTrunk(car_data, current_input_ptr);
                            }
                            i += 2; // Consumes current char + next char (param_2[1])
                        } else {
                            i++; // Unknown character in this range, consume and move on
                        }
                    } else {
                        i++; // Unknown character in this range, consume and move on
                    }
                } else {
                    i++; // Unknown character in this range, consume and move on
                }
            } else { // current_char >= '1' (This block corresponds to the complex inner `while` loop)
                // This nested loop continues parsing characters from the input buffer
                // until its internal break conditions are met or the buffer ends.
                bool inner_parse_active = true;
                while (inner_parse_active && i < bytes_read_count) {
                    char inner_char = input_buffer[i];
                    char *inner_ptr = &input_buffer[i];

                    // Break condition for the inner parsing loop (from original code)
                    // `((iVar2 == 0) || (local_20[7] == '\x10')) || (local_54 <= local_10)`
                    // `iVar2 = validatePtr()` is now `validateStatus()`.
                    // The `(local_54 <= local_10)` (car_data_addr <= current_input_ptr_addr) is an artifact and removed.
                    if (!validateStatus() || car_data[7] == 0x10) {
                        inner_parse_active = false;
                        break; // Break out of this inner while loop
                    }

                    // Process inner_char based on its value
                    if (inner_char < 0x76) {
                        if (inner_char < 0x70) {
                            if (inner_char < 0x11) {
                                if (inner_char < (char)0xD0) { // < -0x30
                                    if (inner_char == (char)0xC0) { // -0x40
                                        validatePtr(car_data);
                                        if (car_data[0xb] != (char)0xC0) {
                                            car_data[0xb] = (char)0xC0;
                                        }
                                    } else if ((inner_char == (char)0x81) || (inner_char == (char)0x91)) { // -0x7F or -0x6F
                                        validatePtr(car_data);
                                        if (car_data[5] != (char)0xff) {
                                            int val_change = 1;
                                            validatePtr(inner_ptr);
                                            if (*inner_ptr == (char)0x91) { // -0x6F
                                                val_change = -1;
                                            }
                                            char c_val = inner_ptr[1];
                                            i += 1; // Consume current char, then c_val is next
                                            short_t short_c_val = (short_t)c_val;

                                            if (val_change == -1) {
                                                if ((short_t)(ushort)car_data[6] < short_c_val) {
                                                    car_data[6] = 0;
                                                } else {
                                                    car_data[6] -= c_val;
                                                }
                                            } else {
                                                car_data[6] += c_val;
                                            }
                                        }
                                    }
                                } else { // >= -0x30
                                    switch ((int)inner_char) {
                                        case 4:
                                            validatePtr(car_data);
                                            validatePtr(inner_ptr);
                                            if (0x30 < car_data[8]) {
                                                car_data[5] = 0x04;
                                                updateBTMetadata(car_data, inner_ptr);
                                            }
                                            i += 1 + inner_ptr[1]; // Consume current char + length from inner_ptr[1]
                                            break;
                                        case 0x10: // ASCII 16
                                            validatePtr(car_data);
                                            if (car_data[8] == 0x10) {
                                                car_data[7] = 0x10;
                                            }
                                            break;
                                        case (int)0xD0: // -0x30
                                            validatePtr(car_data);
                                            if (car_data[0xb] != (char)0xD0) {
                                                car_data[0xb] = (char)0xD0;
                                            }
                                            break;
                                        case (int)0xE0: // -0x20
                                            validatePtr(car_data);
                                            if (car_data[10] != (char)0xE0) {
                                                car_data[10] = (char)0xE0;
                                            }
                                            break;
                                        case (int)0xF0: // -0x10
                                            validatePtr(car_data);
                                            if (car_data[10] != (char)0xF0) {
                                                car_data[10] = (char)0xF0;
                                            }
                                            break;
                                        case -1: // 0xFF
                                            car_data[6] = 0;
                                            // Fallthrough to cases 1,2,3,5
                                        case 1:
                                        case 2:
                                        case 3:
                                        case 5:
                                            validatePtr(car_data);
                                            validatePtr(inner_ptr);
                                            if (0x40 < car_data[8]) {
                                                car_data[5] = *inner_ptr;
                                            }
                                            break;
                                    }
                                }
                            }
                        } else { // >= 0x70
                            switch ((int)inner_char) {
                                case 0x70: // 'p'
                                    validatePtr(car_data);
                                    if (car_data[9] == 'u') {
                                        car_data[0]++;
                                        *(ushort *)(car_data + 3) += 0x19;
                                    } else {
                                        *(ushort *)(car_data + 3) += 0x32;
                                    }
                                    break;
                                case 0x71: // 'q'
                                    validatePtr(car_data);
                                    car_data[0]--;
                                    *(ushort *)(car_data + 3) -= 0x19;
                                    break;
                                case 0x72: // 'r'
                                    validatePtr(car_data);
                                    if (car_data[9] == 't') {
                                        handleShift(car_data, 'r');
                                    }
                                    break;
                                case 0x73: // 's'
                                    validatePtr(car_data);
                                    if (car_data[9] == 't') {
                                        handleShift(car_data, 's'); // 's' is not 'l', but it's the char passed
                                    }
                                    break;
                                case 0x74: // 't'
                                case 0x75: // 'u'
                                    validatePtr(inner_ptr);
                                    validatePtr(car_data);
                                    car_data[9] = *inner_ptr;
                                    break;
                            }
                        }
                    }
                    i++; // Default increment for current_char if not handled by a multi-byte command
                } // End of inner while loop
            } // End of else (current_char >= '1' handling)
        } // End of while (i < bytes_read_count)
    } // End of outer while(true) serviceLoop
}