#include <stdio.h>    // For printf, fgets, snprintf
#include <stdlib.h>   // For strtoul
#include <stdbool.h>  // For bool
#include <limits.h>   // For UINT_MAX
#include <string.h>   // For strcspn (to remove newline from fgets)

// Custom type mapping
typedef unsigned int uint;

// --- Constants ---
#define LARGE_INT_SIZE 8 // largeInts are arrays of 8 unsigned ints
#define USER_COUNT 21    // 0x14 + 1
#define MAX_PASSWORDS_PER_USER 10
#define HANDLER_TABLE_DIM 50 // 0x32
#define HANDLER_TABLE_TOTAL_SIZE (HANDLER_TABLE_DIM * HANDLER_TABLE_DIM * HANDLER_TABLE_DIM)

// --- Global Variables (declarations) ---
// handlerTable: void (*handlerTable[50][50][50])(void);
// Access: handlerTable[secret3 + secret2 * HANDLER_TABLE_DIM + secret1 * (HANDLER_TABLE_DIM * HANDLER_TABLE_DIM)]
void (*handlerTable[HANDLER_TABLE_TOTAL_SIZE])(void);

// userPasswords: unsigned int userPasswords[21][10][3];
// Access: userPasswords[userId][pwSlot][0/1/2]
unsigned int userPasswords[USER_COUNT][MAX_PASSWORDS_PER_USER][3];

// userToPassword: unsigned int userToPassword[USER_COUNT];
unsigned int userToPassword[USER_COUNT];

// String literals
const char DAT_0001300a[] = ": ";
const char DAT_0001300e[] = ", ";
const char DAT_00013012[] = "\n";

// --- Helper Functions (mock implementations for compilation) ---
// Replace my_printf with printf
#define my_printf printf

// Replace snprintdecimal with snprintf
#define snprintdecimal(buf, size, val) snprintf(buf, size, "%u", val)

// Replace readLine with fgets (returns 1 on successful read, 0 on EOF/error). Removes newline.
int readLine(int fd, char *buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return 0; // Error or EOF
    }
    // Remove trailing newline character if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 1;
}

// Replace strToUint32 with strtoul
uint strToUint32(const char *str) {
    return (uint)strtoul(str, NULL, 10);
}

// Helper macro for carry detection in largeIntAdd
// Returns 1 if x + y would overflow an unsigned int, 0 otherwise.
#define _CARRY(x, y) ((unsigned long long)(x) + (y) > UINT_MAX)

// Function: intToLargeInt
// Sets a large_int (array of 8 unsigned int) to a single unsigned int value.
// The value is placed in the least significant word (index 7).
unsigned int * intToLargeInt(unsigned int *largeInt, unsigned int value) {
  if (largeInt != NULL) {
    for (int i = 0; i < LARGE_INT_SIZE; i++) {
      largeInt[i] = 0;
    }
    largeInt[LARGE_INT_SIZE - 1] = value;
  }
  return largeInt;
}

// Function: printLargeInt
// Note: This function appears to be incorrectly decompiled or unused in the original snippet.
// It takes an int, but largeInt operations use unsigned int*.
// The original logic just iterates 8 times and returns 0x8 or 0.
// I'll fix the types to be consistent with largeInt, but keep its current (unhelpful) logic.
void printLargeInt(unsigned int *largeInt) {
  if (largeInt != NULL) {
    // Placeholder for actual printing logic if needed later
    // The original loop just iterated without doing anything meaningful with the largeInt.
    for (int i = 0; i < LARGE_INT_SIZE; i++) {
        // Do nothing, just iterate
    }
  }
}

