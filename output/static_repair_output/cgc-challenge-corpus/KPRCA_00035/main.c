#include <stdbool.h> // For bool type
#include <stdint.h>  // For uint, undefined4, etc.
#include <stdlib.h>  // For calloc, free
#include <string.h>  // For memcmp

// Define types to match the original snippet's inferred types
typedef uint32_t uint;
typedef uint8_t byte;
typedef uint16_t ushort;
typedef int16_t short_int; // Renamed to avoid conflict with 'short' keyword
typedef uint32_t undefined4;
typedef void (*code)(void); // Generic function pointer type for code

// Forward declarations for functions not provided in the snippet
// These are placeholders to make the code compilable.
// In a real scenario, these would be external functions (e.g., system calls).
int receive(undefined4 param_1, void* buffer, uint size, int* bytes_received) {
    // Placeholder implementation
    if (size > 0) {
        // Simulate receiving some bytes
        *bytes_received = size; 
        return 0; // Success
    }
    *bytes_received = 0;
    return -1; // Failure
}

int transmit(undefined4 param_1, const void* buffer, uint size, undefined4 param_4) {
    // Placeholder implementation
    return 0; // Success
}

// Function: read_n_bytes
uint read_n_bytes(undefined4 param_1, uint param_2, void* param_3, undefined4 param_4) {
    if (param_2 == 0) {
        return 0; // If nothing to read, return 0 bytes read.
    }

    uint total_read = 0;
    while (total_read < param_2) {
        int bytes_received_this_call = 0;
        int iVar1 = receive(param_1, (byte*)param_3 + total_read, param_2 - total_read, &bytes_received_this_call);
        if (iVar1 != 0) {
            return 0xFFFFFFFF; // Error
        }
        if (bytes_received_this_call == 0) {
            return total_read; // End of stream or no more data
        }
        total_read += bytes_received_this_call;
    }
    return total_read;
}

// Function: read_header
uint read_header(undefined4 param_1) {
    undefined4 header_magic = 0;
    if (read_n_bytes(param_1, 4, &header_magic, 0x110aa) == 4) {
        return (header_magic == 0x69564D00); // Return 1 if match, 0 otherwise
    }
    return 0xFFFFFFFF; // Error or not enough bytes
}

// Function: read_flags
undefined4 read_flags(undefined4 param_1, undefined4 *param_2) {
    undefined4 flags = 0;
    if (read_n_bytes(param_1, 4, &flags, 0x1110b) != 4) {
        *param_2 = 1; // Indicate error
    }
    return flags;
}

// Function: init_state
void * init_state(undefined4 param_1, uint param_2) {
    void *__ptr = calloc(1, 0x10040);
    if (!__ptr) {
        return NULL;
    }

    uint uVar1 = param_2 & 0xffff; // Lower 16 bits
    bool error = false;

    if ((param_2 & 0x80000000) != 0) { // Check MSB
        if (read_n_bytes(param_1, 0x40, __ptr, 0x1114a) != 0x40) {
            error = true;
        }
    }

    if (!error && uVar1 != 0) {
        if (read_n_bytes(param_1, uVar1, (byte*)__ptr + 0x40, 0x1114a) != uVar1) {
            error = true;
        }
    }
    
    if (error) {
        free(__ptr);
        __ptr = NULL;
    }
    return __ptr;
}

// Function: dump_regs
bool dump_regs(undefined4 param_1, undefined4 param_2) {
    return transmit(param_1, (const void*)param_2, 0x40, 0) == 0;
}

// Function: process_load
undefined4 process_load(int param_1, byte param_2, byte param_3, ushort param_4) {
    ushort addr = param_4;
    if (param_3 != 0) {
        addr += *(short_int *)(param_1 + (uint)param_3 * 4);
    }
    *(uint *)(param_1 + (uint)param_2 * 4) = (uint)*(byte *)(param_1 + 0x40 + (uint)addr);
    return 0;
}

// Function: process_str
undefined4 process_str(int param_1, byte param_2, byte param_3, ushort param_4) {
    ushort addr = param_4;
    if (param_2 != 0) {
        addr += *(short_int *)(param_1 + (uint)param_2 * 4);
    }
    
    byte val = (param_3 == 0) ? 0 : (byte)*(undefined4 *)(param_1 + (uint)param_3 * 4);
    *(byte *)(param_1 + 0x40 + (uint)addr) = val;
    return 0;
}

// Function: frob
undefined4 frob(undefined4 param_1, int param_2, uint param_3) {
    for (uint i = 0; i < param_3; ++i) {
        *(byte *)(i + param_2) ^= 0x42;
    }
    return 0;
}

