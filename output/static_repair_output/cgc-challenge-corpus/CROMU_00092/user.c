#include <stdio.h>    // For printf, puts, scanf, NULL
#include <string.h>   // For strcmp, strlen, memcpy, memset
#include <stdint.h>   // For uint8_t, uint32_t
#include <stdbool.h>  // For bool

// --- Type Definitions ---
// Original types: byte, uint, undefined, undefined4
typedef uint8_t byte;
typedef unsigned int uint; // Used for unsigned integer return/variables
typedef char undefined;    // Used for single byte character arrays
typedef int undefined4;    // Used for 4-byte integer return types

// --- Global Constants ---
#define MAX_USERS         128   // Max number of users, derived from 0x80 checks
#define USER_BLOCK_SIZE   134   // Size of each user's data block (0x86 bytes)
#define CODE1_OFFSET      0x60  // Offset for the first code within a user block
#define CODE1_LEN         5     // Expected length of the first code string (excluding null terminator)
#define CODE2_OFFSET      0x66  // Offset for the second code within a user block
#define CODE2_LEN         31    // Expected length of the second code string (excluding null terminator)
#define USERNAME_INPUT_MAX_LEN 31 // Max length for username input when deleting (0x20 - 1)

// --- Global Variables ---
// Users storage: each user entry is USER_BLOCK_SIZE (134) bytes
// The first part is assumed to be the username, followed by codes at specific offsets.
char Users[MAX_USERS][USER_BLOCK_SIZE];
int NumUsers = 0; // Current count of active users

// --- Mock/Helper Functions ---
/**
 * @brief Simulates reading 'size' bytes into 'buffer' from stdin.
 *        Reads up to size-1 characters to leave space for a null terminator.
 * @param buffer Pointer to the buffer to store the read data.
 * @param size Maximum number of bytes to read (including null terminator).
 * @return The number of characters successfully read (excluding null terminator),
 *         or 0 if an error occurred or no characters were read.
 */
size_t ReadBytes(char* buffer, size_t size) {
    if (size == 0 || buffer == NULL) {
        return 0;
    }
    memset(buffer, 0, size); // Clear buffer and ensure null termination space

    if (fgets(buffer, size, stdin) == NULL) {
        return 0; // Error reading input
    }

    // Remove trailing newline character if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    }
    return len; // Return actual length of string read
}

/**
 * @brief Simulates revoking access for a user.
 *        In a real system, this would involve more complex actions.
 * @param userId The ID of the user whose access is to be revoked.
 */
void RevokeAccess(byte userId) {
    printf("Revoking access for user ID: %u\n", userId);
}

// --- Original Functions (Fixed) ---

/**
 * @brief Finds a username in the Users array.
 * @param param_1 The username string to search for.
 * @return The index of the user if found, or MAX_USERS if not found.
 */
byte FindUsername(char *param_1) {
  byte user_idx = 0;
  // Loop while within bounds and the current user's username doesn't match param_1
  while (user_idx < MAX_USERS && strcmp(Users[user_idx], param_1) != 0) {
    user_idx++;
  }
  return user_idx; // Returns MAX_USERS if not found, or index if found
}

/**
 * @brief Finds if a given code string exists in any user's code slots.
 * @param param_1 The code string to search for.
 * @return 1 if the code is found in either code slot of any user, 0 otherwise.
 */
undefined4 FindCode(char *param_1) {
  byte user_idx = 0;
  while (user_idx < MAX_USERS) {
    // Check first code slot (at CODE1_OFFSET)
    if (strcmp(Users[user_idx] + CODE1_OFFSET, param_1) == 0) {
      return 1;
    }
    // Check second code slot (at CODE2_OFFSET)
    if (strcmp(Users[user_idx] + CODE2_OFFSET, param_1) == 0) {
      return 1;
    }
    user_idx++;
  }
  return 0; // Code not found in any user entry
}

/**
 * @brief Finds the first available (empty) user slot.
 *        An empty slot is identified by the first character of its username being '\0'.
 * @return The index of an available user slot, or 0xffffffff if no slot is available.
 */
uint FindAvailableUser(void) {
  byte user_idx = 0;
  // Loop while within bounds and the current user slot is taken (username starts with non-null)
  while (user_idx < MAX_USERS && Users[user_idx][0] != '\0') {
    user_idx++;
  }
  if (user_idx == MAX_USERS) {
    return 0xffffffff; // No available user slot
  }
  return user_idx; // Return the index of the available slot
}

/**
 * @brief Attempts to add a new user to the system.
 *        Reads user data, validates it, and adds the user if all checks pass.
 * @return 1 on successful addition, 0 on failure.
 */
