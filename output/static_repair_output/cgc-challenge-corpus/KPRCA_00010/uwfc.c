#include <stdio.h>    // For printf
#include <string.h>   // For memcmp, memset
#include <stdlib.h>   // For malloc, free
#include <math.h>     // For sin, cos, sqrt, pow, log, round

// Assume ROUND is `round()` from math.h
#define ROUND round

// Decompiler-specific types and constants
// Replaced with standard C types
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char byte;

// Global variables (placeholders for original DAT_XXXX addresses)
// The actual values are unknown from the snippet, so common/sensible defaults are used.
const char DAT_0001530b[4] = {'R', 'I', 'F', 'F'}; // "RIFF"
const char DAT_00015310[4] = {'W', 'A', 'V', 'E'}; // "WAVE"
const char DAT_00015315[4] = {'f', 'm', 't', ' '}; // "fmt "
const char DAT_0001531a[4] = {'d', 'a', 't', 'a'}; // "data"

// Assuming these are newline characters or similar separators for printing
const char *DAT_0001531f = "\n";
const char *DAT_0001533f = "\n";
const char *DAT_0001535b = "\n";

// Double constants used in calculations
const double DAT_00015360 = 128.0;   // Normalization factor for 8-bit samples (2^7)
const double DAT_00015368 = 32768.0; // Normalization factor for 16-bit samples (2^15)
const double DAT_00015370 = 0.0;     // Plot offset for mono wave (can be adjusted)
const double DAT_00015378 = 0.0;     // Plot offset for stereo wave (can be adjusted)
const double DAT_00015380 = 12.0;    // Scaling factor for frequency bucket index
const double DAT_00015388 = 2.0;     // Exponent for power calculation (e.g., squaring magnitude)
const double DAT_00015390 = 0.0001;  // Threshold for adding to bucket (noise gate)
const double DAT_00015398 = 10.0;    // Scaling factor for log in vis_buckets

// Global variable vis_multiplier
int vis_multiplier = 1; // Default value

// External function declarations (prototypes)
// These functions are used but not defined in the snippet.
// Their exact signatures are inferred from usage.
// `receive`: Assumed to be a socket/file descriptor read function.
int receive(int fd, void *buf, size_t len, int *bytes_received);
// `dft`: Assumed to be a Discrete Fourier Transform function.
// It takes a double array, number of samples, and an error code pointer, returning a pointer to results.
void *dft(double *input, unsigned int num_samples, int *error_code);

// Function: set_vis_multiplier
void set_vis_multiplier(char param_1) {
  if ((param_1 < ':') && ('/' < param_1)) { // Checks if param_1 is a digit '0'-'9'
    vis_multiplier = param_1 - '0';       // Convert character digit to integer
    printf("Multiplier set to %d\n", vis_multiplier);
  } else {
    printf("Bad multiplier argument\n");
  }
}

// Function: get_vis_multiplier
int get_vis_multiplier(void) {
  return vis_multiplier;
}

