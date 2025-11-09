#include <stdint.h> // Required for uint32_t, uintptr_t

// Dummy prototype for make_key_from_name
uint32_t make_key_from_name(uint32_t name_val) {
    // Placeholder implementation
    return name_val ^ 0xDEADBEEF;
}

// Dummy prototype for delete_node
void delete_node(uint32_t p1, uint32_t p2, uint32_t key, uint32_t magic_val) {
    // Placeholder implementation
    (void)p1;
    (void)p2;
    (void)key;
    (void)magic_val;
}

// Function: delete_matches
void delete_matches(uint32_t param_1, uint32_t param_2, int param_3) {
  // The value accessed from param_3 is at an offset of 4 bytes.
  // Assuming param_3 holds a memory address, it's cast to uintptr_t for arithmetic,
  // then to a pointer to uint32_t for dereferencing.
  uint32_t name_value = *(uint32_t *)((uintptr_t)param_3 + 4);

  // Intermediate variables uVar1 and uVar2 are reduced by passing values directly.
  delete_node(param_1, name_value, make_key_from_name(name_value), 0x11b36);
}

// Main function to make the code compilable
int main() {
    uint32_t p1_val = 1;
    uint32_t p2_val = 2;
    uint32_t dummy_data[] = {0xAAAA, 0xBBBB, 0xCCCC}; // 0xBBBB is at offset 4 (index 1)

    // Cast the address of dummy_data to int. This assumes int is large enough to hold the address.
    // On 64-bit systems, this cast might truncate the address, but the internal
    // logic of delete_matches uses uintptr_t for pointer arithmetic, which is robust
    // if the 'int' parameter *contains* a valid address.
    int param_3_address = (int)(uintptr_t)&dummy_data[0];

    delete_matches(p1_val, p2_val, param_3_address);

    return 0;
}