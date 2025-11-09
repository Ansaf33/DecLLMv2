#include <stdio.h>   // For printf
#include <stdint.h>  // For fixed-width integer types (uint8_t, uint16_t, uint32_t, uintptr_t)

// Custom types from original snippet, mapped to standard types
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;

// Define the CPU_State structure based on observed offsets
// This structure maps directly to the memory layout implied by the original code's pointer arithmetic.
typedef struct {
    uintptr_t memory_base; // At offset 0x00. This is the base address for emulated RAM.

    uint8_t _padding_0x04_0x0b[0x0c - 0x04]; // Padding to align registers if needed

    // Registers used in r_8b and r_16b
    // Note: Some 8-bit registers are high/low bytes of 16-bit registers.
    // The offsets are byte offsets from the start of this struct.
    // D (0xc), E (0xd) form DE_word (0xc)
    uint16_t reg_DE_word; // DE register, accessed at offset 0xc
    uint16_t reg_PC;      // Program Counter, accessed at offset 0xe

    uint8_t _padding_0x10_0x2d[0x2e - 0x10];

    uint8_t reg_F;       // Flags register, accessed at offset 0x2e
    uint8_t reg_A;       // Accumulator register, accessed at offset 0x2f

    // B (0x31), C (0x30) form reg_BC (0x30)
    uint16_t reg_BC;      // BC register, accessed at offset 0x30
    // L (0x32), H (0x33) form reg_HL (0x32)
    uint16_t reg_HL;      // HL register, accessed at offset 0x32
    // SPl (0x36), SPh (0x35) form reg_SP (0x36)
    uint16_t reg_SP;      // Stack Pointer, accessed at offset 0x36

    uint8_t _padding_0x38_0x39[0x3a - 0x38];
    uint8_t reg_IME;     // Interrupt Master Enable, accessed at offset 0x3a
    uint8_t _padding_0x3b;
    int32_t halted_flag; // Accessed at offset 0x3c (using int32_t for `int` type)

    uint8_t _padding_0x40_0x4b[0x4c - 0x40];
    int32_t cycles_remaining; // Accessed at offset 0x4c

    // The original code also accesses memory_base + 0xff0f and memory_base + 0xffff
    // These are typically I/O registers mapped into memory.
    // They are not part of the CPU_State struct itself but are accessed indirectly.
    // For the purpose of this code, we assume memory_base points to a region
    // where these addresses are valid.
} CPU_State;

// Function: r_8b - Returns a pointer to an 8-bit register or memory location
// `cpu` is a pointer to the CPU_State structure.
uint8_t *r_8b(CPU_State *cpu, int reg_idx) {
    switch (reg_idx) {
        case 0: return (uint8_t*)cpu + 0x31; // B (high byte of BC)
        case 1: return (uint8_t*)cpu + 0x0c; // D (high byte of DE)
        case 2: return (uint8_t*)cpu + 0x33; // H (high byte of HL)
        case 3: return (uint8_t*)cpu + 0x32; // L (low byte of HL)
        case 4: return (uint8_t*)cpu + 0x35; // SPh (high byte of SP)
        case 5: return (uint8_t*)cpu + 0x0d; // E (low byte of DE)
        case 6: return (uint8_t *)(cpu->memory_base + cpu->reg_DE_word); // (HL) - interpreted as (DE) from original offset 0xd
        case 7: return &cpu->reg_A;          // A (Accumulator)
        default: return NULL;
    }
}

// Function: r_16b - Returns a pointer to a 16-bit register
// `cpu` is a pointer to the CPU_State structure.
uint16_t *r_16b(CPU_State *cpu, int reg_idx) {
    switch (reg_idx) {
        case 0: return &cpu->reg_BC; // BC
        case 1: return &cpu->reg_DE_word; // DE
        case 2: return &cpu->reg_HL; // HL
        case 3: return &cpu->reg_SP; // SP
        default: return NULL;
    }
}

// Function: flags_bitwise
// `cpu_addr` is the base address of the CPU_State structure.
void flags_bitwise(uintptr_t cpu_addr, short value) {
    uint8_t *flags = (uint8_t*)(cpu_addr + 0x2e); // Pointer to F register
    *flags = 0; // Clear flags
    if (value == 0) {
        *flags |= 0x80; // Set Z flag
    }
    // Only Z flag is explicitly handled by this function based on original logic.
    return;
}

// Function: do_add_8b
// `cpu_addr` is the base address of the CPU_State structure.
ushort do_add_8b(uintptr_t cpu_addr, byte param_2, byte param_3, byte param_4) {
    ushort result = (ushort)param_4 + (ushort)param_2 + (ushort)param_3;
    uint8_t *flags = (uint8_t*)(cpu_addr + 0x2e);

    *flags = 0; // Clear all flags initially

    if ((uint8_t)result == 0) { // Check Z flag
        *flags |= 0x80;
    }
    if (result > 0xff) { // Check C flag (carry out of bit 7)
        *flags |= 0x10;
    }
    // Check H flag (carry out of bit 3)
    if (((param_3 & 0xf) + (param_2 & 0xf) & 0x10) != 0 ||
        (((param_3 + param_2) & 0xf) + (param_4 & 0xf) & 0x10) != 0) {
        *flags |= 0x20;
    }
    return result;
}

