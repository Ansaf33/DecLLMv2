#include <stdio.h>
#include <stdlib.h> // For malloc, free, strdup, strtoul, calloc
#include <string.h> // For strlen, strcmp, memcpy, memset, strcpy
#include <ctype.h>  // For isdigit

// Structure for a barcode lookup table entry
// Total size 4 + 11 + 9 = 24 bytes (0x18)
typedef struct BarcodeEntry {
    int id_val;       // Value used for checksum or identification (e.g., ASCII - 0x20, 0-99 for Code C, special codes)
    char bin_rep[11]; // Binary representation string (e.g., "1010101010") - 10 chars + null
    char ascii_rep[9]; // ASCII representation string (e.g., "A", "05", "STARTB") - 8 chars + null
} BarcodeEntry;

// Global lookup table (108 entries total, indices 0 to 107)
// Indices 0-95: Mapped from ASCII 0x20-0x7F for Code B (e.g., ' ' -> 0, 'A' -> 33)
// Index 96-99: Unused (can be filled or remain empty)
// Index 100: TAB (Code B special character)
// Index 101: FNC1 (Code B special character)
// Indices 0-99 (re-used): Mapped from 00-99 for Code C
// Indices 0-102 (re-used): Mapped from checksum calculation (value % 103)
// Index 103: Quiet Zone
// Index 104: Start B
// Index 105: Start C
// Index 106: Stop
// Index 107: (Could be used for something else, or just padding)
BarcodeEntry g_barcode_lut[0x6c];

// Pointers to special entries within the lookup table
BarcodeEntry *g_blut_quiet;
BarcodeEntry *g_blut_startb;
BarcodeEntry *g_blut_startc;
BarcodeEntry *g_blut_stop;

// Dummy initialization function for the global lookup table
// In a real application, this would be loaded from a configuration or data file.
void init_dummy_barcode_lut() {
    // Initialize special control entries
    g_blut_quiet = &g_barcode_lut[103];
    strcpy(g_blut_quiet->bin_rep, "0000000000"); // Dummy binary rep
    strcpy(g_blut_quiet->ascii_rep, "QUIET"); // Dummy ASCII rep
    g_blut_quiet->id_val = 0; // Not typically used for quiet zone

    g_blut_startb = &g_barcode_lut[104];
    strcpy(g_blut_startb->bin_rep, "1111111111");
    strcpy(g_blut_startb->ascii_rep, "STARTB");
    g_blut_startb->id_val = 104; // A unique ID value for this entry

    g_blut_startc = &g_barcode_lut[105];
    strcpy(g_blut_startc->bin_rep, "2222222222");
    strcpy(g_blut_startc->ascii_rep, "STARTC");
    g_blut_startc->id_val = 105;

    g_blut_stop = &g_barcode_lut[106];
    strcpy(g_blut_stop->bin_rep, "3333333333");
    strcpy(g_blut_stop->ascii_rep, "STOP");
    g_blut_stop->id_val = 106;

    // Initialize Code B characters (ASCII 0x20 to 0x7F -> indices 0 to 95)
    // The `id_val` stores the ASCII character for data reconstruction and checksum.
    for (int i = 0; i < 96; ++i) {
        int ascii_char = i + 0x20; // ' ' (32) maps to index 0, 'A' (65) maps to index 33
        g_barcode_lut[i].id_val = ascii_char;
        sprintf(g_barcode_lut[i].bin_rep, "B%03d%03d", ascii_char, i); // Dummy binary representation
        sprintf(g_barcode_lut[i].ascii_rep, "%c", (char)ascii_char); // Single ASCII character
    }

    // Initialize special Code B characters (TAB and FNC1)
    g_barcode_lut[100].id_val = 100; // Original code used 100 for TAB
    strcpy(g_barcode_lut[100].bin_rep, "T\tT\tT\tT\tT\t"); // Dummy binary rep
    strcpy(g_barcode_lut[100].ascii_rep, "TAB"); // ASCII rep for TAB

    g_barcode_lut[101].id_val = 0x65; // Original code used 0x65 (101) for FNC1
    strcpy(g_barcode_lut[101].bin_rep, "FNC1FNC1F1");
    strcpy(g_barcode_lut[101].ascii_rep, "FNC1");

    // Initialize Code C characters (numeric pairs 00-99 -> indices 0 to 99)
    // The `id_val` stores the numeric value (0-99) for data reconstruction and checksum.
    // These entries overlap with Code B ASCII characters, but are used based on barcode_type.
    for (int i = 0; i < 100; ++i) {
        g_barcode_lut[i].id_val = i;
        sprintf(g_barcode_lut[i].bin_rep, "C%02dC%02dC", i, i); // Dummy binary representation
        sprintf(g_barcode_lut[i].ascii_rep, "%02d", i); // Two-digit numeric string
    }
}

