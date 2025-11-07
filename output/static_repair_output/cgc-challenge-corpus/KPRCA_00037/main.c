#include <stdio.h>    // For vdprintf, fprintf (if dprintf not available)
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For strlen, strcpy, strcmp, memmove
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uint32_t, uint16_t, uint8_t, intptr_t
#include <stdbool.h>  // For bool type
#include <stdarg.h>   // For va_list (used by vdprintf)
#include <unistd.h>   // For dprintf, ssize_t (Linux specific)

// --- Custom function prototypes and struct definitions ---

// Structure definitions inferred from offsets in the original code
typedef struct CGCFileHeader {
    uint32_t magic;
    uint32_t entry_point;
    uint32_t flags;
    uint32_t unknown_header_1;
    uint32_t unknown_header_2;
    uint32_t unknown_header_3;
    uint32_t unknown_header_4;
    uint32_t unknown_header_5;
    uint32_t unknown_header_6;
    uint32_t section_header_offset; // Offset 0x24 (assumed from local_94)
    uint16_t section_count;         // Offset 0x28 (assumed from local_48)
    uint16_t unknown_header_7;
} CGCFileHeader; // Total size 0x30 (48 bytes) based on local_78

typedef struct CGCSection {
    uint32_t name_offset_in_strtab; // Offset 0x0 (used as *local_28, points to name in file buffer)
    uint32_t type;                  // Offset 0x4
    uint32_t flags;                 // Offset 0x8
    uint32_t addr;                  // Offset 0xc
    uint32_t offset;                // Offset 0x10 (used in cmp_section)
    uint32_t size;                  // Offset 0x14
    uint32_t link;                  // Offset 0x18 (used for symbol section string table link)
    uint32_t info;                  // Offset 0x1c
    uint32_t align;                 // Offset 0x20
    uint32_t entry_size;            // Offset 0x24
    char *name;                     // Offset 0x28 (dynamically allocated copy of the name string)
} CGCSection; // Total size 0x2c (44 bytes)

typedef struct CGCSymbol {
    uint32_t name_offset_in_strtab; // Offset 0x0 (used as *local_40, points to name in file buffer)
    uint32_t value;                 // Offset 0x4
    uint32_t size;                  // Offset 0x8
    uint8_t info;                   // Offset 0xc (type in lower 4 bits, bind in upper 4 bits)
    uint8_t other;                  // Offset 0xd
    uint16_t shndx;                 // Offset 0xe (section header index)
    char *name;                     // Offset 0x10 (dynamically allocated copy of the name string)
} CGCSymbol; // Total size 0x14 (20 bytes)

// Linked list nodes for sections and symbols
typedef struct CGCSectionListEntry {
    CGCSection *section;
    struct CGCSectionListEntry *next;
} CGCSectionListEntry;

typedef struct CGCSymbolListEntry {
    CGCSymbol *symbol;
    struct CGCSymbolListEntry *next;
} CGCSymbolListEntry;

// --- Placeholder for custom functions and string literals ---

// fdprintf wrapper for dprintf (Linux specific). Fallback to fprintf if dprintf is not available.
// Assuming file descriptor 1 is stdout.
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret;
#ifdef __linux__
    ret = vdprintf(fd, format, args);
#else
    // Fallback for non-Linux systems or if dprintf is not linked
    // This will print to stdout regardless of fd, which might not be desired.
    // A more robust solution would write to a buffer and then to fd.
    if (fd == 1) {
        ret = vfprintf(stdout, format, args);
    } else {
        ret = vfprintf(stderr, format, args); // Or another appropriate stream
    }
#endif
    va_end(args);
    return ret;
}

// Original DAT_00014xxx string literals
static const char *WS_STRING = " ";
static const char *SECTION_FORMAT_TYPE = " %-14s";
static const char *NEWLINE_STRING = "\n";
static const char *SYMBOL_FORMAT_STR = " %-7s";
static const char *SYMBOL_FORMAT_NAME = " %s\n";

// External functions (dummy implementations for compilation)
ssize_t read_n(int fd, void *buf, size_t count) {
    // Dummy: Simulate successful read
    (void)fd; (void)buf;
    // For main's logic, we need to return `count` for success.
    return count;
}

