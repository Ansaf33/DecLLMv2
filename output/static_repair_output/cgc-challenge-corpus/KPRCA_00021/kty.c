#ifndef _GNU_SOURCE
#define _GNU_SOURCE // For dprintf, if available, otherwise use write
#endif

#include <stdio.h>    // For printf, vsnprintf
#include <stdarg.h>   // For va_list
#include <unistd.h>   // For write, read, STDIN_FILENO
#include <stdlib.h>   // For malloc, free, exit, strtol, strtod, calloc, realloc
#include <string.h>   // For strlen, strchr, strncmp, strcpy
#include <ctype.h>    // For isdigit, isspace
#include <math.h>     // For round, double

// --- Global Constants ---
const double _DAT_00015188 = 100.0; // Used in kty_double_to_string for percentage calculation

// String literals from DAT_ addresses
const char DAT_0001501c[] = "\\\\";
const char DAT_00015031[] = "%s";
const char DAT_00015022[] = "\\b";
const char DAT_0001502e[] = "\\t";
const char DAT_00015028[] = "\\n";
const char DAT_00015025[] = "\\f";
const char DAT_0001502b[] = "\\r";
const char DAT_00015019[] = "\\\"";
const char DAT_0001501f[] = "\\/";
const char DAT_000150d4[] = "  "; // Two spaces for indent
const char DAT_000150d6[] = "%d";
const char DAT_000150d9[] = "-";
const char DAT_000150db[] = "%d.";
const char DAT_000150df[] = "%02d";
const char DAT_000150e4[] = "\"";
const char DAT_000150e6[] = "{";
const char DAT_000150e8[] = "\n";
const char DAT_000150ea[] = ",";
const char DAT_000150ed[] = ": ";
const char DAT_000150f1[] = "}";
const char DAT_000150f3[] = "[";
const char DAT_000150f5[] = "]";
const char DAT_00015102[] = "null";
const char DAT_00015198[] = "%s";

// --- Forward Declarations for KTY Data Structures ---
typedef struct KtyItem KtyItem;
typedef struct KtyArray KtyArray;
typedef struct KtyObjectEntry KtyObjectEntry;
typedef struct KtyParser KtyParser;

// --- KTY Data Structures ---
struct KtyObjectEntry {
    char *key;
    KtyItem *value;
    KtyObjectEntry *next;
};

struct KtyArray {
    void **elements;
    int capacity;
    int size;
    void (*free_func)(void *);
};

struct KtyItem {
    int type; // 0=int, 1=double, 2=string, 3=object, 4=array, 5=boolean, 6=null, 7=cat
    union {
        int i_val;
        double d_val;
        struct {
            char *s_ptr;
            int s_len;
        } s_val;
        KtyObjectEntry *obj_ptr; // For objects (linked list)
        KtyArray *arr_ptr;       // For arrays
        char b_val;              // For boolean (0 or 1)
    } data;
};

struct KtyParser {
    int cat_count;
    KtyArray *nyan_array;
    void (*dumps_func)(KtyItem *);
    KtyItem *(*loads_func)(char *);
};

// --- Global Variables ---
KtyItem *my_kty = NULL;
KtyParser *parser = NULL;
void *g_parser = NULL; // Ghidra uses g_parser but it points to the same as parser.

// --- Forward Declarations for KTY Functions ---
// Custom I/O wrappers
int fdprintf(int fd, const char *format, ...);
int transmit(int fd, const void *buf, size_t count, void *ignored_param);
int receive(int fd, void *buf, size_t count, int *bytes_read_out);
void _terminate(int status);

// Array utility functions
KtyArray *array_create(int capacity, void (*free_func)(void *));
void array_append(KtyArray *arr, void *item);
void *array_get(KtyArray *arr, int index);
int array_length(KtyArray *arr);
void array_destroy(KtyArray *arr);

// Hash table utility functions (stub)
void htbl_destroy(void *htbl);