// Function: validate_header
int validate_header(void *param_1) {
  char *header = (char *)param_1; // Cast to char* for byte-wise pointer arithmetic
  int result = -2;                 // Default error value if first memcmp fails

  if (memcmp(header, DAT_0001530b, 4) == 0) { // Check for "RIFF"
    // Check ChunkSize: (DataSize + Subchunk1Size + 20) == TotalChunkSize
    // *(int *)(header + 4) is ChunkSize
    // *(int *)(header + 0x28) is DataSize
    // *(int *)(header + 0x10) is Subchunk1Size
    if (*(int *)(header + 4) == *(int *)(header + 0x28) + *(int *)(header + 0x10) + 0x14) {
      if (memcmp(header + 8, DAT_00015310, 4) == 0) { // Check for "WAVE"
        if (memcmp(header + 0xc, DAT_00015315, 4) == 0) { // Check for "fmt "
          if (*(short *)(header + 0x14) == 1) { // Check AudioFormat == 1 (PCM)
            short numChannels = *(short *)(header + 0x16);
            if ((numChannels == 1) || (numChannels == 2)) { // Check NumChannels (1 or 2)
              int sampleRate = *(int *)(header + 0x18);
              // Check SampleRate (common values)
              if (sampleRate == 8000 || sampleRate == 11025 || sampleRate == 16000 ||
                  sampleRate == 22050 || sampleRate == 32000 || sampleRate == 44100 ||
                  sampleRate == 48000 || sampleRate == 4000) {
                ushort bitsPerSample = *(ushort *)(header + 0x22);
                uint expectedByteRate = (uint)bitsPerSample * sampleRate * numChannels / 8;

                if (*(uint *)(header + 0x1c) == expectedByteRate) { // Check ByteRate
                  uint expectedBlockAlign = (uint)bitsPerSample * numChannels / 8;
                  // The original `if (iVar1 < 0) { iVar1 = iVar1 + 7; }` was a decompiler artifact for
                  // signed integer math or ceiling division, not applicable here due to unsigned types.
                  if (*(ushort *)(header + 0x20) == expectedBlockAlign) { // Check BlockAlign
                    if ((bitsPerSample == 8) || (bitsPerSample == 0x10)) { // Check BitsPerSample (8 or 16)
                      if (memcmp(header + 0x24, DAT_0001531a, 4) == 0) { // Check for "data"
                        result = *(int *)(header + 4) + 8; // Valid, return total size (ChunkSize + 8 for RIFF header)
                      } else {
                        result = -0x800; // "data" chunk missing or invalid
                      }
                    } else {
                      result = -0x400; // Unsupported BitsPerSample
                    }
                  } else {
                    result = -0x100; // BlockAlign mismatch
                  }
                } else {
                  result = -0x100; // ByteRate mismatch
                }
              } else {
                result = -0x80; // Unsupported SampleRate
              }
            } else {
              result = -0x40; // Unsupported NumChannels
            }
          } else {
            result = -0x20; // Unsupported AudioFormat
          }
        } else {
          result = -0x10; // "fmt " chunk missing or invalid
        }
      } else {
        result = -8; // "WAVE" chunk missing or invalid
      }
    } else {
      result = -4; // ChunkSize mismatch
    }
  }
  return result;
}

// Function: init_track
void *init_track(void) {
  void *header_buf = NULL;
  int bytes_received_chunk = 0;
  size_t data_chunk_size = 0;
  void *data_buf = NULL;
  int bytes_read_into_data = 0;

  header_buf = malloc(0x30); // Allocate for header (RIFF, WAVE, fmt, data chunk headers are 44 bytes, plus 4 bytes for data pointer = 48 bytes)
  if (header_buf == NULL) {
    return NULL;
  }

  // Receive initial header (0x2c = 44 bytes)
  if (receive(0, header_buf, 0x2c, &bytes_received_chunk) != 0 || bytes_received_chunk != 0x2c) {
    free(header_buf);
    return NULL;
  }

  int validation_result = validate_header(header_buf);
  data_chunk_size = validation_result - 0x2c; // Total data size reported by header - size of initial header read

  if ((int)data_chunk_size < 0) { // If header validation failed or data size is negative
    free(header_buf);
    return NULL;
  }

  // Store data buffer pointer at offset 0x2c in header_buf, or NULL if no data chunk
  if (data_chunk_size == 0) {
    *(void **)((char *)header_buf + 0x2c) = NULL;
  } else {
    data_buf = malloc(data_chunk_size);
    *(void **)((char *)header_buf + 0x2c) = data_buf; // Store pointer to data_buf in header_buf
    if (data_buf == NULL) {
      free(header_buf);
      return NULL;
    }
  }

  // Read the data chunk if it exists
  int data_expected_size = *(int *)((char *)header_buf + 0x28); // From header, total data size
  while (bytes_read_into_data < data_expected_size) {
    int bytes_to_read = data_expected_size - bytes_read_into_data;
    // Cast data_buf to char* for correct byte-wise pointer arithmetic
    if (receive(0, (char *)data_buf + bytes_read_into_data, bytes_to_read, &bytes_received_chunk) != 0) {
      // Error receiving data
      if (data_buf != NULL) free(data_buf);
      free(header_buf);
      return NULL;
    }

    if (bytes_received_chunk == 0) { // No data received, but not yet at expected size
      // This indicates a premature end of stream or error
      if (data_buf != NULL) free(data_buf);
      free(header_buf);
      return NULL;
    }
    bytes_read_into_data += bytes_received_chunk;
  }

  return header_buf; // Return the header buffer which now contains a pointer to the audio data
}