int cgcf_parse_file_header(const void *file_buffer, size_t file_size, CGCFileHeader *header_out) {
    // Dummy: Simulate parsing success
    (void)file_buffer; (void)file_size;
    if (header_out) {
        header_out->magic = 0x43474346; // "CGCF" magic
        header_out->section_count = 2;   // Dummy count
        header_out->section_header_offset = 0x100; // Dummy offset
    }
    return 0; // 0 for success
}

int cgcf_is_valid(const CGCFileHeader *header) {
    // Dummy: Simulate validation
    return (header && header->magic == 0x43474346); // Check for "CGCF" magic
}

// `section_out` is passed by reference, filled by the function
int cgcf_parse_section_header(const void *file_buffer, size_t file_size, uint16_t section_idx, CGCSection *section_out) {
    // Dummy: Simulate parsing success
    (void)file_buffer; (void)file_size; (void)section_idx;
    if (section_out) {
        section_out->name_offset_in_strtab = 0x0; // Dummy name pointer offset
        section_out->type = (section_idx == 0) ? 1 : ((section_idx == 1) ? 2 : 1); // Dummy type for SYMTAB
        section_out->addr = 0x1000;
        section_out->offset = 0x200 + section_idx * 0x100;
        section_out->size = 0x100;
        section_out->link = 0; // Dummy link
    }
    return 0; // 0 for success
}

const char *cgcf_section_type2str(uint32_t type) {
    switch (type) {
        case 0: return "NULL";
        case 1: return "PROGBITS";
        case 2: return "SYMTAB";
        default: return "UNKNOWN";
    }
}

const char *cgcf_symbol_type2str(uint8_t type) {
    switch (type) {
        case 0: return "NOTYPE";
        case 1: return "OBJECT";
        case 2: return "FUNC";
        default: return "UNKNOWN";
    }
}

const char *cgcf_symbol_bind2str(uint8_t bind) {
    switch (bind) {
        case 0: return "LOCAL";
        case 1: return "GLOBAL";
        default: return "UNKNOWN";
    }
}

// --- Function implementations ---

// Function: cmp_section
int cmp_section(intptr_t param_1, intptr_t param_2) {
  if (param_1 == 0 || param_2 == 0) {
    return 0; // Or some error indicator, 0 usually means equal.
  }
  CGCSection *s1 = (CGCSection *)param_1;
  CGCSection *s2 = (CGCSection *)param_2;
  return (int)(s1->offset - s2->offset); // Cast to int to match original return type
}

// Function: cmp_symbol
int cmp_symbol(intptr_t param_1, intptr_t param_2) {
  if (param_1 == 0 || param_2 == 0) {
    return 0;
  }
  CGCSymbol *sym1 = (CGCSymbol *)param_1;
  CGCSymbol *sym2 = (CGCSymbol *)param_2;
  return strcmp(sym1->name, sym2->name);
}

// Function: free_section
void free_section(void *param_1) {
  CGCSection *section = (CGCSection *)param_1;
  if (section != NULL) {
    free(section->name); // Free the dynamically allocated name string
    free(section);       // Free the section struct itself
  }
}

// Function: free_symbol
void free_symbol(void *param_1) {
  CGCSymbol *symbol = (CGCSymbol *)param_1;
  if (symbol != NULL) {
    free(symbol->name); // Free the dynamically allocated name string
    free(symbol);       // Free the symbol struct itself
  }
}

// Generic comparison function type for sl_insert
typedef int (*sl_cmp_func_t)(intptr_t, intptr_t);
// Generic free function type for sl_destroy
typedef void (*sl_free_func_t)(void *);

