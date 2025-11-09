#include <stdint.h>   // For uint16_t, uint32_t, uint8_t
#include <string.h>   // For memset, memcpy
#include <stdlib.h>   // For exit, EXIT_FAILURE
#include <stdbool.h>  // For bool
#include <stddef.h>   // For size_t

// --- External function declarations (assuming these exist in other files or are linked) ---
// These types are inferred from usage; adjust if actual definitions differ.
// param_1 in ui_main and get_command is likely a state pointer or socket descriptor.
// We'll use 'int state_ptr' for consistency with its usage (e.g., as first arg to 'send').
uint32_t init_state(void);
int receive_bytes(void *buffer, size_t size); // Returns -1 on error, bytes received otherwise
void _terminate(uint32_t status); // Original uses 0xffffffff
int send(int socket_fd, const void *buffer, size_t length, int flags);
uint32_t check_val(const void *buffer, uint16_t length);
void power_off(int state_ptr);
void power_on(int state_ptr);
int set_temp(int state_ptr, uint16_t temp); // Returns 2 on specific error
int add_sensor(int state_ptr, uint16_t sensor_id, uint32_t val1, uint32_t val2, uint32_t val3); // Returns 2 or 3 on specific errors
int remove_sensor(int state_ptr, uint16_t sensor_id); // Returns 1 on specific error
void smoke_off(int state_ptr);
void smoke_on(int state_ptr);
int add_steps(int state_ptr, uint16_t steps_val); // Returns 1, 2, or 3 on specific errors
void get_program(int state_ptr, size_t *count, void *buffer); // Fills buffer with program data and count
int get_status(int state_ptr, void *buffer); // Fills buffer with status data
int simulate_seconds(int state_ptr, uint16_t seconds); // Returns 0 for success, 2 for specific state change
void new_state(int state_ptr); // Updates state_ptr
uint32_t validate_fw(int state_ptr);
void get_sensors(int state_ptr, void *buffer); // Fills buffer with sensor data
int set_ambient_temp(int state_ptr, int32_t temp); // Returns 2 on specific error
void exit_normal(void);

// --- Helper functions from snippet ---

// get_bytes function
void get_bytes(void *buffer, size_t size) {
  int bytes_received = receive_bytes(buffer, size);
  if (bytes_received == -1) {
    _terminate(0xffffffff); // Original uses 0xffffffff, assuming this is an error code.
  }
}

// prime_buf function
// Initializes the buffer with a default response header:
// Magic (0x5aa5) at offset 0, Payload Length (0x0008) at offset 2.
// The remaining 8 bytes of the 12-byte default response are zeroed.
// This sets up a 12-byte response packet (4-byte header + 8-byte payload).
void prime_buf(void *buffer) {
  memset(buffer, 0, 0x108); // Zero out the entire buffer (max size)
  *(uint16_t*)buffer = 0x5aa5; // Set magic value (bytes 0-1)
  *(uint16_t*)(buffer + 2) = 0x0008; // Set default payload length to 8 bytes (bytes 2-3)
                                     // This implies a total packet size of 4 (header) + 8 (payload) = 12 bytes.
}

// get_short function
uint16_t get_short(uint16_t *param_1) {
  return *param_1;
}

// get_int function
uint32_t get_int(uint32_t *param_1) {
  return *param_1;
}

// get_signed_int function
int32_t get_signed_int(int32_t *param_1) {
  return *param_1;
}

// ui_main function
void ui_main(void) {
  uint32_t state = init_state(); // Assuming init_state returns the state/socket descriptor
  do {
    get_command(state);
  } while(true);
}