// KTY specific functions
char * eat_ws(char *param_1);
void print_escaped(const char *s, int len);
void print_indent(int indent_level);
void kty_int_to_string(KtyItem *item);
void kty_double_to_string(KtyItem *item);
void kty_string_to_string(KtyItem *item);
void kty_object_to_string(KtyItem *item, int indent_level);
void kty_array_to_string(KtyItem *item, int indent_level);
void kty_boolean_to_string(KtyItem *item);
void kty_null_to_string(KtyItem *item);
void kty_cat_to_string(KtyItem *item);
void kty_print_item(KtyItem *item, int indent_level);
void kty_dumps(KtyItem *item);
char * parse_number(KtyItem *item, char *s);
char * parse_string(KtyItem *item, char *s);
char * parse_array(KtyItem *item, char *s);
char * parse_object(KtyItem *item, char *s);
char * parse_item(KtyItem *item, char *s);
KtyItem * kty_loads(char *input_str);
int kty_init(KtyParser *p);
void free_kty_item(KtyItem *item);
int read_until(int fd, char *buf, unsigned int max_len, char delimiter);
void import_kty(char *input_str);
void print_kty(void);
void nyan(void);
void quit(void);
void menu(void);


// --- Implementations of Utility Functions (fdprintf, array, htbl) ---

int fdprintf(int fd, const char *format, ...) {
    char buffer[4096]; // A reasonable buffer size for formatted output
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (len < 0) {
        return len; // Error
    }
    // `vsnprintf` ensures null termination and truncates if buffer is too small.
    // The returned `len` is what would have been written, excluding null terminator.
    // If `len` is >= `sizeof(buffer)`, it means truncation happened.
    // We write `len` bytes or `sizeof(buffer) - 1` if truncated.
    size_t bytes_to_write = (size_t)len;
    if (bytes_to_write >= sizeof(buffer)) {
        bytes_to_write = sizeof(buffer) - 1;
    }
    return write(fd, buffer, bytes_to_write);
}

int transmit(int fd, const void *buf, size_t count, void *ignored_param) {
    (void)ignored_param; // Suppress unused parameter warning
    return write(fd, buf, count);
}

int receive(int fd, void *buf, size_t count, int *bytes_read_out) {
    ssize_t res = read(fd, buf, count);
    if (res == -1) {
        *bytes_read_out = 0;
        return -1; // Error
    }
    *bytes_read_out = (int)res;
    return 0; // Success
}

void _terminate(int status) {
    exit(status);
}

KtyArray *array_create(int capacity, void (*free_func)(void *)) {
    KtyArray *arr = (KtyArray *)malloc(sizeof(KtyArray));
    if (arr) {
        arr->capacity = capacity > 0 ? capacity : 1;
        arr->size = 0;
        arr->elements = (void **)calloc(arr->capacity, sizeof(void *));
        arr->free_func = free_func;
        if (!arr->elements) {
            free(arr);
            return NULL;
        }
    }
    return arr;
}

void array_append(KtyArray *arr, void *item) {
    if (!arr) return;
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        void **new_elements = (void **)realloc(arr->elements, arr->capacity * sizeof(void *));
        if (!new_elements) {
            // Realloc failed, item might be leaked, but we must not corrupt array
            return;
        }
        arr->elements = new_elements;
    }
    arr->elements[arr->size++] = item;
}

void *array_get(KtyArray *arr, int index) {
    if (!arr || index < 0 || index >= arr->size) {
        return NULL;
    }
    return arr->elements[index];
}

int array_length(KtyArray *arr) {
    return arr ? arr->size : 0;
}

void array_destroy(KtyArray *arr) {
    if (arr) {
        if (arr->elements) {
            if (arr->free_func) {
                for (int i = 0; i < arr->size; ++i) {
                    if (arr->elements[i]) {
                        arr->free_func(arr->elements[i]);
                    }
                }
            }
            free(arr->elements);
        }
        free(arr);
    }
}

void htbl_destroy(void *htbl) {
    // This is a stub. In the KTY structure, object data is a linked list (KtyObjectEntry*).
    // The freeing of KtyObjectEntry linked list elements is handled by free_kty_item
    // when a KTY_OBJECT item is freed. So, this function is effectively a no-op in this context.
    (void)htbl; // Suppress unused parameter warning
}

// --- Function: eat_ws
char * eat_ws(char *s) {
  if (s == NULL) return NULL;
  for (; *s != '\0' && (unsigned char)*s < '!'; s++);
  return s;
}

