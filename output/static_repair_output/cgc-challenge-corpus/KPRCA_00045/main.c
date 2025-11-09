#include <stdio.h>    // For dprintf, snprintf, fprintf, stderr
#include <stdlib.h>   // For malloc, free, exit, strtoul, strtof, strtol, calloc
#include <string.h>   // For memset, strcmp, strdup, strlen, memcpy
#include <unistd.h>   // For dprintf (if not implicitly included by stdio.h), read
#include <stdbool.h>  // For bool type
#include <stdint.h>   // For uintptr_t

// --- Global Data and Macros ---

// Global string literals (renamed from DAT_... for clarity and C standards)
const char G_DAT_NEWLINE[] = "\n";
const char G_DAT_RENDER_FORMAT[] = "%s\n";
const char G_DAT_BANNER_FORMAT[] = "%s\n";
const char G_DAT_PROMPT[] = "sadface> ";
const char G_BANNER_TEXT[] = "SadFace Template Engine v0.1\n";

// Helper for fdprintf (assuming it's dprintf to file descriptor 1, stdout)
#define fdprintf dprintf

// --- Custom Types and Structures ---

// Enum for variable types
typedef enum {
    VAR_TYPE_STRING = 0,
    VAR_TYPE_INTEGER,
    VAR_TYPE_FLOAT,
    VAR_TYPE_BOOL
} sadface_vartype_t;

// sadface_var_t structure to store variable type and data.
// It uses uintptr_t to store either a pointer (for strings) or the direct value
// (for integers, floats, booleans).
// On 32-bit systems, sizeof(uintptr_t) is typically 4 bytes.
// On 64-bit systems, sizeof(uintptr_t) is typically 8 bytes.
// The original code's malloc(8) implies a 32-bit environment or a specific
// packing. Using sizeof(sadface_var_t) is more robust for portability.
typedef struct {
    sadface_vartype_t type;
    uintptr_t value_data; // Holds pointer for string, or value for int/float/bool
} sadface_var_t;

// Dictionary node structure for linked list within each bucket
typedef struct dict_node {
    char *key;
    sadface_var_t *value; // Points to a sadface_var_t allocated on heap
    struct dict_node *next;
} dict_node_t;

#define DICT_SIZE 0x100 // Number of buckets in the hash table
typedef struct {
    dict_node_t *buckets[DICT_SIZE];
} dict_t;

// --- Forward Declarations of Mock/External Functions ---

// Dictionary operations
dict_t *dict_new(void);
unsigned int hash_string(const char *str);
void dict_insert(dict_t *dict, const char *key, sadface_var_t *value);
sadface_var_t *dict_remove(dict_t *dict, const char *key);
sadface_var_t *dict_lookup(dict_t *dict, const char *key);

// Sadface rendering engine operations
void *sadface_init(void **ctx_out, int flags, const char *template_text, dict_t *vars);
int sadface_render(void *ctx, char *output_buffer, size_t *output_len);
char *sadface_var2str(sadface_var_t *var); // Returns malloc'd string

// Input reading utility
int read_until(int fd, char *buffer, size_t max_len, char terminator);

// --- Mock Implementations (Minimal for compilation and basic functionality) ---

// Basic hash function (djb2)
unsigned int hash_string(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % DICT_SIZE;
}

// Allocates and initializes a new dictionary
dict_t *dict_new(void) {
    dict_t *dict = calloc(1, sizeof(dict_t));
    if (!dict) {
        dprintf(2, "Error: dict_new failed to allocate memory.\n");
        exit(1);
    }
    return dict;
}

