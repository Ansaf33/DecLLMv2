#include <stdlib.h> // For malloc, free, exit
#include <stdint.h> // For uintptr_t (used in transmit_all for casting pointer to int)
#include <stdio.h>  // For fprintf (in allocate_mem) and dummy receive/transmit output

// Type definition for 'uint'
typedef unsigned int uint;

// --- Dummy External Functions and Global Variables ---

// transmit: Simulates sending data over a communication channel.
// It takes a buffer, the count of bytes to send, and a pointer to an int
// where the actual number of bytes sent will be stored.
// Returns 0 on success, non-zero on error.
int transmit(void* buffer, unsigned int count, int* bytes_sent) {
    if (bytes_sent) {
        *bytes_sent = count; // Simulate transmitting all requested bytes
    }
    // For debugging, print what's transmitted.
    if (buffer && count > 0) {
        char* char_buffer = (char*)buffer;
        int printable = 1;
        // Check if the buffer content looks like a printable string
        for (unsigned int i = 0; i < count; ++i) {
            if (char_buffer[i] == '\0' || (char_buffer[i] < 32 && char_buffer[i] != '\n' && char_buffer[i] != '\r')) {
                printable = 0;
                break;
            }
        }
        if (printable) {
            printf("Transmit (string): %.*s\n", count, char_buffer);
        } else {
            // If not a string, or partial string, print as a raw value (e.g., final result)
            if (count == 4) { // Assuming 4-byte values are typical results
                printf("Transmit (value): %u (0x%X)\n", *(unsigned int*)buffer, *(unsigned int*)buffer);
            } else {
                printf("Transmit (raw %u bytes) from %p\n", count, buffer);
            }
        }
    }
    return 0; // Success
}

// _terminate: Halts program execution, typically due to an unrecoverable error.
void _terminate(void) {
    fprintf(stderr, "Program terminated due to an error.\n");
    exit(1);
}

// gHelpMsg: A global string containing a help message.
char gHelpMsg[] = "Help Message\n";

// allocate_mem: Allocates a block of memory of the specified size.
// Returns a pointer to the allocated memory, or terminates on failure.
void* allocate_mem(unsigned int size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed for size %u\n", size);
        _terminate();
    }
    return ptr;
}

// deallocate_mem: Frees a previously allocated block of memory.
void deallocate_mem(void* ptr) {
    free(ptr);
}

// Global variables to control receive behavior for testing/termination
static unsigned int receive_program_counter = 0;
// Example input program for the VM: PUSH 10, PUSH 20, ADD, TERMINATE
// Opcodes: 0=PUSH, 1=POP, 2=CALL, 3=JUMP/RET, 4=SWAP, 5=PEEK, 6=ADD, 7=SUB
// Instruction format: (operand << 3) | opcode
// PUSH 10: (10 << 3) | 0 = 80 (0x50)
// PUSH 20: (20 << 3) | 0 = 160 (0xA0)
// ADD: (0 << 3) | 6 = 6 (0x06)
// TERMINATE: 0xFFFFFFFF
unsigned int program_input[] = {
    0x50,       // PUSH 10
    0xA0,       // PUSH 20
    0x06,       // ADD
    0xFFFFFFFF  // Terminate instruction
};
const unsigned int program_input_size = sizeof(program_input) / sizeof(program_input[0]);

// receive: Simulates receiving data into a buffer.
// For this dummy, it provides instructions from `program_input` array.
// Returns 0 on success, non-zero on error.
int receive(void* buffer, unsigned int count, int* bytes_received) {
    if (bytes_received) {
        *bytes_received = 0; // Default to 0 bytes received
    }

    // Basic argument validation
    if (buffer == NULL || count < sizeof(unsigned int) || bytes_received == NULL) {
        return -1; // Error for invalid arguments
    }

    // Provide instructions from the predefined array
    if (receive_program_counter < program_input_size) {
        *(unsigned int*)buffer = program_input[receive_program_counter];
        *bytes_received = sizeof(unsigned int); // Assume 4 bytes received at once
        receive_program_counter++;
    } else {
        // Once all predefined instructions are "received", signal termination
        *(unsigned int*)buffer = 0xFFFFFFFF; // Termination instruction
        *bytes_received = sizeof(unsigned int);
        // Do not increment receive_program_counter further to avoid out-of-bounds access
    }
    return 0; // Success
}

// --- Main Functions ---

