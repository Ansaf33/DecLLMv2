#include <stdlib.h>   // For malloc, free, exit
#include <string.h>   // For bzero, memcpy
#include <stdbool.h>  // For bool
#include <stdint.h>   // For uint32_t, uint16_t, uint8_t, uintptr_t
#include <math.h>     // For round

// Declare external functions (assuming ui_main is defined elsewhere)
extern void ui_main(void);

// Global constants - guessed values for compilation based on common simulation patterns
// In a real scenario, these would be extracted from the binary's data section.
static const double DAT_00019000 = 0.0;     // Initial temperature or similar
static const double DAT_00019010 = 0.001;   // Sensor update rate multiplier (small value for gradual change)
static const double DAT_00019008 = 100.0;   // Sensor update rate divisor (makes interval more significant)
static const double DAT_00019018 = 5.0;     // Temperature threshold for heating/cooling state change hysteresis
static const double DAT_00019020 = 0.5;     // Temperature decrease rate when cooling (degrees per second)

// Sensor linked list node structure
// Assumes 32-bit addresses are stored in prev_addr/next_addr fields,
// consistent with the decompiled code's use of `int` for pointer values.
typedef struct SensorNode {
    uint32_t prev_addr; // Stores address of previous node as uint32_t
    uint32_t next_addr; // Stores address of next node as uint32_t
    uint16_t id;
    uint16_t padding_id; // Added for 4-byte alignment, as `id` is 2 bytes and next field is 4-byte aligned `type`.
    uint32_t type;
    uint32_t interval;
    double value;
} SensorNode; // Total size: 4+4+2+2+4+4+8 = 28 bytes (0x1C)

// Program step linked list node structure
// Assumes 32-bit addresses are stored in prev_addr/next_addr fields.
typedef struct StepNode {
    uint32_t prev_addr;
    uint32_t next_addr;
    uint32_t type;
    uint32_t duration_or_id; // Interpretation depends on 'type' (e.g., duration, sensor ID)
    uint32_t target_temp;
} StepNode; // Total size: 4+4+4+4+4 = 20 bytes (0x14)

// Main function
int main(void) {
  ui_main();
  return 0;
}

// Function: find_sensor
// Returns a pointer to the SensorNode if found, otherwise NULL.
// state_ptr: Pointer to the main state structure.
// sensor_id: ID of the sensor to find.
void *find_sensor(void *state_ptr, uint16_t sensor_id) {
    if (!state_ptr) return NULL;

    SensorNode *current_sensor = (SensorNode *)(uintptr_t)*(uint32_t *)((char *)state_ptr + 0x658);
    while (current_sensor != NULL) {
        if (current_sensor->id == sensor_id) {
            return current_sensor;
        }
        current_sensor = (SensorNode *)(uintptr_t)current_sensor->next_addr;
    }
    return NULL;
}

// Function: get_step
// Returns a pointer to the StepNode at the given index, otherwise NULL.
// state_ptr: Pointer to the main state structure.
// index: Zero-based index of the step to retrieve.
void *get_step(void *state_ptr, int index) {
    StepNode *current_step = (StepNode *)(uintptr_t)*(uint32_t *)((char *)state_ptr + 0x678);
    int current_index = 0;

    while (current_step != NULL && current_index < index) {
        current_step = (StepNode *)(uintptr_t)current_step->next_addr;
        current_index++;
    }
    return current_step;
}

// Function: free_program
// Frees all StepNodes in the program list and resets related state fields.
// state_ptr: Pointer to the main state structure.
void free_program(void *state_ptr) {
  if (!state_ptr) return;

  StepNode *current_step = (StepNode *)(uintptr_t)*(uint32_t *)((char *)state_ptr + 0x678);
  while (current_step != NULL) {
    StepNode *next_step = (StepNode *)(uintptr_t)current_step->next_addr;
    free(current_step);
    current_step = next_step;
  }
  *(uint32_t *)((char *)state_ptr + 0x678) = 0; // Clear program list head
  *(uint16_t *)((char *)state_ptr + 0x65c) = 0; // Clear step count
}

