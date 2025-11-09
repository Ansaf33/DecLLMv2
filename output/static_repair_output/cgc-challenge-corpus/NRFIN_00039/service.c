#include <stdlib.h> // For malloc, free, atoi
#include <string.h> // For memset, strcpy, strncmp, strlen
#include <unistd.h> // For recv
#include <sys/types.h> // For size_t etc. for recv
#include <sys/socket.h> // For recv flags (though 0x1100c is likely custom)

// Placeholder for external functions
// Assuming 32-bit system for pointer sizes matching 'int' where casts occur
extern int startswith(const char *str, const char *prefix);
extern int transmit_all(int arg1, int arg2, int arg3, int arg4); // Inferred signature
extern void _terminate(void);

// Global data from the original snippet
const char DAT_00013000[] = "EXIT";
const char DAT_0001300b[] = "SORT";

// Using standard integer types for undefinedX to ensure fixed sizes
typedef int uint32_t_alias; // For original undefined4
typedef short uint16_t_alias; // For original undefined2
typedef char uint8_t_alias; // For original undefined

// Structure for report segments (used in new_nodes_head list)
// These nodes define how to parse and interpret parts of a record string.
typedef struct ReportSegmentNode {
    struct ReportSegmentNode *next;
    uint16_t_alias start_offset; // Offset within the record string
    uint16_t_alias length;       // Length of this segment
} ReportSegmentNode; // Total 8 bytes on 32-bit: 4 (ptr) + 2 (short) + 2 (short)

// Structure for report records (used in report_head list)
// These nodes represent individual data records.
typedef struct ReportRecordNode {
    struct ReportRecordNode *next;
    char *data_ptr; // Points to the allocated string data for this record
} ReportRecordNode; // Total 8 bytes on 32-bit: 4 (ptr) + 4 (ptr)

// Main context structure to hold report lists and metadata
// This structure maps to the stack variables used in `main`
typedef struct ReportContext {
    ReportRecordNode *report_head;      // Corresponds to main's local_20 (list of actual records)
    ReportSegmentNode *new_nodes_head;  // Corresponds to main's local_1c (list of segment definitions)
    uint16_t_alias total_record_length; // Corresponds to main's local_18 (total length of a full record string)
    uint32_t_alias unused_field_14;     // Corresponds to main's local_14 (or padding)
} ReportContext;


// Function: newReport
// Initializes the report context by defining segments based on input characters.
void newReport(ReportContext *report_ctx) {
  int flags = 0x1100c;
  char current_char = '\0';
  int char_count = 0;
  int last_colon_pos = 0;
  int num_colons = 0;
  
  // report_ctx->new_nodes_head is already NULL from ReportContext initialization.

  while (current_char != ';') {
    if (current_char == ':') {
      ReportSegmentNode *node = (ReportSegmentNode *)malloc(sizeof(ReportSegmentNode));
      if (node == NULL) { /* Handle malloc error */ return; }
      node->start_offset = last_colon_pos - num_colons;
      node->length = (char_count - last_colon_pos) - 1;
      node->next = report_ctx->new_nodes_head;
      report_ctx->new_nodes_head = node;
      last_colon_pos = char_count;
      num_colons++;
    }
    recv(0, &current_char, 1, flags);
    char_count++;
  }
  
  // Create the final segment node after ';'
  ReportSegmentNode *node = (ReportSegmentNode *)malloc(sizeof(ReportSegmentNode));
  if (node == NULL) { /* Handle malloc error */ return; }
  node->start_offset = last_colon_pos - num_colons;
  node->length = (char_count - last_colon_pos) - 1;
  node->next = report_ctx->new_nodes_head;
  report_ctx->new_nodes_head = node;
  
  report_ctx->total_record_length = char_count - (num_colons + 1);
}

// Function: sendReport
// Sends the report records by iterating through the list and calling transmit_all.
void sendReport(ReportContext *report_ctx) {
  for (ReportRecordNode *current_record_node = report_ctx->report_head; 
       current_record_node != NULL; 
       current_record_node = current_record_node->next) {
    
    // Inferred arguments for transmit_all based on original stack manipulation and variable usage
    int iVar2 = transmit_all(report_ctx->total_record_length, (int)current_record_node->data_ptr, 1, 0x11184);
    
    if (iVar2 != 0) {
      _terminate(); // Error occurred, terminate execution
    }
  }
}

// Function: splitReport
// Splits a singly linked list of ReportRecordNodes into two halves using slow/fast pointers.
void splitReport(ReportRecordNode **head_ptr, ReportRecordNode **left_half_ptr, ReportRecordNode **right_half_ptr) {
  if ((head_ptr == NULL) || (*head_ptr == NULL) || ((*head_ptr)->next == NULL)) {
    *left_half_ptr = *head_ptr;
    *right_half_ptr = NULL;
  } else {
    ReportRecordNode *slow_ptr = *head_ptr;
    ReportRecordNode *fast_ptr = (*head_ptr)->next; // fast_ptr starts one node ahead
    
    while (fast_ptr != NULL) {
      fast_ptr = fast_ptr->next;
      if (fast_ptr != NULL) {
        slow_ptr = slow_ptr->next;
        fast_ptr = fast_ptr->next;
      }
    }
    // slow_ptr is now at the end of the first half
    *left_half_ptr = *head_ptr;
    *right_half_ptr = slow_ptr->next; // The second half starts after slow_ptr
    slow_ptr->next = NULL; // Terminate the first half
  }
}

