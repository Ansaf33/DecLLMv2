#include <stdlib.h> // For malloc, free
#include <string.h> // For memset, memcpy
#include <stdint.h> // For intptr_t

// Function: freeStream
void freeStream(int **param_1) {
    if (param_1 != NULL && *param_1 != NULL) {
        void **stream_info = (void **)*param_1;
        if (*stream_info != NULL) {
            free(*stream_info);
        }
        free(stream_info);
        *param_1 = NULL;
    }
}

// Function: initStream
void **initStream(void *param_1, void *param_2) {
    if (param_1 == NULL || param_2 == NULL) {
        return NULL;
    }

    void **stream_info = (void **)malloc(2 * sizeof(void *));
    if (stream_info == NULL) {
        return NULL;
    }

    memset(stream_info, 0, 2 * sizeof(void *));

    *stream_info = malloc((size_t)param_2);
    if (*stream_info == NULL) {
        free(stream_info);
        return NULL;
    }

    stream_info[1] = param_2;
    memcpy(*stream_info, param_1, (size_t)param_2);

    return stream_info;
}

// Function: readBits
int readBits(int *param_1, unsigned int param_2, unsigned int *param_3) {
    if (param_1 == NULL || param_3 == NULL || param_2 == 0 || param_2 > 32) {
        return 0;
    }

    if (param_1[0] == 0) {
        return 0;
    }

    unsigned int total_bits_in_buffer = (unsigned int)param_1[1] * 8;
    unsigned int current_bit_position = (unsigned int)param_1[2] * 8 + (unsigned int)param_1[3];
    unsigned int target_bit_position = current_bit_position + param_2;

    if (total_bits_in_buffer < target_bit_position) {
        return 0;
    }

    unsigned int bits_read_value = 0;
    int successful_bits_count = 0;

    for (unsigned int i = 0; i < param_2; ++i) {
        char *current_byte_ptr = (char *)((intptr_t)param_1[0] + param_1[2]);
        unsigned char current_byte = *current_byte_ptr;

        unsigned int bit = (current_byte >> (7 - (param_1[3] & 7))) & 1;

        bits_read_value = (bits_read_value << 1) | bit;

        param_1[3] = (param_1[3] + 1) & 7;
        if (param_1[3] == 0) {
            param_1[2]++;
        }
        successful_bits_count++;
    }

    *param_3 = bits_read_value;
    return successful_bits_count;
}