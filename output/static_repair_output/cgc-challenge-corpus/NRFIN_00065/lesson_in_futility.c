#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

typedef unsigned int uint;
typedef void (*code)(void);

void defcon_1(void);
void defcon_2(void);
int do_lesson_in_futility(void);

int game_state_val = 0;

int custom_strtou(const char *str, int base, uint *res) {
    char *endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, base);

    if (str == endptr) {
        return -1;
    }
    if (errno == ERANGE || (val == ULONG_MAX && errno != 0)) {
        return -1;
    }
    while (*endptr != '\0') {
        if (!isspace((unsigned char)*endptr)) {
            return -1;
        }
        endptr++;
    }

    *res = (uint)val;
    return 0;
}

void defcon_1(void) {
  printf("A strange game. The only winning move is not to play.\n");
}

void defcon_2(void) {
  char input_buffer[40];
  uint latitude_val;
  uint longitude_val;
  int conversion_result;

  printf("Enter target latitude:\n");
  if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
      printf("Error reading input.\n");
      return;
  }
  input_buffer[strcspn(input_buffer, "\n")] = 0;

  if (strlen(input_buffer) == 0) {
      printf("Input cannot be empty.\n");
      return;
  }

  conversion_result = custom_strtou(input_buffer, 16, &latitude_val);
  if (conversion_result != -1) {
    printf("Enter target longitude:\n");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    input_buffer[strcspn(input_buffer, "\n")] = 0;

    if (strlen(input_buffer) == 0) {
        printf("Input cannot be empty.\n");
        return;
    }

    conversion_result = custom_strtou(input_buffer, 16, &longitude_val);
    if (conversion_result != -1) {
      uint target_address = latitude_val | 0xf0000000;
      printf("Attempting to call code at address: 0x%x (latitude: 0x%x, longitude: 0x%x)\n",
             target_address, latitude_val, longitude_val);
    } else {
        printf("Invalid longitude input.\n");
    }
  } else {
      printf("Invalid latitude input.\n");
  }
}

int do_lesson_in_futility(void) {
  if (game_state_val == 0) {
    game_state_val = 1;
  }
  if (game_state_val == 1) {
    defcon_1();
  }
  else {
    if (game_state_val != 2) {
      return -1;
    }
    defcon_2();
  }
  return 0;
}

int main() {
    printf("--- Starting Defcon Simulation ---\n");
    do_lesson_in_futility();
    printf("--- Defcon Simulation Ended ---\n");

    game_state_val = 2;
    printf("\n--- Starting Defcon Simulation (second run) ---\n");
    do_lesson_in_futility();
    printf("--- Defcon Simulation Ended ---\n");

    game_state_val = 3;
    printf("\n--- Starting Defcon Simulation (error path) ---\n");
    int result = do_lesson_in_futility();
    printf("Error path returned: %d\n", result);
    printf("--- Defcon Simulation Ended ---\n");

    return 0;
}