// sl_insert implementation for sorted linked list
void *sl_insert(void *list_head, void *new_element_data, sl_cmp_func_t cmp_func) {
    if (cmp_func == cmp_section) {
        CGCSectionListEntry *head = (CGCSectionListEntry *)list_head;
        CGCSection *new_section = (CGCSection *)new_element_data;

        CGCSectionListEntry *new_node = (CGCSectionListEntry *)malloc(sizeof(CGCSectionListEntry));
        if (!new_node) return head; // Allocation failure
        new_node->section = new_section;
        new_node->next = NULL;

        if (!head || cmp_func((intptr_t)new_node->section, (intptr_t)head->section) < 0) {
            new_node->next = head;
            return new_node;
        }

        CGCSectionListEntry *current = head;
        while (current->next && cmp_func((intptr_t)new_node->section, (intptr_t)current->next->section) >= 0) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
        return head;

    } else if (cmp_func == cmp_symbol) {
        CGCSymbolListEntry *head = (CGCSymbolListEntry *)list_head;
        CGCSymbol *new_symbol = (CGCSymbol *)new_element_data;

        CGCSymbolListEntry *new_node = (CGCSymbolListEntry *)malloc(sizeof(CGCSymbolListEntry));
        if (!new_node) return head; // Allocation failure
        new_node->symbol = new_symbol;
        new_node->next = NULL;

        if (!head || cmp_func((intptr_t)new_node->symbol, (intptr_t)head->symbol) < 0) {
            new_node->next = head;
            return new_node;
        }

        CGCSymbolListEntry *current = head;
        while (current->next && cmp_func((intptr_t)new_node->symbol, (intptr_t)current->next->symbol) >= 0) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
        return head;
    }
    return list_head; // Should not reach here with proper usage
}

// sl_destroy implementation for linked list cleanup
void sl_destroy(void *list_head, sl_free_func_t free_func) {
    if (free_func == free_section) {
        CGCSectionListEntry *current = (CGCSectionListEntry *)list_head;
        while (current) {
            CGCSectionListEntry *next = current->next;
            free_func(current->section); // Free the section data
            free(current);               // Free the list node itself
            current = next;
        }
    } else if (free_func == free_symbol) {
        CGCSymbolListEntry *current = (CGCSymbolListEntry *)list_head;
        while (current) {
            CGCSymbolListEntry *next = current->next;
            free_func(current->symbol);  // Free the symbol data
            free(current);               // Free the list node itself
            current = next;
        }
    }
}

// Function: max_section_name_len
size_t max_section_name_len(CGCSectionListEntry *section_list) {
  size_t max_len = 0;
  for (CGCSectionListEntry *current_entry = section_list; current_entry != NULL; current_entry = current_entry->next) {
    CGCSection *section = current_entry->section;
    if (section != NULL && section->name != NULL) {
      size_t current_len = strlen(section->name);
      if (max_len < current_len) {
        max_len = current_len;
      }
    }
  }
  return max_len;
}

// Function: print_ws
void print_ws(int count) {
  for (int i = 0; i < count; ++i) {
    fdprintf(1, WS_STRING);
  }
}

// Function: print_sections
void print_sections(CGCSectionListEntry *section_list, uint32_t section_count) {
  fdprintf(1, "%d section header(s):\n", section_count);
  
  size_t max_name_len = max_section_name_len(section_list);
  fdprintf(1, "  [No.] Name");
  print_ws(max_name_len - 4); // Adjust for "Name" length (4 chars)
  fdprintf(1, "  Type           Addr     Off    Size\n");
  
  int section_idx = 0;
  for (CGCSectionListEntry *current_entry = section_list; current_entry != NULL; current_entry = current_entry->next) {
    CGCSection *section = current_entry->section;
    if (section != NULL) {
      fdprintf(1, "  [%3d] %s", section_idx, section->name);
      size_t name_len = strlen(section->name);
      print_ws(max_name_len - (int)name_len); // Cast to int for print_ws parameter
      
      const char *type_str = cgcf_section_type2str(section->type);
      fdprintf(1, SECTION_FORMAT_TYPE, type_str);
      print_ws(14 - (int)strlen(type_str)); // "Type" column width is 14
      
      fdprintf(1, " %08x %06x %06x\n", section->addr, section->offset, section->size);
      section_idx++;
    }
  }
  fdprintf(1, NEWLINE_STRING);
}

