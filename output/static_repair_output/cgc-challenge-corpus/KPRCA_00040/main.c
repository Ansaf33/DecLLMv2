#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h> // For uintptr_t

// --- Type Definitions and Global Constants ---

// Using 'uint' for unsigned int as per original code's convention
typedef unsigned int uint;

// Assuming fdprintf is fprintf to stderr for debug messages
#define fdprintf(fd, fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

// External declarations for system calls
// These functions are specific to the CGC (Cyber Grand Challenge) environment.
// For standard Linux compilation, they would need to be replaced by system calls like write/read.
// Assuming they are provided for compilation.
// Dummy implementations for compilation if not linked externally:
#ifndef TRANSMIT_RECEIVE_DEFINED
#define TRANSMIT_RECEIVE_DEFINED
int transmit(unsigned int fd, const void *buf, size_t count, size_t *tx_bytes) {
    *tx_bytes = fwrite(buf, 1, count, (fd == 1) ? stdout : stderr);
    return 0; // 0 for success
}
int receive(unsigned int fd, void *buf, size_t count, size_t *rx_bytes) {
    *rx_bytes = fread(buf, 1, count, (fd == 0) ? stdin : stderr);
    return 0; // 0 for success
}
#endif // TRANSMIT_RECEIVE_DEFINED

// Node structure for a doubly linked list
// The original code implies 4-byte pointers (32-bit system) based on `+4` offsets.
// Using `void *` for `data` allows storing different types.
typedef struct AnyListNode {
    struct AnyListNode *prev; // Offset 0
    struct AnyListNode *next; // Offset 4
    void *data;               // Offset 8 (points to actual data)
} AnyListNode; // Total size 12 bytes (on a 32-bit system)

// Writer structure for managing a buffer
typedef struct Writer {
    uint pos;       // Current write position
    uint capacity;  // Total buffer capacity
    char *buffer;   // Pointer to the buffer
} Writer; // Total size 12 bytes (on a 32-bit system)

// LZ element structure (for literals or backpointers)
// Original code implies 16 bytes (0x10) for this structure.
typedef struct LzElem {
    int type;   // Offset 0: 1 for backpointer, 2 for literal
    uint offset; // Offset 4: Backpointer offset
    char value; // Offset 8: Literal character
    char padding[3]; // Offset 9-11: Padding to align 'length' to 4 bytes
    uint length; // Offset 12: Backpointer length or literal length (1)
} LzElem; // Total size 16 bytes

// Suffix structure for suffix array
typedef struct Suffix {
    uint index; // Offset 0: Start index of the suffix in the original text
    char *str;  // Offset 4: Pointer to the start of the suffix string in the original text
} Suffix; // Total size 8 bytes (on a 32-bit system)

// SuffixList structure for managing an array of suffixes
typedef struct SuffixList {
    uint capacity; // Offset 0: Maximum number of suffixes
    uint count;    // Offset 4: Current number of suffixes
    Suffix **suffixes; // Offset 8: Dynamic array of Suffix pointers
} SuffixList; // Total size 12 bytes (on a 32-bit system)

// Global constants
const uint end_marker = 0; // Marker for end of compressed data
const char UNK_SEQUENCE[] = "SEQ\0"; // Example 4-byte sequence for read_until_sequence
const char DAT_0001509e[] = "('%c',1)"; // Format string for printing literal LZ elements

// --- Function Implementations ---

// Function: mcalloc
void *mcalloc(size_t size) {
    void *ptr = calloc(1, size);
    if (ptr == NULL) {
        fdprintf(2, "DEBUG %s:%d:\tBad alloc\n\n", __FILE__, __LINE__);
        exit(1);
    }
    return ptr;
}

// Function: any_list_add
// Inserts new_node between prev_node and next_node
void any_list_add(AnyListNode *new_node, AnyListNode *prev_node, AnyListNode *next_node) {
    if (next_node != NULL) {
        next_node->prev = new_node;
    }
    if (prev_node != NULL) {
        prev_node->next = new_node;
    }
    new_node->next = next_node;
    new_node->prev = prev_node;
}