// Inserts a key-value pair into the dictionary. Updates if key exists.
void dict_insert(dict_t *dict, const char *key, sadface_var_t *value) {
    unsigned int idx = hash_string(key);
    dict_node_t *node = dict->buckets[idx];
    dict_node_t *prev = NULL;

    // Check if key already exists, update its value
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            node->value = value; // Update value for existing key
            return;
        }
        prev = node;
        node = node->next;
    }

    // Key does not exist, create new node
    dict_node_t *new_node = malloc(sizeof(dict_node_t));
    if (!new_node) {
        dprintf(2, "Error: dict_insert failed to allocate memory for node.\n");
        exit(1);
    }
    new_node->key = strdup(key);
    if (!new_node->key) {
        dprintf(2, "Error: dict_insert failed to allocate memory for key string.\n");
        free(new_node);
        exit(1);
    }
    new_node->value = value;
    new_node->next = NULL;

    if (prev == NULL) { // Insert at the head of the bucket
        dict->buckets[idx] = new_node;
    } else { // Insert after the previous node
        prev->next = new_node;
    }
}

// Removes a key-value pair from the dictionary and returns the value.
// Caller is responsible for freeing the returned sadface_var_t*.
sadface_var_t *dict_remove(dict_t *dict, const char *key) {
    unsigned int idx = hash_string(key);
    dict_node_t *node = dict->buckets[idx];
    dict_node_t *prev = NULL;

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            if (prev == NULL) { // Node is the head of the bucket
                dict->buckets[idx] = node->next;
            } else {
                prev->next = node->next;
            }
            sadface_var_t *removed_value = node->value;
            free(node->key); // Free the key string
            free(node);      // Free the dictionary node
            return removed_value;
        }
        prev = node;
        node = node->next;
    }
    return NULL; // Key not found
}

