#include <stdio.h>    // For fprintf, stdout, stderr
#include <stdlib.h>   // For malloc, realloc, calloc, free, EXIT_SUCCESS, EXIT_FAILURE
#include <stdint.h>   // For uint8_t, uint32_t
#include <stdbool.h>  // For bool
#include <ctype.h>    // For isdigit, isalnum
#include <string.h>   // For strlen

// Define Element structure based on pointer arithmetic analysis for a 32-bit system
typedef struct Element {
    uint8_t tag_class;      // Offset 0
    uint8_t tag;            // Offset 1
    bool is_primitive;      // Offset 2
    uint8_t _padding1;      // Offset 3 (1 byte padding to align data_ptr to 4-byte boundary)
    uint8_t *data_ptr;      // Offset 4 (4 bytes on 32-bit)
    uint32_t length;        // Offset 8 (4 bytes)
    int depth;              // Offset 12 (4 bytes)
    uint32_t capacity;      // Offset 16 (4 bytes)
    uint32_t count;         // Offset 20 (4 bytes)
    struct Element **sub_elements; // Offset 24 (4 bytes on 32-bit)
} Element; // Total size: 28 bytes (0x1C)

// Constants for ASN.1 Universal Tags
#define ASN1_TAG_BOOLEAN           0x01
#define ASN1_TAG_INTEGER           0x02
#define ASN1_TAG_BIT_STRING        0x03
#define ASN1_TAG_OCTET_STRING      0x04
#define ASN1_TAG_NULL              0x05
#define ASN1_TAG_OBJECT_IDENTIFIER 0x06
#define ASN1_TAG_UTF8_STRING       0x0C
#define ASN1_TAG_NUMERIC_STRING    0x12
#define ASN1_TAG_PRINTABLE_STRING  0x13
#define ASN1_TAG_T61_STRING        0x14
#define ASN1_TAG_VIDEOTEX_STRING   0x15
#define ASN1_TAG_IA5_STRING        0x16
#define ASN1_TAG_VISIBLE_STRING    0x1A
#define ASN1_TAG_UTC_TIME          0x17
#define ASN1_TAG_GENERALIZED_TIME  0x18
#define ASN1_TAG_SEQUENCE          0x10 // 16
#define ASN1_TAG_SET               0x11 // 17

// Global array for universal tag names
const char *utag_names[] = {
    "EOC", "BOOLEAN", "INTEGER", "BIT STRING", "OCTET STRING", "NULL", "OBJECT IDENTIFIER",
    "ObjectDescriptor", "EXTERNAL", "REAL", "ENUMERATED", "EMBEDDED PDV", "UTF8String",
    "RELATIVE-OID", "TIME", "RESERVED", "SEQUENCE", "SET", "NumericString", "PrintableString",
    "T61String", "VideotexString", "IA5String", "UTCTime", "GeneralizedTime", "GraphicString",
    "VisibleString", "GeneralString", "UniversalString", "CHARACTER STRING", "BMPString"
};

// Function prototypes to resolve dependencies
void free_element(Element *elem);
void print_indent(uint32_t depth);
void print_tag(const Element *elem);
void print_hex(const Element *elem);
void print_oid(const Element *elem);
void print_string(const Element *elem);
void print_time(const Element *elem, int is_utc_time);
void print_primitive(const Element *elem);
int parse_tag_class(const uint8_t *data_byte, Element *elem);
int parse_tag(const uint8_t *data_byte, Element *elem);
int parse_length(const uint8_t *data_byte, Element *elem);
int append_sub(Element *parent, Element *child);
int within(const uint8_t *check_ptr, uint32_t check_len, const uint8_t *buffer_start, uint32_t buffer_len);
Element * _decode(const uint8_t *data_ptr, int depth, const uint8_t *buffer_start, uint32_t buffer_len);
void pprint(Element *elem);
void decode(const uint8_t *data, uint32_t data_len);

