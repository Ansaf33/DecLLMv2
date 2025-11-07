// Function: punch_hole
int punch_hole(int param_1,__off_t param_2,__off_t param_3) {
  char cVar1;
  int *piVar2;
  int local_c;
  
  local_c = fallocate(param_1,3,param_2,param_3);
  if (local_c < 0) {
    piVar2 = __errno_location();
    cVar1 = is_ENOTSUP(*piVar2);
    if ((cVar1 == '\0') && (piVar2 = __errno_location(), *piVar2 != 0x26)) {
      return local_c;
    }
    local_c = 0;
  }
  return local_c;
}

// Function: create_hole
__off_t create_hole(int param_1,undefined8 param_2,long param_3) {
  int iVar1;
  __off_t _Var2;
  undefined8 uVar3;
  undefined8 uVar4;
  int *piVar5;
  
  _Var2 = lseek(param_1,param_3,1);
  if (_Var2 < 0) {
    uVar3 = quotearg_style(4,param_2);
    uVar4 = gettext("cannot lseek %s");
    piVar5 = __errno_location();
    error(0,*piVar5,uVar4,uVar3);
    _Var2 = -1;
  }
  else {
    iVar1 = punch_hole(param_1,_Var2 - param_3,param_3);
    if (iVar1 < 0) {
      uVar3 = quotearg_style(4,param_2);
      uVar4 = gettext("error deallocating %s");
      piVar5 = __errno_location();
      error(0,*piVar5,uVar4,uVar3);
      _Var2 = -1;
    }
  }
  return _Var2;
}

// Function: is_CLONENOTSUP
undefined4 is_CLONENOTSUP(int param_1) {
  char cVar1;
  
  if ((((((param_1 != 0x26) && (param_1 != 0x19)) && (cVar1 = is_ENOTSUP(param_1), cVar1 == '\0'))
       && ((param_1 != 0x16 && (param_1 != 9)))) &&
      ((param_1 != 0x12 && ((param_1 != 0x1a && (param_1 != 1)))))) && (param_1 != 0xd)) {
    return 0;
  }
  return 1;
}

