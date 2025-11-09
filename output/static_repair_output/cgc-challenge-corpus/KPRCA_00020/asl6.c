#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memset, though not explicitly used, good practice
#include <ctype.h>  // For isdigit, isalnum
#include <stddef.h> // For size_t

// Structure definition for an ASN.1 element
typedef struct Element Element;
struct Element {
    unsigned char tag_class;      // bits 7-6 of first octet
    unsigned char tag;            // bits 4-0 of first octet (if < 0x1f)
    unsigned char is_primitive;   // 1 if primitive, 0 if constructed (bit 5 of first octet)
    unsigned char reserved;       // Padding to align data_ptr on 4-byte boundary
    const unsigned char *data_ptr; // Pointer to the raw data value
    unsigned int length;          // Length of the raw data value in bytes
    int depth;                    // Recursion depth in the ASN.1 structure
    unsigned int capacity;        // Capacity of the sub_elements array
    unsigned int num_sub_elements; // Current number of sub_elements
    Element **sub_elements;       // Array of pointers to sub_elements
};

// Global array for universal tag names
const char *utag_names[] = {
    "EOC", "BOOLEAN", "INTEGER", "BIT STRING", "OCTET STRING", "NULL",
    "OBJECT IDENTIFIER", "ObjectDescriptor", "EXTERNAL", "REAL", "ENUMERATED",
    "EMBEDDED PDV", "UTF8String", "RELATIVE OID", NULL, NULL, // 14, 15 are reserved
    "SEQUENCE", "SET", "NumericString", "PrintableString", "T61String",
    "VideotexString", "IA5String", "UTCTime", "GeneralizedTime", "GraphicString",
    "VisibleString", "GeneralString", "UniversalString", "CHARACTER STRING", "BMPString" // 16-30
};
#define MAX_UNIVERSAL_TAG_INDEX 30

// Helper function to check if a memory region is within the buffer bounds
// Returns 0 on success, -1 on error (out of bounds)
int within(const unsigned char *ptr, unsigned int len, const unsigned char *buffer_start, size_t buffer_size) {
    if (ptr < buffer_start) {
        return -1; // Pointer starts before the buffer
    }
    // Check if the region [ptr, ptr + len) is within [buffer_start, buffer_start + buffer_size)
    // Avoid overflow by checking relative offset: (ptr - buffer_start) + len <= buffer_size
    if ((size_t)(ptr - buffer_start) + len > buffer_size) {
        return -1; // Region extends beyond buffer end
    }
    return 0;
}

// Function: parse_tag_class
// Sets elem->tag_class based on the first octet of input.
// Returns 0 on success, -1 on error.
int parse_tag_class(const unsigned char *input, Element *elem) {
    unsigned char tag_class_val = input[0] >> 6;
    if (tag_class_val < 4) {
        elem->tag_class = tag_class_val;
        return 0;
    }
    return -1;
}

// Function: parse_tag
// Sets elem->tag based on the first octet of input.
// Returns 0 on success, -1 on error.
int parse_tag(const unsigned char *input, Element *elem) {
    unsigned char tag_val = input[0] & 0x1f;
    if (tag_val < 0x1f) { // Short form tag
        elem->tag = tag_val;
        return 0;
    }
    // Long form tags (tag_val == 0x1f) are not supported by the original code's `parse_tag` logic.
    // The original code implicitly returns -1 for long form tags.
    return -1;
}

// Function: parse_length
// Parses the length field from input and stores it in elem->length.
// Returns the number of bytes consumed by the length field, or -1 on error.
int parse_length(const unsigned char *input, Element *elem) {
    if ((input[0] & 0x80) != 0) { // Long form length (MSB is set)
        unsigned int num_length_bytes = input[0] & 0x7f;
        if (num_length_bytes == 0) { // Indefinite length (not supported by this parser)
            fprintf(stderr, "ERROR: indefinite length not supported\n");
            return -1;
        }
        if (num_length_bytes > sizeof(unsigned int)) { // Max 4 bytes for unsigned int
            fprintf(stderr, "ERROR: length too large (more than %zu bytes)\n", sizeof(unsigned int));
            return -1;
        }
        unsigned int length_value = 0;
        for (unsigned int i = 0; i < num_length_bytes; ++i) {
            length_value = (length_value << 8) | input[i + 1];
        }
        elem->length = length_value;
        return num_length_bytes + 1; // Total bytes consumed: 1 byte for initial length + num_length_bytes
    } else { // Short form length (MSB is not set)
        elem->length = input[0];
        return 1; // 1 byte consumed
    }
}