// --- Function: print_escaped
void print_escaped(const char *s, int len) {
  for (int i = 0; i < len; ++i) {
    char c = s[i];
    switch (c) {
      case '"':  fdprintf(1, DAT_00015019); break; // \"
      case '\\': fdprintf(1, DAT_0001501c); break; // \\
      case '/':  fdprintf(1, DAT_0001501f); break; // \/ (optional escape in JSON, but present in original)
      case '\b': fdprintf(1, DAT_00015022); break; // \b
      case '\f': fdprintf(1, DAT_00015025); break; // \f
      case '\n': fdprintf(1, DAT_00015028); break; // \n
      case '\r': fdprintf(1, DAT_0001502b); break; // \r
      case '\t': fdprintf(1, DAT_0001502e); break; // \t
      default:
        // For other control characters (0x00-0x1F) or non-ASCII (>=0x7F), use \u00xx escape.
        // Otherwise, print the character literally.
        if ((unsigned char)c < 0x20 || (unsigned char)c >= 0x7F) {
            fdprintf(1, "\\u%04x", (unsigned char)c);
        } else {
            fdprintf(1, "%c", c);
        }
        break;
    }
  }
}

// --- Function: print_indent
void print_indent(int indent_level) {
  for (int i = 0; i < indent_level; ++i) {
    fdprintf(1, DAT_000150d4); // "  "
  }
}

// --- Function: kty_int_to_string
void kty_int_to_string(KtyItem *item) {
  fdprintf(1, DAT_000150d6, item->data.i_val); // %d
}

// --- Function: kty_double_to_string
void kty_double_to_string(KtyItem *item) {
  double d_val = item->data.d_val;
  if (d_val < 0.0 && (int)round(d_val) == 0) { // Check for negative zero or small negative that rounds to zero
    fdprintf(1, DAT_000150d9); // "-"
  }
  fdprintf(1, DAT_000150db, (int)round(d_val)); // "%d."
  int fractional_part = (int)round(_DAT_00015188 * d_val) % 100;
  if (fractional_part < 0) {
    fractional_part = -fractional_part;
  }
  fdprintf(1, DAT_000150df, fractional_part); // "%02d"
}

// --- Function: kty_string_to_string
void kty_string_to_string(KtyItem *item) {
  fdprintf(1, DAT_000150e4); // "\""
  print_escaped(item->data.s_val.s_ptr, item->data.s_val.s_len);
  fdprintf(1, DAT_000150e4); // "\""
}

// --- Function: kty_object_to_string
void kty_object_to_string(KtyItem *item, int indent_level) {
  fdprintf(1, DAT_000150e6); // "{"
  KtyObjectEntry *current_entry = item->data.obj_ptr;
  int count = 0;

  if (current_entry != NULL) {
    fdprintf(1, DAT_000150e8); // "\n"
    for (; current_entry != NULL; current_entry = current_entry->next) {
      if (count > 0) {
        fdprintf(1, DAT_000150ea); // ","
        fdprintf(1, DAT_000150e8); // "\n"
      }
      count++;
      print_indent(indent_level + 1);
      fdprintf(1, DAT_000150e4); // "\""
      print_escaped(current_entry->key, strlen(current_entry->key));
      fdprintf(1, DAT_000150e4); // "\""
      fdprintf(1, DAT_000150ed); // ": "
      kty_print_item(current_entry->value, indent_level + 1);
    }
  }

  if (count > 0) {
    fdprintf(1, DAT_000150e8); // "\n"
    print_indent(indent_level);
  }
  fdprintf(1, DAT_000150f1); // "}"
}

// --- Function: kty_array_to_string
void kty_array_to_string(KtyItem *item, int indent_level) {
  fdprintf(1, DAT_000150f3); // "["
  KtyArray *arr = item->data.arr_ptr;
  int count = 0;
  int len = array_length(arr);

  if (len > 0) {
    fdprintf(1, DAT_000150e8); // "\n"
  }

  for (int i = 0; i < len; ++i) {
    if (count > 0) {
      fdprintf(1, DAT_000150ea); // ","
      fdprintf(1, DAT_000150e8); // "\n"
    }
    count++;
    print_indent(indent_level + 1);
    KtyItem *element = (KtyItem *)array_get(arr, i);
    kty_print_item(element, indent_level + 1);
  }

  if (count > 0) {
    fdprintf(1, DAT_000150e8); // "\n"
    print_indent(indent_level);
  }
  fdprintf(1, DAT_000150f5); // "]"
}

// --- Function: kty_boolean_to_string
void kty_boolean_to_string(KtyItem *item) {
  const char *str_val = item->data.b_val == 0 ? "false" : "true";
  fdprintf(1, DAT_00015031, str_val); // "%s"
}

