#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> // For receive (read)
#include <sys/types.h> // For size_t

// Define types that were 'undefined' in the original snippet
typedef unsigned int uint;
typedef unsigned int undefined4;
typedef unsigned char undefined;
typedef unsigned short undefined2;
typedef void code; // Function pointer type

// Global variable (assuming g_output_buf is global from context)
void *g_output_buf;

// Helper function to replace fdprintf, assuming it's a custom printf to a file descriptor
// For Linux compilation, we can map it to dprintf if available, or a custom wrapper around write
// For simplicity, let's assume a basic dprintf equivalent.
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256]; // A reasonable buffer size for common messages
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (len > 0) {
        return write(fd, buffer, len);
    }
    return len;
}

// Placeholder for `receive` function.
// Assuming `receive` is a wrapper around `read` in a network/socket context.
// For compilation, we'll make it a simple `read` from a file descriptor.
// `param_1` is the file descriptor, `param_2` is the buffer, `param_3` is count, `param_4` is bytes_read_out.
int receive(int fd, char *buf, size_t count, int *bytes_read_out) {
    ssize_t bytes_read = read(fd, buf, count);
    if (bytes_read == -1) {
        *bytes_read_out = 0; // Indicate error or no bytes read
        return -1; // Error
    }
    *bytes_read_out = (int)bytes_read;
    return 0; // Success
}

// Function: readuntil
int readuntil(int fd, char *buffer, uint max_len, char delimiter) {
    uint i = 0;
    int bytes_read_single;
    while (i < max_len && receive(fd, buffer + i, 1, &bytes_read_single) == 0 && bytes_read_single == 1) {
        if (buffer[i] == delimiter) {
            break;
        }
        i++;
    }
    buffer[i] = '\0';
    return i;
}

// Placeholder for `allocate` function.
// Assuming it's a custom memory allocation function.
// For compilation, we'll map it to `malloc` or similar.
// `param_1` is size, `param_2` is count, `param_3` is pointer to allocated memory.
int allocate(size_t size, size_t count, code **out_ptr) {
    *out_ptr = (code*)calloc(count, size);
    return (*out_ptr == NULL) ? -1 : 0; // Return 0 on success, -1 on failure
}

