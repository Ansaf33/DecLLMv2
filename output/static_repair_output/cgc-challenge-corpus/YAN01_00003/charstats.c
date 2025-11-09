#include <stdint.h>   // For uint8_t, uint16_t, uint32_t
#include <stdlib.h>   // For malloc, free, exit
#include <stdio.h>    // For perror
#include <string.h>   // For memset
#include <unistd.h>   // For read, write, STDIN_FILENO, STDOUT_FILENO
#include <sys/select.h> // For fd_set, struct timeval, select
#include <errno.h>    // For errno

// Standard C type replacements (using uint*_t for clarity and standard compliance)
typedef uint8_t  _uint8_t;
typedef uint16_t _uint16_t;
typedef uint32_t _uint32_t;
typedef int8_t   _int8_t;
typedef int16_t  _int16_t;
typedef int32_t  _int32_t;

// Global state memory
uint8_t *state = NULL;
uint8_t *_consts = NULL;

// Helper macros for accessing memory/registers
// These assume 'addr' is an offset (uint16_t) into the 'state' memory block.
#define GET_REG8(addr)       (*(volatile _uint8_t *)(state + (addr)))
#define GET_REG16(addr)      (*(volatile _uint16_t *)(state + (addr)))
#define GET_REG32(addr)      (*(volatile _uint32_t *)(state + (addr)))

#define SET_REG8(addr, val)  (*(volatile _uint8_t *)(state + (addr)) = (_uint8_t)(val))
#define SET_REG16(addr, val) (*(volatile _uint16_t *)(state + (addr)) = (_uint16_t)(val))
#define SET_REG32(addr, val) (*(volatile _uint32_t *)(state + (addr)) = (_uint32_t)(val))

// Constants addresses (based on INIT_MACHINE initialization)
#define ADDR_ZERO_32BIT     0x100 // Value 0
#define ADDR_ONE_32BIT      0x104 // Value 1
#define ADDR_TRUE_8BIT      0x108 // Value 0xFF
#define ADDR_TRUE_16BIT     0x10C // Value 0xFFFF
#define ADDR_TRUE_32BIT     0x110 // Value 0xFFFFFFFF
#define ADDR_FALSE_32BIT    0x114 // Value 0

// Emulated system calls
void _terminate(int status) {
    if (state) {
        free(state);
        state = NULL;
    }
    exit(status);
}

int allocate() {
    // Arbitrary size for the state memory, chosen to accommodate all used offsets
    const size_t STATE_MEMORY_SIZE = 4096; // 4KB, max offset seen is around 0xee8 + 0x114 = 0xFFC
    state = (uint8_t*)malloc(STATE_MEMORY_SIZE);
    if (state == NULL) {
        perror("Failed to allocate state memory");
        return -1; // Indicate failure
    }
    memset(state, 0, STATE_MEMORY_SIZE); // Initialize to zero
    return 0; // Indicate success
}

int transmit(int fd, const void *buf, size_t count, int *bytes_transmitted) {
    ssize_t written = write(fd, buf, count);
    if (written < 0) {
        *bytes_transmitted = 0;
        return errno;
    }
    *bytes_transmitted = (int)written;
    return 0;
}

int receive(int fd, void *buf, size_t count, int *bytes_received) {
    ssize_t read_bytes = read(fd, buf, count);
    if (read_bytes < 0) {
        *bytes_received = 0;
        return errno;
    }
    *bytes_received = (int)read_bytes;
    return 0;
}

// Emulated fdwait function matching the original snippet's call signature
// param_4_tv_sec: pointer to timeout seconds
// param_5_ready_count: pointer to store the number of ready file descriptors
int fdwait(int nfds, fd_set *readfds, fd_set *writefds, _uint32_t *param_4_tv_sec, _uint32_t *param_5_ready_count) {
    struct timeval timeout;
    timeout.tv_sec = param_4_tv_sec ? *param_4_tv_sec : 0;
    timeout.tv_usec = 0; // Original snippet implies tv_usec is always 0 or not used explicitly

    int ret = select(nfds, readfds, writefds, NULL, &timeout);
    if (ret < 0) {
        if (param_5_ready_count) *param_5_ready_count = 0;
        return errno;
    }
    if (param_5_ready_count) *param_5_ready_count = ret;
    return 0;
}

// Function: DATA8
_uint8_t DATA8(_uint16_t param_1) {
  return GET_REG8(param_1 + 0x80);
}

// Function: DATA16
_uint16_t DATA16(_uint16_t param_1) {
  return GET_REG16(param_1 + 0x80);
}

// Function: DATA32
_uint32_t DATA32(_uint16_t param_1) {
  return GET_REG32(param_1 + 0x80);
}

// Function: REG8
_uint8_t REG8(_uint16_t param_1) {
  return GET_REG8(param_1);
}

// Function: REG16
_uint16_t REG16(_uint16_t param_1) {
  return GET_REG16(param_1);
}

// Function: REG32
_uint32_t REG32(_uint16_t param_1) {
  return GET_REG32(param_1);
}

// Function: MOVIM8
void MOVIM8(_uint16_t param_1, _uint8_t param_2) {
  SET_REG8(param_1, param_2);
}

// Function: MOVIM16
void MOVIM16(_uint16_t param_1, _uint16_t param_2) {
  SET_REG16(param_1, param_2);
}

// Function: MOVIM32
void MOVIM32(_uint16_t param_1, _uint32_t param_2) {
  SET_REG32(param_1, param_2);
}

