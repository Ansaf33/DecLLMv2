// Function: lo
undefined8 lo(undefined8 param_1) {
  return param_1;
}

// Function: hi
undefined8 hi(undefined8 param_1,undefined8 param_2) {
  return param_2;
}

// Function: hiset
void hiset(long param_1,undefined8 param_2) {
  *(undefined8 *)(param_1 + 8) = param_2;
  return;
}

// Function: hi_is_set
undefined8 hi_is_set(long param_1) {
  return CONCAT71((int7)((ulong)*(long *)(param_1 + 8) >> 8),*(long *)(param_1 + 8) != 0);
}

// Function: uuset
void uuset(undefined8 *param_1,undefined8 *param_2,undefined8 param_3,undefined8 param_4) {
  undefined8 uVar1;
  
  uVar1 = hi(param_3,param_4);
  *param_1 = uVar1;
  uVar1 = lo(param_3,param_4);
  *param_2 = uVar1;
  return;
}

// Function: make_uuint
undefined  [16] make_uuint(undefined8 param_1,undefined8 param_2) {
  undefined auVar1 [16];
  
  auVar1._8_8_ = param_1;
  auVar1._0_8_ = param_2;
  return auVar1;
}

// Function: lt2
bool lt2(ulong param_1,ulong param_2,ulong param_3,ulong param_4) {
  long in_FS_OFFSET;
  
  if (*(long *)(in_FS_OFFSET + 0x28) != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return param_1 - param_3 < (ulong)(param_2 < param_4) || param_1 < param_3;
}

// Function: ge2
bool ge2(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4) {
  char cVar1;
  
  cVar1 = lt2(param_1,param_2,param_3,param_4);
  return cVar1 == '\0';
}

// Function: highbit_to_mask
long highbit_to_mask(long param_1) {
  return param_1 >> 0x3f;
}

// Function: mod2
void mod2(ulong param_1,ulong param_2,ulong param_3,ulong param_4) {
  long lVar1;
  char cVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  byte bVar6;
  bool bVar7;
  ulong local_48;
  ulong local_40;
  ulong local_38;
  ulong local_30;
  int iStack_28;
  
  if (param_3 == 0) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("d1 != 0",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/factor.c",
                  0x1c6,(char *)&__PRETTY_FUNCTION___6);
  }
  local_38 = param_2;
  local_30 = param_1;
  if (param_1 != 0) {
    if (param_3 == 0) {
      uVar3 = 0x40;
    }
    else {
      lVar1 = 0x3f;
      if (param_3 != 0) {
        for (; param_3 >> lVar1 == 0; lVar1 = lVar1 + -1) {
        }
      }
      uVar3 = (uint)lVar1 ^ 0x3f;
    }
    if (param_1 == 0) {
      uVar4 = 0x40;
    }
    else {
      lVar1 = 0x3f;
      if (param_1 != 0) {
        for (; param_1 >> lVar1 == 0; lVar1 = lVar1 + -1) {
        }
      }
      uVar4 = (uint)lVar1 ^ 0x3f;
    }
    iVar5 = uVar3 - uVar4;
    if (0 < iVar5) {
      bVar6 = (byte)iVar5;
      local_40 = param_4 >> (0x40 - bVar6 & 0x3f) | param_3 << (bVar6 & 0x3f);
      local_48 = param_4 << (bVar6 & 0x3f);
      for (iStack_28 = 0; iStack_28 < iVar5; iStack_28 = iStack_28 + 1) {
        cVar2 = ge2(local_30,local_38,local_40,local_48);
        if (cVar2 != '\0') {
          bVar7 = local_38 < local_48;
          local_38 = local_38 - local_48;
          local_30 = (local_30 - local_40) - (ulong)bVar7;
        }
        local_48 = local_48 >> 1 | local_40 << 0x3f;
        local_40 = local_40 >> 1;
      }
    }
  }
  make_uuint(local_30,local_38);
  return;
}

// Function: gcd_odd
ulong gcd_odd(ulong param_1,ulong param_2) {
  ulong uVar1;
  byte bVar2;
  ulong local_38;
  ulong local_30;
  
  if (param_1 != 0) {
    local_38 = param_2 >> 1;
    local_30 = param_1;
    while( true ) {
      if (local_30 == 0) {
        bVar2 = 0;
      }
      else {
        bVar2 = 0;
        for (uVar1 = local_30; (uVar1 & 1) == 0; uVar1 = uVar1 >> 1 | 0x8000000000000000) {
          bVar2 = bVar2 + 1;
        }
      }
      param_2 = local_30 >> (bVar2 & 0x3f);
      uVar1 = (param_2 >> 1) - local_38;
      if (uVar1 == 0) break;
      local_30 = highbit_to_mask(uVar1);
      local_38 = local_38 + (local_30 & uVar1);
      local_30 = (uVar1 ^ local_30) - local_30;
    }
  }
  return param_2;
}

// Function: gcd2_odd
void gcd2_odd(ulong param_1,ulong param_2,ulong param_3,ulong param_4) {
  byte bVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  ulong uVar5;
  ulong uVar6;
  ulong local_48;
  ulong local_40;
  ulong local_38;
  ulong local_30;
  
  if ((param_4 & 1) == 0) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("b0 & 1",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/factor.c",
                  0x203,__PRETTY_FUNCTION___5);
  }
  local_48 = param_4;
  local_40 = param_3;
  local_38 = param_2;
  local_30 = param_1;
  if ((param_2 == 0) && (local_30 = 0, local_38 = param_1, param_1 == 0)) {
    make_uuint(param_3,param_4);
    return;
  }
  do {
    if (local_38 == 0) {
      iVar3 = 0x40;
    }
    else {
      iVar3 = 0;
      for (uVar5 = local_38; (uVar5 & 1) == 0; uVar5 = uVar5 >> 1 | 0x8000000000000000) {
        iVar3 = iVar3 + 1;
      }
    }
    if (iVar3 != 0) {
      bVar1 = (byte)iVar3;
      local_38 = local_38 >> (bVar1 & 0x3f) | local_30 << (0x40 - bVar1 & 0x3f);
      local_30 = local_30 >> (bVar1 & 0x3f);
    }
    while( true ) {
      if ((local_40 | local_30) == 0) {
        uVar4 = gcd_odd(local_48,local_38);
        make_uuint(0,uVar4);
        return;
      }
      cVar2 = lt2(local_40,local_48,local_30,local_38);
      if (cVar2 != '\0') break;
      uVar5 = local_48 - local_38;
      uVar6 = (local_40 - local_30) - (ulong)(local_48 < local_38);
      local_48 = uVar5;
      local_40 = uVar6;
      if ((uVar5 == 0) && (local_40 = 0, local_48 = uVar6, uVar6 == 0)) {
        make_uuint(local_30,local_38);
        return;
      }
      if (local_48 == 0) {
        iVar3 = 0x40;
      }
      else {
        iVar3 = 0;
        for (uVar5 = local_48; (uVar5 & 1) == 0; uVar5 = uVar5 >> 1 | 0x8000000000000000) {
          iVar3 = iVar3 + 1;
        }
      }
      if (iVar3 != 0) {
        bVar1 = (byte)iVar3;
        local_48 = local_48 >> (bVar1 & 0x3f) | local_40 << (0x40 - bVar1 & 0x3f);
        local_40 = local_40 >> (bVar1 & 0x3f);
      }
    }
    uVar5 = local_38 - local_48;
    uVar6 = (local_30 - local_40) - (ulong)(local_38 < local_48);
    local_38 = uVar5;
    local_30 = uVar6;
    if (uVar5 == 0) {
      local_30 = 0;
      local_38 = uVar6;
    }
  } while( true );
}

