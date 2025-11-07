#include <stdint.h> // For uint32_t, uintptr_t
#include <stdlib.h> // For malloc, free, exit
#include <stdio.h>  // For fprintf (for mock error output)
#include <string.h> // For memcpy, strlen (used in mocks for clarity)

// --- Type Definitions ---
typedef unsigned int uint;
typedef char undefined;      // Represents a byte
typedef uint32_t undefined4; // Represents a 4-byte unsigned integer or pointer

// --- Global Variables ---
// gHelpMsg content, as implied by the snippet's usage
char gHelpMsg[] = "Usage: This program simulates a simple stack-based virtual machine.\n"
                  "It reads 4-byte instructions, executes them, and outputs the final stack top.\n"
                  "Instructions are: PUSH_IMM (val), POP, PUSH_PC, JZ (target), SWAP (idx), DUP (idx), ADD, SUB.\n"
                  "Program terminates on instruction 0xFFFFFFFF.\n";

// --- Mock External Functions ---

// Mock for memory allocation
void allocate(unsigned int size, int flags, uint32_t* ptr_output) {
    void* allocated_mem = malloc(size);
    if (!allocated_mem) {
        fprintf(stderr, "Mock allocate: Failed to allocate %u bytes. Terminating.\n", size);
        exit(1);
    }
    *ptr_output = (uint32_t)(uintptr_t)allocated_mem;
}

// Mock for memory deallocation
void deallocate(uint32_t ptr, unsigned int size) {
    if (ptr != 0) {
        free((void*)(uintptr_t)ptr);
    }
}

// Mock for transmitting data/messages
// param_1: Interpreted as a pointer to the data source (cast from int).
// param_2: Number of bytes to send.
// param_3: Pointer to an int where the actual number of bytes sent will be stored.
int transmit(int param_1, unsigned int param_2, int *param_3) {
    const void* data_source = (const void*)(uintptr_t)param_1; // Cast int to pointer
    unsigned int bytes_to_send = param_2;
    
    if (bytes_to_send == 0) {
        if (param_3) *param_3 = 0;
        return 0; // Success, no bytes sent
    }
    
    // Simulate transmitting data.
    // For simplicity, always "send" up to 4 bytes or the requested amount.
    unsigned int actual_sent = (bytes_to_send < 4) ? bytes_to_send : 4;
    if (actual_sent == 0 && bytes_to_send > 0) actual_sent = 1; // Ensure at least 1 byte if requested
    
    if (param_3) *param_3 = actual_sent;

    // For demonstration, print the transmitted data
    fprintf(stdout, "Transmit: ");
    for (unsigned int i = 0; i < actual_sent; ++i) {
        fprintf(stdout, "%02X ", ((const unsigned char*)data_source)[i]);
    }
    fprintf(stdout, "\n");
    
    return 0; // Success
}

// Mock for receiving data
// This mock simulates reading 4-byte instructions from a predefined array.
static unsigned int mock_input_data[] = {
    0x00000028, // PUSH_IMM 5 (0x28 >> 3 = 5)
    0x00000018, // PUSH_IMM 3 (0x18 >> 3 = 3)
    0x00000006, // ADD (5 + 3 = 8)
    0xFFFFFFFF  // Terminate instruction
};
static int mock_input_word_idx = 0;
static int mock_input_byte_offset = 0; // Offset into the current word being read

int receive(void* buffer, unsigned int bytes_to_read, int* bytes_read_output) {
    if (mock_input_word_idx >= (int)(sizeof(mock_input_data) / sizeof(unsigned int))) {
        if (bytes_read_output) *bytes_read_output = 0;
        return 0; // No more data
    }

    unsigned int current_word = mock_input_data[mock_input_word_idx];
    char* word_bytes = (char*)&current_word;

    unsigned int bytes_to_copy = 0;
    if (bytes_to_read > 0) {
        bytes_to_copy = (bytes_to_read < (sizeof(unsigned int) - mock_input_byte_offset)) 
                        ? bytes_to_read 
                        : (sizeof(unsigned int) - mock_input_byte_offset);
        memcpy(buffer, word_bytes + mock_input_byte_offset, bytes_to_copy);
    }
    
    if (bytes_read_output) *bytes_read_output = bytes_to_copy;

    mock_input_byte_offset += bytes_to_copy;
    if (mock_input_byte_offset == sizeof(unsigned int)) {
        mock_input_word_idx++;
        mock_input_byte_offset = 0;
    }
    return 0; // Success
}

// _terminate: no args, exits program
void _terminate(void) {
    fprintf(stderr, "Program terminated due to unhandled exception.\n");
    exit(1);
}