// Function: MOV8
void MOV8(_uint16_t param_1, _uint16_t param_2) {
  SET_REG8(param_1, GET_REG8(param_2));
}

// Function: MOV16
void MOV16(_uint16_t param_1, _uint16_t param_2) {
  SET_REG16(param_1, GET_REG16(param_2));
}

// Function: MOV32
void MOV32(_uint16_t param_1, _uint16_t param_2) {
  SET_REG32(param_1, GET_REG32(param_2));
}

// Function: AND8
void AND8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG8(param_1, GET_REG8(param_2) & GET_REG8(param_3));
}

// Function: AND16
void AND16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG16(param_1, GET_REG16(param_2) & GET_REG16(param_3));
}

// Function: AND32
void AND32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG32(param_1, GET_REG32(param_3) & GET_REG32(param_2));
}

// Function: OR8
void OR8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG8(param_1, GET_REG8(param_2) | GET_REG8(param_3));
}

// Function: OR16
void OR16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG16(param_1, GET_REG16(param_2) | GET_REG16(param_3));
}

// Function: OR32
void OR32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG32(param_1, GET_REG32(param_3) | GET_REG32(param_2));
}

// Function: NOT8
void NOT8(_uint16_t param_1, _uint16_t param_2) {
  SET_REG8(param_1, ~GET_REG8(param_2));
}

// Function: NOT16
void NOT16(_uint16_t param_1, _uint16_t param_2) {
  SET_REG16(param_1, ~GET_REG16(param_2));
}

// Function: NOT32
void NOT32(_uint16_t param_1, _uint16_t param_2) {
  SET_REG32(param_1, ~GET_REG32(param_2));
}

// Function: ADD8 (signed char addition based on original snippet's type cast)
void ADD8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG8(param_1, (_uint8_t)((_int8_t)GET_REG8(param_2) + (_int8_t)GET_REG8(param_3)));
}

// Function: ADD16 (signed short addition based on original snippet's type cast)
void ADD16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG16(param_1, (_uint16_t)((_int16_t)GET_REG16(param_2) + (_int16_t)GET_REG16(param_3)));
}

// Function: ADD32 (signed int addition based on original snippet's type cast)
void ADD32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG32(param_1, (_uint32_t)((_int32_t)GET_REG32(param_3) + (_int32_t)GET_REG32(param_2)));
}

// Function: SUB8 (signed char subtraction based on original snippet's type cast)
void SUB8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG8(param_1, (_uint8_t)((_int8_t)GET_REG8(param_2) - (_int8_t)GET_REG8(param_3)));
}

// Function: SUB16 (signed short subtraction based on original snippet's type cast)
void SUB16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG16(param_1, (_uint16_t)((_int16_t)GET_REG16(param_2) - (_int16_t)GET_REG16(param_3)));
}

// Function: SUB32 (signed int subtraction based on original snippet's type cast)
void SUB32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  SET_REG32(param_1, (_uint32_t)((_int32_t)GET_REG32(param_2) - (_int32_t)GET_REG32(param_3)));
}

// Logical AND functions (bitwise operations used for logical results)
void LAND8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  AND8(param_1, param_2, param_3);
}

void LAND16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  AND16(param_1, param_2, param_3);
}

void LAND32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  AND32(param_1, param_2, param_3);
}

// Logical OR functions (bitwise operations used for logical results)
void LOR8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  OR8(param_1, param_2, param_3);
}

void LOR16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  OR16(param_1, param_2, param_3);
}

void LOR32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  OR32(param_1, param_2, param_3);
}

// Comparison functions: store TRUE (0xFF/0xFFFF/0xFFFFFFFF) or FALSE (0)
// EQ functions use signed comparison based on original snippet's type casts.
void EQ8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  if ((_int8_t)GET_REG8(param_2) == (_int8_t)GET_REG8(param_3)) {
    MOV8(param_1, ADDR_TRUE_8BIT);
  } else {
    MOV8(param_1, ADDR_FALSE_32BIT);
  }
}

void EQ16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  if ((_int16_t)GET_REG16(param_2) == (_int16_t)GET_REG16(param_3)) {
    MOV16(param_1, ADDR_TRUE_16BIT);
  } else {
    MOV16(param_1, ADDR_FALSE_32BIT);
  }
}

void EQ32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  if ((_int32_t)GET_REG32(param_2) == (_int32_t)GET_REG32(param_3)) {
    MOV32(param_1, ADDR_TRUE_32BIT);
  } else {
    MOV32(param_1, ADDR_FALSE_32BIT);
  }
}

// GT functions use unsigned comparison based on original snippet's type casts.
void GT8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  if (GET_REG8(param_2) > GET_REG8(param_3)) {
    MOV8(param_1, ADDR_TRUE_8BIT);
  } else {
    MOV8(param_1, ADDR_FALSE_32BIT);
  }
}

void GT16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  if (GET_REG16(param_2) > GET_REG16(param_3)) {
    MOV16(param_1, ADDR_TRUE_16BIT);
  } else {
    MOV16(param_1, ADDR_FALSE_32BIT);
  }
}

void GT32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  if (GET_REG32(param_2) > GET_REG32(param_3)) {
    MOV32(param_1, ADDR_TRUE_32BIT);
  } else {
    MOV32(param_1, ADDR_FALSE_32BIT);
  }
}