// Function: process_sys
undefined4 process_sys(int param_1) {
    typedef int (*sys_call_func)(undefined4, ...); // Generic type for system calls

    sys_call_func func_ptr = NULL;
    undefined4 arg1 = 0; // First argument to the system call
    uint arg2 = 0;     // Second argument
    uint arg3 = 0;     // Third argument

    int sys_call_id = *(int *)(param_1 + 4);

    if (sys_call_id == 0) {
        func_ptr = (sys_call_func)transmit;
        arg1 = 1; // Assuming transmit's first arg is the channel
        arg2 = *(uint *)(param_1 + 8) & 0xffff;
        arg3 = *(uint *)(param_1 + 0xc) & 0xffff;
    } else if (sys_call_id == 1) {
        func_ptr = (sys_call_func)receive;
        arg1 = 0; // Assuming receive's first arg is the channel
        arg2 = *(uint *)(param_1 + 8) & 0xffff;
        arg3 = *(uint *)(param_1 + 0xc) & 0xffff;
    } else {
        // If it's not transmit or receive, assume frob based on original logic
        func_ptr = (sys_call_func)frob;
        arg2 = 0; // frob's second arg (offset in memory)
        arg3 = *(uint *)(param_1 + 0x20); // frob's third arg (length)
    }

    if (func_ptr == (sys_call_func)transmit || func_ptr == (sys_call_func)receive) {
        if (0x10000 < arg3 + arg2 || arg3 == 0) {
            return 0; // Invalid size or length
        }
        func_ptr(arg1, arg2 + 0x40 + param_1, arg3, 0); // Call with appropriate args
    } else if (func_ptr == (sys_call_func)frob) {
        func_ptr(0, arg2 + 0x40 + param_1, arg3, 0); // frob doesn't use the first arg in this context
    } else {
        return 0; // Unknown or unsupported function
    }
    
    return 0;
}


// Function: process_add
undefined4 process_add(int param_1, byte param_2, byte param_3, short_int param_4) {
    int val_r2 = *(int *)(param_1 + (uint)param_2 * 4);
    int add_val = param_4;
    if (param_3 != 0) {
        add_val += *(int *)(param_1 + (uint)param_3 * 4);
    }
    *(int *)(param_1 + (uint)param_2 * 4) = val_r2 + add_val;
    return 0;
}

// Function: process_sub
undefined4 process_sub(int param_1, byte param_2, byte param_3, short_int param_4) {
    int val_r2 = *(int *)(param_1 + (uint)param_2 * 4);
    int sub_val = param_4;
    if (param_3 != 0) {
        sub_val += *(int *)(param_1 + (uint)param_3 * 4);
    }
    *(int *)(param_1 + (uint)param_2 * 4) = val_r2 - sub_val;
    return 0;
}

// Function: process_mul
undefined4 process_mul(int param_1, byte param_2, byte param_3, short_int param_4) {
    int val_r2 = *(int *)(param_1 + (uint)param_2 * 4);
    int mul_val = param_4;
    if (param_3 != 0) {
        mul_val += *(int *)(param_1 + (uint)param_3 * 4);
    }
    *(int *)(param_1 + (uint)param_2 * 4) = val_r2 * mul_val;
    return 0;
}

// Function: process_div
undefined4 process_div(int param_1, byte param_2, byte param_3, short_int param_4) {
    int divisor = param_4;
    if (param_3 != 0) {
        divisor += *(int *)(param_1 + (uint)param_3 * 4);
    }
    
    if (divisor == 0) {
        return 0xffffffff; // Division by zero
    }
    *(uint *)(param_1 + (uint)param_2 * 4) = *(uint *)(param_1 + (uint)param_2 * 4) / (uint)divisor;
    return 0;
}

// Function: process_or
undefined4 process_or(int param_1, byte param_2, byte param_3, short_int param_4) {
    uint or_val = (uint)param_4;
    if (param_3 != 0) {
        or_val |= *(uint *)(param_1 + (uint)param_3 * 4);
    }
    *(uint *)(param_1 + (uint)param_2 * 4) |= or_val;
    return 0;
}

// Function: process_and
undefined4 process_and(int param_1, byte param_2, byte param_3, short_int param_4) {
    uint and_val = (uint)param_4;
    if (param_3 != 0) {
        and_val |= *(uint *)(param_1 + (uint)param_3 * 4); // Original code used OR here, check if intended
    }
    *(uint *)(param_1 + (uint)param_2 * 4) &= and_val;
    return 0;
}

