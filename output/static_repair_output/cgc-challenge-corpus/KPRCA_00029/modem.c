#include <stdint.h>   // For int16_t, int32_t, uint32_t, int64_t
#include <stdbool.h>  // For bool
#include <string.h>   // For memcpy, memmove
#include <stdlib.h>   // For exit, abs
#include <unistd.h>   // For read, write
#include <sys/select.h> // For select
#include <sys/time.h> // For struct timeval

// Placeholder for buffer structure and related functions
typedef struct Buffer {
    unsigned char *data;
    uint32_t capacity;      // Capacity in bytes
    uint32_t read_offset;   // Offset in bits
    uint32_t write_offset;  // Offset in bits
} Buffer;

// Forward declarations for external functions (or functions defined later)
// These are minimal definitions to allow compilation. Actual implementations would be complex.
int comp_decode(unsigned char param_1);
void buffer_read_bytes(Buffer *param_1, unsigned char *param_2, uint32_t param_3);
void buffer_write_bit(Buffer *param_1, uint32_t param_2);
int comp_encode(int param_1); // Assuming it returns a value that can be cast to char
int16_t fastsin(int param_1); // Returns short, but argument is int
int buffer_read_bit(Buffer *param_1); // Returns -1 on error/EOF, 0 or 1 for bit
void buffer_write_bytes(Buffer *param_1, unsigned char *param_2, uint32_t param_3);
void buffer_init(Buffer *param_1, unsigned char *param_2, uint32_t param_3);
void buffer_write_seek(Buffer *param_1, uint32_t param_2_bits, int param_3_whence);
int64_t buffer_read_remaining(Buffer *param_1); // Returns remaining bits
void frame_encode(Buffer *param_1, Buffer *param_2);
void ecc_init(void);
int receive(int fd, unsigned char *buf, uint32_t count, int *bytes_read);
int transmit(int fd, const unsigned char *buf, uint32_t count, int *bytes_written);
int frame_decode(Buffer *param_1, Buffer *param_2); // Returns 0 for failure, 1 for success, 2 for specific event
void buffer_read_seek(Buffer *param_1, int32_t offset_bits, int32_t whence_bits);
void buffer_write_bits(Buffer *param_1, uint32_t param_2_value, uint32_t param_3_num_bits);

// Placeholder implementations for undefined functions
int comp_decode(unsigned char param_1) { return (int)param_1; }
void buffer_read_bytes(Buffer *param_1, unsigned char *param_2, uint32_t param_3) {
    if (!param_1 || !param_2 || (param_1->read_offset / 8) + param_3 > param_1->capacity) return;
    memcpy(param_2, param_1->data + (param_1->read_offset / 8), param_3);
    param_1->read_offset += param_3 * 8; // Advance bit offset
}
void buffer_write_bit(Buffer *param_1, uint32_t param_2) { /* stub */ }
int comp_encode(int param_1) { return param_1 & 0xFF; } // Assuming it returns a byte value
int16_t fastsin(int param_1) { return (int16_t)(param_1 % 0x7fff); } // Placeholder
int buffer_read_bit(Buffer *param_1) {
    if (!param_1 || param_1->read_offset >= (uint32_t)param_1->capacity * 8) return -1;
    // For simplicity, always return 0, advance bit offset
    param_1->read_offset++;
    return 0;
}
void buffer_write_bytes(Buffer *param_1, unsigned char *param_2, uint32_t param_3) {
    if (!param_1 || !param_2 || (param_1->write_offset / 8) + param_3 > param_1->capacity) return;
    memcpy(param_1->data + (param_1->write_offset / 8), param_2, param_3);
    param_1->write_offset += param_3 * 8; // Advance bit offset
}
void buffer_init(Buffer *param_1, unsigned char *param_2, uint32_t param_3) {
    if (!param_1) return;
    param_1->data = param_2;
    param_1->capacity = param_3; // Capacity in bytes
    param_1->read_offset = 0; // Bits
    param_1->write_offset = 0; // Bits
}
void buffer_write_seek(Buffer *param_1, uint32_t param_2_bits, int param_3_whence) { /* stub */ }
int64_t buffer_read_remaining(Buffer *param_1) {
    if (!param_1) return 0;
    return (int64_t)param_1->capacity * 8 - param_1->read_offset; // Remaining bits
}
void frame_encode(Buffer *param_1, Buffer *param_2) { /* stub */ }
void ecc_init(void) { /* stub */ }
int receive(int fd, unsigned char *buf, uint32_t count, int *bytes_read) {
    *bytes_read = read(fd, buf, count);
    return (*bytes_read == -1) ? -1 : 0;
}
int transmit(int fd, const unsigned char *buf, uint32_t count, int *bytes_written) {
    *bytes_written = write(fd, buf, count);
    return (*bytes_written == -1) ? -1 : 0;
}
int frame_decode(Buffer *param_1, Buffer *param_2) { return 1; } // Always succeed for now
void buffer_read_seek(Buffer *param_1, int32_t offset_bits, int32_t whence_bits) {
    if (!param_1) return;
    param_1->read_offset = offset_bits; // Simple implementation for SEEK_SET
}
void buffer_write_bits(Buffer *param_1, uint32_t param_2_value, uint32_t param_3_num_bits) { /* stub */ }

