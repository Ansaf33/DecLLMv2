// Function: copy_debug_string
void copy_debug_string(uint param_1) {
                    /* WARNING: Could not recover jumptable at 0x001000fb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(&DAT_00106844 + *(int *)(&DAT_00106844 + (ulong)param_1 * 4)))();
  return;
}

// Function: copy_debug_sparse_string
void copy_debug_sparse_string(uint param_1) {
                    /* WARNING: Could not recover jumptable at 0x00100157. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(&DAT_00106884 + *(int *)(&DAT_00106884 + (ulong)param_1 * 4)))();
  return;
}

// Function: emit_debug
void emit_debug(long param_1) {
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  
  if (((*(char *)(param_1 + 0x17) != '\x01') && (*(char *)(param_1 + 0x3a) != '\x01')) &&
     (*(char *)(param_1 + 0x31) != '\0')) {
    uVar1 = copy_debug_sparse_string(_DAT_00106808);
    uVar2 = copy_debug_string(_DAT_00106804);
    uVar3 = copy_debug_string(copy_debug);
    printf("copy offload: %s, reflink: %s, sparse detection: %s\n",uVar3,uVar2,uVar1);
  }
  return;
}

// Function: follow_fstatat
int follow_fstatat(int param_1,char *param_2,stat *param_3,int param_4) {
  long lVar1;
  int iVar2;
  long in_FS_OFFSET;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  iVar2 = fstatat(param_1,param_2,param_3,param_4);
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return iVar2;
}

// Function: is_terminal_error
undefined4 is_terminal_error(int param_1) {
  undefined4 uVar1;
  
  if ((((param_1 == 5) || (param_1 == 0xc)) || (param_1 == 0x1c)) || (param_1 == 0x7a)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: clone_file
void clone_file(int param_1,uint param_2) {
  ioctl(param_1,0x40049409,(ulong)param_2);
  return;
}

// Function: is_ancestor
undefined8 is_ancestor(ulong *param_1,undefined8 *param_2) {
  undefined8 *local_18;
  
  local_18 = param_2;
  while( true ) {
    if (local_18 == (undefined8 *)0x0) {
      return 0;
    }
    if ((param_1[1] ^ local_18[1] | local_18[2] ^ *param_1) == 0) break;
    local_18 = (undefined8 *)*local_18;
  }
  return 1;
}

// Function: errno_unsupported
undefined4 errno_unsupported(int param_1) {
  undefined4 uVar1;
  
  if ((param_1 == 0x5f) || (param_1 == 0x3d)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: copy_attr
undefined8 copy_attr(void) {
  return 1;
}

// Function: copy_dir
byte copy_dir(undefined8 param_1,long param_2,undefined4 param_3,long param_4,undefined param_5,
             undefined8 param_6,undefined8 param_7,undefined8 *param_8,byte *param_9,byte *param_10) {
  byte bVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  size_t sVar5;
  long in_FS_OFFSET;
  byte local_bd;
  byte local_bc;
  undefined local_bb;
  byte local_ba;
  byte local_b9;
  undefined4 local_b8;
  int local_b4;
  char *local_b0;
  char *local_a8;
  void *local_a0;
  void *local_98;
  undefined8 local_90;
  undefined8 local_88;
  undefined8 local_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_88 = *param_8;
  local_80 = param_8[1];
  local_78 = param_8[2];
  local_70 = param_8[3];
  local_68 = param_8[4];
  local_60 = param_8[5];
  local_58 = param_8[6];
  local_50 = param_8[7];
  local_48 = param_8[8];
  local_40 = param_8[9];
  local_30 = param_8[0xb];
  local_38 = param_8[10];
  local_ba = 1;
  local_a8 = (char *)savedir(param_1,2);
  if (local_a8 == (char *)0x0) {
    uVar2 = quotearg_style(4,param_1);
    uVar3 = gettext("cannot access %s");
    piVar4 = __errno_location();
    local_b4 = *piVar4;
    local_b8 = 0;
    local_90 = uVar3;
    error(0,local_b4,uVar3,uVar2);
    bVar1 = 0;
  }
  else {
    if (*(int *)((long)param_8 + 4) == 3) {
      local_88 = CONCAT44(2,(undefined4)local_88);
    }
    local_b9 = 0;
    for (local_b0 = local_a8; *local_b0 != '\0'; local_b0 = local_b0 + sVar5 + 1) {
      local_a0 = (void *)file_name_concat(param_1,local_b0,0);
      local_98 = (void *)file_name_concat(param_2,local_b0,0);
      local_bc = *param_9;
      bVar1 = copy_internal(local_a0,local_98,param_3,(param_4 - param_2) + (long)local_98,param_5,
                            param_6,param_7,&local_88,0,&local_bc,&local_bd,&local_bb);
      local_ba = (bVar1 & local_ba) != 0;
      *param_10 = (local_bd | *param_10) != 0;
      free(local_98);
      free(local_a0);
      if (local_bd != 0) break;
      local_b9 = (local_bc | local_b9) != 0;
      sVar5 = strlen(local_b0);
    }
    free(local_a8);
    *param_9 = local_b9;
    bVar1 = local_ba;
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return bVar1;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: fchmod_or_lchmod
void fchmod_or_lchmod(int param_1,undefined4 param_2,undefined8 param_3,__mode_t param_4) {
  if (param_1 < 0) {
    lchmodat(param_2,param_3,param_4);
  }
  else {
    fchmod(param_1,param_4);
  }
  return;
}

// Function: fchown_or_lchown
void fchown_or_lchown(int param_1,undefined4 param_2,undefined8 param_3,__uid_t param_4,
                     __gid_t param_5) {
  if (param_1 < 0) {
    lchownat(param_2,param_3,param_4,param_5);
  }
  else {
    fchown(param_1,param_4,param_5);
  }
  return;
}

// Function: set_owner
int set_owner(long param_1,undefined8 param_2,undefined4 param_3,undefined8 param_4,
             undefined4 param_5,long param_6,char param_7,long param_8) {
  undefined4 uVar1;
  undefined4 uVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  int *piVar8;
  
  uVar1 = *(undefined4 *)(param_6 + 0x1c);
  uVar2 = *(undefined4 *)(param_6 + 0x20);
  if ((param_7 != '\x01') &&
     (((*(char *)(param_1 + 0x20) != '\0' || (*(char *)(param_1 + 0x18) != '\0')) ||
      (*(char *)(param_1 + 0x39) != '\0')))) {
    if ((*(char *)(param_1 + 0x20) == '\0') && (*(char *)(param_1 + 0x18) == '\0')) {
      uVar4 = *(uint *)(param_1 + 0x10);
    }
    else {
      uVar4 = *(uint *)(param_6 + 0x18);
    }
    if ((((uint)(ushort)(~(ushort)uVar4 | 0xe00) & *(uint *)(param_8 + 0x18) & 0xfff) != 0) &&
       (iVar5 = qset_acl(param_2,param_5,*(uint *)(param_8 + 0x18) & uVar4 & 0x1c0), iVar5 != 0)) {
      cVar3 = owner_failure_ok(param_1);
      if (cVar3 != '\x01') {
        uVar6 = quotearg_style(4,param_2);
        uVar7 = gettext("clearing permissions for %s");
        piVar8 = __errno_location();
        error(0,*piVar8,uVar7,uVar6);
      }
      return -(uint)*(byte *)(param_1 + 0x32);
    }
  }
  iVar5 = fchown_or_lchown(param_5,param_3,param_4,uVar1,uVar2);
  if (iVar5 == 0) {
    iVar5 = 1;
  }
  else {
    cVar3 = chown_failure_ok(param_1);
    if (cVar3 == '\0') {
      uVar6 = quotearg_style(4,param_2);
      uVar7 = gettext("failed to preserve ownership for %s");
      piVar8 = __errno_location();
      error(0,*piVar8,uVar7,uVar6);
      if (*(char *)(param_1 + 0x32) != '\0') {
        return -1;
      }
    }
    else {
      fchown_or_lchown(param_5,param_3,param_4,0xffffffff,uVar2);
    }
    iVar5 = 0;
  }
  return iVar5;
}

// Function: set_author
void set_author(void) {
  return;
}

// Function: set_process_security_ctx
undefined8
set_process_security_ctx
          (undefined8 param_1,undefined8 param_2,undefined4 param_3,char param_4,long param_5) {
  bool bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  int *piVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (*(char *)(param_5 + 0x33) == '\0') {
    if ((*(long *)(param_5 + 0x28) != 0) && (param_4 != '\0')) {
      iVar4 = defaultcon(*(undefined8 *)(param_5 + 0x28),param_2,param_3);
      if (iVar4 < 0) {
        piVar5 = __errno_location();
        cVar3 = ignorable_ctx_err(*piVar5);
        if (cVar3 != '\x01') {
          uVar6 = quotearg_style(4,param_2);
          uVar7 = gettext("failed to set default file creation context for %s");
          piVar5 = __errno_location();
          local_28 = uVar7;
          error(0,*piVar5,uVar7,uVar6);
        }
      }
    }
  }
  else {
    if ((*(char *)(param_5 + 0x31) == '\x01') && (*(char *)(param_5 + 0x34) == '\0')) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if ((bVar1) || (*(char *)(param_5 + 0x37) == '\x01')) {
      bVar2 = false;
    }
    else {
      bVar2 = true;
    }
    iVar4 = lgetfilecon_raw(param_1,&local_40);
    if (iVar4 < 0) {
      if (bVar1) {
LAB_00100b6f:
        uVar6 = quotearg_style(4,param_1);
        uVar7 = gettext("failed to get security context of %s");
        piVar5 = __errno_location();
        local_30 = uVar7;
        error(0,*piVar5,uVar7,uVar6);
      }
      else if (bVar2) {
        piVar5 = __errno_location();
        cVar3 = errno_unsupported(*piVar5);
        if (cVar3 != '\x01') goto LAB_00100b6f;
      }
      if (*(char *)(param_5 + 0x34) != '\0') {
        uVar6 = 0;
        goto LAB_00100c86;
      }
    }
    else {
      iVar4 = setfscreatecon_raw(local_40);
      if (iVar4 < 0) {
        if (bVar1) {
LAB_00100acb:
          uVar6 = quote(local_40);
          uVar7 = gettext("failed to set default file creation context to %s");
          piVar5 = __errno_location();
          local_38 = uVar7;
          error(0,*piVar5,uVar7,uVar6);
        }
        else if (bVar2) {
          piVar5 = __errno_location();
          cVar3 = errno_unsupported(*piVar5);
          if (cVar3 != '\x01') goto LAB_00100acb;
        }
        if (*(char *)(param_5 + 0x34) != '\0') {
          freecon(local_40);
          uVar6 = 0;
          goto LAB_00100c86;
        }
      }
      freecon(local_40);
    }
  }
  uVar6 = 1;
LAB_00100c86:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar6;
}

// Function: set_file_security_ctx
undefined8 set_file_security_ctx(undefined8 param_1,undefined param_2,long param_3) {
  bool bVar1;
  bool bVar2;
  char cVar3;
  int *piVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  
  if ((*(char *)(param_3 + 0x31) == '\x01') && (*(char *)(param_3 + 0x34) == '\0')) {
    bVar1 = false;
  }
  else {
    bVar1 = true;
  }
  if ((bVar1) || (*(char *)(param_3 + 0x37) == '\x01')) {
    bVar2 = false;
  }
  else {
    bVar2 = true;
  }
  cVar3 = restorecon(*(undefined8 *)(param_3 + 0x28),param_1,param_2);
  if (cVar3 == '\x01') {
    return 1;
  }
  if (!bVar1) {
    if (!bVar2) {
      return 0;
    }
    piVar4 = __errno_location();
    cVar3 = errno_unsupported(*piVar4);
    if (cVar3 == '\x01') {
      return 0;
    }
  }
  uVar5 = quotearg_n_style(0,4,param_1);
  uVar6 = gettext("failed to set the security context of %s");
  piVar4 = __errno_location();
  error(0,*piVar4,uVar6,uVar5);
  return 0;
}

// Function: handle_clone_fail
undefined8
handle_clone_fail(int param_1,char *param_2,undefined8 param_3,undefined8 param_4,int param_5,
                 char param_6,int param_7) {
  char cVar1;
  int iVar2;
  int *piVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  __off_t _Var7;
  
  piVar3 = __errno_location();
  cVar1 = is_terminal_error(*piVar3);
  if ((param_7 == 2) || (cVar1 != '\0')) {
    uVar4 = quotearg_n_style(1,4,param_3);
    uVar5 = quotearg_n_style(0,4,param_4);
    uVar6 = gettext("failed to clone %s from %s");
    piVar3 = __errno_location();
    error(0,*piVar3,uVar6,uVar5,uVar4);
  }
  if ((param_6 != '\0') && (param_7 == 2)) {
    if (cVar1 == '\x01') {
      _Var7 = lseek(param_5,0,2);
      if (_Var7 != 0) goto LAB_00100f40;
    }
    iVar2 = unlinkat(param_1,param_2,0);
    if (iVar2 != 0) {
      piVar3 = __errno_location();
      if (*piVar3 != 2) {
        uVar4 = quotearg_style(4,param_4);
        uVar5 = gettext("cannot remove %s");
        piVar3 = __errno_location();
        error(0,*piVar3,uVar5,uVar4);
      }
    }
  }
LAB_00100f40:
  if (cVar1 != '\x01') {
    _DAT_00106804 = 6;
  }
  if ((param_7 == 2) || (cVar1 != '\0')) {
    uVar4 = 0;
  }
  else {
    uVar4 = 1;
  }
  return uVar4;
}

// Function: copy_reg
undefined copy_reg(undefined8 param_1,char *param_2,int param_3,char *param_4,long param_5,
                  uint param_6,uint param_7,char *param_8,__dev_t *param_9) {
  char cVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  undefined8 uVar6;
  int *piVar7;
  char *__format;
  size_t sVar8;
  long lVar9;
  undefined8 uVar10;
  long in_FS_OFFSET;
  undefined local_23f;
  byte local_23e;
  int local_23c;
  int local_238;
  uint local_234;
  uint local_230;
  undefined local_168 [16];
  undefined local_158 [16];
  stat local_148;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_23e = *(byte *)(param_5 + 0x31);
  copy_debug = 0;
  _DAT_00106804 = (uint)(*(int *)(param_5 + 0x4c) == 0);
  _DAT_00106808 = 0;
  if (*(int *)(param_5 + 4) == 2) {
    uVar10 = 0x20000;
  }
  else {
    uVar10 = 0;
  }
  iVar2 = open_safer(param_1,uVar10);
  if (iVar2 < 0) {
    uVar10 = quotearg_style(4,param_1);
    uVar6 = gettext("cannot open %s for reading");
    piVar7 = __errno_location();
    error(0,*piVar7,uVar6,uVar10);
    local_23f = 0;
    goto LAB_00101f14;
  }
  iVar3 = fstat(iVar2,&local_b8);
  if (iVar3 == 0) {
    cVar1 = psame_inode(param_9,&local_b8);
    if (cVar1 == '\x01') {
      *param_9 = local_b8.st_dev;
      param_9[1] = local_b8.st_ino;
      param_9[2] = local_b8.st_nlink;
      param_9[3] = local_b8._24_8_;
      param_9[4] = local_b8._32_8_;
      param_9[5] = local_b8.st_rdev;
      param_9[6] = local_b8.st_size;
      param_9[7] = local_b8.st_blksize;
      param_9[8] = local_b8.st_blocks;
      param_9[9] = local_b8.st_atim.tv_sec;
      param_9[10] = local_b8.st_atim.tv_nsec;
      param_9[0xb] = local_b8.st_mtim.tv_sec;
      param_9[0xc] = local_b8.st_mtim.tv_nsec;
      param_9[0xd] = local_b8.st_ctim.tv_sec;
      param_9[0xe] = local_b8.st_ctim.tv_nsec;
      param_9[0xf] = local_b8.__unused[0];
      param_9[0x10] = local_b8.__unused[1];
      param_9[0x11] = local_b8.__unused[2];
      local_230 = *(uint *)(param_9 + 3);
      if (*param_8 == '\x01') {
LAB_00101511:
        if (*param_8 == '\0') {
          local_234 = 0;
          param_7 = 0;
        }
        else {
          uVar5 = ~param_7 & param_6;
          local_234 = ~param_6 & uVar5;
          local_23c = openat_safer(param_3,param_4,0xc1,uVar5);
          piVar7 = __errno_location();
          local_238 = *piVar7;
          if (((local_23c < 0) && (local_238 == 0x11)) &&
             ((*(char *)(param_5 + 0x18) != '\x01' &&
              (iVar3 = issymlinkat(param_3,param_4), iVar3 == 1)))) {
            if (*(char *)(param_5 + 0x44) == '\0') {
              uVar10 = quotearg_style(4,param_2);
              uVar6 = gettext("not writing through dangling symlink %s");
              error(0,0,uVar6,uVar10);
              local_23f = 0;
              goto LAB_00101e6d;
            }
            local_23c = openat_safer(param_3,param_4,0x41,uVar5);
            piVar7 = __errno_location();
            local_238 = *piVar7;
          }
          if ((((local_23c < 0) && (local_238 == 0x15)) && (*param_2 != '\0')) &&
             (sVar8 = strlen(param_2), param_2[sVar8 - 1] == '/')) {
            local_238 = 0x14;
          }
        }
        if (local_23c < 0) {
          uVar10 = quotearg_style(4,param_2);
          uVar6 = gettext("cannot create regular file %s");
          error(0,local_238,uVar6,uVar10);
          local_23f = 0;
        }
        else {
          if ((local_23e != 0) && (*(int *)(param_5 + 0x4c) != 0)) {
            iVar3 = clone_file(local_23c,iVar2);
            if (iVar3 == 0) {
              local_23e = 0;
              _DAT_00106804 = 2;
            }
            else {
              cVar1 = handle_clone_fail(param_3,param_4,param_1,param_2,local_23c,*param_8,
                                        *(undefined4 *)(param_5 + 0x4c));
              if (cVar1 != '\x01') {
                local_23f = 0;
                goto LAB_00101de5;
              }
            }
          }
          if (((*(byte *)(param_5 + 0x1f) | local_23e) | local_234) == 0) {
            local_148.st_mode = 0;
          }
          else {
            iVar3 = fstat(local_23c,&local_148);
            if (iVar3 != 0) {
              uVar10 = quotearg_style(4,param_2);
              uVar6 = gettext("cannot fstat %s");
              piVar7 = __errno_location();
              error(0,*piVar7,uVar6,uVar10);
              local_23f = 0;
              goto LAB_00101de5;
            }
          }
          if (((local_148.st_mode | local_234) != local_148.st_mode) &&
             (iVar3 = fchmod_or_lchmod(local_23c,param_3,param_4,local_148.st_mode | local_234),
             iVar3 != 0)) {
            local_234 = 0;
          }
          if (local_23e != 0) {
            lVar9 = copy_file_data(iVar2,&local_b8,0,param_1,local_23c,&local_148,0,param_2,
                                   0x7fffffffffffffff,param_5,&copy_debug);
            if (lVar9 < 0) {
              local_23f = 0;
              goto LAB_00101de5;
            }
          }
          if (*(char *)(param_5 + 0x21) != '\0') {
            local_168 = get_stat_atime(param_9);
            local_158 = get_stat_mtime(param_9);
            iVar3 = fdutimensat(local_23c,param_3,param_4,local_168,0);
            if (iVar3 != 0) {
              uVar10 = quotearg_style(4,param_2);
              uVar6 = gettext("preserving times for %s");
              piVar7 = __errno_location();
              error(0,*piVar7,uVar6,uVar10);
              if (*(char *)(param_5 + 0x32) != '\0') {
                local_23f = 0;
                goto LAB_00101de5;
              }
            }
          }
          if ((*(char *)(param_5 + 0x1f) != '\0') &&
             ((*(__uid_t *)((long)param_9 + 0x1c) != local_148.st_uid ||
              (*(__gid_t *)(param_9 + 4) != local_148.st_gid)))) {
            iVar3 = set_owner(param_5,param_2,param_3,param_4,local_23c,param_9,*param_8,&local_148)
            ;
            if (iVar3 == -1) {
              local_23f = 0;
              goto LAB_00101de5;
            }
            if (iVar3 == 0) {
              local_230 = local_230 & 0xfffff1ff;
            }
          }
          local_23f = 1;
          set_author(param_2,local_23c,param_9);
          if ((*(char *)(param_5 + 0x20) == '\0') && (*(char *)(param_5 + 0x18) == '\0')) {
            if (*(char *)(param_5 + 0x39) == '\0') {
              if ((*(char *)(param_5 + 0x22) == '\0') || (*param_8 == '\0')) {
                if ((param_7 | local_234) != 0) {
                  uVar5 = cached_umask();
                  if ((param_7 & ~uVar5 | local_234) != 0) {
                    uVar5 = cached_umask();
                    iVar3 = fchmod_or_lchmod(local_23c,param_3,param_4,~uVar5 & param_6);
                    if (iVar3 != 0) {
                      uVar10 = quotearg_style(4,param_2);
                      uVar6 = gettext("preserving permissions for %s");
                      piVar7 = __errno_location();
                      error(0,*piVar7,uVar6,uVar10);
                      if (*(char *)(param_5 + 0x32) != '\0') {
                        local_23f = 0;
                      }
                    }
                  }
                }
              }
              else {
                uVar5 = cached_umask();
                iVar3 = xset_acl(param_2,local_23c,~uVar5 & 0x1b6);
                if (iVar3 != 0) {
                  local_23f = 0;
                }
              }
            }
            else {
              iVar3 = xset_acl(param_2,local_23c,*(undefined4 *)(param_5 + 0x10));
              if (iVar3 != 0) {
                local_23f = 0;
              }
            }
          }
          else {
            iVar3 = xcopy_acl(param_1,iVar2,param_2,local_23c,local_230);
            if ((iVar3 != 0) && (*(char *)(param_5 + 0x32) != '\0')) {
              local_23f = 0;
            }
          }
          if (-1 < local_23c) goto LAB_00101de5;
        }
      }
      else {
        if (local_23e == 0) {
          uVar4 = 1;
        }
        else {
          uVar4 = 0x201;
        }
        local_23c = openat_safer(param_3,param_4,uVar4);
        piVar7 = __errno_location();
        local_238 = *piVar7;
        if ((local_23c < 0) ||
           ((*(long *)(param_5 + 0x28) == 0 && (*(char *)(param_5 + 0x33) == '\0')))) {
LAB_00101380:
          if (((local_23c < 0) && (local_238 != 2)) && (*(char *)(param_5 + 0x16) != '\0')) {
            iVar3 = unlinkat(param_3,param_4,0);
            if (iVar3 == 0) {
              if (*(char *)(param_5 + 0x40) != '\0') {
                uVar10 = quotearg_style(4,param_2);
                __format = (char *)gettext("removed %s\n");
                printf(__format,uVar10);
              }
            }
            else {
              piVar7 = __errno_location();
              if (*piVar7 != 2) {
                uVar10 = quotearg_style(4,param_2);
                uVar6 = gettext("cannot remove %s");
                piVar7 = __errno_location();
                error(0,*piVar7,uVar6,uVar10);
                local_23f = 0;
                goto LAB_00101e6d;
              }
            }
            local_238 = 2;
          }
          if ((local_23c < 0) && (local_238 == 2)) {
            if (*(long *)(param_5 + 0x28) != 0) {
              cVar1 = set_process_security_ctx(param_1,param_2,param_6,1,param_5);
              if (cVar1 != '\x01') {
                local_23f = 0;
                goto LAB_00101e6d;
              }
            }
            *param_8 = '\x01';
          }
          goto LAB_00101511;
        }
        cVar1 = set_file_security_ctx(param_2,0,param_5);
        if ((cVar1 == '\x01') || (*(char *)(param_5 + 0x34) == '\0')) goto LAB_00101380;
        local_23f = 0;
LAB_00101de5:
        iVar3 = close(local_23c);
        if (iVar3 < 0) {
          uVar10 = quotearg_style(4,param_2);
          uVar6 = gettext("failed to close %s");
          piVar7 = __errno_location();
          error(0,*piVar7,uVar6,uVar10);
          local_23f = 0;
        }
      }
    }
    else {
      uVar10 = quotearg_style(4,param_1);
      uVar6 = gettext("skipping file %s, as it was replaced while being copied");
      error(0,0,uVar6,uVar10);
      local_23f = 0;
    }
  }
  else {
    uVar10 = quotearg_style(4,param_1);
    uVar6 = gettext("cannot fstat %s");
    piVar7 = __errno_location();
    error(0,*piVar7,uVar6,uVar10);
    local_23f = 0;
  }
LAB_00101e6d:
  iVar2 = close(iVar2);
  if (iVar2 < 0) {
    uVar10 = quotearg_style(4,param_1);
    uVar6 = gettext("failed to close %s");
    piVar7 = __errno_location();
    error(0,*piVar7,uVar6,uVar10);
    local_23f = 0;
  }
  if (*(char *)(param_5 + 0x42) != '\0') {
    emit_debug(param_5);
  }
LAB_00101f14:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_23f;
}

// Function: same_file_ok
bool same_file_ok(char *param_1,stat *param_2,int param_3,char *param_4,stat *param_5,int *param_6,
                 undefined *param_7) {
  char cVar1;
  int iVar2;
  char *__ptr;
  long in_FS_OFFSET;
  bool bVar3;
  char local_164;
  stat *local_160;
  stat *local_158;
  stat local_148;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_164 = psame_inode(param_2,param_5);
  *param_7 = 0;
  if ((local_164 != '\0') && (*(char *)((long)param_6 + 0x17) != '\0')) {
    *param_7 = 1;
    bVar3 = true;
    goto LAB_00102703;
  }
  if (param_6[1] == 2) {
    local_160 = param_2;
    local_158 = param_5;
    if (((param_2->st_mode & 0xf000) == 0xa000) && ((param_5->st_mode & 0xf000) == 0xa000)) {
      cVar1 = same_nameat(0xffffff9c,param_1,param_3,param_4);
      if (cVar1 != '\x01') {
        if (*param_6 != 0) {
          bVar3 = true;
          goto LAB_00102703;
        }
        if (local_164 != '\0') {
          *param_7 = 1;
          bVar3 = *(char *)(param_6 + 6) == '\0';
          goto LAB_00102703;
        }
      }
      bVar3 = cVar1 == '\0';
      goto LAB_00102703;
    }
  }
  else {
    if (local_164 != '\x01') {
      bVar3 = true;
      goto LAB_00102703;
    }
    iVar2 = fstatat(param_3,param_4,&local_148,0x100);
    if ((iVar2 != 0) || (iVar2 = lstat(param_1,&local_b8), iVar2 != 0)) {
      bVar3 = true;
      goto LAB_00102703;
    }
    local_160 = &local_b8;
    local_158 = &local_148;
    local_164 = psame_inode(local_160,local_158);
    if (((local_b8.st_mode & 0xf000) == 0xa000) &&
       (((local_148.st_mode & 0xf000) == 0xa000 && (*(char *)((long)param_6 + 0x15) != '\0')))) {
      bVar3 = true;
      goto LAB_00102703;
    }
  }
  if (*param_6 != 0) {
    if (local_164 == '\x01') {
      cVar1 = same_nameat(0xffffff9c,param_1,param_3,param_4);
      bVar3 = cVar1 == '\0';
    }
    else if ((((*(char *)(param_6 + 6) == '\x01') || (param_6[1] == 2)) ||
             ((local_160->st_mode & 0xf000) != 0xa000)) || ((local_158->st_mode & 0xf000) == 0xa000)
            ) {
      bVar3 = true;
    }
    else {
      bVar3 = false;
    }
    goto LAB_00102703;
  }
  if ((*(char *)(param_6 + 6) != '\0') || (*(char *)((long)param_6 + 0x15) != '\0')) {
    if ((local_158->st_mode & 0xf000) == 0xa000) {
      bVar3 = true;
      goto LAB_00102703;
    }
    if (((local_164 != '\0') && (1 < local_158->st_nlink)) &&
       (cVar1 = same_nameat(0xffffff9c,param_1,param_3,param_4), cVar1 != '\x01')) {
      bVar3 = *(char *)(param_6 + 6) == '\0';
      goto LAB_00102703;
    }
  }
  if (((local_160->st_mode & 0xf000) != 0xa000) && ((local_158->st_mode & 0xf000) != 0xa000)) {
    cVar1 = psame_inode(local_160,local_158);
    if (cVar1 != '\x01') {
      bVar3 = true;
      goto LAB_00102703;
    }
    if (*(char *)((long)param_6 + 0x17) != '\0') {
      *param_7 = 1;
      bVar3 = true;
      goto LAB_00102703;
    }
  }
  if (((*(char *)(param_6 + 6) == '\0') || ((param_2->st_mode & 0xf000) != 0xa000)) ||
     ((local_158->st_nlink < 2 || (__ptr = canonicalize_file_name(param_1), __ptr == (char *)0x0))))
  {
    if ((*(char *)((long)param_6 + 0x3a) == '\0') || ((local_158->st_mode & 0xf000) != 0xa000)) {
      if (param_6[1] == 2) {
        if ((local_160->st_mode & 0xf000) == 0xa000) {
          iVar2 = stat(param_1,&local_b8);
          if (iVar2 != 0) {
            bVar3 = true;
            goto LAB_00102703;
          }
        }
        else {
          local_b8.st_dev = local_160->st_dev;
          local_b8.st_ino = local_160->st_ino;
          local_b8.st_nlink = local_160->st_nlink;
          local_b8.st_mode = local_160->st_mode;
          local_b8.st_uid = local_160->st_uid;
          local_b8.st_gid = local_160->st_gid;
          local_b8.__pad0 = local_160->__pad0;
          local_b8.st_rdev = local_160->st_rdev;
          local_b8.st_size = local_160->st_size;
          local_b8.st_blksize = local_160->st_blksize;
          local_b8.st_blocks = local_160->st_blocks;
          local_b8.st_atim.tv_sec = (local_160->st_atim).tv_sec;
          local_b8.st_atim.tv_nsec = (local_160->st_atim).tv_nsec;
          local_b8.st_mtim.tv_sec = (local_160->st_mtim).tv_sec;
          local_b8.st_mtim.tv_nsec = (local_160->st_mtim).tv_nsec;
          local_b8.st_ctim.tv_sec = (local_160->st_ctim).tv_sec;
          local_b8.st_ctim.tv_nsec = (local_160->st_ctim).tv_nsec;
          local_b8.__unused[0] = local_160->__unused[0];
          local_b8.__unused[2] = local_160->__unused[2];
          local_b8.__unused[1] = local_160->__unused[1];
        }
        if ((local_158->st_mode & 0xf000) == 0xa000) {
          iVar2 = fstatat(param_3,param_4,&local_148,0);
          if (iVar2 != 0) {
            bVar3 = true;
            goto LAB_00102703;
          }
        }
        else {
          local_148.st_dev = local_158->st_dev;
          local_148.st_ino = local_158->st_ino;
          local_148.st_nlink = local_158->st_nlink;
          local_148.st_mode = local_158->st_mode;
          local_148.st_uid = local_158->st_uid;
          local_148.st_gid = local_158->st_gid;
          local_148.__pad0 = local_158->__pad0;
          local_148.st_rdev = local_158->st_rdev;
          local_148.st_size = local_158->st_size;
          local_148.st_blksize = local_158->st_blksize;
          local_148.st_blocks = local_158->st_blocks;
          local_148.st_atim.tv_sec = (local_158->st_atim).tv_sec;
          local_148.st_atim.tv_nsec = (local_158->st_atim).tv_nsec;
          local_148.st_mtim.tv_sec = (local_158->st_mtim).tv_sec;
          local_148.st_mtim.tv_nsec = (local_158->st_mtim).tv_nsec;
          local_148.st_ctim.tv_sec = (local_158->st_ctim).tv_sec;
          local_148.st_ctim.tv_nsec = (local_158->st_ctim).tv_nsec;
          local_148.__unused[0] = local_158->__unused[0];
          local_148.__unused[2] = local_158->__unused[2];
          local_148.__unused[1] = local_158->__unused[1];
        }
        cVar1 = psame_inode(&local_b8,&local_148);
        if (cVar1 != '\x01') {
          bVar3 = true;
          goto LAB_00102703;
        }
        if (*(char *)((long)param_6 + 0x17) != '\0') {
          *param_7 = (local_158->st_mode & 0xf000) != 0xa000;
          bVar3 = true;
          goto LAB_00102703;
        }
      }
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
  }
  else {
    cVar1 = same_nameat(0xffffff9c,__ptr,param_3,param_4);
    bVar3 = cVar1 == '\0';
    free(__ptr);
  }
LAB_00102703:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return bVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: writable_destination
undefined4 writable_destination(int param_1,char *param_2,uint param_3) {
  char cVar1;
  int iVar2;
  
  if ((((param_3 & 0xf000) != 0xa000) && (cVar1 = can_write_any_file(), cVar1 == '\0')) &&
     (iVar2 = faccessat(param_1,param_2,2,0x200), iVar2 != 0)) {
    return 0;
  }
  return 1;
}

// Function: overwrite_ok
void overwrite_ok(long param_1,undefined8 param_2,undefined4 param_3,undefined8 param_4,long param_5
                 ) {
  uint uVar1;
  undefined8 uVar2;
  char cVar3;
  undefined8 uVar4;
  char *pcVar5;
  long in_FS_OFFSET;
  undefined local_3c;
  undefined auStack_3b [9];
  undefined local_32;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  cVar3 = writable_destination(param_3,param_4,*(undefined4 *)(param_5 + 0x18));
  if (cVar3 == '\x01') {
    uVar4 = quotearg_style(4,param_2);
    uVar2 = _program_name;
    pcVar5 = (char *)gettext("%s: overwrite %s? ");
    fprintf(_stderr,pcVar5,uVar2,uVar4);
  }
  else {
    strmode(*(undefined4 *)(param_5 + 0x18),&local_3c);
    local_32 = 0;
    uVar1 = *(uint *)(param_5 + 0x18);
    uVar4 = quotearg_style(4,param_2);
    uVar2 = _program_name;
    if (((*(char *)(param_1 + 0x18) == '\0') && (*(char *)(param_1 + 0x15) == '\0')) &&
       (*(char *)(param_1 + 0x16) == '\0')) {
      pcVar5 = (char *)gettext("%s: unwritable %s (mode %04lo, %s); try anyway? ");
    }
    else {
      pcVar5 = (char *)gettext("%s: replace %s, overriding mode %04lo (%s)? ");
    }
    fprintf(_stderr,pcVar5,uVar2,uVar4,(ulong)(uVar1 & 0xfff),auStack_3b);
  }
  yesno();
  if (local_30 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: dest_info_init
void dest_info_init(long param_1) {
  undefined8 uVar1;
  
  uVar1 = hash_initialize(0x3d,0,&triple_hash,&triple_compare,&triple_free);
  *(undefined8 *)(param_1 + 0x50) = uVar1;
  if (*(long *)(param_1 + 0x50) == 0) {
    xalloc_die();
  }
  return;
}

// Function: src_info_init
void src_info_init(long param_1) {
  undefined8 uVar1;
  
  uVar1 = hash_initialize(0x3d,0,&triple_hash_no_name,&triple_compare,&triple_free);
  *(undefined8 *)(param_1 + 0x58) = uVar1;
  if (*(long *)(param_1 + 0x58) == 0) {
    xalloc_die();
  }
  return;
}

// Function: abandon_move
undefined4
abandon_move(long param_1,undefined8 param_2,undefined4 param_3,undefined8 param_4,long param_5) {
  char cVar1;
  undefined4 uVar2;
  
  if (*(char *)(param_1 + 0x18) == '\0') {
                    /* WARNING: Subroutine does not return */
    __assert_fail("x->move_mode",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/copy.c",
                  0x5d9,"abandon_move");
  }
  if ((*(int *)(param_1 + 0x3c) == 2) || (*(int *)(param_1 + 0x3c) == 3)) {
LAB_00102a78:
    uVar2 = 1;
  }
  else {
    if (*(int *)(param_1 + 8) == 3) {
LAB_00102a53:
      cVar1 = overwrite_ok(param_1,param_2,param_3,param_4,param_5);
      if (cVar1 != '\x01') goto LAB_00102a78;
    }
    else if ((*(int *)(param_1 + 8) == 0) && (*(char *)(param_1 + 0x43) != '\0')) {
      cVar1 = writable_destination(param_3,param_4,*(undefined4 *)(param_5 + 0x18));
      if (cVar1 != '\x01') goto LAB_00102a53;
    }
    uVar2 = 0;
  }
  return uVar2;
}