// Function: sparse_copy
long sparse_copy(int param_1,undefined4 param_2,void **param_3,size_t param_4,char param_5,
                undefined8 param_6,undefined8 param_7,size_t param_8,size_t *param_9,
                undefined4 *param_10) {
  long lVar1;
  char cVar2;
  bool bVar3;
  int iVar4;
  undefined8 uVar5;
  void *pvVar6;
  int *piVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  size_t sVar10;
  long lVar11;
  size_t sVar12;
  undefined4 uVar13;
  long in_FS_OFFSET;
  bool bVar14;
  char local_bc;
  size_t local_98;
  long local_90;
  long local_88;
  size_t local_80;
  size_t local_78;
  void *local_70;
  void *local_68;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_90 = 0;
  if (param_10[2] == 0) {
    if (param_9 == (size_t *)0x0) {
      uVar13 = 1;
    }
    else {
      uVar13 = 2;
    }
    param_10[2] = uVar13;
  }
  else if ((param_9 != (size_t *)0x0) && (param_10[2] == 3)) {
    param_10[2] = 4;
  }
  if ((param_9 == (size_t *)0x0) && (param_5 != '\0')) {
    for (; 0 < (long)param_8; param_8 = param_8 - local_88) {
      sVar10 = param_8;
      if (0x7fffffffbfffffff < (long)param_8) {
        sVar10 = 0x7fffffffc0000000;
      }
      local_88 = rpl_copy_file_range(param_1,0,param_2,0,sVar10,0);
      if (local_88 == 0) {
        if (local_90 != 0) {
          *param_10 = 2;
          goto LAB_00100af1;
        }
        break;
      }
      if (local_88 < 0) {
        *param_10 = 6;
        if (local_90 == 0) {
          piVar7 = __errno_location();
          cVar2 = is_CLONENOTSUP(*piVar7);
          if (cVar2 != '\0') break;
        }
        if ((local_90 == 0) && (piVar7 = __errno_location(), *piVar7 == 2)) break;
        piVar7 = __errno_location();
        if (*piVar7 != 4) {
          uVar8 = quotearg_n_style(1,4,param_7);
          uVar9 = quotearg_n_style(0,4,param_6);
          uVar5 = gettext("error copying %s to %s");
          piVar7 = __errno_location();
          error(0,*piVar7,uVar5,uVar9,uVar8);
          local_90 = -1;
          goto LAB_00100af1;
        }
        local_88 = 0;
      }
      *param_10 = 2;
      local_90 = local_90 + local_88;
    }
  }
  else {
    *param_10 = 5;
  }
  if (param_9 == (size_t *)0x0) {
    local_98 = 0;
  }
  else {
    local_98 = *param_9;
  }
  local_bc = local_98 != 0;
LAB_00100ab1:
  do {
    if ((long)param_8 < 1) {
LAB_00100abf:
      if (param_9 != (size_t *)0x0) {
        if (local_bc == '\0') {
          local_98 = 0;
        }
        *param_9 = local_98;
      }
      goto LAB_00100af1;
    }
    if (*param_3 == (void *)0x0) {
      iVar4 = getpagesize();
      pvVar6 = (void *)xalignalloc((long)iVar4,param_4);
      *param_3 = pvVar6;
    }
    local_70 = *param_3;
    sVar10 = param_8;
    if ((long)param_4 <= (long)param_8) {
      sVar10 = param_4;
    }
    local_80 = read(param_1,local_70,sVar10);
    if ((long)local_80 < 0) break;
    if (local_80 == 0) goto LAB_00100abf;
    param_8 = param_8 - local_80;
    local_90 = local_90 + local_80;
    local_78 = param_4;
    local_68 = local_70;
    if (param_9 != (size_t *)0x0) {
      local_78 = 0x200;
    }
    for (; cVar2 = local_bc, local_80 != 0; local_80 = local_80 - sVar10) {
      sVar10 = local_80;
      if ((long)local_78 <= (long)local_80) {
        sVar10 = local_78;
      }
      if (param_9 != (size_t *)0x0) {
        local_bc = is_nul(local_70,sVar10);
      }
      if ((local_bc == cVar2) || (local_98 == 0)) {
        bVar14 = false;
      }
      else {
        bVar14 = true;
      }
      if ((local_80 == sVar10) && (local_bc != '\x01')) {
        bVar3 = true;
      }
      else {
        bVar3 = false;
      }
      if ((bVar14) || (bVar3)) {
        if (bVar14) {
          if (cVar2 != '\0') {
            lVar11 = create_hole(param_2,param_7,local_98);
            if (lVar11 < 0) {
              local_90 = 0;
              goto LAB_00100af1;
            }
            local_68 = local_70;
            local_98 = sVar10;
          }
        }
        else {
          local_98 = sVar10 + local_98;
        }
        if ((cVar2 != '\x01') || ((bVar14 && (bVar3)))) {
          sVar12 = full_write(param_2,local_68,local_98);
          if (sVar12 != local_98) {
            uVar8 = quotearg_style(4,param_7);
            uVar9 = gettext("error writing %s");
            piVar7 = __errno_location();
            error(0,*piVar7,uVar9,uVar8);
            local_90 = -1;
            goto LAB_00100af1;
          }
          if ((cVar2 == '\x01') || (local_98 = sVar10, !bVar14)) {
            local_98 = 0;
          }
        }
      }
      else {
        bVar14 = SCARRY8(sVar10,local_98);
        local_98 = sVar10 + local_98;
        if (bVar14) {
          uVar8 = quotearg_style(4,param_6);
          uVar9 = gettext("overflow reading %s");
          error(0,0,uVar9,uVar8);
          local_90 = -1;
          goto LAB_00100af1;
        }
      }
      local_70 = (void *)((long)local_70 + sVar10);
      local_78 = sVar10;
    }
  } while( true );
  piVar7 = __errno_location();
  if (*piVar7 != 4) {
    uVar8 = quotearg_style(4,param_6);
    uVar9 = gettext("error reading %s");
    piVar7 = __errno_location();
    error(0,*piVar7,uVar9,uVar8);
    local_90 = -1;
LAB_00100af1:
    if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    return local_90;
  }
  goto LAB_00100ab1;
}

// Function: write_zeros
undefined8 write_zeros(undefined4 param_1,size_t param_2,void **param_3,size_t param_4) {
  int iVar1;
  size_t __n;
  void *pvVar2;
  size_t sVar3;
  size_t local_28;
  void *local_18;
  
  local_18 = (void *)0x0;
  local_28 = param_2;
  while( true ) {
    if (local_28 == 0) {
      return 1;
    }
    __n = param_4;
    if ((long)local_28 <= (long)param_4) {
      __n = local_28;
    }
    if (local_18 == (void *)0x0) {
      if (*param_3 == (void *)0x0) {
        iVar1 = getpagesize();
        pvVar2 = (void *)xalignalloc((long)iVar1,param_4);
        *param_3 = pvVar2;
      }
      local_18 = memset(*param_3,0,__n);
    }
    sVar3 = full_write(param_1,local_18,__n);
    if (__n != sVar3) break;
    local_28 = local_28 - __n;
  }
  return 0;
}