// Placeholder functions for the switch_funcs array in modem_loop
void switch_func_0(void) {}
void switch_func_1(void) {}
void switch_func_2(void) {}
void switch_func_3(void) {}
void switch_func_4(void) {}

// Global variables
const int16_t BANDPASS_COEFFS[] = {
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000
}; // Size 17
const int16_t LOWPASS_COEFFS[] = {
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x3000, 0x4000,
    0x5000, 0x6000, 0x7000, 0x6000, 0x5000, 0x4000, 0x3000, 0x2000,
    0x1000, 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020,
    0x0010, 0x0008, 0x0004, 0x0002, 0x0001, 0x0001, 0x0001, 0x0001,
    0x0001, 0x0001, 0x0001, 0x0001, 0x0001
}; // Size 45

// History buffers for filters
// history_8 needs to be (filter_length - 1 + 80) shorts = (17 - 1 + 80) = 96 shorts
static int16_t history_8[96];
// history_7 needs to be (filter_length - 1 + 80) shorts = (45 - 1 + 80) = 124 shorts
static int16_t history_7[124];

// buf_6 for convolution: (80 new samples) + (9 old samples) = 89 shorts
static int16_t buf_6[89];

static int32_t phase_5 = -1; // Initialized to 0xffffffff
static int16_t t_4 = 0;

// Buffer for modem_decode and modem_encode_frame
static Buffer g_frames;
static unsigned char g_frames_data[0x2000]; // 8192 bytes

// Buffers for get_byte / ready_to_read / send_n_bytes
static unsigned char buf[0x400]; // 1024 bytes
static int buf_read = 0;
static int buf_offset = 0;

static unsigned char out_buf[0x400]; // 1024 bytes
static uint32_t out_buf_len = 0;

// Buffers for modem_loop
static unsigned char indata_3[0x400]; // 1024 bytes
static unsigned char decdata_2[0x200]; // 512 bytes
static unsigned char pktdata_1[0x400]; // 1024 bytes
static unsigned char outdata_0[0x8000]; // 32768 bytes

// Function pointer table for indirect calls in modem_loop
void (*switch_funcs_global[])(void) = {
    switch_func_0,
    switch_func_1,
    switch_func_2,
    switch_func_3,
    switch_func_4
};

// Helper function to resolve decompilation error in modem_encode
// Original `_modem_encode` was `void`, but its return value was used.
// This version returns `unsigned char` as implied by `buffer_write_bytes`.
static unsigned char _modem_encode_returns_char(int param_1) {
  int16_t sVar1 = (param_1 == 0) ? 0x3666 : 0x4333;
  t_4 += sVar1;
  return (unsigned char)comp_encode(fastsin(t_4) >> 2);
}

