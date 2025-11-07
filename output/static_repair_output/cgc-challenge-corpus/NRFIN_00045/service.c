#include <stdint.h> // For uint32_t, uint8_t
#include <string.h> // For memcpy, memcmp, memset
#include <stdlib.h> // For exit

// Declare external functions.
// These are placeholders for functions whose definitions are not provided.
// Their signatures are inferred from their usage in the decompiled code.
extern void init_accounts(void);
extern int recv_all(uint32_t size, void* buffer); // Assumed to return 0 on success, non-zero on error
extern int run_option_transaction(uint32_t arg1, void* arg2, uint32_t arg3); // Assumed to return 0 on success, non-zero on error
extern int gen_order_fill_msg(uint32_t* resp_buf, uint32_t arg2, void* arg3, uint32_t arg4, uint32_t arg5); // Assumed to return message length
extern int transmit_all(uint32_t size, void* buffer); // Assumed to return 0 on success, non-zero on error
extern long double get_current_ask(void); // Returns long double
extern void checksum(uint32_t param_1, uint32_t param_2, const uint32_t* param_3, uint32_t param_4, void* output_buffer); // Inferred signature

// Function: get_data_len
uint32_t get_data_len(const void* param_1) {
  uint32_t data_type = *(const uint32_t*)((const uint8_t*)param_1 + 4);
  if (data_type == 0xdd00) {
    return 8;
  }
  if (data_type == 0) {
    return 0x2c; // 44
  }
  if (data_type == 0xff) {
    return 0x2c; // 44
  }
  return 0;
}

// Function: recv_rest
// Modified to return int based on main's usage of its return value
int recv_rest(void* param_1) {
  uint32_t data_len = get_data_len(param_1);
  return recv_all(data_len, (uint8_t*)param_1 + 0x18);
}

// Function: err_resp
uint32_t err_resp(int error_code, const void* param_2, uint32_t* param_3) {
  if (error_code == 0xee) {
    void* dest_ptr = *(void**)((const uint8_t*)param_2 + 0x10);
    const uint32_t* src_ptr = param_3 + 5;
    size_t len = *(const size_t*)((const uint8_t*)param_2 + 0xc);
    memcpy(dest_ptr, src_ptr, len);
    param_3[3] = *(const uint32_t*)((const uint8_t*)param_2 + 0xc);
    return param_3[3];
  } else {
    param_3[5] = error_code;
    param_3[2] = *(const uint32_t*)((const uint8_t*)param_2 + 8);
    param_3[1] = 0xffff;
    param_3[0] = 1;
    return 0x18; // 24
  }
}

// Function: checksum_order
// Arguments inferred based on common patterns and run_sell/run_buy usage.
uint32_t checksum_order(uint32_t message_type, const uint32_t* message_data_ptr, void* calculated_checksum_buffer) {
  // The `checksum` function is assumed to calculate the checksum and write it into `calculated_checksum_buffer`.
  // `message_data_ptr + 4` is assumed to hold the *expected* checksum for comparison.
  checksum(message_type, message_data_ptr[1], message_data_ptr + 2, message_data_ptr[0], calculated_checksum_buffer);

  if (memcmp(calculated_checksum_buffer, message_data_ptr + 4, 0x20) == 0) {
    return 0; // Success
  } else {
    return 0xcc; // Failure
  }
}

// Function: run_sell
void run_sell(void* msg_ptr, uint32_t* resp_buf) {
  uint8_t checksum_buffer[32];
  uint8_t* order_data_ptr = (uint8_t*)msg_ptr + 0x14;

  // Arguments for checksum_order are inferred based on common message structures
  // and the usage of msg_ptr in the decompiled code.
  if (checksum_order(*(uint32_t*)((uint8_t*)msg_ptr + 4), (const uint32_t*)((uint8_t*)msg_ptr + 8), checksum_buffer) != 0) {
    exit(0x15);
  }

  // Arguments for run_option_transaction are inferred.
  int transaction_result = run_option_transaction(*(uint32_t*)((uint8_t*)msg_ptr + 8), order_data_ptr, *(uint32_t*)((uint8_t*)msg_ptr + 4));

  if (transaction_result != 0) {
    uint32_t error_response_len = err_resp(transaction_result, msg_ptr, resp_buf);
    if (transmit_all(error_response_len, resp_buf) != 0) {
      exit(0x2c);
    }
    return;
  }

  // Arguments for gen_order_fill_msg are inferred.
  int order_fill_msg_len = gen_order_fill_msg(resp_buf, 0xff, (uint8_t*)order_data_ptr + 8,
                                                *(uint32_t*)((uint8_t*)order_data_ptr + 4),
                                                *(uint32_t*)((uint8_t*)msg_ptr + 8));

  if (transmit_all(order_fill_msg_len + 0x18, resp_buf) != 0) {
    exit(0x10);
  }
}

