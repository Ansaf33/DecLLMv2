#include <stdio.h>    // For debug_print (printf-like)
#include <stdlib.h>   // For calloc, free
#include <string.h>   // For memcpy, strcmp
#include <stdint.h>   // For uint8_t, uint16_t, uint32_t, uintptr_t
#include <stdarg.h>   // For debug_print variadic arguments

// Type definitions from decompiler output, mapped to standard types
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;

// Dummy declarations for external functions to make the code compilable
void debug_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
void transmit_fixed(uint32_t arg1, uint32_t *arg2) {
    // Placeholder for actual implementation if needed.
    // In original code, arguments were likely passed via stack manipulation.
    // Assuming a direct call with inferred arguments based on context.
    (void)arg1; // Suppress unused parameter warning
    (void)arg2; // Suppress unused parameter warning
}
void _terminate(int exit_code) {
    fprintf(stderr, "Program terminated with code %d\n", exit_code);
    exit(exit_code);
}

// Function: GetFunctionAddress
// param_2 is treated as a SharedLibraryInfo* (uintptr_t)
int GetFunctionAddress(char *func_name, uintptr_t lib_info_addr) {
  if (lib_info_addr != 0) {
    uint16_t num_functions = *(uint16_t *)(lib_info_addr + 2);
    uintptr_t functions_base_ptr = (uintptr_t)*(uint32_t *)(lib_info_addr + 4); // 32-bit stored pointer
    uintptr_t data_base_ptr = (uintptr_t)*(uint32_t *)(lib_info_addr + 0x10); // 32-bit stored pointer

    for (int i = 0; i < num_functions; i++) {
      // Each function entry is 0x46 bytes. Name is at offset 0, address at offset 0x42.
      char *current_func_name = (char *)(functions_base_ptr + (size_t)i * 0x46);
      if (strcmp(func_name, current_func_name) == 0) {
        // Function address is stored relative to data_base_ptr
        uint32_t relative_addr = *(uint32_t *)(functions_base_ptr + (size_t)i * 0x46 + 0x42);
        return (int)(data_base_ptr + relative_addr); // Return absolute address
      }
    }
  }
  return 0; // Function not found or invalid library info
}

// Function: ExecuteFunction
// instruction_stream is treated as a byte stream of instructions
undefined4 ExecuteFunction(undefined *instruction_stream) {
  undefined4 result_accumulator;

  if (instruction_stream == NULL) {
    result_accumulator = 0xffffffff;
  } else {
    result_accumulator = 0;
    uint8_t current_instruction;

    while ((current_instruction = *instruction_stream) != '@') {
      instruction_stream++; // Advance past the instruction byte

      if (current_instruction == 'A') {
        // Original stack manipulation suggests arguments 4 and &result_accumulator
        transmit_fixed(4, &result_accumulator);
      } else if (current_instruction == '9') {
        // Read a 32-bit address, then store result_accumulator at that address
        uint32_t *target_addr = (uint32_t *)(uintptr_t)*(uint32_t*)instruction_stream;
        instruction_stream += sizeof(uint32_t);
        *target_addr = result_accumulator;
      } else if (current_instruction == '7') {
        // Read a 32-bit address, then load value from that address into result_accumulator
        uint32_t *source_addr = (uint32_t *)(uintptr_t)*(uint32_t*)instruction_stream;
        instruction_stream += sizeof(uint32_t);
        result_accumulator = *source_addr;
      } else if (current_instruction == '8') {
        // Read a 32-bit address, then read another 32-bit value, store value at address
        uint32_t *target_addr = (uint32_t *)(uintptr_t)*(uint32_t*)instruction_stream;
        instruction_stream += sizeof(uint32_t);
        uint32_t value_to_store = *(uint32_t*)instruction_stream;
        instruction_stream += sizeof(uint32_t);
        *target_addr = value_to_store;
      } else { // Invalid instruction
        debug_print("Invalid Instruction: 0x%x\n", current_instruction); // Include instruction byte
        _terminate(-2); // -2 for 0xfffffffe
      }
    }
  }
  return result_accumulator;
}

// Function: FreeLibrary
void FreeLibrary(void *lib_info_ptr) {
  if (lib_info_ptr != NULL) {
    uintptr_t lib_info_addr = (uintptr_t)lib_info_ptr;

    // Free the allocated blocks pointed to by the library info structure
    void *data_block = (void *)(uintptr_t)*(uint32_t *)(lib_info_addr + 0x10);
    if (data_block) free(data_block);

    void *resources_data = (void *)(uintptr_t)*(uint32_t *)(lib_info_addr + 0xc);
    if (resources_data) free(resources_data);

    void *functions_data = (void *)(uintptr_t)*(uint32_t *)(lib_info_addr + 4);
    if (functions_data) free(functions_data);

    // Finally, free the SharedLibraryInfo structure itself
    free(lib_info_ptr);
  }
}

// Function: LookupResource
// lib_info_addr is treated as a SharedLibraryInfo* (uintptr_t)
int LookupResource(uintptr_t lib_info_addr, uint16_t resource_index) {
  if (lib_info_addr == 0) {
    return 0;
  } else {
    uint16_t num_resources = *(uint16_t *)(lib_info_addr + 8);
    if (resource_index < num_resources) {
      uintptr_t resources_base_ptr = (uintptr_t)*(uint32_t *)(lib_info_addr + 0xc); // 32-bit stored pointer
      // Resource data starts at offset 2 within each 0x42-byte entry
      return (int)(resources_base_ptr + (size_t)resource_index * 0x42 + 2);
    } else {
      debug_print("Invalid Resource index %u\n", resource_index);
      return 0;
    }
  }
}