// Function: filter
void filter(int16_t *param_1_samples, const int16_t *param_2_coeffs, uint32_t param_3_filter_len, int16_t *param_4_history) {
  int16_t current_output_sample;
  uint32_t coeff_idx;
  uint32_t output_sample_idx;

  // Copy 80 new samples from param_1_samples into history buffer param_4_history
  // starting at index (filter_length - 1). This overwrites the oldest 80 samples.
  memcpy(param_4_history + (param_3_filter_len - 1), param_1_samples, 80 * sizeof(int16_t));

  // Calculate 80 output samples
  for (output_sample_idx = 0; output_sample_idx < 80; output_sample_idx++) {
    current_output_sample = 0;
    // Perform convolution
    for (coeff_idx = 0; coeff_idx < param_3_filter_len; coeff_idx++) {
      // Coeffs are used in reverse order: param_2_coeffs[param_3_filter_len - 1 - coeff_idx]
      // History is accessed from param_4_history[output_sample_idx] to param_4_history[output_sample_idx + param_3_filter_len - 1]
      current_output_sample += (int16_t)(((int32_t)param_2_coeffs[param_3_filter_len - 1 - coeff_idx] *
                                          (int32_t)param_4_history[coeff_idx + output_sample_idx]) >> 16);
    }
    param_1_samples[output_sample_idx] = current_output_sample; // Store result back into param_1_samples
  }
  // Shift history buffer: move (filter_length - 1) samples from
  // (param_4_history + 80) to param_4_history (discarding the first 80 samples)
  memmove(param_4_history, param_4_history + 80, (param_3_filter_len - 1) * sizeof(int16_t));
  return;
}

// Function: bandpass_filter
void bandpass_filter(int16_t *param_1) {
  filter(param_1, BANDPASS_COEFFS, 0x11, history_8); // 0x11 = 17
  return;
}

// Function: lowpass_filter
void lowpass_filter(int16_t *param_1) {
  filter(param_1, LOWPASS_COEFFS, 0x2d, history_7); // 0x2d = 45
  return;
}

// Function: convolution
void convolution(int16_t *param_1) {
  uint32_t i;
  
  // Copy 80 shorts from param_1 into buf_6 starting at index 9
  memcpy(buf_6 + 9, param_1, 80 * sizeof(int16_t));
  
  for (i = 0; i < 80; i++) { // 0x50 = 80 output shorts
    param_1[i] = (int16_t)(((int32_t)buf_6[i + 9] * (int32_t)buf_6[i]) >> 16);
  }
  // Shift buf_6: move 9 shorts from buf_6 + 80 to buf_6
  memmove(buf_6, buf_6 + 80, 9 * sizeof(int16_t));
  return;
}

// Function: modem_decode
void modem_decode(Buffer *param_1_input_buffer, Buffer *param_2_output_buffer) {
  unsigned char read_byte;
  int16_t samples[80];
  uint32_t i;
  int32_t max_absolute_val = 0;
  uint32_t max_absolute_idx = 0;
  
  for (i = 0; i < 80; i++) {
    buffer_read_bytes(param_1_input_buffer, &read_byte, 1);
    samples[i] = (int16_t)(comp_decode(read_byte) << 2);
  }
  bandpass_filter(samples);
  convolution(samples);
  lowpass_filter(samples);
  
  if (phase_5 == -1) { // 0xffffffff
    for (i = 0; i < 80; i++) {
      if (abs(samples[i]) > max_absolute_val) {
        max_absolute_val = abs(samples[i]);
        max_absolute_idx = i;
      }
    }
    phase_5 = max_absolute_idx % 10;
  }
  
  uint32_t sum_of_samples;
  for (i = phase_5; i < 80; i += 20) { // 0x50 = 80, 0x14 = 20
    sum_of_samples = 0;
    for (int j = 0; j < 5; j++) {
      sum_of_samples += samples[i + j];
    }
    sum_of_samples /= 5;
    buffer_write_bit(param_2_output_buffer, sum_of_samples >> 31); // Get the sign bit
  }
  return;
}

// Function: modem_encode
void modem_encode(Buffer *param_1_input_buffer, Buffer *param_2_output_buffer) {
  unsigned char encoded_byte;
  int bit_value;
  
  while( (bit_value = buffer_read_bit(param_1_input_buffer)) != -1 ) {
    for (int i = 0; i < 20; i++) { // 0x14 = 20 iterations
      encoded_byte = _modem_encode_returns_char(bit_value);
      buffer_write_bytes(param_2_output_buffer, &encoded_byte, 1);
    }
  }
  return;
}

// Function: modem_output
void modem_output(unsigned char *param_1_data_ptr, int param_2_len_bytes) {
  Buffer local_buffer;
  
  buffer_init(&local_buffer, param_1_data_ptr, param_2_len_bytes + 1);
  buffer_write_seek(&local_buffer, (uint32_t)param_2_len_bytes << 3, 0); // Seek by bits
  
  while( buffer_read_remaining(&local_buffer) != 0 ) {
    frame_encode(&local_buffer, &g_frames);
  }
  return;
}

