#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

// --- Global Variables and Constants ---
#define TX_BUFFER_SIZE 1024 // 0x400
char txbuf[TX_BUFFER_SIZE];
size_t txcnt = 0;
int txfd = -1; // Default to an invalid file descriptor

// Constants for limits in do_mix
float DAT_00016018 = 100.0f; // Placeholder value
float _DAT_0001601c = 200.0f; // Placeholder value

// --- Type Definitions for Decompiled Structures ---

// Structure for a state queue element
typedef struct State {
    int field0;
    uint32_t flags;
    uint32_t val1;
    uint32_t val2;
    uint32_t val3;
    uint32_t val4;
    uint32_t val5;
} State; // Total size 28 bytes (0x1C)

// StateQueue structure (dummy implementation)
#define MAX_HISTORY_SIZE 10
typedef struct StateQueue {
    State states[MAX_HISTORY_SIZE];
    int count;
} StateQueue;

// Global instance of the state queue
StateQueue g_history_storage = { .count = 0 };
StateQueue *g_history = &g_history_storage;

// Structure for the input to calculate_speed and param_2 in do_mix
typedef struct __attribute__((packed)) {
    uint8_t type_byte;
    int field0;
    float field1;
    float field2;
    float field3;
} SpeedInput; // Total size 17 bytes (0x11)

// Structure for the error message sent by send_error
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint32_t val1;
    uint32_t val2;
    uint8_t padding[8];
} ErrorMessage; // Total size 17 bytes (0x11)

// Structure for aggregating data in send_aggregate
typedef struct __attribute__((packed)) {
    uint32_t param_val;
    uint32_t state_val1;
    uint32_t state_val2;
    uint32_t state_val3;
    uint32_t state_val4;
    uint32_t state_val5;
    uint32_t flags_accumulated;
} AggregateData; // Total size 28 bytes (0x1c)

// Structure for the output message sent by send_aggregate
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint32_t param_val;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
} AggregateOutputMessage; // Total size 17 bytes (0x11)


// --- Dummy StateQueue Functions ---
int stateq_empty(StateQueue *q) {
    return q->count == 0;
}

int stateq_length(StateQueue *q) {
    return q->count;
}

void* stateq_get(StateQueue *q, int index) {
    if (index < 0 || index >= q->count) {
        return NULL;
    }
    return &(q->states[index]);
}

void* stateq_tail(StateQueue *q) {
    if (q->count == 0) {
        return NULL;
    }
    return &(q->states[q->count - 1]);
}

void stateq_add(StateQueue *q, State s) {
    if (q->count < MAX_HISTORY_SIZE) {
        q->states[q->count++] = s;
    } else {
        memmove(&q->states[0], &q->states[1], (MAX_HISTORY_SIZE - 1) * sizeof(State));
        q->states[MAX_HISTORY_SIZE - 1] = s;
    }
}


// --- Dummy transmit function ---
int transmit(int fd, const void *buf, size_t count, int *bytes_transmitted) {
    ssize_t written_bytes = write(fd, buf, count);
    if (written_bytes < 0) {
        *bytes_transmitted = 0;
        return -1; // Error
    }
    *bytes_transmitted = (int)written_bytes;
    return 0; // Success
}


// Function: writeflush
void writeflush(void) {
    ssize_t bytes_sent;
    size_t current_pos = 0;

    while (true) {
        if (txcnt <= current_pos) {
            txcnt = 0;
            return;
        }
        int transmit_result = transmit(txfd, txbuf + current_pos, txcnt - current_pos, (int*)&bytes_sent);
        if (transmit_result != 0) {
            break;
        }
        if (bytes_sent == 0) {
            txcnt = 0;
            return;
        }
        current_pos += bytes_sent;
    }
    txcnt = 0;
}

// Function: write (renamed to avoid clash with unistd.h write)
ssize_t write_wrapper(int fd, const void *buf, size_t n) {
    if ((TX_BUFFER_SIZE < n + txcnt) || (fd != txfd)) {
        writeflush();
    }
    txfd = fd;
    memcpy(txbuf + txcnt, buf, n);
    txcnt += n;
    return txcnt;
}

// Function: send_error
void send_error(uint32_t param_1, uint32_t param_2) {
    ErrorMessage msg;
    memset(&msg, 0, sizeof(ErrorMessage));
    msg.type = 0;
    msg.val1 = param_1;
    msg.val2 = param_2;
    write_wrapper(1, &msg, sizeof(ErrorMessage));
}

