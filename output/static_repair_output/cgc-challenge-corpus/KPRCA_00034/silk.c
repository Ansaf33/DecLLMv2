#include <stddef.h>  // For size_t, ssize_t
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memset, memcpy
#include <sys/types.h> // For ssize_t (some systems)

// --- External function declarations (stubs for compilation) ---
// These functions are assumed to be defined elsewhere in the project.
// Their actual signatures might differ slightly based on the system/library.

// I/O functions
// Assuming receive/transmit return 0 on success, 1 on failure, and
// bytes_transferred is an output parameter for the actual count.
extern int receive(int fd, void *buf, size_t nbytes, int *bytes_transferred);
extern int transmit(int fd, const void *buf, size_t nbytes, int *bytes_transferred);

// Self-test functions
extern int codes_self_test(void);
extern int bn_self_test(void);
extern int kx_self_test(void);
extern int modes_self_test(void);

// RNG function
// param1: pointer to RNG state, param2: some integer (e.g., seed length or type)
extern int rng_init(int rng_state_ptr, int param2);

// Codes functions
extern unsigned int codes_ksize(int code_id);
// param1: state_ptr, param2: code_id, param3: key (can be NULL)
extern int codes_init(int state_ptr, int code_id, const void *key);

// Modes functions
// param1: modes_state_ptr, param2: mode_id, param3: codes_state_ptr
extern int modes_init(int modes_state_ptr, int mode_id, int codes_state_ptr);
// param1: modes_state_ptr, param2: input_buf_ptr, param3: input_len,
// param4: &output_buf_ptr, param5: &output_len
extern int modes_encode(int modes_state_ptr, int input_buf_ptr, unsigned int input_len, void **output_buf_ptr, unsigned int *output_len);
// param1: modes_state_ptr, param2: encoded_buf, param3: encoded_len,
// param4: &decoded_buf_ptr, param5: &decoded_len
extern int modes_decode(int modes_state_ptr, const void *encoded_buf, unsigned short encoded_len, unsigned int *decoded_buf_ptr, unsigned short *decoded_len);

// Big Number (BN) functions
// These take a pointer to a bn_t object (represented as char[] or void* here)
extern void bn_init(void *bn_obj);
extern int bn_length(const void *bn_obj); // Returns length in bits or similar
extern void bn_destroy(void *bn_obj);
extern int bn_to_bytes(const void *bn_obj, void *buffer, unsigned short len);
extern int bn_from_bytes(void *bn_obj, const void *buffer, unsigned short len);

// Key Exchange (KX) functions
// These take a pointer to a kx_t object (represented as char[] or void* here)
extern int kx_init_std(void *kx_obj, int type);
extern int kx_gen_a(void *kx_obj, int rng_state_ptr);
extern int kx_get_A(void *kx_obj, void *bn_pub_key_obj);
extern int kx_set_b(void *kx_obj, const void *bn_pub_key_obj);
extern int kx_get_sk(void *kx_obj, unsigned char *symmetric_key, int ksize);
extern void kx_destroy(void *kx_obj);

// --- Structure for silk_state_t ---
// Based on offsets used in the original code.
// This is a crucial assumption for correct pointer arithmetic.
typedef struct silk_state {
    // Contexts for sending and receiving operations
    char codes_send_ctx[8];   // Offset 0x00
    char codes_recv_ctx[8];   // Offset 0x08
    char modes_send_ctx[24];  // Offset 0x10
    char modes_recv_ctx[24];  // Offset 0x28

    char rng_state[16];       // Offset 0x40
    int negotiated_code;      // Offset 0x48
    int negotiated_mode;      // Offset 0x4C
    unsigned int kx_ksize;    // Offset 0x50
    void *recv_buf;           // Offset 0x54 (malloc'd buffer for incoming data)
    unsigned short recv_len;  // Offset 0x58 (length of data in recv_buf)
    unsigned short recv_off;  // Offset 0x5A (current offset in recv_buf)
} silk_state_t; // Total size 0x5C = 92 bytes


int _recv_bytes(void *buf, int num_bytes) {
  int bytes_transferred;
  return (receive(0, buf, num_bytes, &bytes_transferred) == 0 && num_bytes == bytes_transferred) ? 0 : 1;
}