// GTE functions use a temporary register for intermediate results.
void GTE8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  EQ8(param_1, param_2, param_3);
  GT8(param_4, param_2, param_3);
  LOR8(param_1, param_1, param_4);
}

void GTE16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  EQ16(param_1, param_2, param_3);
  GT16(param_4, param_2, param_3);
  LOR16(param_1, param_1, param_4);
}

void GTE32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  EQ32(param_1, param_2, param_3);
  GT32(param_4, param_2, param_3);
  LOR32(param_1, param_1, param_4);
}

// LT functions
void LT8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  GT8(param_1, param_3, param_2); // a < b is b > a
}

void LT16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  GT16(param_1, param_3, param_2);
}

void LT32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  GT32(param_1, param_3, param_2);
}

// LTE functions use a temporary register for intermediate results.
void LTE8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  GTE8(param_1, param_3, param_2, param_4); // a <= b is b >= a
}

void LTE16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  GTE16(param_1, param_3, param_2, param_4);
}

void LTE32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  GTE32(param_1, param_3, param_2, param_4);
}

// INC functions
void INC8(_uint16_t param_1) {
  ADD8(param_1, param_1, ADDR_ONE_32BIT);
}

void INC16(_uint16_t param_1) {
  ADD16(param_1, param_1, ADDR_ONE_32BIT);
}

void INC32(_uint16_t param_1) {
  ADD32(param_1, param_1, ADDR_ONE_32BIT);
}

// DEC functions
void DEC8(_uint16_t param_1) {
  SUB8(param_1, param_1, ADDR_ONE_32BIT);
}

void DEC16(_uint16_t param_1) {
  SUB16(param_1, param_1, ADDR_ONE_32BIT);
}

void DEC32(_uint16_t param_1) {
  SUB32(param_1, param_1, ADDR_ONE_32BIT);
}

// Function: MUL32 (implements multiplication via repeated addition)
// param_1: destination (result)
// param_2: operand 1
// param_3: operand 2
// param_4: base address for temporary registers (param_4, param_4+4, param_4+8)
void MUL32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t tmp_counter_reg = param_4;
  _uint16_t tmp_multiplicand_reg = param_4 + 4;
  _uint16_t tmp_gt_result_reg = param_4 + 8; // For loop condition

  MOVIM32(param_1, 0); // Initialize result to 0

  // Determine which operand is smaller to use as counter
  GT32(tmp_gt_result_reg, param_2, param_3); // Check if param_2 > param_3
  if (REG32(tmp_gt_result_reg) != 0) { // If param_2 is greater
    MOV32(tmp_counter_reg, param_3);    // Counter = param_3
    MOV32(tmp_multiplicand_reg, param_2); // Multiplicand = param_2
  } else { // If param_3 is greater or equal
    MOV32(tmp_counter_reg, param_2);    // Counter = param_2
    MOV32(tmp_multiplicand_reg, param_3); // Multiplicand = param_3
  }

  // Loop while counter > 0
  GT32(tmp_gt_result_reg, tmp_counter_reg, ADDR_ZERO_32BIT); // Check counter > 0
  while (REG32(tmp_gt_result_reg) != 0) {
    ADD32(param_1, param_1, tmp_multiplicand_reg); // result += multiplicand
    DEC32(tmp_counter_reg);                         // counter--
    GT32(tmp_gt_result_reg, tmp_counter_reg, ADDR_ZERO_32BIT); // Re-evaluate loop condition
  }
}

// Function: MUL16 (wraps MUL32 for 16-bit operations)
// param_1: destination
// param_2: operand 1
// param_3: operand 2
// param_4: base address for temporary 32-bit registers (param_4 to param_4+0xc)
void MUL16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t temp_dest_32 = param_4;
  _uint16_t temp_op1_32 = param_4 + 4;
  _uint16_t temp_op2_32 = param_4 + 8;
  _uint16_t mul32_tmp_base = param_4 + 0xc;

  MOVIM32(temp_op1_32, 0); MOV16(temp_op1_32, param_2);
  MOVIM32(temp_op2_32, 0); MOV16(temp_op2_32, param_3);

  MUL32(temp_dest_32, temp_op1_32, temp_op2_32, mul32_tmp_base);

  MOV16(param_1, temp_dest_32); // Store lower 16 bits of result
}

// Function: MUL8 (wraps MUL32 for 8-bit operations)
// param_1: destination
// param_2: operand 1
// param_3: operand 2
// param_4: base address for temporary 32-bit registers (param_4 to param_4+0xc)
void MUL8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t temp_dest_32 = param_4;
  _uint16_t temp_op1_32 = param_4 + 4;
  _uint16_t temp_op2_32 = param_4 + 8;
  _uint16_t mul32_tmp_base = param_4 + 0xc;

  MOVIM32(temp_op1_32, 0); MOV8(temp_op1_32, param_2);
  MOVIM32(temp_op2_32, 0); MOV8(temp_op2_32, param_3);

  MUL32(temp_dest_32, temp_op1_32, temp_op2_32, mul32_tmp_base);

  MOV8(param_1, temp_dest_32); // Store lower 8 bits of result
}

