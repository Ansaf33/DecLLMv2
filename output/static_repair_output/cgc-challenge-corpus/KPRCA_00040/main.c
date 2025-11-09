#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>   // For size_t
#include <stdbool.h>  // For bool
#include <string.h>   // For strlen, strcmp, memcmp, memset, memmove
#include <ctype.h>    // For isalnum
#include <unistd.h>   // For STDIN_FILENO, STDOUT_FILENO, ssize_t

// Forward declarations for custom I/O functions
// These functions are assumed to behave like read/write but with an additional bytes_transferred/read argument.
int transmit(int fd, const void *buf, size_t count, int *bytes_written);
int receive(int fd, void *buf, size_t count, int *bytes_read);

// Global debug print function (replaces fdprintf(2, ...))
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG %s:%d:\t" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// Custom types for clarity, matching inferred sizes from original code
typedef unsigned int uint;

// Node structure for any_list_... functions
// Inferred from mcalloc(0xc) and pointer arithmetic (e.g., param_1[1], *param_1)
typedef struct Node {
    struct Node *prev; // Offset 0
    struct Node *next; // Offset 4
    void *data;        // Offset 8
} Node; // Total size 12 bytes (0xc)

// LZ Element structure for print_lzelem, pack_lzelem_bp, unpack_lzelem_bp, lz_backpointer_new, lz_literal_new
// Inferred from mcalloc(0x10) and specific field accesses
typedef struct LZ_Elem {
    int type;      // Offset 0 (1 for backpointer, 2 for literal)
    uint offset;   // Offset 4 (backpointer offset)
    char literal;  // Offset 8 (literal character)
    uint length;   // Offset 12 (backpointer length or literal length=1)
} LZ_Elem; // Total size 16 bytes (0x10)

// Writer structure
// Inferred from mcalloc(0xc) and field assignments
typedef struct Writer {
    size_t current_pos; // Offset 0
    size_t capacity;    // Offset 4
    void *buffer;       // Offset 8
} Writer; // Total size 12 bytes (0xc)

// Suffix structure for suffix array
// Inferred from mcalloc(8) and field assignments
typedef struct Suffix {
    uint index;      // Offset 0
    char *suffix_ptr; // Offset 4
} Suffix; // Total size 8 bytes

// SuffixList structure
// Inferred from mcalloc(0xc) and field assignments
typedef struct SuffixList {
    size_t capacity; // Offset 0
    size_t count;    // Offset 4
    Suffix **suffixes; // Offset 8
} SuffixList; // Total size 12 bytes (0xc)

// Global end marker for decompression/read_until_sequence
// The value 0x04000000 is inferred from `UNK_00105b90 + iVar4` and memcmp usage
const uint end_marker = 0x04000000;

// Function: mcalloc
void * mcalloc(size_t size) {
  void *ptr = calloc(1, size);
  if (ptr == NULL) {
    DEBUG_PRINT("Bad alloc");
    exit(1);
  }
  return ptr;
}

// Function: any_list_add
// This function is re-interpreted as adding a new node to the end of a circular doubly linked list.
// The original decompiled code's `param_2` as `int` and `param_3` as `int**` were likely decompiler artifacts,
// as a literal interpretation would lead to incorrect list manipulation.
void any_list_add(Node *newNode, Node *head) {
  if (newNode == NULL || head == NULL) return;

  // Insert newNode before head and after head->prev
  newNode->prev = head->prev;
  newNode->next = head;
  head->prev->next = newNode;
  head->prev = newNode;
}

// Function: any_list_remv
void any_list_remv(Node *nodeToRemove) {
  if (nodeToRemove != NULL) {
    // Relink prev and next nodes to bypass nodeToRemove
    if (nodeToRemove->next != NULL) {
        nodeToRemove->next->prev = nodeToRemove->prev;
    }
    if (nodeToRemove->prev != NULL) {
        nodeToRemove->prev->next = nodeToRemove->next;
    }
    // Free the data if it exists (param_1[2] access in original)
    if (nodeToRemove->data != NULL) {
      free(nodeToRemove->data);
    }
    free(nodeToRemove);
  }
}