// Function: LoadSharedLibrary
// base_addr is treated as a base address (uintptr_t)
undefined * LoadSharedLibrary(uintptr_t base_addr) {
  undefined *lib_info = NULL; // Initialize to NULL

  if (base_addr == 0) {
    return NULL;
  }

  // Allocate the main SharedLibraryInfo structure (20 bytes based on usage)
  lib_info = (undefined *)calloc(1, 20); 
  if (lib_info == NULL) {
      debug_print("Failed to allocate SharedLibraryInfo\n");
      return NULL;
  }

  // header_ptr_value is the 32-bit pointer stored at (base_addr + 0x5c)
  uint32_t header_ptr_value = *(uint32_t *)(base_addr + 0x5c);
  uintptr_t header_base_addr = (uintptr_t)header_ptr_value; // Treat header as a base address for pointer arithmetic

  // Perform checks on the header
  if (*(uint32_t *)header_base_addr == *(uint32_t *)(base_addr + 0x44)) { // Compare header size
    if (*(uint32_t *)(header_base_addr + 4) == 0xC67CC76C) { // Magic number -0x39833894 (0xC67CC76C unsigned)
      uint8_t header_type = *(uint8_t *)(header_base_addr + 2);
      if ((header_type == 0xE1) || (header_type == 0xE2)) { // -0x1f (0xE1) or -0x1e (0xE2)
        uint16_t num_functions = *(uint16_t *)(header_base_addr + 9);
        if (num_functions == 0) {
          debug_print("Invalid file header: functions\n");
          free(lib_info); return NULL;
        } else {
          uint16_t func_table_offset = *(uint16_t *)(header_base_addr + 0xb);
          if (func_table_offset == 0x15) { // 21
            uint16_t num_resources = *(uint16_t *)(header_base_addr + 0xd);
            uint16_t resource_offset = *(uint16_t *)(header_base_addr + 0xf);
            uint16_t functions_size = *(uint16_t *)(header_base_addr + 0x11);
            uint16_t func_ptr_offset = *(uint16_t *)(header_base_addr + 0x13);

            // Extensive size/offset checks
            if ((*(uint32_t *)(base_addr + 0x44) + (uint32_t)num_functions * -0x46) - 0x15 < (uint32_t)num_resources * 0x42) {
              debug_print("Invalid file header: resource size\n");
              free(lib_info); return NULL;
            } else if ((*(uint32_t *)(base_addr + 0x44) + (uint32_t)num_resources * -0x42) < (uint32_t)resource_offset) {
              debug_print("Invalid file header: resource offset\n");
              free(lib_info); return NULL;
            } else if (((*(uint32_t *)(base_addr + 0x44) + (uint32_t)num_functions * -0x46 + (uint32_t)num_resources * -0x42) - 0x15) < (uint32_t)functions_size) {
              debug_print("Invalid file header: functions size\n");
              free(lib_info); return NULL;
            } else if ((*(uint32_t *)(base_addr + 0x44) - (uint32_t)functions_size) < (uint32_t)func_ptr_offset) {
              debug_print("Invalid file header: functions size\n"); // Duplicate message from original
              free(lib_info); return NULL;
            } else {
              // All checks passed, proceed with allocation and copying
              *(uint8_t *)lib_info = header_type;
              *(uint16_t *)(lib_info + 2) = num_functions;

              void *functions_data = calloc(num_functions, 0x46);
              if (functions_data == NULL) { debug_print("Failed to allocate functions data\n"); free(lib_info); return NULL;}
              *(uint32_t *)(lib_info + 4) = (uint32_t)(uintptr_t)functions_data; // Store as 32-bit pointer

              memcpy(functions_data, (void *)(header_base_addr + func_table_offset), (size_t)num_functions * 0x46);
              
              for (int i = 0; i < num_functions; i++) {
                // Adjust function addresses relative to the base data pointer
                *(uint32_t *)((uintptr_t)functions_data + (size_t)i * 0x46 + 0x42) -= func_ptr_offset;
              }

              *(uint16_t *)(lib_info + 8) = num_resources;
              void *resources_data = calloc(num_resources, 0x42);
              if (resources_data == NULL) { debug_print("Failed to allocate resources data\n"); free(functions_data); free(lib_info); return NULL;}
              *(uint32_t *)(lib_info + 0xc) = (uint32_t)(uintptr_t)resources_data; // Store as 32-bit pointer

              memcpy(resources_data, (void *)(header_base_addr + resource_offset), (size_t)num_resources * 0x42);

              void *data_block = calloc(functions_size, 1); // functions_size is the size of the data block
              if (data_block == NULL) { debug_print("Failed to allocate data block\n"); free(resources_data); free(functions_data); free(lib_info); return NULL;}
              *(uint32_t *)(lib_info + 0x10) = (uint32_t)(uintptr_t)data_block; // Store as 32-bit pointer

              memcpy(data_block, (void *)(header_base_addr + func_ptr_offset), functions_size);
              
              undefined4 main_func_addr = GetFunctionAddress("SharedLibraryMain", (uint32_t)(uintptr_t)lib_info);
              ExecuteFunction((undefined *)(uintptr_t)main_func_addr);
            }
          } else {
            debug_print("Invalid file header: function table offset\n");
            free(lib_info); return NULL;
          }
        }
      } else {
        debug_print("Invalid file header: type\n");
        free(lib_info); return NULL;
      }
    } else {
      debug_print("Invalid file header: magic\n");
      free(lib_info); return NULL;
    }
  } else {
    debug_print("Invalid file header: size %u != %u\n", *(uint32_t *)header_base_addr, *(uint32_t *)(base_addr + 0x44));
    free(lib_info); return NULL;
  }
  return lib_info;
}