// Function: reset_sensors
// Resets the value of all connected sensors to the current ambient temperature.
// state_ptr: Pointer to the main state structure.
void reset_sensors(void *state_ptr) {
    if (!state_ptr) return;

    SensorNode *current_sensor = (SensorNode *)(uintptr_t)*(uint32_t *)((char *)state_ptr + 0x658);
    while (current_sensor != NULL) {
        current_sensor->value = (double)*(uint32_t *)((char *)state_ptr + 0x664); // Ambient temperature
        current_sensor = (SensorNode *)(uintptr_t)current_sensor->next_addr;
    }
}

// Function: new_state
// Resets the entire system state to its initial default values.
// state_ptr: Pointer to the main state structure.
void new_state(void *state_ptr) {
  if (!state_ptr) return;

  free_program(state_ptr);
  *(double *)((char *)state_ptr + 0x648) = DAT_00019000; // Current temperature
  *(uint32_t *)((char *)state_ptr + 0x650) = 0xaf;       // Target temperature (0xaf = 175)
  *(uint32_t *)((char *)state_ptr + 0x4) = 0;           // Power state (0 = off)
  *(uint32_t *)((char *)state_ptr + 0x660) = 0;         // Heating state (0 = off)
  *(uint16_t *)((char *)state_ptr + 0x668) = 0xffff;    // Current step index (0xffff implies no program running)
  *(uint32_t *)((char *)state_ptr + 0x0) = 0;           // Field 0
  *(uint32_t *)((char *)state_ptr + 0x654) = 0;         // Sensor count
  *(uint32_t *)((char *)state_ptr + 0x66c) = 0;         // Elapsed time
  *(uint8_t *)((char *)state_ptr + 0x674) = 0;          // Program finished flag
  *(uint32_t *)((char *)state_ptr + 0x658) = 0;         // Sensor list head
  reset_sensors(state_ptr);
}

// Function: init_state
// Allocates and initializes a new system state structure.
// Returns a pointer to the newly created state structure, or NULL on failure.
void *init_state(void) {
  char *state_ptr = (char *)malloc(0x67c);
  if (!state_ptr) return NULL;

  bzero(state_ptr, 0x67c); // Initialize all memory to zero

  // Initialize specific fields
  *(double *)(state_ptr + 0x648) = DAT_00019000; // Current temperature
  *(uint32_t *)(state_ptr + 0x654) = 0;         // Sensor count
  *(uint32_t *)(state_ptr + 0x650) = 0xaf;       // Target temperature (0xaf = 175)
  *(uint32_t *)(state_ptr + 0x4) = 0;           // Power state (0 = off)
  *(uint32_t *)(state_ptr + 0x660) = 0;         // Heating state (0 = off)
  *(uint16_t *)(state_ptr + 0x668) = 0xffff;    // Current step index (0xffff implies no program running)
  *(uint32_t *)(state_ptr + 0x0) = 0;           // Field 0
  *(uint32_t *)(state_ptr + 0x664) = 0x4b;       // Ambient temperature (0x4b = 75)
  // bzero(state_ptr + 8, 800) for history_data is covered by bzero(state_ptr, 0x67c).
  *(uint32_t *)(state_ptr + 0x66c) = 0;         // Elapsed time
  *(uint8_t *)(state_ptr + 0x674) = 0;          // Program finished flag
  *(uint32_t *)(state_ptr + 0x658) = 0;         // Sensor list head
  *(uint32_t *)(state_ptr + 0x678) = 0;         // Program step list head

  return state_ptr;
}

// Function: power_on
// Turns the system power on.
// state_ptr: Pointer to the main state structure.
// Returns true if state_ptr is NULL (an error condition), false otherwise.
bool power_on(void *state_ptr) {
  if (state_ptr != NULL) {
    *(uint32_t *)((char *)state_ptr + 0x4) = 1; // Set power_state to 1 (on)
  }
  return state_ptr == NULL;
}