// Structure for the generated barcode output
typedef struct Barcode {
    int total_elements;    // Number of barcode elements (quiet zones, start, data, checksum, stop, quiet)
    int barcode_type;      // 100 for CODE B, 0x65 for CODE C
    char *data_string;     // The original or modified input string (human-readable)
    BarcodeEntry **elements; // Array of pointers to BarcodeEntry for each element
    int checksum_value;    // Calculated checksum
} Barcode;


// Function: find_entry_by_bin_rep
BarcodeEntry * find_entry_by_bin_rep(const char *binary_representation) {
  for (unsigned int i = 0; i <= 0x6b; ++i) { // Loop through all possible entries up to index 107 (0x6b)
    if (strcmp(g_barcode_lut[i].bin_rep, binary_representation) == 0) {
      return &g_barcode_lut[i];
    }
  }
  return NULL;
}

// Function: create_barcode_from_str
Barcode * create_barcode_from_str(const char *input_param) {
  size_t input_len = strlen(input_param);
  
  if (input_param == NULL || input_len == 0 || input_len > 0xfa) { // Max length 250
    return NULL;
  }

  Barcode *barcode = (Barcode *)malloc(sizeof(Barcode));
  if (barcode == NULL) {
      return NULL;
  }
  // Initialize fields to safe values
  memset(barcode, 0, sizeof(Barcode));

  int is_numeric = 1;
  for (size_t i = 0; i < input_len; ++i) {
    if (!isdigit((unsigned char)input_param[i])) { // Use unsigned char for isdigit
      is_numeric = 0;
    }
    // Check for invalid characters based on the original logic
    // Characters below ' ' (0x20) or 0x7F are invalid, unless they are '\t' or 0xC0 (-0x40)
    if (((input_param[i] < ' ') || (input_param[i] == '\x7f')) &&
        (input_param[i] != '\t' && (input_param[i] != (char)0xC0))) {
      free(barcode);
      return NULL;
    }
  }
  
  printf("but the q isdid i make it here?\n"); // Original debug print

  int current_element_idx = 0; // Index for barcode->elements array
  
  if (!is_numeric) { // Barcode Type B
    barcode->barcode_type = 100; // CODE B identifier
    barcode->data_string = strdup(input_param);
    if (barcode->data_string == NULL) { free(barcode); return NULL; }

    barcode->checksum_value = 0x68; // Initial checksum for CODE B
    // total_elements: Quiet, StartB, Data (input_len), Checksum, Stop, Quiet
    barcode->total_elements = input_len + 5;
    barcode->elements = (BarcodeEntry **)malloc(barcode->total_elements * sizeof(BarcodeEntry *));
    if (barcode->elements == NULL) { free(barcode->data_string); free(barcode); return NULL; }

    barcode->elements[current_element_idx++] = g_blut_quiet;
    barcode->elements[current_element_idx++] = g_blut_startb;

    for (size_t i = 0; i < input_len; ++i) {
      int char_map_idx;
      if (input_param[i] == '\t') {
        char_map_idx = 100; // Index for TAB entry
      } else if (input_param[i] == (char)0xC0) { // -0x40
        char_map_idx = 101; // Index for FNC1 entry
      } else {
        char_map_idx = input_param[i] - 0x20; // Map ASCII 0x20-0x7F to indices 0-95
      }
      barcode->elements[current_element_idx++] = &g_barcode_lut[char_map_idx];
      barcode->checksum_value += (i + 1) * g_barcode_lut[char_map_idx].id_val; // Use id_val for checksum
    }
  } else { // Barcode Type C
    const char *current_param_ptr = input_param; // Pointer to the string being processed
    char *temp_param_alloc = NULL; // Temporary allocation if '0' is prepended

    barcode->barcode_type = 0x65; // CODE C identifier
    
    // If input length is odd, prepend '0'
    if ((input_len & 1) != 0) {
      temp_param_alloc = (char *)malloc(input_len + 2); // +1 for '0', +1 for null terminator
      if (temp_param_alloc == NULL) { free(barcode); return NULL; }
      temp_param_alloc[0] = '0';
      memcpy(temp_param_alloc + 1, input_param, input_len + 1); // Copy original string + null
      current_param_ptr = temp_param_alloc;
      input_len++; // Update length to include the prepended '0'
    }
    barcode->data_string = strdup(current_param_ptr);
    if (barcode->data_string == NULL) { free(temp_param_alloc); free(barcode); return NULL; }
    free(temp_param_alloc); // Free the temporary buffer if it was used

    barcode->checksum_value = 0x69; // Initial checksum for CODE C
    // total_elements: Quiet, StartC, Data (input_len/2 pairs), Checksum, Stop, Quiet
    barcode->total_elements = (input_len >> 1) + 5;
    barcode->elements = (BarcodeEntry **)malloc(barcode->total_elements * sizeof(BarcodeEntry *));
    if (barcode->elements == NULL) { free(barcode->data_string); free(barcode); return NULL; }

    barcode->elements[current_element_idx++] = g_blut_quiet;
    barcode->elements[current_element_idx++] = g_blut_startc;

    char num_buf[3]; // Buffer for 2-digit number + null terminator
    num_buf[2] = '\0';
    for (size_t i = 0; i < input_len; i += 2) {
      num_buf[0] = current_param_ptr[i];
      num_buf[1] = current_param_ptr[i + 1];
      unsigned long val = strtoul(num_buf, NULL, 10);
      if (val > 99) { // If value is > 99, treat as 0
        val = 0;
      }
      barcode->elements[current_element_idx++] = &g_barcode_lut[val];
      barcode->checksum_value += ((i + 2) / 2) * (int)val; // Use numeric value for checksum
    }
  }

  barcode->checksum_value %= 0x67; // Final checksum modulo 103
  barcode->elements[current_element_idx++] = &g_barcode_lut[barcode->checksum_value]; // Checksum element
  barcode->elements[current_element_idx++] = g_blut_stop;
  barcode->elements[current_element_idx++] = g_blut_quiet;

  if (current_element_idx != barcode->total_elements) {
    printf("Bad barcode processing: element count mismatch. Expected %d, got %d\n", 
           barcode->total_elements, current_element_idx);
    barcode->total_elements = current_element_idx; // Adjust total_elements
  }
  
  return barcode;
}

