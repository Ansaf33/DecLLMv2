#include <stdlib.h> // For calloc, free, perror
#include <stdio.h>  // For fprintf, vfprintf, stderr
#include <stdarg.h> // For va_list
#include <stddef.h> // For size_t

// --- Type definitions (based on decompiled output) ---
// Assuming 32-bit integer sizes where "undefined4" is used for data values.
typedef unsigned int u4;   // For data that's 4 bytes, like message IDs or flags
typedef unsigned char u1;  // For byte-sized flags or message types

// Forward declarations
typedef struct Context Context;
typedef struct SlsFunctions SlsFunctions;
typedef struct MSG MSG;

// Structure for the main context object. Total size 0x8c.
// This structure needs to match the memory layout implied by the offsets.
// It assumes a 32-bit architecture for consistent padding and pointer sizes.
struct Context {
    u1 flag_0;       // Corresponds to *__ptr (offset 0x0)
    u1 stop_flag_1;  // Corresponds to __ptr[1] (offset 0x1)
    u1 _pad_2_3[2];  // Padding to align sls_funcs_ptr to 4 bytes
    SlsFunctions* sls_funcs_ptr; // Corresponds to *(__ptr + 4) (offset 0x4)
    // Placeholder for remaining data to reach 0x8c bytes.
    char _data[0x8c - (2 * sizeof(u1)) - sizeof(u1[2]) - sizeof(SlsFunctions*)];
};

// Structure for the SLS functions table (function pointers).
// This layout assumes a 32-bit architecture where `sizeof(void*)` is 4 bytes,
// matching the byte offsets (0x00, 0x08, 0x18, etc.) from the decompiled code.
// On a 64-bit system, `sizeof(void*)` is typically 8 bytes, which would misalign
// these members unless compiled with `-m32` or specific packing attributes.
struct SlsFunctions {
    void* (*get_message)(Context*);        // Offset 0x00
    void* _reserved_04;                     // Placeholder for 0x04
    void (*cleanup)(Context*);              // Offset 0x08
    void* _reserved_0C;                     // Placeholder for 0x0C
    void* _reserved_10;                     // Placeholder for 0x10
    void* _reserved_14;                     // Placeholder for 0x14
    void (*handle_msg_type_5)(MSG*);        // Offset 0x18
    void* _reserved_1C;                     // Placeholder for 0x1C
    void (*handle_msg_type_3)(Context*, MSG*); // Offset 0x20
    void* _reserved_24;                     // Placeholder for 0x24
    void (*initialize)(Context*);           // Offset 0x28
    void* (*get_related_data)(Context*, u4); // Offset 0x2c (returns void* iVar2)
    void (*handle_msg_type_4)(Context*, void*, MSG*); // Offset 0x30
    void (*handle_msg_type_1)(Context*, void*, MSG*); // Offset 0x34
    void (*handle_msg_type_2)(Context*, MSG*); // Offset 0x38
};

// Structure for a message object.
// Offsets are relative to the base of this struct.
struct MSG {
    u4 _pad_0_3;   // Padding/unknown data
    u1 type;       // Corresponds to *(undefined *)((int)__ptr_00 + 4) (offset 0x4)
    u1 _pad_5_B[7]; // Padding to offset 0xc
    u4 data_0c;    // Corresponds to *(undefined4 *)((int)__ptr_00 + 0xc) (offset 0xc)
    u4 _pad_10_13; // Padding to offset 0x14
    void* data_14; // Corresponds to *(void **)((int)__ptr_00 + 0x14) (offset 0x14)
    // Assume this is the end of the struct based on free calls.
};

// --- Mock functions (since they are not provided in the snippet) ---
// These are placeholders to make the code compilable and runnable for demonstration.

static SlsFunctions* msls_get_sls_functions(void);
static void debug_print(const char* format, ...);

// Mock implementation for compilation
static SlsFunctions* msls_get_sls_functions(void) {
    // In a real scenario, this would return a pointer to a table of actual functions.
    // For compilation, we need to return a valid pointer to a mock SlsFunctions struct.
    static SlsFunctions mock_sls_funcs = {0}; // Initialize all members to NULL
    return &mock_sls_funcs;
}

static void debug_print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

// Mock function for the 'get_message' function pointer
static void* mock_get_message_func(Context* ctx) {
    static int msg_count = 0;
    if (msg_count < 2) { // Simulate returning a message twice
        MSG* dummy_msg = (MSG*)calloc(1, sizeof(MSG));
        if (dummy_msg) {
            dummy_msg->type = (msg_count == 0) ? 1 : 6; // Simulate type 1 then type 6
            dummy_msg->data_0c = 0x1234;
            dummy_msg->data_14 = NULL; // No nested data to free for now
        }
        msg_count++;
        return dummy_msg;
    }
    return NULL; // Simulate no more messages
}

// Mock function for the 'initialize' function pointer
static void mock_initialize_func(Context* ctx) {
    debug_print("Context initialized.\n");
}

