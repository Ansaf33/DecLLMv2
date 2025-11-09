#include <stdlib.h>  // For malloc, free, random
#include <string.h>  // For memset, memcpy, strncpy, strlen, strcmp
#include <ctype.h>   // For isalnum, isalpha
#include <stdint.h>  // For uint32_t, uint8_t, uintptr_t
#include <stdbool.h> // For bool

// Define structs based on memory layout analysis from the original code.
// permit_data_t represents the 0x1a (26-byte) structure.
// The offsets are derived from the original code's direct memory access.
typedef struct {
    uint8_t  bytes_0_3[4];    // Bytes 0-3, part of the sum calculation.
    uint8_t  byte_4;          // Byte 4, specifically set to 0x55, also part of the sum.
    uint8_t  bytes_5_6[2];    // Bytes 5-6, also part of the sum.
    uint8_t  checksum_byte;   // Byte 7, stores the calculated checksum.
    char     license_num[10]; // Bytes 8-17. strncpy copies 9 bytes into license_num[0-8].
                              // license_num[9] (at offset 17) serves as a null terminator or padding.
    uint32_t param_2_val;     // Bytes 18-21 (offset 0x12), stores param_2.
    int      param_3_val;     // Bytes 22-25 (offset 0x16), stores param_3.
} permit_data_t; // Total size: 4+1+2+1+10+4+4 = 26 bytes (0x1a).

// permit_ring_t represents the 0x86 (134-byte) structure.
// It contains a count and an array of 5 permit_data_t structures.
typedef struct {
    int count;                  // First 4 bytes, stores the number of permits.
    permit_data_t permits[5];   // Array of 5 permit_data_t structures.
} permit_ring_t; // Total size: sizeof(int) + 5 * sizeof(permit_data_t) = 4 + 5 * 26 = 134 bytes (0x86).

// Function declarations to ensure correct compilation order.
uint32_t _validate_license_number(char *license_num_str);
uint32_t _validate_permit_token(permit_data_t *permit_ptr);
void * permit_new(char *param_1, uint32_t param_2, int param_3);
permit_ring_t * pring_new(int num_permits, uintptr_t source_permits_ptr);
permit_ring_t * pring_refactor(permit_ring_t *ring_ptr);
uint32_t permit_test(permit_data_t *permit_ptr, int param_2, char *param_3);
int pring_test(permit_ring_t *ring_ptr, uintptr_t param_2_array_ptr, char *param_3_str_base);
void destroy_permit(void *param_1);
void destroy_permit_ring(void *param_1);

// Function: _validate_license_number
uint32_t _validate_license_number(char *license_num_str) {
  if (license_num_str == NULL) {
    return 0xFFFFFFFF;
  }

  int digit_count = 0;
  size_t len = strlen(license_num_str);

  if (len < 10) {
    for (uint32_t i = 0; i < len; ++i) {
      if (!isalnum((unsigned char)license_num_str[i])) { // Cast to unsigned char for ctype functions
        return 0xFFFFFFFF;
      }
      if (!isalpha((unsigned char)license_num_str[i])) { // If not alphabetic, it's a digit (since already checked for alnum)
        digit_count++;
      }
    }
    if ((digit_count < 1) || (4 < digit_count)) {
      return 0xFFFFFFFF;
    } else {
      return 0; // Success
    }
  }
  return 0xFFFFFFFF; // Length is 10 or more
}

// Function: permit_new
void * permit_new(char *param_1, uint32_t param_2, int param_3) {
  permit_data_t *p_data = NULL;

  if ((_validate_license_number(param_1) == 0) && (param_3 < 201)) { // 0xc9 is 201
    p_data = (permit_data_t *)malloc(sizeof(permit_data_t));
    if (p_data != NULL) {
      // Initialize all bytes to 0 to ensure consistent state, especially for bytes_0_3 and bytes_5_6
      // that are part of the sum but not explicitly set otherwise. This also null-terminates license_num.
      memset(p_data, 0, sizeof(permit_data_t));

      random(); // Unused return value, but kept as per original snippet.
      p_data->byte_4 = 0x55;

      uint32_t sum_bytes = 0;
      // Sum bytes from offset 0 to 6
      for (uint32_t i = 0; i < 7; ++i) {
        sum_bytes += ((uint8_t *)p_data)[i]; // Accessing raw bytes of the struct for sum
      }
      p_data->checksum_byte = (uint8_t)(sum_bytes + (sum_bytes / 0xab) * 'U');
      strncpy(p_data->license_num, param_1, 9);
      p_data->license_num[9] = '\0'; // Ensure null termination, as strncpy doesn't guarantee it if param_1 is >= 9 chars.
      p_data->param_2_val = param_2;
      p_data->param_3_val = param_3;
    }
  }
  return p_data;
}