// Function: DIV32 (implements division via repeated subtraction)
// param_1: destination (quotient)
// param_2: dividend
// param_3: divisor
// param_4: base address for temporary registers (param_4 to param_4+0x10)
void DIV32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t tmp_current_dividend_reg = param_4;
  _uint16_t tmp_eq_gte_result_reg = param_4 + 4;
  _uint16_t tmp_gt_result_reg = param_4 + 8;
  _uint16_t tmp_land_result_reg = param_4 + 0xc;
  _uint16_t gte_internal_temp = param_4 + 0x10; // Temp for GTE32 internal call

  MOVIM32(param_1, 0); // Initialize quotient to 0
  MOV32(tmp_current_dividend_reg, param_2); // Copy dividend to a mutable temporary

  EQ32(tmp_eq_gte_result_reg, param_3, ADDR_ZERO_32BIT); // Check if divisor is zero
  if (REG32(tmp_eq_gte_result_reg) == 0) { // If divisor is NOT zero
    // Loop while current_dividend >= divisor AND current_dividend > 0
    GTE32(tmp_eq_gte_result_reg, tmp_current_dividend_reg, param_3, gte_internal_temp);
    GT32(tmp_gt_result_reg, tmp_current_dividend_reg, ADDR_ZERO_32BIT);
    LAND32(tmp_land_result_reg, tmp_eq_gte_result_reg, tmp_gt_result_reg);

    while (REG32(tmp_land_result_reg) != 0) {
      INC32(param_1);                                 // quotient++
      SUB32(tmp_current_dividend_reg, tmp_current_dividend_reg, param_3); // current_dividend -= divisor

      // Re-evaluate loop conditions
      GTE32(tmp_eq_gte_result_reg, tmp_current_dividend_reg, param_3, gte_internal_temp);
      GT32(tmp_gt_result_reg, tmp_current_dividend_reg, ADDR_ZERO_32BIT);
      LAND32(tmp_land_result_reg, tmp_eq_gte_result_reg, tmp_gt_result_reg);
    }
  }
}

// Function: DIV16 (wraps DIV32 for 16-bit operations)
// param_1: destination
// param_2: operand 1
// param_3: operand 2
// param_4: base address for temporary 32-bit registers (param_4 to param_4+0xc)
void DIV16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t temp_dest_32 = param_4;
  _uint16_t temp_op1_32 = param_4 + 4;
  _uint16_t temp_op2_32 = param_4 + 8;
  _uint16_t div32_tmp_base = param_4 + 0xc;

  MOVIM32(temp_op1_32, 0); MOV16(temp_op1_32, param_2);
  MOVIM32(temp_op2_32, 0); MOV16(temp_op2_32, param_3);

  DIV32(temp_dest_32, temp_op1_32, temp_op2_32, div32_tmp_base);

  MOV16(param_1, temp_dest_32);
}

// Function: DIV8 (wraps DIV32 for 8-bit operations)
// param_1: destination
// param_2: operand 1
// param_3: operand 2
// param_4: base address for temporary 32-bit registers (param_4 to param_4+0xc)
void DIV8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t temp_dest_32 = param_4;
  _uint16_t temp_op1_32 = param_4 + 4;
  _uint16_t temp_op2_32 = param_4 + 8;
  _uint16_t div32_tmp_base = param_4 + 0xc;

  MOVIM32(temp_op1_32, 0); MOV8(temp_op1_32, param_2);
  MOVIM32(temp_op2_32, 0); MOV8(temp_op2_32, param_3);

  DIV32(temp_dest_32, temp_op1_32, temp_op2_32, div32_tmp_base);

  MOV8(param_1, temp_dest_32);
}

// Function: REM32 (implements remainder via repeated subtraction)
// param_1: destination (remainder)
// param_2: dividend
// param_3: divisor
// param_4: base address for temporary registers (param_4 to param_4+0x10)
void REM32(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t tmp_current_dividend_reg = param_4;
  _uint16_t tmp_eq_gte_result_reg = param_4 + 4;
  _uint16_t tmp_gt_result_reg = param_4 + 8;
  _uint16_t tmp_land_result_reg = param_4 + 0xc;
  _uint16_t gte_internal_temp = param_4 + 0x10; // Temp for GTE32 internal call

  MOVIM32(param_1, 0); // Initialize remainder to 0 (will be overwritten by final tmp_current_dividend_reg)
  MOV32(tmp_current_dividend_reg, param_2); // Copy dividend to a mutable temporary

  EQ32(tmp_eq_gte_result_reg, param_3, ADDR_ZERO_32BIT); // Check if divisor is zero
  if (REG32(tmp_eq_gte_result_reg) == 0) { // If divisor is NOT zero
    // Loop while current_dividend >= divisor AND current_dividend > 0
    GTE32(tmp_eq_gte_result_reg, tmp_current_dividend_reg, param_3, gte_internal_temp);
    GT32(tmp_gt_result_reg, tmp_current_dividend_reg, ADDR_ZERO_32BIT);
    LAND32(tmp_land_result_reg, tmp_eq_gte_result_reg, tmp_gt_result_reg);

    while (REG32(tmp_land_result_reg) != 0) {
      SUB32(tmp_current_dividend_reg, tmp_current_dividend_reg, param_3); // current_dividend -= divisor

      // Re-evaluate loop conditions
      GTE32(tmp_eq_gte_result_reg, tmp_current_dividend_reg, param_3, gte_internal_temp);
      GT32(tmp_gt_result_reg, tmp_current_dividend_reg, ADDR_ZERO_32BIT);
      LAND32(tmp_land_result_reg, tmp_eq_gte_result_reg, tmp_gt_result_reg);
    }
    MOV32(param_1, tmp_current_dividend_reg); // Store the final remainder
  }
}

