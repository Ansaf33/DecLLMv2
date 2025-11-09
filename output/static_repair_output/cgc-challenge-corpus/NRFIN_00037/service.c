#include <stdint.h> // For uint8_t, uint16_t, uint32_t
#include <stddef.h> // For size_t (though not strictly necessary for this snippet, good practice)

// Dummy external function declarations for compilation.
// These functions are assumed to be provided by the execution environment.
extern int allocate(uint32_t size, int flags, int *out_ptr, uint32_t type);
extern int receive_all(int fd, void *buf, uint16_t len, uint32_t *bytes_received);
extern int transmit_all(int fd, void *buf, uint16_t len, uint32_t *bytes_transmitted);
extern void deallocate(int ptr, uint32_t size);


// Function: inst_decode
// Decodes a single instruction from the bytecode stream.
// param_1: Pointer to the start of the instruction in the bytecode.
// param_2: Remaining length of the bytecode stream.
// param_3: An array to store decoded instruction components:
//          param_3[0]: Instruction length (byte)
//          param_3[1]: Instruction byte (opcode and flags)
//          param_3[4-7]: Decoded Source 1 value (uint32_t)
//          param_3[8-11]: Decoded Source 2 value (uint32_t)
// Returns 0 on success, 0xffffffe0 if param_2 is too small for the instruction.
uint32_t inst_decode(uint8_t *param_1, uint16_t param_2, uint8_t *param_3) {
    param_3[1] = *param_1; // Store the first byte of the instruction (opcode + flags)

    // Calculate instruction length based on flags in param_3[1]
    // (param_3[1] & 4) == 0 ? 3 : 5 : Base length for source 1 (2-byte or 4-byte)
    // (param_3[1] & 8) == 0 ? 2 : 4 : Additional length for source 2 (2-byte or 4-byte)
    uint8_t len = ((param_3[1] & 4) == 0 ? 3 : 5) + ((param_3[1] & 8) == 0 ? 2 : 4);

    if (param_2 < len) {
        return 0xffffffe0; // Not enough bytes for the instruction
    } else {
        *param_3 = len; // Store the actual instruction length in param_3[0]

        int src1_offset_base; // Offset into param_1 for the start of source 2, depends on source 1 size
        if ((param_3[1] & 4) == 0) { // If source 1 is 2 bytes (flags bit 2 is 0)
            // Source 1 value from param_1[1] and param_1[2]
            *(uint32_t *)(param_3 + 4) = ((uint32_t)param_1[1] << 8) | (uint32_t)param_1[2];
            src1_offset_base = 2; // Source 2 starts after 2 bytes of source 1
        } else { // If source 1 is 4 bytes (flags bit 2 is 1)
            // Source 1 value from param_1[1] to param_1[4] (big-endian interpretation)
            *(uint32_t *)(param_3 + 4) =
                 (uint32_t)param_1[4] |
                 (uint32_t)param_1[1] << 24 | (uint32_t)param_1[2] << 16 | (uint32_t)param_1[3] << 8;
            src1_offset_base = 4; // Source 2 starts after 4 bytes of source 1
        }

        if ((param_3[1] & 8) == 0) { // If source 2 is 2 bytes (flags bit 3 is 0)
            // Source 2 value from param_1[src1_offset_base + 1] and param_1[src1_offset_base + 2]
            *(uint32_t *)(param_3 + 8) = ((uint32_t)param_1[src1_offset_base + 1] << 8) | (uint32_t)param_1[src1_offset_base + 2];
        } else { // If source 2 is 4 bytes (flags bit 3 is 1)
            // Source 2 value from param_1[src1_offset_base + 1] to param_1[src1_offset_base + 4]
            *(uint32_t *)(param_3 + 8) =
                 (uint32_t)param_1[src1_offset_base + 4] |
                 (uint32_t)param_1[src1_offset_base + 1] << 24 | (uint32_t)param_1[src1_offset_base + 2] << 16 |
                 (uint32_t)param_1[src1_offset_base + 3] << 8;
        }
    }
    return 0; // Success
}