// Function: pring_new
permit_ring_t * pring_new(int num_permits, uintptr_t source_permits_ptr) {
  permit_ring_t *ring = (permit_ring_t *)malloc(sizeof(permit_ring_t));
  if (ring != NULL) {
    memset(ring, 0, sizeof(permit_ring_t));

    // The original code copies up to `num_permits`.
    // If `num_permits` is greater than 5, this would lead to out-of-bounds writes.
    // We cap the actual permits copied to the array size (5).
    int actual_permits_to_copy = (num_permits < 5) ? num_permits : 5;

    for (int i = 0; i < actual_permits_to_copy; ++i) {
      memcpy(&(ring->permits[i]), (void *)(source_permits_ptr + (uintptr_t)i * sizeof(permit_data_t)), sizeof(permit_data_t));
    }
    // The original code stores the requested count, even if it exceeds the array capacity.
    // This might lead to issues later if `ring->count` is used to iterate `permits`.
    // Adhering to original behavior for now.
    ring->count = num_permits;
  }
  return ring;
}

// Function: pring_refactor
permit_ring_t * pring_refactor(permit_ring_t *ring_ptr) {
  if (ring_ptr == NULL || ring_ptr->count >= 6) { // Max 5 permits in the ring
    return ring_ptr;
  }

  int original_count = ring_ptr->count;
  int num_removed_permits = 0;
  bool changed_in_last_pass;

  // Loop to repeatedly scan and remove until no more permits are found to be removed.
  // This replaces the goto-based logic by restarting the scan if any permit was removed.
  do {
    changed_in_last_pass = false;
    int current_permit_idx = 0;
    while (current_permit_idx < ring_ptr->count) {
      // Condition to remove: param_3_val < 1
      if (ring_ptr->permits[current_permit_idx].param_3_val < 1) {
        changed_in_last_pass = true;
        num_removed_permits++;

        // Shift elements down, overwriting the removed permit
        for (int shift_idx = current_permit_idx; shift_idx < ring_ptr->count - 1; ++shift_idx) {
          memcpy(&(ring_ptr->permits[shift_idx]),
                 &(ring_ptr->permits[shift_idx + 1]),
                 sizeof(permit_data_t));
        }
        ring_ptr->count--;
        // Do not increment current_permit_idx, as the next permit has shifted into its place
        // and needs to be re-evaluated at the current index.
      } else {
        // This permit is valid, move to the next.
        current_permit_idx++;
      }
    }
  } while (changed_in_last_pass); // Continue if any permit was removed in the last full pass.

  // After all removals, clear the memory of the "removed" permits at the end of the array.
  // This clears `num_removed_permits` slots starting from `ring_ptr->permits[ring_ptr->count]`
  // up to `ring_ptr->permits[original_count - 1]`.
  if (num_removed_permits > 0) {
    memset(&(ring_ptr->permits[ring_ptr->count]), 0, num_removed_permits * sizeof(permit_data_t));
  }

  return ring_ptr;
}

// Function: _validate_permit_token
uint32_t _validate_permit_token(permit_data_t *permit_ptr) {
  if (permit_ptr == NULL) {
    return 0xFFFFFFFF; // Added robustness check
  }

  uint32_t sum_bytes = 0;
  if (permit_ptr->byte_4 == 'U') {
    // Sum bytes from offset 0 to 6
    for (int i = 0; i < 7; ++i) {
      sum_bytes += ((uint8_t *)permit_ptr)[i];
    }
    if (permit_ptr->checksum_byte == (uint8_t)(sum_bytes + (sum_bytes / 0xab) * 'U')) {
      return 0; // Success
    }
  }
  return 0xFFFFFFFF; // Failure
}

// Function: permit_test
uint32_t permit_test(permit_data_t *permit_ptr, int param_2, char *param_3) {
  if ((permit_ptr == NULL) || (param_3 == NULL)) {
    return 0x10;
  }

  uint32_t validation_result = _validate_permit_token(permit_ptr);
  if (validation_result == 0) {
    validation_result = _validate_license_number(permit_ptr->license_num);
    if ((validation_result == 0) && (strcmp(permit_ptr->license_num, param_3) == 0)) {
      if (permit_ptr->param_2_val < 1) {
        return 0x20;
      }
      if (200 < param_2) {
        return 0x50;
      }
      if (param_2 != permit_ptr->param_3_val) {
        return 0x50;
      }
      return 0; // Success
    }
    return 0x30; // License number validation or strcmp failed
  }
  return 0x40; // Permit token validation failed
}

// Function: pring_test
int pring_test(permit_ring_t *ring_ptr, uintptr_t param_2_array_ptr, char *param_3_str_base) {
  if ((ring_ptr == NULL) || (param_2_array_ptr == 0) || (param_3_str_base == NULL)) {
    return 0x10;
  }

  for (int i = 0; i < ring_ptr->count; ++i) {
    int result = permit_test(&(ring_ptr->permits[i]),
                             ((uint32_t *)param_2_array_ptr)[i],
                             param_3_str_base + (uintptr_t)i * 10); // Calculate address for i-th string
    if (result != 0) {
      return result;
    }
  }
  return 0;
}

// Function: destroy_permit
void destroy_permit(void *param_1) {
  if (param_1 != NULL) {
    free(param_1);
  }
  return;
}

// Function: destroy_permit_ring
void destroy_permit_ring(void *param_1) {
  if (param_1 != NULL) {
    free(param_1);
  }
  return;
}