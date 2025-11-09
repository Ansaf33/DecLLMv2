#include <stdlib.h> // For calloc, free, NULL
#include <stdio.h>  // For printf, fprintf

// --- Constants and Type Definitions ---
// Error codes
#define TANK_OK                 0
#define TANK_TRUE               1
#define TANK_FALSE              0
#define TANK_ERR_NULL_PTR       -15 // -0xf
#define TANK_ERR_END_OF_LIFE    -19 // -0x13 (Tank is at end of life)
#define TANK_ERR_ALREADY_EOL    -17 // -0x11 (Attempting to set EOL on an already EOL tank)
#define TANK_ERR_EMPTY_WATER    -18 // -0x12 (Attempting to remove water from an empty tank)

// Status codes
#define TANK_STATUS_IDLE        6
#define TANK_STATUS_IN_SERVICE  2

// Assuming H2O_Object is an int, representing some ID or handle
typedef int H2O_Object;

// Dummy H2O functions for compilation
H2O_Object create_h2o() {
    static int h2o_id_counter = 0;
    return ++h2o_id_counter; // Returns a unique ID for demonstration
}

void destroy_h2o(H2O_Object h2o) {
    (void)h2o; // Suppress unused parameter warning
    // In a real system, this would free/cleanup an H2O object
}

// Tank structure definition based on byte offsets from decompiler output
// Assuming default compiler alignment for char and int aligns members as needed.
// The first 8 bytes are char members, followed by an array of 4-byte ints.
typedef struct Tank {
    unsigned char id;          // Offset 0
    unsigned char fill_rate;   // Offset 1
    unsigned char drain_rate;  // Offset 2
    unsigned char valve_open;  // Offset 3
    unsigned char capacity;    // Offset 4 (max H2O objects)
    unsigned char water_count; // Offset 5 (current H2O objects)
    unsigned char status;      // Offset 6
    unsigned char end_of_life; // Offset 7
    H2O_Object water_objects[];// Flexible array member (FAM), starts at offset 8
} Tank;

// --- Function Declarations ---
// Forward declarations for functions that call each other
int is_end_of_life(Tank *tank);
int set_status(Tank *tank, unsigned char status);
int set_fill_rate(Tank *tank, unsigned char rate);
int set_drain_rate(Tank *tank, unsigned char rate);
int close_valve(Tank *tank);
int set_out_of_service(Tank *tank); // Calls set_status, set_fill_rate, set_drain_rate, close_valve

// --- Function Implementations ---

// Function: create_tank
Tank *create_tank(unsigned char id, unsigned char capacity) {
    // The original calloc `(param_2 + 2) * 4` implies `capacity * 4 + 8` bytes.
    // This perfectly matches `sizeof(Tank)` (8 bytes for fixed members) + `capacity * sizeof(H2O_Object)`.
    Tank *tank = (Tank *)calloc(1, sizeof(Tank) + capacity * sizeof(H2O_Object));
    if (tank == NULL) {
        return NULL;
    }

    tank->id = id;
    tank->fill_rate = 0;
    tank->drain_rate = 0;
    tank->valve_open = 0;
    tank->capacity = capacity;
    tank->water_count = 0;
    tank->status = TANK_STATUS_IDLE; // Initial status 6
    tank->end_of_life = 0;

    // The original loop limit was 0x5a (90). Assuming `capacity` is the actual limit for safety.
    for (unsigned char i = 0; i < capacity; i++) {
        tank->water_objects[i] = create_h2o();
        tank->water_count++;
    }
    return tank;
}

// Function: set_fill_rate
int set_fill_rate(Tank *tank, unsigned char rate) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) { // is_end_of_life also checks for NULL
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    if (tank->status == TANK_STATUS_IDLE) { // '\x06'
        tank->fill_rate = 0;
    } else {
        tank->fill_rate = rate;
    }
    return TANK_OK;
}

// Function: set_drain_rate
int set_drain_rate(Tank *tank, unsigned char rate) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    if (tank->status == TANK_STATUS_IDLE) { // '\x06'
        tank->drain_rate = 0;
    } else {
        tank->drain_rate = rate;
    }
    return TANK_OK;
}

// Function: open_valve
int open_valve(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    tank->valve_open = 1;
    return TANK_OK;
}

// Function: close_valve
int close_valve(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    tank->valve_open = 0;
    return TANK_OK;
}

// Function: set_status
int set_status(Tank *tank, unsigned char status) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    tank->status = status;
    return TANK_OK;
}

// Function: set_in_service
int set_in_service(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    int result = set_status(tank, TANK_STATUS_IN_SERVICE); // 2
    if (result != TANK_OK) return result;
    result = set_fill_rate(tank, 12); // 0xc
    if (result != TANK_OK) return result;
    return TANK_OK;
}

// Function: set_out_of_service
int set_out_of_service(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    int result = set_status(tank, TANK_STATUS_IDLE); // 6
    if (result != TANK_OK) return result;
    result = set_fill_rate(tank, 0);
    if (result != TANK_OK) return result;
    result = set_drain_rate(tank, 0);
    if (result != TANK_OK) return result;
    result = close_valve(tank);
    if (result != TANK_OK) return result;
    return TANK_OK;
}

// Function: is_in_service
int is_in_service(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    return (tank->status == TANK_STATUS_IN_SERVICE) ? TANK_TRUE : TANK_FALSE; // '\x02'
}