// Function: largeIntAdd
// Adds two large integers (op2, op3) and stores the result in op1.
// All are arrays of 8 unsigned int.
void largeIntAdd(unsigned int *op1, unsigned int *op2, unsigned int *op3) {
  uint carry_in = 0;
  if ((op1 != NULL) && (op2 != NULL) && (op3 != NULL)) {
    // Iterate from least significant word (index 7) to most significant (index 0)
    for (int i = 0; i < LARGE_INT_SIZE; i++) {
      uint uVar1 = op2[LARGE_INT_SIZE - 1 - i];
      uint uVar2 = op3[LARGE_INT_SIZE - 1 - i];
      
      // Calculate sum of current carry and one operand
      uint uVar3 = carry_in + uVar1; 
      
      // Calculate new carry. _CARRY(carry_in, uVar1) checks overflow of carry_in + uVar1.
      // _CARRY(uVar2, uVar3) checks overflow of uVar2 + (carry_in + uVar1).
      // The sum of these two carries gives the new carry_in for the next word.
      carry_in = _CARRY(carry_in, uVar1) + _CARRY(uVar2, uVar3);
      
      // Store the result word
      op1[LARGE_INT_SIZE - 1 - i] = uVar2 + uVar3; // uVar2 + (uVar1 + old_carry_in)
    }
  }
}

// Function: largeIntShl1
// Performs a left shift by 1 bit on a large integer.
void largeIntShl1(unsigned int *largeInt) {
  if (largeInt != NULL) {
    // Iterate from most significant word (index 0) to least significant (index 7) - 1
    // The most significant word shifts its MSB out, and receives a bit from the next word's MSB.
    for (int i = 0; i < LARGE_INT_SIZE - 1; i++) {
      largeInt[i] = largeInt[i] * 2; // Shift current word left by 1
      // Bring in the MSB of the next word (index i+1) as the LSB of current word
      largeInt[i] |= largeInt[i + 1] >> 31; 
    }
    // Shift the least significant word (index 7)
    largeInt[LARGE_INT_SIZE - 1] *= 2; 
  }
}

// Function: largeIntShl
// Performs a left shift on a large integer by 256 bits.
void largeIntShl(unsigned int *largeInt) {
  for (int i = 0; i < 0x100; i++) { // 0x100 = 256 iterations
    largeIntShl1(largeInt);
  }
}

// Function: largeIntMul
// Multiplies two large integers (op2, op3) and stores the result in op1.
// Uses a binary multiplication algorithm (add-and-shift).
void largeIntMul(unsigned int *op1, unsigned int *op2, unsigned int *op3) {
  // temp_res will hold the accumulating result (initially 0)
  unsigned int temp_res[LARGE_INT_SIZE]; 
  // temp_op2_copy will be shifted left in each iteration
  unsigned int temp_op2_copy[LARGE_INT_SIZE]; 
  
  if ((op1 != NULL) && (op2 != NULL) && (op3 != NULL)) {
    // Initialize temp_res to 0
    for (int i = 0; i < LARGE_INT_SIZE; i++) {
      temp_res[i] = 0;
    }
    // Copy op2 to temp_op2_copy
    for (int i = 0; i < LARGE_INT_SIZE; i++) {
      temp_op2_copy[i] = op2[i];
    }
    
    // Iterate 256 times (for 256-bit multiplication)
    for (int i = 0; i < 0x100; i++) { 
      // Check the i-th bit of op3 (multiplier)
      // (LARGE_INT_SIZE - 1 - (i >> 5)) gives the word index for the current bit
      // (i & 0x1f) gives the bit position within that word
      if ((op3[LARGE_INT_SIZE - 1 - (i >> 5)] & (1U << (i & 0x1f))) != 0) {
        // If the bit is set, add temp_op2_copy to temp_res
        largeIntAdd(temp_res, temp_res, temp_op2_copy);
      }
      // Shift temp_op2_copy left by 1 for the next iteration
      largeIntShl1(temp_op2_copy);
    }
    
    // Copy the final result from temp_res to op1
    for (int i = 0; i < LARGE_INT_SIZE; i++) {
      op1[i] = temp_res[i];
    }
  }
}

