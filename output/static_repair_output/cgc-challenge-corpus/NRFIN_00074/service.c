#include <stdio.h>   // For fgets, sprintf, stdin
#include <stdlib.h>  // For malloc, free, atoi, exit
#include <string.h>  // For strlen, memset, memcpy, strcmp, strcat, strncat, strchr, strtok
#include <stdbool.h> // For bool
#include <stdint.h>  // For uint32_t, etc. (using unsigned int for uint)
#include <unistd.h>  // For write

// Define custom types
typedef unsigned int uint;

// Global data definitions
// DAT_4347c000: Used as byte array, accessed up to index 9.
unsigned char DAT_4347c000[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D // Dummy data, actual content unknown
};
const char DAT_000182ee[] = "IC"; // Used for Roman numeral 90, non-standard (usually "XC")
const char DAT_000182f1[] = "L";
const char DAT_000182f3[] = "XL";
const char DAT_000182f6[] = "IV";
const char DAT_000182f9[] = "V";
const char DAT_000182fb[] = "IX";
const char DAT_000182fe[] = "<";
const char DAT_00018301[] = ">";
const char DAT_00018309[] = "text";
const char DAT_0001834d[] = "list";
const char DAT_0001838d[] = "page";
const char DAT_000183ab[] = "\x03"; // End of Text character
const char DAT_0001834b[] = "\n"; // Newline character

// External references (these should be defined elsewhere or are placeholders for string literals)
const char FourByFourTable_Macro[] = "<macro><name>FourByFourTable</name><definition><table><rows>4</rows><fields>4</fields><border>line</border><row><header>H1</header><field>F1</field><field>F2</field><field>F3</field></row><row><header>H2</header><field>F4</field><field>F5</field><field>F6</field></row><row><header>H3</header><field>F7</field><field>F8</field><field>F9</field></row><row><header>H4</header><field>F10</field><field>F11</field><field>F12</field></row></table></definition></macro>";
const char FiveByFiveTable_Macro[] = "<macro><name>FiveByFiveTable</name><definition><table><rows>5</rows><fields>5</fields><border>star</border><row><header>H1</header><field>F1</field><field>F2</field><field>F3</field><field>F4</field></row><row><header>H2</header><field>F5</field><field>F6</field><field>F7</field><field>F8</field></row><row><header>H3</header><field>F9</field><field>F10</field><field>F11</field><field>F12</field></row><row><header>H4</header><field>F13</field><field>F14</field><field>F15</field><field>F16</field></row><row><header>H5</header><field>F17</field><field>F18</field><field>F19</field><field>F20</field></row></table></definition></macro>";
const char AlphanumericOutline_Macro[] = "<macro><name>AlphanumericOutline</name><definition><list><type>ALPHA</type><element><text>Item A</text></element><element><text>Item B</text></element><element><text>Item C</text><list><type>numeral</type><element><text>Subitem 1</text></element><element><text>Subitem 2</text></element></list></element></list></definition></macro>";
const char BulletedOutline_Macro[] = "<macro><name>BulletedOutline</name><definition><list><type>*</type><element><text>Bullet 1</text></element><element><text>Bullet 2</text></element><element><text>Bullet 3</text><list><type>-</type><element><text>Sub-bullet 1</text></element><element><text>Sub-bullet 2</text></element></list></element></list></definition></macro>";


// Global variable
int columnWidth;

// Helper function definitions
char to_hex(unsigned int nibble) {
    if (nibble < 10) return (char)('0' + nibble);
    return (char)('A' + nibble - 10);
}

// Custom itoa for consistency, though sprintf is usually better.
char* custom_itoa(int val, char* buf) {
    sprintf(buf, "%d", val);
    return buf;
}

// Custom transmit_all using write to stdout (fd=1)
void transmit_all(int fd, const char *buf, size_t count) {
    write(fd, buf, count);
}

// Forward declarations for functions used before definition
// Node structure: [name_string_ptr, children_list_head_ptr, next_sibling_ptr]
// Macro node structure: [macro_handler_func_ptr, next_macro_node_ptr, name_string_ptr]
typedef char* (*macro_handler_func_ptr)(void*, void*);

char * table(void* param_1, void* param_2);
char * element(void* param_1, void* param_2);
char * list(void* param_1, void* param_2);
char * text(void* param_1, void* param_2);
char * paragraph(void* param_1, void* param_2);
char * page(void* param_1, void* param_2);
int document(void* param_1, void* param_2); // Original was undefined4, changed to int

// Function: getDocumentID
int getDocumentID(void) {
  uint local_38[] = {0x1d, 7, 0x12, 0x11, 0x10, 0xe, 0xb, 6, 8, 0xf};
  int local_8 = 1;
  for (uint local_c = 0; local_c < 10; local_c++) {
    if ((uint)(unsigned char)DAT_4347c000[local_c] % local_38[local_c] == 0) {
      local_8 = local_38[local_c] * local_8;
    }
  }
  return local_8;
}