// Function: any_list_remv
// Removes a node from the list and frees its memory, including its data
void any_list_remv(AnyListNode *node) {
    if (node != NULL) {
        // Relink previous and next nodes if they exist
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }
        // Free data if it exists
        if (node->data != NULL) {
            free(node->data);
        }
        free(node);
    }
}

// Function: any_list_empty
// Checks if a list (represented by its head node) is empty
bool any_list_empty(AnyListNode *head) {
    return head == head->next;
}

// Function: any_list_length
// Calculates the number of elements in a list (excluding the head node)
int any_list_length(AnyListNode *head) {
    int count = 0;
    AnyListNode *current = head->next;
    while (current != head) {
        count++;
        current = current->next;
    }
    return count;
}

// Function: any_list_alloc
// Allocates a new list node and initializes it as a self-referencing (empty) list head
AnyListNode *any_list_alloc(void *data) {
    AnyListNode *node = (AnyListNode *)mcalloc(sizeof(AnyListNode));
    node->next = node; // Circular self-reference for an empty list
    node->prev = node; // Circular self-reference for an empty list
    node->data = data; // Store data in the head node (or NULL)
    return node;
}

// Function: any_list_shallow_copy
// Creates a new list and shallow copies the data pointers from the source list
AnyListNode *any_list_shallow_copy(AnyListNode *src_head) {
    if (src_head == NULL) {
        return NULL;
    }
    AnyListNode *new_head = any_list_alloc(NULL); // Allocate new head for the copy
    AnyListNode *current_src = src_head->next; // Start from the first actual element of source
    while (current_src != src_head) { // Iterate through source list elements
        AnyListNode *new_node = (AnyListNode *)mcalloc(sizeof(AnyListNode));
        new_node->data = current_src->data; // Shallow copy: data pointer is copied, not the data itself
        any_list_add(new_node, new_head->prev, new_head); // Add new_node to the end of the new list
        current_src = current_src->next;
    }
    return new_head;
}

// Function: send_n_bytes
// Sends a specified number of bytes from a buffer using the transmit function
uint send_n_bytes(uint fd, uint count, void *buf) {
    if (count == 0 || buf == NULL) {
        return 0xffffffff; // Error or nothing to send
    }
    uint total_sent = 0;
    size_t bytes_sent_this_call = 0;
    while (total_sent < count) {
        if (transmit(fd, (char *)buf + total_sent, count - total_sent, &bytes_sent_this_call) != 0) {
            return 0xffffffff; // Transmit error
        }
        if (bytes_sent_this_call == 0) {
            return total_sent; // Partial send (no more bytes could be sent)
        }
        total_sent += bytes_sent_this_call;
    }
    return total_sent;
}

// Function: read_n_bytes
// Reads a specified number of bytes into a buffer using the receive function
uint read_n_bytes(uint fd, uint count, void *buf) {
    if (count == 0 || buf == NULL) {
        return 0xffffffff; // Error or nothing to read
    }
    uint total_read = 0;
    size_t bytes_read_this_call = 0;
    while (total_read < count) {
        if (receive(fd, (char *)buf + total_read, count - total_read, &bytes_read_this_call) != 0) {
            return 0xffffffff; // Receive error
        }
        if (bytes_read_this_call == 0) {
            return total_read; // Partial read (EOF or no more bytes available)
        }
        total_read += bytes_read_this_call;
    }
    return total_read;
}