// Function: largeIntCmp
// Compares two large integers.
// Returns 1 if op1 > op2, -1 (0xffffffff) if op1 < op2, 0 if op1 == op2.
int largeIntCmp(unsigned int *op1, unsigned int *op2) {
  if ((op1 != NULL) && (op2 != NULL)) {
    // Iterate from most significant word (index 0) to least significant (index 7)
    for (int i = 0; i < LARGE_INT_SIZE; i++) {
      if (op2[i] < op1[i]) { // If op1's current word is greater than op2's
        return 1;
      }
      if (op1[i] < op2[i]) { // If op1's current word is less than op2's
        return -1; // Original returned 0xffffffff, which is -1 for int
      }
    }
  }
  return 0; // All words are equal
}

// Function: userMenu wrappers
void userMenu(unsigned int userId); // Forward declaration

void userMenu1(void) { userMenu(1); }
void userMenu2(void) { userMenu(2); }
void userMenu3(void) { userMenu(3); }
void userMenu4(void) { userMenu(4); }
void userMenu5(void) { userMenu(5); }
void userMenu6(void) { userMenu(6); }
void userMenu7(void) { userMenu(7); }
void userMenu8(void) { userMenu(8); }
void userMenu9(void) { userMenu(9); }
void userMenu10(void) { userMenu(10); }
void userMenu11(void) { userMenu(0xb); } // 11
void userMenu12(void) { userMenu(0xc); } // 12
void userMenu13(void) { userMenu(0xd); } // 13
void userMenu14(void) { userMenu(0xe); } // 14
void userMenu15(void) { userMenu(0xf); } // 15
void userMenu16(void) { userMenu(0x10); } // 16
void userMenu17(void) { userMenu(0x11); } // 17
void userMenu18(void) { userMenu(0x12); } // 18
void userMenu19(void) { userMenu(0x13); } // 19
void userMenu20(void) { userMenu(0x14); } // 20
void userMenu21(void) { userMenu(0x15); } // 21

// Function: initTable
// Initializes the global handlerTable with 0s and specific menu functions.
void initTable(void) {
  // Loop counters for the 3D array (50x50x50)
  for (int c = 0; c < HANDLER_TABLE_DIM; c++) {
    for (int k = 0; k < HANDLER_TABLE_DIM; k++) {
      for (int l = 0; l < HANDLER_TABLE_DIM; l++) {
        handlerTable[l + k * HANDLER_TABLE_DIM + c * (HANDLER_TABLE_DIM * HANDLER_TABLE_DIM)] = NULL;
      }
    }
  }
  
  // Assign specific user menu functions to calculated indices
  // The original indices were byte offsets, divided by 4 for unsigned int pointer index.
  handlerTable[30820 / 4] = userMenu1;
  handlerTable[11232 / 4] = userMenu2;
  handlerTable[61640 / 4] = userMenu3;
  handlerTable[92460 / 4] = userMenu4;
  handlerTable[32072 / 4] = userMenu5;
  handlerTable[72868 / 4] = userMenu6;
  handlerTable[123280 / 4] = userMenu7;
  handlerTable[43488 / 4] = userMenu8;
  handlerTable[33696 / 4] = userMenu9;
  handlerTable[183884 / 4] = userMenu10;
  handlerTable[113108 / 4] = userMenu11;
  handlerTable[154100 / 4] = userMenu12;
  handlerTable[184920 / 4] = userMenu13;
  handlerTable[64144 / 4] = userMenu14;
  handlerTable[145736 / 4] = userMenu15;
  handlerTable[66532 / 4] = userMenu16;
  handlerTable[215740 / 4] = userMenu17;
  handlerTable[164764 / 4] = userMenu18;
  handlerTable[56160 / 4] = userMenu19;
  handlerTable[276148 / 4] = userMenu20;
  handlerTable[246560 / 4] = userMenu21;
}