// Function: parse_tag_class
int parse_tag_class(const uint8_t *data_byte, Element *elem) {
    if ((*data_byte >> 6) < 4) {
        elem->tag_class = *data_byte >> 6;
        return 0; // Success
    }
    return -1; // Error
}

// Function: parse_tag
int parse_tag(const uint8_t *data_byte, Element *elem) {
    if ((*data_byte & 0x1f) < 0x1f) {
        elem->tag = *data_byte & 0x1f;
        return 0; // Success
    }
    return -1; // Error
}

// Function: parse_length
int parse_length(const uint8_t *data_byte, Element *elem) {
    if ((char)*data_byte < '\0') { // Check if MSB is set, indicating multi-byte length
        uint32_t num_length_bytes = (uint32_t)(*data_byte & 0x7f);
        if (num_length_bytes == 0) { // Indefinite length (0x80)
            fprintf(stdout, "ERROR: indefinite length not supported\n");
            return -1; // Indicate error
        } else if (num_length_bytes < 5) { // Max 4 bytes for length value
            uint32_t length_value = 0;
            for (uint32_t i = 0; i < num_length_bytes; i++) {
                length_value = (length_value << 8) | data_byte[i + 1];
            }
            elem->length = length_value;
            return num_length_bytes + 1; // Number of bytes consumed for length field (indicator byte + length bytes)
        } else {
            fprintf(stdout, "ERROR: length too large (more than 4 bytes)\n");
            return -1;
        }
    } else { // Single-byte length
        elem->length = (uint32_t)*data_byte;
        return 1; // Number of bytes consumed for length field (the length byte itself)
    }
}

// Function: free_element
void free_element(Element *elem) {
    if (elem == NULL) {
        return;
    }

    if (elem->sub_elements != NULL) {
        for (uint32_t i = 0; i < elem->count; i++) {
            if (elem->sub_elements[i] != NULL) {
                free_element(elem->sub_elements[i]);
            }
        }
        free(elem->sub_elements);
    }
    free(elem);
}

// Function: append_sub
int append_sub(Element *parent, Element *child) {
    if (parent->count == parent->capacity) {
        uint32_t new_capacity = parent->capacity * 2;
        if (new_capacity / 2 != parent->capacity) { // Overflow check for new_capacity
            return -1; // Error
        }
        Element **new_sub_elements = (Element **)realloc(parent->sub_elements, new_capacity * sizeof(Element *));
        if (new_sub_elements == NULL) {
            return -1; // Error
        }
        parent->sub_elements = new_sub_elements;
        parent->capacity = new_capacity;
    }
    parent->sub_elements[parent->count] = child;
    parent->count++;
    return 0; // Success
}

// Function: within
int within(const uint8_t *check_ptr, uint32_t check_len, const uint8_t *buffer_start, uint32_t buffer_len) {
    const uint8_t *buffer_end = buffer_start + buffer_len;
    // Check if check_ptr is within buffer_start and buffer_end,
    // and if check_ptr + check_len is also within buffer_end.
    if (check_ptr < buffer_start || (check_ptr + check_len) > buffer_end) {
        return -1; // Bounds exceeded
    }
    return 0; // Within bounds
}

