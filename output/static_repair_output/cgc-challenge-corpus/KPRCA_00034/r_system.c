#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

uint32_t system_init(intptr_t param_1) {
  *(uint32_t *)(param_1 + 4) = 0;
  return 0;
}

uint32_t system_get_bytes(uint32_t param_1, uint32_t param_2, int param_3) {
  long lVar1 = random();
  return ((lVar1 == 0) && (param_3 == 0)) ? 0 : 1;
}

int main() {
    uint32_t system_state_area[2];
    system_state_area[1] = 0xDEADBEEF;
    printf("Before system_init: system_state_area[1] = 0x%X\n", system_state_area[1]);

    intptr_t init_param_address = (intptr_t)&system_state_area[0];
    uint32_t init_result = system_init(init_param_address);

    printf("system_init returned: %u\n", init_result);
    printf("After system_init: system_state_area[1] (which is param_1 + 4) = 0x%X\n", system_state_area[1]);

    printf("\nTesting system_get_bytes:\n");
    srandom(time(NULL));

    printf("system_get_bytes(0, 0, 0) returned: %u\n", system_get_bytes(0, 0, 0));
    printf("system_get_bytes(0, 0, 10) returned: %u\n", system_get_bytes(0, 0, 10));
    printf("system_get_bytes(0, 0, 0) returned: %u\n", system_get_bytes(0, 0, 0));

    return 0;
}