// Function: modem_encode_frame
void modem_encode_frame(Buffer *param_1_output_buffer) {
  int64_t remaining_g_frames_bits;
  Buffer local_frame_buffer;
  unsigned char local_data_byte;
  unsigned char local_frame_data[15]; // 0xf = 15 bytes capacity
  
  remaining_g_frames_bits = buffer_read_remaining(&g_frames);
  
  // Check if remaining_g_frames_bits is greater than 0x6f (111 bits)
  if (remaining_g_frames_bits > 0x6f) {
    buffer_init(&local_frame_buffer, local_frame_data, 0xf); // 15 bytes capacity
    buffer_write_bits(&local_frame_buffer, 0x13, 5); // Write 5 bits of value 0x13
    for (uint32_t i = 0; i < 0xe; i++) { // 0xe = 14 iterations
      buffer_read_bytes(&g_frames, &local_data_byte, 1);
      buffer_write_bytes(&local_frame_buffer, &local_data_byte, 1);
    }
    modem_encode(&local_frame_buffer, param_1_output_buffer);
  }
  return;
}

// Function: modem_init
void modem_init(void) {
  ecc_init();
  buffer_init(&g_frames, g_frames_data, 0x2000); // 8192 bytes
  return;
}

// Function: get_byte
int get_byte(unsigned char *param_1_byte) {
  int bytes_read_count;
  
  if (buf_offset == buf_read) {
    int ret = receive(0, buf, 0x400, &bytes_read_count); // Read from stdin (fd 0)
    if (ret != 0 || bytes_read_count == 0) {
      return -1; // 0xffffffff
    }
    buf_read = bytes_read_count;
    buf_offset = 0;
  }
  *param_1_byte = buf[buf_offset];
  buf_offset++;
  return 0;
}

// Function: ready_to_read
int ready_to_read(void) {
  fd_set read_fds;
  struct timeval timeout;
  int select_ret;
  
  if (buf_offset < buf_read) {
    return 1; // Data already in buffer
  }
  
  FD_ZERO(&read_fds);
  FD_SET(0, &read_fds); // Check stdin (fd 0)
  
  timeout.tv_sec = 0;
  timeout.tv_usec = 1; // 1 microsecond timeout
  
  select_ret = select(1, &read_fds, NULL, NULL, &timeout); // Check fd 0, max_fd + 1 = 1
  
  if (select_ret == -1) {
    exit(1);
  }
  
  if (select_ret == 0) { // Timeout, no data ready
    return 0;
  }
  
  // If select_ret > 0, then fd 0 must be ready (since it's the only one checked)
  return FD_ISSET(0, &read_fds) ? 1 : 0;
}

// Function: send_n_bytes
uint32_t send_n_bytes(int fd, uint32_t count, const unsigned char *data) {
  int bytes_written_count;
  uint32_t total_written = 0;
  
  if (count == 0 || data == NULL) {
    return (uint32_t)-1; // 0xffffffff
  }
  
  while (total_written < count) {
    int ret = transmit(fd, data + total_written, count - total_written, &bytes_written_count);
    if (ret != 0) {
      exit(1);
    }
    if (bytes_written_count == 0) { // No bytes written, non-blocking might return 0
      return total_written;
    }
    total_written += bytes_written_count;
  }
  return total_written;
}

// Function: flush_output
void flush_output(void) {
  if (out_buf_len != 0) {
    send_n_bytes(1, out_buf_len, out_buf); // fd 1 is stdout
    out_buf_len = 0;
  }
  return;
}

// Function: write_byte
void write_byte(unsigned char param_1_byte) {
  out_buf[out_buf_len] = param_1_byte;
  out_buf_len++;
  if (out_buf_len > 0x3ff) { // If buffer is full (1024 bytes), flush
    flush_output();
  }
  return;
}