// Function: _decode
Element * _decode(const uint8_t *data_ptr, int depth, const uint8_t *buffer_start, uint32_t buffer_len) {
    // Check if at least 1 byte is available for the tag byte
    if (within(data_ptr, 1, buffer_start, buffer_len) < 0) {
        fprintf(stdout, "ERROR: bounds exceeded (initial tag byte check)\n");
        return NULL;
    }

    Element *elem = (Element *)malloc(sizeof(Element));
    if (elem == NULL) {
        return NULL;
    }
    // Initialize fields to safe defaults
    elem->tag_class = 0;
    elem->tag = 0;
    elem->is_primitive = false;
    elem->data_ptr = NULL;
    elem->length = 0;
    elem->depth = 0;
    elem->capacity = 0;
    elem->count = 0;
    elem->sub_elements = NULL;

    elem->capacity = 2; // Initial capacity for sub-elements
    elem->sub_elements = (Element **)calloc(elem->capacity, sizeof(Element *));
    if (elem->sub_elements == NULL) {
        free(elem); // Free the element itself if sub_elements allocation fails
        return NULL;
    }
    elem->count = 0;
    elem->depth = depth;
    elem->is_primitive = !(*data_ptr & 0x20); // Bit 5 (0x20) is constructed/primitive flag

    if (parse_tag_class(data_ptr, elem) < 0) {
        fprintf(stdout, "ERROR: unknown class\n");
        free_element(elem);
        return NULL;
    }

    if (parse_tag(data_ptr, elem) < 0) {
        fprintf(stdout, "ERROR: unknown tag\n");
        free_element(elem);
        return NULL;
    }

    // Check for constructed/primitive type consistency
    if (!elem->is_primitive && elem->tag != ASN1_TAG_SEQUENCE && elem->tag != ASN1_TAG_SET) {
        fprintf(stdout, "ERROR: bad constructed type (tag %u)\n", elem->tag);
        free_element(elem);
        return NULL;
    }
    if (elem->is_primitive && (elem->tag == ASN1_TAG_SEQUENCE || elem->tag == ASN1_TAG_SET)) {
        fprintf(stdout, "ERROR: bad primitive type (tag %u)\n", elem->tag);
        free_element(elem);
        return NULL;
    }

    // Determine how many bytes the length field itself will consume
    uint32_t length_field_bytes_needed = 0;
    if (within(data_ptr + 1, 1, buffer_start, buffer_len) < 0) { // Check if the first length byte exists
        fprintf(stdout, "ERROR: bounds exceeded (first length byte check)\n");
        free_element(elem);
        return NULL;
    }

    uint8_t first_length_byte = data_ptr[1]; // The byte immediately after the tag byte
    if ((char)first_length_byte < '\0') { // Multi-byte length (MSB set)
        uint32_t num_length_bytes_indicator = first_length_byte & 0x7f;
        if (num_length_bytes_indicator == 0) { // Indefinite length (0x80)
            fprintf(stdout, "ERROR: indefinite length not supported\n");
            free_element(elem);
            return NULL;
        }
        if (num_length_bytes_indicator >= 5) { // Length value takes more than 4 bytes
            fprintf(stdout, "ERROR: length too large (more than 4 bytes)\n");
            free_element(elem);
            return NULL;
        }
        length_field_bytes_needed = 1 + num_length_bytes_indicator; // 1 for indicator byte + N for length value bytes
    } else { // Single-byte length
        length_field_bytes_needed = 1; // 1 for the length byte itself
    }

    // Check if there are enough bytes in the buffer for the entire length field
    if (within(data_ptr + 1, length_field_bytes_needed, buffer_start, buffer_len) < 0) {
        fprintf(stdout, "ERROR: bounds exceeded (entire length field)\n");
        free_element(elem);
        return NULL;
    }

    // Now it's safe to call parse_length
    int length_bytes_consumed = parse_length(data_ptr + 1, elem);
    if (length_bytes_consumed < 0) { // parse_length returns -1 for error
        // Error message already printed by parse_length
        free_element(elem);
        return NULL;
    }

    // Check for specific primitive type length constraints
    if (elem->tag == ASN1_TAG_BOOLEAN && elem->length != 1) {
        fprintf(stdout, "ERROR: invalid length for BOOLEAN (expected 1, got %u)\n", elem->length);
        free_element(elem);
        return NULL;
    }
    if (elem->tag == ASN1_TAG_NULL && elem->length != 0) {
        fprintf(stdout, "ERROR: invalid length for NULL (expected 0, got %u)\n", elem->length);
        free_element(elem);
        return NULL;
    }

    elem->data_ptr = (uint8_t *)data_ptr + 1 + length_bytes_consumed;

    // Check if the data portion of the element is within overall buffer bounds
    if (within(elem->data_ptr, elem->length, buffer_start, buffer_len) < 0) {
        fprintf(stdout, "ERROR: bounds exceeded (element data field)\n");
        free_element(elem);
        return NULL;
    }

    if (elem->is_primitive) {
        return elem; // Primitive type, no sub-elements to decode
    }

    // Constructed type, decode sub-elements
    const uint8_t *current_sub_data_ptr = elem->data_ptr;
    const uint8_t *parent_data_end = elem->data_ptr + elem->length;

    while (current_sub_data_ptr < parent_data_end) {
        Element *sub_elem = _decode(current_sub_data_ptr, elem->depth + 1, buffer_start, buffer_len);
        if (sub_elem == NULL) {
            // Error decoding sub-element, return parent (partially decoded).
            // This matches the original logic of returning the partially constructed element.
            return elem;
        }

        if (append_sub(elem, sub_elem) < 0) {
            fprintf(stdout, "ERROR: failed to append sub-element\n");
            free_element(sub_elem); // Free the sub-element that couldn't be appended
            return elem; // Return partially constructed parent
        }

        current_sub_data_ptr = sub_elem->data_ptr + sub_elem->length;

        if (current_sub_data_ptr < buffer_start) { // Should not happen if within is correct for sub_elem
             fprintf(stdout, "ERROR: sub-element end pointer before buffer start\n");
             return elem;
        }
        // If the next sub-element would start at or beyond the parent's data end, stop.
        // This handles both normal termination and sub-element extending beyond parent's declared length.
        if (current_sub_data_ptr > parent_data_end) {
            fprintf(stdout, "ERROR: sub-element extends beyond parent's data boundary\n");
            return elem;
        }
    }
    return elem; // Successfully decoded all sub-elements (or reached end of parent's data)
}