// Function: set_end_of_life
int set_end_of_life(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    if (tank->end_of_life == TANK_TRUE) { // '\x01'
        return TANK_ERR_ALREADY_EOL;
    }
    // Original code doesn't check return value of set_out_of_service
    set_out_of_service(tank);
    tank->end_of_life = TANK_TRUE; // 1
    return TANK_OK;
}

// Function: is_end_of_life
int is_end_of_life(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    return (tank->end_of_life == TANK_TRUE) ? TANK_TRUE : TANK_FALSE; // '\x01'
}

// Function: add_water
int add_water(Tank *tank, unsigned char amount) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    // No explicit capacity check in original, assuming caller or system ensures space.
    // If `water_count + amount` exceeds `capacity`, this is a buffer overflow.
    // For strict interpretation, I won't add an explicit check not present in original.
    // However, if `amount` would cause `water_count` to exceed `capacity`, it's an issue.
    // Assuming `water_count` will not exceed `capacity` due to `unsigned char` limits or external control.
    for (unsigned char i = 0; i < amount; i++) {
        tank->water_objects[tank->water_count] = create_h2o();
        tank->water_count++;
    }
    return TANK_OK;
}

// Function: rm_water
int rm_water(Tank *tank, unsigned char amount) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    for (unsigned char i = 0; i < amount; i++) {
        if (tank->water_count == 0) {
            return TANK_ERR_EMPTY_WATER; // 0xffffffee
        }
        tank->water_count--;
        destroy_h2o(tank->water_objects[tank->water_count]);
        tank->water_objects[tank->water_count] = 0; // Clear the slot
    }
    return TANK_OK;
}

// Function: do_fill
int do_fill(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    if (tank->valve_open == 1 && tank->status == TANK_STATUS_IN_SERVICE) { // '\x01' and '\x02'
        return add_water(tank, tank->fill_rate);
    }
    return TANK_OK; // Returns 0 if conditions not met
}

// Function: do_drain
int do_drain(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    if (tank->status == TANK_STATUS_IN_SERVICE) { // '\x02'
        return rm_water(tank, tank->drain_rate);
    }
    return TANK_OK; // Returns 0 if conditions not met
}

// Function: is_level_low
int is_level_low(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    return (tank->water_count < 0x1a) ? TANK_TRUE : TANK_FALSE; // 26
}

// Function: is_level_crit_low
int is_level_crit_low(Tank *tank) {
    if (tank == NULL) {
        return TANK_ERR_NULL_PTR;
    }
    int eol_status = is_end_of_life(tank);
    if (eol_status == TANK_ERR_NULL_PTR) {
        return TANK_ERR_NULL_PTR;
    }
    if (eol_status == TANK_TRUE) {
        return TANK_ERR_END_OF_LIFE;
    }

    return (tank->water_count < 6) ? TANK_TRUE : TANK_FALSE;
}

// Main function to demonstrate and ensure compilability
int main() {
    printf("Initializing tank system...\n");

    Tank *myTank = create_tank(1, 100); // ID 1, capacity 100
    if (myTank == NULL) {
        fprintf(stderr, "Failed to create tank.\n");
        return 1;
    }
    printf("Tank created with ID: %u, Capacity: %u, Water Count: %u\n",
           myTank->id, myTank->capacity, myTank->water_count);

    int result;

    result = set_in_service(myTank);
    printf("Set in service: %d (Expected %d)\n", result, TANK_OK);
    printf("Tank status: %u, Fill rate: %u\n", myTank->status, myTank->fill_rate);

    result = open_valve(myTank);
    printf("Open valve: %d (Expected %d)\n", result, TANK_OK);
    printf("Valve open: %u\n", myTank->valve_open);

    printf("Current water count: %u\n", myTank->water_count);
    result = do_fill(myTank);
    printf("Do fill (1st time): %d (Expected %d)\n", result, TANK_OK);
    printf("Water count after fill: %u\n", myTank->water_count);

    result = set_drain_rate(myTank, 5);
    printf("Set drain rate to 5: %d (Expected %d)\n", result, TANK_OK);
    printf("Drain rate: %u\n", myTank->drain_rate);

    result = do_drain(myTank);
    printf("Do drain (1st time): %d (Expected %d)\n", result, TANK_OK);
    printf("Water count after drain: %u\n", myTank->water_count);

    result = is_level_low(myTank);
    printf("Is level low? %d (Expected %d or %d)\n", result, TANK_TRUE, TANK_FALSE);
    result = is_level_crit_low(myTank);
    printf("Is level critically low? %d (Expected %d or %d)\n", result, TANK_TRUE, TANK_FALSE);

    result = set_end_of_life(myTank);
    printf("Set end of life: %d (Expected %d)\n", result, TANK_OK);
    printf("Tank end of life status: %u\n", myTank->end_of_life);

    result = is_end_of_life(myTank);
    printf("Is tank end of life? %d (Expected %d)\n", result, TANK_TRUE);

    result = set_in_service(myTank);
    printf("Attempt set in service on EOL tank: %d (Expected %d)\n", result, TANK_ERR_END_OF_LIFE);

    free(myTank);
    myTank = NULL;

    printf("Tank system demonstration complete.\n");
    return 0;
}