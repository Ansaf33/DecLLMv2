#include <stdint.h>  // For fixed-width integers like uint32_t, uint16_t, uint8_t
#include <string.h>  // For memset, memcpy, memcmp
#include <stdlib.h>  // For malloc, free, rand, srand
#include <stdbool.h> // For bool
#include <time.h>    // For time() to seed srand

// Decompiler-generated types mapping to standard C types
typedef uint32_t undefined4;
typedef uint16_t undefined2;
typedef uint8_t undefined;
typedef uint8_t byte;
typedef uint32_t uint;
typedef uint16_t ushort;

// Byte swap macro for 32-bit integers (using GCC/Clang built-in)
#define BSWAP32(x) __builtin_bswap32(x)

// Generic function pointer type for decompiler output 'code'
// This will be cast to specific function pointer types at call sites.
typedef void (*code)();

// --- Inferred Context Structures and Function Pointer Types ---

// Type for the 'read' operation in usb_transfer_context_t
typedef int (*usb_read_op_t)(void *, size_t);
// Type for the 'write' operation in usb_transfer_context_t and usb_device_context_t
typedef void (*usb_write_op_t)(void *, size_t);
// Type for msc_init function pointer
typedef int (*msc_init_op_t)(void *);

// usb_transfer_context_t: Used for transfer-level operations (param_1 in usb_handle_packet, handle_submit, handle_import, handle_unlink)
// This structure is inferred from accesses like `param_1[0]`, `param_1[1]`, `param_1[2]`, `param_1[3]`
typedef struct {
    usb_read_op_t read_op;     // Corresponds to `(**param_1)` or `param_1[0]`
    usb_write_op_t write_op;   // Corresponds to `(*param_1[1])` or `(**(code **)(param_1 + 4))`
    int status;                 // Corresponds to `param_1[2]`
    msc_init_op_t msc_init_op;  // Corresponds to `param_1[3]`
    // A pointer to the device context might be needed if transfer handlers need to access it.
    // For now, assuming it's available via other means or implicitly.
} usb_transfer_context_t;

// usb_device_context_t: Used for device-level operations (param_1 in handle_ep0, send_config_rep, usb_send_reply, usb_init)
// This structure is inferred from accesses like `*(code **)(param_1 + 4)` and `*(uint *)(param_1 + 8)`
typedef struct {
    void *data_0;               // Placeholder for data at offset 0 (if any)
    usb_write_op_t device_write_op; // Corresponds to `*(code **)(param_1 + 4)`
    uint32_t device_status;     // Corresponds to `*(uint *)(param_1 + 8)`
} usb_device_context_t;

// Structure for command table entries
typedef struct {
    int id;
    intptr_t handler_ptr; // Store function pointer as integer type for lookup_command
} command_entry_t;

// --- Global Variables (dummy declarations for compilation) ---
// In a real application, these would be properly defined and initialized.
uint8_t config_desc[9] = {0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32};
uint8_t intf_desc[9] = {0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00};
uint8_t ep_desc[7] = {0x07, 0x05, 0x81, 0x02, 0x00, 0x02, 0x00};
uint8_t device_desc[18] = {0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 0x08, 0x83, 0x76, 0x00, 0x01, 0x00, 0x01, 0x01, 0x02, 0x03, 0x01};
uint8_t device_info[0x138]; // 312 bytes
uint8_t device_intf;

uint8_t DAT_00016100[32];
uint8_t DAT_00016101;
uint8_t DAT_00016102;
uint32_t DAT_00016120;
uint32_t DAT_00016124;

// --- Forward Declarations for Functions ---
// These declarations use the clarified types for parameters.
uint32_t handle_ep0(usb_device_context_t *dev_ctx, uint32_t *urb_data);
uint32_t handle_devlist(usb_device_context_t *dev_ctx);
uint32_t handle_import(usb_transfer_context_t *transfer_ctx);
uint32_t handle_submit(usb_transfer_context_t *transfer_ctx, uint32_t *param_struct);
bool handle_unlink(usb_transfer_context_t *transfer_ctx);
int *lookup_command(int command_id);
uint32_t usb_send_reply(usb_device_context_t *dev_ctx, uint32_t *param_data, uint32_t reply_status, uint32_t data_len);
void send_config_rep(usb_device_context_t *dev_ctx, uint32_t *param_data);
uint32_t usb_init(usb_device_context_t *dev_ctx);
uint32_t usb_handle_packet(usb_transfer_context_t *transfer_ctx); // Declared here for commands array