// Function: romanNumeral
char * romanNumeral(uint param_1) {
  char *result_string;
  uint num_tens_thousands; // Not explicitly used but implied by divisions
  uint num_thousands;      // Not explicitly used but implied by divisions
  uint num_hundreds;       // local_20
  uint num_fifties;        // local_24
  uint remainder_after_fifties; // local_30
  uint num_tens;           // local_28
  uint remainder_after_tens;    // local_30 (reused)
  uint num_fives;          // local_2c
  uint remainder_after_fives;   // local_30 (reused)
  size_t current_len = 0;

  if (param_1 == 0) {
    result_string = NULL;
  }
  else if (param_1 < 999) { // param_1 < 0x3e9
    num_hundreds = param_1 / 100;
    num_fifties = (param_1 % 100) / 50;
    remainder_after_fifties = (param_1 % 100) % 50;
    num_tens = remainder_after_fifties / 10;
    remainder_after_tens = remainder_after_fifties % 10;
    num_fives = remainder_after_tens / 5;
    remainder_after_fives = remainder_after_tens % 5;
    
    // Allocate a buffer large enough for Roman numerals up to 999 (e.g., CMXCIX is 7 chars) + padding
    result_string = (char *)malloc(29); // Original size 0x1d (29 bytes)
    if (result_string == NULL) {
      return NULL;
    }
    memset(result_string, 0, 29);

    // Add 'C's
    memset(result_string, 'C', num_hundreds);
    current_len += num_hundreds;

    // Handle 'L' (50) and 'X' (10) combinations
    // The original logic `local_28 + local_24 == 5` is ambiguous.
    // Assuming it's meant for 'XC' (90) or 'XL' (40)
    if (num_tens == 4 && num_fifties == 1) { // 90 (XC)
        strcat(result_string, DAT_000182ee); // "IC" (non-standard, but as per source)
        current_len += 2;
        num_fifties = 0; num_tens = 0; // Handled
    } else if (num_fifties != 0) { // Add 'L' for 50
      strcat(result_string, DAT_000182f1); // "L"
      current_len += 1;
    }
    if (num_tens == 4) { // Add 'XL' for 40
      strcat(result_string, DAT_000182f3); // "XL"
      current_len += 2;
    } else { // Add 'X's
      memset(result_string + current_len, 'X', num_tens);
      current_len += num_tens;
    }

    // Handle 'V' (5) and 'I' (1) combinations
    if (remainder_after_fives == 4 && num_fives == 1) { // 9 (IX)
        strcat(result_string, DAT_000182fb); // "IX"
    } else if (remainder_after_fives == 4) { // 4 (IV)
        strcat(result_string, DAT_000182f6); // "IV"
    } else {
      if (num_fives != 0) { // Add 'V' for 5
        strcat(result_string, DAT_000182f9); // "V"
        current_len += 1;
      }
      memset(result_string + current_len, 'I', remainder_after_fives); // Add 'I's
    }
  }
  else {
    result_string = NULL;
  }
  return result_string;
}

// Function: getCustomMacro
char ** getCustomMacro(char **param_1, char *param_2) {
  char **current_macro_list = param_1;
  while ((current_macro_list != NULL) && (strcmp(*current_macro_list, param_2) != 0)) {
    current_macro_list = (char **)current_macro_list[2];
  }
  return current_macro_list;
}

// Function: removeObjectFromList
char ** removeObjectFromList(char **param_1, char *param_2) {
  char **prev = NULL;
  char **current = (char **)*param_1; // Head of the list

  while(current != NULL) {
    if (strcmp(*current, param_2) == 0) {
      if (prev != NULL) {
        prev[2] = current[2]; // Link prev to next
      } else {
        *param_1 = current[2]; // Update head if first element
      }
      current[2] = NULL; // Detach current from list
      return current;
    }
    prev = current;
    current = (char **)current[2];
  }
  return NULL; // Object not found
}

// Function: reverseObjectList
void reverseObjectList(char ***param_1) { // Takes pointer to the head of the list
  char **prev = NULL;
  char **current = *param_1;
  char **next;
  
  while (current != NULL) {
    next = (char **)current[2]; // Save next
    current[2] = (char *)prev;  // Reverse current's pointer
    prev = current;             // Move prev to current
    current = next;             // Move current to next
  }
  *param_1 = prev; // Update head
}

// Function: executeMacro
char ** executeMacro(char **param_1, char **param_2) {
  char **result_list_head = NULL;
  if (param_1 != NULL) {
    for (char **current_param2 = param_2; current_param2 != NULL; current_param2 = (char **)current_param2[2]) {
      bool found_in_param1 = false;
      char **new_node = (char **)malloc(sizeof(char*) * 3); // Node structure: [name, value, next]
      if (new_node == NULL) return NULL; // Error handling

      size_t name_len = strlen(*current_param2);
      new_node[0] = (char *)malloc(name_len + 1);
      if (new_node[0] == NULL) { free(new_node); return NULL; }
      memcpy(new_node[0], *current_param2, name_len + 1);
      
      for (char **current_param1 = param_1; current_param1 != NULL; current_param1 = (char **)current_param1[2]) {
        if (strcmp(*current_param1, new_node[0]) == 0) {
          found_in_param1 = true;
          new_node[1] = executeMacro(current_param1[1], current_param2[1]); // Recursive call
          break;
        }
      }
      if (!found_in_param1) {
        new_node[1] = executeMacro(NULL, current_param2[1]); // Recursive call
      }
      new_node[2] = (char *)result_list_head;
      result_list_head = new_node;
    }

    for (char **current_param1 = param_1; current_param1 != NULL; current_param1 = (char **)current_param1[2]) {
      bool found_in_param2 = false;
      for (char **current_param2 = param_2; current_param2 != NULL; current_param2 = (char **)current_param2[2]) {
        if (strcmp(*current_param2, *current_param1) == 0) {
          found_in_param2 = true;
          break;
        }
      }
      if (!found_in_param2) {
        char **new_node = (char **)malloc(sizeof(char*) * 3);
        if (new_node == NULL) return NULL;

        size_t name_len = strlen(*current_param1);
        new_node[0] = (char *)malloc(name_len + 1);
        if (new_node[0] == NULL) { free(new_node); return NULL; }
        memcpy(new_node[0], *current_param1, name_len + 1);
        
        new_node[1] = executeMacro(current_param1[1], NULL); // Recursive call
        new_node[2] = (char *)result_list_head;
        result_list_head = new_node;
      }
    }
    reverseObjectList(&result_list_head); // Pass address of head pointer
    param_2 = result_list_head;
  }
  return param_2; // Return the new list (or original param_2 if param_1 was NULL)
}

// Function: getNextInputLine
// param_1 is char ** because it's modified (pointer advanced)
char * getNextInputLine(char **param_1) {
  if (*param_1 == NULL || **param_1 == '\0') { // Check for empty or null string
      return NULL;
  }
  char *newline_pos = strchr(*param_1, '\n');
  size_t line_len;
  if (newline_pos == NULL) {
    line_len = strlen(*param_1); // No newline, take entire string
  } else {
    line_len = newline_pos - *param_1 + 1; // Include newline character
  }
  
  char *result_line = (char *)malloc(line_len + 1);
  if (result_line == NULL) {
    return NULL;
  }
  memset(result_line, 0, line_len + 1);
  memcpy(result_line, *param_1, line_len);
  
  *param_1 += line_len; // Advance the pointer in the input buffer
  return result_line;
}

