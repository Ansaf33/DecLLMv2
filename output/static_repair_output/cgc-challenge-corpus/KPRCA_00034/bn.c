#include <stdio.h>    // For fprintf, stderr
#include <stdlib.h>   // For free, calloc
#include <stdbool.h>  // For bool
#include <string.h>   // For memcpy, memset
#include <stdint.h>   // For uint32_t, uint8_t, uint16_t, uint64_t

// Big Number structure
typedef struct {
    uint32_t *data;
    uint32_t size; // Number of uint32_t words
} bn_t;

// Dummy function for rng_get_bytes - replace with actual RNG if needed
// This assumes it fills the buffer with random bytes and returns 0 on success.
// For testing, fills with non-zero values to avoid infinite loops if expected.
int rng_get_bytes(void *rng_ctx, uint8_t *buffer, size_t len) {
    (void)rng_ctx; // Unused parameter
    for (size_t i = 0; i < len; ++i) {
        buffer[i] = (uint8_t)(rand() % 255 + 1); // Avoids 0 to prevent trivial equality
    }
    return 0; // Success
}

// Forward declarations for functions
void bn_init(bn_t *bn);
void bn_destroy(bn_t *bn);
bool bn_init_bits(bn_t *bn, int bits);
int bn_length(const bn_t *bn);
bool bn_const(bn_t *bn, uint32_t val);
int bn_cmp(const bn_t *a, const bn_t *b);
void _add(uint32_t *sum_ptr, const uint32_t *operand, uint32_t num_words);
void _sub(uint32_t *minuend_ptr, const uint32_t *subtrahend_ptr, uint32_t num_words);
void _sll(bn_t *bn, uint32_t bits); // Shift Left Logical

// Function: bn_init
void bn_init(bn_t *bn) {
  bn->data = NULL;
  bn->size = 0;
}

// Function: bn_destroy
void bn_destroy(bn_t *bn) {
  free(bn->data);
  bn->data = NULL;
  bn->size = 0;
}

// Function: bn_const
bool bn_const(bn_t *bn, uint32_t val) {
  bn_destroy(bn);
  if (bn_init_bits(bn, 32)) { // Allocate for 32 bits
    return true; // Failure
  }
  bn->data[0] = val;
  return false; // Success
}

// Function: bn_init_bits
bool bn_init_bits(bn_t *bn, int bits) {
  uint32_t num_words = (bits + 31) / 32; // Calculate number of 32-bit words needed, rounding up
  if (num_words == 0) { // Always allocate at least one word for 0
      num_words = 1;
  }
  
  bn->data = (uint32_t *)calloc(num_words, sizeof(uint32_t));
  if (bn->data != NULL) {
    bn->size = num_words;
    return false; // Success
  }
  return true; // Failure (out of memory)
}

// Function: bn_length
int bn_length(const bn_t *bn) {
  uint32_t word_idx = bn->size; // 1-indexed word count
  while (word_idx > 0 && bn->data[word_idx - 1] == 0) { // Find most significant non-zero word (0-indexed)
    word_idx--;
  }

  if (word_idx == 0) {
    return 0; // Number is 0
  }

  int bits = word_idx * 32; // Total bits up to MSW
  uint32_t ms_word = bn->data[word_idx - 1]; // Most significant non-zero word

  int bit_pos = 32; // Check from MSB (bit 31) down to LSB (bit 0)
  while (bit_pos > 0 && ((1U << (bit_pos - 1)) & ms_word) == 0) {
    bits--;
    bit_pos--;
  }
  return bits;
}

// Function: bn_copy
bool bn_copy(bn_t *dest, const bn_t *src) {
  bn_destroy(dest);
  if (bn_init_bits(dest, src->size * 32)) { // Allocate same number of bits
    return true; // Failure
  }
  memcpy(dest->data, src->data, src->size * sizeof(uint32_t));
  return false; // Success
}