// Function: read_until_sequence
// Reads bytes from a file descriptor until a specific sequence is found
void *read_until_sequence(uint fd, const void *sequence, uint sequence_len) {
    size_t current_size = 0;
    size_t allocated_size = 0x80; // Initial buffer size
    char *buffer = (char *)mcalloc(allocated_size);

    while (1) {
        // Resize buffer if needed, leaving space for potential null terminator
        if (current_size >= allocated_size - 1) {
            allocated_size += 0x80;
            char *new_buffer = (char *)realloc(buffer, allocated_size);
            if (new_buffer == NULL) {
                fdprintf(2, "DEBUG %s:%d:\tBad alloc\n", __FILE__, __LINE__);
                exit(1);
            }
            buffer = new_buffer;
        }

        size_t bytes_read_this_call = 0;
        if (receive(fd, buffer + current_size, 1, &bytes_read_this_call) != 0) { // Read 1 byte at a time
            free(buffer);
            return NULL; // Read error
        }
        if (bytes_read_this_call == 0) { // EOF
            free(buffer);
            return NULL;
        }
        current_size += bytes_read_this_call;

        // Check if the sequence is at the end of the current buffer
        if (current_size >= sequence_len) {
            if (memcmp(buffer + current_size - sequence_len, sequence, sequence_len) == 0) {
                return buffer; // Sequence found
            }
        }
    }
}

// Function: readline
// Reads a line from a file descriptor until a newline character or EOF
void *readline(uint fd) {
    size_t current_size = 0;
    size_t allocated_size = 0x80; // Initial buffer size
    char *buffer = (char *)mcalloc(allocated_size);

    while (1) {
        // Resize buffer if needed, leaving space for null terminator
        if (current_size >= allocated_size - 1) {
            allocated_size += 0x80;
            char *new_buffer = (char *)realloc(buffer, allocated_size);
            if (new_buffer == NULL) {
                fdprintf(2, "DEBUG %s:%d:\tBad alloc\n", __FILE__, __LINE__);
                exit(1);
            }
            buffer = new_buffer;
        }

        size_t bytes_read_this_call = 0;
        if (receive(fd, buffer + current_size, 1, &bytes_read_this_call) != 0) { // Read 1 byte
            free(buffer);
            return NULL; // Read error
        }
        if (bytes_read_this_call == 0) { // EOF
            free(buffer);
            return NULL;
        }

        if (buffer[current_size] == '\n') {
            buffer[current_size] = '\0'; // Null-terminate at newline
            return buffer;
        }
        current_size++;
    }
}

// Function: writer_new
// Creates and initializes a new Writer object
Writer *writer_new(char *buffer, uint capacity) {
    Writer *writer = (Writer *)mcalloc(sizeof(Writer));
    writer->pos = 0;
    writer->capacity = capacity;
    writer->buffer = buffer;
    return writer;
}

// Function: writer_write
// Writes data to the writer's buffer
uint writer_write(Writer *writer, const void *src_buffer, uint length) {
    if (writer->pos + length > writer->capacity) {
        fdprintf(2, "DEBUG %s:%d:\tWriter buffer overflow\n", __FILE__, __LINE__);
        exit(1); // Exit on buffer overflow
    }
    memcpy(writer->buffer + writer->pos, src_buffer, length);
    writer->pos += length;
    return 0; // Return 0 on success
}

// Function: print_lzelem
// Prints an LZ element (literal or backpointer) to the specified file descriptor
void print_lzelem(uint fd, LzElem *elem) {
    if (elem->type == 1) { // Backpointer
        fdprintf(fd, "(%d,%d)", elem->offset, elem->length);
    } else if (elem->type == 2) { // Literal
        fdprintf(fd, DAT_0001509e, elem->value);
    }
}

// Function: swap_u32
// Performs a 32-bit byte swap (endianness conversion)
uint swap_u32(uint value) {
    return ((value >> 24) & 0x000000ff) |
           ((value >> 8)  & 0x0000ff00) |
           ((value << 8)  & 0x00ff0000) |
           ((value << 24) & 0xff000000);
}

// Function: pack_lzelem_bp
// Packs LZ backpointer data (offset, length) into a single 32-bit unsigned integer
uint pack_lzelem_bp(LzElem *elem) {
    if (0x7fff < elem->length) { // Check if length fits in 15 bits
        fdprintf(2, "DEBUG %s:%d:\tCan\'t pack LZ element: length too large\n", __FILE__, __LINE__);
        exit(1);
    }
    // Format: [1-bit type (1 for BP), 15-bit length, 16-bit offset]
    // The 0x80000000U flag indicates it's a backpointer
    uint packed_value = ((elem->length & 0x7fff) << 16) | 0x80000000U | (elem->offset & 0xffff);
    return swap_u32(packed_value); // Byte swap for network/storage order
}

