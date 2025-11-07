#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h> // For memset, strcpy

// Type definitions based on typical disassembler output
typedef uint32_t undefined4;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef int16_t marker_t; // Using int16_t for markers like -8, -0x10, -0xf
typedef uint8_t undefined;

// Mock declarations for external functions
int receive_bytes_status(void);
int receive_bytes_data(void *buf, size_t len); // Reads into buf, returns bytes read or -1

void _terminate(void); // Maps to exit(1)

// Swap functions now take arguments
uint16_t intel_swap_short(uint16_t val) { return val; }
uint32_t intel_swap_word(uint32_t val) { return val; }
uint16_t motorola_swap_short(uint16_t val) { return (val >> 8) | (val << 8); }
uint32_t motorola_swap_word(uint32_t val) { return ((val >> 24) & 0x000000ff) | ((val >> 8) & 0x0000ff00) | ((val << 8) & 0x00ff0000) | ((val << 24) & 0xff000000); }

// Global function pointers
uint16_t (*swap_short)(uint16_t);
uint32_t (*swap_word)(uint32_t);

// print_tag_text and print_type should take the value they are printing info about
void print_tag_text(uint16_t tag) {
    switch (tag) {
        case 0x0100: printf("ImageWidth"); break;
        case 0x8825: printf("GPS IFD"); break;
        case 0x8769: printf("EXIF IFD"); break;
        default: printf("Unknown Tag 0x%X", tag); break;
    }
}
void print_type(uint16_t type) {
    switch (type) {
        case 1: printf("BYTE"); break;
        case 2: printf("ASCII"); break;
        case 3: printf("SHORT"); break;
        case 4: printf("LONG"); break;
        default: printf("Unknown Type 0x%X", type); break;
    }
}

// process_gps_ifd and process_xif_ifd arguments inferred from usage
void process_gps_ifd(void *ifd_ptr, int16_t *base_ptr, uint32_t section_size, int section_end) {
    printf("  (Stub) Processing GPS IFD at offset %p\n", ifd_ptr);
}
void process_xif_ifd(void *ifd_ptr, int16_t *base_ptr, uint32_t section_size, int section_end) {
    printf("  (Stub) Processing XIF IFD at offset %p\n", ifd_ptr);
}

// --- Mock Implementations ---
void _terminate(void) {
    printf("Program terminated.\n");
    exit(1);
}

static int receive_stage = 0;
static uint32_t mock_sap1_segment_size = 0; // To keep track of the malloc'd buffer size
static void* mock_sap1_segment_data = NULL; // To keep track of the malloc'd buffer pointer

int receive_bytes_status(void) {
    return 0; // Always successful status
}