// Function: factor_insert_multiplicity
void factor_insert_multiplicity(long param_1,ulong param_2,char param_3) {
  uint uVar1;
  long lVar2;
  long lVar3;
  uint local_20;
  
  uVar1 = (uint)*(byte *)(param_1 + 0xfa);
  lVar2 = param_1 + 0x10;
  lVar3 = param_1 + 0xe0;
  local_20 = uVar1;
  while ((0 < (int)local_20 && (param_2 <= *(ulong *)(lVar2 + (long)(int)local_20 * 8 + -8)))) {
    if (param_2 == *(ulong *)(lVar2 + (long)(int)local_20 * 8 + -8)) {
      *(char *)(lVar3 + (long)(int)local_20 + -1) =
           *(char *)(lVar3 + (long)(int)local_20 + -1) + param_3;
      return;
    }
    local_20 = local_20 - 1;
  }
  *(byte *)(param_1 + 0xfa) = *(byte *)(param_1 + 0xfa) + 1;
  memmove((void *)(lVar2 + ((long)(int)local_20 + 1) * 8),(void *)((long)(int)local_20 * 8 + lVar2),
          (long)(int)(uVar1 - local_20) * 8);
  memmove((void *)((long)(int)local_20 + 1 + lVar3),(void *)((int)local_20 + lVar3),
          (long)(int)(uVar1 - local_20));
  *(char *)(lVar3 + (int)local_20) = param_3;
  *(ulong *)((long)(int)local_20 * 8 + lVar2) = param_2;
  return;
}

// Function: factor_insert
void factor_insert(undefined8 param_1,undefined8 param_2) {
  factor_insert_multiplicity(param_1,param_2,1);
  return;
}

