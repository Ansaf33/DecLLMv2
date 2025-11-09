#include <stdint.h>
#include <string.h>
#include <stdlib.h> // For exit
#include <stddef.h> // For size_t
#include <stdio.h>  // For potential debugging, though not strictly used in current logic

// External functions (signatures inferred based on usage)
extern int recv_all(size_t size, void *buffer);
extern void checksum(uint32_t p1, uint32_t p2, uint32_t *p3, uint32_t p4, void *p5);
extern uint32_t run_option_transaction(uint32_t p1, const uint8_t *p2, uint32_t p3);
extern int transmit_all(size_t size, const void *buffer);
extern uint32_t gen_order_fill_msg(uint32_t *p1, uint32_t p2, const uint8_t *p3, uint32_t p4, uint32_t p5);
extern long double get_current_ask(void);
extern void init_accounts(void);

// Function: get_data_len
uint32_t get_data_len(const uint8_t *param_1) {
    uint32_t uVar1 = *(const uint32_t *)(param_1 + 4);
    if (uVar1 == 0xdd00) {
        return 8;
    }
    if (uVar1 == 0 || uVar1 == 0xff) {
        return 0x2c;
    }
    return 0;
}

// Function: recv_rest
int recv_rest(uint8_t *param_1) {
    uint32_t data_len = get_data_len(param_1);
    return recv_all(data_len, param_1 + 0x18);
}

// Function: err_resp
uint32_t err_resp(int error_code, const uint8_t *param_2, uint32_t *param_3) {
    if (error_code == 0xee) {
        memcpy(*(void **)(param_2 + 0x10), param_3 + 5, *(const size_t *)(param_2 + 0xc));
        param_3[3] = *(const uint32_t *)(param_2 + 0xc);
        return param_3[3];
    } else {
        param_3[5] = error_code;
        param_3[2] = *(const uint32_t *)(param_2 + 8);
        param_3[1] = 0xffff;
        param_3[0] = 1;
        return 0x18;
    }
}

// Function: checksum_order
uint32_t checksum_order(uint32_t param_1, const uint32_t *param_2, void *param_3) {
    checksum(param_1, param_2[1], (uint32_t *)(param_2 + 2), param_2[0], param_3);
    return (memcmp(param_3, param_2 + 4, 0x20) == 0) ? 0 : 0xcc;
}

// Function: run_sell
void run_sell(uint8_t *param_1, uint32_t *param_2) {
    uint8_t local_48[32];
    const uint8_t *order_data_ptr = param_1 + 0x14;
    uint32_t param_1_val8 = *(const uint32_t *)(param_1 + 8);
    uint32_t param_1_val4 = *(const uint32_t *)(param_1 + 4);

    if (checksum_order(param_1_val8, (const uint32_t *)order_data_ptr, local_48) != 0) {
        exit(0x15);
    }

    uint32_t transaction_result = run_option_transaction(param_1_val8, order_data_ptr, param_1_val4);

    if (transaction_result != 0) {
        uint32_t err_resp_len = err_resp(transaction_result, param_1, param_2);
        if (transmit_all(err_resp_len, param_2) == 0) {
            return;
        }
        exit(0x2c);
    }

    uint32_t order_data_val4 = *(const uint32_t *)(order_data_ptr + 4);
    uint32_t msg_len = gen_order_fill_msg(param_2, 0xff, order_data_ptr + 8, order_data_val4, param_1_val8);

    if (transmit_all(msg_len + 0x18, param_2) != 0) {
        exit(0x10);
    }
}

// Function: run_buy
void run_buy(uint8_t *param_1, uint32_t *param_2) {
    uint8_t local_44[32];
    const uint8_t *order_data_ptr = param_1 + 0x14;
    uint32_t param_1_val8 = *(const uint32_t *)(param_1 + 8);
    uint32_t param_1_val4 = *(const uint32_t *)(param_1 + 4);

    if (checksum_order(param_1_val8, (const uint32_t *)order_data_ptr, local_44) != 0) {
        exit(0x1c);
    }

    uint32_t transaction_result = run_option_transaction(param_1_val8, order_data_ptr, param_1_val4);

    if (transaction_result != 0) {
        uint32_t err_resp_len = err_resp(transaction_result, param_1, param_2);
        if (transmit_all(err_resp_len, param_2) != 0) {
            exit(7);
        }
        if (transaction_result != 0xee) {
            return;
        }
        exit(0x40);
    }

    uint32_t order_data_val4 = *(const uint32_t *)(order_data_ptr + 4);
    uint32_t msg_len = gen_order_fill_msg(param_2, 0, order_data_ptr + 8, order_data_val4, param_1_val8);

    if (transmit_all(msg_len + 0x18, param_2) != 0) {
        exit(0xf);
    }
}

// Function: run_quote
void run_quote(uint8_t *param_1, uint32_t *param_2) {
    float *quote_val_ptr = (float *)(param_1 + 0x14);

    if ((uintptr_t)quote_val_ptr != (uintptr_t)0xfffffffc) {
        *quote_val_ptr = (float)get_current_ask();
        uint32_t data_len = get_data_len(param_1);

        memcpy(param_1, param_2, data_len + 0x18);
        param_2[0] = 1;

        if (transmit_all(data_len + 0x18, param_1) == 0) {
            return;
        }
        exit(0x2c);
    }
    exit(0x2c);
}

// Function: main
void main(void) {
    uint32_t input_msg_buffer[4096];
    uint8_t output_msg_buffer[24];

    init_accounts();

    while (1) {
        memset(input_msg_buffer, 0, 0x18);

        if (recv_all(0x18, input_msg_buffer) != 0) {
            exit(1);
        }

        if (input_msg_buffer[0] != 0) {
            exit(2);
        }
        if (input_msg_buffer[2] > 99) {
            exit(3);
        }

        if (recv_rest((uint8_t *)input_msg_buffer) != 0) {
            exit(recv_rest((uint8_t *)input_msg_buffer)); // This call will be redundant or re-execute if error. Assuming the first call returns the error.
        }

        uint32_t command_id = input_msg_buffer[1];

        if (command_id == 0xdd00) {
            run_quote((uint8_t *)input_msg_buffer, output_msg_buffer);
        } else if (command_id == 0) {
            run_buy((uint8_t *)input_msg_buffer, output_msg_buffer);
        } else if (command_id == 0xff) {
            run_sell((uint8_t *)input_msg_buffer, output_msg_buffer);
        } else if (command_id == 0xffff) {
            exit(0x14);
        }
    }
}