int receive_bytes_data(void *buf, size_t len) {
    if (buf == NULL || len == 0) return -1;

    // Simulate different reads based on stage
    switch (receive_stage) {
        case 0: // Read SOM marker
            *(marker_t*)buf = (marker_t)0xfff8; // -8
            receive_stage++;
            break;
        case 1: // Read SAP0 marker (first time)
            *(marker_t*)buf = (marker_t)0xfff0; // -0x10
            receive_stage++;
            break;
        case 2: // Read SAP0 segment size
            *(ushort*)buf = 5; // Example size >= 3
            receive_stage++;
            break;
        case 3: // Read SAP0 segment data (no specific content needed for this mock)
            memset(buf, 0, len);
            receive_stage++;
            break;
        case 4: // Read next marker after SAP0 segment (should be SAP1)
            *(marker_t*)buf = (marker_t)0xfff1; // -0xf
            receive_stage++;
            break;
        case 5: // Read SAP1 segment size
            *(ushort*)buf = 100; // Example size >= 0x10
            mock_sap1_segment_size = *(ushort*)buf;
            receive_stage++;
            break;
        case 6: // Read SAP1 segment data
            memset(buf, 0, len);
            mock_sap1_segment_data = buf; // Store pointer to this buffer for easier mock setup

            // Simulate TIFF header and IFD entries
            if (len >= mock_sap1_segment_size) {
                uint16_t *ptr_u16 = (uint16_t*)((char*)buf + 6); // Offset 6 for byte order
                *(ptr_u16) = 0x4949; // Intel 'II'
                *(ptr_u16 + 1) = 0x002A; // TagMark 0x2A
                *(uint32_t*)(ptr_u16 + 2) = 0x00000008; // Offset to first IFD (8 bytes from ptr_u16, so 14 from buf start)

                // Now setup the IFD entries starting at (char*)buf + 14
                uint16_t *ifd_base = (uint16_t*)((char*)buf + 14);
                *ifd_base = 3; // Number of IFD entries

                // Entry 1: Tag 0x0100 (ImageWidth), Type 3 (SHORT), Count 1, Value 1024
                *(ifd_base + 1) = 0x0100; // Tag
                *(ifd_base + 2) = 0x0003; // Type (SHORT)
                *(uint32_t*)(ifd_base + 3) = 0x00000001; // Count
                *(uint32_t*)(ifd_base + 5) = 0x00000400; // Value (1024)

                // Entry 2: Tag 0x8825 (GPS IFD), Type 4 (LONG), Count 1, ValueOffset to GPS IFD (e.g., at offset 50 from buf start)
                *(ifd_base + 1 + 6) = 0x8825; // Tag (GPS IFD)
                *(ifd_base + 2 + 6) = 0x0004; // Type (LONG)
                *(uint32_t*)(ifd_base + 3 + 6) = 0x00000001; // Count
                *(uint32_t*)(ifd_base + 5 + 6) = 0x00000032; // ValueOffset (50, points to GPS IFD)

                // Entry 3: Tag 0x8769 (EXIF IFD), Type 4 (LONG), Count 1, ValueOffset to EXIF IFD (e.g., at offset 70 from buf start)
                *(ifd_base + 1 + 12) = 0x8769; // Tag (EXIF IFD)
                *(ifd_base + 2 + 12) = 0x0004; // Type (LONG)
                *(uint32_t*)(ifd_base + 3 + 12) = 0x00000001; // Count
                *(uint32_t*)(ifd_base + 5 + 12) = 0x00000046; // ValueOffset (70, points to EXIF IFD)

                // Also, put a sample string for type 2 (ASCII) for testing if such an entry exists
                // Example string at offset 0x60 (96) from buf start
                if (len > 0x60 + strlen("SampleString")) {
                    strcpy((char*)buf + 0x60, "SampleString");
                }
            }
            receive_stage++;
            break;
        default:
            // No more data to read, or handle as needed
            return 0; // Indicate EOF or no more data
    }
    return (int)len;
}

