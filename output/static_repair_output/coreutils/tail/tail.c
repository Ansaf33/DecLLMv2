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
                            "Print the last %d lines of each FILE to standard output.\nWith more than one FILE, precede each with a header giving the file name.\n"
                            );
    printf(pcVar3,10);
    emit_stdin_note();
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -c, --bytes=[+]NUM       output the last NUM bytes; or use -c +NUM to\n                             output starting with byte NUM of each file\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -f, --follow[={name|descriptor}]\n                           output appended data as the file grows;\n                             an absent option argument means \'descriptor\'\n  -F                       same as --follow=name --retry\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pcVar3 = (char *)gettext(
                            "  -n, --lines=[+]NUM       output the last NUM lines, instead of the last %d;\n                             or use -n +NUM to skip NUM-1 lines at the start\n"
                            );
    printf(pcVar3,10);
    pcVar3 = (char *)gettext(
                            "      --max-unchanged-stats=N\n                           with --follow=name, reopen a FILE which has not\n                             changed size after N (default %d) iterations\n                             to see if it has been unlinked or renamed\n                             (this is the usual case of rotated log files);\n                             with inotify, this option is rarely useful\n"
                            );
    printf(pcVar3,5);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --pid=PID            with -f, exit after PID no longer exists;\n                             can be repeated to watch multiple processes\n  -q, --quiet, --silent    never output headers giving file names\n      --retry              keep trying to open a file if it is inaccessible\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -s, --sleep-interval=N   with -f, sleep for approximately N seconds\n                             (default 1.0) between iterations;\n                             with inotify and --pid=P, check process P at\n                             least once every N seconds\n  -v, --verbose            always output headers giving file names\n"
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
                            "\nNUM may have a multiplier suffix:\nb 512, kB 1000, K 1024, MB 1000*1000, M 1024*1024,\nGB 1000*1000*1000, G 1024*1024*1024, and so on for T, P, E, Z, Y, R, Q.\nBinary prefixes can be used, too: KiB=K, MiB=M, and so on.\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "With --follow (-f), tail defaults to following the file descriptor, which\nmeans that even if a tail\'ed file is renamed, tail will continue to track\nits end.  This default behavior is not desirable when you really want to\ntrack the actual name of the file, not the file descriptor (e.g., log\nrotation).  Use --follow=name in that case.  That causes tail to track the\nnamed file in a way that accommodates renaming, removal and creation.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00108bfd);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: die_pipe
void die_pipe(void) {
  raise(0xd);
                    /* WARNING: Subroutine does not return */
  exit(1);
}

// Function: check_output_alive
void check_output_alive(void) {
  int iVar1;
  
  if (monitor_output == '\x01') {
    iVar1 = iopoll(0xffffffff,1,0);
    if (iVar1 == -2) {
      die_pipe();
    }
  }
  return;
}

// Function: valid_file_spec
bool valid_file_spec(long param_1) {
  return (uint)(*(int *)(param_1 + 0x48) == 0) != *(uint *)(param_1 + 0x44) >> 0x1f;
}

// Function: xlseek
__off_t xlseek(int param_1,__off_t param_2,int param_3,undefined8 param_4) {
  __off_t _Var1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  
  _Var1 = lseek(param_1,param_2,param_3);
  if (_Var1 < 0) {
    uVar2 = quotearg_n_style_colon(0,3,param_4);
    uVar3 = gettext(*(undefined8 *)(whence_msgid_5 + (long)param_3 * 8));
    piVar4 = __errno_location();
    _Var1 = error(1,*piVar4,uVar3,uVar2,param_2);
  }
  return _Var1;
}

// Function: record_open_fd
void record_open_fd(long param_1,undefined4 param_2,long param_3,undefined8 *param_4,
                   undefined4 param_5) {
  undefined auVar1 [16];
  
  *(undefined4 *)(param_1 + 0x44) = param_2;
  auVar1 = get_stat_mtime(param_4);
  *(undefined (*) [16])(param_1 + 0x10) = auVar1;
  *(undefined8 *)(param_1 + 0x20) = *param_4;
  *(undefined8 *)(param_1 + 0x28) = param_4[1];
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(param_4 + 3);
  if ((*(uint *)(param_4 + 3) & 0xf000) == 0x8000) {
    if (param_3 < 0) {
      param_3 = xlseek(param_2,0,1,*(undefined8 *)(param_1 + 8));
    }
    *(long *)(param_1 + 0x38) = param_3;
  }
  *(undefined4 *)(param_1 + 0x4c) = param_5;
  *(undefined8 *)(param_1 + 0x60) = 0;
  *(undefined *)(param_1 + 0x40) = 0;
  return;
}

// Function: close_fd
void close_fd(int param_1,long param_2) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  
  if (0 < param_1) {
    iVar1 = close(param_1);
    if (iVar1 < 0) {
      uVar2 = quotearg_style(4,*(undefined8 *)(param_2 + 8));
      uVar3 = gettext("closing %s (fd=%d)");
      piVar4 = __errno_location();
      error(0,*piVar4,uVar3,uVar2,param_1);
    }
  }
  return;
}

// Function: write_header
void write_header(undefined8 param_1) {
  undefined *puVar1;
  
  if (first_file_4 == '\0') {
    puVar1 = &DAT_00108c15;
  }
  else {
    puVar1 = &DAT_001080c4;
  }
  printf("%s==> %s <==\n",puVar1,param_1);
  first_file_4 = 0;
  return;
}

// Function: xwrite_stdout
void xwrite_stdout(void *param_1,ulong param_2) {
  size_t sVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  
  if (0 < (long)param_2) {
    sVar1 = fwrite_unlocked(param_1,1,param_2,_stdout);
    if (sVar1 < param_2) {
      clearerr_unlocked(_stdout);
      uVar2 = quotearg_style(4,"standard output");
      uVar3 = gettext("error writing %s");
      piVar4 = __errno_location();
      error(1,*piVar4,uVar3,uVar2);
    }
  }
  return;
}

// Function: dump_remainder
long dump_remainder(char param_1,undefined8 param_2,int param_3,size_t param_4) {
  size_t __nbytes;
  ssize_t sVar1;
  int *piVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  char local_205c;
  long local_2050;
  size_t local_2048;
  undefined local_2028 [8200];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_2050 = 0;
  local_205c = param_1;
  local_2048 = param_4;
  do {
    if (((long)param_4 < 0) || (__nbytes = local_2048, 0x2000 < (long)local_2048)) {
      __nbytes = 0x2000;
    }
    sVar1 = read(param_3,local_2028,__nbytes);
    if (sVar1 < 0) {
      piVar2 = __errno_location();
      if (*piVar2 == 0xb) {
LAB_00101eed:
        if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
          return local_2050;
        }
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      uVar3 = quotearg_style(4,param_2);
      uVar4 = gettext("error reading %s");
      piVar2 = __errno_location();
      error(1,*piVar2,uVar4,uVar3);
    }
    if (sVar1 == 0) goto LAB_00101eed;
    local_2050 = local_2050 + sVar1;
    if (local_205c != '\0') {
      write_header(param_2);
      local_205c = '\0';
    }
    xwrite_stdout(local_2028,sVar1);
    if ((param_4 == 0xfffffffffffffffe) || (local_2048 = local_2048 - sVar1, local_2048 == 0))
    goto LAB_00101eed;
  } while( true );
}

// Function: file_lines
long file_lines(undefined8 param_1,int param_2,long param_3,long param_4,long param_5,long param_6) {
  void *__buf;
  uint *puVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  void *pvVar5;
  long lVar6;
  bool bVar7;
  long local_98;
  size_t local_60;
  long local_58;
  size_t local_50;
  size_t local_48;
  size_t local_40;
  
  local_60 = 0x2000;
  local_58 = param_6;
  if (param_4 != 0) {
    if ((*(uint *)(param_3 + 0x18) & 0xf000) != 0x8000) {
                    /* WARNING: Subroutine does not return */
      __assert_fail("((((sb->st_mode)) & 0170000) == (0100000))",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tail.c",
                    0x21b,"file_lines");
    }
    if ((*(long *)(param_3 + 0x30) % (long)page_size == 0) &&
       (local_60 = page_size, (long)page_size < 0x2000)) {
      local_60 = 0x2000;
    }
    __buf = (void *)ximalloc(local_60);
    local_50 = (param_6 - param_5) % (long)local_60;
    if (local_50 == 0) {
      local_50 = local_60;
    }
    local_58 = xlseek(param_2,-local_50,1,param_1);
    local_48 = read(param_2,__buf,local_50);
    if ((long)local_48 < 0) {
      puVar1 = (uint *)__errno_location();
      local_58 = (long)(int)~*puVar1;
      uVar2 = quotearg_style(4,param_1);
      uVar3 = gettext("error reading %s");
      piVar4 = __errno_location();
      error(0,*piVar4,uVar3,uVar2);
    }
    else {
      local_98 = param_4;
      if ((local_48 != 0) && (*(char *)((long)__buf + (local_48 - 1)) != line_end)) {
        local_98 = param_4 + -1;
      }
      do {
        local_58 = local_58 + local_48;
        local_40 = local_48;
        while ((local_40 != 0 &&
               (pvVar5 = memrchr(__buf,(int)line_end,local_40), pvVar5 != (void *)0x0))) {
          local_40 = (long)pvVar5 - (long)__buf;
          bVar7 = local_98 == 0;
          local_98 = local_98 + -1;
          if (bVar7) {
            xwrite_stdout((long)pvVar5 + 1,local_48 - (local_40 + 1));
            lVar6 = dump_remainder(0,param_1,param_2,param_6 - local_58);
            local_58 = local_58 + lVar6;
            goto LAB_001022b5;
          }
        }
        if (param_5 == local_58 - local_48) {
          xwrite_stdout(__buf,local_48);
          dump_remainder(0,param_1,param_2,param_6 - local_58);
          break;
        }
        local_58 = xlseek(param_2,-(local_48 + local_60),1,param_1);
        local_48 = read(param_2,__buf,local_60);
        if ((long)local_48 < 0) {
          puVar1 = (uint *)__errno_location();
          local_58 = (long)(int)~*puVar1;
          uVar2 = quotearg_style(4,param_1);
          uVar3 = gettext("error reading %s");
          piVar4 = __errno_location();
          error(0,*piVar4,uVar3,uVar2);
          break;
        }
      } while (0 < (long)local_48);
    }
LAB_001022b5:
    free(__buf);
  }
  return local_58;
}