// Function: power_off
// Turns the system power off and resets its state.
// state_ptr: Pointer to the main state structure.
// Returns true if state_ptr is NULL (an error condition), false otherwise.
bool power_off(void *state_ptr) {
  if (state_ptr != NULL) {
    new_state(state_ptr);
  }
  return state_ptr == NULL;
}

// Function: smoke_on
// Turns the smoke detector on.
// smoke_state_ptr: Pointer to an integer representing the smoke detector's state.
// Returns 0 on success, 1 if NULL pointer, 2 if already on.
uint32_t smoke_on(int *smoke_state_ptr) {
  if (smoke_state_ptr == NULL) return 1;
  if (*smoke_state_ptr == 1) return 2;
  *smoke_state_ptr = 1;
  return 0;
}

// Function: smoke_off
// Turns the smoke detector off.
// smoke_state_ptr: Pointer to an integer representing the smoke detector's state.
// Returns 0 on success, 1 if NULL pointer, 2 if already off.
uint32_t smoke_off(int *smoke_state_ptr) {
  if (smoke_state_ptr == NULL) return 1;
  if (*smoke_state_ptr == 0) return 2;
  *smoke_state_ptr = 0;
  return 0;
}

// Function: set_ambient_temp
// Sets the ambient temperature.
// state_ptr: Pointer to the main state structure.
// temp: The new ambient temperature.
// Returns 0 on success, 1 if NULL state_ptr, 2 if temperature is too high.
uint32_t set_ambient_temp(void *state_ptr, int temp) {
  if (state_ptr == NULL) return 1;
  if (temp < 0xb0) { // Max ambient temp 0xb0 = 176
    *(int *)((char *)state_ptr + 0x664) = temp; // Set ambient_temp
    return 0;
  }
  return 2;
}

// Function: set_temp
// Sets the target temperature.
// state_ptr: Pointer to the main state structure.
// temp: The new target temperature.
// Returns 0 on success, 1 if NULL state_ptr, 2 if temperature is out of range.
uint32_t set_temp(void *state_ptr, uint16_t temp) {
  if (state_ptr == NULL) return 1;
  if ((temp < 0xaf) || (0x15e < temp)) { // Range 0xaf (175) to 0x15e (350)
    return 2;
  }
  *(uint32_t *)((char *)state_ptr + 0x650) = temp; // Set target_temp
  return 0;
}

// Function: add_sensor
// Adds a new sensor to the system.
// state_ptr: Pointer to the main state structure.
// sensor_id: Unique ID for the new sensor.
// type: Type of the sensor.
// interval: Update interval for the sensor.
// Returns 0 on success, 1 if NULL state_ptr, 2 if sensor ID exists, 3 if max sensors reached, 4 if memory allocation fails.
uint32_t add_sensor(void *state_ptr, uint16_t sensor_id, uint32_t type, uint32_t interval) {
  if (state_ptr == NULL) return 1;
  if (find_sensor(state_ptr, sensor_id) != NULL) return 2;

  uint32_t *sensor_count_ptr = (uint32_t *)((char *)state_ptr + 0x654);
  if (*sensor_count_ptr >= 10) return 3; // Max 10 sensors

  SensorNode *new_sensor = (SensorNode *)malloc(sizeof(SensorNode));
  if (!new_sensor) return 4; // Memory allocation failed

  bzero(new_sensor, sizeof(SensorNode));
  new_sensor->id = sensor_id;
  new_sensor->type = type;
  new_sensor->interval = interval;
  new_sensor->value = (double)*(uint32_t *)((char *)state_ptr + 0x664); // Initial value from ambient_temp

  // Add to linked list
  uint32_t *head_ptr_addr = (uint32_t *)((char *)state_ptr + 0x658);
  if (*head_ptr_addr == 0) { // List is empty
    *head_ptr_addr = (uint32_t)(uintptr_t)new_sensor;
  } else {
    SensorNode *current = (SensorNode *)(uintptr_t)*head_ptr_addr;
    while (current->next_addr != 0) {
      current = (SensorNode *)(uintptr_t)current->next_addr;
    }
    current->next_addr = (uint32_t)(uintptr_t)new_sensor;
    new_sensor->prev_addr = (uint32_t)(uintptr_t)current;
  }
  
  (*sensor_count_ptr)++;
  return 0;
}