// Function: bn_from_bytes
bool bn_from_bytes(bn_t *bn, const uint8_t *byte_array, int num_bytes) {
  bn_destroy(bn);
  if (num_bytes == 0) {
      return bn_const(bn, 0); // Initialize to zero if no bytes
  }

  if (bn_init_bits(bn, num_bytes * 8)) {
    return true; // Allocation failed
  }

  uint32_t *word_ptr = bn->data;
  int bytes_processed = 0;

  // Process bytes from the end of the byte_array (big-endian) to fill words (little-endian)
  for (int i = 0; i < bn->size; ++i) {
      int current_byte_pos = num_bytes - bytes_processed - 1; // Index of the last byte to process for this word
      
      uint32_t word_val = 0;
      for (int j = 0; j < 4 && current_byte_pos >= 0; ++j) {
          word_val |= ((uint32_t)byte_array[current_byte_pos] << (j * 8));
          current_byte_pos--;
          bytes_processed++;
      }
      *word_ptr++ = word_val;
  }
  return false; // Success
}

// Function: bn_to_bytes
void bn_to_bytes(const bn_t *bn, uint8_t *byte_array, uint32_t num_bytes) {
  // Pad with zeros if target byte length is larger than actual big int byte length
  uint32_t bn_bytes_len = bn->size * sizeof(uint32_t);
  uint32_t zero_pad_count = 0;
  if (num_bytes > bn_bytes_len) {
    zero_pad_count = num_bytes - bn_bytes_len;
    memset(byte_array, 0, zero_pad_count);
  }
  uint8_t *current_byte_ptr = byte_array + zero_pad_count;

  // Determine how many bytes to actually copy from the big int
  uint32_t bytes_to_copy = (num_bytes < bn_bytes_len) ? num_bytes : bn_bytes_len;
  
  // Start from the most significant word and write bytes in big-endian order.
  // The big int words are stored in little-endian order (data[0] is LSW).
  for (int i = bn->size - 1; i >= 0; --i) {
    uint32_t word = bn->data[i];
    
    // Write bytes from MSB to LSB for the current word
    for (int j = 3; j >= 0; --j) {
      if (bytes_to_copy == 0) return; // Stop if target buffer is full
      *current_byte_ptr++ = (uint8_t)(word >> (j * 8));
      bytes_to_copy--;
    }
  }
}

// Function: bn_random
bool bn_random(bn_t *res, const bn_t *mod, void *rng_ctx) {
  uint32_t bit_len = bn_length(mod);
  bn_destroy(res);
  if (bn_init_bits(res, bit_len)) {
    return true; // Allocation failed
  }

  do {
    if (rng_get_bytes(rng_ctx, (uint8_t*)res->data, res->size * sizeof(uint32_t))) {
      bn_destroy(res);
      return true; // RNG failure
    }

    // Clear higher bits if total bit_len is not a multiple of 32
    if ((bit_len & 0x1f) != 0) { // If bit_len % 32 != 0
      uint32_t word_idx = (bit_len - 1) / 32; // 0-indexed word for MSB
      uint32_t bit_mask = (1U << (bit_len & 0x1f)) - 1; // Mask for relevant bits
      res->data[word_idx] &= bit_mask;
    }

    // Clear any words beyond the required bit_len (if bn_init_bits allocated more)
    for (uint32_t i = (bit_len + 31) / 32; i < res->size; ++i) {
        res->data[i] = 0;
    }
  } while (bn_cmp(res, mod) >= 0); // Loop until res < mod
  
  return false; // Success
}

// Function: bn_cmp
int bn_cmp(const bn_t *a, const bn_t *b) {
  // Find the highest non-zero word for a
  uint32_t a_len_words = a->size;
  while (a_len_words > 0 && a->data[a_len_words - 1] == 0) {
    a_len_words--;
  }

  // Find the highest non-zero word for b
  uint32_t b_len_words = b->size;
  while (b_len_words > 0 && b->data[b_len_words - 1] == 0) {
    b_len_words--;
  }

  if (a_len_words > b_len_words) {
    return 1; // a is larger
  }
  if (a_len_words < b_len_words) {
    return -1; // b is larger
  }

  // If same effective length, compare word by word from MSW down
  for (int i = a_len_words - 1; i >= 0; --i) {
    if (a->data[i] > b->data[i]) {
      return 1;
    }
    if (a->data[i] < b->data[i]) {
      return -1;
    }
  }
  return 0; // Numbers are equal
}