// Function: REM16 (wraps REM32 for 16-bit operations)
// param_1: destination
// param_2: operand 1
// param_3: operand 2
// param_4: base address for temporary 32-bit registers (param_4 to param_4+0xc)
void REM16(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t temp_dest_32 = param_4;
  _uint16_t temp_op1_32 = param_4 + 4;
  _uint16_t temp_op2_32 = param_4 + 8;
  _uint16_t rem32_tmp_base = param_4 + 0xc;

  MOVIM32(temp_op1_32, 0); MOV16(temp_op1_32, param_2);
  MOVIM32(temp_op2_32, 0); MOV16(temp_op2_32, param_3);

  REM32(temp_dest_32, temp_op1_32, temp_op2_32, rem32_tmp_base);

  MOV16(param_1, temp_dest_32);
}

// Function: REM8 (wraps REM32 for 8-bit operations)
// param_1: destination
// param_2: operand 1
// param_3: operand 2
// param_4: base address for temporary 32-bit registers (param_4 to param_4+0xc)
void REM8(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3, _uint16_t param_4) {
  _uint16_t temp_dest_32 = param_4;
  _uint16_t temp_op1_32 = param_4 + 4;
  _uint16_t temp_op2_32 = param_4 + 8;
  _uint16_t rem32_tmp_base = param_4 + 0xc;

  MOVIM32(temp_op1_32, 0); MOV8(temp_op1_32, param_2);
  MOVIM32(temp_op2_32, 0); MOV8(temp_op2_32, param_3);

  REM32(temp_dest_32, temp_op1_32, temp_op2_32, rem32_tmp_base);

  MOV8(param_1, temp_dest_32);
}

// Function: INIT_MACHINE
void INIT_MACHINE(void) {
  if (allocate() != 0) {
    _terminate(1);
  }

  // Preserve the original base pointer to calculate _consts
  uint8_t *original_state_base = state;
  _consts = original_state_base + 0xfe8;
  // Adjust the 'state' pointer; all subsequent REG/MOV operations will use this new base.
  // This means addresses like 0x100 are relative to (original_state_base + 0xee8).
  state = original_state_base + 0xee8;

  // Initialize memory region (0x0 to 0xFC relative to new 'state' base) with 0s
  for (_uint16_t addr = 0; addr < 0x100; addr += 4) {
    MOVIM32(addr, 0);
  }

  // Initialize specific constant registers
  MOVIM32(ADDR_ZERO_32BIT, 0);
  MOVIM32(ADDR_ONE_32BIT, 1);
  MOVIM32(ADDR_TRUE_8BIT, 0xFF);
  MOVIM32(ADDR_TRUE_16BIT, 0xFFFF);
  MOVIM32(ADDR_TRUE_32BIT, 0xFFFFFFFF);
  MOVIM32(ADDR_FALSE_32BIT, 0);
}

// Function: printChar
void printChar(_uint16_t param_1) {
  _uint8_t c = REG8(param_1);
  int bytes_transmitted;
  if (transmit(STDOUT_FILENO, &c, 1, &bytes_transmitted) != 0 || bytes_transmitted != 1) {
    _terminate(1);
  }
}

// Function: getChar
_uint32_t getChar(_uint16_t param_1) {
  fd_set fds;
  _uint32_t tv_sec_val = 1; // 1 second timeout as per original snippet's local_a4 = 1
  _uint32_t ready_count_val = 0; // Will store the number of ready FDs

  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);

  // Call the emulated fdwait
  _int32_t fdwait_result = fdwait(STDIN_FILENO + 1, &fds, NULL, &tv_sec_val, &ready_count_val);

  if (fdwait_result == 0 && ready_count_val == 1) {
    _uint8_t received_char;
    int bytes_received;
    if (receive(STDIN_FILENO, &received_char, 1, &bytes_received) == 0 && bytes_received == 1) {
      MOVIM8(param_1, received_char);
      return 0; // Success
    }
  }
  return 0xFFFFFFFF; // Failure
}

// Function: printString
// param_1: address of string start in state memory
// param_2: address to store length (original only uses lower 8 bits, so MOV8)
// param_3: base address for temporary registers
void printString(_uint16_t param_1, _uint16_t param_2, _uint16_t param_3) {
  _uint16_t current_str_ptr_reg = param_3;     // Pointer to current character in string
  _uint16_t char_val_reg = param_3 + 2;        // Holds current character value
  _uint16_t eq_zero_result_reg = param_3 + 3;  // Result of char == 0 check
  _uint16_t length_counter_reg = param_3 + 4;  // Counts characters for length

  MOV16(current_str_ptr_reg, param_1); // Initialize string pointer
  MOVIM8(length_counter_reg, 0);       // Initialize length to 0

  while (1) {
    MOV8(char_val_reg, current_str_ptr_reg);          // Get character
    EQ8(eq_zero_result_reg, char_val_reg, ADDR_ZERO_32BIT); // Check if it's null terminator
    if (REG8(eq_zero_result_reg) != 0) break;         // If true, end of string

    printChar(char_val_reg); // Print the character
    INC8(length_counter_reg); // Increment length counter (8-bit)
    INC16(current_str_ptr_reg); // Advance string pointer
  }
  MOV8(param_2, length_counter_reg); // Store final length
}