// Function: decode
void decode(const uint8_t *data, uint32_t data_len) {
    // The return value of _decode is ignored here, matching the original function signature.
    // A typical design would return the root Element* or an error code.
    Element *root_element = _decode(data, 0, data, data_len);
    // If root_element is NULL, an error occurred during decoding.
    // Otherwise, print the structure. pprint also handles freeing the element tree.
    if (root_element != NULL) {
        pprint(root_element);
    } else {
        fprintf(stdout, "ERROR: Failed to decode root element.\n");
    }
}

// Function: print_indent
void print_indent(uint32_t depth) {
    for (uint32_t i = 0; i < depth; i++) {
        fprintf(stdout, "  ");
    }
}

// Function: print_time
void print_time(const Element *elem, int is_utc_time) {
    uint32_t expected_len;
    const uint8_t *time_str_ptr = elem->data_ptr;

    if (is_utc_time == 0) { // GeneralizedTime (YYYYMMDDHHMMSS)
        expected_len = 14;
    } else { // UTCTime (YYMMDDHHMMSS)
        expected_len = 12;
    }

    if (elem->length < expected_len) {
        fprintf(stdout, "INVALID TIME");
        return;
    }

    for (uint32_t i = 0; i < expected_len; i++) {
        if (!isdigit(time_str_ptr[i])) {
            fprintf(stdout, "INVALID TIME");
            return;
        }
    }

    if (is_utc_time == 0) { // GeneralizedTime
        // Format: DD/MM/YYYY
        fprintf(stdout, "%c%c/%c%c/%c%c%c%c",
                time_str_ptr[6], time_str_ptr[7],   // DD
                time_str_ptr[4], time_str_ptr[5],   // MM
                time_str_ptr[0], time_str_ptr[1],   // YYYY
                time_str_ptr[2], time_str_ptr[3]);
        time_str_ptr += 2; // Adjust pointer for HH:MM:SS part
    } else { // UTCTime
        // Format: MM/DD/YYYY
        fprintf(stdout, "%c%c/%c%c/",
                time_str_ptr[4], time_str_ptr[5],   // MM
                time_str_ptr[2], time_str_ptr[3]);   // DD

        // Determine century for YY
        if (time_str_ptr[0] < '5') { // Years 00-49 are 20YY, 50-99 are 19YY
            fprintf(stdout, "20");
        } else {
            fprintf(stdout, "19");
        }
        fprintf(stdout, "%c%c", time_str_ptr[0], time_str_ptr[1]); // YY
    }
    // Print HH:MM:SS GMT (common for both after adjustments/initial prints)
    fprintf(stdout, " %c%c:%c%c:%c%c GMT",
            time_str_ptr[6], time_str_ptr[7],   // HH
            time_str_ptr[8], time_str_ptr[9],   // MM
            time_str_ptr[10], time_str_ptr[11]); // SS
}