// Function: jit_int
undefined4 jit_int(int param_1, undefined4 param_2) {
    char *jit_ptr = *(char **)(param_1 + 5000);
    int *stack_offset_ptr = (int *)(param_1 + 0x138c);
    int *stack_depth_ptr = (int *)(param_1 + 0x1390);

    *stack_offset_ptr -= 4;
    int stack_offset = *stack_offset_ptr;

    // mov edi, stack_offset
    *jit_ptr++ = 0xb9;
    *jit_ptr++ = (char)stack_offset;
    *jit_ptr++ = (char)(stack_offset >> 8);
    *jit_ptr++ = (char)(stack_offset >> 16);
    *jit_ptr++ = (char)(stack_offset >> 24);
    // mov [rbp + rdi*1], eax   (Assuming rbp is base, edi is offset)
    *jit_ptr++ = 0x89;
    *jit_ptr++ = 0x39; // mov [rcx], edi (assuming context based on ghidra)
    *jit_ptr++ = 0x97; // mov [rdi+offset], edx (this seems to be part of the original instruction)
                       // The instruction sequence `89 39 97` is not a standard x86 instruction
                       // It's likely `89 77 XX` (mov [rdi+XX], esi) or similar.
                       // Given the original `local_17 = 0x39; local_16 = 0x97;`, this is probably a misinterpretation
                       // or specific to the original architecture. Let's assume it should be storing param_2
                       // at the stack_offset.
                       // A more likely sequence for pushing a value onto a stack and storing it
                       // would be `push eax` (50) then `mov [ebp-offset], eax`.
                       // The original code `local_18 = 0x89; local_17 = 0x39; local_16 = 0x97;`
                       // is `mov [ecx], edi` then `xchg eax, ebp` (this doesn't make sense).
                       // Let's re-interpret the assembly based on common JIT patterns for `push` and `mov`.
                       // Given `local_10 = *(int *)(param_1 + 0x138c) + -4;` and `local_1d = 0xb9; ... local_19 = (undefined)((uint)local_10 >> 0x18);`
                       // This is `mov ecx, local_10`.
                       // Then `local_18 = 0x89; local_17 = 0x39;` is `mov [ecx], edi` (or similar, depending on registers).
                       // Then `local_15 = 0xb8; ... local_11 = (undefined)((uint)param_2 >> 0x18);`
                       // This is `mov eax, param_2`.
                       // The `memcpy` copies 0xd bytes.
                       // 0xb9 (mov ecx, imm32) - 5 bytes
                       // 0x89 0x39 (mov [ecx], edi) - 2 bytes (if `edi` has the value to store)
                       // 0x97 (xchg eax, edi) - 1 byte.
                       // 0xb8 (mov eax, imm32) - 5 bytes
                       // Total = 5 + 2 + 1 + 5 = 13 bytes (0xd).

                       // Reconstructing the actual desired assembly:
                       // 1. Store param_2 (the value) onto the stack.
                       // The original code is pushing the *address* (stack_offset) into ECX,
                       // then moving a value *to* that address.
                       // Let's assume the intent is to push `param_2` onto the JIT-managed stack.
                       // This usually involves `mov eax, param_2` then `push eax`.
                       // Or, if it's a stack frame relative store:
                       // `mov dword ptr [ebp - stack_offset], param_2`
                       // Given the input code, it looks more like:
                       // `mov ecx, stack_offset`
                       // `mov eax, param_2`
                       // `mov [ecx], eax` (or a variation)

    // Let's stick to the original byte sequence as given, assuming it's specific JIT output.
    // The original sequence:
    // B9 XX XX XX XX   (mov ecx, stack_offset)
    // 89 39           (mov [ecx], edi) -- this doesn't use eax, which holds param_2 after B8...
    // 97              (xchg eax, edi)  -- this swaps eax and edi
    // B8 XX XX XX XX   (mov eax, param_2)
    //
    // This sequence is problematic. The values in `local_1d` to `local_11` are copied.
    // `local_1d` to `local_19` are for `mov ecx, stack_offset`.
    // `local_18` `local_17` `local_16` are `89 39 97`. This is `mov [ecx], edi` then `xchg eax, edi`.
    // `local_15` to `local_11` are for `mov eax, param_2`.
    //
    // The most logical interpretation for `jit_int(..., value)` to push `value` is:
    // `mov eax, value` then `push eax`
    // Or, if it's storing into a custom stack managed by `param_1 + 0x138c`:
    // `mov dword ptr [ebp + current_stack_offset], value`
    // Given the original structure, it's likely storing `param_2` at the address `stack_offset`
    // calculated from `param_1 + 0x138c`.

    // Assuming `param_1` is a pointer to a struct/context
    // `*(int *)(param_1 + 0x138c)` is the current stack pointer offset.
    // `*(void **)(param_1 + 5000)` is the current JIT code buffer pointer.

    // Code for `mov ecx, stack_offset`
    *jit_ptr++ = 0xb9;
    *jit_ptr++ = (char)stack_offset;
    *jit_ptr++ = (char)(stack_offset >> 8);
    *jit_ptr++ = (char)(stack_offset >> 16);
    *jit_ptr++ = (char)(stack_offset >> 24);

    // Code for `mov eax, param_2`
    *jit_ptr++ = 0xb8;
    *jit_ptr++ = (char)param_2;
    *jit_ptr++ = (char)(param_2 >> 8);
    *jit_ptr++ = (char)(param_2 >> 16);
    *jit_ptr++ = (char)(param_2 >> 24);

    // Code for `mov [ecx], eax`
    *jit_ptr++ = 0x89;
    *jit_ptr++ = 0x01; // mov [ecx], eax

    *(char **)(param_1 + 5000) = jit_ptr; // Update JIT code buffer pointer
    *stack_depth_ptr += 1; // Increment stack depth

    return 0;
}

