#include <stdint.h>
#include <math.h>   // For round, floor
#include <string.h> // For memset
#include <stdio.h>  // For printf, fprintf
#include <stdlib.h> // For exit

// Define placeholder constants
// These values are arbitrary and chosen to make the code compile and run without immediate errors.
// In a real scenario, these would come from actual hardware/algorithm specifications.
const double DAT_00016000 = 0.0001;  // Small frequency adjustment step
const double DAT_00016008 = 64.0;    // Scaling factor for phase error
const double DAT_00016010 = 1.0;     // Initial frequency/phase accumulator value
const double DAT_00016018 = 0.5;     // Offset for floor calculation
const double _DAT_00016020 = 0.0001; // Another frequency adjustment step (might be same as DAT_00016000)

// Dummy function for external calls
void _terminate(int status) {
    fprintf(stderr, "Program terminated with status: %d\n", status);
    exit(status);
}

void receive_packet(void *data, uint8_t length, uint16_t crc) {
    printf("Packet Received: Length = %u, CRC = 0x%04X\n", length, crc);
    uint8_t *packet_data = (uint8_t *)data;
    printf("Packet Data: ");
    for (int i = 0; i < length && i < 16; ++i) { // Print up to 16 bytes
        printf("%02X ", packet_data[i]);
    }
    printf("\n");
}

// Structure representing the baseband state
typedef struct {
    uint8_t state_main;             // Offset 0x00 (*param_1 in run_cdr, and what param_1+8 might have referred to in do_sample)
    uint8_t prev_sample;            // Offset 0x01 (param_1[1])
    uint8_t _padding_02_03[2];      // Padding to align double
    double freq_phase_acc;          // Offset 0x04 (*(double *)(param_1 + 4))
    double phase_error_acc;         // Offset 0x0c (*(double *)(param_1 + 0xc))
    uint32_t sample_count_total;    // Offset 0x14 (*(uint *)(param_1 + 0x14))
    uint32_t sample_count_current;  // Offset 0x18 (*(uint *)(param_1 + 0x18))
    uint32_t lock_counter;          // Offset 0x1c (*(uint *)(param_1 + 0x1c))
    uint8_t flag_0;                 // Offset 0x20 (param_1[0x20] in run_cdr)
    uint8_t data_bit_val;           // Offset 0x21 (param_1[0x21] in run_cdr, do_sample)
    uint8_t bit_counter_state1;     // Offset 0x22 (param_1[0x22] in run_cdr, do_sample)
    uint8_t payload_length;         // Offset 0x23 (param_1[0x23] in run_cdr, do_sample)
    uint8_t payload_byte_index;     // Offset 0x24 (Used as param_1[9] in do_sample, moved here)
    uint8_t bit_counter_current;    // Offset 0x25 (param_1[0x25] in run_cdr, do_sample)
    uint8_t payload[256];           // Offset 0x26 (param_1 + 0x26)
    uint16_t crc;                   // Offset 0x126 (*(uint16_t *)(param_1 + 0x126))
} baseband_state_t;

// Function prototypes
void init_baseband(baseband_state_t *state);
void reset_baseband_state(baseband_state_t *state);
void do_sample(baseband_state_t *state, uint8_t sample_bit);
void free_run_lock(baseband_state_t *state);
void run_cdr(baseband_state_t *state, uint8_t sample_input);
void process_sample(baseband_state_t *state, uint8_t sample);

// Function: init_baseband
void init_baseband(baseband_state_t *state) {
  state->state_main = 0;
  state->prev_sample = 0;
  state->freq_phase_acc = 0.0;
  state->phase_error_acc = 0.0;
  state->sample_count_total = 0;
  state->sample_count_current = 0;
  state->lock_counter = 0;
  state->flag_0 = 0;
  state->bit_counter_state1 = 0;
  state->data_bit_val = 0;
  state->payload_length = 0;
  state->payload_byte_index = 0;
  state->bit_counter_current = 0;
  state->crc = 0;
  return;
}

// Function: reset_baseband_state
void reset_baseband_state(baseband_state_t *state) {
  state->state_main = 0;
  state->flag_0 = 0;
  state->prev_sample = 0;
  state->freq_phase_acc = 0.0;
  state->phase_error_acc = 0.0;
  state->sample_count_total = 0;
  state->sample_count_current = 0;
  state->lock_counter = 0;
  state->bit_counter_state1 = 0;
  state->data_bit_val = 0;
  state->payload_length = 0;
  state->payload_byte_index = 0;
  state->bit_counter_current = 0;
  state->crc = 0;
  return;
}