// Function: mergeReport
// Merges two sorted ReportRecordNode lists into a single sorted list.
// The comparison is based on a specific segment defined by sort_key_spec.
ReportRecordNode * mergeReport(ReportRecordNode *left, ReportRecordNode *right, ReportSegmentNode *sort_key_spec) {
  if (left == NULL) return right;
  if (right == NULL) return left;

  ReportRecordNode *result_head;
  
  // Compare using the segment specified by sort_key_spec
  int cmp_result = strncmp(left->data_ptr + sort_key_spec->start_offset,
                           right->data_ptr + sort_key_spec->start_offset,
                           sort_key_spec->length);
  
  if (cmp_result < 1) { // left is less than or equal to right
    result_head = left;
    result_head->next = mergeReport(left->next, right, sort_key_spec);
  } else { // right is less than left
    result_head = right;
    result_head->next = mergeReport(left, right->next, sort_key_spec);
  }
  
  return result_head;
}

// Function: sortReport
// Sorts a ReportRecordNode list using merge sort.
// head_ptr is a pointer to the head pointer of the list to be sorted.
// key_node specifies the segment definition to sort by.
void sortReport(ReportRecordNode **head_ptr, ReportSegmentNode *key_node) {
  ReportRecordNode *head = *head_ptr;
  ReportRecordNode *left_half = NULL;
  ReportRecordNode *right_half = NULL;
  
  if ((head == NULL) || (head->next == NULL)) { // Base case: 0 or 1 element list
    return;
  }
  
  splitReport(&head, &left_half, &right_half);
  
  sortReport(&left_half, key_node);
  sortReport(&right_half, key_node);
  
  *head_ptr = mergeReport(left_half, right_half, key_node);
}

// Function: filterReport
// Filters records from a list based on a segment match with a filter string.
// report_ctx provides the segment definitions and the full report list.
// filtered_list_head_ptr is a pointer to store the new filtered list.
// filter_string is the string to match against.
void filterReport(ReportContext *report_ctx, ReportRecordNode **filtered_list_head_ptr, char *filter_string) {
  *filtered_list_head_ptr = NULL; // Initialize filtered list as empty
  
  for (ReportRecordNode *current_record = report_ctx->report_head; 
       current_record != NULL; 
       current_record = current_record->next) {
    
    // Iterate through segment specifications (new_nodes_head list)
    for (ReportSegmentNode *current_segment_spec = report_ctx->new_nodes_head; 
         current_segment_spec != NULL; 
         current_segment_spec = current_segment_spec->next) {
      
      // Compare the segment of the current record with the filter string's corresponding segment
      int cmp_result = strncmp(current_record->data_ptr + current_segment_spec->start_offset,
                               filter_string + current_segment_spec->start_offset,
                               current_segment_spec->length);
      
      if (cmp_result == 0) { // Match found
        ReportRecordNode *new_filtered_node = (ReportRecordNode *)malloc(sizeof(ReportRecordNode));
        if (new_filtered_node == NULL) { /* Handle malloc error */ return; }
        new_filtered_node->next = *filtered_list_head_ptr;
        new_filtered_node->data_ptr = current_record->data_ptr; // Shallow copy of data_ptr
        *filtered_list_head_ptr = new_filtered_node;
        break; // Matched, move to next record
      }
    }
  }
}

