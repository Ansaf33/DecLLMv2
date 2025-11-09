#include <stdint.h> // For uint32_t, uint8_t, uintptr_t
#include <stdlib.h> // For malloc, free, size_t
#include <string.h> // For memcpy

// Define custom types if they are not standard
typedef uint32_t undefined4; // Assuming undefined4 is a 4-byte unsigned integer
typedef uint8_t byte;        // Assuming byte is an 8-bit unsigned integer

// Global variables (assuming they are global based on usage)
int ERRNO = 0;
void* last = NULL; // Initialized to NULL
uint32_t* ACCOUNTS = NULL; // Initialized to NULL (points to a uint32_t, which is a list head)

// Forward declarations for functions not provided or defined later
// These signatures are inferred from usage and adjusted for type correctness.
uint32_t get_head(uint32_t list_head_val); // list_head_val is typically a pointer stored as an int
uint32_t register_transaction(void);
uint32_t init_ll(void); // Returns a uint32_t which is stored as list head
int add_node(uint32_t list_head_val, int node_ptr); // node_ptr is likely a pointer stored as an int
void remove_node(uint32_t list_head_val, int node_ptr);

// Forward declarations for functions defined in this snippet but potentially called before definition
void init_account_register(void);
void* alloc_txn_head(void);
int vendor_calc(int param_1);
uint32_t packet_sz_by_op_code(uint32_t op_code, int param_2);
int get_log_data_sz(uint32_t param_1, int param_2);
uint32_t get_ops_data_sz(int param_1, uint32_t unused_param_2, uint32_t param_3);
int get_size_of_data_for_pkt(int param_1, int *param_2, uint32_t param_3);
void get_size_of_data_for_pkt_head_only(int param_1, uint32_t param_2);
int get_transaction(int param_1, int param_2);
uint32_t finalize_transaction(char *param_1);
void* open_txn(int param_1, void *param_2);
void* remove_transaction_log_entry(int param_1, int param_2);
int add_transaction_log_entry(uint32_t param_1, void **param_2);
uint32_t* init_ll_heap(void);
int create_account(uint32_t param_1, uint32_t param_2, uint32_t *param_3);
int * get_account(int param_1, int param_2);


// Function: transaction_iterator
uint32_t transaction_iterator(void* param_1) { // Assuming param_1 is a pointer to a structure
  if (param_1 == NULL) {
    if (last == NULL) {
      return 0;
    }
    last = *(void**)((char*)last + 4);
    if (last == NULL) {
      return 0;
    }
    return *(uint32_t*)((char*)last + 8);
  }
  
  last = (void*)(uintptr_t)get_head(*(uint32_t*)((char*)param_1 + 0x10));
  return *(uint32_t*)((char*)last + 8);
}

// Function: lazy_account_init
void lazy_account_init(void) {
  if (ACCOUNTS == NULL) {
    init_account_register();
  }
}

// Function: alloc_txn_head
void* alloc_txn_head(void) {
  return malloc(0xc);
}

// Function: vendor_calc
int vendor_calc(int param_1) { // param_1 is an address stored as an int
  return *(byte*)((char*)param_1 + 5) - 4;
}

// Function: packet_sz_by_op_code
uint32_t packet_sz_by_op_code(uint32_t param_1, int param_2) {
  switch(param_1) {
    case 0: return (param_2 == 0) ? 0 : 0xe;
    case 1: return (param_2 == 0) ? 0 : 10;
    case 2: return (param_2 == 1) ? 0 : 4;
    case 3: return 4;
    case 4: return (param_2 == 0) ? 0 : 4;
    case 5: return (param_2 == 0) ? 0 : 8;
    default: return 0;
  }
}

// Function: get_log_data_sz
int get_log_data_sz(uint32_t param_1, int param_2) { // param_2 is an address stored as an int
  switch(param_1) {
    case 0: return vendor_calc((char*)param_2 + 8) + packet_sz_by_op_code(param_1, 1);
    case 1: return vendor_calc((char*)param_2 + 4) + packet_sz_by_op_code(param_1, 1);
    case 2: return 4;
    case 3: return 4;
    case 4: return 4;
    case 5: return 8;
    default: return 0;
  }
}