// --- Function: kty_null_to_string
void kty_null_to_string(KtyItem *item) { // item parameter added for consistency, but unused
  (void)item; // Suppress unused parameter warning
  fdprintf(1, DAT_00015102); // "null"
}

// --- Function: kty_cat_to_string
void kty_cat_to_string(KtyItem *item) { // item parameter added for consistency, but unused
  (void)item; // Suppress unused parameter warning
  if (parser) {
      parser->cat_count++;
  }
  fdprintf(1, "=^.^=");
}

// --- Function: kty_print_item
void kty_print_item(KtyItem *item, int indent_level) {
  if (item == NULL) {
    fdprintf(1, "null"); // Or handle error appropriately
    return;
  }
  switch (item->type) {
    case 0: kty_int_to_string(item); break;
    case 1: kty_double_to_string(item); break;
    case 2: kty_string_to_string(item); break;
    case 3: kty_object_to_string(item, indent_level); break;
    case 4: kty_array_to_string(item, indent_level); break;
    case 5: kty_boolean_to_string(item); break;
    case 6: kty_null_to_string(item); break;
    case 7: kty_cat_to_string(item); break;
    default:
        fdprintf(1, "<UNKNOWN_TYPE>"); // Handle unknown types
        break;
  }
}

// --- Function: kty_dumps
void kty_dumps(KtyItem *item) {
  if (parser) {
      parser->cat_count = 0;
  }
  if (item != NULL) {
    kty_print_item(item, 0);
  }
}

// --- Function: parse_number
char * parse_number(KtyItem *item, char *s) {
  char *endptr = NULL;
  char *decimal_point = strchr(s, '.');
  char *delimiter_space = strchr(s, ' ');
  char *delimiter_comma = strchr(s, ',');
  char *delimiter_sq_bracket = strchr(s, ']');
  char *delimiter_curly_bracket = strchr(s, '}');

  // Find the first delimiter
  char *first_delimiter = NULL;
  if (delimiter_space) first_delimiter = delimiter_space;
  if (delimiter_comma && (first_delimiter == NULL || delimiter_comma < first_delimiter)) first_delimiter = delimiter_comma;
  if (delimiter_sq_bracket && (first_delimiter == NULL || delimiter_sq_bracket < first_delimiter)) first_delimiter = delimiter_sq_bracket;
  if (delimiter_curly_bracket && (first_delimiter == NULL || delimiter_curly_bracket < first_delimiter)) first_delimiter = delimiter_curly_bracket;

  // Determine if it's a double: a decimal point exists before any other delimiter
  int is_double = 0;
  if (decimal_point != NULL) {
    if (first_delimiter == NULL || decimal_point < first_delimiter) {
      is_double = 1;
    }
  }

  if (is_double) {
    item->type = 1; // KTY_DOUBLE
    item->data.d_val = strtod(s, &endptr);
  } else {
    item->type = 0; // KTY_INT
    item->data.i_val = (int)strtol(s, &endptr, 10);
  }
  return endptr;
}

// --- Function: parse_string
char * parse_string(KtyItem *item, char *s) {
  if (*s != '"') return NULL; // String must start with a quote

  char *read_ptr = s + 1; // Start after opening quote
  int decoded_len = 0;    // Length of the unescaped string
  int total_chars_to_advance = 0; // Total chars from s to advance past, including escapes and quotes

  // First pass: calculate decoded length and total characters to read
  while (*read_ptr != '\0' && *read_ptr != '"') {
    if (*read_ptr == '\\') {
      read_ptr++; // Skip backslash
      if (*read_ptr == '\0') return NULL; // Malformed escape
    }
    decoded_len++;
    read_ptr++;
    total_chars_to_advance++;
  }

  if (*read_ptr != '"') return NULL; // String must end with a quote

  // The original code had a 0x100 (256) limit. Let's keep it for now.
  if (decoded_len >= 0x100) return NULL; // String too long

  char *decoded_str = (char *)malloc(decoded_len + 1);
  if (!decoded_str) return NULL;

  item->type = 2; // KTY_STRING
  item->data.s_val.s_ptr = decoded_str;
  item->data.s_val.s_len = decoded_len;

  read_ptr = s + 1; // Reset read pointer to after opening quote
  int write_idx = 0;

  // Second pass: decode and copy string
  while (*read_ptr != '\0' && *read_ptr != '"') {
    if (*read_ptr == '\\') {
      read_ptr++;
      switch (*read_ptr) {
        case 'b': decoded_str[write_idx] = '\b'; break;
        case 'f': decoded_str[write_idx] = '\f'; break;
        case 'n': decoded_str[write_idx] = '\n'; break;
        case 'r': decoded_str[write_idx] = '\r'; break;
        case 't': decoded_str[write_idx] = '\t'; break;
        case '"': // Fallthrough for \"
        case '\\': // Fallthrough for \\
        case '/': // Fallthrough for \/
        default:  decoded_str[write_idx] = *read_ptr; break; // Copy literally
      }
    } else {
      decoded_str[write_idx] = *read_ptr;
    }
    write_idx++;
    read_ptr++;
  }
  decoded_str[write_idx] = '\0'; // Null-terminate the decoded string

  return s + total_chars_to_advance + 2; // Advance past the whole quoted string (total_chars_to_advance + two quotes)
}

