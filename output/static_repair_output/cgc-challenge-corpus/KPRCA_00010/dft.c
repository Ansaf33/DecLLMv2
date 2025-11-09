#include <stdlib.h> // For malloc, free
#include <math.h>   // For atan2, cos, sin, log2, round, ceil
#include <string.h> // For memset, memcpy

// Define M_PI if not available (e.g., if _GNU_SOURCE is not defined)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Structure for a complex number (real and imaginary parts)
typedef struct {
    double real;
    double imag;
} complex_double;

// Helper to check if a number is a power of 2
static inline int is_power_of_two(unsigned int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

// Helper to get the smallest power of 2 greater than or equal to n
static inline unsigned int next_power_of_two(unsigned int n) {
    if (n == 0) return 1;
    if (is_power_of_two(n)) return n;
    // Compute next power of 2 (for 32-bit unsigned int)
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

// Function: cfft
// Computes the Cooley-Tukey Radix-2 DIT FFT recursively.
// param_1: Pointer to an array of complex_double (input)
// param_2: Number of complex samples (must be a power of 2)
// Returns: Pointer to a new array of complex_double (output), or NULL on error
complex_double *cfft(complex_double *param_1, unsigned int param_2) {
    if (param_2 == 0) return NULL;

    // Base case: if only one sample, return it as is (copy to new memory)
    if (param_2 == 1) {
        complex_double *result = (complex_double *)malloc(sizeof(complex_double));
        if (!result) return NULL;
        *result = *param_1;
        return result;
    }

    const double TWO_PI = 2.0 * M_PI;
    unsigned int half_N = param_2 / 2;

    // Allocate memory for even and odd indexed inputs
    complex_double *even_input = (complex_double *)malloc(half_N * sizeof(complex_double));
    complex_double *odd_input = (complex_double *)malloc(half_N * sizeof(complex_double));

    if (!even_input || !odd_input) {
        free(even_input); // Safe to call free on NULL
        free(odd_input);
        return NULL;
    }

    // Split input into even and odd parts
    for (unsigned int i = 0; i < half_N; ++i) {
        even_input[i] = param_1[2 * i];
        odd_input[i] = param_1[2 * i + 1];
    }

    // Recursively compute FFT for even and odd parts
    complex_double *even_output = cfft(even_input, half_N);
    complex_double *odd_output = cfft(odd_input, half_N);

    // Free temporary input arrays
    free(even_input);
    free(odd_input);

    if (!even_output || !odd_output) {
        free(even_output);
        free(odd_output);
        return NULL;
    }

    // Allocate memory for the result
    complex_double *result = (complex_double *)malloc(param_2 * sizeof(complex_double));
    if (!result) {
        free(even_output);
        free(odd_output);
        return NULL;
    }

    // Combine results using butterfly operation
    for (unsigned int k = 0; k < half_N; ++k) {
        double angle = (double)k * TWO_PI / param_2;
        double cos_val = cos(angle);
        double sin_val = sin(angle);

        // Twiddle factor W_k = cos(angle) - i*sin(angle) for forward FFT
        // term = W_k * odd_output[k]
        double term_real = odd_output[k].real * cos_val + odd_output[k].imag * sin_val;
        double term_imag = odd_output[k].imag * cos_val - odd_output[k].real * sin_val;

        // X[k] = even_output[k] + term
        result[k].real = even_output[k].real + term_real;
        result[k].imag = even_output[k].imag + term_imag;

        // X[k + half_N] = even_output[k] - term
        result[k + half_N].real = even_output[k].real - term_real;
        result[k + half_N].imag = even_output[k].imag - term_imag;
    }

    // Free recursive call results
    free(even_output);
    free(odd_output);

    return result;
}

// Function: fft
// Computes the Fast Fourier Transform for real input.
// param_1: Pointer to an array of doubles (real input signal)
// param_2: Number of samples
// Returns: Pointer to a new array of complex_double (complex output), or NULL on error
complex_double *fft(double *param_1, unsigned int param_2) {
    complex_double *complex_input = (complex_double *)malloc(param_2 * sizeof(complex_double));
    if (!complex_input) return NULL;

    // Convert real input to complex input (imaginary parts are zero)
    for (unsigned int i = 0; i < param_2; ++i) {
        complex_input[i].real = param_1[i];
        complex_input[i].imag = 0.0;
    }

    complex_double *result = cfft(complex_input, param_2);
    free(complex_input); // Free the temporary complex input array

    return result;
}

// Function: dft (Discrete Fourier Transform)
// Computes the DFT, handling non-power-of-2 input by zero-padding.
// param_1: Pointer to an array of doubles (real input signal)
// param_2: Original number of samples
// param_3: Output parameter, stores the actual size of the transformed array (padded size)
// Returns: Pointer to a new array of complex_double (complex output), or NULL on error
complex_double *dft(double *param_1, unsigned int param_2, unsigned int *param_3) {
    complex_double *result = NULL;
    unsigned int final_N;

    if (param_2 == 0) {
        *param_3 = 0;
        return NULL;
    }

    if (is_power_of_two(param_2)) {
        // If input size is already a power of 2, call fft directly
        result = fft(param_1, param_2);
        final_N = param_2;
    } else {
        // If not a power of 2, pad with zeros to the next power of 2
        unsigned int padded_N = next_power_of_two(param_2);
        double *padded_input = (double *)malloc(padded_N * sizeof(double));
        if (!padded_input) {
            *param_3 = 0;
            return NULL;
        }
        // Initialize padded_input with zeros
        memset(padded_input, 0, padded_N * sizeof(double));
        // Copy original input data
        memcpy(padded_input, param_1, param_2 * sizeof(double));

        result = fft(padded_input, padded_N);
        free(padded_input); // Free temporary padded input array
        final_N = padded_N;
    }

    if (result == NULL) {
        *param_3 = 0;
    } else {
        *param_3 = final_N;
    }
    return result;
}

// Function: idft (Inverse Discrete Fourier Transform)
// Computes the IDFT, handling non-power-of-2 input by zero-padding.
// param_1: Pointer to an array of complex_double (complex input signal).
//          Note: This array's imaginary parts will be conjugated during the process.
// param_2: Original number of complex samples
// param_3: Output parameter, stores the actual size of the transformed array (padded size)
// Returns: Pointer to a new array of doubles (real output signal), or NULL on error
double *idft(complex_double *param_1, unsigned int param_2, unsigned int *param_3) {
    if (param_2 == 0) {
        *param_3 = 0;
        return NULL;
    }

    // Conjugate input for IFFT (flip sign of imaginary parts)
    for (unsigned int i = 0; i < param_2; ++i) {
        param_1[i].imag = -param_1[i].imag;
    }

    complex_double *cfft_result = NULL;
    unsigned int final_N;

    if (is_power_of_two(param_2)) {
        // If input size is already a power of 2, call cfft directly
        cfft_result = cfft(param_1, param_2);
        final_N = param_2;
    } else {
        // If not a power of 2, pad with zeros to the next power of 2
        unsigned int padded_N = next_power_of_two(param_2);
        complex_double *padded_input = (complex_double *)malloc(padded_N * sizeof(complex_double));
        if (!padded_input) {
            // Revert original input's imaginary parts if allocation fails
            for (unsigned int i = 0; i < param_2; ++i) {
                param_1[i].imag = -param_1[i].imag;
            }
            *param_3 = 0;
            return NULL;
        }
        // Initialize padded_input with zeros
        memset(padded_input, 0, padded_N * sizeof(complex_double));
        // Copy original input data
        memcpy(padded_input, param_1, param_2 * sizeof(complex_double));

        cfft_result = cfft(padded_input, padded_N);
        free(padded_input); // Free temporary padded input array
        final_N = padded_N;
    }

    // Revert original input's imaginary parts (param_1 is modified)
    // This restores param_1 to its original state before the function call.
    for (unsigned int i = 0; i < param_2; ++i) {
        param_1[i].imag = -param_1[i].imag;
    }

    double *final_result = NULL;
    if (cfft_result != NULL) {
        final_result = (double *)malloc(final_N * sizeof(double));
        if (final_result == NULL) {
            free(cfft_result);
            *param_3 = 0;
            return NULL;
        }
        // Extract real parts and scale by 1/N
        for (unsigned int i = 0; i < final_N; ++i) {
            final_result[i] = cfft_result[i].real / final_N;
        }
        free(cfft_result); // Free temporary complex result array
        *param_3 = final_N;
    } else {
        *param_3 = 0;
    }

    return final_result;
}