// Function: initPasswords
// Initializes the global userPasswords array to all zeros.
void initPasswords(void) {
  for (int userIdx = 0; userIdx < USER_COUNT; userIdx++) {
    for (int pwSlot = 0; pwSlot < MAX_PASSWORDS_PER_USER; pwSlot++) {
      userPasswords[userIdx][pwSlot][0] = 0;
      userPasswords[userIdx][pwSlot][1] = 0;
      userPasswords[userIdx][pwSlot][2] = 0;
    }
  }
}

// Function: addPW
// Adds a new password to the first available slot for a given user.
// Returns 0 on success, -1 on failure (no available slots).
int addPW(unsigned int userId, unsigned int secret1, unsigned int secret2, unsigned int secret3) {
  for (int pwSlot = 0; pwSlot < MAX_PASSWORDS_PER_USER; pwSlot++) {
    // Check if the current password slot is empty (all three parts are 0)
    if ((userPasswords[userId][pwSlot][0] == 0) &&
        (userPasswords[userId][pwSlot][1] == 0) &&
        (userPasswords[userId][pwSlot][2] == 0)) {
      // Found an empty slot, store the new password parts
      userPasswords[userId][pwSlot][0] = secret1;
      userPasswords[userId][pwSlot][1] = secret2;
      userPasswords[userId][pwSlot][2] = secret3;
      return 0; // Success
    }
  }
  return -1; // No available slots
}

// Function: rmPW
// Removes a password by its display number (not array index) for a given user.
// Returns 0 on success, -1 on failure (password number not found).
int rmPW(unsigned int userId, unsigned int pwNumToRemove) {
  int validPwCount = 0; // Counts only non-empty password slots
  for (int pwSlot = 0; pwSlot < MAX_PASSWORDS_PER_USER; pwSlot++) {
    // Check if the current password slot is NOT empty
    if (!((userPasswords[userId][pwSlot][0] == 0) &&
          (userPasswords[userId][pwSlot][1] == 0) &&
          (userPasswords[userId][pwSlot][2] == 0))) {
      // This is a valid (non-empty) password slot
      if (pwNumToRemove == validPwCount) {
        // Found the password to remove, clear its parts
        userPasswords[userId][pwSlot][0] = 0;
        userPasswords[userId][pwSlot][1] = 0;
        userPasswords[userId][pwSlot][2] = 0;
        return 0; // Success
      }
      validPwCount++; // Increment count of valid passwords
    }
  }
  return -1; // Password number not found
}

// Function: printPW
// Prints all stored passwords for a given user.
void printPW(unsigned int userId) {
  char buffer[64];
  int validPwCount = 0;
  
  for (int pwSlot = 0; pwSlot < MAX_PASSWORDS_PER_USER; pwSlot++) {
    // Check if the current password slot is NOT empty
    if (!((userPasswords[userId][pwSlot][0] == 0) &&
          (userPasswords[userId][pwSlot][1] == 0) &&
          (userPasswords[userId][pwSlot][2] == 0))) {
      my_printf("Password ");
      snprintdecimal(buffer, sizeof(buffer), validPwCount);
      my_printf(buffer);
      my_printf(DAT_0001300a); // ": "
      
      snprintdecimal(buffer, sizeof(buffer), userPasswords[userId][pwSlot][0]);
      my_printf(buffer);
      my_printf(DAT_0001300e); // ", "
      
      snprintdecimal(buffer, sizeof(buffer), userPasswords[userId][pwSlot][1]);
      my_printf(buffer);
      my_printf(DAT_0001300e); // ", "
      
      snprintdecimal(buffer, sizeof(buffer), userPasswords[userId][pwSlot][2]);
      my_printf(buffer);
      my_printf(DAT_00013012); // "\n"
      
      validPwCount++;
    }
  }
}

