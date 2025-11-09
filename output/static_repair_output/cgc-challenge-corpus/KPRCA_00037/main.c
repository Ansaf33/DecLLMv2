#define _GNU_SOURCE // Required for dprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h> // For uint32_t, uint8_t, uint16_t, uintptr_t

// --- Type Definitions based on original snippet's memory access and sizes ---
// Assuming 32-bit like memory layout for struct members, especially pointers.
// Pointers to allocated strings are stored as uint32_t within these structs.

// ElfFileHeader: inferred from local_78 (48 bytes) in main
typedef struct {
    uint8_t magic[4];   // 0x0
    uint32_t entry;     // 0x4
    uint32_t phoff;     // 0x8
    uint32_t shoff;     // 0xc (offset to section header table)
    uint32_t flags;     // 0x10
    uint16_t ehsize;    // 0x14
    uint16_t phentsize; // 0x16
    uint16_t phnum;     // 0x18
    uint16_t shentsize; // 0x1a
    uint16_t shnum;     // 0x1c (number of sections)
    uint16_t shstrndx;  // 0x1e (index of section name string table)
    uint8_t padding[48 - 0x20]; // Pad to 48 bytes
} ElfFileHeader;

// ElfSection: inferred from malloc(0x2c) and offsets in print_sections/cmp_section
typedef struct {
    uint32_t sh_name_offset; // 0x0: Offset to name in string table
    uint32_t sh_type;        // 0x4
    uint32_t sh_flags;       // 0x8
    uint32_t sh_addr;        // 0xc
    uint32_t sh_offset;      // 0x10: Section file offset
    uint32_t sh_size;        // 0x14
    uint32_t sh_link;        // 0x18: Link to another section (e.g., symbol string table)
    uint32_t sh_info;        // 0x1c
    uint32_t sh_addralign;   // 0x20
    uint32_t sh_entsize;     // 0x24
    uint32_t name_ptr_addr;  // 0x28: Pointer to allocated name string (stored as uint32_t)
} ElfSection; // Total size 0x2c (44 bytes)

// ElfSymbol: inferred from malloc(0x14) and offsets in print_symbols/cmp_symbol
typedef struct {
    uint32_t st_name_offset; // 0x0: Offset to name in string table
    uint32_t st_value;       // 0x4
    uint32_t st_size;        // 0x8
    uint8_t st_info;         // 0xc: Contains type (lower 4 bits) and bind (upper 4 bits)
    uint8_t st_other;        // 0xd
    uint16_t st_shndx;       // 0xe
    uint32_t name_ptr_addr;  // 0x10: Pointer to allocated name string (stored as uint32_t)
} ElfSymbol; // Total size 0x14 (20 bytes)

// Linked list node for sections
typedef struct SectionListEntry {
    ElfSection *section_data;
    struct SectionListEntry *next;
} SectionListEntry;

// Linked list node for symbols
typedef struct SymbolListEntry {
    ElfSymbol *symbol_data;
    struct SymbolListEntry *next;
} SymbolListEntry;

// --- Global String Literals (DAT_000XXXXX) ---
// Inferred from usage in fdprintf calls.
// DAT_0001414c is a single space.
// DAT_000141a6 is " %s".
// DAT_000141bc is "\n".
// DAT_00014212 is " %s".
// DAT_00014216 is " %s\n".

// --- Helper Functions (Stubs for external CGC functions) ---

// Reads exactly 'count' bytes from file descriptor 'fd' into 'buf'.
ssize_t read_n(int fd, void *buf, size_t count) {
    size_t total_read = 0;
    ssize_t bytes_read;
    while (total_read < count) {
        bytes_read = read(fd, (char *)buf + total_read, count - total_read);
        if (bytes_read <= 0) { // Error or EOF
            return bytes_read;
        }
        total_read += bytes_read;
    }
    return total_read;
}

// Stubs for CGC-specific functions
// These mock the behavior implied by the original code.
int cgcf_parse_file_header(const void *file_buffer, size_t file_size, ElfFileHeader *header) {
    if (file_size < sizeof(ElfFileHeader)) return -1;
    memcpy(header, file_buffer, sizeof(ElfFileHeader));
    return 0; // Success
}

int cgcf_is_valid(const ElfFileHeader *header) {
    return (header->magic[0] == 0x7f && header->magic[1] == 'C' && header->magic[2] == 'G' && header->magic[3] == 'C');
}

