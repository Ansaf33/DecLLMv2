#include <stdint.h>
#include <stdbool.h>

uint8_t sort_n_sum(uint8_t *param_1, uint32_t param_2) {
  if (param_2 == 0) {
    return 0;
  }

  bool bVar2 = true;
  uint8_t local_d;
  uint32_t local_c;

  while (bVar2) {
    bVar2 = false;
    local_d = param_1[0];
    for (local_c = 1; local_c < param_2; local_c++) {
      if ((char)param_1[local_c] < (char)param_1[local_c - 1]) {
        param_1[local_c - 1] = param_1[local_c - 1] ^ param_1[local_c];
        param_1[local_c] = param_1[local_c - 1] ^ param_1[local_c];
        param_1[local_c - 1] = param_1[local_c - 1] ^ param_1[local_c];
        bVar2 = true;
      }
      local_d ^= param_1[local_c];
    }
  }
  return local_d;
}