// Function: do_add_16b
// `cpu_addr` is the base address of the CPU_State structure.
uint do_add_16b(uintptr_t cpu_addr, ushort param_2, ushort param_3, byte param_4) {
    uint result = (uint)param_4 + (uint)param_2 + (uint)param_3;
    uint8_t *flags = (uint8_t*)(cpu_addr + 0x2e);

    *flags &= 0x8f; // Clear N, H flags, preserve Z, C

    if (result > 0xffff) { // Check C flag (carry out of bit 15)
        *flags |= 0x10;
    }
    // Check H flag (carry out of bit 11)
    if (((param_3 & 0xfff) + (param_2 & 0xfff) & 0x1000) != 0) {
        *flags |= 0x20;
    }
    return result;
}

// Function: do_sub_8b
// `cpu_addr` is the base address of the CPU_State structure.
ushort do_sub_8b(uintptr_t cpu_addr, byte param_2, byte param_3, byte param_4) {
    ushort result = ((ushort)param_2 - (ushort)param_3) - (ushort)param_4;
    uint8_t *flags = (uint8_t*)(cpu_addr + 0x2e);

    *flags = 0x40; // Set N flag (subtraction)

    if ((uint8_t)result == 0) { // Check Z flag
        *flags |= 0x80;
    }
    if (result > 0xff) { // Check C flag (borrow)
        *flags |= 0x10;
    }
    // Check H flag (borrow from bit 4)
    if (((param_2 & 0xf) - (param_3 & 0xf) & 0x10) != 0 ||
        (((param_2 - param_3) & 0xf) - (param_4 & 0xf) & 0x10) != 0) {
        *flags |= 0x20;
    }
    return result;
}

// Function: do_call
// `cpu` is a pointer to the CPU_State structure.
void do_call(CPU_State *cpu, undefined2 target_addr) {
    cpu->reg_SP -= 2; // Decrement SP by 2
    // Push current PC onto stack
    *(uint16_t *)(cpu->memory_base + cpu->reg_SP) = cpu->reg_PC;
    cpu->reg_PC = target_addr; // Set PC to target address
    return;
}

// Function: do_ret
// `cpu` is a pointer to the CPU_State structure.
void do_ret(CPU_State *cpu) {
    // Pop PC from stack
    cpu->reg_PC = *(uint16_t *)(cpu->memory_base + cpu->reg_SP);
    cpu->reg_SP += 2; // Increment SP by 2
    return;
}

// Function: step_prefix
// `cpu` is a pointer to the CPU_State structure.
undefined4 step_prefix(CPU_State *cpu) {
    uint8_t opcode_byte = *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1);
    uint8_t *reg_ptr = r_8b(cpu, opcode_byte & 7);
    uint8_t bit_idx = (opcode_byte >> 3) & 7;
    uint8_t reg_val = *reg_ptr;
    uint32_t cycles = 8; // Default cycles

    if ((opcode_byte & 0xf8) == 0) { // RLC r8
        *reg_ptr = (reg_val << 1) | (reg_val >> 7);
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
        if ((reg_val & 0x80) != 0) { // Check if original MSB was set
            cpu->reg_F |= 0x10; // Set C flag
        }
    } else if ((opcode_byte & 0xf8) == 8) { // RRC r8
        *reg_ptr = (reg_val >> 1) | ((reg_val & 1) << 7);
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
        if ((reg_val & 1) != 0) { // Check if original LSB was set
            cpu->reg_F |= 0x10; // Set C flag
        }
    } else if ((opcode_byte & 0xf8) == 0x10) { // RL r8
        *reg_ptr = (reg_val << 1) | ((cpu->reg_F & 0x10) != 0); // Rotate left through carry
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
        if ((reg_val & 0x80) != 0) { // Check if original MSB was set
            cpu->reg_F |= 0x10; // Set C flag
        }
    } else if ((opcode_byte & 0xf8) == 0x18) { // RR r8
        *reg_ptr = (reg_val >> 1) | ((cpu->reg_F & 0x10) << 3); // Rotate right through carry
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
        if ((reg_val & 1) != 0) { // Check if original LSB was set
            cpu->reg_F |= 0x10; // Set C flag
        }
    } else if ((opcode_byte & 0xf8) == 0x20) { // SLA r8
        *reg_ptr = reg_val << 1;
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
        if ((reg_val & 0x80) != 0) { // Check if original MSB was set
            cpu->reg_F |= 0x10; // Set C flag
        }
    } else if ((opcode_byte & 0xf8) == 0x28) { // SRA r8
        *reg_ptr = (reg_val >> 1) | (reg_val & 0x80); // Shift right arithmetic (preserve MSB)
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
        if ((reg_val & 1) != 0) { // Check if original LSB was set
            cpu->reg_F |= 0x10; // Set C flag
        }
    } else if ((opcode_byte & 0xf8) == 0x30) { // SWAP r8
        *reg_ptr = (reg_val >> 4) | (reg_val << 4);
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
    } else if ((opcode_byte & 0xf8) == 0x38) { // SRL r8
        *reg_ptr = reg_val >> 1;
        flags_bitwise((uintptr_t)cpu, *reg_ptr);
        if ((reg_val & 1) != 0) { // Check if original LSB was set
            cpu->reg_F |= 0x10; // Set C flag
        }
    } else if ((opcode_byte & 0xc0) == 0x40) { // BIT bit, r8
        cpu->reg_F &= 0x3f; // Clear N, H flags
        cpu->reg_F |= 0x20; // Set H flag
        if (!((reg_val >> bit_idx) & 1)) { // Check if bit is 0
            cpu->reg_F |= 0x80; // Set Z flag
        }
    } else if ((opcode_byte & 0xc0) == 0x80) { // RES bit, r8
        *reg_ptr &= ~(1 << bit_idx);
    } else { // (opcode_byte & 0xc0) == 0xc0 (SET bit, r8)
        *reg_ptr |= (1 << bit_idx);
    }
    cpu->reg_PC += 2; // Prefix opcodes are 2 bytes
    if ((opcode_byte & 7) == 6) { // If target is (HL) / (DE)
        cycles = 0x10;
    }
    return cycles;
}