// This function is assumed to read a specific section header entry by index
// and populate the ElfSection struct. It needs the global file header to
// locate the section header table.
int cgcf_get_section_header_entry(const void *file_buffer, size_t file_size, uint16_t section_idx, ElfSection *section_entry) {
    ElfFileHeader file_header_tmp;
    if (cgcf_parse_file_header(file_buffer, file_size, &file_header_tmp) != 0) {
        return -1; // Failed to get file header info
    }

    if (section_idx >= file_header_tmp.shnum) {
        return -1; // Section index out of bounds
    }

    // Calculate offset to the target section header entry
    uintptr_t section_header_offset = (uintptr_t)file_header_tmp.shoff + (uintptr_t)section_idx * file_header_tmp.shentsize;
    
    // Check if the section header entry is within file bounds
    if (section_header_offset + file_header_tmp.shentsize > file_size) {
        return -1;
    }

    // Copy raw section header data. Assuming ElfSection matches the layout of actual data on file.
    // However, ElfSection has an extra `name_ptr_addr` field.
    // We only copy the raw data (up to sh_entsize) and leave `name_ptr_addr` untouched for now.
    // The original ElfSection struct must be able to hold the raw data.
    // The `sh_entsize` field from the file header is the actual size of a section header entry in the file.
    // Our ElfSection struct is 0x2c bytes. If sh_entsize is smaller (e.g. 0x28 for Elf32_Shdr),
    // we should only copy sh_entsize bytes.
    // Let's assume sh_entsize is at least 0x28 and the relevant fields are within that.
    
    // Temporary struct to match the raw on-disk section header (e.g., Elf32_Shdr which is 0x28 bytes)
    typedef struct {
        uint32_t sh_name;      /* Section name (string tbl index) */
        uint32_t sh_type;      /* Section type */
        uint32_t sh_flags;     /* Section flags */
        uint32_t sh_addr;      /* Section virtual addr at execution */
        uint32_t sh_offset;    /* Section file offset */
        uint32_t sh_size;      /* Section size in bytes */
        uint32_t sh_link;      /* Link to another section */
        uint32_t sh_info;      /* Additional section information */
        uint32_t sh_addralign; /* Section alignment */
        uint32_t sh_entsize;   /* Entry size if section holds table */
    } RawSectionHeaderEntry; // This is 0x28 bytes

    RawSectionHeaderEntry raw_entry;
    if (file_header_tmp.shentsize < sizeof(RawSectionHeaderEntry)) {
        // If the actual entry size is smaller than our assumed raw struct, copy less.
        // This scenario is less likely for CGC, but robust.
        memcpy(&raw_entry, (char *)file_buffer + section_header_offset, file_header_tmp.shentsize);
    } else {
        memcpy(&raw_entry, (char *)file_buffer + section_header_offset, sizeof(RawSectionHeaderEntry));
    }

    // Map raw data to our ElfSection struct
    section_entry->sh_name_offset = raw_entry.sh_name;
    section_entry->sh_type = raw_entry.sh_type;
    section_entry->sh_flags = raw_entry.sh_flags; // Not used in snippet, but good to copy
    section_entry->sh_addr = raw_entry.sh_addr;
    section_entry->sh_offset = raw_entry.sh_offset;
    section_entry->sh_size = raw_entry.sh_size;
    section_entry->sh_link = raw_entry.sh_link;
    section_entry->sh_info = raw_entry.sh_info; // Not used in snippet
    section_entry->sh_addralign = raw_entry.sh_addralign; // Not used in snippet
    section_entry->sh_entsize = raw_entry.sh_entsize; // Not used in snippet

    return 0; // Success
}

const char *cgcf_section_type2str(uint32_t type) {
    switch (type) {
        case 0: return "NULL";
        case 1: return "PROGBITS";
        case 2: return "SYMTAB";
        case 3: return "STRTAB";
        case 4: return "RELA";
        case 5: return "HASH";
        case 6: return "DYNAMIC";
        case 7: return "NOTE";
        case 8: return "NOBITS";
        case 9: return "REL";
        case 10: return "SHLIB";
        case 11: return "DYNSYM";
        default: return "UNKNOWN";
    }
}

const char *cgcf_symbol_type2str(uint8_t type) {
    switch (type) {
        case 0: return "NOTYPE";
        case 1: return "OBJECT";
        case 2: return "FUNC";
        case 3: return "SECTION";
        case 4: return "FILE";
        case 5: return "COMMON";
        case 6: return "TLS";
        default: return "UNKNOWN";
    }
}

const char *cgcf_symbol_bind2str(uint8_t bind) {
    switch (bind) {
        case 0: return "LOCAL";
        case 1: return "GLOBAL";
        case 2: return "WEAK";
        default: return "UNKNOWN";
    }
}