// main function
int main(void) {
  marker_t local_46; // SOM marker
  marker_t local_48; // SAP0/SAP1 marker
  ushort local_4a; // Segment size
  int local_28; // Bytes received status or count
  void *local_2c = NULL; // Allocated memory for segment data
  int16_t *local_30; // Pointer into local_2c for IFD base
  uint32_t local_38; // Offset to first IFD
  intptr_t local_34; // End of section (local_2c + local_4a)
  ushort *local_3c; // Pointer to IFD entries count
  int local_24; // Loop counter for IFD entries
  int local_44; // GPS IFD offset
  int local_40; // XIF IFD offset

  // Initialize global function pointers
  swap_short = intel_swap_short; // Default, will be set later
  swap_word = intel_swap_word;   // Default, will be set later

  // Initial receive_bytes call (status check)
  local_28 = receive_bytes_status();
  if (local_28 == -1) {
    printf("did not receive bytes\n");
    _terminate();
  }

  // Receive SOM marker
  if (receive_bytes_data(&local_46, sizeof(local_46)) != sizeof(local_46) || local_46 != (marker_t)0xfff8) { // -8 in short
    printf("Did not find SOM marker\n");
    _terminate();
  }
  printf("SOM marker found\n");

  // Receive SAP0 marker
  if (receive_bytes_data(&local_48, sizeof(local_48)) != sizeof(local_48)) {
    printf("did not receive bytes\n");
    _terminate();
  }

  if (local_48 == (marker_t)0xfff0) { // -0x10 in short
    printf("SAP0 marker found\n");

    // Receive SAP0 segment size
    if (receive_bytes_data(&local_4a, sizeof(local_4a)) != sizeof(local_4a)) {
      printf("did not receive bytes\n");
      _terminate();
    }

    if (local_4a < 3) {
      printf("Invalid segment size (SAP0)\n");
      _terminate();
    }

    size_t segment_size_sap0 = local_4a - 2;
    local_2c = malloc(segment_size_sap0);
    if (local_2c == NULL) {
      printf("Unable to allocate memory (SAP0)\n");
      _terminate();
    }

    if (receive_bytes_data(local_2c, segment_size_sap0) != segment_size_sap0) {
      printf("unable to read SAP0 segment\n");
      _terminate();
    }

    free(local_2c);
    local_2c = NULL;

    // Receive next marker after SAP0 segment (expecting SAP1)
    if (receive_bytes_data(&local_48, sizeof(local_48)) != sizeof(local_48)) {
      printf("did not receive bytes\n");
      _terminate();
    }
  }

  // Check for SAP1 marker
  if (local_48 != (marker_t)0xfff1) { // -0xf in short
    printf("Did not find SAP1 marker\n");
    _terminate();
  }
  printf("SAP1 marker found\n");

  // Receive SAP1 segment size
  if (receive_bytes_data(&local_4a, sizeof(local_4a)) != sizeof(local_4a)) {
    printf("did not receive bytes\n");
    _terminate();
  }

  printf("sizeof section is %u\n", local_4a);

  if (local_4a == 0) {
    printf("Invalid segment size\n");
    _terminate();
  }

  local_2c = malloc(local_4a);
  if (local_2c == NULL) {
    printf("Unable to allocate memory\n");
    _terminate();
  }

  if (receive_bytes_data(local_2c, local_4a) != local_4a) {
    printf("unable to read SAP1 segment\n");
    _terminate();
  }

  if (local_4a < 0x10) {
    printf("not enough data received\n");
    _terminate();
  }

  local_30 = (int16_t *)((intptr_t)local_2c + 6); // Pointer to TIFF header (bytes 6-7 of SAP1)
  local_34 = (intptr_t)local_2c + local_4a; // End of segment memory

  // Check byte order marker
  uint16_t byte_order_marker = *(uint16_t *)local_30;
  if (byte_order_marker == 0x4949) { // 'II' for Intel
    printf("Intel formatted integers\n");
    swap_short = intel_swap_short;
    swap_word = intel_swap_word;
  }
  else if (byte_order_marker == 0x4d4d) { // 'MM' for Motorola
    printf("Motorola formatted integers\n");
    swap_short = motorola_swap_short;
    swap_word = motorola_swap_word;
  } else {
    printf("Unknown byte order marker 0x%X, defaulting to Intel\n", byte_order_marker);
    swap_short = intel_swap_short;
    swap_word = intel_swap_word;
  }

  // Read TagMark (0x2A)
  uint16_t tag_mark = swap_short(local_30[1]); // local_30[1] is 2 bytes after local_30, which is at offset 6. So this is offset 8.
  printf("TagMark = %x\n", tag_mark);

  // Read Offset to first IFD
  local_38 = swap_word(*(uint32_t *)(local_30 + 2)); // local_30 + 2 is 4 bytes after local_30, which is at offset 10.
  printf("Offset = %x\n", local_38);

  if (local_4a < local_38) {
    printf("Invalid offset\n");
    _terminate();
  }

  // Pointer to the first IFD entry count
  local_3c = (ushort *)((intptr_t)local_30 + local_38);
  ushort num_ifd_entries = swap_short(*local_3c);
  *local_3c = num_ifd_entries; // Original code writes back the swapped value.

  printf("# of compatility arrays: %u\n", num_ifd_entries);

  if ((local_4a - 0x10) < (uint32_t)num_ifd_entries * 0xc) { // 0xc is 12 bytes per IFD entry
    printf("Invalid number of IFD entries\n");
    _terminate();
  }

  local_24 = 0; // Loop counter
  while (true) {
    if (num_ifd_entries <= local_24) {
      printf("Finished processing\n");
      break; // Exit loop
    }

    // Process each IFD entry (12 bytes: Tag, Type, Count, Value/Offset)
    // local_3c[0] is the count. Entries start at local_3c + 1.
    // Each entry is 6 ushorts (12 bytes).
    // An entry's Tag is at (local_3c + 1 + local_24 * 6)
    // An entry's Type is at (local_3c + 1 + local_24 * 6 + 1)
    // An entry's Count (32-bit) is at (local_3c + 1 + local_24 * 6 + 2)
    // An entry's Value/Offset (32-bit) is at (local_3c + 1 + local_24 * 6 + 4)
    // The original code uses `local_3c[local_24 * 6 + 1]` etc.
    // This implies `local_3c` already points to the start of the *entries* array,
    // and `*local_3c` (or `local_3c[0]`) was the count read previously.
    // Let's adjust `local_3c` to point to the first actual entry.
    // If `local_3c` points to the count, then the entries start at `local_3c + 1`.

    // The snippet uses `local_3c[local_24 * 6 + 1]` for Tag, `local_3c[local_24 * 6 + 2]` for Type.
    // This means `local_3c` points to the *count*, and `local_3c[0]` is the count.
    // The first entry's tag is at `local_3c[1]`.
    // The `+1` in `local_3c[local_24 * 6 + 1]` already accounts for the count field.
    // So if `local_3c` is the pointer to the count, `local_3c[1]` is the first tag.

    uint16_t current_tag = swap_short(local_3c[local_24 * 6 + 1]);
    printf("Tag: %x (", current_tag);
    print_tag_text(current_tag);
    printf(")\n");

    uint16_t current_type = swap_short(local_3c[local_24 * 6 + 2]);
    printf("Type: %x (", current_type);
    print_type(current_type);
    printf(")\n");

    uint32_t current_count = swap_word(*(uint32_t *)(local_3c + local_24 * 6 + 3));
    printf("Count: %u\n", current_count);

    // Value/Offset handling (replaces goto)
    bool value_printed = false;
    uint32_t value_offset_data = swap_word(*(uint32_t *)(local_3c + local_24 * 6 + 5));

    if (current_type == 2) { // ASCII string type
        if (value_offset_data < (local_4a - 8)) { // Check if offset is within segment bounds
            if (value_offset_data != 0) {
                printf("Value: %s\n", (char *)((intptr_t)local_30 + value_offset_data));
                value_printed = true;
            }
        }
        if (!value_printed) { // If conditions for string were not met, or offset was 0
            printf("Value: 0\n");
            value_printed = true;
        }
    }
    // If not an ASCII type, or if ASCII but already handled above (value_printed is true)
    if (!value_printed) {
        printf("Value: %u\n", value_offset_data); // Print as unsigned integer
    }

    // Process specific IFDs (GPS IFD, EXIF IFD)
    if (current_tag == (uint16_t)0x8825) { // EXIF_GPS_IFD (0x8825 = -0x77db)
      local_44 = value_offset_data; // This is the offset to the GPS IFD
      process_gps_ifd((void *)((intptr_t)local_30 + local_44), local_30, local_4a, local_34);
    } else if (current_tag == (uint16_t)0x8769) { // EXIF_EXIF_IFD (0x8769 = -0x7897)
      local_40 = value_offset_data; // This is the offset to the EXIF IFD
      process_xif_ifd((void *)((intptr_t)local_30 + local_40), local_30, local_4a, local_34);
    }

    local_24 = local_24 + 1;
  }

  free(local_2c); // Free the last allocated memory
  return 0;
}