// Function: lseek_copy
long lseek_copy(int param_1,undefined4 param_2,undefined8 param_3,undefined8 param_4,long param_5,
               long param_6,long *param_7,long param_8,int param_9,undefined param_10,
               undefined8 param_11,undefined8 param_12,long *param_13,long param_14) {
  long lVar1;
  char cVar2;
  long lVar3;
  __off_t _Var4;
  long lVar5;
  int *piVar6;
  long lVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  long *plVar10;
  long in_FS_OFFSET;
  long local_78;
  long local_70;
  long local_68;
  long local_60;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_70 = 0;
  lVar3 = param_6 + param_5;
  if (SCARRY8(param_6,param_5)) {
    lVar3 = 0x7fffffffffffffff;
  }
  lVar7 = lVar3;
  if (param_8 <= lVar3) {
    lVar7 = param_8;
  }
  *(undefined4 *)(param_14 + 8) = 3;
  local_68 = *param_7;
  param_8 = lVar7;
  local_78 = param_5;
  do {
    lVar7 = param_8;
    if ((local_68 < 0) || (lVar3 <= local_68)) {
LAB_00100ff2:
      param_8 = lVar7;
      *param_13 = *param_13 + (param_8 - (local_70 + local_78));
      lVar7 = param_8 - param_5;
      goto LAB_00101092;
    }
    if (local_68 == 0) {
      lVar7 = param_7[1];
    }
    else {
      lVar7 = lseek(param_1,local_68,4);
    }
    if (lVar7 < 0) {
      piVar6 = __errno_location();
      if (*piVar6 != 6) break;
      local_60 = param_8;
      if (param_8 <= local_68) {
        _Var4 = lseek(param_1,0,2);
        if (_Var4 < 0) break;
        param_8 = lVar3;
        if (_Var4 <= lVar3) {
          param_8 = _Var4;
        }
        lVar7 = param_8;
        local_60 = param_8;
        if (param_8 <= local_68) goto LAB_00100ff2;
      }
    }
    else {
      local_60 = lVar3;
      if (lVar7 <= lVar3) {
        local_60 = lVar7;
      }
    }
    if (param_8 < local_60) {
      param_8 = local_60;
    }
    _Var4 = lseek(param_1,local_68,0);
    if (_Var4 < 0) break;
    local_70 = (local_68 - local_78) - local_70;
    if (local_70 != 0) {
      if (param_9 == 3) {
        *param_13 = *param_13 + local_70;
      }
      else if (param_9 == 1) {
        cVar2 = write_zeros(param_2,local_70,param_3,param_4);
        if (cVar2 != '\x01') {
          uVar8 = quotearg_n_style_colon(0,3,param_12);
          uVar9 = gettext("%s: write failed");
          piVar6 = __errno_location();
          error(0,*piVar6,uVar9,uVar8);
          lVar7 = -1;
          goto LAB_00101092;
        }
      }
      else {
        lVar7 = create_hole(param_2,param_12,local_70);
        if (lVar7 < 0) goto LAB_00101092;
      }
    }
    local_70 = local_60 - local_68;
    local_78 = local_68;
    plVar10 = param_13;
    if (param_9 != 3) {
      plVar10 = (long *)0x0;
    }
    lVar5 = sparse_copy(param_1,param_2,param_3,param_4,param_10,param_11,param_12,local_70,plVar10,
                        param_14);
    if (lVar5 < 0) {
      lVar7 = -1;
      goto LAB_00101092;
    }
    lVar7 = lVar5 + local_68;
    if (lVar5 < local_70) goto LAB_00100ff2;
    local_68 = lseek(param_1,lVar5 + local_68,3);
  } while ((-1 < local_68) || (piVar6 = __errno_location(), *piVar6 == 6));
  uVar8 = quotearg_style(4,param_11);
  uVar9 = gettext("cannot lseek %s");
  piVar6 = __errno_location();
  error(0,*piVar6,uVar9,uVar8);
  lVar7 = -1;
LAB_00101092:
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return lVar7;
}

// Function: infer_scantype
undefined8 infer_scantype(int param_1,long param_2,long param_3,long *param_4) {
  char cVar1;
  long lVar2;
  __off_t _Var3;
  int *piVar4;
  
  if ((*(uint *)(param_2 + 0x18) & 0xf000) == 0x8000) {
    lVar2 = *(long *)(param_2 + 0x30);
    if (lVar2 < 0) {
      lVar2 = lVar2 + 0x1ff;
    }
    if (*(long *)(param_2 + 0x40) < lVar2 >> 9) {
      _Var3 = lseek(param_1,param_3,3);
      *param_4 = _Var3;
      if (param_3 == *param_4) {
        _Var3 = lseek(param_1,param_3,4);
        param_4[1] = _Var3;
        if (-1 < param_4[1]) {
          if (param_4[1] < *(long *)(param_2 + 0x30)) {
            return 3;
          }
          _Var3 = lseek(param_1,param_3,0);
          if (_Var3 < 0) {
            return 0;
          }
          return 1;
        }
      }
      else {
        if ((param_3 < *param_4) || (piVar4 = __errno_location(), *piVar4 == 6)) {
          param_4[1] = 0;
          return 3;
        }
        piVar4 = __errno_location();
        if (*piVar4 != 0x16) {
          piVar4 = __errno_location();
          cVar1 = is_ENOTSUP(*piVar4);
          if (cVar1 != '\x01') {
            return 0;
          }
        }
      }
      return 2;
    }
  }
  return 1;
}