// Function: _add (internal helper for word-wise addition)
// Adds `operand` (param_2) to `sum_ptr` (param_1) for `num_words` (param_3) words.
// Propagates carry beyond `num_words` within `sum_ptr`'s buffer.
void _add(uint32_t *sum_ptr, const uint32_t *operand, uint32_t num_words) {
  uint32_t carry = 0;
  uint32_t i;
  
  for (i = 0; i < num_words; ++i) {
    uint64_t temp_sum = (uint64_t)sum_ptr[i] + operand[i] + carry;
    sum_ptr[i] = (uint32_t)temp_sum;
    carry = temp_sum >> 32;
  }
  
  // Propagate remaining carry.
  // This assumes `sum_ptr` points to a buffer large enough to absorb the carry.
  while (carry != 0) {
    uint64_t temp_sum = (uint64_t)sum_ptr[i] + carry;
    sum_ptr[i] = (uint32_t)temp_sum;
    carry = temp_sum >> 32;
    i++;
  }
}

// Function: _sub (internal helper for word-wise subtraction)
// Subtracts `subtrahend_ptr` (param_2) from `minuend_ptr` (param_1) for `num_words` (param_3) words.
// Propagates borrow beyond `num_words` within `minuend_ptr`'s buffer.
void _sub(uint32_t *minuend_ptr, const uint32_t *subtrahend_ptr, uint32_t num_words) {
  uint32_t borrow = 0;
  uint32_t i;

  for (i = 0; i < num_words; ++i) {
    uint64_t temp_diff = (uint64_t)minuend_ptr[i] - subtrahend_ptr[i] - borrow;
    minuend_ptr[i] = (uint32_t)temp_diff;
    borrow = (temp_diff > 0xFFFFFFFFU); // Check for underflow
  }
  
  // Propagate remaining borrow.
  // This assumes `minuend_ptr` points to a buffer large enough to absorb the borrow.
  while (borrow != 0) {
    uint64_t temp_diff = (uint64_t)minuend_ptr[i] - borrow;
    minuend_ptr[i] = (uint32_t)temp_diff;
    borrow = (temp_diff > 0xFFFFFFFFU); // Check for underflow
    i++;
  }
}

// Function: bn_mul
bool bn_mul(bn_t *res, const bn_t *a, const bn_t *b) {
  bn_destroy(res);
  
  uint32_t a_len_bits = bn_length(a);
  uint32_t b_len_bits = bn_length(b);

  if (a_len_bits == 0 || b_len_bits == 0) {
    return bn_const(res, 0); // Result is 0
  }

  // A product of N words and M words can be up to N+M words long.
  uint32_t res_words = a->size + b->size;
  if (bn_init_bits(res, res_words * 32)) { // Allocate enough for res_words
    return true; // Allocation failed
  }

  uint32_t product_words[2]; // To hold 64-bit product (low and high 32-bits)

  for (uint32_t i = 0; i < a->size; ++i) {
    if (a->data[i] == 0) continue; // Optimization: skip if factor is zero
    for (uint32_t j = 0; j < b->size; ++j) {
      if (b->data[j] == 0) continue; // Optimization: skip if factor is zero
      uint64_t product = (uint64_t)a->data[i] * b->data[j];
      product_words[0] = (uint32_t)product;       // Low 32 bits
      product_words[1] = (uint32_t)(product >> 32); // High 32 bits
      
      // Add the 64-bit product (product_words) to res->data[i+j] onwards.
      // The _add function handles carry propagation correctly.
      _add(&res->data[i+j], product_words, 2);
    }
  }
  return false; // Success
}