// --- Dummy External Functions ---
// These functions are called but not defined in the provided snippet.
extern uint32_t msc_handle_ep0(usb_device_context_t *dev_ctx, uint32_t *data_ptr);
extern int msc_init(void *msc_init_ctx_ptr);
extern uint32_t msc_handle_urb(usb_transfer_context_t *transfer_ctx, uint32_t *urb_data);

// --- Commands Array Definition ---
// The actual command IDs are unknown from the snippet, so placeholder values are used.
// The `lookup_command` function expects `int` and `intptr_t` pairs.
command_entry_t commands[] = {
    // These handlers are called by `usb_handle_packet` via `lookup_command`.
    // The `param_data` in the handlers will be `&local_1c` (ushort) or `local_30` (uint[5]).
    // The `param_1` in `usb_handle_packet` (`usb_transfer_context_t *`) is passed directly.
    // The `handle_*` functions in the snippet expect `usb_transfer_context_t *` for their `param_1` argument.
    // For `handle_devlist`, it expects `usb_device_context_t *`, which is a mismatch.
    // This implies `handle_devlist` is not a direct command handler. However, to make it compile as per prompt,
    // and given `lookup_command` returns an entry that leads to `handle_devlist`,
    // we'll assume `handle_devlist` is called with `(usb_transfer_context_t *)` as its first argument
    // and it internally casts it to `usb_device_context_t *` or knows how to extract it.
    // This is a common decompiler artifact where pointer types are lost.
    {0x80000101, (intptr_t)handle_devlist}, // Example ID for a command that might call handle_devlist
    {0x10000001, (intptr_t)handle_import},
    {0x10000002, (intptr_t)handle_submit},
    {0x10000003, (intptr_t)handle_unlink},
    {0, 0} // Terminator
};

// Function: fill_submit_reply
void fill_submit_reply(uint32_t *reply_buffer, uint32_t *param_data, uint32_t reply_status, uint32_t data_len) {
    reply_buffer[0] = 0x3000000;
    reply_buffer[1] = BSWAP32(param_data[1]);
    reply_buffer[2] = BSWAP32(param_data[0]);
    reply_buffer[3] = BSWAP32(param_data[2]);
    reply_buffer[4] = BSWAP32(param_data[3]);
    reply_buffer[6] = BSWAP32(data_len);
    reply_buffer[5] = BSWAP32(reply_status);
    reply_buffer[7] = 0;
    reply_buffer[8] = 0;
    reply_buffer[9] = 0;
    memset(reply_buffer + 10, 0, 8);
}

// Function: send_config_rep
void send_config_rep(usb_device_context_t *dev_ctx, uint32_t *param_data) {
    uint8_t config_reply_buffer[64]; // Buffer for config descriptors
    uint32_t data_len = BSWAP32(param_data[5]); // param_data[0x14] -> param_data[5]

    // The decompiler's for loop for stack initialization is a no-op and removed.
    // The series of memcpy operations are directly translated.
    // `local_18` refers to `config_reply_buffer`
    memcpy(config_reply_buffer, config_desc, sizeof(config_desc));
    memcpy(config_reply_buffer + sizeof(config_desc), intf_desc, sizeof(intf_desc));
    memcpy(config_reply_buffer + sizeof(config_desc) + sizeof(intf_desc), ep_desc, sizeof(ep_desc));

    if (0x20 < data_len) {
        data_len = 0x20;
    }

    // Call usb_send_reply with inferred arguments
    usb_send_reply(dev_ctx, param_data, 0, data_len);

    // Call the device's write operation function
    // pcVar2 = *(code **)(param_1 + 4) -> dev_ctx->device_write_op
    ((usb_write_op_t)dev_ctx->device_write_op)(config_reply_buffer, data_len);
}

