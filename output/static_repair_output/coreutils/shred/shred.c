// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... FILE...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Overwrite the specified FILE(s) repeatedly, in order to make it harder\nfor even very expensive hardware probing to recover the data.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("\nIf FILE is -, shred standard output.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pcVar3 = (char *)gettext(
                            "  -f, --force    change permissions to allow writing if necessary\n  -n, --iterations=N  overwrite N times instead of the default (%d)\n      --random-source=FILE  get random bytes from FILE\n  -s, --size=N   shred this many bytes (suffixes like K, M, G accepted)\n"
                            );
    printf(pcVar3,3);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -u             deallocate and remove file after overwriting\n      --remove[=HOW]  like -u but give control on HOW to delete;  See below\n  -v, --verbose  show progress\n  -x, --exact    do not round file sizes up to the next full block;\n                   this is the default for non-regular files\n  -z, --zero     add a final overwrite with zeros to hide shredding\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nDelete FILE(s) if --remove (-u) is specified.  The default is not to remove\nthe files because it is common to operate on device files like /dev/hda,\nand those files usually should not be removed.\nThe optional HOW parameter indicates how to remove a directory entry:\n\'unlink\' => use a standard unlink call.\n\'wipe\' => also first obfuscate bytes in the name.\n\'wipesync\' => also sync each obfuscated byte to the device.\nThe default mode is \'wipesync\', but note it can be expensive.\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "CAUTION: shred assumes the file system and hardware overwrite data in place.\nAlthough this is common, many platforms operate otherwise.  Also, backups\nand mirrors may contain unremovable copies that will let a shredded file\nbe recovered later.  See the GNU coreutils manual for details.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info("shred");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: periodic_pattern