// Function: bn_slr (shift right)
// Shifts bn right by `bits` bits.
void bn_slr(bn_t *bn, uint32_t bits) {
  if (bn->size == 0 || bits == 0) return;

  uint32_t word_shift = bits / 32;
  uint32_t bit_shift = bits % 32;

  if (word_shift >= bn->size) {
    // Shifting more words than available, result is 0
    memset(bn->data, 0, bn->size * sizeof(uint32_t));
    return;
  }

  // Shift words
  if (word_shift > 0) {
    for (uint32_t i = 0; i < bn->size - word_shift; ++i) {
      bn->data[i] = bn->data[i + word_shift];
    }
    for (uint32_t i = bn->size - word_shift; i < bn->size; ++i) {
      bn->data[i] = 0; // Clear high words
    }
  }

  // Shift remaining bits within words
  if (bit_shift > 0) {
    for (uint32_t i = 0; i < bn->size - 1; ++i) {
      bn->data[i] = (bn->data[i] >> bit_shift) | (bn->data[i + 1] << (32 - bit_shift));
    }
    bn->data[bn->size - 1] >>= bit_shift; // MSW shift, no carry in from higher word
  }
}

// Function: _sll (shift left)
// Shifts bn left by `bits` bits.
void _sll(bn_t *bn, uint32_t bits) {
  if (bn->size == 0 || bits == 0) return;

  uint32_t word_shift = bits / 32;
  uint32_t bit_shift = bits % 32;

  // Shift words
  if (word_shift > 0) {
    for (int i = bn->size - 1; i >= (int)word_shift; --i) {
      bn->data[i] = bn->data[i - word_shift];
    }
    for (uint32_t i = 0; i < word_shift; ++i) {
      bn->data[i] = 0; // Clear low words
    }
  }

  // Shift remaining bits within words
  if (bit_shift > 0) {
    uint32_t carry = 0;
    for (uint32_t i = 0; i < bn->size; ++i) {
      uint32_t next_carry = bn->data[i] >> (32 - bit_shift);
      bn->data[i] = (bn->data[i] << bit_shift) | carry;
      carry = next_carry;
    }
    // Any remaining carry would imply the number grew beyond bn->size.
    // The caller (bn_div) is expected to have allocated sufficient space.
  }
}

// Function: bn_div
bool bn_div(bn_t *remainder, bn_t *quotient, const bn_t *num, const bn_t *den) {
  bn_destroy(remainder);
  bn_destroy(quotient);

  bn_t zero;
  bn_init(&zero);
  if (bn_const(&zero, 0)) return true; // Failed to init zero

  if (bn_cmp(den, &zero) == 0) { // Division by zero
    bn_destroy(&zero);
    return true; // Failure
  }
  
  if (bn_cmp(num, den) < 0) { // If num < den, quotient is 0, remainder is num
    bn_const(quotient, 0);
    bool copy_fail = bn_copy(remainder, num);
    bn_destroy(&zero);
    return copy_fail;
  }
  
  uint32_t num_len_bits = bn_length(num);
  uint32_t den_len_bits = bn_length(den);
  
  // Quotient can have up to (num_len_bits - den_len_bits + 1) bits
  uint32_t quot_alloc_bits = num_len_bits - den_len_bits + 1;
  // Remainder can be up to den_len_bits in size (plus one word for shifting intermediate remainder)
  uint32_t rem_alloc_bits = den_len_bits + 32;

  if (bn_init_bits(quotient, quot_alloc_bits)) { bn_destroy(&zero); return true; }
  if (bn_init_bits(remainder, rem_alloc_bits)) {
    bn_destroy(quotient); bn_destroy(&zero); return true;
  }
  
  // Initialize remainder to 0 (already done by calloc in bn_init_bits)

  // Copy numerator to a temporary, since we consume its bits.
  bn_t temp_num_copy;
  bn_init(&temp_num_copy);
  if (bn_copy(&temp_num_copy, num)) {
    bn_destroy(quotient); bn_destroy(remainder); bn_destroy(&zero); return true;
  }

  int current_bit = num_len_bits;
  while (current_bit > 0) {
    current_bit--;
    
    _sll(remainder, 1); // Shift remainder left by 1 bit
    
    // Add the current bit from temp_num_copy to remainder (LSB)
    uint32_t word_idx = current_bit / 32;
    uint32_t bit_in_word = current_bit % 32;
    if (word_idx < temp_num_copy.size && ((temp_num_copy.data[word_idx] >> bit_in_word) & 1)) {
      if (remainder->size > 0) { // Ensure remainder has at least one word
          remainder->data[0] |= 1; // Set LSB of remainder
      }
    }
    
    if (bn_cmp(remainder, den) >= 0) {
      _sub(remainder->data, den->data, den->size); // remainder -= den
      
      // Set corresponding bit in quotient
      uint32_t quot_word_idx = current_bit / 32;
      uint32_t quot_bit_in_word = current_bit % 32;
      if (quot_word_idx < quotient->size) { // Ensure within bounds
          quotient->data[quot_word_idx] |= (1U << quot_bit_in_word);
      }
    }
  }

  bn_destroy(&temp_num_copy);
  bn_destroy(&zero);
  return false; // Success
}

