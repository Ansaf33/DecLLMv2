#include <stdint.h>
#include <stdio.h>

int _ls_mode;

void single_binary_main_ls(uint32_t param_1, uint64_t param_2);

void single_binary_main_vdir(uint32_t param_1, uint64_t param_2) {
  _ls_mode = 3;
  single_binary_main_ls(param_1, param_2);
  return;
}

void single_binary_main_ls(uint32_t param_1, uint64_t param_2) {
  printf("single_binary_main_ls called with param_1: %u, param_2: %llu, _ls_mode: %d\n", param_1, param_2, _ls_mode);
}

int main() {
    printf("Calling single_binary_main_vdir...\n");
    single_binary_main_vdir(10, 20000000000ULL);
    printf("Exiting main.\n");
    return 0;
}