undefined4 periodic_pattern(uint param_1) {
  uint uVar1;
  char cVar2;
  undefined4 uVar3;
  long in_FS_OFFSET;
  
  if ((int)param_1 < 1) {
    uVar3 = 0;
  }
  else {
    param_1 = param_1 & 0xfff;
    uVar1 = param_1 | param_1 << 0xc;
    cVar2 = (char)(uVar1 >> 4);
    if ((cVar2 == (char)(uVar1 >> 8)) && (cVar2 == (char)param_1)) {
      uVar3 = 0;
    }
    else {
      uVar3 = 1;
    }
  }
  if (*(long *)(in_FS_OFFSET + 0x28) == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: fillpattern
void fillpattern(uint param_1,undefined *param_2,ulong param_3) {
  uint uVar1;
  uint uVar2;
  ulong local_10;
  
  uVar2 = param_1 & 0xfff;
  uVar1 = uVar2 | uVar2 << 0xc;
  *param_2 = (char)(uVar1 >> 4);
  param_2[1] = (char)(uVar1 >> 8);
  param_2[2] = (char)uVar2;
  for (local_10 = 3; local_10 <= param_3 >> 1; local_10 = local_10 << 1) {
    memcpy(param_2 + local_10,param_2,local_10);
  }
  if (local_10 < param_3) {
    memcpy(param_2 + local_10,param_2,param_3 - local_10);
  }
  if ((param_1 & 0x1000) != 0) {
    for (local_10 = 0; local_10 < param_3; local_10 = local_10 + 0x200) {
      param_2[local_10] = param_2[local_10] ^ 0x80;
    }
  }
  return;
}

// Function: passname
void passname(byte *param_1,char *param_2) {
  if (param_1 == (byte *)0x0) {
    memcpy(param_2,"random",7);
  }
  else {
    sprintf(param_2,"%02x%02x%02x",(ulong)*param_1,(ulong)param_1[1],(ulong)param_1[2]);
  }
  return;
}

// Function: ignorable_sync_errno
undefined4 ignorable_sync_errno(int param_1) {
  undefined4 uVar1;
  
  if (((param_1 == 0x16) || (param_1 == 9)) || (param_1 == 0x15)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: dosync
undefined8 dosync(int param_1,undefined8 param_2) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  int *piVar4;
  
  iVar2 = fdatasync(param_1);
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    piVar4 = __errno_location();
    iVar2 = *piVar4;
    cVar1 = ignorable_sync_errno(iVar2);
    if (cVar1 == '\x01') {
      iVar2 = fsync(param_1);
      if (iVar2 == 0) {
        uVar3 = 0;
      }
      else {
        piVar4 = __errno_location();
        iVar2 = *piVar4;
        cVar1 = ignorable_sync_errno(iVar2);
        if (cVar1 == '\x01') {
          sync();
          uVar3 = 0;
        }
        else {
          uVar3 = gettext("%s: fsync failed");
          error(0,iVar2,uVar3,param_2);
          piVar4 = __errno_location();
          *piVar4 = iVar2;
          uVar3 = 0xffffffff;
        }
      }
    }
    else {
      uVar3 = gettext("%s: fdatasync failed");
      error(0,iVar2,uVar3,param_2);
      piVar4 = __errno_location();
      *piVar4 = iVar2;
      uVar3 = 0xffffffff;
    }
  }
  return uVar3;
}

// Function: direct_mode
void direct_mode(undefined4 param_1,char param_2) {
  uint uVar1;
  uint uVar2;
  
  uVar1 = rpl_fcntl(param_1,3);
  if (0 < (int)uVar1) {
    if (param_2 == '\0') {
      uVar2 = uVar1 & 0xffffbfff;
    }
    else {
      uVar2 = uVar1 | 0x4000;
    }
    if (uVar2 != uVar1) {
      rpl_fcntl(param_1,4,uVar2);
    }
  }
  return;
}

// Function: dorewind
bool dorewind(int param_1,long param_2) {
  int iVar1;
  __off_t _Var2;
  int *piVar3;
  long in_FS_OFFSET;
  bool bVar4;
  undefined2 local_18 [2];
  undefined4 local_14;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if ((*(uint *)(param_2 + 0x18) & 0xf000) == 0x2000) {
    local_18[0] = 6;
    local_14 = 1;
    iVar1 = ioctl(param_1,0x40086d01,local_18);
    if (iVar1 == 0) {
      bVar4 = true;
      goto LAB_001008c6;
    }
  }
  _Var2 = lseek(param_1,0,0);
  if (0 < _Var2) {
    piVar3 = __errno_location();
    *piVar3 = 0x16;
  }
  bVar4 = _Var2 == 0;
LAB_001008c6:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return bVar4;
}

// Function: known
ulong known(ulong param_1) {
  return ~param_1 >> 0x3f;
}

// Function: dopass
ulong dopass(int param_1,undefined8 param_2,undefined8 param_3,ulong *param_4,int param_5,
            undefined8 param_6,undefined8 param_7,long param_8) {
  bool bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  ulong uVar6;
  long lVar7;
  undefined8 uVar8;
  int *piVar9;
  ulong uVar10;
  long lVar11;
  __off_t _Var12;
  char *pcVar13;
  long in_FS_OFFSET;
  bool bVar14;
  byte local_8e0;
  ulong local_898;
  ulong local_890;
  long local_888;
  long local_880;
  ulong local_878;
  ulong local_870;
  ssize_t local_868;
  char *local_860;
  char *local_858;
  undefined local_7df [7];
  char local_7d8 [656];
  undefined local_548 [656];
  undefined local_2b8 [664];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_890 = *param_4;
  local_888 = 0;
  local_880 = 0;
  iVar4 = getpagesize();
  cVar3 = periodic_pattern(param_5);
  if (cVar3 == '\0') {
    uVar6 = 0x10000;
  }
  else {
    uVar6 = 0xf000;
  }
  lVar7 = xalignalloc((long)iVar4,((uVar6 + 2) / 3) * 3);
  local_8e0 = 0;
  bVar2 = false;
  if (((long)local_890 < 1) || (uVar6 <= local_890)) {
    bVar1 = false;
  }
  else {
    bVar1 = true;
  }
  if (!bVar1) {
    direct_mode(param_1,1);
  }
  cVar3 = dorewind(param_1,param_2);
  if (cVar3 == '\x01') {
    if (param_5 < 0) {
      passname(0,local_7df);
    }
    else {
      cVar3 = known(local_890);
      if ((cVar3 == '\0') || (uVar10 = local_890, ((uVar6 + 2) / 3) * 3 <= local_890)) {
        uVar10 = ((uVar6 + 2) / 3) * 3;
      }
      fillpattern(param_5,lVar7,uVar10);
      passname(lVar7,local_7df);
    }
    if (param_8 != 0) {
      uVar8 = gettext("%s: pass %lu/%lu (%s)...");
      error(0,0,uVar8,param_3,param_7,param_8,local_7df);
      local_888 = rpl_time(0);
      local_888 = local_888 + 5;
      local_860 = "";
    }
    local_898 = 0;
LAB_00100c2b:
    cVar3 = known(local_890);
    local_878 = uVar6;
    if (((cVar3 != '\0') && (local_890 - local_898 < uVar6)) &&
       (((long)local_890 < (long)local_898 || (local_878 = local_890 - local_898, local_878 == 0))))
    {
      iVar4 = dosync(param_1,param_3);
      if (iVar4 != 0) {
        piVar9 = __errno_location();
        if (*piVar9 == 5) {
          local_8e0 = 1;
        }
        else {
          bVar2 = true;
        }
      }
      goto LAB_001013a8;
    }
    if (param_5 < 0) {
      randread(param_6,lVar7,local_878);
    }
    for (local_870 = 0; local_870 < local_878; local_870 = local_870 + local_868) {
      local_868 = write(param_1,(void *)(lVar7 + local_870),local_878 - local_870);
      if (local_868 < 1) {
        cVar3 = known(local_890);
        if ((cVar3 != '\x01') &&
           ((local_868 == 0 || (piVar9 = __errno_location(), *piVar9 == 0x1c)))) {
          if (local_870 <= 0x7fffffffffffffff - local_898) {
            local_890 = local_870 + local_898;
            *param_4 = local_890;
          }
          break;
        }
        piVar9 = __errno_location();
        iVar4 = *piVar9;
        if ((bVar1) || (piVar9 = __errno_location(), *piVar9 != 0x16)) {
          uVar8 = gettext("%s: error writing at offset %ju");
          error(0,iVar4,uVar8,param_3,local_870 + local_898);
          if ((iVar4 == 5) &&
             ((cVar3 = known(local_890), cVar3 != '\0' && ((local_870 | 0x1ff) < local_878)))) {
            lVar11 = (local_870 | 0x1ff) + 1;
            _Var12 = lseek(param_1,lVar11 + local_898,0);
            if (_Var12 != -1) {
              local_868 = lVar11 - local_870;
              local_8e0 = 1;
              goto LAB_00100f75;
            }
            uVar8 = gettext("%s: lseek failed");
            piVar9 = __errno_location();
            error(0,*piVar9,uVar8,param_3);
          }
          bVar2 = true;
          goto LAB_001013a8;
        }
        direct_mode(param_1,0);
        local_868 = 0;
        bVar1 = true;
      }
LAB_00100f75:
    }
    local_898 = local_870 + local_898;
    if (local_870 <= local_898 + 0x8000000000000000) {
      bVar14 = local_890 == local_898;
      if (((param_8 != 0) &&
          (((bVar14 && (*local_860 != '\0')) || (local_880 = rpl_time(0), local_888 <= local_880))))
         && ((local_858 = (char *)human_readable(local_898,local_548,0x1b2,1,1), bVar14 ||
             (cVar3 = streq(local_860,local_858), cVar3 != '\x01')))) {
        cVar3 = known(local_890);
        if (cVar3 == '\x01') {
          if (local_890 == 0) {
            uVar5 = 100;
          }
          else if (local_898 < 0x28f5c28f5c28f5d) {
            uVar5 = (undefined4)((local_898 * 100) / local_890);
          }
          else {
            uVar5 = (undefined4)(local_898 / (ulong)((long)local_890 / 100));
          }
          pcVar13 = (char *)human_readable(local_890,local_2b8,0x1b0,1,1);
          if (bVar14) {
            local_858 = pcVar13;
          }
          uVar8 = gettext("%s: pass %lu/%lu (%s)...%s/%s %d%%");
          error(0,0,uVar8,param_3,param_7,param_8,local_7df,local_858,pcVar13,uVar5);
        }
        else {
          uVar8 = gettext("%s: pass %lu/%lu (%s)...%s");
          error(0,0,uVar8,param_3,param_7,param_8,local_7df,local_858);
        }
        strcpy(local_7d8,local_858);
        local_860 = local_7d8;
        local_888 = local_880 + 5;
        iVar4 = dosync(param_1,param_3);
        if (iVar4 != 0) {
          piVar9 = __errno_location();
          if (*piVar9 != 5) {
            bVar2 = true;
            goto LAB_001013a8;
          }
          local_8e0 = 1;
        }
      }
      goto LAB_00100c2b;
    }
    uVar8 = gettext("%s: file too large");
    error(0,0,uVar8,param_3);
    bVar2 = true;
  }
  else {
    uVar8 = gettext("%s: cannot rewind");
    piVar9 = __errno_location();
    error(0,*piVar9,uVar8,param_3);
    bVar2 = true;
  }
LAB_001013a8:
  alignfree(lVar7);
  if (bVar2) {
    uVar6 = 0xffffffff;
  }
  else {
    uVar6 = (ulong)local_8e0;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar6;
}

// Function: genpattern
void genpattern(int *param_1,ulong param_2,undefined8 param_3) {
  int *__src;
  int iVar1;
  ulong uVar2;
  long lVar3;
  int local_44;
  long local_40;
  int *local_38;
  int *local_30;
  ulong local_28;
  ulong local_20;
  long local_18;
  
  if (param_2 != 0) {
    local_38 = &patterns;
    local_40 = 0;
    local_30 = param_1;
    local_28 = param_2;
    while( true ) {
      while( true ) {
        while( true ) {
          __src = local_38 + 1;
          local_44 = *local_38;
          if (local_44 != 0) break;
          local_38 = &patterns;
        }
        local_38 = __src;
        if (-1 < local_44) break;
        local_44 = -local_44;
        if (local_28 <= (ulong)(long)local_44) {
          local_40 = local_40 + local_28;
          goto LAB_00101551;
        }
        local_40 = local_40 + local_44;
        local_28 = local_28 - (long)local_44;
      }
      if (local_28 < (ulong)(long)local_44) break;
      memcpy(local_30,__src,(long)local_44 * 4);
      local_38 = __src + local_44;
      local_30 = local_30 + local_44;
      local_28 = local_28 - (long)local_44;
    }
    if ((local_28 < 2) || (local_28 * 3 < (ulong)(long)local_44)) {
      local_40 = local_40 + local_28;
    }
    else {
      do {
        if ((local_28 == (long)local_44) ||
           (uVar2 = randint_choose(param_3,(long)local_44), uVar2 < local_28)) {
          *local_30 = *local_38;
          local_28 = local_28 - 1;
          local_30 = local_30 + 1;
        }
        local_38 = local_38 + 1;
        local_44 = local_44 + -1;
      } while (local_28 != 0);
    }
LAB_00101551:
    local_18 = param_2 - local_40;
    uVar2 = local_40 - 1;
    local_20 = uVar2;
    for (local_28 = 0; local_28 < param_2; local_28 = local_28 + 1) {
      if (uVar2 < local_20) {
        lVar3 = randint_choose(param_3,local_18 - local_28);
        iVar1 = param_1[local_28];
        param_1[local_28] = param_1[lVar3 + local_28];
        param_1[lVar3 + local_28] = iVar1;
      }
      else {
        local_20 = (local_20 + param_2) - 1;
        param_1[local_18] = param_1[local_28];
        param_1[local_28] = -1;
        local_18 = local_18 + 1;
      }
      local_20 = local_20 - uVar2;
    }
  }
  return;
}

// Function: do_wipefd
undefined do_wipefd(int param_1,undefined8 param_2,undefined8 param_3,long param_4) {
  int iVar1;
  undefined4 uVar2;
  undefined8 uVar3;
  int *piVar4;
  ulong uVar5;
  long lVar6;
  long in_FS_OFFSET;
  undefined local_151;
  ulong local_128;
  ulong local_120;
  ulong local_118;
  ulong local_110;
  long local_108;
  long local_100;
  void *local_f8;
  long local_f0;
  long local_e8;
  undefined8 local_e0;
  undefined8 local_d8;
  undefined8 local_d0;
  undefined8 local_c8;
  undefined8 local_c0;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_110 = 0;
  local_151 = 1;
  local_108 = 0;
  if (*(char *)(param_4 + 0x1c) != '\0') {
    local_108 = (ulong)*(byte *)(param_4 + 0x1e) + *(long *)(param_4 + 8);
  }
  iVar1 = fstat(param_1,&local_b8);
  if (iVar1 == 0) {
    if (((((local_b8.st_mode & 0xf000) == 0x2000) && (iVar1 = isatty(param_1), iVar1 != 0)) ||
        ((local_b8.st_mode & 0xf000) == 0x1000)) || ((local_b8.st_mode & 0xf000) == 0xc000)) {
      local_d0 = gettext("%s: invalid file type");
      error(0,0,local_d0,param_2);
      local_151 = 0;
    }
    else {
      if (((local_b8.st_mode & 0xf000) != 0x8000) || (-1 < local_b8.st_size)) {
        local_f8 = (void *)xnmalloc(*(undefined8 *)(param_4 + 8),4);
        local_118 = *(ulong *)(param_4 + 0x10);
        if (local_118 == 0xffffffffffffffff) {
          if ((local_b8.st_mode & 0xf000) == 0x8000) {
            local_118 = local_b8.st_size;
            if (*(char *)(param_4 + 0x1d) != '\x01') {
              if ((local_b8.st_blksize < 1) ||
                 (uVar5 = local_b8.st_blksize, 0x2000000000000000 < (ulong)local_b8.st_blksize)) {
                uVar5 = 0x200;
              }
              local_f0 = local_b8.st_size % (long)uVar5;
              if (local_b8.st_size != 0) {
                if ((local_b8.st_blksize < 1) ||
                   (uVar5 = local_b8.st_blksize, 0x2000000000000000 < (ulong)local_b8.st_blksize)) {
                  uVar5 = 0x200;
                }
                if (local_b8.st_size < (long)uVar5) {
                  local_110 = local_b8.st_size;
                }
              }
              if (local_f0 != 0) {
                if ((local_b8.st_blksize < 1) || (0x2000000000000000 < (ulong)local_b8.st_blksize))
                {
                  local_b8.st_blksize = 0x200;
                }
                local_e8 = local_b8.st_blksize - local_f0;
                lVar6 = local_e8;
                if (0x7fffffffffffffff - local_b8.st_size <= local_e8) {
                  lVar6 = 0x7fffffffffffffff - local_b8.st_size;
                }
                local_118 = local_b8.st_size + lVar6;
              }
            }
          }
          else {
            local_118 = lseek(param_1,0,2);
            if ((long)local_118 < 1) {
              local_118 = 0xffffffffffffffff;
            }
          }
        }
        else if ((local_b8.st_mode & 0xf000) == 0x8000) {
          if ((local_b8.st_blksize < 1) || (0x2000000000000000 < (ulong)local_b8.st_blksize)) {
            local_b8.st_blksize = 0x200;
          }
          if ((long)local_118 < local_b8.st_blksize) {
            local_b8.st_blksize = local_118;
          }
          if (local_b8.st_size < local_b8.st_blksize) {
            local_110 = local_b8.st_size;
          }
        }
        genpattern(local_f8,*(undefined8 *)(param_4 + 8),param_3);
        local_e0 = randint_get_source(param_3);
        do {
          local_100 = local_108;
          if (local_110 == 0) {
            if (local_118 == 0) {
              if (((*(int *)(param_4 + 0x18) != 0) && (iVar1 = ftruncate(param_1,0), iVar1 != 0)) &&
                 ((local_b8.st_mode & 0xf000) == 0x8000)) {
                uVar3 = gettext("%s: error truncating");
                piVar4 = __errno_location();
                local_c0 = uVar3;
                error(0,*piVar4,uVar3,param_2);
                local_151 = 0;
              }
LAB_00101cdb:
              free(local_f8);
              goto LAB_00101cf1;
            }
            local_128 = local_118;
            local_118 = 0;
          }
          else {
            local_128 = local_110;
            local_110 = 0;
            local_100 = 0;
          }
          for (local_120 = 0; local_120 < (ulong)*(byte *)(param_4 + 0x1e) + *(long *)(param_4 + 8);
              local_120 = local_120 + 1) {
            if (local_120 < *(ulong *)(param_4 + 8)) {
              uVar2 = *(undefined4 *)((long)local_f8 + local_120 * 4);
            }
            else {
              uVar2 = 0;
            }
            iVar1 = dopass(param_1,&local_b8,param_2,&local_128,uVar2,local_e0,local_120 + 1,
                           local_100);
            if ((iVar1 != 0) && (local_151 = 0, iVar1 < 0)) goto LAB_00101cdb;
          }
        } while( true );
      }
      local_c8 = gettext("%s: file has negative size");
      error(0,0,local_c8,param_2);
      local_151 = 0;
    }
  }
  else {
    uVar3 = gettext("%s: fstat failed");
    piVar4 = __errno_location();
    local_d8 = uVar3;
    error(0,*piVar4,uVar3,param_2);
    local_151 = 0;
  }
LAB_00101cf1:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_151;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: wipefd
undefined8 wipefd(undefined4 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4) {
  uint uVar1;
  undefined8 uVar2;
  int *piVar3;
  
  uVar1 = rpl_fcntl(param_1,3);
  if ((int)uVar1 < 0) {
    uVar2 = gettext("%s: fcntl failed");
    piVar3 = __errno_location();
    error(0,*piVar3,uVar2,param_2);
    uVar2 = 0;
  }
  else if ((uVar1 & 0x400) == 0) {
    uVar2 = do_wipefd(param_1,param_2,param_3,param_4);
  }
  else {
    uVar2 = gettext("%s: cannot shred append-only file descriptor");
    error(0,0,uVar2,param_2);
    uVar2 = 0;
  }
  return uVar2;
}

// Function: incname
undefined8 incname(long param_1,long param_2) {
  long lVar1;
  char *pcVar2;
  long local_28;
  
  local_28 = param_2;
  while( true ) {
    lVar1 = local_28 + -1;
    if (local_28 == 0) {
      return 0;
    }
    pcVar2 = strchr("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_.",
                    (int)*(char *)(lVar1 + param_1));
    if (pcVar2[1] != '\0') break;
    *(undefined *)(lVar1 + param_1) = 0x30;
    local_28 = lVar1;
  }
  *(char *)(lVar1 + param_1) = pcVar2[1];
  return 1;
}

// Function: wipename
undefined wipename(char *param_1,char *param_2,long param_3) {
  bool bVar1;
  char cVar2;
  int iVar3;
  void *__ptr;
  void *__s;
  void *__ptr_00;
  undefined8 uVar4;
  void *__ptr_01;
  char *pcVar5;
  int *piVar6;
  undefined local_9e;
  int local_9c;
  size_t local_70;
  
  __ptr = (void *)xstrdup(param_1);
  __s = (void *)last_component(__ptr);
  __ptr_00 = (void *)dir_name(__ptr);
  uVar4 = quotearg_n_style_colon(0,3,__ptr_00);
  __ptr_01 = (void *)xstrdup(uVar4);
  bVar1 = true;
  local_9e = 1;
  local_9c = -1;
  if (*(int *)(param_3 + 0x18) == 3) {
    local_9c = open_safer(__ptr_00,0x10900);
  }
  if (*(char *)(param_3 + 0x1c) != '\0') {
    uVar4 = gettext("%s: removing");
    error(0,0,uVar4,param_2);
  }
  if (*(int *)(param_3 + 0x18) != 1) {
    for (local_70 = base_len(__s); local_70 != 0; local_70 = local_70 - 1) {
      memset(__s,0x30,local_70);
      *(undefined *)(local_70 + (long)__s) = 0;
      do {
        iVar3 = renameatu(0xffffff9c,param_1,0xffffff9c,__ptr,1);
        if ((iVar3 == 0) || (piVar6 = __errno_location(), *piVar6 != 0x11)) break;
        cVar2 = incname(__s,local_70);
      } while (cVar2 != '\0');
      if (iVar3 == 0) {
        if ((-1 < local_9c) && (iVar3 = dosync(local_9c,__ptr_01), iVar3 != 0)) {
          local_9e = 0;
        }
        if (*(char *)(param_3 + 0x1c) != '\0') {
          pcVar5 = param_1;
          if (bVar1) {
            pcVar5 = param_2;
          }
          uVar4 = gettext("%s: renamed to %s");
          error(0,0,uVar4,pcVar5,__ptr);
          bVar1 = false;
        }
        memcpy(param_1 + ((long)__s - (long)__ptr),__s,local_70 + 1);
      }
    }
  }
  iVar3 = unlink(param_1);
  if (iVar3 == 0) {
    if (*(char *)(param_3 + 0x1c) != '\0') {
      uVar4 = gettext("%s: removed");
      error(0,0,uVar4,param_2);
    }
  }
  else {
    uVar4 = gettext("%s: failed to remove");
    piVar6 = __errno_location();
    error(0,*piVar6,uVar4,param_2);
    local_9e = 0;
  }
  if (-1 < local_9c) {
    iVar3 = dosync(local_9c,__ptr_01);
    if (iVar3 != 0) {
      local_9e = 0;
    }
    iVar3 = close(local_9c);
    if (iVar3 != 0) {
      uVar4 = gettext("%s: failed to close");
      piVar6 = __errno_location();
      error(0,*piVar6,uVar4,__ptr_01);
      local_9e = 0;
    }
  }
  free(__ptr);
  free(__ptr_00);
  free(__ptr_01);
  return local_9e;
}

// Function: wipefile
char wipefile(char *param_1,undefined8 param_2,undefined8 param_3,char *param_4) {
  int iVar1;
  int *piVar2;
  undefined8 uVar3;
  char local_3d;
  int local_3c;
  
  local_3c = open_safer(param_1,0x101);
  if ((((local_3c < 0) && (piVar2 = __errno_location(), *piVar2 == 0xd)) && (*param_4 != '\0')) &&
     (iVar1 = chmod(param_1,0x80), iVar1 == 0)) {
    local_3c = open_safer(param_1,0x101);
  }
  if (local_3c < 0) {
    uVar3 = gettext("%s: failed to open for writing");
    piVar2 = __errno_location();
    error(0,*piVar2,uVar3,param_2);
    local_3d = '\0';
  }
  else {
    local_3d = do_wipefd(local_3c,param_2,param_3,param_4);
    iVar1 = close(local_3c);
    if (iVar1 != 0) {
      uVar3 = gettext("%s: failed to close");
      piVar2 = __errno_location();
      error(0,*piVar2,uVar3,param_2);
      local_3d = '\0';
    }
    if ((local_3d != '\0') && (*(int *)(param_4 + 0x18) != 0)) {
      local_3d = wipename(param_1,param_2,param_4);
    }
  }
  return local_3d;
}

// Function: clear_random_data
void clear_random_data(void) {
  randint_all_free(randint_source);
  return;
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  int iVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  long lVar5;
  undefined8 uVar6;
  int *piVar7;
  void *__ptr;
  long in_FS_OFFSET;
  bool local_9d;
  int local_9c;
  char *local_78;
  undefined local_48 [8];
  undefined8 uStack_40;
  undefined local_38 [8];
  undefined8 uStack_30;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_9d = true;
  _local_48 = (undefined  [16])0x0;
  _local_38 = (undefined  [16])0x0;
  local_78 = (char *)0x0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  uStack_40 = 3;
  local_38 = (undefined  [8])0xffffffffffffffff;
  do {
    while( true ) {
      iVar4 = getopt_long(param_1,param_2,"fn:s:uvxz",long_opts,0);
      iVar1 = _optind;
      if (iVar4 == -1) {
        iVar4 = param_1 - _optind;
        if (param_1 == _optind) {
          uVar6 = gettext("missing file operand");
          error(0,0,uVar6);
          usage(1);
        }
        randint_source = randint_all_new(local_78,0xffffffffffffffff);
        if (randint_source == 0) {
          if (local_78 == (char *)0x0) {
            local_78 = "getrandom";
          }
          uVar6 = quotearg_n_style_colon(0,3,local_78);
          piVar7 = __errno_location();
          error(1,*piVar7,&DAT_001036f8,uVar6);
        }
        atexit(clear_random_data);
        for (local_9c = 0; local_9c < iVar4; local_9c = local_9c + 1) {
          uVar6 = quotearg_n_style_colon(0,3,param_2[(long)iVar1 + (long)local_9c]);
          __ptr = (void *)xstrdup(uVar6);
          cVar2 = streq(param_2[(long)iVar1 + (long)local_9c],&DAT_001036fb);
          if (cVar2 == '\0') {
            bVar3 = wipefile(param_2[(long)iVar1 + (long)local_9c],__ptr,randint_source,local_48);
          }
          else {
            bVar3 = wipefd(1,__ptr,randint_source,local_48);
          }
          local_9d = (bVar3 & local_9d) != 0;
          free(__ptr);
        }
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        return local_9d ^ 1;
      }
      if (iVar4 < 0x81) break;
switchD_00102565_caseD_67:
      usage(1);
    }
    if (iVar4 < 0x66) {
      if (iVar4 == -0x83) {
LAB_0010271c:
        uVar6 = proper_name_lite("Colin Plumb","Colin Plumb");
        version_etc(_stdout,"shred","GNU coreutils",_Version,uVar6,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar4 == -0x82) {
        usage(0);
        goto LAB_0010271c;
      }
      goto switchD_00102565_caseD_67;
    }
    switch(iVar4) {
    case 0x66:
      local_48[0] = 1;
      break;
    default:
      goto switchD_00102565_caseD_67;
    case 0x6e:
      uVar6 = gettext("invalid number of passes");
      uVar6 = xdectoumax(_optarg,0,0x3fffffffffffffff,&DAT_00102b74,uVar6,0);
      uStack_40 = uVar6;
      break;
    case 0x73:
      uVar6 = gettext("invalid file size");
      uVar6 = xnumtoumax(_optarg,0,0,0x7fffffffffffffff,"cbBkKMGTPEZYRQ0",uVar6,0,0);
      local_38 = (undefined  [8])uVar6;
      break;
    case 0x75:
      if (_optarg == (char *)0x0) {
        uStack_30._0_4_ = 3;
      }
      else {
        lVar5 = __xargmatch_internal
                          ("--remove",_optarg,remove_args,remove_methods,4,_argmatch_die,1);
        uStack_30._0_4_ = *(undefined4 *)(remove_methods + lVar5 * 4);
      }
      break;
    case 0x76:
      uStack_30._4_1_ = 1;
      break;
    case 0x78:
      uStack_30._5_1_ = 1;
      break;
    case 0x7a:
      uStack_30._6_1_ = 1;
      break;
    case 0x80:
      if ((local_78 != (char *)0x0) && (cVar2 = streq(local_78,_optarg), cVar2 != '\x01')) {
        uVar6 = gettext("multiple random sources specified");
        error(1,0,uVar6);
      }
      local_78 = _optarg;
    }
  } while( true );
}

