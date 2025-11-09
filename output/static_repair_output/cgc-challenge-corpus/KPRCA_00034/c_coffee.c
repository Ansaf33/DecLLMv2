#include <stdint.h> // For uint32_t
#include <stdlib.h> // For malloc, free

// Function: fE
void fE(const uint32_t* key_schedule, int idx, uint32_t *v0, uint32_t *v1) {
  uint32_t v1_old = *v1;
  uint32_t v0_new;
  
  // v0_new = ((v1_old >> 4 ^ v1_old * 8) + v1_old ^ key_schedule[idx * 2]) + *v0;
  v0_new = ((v1_old >> 4 ^ v1_old * 8) + v1_old ^ key_schedule[idx * 2]) + *v0;
  *v0 = v0_new;
  
  // *v1 = ((v0_new >> 4 ^ v0_new * 8) + v0_new ^ key_schedule[idx * 2 + 1]) + v1_old;
  *v1 = ((v0_new >> 4 ^ v0_new * 8) + v0_new ^ key_schedule[idx * 2 + 1]) + v1_old;
}

// Function: fD
void fD(const uint32_t* key_schedule, int idx, uint32_t *v0, uint32_t *v1) {
  uint32_t v1_old = *v1;
  uint32_t v0_new;
  
  // v0_new = *v0 - ((v1_old >> 4 ^ v1_old * 8) + v1_old ^ key_schedule[idx * 2 + 1]);
  v0_new = *v0 - ((v1_old >> 4 ^ v1_old * 8) + v1_old ^ key_schedule[idx * 2 + 1]);
  *v0 = v0_new;
  
  // *v1 = v1_old - ((v0_new >> 4 ^ v0_new * 8) + v0_new ^ key_schedule[idx * 2]);
  *v1 = v1_old - ((v0_new >> 4 ^ v0_new * 8) + v0_new ^ key_schedule[idx * 2]);
}

// Function: coffee_init
uint32_t coffee_init(void* ctx_ptr, const uint32_t* initial_key) {
  uint32_t* key_schedule_ptr;
  uint32_t iteration_constant = 0;
  int i;
  
  key_schedule_ptr = (uint32_t*)malloc(0x100); // Allocate 256 bytes for key schedule
  // Store the pointer to the key schedule in the context structure at offset 4
  *(uint32_t**)((char*)ctx_ptr + 4) = key_schedule_ptr;
  
  if (key_schedule_ptr == NULL) {
    return 1; // Allocation failed
  } else {
    for (i = 0; i < 0x20; i++) { // 32 rounds
      key_schedule_ptr[i * 2] = initial_key[iteration_constant & 3] + iteration_constant;
      key_schedule_ptr[i * 2 + 1] = initial_key[(iteration_constant + 0x517cc1b7 >> 0xe) & 3] + iteration_constant + 0x517cc1b7;
      iteration_constant += 0xa2f9836e;
    }
    return 0; // Success
  }
}

// Function: coffee_destroy
void coffee_destroy(void* ctx_ptr) {
  // Free the key schedule pointer stored in the context structure at offset 4
  free(*(uint32_t**)((char*)ctx_ptr + 4));
}

// Function: coffee_encode
uint32_t coffee_encode(void* ctx_ptr, uint32_t *data_block) {
  // Retrieve the key schedule pointer from the context structure at offset 4
  uint32_t* key_schedule_ptr = *(uint32_t**)((char*)ctx_ptr + 4);
  
  uint32_t p0_val = data_block[0];
  uint32_t p1_val = data_block[1];
  
  for (int i = 0; i < 0x20; i++) { // 32 rounds
    fE(key_schedule_ptr, i, &p0_val, &p1_val);
  }
  
  // Store the final values back, with a swap
  data_block[0] = p1_val;
  data_block[1] = p0_val;
  return 0;
}

// Function: coffee_decode
uint32_t coffee_decode(void* ctx_ptr, uint32_t *data_block) {
  // Retrieve the key schedule pointer from the context structure at offset 4
  uint32_t* key_schedule_ptr = *(uint32_t**)((char*)ctx_ptr + 4);
  
  uint32_t p0_val = data_block[0];
  uint32_t p1_val = data_block[1];
  
  for (int i = 0x1f; i >= 0; i--) { // 32 rounds in reverse order
    fD(key_schedule_ptr, i, &p0_val, &p1_val);
  }
  
  // Store the final values back, with a swap
  data_block[0] = p1_val;
  data_block[1] = p0_val;
  return 0;
}