// Function: userMenu
// Provides an interactive menu for a specific user.
void userMenu(unsigned int userId) {
  char input_buffer[64];
  unsigned int val1, val2, val3; // Used for secrets or password number
  
  while (true) { // Replaced goto with an infinite loop, returning on 'L' command
    my_printf("Menu (A)dd, (R)emove, (P)rint, (L)ogout\n> ");
    if (!readLine(0, input_buffer, sizeof(input_buffer))) {
        my_printf("Error reading input, logging out.\n");
        break; // Exit menu on read error
    }

    if (input_buffer[0] == 'R') {
      my_printf("Remove PW Num?\n> ");
      if (!readLine(0, input_buffer, sizeof(input_buffer))) { break; }
      val1 = strToUint32(input_buffer);
      rmPW(userId, val1);
    } else if (input_buffer[0] == 'P') {
      printPW(userId);
    } else if (input_buffer[0] == 'A') {
      my_printf("First Secret\n> ");
      if (!readLine(0, input_buffer, sizeof(input_buffer))) { break; }
      val1 = strToUint32(input_buffer);
      my_printf("Second Secret\n> ");
      if (!readLine(0, input_buffer, sizeof(input_buffer))) { break; }
      val2 = strToUint32(input_buffer);
      my_printf("Third Secret\n> ");
      if (!readLine(0, input_buffer, sizeof(input_buffer))) { break; }
      val3 = strToUint32(input_buffer);
      addPW(userId, val1, val2, val3);
    } else if (input_buffer[0] == 'L') {
      return; // Exit the menu loop
    } else {
      my_printf("Bad command\n");
    }
  }
}

// Function: checkLogin
// Checks if the provided secrets satisfy a specific Diophantine equation.
// Equation: secret1^3 + secret2^3 + secret3^3 == userPassword^3
bool checkLogin(unsigned int *userPassword, unsigned int *secret1, unsigned int *secret2, unsigned int *secret3) {
  int cmp_result;
  
  // Check ordering: secret1 <= secret2 <= secret3 <= userPassword
  cmp_result = largeIntCmp(secret1, secret2);
  if (cmp_result > 0) return false; // secret1 > secret2
  cmp_result = largeIntCmp(secret2, secret3);
  if (cmp_result > 0) return false; // secret2 > secret3
  cmp_result = largeIntCmp(secret3, userPassword);
  if (cmp_result > 0) return false; // secret3 > userPassword

  // Calculate sum of cubes: secret1^3 + secret2^3 + secret3^3
  unsigned int sum_of_cubes[LARGE_INT_SIZE];
  unsigned int temp_cube_val[LARGE_INT_SIZE]; // Reused for each cube calculation

  // Initialize sum_of_cubes to 0
  for (int i = 0; i < LARGE_INT_SIZE; i++) {
    sum_of_cubes[i] = 0;
  }
  
  // Calculate secret1^3 and add to sum_of_cubes
  largeIntMul(temp_cube_val, secret1, secret1); // temp_cube_val = secret1^2
  largeIntMul(temp_cube_val, temp_cube_val, secret1); // temp_cube_val = secret1^3
  largeIntAdd(sum_of_cubes, sum_of_cubes, temp_cube_val); // sum_of_cubes += secret1^3

  // Calculate secret2^3 and add to sum_of_cubes
  largeIntMul(temp_cube_val, secret2, secret2); // temp_cube_val = secret2^2
  largeIntMul(temp_cube_val, temp_cube_val, secret2); // temp_cube_val = secret2^3
  largeIntAdd(sum_of_cubes, sum_of_cubes, temp_cube_val); // sum_of_cubes += secret2^3

  // Calculate secret3^3 and add to sum_of_cubes
  largeIntMul(temp_cube_val, secret3, secret3); // temp_cube_val = secret3^2
  largeIntMul(temp_cube_val, temp_cube_val, secret3); // temp_cube_val = secret3^3
  largeIntAdd(sum_of_cubes, sum_of_cubes, temp_cube_val); // sum_of_cubes += secret3^3

  // Calculate userPassword^3
  unsigned int user_pw_cube[LARGE_INT_SIZE];
  largeIntMul(user_pw_cube, userPassword, userPassword); // user_pw_cube = userPassword^2
  largeIntMul(user_pw_cube, user_pw_cube, userPassword); // user_pw_cube = userPassword^3

  // Compare sum_of_cubes with userPassword^3
  cmp_result = largeIntCmp(user_pw_cube, sum_of_cubes);
  return cmp_result == 0; // Return true if they are equal
}