// Function: free_element
// Recursively frees an Element and its sub-elements.
void free_element(Element *elem) {
    if (elem == NULL) {
        return;
    }

    if (elem->sub_elements != NULL) {
        for (unsigned int i = 0; i < elem->num_sub_elements; ++i) {
            if (elem->sub_elements[i] != NULL) {
                free_element(elem->sub_elements[i]);
            }
        }
        free(elem->sub_elements);
    }
    free(elem);
}

// Function: append_sub
// Appends a child element to a parent element's sub_elements array.
// Dynamically resizes the array if needed.
// Returns 0 on success, -1 on error.
int append_sub(Element *parent, Element *child) {
    if (parent->num_sub_elements == parent->capacity) {
        unsigned int new_capacity = parent->capacity * 2;
        if (new_capacity == 0) { // Handle initial capacity of 0
            new_capacity = 2; // Start with a small capacity
        }
        // Check for overflow (e.g., if parent->capacity was already MAX_UINT / 2 + 1)
        if (new_capacity / 2 != parent->capacity) {
            return -1; // Capacity overflowed
        }

        Element **new_sub_elements = realloc(parent->sub_elements, new_capacity * sizeof(Element *));
        if (new_sub_elements == NULL) {
            return -1; // Reallocation failed
        }
        parent->sub_elements = new_sub_elements;
        parent->capacity = new_capacity;
    }

    parent->sub_elements[parent->num_sub_elements] = child;
    parent->num_sub_elements++;
    return 0;
}