// Function: bytecode_vrfy
// Verifies the bytecode stream for certain conditions.
// param_1: Start address of the bytecode.
// param_2: Length of the bytecode.
// Returns 0 on success, or an error code if verification fails.
uint32_t bytecode_vrfy(int param_1, uint16_t param_2) {
    uint32_t current_bytecode_ptr = (uint32_t)param_1;
    uint16_t remaining_len = param_2;

    // Array to hold decoded instruction parts:
    // [0]: Instruction length
    // [1]: Instruction byte (opcode and flags)
    // [4-7]: Source 1 value
    // [8-11]: Source 2 value
    uint8_t decoded_inst[12];

    while (1) {
        if (remaining_len == 0) {
            return 0; // All bytecode verified successfully
        }

        int decode_result = inst_decode((uint8_t *)current_bytecode_ptr, remaining_len, decoded_inst);
        if (decode_result != 0) {
            return 0xffffffbd; // Instruction decode failed
        }

        uint8_t inst_len = decoded_inst[0];
        uint8_t inst_byte = decoded_inst[1];
        uint32_t src1_val = *(uint32_t *)(decoded_inst + 4);
        uint32_t src2_val = *(uint32_t *)(decoded_inst + 8);

        // Verification checks
        if (((inst_byte & 4) != 0) && (0xffc < src1_val)) {
            return 0xffffffe5; // Source 1 value out of bounds for memory access
        }
        if (((inst_byte & 8) != 0) && (0xffc < src2_val)) {
            return 0xffffffe5; // Source 2 value out of bounds for memory access
        }

        current_bytecode_ptr += inst_len;
        remaining_len -= inst_len;
    }
}

// Function: bytecode_exec
// Executes the bytecode stream.
// param_1: Start address of the bytecode.
// param_2: Length of the bytecode.
// param_3: Base address of the memory region for bytecode operations (e.g., stack).
// param_4: Pointer to store the final result of the execution.
// Returns 0 on success, or an error code on failure.
int bytecode_exec(int param_1, uint16_t param_2, int param_3, uint32_t *param_4) {
    uint32_t current_bytecode_ptr = (uint32_t)param_1;
    uint16_t remaining_len = param_2;
    int ret_val = 0;

    // Array to hold decoded instruction parts
    uint8_t decoded_inst[12];
    uint32_t temp_result_reg = 0; // Temporary register for results when destination is not memory

    while (1) {
        if (remaining_len == 0) {
            return ret_val; // All bytecode executed successfully
        }

        ret_val = inst_decode((uint8_t *)current_bytecode_ptr, remaining_len, decoded_inst);
        if (ret_val != 0) {
            return -0x43; // Instruction decode failed
        }

        uint8_t inst_len = decoded_inst[0];
        uint8_t inst_byte = decoded_inst[1];
        uint32_t src1_val_raw = *(uint32_t *)(decoded_inst + 4); // Raw value for Source 1
        uint32_t src2_val_raw = *(uint32_t *)(decoded_inst + 8); // Raw value for Source 2

        uint32_t *dest_ptr; // Pointer to where the operation result will be stored

        // Determine destination for the result
        int is_dest_memory = (inst_byte & 1) != 0; // Bit 0: 0=temp register, 1=memory
        if (!is_dest_memory) {
            dest_ptr = &temp_result_reg;
        } else {
            dest_ptr = (uint32_t *)((uint32_t)param_3 + src1_val_raw);
        }

        uint32_t operand1_val; // Value of the first operand
        int is_src1_register = (inst_byte & 2) != 0; // Bit 1: 0=immediate/memory, 1=register
        int is_src1_memory = (inst_byte & 4) != 0;   // Bit 2: 0=immediate, 1=memory (if Bit 1 is 0)

        // Error conditions for operand 1 addressing mode
        if (is_dest_memory && !is_src1_register) {
            // If destination is memory, source 1 MUST be a register.
            return -0x3e;
        }
        if (is_src1_register && is_src1_memory) {
            // Source 1 cannot be both a register and memory-addressed.
            return -0x3e;
        }

        // Determine operand 1 value
        if (is_src1_register) {
            operand1_val = *dest_ptr; // Value from the destination register/memory (previous result)
        } else { // Source 1 is immediate or memory
            if (is_src1_memory) {
                // Read 4 bytes from memory at param_3 + src1_val_raw (big-endian)
                operand1_val =
                    ((uint32_t)((uint8_t *)param_3)[src1_val_raw + 3] << 24) |
                    ((uint32_t)((uint8_t *)param_3)[src1_val_raw + 2] << 16) |
                    ((uint32_t)((uint8_t *)param_3)[src1_val_raw + 1] << 8) |
                    ((uint32_t)((uint8_t *)param_3)[src1_val_raw]);
            } else { // Source 1 is immediate
                operand1_val = src1_val_raw;
            }
        }

        uint32_t operand2_val; // Value of the second operand
        int is_src2_memory = (inst_byte & 8) != 0; // Bit 3: 0=immediate, 1=memory

        // Determine operand 2 value
        if (!is_src2_memory) {
            operand2_val = src2_val_raw; // Source 2 is immediate
        } else {
            // Read 4 bytes from memory at param_3 + src2_val_raw (big-endian)
            operand2_val =
                ((uint32_t)((uint8_t *)param_3)[src2_val_raw + 3] << 24) |
                ((uint32_t)((uint8_t *)param_3)[src2_val_raw + 2] << 16) |
                ((uint32_t)((uint8_t *)param_3)[src2_val_raw + 1] << 8) |
                ((uint32_t)((uint8_t *)param_3)[src2_val_raw]);
        }

        uint8_t opcode = inst_byte & 0xf0; // Extract opcode from higher nibble of instruction byte

        // Execute operation based on opcode
        switch (opcode) {
            case 0xf0: // Store operation: copy result to param_4
                *param_4 = *dest_ptr;
                break;
            case 0x30: // Divide operation
                if (operand2_val == 0) {
                    operand2_val = 0xf000f000; // Special handling for division by zero
                }
                *dest_ptr = operand1_val / operand2_val;
                break;
            case 0x20: // Multiply operation
                *dest_ptr = operand1_val * operand2_val;
                break;
            case 0x00: // Add operation
                *dest_ptr = operand1_val + operand2_val;
                break;
            case 0x10: // Subtract operation
                *dest_ptr = operand1_val - operand2_val;
                break;
            default: // Invalid opcode
                return -0x3e;
        }

        current_bytecode_ptr += inst_len;
        remaining_len -= inst_len;
    }
}