// Function: step_misc
// `cpu` is a pointer to the CPU_State structure.
undefined4 step_misc(CPU_State *cpu) {
    uint8_t opcode = *(uint8_t *)(cpu->memory_base + cpu->reg_PC);
    uint32_t cycles = 0; // Default to 0, meaning not handled by this function

    switch (opcode) {
        case 0x00: // NOP
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        case 0x07: { // RLCA
            uint8_t old_A = cpu->reg_A;
            cpu->reg_A = (old_A << 1) | (old_A >> 7);
            cpu->reg_F = 0; // Clear all flags
            if ((old_A & 0x80) != 0) { // Check if original MSB was set
                cpu->reg_F |= 0x10; // Set C flag
            }
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        }
        case 0x08: // LD (a16), SP
            *(uint16_t *)(cpu->memory_base + *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1)) = cpu->reg_SP;
            cpu->reg_PC += 3;
            cycles = 0x14;
            break;
        case 0x0f: { // RRCA
            uint8_t old_A = cpu->reg_A;
            cpu->reg_A = (old_A >> 1) | ((old_A & 1) << 7);
            cpu->reg_F = 0; // Clear all flags
            if ((old_A & 1) != 0) { // Check if original LSB was set
                cpu->reg_F |= 0x10; // Set C flag
            }
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        }
        case 0x10: // STOP
            cpu->reg_PC += 1;
            cycles = 0; // CPU stops, no cycles consumed for next instruction
            break;
        case 0x17: { // RLA
            uint8_t old_A = cpu->reg_A;
            cpu->reg_A = (old_A << 1) | ((cpu->reg_F & 0x10) != 0); // Rotate left through carry
            cpu->reg_F = 0; // Clear all flags
            if ((old_A & 0x80) != 0) { // Check if original MSB was set
                cpu->reg_F |= 0x10; // Set C flag
            }
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        }
        case 0x18: // JR e8
            cpu->reg_PC += 2; // Advance PC past opcode and operand
            cpu->reg_PC += (int8_t)*(uint8_t *)(cpu->memory_base + cpu->reg_PC - 1); // Add signed 8-bit offset
            cycles = 0xc;
            break;
        case 0x1f: { // RRA
            uint8_t old_A = cpu->reg_A;
            cpu->reg_A = (old_A >> 1) | ((cpu->reg_F & 0x10) << 3); // Rotate right through carry
            cpu->reg_F = 0; // Clear all flags
            if ((old_A & 1) != 0) { // Check if original LSB was set
                cpu->reg_F |= 0x10; // Set C flag
            }
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        }
        case 0x20: // JR NZ, e8
            cpu->reg_PC += 2;
            if (!((cpu->reg_F & 0x80) != 0)) { // If Z flag is not set
                cpu->reg_PC += (int8_t)*(uint8_t *)(cpu->memory_base + cpu->reg_PC - 1);
                cycles = 0xc;
            } else {
                cycles = 8;
            }
            break;
        case 0x27: { // DAA (Decimal Adjust Accumulator)
            uint8_t old_A = cpu->reg_A;
            uint8_t old_F = cpu->reg_F;
            cpu->reg_F &= ~0x20; // Clear H flag
            if (!((old_F & 0x40) != 0)) { // If not a subtraction
                if ((cpu->reg_A > 0x99) || ((old_F & 0x10) != 0)) {
                    cpu->reg_A += 0x60;
                    cpu->reg_F |= 0x10; // Set C flag
                }
                if (((cpu->reg_A & 0xf) > 9) || ((old_F & 0x20) != 0)) {
                    cpu->reg_A += 6;
                }
            } else { // If subtraction
                if ((old_F & 0x20) != 0) {
                    cpu->reg_A -= 6;
                }
                if ((old_F & 0x10) != 0) {
                    cpu->reg_A -= 0x60;
                }
            }
            cpu->reg_F &= ~0x80; // Clear Z flag
            if (cpu->reg_A == 0) {
                cpu->reg_F |= 0x80; // Set Z flag if result is 0
            }
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        }
        case 0x28: // JR Z, e8
            cpu->reg_PC += 2;
            if ((cpu->reg_F & 0x80) != 0) { // If Z flag is set
                cpu->reg_PC += (int8_t)*(uint8_t *)(cpu->memory_base + cpu->reg_PC - 1);
                cycles = 0xc;
            } else {
                cycles = 8;
            }
            break;
        case 0x2f: // CPL (Complement Accumulator)
            cpu->reg_A = ~cpu->reg_A;
            cpu->reg_F |= 0x60; // Set N and H flags
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        case 0x30: // JR NC, e8
            cpu->reg_PC += 2;
            if (!((cpu->reg_F & 0x10) != 0)) { // If C flag is not set
                cpu->reg_PC += (int8_t)*(uint8_t *)(cpu->memory_base + cpu->reg_PC - 1);
                cycles = 0xc;
            } else {
                cycles = 8;
            }
            break;
        case 0x37: // SCF (Set Carry Flag)
            cpu->reg_F &= ~0x60; // Clear N and H flags
            cpu->reg_F |= 0x10; // Set C flag
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        case 0x38: // JR C, e8
            cpu->reg_PC += 2;
            if ((cpu->reg_F & 0x10) != 0) { // If C flag is set
                cpu->reg_PC += (int8_t)*(uint8_t *)(cpu->memory_base + cpu->reg_PC - 1);
                cycles = 0xc;
            } else {
                cycles = 8;
            }
            break;
        case 0x3f: // CCF (Complement Carry Flag)
            cpu->reg_F &= ~0x60; // Clear N and H flags
            if (!((cpu->reg_F & 0x10) != 0)) { // If C flag was not set
                cpu->reg_F |= 0x10; // Set C flag
            } else {
                cpu->reg_F &= ~0x10; // Clear C flag
            }
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        case 0x76: // HALT
            cpu->halted_flag = 1;
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        case 0xc0: // RET NZ
            if (!((cpu->reg_F & 0x80) != 0)) { // If Z flag is not set
                do_ret(cpu);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 1;
                cycles = 8;
            }
            break;
        case 0xc2: // JP NZ, a16
            if (!((cpu->reg_F & 0x80) != 0)) { // If Z flag is not set
                cpu->reg_PC = *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 3;
                cycles = 0xc;
            }
            break;
        case 0xc3: // JP a16
            cpu->reg_PC = *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1);
            cycles = 0x10;
            break;
        case 0xc4: // CALL NZ, a16
            cpu->reg_PC += 3;
            if (!((cpu->reg_F & 0x80) != 0)) { // If Z flag is not set
                do_call(cpu, *(uint16_t *)(cpu->memory_base + cpu->reg_PC - 2));
                cycles = 0x18;
            } else {
                cycles = 0xc;
            }
            break;
        case 0xc6: // ADD A, n8
            cpu->reg_A = do_add_8b((uintptr_t)cpu, cpu->reg_A, *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1), 0);
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        case 0xc8: // RET Z
            if ((cpu->reg_F & 0x80) != 0) { // If Z flag is set
                do_ret(cpu);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 1;
                cycles = 8;
            }
            break;
        case 0xc9: // RET
            do_ret(cpu);
            cycles = 0x10;
            break;
        case 0xca: // JP Z, a16
            if ((cpu->reg_F & 0x80) != 0) { // If Z flag is set
                cpu->reg_PC = *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 3;
                cycles = 0xc;
            }
            break;
        case 0xcb: // PREFIX CB
            cycles = step_prefix(cpu);
            break;
        case 0xcc: // CALL Z, a16
            cpu->reg_PC += 3;
            if ((cpu->reg_F & 0x80) != 0) { // If Z flag is set
                do_call(cpu, *(uint16_t *)(cpu->memory_base + cpu->reg_PC - 2));
                cycles = 0x18;
            } else {
                cycles = 0xc;
            }
            break;
        case 0xcd: // CALL a16
            cpu->reg_PC += 3;
            do_call(cpu, *(uint16_t *)(cpu->memory_base + cpu->reg_PC - 2));
            cycles = 0x18;
            break;
        case 0xce: // ADC A, n8
            cpu->reg_A = do_add_8b((uintptr_t)cpu, cpu->reg_A, *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1), ((cpu->reg_F & 0x10) != 0));
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        case 0xd0: // RET NC
            if (!((cpu->reg_F & 0x10) != 0)) { // If C flag is not set
                do_ret(cpu);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 1;
                cycles = 8;
            }
            break;
        case 0xd2: // JP NC, a16
            if (!((cpu->reg_F & 0x10) != 0)) { // If C flag is not set
                cpu->reg_PC = *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 3;
                cycles = 0xc;
            }
            break;
        case 0xd4: // CALL NC, a16
            cpu->reg_PC += 3;
            if (!((cpu->reg_F & 0x10) != 0)) { // If C flag is not set
                do_call(cpu, *(uint16_t *)(cpu->memory_base + cpu->reg_PC - 2));
                cycles = 0x18;
            } else {
                cycles = 0xc;
            }
            break;
        case 0xd6: // SUB A, n8
            cpu->reg_A = do_sub_8b((uintptr_t)cpu, cpu->reg_A, *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1), 0);
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        case 0xd8: // RET C
            if ((cpu->reg_F & 0x10) != 0) { // If C flag is set
                do_ret(cpu);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 1;
                cycles = 8;
            }
            break;
        case 0xd9: // RETI
            do_ret(cpu);
            cpu->reg_IME = 1; // Enable interrupts immediately
            cycles = 0x10;
            break;
        case 0xda: // JP C, a16
            if ((cpu->reg_F & 0x10) != 0) { // If C flag is set
                cpu->reg_PC = *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1);
                cycles = 0x10;
            } else {
                cpu->reg_PC += 3;
                cycles = 0xc;
            }
            break;
        case 0xdc: // CALL C, a16
            cpu->reg_PC += 3;
            if ((cpu->reg_F & 0x10) != 0) { // If C flag is set
                do_call(cpu, *(uint16_t *)(cpu->memory_base + cpu->reg_PC - 2));
                cycles = 0x18;
            } else {
                cycles = 0xc;
            }
            break;
        case 0xde: // SBC A, n8
            cpu->reg_A = do_sub_8b((uintptr_t)cpu, cpu->reg_A, *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1), ((cpu->reg_F & 0x10) != 0));
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        case 0xe0: // LDH (a8), A
            *(uint8_t *)(cpu->memory_base + 0xff00 + *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1)) = cpu->reg_A;
            cpu->reg_PC += 2;
            cycles = 0xc;
            break;
        case 0xe2: // LD (C), A
            *(uint8_t *)(cpu->memory_base + 0xff00 + ((uint8_t*)&cpu->reg_BC)[0]) = cpu->reg_A; // C is low byte of BC
            cpu->reg_PC += 1;
            cycles = 8;
            break;
        case 0xe6: // AND A, n8
            cpu->reg_A &= *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1);
            flags_bitwise((uintptr_t)cpu, cpu->reg_A);
            cpu->reg_F |= 0x20; // Set H flag
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        case 0xe8: { // ADD SP, e8
            int8_t immediate_val = *(int8_t *)(cpu->memory_base + cpu->reg_PC + 1);
            cpu->reg_SP = do_add_16b((uintptr_t)cpu, cpu->reg_SP, immediate_val, 0);
            cpu->reg_F &= ~0x70; // Clear Z, N, H flags
            cpu->reg_PC += 2;
            cycles = 0x10;
            break;
        }
        case 0xe9: // JP (HL)
            cpu->reg_PC = cpu->reg_HL;
            cycles = 4;
            break;
        case 0xea: // LD (a16), A
            *(uint8_t *)(cpu->memory_base + *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1)) = cpu->reg_A;
            cpu->reg_PC += 3;
            cycles = 0x10;
            break;
        case 0xee: // XOR A, n8
            cpu->reg_A ^= *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1);
            flags_bitwise((uintptr_t)cpu, cpu->reg_A);
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        case 0xf0: // LDH A, (a8)
            cpu->reg_A = *(uint8_t *)(cpu->memory_base + 0xff00 + *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1));
            cpu->reg_PC += 2;
            cycles = 0xc;
            break;
        case 0xf2: // LD A, (C)
            cpu->reg_A = *(uint8_t *)(cpu->memory_base + 0xff00 + ((uint8_t*)&cpu->reg_BC)[0]); // C is low byte of BC
            cpu->reg_PC += 1;
            cycles = 8;
            break;
        case 0xf3: // DI (Disable Interrupts)
            cpu->reg_IME = 0; // Disable interrupts
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        case 0xf6: // OR A, n8
            cpu->reg_A |= *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1);
            flags_bitwise((uintptr_t)cpu, cpu->reg_A);
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        case 0xf8: { // LD HL, SP+e8
            int8_t immediate_val = *(int8_t *)(cpu->memory_base + cpu->reg_PC + 1);
            cpu->reg_HL = do_add_16b((uintptr_t)cpu, cpu->reg_SP, immediate_val, 0);
            cpu->reg_F &= ~0x70; // Clear Z, N, H flags
            cpu->reg_PC += 2;
            cycles = 0xc;
            break;
        }
        case 0xf9: // LD SP, HL
            cpu->reg_SP = cpu->reg_HL;
            cpu->reg_PC += 1;
            cycles = 8;
            break;
        case 0xfa: // LD A, (a16)
            cpu->reg_A = *(uint8_t *)(cpu->memory_base + *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1));
            cpu->reg_PC += 3;
            cycles = 0x10;
            break;
        case 0xfb: // EI (Enable Interrupts)
            cpu->reg_IME = 1; // Enable interrupts (effect is delayed by one instruction)
            cpu->reg_PC += 1;
            cycles = 4;
            break;
        case 0xfe: // CP A, n8
            do_sub_8b((uintptr_t)cpu, cpu->reg_A, *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1), 0);
            cpu->reg_PC += 2;
            cycles = 8;
            break;
        default:
            // If opcode not handled by step_misc, cycles remains 0.
            break;
    }
    return cycles;
}