// Function: unpack_lzelem_bp
// Unpacks a 32-bit unsigned integer into LZ backpointer data
LzElem *unpack_lzelem_bp(uint packed_value) {
    uint unpacked_val = swap_u32(packed_value); // Byte swap back
    LzElem *elem = (LzElem *)mcalloc(sizeof(LzElem));
    elem->type = 1; // It's a backpointer
    elem->length = (unpacked_val >> 16) & 0x7fff;
    elem->offset = unpacked_val & 0xffff;
    return elem;
}

// Function: lz_backpointer_new
// Creates a new LZ element representing a backpointer
LzElem *lz_backpointer_new(uint offset, uint length) {
    LzElem *elem = (LzElem *)mcalloc(sizeof(LzElem));
    elem->type = 1; // Backpointer type
    elem->offset = offset;
    elem->length = length;
    return elem;
}

// Function: lz_literal_new
// Creates a new LZ element representing a literal character
LzElem *lz_literal_new(char value) {
    LzElem *elem = (LzElem *)mcalloc(sizeof(LzElem));
    elem->type = 2; // Literal type
    elem->value = value;
    elem->length = 1; // Literal length is always 1
    return elem;
}

// Function: alnumspc_filter
// Filters a string, keeping only alphanumeric characters and spaces
char *alnumspc_filter(const char *src_str) {
    if (src_str == NULL) {
        return NULL;
    }
    size_t len = strlen(src_str);
    // Always allocate at least 1 byte for null terminator, even for empty input
    char *filtered_str = (char *)mcalloc(len + 1);
    size_t write_idx = 0;
    for (size_t read_idx = 0; read_idx < len; ++read_idx) {
        if (isalnum((int)src_str[read_idx]) || src_str[read_idx] == ' ') {
            filtered_str[write_idx++] = src_str[read_idx];
        }
    }
    filtered_str[write_idx] = '\0'; // Null-terminate the filtered string
    return filtered_str; // Returns an empty string "" if nothing matched or input was empty.
}

// Function: make_suffix
// Creates a new Suffix object
Suffix *make_suffix(uint index, char *str) {
    Suffix *suffix = (Suffix *)mcalloc(sizeof(Suffix));
    suffix->index = index;
    suffix->str = str;
    return suffix;
}

// Function: make_suffix_list
// Creates and initializes a new SuffixList
SuffixList *make_suffix_list(uint capacity) {
    SuffixList *list = (SuffixList *)mcalloc(sizeof(SuffixList));
    list->capacity = capacity;
    list->count = 0;
    list->suffixes = (Suffix **)mcalloc(capacity * sizeof(Suffix *));
    return list;
}

// Function: append_suffix_list
// Appends a suffix to the SuffixList
uint append_suffix_list(SuffixList *list, Suffix *suffix) {
    if (list->count == list->capacity) {
        return 0xffffffff; // Error: list is full
    }
    list->suffixes[list->count++] = suffix;
    return 0; // Success
}

// Function: get_suffix
// Retrieves a suffix from the SuffixList at a given index
Suffix *get_suffix(SuffixList *list, uint index) {
    if (list->count <= index) { // Use count, not capacity, for valid elements
        fdprintf(2, "DEBUG %s:%d:\tBad suffix list access (index %u, count %u)\n", __FILE__, __LINE__, index, list->count);
        exit(1);
    }
    return list->suffixes[index];
}

// Function: cmp_suffix
// Compares two suffixes based on their string content
int cmp_suffix(Suffix *suffix1, Suffix *suffix2) {
    return strcmp(suffix1->str, suffix2->str);
}

// Function: free_suffix_list
// Frees the memory associated with a SuffixList (but not the Suffix objects or their strings)
void free_suffix_list(SuffixList *list) {
    if (list == NULL) return;
    if (list->suffixes != NULL) {
        free(list->suffixes);
    }
    free(list);
}