// Function: do_sample
void do_sample(baseband_state_t *state, uint8_t sample_bit) {
  if (state->state_main != 0) { // Assuming state_main is the primary state for do_sample too
    if (state->state_main == 1) { // State 1: Preamble/Sync detection
      if (state->bit_counter_state1 == 0) {
        if (sample_bit == 1) {
          state->bit_counter_state1 = 1;
        } else if (sample_bit == state->data_bit_val) {
          reset_baseband_state(state);
        } else {
          state->data_bit_val = sample_bit;
        }
      } else if (state->bit_counter_state1 == 1) {
        if (sample_bit == 0) {
          state->data_bit_val = 0;
          state->bit_counter_state1 = 0;
        } else {
          state->bit_counter_state1 = 2;
        }
      } else if (state->data_bit_val == 0 && state->bit_counter_state1 == 2 && sample_bit == 1) {
        state->data_bit_val = 1;
      } else {
        // The original expression extracts the bit at `15 - state->bit_counter_state1` position from `0xcfa3U`.
        uint8_t shift_val = 15 - state->bit_counter_state1;
        if ((sample_bit & 1) == ((0xcfa3U >> shift_val) & 1)) {
          state->bit_counter_state1++;
        } else {
          reset_baseband_state(state);
        }

        if (state->bit_counter_state1 == 0x10) { // If 16 bits matched
          state->state_main = 2; // Transition to state 2 for length reception
          state->bit_counter_current = 0;
          state->payload_byte_index = 0;
          state->payload_length = 0; // Ensure payload_length is reset for new packet
        }
      }
    } else if (state->state_main == 2) { // State 2: Length reception
      state->payload_length = (state->payload_length & ~(1 << (7 - state->bit_counter_current))) | ((sample_bit & 1) << (7 - state->bit_counter_current));
      state->bit_counter_current++;
      if (state->bit_counter_current == 8) {
        if (state->payload_length == 0) {
          reset_baseband_state(state);
        } else {
          state->state_main = 3; // Transition to state 3 for payload reception
          state->bit_counter_current = 0;
          state->payload_byte_index = 0;
          memset(state->payload, 0, sizeof(state->payload)); // Clear payload buffer
        }
      }
    } else if (state->state_main == 3) { // State 3: Payload reception
      state->payload[state->payload_byte_index] = (state->payload[state->payload_byte_index] & ~(1 << (7 - state->bit_counter_current))) | ((sample_bit & 1) << (7 - state->bit_counter_current));
      state->bit_counter_current++;
      if (state->bit_counter_current == 8) {
        state->bit_counter_current = 0;
        state->payload_byte_index++;
        if (state->payload_byte_index == state->payload_length) {
          state->state_main = 4; // Transition to state 4 for CRC reception
          state->bit_counter_current = 0; // Reset for CRC bits
          state->crc = 0; // Reset CRC
        }
      }
    } else if (state->state_main == 4) { // State 4: CRC reception
      state->crc = (state->crc & ~(1 << (15 - state->bit_counter_current))) | ((sample_bit & 1) << (15 - state->bit_counter_current));
      state->bit_counter_current++;
      if (state->bit_counter_current == 16) {
        receive_packet(state->payload, state->payload_length, state->crc);
        reset_baseband_state(state);
      }
    }
  }
  return;
}

// Function: free_run_lock
void free_run_lock(baseband_state_t *state) {
  double ratio = (double)state->sample_count_current /
                 ((double)state->sample_count_total * state->freq_phase_acc);
  if (1.0 <= ratio) {
    if (1.0 < ratio) {
      state->freq_phase_acc += DAT_00016000;
    }
  } else {
    state->freq_phase_acc -= DAT_00016000;
  }
  state->sample_count_total = 0;
  state->phase_error_acc = state->freq_phase_acc / DAT_00016008;
  return;
}