// Function: getObject
// param_1: char ***list_head_ptr (pointer to the head of the list where new object is added)
// param_2: const char *stop_tag (string to compare against, e.g., "</page>")
// param_3: char **input_buffer_ptr (NULL for stdin, or pointer to buffer that getNextInputLine will modify)
void ** getObject(char ***list_head_ptr, const char *stop_tag, char **input_buffer_ptr) {
  char *line_buffer = NULL;
  void **new_object = NULL;
  
  // Read input line
  if (input_buffer_ptr == NULL) { // Read from stdin
    line_buffer = (char *)malloc(1024); // 0x400
    if (line_buffer == NULL) return NULL;
    if (fgets(line_buffer, 1024, stdin) == NULL) {
      free(line_buffer);
      return NULL; // Error or EOF
    }
    size_t len = strlen(line_buffer);
    if (len > 0 && line_buffer[len - 1] == '\n') {
      line_buffer[len - 1] = '\0'; // Remove trailing newline
    }
    if (len == 0 && feof(stdin)) { // Empty line at EOF
        free(line_buffer);
        return NULL;
    }
  } else { // Read from provided buffer
    line_buffer = getNextInputLine(input_buffer_ptr);
    if (line_buffer == NULL) return NULL; // End of buffer or error
    size_t len = strlen(line_buffer);
    if (len > 0 && line_buffer[len - 1] == '\n') {
      line_buffer[len - 1] = '\0'; // Remove trailing newline
    }
  }

  // Check if it's a stop tag
  if (stop_tag != NULL && strcmp(line_buffer, stop_tag) == 0) {
    free(line_buffer);
    return NULL; // Found stop tag, signal end of list
  }

  // Allocate new object node
  new_object = (void **)malloc(sizeof(void*) * 3); // [name, children_list_head, next_sibling]
  if (new_object == NULL) {
    free(line_buffer);
    return NULL;
  }
  new_object[1] = NULL; // children_list_head
  new_object[2] = NULL; // next_sibling

  size_t line_len = strlen(line_buffer);
  
  // Check for <tag>...</tag> format
  if (*line_buffer == '<' && line_buffer[line_len - 1] == '>') {
    // It's a tag, extract tag name
    size_t tag_name_len = line_len - 2; // Exclude '<' and '>'
    char *tag_name = (char *)malloc(tag_name_len + 1);
    if (tag_name == NULL) {
      free(new_object); free(line_buffer); return NULL;
    }
    memcpy(tag_name, line_buffer + 1, tag_name_len);
    tag_name[tag_name_len] = '\0';
    new_object[0] = tag_name; // Set object name

    // Construct the closing tag string for recursive calls
    size_t close_tag_len = strlen(tag_name) + 4; // "</" + tag_name + ">" + '\0'
    char *closing_tag = (char *)malloc(close_tag_len);
    if (closing_tag == NULL) {
      free(tag_name); free(new_object); free(line_buffer); return NULL;
    }
    sprintf(closing_tag, "</%s>", tag_name);
    
    // Recursively get child objects
    char **child_list_head = NULL;
    void **child_object;
    while ((child_object = getObject(&child_list_head, closing_tag, input_buffer_ptr)) != NULL) {
      // getObject prepends. The last object read is the first in the child_list_head.
      // So, child_list_head points to the newest child.
    }
    reverseObjectList(&child_list_head); // Reverse children list to maintain original order
    new_object[1] = child_list_head; // Assign reversed list to parent
    
    free(closing_tag);
  } else {
    // It's a plain text element
    new_object[0] = (char *)malloc(line_len + 1);
    if (new_object[0] == NULL) {
      free(new_object); free(line_buffer); return NULL;
    }
    memcpy(new_object[0], line_buffer, line_len + 1);
  }
  
  free(line_buffer);

  // Add new_object to the list_head_ptr (prepends to the list)
  if (list_head_ptr != NULL) {
      new_object[2] = *list_head_ptr;
      *list_head_ptr = new_object;
  }

  return new_object; // Return the newly created object (which is also the new head of the list)
}

// Function: getMacro
// param_1: head of macro list (char**)
// param_2: macro name (char*)
char ** getMacro(char **param_1, char *param_2) {
  char **current_macro = param_1;
  while (current_macro != NULL) {
    if (strcmp((char*)current_macro[2], param_2) == 0) { // Macro name is stored at index 2
      return current_macro;
    }
    current_macro = (char **)current_macro[1]; // Next macro is at index 1
  }
  return NULL;
}

