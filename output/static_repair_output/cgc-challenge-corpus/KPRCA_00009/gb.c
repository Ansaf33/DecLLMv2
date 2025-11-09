#include <stdio.h>    // For printf
#include <stdlib.h>   // For malloc
#include <string.h>   // For memcpy
#include <stdint.h>   // For uint8_t, uint16_t, uint32_t, uintptr_t
#include <ctype.h>    // For isupper

// Define custom types based on typical disassembler output
// undefined is typically 1 byte, undefined2 is 2 bytes, undefined4 is 4 bytes.
// Assuming a 32-bit architecture where pointers are 4 bytes.
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint8_t byte; // Renaming for clarity as `byte` is used in snippet

// Global variable definition (assuming it's a 32-bit unsigned integer)
static uint32_t _DAT_0001745c = 0; // Initialize to 0 or appropriate default

// --- Stub Functions ---
// The original `allocate` returns 0 on success, non-zero on failure.
// `out_ptr_val` is a pointer to a uint32_t where the allocated address will be stored.
int allocate(size_t size, int unused1, uint32_t *out_ptr_val, uint32_t unused2) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        *out_ptr_val = 0; // Store 0 (NULL) on failure
        return 1;         // Indicate failure
    }
    *out_ptr_val = (uint32_t)ptr; // Store the allocated address
    return 0;                     // Indicate success
}

// Stubs for functions not provided in the snippet
int cpu_tick(uint32_t *param_1) {
    (void)param_1; // Suppress unused parameter warning
    return 1; // Assume success for compilation
}

int lcd_tick(uint32_t *param_1) {
    (void)param_1; // Suppress unused parameter warning
    return 1; // Assume success for compilation
}

void cpu_interrupt(uint32_t *param_1, int interrupt_num) {
    (void)param_1;      // Suppress unused parameter warning
    (void)interrupt_num; // Suppress unused parameter warning
}
// --- End Stub Functions ---


// Function: gb_init
undefined4 gb_init(undefined4 *param_1) {
    const undefined4 constant_val = 0x13918;

    if (allocate(0x10001, 0, &param_1[0], constant_val) != 0) {
        return 0; // Allocation for param_1[0] failed
    }
    param_1[1] = param_1[0]; // Copy the allocated pointer from param_1[0] to param_1[1]

    if (allocate(0x5a00, 0, &param_1[7], constant_val) != 0) {
        return 0; // Allocation for param_1[7] failed
    }

    if (allocate(0x10000, 0, &param_1[6], constant_val) != 0) {
        return 0; // Allocation for param_1[6] failed
    }
    return 1; // All allocations successful
}