// Function: pipe_lines
uint pipe_lines(undefined8 param_1,int param_2,long param_3) {
  long *plVar1;
  long lVar2;
  void *pvVar3;
  ssize_t sVar4;
  int *piVar5;
  uint *puVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  uint local_7c;
  void *local_70;
  void *local_68;
  void *local_60;
  long local_58;
  void *local_50;
  void *local_48;
  long local_40;
  
  local_58 = 0;
  local_7c = 0xffffffff;
  local_70 = (void *)xmalloc(0x2018);
  *(undefined8 *)((long)local_70 + 0x2008) = 0;
  *(undefined8 *)((long)local_70 + 0x2000) = *(undefined8 *)((long)local_70 + 0x2008);
  *(undefined8 *)((long)local_70 + 0x2010) = 0;
  local_60 = (void *)xmalloc(0x2018);
  local_68 = local_70;
  while (sVar4 = read(param_2,local_60,0x2000), 0 < sVar4) {
    *(ssize_t *)((long)local_60 + 0x2000) = sVar4;
    *(undefined8 *)((long)local_60 + 0x2008) = 0;
    *(undefined8 *)((long)local_60 + 0x2010) = 0;
    local_50 = local_60;
    while (local_50 = memchr(local_50,(int)line_end,(long)local_60 + (sVar4 - (long)local_50)),
          local_50 != (void *)0x0) {
      local_50 = (void *)((long)local_50 + 1);
      *(long *)((long)local_60 + 0x2008) = *(long *)((long)local_60 + 0x2008) + 1;
    }
    local_58 = local_58 + *(long *)((long)local_60 + 0x2008);
    if (*(long *)((long)local_68 + 0x2000) + *(long *)((long)local_60 + 0x2000) < 0x2000) {
      memcpy((void *)(*(long *)((long)local_68 + 0x2000) + (long)local_68),local_60,
             *(size_t *)((long)local_60 + 0x2000));
      *(long *)((long)local_68 + 0x2000) =
           *(long *)((long)local_68 + 0x2000) + *(long *)((long)local_60 + 0x2000);
      *(long *)((long)local_68 + 0x2008) =
           *(long *)((long)local_68 + 0x2008) + *(long *)((long)local_60 + 0x2008);
    }
    else {
      *(void **)((long)local_68 + 0x2010) = local_60;
      local_68 = *(void **)((long)local_68 + 0x2010);
      if (param_3 < local_58 - *(long *)((long)local_70 + 0x2008)) {
        local_60 = local_70;
        local_58 = local_58 - *(long *)((long)local_70 + 0x2008);
        local_70 = *(void **)((long)local_70 + 0x2010);
      }
      else {
        local_60 = (void *)xmalloc(0x2018);
      }
    }
  }
  free(local_60);
  if ((sVar4 < 0) && (piVar5 = __errno_location(), *piVar5 != 0xb)) {
    puVar6 = (uint *)__errno_location();
    local_7c = ~*puVar6;
    uVar7 = quotearg_style(4,param_1);
    uVar8 = gettext("error reading %s");
    piVar5 = __errno_location();
    error(0,*piVar5,uVar8,uVar7);
  }
  else if ((*(long *)((long)local_68 + 0x2000) != 0) && (param_3 != 0)) {
    if (*(char *)((long)local_68 + *(long *)((long)local_68 + 0x2000) + -1) != line_end) {
      *(long *)((long)local_68 + 0x2008) = *(long *)((long)local_68 + 0x2008) + 1;
      local_58 = local_58 + 1;
    }
    local_60 = local_70;
    for (; param_3 < local_58 - *(long *)((long)local_60 + 0x2008); local_58 = local_58 - *plVar1) {
      plVar1 = (long *)((long)local_60 + 0x2008);
      local_60 = *(void **)((long)local_60 + 0x2010);
    }
    local_48 = local_60;
    lVar2 = *(long *)((long)local_60 + 0x2000);
    if (param_3 < local_58) {
      for (local_40 = local_58 - param_3; local_40 != 0; local_40 = local_40 + -1) {
        local_48 = rawmemchr(local_48,(int)line_end);
        local_48 = (void *)((long)local_48 + 1);
      }
    }
    xwrite_stdout(local_48,(long)local_60 + (lVar2 - (long)local_48));
    for (local_60 = *(void **)((long)local_60 + 0x2010); local_60 != (void *)0x0;
        local_60 = *(void **)((long)local_60 + 0x2010)) {
      xwrite_stdout(local_60,*(undefined8 *)((long)local_60 + 0x2000));
    }
  }
  while (local_70 != (void *)0x0) {
    pvVar3 = *(void **)((long)local_70 + 0x2010);
    free(local_70);
    local_70 = pvVar3;
  }
  return local_7c;
}

// Function: pipe_bytes
long pipe_bytes(undefined8 param_1,int param_2,long param_3,long param_4) {
  long *plVar1;
  void *pvVar2;
  ssize_t sVar3;
  int *piVar4;
  uint *puVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  long local_78;
  void *local_50;
  void *local_48;
  void *local_40;
  long local_38;
  long local_30;
  
  local_30 = 0;
  local_78 = param_4;
  if (param_4 < 0) {
    local_78 = -0x8000000000000000;
  }
  local_50 = (void *)xmalloc(0x2010);
  *(undefined8 *)((long)local_50 + 0x2000) = 0;
  *(undefined8 *)((long)local_50 + 0x2008) = 0;
  local_40 = (void *)xmalloc(0x2010);
  local_48 = local_50;
  while (sVar3 = read(param_2,local_40,0x2000), 0 < sVar3) {
    local_78 = local_78 + sVar3;
    *(ssize_t *)((long)local_40 + 0x2000) = sVar3;
    *(undefined8 *)((long)local_40 + 0x2008) = 0;
    local_30 = local_30 + *(long *)((long)local_40 + 0x2000);
    if (*(long *)((long)local_48 + 0x2000) + *(long *)((long)local_40 + 0x2000) < 0x2000) {
      memcpy((void *)(*(long *)((long)local_48 + 0x2000) + (long)local_48),local_40,
             *(size_t *)((long)local_40 + 0x2000));
      *(long *)((long)local_48 + 0x2000) =
           *(long *)((long)local_48 + 0x2000) + *(long *)((long)local_40 + 0x2000);
    }
    else {
      *(void **)((long)local_48 + 0x2008) = local_40;
      local_48 = *(void **)((long)local_48 + 0x2008);
      if (param_3 < local_30 - *(long *)((long)local_50 + 0x2000)) {
        local_40 = local_50;
        local_30 = local_30 - *(long *)((long)local_50 + 0x2000);
        local_50 = *(void **)((long)local_50 + 0x2008);
      }
      else {
        local_40 = (void *)xmalloc(0x2010);
      }
    }
  }
  free(local_40);
  if ((sVar3 < 0) && (piVar4 = __errno_location(), *piVar4 != 0xb)) {
    puVar5 = (uint *)__errno_location();
    local_78 = (long)(int)~*puVar5;
    uVar6 = quotearg_style(4,param_1);
    uVar7 = gettext("error reading %s");
    piVar4 = __errno_location();
    error(0,*piVar4,uVar7,uVar6);
  }
  else {
    local_40 = local_50;
    for (; param_3 < local_30 - *(long *)((long)local_40 + 0x2000); local_30 = local_30 - *plVar1) {
      plVar1 = (long *)((long)local_40 + 0x2000);
      local_40 = *(void **)((long)local_40 + 0x2008);
    }
    if (param_3 < local_30) {
      local_38 = local_30 - param_3;
    }
    else {
      local_38 = 0;
    }
    xwrite_stdout(local_38 + (long)local_40,*(long *)((long)local_40 + 0x2000) - local_38);
    for (local_40 = *(void **)((long)local_40 + 0x2008); local_40 != (void *)0x0;
        local_40 = *(void **)((long)local_40 + 0x2008)) {
      xwrite_stdout(local_40,*(undefined8 *)((long)local_40 + 0x2000));
    }
    if (local_78 < 0) {
      local_78 = -1;
    }
  }
  while (local_50 != (void *)0x0) {
    pvVar2 = *(void **)((long)local_50 + 0x2008);
    free(local_50);
    local_50 = pvVar2;
  }
  return local_78;
}

// Function: start_bytes
uint start_bytes(undefined8 param_1,int param_2,long param_3) {
  uint uVar1;
  ssize_t sVar2;
  uint *puVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  long in_FS_OFFSET;
  long local_2060;
  undefined local_2028 [8200];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_2060 = param_3;
  do {
    if (local_2060 < 1) {
LAB_00102c43:
      uVar1 = 0;
LAB_00102c48:
      if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return uVar1;
    }
    sVar2 = read(param_2,local_2028,0x2000);
    if (sVar2 == 0) {
      uVar1 = 0xffffffff;
      goto LAB_00102c48;
    }
    if (sVar2 < 0) {
      puVar3 = (uint *)__errno_location();
      uVar1 = ~*puVar3;
      uVar4 = quotearg_style(4,param_1);
      uVar5 = gettext("error reading %s");
      piVar6 = __errno_location();
      error(0,*piVar6,uVar5,uVar4);
      goto LAB_00102c48;
    }
    if (local_2060 < sVar2) {
      xwrite_stdout(local_2028 + local_2060,sVar2 - local_2060);
      goto LAB_00102c43;
    }
    local_2060 = local_2060 - sVar2;
  } while( true );
}