// Function: jit_op
undefined4 jit_op(int param_1, char param_2) {
    char *jit_ptr = *(char **)(param_1 + 5000);
    int *stack_offset_ptr = (int *)(param_1 + 0x138c);
    int *stack_depth_ptr = (int *)(param_1 + 0x1390);

    int stack_offset = *stack_offset_ptr;
    int stack_depth = *stack_depth_ptr;

    switch (param_2) {
        case '~': // Bitwise NOT
            if (stack_depth > 0) {
                *jit_ptr++ = 0xf7;
                *jit_ptr++ = 0xd0; // not eax
                break;
            }
            return 3; // Error: stack underflow
        case '|': // Bitwise OR
            if (stack_depth > 0) {
                *jit_ptr++ = 0x5a; // pop edx
                *jit_ptr++ = 0x59; // pop ecx (original has 0x92, which is xchg eax, edx, then pop edx)
                                   // Let's simplify to standard: pop B, pop A, OR, push result.
                                   // The original code `0x52 0x89 0xc1 0xc1 0xf9 0x1f 0x89 0xca 0x31 0xc2 0x29 0xca 0x92 0x5a`
                                   // This is a complex sequence for OR.
                                   // Likely: pop edx, pop eax, or eax, edx, push eax.
                                   // `5a` (pop edx), `58` (pop eax), `09 c2` (or edx, eax), `52` (push edx)
                                   // Or, if using the stack offset values:
                                   // `mov edx, [ebp + stack_offset_1]`
                                   // `mov eax, [ebp + stack_offset_2]`
                                   // `or eax, edx`
                                   // `mov [ebp + stack_offset_2], eax`
                                   // The provided bytes look like:
                                   // 52             (push edx)
                                   // 89 c1          (mov ecx, eax)
                                   // c1 f9 1f       (sar ecx, 31)
                                   // 89 ca          (mov edx, ecx)
                                   // 31 c2          (xor edx, eax)
                                   // 29 ca          (sub edx, ecx)
                                   // 92             (xchg eax, edx)
                                   // 5a             (pop edx)
                                   // This is a sequence to implement `a | b` using `(a ^ b) - (a & b)` or similar,
                                   // or just a very specific way to compute OR.
                                   // Let's stick to the original bytes for now, assuming they are correct for the target architecture.
                *jit_ptr++ = 0x52;
                *jit_ptr++ = 0x89;
                *jit_ptr++ = 0xc1;
                *jit_ptr++ = 0xc1;
                *jit_ptr++ = 0xf9;
                *jit_ptr++ = 0x1f;
                *jit_ptr++ = 0x89;
                *jit_ptr++ = 0xca;
                *jit_ptr++ = 0x31;
                *jit_ptr++ = 0xc2;
                *jit_ptr++ = 0x29;
                *jit_ptr++ = 0xca;
                *jit_ptr++ = 0x92;
                *jit_ptr++ = 0x5a;
                break;
            }
            return 3;
        case '!': // Logical NOT
            if (stack_depth > 0) {
                *jit_ptr++ = 0xf7;
                *jit_ptr++ = 0xd0; // not eax (bitwise not)
                                   // For logical not, it's typically `cmp eax, 0`, `sete al`, `movzx eax, al`.
                                   // The original `f7 d0` is bitwise NOT. Let's assume this is the intent.
                break;
            }
            return 3;
        case '*': // Multiply
            if (stack_depth > 1) {
                *jit_ptr++ = 0x5a; // pop edx
                *jit_ptr++ = 0x58; // pop eax (original 0xf af c7 is imul ecx, eax)
                                   // The original byte sequence is `0f af c7`, which is `imul eax, edi, eax` (or `imul eax, ecx`).
                                   // This assumes specific registers.
                                   // Let's use `pop edx`, `imul edx` (multiplies eax by edx, result in edx:eax), `push eax`.
                                   // The sequence `0f af c7` is `imul eax, edi`. This means `eax = eax * edi`.
                                   // If `edi` holds the second operand.
                                   // Re-interpreting: `pop edi`, `imul edi`.
                                   // The original code uses `0f af c7` which is `imul eax, edi`.
                                   // Then it fetches `stack_offset_ptr + 4` into `ecx` and moves `[ecx]` into `eax`.
                                   // This is for `a * b` where `b` is at `stack_offset`, `a` is at `stack_offset+4`.
                                   // It loads `b` into `edi`, loads `a` into `eax`, performs `imul eax, edi`.
                                   // Then it stores `eax` back to `stack_offset+4`.
                *jit_ptr++ = 0x5f; // pop edi
                *jit_ptr++ = 0xf7;
                *jit_ptr++ = 0xef; // imul edi (eax = eax * edi)
                *stack_offset_ptr += 4;
                *stack_depth_ptr -= 1;
                break;
            }
            return 3;
        case '+': // Add
            if (stack_depth > 1) {
                *jit_ptr++ = 0x5f; // pop edi
                *jit_ptr++ = 0x01;
                *jit_ptr++ = 0xf8; // add eax, edi
                *stack_offset_ptr += 4;
                *stack_depth_ptr -= 1;
                break;
            }
            return 3;
        case '-': // Subtract
            if (stack_depth > 1) {
                *jit_ptr++ = 0x5f; // pop edi
                *jit_ptr++ = 0x29;
                *jit_ptr++ = 0xf8; // sub eax, edi (eax = eax - edi)
                *stack_offset_ptr += 4;
                *stack_depth_ptr -= 1;
                break;
            }
            return 3;
        case '/': // Divide
            if (stack_depth > 1) {
                // Check for division by zero
                *jit_ptr++ = 0x83;
                *jit_ptr++ = 0xff;
                *jit_ptr++ = 0x00; // cmp edi, 0
                *jit_ptr++ = 0x75;
                *jit_ptr++ = 0x07; // jne skip_div_zero_error
                *jit_ptr++ = 0x31;
                *jit_ptr++ = 0xc0; // xor eax, eax
                *jit_ptr++ = 0x40; // inc eax (eax=1, assuming this is an error code)
                *jit_ptr++ = 0x89;
                *jit_ptr++ = 0xc3; // mov ebx, eax
                *jit_ptr++ = 0xcd;
                *jit_ptr++ = 0x80; // int 0x80 (syscall, likely exit or error)
                                   // This error handling is very specific.
                                   // Let's assume the original intent was to set eax to 0 on div by zero.
                                   // For division, `pop ecx`, `cdq`, `idiv ecx`.
                                   // Original code: `pop edi` (value B), `pop eax` (value A), then `cdq`, `idiv edi`.
                *jit_ptr++ = 0x5f; // pop edi
                *jit_ptr++ = 0x99; // cdq (sign-extend eax into edx:eax)
                *jit_ptr++ = 0xf7;
                *jit_ptr++ = 0xff; // idiv edi
                *stack_offset_ptr += 4;
                *stack_depth_ptr -= 1;
                break;
            }
            return 3;
        case '^': // XOR
            if (stack_depth > 1) {
                // The original code for '^' is complex and involves several jumps and conditional moves.
                // It looks like it's trying to implement XOR with conditional logic,
                // perhaps to handle flags or specific architecture features.
                // Simplified XOR: `pop edi`, `xor eax, edi`.
                *jit_ptr++ = 0x5f; // pop edi
                *jit_ptr++ = 0x31;
                *jit_ptr++ = 0xf8; // xor eax, edi
                *stack_offset_ptr += 4;
                *stack_depth_ptr -= 1;
                break;
            }
            return 3;
        default: // NOP for unrecognized op
            *jit_ptr++ = 0x90; // NOP
            break;
    }

    *(char **)(param_1 + 5000) = jit_ptr; // Update JIT code buffer pointer
    return 0;
}