// Function: table
char * table(void* macro_list_head, void* object_node) {
  char **macro_head = (char**)macro_list_head;
  char **object_params = (char**)((char**)object_node)[1]; // Children of table tag

  char **row_objects_head = NULL; // List of "row" objects
  uint num_rows = 0;
  uint num_fields = 0;
  bool use_star_border = false;
  
  // Parse table parameters (rows, fields, border, row objects)
  for (char **current_param = object_params; current_param != NULL; current_param = (char **)current_param[2]) {
    if (strcmp(*current_param, "rows") == 0) {
      if ((char**)current_param[1] != NULL) {
        num_rows = atoi((char*)(*current_param[1]));
      }
    } else if (strcmp(*current_param, "fields") == 0) {
      if ((char**)current_param[1] != NULL) {
        num_fields = atoi((char*)(*current_param[1]));
      }
    } else if (strcmp(*current_param, "border") == 0) {
      char **border_type = (char**)current_param[1];
      if (border_type != NULL) {
        if (strcmp(*border_type, "star") == 0) {
          use_star_border = true;
        }
      }
    } else if (strcmp(*current_param, "row") == 0) {
      current_param[2] = (char*)row_objects_head; // Prepend row object to list
      row_objects_head = current_param;
    }
  }
  reverseObjectList(&row_objects_head); // Restore original order of rows

  if (num_rows == 0 || num_fields == 0 || columnWidth < num_fields * 4 + 1) {
    return NULL;
  }

  uint cell_width = (columnWidth - 1) / num_fields;
  uint inner_cell_width = cell_width - 3; // For padding and border char
  uint total_table_width = cell_width * num_fields + 1; // Includes outer borders

  uint current_table_height = num_rows * 2 + 1; // Rows for data + rows for borders
  char **table_lines = (char **)malloc(current_table_height * sizeof(char*));
  if (table_lines == NULL) return NULL;

  for (uint i = 0; i < current_table_height; i++) {
    table_lines[i] = (char *)malloc(total_table_width + 4); // +4 for safety (null terminator + padding)
    if (table_lines[i] == NULL) {
      for (uint j = 0; j < i; j++) free(table_lines[j]);
      free(table_lines);
      return NULL;
    }
    memset(table_lines[i], 0, total_table_width + 4);
  }

  uint current_line_idx = 0;
  char **current_row = row_objects_head;

  macro_handler_func_ptr get_text_macro = (macro_handler_func_ptr)getMacro(macro_head, (char*)DAT_00018309); // "text"
  macro_handler_func_ptr get_pgraph_macro = (macro_handler_func_ptr)getMacro(macro_head, "pgraph"); // Assuming this is also a macro

  for (uint r = 0; r < num_rows; r++) {
    // Draw horizontal border
    char border_char = use_star_border ? '*' : '+';
    char fill_char = use_star_border ? '*' : '-';
    for (uint f = 0; f < num_fields; f++) {
      strncat(table_lines[current_line_idx], &border_char, 1);
      memset(table_lines[current_line_idx] + strlen(table_lines[current_line_idx]), fill_char, cell_width - 1);
    }
    strncat(table_lines[current_line_idx], &border_char, 1);
    current_line_idx++;

    // Process fields for the current row
    char **current_field_list = NULL;
    if (current_row != NULL) {
      reverseObjectList((char***)&current_row[1]); // Reverse children of 'row'
      current_field_list = (char**)current_row[1];
    }

    uint max_field_lines = 1; // Track max lines needed for any field in this row
    
    char **field_contents = (char**)calloc(num_fields, sizeof(char*));
    uint *field_num_lines = (uint*)calloc(num_fields, sizeof(uint));

    if (!field_contents || !field_num_lines) { // Handle allocation failure for field data
        // Cleanup and return NULL
        for(uint j=0; j<num_fields; ++j) {
            free(field_contents[j]);
        }
        free(field_contents); free(field_num_lines);
        for (uint j = 0; j < current_table_height; j++) free(table_lines[j]);
        free(table_lines);
        return NULL;
    }

    for (uint f = 0; f < num_fields; f++) {
        char *cell_data = NULL;
        if (current_field_list != NULL) {
            if (strcmp(*current_field_list, "header") == 0) {
                cell_data = get_text_macro(macro_head, current_field_list[1]);
            } else if (strcmp(*current_field_list, "field") == 0) {
                cell_data = get_pgraph_macro(macro_head, current_field_list[1]);
            }
        }
        if (cell_data == NULL) { // Default to empty string if no data or macro not found
            cell_data = (char *)malloc(inner_cell_width + 1);
            if (cell_data == NULL) { /* cleanup */ return NULL; }
            memset(cell_data, ' ', inner_cell_width);
            cell_data[inner_cell_width] = '\0';
        }
        field_contents[f] = cell_data;

        // Calculate lines needed for this field
        uint current_field_lines = 0;
        size_t data_len = strlen(cell_data);
        if (data_len > 0) {
            current_field_lines = (data_len + inner_cell_width - 1) / inner_cell_width;
        } else {
            current_field_lines = 1; // Even empty cells take one line
        }

        field_num_lines[f] = current_field_lines;
        if (current_field_lines > max_field_lines) {
            max_field_lines = current_field_lines;
        }

        if (current_field_list != NULL) {
            current_field_list = (char**)current_field_list[2]; // Move to next field in row
        }
    }

    // Resize table_lines array if needed
    if (max_field_lines > 1) {
        uint lines_to_add = max_field_lines - 1;
        uint new_table_height = current_table_height + lines_to_add;
        char **new_table_lines = (char **)realloc(table_lines, new_table_height * sizeof(char*));
        if (new_table_lines == NULL) {
            // Cleanup and return NULL
            for(uint j=0; j<num_fields; ++j) { free(field_contents[j]); } free(field_contents); free(field_num_lines);
            for (uint j = 0; j < current_table_height; j++) free(table_lines[j]); free(table_lines);
            return NULL;
        }
        table_lines = new_table_lines;
        for (uint i = current_table_height; i < new_table_height; i++) {
            table_lines[i] = (char *)malloc(total_table_width + 4);
            if (table_lines[i] == NULL) { /* cleanup */ return NULL; }
            memset(table_lines[i], 0, total_table_width + 4);
        }
        current_table_height = new_table_height;
    }

    // Fill data lines
    for (uint line_in_field = 0; line_in_field < max_field_lines; line_in_field++) {
        char vertical_border_char = use_star_border ? '*' : '|';
        for (uint f = 0; f < num_fields; f++) {
            strncat(table_lines[current_line_idx], &vertical_border_char, 1);
            strcat(table_lines[current_line_idx], " ");

            char temp_buf[inner_cell_width + 1];
            memset(temp_buf, ' ', inner_cell_width);
            temp_buf[inner_cell_width] = '\0';

            if (line_in_field < field_num_lines[f]) {
                char *field_str = field_contents[f];
                size_t offset = line_in_field * inner_cell_width;
                size_t chars_to_copy = inner_cell_width;
                if (offset + chars_to_copy > strlen(field_str)) {
                    chars_to_copy = strlen(field_str) - offset;
                }
                if (chars_to_copy > 0) {
                    memcpy(temp_buf, field_str + offset, chars_to_copy);
                }
            }
            strcat(table_lines[current_line_idx], temp_buf);
            strcat(table_lines[current_line_idx], " ");
        }
        strncat(table_lines[current_line_idx], &vertical_border_char, 1);
        current_line_idx++;
    }

    // Free field contents
    for (uint f = 0; f < num_fields; f++) {
        free(field_contents[f]);
    }
    free(field_contents);
    free(field_num_lines);

    if (current_row != NULL) {
      current_row = (char**)current_row[2]; // Move to next row
    }
  }

  // Draw final horizontal border
  char border_char = use_star_border ? '*' : '+';
  char fill_char = use_star_border ? '*' : '-';
  for (uint f = 0; f < num_fields; f++) {
    strncat(table_lines[current_line_idx], &border_char, 1);
    memset(table_lines[current_line_idx] + strlen(table_lines[current_line_idx]), fill_char, cell_width - 1);
  }
  strncat(table_lines[current_line_idx], &border_char, 1);
  current_line_idx++; // Increment for final line

  // Concatenate all lines into final result string
  size_t total_output_len = 0;
  for (uint i = 0; i < current_table_height; i++) {
    total_output_len += strlen(table_lines[i]) + 1; // +1 for newline
  }
  char *final_output = (char *)malloc(total_output_len + 1);
  if (final_output == NULL) {
    for (uint i = 0; i < current_table_height; i++) free(table_lines[i]);
    free(table_lines);
    return NULL;
  }
  memset(final_output, 0, total_output_len + 1);

  for (uint i = 0; i < current_table_height; i++) {
    strcat(final_output, table_lines[i]);
    strcat(final_output, "\n");
    free(table_lines[i]); // Free individual line after copying
  }
  free(table_lines); // Free array of pointers

  return final_output;
}