// Function: send_aggregate
void send_aggregate(uint32_t param_1) {
    if (stateq_empty(g_history) == 0) {
        AggregateData agg_data;
        memset(&agg_data, 0, sizeof(agg_data));
        agg_data.param_val = param_1;

        for (int i = stateq_length(g_history); i != 0; --i) {
            State *current_state = (State *)stateq_get(g_history, i - 1);
            if (current_state == NULL) continue;

            uint32_t current_state_flags = current_state->flags & ~agg_data.flags_accumulated;

            if ((current_state_flags & 1) != 0) {
                agg_data.state_val1 = current_state->val1;
                agg_data.state_val2 = current_state->val2;
                agg_data.state_val3 = current_state->val3;
                agg_data.flags_accumulated |= 1;
            }
            if ((current_state_flags & 2) != 0) {
                agg_data.state_val4 = current_state->val4;
                agg_data.flags_accumulated |= 2;
            }
            if ((current_state_flags & 4) != 0) {
                agg_data.state_val5 = current_state->val5;
                agg_data.flags_accumulated |= 4;
            }
        }

        AggregateOutputMessage out_msg;
        memset(&out_msg, 0, sizeof(out_msg));
        out_msg.param_val = agg_data.param_val;

        if ((agg_data.flags_accumulated & 2) != 0) {
            out_msg.type = 3;
            out_msg.data1 = agg_data.state_val4;
            write_wrapper(1, &out_msg, sizeof(out_msg));
        }
        if ((agg_data.flags_accumulated & 4) != 0) {
            out_msg.type = 4;
            out_msg.data1 = agg_data.state_val5;
            write_wrapper(1, &out_msg, sizeof(out_msg));
        }
        if ((agg_data.flags_accumulated & 1) != 0) {
            out_msg.type = 2;
            out_msg.data1 = agg_data.state_val1;
            out_msg.data2 = agg_data.state_val2;
            out_msg.data3 = agg_data.state_val3;
            write_wrapper(1, &out_msg, sizeof(out_msg));
        }
    }
}

// Function: calculate_speed
long double calculate_speed(const SpeedInput *input) {
    int i = stateq_length(g_history);
    State *state_ptr = NULL;

    while (i > 0) {
        state_ptr = (State *)stateq_get(g_history, i - 1);
        if (state_ptr == NULL) {
            return 0.0L;
        }
        if ((state_ptr->field0 != input->field0) && ((state_ptr->flags & 1U) != 0)) {
            break;
        }
        i--;
        state_ptr = NULL;
    }

    if (state_ptr == NULL) {
        return 0.0L;
    }

    float fVar2_val = exp2f((float)state_ptr->val1 - input->field1);
    float fVar3_val = exp2f((float)state_ptr->val2 - input->field2);
    float fVar4_val = exp2f((float)state_ptr->val3 - input->field3);

    float result_sqrt = sqrtf(fVar4_val + fVar3_val + fVar2_val);

    int time_diff = input->field0 - state_ptr->field0;
    if (time_diff == 0) {
        return 0.0L;
    }

    return (long double)result_sqrt / (long double)time_diff;
}

// Function: do_mix
uint32_t do_mix(State *output_state, const SpeedInput *input_data) {
    uint8_t type_byte = input_data->type_byte;

    if (type_byte == 4) {
        if ((input_data->field1 < 0.0f) || (_DAT_0001601c <= input_data->field1)) {
            send_error(3, 4);
            return 0;
        }
    } else if (type_byte == 3) {
        if ((input_data->field1 < 0.0f) || (DAT_00016018 <= input_data->field1)) {
            send_error(3, 3);
            return 0;
        }
    } else if (type_byte < 2 || type_byte > 4) {
        return 0;
    }

    State *previous_state = NULL;
    if ((stateq_empty(g_history) == 0) && (((State*)stateq_tail(g_history))->field0 != input_data->field0)) {
        previous_state = (State *)stateq_tail(g_history);
    } else {
        if (stateq_length(g_history) >= 2) {
            previous_state = (State *)stateq_get(g_history, stateq_length(g_history) - 2);
        }
    }

    if ((previous_state == NULL) || (type_byte != 2) ||
        (calculate_speed(input_data) <= (long double)DAT_00016018)) {
        if (type_byte == 4) {
            output_state->val5 = *(uint32_t*)&input_data->field1;
            output_state->flags |= 4;
        } else if (type_byte == 2) {
            output_state->val1 = *(uint32_t*)&input_data->field1;
            output_state->val2 = *(uint32_t*)&input_data->field2;
            output_state->val3 = *(uint32_t*)&input_data->field3;
            output_state->flags |= 1;
        } else if (type_byte == 3) {
            output_state->val4 = *(uint32_t*)&input_data->field1;
            output_state->flags |= 2;
        }
        return 1;
    } else {
        send_error(3, 2);
        return 0;
    }
}

// Function: do_hash
int do_hash(const void *data, size_t data_len) {
    if (data_len == 0) {
        return 0;
    }

    void *temp_buffer = malloc(data_len);
    if (temp_buffer == NULL) {
        return -1;
    }

    int total_sum = 0;
    uint32_t hash_seed = 0x12345678;
    memcpy(temp_buffer, data, data_len);

    for (size_t i = 0; i <= data_len - 4; i += 4) {
        uint32_t *word_ptr = (uint32_t *)((char *)temp_buffer + i);
        *word_ptr ^= hash_seed;
        hash_seed ^= hash_seed >> 0xd;
        hash_seed ^= hash_seed << 7;
        hash_seed ^= hash_seed >> 0x11;
    }

    for (size_t i = 0; i <= data_len - 4; i += 4) {
        int *int_ptr = (int *)((char *)temp_buffer + i);
        total_sum += *int_ptr;
    }

    free(temp_buffer);
    return total_sum;
}

// --- Main function ---
int main() {
    return 0;
}