// Function: merge
// Merges two sorted SuffixLists into a new sorted SuffixList
SuffixList *merge(SuffixList *list1, SuffixList *list2) {
    uint len1 = list1->count;
    uint len2 = list2->count;
    SuffixList *merged_list = make_suffix_list(len1 + len2);

    uint idx1 = 0;
    uint idx2 = 0;

    while (idx1 < len1 && idx2 < len2) {
        if (cmp_suffix(get_suffix(list1, idx1), get_suffix(list2, idx2)) <= 0) { // Less than or equal
            append_suffix_list(merged_list, get_suffix(list1, idx1));
            idx1++;
        } else {
            append_suffix_list(merged_list, get_suffix(list2, idx2));
            idx2++;
        }
    }

    // Append remaining elements from list1
    while (idx1 < len1) {
        append_suffix_list(merged_list, get_suffix(list1, idx1));
        idx1++;
    }

    // Append remaining elements from list2
    while (idx2 < len2) {
        append_suffix_list(merged_list, get_suffix(list2, idx2));
        idx2++;
    }
    return merged_list;
}

// Function: merge_sort
// Sorts a SuffixList using the merge sort algorithm
SuffixList *merge_sort(SuffixList *list) {
    uint len = list->count;

    if (len <= 1) { // Base case: 0 or 1 element list is already sorted
        return list;
    }

    // Split the list into two halves
    uint mid_point = len / 2;
    SuffixList *left_list = make_suffix_list(mid_point);
    SuffixList *right_list = make_suffix_list(len - mid_point);

    for (uint i = 0; i < mid_point; ++i) {
        append_suffix_list(left_list, get_suffix(list, i));
    }
    for (uint i = mid_point; i < len; ++i) {
        append_suffix_list(right_list, get_suffix(list, i));
    }

    // Recursively sort the halves
    SuffixList *sorted_left = merge_sort(left_list);
    SuffixList *sorted_right = merge_sort(right_list);

    // Merge the sorted halves
    SuffixList *result_list = merge(sorted_left, sorted_right);

    // Free temporary list containers if they are not the result of a recursive call
    if (sorted_left != left_list) {
        free_suffix_list(left_list);
    }
    if (sorted_right != right_list) {
        free_suffix_list(right_list);
    }
    // Free the input list container, as a new sorted one is returned.
    free_suffix_list(list);

    return result_list;
}

// Function: build_suffix_array
// Constructs a suffix array for a given text
uint *build_suffix_array(char *text) {
    size_t len = strlen(text);
    uint *suffix_array = (uint *)mcalloc(len * sizeof(uint)); // Array to store suffix starting indices

    SuffixList *list = make_suffix_list(len);
    for (uint i = 0; i < len; ++i) {
        Suffix *suffix = make_suffix(i, text + i); // Create suffix for each position
        append_suffix_list(list, suffix);
    }

    SuffixList *sorted_list = merge_sort(list); // Sort the list of suffixes

    // Populate the suffix array with the sorted indices
    for (uint i = 0; i < len; ++i) {
        Suffix *s = get_suffix(sorted_list, i);
        suffix_array[i] = s->index;
        free(s); // Free the Suffix object, but not s->str (which points into the original text)
    }
    free_suffix_list(sorted_list); // Free the sorted list container

    return suffix_array;
}