// Function: _decode
// Recursively decodes an ASN.1 element from the buffer.
// Returns a pointer to the decoded Element on success, or NULL on error.
Element *_decode(const unsigned char *current_ptr, int depth, const unsigned char *buffer_base, size_t buffer_len) {
    // 1. Check if current_ptr is within the overall buffer bounds
    if (within(current_ptr, 0, buffer_base, buffer_len) != 0) {
        fprintf(stderr, "ERROR: bounds exceeded for element start\n");
        return NULL;
    }

    // 2. Allocate memory for the new element
    Element *elem = calloc(1, sizeof(Element)); // Use calloc to zero-initialize
    if (elem == NULL) {
        return NULL; // Memory allocation failed
    }

    // Initialize fields
    elem->depth = depth;
    elem->capacity = 2; // Initial capacity for sub-elements
    elem->sub_elements = calloc(elem->capacity, sizeof(Element *));
    if (elem->sub_elements == NULL) {
        free(elem);
        return NULL; // Memory allocation for sub_elements failed
    }

    // 3. Parse tag class, constructed flag, and tag number
    // `is_primitive` is 1 if constructed bit (0x20) is NOT set, 0 if set.
    elem->is_primitive = ((*current_ptr & 0x20) == 0);

    if (parse_tag_class(current_ptr, elem) != 0) {
        fprintf(stderr, "ERROR: unknown class\n");
        free_element(elem);
        return NULL;
    }

    if (parse_tag(current_ptr, elem) != 0) {
        fprintf(stderr, "ERROR: unknown tag (long form tags not supported)\n");
        free_element(elem);
        return NULL;
    }

    // Special checks for constructed/primitive types for Universal class
    if (elem->tag_class == 0) {
        // Universal primitive types 0x10 (SEQUENCE) and 0x11 (SET) are invalid.
        // Universal constructed types 0x01 (BOOLEAN) and 0x05 (NULL) are invalid.
        if ((elem->is_primitive && (elem->tag == 0x10 || elem->tag == 0x11)) || // Primitive SEQUENCE/SET
            (!elem->is_primitive && (elem->tag == 0x01 || elem->tag == 0x05))) { // Constructed BOOLEAN/NULL
            fprintf(stderr, "ERROR: bad constructed/primitive type for universal tag 0x%02X\n", elem->tag);
            free_element(elem);
            return NULL;
        }
    }

    // 4. Parse length
    // current_ptr + 1 is the start of the length field
    int len_bytes_consumed = parse_length(current_ptr + 1, elem);
    if (len_bytes_consumed < 0) {
        // Error already printed by parse_length
        free_element(elem);
        return NULL;
    }

    // 5. Calculate data_ptr and check data bounds
    elem->data_ptr = current_ptr + 1 + len_bytes_consumed;

    // Check if the data region [data_ptr, data_ptr + length) is within the overall buffer
    if (within(elem->data_ptr, elem->length, buffer_base, buffer_len) != 0) {
        fprintf(stderr, "ERROR: data bounds exceeded for element data\n");
        free_element(elem);
        return NULL;
    }

    // 6. Specific length checks for some primitive types
    if (elem->is_primitive) {
        if (elem->tag_class == 0) { // Universal primitive types
            if (elem->tag == 0x01 && elem->length != 1) { // BOOLEAN must have length 1
                fprintf(stderr, "ERROR: invalid length for BOOLEAN (expected 1, got %u)\n", elem->length);
                free_element(elem);
                return NULL;
            }
            if (elem->tag == 0x05 && elem->length != 0) { // NULL must have length 0
                fprintf(stderr, "ERROR: invalid length for NULL (expected 0, got %u)\n", elem->length);
                free_element(elem);
                return NULL;
            }
        }
        return elem; // Primitive types don't have sub-elements
    }

    // 7. For constructed types, recursively decode sub-elements
    const unsigned char *sub_element_ptr = elem->data_ptr;
    const unsigned char *elem_data_end = elem->data_ptr + elem->length;

    while (sub_element_ptr < elem_data_end) {
        Element *sub_elem = _decode(sub_element_ptr, depth + 1, buffer_base, buffer_len);
        if (sub_elem == NULL) {
            // Error during sub-element decoding, free current element and return NULL
            free_element(elem);
            return NULL;
        }
        if (append_sub(elem, sub_elem) != 0) {
            // Error appending sub-element, free current element and return NULL
            fprintf(stderr, "ERROR: failed to append sub-element\n");
            free_element(elem);
            return NULL;
        }
        // Update pointer for next sub-element
        sub_element_ptr = sub_elem->data_ptr + sub_elem->length;

        // Check if the next sub_element_ptr is within the parent's declared data length
        if (sub_element_ptr > elem_data_end) {
            fprintf(stderr, "ERROR: Sub-element extends beyond parent's declared length\n");
            free_element(elem);
            return NULL;
        }
    }
    return elem; // Successfully parsed all sub-elements
}

// Function: decode (main entry point for decoding)
// Decodes an ASN.1 structure from the given buffer.
// Returns a pointer to the root Element on success, or NULL on error.
Element *decode(const unsigned char *buffer, size_t buffer_len) {
    return _decode(buffer, 0, buffer, buffer_len);
}

// Function: print_indent
// Prints indentation spaces based on the given depth.
void print_indent(unsigned int depth) {
    for (unsigned int i = 0; i < depth; ++i) {
        fprintf(stdout, "  ");
    }
}

