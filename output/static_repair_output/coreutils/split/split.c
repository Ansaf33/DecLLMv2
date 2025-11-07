// Function: ignorable
undefined4 ignorable(int param_1) {
  undefined4 uVar1;
  
  if ((filter_command == 0) || (param_1 != 0x20)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

// Function: set_suffix_length
void set_suffix_length(long param_1,int param_2) {
  int iVar1;
  long in_FS_OFFSET;
  bool bVar2;
  int local_40;
  long local_30;
  long local_28;
  size_t local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_40 = 0;
  if (numeric_suffix_start != 0) {
    suffix_auto = 0;
  }
  if (((param_2 == 5) || (param_2 == 6)) || (param_2 == 7)) {
    local_30 = param_1 + -1;
    if (((numeric_suffix_start != 0) &&
        (iVar1 = xstrtoimax(numeric_suffix_start,0,10,&local_28,&DAT_001051ac), iVar1 == 0)) &&
       ((local_28 < param_1 &&
        (bVar2 = SCARRY8(local_28,local_30), local_30 = local_28 + local_30, bVar2)))) {
      local_30 = 0x7fffffffffffffff;
    }
    local_20 = strlen(suffix_alphabet);
    do {
      local_40 = local_40 + 1;
      local_30 = local_30 / (long)local_20;
    } while (local_30 != 0);
    suffix_auto = 0;
  }
  if (suffix_length == 0) {
    if (local_40 < 2) {
      local_40 = 2;
    }
    suffix_length = (long)local_40;
  }
  else {
    if (suffix_length < local_40) {
      local_18 = gettext("the suffix length needs to be at least %d");
      error(1,0,local_18,local_40);
    }
    suffix_auto = 0;
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [FILE [PREFIX]]\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Output pieces of FILE to PREFIXaa, PREFIXab, ...;\ndefault size is 1000 lines, and default PREFIX is \'x\'.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    emit_mandatory_arg_note();
    pcVar3 = (char *)gettext(
                            "  -a, --suffix-length=N   generate suffixes of length N (default %d)\n      --additional-suffix=SUFFIX  append an additional SUFFIX to file names\n  -b, --bytes=SIZE        put SIZE bytes per output file\n  -C, --line-bytes=SIZE   put at most SIZE bytes of records per output file\n  -d                      use numeric suffixes starting at 0, not alphabetic\n      --numeric-suffixes[=FROM]  same as -d, but allow setting the start value\n  -x                      use hex suffixes starting at 0, not alphabetic\n      --hex-suffixes[=FROM]  same as -x, but allow setting the start value\n  -e, --elide-empty-files  do not generate empty output files with \'-n\'\n      --filter=COMMAND    write to shell COMMAND; file name is $FILE\n  -l, --lines=NUMBER      put NUMBER lines/records per output file\n  -n, --number=CHUNKS     generate CHUNKS output files; see explanation below\n  -t, --separator=SEP     use SEP instead of newline as the record separator;\n                            \'\\0\' (zero) specifies the NUL character\n  -u, --unbuffered        immediately copy input to output with \'-n r/...\'\n"
                            );
    fprintf(_stdout,pcVar3,2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --verbose           print a diagnostic just before each\n                            output file is opened\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_size_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nCHUNKS may be:\n  N       split into N files based on size of input\n  K/N     output Kth of N to standard output\n  l/N     split into N files without splitting lines/records\n  l/K/N   output Kth of N to standard output without splitting lines/records\n  r/N     like \'l\' but use round robin distribution\n  r/K/N   likewise but only output Kth of N to standard output\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info("split");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: copy_to_tmpfile
size_t copy_to_tmpfile(int param_1,void *param_2,size_t param_3) {
  char cVar1;
  int iVar2;
  int *piVar3;
  size_t sVar4;
  long in_FS_OFFSET;
  bool bVar5;
  FILE *local_28;
  size_t local_20;
  size_t local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  cVar1 = temp_stream(&local_28,0);
  if (cVar1 == '\x01') {
    local_20 = 0;
    do {
      sVar4 = read(param_1,param_2,param_3);
      local_18 = sVar4;
      if ((long)sVar4 < 1) {
        if (-1 < (long)sVar4) {
          iVar2 = fileno_unlocked(local_28);
          iVar2 = dup2(iVar2,param_1);
          sVar4 = (size_t)iVar2;
          local_18 = sVar4;
          if ((-1 < (long)sVar4) && (iVar2 = rpl_fclose(local_28), sVar4 = local_20, iVar2 < 0)) {
            sVar4 = 0xffffffffffffffff;
          }
        }
        goto LAB_00100947;
      }
      sVar4 = fwrite_unlocked(param_2,1,sVar4,local_28);
      if (sVar4 != local_18) {
        sVar4 = 0xffffffffffffffff;
        goto LAB_00100947;
      }
      bVar5 = SCARRY8(local_18,local_20);
      local_20 = local_18 + local_20;
    } while (!bVar5);
    piVar3 = __errno_location();
    *piVar3 = 0x4b;
    sVar4 = 0xffffffffffffffff;
  }
  else {
    sVar4 = 0xffffffffffffffff;
  }
LAB_00100947:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return sVar4;
}

// Function: input_file_size
ssize_t input_file_size(int param_1,long param_2,long param_3,long param_4) {
  long lVar1;
  char cVar2;
  ssize_t sVar3;
  long lVar4;
  void *__ptr;
  int *piVar5;
  __off_t _Var6;
  long in_FS_OFFSET;
  ssize_t local_40;
  __off_t local_38;
  long local_30;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_40 = 0;
  do {
    sVar3 = read(param_1,(void *)(local_40 + param_3),param_4 - local_40);
    if (sVar3 < 1) {
      if (sVar3 < 0) {
        local_40 = sVar3;
      }
      goto LAB_00100b34;
    }
    lVar4 = sVar3 + local_40;
    local_40 = lVar4;
  } while (lVar4 < param_4);
  cVar2 = usable_st_size(param_2);
  if ((((cVar2 != '\0') && (*(long *)(param_2 + 0x30) < lVar4)) ||
      (local_38 = lseek(param_1,0,1), local_38 < 0)) ||
     ((local_38 < lVar4 || (local_30 = lseek(param_1,0,2), local_30 < 0)))) {
    __ptr = (void *)xmalloc(param_4);
    local_40 = copy_to_tmpfile(param_1,__ptr,param_4);
    free(__ptr);
    if (local_40 < 0) goto LAB_00100b34;
    local_38 = 0;
    local_30 = local_40;
  }
  if ((local_30 == 0x7fffffffffffffff) ||
     ((local_40 = lVar4, local_38 < local_30 &&
      (local_40 = (local_30 - local_38) + lVar4, SCARRY8(local_30 - local_38,lVar4))))) {
    piVar5 = __errno_location();
    *piVar5 = 0x4b;
    local_40 = -1;
  }
  else if ((local_38 < local_30) && (_Var6 = lseek(param_1,local_38,0), _Var6 < 0)) {
    local_40 = _Var6;
  }
LAB_00100b34:
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_40;
}

// Function: next_file_name
void next_file_name(void) {
  size_t sVar1;
  long lVar2;
  long lVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  bool bVar5;
  bool local_42;
  size_t local_30;
  long *local_28;
  size_t local_20;
  
  lVar2 = *(long *)(in_FS_OFFSET + 0x28);
  if (outfile == (void *)0x0) {
LAB_00100b79:
    bVar5 = outfile_length_8 != 0;
    if (bVar5) {
      local_42 = SCARRY8(outfile_length_8,2);
      outfile_length_8 = outfile_length_8 + 2;
      suffix_length = suffix_length + 1;
    }
    else {
      outbase_length_7 = strlen(outbase);
      if (additional_suffix == (char *)0x0) {
        addsuf_length_6 = 0;
      }
      else {
        addsuf_length_6 = strlen(additional_suffix);
      }
      local_42 = SCARRY8(suffix_length,outbase_length_7 + addsuf_length_6);
      outfile_length_8 = suffix_length + outbase_length_7 + addsuf_length_6;
    }
    lVar3 = outfile_length_8 + 1;
    if ((byte)(SCARRY8(outfile_length_8,1) | local_42) != 0) {
      xalloc_die();
    }
    outfile = (void *)xirealloc(outfile,lVar3);
    if (bVar5) {
      *(undefined *)((long)outfile + outbase_length_7) = suffix_alphabet[*sufindex_5];
      outbase_length_7 = outbase_length_7 + 1;
    }
    else {
      memcpy(outfile,outbase,outbase_length_7);
    }
    outfile_mid = (void *)((long)outfile + outbase_length_7);
    memset(outfile_mid,(int)(char)*suffix_alphabet,suffix_length);
    if (additional_suffix != (char *)0x0) {
      memcpy((void *)(suffix_length + (long)outfile_mid),additional_suffix,addsuf_length_6);
    }
    *(undefined *)((long)outfile + outfile_length_8) = 0;
    free(sufindex_5);
    sufindex_5 = (long *)xicalloc(suffix_length,8);
    if (numeric_suffix_start != (char *)0x0) {
      if (bVar5) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("! widen",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/split.c"
                      ,0x1a8,"next_file_name");
      }
      local_30 = strlen(numeric_suffix_start);
      memcpy((void *)((long)outfile_mid + (suffix_length - local_30)),numeric_suffix_start,local_30)
      ;
      local_28 = sufindex_5 + suffix_length;
      while (local_30 != 0) {
        local_28 = local_28 + -1;
        *local_28 = (long)(numeric_suffix_start[local_30 - 1] + -0x30);
        local_30 = local_30 - 1;
      }
    }
  }
  else {
    local_20 = suffix_length;
    while (sVar1 = local_20 - 1, local_20 != 0) {
      sufindex_5[sVar1] = sufindex_5[sVar1] + 1;
      if (((suffix_auto != '\0') && (sVar1 == 0)) && (suffix_alphabet[*sufindex_5 + 1] == '\0'))
      goto LAB_00100b79;
      *(undefined *)(sVar1 + (long)outfile_mid) = suffix_alphabet[sufindex_5[sVar1]];
      if (*(char *)(sVar1 + (long)outfile_mid) != '\0') goto LAB_00100fea;
      sufindex_5[sVar1] = 0;
      *(undefined *)(sVar1 + (long)outfile_mid) = suffix_alphabet[sufindex_5[sVar1]];
      local_20 = sVar1;
    }
    uVar4 = gettext("output file suffixes exhausted");
    error(1,0,uVar4);
  }
LAB_00100fea:
  if (lVar2 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: create
int create(char *param_1) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  char *pcVar4;
  int *piVar5;
  undefined8 uVar6;
  long lVar7;
  long in_FS_OFFSET;
  pid_t local_31c;
  int local_318;
  int local_314;
  undefined4 local_310;
  int local_30c;
  undefined4 local_308;
  int local_304;
  undefined4 local_300;
  undefined4 local_2fc;
  undefined4 local_2f8;
  int local_2f4;
  undefined4 local_2f0;
  int local_2ec;
  undefined4 local_2e8;
  int local_2e4;
  undefined4 local_2e0;
  int local_2dc;
  undefined4 local_2d8;
  int local_2d4;
  undefined4 local_2d0;
  int local_2cc;
  undefined4 local_2c8;
  int local_2c4;
  char *local_2c0;
  undefined8 local_2b8;
  undefined8 local_2b0;
  undefined8 local_2a8;
  undefined8 local_2a0;
  undefined8 local_298;
  undefined8 local_290;
  undefined8 local_288;
  undefined8 local_280;
  undefined8 local_278;
  int local_270;
  int local_26c;
  char *local_268;
  undefined *local_260;
  long local_258;
  undefined8 local_250;
  posix_spawn_file_actions_t local_248;
  sigset_t local_1f8;
  undefined local_178 [344];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (filter_command == 0) {
    if (verbose != '\0') {
      uVar3 = quotearg_style(4,param_1);
      pcVar4 = (char *)gettext("creating file %s\n");
      fprintf(_stdout,pcVar4,uVar3);
    }
    local_310 = 0x41;
    local_30c = open_safer(param_1,0xc1,0x1b6);
    local_26c = local_30c;
    if (((local_30c < 0) && (piVar5 = __errno_location(), local_26c = local_30c, *piVar5 == 0x11))
       && (local_30c = open_safer(param_1,local_310,0x1b6), local_26c = local_30c, -1 < local_30c))
    {
      iVar2 = fstat(local_30c,(stat *)local_178);
      if (iVar2 != 0) {
        uVar3 = quotearg_style(4,param_1);
        uVar6 = gettext("failed to stat %s");
        piVar5 = __errno_location();
        local_304 = *piVar5;
        local_308 = 1;
        local_2b8 = uVar6;
        error(1,local_304,uVar6,uVar3);
      }
      cVar1 = psame_inode(in_stat_buf,local_178);
      if (cVar1 != '\0') {
        uVar3 = quotearg_style(4,param_1);
        local_2b0 = gettext("%s would overwrite input; aborting");
        local_300 = 1;
        local_2fc = 0;
        error(1,0,local_2b0,uVar3);
      }
      iVar2 = ftruncate(local_30c,0);
      local_26c = local_30c;
      if ((iVar2 < 0) && ((local_178._24_4_ & 0xf000) == 0x8000)) {
        uVar3 = quotearg_n_style_colon(0,3,param_1);
        uVar6 = gettext("%s: error truncating");
        piVar5 = __errno_location();
        local_2f4 = *piVar5;
        local_2f8 = 1;
        local_2a8 = uVar6;
        error(1,local_2f4,uVar6,uVar3);
        local_26c = local_30c;
      }
    }
  }
  else {
    iVar2 = setenv("FILE",param_1,1);
    if (iVar2 != 0) {
      uVar3 = gettext("failed to set FILE environment variable");
      piVar5 = __errno_location();
      local_2ec = *piVar5;
      local_2f0 = 1;
      local_2a0 = uVar3;
      error(1,local_2ec,uVar3);
    }
    if (verbose != '\0') {
      uVar3 = quotearg_n_style_colon(0,3,param_1);
      pcVar4 = (char *)gettext("executing with FILE=%s\n");
      fprintf(_stdout,pcVar4,uVar3);
    }
    sigemptyset(&local_1f8);
    if (default_SIGPIPE != '\0') {
      sigaddset(&local_1f8,0xd);
    }
    local_318 = posix_spawnattr_init((posix_spawnattr_t *)local_178);
    if (((local_318 != 0) ||
        (local_318 = posix_spawnattr_setflags((posix_spawnattr_t *)local_178,0x44), local_318 != 0))
       || ((local_318 = posix_spawnattr_setsigdefault((posix_spawnattr_t *)local_178,&local_1f8),
           local_318 != 0 || (local_318 = posix_spawn_file_actions_init(&local_248), local_318 != 0)
           ))) {
      local_298 = gettext("posix_spawn initialization failed");
      local_2e8 = 1;
      local_2e4 = local_318;
      error(1,local_318,local_298);
    }
    iVar2 = pipe(&local_270);
    if (iVar2 != 0) {
      uVar3 = gettext("failed to create pipe");
      piVar5 = __errno_location();
      local_2dc = *piVar5;
      local_2e0 = 1;
      local_290 = uVar3;
      error(1,local_2dc,uVar3);
    }
    local_314 = 0;
    while ((local_314 < n_open_pipes &&
           (local_318 = posix_spawn_file_actions_addclose
                                  (&local_248,*(int *)(open_pipes + (long)local_314 * 4)),
           local_318 == 0))) {
      local_314 = local_314 + 1;
    }
    if (((local_318 != 0) ||
        (local_318 = posix_spawn_file_actions_addclose(&local_248,local_26c), local_318 != 0)) ||
       ((local_270 != 0 &&
        ((local_318 = posix_spawn_file_actions_adddup2(&local_248,local_270,0), local_318 != 0 ||
         (local_318 = posix_spawn_file_actions_addclose(&local_248,local_270), local_318 != 0))))))
    {
      local_288 = gettext("posix_spawn setup failed");
      local_2d8 = 1;
      local_2d4 = local_318;
      error(1,local_318,local_288);
    }
    local_2c0 = getenv("SHELL");
    if (local_2c0 == (char *)0x0) {
      local_2c0 = "/bin/sh";
    }
    local_268 = (char *)last_component(local_2c0);
    local_260 = &DAT_00105bcf;
    local_258 = filter_command;
    local_250 = 0;
    local_318 = posix_spawn(&local_31c,local_2c0,&local_248,(posix_spawnattr_t *)local_178,
                            &local_268,_environ);
    lVar7 = filter_command;
    if (local_318 != 0) {
      uVar3 = gettext("failed to run command: \"%s -c %s\"");
      piVar5 = __errno_location();
      local_2cc = *piVar5;
      local_2d0 = 1;
      local_280 = uVar3;
      error(1,local_2cc,uVar3,local_2c0,lVar7);
    }
    posix_spawnattr_destroy((posix_spawnattr_t *)local_178);
    posix_spawn_file_actions_destroy(&local_248);
    iVar2 = close(local_270);
    if (iVar2 != 0) {
      uVar3 = gettext("failed to close input pipe");
      piVar5 = __errno_location();
      local_2c4 = *piVar5;
      local_2c8 = 1;
      local_278 = uVar3;
      error(1,local_2c4,uVar3);
    }
    filter_pid = local_31c;
    if (n_open_pipes == open_pipes_alloc) {
      open_pipes = xpalloc(open_pipes,&open_pipes_alloc,1,0x7fffffff,4);
    }
    lVar7 = (long)n_open_pipes;
    n_open_pipes = n_open_pipes + 1;
    *(int *)(open_pipes + lVar7 * 4) = local_26c;
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_26c;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: closeout
void closeout(long param_1,int param_2,int param_3,undefined8 param_4) {
  char cVar1;
  int iVar2;
  __pid_t _Var3;
  int *piVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  long in_FS_OFFSET;
  uint local_b0;
  int local_ac;
  uint local_a8;
  uint local_a4;
  uint local_a0;
  int local_9c;
  undefined4 local_98;
  int local_94;
  undefined4 local_90;
  int local_8c;
  undefined4 local_88;
  int local_84;
  int local_80;
  undefined4 local_7c;
  uint local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined *local_68;
  undefined *local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  char local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (param_1 != 0) {
    iVar2 = rpl_fclose(param_1);
    if (iVar2 != 0) {
      piVar4 = __errno_location();
      cVar1 = ignorable(*piVar4);
      if (cVar1 != '\x01') {
        uVar5 = quotearg_n_style_colon(0,3,param_4);
        piVar4 = __errno_location();
        local_94 = *piVar4;
        local_98 = 1;
        local_68 = &DAT_00105c15;
        error(1,local_94,&DAT_00105c15,uVar5);
      }
    }
  }
  if (-1 < param_2) {
    if (param_1 == 0) {
      iVar2 = close(param_2);
      if (iVar2 < 0) {
        uVar5 = quotearg_n_style_colon(0,3,param_4);
        piVar4 = __errno_location();
        local_8c = *piVar4;
        local_90 = 1;
        local_60 = &DAT_00105c15;
        error(1,local_8c,&DAT_00105c15,uVar5);
      }
    }
    for (local_ac = 0; local_ac < n_open_pipes; local_ac = local_ac + 1) {
      if (param_2 == *(int *)(open_pipes + (long)local_ac * 4)) {
        n_open_pipes = n_open_pipes + -1;
        *(undefined4 *)(open_pipes + (long)local_ac * 4) =
             *(undefined4 *)(open_pipes + (long)n_open_pipes * 4);
        break;
      }
    }
  }
  if (0 < param_3) {
    _Var3 = waitpid(param_3,(int *)&local_b0,0);
    if (_Var3 < 0) {
      uVar5 = gettext("waiting for child process");
      piVar4 = __errno_location();
      local_84 = *piVar4;
      local_88 = 1;
      local_58 = uVar5;
      error(1,local_84,uVar5);
    }
    uVar5 = filter_command;
    if ((char)(((byte)local_b0 & 0x7f) + 1) >> 1 < '\x01') {
      if ((local_b0 & 0x7f) == 0) {
        local_a8 = (int)local_b0 >> 8 & 0xff;
        if (local_a8 != 0) {
          local_a4 = local_a8;
          uVar6 = quotearg_n_style_colon(0,3,param_4);
          local_48 = gettext("with FILE=%s, exit %d from command: %s");
          local_78 = local_a4;
          local_74 = 0;
          error(local_a4,0,local_48,uVar6,local_a8,uVar5);
        }
      }
      else {
        local_40 = gettext("unknown status from command (0x%X)");
        local_70 = 1;
        local_6c = 0;
        error(1,0,local_40,local_b0);
      }
    }
    else {
      local_a0 = local_b0 & 0x7f;
      if (local_a0 != 0xd) {
        iVar2 = sig2str(local_a0,local_38);
        if (iVar2 != 0) {
          sprintf(local_38,"%d",(ulong)local_a0);
        }
        uVar5 = filter_command;
        local_9c = local_a0 + 0x80;
        uVar6 = quotearg_n_style_colon(0,3,param_4);
        local_50 = gettext("with FILE=%s, signal %s from command: %s");
        local_80 = local_9c;
        local_7c = 0;
        error(local_9c,0,local_50,uVar6,local_38,uVar5);
      }
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: cwrite
undefined8 cwrite(char param_1,long param_2,long param_3) {
  char cVar1;
  undefined8 uVar2;
  int *piVar3;
  long lVar4;
  
  if (param_1 != '\0') {
    if (((param_2 == 0) && (param_3 == 0)) && (elide_empty_files != '\0')) {
      return 1;
    }
    closeout(0,output_desc,filter_pid,outfile);
    next_file_name();
    output_desc = create(outfile);
    if (output_desc < 0) {
      uVar2 = quotearg_n_style_colon(0,3,outfile);
      piVar3 = __errno_location();
      error(1,*piVar3,&DAT_00105c15,uVar2);
    }
  }
  lVar4 = full_write(output_desc,param_2,param_3);
  if (param_3 == lVar4) {
    uVar2 = 1;
  }
  else {
    piVar3 = __errno_location();
    cVar1 = ignorable(*piVar3);
    if (cVar1 != '\x01') {
      uVar2 = quotearg_n_style_colon(0,3,outfile);
      piVar3 = __errno_location();
      error(1,*piVar3,&DAT_00105c15,uVar2);
    }
    uVar2 = 0;
  }
  return uVar2;
}

// Function: bytes_split
void bytes_split(long param_1,long param_2,void *param_3,size_t param_4,long param_5,long param_6) {
  long lVar1;
  __off_t _Var2;
  undefined8 uVar3;
  int *piVar4;
  bool bVar5;
  long local_80;
  byte local_4b;
  char local_4a;
  long local_40;
  long local_38;
  long local_30;
  void *local_28;
  
  local_4b = 1;
  local_4a = '\x01';
  local_40 = 0;
  local_38 = param_1 + (ulong)(0 < param_2);
  bVar5 = local_38 == 0;
  local_80 = param_5;
  while (lVar1 = local_80, !bVar5) {
    local_28 = param_3;
    if (local_80 < 0) {
      if ((local_4a != '\x01') && (_Var2 = lseek(0,local_38,1), -1 < _Var2)) {
        local_38 = param_1 + (ulong)(local_40 + 1 < param_2);
        local_4b = 1;
      }
      local_30 = read(0,param_3,param_4);
      if (local_30 < 0) {
        uVar3 = quotearg_n_style_colon(0,3,infile);
        piVar4 = __errno_location();
        error(1,*piVar4,&DAT_00105c15,uVar3);
      }
      bVar5 = local_30 == 0;
    }
    else {
      local_30 = local_80;
      local_80 = -1;
      bVar5 = lVar1 < (long)param_4;
    }
    while( true ) {
      if ((local_38 < 1) || (local_30 < local_38)) goto LAB_00102027;
      if ((local_4a != '\0') || (local_4b != 0)) {
        local_4a = cwrite(local_4b,local_28,local_38);
      }
      local_40 = local_40 + (ulong)local_4b;
      if ((param_6 == 0) || (local_40 < param_6)) {
        local_4b = 1;
      }
      else {
        local_4b = 0;
      }
      if ((local_4a != '\x01') && (local_4b != 1)) break;
      local_28 = (void *)((long)local_28 + local_38);
      local_30 = local_30 - local_38;
      local_38 = param_1 + (ulong)(local_40 < param_2);
    }
    local_30 = 0;
    bVar5 = true;
LAB_00102027:
    if (0 < local_30) {
      if ((local_4a != '\0') || (local_4b != 0)) {
        local_4a = cwrite(local_4b,local_28,local_30);
      }
      local_40 = local_40 + (ulong)local_4b;
      local_4b = 0;
      if ((local_4a != '\x01') && (local_40 == param_6)) break;
      local_38 = local_38 - local_30;
    }
  }
  while (local_40 < param_6) {
    cwrite(1,0,0);
    local_40 = local_40 + 1;
  }
  return;
}

// Function: lines_split
void lines_split(long param_1,undefined *param_2,size_t param_3) {
  ssize_t sVar1;
  undefined8 uVar2;
  int *piVar3;
  undefined *puVar4;
  undefined local_59;
  undefined *local_50;
  undefined *local_48;
  long local_40;
  
  local_59 = 1;
  local_40 = 0;
  do {
    sVar1 = read(0,param_2,param_3);
    if (sVar1 < 0) {
      uVar2 = quotearg_n_style_colon(0,3,infile);
      piVar3 = __errno_location();
      error(1,*piVar3,&DAT_00105c15,uVar2);
    }
    puVar4 = param_2 + sVar1;
    *puVar4 = (char)eolchar;
    local_50 = param_2;
    local_48 = param_2;
    while( true ) {
      local_50 = (undefined *)rawmemchr(local_50,eolchar);
      if (local_50 == puVar4) break;
      local_50 = local_50 + 1;
      local_40 = local_40 + 1;
      if (param_1 <= local_40) {
        cwrite(local_59,local_48,(long)local_50 - (long)local_48);
        local_59 = 1;
        local_40 = 0;
        local_48 = local_50;
      }
    }
    if (puVar4 != local_48) {
      cwrite(local_59,local_48,(long)puVar4 - (long)local_48);
      local_59 = 0;
    }
  } while (sVar1 != 0);
  return;
}

// Function: line_bytes_split
void line_bytes_split(long param_1,void *param_2,size_t param_3) {
  bool bVar1;
  undefined8 uVar2;
  int *piVar3;
  long in_FS_OFFSET;
  long local_90;
  long local_88;
  long local_80;
  void *local_78;
  size_t local_70;
  void *local_68;
  size_t local_60;
  long local_58;
  void *local_50;
  size_t local_48;
  long local_40;
  size_t local_38;
  size_t local_30;
  undefined *local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_88 = 0;
  local_80 = 0;
  local_78 = (void *)0x0;
  local_90 = 0;
  bVar1 = false;
  do {
    local_48 = read(0,param_2,param_3);
    if ((long)local_48 < 0) {
      uVar2 = quotearg_n_style_colon(0,3,infile);
      piVar3 = __errno_location();
      local_28 = &DAT_00105c15;
      error(1,*piVar3,&DAT_00105c15,uVar2);
    }
    local_70 = local_48;
    local_68 = param_2;
    while (local_70 != 0) {
      local_60 = 0;
      local_58 = 0;
      if ((long)local_70 < (param_1 - local_88) - local_80) {
        local_50 = memrchr(local_68,eolchar,local_70);
      }
      else {
        local_60 = (param_1 - local_88) - local_80;
        local_58 = (long)local_68 + (local_60 - 1);
        local_50 = memrchr(local_68,eolchar,local_60);
      }
      if ((local_80 != 0) && ((local_50 != (void *)0x0 || (local_88 == 0)))) {
        cwrite(local_88 == 0,local_78,local_80);
        local_88 = local_88 + local_80;
        local_80 = 0;
      }
      if (local_50 != (void *)0x0) {
        bVar1 = true;
        local_40 = (long)local_50 + (1 - (long)local_68);
        cwrite(local_88 == 0,local_68,local_40);
        local_88 = local_88 + local_40;
        local_70 = local_70 - local_40;
        local_68 = (void *)((long)local_68 + local_40);
        if (local_58 != 0) {
          local_60 = local_60 - local_40;
        }
      }
      if ((local_70 != 0) && (!bVar1)) {
        local_38 = local_70;
        if (local_58 != 0) {
          local_38 = local_60;
        }
        cwrite(local_88 == 0,local_68,local_38);
        local_88 = local_88 + local_38;
        local_70 = local_70 - local_38;
        local_68 = (void *)((long)local_68 + local_38);
        if (local_58 != 0) {
          local_60 = local_60 - local_38;
        }
      }
      if (((local_58 != 0) && (local_60 != 0)) || ((local_58 == 0 && (local_70 != 0)))) {
        local_30 = local_70;
        if (local_58 != 0) {
          local_30 = local_60;
        }
        if (local_90 - local_80 < (long)local_30) {
          local_78 = (void *)xpalloc(local_78,&local_90,local_30 - (local_90 - local_80),
                                     0xffffffffffffffff,1);
        }
        memcpy((void *)(local_80 + (long)local_78),local_68,local_30);
        local_80 = local_80 + local_30;
        local_70 = local_70 - local_30;
        local_68 = (void *)((long)local_68 + local_30);
      }
      if (local_58 != 0) {
        local_88 = 0;
        bVar1 = false;
      }
    }
  } while (local_48 != 0);
  if (local_80 != 0) {
    cwrite(local_88 == 0,local_78,local_80);
  }
  free(local_78);
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: lines_chunk_split
void lines_chunk_split(long param_1,long param_2,void *param_3,size_t param_4,long param_5,
                      long param_6) {
  bool bVar1;
  bool bVar2;
  long lVar3;
  __off_t _Var4;
  size_t __nbytes;
  undefined8 uVar5;
  int *piVar6;
  void *pvVar7;
  long lVar8;
  void *pvVar9;
  long lVar10;
  long lVar11;
  bool bVar12;
  long local_d0;
  bool local_9b;
  long local_88;
  long local_80;
  long local_78;
  void *local_70;
  long local_68;
  void *local_60;
  
  if ((param_2 == 0) || (param_2 < param_1)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("n && k <= n",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/split.c",
                  0x387,"lines_chunk_split");
  }
  lVar11 = param_6 % param_2;
  lVar3 = param_6 / param_2;
  local_88 = 1;
  local_80 = lVar3 + (ulong)(0 < lVar11);
  local_78 = 0;
  local_9b = true;
  bVar2 = false;
  local_d0 = param_5;
  if ((1 < param_1) && (0 < param_6)) {
    local_80 = lVar11;
    if (param_1 <= lVar11) {
      local_80 = param_1 + -1;
    }
    local_80 = local_80 + (param_1 + -1) * lVar3;
    local_78 = local_80 + -1;
    if (local_78 < param_5) {
      memmove(param_3,(void *)(local_78 + (long)param_3),param_5 - local_78);
      local_d0 = param_5 - local_78;
    }
    else {
      if ((param_5 < local_78) && (_Var4 = lseek(0,local_78 - param_5,1), _Var4 < 0)) {
        uVar5 = quotearg_n_style_colon(0,3,infile);
        piVar6 = __errno_location();
        error(1,*piVar6,&DAT_00105c15,uVar5);
      }
      local_d0 = -1;
    }
    local_88 = param_1 + -1;
  }
  while (local_78 < param_6) {
    if (local_d0 < 0) {
      __nbytes = param_4;
      if (param_6 - local_78 <= (long)param_4) {
        __nbytes = param_6 - local_78;
      }
      local_68 = read(0,param_3,__nbytes);
      if (local_68 < 0) {
        uVar5 = quotearg_n_style_colon(0,3,infile);
        piVar6 = __errno_location();
        error(1,*piVar6,&DAT_00105c15,uVar5);
      }
    }
    else {
      local_68 = local_d0;
      local_d0 = -1;
    }
    if (local_68 == 0) break;
    bVar2 = false;
    pvVar7 = (void *)((long)param_3 + local_68);
    local_70 = param_3;
LAB_00102ac6:
    if (local_70 != pvVar7) {
      lVar8 = (local_80 + -1) - local_78;
      if (lVar8 < 0) {
        lVar8 = 0;
      }
      if (local_68 <= lVar8) {
        lVar8 = local_68;
      }
      pvVar9 = memchr((void *)(lVar8 + (long)local_70),eolchar,local_68 - lVar8);
      bVar12 = pvVar9 != (void *)0x0;
      local_60 = pvVar7;
      if (bVar12) {
        local_60 = (void *)((long)pvVar9 + 1);
      }
      lVar8 = (long)local_60 - (long)local_70;
      if (param_1 == local_88) {
        lVar10 = full_write(1,local_70,lVar8);
        if (lVar8 != lVar10) {
          write_error();
        }
      }
      else if (param_1 == 0) {
        cwrite(local_9b,local_70,lVar8);
      }
      local_78 = local_78 + lVar8;
      local_70 = (void *)((long)local_70 + lVar8);
      local_68 = local_68 - lVar8;
      bVar1 = bVar12;
      while( true ) {
        local_9b = bVar12;
        if ((!bVar1) && (local_78 < local_80)) goto LAB_00102ac6;
        if ((!bVar1) && (local_70 == pvVar7)) break;
        if (param_1 == local_88) {
          return;
        }
        local_80 = local_80 + lVar3 + (ulong)(local_88 < lVar11);
        local_88 = local_88 + 1;
        if (local_78 < local_80) {
          bVar1 = false;
        }
        else if (param_1 == 0) {
          cwrite(1,0,0);
        }
      }
      bVar2 = true;
      goto LAB_00102ac6;
    }
  }
  if (bVar2) {
    local_88 = local_88 + 1;
  }
  if (param_1 == 0) {
    while (local_88 <= param_2) {
      cwrite(1,0,0);
      local_88 = local_88 + 1;
    }
  }
  return;
}

// Function: bytes_chunk_extract
void bytes_chunk_extract(long param_1,long param_2,void *param_3,size_t param_4,long param_5,
                        long param_6) {
  char cVar1;
  __off_t _Var2;
  long lVar3;
  long lVar4;
  int *piVar5;
  undefined8 uVar6;
  long local_90;
  long local_48;
  ssize_t local_40;
  
  if ((0 < param_1) && (param_1 <= param_2)) {
    if (param_6 % param_2 < param_1) {
      local_48 = param_6 % param_2;
    }
    else {
      local_48 = param_1 + -1;
    }
    local_48 = local_48 + (param_1 + -1) * (param_6 / param_2);
    if (param_1 != param_2) {
      lVar3 = param_1;
      if (param_6 % param_2 <= param_1) {
        lVar3 = param_6 % param_2;
      }
      param_6 = lVar3 + (param_6 / param_2) * param_1;
    }
    if (local_48 < param_5) {
      memmove(param_3,(void *)(local_48 + (long)param_3),param_5 - local_48);
      local_90 = param_5 - local_48;
    }
    else {
      if ((param_5 < local_48) && (_Var2 = lseek(0,local_48 - param_5,1), _Var2 < 0)) {
        uVar6 = quotearg_n_style_colon(0,3,infile);
        piVar5 = __errno_location();
        error(1,*piVar5,&DAT_00105c15,uVar6);
      }
      local_90 = -1;
    }
    while( true ) {
      if (param_6 <= local_48) {
        return;
      }
      if (local_90 < 0) {
        local_40 = read(0,param_3,param_4);
        if (local_40 < 0) {
          uVar6 = quotearg_n_style_colon(0,3,infile);
          piVar5 = __errno_location();
          error(1,*piVar5,&DAT_00105c15,uVar6);
        }
      }
      else {
        local_40 = local_90;
        local_90 = -1;
      }
      if (local_40 == 0) break;
      lVar3 = param_6 - local_48;
      if (local_40 <= param_6 - local_48) {
        lVar3 = local_40;
      }
      lVar4 = full_write(1,param_3,lVar3);
      if (lVar3 != lVar4) {
        piVar5 = __errno_location();
        cVar1 = ignorable(*piVar5);
        if (cVar1 != '\x01') {
          uVar6 = quotearg_n_style_colon(0,3,&DAT_00105ccf);
          piVar5 = __errno_location();
          error(1,*piVar5,&DAT_00105c15,uVar6);
        }
      }
      local_48 = local_48 + lVar3;
    }
    return;
  }
                    /* WARNING: Subroutine does not return */
  __assert_fail("0 < k && k <= n",
                "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/split.c",
                0x408,"bytes_chunk_extract");
}

// Function: ofile_open
undefined ofile_open(long param_1,long param_2,long param_3) {
  long lVar1;
  int iVar2;
  int *piVar3;
  undefined8 uVar4;
  FILE *pFVar5;
  undefined local_6d;
  int local_6c;
  long local_48;
  
  local_6d = 0;
  if (*(int *)(param_1 + param_2 * 0x20 + 8) < 0) {
    local_48 = param_3;
    if (param_2 != 0) {
      local_48 = param_2;
    }
    local_48 = local_48 + -1;
    while( true ) {
      if (*(int *)(param_1 + param_2 * 0x20 + 8) == -1) {
        local_6c = create(*(undefined8 *)(param_1 + param_2 * 0x20));
      }
      else {
        local_6c = open_safer(*(undefined8 *)(param_1 + param_2 * 0x20),0xc01);
      }
      if (-1 < local_6c) break;
      piVar3 = __errno_location();
      if ((*piVar3 != 0x18) && (piVar3 = __errno_location(), *piVar3 != 0x17)) {
        uVar4 = quotearg_n_style_colon(0,3,*(undefined8 *)(param_1 + param_2 * 0x20));
        piVar3 = __errno_location();
        error(1,*piVar3,&DAT_00105c15,uVar4);
      }
      local_6d = 1;
      while (*(int *)(param_1 + local_48 * 0x20 + 8) < 0) {
        lVar1 = param_3;
        if (local_48 != 0) {
          lVar1 = local_48;
        }
        local_48 = lVar1 + -1;
        if (local_48 == param_2) {
          uVar4 = quotearg_n_style_colon(0,3,*(undefined8 *)(param_1 + param_2 * 0x20));
          piVar3 = __errno_location();
          error(1,*piVar3,&DAT_00105c15,uVar4);
        }
      }
      iVar2 = rpl_fclose(*(undefined8 *)(param_1 + local_48 * 0x20 + 0x10));
      if (iVar2 != 0) {
        uVar4 = quotearg_n_style_colon(0,3,*(undefined8 *)(param_1 + local_48 * 0x20));
        piVar3 = __errno_location();
        error(1,*piVar3,&DAT_00105c15,uVar4);
      }
      *(undefined8 *)(param_1 + local_48 * 0x20 + 0x10) = 0;
      *(undefined4 *)(param_1 + local_48 * 0x20 + 8) = 0xfffffffe;
    }
    *(int *)(param_2 * 0x20 + param_1 + 8) = local_6c;
    pFVar5 = fdopen(local_6c,"a");
    if (pFVar5 == (FILE *)0x0) {
      uVar4 = quotearg_n_style_colon(0,3,*(undefined8 *)(param_1 + param_2 * 0x20));
      piVar3 = __errno_location();
      error(1,*piVar3,&DAT_00105c15,uVar4);
    }
    *(FILE **)(param_2 * 0x20 + param_1 + 0x10) = pFVar5;
    *(undefined4 *)(param_2 * 0x20 + param_1 + 0x18) = filter_pid;
    filter_pid = 0;
  }
  return local_6d;
}

// Function: lines_rr
void lines_rr(long param_1,long param_2,void *param_3,size_t param_4,long *param_5) {
  bool bVar1;
  bool bVar2;
  byte bVar3;
  char cVar4;
  int iVar5;
  long lVar6;
  ssize_t sVar7;
  undefined8 uVar8;
  int *piVar9;
  void *pvVar10;
  void *pvVar11;
  size_t sVar12;
  size_t sVar13;
  bool bVar14;
  byte local_a2;
  long local_80;
  long local_78;
  long local_70;
  void *local_68;
  void *local_60;
  
  bVar2 = false;
  bVar1 = false;
  local_78 = 0;
  if (param_1 == 0) {
    lVar6 = xinmalloc(param_2,0x20);
    *param_5 = lVar6;
    local_78 = *param_5;
    for (local_80 = 0; local_80 < param_2; local_80 = local_80 + 1) {
      next_file_name();
      uVar8 = xstrdup(outfile);
      *(undefined8 *)(local_80 * 0x20 + local_78) = uVar8;
      *(undefined4 *)(local_78 + local_80 * 0x20 + 8) = 0xffffffff;
      *(undefined8 *)(local_78 + local_80 * 0x20 + 0x10) = 0;
      *(undefined4 *)(local_78 + local_80 * 0x20 + 0x18) = 0;
    }
    local_80 = 0;
    local_a2 = 0;
  }
  else {
    local_70 = 1;
  }
  while( true ) {
    sVar7 = read(0,param_3,param_4);
    if (sVar7 < 0) {
      uVar8 = quotearg_n_style_colon(0,3,infile);
      piVar9 = __errno_location();
      error(1,*piVar9,&DAT_00105c15,uVar8);
    }
    if (sVar7 == 0) break;
    pvVar10 = (void *)((long)param_3 + sVar7);
    local_68 = param_3;
    while (local_68 != pvVar10) {
      pvVar11 = memchr(local_68,eolchar,(long)pvVar10 - (long)local_68);
      bVar14 = pvVar11 != (void *)0x0;
      local_60 = pvVar10;
      if (bVar14) {
        local_60 = (void *)((long)pvVar11 + 1);
      }
      sVar12 = (long)local_60 - (long)local_68;
      if (param_1 == 0) {
        bVar3 = ofile_open(local_78,local_80,param_2);
        local_a2 = (bVar3 | local_a2) != 0;
        if (unbuffered == '\0') {
LAB_001035d3:
          sVar12 = fwrite_unlocked(local_68,sVar12,1,*(FILE **)(local_78 + local_80 * 0x20 + 0x10));
          if (sVar12 != 1) {
            piVar9 = __errno_location();
            cVar4 = ignorable(*piVar9);
            if (cVar4 != '\x01') {
              uVar8 = quotearg_n_style_colon(0,3,*(undefined8 *)(local_78 + local_80 * 0x20));
              piVar9 = __errno_location();
              error(1,*piVar9,&DAT_00105c15,uVar8);
            }
          }
        }
        else {
          sVar13 = full_write(*(undefined4 *)(local_78 + local_80 * 0x20 + 8),local_68,sVar12);
          if (sVar12 != sVar13) {
            piVar9 = __errno_location();
            cVar4 = ignorable(*piVar9);
            if (cVar4 != '\x01') {
              uVar8 = quotearg_n_style_colon(0,3,*(undefined8 *)(local_78 + local_80 * 0x20));
              piVar9 = __errno_location();
              error(1,*piVar9,&DAT_00105c15,uVar8);
              goto LAB_001035d3;
            }
          }
        }
        piVar9 = __errno_location();
        cVar4 = ignorable(*piVar9);
        if (cVar4 != '\x01') {
          bVar1 = true;
        }
        if ((bool)local_a2) {
          iVar5 = rpl_fclose(*(undefined8 *)(local_78 + local_80 * 0x20 + 0x10));
          if (iVar5 != 0) {
            uVar8 = quotearg_n_style_colon(0,3,*(undefined8 *)(local_78 + local_80 * 0x20));
            piVar9 = __errno_location();
            error(1,*piVar9,&DAT_00105c15,uVar8);
          }
          *(undefined8 *)(local_78 + local_80 * 0x20 + 0x10) = 0;
          *(undefined4 *)(local_78 + local_80 * 0x20 + 8) = 0xfffffffe;
        }
        if ((bVar14) && (local_80 = local_80 + 1, local_80 == param_2)) {
          bVar2 = true;
          if (!bVar1) goto LAB_001037c8;
          bVar1 = false;
          local_80 = 0;
        }
      }
      else {
        if ((local_70 == param_1) && (unbuffered != '\0')) {
          sVar13 = full_write(1,local_68,sVar12);
          if (sVar12 != sVar13) {
            write_error();
          }
        }
        else if ((local_70 == param_1) &&
                (sVar12 = fwrite_unlocked(local_68,sVar12,1,_stdout), sVar12 != 1)) {
          write_error();
        }
        if (bVar14) {
          if (local_70 == param_2) {
            local_70 = 1;
          }
          else {
            local_70 = local_70 + 1;
          }
        }
      }
      local_68 = local_60;
    }
  }
LAB_001037c8:
  if (param_1 == 0) {
    lVar6 = local_80;
    if (bVar2) {
      lVar6 = param_2;
    }
    for (local_80 = 0; local_80 < param_2; local_80 = local_80 + 1) {
      if ((lVar6 <= local_80) && (elide_empty_files != '\x01')) {
        ofile_open(local_78,local_80,param_2);
      }
      if (-1 < *(int *)(local_78 + local_80 * 0x20 + 8)) {
        closeout(*(undefined8 *)(local_78 + local_80 * 0x20 + 0x10),
                 *(undefined4 *)(local_78 + local_80 * 0x20 + 8),
                 *(undefined4 *)(local_78 + local_80 * 0x20 + 0x18),
                 *(undefined8 *)(local_78 + local_80 * 0x20));
      }
      *(undefined4 *)(local_78 + local_80 * 0x20 + 8) = 0xfffffffe;
    }
  }
  return;
}

// Function: strtoint_die
long strtoint_die(undefined8 param_1,undefined8 param_2) {
  uint uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  uint *puVar5;
  undefined8 extraout_RDX;
  ulong uVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  long lStack_50;
  long lStack_48;
  undefined *puStack_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined4 local_28;
  uint local_24;
  char *local_20;
  
  puStack_40 = (undefined *)0x103912;
  local_38 = param_2;
  local_30 = param_1;
  uVar2 = quote(param_2);
  puStack_40 = (undefined *)0x103921;
  uVar3 = gettext(local_30);
  puStack_40 = (undefined *)0x103929;
  piVar4 = __errno_location();
  if (*piVar4 == 0x16) {
    local_24 = 0;
  }
  else {
    puStack_40 = (undefined *)0x103935;
    puVar5 = (uint *)__errno_location();
    local_24 = *puVar5;
  }
  local_28 = 1;
  local_20 = "%s: %s";
  uVar6 = (ulong)local_24;
  uVar7 = 1;
  puStack_40 = (undefined *)0x10396f;
  error(1,uVar6,"%s: %s",uVar3,uVar2);
  lStack_48 = *(long *)(in_FS_OFFSET + 0x28);
  puStack_40 = &stack0xfffffffffffffff8;
  uVar1 = xstrtoimax(uVar7,0,10,&lStack_50,uVar6);
  if ((1 < uVar1) || (lStack_50 < 1)) {
    strtoint_die(extraout_RDX,uVar7);
  }
  if (lStack_48 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return lStack_50;
}

// Function: parse_n_units
long parse_n_units(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  uint uVar1;
  long in_FS_OFFSET;
  long local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  uVar1 = xstrtoimax(param_1,0,10,&local_18,param_2);
  if ((1 < uVar1) || (local_18 < 1)) {
    strtoint_die(param_3,param_1);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_18;
}

// Function: parse_chunk
void parse_chunk(long *param_1,long *param_2,long param_3) {
  uint uVar1;
  long lVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  char *local_30;
  char *local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  uVar1 = xstrtoimax(param_3,&local_30,10,param_2,&DAT_001051ac);
  if ((uVar1 == 2) && (*local_30 == '/')) {
    *param_1 = *param_2;
    lVar2 = parse_n_units(local_30 + 1,&DAT_001051ac,"invalid number of chunks");
    *param_2 = lVar2;
    if ((0 < *param_1) && (*param_1 <= *param_2)) goto code_r0x00103b42;
    uVar3 = quote_mem(param_3,(long)local_30 - param_3);
    uVar4 = gettext("invalid chunk number");
    local_28 = "%s: %s";
    error(1,0,"%s: %s",uVar4,uVar3);
  }
  if ((1 < uVar1) || (*param_2 < 1)) {
    strtoint_die("invalid number of chunks",param_3);
  }
code_r0x00103b42:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  ushort *puVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  ushort **ppuVar7;
  long lVar8;
  size_t sVar9;
  undefined8 uVar10;
  undefined8 uVar11;
  size_t sVar12;
  int *piVar13;
  __sighandler_t p_Var14;
  long in_FS_OFFSET;
  char local_195;
  int local_194;
  int local_190;
  long local_f0;
  long local_e8 [2];
  long local_d8;
  long local_d0;
  long local_c8;
  long local_c0;
  undefined8 local_b8;
  undefined8 local_b0;
  undefined8 local_a8;
  undefined8 local_a0;
  undefined8 local_98;
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
  undefined *local_38;
  undefined8 local_30;
  undefined *local_28;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  local_194 = 0;
  local_d8 = 0;
  iVar4 = getpagesize();
  local_c0 = (long)iVar4;
  local_f0 = 0;
  local_e8[0] = 0;
  local_190 = 0;
  local_d0 = 0x7fffffffffffffff;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  infile = &DAT_00105ccf;
  outbase = &DAT_00105d20;
  do {
    while( true ) {
      iVar4 = _optind;
      if (_optind == 0) {
        iVar4 = 1;
      }
      iVar5 = getopt_long(param_1,param_2,"0123456789C:a:b:del:n:t:ux",longopts,0);
      if (iVar5 == -1) {
        if ((local_f0 != 0) && (filter_command != (char *)0x0)) {
          local_60 = gettext("--filter does not process a chunk extracted to standard output");
          error(0,0,local_60);
          usage(1);
        }
        if (local_194 == 0) {
          local_194 = 3;
          local_e8[0] = 1000;
        }
        if (local_e8[0] == 0) {
          uVar10 = quote(&DAT_00105f5f);
          local_58 = gettext("invalid number of lines: %s");
          error(0,0,local_58,uVar10);
          usage(1);
        }
        if (eolchar < 0) {
          eolchar = 10;
        }
        set_suffix_length(local_e8[0],local_194);
        if (_optind < param_1) {
          infile = (undefined *)param_2[_optind];
          _optind = _optind + 1;
        }
        if (_optind < param_1) {
          outbase = (undefined *)param_2[_optind];
          _optind = _optind + 1;
        }
        if (_optind < param_1) {
          uVar10 = quote(param_2[_optind]);
          local_50 = gettext("extra operand %s");
          error(0,0,local_50,uVar10);
          usage(1);
        }
        if ((numeric_suffix_start != (char *)0x0) &&
           (sVar12 = strlen(numeric_suffix_start), suffix_length < sVar12)) {
          local_48 = gettext("numerical suffix start value is too large for the suffix length");
          error(0,0,local_48);
          usage(1);
        }
        cVar3 = streq(infile,&DAT_00105ccf);
        if ((cVar3 != '\x01') && (iVar4 = fd_reopen(0,infile,0,0), iVar4 < 0)) {
          uVar10 = quotearg_style(4,infile);
          uVar11 = gettext("cannot open %s for reading");
          piVar13 = __errno_location();
          local_40 = uVar11;
          error(1,*piVar13,uVar11,uVar10);
        }
        xset_binary_mode(0,0);
        fdadvise(0,0,0,2);
        iVar4 = fstat(0,(stat *)in_stat_buf);
        if (iVar4 != 0) {
          uVar10 = quotearg_n_style_colon(0,3,infile);
          piVar13 = __errno_location();
          local_38 = &DAT_00105c15;
          error(1,*piVar13,&DAT_00105c15,uVar10);
        }
        if ((local_d8 == 0) && (local_d8 = io_blksize(in_stat_buf), 0x7ff00000 < local_d8)) {
          local_d8 = 0x7ff00000;
        }
        local_b8 = xalignalloc(local_c0,local_d8 + 1);
        local_c8 = -1;
        if ((local_194 == 5) || (local_194 == 6)) {
          local_d0 = input_file_size(0,in_stat_buf,local_b8,local_d8);
          if (local_d0 < 0) {
            uVar10 = quotearg_n_style_colon(0,3,infile);
            uVar11 = gettext("%s: cannot determine file size");
            piVar13 = __errno_location();
            local_30 = uVar11;
            error(1,*piVar13,uVar11,uVar10);
          }
          local_c8 = local_d0;
          if (local_d8 <= local_d0) {
            local_c8 = local_d8;
          }
        }
        if (filter_command != (char *)0x0) {
          p_Var14 = signal(0xd,(__sighandler_t)0x1);
          default_SIGPIPE = p_Var14 == (__sighandler_t)0x0;
        }
                    /* WARNING: Could not find normalized switch variable to match jumptable */
        switch(local_194) {
        default:
                    /* WARNING: Subroutine does not return */
          __assert_fail("0",
                        "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/split.c"
                        ,0x6af,(char *)&__PRETTY_FUNCTION___0);
        case 1:
          bytes_split(local_e8[0],0,local_b8,local_d8,0xffffffffffffffff,0);
          break;
        case 2:
          line_bytes_split(local_e8[0],local_b8,local_d8);
          break;
        case 3:
          lines_split(local_e8[0],local_b8,local_d8);
        }
        iVar4 = close(0);
        if (iVar4 != 0) {
          uVar10 = quotearg_n_style_colon(0,3,infile);
          piVar13 = __errno_location();
          local_28 = &DAT_00105c15;
          error(1,*piVar13,&DAT_00105c15,uVar10);
        }
        closeout(0,output_desc,filter_pid,outfile);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar5 < 0x84) break;
switchD_00103d22_caseD_3a:
      usage(1);
    }
    if (iVar5 < 0x30) {
      if (iVar5 != -0x83) {
        if (iVar5 != -0x82) goto switchD_00103d22_caseD_3a;
        usage(0);
      }
      uVar10 = proper_name_lite("Richard M. Stallman","Richard M. Stallman");
      uVar11 = proper_name_lite("Torbjorn Granlund",&DAT_00105ee9);
      version_etc(_stdout,"split","GNU coreutils",_Version,uVar11,uVar10,0);
                    /* WARNING: Subroutine does not return */
      exit(0);
    }
    switch(iVar5) {
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
      if (local_194 == 0) {
        local_194 = 4;
        local_e8[0] = 0;
      }
      if ((local_194 != 0) && (local_194 != 4)) {
        local_70 = gettext("cannot split in more than one way");
        error(0,0,local_70);
        usage(1);
      }
      if ((local_190 != 0) && (local_190 != iVar4)) {
        local_e8[0] = 0;
      }
      lVar8 = local_e8[0] * 10;
      local_190 = iVar4;
      if ((SEXT816(lVar8) != SEXT816(local_e8[0]) * SEXT816(10)) ||
         (local_e8[0] = (iVar5 + -0x30) + lVar8, SCARRY8((long)(iVar5 + -0x30),lVar8))) {
        local_e8[0] = 0x7fffffffffffffff;
      }
      break;
    default:
      goto switchD_00103d22_caseD_3a;
    case 0x43:
      if (local_194 != 0) {
        local_98 = gettext("cannot split in more than one way");
        error(0,0,local_98);
        usage(1);
      }
      local_194 = 2;
      local_e8[0] = parse_n_units(_optarg,multipliers_1,"invalid number of lines");
      break;
    case 0x61:
      uVar10 = gettext("invalid suffix length");
      suffix_length = xdectoimax(_optarg,0,0x7fffffffffffffff,&DAT_001051ac,uVar10,0);
      break;
    case 0x62:
      if (local_194 != 0) {
        local_a8 = gettext("cannot split in more than one way");
        error(0,0,local_a8);
        usage(1);
      }
      local_194 = 1;
      local_e8[0] = parse_n_units(_optarg,multipliers_1,"invalid number of bytes");
      break;
    case 100:
    case 0x78:
      if (iVar5 == 100) {
        suffix_alphabet = s_0123456789_00105e40;
      }
      else {
        suffix_alphabet = s_0123456789abcdef_00105e4b;
      }
      if (_optarg != (char *)0x0) {
        sVar12 = strlen(_optarg);
        sVar9 = strspn(_optarg,suffix_alphabet);
        if (sVar12 == sVar9) goto LAB_001044ec;
        uVar10 = quote(_optarg);
        if (iVar5 == 100) {
          local_68 = gettext("%s: invalid start value for numerical suffix");
        }
        else {
          local_68 = gettext("%s: invalid start value for hexadecimal suffix");
        }
        error(0,0,local_68,uVar10);
        usage(1);
        do {
          _optarg = _optarg + 1;
LAB_001044ec:
        } while ((*_optarg == '0') && (_optarg[1] != '\0'));
        numeric_suffix_start = _optarg;
      }
      break;
    case 0x65:
      elide_empty_files = 1;
      break;
    case 0x6c:
      if (local_194 != 0) {
        local_a0 = gettext("cannot split in more than one way");
        error(0,0,local_a0);
        usage(1);
      }
      local_194 = 3;
      local_e8[0] = parse_n_units(_optarg,&DAT_001051ac,"invalid number of lines");
      break;
    case 0x6e:
      if (local_194 == 0) goto LAB_0010407e;
      local_90 = gettext("cannot split in more than one way");
      error(0,0,local_90);
      usage(1);
      do {
        _optarg = _optarg + 1;
LAB_0010407e:
        ppuVar7 = __ctype_b_loc();
        puVar1 = *ppuVar7;
        bVar2 = to_uchar((int)*_optarg);
      } while ((puVar1[bVar2] & 0x2000) != 0);
      iVar4 = strncmp(_optarg,"r/",2);
      if (iVar4 == 0) {
        local_194 = 7;
        _optarg = _optarg + 2;
      }
      else {
        iVar4 = strncmp(_optarg,"l/",2);
        if (iVar4 == 0) {
          local_194 = 6;
          _optarg = _optarg + 2;
        }
        else {
          local_194 = 5;
        }
      }
      parse_chunk(&local_f0,local_e8,_optarg);
      break;
    case 0x74:
      local_195 = *_optarg;
      if (local_195 == '\0') {
        local_88 = gettext("empty record separator");
        error(1,0,local_88);
      }
      if (_optarg[1] != '\0') {
        cVar3 = streq(_optarg,&DAT_00105df7);
        if (cVar3 == '\0') {
          uVar10 = quote(_optarg);
          local_80 = gettext("multi-character separator %s");
          error(1,0,local_80,uVar10);
        }
        else {
          local_195 = '\0';
        }
      }
      if ((-1 < eolchar) && (local_195 != eolchar)) {
        local_78 = gettext("multiple separator characters specified");
        error(1,0,local_78);
      }
      eolchar = (int)local_195;
      break;
    case 0x75:
      unbuffered = 1;
      break;
    case 0x80:
      verbose = 1;
      break;
    case 0x81:
      filter_command = _optarg;
      break;
    case 0x82:
      uVar10 = gettext("invalid IO block size");
      local_d8 = xnumtoumax(_optarg,10,1,0x7ff00000,multipliers_1,uVar10,0,4);
      break;
    case 0x83:
      sVar12 = strlen(_optarg);
      pcVar6 = (char *)last_component(_optarg);
      if ((pcVar6 != _optarg) || (((int)sVar12 != 0 && (_optarg[(long)(int)sVar12 + -1] == '/')))) {
        uVar10 = quote(_optarg);
        local_b0 = gettext("invalid suffix %s, contains directory separator");
        error(0,0,local_b0,uVar10);
        usage(1);
      }
      additional_suffix = _optarg;
    }
  } while( true );
}