// Function: element
char * element(void* macro_list_head, void* object_node) {
  char **macro_head = (char**)macro_list_head;
  char **object_params = (char**)((char**)object_node)[1]; // Children of element tag

  char *text_content = NULL;
  char *list_content = NULL;
  
  macro_handler_func_ptr get_text_macro = (macro_handler_func_ptr)getMacro(macro_head, (char*)DAT_00018309); // "text"
  macro_handler_func_ptr get_list_macro = (macro_handler_func_ptr)getMacro(macro_head, (char*)DAT_0001834d); // "list"

  for (char **current_param = object_params; current_param != NULL; current_param = (char **)current_param[2]) {
    if (strcmp(*current_param, "text") == 0) {
      text_content = get_text_macro(macro_head, current_param);
    } else if (strcmp(*current_param, "list") == 0) {
      list_content = get_list_macro(macro_head, current_param);
    }
  }

  if (text_content == NULL && list_content == NULL) {
      return NULL;
  }

  size_t total_len = 0;
  if (text_content != NULL) total_len += strlen(text_content);
  if (list_content != NULL) total_len += strlen(list_content) + (text_content != NULL ? 1 : 0); // +1 for potential newline

  char *result_string = (char *)malloc(total_len + 1);
  if (result_string == NULL) {
    free(text_content); free(list_content);
    return NULL;
  }
  memset(result_string, 0, total_len + 1);

  if (text_content != NULL) {
    strcat(result_string, text_content);
    free(text_content);
  }

  if (list_content != NULL) {
    if (strlen(result_string) > 0) { // If there's text, add newline before list
        strcat(result_string, "\n");
    }
    char *list_token_copy = strdup(list_content);
    if (list_token_copy == NULL) {
        free(result_string); free(list_content); return NULL;
    }
    char *token = strtok(list_token_copy, "\n");
    while (token != NULL) {
      strcat(result_string, "  "); // Indent list items
      strcat(result_string, token);
      strcat(result_string, "\n");
      token = strtok(NULL, "\n");
    }
    free(list_content);
    free(list_token_copy);
  }
  
  return result_string;
}

// Function: list
char * list(void* macro_list_head, void* object_node) {
  char **macro_head = (char**)macro_list_head;
  char **object_params = (char**)((char**)object_node)[1]; // Children of list tag

  char *current_list_output_buffer = NULL;
  
  char list_type_char[16] = {0}; // Increased size for custom types
  uint list_type_enum = 0x2a; // Default to '*' (bullet)

  // Collect element nodes and parse list type
  char **element_objects_head = NULL;
  for (char **current_param = object_params; current_param != NULL; current_param = (char **)current_param[2]) {
    if (strcmp(*current_param, "type") == 0) {
      char **type_value = (char**)current_param[1];
      if (type_value != NULL) {
        if (strcmp(*type_value, "numeral") == 0) {
          list_type_enum = 0; // 1, 2, 3...
        } else if (strcmp(*type_value, "ALPHA") == 0) {
          list_type_enum = 1; // A, B, C...
        } else if (strcmp(*type_value, "alpha") == 0) {
          list_type_enum = 2; // a, b, c...
        } else if (strcmp(*type_value, "roman") == 0) {
          list_type_enum = 3; // I, II, III...
        } else {
          strncpy(list_type_char, *type_value, sizeof(list_type_char) - 1); // Custom char
          list_type_enum = 4; // Indicate custom char type
        }
      }
    } else if (strcmp(*current_param, "element") == 0) {
      current_param[2] = (char*)element_objects_head; // Prepend element to list
      element_objects_head = current_param;
    }
  }
  reverseObjectList(&element_objects_head); // Restore original order

  macro_handler_func_ptr get_element_macro = (macro_handler_func_ptr)getMacro(macro_head, "element");

  int item_count = 0;
  for (char **current_element = element_objects_head; current_element != NULL; current_element = (char **)current_element[2]) {
    item_count++;
    char *element_output = NULL;
    if (strcmp(*current_element, "element") == 0) {
      element_output = get_element_macro(macro_head, current_element);
    }
    
    if (element_output == NULL) {
        continue; // Skip if element output is empty
    }

    char item_prefix[32]; // Buffer for "1.", "A.", "I." etc.
    memset(item_prefix, 0, sizeof(item_prefix));

    if (list_type_enum == 0) { // Numerals
      custom_itoa(item_count, item_prefix);
      strcat(item_prefix, ".");
    } else if (list_type_enum == 1) { // ALPHA
      sprintf(item_prefix, "%c.", (char)item_count + '@');
    } else if (list_type_enum == 2) { // alpha
      sprintf(item_prefix, "%c.", (char)item_count + '`');
    } else if (list_type_enum == 3) { // Roman
      char numeral_buf[16]; // Max length for Roman numeral up to 999 is "CMXCIX" (7 chars)
      char *roman_val = romanNumeral(item_count);
      if (roman_val != NULL) {
          strncpy(numeral_buf, roman_val, sizeof(numeral_buf) - 1);
          free(roman_val);
      } else {
          strncpy(numeral_buf, "?", sizeof(numeral_buf) - 1);
      }
      strcat(item_prefix, numeral_buf);
      strcat(item_prefix, "."); // Original code adds dot for roman too
    } else { // Custom char (e.g., '*')
      strncpy(item_prefix, list_type_char, sizeof(item_prefix) - 1);
    }
    
    size_t prefix_len = strlen(item_prefix);
    size_t element_len = strlen(element_output);
    
    // Calculate required size for this item
    size_t item_total_len = prefix_len + 1 + element_len + 1; // prefix + space + element_output + newline

    // Reallocate or allocate result buffer
    size_t current_total_len = (current_list_output_buffer != NULL) ? strlen(current_list_output_buffer) : 0;
    char *new_buffer = (char *)realloc(current_list_output_buffer, current_total_len + item_total_len + 1);
    if (new_buffer == NULL) {
      free(current_list_output_buffer); free(element_output);
      return NULL;
    }
    current_list_output_buffer = new_buffer;
    
    // Append current item to buffer
    strcat(current_list_output_buffer, item_prefix);
    strcat(current_list_output_buffer, " ");
    strcat(current_list_output_buffer, element_output);
    strcat(current_list_output_buffer, "\n");

    free(element_output);
  }
  
  return current_list_output_buffer;
}

