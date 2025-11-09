#include <stdio.h>    // For stdin, stdout, stderr, fprintf, fflush
#include <stdlib.h>   // For malloc, free, exit, size_t
#include <stdint.h>   // For uint32_t (if specific width is needed, otherwise unsigned int is fine)
#include <string.h>   // For strlen, memset
#include <unistd.h>   // For STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO

// --- Dummy/Mock implementations for external functions ---
// These are placeholders to make the code compilable and runnable for demonstration.
// In a real system, these would be provided by the OS or a specific library.

// Mock transmit: This function is used by _transmit_bytes_loop.
// It's defined to take a file descriptor, a size, and a pointer to return actual bytes sent.
// It does NOT take a data buffer directly, which is unusual for a generic transmit.
// For the purpose of this mock, it will simulate success.
int transmit(int fd, unsigned int size_to_send, int* actual_sent_bytes) {
    // In a real scenario, this would perform actual I/O.
    // For this mock, we assume successful transmission of requested bytes.
    *actual_sent_bytes = size_to_send;
    return 0; // Success
}

// Mock receive: Reads from stdin or simulates data for the VM.
// Called as receive(fd, buffer, size, &bytes_read)
int receive(int fd, void* buffer, unsigned int size_to_read, int* actual_read_bytes) {
    if (fd == STDIN_FILENO) {
        // Simulate reading 4-byte instructions.
        // This sequence pushes 10, then 5, adds them (result 15),
        // then pushes 2, subtracts (result 13), then terminates.
        static unsigned int mock_instructions[] = {
            (0b000 << 0) | (10 << 3), // PUSH 10
            (0b000 << 0) | (5 << 3),  // PUSH 5
            (0b110 << 0),             // ADD (10+5=15)
            (0b000 << 0) | (2 << 3),  // PUSH 2
            (0b111 << 0),             // SUB (15-2=13)
            0xFFFFFFFF                // Sentinel instruction to stop loading
        };
        static int mock_idx = 0;

        if (mock_idx < sizeof(mock_instructions) / sizeof(mock_instructions[0]) &&
            size_to_read >= sizeof(unsigned int)) {
            ((unsigned int*)buffer)[0] = mock_instructions[mock_idx++];
            *actual_read_bytes = sizeof(unsigned int);
            return 0; // Success
        }
        *actual_read_bytes = 0;
        return -1; // No more mock data or read error
    }
    // For other FDs, simulate failure or no data.
    fprintf(stderr, "MOCK RECEIVE (fd=%d, size=%u): No data simulated.\n", fd, size_to_read);
    *actual_read_bytes = 0;
    return -1; // Failure
}

// Mock _terminate: Exits the program.
void _terminate_execution() {
    fprintf(stderr, "Program terminated due to an error.\n");
    exit(EXIT_FAILURE);
}

// Mock allocate: Simple malloc wrapper.
// allocate(size, flags, &ptr_out)
void allocate(size_t size, int flags, void** ptr_out) {
    *ptr_out = malloc(size);
    if (!*ptr_out) {
        fprintf(stderr, "Memory allocation failed for size %zu.\n", size);
        _terminate_execution();
    }
    // Initialize allocated memory to zero for predictable behavior.
    memset(*ptr_out, 0, size);
}

// Mock deallocate: Simple free wrapper.
// The 'size' argument is often unused in standard free, but kept for signature consistency.
void deallocate(void* ptr, size_t size) {
    free(ptr);
}

// Global help message
const char* gHelpMsg = "Usage: vm_emulator [options]\n";

// --- Function: _transmit_bytes_loop (Original snippet's transmit_all, renamed) ---
// This function transmits 'total_size' bytes to 'fd'.
// It repeatedly calls the low-level 'transmit' function until all bytes are sent.
// It does NOT take a data buffer; it assumes 'transmit' has internal access to data.
unsigned int _transmit_bytes_loop(int fd, unsigned int total_size) {
    if (fd == 0) {
        return 0; // If file descriptor is 0, nothing to transmit.
    }

    unsigned int transmitted_count = 0;
    int current_transmitted_bytes = 0; // Bytes sent in the current 'transmit' call.

    do {
        int transmit_status = transmit(fd, total_size - transmitted_count, &current_transmitted_bytes);

        if (transmit_status != 0 || current_transmitted_bytes == 0) {
            // Error during transmission or 0 bytes transmitted when more were expected.
            _terminate_execution();
        }
        transmitted_count += current_transmitted_bytes;
    } while (transmitted_count < total_size);

    return total_size; // Indicate that the target total_size was transmitted.
}

