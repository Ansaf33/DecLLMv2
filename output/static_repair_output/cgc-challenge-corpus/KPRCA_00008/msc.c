#include <stdint.h>
#include <string.h>

// Define types based on Ghidra's output
typedef uint8_t undefined;
typedef uint8_t undefined1;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint8_t byte;
typedef uint32_t uint;
typedef uint16_t ushort;

// Function pointer type for callback functions
typedef void (*callback_func_t)(void*, uint32_t);

// Dummy declarations for external functions
void usb_send_reply(void* param_1, void* param_2, uint32_t param_3, uint32_t param_4);
uint32_t rbc_handle_packet(void* param_1, byte* param_2_ptr, uint8_t param_3_byte);
uint32_t rbc_handle_data(void* param_1, void* param_2_src, uint32_t param_3_len, uint32_t param_4_unused);

// Global variables (dummy definitions)
uint8_t memory[0x10000]; // Example size for memory block
uint8_t inquiry_data[0x24]; // Example size for inquiry_data

// Function: msc_init
uint32_t msc_init(void *param_1) {
  memset(param_1, 0, 0x1050);
  *(uint32_t *)((uintptr_t)param_1 + 0x2c) = 0;
  *(uint8_t **)((uintptr_t)param_1 + 0x38) = memory;
  return 1;
}

// Function: msc_handle_ep0
uint32_t msc_handle_ep0(void* param_1, void* param_2) {
  uint8_t local_d[5];
  
  if (*(int8_t *)((uintptr_t)param_2 + 0x1c) == (int8_t)-0x5f) {
    if (*(int8_t *)((uintptr_t)param_2 + 0x1d) == (int8_t)-2) {
      local_d[0] = 1;
      usb_send_reply(param_1, param_2, 0, 1);
      ((callback_func_t*)((uintptr_t)param_1 + 4))[0](local_d, 1);
      return 1;
    }
  }
  else if ((*(int8_t *)((uintptr_t)param_2 + 0x1c) == '!') && (*(int8_t *)((uintptr_t)param_2 + 0x1d) == (int8_t)-1)) {
    usb_send_reply(param_1, param_2, 0, 0);
    return 1;
  }
  return 0;
}