// Function: clear_track
void clear_track(void **param_1) {
  void *track_header = *param_1;

  if (track_header != NULL) {
    void *data_chunk_ptr = *(void **)((char *)track_header + 0x2c); // Get pointer to data chunk
    if (data_chunk_ptr != NULL) {
      free(data_chunk_ptr);
    }
    free(track_header);
    *param_1 = NULL; // Set the caller's pointer to NULL after freeing
  }
}

// Function: slow_vis
void slow_vis(void) {
  for (int i = 0; i < vis_multiplier * 200000; i++) {
    sin((double)i); // Perform some dummy floating-point operations to "slow" down
    cos((double)i);
  }
}

// Function: plot_mono_wave
void plot_mono_wave(void *param_1, short *param_2) {
  char *header = (char *)param_1; // Cast param_1 to char* to access header fields
  double sample_value_normalized;
  int sample_value_int;

  if (*(short *)(header + 0x22) == 8) { // 8-bit samples
    sample_value_int = *(byte *)param_2 - 0x7f; // Convert unsigned 8-bit to signed range [-127, 128]
    sample_value_normalized = (double)sample_value_int / DAT_00015360; // Normalize to approx [-1.0, 1.0]
  } else { // 16-bit samples
    sample_value_int = *param_2; // Sample is already signed short
    sample_value_normalized = (double)sample_value_int / DAT_00015368; // Normalize to approx [-1.0, 1.0]
  }

  const int buffer_len = 42; // Corresponds to original 0x2a, length of the display buffer
  char display_buffer[buffer_len]; // No +1 for null terminator, as original sets `[len-1] = 0`

  // Calculate plot position: maps normalized sample value from [-1, 1] to [0, 40]
  int plot_pos = (int)ROUND(DAT_00015370 + sample_value_normalized * 20.0 + 20.0);

  memset(display_buffer, ' ', buffer_len); // Fill buffer with spaces
  display_buffer[buffer_len - 1] = '\0';   // Null-terminate the string at the last character position

  // Place '*' at the calculated plot position
  if (plot_pos >= 0 && plot_pos < buffer_len -1) { // Ensure plot_pos is within printable bounds
      display_buffer[plot_pos] = '*';
  }

  printf("%s", display_buffer); // Print the wave form
  printf(DAT_0001531f);         // Print newline (or whatever DAT_0001531f is)
}

// Function: plot_stereo_wave
void plot_stereo_wave(void *param_1, short *param_2) {
  char *header = (char *)param_1; // Cast param_1 to char* to access header fields
  double sample_left_normalized, sample_right_normalized;
  int sample_left_int, sample_right_int;

  if (*(short *)(header + 0x22) == 8) { // 8-bit samples
    sample_left_int = *(byte *)param_2 - 0x7f;
    sample_left_normalized = (double)sample_left_int / DAT_00015360;
    sample_right_int = *(byte *)((char *)param_2 + 1) - 0x7f; // Access next byte for right channel
    sample_right_normalized = (double)sample_right_int / DAT_00015360;
  } else { // 16-bit samples
    sample_left_int = *param_2;
    sample_left_normalized = (double)sample_left_int / DAT_00015368;
    sample_right_int = param_2[1]; // Access next short for right channel
    sample_right_normalized = (double)sample_right_int / DAT_00015368;
  }

  const int buffer_len = 83; // Corresponds to original 0x53, length of the display buffer
  char display_buffer[buffer_len];

  // Calculate plot positions for left and right channels
  // Left channel maps [-1, 1] to [0, 38]
  int plot_pos_left = (int)ROUND(DAT_00015370 + sample_left_normalized * 19.0 + 19.0);
  // Right channel maps [-1, 1] to [0, 38] then shifts by 41 to separate visually
  int plot_pos_right = (int)ROUND(DAT_00015378 + sample_right_normalized * 19.0 + 19.0 + 41.0);

  memset(display_buffer, ' ', buffer_len); // Fill buffer with spaces
  display_buffer[buffer_len - 1] = '\0';   // Null-terminate

  // Place center line character '|' (0x7c) at index 41 (0x29)
  if (0x29 >= 0 && 0x29 < buffer_len - 1) {
      display_buffer[0x29] = '|';
  }

  // Place '*' (0x2a) for left channel
  if (plot_pos_left >= 0 && plot_pos_left < buffer_len - 1) {
      display_buffer[plot_pos_left] = '*';
  }
  // Place '*' (0x2a) for right channel
  if (plot_pos_right >= 0 && plot_pos_right < buffer_len - 1) {
      display_buffer[plot_pos_right] = '*';
  }

  printf("%s", display_buffer); // Print the waveform
  printf(DAT_0001531f);         // Print newline
}