// Function: copy_file_data
long copy_file_data(undefined4 param_1,long param_2,long param_3,undefined8 param_4,int param_5,
                   long param_6,long param_7,undefined8 param_8,long param_9,long param_10,
                   undefined8 param_11) {
  bool bVar1;
  byte bVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  long lVar7;
  undefined4 uVar8;
  long *plVar9;
  long in_FS_OFFSET;
  long local_88;
  undefined8 local_80;
  long local_78;
  long local_70;
  long local_68;
  undefined8 local_60;
  long local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_88 = io_blksize(param_6);
  iVar3 = infer_scantype(param_1,param_2,param_3,local_38);
  if (iVar3 == 0) {
    uVar4 = quotearg_style(4,param_4);
    uVar5 = gettext("cannot lseek %s");
    piVar6 = __errno_location();
    local_50 = uVar5;
    error(0,*piVar6,uVar5,uVar4);
    lVar7 = -1;
  }
  else {
    if (((*(uint *)(param_6 + 0x18) & 0xf000) == 0x8000) &&
       ((*(int *)(param_10 + 0xc) == 3 || ((*(int *)(param_10 + 0xc) == 2 && (iVar3 != 1)))))) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (((param_3 == 0) && (param_9 == 0x7fffffffffffffff)) &&
       ((*(int *)(param_10 + 0x4c) != 1 || (*(int *)(param_10 + 0xc) != 2)))) {
      fdadvise(param_1,0,0,2);
    }
    if (!bVar1) {
      local_60 = 0x7ffffffffffffffe;
      uVar4 = io_blksize(param_2);
      local_58 = buffer_lcm(uVar4,local_88,0x7ffffffffffffffe);
      if ((((*(uint *)(param_2 + 0x18) & 0xf000) == 0x8000) && (-1 < *(long *)(param_2 + 0x30))) &&
         (*(long *)(param_2 + 0x30) < local_88)) {
        local_88 = *(long *)(param_2 + 0x30) + 1;
      }
      lVar7 = local_58 + -1 + local_88;
      if (SCARRY8(local_58 + -1,local_88)) {
        lVar7 = 0x7fffffffffffffff;
      }
      local_88 = lVar7 - lVar7 % local_58;
    }
    local_80 = 0;
    local_78 = 0;
    if (iVar3 == 3) {
      if (bVar1) {
        uVar8 = *(undefined4 *)(param_10 + 0xc);
      }
      else {
        uVar8 = 1;
      }
      local_68 = lseek_copy(param_1,param_5,&local_80,local_88,param_3,param_9,local_38,
                            *(undefined8 *)(param_2 + 0x30),uVar8,*(int *)(param_10 + 0x4c) != 0,
                            param_4,param_8,&local_78,param_11);
    }
    else {
      if (bVar1) {
        plVar9 = &local_78;
      }
      else {
        plVar9 = (long *)0x0;
      }
      local_68 = sparse_copy(param_1,param_5,&local_80,local_88,*(int *)(param_10 + 0x4c) != 0,
                             param_4,param_8,param_9,plVar9,param_11);
    }
    if ((-1 < local_68) && (0 < local_78)) {
      local_70 = local_68 + param_7;
      if (SCARRY8(local_68,param_7)) {
        piVar6 = __errno_location();
        *piVar6 = 0x4b;
        bVar2 = 1;
      }
      else if (bVar1) {
        iVar3 = ftruncate(param_5,local_70);
        bVar2 = (byte)((uint)iVar3 >> 0x1f);
      }
      else {
        bVar2 = write_zeros(param_5,local_78,&local_80,local_88);
        bVar2 = bVar2 ^ 1;
      }
      if (bVar2 == 0) {
        if ((bVar1) && (iVar3 = punch_hole(param_5,local_70 - local_78,local_78), iVar3 < 0)) {
          uVar4 = quotearg_style(4,param_8);
          uVar5 = gettext("error deallocating %s");
          piVar6 = __errno_location();
          local_40 = uVar5;
          error(0,*piVar6,uVar5,uVar4);
          local_68 = -1;
        }
      }
      else {
        uVar4 = quotearg_style(4,param_8);
        uVar5 = gettext("failed to extend %s");
        piVar6 = __errno_location();
        local_48 = uVar5;
        error(0,*piVar6,uVar5,uVar4);
        local_68 = -1;
      }
    }
    alignfree(local_80);
    lVar7 = local_68;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return lVar7;
}