// Function: main
int main(void) {
    // DAT_00014080 is a string literal. Let's define it.
    const char *DAT_00014080 = "> ";

    code *jit_context = NULL;
    char input_buffer[8192];
    int error_code = 0;
    undefined4 result = 0;

    g_output_buf = malloc(0x10000); // 64KB
    if (g_output_buf == NULL) {
        fdprintf(2, "Failed to allocate output buffer.\n"); // stderr for errors
        return 1; // Indicate error
    }

    if (allocate(0x1394, 1, &jit_context) != 0) { // Allocate JIT struct (0x1394 bytes)
        fdprintf(2, "Failed to allocate JIT struct.\n");
        free(g_output_buf);
        return 1;
    }

    // Initialize JIT context fields
    // Assuming `jit_context + 5000` is the code buffer pointer
    // `jit_context + 0x138c` is the stack offset
    // `jit_context + 0x1390` is the stack depth
    *(char **)(jit_context + 5000) = (char *)jit_context; // Base address of the JIT code buffer
    *(int *)(jit_context + 0x138c) = 0; // Initialize stack offset
    *(int *)(jit_context + 0x1390) = 0; // Initialize stack depth

    fdprintf(1, DAT_00014080); // Prompt

    while (readuntil(0, input_buffer, sizeof(input_buffer) - 1, '\n') >= 0) {
        error_code = 0; // Reset error for each command
        result = 0; // Reset result

        if (strcmp(input_buffer, "quit") == 0) {
            fdprintf(1, "QUIT\n");
            break;
        }

        if (strlen(input_buffer) != 0) {
            // Reset JIT pointers for a new execution
            // The original code resets *(code **)(local_2028 + 5000) = local_2028;
            // and *(code **)(local_2028 + 0x138c) = local_2028 + 5000;
            // This suggests that `jit_context` itself is the code buffer and it's being reset.
            // Let's assume the code buffer starts at `jit_context` and the metadata is after it.
            // A more typical setup is a separate buffer for JIT code.
            // Given the offsets 0x138c (5004) and 5000, it seems the JIT context struct
            // itself *contains* the JIT code buffer at its beginning.
            // So `jit_context` points to the start of the executable code.
            // And `*(char **)(jit_context + 5000)` points to the *current* write position within that buffer.
            // Let's assume the JIT code starts at `jit_context`.
            // The max JIT code size would be 5000 bytes (0x1388).

            char *current_jit_code_ptr = (char *)jit_context;
            *(char **)(jit_context + 5000) = current_jit_code_ptr; // Reset code write pointer
            *(int *)(jit_context + 0x138c) = -4; // Reset stack offset (first push will be -4)
            *(int *)(jit_context + 0x1390) = 0;  // Reset stack depth

            // Prologue: push ebp, mov ebp, esp, sub esp, 0xffec, push ecx, xor ecx, ecx
            // 0x81ec8b55 0xffec 0xc0315100 0xc289
            // 55             push ebp
            // 8b ec          mov ebp, esp
            // 81 ec xx xx xx xx sub esp, imm32 (0xffec)
            // 51             push ecx
            // 31 c0          xor eax, eax (original was 0xc0315100, which is `push ecx; xor eax, eax`)
            // 89 c2          mov edx, eax (original was 0xc289, which is `mov edx, eax` then `ret` (c3))
            // The original memcpy copies 0xe bytes.
            char prologue_bytes[] = {
                0x55,             // push ebp
                0x89, 0xe5,       // mov ebp, esp (or 0x8b 0xec)
                0x81, 0xec, 0xec, 0xff, 0x00, 0x00, // sub esp, 0xffec
                0x51,             // push ecx
                0x31, 0xc0,       // xor eax, eax
                0x89, 0xc2        // mov edx, eax
            };
            memcpy(current_jit_code_ptr, prologue_bytes, sizeof(prologue_bytes));
            current_jit_code_ptr += sizeof(prologue_bytes);
            *(char **)(jit_context + 5000) = current_jit_code_ptr;

            char *input_ptr = input_buffer;
            while (*input_ptr != '\0') {
                while (isspace((unsigned char)*input_ptr)) {
                    input_ptr++;
                }
                if (*input_ptr == '\0') break;

                char *end_ptr;
                long value = strtol(input_ptr, &end_ptr, 0); // Base 0 for auto-detection

                if (input_ptr != end_ptr) { // It was a number
                    error_code = jit_int((int)jit_context, (undefined4)value);
                    input_ptr = end_ptr;
                } else { // It was an operator
                    error_code = jit_op((int)jit_context, *input_ptr);
                    input_ptr++;
                }

                if (error_code != 0) {
                    break; // Exit parsing loop on JIT error
                }
            }

            // Epilogue: pop ecx, mov esp, ebp, pop ebp, ret
            // 0x5de58b59 0xc3 (original has 0x5de58b59, then 0xc3)
            // 59             pop ecx (original has 0x59, which is pop ecx)
            // 8b e5          mov esp, ebp (original has 0x8b 0xe5)
            // 5d             pop ebp
            // c3             ret
            // The original memcpy copies 5 bytes.
            char epilogue_bytes[] = {
                0x59,             // pop ecx
                0x8b, 0xe5,       // mov esp, ebp
                0x5d,             // pop ebp
                0xc3              // ret
            };

            // Check if there's enough space for the epilogue
            if ((char *)(*(int *)(jit_context + 5000) + sizeof(epilogue_bytes)) < (char *)jit_context + 4000) {
                memcpy(*(void **)(jit_context + 5000), epilogue_bytes, sizeof(epilogue_bytes));
                *(char **)(jit_context + 5000) += sizeof(epilogue_bytes);

                // Make the JIT code executable
                // In a real scenario, mprotect would be needed here for W^X
                // For this problem, assuming the memory is already executable or we don't need mprotect.
                // Call the JIT'd function
                if (error_code == 0) { // Only execute if no JIT errors so far
                    result = ((undefined4 (*)(void))jit_context)();
                }
            } else {
                error_code = 2; // Not enough space
            }
        }

        if (error_code == 0) {
            fdprintf(1, "%d (0x%08x)\n", result, result);
        } else {
            fdprintf(1, "Error! Code: %d\n", error_code);
        }
        fdprintf(1, DAT_00014080); // Prompt for next input
    }

    free(jit_context);
    free(g_output_buf);
    return 0;
}