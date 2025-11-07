// Function: maybe_close_stdout
void maybe_close_stdout(void) {
  int iVar1;
  
  if (close_stdout_required == '\0') {
    iVar1 = close_stream(_stderr);
    if (iVar1 != 0) {
                    /* WARNING: Subroutine does not return */
      _exit(1);
    }
  }
  else {
    close_stdout();
  }
  return;
}

// Function: diagnose
void diagnose(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
             undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
             undefined4 param_9,undefined8 param_10,undefined8 param_11,undefined8 param_12,
             undefined8 param_13,undefined8 param_14) {
  char in_AL;
  long in_FS_OFFSET;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined *local_d0;
  undefined *local_c8;
  long local_c0;
  undefined local_b8 [16];
  undefined8 local_a8;
  undefined8 local_a0;
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
  local_c0 = *(long *)(in_FS_OFFSET + 0x28);
  local_a8 = param_11;
  local_a0 = param_12;
  local_98 = param_13;
  local_90 = param_14;
  if (0 < progress_len) {
    fputc_unlocked(10,_stderr);
    progress_len = 0;
  }
  local_d8 = 0x10;
  local_d4 = 0x30;
  local_d0 = &stack0x00000008;
  local_c8 = local_b8;
  verror(0,param_9,param_10,&local_d8);
  if (local_c0 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPERAND]...\n  or:  %s OPTION\n");
    printf(pcVar3,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Copy a file, converting and formatting according to the operands.\n\n  bs=BYTES        read and write up to BYTES bytes at a time (default: 512);\n                  overrides ibs and obs\n  cbs=BYTES       convert BYTES bytes at a time\n  conv=CONVS      convert the file as per the comma separated symbol list\n  count=N         copy only N input blocks\n  ibs=BYTES       read up to BYTES bytes at a time (default: 512)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  if=FILE         read from FILE instead of standard input\n  iflag=FLAGS     read as per the comma separated symbol list\n  obs=BYTES       write BYTES bytes at a time (default: 512)\n  of=FILE         write to FILE instead of standard output\n  oflag=FLAGS     write as per the comma separated symbol list\n  seek=N          (or oseek=N) skip N obs-sized output blocks\n  skip=N          (or iseek=N) skip N ibs-sized input blocks\n  status=LEVEL    The LEVEL of information to print to standard error;\n                  \'none\' suppresses everything but error messages,\n                  \'noxfer\' suppresses the final transfer statistics,\n                  \'progress\' shows periodic transfer statistics\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nN and BYTES may be followed by the following multiplicative suffixes:\nc=1, w=2, b=512, kB=1000, K=1024, MB=1000*1000, M=1024*1024, xM=M,\nGB=1000*1000*1000, G=1024*1024*1024, and so on for T, P, E, Z, Y, R, Q.\nBinary prefixes can be used, too: KiB=K, MiB=M, and so on.\nIf N ends in \'B\', it counts bytes not blocks.\n\nEach CONV symbol may be:\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  ascii     from EBCDIC to ASCII\n  ebcdic    from ASCII to EBCDIC\n  ibm       from ASCII to alternate EBCDIC\n  block     pad newline-terminated records with spaces to cbs-size\n  unblock   replace trailing spaces in cbs-size records with newline\n  lcase     change upper case to lower case\n  ucase     change lower case to upper case\n  sparse    try to seek rather than write all-NUL output blocks\n  swab      swap every pair of input bytes\n  sync      pad every input block with NULs to ibs-size; when used\n            with block or unblock, pad with spaces rather than NULs\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  excl      fail if the output file already exists\n  nocreat   do not create the output file\n  notrunc   do not truncate the output file\n  noerror   continue after read errors\n  fdatasync  physically write output file data before finishing\n  fsync     likewise, but also write metadata\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nEach FLAG symbol may be:\n\n  append    append mode (makes sense only for output; conv=notrunc suggested)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  direct    use direct I/O for data\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  directory  fail unless a directory\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  dsync     use synchronized I/O for data\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  sync      likewise, but also for metadata\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  fullblock  accumulate full blocks of input (iflag only)\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  nonblock  use non-blocking I/O\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  noatime   do not update access time\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  nocache   Request to drop cache.  See also oflag=sync\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  noctty    do not assign controlling terminal from file\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  nofollow  do not follow symlinks\n");
    fputs_unlocked(pcVar3,pFVar1);
    pcVar3 = (char *)gettext(
                            "\nSending a %s signal to a running \'dd\' process makes it\nprint I/O statistics to standard error and then resume copying.\n\nOptions are:\n\n"
                            );
    printf(pcVar3,&DAT_001067b0);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00106821);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: alloc_ibuf
void alloc_ibuf(void) {
  long lVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  undefined local_2b8 [664];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (ibuf == 0) {
    ibuf = alignalloc(page_size,(ulong)((conversions_mask & 0x80) != 0) + input_blocksize);
    if (ibuf == 0) {
      uVar2 = human_readable(input_blocksize,local_2b8,0x1f1,1,1);
      lVar1 = input_blocksize;
      uVar3 = gettext("memory exhausted by input buffer of size %td bytes (%s)");
      error(1,0,uVar3,lVar1,uVar2);
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: alloc_obuf
void alloc_obuf(void) {
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  undefined local_2b8 [664];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (obuf == 0) {
    if ((conversions_mask & 0x800) != 0) {
      obuf = alignalloc(page_size,output_blocksize);
      if (obuf != 0) goto LAB_001009e8;
      uVar2 = human_readable(output_blocksize,local_2b8,0x1f1,1,1);
      uVar1 = output_blocksize;
      uVar3 = gettext("memory exhausted by output buffer of size %td bytes (%s)");
      error(1,0,uVar3,uVar1,uVar2);
    }
    alloc_ibuf();
    obuf = ibuf;
  }
LAB_001009e8:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: translate_charset
void translate_charset(long param_1) {
  int local_c;
  
  for (local_c = 0; local_c < 0x100; local_c = local_c + 1) {
    trans_table[local_c] = *(undefined *)(param_1 + (ulong)(byte)trans_table[local_c]);
  }
  translation_needed = 1;
  return;
}

// Function: multiple_bits_set
undefined4 multiple_bits_set(uint param_1) {
  param_1 = param_1 - 1 & param_1;
  return CONCAT31((int3)(param_1 >> 8),param_1 != 0);
}

// Function: abbreviation_lacks_prefix
bool abbreviation_lacks_prefix(char *param_1) {
  size_t sVar1;
  
  sVar1 = strlen(param_1);
  return param_1[sVar1 - 2] == ' ';
}

// Function: print_xfer_stats
void print_xfer_stats(long param_1) {
  char cVar1;
  int iVar2;
  long lVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  char *pcVar7;
  long in_FS_OFFSET;
  double local_828;
  char *local_820;
  undefined local_7e8 [654];
  undefined auStack_55a [654];
  char acStack_2cc [660];
  char local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  lVar3 = param_1;
  if (param_1 == 0) {
    lVar3 = gethrxtime();
  }
  uVar4 = human_readable(w_bytes,local_7e8,0x1d1,1,1);
  uVar5 = human_readable(w_bytes,auStack_55a,0x1f1,1,1);
  local_820 = acStack_2cc;
  if (start_time < lVar3) {
    local_828 = (double)(lVar3 - start_time) / DAT_00106eb0;
    local_820 = (char *)human_readable(w_bytes,local_820,0x1d1,1000000000,lVar3 - start_time);
    strcat(local_820,(char *)&slash_s_6);
  }
  else {
    local_828 = 0.0;
    uVar6 = gettext("Infinity");
    snprintf(local_820,0x28e,"%s B/s",uVar6);
  }
  if (param_1 == 0) {
    pcVar7 = "%g s";
  }
  else {
    fputc_unlocked(0xd,_stderr);
    pcVar7 = "%.0f s";
  }
  snprintf(local_38,0x18,pcVar7,local_828);
  cVar1 = abbreviation_lacks_prefix(uVar4);
  uVar6 = w_bytes;
  if (cVar1 == '\0') {
    cVar1 = abbreviation_lacks_prefix(uVar5);
    uVar6 = w_bytes;
    if (cVar1 == '\0') {
      pcVar7 = (char *)gettext("%jd bytes (%s, %s) copied, %s, %s");
      iVar2 = fprintf(_stderr,pcVar7,uVar6,uVar4,uVar5,local_38,local_820);
    }
    else {
      pcVar7 = (char *)gettext("%jd bytes (%s) copied, %s, %s");
      iVar2 = fprintf(_stderr,pcVar7,uVar6,uVar4,local_38,local_820);
    }
  }
  else {
    uVar4 = select_plural(w_bytes);
    pcVar7 = (char *)ngettext("%jd byte copied, %s, %s","%jd bytes copied, %s, %s",uVar4);
    iVar2 = fprintf(_stderr,pcVar7,uVar6,local_38,local_820);
  }
  if (param_1 == 0) {
    fputc_unlocked(10,_stderr);
    iVar2 = progress_len;
  }
  else if ((-1 < iVar2) && (iVar2 < progress_len)) {
    fprintf(_stderr,"%*s",(ulong)(uint)(progress_len - iVar2),&DAT_0010556c);
  }
  progress_len = iVar2;
  reported_w_bytes = w_bytes;
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: print_stats
void print_stats(void) {
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  long lVar4;
  char *pcVar5;
  undefined8 uVar6;
  
  if (status_level != 1) {
    if (0 < progress_len) {
      fputc_unlocked(10,_stderr);
      progress_len = 0;
    }
    uVar3 = r_full;
    uVar2 = r_partial;
    uVar1 = w_full;
    uVar6 = w_partial;
    pcVar5 = (char *)gettext("%jd+%jd records in\n%jd+%jd records out\n");
    fprintf(_stderr,pcVar5,uVar3,uVar2,uVar1,uVar6);
    lVar4 = r_truncate;
    if (r_truncate != 0) {
      uVar6 = select_plural(r_truncate);
      pcVar5 = (char *)ngettext("%jd truncated record\n","%jd truncated records\n",uVar6);
      fprintf(_stderr,pcVar5,lVar4);
    }
    if (status_level != 2) {
      print_xfer_stats(0);
    }
  }
  return;
}

// Function: interrupt_handler
void interrupt_handler(undefined4 param_1) {
  interrupt_signal = param_1;
  return;
}

// Function: siginfo_handler
void siginfo_handler(void) {
  info_signal_count = info_signal_count + 1;
  return;
}

// Function: install_signal_handlers
void install_signal_handlers(void) {
  int iVar1;
  char *pcVar2;
  long in_FS_OFFSET;
  sigaction local_a8;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  pcVar2 = getenv("POSIXLY_CORRECT");
  sigemptyset((sigset_t *)&caught_signals);
  if (pcVar2 == (char *)0x0) {
    sigaddset((sigset_t *)&caught_signals,10);
  }
  sigaction(2,(sigaction *)0x0,&local_a8);
  if (local_a8.__sigaction_handler.sa_handler != (__sighandler_t)0x1) {
    sigaddset((sigset_t *)&caught_signals,2);
  }
  local_a8.sa_mask.__val[0] = caught_signals;
  local_a8.sa_mask.__val[1] = DAT_001052a8;
  local_a8.sa_mask.__val[2] = DAT_001052b0;
  local_a8.sa_mask.__val[3] = DAT_001052b8;
  local_a8.sa_mask.__val[4] = DAT_001052c0;
  local_a8.sa_mask.__val[5] = DAT_001052c8;
  local_a8.sa_mask.__val[6] = DAT_001052d0;
  local_a8.sa_mask.__val[7] = DAT_001052d8;
  local_a8.sa_mask.__val[8] = DAT_001052e0;
  local_a8.sa_mask.__val[9] = DAT_001052e8;
  local_a8.sa_mask.__val[10] = DAT_001052f0;
  local_a8.sa_mask.__val[11] = DAT_001052f8;
  local_a8.sa_mask.__val[12] = DAT_00105300;
  local_a8.sa_mask.__val[13] = DAT_00105308;
  local_a8.sa_mask.__val[14] = DAT_00105310;
  local_a8.sa_mask.__val[15] = DAT_00105318;
  iVar1 = sigismember((sigset_t *)&caught_signals,10);
  if (iVar1 != 0) {
    local_a8.__sigaction_handler.sa_handler = siginfo_handler;
    local_a8.sa_flags = 0;
    sigaction(10,&local_a8,(sigaction *)0x0);
  }
  iVar1 = sigismember((sigset_t *)&caught_signals,2);
  if (iVar1 != 0) {
    local_a8.__sigaction_handler.sa_handler = interrupt_handler;
    local_a8.sa_flags = -0x40000000;
    sigaction(2,&local_a8,(sigaction *)0x0);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: iclose
undefined8 iclose(int param_1) {
  int iVar1;
  int *piVar2;
  
  iVar1 = close(param_1);
  if (iVar1 != 0) {
    do {
      piVar2 = __errno_location();
      if (*piVar2 != 4) {
        return 0xffffffff;
      }
      iVar1 = close(param_1);
    } while ((iVar1 != 0) && (piVar2 = __errno_location(), *piVar2 != 9));
  }
  return 0;
}

// Function: cleanup
void cleanup(EVP_PKEY_CTX *ctx) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  
  if (interrupt_signal == 0) {
    iVar1 = synchronize_output();
    if (iVar1 != 0) {
                    /* WARNING: Subroutine does not return */
      exit(iVar1);
    }
  }
  iVar1 = iclose(0);
  if (iVar1 != 0) {
    uVar2 = quotearg_style(4,input_file);
    uVar3 = gettext("closing input file %s");
    piVar4 = __errno_location();
    error(1,*piVar4,uVar3,uVar2);
  }
  iVar1 = iclose(1);
  if (iVar1 != 0) {
    uVar2 = quotearg_style(4,output_file);
    uVar3 = gettext("closing output file %s");
    piVar4 = __errno_location();
    error(1,*piVar4,uVar3,uVar2);
  }
  return;
}

// Function: process_signals
void process_signals(void) {
  int __sig;
  EVP_PKEY_CTX *ctx;
  long in_FS_OFFSET;
  sigset_t local_98;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  while ((interrupt_signal != 0 || (info_signal_count != 0))) {
    sigprocmask(0,(sigset_t *)&caught_signals,&local_98);
    __sig = interrupt_signal;
    if (info_signal_count != 0) {
      info_signal_count = info_signal_count + -1;
    }
    ctx = (EVP_PKEY_CTX *)0x2;
    sigprocmask(2,&local_98,(sigset_t *)0x0);
    if (__sig != 0) {
      cleanup(ctx);
    }
    print_stats();
    if (__sig != 0) {
      raise(__sig);
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: finish_up
void finish_up(EVP_PKEY_CTX *param_1) {
  process_signals();
  cleanup(param_1);
  print_stats();
  return;
}

// Function: quit
void quit(int param_1) {
  finish_up();
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: cache_round
long cache_round(int param_1,long param_2) {
  long lVar1;
  long *plVar2;
  long in_FS_OFFSET;
  long local_38;
  long local_20;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  if (param_1 == 0) {
    plVar2 = &i_pending_5;
  }
  else {
    plVar2 = &o_pending_4;
  }
  if (param_2 == 0) {
    local_38 = *plVar2;
  }
  else {
    local_20 = param_2 + *plVar2;
    if (SCARRY8(param_2,*plVar2)) {
      local_20 = 0x7fffffffffffffff;
    }
    *plVar2 = local_20 % 0x40000;
    if (*plVar2 < local_20) {
      local_38 = local_20 - *plVar2;
    }
    else {
      local_38 = 0;
    }
  }
  if (lVar1 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_38;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: invalidate_cache
bool invalidate_cache(int param_1,long param_2) {
  char cVar1;
  long lVar2;
  int *piVar3;
  bool bVar4;
  int local_24;
  long local_20;
  long local_18;
  long local_10;
  
  local_24 = -1;
  cVar1 = o_nocache_eof;
  if (param_1 == 0) {
    cVar1 = i_nocache_eof;
  }
  lVar2 = cache_round(param_1,param_2);
  if ((param_2 == 0) || (lVar2 != 0)) {
    if ((param_2 != 0) || ((lVar2 != 0 || (cVar1 != '\0')))) {
      if (param_2 == 0) {
        local_10 = 0;
      }
      else {
        local_10 = cache_round(param_1,0);
      }
      if (param_1 == 0) {
        if (input_seekable == '\0') {
          local_20 = -1;
          piVar3 = __errno_location();
          *piVar3 = 0x1d;
        }
        else {
          local_20 = input_offset;
        }
      }
      else {
        if (output_offset_3 != -1) {
          if (output_offset_3 < 0) {
            output_offset_3 = lseek(param_1,0,1);
          }
          else if (param_2 != 0) {
            output_offset_3 = output_offset_3 + lVar2 + local_10;
          }
        }
        local_20 = output_offset_3;
      }
      if (-1 < local_20) {
        local_18 = lVar2;
        if (((param_2 == 0) && (lVar2 != 0)) && (cVar1 != '\0')) {
          local_18 = 0;
          local_10 = lVar2;
        }
        local_20 = (local_20 - local_18) - local_10;
        if (local_18 == 0) {
          local_20 = local_20 - local_20 % page_size;
        }
        local_24 = posix_fadvise(param_1,local_20,local_18,4);
        piVar3 = __errno_location();
        *piVar3 = local_24;
      }
      bVar4 = local_24 == 0;
    }
    else {
      bVar4 = true;
    }
  }
  else {
    bVar4 = true;
  }
  return bVar4;
}

// Function: iread
long iread(int param_1,void *param_2,size_t param_3) {
  long lVar1;
  int *piVar2;
  undefined8 uVar3;
  ssize_t local_18;
  
  do {
    process_signals();
    local_18 = read(param_1,param_2,param_3);
    if (local_18 == -1) {
      piVar2 = __errno_location();
      if ((((*piVar2 == 0x16) && (0 < prev_nread_2)) && (prev_nread_2 < (long)param_3)) &&
         ((input_flags & 0x4000) != 0)) {
        piVar2 = __errno_location();
        *piVar2 = 0;
        local_18 = 0;
      }
    }
    if (-1 < local_18) break;
    piVar2 = __errno_location();
  } while (*piVar2 == 4);
  if ((0 < local_18) && (local_18 < (long)param_3)) {
    process_signals();
  }
  lVar1 = prev_nread_2;
  if (((0 < local_18) && (warn_partial_read != '\0')) &&
     ((0 < prev_nread_2 && (prev_nread_2 < (long)param_3)))) {
    if (status_level != 1) {
      uVar3 = select_plural(prev_nread_2);
      uVar3 = ngettext("warning: partial read (%td byte); suggest iflag=fullblock",
                       "warning: partial read (%td bytes); suggest iflag=fullblock",uVar3);
      diagnose(0,uVar3,lVar1);
    }
    warn_partial_read = '\0';
  }
  prev_nread_2 = local_18;
  return local_18;
}

// Function: iread_fullblock
long iread_fullblock(undefined4 param_1,long param_2,long param_3) {
  long lVar1;
  long lVar2;
  long local_30;
  long local_28;
  long local_18;
  
  local_18 = 0;
  local_30 = param_3;
  local_28 = param_2;
  while (((lVar2 = local_18, 0 < local_30 &&
          (lVar1 = iread(param_1,local_28,local_30), lVar2 = lVar1, -1 < lVar1)) &&
         (lVar2 = local_18, lVar1 != 0))) {
    local_18 = local_18 + lVar1;
    local_28 = local_28 + lVar1;
    local_30 = local_30 - lVar1;
  }
  return lVar2;
}

// Function: iwrite
long iwrite(int param_1,long param_2,long param_3) {
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  __off_t _Var7;
  long local_28;
  long local_20;
  
  local_28 = 0;
  if (((output_flags & 0x4000) != 0) && (param_3 < output_blocksize)) {
    uVar2 = rpl_fcntl(1,3);
    iVar3 = rpl_fcntl(1,4,uVar2 & 0xffffbfff);
    if ((iVar3 != 0) && (status_level != 1)) {
      uVar4 = quotearg_n_style_colon(0,3,output_file);
      uVar5 = gettext("failed to turn off O_DIRECT: %s");
      piVar6 = __errno_location();
      diagnose(*piVar6,uVar5,uVar4);
    }
    o_nocache_eof = 1;
    invalidate_cache(1,0);
    conversions_mask = conversions_mask | 0x8000;
  }
  do {
    while( true ) {
      if (param_3 <= local_28) goto LAB_00101abd;
      local_20 = 0;
      process_signals();
      final_op_was_seek = 0;
      if (((conversions_mask & 0x10000) != 0) && (cVar1 = is_nul(param_2,param_3), cVar1 != '\0')) {
        _Var7 = lseek(param_1,param_3,1);
        if (_Var7 < 0) {
          conversions_mask = conversions_mask & 0xfffeffff;
        }
        else {
          final_op_was_seek = 1;
          local_20 = param_3;
        }
      }
      if (local_20 == 0) {
        local_20 = write(param_1,(void *)(local_28 + param_2),param_3 - local_28);
      }
      if (local_20 < 0) break;
      if (local_20 == 0) {
        piVar6 = __errno_location();
        *piVar6 = 0x1c;
        goto LAB_00101abd;
      }
      local_28 = local_28 + local_20;
    }
    piVar6 = __errno_location();
  } while (*piVar6 == 4);
LAB_00101abd:
  if ((o_nocache != '\0') && (local_28 != 0)) {
    invalidate_cache(param_1,local_28);
  }
  return local_28;
}

// Function: write_output
void write_output(void) {
  long lVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  
  lVar1 = iwrite(1,obuf,output_blocksize);
  w_bytes = lVar1 + w_bytes;
  if (lVar1 == output_blocksize) {
    w_full = w_full + 1;
  }
  else {
    uVar2 = quotearg_style(4,output_file);
    uVar3 = gettext("writing to %s");
    piVar4 = __errno_location();
    diagnose(*piVar4,uVar3,uVar2);
    if (lVar1 != 0) {
      w_partial = w_partial + 1;
    }
    quit(1);
  }
  oc = 0;
  return;
}

// Function: ifdatasync
int ifdatasync(int param_1) {
  int iVar1;
  int *piVar2;
  
  do {
    process_signals();
    iVar1 = fdatasync(param_1);
    if (-1 < iVar1) {
      return iVar1;
    }
    piVar2 = __errno_location();
  } while (*piVar2 == 4);
  return iVar1;
}

// Function: ifd_reopen
int ifd_reopen(undefined4 param_1,undefined8 param_2,undefined4 param_3,undefined4 param_4) {
  int iVar1;
  int *piVar2;
  
  do {
    process_signals();
    iVar1 = fd_reopen(param_1,param_2,param_3,param_4);
    if (-1 < iVar1) {
      return iVar1;
    }
    piVar2 = __errno_location();
  } while (*piVar2 == 4);
  return iVar1;
}

// Function: ifstat
int ifstat(int param_1,stat *param_2) {
  int iVar1;
  int *piVar2;
  
  do {
    process_signals();
    iVar1 = fstat(param_1,param_2);
    if (-1 < iVar1) {
      return iVar1;
    }
    piVar2 = __errno_location();
  } while (*piVar2 == 4);
  return iVar1;
}

// Function: ifsync
int ifsync(int param_1) {
  int iVar1;
  int *piVar2;
  
  do {
    process_signals();
    iVar1 = fsync(param_1);
    if (-1 < iVar1) {
      return iVar1;
    }
    piVar2 = __errno_location();
  } while (*piVar2 == 4);
  return iVar1;
}

// Function: iftruncate
int iftruncate(int param_1,__off_t param_2) {
  int iVar1;
  int *piVar2;
  
  do {
    process_signals();
    iVar1 = ftruncate(param_1,param_2);
    if (-1 < iVar1) {
      return iVar1;
    }
    piVar2 = __errno_location();
  } while (*piVar2 == 4);
  return iVar1;
}

// Function: operand_matches
undefined4 operand_matches(char *param_1,char *param_2,char param_3) {
  char cVar1;
  char cVar2;
  char *local_18;
  char *local_10;
  
  local_18 = param_2;
  local_10 = param_1;
  while (*local_18 != '\0') {
    cVar1 = *local_10;
    cVar2 = *local_18;
    local_18 = local_18 + 1;
    local_10 = local_10 + 1;
    if (cVar1 != cVar2) {
      return 0;
    }
  }
  if ((*local_10 != '\0') && (param_3 != *local_10)) {
    return 0;
  }
  return 1;
}

// Function: parse_symbols
uint parse_symbols(char *param_1,char *param_2,char param_3,undefined8 param_4) {
  char cVar1;
  char *pcVar2;
  size_t sVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  char *local_40;
  uint local_34;
  char *local_30;
  
  local_34 = 0;
  local_40 = param_1;
  while( true ) {
    pcVar2 = strchr(local_40,0x2c);
    local_30 = param_2;
    while( true ) {
      cVar1 = operand_matches(local_40,local_30,0x2c);
      if ((cVar1 == '\x01') && (*(int *)(local_30 + 0xc) != 0)) break;
      if (*local_30 == '\0') {
        if (pcVar2 == (char *)0x0) {
          sVar3 = strlen(local_40);
        }
        else {
          sVar3 = (long)pcVar2 - (long)local_40;
        }
        uVar4 = quotearg_n_style_mem(0,8,local_40,sVar3);
        uVar5 = gettext(param_4);
        diagnose(0,"%s: %s",uVar5,uVar4);
        usage(1);
      }
      local_30 = local_30 + 0x10;
    }
    if (param_3 == '\0') {
      local_34 = local_34 | *(uint *)(local_30 + 0xc);
    }
    else {
      local_34 = *(uint *)(local_30 + 0xc);
    }
    if (pcVar2 == (char *)0x0) break;
    local_40 = pcVar2 + 1;
  }
  return local_34;
}

// Function: parse_integer
long parse_integer(char *param_1,uint *param_2) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  bool bVar5;
  uint local_4c;
  uint local_48;
  int local_44;
  long local_40;
  char *local_38;
  long local_30;
  long local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_44 = 0;
  local_40 = 0;
  local_48 = xstrtoumax(param_1,&local_38,10,&local_40,suffixes_1);
  if (((((local_48 & 0xfffffffe) == 2) && (*local_38 == 'B')) && (param_1 < local_38)) &&
     ((local_38[-1] != 'B' && (local_38 = local_38 + 1, *local_38 == '\0')))) {
    local_48 = local_48 & 0xfffffffd;
  }
  if (((local_48 & 0xfffffffe) == 2) && (*local_38 == 'x')) {
    local_4c = 0;
    local_28 = parse_integer(local_38 + 1,&local_4c);
    if (local_4c < 2) {
      local_30 = local_28 * local_40;
      bVar5 = SEXT816(local_30) != SEXT816(local_28) * SEXT816(local_40);
      if ((local_40 < 0) &&
         ((bVar5 = true, local_28 == 0 || ((local_28 == -1 && (local_40 == local_30)))))) {
        bVar5 = false;
      }
      if ((bVar5) || ((local_30 != 0 && (((local_4c | local_48) & 1) != 0)))) {
        local_48 = 1;
        local_30 = 0x7fffffffffffffff;
      }
      else {
        if ((local_30 == 0) && (iVar1 = strncmp(param_1,"0x",2), iVar1 == 0)) {
          uVar2 = quote_n(1,&DAT_00106a80);
          uVar3 = quote_n(0,&DAT_00106a7d);
          uVar4 = gettext("warning: %s is a zero multiplier; use %s if that is intended");
          diagnose(0,uVar4,uVar3,uVar2);
        }
        local_48 = 0;
      }
    }
    else {
      local_48 = local_4c;
      local_30 = (long)local_44;
    }
  }
  else if (local_40 < 0) {
    local_48 = 1;
    local_30 = 0x7fffffffffffffff;
  }
  else {
    local_30 = local_40;
  }
  *param_2 = local_48;
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_30;
}

// Function: operand_is
void operand_is(undefined8 param_1,undefined8 param_2) {
  operand_matches(param_1,param_2,0x3d);
  return;
}

// Function: scanargs
void scanargs(int param_1,long param_2) {
  bool bVar1;
  bool bVar2;
  bool bVar3;
  char cVar4;
  uint uVar5;
  char *pcVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  long in_FS_OFFSET;
  bool bVar9;
  int local_e0;
  int local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  long local_a8;
  long local_a0;
  long local_98;
  long local_90;
  long local_88;
  long local_80;
  long *local_78;
  char *local_70;
  char *local_68;
  long local_60;
  long local_58;
  char *local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_a8 = 0;
  local_a0 = 0x7fffffffffffffff;
  local_98 = 0;
  local_90 = 0;
  bVar2 = false;
  bVar1 = false;
  bVar3 = false;
  for (local_dc = _optind; local_dc < param_1; local_dc = local_dc + 1) {
    local_70 = *(char **)(param_2 + (long)local_dc * 8);
    local_68 = strchr(local_70,0x3d);
    if (local_68 == (char *)0x0) {
      uVar7 = quotearg_style(4,local_70);
      uVar8 = gettext("unrecognized operand %s");
      diagnose(0,uVar8,uVar7);
      usage(1);
    }
    local_68 = local_68 + 1;
    cVar4 = operand_is(local_70,&DAT_00106add);
    if (cVar4 == '\0') {
      cVar4 = operand_is(local_70,&DAT_00106ae0);
      if (cVar4 == '\0') {
        cVar4 = operand_is(local_70,&DAT_00106ae3);
        if (cVar4 == '\0') {
          cVar4 = operand_is(local_70,"iflag");
          if (cVar4 == '\0') {
            cVar4 = operand_is(local_70,"oflag");
            if (cVar4 == '\0') {
              cVar4 = operand_is(local_70,"status");
              if (cVar4 == '\0') {
                local_e0 = 0;
                local_60 = parse_integer(local_68,&local_e0);
                pcVar6 = strchr(local_68,0x42);
                bVar9 = pcVar6 != (char *)0x0;
                local_88 = 0;
                local_80 = 0x7fffffffffffffff;
                local_78 = (long *)0x0;
                local_58 = 0x7ffffffffffffffe;
                cVar4 = operand_is(local_70,&DAT_00106b4a);
                if (cVar4 == '\0') {
                  cVar4 = operand_is(local_70,&DAT_00106b4e);
                  if (cVar4 == '\0') {
                    cVar4 = operand_is(local_70,&DAT_00106b52);
                    if (cVar4 == '\0') {
                      cVar4 = operand_is(local_70,&DAT_00106b55);
                      if (cVar4 == '\0') {
                        cVar4 = operand_is(local_70,&DAT_00106b59);
                        if (cVar4 == '\0') {
                          cVar4 = operand_is(local_70,"iseek");
                          if (cVar4 == '\0') {
                            cVar4 = operand_is(local_70 + (*local_70 == 'o'),&DAT_00106b64);
                            if (cVar4 == '\0') {
                              cVar4 = operand_is(local_70,"count");
                              if (cVar4 == '\0') {
                                uVar7 = quotearg_style(4,local_70);
                                uVar8 = gettext("unrecognized operand %s");
                                diagnose(0,uVar8,uVar7);
                                usage(1);
                              }
                              else {
                                local_a0 = local_60;
                                bVar2 = bVar9;
                              }
                            }
                            else {
                              local_90 = local_60;
                              bVar3 = bVar9;
                            }
                            goto LAB_00102638;
                          }
                        }
                        local_98 = local_60;
                        bVar1 = bVar9;
                      }
                      else {
                        local_88 = 1;
                        local_80 = 0x7fffffffffffffff;
                        local_78 = &conversion_blocksize;
                      }
                    }
                    else {
                      local_88 = 1;
                      local_80 = local_58;
                      local_78 = &local_a8;
                    }
                  }
                  else {
                    local_88 = 1;
                    local_80 = local_58;
                    local_78 = &output_blocksize;
                  }
                }
                else {
                  local_88 = 1;
                  local_80 = local_58;
                  local_78 = &input_blocksize;
                }
LAB_00102638:
                if (local_60 < local_88) {
                  local_e0 = 4;
                }
                else if (local_80 < local_60) {
                  local_e0 = 1;
                }
                if (local_e0 != 0) {
                  uVar7 = quotearg_style(4,local_68);
                  uVar8 = gettext("invalid number");
                  if (local_e0 == 1) {
                    local_d4 = 0x4b;
                  }
                  else {
                    local_d4 = 0;
                  }
                  local_d8 = 1;
                  local_50 = "%s: %s";
                  error(1,local_d4,"%s: %s",uVar8,uVar7);
                }
                if (local_78 != (long *)0x0) {
                  *local_78 = local_60;
                }
              }
              else {
                status_level = parse_symbols(local_68,&statuses,1,"invalid status level");
              }
            }
            else {
              uVar5 = parse_symbols(local_68,"append",0,"invalid output flag");
              output_flags = uVar5 | output_flags;
            }
          }
          else {
            uVar5 = parse_symbols(local_68,"append",0,"invalid input flag");
            input_flags = uVar5 | input_flags;
          }
        }
        else {
          uVar5 = parse_symbols(local_68,&conversions,0,"invalid conversion");
          conversions_mask = uVar5 | conversions_mask;
        }
      }
      else {
        output_file = local_68;
      }
    }
    else {
      input_file = local_68;
    }
  }
  if (local_a8 == 0) {
    conversions_mask = conversions_mask | 0x800;
  }
  else {
    output_blocksize = local_a8;
    input_blocksize = local_a8;
  }
  if (input_blocksize == 0) {
    input_blocksize = 0x200;
  }
  if (output_blocksize == 0) {
    output_blocksize = 0x200;
  }
  if (conversion_blocksize == 0) {
    conversions_mask = conversions_mask & 0xffffffe7;
  }
  if ((input_flags & 0x101000) != 0) {
    input_flags = input_flags | 0x101000;
  }
  if ((output_flags & 1) != 0) {
    uVar7 = quote("fullblock");
    uVar8 = gettext("invalid output flag");
    diagnose(0,"%s: %s",uVar8,uVar7);
    usage(1);
  }
  if (bVar1) {
    input_flags = input_flags | 8;
  }
  if (((input_flags & 8) == 0) || (local_98 == 0)) {
    if (local_98 != 0) {
      skip_records = local_98;
    }
  }
  else {
    skip_records = local_98 / input_blocksize;
    skip_bytes = local_98 % input_blocksize;
  }
  if (bVar2) {
    input_flags = input_flags | 4;
  }
  if (((input_flags & 4) == 0) || (local_a0 == 0x7fffffffffffffff)) {
    if (local_a0 != 0x7fffffffffffffff) {
      max_records = local_a0;
    }
  }
  else {
    max_records = local_a0 / input_blocksize;
    max_bytes = local_a0 % input_blocksize;
  }
  if (bVar3) {
    output_flags = output_flags | 0x10;
  }
  if (((output_flags & 0x10) == 0) || (local_90 == 0)) {
    if (local_90 != 0) {
      seek_records = local_90;
    }
  }
  else {
    seek_records = local_90 / output_blocksize;
    seek_bytes = local_90 % output_blocksize;
  }
  if ((((conversions_mask & 0x800) == 0) && ((input_flags & 1) == 0)) &&
     ((skip_records != 0 ||
      (((0 < max_records && (max_records != 0x7fffffffffffffff)) ||
       (((output_flags | input_flags) & 0x4000) != 0)))))) {
    warn_partial_read = 1;
  }
  else {
    warn_partial_read = 0;
  }
  if ((input_flags & 1) == 0) {
    iread_fnc = iread;
  }
  else {
    iread_fnc = iread_fullblock;
  }
  input_flags = input_flags & 0xfffffffe;
  cVar4 = multiple_bits_set(conversions_mask & 7);
  if (cVar4 != '\0') {
    local_48 = gettext("cannot combine any two of {ascii,ebcdic,ibm}");
    local_d0 = 1;
    local_cc = 0;
    error(1,0,local_48);
  }
  cVar4 = multiple_bits_set(conversions_mask & 0x18);
  if (cVar4 != '\0') {
    local_40 = gettext("cannot combine block and unblock");
    local_c8 = 1;
    local_c4 = 0;
    error(1,0,local_40);
  }
  cVar4 = multiple_bits_set(conversions_mask & 0x60);
  if (cVar4 != '\0') {
    local_38 = gettext("cannot combine lcase and ucase");
    local_c0 = 1;
    local_bc = 0;
    error(1,0,local_38);
  }
  cVar4 = multiple_bits_set(conversions_mask & 0x3000);
  if (cVar4 != '\0') {
    local_30 = gettext("cannot combine excl and nocreat");
    local_b8 = 1;
    local_b4 = 0;
    error(1,0,local_30);
  }
  cVar4 = multiple_bits_set(input_flags & 0x4002);
  if (cVar4 == '\0') {
    cVar4 = multiple_bits_set(output_flags & 0x4002);
    if (cVar4 == '\0') goto LAB_00102c28;
  }
  local_28 = gettext("cannot combine direct and nocache");
  local_b0 = 1;
  local_ac = 0;
  error(1,0,local_28);
LAB_00102c28:
  if ((input_flags & 2) != 0) {
    i_nocache = 1;
    if ((max_records == 0) && (max_bytes == 0)) {
      i_nocache_eof = 1;
    }
    else {
      i_nocache_eof = 0;
    }
    input_flags = input_flags & 0xfffffffd;
  }
  if ((output_flags & 2) != 0) {
    o_nocache = 1;
    if ((max_records == 0) && (max_bytes == 0)) {
      o_nocache_eof = 1;
    }
    else {
      o_nocache_eof = 0;
    }
    output_flags = output_flags & 0xfffffffd;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: apply_translations
void apply_translations(void) {
  int iVar1;
  int local_c;
  
  if ((conversions_mask & 1) != 0) {
    translate_charset(ebcdic_to_ascii);
  }
  if ((conversions_mask & 0x40) == 0) {
    if ((conversions_mask & 0x20) != 0) {
      for (local_c = 0; local_c < 0x100; local_c = local_c + 1) {
        iVar1 = tolower((uint)(byte)trans_table[local_c]);
        trans_table[local_c] = (char)iVar1;
      }
      translation_needed = 1;
    }
  }
  else {
    for (local_c = 0; local_c < 0x100; local_c = local_c + 1) {
      iVar1 = toupper((uint)(byte)trans_table[local_c]);
      trans_table[local_c] = (char)iVar1;
    }
    translation_needed = 1;
  }
  if ((conversions_mask & 2) == 0) {
    if ((conversions_mask & 4) != 0) {
      translate_charset(ascii_to_ibm);
      newline_character = 0x25;
      space_character = 0x40;
    }
  }
  else {
    translate_charset(ascii_to_ebcdic);
    newline_character = 0x25;
    space_character = 0x40;
  }
  return;
}

// Function: translate_buffer
void translate_buffer(char *param_1,long param_2) {
  byte bVar1;
  long local_18;
  char *local_10;
  
  local_10 = param_1;
  for (local_18 = param_2; local_18 != 0; local_18 = local_18 + -1) {
    bVar1 = to_uchar((int)*local_10);
    *local_10 = trans_table[(int)(uint)bVar1];
    local_10 = local_10 + 1;
  }
  return;
}

// Function: swab_buffer
long swab_buffer(long param_1,long *param_2,uint *param_3) {
  uint uVar1;
  long local_10;
  
  if (*param_2 != 0) {
    uVar1 = *param_3;
    if (((byte)*param_2 & 1) == (byte)~(byte)(uVar1 >> 0x18) >> 7) {
      *param_3 = 0xffffffff;
    }
    else {
      *param_2 = *param_2 + -1;
      *param_3 = (uint)*(byte *)(param_1 + *param_2);
    }
    for (local_10 = *param_2; 1 < local_10; local_10 = local_10 + -2) {
      *(undefined *)(param_1 + local_10) = *(undefined *)(param_1 + local_10 + -2);
    }
    if ((int)uVar1 < 0) {
      param_1 = param_1 + 1;
    }
    else {
      *(char *)(param_1 + 1) = (char)uVar1;
      *param_2 = *param_2 + 1;
    }
  }
  return param_1;
}

// Function: advance_input_offset
void advance_input_offset(long param_1) {
  bool bVar1;
  
  if ((-1 < input_offset) &&
     (bVar1 = SCARRY8(param_1,input_offset), input_offset = param_1 + input_offset, bVar1)) {
    input_offset = -1;
  }
  return;
}

// Function: skip
long skip(int param_1,undefined8 param_2,long param_3,long param_4,long *param_5) {
  char cVar1;
  int iVar2;
  int *piVar3;
  long lVar4;
  __off_t _Var5;
  undefined8 uVar6;
  undefined8 uVar7;
  char *pcVar8;
  long in_FS_OFFSET;
  int local_e4;
  long local_d8;
  undefined8 local_d0;
  undefined local_b8 [48];
  long local_88;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  piVar3 = __errno_location();
  *piVar3 = 0;
  lVar4 = param_3 * param_4;
  if (((SEXT816(lVar4) != SEXT816(param_3) * SEXT816(param_4)) ||
      (local_d8 = *param_5 + lVar4, SCARRY8(*param_5,lVar4))) ||
     (_Var5 = lseek(param_1,local_d8,1), _Var5 < 0)) {
    piVar3 = __errno_location();
    local_e4 = *piVar3;
    _Var5 = lseek(param_1,0,2);
    if (-1 < _Var5) {
      if (local_e4 == 0) {
        local_e4 = 0x4b;
      }
      uVar6 = quotearg_n_style_colon(0,3,param_2);
      if (param_1 == 0) {
        pcVar8 = "%s: cannot skip";
      }
      else {
        pcVar8 = "%s: cannot seek";
      }
      uVar7 = gettext(pcVar8);
      diagnose(local_e4,uVar7,uVar6);
      quit(1);
    }
    if (param_1 == 0) {
      alloc_ibuf();
      local_d0 = ibuf;
    }
    else {
      alloc_obuf();
      local_d0 = obuf;
    }
    do {
      lVar4 = param_4;
      if (param_3 == 0) {
        lVar4 = *param_5;
      }
      lVar4 = (*iread_fnc)(param_1,local_d0,lVar4);
      if (lVar4 < 0) {
        if (param_1 == 0) {
          uVar6 = quotearg_style(4,param_2);
          uVar7 = gettext("error reading %s");
          piVar3 = __errno_location();
          diagnose(*piVar3,uVar7,uVar6);
          if ((conversions_mask & 0x100) != 0) {
            print_stats();
          }
        }
        else {
          uVar6 = quotearg_n_style_colon(0,3,param_2);
          uVar7 = gettext("%s: cannot seek");
          diagnose(local_e4,uVar7,uVar6);
        }
        quit(1);
      }
      else {
        if (lVar4 == 0) break;
        if (param_1 == 0) {
          advance_input_offset(lVar4);
        }
      }
      if (param_3 == 0) {
        *param_5 = 0;
      }
      else {
        param_3 = param_3 + -1;
      }
    } while ((param_3 != 0) || (*param_5 != 0));
  }
  else if (param_1 == 0) {
    iVar2 = ifstat(0,local_b8);
    if (iVar2 != 0) {
      uVar6 = quotearg_style(4,param_2);
      uVar7 = gettext("cannot fstat %s");
      piVar3 = __errno_location();
      error(1,*piVar3,uVar7,uVar6);
    }
    cVar1 = usable_st_size(local_b8);
    if (((cVar1 == '\0') || (local_88 < 1)) ||
       ((input_offset < 0 || (local_d8 <= local_88 - input_offset)))) {
      param_3 = 0;
    }
    else {
      param_3 = (local_d8 - local_88) / param_4;
      local_d8 = local_88 - input_offset;
    }
    advance_input_offset(local_d8);
  }
  else {
    param_3 = 0;
    *param_5 = 0;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return param_3;
}

// Function: advance_input_after_read_error
undefined8 advance_input_after_read_error(long param_1) {
  int *piVar1;
  __off_t _Var2;
  long __offset;
  undefined8 uVar3;
  undefined8 uVar4;
  
  if (input_seekable == '\x01') {
    advance_input_offset(param_1);
    if (input_offset < 0) {
      uVar3 = quotearg_style(4,input_file);
      uVar4 = gettext("offset overflow while reading file %s");
      diagnose(0,uVar4,uVar3);
      return 0;
    }
    _Var2 = lseek(0,0,1);
    if (-1 < _Var2) {
      if (_Var2 == input_offset) {
        return 1;
      }
      __offset = input_offset - _Var2;
      if (((__offset < 0) || (param_1 < __offset)) && (status_level != 1)) {
        uVar3 = gettext("warning: invalid file offset after failed read");
        diagnose(0,uVar3);
      }
      _Var2 = lseek(0,__offset,1);
      if (-1 < _Var2) {
        return 1;
      }
      piVar1 = __errno_location();
      if (*piVar1 == 0) {
        uVar3 = gettext("cannot work around kernel bug after all");
        diagnose(0,uVar3);
      }
    }
  }
  else {
    if (input_seek_errno == 0x1d) {
      return 1;
    }
    piVar1 = __errno_location();
    *piVar1 = input_seek_errno;
  }
  uVar3 = quotearg_n_style_colon(0,3,input_file);
  uVar4 = gettext("%s: cannot seek");
  piVar1 = __errno_location();
  diagnose(*piVar1,uVar4,uVar3);
  return 0;
}

// Function: copy_simple
void copy_simple(void *param_1,size_t param_2) {
  size_t __n;
  size_t local_28;
  void *local_18;
  
  local_28 = param_2;
  local_18 = param_1;
  do {
    __n = output_blocksize - oc;
    if ((long)local_28 <= output_blocksize - oc) {
      __n = local_28;
    }
    memcpy((void *)(oc + obuf),local_18,__n);
    local_28 = local_28 - __n;
    local_18 = (void *)((long)local_18 + __n);
    oc = __n + oc;
    if (output_blocksize <= oc) {
      write_output();
    }
  } while (local_28 != 0);
  return;
}

// Function: copy_with_block
void copy_with_block(char *param_1,long param_2) {
  undefined *puVar1;
  char *pcVar2;
  char *local_20;
  long local_18;
  long local_10;
  
  local_20 = param_1;
  for (local_18 = param_2; local_18 != 0; local_18 = local_18 + -1) {
    if (*local_20 == newline_character) {
      if (col < conversion_blocksize) {
        for (local_10 = col; local_10 < conversion_blocksize; local_10 = local_10 + 1) {
          puVar1 = (undefined *)(obuf + oc);
          oc = oc + 1;
          *puVar1 = space_character;
          if (output_blocksize <= oc) {
            write_output();
          }
        }
      }
      col = 0;
    }
    else {
      if (col == conversion_blocksize) {
        r_truncate = r_truncate + 1;
      }
      else if (col < conversion_blocksize) {
        pcVar2 = (char *)(obuf + oc);
        oc = oc + 1;
        *pcVar2 = *local_20;
        if (output_blocksize <= oc) {
          write_output();
        }
      }
      col = col + 1;
    }
    local_20 = local_20 + 1;
  }
  return;
}

// Function: copy_with_unblock
void copy_with_unblock(long param_1,long param_2) {
  long lVar1;
  undefined *puVar2;
  char *pcVar3;
  char cVar4;
  long local_10;
  
  for (local_10 = 0; local_10 < param_2; local_10 = local_10 + 1) {
    cVar4 = *(char *)(param_1 + local_10);
    lVar1 = col + 1;
    if (col < conversion_blocksize) {
      col = lVar1;
      if (cVar4 == space_character) {
        pending_spaces_0 = pending_spaces_0 + 1;
      }
      else {
        for (; pending_spaces_0 != 0; pending_spaces_0 = pending_spaces_0 + -1) {
          pcVar3 = (char *)(obuf + oc);
          oc = oc + 1;
          *pcVar3 = space_character;
          if (output_blocksize <= oc) {
            write_output();
          }
        }
        pcVar3 = (char *)(obuf + oc);
        oc = oc + 1;
        *pcVar3 = cVar4;
        if (output_blocksize <= oc) {
          write_output();
        }
      }
    }
    else {
      pending_spaces_0 = 0;
      col = 0;
      local_10 = local_10 + -1;
      puVar2 = (undefined *)(obuf + oc);
      oc = oc + 1;
      *puVar2 = newline_character;
      if (output_blocksize <= oc) {
        write_output();
      }
    }
  }
  return;
}

// Function: set_fd_flags
void set_fd_flags(undefined4 param_1,uint param_2,undefined8 param_3) {
  bool bVar1;
  uint uVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  long in_FS_OFFSET;
  uint local_d0;
  undefined local_b8 [24];
  uint local_a0;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if ((param_2 & 0xfffdfeff) != 0) {
    uVar2 = rpl_fcntl(param_1,3);
    local_d0 = uVar2 | param_2 & 0xfffdfeff;
    bVar1 = true;
    if ((int)uVar2 < 0) {
      bVar1 = false;
    }
    else if (uVar2 != local_d0) {
      if ((local_d0 & 0x10000) != 0) {
        iVar3 = ifstat(param_1,local_b8);
        if (iVar3 == 0) {
          if (((local_d0 & 0x10000) != 0) && ((local_a0 & 0xf000) != 0x4000)) {
            piVar6 = __errno_location();
            *piVar6 = 0x14;
            bVar1 = false;
          }
        }
        else {
          bVar1 = false;
        }
        local_d0 = local_d0 & 0xfffeffff;
      }
      if ((bVar1) && (uVar2 != local_d0)) {
        iVar3 = rpl_fcntl(param_1,4,local_d0);
        if (iVar3 == -1) {
          bVar1 = false;
        }
      }
    }
    if (!bVar1) {
      uVar4 = quotearg_style(4,param_3);
      uVar5 = gettext("setting flags for %s");
      piVar6 = __errno_location();
      error(1,*piVar6,uVar5,uVar4);
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: dd_copy
undefined4 dd_copy(void) {
  undefined *puVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  size_t sVar8;
  int *piVar9;
  long in_FS_OFFSET;
  bool bVar10;
  undefined local_132;
  char local_131;
  int local_130;
  undefined4 local_12c;
  size_t local_128;
  void *local_120;
  size_t local_118;
  size_t local_110;
  long local_108;
  long local_100;
  long local_f8;
  long local_f0;
  size_t local_e8;
  long local_e0;
  long local_d8;
  size_t local_d0;
  long local_c8;
  __off_t local_c0;
  undefined local_b8 [24];
  uint local_a0;
  long local_88;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_110 = 0;
  local_12c = 0;
  if ((skip_records != 0) || (skip_bytes != 0)) {
    local_128 = input_blocksize * skip_records;
    if ((SEXT816((long)local_128) != SEXT816((long)input_blocksize) * SEXT816(skip_records)) ||
       (bVar10 = SCARRY8(local_128,skip_bytes), local_128 = local_128 + skip_bytes, bVar10)) {
      local_131 = '\x01';
    }
    else {
      local_131 = '\0';
    }
    local_f8 = input_offset;
    local_f0 = skip(0,input_file,skip_records,input_blocksize,&skip_bytes);
    if (((local_f0 != 0) ||
        ((-1 < input_offset && ((local_131 != '\0' || (input_offset - local_f8 != local_128)))))) &&
       (status_level != 1)) {
      uVar6 = quotearg_n_style_colon(0,3,input_file);
      uVar7 = gettext("%s: cannot skip to specified offset");
      diagnose(0,uVar7,uVar6);
    }
  }
  if ((seek_records != 0) || (seek_bytes != 0)) {
    local_128 = seek_bytes;
    local_108 = skip(1,output_file,seek_records,output_blocksize,&local_128);
    if ((local_108 != 0) || (local_128 != 0)) {
      sVar8 = local_128;
      if (local_108 != 0) {
        sVar8 = output_blocksize;
      }
      memset(obuf,0,sVar8);
      do {
        local_e8 = local_128;
        if (local_108 != 0) {
          local_e8 = output_blocksize;
        }
        sVar8 = iwrite(1,obuf,local_e8);
        if (local_e8 != sVar8) {
          uVar6 = quotearg_style(4,output_file);
          uVar7 = gettext("writing to %s");
          piVar9 = __errno_location();
          diagnose(*piVar9,uVar7,uVar6);
          quit(1);
        }
        if (local_108 == 0) {
          local_128 = 0;
        }
        else {
          local_108 = local_108 + -1;
        }
      } while ((local_108 != 0) || (local_128 != 0));
    }
  }
  if ((max_records == 0) && (uVar5 = local_12c, max_bytes == 0)) goto LAB_00104822;
  alloc_ibuf();
  alloc_obuf();
  local_130 = -1;
LAB_00103edc:
  if ((status_level == 4) && (local_e0 = gethrxtime(), next_time <= local_e0)) {
    print_xfer_stats(local_e0);
    next_time = next_time + 1000000000;
  }
  if (r_partial + r_full < (long)(max_records + (ulong)(max_bytes != 0))) {
    if (((conversions_mask & 0x400) != 0) && ((conversions_mask & 0x100) != 0)) {
      if ((conversions_mask & 0x18) == 0) {
        iVar4 = 0;
      }
      else {
        iVar4 = 0x20;
      }
      memset(ibuf,iVar4,input_blocksize);
    }
    if (r_partial + r_full < max_records) {
      local_118 = (*iread_fnc)(0,ibuf,input_blocksize);
    }
    else {
      local_118 = (*iread_fnc)(0,ibuf,max_bytes);
    }
    if (0 < (long)local_118) {
      advance_input_offset(local_118);
      if (i_nocache != 0) {
        invalidate_cache(0,local_118);
      }
      goto LAB_001041ce;
    }
    if (local_118 == 0) {
      i_nocache_eof = (i_nocache | i_nocache_eof) != 0;
      if ((o_nocache == '\0') || ((conversions_mask & 0x200) != 0)) {
        bVar2 = 0;
      }
      else {
        bVar2 = 1;
      }
      o_nocache_eof = (bVar2 | o_nocache_eof) != 0;
    }
    else {
      if (((conversions_mask & 0x100) == 0) || (status_level != 1)) {
        uVar6 = quotearg_style(4,input_file);
        uVar7 = gettext("error reading %s");
        piVar9 = __errno_location();
        diagnose(*piVar9,uVar7,uVar6);
      }
      if ((conversions_mask & 0x100) != 0) goto code_r0x00104132;
      local_12c = 1;
    }
  }
  if (-1 < local_130) {
    local_132 = (undefined)local_130;
    if ((conversions_mask & 8) == 0) {
      if ((conversions_mask & 0x10) == 0) {
        puVar1 = (undefined *)((long)obuf + oc);
        oc = oc + 1;
        *puVar1 = local_132;
        if ((long)output_blocksize <= oc) {
          write_output();
        }
      }
      else {
        copy_with_unblock(&local_132,1);
      }
    }
    else {
      copy_with_block(&local_132,1);
    }
  }
  if (((conversions_mask & 8) != 0) && (0 < col)) {
    for (local_100 = col; local_100 < conversion_blocksize; local_100 = local_100 + 1) {
      puVar1 = (undefined *)((long)obuf + oc);
      oc = oc + 1;
      *puVar1 = space_character;
      if ((long)output_blocksize <= oc) {
        write_output();
      }
    }
  }
  if ((col != 0) && ((conversions_mask & 0x10) != 0)) {
    puVar1 = (undefined *)((long)obuf + oc);
    oc = oc + 1;
    *puVar1 = newline_character;
    if ((long)output_blocksize <= oc) {
      write_output();
    }
  }
  if (oc != 0) {
    local_c8 = iwrite(1,obuf,oc);
    w_bytes = local_c8 + w_bytes;
    if (local_c8 != 0) {
      w_partial = w_partial + 1;
    }
    if (local_c8 != oc) {
      uVar6 = quotearg_style(4,output_file);
      uVar7 = gettext("error writing %s");
      piVar9 = __errno_location();
      diagnose(*piVar9,uVar7,uVar6);
      uVar5 = 1;
      goto LAB_00104822;
    }
  }
  if (final_op_was_seek != '\0') {
    iVar4 = ifstat(1,local_b8);
    if (iVar4 != 0) {
      uVar6 = quotearg_style(4,output_file);
      uVar7 = gettext("cannot fstat %s");
      piVar9 = __errno_location();
      diagnose(*piVar9,uVar7,uVar6);
      uVar5 = 1;
      goto LAB_00104822;
    }
    if (((((local_a0 & 0xf000) == 0x8000) && (local_c0 = lseek(1,0,1), -1 < local_c0)) &&
        (local_88 < local_c0)) && (iVar4 = iftruncate(1,local_c0), iVar4 != 0)) {
      uVar6 = quotearg_style(4,output_file);
      uVar7 = gettext("failed to truncate to %jd bytes in output file %s");
      piVar9 = __errno_location();
      diagnose(*piVar9,uVar7,local_c0,uVar6);
      uVar5 = 1;
      goto LAB_00104822;
    }
  }
  uVar5 = local_12c;
  if ((((conversions_mask & 0xc000) != 0) && (status_level == 4)) &&
     ((-1 < reported_w_bytes && (reported_w_bytes < w_bytes)))) {
    print_xfer_stats(0);
    uVar5 = local_12c;
  }
LAB_00104822:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar5;
code_r0x00104132:
  print_stats();
  local_d8 = input_blocksize - local_110;
  invalidate_cache(0,local_d8);
  cVar3 = advance_input_after_read_error(local_d8);
  if (cVar3 != '\x01') {
    local_12c = 1;
    input_seekable = 0;
    input_seek_errno = 0x1d;
  }
  if (((conversions_mask & 0x400) != 0) && (local_110 == 0)) {
    local_118 = 0;
LAB_001041ce:
    local_128 = local_118;
    if ((long)local_118 < (long)input_blocksize) {
      r_partial = r_partial + 1;
      local_110 = local_118;
      if ((conversions_mask & 0x400) != 0) {
        if ((conversions_mask & 0x100) == 0) {
          if ((conversions_mask & 0x18) == 0) {
            iVar4 = 0;
          }
          else {
            iVar4 = 0x20;
          }
          memset((void *)((long)ibuf + local_118),iVar4,input_blocksize - local_118);
        }
        local_128 = input_blocksize;
      }
    }
    else {
      r_full = r_full + 1;
      local_110 = 0;
    }
    if (ibuf == obuf) {
      local_d0 = iwrite(1,obuf,local_128);
      w_bytes = local_d0 + w_bytes;
      if (local_d0 != local_128) {
        uVar6 = quotearg_style(4,output_file);
        uVar7 = gettext("error writing %s");
        piVar9 = __errno_location();
        diagnose(*piVar9,uVar7,uVar6);
        uVar5 = 1;
        goto LAB_00104822;
      }
      if (local_128 == input_blocksize) {
        w_full = w_full + 1;
      }
      else {
        w_partial = w_partial + 1;
      }
    }
    else {
      if (translation_needed != '\0') {
        translate_buffer(ibuf,local_128);
      }
      if ((conversions_mask & 0x80) == 0) {
        local_120 = ibuf;
      }
      else {
        local_120 = (void *)swab_buffer(ibuf,&local_128,&local_130);
      }
      if ((conversions_mask & 8) == 0) {
        if ((conversions_mask & 0x10) == 0) {
          copy_simple(local_120,local_128);
        }
        else {
          copy_with_unblock(local_120,local_128);
        }
      }
      else {
        copy_with_block(local_120,local_128);
      }
    }
  }
  goto LAB_00103edc;
}

// Function: synchronize_output
undefined4 synchronize_output(void) {
  uint uVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined4 local_20;
  uint local_1c;
  
  local_1c = conversions_mask;
  local_20 = 0;
  uVar1 = conversions_mask & 0xffff3fff;
  uVar2 = conversions_mask & 0x4000;
  conversions_mask = uVar1;
  if ((uVar2 != 0) && (iVar3 = ifdatasync(1), iVar3 != 0)) {
    piVar4 = __errno_location();
    if ((*piVar4 != 0x26) && (piVar4 = __errno_location(), *piVar4 != 0x16)) {
      uVar5 = quotearg_style(4,output_file);
      uVar6 = gettext("fdatasync failed for %s");
      piVar4 = __errno_location();
      diagnose(*piVar4,uVar6,uVar5);
      local_20 = 1;
    }
    local_1c = local_1c | 0x8000;
  }
  if (((local_1c & 0x8000) != 0) && (iVar3 = ifsync(1), iVar3 != 0)) {
    uVar5 = quotearg_style(4,output_file);
    uVar6 = gettext("fsync failed for %s");
    piVar4 = __errno_location();
    diagnose(*piVar4,uVar6,uVar5);
    local_20 = 1;
  }
  return local_20;
}

// Function: main
void main(undefined4 param_1,undefined8 *param_2) {
  long lVar1;
  long lVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  int *piVar10;
  uint uVar11;
  long in_FS_OFFSET;
  bool bVar12;
  int local_118;
  int local_114;
  long local_e8;
  undefined local_b8 [24];
  uint local_a0;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  install_signal_handlers();
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit(maybe_close_stdout);
  iVar4 = getpagesize();
  page_size = (long)iVar4;
  uVar7 = proper_name_lite("Stuart Kemp","Stuart Kemp");
  uVar8 = proper_name_lite("David MacKenzie","David MacKenzie");
  uVar9 = proper_name_lite("Paul Rubin","Paul Rubin");
  parse_gnu_standard_options_only
            (param_1,param_2,&DAT_00106821,"coreutils",_Version,1,usage,uVar9,uVar8,uVar7,0);
  close_stdout_required = 0;
  for (local_118 = 0; local_118 < 0x100; local_118 = local_118 + 1) {
    trans_table[local_118] = (char)local_118;
  }
  scanargs(param_1,param_2);
  apply_translations();
  if (input_file == 0) {
    input_file = gettext("standard input");
    set_fd_flags(0,input_flags,input_file);
  }
  else {
    iVar4 = ifd_reopen(0,input_file,input_flags,0);
    if (iVar4 < 0) {
      uVar7 = quotearg_style(4,input_file);
      uVar8 = gettext("failed to open %s");
      piVar10 = __errno_location();
      error(1,*piVar10,uVar8,uVar7);
    }
  }
  input_offset = lseek(0,0,1);
  input_seekable = (byte)~(byte)((ulong)input_offset >> 0x38) >> 7;
  if (input_offset < 0) {
    input_offset = 0;
  }
  piVar10 = __errno_location();
  lVar2 = seek_records;
  lVar1 = output_blocksize;
  input_seek_errno = *piVar10;
  if (output_file == 0) {
    output_file = gettext("standard output");
    set_fd_flags(1,output_flags,output_file);
  }
  else {
    if ((conversions_mask & 0x1000) == 0) {
      uVar11 = 0x40;
    }
    else {
      uVar11 = 0;
    }
    if ((seek_records == 0) && ((conversions_mask & 0x200) == 0)) {
      uVar5 = 0x200;
    }
    else {
      uVar5 = 0;
    }
    uVar5 = uVar5 | uVar11 | output_flags | (int)conversions_mask >> 6 & 0x80U;
    local_e8 = output_blocksize * seek_records;
    if (((SEXT816(local_e8) != SEXT816(output_blocksize) * SEXT816(seek_records)) ||
        (bVar12 = SCARRY8(local_e8,seek_bytes), local_e8 = local_e8 + seek_bytes, bVar12)) &&
       ((conversions_mask & 0x200) == 0)) {
      uVar7 = gettext("offset too large: cannot truncate to a length of seek=%jd (%td-byte) blocks")
      ;
      error(1,0,uVar7,lVar2,lVar1);
    }
    if (((seek_records == 0) || (iVar4 = ifd_reopen(1,output_file,uVar5 | 2,0x1b6), iVar4 < 0)) &&
       (iVar4 = ifd_reopen(1,output_file,uVar5 | 1,0x1b6), iVar4 < 0)) {
      uVar7 = quotearg_style(4,output_file);
      uVar8 = gettext("failed to open %s");
      piVar10 = __errno_location();
      error(1,*piVar10,uVar8,uVar7);
    }
    if (((seek_records != 0) && ((conversions_mask & 0x200) == 0)) &&
       (iVar4 = iftruncate(1,local_e8), iVar4 != 0)) {
      piVar10 = __errno_location();
      iVar4 = *piVar10;
      iVar6 = ifstat(1,local_b8);
      if (iVar6 == 0) {
        if (((local_a0 & 0xf000) == 0x8000) || ((local_a0 & 0xf000) == 0x4000)) {
          uVar7 = quotearg_style(4,output_file);
          uVar8 = gettext("failed to truncate to %jd bytes in output file %s");
          diagnose(iVar4,uVar8,local_e8,uVar7);
        }
      }
      else {
        uVar7 = quotearg_style(4,output_file);
        uVar8 = gettext("cannot fstat %s");
        piVar10 = __errno_location();
        diagnose(*piVar10,uVar8,uVar7);
      }
    }
  }
  start_time = gethrxtime();
  next_time = start_time + 1000000000;
  local_114 = dd_copy();
  iVar4 = synchronize_output();
  if (iVar4 != 0) {
    local_114 = iVar4;
  }
  if ((max_records == 0) && (max_bytes == 0)) {
    if ((i_nocache != '\0') && (cVar3 = invalidate_cache(0,0), cVar3 != '\x01')) {
      uVar7 = quotearg_n_style_colon(0,3,input_file);
      uVar8 = gettext("failed to discard cache for: %s");
      piVar10 = __errno_location();
      diagnose(*piVar10,uVar8,uVar7);
      local_114 = 1;
    }
    if ((o_nocache != '\0') && (cVar3 = invalidate_cache(1,0), cVar3 != '\x01')) {
      uVar7 = quotearg_n_style_colon(0,3,output_file);
      uVar8 = gettext("failed to discard cache for: %s");
      piVar10 = __errno_location();
      diagnose(*piVar10,uVar8,uVar7);
      local_114 = 1;
    }
  }
  else {
    if ((i_nocache != '\0') || (i_nocache_eof != '\0')) {
      invalidate_cache(0,0);
    }
    if ((o_nocache != '\0') || (o_nocache_eof != '\0')) {
      invalidate_cache(1,0);
    }
  }
  finish_up();
                    /* WARNING: Subroutine does not return */
  exit(local_114);
}

