#include <stdint.h>  // For uint8_t, uint32_t, uintptr_t
#include <stdbool.h> // For bool
#include <stdio.h>   // For printf (used in dummy SendResp and main)
#include <stddef.h>  // For NULL

// Define a structure that matches the 8-byte device entry layout
// assuming a 32-bit pointer stored as a uint32_t.
// This ensures compatibility with the original memory access patterns
// even when compiled on a 64-bit system, by explicitly using a 32-bit integer
// to store the pointer value.
typedef struct {
    uint8_t type;
    uint8_t _padding[3]; // Padding to ensure data_ptr_val starts at offset 4
    uint32_t data_ptr_val; // Stores the pointer value as a 32-bit integer
} DeviceEntry;

// Define maximum number of devices and maximum alarm sources (for inner loops)
#define MAX_DEVICES 10        // Example: The number of DeviceEntry elements in the Devices array
#define MAX_ALARM_SOURCES 256 // The original loop condition `for (i = 0; -1 < (char)i; i++)`
                              // iterates 256 times for a uint8_t 'i' (0 to 255).

// Global array of devices
DeviceEntry Devices[MAX_DEVICES];

// Dummy SendResp function to make the code compilable
void SendResp(int code, uint32_t data) {
    printf("SendResp called with code: %d, data: 0x%08X\n", code, data);
}

// Function: IsDeviceInAlarm
// Returns 1 if the device specified by param_1 is in an alarm state, 0 otherwise.
int IsDeviceInAlarm(uint8_t param_1) {
    // Basic bounds check to prevent accessing out-of-bounds memory
    if (param_1 >= MAX_DEVICES) {
        return 0;
    }

    uint8_t device_type = Devices[param_1].type;
    // Cast the stored 32-bit integer back to a pointer type for access
    char *ptr_data = (char*)(uintptr_t)Devices[param_1].data_ptr_val;

    // A robust implementation might check `if (!ptr_data)` here,
    // but the original snippet does not, implying valid pointers are expected.

    switch (device_type) {
        case 6:
            // Alarm if the first byte pointed to is 1
            return (*ptr_data == 1);
        case 5:
            // Alarm if the second byte pointed to is less than the first byte
            // Assumes ptr_data points to at least two bytes.
            return (ptr_data[1] < *ptr_data);
        case 3: {
            // Assumes ptr_data points to at least two bytes.
            char first_byte = *ptr_data;
            char second_byte = ptr_data[1];
            if (first_byte == 0) {
                // If first byte is 0, alarm if second byte is not 0
                return (second_byte != 0);
            }
            // If first byte is not 0, alarm if first byte is 1 AND second byte is 0
            return (first_byte == 1 && second_byte == 0);
        }
        case 4:
            // Alarm if the first byte pointed to is 1
            return (*ptr_data == 1);
        default:
            // For all other device types (0, 1, 2, and anything >= 7), not in alarm
            return 0;
    }
}

// Function: InvertAlarmState
// Inverts the alarm state (first byte of data_ptr_val) for devices of type 7
// that are linked to the device specified by param_1.
void InvertAlarmState(char param_1) {
    for (uint8_t i = 0; i < MAX_DEVICES; ++i) {
        if (Devices[i].type == 7) {
            char *alarm_data = (char*)(uintptr_t)Devices[i].data_ptr_val;
            if (alarm_data != NULL) {
                // Loop through potential linked alarm sources, starting from offset 2
                for (uint8_t j = 0; j < MAX_ALARM_SOURCES; ++j) {
                    if (param_1 == alarm_data[j + 2]) {
                        // Toggle the first byte (alarm_data[0]) between 0 and 1
                        *alarm_data = (*alarm_data == 0);
                        // Assuming only one match per device for inversion is intended
                        break;
                    }
                }
            }
        }
    }
}