// Function: text
// Returns the actual text string from the object node's children list head
char * text(void* macro_list_head, void* object_node) {
  char **object_children = (char**)((char**)object_node)[1]; // Children of text tag
  if (object_children == NULL) {
    return NULL;
  }
  return strdup(*object_children); // Return a duplicate of the text content
}

// Function: paragraph
char * paragraph(void* macro_list_head, void* object_node) {
  char **macro_head = (char**)macro_list_head;
  char **object_children = (char**)((char**)object_node)[1]; // Children of pgraph tag

  char *result_string = NULL;
  
  if (object_children == NULL) {
    return NULL;
  }

  macro_handler_func_ptr get_text_macro = (macro_handler_func_ptr)getMacro(macro_head, (char*)DAT_00018309); // "text"
  macro_handler_func_ptr get_list_macro = (macro_handler_func_ptr)getMacro(macro_head, (char*)DAT_0001834d); // "list"
  macro_handler_func_ptr get_table_macro = (macro_handler_func_ptr)getMacro(macro_head, "table");

  if (strcmp(*object_children, "text") == 0) {
    char *content = get_text_macro(macro_head, object_children);
    if (content != NULL) {
      size_t len = strlen(content);
      result_string = (char *)malloc(len + 6); // "     " + content + '\0'
      if (result_string != NULL) {
        sprintf(result_string, "     %s", content);
      }
      free(content);
    }
  } else if (strcmp(*object_children, "list") == 0) {
    char *content = get_list_macro(macro_head, object_children);
    if (content != NULL) {
      result_string = strdup(content);
      free(content);
    }
  } else if (strcmp(*object_children, "table") == 0) {
    char *content = get_table_macro(macro_head, object_children);
    if (content != NULL) {
      result_string = strdup(content);
      free(content);
    }
  }
  return result_string;
}

// Function: page
char * page(void* macro_list_head, void* object_node) {
  char **macro_head = (char**)macro_list_head;
  char **object_children = (char**)((char**)object_node)[1]; // Children of page tag

  char *current_output_buffer = NULL;

  // Reverse children list to process in original order
  char **pgraph_objects_head = NULL;
  for (char **current_child = object_children; current_child != NULL; current_child = (char **)current_child[2]) {
    current_child[2] = (char*)pgraph_objects_head; // Prepend
    pgraph_objects_head = current_child;
  }
  reverseObjectList(&pgraph_objects_head); // Restore original order

  macro_handler_func_ptr get_pgraph_macro = (macro_handler_func_ptr)getMacro(macro_head, "pgraph");

  for (char **current_pgraph = pgraph_objects_head; current_pgraph != NULL; current_pgraph = (char **)current_pgraph[2]) {
    if (strcmp(*current_pgraph, "pgraph") == 0) {
      char *pgraph_output = get_pgraph_macro(macro_head, current_pgraph);
      if (pgraph_output != NULL) {
        size_t pgraph_len = strlen(pgraph_output);
        size_t current_total_len = (current_output_buffer != NULL) ? strlen(current_output_buffer) : 0;
        size_t required_len = pgraph_len + 1; // +1 for newline

        char *new_buffer = (char *)realloc(current_output_buffer, current_total_len + required_len + 1);
        if (new_buffer == NULL) {
          free(current_output_buffer); free(pgraph_output);
          return NULL;
        }
        current_output_buffer = new_buffer;

        strcat(current_output_buffer, pgraph_output);
        strcat(current_output_buffer, "\n");
        free(pgraph_output);
      }
    }
  }
  return current_output_buffer;
}