// Mock function for the 'get_related_data' function pointer
static void* mock_get_related_data_func(Context* ctx, u4 msg_data) {
    debug_print("Getting related data for msg_data: 0x%x\n", msg_data);
    // Simulate returning a pointer to some data.
    // This data would then be dereferenced at offsets +0xc and +0x4.
    // Let's create a dummy structure for this to satisfy the checks.
    static struct {
        char _data_0_3[4];  // Offset 0x0
        char flag_at_4;     // Offset 0x4
        char _data_5_B[7];  // Padding
        char flag_at_C;     // Offset 0xC
    } dummy_related_data = { .flag_at_4 = 'A', .flag_at_C = 'B' };

    return &dummy_related_data; // Simulate returning a valid pointer
}

// Mock function for cleanup
static void mock_cleanup_func(Context* ctx) {
    debug_print("Context cleaned up.\n");
}

// Mock functions for handle_msg_type_X
static void mock_handle_msg_type_1(Context* ctx, void* data_ptr, MSG* msg) { debug_print("handle_msg_type_1 called for msg type %d\n", msg->type); }
static void mock_handle_msg_type_2(Context* ctx, MSG* msg) { debug_print("handle_msg_type_2 called for msg type %d\n", msg->type); }
static void mock_handle_msg_type_3(Context* ctx, MSG* msg) { debug_print("handle_msg_type_3 called for msg type %d\n", msg->type); }
static void mock_handle_msg_type_4(Context* ctx, void* data_ptr, MSG* msg) { debug_print("handle_msg_type_4 called for msg type %d\n", msg->type); }
static void mock_handle_msg_type_5(MSG* msg) { debug_print("handle_msg_type_5 called for msg type %d\n", msg->type); }


// --- Main function ---
int main(void) {
    // Allocate and initialize the main context structure.
    // The original calloc(0x8c, in_stack_ffffffd4) implies calloc(1, 0x8c) for a single struct of 0x8c bytes.
    Context* ctx = (Context*)calloc(1, sizeof(Context));
    if (!ctx) {
        perror("Failed to allocate context");
        return 1;
    }

    // Get the SLS functions table and store its pointer in the context.
    ctx->sls_funcs_ptr = msls_get_sls_functions();

    // Populate mock SlsFunctions with actual mock functions for testing.
    // In a real scenario, msls_get_sls_functions would return a pointer to an already populated table.
    if (ctx->sls_funcs_ptr) {
        ctx->sls_funcs_ptr->get_message = mock_get_message_func;
        ctx->sls_funcs_ptr->initialize = mock_initialize_func;
        ctx->sls_funcs_ptr->get_related_data = mock_get_related_data_func;
        ctx->sls_funcs_ptr->cleanup = mock_cleanup_func;
        ctx->sls_funcs_ptr->handle_msg_type_1 = mock_handle_msg_type_1;
        ctx->sls_funcs_ptr->handle_msg_type_2 = mock_handle_msg_type_2;
        ctx->sls_funcs_ptr->handle_msg_type_3 = mock_handle_msg_type_3;
        ctx->sls_funcs_ptr->handle_msg_type_4 = mock_handle_msg_type_4;
        ctx->sls_funcs_ptr->handle_msg_type_5 = mock_handle_msg_type_5;
    } else {
        debug_print("ERROR: SLS functions table not available.\n");
        free(ctx);
        return 1;
    }

    // Call the initialization function.
    ctx->sls_funcs_ptr->initialize(ctx);

    // Set the initial flag.
    ctx->flag_0 = 1;

    // Main loop: continue while stop_flag_1 is 0.
    while (ctx->stop_flag_1 == 0) {
        MSG* msg = (MSG*)ctx->sls_funcs_ptr->get_message(ctx);

        if (msg == NULL) {
            debug_print("Skipping invalid MSG\n");
        } else {
            // Get related data; assuming the function returns a pointer.
            // This replaces the intermediate variable `iVar2`.
            void* related_data_ptr = ctx->sls_funcs_ptr->get_related_data(ctx, msg->data_0c);

            switch (msg->type) {
            case 1:
                // Check if data_ptr is valid and a specific byte at offset 0xc is not null.
                if ((related_data_ptr != NULL) && (*((char*)related_data_ptr + 0xc) != '\0')) {
                    ctx->sls_funcs_ptr->handle_msg_type_1(ctx, related_data_ptr, msg);
                }
                break;
            case 2:
                ctx->sls_funcs_ptr->handle_msg_type_2(ctx, msg);
                break;
            case 3:
                ctx->sls_funcs_ptr->handle_msg_type_3(ctx, msg);
                break;
            case 4:
                // Check if data_ptr is valid and a specific byte at offset 0xc is not null.
                if ((related_data_ptr != NULL) && (*((char*)related_data_ptr + 0xc) != '\0')) {
                    ctx->sls_funcs_ptr->handle_msg_type_4(ctx, related_data_ptr, msg);
                }
                break;
            case 5:
                // Check if data_ptr is null OR a specific byte at offset 0x4 is null.
                if ((related_data_ptr == NULL) || (*((char*)related_data_ptr + 4) == '\0')) {
                    ctx->sls_funcs_ptr->handle_msg_type_5(msg);
                }
                break;
            case 6:
                ctx->stop_flag_1 = 1; // Set flag to exit loop
                break;
            }
            free(msg->data_14); // Free nested data if it was allocated
            free(msg);          // Free the message itself
        }
    }

    // Call the cleanup function.
    ctx->sls_funcs_ptr->cleanup(ctx);

    free(ctx);
    return 0;
}