// Function: msc_handle_urb
uint32_t msc_handle_urb(void* param_1, void* param_2) {
  uint32_t* piVar1;
  uint32_t iVar2;

  piVar1 = (uint32_t*)((uintptr_t)param_1 + 0xc);
  if (*(int32_t *)((uintptr_t)param_1 + 0x38) == 0) {
    if (*(int32_t *)((uintptr_t)param_2 + 8) != 0) {
      return 0;
    }
    if (*(uint32_t *)((uintptr_t)param_2 + 0x14) < 0x1f) {
      return 0;
    }
    if (*(uint32_t *)((uintptr_t)param_2 + 0x30) != 0x43425355) {
      return 0;
    }
    *piVar1 = *(uint32_t *)((uintptr_t)param_2 + 0x30);
    *(uint32_t *)((uintptr_t)param_1 + 0x10) = *(uint32_t *)((uintptr_t)param_2 + 0x34);
    *(uint32_t *)((uintptr_t)param_1 + 0x14) = *(uint32_t *)((uintptr_t)param_2 + 0x38);
    *(uint32_t *)((uintptr_t)param_1 + 0x18) = *(uint32_t *)((uintptr_t)param_2 + 0x3c);
    *(uint32_t *)((uintptr_t)param_1 + 0x1c) = *(uint32_t *)((uintptr_t)param_2 + 0x40);
    *(uint32_t *)((uintptr_t)param_1 + 0x20) = *(uint32_t *)((uintptr_t)param_2 + 0x44);
    *(uint32_t *)((uintptr_t)param_1 + 0x24) = *(uint32_t *)((uintptr_t)param_2 + 0x48);
    *(uint32_t *)((uintptr_t)param_1 + 0x27) = *(uint32_t *)((uintptr_t)param_2 + 0x4b);
    *(uint32_t *)((uintptr_t)param_1 + 0x40) = 0;
    iVar2 = rbc_handle_packet(piVar1, (byte*)((uintptr_t)param_2 + 0x3f), *(uint8_t*)((uintptr_t)param_2 + 0x3e));
    *(uint32_t *)((uintptr_t)param_1 + 0x3c) = (uint32_t)(iVar2 == 0);
    usb_send_reply(param_1, param_2, 0, *(uint32_t *)((uintptr_t)param_2 + 0x14));
  }
  else if (*(int32_t *)((uintptr_t)param_1 + 0x38) == 1) {
    if (*(int32_t *)((uintptr_t)param_2 + 8) != 1) {
      return 0;
    }
    uint32_t len_to_send = *(uint32_t *)((uintptr_t)param_2 + 0x14);
    if (*(uint32_t *)((uintptr_t)param_1 + 0x48) < len_to_send) {
      len_to_send = *(uint32_t *)((uintptr_t)param_1 + 0x48);
    }
    usb_send_reply(param_1, param_2, 0, len_to_send);
    ((callback_func_t*)((uintptr_t)param_1 + 4))[0](
        (void*)((uintptr_t)*(int32_t *)((uintptr_t)param_1 + 0x40) + *(int32_t *)((uintptr_t)param_1 + 0x4c)), len_to_send);
    *(uint32_t *)((uintptr_t)param_1 + 0x40) = *(int32_t *)((uintptr_t)param_1 + 0x40) + len_to_send;
    *(uint32_t *)((uintptr_t)param_1 + 0x48) = *(int32_t *)((uintptr_t)param_1 + 0x48) - len_to_send;
    if (*(int32_t *)((uintptr_t)param_1 + 0x48) == 0) {
      *(uint32_t *)((uintptr_t)param_1 + 0x38) = 3;
    }
  }
  else if (*(int32_t *)((uintptr_t)param_1 + 0x38) == 2) {
    if (*(int32_t *)((uintptr_t)param_2 + 8) != 0) {
      return 0;
    }
    iVar2 = rbc_handle_data(piVar1, (void*)((uintptr_t)param_2 + 0x30), *(uint32_t *)((uintptr_t)param_2 + 0x14), 0x114f5);
    if (iVar2 == 0) {
      *(uint32_t *)((uintptr_t)param_1 + 0x3c) = 1;
      *(uint32_t *)((uintptr_t)param_1 + 0x38) = 3;
      usb_send_reply(param_1, param_2, 0, 0);
    }
    else {
      usb_send_reply(param_1, param_2, 0, *(uint32_t *)((uintptr_t)param_2 + 0x14));
    }
  }
  else if (*(int32_t *)((uintptr_t)param_1 + 0x38) == 3) {
    if (*(int32_t *)((uintptr_t)param_2 + 8) != 1) {
      return 0;
    }
    if (*(uint32_t *)((uintptr_t)param_2 + 0x14) < 0xd) {
      return 0;
    }
    *(uint32_t *)((uintptr_t)param_1 + 0x2b) = 0x53425355;
    *(uint32_t *)((uintptr_t)param_1 + 0x2f) = *(uint32_t *)((uintptr_t)param_1 + 0x10);
    *(int32_t *)((uintptr_t)param_1 + 0x33) = *(int32_t *)((uintptr_t)param_1 + 0x14) - *(int32_t *)((uintptr_t)param_1 + 0x40);
    *(int8_t *)((uintptr_t)param_1 + 0x37) = (int8_t)*(uint32_t *)((uintptr_t)param_1 + 0x3c);
    usb_send_reply(param_1, param_2, 0, 0xd);
    ((callback_func_t*)((uintptr_t)param_1 + 4))[0]((void*)((uintptr_t)param_1 + 0x2b), 0xd);
    *(uint32_t *)((uintptr_t)param_1 + 0x38) = 0;
  }
  return 1;
}

// Function: msc_send
void msc_send(void* param_1, void* param_2_ptr, uint32_t param_3, uint32_t param_4) {
  *(uint32_t *)((uintptr_t)param_1 + 0x44) = param_4;
  *(uint32_t *)((uintptr_t)param_1 + 0x3c) = param_3;
  *(void **)((uintptr_t)param_1 + 0x40) = param_2_ptr;
  *(uint32_t *)((uintptr_t)param_1 + 0x2c) = 1;
  if (*(uint32_t *)((uintptr_t)param_1 + 8) < *(uint32_t *)((uintptr_t)param_1 + 0x3c)) {
    *(uint32_t *)((uintptr_t)param_1 + 0x3c) = *(uint32_t *)((uintptr_t)param_1 + 8);
  }
  return;
}