// Function: emit_verbose
void emit_verbose(char *param_1,undefined8 param_2,undefined8 param_3,long param_4) {
  undefined8 uVar1;
  undefined8 uVar2;
  char *__format;
  
  uVar1 = quotearg_n_style(1,4,param_3);
  uVar2 = quotearg_n_style(0,4,param_2);
  printf(param_1,uVar2,uVar1);
  if (param_4 != 0) {
    uVar1 = quotearg_style(4,param_4);
    __format = (char *)gettext(" (backup: %s)");
    printf(__format,uVar1);
  }
  putchar_unlocked(10);
  return;
}

// Function: restore_default_fscreatecon_or_die
void restore_default_fscreatecon_or_die(void) {
  int iVar1;
  undefined8 uVar2;
  int *piVar3;
  
  iVar1 = setfscreatecon(0);
  if (iVar1 != 0) {
    uVar2 = gettext("failed to restore the default file creation context");
    piVar3 = __errno_location();
    error(1,*piVar3,uVar2);
  }
  return;
}

// Function: subst_suffix
void subst_suffix(void *param_1,long param_2,char *param_3) {
  size_t __n;
  size_t sVar1;
  void *__dest;
  
  __n = param_2 - (long)param_1;
  sVar1 = strlen(param_3);
  __dest = (void *)ximalloc(sVar1 + 1 + __n);
  memcpy((void *)(__n + (long)__dest),param_3,sVar1 + 1);
  memcpy(__dest,param_1,__n);
  return;
}