// --- Function: parse_array
char * parse_array(KtyItem *item, char *s) {
  if (*s != '[') return NULL; // Must start with '['

  item->type = 4; // KTY_ARRAY
  item->data.arr_ptr = NULL; // Initialize to NULL

  char *read_ptr = eat_ws(s + 1); // Skip '[' and whitespace

  if (*read_ptr == ']') {
    item->data.arr_ptr = array_create(0, free_kty_item); // Empty array
    return read_ptr + 1; // Advance past ']'
  }

  KtyArray *arr = array_create(4, free_kty_item); // Initial capacity 4
  if (!arr) return NULL; // Array creation failed
  item->data.arr_ptr = arr;

  KtyItem *current_item_ptr = NULL; // For parsing each element

  while (1) { // Loop until ']' or error
    current_item_ptr = (KtyItem *)malloc(sizeof(KtyItem)); // Allocate memory for the item
    if (!current_item_ptr) {
        array_destroy(arr); // Clean up already parsed items
        item->data.arr_ptr = NULL;
        return NULL; // Memory allocation failed
    }

    read_ptr = eat_ws(read_ptr); // Skip whitespace before item
    read_ptr = parse_item(current_item_ptr, read_ptr);
    if (!read_ptr) { // Item parsing failed
        free_kty_item(current_item_ptr); // Free current item (will free current_item_ptr itself)
        array_destroy(arr);
        item->data.arr_ptr = NULL;
        return NULL;
    }
    array_append(arr, current_item_ptr);

    read_ptr = eat_ws(read_ptr); // Skip whitespace after item

    if (*read_ptr == ',') {
      read_ptr++; // Consume ','
    } else if (*read_ptr == ']') {
      return read_ptr + 1; // Found end of array, return
    } else {
      // Unexpected character after item
      array_destroy(arr);
      item->data.arr_ptr = NULL;
      return NULL;
    }
  }
}