// Function: run_cdr
void run_cdr(baseband_state_t *state, uint8_t sample_input) {
  state->sample_count_current++;

  if (sample_input == state->prev_sample) {
    if (state->state_main == 2) { // Locked state
      uint32_t rounded_phase_error = (uint32_t)round(state->phase_error_acc);
      if (state->sample_count_current == rounded_phase_error) {
        state->phase_error_acc += state->freq_phase_acc; // Accumulate phase
        state->sample_count_total++; // Increment total samples for lock
        do_sample(state, sample_input); // Process the sample bit
      }
    }
  } else { // Sample changed
    if (state->state_main == 0) { // Unlocked state, try to acquire
      state->state_main = 1; // Transition to acquisition state
      state->freq_phase_acc = DAT_00016010; // Reset frequency/phase accumulator
      state->sample_count_current = 0; // Reset current sample count
    } else if (state->state_main == 1) { // Acquisition state
      // Condition: (state->sample_count_current < 36) && (28 < state->sample_count_current)
      if ((state->sample_count_current < 36) && (state->sample_count_current > 28)) {
        uint32_t floor_val = (uint32_t)floor(DAT_00016018 + state->freq_phase_acc);
        if (state->sample_count_current < floor_val) {
          state->freq_phase_acc -= _DAT_00016020;
        } else if (floor_val < state->sample_count_current) {
          state->freq_phase_acc += _DAT_00016020;
        }
        state->lock_counter++;
        if (state->lock_counter > 0xe) { // If lock_counter > 14 (i.e., >= 15)
          state->flag_0 = 1; // Flag indicating sync/preamble detection enabled
          state->bit_counter_state1 = 0; // Reset bit counter for state 1
          state->data_bit_val = state->prev_sample; // Store previous sample for comparison
          state->state_main = 2; // Transition to locked state
          state->sample_count_total = 0; // Reset total samples
          state->phase_error_acc = state->freq_phase_acc / DAT_00016008; // Recalculate phase error accumulator
        }
      } else {
        state->state_main = 0; // Out of range, reset to unlocked
      }
    } else if (state->state_main == 2) { // Locked state, but sample changed (loss of lock)
      free_run_lock(state);
    } else { // Unknown state, terminate
      _terminate(0xfffffffd);
    }
    state->prev_sample = sample_input; // Update previous sample
    state->sample_count_current = 0; // Reset current sample count
  }
  return;
}

// Function: process_sample
void process_sample(baseband_state_t *state, uint8_t sample) {
  run_cdr(state, sample);
  return;
}

// Main function for testing
int main() {
    baseband_state_t bb_state;
    init_baseband(&bb_state);

    printf("Initial baseband state: state_main = %u\n", bb_state.state_main);

    // Simulate some samples
    // A sequence of 0s and 1s to test state transitions and packet reception.
    // Preamble: (0xcfa3U = 1100111110100011 binary) - 16 bits, extracted MSB to LSB
    // Expected bits: 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1
    uint8_t samples[] = {
        // Initial samples to trigger acquisition and lock (state_main=0 -> 1 -> 2)
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        // More samples to ensure lock_counter exceeds 14 and state_main becomes 2
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // A sequence that should trigger state changes in do_sample
        // These samples are fed to do_sample when state_main is 2
        // Initial 0,1,0,1 sequence for do_sample's state_main=1 condition (bit_counter_state1 = 0,1,2)
        0, // data_bit_val = 0
        1, // bit_counter_state1 = 1
        0, // data_bit_val = 0, bit_counter_state1 = 0
        1, // bit_counter_state1 = 1
        1, // bit_counter_state1 = 2
        1, // data_bit_val = 1 (if prev data_bit_val was 0)

        // Preamble sequence (0xcfa3U = 1100111110100011)
        // Expected bits: 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1
        1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1,
        // Length (e.g., 5 bytes = 00000101 binary)
        0, 0, 0, 0, 0, 1, 0, 1,
        // Payload (e.g., 5 bytes of 0xAA, 0xBB, 0xCC, 0xDD, 0xEE)
        // 0xAA = 10101010
        1,0,1,0,1,0,1,0,
        // 0xBB = 10111011
        1,0,1,1,1,0,1,1,
        // 0xCC = 11001100
        1,1,0,0,1,1,0,0,
        // 0xDD = 11011101
        1,1,0,1,1,1,0,1,
        // 0xEE = 11101110
        1,1,1,0,1,1,1,0,
        // CRC (e.g., 0x1234 = 0001001000110100 binary)
        0,0,0,1,0,0,1,0,0,0,1,1,0,1,0,0,
        // More samples to ensure reset
        0,0,0,0,0,0,0,0
    };

    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        process_sample(&bb_state, samples[i]);
        // Uncomment for detailed step-by-step output:
        // printf("Sample %zu: %u, state_main = %u, prev_sample = %u, lock_counter = %u, bit_counter_state1 = %u, payload_len = %u, payload_idx = %u, bit_curr = %u\n",
        //        i, samples[i], bb_state.state_main, bb_state.prev_sample, bb_state.lock_counter, bb_state.bit_counter_state1, bb_state.payload_length, bb_state.payload_byte_index, bb_state.bit_counter_current);
    }

    printf("Final baseband state: state_main = %u\n", bb_state.state_main);

    return 0;
}