// Function: modem_loop
void modem_loop(void (*param_1_callback)(void*, uint32_t)) {
  unsigned char byte_read;
  int get_byte_status;
  
  Buffer in_buffer;
  Buffer decode_buffer;
  Buffer packet_buffer;
  Buffer output_buffer;

  uint32_t local_10_state = 0; // State variable for bit processing and frame decode
  int32_t decode_buffer_offset_bits = 0; // Snapshot of decode_buffer read offset
  int32_t decode_buffer_whence_bits = 0; // Snapshot of decode_buffer read whence

  buffer_init(&in_buffer, indata_3, 0x400);
  buffer_init(&decode_buffer, decdata_2, 0x200);
  buffer_init(&packet_buffer, pktdata_1, 0x400);
  buffer_init(&output_buffer, outdata_0, 0x8000);

  while (true) {
    // Check for input readiness and flush output
    if (ready_to_read() == 0) {
      flush_output();
    }
    
    // Read a byte from input
    get_byte_status = get_byte(&byte_read);
    if (get_byte_status != 0) {
      return; // Exit modem_loop on input error
    }
    // local_14 incremented in original, but unused. Removed.
    buffer_write_bytes(&in_buffer, &byte_read, 1);

    int64_t remaining_in_in_buffer = buffer_read_remaining(&in_buffer);
    bool should_process_decode_block = false;

    // Check if enough data for modem_decode (0x27f bits = 63.875 bytes. So, at least 64 bytes needed)
    if (remaining_in_in_buffer > 0x27f) {
      modem_decode(&in_buffer, &decode_buffer);
      should_process_decode_block = true;
    }

    // This loop structure replaces the `goto switchD_000138f8_caseD_5`
    while (should_process_decode_block) {
        bool exit_bit_processing_loop = false;

        // This inner loop processes bits until local_10_state reaches 5
        while (local_10_state != 5) {
            int64_t remaining_in_decode_buffer = buffer_read_remaining(&decode_buffer);
            // Check if enough bits in decode_buffer (assuming buffer_read_bit needs at least 8 bits/1 byte)
            if (remaining_in_decode_buffer < 8) {
                exit_bit_processing_loop = true;
                break;
            }
            
            // Read a bit
            int bit_val = buffer_read_bit(&decode_buffer);
            if (bit_val == -1) { // Error or EOF from buffer_read_bit
                exit_bit_processing_loop = true;
                break;
            }

            if (local_10_state < 5) {
                // Call function from jump table
                if (switch_funcs_global[local_10_state] != NULL) {
                    switch_funcs_global[local_10_state]();
                }
                return; // Original code returns after indirect call.
            }
        }

        if (exit_bit_processing_loop) {
            should_process_decode_block = false; // Not enough bits, exit this block
            break; // Break from `while (should_process_decode_block)` to go to common processing
        }

        // If local_10_state == 5, proceed to frame_decode block
        int64_t remaining_in_decode_buffer_for_frame = buffer_read_remaining(&decode_buffer);
        // Check if enough data for frame_decode (0x70 bits = 14 bytes)
        if (remaining_in_decode_buffer_for_frame < 0x70) {
            should_process_decode_block = false; // Not enough data for frame_decode, exit this block
            break; // Break from `while (should_process_decode_block)` to go to common processing
        }

        int frame_decode_result = frame_decode(&decode_buffer, &packet_buffer);
        if (frame_decode_result == 0) { // Frame decode failed or incomplete
            local_10_state = 0; // Reset state
            // Revert buffer read position
            buffer_read_seek(&decode_buffer, decode_buffer_offset_bits, decode_buffer_whence_bits);
            // This was a `goto switchD_000138f8_caseD_5;`. By keeping `should_process_decode_block` true,
            // the outer loop repeats, re-evaluating `while (local_10_state != 5)`.
            continue;
        }
        if (frame_decode_result == 2) { // Frame successfully decoded
            int64_t packet_data_len_bits = buffer_read_remaining(&packet_buffer);
            // param_1_callback expects data pointer and length in bytes
            param_1_callback(pktdata_1, (uint32_t)(packet_data_len_bits / 8));
            buffer_init(&packet_buffer, pktdata_1, 0x400); // Re-initialize packet buffer
        }
        local_10_state = 0; // Reset state after successful frame processing
        should_process_decode_block = false; // Exit this block after processing a frame
    } // End of while (should_process_decode_block)

    // Common processing (corresponds to LAB_00013a4c)
    int64_t remaining_in_outdata = buffer_read_remaining(&output_buffer);
    if (remaining_in_outdata == 0) {
      modem_encode_frame(&output_buffer);
    }
    
    // Re-check remaining after modem_encode_frame might have added data
    if (buffer_read_remaining(&output_buffer) == 0) {
      byte_read = _modem_encode_returns_char(1);
    } else {
      buffer_read_bytes(&output_buffer, &byte_read, 1);
    }
    write_byte(byte_read);
  }
}