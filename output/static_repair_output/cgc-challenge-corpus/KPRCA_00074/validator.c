#include <stdint.h> // For uint32_t, uint8_t, uint16_t
#include <stdbool.h> // For bool

// Assume syscalls is an external array of uint32_t, where each "syscall entry" is 9 uint32_t elements long.
// The raw byte offsets 0x24 (36 bytes) and 4 bytes (for int) confirm this.
extern const uint32_t syscalls[];

// Forward declaration for validate_state, assuming it takes uint32_t* for the state array.
uint32_t validate_state(uint32_t *state);

uint32_t validate_jmp(uint32_t *state) {
    uint32_t current_instruction_addr = state[1] * 8 + state[0];
    uint8_t *instruction_bytes = (uint8_t *)(current_instruction_addr + 4);

    uint8_t opcode_prefix = (*instruction_bytes & 0xf0);
    uint32_t result = 0;

    if (opcode_prefix <= 0x30) { // Check for 0x00, 0x10, 0x20, 0x30
        uint32_t reg_val = ((*instruction_bytes & 8) == 0) ? state[(*(instruction_bytes + 1) >> 4) + 2] : 2;

        if ((reg_val == 2) && (state[(*(instruction_bytes + 1) & 0xf) + 2] == 2)) {
            uint32_t new_state[18];
            for (int i = 0; i < 18; ++i) {
                new_state[i] = state[i];
            }
            new_state[1] = (uint32_t)*(uint16_t *)(instruction_bytes + 2) + new_state[1] + 1;
            result = validate_state(new_state);
        }
    } else if (opcode_prefix == 0x40) {
        uint32_t syscall_idx = 0;
        uint16_t target_syscall_id = *(uint16_t *)(current_instruction_addr + 6);

        while (syscalls[syscall_idx * 9] != 0 && syscall_idx != target_syscall_id) {
            syscall_idx++;
        }

        if (syscalls[syscall_idx * 9] == 0) {
            result = 0;
        } else {
            syscall_idx = 0;
            while (syscalls[(uint32_t)target_syscall_id * 9 + syscall_idx + 1] != 0) {
                if ((state[syscall_idx + 2] & syscalls[(uint32_t)target_syscall_id * 9 + syscall_idx + 1]) == 0) {
                    return 0;
                }
                syscall_idx++;
            }
            state[2] = syscalls[(uint32_t)target_syscall_id * 9];
            result = 1;
        }
    } else if (opcode_prefix == 0x50) {
        result = (state[2] == 2);
    }
    return result;
}

uint32_t validate_alu(uint32_t *state) {
    uint32_t current_instruction_addr = state[1] * 8 + state[0];
    uint8_t *instruction_bytes = (uint8_t *)(current_instruction_addr + 4);

    uint8_t opcode_prefix = *instruction_bytes >> 4;
    uint8_t reg_dest_idx = *(instruction_bytes + 1) & 0xf;

    if (opcode_prefix < 0xd) {
        if (reg_dest_idx == 0xf) {
            return 0;
        } else {
            uint32_t src_val = ((*instruction_bytes & 8) == 0) ? state[(*(instruction_bytes + 1) >> 4) + 2] : 2;
            uint32_t dest_val = state[reg_dest_idx + 2];
            uint32_t combined_val;

            if (opcode_prefix == 0xa || opcode_prefix == 0xc) {
                if (dest_val != 2) {
                    state[reg_dest_idx + 2] = 1;
                }
            } else if (opcode_prefix == 0xb) {
                state[reg_dest_idx + 2] = src_val;
            } else {
                combined_val = dest_val | src_val;
                if ((combined_val & 1) == 0) {
                    if (combined_val == 4 || combined_val == 8 || combined_val == 0xc) {
                        state[reg_dest_idx + 2] = 1;
                    } else if ((combined_val & 4) == 0) {
                        state[reg_dest_idx + 2] = ((combined_val & 8) == 0) ? 2 : 8;
                    } else {
                        state[reg_dest_idx + 2] = 4;
                    }
                } else {
                    state[reg_dest_idx + 2] = 1;
                }
                if ((opcode_prefix != 0x1 && opcode_prefix != 0x0) && ((combined_val & 0xc) != 0)) {
                    state[reg_dest_idx + 2] = 1;
                }
            }
            return 1;
        }
    } else {
        return 0;
    }
}

uint32_t validate_ld(uint32_t *state) {
    uint32_t current_instruction_addr = state[1] * 8 + state[0];
    uint8_t *instruction_bytes = (uint8_t *)(current_instruction_addr + 4);

    if (((*instruction_bytes >> 5) < 2) && ((*instruction_bytes & 0x18) != 0x18)) {
        if ((state[(*(instruction_bytes + 1) >> 4) + 2] & 0xcU) == 0) {
            return 0;
        } else if ((*(instruction_bytes + 1) & 0xf) == 0xf) {
            return 0;
        } else {
            state[(*(instruction_bytes + 1) & 0xf) + 2] = 2;
            return 1;
        }
    } else {
        return 0;
    }
}

uint32_t validate_st(uint32_t *state) {
    uint32_t current_instruction_addr = state[1] * 8 + state[0];
    uint8_t *instruction_bytes = (uint8_t *)(current_instruction_addr + 4);

    if (((*instruction_bytes >> 5) < 2) && ((*instruction_bytes & 0x18) != 0x18)) {
        if ((state[(*(instruction_bytes + 1) & 0xf) + 2] & 8U) == 0) {
            return 0;
        } else if (((*instruction_bytes & 7) == 2) && (state[(*(instruction_bytes + 1) >> 4) + 2] != 2)) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

uint32_t validate_state(uint32_t *state) {
    do {
        // This condition (state[0] <= state[1]) implies state[0] acts as a maximum instruction index
        // for loop termination, which contradicts its use as an instruction base address in other functions.
        // It's a direct translation of the original source's logic.
        if (state[0] <= state[1]) {
            return 0;
        }

        uint8_t *opcode_ptr = (uint8_t *)(state[0] + (uint32_t)state[1] * 8 + 4);
        uint32_t validation_result;

        switch (*opcode_ptr & 7) {
            case 4: // JMP/CALL
                validation_result = validate_jmp(state);
                if (validation_result == 0) {
                    return 0;
                }
                if ((*opcode_ptr & 0xf0) == 0x50) { // BPF_EXIT instruction
                    return 1;
                }
                break;
            case 3: // ALU
                validation_result = validate_alu(state);
                if (validation_result == 0) {
                    return 0;
                }
                break;
            case 0: // LD
                validation_result = validate_ld(state);
                if (validation_result == 0) {
                    return 0;
                }
                break;
            case 1: // ST
            case 2: // ST
                validation_result = validate_st(state);
                if (validation_result == 0) {
                    return 0;
                }
                break;
            default:
                return 0;
        }
        state[1]++; // Increment instruction pointer
    } while (true);
}

void filter_validate(uint32_t initial_base_address) {
    uint32_t state[18];
    state[0] = initial_base_address; // Instruction base address (and max_index for validate_state's loop)
    state[1] = 0; // Current instruction index
    for (uint32_t i = 0; i < 16; ++i) {
        state[i + 2] = 1; // Registers R0-R15 (state[2] to state[17]) default to 1
    }
    state[2] = 4;  // Register R0 (state[2]) set to 4
    state[17] = 8; // Register R15 (state[17]) set to 8
    validate_state(state);
    return;
}