// Function: document
// Returns 0 on success, non-zero on error.
int document(void* macro_list_head, void* object_node) {
  char **macro_head = (char**)macro_list_head;
  char **object_children = (char**)((char**)object_node)[1]; // Children of document tag

  char **page_column_objects_head = NULL; // List of "page" or "column" objects
  uint page_length = 0xb; // Default length: 11 lines
  uint page_width = 0x14; // Default width: 20 characters
  
  // Parse document parameters (length, width, page/column objects)
  for (char **current_child = object_children; current_child != NULL; current_child = (char **)current_child[2]) {
    if (strcmp(*current_child, "page") == 0 || strcmp(*current_child, "column") == 0) {
      current_child[2] = (char*)page_column_objects_head; // Prepend
      page_column_objects_head = current_child;
    } else if (strcmp(*current_child, "length") == 0) {
      if ((char**)current_child[1] != NULL) {
        page_length = atoi((char*)(*current_child[1]));
      }
    } else if (strcmp(*current_child, "width") == 0) {
      if ((char**)current_child[1] != NULL) {
        page_width = atoi((char*)(*current_child[1]));
      }
    }
  }
  reverseObjectList(&page_column_objects_head); // Restore original order

  char **page_lines = (char **)malloc(page_length * sizeof(char*));
  if (page_lines == NULL) return 1;

  for (uint i = 0; i < page_length; i++) {
    page_lines[i] = (char *)malloc(page_width + 4); // +4 for safety
    if (page_lines[i] == NULL) {
      for (uint j = 0; j < i; j++) free(page_lines[j]);
      free(page_lines);
      return 1;
    }
    memset(page_lines[i], 0, page_width + 4);
  }

  columnWidth = page_width; // Set global column width for other functions

  macro_handler_func_ptr get_page_macro = (macro_handler_func_ptr)getMacro(macro_head, (char*)DAT_0001838d); // "page"
  macro_handler_func_ptr get_column_macro = (macro_handler_func_ptr)getMacro(macro_head, "column"); // This actually uses 'text' macro

  int doc_page_num = 0;
  char *current_document_output = NULL;

  for (char **current_obj = page_column_objects_head; current_obj != NULL; current_obj = (char **)current_obj[2]) {
    uint num_columns = 1; // Default to 1 column
    int effective_column_width = page_width;

    if (strcmp(*current_obj, "column") == 0) {
      char *col_count_str = get_column_macro(macro_head, current_obj); // Expects <column>N</column>
      if (col_count_str != NULL) {
        num_columns = atoi(col_count_str);
        free(col_count_str);
        if (num_columns == 0) num_columns = 1; // Prevent division by zero
      }
      
      size_t column_spacing_len = strlen("    "); // 4 spaces
      if (page_width / num_columns <= column_spacing_len) { // If columns are too narrow
        num_columns = 1; // Revert to single column
      }
      effective_column_width = page_width / num_columns - (num_columns > 1 ? column_spacing_len : 0);
    } else if (strcmp(*current_obj, "page") == 0) {
      char *page_content = get_page_macro(macro_head, current_obj);
      if (page_content != NULL) {
        doc_page_num++;
        char *page_content_copy = strdup(page_content); // strtok modifies string
        if (page_content_copy == NULL) { free(page_content); return 1; }

        char *line_token = strtok(page_content_copy, "\n");
        uint current_line_in_page = 0;
        uint current_column = 1;

        while (line_token != NULL) {
          // Check if current line exceeds page length (excluding last line for page number)
          if (current_line_in_page >= page_length -1) { 
            // Page is full, print it and start a new one
            char page_num_str[16];
            custom_itoa(doc_page_num, page_num_str);
            
            size_t current_doc_output_len = (current_document_output != NULL) ? strlen(current_document_output) : 0;
            size_t page_output_len = (page_width + 1) * (page_length + 2) + strlen(page_num_str) + 1;

            current_document_output = (char *)realloc(current_document_output, current_doc_output_len + page_output_len + 1);
            if(current_document_output == NULL) { free(page_content); free(page_content_copy); return 1; }
            current_document_output[current_doc_output_len] = '\0'; // Ensure null termination

            char temp_page_buf[page_output_len + 1];
            memset(temp_page_buf, 0, sizeof(temp_page_buf));

            memset(temp_page_buf, '=', page_width);
            strcat(temp_page_buf, "\n");

            for (uint i = 0; i < page_length; i++) {
              strcat(temp_page_buf, page_lines[i]);
              strcat(temp_page_buf, "\n");
              memset(page_lines[i], 0, page_width + 4); // Clear line for next page
            }
            
            memset(temp_page_buf + strlen(temp_page_buf), '=', page_width);
            strcat(temp_page_buf, "\n");

            char footer_line[page_width + 1];
            memset(footer_line, ' ', page_width);
            footer_line[page_width] = '\0';
            size_t page_num_len = strlen(page_num_str);
            size_t start_pos = (page_width - page_num_len) / 2;
            memcpy(footer_line + start_pos, page_num_str, page_num_len);
            strcat(temp_page_buf, footer_line);
            strcat(temp_page_buf, "\n");

            strcat(current_document_output, temp_page_buf);

            current_line_in_page = 0;
            current_column = 1;
          }

          // Add line to current page layout
          char line_buf[effective_column_width + 1];
          size_t remaining_len = strlen(line_token);
          char *current_pos_in_token = line_token;

          while (remaining_len > 0) {
            size_t copy_len = remaining_len > effective_column_width ? effective_column_width : remaining_len;
            strncpy(line_buf, current_pos_in_token, copy_len);
            line_buf[copy_len] = '\0';
            
            strcat(page_lines[current_line_in_page], line_buf);
            for (uint i = 0; i < effective_column_width - copy_len; i++) {
                strcat(page_lines[current_line_in_page], " ");
            }
            if (num_columns > 1 && current_column < num_columns) {
                strcat(page_lines[current_line_in_page], "    "); // Column separator
            }
            
            remaining_len -= copy_len;
            current_pos_in_token += copy_len;

            if (remaining_len > 0) { // If more text for this line, go to next column or next line
                current_column++;
                if (current_column > num_columns) {
                    current_column = 1;
                    current_line_in_page++;
                    if (current_line_in_page >= page_length -1) { // If page is full, break and print
                        break;
                    }
                }
                // Clear the current line for the next column's content
                memset(page_lines[current_line_in_page], 0, page_width + 4);
            }
          }
          current_line_in_page++;
          line_token = strtok(NULL, "\n");
        }
        free(page_content);
        free(page_content_copy);
      }
    }
  }

  // Print remaining content if any
  if (doc_page_num > 0 || strlen(page_lines[0]) > 0) { // If there was content
      doc_page_num++;
      char page_num_str[16];
      custom_itoa(doc_page_num, page_num_str);
      
      size_t current_doc_output_len = (current_document_output != NULL) ? strlen(current_document_output) : 0;
      size_t required_current_page_len = (page_width + 1) * (page_length + 2) + strlen(page_num_str) + 1; // +2 for borders

      current_document_output = (char *)realloc(current_document_output, current_doc_output_len + required_current_page_len + 1);
      if(current_document_output == NULL) { return 1; }
      current_document_output[current_doc_output_len] = '\0'; // Ensure null termination
      
      char *temp_buffer = (char*)malloc(required_current_page_len + 1);
      if(!temp_buffer) { return 1; }
      memset(temp_buffer, 0, required_current_page_len + 1);

      memset(temp_buffer, '=', page_width);
      strcat(temp_buffer, "\n");

      for (uint i = 0; i < page_length; i++) {
        strcat(temp_buffer, page_lines[i]);
        strcat(temp_buffer, "\n");
      }
      
      memset(temp_buffer + strlen(temp_buffer), '=', page_width);
      strcat(temp_buffer, "\n");
      
      char footer_line[page_width + 1];
      memset(footer_line, ' ', page_width);
      footer_line[page_width] = '\0';
      size_t page_num_len = strlen(page_num_str);
      size_t start_pos = (page_width - page_num_len) / 2;
      memcpy(footer_line + start_pos, page_num_str, page_num_len);
      strcat(temp_buffer, footer_line);
      strcat(temp_buffer, "\n");

      strcat(current_document_output, temp_buffer);
      free(temp_buffer);
  }

  if (current_document_output != NULL) {
    transmit_all(1, current_document_output, strlen(current_document_output));
    transmit_all(1, DAT_000183ab, strlen(DAT_000183ab)); // End of Text character
    free(current_document_output);
  }

  // Free page_lines array
  for (uint i = 0; i < page_length; i++) {
    free(page_lines[i]);
  }
  free(page_lines);

  return 0;
}