// Function: any_list_empty
bool any_list_empty(Node *head) {
  return head == head->next;
}

// Function: any_list_length
// Fixed loop logic to correctly count elements in a circular list.
int any_list_length(Node *head) {
  int count = 0;
  Node *current = head->next; // Start from the first actual element
  while (current != head) {
    count++;
    current = current->next;
  }
  return count;
}

// Function: any_list_alloc
Node * any_list_alloc(void *data) {
  Node *newNode = mcalloc(sizeof(Node));
  newNode->next = newNode; // Circular link for an empty list
  newNode->prev = newNode; // Circular link for an empty list
  newNode->data = data;
  return newNode;
}

// Function: any_list_shallow_copy
// Fixed list iteration and `any_list_add` call to match the re-interpreted list functions.
Node * any_list_shallow_copy(Node *sourceListHead) {
  if (sourceListHead == NULL) {
    return NULL;
  }
  
  Node *newListHead = any_list_alloc(NULL); // Allocate new head for the copy
  Node *current = sourceListHead->next; // Start from the first actual element
  
  // Iterate through source list, adding shallow copies of elements to the new list
  while (current != sourceListHead) {
    any_list_add(any_list_alloc(current->data), newListHead); // Add a new node with shallow copy of data
    current = current->next;
  }
  return newListHead;
}

// Function: send_n_bytes
uint send_n_bytes(int fd, uint count, char *buffer) {
  if ((count == 0) || (buffer == NULL)) {
    return 0xffffffff; // Error or nothing to send
  }
  
  size_t bytes_transferred = 0;
  int chunk_size = 0;
  
  while (bytes_transferred < count) {
    int ret = transmit(fd, buffer + bytes_transferred, count - bytes_transferred, &chunk_size);
    if (ret != 0) { // Transmit error
      return 0xffffffff;
    }
    if (chunk_size == 0) { // No bytes sent, prevent infinite loop
      return bytes_transferred;
    }
    bytes_transferred += chunk_size;
  }
  return bytes_transferred;
}

// Function: read_n_bytes
uint read_n_bytes(int fd, uint count, char *buffer) {
  if ((count == 0) || (buffer == NULL)) {
    return 0xffffffff; // Error or nothing to read
  }
  
  size_t bytes_read_total = 0;
  int chunk_size = 0;
  
  while (bytes_read_total < count) {
    int ret = receive(fd, buffer + bytes_read_total, count - bytes_read_total, &chunk_size);
    if (ret != 0) { // Receive error
      return 0xffffffff;
    }
    if (chunk_size == 0) { // No bytes read, prevent infinite loop
      return bytes_read_total;
    }
    bytes_read_total += chunk_size;
  }
  return bytes_read_total;
}

// Function: read_until_sequence
// Fixed sequence search logic to correctly find sequence at the end of the buffer as it grows.
void * read_until_sequence(int fd, const void *sequence, uint seq_len) {
  void *buffer = mcalloc(0x80);
  size_t allocated_size = 0x80;
  size_t current_size = 0;
  
  while (true) {
    if (current_size == allocated_size) {
      allocated_size += 0x80;
      buffer = realloc(buffer, allocated_size);
      if (buffer == NULL) {
        DEBUG_PRINT("Bad alloc");
        exit(1);
      }
    }
    
    int bytes_read = read_n_bytes(fd, 1, (char*)buffer + current_size);
    if (bytes_read == 0) { // End of input or error
      free(buffer);
      return NULL;
    }
    current_size += bytes_read;

    // Check if the sequence is at the end of the buffer
    if (current_size >= seq_len && memcmp((char*)buffer + current_size - seq_len, sequence, seq_len) == 0) {
      return buffer;
    }
  }
}

