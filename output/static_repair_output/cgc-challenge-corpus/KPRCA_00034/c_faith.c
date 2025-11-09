#include <stdlib.h> // For malloc, free, NULL
#include <stdint.h> // For uint8_t, uint16_t, uint32_t

// Type aliases for clarity, matching original intent
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;

// Macro for CONCAT22, which combines two 16-bit values into a 32-bit value.
// (high << 16) | low
#define CONCAT22(high, low) (((uint)(high) << 16) | (low))

// Function: S
uint S(byte param_1, byte param_2, byte param_3) {
  byte bVar1 = param_3 + param_1 + param_2;
  return ((uint)bVar1 >> 6) | ((uint)bVar1 * 4);
}

// Function: f
// param_1 is a pointer to the key schedule (array of ushorts)
void f(const ushort *key_schedule_ptr, int index, uint *param_3, uint *param_4) {
  uint uVar_P_curr = *param_3;
  uint uVar_Q_curr = *param_4;

  byte b5 = uVar_Q_curr >> 24;
  ushort key_val = key_schedule_ptr[index]; // Accessing key schedule based on index

  byte b4 = (uVar_Q_curr >> 8) ^ (key_val & 0xFF) ^ (uVar_Q_curr & 0xFF);
  byte b3 = S((uVar_Q_curr >> 16) ^ (key_val >> 8) ^ b5, b4, 0);
  
  b4 = S(b4, b3, 1);
  b5 = S(b5, b3, 1);
  byte b6 = S(uVar_Q_curr & 0xFF, b4, 0);
  
  *param_3 = uVar_Q_curr;
  *param_4 = ((uint)b6 | ((uint)b5 << 24) | ((uint)b3 << 16) | ((uint)b4 << 8)) ^ uVar_P_curr;
}

// Function: fK
uint fK(uint param_1, uint param_2) {
  byte b3_val = param_1 >> 24;
  byte b1_val = (param_1 >> 8) ^ (param_1 & 0xFF);
  
  byte b2_ret = S((param_1 >> 16) ^ b3_val, b1_val ^ (param_2 >> 24), 1);
  b1_val = S(b1_val, b2_ret ^ (param_2 >> 16), 0);
  b3_val = S(b3_val, b2_ret ^ (param_2 >> 8), 0);
  byte b4_ret = S(param_1 & 0xFF, b1_val ^ (param_2 & 0xFF), 1);
  
  return ((uint)b4_ret | ((uint)b3_val << 24) | ((uint)b2_ret << 16) | ((uint)b1_val << 8));
}

// Function: faith_init
// param_1_struct_ptr is a pointer to a structure or array that stores the key schedule pointer at offset +4.
// param_2_keys is a pointer to an array of 4 uints used for initialization.
uint faith_init(void *param_1_struct_ptr, const uint *param_2_keys) {
  ushort *key_schedule_ptr = (ushort*)malloc(0x50); // Allocate 80 bytes for 40 ushorts
  // Store the allocated pointer in the second element (offset 4) of the structure pointed to by param_1_struct_ptr.
  ((void**)param_1_struct_ptr)[1] = key_schedule_ptr; 

  if (key_schedule_ptr == NULL) {
    return 1; // Indicate failure
  }
  
  uint k0 = param_2_keys[0]; 
  uint k1 = param_2_keys[1]; 
  uint k2 = param_2_keys[2]; 
  uint k3 = param_2_keys[3]; 
  uint k_prev = 0; 

  for (int i = 0; i < 20; ++i) { // 0x14 = 20 iterations
    uint tmp_key_part; 
    if (i % 3 == 0) {
      tmp_key_part = k2 ^ k3;
    } else if (i % 3 == 1) {
      tmp_key_part = k2;
    } else { // i % 3 == 2
      tmp_key_part = k3;
    }
    
    uint new_key_val = fK(k0, k1 ^ k_prev ^ tmp_key_part);
    
    key_schedule_ptr[i * 2] = new_key_val >> 16;
    key_schedule_ptr[i * 2 + 1] = new_key_val & 0xFFFF; // Store lower 16 bits
    
    k_prev = k0;
    k0 = k1;
    k1 = new_key_val;
  }
  
  return 0; // Indicate success
}

// Function: faith_destroy
// param_1_struct_ptr is a pointer to a structure or array that stores the key schedule pointer at offset +4.
void faith_destroy(void *param_1_struct_ptr) {
  free(((void**)param_1_struct_ptr)[1]);
}

// Function: faith_encode
// param_1_struct_ptr is a pointer to a structure or array that stores the key schedule pointer at offset +4.
// param_2_data is a pointer to two uints (input/output plaintext/ciphertext block).
uint faith_encode(void *param_1_struct_ptr, uint *param_2_data) {
  const ushort *key_schedule_ptr = (const ushort*)((void**)param_1_struct_ptr)[1];
  
  // Initial XOR with key schedule words
  uint data_P = param_2_data[0] ^ CONCAT22(key_schedule_ptr[32], key_schedule_ptr[33]); // Words at 0x40, 0x42
  uint data_Q = param_2_data[1] ^ CONCAT22(key_schedule_ptr[34], key_schedule_ptr[35]); // Words at 0x44, 0x46
  
  for (int i = 0; i < 32; ++i) { // 0 to 0x1f (31) rounds
    f(key_schedule_ptr, i, &data_P, &data_Q);
  }
  
  // Final XOR with key schedule words
  param_2_data[0] = data_Q ^ CONCAT22(key_schedule_ptr[36], key_schedule_ptr[37]); // Words at 0x48, 0x4a
  param_2_data[1] = data_P ^ CONCAT22(key_schedule_ptr[38], key_schedule_ptr[39]); // Words at 0x4c, 0x4e
  
  return 0;
}

// Function: faith_decode
// param_1_struct_ptr is a pointer to a structure or array that stores the key schedule pointer at offset +4.
// param_2_data is a pointer to two uints (input/output ciphertext/plaintext block).
uint faith_decode(void *param_1_struct_ptr, uint *param_2_data) {
  const ushort *key_schedule_ptr = (const ushort*)((void**)param_1_struct_ptr)[1];
  
  // Initial XOR with key schedule words (reversed from encode)
  uint data_Q = param_2_data[0] ^ CONCAT22(key_schedule_ptr[36], key_schedule_ptr[37]); // Words at 0x48, 0x4a
  uint data_P = param_2_data[1] ^ CONCAT22(key_schedule_ptr[38], key_schedule_ptr[39]); // Words at 0x4c, 0x4e
  
  for (int i = 31; i >= 0; --i) { // 0x1f (31) down to 0 rounds
    f(key_schedule_ptr, i, &data_Q, &data_P); // Note the swap of P and Q for decode
  }
  
  // Final XOR with key schedule words (reversed from encode)
  param_2_data[0] = data_P ^ CONCAT22(key_schedule_ptr[32], key_schedule_ptr[33]); // Words at 0x40, 0x42
  param_2_data[1] = data_Q ^ CONCAT22(key_schedule_ptr[34], key_schedule_ptr[35]); // Words at 0x44, 0x46
  
  return 0;
}