ssize_t _recv(int fd, void *buf, size_t n, int flags) {
  char received_fd_byte;
  if (_recv_bytes(&received_fd_byte, 1) != 0) {
    return 1;
  }
  if ((char)fd != received_fd_byte) {
    return 1;
  }
  if (n == 0) {
    return 0;
  }
  return _recv_bytes(buf, n);
}

int _send_bytes(void *buf, int num_bytes) {
  int bytes_transferred;
  return (transmit(1, buf, num_bytes, &bytes_transferred) == 0 && num_bytes == bytes_transferred) ? 0 : 1;
}

ssize_t _send(int fd, void *buf, size_t n, int flags) {
  char fd_byte = (char)fd;
  if (_send_bytes(&fd_byte, 1) != 0) {
    return 1;
  }
  if (n == 0) {
    return 0;
  }
  return _send_bytes(buf, n);
}

int silk_init(void **state_ptr) {
  silk_state_t *state = (silk_state_t *)malloc(sizeof(silk_state_t));
  *state_ptr = state;

  if (state == NULL) {
    return 1;
  }

  if (codes_self_test() != 0 || bn_self_test() != 0 || kx_self_test() != 0 || modes_self_test() != 0) {
    free(state);
    *state_ptr = NULL;
    return 1;
  }

  if (rng_init((int)&state->rng_state, 1) != 0) {
    free(state);
    *state_ptr = NULL;
    return 1;
  }

  state->recv_buf = NULL;
  state->recv_len = 0;
  state->recv_off = 0;

  return 0;
}

unsigned int silk_allowed_codes(void) {
  return 0xf001f;
}

int silk_preferred_code(unsigned int remote_codes) {
  unsigned int common_codes = silk_allowed_codes() & remote_codes;
  if ((common_codes & 0x10) != 0) return 4;
  if ((common_codes & 8) != 0) return 3;
  if ((common_codes & 4) != 0) return 2;
  if ((common_codes & 2) != 0) return 1;
  if ((common_codes & 1) != 0) return 0;
  return -1;
}

int silk_preferred_mode(unsigned int remote_modes) {
  unsigned int common_modes = silk_allowed_codes() & remote_modes;
  if ((common_modes & 0x80000) != 0) return 3;
  if ((common_modes & 0x40000) != 0) return 2;
  if ((common_modes & 0x20000) != 0) return 1;
  if ((common_modes & 0x10000) != 0) return 0;
  return -1;
}

int silk_negotiate(silk_state_t *state) {
  // Assuming flags for _send/_recv are a constant, or 0 if not specified.
  // The original code passed `*param_1` (which is `state` in our interpretation)
  // as the flags argument. This is inconsistent with other functions.
  // Using 0 as a placeholder for flags.
  const int io_flags = 0;

  unsigned int remote_codes;
  unsigned int allowed_codes = silk_allowed_codes();

  if (_send(1, &allowed_codes, sizeof(unsigned int), io_flags) != 0) {
    return 1;
  }

  if (_recv(1, &remote_codes, sizeof(unsigned int), io_flags) != 0) {
    return 1;
  }

  int preferred_code = silk_preferred_code(remote_codes);
  if (preferred_code == -1) {
    return 1;
  }

  int preferred_mode = silk_preferred_mode(remote_codes);
  if (preferred_mode == -1) {
    return 1;
  }

  unsigned int negotiated_bits = (1U << ((unsigned char)preferred_mode + 0x10U & 0x1f)) | (1U << ((unsigned char)preferred_code & 0x1f));
  if (negotiated_bits != remote_codes) {
    if (_send(1, &negotiated_bits, sizeof(unsigned int), io_flags) != 0) {
      return 1;
    }
  }

  state->negotiated_code = preferred_code;
  state->kx_ksize = codes_ksize(state->negotiated_code);
  state->negotiated_mode = preferred_mode;

  return 0;
}

