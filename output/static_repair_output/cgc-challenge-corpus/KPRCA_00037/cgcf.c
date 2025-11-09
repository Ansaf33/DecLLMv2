#include <stddef.h>  // For NULL
#include <string.h>  // For memcmp, memmove
#include <stdbool.h> // For bool
#include <stdint.h>  // For uint16_t, uint32_t

// Forward declaration for cgcf_parse_file_header as it's used by cgcf_parse_section_header
int cgcf_parse_file_header(const void *param_1, unsigned int param_2, void *param_3);

// Define minimal ELF structures for 32-bit, based on sizes observed (0x34 and 0x28)
// These definitions assume standard byte alignment.
typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr; // Size 0x34 (52 bytes)

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} Elf32_Shdr; // Size 0x28 (40 bytes)

// Global constant for ELF magic number (0x7f 'E' 'L' 'F')
static const unsigned char ELF_MAGIC[4] = {0x7f, 'E', 'L', 'F'};

// Function: cgcf_section_type2str
char * cgcf_section_type2str(unsigned int param_1) {
  switch(param_1) {
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
  case 10: return "-reserved-";
  case 0xb: return "DYNSYM";
  default: return "UNKNOWN"; // Covers cases not explicitly listed before it
  case 0xe: return "INIT_ARRAY";
  case 0xf: return "FINI_ARRAY";
  case 0x10: return "PREINIT_ARRAY";
  case 0x11: return "GROUP";
  case 0x12: return "SYMTAB_SHNDX";
  case 0x13: return "NUM";
  }
}

// Function: cgcf_symbol_bind2str
char * cgcf_symbol_bind2str(unsigned char param_1) {
  if (param_1 == 0) return "LOCAL";
  if (param_1 == 1) return "GLOBAL";
  if (param_1 == 2) return "WEAK";
  if (param_1 == 3) return "NUM";
  return "UNKNOWN";
}

// Function: cgcf_symbol_type2str
char * cgcf_symbol_type2str(unsigned char param_1) {
  switch(param_1) {
  case 0: return "NOTYPE";
  case 1: return "OBJECT";
  case 2: return "FUNC";
  case 3: return "SECTION";
  case 4: return "FILE";
  case 5: return "COMMON";
  case 6: return "TLS";
  case 7: return "NUM";
  default: return "UNKNOWN";
  }
}

// Function: cgcf_is_valid
_Bool cgcf_is_valid(const void *param_1) {
  return memcmp(param_1, ELF_MAGIC, 4) == 0;
}

// Function: cgcf_parse_file_header
int cgcf_parse_file_header(const void *param_1, unsigned int param_2, void *param_3) {
  if (param_1 == NULL || param_3 == NULL || param_2 < sizeof(Elf32_Ehdr)) {
    return -1; // Error: NULL pointers or insufficient file size
  }
  memmove(param_3, param_1, sizeof(Elf32_Ehdr));
  return 0; // Success
}

// Function: cgcf_parse_section_header
int cgcf_parse_section_header(const unsigned char *base_addr, unsigned int file_size, unsigned short section_idx, void *out_shdr) {
    if (base_addr == NULL || out_shdr == NULL) {
        return -1; // Error: NULL pointers
    }

    Elf32_Ehdr ehdr; // Buffer to hold the parsed file header
    if (cgcf_parse_file_header(base_addr, file_size, &ehdr) != 0) {
        return -1; // Error parsing file header
    }

    // Validate section index against the number of section headers
    if (section_idx >= ehdr.e_shnum) {
        return -1; // Error: section_idx out of bounds
    }

    // Ensure section header entry size is valid to avoid invalid calculations
    if (ehdr.e_shentsize == 0) {
        return -1; // Error: Invalid section header entry size
    }

    // Calculate the offset to the target section header
    unsigned long target_shdr_offset = (unsigned long)ehdr.e_shoff + (unsigned long)section_idx * ehdr.e_shentsize;

    // Check bounds: ensure the entire section header is within the file
    if (target_shdr_offset + ehdr.e_shentsize > file_size) {
        return -1; // Error: Section header extends beyond file bounds
    }

    // Copy the section header data to the output buffer
    memmove(out_shdr, base_addr + target_shdr_offset, ehdr.e_shentsize);

    return 0; // Success
}