// Function: mpz_va_init
void mpz_va_init(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
                code *param_9,undefined8 param_10,undefined8 param_11,undefined8 param_12,
                undefined8 param_13,undefined8 param_14) {
  long lVar1;
  char in_AL;
  long *plVar2;
  ulong uVar3;
  long in_FS_OFFSET;
  uint local_d8;
  long *local_d0;
  long local_b8 [4];
  undefined8 local_98;
  undefined8 local_90;
  undefined4 local_88;
  undefined4 local_78;
  undefined4 local_68;
  undefined4 local_58;
  undefined4 local_48;
  undefined4 local_38;
  undefined4 local_28;
  undefined4 local_18;
  
  if (in_AL != '\0') {
    local_88 = param_1;
    local_78 = param_2;
    local_68 = param_3;
    local_58 = param_4;
    local_48 = param_5;
    local_38 = param_6;
    local_28 = param_7;
    local_18 = param_8;
  }
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_d8 = 8;
  local_d0 = (long *)&stack0x00000008;
  local_b8[1] = param_10;
  local_b8[2] = param_11;
  local_b8[3] = param_12;
  local_98 = param_13;
  local_90 = param_14;
  while( true ) {
    if (local_d8 < 0x30) {
      uVar3 = (ulong)local_d8;
      local_d8 = local_d8 + 8;
      plVar2 = (long *)((long)local_b8 + uVar3);
    }
    else {
      plVar2 = local_d0;
      local_d0 = local_d0 + 1;
    }
    if (*plVar2 == 0) break;
    (*param_9)(*plVar2);
  }
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: copy_mpn_from_mpz
void copy_mpn_from_mpz(long param_1,long param_2,undefined8 param_3) {
  long lVar1;
  undefined8 uVar2;
  
  lVar1 = mpz_size(param_3);
  uVar2 = mpz_limbs_read(param_3);
  mpn_copyi(param_1,uVar2,lVar1);
  mpn_zero(param_1 + lVar1 * 8,param_2 - lVar1);
  return;
}

// Function: mpn_tdiv_qr
void mpn_tdiv_qr(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
                long param_5,undefined8 param_6,long param_7) {
  undefined8 uVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  undefined local_68 [16];
  undefined local_58 [16];
  undefined local_48 [16];
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  mpz_va_init(mpz_init,local_68,local_58,0,param_5,param_6,param_6,param_5,param_4,param_3);
  uVar1 = mpz_roinit_n(local_38,param_6,param_7);
  uVar2 = mpz_roinit_n(local_48,param_4,param_5);
  mpz_tdiv_qr(local_68,local_58,uVar2,uVar1);
  copy_mpn_from_mpz(param_1,(param_5 - param_7) + 1,local_68);
  copy_mpn_from_mpz(param_2,param_7,local_58);
  mpz_va_init(mpz_clear,local_68,local_58,0);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: mp_factor_clear
void mp_factor_clear(void **param_1) {
  void *__ptr;
  long local_18;
  
  __ptr = *param_1;
  for (local_18 = 0; local_18 < (long)param_1[1]; local_18 = local_18 + 1) {
    mpz_clear((void *)((long)__ptr + local_18 * 0x18));
  }
  free(__ptr);
  return;
}

// Function: mp_factor_insert
void mp_factor_insert(long *param_1,undefined8 param_2,long param_3) {
  long lVar1;
  int iVar2;
  long local_20;
  long local_18;
  
  lVar1 = param_1[1];
  local_20 = *param_1;
  local_18 = lVar1;
  while ((0 < local_18 && (iVar2 = mpz_cmp(local_20 + local_18 * 0x18 + -0x18,param_2), -1 < iVar2))
        ) {
    if (iVar2 == 0) {
      *(long *)(local_20 + local_18 * 0x18 + -8) =
           param_3 + *(long *)(local_20 + local_18 * 0x18 + -8);
      return;
    }
    local_18 = local_18 + -1;
  }
  if (lVar1 == param_1[2]) {
    local_20 = xpalloc(local_20,param_1 + 2,1,0xffffffffffffffff,0x18);
    *param_1 = local_20;
  }
  param_1[1] = lVar1 + 1;
  memmove((void *)(local_20 + (local_18 + 1) * 0x18),(void *)(local_18 * 0x18 + local_20),
          (lVar1 - local_18) * 0x18);
  *(long *)(local_18 * 0x18 + local_20 + 0x10) = param_3;
  mpz_init_set(local_20 + local_18 * 0x18,param_2);
  return;
}

// Function: mp_factor_insert1
void mp_factor_insert1(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  long in_FS_OFFSET;
  undefined8 local_38;
  undefined8 local_30;
  undefined4 local_28;
  undefined4 local_24;
  undefined8 *local_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_28 = 0;
  local_24 = 1;
  local_20 = &local_38;
  local_38 = param_2;
  local_30 = param_1;
  mp_factor_insert(param_1,&local_28,param_3);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: divblock
ulong divblock(undefined8 param_1,ulong param_2,long param_3,long param_4,int param_5) {
  ulong uVar1;
  ulong local_28;
  
  local_28 = param_2;
  while (uVar1 = *(long *)(param_3 + (long)param_5 * 0x10) * local_28,
        uVar1 <= *(ulong *)(param_3 + (long)param_5 * 0x10 + 8)) {
    factor_insert(param_1,(long)*(short *)(primes_ptab + (param_4 + param_5) * 2));
    local_28 = uVar1;
  }
  return local_28;
}

// Function: factor_using_division
void factor_using_division(undefined8 param_1,ulong param_2,ulong param_3,long param_4) {
  short sVar1;
  long lVar2;
  undefined auVar3 [16];
  undefined auVar4 [16];
  byte bVar5;
  ulong uVar6;
  undefined1 *puVar7;
  undefined8 uVar8;
  long local_78;
  ulong local_70;
  ulong local_68;
  int local_50;
  ulong uStack_30;
  
  local_78 = param_4;
  local_70 = param_3;
  local_68 = param_2;
  if ((param_3 & 1) == 0) {
    if (param_3 == 0) {
      if (param_2 == 0) {
        local_50 = 0x40;
      }
      else {
        local_50 = 0;
        for (uVar6 = param_2; (uVar6 & 1) == 0; uVar6 = uVar6 >> 1 | 0x8000000000000000) {
          local_50 = local_50 + 1;
        }
      }
      local_70 = param_2 >> ((byte)local_50 & 0x3f);
      local_68 = 0;
      local_50 = local_50 + 0x40;
    }
    else {
      if (param_3 == 0) {
        local_50 = 0x40;
      }
      else {
        local_50 = 0;
        for (uVar6 = param_3; (uVar6 & 1) == 0; uVar6 = uVar6 >> 1 | 0x8000000000000000) {
          local_50 = local_50 + 1;
        }
      }
      bVar5 = (byte)local_50;
      local_70 = param_3 >> (bVar5 & 0x3f) | param_2 << (0x40 - bVar5 & 0x3f);
      local_68 = param_2 >> (bVar5 & 0x3f);
    }
    factor_insert_multiplicity(param_1,2,local_50);
  }
  for (; (local_68 != 0 && (local_78 < 0x29c)); local_78 = local_78 + 1) {
    sVar1 = *(short *)(primes_ptab + local_78 * 2);
    while( true ) {
      lVar2 = *(long *)(primes_dtab + local_78 * 0x10);
      auVar3._8_8_ = 0;
      auVar3._0_8_ = lVar2 * local_70;
      auVar4._8_8_ = 0;
      auVar4._0_8_ = (long)sVar1;
      uStack_30 = SUB168(auVar3 * auVar4,8);
      if ((local_68 < uStack_30) ||
         (uVar6 = *(long *)(primes_dtab + local_78 * 0x10) * (local_68 - uStack_30),
         *(ulong *)(primes_dtab + local_78 * 0x10 + 8) < uVar6)) break;
      factor_insert(param_1,(long)sVar1);
      local_70 = lVar2 * local_70;
      local_68 = uVar6;
    }
  }
  for (; local_78 < 0x29c; local_78 = local_78 + 8) {
    puVar7 = primes_dtab + local_78 * 0x10;
    uVar8 = divblock(param_1,local_70,puVar7,local_78,0);
    uVar8 = divblock(param_1,uVar8,puVar7,local_78,1);
    uVar8 = divblock(param_1,uVar8,puVar7,local_78,2);
    uVar8 = divblock(param_1,uVar8,puVar7,local_78,3);
    uVar8 = divblock(param_1,uVar8,puVar7,local_78,4);
    uVar8 = divblock(param_1,uVar8,puVar7,local_78,5);
    uVar8 = divblock(param_1,uVar8,puVar7,local_78,6);
    local_70 = divblock(param_1,uVar8,puVar7,local_78,7);
    if (local_70 <
        (ulong)(long)((int)*(short *)(primes_ptab + (local_78 + 8) * 2) *
                     (int)*(short *)(primes_ptab + (local_78 + 8) * 2))) break;
  }
  make_uuint(local_68,local_70);
  return;
}

// Function: mp_size
void mp_size(undefined8 param_1) {
  mpz_sgn(param_1);
  mpz_size(param_1);
  return;
}

// Function: mp_finish_up_in_single
undefined8 mp_finish_up_in_single(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  char cVar1;
  long lVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  uint local_13c;
  undefined4 local_128;
  undefined4 local_124;
  undefined *local_120;
  undefined local_118 [224];
  undefined auStack_38 [26];
  byte local_1e;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  lVar2 = mp_size(param_2);
  if (lVar2 < 3) {
    lVar2 = mpz_getlimbn(param_2,1);
    if (lVar2 < 0) {
      uVar3 = 0;
    }
    else {
      uVar3 = mpz_getlimbn(param_2,0);
      mpz_set_ui(param_2,1);
      factor_up(local_118,lVar2,uVar3,param_3);
      cVar1 = hi_is_set(local_118);
      if (cVar1 != '\0') {
        local_128 = 0;
        local_124 = 2;
        local_120 = local_118;
        mp_factor_insert(param_1,&local_128,1);
      }
      for (local_13c = (uint)local_1e; 0 < (int)local_13c; local_13c = local_13c - 1) {
        local_128 = 0;
        local_124 = 1;
        local_120 = local_118 + ((long)(int)(local_13c - 1) + 2) * 8;
        mp_factor_insert(param_1,&local_128,auStack_38[(int)(local_13c - 1)]);
      }
      uVar3 = 1;
    }
  }
  else {
    uVar3 = 0;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar3;
}

// Function: mp_finish_in_single
void mp_finish_in_single(undefined8 param_1,undefined8 param_2) {
  mp_finish_up_in_single(param_1,param_2,0);
  return;
}

// Function: mp_factor_using_division
undefined8 * mp_factor_using_division(undefined8 *param_1,undefined8 param_2) {
  char cVar1;
  int iVar2;
  long lVar3;
  long in_FS_OFFSET;
  long local_40;
  long local_38;
  undefined8 local_28;
  undefined8 local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (dev_debug != '\0') {
    fwrite("[trial division] ",1,0x11,_stderr);
  }
  mp_no_factors(&local_28);
  lVar3 = mpz_scan1(param_2,0);
  if (lVar3 != 0) {
    mpz_fdiv_q_2exp(param_2,param_2,lVar3);
    mp_factor_insert1(&local_28,2,lVar3);
    cVar1 = mp_finish_in_single(&local_28,param_2);
    if (cVar1 != '\0') {
      *param_1 = local_28;
      param_1[1] = local_20;
      param_1[2] = local_18;
      goto LAB_001018fe;
    }
  }
  for (local_38 = 0; local_38 < 0x29c; local_38 = local_38 + 1) {
    lVar3 = (long)*(short *)(primes_ptab + local_38 * 2);
    local_40 = 0;
    while (iVar2 = mpz_divisible_ui_p(param_2,lVar3), iVar2 != 0) {
      mpz_divexact_ui(param_2,param_2,lVar3);
      cVar1 = mp_finish_up_in_single(&local_28,param_2,local_38);
      if (cVar1 != '\0') {
        mp_factor_insert1(&local_28,lVar3,local_40 + 1);
        *param_1 = local_28;
        param_1[1] = local_20;
        param_1[2] = local_18;
        goto LAB_001018fe;
      }
      local_40 = local_40 + 1;
    }
    if (local_40 != 0) {
      mp_factor_insert1(&local_28,lVar3,local_40);
    }
    iVar2 = mpz_cmp_ui(param_2,lVar3 * lVar3);
    if (iVar2 < 0) break;
  }
  *param_1 = local_28;
  param_1[1] = local_20;
  param_1[2] = local_18;
LAB_001018fe:
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return param_1;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: binv_limb
long binv_limb(ulong param_1) {
  ulong uVar1;
  long lVar2;
  int local_14;
  long local_10;
  
  uVar1 = (ulong)(byte)binvert_table[(uint)(param_1 >> 1) & 0x7f];
  lVar2 = uVar1 * 2 - uVar1 * uVar1 * param_1;
  lVar2 = lVar2 * 2 - lVar2 * lVar2 * param_1;
  local_10 = lVar2 * 2 - lVar2 * lVar2 * param_1;
  for (local_14 = 0x40; local_14 < 0x40; local_14 = local_14 << 1) {
    local_10 = local_10 * 2 - local_10 * local_10 * param_1;
  }
  return local_10;
}

// Function: mulredc
long mulredc(ulong param_1,ulong param_2,ulong param_3,long param_4) {
  undefined auVar1 [16];
  undefined auVar2 [16];
  undefined auVar3 [16];
  undefined auVar4 [16];
  long in_FS_OFFSET;
  long local_40;
  long local_38;
  ulong local_30;
  ulong local_18;
  
  auVar1._8_8_ = 0;
  auVar1._0_8_ = param_1;
  auVar3._8_8_ = 0;
  auVar3._0_8_ = param_2;
  local_38 = SUB168(auVar1 * auVar3,0);
  auVar2._8_8_ = 0;
  auVar2._0_8_ = local_38 * param_4;
  auVar4._8_8_ = 0;
  auVar4._0_8_ = param_3;
  local_30 = SUB168(auVar1 * auVar3,8);
  local_18 = SUB168(auVar2 * auVar4,8);
  local_40 = local_30 - local_18;
  if (local_30 < local_18) {
    local_40 = param_3 + local_40;
  }
  if (*(long *)(in_FS_OFFSET + 0x28) == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_40;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: mulredc2
undefined8
mulredc2(undefined8 *param_1,ulong param_2,ulong param_3,ulong param_4,ulong param_5,ulong param_6,
        ulong param_7,long param_8) {
  undefined auVar1 [16];
  undefined auVar2 [16];
  undefined auVar3 [16];
  undefined auVar4 [16];
  undefined auVar5 [16];
  undefined auVar6 [16];
  undefined auVar7 [16];
  undefined auVar8 [16];
  undefined auVar9 [16];
  undefined auVar10 [16];
  undefined auVar11 [16];
  undefined auVar12 [16];
  undefined auVar13 [16];
  undefined auVar14 [16];
  undefined auVar15 [16];
  undefined auVar16 [16];
  undefined auVar17 [16];
  undefined auVar18 [16];
  undefined auVar19 [16];
  undefined auVar20 [16];
  char cVar21;
  ulong uVar22;
  ulong uVar23;
  long lVar24;
  ulong uVar25;
  ulong uVar26;
  undefined8 local_50;
  undefined8 local_48;
  long local_40;
  ulong local_38;
  ulong local_20;
  ulong local_18;
  long local_10;
  
  auVar20._8_8_ = param_6;
  auVar20._0_8_ = param_7;
  if ((long)param_6 < 0) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("(m1 >> (64 - 1)) == 0",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/factor.c",
                  0x449,__PRETTY_FUNCTION___3);
  }
  auVar1._8_8_ = 0;
  auVar1._0_8_ = param_3;
  auVar9._8_8_ = 0;
  auVar9._0_8_ = param_5;
  auVar2._8_8_ = 0;
  auVar2._0_8_ = param_3;
  auVar10._8_8_ = 0;
  auVar10._0_8_ = param_4;
  local_40 = SUB168(auVar1 * auVar9,0);
  uVar22 = -param_8 * local_40;
  auVar3._8_8_ = 0;
  auVar3._0_8_ = uVar22;
  auVar11._8_8_ = 0;
  auVar11._0_8_ = param_7;
  auVar4._8_8_ = 0;
  auVar4._0_8_ = uVar22;
  auVar12._8_8_ = 0;
  auVar12._0_8_ = param_6;
  uVar22 = SUB168(auVar2 * auVar10,0) + (ulong)(local_40 != 0);
  local_20 = SUB168(auVar3 * auVar11,8);
  uVar23 = uVar22 + local_20;
  local_38 = SUB168(auVar1 * auVar9,8);
  auVar17._8_8_ =
       SUB168(auVar2 * auVar10,8) + (ulong)CARRY8(uVar22,local_20) + (ulong)CARRY8(uVar23,local_38);
  auVar17._0_8_ = uVar23 + local_38;
  auVar17 = auVar4 * auVar12 + auVar17;
  auVar18._8_8_ = 0;
  auVar18._0_8_ = auVar17._0_8_;
  uVar25 = auVar17._8_8_;
  auVar5._8_8_ = 0;
  auVar5._0_8_ = param_2;
  auVar13._8_8_ = 0;
  auVar13._0_8_ = param_5;
  auVar6._8_8_ = 0;
  auVar6._0_8_ = param_2;
  auVar14._8_8_ = 0;
  auVar14._0_8_ = param_4;
  auVar18 = auVar5 * auVar13 + auVar18;
  lVar24 = auVar18._0_8_;
  uVar26 = auVar18._8_8_;
  uVar22 = -param_8 * lVar24;
  local_10 = SUB168(auVar6 * auVar14,8);
  local_18 = SUB168(auVar6 * auVar14,0);
  auVar7._8_8_ = 0;
  auVar7._0_8_ = uVar22;
  auVar15._8_8_ = 0;
  auVar15._0_8_ = param_7;
  auVar8._8_8_ = 0;
  auVar8._0_8_ = uVar22;
  auVar16._8_8_ = 0;
  auVar16._0_8_ = param_6;
  uVar22 = local_18 + uVar25 + (ulong)(lVar24 != 0);
  local_20 = SUB168(auVar7 * auVar15,8);
  uVar23 = uVar22 + local_20;
  auVar19._8_8_ =
       local_10 + (ulong)CARRY8(local_18,uVar25) + (ulong)CARRY8(uVar22,local_20) +
       (ulong)CARRY8(uVar23,uVar26);
  auVar19._0_8_ = uVar23 + uVar26;
  auVar19 = auVar8 * auVar16 + auVar19;
  cVar21 = ge2(auVar19._8_8_,auVar19._0_8_,param_6,param_7);
  if (cVar21 != '\0') {
    auVar19 = auVar19 - auVar20;
  }
  local_50 = auVar19._8_8_;
  local_48 = auVar19._0_8_;
  *param_1 = local_50;
  return local_48;
}

// Function: prime_p
undefined8 prime_p(ulong param_1) {
  undefined8 uVar1;
  long in_FS_OFFSET;
  ulong local_30;
  undefined4 local_28;
  undefined4 local_24;
  ulong *local_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_30 = param_1;
  if (param_1 < 2) {
    uVar1 = 0;
  }
  else if (param_1 < 0x17ded79) {
    uVar1 = 1;
  }
  else {
    local_28 = 0;
    local_24 = 1;
    local_20 = &local_30;
    uVar1 = mp_prime_p(&local_28);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar1;
}

// Function: prime2_p
void prime2_p(long param_1,undefined8 param_2) {
  long in_FS_OFFSET;
  undefined local_38 [16];
  undefined4 local_28;
  undefined4 local_24;
  undefined *local_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (param_1 == 0) {
    prime_p(param_2);
  }
  else {
    local_38 = make_uuint(param_1,param_2);
    local_28 = 0;
    local_24 = 2;
    local_20 = local_38;
    mp_prime_p(&local_28);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: mp_prime_p
bool mp_prime_p(undefined8 param_1) {
  int iVar1;
  bool bVar2;
  
  iVar1 = mpz_cmp_ui(param_1,1);
  if (iVar1 < 1) {
    bVar2 = false;
  }
  else {
    iVar1 = mpz_cmp_ui(param_1,0x17ded79);
    if (iVar1 < 0) {
      bVar2 = true;
    }
    else {
      iVar1 = mpz_probab_prime_p(param_1,0x18);
      bVar2 = iVar1 != 0;
    }
  }
  return bVar2;
}

// Function: factor_using_pollard_rho
void factor_using_pollard_rho(undefined8 param_1,ulong param_2,ulong param_3) {
  long lVar1;
  char cVar2;
  undefined8 uVar3;
  ulong uVar4;
  ulong uVar5;
  long lVar6;
  long in_FS_OFFSET;
  bool bVar7;
  ulong local_e8;
  int local_cc;
  ulong local_c0;
  ulong local_b8;
  ulong local_b0;
  ulong local_a8;
  long local_a0;
  long local_98;
  ulong local_90;
  ulong local_88;
  long local_78;
  ulong local_70;
  long local_68;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_a0 = 1;
  local_98 = 1;
  local_88 = 0;
  local_78 = 1;
  local_70 = 0;
  if (param_2 < 2) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("__r1 < __d1",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/factor.c",
                  0x4bc,"factor_using_pollard_rho");
  }
  local_90 = param_2;
  for (local_cc = 0x40; 0 < local_cc; local_cc = local_cc + -1) {
    local_88 = local_88 >> 1 | local_90 << 0x3f;
    local_90 = local_90 >> 1;
    cVar2 = ge2(local_78,local_70,local_90,local_88);
    if (cVar2 != '\0') {
      bVar7 = local_70 < local_88;
      local_70 = local_70 - local_88;
      local_78 = (local_78 - local_90) - (ulong)bVar7;
    }
  }
  local_a8 = local_70;
  local_c0 = (local_70 - (param_2 - local_70)) +
             (param_2 & (long)(int)-(uint)(local_70 < param_2 - local_70));
  local_e8 = param_2;
  local_b8 = local_c0;
  local_b0 = local_c0;
  do {
    if (local_e8 == 1) {
LAB_0010243d:
      if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return;
    }
    if (local_e8 <= param_3) {
                    /* WARNING: Subroutine does not return */
      __assert_fail("a < n",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/factor.c"
                    ,0x4c2,"factor_using_pollard_rho");
    }
    uVar3 = binv_limb(local_e8);
    while( true ) {
      uVar4 = mulredc(local_c0,local_c0,local_e8,uVar3);
      uVar5 = (uVar4 - (local_e8 - param_3)) +
              (local_e8 & (long)(int)-(uint)(uVar4 < local_e8 - param_3));
      local_a8 = mulredc(local_a8,(local_b8 - uVar5) +
                                  (local_e8 & (long)(int)-(uint)(local_b8 < uVar5)),local_e8,uVar3);
      uVar4 = local_b0;
      if ((((uint)local_a0 & 0x1f) == 1) &&
         (lVar6 = gcd_odd(local_a8,local_e8), uVar4 = uVar5, lVar6 != 1)) break;
      local_b0 = uVar4;
      local_a0 = local_a0 + -1;
      local_c0 = uVar5;
      if (local_a0 == 0) {
        local_a0 = local_98;
        for (local_68 = 0; local_68 < local_98; local_68 = local_68 + 1) {
          uVar4 = mulredc(local_c0,local_c0,local_e8,uVar3);
          local_c0 = (uVar4 - (local_e8 - param_3)) +
                     (local_e8 & (long)(int)-(uint)(uVar4 < local_e8 - param_3));
        }
        local_b0 = local_c0;
        local_b8 = uVar5;
        local_98 = local_98 << 1;
      }
    }
    do {
      uVar4 = mulredc(local_b0,local_b0,local_e8,uVar3);
      local_b0 = (uVar4 - (local_e8 - param_3)) +
                 (local_e8 & (long)(int)-(uint)(uVar4 < local_e8 - param_3));
      uVar4 = gcd_odd((local_b8 - local_b0) + (local_e8 & (long)(int)-(uint)(local_b8 < local_b0)),
                      local_e8);
    } while (uVar4 == 1);
    if (local_e8 == uVar4) {
      factor_using_pollard_rho(param_1,local_e8,param_3 + 1);
      goto LAB_0010243d;
    }
    local_e8 = local_e8 / uVar4;
    cVar2 = prime_p(uVar4);
    if (cVar2 == '\x01') {
      factor_insert(param_1,uVar4);
    }
    else {
      factor_using_pollard_rho(param_1,uVar4,param_3 + 1);
    }
    cVar2 = prime_p(local_e8);
    if (cVar2 != '\0') {
      factor_insert(param_1,local_e8);
      goto LAB_0010243d;
    }
    local_c0 = uVar5 % local_e8;
    local_b8 = local_b8 % local_e8;
    local_b0 = local_b0 % local_e8;
  } while( true );
}

// Function: factor_using_pollard_rho2
void factor_using_pollard_rho2(undefined8 param_1,ulong param_2,ulong param_3,ulong param_4) {
  undefined auVar1 [16];
  char cVar2;
  ulong uVar3;
  long lVar4;
  long in_FS_OFFSET;
  bool bVar5;
  undefined auVar6 [16];
  ulong local_100;
  ulong local_f8;
  ulong local_e0;
  ulong local_d8;
  ulong local_d0;
  ulong local_c8;
  ulong local_c0;
  ulong local_b8;
  long local_b0;
  ulong local_a8;
  ulong local_a0;
  ulong local_98;
  ulong local_90;
  ulong local_88;
  ulong local_80;
  long local_78;
  long local_70;
  ulong local_68;
  ulong local_60;
  long local_58;
  long local_50;
  undefined8 local_48;
  ulong local_40;
  ulong local_38;
  long local_30;
  ulong local_28;
  undefined local_20 [16];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_78 = 1;
  local_70 = 1;
  if (param_2 < 2) {
    local_68 = 0;
    local_60 = 1;
    local_58 = 0x80;
  }
  else {
    local_68 = 1;
    local_60 = 0;
    local_58 = 0x40;
  }
  while (local_58 != 0) {
    local_68 = local_60 >> 0x3f | local_68 * 2;
    local_60 = local_60 << 1;
    local_58 = local_58 + -1;
    cVar2 = ge2(local_68,local_60,param_2,param_3);
    if (cVar2 != '\0') {
      bVar5 = local_60 < param_3;
      local_60 = local_60 - param_3;
      local_68 = (local_68 - param_2) - (ulong)bVar5;
    }
  }
  local_88 = local_68;
  local_80 = local_60;
  local_d8 = local_60 * 2;
  local_e0 = local_68 * 2 + (ulong)CARRY8(local_60,local_60);
  local_58 = local_58 + -1;
  cVar2 = ge2(local_e0,local_d8,param_2,param_3);
  if (cVar2 != '\0') {
    bVar5 = local_d8 < param_3;
    local_d8 = local_d8 - param_3;
    local_e0 = (local_e0 - param_2) - (ulong)bVar5;
  }
  local_d0 = local_e0;
  local_c0 = local_e0;
  local_c8 = local_d8;
  local_b8 = local_d8;
  local_100 = param_3;
  local_f8 = param_2;
LAB_00102ee8:
  if ((local_f8 != 0) || (local_100 != 1)) {
    local_48 = binv_limb(local_100);
    do {
      uVar3 = mulredc2(&local_90,local_e0,local_d8,local_e0,local_d8,local_f8,local_100,local_48);
      local_d8 = uVar3 + param_4;
      local_e0 = local_90 + CARRY8(uVar3,param_4);
      cVar2 = ge2(local_e0,local_d8,local_f8,local_100);
      if (cVar2 != '\0') {
        bVar5 = local_d8 < local_100;
        local_d8 = local_d8 - local_100;
        local_e0 = (local_e0 - local_f8) - (ulong)bVar5;
      }
      local_a8 = local_c8 - local_d8;
      local_40 = (ulong)(local_c8 < local_d8);
      local_b0 = (local_d0 - local_e0) - local_40;
      if (local_d0 - local_e0 < local_40 || local_d0 < local_e0) {
        bVar5 = CARRY8(local_a8,local_100);
        local_a8 = local_a8 + local_100;
        local_b0 = local_b0 + local_f8 + (ulong)bVar5;
      }
      local_80 = mulredc2(&local_90,local_88,local_80,local_b0,local_a8,local_f8,local_100,local_48)
      ;
      local_88 = local_90;
      if (((uint)local_78 & 0x1f) == 1) {
        auVar6 = gcd2_odd(local_90,local_80,local_f8,local_100);
        uuset(&local_a0,&local_98,auVar6._0_8_,auVar6._8_8_);
        if ((local_a0 != 0) || (local_98 != 1)) goto LAB_001029c9;
        local_c0 = local_e0;
        local_b8 = local_d8;
      }
      local_78 = local_78 + -1;
      if (local_78 == 0) {
        local_d0 = local_e0;
        local_c8 = local_d8;
        local_78 = local_70;
        local_70 = local_70 << 1;
        for (local_50 = 0; local_50 < local_78; local_50 = local_50 + 1) {
          uVar3 = mulredc2(&local_90,local_e0,local_d8,local_e0,local_d8,local_f8,local_100,local_48
                          );
          local_d8 = uVar3 + param_4;
          local_e0 = local_90 + CARRY8(uVar3,param_4);
          cVar2 = ge2(local_e0,local_d8,local_f8,local_100);
          if (cVar2 != '\0') {
            bVar5 = local_d8 < local_100;
            local_d8 = local_d8 - local_100;
            local_e0 = (local_e0 - local_f8) - (ulong)bVar5;
          }
        }
        local_c0 = local_e0;
        local_b8 = local_d8;
      }
    } while( true );
  }
  goto LAB_00102f04;
LAB_001029c9:
  do {
    uVar3 = mulredc2(&local_90,local_c0,local_b8,local_c0,local_b8,local_f8,local_100,local_48);
    local_b8 = uVar3 + param_4;
    local_c0 = local_90 + CARRY8(uVar3,param_4);
    cVar2 = ge2(local_c0,local_b8,local_f8,local_100);
    if (cVar2 != '\0') {
      bVar5 = local_b8 < local_100;
      local_b8 = local_b8 - local_100;
      local_c0 = (local_c0 - local_f8) - (ulong)bVar5;
    }
    local_a8 = local_c8 - local_b8;
    local_38 = (ulong)(local_c8 < local_b8);
    local_b0 = (local_d0 - local_c0) - local_38;
    if (local_d0 - local_c0 < local_38 || local_d0 < local_c0) {
      bVar5 = CARRY8(local_a8,local_100);
      local_a8 = local_a8 + local_100;
      local_b0 = local_b0 + local_f8 + (ulong)bVar5;
    }
    auVar6 = gcd2_odd(local_b0,local_a8,local_f8,local_100);
    uuset(&local_a0,&local_98,auVar6._0_8_,auVar6._8_8_);
  } while ((local_a0 == 0) && (local_98 == 1));
  if (local_a0 == 0) {
    local_30 = binv_limb(local_98);
    local_100 = local_100 * local_30;
    if (local_f8 < local_98) {
      local_f8 = 0;
    }
    else {
      auVar6._8_8_ = 0;
      auVar6._0_8_ = local_100;
      auVar1._8_8_ = 0;
      auVar1._0_8_ = local_98;
      local_20 = auVar6 * auVar1;
      local_f8 = (local_f8 - local_20._8_8_) * local_30;
    }
    local_28 = local_100;
    cVar2 = prime_p(local_98);
    if (cVar2 == '\x01') {
      factor_insert(param_1,local_98);
    }
    else {
      factor_using_pollard_rho(param_1,local_98,param_4 + 1);
    }
  }
  else {
    if ((local_f8 == local_a0) && (local_100 == local_98)) {
      factor_using_pollard_rho2(param_1,local_f8,local_100,param_4 + 1);
      goto LAB_00102f04;
    }
    lVar4 = binv_limb(local_98);
    local_100 = lVar4 * local_100;
    local_f8 = 0;
    cVar2 = prime2_p(local_a0,local_98);
    if (cVar2 == '\x01') {
      factor_insert_large(param_1,local_a0,local_98);
    }
    else {
      factor_using_pollard_rho2(param_1,local_a0,local_98,param_4 + 1);
    }
  }
  if (local_f8 == 0) {
    cVar2 = prime_p(local_100);
    if (cVar2 == '\0') {
      factor_using_pollard_rho(param_1,local_100,param_4);
    }
    else {
      factor_insert(param_1,local_100);
    }
    goto LAB_00102f04;
  }
  cVar2 = prime2_p(local_f8,local_100);
  if (cVar2 != '\0') {
    factor_insert_large(param_1,local_f8,local_100);
LAB_00102f04:
    if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    return;
  }
  auVar6 = mod2(local_e0,local_d8,local_f8,local_100);
  uuset(&local_e0,&local_d8,auVar6._0_8_,auVar6._8_8_);
  auVar6 = mod2(local_d0,local_c8,local_f8,local_100);
  uuset(&local_d0,&local_c8,auVar6._0_8_,auVar6._8_8_);
  auVar6 = mod2(local_c0,local_b8,local_f8,local_100);
  uuset(&local_c0,&local_b8,auVar6._0_8_,auVar6._8_8_);
  goto LAB_00102ee8;
}

// Function: mp_modadd
void mp_modadd(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
              undefined8 param_5) {
  int iVar1;
  long lVar2;
  
  lVar2 = mpn_add_n(param_1,param_2,param_3,param_5);
  if ((lVar2 == 0) && (iVar1 = mpn_cmp(param_1,param_4,param_5), iVar1 < 0)) {
    return;
  }
  mpn_sub_n(param_1,param_1,param_4,param_5);
  return;
}

// Function: mp_modsub
void mp_modsub(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
              undefined8 param_5) {
  long lVar1;
  
  lVar1 = mpn_sub_n(param_1,param_2,param_3,param_5);
  if (lVar1 != 0) {
    mpn_add_n(param_1,param_1,param_4,param_5);
  }
  return;
}

// Function: mp_modadd_1
void mp_modadd_1(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
                undefined8 param_5) {
  int iVar1;
  long lVar2;
  
  lVar2 = mpn_add_1(param_1,param_2,param_5,param_3);
  if ((lVar2 == 0) && (iVar1 = mpn_cmp(param_1,param_4,param_5), iVar1 < 0)) {
    return;
  }
  mpn_sub_n(param_1,param_1,param_4,param_5);
  return;
}

// Function: mp_mulredc
void mp_mulredc(undefined8 param_1,undefined8 param_2,undefined8 *param_3,undefined8 param_4,
               long param_5,long param_6,long *param_7) {
  long lVar1;
  long local_20;
  
  lVar1 = mpn_mul_1(param_7,param_2,param_5,*param_3);
  param_7[param_5] = lVar1;
  lVar1 = mpn_addmul_1(param_7,param_4,param_5,*param_7 * param_6);
  *param_7 = lVar1;
  for (local_20 = 1; local_20 < param_5; local_20 = local_20 + 1) {
    lVar1 = mpn_addmul_1(param_7 + local_20,param_2,param_5,param_3[local_20]);
    param_7[local_20 + param_5] = lVar1;
    lVar1 = mpn_addmul_1(param_7 + local_20,param_4,param_5,param_7[local_20] * param_6);
    param_7[local_20] = lVar1;
  }
  mp_modadd(param_1,param_7,param_7 + param_5,param_4,param_5);
  return;
}

// Function: mp_factor_using_pollard_rho
void mp_factor_using_pollard_rho(undefined8 param_1,long *param_2,long param_3,long param_4) {
  char cVar1;
  int iVar2;
  void *__ptr;
  long lVar3;
  long lVar4;
  long lVar5;
  long lVar6;
  long lVar7;
  long lVar8;
  long lVar9;
  long *plVar10;
  undefined8 uVar11;
  undefined8 uVar12;
  long in_FS_OFFSET;
  long local_c8;
  long local_c0;
  long local_b8;
  undefined4 local_58;
  undefined4 local_54;
  long *local_50;
  undefined4 local_48;
  undefined4 local_44;
  long *local_40;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (dev_debug != '\0') {
    fprintf(_stderr,"[pollard-rho (%lu)] ",param_4);
  }
  __ptr = (void *)xinmalloc(param_3 * 10 + 3,8);
  lVar3 = (long)__ptr + param_3 * 0x10 + 8;
  lVar4 = lVar3 + (param_3 + 2) * 8;
  lVar5 = lVar4 + param_3 * 8;
  lVar6 = lVar5 + param_3 * 8;
  lVar7 = lVar6 + param_3 * 8;
  lVar8 = lVar7 + param_3 * 8;
  lVar9 = lVar8 + param_3 * 8;
  plVar10 = (long *)(lVar9 + param_3 * 8);
  mpn_zero(__ptr,param_3);
  *(undefined8 *)((long)__ptr + param_3 * 8) = 1;
  mpn_tdiv_qr(lVar3,lVar4,0,__ptr,param_3 + 1,param_2,param_3);
  mp_modadd(lVar5,lVar4,lVar4,param_2,param_3);
  mpn_copyi(lVar6,lVar5,param_3);
  mpn_copyi(lVar7,lVar5,param_3);
  uVar11 = binv_limb(-*param_2);
  local_c8 = 1;
  do {
    for (local_c0 = local_c8; 0 < local_c0; local_c0 = local_c0 + -1) {
      mp_mulredc(lVar8,lVar5,lVar5,param_2,param_3,uVar11,__ptr);
      mp_modadd_1(lVar5,lVar8,param_4,param_2,param_3);
      mp_modsub(lVar8,lVar7,lVar5,param_2,param_3);
      mp_mulredc(lVar4,lVar4,lVar8,param_2,param_3,uVar11,__ptr);
      if (local_c0 % 0x80 == 1) {
        iVar2 = mpn_zero_p(lVar4,param_3);
        if (iVar2 != 0) {
          mp_factor_using_pollard_rho(param_1,param_2,param_3,param_4 + 1);
          goto LAB_001039f2;
        }
        mpn_copyi(lVar8,lVar4,param_3);
        mpn_copyi(lVar9,param_2,param_3);
        lVar3 = mpn_gcd(plVar10,lVar8,param_3,lVar9,param_3);
        if ((lVar3 != 1) || (*plVar10 != 1)) goto LAB_0010377a;
        mpn_copyi(lVar6,lVar5,param_3);
      }
    }
    mpn_copyi(lVar7,lVar5,param_3);
    for (local_b8 = local_c8 * 2; 0 < local_b8; local_b8 = local_b8 + -1) {
      mp_mulredc(lVar8,lVar5,lVar5,param_2,param_3,uVar11,__ptr);
      mp_modadd_1(lVar5,lVar8,param_4,param_2,param_3);
    }
    mpn_copyi(lVar6,lVar5,param_3);
    local_c8 = local_c8 << 1;
  } while( true );
  while (*plVar10 == 1) {
LAB_0010377a:
    mp_mulredc(lVar8,lVar6,lVar6,param_2,param_3,uVar11,__ptr);
    mp_modadd_1(lVar6,lVar8,param_4,param_2,param_3);
    mp_modsub(lVar8,lVar7,lVar6,param_2,param_3);
    mpn_copyi(lVar9,param_2,param_3);
    lVar3 = mpn_gcd(plVar10,lVar8,param_3,lVar9,param_3);
    if (lVar3 != 1) break;
  }
  local_58 = 0;
  local_54 = (undefined4)lVar3;
  local_48 = 0;
  local_44 = (undefined4)param_3;
  local_50 = plVar10;
  local_40 = param_2;
  mpz_init(local_38);
  mpz_divexact(local_38,&local_48,&local_58);
  cVar1 = mp_finish_in_single(param_1,&local_58);
  if (cVar1 != '\x01') {
    cVar1 = mp_prime_p(&local_58);
    if (cVar1 == '\0') {
      mp_factor_using_pollard_rho(param_1,plVar10,lVar3,param_4 + 1);
    }
    else {
      mp_factor_insert(param_1,&local_58,1);
    }
  }
  cVar1 = mp_finish_in_single(param_1,local_38);
  if (cVar1 != '\x01') {
    cVar1 = mp_prime_p(local_38);
    if (cVar1 == '\0') {
      if (dev_debug != '\0') {
        fwrite("[composite factor--restarting pollard-rho] ",1,0x2b,_stderr);
      }
      uVar11 = mp_size(local_38);
      uVar12 = mpz_limbs_read(local_38);
      mp_factor_using_pollard_rho(param_1,uVar12,uVar11,param_4 + 1);
    }
    else {
      mp_factor_insert(param_1,local_38,1);
    }
  }
  mpz_clear(local_38);
LAB_001039f2:
  free(__ptr);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: factor_up
void factor_up(long param_1,long param_2,ulong param_3,undefined8 param_4) {
  char cVar1;
  undefined auVar2 [16];
  ulong local_20;
  long local_18;
  long local_10;
  
  *(undefined *)(param_1 + 0xfa) = 0;
  local_20 = param_3;
  local_18 = param_2;
  local_10 = param_1;
  hiset(param_1,0);
  if ((local_18 != 0) || (1 < local_20)) {
    auVar2 = factor_using_division(local_10,local_18,local_20,param_4);
    uuset(&local_18,&local_20,auVar2._0_8_,auVar2._8_8_);
    if ((local_18 != 0) || (1 < local_20)) {
      cVar1 = prime2_p(local_18,local_20);
      if (cVar1 == '\0') {
        if (local_18 == 0) {
          factor_using_pollard_rho(local_10,local_20,1);
        }
        else {
          factor_using_pollard_rho2(local_10,local_18,local_20,1);
        }
      }
      else {
        factor_insert_large(local_10,local_18,local_20);
      }
    }
  }
  return;
}

// Function: factor
void factor(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  factor_up(param_1,param_2,param_3,0);
  return;
}

// Function: mp_factor
undefined8 * mp_factor(undefined8 *param_1,undefined8 param_2) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  undefined8 local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_48;
  undefined8 *local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_48 = param_2;
  local_40 = param_1;
  mp_no_factors(&local_38);
  iVar2 = mpz_sgn(local_48);
  if (iVar2 != 0) {
    mp_factor_using_division(&local_68,local_48);
    local_38 = local_68;
    local_30 = local_60;
    local_28 = local_58;
    iVar2 = mpz_cmp_ui(local_48,1);
    if (iVar2 != 0) {
      if (dev_debug != '\0') {
        fwrite("[is number prime?] ",1,0x13,_stderr);
      }
      cVar1 = mp_prime_p(local_48);
      if (cVar1 == '\0') {
        uVar3 = mp_size(local_48);
        uVar4 = mpz_limbs_read(local_48);
        mp_factor_using_pollard_rho(&local_38,uVar4,uVar3,1);
      }
      else {
        mp_factor_insert(&local_38,local_48,1);
      }
    }
  }
  *local_40 = local_38;
  local_40[1] = local_30;
  local_40[2] = local_28;
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_40;
}

// Function: lbuf_flush
void lbuf_flush(void) {
  long lVar1;
  long lVar2;
  
  lVar1 = lbuffered;
  lbuffered = 0;
  lVar2 = full_write(1,lbuf_buf,lVar1);
  if (lVar1 != lVar2) {
    write_error();
  }
  return;
}

// Function: lbuf_half_flush
void lbuf_half_flush(void) {
  void *pvVar1;
  undefined1 *__src;
  size_t __n;
  
  pvVar1 = memrchr(lbuf_buf,10,0x1000);
  if (pvVar1 == (void *)0x0) {
    __src = lbuf_buf + 0x1000;
  }
  else {
    __src = (undefined1 *)((long)pvVar1 + 1);
  }
  __n = (long)lbuffered - (long)(__src + -0x104dc0);
  lbuffered = __src + -0x104dc0;
  lbuf_flush();
  lbuffered = (undefined1 *)__n;
  memmove(lbuf_buf,__src,__n);
  return;
}

// Function: lbuf_putc
void lbuf_putc(undefined param_1) {
  undefined1 *puVar1;
  
  puVar1 = lbuf_buf + lbuffered;
  lbuffered = lbuffered + 1;
  *puVar1 = param_1;
  return;
}

// Function: lbuf_putnl
void lbuf_putnl(void) {
  lbuf_putc(10);
  if (line_buffered_1 < 0) {
    line_buffered_1 = isatty(1);
  }
  if (line_buffered_1 == 0) {
    if (0xfff < lbuffered) {
      lbuf_half_flush();
    }
  }
  else {
    lbuf_flush();
  }
  return;
}

// Function: lbuf_putint_append
void lbuf_putint_append(ulong param_1,undefined8 *param_2) {
  undefined8 *puVar1;
  undefined *puVar2;
  ulong local_20;
  undefined8 *local_18;
  undefined *local_10;
  
  local_20 = param_1;
  local_18 = param_2;
  do {
    local_18 = (undefined8 *)((long)local_18 + -1);
    *(char *)local_18 =
         (char)local_20 + ((char)(local_20 / 10 << 2) + (char)(local_20 / 10)) * -2 + '0';
    local_20 = local_20 / 10;
  } while (local_20 != 0);
  puVar2 = lbuf_buf + (long)lbuffered;
  do {
    local_10 = puVar2;
    puVar1 = (undefined8 *)((long)local_18 + 1);
    *local_10 = *(undefined *)local_18;
    local_18 = puVar1;
    puVar2 = local_10 + 1;
  } while (puVar1 < &lbuffered);
  lbuffered = local_10 + -0x104dbf;
  return;
}

// Function: lbuf_putint
void lbuf_putint(undefined8 param_1) {
  lbuf_putint_append(param_1,&lbuffered);
  return;
}

// Function: lbuf_putbitcnt
void lbuf_putbitcnt(undefined8 param_1) {
  lbuf_putint_append(param_1,&lbuffered);
  return;
}

// Function: print_uuint
void print_uuint(undefined8 param_1,undefined8 param_2) {
  char cVar1;
  bool bVar2;
  int local_58;
  int local_54;
  ulong local_50;
  ulong local_48;
  undefined8 *local_40;
  ulong local_38;
  ulong local_30;
  ulong local_28;
  ulong local_20;
  ulong local_18;
  ulong local_10;
  
  local_50 = hi(param_1,param_2);
  local_48 = lo(param_1,param_2);
  local_40 = &lbuffered;
  while( true ) {
    if (local_50 == 0) {
      lbuf_putint_append(local_48,local_40);
      return;
    }
    local_18 = local_50 % 10000000000000000000;
    local_50 = local_50 / 10000000000000000000;
    local_30 = 10000000000000000000;
    local_28 = 0;
    local_20 = 0;
    local_10 = local_48;
    if (9999999999999999999 < local_18) break;
    for (local_58 = 0x40; 0 < local_58; local_58 = local_58 + -1) {
      local_28 = local_28 >> 1 | local_30 << 0x3f;
      local_30 = local_30 >> 1;
      local_20 = local_20 * 2;
      cVar1 = ge2(local_18,local_10,local_30,local_28);
      if (cVar1 != '\0') {
        local_20 = local_20 + 1;
        bVar2 = local_10 < local_28;
        local_10 = local_10 - local_28;
        local_18 = (local_18 - local_30) - (ulong)bVar2;
      }
    }
    local_38 = local_10;
    local_48 = local_20;
    for (local_54 = 0; local_54 < 0x13; local_54 = local_54 + 1) {
      local_40 = (undefined8 *)((long)local_40 + -1);
      *(char *)local_40 =
           (char)local_38 + ((char)(local_38 / 10 << 2) + (char)(local_38 / 10)) * -2 + '0';
      local_38 = local_38 / 10;
    }
  }
                    /* WARNING: Subroutine does not return */
  __assert_fail("__r1 < __d1",
                "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/factor.c",
                0x704,"print_uuint");
}

// Function: lbuf_putmpz
void lbuf_putmpz(undefined8 param_1) {
  long lVar1;
  undefined1 *puVar2;
  void *__ptr;
  long lVar3;
  
  lVar1 = mpz_sizeinbase(param_1,10);
  puVar2 = lbuf_buf + (long)lbuffered;
  if (lVar1 < (long)&lbuffered - (long)puVar2) {
    mpz_get_str(puVar2,10,param_1);
    lbuffered = puVar2 + (lVar1 - (ulong)(puVar2[lVar1 + -1] == '\0')) + -0x104dc0;
    while (0xfff < (long)lbuffered) {
      lbuf_half_flush();
    }
  }
  else {
    lbuf_flush();
    __ptr = (void *)ximalloc(lVar1 + 1);
    mpz_get_str(__ptr,10,param_1);
    lVar1 = lVar1 - (ulong)(*(char *)((long)__ptr + lVar1 + -1) == '\0');
    lVar3 = full_write(1,__ptr,lVar1);
    if (lVar1 != lVar3) {
      write_error();
    }
    free(__ptr);
  }
  return;
}

// Function: print_factors_single
void print_factors_single(undefined8 param_1,undefined8 param_2) {
  char cVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  undefined auVar4 [16];
  int local_130;
  int local_12c;
  undefined8 local_128 [28];
  byte abStack_48 [26];
  byte local_2e;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  print_uuint(param_1,param_2);
  lbuf_putc(0x3a);
  uVar2 = lo(param_1,param_2);
  uVar3 = hi(param_1,param_2);
  factor(local_128,uVar3,uVar2);
  local_130 = 0;
  do {
    if ((int)(uint)local_2e <= local_130) {
      cVar1 = hi_is_set(local_128);
      if (cVar1 != '\0') {
        lbuf_putc(0x20);
        print_uuint(local_128[0],local_128[1]);
      }
      lbuf_putnl();
      if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return;
    }
    for (local_12c = 0; local_12c < (int)(uint)abStack_48[local_130]; local_12c = local_12c + 1) {
      lbuf_putc(0x20);
      auVar4 = make_uuint(0,local_128[(long)local_130 + 2]);
      uVar2 = auVar4._0_8_;
      print_uuint(uVar2,auVar4._8_8_,uVar2,uVar2);
      if ((print_exponents != '\0') && (1 < abStack_48[local_130])) {
        lbuf_putc(0x5e);
        lbuf_putint(abStack_48[local_130]);
        break;
      }
    }
    local_130 = local_130 + 1;
  } while( true );
}

// Function: print_factors
undefined8 print_factors(char *param_1) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  long lVar5;
  long in_FS_OFFSET;
  char *local_88;
  long local_80;
  ulong local_78;
  long local_68;
  long local_60;
  undefined8 local_48;
  undefined8 local_40;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  for (local_88 = param_1; *local_88 == ' '; local_88 = local_88 + 1) {
  }
  cVar1 = *local_88;
  iVar2 = strtouuint(&local_48,local_88 + (cVar1 == '+'));
  if (iVar2 == 0) {
    lVar5 = hi(local_48,local_40);
    if (-1 < lVar5) {
      if (dev_debug != '\0') {
        fwrite("[using single-precision arithmetic] ",1,0x24,_stderr);
      }
      print_factors_single(local_48,local_40);
      uVar3 = 1;
      goto LAB_00104938;
    }
  }
  else if (iVar2 != 1) {
    uVar3 = quote(param_1);
    uVar4 = gettext("%s is not a valid positive integer");
    error(0,0,uVar4,uVar3);
    uVar3 = 0;
    goto LAB_00104938;
  }
  if (dev_debug != '\0') {
    fwrite("[using arbitrary-precision arithmetic] ",1,0x27,_stderr);
  }
  mpz_init_set_str(local_38,local_88 + (cVar1 == '+'),10);
  lVar5 = mp_size(local_38);
  if (0xccccccccccccccc < lVar5) {
    xalloc_die();
  }
  lbuf_putmpz(local_38);
  lbuf_putc(0x3a);
  mp_factor(&local_68,local_38);
  for (local_80 = 0; local_80 < local_60; local_80 = local_80 + 1) {
    local_78 = 0;
    while( true ) {
      if (*(ulong *)(local_80 * 0x18 + local_68 + 0x10) <= local_78) break;
      lbuf_putc(0x20);
      lbuf_putmpz(local_80 * 0x18 + local_68);
      if (print_exponents != '\0') {
        if (1 < *(ulong *)(local_80 * 0x18 + local_68 + 0x10)) {
          lbuf_putc(0x5e);
          lbuf_putbitcnt(*(undefined8 *)(local_80 * 0x18 + local_68 + 0x10));
          break;
        }
      }
      local_78 = local_78 + 1;
    }
  }
  mp_factor_clear(&local_68);
  mpz_clear(local_38);
  lbuf_putnl();
  uVar3 = 1;
LAB_00104938:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar3;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION] [NUMBER]...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Print the prime factors of each specified integer NUMBER.  If none\nare specified on the command line, read them from standard input.\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    fwrite_unlocked("  -h, --exponents   print repeated factors in form p^e unless e is 1\n",1,0x45,
                    _stdout);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info("factor");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: do_stdin
bool do_stdin(void) {
  byte bVar1;
  int iVar2;
  long lVar3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  bool local_51;
  undefined local_38 [8];
  void *local_30;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_51 = true;
  init_tokenbuffer(local_38);
  do {
    lVar3 = readtoken(_stdin,&DELIM,3,local_38);
    if (lVar3 == -1) {
      iVar2 = ferror_unlocked(_stdin);
      if (iVar2 == 0) {
        free(local_30);
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        return local_51;
      }
      uVar4 = gettext("error reading input");
      piVar5 = __errno_location();
      error(1,*piVar5,uVar4);
    }
    bVar1 = print_factors(local_30);
    local_51 = (bVar1 & local_51) != 0;
  } while( true );
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  byte local_21;
  int local_20;
  
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  do {
    while( true ) {
      while( true ) {
        iVar2 = getopt_long(param_1,param_2,&DAT_0010a325,long_options,0);
        if (iVar2 == -1) {
          atexit(lbuf_flush);
          if (_optind < param_1) {
            local_21 = 1;
            for (local_20 = _optind; local_20 < param_1; local_20 = local_20 + 1) {
              cVar1 = print_factors(param_2[local_20]);
              if (cVar1 != '\x01') {
                local_21 = 0;
              }
            }
          }
          else {
            local_21 = do_stdin();
          }
          return local_21 ^ 1;
        }
        if (iVar2 != 0x80) break;
        dev_debug = 1;
      }
      if (iVar2 < 0x81) break;
LAB_00104cc9:
      usage(1);
    }
    if (iVar2 != 0x68) {
      if (iVar2 < 0x69) {
        if (iVar2 == -0x83) {
LAB_00104c32:
          uVar3 = proper_name_lite("Niels Moller",&DAT_0010a2cc);
          uVar4 = proper_name_lite("Torbjorn Granlund",&DAT_0010a2e7);
          uVar5 = proper_name_lite("Paul Rubin","Paul Rubin");
          version_etc(_stdout,"factor","GNU coreutils",_Version,uVar5,uVar4,uVar3,0);
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        if (iVar2 == -0x82) {
          usage(0);
          goto LAB_00104c32;
        }
      }
      goto LAB_00104cc9;
    }
    print_exponents = 1;
  } while( true );
}

