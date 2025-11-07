// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [FILE]...\n");
    printf(pcVar3,uVar2);
    pcVar3 = (char *)gettext(
                            "Print the first %d lines of each FILE to standard output.\nWith more than one FILE, precede each with a header giving the file name.\n"
                            );
    printf(pcVar3,10);
    emit_stdin_note();
    emit_mandatory_arg_note();
    pcVar3 = (char *)gettext(
                            "  -c, --bytes=[-]NUM       print the first NUM bytes of each file;\n                             with the leading \'-\', print all but the last\n                             NUM bytes of each file\n  -n, --lines=[-]NUM       print the first NUM lines instead of the first %d;\n                             with the leading \'-\', print all but the last\n                             NUM lines of each file\n"
                            );
    printf(pcVar3,10);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -q, --quiet, --silent    never print headers giving file names\n  -v, --verbose            always print headers giving file names\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -z, --zero-terminated    line delimiter is NUL, not newline\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nNUM may have a multiplier suffix:\nb 512, kB 1000, K 1024, MB 1000*1000, M 1024*1024,\nGB 1000*1000*1000, G 1024*1024*1024, and so on for T, P, E, Z, Y, R, Q.\nBinary prefixes can be used, too: KiB=K, MiB=M, and so on.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00102bd2);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: diagnose_read_failure
void diagnose_read_failure(undefined8 param_1) {
  undefined8 uVar1;
  undefined8 uVar2;
  int *piVar3;
  
  uVar1 = quotearg_style(4,param_1);
  uVar2 = gettext("error reading %s");
  piVar3 = __errno_location();
  error(0,*piVar3,uVar2,uVar1);
  return;
}

// Function: diagnose_copy_fd_failure
void diagnose_copy_fd_failure(int param_1,undefined8 param_2) {
  undefined8 uVar1;
  undefined8 uVar2;
  int *piVar3;
  
  if (param_1 == 1) {
    diagnose_read_failure(param_2);
  }
  else {
    if (param_1 != 2) {
                    /* WARNING: Subroutine does not return */
      __assert_fail("0",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/head.c",
                    0xa7,"diagnose_copy_fd_failure");
    }
    uVar1 = quotearg_n_style_colon(0,3,param_2);
    uVar2 = gettext("%s: file has shrunk too much");
    piVar3 = __errno_location();
    error(0,*piVar3,uVar2,uVar1);
  }
  return;
}

// Function: write_header
void write_header(undefined8 param_1) {
  undefined *puVar1;
  
  if (first_file_1 == '\0') {
    puVar1 = &DAT_00102c58;
  }
  else {
    puVar1 = &DAT_001026cc;
  }
  printf("%s==> %s <==\n",puVar1,param_1);
  first_file_1 = 0;
  return;
}

// Function: xwrite_stdout
void xwrite_stdout(void *param_1,ulong param_2) {
  size_t sVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  
  if (param_2 != 0) {
    sVar1 = fwrite_unlocked(param_1,1,param_2,_stdout);
    if (sVar1 < param_2) {
      clearerr_unlocked(_stdout);
      fpurge(_stdout);
      uVar2 = quotearg_style(4,"standard output");
      uVar3 = gettext("error writing %s");
      piVar4 = __errno_location();
      error(1,*piVar4,uVar3,uVar2);
    }
  }
  return;
}

// Function: copy_fd
undefined8 copy_fd(int param_1,ulong param_2) {
  ulong __nbytes;
  ssize_t sVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  ulong local_2038;
  undefined local_2018 [8200];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_2038 = param_2;
  do {
    if (local_2038 == 0) {
      uVar2 = 0;
LAB_0010077f:
      if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return uVar2;
    }
    __nbytes = local_2038;
    if (0x2000 < local_2038) {
      __nbytes = 0x2000;
    }
    sVar1 = read(param_1,local_2018,__nbytes);
    if (sVar1 < 0) {
      uVar2 = 1;
      goto LAB_0010077f;
    }
    local_2038 = local_2038 - sVar1;
    if ((sVar1 == 0) && (local_2038 != 0)) {
      uVar2 = 2;
      goto LAB_0010077f;
    }
    xwrite_stdout(local_2018,sVar1);
  } while( true );
}