// Function: readline
// Fixed goto and memchr logic. Simplified to find and null-terminate at the first newline.
void * readline(int fd) {
  void *buffer = mcalloc(0x80);
  size_t allocated_size = 0x80;
  size_t current_size = 0;
  
  while (true) {
    if (current_size == allocated_size) {
      allocated_size += 0x80;
      buffer = realloc(buffer, allocated_size);
      if (buffer == NULL) {
        DEBUG_PRINT("Bad alloc");
        exit(1);
      }
    }
    
    int bytes_read = read_n_bytes(fd, 1, (char*)buffer + current_size);
    if (bytes_read == 0) { // End of input or error
      free(buffer);
      return NULL;
    }
    
    if (((char*)buffer)[current_size] == '\n') {
      ((char*)buffer)[current_size] = '\0'; // Null-terminate at the newline
      return buffer;
    }
    current_size += bytes_read;
  }
}

// Function: writer_new
Writer * writer_new(void *buffer, size_t capacity) {
  Writer *writer = mcalloc(sizeof(Writer));
  writer->current_pos = 0;
  writer->capacity = capacity;
  writer->buffer = buffer;
  return writer;
}

// Function: writer_write
uint writer_write(Writer *writer, const void *src, size_t len) {
  if (writer == NULL || src == NULL || writer->buffer == NULL) {
    return 0xffffffff; // Error
  }
  if (writer->current_pos + len > writer->capacity) {
    return 0xffffffff; // Buffer overflow
  }

  memcpy((char*)writer->buffer + writer->current_pos, src, len);
  writer->current_pos += len;
  return 0; // Success
}

// Function: print_lzelem
void print_lzelem(int fd, LZ_Elem *elem) {
  if (elem == NULL) return;
  if (elem->type == 1) { // Backpointer
    fprintf(stderr,"(%u,%u)", elem->offset, elem->length);
  } else if (elem->type == 2) { // Literal
    fprintf(stderr,"%c", elem->literal); // Assuming DAT_0001509e was "%c"
  }
  return;
}

// Function: swap_u32
uint swap_u32(uint value) {
  // Standard byte swap for 32-bit integer
  return ((value >> 24) & 0x000000FF) |
         ((value >> 8) & 0x0000FF00) |
         ((value << 8) & 0x00FF0000) |
         ((value << 24) & 0xFF000000);
}

// Function: pack_lzelem_bp
uint pack_lzelem_bp(LZ_Elem *elem) {
  if (elem->length > 0x7fff) {
    DEBUG_PRINT("Can\'t pack length %u, exceeds 0x7fff", elem->length);
    exit(1);
  }
  // Format: 1 bit (1 for backpointer) | 15 bits length | 16 bits offset
  uint packed_val = (0x80000000 | ((elem->length & 0x7fff) << 16) | (elem->offset & 0xffff));
  return swap_u32(packed_val);
}

// Function: unpack_lzelem_bp
LZ_Elem * unpack_lzelem_bp(uint packed_val) {
  uint swapped_val = swap_u32(packed_val);
  LZ_Elem *elem = mcalloc(sizeof(LZ_Elem));
  elem->type = 1; // It's a backpointer
  elem->length = (swapped_val >> 16) & 0x7fff;
  elem->offset = swapped_val & 0xffff;
  return elem;
}

// Function: lz_backpointer_new
LZ_Elem * lz_backpointer_new(uint offset, uint length) {
  LZ_Elem *elem = mcalloc(sizeof(LZ_Elem));
  elem->type = 1; // Backpointer type
  elem->offset = offset;
  elem->length = length;
  return elem;
}

// Function: lz_literal_new
LZ_Elem * lz_literal_new(char literal_char) {
  LZ_Elem *elem = mcalloc(sizeof(LZ_Elem));
  elem->type = 2; // Literal type
  elem->literal = literal_char;
  elem->length = 1; // Literal length is always 1
  return elem;
}

// Function: alnumspc_filter
char * alnumspc_filter(const char *input) {
  if (input == NULL) {
    return NULL;
  }
  size_t input_len = strlen(input);
  if (input_len == 0) {
    return NULL;
  }
  
  char *filtered_str = mcalloc(input_len + 1); // +1 for null terminator
  int write_idx = 0;
  for (size_t i = 0; i < input_len; i++) {
    if (isalnum((int)input[i]) || (input[i] == ' ')) {
      filtered_str[write_idx++] = input[i];
    }
  }
  filtered_str[write_idx] = '\0'; // Null-terminate the filtered string
  
  if (write_idx == 0) { // If nothing was filtered in
      free(filtered_str);
      return NULL;
  }
  return filtered_str;
}

