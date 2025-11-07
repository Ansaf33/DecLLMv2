#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <byteswap.h> // For __bswap_32
#include <libintl.h>  // For gettext
#include <error.h>    // For error
#include <limits.h>   // For ULONG_MAX

// --- External declarations (assuming these are defined elsewhere) ---

// Global variables (from decompiled snippet)
extern char cksum_debug;
extern void *cksum_pclmul; // Pointer to function or data structure for pclmul implementation
extern void *cksum_avx2;   // Pointer to function or data structure for avx2 implementation
extern void *cksum_avx512; // Pointer to function or data structure for avx512 implementation

extern const char DAT_00100c10[]; // String literal like "sse4.2"
extern const char DAT_00100c19[]; // String literal like "avx"
extern const char DAT_00100cb8[]; // Format string for error, e.g., "%s"

extern uint32_t _DAT_0010101c;     // CPU feature flags (e.g., from /proc/cpuinfo)
extern uint32_t ___cpu_features2;  // More CPU feature flags

// CRC lookup table (assuming it's an array of uint64_t)
extern uint64_t crctab[];

// External functions (from glibc/system)
extern uint64_t gcc_feature_to_glibc_hwcap(const char *);
extern char hwcap_allowed(uint64_t);
// gettext and error included from libintl.h and error.h
// __errno_location included from errno.h
extern uint32_t crc32_update(uint32_t crc, const void *buf, size_t len);

// Function pointer for chosen checksum implementation
typedef int (*cksum_func_ptr)(FILE *, uint64_t *, uint64_t *);
extern cksum_func_ptr cksum_fp_0;

// --- Function Prototypes (for functions defined below) ---
void *pclmul_supported(void);
void *avx2_supported(void);
void *avx512_supported(void);
void *vmull_supported(void);
int cksum_slice8(FILE *file, uint64_t *crc_out, uint64_t *total_bytes_out);


// Function: pclmul_supported
void *pclmul_supported(void) {
    bool supported = false;

    if (hwcap_allowed(gcc_feature_to_glibc_hwcap(DAT_00100c10)) &&
        ((_DAT_0010101c & 0x200) != 0)) {
        if (hwcap_allowed(gcc_feature_to_glibc_hwcap("pclmul")) &&
            ((_DAT_0010101c & 0x80000) != 0)) {
            supported = true;
        }
    }

    if (cksum_debug) {
        error(0, 0, DAT_00100cb8,
              gettext(supported ? "using pclmul hardware support" : "pclmul support not detected"));
    }

    return supported ? &cksum_pclmul : NULL;
}

// Function: avx2_supported
void *avx2_supported(void) {
    bool supported = false;

    if (hwcap_allowed(gcc_feature_to_glibc_hwcap(DAT_00100c19)) &&
        ((_DAT_0010101c & 0x400) != 0)) {
        if (hwcap_allowed(gcc_feature_to_glibc_hwcap("vpclmulqdq")) &&
            ((___cpu_features2 & 2) != 0)) {
            supported = true;
        }
    }

    if (cksum_debug) {
        error(0, 0, DAT_00100cb8,
              gettext(supported ? "using avx2 hardware support" : "avx2 support not detected"));
    }

    return supported ? &cksum_avx2 : NULL;
}

// Function: avx512_supported
void *avx512_supported(void) {
    bool supported = false;

    if (hwcap_allowed(gcc_feature_to_glibc_hwcap("avx512f")) &&
        ((_DAT_0010101c & 0x8000) != 0)) {
        if (hwcap_allowed(gcc_feature_to_glibc_hwcap("avx512bw")) &&
            ((_DAT_0010101c & 0x200000) != 0)) {
            if (hwcap_allowed(gcc_feature_to_glibc_hwcap("vpclmulqdq")) &&
                ((___cpu_features2 & 2) != 0)) {
                supported = true;
            }
        }
    }

    if (cksum_debug) {
        error(0, 0, DAT_00100cb8,
              gettext(supported ? "using avx512 hardware support" : "avx512 support not detected"));
    }

    return supported ? &cksum_avx512 : NULL;
}

// Function: vmull_supported
void *vmull_supported(void) {
  return NULL;
}