// Function: step
// `cpu` is a pointer to the CPU_State structure.
int step(CPU_State *cpu) {
    uint8_t opcode = *(uint8_t *)(cpu->memory_base + cpu->reg_PC);
    int cycles = step_misc(cpu);
    if (cycles != 0) {
        return cycles; // Opcode handled by step_misc
    }

    // Opcodes not handled by step_misc
    if ((opcode & 0xC7) == 2) { // Opcodes like LD (BC), A; LD A, (BC); LD (DE), A; LD A, (DE); INC/DEC DE
        uint16_t *target_reg_ptr;
        uint8_t reg_group = (opcode & 0x30); // 0x00 for BC, 0x10 for DE, 0x20 for HL, 0x30 for SP (but HL/SP not used directly here)

        if (reg_group == 0x00) { // (BC)
            target_reg_ptr = &cpu->reg_BC;
        } else if (reg_group == 0x10) { // (DE)
            target_reg_ptr = &cpu->reg_DE_word;
        } else { // This block covers 0x20 and 0x30, which are for HL/SP.
                 // However, the original code's interpretation for these groups
                 // was `param_1 + 0xd` which is `&cpu->reg_DE_word`.
                 // This seems like a specific GB instruction interpretation where
                 // LD (HL+), A or LD (HL-), A might use DE for some reason,
                 // or it's a decompiler misinterpretation. Sticking to original logic.
            target_reg_ptr = &cpu->reg_DE_word;
        }

        if ((opcode & 8) == 0) { // LD (r16), A (write A to memory pointed by r16)
            *(uint8_t *)(cpu->memory_base + *target_reg_ptr) = cpu->reg_A;
        } else { // LD A, (r16) (read from memory pointed by r16 to A)
            cpu->reg_A = *(uint8_t *)(cpu->memory_base + *target_reg_ptr);
        }

        if (reg_group == 0x20) { // INC (HL) or INC DE (if target_reg_ptr is DE)
            *target_reg_ptr += 1; // Increment the 16-bit register
        } else if (reg_group == 0x30) { // DEC (HL) or DEC DE (if target_reg_ptr is DE)
            *target_reg_ptr -= 1; // Decrement the 16-bit register
        }
        cpu->reg_PC += 1;
        return 8; // Cycles for LD (r16), A or LD A, (r16)
    }

    if ((opcode & 0xC7) == 4) { // INC r8 (e.g., INC B, INC C, ...)
        uint8_t reg_idx = (opcode >> 3) & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        uint8_t old_val = *reg_ptr;
        *reg_ptr += 1;
        cpu->reg_F &= 0x1f; // Clear Z, N, H flags
        if (*reg_ptr == 0) {
            cpu->reg_F |= 0x80; // Set Z flag
        }
        if (((*reg_ptr ^ old_val) & 0x10) != 0) { // Check for H flag (carry from bit 3 to bit 4)
            cpu->reg_F |= 0x20;
        }
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 0xc : 4; // 0xc for (HL), 4 for register
    }
    if ((opcode & 0xC7) == 5) { // DEC r8
        uint8_t reg_idx = (opcode >> 3) & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        uint8_t old_val = *reg_ptr;
        *reg_ptr -= 1;
        cpu->reg_F &= 0x1f; // Clear Z, N, H flags
        cpu->reg_F |= 0x40; // Set N flag
        if (*reg_ptr == 0) {
            cpu->reg_F |= 0x80; // Set Z flag
        }
        if (((*reg_ptr ^ old_val) & 0x10) != 0) { // Check for H flag (borrow from bit 4 to bit 3)
            cpu->reg_F |= 0x20;
        }
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 0xc : 4;
    }
    if ((opcode & 0xC7) == 6) { // LD r8, n8
        uint8_t reg_idx = (opcode >> 3) & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        *reg_ptr = *(uint8_t *)(cpu->memory_base + cpu->reg_PC + 1); // Get immediate byte
        cpu->reg_PC += 2;
        return (reg_idx == 6) ? 0xc : 8;
    }
    if ((opcode & 0xCF) == 1) { // LD r16, n16 (e.g., LD BC, nn)
        uint8_t reg_idx = (opcode >> 4) & 3;
        uint16_t *reg_ptr = r_16b(cpu, reg_idx);
        *reg_ptr = *(uint16_t *)(cpu->memory_base + cpu->reg_PC + 1); // Get immediate word
        cpu->reg_PC += 3;
        return 0xc;
    }
    if ((opcode & 0xCF) == 3) { // INC r16
        uint8_t reg_idx = (opcode >> 4) & 3;
        uint16_t *reg_ptr = r_16b(cpu, reg_idx);
        *reg_ptr += 1;
        cpu->reg_PC += 1;
        return 8;
    }
    if ((opcode & 0xCF) == 9) { // ADD HL, r16
        uint8_t reg_idx = (opcode >> 4) & 3;
        uint16_t *reg_ptr = r_16b(cpu, reg_idx);
        cpu->reg_HL = do_add_16b((uintptr_t)cpu, cpu->reg_HL, *reg_ptr, 0);
        cpu->reg_PC += 1;
        return 8;
    }
    if ((opcode & 0xCF) == 0xB) { // DEC r16
        uint8_t reg_idx = (opcode >> 4) & 3;
        uint16_t *reg_ptr = r_16b(cpu, reg_idx);
        *reg_ptr -= 1;
        cpu->reg_PC += 1;
        return 8;
    }
    if ((opcode & 0xC0) == 0x40) { // LD r8, r8
        uint8_t dest_reg_idx = (opcode >> 3) & 7;
        uint8_t src_reg_idx = opcode & 7;
        uint8_t *dest_reg_ptr = r_8b(cpu, dest_reg_idx);
        uint8_t *src_reg_ptr = r_8b(cpu, src_reg_idx);
        *dest_reg_ptr = *src_reg_ptr;
        cpu->reg_PC += 1;
        return ((src_reg_idx == 6) || (dest_reg_idx == 6)) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0x80) { // ADD A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        cpu->reg_A = do_add_8b((uintptr_t)cpu, cpu->reg_A, *reg_ptr, 0);
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0x88) { // ADC A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        cpu->reg_A = do_add_8b((uintptr_t)cpu, cpu->reg_A, *reg_ptr, ((cpu->reg_F & 0x10) != 0));
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0x90) { // SUB A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        cpu->reg_A = do_sub_8b((uintptr_t)cpu, cpu->reg_A, *reg_ptr, 0);
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0x98) { // SBC A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        cpu->reg_A = do_sub_8b((uintptr_t)cpu, cpu->reg_A, *reg_ptr, ((cpu->reg_F & 0x10) != 0));
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0xA0) { // AND A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        cpu->reg_A &= *reg_ptr;
        flags_bitwise((uintptr_t)cpu, cpu->reg_A);
        cpu->reg_F |= 0x20; // Set H flag
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0xA8) { // XOR A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        cpu->reg_A ^= *reg_ptr;
        flags_bitwise((uintptr_t)cpu, cpu->reg_A);
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0xB0) { // OR A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        cpu->reg_A |= *reg_ptr;
        flags_bitwise((uintptr_t)cpu, cpu->reg_A);
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xF8) == 0xB8) { // CP A, r8
        uint8_t reg_idx = opcode & 7;
        uint8_t *reg_ptr = r_8b(cpu, reg_idx);
        do_sub_8b((uintptr_t)cpu, cpu->reg_A, *reg_ptr, 0); // CP performs SUB but discards result
        cpu->reg_PC += 1;
        return (reg_idx == 6) ? 8 : 4;
    }
    if ((opcode & 0xC7) == 0xC7) { // RST n
        cpu->reg_PC += 1;
        do_call(cpu, opcode & 0x38); // RST vector is opcode & 0x38 (00H, 08H, 10H, etc.)
        return 0x10;
    }
    if ((opcode & 0xCF) == 0xC1) { // POP r16
        uint8_t reg_idx = (opcode >> 4) & 3;
        uint16_t *reg_ptr;
        if (reg_idx == 3) { // AF (special case for F)
            reg_ptr = (uint16_t*)((uint8_t*)cpu + 0x2e); // Points to F, A is before it (0x2f)
        } else {
            reg_ptr = r_16b(cpu, reg_idx);
        }
        *reg_ptr = *(uint16_t *)(cpu->memory_base + cpu->reg_SP); // Read word from stack
        if (reg_idx == 3) { // AF, lower 4 bits of F are always zero
            cpu->reg_F &= 0xF0;
        }
        cpu->reg_SP += 2; // SP += 2
        cpu->reg_PC += 1; // PC += 1
        return 0x10;
    }
    if ((opcode & 0xCF) == 0xC5) { // PUSH r16
        uint8_t reg_idx = (opcode >> 4) & 3;
        uint16_t *reg_ptr;
        if (reg_idx == 3) { // AF
            reg_ptr = (uint16_t*)((uint8_t*)cpu + 0x2e);
        } else {
            reg_ptr = r_16b(cpu, reg_idx);
        }
        cpu->reg_SP -= 2; // SP -= 2
        *(uint16_t *)(cpu->memory_base + cpu->reg_SP) = *reg_ptr; // Write word to stack
        cpu->reg_PC += 1; // PC += 1
        return 0x10;
    }

    printf("Invalid opcode %02X\n", (uint32_t)opcode);
    return 0; // Indicate error
}