// Function: make_suffix
Suffix * make_suffix(uint index, char *suffix_ptr) {
  Suffix *suffix = mcalloc(sizeof(Suffix));
  suffix->index = index;
  suffix->suffix_ptr = suffix_ptr;
  return suffix;
}

// Function: make_suffix_list
SuffixList * make_suffix_list(size_t capacity) {
  SuffixList *list = mcalloc(sizeof(SuffixList));
  list->capacity = capacity;
  list->count = 0;
  list->suffixes = mcalloc(capacity * sizeof(Suffix*)); // Array of Suffix pointers
  return list;
}

// Function: append_suffix_list
uint append_suffix_list(SuffixList *list, Suffix *suffix) {
  if (list->count == list->capacity) {
    return 0xffffffff; // List full
  }
  list->suffixes[list->count] = suffix;
  list->count++;
  return 0;
}

// Function: get_suffix
Suffix * get_suffix(SuffixList *list, uint index) {
  if (list->count <= index) {
    DEBUG_PRINT("Bad suffix list access at index %u, count %zu", index, list->count);
    exit(1);
  }
  return list->suffixes[index];
}

// Function: cmp_suffix
// Fixed return type from void to int
int cmp_suffix(Suffix *s1, Suffix *s2) {
  return strcmp(s1->suffix_ptr, s2->suffix_ptr);
}

// Function: merge
SuffixList * merge(SuffixList *list1, SuffixList *list2) {
  size_t count1 = list1->count;
  size_t count2 = list2->count;
  SuffixList *merged_list = make_suffix_list(count1 + count2);
  
  size_t i = 0; // index for list2
  size_t j = 0; // index for list1
  
  while (j < count1 && i < count2) {
    int cmp_result = cmp_suffix(get_suffix(list1, j), get_suffix(list2, i));
    if (cmp_result < 1) { // list1 suffix is smaller or equal
      append_suffix_list(merged_list, get_suffix(list1, j));
      j++;
    } else { // list2 suffix is smaller
      append_suffix_list(merged_list, get_suffix(list2, i));
      i++;
    }
  }
  
  // Append remaining elements from list1
  while (j < count1) {
    append_suffix_list(merged_list, get_suffix(list1, j));
    j++;
  }
  
  // Append remaining elements from list2
  while (i < count2) {
    append_suffix_list(merged_list, get_suffix(list2, i));
    i++;
  }
  
  // Free the input lists' internal arrays and structs (but not the Suffixes themselves, as they are shallow copied)
  free(list1->suffixes);
  free(list1);
  free(list2->suffixes);
  free(list2);

  return merged_list;
}

// Function: merge_sort
SuffixList * merge_sort(SuffixList *list) {
  size_t count = list->count;
  
  if (count <= 1) { // Base case: list with 0 or 1 element is already sorted
    return list;
  }
  
  SuffixList *left_half = make_suffix_list(count / 2);
  SuffixList *right_half = make_suffix_list(count - (count / 2));
  
  size_t mid = count / 2;
  for (size_t i = 0; i < mid; i++) {
    append_suffix_list(left_half, get_suffix(list, i));
  }
  for (size_t i = mid; i < count; i++) {
    append_suffix_list(right_half, get_suffix(list, i));
  }
  
  SuffixList *sorted_left = merge_sort(left_half);
  SuffixList *sorted_right = merge_sort(right_half);
  
  // Free the original list's internal array and the list struct itself
  // as it's replaced by the merged result.
  free(list->suffixes);
  free(list);

  return merge(sorted_left, sorted_right);
}

// Function: build_suffix_array
uint * build_suffix_array(char *text) {
  size_t len = strlen(text);
  if (len == 0) return NULL;

  SuffixList *suffix_list = make_suffix_list(len);
  for (uint i = 0; i < len; i++) {
    Suffix *suffix = make_suffix(i, text + i);
    append_suffix_list(suffix_list, suffix);
  }
  
  suffix_list = merge_sort(suffix_list); // suffix_list now contains sorted suffixes

  uint *suffix_array = mcalloc(len * sizeof(uint));
  for (uint i = 0; i < len; i++) {
    Suffix *s = get_suffix(suffix_list, i);
    suffix_array[i] = s->index;
    free(s); // Free individual Suffix structs
  }

  // Free the SuffixList structure itself. Its `suffixes` array was freed by `merge_sort`'s final `merge` call.
  free(suffix_list->suffixes); 
  free(suffix_list);

  return suffix_array;
}