// Function: start_lines
uint start_lines(undefined8 param_1,int param_2,long param_3) {
  uint uVar1;
  ssize_t sVar2;
  uint *puVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  undefined *puVar7;
  void *pvVar8;
  long in_FS_OFFSET;
  long local_2070;
  undefined *local_2048;
  undefined auStack_2029 [8201];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_2070 = param_3;
  if (param_3 == 0) {
    uVar1 = 0;
  }
  else {
    while (sVar2 = read(param_2,(undefined *)((long)register0x00000020 + -0x2028),0x2000),
          sVar2 != 0) {
      if (sVar2 < 0) {
        puVar3 = (uint *)__errno_location();
        uVar1 = ~*puVar3;
        uVar4 = quotearg_style(4,param_1);
        uVar5 = gettext("error reading %s");
        piVar6 = __errno_location();
        error(0,*piVar6,uVar5,uVar4);
        goto LAB_00102e51;
      }
      puVar7 = (undefined *)((long)register0x00000020 + -0x2028) + sVar2;
      local_2048 = (undefined *)((long)register0x00000020 + -0x2028);
      while (pvVar8 = memchr(local_2048,(int)line_end,(long)puVar7 - (long)local_2048),
            pvVar8 != (void *)0x0) {
        local_2048 = (undefined *)((long)pvVar8 + 1);
        local_2070 = local_2070 + -1;
        if (local_2070 == 0) {
          if (local_2048 < puVar7) {
            xwrite_stdout(local_2048,(long)puVar7 - (long)local_2048);
          }
          uVar1 = 0;
          goto LAB_00102e51;
        }
      }
    }
    uVar1 = 0xffffffff;
  }
LAB_00102e51:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar1;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: fremote
bool fremote(int param_1,long param_2) {
  int iVar1;
  int *piVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  bool local_ad;
  statfs local_98;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_ad = true;
  iVar1 = fstatfs(param_1,&local_98);
  if (iVar1 == 0) {
    iVar1 = is_local_fs_type(local_98.f_type);
    local_ad = iVar1 < 1;
  }
  else {
    piVar2 = __errno_location();
    if (*piVar2 != 0x26) {
      uVar3 = quotearg_style(4,*(undefined8 *)(param_2 + 8));
      uVar4 = gettext("cannot determine location of %s. reverting to polling");
      piVar2 = __errno_location();
      error(0,*piVar2,uVar4,uVar3);
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_ad;
}

// Function: recheck
void recheck(undefined8 *param_1,byte param_2) {
  int iVar1;
  bool bVar2;
  char cVar3;
  char cVar4;
  undefined uVar5;
  int __fd;
  int iVar6;
  int iVar7;
  int *piVar8;
  undefined8 uVar9;
  undefined *puVar10;
  uint uVar11;
  undefined8 uVar12;
  long in_FS_OFFSET;
  char local_147;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_147 = '\0';
  cVar3 = streq(*param_1,&DAT_00108d0e);
  iVar1 = *(int *)(param_1 + 9);
  if (cVar3 == '\0') {
    if (param_2 == 0) {
      uVar12 = 0x800;
    }
    else {
      uVar12 = 0;
    }
    __fd = open_safer(*param_1,uVar12);
  }
  else {
    __fd = 0;
  }
  if (__fd < 0) {
    piVar8 = __errno_location();
    iVar6 = *piVar8;
  }
  else {
    iVar6 = 0;
  }
  cVar4 = valid_file_spec(param_1);
  if (cVar4 == '\0') {
                    /* WARNING: Subroutine does not return */
    __assert_fail("valid_file_spec (f)",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tail.c",
                  0x3d2,"recheck");
  }
  if ((disable_inotify == '\x01') || (iVar7 = issymlink(*param_1), iVar7 != 1)) {
    if ((__fd < 0) || (iVar7 = fstat(__fd,&local_b8), iVar7 < 0)) {
      if (-1 < __fd) {
        piVar8 = __errno_location();
        iVar6 = *piVar8;
      }
      *(int *)(param_1 + 9) = iVar6;
      if (__fd < 0) {
        if (*(char *)((long)param_1 + 0x42) != '\0') {
          uVar12 = quotearg_style(4,param_1[1]);
          uVar9 = gettext("%s has become inaccessible");
          error(0,*(undefined4 *)(param_1 + 9),uVar9,uVar12);
        }
      }
      else if (iVar1 != *(int *)(param_1 + 9)) {
        uVar12 = quotearg_n_style_colon(0,3,param_1[1]);
        error(0,*(undefined4 *)(param_1 + 9),&DAT_00108d79,uVar12);
      }
    }
    else if (((local_b8.st_mode & 0xf000) == 0x8000) ||
            ((((local_b8.st_mode & 0xf000) == 0x1000 || ((local_b8.st_mode & 0xf000) == 0xc000)) ||
             ((local_b8.st_mode & 0xf000) == 0x2000)))) {
      uVar5 = fremote(__fd,param_1);
      *(undefined *)((long)param_1 + 0x41) = uVar5;
      if ((*(char *)((long)param_1 + 0x41) == '\0') || (disable_inotify == '\x01')) {
        local_147 = '\x01';
        *(undefined4 *)(param_1 + 9) = 0;
      }
      else {
        *(undefined4 *)(param_1 + 9) = 0xffffffff;
        uVar12 = quotearg_style(4,param_1[1]);
        uVar9 = gettext("%s has been replaced with an untailable remote file");
        error(0,0,uVar9,uVar12);
        *(undefined *)(param_1 + 8) = 1;
      }
    }
    else {
      *(undefined4 *)(param_1 + 9) = 0xffffffff;
      if ((reopen_inaccessible_files == '\x01') && (follow_mode == 1)) {
        uVar5 = 0;
      }
      else {
        uVar5 = 1;
      }
      *(undefined *)(param_1 + 8) = uVar5;
      if ((*(char *)((long)param_1 + 0x42) != '\0') || ((-1 < iVar1 && (iVar1 != 0x15)))) {
        if (*(char *)(param_1 + 8) == '\0') {
          puVar10 = &DAT_001080c4;
        }
        else {
          puVar10 = (undefined *)gettext("; giving up on this name");
        }
        uVar12 = quotearg_style(4,param_1[1]);
        uVar9 = gettext("%s has been replaced with an untailable file%s");
        error(0,0,uVar9,uVar12,puVar10);
      }
    }
  }
  else {
    *(undefined4 *)(param_1 + 9) = 0xffffffff;
    *(undefined *)(param_1 + 8) = 1;
    uVar12 = quotearg_style(4,param_1[1]);
    uVar9 = gettext("%s has been replaced with an untailable symbolic link");
    error(0,0,uVar9,uVar12);
  }
  *(char *)((long)param_1 + 0x42) = local_147;
  bVar2 = false;
  if (local_147 == '\x01') {
    if ((iVar1 == 0) || (iVar1 == 2)) {
      if (*(int *)((long)param_1 + 0x44) < 0) {
        bVar2 = true;
        uVar12 = quotearg_style(4,param_1[1]);
        uVar9 = gettext("%s has appeared;  following new file");
        error(0,0,uVar9,uVar12);
      }
      else if ((local_b8.st_ino ^ param_1[5] | param_1[4] ^ local_b8.st_dev) == 0) {
        close_fd(__fd,param_1);
      }
      else {
        bVar2 = true;
        uVar12 = quotearg_style(4,param_1[1]);
        uVar9 = gettext("%s has been replaced;  following new file");
        error(0,0,uVar9,uVar12);
        close_fd(*(undefined4 *)((long)param_1 + 0x44),param_1);
      }
    }
    else {
      bVar2 = true;
      if (-1 < *(int *)((long)param_1 + 0x44)) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("f->fd < 0",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tail.c"
                      ,0x417,"recheck");
      }
      uVar12 = quotearg_style(4,param_1[1]);
      uVar9 = gettext("%s has become accessible");
      error(0,0,uVar9,uVar12);
    }
  }
  else {
    close_fd(__fd,param_1);
    close_fd(*(undefined4 *)((long)param_1 + 0x44),param_1);
    *(undefined4 *)((long)param_1 + 0x44) = 0xffffffff;
  }
  if (bVar2) {
    if (cVar3 == '\0') {
      uVar11 = (uint)param_2;
    }
    else {
      uVar11 = 0xffffffff;
    }
    record_open_fd(param_1,__fd,0xffffffffffffffff,&local_b8,uVar11);
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: any_live_files
undefined8 any_live_files(long param_1,int param_2) {
  undefined8 uVar1;
  int local_c;
  
  if ((reopen_inaccessible_files == '\0') || (follow_mode != 1)) {
    for (local_c = 0; local_c < param_2; local_c = local_c + 1) {
      if (-1 < *(int *)(param_1 + (long)local_c * 0x68 + 0x44)) {
        return 1;
      }
      if ((*(char *)(param_1 + (long)local_c * 0x68 + 0x40) != '\x01') &&
         (reopen_inaccessible_files != '\0')) {
        return 1;
      }
    }
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

// Function: some_writers_exist
bool some_writers_exist(void) {
  int iVar1;
  int *piVar2;
  long local_10;
  
  local_10 = 0;
LAB_00103939:
  do {
    if (nbpids <= local_10) {
      return 0 < nbpids;
    }
    iVar1 = kill(*(__pid_t *)(pids + local_10 * 4),0);
    if (iVar1 < 0) {
      piVar2 = __errno_location();
      if (*piVar2 == 3) {
        nbpids = nbpids + -1;
        memmove((void *)(pids + local_10 * 4),(void *)((local_10 + 1) * 4 + pids),
                (nbpids - local_10) * 4);
        goto LAB_00103939;
      }
    }
    local_10 = local_10 + 1;
  } while( true );
}

// Function: tail_forever
void tail_forever(undefined8 param_1,long param_2,int param_3) {
  uint uVar1;
  bool bVar2;
  bool bVar3;
  byte bVar4;
  char cVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  undefined8 uVar10;
  undefined8 uVar11;
  long lVar12;
  undefined8 uVar13;
  int *piVar14;
  long lVar15;
  long in_FS_OFFSET;
  undefined auVar16 [16];
  int local_138;
  int local_134;
  long local_f8;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_138 = param_3 + -1;
LAB_0010399d:
  do {
    if ((((nbpids == 0) && (follow_mode == 2)) && (param_3 == 1)) &&
       ((-1 < *(int *)(param_2 + 0x44) && ((*(uint *)(param_2 + 0x30) & 0xf000) != 0x8000)))) {
      bVar4 = 1;
    }
    else {
      bVar4 = 0;
    }
    bVar3 = false;
    for (local_134 = 0; local_134 < param_3; local_134 = local_134 + 1) {
      if (*(char *)(param_2 + (long)local_134 * 0x68 + 0x40) == '\0') {
        iVar9 = *(int *)(param_2 + (long)local_134 * 0x68 + 0x44);
        if (iVar9 < 0) {
          recheck(param_2 + (long)local_134 * 0x68,bVar4);
        }
        else {
          uVar13 = *(undefined8 *)(param_2 + (long)local_134 * 0x68 + 8);
          uVar1 = *(uint *)(param_2 + (long)local_134 * 0x68 + 0x30);
          if (*(uint *)(param_2 + (long)local_134 * 0x68 + 0x4c) != (uint)bVar4) {
            uVar6 = rpl_fcntl(iVar9,3);
            if (bVar4 == 0) {
              uVar7 = 0x800;
            }
            else {
              uVar7 = 0;
            }
            if (((int)uVar6 < 0) ||
               (((uVar7 | uVar6) != uVar6 && (iVar8 = rpl_fcntl(iVar9,4,uVar7 | uVar6), iVar8 < 0)))
               ) {
              if (((*(uint *)(param_2 + (long)local_134 * 0x68 + 0x30) & 0xf000) != 0x8000) ||
                 (piVar14 = __errno_location(), *piVar14 != 1)) {
                uVar10 = quotearg_n_style_colon(0,3,uVar13);
                uVar11 = gettext("%s: cannot change nonblocking mode");
                piVar14 = __errno_location();
                error(1,*piVar14,uVar11,uVar10);
              }
            }
            else {
              *(uint *)((long)local_134 * 0x68 + param_2 + 0x4c) = (uint)bVar4;
            }
          }
          bVar2 = false;
          if (*(int *)(param_2 + (long)local_134 * 0x68 + 0x4c) == 0) {
            iVar8 = fstat(iVar9,&local_b8);
            if (iVar8 < 0) {
              *(undefined4 *)(param_2 + (long)local_134 * 0x68 + 0x44) = 0xffffffff;
              piVar14 = __errno_location();
              *(int *)((long)local_134 * 0x68 + param_2 + 0x48) = *piVar14;
              uVar13 = quotearg_n_style_colon(0,3,uVar13);
              piVar14 = __errno_location();
              error(0,*piVar14,&DAT_00108d79,uVar13);
              close(iVar9);
              goto LAB_00104457;
            }
            if ((*(uint *)(param_2 + (long)local_134 * 0x68 + 0x30) == local_b8.st_mode) &&
               (((local_b8.st_mode & 0xf000) != 0x8000 ||
                (*(long *)(param_2 + (long)local_134 * 0x68 + 0x38) == local_b8.st_size)))) {
              auVar16 = get_stat_mtime(&local_b8);
              lVar12 = param_2 + (long)local_134 * 0x68;
              iVar8 = timespec_cmp(*(undefined8 *)(lVar12 + 0x10),*(undefined8 *)(lVar12 + 0x18),
                                   auVar16._0_8_,auVar16._8_8_);
              if (iVar8 == 0) {
                lVar15 = (long)local_134 * 0x68 + param_2;
                lVar12 = *(long *)(lVar15 + 0x60);
                *(long *)(lVar15 + 0x60) = lVar12 + 1;
                if ((max_n_unchanged_stats_between_opens <= lVar12) && (follow_mode == 1)) {
                  recheck(param_2 + (long)local_134 * 0x68,
                          *(int *)(param_2 + (long)local_134 * 0x68 + 0x4c) != 0);
                  *(undefined8 *)(param_2 + (long)local_134 * 0x68 + 0x60) = 0;
                }
                if (((iVar9 != *(int *)(param_2 + (long)local_134 * 0x68 + 0x44)) ||
                    ((local_b8.st_mode & 0xf000) == 0x8000)) || (1 < param_3)) goto LAB_00104457;
                bVar2 = true;
              }
            }
            if (iVar9 != *(int *)(param_2 + (long)local_134 * 0x68 + 0x44)) {
                    /* WARNING: Subroutine does not return */
              __assert_fail("fd == f[i].fd",
                            "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tail.c"
                            ,0x4d3,"tail_forever");
            }
            auVar16 = get_stat_mtime(&local_b8);
            *(undefined (*) [16])((long)local_134 * 0x68 + param_2 + 0x10) = auVar16;
            *(__mode_t *)((long)local_134 * 0x68 + param_2 + 0x30) = local_b8.st_mode;
            if (!bVar2) {
              *(undefined8 *)(param_2 + (long)local_134 * 0x68 + 0x60) = 0;
            }
            if (((uVar1 & 0xf000) == 0x8000) &&
               (local_b8.st_size < *(long *)(param_2 + (long)local_134 * 0x68 + 0x38))) {
              uVar10 = quotearg_n_style_colon(0,3,uVar13);
              uVar11 = gettext("%s: file truncated");
              error(0,0,uVar11,uVar10);
              uVar10 = xlseek(iVar9,0,0,uVar13);
              *(undefined8 *)((long)local_134 * 0x68 + param_2 + 0x38) = uVar10;
            }
            if (local_134 != local_138) {
              if (print_headers != '\0') {
                write_header(uVar13);
              }
              local_138 = local_134;
            }
          }
          if (*(int *)(param_2 + (long)local_134 * 0x68 + 0x4c) == 0) {
            if (((uVar1 & 0xf000) == 0x8000) &&
               (*(char *)(param_2 + (long)local_134 * 0x68 + 0x41) != '\0')) {
              local_f8 = local_b8.st_size - *(long *)(param_2 + (long)local_134 * 0x68 + 0x38);
            }
            else {
              local_f8 = -1;
            }
          }
          else {
            local_f8 = -2;
          }
          lVar12 = dump_remainder(0,uVar13,iVar9,local_f8);
          if (0 < lVar12) {
            if ((uVar1 & 0xf000) == 0x8000) {
              *(long *)(param_2 + (long)local_134 * 0x68 + 0x38) =
                   lVar12 + *(long *)(param_2 + (long)local_134 * 0x68 + 0x38);
            }
            if (bVar2) {
              *(undefined8 *)(param_2 + (long)local_134 * 0x68 + 0x60) = 0;
            }
            bVar3 = true;
          }
        }
      }
LAB_00104457:
    }
    cVar5 = any_live_files(param_2,param_3);
    if (cVar5 != '\x01') {
      uVar13 = gettext("no files remaining");
      error(1,0,uVar13);
    }
    if (((!bVar3) || (bVar4 != 0)) && (iVar9 = fflush_unlocked(_stdout), iVar9 < 0)) {
      write_error();
    }
    check_output_alive();
  } while (bVar3);
  if (pids != 0) goto code_r0x0010452c;
  goto LAB_00104542;
code_r0x0010452c:
  if (nbpids == 0) {
    if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
      return;
    }
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  cVar5 = some_writers_exist();
  if (cVar5 == '\x01') {
LAB_00104542:
    iVar9 = xnanosleep(param_1);
    if (iVar9 != 0) {
      uVar13 = gettext("cannot read realtime clock");
      piVar14 = __errno_location();
      error(1,*piVar14,uVar13);
    }
  }
  goto LAB_0010399d;
}

// Function: any_remote_file
undefined8 any_remote_file(long param_1,int param_2) {
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (param_2 <= local_c) {
      return 0;
    }
    if ((-1 < *(int *)(param_1 + (long)local_c * 0x68 + 0x44)) &&
       (*(char *)(param_1 + (long)local_c * 0x68 + 0x41) != '\0')) break;
    local_c = local_c + 1;
  }
  return 1;
}

// Function: any_non_remote_file
undefined8 any_non_remote_file(long param_1,int param_2) {
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (param_2 <= local_c) {
      return 0;
    }
    if ((-1 < *(int *)(param_1 + (long)local_c * 0x68 + 0x44)) &&
       (*(char *)(param_1 + (long)local_c * 0x68 + 0x41) != '\x01')) break;
    local_c = local_c + 1;
  }
  return 1;
}

// Function: any_symlinks
undefined8 any_symlinks(long param_1,int param_2) {
  int iVar1;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (param_2 <= local_c) {
      return 0;
    }
    iVar1 = issymlink(*(undefined8 *)(param_1 + (long)local_c * 0x68));
    if (iVar1 == 1) break;
    local_c = local_c + 1;
  }
  return 1;
}

// Function: any_non_regular_fifo
undefined8 any_non_regular_fifo(long param_1,int param_2) {
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (param_2 <= local_c) {
      return 0;
    }
    if (((-1 < *(int *)(param_1 + (long)local_c * 0x68 + 0x44)) &&
        ((*(uint *)(param_1 + (long)local_c * 0x68 + 0x30) & 0xf000) != 0x8000)) &&
       ((*(uint *)(param_1 + (long)local_c * 0x68 + 0x30) & 0xf000) != 0x1000)) break;
    local_c = local_c + 1;
  }
  return 1;
}

// Function: tailable_stdin
undefined8 tailable_stdin(long param_1,int param_2) {
  char cVar1;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (param_2 <= local_c) {
      return 0;
    }
    if ((*(char *)(param_1 + (long)local_c * 0x68 + 0x40) != '\x01') &&
       (cVar1 = streq(*(undefined8 *)(param_1 + (long)local_c * 0x68),&DAT_00108d0e), cVar1 != '\0')
       ) break;
    local_c = local_c + 1;
  }
  return 1;
}

// Function: wd_hasher
ulong wd_hasher(long param_1,ulong param_2) {
  return (ulong)(long)*(int *)(param_1 + 0x50) % param_2;
}

// Function: wd_comparator
undefined4 wd_comparator(long param_1,long param_2) {
  return CONCAT31((int3)((uint)*(int *)(param_2 + 0x50) >> 8),
                  *(int *)(param_1 + 0x50) == *(int *)(param_2 + 0x50));
}

// Function: check_fspec
void check_fspec(long param_1,long *param_2) {
  undefined8 uVar1;
  undefined uVar2;
  int iVar3;
  int *piVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  long lVar7;
  long in_FS_OFFSET;
  undefined auVar8 [16];
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (-1 < *(int *)(param_1 + 0x44)) {
    iVar3 = fstat(*(int *)(param_1 + 0x44),&local_b8);
    if (iVar3 < 0) {
      piVar4 = __errno_location();
      *(int *)(param_1 + 0x48) = *piVar4;
      close_fd(*(undefined4 *)(param_1 + 0x44),param_1);
      *(undefined4 *)(param_1 + 0x44) = 0xffffffff;
    }
    else {
      uVar1 = *(undefined8 *)(param_1 + 8);
      if (((*(uint *)(param_1 + 0x30) & 0xf000) == 0x8000) &&
         (local_b8.st_size < *(long *)(param_1 + 0x38))) {
        uVar5 = quotearg_n_style_colon(0,3,uVar1);
        uVar6 = gettext("%s: file truncated");
        error(0,0,uVar6,uVar5);
        uVar5 = xlseek(*(undefined4 *)(param_1 + 0x44),0,0,uVar1);
        *(undefined8 *)(param_1 + 0x38) = uVar5;
      }
      else if (((*(uint *)(param_1 + 0x30) & 0xf000) == 0x8000) &&
              (local_b8.st_size == *(long *)(param_1 + 0x38))) {
        auVar8 = get_stat_mtime(&local_b8);
        iVar3 = timespec_cmp(*(undefined8 *)(param_1 + 0x10),*(undefined8 *)(param_1 + 0x18),
                             auVar8._0_8_,auVar8._8_8_);
        if (iVar3 == 0) goto LAB_00104bce;
      }
      if ((print_headers == '\0') || (param_1 == *param_2)) {
        uVar2 = 0;
      }
      else {
        uVar2 = 1;
      }
      lVar7 = dump_remainder(uVar2,uVar1,*(undefined4 *)(param_1 + 0x44),0xffffffffffffffff);
      if (0 < lVar7) {
        if ((*(uint *)(param_1 + 0x30) & 0xf000) == 0x8000) {
          *(long *)(param_1 + 0x38) = *(long *)(param_1 + 0x38) + lVar7;
        }
        *param_2 = param_1;
        iVar3 = fflush_unlocked(_stdout);
        if (iVar3 < 0) {
          write_error();
        }
      }
    }
  }
LAB_00104bce:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: tail_forever_inotify
void tail_forever_inotify(double param_1,int param_2,long param_3,int param_4,long *param_5) {
  undefined uVar1;
  bool bVar2;
  bool bVar3;
  bool bVar4;
  char cVar5;
  int iVar6;
  long lVar7;
  undefined8 uVar8;
  int *piVar9;
  undefined8 uVar10;
  char *__name;
  long in_FS_OFFSET;
  bool bVar11;
  int local_264;
  uint32_t local_260;
  int local_25c;
  int local_258;
  int local_254;
  int local_250;
  int local_24c;
  int local_248;
  long local_1e8;
  size_t local_1e0;
  void *local_1d8;
  long local_1d0;
  ssize_t local_1c8;
  long local_1c0;
  long local_1b8;
  size_t local_1b0;
  long local_1a8;
  double local_1a0;
  int *local_198;
  int *local_190;
  long local_188;
  undefined8 local_180;
  undefined8 local_178;
  undefined8 local_170;
  undefined8 local_168;
  undefined8 local_160;
  undefined8 local_158;
  undefined8 local_150;
  undefined8 local_148;
  undefined8 local_140;
  undefined8 local_138;
  undefined8 local_130;
  undefined local_128 [80];
  int local_d8;
  undefined local_b8 [6];
  short sStack_b2;
  undefined4 local_b0;
  undefined2 uStack_ac;
  short sStack_aa;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_264 = 3;
  bVar4 = false;
  bVar3 = false;
  bVar2 = false;
  bVar11 = false;
  local_1e0 = 0;
  local_1d0 = 0;
  local_1b8 = hash_initialize((long)param_4,0,wd_hasher,wd_comparator,0);
  if (local_1b8 == 0) {
    xalloc_die();
  }
  *param_5 = local_1b8;
  local_260 = 2;
  if (follow_mode == 1) {
    local_260 = 0xc06;
  }
  for (local_25c = 0; local_25c < param_4; local_25c = local_25c + 1) {
    if (*(char *)(param_3 + (long)local_25c * 0x68 + 0x40) != '\x01') {
      local_1b0 = strlen(*(char **)(param_3 + (long)local_25c * 0x68));
      if ((long)local_1e0 < (long)local_1b0) {
        local_1e0 = local_1b0;
      }
      *(undefined4 *)(param_3 + (long)local_25c * 0x68 + 0x50) = 0xffffffff;
      if (follow_mode == 1) {
        local_1a8 = dir_len(*(undefined8 *)(param_3 + (long)local_25c * 0x68));
        uVar1 = *(undefined *)(local_1a8 + *(long *)(param_3 + (long)local_25c * 0x68));
        lVar7 = last_component(*(undefined8 *)(param_3 + (long)local_25c * 0x68));
        *(long *)(param_3 + (long)local_25c * 0x68 + 0x58) =
             lVar7 - *(long *)(param_3 + (long)local_25c * 0x68);
        *(undefined *)(local_1a8 + *(long *)(param_3 + (long)local_25c * 0x68)) = 0;
        if (local_1a8 == 0) {
          __name = ".";
        }
        else {
          __name = *(char **)(param_3 + (long)local_25c * 0x68);
        }
        iVar6 = inotify_add_watch(param_2,__name,0x784);
        *(int *)((long)local_25c * 0x68 + param_3 + 0x54) = iVar6;
        *(undefined *)(*(long *)(param_3 + (long)local_25c * 0x68) + local_1a8) = uVar1;
        if (*(int *)(param_3 + (long)local_25c * 0x68 + 0x54) < 0) {
          piVar9 = __errno_location();
          if (*piVar9 == 0x1c) {
            local_178 = gettext("inotify resources exhausted");
            error(0,0,local_178);
          }
          else {
            uVar8 = quotearg_style(4,*(undefined8 *)(param_3 + (long)local_25c * 0x68));
            uVar10 = gettext("cannot watch parent directory of %s");
            piVar9 = __errno_location();
            local_180 = uVar10;
            error(0,*piVar9,uVar10,uVar8);
          }
          bVar2 = true;
          break;
        }
      }
      iVar6 = inotify_add_watch(param_2,*(char **)(param_3 + (long)local_25c * 0x68),local_260);
      *(int *)((long)local_25c * 0x68 + param_3 + 0x50) = iVar6;
      if (*(int *)(param_3 + (long)local_25c * 0x68 + 0x50) < 0) {
        if (-1 < *(int *)(param_3 + (long)local_25c * 0x68 + 0x44)) {
          bVar3 = true;
        }
        piVar9 = __errno_location();
        if ((*piVar9 == 0x1c) || (piVar9 = __errno_location(), *piVar9 == 0xc)) {
          bVar11 = true;
          local_170 = gettext("inotify resources exhausted");
          error(0,0,local_170);
          break;
        }
        piVar9 = __errno_location();
        if (*piVar9 != *(int *)(param_3 + (long)local_25c * 0x68 + 0x48)) {
          uVar8 = quotearg_style(4,*(undefined8 *)(param_3 + (long)local_25c * 0x68));
          uVar10 = gettext("cannot watch %s");
          piVar9 = __errno_location();
          local_168 = uVar10;
          error(0,*piVar9,uVar10,uVar8);
        }
      }
      else {
        lVar7 = hash_insert(local_1b8,(long)local_25c * 0x68 + param_3);
        if (lVar7 == 0) {
          xalloc_die();
        }
        bVar4 = true;
      }
    }
  }
  if (((bVar11) || (bVar2)) || ((follow_mode == 2 && (bVar3)))) {
LAB_00106023:
    if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    return;
  }
  if ((follow_mode == 2) && (!bVar4)) {
                    /* WARNING: Subroutine does not return */
    exit(1);
  }
  local_1e8 = param_3 + (long)param_4 * 0x68 + -0x68;
  for (local_258 = 0; local_258 < param_4; local_258 = local_258 + 1) {
    if (*(char *)(param_3 + (long)local_258 * 0x68 + 0x40) != '\x01') {
      if (follow_mode == 1) {
        recheck(param_3 + (long)local_258 * 0x68,0);
      }
      else if (((-1 < *(int *)(param_3 + (long)local_258 * 0x68 + 0x44)) &&
               (iVar6 = stat(*(char **)(param_3 + (long)local_258 * 0x68),(stat *)local_b8),
               -1 < iVar6)) &&
              ((CONCAT26(sStack_aa,CONCAT24(uStack_ac,local_b0)) ^
                *(ulong *)(param_3 + (long)local_258 * 0x68 + 0x28) |
               *(ulong *)(param_3 + (long)local_258 * 0x68 + 0x20) ^
               CONCAT26(sStack_b2,CONCAT24(local_b8._4_2_,local_b8._0_4_))) != 0)) {
        uVar8 = quotearg_style(4,*(undefined8 *)(param_3 + (long)local_258 * 0x68 + 8));
        uVar10 = gettext("%s was replaced");
        piVar9 = __errno_location();
        local_160 = uVar10;
        error(0,*piVar9,uVar10,uVar8);
        goto LAB_00106023;
      }
      check_fspec((long)local_258 * 0x68 + param_3,&local_1e8);
    }
  }
  local_1e0 = local_1e0 + 0x11;
  local_1d8 = (void *)ximalloc(local_1e0);
  local_1c8 = 0;
LAB_001056ae:
  if (((follow_mode == 1) && (reopen_inaccessible_files != '\x01')) &&
     (lVar7 = hash_get_n_entries(local_1b8), lVar7 == 0)) {
    local_158 = gettext("no files remaining");
    error(1,0,local_158);
  }
  if (local_1c8 <= local_1d0) {
    do {
      local_254 = -1;
      if (pids != 0) {
        if (nbpids == 0) {
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        cVar5 = some_writers_exist();
        if ((cVar5 != '\x01') || (param_1 <= 0.0)) {
          local_254 = 0;
        }
        else if (param_1 < DAT_00109500) {
          local_1a0 = DAT_00109508 * param_1;
          local_254 = (int)local_1a0 + (uint)((double)(int)local_1a0 < local_1a0);
        }
      }
      local_b8._4_2_ = 1;
      local_b0 = 1;
      sStack_aa = 0;
      uStack_ac = 0;
      local_b8._0_4_ = param_2;
      iVar6 = poll((pollfd *)local_b8,(long)(int)(monitor_output + 1),local_254);
    } while (iVar6 == 0);
    if (iVar6 < 0) {
      uVar8 = gettext("error waiting for inotify and output events");
      piVar9 = __errno_location();
      local_150 = uVar8;
      error(1,*piVar9,uVar8);
    }
    if (sStack_aa != 0) {
      die_pipe();
    }
    local_1c8 = read(param_2,local_1d8,local_1e0);
    local_1d0 = 0;
    if (((local_1c8 == 0) || ((local_1c8 < 0 && (piVar9 = __errno_location(), *piVar9 == 0x16)))) &&
       (iVar6 = local_264 + -1, bVar11 = local_264 != 0, local_264 = iVar6, bVar11)) {
      local_1c8 = 0;
      local_1e0 = local_1e0 << 1;
      local_1d8 = (void *)xirealloc(local_1d8,local_1e0);
      goto LAB_001056ae;
    }
    if (local_1c8 < 1) {
      uVar8 = gettext("error reading inotify event");
      piVar9 = __errno_location();
      local_148 = uVar8;
      error(1,*piVar9,uVar8);
    }
  }
  local_198 = (int *)((long)local_1d8 + local_1d0);
  local_1d0 = local_1d0 + (ulong)(uint)local_198[3] + 0x10;
  local_190 = local_198;
  if (((local_198[1] & 0x400U) != 0) && (local_198[3] == 0)) {
    for (local_250 = 0; local_250 < param_4; local_250 = local_250 + 1) {
      if (*local_198 == *(int *)(param_3 + (long)local_250 * 0x68 + 0x54)) {
        local_140 = gettext("directory containing watched file was removed");
        error(0,0,local_140);
        goto LAB_00106023;
      }
    }
  }
  if (local_198[3] != 0) goto code_r0x00105afb;
  local_d8 = *local_198;
  local_1c0 = hash_lookup(local_1b8,local_128);
  goto LAB_00105f55;
code_r0x00105afb:
  local_24c = 0;
  while ((local_24c < param_4 &&
         ((*(int *)(param_3 + (long)local_24c * 0x68 + 0x54) != *local_190 ||
          (cVar5 = streq(local_190 + 4,
                         *(long *)(param_3 + (long)local_24c * 0x68) +
                         *(long *)(param_3 + (long)local_24c * 0x68 + 0x58)), cVar5 == '\0'))))) {
    local_24c = local_24c + 1;
  }
  if (local_24c != param_4) {
    local_1c0 = param_3 + (long)local_24c * 0x68;
    local_248 = -1;
    bVar11 = (local_190[1] & 0x200U) != 0;
    if ((!bVar11) &&
       (local_248 = inotify_add_watch(param_2,*(char **)(param_3 + (long)local_24c * 0x68),local_260
                                     ), local_248 < 0)) {
      piVar9 = __errno_location();
      if ((*piVar9 == 0x1c) || (piVar9 = __errno_location(), *piVar9 == 0xc)) {
        local_138 = gettext("inotify resources exhausted");
        error(0,0,local_138);
        goto LAB_00106023;
      }
      uVar8 = quotearg_style(4,*(undefined8 *)(param_3 + (long)local_24c * 0x68));
      uVar10 = gettext("cannot watch %s");
      piVar9 = __errno_location();
      local_130 = uVar10;
      error(0,*piVar9,uVar10,uVar8);
    }
    if ((bVar11) || ((-1 < *(int *)(local_1c0 + 0x50) && (local_248 == *(int *)(local_1c0 + 0x50))))
       ) {
      bVar11 = false;
    }
    else {
      bVar11 = true;
    }
    if (bVar11) {
      if (-1 < *(int *)(local_1c0 + 0x50)) {
        inotify_rm_watch(param_2,*(int *)(local_1c0 + 0x50));
        hash_remove(local_1b8,local_1c0);
      }
      *(int *)(local_1c0 + 0x50) = local_248;
      if (local_248 < 0) goto LAB_001056ae;
      local_188 = hash_remove(local_1b8,local_1c0);
      if ((local_188 != 0) && (local_188 != local_1c0)) {
        if (follow_mode == 1) {
          recheck(local_188,0);
        }
        *(undefined4 *)(local_188 + 0x50) = 0xffffffff;
        close_fd(*(undefined4 *)(local_188 + 0x44),local_188);
      }
      lVar7 = hash_insert(local_1b8,local_1c0);
      if (lVar7 == 0) {
        xalloc_die();
      }
    }
    if (follow_mode == 1) {
      recheck(local_1c0,0);
    }
LAB_00105f55:
    if (local_1c0 != 0) {
      if ((local_190[1] & 0xe04U) == 0) {
        check_fspec(local_1c0,&local_1e8);
      }
      else {
        if (((local_190[1] & 0x400U) != 0) ||
           ((reopen_inaccessible_files != '\x01' && ((local_190[1] & 0x800U) != 0)))) {
          inotify_rm_watch(param_2,*(int *)(local_1c0 + 0x50));
          hash_remove(local_1b8,local_1c0);
        }
        recheck(local_1c0,0);
      }
    }
  }
  goto LAB_001056ae;
}

// Function: tail_bytes
long tail_bytes(undefined8 param_1,int param_2,long param_3,long param_4) {
  long lVar1;
  char cVar2;
  int iVar3;
  long lVar4;
  long lVar5;
  __off_t _Var6;
  long in_FS_OFFSET;
  long local_88;
  __off_t local_50;
  long local_48;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  if (presume_input_pipe == '\0') {
    _Var6 = param_4;
    if (from_start == '\0') {
      _Var6 = 0;
    }
    lVar4 = lseek(param_2,_Var6,1);
  }
  else {
    lVar4 = -1;
  }
  local_50 = lVar4;
  if (from_start == '\0') {
    local_48 = -1;
    if (-1 < lVar4) {
      cVar2 = usable_st_size(param_3);
      if (cVar2 == '\0') {
        local_50 = lseek(param_2,-param_4,2);
        if (local_50 < 0) {
          _Var6 = lseek(param_2,0,2);
          local_50 = lVar4;
          if (-1 < _Var6) {
            local_50 = _Var6;
            local_48 = _Var6;
          }
        }
        else {
          local_48 = param_4 + local_50;
        }
      }
      else {
        if ((*(long *)(param_3 + 0x38) < 1) || (0x2000000000000000 < *(ulong *)(param_3 + 0x38))) {
          lVar5 = 0x200;
        }
        else {
          lVar5 = *(long *)(param_3 + 0x38);
        }
        if (lVar5 < *(long *)(param_3 + 0x30)) {
          local_48 = *(long *)(param_3 + 0x30);
        }
      }
    }
    if ((lVar4 < local_48) && (param_4 < local_48 - lVar4)) {
      lVar4 = local_48 - param_4;
    }
    if (lVar4 != local_50) {
      local_50 = xlseek(param_2,lVar4,0,param_1);
    }
    local_88 = param_4;
    if (local_48 < 0) {
      lVar4 = pipe_bytes(param_1,param_2,param_4,local_50);
      goto LAB_001062b9;
    }
  }
  else {
    if ((lVar4 < 0) && (iVar3 = start_bytes(param_1,param_2,param_4), iVar3 < 0)) {
      lVar4 = (long)iVar3;
      goto LAB_001062b9;
    }
    local_88 = -1;
  }
  lVar4 = dump_remainder(0,param_1,param_2,local_88);
  if (local_50 < 0) {
    lVar4 = -1;
  }
  else {
    lVar4 = lVar4 + local_50;
  }
LAB_001062b9:
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return lVar4;
}

// Function: tail_lines
__off_t tail_lines(undefined8 param_1,int param_2,long param_3,long param_4) {
  int iVar1;
  __off_t _Var2;
  long lVar3;
  
  if (from_start == '\0') {
    if ((presume_input_pipe == '\x01') || ((*(uint *)(param_3 + 0x18) & 0xf000) != 0x8000)) {
      _Var2 = -1;
    }
    else {
      _Var2 = lseek(param_2,0,1);
    }
    if (_Var2 < 0) {
      lVar3 = -1;
    }
    else {
      lVar3 = lseek(param_2,0,2);
    }
    if (lVar3 < 0) {
      iVar1 = pipe_lines(param_1,param_2,param_4);
      _Var2 = (__off_t)iVar1;
    }
    else if (_Var2 < lVar3) {
      _Var2 = file_lines(param_1,param_2,param_3,param_4,_Var2,lVar3);
    }
    else if (_Var2 != lVar3) {
      _Var2 = xlseek(param_2,_Var2,0,param_1);
    }
  }
  else if ((param_4 != 0x7fffffffffffffff) || (_Var2 = lseek(param_2,0,2), _Var2 < 0)) {
    iVar1 = start_lines(param_1,param_2,param_4);
    if (iVar1 < 0) {
      _Var2 = (__off_t)iVar1;
    }
    else {
      dump_remainder(0,param_1,param_2,0xffffffffffffffff);
      _Var2 = -1;
    }
  }
  return _Var2;
}

// Function: tail
void tail(undefined8 param_1,undefined4 param_2,undefined8 param_3,undefined8 param_4) {
  code *pcVar1;
  
  if (count_lines == '\0') {
    pcVar1 = tail_bytes;
  }
  else {
    pcVar1 = tail_lines;
  }
  (*pcVar1)(param_1,param_2,param_3,param_4);
  return;
}

// Function: tail_file
char tail_file(undefined8 *param_1,long param_2,undefined8 param_3) {
  bool bVar1;
  char cVar2;
  undefined uVar3;
  int iVar4;
  uint uVar5;
  int *piVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined *puVar9;
  undefined4 uVar10;
  long in_FS_OFFSET;
  char local_107;
  int local_104;
  long local_e0;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if ((forever == '\0') || ((nbpids == 0 && (param_2 < 2)))) {
    bVar1 = false;
  }
  else {
    bVar1 = true;
  }
  cVar2 = streq(*param_1,&DAT_00108d0e);
  if (cVar2 == '\0') {
    if (bVar1) {
      uVar7 = 0x800;
    }
    else {
      uVar7 = 0;
    }
    local_104 = open_safer(*param_1,uVar7);
  }
  else {
    have_read_stdin = 1;
    local_104 = 0;
    xset_binary_mode(0,0);
  }
  *(undefined *)((long)param_1 + 0x42) = 0;
  if (local_104 < 0) {
    if (forever != '\0') {
      *(undefined4 *)((long)param_1 + 0x44) = 0xffffffff;
      piVar6 = __errno_location();
      *(int *)(param_1 + 9) = *piVar6;
      *(bool *)(param_1 + 8) = reopen_inaccessible_files == '\0';
      param_1[4] = 0;
      param_1[5] = 0;
    }
    uVar7 = quotearg_style(4,param_1[1]);
    uVar8 = gettext("cannot open %s for reading");
    piVar6 = __errno_location();
    error(0,*piVar6,uVar8,uVar7);
    local_107 = '\0';
  }
  else {
    if (print_headers != '\0') {
      write_header(param_1[1]);
    }
    iVar4 = fstat(local_104,&local_b8);
    local_107 = -((char)~(byte)((uint)iVar4 >> 0x18) >> 7);
    if (local_107 == '\x01') {
      local_e0 = tail(param_1[1],local_104,&local_b8,param_3);
      local_107 = -2 < local_e0;
      if ((bool)local_107) {
        uVar5 = 0;
      }
      else {
        uVar5 = ~(uint)local_e0;
      }
      *(uint *)(param_1 + 9) = uVar5;
      if (((((local_b8.st_mode & 0xf000) == 0x8000) || ((local_b8.st_mode & 0xf000) == 0x1000)) ||
          ((local_b8.st_mode & 0xf000) == 0xc000)) || ((local_b8.st_mode & 0xf000) == 0x2000)) {
        *(undefined *)((long)param_1 + 0x42) = 1;
      }
      else if (forever != '\0') {
        local_107 = '\0';
        *(undefined4 *)(param_1 + 9) = 0xffffffff;
        if (reopen_inaccessible_files == '\0') {
          puVar9 = (undefined *)gettext("; giving up on this name");
        }
        else {
          puVar9 = &DAT_001080c4;
        }
        uVar7 = quotearg_n_style_colon(0,3,param_1[1]);
        uVar8 = gettext("%s: cannot follow end of this type of file%s");
        error(0,0,uVar8,uVar7,puVar9);
      }
    }
    else {
      piVar6 = __errno_location();
      *(int *)(param_1 + 9) = *piVar6;
      uVar7 = quotearg_style(4,param_1[1]);
      uVar8 = gettext("cannot fstat %s");
      error(0,*(undefined4 *)(param_1 + 9),uVar8,uVar7);
    }
    if (forever == '\0') {
      if (cVar2 != '\x01') {
        iVar4 = close(local_104);
        if (iVar4 < 0) {
          uVar7 = quotearg_style(4,param_1[1]);
          uVar8 = gettext("error reading %s");
          piVar6 = __errno_location();
          error(0,*piVar6,uVar8,uVar7);
          local_107 = '\0';
        }
      }
    }
    else if (local_107 == '\x01') {
      if (cVar2 == '\0') {
        uVar10 = 1;
      }
      else {
        uVar10 = 0xffffffff;
      }
      record_open_fd(param_1,local_104,local_e0,&local_b8,uVar10);
      uVar3 = fremote(local_104,param_1);
      *(undefined *)((long)param_1 + 0x41) = uVar3;
    }
    else {
      *(bool *)(param_1 + 8) = reopen_inaccessible_files == '\0';
      close_fd(local_104,param_1);
      *(undefined4 *)((long)param_1 + 0x44) = 0xffffffff;
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_107;
}

// Function: parse_obsolete_option
undefined8 parse_obsolete_option(int param_1,long param_2,long *param_3) {
  long lVar1;
  undefined auVar2 [16];
  char cVar3;
  bool bVar4;
  int iVar5;
  undefined8 uVar6;
  long lVar7;
  long in_FS_OFFSET;
  undefined local_29;
  undefined local_28;
  long local_20;
  char *local_18;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_28 = 1;
  if (((param_1 != 2) &&
      ((param_1 != 3 ||
       ((**(char **)(param_2 + 0x10) == '-' && (*(char *)(*(long *)(param_2 + 0x10) + 1) != '\0'))))
      )) && ((param_1 < 3 ||
             ((4 < param_1 ||
              (cVar3 = streq(*(undefined8 *)(param_2 + 0x10),&DAT_00109025), cVar3 != '\x01')))))) {
    uVar6 = 0;
    goto LAB_00106d2d;
  }
  iVar5 = posix2_version();
  if ((iVar5 < 0x30db0) || (0x31068 < iVar5)) {
    bVar4 = true;
  }
  else {
    bVar4 = false;
  }
  local_18 = *(char **)(param_2 + 8) + 1;
  cVar3 = **(char **)(param_2 + 8);
  if (cVar3 == '+') {
    if (!bVar4) {
      uVar6 = 0;
      goto LAB_00106d2d;
    }
    local_29 = 1;
  }
  else {
    if (cVar3 != '-') {
      uVar6 = 0;
      goto LAB_00106d2d;
    }
    if ((iVar5 >= 0x30db0) && (local_18[*local_18 == 'c'] == '\0')) {
      uVar6 = 0;
      goto LAB_00106d2d;
    }
    local_29 = 0;
  }
  cVar3 = c_isdigit((int)*local_18);
  if (cVar3 == '\x01') {
    local_20 = 0;
    while (cVar3 = c_isdigit((int)*local_18), cVar3 != '\0') {
      lVar7 = local_20 * 10;
      if ((SEXT816(lVar7) != SEXT816(local_20) * SEXT816(10)) ||
         (local_20 = (*local_18 + -0x30) + lVar7, SCARRY8((long)(*local_18 + -0x30),lVar7))) {
        local_20 = 0x7fffffffffffffff;
      }
      local_18 = local_18 + 1;
    }
  }
  else {
    local_20 = 10;
  }
  cVar3 = *local_18;
  if (cVar3 == 'l') {
LAB_00106cd3:
    local_18 = local_18 + 1;
  }
  else if (cVar3 < 'm') {
    if (cVar3 == 'b') {
      auVar2 = SEXT816(local_20);
      local_20 = local_20 * 0x200;
      if (SEXT816(local_20) != auVar2 * SEXT416(0x200)) {
        local_20 = 0x7fffffffffffffff;
      }
    }
    else if (cVar3 != 'c') goto LAB_00106cd9;
    local_28 = 0;
    goto LAB_00106cd3;
  }
LAB_00106cd9:
  cVar3 = *local_18;
  if (cVar3 == 'f') {
    local_18 = local_18 + 1;
  }
  if (*local_18 == '\0') {
    *param_3 = local_20;
    from_start = local_29;
    count_lines = local_28;
    uVar6 = 1;
    forever = cVar3 == 'f';
  }
  else {
    uVar6 = 0;
  }
LAB_00106d2d:
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar6;
}

// Function: parse_options
void parse_options(undefined4 param_1,undefined8 param_2,undefined8 *param_3,undefined4 *param_4,
                  undefined8 *param_5) {
  __pid_t _Var1;
  int iVar2;
  long lVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  int *piVar8;
  char *pcVar9;
  long in_FS_OFFSET;
  char *local_68;
  double local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  do {
    iVar2 = getopt_long(param_1,param_2,"c:n:fFqs:vz0123456789",long_options,0);
    if (iVar2 == -1) {
      if (reopen_inaccessible_files != '\0') {
        if (forever == '\x01') {
          if (follow_mode == 2) {
            local_40 = gettext("warning: --retry only effective for the initial open");
            error(0,0,local_40);
          }
        }
        else {
          reopen_inaccessible_files = '\0';
          local_48 = gettext("warning: --retry ignored; --retry is useful only when following");
          error(0,0,local_48);
        }
      }
      if ((pids != (__pid_t *)0x0) &&
         ((forever != '\x01' ||
          ((iVar2 = kill(*pids,0), iVar2 < 0 && (piVar8 = __errno_location(), *piVar8 == 0x26))))))
      {
        if (forever == '\0') {
          pcVar9 = "warning: PID ignored; --pid=PID is useful only when following";
        }
        else {
          pcVar9 = "warning: --pid=PID is not supported on this system";
        }
        local_38 = gettext(pcVar9);
        error(0,0,local_38);
        free(pids);
        pids = (__pid_t *)0x0;
      }
      if (local_30 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return;
    }
    if (0x85 < iVar2) goto switchD_00106dfd_caseD_3a;
    if (iVar2 < 0x30) {
      if (iVar2 == -0x83) {
LAB_00107195:
        uVar4 = proper_name_lite("Jim Meyering","Jim Meyering");
        uVar5 = proper_name_lite("Ian Lance Taylor","Ian Lance Taylor");
        uVar6 = proper_name_lite("David MacKenzie","David MacKenzie");
        uVar7 = proper_name_lite("Paul Rubin","Paul Rubin");
        version_etc(_stdout,&DAT_00108bfd,"GNU coreutils",_Version,uVar7,uVar6,uVar5,uVar4,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar2 == -0x82) {
        usage(0);
        goto LAB_00107195;
      }
      goto switchD_00106dfd_caseD_3a;
    }
    switch(iVar2) {
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
      local_50 = gettext("option used in invalid context -- %c");
      error(1,0,local_50,iVar2);
    default:
switchD_00106dfd_caseD_3a:
      usage(1);
      break;
    case 0x46:
      forever = '\x01';
      follow_mode = 1;
      reopen_inaccessible_files = '\x01';
      break;
    case 99:
    case 0x6e:
      count_lines = iVar2 == 0x6e;
      if (*_optarg == '+') {
        from_start = 1;
      }
      else if (*_optarg == '-') {
        _optarg = _optarg + 1;
      }
      if ((bool)count_lines) {
        uVar4 = gettext("invalid number of lines");
      }
      else {
        uVar4 = gettext("invalid number of bytes");
      }
      uVar4 = xnumtoimax(_optarg,10,0,0x7fffffffffffffff,"bkKmMGTPEZYRQ0",uVar4,0,2);
      *param_3 = uVar4;
      break;
    case 0x66:
    case 0x84:
      forever = '\x01';
      if (_optarg == (char *)0x0) {
        follow_mode = 2;
      }
      else {
        lVar3 = __xargmatch_internal
                          ("--follow",_optarg,follow_mode_string,&follow_mode_map,4,_argmatch_die,1)
        ;
        follow_mode = *(int *)((long)&follow_mode_map + lVar3 * 4);
      }
      break;
    case 0x71:
      *param_4 = 2;
      break;
    case 0x73:
      piVar8 = __errno_location();
      *piVar8 = 0;
      local_60 = (double)cl_strtod(_optarg,&local_68);
      if (((_optarg == local_68) || (*local_68 != '\0')) || (local_60 < 0.0)) {
        uVar4 = quote(_optarg);
        local_58 = gettext("invalid number of seconds: %s");
        error(1,0,local_58,uVar4);
      }
      piVar8 = __errno_location();
      uVar4 = dtimespec_bound(local_60,*piVar8);
      *param_5 = uVar4;
      break;
    case 0x76:
      *param_4 = 1;
      break;
    case 0x7a:
      line_end = 0;
      break;
    case 0x80:
      reopen_inaccessible_files = '\x01';
      break;
    case 0x81:
      uVar4 = gettext("invalid maximum number of unchanged stats between opens");
      max_n_unchanged_stats_between_opens =
           xnumtoimax(_optarg,10,0,0x7fffffffffffffff,&DAT_001080c4,uVar4,0,2);
      break;
    case 0x82:
      if (nbpids == pids_alloc) {
        pids = (__pid_t *)xpalloc(pids,&pids_alloc,1,0x7fffffff,4);
      }
      uVar4 = gettext("invalid PID");
      _Var1 = xdectoumax(_optarg,0,0x7fffffff,&DAT_001080c4,uVar4,0);
      lVar3 = (long)nbpids;
      nbpids = nbpids + 1;
      pids[lVar3] = _Var1;
      break;
    case 0x83:
      presume_input_pipe = 1;
      break;
    case 0x85:
      disable_inotify = 1;
    }
  } while( true );
}

// Function: ignore_fifo_and_pipe
undefined ignore_fifo_and_pipe(long param_1,int param_2) {
  char cVar1;
  bool bVar2;
  undefined local_e;
  int local_c;
  
  local_e = 0;
  for (local_c = 0; local_c < param_2; local_c = local_c + 1) {
    cVar1 = streq(*(undefined8 *)(param_1 + (long)local_c * 0x68),&DAT_00108d0e);
    if ((((cVar1 == '\0') || (*(char *)(param_1 + (long)local_c * 0x68 + 0x40) == '\x01')) ||
        (*(int *)(param_1 + (long)local_c * 0x68 + 0x44) < 0)) ||
       ((*(uint *)(param_1 + (long)local_c * 0x68 + 0x30) & 0xf000) != 0x1000)) {
      bVar2 = false;
    }
    else {
      bVar2 = true;
    }
    if (bVar2) {
      *(undefined4 *)(param_1 + (long)local_c * 0x68 + 0x44) = 0xffffffff;
      *(undefined4 *)(param_1 + (long)local_c * 0x68 + 0x48) = 0xffffffff;
      *(undefined *)(param_1 + (long)local_c * 0x68 + 0x40) = 1;
    }
    else {
      local_e = 1;
    }
  }
  return local_e;
}

// Function: main
undefined8 main(int param_1,undefined8 *param_2) {
  byte bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  undefined8 uVar5;
  undefined *puVar6;
  int *piVar7;
  long lVar8;
  long in_FS_OFFSET;
  bool local_150;
  int local_14c;
  int local_148;
  int local_144;
  int local_140;
  int local_13c;
  int local_138;
  int local_134;
  undefined4 local_130;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined4 local_120;
  int local_11c;
  undefined4 local_118;
  int local_114;
  undefined4 local_110;
  int local_10c;
  long local_108;
  undefined8 local_100;
  undefined8 local_f8;
  undefined **local_f0;
  long local_e8;
  undefined8 local_e0;
  undefined8 local_d8;
  undefined8 local_d0;
  undefined8 local_c8;
  undefined *local_c0;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_14c = 0;
  local_150 = true;
  local_108 = 10;
  local_100 = DAT_00109510;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  local_138 = getpagesize();
  page_size = (long)local_138;
  have_read_stdin = '\0';
  count_lines = 1;
  print_headers = 0;
  from_start = '\0';
  forever = '\0';
  line_end = 10;
  bVar1 = parse_obsolete_option(param_1,param_2,&local_108);
  param_1 = param_1 - (uint)bVar1;
  parse_options(param_1,param_2 + bVar1,&local_108,&local_14c,&local_100);
  if (((from_start == '\0') || (local_108 < 1)) || (local_108 == 0x7fffffffffffffff)) {
    iVar4 = 0;
  }
  else {
    iVar4 = 1;
  }
  local_108 = local_108 - iVar4;
  if (_optind < param_1) {
    local_148 = param_1 - _optind;
    local_f0 = (undefined **)(param_2 + bVar1 + _optind);
  }
  else {
    local_148 = 1;
    local_f0 = &dummy_stdin_0;
  }
  bVar2 = false;
  for (local_144 = 0; local_144 < local_148; local_144 = local_144 + 1) {
    cVar3 = streq(local_f0[local_144],&DAT_00108d0e);
    if (cVar3 != '\0') {
      bVar2 = true;
    }
  }
  if ((bVar2) && (follow_mode == 1)) {
    uVar5 = quotearg_style(4,&DAT_00108d0e);
    local_e0 = gettext("cannot follow %s by name");
    local_130 = 1;
    local_12c = 0;
    error(1,0,local_e0,uVar5);
  }
  if ((forever != '\0') && (bVar2)) {
    if ((nbpids == 0) && ((follow_mode == 2 && (local_148 == 1)))) {
      iVar4 = fstat(0,&local_b8);
      if ((iVar4 != 0) || ((local_b8.st_mode & 0xf000) == 0x8000)) goto LAB_0010794d;
      bVar2 = true;
    }
    else {
LAB_0010794d:
      bVar2 = false;
    }
    if (!bVar2) {
      iVar4 = isatty(0);
      if (iVar4 != 0) {
        local_d8 = gettext("warning: following standard input indefinitely is ineffective");
        local_128 = 0;
        local_124 = 0;
        error(0,0,local_d8);
      }
    }
  }
  if (forever != '\x01') {
    if (from_start == '\0') {
      lVar8 = 0;
    }
    else {
      lVar8 = 0x7fffffffffffffff;
    }
    if (lVar8 == local_108) {
      if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return 0;
    }
  }
  local_e8 = xinmalloc((long)local_148,0x68);
  for (local_140 = 0; local_140 < local_148; local_140 = local_140 + 1) {
    *(undefined **)((long)local_140 * 0x68 + local_e8) = local_f0[local_140];
    cVar3 = streq(local_f0[local_140],&DAT_00108d0e);
    if (cVar3 == '\0') {
      puVar6 = local_f0[local_140];
    }
    else {
      puVar6 = (undefined *)gettext("standard input");
    }
    *(undefined **)(local_e8 + (long)local_140 * 0x68 + 8) = puVar6;
  }
  if ((local_14c == 1) || ((local_14c == 0 && (1 < local_148)))) {
    print_headers = 1;
  }
  xset_binary_mode(1,0);
  for (local_13c = 0; local_13c < local_148; local_13c = local_13c + 1) {
    bVar1 = tail_file(local_e8 + (long)local_13c * 0x68,(long)local_148,local_108);
    local_150 = (bVar1 & local_150) != 0;
  }
  if (forever == '\0') goto LAB_00107e6e;
  cVar3 = ignore_fifo_and_pipe(local_e8,local_148);
  if (cVar3 == '\0') goto LAB_00107e6e;
  iVar4 = fstat(1,&local_b8);
  if (iVar4 < 0) {
    uVar5 = gettext("standard output");
    piVar7 = __errno_location();
    local_11c = *piVar7;
    local_120 = 1;
    local_d0 = uVar5;
    error(1,local_11c,uVar5);
  }
  monitor_output = (local_b8.st_mode & 0xf000) == 0x1000;
  if (disable_inotify != '\x01') {
    cVar3 = tailable_stdin(local_e8,local_148);
    if (cVar3 == '\0') {
      cVar3 = any_remote_file(local_e8,local_148);
      if (cVar3 == '\0') {
        cVar3 = any_non_remote_file(local_e8,local_148);
        if (cVar3 == '\x01') {
          cVar3 = any_symlinks(local_e8,local_148);
          if (cVar3 == '\0') {
            cVar3 = any_non_regular_fifo(local_e8,local_148);
            if ((cVar3 == '\0') && ((local_150 == true || (follow_mode != 2)))) goto LAB_00107d60;
          }
        }
      }
    }
    disable_inotify = '\x01';
  }
LAB_00107d60:
  if (disable_inotify != '\x01') {
    local_134 = inotify_init();
    if (-1 < local_134) {
      iVar4 = fflush_unlocked(_stdout);
      if (iVar4 < 0) {
        write_error();
      }
      tail_forever_inotify(local_100,local_134,local_e8,local_148,&local_f8);
      hash_free(local_f8);
      close(local_134);
      piVar7 = __errno_location();
      *piVar7 = 0;
    }
    uVar5 = gettext("inotify cannot be used, reverting to polling");
    piVar7 = __errno_location();
    local_114 = *piVar7;
    local_118 = 0;
    local_c8 = uVar5;
    error(0,local_114,uVar5);
  }
  disable_inotify = '\x01';
  tail_forever(local_100,local_e8,local_148);
LAB_00107e6e:
  if (have_read_stdin != '\0') {
    iVar4 = close(0);
    if (iVar4 < 0) {
      piVar7 = __errno_location();
      local_10c = *piVar7;
      local_110 = 1;
      local_c0 = &DAT_00108d0e;
      error(1,local_10c,&DAT_00108d0e);
    }
  }
                    /* WARNING: Subroutine does not return */
  exit((uint)(local_150 ^ 1));
}