// Function: search
// Performs a binary search for a pattern in the text using the suffix array
uint search(const char *pattern, size_t pattern_len, const uint *suffix_array, const char *text, uint text_len, uint offset_limit) {
    if (pattern == NULL || pattern_len == 0 || suffix_array == NULL || text == NULL || text_len == 0) {
        return 0xffffffff; // Invalid input
    }

    uint low = 0;
    uint high = text_len - 1; // Suffix array has `text_len` elements

    while (low <= high) {
        uint mid = low + (high - low) / 2; // Calculate mid-point to prevent overflow
        uint text_offset = suffix_array[mid];

        // Determine effective length for comparison to avoid reading past text boundaries
        size_t compare_len = pattern_len;
        if (text_offset + compare_len > text_len) {
            compare_len = text_len - text_offset;
        }

        int cmp_result = strncmp(pattern, text + text_offset, compare_len);

        if (cmp_result == 0) {
            // If pattern is longer than available text at this offset, it's not a full match
            if (compare_len < pattern_len) {
                low = mid + 1; // Pattern is "greater", search higher
                continue;
            }

            // Found a match. Check if it violates the offset_limit.
            // Original logic: if `current_text_pos <= text_offset + pattern_len`, it's an invalid match.
            // This means the match must *end before* `offset_limit`.
            if (offset_limit != 0xffffffff && (text_offset + pattern_len > offset_limit)) {
                high = mid - 1; // Match found, but invalid. Search earlier in suffix array.
                continue;
            }
            return text_offset; // Valid match found, return its starting offset
        }
        if (cmp_result < 0) {
            high = mid - 1; // Pattern is smaller, search lower half
        } else {
            low = mid + 1; // Pattern is larger, search upper half
        }
    }
    return 0xffffffff; // Pattern not found
}

// Function: prefix_len
// Calculates the length of the common prefix between two strings, up to a text end boundary
uint prefix_len(const char *str1, const char *str2, const char *text_end) {
    uint len = 0;
    // Calculate maximum safe comparison length for each string
    size_t max_compare_len1 = (size_t)(text_end - str1);
    size_t max_compare_len2 = (size_t)(text_end - str2);
    size_t max_compare_len = (max_compare_len1 < max_compare_len2) ? max_compare_len1 : max_compare_len2;

    // Optimized loop for 8-byte chunks
    while (len < (max_compare_len & ~7)) {
        if (str1[0] != str2[0]) break;
        if (str1[1] != str2[1]) break;
        if (str1[2] != str2[2]) break;
        if (str1[3] != str2[3]) break;
        if (str1[4] != str2[4]) break;
        if (str1[5] != str2[5]) break;
        if (str1[6] != str2[6]) break;
        if (str1[7] != str2[7]) break;
        str1 += 8;
        str2 += 8;
        len += 8;
    }

    // Process remaining bytes
    while (len < max_compare_len && *str1 != '\0' && *str2 != '\0' && *str1 == *str2) {
        len++;
        str1++;
        str2++;
    }
    return len;
}

