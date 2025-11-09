#include <stdio.h>    // For printf, puts
#include <stdlib.h>   // For malloc, free, exit
#include <stdint.h>   // For fixed-width integers like uint16_t, uint32_t
#include <string.h>   // For memcpy

// Custom type definitions based on common decompiler output
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint32_t undefined4; // Original type, replaced by uint for clarity
typedef unsigned char undefined; // Single byte, if needed

// Function pointer types for byte swapping
typedef ushort (*swap_short_func)(ushort);
typedef uint (*swap_word_func)(uint);

// Global function pointers
swap_short_func swap_short;
swap_word_func swap_word;

// --- Stub Functions (to make the code compilable) ---

// Placeholder for the actual byte receiving mechanism.
// In a real application, this would interact with a file, network socket, etc.
// Assumes it reads 'size' bytes into 'buffer' and returns number of bytes read or -1 on error.
int receive_bytes(void* buffer, size_t size) {
    // For compilation and basic execution, fill with zeros and return success.
    // Replace with actual I/O logic as needed.
    memset(buffer, 0, size);
    return (int)size;
}

// Placeholder for program termination
void _terminate(void) {
    printf("Program terminated.\n");
    exit(1);
}

// Placeholder for tag text printing
void print_tag_text(uint tag) {
    // Implement actual tag name lookup here
    switch (tag) {
        case 0x0001: printf("SomeTag"); break;
        case 0x8825: printf("GPS IFD Pointer"); break;
        case 0x8769: printf("Exif IFD Pointer"); break;
        default: printf("Unknown Tag"); break;
    }
}

// Placeholder for type printing
void print_type(uint type) {
    // Implement actual type name lookup here
    switch (type) {
        case 1: printf("BYTE"); break;
        case 2: printf("ASCII"); break;
        case 3: printf("SHORT"); break;
        case 4: printf("LONG"); break;
        default: printf("Unknown Type"); break;
    }
}

// Placeholder for GPS IFD processing
void process_gps_ifd(void* ifd_ptr, uint segment_size, void* base_ptr, void* end_ptr) {
    printf("Processing GPS IFD at offset %ld (from segment base + 6)\n", (char*)ifd_ptr - ((char*)base_ptr + 6));
    // Implement GPS IFD processing logic here
}

// Placeholder for XIF IFD processing
void process_xif_ifd(void* ifd_ptr, uint segment_size, void* base_ptr, void* end_ptr) {
    printf("Processing XIF IFD at offset %ld (from segment base + 6)\n", (char*)ifd_ptr - ((char*)base_ptr + 6));
    // Implement XIF IFD processing logic here
}

// Byte swapping functions (assuming Linux is typically little-endian)
// If the system is little-endian, intel_swap_* are identity functions.
// If the system is big-endian, these would perform actual byte swapping.
ushort intel_swap_short(ushort val) {
    return val; // Little-endian, no swap needed on little-endian system
}

uint intel_swap_word(uint val) {
    return val; // Little-endian, no swap needed on little-endian system
}

ushort motorola_swap_short(ushort val) {
    return (val >> 8) | (val << 8); // Big-endian, swap bytes
}

uint motorola_swap_word(uint val) {
    return ((val >> 24) & 0x000000FF) |
           ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) |
           ((val << 24) & 0xFF000000); // Big-endian, swap bytes
}

// Global string literal (DAT_00016ca5)
const char* DAT_00016ca5 = ")\n";

