#include <stddef.h> // For size_t
#include <unistd.h> // For ssize_t (though not strictly necessary for mocks)

// --- Mock Function Declarations ---
// These functions are not part of the original snippet but are called by it.
// They are provided as minimal mocks to allow the code to compile.
// In a real application, these would be replaced by actual implementations
// (e.g., using socket functions like `send` and `recv` for `transmit` and `receive`).

// Mock for a receive function, similar to `recv` or `read`.
// Expects: `fd` (file descriptor/socket), `buf` (buffer to fill), `len` (max bytes to read), `bytes_transferred` (output: actual bytes read).
// Returns: 0 on success, non-zero on error.
int receive(int fd, void *buf, size_t len, int *bytes_transferred) {
    if (len == 0) {
        *bytes_transferred = 0;
        return 0;
    }
    if (buf == NULL || bytes_transferred == NULL) {
        return -1; // Invalid arguments
    }
    // Simulate reading all requested bytes for success
    *bytes_transferred = len;
    // For testing/mocking purposes, you might want to fill `buf` with specific data.
    // Example: if command byte (len=1), set it to 0 for `AddUser` case
    if (len == 1) {
        *(unsigned char*)buf = 0;
    }
    return 0; // Success
}

// Mock for a transmit function, similar to `send` or `write`.
// Expects: `fd` (file descriptor/socket), `buf` (buffer to send), `len` (bytes to send), `bytes_transferred` (output: actual bytes sent).
// Returns: 0 on success, non-zero on error.
int transmit(int fd, const void *buf, size_t len, int *bytes_transferred) {
    if (len == 0) {
        *bytes_transferred = 0;
        return 0;
    }
    if (buf == NULL || bytes_transferred == NULL) {
        return -1; // Invalid arguments
    }
    // Simulate sending all requested bytes for success
    *bytes_transferred = len;
    // In a real scenario, you might log or process the data being sent.
    return 0; // Success
}

// Mock functions for various operations mentioned in RecvCommand
char AddUser(void) { return 1; } // Returns 1 for success, 0 for failure
char DelUser(void) { return 1; } // Returns 1 for success, 0 for failure
signed char FindAvailableDevice(void) { return 1; } // Returns a device index (e.g., >0 for success), -1 for failure
short NewGenericDevice(unsigned char type, signed char index) { (void)type; (void)index; return 100; } // Returns device ID, 0 for failure
short NewContact(signed char index) { (void)index; return 101; }
short NewMotion(signed char index) { (void)index; return 102; }
short NewHeat(signed char index) { (void)index; return 103; }
short NewSmoke(signed char index) { (void)index; return 104; }
char DeleteDevice(short device_id) { (void)device_id; return 1; }
char GrantAccess(short device_id, unsigned char access_byte) { (void)device_id; (void)access_byte; return 1; }
char UpdateDevice(short device_id) { (void)device_id; return 1; }
char AddDeviceToAlarm(short device_id, unsigned short alarm_code) { (void)device_id; (void)alarm_code; return 1; }
char ListValidAlarmCodes(short device_id) { (void)device_id; return 1; }

// --- Fixed Functions from Snippet ---

// Function: ReadBytes
unsigned int ReadBytes(void *buf, size_t size) {
    size_t total_read = 0;
    int bytes_this_call;
    int ret;

    while (total_read < size) {
        ret = receive(0, (char *)buf + total_read, size - total_read, &bytes_this_call);
        if (ret != 0) { // receive failed
            return 0;
        }
        if (bytes_this_call == 0) { // EOF or no data read, but not an error from receive
            return 0; // Treat as failure to read 'size' bytes
        }
        total_read += bytes_this_call;
    }
    return total_read; // If loop finishes, total_read == size
}

// Function: SendBytes
unsigned int SendBytes(const void *buf, size_t size) {
    size_t total_sent = 0;
    int bytes_this_call;
    int ret;

    while (total_sent < size) {
        ret = transmit(1, (char *)buf + total_sent, size - total_sent, &bytes_this_call);
        if (ret != 0) { // transmit failed
            return 0;
        }
        if (bytes_this_call == 0) { // No bytes sent, but not an error from transmit
            return 0; // Treat as failure to send 'size' bytes
        }
        total_sent += bytes_this_call;
    }
    return total_sent;
}

// Function: SendResp
unsigned int SendResp(unsigned char response_code, const void *data_ptr) {
    // Send the single byte response_code
    if (SendBytes(&response_code, 1) == 0) {
        return 0; // Failed to send response_code
    }

    // Send the 2-byte data
    if (data_ptr == NULL) {
        // If data_ptr is NULL, send two zero bytes (0x0000)
        short zero_data = 0; // Use short to ensure 2 bytes
        if (SendBytes(&zero_data, 2) == 0) {
            return 0; // Failed to send zero data
        }
    } else {
        // If data_ptr is not NULL, send 2 bytes from the pointed location
        if (SendBytes(data_ptr, 2) == 0) {
            return 0; // Failed to send actual data
        }
    }
    return 1; // Success
}