// Function: print_symbols
void print_symbols(CGCSymbolListEntry *symbol_list, uint32_t symbol_count) {
  fdprintf(1, "%d symbol(s):\n", symbol_count);
  fdprintf(1, "  [No.] Value       Size Type    Bind    Name\n");
  
  int symbol_idx = 0;
  for (CGCSymbolListEntry *current_entry = symbol_list; current_entry != NULL; current_entry = current_entry->next) {
    CGCSymbol *symbol = current_entry->symbol;
    if (symbol != NULL) {
      const char *type_str = cgcf_symbol_type2str(symbol->info & 0xf);
      const char *bind_str = cgcf_symbol_bind2str(symbol->info >> 4);
      
      fdprintf(1, "  [%3d] 0x%08x %5d", symbol_idx, symbol->value, symbol->size);
      
      fdprintf(1, SYMBOL_FORMAT_STR, type_str);
      print_ws(7 - (int)strlen(type_str)); // "Type" column width is 7
      
      fdprintf(1, SYMBOL_FORMAT_STR, bind_str);
      print_ws(7 - (int)strlen(bind_str)); // "Bind" column width is 7
      
      fdprintf(1, SYMBOL_FORMAT_NAME, symbol->name);
      symbol_idx++;
    }
  }
  fdprintf(1, NEWLINE_STRING);
}