// Function: gb_reset
void gb_reset(uint32_t *param_1) {
    // Accessing members of a structure pointed to by param_1 using byte offsets
    *(undefined2 *)((uintptr_t)param_1 + 0x2e) = 0x1b0;
    *(undefined2 *)((uintptr_t)param_1 + 0xc * sizeof(uint32_t)) = 0x13;
    *(undefined2 *)((uintptr_t)param_1 + 0x32) = 0xd8;
    *(undefined2 *)((uintptr_t)param_1 + 0xd * sizeof(uint32_t)) = 0x14d;
    *(undefined2 *)((uintptr_t)param_1 + 0x36) = 0xfffe;
    *(undefined2 *)((uintptr_t)param_1 + 0xe * sizeof(uint32_t)) = 0x100;

    // *param_1 (param_1[0]) is the base address of the main allocated memory block
    uint32_t base_mem = param_1[0];

    *(undefined *)(base_mem + 0xff05) = 0;
    *(undefined *)(base_mem + 0xff06) = 0;
    *(undefined *)(base_mem + 0xff07) = 0;
    *(undefined *)(base_mem + 0xff10) = 0x80;
    *(undefined *)(base_mem + 0xff11) = 0xbf;
    *(undefined *)(base_mem + 0xff12) = 0xf3;
    *(undefined *)(base_mem + 0xff14) = 0xbf;
    *(undefined *)(base_mem + 0xff16) = 0x3f;
    *(undefined *)(base_mem + 0xff17) = 0;
    *(undefined *)(base_mem + 0xff19) = 0xbf;
    *(undefined *)(base_mem + 0xff1a) = 0x7f;
    *(undefined *)(base_mem + 0xff1b) = 0xff;
    *(undefined *)(base_mem + 0xff1c) = 0x9f;
    *(undefined *)(base_mem + 0xff1d) = 0xbf;
    *(undefined *)(base_mem + 0xff20) = 0xff;
    *(undefined *)(base_mem + 0xff21) = 0;
    *(undefined *)(base_mem + 0xff22) = 0;
    *(undefined *)(base_mem + 0xff23) = 0xbf;
    *(undefined *)(base_mem + 0xff24) = 0x77;
    *(undefined *)(base_mem + 0xff25) = 0xf3;
    *(undefined *)(base_mem + 0xff26) = 0xf1;
    *(undefined *)(base_mem + 0xff40) = 0x90;
    *(undefined *)(base_mem + 0xff42) = 0;
    *(undefined *)(base_mem + 0xff43) = 0;
    *(undefined *)(base_mem + 0xff45) = 0;
    *(undefined *)(base_mem + 0xff47) = 0xfc;
    *(undefined *)(base_mem + 0xff48) = 0xff;
    *(undefined *)(base_mem + 0xff49) = 0xff;
    *(undefined *)(base_mem + 0xff4a) = 0;
    *(undefined *)(base_mem + 0xff4b) = 0;
    *(undefined *)(base_mem + 0xffff) = 0;
    *(undefined *)(base_mem + 0xff46) = 0xff;

    // Accessing elements of the uint32_t array pointed to by param_1
    param_1[0x12] = 0;
    param_1[0x14] = 0;
    param_1[0x15] = 0;
    param_1[0x16] = 0;
    param_1[0x13] = 0;
    *(undefined *)((uintptr_t)param_1 + 0xb * sizeof(uint32_t)) = 0; // Accessing first byte of param_1[0xb]
    param_1[0xf] = 0;
    param_1[0x10] = _DAT_0001745c;
    param_1[0x11] = 0;
    return;
}

// Function: gb_new
undefined4 gb_new(void) {
    undefined4 gb_struct_ptr_storage[2]; // local_10[0] will hold the pointer to the GB struct

    if (allocate(0x5c, 0, &gb_struct_ptr_storage[0], 0x13be8) != 0) {
        return 0; // Allocation failed, gb_struct_ptr_storage[0] is 0
    }

    // Pass the address of the allocated struct (stored in gb_struct_ptr_storage[0])
    // gb_init expects a pointer to an array of uint32_t, so passing gb_struct_ptr_storage is correct.
    if (gb_init(gb_struct_ptr_storage) == 0) {
        return 0; // Initialization failed
    }

    return gb_struct_ptr_storage[0]; // Return the pointer to the newly created GB struct
}

// Function: copy_title
void copy_title(uint32_t dest_addr, uint32_t src_addr) {
    unsigned int i = 0;
    // Loop until a non-uppercase character is found, or 16 characters are copied
    while (i < 0x10 && isupper(*(unsigned char *)(src_addr + 0x34 + i))) {
        *(undefined *)(dest_addr + i) = *(undefined *)(src_addr + 0x34 + i);
        i++;
    }
    *(undefined *)(dest_addr + i) = 0; // Null-terminate the string
    return;
}

// Function: gb_load
undefined4 gb_load(uint32_t gb_struct_ptr, void *param_2) {
    // The ROM base pointer is stored at offset 4 bytes from gb_struct_ptr (gb_struct_ptr[1])
    uint32_t rom_base_ptr = *(uint32_t*)(gb_struct_ptr + 4);
    memcpy((void*)rom_base_ptr, param_2, 0x8000);

    char title_buffer[17]; // Max 16 chars + null terminator
    copy_title((uint32_t)title_buffer, rom_base_ptr + 0x100);
    printf("Loading %s\n", title_buffer);

    // Check cartridge type bytes at rom_base_ptr + 0x148 and 0x149
    if ((*(byte *)(rom_base_ptr + 0x148) == 0) && (*(byte *)(rom_base_ptr + 0x149) == 0)) {
        return 1; // Compatible cartridge
    } else {
        printf("Incompatible cartridge.\n");
        return 0; // Incompatible cartridge
    }
}