// --- Function: parse_object
char * parse_object(KtyItem *item, char *s) {
  if (*s != '{') return NULL; // Must start with '{'

  item->type = 3; // KTY_OBJECT
  item->data.obj_ptr = NULL; // Initialize to NULL

  char *read_ptr = eat_ws(s + 1); // Skip '{' and whitespace

  if (*read_ptr == '}') {
    // Empty object
    return read_ptr + 1; // Advance past '}'
  }

  KtyObjectEntry *head = NULL;
  KtyObjectEntry *tail = NULL;
  char *key_str = NULL;

  while (1) { // Loop until '}' or error
    // Parse key (must be a string)
    read_ptr = eat_ws(read_ptr);
    if (*read_ptr != '"') {
      // Error cleanup: free existing entries
      KtyObjectEntry *curr = head;
      while (curr) {
          KtyObjectEntry *next = curr->next;
          free(curr->key);
          free_kty_item(curr->value);
          free(curr);
          curr = next;
      }
      return NULL;
    }

    KtyItem key_item_temp; // Temporary KtyItem for parsing the key string
    char *key_end_ptr = parse_string(&key_item_temp, read_ptr);
    if (!key_end_ptr) {
        // Error cleanup
        KtyObjectEntry *curr = head;
        while (curr) {
            KtyObjectEntry *next = curr->next;
            free(curr->key);
            free_kty_item(curr->value);
            free(curr);
            curr = next;
        }
        return NULL;
    }
    key_str = key_item_temp.data.s_val.s_ptr; // Get the parsed key string
    read_ptr = key_end_ptr;

    // Parse ':'
    read_ptr = eat_ws(read_ptr);
    if (*read_ptr != ':') {
        free(key_str); // Free key string on error
        // Error cleanup
        KtyObjectEntry *curr = head;
        while (curr) {
            KtyObjectEntry *next = curr->next;
            free(curr->key);
            free_kty_item(curr->value);
            free(curr);
            curr = next;
        }
        return NULL;
    }
    read_ptr++; // Consume ':'

    // Parse value
    KtyItem *value_item = (KtyItem *)malloc(sizeof(KtyItem));
    if (!value_item) {
        free(key_str); // Free key string on error
        // Error cleanup
        KtyObjectEntry *curr = head;
        while (curr) {
            KtyObjectEntry *next = curr->next;
            free(curr->key);
            free_kty_item(curr->value);
            free(curr);
            curr = next;
        }
        return NULL; // Memory allocation failed
    }

    read_ptr = eat_ws(read_ptr);
    read_ptr = parse_item(value_item, read_ptr);
    if (!read_ptr) { // Value parsing failed
        free(key_str); // Free key string on error
        free(value_item); // Free value item on error
        // Error cleanup
        KtyObjectEntry *curr = head;
        while (curr) {
            KtyObjectEntry *next = curr->next;
            free(curr->key);
            free_kty_item(curr->value);
            free(curr);
            curr = next;
        }
        return NULL;
    }

    // Create and link new object entry
    KtyObjectEntry *new_entry = (KtyObjectEntry *)malloc(sizeof(KtyObjectEntry));
    if (!new_entry) {
        free(key_str);
        free_kty_item(value_item);
        // Error cleanup
        KtyObjectEntry *curr = head;
        while (curr) {
            KtyObjectEntry *next = curr->next;
            free(curr->key);
            free_kty_item(curr->value);
            free(curr);
            curr = next;
        }
        return NULL;
    }
    new_entry->key = key_str;
    new_entry->value = value_item;
    new_entry->next = NULL;

    if (head == NULL) {
      head = new_entry;
      tail = new_entry;
    } else {
      tail->next = new_entry;
      tail = new_entry;
    }

    read_ptr = eat_ws(read_ptr);

    if (*read_ptr == ',') {
      read_ptr++; // Consume ','
    } else if (*read_ptr == '}') {
      item->data.obj_ptr = head; // Assign the linked list to obj_ptr
      return read_ptr + 1; // Found end of object, return
    } else {
      // Unexpected character
      // Error cleanup
      KtyObjectEntry *curr = head;
      while (curr) {
          KtyObjectEntry *next = curr->next;
          free(curr->key);
          free_kty_item(curr->value);
          free(curr);
          curr = next;
      }
      return NULL;
    }
  }
}

// --- Function: parse_item
char * parse_item(KtyItem *item, char *s) {
  if (item == NULL || s == NULL) return NULL;

  // Check for keywords
  if (strncmp(s, "true", 4) == 0) {
    item->type = 5; // KTY_BOOLEAN
    item->data.b_val = 1;
    return s + 4;
  }
  if (strncmp(s, "false", 5) == 0) {
    item->type = 5; // KTY_BOOLEAN
    item->data.b_val = 0;
    return s + 5;
  }
  if (strncmp(s, "null", 4) == 0) {
    item->type = 6; // KTY_NULL
    // No additional data needed for null
    return s + 4;
  }
  if (strncmp(s, "=^.^=", 5) == 0) {
    item->type = 7; // KTY_CAT
    // No additional data needed for cat
    return s + 5;
  }

  char c = *s;
  if (c == '{') {
    return parse_object(item, s);
  }
  if (c == '[') {
    return parse_array(item, s);
  }
  if (c == '"') {
    return parse_string(item, s);
  }
  // Check for numbers: digits, '-', '+' or '.'
  if (isdigit((unsigned char)c) || c == '-' || c == '+' || c == '.') {
    return parse_number(item, s);
  }

  return NULL; // Unknown item type or parse error
}