// Function: remove_sensor
// Removes a sensor from the system.
// state_ptr: Pointer to the main state structure.
// sensor_id: ID of the sensor to remove.
// Returns 0 on success, 1 if sensor not found.
uint32_t remove_sensor(void *state_ptr, uint16_t sensor_id) {
  SensorNode *sensor_to_remove = (SensorNode *)find_sensor(state_ptr, sensor_id);
  if (sensor_to_remove == NULL) return 1; // Sensor not found

  // Update linked list pointers
  if (sensor_to_remove->prev_addr == 0) { // It's the head
    *(uint32_t *)((char *)state_ptr + 0x658) = sensor_to_remove->next_addr;
  } else {
    ((SensorNode *)(uintptr_t)sensor_to_remove->prev_addr)->next_addr = sensor_to_remove->next_addr;
  }

  if (sensor_to_remove->next_addr != 0) {
    ((SensorNode *)(uintptr_t)sensor_to_remove->next_addr)->prev_addr = sensor_to_remove->prev_addr;
  }

  *(uint32_t *)((char *)state_ptr + 0x654) -= 1; // Decrement sensor count
  free(sensor_to_remove);
  return 0;
}

// Function: add_step
// Adds a single program step to the system's program list.
// state_ptr: Pointer to the main state structure.
// type: Type of the step.
// duration_or_id: Duration for time-based steps, or sensor ID for sensor-based steps.
// target_temp: Target temperature for this step.
// Returns 0 on success, 1 if memory allocation fails.
uint32_t add_step(void *state_ptr, uint32_t type, uint32_t duration_or_id, uint32_t target_temp) {
  StepNode *new_step = (StepNode *)malloc(sizeof(StepNode));
  if (!new_step) return 1; // Memory allocation failed

  bzero(new_step, sizeof(StepNode));
  new_step->type = type;
  new_step->duration_or_id = duration_or_id;
  new_step->target_temp = target_temp;

  uint32_t *head_ptr_addr = (uint32_t *)((char *)state_ptr + 0x678);
  if (*head_ptr_addr == 0) { // List is empty
    *head_ptr_addr = (uint32_t)(uintptr_t)new_step;
  } else {
    StepNode *current = (StepNode *)(uintptr_t)*head_ptr_addr;
    while (current->next_addr != 0) {
      current = (StepNode *)(uintptr_t)current->next_addr;
    }
    current->next_addr = (uint32_t)(uintptr_t)new_step;
    new_step->prev_addr = (uint32_t)(uintptr_t)current;
  }
  return 0;
}