// Function: handle_ep0
uint32_t handle_ep0(usb_device_context_t *dev_ctx, uint32_t *urb_data) {
    uint32_t data_len;

    // param_2 + 0x1c -> urb_data[7] (assuming urb_data is uint32_t *)
    // param_2 + 0x1d -> urb_data[7] (byte offset 1)
    // param_2 + 0x1f -> urb_data[7] (byte offset 3)
    // Accessing individual bytes of a uint32_t requires careful casting or shifting.
    // Assuming little-endian and urb_data is an array of uint32_t,
    // then urb_data[7] contains bytes at 0x1c, 0x1d, 0x1e, 0x1f.
    // The decompiler probably means `*(uint8_t *)((uintptr_t)urb_data + 0x1c)`.
    // Let's use direct byte access if urb_data is treated as a byte array, or cast.
    uint8_t *urb_bytes = (uint8_t *)urb_data;

    if (urb_bytes[0x1c] == (uint8_t)-0x80) { // Equivalent to 0x80 for unsigned
        if (urb_bytes[0x1d] == 0x06) {
            if (urb_bytes[0x1f] == 0x01) {
                data_len = BSWAP32(urb_data[5]); // param_2 + 0x14 -> urb_data[5]
                if (0x12 < data_len) {
                    data_len = 0x12;
                }
                usb_send_reply(dev_ctx, urb_data, 0, data_len);
                ((usb_write_op_t)dev_ctx->device_write_op)(device_desc, data_len); // param_1 + 4
                return 1;
            }
            if (urb_bytes[0x1f] == 0x02) {
                send_config_rep(dev_ctx, urb_data);
                return 1;
            }
        }
    } else if (urb_bytes[0x1c] == 0x00) {
        if ((urb_bytes[0x1d] == 0x09) && (urb_bytes[0x1e] == 0x01)) {
            usb_send_reply(dev_ctx, urb_data, 0, 0);
            return 1;
        }
    } else if ((urb_bytes[0x1c] & 0x60) == 0x20) {
        return msc_handle_ep0(dev_ctx, urb_data);
    }
    return 0;
}

// Function: handle_devlist
uint32_t handle_devlist(usb_device_context_t *dev_ctx) { // param_1 is dev_ctx
    undefined2 local_data[6]; // 0xc bytes total
    memset(local_data, 0, 0xc);
    local_data[0] = 0x1101; // 0x1101 (ushort)
    local_data[1] = 0x800;  // 0x800 (ushort)
    ((uint32_t *)local_data)[2] = 0; // local_14
    ((uint32_t *)local_data)[3] = 0x1000000; // local_10 (0x1000000 is 16777216)

    ((usb_write_op_t)dev_ctx->device_write_op)(local_data, 0xc);
    ((usb_write_op_t)dev_ctx->device_write_op)(device_info, 0x138);
    ((usb_write_op_t)dev_ctx->device_write_op)(&device_intf, 4);
    return 1;
}

// Function: handle_import
uint32_t handle_import(usb_transfer_context_t *transfer_ctx) { // param_1 is transfer_ctx
    uint8_t local_3c[44]; // Used as a buffer for reading data
    uint32_t local_reply_data[4]; // local_17c, local_17a, local_178, local_174 (first element)
    uint32_t device_info_copy[78]; // local_174, 0x4e * sizeof(undefined4) = 0x138 bytes

    int bytes_read = transfer_ctx->read_op(local_3c, 0x20);
    if (bytes_read == 0x20) {
        if (memcmp(local_3c, DAT_00016100, 0x20) == 0) {
            ((undefined2 *)local_reply_data)[0] = 0x1101; // local_17c
            ((undefined2 *)local_reply_data)[1] = 0x900;  // local_17a
            local_reply_data[1] = 0;                      // local_178 (param_1[2])
            
            // Replaced loop with memcpy for efficiency and clarity
            memcpy(device_info_copy, device_info, sizeof(device_info_copy));

            int msc_init_result = msc_init(&transfer_ctx->msc_init_op); // param_1 + 3 -> &transfer_ctx->msc_init_op
            if (msc_init_result != 0) {
                transfer_ctx->status = 0x1; // param_1[2] = (code *)0x1
            }
            transfer_ctx->write_op(local_reply_data, 0x140); // param_1[1] is write_op
        } else {
            ((undefined2 *)local_reply_data)[0] = 0x1101; // local_17c
            ((undefined2 *)local_reply_data)[1] = 0x900;  // local_17a
            local_reply_data[1] = 0x1000000;              // local_178
            transfer_ctx->write_op(local_reply_data, 8); // param_1[1] is write_op
        }
        return 1;
    }
    return 0;
}

