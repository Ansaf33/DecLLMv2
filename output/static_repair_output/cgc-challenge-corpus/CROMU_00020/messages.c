#include <stdlib.h> // For abort()

// Function: extract_seed_req
int extract_seed_req(int *param_1) {
  if (*param_1 != 1) {
    abort();
  }
  if (param_1[1] != 0x20) {
    abort();
  }
  return param_1[2];
}

// Function: extract_rand_req
int extract_rand_req(int *param_1) {
  if (*param_1 != 10) {
    abort();
  }
  if (param_1[1] != 1) {
    abort();
  }
  return param_1[2];
}

// Function: extract_rand_resp
int extract_rand_resp(int *param_1) {
  if (*param_1 != 0xb) {
    abort();
  }
  return param_1[2];
}

// Function: extract_guess_req
int extract_guess_req(int *param_1) {
  if (*param_1 != 0x14) {
    abort();
  }
  if (param_1[1] != 8) {
    abort();
  }
  return param_1[2];
}

// Function: extract_echo_req
int extract_echo_req(int *param_1) {
  if (*param_1 != 0x1e) {
    abort();
  }
  return param_1[2];
}

// Function: extract_echo_resp
int extract_echo_resp(int *param_1) {
  if (*param_1 != 0x1f) {
    abort();
  }
  return param_1[2];
}