// Function: printReg32 (prints the decimal value of a 32-bit register)
// param_1: address of the 32-bit register to print
// param_2: base address for temporary registers
void printReg32(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t val_reg = param_2;               // Holds the current remainder
  _uint16_t current_divisor_reg = param_2 + 4; // Holds the current divisor
  _uint16_t quotient_reg = param_2 + 8;      // Holds the quotient (digit)
  _uint16_t digit_char_reg = param_2 + 12;   // Holds the ASCII character for the digit
  _uint16_t div_rem_tmp_base = param_2 + 16; // Base for DIV32/REM32 internal temporaries (needs 0x10 bytes)

  MOV32(val_reg, param_1); // Copy value to temporary register

  _uint32_t divisors[] = {
      1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1
  };
  _Bool leading_zeros_suppressed = 1;

  for (int i = 0; i < sizeof(divisors) / sizeof(divisors[0]); ++i) {
    MOVIM32(current_divisor_reg, divisors[i]); // Set current divisor

    _uint32_t divisor_val = REG32(current_divisor_reg);
    if (divisor_val == 0) continue; // Should not happen with these constants

    DIV32(quotient_reg, val_reg, current_divisor_reg, div_rem_tmp_base); // Calculate digit
    _uint8_t digit = REG8(quotient_reg); // Get the lowest byte (the digit)

    // Print digit if it's non-zero, or if we've already printed a non-zero digit, or if it's the last digit
    if (digit != 0 || !leading_zeros_suppressed || i == (sizeof(divisors) / sizeof(divisors[0])) - 1) {
      leading_zeros_suppressed = 0;
      MOVIM8(digit_char_reg, '0');
      ADD8(digit_char_reg, digit_char_reg, quotient_reg); // Convert digit to ASCII
      printChar(digit_char_reg);
    }

    REM32(val_reg, val_reg, current_divisor_reg, div_rem_tmp_base); // Calculate new remainder
  }
}

// Function: printReg16 (wraps printReg32 for 16-bit operations)
void printReg16(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t temp_reg32 = param_2; // Use param_2 as the 32-bit temporary register
  MOVIM32(temp_reg32, 0);     // Clear the 32-bit temp
  MOV16(temp_reg32, param_1); // Copy 16-bit value into lower 16 bits of temp_reg32
  printReg32(temp_reg32, param_2 + 4); // param_2+4 ensures printReg32 has enough temp space
}

// Function: printReg8 (wraps printReg32 for 8-bit operations)
void printReg8(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t temp_reg32 = param_2; // Use param_2 as the 32-bit temporary register
  MOVIM32(temp_reg32, 0);     // Clear the 32-bit temp
  MOV8(temp_reg32, param_1);  // Copy 8-bit value into lower 8 bits of temp_reg32
  printReg32(temp_reg32, param_2 + 4); // param_2+4 ensures printReg32 has enough temp space
}

// Function: isSpace (checks if character is space or tab)
// param_1: address of character to check
// param_2: base address for temporary registers
_uint8_t isSpace(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t temp_space_char_reg = param_2;
  _uint16_t temp_tab_char_reg = param_2 + 1;
  _uint16_t eq_space_result_reg = param_2 + 2;
  _uint16_t eq_tab_result_reg = param_2 + 3;
  _uint16_t lor_result_reg = param_2 + 2; // Reuse eq_space_result_reg for final result

  MOVIM8(temp_space_char_reg, ' ');
  MOVIM8(temp_tab_char_reg, '\t');

  EQ8(eq_space_result_reg, param_1, temp_space_char_reg);
  EQ8(eq_tab_result_reg, param_1, temp_tab_char_reg);
  LOR8(lor_result_reg, eq_space_result_reg, eq_tab_result_reg);

  return REG8(lor_result_reg);
}

// Function: isNumber (checks if character is '0'-'9')
// param_1: address of character to check
// param_2: base address for temporary registers
_uint8_t isNumber(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t temp_zero_char_reg = param_2;
  _uint16_t diff_reg = param_2 + 1;
  _uint16_t temp_ten_reg = param_2 + 2;
  _uint16_t lt_result_reg = param_2 + 3;

  MOVIM8(temp_zero_char_reg, '0');
  SUB8(diff_reg, param_1, temp_zero_char_reg); // diff_reg = char_to_check - '0'
  MOVIM8(temp_ten_reg, 10);
  LT8(lt_result_reg, diff_reg, temp_ten_reg); // lt_result_reg = (diff_reg < 10)

  return REG8(lt_result_reg);
}

// Function: isUpLetter (checks if character is 'A'-'Z')
// param_1: address of character to check
// param_2: base address for temporary registers
_uint8_t isUpLetter(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t temp_A_char_reg = param_2;
  _uint16_t temp_Z_char_reg = param_2 + 1;
  _uint16_t gte_result_reg = param_2 + 2;
  _uint16_t lte_result_reg = param_2 + 3;
  _uint16_t land_result_reg = param_2 + 2; // Reuse gte_result_reg for final result
  _uint16_t gte_lte_internal_temp = param_2 + 4; // For GTE8/LTE8 internal use

  MOVIM8(temp_A_char_reg, 'A');
  MOVIM8(temp_Z_char_reg, 'Z');

  GTE8(gte_result_reg, param_1, temp_A_char_reg, gte_lte_internal_temp);
  LTE8(lte_result_reg, param_1, temp_Z_char_reg, gte_lte_internal_temp);
  LAND8(land_result_reg, gte_result_reg, lte_result_reg);

  return REG8(land_result_reg);
}