// Function: handle_submit
uint32_t handle_submit(usb_transfer_context_t *transfer_ctx, uint32_t *param_struct) { // param_1 is transfer_ctx, param_2 is param_struct
    uint32_t local_header_buf[12]; // local_5c, 0x30 bytes for header
    uint32_t local_read_data[3];   // local_2c, local_28, local_1c (0x1c bytes total)
    uint8_t uninitialized_data[8] = {0}; // auStack_18, explicitly initialized to 0

    int bytes_read = transfer_ctx->read_op(local_read_data, 0x1c);
    if (bytes_read == 0x1c) {
        uint32_t data_len = BSWAP32(local_read_data[1]); // local_28 after byte swap
        if ((int)data_len < 0) { // Check for negative length
            return 0;
        }

        uint32_t *urb_data_ptr;
        uint32_t transfer_flags = BSWAP32(param_struct[3]); // param_2 + 0xc -> param_struct[3]
        if (transfer_flags == 0) {
            urb_data_ptr = (uint32_t *)malloc(data_len + 0x30);
        } else {
            urb_data_ptr = local_header_buf;
        }

        if (urb_data_ptr == NULL) {
            return 0;
        }

        urb_data_ptr[0] = BSWAP32(param_struct[2]); // param_2 + 8 -> param_struct[2]
        urb_data_ptr[1] = BSWAP32(param_struct[1]); // param_2 + 4 -> param_struct[1]
        urb_data_ptr[2] = transfer_flags;
        urb_data_ptr[3] = BSWAP32(param_struct[4]); // param_2 + 0x10 -> param_struct[4]
        urb_data_ptr[4] = BSWAP32(local_read_data[0]); // local_2c
        urb_data_ptr[5] = data_len;
        urb_data_ptr[6] = BSWAP32(local_read_data[2]); // local_1c
        memcpy(urb_data_ptr + 7, uninitialized_data, 8); // Copies 8 bytes (2 uint32_t)

        // Potential buffer overflow: If urb_data_ptr points to local_header_buf (48 bytes)
        // and local_header_buf[5] (data_len) is > 0, the read operation will write past the end of local_header_buf.
        if ((urb_data_ptr[2] == 0) && (transfer_ctx->read_op(urb_data_ptr + 12, urb_data_ptr[5]) != urb_data_ptr[5])) {
            if (urb_data_ptr != local_header_buf) { // Only free if it was malloc'd
                free(urb_data_ptr);
            }
            return 0;
        }

        if (urb_data_ptr[3] == 0) {
            // Here param_1 (transfer_ctx) is passed as the first argument to handle_ep0,
            // which expects usb_device_context_t *. This implies a type mismatch or implicit conversion.
            // Assuming transfer_ctx contains or can be cast to usb_device_context_t.
            // For compilation, we cast transfer_ctx to int and then to usb_device_context_t *.
            // A proper fix would involve passing dev_ctx explicitly or having transfer_ctx contain dev_ctx.
            return handle_ep0((usb_device_context_t *)(uintptr_t)transfer_ctx, urb_data_ptr);
        } else {
            return msc_handle_urb(transfer_ctx, urb_data_ptr);
        }
    }
    return 0;
}

// Function: handle_unlink
bool handle_unlink(usb_transfer_context_t *transfer_ctx) { // param_1 is transfer_ctx
    uint8_t local_data[12]; // local_10, 4 bytes used
    int bytes_read = transfer_ctx->read_op(local_data, 4);
    return bytes_read == 4;
}

// Function: lookup_command
int *lookup_command(int command_id) {
    command_entry_t *cmd_entry = commands;
    while (cmd_entry->handler_ptr != 0) { // Check against the terminator's handler_ptr (0)
        if (command_id == cmd_entry->id) {
            return (int *)cmd_entry; // Return pointer to the entry
        }
        cmd_entry++; // Move to the next entry
    }
    return NULL;
}