// Function: ProcessAlarms
// Processes alarm states for devices of type 7.
void ProcessAlarms(void) {
    for (uint8_t i = 0; i < MAX_DEVICES; ++i) {
        if (Devices[i].type == 7) {
            char *alarm_data = (char*)(uintptr_t)Devices[i].data_ptr_val;
            if (alarm_data != NULL) {
                uint8_t initial_alarm_state_byte1 = alarm_data[1]; // Store original state of alarm_data[1]
                
                if (*alarm_data == 1) { // If alarm_data[0] is 1 (master alarm control active)
                    bool any_linked_device_in_alarm = false;
                    // Check if any linked device is currently in alarm
                    for (uint8_t j = 0; j < MAX_ALARM_SOURCES; ++j) {
                        if (IsDeviceInAlarm(alarm_data[j + 2]) != 0) {
                            any_linked_device_in_alarm = true;
                            // Continue checking all sources, do not break early
                        }
                    }
                    
                    // If alarm_data[1] was 0 and a linked device is now in alarm
                    if (initial_alarm_state_byte1 == 0 && any_linked_device_in_alarm) {
                        alarm_data[1] = 1; // Set alarm_data[1] to 1 (alarm triggered)
                        SendResp(10, (uint32_t)i * sizeof(DeviceEntry) + 0x18262);
                    }
                } else { // If alarm_data[0] is 0 (master alarm control inactive)
                    alarm_data[1] = 0; // Reset alarm_data[1] to 0 (no alarm triggered)
                    // If alarm_data[1] was previously 1 (alarm was active)
                    if (initial_alarm_state_byte1 == 1) {
                        SendResp(11, (uint32_t)i * sizeof(DeviceEntry) + 0x18262);
                    }
                }
            }
        }
    }
}