// --- Function: kty_loads
KtyItem * kty_loads(char *input_str) {
  KtyItem *root_item = (KtyItem *)calloc(1, sizeof(KtyItem)); // Allocate KtyItem
  if (!root_item) return NULL;

  if (parser) {
      if (parser->nyan_array) {
          array_destroy(parser->nyan_array);
          parser->nyan_array = NULL;
      }
      // Create new array for nyan, with free_kty_item as cleanup function
      parser->nyan_array = array_create(0x10, (void (*)(void*))free_kty_item);
      if (!parser->nyan_array) {
          free(root_item);
          return NULL;
      }
  }

  char *trimmed_str = eat_ws(input_str);
  char *end_ptr = parse_item(root_item, trimmed_str);

  if (end_ptr != NULL) {
    // Optionally, check if there's any remaining non-whitespace data after the item
    end_ptr = eat_ws(end_ptr);
    if (*end_ptr == '\0') {
      return root_item;
    }
  }

  // If parsing failed or extra data found, free the allocated item
  free_kty_item(root_item); // This will free root_item and its contents
  return NULL;
}

// --- Function: kty_init
int kty_init(KtyParser *p) {
  if (p == NULL) return -1; // Error

  p->cat_count = 0;
  p->nyan_array = NULL;
  p->dumps_func = kty_dumps;
  p->loads_func = kty_loads;
  parser = p; // Assign to global parser pointer
  g_parser = p; // Also assign to g_parser (seems redundant/same variable)

  return 0; // Success
}

// --- Function: free_kty_item
void free_kty_item(KtyItem *item) {
  if (item != NULL) {
    switch (item->type) {
      case 2: // KTY_STRING
        if (item->data.s_val.s_ptr != NULL) {
          free(item->data.s_val.s_ptr);
        }
        break;
      case 3: // KTY_OBJECT
        // Free linked list of KtyObjectEntry
        KtyObjectEntry *current_entry = item->data.obj_ptr;
        while (current_entry) {
            KtyObjectEntry *next = current_entry->next;
            free(current_entry->key); // Free key string
            free_kty_item(current_entry->value); // Recursively free value item
            free(current_entry); // Free the entry itself
            current_entry = next;
        }
        break;
      case 4: // KTY_ARRAY
        array_destroy(item->data.arr_ptr); // array_destroy handles freeing elements
        break;
      // For INT, DOUBLE, BOOLEAN, NULL, CAT, no additional nested allocations
    }
    free(item); // Free the KtyItem itself
  }
}

// --- Function: read_until
int read_until(int fd, char *buf, unsigned int max_len, char delimiter) {
  unsigned int i;
  int bytes_read_single;
  for (i = 0; i < max_len; ++i) {
    if (receive(fd, buf + i, 1, &bytes_read_single) != 0 || bytes_read_single == 0) {
      // Error or EOF
      buf[i] = '\0'; // Null-terminate what was read so far
      return -1;
    }
    if (buf[i] == delimiter) {
      buf[i] = '\0'; // Replace delimiter with null terminator
      return (int)i; // Return number of characters *before* the delimiter
    }
  }
  buf[max_len - 1] = '\0'; // Ensure buffer is null-terminated if max_len reached
  return (int)(max_len - 1); // Indicate max_len was reached without delimiter, return actual content length
}

// --- Function: import_kty
void import_kty(char *input_str) {
  if (my_kty != NULL) {
    free_kty_item(my_kty);
    my_kty = NULL;
  }
  if (parser) {
      parser->cat_count = 0; // Reset cat count for new import
      my_kty = parser->loads_func(input_str);
  } else {
      // Parser not initialized, attempt to load directly (should not happen if main is correct)
      my_kty = kty_loads(input_str);
  }

  if (my_kty == NULL) {
    fdprintf(1, "Error importing KTY data!\n");
  }
}

// --- Function: print_kty
void print_kty(void) {
  if (my_kty == NULL) {
    fdprintf(1, "Error: No KTY data to print!\n");
  } else {
    if (parser) {
        parser->cat_count = 0; // Reset cat count before printing
        parser->dumps_func(my_kty);
    } else {
        kty_dumps(my_kty); // Fallback if parser is NULL
    }
    fdprintf(1, "\n"); // Add a newline for cleaner output
  }
}