// Function: usb_handle_packet
uint32_t usb_handle_packet(usb_transfer_context_t *transfer_ctx) { // param_1 is transfer_ctx
    uint32_t command_payload[5]; // local_30
    ushort local_1c_val; // local_1c
    ushort local_1a_val; // local_1a
    int *command_entry_ptr;
    uint32_t result;

    if (transfer_ctx->status == 0) { // param_1[2] == (code *)0x0
        // Read 8 bytes into local_1c_val and local_1a_val.
        // Assuming local_1c_val and local_1a_val are consecutive in memory for the read.
        int bytes_read = transfer_ctx->read_op(&local_1c_val, 8);
        if (bytes_read == 8) {
            // The decompiler output implies local_1c and local_1a are read sequentially.
            // `&local_1c` points to the start, 8 bytes are read.
            // So `local_1c_val` gets the first 4 bytes, `local_1a_val` gets the next 4 bytes.
            // This is incorrect for `ushort`. It should be `uint32_t local_header_word; uint32_t local_command_word;`
            // Let's assume `local_1c_val` and `local_1a_val` are part of a larger buffer or `uint32_t` type.
            // Given the original source, it seems `local_1c` and `local_1a` are 2-byte values.
            // If 8 bytes are read into `&local_1c`, it means `local_1c` is the start of a 8-byte buffer.
            // Let's use a temporary `uint32_t` array for the read to avoid ambiguity.
            uint32_t read_buffer[2];
            memcpy(read_buffer, &local_1c_val, 8); // Read into a buffer
            local_1c_val = (ushort)BSWAP32(read_buffer[0]); // Reconstruct local_1c (ushort)
            local_1a_val = (ushort)BSWAP32(read_buffer[1]); // Reconstruct local_1a (ushort)

            if (local_1c_val == 0x111) { // (local_1c & 0xff) << 8 | (uint)(local_1c >> 8) -> BSWAP16(local_1c)
                // This is a fixed header check.
                command_entry_ptr = lookup_command(BSWAP32(local_1a_val) | 0x80000000); // Command ID from local_1a_val
                if (command_entry_ptr == NULL) {
                    result = 0;
                } else {
                    // Call the handler function. `commands` stores `intptr_t handler_ptr`.
                    // The handler is expected to take `usb_transfer_context_t *` and `void *`.
                    result = ((uint32_t (*)(usb_transfer_context_t *, void *))((command_entry_t *)command_entry_ptr)->handler_ptr)(transfer_ctx, &local_1c_val);
                }
            } else {
                result = 0;
            }
        } else {
            result = 0;
        }
    } else {
        int bytes_read = transfer_ctx->read_op(command_payload, 0x14); // Read 20 bytes into local_30
        if (bytes_read == 0x14) {
            command_entry_ptr = lookup_command(BSWAP32(command_payload[0])); // Command ID from local_30[0]
            if (command_entry_ptr == NULL) {
                result = 0;
            } else {
                // Call the handler function.
                result = ((uint32_t (*)(usb_transfer_context_t *, void *))((command_entry_t *)command_entry_ptr)->handler_ptr)(transfer_ctx, command_payload);
            }
        } else {
            result = 0;
        }
    }
    return result;
}

// Function: usb_send_reply
uint32_t usb_send_reply(usb_device_context_t *dev_ctx, uint32_t *param_data, uint32_t reply_status, uint32_t data_len) {
    uint32_t reply_buffer[12]; // local_3c, 0x30 bytes
    // param_2 + 0x14 -> param_data[5] (length)
    // param_2 + 0x10 -> param_data[4] (flags)
    if ((data_len < BSWAP32(param_data[5])) && ((BSWAP32(param_data[4]) & 1) != 0)) {
        reply_status = 0xffffff87;
    }
    fill_submit_reply(reply_buffer, param_data, reply_status, data_len);
    ((usb_write_op_t)dev_ctx->device_write_op)(reply_buffer, 0x30); // param_1 + 4
    return 1;
}

// Function: usb_init
uint32_t usb_init(usb_device_context_t *dev_ctx) { // param_1 is dev_ctx
    srand(time(NULL)); // Seed random number generator

    // The original random number generation was flawed due to uninitialized local_15.
    // This generates two random digits for DAT_00016100, DAT_00016101, DAT_00016102.
    // Assuming it's for a string like "X-Y".
    uint8_t digit1 = rand() % 10;
    uint8_t digit2 = rand() % 10;

    DAT_00016120 = digit1 << 0x18; // Store byte in MSB, assuming this is a placeholder
    DAT_00016124 = digit2 << 0x18; // Store byte in MSB, assuming this is a placeholder

    memset(DAT_00016100, 0, 0x20);
    DAT_00016100[0] = digit1 + 0x30; // Convert digit to ASCII char
    DAT_00016100[1] = 0x2d;          // '-'
    DAT_00016100[2] = digit2 + 0x30; // Convert digit to ASCII char
    DAT_00016101 = 0x2d;             // Redundant if DAT_00016100 is char array, but kept for decompiler fidelity.
    DAT_00016102 = digit2 + 0x30;    // Redundant, but kept.

    dev_ctx->device_status = 0; // *(undefined4 *)(param_1 + 8) = 0;
    return 1;
}