// Function: main
// Entry point of the program. Handles allocation, data reception,
// bytecode verification and execution, and result transmission.
int main(void) {
    int status = 0; // General status/return value
    int allocated_mem_ptr = 0; // Pointer to allocated memory, 0 if not allocated
    uint16_t received_len = 0; // Length of bytecode received
    uint32_t bytes_io_status = 0; // Status for I/O operations (bytes transferred)
    uint32_t exec_result = 0xdeadbeef; // Variable to hold the bytecode execution result

    // Allocate 0x1000 bytes of memory
    status = allocate(0x1000, 0, &allocated_mem_ptr, 0x1157f);
    if (status == 0) { // Allocation successful
        // Receive 2 bytes (the length of the bytecode)
        status = receive_all(0, &received_len, 2, &bytes_io_status);
        if (status == 0) { // Received length successfully
            if (received_len < 0x7fd) { // Check if bytecode length is within limits
                // Receive the actual bytecode into allocated memory (offset 0x800)
                status = receive_all(0, (void *)(allocated_mem_ptr + 0x800), received_len, &bytes_io_status);
                if (status == 0) { // Received bytecode successfully
                    // Verify the bytecode
                    status = bytecode_vrfy(allocated_mem_ptr + 0x800, received_len);
                    if (status == 0) { // Bytecode verified successfully
                        // Execute the bytecode
                        status = bytecode_exec(allocated_mem_ptr + 0x800, received_len, allocated_mem_ptr, &exec_result);
                        if (status == 0) { // Bytecode executed successfully
                            // Transmit the execution result (4 bytes)
                            status = transmit_all(1, &exec_result, 4, &bytes_io_status);
                            if (status != 0) {
                                status = -0x58; // Error transmitting result
                            }
                        }
                    }
                } else {
                    status = -0x3f; // Error receiving bytecode
                }
            } else {
                status = 0; // Length too large, but treated as non-error in original logic
            }
        } else {
            status = -0x3f; // Error receiving length
        }
    }
    // If allocate failed (status != 0), the block above is skipped, and the error status is preserved.

    // Deallocate memory if it was successfully allocated
    if (allocated_mem_ptr != 0) {
        deallocate(allocated_mem_ptr, 0x1000);
    }

    return status;
}