// Function: bn_modexp
bool bn_modexp(bn_t *res, const bn_t *base, const bn_t *exp, const bn_t *mod) {
  bn_t R0, R1, temp_mul, temp_rem, temp_quot;
  bn_init(&R0);
  bn_init(&R1);
  bn_init(&temp_mul);
  bn_init(&temp_rem);
  bn_init(&temp_quot);

  bn_destroy(res);

  bool failure = true; // Assume failure until success path is complete

  // Handle exp = 0: result is 1
  if (bn_length(exp) == 0) {
      if (!bn_const(res, 1)) {
          failure = false;
      }
      goto cleanup;
  }

  // Initialize result to 1
  if (bn_const(res, 1)) {
    goto cleanup;
  }

  // R1 = base % mod
  if (bn_div(&R1, &temp_quot, base, mod)) {
    goto cleanup;
  }
  // R0 = R1
  if (bn_copy(&R0, &R1)) {
    goto cleanup;
  }

  uint32_t exp_len_bits = bn_length(exp);

  for (uint32_t i = 0; i < exp_len_bits; ++i) {
    // Check if current bit of exponent is set
    uint32_t word_idx = i / 32;
    uint32_t bit_in_word = i % 32;
    bool exp_bit_set = (exp->data[word_idx] >> bit_in_word) & 1;

    if (exp_bit_set) {
      // res = (res * R0) % mod
      if (bn_mul(&temp_mul, res, &R0)) {
        goto cleanup;
      }
      if (bn_div(&temp_rem, &temp_quot, &temp_mul, mod)) {
        goto cleanup;
      }
      if (bn_copy(res, &temp_rem)) {
        goto cleanup;
      }
    }

    // R0 = (R0 * R0) % mod
    if (i < exp_len_bits - 1) { // Avoid final squaring if it's the last bit
        if (bn_mul(&temp_mul, &R0, &R0)) {
            goto cleanup;
        }
        if (bn_div(&temp_rem, &temp_quot, &temp_mul, mod)) {
            goto cleanup;
        }
        if (bn_copy(&R0, &temp_rem)) {
            goto cleanup;
        }
    }
  }
  failure = false; // If loop completes without `goto cleanup`

cleanup:
  bn_destroy(&R0);
  bn_destroy(&R1);
  bn_destroy(&temp_mul);
  bn_destroy(&temp_rem);
  bn_destroy(&temp_quot);
  return failure;
}


// Function: _print_bn
void _print_bn(const bn_t *bn) {
  fprintf(stderr, "\n"); // Equivalent to fdprintf(2,0x1c000);
  if (bn->size == 0 || (bn->size == 1 && bn->data[0] == 0)) {
    fprintf(stderr, "0x00000000"); // Equivalent to fdprintf(2,&DAT_0001c003);
  } else {
    // Iterate from MSW down to LSW, skipping leading zeros
    uint32_t first_non_zero_word_idx = bn->size;
    while(first_non_zero_word_idx > 0 && bn->data[first_non_zero_word_idx - 1] == 0) {
        first_non_zero_word_idx--;
    }
    if (first_non_zero_word_idx == 0) { // If all words are zero after trimming
        fprintf(stderr, "0x00000000");
    } else {
        for (int i = first_non_zero_word_idx - 1; i >= 0; --i) {
            fprintf(stderr, "%08x", bn->data[i]); // Equivalent to fdprintf(2,&DAT_0001c005,...);
        }
    }
  }
  fprintf(stderr, "\n"); // Equivalent to fdprintf(2,&DAT_0001c00a);
}