// Function: print_time
// Prints a formatted time string from an Element's data.
// is_utc_time: 1 for UTCTime (YYMMDDHHMMSS), 0 for GeneralizedTime (YYYYMMDDHHMMSS).
void print_time(const Element *elem, int is_utc_time) {
    const unsigned char *time_str = elem->data_ptr;
    unsigned int expected_len;

    if (is_utc_time == 0) { // GeneralizedTime: YYYYMMDDHHMMSS
        expected_len = 14;
    } else { // UTCTime: YYMMDDHHMMSS
        expected_len = 12;
    }

    if (elem->length < expected_len) {
        fprintf(stdout, "INVALID TIME");
        return;
    }

    // Validate characters are digits
    for (unsigned int i = 0; i < expected_len; ++i) {
        if (!isdigit(time_str[i])) {
            fprintf(stdout, "INVALID TIME");
            return;
        }
    }

    if (is_utc_time == 0) { // GeneralizedTime
        fprintf(stdout, "%c%c%c%c/%c%c/%c%c",
                time_str[0], time_str[1], time_str[2], time_str[3], // YYYY
                time_str[4], time_str[5], // MM
                time_str[6], time_str[7]); // DD
        fprintf(stdout, " %c%c:%c%c:%c%c GMT",
                time_str[8], time_str[9],   // HH
                time_str[10], time_str[11], // MM
                time_str[12], time_str[13]);// SS
    } else { // UTCTime
        fprintf(stdout, "%c%c/%c%c/",
                time_str[2], time_str[3], // MM
                time_str[4], time_str[5]); // DD
        if (time_str[0] < '6') { // Year 20YY (e.g., 50-99 -> 19YY; 00-49 -> 20YY)
            fprintf(stdout, "20");
        } else {
            fprintf(stdout, "19");
        }
        fprintf(stdout, "%c%c", time_str[0], time_str[1]); // YY
        fprintf(stdout, " %c%c:%c%c:%c%c GMT",
                time_str[6], time_str[7],   // HH
                time_str[8], time_str[9],   // MM
                time_str[10], time_str[11]);// SS
    }
}

// Function: print_hex
// Prints an Element's data in hexadecimal format.
void print_hex(const Element *elem) {
    const unsigned char *data = elem->data_ptr;
    unsigned int len = elem->length;

    if (len < 17) { // Print on one line if short
        for (unsigned int i = 0; i < len; ++i) {
            fprintf(stdout, "%02X ", data[i]);
        }
    } else { // Print on multiple lines with indentation if long
        fprintf(stdout, "\n");
        print_indent(elem->depth + 1);
        for (unsigned int i = 0; i < len; ++i) {
            fprintf(stdout, "%02X", data[i]);
            if ((i & 0x1f) == 0x1f && (i + 1) < len) { // Every 32 bytes (0-indexed, so after 31st byte), if not the very last byte
                fprintf(stdout, "\n");
                print_indent(elem->depth + 1);
            } else if ((i + 1) < len) { // Not the last byte, print space
                fprintf(stdout, " ");
            }
        }
    }
}

// Function: read_octet_int
// Reads a variable-length integer encoded in octets (MSB indicates continuation).
// Stores the result in *value.
// Returns the number of bytes consumed, or -1 on error.
int read_octet_int(const unsigned char *data, unsigned int max_len, unsigned int *value) {
    *value = 0;
    for (unsigned int i = 0; i < sizeof(unsigned int) && i < max_len; ++i) {
        *value <<= 7;
        *value |= (data[i] & 0x7f);
        if (!((data[i] & 0x80) != 0)) { // If MSB is 0, it's the last byte
            return i + 1; // Number of bytes read
        }
    }
    return -1; // Error: value too long for unsigned int or max_len exceeded without finding end
}

// Function: print_oid
// Prints an Object Identifier (OID) from an Element's data.
void print_oid(const Element *elem) {
    const unsigned char *data = elem->data_ptr;
    unsigned int len = elem->length;
    unsigned int bytes_read = 0;
    unsigned int current_oid_value;
    int bytes_consumed;

    // First octet group (special encoding for first two arcs)
    bytes_consumed = read_octet_int(data + bytes_read, len - bytes_read, &current_oid_value);
    if (bytes_consumed < 0 || bytes_read + bytes_consumed > len) {
        fprintf(stdout, "INVALID OID");
        return;
    }
    bytes_read += bytes_consumed;

    if (current_oid_value < 40) { // 0.* or 1.*
        fprintf(stdout, " %u.%u", current_oid_value / 40, current_oid_value % 40);
    } else { // 2.*
        fprintf(stdout, " 2.%u", current_oid_value - 80); // 80 (0x50) is (2*40 + 0)
    }

    // Subsequent octet groups
    while (bytes_read < len) {
        bytes_consumed = read_octet_int(data + bytes_read, len - bytes_read, &current_oid_value);
        if (bytes_consumed < 0 || bytes_read + bytes_consumed > len) {
            fprintf(stdout, "INVALID OID");
            return;
        }
        bytes_read += bytes_consumed;
        fprintf(stdout, ".%u", current_oid_value);
    }
}