// Function: wave_vis
void wave_vis(void *param_1) {
  char *header = (char *)param_1;
  char *current_sample_ptr = *(char **)(header + 0x2c); // Pointer to the start of audio data
  uint data_size = *(uint *)(header + 0x28);             // Size of audio data in bytes
  ushort block_align = *(ushort *)(header + 0x20);       // Bytes per sample frame (BlockAlign)
  uint sample_rate = *(uint *)(header + 0x18);           // Sample rate for slow_vis check
  short num_channels = *(short *)(header + 0x16);        // Number of channels

  // Iterate through sample frames
  // data_size / block_align gives the total number of sample frames
  for (uint i = 0; i < data_size / block_align; i++) {
    // Call slow_vis periodically (e.g., once per second if i is sample index and sample_rate is samples/second)
    if (i % sample_rate == 0) {
      slow_vis();
    }

    // Plot based on number of channels
    if (num_channels == 2) {
      plot_stereo_wave(param_1, (short *)current_sample_ptr);
    } else if (num_channels == 1) {
      plot_mono_wave(param_1, (short *)current_sample_ptr);
    }
    current_sample_ptr += block_align; // Move to the next sample frame
  }
  printf("++++END VISUALIZATION++++\n\n");
}

// Function: add_to_bucket
void add_to_bucket(void *bucket_array_ptr, double *dft_result_pair, int current_freq_bin, int total_freq_bins) {
  // `bucket_array_ptr` points to an array of structs, each 0x14 (20) bytes long.
  // Each struct is likely: { double sum_magn_sq, double max_magn_sq, int count }
  // `dft_result_pair` points to a (real, imaginary) pair of doubles.

  double freq_ratio = (double)current_freq_bin / (double)total_freq_bins;
  // Calculate bucket index using sqrt scaling (common for frequency visualizations)
  int bucket_index = (int)ROUND(DAT_00015380 * sqrt(freq_ratio));

  // Clamp bucket_index to 0-11 for 12 buckets
  if (bucket_index < 0) bucket_index = 0;
  if (bucket_index >= 12) bucket_index = 11;

  // Pointers to the specific bucket element's fields
  // Cast to char* for byte arithmetic, then to appropriate type pointers
  double *sum_magn_sq_ptr = (double *)((char *)bucket_array_ptr + bucket_index * 0x14);
  double *max_magn_sq_ptr = sum_magn_sq_ptr + 1; // Second double in the struct
  int *count_ptr = (int *)(sum_magn_sq_ptr + 2); // Integer count in the struct

  // Calculate magnitude squared of DFT result (real^2 + imag^2)
  double magnitude_sq = pow(dft_result_pair[0], DAT_00015388) + pow(dft_result_pair[1], DAT_00015388);

  if (magnitude_sq > DAT_00015390) { // Check against a noise threshold
    *sum_magn_sq_ptr += magnitude_sq; // Accumulate magnitude squared
  }
  if (*max_magn_sq_ptr < magnitude_sq) {
    *max_magn_sq_ptr = magnitude_sq; // Update max magnitude squared
  }
  *count_ptr += 1; // Increment count for this bucket
}