// Function: bn_self_test
int bn_self_test(void) {
  int result = 1; // Assume failure

  // Test values (converted from original decompiled data values to big-endian byte arrays)
  // a = 0x4030201050 (5 bytes)
  uint8_t a_bytes[] = {0x40, 0x30, 0x20, 0x10, 0x50};
  // b = 0x5555555555 (5 bytes)
  uint8_t b_bytes[] = {0x55, 0x55, 0x55, 0x55, 0x55};
  // expected_mul = 0x15106005EF9FFA6F90EA (10 bytes)
  uint8_t expected_mul_bytes[] = {0x15, 0x10, 0x60, 0x05, 0xEF, 0x9F, 0xFA, 0x6F, 0x90, 0xEA};

  // Division test: b / a (0x5555555555 / 0x4030201050)
  // Expected quotient: 0x05 (1 byte)
  uint8_t expected_div_quot_bytes[] = {0x05};
  // Expected remainder: 0x1364B404 (4 bytes)
  uint8_t expected_div_rem_bytes[] = {0x13, 0x64, 0xB4, 0x04};

  // Modexp test: base=2, exp=0, mod=5 => result=1
  uint8_t modexp_base_bytes[] = {0x02}; // Base = 2
  uint8_t modexp_exp_bytes[] = {0x00};   // Exponent = 0
  uint8_t modexp_mod_bytes[] = {0x05};   // Modulus = 5

  bn_t a, b, mul_res, div_quot, div_rem, modexp_res, modexp_base, modexp_exp, modexp_mod, one;
  bn_init(&a);
  bn_init(&b);
  bn_init(&mul_res);
  bn_init(&div_quot);
  bn_init(&div_rem);
  bn_init(&modexp_res);
  bn_init(&modexp_base);
  bn_init(&modexp_exp);
  bn_init(&modexp_mod);
  bn_init(&one);

  fprintf(stderr, "Starting BN self-test...\n");

  // Test bn_from_bytes and bn_mul
  if (bn_from_bytes(&a, a_bytes, sizeof(a_bytes))) { fprintf(stderr, "Error in bn_from_bytes for a\n"); goto cleanup; }
  if (bn_from_bytes(&b, b_bytes, sizeof(b_bytes))) { fprintf(stderr, "Error in bn_from_bytes for b\n"); goto cleanup; }
  
  bn_t actual_mul_res;
  bn_init(&actual_mul_res);
  if (bn_mul(&actual_mul_res, &a, &b)) { fprintf(stderr, "Error in bn_mul\n"); goto cleanup; }
  if (bn_from_bytes(&mul_res, expected_mul_bytes, sizeof(expected_mul_bytes))) { fprintf(stderr, "Error in bn_from_bytes for expected_mul\n"); goto cleanup; }

  if (bn_cmp(&actual_mul_res, &mul_res) != 0) {
    fprintf(stderr, "BN multiplication test FAILED.\n");
    fprintf(stderr, "Expected: "); _print_bn(&mul_res);
    fprintf(stderr, "Actual:   "); _print_bn(&actual_mul_res);
    goto cleanup;
  }
  bn_destroy(&actual_mul_res);
  fprintf(stderr, "BN multiplication test PASSED.\n");

  // Test bn_div (mul_res / a)
  // Should result in quotient 'b' and remainder '0'
  if (bn_div(&div_rem, &div_quot, &mul_res, &a)) { 
    fprintf(stderr, "Error in bn_div (mul_res / a)\n");
    goto cleanup;
  }
  
  bn_t zero_bn;
  bn_init(&zero_bn);
  if (bn_const(&zero_bn, 0)) { fprintf(stderr, "Error in bn_const for zero_bn\n"); goto cleanup; }

  if (bn_cmp(&div_quot, &b) != 0) { 
    fprintf(stderr, "BN division quotient test FAILED (mul_res / a).\n");
    fprintf(stderr, "Expected: "); _print_bn(&b);
    fprintf(stderr, "Actual:   "); _print_bn(&div_quot);
    goto cleanup;
  }
  if (bn_cmp(&div_rem, &zero_bn) != 0) { 
    fprintf(stderr, "BN division remainder test FAILED (mul_res / a).\n");
    fprintf(stderr, "Expected: "); _print_bn(&zero_bn);
    fprintf(stderr, "Actual:   "); _print_bn(&div_rem);
    goto cleanup;
  }
  fprintf(stderr, "BN division test (mul_res / a) PASSED.\n");
  
  // Second division test from original code: (b / a)
  if (bn_div(&div_rem, &div_quot, &b, &a)) { 
    fprintf(stderr, "Error in bn_div (b / a)\n");
    goto cleanup;
  }
  
  bn_t expected_div_quot, expected_div_rem;
  bn_init(&expected_div_quot);
  bn_init(&expected_div_rem);
  if (bn_from_bytes(&expected_div_quot, expected_div_quot_bytes, sizeof(expected_div_quot_bytes))) { fprintf(stderr, "Error in bn_from_bytes for expected_div_quot\n"); goto cleanup; }
  if (bn_from_bytes(&expected_div_rem, expected_div_rem_bytes, sizeof(expected_div_rem_bytes))) { fprintf(stderr, "Error in bn_from_bytes for expected_div_rem\n"); goto cleanup; }

  if (bn_cmp(&div_quot, &expected_div_quot) != 0) {
    fprintf(stderr, "BN division quotient test FAILED (b / a).\n");
    fprintf(stderr, "Expected: "); _print_bn(&expected_div_quot);
    fprintf(stderr, "Actual:   "); _print_bn(&div_quot);
    goto cleanup;
  }
  if (bn_cmp(&div_rem, &expected_div_rem) != 0) {
    fprintf(stderr, "BN division remainder test FAILED (b / a).\n");
    fprintf(stderr, "Expected: "); _print_bn(&expected_div_rem);
    fprintf(stderr, "Actual:   "); _print_bn(&div_rem);
    goto cleanup;
  }
  fprintf(stderr, "BN division test (b / a) PASSED.\n");
  bn_destroy(&expected_div_quot);
  bn_destroy(&expected_div_rem);
  bn_destroy(&zero_bn);

  // Test bn_modexp
  if (bn_from_bytes(&modexp_base, modexp_base_bytes, sizeof(modexp_base_bytes))) { fprintf(stderr, "Error in bn_from_bytes for modexp_base\n"); goto cleanup; }
  if (bn_from_bytes(&modexp_exp, modexp_exp_bytes, sizeof(modexp_exp_bytes))) { fprintf(stderr, "Error in bn_from_bytes for modexp_exp\n"); goto cleanup; }
  if (bn_from_bytes(&modexp_mod, modexp_mod_bytes, sizeof(modexp_mod_bytes))) { fprintf(stderr, "Error in bn_from_bytes for modexp_mod\n"); goto cleanup; }
  
  if (bn_modexp(&modexp_res, &modexp_base, &modexp_exp, &modexp_mod)) {
    fprintf(stderr, "Error in bn_modexp.\n");
    goto cleanup;
  }
  if (bn_const(&one, 1)) { fprintf(stderr, "Error in bn_const for one.\n"); goto cleanup; }

  if (bn_cmp(&modexp_res, &one) != 0) {
    fprintf(stderr, "BN modular exponentiation test FAILED.\n");
    fprintf(stderr, "Expected: "); _print_bn(&one);
    fprintf(stderr, "Actual:   "); _print_bn(&modexp_res);
    goto cleanup;
  }
  fprintf(stderr, "BN modular exponentiation test PASSED.\n");

  result = 0; // All tests passed

cleanup:
  if (result != 0) {
    fprintf(stderr, "BN self-test FAILED!\n");
  } else {
    fprintf(stderr, "BN self-test PASSED.\n");
  }

  bn_destroy(&a);
  bn_destroy(&b);
  bn_destroy(&mul_res);
  bn_destroy(&div_quot);
  bn_destroy(&div_rem);
  bn_destroy(&modexp_res);
  bn_destroy(&modexp_base);
  bn_destroy(&modexp_exp);
  bn_destroy(&modexp_mod);
  bn_destroy(&one);
  return result;
}