// Function: update_joypad
void update_joypad(uint32_t *param_1) {
    // *param_1 (param_1[0]) is the base address of the main allocated memory block
    uint32_t base_mem = param_1[0];

    *(byte *)(base_mem + 0xff00) |= 0xf;
    if ((*(byte *)(base_mem + 0xff00) & 0x10) == 0) {
        // param_1 + 0xb is (param_1 + 11 * sizeof(uint32_t))
        *(byte *)(base_mem + 0xff00) &= ~(*(byte *)((uintptr_t)param_1 + 0xb * sizeof(uint32_t)) & 0xf);
    }
    if ((*(byte *)(base_mem + 0xff00) & 0x20) == 0) {
        // param_1 + 0xb is (param_1 + 11 * sizeof(uint32_t))
        *(byte *)(base_mem + 0xff00) &= ~(*(byte *)((uintptr_t)param_1 + 0xb * sizeof(uint32_t)) >> 4);
    }
    return;
}

// Function: gb_tick
undefined4 gb_tick(uint32_t *param_1) {
    update_joypad(param_1);

    if (cpu_tick(param_1) == 0) {
        return 0;
    }
    if (lcd_tick(param_1) == 0) {
        return 0;
    }

    param_1[0x12]++; // Increment counter at param_1[18]
    if (param_1[0x12] == 10000000) {
        printf("Game Over\n");
        return 0;
    }

    // *param_1 (param_1[0]) is the base address of the main allocated memory block
    uint32_t base_mem = param_1[0];

    // Check if DMA transfer is requested
    if (*(byte *)(base_mem + 0xff46) != 0xff) { // -1 as char is 0xff as byte
        memcpy((void *)(base_mem + 0xfe00),
               (void *)(base_mem + (uint32_t)*(byte *)(base_mem + 0xff46) * 0x100), 0xa0);
        *(undefined *)(base_mem + 0xff46) = 0xff; // Reset DMA register
    }

    param_1[0x15]++; // Increment counter at param_1[21]
    if (param_1[0x15] == 0x3d) { // If counter reaches 61
        *(byte *)(base_mem + 0xff04) = *(byte *)(base_mem + 0xff04) + 1; // Increment DIV register
        param_1[0x15] = 0; // Reset counter
    }

    // Timer logic
    if ((*(byte *)(base_mem + 0xff07) & 4) != 0) { // If Timer Enable bit is set
        uint32_t timer_frequency;
        byte timer_control_bits = *(byte *)(base_mem + 0xff07) & 3; // Get clock select bits

        if (timer_control_bits == 3) {
            timer_frequency = 0x3d; // 262144 Hz -> ~61 ticks per TIMA increment
        } else if (timer_control_bits == 2) {
            timer_frequency = 0xf;  // 65536 Hz -> ~15 ticks per TIMA increment
        } else if (timer_control_bits == 1) {
            timer_frequency = 4;    // 4096 Hz -> 4 ticks per TIMA increment
        } else { // timer_control_bits == 0
            timer_frequency = 0xf4; // 16384 Hz -> ~244 ticks per TIMA increment
        }

        param_1[0x16]++; // Increment timer internal counter at param_1[22]
        if (timer_frequency <= param_1[0x16]) {
            param_1[0x16] = 0; // Reset internal counter
            uint8_t current_tima = *(byte *)(base_mem + 0xff05);
            *(byte *)(base_mem + 0xff05) = current_tima + 1; // Increment TIMA register

            if (current_tima == 0xff) { // If TIMA overflowed
                *(undefined *)(base_mem + 0xff05) = *(undefined *)(base_mem + 0xff06); // Reload TIMA from TMA
                cpu_interrupt(param_1, 2); // Request Timer interrupt (Interrupt 2 for Timer)
            }
        }
    }
    return 1; // Tick successful
}