// Function: vis_buckets
void vis_buckets(void *param_1, int param_2) {
  char *bucket_array = (char *)param_1; // Cast for pointer arithmetic

  slow_vis();

  if (param_2 == 1) { // Type 1 visualization (bar graph for each bucket)
    const int max_bar_height = 90; // Corresponds to original 0x5a
    const int num_buckets = 12;

    for (int bar_height = max_bar_height; bar_height >= -1; bar_height--) { // Loop from max height down
      printf(DAT_0001533f); // Print newline (or separator)
      for (int i = 0; i < num_buckets; i++) {
        if (bar_height == -1) { // This corresponds to printing the base line
          printf("|-----|");
        } else {
          // Access the second double in the bucket struct (max_magnitude_sq)
          double val = *(double *)(bucket_array + i * 0x14 + 8);
          // Use log scale for visualization, handle log(0)
          double log_val = (val > 0) ? log(val) : log(DAT_00015390); // Use threshold if val is zero or negative
          int current_bar_level = (int)ROUND(log_val);

          if (current_bar_level >= bar_height) {
            printf("|=====|"); // Filled part of the bar
          } else {
            printf("|     |"); // Empty part of the bar
          }
        }
      }
      printf(DAT_0001535b); // Print newline (or separator)
    }
  } else if (param_2 == 2) { // Type 2 visualization (single bar for total energy)
    double total_energy = 0.0;
    const int num_buckets = 12;
    const int buffer_size = 91; // Corresponds to original 0x5a + 1
    char display_buffer[buffer_size];

    for (int i = 0; i < num_buckets; i++) {
      total_energy += *(double *)(bucket_array + i * 0x14); // Sum first double (sum_magn_sq)
    }

    // Use log scale for total energy visualization
    double log_total_energy = log(total_energy + 1.0); // Add 1.0 to avoid log(0)
    int plot_length = (int)ROUND(DAT_00015398 * log_total_energy); // Scale by DAT_00015398

    // Clamp plot_length to valid buffer range
    if (plot_length < 0) plot_length = 0;
    if (plot_length >= buffer_size - 1) plot_length = buffer_size - 1; // Leave space for ']' and '\0'

    // Fill the bar with '=' characters
    memset(display_buffer, '=', plot_length);
    // Fill the rest of the buffer with spaces
    if (plot_length < buffer_size - 1) {
        memset(display_buffer + plot_length, ' ', (buffer_size - 1) - plot_length);
    }
    // Place ']' at the end of the bar segment (or at the beginning if length is 0)
    display_buffer[plot_length] = ']';
    display_buffer[buffer_size - 1] = '\0'; // Null-terminate the string

    // Original code printed the bar twice
    for (int i = 0; i < 2; i++) {
      printf("%s", display_buffer);
      printf(DAT_0001531f); // Print newline
    }
  }
}