// Function: main
int main(void) {
  int ret_val = 1; // Default error return value
  
  void *file_buffer = NULL;
  CGCSectionListEntry *section_list = NULL;
  CGCSymbolListEntry *symbol_list = NULL;
  uint32_t section_count = 0;
  uint32_t symbol_count = 0;
  
  bool error_occurred = false;

  uint32_t file_size = 0;
  if (read_n(0, &file_size, sizeof(file_size)) != sizeof(file_size)) {
    fdprintf(1, "ERROR: Failed to read file size.\n");
    error_occurred = true;
  }

  if (!error_occurred && file_size >= 0x200001) { // 0x200001 is 2MB + 1
    fdprintf(1, "Too big.\n");
    error_occurred = true;
  }

  if (!error_occurred) {
    file_buffer = malloc(file_size);
    if (file_buffer == NULL) {
      fdprintf(1, "ERROR: Failed to allocate memory for file buffer.\n");
      error_occurred = true;
    }
  }

  if (!error_occurred && read_n(0, file_buffer, file_size) != file_size) {
    fdprintf(1, "ERROR: Failed to read file content.\n");
    error_occurred = true;
  }

  CGCFileHeader file_header;
  if (!error_occurred && cgcf_parse_file_header(file_buffer, file_size, &file_header) != 0) {
    fdprintf(1, "Invalid CGC file header.\n");
    error_occurred = true;
  }

  if (!error_occurred && cgcf_is_valid(&file_header) == 0) {
    fdprintf(1, "Invalid CGC magic.\n");
    error_occurred = true;
  }

  if (!error_occurred) {
    fdprintf(1, "Valid CGC executable format found [%d bytes]\n\n", file_size);

    section_count = file_header.section_count;
    uint32_t section_names_base_offset = file_header.section_header_offset;

    // Parse sections
    for (uint16_t i = 0; i < section_count; ++i) {
      CGCSection *new_section = (CGCSection *)malloc(sizeof(CGCSection));
      if (new_section == NULL) {
        fdprintf(1, "ERROR: Failed to allocate memory for section.\n");
        error_occurred = true;
        break; // Exit loop on error
      }

      if (cgcf_parse_section_header(file_buffer, file_size, i, new_section) != 0) {
        fdprintf(1, "ERROR: Failed to parse section header %d.\n", i);
        free_section(new_section);
        error_occurred = true;
        break;
      }

      char *name_ptr_in_file_buffer = (char *)file_buffer + section_names_base_offset + new_section->name_offset_in_strtab;

      // Bounds check for section name pointer
      if (new_section->name_offset_in_strtab != 0 &&
          (name_ptr_in_file_buffer < (char*)file_buffer || name_ptr_in_file_buffer >= (char*)file_buffer + file_size)) {
          fdprintf(1, "ERROR: Section name pointer out of bounds for section %d.\n", i);
          free_section(new_section);
          error_occurred = true;
          break;
      }

      size_t name_len = strlen(name_ptr_in_file_buffer);
      new_section->name = (char *)malloc(name_len + 1);
      if (new_section->name == NULL) {
        fdprintf(1, "ERROR: Failed to allocate memory for section name.\n");
        free_section(new_section);
        error_occurred = true;
        break;
      }
      strcpy(new_section->name, name_ptr_in_file_buffer);
      
      section_list = (CGCSectionListEntry *)sl_insert(section_list, new_section, (sl_cmp_func_t)cmp_section);
    }
  }

  if (!error_occurred) {
    print_sections(section_list, section_count);

    // Parse symbols from SYMTAB sections
    CGCSectionListEntry *current_section_entry = section_list;
    while (current_section_entry != NULL && !error_occurred) {
      CGCSection *section = current_section_entry->section;
      if (section != NULL && section->type == 2) { // Type 2 is SYMTAB
        CGCSection string_table_section_header;
        if (cgcf_parse_section_header(file_buffer, file_size, (uint16_t)section->link, &string_table_section_header) != 0) {
          fdprintf(1, "ERROR: Failed to parse string table header (link %u) for symbol section.\n", section->link);
          error_occurred = true;
          break;
        }
        
        uint32_t symbol_names_data_offset = string_table_section_header.offset;
        char *symbol_names_data_ptr = (char*)file_buffer + symbol_names_data_offset;

        uint32_t symtab_data_offset = section->offset;
        uint32_t symtab_data_size = section->size;
        uint32_t symbol_entry_size = 0x10; // Inferred from original code
        uint32_t num_symbols_in_section = symtab_data_size / symbol_entry_size;

        // Bounds check for symbol table data within file_buffer
        if ((symtab_data_offset + num_symbols_in_section * symbol_entry_size > file_size) ||
            ((char*)file_buffer + symtab_data_offset < (char*)file_buffer) ) {
            fdprintf(1, "ERROR: Symbol table data out of bounds for section %u.\n", section->link);
            error_occurred = true;
            break;
        }

        for (uint32_t i = 0; i < num_symbols_in_section; ++i) {
          CGCSymbol *new_symbol = (CGCSymbol *)malloc(sizeof(CGCSymbol));
          if (new_symbol == NULL) {
            fdprintf(1, "ERROR: Failed to allocate memory for symbol.\n");
            error_occurred = true;
            break;
          }

          // Copy symbol data from file_buffer
          memmove(new_symbol, (char *)file_buffer + symtab_data_offset + i * symbol_entry_size, symbol_entry_size);

          char *symbol_name_ptr_in_file_buffer = symbol_names_data_ptr + new_symbol->name_offset_in_strtab;

          // Bounds check for symbol name pointer within file_buffer
          if (new_symbol->name_offset_in_strtab != 0 &&
              (symbol_name_ptr_in_file_buffer < (char*)file_buffer || symbol_name_ptr_in_file_buffer >= (char*)file_buffer + file_size)) {
              fdprintf(1, "ERROR: Symbol name pointer out of bounds for symbol %d.\n", i);
              free(new_symbol); // Only free the struct, name isn't allocated yet.
              error_occurred = true;
              break;
          }

          size_t name_len = strlen(symbol_name_ptr_in_file_buffer);
          new_symbol->name = (char *)malloc(name_len + 1);
          if (new_symbol->name == NULL) {
            fdprintf(1, "ERROR: Failed to allocate memory for symbol name.\n");
            free(new_symbol);
            error_occurred = true;
            break;
          }
          strcpy(new_symbol->name, symbol_name_ptr_in_file_buffer);
          
          symbol_list = (CGCSymbolListEntry *)sl_insert(symbol_list, new_symbol, (sl_cmp_func_t)cmp_symbol);
          symbol_count++;
        }
      }
      current_section_entry = current_section_entry->next;
    }
  }

  if (!error_occurred) {
    print_symbols(symbol_list, symbol_count);
    fdprintf(1, "DONE\n");
    ret_val = 0; // Success
  } else {
    fdprintf(1, "ERROR\n"); // Original code prints "ERROR" on failure
  }
  
  // Cleanup resources
  if (file_buffer != NULL) {
    free(file_buffer);
  }
  if (section_list != NULL) {
    sl_destroy(section_list, (sl_free_func_t)free_section);
  }
  if (symbol_list != NULL) {
    sl_destroy(symbol_list, (sl_free_func_t)free_symbol);
  }
  
  return ret_val;
}