// Function: isLowLetter (checks if character is 'a'-'z')
// param_1: address of character to check
// param_2: base address for temporary registers
_uint8_t isLowLetter(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t temp_a_char_reg = param_2;
  _uint16_t temp_z_char_reg = param_2 + 1;
  _uint16_t gte_result_reg = param_2 + 2;
  _uint16_t lte_result_reg = param_2 + 3;
  _uint16_t land_result_reg = param_2 + 2; // Reuse gte_result_reg for final result
  _uint16_t gte_lte_internal_temp = param_2 + 4; // For GTE8/LTE8 internal use

  MOVIM8(temp_a_char_reg, 'a');
  MOVIM8(temp_z_char_reg, 'z');

  GTE8(gte_result_reg, param_1, temp_a_char_reg, gte_lte_internal_temp);
  LTE8(lte_result_reg, param_1, temp_z_char_reg, gte_lte_internal_temp);
  LAND8(land_result_reg, gte_result_reg, lte_result_reg);

  return REG8(land_result_reg);
}

// Function: isSpecial (checks if character falls into several special ASCII ranges)
// param_1: address of character to check
// param_2: base address for temporary registers
_uint8_t isSpecial(_uint16_t param_1, _uint16_t param_2) {
  _uint16_t range_start_1 = param_2;
  _uint16_t range_end_1 = param_2 + 1;
  _uint16_t range_start_2 = param_2 + 2;
  _uint16_t range_end_2 = param_2 + 3;
  _uint16_t range_start_3 = param_2 + 4;
  _uint16_t range_end_3 = param_2 + 5;
  _uint16_t range_start_4 = param_2 + 6; // Corrected from 0x50 to 0x7b
  _uint16_t range_end_4 = param_2 + 7;

  _uint16_t current_overall_result_reg = param_2 + 8; // Accumulates OR results
  _uint16_t gte_result_reg = param_2 + 9;
  _uint16_t lte_result_reg = param_2 + 10;
  _uint16_t land_result_reg = param_2 + 11;
  _uint16_t gte_lte_internal_temp = param_2 + 12; // For GTE8/LTE8 internal use

  MOVIM8(current_overall_result_reg, 0); // Initialize overall result to false

  // Range 1: [0x21, 0x2F] (! to /)
  MOVIM8(range_start_1, 0x21); MOVIM8(range_end_1, 0x2f);
  GTE8(gte_result_reg, param_1, range_start_1, gte_lte_internal_temp);
  LTE8(lte_result_reg, param_1, range_end_1, gte_lte_internal_temp);
  LAND8(land_result_reg, gte_result_reg, lte_result_reg);
  LOR8(current_overall_result_reg, current_overall_result_reg, land_result_reg);

  // Range 2: [0x3A, 0x40] (: to @)
  MOVIM8(range_start_2, 0x3a); MOVIM8(range_end_2, 0x40);
  GTE8(gte_result_reg, param_1, range_start_2, gte_lte_internal_temp);
  LTE8(lte_result_reg, param_1, range_end_2, gte_lte_internal_temp);
  LAND8(land_result_reg, gte_result_reg, lte_result_reg);
  LOR8(current_overall_result_reg, current_overall_result_reg, land_result_reg);

  // Range 3: [0x5B, 0x60] ([ to `)
  MOVIM8(range_start_3, 0x5b); MOVIM8(range_end_3, 0x60);
  GTE8(gte_result_reg, param_1, range_start_3, gte_lte_internal_temp);
  LTE8(lte_result_reg, param_1, range_end_3, gte_lte_internal_temp);
  LAND8(land_result_reg, gte_result_reg, lte_result_reg);
  LOR8(current_overall_result_reg, current_overall_result_reg, land_result_reg);

  // Range 4: [0x7B, 0x7E] ({ to ~) - Original 0x50 was likely a typo.
  MOVIM8(range_start_4, 0x7b); MOVIM8(range_end_4, 0x7e);
  GTE8(gte_result_reg, param_1, range_start_4, gte_lte_internal_temp);
  LTE8(lte_result_reg, param_1, range_end_4, gte_lte_internal_temp);
  LAND8(land_result_reg, gte_result_reg, lte_result_reg);
  LOR8(current_overall_result_reg, current_overall_result_reg, land_result_reg);

  return REG8(current_overall_result_reg);
}

// Function: insertString
// param_1: destination address in state memory
// param_2: C string pointer (char*)
// param_3: base address for temporary registers
void insertString(_uint16_t param_1, const char *param_2, _uint16_t param_3) {
  _uint16_t current_dest_addr_reg = param_3; // Pointer to current destination in state memory
  MOV16(current_dest_addr_reg, param_1);     // Initialize destination pointer

  if (param_2 != NULL) {
    for (int i = 0; param_2[i] != '\0'; ++i) {
      MOVIM8(current_dest_addr_reg, param_2[i]); // Write character to state memory
      INC16(current_dest_addr_reg);             // Advance destination pointer
    }
  }
  MOVIM8(current_dest_addr_reg, 0); // Null-terminate the string in state memory
}

