#include <stdio.h>   // For puts
#include <stdlib.h>  // For exit
#include <string.h>  // For memset
#include <stdint.h>  // For uint32_t (if needed, but simple int might suffice for this code)

// External data declarations (assuming these are global variables defined elsewhere)
extern uint32_t DAT_4347c000;
extern char DAT_00018047;

// Function prototypes (assuming their signatures based on usage)
char InitFilesystem(void);
char *FsError(void);
void seed_prng(void);
char random_in_range(char min_val, char max_val); // Assuming char return and arguments
char AddUser(char *user_data, char *some_other_data); // Assuming char* arguments
int ReceiveRequest(void);
int HandleRequest(int request_id);
void FreeRequest(int request_id);
void SendResponse(int response_id);
void FreeResponse(int response_id);

int main(void) {
  char cVar1;
  char uVar2;
  char *s_error_message;
  char local_2f[11];
  int local_24; // Corresponds to HandleRequest return value
  int local_20; // Corresponds to ReceiveRequest return value
  unsigned int local_14; // Loop counter

  cVar1 = InitFilesystem();
  if (cVar1 == '\0') {
    s_error_message = FsError();
    puts(s_error_message);
    exit(1); // Terminate with an error code
  }

  // The original code had:
  // *(undefined4 *)(puVar3 + -0x10) = *local_1c;
  // *(undefined4 *)(puVar3 + -0x14) = 0x12092;
  // local_1c was (undefined4 *)&DAT_4347c000;
  // These seem to be unused assignments or stack artifacts from disassembly. Removed.

  seed_prng();

  // The original code had:
  // *(undefined4 *)(puVar3 + -0xc) = 0xb;
  // *(undefined **)(puVar3 + -0x10) = local_2f;
  // bzero(*(void **)(puVar3 + -0x10),*(size_t *)(puVar3 + -0xc));
  // This translates to bzero(local_2f, 11). Using memset.
  memset(local_2f, 0, sizeof(local_2f));

  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    // The original code had:
    // *(undefined4 *)(puVar3 + -0xc) = 0x7a; // max_val
    // *(undefined4 *)(puVar3 + -0x10) = 0x41; // min_val
    // uVar2 = random_in_range();
    uVar2 = random_in_range(0x41, 0x7a); // ASCII 'A' to 'z'
    local_2f[local_14] = uVar2;
  }

  // The original code had:
  // *(undefined **)(puVar3 + -0xc) = local_2f;
  // *(undefined **)(puVar3 + -0x10) = &DAT_00018047;
  // cVar1 = AddUser();
  cVar1 = AddUser(local_2f, &DAT_00018047);
  if (cVar1 == '\0') {
    // The original code had:
    // *(undefined4 *)(puVar3 + -0x10) = 0;
    exit(1); // Terminate with an error code
  }

  do {
    do {
      local_20 = ReceiveRequest();
    } while (local_20 == 0); // Keep receiving until a non-zero request is received

    // The original code had: *(int *)(puVar4 + -0x10) = local_20; before HandleRequest
    local_24 = HandleRequest(local_20);

    if (local_24 == 0) {
      // The original code had: *(int *)(puVar4 + -0x10) = local_20; before FreeRequest
      FreeRequest(local_20);
    } else {
      // The original code had: *(int *)(puVar4 + -0x10) = local_24; before SendResponse
      SendResponse(local_24);
      // The original code had: *(int *)(puVar4 + -0x10) = local_20; before FreeRequest
      FreeRequest(local_20);
      // The original code had: *(int *)(puVar4 + -0x10) = local_24; before FreeResponse
      FreeResponse(local_24);
    }
  } while (1); // Loop indefinitely

  return 0; // Unreachable due to do-while(1)
}