int silk_kx(silk_state_t *state) {
  const int io_flags = 0x199af;
  int result = 1; // Assume failure

  char kx_state[8];
  char bn_P[32]; // Based on auStack_78
  char bn_B[8];  // Based on auStack_80
  char local_pub_key[8];
  char remote_pub_key[8];
  unsigned char symmetric_key[32];

  void *temp_buffer = NULL;

  int kx_state_initialized = 0;
  int local_pub_key_initialized = 0;
  int remote_pub_key_initialized = 0;

  if (state->kx_ksize == 0) {
    // Case 1: No key exchange needed (kx_ksize is 0)
    unsigned short zero_bytes[4] = {0, 0, 0, 0}; // 8 bytes
    if (_send(2, zero_bytes, sizeof(zero_bytes), io_flags) == 0) {
      unsigned short remote_key_len;
      if (_recv(3, &remote_key_len, sizeof(remote_key_len), io_flags) == 0) {
        if (remote_key_len == 0) {
          if (codes_init((int)&state->codes_send_ctx, state->negotiated_code, NULL) == 0) {
            if (modes_init((int)&state->modes_send_ctx, state->negotiated_mode, (int)&state->codes_send_ctx) == 0) {
              if (codes_init((int)&state->codes_recv_ctx, state->negotiated_code, NULL) == 0) {
                if (modes_init((int)&state->modes_recv_ctx, state->negotiated_mode, (int)&state->codes_recv_ctx) == 0) {
                  result = 0; // Success
                }
              }
            }
          }
        }
      }
    }
  } else {
    // Case 2: Perform key exchange
    int kx_type;
    if (state->kx_ksize < 0x81) { // 129
      kx_type = 2; // small key
    } else {
      kx_type = 3; // large key
    }

    bn_init(local_pub_key);
    local_pub_key_initialized = 1;
    bn_init(remote_pub_key);
    remote_pub_key_initialized = 1;

    if (kx_init_std(kx_state, kx_type) == 0) {
      kx_state_initialized = 1;
      if (kx_gen_a(kx_state, (int)&state->rng_state) == 0) {
        if (kx_get_A(kx_state, local_pub_key) == 0) {
          unsigned short len1 = (unsigned short)((bn_length(kx_state) + 7U) >> 3);
          unsigned short len2 = (unsigned short)((bn_length(bn_P) + 7U) >> 3);
          unsigned short len3 = (unsigned short)((bn_length(bn_B) + 7U) >> 3);
          unsigned short len4 = (unsigned short)((bn_length(local_pub_key) + 7U) >> 3);

          size_t total_len = (size_t)len1 + len2 + len3 + len4;
          temp_buffer = malloc(total_len);

          if (temp_buffer != NULL) {
            int current_offset = 0;
            bn_to_bytes(kx_state, (char *)temp_buffer + current_offset, len1);
            current_offset += len1;
            bn_to_bytes(bn_P, (char *)temp_buffer + current_offset, len2);
            current_offset += len2;
            bn_to_bytes(bn_B, (char *)temp_buffer + current_offset, len3);
            current_offset += len3;
            bn_to_bytes(local_pub_key, (char *)temp_buffer + current_offset, len4);
            current_offset += len4;

            unsigned short lengths[4] = {len1, len2, len3, len4};
            if (_send(2, lengths, sizeof(lengths), io_flags) == 0) {
              if (_send_bytes(temp_buffer, current_offset) == 0) {
                free(temp_buffer);
                temp_buffer = NULL;

                unsigned short remote_key_len;
                if (_recv(3, &remote_key_len, sizeof(remote_key_len), io_flags) == 0) {
                  temp_buffer = malloc(remote_key_len);
                  if (temp_buffer != NULL) {
                    if (_recv_bytes(temp_buffer, remote_key_len) == 0) {
                      if (bn_from_bytes(remote_pub_key, temp_buffer, remote_key_len) == 0) {
                        if (kx_set_b(kx_state, remote_pub_key) == 0) {
                          if (kx_get_sk(kx_state, symmetric_key, state->kx_ksize) == 0) {
                            if (codes_init((int)&state->codes_send_ctx, state->negotiated_code, symmetric_key) == 0) {
                              if (modes_init((int)&state->modes_send_ctx, state->negotiated_mode, (int)&state->codes_send_ctx) == 0) {
                                unsigned char xor_byte = 0xff;
                                for (unsigned int i = 0; i < state->kx_ksize >> 3; ++i) {
                                  symmetric_key[i] = symmetric_key[i] ^ xor_byte;
                                  xor_byte = xor_byte ^ (symmetric_key[i] >> 3);
                                }
                                if (codes_init((int)&state->codes_recv_ctx, state->negotiated_code, symmetric_key) == 0) {
                                  if (modes_init((int)&state->modes_recv_ctx, state->negotiated_mode, (int)&state->codes_recv_ctx) == 0) {
                                    result = 0; // Success
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  if (temp_buffer != NULL) free(temp_buffer);
  if (local_pub_key_initialized) bn_destroy(local_pub_key);
  if (remote_pub_key_initialized) bn_destroy(remote_pub_key);
  if (kx_state_initialized) kx_destroy(kx_state);

  return result;
}

int silk_prepare(silk_state_t *state) {
  if (silk_negotiate(state) == 0) {
    if (silk_kx(state) == 0) {
      return 0;
    }
  }
  return 1;
}

int silk_send(silk_state_t *state, const void *buf, size_t nbytes) {
  const int io_flags = 0x19ff1;
  const unsigned int max_chunk_size = 0xffef;

  while (nbytes > 0) {
    unsigned int chunk_size = (nbytes <= max_chunk_size) ? (unsigned int)nbytes : max_chunk_size;
    void *encoded_data = NULL;
    unsigned int actual_encoded_len = 0;

    if (modes_encode((int)&state->modes_send_ctx, (int)buf, chunk_size, &encoded_data, &actual_encoded_len) != 0) {
      return 1;
    }

    if (actual_encoded_len > 0xffff) {
      free(encoded_data);
      return 1;
    }

    unsigned short encoded_len_short = (unsigned short)actual_encoded_len;

    if (_send(4, &encoded_len_short, sizeof(encoded_len_short), io_flags) != 0) {
      free(encoded_data);
      return 1;
    }
    if (_send_bytes(encoded_data, actual_encoded_len) != 0) {
      free(encoded_data);
      return 1;
    }

    free(encoded_data);

    buf = (const char *)buf + chunk_size;
    nbytes -= chunk_size;
  }
  return 0;
}

int _fill_recvbuf(silk_state_t *state) {
  const int io_flags = 0x1a0bd;
  int result = 1; // Assume failure
  void *encoded_data_buf = NULL;

  unsigned short encoded_len;
  if (_recv(4, &encoded_len, sizeof(encoded_len), io_flags) == 0) {
    encoded_data_buf = malloc(encoded_len);
    if (encoded_data_buf != NULL) {
      if (_recv_bytes(encoded_data_buf, encoded_len) == 0) {
        void *decoded_data_ptr = NULL;
        unsigned short decoded_len_arr[1]; // Only [0] is used

        if (modes_decode((int)&state->modes_recv_ctx, encoded_data_buf, encoded_len, (unsigned int*)&decoded_data_ptr, &decoded_len_arr[0]) == 0) {
          state->recv_buf = decoded_data_ptr;
          state->recv_len = decoded_len_arr[0];
          state->recv_off = 0;
          result = 0; // Success
        }
      }
    }
  }
  free(encoded_data_buf);
  return result;
}

int silk_recv(silk_state_t *state, void *buf, size_t nbytes) {
  while (nbytes > 0) {
    if (state->recv_buf == NULL || state->recv_len == 0) { // Check both buf and length
      if (_fill_recvbuf(state) != 0) {
        return 1; // Error filling buffer
      }
      // If _fill_recvbuf succeeds but provides no data, it's an issue.
      if (state->recv_buf == NULL || state->recv_len == 0) {
          return 1; // Failed to get data, or got empty data when more was expected
      }
    }

    size_t bytes_available = state->recv_len - state->recv_off;
    size_t bytes_to_copy = (nbytes <= bytes_available) ? nbytes : bytes_available;

    if (bytes_to_copy == 0) {
        // This should not happen if recv_buf is not NULL and recv_len > recv_off
        // but as a safeguard, if we can't copy anything and still need bytes, it's an error.
        if (nbytes > 0) return 1;
    }

    memcpy(buf, (char *)state->recv_buf + state->recv_off, bytes_to_copy);

    buf = (char *)buf + bytes_to_copy;
    nbytes -= bytes_to_copy;
    state->recv_off += bytes_to_copy;
    state->recv_len -= bytes_to_copy; // This will become 0 if all available bytes are copied.

    if (state->recv_len == 0) {
      free(state->recv_buf);
      state->recv_buf = NULL;
      state->recv_off = 0;
    }
  }
  return 0;
}