// Function: cksum_slice8
// Returns 1 on success, 0 on failure.
int cksum_slice8(FILE *file, uint64_t *crc_out, uint64_t *total_bytes_out) {
    uint64_t crc = 0;
    uint64_t total_bytes = 0;
    char buffer[0x10000]; // 65536 bytes
    size_t bytes_read;

    if (!file || !crc_out || !total_bytes_out) {
        return 0; // Failure
    }

    do {
        bytes_read = fread_unlocked(buffer, 1, sizeof(buffer), file);
        if (bytes_read == 0) {
            break;
        }

        // Check for overflow before adding
        if (total_bytes > ULONG_MAX - bytes_read) {
            *__errno_location() = EOVERFLOW; // 0x4b is EOVERFLOW
            return 0; // Failure
        }
        total_bytes += bytes_read;

        size_t i = 0;
        // Process 8-byte chunks
        for (; i + 7 < bytes_read; i += 8) {
            uint32_t val1 = *(uint32_t *)(buffer + i);
            uint32_t val2 = *(uint32_t *)(buffer + i + 4);

            crc ^= __bswap_32(val1);
            uint32_t bswapped_val2 = __bswap_32(val2);

            // This is a specific CRC algorithm, likely CRC-32-C or similar,
            // using an 8-byte parallel lookup table.
            crc = crctab[(bswapped_val2 & 0xff)] ^
                  crctab[((crc >> 0x18 & 0xff) + 0x700)] ^
                  crctab[((crc >> 0x10 & 0xff) + 0x600)] ^
                  crctab[((crc >> 8 & 0xff) + 0x500)] ^
                  crctab[((crc & 0xff) + 0x400)] ^
                  crctab[((bswapped_val2 >> 0x18) + 0x300)] ^
                  crctab[((bswapped_val2 >> 0x10 & 0xff) + 0x200)] ^
                  crctab[((bswapped_val2 >> 8 & 0xff) + 0x100)];
        }

        // Process remaining bytes
        for (; i < bytes_read; ++i) {
            crc = crctab[(uint8_t)(buffer[i] ^ (crc >> 0x18))] ^ (crc << 8);
        }

    } while (!feof_unlocked(file));

    *crc_out = crc;
    *total_bytes_out = total_bytes;

    return ferror_unlocked(file) == 0 ? 1 : 0; // 1 for success, 0 for error
}

// Function: crc_sum_stream
// Returns 0 on success, 0xffffffff on failure.
int crc_sum_stream(FILE *file, uint32_t *crc_out, uint64_t *total_bytes_out) {
    uint64_t current_crc = 0;
    uint64_t total_len = 0;

    // Initialize function pointer if not set
    if (cksum_fp_0 == NULL) {
        cksum_fp_0 = (cksum_func_ptr)avx512_supported();
    }
    if (cksum_fp_0 == NULL) {
        cksum_fp_0 = (cksum_func_ptr)avx2_supported();
    }
    if (cksum_fp_0 == NULL) {
        cksum_fp_0 = (cksum_func_ptr)pclmul_supported();
    }
    if (cksum_fp_0 == NULL) {
        cksum_fp_0 = (cksum_func_ptr)vmull_supported();
    }
    if (cksum_fp_0 == NULL) {
        cksum_fp_0 = cksum_slice8;
    }

    if ((*cksum_fp_0)(file, &current_crc, &total_len)) { // Check if the called function returns success (1)
        *total_bytes_out = total_len;
        uint64_t len_temp = total_len; // Use a temporary for the loop
        for (; len_temp != 0; len_temp >>= 8) {
            current_crc = crctab[(uint8_t)((uint8_t)len_temp ^ (current_crc >> 0x18))] ^ (current_crc << 8);
        }
        *crc_out = (uint32_t)~current_crc;
        return 0; // Success
    } else {
        return 0xffffffff; // Failure
    }
}

// Function: crc32b_sum_stream
// Returns 0 on success, 0xffffffff on failure.
int crc32b_sum_stream(FILE *file, uint32_t *crc_out, uint64_t *total_bytes_out) {
    uint32_t current_crc = 0;
    uint64_t total_bytes = 0;
    char buffer[0x10000]; // 65536 bytes
    size_t bytes_read;

    if (!file || !crc_out || !total_bytes_out) {
        return 0xffffffff; // Failure
    }

    if (cksum_debug) {
        // This call seems to be for debug logging, return value is ignored.
        pclmul_supported();
    }

    do {
        bytes_read = fread_unlocked(buffer, 1, sizeof(buffer), file);
        if (bytes_read == 0) {
            break;
        }

        // Check for overflow before adding
        if (total_bytes > ULONG_MAX - bytes_read) {
            *__errno_location() = EOVERFLOW; // 0x4b is EOVERFLOW
            return 0xffffffff; // Failure
        }
        total_bytes += bytes_read;

        current_crc = crc32_update(current_crc, buffer, bytes_read);

    } while (!feof_unlocked(file));

    *crc_out = current_crc;
    *total_bytes_out = total_bytes;

    return ferror_unlocked(file) == 0 ? 0 : 0xffffffff; // 0 for success, 0xffffffff for error
}

// Function: output_crc
void output_crc(const char *filename, uint64_t param_2_unused, uint32_t *crc_val, bool binary_output, uint64_t param_5_unused,
               int newline_char, bool print_filename, uint64_t total_bytes) {
    (void)param_2_unused; // Mark as unused
    (void)param_5_unused; // Mark as unused

    if (!binary_output) {
        printf("%u %llu", *crc_val, (unsigned long long)total_bytes);
        if (print_filename) {
            printf(" %s", filename);
        }
        putchar_unlocked(newline_char);
    } else {
        uint32_t bswapped_crc = __bswap_32(*crc_val);
        fwrite_unlocked(&bswapped_crc, 1, sizeof(uint32_t), stdout);
    }
}