// Function: main
_uint32_t main(void) {
  INIT_MACHINE();

  // Initialize character counters (addresses 1, 2, 3, 4, 5 are 8-bit counters)
  MOVIM8(1, 0); // numbers_count
  MOVIM8(2, 0); // uppers_count
  MOVIM8(3, 0); // lowers_count
  MOVIM8(4, 0); // special_count
  MOVIM8(5, 0); // other_count
  MOVIM32(6, 0); // spaces_count (original snippet uses INC32, so 32-bit)

  // Register addresses storing pointers to strings
  _uint16_t REG_NUMBERS_STR_PTR = 7;
  _uint16_t REG_SPACES_STR_PTR = 0xD;  // 0x90
  _uint16_t REG_UPPERS_STR_PTR = 9;   // 0xA0
  _uint16_t REG_LOWERS_STR_PTR = 0xB;  // 0xB0
  _uint16_t REG_SPECIAL_STR_PTR = 0xF; // 0xC0
  _uint16_t REG_OTHER_STR_PTR = 0x11; // 0xD0
  _uint16_t REG_NEWLINE_STR_PTR = 0x13; // 0xE0

  // String data addresses (relative to the adjusted 'state' base)
  _uint16_t numbers_str_data_addr = 0x80;
  _uint16_t spaces_str_data_addr = 0x90;
  _uint16_t uppers_str_data_addr = 0xA0;
  _uint16_t lowers_str_data_addr = 0xB0;
  _uint16_t special_str_data_addr = 0xC0;
  _uint16_t other_str_data_addr = 0xD0;
  _uint16_t newline_str_data_addr = 0xE0;

  // Set registers to hold string data addresses
  MOVIM16(REG_NUMBERS_STR_PTR, numbers_str_data_addr);
  MOVIM16(REG_SPACES_STR_PTR, spaces_str_data_addr);
  MOVIM16(REG_UPPERS_STR_PTR, uppers_str_data_addr);
  MOVIM16(REG_LOWERS_STR_PTR, lowers_str_data_addr);
  MOVIM16(REG_SPECIAL_STR_PTR, special_str_data_addr);
  MOVIM16(REG_OTHER_STR_PTR, other_str_data_addr);
  MOVIM16(REG_NEWLINE_STR_PTR, newline_str_data_addr);

  // Temporary registers for function calls in main loop
  _uint16_t temp_char_input_reg = 0x15; // For getChar/printChar input/output
  _uint16_t temp_op_base_reg = 0x16;    // Base for is* functions' temporaries
  _uint16_t temp_length_result_reg = 0x19; // For printString length result

  // Insert prompt strings into state memory
  insertString(REG_NUMBERS_STR_PTR, "numbers  = ", temp_op_base_reg);
  insertString(REG_UPPERS_STR_PTR, "uppers   = ", temp_op_base_reg);
  insertString(REG_LOWERS_STR_PTR, "lowers   = ", temp_op_base_reg);
  insertString(REG_SPACES_STR_PTR, "spaces   = ", temp_op_base_reg);
  insertString(REG_SPECIAL_STR_PTR, "special  = ", temp_op_base_reg);
  insertString(REG_OTHER_STR_PTR, "other    = ", temp_op_base_reg);
  MOVIM8(REG_NEWLINE_STR_PTR, '\n'); // Store newline character
  MOVIM8(REG_NEWLINE_STR_PTR + 1, 0); // Null-terminate newline string

  _int32_t getchar_status;
  getchar_status = getChar(temp_char_input_reg); // Read first character

  while (getchar_status != -1) { // Loop until EOF (getChar returns -1)
    if (isUpLetter(temp_char_input_reg, temp_op_base_reg) != 0) {
      INC8(2); // uppers_count
    } else if (isLowLetter(temp_char_input_reg, temp_op_base_reg) != 0) {
      INC8(3); // lowers_count
    } else if (isNumber(temp_char_input_reg, temp_op_base_reg) != 0) {
      INC8(1); // numbers_count
    } else if (isSpace(temp_char_input_reg, temp_op_base_reg) != 0) {
      INC32(6); // spaces_count
    } else if (isSpecial(temp_char_input_reg, temp_op_base_reg) != 0) {
      INC8(4); // special_count
    } else {
      INC8(5); // other_count
    }

    printChar(temp_char_input_reg); // Echo the character
    getchar_status = getChar(temp_char_input_reg); // Read next character
  }

  // Print results
  printString(REG_NUMBERS_STR_PTR, temp_length_result_reg, temp_op_base_reg);
  printReg8(1, temp_op_base_reg);
  printString(REG_NEWLINE_STR_PTR, temp_length_result_reg, temp_op_base_reg);

  printString(REG_UPPERS_STR_PTR, temp_length_result_reg, temp_op_base_reg);
  printReg8(2, temp_op_base_reg);
  printString(REG_NEWLINE_STR_PTR, temp_length_result_reg, temp_op_base_reg);

  printString(REG_LOWERS_STR_PTR, temp_length_result_reg, temp_op_base_reg);
  printReg8(3, temp_op_base_reg);
  printString(REG_NEWLINE_STR_PTR, temp_length_result_reg, temp_op_base_reg);

  printString(REG_SPACES_STR_PTR, temp_length_result_reg, temp_op_base_reg);
  printReg32(6, temp_op_base_reg); // spaces_count is 32-bit
  printString(REG_NEWLINE_STR_PTR, temp_length_result_reg, temp_op_base_reg);

  printString(REG_SPECIAL_STR_PTR, temp_length_result_reg, temp_op_base_reg);
  printReg8(4, temp_op_base_reg);
  printString(REG_NEWLINE_STR_PTR, temp_length_result_reg, temp_op_base_reg);

  printString(REG_OTHER_STR_PTR, temp_length_result_reg, temp_op_base_reg);
  printReg8(5, temp_op_base_reg);
  printString(REG_NEWLINE_STR_PTR, temp_length_result_reg, temp_op_base_reg);

  _terminate(0); // Clean up and exit
  return 0;
}