undefined4 AddUser(void) {
  char newUserBlock[USER_BLOCK_SIZE]; // Buffer to temporarily hold new user data
  
  printf("Enter new user data (username, codes, etc. up to %d chars):\n", USER_BLOCK_SIZE - 1);
  if (ReadBytes(newUserBlock, USER_BLOCK_SIZE) == 0) {
    puts("Failed to read user data.");
    return 0;
  }

  if (NumUsers >= MAX_USERS) {
    puts("Maximum number of users reached.");
    return 0;
  }

  uint availableUserIdx = FindAvailableUser();
  if (availableUserIdx == 0xffffffff) { 
    puts("No available user slot found."); // This should ideally be caught by NumUsers >= MAX_USERS
    return 0;
  }

  // Check if the username to be added already exists
  if (FindUsername(newUserBlock) != MAX_USERS) {
    puts("Username already exists.");
    return 0;
  }

  // Pointers to the code strings within the newUserBlock
  char* code1_ptr = newUserBlock + CODE1_OFFSET;
  char* code2_ptr = newUserBlock + CODE2_OFFSET;

  // Validate lengths of codes
  if (strlen(code1_ptr) != CODE1_LEN) {
    printf("Invalid length for code 1. Expected %d, got %zu.\n", CODE1_LEN, strlen(code1_ptr));
    return 0;
  }
  if (strlen(code2_ptr) != CODE2_LEN) {
    printf("Invalid length for code 2. Expected %d, got %zu.\n", CODE2_LEN, strlen(code2_ptr));
    return 0;
  }

  // Check if codes are already in use by other users
  if (FindCode(code1_ptr) != 0) {
    puts("Code 1 is already in use by another user.");
    return 0;
  }
  if (FindCode(code2_ptr) != 0) {
    puts("Code 2 is already in use by another user.");
    return 0;
  }

  // If all checks pass, add the user
  memcpy(Users[availableUserIdx], newUserBlock, USER_BLOCK_SIZE);
  NumUsers++;
  puts("User added successfully.");
  return 1;
}

/**
 * @brief Deletes an existing user from the system.
 *        Reads a username, finds the user, revokes access, and clears their data.
 * @return 1 on successful deletion, 0 on failure.
 */
undefined4 DelUser(void) {
  char username_to_delete[USERNAME_INPUT_MAX_LEN + 1]; // Buffer for username input (+1 for null terminator)
  memset(username_to_delete, 0, sizeof(username_to_delete));
  
  printf("Enter username to delete (max %d chars):\n", USERNAME_INPUT_MAX_LEN);
  if (ReadBytes(username_to_delete, sizeof(username_to_delete)) == 0) {
    puts("Failed to read username.");
    return 0;
  }

  byte user_idx = FindUsername(username_to_delete);
  if (user_idx == MAX_USERS) { // Username not found
    puts("Username not found.");
    return 0;
  }

  RevokeAccess(user_idx); // Call mock function for revoking access
  memset(Users[user_idx], 0, USER_BLOCK_SIZE); // Clear the user's data block
  NumUsers--;
  puts("User deleted successfully.");
  return 1;
}

// --- Main function for demonstration ---
int main() {
    // Initialize all user blocks to null (empty) at startup
    for (int i = 0; i < MAX_USERS; ++i) {
        memset(Users[i], 0, USER_BLOCK_SIZE);
    }
    NumUsers = 0; // Ensure user count is zero

    printf("--- User Management System Demo ---\n");

    // --- Test AddUser ---
    printf("\n--- Adding User 1 ---\n");
    // Expected input format for ReadBytes: "username<padding>code1<padding>code2<padding>"
    // Example input: user1AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAcode1code2AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    // (Ensure code1 is 5 chars, code2 is 31 chars, and total length is <= USER_BLOCK_SIZE-1)
    AddUser(); 

    printf("\n--- Adding User 2 ---\n");
    // Example input: user2BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBcodeAcodeBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
    AddUser(); 

    printf("\n--- Trying to add user with existing username (user1) ---\n");
    // Input for user1: user1...
    AddUser(); 

    printf("\n--- Trying to add user with existing code (code1) ---\n");
    // Input for user3: user3...code1...
    AddUser(); 

    printf("\n--- Current Users After Additions ---\n");
    for (int i = 0; i < MAX_USERS; ++i) {
        if (Users[i][0] != '\0') { // If the username field is not empty
            printf("User %d: Username: '%s', Code1: '%s', Code2: '%s'\n",
                   i, Users[i], Users[i] + CODE1_OFFSET, Users[i] + CODE2_OFFSET);
        }
    }
    printf("Total active users: %d\n", NumUsers);

    // --- Test DelUser ---
    printf("\n--- Deleting User 1 ---\n");
    // Input: user1
    DelUser(); 

    printf("\n--- Trying to delete a non-existent user ---\n");
    // Input: non_existent_user
    DelUser(); 

    printf("\n--- Current Users After Deletion ---\n");
    for (int i = 0; i < MAX_USERS; ++i) {
        if (Users[i][0] != '\0') { // If the username field is not empty
            printf("User %d: Username: '%s', Code1: '%s', Code2: '%s'\n",
                   i, Users[i], Users[i] + CODE1_OFFSET, Users[i] + CODE2_OFFSET);
        }
    }
    printf("Total active users: %d\n", NumUsers);

    return 0;
}