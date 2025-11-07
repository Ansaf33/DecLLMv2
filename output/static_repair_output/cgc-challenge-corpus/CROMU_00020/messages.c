#include <stdlib.h> // For exit() and EXIT_FAILURE

// Custom _terminate function to replace decompiler artifact
void _terminate(int error_code) {
    exit(error_code);
}

// Function: extract_seed_req
int extract_seed_req(int *param_1) {
  if (*param_1 != 1) {
    _terminate(EXIT_FAILURE);
  }
  if (param_1[1] != 0x20) {
    _terminate(EXIT_FAILURE);
  }
  return param_1[2];
}

// Function: extract_rand_req
int extract_rand_req(int *param_1) {
  if (*param_1 != 10) {
    _terminate(EXIT_FAILURE);
  }
  if (param_1[1] != 1) {
    _terminate(EXIT_FAILURE);
  }
  return param_1[2];
}

// Function: extract_rand_resp
int extract_rand_resp(int *param_1) {
  if (*param_1 != 0xb) {
    _terminate(0xffffffff);
  }
  return param_1[2];
}

// Function: extract_guess_req
int extract_guess_req(int *param_1) {
  if (*param_1 != 0x14) {
    _terminate(EXIT_FAILURE);
  }
  if (param_1[1] != 8) {
    _terminate(EXIT_FAILURE);
  }
  return param_1[2];
}

// Function: extract_echo_req
int extract_echo_req(int *param_1) {
  if (*param_1 != 0x1e) {
    _terminate(0xffffffff);
  }
  return param_1[2];
}

// Function: extract_echo_resp
int extract_echo_resp(int *param_1) {
  if (*param_1 != 0x1f) {
    _terminate(0xffffffff);
  }
  return param_1[2];
}