// Function: RecvCommand
unsigned int RecvCommand(void) {
    unsigned char command_byte;
    // Read the command byte
    if (ReadBytes(&command_byte, 1) == 0) {
        return 0;
    }

    char success_flag;
    signed char device_index;
    short device_id;
    unsigned char grant_access_byte;
    unsigned short alarm_code;
    unsigned char device_type_val; // For case 2, only 1 byte is read into local_30[0]

    switch (command_byte) {
        case 0: // AddUser
            success_flag = AddUser();
            if (success_flag == 0) { // '\0'
                if (SendResp(3, NULL) == 0) return 0; // Error response, return 0 if SendResp fails
                return 1; // Command processed, reported failure
            }
            if (SendResp(2, NULL) == 0) return 0; // Success response, return 0 if SendResp fails
            break;
        case 1: // DelUser
            success_flag = DelUser();
            if (success_flag == 0) { // '\0'
                if (SendResp(3, NULL) == 0) return 0; // Error response
                return 1; // Command processed, reported failure
            }
            if (SendResp(4, NULL) == 0) return 0; // Success response
            break;
        case 2: // NewDevice
            if (ReadBytes(&device_type_val, 1) == 0) { // Read 1 byte for device type
                return 0;
            }
            device_index = FindAvailableDevice();
            if (device_index < 0) {
                if (SendResp(0, NULL) == 0) return 0; // Error response
                return 0; // Command failed to process
            }

            device_id = 0; // Initialize to 0, indicating failure
            switch (device_type_val) {
                case 1:
                case 2:
                case 7:
                    device_id = NewGenericDevice(device_type_val, device_index);
                    break;
                case 3:
                    device_id = NewContact(device_index);
                    break;
                case 4:
                    device_id = NewMotion(device_index);
                    break;
                case 5:
                    device_id = NewHeat(device_index);
                    break;
                case 6:
                    device_id = NewSmoke(device_index);
                    break;
                default:
                    if (SendResp(5, NULL) == 0) return 0; // Unknown device type error
                    return 0; // Command failed to process
            }

            if (device_id == 0) { // If device creation failed
                if (SendResp(5, NULL) == 0) return 0; // Device creation failure response
                return 0; // Command failed to process
            }
            if (SendResp(3, &device_id) == 0) return 0; // Success, send device_id
            break;
        case 3: // DeleteDevice
            if (ReadBytes(&device_id, 2) == 0) {
                return 0;
            }
            success_flag = DeleteDevice(device_id);
            if (success_flag == 0) { // '\0'
                if (SendResp(3, NULL) == 0) return 0; // Error response
                return 1; // Command processed, reported failure
            }
            if (SendResp(6, NULL) == 0) return 0; // Success response
            break;
        case 4: // Placeholder/No-op
            if (SendResp(3, NULL) == 0) return 0;
            return 0; // Original returns 0 here
        case 5: // GrantAccess
            if (ReadBytes(&device_id, 2) == 0) {
                return 0;
            }
            if (ReadBytes(&grant_access_byte, 1) == 0) {
                return 0;
            }
            success_flag = GrantAccess(device_id, grant_access_byte);
            if (success_flag == 0) { // '\0'
                if (SendResp(3, NULL) == 0) return 0; // Error response
                return 1; // Command processed, reported failure
            }
            if (SendResp(7, NULL) == 0) return 0; // Success response
            break;
        case 6: // UpdateDevice
            if (ReadBytes(&device_id, 2) == 0) {
                return 0;
            }
            success_flag = UpdateDevice(device_id);
            if (success_flag == 0) { // '\0'
                if (SendResp(3, NULL) == 0) return 0; // Error response
                return 1; // Command processed, reported failure
            }
            if (SendResp(8, NULL) == 0) return 0; // Success response
            break;
        case 7: // AddDeviceToAlarm
            if (ReadBytes(&device_id, 2) == 0) {
                return 0;
            }
            if (ReadBytes(&alarm_code, 2) == 0) {
                return 0;
            }
            success_flag = AddDeviceToAlarm(device_id, alarm_code);
            if (success_flag == 0) { // '\0'
                if (SendResp(3, NULL) == 0) return 0; // Error response
                return 0; // Command failed to process
            }
            if (SendResp(9, NULL) == 0) return 0; // Success response
            break;
        case 8: // ListValidAlarmCodes
            if (ReadBytes(&device_id, 2) == 0) {
                return 0;
            }
            success_flag = ListValidAlarmCodes(device_id);
            if (success_flag == 0) { // '\0'
                if (SendResp(0, NULL) == 0) return 0; // Error response
                return 0; // Command failed to process
            }
            // No success response explicit in original, just breaks.
            break;
        default:
            if (SendResp(0, NULL) == 0) return 0; // Unknown command error
            return 1; // Original returns 1 for default case after sending error resp
    }
    return 1; // Default success return for cases that break out of switch and SendResp succeeded
}