// Function: cpu_interrupt
// `cpu` is a pointer to the CPU_State structure.
void cpu_interrupt(CPU_State *cpu, byte interrupt_bit) {
    // Assuming 0xff0f is the Interrupt Flag (IF) register in emulated memory
    *(uint8_t *)(cpu->memory_base + 0xff0f) |= (1 << (interrupt_bit & 0x1f));
    return;
}

// Function: do_interrupt
// `cpu` is a pointer to the CPU_State structure.
void do_interrupt(CPU_State *cpu, short interrupt_vector_idx) {
    // Clear the specific interrupt bit in IF register
    *(uint8_t *)(cpu->memory_base + 0xff0f) &= ~(1 << (interrupt_vector_idx & 0x1f));
    cpu->reg_IME = 0; // Disable interrupts
    do_call(cpu, (interrupt_vector_idx + 8) * 8); // Call interrupt handler
    return;
}

// Function: check_interrupts
// `cpu` is a pointer to the CPU_State structure.
void check_interrupts(CPU_State *cpu) {
    // Assuming 0xff0f is IF and 0xffff is IE (Interrupt Enable)
    uint8_t if_reg = *(uint8_t *)(cpu->memory_base + 0xff0f);
    uint8_t ie_reg = *(uint8_t *)(cpu->memory_base + 0xffff);
    uint8_t pending_interrupts = if_reg & ie_reg;

    if (pending_interrupts != 0) {
        cpu->halted_flag = 0; // Exit HALT state if interrupts are pending
    }

    if ((cpu->reg_IME & 1) != 0) { // If IME (Interrupt Master Enable) is active
        if ((pending_interrupts & 1) != 0) { // VBlank (bit 0)
            do_interrupt(cpu, 0);
        } else if ((pending_interrupts & 2) != 0) { // LCD STAT (bit 1)
            do_interrupt(cpu, 1);
        } else if ((pending_interrupts & 4) != 0) { // Timer (bit 2)
            do_interrupt(cpu, 2);
        } else if ((pending_interrupts & 8) != 0) { // Serial (bit 3)
            do_interrupt(cpu, 3);
        } else if ((pending_interrupts & 0x10) != 0) { // Joypad (bit 4)
            do_interrupt(cpu, 4);
        }
    }
    return;
}