// Function: main
// Main entry point of the program.
int main(void) {
  char input_buffer[64];
  unsigned int user_id_input;
  unsigned int secret1_input;
  unsigned int secret2_input;
  unsigned int secret3_input;

  // Large integer arrays for login check
  unsigned int user_pw_large[LARGE_INT_SIZE];
  unsigned int secret1_large[LARGE_INT_SIZE];
  unsigned int secret2_large[LARGE_INT_SIZE];
  unsigned int secret3_large[LARGE_INT_SIZE];
  
  initTable();
  initPasswords();
  
  // Initialize some default user passwords for testing purposes
  // For the checkLogin equation to pass, we need specific values.
  // Example: 3^3+4^3+5^3 = 27+64+125 = 216. 6^3 = 216.
  // So, if userPassword = 6, secret1=3, secret2=4, secret3=5 should work.
  // Let's set user 1's password to 6.
  userToPassword[1] = 6; 

  while (true) { // Main program loop
    my_printf("\nWelcome to the Diophantine Password Wallet\n");
    my_printf("Login\n");
    
    my_printf("User ID (0-%d)\n> ", USER_COUNT - 1);
    if (!readLine(0, input_buffer, sizeof(input_buffer))) { return 0; }
    user_id_input = strToUint32(input_buffer);
    
    my_printf("First Secret\n> ");
    if (!readLine(0, input_buffer, sizeof(input_buffer))) { return 0; }
    secret1_input = strToUint32(input_buffer);
    
    my_printf("Second Secret\n> ");
    if (!readLine(0, input_buffer, sizeof(input_buffer))) { return 0; }
    secret2_input = strToUint33(input_buffer);
    
    my_printf("Third Secret\n> ");
    if (!readLine(0, input_buffer, sizeof(input_buffer))) { return 0; }
    secret3_input = strToUint32(input_buffer);
    
    if (user_id_input >= USER_COUNT) {
      my_printf("User Not Found\n");
      return 0; // Exit program on invalid user ID
    }
    
    // Convert single ints to largeInt format for calculations
    intToLargeInt(user_pw_large, userToPassword[user_id_input]);
    intToLargeInt(secret1_large, secret1_input);
    intToLargeInt(secret2_large, secret2_input);
    intToLargeInt(secret3_large, secret3_input);
    
    // Check login credentials
    if (checkLogin(user_pw_large, secret1_large, secret2_large, secret3_large)) {
      my_printf("Login successful!\n");
      // Login successful, call the appropriate handler function based on secrets
      // The index is calculated as secret3 + secret2 * 50 + secret1 * 2500
      unsigned int handler_idx = secret3_input + secret2_input * HANDLER_TABLE_DIM + secret1_input * (HANDLER_TABLE_DIM * HANDLER_TABLE_DIM);
      
      // Check if the calculated index is within bounds and has a valid function pointer
      if (handler_idx < HANDLER_TABLE_TOTAL_SIZE && handlerTable[handler_idx] != NULL) {
          handlerTable[handler_idx](); // Call the user menu for the given secrets
      } else {
          my_printf("No specific menu found for these secrets. Logging out.\n");
      }
      // After user menu, loop back to login screen
    } else {
      my_printf("Login failed\n");
      // Loop back to login screen
    }
  }
  
  return 0; // Should be unreachable in an infinite loop unless readLine fails in main
}