// Function: main
int main(void) {
  short som_marker = 0; // SOM marker, initialized to avoid uninitialized read
  short sap_marker;     // SAP0/SAP1 marker
  ushort segment_size;  // Segment size
  int bytes_received;
  void *segment_memory = NULL; // Allocated memory for segments
  char *segment_base_ptr;      // Base of the current segment for processing
  char *segment_end_ptr;       // End of the current segment for bounds checking

  // Initialize swap function pointers to Intel (little-endian) as default
  swap_short = intel_swap_short;
  swap_word = intel_swap_word;

  // Receive SOM marker
  bytes_received = receive_bytes(&som_marker, sizeof(som_marker));
  if (bytes_received == -1) {
    printf("did not receive SOM marker bytes\n");
    _terminate();
  }

  if (som_marker != (short)0xFFF8) { // Assuming -8 is 0xFFF8 (SOM marker value)
    printf("Did not find SOM marker (received 0x%hX)\n", som_marker);
    _terminate();
  }
  printf("SOM marker found\n");

  // Loop for SAP0 segments until SAP1 marker is found
  while (1) {
    // Receive SAP marker (could be SAP0 or SAP1)
    bytes_received = receive_bytes(&sap_marker, sizeof(sap_marker));
    if (bytes_received == -1) {
      printf("did not receive SAP marker bytes\n");
      _terminate();
    }

    if (sap_marker == (short)0xFFF0) { // Assuming -0x10 is 0xFFF0 (SAP0 marker value)
      printf("SAP0 marker found\n");

      // Receive SAP0 segment size
      bytes_received = receive_bytes(&segment_size, sizeof(segment_size));
      if (bytes_received == -1) {
        printf("did not receive SAP0 segment size bytes\n");
        _terminate();
      }

      if (segment_size < 3) {
        printf("Invalid SAP0 segment size (%u)\n", segment_size);
        _terminate();
      }

      // Allocate memory for SAP0 segment data (size - 2 for the size itself)
      size_t sap0_data_size = segment_size - 2;
      segment_memory = malloc(sap0_data_size);
      if (segment_memory == NULL) {
        printf("Unable to allocate memory for SAP0 segment\n");
        _terminate();
      }

      // Receive SAP0 segment data
      bytes_received = receive_bytes(segment_memory, sap0_data_size);
      if (bytes_received == -1) {
        printf("unable to read SAP0 segment data\n");
        free(segment_memory); // Free allocated memory before terminating
        segment_memory = NULL;
        _terminate();
      }

      // Original code just frees SAP0 segment, no further processing indicated.
      free(segment_memory);
      segment_memory = NULL; // Clear pointer after freeing
      // Loop back to check for next SAP marker (could be another SAP0 or SAP1)
    } else {
      // Not a SAP0 marker, so it must be SAP1 or an error
      break; // Exit the loop to process SAP1
    }
  }

  // After the loop, sap_marker should contain the SAP1 marker or an error
  if (sap_marker != (short)0xFFF1) { // Assuming -0xf is 0xFFF1 (SAP1 marker value)
    printf("Did not find SAP1 marker (received 0x%hX)\n", sap_marker);
    _terminate();
  }
  printf("SAP1 marker found\n");

  // Receive SAP1 segment size
  bytes_received = receive_bytes(&segment_size, sizeof(segment_size));
  if (bytes_received == -1) {
    printf("did not receive SAP1 segment size bytes\n");
    _terminate();
  }

  printf("sizeof section is %u\n", segment_size);
  if (segment_size == 0) {
    printf("Invalid SAP1 segment size (0)\n");
    _terminate();
  }

  // Allocate memory for SAP1 segment
  segment_memory = malloc(segment_size);
  if (segment_memory == NULL) {
    printf("Unable to allocate memory for SAP1 segment\n");
    _terminate();
  }

  // Receive SAP1 segment data
  bytes_received = receive_bytes(segment_memory, segment_size);
  if (bytes_received == -1) {
    printf("unable to read SAP1 segment data\n");
    free(segment_memory);
    segment_memory = NULL;
    _terminate();
  }

  if (segment_size < 0x10) { // Minimum size for header + IFD entry (approx.)
    printf("not enough data received for SAP1 segment (size %u)\n", segment_size);
    free(segment_memory);
    segment_memory = NULL;
    _terminate();
  }

  segment_base_ptr = (char *)segment_memory;
  segment_end_ptr = segment_base_ptr + segment_size;

  // `endian_marker_ptr` points to the 6th byte (offset 6) within the segment,
  // where the II/MM endianness marker is located.
  short *endian_marker_ptr = (short *)(segment_base_ptr + 6);

  // Check endianness marker
  if (*endian_marker_ptr == (short)0x4949) { // 'II' for Intel (little-endian)
    printf("Intel formatted integers\n");
    swap_short = intel_swap_short;
    swap_word = intel_swap_word;
  } else if (*endian_marker_ptr == (short)0x4d4d) { // 'MM' for Motorola (big-endian)
    printf("Motorola formatted integers\n");
    swap_short = motorola_swap_short;
    swap_word = motorola_swap_word;
  } else {
    printf("Unknown endianness marker (0x%hX), defaulting to Intel\n", *endian_marker_ptr);
    // Could _terminate() here if strict, but original code just defaults.
  }

  // TagMark (from endian_marker_ptr[1])
  ushort tag_mark = swap_short(endian_marker_ptr[1]);
  printf("TagMark = 0x%hX\n", tag_mark);

  // Offset to first IFD (from endian_marker_ptr[2], which is a 4-byte uint)
  uint ifd_offset_from_endian_base;
  memcpy(&ifd_offset_from_endian_base, &endian_marker_ptr[2], sizeof(uint));
  ifd_offset_from_endian_base = swap_word(ifd_offset_from_endian_base);

  printf("Offset = 0x%X\n", ifd_offset_from_endian_base);

  // The IFD offset is relative to `endian_marker_ptr` (segment_base_ptr + 6).
  char *ifd_section_base = (char *)endian_marker_ptr;
  ushort *num_ifd_entries_ptr = (ushort *)(ifd_section_base + ifd_offset_from_endian_base);

  // Check if IFD offset points out of segment bounds
  if ((char*)num_ifd_entries_ptr + sizeof(ushort) > segment_end_ptr || ifd_offset_from_endian_base == 0) {
    printf("Invalid IFD offset (0x%X) or points out of bounds\n", ifd_offset_from_endian_base);
    free(segment_memory);
    segment_memory = NULL;
    _terminate();
  }

  // Read and swap the number of IFD entries
  ushort num_ifd_entries = swap_short(*num_ifd_entries_ptr);
  printf("# of compatibility arrays: %u\n", num_ifd_entries);

  // Check if enough space for IFD entries (each entry is 12 bytes: Tag(2) + Type(2) + Count(4) + Value/Offset(4))
  if ((char*)(num_ifd_entries_ptr + 1) + (uint)num_ifd_entries * 12 > segment_end_ptr) {
    printf("Invalid number of IFD entries (not enough space for 12-byte entries)\n");
    free(segment_memory);
    segment_memory = NULL;
    _terminate();
  }

  // `ifd_entries_base` points to the start of the actual IFD entries (immediately after the count ushort)
  ushort *ifd_entries_base = num_ifd_entries_ptr + 1;

  for (int i = 0; i < num_ifd_entries; ++i) {
    // Each IFD entry is 6 ushorts (12 bytes)
    ushort *current_ifd_entry = ifd_entries_base + (i * 6);

    // Tag (ushort, bytes 0-1 of entry)
    ushort entry_tag = swap_short(current_ifd_entry[0]);
    printf("Tag: 0x%hX (", entry_tag);
    print_tag_text(entry_tag);
    printf("%s", DAT_00016ca5);

    // Type (ushort, bytes 2-3 of entry)
    ushort entry_type = swap_short(current_ifd_entry[1]);
    printf("Type: 0x%hX (", entry_type);
    print_type(entry_type);
    printf("%s", DAT_00016ca5);

    // Count (uint, bytes 4-7 of entry)
    uint entry_count;
    memcpy(&entry_count, &current_ifd_entry[2], sizeof(uint));
    entry_count = swap_word(entry_count);
    printf("Count: %u\n", entry_count);

    // Value/Offset (uint, bytes 8-11 of entry)
    uint entry_value_offset;
    memcpy(&entry_value_offset, &current_ifd_entry[4], sizeof(uint));
    entry_value_offset = swap_word(entry_value_offset);

    // Process Value/Offset based on Type
    if (entry_type == 2) { // Type 2 (ASCII string)
      // The offset for the string is relative to `ifd_section_base`.
      // Check if the offset is non-zero and points within the segment.
      if (entry_value_offset != 0 && (ifd_section_base + entry_value_offset) < segment_end_ptr) {
        char *string_ptr = ifd_section_base + entry_value_offset;
        printf("Value: %s\n", string_ptr);
      } else {
        printf("Value: 0 (invalid string offset or zero)\n");
      }
    } else {
      printf("Value: %u\n", entry_value_offset); // Print as unsigned integer
    }

    // Check for specific tags (GPS IFD, Exif IFD)
    // Tags 0x8825 (GPS IFD Pointer) and 0x8769 (Exif IFD Pointer)
    if (entry_tag == 0x8825) { // GPS IFD Pointer
      // The value is an offset to the GPS IFD, relative to `ifd_section_base`
      process_gps_ifd(ifd_section_base + entry_value_offset, segment_size, segment_base_ptr, segment_end_ptr);
    } else if (entry_tag == 0x8769) { // Exif IFD Pointer
      // The value is an offset to the Exif IFD, relative to `ifd_section_base`
      process_xif_ifd(ifd_section_base + entry_value_offset, segment_size, segment_base_ptr, segment_end_ptr);
    }
  }

  printf("Finished processing\n");

  // Free allocated memory at the end
  if (segment_memory != NULL) {
    free(segment_memory);
    segment_memory = NULL;
  }

  return 0;
}