// Function: process_xor
undefined4 process_xor(int param_1, byte param_2, byte param_3, short_int param_4) {
    uint xor_val = (uint)param_4;
    if (param_3 != 0) {
        xor_val |= *(uint *)(param_1 + (uint)param_3 * 4); // Original code used OR here, check if intended
    }
    *(uint *)(param_1 + (uint)param_2 * 4) ^= xor_val;
    return 0;
}

// Function: process_slt
undefined4 process_slt(int param_1, byte param_2, byte param_3, ushort param_4) {
    uint val1 = (param_3 == 0) ? 0 : *(uint *)(param_1 + (uint)param_3 * 4);
    uint val2 = (param_4 == 0) ? 0 : *(uint *)(param_1 + (param_4 & 0xf) * 4);
    *(uint *)(param_1 + (uint)param_2 * 4) = (uint)(val1 < val2);
    return 0;
}

// Function: process_slte
undefined4 process_slte(int param_1, byte param_2, byte param_3, ushort param_4) {
    uint val1 = (param_3 == 0) ? 0 : *(uint *)(param_1 + (uint)param_3 * 4);
    uint val2 = (param_4 == 0) ? 0 : *(uint *)(param_1 + (param_4 & 0xf) * 4);
    *(uint *)(param_1 + (uint)param_2 * 4) = (uint)(val1 <= val2);
    return 0;
}

// Function: handle_inst
undefined4 handle_inst(undefined4 param_1, short_int *param_2) {
    byte opcode = *(byte *)((int)param_2 + 3);
    byte rD = *(byte *)(param_2 + 1) >> 4;
    byte rS = *(byte *)(param_2 + 1) & 0xf;
    short_int immediate = *param_2;

    switch(opcode) {
        case 1:  return process_load(param_1, rD, rS, immediate);
        case 2:  return process_str(param_1, rD, rS, immediate);
        case 3:  return process_sys(param_1);
        case 4:  return process_add(param_1, rD, rS, immediate);
        case 5:  return process_sub(param_1, rD, rS, immediate);
        case 6:  return process_mul(param_1, rD, rS, immediate);
        case 7:  return process_div(param_1, rD, rS, immediate);
        case 8:  return process_or(param_1, rD, rS, immediate);
        case 9:  return process_and(param_1, rD, rS, immediate);
        case 10: return process_xor(param_1, rD, rS, immediate);
        case 0xb: return process_slt(param_1, rD, rS, immediate);
        case 0xc: return process_slte(param_1, rD, rS, immediate);
        default: return 0xffffffff;
    }
}

// Function: read_inst
undefined4 read_inst(undefined4 param_1, undefined4 param_2, undefined4 param_3) {
    read_n_bytes(param_1, 4, (void*)param_3, 0x11b41);
    return handle_inst(param_2, (short_int*)param_3);
}

// Global data placeholder
// Assuming DAT_00014034 is a string literal or similar
const char DAT_00014034[] = "DONE\n";

// Function: main
undefined4 main(void) {
    // The original snippet used a stack variable `local_10 = &stack0x00000004;`
    // This is often a decompiler artifact indicating a stack frame setup.
    // It's not standard C and should be removed.
    // The `transmit(1,&local_1c,4,0);` call with `local_1c = frob;`
    // seems to be an initialization or debug print, sending the address of frob.
    // We'll keep it as is, assuming 'code *' can be cast to 'const void*'.
    
    code *local_1c = (code *)frob; // Store address of frob
    transmit(1, &local_1c, sizeof(local_1c), 0); // Transmit the address of frob

    if (!read_header(0)) { // Check if header read was successful (non-zero implies success)
        return 0xffffffff; // Error
    }

    int error_flag = 0;
    undefined4 flags = read_flags(0, (undefined4*)&error_flag);
    if (error_flag != 0) {
        return 0xffffffff; // Error reading flags
    }

    undefined4 state_ptr = (undefined4)init_state(0, flags);
    if (state_ptr == 0) { // Check if init_state failed
        return 0xffffffff;
    }

    undefined4 instruction_buffer[1]; // Buffer for a 4-byte instruction
    while (true) {
        if (read_inst(0, state_ptr, (undefined4)instruction_buffer) < 0) {
            break; // Error or end of instructions
        }
        if (!dump_regs(1, state_ptr)) {
            break; // Error dumping registers
        }
    }

    transmit(1, DAT_00014034, strlen(DAT_00014034), 0);
    // free(state_ptr); // Original code doesn't free, but good practice.
    return 0;
}