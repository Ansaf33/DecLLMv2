// Function: cache_fstatat
undefined8 cache_fstatat(int param_1,char *param_2,stat *param_3,int param_4) {
  __ino_t _Var1;
  int iVar2;
  undefined8 uVar3;
  int *piVar4;
  
  if ((param_3->st_atim).tv_nsec < 0) {
    if ((param_3->st_atim).tv_nsec == -1) {
      iVar2 = fstatat(param_1,param_2,param_3,param_4);
      if (iVar2 == 0) {
        return 0;
      }
      (param_3->st_atim).tv_nsec = -2;
      piVar4 = __errno_location();
      param_3->st_ino = (long)*piVar4;
    }
    _Var1 = param_3->st_ino;
    piVar4 = __errno_location();
    *piVar4 = (int)_Var1;
    uVar3 = 0xffffffff;
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

// Function: cache_stat_init
long cache_stat_init(long param_1) {
  *(undefined8 *)(param_1 + 0x50) = 0xffffffffffffffff;
  return param_1;
}

// Function: write_protected_non_symlink
undefined8 write_protected_non_symlink(int param_1,char *param_2,long param_3) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  int *piVar4;
  
  cVar1 = can_write_any_file();
  if (cVar1 == '\0') {
    iVar2 = cache_fstatat(param_1,param_2,param_3,0x100);
    if (iVar2 == 0) {
      if ((*(uint *)(param_3 + 0x18) & 0xf000) == 0xa000) {
        uVar3 = 0;
      }
      else {
        iVar2 = faccessat(param_1,param_2,2,0x200);
        if (iVar2 == 0) {
          uVar3 = 0;
        }
        else {
          piVar4 = __errno_location();
          if (*piVar4 == 0xd) {
            uVar3 = 1;
          }
          else {
            uVar3 = 0xffffffff;
          }
        }
      }
    }
    else {
      uVar3 = 0xffffffff;
    }
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

// Function: get_dir_status
int get_dir_status(long param_1,long param_2,int *param_3) {
  int iVar1;
  
  if (*param_3 == -2) {
    iVar1 = directory_status(*(undefined4 *)(param_1 + 0x2c),*(undefined8 *)(param_2 + 0x30));
    *param_3 = iVar1;
  }
  return *param_3;
}

// Function: prompt
undefined8 prompt(long param_1,long param_2,char param_3,char *param_4,int param_5,int *param_6) {
  undefined4 uVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  int *piVar5;
  undefined8 uVar6;
  char *pcVar7;
  undefined8 uVar8;
  long in_FS_OFFSET;
  int local_118;
  int local_114;
  int local_110;
  undefined local_b8 [24];
  uint local_a0;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  uVar1 = *(undefined4 *)(param_1 + 0x2c);
  uVar4 = *(undefined8 *)(param_2 + 0x38);
  uVar6 = *(undefined8 *)(param_2 + 0x30);
  cache_stat_init(local_b8);
  if (param_3 == '\0') {
    local_118 = 0;
  }
  else {
    local_118 = 4;
  }
  local_114 = 0;
  if (*(long *)(param_2 + 0x20) == 0) {
    if (*(int *)(param_4 + 4) == 5) {
      uVar4 = 2;
    }
    else {
      local_110 = 0;
      if ((*param_4 != '\x01') &&
         (((*(int *)(param_4 + 4) == 3 || (param_4[0x19] != '\0')) && (local_118 != 10)))) {
        local_114 = write_protected_non_symlink(uVar1,uVar6,local_b8);
        piVar5 = __errno_location();
        local_110 = *piVar5;
      }
      if ((local_114 == 0) && (*(int *)(param_4 + 4) != 3)) {
        uVar4 = 2;
      }
      else {
        if ((-1 < local_114) && (local_118 == 0)) {
          iVar3 = cache_fstatat(uVar1,uVar6,local_b8,0x100);
          if (iVar3 == 0) {
            if ((local_a0 & 0xf000) == 0xa000) {
              local_118 = 10;
            }
            else if ((local_a0 & 0xf000) == 0x4000) {
              local_118 = 4;
            }
          }
          else {
            local_114 = -1;
            piVar5 = __errno_location();
            local_110 = *piVar5;
          }
        }
        if (-1 < local_114) {
          if (local_118 == 4) {
            if ((param_4[9] != '\x01') &&
               ((param_4[10] != '\x01' ||
                (iVar3 = get_dir_status(param_1,param_2,param_6), iVar3 == 0)))) {
              local_114 = -1;
              if (*param_6 < 1) {
                local_110 = 0x15;
              }
              else {
                local_110 = *param_6;
              }
            }
          }
          else if ((local_118 == 10) && (*(int *)(param_4 + 4) != 3)) {
            uVar4 = 2;
            goto LAB_001008c1;
          }
        }
        uVar4 = quotearg_style(4,uVar4);
        if (local_114 < 0) {
          uVar6 = gettext("cannot remove %s");
          error(0,local_110,uVar6,uVar4);
          uVar4 = 5;
        }
        else {
          if (((local_118 == 4) && (param_5 == 2)) &&
             (iVar3 = get_dir_status(param_1,param_2,param_6), uVar8 = _program_name, iVar3 == 0)) {
            if (local_114 == 0) {
              pcVar7 = (char *)gettext("%s: descend into directory %s? ");
            }
            else {
              pcVar7 = (char *)gettext("%s: descend into write-protected directory %s? ");
            }
            fprintf(_stderr,pcVar7,uVar8,uVar4);
          }
          else {
            uVar8 = _program_name;
            if (*param_6 < 1) {
              iVar3 = cache_fstatat(uVar1,uVar6,local_b8,0x100);
              if (iVar3 != 0) {
                uVar6 = gettext("cannot remove %s");
                piVar5 = __errno_location();
                error(0,*piVar5,uVar6,uVar4);
                uVar4 = 5;
                goto LAB_001008c1;
              }
              uVar8 = file_type(local_b8);
              uVar6 = _program_name;
              if (local_114 == 0) {
                pcVar7 = (char *)gettext("%s: remove %s %s? ");
              }
              else {
                pcVar7 = (char *)gettext("%s: remove write-protected %s %s? ");
              }
              fprintf(_stderr,pcVar7,uVar6,uVar8,uVar4);
            }
            else {
              if ((param_4[10] != '\x01') || (*param_6 != 0xd)) {
                uVar6 = gettext("cannot remove %s");
                error(0,*param_6,uVar6,uVar4);
                uVar4 = 5;
                goto LAB_001008c1;
              }
              if (param_5 == 2) {
                uVar4 = 2;
                goto LAB_001008c1;
              }
              pcVar7 = (char *)gettext("%s: attempt removal of inaccessible directory %s? ");
              fprintf(_stderr,pcVar7,uVar8,uVar4);
            }
          }
          cVar2 = yesno();
          if (cVar2 == '\0') {
            uVar4 = 4;
          }
          else {
            uVar4 = 3;
          }
        }
      }
    }
  }
  else {
    uVar4 = 4;
  }
LAB_001008c1:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar4;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: nonexistent_file_errno
undefined8 nonexistent_file_errno(int param_1) {
  undefined8 uVar1;
  
  if ((param_1 == 0x54) ||
     ((param_1 < 0x55 &&
      ((param_1 == 0x16 || ((param_1 < 0x17 && ((param_1 == 2 || (param_1 == 0x14)))))))))) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: ignorable_missing
undefined4 ignorable_missing(char *param_1,undefined4 param_2) {
  char cVar1;
  
  if ((*param_1 != '\0') && (cVar1 = nonexistent_file_errno(param_2), cVar1 != '\0')) {
    return 1;
  }
  return 0;
}

// Function: fts_skip_tree
void fts_skip_tree(undefined8 param_1,undefined8 param_2) {
  rpl_fts_set(param_1,param_2,4);
  rpl_fts_read(param_1);
  return;
}

// Function: mark_ancestor_dirs
void mark_ancestor_dirs(long param_1) {
  long local_10;
  
  for (local_10 = *(long *)(param_1 + 8);
      (-1 < *(long *)(local_10 + 0x58) && (*(long *)(local_10 + 0x20) == 0));
      local_10 = *(long *)(local_10 + 8)) {
    *(undefined8 *)(local_10 + 0x20) = 1;
  }
  return;
}

// Function: excise
undefined8 excise(long param_1,long param_2,long param_3,char param_4) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  char *__format;
  int *piVar4;
  undefined8 uVar5;
  long in_FS_OFFSET;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (param_4 == '\0') {
    iVar2 = 0;
  }
  else {
    iVar2 = 0x200;
  }
  iVar2 = unlinkat(*(int *)(param_1 + 0x2c),*(char **)(param_2 + 0x30),iVar2);
  if (iVar2 == 0) {
    if (*(char *)(param_3 + 0x1a) != '\0') {
      uVar3 = quotearg_style(4,*(undefined8 *)(param_2 + 0x38));
      if (param_4 == '\0') {
        __format = (char *)gettext("removed %s\n");
      }
      else {
        __format = (char *)gettext("removed directory %s\n");
      }
      printf(__format,uVar3);
    }
    uVar3 = 2;
    goto LAB_00100c2a;
  }
  piVar4 = __errno_location();
  if (*piVar4 == 0x1e) {
    iVar2 = fstatat(*(int *)(param_1 + 0x2c),*(char **)(param_2 + 0x30),&local_b8,0x100);
    if (iVar2 != 0) {
      piVar4 = __errno_location();
      if (*piVar4 == 2) goto LAB_00100b22;
    }
    piVar4 = __errno_location();
    *piVar4 = 0x1e;
  }
LAB_00100b22:
  piVar4 = __errno_location();
  cVar1 = ignorable_missing(param_3,*piVar4);
  if (cVar1 != '\0') {
    uVar3 = 2;
    goto LAB_00100c2a;
  }
  if (*(short *)(param_2 + 0x68) == 4) {
    piVar4 = __errno_location();
    if (*piVar4 != 0x27) {
      piVar4 = __errno_location();
      if (*piVar4 != 0x15) {
        piVar4 = __errno_location();
        if (*piVar4 != 0x14) {
          piVar4 = __errno_location();
          if (*piVar4 != 0x11) goto LAB_00100ba8;
        }
      }
    }
    if (*(int *)(param_2 + 0x40) != 0) {
      piVar4 = __errno_location();
      *piVar4 = *(int *)(param_2 + 0x40);
    }
  }
LAB_00100ba8:
  uVar3 = quotearg_style(4,*(undefined8 *)(param_2 + 0x38));
  uVar5 = gettext("cannot remove %s");
  piVar4 = __errno_location();
  error(0,*piVar4,uVar5,uVar3);
  mark_ancestor_dirs(param_2);
  uVar3 = 5;
LAB_00100c2a:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar3;
}

// Function: rm_fts
ulong rm_fts(long param_1,long param_2,long param_3) {
  ushort uVar1;
  undefined2 uVar2;
  bool bVar3;
  char cVar4;
  undefined uVar5;
  int iVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  ulong uVar10;
  long in_FS_OFFSET;
  int local_1a4;
  uint local_1a0;
  uint local_19c;
  undefined4 local_198;
  undefined4 local_194;
  undefined4 local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined4 local_15c;
  undefined4 local_158;
  undefined4 local_154;
  undefined4 local_150;
  undefined4 local_14c;
  undefined4 local_148;
  undefined4 local_144;
  undefined4 local_140;
  undefined4 local_13c;
  char *local_138;
  undefined8 local_130;
  undefined8 local_128;
  undefined8 local_120;
  undefined8 local_118;
  undefined8 local_110;
  undefined8 local_108;
  undefined8 local_100;
  undefined8 local_f8;
  undefined8 local_f0;
  undefined8 local_e8;
  undefined8 local_e0;
  undefined8 local_d8;
  undefined8 local_d0;
  stat local_c8;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  local_1a4 = -2;
  uVar1 = *(ushort *)(param_2 + 0x68);
  if (uVar1 == 1) {
    if (*(char *)(param_3 + 9) == '\x01') {
      if (*(long *)(param_2 + 0x58) == 0) {
        uVar9 = last_component(*(undefined8 *)(param_2 + 0x30));
        cVar4 = dot_or_dotdot(uVar9);
        if (cVar4 != '\0') {
          uVar9 = quotearg_n_style(2,4,*(undefined8 *)(param_2 + 0x38));
          uVar7 = quotearg_n_style(1,4,&DAT_001019d0);
          uVar8 = quotearg_n_style(0,4,&DAT_001019d3);
          local_120 = gettext("refusing to remove %s or %s directory: skipping %s");
          local_190 = 0;
          local_18c = 0;
          error(0,0,local_120,uVar8,uVar7,uVar9);
          fts_skip_tree(param_1,param_2);
          uVar10 = 5;
          goto LAB_00101709;
        }
        if ((*(long *)(param_3 + 0x10) != 0) &&
           ((**(ulong **)(param_3 + 0x10) ^ *(ulong *)(param_2 + 0x78) |
            *(ulong *)(param_2 + 0x70) ^ *(ulong *)(*(long *)(param_3 + 0x10) + 8)) == 0)) {
          cVar4 = streq(*(undefined8 *)(param_2 + 0x38),&DAT_00101a0b);
          if (cVar4 == '\0') {
            uVar9 = quotearg_n_style(1,4,&DAT_00101a0b);
            uVar7 = quotearg_n_style(0,4,*(undefined8 *)(param_2 + 0x38));
            local_110 = gettext("it is dangerous to operate recursively on %s (same as %s)");
            local_180 = 0;
            local_17c = 0;
            error(0,0,local_110,uVar7,uVar9);
          }
          else {
            uVar9 = quotearg_style(4,*(undefined8 *)(param_2 + 0x38));
            local_118 = gettext("it is dangerous to operate recursively on %s");
            local_188 = 0;
            local_184 = 0;
            error(0,0,local_118,uVar9);
          }
          local_108 = gettext("use --no-preserve-root to override this failsafe");
          local_178 = 0;
          local_174 = 0;
          error(0,0,local_108);
          fts_skip_tree(param_1,param_2);
          uVar10 = 5;
          goto LAB_00101709;
        }
        if (*(char *)(param_3 + 0x18) != '\0') {
          bVar3 = false;
          local_138 = (char *)file_name_concat(*(undefined8 *)(param_2 + 0x30),&DAT_001019d0,0);
          if ((local_138 == (char *)0x0) || (iVar6 = lstat(local_138,&local_c8), iVar6 != 0)) {
            uVar9 = quotearg_n_style(1,4,*(undefined8 *)(param_2 + 0x30));
            uVar7 = quotearg_n_style(0,4,local_138);
            local_100 = gettext("failed to stat %s: skipping %s");
            local_170 = 0;
            local_16c = 0;
            error(0,0,local_100,uVar7,uVar9);
            bVar3 = true;
          }
          free(local_138);
          if ((bVar3) || (*(__dev_t *)(param_1 + 0x18) != local_c8.st_dev)) {
            if (!bVar3) {
              uVar9 = quotearg_style(4,*(undefined8 *)(param_2 + 0x38));
              local_f0 = gettext("skipping %s, since it\'s on a different device");
              local_160 = 0;
              local_15c = 0;
              error(0,0,local_f0,uVar9);
              local_f8 = gettext("and --preserve-root=all is in effect");
              local_168 = 0;
              local_164 = 0;
              error(0,0,local_f8);
            }
            fts_skip_tree(param_1,param_2);
            uVar10 = 5;
            goto LAB_00101709;
          }
        }
      }
      local_1a0 = prompt(param_1,param_2,1,param_3,2,&local_1a4);
      if (((local_1a0 == 3) && (local_1a4 == -1)) &&
         (local_1a0 = excise(param_1,param_2,param_3,1), local_1a0 == 2)) {
        fts_skip_tree(param_1,param_2);
      }
      if ((local_1a0 != 2) && (local_1a0 != 3)) {
        mark_ancestor_dirs(param_2);
        fts_skip_tree(param_1,param_2);
      }
      uVar10 = (ulong)local_1a0;
    }
    else {
      rpl_fts_set(param_1,param_2,4);
      if (*(char *)(param_3 + 10) == '\0') {
        uVar9 = quotearg_style(4,*(undefined8 *)(param_2 + 0x38));
        local_128 = gettext("cannot remove %s");
        local_198 = 0;
        local_194 = 0x15;
        error(0,0x15,local_128,uVar9);
        local_130 = rpl_fts_read(param_1);
        uVar10 = 5;
      }
      else {
        uVar10 = 2;
      }
    }
  }
  else {
    if (((uVar1 == 0) || (0xd < uVar1)) || (uVar1 < 2)) {
LAB_00101680:
      uVar9 = quotearg_n_style_colon(0,3,*(undefined8 *)(param_2 + 0x38));
      uVar2 = *(undefined2 *)(param_2 + 0x68);
      local_d0 = gettext("unexpected failure: fts_info=%d: %s\nplease report to %s");
      local_140 = 0;
      local_13c = 0;
      error(0,0,local_d0,uVar2,uVar9,"bug-coreutils@gnu.org");
                    /* WARNING: Subroutine does not return */
      abort();
    }
    uVar10 = 1L << ((byte)uVar1 & 0x3f);
    if ((uVar10 & 0x3d58) == 0) {
      if ((uVar10 & 0x80) == 0) {
        if ((uVar10 & 4) == 0) goto LAB_00101680;
        uVar9 = quotearg_n_style_colon(0,3,*(undefined8 *)(param_2 + 0x38));
        local_e0 = gettext(
                          "WARNING: Circular directory structure.\nThis almost certainly means that you have a corrupted file system.\nNOTIFY YOUR SYSTEM MANAGER.\nThe following directory is part of the cycle:\n  %s\n"
                          );
        local_150 = 0;
        local_14c = 0;
        error(0,0,local_e0,uVar9);
        fts_skip_tree(param_1,param_2);
        uVar10 = 5;
      }
      else {
        uVar9 = quotearg_n_style_colon(0,3,*(undefined8 *)(param_2 + 0x38));
        local_d8 = gettext("traversal failed: %s");
        local_144 = *(undefined4 *)(param_2 + 0x40);
        local_148 = 0;
        error(0,local_144,local_d8,uVar9);
        fts_skip_tree(param_1,param_2);
        uVar10 = 5;
      }
    }
    else if (((*(short *)(param_2 + 0x68) == 6) && (*(char *)(param_3 + 8) != '\0')) &&
            ((0 < *(long *)(param_2 + 0x58) &&
             (*(long *)(param_2 + 0x70) != *(long *)(param_1 + 0x18))))) {
      mark_ancestor_dirs(param_2);
      uVar9 = quotearg_style(4,*(undefined8 *)(param_2 + 0x38));
      local_e8 = gettext("skipping %s, since it\'s on a different device");
      local_158 = 0;
      local_154 = 0;
      error(0,0,local_e8,uVar9);
      uVar10 = 5;
    }
    else {
      if ((*(short *)(param_2 + 0x68) == 6) || (*(short *)(param_2 + 0x68) == 4)) {
        uVar5 = 1;
      }
      else {
        uVar5 = 0;
      }
      local_19c = prompt(param_1,param_2,uVar5,param_3,3,&local_1a4);
      if ((local_19c == 2) || (local_19c == 3)) {
        uVar10 = excise(param_1,param_2,param_3,uVar5);
      }
      else {
        uVar10 = (ulong)local_19c;
      }
    }
  }
LAB_00101709:
  if (local_30 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar10;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: rm
int rm(long *param_1,long param_2) {
  int iVar1;
  undefined8 uVar2;
  long lVar3;
  int *piVar4;
  undefined8 uVar5;
  int local_54;
  undefined4 local_50;
  
  local_54 = 2;
  if (*param_1 != 0) {
    local_50 = 0x218;
    if (*(char *)(param_2 + 8) != '\0') {
      local_50 = 600;
    }
    uVar2 = xfts_open(param_1,local_50,0);
    while( true ) {
      lVar3 = rpl_fts_read(uVar2);
      if (lVar3 == 0) break;
      iVar1 = rm_fts(uVar2,lVar3,param_2);
      if ((((iVar1 != 2) && (iVar1 != 3)) && (iVar1 != 4)) && (iVar1 != 5)) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("((s) == RM_OK || (s) == RM_USER_ACCEPTED || (s) == RM_USER_DECLINED || (s) == RM_ERROR)"
                      ,
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/remove.c"
                      ,0x279,(char *)&__PRETTY_FUNCTION___0);
      }
      if ((iVar1 == 5) || ((iVar1 == 4 && (local_54 == 2)))) {
        local_54 = iVar1;
      }
    }
    piVar4 = __errno_location();
    if (*piVar4 != 0) {
      uVar5 = gettext("fts_read failed");
      piVar4 = __errno_location();
      error(0,*piVar4,uVar5);
      local_54 = 5;
    }
    iVar1 = rpl_fts_close(uVar2);
    if (iVar1 != 0) {
      uVar2 = gettext("fts_close failed");
      piVar4 = __errno_location();
      error(0,*piVar4,uVar2);
      local_54 = 5;
    }
  }
  return local_54;
}