// Looks up a key in the dictionary and returns its value.
sadface_var_t *dict_lookup(dict_t *dict, const char *key) {
    unsigned int idx = hash_string(key);
    dict_node_t *node = dict->buckets[idx];
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

// Converts a sadface_var_t to a dynamically allocated string representation.
// The caller is responsible for freeing the returned string.
char *sadface_var2str(sadface_var_t *var) {
    char buffer[256]; // Temporary buffer for snprintf
    int len;

    if (!var) return strdup("(null)");

    switch (var->type) {
        case VAR_TYPE_STRING:
            return strdup((char *)var->value_data);
        case VAR_TYPE_INTEGER:
            len = snprintf(buffer, sizeof(buffer), "%ld", (long)var->value_data);
            break;
        case VAR_TYPE_FLOAT: {
            // Reinterpret the uintptr_t as a float by using a union
            union { float f; uintptr_t u; } converter;
            converter.u = var->value_data;
            len = snprintf(buffer, sizeof(buffer), "%f", converter.f);
            break;
        }
        case VAR_TYPE_BOOL:
            len = snprintf(buffer, sizeof(buffer), "%s", var->value_data ? "true" : "false");
            break;
        default:
            return strdup("(unknown type)");
    }
    if (len < 0 || (size_t)len >= sizeof(buffer)) {
        return strdup("(conversion error or truncated)");
    }
    return strdup(buffer);
}

// Context structure for the sadface rendering engine
typedef struct {
    const char *template_text;
    dict_t *vars;
} sadface_ctx_t;

// Initializes the sadface rendering context.
// Sets *ctx_out to a newly allocated context and returns 0 on success, -1 on failure.
void *sadface_init(void **ctx_out, int flags, const char *template_text, dict_t *vars) {
    (void)flags; // flags parameter is unused in this mock
    sadface_ctx_t *new_ctx = malloc(sizeof(sadface_ctx_t));
    if (!new_ctx) {
        dprintf(2, "Error: sadface_init failed to allocate context.\n");
        *ctx_out = NULL;
        return NULL; // Return NULL to indicate failure
    }
    new_ctx->template_text = template_text;
    new_ctx->vars = vars;
    *ctx_out = new_ctx;
    return new_ctx; // Return the context pointer on success
}

// Renders the template text using variables from the dictionary.
// Output is written to output_buffer, and output_len is updated with actual length.
// Returns 0 on success, -1 on failure. Frees the rendering context.
int sadface_render(void *ctx_ptr, char *output_buffer, size_t *output_len) {
    if (!ctx_ptr || !output_buffer || !output_len || *output_len == 0) {
        dprintf(2, "Error: sadface_render received null arguments or zero output_len.\n");
        return -1;
    }
    sadface_ctx_t *ctx = (sadface_ctx_t *)ctx_ptr;
    const char *template = ctx->template_text;
    size_t current_output_len = 0;
    const char *p = template;
    const char *start_tag;

    // Iterate through the template, substituting variables
    while (*p != '\0' && current_output_len < *output_len - 1) { // Leave space for null terminator
        start_tag = strstr(p, "{{");
        if (start_tag) {
            // Copy text before the variable tag
            size_t text_before_tag_len = start_tag - p;
            if (current_output_len + text_before_tag_len >= *output_len - 1) {
                text_before_tag_len = (*output_len - 1) - current_output_len;
            }
            memcpy(output_buffer + current_output_len, p, text_before_tag_len);
            current_output_len += text_before_tag_len;
            p += text_before_tag_len;

            if (current_output_len >= *output_len - 1) break; // Output buffer full

            // Look for the closing tag
            const char *end_tag = strstr(start_tag + 2, "}}");
            if (end_tag) {
                // Extract variable name
                size_t var_name_len = end_tag - (start_tag + 2);
                char var_name[64]; // Limit variable name length
                if (var_name_len >= sizeof(var_name)) var_name_len = sizeof(var_name) - 1;
                memcpy(var_name, start_tag + 2, var_name_len);
                var_name[var_name_len] = '\0';

                // Look up and substitute variable value
                sadface_var_t *var = dict_lookup(ctx->vars, var_name);
                if (var) {
                    char *var_str = sadface_var2str(var);
                    if (var_str) {
                        size_t var_str_len = strlen(var_str);
                        if (current_output_len + var_str_len >= *output_len - 1) {
                            var_str_len = (*output_len - 1) - current_output_len; // Truncate if too long
                        }
                        memcpy(output_buffer + current_output_len, var_str, var_str_len);
                        current_output_len += var_str_len;
                        free(var_str);
                    }
                } else {
                    // Variable not found, insert a placeholder message
                    char not_found_msg[128];
                    int nf_len = snprintf(not_found_msg, sizeof(not_found_msg), "{{%s (not found)}}", var_name);
                    if (nf_len > 0) {
                        size_t msg_len = (size_t)nf_len;
                        if (current_output_len + msg_len >= *output_len - 1) {
                            msg_len = (*output_len - 1) - current_output_len;
                        }
                        memcpy(output_buffer + current_output_len, not_found_msg, msg_len);
                        current_output_len += msg_len;
                    }
                }
                p = end_tag + 2; // Move past the closing tag
            } else {
                // No closing tag found, treat "{{..." as literal text
                size_t remaining_len = strlen(p);
                if (current_output_len + remaining_len >= *output_len - 1) {
                    remaining_len = (*output_len - 1) - current_output_len;
                }
                memcpy(output_buffer + current_output_len, p, remaining_len);
                current_output_len += remaining_len;
                p += remaining_len;
            }
        } else {
            // No more tags, copy the rest of the template as literal
            size_t remaining_len = strlen(p);
            if (current_output_len + remaining_len >= *output_len - 1) {
                remaining_len = (*output_len - 1) - current_output_len;
            }
            memcpy(output_buffer + current_output_len, p, remaining_len);
            current_output_len += remaining_len;
            p += remaining_len;
        }
    }

    output_buffer[current_output_len] = '\0'; // Null-terminate the output
    *output_len = current_output_len; // Update actual length
    free(ctx); // Free the rendering context
    return 0;
}

// Reads characters from a file descriptor until a terminator character is found
// or max_len is reached. Handles newline as terminator if terminator is 0.
int read_until(int fd, char *buffer, size_t max_len, char terminator) {
    size_t bytes_read = 0;
    char c;
    ssize_t res;

    if (max_len == 0) return 0;

    // Read until max_len-1 to leave space for the null terminator
    while (bytes_read < max_len - 1) {
        res = read(fd, &c, 1);
        if (res <= 0) {
            if (bytes_read == 0 && res == 0) { // EOF on the very first read
                buffer[0] = '\0';
                return 0;
            }
            if (res < 0) { // Error reading
                buffer[bytes_read] = '\0';
                return -1;
            }
            // EOF after reading some bytes
            break;
        }
        // Check for terminator: either specified char or newline if terminator is 0
        if ((terminator == 0 && c == '\n') || (terminator != 0 && c == terminator)) {
            break;
        }
        buffer[bytes_read++] = c;
    }
    buffer[bytes_read] = '\0'; // Null-terminate the buffer
    return (int)bytes_read;
}

// --- Main Program Functions ---

// Function: print_menu
void print_menu(void) {
  fdprintf(1,"1. Define variables\n");
  fdprintf(1,"2. View variables\n");
  fdprintf(1,"3. Submit templated text\n");
  fdprintf(1,"4. Render text\n");
  fdprintf(1,"5. Quit\n");
  fdprintf(1,G_DAT_NEWLINE); // Prints a newline
}

// Function: define_vars
void define_vars(dict_t *vars_dict) {
  char var_name_buf[4096];
  char var_value_buf[4096];
  
  fdprintf(1,"-- Empty variable name will exit this menu\n");
  fdprintf(1,"-- Empty value will undefine the variable (if exists)\n");
  do {
    fdprintf(1,"var name: ");
    if (read_until(0, var_name_buf, sizeof(var_name_buf), '\n') < 0 || var_name_buf[0] == '\0') {
      return; // Exit if read error or empty name
    }
    
    fdprintf(1,"-- Available types\n");
    fdprintf(1,"    0 - String (default)\n");
    fdprintf(1,"    1 - Integer\n");
    fdprintf(1,"    2 - Float\n");
    fdprintf(1,"    3 - Bool [true/false]\n");
    fdprintf(1,"var type: ");
    
    sadface_vartype_t var_type = VAR_TYPE_STRING; // Default to String
    if (read_until(0, var_value_buf, sizeof(var_value_buf), '\n') >= 0) {
      unsigned long type_choice = strtoul(var_value_buf, NULL, 10);
      if (type_choice == VAR_TYPE_INTEGER) {
        var_type = VAR_TYPE_INTEGER;
      } else if (type_choice == VAR_TYPE_FLOAT) {
        var_type = VAR_TYPE_FLOAT;
      } else if (type_choice == VAR_TYPE_BOOL) {
        var_type = VAR_TYPE_BOOL;
      } // Else, var_type remains VAR_TYPE_STRING (0)
    } // If read_until returns < 0, var_type remains VAR_TYPE_STRING
    
    fdprintf(1,"var value: ");
    if (read_until(0, var_value_buf, sizeof(var_value_buf), '\n') < 0) {
      return; // Exit on read error
    }
    
    if (var_value_buf[0] == '\0') { // Empty value: undefine variable
      sadface_var_t *removed_var = dict_remove(vars_dict, var_name_buf);
      if (removed_var != NULL) {
        // Free dynamically allocated data within the variable
        if (removed_var->type == VAR_TYPE_STRING && removed_var->value_data != 0) {
          free((void *)removed_var->value_data);
        }
        free(removed_var); // Free the sadface_var_t structure itself
      }
    } else { // Define or update variable
      sadface_var_t *new_var = malloc(sizeof(sadface_var_t));
      if (new_var == NULL) {
          dprintf(2, "Error: malloc failed for new_var.\n");
          continue; // Continue to next loop iteration
      }
      
      new_var->type = var_type;
      switch (var_type) {
        case VAR_TYPE_STRING: {
          new_var->value_data = (uintptr_t)strdup(var_value_buf);
          if (new_var->value_data == 0) { // Check for strdup failure
              dprintf(2, "Error: strdup failed for string variable.\n");
              free(new_var);
              continue; // Continue to next loop iteration
          }
          break;
        }
        case VAR_TYPE_INTEGER: {
          new_var->value_data = strtoul(var_value_buf, NULL, 10);
          break;
        }
        case VAR_TYPE_FLOAT: {
          float f_val = strtof(var_value_buf, NULL);
          // Store raw bits of float into uintptr_t using a union for type-punning
          union { float f; uintptr_t u; } converter;
          converter.f = f_val;
          new_var->value_data = converter.u;
          break;
        }
        case VAR_TYPE_BOOL: {
          // Store 0 for "false", 1 for anything else (interpreted as true)
          new_var->value_data = (uintptr_t)(strcmp(var_value_buf, "false") == 0 ? 0 : 1);
          break;
        }
      }
      dict_insert(vars_dict, var_name_buf, new_var);
    }
  } while( true );
}

// Function: view_vars
void view_vars(dict_t *vars_dict) {
  fdprintf(1,"-- Current variables:\n");
  int var_count = 0;
  for (int i = 0; i < DICT_SIZE; i++) {
    for (dict_node_t *node = vars_dict->buckets[i]; node != NULL; node = node->next) {
      char *var_str_value = sadface_var2str(node->value);
      if (var_str_value != NULL) {
        fdprintf(1," > %s : %s\n", node->key, var_str_value);
        free(var_str_value); // Free the string returned by sadface_var2str
        var_count++;
      }
    }
  }
  if (var_count == 0) {
    fdprintf(1,"-- None\n");
  } else {
    fdprintf(1,"-- Total %d variable(s).\n", var_count);
  }
}

// Function: submit_text
void submit_text(char *template_buffer, size_t buffer_size) {
  memset(template_buffer, 0, buffer_size);
  fdprintf(1,"-- Submit a null-terminated string\n");
  if (read_until(0, template_buffer, buffer_size, '\n') < 0) { // Read until newline
    fdprintf(1,"error.\n");
  }
}

// Function: render_text
void render_text(const char *template_text, dict_t *vars_dict) {
  void *render_ctx = NULL;
  
  // sadface_init returns the context pointer, or NULL on error.
  if (sadface_init(&render_ctx, 0, template_text, vars_dict) == NULL) {
    fdprintf(1,"error during sadface_init.\n");
  } else {
    size_t output_buffer_len = 0x1000;
    char *output_buffer = calloc(1, output_buffer_len);
    if (output_buffer != NULL) {
      if (sadface_render(render_ctx, output_buffer, &output_buffer_len) >= 0) {
        fdprintf(1,"-- Render start.\n");
        fdprintf(1,G_DAT_RENDER_FORMAT,output_buffer); // G_DAT_RENDER_FORMAT is "%s\n"
        fdprintf(1,"-- Render complete (%zu bytes).\n",output_buffer_len);
      } else {
        fdprintf(1,"error during sadface_render.\n");
      }
      free(output_buffer); // Free the output buffer
    } else {
      fdprintf(1,"error: calloc failed for render output buffer.\n");
      // The render_ctx would need to be freed here if sadface_render wasn't called.
      // Assuming sadface_render frees ctx if it's called with a valid ctx.
    }
  }
}

// Function: main
int main(void) {
  char template_text_buffer[4096];
  char menu_choice_buffer[4096];
  
  dict_t *variables_dict = dict_new(); // Initialize the dictionary
  
  fdprintf(1,G_DAT_BANNER_FORMAT,G_BANNER_TEXT); // Print the banner
  
  while( true ) {
    print_menu();
    fdprintf(1,G_DAT_PROMPT); // Print the prompt
    
    if (read_until(0, menu_choice_buffer, sizeof(menu_choice_buffer), '\n') < 0) {
      break; // Exit loop on read error
    }
    
    long choice = strtol(menu_choice_buffer, NULL, 10);
    
    switch(choice) {
    default:
      fdprintf(1,"Invalid menu. Try again.\n");
      break;
    case 1:
      define_vars(variables_dict);
      break;
    case 2:
      view_vars(variables_dict);
      break;
    case 3:
      submit_text(template_text_buffer, sizeof(template_text_buffer));
      break;
    case 4:
      render_text(template_text_buffer, variables_dict);
      break;
    case 5:
      fdprintf(1,"# Bye.\n\n");
      // TODO: Implement proper cleanup for all dictionary resources before exiting.
      // For now, exit directly as in original snippet.
      exit(0);
    }
    fdprintf(1,G_DAT_NEWLINE); // Print a newline after each operation
  }
  
  // If the loop breaks due to a read error in main menu
  return 1; // Indicate an error exit
}