// Function: newMacro
// Returns a pointer to the newly created macro node (char**).
// macro_node: [func_ptr, next_macro, name_string]
char ** newMacro(const char *name, macro_handler_func_ptr func_ptr) {
  char **macro_node = (char **)malloc(sizeof(char*) * 3);
  if (macro_node == NULL) {
    return NULL;
  }
  
  macro_node[0] = (char*)func_ptr; // Store function pointer
  macro_node[1] = NULL; // Next macro in list
  
  size_t name_len = strlen(name);
  macro_node[2] = (char *)malloc(name_len + 1); // Store name string
  if (macro_node[2] == NULL) {
    free(macro_node);
    return NULL;
  }
  memcpy(macro_node[2], name, name_len + 1);
  
  return macro_node;
}

// Function: initMacros
void initMacros(char ***macro_list_head_ptr) { // Takes pointer to head of macro list
  char **new_macro_node;

  new_macro_node = newMacro("element", element);
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;

  new_macro_node = newMacro("document", (macro_handler_func_ptr)document); // Cast to func_ptr
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;

  new_macro_node = newMacro((char*)DAT_0001838d, page); // "page"
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;

  new_macro_node = newMacro("pgraph", paragraph);
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;

  new_macro_node = newMacro((char*)DAT_00018309, (macro_handler_func_ptr)text); // "text" - cast to func_ptr
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;

  new_macro_node = newMacro((char*)DAT_0001834d, (macro_handler_func_ptr)list); // "list" - cast to func_ptr
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;
  
  // 'column' uses 'text' handler. So it expects <column>N</column>
  new_macro_node = newMacro("column", (macro_handler_func_ptr)text);
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;

  new_macro_node = newMacro("table", table);
  new_macro_node[1] = *macro_list_head_ptr;
  *macro_list_head_ptr = new_macro_node;
}

// Function: initCustomMacros
void initCustomMacros(char ***custom_macro_list_head_ptr) {
  char *input_buffer;

  input_buffer = (char*)FourByFourTable_Macro;
  getObject(custom_macro_list_head_ptr, NULL, &input_buffer);
  
  input_buffer = (char*)FiveByFiveTable_Macro;
  getObject(custom_macro_list_head_ptr, NULL, &input_buffer);
  
  input_buffer = (char*)AlphanumericOutline_Macro;
  getObject(custom_macro_list_head_ptr, NULL, &input_buffer);
  
  input_buffer = (char*)BulletedOutline_Macro;
  getObject(custom_macro_list_head_ptr, NULL, &input_buffer);
}

// Function: sendDocumentID
void sendDocumentID(uint param_1) {
  char id_str[20]; // Buffer for integer to string conversion
  custom_itoa(param_1, id_str);
  
  transmit_all(1, "Document ID: ", strlen("Document ID: "));
  transmit_all(1, id_str, strlen(id_str));
  transmit_all(1, DAT_0001834b, strlen(DAT_0001834b)); // Newline
}

// Function: main
int main(void) {
  char **built_in_macros_head = NULL;
  char **custom_macros_head = NULL;
  char **document_object_tree = NULL;
  int doc_id;
  
  doc_id = getDocumentID();

  initMacros(&built_in_macros_head);
  initCustomMacros(&custom_macros_head);
  
  // getObject to read the main document structure from stdin
  // The 'document_object_tree' will point to the head of the list of top-level objects parsed from stdin.
  // These objects are also added to 'custom_macros_head' as per the original snippet's logic.
  // This is a potential design issue if custom_macros_head should only contain macros.
  document_object_tree = (char**)getObject(&custom_macros_head, NULL, NULL); 
  
  sendDocumentID(doc_id);
  
  // Execute the 'document' macro on the parsed document object tree
  ((macro_handler_func_ptr)getMacro(built_in_macros_head, "document")[0])(built_in_macros_head, document_object_tree);

  // TODO: Add cleanup for document_object_tree and macro lists (free all nodes and their contents)

  return 0;
}