// Main function for testing the provided code
int main() {
    // --- Setup Dummy Data for Testing ---
    // Allocate static memory for alarm data for various devices
    static char alarm_data_0[MAX_ALARM_SOURCES + 2]; // For Device 0 (type 7)
    static char alarm_data_1[1];                     // For Device 1 (type 6)
    static char alarm_data_2[2];                     // For Device 2 (type 3)
    static char alarm_data_3[MAX_ALARM_SOURCES + 2]; // For Device 3 (type 7)

    // Initialize specific alarm data arrays
    alarm_data_0[0] = 1; // Device 0: Master alarm control active
    alarm_data_0[1] = 0; // Device 0: Initial state of triggered alarm (not yet triggered)
    alarm_data_0[2] = 1; // Device 0: Linked to Device 1
    alarm_data_0[3] = 2; // Device 0: Linked to Device 2
    alarm_data_0[4] = 99; // Device 0: Linked to a non-existent device (will not trigger)

    alarm_data_1[0] = 1; // Device 1 (type 6): Is in alarm

    alarm_data_2[0] = 0; // Device 2 (type 3): First byte
    alarm_data_2[1] = 1; // Device 2 (type 3): Second byte (causes alarm per IsDeviceInAlarm logic)

    alarm_data_3[0] = 1; // Device 3: Master alarm control active
    alarm_data_3[1] = 1; // Device 3: Already in triggered state
    alarm_data_3[2] = 1; // Device 3: Linked to Device 1
    alarm_data_3[3] = 2; // Device 3: Linked to Device 2

    // Initialize the Devices global array
    for (int i = 0; i < MAX_DEVICES; ++i) {
        Devices[i].type = 0; // Default type
        Devices[i].data_ptr_val = 0; // Default NULL pointer
    }

    Devices[0].type = 7; // Alarm controller
    Devices[0].data_ptr_val = (uint32_t)(uintptr_t)alarm_data_0;

    Devices[1].type = 6; // Device type 6
    Devices[1].data_ptr_val = (uint32_t)(uintptr_t)alarm_data_1;

    Devices[2].type = 3; // Device type 3
    Devices[2].data_ptr_val = (uint32_t)(uintptr_t)alarm_data_2;

    Devices[3].type = 7; // Another alarm controller
    Devices[3].data_ptr_val = (uint32_t)(uintptr_t)alarm_data_3;

    // --- Test Execution ---
    printf("--- Initial State ---\n");
    printf("Device 0 (type %d): alarm_data_0[0]=%d, alarm_data_0[1]=%d\n", Devices[0].type, alarm_data_0[0], alarm_data_0[1]);
    printf("Device 1 (type %d): alarm_data_1[0]=%d\n", Devices[1].type, alarm_data_1[0]);
    printf("Device 2 (type %d): alarm_data_2[0]=%d, alarm_data_2[1]=%d\n", Devices[2].type, alarm_data_2[0], alarm_data_2[1]);
    printf("Device 3 (type %d): alarm_data_3[0]=%d, alarm_data_3[1]=%d\n", Devices[3].type, alarm_data_3[0], alarm_data_3[1]);

    printf("\n--- Calling ProcessAlarms first time ---\n");
    ProcessAlarms();

    printf("\n--- State After First ProcessAlarms ---\n");
    printf("Device 0 (type %d): alarm_data_0[0]=%d, alarm_data_0[1]=%d (Expected: 1,1 - Device 1 and 2 are in alarm)\n", Devices[0].type, alarm_data_0[0], alarm_data_0[1]);
    printf("Device 1 (type %d): alarm_data_1[0]=%d\n", Devices[1].type, alarm_data_1[0]);
    printf("Device 2 (type %d): alarm_data_2[0]=%d, alarm_data_2[1]=%d\n", Devices[2].type, alarm_data_2[0], alarm_data_2[1]);
    printf("Device 3 (type %d): alarm_data_3[0]=%d, alarm_data_3[1]=%d (Expected: 1,1 - already triggered)\n", Devices[3].type, alarm_data_3[0], alarm_data_3[1]);

    printf("\n--- Calling InvertAlarmState for linked device ID 1 ---\n");
    InvertAlarmState(1); // This will toggle alarm_data[0] for Device 0 and Device 3

    printf("\n--- State After InvertAlarmState(1) ---\n");
    printf("Device 0 (type %d): alarm_data_0[0]=%d, alarm_data_0[1]=%d (Expected: 0,1 - toggled from 1 to 0)\n", Devices[0].type, alarm_data_0[0], alarm_data_0[1]);
    printf("Device 1 (type %d): alarm_data_1[0]=%d\n", Devices[1].type, alarm_data_1[0]);
    printf("Device 2 (type %d): alarm_data_2[0]=%d, alarm_data_2[1]=%d\n", Devices[2].type, alarm_data_2[0], alarm_data_2[1]);
    printf("Device 3 (type %d): alarm_data_3[0]=%d, alarm_data_3[1]=%d (Expected: 0,1 - toggled from 1 to 0)\n", Devices[3].type, alarm_data_3[0], alarm_data_3[1]);

    printf("\n--- Calling ProcessAlarms second time ---\n");
    ProcessAlarms();

    printf("\n--- State After Second ProcessAlarms ---\n");
    printf("Device 0 (type %d): alarm_data_0[0]=%d, alarm_data_0[1]=%d (Expected: 0,0 - master control now off, so alarm_data[1] resets)\n", Devices[0].type, alarm_data_0[0], alarm_data_0[1]);
    printf("Device 1 (type %d): alarm_data_1[0]=%d\n", Devices[1].type, alarm_data_1[0]);
    printf("Device 2 (type %d): alarm_data_2[0]=%d, alarm_data_2[1]=%d\n", Devices[2].type, alarm_data_2[0], alarm_data_2[1]);
    printf("Device 3 (type %d): alarm_data_3[0]=%d, alarm_data_3[1]=%d (Expected: 0,0 - master control now off, so alarm_data[1] resets)\n", Devices[3].type, alarm_data_3[0], alarm_data_3[1]);

    return 0;
}