// Function: elseek_diagnostic
void elseek_diagnostic(undefined8 param_1,int param_2,undefined8 param_3) {
  undefined8 uVar1;
  char *pcVar2;
  undefined8 uVar3;
  int *piVar4;
  
  uVar1 = quotearg_n_style_colon(0,3,param_3);
  if (param_2 == 0) {
    pcVar2 = "%s: cannot seek to offset %jd";
  }
  else {
    pcVar2 = "%s: cannot seek to relative offset %jd";
  }
  uVar3 = gettext(pcVar2);
  piVar4 = __errno_location();
  error(0,*piVar4,uVar3,uVar1,param_1);
  return;
}

// Function: elseek
__off_t elseek(int param_1,__off_t param_2,int param_3,undefined8 param_4) {
  __off_t _Var1;
  
  _Var1 = lseek(param_1,param_2,param_3);
  if (_Var1 < 0) {
    elseek_diagnostic(param_2,param_3,param_4);
  }
  return _Var1;
}

// Function: elide_tail_bytes_pipe
undefined elide_tail_bytes_pipe(undefined8 param_1,undefined4 param_2,ulong param_3,long param_4) {
  undefined8 *puVar1;
  bool bVar2;
  bool bVar3;
  int *piVar4;
  ulong uVar5;
  undefined8 uVar6;
  long lVar7;
  long in_FS_OFFSET;
  undefined local_c6;
  bool local_c3;
  ulong local_c0;
  long local_b8;
  long local_b0;
  long local_a8;
  void *local_a0;
  long local_98;
  ulong local_90;
  ulong local_88;
  ulong local_80;
  ulong local_78;
  long local_70;
  long local_68;
  long local_60;
  long local_58;
  ulong local_50;
  long local_48;
  long local_40;
  void *local_38;
  long local_30;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_c6 = 1;
  local_b8 = param_4;
  if (param_3 < 0x100001) {
    bVar2 = true;
    bVar3 = false;
    local_48 = param_3 + 0x2000;
    local_50 = param_3;
    local_38 = (void *)xnmalloc(2,local_48);
    local_30 = local_48 + (long)local_38;
    local_c3 = false;
    while (!bVar3) {
      local_40 = full_read(param_2,(&local_38)[(int)(uint)local_c3],local_48);
      local_b0 = 0;
      if (local_40 < local_48) {
        piVar4 = __errno_location();
        if (*piVar4 != 0) {
          diagnose_read_failure(param_1);
          local_c6 = 0;
          break;
        }
        if ((local_40 <= (long)local_50) && (!bVar2)) {
          local_b0 = local_50 - local_40;
        }
        bVar3 = true;
      }
      if (!bVar2) {
        local_b8 = local_b8 + (local_50 - local_b0);
        xwrite_stdout((long)(&local_38)[(int)(uint)(local_c3 ^ 1)] + 0x2000,local_50 - local_b0);
      }
      bVar2 = false;
      if ((long)local_50 < local_40) {
        local_b8 = local_b8 + (local_40 - local_50);
        xwrite_stdout((&local_38)[(int)(uint)local_c3],local_40 - local_50);
      }
      local_c3 = local_c3 == false;
    }
    free(local_38);
  }
  else {
    bVar3 = false;
    local_a0 = (void *)0x0;
    local_80 = (ulong)((uint)param_3 & 0x1fff);
    local_78 = (ulong)(local_80 != 0) + (param_3 >> 0xd) + 1;
    local_98 = 0;
    local_c0 = 0;
    bVar2 = false;
    local_90 = 0;
    local_88 = 1;
    while (!bVar3) {
      if (local_90 == local_c0) {
        uVar5 = 0x7fffffffffffffff;
        if (local_78 < 0x8000000000000000) {
          uVar5 = local_78;
        }
        local_a0 = (void *)xpalloc(local_a0,&local_c0,1,uVar5,8);
      }
      if (!bVar2) {
        puVar1 = (undefined8 *)(local_90 * 8 + (long)local_a0);
        uVar6 = xmalloc(0x2000);
        *puVar1 = uVar6;
        local_98 = local_90 + 1;
      }
      local_a8 = full_read(param_2,*(undefined8 *)((long)local_a0 + local_90 * 8),0x2000);
      if (local_a8 < 0x2000) {
        piVar4 = __errno_location();
        if (*piVar4 != 0) {
          diagnose_read_failure(param_1);
          local_c6 = 0;
          goto LAB_00100e00;
        }
        bVar3 = true;
      }
      if (local_90 + 1 == local_78) {
        bVar2 = true;
      }
      if (bVar2) {
        local_b8 = local_b8 + local_a8;
        xwrite_stdout(*(undefined8 *)((long)local_a0 + local_88 * 8),local_a8);
      }
      local_90 = local_88;
      local_88 = (local_88 + 1) % local_78;
    }
    local_70 = 0x2000 - local_80;
    if (bVar2) {
      local_58 = 0x2000 - local_a8;
      local_b8 = local_b8 + local_70;
      if (local_70 < local_58) {
        xwrite_stdout(local_a8 + *(long *)((long)local_a0 + local_90 * 8),local_70);
      }
      else {
        xwrite_stdout(local_a8 + *(long *)((long)local_a0 + local_90 * 8),local_58);
        xwrite_stdout(*(undefined8 *)((long)local_a0 + local_88 * 8),local_70 - local_58);
      }
    }
    else if (local_90 + 1 == local_78) {
      local_68 = 0x2000 - local_70;
      local_60 = local_a8 - local_68;
      local_b8 = local_b8 + local_60;
      xwrite_stdout(*(undefined8 *)((long)local_a0 + local_88 * 8),local_60);
    }
LAB_00100e00:
    for (local_90 = 0; (long)local_90 < local_98; local_90 = local_90 + 1) {
      free(*(void **)((long)local_a0 + local_90 * 8));
    }
    free(local_a0);
  }
  if (-1 < param_4) {
    lVar7 = elseek(param_2,local_b8,0,param_1);
    if (lVar7 < 0) {
      local_c6 = 0;
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_c6;
}

// Function: elide_tail_bytes_file
undefined8
elide_tail_bytes_file(undefined8 param_1,undefined4 param_2,ulong param_3,long param_4,long param_5) {
  int iVar1;
  long lVar2;
  undefined8 uVar3;
  ulong uVar4;
  
  if (-1 < param_5) {
    if ((*(long *)(param_4 + 0x38) < 1) || (0x2000000000000000 < *(ulong *)(param_4 + 0x38))) {
      lVar2 = 0x200;
    }
    else {
      lVar2 = *(long *)(param_4 + 0x38);
    }
    if (lVar2 < *(long *)(param_4 + 0x30)) {
      uVar4 = *(long *)(param_4 + 0x30) - param_5;
      if ((long)uVar4 < 0) {
        uVar4 = 0;
      }
      if (uVar4 <= param_3) {
        return 1;
      }
      iVar1 = copy_fd(param_2,uVar4 - param_3);
      if (iVar1 == 0) {
        return 1;
      }
      diagnose_copy_fd_failure(iVar1,param_1);
      return 0;
    }
  }
  uVar3 = elide_tail_bytes_pipe(param_1,param_2,param_3,param_5);
  return uVar3;
}

// Function: elide_tail_lines_pipe
undefined elide_tail_lines_pipe(undefined8 param_1,int param_2,ulong param_3,char *param_4) {
  long *plVar1;
  char *pcVar2;
  ssize_t sVar3;
  long lVar4;
  undefined local_61;
  char *local_60;
  char *local_58;
  char *local_50;
  char *local_48;
  ulong local_40;
  char *local_38;
  long local_30;
  char *local_28;
  
  local_40 = 0;
  local_61 = 1;
  local_58 = (char *)xmalloc(0x2020);
  *(undefined8 *)(local_58 + 0x2010) = 0;
  *(undefined8 *)(local_58 + 0x2008) = *(undefined8 *)(local_58 + 0x2010);
  *(undefined8 *)(local_58 + 0x2018) = 0;
  local_48 = (char *)xmalloc(0x2020);
  local_60 = param_4;
  local_50 = local_58;
  while (sVar3 = read(param_2,local_48,0x2000), 0 < sVar3) {
    if (param_3 == 0) {
      local_60 = local_60 + sVar3;
      xwrite_stdout(local_48,sVar3);
    }
    else {
      *(ssize_t *)(local_48 + 0x2008) = sVar3;
      *(undefined8 *)(local_48 + 0x2010) = 0;
      *(undefined8 *)(local_48 + 0x2018) = 0;
      local_48[sVar3] = line_end;
      local_38 = local_48;
      while (local_38 = (char *)rawmemchr(local_38,(int)line_end), local_38 < local_48 + sVar3) {
        local_38 = local_38 + 1;
        *(long *)(local_48 + 0x2010) = *(long *)(local_48 + 0x2010) + 1;
      }
      local_40 = local_40 + *(long *)(local_48 + 0x2010);
      if ((ulong)(*(long *)(local_50 + 0x2008) + *(long *)(local_48 + 0x2008)) < 0x2000) {
        memcpy(local_50 + *(long *)(local_50 + 0x2008),local_48,*(size_t *)(local_48 + 0x2008));
        *(long *)(local_50 + 0x2008) = *(long *)(local_50 + 0x2008) + *(long *)(local_48 + 0x2008);
        *(long *)(local_50 + 0x2010) = *(long *)(local_50 + 0x2010) + *(long *)(local_48 + 0x2010);
      }
      else {
        *(char **)(local_50 + 0x2018) = local_48;
        local_50 = *(char **)(local_50 + 0x2018);
        if (param_3 < local_40 - *(long *)(local_58 + 0x2010)) {
          local_60 = local_60 + *(long *)(local_58 + 0x2008);
          xwrite_stdout(local_58,*(undefined8 *)(local_58 + 0x2008));
          local_48 = local_58;
          local_40 = local_40 - *(long *)(local_58 + 0x2010);
          local_58 = *(char **)(local_58 + 0x2018);
        }
        else {
          local_48 = (char *)xmalloc(0x2020);
        }
      }
    }
  }
  free(local_48);
  if (sVar3 < 0) {
    diagnose_read_failure(param_1);
    local_61 = 0;
  }
  else {
    if ((*(long *)(local_50 + 0x2008) != 0) &&
       (local_50[*(long *)(local_50 + 0x2008) + -1] != line_end)) {
      *(long *)(local_50 + 0x2010) = *(long *)(local_50 + 0x2010) + 1;
      local_40 = local_40 + 1;
    }
    local_48 = local_58;
    for (; param_3 < local_40 - *(long *)(local_48 + 0x2010); local_40 = local_40 - *plVar1) {
      local_60 = local_60 + *(long *)(local_48 + 0x2008);
      xwrite_stdout(local_48,*(undefined8 *)(local_48 + 0x2008));
      plVar1 = (long *)(local_48 + 0x2010);
      local_48 = *(char **)(local_48 + 0x2018);
    }
    if (param_3 < local_40) {
      local_30 = local_40 - param_3;
      lVar4 = *(long *)(local_48 + 0x2008);
      local_28 = local_48;
      while ((local_30 != 0 &&
             (local_28 = (char *)memchr(local_28,(int)line_end,
                                        (size_t)(local_48 + (lVar4 - (long)local_28))),
             local_28 != (char *)0x0))) {
        local_28 = local_28 + 1;
        *(long *)(local_48 + 0x2010) = *(long *)(local_48 + 0x2010) + 1;
        local_30 = local_30 + -1;
      }
      local_60 = local_28 + ((long)local_60 - (long)local_48);
      xwrite_stdout(local_48,(long)local_28 - (long)local_48);
    }
  }
  while (local_58 != (char *)0x0) {
    pcVar2 = *(char **)(local_58 + 0x2018);
    free(local_58);
    local_58 = pcVar2;
  }
  if ((-1 < (long)param_4) && (lVar4 = elseek(param_2,local_60,0,param_1), lVar4 < 0)) {
    local_61 = 0;
  }
  return local_61;
}

// Function: elide_tail_lines_seekable
ulong elide_tail_lines_seekable
                (undefined8 param_1,int param_2,long param_3,long param_4,long param_5) {
  int iVar1;
  long lVar2;
  ulong uVar3;
  long in_FS_OFFSET;
  bool bVar4;
  long local_2060;
  size_t local_2038;
  long local_2030;
  size_t local_2028;
  undefined8 local_2020;
  undefined local_2018 [8200];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_2038 = (param_5 - param_4) % 0x2000;
  if (local_2038 == 0) {
    local_2038 = 0x2000;
  }
  local_2030 = param_5 - local_2038;
  lVar2 = elseek(param_2,local_2030,0,param_1);
  if (lVar2 < 0) {
    uVar3 = 0;
  }
  else {
    local_2038 = read(param_2,local_2018,local_2038);
    if ((long)local_2038 < 0) {
      diagnose_read_failure(param_1);
      uVar3 = 0;
    }
    else {
      local_2060 = param_3;
      if (((param_3 != 0) && (local_2038 != 0)) && (local_2018[local_2038 - 1] != line_end)) {
        local_2060 = param_3 + -1;
      }
      do {
        local_2028 = local_2038;
        while (local_2028 != 0) {
          if (param_3 == 0) {
            local_2028 = local_2028 - 1;
          }
          else {
            local_2020 = memrchr(local_2018,(int)line_end,local_2028);
            if (local_2020 == (void *)0x0) break;
            local_2028 = (long)local_2020 - (long)local_2018;
          }
          bVar4 = local_2060 == 0;
          local_2060 = local_2060 + -1;
          if (bVar4) {
            if (param_4 < local_2030) {
              lVar2 = elseek(param_2,param_4,0,param_1);
              if (lVar2 < 0) {
                uVar3 = 0;
                goto LAB_00101820;
              }
              iVar1 = copy_fd(param_2,local_2030 - param_4);
              if (iVar1 != 0) {
                diagnose_copy_fd_failure(iVar1,param_1);
                uVar3 = 0;
                goto LAB_00101820;
              }
            }
            xwrite_stdout(local_2018,local_2028 + 1);
            uVar3 = elseek(param_2,local_2028 + local_2030 + 1,0,param_1);
            uVar3 = ~uVar3 >> 0x3f;
            goto LAB_00101820;
          }
        }
        if (local_2030 == param_4) {
          uVar3 = 1;
          goto LAB_00101820;
        }
        local_2030 = local_2030 + -0x2000;
        lVar2 = elseek(param_2,local_2030,0,param_1);
        if (lVar2 < 0) {
          uVar3 = 0;
          goto LAB_00101820;
        }
        local_2038 = read(param_2,local_2018,0x2000);
        if ((long)local_2038 < 0) {
          diagnose_read_failure(param_1);
          uVar3 = 0;
          goto LAB_00101820;
        }
      } while (local_2038 != 0);
      uVar3 = 1;
    }
  }
LAB_00101820:
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: elide_tail_lines_file
ulong elide_tail_lines_file
                (undefined8 param_1,undefined4 param_2,undefined8 param_3,long param_4,long param_5) {
  long lVar1;
  char cVar2;
  uint uVar3;
  long lVar4;
  ulong uVar5;
  
  lVar1 = *(long *)(param_4 + 0x30);
  if (-1 < param_5) {
    if ((*(long *)(param_4 + 0x38) < 1) || (0x2000000000000000 < *(ulong *)(param_4 + 0x38))) {
      lVar4 = 0x200;
    }
    else {
      lVar4 = *(long *)(param_4 + 0x38);
    }
    if (lVar4 < lVar1) {
      if ((param_5 < lVar1) &&
         (cVar2 = elide_tail_lines_seekable(param_1,param_2,param_3,param_5,lVar1), cVar2 == '\0'))
      {
        uVar3 = 0;
      }
      else {
        uVar3 = 1;
      }
      return (ulong)uVar3;
    }
  }
  uVar5 = elide_tail_lines_pipe(param_1,param_2,param_3,param_5);
  return uVar5;
}

// Function: head_bytes
bool head_bytes(undefined8 param_1,undefined4 param_2,undefined8 param_3) {
  int iVar1;
  
  iVar1 = copy_fd(param_2,param_3);
  if (iVar1 == 1) {
    diagnose_read_failure(param_1);
  }
  return iVar1 != 1;
}

// Function: head_lines
undefined8 head_lines(undefined8 param_1,int param_2,long param_3) {
  long lVar1;
  char *pcVar2;
  int iVar3;
  ssize_t sVar4;
  __off_t _Var5;
  undefined8 uVar6;
  long in_FS_OFFSET;
  long local_20e0;
  long local_20c0;
  stat local_20a8;
  char local_2018 [8200];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_20e0 = param_3;
  while (local_20e0 != 0) {
    sVar4 = read(param_2,local_2018,0x2000);
    local_20c0 = 0;
    if (sVar4 < 0) {
      diagnose_read_failure(param_1);
      uVar6 = 0;
      goto LAB_00101b08;
    }
    if (sVar4 == 0) break;
    do {
      if (sVar4 <= local_20c0) goto LAB_00101ad9;
      lVar1 = local_20c0 + 1;
      pcVar2 = local_2018 + local_20c0;
      local_20c0 = lVar1;
    } while ((*pcVar2 != line_end) || (local_20e0 = local_20e0 + -1, local_20e0 != 0));
    _Var5 = lseek(param_2,-(sVar4 - lVar1),1);
    if ((_Var5 < 0) &&
       ((iVar3 = fstat(param_2,&local_20a8), iVar3 != 0 || ((local_20a8.st_mode & 0xf000) == 0x8000)
        ))) {
      elseek_diagnostic(-(sVar4 - lVar1),1,param_1);
    }
LAB_00101ad9:
    xwrite_stdout(local_2018,local_20c0);
  }
  uVar6 = 1;
LAB_00101b08:
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar6;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: head
undefined8 head(undefined8 param_1,int param_2,long param_3,char param_4,char param_5) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  long in_FS_OFFSET;
  long local_c8;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (print_headers != '\0') {
    write_header(param_1);
  }
  if (param_5 == '\0') {
    if (param_4 == '\0') {
      uVar2 = head_bytes(param_1,param_2,param_3);
    }
    else {
      uVar2 = head_lines(param_1,param_2,param_3);
    }
  }
  else if (param_3 == -1) {
    uVar2 = 1;
  }
  else {
    local_c8 = -1;
    iVar1 = fstat(param_2,&local_b8);
    if (iVar1 == 0) {
      if ((presume_input_pipe != '\x01') && ((local_b8.st_mode & 0xf000) == 0x8000)) {
        local_c8 = elseek(param_2,0,1,param_1);
        if (local_c8 < 0) {
          uVar2 = 0;
          goto LAB_00101d45;
        }
      }
      if (param_4 == '\0') {
        uVar2 = elide_tail_bytes_file(param_1,param_2,param_3,&local_b8,local_c8);
      }
      else {
        uVar2 = elide_tail_lines_file(param_1,param_2,param_3,&local_b8,local_c8);
      }
    }
    else {
      uVar2 = quotearg_style(4,param_1);
      uVar3 = gettext("cannot fstat %s");
      piVar4 = __errno_location();
      error(0,*piVar4,uVar3,uVar2);
      uVar2 = 0;
    }
  }
LAB_00101d45:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar2;
}

// Function: head_file
undefined head_file(char *param_1,undefined8 param_2,undefined param_3,undefined param_4) {
  char cVar1;
  undefined uVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  char *local_50;
  int local_3c;
  
  cVar1 = streq(param_1,&DAT_00102cdf);
  if (cVar1 == '\0') {
    local_3c = open(param_1,0);
    local_50 = param_1;
    if (local_3c < 0) {
      uVar4 = quotearg_style(4,param_1);
      uVar5 = gettext("cannot open %s for reading");
      piVar6 = __errno_location();
      error(0,*piVar6,uVar5,uVar4);
      return 0;
    }
  }
  else {
    have_read_stdin = 1;
    local_3c = 0;
    local_50 = (char *)gettext("standard input");
    xset_binary_mode(0,0);
  }
  uVar2 = head(local_50,local_3c,param_2,param_3,param_4);
  if ((cVar1 != '\x01') && (iVar3 = close(local_3c), iVar3 != 0)) {
    uVar4 = quotearg_style(4,local_50);
    uVar5 = gettext("failed to close %s");
    piVar6 = __errno_location();
    error(0,*piVar6,uVar5,uVar4);
    uVar2 = 0;
  }
  return uVar2;
}

// Function: string_to_integer
void string_to_integer(char param_1,undefined8 param_2) {
  undefined8 uVar1;
  
  if (param_1 == '\0') {
    uVar1 = gettext("invalid number of bytes");
  }
  else {
    uVar1 = gettext("invalid number of lines");
  }
  xnumtoumax(param_2,10,0,0xffffffffffffffff,"bkKmMGTPEZYRQ0",uVar1,0,2);
  return;
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  char *pcVar1;
  long lVar2;
  char *pcVar3;
  char cVar4;
  byte bVar5;
  int iVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined1 *puVar9;
  int *piVar10;
  undefined8 *local_98;
  int local_8c;
  bool local_84;
  undefined local_83;
  bool local_82;
  char local_81;
  int local_80;
  long local_60;
  undefined8 local_58;
  char *local_50;
  
  local_80 = 0;
  local_84 = true;
  local_58 = 10;
  local_83 = 1;
  local_82 = false;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  have_read_stdin = '\0';
  print_headers = 0;
  line_end = 10;
  local_98 = param_2;
  local_8c = param_1;
  if (((1 < param_1) && (*(char *)param_2[1] == '-')) &&
     (cVar4 = c_isdigit((int)*(char *)(param_2[1] + 1)), cVar4 != '\0')) {
    lVar2 = param_2[1];
    local_81 = '\0';
    local_50 = (char *)(lVar2 + 1);
    do {
      pcVar3 = local_50;
      pcVar1 = local_50 + 1;
      cVar4 = c_isdigit((int)*pcVar1);
      local_50 = pcVar1;
    } while (cVar4 != '\0');
    for (; *local_50 != '\0'; local_50 = local_50 + 1) {
      switch(*local_50) {
      case 'b':
      case 'k':
      case 'm':
        local_83 = 0;
        local_81 = *local_50;
        break;
      case 'c':
        local_83 = 0;
        local_81 = '\0';
        break;
      default:
        cVar4 = *local_50;
        uVar8 = gettext("invalid trailing option -- %c");
        error(0,0,uVar8,(int)cVar4);
        usage(1);
        break;
      case 'l':
        local_83 = 1;
        break;
      case 'q':
        local_80 = 2;
        break;
      case 'v':
        local_80 = 1;
        break;
      case 'z':
        line_end = 0;
      }
    }
    *pcVar1 = local_81;
    if (local_81 != '\0') {
      pcVar3[2] = '\0';
    }
    local_58 = string_to_integer(local_83,(char *)(lVar2 + 1));
    param_2[1] = *param_2;
    local_98 = param_2 + 1;
    local_8c = param_1 + -1;
  }
  do {
    while( true ) {
      iVar6 = getopt_long(local_8c,local_98,"c:n:qvz0123456789",long_options,0);
      if (iVar6 == -1) {
        if ((local_80 == 1) || ((local_80 == 0 && (_optind < local_8c + -1)))) {
          print_headers = 1;
        }
        if (_optind < local_8c) {
          puVar9 = (undefined1 *)(local_98 + _optind);
        }
        else {
          puVar9 = default_file_list_0;
        }
        xset_binary_mode(1,0);
        for (local_60 = 0; *(long *)((long)puVar9 + local_60 * 8) != 0; local_60 = local_60 + 1) {
          bVar5 = head_file(*(undefined8 *)((long)puVar9 + local_60 * 8),local_58,local_83,local_82)
          ;
          local_84 = (bVar5 & local_84) != 0;
        }
        if ((have_read_stdin != '\0') && (iVar6 = close(0), iVar6 < 0)) {
          piVar10 = __errno_location();
          error(1,*piVar10,&DAT_00102cdf);
        }
        return local_84 ^ 1;
      }
      if (iVar6 < 0x81) break;
switchD_0010222e_caseD_64:
      cVar4 = c_isdigit(iVar6);
      if (cVar4 != '\0') {
        uVar8 = gettext("invalid trailing option -- %c");
        error(0,0,uVar8,iVar6);
      }
      usage(1);
    }
    if (iVar6 < 99) {
      if (iVar6 == -0x83) {
LAB_00102303:
        uVar8 = proper_name_lite("Jim Meyering","Jim Meyering");
        uVar7 = proper_name_lite("David MacKenzie","David MacKenzie");
        version_etc(_stdout,&DAT_00102bd2,"GNU coreutils",_Version,uVar7,uVar8,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar6 == -0x82) {
        usage(0);
        goto LAB_00102303;
      }
      goto switchD_0010222e_caseD_64;
    }
    switch(iVar6) {
    case 99:
      local_83 = 0;
      local_82 = *_optarg == '-';
      if (local_82) {
        _optarg = _optarg + 1;
      }
      local_58 = string_to_integer(0,_optarg);
      break;
    default:
      goto switchD_0010222e_caseD_64;
    case 0x6e:
      local_83 = 1;
      local_82 = *_optarg == '-';
      if (local_82) {
        _optarg = _optarg + 1;
      }
      local_58 = string_to_integer(1,_optarg);
      break;
    case 0x71:
      local_80 = 2;
      break;
    case 0x76:
      local_80 = 1;
      break;
    case 0x7a:
      line_end = 0;
      break;
    case 0x80:
      presume_input_pipe = 1;
    }
  } while( true );
}