// Function: find_stop_code
char * find_stop_code(const char *encoded_data) {
  const char *stop_pattern = g_blut_stop->bin_rep;
  size_t stop_len = strlen(stop_pattern);
  size_t data_len = strlen(encoded_data);

  // Trim trailing spaces from encoded_data
  while (data_len > 0 && encoded_data[data_len - 1] == ' ') {
    data_len--;
  }

  // If the data is shorter than the stop pattern, it cannot contain it
  if (data_len < stop_len) {
      return NULL;
  }

  // Compare from the end of the (trimmed) encoded_data
  size_t param_idx = data_len;
  size_t stop_idx = stop_len;

  while (stop_idx > 0) {
    stop_idx--;
    param_idx--;
    if (stop_pattern[stop_idx] != encoded_data[param_idx]) {
      return NULL; // Mismatch
    }
  }
  
  return (char *)(encoded_data + param_idx); // Return pointer to the start of the matched stop code
}

// Function: create_barcode_from_encoded_data
Barcode * create_barcode_from_encoded_data(const char *encoded_data_param) {
  Barcode *barcode = NULL;
  char bin_rep_buf[11]; // Buffer for 10-char binary representation + null terminator
  bin_rep_buf[10] = '\0'; 

  char *stop_pos = find_stop_code(encoded_data_param);
  if (stop_pos == NULL) {
    return NULL;
  }

  // Calculate the offset of the last character of the checksum element's binary representation.
  // The stop code's binary representation is 10 characters long.
  // The checksum element's binary representation is also 10 characters long.
  // So, `stop_pos` points to the start of the stop code.
  // `stop_pos - 10` would be the start of the checksum element.
  // `stop_pos - 1` would be the last char of checksum element.
  // `data_end_offset` should be the index of the last character of the checksum element.
  int data_end_offset = (stop_pos - encoded_data_param) - 1; // Index of last char of checksum_bin

  int prefix_len = 0;
  // Find the '|' separator or end of string to determine prefix length
  for (prefix_len = 0;
       encoded_data_param[prefix_len] != '\0' && encoded_data_param[prefix_len] != '|';
       prefix_len++);

  // Check if the data portion (after prefix, before checksum/stop) is valid
  // `prefix_len + 1` is the index of the first char of the first data element (after '|').
  // `data_end_offset` is the index of the last char of the checksum element.
  // The total length of the binary block (start_code + data_elements + checksum) is
  // `data_end_offset - (prefix_len + 1) + 1`.
  if (data_end_offset < (prefix_len + 1) || encoded_data_param[prefix_len] == '\0') {
    return NULL;
  }

  // Allocate Barcode structure
  barcode = (Barcode *)malloc(sizeof(Barcode));
  if (barcode == NULL) { return NULL; }
  memset(barcode, 0, sizeof(Barcode)); // Initialize all members to zero

  int current_element_idx = 0; // Index for barcode->elements array
  int data_element_count = 0; // Counter for checksum calculation weight

  // The number of data elements (start code, data elements, checksum element)
  // `(data_end_offset - (prefix_len + 1) + 1)` is the total length of the binary block.
  // This length divided by 11 (each binary element is 10 chars + a separator or implicit end) gives the number of binary elements.
  // Add 3 for initial quiet, and final stop, quiet.
  // Total elements = Quiet + (Start + Data + Checksum) + Stop + Quiet
  barcode->total_elements = ((data_end_offset - (prefix_len + 1) + 1) / 10) + 4;
  
  barcode->elements = (BarcodeEntry **)malloc(barcode->total_elements * sizeof(BarcodeEntry *));
  if (barcode->elements == NULL) { free(barcode); return NULL; }
  // Initialize elements array pointers to NULL
  for(int i = 0; i < barcode->total_elements; ++i) barcode->elements[i] = NULL;

  barcode->elements[current_element_idx++] = g_blut_quiet;

  // Start processing after the prefix and '|'
  int current_encoded_offset = prefix_len + 1; // Start of binary data elements

  // Read the first data element (which should be a Start B or Start C code)
  memcpy(bin_rep_buf, encoded_data_param + current_encoded_offset, 10);
  BarcodeEntry *entry = find_entry_by_bin_rep(bin_rep_buf);
  
  if (entry == NULL) { // First element (start code) not found
      free(barcode->elements); free(barcode); return NULL;
  }
  
  barcode->elements[current_element_idx++] = entry;
  
  if (entry == g_blut_startb) {
    barcode->barcode_type = 100; // CODE B
    barcode->checksum_value = 0x68; // Initial checksum for CODE B
  } else if (entry == g_blut_startc) {
    barcode->barcode_type = 0x65; // CODE C
    barcode->checksum_value = 0x69; // Initial checksum for CODE C
  } else { // Not a valid start code
      free(barcode->elements); free(barcode); return NULL;
  }

  // Process subsequent data elements until the checksum element
  current_encoded_offset += 10; // Move past the start code binary representation (10 chars)
  while (current_encoded_offset <= data_end_offset) {
    memcpy(bin_rep_buf, encoded_data_param + current_encoded_offset, 10);
    entry = find_entry_by_bin_rep(bin_rep_buf);
    
    if (entry == NULL) { // Data element not found
        free(barcode->elements); free(barcode); return NULL;
    }
    
    barcode->elements[current_element_idx++] = entry;

    // Checksum calculation based on entry's id_val and position
    // The original code used `*local_28 == 9` (TAB) or `*local_28 == 0xc0` (FNC1)
    // and then `*local_28 - 0x20` for others.
    // This implies `entry->id_val` should be used, as it stores the original value.
    if (entry->id_val == 100) { // TAB
      barcode->checksum_value += (data_element_count + 1) * 100;
    } else if (entry->id_val == 0x65) { // FNC1
      barcode->checksum_value += (data_element_count + 1) * 0x65;
    } else { // Normal character (ASCII for B, numeric for C)
      barcode->checksum_value += (data_element_count + 1) * entry->id_val;
    }
    
    data_element_count++;
    current_encoded_offset += 10; // Move to the next 10-char binary representation
  }

  // Final checksum calculation and add checksum element
  barcode->checksum_value %= 0x67;
  // Verify the calculated checksum against the actual checksum element from encoded data
  BarcodeEntry *actual_checksum_entry = barcode->elements[current_element_idx - 1]; // This is the last element processed in the loop
  BarcodeEntry *expected_checksum_entry = &g_barcode_lut[barcode->checksum_value];

  if (actual_checksum_entry != expected_checksum_entry) {
      // Checksum mismatch, this implies corruption or incorrect encoding
      printf("Checksum mismatch during decoding. Calculated ID: %d, Encoded ID: %d\n",
             expected_checksum_entry->id_val, actual_checksum_entry->id_val);
      free(barcode->elements); free(barcode); return NULL;
  }
  // If we reach here, the checksum element was already processed in the loop `current_encoded_offset <= data_end_offset`
  // and `current_element_idx` already points to the next slot after checksum.
  // So no need to add it again.

  // Add stop and quiet zone elements (these are *not* part of the checksum calculation)
  barcode->elements[current_element_idx++] = g_blut_stop;
  barcode->elements[current_element_idx++] = g_blut_quiet;

  if (current_element_idx != barcode->total_elements) {
    printf("Bad barcode processing: element count mismatch. Expected %d, got %d\n",
           barcode->total_elements, current_element_idx);
    barcode->total_elements = current_element_idx; // Adjust total_elements
  }

  // Reconstruct data_string (human-readable)
  size_t data_str_len = 0;
  if (barcode->barcode_type == 100) { // CODE B
    // Number of data elements is (total_elements - 4) -> (quiet, start, checksum, stop, quiet)
    data_str_len = barcode->total_elements - 4;
  } else if (barcode->barcode_type == 0x65) { // CODE C
    // Number of 2-digit pairs is (total_elements - 5). Each pair is 2 chars.
    data_str_len = (barcode->total_elements - 5) * 2;
  }
  
  if (data_str_len > 0) {
      barcode->data_string = (char *)malloc(data_str_len + 1);
      if (barcode->data_string == NULL) { free(barcode->elements); free(barcode); return NULL; }
      char *current_char_ptr = barcode->data_string;

      // Loop from index 2 (after quiet and start) up to the checksum element
      for (int i = 2; i < barcode->total_elements - 3; ++i) { // -3 for checksum, stop, quiet
          BarcodeEntry *data_entry = barcode->elements[i];
          if (barcode->barcode_type == 100) { // CODE B
              *current_char_ptr++ = (char)data_entry->id_val;
          } else if (barcode->barcode_type == 0x65) { // CODE C
              sprintf(current_char_ptr, "%02d", data_entry->id_val);
              current_char_ptr += 2;
          }
      }
      *current_char_ptr = '\0';
  } else {
      barcode->data_string = strdup(""); // Empty string if no data was decoded
      if (barcode->data_string == NULL) { free(barcode->elements); free(barcode); return NULL; }
  }

  return barcode;
}

