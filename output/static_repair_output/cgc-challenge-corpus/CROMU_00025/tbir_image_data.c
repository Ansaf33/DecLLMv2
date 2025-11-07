#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // For uint8_t, uint16_t, uint32_t
#include <stddef.h> // For size_t

// Replace undefined types
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;

// Mock functions (replace with actual implementations if available)
// `allocate` and `deallocate` are mapped to `malloc` and `free`.
static inline int allocate(size_t size, int flags, void** ptr_out) {
    (void)flags; // Unused parameter
    *ptr_out = malloc(size);
    return (*ptr_out != NULL);
}

static inline void deallocate(void* ptr, size_t size) {
    (void)size; // Unused parameter
    free(ptr);
}

// Define the TbirContext structure based on param_1 indexing
// Assuming sizeof(int) == 4 bytes
typedef struct {
    uint32_t buffer_start_address;     // param_1[0] - base address of the image data in memory
    uint32_t buffer_total_bytes;       // param_1[1] - total size of the image data buffer
    uint32_t current_byte_offset;      // param_1[2] - current byte offset within the buffer
    uint32_t current_bit_offset;       // param_1[3] - current bit offset within the current byte (0-7)
    uint32_t width;                    // param_1[4] - image width (from header)
    uint32_t height;                   // param_1[5] - image height (from header)
    uint32_t pixel_type;               // param_1[6] - pixel bit depth / type (from flags)
    uint32_t load_direction;           // param_1[7] - pixel load direction (from flags)
    char* pixel_data_buffer;           // param_1[8] - pointer to allocated pixel data
    uint32_t pixel_data_buffer_size;   // param_1[9] - size of allocated pixel data
    uint32_t checksum;                 // param_1[10] - stored/calculated checksum
    uint32_t flags;                    // param_1[11] - various flags (e.g., 0x1=header read, 0x10=flags read, 0x100=pixel type read, 0x1000=load direction read, 0x10000=pixels read, 0x100000=checksum read)
} TbirContext;

// Forward declarations for functions defined later
static undefined4 tbir_read_nbits(TbirContext *ctx, int num_bits, uint32_t *out_value);
static undefined4 tbir_read_check(TbirContext *ctx, int num_bits);
static undefined4 tbir_read_pixeltype(TbirContext *ctx);
static undefined4 tbir_read_loadd(TbirContext *ctx);
static undefined4 tbir_read_header(TbirContext *ctx);
static undefined4 tbir_read_magic(TbirContext *ctx);
static int tbir_read_typef(TbirContext *ctx);

// Global character tables (extracted from tbir_read_pixels)
// Main character lookup table (pixel_type 7)
static const char pixel_chars_type7[] =
    " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
static const size_t pixel_chars_type7_len = sizeof(pixel_chars_type7) - 1; // Exclude null terminator