// Function: search
uint search(const char *pattern, size_t pattern_len, uint *suffix_array, const char *text, size_t text_len, uint start_idx_in_text) {
  if (pattern == NULL || pattern_len == 0 || suffix_array == NULL || text == NULL || text_len == 0) {
    return 0xffffffff;
  }
  
  size_t low = 0;
  size_t high = text_len - 1;
  
  while (low <= high) {
    size_t mid = low + (high - low) / 2;
    uint text_offset = suffix_array[mid];
    
    // Ensure we don't read beyond text_len during comparison
    size_t cmp_len = pattern_len;
    if (text_offset + cmp_len > text_len) {
        cmp_len = text_len - text_offset;
    }
    
    int cmp_result = strncmp(pattern, text + text_offset, cmp_len);
    
    if (cmp_result == 0) {
      // If the match starts before `start_idx_in_text + pattern_len`, it's considered an overlap
      if (start_idx_in_text != (uint)-1 && text_offset < start_idx_in_text + pattern_len) {
          return 0xffffffff; // Invalid match due to overlap
      }
      return text_offset; // Found
    }
    
    if (cmp_result < 0) { // Pattern is lexicographically smaller, search left
      high = mid - 1;
    } else { // Pattern is lexicographically larger, search right
      low = mid + 1;
    }
  }
  return 0xffffffff; // Not found
}

// Function: prefix_len
// Fixed calculation of `max_len_to_compare` using pointer subtraction.
uint prefix_len(const char *s1, const char *s2, const char *text_end) {
  size_t len = 0;
  size_t max_len_to_compare = 0;

  // Calculate maximum length to compare without going past text_end
  size_t len1 = text_end - s1;
  size_t len2 = text_end - s2;
  max_len_to_compare = (len1 < len2) ? len1 : len2;

  // Unrolled loop for faster comparison (8 bytes at a time)
  for (; len < (max_len_to_compare & ~7); len += 8) {
    if (s1[len] != s2[len]) break;
    if (s1[len+1] != s2[len+1]) break;
    if (s1[len+2] != s2[len+2]) break;
    if (s1[len+3] != s2[len+3]) break;
    if (s1[len+4] != s2[len+4]) break;
    if (s1[len+5] != s2[len+5]) break;
    if (s1[len+6] != s2[len+6]) break;
    if (s1[len+7] != s2[len+7]) break;
  }

  // Compare remaining bytes
  while (len < max_len_to_compare && s1[len] != '\0' && s2[len] != '\0' && s1[len] == s2[len]) {
    len++;
  }
  return len;
}