// Function: print_hex
void print_hex(const Element *elem) {
    if (elem->length < 17) { // Print on one line if short
        for (uint32_t i = 0; i < elem->length; i++) {
            fprintf(stdout, "%02X ", elem->data_ptr[i]);
        }
    } else { // Print with indentation and line breaks
        fprintf(stdout, "\n");
        print_indent(elem->depth + 1);
        for (uint32_t i = 0; i < elem->length; i++) {
            fprintf(stdout, "%02X", elem->data_ptr[i]);
            if ((i & 0x1f) == 0x1f) { // Every 32 bytes (0x1f index)
                fprintf(stdout, "\n"); // Newline
                print_indent(elem->depth + 1);
            } else {
                fprintf(stdout, " "); // Space separator
            }
        }
    }
}

// Function: read_octet_int
int read_octet_int(const uint8_t *data, uint32_t max_len, uint32_t *out_value) {
    *out_value = 0;
    for (uint32_t i = 0; i < 4 && i < max_len; i++) {
        *out_value = (*out_value << 7) | (data[i] & 0x7f);
        if (!(data[i] & 0x80)) { // If MSB is 0, it's the last octet
            return i + 1; // Number of bytes consumed
        }
    }
    return -1; // Error: more than 4 octets or end of buffer without a terminating octet
}

// Function: print_oid
void print_oid(const Element *elem) {
    uint32_t oid_sub_identifier;
    int bytes_read;
    uint32_t current_offset = 0;

    // First sub-identifier (special rule: X*40 + Y)
    bytes_read = read_octet_int(elem->data_ptr + current_offset, elem->length - current_offset, &oid_sub_identifier);
    if (bytes_read < 0 || current_offset + bytes_read > elem->length) {
        fprintf(stdout, "INVALID OID");
        return;
    }
    current_offset += bytes_read;

    // Print first two arcs
    if (oid_sub_identifier < 80) { // 0.x, 1.x (0.0 to 1.39)
        fprintf(stdout, "%u.%u", oid_sub_identifier / 40, oid_sub_identifier % 40);
    } else { // 2.x (2.0 and up)
        fprintf(stdout, "2.%u", oid_sub_identifier - 80);
    }

    // Subsequent sub-identifiers
    while (current_offset < elem->length) {
        bytes_read = read_octet_int(elem->data_ptr + current_offset, elem->length - current_offset, &oid_sub_identifier);
        if (bytes_read < 0 || current_offset + bytes_read > elem->length) {
            fprintf(stdout, ".INVALID_OID_SUFFIX"); // Indicate error but print what was parsed
            return;
        }
        current_offset += bytes_read;
        fprintf(stdout, ".%u", oid_sub_identifier);
    }
}

// Function: print_tag
void print_tag(const Element *elem) {
    switch (elem->tag_class) {
        case 0: // Universal
            if (elem->tag > 30) { // Check against size of utag_names array
                fprintf(stdout, "UNIVERSAL_%u ", elem->tag);
            } else {
                fprintf(stdout, "UNIVERSAL %s ", utag_names[elem->tag]);
            }
            break;
        case 1: // Application
            fprintf(stdout, "APPLICATION_%u ", elem->tag);
            break;
        case 2: // Context-specific
            fprintf(stdout, "[%u] ", elem->tag);
            break;
        case 3: // Private
            fprintf(stdout, "PRIVATE_%u ", elem->tag);
            break;
        default:
            fprintf(stdout, "UNKNOWN_CLASS_%u TAG_%u ", elem->tag_class, elem->tag);
            break;
    }
}