// --- Function: transmit_all (General purpose, used by main) ---
// This function transmits a specified buffer of data to a file descriptor.
// It is intended for the calls in main() which pass a buffer.
// It prints to stdout/stderr for standard file descriptors.
int transmit_all(int fd, const void* buffer, unsigned int size) {
    if (fd == 0 || buffer == NULL || size == 0) {
        return 0;
    }

    // For stdout/stderr, perform actual printing.
    if (fd == STDOUT_FILENO) {
        fwrite(buffer, 1, size, stdout);
        fflush(stdout);
    } else if (fd == STDERR_FILENO) {
        fwrite(buffer, 1, size, stderr);
        fflush(stderr);
    }

    // Call the underlying mock transmit function to track the "transmission".
    int bytes_sent;
    int status = transmit(fd, size, &bytes_sent);

    if (status != 0 || bytes_sent == 0) {
        _terminate_execution();
    }
    return bytes_sent;
}


// Function: main
int main(void) {
    // Memory blocks for VM components
    void* vm_stack = NULL;        // VM stack memory (corresponds to local_50)
    void* vm_instructions = NULL; // VM instructions memory (corresponds to local_4c)

    // Variables for instruction loading phase
    unsigned int current_instruction_value = 0; // Value of the instruction being read (corresponds to local_44)
    int bytes_read_in_call = 0;                 // Bytes read by a single 'receive' call (corresponds to local_48)
    unsigned int instruction_bytes_loaded = 0;  // Total bytes loaded for current instruction (corresponds to local_24)
    int instruction_count = 0;                  // Number of instructions loaded (corresponds to local_2c)

    // Variables for VM execution phase
    int stack_pointer = -1; // VM stack pointer, -1 for empty stack (corresponds to local_28)
    int program_counter = 0; // VM program counter for execution loop (corresponds to local_30)

    // Allocate memory blocks for VM stack and instructions
    // The original code allocates additional blocks (local_54, local_58) and immediately deallocates them.
    // This behavior is mimicked to match the snippet's original structure.
    void* temp_block_a = NULL;
    void* temp_block_b = NULL;

    allocate(0x1000, 0, &temp_block_a);     // Corresponds to local_54
    allocate(0x1000, 0, &vm_stack);         // Corresponds to local_50 (VM stack)
    allocate(0x1000, 0, &temp_block_b);     // Corresponds to local_58
    allocate(0x2000, 0, &vm_instructions);  // Corresponds to local_4c (VM instructions)

    // Deallocate temporary blocks as in original snippet
    deallocate(temp_block_a, 0x1000);
    deallocate(temp_block_b, 0x1000);

    // --- First loop: Load instructions into vm_instructions buffer ---
    // Reads 4-byte instructions from STDIN_FILENO until 0xFFFFFFFF is encountered.
    do {
        instruction_bytes_loaded = 0; // Reset bytes loaded for the current instruction
        do {
            // Read 4 bytes into current_instruction_value
            int receive_status = receive(STDIN_FILENO,
                                         (char*)&current_instruction_value + instruction_bytes_loaded,
                                         sizeof(unsigned int) - instruction_bytes_loaded,
                                         &bytes_read_in_call);

            if (receive_status != 0 || bytes_read_in_call == 0) {
                if (instruction_count == 0) { // If failure on reading the very first instruction
                    transmit_all(STDERR_FILENO, gHelpMsg, strlen(gHelpMsg));
                }
                _terminate_execution();
            }
            instruction_bytes_loaded += bytes_read_in_call;
        } while (instruction_bytes_loaded < sizeof(unsigned int)); // Ensure 4 bytes are read for one instruction

        // Store the received instruction in the instruction buffer
        ((unsigned int*)vm_instructions)[instruction_count] = current_instruction_value;
        instruction_count++;

        // Check for instruction buffer overflow (0x7FF == 2047, so max 2048 instructions)
        if (instruction_count > 0x7FF) {
            transmit_all(STDERR_FILENO, "INSNS OVERFLOW EXCEPTION\n", strlen("INSNS OVERFLOW EXCEPTION\n"));
            _terminate_execution();
        }
    } while (current_instruction_value != 0xFFFFFFFF); // Loop until the sentinel instruction is read

    // --- Second loop: Execute loaded instructions ---
    // Loop through instructions until instruction_count limit or sentinel value is hit.
    for (program_counter = 0;
         program_counter < instruction_count &&
         ((unsigned int*)vm_instructions)[program_counter] != 0xFFFFFFFF;
         program_counter++)
    {
        current_instruction_value = ((unsigned int*)vm_instructions)[program_counter];
        unsigned int opcode = current_instruction_value & 7; // Last 3 bits for opcode
        int operand = current_instruction_value >> 3;       // Remaining bits for operand

        switch (opcode) {
            case 0: { // PUSH operand
                stack_pointer++;
                // Check for stack overflow (0x3FF == 1023, so max 1024 elements)
                if (stack_pointer >= 0x400) {
                    transmit_all(STDERR_FILENO, "STACK OVERFLOW EXCEPTION\n", strlen("STACK OVERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                ((int*)vm_stack)[stack_pointer] = operand;
                break;
            }
            case 1: { // POP
                if (stack_pointer < 0) {
                    transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                stack_pointer--;
                break;
            }
            case 2: { // CALL (push current PC onto stack)
                stack_pointer++;
                if (stack_pointer >= 0x400) {
                    transmit_all(STDERR_FILENO, "STACK OVERFLOW EXCEPTION\n", strlen("STACK OVERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                ((int*)vm_stack)[stack_pointer] = program_counter;
                break;
            }
            case 3: { // JUMP_IF_ZERO / RETURN
                // Requires at least two elements: condition and target PC
                if (stack_pointer < 1) {
                    transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                int condition = ((int*)vm_stack)[stack_pointer];
                int target_pc = ((int*)vm_stack)[stack_pointer - 1]; // Second from top

                if (condition == 0) { // If condition is zero, jump
                    if (target_pc < 0 || target_pc >= instruction_count) {
                        transmit_all(STDERR_FILENO, "INVALID PROGRAM COUNTER EXCEPTION\n", strlen("INVALID PROGRAM COUNTER EXCEPTION\n"));
                        _terminate_execution();
                    }
                    program_counter = target_pc - 1; // -1 because the for-loop will increment PC
                }
                stack_pointer -= 2; // Pop condition and target PC
                break;
            }
            case 4: { // SWAP (top with element at (sp - operand))
                if (stack_pointer < 0) {
                    transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                int target_idx = stack_pointer - operand;
                // target_idx must be a valid index on the stack and within the current stack range
                if (target_idx < 0 || target_idx > stack_pointer) {
                    transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                int temp_val = ((int*)vm_stack)[stack_pointer];
                ((int*)vm_stack)[stack_pointer] = ((int*)vm_stack)[target_idx];
                ((int*)vm_stack)[target_idx] = temp_val;
                break;
            }
            case 5: { // DUP_N (duplicate element at (sp - operand) to top)
                stack_pointer++; // Make space for the new element
                if (stack_pointer >= 0x400) {
                    transmit_all(STDERR_FILENO, "STACK OVERFLOW EXCEPTION\n", strlen("STACK OVERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                // Source is 'operand' elements below the *new* top of stack (stack_pointer - 1)
                int source_idx = (stack_pointer - 1) - operand;
                if (source_idx < 0 || source_idx >= stack_pointer) { // Source must be valid and within current stack
                    transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                ((int*)vm_stack)[stack_pointer] = ((int*)vm_stack)[source_idx];
                break;
            }
            case 6: { // ADD (top two elements)
                if (stack_pointer < 1) { // Need at least two elements for addition
                    transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                ((int*)vm_stack)[stack_pointer - 1] += ((int*)vm_stack)[stack_pointer];
                stack_pointer--; // Pop the top element
                break;
            }
            case 7: { // SUB (second from top - top)
                if (stack_pointer < 1) { // Need at least two elements for subtraction
                    transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
                    _terminate_execution();
                }
                ((int*)vm_stack)[stack_pointer - 1] -= ((int*)vm_stack)[stack_pointer];
                stack_pointer--; // Pop the top element
                break;
            }
        }
    }

    // After execution loop: Transmit the final result (value at top of stack)
    if (stack_pointer < 0) {
        transmit_all(STDERR_FILENO, "STACK UNDERFLOW EXCEPTION\n", strlen("STACK UNDERFLOW EXCEPTION\n"));
        _terminate_execution();
    }
    // Transmit the 4-byte integer value at the top of the VM stack to stdout.
    transmit_all(STDOUT_FILENO, &((int*)vm_stack)[stack_pointer], sizeof(int));

    // Cleanup dynamically allocated memory
    deallocate(vm_stack, 0x1000);
    deallocate(vm_instructions, 0x2000);

    return 0;
}