// Function: cpu_tick
// `cpu_addr` is the base address of the CPU_State structure.
uint32_t cpu_tick(uintptr_t cpu_addr) {
    CPU_State *cpu = (CPU_State*)cpu_addr; // Cast back to CPU_State*

    // Handle IME delay
    if ((char)cpu->reg_IME < '\0') { // If bit 7 is set (IME delay active)
        cpu->reg_IME &= 0x7f; // Clear bit 7
    } else {
        if ((cpu->reg_IME & 0x40) != 0) { // If bit 6 is set (IME schedule enable)
            cpu->reg_IME &= ~0x01; // Clear IME enable bit
        }
        if ((cpu->reg_IME & 0x20) != 0) { // If bit 5 is set (IME schedule disable)
            cpu->reg_IME |= 0x01; // Set IME enable bit
        }
        cpu->reg_IME &= 0x9f; // Clear bits 5 and 6
    }
    // Note: The IME flag logic here is unusual, typically a simple 1-instruction delay.
    // This looks like a custom interrupt enable/disable scheduling.
    // Assuming the original logic is intended.

    check_interrupts(cpu);

    if (cpu->cycles_remaining < 2) { // If no cycles left from previous instruction
        if (cpu->halted_flag == 0) { // If not halted
            int instruction_cycles = step(cpu);
            if (instruction_cycles == 0) {
                return 0; // Indicate error/halt
            }
            // Original code: if (iVar1 < 0) { iVar1 = iVar1 + 3; }
            // This suggests special handling for negative cycle values, possibly indicating an error.
            // Assuming instruction_cycles is always positive or 0 for error.
            cpu->cycles_remaining = instruction_cycles >> 2; // Divide by 4, assuming M-cycles vs T-cycles
        }
    } else {
        cpu->cycles_remaining -= 1;
    }
    return 1; // Indicate success/continue
}