// Function: create_hard_link
undefined8
create_hard_link(void *param_1,undefined4 param_2,undefined8 param_3,undefined8 param_4,
                undefined4 param_5,undefined8 param_6,undefined param_7,char param_8,char param_9) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  char *__format;
  undefined8 uVar4;
  void *local_40;
  void *local_28;
  
  if (param_9 == '\0') {
    uVar4 = 0;
  }
  else {
    uVar4 = 0x400;
  }
  iVar1 = force_linkat(param_2,param_3,param_5,param_6,uVar4,param_7,0xffffffffffffffff);
  if (iVar1 < 1) {
    if ((iVar1 < 0) && (param_8 != '\0')) {
      uVar4 = quotearg_style(4,param_4);
      __format = (char *)gettext("removed %s\n");
      printf(__format,uVar4);
    }
    uVar4 = 1;
  }
  else {
    local_28 = (void *)0x0;
    local_40 = param_1;
    if (param_1 == (void *)0x0) {
      local_40 = (void *)subst_suffix(param_4,param_6,param_3);
      local_28 = local_40;
    }
    uVar4 = quotearg_n_style(1,4,local_40);
    uVar2 = quotearg_n_style(0,4,param_4);
    uVar3 = gettext("cannot create hard link %s to %s");
    error(0,iVar1,uVar3,uVar2,uVar4);
    free(local_28);
    uVar4 = 0;
  }
  return uVar4;
}