// Function: add_steps
// Adds multiple program steps to the system. Validates steps before adding.
// state_ptr: Pointer to the main state structure.
// num_steps: Number of steps to add.
// steps_data: Pointer to an array of step data (each step 12 bytes: type, duration_or_id, target_temp).
// Returns 0 on success, 1 if target temperature out of range, 2/3/4 for other validation errors, 5 if too many steps.
uint32_t add_steps(void *state_ptr, uint32_t num_steps, const void *steps_data) {
  if (num_steps >= 0xb) return 5; // Max 10 steps (0xb = 11, so 0-10 steps)

  // Validate all steps first
  for (uint32_t i = 0; i < num_steps; i++) {
    // Each step in steps_data is 12 bytes: uint32_t type, uint32_t duration_or_id, uint32_t target_temp
    uint32_t step_type = *(uint32_t *)((char *)steps_data + i * 0xc);
    uint32_t step_duration_or_id = *(uint32_t *)((char *)steps_data + i * 0xc + 4);
    uint32_t step_target_temp = *(uint32_t *)((char *)steps_data + i * 0xc + 8);

    if (step_type == 0) { // Type 0: duration-based step (but `check_program` uses duration_or_id as sensor ID)
      // Original validation for type 0 (duration_or_id max 12)
      if (step_duration_or_id > 0xc) return 3; // Error: Duration (or sensor ID) too large
      if ((step_target_temp < 0xaf) || (0x15e < step_target_temp)) return 1; // Temp out of range
    } else if (step_type == 1) { // Type 1: sensor-based step (duration_or_id is actual duration)
      if ((step_duration_or_id < 0x708) || (0x3840 < step_duration_or_id)) return 2; // Duration out of range
      if ((step_target_temp < 0xaf) || (0x15e < step_target_temp)) return 1; // Temp out of range
    } else {
      return 4; // Error: Unknown step type
    }
  }

  new_state(state_ptr); // Reset state before adding new program
  for (uint32_t i = 0; i < num_steps; i++) {
    add_step(state_ptr,
             *(uint32_t *)((char *)steps_data + i * 0xc),
             *(uint32_t *)((char *)steps_data + i * 0xc + 4),
             *(uint32_t *)((char *)steps_data + i * 0xc + 8));
  }
  *(uint16_t *)((char *)state_ptr + 0x668) = 0;       // Set current step index to 0
  *(uint16_t *)((char *)state_ptr + 0x65c) = num_steps; // Set total step count
  return 0;
}

// Function: get_program
// Retrieves the current program steps from the system.
// state_ptr: Pointer to the main state structure.
// num_steps_out: Pointer to an integer to store the number of retrieved steps.
// output_buffer: Pointer to a buffer to store the step data (each step 12 bytes).
// Returns 0 on success, 1 if NULL state_ptr or no program loaded.
uint32_t get_program(void *state_ptr, int *num_steps_out, void *output_buffer) {
  if (state_ptr == NULL || *(uint32_t *)((char *)state_ptr + 0x678) == 0) {
    *num_steps_out = 0;
    return 1; // Error: No program loaded or NULL state_ptr
  }

  StepNode *current_step = (StepNode *)(uintptr_t)*(uint32_t *)((char *)state_ptr + 0x678);
  int step_index = 0;
  while (current_step != NULL) {
    // Copy step data to output buffer (12 bytes per step)
    *(uint32_t *)((char *)output_buffer + step_index * 0xc) = current_step->type;
    *(uint32_t *)((char *)output_buffer + step_index * 0xc + 4) = current_step->duration_or_id;
    *(uint32_t *)((char *)output_buffer + step_index * 0xc + 8) = current_step->target_temp;
    
    current_step = (StepNode *)(uintptr_t)current_step->next_addr;
    step_index++;
  }
  *num_steps_out = step_index;
  return 0;
}

// Function: get_status
// Retrieves various status parameters of the system.
// state_ptr: Pointer to the main state structure.
// status_out: Pointer to an array of uint32_t to store the status values.
// Returns true if state_ptr is NULL (an error condition), false otherwise.
bool get_status(void *state_ptr, uint32_t *status_out) {
  if (state_ptr != NULL) {
    status_out[0] = 0x10a; // Some fixed status code
    status_out[1] = *(uint32_t *)((char *)state_ptr + 0x650); // Target temp
    status_out[2] = (uint32_t)round(*(double *)((char *)state_ptr + 0x648)); // Current temp
    status_out[3] = *(uint32_t *)((char *)state_ptr + 0x4); // Power state
    status_out[4] = *(uint32_t *)((char *)state_ptr + 0x660); // Heating state
    status_out[5] = *(uint32_t *)((char *)state_ptr + 0x0); // Field 0
  }
  return state_ptr == NULL;
}