// Function: get_ops_data_sz
// Corrected to return the value of packet_sz_by_op_code as implied by its usage
uint32_t get_ops_data_sz(int param_1, uint32_t unused_param_2, uint32_t param_3) { // param_1 is an address
  return packet_sz_by_op_code(*(uint8_t*)((char*)param_1 + 9), param_3);
}

// Function: get_size_of_data_for_pkt
int get_size_of_data_for_pkt(int param_1, int *param_2, uint32_t param_3) { // param_1 is an address
  if (*(char*)((char*)param_1 + 10) == '\0') {
    byte bVar1_val = *(byte*)((char*)param_1 + 8);
    if (bVar1_val == 3) {
      return 0;
    }
    if (bVar1_val < 4) {
      if (bVar1_val == 2) {
        return (int)get_ops_data_sz(param_1, (uint32_t)(uintptr_t)param_2, param_3);
      }
      if (bVar1_val < 3) {
        if (bVar1_val == 0) {
          if ((*(char*)((char*)param_1 + 9) == 0x04) && (*(int*)((char*)param_1 + 0xc) == 0)) {
            return 4;
          }
          return 0;
        }
        if (bVar1_val == 1) {
          return 0;
        }
      }
    }
    return 0;
  }
  return *param_2 + 4;
}

// Function: get_size_of_data_for_pkt_head_only
void get_size_of_data_for_pkt_head_only(int param_1, uint32_t param_2) { // param_1 is an address
  get_size_of_data_for_pkt(param_1, (int*)((char*)param_1 + 0x10), param_2);
}

// Function: get_transaction
int get_transaction(int param_1, int param_2) { // param_1 is an address
  int current_node = (int)(uintptr_t)get_head(*(uint32_t*)((char*)param_1 + 0x10));
  
  while(current_node != 0) {
    if (param_2 == *(int*)(*(int*)((char*)current_node + 8) + 4)) {
      return *(int*)((char*)current_node + 8);
    }
    current_node = *(int*)((char*)current_node + 4);
  }
  
  ERRNO = 0xf;
  return 0;
}

// Function: finalize_transaction
uint32_t finalize_transaction(char *param_1) {
  *param_1 = *param_1 - 1;
  if (*param_1 < 2) {
    *(uint32_t*)((char*)param_1 + 0x10) = 3;
    return 0;
  }
  ERRNO = 0xff;
  return 0xf;
}

// Function: open_txn
void * open_txn(int param_1, void *param_2) { // param_1 is an address
  void* head_node = alloc_txn_head();
  if (head_node == NULL) return NULL;
  
  void *txn_data = malloc(0x18);
  if (txn_data == NULL) {
    free(head_node);
    return NULL;
  }
  
  *(void **)((char*)head_node + 8) = txn_data;
  
  *(uint8_t*)txn_data = 1;
  uint32_t transaction_id = register_transaction();
  *(uint32_t*)((char*)txn_data + 4) = transaction_id;
  
  *(uint32_t*)((char*)param_2 + 0xc) = transaction_id;
  
  void *extra_data = malloc(0x10);
  if (extra_data == NULL) {
    // Original code doesn't free previous allocations here.
    return NULL;
  }
  *(void**)((char*)txn_data + 0xc) = extra_data;
  
  memcpy(extra_data, param_2, 0x10);
  add_node(*(uint32_t*)((char*)param_1 + 0x10), (int)(uintptr_t)head_node);
  
  return txn_data;
}

// Function: remove_transaction_log_entry
void * remove_transaction_log_entry(int param_1, int param_2) { // param_1, param_2 are addresses
  int current_node = (int)(uintptr_t)get_head(*(uint32_t*)((char*)param_1 + 0x10));
  void *__ptr = NULL;
  
  while(current_node != 0) {
    __ptr = *(void**)((char*)current_node + 8);
    if (__ptr && *(int*)((char*)__ptr + 4) == *(int*)((char*)param_2 + 4)) {
      break;
    }
    current_node = *(int*)((char*)current_node + 4);
  }
  
  if (current_node == 0) {
    ERRNO = 0xf;
    return NULL;
  }

  if (*(void**)((char*)__ptr + 0xc) != NULL) {
    free(*(void**)((char*)__ptr + 0xc));
  }
  // Corrected logical bug: original checked offset 8 but freed offset 0x14
  if (*(void**)((char*)__ptr + 0x14) != NULL) {
    free(*(void**)((char*)__ptr + 0x14));
  }
  free(__ptr);
  remove_node(*(uint32_t*)((char*)param_1 + 0x10), current_node);
  return __ptr;
}