// Function: eq_vis
void eq_vis(void *param_1, int param_2, int param_3) {
  char *header = (char *)param_1;

  // `bucket_data` holds 12 buckets, each 20 bytes (double sum, double max, int count)
  char bucket_data[12 * 20];

  double *mono_samples_buffer = NULL;
  double *stereo_samples_buffer = NULL; // For right channel if stereo
  double *dft_result_mono = NULL;
  double *dft_result_stereo = NULL;

  short *current_audio_ptr = *(short **)(header + 0x2c); // Pointer to start of audio data
  uint data_size = *(uint *)(header + 0x28);
  ushort block_align = *(ushort *)(header + 0x20);
  uint sample_rate = *(uint *)(header + 0x18); // Used as DFT window size
  short num_channels = *(short *)(header + 0x16);
  ushort bits_per_sample = *(ushort *)(header + 0x22);

  // Allocate buffers for DFT input (double arrays for samples)
  mono_samples_buffer = (double *)malloc(sample_rate * sizeof(double));
  if (mono_samples_buffer == NULL) {
    return; // Failed to allocate
  }

  if (num_channels == 2) {
    stereo_samples_buffer = (double *)malloc(sample_rate * sizeof(double));
    if (stereo_samples_buffer == NULL) {
      free(mono_samples_buffer);
      return; // Failed to allocate
    }
  }

  uint current_sample_frame_idx = 0;
  // Loop through all sample frames in the audio data, processing in windows of `sample_rate` size
  while (current_sample_frame_idx < data_size / block_align) {
    // Initialize buckets for this frame window
    for (int i = 0; i < 12; i++) {
      *(double *)(bucket_data + i * 0x14) = 0.0;     // sum_magnitude_sq
      *(double *)(bucket_data + i * 0x14 + 8) = 0.0; // max_magnitude_sq
      *(int *)(bucket_data + i * 0x14 + 16) = 0;     // count
    }

    uint samples_in_window = 0;
    // Collect samples for one DFT window (size `sample_rate`)
    while (samples_in_window < sample_rate &&
           current_sample_frame_idx < data_size / block_align) {

      double normalized_sample_left, normalized_sample_right;

      if (bits_per_sample == 8) {
        normalized_sample_left = (double)(*(byte *)current_audio_ptr - 0x7f) / DAT_00015360;
        if (num_channels == 2) {
          normalized_sample_right = (double)(*(byte *)((char *)current_audio_ptr + 1) - 0x7f) / DAT_00015360;
        }
      } else { // 16-bit samples
        normalized_sample_left = (double)*current_audio_ptr / DAT_00015368;
        if (num_channels == 2) {
          normalized_sample_right = (double)current_audio_ptr[1] / DAT_00015368;
        }
      }

      mono_samples_buffer[samples_in_window] = normalized_sample_left;
      if (num_channels == 2) {
        stereo_samples_buffer[samples_in_window] = normalized_sample_right;
      }

      samples_in_window++;
      current_sample_frame_idx++;
      current_audio_ptr = (short *)((char *)current_audio_ptr + block_align);
    }

    int dft_error_code = 0;
    dft_result_mono = (double *)dft(mono_samples_buffer, samples_in_window, &dft_error_code);
    if (dft_result_mono == NULL) { /* Handle DFT error */ goto cleanup_buffers; }

    if (num_channels == 2) {
      dft_result_stereo = (double *)dft(stereo_samples_buffer, samples_in_window, &dft_error_code);
      if (dft_result_stereo == NULL) { /* Handle DFT error */ free(dft_result_mono); goto cleanup_buffers; }
    }

    // Apply frequency filtering based on param_3
    // DFT result is an array of complex numbers (real, imag doubles). Each complex number is 16 bytes.
    // `samples_in_window / 2` represents the number of unique frequency bins.
    if (param_3 == 1) { // High-pass filter
      // Zero out low frequencies. Thresholds inferred from original code's shifting and division.
      // (sample_rate * 4) / 20 is an example cutoff.
      uint cutoff_idx_low = (sample_rate * 4) / 20;
      for (uint i = 0; i < samples_in_window / 2; i++) {
        if (i < cutoff_idx_low) {
          dft_result_mono[i * 2] = 0.0;     // Real part
          dft_result_mono[i * 2 + 1] = 0.0; // Imaginary part
          if (num_channels == 2) {
            dft_result_stereo[i * 2] = 0.0;
            dft_result_stereo[i * 2 + 1] = 0.0;
          }
        }
      }
    } else if (param_3 == 2) { // Low-pass filter
      // Zero out high frequencies.
      // (sample_rate * 2) / 20 is an example cutoff.
      uint cutoff_idx_high = (sample_rate * 2) / 20;
      for (uint i = 0; i < samples_in_window / 2; i++) {
        if (i >= cutoff_idx_high) {
          dft_result_mono[i * 2] = 0.0;
          dft_result_mono[i * 2 + 1] = 0.0;
          if (num_channels == 2) {
            dft_result_stereo[i * 2] = 0.0;
            dft_result_stereo[i * 2 + 1] = 0.0;
          }
        }
      }
    }

    // Add DFT results to frequency buckets
    for (uint i = 0; i < samples_in_window / 2; i++) {
      add_to_bucket(bucket_data, dft_result_mono + i * 2, i, samples_in_window / 2);
      if (num_channels == 2) {
        add_to_bucket(bucket_data, dft_result_stereo + i * 2, i, samples_in_window / 2);
      }
    }

    vis_buckets(bucket_data, param_2); // Pass the bucket array for visualization

    free(dft_result_mono);
    dft_result_mono = NULL; // Clear pointer after freeing
    if (num_channels == 2) {
      free(dft_result_stereo);
      dft_result_stereo = NULL; // Clear pointer after freeing
    }
  }

  printf("++++END VISUALIZATION++++\n\n");

cleanup_buffers: // Use a label for cleanup, as original had `return` statements inside loops.
  if (mono_samples_buffer) free(mono_samples_buffer);
  if (stereo_samples_buffer) free(stereo_samples_buffer);
}