// Function: print_string
void print_string(const Element *elem) {
    for (uint32_t i = 0; i < elem->length; i++) {
        fprintf(stdout, "%c", elem->data_ptr[i]);
    }
}

// Function: print_primitive
void print_primitive(const Element *elem) {
    // The original code used a goto to handle default/non-universal cases.
    // Refactored to use standard switch-case and if-else logic.
    if (elem->tag_class == 0) { // Universal class
        switch (elem->tag) {
            case ASN1_TAG_BOOLEAN:
                fprintf(stdout, "%s", (elem->data_ptr[0] == 0) ? "False" : "True");
                break;
            case ASN1_TAG_INTEGER:
            case ASN1_TAG_BIT_STRING:
            case ASN1_TAG_OCTET_STRING:
                print_hex(elem);
                break;
            case ASN1_TAG_OBJECT_IDENTIFIER:
                print_oid(elem);
                break;
            case ASN1_TAG_UTF8_STRING:
            case ASN1_TAG_NUMERIC_STRING:
            case ASN1_TAG_PRINTABLE_STRING:
            case ASN1_TAG_T61_STRING:
            case ASN1_TAG_VIDEOTEX_STRING:
            case ASN1_TAG_IA5_STRING:
            case ASN1_TAG_VISIBLE_STRING:
                print_string(elem);
                break;
            case ASN1_TAG_UTC_TIME:
                print_time(elem, 1); // 1 for UTCTime
                break;
            case ASN1_TAG_GENERALIZED_TIME:
                print_time(elem, 0); // 0 for GeneralizedTime
                break;
            default:
                fprintf(stdout, "UNPRINTABLE");
                break;
        }
    } else { // Non-universal class or other unprintable types
        fprintf(stdout, "UNPRINTABLE");
    }
}

// Function: pprint
void pprint(Element *elem) {
    if (elem == NULL) {
        return;
    }

    print_indent(elem->depth);
    print_tag(elem);

    if (!elem->is_primitive) { // Constructed type
        fprintf(stdout, "\n");
        for (uint32_t i = 0; i < elem->count; i++) {
            pprint(elem->sub_elements[i]);
        }
    } else { // Primitive type
        print_primitive(elem);
        fprintf(stdout, "\n");
    }

    // Free the element tree when the root element (depth 0) has been processed.
    // This is an unusual place for memory management, but matches the original logic.
    if (elem->depth == 0) {
        free_element(elem);
    }
}

// Utility to convert hex character to byte value
static uint8_t hex_to_byte(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0; // Should not happen with valid hex input
}

// Utility to parse a hex string into a byte array
static uint8_t *parse_hex_string(const char *hex_str, uint32_t *out_len) {
    size_t len = strlen(hex_str);
    if (len % 2 != 0) {
        fprintf(stderr, "Error: Hex string length must be even.\n");
        return NULL;
    }
    *out_len = len / 2;
    uint8_t *data = (uint8_t *)malloc(*out_len);
    if (data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    for (size_t i = 0; i < *out_len; i++) {
        data[i] = (hex_to_byte(hex_str[i * 2]) << 4) | hex_to_byte(hex_str[i * 2 + 1]);
    }
    return data;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hex_encoded_asn1_data>\n", argv[0]);
        return EXIT_FAILURE;
    }

    uint32_t data_len;
    uint8_t *asn1_data = parse_hex_string(argv[1], &data_len);

    if (asn1_data == NULL) {
        return EXIT_FAILURE;
    }

    decode(asn1_data, data_len);

    // The asn1_data buffer allocated by parse_hex_string needs to be freed.
    // The decode function eventually frees the Element structure, but not the raw input data buffer.
    free(asn1_data);

    return EXIT_SUCCESS;
}