// Function: add_transaction_log_entry
int add_transaction_log_entry(uint32_t param_1, void **param_2) {
  void *pvVar1 = *param_2;
  void *__src = param_2[2];
  
  int iVar2 = get_transaction(param_1, *(uint32_t*)((char*)pvVar1 + 0xc));
  if (iVar2 == 0) {
    return 0;
  }
  
  if (*(void**)((char*)iVar2 + 0xc) == NULL) {
    void *pvVar3_new = malloc(0x10);
    if (pvVar3_new == NULL) return 0;
    *(void**)((char*)iVar2 + 0xc) = pvVar3_new;
  }
  
  memcpy(*(void**)((char*)iVar2 + 0xc), pvVar1, 0x10);
  
  void *pvVar1_size = param_2[1]; // Assuming param_2[1] holds a size_t value
  *(void**)((char*)iVar2 + 8) = pvVar1_size;
  
  if (pvVar1_size != NULL) { // Check if size is non-zero
    void *pvVar3_data = malloc((size_t)pvVar1_size);
    if (pvVar3_data == NULL) return 0;
    *(void**)((char*)iVar2 + 0x14) = pvVar3_data;
    
    if (__src == NULL) {
      return 0;
    }
    memcpy(pvVar3_data, __src, (size_t)pvVar1_size);
  }
  return iVar2;
}

// Function: init_ll_heap
uint32_t * init_ll_heap(void) {
  uint32_t *puVar1 = (uint32_t *)malloc(8);
  if (puVar1 == NULL) {
    return NULL;
  }
  *puVar1 = 0;
  puVar1[1] = 0;
  return puVar1;
}

// Function: create_account
int create_account(uint32_t param_1, uint32_t param_2, uint32_t *param_3) {
  uint32_t uVar4 = *param_3;
  lazy_account_init();
  
  void *pvVar1 = malloc(0xc);
  uint32_t *puVar2 = (uint32_t *)malloc(0x14);
  
  if ((puVar2 == NULL) || (pvVar1 == NULL)) {
    free(pvVar1);
    free(puVar2);
    return 3;
  }
  
  *puVar2 = param_1;
  puVar2[1] = param_2;
  puVar2[2] = uVar4;
  puVar2[3] = uVar4;
  
  uint32_t list_head = init_ll_heap();
  puVar2[4] = list_head;
  
  if (puVar2[4] == 0) {
    free(pvVar1);
    free(puVar2);
    return 3;
  }
  
  *(uint32_t **)((char*)pvVar1 + 8) = puVar2;
  int add_result = add_node(*ACCOUNTS, (int)(uintptr_t)pvVar1);
  
  return add_result; // Returns 0 for success, non-zero for failure (based on original logic)
}

// Function: get_account
int * get_account(int param_1, int param_2) {
  lazy_account_init();
  
  int current_node = (int)(uintptr_t)get_head(*ACCOUNTS);
  
  if (current_node == 0) {
    ERRNO = 0xf;
    return NULL;
  }
  
  int *local_14 = *(int**)((char*)current_node + 8);
  while (local_14 != NULL) {
    if (param_1 == *local_14) {
      if (param_2 != local_14[1]) {
        ERRNO = 0xc;
        return NULL;
      }
      return local_14;
    }
    current_node = *(int*)((char*)current_node + 4);
    if (current_node == 0) break;
    local_14 = *(int**)((char*)current_node + 8);
  }
  
  ERRNO = 0xf;
  return NULL;
}

// Function: init_account_register
void init_account_register(void) {
  uint32_t *puVar1 = (uint32_t *)malloc(8);
  if (puVar1 == NULL) {
    ACCOUNTS = NULL; // Indicate failure
    return;
  }
  uint32_t uVar2 = init_ll();
  *puVar1 = uVar2;
  ACCOUNTS = puVar1;
}