// Function: should_dereference
undefined4 should_dereference(long param_1,char param_2) {
  undefined4 uVar1;
  
  if ((*(int *)(param_1 + 4) == 4) || ((*(int *)(param_1 + 4) == 3 && (param_2 != '\0')))) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: source_is_dst_backup
undefined4 source_is_dst_backup(char *param_1,undefined8 param_2,int param_3,char *param_4) {
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  size_t sVar4;
  char *pcVar5;
  size_t sVar6;
  size_t sVar7;
  long in_FS_OFFSET;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  sVar4 = strlen(param_1);
  pcVar5 = (char *)last_component(param_4);
  sVar6 = strlen(pcVar5);
  sVar7 = strlen(_simple_backup_suffix);
  if (((sVar4 == sVar7 + sVar6) && (cVar1 = memeq(param_1,pcVar5,sVar6), cVar1 == '\x01')) &&
     (cVar1 = streq(param_1 + sVar6,_simple_backup_suffix), pcVar5 = _simple_backup_suffix,
     cVar1 == '\x01')) {
    sVar4 = strlen(param_4);
    pcVar5 = (char *)subst_suffix(param_4,param_4 + sVar4,pcVar5);
    iVar2 = fstatat(param_3,pcVar5,&local_b8,0);
    free(pcVar5);
    if ((iVar2 == 0) && (cVar1 = psame_inode(param_2,&local_b8), cVar1 != '\0')) {
      uVar3 = 1;
    }
    else {
      uVar3 = 0;
    }
  }
  else {
    uVar3 = 0;
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: copy_internal
undefined copy_internal(char *param_1,char *param_2,int param_3,char *param_4,uint param_5,
                       __dev_t *param_6,undefined8 param_7,int *param_8,char param_9,char *param_10,
                       undefined *param_11,byte *param_12) {
  long lVar1;
  undefined auVar2 [16];
  __dev_t _Var3;
  __uid_t _Var4;
  __gid_t _Var5;
  byte bVar6;
  char cVar7;
  undefined uVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  int *piVar12;
  undefined8 uVar13;
  undefined8 uVar14;
  char *pcVar15;
  size_t sVar16;
  ulong uVar17;
  void *pvVar18;
  stat *psVar19;
  undefined8 uVar20;
  char *pcVar21;
  size_t sVar22;
  char *pcVar23;
  undefined *puVar24;
  undefined *puVar25;
  undefined *puVar26;
  undefined *puVar27;
  undefined *puVar28;
  undefined4 uVar29;
  long in_FS_OFFSET;
  undefined auStack_5f8 [8];
  byte *local_5f0;
  undefined *local_5e8;
  char *local_5e0;
  char local_5d4;
  int *local_5d0;
  undefined8 local_5c8;
  __dev_t *local_5c0;
  char *local_5b8;
  uint local_5b0;
  int local_5ac;
  char *local_5a8;
  char *local_5a0;
  byte local_591;
  char local_590;
  char local_58f;
  undefined local_58e;
  char local_58d;
  char local_58c;
  char local_58b;
  undefined local_58a;
  char local_589;
  byte local_588;
  char local_587;
  byte local_586;
  char local_585;
  uint local_584;
  uint local_580;
  uint local_57c;
  int local_578;
  int local_574;
  uint local_570;
  int local_56c;
  undefined4 local_568;
  undefined4 local_564;
  uint local_560;
  uint local_55c;
  uint local_558;
  int local_554;
  uint local_550;
  int local_54c;
  undefined4 local_548;
  int local_544;
  undefined4 local_540;
  undefined4 local_53c;
  undefined4 local_538;
  undefined4 local_534;
  undefined4 local_530;
  int local_52c;
  undefined4 local_528;
  undefined4 local_524;
  undefined4 local_520;
  undefined4 local_51c;
  undefined4 local_518;
  undefined4 local_514;
  undefined4 local_510;
  undefined4 local_50c;
  undefined4 local_508;
  undefined4 local_504;
  undefined4 local_500;
  int local_4fc;
  undefined4 local_4f8;
  int local_4f4;
  undefined4 local_4f0;
  undefined4 local_4ec;
  undefined4 local_4e8;
  undefined4 local_4e4;
  undefined4 local_4e0;
  undefined4 local_4dc;
  undefined4 local_4d8;
  undefined4 local_4d4;
  undefined4 local_4d0;
  undefined4 local_4cc;
  undefined4 local_4c8;
  int local_4c4;
  undefined4 local_4c0;
  int local_4bc;
  undefined4 local_4b8;
  int local_4b4;
  undefined4 local_4b0;
  int local_4ac;
  undefined4 local_4a8;
  undefined4 local_4a4;
  undefined4 local_4a0;
  int local_49c;
  undefined4 local_498;
  int local_494;
  undefined4 local_490;
  int local_48c;
  undefined4 local_488;
  undefined4 local_484;
  undefined4 local_480;
  int local_47c;
  undefined4 local_478;
  int local_474;
  undefined4 local_470;
  int local_46c;
  undefined4 local_468;
  int local_464;
  undefined4 local_460;
  int local_45c;
  undefined4 local_458;
  int local_454;
  undefined4 local_450;
  undefined4 local_44c;
  undefined4 local_448;
  int local_444;
  undefined4 local_440;
  int local_43c;
  undefined4 local_438;
  int local_434;
  undefined4 local_430;
  int local_42c;
  long local_428;
  void *local_420;
  char *local_418;
  char *local_410;
  char *local_408;
  undefined8 local_400;
  undefined8 local_3f8;
  char *local_3f0;
  size_t local_3e8;
  size_t local_3e0;
  stat *local_3d8;
  void *local_3d0;
  undefined8 local_3c8;
  char *local_3c0;
  void *local_3b8;
  char *local_3b0;
  undefined8 *local_3a8;
  char *local_3a0;
  undefined8 local_398;
  undefined8 local_390;
  undefined8 local_388;
  undefined8 local_380;
  undefined8 local_378;
  undefined8 local_370;
  undefined8 local_368;
  undefined8 local_360;
  undefined8 local_358;
  undefined8 local_350;
  undefined8 local_348;
  undefined8 local_340;
  undefined8 local_338;
  undefined8 local_330;
  undefined8 local_328;
  undefined8 local_320;
  undefined8 local_318;
  undefined8 local_310;
  undefined8 local_308;
  undefined8 local_300;
  undefined8 local_2f8;
  undefined8 local_2f0;
  undefined8 local_2e8;
  undefined8 local_2e0;
  undefined8 local_2d8;
  undefined8 local_2d0;
  undefined8 local_2c8;
  undefined8 local_2c0;
  undefined8 local_2b8;
  undefined8 local_2b0;
  undefined8 local_2a8;
  undefined8 local_2a0;
  undefined8 local_298;
  undefined8 local_290;
  undefined8 local_288;
  undefined8 local_280;
  __dev_t local_278;
  undefined8 local_270;
  ulong local_268;
  uint local_260;
  __uid_t local_25c;
  __gid_t local_258;
  __dev_t local_250;
  undefined8 local_248;
  stat local_1e8;
  stat local_158;
  undefined local_c8 [24];
  undefined8 local_b0;
  long local_30;
  
  puVar28 = auStack_5f8;
  puVar24 = auStack_5f8;
  puVar25 = auStack_5f8;
  puVar27 = auStack_5f8;
  local_5c8 = param_7;
  local_5d0 = param_8;
  local_5d4 = param_9;
  local_5e0 = param_10;
  local_5e8 = param_11;
  local_5f0 = param_12;
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  local_584 = 0;
  local_580 = 0;
  local_58f = '\0';
  local_428 = 0;
  local_420 = (void *)0x0;
  local_418 = param_4;
  if (*param_4 == '\0') {
    local_418 = ".";
  }
  local_58d = '\0';
  local_58c = '\0';
  local_58b = '\0';
  *param_11 = 0;
  local_578 = param_8[0x12];
  local_5c0 = param_6;
  local_5b8 = param_4;
  local_5b0 = param_5;
  local_5ac = param_3;
  local_5a8 = param_2;
  local_5a0 = param_1;
  if ((*(char *)(param_8 + 6) != '\0') && (*(char *)((long)param_8 + 0x1a) != '\x01')) {
    if (local_578 < 0) {
      iVar9 = renameatu(0xffffff9c,param_1,param_3,local_418,1);
      if (iVar9 == 0) {
        local_578 = 0;
      }
      else {
        piVar12 = __errno_location();
        local_578 = *piVar12;
      }
    }
    *local_5f0 = local_578 == 0;
    local_5b0 = (uint)*local_5f0;
  }
  if (local_578 == 0) {
    bVar6 = *(byte *)((long)local_5d0 + 0x45) ^ 1;
  }
  else if ((local_578 == 0x11) && ((local_5d0[0xf] == 2 || (local_5d0[0xf] == 3)))) {
    bVar6 = 0;
  }
  else {
    bVar6 = 1;
  }
  if (bVar6 != 0) {
    local_408 = local_418;
    local_410 = local_5a8;
    local_56c = local_5ac;
    if (local_578 != 0) {
      local_56c = -100;
      local_408 = local_5a0;
      local_410 = local_5a0;
    }
    if (local_5d0[1] == 2) {
      local_568 = 0x100;
    }
    else {
      local_568 = 0;
    }
    iVar9 = follow_fstatat(local_56c,local_408,&local_278,local_568);
    if (iVar9 != 0) {
      uVar13 = quotearg_style(4,local_410);
      uVar14 = gettext("cannot stat %s");
      piVar12 = __errno_location();
      local_544 = *piVar12;
      local_548 = 0;
      local_398 = uVar14;
      error(0,local_544,uVar14,uVar13);
      auVar2._8_4_ = (__mode_t)local_b0;
      auVar2._12_4_ = local_b0._4_4_;
      auVar2._0_8_ = local_c8._16_8_;
      puVar28 = auStack_5f8;
      uVar8 = 0;
      goto LAB_00106476;
    }
    local_584 = local_260;
    if (((local_260 & 0xf000) == 0x4000) && (*(char *)(local_5d0 + 0xe) != '\x01')) {
      uVar13 = quotearg_style(4,local_5a0);
      if (*(char *)((long)local_5d0 + 0x1b) == '\x01') {
        local_390 = gettext("omitting directory %s");
      }
      else {
        local_390 = gettext("-r not specified; omitting directory %s");
      }
      local_540 = 0;
      local_53c = 0;
      error(0,0,local_390,uVar13);
      auVar2._8_4_ = (__mode_t)local_b0;
      auVar2._12_4_ = local_b0._4_4_;
      auVar2._0_8_ = local_c8._16_8_;
      puVar28 = auStack_5f8;
      uVar8 = 0;
      goto LAB_00106476;
    }
  }
  if ((local_5d4 != '\0') && (*(long *)(local_5d0 + 0x16) != 0)) {
    if (((local_584 & 0xf000) != 0x4000) &&
       ((*local_5d0 == 0 &&
        (cVar7 = seen_file(*(undefined8 *)(local_5d0 + 0x16),local_5a0,&local_278), cVar7 != '\0')))
       ) {
      uVar13 = quotearg_style(4,local_5a0);
      local_388 = gettext("warning: source file %s specified more than once");
      local_538 = 0;
      local_534 = 0;
      error(0,0,local_388,uVar13);
      auVar2._8_4_ = (__mode_t)local_b0;
      auVar2._12_4_ = local_b0._4_4_;
      auVar2._0_8_ = local_c8._16_8_;
      puVar28 = auStack_5f8;
      uVar8 = 1;
      goto LAB_00106476;
    }
    record_file(*(undefined8 *)(local_5d0 + 0x16),local_5a0,&local_278);
  }
  local_588 = should_dereference(local_5d0,local_5d4);
  local_591 = 0 < (int)local_5b0;
  puVar26 = auStack_5f8;
  if ((bool)local_591) {
LAB_0010420b:
    puVar28 = puVar26;
    pcVar15 = local_418;
    iVar9 = local_5ac;
    if ((((local_5d4 != '\0') && (*(long *)(local_5d0 + 0x14) != 0)) &&
        (*(char *)(local_5d0 + 6) != '\x01')) && (*local_5d0 == 0)) {
      if (local_58b == '\0') {
        *(undefined8 *)(puVar28 + -8) = 0x104285;
        iVar9 = fstatat(iVar9,pcVar15,(stat *)local_c8,0x100);
        if (iVar9 < 0) {
          psVar19 = (stat *)0x0;
        }
        else {
          psVar19 = (stat *)local_c8;
        }
      }
      else {
        psVar19 = &local_1e8;
      }
      pcVar15 = local_5b8;
      local_3d8 = psVar19;
      if ((psVar19 != (stat *)0x0) && ((psVar19->st_mode & 0xf000) == 0xa000)) {
        uVar13 = *(undefined8 *)(local_5d0 + 0x14);
        *(undefined8 *)(puVar28 + -8) = 0x1042ea;
        cVar7 = seen_file(uVar13,pcVar15,psVar19);
        pcVar15 = local_5a8;
        if (cVar7 != '\0') {
          *(undefined8 *)(puVar28 + -8) = 0x10430b;
          uVar13 = quotearg_n_style(1,4,pcVar15);
          pcVar15 = local_5a0;
          *(undefined8 *)(puVar28 + -8) = 0x104327;
          uVar14 = quotearg_n_style(0,4,pcVar15);
          *(undefined8 *)(puVar28 + -8) = 0x104339;
          uVar20 = gettext("will not copy %s through just-created symlink %s");
          local_4f0 = 0;
          local_4ec = 0;
          local_340 = uVar20;
          *(undefined8 *)(puVar28 + -8) = 0x104379;
          error(0,0,uVar20,uVar14,uVar13);
          auVar2._8_4_ = (__mode_t)local_b0;
          auVar2._12_4_ = local_b0._4_4_;
          auVar2._0_8_ = local_c8._16_8_;
          uVar8 = 0;
          goto LAB_00106476;
        }
      }
    }
    pvVar18 = local_420;
    pcVar23 = local_5a0;
    pcVar15 = local_5a8;
    if (((*(char *)(local_5d0 + 0x10) != '\0') && (*(char *)(local_5d0 + 6) != '\x01')) &&
       ((local_584 & 0xf000) != 0x4000)) {
      *(undefined8 *)(puVar28 + -8) = 0x1043de;
      emit_verbose("%s -> %s",pcVar23,pcVar15,pvVar18);
    }
    uVar13 = local_270;
    _Var3 = local_278;
    pcVar15 = local_5b8;
    if ((local_578 == 0) || (*(char *)((long)local_5d0 + 0x1a) != '\0')) {
      local_428 = 0;
    }
    else if ((*(char *)(local_5d0 + 0xe) == '\0') || ((local_584 & 0xf000) != 0x4000)) {
      if ((*(char *)(local_5d0 + 6) == '\0') || (local_268 != 1)) {
        if (((*(char *)(local_5d0 + 0xc) != '\0') && (*(char *)((long)local_5d0 + 0x17) != '\x01'))
           && ((1 < local_268 ||
               (((local_5d4 != '\0' && (local_5d0[1] == 3)) || (local_5d0[1] == 4)))))) {
          *(undefined8 *)(puVar28 + -8) = 0x104534;
          local_428 = remember_copied(pcVar15,uVar13,_Var3);
        }
      }
      else {
        *(undefined8 *)(puVar28 + -8) = 0x1044b6;
        local_428 = src_to_dest_lookup(uVar13,_Var3);
      }
    }
    else if (local_5d4 == '\0') {
      *(undefined8 *)(puVar28 + -8) = 0x104475;
      local_428 = src_to_dest_lookup(uVar13,_Var3);
    }
    else {
      *(undefined8 *)(puVar28 + -8) = 0x104450;
      local_428 = remember_copied(pcVar15,uVar13,_Var3);
    }
    lVar1 = local_428;
    pcVar21 = local_5a0;
    pcVar23 = local_5a8;
    iVar9 = local_5ac;
    pcVar15 = local_5b8;
    puVar27 = puVar28;
    if (local_428 == 0) {
LAB_0010482f:
      pcVar23 = local_418;
      pcVar15 = local_5a0;
      iVar9 = local_5ac;
      if (*(char *)(local_5d0 + 6) != '\0') {
        if (local_578 == 0x11) {
          if (*(char *)((long)local_5d0 + 0x1a) == '\0') {
            uVar29 = 0;
          }
          else {
            uVar29 = 2;
          }
          *(undefined8 *)(puVar28 + -8) = 0x10488a;
          iVar9 = renameatu(0xffffff9c,pcVar15,iVar9,pcVar23,uVar29);
          if (iVar9 == 0) {
            local_578 = 0;
          }
          else {
            *(undefined8 *)(puVar28 + -8) = 0x104893;
            piVar12 = __errno_location();
            local_578 = *piVar12;
          }
        }
        pcVar23 = local_418;
        pcVar15 = local_5a8;
        iVar9 = local_5ac;
        if (local_578 == 0) {
          if (*(char *)(local_5d0 + 0x10) != '\0') {
            if (*(char *)((long)local_5d0 + 0x1a) == '\0') {
              *(undefined8 *)(puVar28 + -8) = 0x1048ed;
              uVar13 = gettext("renamed %s -> %s");
            }
            else {
              *(undefined8 *)(puVar28 + -8) = 0x1048dc;
              uVar13 = gettext("exchanged %s <-> %s");
            }
            pvVar18 = local_420;
            pcVar23 = local_5a0;
            pcVar15 = local_5a8;
            *(undefined8 *)(puVar28 + -8) = 0x10490a;
            emit_verbose(uVar13,pcVar23,pcVar15,pvVar18);
          }
          pcVar15 = local_5a8;
          piVar12 = local_5d0;
          if (*(long *)(local_5d0 + 10) != 0) {
            *(undefined8 *)(puVar28 + -8) = 0x104935;
            set_file_security_ctx(pcVar15,1,piVar12);
          }
          pcVar15 = local_5b8;
          if (local_5f0 != (byte *)0x0) {
            *local_5f0 = 1;
          }
          if ((local_5d4 != '\0') && (*(char *)((long)local_5d0 + 0x45) != '\x01')) {
            uVar13 = *(undefined8 *)(local_5d0 + 0x14);
            *(undefined8 *)(puVar28 + -8) = 0x104988;
            record_file(uVar13,pcVar15,&local_278);
          }
          auVar2._8_4_ = (__mode_t)local_b0;
          auVar2._12_4_ = local_b0._4_4_;
          auVar2._0_8_ = local_c8._16_8_;
          uVar8 = 1;
          goto LAB_00106476;
        }
        if (local_578 == 0x16) {
          *(undefined8 *)(puVar28 + -8) = 0x1049b8;
          uVar13 = quotearg_n_style(1,4,top_level_dst_name);
          *(undefined8 *)(puVar28 + -8) = 0x1049d4;
          uVar14 = quotearg_n_style(0,4,top_level_src_name);
          *(undefined8 *)(puVar28 + -8) = 0x1049e6;
          uVar20 = gettext("cannot move %s to a subdirectory of itself, %s");
          local_4d0 = 0;
          local_4cc = 0;
          local_320 = uVar20;
          *(undefined8 *)(puVar28 + -8) = 0x104a26;
          error(0,0,uVar20,uVar14,uVar13);
          auVar2._8_4_ = (__mode_t)local_b0;
          auVar2._12_4_ = local_b0._4_4_;
          auVar2._0_8_ = local_c8._16_8_;
          *local_5e8 = 1;
          uVar8 = 1;
          goto LAB_00106476;
        }
        if (((local_578 == 0x12) && (*(char *)((long)local_5d0 + 0x19) == '\0')) &&
           (*(char *)((long)local_5d0 + 0x1a) == '\0')) {
          if ((local_584 & 0xf000) == 0x4000) {
            iVar11 = 0x200;
          }
          else {
            iVar11 = 0;
          }
          *(undefined8 *)(puVar28 + -8) = 0x104c77;
          iVar9 = unlinkat(iVar9,pcVar23,iVar11);
          if (iVar9 != 0) {
            *(undefined8 *)(puVar28 + -8) = 0x104c84;
            piVar12 = __errno_location();
            pcVar15 = local_5a8;
            if (*piVar12 != 2) {
              *(undefined8 *)(puVar28 + -8) = 0x104ca8;
              uVar13 = quotearg_n_style(1,4,pcVar15);
              pcVar15 = local_5a0;
              *(undefined8 *)(puVar28 + -8) = 0x104cc4;
              uVar14 = quotearg_n_style(0,4,pcVar15);
              *(undefined8 *)(puVar28 + -8) = 0x104cd6;
              uVar20 = gettext("inter-device move failed: %s to %s; unable to remove target");
              *(undefined8 *)(puVar28 + -8) = 0x104cde;
              piVar12 = __errno_location();
              iVar9 = *piVar12;
              local_4b0 = 0;
              local_4ac = iVar9;
              local_300 = uVar20;
              *(undefined8 *)(puVar28 + -8) = 0x104d1c;
              error(0,iVar9,uVar20,uVar14,uVar13);
              *(undefined8 *)(puVar28 + -8) = 0x104d36;
              forget_created(local_270,local_278);
              auVar2._8_4_ = (__mode_t)local_b0;
              auVar2._12_4_ = local_b0._4_4_;
              auVar2._0_8_ = local_c8._16_8_;
              uVar8 = 0;
              goto LAB_00106476;
            }
          }
          if ((*(char *)(local_5d0 + 0x10) != '\0') && ((local_584 & 0xf000) != 0x4000)) {
            *(undefined8 *)(puVar28 + -8) = 0x104d70;
            uVar13 = gettext("copied %s -> %s");
            pvVar18 = local_420;
            pcVar23 = local_5a0;
            pcVar15 = local_5a8;
            *(undefined8 *)(puVar28 + -8) = 0x104d90;
            emit_verbose(uVar13,pcVar23,pcVar15,pvVar18);
          }
          local_591 = 1;
          goto LAB_00104d97;
        }
        *(undefined8 *)(puVar28 + -8) = 0x104a7f;
        local_3c8 = quotearg_n_style(1,4,pcVar15);
        pcVar15 = local_5a0;
        if (*(char *)((long)local_5d0 + 0x1a) == '\0') {
          if (local_578 < 0x28) {
            if ((0x10 < local_578) && ((0x8094220000U >> ((byte)local_578 & 0x3f) & 1) != 0)) {
LAB_00104b54:
              *(undefined8 *)(puVar28 + -8) = 0x104b63;
              uVar14 = gettext("cannot overwrite %s");
              uVar13 = local_3c8;
              iVar9 = local_578;
              local_4c0 = 0;
              local_4bc = local_578;
              local_310 = uVar14;
              *(undefined8 *)(puVar28 + -8) = 0x104ba6;
              error(0,iVar9,uVar14,uVar13);
              goto LAB_00104c1f;
            }
          }
          else if (local_578 == 0x7a) goto LAB_00104b54;
          *(undefined8 *)(puVar28 + -8) = 0x104bc2;
          uVar14 = quotearg_n_style(0,4,pcVar15);
          *(undefined8 *)(puVar28 + -8) = 0x104bd4;
          uVar20 = gettext("cannot move %s to %s");
          uVar13 = local_3c8;
          iVar9 = local_578;
          local_4b8 = 0;
          local_4b4 = local_578;
          local_308 = uVar20;
          *(undefined8 *)(puVar28 + -8) = 0x104c1d;
          error(0,iVar9,uVar20,uVar14,uVar13);
        }
        else {
          *(undefined8 *)(puVar28 + -8) = 0x104aae;
          uVar14 = quotearg_n_style(0,4,pcVar15);
          *(undefined8 *)(puVar28 + -8) = 0x104ac0;
          uVar20 = gettext("cannot exchange %s and %s");
          uVar13 = local_3c8;
          iVar9 = local_578;
          local_4c8 = 0;
          local_4c4 = local_578;
          local_318 = uVar20;
          *(undefined8 *)(puVar28 + -8) = 0x104b09;
          error(0,iVar9,uVar20,uVar14,uVar13);
        }
LAB_00104c1f:
        *(undefined8 *)(puVar28 + -8) = 0x104c38;
        forget_created(local_270,local_278);
        auVar2._8_4_ = (__mode_t)local_b0;
        auVar2._12_4_ = local_b0._4_4_;
        auVar2._0_8_ = local_c8._16_8_;
        uVar8 = 0;
        goto LAB_00106476;
      }
LAB_00104d97:
      uVar10 = local_584;
      bVar6 = local_591;
      pcVar23 = local_5a0;
      pcVar15 = local_5a8;
      piVar12 = local_5d0;
      local_560 = local_584;
      if (*(char *)((long)local_5d0 + 0x39) != '\0') {
        local_560 = local_5d0[4];
      }
      local_560 = local_560 & 0xfff;
      if (*(char *)((long)local_5d0 + 0x1f) == '\x01') {
        local_57c = 0x3f;
      }
      else if ((local_584 & 0xf000) == 0x4000) {
        local_57c = 0x12;
      }
      else {
        local_57c = 0;
      }
      local_57c = local_57c & local_560;
      local_58e = 1;
      *(undefined8 *)(puVar28 + -8) = 0x104e42;
      cVar7 = set_process_security_ctx(pcVar23,pcVar15,uVar10,bVar6,piVar12);
      pcVar21 = local_5a0;
      pcVar23 = local_5a8;
      iVar9 = local_5ac;
      pcVar15 = local_5b8;
      uVar13 = local_5c8;
      piVar12 = local_5d0;
      auVar2._8_4_ = (__mode_t)local_b0;
      auVar2._12_4_ = local_b0._4_4_;
      auVar2._0_8_ = local_c8._16_8_;
      if (cVar7 != '\x01') {
        uVar8 = 0;
        goto LAB_00106476;
      }
      if ((local_584 & 0xf000) == 0x4000) {
        *(undefined8 *)(puVar28 + -8) = 0x104e82;
        cVar7 = is_ancestor(&local_278,uVar13);
        pcVar15 = local_5a0;
        puVar26 = puVar28;
        if (cVar7 == '\0') {
          for (; puVar26 != puVar28; puVar26 = puVar26 + -0x1000) {
            *(undefined8 *)(puVar26 + -8) = *(undefined8 *)(puVar26 + -8);
          }
          puVar27 = puVar26 + -0x20;
          *(undefined8 *)(puVar26 + -8) = *(undefined8 *)(puVar26 + -8);
          pcVar23 = local_418;
          pcVar15 = local_5a8;
          iVar9 = local_5ac;
          piVar12 = local_5d0;
          local_3a8 = (undefined8 *)((ulong)(puVar26 + -0x11) & 0xfffffffffffffff0);
          *local_3a8 = local_5c8;
          local_3a8[1] = local_270;
          local_3a8[2] = local_278;
          if ((local_591 == 0) && ((local_1e8.st_mode & 0xf000) == 0x4000)) {
            local_57c = 0;
            if ((*(long *)(local_5d0 + 10) != 0) || (*(char *)((long)local_5d0 + 0x33) != '\0')) {
              *(undefined8 *)(puVar26 + -0x28) = 0x1052c4;
              cVar7 = set_file_security_ctx(pcVar15,0,piVar12);
              if ((cVar7 != '\x01') && (*(char *)(local_5d0 + 0xd) != '\0')) goto LAB_00106311;
            }
LAB_001052e1:
            bVar6 = local_591;
            pcVar21 = local_5a0;
            pcVar23 = local_5a8;
            iVar9 = local_5ac;
            pcVar15 = local_5b8;
            if (((*(char *)((long)local_5d0 + 0x1e) == '\0') || (local_5c0 == (__dev_t *)0x0)) ||
               (puVar28 = puVar26 + -0x20, *local_5c0 == local_278)) {
              *(undefined **)(puVar26 + -0x28) = local_5e8;
              *(char **)(puVar26 + -0x30) = local_5e0;
              *(int **)(puVar26 + -0x38) = local_5d0;
              *(undefined8 **)(puVar26 + -0x40) = local_3a8;
              *(undefined8 *)(puVar26 + -0x48) = 0x105366;
              local_58e = copy_dir(pcVar21,pcVar23,iVar9,pcVar15,bVar6,&local_278);
              puVar28 = puVar26 + -0x20;
            }
            goto LAB_00105c44;
          }
          uVar10 = ~local_57c & local_560;
          local_550 = uVar10;
          *(undefined8 *)(puVar26 + -0x28) = 0x104ffb;
          iVar11 = mkdirat(iVar9,pcVar23,uVar10);
          pcVar23 = local_418;
          pcVar15 = local_5a8;
          iVar9 = local_5ac;
          if (iVar11 == 0) {
            *(undefined8 *)(puVar26 + -0x28) = 0x10508e;
            iVar11 = fstatat(iVar9,pcVar23,&local_1e8,0x100);
            pcVar23 = local_418;
            pcVar15 = local_5a8;
            iVar9 = local_5ac;
            if (iVar11 == 0) {
              if ((local_1e8.st_mode & 0x1c0) != 0x1c0) {
                local_580 = local_1e8.st_mode;
                local_58f = '\x01';
                uVar10 = local_1e8.st_mode | 0x1c0;
                *(undefined8 *)(puVar26 + -0x28) = 0x10514e;
                iVar9 = lchmodat(iVar9,pcVar23,uVar10);
                pcVar15 = local_5a8;
                if (iVar9 != 0) {
                  *(undefined8 *)(puVar26 + -0x28) = 0x105166;
                  uVar13 = quotearg_style(4,pcVar15);
                  *(undefined8 *)(puVar26 + -0x28) = 0x105178;
                  uVar14 = gettext("setting permissions for %s");
                  *(undefined8 *)(puVar26 + -0x28) = 0x105180;
                  piVar12 = __errno_location();
                  iVar9 = *piVar12;
                  local_490 = 0;
                  local_48c = iVar9;
                  local_2e0 = uVar14;
                  *(undefined8 *)(puVar26 + -0x28) = 0x1051bb;
                  error(0,iVar9,uVar14,uVar13);
                  puVar27 = puVar26 + -0x20;
                  goto LAB_00106311;
                }
              }
              pcVar15 = local_5b8;
              if (*local_5e0 != '\x01') {
                *(undefined8 *)(puVar26 + -0x28) = 0x1051f2;
                remember_copied(pcVar15,local_1e8.st_ino,local_1e8.st_dev);
                *local_5e0 = '\x01';
              }
              pcVar23 = local_5a0;
              pcVar15 = local_5a8;
              if (*(char *)(local_5d0 + 0x10) != '\0') {
                if (*(char *)(local_5d0 + 6) == '\0') {
                  *(undefined8 *)(puVar26 + -0x28) = 0x10527e;
                  emit_verbose("%s -> %s",pcVar23,pcVar15,0);
                }
                else {
                  *(undefined8 *)(puVar26 + -0x28) = 0x105232;
                  uVar13 = quotearg_style(4,pcVar15);
                  *(undefined8 *)(puVar26 + -0x28) = 0x105244;
                  pcVar15 = (char *)gettext("created directory %s\n");
                  *(undefined8 *)(puVar26 + -0x28) = 0x105254;
                  printf(pcVar15,uVar13);
                }
              }
              goto LAB_001052e1;
            }
            *(undefined8 *)(puVar26 + -0x28) = 0x1050a6;
            uVar13 = quotearg_style(4,pcVar15);
            *(undefined8 *)(puVar26 + -0x28) = 0x1050b8;
            uVar14 = gettext("cannot stat %s");
            *(undefined8 *)(puVar26 + -0x28) = 0x1050c0;
            piVar12 = __errno_location();
            iVar9 = *piVar12;
            local_498 = 0;
            local_494 = iVar9;
            local_2e8 = uVar14;
            *(undefined8 *)(puVar26 + -0x28) = 0x1050fb;
            error(0,iVar9,uVar14,uVar13);
            puVar27 = puVar26 + -0x20;
          }
          else {
            *(undefined8 *)(puVar26 + -0x28) = 0x105013;
            uVar13 = quotearg_style(4,pcVar15);
            *(undefined8 *)(puVar26 + -0x28) = 0x105025;
            uVar14 = gettext("cannot create directory %s");
            *(undefined8 *)(puVar26 + -0x28) = 0x10502d;
            piVar12 = __errno_location();
            iVar9 = *piVar12;
            local_4a0 = 0;
            local_49c = iVar9;
            local_2f0 = uVar14;
            *(undefined8 *)(puVar26 + -0x28) = 0x105068;
            error(0,iVar9,uVar14,uVar13);
            puVar27 = puVar26 + -0x20;
          }
        }
        else {
          *(undefined8 *)(puVar28 + -8) = 0x104e9a;
          uVar13 = quotearg_style(4,pcVar15);
          *(undefined8 *)(puVar28 + -8) = 0x104eac;
          uVar14 = gettext("cannot copy cyclic symbolic link %s");
          local_4a8 = 0;
          local_4a4 = 0;
          local_2f8 = uVar14;
          *(undefined8 *)(puVar28 + -8) = 0x104ee9;
          error(0,0,uVar14,uVar13);
        }
      }
      else {
        if (*(char *)((long)local_5d0 + 0x3a) == '\0') {
          if (*(char *)((long)local_5d0 + 0x17) == '\0') {
            if (((local_584 & 0xf000) != 0x8000) &&
               ((*(char *)(local_5d0 + 5) == '\0' || ((local_584 & 0xf000) == 0xa000)))) {
              if ((local_584 & 0xf000) != 0x1000) {
                if ((((local_584 & 0xf000) == 0x6000) || ((local_584 & 0xf000) == 0x2000)) ||
                   ((local_584 & 0xf000) == 0xc000)) {
                  uVar10 = ~local_57c & local_584;
                  local_55c = uVar10;
                  *(undefined8 *)(puVar28 + -8) = 0x105852;
                  iVar9 = mknodat(iVar9,pcVar15,uVar10,local_250);
                  pcVar15 = local_5a8;
                  if (iVar9 == 0) goto LAB_00105c44;
                  *(undefined8 *)(puVar28 + -8) = 0x10586e;
                  uVar13 = quotearg_style(4,pcVar15);
                  *(undefined8 *)(puVar28 + -8) = 0x105880;
                  uVar14 = gettext("cannot create special file %s");
                  *(undefined8 *)(puVar28 + -8) = 0x105888;
                  piVar12 = __errno_location();
                  iVar9 = *piVar12;
                  local_470 = 0;
                  local_46c = iVar9;
                  local_2c0 = uVar14;
                  *(undefined8 *)(puVar28 + -8) = 0x1058c3;
                  error(0,iVar9,uVar14,uVar13);
                }
                else if ((local_584 & 0xf000) == 0xa000) {
                  *(undefined8 *)(puVar28 + -8) = 0x1058fb;
                  pcVar21 = (char *)areadlink_with_size(pcVar21,local_248);
                  pcVar23 = local_5a0;
                  iVar9 = local_5ac;
                  pcVar15 = local_5b8;
                  local_58c = '\x01';
                  local_3c0 = pcVar21;
                  if (pcVar21 == (char *)0x0) {
                    *(undefined8 *)(puVar28 + -8) = 0x105927;
                    uVar13 = quotearg_style(4,pcVar23);
                    *(undefined8 *)(puVar28 + -8) = 0x105939;
                    uVar14 = gettext("cannot read symbolic link %s");
                    *(undefined8 *)(puVar28 + -8) = 0x105941;
                    piVar12 = __errno_location();
                    iVar9 = *piVar12;
                    local_468 = 0;
                    local_464 = iVar9;
                    local_2b8 = uVar14;
                    *(undefined8 *)(puVar28 + -8) = 0x10597c;
                    error(0,iVar9,uVar14,uVar13);
                  }
                  else {
                    uVar8 = *(undefined *)((long)local_5d0 + 0x16);
                    *(undefined8 *)(puVar28 + -8) = 0x1059b2;
                    local_574 = force_symlinkat(pcVar21,iVar9,pcVar15,uVar8,0xffffffff);
                    sVar16 = local_1e8.st_size;
                    pcVar15 = local_3c0;
                    if (((0 < local_574) && (local_5d0[0xf] == 1)) &&
                       ((local_591 != 1 && ((local_1e8.st_mode & 0xf000) == 0xa000)))) {
                      *(undefined8 *)(puVar28 + -8) = 0x105a16;
                      sVar22 = strlen(pcVar15);
                      iVar9 = local_5ac;
                      pcVar15 = local_5b8;
                      if (sVar16 == sVar22) {
                        *(undefined8 *)(puVar28 + -8) = 0x105a3c;
                        pvVar18 = (void *)areadlinkat_with_size(iVar9,pcVar15,local_1e8.st_size);
                        pcVar15 = local_3c0;
                        local_3b8 = pvVar18;
                        if (pvVar18 != (void *)0x0) {
                          *(undefined8 *)(puVar28 + -8) = 0x105a66;
                          cVar7 = streq(pvVar18,pcVar15);
                          pvVar18 = local_3b8;
                          if (cVar7 != '\0') {
                            local_574 = 0;
                          }
                          *(undefined8 *)(puVar28 + -8) = 0x105a83;
                          free(pvVar18);
                        }
                      }
                    }
                    pcVar15 = local_3c0;
                    *(undefined8 *)(puVar28 + -8) = 0x105a92;
                    free(pcVar15);
                    pcVar15 = local_5a8;
                    if (local_574 < 1) {
                      if (*(char *)((long)local_5d0 + 0x33) != '\0') {
                        *(undefined8 *)(puVar28 + -8) = 0x105b1a;
                        restore_default_fscreatecon_or_die();
                      }
                      _Var5 = local_258;
                      _Var4 = local_25c;
                      iVar9 = local_5ac;
                      pcVar15 = local_5b8;
                      if (*(char *)((long)local_5d0 + 0x1f) != '\0') {
                        *(undefined8 *)(puVar28 + -8) = 0x105b4d;
                        iVar9 = lchownat(iVar9,pcVar15,_Var4,_Var5);
                        piVar12 = local_5d0;
                        if (iVar9 != 0) {
                          *(undefined8 *)(puVar28 + -8) = 0x105b64;
                          cVar7 = chown_failure_ok(piVar12);
                          if (cVar7 != '\x01') {
                            *(undefined8 *)(puVar28 + -8) = 0x105b7e;
                            uVar13 = gettext("failed to preserve ownership for %s");
                            *(undefined8 *)(puVar28 + -8) = 0x105b86;
                            piVar12 = __errno_location();
                            pcVar15 = local_5a8;
                            iVar9 = *piVar12;
                            local_458 = 0;
                            local_454 = iVar9;
                            local_2a8 = uVar13;
                            *(undefined8 *)(puVar28 + -8) = 0x105bc5;
                            error(0,iVar9,uVar13,pcVar15);
                            if (*(char *)((long)local_5d0 + 0x32) != '\0') goto LAB_00106311;
                          }
                        }
                      }
                      goto LAB_00105c44;
                    }
                    *(undefined8 *)(puVar28 + -8) = 0x105aaf;
                    uVar13 = quotearg_style(4,pcVar15);
                    *(undefined8 *)(puVar28 + -8) = 0x105ac1;
                    uVar14 = gettext("cannot create symbolic link %s");
                    iVar9 = local_574;
                    local_460 = 0;
                    local_45c = local_574;
                    local_2b0 = uVar14;
                    *(undefined8 *)(puVar28 + -8) = 0x105b00;
                    error(0,iVar9,uVar14,uVar13);
                  }
                }
                else {
                  *(undefined8 *)(puVar28 + -8) = 0x105bee;
                  uVar13 = quotearg_style(4,pcVar21);
                  *(undefined8 *)(puVar28 + -8) = 0x105c00;
                  uVar14 = gettext("%s has unknown file type");
                  local_450 = 0;
                  local_44c = 0;
                  local_2a0 = uVar14;
                  *(undefined8 *)(puVar28 + -8) = 0x105c3d;
                  error(0,0,uVar14,uVar13);
                }
                goto LAB_00106311;
              }
              uVar10 = ~local_57c & local_584;
              local_558 = uVar10;
              *(undefined8 *)(puVar28 + -8) = 0x105742;
              iVar11 = mknodat(iVar9,pcVar15,uVar10,0);
              iVar9 = local_5ac;
              pcVar15 = local_5b8;
              if (iVar11 != 0) {
                uVar10 = local_558 & 0xffffefff;
                *(undefined8 *)(puVar28 + -8) = 0x10576c;
                iVar9 = mkfifoat(iVar9,pcVar15,uVar10);
                pcVar15 = local_5a8;
                if (iVar9 != 0) {
                  *(undefined8 *)(puVar28 + -8) = 0x105788;
                  uVar13 = quotearg_style(4,pcVar15);
                  *(undefined8 *)(puVar28 + -8) = 0x10579a;
                  uVar14 = gettext("cannot create fifo %s");
                  *(undefined8 *)(puVar28 + -8) = 0x1057a2;
                  piVar12 = __errno_location();
                  iVar9 = *piVar12;
                  local_478 = 0;
                  local_474 = iVar9;
                  local_2c8 = uVar14;
                  *(undefined8 *)(puVar28 + -8) = 0x1057dd;
                  error(0,iVar9,uVar14,uVar13);
                  goto LAB_00106311;
                }
              }
              goto LAB_00105c44;
            }
            local_58d = '\x01';
            uVar10 = local_560 & 0x1ff;
            *(__dev_t **)(puVar28 + -0x10) = &local_278;
            *(byte **)(puVar28 + -0x18) = &local_591;
            *(ulong *)(puVar28 + -0x20) = (ulong)local_57c;
            *(undefined8 *)(puVar28 + -0x28) = 0x1056e5;
            cVar7 = copy_reg(pcVar21,pcVar23,iVar9,pcVar15,piVar12,uVar10);
          }
          else {
            if ((*(char *)((long)local_5d0 + 0x16) == '\0') && (local_5d0[2] != 3)) {
              local_586 = 0;
            }
            else {
              local_586 = 1;
            }
            uVar17 = (ulong)local_586;
            *(ulong *)(puVar28 + -0x10) = (ulong)local_588;
            *(undefined8 *)(puVar28 + -0x18) = 0;
            *(ulong *)(puVar28 + -0x20) = uVar17;
            *(undefined8 *)(puVar28 + -0x28) = 0x105640;
            cVar7 = create_hard_link(pcVar21,0xffffff9c,pcVar21,pcVar23,iVar9,pcVar15);
          }
          if (cVar7 == '\x01') goto LAB_00105c44;
          goto LAB_00106311;
        }
        local_58c = '\x01';
        if (*local_5a0 != '/') {
          *(undefined8 *)(puVar28 + -8) = 0x1053b0;
          pcVar15 = (char *)dir_name(pcVar15);
          local_3b0 = pcVar15;
          if (local_5ac == -100) {
            *(undefined8 *)(puVar28 + -8) = 0x1053d9;
            cVar7 = streq(&DAT_00106c59,pcVar15);
            if (cVar7 == '\0') goto LAB_001053dd;
LAB_0010543b:
            local_585 = '\x01';
          }
          else {
LAB_001053dd:
            *(undefined8 *)(puVar28 + -8) = 0x1053f6;
            iVar11 = stat(".",&local_158);
            pcVar15 = local_3b0;
            iVar9 = local_5ac;
            if (iVar11 != 0) goto LAB_0010543b;
            *(undefined8 *)(puVar28 + -8) = 0x10541a;
            iVar9 = fstatat(iVar9,pcVar15,(stat *)local_c8,0);
            if (iVar9 != 0) goto LAB_0010543b;
            *(undefined8 *)(puVar28 + -8) = 0x105437;
            cVar7 = psame_inode(&local_158,local_c8);
            if (cVar7 != '\0') goto LAB_0010543b;
            local_585 = '\0';
          }
          pcVar15 = local_3b0;
          *(undefined8 *)(puVar28 + -8) = 0x105463;
          free(pcVar15);
          pcVar15 = local_5a8;
          if (local_585 != '\x01') {
            *(undefined8 *)(puVar28 + -8) = 0x10548a;
            uVar13 = quotearg_n_style_colon(0,3,pcVar15);
            *(undefined8 *)(puVar28 + -8) = 0x10549c;
            uVar14 = gettext("%s: can make relative symbolic links only in current directory");
            local_488 = 0;
            local_484 = 0;
            local_2d8 = uVar14;
            *(undefined8 *)(puVar28 + -8) = 0x1054d9;
            error(0,0,uVar14,uVar13);
            goto LAB_00106311;
          }
        }
        pcVar23 = local_5a0;
        iVar9 = local_5ac;
        pcVar15 = local_5b8;
        uVar8 = *(undefined *)((long)local_5d0 + 0x16);
        *(undefined8 *)(puVar28 + -8) = 0x10550e;
        local_554 = force_symlinkat(pcVar23,iVar9,pcVar15,uVar8,0xffffffff);
        pcVar15 = local_5a0;
        if (local_554 < 1) {
LAB_00105c44:
          pcVar15 = local_5a8;
          piVar12 = local_5d0;
          if (((local_591 != 1) && (*(char *)(local_5d0 + 5) != '\x01')) &&
             (((local_584 & 0xf000) != 0x4000 &&
              ((*(long *)(local_5d0 + 10) != 0 || (*(char *)((long)local_5d0 + 0x33) != '\0')))))) {
            *(undefined8 *)(puVar28 + -8) = 0x105cb0;
            cVar7 = set_file_security_ctx(pcVar15,0,piVar12);
            if ((cVar7 != '\x01') && (puVar27 = puVar28, *(char *)(local_5d0 + 0xd) != '\0'))
            goto LAB_00106311;
          }
          pcVar15 = local_418;
          iVar9 = local_5ac;
          if ((local_5d4 != '\0') && (*(long *)(local_5d0 + 0x14) != 0)) {
            *(undefined8 *)(puVar28 + -8) = 0x105d03;
            iVar9 = fstatat(iVar9,pcVar15,(stat *)local_c8,0x100);
            pcVar15 = local_5b8;
            if (iVar9 == 0) {
              uVar13 = *(undefined8 *)(local_5d0 + 0x14);
              *(undefined8 *)(puVar28 + -8) = 0x105d2b;
              record_file(uVar13,pcVar15,local_c8);
            }
          }
          uVar8 = local_58e;
          auVar2 = unique0x1000157c;
          if (((*(char *)((long)local_5d0 + 0x17) == '\0') || ((local_584 & 0xf000) == 0x4000)) &&
             (local_58d == '\0')) {
            if (*(char *)((long)local_5d0 + 0x21) != '\0') {
              *(undefined8 *)(puVar28 + -8) = 0x105d8f;
              local_c8._0_16_ = get_stat_atime(&local_278);
              *(undefined8 *)(puVar28 + -8) = 0x105dac;
              join_0x00000010_0x00000000_ = get_stat_mtime(&local_278);
              pcVar15 = local_418;
              iVar9 = local_5ac;
              if (local_58c == '\0') {
                iVar11 = 0;
              }
              else {
                iVar11 = 0x100;
              }
              local_54c = iVar11;
              *(undefined8 *)(puVar28 + -8) = 0x105df6;
              iVar9 = utimensat(iVar9,pcVar15,(timespec *)local_c8,iVar11);
              pcVar15 = local_5a8;
              if (iVar9 != 0) {
                *(undefined8 *)(puVar28 + -8) = 0x105e12;
                uVar13 = quotearg_style(4,pcVar15);
                *(undefined8 *)(puVar28 + -8) = 0x105e24;
                uVar14 = gettext("preserving times for %s");
                *(undefined8 *)(puVar28 + -8) = 0x105e2c;
                piVar12 = __errno_location();
                iVar9 = *piVar12;
                local_448 = 0;
                local_444 = iVar9;
                local_298 = uVar14;
                *(undefined8 *)(puVar28 + -8) = 0x105e67;
                error(0,iVar9,uVar14,uVar13);
                if (*(char *)((long)local_5d0 + 0x32) != '\0') {
                  uVar8 = 0;
                  auVar2 = stack0xffffffffffffff48;
                  goto LAB_00106476;
                }
              }
            }
            pcVar23 = local_418;
            pcVar15 = local_5a8;
            iVar9 = local_5ac;
            piVar12 = local_5d0;
            if (((local_58c != '\x01') && (*(char *)((long)local_5d0 + 0x1f) != '\0')) &&
               ((local_591 != 0 ||
                ((local_25c != local_1e8.st_uid || (local_258 != local_1e8.st_gid)))))) {
              uVar17 = (ulong)local_591;
              *(stat **)(puVar28 + -8) = &local_1e8;
              *(ulong *)(puVar28 + -0x10) = uVar17;
              *(undefined8 *)(puVar28 + -0x18) = 0x105f15;
              iVar9 = set_owner(piVar12,pcVar15,iVar9,pcVar23,0xffffffff,&local_278);
              if (iVar9 == -1) {
                uVar8 = 0;
                auVar2 = stack0xffffffffffffff48;
                goto LAB_00106476;
              }
              if (iVar9 == 0) {
                local_584 = local_584 & 0xfffff1ff;
              }
            }
            pcVar23 = local_5a0;
            pcVar15 = local_5a8;
            piVar12 = local_5d0;
            if (*(char *)((long)local_5d0 + 0x35) != '\0') {
              *(undefined8 *)(puVar28 + -8) = 0x105f72;
              cVar7 = copy_attr(pcVar23,0xffffffff,pcVar15,0xffffffff,piVar12);
              if ((cVar7 != '\x01') && (*(char *)((long)local_5d0 + 0x36) != '\0')) {
                uVar8 = 0;
                auVar2 = stack0xffffffffffffff48;
                goto LAB_00106476;
              }
            }
            pcVar15 = local_5a8;
            uVar8 = local_58e;
            auVar2 = unique0x100015cc;
            if (local_58c == '\0') {
              *(undefined8 *)(puVar28 + -8) = 0x105fc2;
              set_author(pcVar15,0xffffffff,&local_278);
              uVar10 = local_584;
              pcVar23 = local_5a0;
              pcVar15 = local_5a8;
              if ((*(char *)(local_5d0 + 8) == '\0') && (*(char *)(local_5d0 + 6) == '\0')) {
                if (*(char *)((long)local_5d0 + 0x39) == '\0') {
                  if ((*(char *)((long)local_5d0 + 0x22) == '\0') || (local_591 == 0)) {
                    if (local_57c != 0) {
                      *(undefined8 *)(puVar28 + -8) = 0x106147;
                      uVar10 = cached_umask();
                      pcVar15 = local_418;
                      iVar9 = local_5ac;
                      local_57c = local_57c & ~uVar10;
                      if ((local_57c != 0) && (local_58f != '\x01')) {
                        if (local_591 != 0) {
                          *(undefined8 *)(puVar28 + -8) = 0x10619d;
                          iVar9 = fstatat(iVar9,pcVar15,&local_1e8,0x100);
                          pcVar15 = local_5a8;
                          if (iVar9 != 0) {
                            *(undefined8 *)(puVar28 + -8) = 0x1061b5;
                            uVar13 = quotearg_style(4,pcVar15);
                            *(undefined8 *)(puVar28 + -8) = 0x1061c7;
                            uVar14 = gettext("cannot stat %s");
                            *(undefined8 *)(puVar28 + -8) = 0x1061cf;
                            piVar12 = __errno_location();
                            iVar9 = *piVar12;
                            local_440 = 0;
                            local_43c = iVar9;
                            local_290 = uVar14;
                            *(undefined8 *)(puVar28 + -8) = 0x10620a;
                            error(0,iVar9,uVar14,uVar13);
                            uVar8 = 0;
                            auVar2 = stack0xffffffffffffff48;
                            goto LAB_00106476;
                          }
                        }
                        local_580 = local_1e8.st_mode;
                        if ((~local_1e8.st_mode & local_57c) != 0) {
                          local_58f = '\x01';
                        }
                      }
                    }
                    pcVar15 = local_418;
                    iVar9 = local_5ac;
                    uVar8 = local_58e;
                    auVar2 = unique0x10001aac;
                    if (local_58f != '\0') {
                      uVar10 = local_580 | local_57c;
                      *(undefined8 *)(puVar28 + -8) = 0x10626c;
                      iVar9 = lchmodat(iVar9,pcVar15,uVar10);
                      pcVar15 = local_5a8;
                      uVar8 = local_58e;
                      auVar2 = stack0xffffffffffffff48;
                      if (iVar9 != 0) {
                        *(undefined8 *)(puVar28 + -8) = 0x106288;
                        uVar13 = quotearg_style(4,pcVar15);
                        *(undefined8 *)(puVar28 + -8) = 0x10629a;
                        uVar14 = gettext("preserving permissions for %s");
                        *(undefined8 *)(puVar28 + -8) = 0x1062a2;
                        piVar12 = __errno_location();
                        iVar9 = *piVar12;
                        local_438 = 0;
                        local_434 = iVar9;
                        local_288 = uVar14;
                        *(undefined8 *)(puVar28 + -8) = 0x1062dd;
                        error(0,iVar9,uVar14,uVar13);
                        uVar8 = local_58e;
                        auVar2 = stack0xffffffffffffff48;
                        if (*(char *)((long)local_5d0 + 0x32) != '\0') {
                          uVar8 = 0;
                        }
                      }
                    }
                  }
                  else {
                    if (((local_584 & 0xf000) == 0x4000) || ((local_584 & 0xf000) == 0xc000)) {
                      local_570 = 0x1ff;
                    }
                    else {
                      local_570 = 0x1b6;
                    }
                    local_580 = local_1e8.st_mode;
                    if ((local_584 & 0xf000) == 0x4000) {
                      local_570 = local_570 | local_1e8.st_mode & 0x400;
                    }
                    *(undefined8 *)(puVar28 + -8) = 0x106103;
                    uVar10 = cached_umask();
                    pcVar15 = local_5a8;
                    uVar10 = ~uVar10 & local_570;
                    *(undefined8 *)(puVar28 + -8) = 0x106123;
                    iVar9 = xset_acl(pcVar15,0xffffffff,uVar10);
                    uVar8 = local_58e;
                    auVar2 = stack0xffffffffffffff48;
                    if (iVar9 != 0) {
                      uVar8 = 0;
                    }
                  }
                }
                else {
                  iVar9 = local_5d0[4];
                  *(undefined8 *)(puVar28 + -8) = 0x10605b;
                  iVar9 = xset_acl(pcVar15,0xffffffff,iVar9);
                  uVar8 = local_58e;
                  auVar2 = stack0xffffffffffffff48;
                  if (iVar9 != 0) {
                    uVar8 = 0;
                  }
                }
              }
              else {
                *(undefined8 *)(puVar28 + -8) = 0x106009;
                iVar9 = xcopy_acl(pcVar23,0xffffffff,pcVar15,0xffffffff,uVar10);
                uVar8 = local_58e;
                auVar2 = stack0xffffffffffffff48;
                if ((iVar9 != 0) && (*(char *)((long)local_5d0 + 0x32) != '\0')) {
                  uVar8 = 0;
                }
              }
            }
          }
          goto LAB_00106476;
        }
        *(undefined8 *)(puVar28 + -8) = 0x10553a;
        uVar13 = quotearg_n_style(1,4,pcVar15);
        pcVar15 = local_5a8;
        *(undefined8 *)(puVar28 + -8) = 0x105556;
        uVar14 = quotearg_n_style(0,4,pcVar15);
        *(undefined8 *)(puVar28 + -8) = 0x105568;
        uVar20 = gettext("cannot create symbolic link %s to %s");
        iVar9 = local_554;
        local_480 = 0;
        local_47c = local_554;
        local_2d0 = uVar20;
        *(undefined8 *)(puVar28 + -8) = 0x1055aa;
        error(0,iVar9,uVar20,uVar14,uVar13);
      }
    }
    else if ((local_584 & 0xf000) == 0x4000) {
      *(undefined8 *)(puVar28 + -8) = 0x104580;
      cVar7 = same_nameat(0xffffff9c,pcVar21,iVar9,lVar1);
      lVar1 = local_428;
      iVar9 = local_5ac;
      pcVar15 = local_5b8;
      if (cVar7 == '\0') {
        *(undefined8 *)(puVar28 + -8) = 0x104640;
        cVar7 = same_nameat(iVar9,pcVar15,iVar9,lVar1);
        lVar1 = local_428;
        pcVar23 = local_5a8;
        pcVar15 = local_5b8;
        if (cVar7 != '\0') {
          *(undefined8 *)(puVar28 + -8) = 0x10465c;
          uVar13 = quotearg_style(4,top_level_src_name);
          *(undefined8 *)(puVar28 + -8) = 0x10466e;
          uVar14 = gettext("warning: source directory %s specified more than once");
          local_4e0 = 0;
          local_4dc = 0;
          local_330 = uVar14;
          *(undefined8 *)(puVar28 + -8) = 0x1046ab;
          error(0,0,uVar14,uVar13);
          auVar2._8_4_ = (__mode_t)local_b0;
          auVar2._12_4_ = local_b0._4_4_;
          auVar2._0_8_ = local_c8._16_8_;
          if ((*(char *)(local_5d0 + 6) != '\0') && (local_5f0 != (byte *)0x0)) {
            *local_5f0 = 1;
          }
          uVar8 = 1;
          goto LAB_00106476;
        }
        if ((local_5d0[1] == 4) || ((local_5d4 != '\0' && (local_5d0[1] == 3)))) goto LAB_0010482f;
        *(undefined8 *)(puVar28 + -8) = 0x104728;
        pvVar18 = (void *)subst_suffix(pcVar23,pcVar15,lVar1);
        local_3d0 = pvVar18;
        *(undefined8 *)(puVar28 + -8) = 0x104748;
        uVar13 = quotearg_n_style(1,4,pvVar18);
        pcVar15 = local_5a8;
        *(undefined8 *)(puVar28 + -8) = 0x104764;
        uVar14 = quotearg_n_style(0,4,pcVar15);
        *(undefined8 *)(puVar28 + -8) = 0x104776;
        uVar20 = gettext("will not create hard link %s to directory %s");
        local_4d8 = 0;
        local_4d4 = 0;
        local_328 = uVar20;
        *(undefined8 *)(puVar28 + -8) = 0x1047b6;
        error(0,0,uVar20,uVar14,uVar13);
        pvVar18 = local_3d0;
        *(undefined8 *)(puVar28 + -8) = 0x1047c6;
        free(pvVar18);
      }
      else {
        *(undefined8 *)(puVar28 + -8) = 0x1045a1;
        uVar13 = quotearg_n_style(1,4,top_level_dst_name);
        *(undefined8 *)(puVar28 + -8) = 0x1045bd;
        uVar14 = quotearg_n_style(0,4,top_level_src_name);
        *(undefined8 *)(puVar28 + -8) = 0x1045cf;
        uVar20 = gettext("cannot copy a directory, %s, into itself, %s");
        local_4e8 = 0;
        local_4e4 = 0;
        local_338 = uVar20;
        *(undefined8 *)(puVar28 + -8) = 0x10460f;
        error(0,0,uVar20,uVar14,uVar13);
        *local_5e8 = 1;
      }
    }
    else {
      bVar6 = *(byte *)(local_5d0 + 0x10);
      *(ulong *)(puVar28 + -0x10) = (ulong)local_588;
      *(ulong *)(puVar28 + -0x18) = (ulong)bVar6;
      *(undefined8 *)(puVar28 + -0x20) = 1;
      *(undefined8 *)(puVar28 + -0x28) = 0x104816;
      cVar7 = create_hard_link(0,iVar9,lVar1,pcVar23,iVar9,pcVar15);
      auVar2._8_4_ = (__mode_t)local_b0;
      auVar2._12_4_ = local_b0._4_4_;
      auVar2._0_8_ = local_c8._16_8_;
      if (cVar7 == '\x01') {
        uVar8 = 1;
        goto LAB_00106476;
      }
    }
  }
  else {
    if ((local_578 != 0x11) || ((local_5d0[0xf] != 2 && (local_5d0[0xf] != 3)))) {
      if ((((local_584 & 0xf000) == 0x8000) ||
          ((*(char *)(local_5d0 + 5) == '\x01' &&
           ((((local_584 & 0xf000) != 0x4000 || (*(char *)((long)local_5d0 + 0x41) == '\x01')) &&
            ((local_584 & 0xf000) != 0xa000)))))) &&
         ((((*(char *)(local_5d0 + 6) == '\0' && (*(char *)((long)local_5d0 + 0x3a) == '\0')) &&
           (*(char *)((long)local_5d0 + 0x17) == '\0')) &&
          ((*local_5d0 == 0 && (*(char *)((long)local_5d0 + 0x15) == '\0')))))) {
        local_587 = '\0';
      }
      else {
        local_587 = '\x01';
      }
      if ((local_587 == '\x01') || (-1 < (int)local_5b0)) {
        if (local_587 == '\0') {
          uVar13 = 0;
        }
        else {
          uVar13 = 0x100;
        }
        iVar9 = follow_fstatat(local_5ac,local_418,&local_1e8,uVar13);
        if (iVar9 < 0) {
          piVar12 = __errno_location();
          if (*piVar12 == 2) {
            local_591 = 1;
          }
          else {
            piVar12 = __errno_location();
            if (((*piVar12 != 0x28) || (local_587 == '\x01')) ||
               (*(char *)((long)local_5d0 + 0x16) == '\0')) {
              uVar13 = quotearg_style(4,local_5a8);
              uVar14 = gettext("cannot stat %s");
              piVar12 = __errno_location();
              local_52c = *piVar12;
              local_530 = 0;
              local_380 = uVar14;
              error(0,local_52c,uVar14,uVar13);
              auVar2._8_4_ = (__mode_t)local_b0;
              auVar2._12_4_ = local_b0._4_4_;
              auVar2._0_8_ = local_c8._16_8_;
              puVar28 = auStack_5f8;
              uVar8 = 0;
              goto LAB_00106476;
            }
          }
        }
        else {
          local_58b = local_587;
          local_578 = 0x11;
        }
      }
      else {
        local_591 = 1;
      }
    }
    puVar26 = auStack_5f8;
    if (local_578 != 0x11) goto LAB_0010420b;
    local_590 = '\0';
    local_58a = 1;
    local_589 = '\0';
    if (((local_5d0[0xf] != 2) && (local_5d0[0xf] != 3)) &&
       (cVar7 = same_file_ok(local_5a0,&local_278,local_5ac,local_418,&local_1e8,local_5d0,
                             &local_590), cVar7 != '\x01')) {
      uVar13 = quotearg_n_style(1,4,local_5a8);
      uVar14 = quotearg_n_style(0,4,local_5a0);
      local_378 = gettext("%s and %s are the same file");
      local_528 = 0;
      local_524 = 0;
      error(0,0,local_378,uVar14,uVar13);
      auVar2._8_4_ = (__mode_t)local_b0;
      auVar2._12_4_ = local_b0._4_4_;
      auVar2._0_8_ = local_c8._16_8_;
      puVar28 = auStack_5f8;
      uVar8 = 0;
      goto LAB_00106476;
    }
    if ((local_5d0[0xf] != 1) || ((local_584 & 0xf000) == 0x4000)) {
LAB_0010394a:
      if (*(char *)(local_5d0 + 6) == '\0') {
        if (((local_584 & 0xf000) != 0x4000) &&
           (((local_5d0[0xf] == 2 || (local_5d0[0xf] == 3)) ||
            ((local_5d0[2] == 3 &&
             (cVar7 = overwrite_ok(local_5d0,local_5a8,local_5ac,local_5b8,&local_1e8),
             cVar7 != '\x01')))))) {
          local_589 = '\x01';
          local_58a = local_5d0[0xf] == 2;
        }
      }
      else {
        cVar7 = abandon_move(local_5d0,local_5a8,local_5ac,local_418,&local_1e8);
        if (cVar7 != '\0') {
          if (local_5f0 != (byte *)0x0) {
            *local_5f0 = 1;
          }
          local_589 = '\x01';
          local_58a = local_5d0[0xf] == 2;
        }
      }
LAB_00103a5b:
      if (local_589 != '\0') {
        if (local_5d0[0xf] == 3) {
          uVar13 = quotearg_style(4,local_5a8);
          local_370 = gettext("not replacing %s");
          local_520 = 0;
          local_51c = 0;
          error(0,0,local_370,uVar13);
        }
        else if (*(char *)((long)local_5d0 + 0x42) != '\0') {
          uVar13 = quotearg_style(4,local_5a8);
          pcVar15 = (char *)gettext("skipped %s\n");
          printf(pcVar15,uVar13);
        }
        local_590 = '\x01';
      }
      auVar2._8_4_ = (__mode_t)local_b0;
      auVar2._12_4_ = local_b0._4_4_;
      auVar2._0_8_ = local_c8._16_8_;
      uVar8 = local_58a;
      if (local_590 != '\0') goto LAB_00106476;
      if (((((local_1e8.st_mode & 0xf000) != 0x4000) != ((local_584 & 0xf000) != 0x4000)) &&
          (*local_5d0 == 0)) && (*(char *)((long)local_5d0 + 0x1a) != '\x01')) {
        uVar13 = quotearg_n_style(1,4,local_5a0);
        uVar14 = quotearg_n_style(0,4,local_5a8);
        if ((local_584 & 0xf000) == 0x4000) {
          pcVar15 = "cannot overwrite non-directory %s with directory %s";
        }
        else {
          pcVar15 = "cannot overwrite directory %s with non-directory %s";
        }
        local_368 = gettext(pcVar15);
        local_518 = 0;
        local_514 = 0;
        error(0,0,local_368,uVar14,uVar13);
        auVar2._8_4_ = (__mode_t)local_b0;
        auVar2._12_4_ = local_b0._4_4_;
        auVar2._0_8_ = local_c8._16_8_;
        puVar28 = auStack_5f8;
        uVar8 = 0;
        goto LAB_00106476;
      }
      if ((((local_1e8.st_mode & 0xf000) != 0x4000) && (local_5d4 != '\0')) &&
         ((*local_5d0 != 3 &&
          ((*(char *)((long)local_5d0 + 0x1a) != '\x01' &&
           (cVar7 = seen_file(*(undefined8 *)(local_5d0 + 0x14),local_5b8,&local_1e8), cVar7 != '\0'
           )))))) {
        uVar13 = quotearg_n_style(1,4,local_5a0);
        uVar14 = quotearg_n_style(0,4,local_5a8);
        local_360 = gettext("will not overwrite just-created %s with %s");
        local_510 = 0;
        local_50c = 0;
        error(0,0,local_360,uVar14,uVar13);
        auVar2._8_4_ = (__mode_t)local_b0;
        auVar2._12_4_ = local_b0._4_4_;
        auVar2._0_8_ = local_c8._16_8_;
        puVar28 = auStack_5f8;
        uVar8 = 0;
        goto LAB_00106476;
      }
      if (*local_5d0 != 0) {
        local_400 = last_component(local_5a0);
        cVar7 = dot_or_dotdot(local_400);
        if ((cVar7 != '\x01') &&
           ((*(char *)(local_5d0 + 6) != '\0' || ((local_1e8.st_mode & 0xf000) != 0x4000)))) {
          if ((*local_5d0 != 3) &&
             (cVar7 = source_is_dst_backup(local_400,&local_278,local_5ac,local_5b8), cVar7 != '\0')
             ) {
            if (*(char *)(local_5d0 + 6) == '\0') {
              local_3f8 = gettext("backing up %s might destroy source;  %s not copied");
            }
            else {
              local_3f8 = gettext("backing up %s might destroy source;  %s not moved");
            }
            uVar13 = quotearg_n_style(1,4,local_5a0);
            uVar14 = quotearg_n_style(0,4,local_5a8);
            local_508 = 0;
            local_504 = 0;
            local_358 = local_3f8;
            error(0,0,local_3f8,uVar14,uVar13);
            auVar2._8_4_ = (__mode_t)local_b0;
            auVar2._12_4_ = local_b0._4_4_;
            auVar2._0_8_ = local_c8._16_8_;
            puVar28 = auStack_5f8;
            uVar8 = 0;
            goto LAB_00106476;
          }
          local_3f0 = (char *)backup_file_rename(local_5ac,local_5b8,*local_5d0);
          if (local_3f0 == (char *)0x0) {
            piVar12 = __errno_location();
            if (*piVar12 != 2) {
              uVar13 = quotearg_style(4,local_5a8);
              uVar14 = gettext("cannot backup %s");
              piVar12 = __errno_location();
              local_4fc = *piVar12;
              local_500 = 0;
              local_350 = uVar14;
              error(0,local_4fc,uVar14,uVar13);
              auVar2._8_4_ = (__mode_t)local_b0;
              auVar2._12_4_ = local_b0._4_4_;
              auVar2._0_8_ = local_c8._16_8_;
              puVar28 = auStack_5f8;
              uVar8 = 0;
              goto LAB_00106476;
            }
          }
          else {
            local_3e8 = (long)local_5b8 - (long)local_5a8;
            sVar16 = strlen(local_3f0);
            local_3e0 = sVar16 + 1;
            uVar17 = ((sVar16 + 1 + local_3e8 + 0x17) / 0x10) * 0x10;
            for (; puVar24 != auStack_5f8 + -(uVar17 & 0xfffffffffffff000);
                puVar24 = puVar24 + -0x1000) {
              *(undefined8 *)(puVar24 + -8) = *(undefined8 *)(puVar24 + -8);
            }
            lVar1 = -(ulong)((uint)uVar17 & 0xfff);
            puVar25 = puVar24 + lVar1;
            if ((uVar17 & 0xfff) != 0) {
              *(undefined8 *)(puVar24 + ((ulong)((uint)uVar17 & 0xfff) - 8) + lVar1) =
                   *(undefined8 *)(puVar24 + ((ulong)((uint)uVar17 & 0xfff) - 8) + lVar1);
            }
            sVar22 = local_3e0;
            sVar16 = local_3e8;
            pcVar15 = local_5a8;
            local_420 = (void *)((ulong)(puVar24 + lVar1 + 0xf) & 0xfffffffffffffff0);
            *(undefined8 *)(puVar24 + lVar1 + -8) = 0x103fd0;
            pvVar18 = mempcpy((void *)((ulong)(puVar24 + lVar1 + 0xf) & 0xfffffffffffffff0),pcVar15,
                              sVar16);
            pcVar15 = local_3f0;
            *(undefined8 *)(puVar24 + lVar1 + -8) = 0x103fe8;
            memcpy(pvVar18,pcVar15,sVar22);
            pcVar15 = local_3f0;
            *(undefined8 *)(puVar24 + lVar1 + -8) = 0x103ff7;
            free(pcVar15);
          }
          local_591 = 1;
          puVar26 = puVar25;
          goto LAB_0010420b;
        }
      }
      puVar26 = auStack_5f8;
      if ((((local_1e8.st_mode & 0xf000) != 0x4000) &&
          (puVar26 = auStack_5f8, *(char *)(local_5d0 + 6) != '\x01')) &&
         ((*(char *)((long)local_5d0 + 0x15) != '\0' ||
          ((puVar26 = auStack_5f8, *(char *)((long)local_5d0 + 0x31) != '\0' &&
           (((*(char *)(local_5d0 + 0xc) != '\0' && (1 < local_1e8.st_nlink)) ||
            ((puVar26 = auStack_5f8, local_5d0[1] == 2 &&
             (puVar26 = auStack_5f8, (local_260 & 0xf000) != 0x8000)))))))))) {
        iVar9 = unlinkat(local_5ac,local_5b8,0);
        if ((iVar9 != 0) && (piVar12 = __errno_location(), *piVar12 != 2)) {
          uVar13 = quotearg_style(4,local_5a8);
          uVar14 = gettext("cannot remove %s");
          piVar12 = __errno_location();
          local_4f4 = *piVar12;
          local_4f8 = 0;
          local_348 = uVar14;
          error(0,local_4f4,uVar14,uVar13);
          auVar2._8_4_ = (__mode_t)local_b0;
          auVar2._12_4_ = local_b0._4_4_;
          auVar2._0_8_ = local_c8._16_8_;
          puVar28 = auStack_5f8;
          uVar8 = 0;
          goto LAB_00106476;
        }
        local_591 = 1;
        puVar26 = auStack_5f8;
        if (*(char *)(local_5d0 + 0x10) != '\0') {
          uVar13 = quotearg_style(4,local_5a8);
          pcVar15 = (char *)gettext("removed %s\n");
          printf(pcVar15,uVar13);
          puVar26 = auStack_5f8;
        }
      }
      goto LAB_0010420b;
    }
    if ((*(char *)((long)local_5d0 + 0x21) == '\0') ||
       ((*(char *)(local_5d0 + 6) == '\x01' && (local_1e8.st_dev == local_278)))) {
      local_564 = 0;
    }
    else {
      local_564 = 1;
    }
    iVar9 = utimecmpat(local_5ac,local_5b8,&local_1e8,&local_278,local_564);
    if (iVar9 < 0) goto LAB_0010394a;
    if (local_5f0 != (byte *)0x0) {
      *local_5f0 = 1;
    }
    local_428 = remember_copied(local_5b8,local_270,local_278);
    if ((local_428 == 0) ||
       (cVar7 = create_hard_link(0,local_5ac,local_428,local_5a8,local_5ac,local_5b8,1,
                                 *(undefined *)(local_5d0 + 0x10),local_588), cVar7 == '\x01')) {
      local_589 = '\x01';
      goto LAB_00103a5b;
    }
  }
LAB_00106311:
  if (*(char *)((long)local_5d0 + 0x33) != '\0') {
    *(undefined8 *)(puVar27 + -8) = 0x106325;
    restore_default_fscreatecon_or_die();
  }
  if (local_428 == 0) {
    *(undefined8 *)(puVar27 + -8) = 0x106348;
    forget_created(local_270,local_278);
  }
  pcVar15 = local_418;
  iVar9 = local_5ac;
  if (local_420 != (void *)0x0) {
    pcVar23 = (char *)((long)local_420 + ((long)local_5b8 - (long)local_5a8));
    local_3a0 = pcVar23;
    *(undefined8 *)(puVar27 + -8) = 0x106399;
    iVar9 = renameat(iVar9,pcVar23,iVar9,pcVar15);
    pcVar15 = local_5a8;
    if (iVar9 == 0) {
      if (*(char *)(local_5d0 + 0x10) != '\0') {
        *(undefined8 *)(puVar27 + -8) = 0x106430;
        uVar13 = quotearg_n_style(1,4,pcVar15);
        pvVar18 = local_420;
        *(undefined8 *)(puVar27 + -8) = 0x10644c;
        uVar14 = quotearg_n_style(0,4,pvVar18);
        *(undefined8 *)(puVar27 + -8) = 0x10645e;
        pcVar15 = (char *)gettext("%s -> %s (unbackup)\n");
        *(undefined8 *)(puVar27 + -8) = 0x106471;
        printf(pcVar15,uVar14,uVar13);
      }
    }
    else {
      *(undefined8 *)(puVar27 + -8) = 0x1063b1;
      uVar13 = quotearg_style(4,pcVar15);
      *(undefined8 *)(puVar27 + -8) = 0x1063c3;
      uVar14 = gettext("cannot un-backup %s");
      *(undefined8 *)(puVar27 + -8) = 0x1063cb;
      piVar12 = __errno_location();
      iVar9 = *piVar12;
      local_430 = 0;
      local_42c = iVar9;
      local_280 = uVar14;
      *(undefined8 *)(puVar27 + -8) = 0x106406;
      error(0,iVar9,uVar14,uVar13);
    }
  }
  auVar2._8_4_ = (__mode_t)local_b0;
  auVar2._12_4_ = local_b0._4_4_;
  auVar2._0_8_ = local_c8._16_8_;
  puVar28 = puVar27;
  uVar8 = 0;
LAB_00106476:
  if (local_30 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    unique0x10003046 = auVar2;
    *(undefined8 *)(puVar28 + -8) = 0x10648a;
    __stack_chk_fail();
  }
  return uVar8;
}

// Function: valid_options
void valid_options(uint *param_1) {
  if (3 < *param_1) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("((unsigned int) (co->backup_type) <= numbered_backups)",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/copy.c",
                  0xae0,"valid_options");
  }
  if (((param_1[3] != 1) && (param_1[3] != 2)) && (param_1[3] != 3)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("((co->sparse_mode) == SPARSE_NEVER || (co->sparse_mode) == SPARSE_AUTO || (co->sparse_mode) == SPARSE_ALWAYS)"
                  ,"/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/copy.c",
                  0xae1,"valid_options");
  }
  if (((param_1[0x13] != 0) && (param_1[0x13] != 1)) && (param_1[0x13] != 2)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("((co->reflink_mode) == REFLINK_NEVER || (co->reflink_mode) == REFLINK_AUTO || (co->reflink_mode) == REFLINK_ALWAYS)"
                  ,"/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/copy.c",
                  0xae2,"valid_options");
  }
  if ((*(char *)((long)param_1 + 0x17) == '\x01') && (*(char *)((long)param_1 + 0x3a) == '\x01')) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("!(co->hard_link && co->symbolic_link)",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/copy.c",
                  0xae3,"valid_options");
  }
  if ((param_1[0x13] == 2) && (param_1[3] != 2)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("! (co->reflink_mode == REFLINK_ALWAYS && co->sparse_mode != SPARSE_AUTO)",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/copy.c",
                  0xae4,"valid_options");
  }
  return;
}

// Function: copy
int copy(EVP_PKEY_CTX *dst,EVP_PKEY_CTX *src) {
  int iVar1;
  undefined8 in_RCX;
  undefined4 in_EDX;
  undefined4 in_R8D;
  undefined8 in_R9;
  long in_FS_OFFSET;
  undefined8 in_stack_00000008;
  undefined8 in_stack_00000010;
  undefined local_11;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  valid_options(in_R9);
  local_11 = 0;
  top_level_src_name = dst;
  top_level_dst_name = src;
  iVar1 = copy_internal(dst,src,in_EDX,in_RCX,in_R8D,0,0,in_R9,1,&local_11,in_stack_00000008,
                        in_stack_00000010);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return iVar1;
}

// Function: cp_options_default
void cp_options_default(void *param_1) {
  __uid_t _Var1;
  
  memset(param_1,0,0x60);
  _Var1 = geteuid();
  *(bool *)((long)param_1 + 0x1d) = _Var1 == 0;
  *(undefined *)((long)param_1 + 0x1c) = *(undefined *)((long)param_1 + 0x1d);
  *(undefined4 *)((long)param_1 + 0x48) = 0xffffffff;
  return;
}

// Function: chown_failure_ok
undefined4 chown_failure_ok(long param_1) {
  int *piVar1;
  
  piVar1 = __errno_location();
  if ((((*piVar1 == 1) || (piVar1 = __errno_location(), *piVar1 == 0x16)) ||
      (piVar1 = __errno_location(), *piVar1 == 0xd)) && (*(char *)(param_1 + 0x1c) != '\x01')) {
    return 1;
  }
  return 0;
}

// Function: owner_failure_ok
undefined4 owner_failure_ok(long param_1) {
  int *piVar1;
  
  piVar1 = __errno_location();
  if ((((*piVar1 == 1) || (piVar1 = __errno_location(), *piVar1 == 0x16)) ||
      (piVar1 = __errno_location(), *piVar1 == 0xd)) && (*(char *)(param_1 + 0x1d) != '\x01')) {
    return 1;
  }
  return 0;
}

// Function: cached_umask
__mode_t cached_umask(void) {
  if (cached_1 != '\x01') {
    cached_1 = '\x01';
    mask_0 = umask(0);
    umask(mask_0);
  }
  return mask_0;
}