// Function: run_buy
void run_buy(void* msg_ptr, uint32_t* resp_buf) {
  uint8_t checksum_buffer[32];
  uint8_t* order_data_ptr = (uint8_t*)msg_ptr + 0x14;

  // Arguments for checksum_order are inferred.
  if (checksum_order(*(uint32_t*)((uint8_t*)msg_ptr + 4), (const uint32_t*)((uint8_t*)msg_ptr + 8), checksum_buffer) != 0) {
    exit(0x1c);
  }

  // Arguments for run_option_transaction are inferred.
  int transaction_result = run_option_transaction(*(uint32_t*)((uint8_t*)msg_ptr + 8), order_data_ptr, *(uint32_t*)((uint8_t*)msg_ptr + 4));

  if (transaction_result != 0) {
    uint32_t error_response_len = err_resp(transaction_result, msg_ptr, resp_buf);
    if (transmit_all(error_response_len, resp_buf) != 0) {
      exit(7);
    }
    // If transaction_result is NOT 0xee, it returns.
    // If it IS 0xee, it calls exit(0x40).
    if (transaction_result != 0xee) {
      return;
    }
    exit(0x40);
  }

  // Arguments for gen_order_fill_msg are inferred.
  int order_fill_msg_len = gen_order_fill_msg(resp_buf, 0, (uint8_t*)order_data_ptr + 8,
                                                *(uint32_t*)((uint8_t*)order_data_ptr + 4),
                                                *(uint32_t*)((uint8_t*)msg_ptr + 8));

  if (transmit_all(order_fill_msg_len + 0x18, resp_buf) != 0) {
    exit(0xf);
  }
}

// Function: run_quote
void run_quote(void* msg_ptr, uint32_t* resp_buf) {
  float* quote_value_ptr = (float*)((uint8_t*)msg_ptr + 0x14);

  // The condition `local_10 != (float *)0xfffffffc` is interpreted as a check against a magic invalid pointer value.
  // 0xfffffffc is -4.
  if (quote_value_ptr != (float*)(void*)-4) {
    long double current_ask_ld = get_current_ask();
    *quote_value_ptr = (float)current_ask_ld;

    uint32_t data_len = get_data_len(msg_ptr);
    memcpy(resp_buf, msg_ptr, data_len + 0x18);
    resp_buf[0] = 1;

    if (transmit_all(data_len + 0x18, resp_buf) == 0) {
      return;
    }
    exit(0x2c);
  }
  exit(0x2c);
}

// Function: main
int main(void) {
  // The initial stack manipulation loop (e.g., `do { ... } while (puVar3 + -0x1000 != local_8010);`)
  // is removed as it's likely platform-specific stack setup or canary initialization,
  // not standard C behavior for Linux compilation.
  // The `local_8010` array associated with it is also removed.

  init_accounts();

  uint32_t message_buffer[4096]; // Corresponds to local_4028
  uint32_t response_buffer[6];   // Corresponds to local_8028 (24 bytes)

  while (1) { // Infinite loop as per original `do { ... } while(true)`
    // Clear the first 24 bytes (0x18) of the message buffer for the header.
    memset(message_buffer, 0, 0x18);

    // Receive the message header (0x18 bytes).
    if (recv_all(0x18, message_buffer) != 0) {
      exit(1);
    }

    // Validate message header fields.
    if (message_buffer[0] != 0) {
      exit(2);
    }

    if (message_buffer[2] > 99) {
      exit(3);
    }

    // Receive the rest of the message data based on the header.
    int recv_status = recv_rest(message_buffer);
    if (recv_status != 0) {
      exit(recv_status);
    }

    uint32_t message_type = message_buffer[1];

    // Dispatch based on message type.
    if (message_type == 0xffff) {
      exit(0x14);
    } else if (message_type == 0xdd00) {
      run_quote(message_buffer, response_buffer);
    } else if (message_type == 0) {
      run_buy(message_buffer, response_buffer);
    } else if (message_type == 0xff) {
      run_sell(message_buffer, response_buffer);
    }
    // Other message types implicitly fall through and the loop continues without specific handling.
  }

  return 0; // Unreachable in an infinite loop, but good practice for main.
}