// transmit_all: Transmits a specified total size of data from a buffer.
// It repeatedly calls the `transmit` function until all data is sent or an error occurs.
// param_1_total_size: The total number of bytes to transmit.
// param_2_buf: A pointer to the buffer containing the data to transmit.
// Returns the total number of bytes successfully transmitted (which should be param_1_total_size on success).
unsigned int transmit_all(unsigned int param_1_total_size, void* param_2_buf) {
    // If the buffer is NULL, or total size is 0, nothing to transmit.
    // The original code returns 0 if the buffer pointer (param_1 in original) was 0.
    if (param_2_buf == NULL) {
        return 0;
    }

    unsigned int bytes_transmitted_total = 0;
    int bytes_transmitted_chunk = 0;
    int transmit_result;

    while (bytes_transmitted_total < param_1_total_size) {
        // Calculate remaining bytes and advance buffer pointer
        transmit_result = transmit((char*)param_2_buf + bytes_transmitted_total,
                                   param_1_total_size - bytes_transmitted_total,
                                   &bytes_transmitted_chunk);
        
        // Check for transmission errors or if no bytes were transmitted in a call
        if (transmit_result != 0 || bytes_transmitted_chunk == 0) {
            _terminate(); // Unrecoverable error, terminate program
        }
        bytes_transmitted_total += bytes_transmitted_chunk;
    }
    return param_1_total_size; // Return the total size requested, as per original logic
}