// Function: newRecord
// Reads a new record from input, processes it, and adds it to the report list or performs an action.
uint32_t_alias newRecord(ReportContext *report_ctx) {
  ReportRecordNode *new_record_node;
  char *record_data_buffer;
  char *temp_read_buffer;
  uint16_t_alias current_data_offset = 0;
  int recv_flags = 0x11473; // Original iVar3 (flags for recv)
  
  new_record_node = (ReportRecordNode *)malloc(sizeof(ReportRecordNode));
  if (new_record_node == NULL) { /* Handle malloc error */ return 0; }
  new_record_node->next = NULL;

  // Allocate buffer for the entire record data
  record_data_buffer = (char *)malloc(report_ctx->total_record_length + 1); // +1 for null terminator
  if (record_data_buffer == NULL) { free(new_record_node); return 0; }
  new_record_node->data_ptr = record_data_buffer;
  
  // Allocate a temporary buffer for reading segments
  temp_read_buffer = (char *)malloc(report_ctx->total_record_length + 1); // Max segment size could be total length
  if (temp_read_buffer == NULL) { free(record_data_buffer); free(new_record_node); return 0; }

  memset(record_data_buffer, 0, report_ctx->total_record_length + 1);
  
  // Iterate through segment specifications to read input for each segment
  for (ReportSegmentNode *current_segment_spec = report_ctx->new_nodes_head; 
       current_segment_spec != NULL; 
       current_segment_spec = current_segment_spec->next) {
    
    memset(temp_read_buffer, 0, report_ctx->total_record_length + 1); // Clear for each segment
    
    recv(0, temp_read_buffer, current_segment_spec->length, recv_flags);
    
    // Copy segment data into the main record buffer
    strcpy(record_data_buffer + current_data_offset, temp_read_buffer);
    current_data_offset += current_segment_spec->length;
  }
  free(temp_read_buffer); // Free temporary buffer
  
  // Process record based on its content
  if (startswith(new_record_node->data_ptr, DAT_00013000) == 0) { // "EXIT"
    free(new_record_node->data_ptr);
    free(new_record_node);
    return 0; // Signal to exit main loop
  }
  
  if (startswith(new_record_node->data_ptr, "REPORT") == 0) {
    sendReport(report_ctx);
    free(new_record_node->data_ptr);
    free(new_record_node);
    return 1;
  }
  
  if (startswith(new_record_node->data_ptr, DAT_0001300b) == 0) { // "SORT"
    // Assuming the sort index is a single digit character at offset 4 in the record data
    // e.g., "SORT1" means sort by 1st segment (index 0).
    int sort_segment_idx = 0;
    if (strlen(new_record_node->data_ptr) >= 5 && 
        new_record_node->data_ptr[4] >= '0' && new_record_node->data_ptr[4] <= '9') {
        sort_segment_idx = new_record_node->data_ptr[4] - '0';
    }
    
    ReportSegmentNode *sort_key_segment = report_ctx->new_nodes_head;
    for (int i = 0; sort_key_segment != NULL && i < sort_segment_idx; i++) {
        sort_key_segment = sort_key_segment->next;
    }
    
    if (sort_key_segment != NULL) {
      sortReport(&report_ctx->report_head, sort_key_segment);
    }
    
    free(new_record_node->data_ptr);
    free(new_record_node);
    return 1;
  }
  
  if (startswith(new_record_node->data_ptr, "FREPORT") == 0) {
    ReportRecordNode *filtered_list_head = NULL;
    filterReport(report_ctx, &filtered_list_head, new_record_node->data_ptr); // Filter string is the current record's data
    
    // Create a temporary ReportContext for sendReport to use the filtered list
    ReportContext temp_filtered_ctx = {
        .report_head = filtered_list_head,
        .new_nodes_head = NULL, // Not used by sendReport
        .total_record_length = report_ctx->total_record_length,
        .unused_field_14 = 0 // Not used by sendReport
    };
    sendReport(&temp_filtered_ctx);
    
    // Free the filtered list nodes (data_ptr are shallow copies, so only free nodes)
    ReportRecordNode *temp_node;
    while(filtered_list_head != NULL) {
        temp_node = filtered_list_head;
        filtered_list_head = filtered_list_head->next;
        free(temp_node);
    }

    free(new_record_node->data_ptr);
    free(new_record_node);
    return 1;
  }
  
  // Default case: Check for "ERROR" in the first segment of the new record
  char *first_segment_data = new_record_node->data_ptr;
  if (report_ctx->new_nodes_head != NULL) {
      first_segment_data += report_ctx->new_nodes_head->start_offset;
  }
  
  if (startswith(first_segment_data, "ERROR") == 0) {
    free(new_record_node->data_ptr);
    free(new_record_node);
    return 1; // Error record, discard but continue processing
  }
  
  // Add new record to the head of the main report list
  new_record_node->next = report_ctx->report_head;
  report_ctx->report_head = new_record_node;
  return 1;
}

// Function: main
// Entry point of the program. Initializes report context, defines segments, and processes records.
uint32_t_alias main(void) {
  ReportContext report_ctx = {0}; // All fields initialized to 0/NULL
  
  newReport(&report_ctx); // Initialize report segments and total record length
  
  // Main loop to process records until newRecord signals to exit
  while (newRecord(&report_ctx) != 0) {
    // Loop continues as long as newRecord returns 1
  }
  
  // Cleanup: Free all allocated ReportRecordNodes and their associated data buffers
  ReportRecordNode *current_record = report_ctx.report_head;
  while (current_record != NULL) {
      ReportRecordNode *temp_record = current_record;
      current_record = current_record->next;
      free(temp_record->data_ptr); // Free the string data buffer
      free(temp_record);          // Free the node itself
  }

  // Cleanup: Free all allocated ReportSegmentNodes
  ReportSegmentNode *current_segment = report_ctx.new_nodes_head;
  while (current_segment != NULL) {
      ReportSegmentNode *temp_segment = current_segment;
      current_segment = current_segment->next;
      free(temp_segment);
  }

  return 0;
}