// Function: get_sensors
// Retrieves data for all connected sensors.
// state_ptr: Pointer to the main state structure.
// output_buffer: Pointer to a buffer to store sensor data (each sensor 4 uint32_t values).
void get_sensors(void *state_ptr, void *output_buffer) {
  if (!state_ptr) return;

  uint32_t sensor_data_buffer[10 * 4]; // Max 10 sensors, each storing 4 uint32_t values
  int sensor_index = 0;

  SensorNode *current_sensor = (SensorNode *)(uintptr_t)*(uint32_t *)((char *)state_ptr + 0x658);
  while (current_sensor != NULL) {
    sensor_data_buffer[sensor_index * 4] = current_sensor->id;
    sensor_data_buffer[sensor_index * 4 + 1] = current_sensor->type;
    sensor_data_buffer[sensor_index * 4 + 2] = current_sensor->interval;
    sensor_data_buffer[sensor_index * 4 + 3] = (uint32_t)round(current_sensor->value);
    
    current_sensor = (SensorNode *)(uintptr_t)current_sensor->next_addr;
    sensor_index++;
  }
  memcpy(output_buffer, sensor_data_buffer, sensor_index * 4 * sizeof(uint32_t));
}

// Function: update_sensors
// Updates the value of all connected sensors based on the current system temperature.
// state_ptr: Pointer to the main state structure.
void update_sensors(void *state_ptr) {
  if (!state_ptr) return;

  SensorNode *current_sensor = (SensorNode *)(uintptr_t)*(uint32_t *)((char *)state_ptr + 0x658);
  while (current_sensor != NULL) {
    current_sensor->value =
        current_sensor->value +
        (*(double *)((char *)state_ptr + 0x648) - current_sensor->value) *
        (DAT_00019010 + (double)current_sensor->interval / DAT_00019008);
    current_sensor = (SensorNode *)(uintptr_t)current_sensor->next_addr;
  }
}

// Function: check_program
// Checks the current program step and advances if conditions are met.
// state_ptr: Pointer to the main state structure.
void check_program(void *state_ptr) {
  if (!state_ptr) return;

  int current_step_idx = *(uint16_t *)((char *)state_ptr + 0x668);
  StepNode *current_program_step = (StepNode *)get_step(state_ptr, current_step_idx);

  if (current_program_step == NULL) {
    *(uint8_t *)((char *)state_ptr + 0x674) = 1; // Program finished
  } else if (current_program_step->type == 0) { // Type 0: sensor-based step (as interpreted by check_program)
    // Note: There's an inconsistency in the original snippet. `add_steps` validates
    // `duration_or_id` for type 0 as a duration (max 12), while `check_program`
    // uses it as a sensor ID for `find_sensor`. Following `check_program`'s logic here.
    SensorNode *target_sensor = (SensorNode *)find_sensor(state_ptr, current_program_step->duration_or_id & 0xffff);

    if (target_sensor == NULL) {
      *(uint16_t *)((char *)state_ptr + 0x668) += 1; // Advance to next step (sensor not found)
    } else {
      int sensor_temp = (int)round(target_sensor->value);
      uint32_t target_state_temp = *(uint32_t *)((char *)state_ptr + 0x650);
      if (target_state_temp < (uint32_t)sensor_temp + 5U) { // If current temp is close to target + 5
        *(uint32_t *)((char *)state_ptr + 0x650) = current_program_step->target_temp; // Set new target temp
        *(uint16_t *)((char *)state_ptr + 0x668) += 1; // Advance to next step
      }
    }
  } else if (current_program_step->type == 1) { // Type 1: time-based step
    // Check if the elapsed time exceeds the step's duration
    if (current_program_step->duration_or_id < *(uint32_t *)((char *)state_ptr + 0x66c)) {
      *(uint32_t *)((char *)state_ptr + 0x650) = current_program_step->target_temp; // Set new target temp
      *(uint16_t *)((char *)state_ptr + 0x668) += 1; // Advance to next step
    }
  }
}

// Function: add_history
// Records the current system temperature into the history log.
// state_ptr: Pointer to the main state structure.
void add_history(void *state_ptr) {
  if (!state_ptr) return;

  uint32_t current_temp_rounded = (uint32_t)round(*(double *)((char *)state_ptr + 0x648));
  uint32_t *history_idx_ptr = (uint32_t *)((char *)state_ptr + 0x670);
  // History data starts at offset 8, and is an array of uint32_t
  *(uint32_t *)((char *)state_ptr + 0x8 + (*history_idx_ptr) * sizeof(uint32_t)) = current_temp_rounded;
  (*history_idx_ptr)++;
}