// Function: create_barcode_ascii
char * create_barcode_ascii(const Barcode *barcode) {
  if (barcode == NULL || barcode->elements == NULL) {
      return NULL;
  }

  // Calculate total length needed for concatenated ascii_rep strings
  size_t total_ascii_len = 0;
  for (int i = 0; i < barcode->total_elements; ++i) {
      if (barcode->elements[i] != NULL) {
          total_ascii_len += strlen(barcode->elements[i]->ascii_rep);
      }
  }
  
  char *ascii_output = (char *)calloc(1, total_ascii_len + 1); // +1 for null terminator
  if (ascii_output == NULL) {
      return NULL;
  }
  
  char *current_pos = ascii_output;
  for (int i = 0; i < barcode->total_elements; ++i) {
    if (barcode->elements[i] != NULL) {
        strcpy(current_pos, barcode->elements[i]->ascii_rep);
        current_pos += strlen(barcode->elements[i]->ascii_rep);
    }
  }
  return ascii_output;
}

// Function: print_barcode_ascii
void print_barcode_ascii(const Barcode *barcode, int print_human_readable) {
  if (barcode == NULL) {
      printf("Error: Barcode is NULL.\n");
      return;
  }

  if (print_human_readable != 0 && barcode->data_string != NULL) {
    printf("Human-readable data: %s\n", barcode->data_string);
  }

  // Print binary representations for all elements from Start Code to Stop Code
  // Original loop was from index 1 to total_elements - 1 (exclusive of first/last quiet zone)
  printf("Binary representation: ");
  for (int i = 1; i < barcode->total_elements - 1; ++i) {
    if (barcode->elements[i] != NULL) {
        printf("%s", barcode->elements[i]->bin_rep);
    } else {
        printf("[NULL_ELEMENT]"); // Should not happen with proper creation
    }
  }
  printf("\n");
}