// --- Function: nyan
void nyan(void) {
  // ASCII art for the cat
  const char *cat_art =
           "\n\n+      o     +              o\n    +             o     +       +\no          +\n    o  +           +        +\n+        o     o       +        o\n-_-_-_-_-_-_-_,------,      o\n_-_-_-_-_-_-_-|   /\\_/\\\n-_-_-_-_-_-_-~|__( ^ .^)  +     +\n_-_-_-_-_-_-_-\"\"  \"\"\n+      o         o   +       o\n    +         +\no        o         o      o     +\n    o           +\n+      +     o        o      +\n\n";

  char nyan_message_buffer[8192] = {0}; // Buffer for cat's message
  char *current_pos = nyan_message_buffer;

  if (my_kty == NULL) {
    fdprintf(1, "Error: No KTY data to nyan!\n");
  } else {
    if (parser) {
        parser->dumps_func(my_kty); // Print the KTY item
        if (parser->cat_count > 2) { // Check if cat_count is greater than 2
          fdprintf(1, DAT_00015198, cat_art); // Print cat art
          int len = array_length(parser->nyan_array);
          for (int i = 0; i < len; ++i) {
            KtyItem *item = (KtyItem *)array_get(parser->nyan_array, i);
            // Only append string items to the nyan message
            if (item && item->type == 2 && item->data.s_val.s_ptr) {
                int str_len = item->data.s_val.s_len;
                // Ensure not to overflow buffer
                if (current_pos + str_len + 1 < nyan_message_buffer + sizeof(nyan_message_buffer)) {
                    strcpy(current_pos, item->data.s_val.s_ptr);
                    current_pos += str_len;
                } else {
                    // Buffer full, truncate message and break
                    *(nyan_message_buffer + sizeof(nyan_message_buffer) - 1) = '\0';
                    break;
                }
            }
          }
          fdprintf(1, "NYAN SAYS...\n\"\n%s\n\"", nyan_message_buffer);
        }
    } else {
        kty_dumps(my_kty); // Fallback if parser is NULL
    }
    fdprintf(1, "\n"); // Add a newline for cleaner output
  }
}

// --- Function: quit
void quit(void) {
  fdprintf(1, "\n\n=^.^=// Bye!\n\n");
  if (my_kty) {
      free_kty_item(my_kty);
      my_kty = NULL;
  }
  if (parser) {
      if (parser->nyan_array) {
          array_destroy(parser->nyan_array);
          parser->nyan_array = NULL;
      }
      free(parser); // Free the parser struct itself
      parser = NULL;
      g_parser = NULL;
  }
  exit(0);
}

// --- Function: menu
void menu(void) {
  fdprintf(1, "=======================\n");
  fdprintf(1, " 1. Import KTY\n");
  fdprintf(1, " 2. Print KTY\n");
  fdprintf(1, " 3. Quit\n");
  fdprintf(1, "=======================\n");
}

// --- Function: main
int main(void) {
  char menu_choice_buffer[16];
  char kty_input_buffer[8192];
  long choice;

  fdprintf(1, "KTY Pretty Printer v0.1\n");

  parser = (KtyParser *)malloc(sizeof(KtyParser));
  if (!parser) {
    fdprintf(1, "Error: Failed to allocate parser.\n");
    exit(1);
  }

  if (kty_init(parser) != 0) {
    fdprintf(1, "Error: KTY initialization failed!\n");
    quit(); // Exits
  }

  menu(); // Display menu once

  while (1) {
    fdprintf(1, "> "); // Prompt for input
    int bytes_read_menu = read_until(STDIN_FILENO, menu_choice_buffer, sizeof(menu_choice_buffer), '\n');
    if (bytes_read_menu < 0) { // Error or EOF
      fdprintf(1, "Error reading input. Exiting.\n");
      break;
    }
    if (bytes_read_menu == 0) { // Empty input, prompt again
        continue;
    }

    choice = strtol(menu_choice_buffer, NULL, 10);

    if (choice == 777) { // 0x309
      nyan();
    } else if (choice == 1) {
      fdprintf(1, "Enter KTY data (terminate with null byte, or newline for interactive use): ");
      // The original code used 0 (NULL byte) as delimiter for KTY data.
      // For interactive use, this is problematic. I'll read until newline.
      // If a NULL byte is truly intended, input should be piped (e.g., echo -ne "{...}\0" | ./program)
      int kty_data_len = read_until(STDIN_FILENO, kty_input_buffer, sizeof(kty_input_buffer), '\n');
      if (kty_data_len < 0) {
        fdprintf(1, "Error reading KTY data.\n");
      } else {
        import_kty(kty_input_buffer);
      }
    } else if (choice == 2) {
      print_kty();
    } else if (choice == 3) {
      quit(); // Exits the program
    } else {
      fdprintf(1, "Invalid menu option. Try again.\n");
    }
    menu(); // Display menu again
  }

  return 0;
}