// main: The entry point of the program.
// It loads VM instructions, then executes them on a simple stack-based VM.
int main(void) {
  void* vm_stack_ptr = NULL;     // Base address of the VM's operational stack
  void* program_mem_ptr = NULL;  // Base address of the program's instruction memory

  // Variables for the instruction loading phase
  unsigned int current_instruction_word = 0; // Stores the 4-byte instruction word read from 'receive'
  int bytes_received_chunk = 0;              // Stores bytes received in a single 'receive' call
  unsigned int total_bytes_received_for_word = 0; // Accumulates bytes for the current instruction word
  int receive_result;

  // Variables for the VM state and execution phase
  int vm_stack_idx = -1; // VM stack pointer, an index into vm_stack_ptr (0-indexed). -1 indicates empty.
  int program_counter = 0; // Current instruction index into program_mem_ptr during loading

  // --- Memory Allocation for VM Components ---
  // Allocate 0x1000 bytes for the VM stack (1024 unsigned int entries)
  vm_stack_ptr = allocate_mem(0x1000); 
  // Allocate 0x2000 bytes for program instruction memory (2048 unsigned int entries)
  program_mem_ptr = allocate_mem(0x2000); 

  // --- Instruction Loading Loop ---
  // Reads 4-byte instruction words from the 'receive' function into 'program_mem_ptr'
  // until a 0xFFFFFFFF termination instruction is encountered.
  do {
    total_bytes_received_for_word = 0; // Reset accumulator for each new instruction word
    do {
      // Attempt to receive 4 bytes for one instruction word
      receive_result = receive((char*)&current_instruction_word + total_bytes_received_for_word,
                               sizeof(unsigned int) - total_bytes_received_for_word,
                               &bytes_received_chunk);
      
      // Check for errors during reception or if no bytes were received
      if (receive_result != 0 || bytes_received_chunk == 0) {
        // If an error occurs before any instructions are loaded, transmit help message
        if (program_counter == 0) {
          transmit_all(sizeof(gHelpMsg), gHelpMsg);
        }
        _terminate(); // Unrecoverable error, terminate program
      }
      total_bytes_received_for_word += bytes_received_chunk;
    } while (total_bytes_received_for_word < sizeof(unsigned int)); // Loop until a full 4-byte word is received

    // Store the received instruction word into the program memory
    if (program_counter >= (0x2000 / sizeof(unsigned int))) { // Check for program memory overflow
      transmit_all(sizeof("INSNS OVERFLOW EXCEPTION\n"), "INSNS OVERFLOW EXCEPTION\n");
      _terminate();
    }
    ((unsigned int*)program_mem_ptr)[program_counter] = current_instruction_word;
    program_counter++;

  } while (current_instruction_word != 0xffffffff); // Continue until the termination instruction is received

  // --- Instruction Execution Loop ---
  // Iterates through the loaded instructions and executes VM operations based on their opcodes.
  // 'i' serves as the program counter for execution.
  // The loop runs as long as 'i' is within the loaded program bounds and the instruction is not 0xFFFFFFFF.
  for (int i = 0; i < program_counter && ((unsigned int*)program_mem_ptr)[i] != 0xffffffff; i++) {
    unsigned int instruction = ((unsigned int*)program_mem_ptr)[i];
    unsigned int opcode = instruction & 7;     // Extract opcode (last 3 bits)
    unsigned int operand = instruction >> 3;   // Extract operand (remaining bits)

    switch(opcode) {
    case 0: { // PUSH: Push operand onto the stack
      vm_stack_idx++;
      if (vm_stack_idx >= (0x1000 / sizeof(unsigned int))) { // Stack overflow check
        transmit_all(sizeof("STACK OVERFLOW EXCEPTION\n"), "STACK OVERFLOW EXCEPTION\n");
        _terminate();
      }
      ((unsigned int*)vm_stack_ptr)[vm_stack_idx] = operand;
      break;
    }
    case 1: { // POP: Pop value from the stack
      if (vm_stack_idx < 0) { // Stack underflow check
        transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
        _terminate();
      }
      vm_stack_idx--;
      break;
    }
    case 2: { // CALL: Push current PC onto stack for return
      vm_stack_idx++;
      if (vm_stack_idx >= (0x1000 / sizeof(unsigned int))) { // Stack overflow check
        transmit_all(sizeof("STACK OVERFLOW EXCEPTION\n"), "STACK OVERFLOW EXCEPTION\n");
        _terminate();
      }
      ((int*)vm_stack_ptr)[vm_stack_idx] = i; // Push current instruction index (PC)
      break;
    }
    case 3: { // JUMP/RET: Conditional jump based on top of stack
      if (vm_stack_idx < 1) { // Requires at least two elements: condition and target PC
        transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
        _terminate();
      }
      // If the top of the stack (condition) is 0, perform the jump
      if (((int*)vm_stack_ptr)[vm_stack_idx] == 0) { 
        int target_pc = ((int*)vm_stack_ptr)[vm_stack_idx - 1]; // Get target PC from the second stack item
        if (target_pc < 0 || program_counter <= target_pc) { // Validate target PC
          transmit_all(sizeof("INVALID PROGRAM COUNTER EXCEPTION\n"), "INVALID PROGRAM COUNTER EXCEPTION\n");
          _terminate();
        }
        i = target_pc - 1; // Adjust loop counter 'i' to jump. '-1' because 'i' will be incremented by the for loop.
      }
      vm_stack_idx -= 2; // Pop both the condition and the target PC from the stack
      break;
    }
    case 4: { // SWAP: Swap top of stack with element at an offset
      if (vm_stack_idx < 0) { // Requires at least one element on the stack
        transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
        _terminate();
      }
      unsigned int top_value = ((unsigned int*)vm_stack_ptr)[vm_stack_idx]; // Store the value at the top of the stack
      int target_idx = vm_stack_idx - operand; // Calculate the target index using operand as an offset
      if (target_idx < 0 || vm_stack_idx < target_idx) { // Validate the calculated target index
        transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
        _terminate();
      }
      // Perform the swap operation
      ((unsigned int*)vm_stack_ptr)[vm_stack_idx] = ((unsigned int*)vm_stack_ptr)[target_idx];
      ((unsigned int*)vm_stack_ptr)[target_idx] = top_value;
      break;
    }
    case 5: { // PEEK: Push value from an offset on stack to top
      vm_stack_idx++;
      if (vm_stack_idx >= (0x1000 / sizeof(unsigned int))) { // Stack overflow check
        transmit_all(sizeof("STACK OVERFLOW EXCEPTION\n"), "STACK OVERFLOW EXCEPTION\n");
        _terminate();
      }
      int source_idx = (vm_stack_idx - operand) - 1; // Calculate the source index relative to the new stack top
      if (source_idx < 0 || vm_stack_idx < source_idx) { // Validate the source index
        transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
        _terminate();
      }
      ((unsigned int*)vm_stack_ptr)[vm_stack_idx] = ((unsigned int*)vm_stack_ptr)[source_idx]; // Push the peeked value onto the stack
      break;
    }
    case 6: { // ADD: Pop two, add, push result
      if (vm_stack_idx < 1) { // Requires at least two elements for addition
        transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
        _terminate();
      }
      // Perform addition: second_item = second_item + top_item
      ((int*)vm_stack_ptr)[vm_stack_idx - 1] = ((int*)vm_stack_ptr)[vm_stack_idx - 1] + ((int*)vm_stack_ptr)[vm_stack_idx];
      vm_stack_idx--; // Pop the top item (result is now at vm_stack_idx-1)
      break;
    }
    case 7: { // SUB: Pop two, subtract, push result
      if (vm_stack_idx < 1) { // Requires at least two elements for subtraction
        transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
        _terminate();
      }
      // Perform subtraction: second_item = second_item - top_item
      ((int*)vm_stack_ptr)[vm_stack_idx - 1] = ((int*)vm_stack_ptr)[vm_stack_idx - 1] - ((int*)vm_stack_ptr)[vm_stack_idx];
      vm_stack_idx--; // Pop the top item
      break;
    }
    default: { // Handle unknown opcodes
        transmit_all(sizeof("UNKNOWN OPCODE EXCEPTION\n"), "UNKNOWN OPCODE EXCEPTION\n");
        _terminate();
    }
    }
  }

  // --- Post-Execution Finalization ---
  // After the instruction loop, transmit the final value remaining on the stack.
  if (vm_stack_idx < 0) { // Check for stack underflow before attempting to access the result
    transmit_all(sizeof("STACK UNDERFLOW EXCEPTION\n"), "STACK UNDERFLOW EXCEPTION\n");
    _terminate();
  }
  // Transmit the top 4 bytes (one unsigned int) from the stack as the program's final result.
  transmit_all(sizeof(int), &((int*)vm_stack_ptr)[vm_stack_idx]);

  // Deallocate dynamically allocated memory to prevent memory leaks
  deallocate_mem(vm_stack_ptr);
  deallocate_mem(program_mem_ptr);

  return 0; // Program completed successfully
}