// Function: compress
// Compresses a given text using LZ77 algorithm with suffix array for efficient searching
int compress(char *text, uint text_len, char *output_buffer, uint output_capacity, const uint *suffix_array) {
    if (text == NULL || text_len == 0 || output_buffer == NULL || output_capacity == 0 || suffix_array == NULL) {
        return -1; // Indicate error
    }

    AnyListNode *lz_elements_list_head = any_list_alloc(NULL); // Head of the list to store LZ elements
    Writer *writer = writer_new(output_buffer, output_capacity);

    // Handle the first character as a literal
    if (text_len > 0) {
        LzElem *first_literal = lz_literal_new(text[0]);
        AnyListNode *first_node = any_list_alloc(first_literal);
        any_list_add(first_node, lz_elements_list_head->prev, lz_elements_list_head);
    }

    uint current_text_pos;
    for (current_text_pos = 1; current_text_pos < text_len; ) {
        uint best_match_len = 0;
        uint best_match_relative_offset = 0;

        // Search for the longest match (backpointer) ending before current_text_pos
        for (uint search_len = 1; current_text_pos + search_len <= text_len; ++search_len) {
            // Search for pattern `text + current_text_pos` of length `search_len`
            // in `text` using `suffix_array`, ensuring matches end before `current_text_pos`.
            uint actual_match_start_offset = search(text + current_text_pos, search_len, suffix_array, text, text_len, current_text_pos);

            if (actual_match_start_offset != 0xffffffff) { // Match found
                // Calculate actual prefix length from this match, up to the end of text
                uint current_match_len = prefix_len(text + actual_match_start_offset, text + current_text_pos, text + text_len);
                if (current_match_len > best_match_len) {
                    best_match_relative_offset = current_text_pos - actual_match_start_offset;
                    best_match_len = current_match_len;
                }
            }
        }

        LzElem *new_lz_elem;
        // If no good backpointer (match length less than 8, or no match found)
        if (best_match_len < 8) { // Minimum match length for a backpointer to be effective
            new_lz_elem = lz_literal_new(text[current_text_pos]);
        } else {
            new_lz_elem = lz_backpointer_new(best_match_relative_offset, best_match_len);
        }

        AnyListNode *new_node = any_list_alloc(new_lz_elem);
        any_list_add(new_node, lz_elements_list_head->prev, lz_elements_list_head);
        current_text_pos += new_lz_elem->length; // Advance position by the length of the compressed element
    }

    // Write LZ elements to the output buffer
    AnyListNode *current_list_node = lz_elements_list_head->next;
    while (current_list_node != lz_elements_list_head) {
        LzElem *elem = (LzElem *)current_list_node->data;

        if (elem->type == 2) { // Literal
            if (writer->pos + 1 > writer->capacity) {
                fdprintf(2, "DEBUG %s:%d:\tOutput buffer overflow during literal write\n", __FILE__, __LINE__);
                exit(1);
            }
            writer_write(writer, &elem->value, 1);
        } else { // Backpointer
            if (writer->pos + sizeof(uint) > writer->capacity) {
                fdprintf(2, "DEBUG %s:%d:\tOutput buffer overflow during backpointer write\n", __FILE__, __LINE__);
                exit(1);
            }
            uint packed_bp = pack_lzelem_bp(elem);
            writer_write(writer, &packed_bp, sizeof(uint));
        }
        print_lzelem(1, elem); // Assuming 1 is stdout for debug prints

        AnyListNode *next_list_node = current_list_node->next;
        any_list_remv(current_list_node); // Remove node and free its data (LzElem)
        current_list_node = next_list_node;
    }
    writer_write(writer, &end_marker, sizeof(end_marker)); // Write the end marker

    int final_size = writer->pos;
    free(writer);
    any_list_remv(lz_elements_list_head); // Free the head node and its data (which is NULL)
    return final_size;
}

// Function: decompress
// Decompresses data from a compressed buffer using LZ77 decompression
uint decompress(const char *compressed_buffer, char *output_buffer, uint output_capacity) {
    if (compressed_buffer == NULL || output_buffer == NULL || output_capacity == 0) {
        return 0xffffffff; // Invalid input
    }

    Writer *writer = writer_new(output_buffer, output_capacity);
    uint read_pos = 0;

    while (1) {
        // Check for end marker
        if (memcmp(compressed_buffer + read_pos, &end_marker, sizeof(end_marker)) == 0) {
            uint final_size = writer->pos;
            free(writer);
            return final_size;
        }

        // Determine if it's a backpointer or literal by checking the MSB of the first byte
        // (after byte-swapping, the 0x80000000 flag from pack_lzelem_bp will be in the first byte)
        if (*(const signed char *)(compressed_buffer + read_pos) < 0) { // It's a backpointer
            uint packed_bp_val = *(const uint *)(compressed_buffer + read_pos);
            LzElem *bp_elem = unpack_lzelem_bp(packed_bp_val);

            // Validate backpointer offset and ensure it doesn't cause output buffer overflow
            if (writer->pos < bp_elem->offset || writer->pos + bp_elem->length > writer->capacity) {
                fdprintf(2, "DEBUG %s:%d:\tBad back pointer or output overflow (offset %d, length %d)\n", __FILE__, __LINE__, bp_elem->offset, bp_elem->length);
                free(bp_elem);
                exit(1);
            }

            // Copy data from previously decompressed output
            // Source: `writer->buffer + (writer->pos - bp_elem->offset)`
            // Destination: `writer->buffer + writer->pos`
            writer_write(writer, writer->buffer + (writer->pos - bp_elem->offset), bp_elem->length);
            read_pos += sizeof(uint); // Advance 4 bytes for backpointer
            free(bp_elem); // Free the temporary LzElem object
        } else { // It's a literal (1 byte)
            if (writer->pos + 1 > writer->capacity) {
                fdprintf(2, "DEBUG %s:%d:\tOutput buffer overflow for literal\n", __FILE__, __LINE__);
                exit(1);
            }
            writer_write(writer, compressed_buffer + read_pos, 1);
            read_pos += 1; // Advance 1 byte for literal
        }
    }
}