// Function: print_string
// Prints an Element's data as a string, escaping non-alphanumeric characters.
void print_string(const Element *elem) {
    const unsigned char *data = elem->data_ptr;
    unsigned int len = elem->length;

    for (unsigned int i = 0; i < len; ++i) {
        if (isalnum(data[i])) {
            fprintf(stdout, "%c", data[i]);
        } else {
            fprintf(stdout, "\\%02X", data[i]);
        }
    }
}

// Function: print_tag
// Prints the tag class and tag number of an Element.
void print_tag(const Element *elem) {
    unsigned char tag_class = elem->tag_class;
    unsigned char tag = elem->tag;

    if (tag_class == 3) { // Private
        fprintf(stdout, "PRIVATE_%u ", tag);
        return;
    }
    if (tag_class == 2) { // Context-specific
        fprintf(stdout, "[%u] ", tag);
        return;
    }
    if (tag_class == 1) { // Application
        fprintf(stdout, "APPLICATION_%u ", tag);
        return;
    }
    if (tag_class == 0) { // Universal
        if (tag > MAX_UNIVERSAL_TAG_INDEX || utag_names[tag] == NULL) {
            fprintf(stdout, "UNIVERSAL_%u ", tag);
            return;
        }
        fprintf(stdout, "UNIVERSAL %s ", utag_names[tag]);
        return;
    }
    fprintf(stdout, "UNKNOWN_CLASS_%u_TAG_%u ", tag_class, tag); // Fallback for unknown class
    return;
}

// Function: print_primitive
// Prints the value of a primitive ASN.1 element based on its tag.
void print_primitive(const Element *elem) {
    if (elem->tag_class != 0) { // Only Universal class tags have predefined primitive printing
        fprintf(stdout, "UNPRINTABLE");
        return;
    }

    switch (elem->tag) {
        case 0x01: // BOOLEAN
            fprintf(stdout, "%s", (elem->data_ptr[0] == 0) ? "False" : "True");
            break;
        case 0x02: // INTEGER
        case 0x03: // BIT STRING
        case 0x04: // OCTET STRING
            print_hex(elem);
            break;
        case 0x05: // NULL (length must be 0)
            // Nothing extra to print for NULL, its presence is enough.
            break;
        case 0x06: // OBJECT IDENTIFIER
            print_oid(elem);
            break;
        case 0x0C: // UTF8String
        case 0x12: // NumericString
        case 0x13: // PrintableString
        case 0x14: // T61String
        case 0x15: // VideotexString
        case 0x16: // IA5String
        case 0x1A: // VisibleString / ISO646String
            print_string(elem);
            break;
        case 0x17: // UTCTime
            print_time(elem, 1);
            break;
        case 0x18: // GeneralizedTime
            print_time(elem, 0);
            break;
        default:
            fprintf(stdout, "UNPRINTABLE");
            break;
    }
}

// Function: pprint (pretty print)
// Recursively prints the ASN.1 structure.
void pprint(const Element *elem) {
    if (elem == NULL) {
        return;
    }

    print_indent(elem->depth);
    print_tag(elem);
    fprintf(stdout, "\n"); // Always print newline after tag info

    if (elem->is_primitive) { // If it's a primitive type
        print_indent(elem->depth + 1); // Indent for primitive value
        print_primitive(elem);
        fprintf(stdout, "\n"); // Newline after primitive value
    } else { // If it's a constructed type
        for (unsigned int i = 0; i < elem->num_sub_elements; ++i) {
            pprint(elem->sub_elements[i]);
        }
    }

    if (elem->depth == 0) { // Only free the root element when done printing
        free_element((Element *)elem); // Cast away const for free_element
    }
}