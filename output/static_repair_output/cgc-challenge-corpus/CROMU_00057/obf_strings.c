#include <stdio.h>  // Required for printf
#include <string.h> // Required for general string operations, though not directly used in the optimized loop
#include <stddef.h> // Required for size_t

// Define a static global buffer for the obfuscated string.
// A fixed size is chosen; consider dynamic allocation or passing a buffer
// in a real-world scenario to avoid buffer overflows for arbitrary input sizes.
static char tmp_string_0[256];

/**
 * @brief Obfuscates a given string by XORing each character with 0xa9.
 *        The result is stored in an internal static buffer `tmp_string_0`.
 *        Handles potential buffer overflow by truncating the output if the
 *        input string is longer than the buffer's capacity.
 *
 * @param param_1 The input string to obfuscate. It is treated as constant.
 * @return A pointer to the internally stored obfuscated string (`tmp_string_0`).
 *         Note: Subsequent calls will overwrite the previous result in `tmp_string_0`.
 */
char *obf_strings(const char *param_1) {
    size_t i = 0;
    // Iterate through the input string until the null terminator is found
    // or the internal buffer limit (minus one for the null terminator) is reached.
    while (param_1[i] != '\0' && i < sizeof(tmp_string_0) - 1) {
        tmp_string_0[i] = param_1[i] ^ 0xa9;
        i++;
    }
    // Null-terminate the obfuscated string.
    tmp_string_0[i] = '\0';
    return tmp_string_0;
}

/**
 * @brief Main function to demonstrate the usage of obf_strings.
 */
int main() {
    // Test string 1
    const char *original_str1 = "Hello, World!";
    printf("Original 1: \"%s\"\n", original_str1);

    char *obfuscated_str1 = obf_strings(original_str1);
    printf("Obfuscated 1: \"%s\"\n", obfuscated_str1);

    // To deobfuscate, apply the same XOR operation.
    // A temporary buffer is used to store the deobfuscated string for printing.
    char deobfuscated_buf[sizeof(tmp_string_0)];
    size_t i = 0;
    while (obfuscated_str1[i] != '\0') {
        deobfuscated_buf[i] = obfuscated_str1[i] ^ 0xa9;
        i++;
    }
    deobfuscated_buf[i] = '\0';
    printf("Deobfuscated 1: \"%s\"\n\n", deobfuscated_buf);

    // Test string 2 (longer string to test buffer overflow truncation)
    const char *original_str2 = "This is a much longer string that will test the buffer limit. If it were even longer, it would be truncated. Let's make it long enough to be slightly over 200 characters to ensure the truncation logic is tested properly with a realistic length that could exceed a small default buffer.";
    printf("Original 2: \"%s\"\n", original_str2);

    char *obfuscated_str2 = obf_strings(original_str2);
    printf("Obfuscated 2 (truncated if too long): \"%s\"\n", obfuscated_str2);

    // Deobfuscate string 2
    i = 0;
    while (obfuscated_str2[i] != '\0') {
        deobfuscated_buf[i] = obfuscated_str2[i] ^ 0xa9;
        i++;
    }
    deobfuscated_buf[i] = '\0';
    printf("Deobfuscated 2: \"%s\"\n\n", deobfuscated_buf);

    // Test with an empty string
    const char *empty_str = "";
    printf("Original empty: \"%s\"\n", empty_str);
    char *obfuscated_empty = obf_strings(empty_str);
    printf("Obfuscated empty: \"%s\"\n", obfuscated_empty);
    // Deobfuscate empty string
    i = 0;
    while (obfuscated_empty[i] != '\0') {
        deobfuscated_buf[i] = obfuscated_empty[i] ^ 0xa9;
        i++;
    }
    deobfuscated_buf[i] = '\0';
    printf("Deobfuscated empty: \"%s\"\n\n", deobfuscated_buf);

    return 0;
}