// --- Function: transmit_all ---
// param_1: Interpreted as a pointer to the data source (cast from int).
// param_2: Total number of bytes to transmit from the data source.
uint transmit_all(int param_1, uint param_2) {
  int bytes_sent_this_call = 0;
  uint total_bytes_accumulated = 0;
  
  if (param_1 == 0 || param_2 == 0) {
    return 0; // Nothing to transmit or invalid source, return 0 bytes sent.
  }

  do {
    int transmit_result = transmit(
        (int)((uintptr_t)param_1 + total_bytes_accumulated), // Offset into data_source
        param_2 - total_bytes_accumulated,                   // Bytes remaining
        &bytes_sent_this_call
    );

    if (transmit_result != 0 || bytes_sent_this_call == 0) {
        _terminate(); // Terminate if transmit fails or sends 0 bytes
    }
    total_bytes_accumulated += bytes_sent_this_call;
  } while (total_bytes_accumulated < param_2);

  return param_2; // If all bytes were sent successfully, return the total requested count.
}

// --- Function: main ---
undefined4 main(void) {
  // Removed unused and decompiler-artifact variables (e.g., puVarX, auStack_XX, local_14, local_54, local_58, etc.)
  
  // Renamed variables for improved clarity, reducing intermediate variables where possible.
  uint32_t stack_base_addr = 0; // Base address for the stack memory region
  uint32_t code_base_addr = 0;  // Base address for the program code memory region

  int bytes_received_this_call = 0; // Number of bytes received in a single 'receive' call
  uint current_instruction = 0;     // The 4-byte instruction word currently being processed
  uint total_bytes_received_for_instruction = 0; // Accumulator for bytes read for current_instruction

  int stack_pointer = -1; // Stack pointer, initialized to -1 (indicating an empty stack)
  int instruction_count = 0; // Counter for loaded instructions (also used as program size limit)

  int receive_result = 0; // Return value of the 'receive' function

  // Temporary variables for initial memory allocations/deallocations
  uint32_t temp_alloc_1 = 0;
  uint32_t temp_alloc_2 = 0;

  // Memory allocations as per the snippet
  allocate(0x1000, 0, &temp_alloc_1);    // Allocate 4KB (likely a temporary buffer)
  allocate(0x1000, 0, &stack_base_addr); // Allocate 4KB for the stack
  allocate(0x1000, 0, &temp_alloc_2);    // Allocate 4KB (another temporary buffer)
  allocate(0x2000, 0, &code_base_addr);  // Allocate 8KB for the program code

  // Deallocations of temporary buffers
  deallocate(temp_alloc_1, 0x1000);
  deallocate(temp_alloc_2, 0x1000);

  // --- Program Loading Loop ---
  // Reads 4-byte instructions until 0xFFFFFFFF is encountered.
  do {
    total_bytes_received_for_instruction = 0; // Reset byte counter for the current instruction
    do {
      receive_result = receive(
          (char*)&current_instruction + total_bytes_received_for_instruction, // Buffer to fill (byte by byte into current_instruction)
          4 - total_bytes_received_for_instruction,                           // Bytes remaining to read for this instruction
          &bytes_received_this_call                                           // Output: actual bytes received by mock 'receive'
      );

      if (receive_result != 0 || bytes_received_this_call == 0) {
        if (instruction_count == 0) { // If no instructions were loaded before an error
          // Original snippet used stack manipulation to pass arguments, then called transmit_all().
          // Interpreted as: transmit_all((int)(uintptr_t)gHelpMsg, 0x7a4);
          // Note: 0x7a4 is the hardcoded length from the original snippet, even if it's larger than strlen(gHelpMsg).
          transmit_all((int)(uintptr_t)gHelpMsg, 0x7a4);
        }
        _terminate(); // Terminate on receive error or zero bytes received
      }
      total_bytes_received_for_instruction += bytes_received_this_call; // Accumulate bytes
    } while (total_bytes_received_for_instruction < 4); // Loop until a full 4-byte instruction is received

    // Store the received instruction into the code memory region
    *(uint32_t*)(code_base_addr + instruction_count * sizeof(uint32_t)) = current_instruction;
    instruction_count++; // Increment total instruction count

    // Check for instruction memory overflow (max 2048 instructions)
    if (0x7ff < instruction_count) {
      // Original snippet error handling
      transmit_all((int)(uintptr_t)"INSNS OVERFLOW EXCEPTION\n", 0x1a); // 0x1a is hardcoded length
      _terminate();
    }
  } while (current_instruction != 0xffffffff); // Continue until the program termination instruction (0xFFFFFFFF) is read

  // --- Instruction Execution Loop ---
  // Iterates through loaded instructions, executing them based on the opcode.
  for (int program_counter = 0; // Program Counter (PC)
      (program_counter < instruction_count && 
       *(int*)(code_base_addr + program_counter * sizeof(uint32_t)) != -1 && // Ensure instruction is not -1
       (current_instruction = *(uint32_t*)(code_base_addr + program_counter * sizeof(uint32_t)), current_instruction != 0xffffffff)); // Load instruction and check for termination
      program_counter++) { // Increment PC after each instruction
    
    // Decode instruction opcode (last 3 bits of the instruction word)
    switch(current_instruction & 7) { 
    case 0: { // PUSH_IMM: Push immediate value onto stack
      stack_pointer++; // Increment stack pointer
      *(uint32_t*)(stack_base_addr + stack_pointer * sizeof(uint32_t)) = current_instruction >> 3; // Push value (instruction shifted right by 3 bits)
      break;
    }
    case 1: { // POP: Remove top element from stack
      if (stack_pointer < 0) { // Stack underflow check
        transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
        _terminate();
      }
      stack_pointer--; // Decrement stack pointer
      break;
    }
    case 2: { // PUSH_PC: Push current program counter onto stack
      stack_pointer++; // Increment stack pointer
      if (0x3ff < stack_pointer) { // Stack overflow check (max 1024 elements)
        transmit_all((int)(uintptr_t)"STACK OVERFLOW EXCEPTION\n", 0x1a);
        _terminate();
      }
      *(int*)(stack_base_addr + stack_pointer * sizeof(uint32_t)) = program_counter; // Push current PC
      break;
    }
    case 3: { // JZ (Jump if Zero): Pop two elements; if the first is zero, jump to the second.
      if (stack_pointer < 1) { // Stack underflow check (needs at least 2 elements)
        transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
        _terminate();
      }
      // Top of stack is value to check, second is the jump target address
      if (*(int*)(stack_base_addr + stack_pointer * sizeof(uint32_t)) == 0) { // If top element is 0
        int jump_target_pc = *(int*)(stack_base_addr + (stack_pointer - 1) * sizeof(uint32_t)); // Get jump target
        if (jump_target_pc < 0 || instruction_count <= jump_target_pc) { // Validate jump target
          transmit_all((int)(uintptr_t)"INVALID PROGRAM COUNTER EXCEPTION\n", 0x23);
          _terminate();
        }
        program_counter = jump_target_pc - 1; // Set PC to target - 1, as loop increments PC
      }
      stack_pointer -= 2; // Pop both elements (value and target)
      break;
    }
    case 4: { // SWAP: Swap top element with an element at a given offset from top.
      if (stack_pointer < 0) { // Stack underflow check
        transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
        _terminate();
      }
      uint32_t temp_val_for_swap = *(uint32_t*)(stack_base_addr + stack_pointer * sizeof(uint32_t)); // Store top value
      int swap_offset = current_instruction >> 3; // Offset from top (encoded in instruction)
      int swap_target_idx = stack_pointer - swap_offset; // Calculate index of element to swap with

      if (swap_target_idx < 0 || stack_pointer < swap_target_idx) { // Validate target index
        transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
        _terminate();
      }
      // Perform the swap
      *(uint32_t*)(stack_base_addr + stack_pointer * sizeof(uint32_t)) = *(uint32_t*)(stack_base_addr + swap_target_idx * sizeof(uint32_t));
      *(uint32_t*)(stack_base_addr + swap_target_idx * sizeof(uint32_t)) = temp_val_for_swap;
      break;
    }
    case 5: { // DUP: Duplicate an element at a given offset from top to the new top.
      stack_pointer++; // Increment stack pointer for the new duplicated element
      int dup_offset = current_instruction >> 3; // Offset from top (encoded in instruction)
      int source_idx = stack_pointer - 1 - dup_offset; // Calculate index of element to duplicate

      if (source_idx < 0 || source_idx >= stack_pointer) { // Stack underflow check for source index
        transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
        _terminate();
      }
      // Duplicate the value
      *(uint32_t*)(stack_base_addr + stack_pointer * sizeof(uint32_t)) = *(uint32_t*)(stack_base_addr + source_idx * sizeof(uint32_t));
      break;
    }
    case 6: { // ADD: Pop two, push sum.
      if (stack_pointer < 1) { // Stack underflow check (needs at least 2 elements)
        transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
        _terminate();
      }
      // Add top two elements, store result in the second-from-top position
      *(int*)(stack_base_addr + (stack_pointer - 1) * sizeof(uint32_t)) += *(int*)(stack_base_addr + stack_pointer * sizeof(uint32_t));
      stack_pointer--; // Pop the top element
      break;
    }
    case 7: { // SUB: Pop two, push difference (second - top).
      if (stack_pointer < 1) { // Stack underflow check (needs at least 2 elements)
        transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
        _terminate();
      }
      // Subtract top element from the second-from-top, store result in second-from-top
      *(int*)(stack_base_addr + (stack_pointer - 1) * sizeof(uint32_t)) -= *(int*)(stack_base_addr + stack_pointer * sizeof(uint32_t));
      stack_pointer--; // Pop the top element
      break;
    }
    }
  }

  // --- Final Output ---
  if (stack_pointer < 0) { // Final stack underflow check
    transmit_all((int)(uintptr_t)"STACK UNDERFLOW EXCEPTION\n", 0x1b);
    _terminate();
  }
  uint32_t final_result_addr = stack_base_addr + stack_pointer * sizeof(uint32_t);
  // Transmit the value at the top of the stack (the program's final result)
  transmit_all((int)(uintptr_t)final_result_addr, sizeof(uint32_t)); 
  
  // Clean up allocated memory
  deallocate(code_base_addr, 0x2000);
  deallocate(stack_base_addr, 0x1000);

  return 0; // Program finished successfully
}