// Function: main
int main(void) {
    // Using static buffers for output to avoid large stack allocations.
    // Original decompiler output had a bug here with a small stack buffer.
    static char decompress_output_buffer[0x10000]; // 64KB
    static char compress_output_buffer[0x8004];   // ~32KB

    char *input_line = NULL;
    char *second_input_line = NULL;
    char *filtered_text = NULL;
    uint *suffix_array = NULL;
    char *compressed_data = NULL; // Buffer for data read during decompress command

    // Command strings
    const char *COMMAND_COMPRESS = "compress";
    const char *COMMAND_DECOMPRESS = "decompress";
    const char *COMMAND_QUIT = "quit";

    while (1) {
        input_line = (char *)readline(0); // Read command from stdin (fd 0)
        if (input_line == NULL) {
            // EOF or read error, exit
            return 0xffffffff;
        }

        if (strcmp(input_line, COMMAND_COMPRESS) == 0) {
            free(input_line); // Free the "compress" command line

            second_input_line = (char *)readline(0); // Read text to compress
            if (second_input_line == NULL) {
                return 0xffffffff;
            }

            filtered_text = alnumspc_filter(second_input_line);
            free(second_input_line);

            if (filtered_text == NULL || strlen(filtered_text) == 0) {
                free(filtered_text); // Free the empty string allocated by alnumspc_filter
                fdprintf(2, "DEBUG %s:%d:\tNo valid text to compress\n", __FILE__, __LINE__);
                return 0xffffffff;
            }

            size_t text_len = strlen(filtered_text);
            if (text_len > 0x10000) { // Max allowed text length for processing
                free(filtered_text);
                fdprintf(2, "DEBUG %s:%d:\tInput text too long (%zu > %u)\n", __FILE__, __LINE__, text_len, 0x10000);
                return 0xffffffff;
            }

            suffix_array = build_suffix_array(filtered_text);
            if (suffix_array == NULL) {
                free(filtered_text);
                fdprintf(2, "DEBUG %s:%d:\tFailed to build suffix array\n", __FILE__, __LINE__);
                return 0xffffffff;
            }

            uint compressed_size = compress(filtered_text, (uint)text_len, compress_output_buffer, sizeof(compress_output_buffer), suffix_array);

            send_n_bytes(1, compressed_size, compress_output_buffer); // Send compressed data to stdout (fd 1)

            free(filtered_text);
            free(suffix_array); // Free the array of indices

        } else if (strcmp(input_line, COMMAND_DECOMPRESS) == 0) {
            free(input_line); // Free the "decompress" command line

            // Read compressed data until the UNK_SEQUENCE is found
            compressed_data = (char *)read_until_sequence(0, UNK_SEQUENCE, sizeof(UNK_SEQUENCE) - 1); // -1 for null terminator
            if (compressed_data == NULL) {
                fdprintf(2, "DEBUG %s:%d:\tFailed to read compressed data or sequence not found\n", __FILE__, __LINE__);
                return 0xffffffff;
            }

            // Clear the output buffer before decompressing
            memset(decompress_output_buffer, 0, sizeof(decompress_output_buffer));

            uint decompressed_size = decompress(compressed_data, decompress_output_buffer, sizeof(decompress_output_buffer));

            send_n_bytes(1, decompressed_size, decompress_output_buffer); // Send decompressed data to stdout

            free(compressed_data);

        } else if (strcmp(input_line, COMMAND_QUIT) == 0) {
            free(input_line);
            return 0; // Exit successfully
        } else {
            // Unknown command, free the line and loop again
            free(input_line);
        }
    }
}