// Function: compress
// Fixed any_list_add call and iteration logic.
int compress(char *text, uint text_len, void *output_buffer, size_t output_capacity, uint *suffix_array) {
  Node *lz_list_head = any_list_alloc(NULL); // Head of the LZ element list
  Writer *writer = writer_new(output_buffer, output_capacity);
  
  // Add first literal (if text is not empty)
  if (text_len > 0) {
    LZ_Elem *first_literal = lz_literal_new(text[0]);
    any_list_add(any_list_alloc(first_literal), lz_list_head);
  }

  for (size_t current_text_idx = 1; current_text_idx < text_len; ) {
    size_t best_match_len = 0;
    int best_match_offset_in_text = -1;
    
    // Search for longest match starting from current_text_idx
    // `search_len` iterates from 1 upwards, trying to find longer and longer matches.
    for (size_t search_len = 1; current_text_idx + search_len <= text_len; search_len++) {
      uint match_offset_in_text = search(text + current_text_idx, search_len, suffix_array, text, text_len, current_text_idx);
      
      if (match_offset_in_text != (uint)-1) { // Found a match
        // Calculate actual prefix length from the matched offset
        size_t current_prefix_len = prefix_len(text + match_offset_in_text, text + current_text_idx, text + text_len);
        
        if (best_match_len < current_prefix_len) {
          best_match_offset_in_text = match_offset_in_text;
          best_match_len = current_prefix_len;
        }
      } else {
        // No match found for `search_len`, so no longer matches starting with this prefix will exist.
        break; // Optimize: break early
      }
    }
    
    LZ_Elem *lz_elem;
    size_t advance_len;

    // Use a backpointer if a good match (length >= 8) is found
    if (best_match_offset_in_text >= 0 && best_match_len >= 8) {
      lz_elem = lz_backpointer_new(current_text_idx - best_match_offset_in_text, best_match_len);
      advance_len = best_match_len;
    } else { // No good backpointer, emit a literal
      lz_elem = lz_literal_new(text[current_text_idx]);
      advance_len = 1;
    }
    any_list_add(any_list_alloc(lz_elem), lz_list_head);
    current_text_idx += advance_len;
  }
  
  // Write LZ elements from the list to the output buffer
  Node *current_lz_node = lz_list_head->next;
  while (current_lz_node != lz_list_head) {
    LZ_Elem *elem = (LZ_Elem *)current_lz_node->data;
    
    if (elem->type == 2) { // Literal
      if (writer->capacity < writer->current_pos + 1) {
        DEBUG_PRINT("Output buffer capacity %zu, current_pos %zu, needed %zu. Doesn\'t compress enough (literal)", writer->capacity, writer->current_pos, writer->current_pos + 1);
        exit(1);
      }
      writer_write(writer, &elem->literal, 1);
    } else { // Backpointer
      uint packed_val = pack_lzelem_bp(elem);
      if (writer->capacity < writer->current_pos + 4) {
        DEBUG_PRINT("Output buffer capacity %zu, current_pos %zu, needed %zu. Doesn\'t compress enough (backpointer)", writer->capacity, writer->current_pos, writer->current_pos + 4);
        exit(1);
      }
      writer_write(writer, &packed_val, 4);
    }
    print_lzelem(STDERR_FILENO, elem); // Print to stderr (fd 2) for debug/trace
    
    Node *temp_node = current_lz_node->next; // Store next node before removing current
    any_list_remv(current_lz_node); // Free current LZ_Elem and its Node
    current_lz_node = temp_node;
  }
  
  writer_write(writer, &end_marker, 4); // Write end marker
  
  int final_output_size = writer->current_pos;
  free(writer);
  any_list_remv(lz_list_head); // Free the head node (its data should be NULL from `any_list_alloc(NULL)`)
  
  return final_output_size;
}

// Function: decompress
uint decompress(const char *compressed_data, void *output_buffer, size_t output_capacity) {
  Writer *writer = writer_new(output_buffer, output_capacity);
  size_t current_input_pos = 0;
  
  while (true) {
    // Check for end marker
    if (memcmp(&end_marker, compressed_data + current_input_pos, 4) == 0) {
      uint final_output_size = writer->current_pos;
      free(writer);
      return final_output_size;
    }
    
    // Determine if it's a literal (1 byte) or backpointer (4 bytes)
    // Backpointers typically have a high bit set in their first byte.
    // The original code used `*(char *) < '\0'` which checks the sign bit.
    if (((signed char)compressed_data[current_input_pos]) < 0) { // Backpointer (sign bit set)
      uint packed_val;
      memcpy(&packed_val, compressed_data + current_input_pos, 4);
      LZ_Elem *elem = unpack_lzelem_bp(packed_val);
      
      if (writer->current_pos < elem->offset) {
        DEBUG_PRINT("Bad back pointer offset %u, current_pos %zu", elem->offset, writer->current_pos);
        exit(1);
      }
      
      // Calculate source address for copy
      char *src_ptr = (char*)writer->buffer + (writer->current_pos - elem->offset);
      
      // Ensure we don't write past capacity
      if (writer->current_pos + elem->length > writer->capacity) {
          DEBUG_PRINT("Decompression buffer overflow: current_pos %zu + length %u > capacity %zu", writer->current_pos, elem->length, writer->capacity);
          exit(1);
      }

      // Use memmove for potentially overlapping copies (e.g., repeating "abc" from "ab")
      memmove((char*)writer->buffer + writer->current_pos, src_ptr, elem->length);
      writer->current_pos += elem->length;
      free(elem); // Free the unpacked LZ_Elem
      current_input_pos += 4; // Advance 4 bytes for backpointer
    } else { // Literal (sign bit not set)
      if (writer->current_pos + 1 > writer->capacity) {
          DEBUG_PRINT("Decompression buffer overflow: current_pos %zu + 1 > capacity %zu", writer->current_pos, writer->capacity);
          exit(1);
      }
      writer_write(writer, compressed_data + current_input_pos, 1);
      current_input_pos += 1; // Advance 1 byte for literal
    }
  }
}