// pixel_type 6 characters: 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
static const char pixel_chars_type6_lookup[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const size_t pixel_chars_type6_lookup_len = sizeof(pixel_chars_type6_lookup) - 1;

// pixel_type 1 characters: ' ' and '.'
static const char pixel_chars_type1[] = " .";
static const size_t pixel_chars_type1_len = sizeof(pixel_chars_type1) - 1;

// pixel_type 2 characters: ' ', '#', '*', '@'
static const char pixel_chars_type2[] = " #*@";
static const size_t pixel_chars_type2_len = sizeof(pixel_chars_type2) - 1;


// Function: tbir_read_chksum
undefined4 tbir_read_chksum(TbirContext *ctx) {
    uint32_t read_checksum = 0;
    if (ctx == NULL || tbir_read_nbits(ctx, 0x20, &read_checksum) == 0) {
        return 0;
    }

    if (((ctx->flags & 0x10000) != 0) && (ctx->checksum != read_checksum)) {
        return 0; // Checksum mismatch
    }

    ctx->checksum = read_checksum;
    ctx->flags |= 0x100000;
    return 1;
}

// Function: tbir_read_pixels
int tbir_read_pixels(TbirContext *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    if (!(((ctx->flags & 1U) != 0) && ((ctx->flags & 0x10U) != 0) &&
          ((ctx->flags & 0x100U) != 0) && ((ctx->flags & 0x1000U) != 0))) {
        return 0; // Missing required flags
    }

    if (ctx->current_bit_offset != 0) {
        return 0; // Should be byte aligned
    }

    ctx->flags |= 0x10000;

    uint32_t total_pixels_bits = ctx->pixel_type * ctx->width * ctx->height;
    uint32_t padding_bits = (total_pixels_bits + 0x1fU & 0xffffffe0) - total_pixels_bits;
    uint32_t total_dwords = (total_pixels_bits + padding_bits + 31) / 32;

    if (ctx->buffer_total_bytes < total_dwords * 4 + ctx->current_byte_offset) {
        printf("Dword count too high: %u %u %u\n", total_dwords * 4, ctx->current_byte_offset, ctx->buffer_total_bytes);
        return 0;
    }

    uint32_t current_checksum_val = 0;
    uint32_t data_start_addr = ctx->buffer_start_address + ctx->current_byte_offset;
    for (int i = 0; i < total_dwords; ++i) {
        current_checksum_val ^= *(uint32_t *)(data_start_addr + i * 4);
    }

    if ((ctx->flags & 0x100000U) == 0) {
        ctx->checksum = current_checksum_val;
    } else if (current_checksum_val != ctx->checksum) {
        return 0; // Checksum mismatch
    }

    ctx->pixel_data_buffer_size = ctx->height * ctx->width;
    if (allocate(ctx->pixel_data_buffer_size + 1, 0, (void**)&ctx->pixel_data_buffer) == 0) {
        return 0; // Allocation failed
    }

    memset(ctx->pixel_data_buffer, 0x20, ctx->pixel_data_buffer_size);
    ctx->pixel_data_buffer[ctx->pixel_data_buffer_size] = 0;

    uint32_t pixel_value;
    int row_idx, col_idx;
    int pixel_index_in_buffer;
    
    for (int i = 0; i < ctx->pixel_data_buffer_size; ++i) {
        if (tbir_read_nbits(ctx, ctx->pixel_type, &pixel_value) == 0) {
            goto cleanup_and_fail;
        }

        switch (ctx->load_direction) {
            case 0:
                row_idx = i / ctx->width;
                col_idx = i % ctx->width;
                break;
            case 1:
                row_idx = i / ctx->width;
                col_idx = (ctx->width - 1) - (i % ctx->width);
                break;
            case 2:
                row_idx = (ctx->height - 1) - (i / ctx->width);
                col_idx = i % ctx->width;
                break;
            case 3:
                row_idx = (ctx->height - 1) - (i / ctx->width);
                col_idx = (ctx->width - 1) - (i % ctx->width);
                break;
            case 4:
                row_idx = i % ctx->height;
                col_idx = i / ctx->height;
                break;
            case 5:
                row_idx = i % ctx->height;
                col_idx = (ctx->width - 1) - (i / ctx->height);
                break;
            case 6:
                row_idx = (ctx->height - 1) - (i % ctx->height);
                col_idx = i / ctx->height;
                break;
            case 7:
                row_idx = (ctx->height - 1) - (i % ctx->height);
                col_idx = (ctx->width - 1) - (i / ctx->height);
                break;
            default:
                printf("[ERROR] Invalid load direction\n");
                goto cleanup_and_fail;
        }

        pixel_index_in_buffer = col_idx + ctx->width * row_idx;

        if (pixel_index_in_buffer < 0 || pixel_index_in_buffer >= ctx->pixel_data_buffer_size) {
            printf("[ERROR] Pixel index out of bounds: %d (max %u)\n", pixel_index_in_buffer, ctx->pixel_data_buffer_size);
            goto cleanup_and_fail;
        }

        switch (ctx->pixel_type) {
            case 7:
                if (pixel_value >= pixel_chars_type7_len) {
                    printf("[ERROR] Invalid pixel value %u for pixel type 7 (max %zu)\n", pixel_value, pixel_chars_type7_len - 1);
                    goto cleanup_and_fail;
                }
                ctx->pixel_data_buffer[pixel_index_in_buffer] = pixel_chars_type7[pixel_value];
                break;
            case 6:
                if (pixel_value >= pixel_chars_type6_lookup_len) {
                    printf("[ERROR] Invalid pixel value %u for pixel type 6 (max %zu)\n", pixel_value, pixel_chars_type6_lookup_len - 1);
                    goto cleanup_and_fail;
                }
                ctx->pixel_data_buffer[pixel_index_in_buffer] = pixel_chars_type6_lookup[pixel_value];
                break;
            case 1:
                if (pixel_value >= pixel_chars_type1_len) {
                    printf("[ERROR] Invalid pixel value %u for pixel type 1 (max %zu)\n", pixel_value, pixel_chars_type1_len - 1);
                    goto cleanup_and_fail;
                }
                ctx->pixel_data_buffer[pixel_index_in_buffer] = pixel_chars_type1[pixel_value];
                break;
            case 2:
                if (pixel_value >= pixel_chars_type2_len) {
                    printf("[ERROR] Invalid pixel value %u for pixel type 2 (max %zu)\n", pixel_value, pixel_chars_type2_len - 1);
                    goto cleanup_and_fail;
                }
                ctx->pixel_data_buffer[pixel_index_in_buffer] = pixel_chars_type2[pixel_value];
                break;
            default:
                printf("[ERROR] Invalid pixel type %u\n", ctx->pixel_type);
                goto cleanup_and_fail;
        }
    }

    if (tbir_read_nbits(ctx, padding_bits, &pixel_value) == 0) { // pixel_value used for dummy read
        goto cleanup_and_fail;
    }

    return 1;

cleanup_and_fail:
    deallocate(ctx->pixel_data_buffer, ctx->pixel_data_buffer_size + 1);
    ctx->pixel_data_buffer = NULL;
    ctx->pixel_data_buffer_size = 0;
    return 0;
}

// Function: tbir_read_flags
undefined4 tbir_read_flags(TbirContext *ctx) {
    if (tbir_read_pixeltype(ctx) == 0) {
        return 0;
    }
    if (tbir_read_loadd(ctx) == 0) {
        return 0;
    }
    ctx->flags |= 0x10;
    return 1;
}

// Function: tbir_display_img
undefined4 tbir_display_img(TbirContext *ctx) {
    if (ctx == NULL || tbir_read_magic(ctx) == 0) {
        return 0;
    }

    int field_type;
    while ((field_type = tbir_read_typef(ctx)) != 0) {
        switch (field_type) {
            case 0xaaee: // End of image tag
                if (!(((ctx->flags & 1) != 0) && ((ctx->flags & 0x10) != 0) &&
                      ((ctx->flags & 0x10000) != 0) && ((ctx->flags & 0x100000) != 0))) {
                    printf("[ERROR] Missing a required tag\n");
                    return 0;
                }
                for (int i = 0; i < ctx->pixel_data_buffer_size; ++i) {
                    if ((i % ctx->width == 0) && (i != 0)) {
                        printf("\n");
                    }
                    printf("%c", ctx->pixel_data_buffer[i]);
                }
                printf("\n");
                return 1;

            case 0xaadd: // Checksum tag
                if ((ctx->flags & 0x100000) != 0) {
                    printf("[ERROR] Only one checksum field\n");
                    return 0;
                }
                if (tbir_read_chksum(ctx) == 0) {
                    return 0;
                }
                break;

            case 0xaacc: // Pixels tag
                if ((ctx->flags & 0x10000) != 0) {
                    printf("[ERROR] Only one pixel set\n");
                    return 0;
                }
                if (tbir_read_pixels(ctx) == 0) {
                    return 0;
                }
                break;

            case 0xaaaa: // Header tag
                if ((ctx->flags & 1) != 0) {
                    printf("[ERROR] Only one header\n");
                    return 0;
                }
                if (tbir_read_header(ctx) == 0) {
                    return 0;
                }
                break;

            case 0xaabb: // Flags tag
                if ((ctx->flags & 0x10) != 0) {
                    printf("[ERROR] Only one flags field\n");
                    return 0;
                }
                if (tbir_read_flags(ctx) == 0) {
                    return 0;
                }
                break;

            default:
                printf("invalide field\n");
                return 0;
        }
    }
    return 0; // Loop ended, but 0xaaee tag not found
}

// Function: tbir_read_width
undefined4 tbir_read_width(TbirContext *ctx) {
    uint32_t read_width = 0;
    if (ctx == NULL || tbir_read_nbits(ctx, 8, &read_width) == 0) {
        return 0;
    }

    if (read_width < 0x81) { // Max width 128
        ctx->width = read_width;
        return 1;
    }
    return 0;
}

// Function: tbir_read_height
undefined4 tbir_read_height(TbirContext *ctx) {
    if (ctx == NULL || tbir_read_nbits(ctx, 8, &ctx->height) == 0) {
        return 0;
    }
    if (ctx->height < 0x81) { // Max height 128
        return 1;
    }
    return 0;
}

// Function: tbir_read_header
undefined4 tbir_read_header(TbirContext *ctx) {
    if (ctx == NULL || tbir_read_width(ctx) == 0 || tbir_read_height(ctx) == 0) {
        return 0;
    }
    ctx->flags |= 1;
    return 1;
}

// Function: tbir_read_loadd
undefined4 tbir_read_loadd(TbirContext *ctx) {
    uint32_t read_loadd = 0;
    if (ctx == NULL || tbir_read_nbits(ctx, 4, &read_loadd) == 0) {
        return 0;
    }

    if (read_loadd < 9) { // Max load direction 8
        ctx->load_direction = read_loadd;
        ctx->flags |= 0x1000;
        return 1;
    }
    return 0;
}

// Function: tbir_read_pixeltype
undefined4 tbir_read_pixeltype(TbirContext *ctx) {
    uint32_t read_pixeltype = 0;
    if (ctx == NULL || tbir_read_nbits(ctx, 4, &read_pixeltype) == 0) {
        return 0;
    }

    // Valid pixel types: 1, 2, 6, 7
    if (read_pixeltype < 8 &&
        (read_pixeltype == 1 || read_pixeltype == 2 || read_pixeltype == 6 || read_pixeltype == 7)) {
        ctx->pixel_type = read_pixeltype;
        ctx->flags |= 0x100;
        return 1;
    }
    return 0;
}

// Function: tbir_read_typef
int tbir_read_typef(TbirContext *ctx) {
    uint32_t field_type = 0;
    if (tbir_read_nbits(ctx, 0x10, &field_type) == 0) {
        printf("da bits failed\n");
        return 0;
    }

    switch (field_type) {
        case 0xaaee:
        case 0xaadd:
        case 0xaacc:
        case 0xaaaa:
        case 0xaabb:
            return field_type;
        default:
            printf("wrong one\n");
            return 0;
    }
}

// Function: tbir_read_check
undefined4 tbir_read_check(TbirContext *ctx, int num_bits) {
    if (ctx == NULL || ctx->buffer_start_address == 0) {
        return 0;
    }

    uint32_t remaining_bits = ctx->buffer_total_bytes * 8 - (ctx->current_bit_offset + ctx->current_byte_offset * 8);

    if (num_bits < 0x21 && num_bits <= remaining_bits) { // 0x21 is 33, so max 32 bits can be read
        return 1;
    }
    return 0;
}

// Function: tbir_read_nbits
undefined4 tbir_read_nbits(TbirContext *ctx, int num_bits, uint32_t *out_value) {
    if (ctx == NULL || out_value == NULL || tbir_read_check(ctx, num_bits) == 0) {
        return 0;
    }

    uint32_t result = 0;
    int bits_in_current_byte = 8 - ctx->current_bit_offset;

    if (num_bits <= bits_in_current_byte) {
        uint8_t current_byte = *(uint8_t *)(ctx->buffer_start_address + ctx->current_byte_offset);
        result = (current_byte >> (bits_in_current_byte - num_bits)) & ((1U << num_bits) - 1);
        ctx->current_bit_offset += num_bits;
    } else {
        for (int i = 0; i < num_bits; ++i) {
            result <<= 1;
            uint8_t current_byte = *(uint8_t *)(ctx->buffer_start_address + ctx->current_byte_offset);
            result |= (current_byte >> (7 - ctx->current_bit_offset)) & 1U;

            ctx->current_bit_offset++;
            if (ctx->current_bit_offset >= 8) {
                ctx->current_bit_offset = 0;
                ctx->current_byte_offset++;
            }
        }
    }
    *out_value = result;
    return 1;
}

// Function: tbir_read_magic
undefined4 tbir_read_magic(TbirContext *ctx) {
    uint32_t magic_val = 0;
    if (tbir_read_nbits(ctx, 0x20, &magic_val) == 0) {
        return 0;
    }
    if (magic_val == 0xb0c4df76) { // -0x4f3b208a as unsigned int
        return 1;
    }
    return 0;
}