// Generic linked list node for sl_insert/sl_destroy
typedef struct GenericNode {
    void *data;
    struct GenericNode *next;
} GenericNode;

// Inserts data into a sorted linked list.
void *sl_insert(void *head, void *data, int (*cmp_func)(const void *, const void *)) {
    GenericNode *new_node = (GenericNode *)malloc(sizeof(GenericNode));
    if (new_node == NULL) {
        // In a real application, handle allocation failure more robustly.
        return head;
    }
    new_node->data = data;
    new_node->next = NULL;

    if (head == NULL || cmp_func(data, ((GenericNode *)head)->data) < 0) {
        new_node->next = (GenericNode *)head;
        return new_node;
    }

    GenericNode *current = (GenericNode *)head;
    while (current->next != NULL && cmp_func(data, current->next->data) >= 0) {
        current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
    return head;
}

// Destroys a linked list, freeing node data using free_func if provided.
void sl_destroy(void *head, void (*free_func)(void *)) {
    GenericNode *current = (GenericNode *)head;
    while (current != NULL) {
        GenericNode *next = current->next;
        if (free_func != NULL && current->data != NULL) {
            free_func(current->data);
        }
        free(current);
        current = next;
    }
}

// Helper to check if a derived pointer is within the bounds of the original buffer.
static inline bool is_valid_ptr(const void *base, size_t size, const void *ptr) {
    uintptr_t base_addr = (uintptr_t)base;
    uintptr_t ptr_addr = (uintptr_t)ptr;
    return (ptr_addr >= base_addr) && (ptr_addr < base_addr + size);
}

// --- Original Functions ---

// Function: max_section_name_len
size_t max_section_name_len(SectionListEntry *head) {
  size_t max_len = 0;
  for (SectionListEntry *current = head; current != NULL; current = current->next) {
    if (current->section_data != NULL) {
      // Cast uint32_t address to char*
      size_t current_len = strlen((char *)(uintptr_t)current->section_data->name_ptr_addr);
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
    dprintf(1, " "); // DAT_0001414c
  }
}

// Function: print_sections
void print_sections(SectionListEntry *head, uint32_t section_count) {
  dprintf(1, "%u section header(s):\n", section_count);
  size_t max_name_len = max_section_name_len(head);
  dprintf(1, "  [No.] Name");
  print_ws(max_name_len - 4); // "Name" is 4 chars
  dprintf(1, "  Type           Addr     Off    Size\n");

  uint32_t section_idx = 0;
  for (SectionListEntry *current = head; current != NULL; current = current->next) {
    if (current->section_data != NULL) {
      char *name = (char *)(uintptr_t)current->section_data->name_ptr_addr;
      dprintf(1, "  [%3u] %s", section_idx, name);
      print_ws(max_name_len - strlen(name));

      const char *type_str = cgcf_section_type2str(current->section_data->sh_type);
      dprintf(1, " %s", type_str); // DAT_000141a6
      print_ws(14 - strlen(type_str)); // Pad to 14 characters for Type column

      dprintf(1, " %08x %06x %06x\n",
              current->section_data->sh_addr,
              current->section_data->sh_offset,
              current->section_data->sh_size);
      section_idx++;
    }
  }
  dprintf(1, "\n"); // DAT_000141bc
}

// Function: print_symbols
void print_symbols(SymbolListEntry *head, uint32_t symbol_count) {
  dprintf(1, "%u symbol(s):\n", symbol_count);
  dprintf(1, "  [No.] Value       Size Type    Bind    Name\n");

  uint32_t symbol_idx = 0;
  for (SymbolListEntry *current = head; current != NULL; current = current->next) {
    if (current->symbol_data != NULL) {
      uint8_t type_val = current->symbol_data->st_info & 0xf;
      uint8_t bind_val = current->symbol_data->st_info >> 4;
      const char *type_str = cgcf_symbol_type2str(type_val);
      const char *bind_str = cgcf_symbol_bind2str(bind_val);

      dprintf(1, "  [%3u] 0x%08x %5u",
              symbol_idx,
              current->symbol_data->st_value,
              current->symbol_data->st_size);
      
      dprintf(1, " %s", type_str); // DAT_00014212
      print_ws(7 - strlen(type_str)); // Pad to 7 characters for Type column

      dprintf(1, " %s", bind_str); // DAT_00014212
      print_ws(7 - strlen(bind_str)); // Pad to 7 characters for Bind column

      dprintf(1, " %s\n", (char *)(uintptr_t)current->symbol_data->name_ptr_addr); // DAT_00014216
      symbol_idx++;
    }
  }
  dprintf(1, "\n"); // DAT_000141bc
}

// Function: cmp_section
int cmp_section(const void *a, const void *b) {
  const ElfSection *sec1 = (const ElfSection *)a;
  const ElfSection *sec2 = (const ElfSection *)b;

  if (sec1 == NULL || sec2 == NULL) {
    return 0;
  }
  // Compare by sh_offset (offset 0x10)
  return (int)(sec1->sh_offset - sec2->sh_offset);
}

// Function: cmp_symbol
int cmp_symbol(const void *a, const void *b) {
  const ElfSymbol *sym1 = (const ElfSymbol *)a;
  const ElfSymbol *sym2 = (const ElfSymbol *)b;

  if (sym1 == NULL || sym2 == NULL) {
    return 0;
  }
  // Compare by name (offset 0x10 is name_ptr_addr)
  return strcmp((char *)(uintptr_t)sym1->name_ptr_addr, (char *)(uintptr_t)sym2->name_ptr_addr);
}

// Function: free_section
void free_section(void *ptr) {
  if (ptr != NULL) {
    ElfSection *section = (ElfSection *)ptr;
    if (section->name_ptr_addr != 0) {
      free((void *)(uintptr_t)section->name_ptr_addr);
    }
    free(ptr);
  }
}

// Function: free_symbol
void free_symbol(void *ptr) {
  if (ptr != NULL) {
    ElfSymbol *symbol = (ElfSymbol *)ptr;
    if (symbol->name_ptr_addr != 0) {
      free((void *)(uintptr_t)symbol->name_ptr_addr);
    }
    free(ptr);
  }
}

// Function: main
int main(void) {
  ElfFileHeader file_header;
  uint32_t file_size = 0;
  void *file_buffer = NULL;
  SectionListEntry *sections_head = NULL;
  SymbolListEntry *symbols_head = NULL;
  bool success = false;

  do { // Use do-while(0) for structured cleanup instead of goto
    if (read_n(0, &file_size, sizeof(file_size)) != sizeof(file_size)) {
      dprintf(1, "Failed to read file size.\n");
      break;
    }

    if (file_size >= 0x200001) { // 2MB + 1
      dprintf(1, "Too big.\n");
      break;
    }

    file_buffer = malloc(file_size);
    if (file_buffer == NULL) {
      dprintf(1, "Failed to allocate file buffer.\n");
      break;
    }

    if (read_n(0, file_buffer, file_size) != file_size) {
      dprintf(1, "Failed to read file content.\n");
      break;
    }

    if (cgcf_parse_file_header(file_buffer, file_size, &file_header) != 0) {
      dprintf(1, "Invalid CGC file header.\n");
      break;
    }

    if (cgcf_is_valid(&file_header) == 0) {
      dprintf(1, "Invalid CGC magic.\n");
      break;
    }
    dprintf(1, "Valid CGC executable format found [%u bytes]\n\n", file_size);

    // Section header string table offset and count come from the ElfFileHeader
    uint32_t shstrtab_offset = file_header.shoff + file_header.shstrndx * file_header.shentsize;
    uint32_t shstrtab_base_addr = (uint32_t)(uintptr_t)file_buffer + shstrtab_offset;

    if (shstrtab_offset >= file_size) {
        dprintf(1, "Section string table offset out of file bounds.\n");
        break;
    }

    bool section_parsing_error = false;
    for (uint16_t i = 0; i < file_header.shnum; ++i) {
      ElfSection *section = (ElfSection *)malloc(sizeof(ElfSection));
      if (section == NULL) {
        dprintf(1, "Failed to allocate ElfSection for section %u.\n", i);
        section_parsing_error = true;
        break;
      }
      
      // Initialize name_ptr_addr to 0 (NULL)
      section->name_ptr_addr = 0;

      if (cgcf_get_section_header_entry(file_buffer, file_size, i, section) != 0) {
        dprintf(1, "Failed to parse section header %u.\n", i);
        free(section); // Free the current section struct
        section_parsing_error = true;
        break;
      }

      char *section_name_src = (char *)(uintptr_t)(shstrtab_base_addr + section->sh_name_offset);
      // Check if name offset is valid and pointer is within bounds
      if (section->sh_name_offset != 0 && !is_valid_ptr(file_buffer, file_size, section_name_src)) {
        dprintf(1, "Section name pointer out of bounds for section %u.\n", i);
        free(section);
        section_parsing_error = true;
        break;
      }
      
      size_t name_len = strlen(section_name_src);
      char *allocated_name = (char *)malloc(name_len + 1);
      if (allocated_name == NULL) {
        dprintf(1, "Failed to allocate section name for section %u.\n", i);
        free(section);
        section_parsing_error = true;
        break;
      }
      strcpy(allocated_name, section_name_src);
      section->name_ptr_addr = (uint32_t)(uintptr_t)allocated_name;

      sections_head = (SectionListEntry *)sl_insert(sections_head, section, cmp_section);
    }
    if (section_parsing_error) {
      break; // Exit do-while(0) for cleanup
    }

    print_sections(sections_head, file_header.shnum);

    uint32_t total_symbol_count = 0; // Accumulate symbol count across all symbol tables
    SectionListEntry *current_section_node = sections_head;
    bool symbol_parsing_error = false;
    while (current_section_node != NULL) {
      ElfSection *symtab_section = current_section_node->section_data;
      if (symtab_section != NULL && symtab_section->sh_type == 2) { // SHT_SYMTAB = 2
        // Get info for the string table section linked by the symbol table
        ElfSection symtab_strtab_info;
        if (cgcf_get_section_header_entry(file_buffer, file_size, symtab_section->sh_link, &symtab_strtab_info) != 0) {
          dprintf(1, "Failed to get symbol string table info for section linked by %u.\n", symtab_section->sh_name_offset);
          symbol_parsing_error = true;
          break;
        }

        uint32_t sym_strtab_base_addr = (uint32_t)(uintptr_t)file_buffer + symtab_strtab_info.sh_offset;
        uint32_t symbol_entry_size = symtab_section->sh_entsize; // Use sh_entsize from symtab_section
        if (symbol_entry_size == 0) symbol_entry_size = 0x10; // Default if not set, as per original code's 0x10 constant

        uint32_t current_symtab_symbol_count = symtab_section->sh_size / symbol_entry_size;
        total_symbol_count += current_symtab_symbol_count;

        // Bounds check for symbol table data
        if (symtab_section->sh_offset > file_size || (uintptr_t)symtab_section->sh_offset + symtab_section->sh_size > file_size) {
            dprintf(1, "Symbol table section data out of file bounds.\n");
            symbol_parsing_error = true;
            break;
        }

        for (uint32_t i = 0; i < current_symtab_symbol_count; ++i) {
          ElfSymbol *symbol = (ElfSymbol *)malloc(sizeof(ElfSymbol));
          if (symbol == NULL) {
            dprintf(1, "Failed to allocate ElfSymbol for symbol %u.\n", i);
            symbol_parsing_error = true;
            break;
          }
          // Initialize name_ptr_addr to 0 (NULL)
          symbol->name_ptr_addr = 0;

          void *symbol_src_ptr = (char *)file_buffer + symtab_section->sh_offset + i * symbol_entry_size;
          memmove(symbol, symbol_src_ptr, symbol_entry_size); // Copy raw symbol data

          char *symbol_name_src = (char *)(uintptr_t)(sym_strtab_base_addr + symbol->st_name_offset);
          // Check bounds for symbol name pointer
          if (symbol->st_name_offset != 0 && !is_valid_ptr(file_buffer, file_size, symbol_name_src)) {
            dprintf(1, "Symbol name pointer out of bounds for symbol %u.\n", i);
            free(symbol);
            symbol_parsing_error = true;
            break;
          }

          size_t name_len = strlen(symbol_name_src);
          char *allocated_name = (char *)malloc(name_len + 1);
          if (allocated_name == NULL) {
            dprintf(1, "Failed to allocate symbol name for symbol %u.\n", i);
            free(symbol);
            symbol_parsing_error = true;
            break;
          }
          strcpy(allocated_name, symbol_name_src);
          symbol->name_ptr_addr = (uint32_t)(uintptr_t)allocated_name;

          symbols_head = (SymbolListEntry *)sl_insert(symbols_head, symbol, cmp_symbol);
        }
      }
      if (symbol_parsing_error) {
        break; // Exit while loop for cleanup
      }
      current_section_node = current_section_node->next;
    }
    if (symbol_parsing_error) {
      dprintf(1, "Error during symbol parsing.\n");
      break; // Exit do-while(0) for cleanup
    }

    print_symbols(symbols_head, total_symbol_count);
    dprintf(1, "DONE\n");
    success = true;

  } while (0); // End of do-while(0) block for structured error handling

  // Cleanup block
  if (file_buffer != NULL) {
    free(file_buffer);
  }
  if (sections_head != NULL) {
    sl_destroy(sections_head, free_section);
  }
  if (symbols_head != NULL) {
    sl_destroy(symbols_head, free_symbol);
  }

  return success ? 0 : 0xFFFFFFFF;
}