// get_command function
// param_1 is assumed to be the socket descriptor or state pointer.
int get_command(int state_ptr) {
  #define COMMAND_BUFFER_SIZE 0x108
  uint8_t command_response_buffer[COMMAND_BUFFER_SIZE]; // Main buffer for incoming command & outgoing response

  uint16_t magic_val;
  uint16_t data_len;
  uint32_t received_checksum;
  uint32_t calculated_checksum;
  uint16_t command_id;
  int send_flags = 0x124d0; // Fixed flags for send function

  // Variables for common response handling
  size_t total_response_len = 0; // Total bytes to send, including header
  bool send_custom_response = false; // Flag for cases with unique send logic

  // Initialize the main buffer to all zeros
  memset(command_response_buffer, 0, COMMAND_BUFFER_SIZE);

  // 1. Read Magic (2 bytes)
  get_bytes(command_response_buffer, 2);
  magic_val = get_short((uint16_t*)command_response_buffer);

  if (magic_val != 0x5aa5) {
    prime_buf(command_response_buffer); // Prepare default error response
    send(state_ptr, command_response_buffer, 0xC, send_flags); // Send 12-byte error packet
    return 1;
  }

  // 2. Read Payload Length (2 bytes)
  get_bytes(command_response_buffer + 2, 2);
  data_len = get_short((uint16_t*)(command_response_buffer + 2));

  if (data_len > 0x100) { // Max payload length check (0x100 bytes)
    prime_buf(command_response_buffer); // Prepare default error response
    send(state_ptr, command_response_buffer, 0xC, send_flags); // Send 12-byte error packet
    return 1;
  }

  // 3. Read Command Data and Checksum
  // The command data starts at offset 4, followed by a 4-byte checksum.
  get_bytes(command_response_buffer + 4, data_len + 4);

  // Extract checksum from the end of the received data
  received_checksum = get_int((uint32_t*)(command_response_buffer + 4 + data_len));
  // Calculate checksum over the data part only
  calculated_checksum = check_val(command_response_buffer + 4, data_len);

  if (received_checksum != calculated_checksum) {
    prime_buf(command_response_buffer); // Prepare default error response
    send(state_ptr, command_response_buffer, 0xC, send_flags); // Send 12-byte error packet
    return 1;
  }

  // Extract command ID (first 2 bytes of payload)
  command_id = get_short((uint16_t*)(command_response_buffer + 4));

  // Default response setup for most cases
  prime_buf(command_response_buffer); // Reset buffer for new response header
  // Set default success status for response (byte 4 and byte 6 of payload)
  // *(uint8_t*)(command_response_buffer + 4) = 1; // Success status byte 1
  // *(uint8_t*)(command_response_buffer + 6) = 0; // Success status code (default)
  // Note: prime_buf already zeroes out bytes 4-11, so default status is implicitly 0.
  // Specific status codes will overwrite byte 6.

  // Process command based on ID
  if (command_id < 0xd) {
    switch(command_id) {
      case 1: { // Power On/Off
        uint16_t power_cmd = get_short((uint16_t*)(command_response_buffer + 4 + 2));
        if (power_cmd == 0) {
          power_off(state_ptr);
        } else if (power_cmd == 1) {
          power_on(state_ptr);
        }
        total_response_len = 0xC; // Default 12-byte response
      } break;
      case 2: { // Set Temperature
        uint16_t temp_val = get_short((uint16_t*)(command_response_buffer + 4 + 2));
        int result = set_temp(state_ptr, temp_val);
        if (result == 2) {
          *(uint8_t*)(command_response_buffer + 6) = 1; // Error code 1
        }
        total_response_len = 0xC; // Default 12-byte response
      } break;
      case 3: { // Add Sensor
        uint16_t sensor_id = get_short((uint16_t*)(command_response_buffer + 4 + 2));
        uint32_t val1 = get_int((uint32_t*)(command_response_buffer + 4 + 4));
        uint32_t val2 = get_int((uint32_t*)(command_response_buffer + 4 + 8));
        uint32_t val3 = get_int((uint32_t*)(command_response_buffer + 4 + 12));
        int result = add_sensor(state_ptr, sensor_id, val1, val2, val3);
        if (result == 2) {
          *(uint8_t*)(command_response_buffer + 6) = 7; // Error code 7
        } else if (result == 3) {
          *(uint8_t*)(command_response_buffer + 6) = 8; // Error code 8
        }
        total_response_len = 0xC; // Default 12-byte response
      } break;
      case 4: { // Remove Sensor
        uint16_t sensor_id = get_short((uint16_t*)(command_response_buffer + 4 + 2));
        int result = remove_sensor(state_ptr, sensor_id);
        if (result == 1) {
          *(uint8_t*)(command_response_buffer + 6) = 6; // Error code 6
        }
        total_response_len = 0xC; // Default 12-byte response
      } break;
      case 5: { // Smoke On/Off
        uint16_t smoke_cmd = get_short((uint16_t*)(command_response_buffer + 4 + 2));
        if (smoke_cmd == 0) {
          smoke_off(state_ptr);
        } else if (smoke_cmd == 1) {
          smoke_on(state_ptr);
        }
        total_response_len = 0xC; // Default 12-byte response
      } break;
      case 6: { // Add Steps
        uint16_t steps_val = get_short((uint16_t*)(command_response_buffer + 4 + 2));
        int result = add_steps(state_ptr, steps_val);
        if (result == 3) {
          *(uint8_t*)(command_response_buffer + 6) = 3; // Error code 3
        } else if (result == 2) {
          *(uint8_t*)(command_response_buffer + 6) = 2; // Error code 2
        } else if (result == 1) {
          *(uint8_t*)(command_response_buffer + 6) = 1; // Error code 1
        }
        total_response_len = 0xC; // Default 12-byte response
      } break;
      case 7: { // Get Program
        send_custom_response = true; // This case has its own send logic
        *(uint8_t*)(command_response_buffer + 4) = 1; // Status byte 1
        *(uint8_t*)(command_response_buffer + 6) = 7; // Status code 7

        uint8_t program_data[0x78]; // Temporary buffer for program data
        size_t program_entry_count = 0;
        get_program(state_ptr, &program_entry_count, program_data);

        size_t program_data_bytes = program_entry_count * 0xc; // 0xc bytes per entry
        
        // Copy program_data_bytes into payload (offset 8)
        memcpy(command_response_buffer + 8, &program_data_bytes, sizeof(program_data_bytes));
        // Copy program_data into payload (offset 12)
        memcpy(command_response_buffer + 12, program_data, program_data_bytes);

        // Update header payload length and calculate total response length
        *(uint16_t*)(command_response_buffer + 2) = 4 + program_data_bytes; // Payload length: status (4) + data_len
        total_response_len = 4 + (4 + program_data_bytes); // Total: header (4) + payload
        send(state_ptr, command_response_buffer, total_response_len, send_flags);
      } break;
      case 8: { // Get Status
        send_custom_response = true; // This case has its own send logic
        *(uint8_t*)(command_response_buffer + 4) = 1; // Status byte 1
        *(uint8_t*)(command_response_buffer + 6) = 8; // Status code 8

        uint8_t status_data[24]; // Temporary buffer for status data
        get_status(state_ptr, status_data);

        uint32_t status_data_len = 0x18;
        // Copy status_data_len into payload (offset 8)
        memcpy(command_response_buffer + 8, &status_data_len, sizeof(status_data_len));
        // Copy status_data into payload (offset 12)
        memcpy(command_response_buffer + 12, status_data, status_data_len);

        // Update header payload length and calculate total response length
        *(uint16_t*)(command_response_buffer + 2) = 4 + status_data_len; // Payload length: status (4) + data_len
        total_response_len = 4 + (4 + status_data_len); // Total: header (4) + payload
        send(state_ptr, command_response_buffer, total_response_len, send_flags);
      } break;
      case 9: { // Simulate Seconds
        send_custom_response = true; // This case has its own send logic
        uint16_t seconds_to_sim = get_short((uint16_t*)(command_response_buffer + 4 + 2));
        int sim_result = simulate_seconds(state_ptr, seconds_to_sim);

        prime_buf(command_response_buffer); // Re-prime for response

        if (sim_result == 0) { // Success path
          *(uint8_t*)(command_response_buffer + 6) = 9; // Status code 9
          uint32_t val1 = *(uint32_t*)(state_ptr + 0x66c); // Data from state_ptr
          memcpy(command_response_buffer + 8, &val1, sizeof(val1)); // Copy data to payload
          
          // Update header payload length and calculate total response length
          *(uint16_t*)(command_response_buffer + 2) = 4 + sizeof(val1); // Payload: status (4) + val1 (4)
          total_response_len = 4 + (4 + sizeof(val1)); // Total: header (4) + payload
          send(state_ptr, command_response_buffer, total_response_len, send_flags);
        } else if (sim_result == 2) { // Other result path
          *(uint8_t*)(command_response_buffer + 4) = 1; // Status byte 1
          *(uint8_t*)(command_response_buffer + 6) = 0xC; // Status code 0xC

          size_t new_state_data_len = *(int*)(state_ptr + 0x670) << 2;
          void *new_state_data_ptr = (void*)(state_ptr + 8);
          uint32_t val2 = *(uint32_t*)(state_ptr + 0x664);
          uint32_t val3 = *(uint32_t*)(state_ptr + 0x650);

          new_state(state_ptr); // Call new_state

          size_t current_payload_offset = 4; // Start of payload after header
          if (*(int*)(state_ptr + 0x670) != 0) {
            memcpy(command_response_buffer + 4 + current_payload_offset, &(*(int*)(state_ptr + 0x670)), sizeof(int));
            current_payload_offset += sizeof(int);
            memcpy(command_response_buffer + 4 + current_payload_offset, new_state_data_ptr, new_state_data_len);
            current_payload_offset += new_state_data_len;
          }
          memcpy(command_response_buffer + 4 + current_payload_offset, &val2, sizeof(val2));
          current_payload_offset += sizeof(val2);
          memcpy(command_response_buffer + 4 + current_payload_offset, &val3, sizeof(val3));
          current_payload_offset += sizeof(val3);

          // Update header payload length and calculate total response length
          *(uint16_t*)(command_response_buffer + 2) = (uint16_t)current_payload_offset; // Total payload length
          total_response_len = 4 + current_payload_offset; // Total: header (4) + payload
          send(state_ptr, command_response_buffer, total_response_len, send_flags);
        }
      } break;
      case 10: { // Validate Firmware
        send_custom_response = true; // This case has its own send logic
        uint32_t fw_validation_result = validate_fw(state_ptr);
        prime_buf(command_response_buffer); // Re-prime for response
        *(uint8_t*)(command_response_buffer + 4) = 1; // Status byte 1
        *(uint8_t*)(command_response_buffer + 6) = 10; // Status code 10

        uint32_t data_size = 4; // This value is copied as part of the response
        memcpy(command_response_buffer + 8, &data_size, sizeof(data_size));
        memcpy(command_response_buffer + 12, &fw_validation_result, sizeof(fw_validation_result));
        
        // Update header payload length and calculate total response length
        *(uint16_t*)(command_response_buffer + 2) = 4 + sizeof(data_size) + sizeof(fw_validation_result); // Payload: status (4) + data_size (4) + fw_result (4)
        total_response_len = 4 + (4 + sizeof(data_size) + sizeof(fw_validation_result)); // Total: header (4) + payload
        send(state_ptr, command_response_buffer, total_response_len, send_flags);
      } break;
      case 0xb: { // Get Sensors
        send_custom_response = true; // This case has its own send logic
        prime_buf(command_response_buffer); // Re-prime for response
        *(uint8_t*)(command_response_buffer + 4) = 1; // Status byte 1
        *(uint8_t*)(command_response_buffer + 6) = 0xb; // Status code 0xb

        size_t sensor_data_bytes = *(int*)(state_ptr + 0x654) << 4;
        uint8_t sensor_data[640]; // Temporary buffer for sensor data
        get_sensors(state_ptr, sensor_data);

        // Copy sensor_data_bytes into payload (offset 8)
        memcpy(command_response_buffer + 8, &sensor_data_bytes, sizeof(sensor_data_bytes));
        // Copy sensor_data into payload (offset 12)
        memcpy(command_response_buffer + 12, sensor_data, sensor_data_bytes);

        // Update header payload length and calculate total response length
        *(uint16_t*)(command_response_buffer + 2) = 4 + sensor_data_bytes; // Payload length: status (4) + data_len
        total_response_len = 4 + (4 + sensor_data_bytes); // Total: header (4) + payload
        send(state_ptr, command_response_buffer, total_response_len, send_flags);
      } break;
      case 0xc: { // Set Ambient Temperature
        int32_t ambient_temp = get_signed_int((int32_t*)(command_response_buffer + 4 + 2));
        int result = set_ambient_temp(state_ptr, ambient_temp);
        if (result == 2) {
          *(uint8_t*)(command_response_buffer + 6) = 1; // Error code 1
        }
        total_response_len = 0xC; // Default 12-byte response
      } break;
      default:
        // Unrecognized command within the 0-C range
        *(uint8_t*)(command_response_buffer + 6) = 5; // Error code 5 (Unrecognized command)
        total_response_len = 0xC; // Default 12-byte response
        break;
    }
  } else if (command_id == 0xff) { // Exit command
    exit_normal();
    return 0; // Command handled, no further response
  } else {
    // Command ID >= 0xd and not 0xff (unrecognized command)
    *(uint8_t*)(command_response_buffer + 6) = 5; // Error code 5 (Unrecognized command)
    total_response_len = 0xC; // Default 12-byte response
  }

  // Common response sending for cases that did not handle their own 'send'
  if (!send_custom_response) {
    // Update the payload length in the header (bytes 2-3)
    // total_response_len - 4 = payload_length
    *(uint16_t*)(command_response_buffer + 2) = (uint16_t)(total_response_len - 4);
    send(state_ptr, command_response_buffer, total_response_len, send_flags);
  }

  return 0;
}