// Function: simulate_seconds
// Simulates the system's operation for a specified number of seconds.
// state_ptr: Pointer to the main state structure.
// seconds: Number of seconds to simulate.
// Returns 0 on normal completion, 1 if NULL state_ptr or invalid seconds, 2 if program finishes.
uint32_t simulate_seconds(void *state_ptr, int seconds) {
  if (state_ptr == NULL || seconds < 10 || seconds > 8000) return 1;

  for (int i = 0; i < seconds; i++) {
    uint32_t power_state = *(uint32_t *)((char *)state_ptr + 0x4);
    uint32_t heating_state = *(uint32_t *)((char *)state_ptr + 0x660);
    double *current_temp_ptr = (double *)((char *)state_ptr + 0x648);
    uint32_t target_temp = *(uint32_t *)((char *)state_ptr + 0x650);
    uint32_t ambient_temp = *(uint32_t *)((char *)state_ptr + 0x664);

    if (power_state == 1) { // If powered on
      if (heating_state == 1) { // If heating
        *current_temp_ptr += 1.0; // Increase temp
        if (DAT_00019018 + (double)target_temp < *current_temp_ptr) {
          *(uint32_t *)((char *)state_ptr + 0x660) = 0; // Turn off heating
        }
      } else { // If cooling
        if ((double)ambient_temp < *current_temp_ptr) {
          *current_temp_ptr -= DAT_00019020; // Decrease temp
        }
        if (*current_temp_ptr < (double)target_temp - DAT_00019018) {
          *(uint32_t *)((char *)state_ptr + 0x660) = 1; // Turn on heating
        }
      }
    } else { // If powered off
      if ((double)ambient_temp < *current_temp_ptr) {
        *current_temp_ptr -= DAT_00019020; // Decrease temp towards ambient
      }
    }

    update_sensors(state_ptr);

    int current_step_idx = *(uint16_t *)((char *)state_ptr + 0x668);
    if (current_step_idx != 0xffff) { // If a program is running (not 0xffff)
      check_program(state_ptr);
      if (*(uint8_t *)((char *)state_ptr + 0x674) != 0) { // If program finished
        return 2; // Program completed or error
      }
    }

    uint32_t *elapsed_time_ptr = (uint32_t *)((char *)state_ptr + 0x66c);
    if (*elapsed_time_ptr % 0x3c == 0) { // Every 60 seconds (0x3c)
      add_history(state_ptr);
    }
    (*elapsed_time_ptr)++;
  }
  return 0;
}

// Function: validate_fw
// Calculates a checksum for a firmware block.
// firmware_ptr: Pointer to the firmware data.
// Returns the calculated checksum, or 1 if firmware_ptr is NULL.
int validate_fw(const void *firmware_ptr) {
  if (firmware_ptr == NULL) return 1;
  int checksum = 0;
  // Note: 0x4347c000 is a hardcoded base address in the original snippet.
  // Assuming firmware_ptr points to this base or equivalent.
  for (uint16_t i = 0; i < 0x1000; i++) {
    checksum += (uint32_t)*(uint8_t *)((char *)firmware_ptr + i) * (i + 1);
  }
  return checksum;
}

// Function: check_val
// Calculates a checksum for a data block.
// data_ptr: Pointer to the data.
// length: Length of the data block.
// Returns the calculated checksum.
int check_val(const void *data_ptr, uint16_t length) {
  int checksum = 0;
  for (uint16_t i = 0; i < length; i++) {
    checksum += (uint32_t)*(uint8_t *)((char *)data_ptr + i) * (i + 1);
  }
  return checksum;
}

// Function: exit_normal
// Exits the program with a normal status.
int exit_normal(void) {
  exit(0);
  return 0; // Unreachable
}