// Function: main
// Removed decompiler-specific stack setup and pointer arithmetic.
int main(void) {
  // Command strings
  const char *compress_cmd = "/compress";
  const char *decompress_cmd = "/decompress";
  const char *quit_cmd = "/quit";
  
  // Buffers for compression/decompression, sized according to original code's usage
  char decompressed_output_buffer[0x10000]; // 65536 bytes
  char compressed_output_buffer[0x8004];   // 32772 bytes + 4 bytes for end marker

  while (true) {
    char *command_line = readline(STDIN_FILENO);
    if (command_line == NULL) {
      // EOF or error reading command, exit
      return 0xffffffff; 
    }

    if (strcmp(command_line, compress_cmd) == 0) {
      free(command_line); // Free command line input

      char *input_line = readline(STDIN_FILENO);
      if (input_line == NULL) {
        return 0xffffffff;
      }
      
      char *filtered_input = alnumspc_filter(input_line);
      free(input_line); // Free original input line
      
      if (filtered_input == NULL) {
        return 0xffffffff; // No valid characters or empty after filter
      }
      
      size_t filtered_len = strlen(filtered_input);
      if (filtered_len > 0x10000) { // Max input size constraint from original code
        free(filtered_input);
        return 0xffffffff;
      }
      
      uint *suffix_array = build_suffix_array(filtered_input);
      if (suffix_array == NULL) {
        free(filtered_input);
        return 0xffffffff;
      }
      
      int compressed_size = compress(filtered_input, filtered_len, compressed_output_buffer, sizeof(compressed_output_buffer), suffix_array);
      
      send_n_bytes(STDOUT_FILENO, compressed_size, compressed_output_buffer);
      
      free(filtered_input);
      free(suffix_array); // Free the suffix array itself

    } else if (strcmp(command_line, decompress_cmd) == 0) {
      free(command_line); // Free command line input

      // read_until_sequence reads data until the end_marker.
      // The decompress function then processes this buffer, stopping at the same end_marker.
      char *compressed_input_buffer = read_until_sequence(STDIN_FILENO, &end_marker, 4);
      if (compressed_input_buffer == NULL) {
        return 0xffffffff;
      }
      
      uint decompressed_size = decompress(compressed_input_buffer, decompressed_output_buffer, sizeof(decompressed_output_buffer));
      
      send_n_bytes(STDOUT_FILENO, decompressed_size, decompressed_output_buffer);
      
      free(compressed_input_buffer);

    } else if (strcmp(command_line, quit_cmd) == 0) {
      free(command_line); // Free command line input
      return 0; // Exit successfully
    } else {
      // Unknown command, free and continue to next iteration
      free(command_line);
    }
  }
  // Should not reach here
  return 0xffffffff;
}

// Dummy implementations for transmit and receive
// In a real CGC challenge, these would be provided by the system.
int transmit(int fd, const void *buf, size_t count, int *bytes_written) {
    ssize_t res = write(fd, buf, count);
    if (res < 0) {
        *bytes_written = 0;
        return 1; // Error
    }
    *bytes_written = res;
    return 0; // Success
}

int receive(int fd, void *buf, size_t count, int *bytes_read) {
    ssize_t res = read(fd, buf, count);
    if (res < 0) {
        *bytes_read = 0;
        return 1; // Error
    }
    *bytes_read = res;
    return 0; // Success
}