// Function: rbc_handle_packet
uint32_t rbc_handle_packet(void* param_1, byte *param_2_ptr, uint8_t param_3_byte) {
  uint16_t uVar2;
  uint32_t* __s;
  uint32_t uVar3;
  uint32_t uVar4;
  
  __s = (uint32_t*)((uintptr_t)param_1 + 0x48);
  *(uint32_t *)((uintptr_t)param_1 + 0x2c) = 3;
  *(uint32_t *)((uintptr_t)param_1 + 0x3c) = 0;
  *(void **)((uintptr_t)param_1 + 0x40) = 0;
  *(uint32_t *)((uintptr_t)param_1 + 0x44) = 0;
  
  if (param_3_byte == 0x5a) {
    if ((param_2_ptr[2] != 0x3f) && (param_2_ptr[2] != 6)) {
      return 1;
    }
    memset(__s, 0, 0x12);
    *(uint16_t *)__s = 0x1000;
    *(uint8_t *)((uintptr_t)param_1 + 0x50) = 0x86;
    *(uint8_t *)((uintptr_t)param_1 + 0x51) = 8;
    *(uint8_t *)((uintptr_t)param_1 + 0x52) = 1;
    *(uint16_t *)((uintptr_t)param_1 + 0x53) = 2;
    *(uint8_t *)((uintptr_t)param_1 + 0x55) = 0;
    *(uint32_t *)((uintptr_t)param_1 + 0x56) = 0x80000000;
    msc_send(param_1, __s, 0x12, 0);
    return 1;
  }
  
  if (param_3_byte < 0x5b) {
    if (param_3_byte == 0x2a) {
      uVar3 = __builtin_bswap32(*(uint32_t*)(param_2_ptr + 2));
      uVar2 = __builtin_bswap16(*(uint16_t*)(param_2_ptr + 7));
      
      if (0x80 < uVar3) {
        return 0;
      }
      if (uVar3 + uVar2 < 0x81) {
        *(uint32_t *)((uintptr_t)param_1 + 0x2c) = 2;
        *(uint32_t *)((uintptr_t)param_1 + 0x1048) = uVar3;
        *(uint32_t *)((uintptr_t)param_1 + 0x104c) = (uint32_t)uVar2 << 9;
        return 1;
      }
      return 0;
    }
    if (param_3_byte < 0x2b) {
      if (param_3_byte == 0x28) {
        uint32_t raw_val = *(uint32_t*)(param_2_ptr + 2);
        uVar3 = __builtin_bswap32(raw_val);
        uVar4 = uVar3 & 0x00FFFFFF; // LBA is often 24-bit
        uVar2 = __builtin_bswap16(*(uint16_t*)(param_2_ptr + 7));
        if (0x80 < uVar3) {
          return 0;
        }
        if (uVar3 + uVar2 < 0x81) {
          msc_send(param_1, (void*)((uintptr_t)*(uint8_t**)((uintptr_t)param_1 + 0x38) + (uintptr_t)uVar4 * 0x200), (uint32_t)uVar2 << 9, 0);
          return 1;
        }
        return 0;
      }
      if (param_3_byte < 0x29) {
        if (param_3_byte == 0x25) {
          *__s = 0x7f000000;
          *(uint32_t *)((uintptr_t)param_1 + 0x4c) = 0x20000;
          msc_send(param_1, __s, 8, 0);
          return 1;
        }
        if (param_3_byte < 0x26) {
          if (param_3_byte == 0) {
            *(uint32_t *)((uintptr_t)param_1 + 0x2c) = 3;
            return 1;
          }
          if (param_3_byte == 0x12) {
            uint8_t len = param_2_ptr[4];
            if (0x24 < len) {
              len = 0x24;
            }
            msc_send(param_1, inquiry_data, len, 0);
            return 1;
          }
        }
      }
    }
  }
  *(uint32_t *)((uintptr_t)param_1 + 0x30) = 1;
  msc_send(param_1, NULL, 0, 0);
  return 1;
}

// Function: rbc_handle_data
uint32_t rbc_handle_data(void* param_1, void* param_2_src, uint32_t param_3_len, uint32_t param_4_unused) {
  uint32_t ret_val = 0;
  
  if (*(int8_t *)((uintptr_t)param_1 + 0xf) == '*') {
    uint32_t current_data_offset = *(uint32_t *)((uintptr_t)param_1 + 0x34);
    uint32_t total_data_len = *(uint32_t *)((uintptr_t)param_1 + 0x104c);
    
    if (total_data_len < (current_data_offset + param_3_len)) {
      param_3_len = total_data_len - current_data_offset;
    }
    
    memcpy((void*)((uintptr_t)*(uint8_t**)((uintptr_t)param_1 + 0x38) + (uintptr_t)*(uint32_t *)((uintptr_t)param_1 + 0x1048) * 0x200 + current_data_offset), param_2_src, param_3_len);
    
    *(uint32_t *)((uintptr_t)param_1 + 0x34) = current_data_offset + param_3_len;
    
    if (*(uint32_t *)((uintptr_t)param_1 + 0x34) == total_data_len) {
      *(uint32_t *)((uintptr_t)param_1 + 0x2c) = 3;
    }
    ret_val = 1;
  }
  return ret_val;
}