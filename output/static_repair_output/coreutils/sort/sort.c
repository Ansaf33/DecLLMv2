// Function: cleanup
void cleanup(EVP_PKEY_CTX *ctx) {
  undefined8 *local_10;
  
  for (local_10 = temphead; local_10 != (undefined8 *)0x0; local_10 = (undefined8 *)*local_10) {
    unlink((char *)((long)local_10 + 0xd));
  }
  temphead = (undefined8 *)0x0;
  return;
}

// Function: sighandler
void sighandler(EVP_PKEY_CTX *param_1) {
  cleanup(param_1);
  signal((int)param_1,(__sighandler_t)0x0);
  raise((int)param_1);
  return;
}

// Function: sort_die
void sort_die(undefined8 param_1,long param_2) {
  int *piVar1;
  undefined8 uVar2;
  
  piVar1 = __errno_location();
  if (((*piVar1 == 0x20) && (param_2 == 0)) && (default_SIGPIPE != '\0')) {
    sighandler(0xd);
  }
  if (param_2 == 0) {
    param_2 = gettext("standard output");
  }
  uVar2 = quotearg_n_style_colon(0,3,param_2);
  piVar1 = __errno_location();
                    /* WARNING: Subroutine does not return */
  error(2,*piVar1,"%s: %s",param_1,uVar2);
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... [FILE]...\n  or:  %s [OPTION]... --files0-from=F\n"
                            );
    printf(pcVar3,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Write sorted concatenation of all FILE(s) to standard output.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Ordering options:\n\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -b, --ignore-leading-blanks  ignore leading blanks\n  -d, --dictionary-order      consider only blanks and alphanumeric characters\n  -f, --ignore-case           fold lower case to upper case characters\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -g, --general-numeric-sort  compare according to general numerical value\n  -i, --ignore-nonprinting    consider only printable characters\n  -M, --month-sort            compare (unknown) < \'JAN\' < ... < \'DEC\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -h, --human-numeric-sort    compare human readable numbers (e.g., 2K 1G)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -n, --numeric-sort          compare according to string numerical value;\n                                see full documentation for supported strings\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -R, --random-sort           shuffle, but group identical keys.  See shuf(1)\n      --random-source=FILE    get random bytes from FILE\n  -r, --reverse               reverse the result of comparisons\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --sort=WORD             sort according to WORD:\n                                general-numeric -g, human-numeric -h, month -M,\n                                numeric -n, random -R, version -V\n  -V, --version-sort          natural sort of (version) numbers within text\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Other options:\n\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --batch-size=NMERGE   merge at most NMERGE inputs at once;\n                            for more use temp files\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -c, --check, --check=diagnose-first  check for sorted input; do not sort\n  -C, --check=quiet, --check=silent  like -c, but do not report first bad line\n      --compress-program=PROG  compress temporaries with PROG;\n                              decompress them with PROG -d\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --debug               annotate the part of the line used to sort, and\n                              warn about questionable usage to standard error\n      --files0-from=F       read input from the files specified by\n                            NUL-terminated names in file F;\n                            If F is - then read names from standard input\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -k, --key=KEYDEF          sort via a key; KEYDEF gives location and type\n  -m, --merge               merge already sorted files; do not sort\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -o, --output=FILE         write result to FILE instead of standard output\n  -s, --stable              stabilize sort by disabling last-resort comparison\n  -S, --buffer-size=SIZE    use SIZE for main memory buffer\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pcVar3 = (char *)gettext(
                            "  -t, --field-separator=SEP  use SEP instead of non-blank to blank transition\n  -T, --temporary-directory=DIR  use DIR for temporaries, not $TMPDIR or %s;\n                              multiple options specify multiple directories\n      --parallel=N          change the number of sorts run concurrently to N\n  -u, --unique              output only the first of lines with equal keys;\n                              with -c, check for strict ordering\n"
                            );
    printf(pcVar3,&DAT_0010d202);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -z, --zero-terminated     line delimiter is NUL, not newline\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nKEYDEF is F[.C][OPTS][,F[.C][OPTS]] for start and stop position, where F is a\nfield number and C a character position in the field; both are origin 1, and\nthe stop position defaults to the line\'s end.  If neither -t nor -b is in\neffect, characters in a field are counted from the beginning of the preceding\nwhitespace.  OPTS is one or more single-letter ordering options [bdfgiMhnRrV],\nwhich override global ordering options for that key.  If no key is given, use\nthe entire line as the key.  Use --debug to diagnose incorrect key usage.\n\nSIZE may be followed by the following multiplicative suffixes:\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "% 1% of memory, b 1, K 1024 (default), and so on for M, G, T, P, E, Z, Y, R, Q.\n\n*** WARNING ***\nThe locale specified by the environment affects sort order.\nSet LC_ALL=C to get the traditional sort order that uses\nnative byte values.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_0010d603);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: cs_enter
void cs_enter(long param_1) {
  int iVar1;
  
  iVar1 = pthread_sigmask(0,(__sigset_t *)&caught_signals,(__sigset_t *)(param_1 + 8));
  *(bool *)param_1 = iVar1 == 0;
  return;
}

// Function: cs_leave
void cs_leave(char *param_1) {
  if (*param_1 != '\0') {
    pthread_sigmask(2,(__sigset_t *)(param_1 + 8),(__sigset_t *)0x0);
  }
  return;
}

// Function: proctab_hasher
ulong proctab_hasher(long param_1,ulong param_2) {
  return (ulong)(long)*(int *)(param_1 + 8) % param_2;
}

// Function: proctab_comparator
undefined4 proctab_comparator(long param_1,long param_2) {
  return CONCAT31((int3)((uint)*(int *)(param_2 + 8) >> 8),
                  *(int *)(param_1 + 8) == *(int *)(param_2 + 8));
}

// Function: reap
__pid_t reap(int param_1) {
  char cVar1;
  __pid_t __pid;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  long in_FS_OFFSET;
  uint local_48;
  __pid_t local_44;
  undefined4 local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  __pid = param_1;
  if (param_1 == 0) {
    __pid = -1;
  }
  local_44 = waitpid(__pid,(int *)&local_48,(uint)(param_1 == 0));
  if (-1 < local_44) {
    if ((0 < local_44) && ((0 < param_1 || (cVar1 = delete_proc(local_44), cVar1 != '\0')))) {
      if (((local_48 & 0x7f) != 0) || ((local_48 & 0xff00) != 0)) {
        uVar2 = quotearg_style(4,compress_program);
        local_28 = gettext("%s [-d] terminated abnormally");
        local_38 = 2;
        local_34 = 0;
                    /* WARNING: Subroutine does not return */
        error(2,0,local_28,uVar2);
      }
      nprocs = nprocs + -1;
    }
    if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
      return local_44;
    }
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  uVar2 = quotearg_style(4,compress_program);
  uVar3 = gettext("waiting for %s [-d]");
  piVar4 = __errno_location();
  local_3c = *piVar4;
  local_40 = 2;
  local_30 = uVar3;
                    /* WARNING: Subroutine does not return */
  error(2,local_3c,uVar3,uVar2);
}

// Function: register_proc
void register_proc(long param_1) {
  long lVar1;
  
  if (proctab == 0) {
    proctab = hash_initialize(0x2f,0,proctab_hasher,proctab_comparator,0);
    if (proctab == 0) {
      xalloc_die();
    }
  }
  *(undefined *)(param_1 + 0xc) = 1;
  lVar1 = hash_insert(proctab,param_1);
  if (lVar1 == 0) {
    xalloc_die();
  }
  return;
}

// Function: delete_proc
bool delete_proc(undefined4 param_1) {
  long lVar1;
  long in_FS_OFFSET;
  undefined local_28 [8];
  undefined4 local_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_20 = param_1;
  lVar1 = hash_remove(proctab,local_28);
  if (lVar1 != 0) {
    *(undefined *)(lVar1 + 0xc) = 2;
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return lVar1 != 0;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: wait_proc
void wait_proc(undefined4 param_1) {
  char cVar1;
  
  cVar1 = delete_proc(param_1);
  if (cVar1 != '\0') {
    reap(param_1);
  }
  return;
}

// Function: reap_exited
void reap_exited(void) {
  int iVar1;
  
  do {
    if (nprocs < 1) {
      return;
    }
    iVar1 = reap(0);
  } while (iVar1 != 0);
  return;
}

// Function: reap_some
void reap_some(void) {
  reap(0xffffffff);
  reap_exited();
  return;
}

// Function: reap_all
void reap_all(void) {
  while (0 < nprocs) {
    reap(0xffffffff);
  }
  return;
}

// Function: exit_cleanup
void exit_cleanup(void) {
  EVP_PKEY_CTX *ctx;
  long in_FS_OFFSET;
  EVP_PKEY_CTX local_98 [136];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (temphead != 0) {
    ctx = local_98;
    cs_enter();
    cleanup(ctx);
    cs_leave(local_98);
  }
  close_stdout();
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: create_temp_file
undefined8 * create_temp_file(int *param_1,char param_2) {
  int iVar1;
  char *__s;
  int iVar2;
  size_t __n;
  void *__dest;
  int *piVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  long in_FS_OFFSET;
  undefined8 *local_d0;
  undefined local_a8 [136];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  __s = *(char **)(temp_dirs + temp_dir_index_10 * 8);
  __n = strlen(__s);
  local_d0 = (undefined8 *)xmalloc(__n + 0x20 & 0xfffffffffffffff8);
  __dest = (void *)((long)local_d0 + 0xd);
  memcpy(__dest,__s,__n);
  *(undefined8 *)(__n + (long)__dest) = slashbase_9._0_8_;
  *(undefined4 *)((undefined8 *)(__n + (long)__dest) + 1) = slashbase_9._8_4_;
  *local_d0 = 0;
  temp_dir_index_10 = temp_dir_index_10 + 1;
  if (temp_dir_index_10 == temp_dir_count) {
    temp_dir_index_10 = 0;
  }
  cs_enter(local_a8);
  iVar2 = mkostemp_safer(__dest,0x80000);
  if (-1 < iVar2) {
    *(undefined8 **)temptail = local_d0;
    temptail = (undefined *)local_d0;
  }
  piVar3 = __errno_location();
  iVar1 = *piVar3;
  cs_leave(local_a8);
  piVar3 = __errno_location();
  *piVar3 = iVar1;
  if (iVar2 < 0) {
    if (param_2 == '\x01') {
      piVar3 = __errno_location();
      if (*piVar3 == 0x18) {
        free(local_d0);
        local_d0 = (undefined8 *)0x0;
        goto LAB_00100e08;
      }
    }
    uVar4 = quotearg_style(4,__s);
    uVar5 = gettext("cannot create temporary file in %s");
    piVar3 = __errno_location();
                    /* WARNING: Subroutine does not return */
    error(2,*piVar3,uVar5,uVar4);
  }
LAB_00100e08:
  *param_1 = iVar2;
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_d0;
}

// Function: get_outstatus
undefined1 * get_outstatus(void) {
  int iVar1;
  int *piVar2;
  undefined1 *puVar3;
  
  if (outstat_errno_8 == 0) {
    iVar1 = fstat(1,(stat *)outstat_7);
    if (iVar1 == 0) {
      outstat_errno_8 = -1;
    }
    else {
      piVar2 = __errno_location();
      outstat_errno_8 = *piVar2;
    }
  }
  if (outstat_errno_8 < 0) {
    puVar3 = outstat_7;
  }
  else {
    puVar3 = (undefined1 *)0x0;
  }
  return puVar3;
}

// Function: stream_open
FILE * stream_open(char *param_1,char *param_2) {
  char cVar1;
  int iVar2;
  int *piVar3;
  long lVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  FILE *local_30;
  
  if (*param_2 == 'r') {
    cVar1 = streq(param_1,&DAT_0010d83b);
    if (cVar1 == '\0') {
      iVar2 = open(param_1,0x80000);
      if (iVar2 < 0) {
        local_30 = (FILE *)0x0;
      }
      else {
        local_30 = fdopen(iVar2,param_2);
      }
    }
    else {
      have_read_stdin = 1;
      local_30 = _stdin;
    }
    fadvise(local_30,2);
  }
  else {
    if (*param_2 != 'w') {
                    /* WARNING: Subroutine does not return */
      __assert_fail("!\"unexpected mode passed to stream_open\"",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/sort.c",
                    0x3d3,"stream_open");
    }
    if ((param_1 != (char *)0x0) && (iVar2 = ftruncate(1,0), iVar2 != 0)) {
      piVar3 = __errno_location();
      iVar2 = *piVar3;
      lVar4 = get_outstatus();
      if ((lVar4 == 0) || ((*(uint *)(lVar4 + 0x18) & 0xf000) == 0x8000)) {
        uVar5 = quotearg_n_style_colon(0,3,param_1);
        uVar6 = gettext("%s: error truncating");
                    /* WARNING: Subroutine does not return */
        error(2,iVar2,uVar6,uVar5);
      }
    }
    local_30 = _stdout;
  }
  return local_30;
}

// Function: xfopen
long xfopen(undefined8 param_1,undefined8 param_2) {
  long lVar1;
  undefined8 uVar2;
  
  lVar1 = stream_open(param_1,param_2);
  if (lVar1 == 0) {
    uVar2 = gettext("open failed");
    sort_die(uVar2,param_1);
  }
  return lVar1;
}

// Function: xfclose
void xfclose(FILE *param_1,undefined8 param_2) {
  int iVar1;
  undefined8 uVar2;
  
  iVar1 = fileno_unlocked(param_1);
  if (iVar1 == 0) {
    clearerr_unlocked(param_1);
  }
  else if (iVar1 == 1) {
    iVar1 = fflush_unlocked(param_1);
    if (iVar1 != 0) {
      uVar2 = gettext("fflush failed");
      sort_die(uVar2,param_2);
    }
  }
  else {
    iVar1 = rpl_fclose(param_1);
    if (iVar1 != 0) {
      uVar2 = gettext("close failed");
      sort_die(uVar2,param_2);
    }
  }
  return;
}

// Function: move_fd
void move_fd(int param_1,int param_2) {
  if (param_1 != param_2) {
    dup2(param_1,param_2);
    close(param_1);
  }
  return;
}

// Function: posix_spawn_file_actions_move_fd
int posix_spawn_file_actions_move_fd(posix_spawn_file_actions_t *param_1,int param_2,int param_3) {
  int local_c;
  
  local_c = 0;
  if (param_2 != param_3) {
    local_c = posix_spawn_file_actions_adddup2(param_1,param_2,param_3);
    if (local_c == 0) {
      local_c = posix_spawn_file_actions_addclose(param_1,param_2);
    }
  }
  return local_c;
}

// Function: get_resolved_compress_program
long get_resolved_compress_program(void) {
  char *pcVar1;
  
  if (resolved_compress_program_cache_5 == 0) {
    pcVar1 = getenv("PATH");
    resolved_compress_program_cache_5 = find_in_given_path(compress_program,pcVar1,0,0);
  }
  return resolved_compress_program_cache_5;
}

// Function: pipe_child
int pipe_child(pid_t *param_1,int *param_2,undefined4 param_3,char param_4,long param_5) {
  bool bVar1;
  undefined8 uVar2;
  int iVar3;
  char *__file;
  int *piVar4;
  long in_FS_OFFSET;
  long local_2a8;
  int local_288;
  double local_280;
  char *local_268;
  undefined *local_260;
  undefined8 local_258;
  posix_spawn_file_actions_t local_248;
  undefined local_1f8 [144];
  posix_spawnattr_t local_168;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_280 = DAT_0010e608;
  __file = (char *)get_resolved_compress_program();
  if (__file == (char *)0x0) {
    piVar4 = __errno_location();
    local_288 = *piVar4;
  }
  else {
    local_288 = posix_spawnattr_init(&local_168);
    if (local_288 == 0) {
      local_288 = posix_spawnattr_setflags(&local_168,0x40);
      if ((local_288 == 0) &&
         (local_288 = posix_spawn_file_actions_init(&local_248), local_288 == 0)) {
        iVar3 = rpl_pipe2(param_2,0x80000);
        if (iVar3 < 0) {
          piVar4 = __errno_location();
          local_288 = *piVar4;
          posix_spawnattr_destroy(&local_168);
          posix_spawn_file_actions_destroy(&local_248);
        }
        else {
          local_288 = posix_spawn_file_actions_addclose(&local_248,0);
          if ((local_288 == 0) &&
             (local_288 = posix_spawn_file_actions_addclose(&local_248,1), local_288 == 0)) {
            if (param_4 == '\0') {
              local_288 = posix_spawn_file_actions_addclose(&local_248,param_2[1]);
              if (((local_288 == 0) &&
                  (local_288 = posix_spawn_file_actions_move_fd(&local_248,param_3,1),
                  local_288 == 0)) &&
                 (local_288 = posix_spawn_file_actions_move_fd(&local_248,*param_2,0),
                 local_288 == 0)) {
                bVar1 = false;
              }
              else {
                bVar1 = true;
              }
            }
            else {
              local_288 = posix_spawn_file_actions_addclose(&local_248,*param_2);
              if (((local_288 == 0) &&
                  (local_288 = posix_spawn_file_actions_move_fd(&local_248,param_3,0),
                  local_288 == 0)) &&
                 (local_288 = posix_spawn_file_actions_move_fd(&local_248,param_2[1],1),
                 local_288 == 0)) {
                bVar1 = false;
              }
              else {
                bVar1 = true;
              }
            }
            if (!bVar1) {
              if (param_4 == '\0') {
                local_260 = (undefined *)0x0;
              }
              else {
                local_260 = &DAT_0010d8fd;
              }
              local_258 = 0;
              local_2a8 = param_5;
              local_268 = __file;
              if (nmerge + 1U < nprocs) {
                reap_some();
                local_2a8 = param_5;
              }
              while (local_2a8 != 0) {
                cs_enter(local_1f8);
                uVar2 = temphead;
                temphead = 0;
                local_288 = posix_spawnp(param_1,__file,&local_248,&local_168,&local_268,_environ);
                temphead = uVar2;
                cs_leave(local_1f8);
                if (local_288 != 0xb) break;
                xnanosleep(local_280);
                local_280 = local_280 + local_280;
                reap_exited();
                local_2a8 = local_2a8 + -1;
              }
              posix_spawnattr_destroy(&local_168);
              posix_spawn_file_actions_destroy(&local_248);
              if (local_288 == 0) {
                nprocs = nprocs + 1;
              }
              else {
                close(*param_2);
                close(param_2[1]);
              }
              goto LAB_001016c0;
            }
          }
          close(*param_2);
          close(param_2[1]);
          posix_spawnattr_destroy(&local_168);
          posix_spawn_file_actions_destroy(&local_248);
        }
      }
      else {
        posix_spawnattr_destroy(&local_168);
      }
    }
  }
LAB_001016c0:
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_288;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: maybe_create_temp
long maybe_create_temp(FILE **param_1,undefined param_2) {
  undefined8 uVar1;
  FILE *pFVar2;
  long lVar3;
  long in_FS_OFFSET;
  int local_48;
  int local_44;
  undefined4 local_40;
  int local_3c;
  long local_38;
  undefined8 local_30;
  int local_28;
  int local_24;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_38 = create_temp_file(&local_48,param_2);
  if (local_38 == 0) {
    lVar3 = 0;
  }
  else {
    *(undefined *)(local_38 + 0xc) = 0;
    if (compress_program != 0) {
      local_44 = pipe_child(local_38 + 8,&local_28,local_48,0,4);
      if (local_44 == 0) {
        close(local_48);
        close(local_28);
        local_48 = local_24;
        register_proc(local_38);
      }
      else if (local_44 != last_result_4) {
        uVar1 = quotearg_style(4,compress_program);
        local_30 = gettext("could not run compress program %s");
        local_40 = 0;
        local_3c = local_44;
                    /* WARNING: Subroutine does not return */
        error(0,local_44,local_30,uVar1);
      }
      last_result_4 = local_44;
    }
    pFVar2 = fdopen(local_48,"w");
    *param_1 = pFVar2;
    lVar3 = local_38;
    if (*param_1 == (FILE *)0x0) {
      lVar3 = local_38 + 0xd;
      uVar1 = gettext("couldn\'t create temporary file");
      sort_die(uVar1,lVar3);
      lVar3 = local_38;
    }
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return lVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: create_temp
void create_temp(undefined8 param_1) {
  maybe_create_temp(param_1,0);
  return;
}

// Function: open_temp
FILE * open_temp(long param_1) {
  undefined8 uVar1;
  int *piVar2;
  FILE *pFVar3;
  long in_FS_OFFSET;
  undefined4 local_50;
  int local_4c;
  int local_48;
  int local_44;
  undefined4 local_40;
  int local_3c;
  FILE *local_38;
  undefined8 local_30;
  int local_28;
  int local_24;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_38 = (FILE *)0x0;
  if (*(char *)(param_1 + 0xc) == '\x01') {
    wait_proc(*(undefined4 *)(param_1 + 8));
  }
  local_4c = open((char *)(param_1 + 0xd),0);
  if (local_4c < 0) {
    pFVar3 = (FILE *)0x0;
  }
  else {
    local_48 = pipe_child(&local_50,&local_28,local_4c,1,9);
    if (local_48 == 0) {
      *(undefined4 *)(param_1 + 8) = local_50;
      register_proc(param_1);
      close(local_4c);
      close(local_24);
      local_38 = fdopen(local_28,"r");
      pFVar3 = local_38;
      if (local_38 == (FILE *)0x0) {
        piVar2 = __errno_location();
        local_44 = *piVar2;
        close(local_28);
        piVar2 = __errno_location();
        *piVar2 = local_44;
        pFVar3 = local_38;
      }
    }
    else {
      if (local_48 != 0x18) {
        uVar1 = quotearg_style(4,compress_program);
        local_30 = gettext("could not run compress program %s -d");
        local_40 = 2;
        local_3c = local_48;
                    /* WARNING: Subroutine does not return */
        error(2,local_48,local_30,uVar1);
      }
      close(local_4c);
      piVar2 = __errno_location();
      *piVar2 = 0x18;
      pFVar3 = local_38;
    }
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return pFVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: add_temp_dir
void add_temp_dir(undefined8 param_1) {
  long lVar1;
  
  if (temp_dir_count == temp_dir_alloc) {
    temp_dirs = xpalloc(temp_dirs,&temp_dir_alloc,1,0xffffffffffffffff,8);
  }
  lVar1 = temp_dir_count * 8;
  temp_dir_count = temp_dir_count + 1;
  *(undefined8 *)(temp_dirs + lVar1) = param_1;
  return;
}

// Function: zaptemp
void zaptemp(char *param_1) {
  int iVar1;
  long **__ptr;
  long *plVar2;
  int iVar3;
  int *piVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  long in_FS_OFFSET;
  long **local_c8;
  undefined local_a8 [136];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_c8 = (long **)&temphead;
  __ptr = local_c8;
  do {
    local_c8 = __ptr;
    __ptr = (long **)*local_c8;
  } while (param_1 != (char *)((long)__ptr + 0xd));
  if (*(char *)((long)__ptr + 0xc) == '\x01') {
    wait_proc(*(undefined4 *)(__ptr + 1));
  }
  plVar2 = *__ptr;
  cs_enter(local_a8);
  iVar3 = unlink(param_1);
  piVar4 = __errno_location();
  iVar1 = *piVar4;
  *local_c8 = plVar2;
  cs_leave(local_a8);
  if (iVar3 == 0) {
    if (plVar2 == (long *)0x0) {
      temptail = (undefined *)local_c8;
    }
    free(__ptr);
    if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
      return;
    }
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  uVar5 = quotearg_n_style_colon(0,3,param_1);
  uVar6 = gettext("warning: cannot remove: %s");
                    /* WARNING: Subroutine does not return */
  error(0,iVar1,uVar6,uVar5);
}

// Function: struct_month_cmp
void struct_month_cmp(char **param_1,char **param_2) {
  strcmp(*param_1,*param_2);
  return;
}

// Function: inittables
void inittables(void) {
  ushort *puVar1;
  undefined uVar2;
  byte bVar3;
  int iVar4;
  char *__s;
  size_t sVar5;
  long lVar6;
  ushort **ppuVar7;
  ulong local_48;
  ulong local_40;
  long local_38;
  
  local_48 = 0;
  do {
    if (0xff < local_48) {
      if (hard_LC_TIME != '\0') {
        for (local_48 = 0; local_48 < 0xc; local_48 = local_48 + 1) {
          __s = (char *)rpl_nl_langinfo((int)local_48 + 0x2000e);
          sVar5 = strlen(__s);
          lVar6 = xmalloc(sVar5 + 1);
          *(long *)(monthtab + local_48 * 0x10) = lVar6;
          *(int *)(monthtab + local_48 * 0x10 + 8) = (int)local_48 + 1;
          local_38 = 0;
          for (local_40 = 0; local_40 < sVar5; local_40 = local_40 + 1) {
            ppuVar7 = __ctype_b_loc();
            puVar1 = *ppuVar7;
            bVar3 = to_uchar((int)__s[local_40]);
            if ((puVar1[bVar3] & 1) == 0) {
              bVar3 = to_uchar((int)__s[local_40]);
              *(undefined1 *)(lVar6 + local_38) = fold_toupper[(int)(uint)bVar3];
              local_38 = local_38 + 1;
            }
          }
          *(undefined *)(local_38 + lVar6) = 0;
        }
        qsort(monthtab,0xc,0x10,struct_month_cmp);
      }
      return;
    }
    iVar4 = (int)local_48;
    if (local_48 == 10) {
LAB_00101cb2:
      uVar2 = 1;
    }
    else {
      ppuVar7 = __ctype_b_loc();
      if (((*ppuVar7)[iVar4] & 1) != 0) goto LAB_00101cb2;
      uVar2 = 0;
    }
    blanks[local_48] = uVar2;
    if (blanks[local_48] == '\x01') {
LAB_00101d12:
      uVar2 = 0;
    }
    else {
      ppuVar7 = __ctype_b_loc();
      if (((*ppuVar7)[iVar4] & 8) != 0) goto LAB_00101d12;
      uVar2 = 1;
    }
    nondictionary[local_48] = uVar2;
    ppuVar7 = __ctype_b_loc();
    nonprinting[local_48] = ((*ppuVar7)[iVar4] & 0x4000) == 0;
    iVar4 = toupper(iVar4);
    fold_toupper[local_48] = (char)iVar4;
    local_48 = local_48 + 1;
  } while( true );
}

// Function: specify_nmerge
void specify_nmerge(int param_1,char param_2,undefined8 param_3) {
  undefined8 uVar1;
  int iVar2;
  uint uVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  int local_88;
  ulong local_60 [2];
  undefined8 local_50;
  undefined8 local_40;
  int local_38;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_88 = xstrtoumax(param_3,0,10,local_60,&DAT_0010c60c);
  iVar2 = getrlimit(RLIMIT_NOFILE,(rlimit *)&local_38);
  if (iVar2 == 0) {
    uVar3 = local_38 - 3;
  }
  else {
    uVar3 = 0x11;
  }
  if (local_88 == 0) {
    nmerge = (uint)local_60[0];
    if ((local_60[0] & 0xffffffff) == local_60[0]) {
      if (nmerge < 2) {
        uVar4 = quote(param_3);
        uVar1 = *(undefined8 *)(long_options + (long)param_1 * 0x20);
        local_50 = gettext("invalid --%s argument %s");
                    /* WARNING: Subroutine does not return */
        error(0,0,local_50,uVar1,uVar4);
      }
      if (nmerge <= uVar3) goto code_r0x001021a1;
      local_88 = 1;
    }
    else {
      local_88 = 1;
    }
  }
  if (local_88 == 1) {
    uVar4 = quote(param_3);
    uVar1 = *(undefined8 *)(long_options + (long)param_1 * 0x20);
    local_40 = gettext("--%s argument %s too large");
                    /* WARNING: Subroutine does not return */
    error(0,0,local_40,uVar1,uVar4);
  }
  xstrtol_fatal(local_88,param_1,(int)param_2,long_options,param_3);
code_r0x001021a1:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: specify_sort_size
void specify_sort_size(undefined4 param_1,char param_2,undefined8 param_3) {
  char cVar1;
  ulong uVar2;
  ulong uVar3;
  long in_FS_OFFSET;
  double dVar4;
  double dVar5;
  int local_2c;
  ulong local_28;
  char *local_20;
  double local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_2c = xstrtoumax(param_3,&local_20,10,&local_28,"EgGkKmMPQRtTYZ");
  if (((local_2c == 0) && (cVar1 = c_isdigit((int)local_20[-1]), cVar1 != '\0')) &&
     (uVar3 = local_28 << 10, uVar2 = local_28 >> 0x36, local_28 = uVar3, uVar2 != 0)) {
    local_2c = 1;
  }
  if (((local_2c == 2) && (cVar1 = c_isdigit((int)local_20[-1]), cVar1 != '\0')) &&
     (local_20[1] == '\0')) {
    if (*local_20 == '%') {
      dVar4 = (double)physmem_total();
      if ((long)local_28 < 0) {
        dVar5 = (double)(local_28 >> 1 | (ulong)((uint)local_28 & 1));
        dVar5 = dVar5 + dVar5;
      }
      else {
        dVar5 = (double)local_28;
      }
      local_18 = (dVar5 * dVar4) / DAT_0010e610;
      if (DAT_0010e618 <= local_18) {
        local_2c = 1;
      }
      else {
        if (DAT_0010e620 <= local_18) {
          local_28 = (long)(local_18 - DAT_0010e620) ^ 0x8000000000000000;
        }
        else {
          local_28 = (ulong)local_18;
        }
        local_2c = 0;
      }
    }
    else if (*local_20 == 'b') {
      local_2c = 0;
    }
  }
  if (local_2c == 0) {
    if ((sort_size <= local_28) && (sort_size = local_28, local_28 <= (ulong)nmerge * 0x22)) {
      sort_size = (ulong)nmerge * 0x22;
    }
  }
  else {
    xstrtol_fatal(local_2c,param_1,(int)param_2,long_options,param_3);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: specify_nthreads
long specify_nthreads(undefined4 param_1,char param_2,undefined8 param_3) {
  int iVar1;
  long in_FS_OFFSET;
  long local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  iVar1 = xstrtoumax(param_3,0,10,&local_20,&DAT_0010c60c);
  if (iVar1 == 1) {
    local_20 = -1;
  }
  else {
    if (iVar1 != 0) {
      xstrtol_fatal(iVar1,param_1,(int)param_2,long_options,param_3);
    }
    if (local_20 == 0) {
      local_18 = gettext("number in parallel must be nonzero");
                    /* WARNING: Subroutine does not return */
      error(2,0,local_18);
    }
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_20;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: default_sort_size
ulong default_sort_size(void) {
  int iVar1;
  long in_FS_OFFSET;
  double dVar2;
  double dVar3;
  double dVar4;
  ulong local_48;
  rlimit local_28;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_48 = 0xffffffffffffffff;
  iVar1 = getrlimit(RLIMIT_DATA,&local_28);
  if ((iVar1 == 0) && (local_28.rlim_cur != 0xffffffffffffffff)) {
    local_48 = local_28.rlim_cur;
  }
  iVar1 = getrlimit(RLIMIT_AS,&local_28);
  if ((iVar1 == 0) && (local_28.rlim_cur < local_48)) {
    local_48 = local_28.rlim_cur;
  }
  local_48 = local_48 >> 1;
  iVar1 = getrlimit(__RLIMIT_RSS,&local_28);
  if ((iVar1 == 0) &&
     ((local_28.rlim_cur & 0xfffffffffffffff0) - (local_28.rlim_cur >> 4) < local_48)) {
    local_48 = (local_28.rlim_cur & 0xfffffffffffffff0) - (local_28.rlim_cur >> 4);
  }
  dVar2 = (double)physmem_available();
  dVar3 = (double)physmem_total();
  if (dVar2 <= dVar3 / DAT_0010e628) {
    dVar2 = dVar3 / DAT_0010e628;
  }
  if ((long)local_48 < 0) {
    dVar4 = (double)(local_48 >> 1 | (ulong)((uint)local_48 & 1));
    dVar4 = dVar4 + dVar4;
  }
  else {
    dVar4 = (double)local_48;
  }
  if (dVar3 * DAT_0010e630 < dVar4) {
    dVar3 = DAT_0010e630 * dVar3;
    if (DAT_0010e620 <= dVar3) {
      local_48 = (long)(dVar3 - DAT_0010e620) ^ 0x8000000000000000;
    }
    else {
      local_48 = (ulong)dVar3;
    }
  }
  if ((long)local_48 < 0) {
    dVar3 = (double)(local_48 >> 1 | (ulong)((uint)local_48 & 1));
    dVar3 = dVar3 + dVar3;
  }
  else {
    dVar3 = (double)local_48;
  }
  if (dVar2 < dVar3) {
    if (DAT_0010e620 <= dVar2) {
      local_48 = (long)(dVar2 - DAT_0010e620) ^ 0x8000000000000000;
    }
    else {
      local_48 = (ulong)dVar2;
    }
  }
  if (local_48 <= (ulong)nmerge * 0x22) {
    local_48 = (ulong)nmerge * 0x22;
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_48;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: sort_buffer_size
long sort_buffer_size(long param_1,ulong param_2,long param_3,ulong param_4,long param_5) {
  undefined8 uVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  ulong uVar5;
  long lVar6;
  long in_FS_OFFSET;
  bool bVar7;
  long local_e0;
  ulong local_d8;
  ulong local_d0;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_e0 = param_5 + 2;
  for (local_d8 = 0; lVar6 = local_e0, local_d8 < param_4; local_d8 = local_d8 + 1) {
    if (local_d8 < param_2) {
      iVar3 = fileno_unlocked(*(FILE **)(param_1 + local_d8 * 8));
      iVar3 = fstat(iVar3,&local_b8);
      bVar7 = iVar3 != 0;
    }
    else {
      cVar2 = streq(*(undefined8 *)(param_3 + local_d8 * 8),&DAT_0010d83b);
      if (cVar2 == '\0') {
        iVar3 = stat(*(char **)(param_3 + local_d8 * 8),&local_b8);
        bVar7 = iVar3 != 0;
      }
      else {
        iVar3 = fstat(0,&local_b8);
        bVar7 = iVar3 != 0;
      }
    }
    if (bVar7) {
      uVar1 = *(undefined8 *)(param_3 + local_d8 * 8);
      uVar4 = gettext("stat failed");
      sort_die(uVar4,uVar1);
    }
    cVar2 = usable_st_size(&local_b8);
    if ((cVar2 == '\0') || (local_b8.st_size < 1)) {
      lVar6 = sort_size;
      if (sort_size != 0) break;
      local_d0 = 0x20000;
    }
    else {
      local_d0 = local_b8.st_size;
    }
    if ((size_bound_3 == 0) && (size_bound_3 = sort_size, sort_size == 0)) {
      size_bound_3 = default_sort_size();
    }
    uVar5 = local_d0 * (param_5 + 1U) + 1;
    lVar6 = size_bound_3;
    if ((uVar5 / (param_5 + 1U) != local_d0) || ((ulong)(size_bound_3 - local_e0) <= uVar5)) break;
    local_e0 = local_e0 + uVar5;
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return lVar6;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: initbuf
void initbuf(void **param_1,void *param_2,void *param_3) {
  void *pvVar1;
  void *local_20;
  
  local_20 = param_3;
  do {
    local_20 = (void *)(((ulong)local_20 & 0xffffffffffffffe0) + 0x20);
    pvVar1 = malloc((size_t)local_20);
    *param_1 = pvVar1;
    if (*param_1 != (void *)0x0) goto code_r0x00102a65;
    local_20 = (void *)((ulong)local_20 >> 1);
  } while ((void *)((long)param_2 + 1) < local_20);
  xalloc_die();
code_r0x00102a65:
  param_1[5] = param_2;
  param_1[3] = local_20;
  param_1[2] = (void *)0x0;
  param_1[4] = param_1[2];
  param_1[1] = param_1[4];
  *(undefined *)(param_1 + 6) = 0;
  return;
}

// Function: buffer_linelim
long buffer_linelim(long *param_1) {
  return param_1[3] + *param_1;
}

// Function: begfield
char * begfield(char **param_1,long *param_2) {
  long lVar1;
  ulong uVar2;
  byte bVar3;
  char *pcVar4;
  char *local_30;
  long local_28;
  
  local_30 = *param_1;
  pcVar4 = local_30 + (long)(param_1[1] + -1);
  local_28 = *param_2;
  uVar2 = param_2[1];
  if (tab == 0x80) {
    while ((local_30 < pcVar4 && (lVar1 = local_28 + -1, local_28 != 0))) {
      while ((local_30 < pcVar4 &&
             (bVar3 = to_uchar((int)*local_30), blanks[(int)(uint)bVar3] != '\0'))) {
        local_30 = local_30 + 1;
      }
      while ((local_28 = lVar1, local_30 < pcVar4 &&
             (bVar3 = to_uchar((int)*local_30), blanks[(int)(uint)bVar3] != '\x01'))) {
        local_30 = local_30 + 1;
      }
    }
  }
  else {
    while ((local_30 < pcVar4 && (lVar1 = local_28 + -1, local_28 != 0))) {
      for (; (local_30 < pcVar4 && (*local_30 != tab)); local_30 = local_30 + 1) {
      }
      local_28 = lVar1;
      if (local_30 < pcVar4) {
        local_30 = local_30 + 1;
      }
    }
  }
  if (*(char *)(param_2 + 6) != '\0') {
    while ((local_30 < pcVar4 &&
           (bVar3 = to_uchar((int)*local_30), blanks[(int)(uint)bVar3] != '\0'))) {
      local_30 = local_30 + 1;
    }
  }
  if (uVar2 < (ulong)((long)pcVar4 - (long)local_30)) {
    pcVar4 = local_30 + uVar2;
  }
  local_30 = pcVar4;
  return local_30;
}

// Function: limfield
char * limfield(char **param_1,long param_2) {
  long lVar1;
  ulong uVar2;
  char *pcVar3;
  byte bVar4;
  char *pcVar5;
  char *local_30;
  long local_28;
  
  local_30 = *param_1;
  pcVar5 = local_30 + (long)(param_1[1] + -1);
  local_28 = *(long *)(param_2 + 0x10);
  uVar2 = *(ulong *)(param_2 + 0x18);
  if (uVar2 == 0) {
    local_28 = local_28 + 1;
  }
  if (tab == 0x80) {
    while ((local_30 < pcVar5 && (lVar1 = local_28 + -1, local_28 != 0))) {
      while ((local_30 < pcVar5 &&
             (bVar4 = to_uchar((int)*local_30), blanks[(int)(uint)bVar4] != '\0'))) {
        local_30 = local_30 + 1;
      }
      while ((local_28 = lVar1, local_30 < pcVar5 &&
             (bVar4 = to_uchar((int)*local_30), blanks[(int)(uint)bVar4] != '\x01'))) {
        local_30 = local_30 + 1;
      }
    }
  }
  else {
    while ((local_30 < pcVar5 && (lVar1 = local_28 + -1, local_28 != 0))) {
      for (; (local_30 < pcVar5 && (*local_30 != tab)); local_30 = local_30 + 1) {
      }
      local_28 = lVar1;
      if ((local_30 < pcVar5) && ((lVar1 != 0 || (uVar2 != 0)))) {
        local_30 = local_30 + 1;
      }
    }
  }
  pcVar3 = local_30;
  if (uVar2 != 0) {
    if (*(char *)(param_2 + 0x31) != '\0') {
      while ((local_30 < pcVar5 &&
             (bVar4 = to_uchar((int)*local_30), blanks[(int)(uint)bVar4] != '\0'))) {
        local_30 = local_30 + 1;
      }
    }
    pcVar3 = pcVar5;
    if (uVar2 < (ulong)((long)pcVar5 - (long)local_30)) {
      pcVar3 = local_30 + uVar2;
    }
  }
  local_30 = pcVar3;
  return local_30;
}

// Function: fillbuf
undefined8 fillbuf(char **param_1,FILE *param_2,undefined8 param_3) {
  char **ppcVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  undefined8 uVar5;
  char *pcVar6;
  long lVar7;
  char *pcVar8;
  long in_FS_OFFSET;
  ulong local_78;
  char *local_70;
  char *local_68;
  char **local_60;
  char *local_58;
  char *local_50;
  char *local_48;
  long *local_40;
  char *local_38;
  long local_30;
  ulong local_28;
  size_t local_20;
  char *local_18;
  long local_10;
  
  cVar2 = eolchar;
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_40 = keylist;
  local_38 = param_1[5];
  local_70 = merge_buffer_size + -0x22;
  if (*(char *)(param_1 + 6) == '\0') {
    if (param_1[1] != param_1[4]) {
      memmove(*param_1,*param_1 + ((long)param_1[1] - (long)param_1[4]),(size_t)param_1[4]);
      param_1[1] = param_1[4];
      param_1[2] = (char *)0x0;
    }
    do {
      local_68 = param_1[1] + (long)*param_1;
      local_30 = buffer_linelim(param_1);
      local_60 = (char **)(local_30 + (long)param_1[2] * -0x20);
      local_58 = (char *)((local_30 - (long)param_1[2] * (long)local_38) - (long)local_68);
      if (param_1[2] == (char *)0x0) {
        local_50 = *param_1;
      }
      else {
        local_50 = local_60[1] + (long)*local_60;
      }
      do {
        if (local_58 <= local_38 + 1) break;
        local_28 = (ulong)(local_58 + -1) / (ulong)(local_38 + 1);
        local_20 = fread_unlocked(local_68,1,local_28,param_2);
        local_48 = local_68 + local_20;
        local_58 = local_58 + -local_20;
        if (local_20 != local_28) {
          iVar4 = ferror_unlocked(param_2);
          if (iVar4 != 0) {
            uVar5 = gettext("read failed");
            sort_die(uVar5,param_3);
          }
          iVar4 = feof_unlocked(param_2);
          if (iVar4 != 0) {
            *(undefined *)(param_1 + 6) = 1;
            if (local_48 == *param_1) {
              uVar5 = 0;
              goto LAB_00103355;
            }
            if ((local_50 != local_48) && (cVar2 != local_48[-1])) {
              *local_48 = cVar2;
              local_48 = local_48 + 1;
            }
          }
        }
        while (pcVar8 = (char *)memchr(local_68,(int)cVar2,(long)local_48 - (long)local_68),
              local_18 = pcVar8, pcVar8 != (char *)0x0) {
          *pcVar8 = '\0';
          local_68 = pcVar8 + 1;
          ppcVar1 = local_60 + -4;
          *ppcVar1 = local_50;
          local_60[-3] = local_68 + -(long)local_50;
          pcVar6 = local_60[-3];
          if (local_60[-3] <= local_70) {
            pcVar6 = local_70;
          }
          local_58 = local_58 + -(long)local_38;
          local_70 = pcVar6;
          local_60 = ppcVar1;
          if (local_40 != (long *)0x0) {
            if (local_40[2] != -1) {
              pcVar8 = (char *)limfield(ppcVar1,local_40);
            }
            local_60[3] = pcVar8;
            if (*local_40 == -1) {
              if (*(char *)(local_40 + 6) != '\0') {
                while (bVar3 = to_uchar((int)*local_50), blanks[(int)(uint)bVar3] != '\0') {
                  local_50 = local_50 + 1;
                }
              }
              local_60[2] = local_50;
            }
            else {
              pcVar8 = (char *)begfield(local_60,local_40);
              local_60[2] = pcVar8;
            }
          }
          local_50 = local_68;
        }
        local_68 = local_48;
      } while (*(char *)(param_1 + 6) == '\0');
      param_1[1] = local_68 + -(long)*param_1;
      lVar7 = buffer_linelim(param_1);
      param_1[2] = (char *)(lVar7 - (long)local_60 >> 5);
      if (param_1[2] != (char *)0x0) goto code_r0x001032c9;
      local_78 = (ulong)param_1[3] >> 5;
      pcVar8 = (char *)xpalloc(*param_1,&local_78,1,0xffffffffffffffff,0x20);
      *param_1 = pcVar8;
      param_1[3] = (char *)(local_78 << 5);
    } while( true );
  }
  uVar5 = 0;
LAB_00103355:
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar5;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
code_r0x001032c9:
  param_1[4] = local_68 + -(long)local_50;
  merge_buffer_size = local_70 + 0x22;
  uVar5 = 1;
  goto LAB_00103355;
}

// Function: traverse_raw_number
char traverse_raw_number(char **param_1) {
  char *pcVar1;
  char cVar2;
  char cVar3;
  bool bVar4;
  char local_13;
  char *local_10;
  
  local_13 = '\0';
  bVar4 = false;
  pcVar1 = *param_1;
  while( true ) {
    local_10 = pcVar1;
    pcVar1 = local_10 + 1;
    cVar2 = *local_10;
    cVar3 = c_isdigit((int)cVar2);
    if (cVar3 == '\0') break;
    if (local_13 < cVar2) {
      local_13 = cVar2;
    }
    bVar4 = *pcVar1 == thousands_sep;
    if (bVar4) {
      pcVar1 = local_10 + 2;
    }
  }
  if (bVar4) {
    *param_1 = local_10 + -1;
  }
  else {
    local_10 = pcVar1;
    if (cVar2 == decimal_point) {
      while( true ) {
        pcVar1 = local_10 + 1;
        cVar2 = *local_10;
        cVar3 = c_isdigit((int)cVar2);
        local_10 = pcVar1;
        if (cVar3 == '\0') break;
        if (local_13 < cVar2) {
          local_13 = cVar2;
        }
      }
    }
    *param_1 = local_10 + -1;
  }
  return local_13;
}

// Function: find_unit_order
int find_unit_order(char *param_1) {
  char cVar1;
  char cVar2;
  int iVar3;
  long in_FS_OFFSET;
  byte *local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  cVar1 = *param_1;
  local_18 = (byte *)(param_1 + (cVar1 == '-'));
  cVar2 = traverse_raw_number(&local_18);
  if (cVar2 < '1') {
    iVar3 = 0;
  }
  else {
    iVar3 = (int)(char)unit_order[(int)(uint)*local_18];
    if (cVar1 == '-') {
      iVar3 = -iVar3;
    }
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return iVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: human_numcompare
ulong human_numcompare(char *param_1,char *param_2) {
  byte bVar1;
  int iVar2;
  int iVar3;
  ulong uVar4;
  char *local_38;
  char *local_30;
  
  local_30 = param_1;
  while (bVar1 = to_uchar((int)*local_30), local_38 = param_2, blanks[(int)(uint)bVar1] != '\0') {
    local_30 = local_30 + 1;
  }
  while (bVar1 = to_uchar((int)*local_38), blanks[(int)(uint)bVar1] != '\0') {
    local_38 = local_38 + 1;
  }
  iVar2 = find_unit_order(local_30);
  iVar3 = find_unit_order(local_38);
  if (iVar2 - iVar3 == 0) {
    uVar4 = strnumcmp(local_30,local_38,(int)decimal_point,thousands_sep);
  }
  else {
    uVar4 = (ulong)(uint)(iVar2 - iVar3);
  }
  return uVar4;
}

// Function: numcompare
void numcompare(char *param_1,char *param_2) {
  byte bVar1;
  char *local_18;
  char *local_10;
  
  local_10 = param_1;
  while (bVar1 = to_uchar((int)*local_10), local_18 = param_2, blanks[(int)(uint)bVar1] != '\0') {
    local_10 = local_10 + 1;
  }
  while (bVar1 = to_uchar((int)*local_18), blanks[(int)(uint)bVar1] != '\0') {
    local_18 = local_18 + 1;
  }
  strnumcmp(local_10,local_18,(int)decimal_point,thousands_sep);
  return;
}

// Function: nan_compare
void nan_compare(void) {
  long in_FS_OFFSET;
  char local_128 [135];
  char local_a1 [145];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  snprintf(local_128,0x87,"%Lf");
  snprintf(local_a1,0x87,"%Lf");
  strcmp(local_128,local_a1);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: general_numcompare
undefined8 general_numcompare(char *param_1,char *param_2) {
  undefined8 uVar1;
  long in_FS_OFFSET;
  unkbyte10 in_ST0;
  longdouble in_ST1;
  char *local_48;
  char *local_40;
  undefined8 local_38;
  undefined2 uStack_30;
  undefined8 local_28;
  undefined2 uStack_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  strtold(param_1,&local_48);
  local_38 = (undefined8)in_ST0;
  uStack_30 = (undefined2)((unkuint10)in_ST0 >> 0x40);
  strtold(param_2,&local_40);
  local_28 = SUB108(in_ST1,0);
  uStack_20 = (undefined2)((unkuint10)in_ST1 >> 0x40);
  if (param_1 == local_48) {
    if (param_2 == local_40) {
      uVar1 = 0;
    }
    else {
      uVar1 = 0xffffffff;
    }
  }
  else if (param_2 == local_40) {
    uVar1 = 1;
  }
  else if (in_ST1 <= (longdouble)CONCAT28(uStack_30,local_38)) {
    if ((longdouble)CONCAT28(uStack_30,local_38) <= in_ST1) {
      if (in_ST1 == (longdouble)CONCAT28(uStack_30,local_38)) {
        uVar1 = 0;
      }
      else if (NAN(in_ST1) || NAN(in_ST1)) {
        if (NAN((longdouble)CONCAT28(uStack_30,local_38)) ||
            NAN((longdouble)CONCAT28(uStack_30,local_38))) {
          uVar1 = nan_compare();
        }
        else {
          uVar1 = 1;
        }
      }
      else {
        uVar1 = 0xffffffff;
      }
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0xffffffff;
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar1;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: getmonth
undefined4 getmonth(char *param_1,char **param_2) {
  byte bVar1;
  byte bVar2;
  ulong uVar3;
  char *local_48;
  ulong local_38;
  ulong local_30;
  char *local_28;
  char *local_20;
  
  local_38 = 0;
  local_30 = 0xc;
  local_48 = param_1;
  while (bVar1 = to_uchar((int)*local_48), blanks[(int)(uint)bVar1] != '\0') {
    local_48 = local_48 + 1;
  }
  do {
    uVar3 = local_30 + local_38 >> 1;
    local_28 = local_48;
    local_20 = *(char **)(monthtab + uVar3 * 0x10);
    while( true ) {
      if (*local_20 == '\0') {
        if (param_2 != (char **)0x0) {
          *param_2 = local_28;
        }
        return *(undefined4 *)(monthtab + uVar3 * 0x10 + 8);
      }
      bVar1 = to_uchar((int)*local_28);
      bVar1 = to_uchar((int)(char)fold_toupper[(int)(uint)bVar1]);
      bVar2 = to_uchar((int)*local_20);
      if (bVar1 < bVar2) goto LAB_00103977;
      bVar1 = to_uchar((int)*local_28);
      bVar1 = to_uchar((int)(char)fold_toupper[(int)(uint)bVar1]);
      bVar2 = to_uchar((int)*local_20);
      if (bVar2 < bVar1) break;
      local_28 = local_28 + 1;
      local_20 = local_20 + 1;
    }
    local_38 = uVar3 + 1;
    uVar3 = local_30;
LAB_00103977:
    local_30 = uVar3;
    if (local_30 <= local_38) {
      return 0;
    }
  } while( true );
}

// Function: link_failure
void link_failure(void) {
  undefined8 uVar1;
  
  uVar1 = dlerror();
                    /* WARNING: Subroutine does not return */
  error(2,0,&DAT_0010db64,uVar1);
}

// Function: symbol_address
long symbol_address(undefined8 param_1,undefined8 param_2) {
  long lVar1;
  
  lVar1 = dlsym(param_1,param_2);
  if (lVar1 == 0) {
    link_failure();
  }
  return lVar1;
}

// Function: link_libcrypto
void link_libcrypto(void) {
  long lVar1;
  
  lVar1 = dlopen("libcrypto.so.3",0x101);
  if (lVar1 == 0) {
    link_failure();
  }
  ptr_MD5_Init = symbol_address(lVar1,"MD5_Init");
  ptr_MD5_Update = symbol_address(lVar1,"MD5_Update");
  ptr_MD5_Final = symbol_address(lVar1,"MD5_Final");
  return;
}

// Function: random_md5_state_init
void random_md5_state_init(char *param_1) {
  int iVar1;
  long lVar2;
  undefined8 uVar3;
  char *pcVar4;
  long in_FS_OFFSET;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  lVar2 = randread_new(param_1,0x10);
  if (lVar2 == 0) {
    pcVar4 = param_1;
    if (param_1 == (char *)0x0) {
      pcVar4 = "getrandom";
    }
    uVar3 = gettext("open failed");
    sort_die(uVar3,pcVar4);
  }
  randread(lVar2,local_38,0x10);
  iVar1 = randread_free(lVar2);
  if (iVar1 != 0) {
    uVar3 = gettext("close failed");
    sort_die(uVar3,param_1);
  }
  link_libcrypto();
  md5_init_ctx(&random_md5_state);
  md5_process_bytes(local_38,0x10,&random_md5_state);
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: xstrxfrm
size_t xstrxfrm(char *param_1,char *param_2,size_t param_3) {
  int *piVar1;
  size_t sVar2;
  undefined8 uVar3;
  
  piVar1 = __errno_location();
  *piVar1 = 0;
  sVar2 = strxfrm(param_1,param_2,param_3);
  piVar1 = __errno_location();
  if (*piVar1 != 0) {
    uVar3 = gettext("string transformation failed");
    piVar1 = __errno_location();
                    /* WARNING: Subroutine does not return */
    error(0,*piVar1,uVar3);
  }
  return sVar2;
}

// Function: compare_random
int compare_random(char *param_1,ulong param_2,char *param_3,ulong param_4) {
  ulong uVar1;
  long lVar2;
  size_t sVar3;
  char *pcVar4;
  long in_FS_OFFSET;
  ulong local_1108;
  char *local_1100;
  ulong local_10f8;
  char *local_10f0;
  int local_10e0;
  int local_10dc;
  char *local_10d8;
  ulong local_10d0;
  char *local_10c8;
  undefined local_1098 [16];
  undefined local_1088 [16];
  undefined8 local_1078;
  undefined8 local_1070;
  undefined8 local_1068;
  undefined8 local_1060;
  undefined8 local_1058;
  undefined8 local_1050;
  undefined8 local_1048;
  undefined8 local_1040;
  undefined8 local_1038;
  undefined4 local_1030;
  undefined4 uStack_102c;
  undefined4 uStack_1028;
  undefined8 local_1024;
  undefined8 local_101c;
  undefined8 local_1014;
  undefined4 uStack_100c;
  undefined4 local_1008;
  undefined4 uStack_1004;
  undefined4 uStack_1000;
  undefined8 local_ffc;
  undefined8 local_ff4;
  undefined8 local_fec;
  undefined8 local_fe4;
  undefined8 local_fdc;
  undefined4 local_fd4;
  undefined4 uStack_fd0;
  undefined4 uStack_fcc;
  undefined8 local_fc8;
  char local_fb8 [4008];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_10e0 = 0;
  local_10d8 = local_fb8;
  local_10d0 = 4000;
  local_10c8 = (char *)0x0;
  local_101c = random_md5_state;
  local_1014 = DAT_0010c348;
  uStack_100c = (undefined4)DAT_0010c350;
  local_1008 = (undefined4)((ulong)DAT_0010c350 >> 0x20);
  uStack_1004 = (undefined4)DAT_0010c358;
  uStack_1000 = (undefined4)((ulong)DAT_0010c358 >> 0x20);
  local_ffc = DAT_0010c360;
  local_ff4 = DAT_0010c368;
  local_fec = DAT_0010c370;
  local_fe4 = DAT_0010c378;
  local_fdc = DAT_0010c380;
  local_fd4 = (undefined4)DAT_0010c388;
  uStack_fd0 = DAT_0010c388._4_4_;
  uStack_fcc = uRam000000000010c390;
  local_fc8 = DAT_0010c394;
  local_1078 = random_md5_state;
  local_1070 = DAT_0010c348;
  local_1068 = DAT_0010c350;
  local_1060 = DAT_0010c358;
  local_1058 = DAT_0010c360;
  local_1050 = DAT_0010c368;
  local_1048 = DAT_0010c370;
  local_1040 = DAT_0010c378;
  local_1038 = DAT_0010c380;
  local_1030 = (undefined4)DAT_0010c388;
  uStack_102c = DAT_0010c388._4_4_;
  uStack_1028 = uRam000000000010c390;
  local_1024 = DAT_0010c394;
  local_1108 = param_4;
  local_1100 = param_3;
  local_10f8 = param_2;
  local_10f0 = param_1;
  if (hard_LC_COLLATE != '\0') {
    param_1 = param_1 + param_2;
    param_3 = param_3 + param_4;
    while( true ) {
      uVar1 = (param_2 + param_4) * 3 + 2;
      if (local_10d0 < uVar1) {
        local_10d0 = local_10d0 * 3 >> 1;
        if (local_10d0 <= uVar1) {
          local_10d0 = uVar1;
        }
        free(local_10c8);
        local_10c8 = (char *)malloc(local_10d0);
        local_10d8 = local_10c8;
        if (local_10c8 == (char *)0x0) {
          local_10d8 = local_fb8;
          local_10d0 = 4000;
        }
      }
      if (local_10f0 < param_1) {
        lVar2 = xstrxfrm(local_10d8,local_10f0,local_10d0);
        local_10f8 = lVar2 + 1;
      }
      else {
        local_10f8 = 0;
      }
      if (local_1100 < param_3) {
        if (local_10d0 < local_10f8) {
          lVar2 = 0;
          pcVar4 = (char *)0x0;
        }
        else {
          lVar2 = local_10d0 - local_10f8;
          pcVar4 = local_10d8 + local_10f8;
        }
        lVar2 = xstrxfrm(pcVar4,local_1100,lVar2);
        local_1108 = lVar2 + 1;
      }
      else {
        local_1108 = 0;
      }
      if ((local_10d0 < local_10f8) || (local_10d0 < local_1108 + local_10f8)) {
        local_10d0 = local_1108 + local_10f8;
        if (local_10d0 < 0x5555555555555555) {
          local_10d0 = local_10d0 * 3 >> 1;
        }
        free(local_10c8);
        local_10d8 = (char *)xmalloc(local_10d0);
        if (local_10f0 < param_1) {
          strxfrm(local_10d8,local_10f0,local_10f8);
        }
        local_10c8 = local_10d8;
        if (local_1100 < param_3) {
          strxfrm(local_10d8 + local_10f8,local_1100,local_1108);
        }
      }
      if (local_10f0 < param_1) {
        sVar3 = strlen(local_10f0);
        local_10f0 = local_10f0 + sVar3 + 1;
      }
      if (local_1100 < param_3) {
        sVar3 = strlen(local_1100);
        local_1100 = local_1100 + sVar3 + 1;
      }
      if ((param_1 <= local_10f0) && (param_3 <= local_1100)) break;
      md5_process_bytes(local_10d8,local_10f8,&local_1078);
      md5_process_bytes(local_10d8 + local_10f8,local_1108,&local_101c);
      if (local_10e0 == 0) {
        uVar1 = local_1108;
        if (local_10f8 < local_1108) {
          uVar1 = local_10f8;
        }
        local_10e0 = memcmp(local_10d8,local_10d8 + local_10f8,uVar1);
        if (local_10e0 == 0) {
          local_10e0 = (uint)(local_1108 < local_10f8) - (uint)(local_10f8 < local_1108);
        }
      }
    }
    local_10f0 = local_10d8;
    local_1100 = local_10d8 + local_10f8;
  }
  md5_process_bytes(local_10f0,local_10f8,&local_1078);
  md5_finish_ctx(&local_1078,local_1098);
  md5_process_bytes(local_1100,local_1108,&local_101c);
  md5_finish_ctx(&local_101c,local_1088);
  local_10dc = memcmp(local_1098,local_1088,0x10);
  if (local_10dc == 0) {
    if (local_10e0 == 0) {
      uVar1 = local_1108;
      if (local_10f8 < local_1108) {
        uVar1 = local_10f8;
      }
      local_10e0 = memcmp(local_10f0,local_1100,uVar1);
      if (local_10e0 == 0) {
        local_10e0 = (uint)(local_1108 < local_10f8) - (uint)(local_10f8 < local_1108);
      }
    }
    local_10dc = local_10e0;
  }
  free(local_10c8);
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_10dc;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: debug_width
long debug_width(char *param_1,char *param_2) {
  int iVar1;
  char *local_20;
  long local_10;
  
  iVar1 = mbsnwidth(param_1,(long)param_2 - (long)param_1,0);
  local_10 = (long)iVar1;
  for (local_20 = param_1; local_20 < param_2; local_20 = local_20 + 1) {
    local_10 = local_10 + (int)(uint)(*local_20 == '\t');
  }
  return local_10;
}

// Function: mark_key
void mark_key(long param_1,long param_2) {
  char *__format;
  long local_18;
  long local_10;
  
  local_10 = param_1;
  while( true ) {
    if (local_10 == 0) break;
    putchar_unlocked(0x20);
    local_10 = local_10 + -1;
  }
  local_18 = param_2;
  if (param_2 == 0) {
    __format = (char *)gettext("^ no match for key\n");
    printf(__format);
  }
  else {
    do {
      putchar_unlocked(0x5f);
      local_18 = local_18 + -1;
    } while (local_18 != 0);
    putchar_unlocked(10);
  }
  return;
}

// Function: key_numeric
undefined4 key_numeric(long param_1) {
  undefined4 uVar1;
  
  if (((*(char *)(param_1 + 0x32) == '\0') && (*(char *)(param_1 + 0x34) == '\0')) &&
     (*(char *)(param_1 + 0x35) == '\0')) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

// Function: debug_key
void debug_key(byte **param_1,long *param_2) {
  byte bVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  byte *pbVar5;
  long in_FS_OFFSET;
  byte *local_60;
  byte *local_58;
  byte *local_50;
  byte *local_48;
  byte *local_40;
  undefined8 local_38;
  undefined8 local_30;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_50 = *param_1;
  local_48 = local_50 + (long)(param_1[1] + -1);
  local_40 = local_50;
  if (param_2 != (long *)0x0) {
    if (*param_2 != -1) {
      local_50 = (byte *)begfield(param_1,param_2);
    }
    if ((param_2[2] != -1) &&
       (pbVar5 = (byte *)limfield(param_1,param_2), local_48 = local_50, local_50 <= pbVar5)) {
      local_48 = pbVar5;
    }
    if ((((*(char *)(param_2 + 6) != '\0') && (*param_2 == -1)) ||
        (*(char *)((long)param_2 + 0x36) != '\0')) || (cVar2 = key_numeric(param_2), cVar2 != '\0'))
    {
      bVar1 = *local_48;
      *local_48 = 0;
      while (bVar3 = to_uchar((int)(char)*local_50), blanks[(int)(uint)bVar3] != '\0') {
        local_50 = local_50 + 1;
      }
      local_60 = local_50;
      if (local_48 < local_50) {
        local_60 = local_48;
      }
      else if (*(char *)((long)param_2 + 0x36) == '\0') {
        if (*(char *)((long)param_2 + 0x34) == '\0') {
          if ((*(char *)((long)param_2 + 0x32) == '\0') && (*(char *)((long)param_2 + 0x35) == '\0')
             ) {
            local_60 = local_48;
          }
          else {
            if ((local_50 < local_48) && (*local_50 == 0x2d)) {
              iVar4 = 1;
            }
            else {
              iVar4 = 0;
            }
            local_58 = local_50 + iVar4;
            cVar2 = traverse_raw_number(&local_58);
            if ('/' < cVar2) {
              if ((*(char *)((long)param_2 + 0x35) == '\0') ||
                 (unit_order[(int)(uint)*local_58] == '\0')) {
                iVar4 = 0;
              }
              else {
                iVar4 = 1;
              }
              local_60 = local_58 + iVar4;
            }
          }
        }
        else {
          strtold((char *)local_50,(char **)&local_60);
        }
      }
      else {
        getmonth(local_50,&local_60);
      }
      *local_48 = bVar1;
      local_48 = local_60;
    }
  }
  local_38 = debug_width(local_40,local_50);
  local_30 = debug_width(local_50,local_48);
  mark_key(local_38,local_30);
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: debug_line
void debug_line(undefined8 param_1) {
  long local_10;
  
  local_10 = keylist;
  while( true ) {
    do {
      debug_key(param_1,local_10);
      if (local_10 == 0) {
        return;
      }
      local_10 = *(long *)(local_10 + 0x40);
    } while (local_10 != 0);
    if (unique == '\x01') break;
    if (stable == '\x01') {
      return;
    }
  }
  return;
}

// Function: default_key_compare
undefined4 default_key_compare(long param_1) {
  char cVar1;
  
  if (((((*(long *)(param_1 + 0x20) == 0) && (*(long *)(param_1 + 0x28) == 0)) &&
       (*(char *)(param_1 + 0x30) != '\x01')) &&
      ((*(char *)(param_1 + 0x31) != '\x01' && (cVar1 = key_numeric(param_1), cVar1 != '\x01')))) &&
     ((*(char *)(param_1 + 0x36) != '\x01' &&
      ((*(char *)(param_1 + 0x38) != '\x01' && (*(char *)(param_1 + 0x33) != '\x01')))))) {
    return 1;
  }
  return 0;
}

// Function: key_to_opts
void key_to_opts(long param_1,undefined *param_2) {
  undefined *local_18;
  
  if ((*(char *)(param_1 + 0x30) != '\0') || (local_18 = param_2, *(char *)(param_1 + 0x31) != '\0')
     ) {
    local_18 = param_2 + 1;
    *param_2 = 0x62;
  }
  if (*(undefined1 **)(param_1 + 0x20) == nondictionary) {
    *local_18 = 100;
    local_18 = local_18 + 1;
  }
  if (*(long *)(param_1 + 0x28) != 0) {
    *local_18 = 0x66;
    local_18 = local_18 + 1;
  }
  if (*(char *)(param_1 + 0x34) != '\0') {
    *local_18 = 0x67;
    local_18 = local_18 + 1;
  }
  if (*(char *)(param_1 + 0x35) != '\0') {
    *local_18 = 0x68;
    local_18 = local_18 + 1;
  }
  if (*(undefined1 **)(param_1 + 0x20) == nonprinting) {
    *local_18 = 0x69;
    local_18 = local_18 + 1;
  }
  if (*(char *)(param_1 + 0x36) != '\0') {
    *local_18 = 0x4d;
    local_18 = local_18 + 1;
  }
  if (*(char *)(param_1 + 0x32) != '\0') {
    *local_18 = 0x6e;
    local_18 = local_18 + 1;
  }
  if (*(char *)(param_1 + 0x33) != '\0') {
    *local_18 = 0x52;
    local_18 = local_18 + 1;
  }
  if (*(char *)(param_1 + 0x37) != '\0') {
    *local_18 = 0x72;
    local_18 = local_18 + 1;
  }
  if (*(char *)(param_1 + 0x38) != '\0') {
    *local_18 = 0x56;
    local_18 = local_18 + 1;
  }
  *local_18 = 0;
  return;
}

// Function: key_warnings
void key_warnings(undefined8 *param_1,char param_2) {
  ulong uVar1;
  byte bVar2;
  byte bVar3;
  byte bVar4;
  byte bVar5;
  bool bVar6;
  bool bVar7;
  bool bVar8;
  bool bVar9;
  bool bVar10;
  char cVar11;
  char *pcVar12;
  undefined2 *puVar13;
  undefined2 *puVar14;
  undefined8 uVar15;
  undefined8 uVar16;
  undefined8 uVar17;
  size_t sVar18;
  long in_FS_OFFSET;
  ulong *local_198;
  long local_190;
  ulong local_188;
  ulong local_180;
  undefined8 local_f8;
  undefined8 local_f0;
  undefined8 local_e8;
  undefined8 local_e0;
  ulong local_d8;
  ulong local_d0;
  undefined8 local_c8;
  undefined8 local_c0;
  undefined8 local_b8;
  undefined local_a8 [32];
  undefined2 local_88;
  char local_58;
  undefined uStack_57;
  char acStack_55 [53];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_f8 = *param_1;
  local_f0 = param_1[1];
  local_e8 = param_1[2];
  local_e0 = param_1[3];
  local_d8 = param_1[4];
  local_d0 = param_1[5];
  local_c8 = param_1[6];
  local_c0 = param_1[7];
  local_b8 = param_1[8];
  local_190 = 1;
  bVar6 = false;
  bVar7 = false;
  local_198 = keylist;
  while( true ) {
    if (local_198 == (ulong *)0x0) {
      if ((bVar6) || (bVar7)) {
        local_58 = decimal_point;
        uStack_57 = 0;
        uVar16 = quote(&local_58);
        uVar17 = gettext("numbers use %s as a decimal point in this locale");
                    /* WARNING: Subroutine does not return */
        error(0,0,uVar17,uVar16);
      }
      cVar11 = default_key_compare(&local_f8);
      if ((cVar11 == '\x01') &&
         ((local_c8._7_1_ == '\0' ||
          (((stable == '\0' && (unique == '\0')) || (keylist == (ulong *)0x0)))))) {
        if (((local_c8._7_1_ != '\0') && (stable != '\x01')) &&
           ((unique != '\x01' && (keylist != (ulong *)0x0)))) {
          uVar16 = gettext("option \'-r\' only applies to last-resort comparison");
                    /* WARNING: Subroutine does not return */
          error(0,0,uVar16);
        }
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        return;
      }
      if ((stable != '\x01') && (unique != '\x01')) {
        local_c8 = local_c8 & 0xffffffffffffff;
      }
      key_to_opts(&local_f8,&local_58);
      sVar18 = strlen(&local_58);
      uVar16 = select_plural(sVar18);
      uVar16 = ngettext("option \'-%s\' is ignored","options \'-%s\' are ignored",uVar16);
                    /* WARNING: Subroutine does not return */
      error(0,0,uVar16,&local_58);
    }
    cVar11 = key_numeric(local_198);
    if (cVar11 != '\0') {
      if (*(char *)((long)local_198 + 0x34) == '\0') {
        bVar6 = true;
      }
      else {
        bVar7 = true;
      }
    }
    if (*(char *)((long)local_198 + 0x39) != '\0') {
      local_188 = *local_198;
      uVar1 = local_198[2];
      if (local_188 == 0xffffffffffffffff) {
        local_188 = 0;
      }
      pcVar12 = (char *)umaxtostr(local_188,local_a8);
      local_88 = 0x2b;
      puVar13 = (undefined2 *)stpcpy((char *)((long)&local_88 + 1),pcVar12);
      pcVar12 = (char *)umaxtostr(local_188 + 1,local_a8);
      puVar14 = (undefined2 *)stpcpy(acStack_55,pcVar12);
      if (local_198[2] != 0xffffffffffffffff) {
        pcVar12 = (char *)umaxtostr(uVar1 + 1,local_a8);
        *puVar13 = 0x2d20;
        *(undefined *)(puVar13 + 1) = 0;
        strcpy((char *)(puVar13 + 1),pcVar12);
        pcVar12 = (char *)umaxtostr(uVar1 + (local_198[3] == 0xffffffffffffffff) + 1,local_a8);
        *puVar14 = 0x2c;
        strcpy((char *)((long)puVar14 + 1),pcVar12);
      }
      uVar16 = quote_n(1,&local_58);
      uVar17 = quote_n(0,&local_88);
      uVar15 = gettext("obsolescent key %s used; consider %s instead");
                    /* WARNING: Subroutine does not return */
      error(0,0,uVar15,uVar17,uVar16);
    }
    if ((*local_198 == 0xffffffffffffffff) || (*local_198 <= local_198[2])) {
      bVar8 = false;
    }
    else {
      bVar8 = true;
    }
    if (bVar8) {
      uVar16 = gettext("key %lu has zero width and will be ignored");
                    /* WARNING: Subroutine does not return */
      error(0,0,uVar16,local_190);
    }
    cVar11 = key_numeric(local_198);
    if ((cVar11 == '\0') && (*(char *)((long)local_198 + 0x36) == '\0')) {
      bVar9 = false;
    }
    else {
      bVar9 = true;
    }
    if ((local_198[2] == 0) && (local_198[3] != 0)) {
      bVar10 = true;
    }
    else {
      bVar10 = false;
    }
    if (((((!bVar8) && (param_2 != '\x01')) && (tab == 0x80)) && (!bVar10)) &&
       ((((*(char *)(local_198 + 6) != '\x01' && (!bVar9)) ||
         ((*(char *)(local_198 + 6) != '\x01' && (local_198[1] != 0)))) ||
        ((*(char *)((long)local_198 + 0x31) != '\x01' && (local_198[3] != 0)))))) break;
    if ((param_2 != '\x01') && (cVar11 = key_numeric(local_198), cVar11 != '\0')) {
      local_180 = *local_198 + 1;
      if (local_180 == 0) {
        local_180 = *local_198 + 2;
      }
      if ((local_198[2] == 0xffffffffffffffff) || (local_180 < local_198[2] + 1)) {
        uVar16 = gettext("key %lu is numeric and spans multiple fields");
                    /* WARNING: Subroutine does not return */
        error(0,0,uVar16,local_190);
      }
    }
    uVar1 = local_c8;
    if ((local_d8 != 0) && (local_d8 == local_198[4])) {
      local_d8 = 0;
    }
    if ((local_d0 != 0) && (local_d0 == local_198[5])) {
      local_d0 = 0;
    }
    bVar2 = (*(byte *)(local_198 + 6) ^ 1) & (byte)local_c8;
    bVar3 = (*(byte *)((long)local_198 + 0x31) ^ 1) & local_c8._1_1_;
    local_c8._6_1_ = SUB81(uVar1,6);
    local_c8._7_1_ = SUB81(uVar1,7);
    local_c8._2_1_ = SUB81(uVar1,2);
    bVar4 = (*(byte *)((long)local_198 + 0x32) ^ 1) & local_c8._2_1_;
    local_c8._4_1_ = SUB81(uVar1,4);
    local_c8._5_1_ = SUB81(uVar1,5);
    local_c8._3_1_ = SUB81(uVar1,3);
    bVar5 = (*(byte *)((long)local_198 + 0x33) ^ 1) & local_c8._3_1_;
    local_c8 = CONCAT44(CONCAT22(CONCAT11(local_c8._7_1_,
                                          ((*(byte *)((long)local_198 + 0x36) ^ 1) & local_c8._6_1_)
                                          != 0),
                                 CONCAT11(((*(byte *)((long)local_198 + 0x35) ^ 1) & local_c8._5_1_)
                                          != 0,((*(byte *)((long)local_198 + 0x34) ^ 1) &
                                               local_c8._4_1_) != 0)),
                        CONCAT13(bVar5 != 0,CONCAT12(bVar4 != 0,CONCAT11(bVar3 != 0,bVar2 != 0))));
    local_c0 = CONCAT71(local_c0._1_7_,((*(byte *)(local_198 + 7) ^ 1) & (byte)local_c0) != 0);
    local_c8 = CONCAT17(((*(byte *)((long)local_198 + 0x37) ^ 1) & local_c8._7_1_) != 0,
                        (undefined7)local_c8);
    local_198 = (ulong *)local_198[8];
    local_190 = local_190 + 1;
  }
  uVar16 = gettext("leading blanks are significant in key %lu; consider also specifying \'b\'");
                    /* WARNING: Subroutine does not return */
  error(0,0,uVar16,local_190);
}

// Function: diff_reversed
int diff_reversed(int param_1,char param_2) {
  if (param_2 != '\0') {
    param_1 = -(uint)(0 < param_1) - (param_1 >> 0x1f);
  }
  return param_1;
}

// Function: keycompare
undefined8 keycompare(char **param_1,char **param_2) {
  char cVar1;
  long lVar2;
  long lVar3;
  char cVar4;
  char cVar5;
  byte bVar6;
  byte bVar7;
  int iVar8;
  char *pcVar9;
  char *pcVar10;
  ulong uVar11;
  ulong uVar12;
  ulong uVar13;
  undefined8 uVar14;
  long in_FS_OFFSET;
  int local_105c;
  long *local_1058;
  char *local_1050;
  char *local_1048;
  char *local_1040;
  char *local_1038;
  char *local_1030;
  char *local_1028;
  ulong local_1020;
  ulong local_1018;
  char *local_1010;
  ulong local_1008;
  ulong local_1000;
  char local_fc8 [4008];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_1058 = keylist;
  local_1050 = param_1[2];
  local_1048 = param_2[2];
  local_1040 = param_1[3];
  local_1038 = param_2[3];
  do {
    lVar2 = local_1058[5];
    lVar3 = local_1058[4];
    pcVar9 = local_1050;
    if (local_1050 <= local_1040) {
      pcVar9 = local_1040;
    }
    pcVar10 = local_1048;
    if (local_1048 <= local_1038) {
      pcVar10 = local_1038;
    }
    uVar11 = (long)pcVar9 - (long)local_1050;
    uVar12 = (long)pcVar10 - (long)local_1048;
    if ((((hard_LC_COLLATE == '\0') && (cVar4 = key_numeric(local_1058), cVar4 == '\0')) &&
        (*(char *)((long)local_1058 + 0x36) == '\0')) &&
       ((*(char *)((long)local_1058 + 0x33) == '\0' && (*(char *)(local_1058 + 7) == '\0')))) {
      if (lVar3 == 0) {
        uVar13 = uVar11;
        if (uVar12 <= uVar11) {
          uVar13 = uVar12;
        }
        if (uVar13 == 0) {
          local_105c = 0;
        }
        else if (lVar2 == 0) {
          local_105c = memcmp(local_1050,local_1048,uVar13);
        }
        else {
          local_1000 = 0;
          do {
            bVar7 = to_uchar((int)local_1050[local_1000]);
            bVar7 = to_uchar((int)*(char *)(lVar2 + (ulong)bVar7));
            bVar6 = to_uchar((int)local_1048[local_1000]);
            bVar6 = to_uchar((int)*(char *)(lVar2 + (ulong)bVar6));
            local_105c = (uint)bVar7 - (uint)bVar6;
            if (local_105c != 0) break;
            local_1000 = local_1000 + 1;
          } while (local_1000 < uVar13);
        }
        if (local_105c == 0) {
          local_105c = (uint)(uVar12 < uVar11) - (uint)(uVar11 < uVar12);
        }
      }
      else if (lVar2 == 0) {
        while( true ) {
          while ((local_1050 < pcVar9 &&
                 (bVar7 = to_uchar((int)*local_1050), *(char *)(lVar3 + (ulong)bVar7) != '\0'))) {
            local_1050 = local_1050 + 1;
          }
          while ((local_1048 < pcVar10 &&
                 (bVar7 = to_uchar((int)*local_1048), *(char *)(lVar3 + (ulong)bVar7) != '\0'))) {
            local_1048 = local_1048 + 1;
          }
          if ((pcVar9 <= local_1050) || (pcVar10 <= local_1048)) break;
          bVar7 = to_uchar((int)*local_1050);
          bVar6 = to_uchar((int)*local_1048);
          local_105c = (uint)bVar7 - (uint)bVar6;
          if (local_105c != 0) goto LAB_0010629a;
          local_1050 = local_1050 + 1;
          local_1048 = local_1048 + 1;
        }
        local_105c = (uint)(local_1050 < pcVar9) - (uint)(local_1048 < pcVar10);
      }
      else {
        while( true ) {
          while ((local_1050 < pcVar9 &&
                 (bVar7 = to_uchar((int)*local_1050), *(char *)(lVar3 + (ulong)bVar7) != '\0'))) {
            local_1050 = local_1050 + 1;
          }
          while ((local_1048 < pcVar10 &&
                 (bVar7 = to_uchar((int)*local_1048), *(char *)(lVar3 + (ulong)bVar7) != '\0'))) {
            local_1048 = local_1048 + 1;
          }
          if ((pcVar9 <= local_1050) || (pcVar10 <= local_1048)) break;
          bVar7 = to_uchar((int)*local_1050);
          bVar7 = to_uchar((int)*(char *)(lVar2 + (ulong)bVar7));
          bVar6 = to_uchar((int)*local_1048);
          bVar6 = to_uchar((int)*(char *)(lVar2 + (ulong)bVar6));
          local_105c = (uint)bVar7 - (uint)bVar6;
          if (local_105c != 0) goto LAB_0010629a;
          local_1050 = local_1050 + 1;
          local_1048 = local_1048 + 1;
        }
        local_105c = (uint)(local_1050 < pcVar9) - (uint)(local_1048 < pcVar10);
      }
    }
    else {
      local_1030 = local_1050;
      local_1028 = local_1048;
      cVar4 = local_1050[uVar11];
      cVar1 = local_1048[uVar12];
      local_1010 = (char *)0x0;
      if ((lVar3 != 0) || (local_1020 = uVar11, local_1018 = uVar12, lVar2 != 0)) {
        uVar13 = uVar12 + uVar11 + 2;
        if (uVar13 < 0xfa1) {
          local_1030 = local_fc8;
        }
        else {
          local_1030 = (char *)xmalloc(uVar13);
          local_1010 = local_1030;
        }
        local_1028 = local_1030 + uVar11 + 1;
        local_1020 = 0;
        for (local_1008 = 0; local_1008 < uVar11; local_1008 = local_1008 + 1) {
          if ((lVar3 == 0) ||
             (bVar7 = to_uchar((int)local_1050[local_1008]),
             *(char *)(lVar3 + (ulong)bVar7) != '\x01')) {
            if (lVar2 == 0) {
              cVar5 = local_1050[local_1008];
            }
            else {
              bVar7 = to_uchar((int)local_1050[local_1008]);
              cVar5 = *(char *)(lVar2 + (ulong)bVar7);
            }
            local_1030[local_1020] = cVar5;
            local_1020 = local_1020 + 1;
          }
        }
        local_1018 = 0;
        for (local_1008 = 0; local_1008 < uVar12; local_1008 = local_1008 + 1) {
          if ((lVar3 == 0) ||
             (bVar7 = to_uchar((int)local_1048[local_1008]),
             *(char *)(lVar3 + (ulong)bVar7) != '\x01')) {
            if (lVar2 == 0) {
              cVar5 = local_1048[local_1008];
            }
            else {
              bVar7 = to_uchar((int)local_1048[local_1008]);
              cVar5 = *(char *)(lVar2 + (ulong)bVar7);
            }
            local_1028[local_1018] = cVar5;
            local_1018 = local_1018 + 1;
          }
        }
      }
      local_1030[local_1020] = '\0';
      local_1028[local_1018] = '\0';
      if (*(char *)((long)local_1058 + 0x32) == '\0') {
        if (*(char *)((long)local_1058 + 0x34) == '\0') {
          if (*(char *)((long)local_1058 + 0x35) == '\0') {
            if (*(char *)((long)local_1058 + 0x36) == '\0') {
              if (*(char *)((long)local_1058 + 0x33) == '\0') {
                if (*(char *)(local_1058 + 7) == '\0') {
                  if (local_1020 == 0) {
                    local_105c = -(uint)(local_1018 != 0);
                  }
                  else if (local_1018 == 0) {
                    local_105c = 1;
                  }
                  else {
                    local_105c = xmemcoll0(local_1030,local_1020 + 1,local_1028,local_1018 + 1);
                  }
                }
                else {
                  local_105c = filenvercmp(local_1030,local_1020,local_1028,local_1018);
                }
              }
              else {
                local_105c = compare_random(local_1030,local_1020,local_1028,local_1018);
              }
            }
            else {
              local_105c = getmonth(local_1030,0);
              iVar8 = getmonth(local_1028,0);
              local_105c = local_105c - iVar8;
            }
          }
          else {
            local_105c = human_numcompare(local_1030,local_1028);
          }
        }
        else {
          local_105c = general_numcompare(local_1030,local_1028);
        }
      }
      else {
        local_105c = numcompare(local_1030,local_1028);
      }
      local_1030[local_1020] = cVar4;
      local_1028[local_1018] = cVar1;
      free(local_1010);
    }
LAB_0010629a:
    if (local_105c != 0) {
      uVar14 = diff_reversed(local_105c,*(undefined *)((long)local_1058 + 0x37));
      goto LAB_0010649b;
    }
    local_1058 = (long *)local_1058[8];
    if (local_1058 == (long *)0x0) {
      uVar14 = 0;
LAB_0010649b:
      if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
        return uVar14;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    if (local_1058[2] == -1) {
      local_1040 = param_1[1] + -1 + (long)*param_1;
      local_1038 = param_2[1] + -1 + (long)*param_2;
    }
    else {
      local_1040 = (char *)limfield(param_1,local_1058);
      local_1038 = (char *)limfield(param_2,local_1058);
    }
    if (*local_1058 == -1) {
      local_1050 = *param_1;
      local_1048 = *param_2;
      if (*(char *)(local_1058 + 6) != '\0') {
        while ((local_1050 < local_1040 &&
               (bVar7 = to_uchar((int)*local_1050), blanks[(int)(uint)bVar7] != '\0'))) {
          local_1050 = local_1050 + 1;
        }
        while ((local_1048 < local_1038 &&
               (bVar7 = to_uchar((int)*local_1048), blanks[(int)(uint)bVar7] != '\0'))) {
          local_1048 = local_1048 + 1;
        }
      }
    }
    else {
      local_1050 = (char *)begfield(param_1,local_1058);
      local_1048 = (char *)begfield(param_2,local_1058);
    }
  } while( true );
}

// Function: compare
ulong compare(void **param_1,void **param_2) {
  uint uVar1;
  ulong uVar2;
  ulong uVar3;
  ulong __n;
  int local_1c;
  
  if ((keylist != 0) &&
     (((uVar1 = keycompare(param_1,param_2), uVar1 != 0 || (unique != '\0')) || (stable != '\0'))))
  {
    return (ulong)uVar1;
  }
  uVar2 = (long)param_1[1] - 1;
  uVar3 = (long)param_2[1] - 1;
  if (uVar2 == 0) {
    local_1c = -(uint)(uVar3 != 0);
  }
  else if (uVar3 == 0) {
    local_1c = 1;
  }
  else if (hard_LC_COLLATE == '\0') {
    __n = uVar3;
    if (uVar2 < uVar3) {
      __n = uVar2;
    }
    local_1c = memcmp(*param_1,*param_2,__n);
    if (local_1c == 0) {
      local_1c = (uint)(uVar3 < uVar2) - (uint)(uVar2 < uVar3);
    }
  }
  else {
    local_1c = xmemcoll0(*param_1,param_1[1],*param_2,param_2[1]);
  }
  uVar2 = diff_reversed(local_1c,reverse);
  return uVar2;
}

// Function: write_line
void write_line(char **param_1,FILE *param_2,long param_3) {
  char *pcVar1;
  int iVar2;
  char *pcVar3;
  undefined8 uVar4;
  char *pcVar5;
  char local_29;
  char *local_28;
  
  local_28 = *param_1;
  pcVar1 = param_1[1];
  pcVar3 = pcVar1 + (long)local_28;
  if ((param_3 == 0) && (debug != '\0')) {
    while (local_28 < pcVar3) {
      pcVar1 = local_28 + 1;
      local_29 = *local_28;
      if (local_29 == '\t') {
        local_29 = '>';
      }
      else if (pcVar1 == pcVar3) {
        local_29 = '\n';
      }
      iVar2 = fputc_unlocked((int)local_29,param_2);
      local_28 = pcVar1;
      if (iVar2 == -1) {
        uVar4 = gettext("write failed");
        sort_die(uVar4,0);
      }
    }
    debug_line(param_1);
  }
  else {
    pcVar3[-1] = eolchar;
    pcVar5 = (char *)fwrite_unlocked(local_28,1,(size_t)pcVar1,param_2);
    if (pcVar1 != pcVar5) {
      uVar4 = gettext("write failed");
      sort_die(uVar4,param_3);
    }
    pcVar3[-1] = '\0';
  }
  return;
}

// Function: check
undefined check(undefined8 param_1,char param_2) {
  void **ppvVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  void **ppvVar5;
  long lVar6;
  char *__format;
  undefined8 uVar7;
  ulong uVar8;
  long in_FS_OFFSET;
  bool bVar9;
  undefined local_ba;
  void *local_b8;
  long local_b0;
  void **local_a8;
  void *local_78;
  void *local_70;
  long local_68;
  long local_60;
  void *local_58 [2];
  long local_48;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  uVar4 = xfopen(param_1,&DAT_0010d96d);
  lVar6 = keylist;
  local_b8 = (void *)0x0;
  local_b0 = 0;
  bVar9 = unique == '\0';
  local_ba = 1;
  uVar8 = sort_size;
  if (sort_size < merge_buffer_size) {
    uVar8 = merge_buffer_size;
  }
  initbuf(local_58,0x20,uVar8);
  local_78 = (void *)0x0;
  do {
    cVar2 = fillbuf(local_58,uVar4,param_1);
    if (cVar2 == '\0') {
LAB_00106a96:
      xfclose(uVar4,param_1);
      free(local_58[0]);
      free(local_78);
      if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
        return local_ba;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    local_a8 = (void **)buffer_linelim(local_58);
    ppvVar5 = local_a8 + local_48 * -4;
    if ((local_b8 != (void *)0x0) &&
       (iVar3 = compare(&local_78,local_a8 + -4), (int)(uint)bVar9 <= iVar3)) {
LAB_00106877:
      if (param_2 == 'c') {
        lVar6 = buffer_linelim(local_58);
        uVar7 = _program_name;
        __format = (char *)gettext("%s: %s:%ju: disorder: ");
        fprintf(_stderr,__format,uVar7,param_1,local_b0 + (lVar6 - (long)(local_a8 + -4) >> 5));
        uVar7 = gettext("standard error");
        write_line(local_a8 + -4,_stderr,uVar7);
      }
      local_ba = 0;
      goto LAB_00106a96;
    }
    while (ppvVar1 = local_a8 + -4, ppvVar5 < ppvVar1) {
      iVar3 = compare(ppvVar1,local_a8 + -8);
      local_a8 = ppvVar1;
      if ((int)(uint)bVar9 <= iVar3) goto LAB_00106877;
    }
    local_b0 = local_b0 + local_48;
    if (local_b8 < local_a8[-3]) {
      do {
        local_b8 = (void *)((long)local_b8 << 1);
        if (local_b8 == (void *)0x0) {
          local_b8 = local_a8[-3];
          break;
        }
      } while (local_b8 < local_a8[-3]);
      free(local_78);
      local_78 = (void *)xmalloc(local_b8);
    }
    memcpy(local_78,*ppvVar1,(size_t)local_a8[-3]);
    local_70 = local_a8[-3];
    if (lVar6 != 0) {
      local_68 = ((long)local_a8[-2] - (long)*ppvVar1) + (long)local_78;
      local_60 = ((long)local_a8[-1] - (long)*ppvVar1) + (long)local_78;
    }
  } while( true );
}

// Function: open_input_files
long open_input_files(long param_1,ulong param_2,long *param_3) {
  long lVar1;
  undefined8 uVar2;
  int local_14;
  
  lVar1 = xnmalloc(param_2,8);
  *param_3 = lVar1;
  lVar1 = *param_3;
  for (local_14 = 0; (ulong)(long)local_14 < param_2; local_14 = local_14 + 1) {
    if ((*(long *)(param_1 + (long)local_14 * 0x10 + 8) == 0) ||
       (*(char *)(*(long *)(param_1 + (long)local_14 * 0x10 + 8) + 0xc) == '\0')) {
      uVar2 = stream_open(*(undefined8 *)(param_1 + (long)local_14 * 0x10),&DAT_0010d96d);
    }
    else {
      uVar2 = open_temp(*(undefined8 *)(param_1 + (long)local_14 * 0x10 + 8));
    }
    *(undefined8 *)(lVar1 + (long)local_14 * 8) = uVar2;
    if (*(long *)(lVar1 + (long)local_14 * 8) == 0) break;
  }
  return (long)local_14;
}

// Function: mergefps
void mergefps(long param_1,ulong param_2,ulong param_3,undefined8 param_4,undefined8 param_5,
             void *param_6) {
  void **ppvVar1;
  undefined8 uVar2;
  long lVar3;
  char cVar4;
  int iVar5;
  void *__ptr;
  void *__ptr_00;
  void *__ptr_01;
  ulong *__ptr_02;
  ulong uVar6;
  long lVar7;
  undefined8 *puVar8;
  undefined8 *puVar9;
  long in_FS_OFFSET;
  ulong local_100;
  ulong local_f8;
  void **local_d8;
  void *local_d0;
  ulong local_c8;
  ulong local_c0;
  ulong local_b8;
  ulong local_b0;
  ulong local_a8;
  void *local_48;
  void *local_40;
  long local_38;
  long local_30;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  __ptr = (void *)xnmalloc(param_3,0x38);
  local_d8 = (void **)0x0;
  local_d0 = (void *)0x0;
  __ptr_00 = (void *)xnmalloc(param_3,8);
  __ptr_01 = (void *)xnmalloc(param_3,8);
  __ptr_02 = (ulong *)xnmalloc(param_3,8);
  lVar3 = keylist;
  local_48 = (void *)0x0;
  local_c8 = 0;
  local_100 = param_3;
  local_f8 = param_2;
  while (local_c8 < local_100) {
    uVar6 = merge_buffer_size;
    if (merge_buffer_size <= sort_size / local_100) {
      uVar6 = sort_size / local_100;
    }
    initbuf((void *)((long)__ptr + local_c8 * 0x38),0x20,uVar6);
    cVar4 = fillbuf((void *)((long)__ptr + local_c8 * 0x38),
                    *(undefined8 *)((long)param_6 + local_c8 * 8),
                    *(undefined8 *)(param_1 + local_c8 * 0x10));
    if (cVar4 == '\0') {
      xfclose(*(undefined8 *)((long)param_6 + local_c8 * 8),
              *(undefined8 *)(param_1 + local_c8 * 0x10));
      if (local_c8 < local_f8) {
        local_f8 = local_f8 - 1;
        zaptemp(*(undefined8 *)(param_1 + local_c8 * 0x10));
      }
      free(*(void **)((long)__ptr + local_c8 * 0x38));
      local_100 = local_100 - 1;
      for (local_c0 = local_c8; local_c0 < local_100; local_c0 = local_c0 + 1) {
        puVar8 = (undefined8 *)(param_1 + (local_c0 + 1) * 0x10);
        puVar9 = (undefined8 *)(local_c0 * 0x10 + param_1);
        uVar2 = puVar8[1];
        *puVar9 = *puVar8;
        puVar9[1] = uVar2;
        *(undefined8 *)((long)param_6 + local_c0 * 8) =
             *(undefined8 *)((long)param_6 + (local_c0 + 1) * 8);
      }
    }
    else {
      lVar7 = buffer_linelim((void *)((long)__ptr + local_c8 * 0x38));
      *(long *)((long)__ptr_00 + local_c8 * 8) = lVar7 + -0x20;
      *(long *)((long)__ptr_01 + local_c8 * 8) =
           lVar7 + *(long *)((long)__ptr + local_c8 * 0x38 + 0x10) * -0x20;
      local_c8 = local_c8 + 1;
    }
  }
  for (local_c8 = 0; local_c8 < local_100; local_c8 = local_c8 + 1) {
    __ptr_02[local_c8] = local_c8;
  }
  for (local_c8 = 1; local_c8 < local_100; local_c8 = local_c8 + 1) {
    iVar5 = compare(*(undefined8 *)((long)__ptr_00 + __ptr_02[local_c8 - 1] * 8),
                    *(undefined8 *)((long)__ptr_00 + __ptr_02[local_c8] * 8));
    if (0 < iVar5) {
      uVar6 = __ptr_02[local_c8 - 1];
      __ptr_02[local_c8 - 1] = __ptr_02[local_c8];
      __ptr_02[local_c8] = uVar6;
      local_c8 = 0;
    }
  }
LAB_001078cf:
  do {
    if (local_100 == 0) {
      if ((unique != '\0') && (local_d8 != (void **)0x0)) {
        write_line(&local_48,param_4,param_5);
        free(local_48);
      }
      xfclose(param_4,param_5);
      free(param_6);
      free(__ptr);
      free(__ptr_02);
      free(__ptr_01);
      free(__ptr_00);
      if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
        return;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    ppvVar1 = *(void ***)((long)__ptr_00 + *__ptr_02 * 8);
    if (unique == '\0') {
      write_line(ppvVar1,param_4,param_5);
    }
    else {
      if ((local_d8 != (void **)0x0) && (iVar5 = compare(local_d8,ppvVar1), iVar5 != 0)) {
        local_d8 = (void **)0x0;
        write_line(&local_48,param_4,param_5);
      }
      if (local_d8 == (void **)0x0) {
        local_d8 = &local_48;
        if (local_d0 < ppvVar1[1]) {
          do {
            if (local_d0 == (void *)0x0) {
              local_d0 = ppvVar1[1];
              break;
            }
            local_d0 = (void *)((long)local_d0 << 1);
          } while (local_d0 < ppvVar1[1]);
          free(local_48);
          local_48 = (void *)xmalloc(local_d0);
        }
        local_40 = ppvVar1[1];
        memcpy(local_48,*ppvVar1,(size_t)local_40);
        if (lVar3 != 0) {
          local_38 = ((long)ppvVar1[2] - (long)*ppvVar1) + (long)local_48;
          local_30 = ((long)ppvVar1[3] - (long)*ppvVar1) + (long)local_48;
        }
      }
    }
    if (*(void ***)((long)__ptr_01 + *__ptr_02 * 8) < ppvVar1) {
      *(void ***)((long)__ptr_00 + *__ptr_02 * 8) = ppvVar1 + -4;
    }
    else {
      cVar4 = fillbuf((void *)((long)__ptr + *__ptr_02 * 0x38),
                      *(undefined8 *)((long)param_6 + *__ptr_02 * 8),
                      *(undefined8 *)(param_1 + *__ptr_02 * 0x10));
      if (cVar4 == '\0') {
        for (local_c8 = 1; local_c8 < local_100; local_c8 = local_c8 + 1) {
          if (*__ptr_02 < __ptr_02[local_c8]) {
            __ptr_02[local_c8] = __ptr_02[local_c8] - 1;
          }
        }
        local_100 = local_100 - 1;
        xfclose(*(undefined8 *)((long)param_6 + *__ptr_02 * 8),
                *(undefined8 *)(param_1 + *__ptr_02 * 0x10));
        if (*__ptr_02 < local_f8) {
          local_f8 = local_f8 - 1;
          zaptemp(*(undefined8 *)(param_1 + *__ptr_02 * 0x10));
        }
        free(*(void **)((long)__ptr + *__ptr_02 * 0x38));
        for (local_c8 = *__ptr_02; local_c8 < local_100; local_c8 = local_c8 + 1) {
          *(undefined8 *)((long)param_6 + local_c8 * 8) =
               *(undefined8 *)((long)param_6 + (local_c8 + 1) * 8);
          puVar8 = (undefined8 *)(param_1 + (local_c8 + 1) * 0x10);
          puVar9 = (undefined8 *)(local_c8 * 0x10 + param_1);
          uVar2 = puVar8[1];
          *puVar9 = *puVar8;
          puVar9[1] = uVar2;
          puVar9 = (undefined8 *)((local_c8 + 1) * 0x38 + (long)__ptr);
          puVar8 = (undefined8 *)((long)__ptr + local_c8 * 0x38);
          uVar2 = puVar9[1];
          *puVar8 = *puVar9;
          puVar8[1] = uVar2;
          uVar2 = puVar9[3];
          puVar8[2] = puVar9[2];
          puVar8[3] = uVar2;
          uVar2 = puVar9[5];
          puVar8[4] = puVar9[4];
          puVar8[5] = uVar2;
          puVar8[6] = puVar9[6];
          *(undefined8 *)((long)__ptr_00 + local_c8 * 8) =
               *(undefined8 *)((long)__ptr_00 + (local_c8 + 1) * 8);
          *(undefined8 *)((long)__ptr_01 + local_c8 * 8) =
               *(undefined8 *)((long)__ptr_01 + (local_c8 + 1) * 8);
        }
        for (local_c8 = 0; local_c8 < local_100; local_c8 = local_c8 + 1) {
          __ptr_02[local_c8] = __ptr_02[local_c8 + 1];
        }
        goto LAB_001078cf;
      }
      lVar7 = buffer_linelim((void *)((long)__ptr + *__ptr_02 * 0x38));
      *(long *)((long)__ptr_00 + *__ptr_02 * 8) = lVar7 + -0x20;
      *(long *)((long)__ptr_01 + *__ptr_02 * 8) =
           lVar7 + *(long *)((long)__ptr + *__ptr_02 * 0x38 + 0x10) * -0x20;
    }
    local_b8 = 1;
    local_b0 = local_100;
    local_a8 = 1;
    uVar6 = *__ptr_02;
    while (local_b8 < local_b0) {
      iVar5 = compare(*(undefined8 *)((long)__ptr_00 + uVar6 * 8),
                      *(undefined8 *)((long)__ptr_00 + __ptr_02[local_a8] * 8));
      if ((iVar5 < 0) || ((iVar5 == 0 && (uVar6 < __ptr_02[local_a8])))) {
        local_b0 = local_a8;
      }
      else {
        local_b8 = local_a8 + 1;
      }
      local_a8 = local_b0 + local_b8 >> 1;
    }
    for (local_c0 = 0; local_c0 < local_b8 - 1; local_c0 = local_c0 + 1) {
      __ptr_02[local_c0] = __ptr_02[local_c0 + 1];
    }
    __ptr_02[local_b8 - 1] = uVar6;
  } while( true );
}

// Function: mergefiles
ulong mergefiles(long param_1,undefined8 param_2,ulong param_3,undefined8 param_4,undefined8 param_5
                ) {
  undefined8 uVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  undefined8 local_30;
  ulong local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_28 = open_input_files(param_1,param_3,&local_30);
  if ((local_28 < param_3) && (local_28 < 2)) {
    uVar1 = *(undefined8 *)(param_1 + local_28 * 0x10);
    uVar2 = gettext("open failed");
    sort_die(uVar2,uVar1);
  }
  mergefps(param_1,param_2,local_28,param_4,param_5,local_30);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_28;
}

// Function: mergelines
void mergelines(undefined8 *param_1,ulong param_2,undefined8 *param_3) {
  undefined8 *puVar1;
  undefined8 uVar2;
  int iVar3;
  undefined8 *local_40;
  undefined8 *local_30;
  ulong local_20;
  long local_18;
  undefined8 *local_10;
  
  local_20 = param_2 >> 1;
  local_18 = param_2 - local_20;
  local_40 = param_3;
  local_30 = param_1;
  local_10 = param_1 + local_20 * -4;
  do {
    while (iVar3 = compare(local_40 + -4,local_10 + -4), 0 < iVar3) {
      puVar1 = local_30 + -4;
      uVar2 = local_10[-3];
      *puVar1 = local_10[-4];
      local_30[-3] = uVar2;
      uVar2 = local_10[-1];
      local_30[-2] = local_10[-2];
      local_30[-1] = uVar2;
      local_18 = local_18 + -1;
      local_30 = puVar1;
      local_10 = local_10 + -4;
      if (local_18 == 0) {
        do {
          uVar2 = local_40[-3];
          local_30[-4] = local_40[-4];
          local_30[-3] = uVar2;
          uVar2 = local_40[-1];
          local_30[-2] = local_40[-2];
          local_30[-1] = uVar2;
          local_20 = local_20 - 1;
          local_40 = local_40 + -4;
          local_30 = local_30 + -4;
        } while (local_20 != 0);
        return;
      }
    }
    uVar2 = local_40[-3];
    local_30[-4] = local_40[-4];
    local_30[-3] = uVar2;
    uVar2 = local_40[-1];
    local_30[-2] = local_40[-2];
    local_30[-1] = uVar2;
    local_20 = local_20 - 1;
    local_40 = local_40 + -4;
    local_30 = local_30 + -4;
  } while (local_20 != 0);
  return;
}

// Function: sequential_sort
void sequential_sort(long param_1,ulong param_2,long param_3,char param_4) {
  undefined8 *puVar1;
  undefined8 uVar2;
  int iVar3;
  uint uVar4;
  ulong uVar5;
  long lVar6;
  long local_38;
  long local_30;
  
  if (param_2 == 2) {
    iVar3 = compare(param_1 + -0x20,param_1 + -0x40);
    uVar4 = (uint)(0 < iVar3);
    if (param_4 == '\0') {
      if (uVar4 != 0) {
        uVar2 = *(undefined8 *)(param_1 + -0x18);
        *(undefined8 *)(param_3 + -0x20) = *(undefined8 *)(param_1 + -0x20);
        *(undefined8 *)(param_3 + -0x18) = uVar2;
        uVar2 = *(undefined8 *)(param_1 + -8);
        *(undefined8 *)(param_3 + -0x10) = *(undefined8 *)(param_1 + -0x10);
        *(undefined8 *)(param_3 + -8) = uVar2;
        *(undefined8 *)(param_1 + -0x20) = *(undefined8 *)(param_1 + -0x40);
        *(undefined8 *)(param_1 + -0x18) = *(undefined8 *)(param_1 + -0x38);
        *(undefined8 *)(param_1 + -0x10) = *(undefined8 *)(param_1 + -0x30);
        *(undefined8 *)(param_1 + -8) = *(undefined8 *)(param_1 + -0x28);
        uVar2 = *(undefined8 *)(param_3 + -0x18);
        *(undefined8 *)(param_1 + -0x40) = *(undefined8 *)(param_3 + -0x20);
        *(undefined8 *)(param_1 + -0x38) = uVar2;
        uVar2 = *(undefined8 *)(param_3 + -8);
        *(undefined8 *)(param_1 + -0x30) = *(undefined8 *)(param_3 + -0x10);
        *(undefined8 *)(param_1 + -0x28) = uVar2;
      }
    }
    else {
      puVar1 = (undefined8 *)((long)(int)~uVar4 * 0x20 + param_1);
      uVar2 = puVar1[1];
      *(undefined8 *)(param_3 + -0x20) = *puVar1;
      *(undefined8 *)(param_3 + -0x18) = uVar2;
      uVar2 = puVar1[3];
      *(undefined8 *)(param_3 + -0x10) = puVar1[2];
      *(undefined8 *)(param_3 + -8) = uVar2;
      puVar1 = (undefined8 *)((long)(int)(uVar4 - 2) * 0x20 + param_1);
      uVar2 = puVar1[1];
      *(undefined8 *)(param_3 + -0x40) = *puVar1;
      *(undefined8 *)(param_3 + -0x38) = uVar2;
      uVar2 = puVar1[3];
      *(undefined8 *)(param_3 + -0x30) = puVar1[2];
      *(undefined8 *)(param_3 + -0x28) = uVar2;
    }
  }
  else {
    uVar5 = param_2 >> 1;
    if (param_4 == '\0') {
      lVar6 = 0;
    }
    else {
      lVar6 = uVar5 * -0x20;
    }
    sequential_sort(param_1 + uVar5 * -0x20,param_2 - uVar5,lVar6 + param_3,param_4);
    if (uVar5 < 2) {
      if (param_4 != '\x01') {
        uVar2 = *(undefined8 *)(param_1 + -0x18);
        *(undefined8 *)(param_3 + -0x20) = *(undefined8 *)(param_1 + -0x20);
        *(undefined8 *)(param_3 + -0x18) = uVar2;
        uVar2 = *(undefined8 *)(param_1 + -8);
        *(undefined8 *)(param_3 + -0x10) = *(undefined8 *)(param_1 + -0x10);
        *(undefined8 *)(param_3 + -8) = uVar2;
      }
    }
    else {
      sequential_sort(param_1,uVar5,param_3,param_4 == '\0');
    }
    local_38 = param_1;
    local_30 = param_3;
    if (param_4 != '\0') {
      local_38 = param_3;
      local_30 = param_1;
    }
    mergelines(local_38,param_2,local_30);
  }
  return;
}

// Function: merge_tree_init
undefined8 * merge_tree_init(long param_1,undefined8 param_2,undefined8 param_3) {
  undefined8 *puVar1;
  
  puVar1 = (undefined8 *)xmalloc(param_1 << 8);
  puVar1[3] = 0;
  puVar1[2] = puVar1[3];
  puVar1[1] = puVar1[2];
  *puVar1 = puVar1[1];
  puVar1[4] = 0;
  puVar1[6] = param_2;
  puVar1[5] = puVar1[6];
  puVar1[7] = 0;
  *(undefined4 *)(puVar1 + 10) = 0;
  *(undefined *)((long)puVar1 + 0x54) = 0;
  pthread_mutex_init((pthread_mutex_t *)(puVar1 + 0xb),(pthread_mutexattr_t *)0x0);
  init_node(puVar1,puVar1 + 0x10,param_3,param_1,param_2,0);
  return puVar1;
}

// Function: merge_tree_destroy
void merge_tree_destroy(long param_1,void *param_2) {
  long local_18;
  void *local_10;
  
  local_18 = param_1 * 2;
  local_10 = param_2;
  while (local_18 != 0) {
    pthread_mutex_destroy((pthread_mutex_t *)((long)local_10 + 0x58));
    local_10 = (void *)((long)local_10 + 0x80);
    local_18 = local_18 + -1;
  }
  free(param_2);
  return;
}

// Function: init_node
undefined8 *
init_node(long param_1,undefined8 *param_2,long param_3,ulong param_4,long param_5,char param_6) {
  ulong uVar1;
  ulong uVar2;
  long lVar3;
  long lVar4;
  undefined8 uVar5;
  undefined8 *local_68;
  
  if (param_6 == '\0') {
    uVar1 = *(ulong *)(param_1 + 0x30);
  }
  else {
    uVar1 = *(ulong *)(param_1 + 0x28);
  }
  uVar2 = uVar1 >> 1;
  param_3 = param_3 + param_5 * -0x20;
  lVar3 = param_3 + uVar2 * -0x20;
  if (param_6 == '\0') {
    lVar4 = param_1 + 0x18;
  }
  else {
    lVar4 = param_1 + 0x10;
  }
  local_68 = param_2 + 0x10;
  param_2[2] = param_3;
  *param_2 = param_2[2];
  param_2[3] = lVar3;
  param_2[1] = param_2[3];
  param_2[4] = lVar4;
  param_2[5] = uVar2;
  param_2[6] = uVar1 - uVar2;
  param_2[7] = param_1;
  *(int *)(param_2 + 10) = *(int *)(param_1 + 0x50) + 1;
  *(undefined *)((long)param_2 + 0x54) = 0;
  pthread_mutex_init((pthread_mutex_t *)(param_2 + 0xb),(pthread_mutexattr_t *)0x0);
  if (param_4 < 2) {
    param_2[8] = 0;
    param_2[9] = 0;
  }
  else {
    param_2[8] = local_68;
    uVar5 = init_node(param_2,local_68,param_3,param_4 >> 1,param_5,1);
    param_2[9] = uVar5;
    local_68 = (undefined8 *)init_node(param_2,uVar5,lVar3,param_4 - (param_4 >> 1),param_5,0);
  }
  return local_68;
}

// Function: compare_nodes
bool compare_nodes(long param_1,long param_2) {
  bool bVar1;
  
  if (*(int *)(param_1 + 0x50) == *(int *)(param_2 + 0x50)) {
    bVar1 = (ulong)(*(long *)(param_1 + 0x28) + *(long *)(param_1 + 0x30)) <
            (ulong)(*(long *)(param_2 + 0x30) + *(long *)(param_2 + 0x28));
  }
  else {
    bVar1 = *(uint *)(param_1 + 0x50) < *(uint *)(param_2 + 0x50);
  }
  return bVar1;
}

// Function: lock_node
void lock_node(long param_1) {
  pthread_mutex_lock((pthread_mutex_t *)(param_1 + 0x58));
  return;
}

// Function: unlock_node
void unlock_node(long param_1) {
  pthread_mutex_unlock((pthread_mutex_t *)(param_1 + 0x58));
  return;
}

// Function: queue_destroy
void queue_destroy(undefined8 *param_1) {
  heap_free(*param_1);
  pthread_cond_destroy((pthread_cond_t *)(param_1 + 6));
  pthread_mutex_destroy((pthread_mutex_t *)(param_1 + 1));
  return;
}

// Function: queue_init
void queue_init(undefined8 *param_1,long param_2) {
  undefined8 uVar1;
  
  uVar1 = heap_alloc(compare_nodes,param_2 * 2);
  *param_1 = uVar1;
  pthread_mutex_init((pthread_mutex_t *)(param_1 + 1),(pthread_mutexattr_t *)0x0);
  pthread_cond_init((pthread_cond_t *)(param_1 + 6),(pthread_condattr_t *)0x0);
  return;
}

// Function: queue_insert
void queue_insert(undefined8 *param_1,long param_2) {
  pthread_mutex_lock((pthread_mutex_t *)(param_1 + 1));
  heap_insert(*param_1,param_2);
  *(undefined *)(param_2 + 0x54) = 1;
  pthread_cond_signal((pthread_cond_t *)(param_1 + 6));
  pthread_mutex_unlock((pthread_mutex_t *)(param_1 + 1));
  return;
}

// Function: queue_pop
long queue_pop(undefined8 *param_1) {
  long lVar1;
  
  pthread_mutex_lock((pthread_mutex_t *)(param_1 + 1));
  while( true ) {
    lVar1 = heap_remove_top(*param_1);
    if (lVar1 != 0) break;
    pthread_cond_wait((pthread_cond_t *)(param_1 + 6),(pthread_mutex_t *)(param_1 + 1));
  }
  pthread_mutex_unlock((pthread_mutex_t *)(param_1 + 1));
  lock_node(lVar1);
  *(undefined *)(lVar1 + 0x54) = 0;
  return lVar1;
}

// Function: write_unique
void write_unique(long *param_1,undefined8 param_2,undefined8 param_3) {
  int iVar1;
  
  if (unique != '\0') {
    if ((saved_line != 0) && (iVar1 = compare(param_1,&saved_line), iVar1 == 0)) {
      return;
    }
    saved_line = *param_1;
    _DAT_0010be28 = param_1[1];
    _DAT_0010be30 = param_1[2];
    _DAT_0010be38 = param_1[3];
  }
  write_line(param_1,param_2,param_3);
  return;
}

// Function: mergelines_node
void mergelines_node(long *param_1,ulong param_2,undefined8 param_3,undefined8 param_4) {
  long lVar1;
  long lVar2;
  long lVar3;
  undefined8 *puVar4;
  undefined8 uVar5;
  int iVar6;
  bool bVar7;
  long local_38;
  undefined8 *local_30;
  
  lVar2 = *param_1;
  lVar3 = param_1[1];
  local_38 = (param_2 >> (((char)*(undefined4 *)(param_1 + 10) + '\x01') * '\x02' & 0x3fU)) + 1;
  if (*(uint *)(param_1 + 10) < 2) {
    while (((*param_1 != param_1[2] && (param_1[1] != param_1[3])) &&
           (lVar1 = local_38 + -1, bVar7 = local_38 != 0, local_38 = lVar1, bVar7))) {
      iVar6 = compare(*param_1 + -0x20,param_1[1] + -0x20);
      if (iVar6 < 1) {
        *param_1 = *param_1 + -0x20;
        write_unique(*param_1,param_3,param_4);
      }
      else {
        param_1[1] = param_1[1] + -0x20;
        write_unique(param_1[1],param_3,param_4);
      }
    }
    if (lVar3 - param_1[1] >> 5 == param_1[6]) {
      while ((*param_1 != param_1[2] && (local_38 != 0))) {
        *param_1 = *param_1 + -0x20;
        write_unique(*param_1,param_3,param_4);
        local_38 = local_38 + -1;
      }
    }
    else if (lVar2 - *param_1 >> 5 == param_1[5]) {
      while ((param_1[1] != param_1[3] && (local_38 != 0))) {
        param_1[1] = param_1[1] + -0x20;
        write_unique(param_1[1],param_3,param_4);
        local_38 = local_38 + -1;
      }
    }
  }
  else {
    local_30 = *(undefined8 **)param_1[4];
    while (((*param_1 != param_1[2] && (param_1[1] != param_1[3])) &&
           (lVar1 = local_38 + -1, bVar7 = local_38 != 0, local_38 = lVar1, bVar7))) {
      iVar6 = compare(*param_1 + -0x20,param_1[1] + -0x20);
      if (iVar6 < 1) {
        *param_1 = *param_1 + -0x20;
        puVar4 = (undefined8 *)*param_1;
        uVar5 = puVar4[1];
        local_30[-4] = *puVar4;
        local_30[-3] = uVar5;
        uVar5 = puVar4[3];
        local_30[-2] = puVar4[2];
        local_30[-1] = uVar5;
        local_30 = local_30 + -4;
      }
      else {
        param_1[1] = param_1[1] + -0x20;
        puVar4 = (undefined8 *)param_1[1];
        uVar5 = puVar4[1];
        local_30[-4] = *puVar4;
        local_30[-3] = uVar5;
        uVar5 = puVar4[3];
        local_30[-2] = puVar4[2];
        local_30[-1] = uVar5;
        local_30 = local_30 + -4;
      }
    }
    if (lVar3 - param_1[1] >> 5 == param_1[6]) {
      while ((*param_1 != param_1[2] && (local_38 != 0))) {
        *param_1 = *param_1 + -0x20;
        puVar4 = (undefined8 *)*param_1;
        uVar5 = puVar4[1];
        local_30[-4] = *puVar4;
        local_30[-3] = uVar5;
        uVar5 = puVar4[3];
        local_30[-2] = puVar4[2];
        local_30[-1] = uVar5;
        local_38 = local_38 + -1;
        local_30 = local_30 + -4;
      }
    }
    else if (lVar2 - *param_1 >> 5 == param_1[5]) {
      while ((param_1[1] != param_1[3] && (local_38 != 0))) {
        param_1[1] = param_1[1] + -0x20;
        puVar4 = (undefined8 *)param_1[1];
        uVar5 = puVar4[1];
        local_30[-4] = *puVar4;
        local_30[-3] = uVar5;
        uVar5 = puVar4[3];
        local_30[-2] = puVar4[2];
        local_30[-1] = uVar5;
        local_38 = local_38 + -1;
        local_30 = local_30 + -4;
      }
    }
    *(undefined8 **)param_1[4] = local_30;
  }
  param_1[5] = param_1[5] - (lVar2 - *param_1 >> 5);
  param_1[6] = param_1[6] - (lVar3 - param_1[1] >> 5);
  return;
}

// Function: queue_check_insert
void queue_check_insert(undefined8 param_1,long *param_2) {
  bool bVar1;
  
  if (*(char *)((long)param_2 + 0x54) != '\x01') {
    if (*param_2 == param_2[2]) {
      if ((param_2[1] != param_2[3]) && (param_2[5] == 0)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
    }
    else if ((param_2[1] != param_2[3]) || (param_2[6] == 0)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (bVar1) {
      queue_insert(param_1,param_2);
    }
  }
  return;
}

// Function: queue_check_insert_parent
void queue_check_insert_parent(undefined8 param_1,long param_2) {
  if (*(uint *)(param_2 + 0x50) < 2) {
    if (*(long *)(param_2 + 0x30) + *(long *)(param_2 + 0x28) == 0) {
      queue_insert(param_1,*(undefined8 *)(param_2 + 0x38));
    }
  }
  else {
    lock_node(*(undefined8 *)(param_2 + 0x38));
    queue_check_insert(param_1,*(undefined8 *)(param_2 + 0x38));
    unlock_node(*(undefined8 *)(param_2 + 0x38));
  }
  return;
}

// Function: merge_loop
void merge_loop(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4) {
  long lVar1;
  
  while( true ) {
    lVar1 = queue_pop(param_1);
    if (*(int *)(lVar1 + 0x50) == 0) break;
    mergelines_node(lVar1,param_2,param_3,param_4);
    queue_check_insert(param_1,lVar1);
    queue_check_insert_parent(param_1,lVar1);
    unlock_node(lVar1);
  }
  unlock_node(lVar1);
  queue_insert(param_1,lVar1);
  return;
}

// Function: sortlines_thread
undefined8 sortlines_thread(undefined8 *param_1) {
  sortlines(*param_1,param_1[1],param_1[2],param_1[3],param_1[4],param_1[5],param_1[6]);
  return 0;
}

// Function: sortlines
void sortlines(long param_1,ulong param_2,long param_3,long *param_4,undefined8 param_5,
              undefined8 param_6,undefined8 param_7) {
  int iVar1;
  long in_FS_OFFSET;
  pthread_t local_80;
  ulong local_78;
  ulong local_70;
  long local_68;
  ulong local_60;
  ulong local_58;
  long local_50;
  long local_48;
  ulong local_40;
  long local_38;
  long local_30;
  undefined8 local_28;
  undefined8 local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_78 = param_4[6] + param_4[5];
  local_70 = param_2 >> 1;
  local_68 = param_2 - local_70;
  local_30 = param_4[8];
  local_18 = param_7;
  local_48 = param_1;
  local_40 = local_70;
  local_38 = param_3;
  local_28 = param_5;
  local_20 = param_6;
  if ((1 < param_2) && (0x1ffff < local_78)) {
    iVar1 = pthread_create(&local_80,(pthread_attr_t *)0x0,sortlines_thread,&local_48);
    if (iVar1 == 0) {
      sortlines(param_4[5] * -0x20 + param_1,local_68,param_3,param_4[9],param_5,param_6,param_7);
      pthread_join(local_80,(void **)0x0);
      goto code_r0x00108e4a;
    }
  }
  local_60 = param_4[5];
  local_58 = param_4[6];
  local_50 = param_1 + param_3 * -0x20;
  if (1 < local_58) {
    sequential_sort(local_60 * -0x20 + param_1,local_58,(local_60 >> 1) * -0x20 + local_50,0);
  }
  if (1 < local_60) {
    sequential_sort(param_1,local_60,local_50,0);
  }
  *param_4 = param_1;
  param_4[1] = local_60 * -0x20 + param_1;
  param_4[2] = local_60 * -0x20 + param_1;
  param_4[3] = local_60 * -0x20 + local_58 * -0x20 + param_1;
  queue_insert(param_5,param_4);
  merge_loop(param_5,param_3,param_6,param_7);
code_r0x00108e4a:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: avoid_trashing_input
void avoid_trashing_input(long param_1,ulong param_2,ulong param_3,long param_4) {
  char cVar1;
  char cVar2;
  bool bVar3;
  int iVar4;
  long in_FS_OFFSET;
  undefined8 local_c8;
  long local_c0;
  ulong local_b8;
  long local_b0;
  stat local_a8;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_c0 = 0;
  for (local_b8 = param_2; local_b8 < param_3; local_b8 = local_b8 + 1) {
    cVar1 = streq(*(undefined8 *)(param_1 + local_b8 * 0x10),&DAT_0010d83b);
    if (((param_4 == 0) ||
        (cVar2 = streq(param_4,*(undefined8 *)(param_1 + local_b8 * 0x10)), cVar2 == '\0')) ||
       (cVar1 == '\x01')) {
      local_b0 = get_outstatus();
      if (local_b0 == 0) break;
      if (cVar1 == '\0') {
        iVar4 = stat(*(char **)(param_1 + local_b8 * 0x10),&local_a8);
      }
      else {
        iVar4 = fstat(0,&local_a8);
      }
      if ((iVar4 == 0) && (cVar1 = psame_inode(&local_a8,local_b0), cVar1 != '\0')) {
        bVar3 = true;
      }
      else {
        bVar3 = false;
      }
    }
    else {
      bVar3 = true;
    }
    if (bVar3) {
      if (local_c0 == 0) {
        local_c0 = create_temp(&local_c8);
        mergefiles(local_b8 * 0x10 + param_1,0,1,local_c8,local_c0 + 0xd);
      }
      *(long *)(param_1 + local_b8 * 0x10) = local_c0 + 0xd;
      *(long *)(local_b8 * 0x10 + param_1 + 8) = local_c0;
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: check_inputs
void check_inputs(long param_1,ulong param_2) {
  undefined8 uVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  ulong local_20;
  
  for (local_20 = 0; local_20 < param_2; local_20 = local_20 + 1) {
    cVar2 = streq(*(undefined8 *)(param_1 + local_20 * 8),&DAT_0010d83b);
    if (cVar2 == '\0') {
      iVar3 = euidaccess(*(char **)(param_1 + local_20 * 8),4);
      if (iVar3 != 0) {
        uVar1 = *(undefined8 *)(param_1 + local_20 * 8);
        uVar4 = gettext("cannot read");
        sort_die(uVar4,uVar1);
      }
    }
  }
  return;
}

// Function: check_output
void check_output(char *param_1) {
  int iVar1;
  undefined8 uVar2;
  
  if (param_1 != (char *)0x0) {
    iVar1 = open(param_1,0x80041,0x1b6);
    if (iVar1 < 0) {
      uVar2 = gettext("open failed");
      sort_die(uVar2,param_1);
    }
    move_fd(iVar1,1);
  }
  return;
}

// Function: merge
void merge(long *param_1,ulong param_2,ulong param_3,undefined8 param_4) {
  long lVar1;
  int *piVar2;
  undefined8 uVar3;
  ulong uVar4;
  long in_FS_OFFSET;
  ulong local_b0;
  ulong local_a8;
  long local_90;
  undefined8 local_88;
  long local_80;
  ulong local_78;
  ulong local_70;
  long local_68;
  long local_60;
  ulong local_58;
  ulong local_50;
  ulong local_48;
  long local_40;
  ulong local_38;
  long local_30;
  ulong local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_a8 = param_2;
  for (local_b0 = param_3; nmerge < local_b0; local_b0 = local_b0 + (local_78 - local_80)) {
    local_78 = 0;
    for (local_80 = 0; (ulong)nmerge <= local_b0 - local_80; local_80 = local_80 + local_28) {
      local_30 = create_temp(&local_88);
      uVar4 = local_a8;
      if (nmerge <= local_a8) {
        uVar4 = (ulong)nmerge;
      }
      local_28 = mergefiles(param_1 + local_80 * 2,uVar4,nmerge,local_88,local_30 + 0xd);
      uVar4 = local_a8;
      if (local_28 <= local_a8) {
        uVar4 = local_28;
      }
      local_a8 = local_a8 - uVar4;
      param_1[local_78 * 2] = local_30 + 0xd;
      param_1[local_78 * 2 + 1] = local_30;
      local_78 = local_78 + 1;
    }
    local_58 = local_b0 - local_80;
    local_50 = (ulong)nmerge - local_78 % (ulong)nmerge;
    if (local_50 < local_58) {
      local_48 = (local_58 - local_50) + 1;
      local_40 = create_temp(&local_88);
      uVar4 = local_a8;
      if (local_48 <= local_a8) {
        uVar4 = local_48;
      }
      local_38 = mergefiles(param_1 + local_80 * 2,uVar4,local_48,local_88,local_40 + 0xd);
      uVar4 = local_a8;
      if (local_38 <= local_a8) {
        uVar4 = local_38;
      }
      local_a8 = local_a8 - uVar4;
      param_1[local_78 * 2] = local_40 + 0xd;
      param_1[local_78 * 2 + 1] = local_40;
      local_80 = local_80 + local_38;
      local_78 = local_78 + 1;
    }
    memmove(param_1 + local_78 * 2,param_1 + local_80 * 2,(local_b0 - local_80) * 0x10);
    local_a8 = local_a8 + local_78;
  }
  avoid_trashing_input(param_1,local_a8,local_b0,param_4);
  do {
    local_70 = open_input_files(param_1,local_b0,&local_90);
    if (local_70 == local_b0) {
      local_68 = stream_open(param_4,&DAT_0010d922);
      if (local_68 != 0) {
        mergefps(param_1,local_a8,local_b0,local_68,param_4,local_90);
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        return;
      }
      piVar2 = __errno_location();
      if ((*piVar2 != 0x18) || (local_70 < 3)) {
        uVar3 = gettext("open failed");
        sort_die(uVar3,param_4);
      }
    }
    else if (local_70 < 3) {
      lVar1 = param_1[local_70 * 2];
      uVar3 = gettext("open failed");
      sort_die(uVar3,lVar1);
    }
    do {
      local_70 = local_70 - 1;
      xfclose(*(undefined8 *)(local_90 + local_70 * 8),param_1[local_70 * 2]);
      local_60 = maybe_create_temp(&local_88,2 < local_70);
    } while (local_60 == 0);
    uVar4 = local_a8;
    if (local_70 <= local_a8) {
      uVar4 = local_70;
    }
    mergefps(param_1,uVar4,local_70,local_88,local_60 + 0xd,local_90);
    uVar4 = local_a8;
    if (local_70 <= local_a8) {
      uVar4 = local_70;
    }
    *param_1 = local_60 + 0xd;
    param_1[1] = local_60;
    memmove(param_1 + 2,param_1 + local_70 * 2,(local_b0 - local_70) * 0x10);
    local_a8 = (local_a8 - uVar4) + 1;
    local_b0 = (local_b0 - local_70) + 1;
  } while( true );
}

// Function: sort
void sort(undefined8 *param_1,long param_2,long param_3,ulong param_4) {
  bool bVar1;
  char cVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  long local_138;
  undefined8 *local_130;
  undefined8 local_120;
  undefined8 local_118;
  long local_110;
  long local_108;
  long local_100;
  ulong local_f8;
  long local_f0;
  undefined8 *local_e8;
  long local_e0;
  undefined8 local_d8;
  long local_d0;
  long local_c8;
  void *local_c0;
  void *local_b8;
  long local_b0;
  ulong local_a8;
  long local_a0;
  long local_98;
  char local_88;
  undefined local_78 [104];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_110 = 0;
  bVar1 = false;
  local_a0 = 0;
  local_138 = param_2;
  local_130 = param_1;
  do {
    if (local_138 == 0) {
LAB_00109b0d:
      free(local_b8);
      if (!bVar1) {
        local_e8 = temphead;
        local_c0 = (void *)xnmalloc(local_110,0x10);
        local_e0 = 0;
        for (; local_e8 != (undefined8 *)0x0; local_e8 = (undefined8 *)*local_e8) {
          *(long *)((long)local_c0 + local_e0 * 0x10) = (long)local_e8 + 0xd;
          *(undefined8 **)((long)local_c0 + local_e0 * 0x10 + 8) = local_e8;
          local_e0 = local_e0 + 1;
        }
        merge(local_c0,local_110,local_110,param_3);
        free(local_c0);
      }
      reap_all();
      if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
        return;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    local_d8 = *local_130;
    local_120 = xfopen(local_d8,&DAT_0010d96d);
    if (param_4 < 2) {
      local_100 = 0x30;
    }
    else {
      local_f0 = 1;
      for (local_f8 = 1; local_f8 < param_4; local_f8 = local_f8 << 1) {
        local_f0 = local_f0 + 1;
      }
      local_100 = local_f0 << 5;
    }
    if (local_a0 == 0) {
      uVar3 = sort_buffer_size(&local_120,1,local_130,local_138,local_100);
      initbuf(&local_b8,local_100,uVar3);
    }
    local_88 = '\0';
    local_130 = local_130 + 1;
    local_138 = local_138 + -1;
    while (cVar2 = fillbuf(&local_b8,local_120,local_d8), cVar2 != '\0') {
      if (((local_88 != '\0') && (local_138 != 0)) &&
         (local_100 + 1U < (local_a0 - local_b0) - local_a8 * local_100)) {
        local_98 = local_b0;
        break;
      }
      saved_line = 0;
      local_d0 = buffer_linelim(&local_b8);
      if (((local_88 == '\0') || (local_138 != 0)) || ((local_110 != 0 || (local_98 != 0)))) {
        local_110 = local_110 + 1;
        local_108 = create_temp(&local_118);
        local_108 = local_108 + 0xd;
      }
      else {
        xfclose(local_120,local_d8);
        local_118 = xfopen(param_3,&DAT_0010d922);
        bVar1 = true;
        local_108 = param_3;
      }
      if (local_a8 < 2) {
        write_unique(local_d0 + -0x20,local_118,local_108);
      }
      else {
        queue_init(local_78,param_4);
        local_c8 = merge_tree_init(param_4,local_a8,local_d0);
        sortlines(local_d0,param_4,local_a8,local_c8 + 0x80,local_78,local_118,local_108);
        merge_tree_destroy(param_4,local_c8);
        queue_destroy(local_78);
      }
      xfclose(local_118,local_108);
      if (bVar1) goto LAB_00109b0d;
    }
    xfclose(local_120,local_d8);
  } while( true );
}

// Function: insertkey
void insertkey(undefined8 param_1) {
  long lVar1;
  long *local_18;
  
  lVar1 = xmemdup(param_1,0x48);
  for (local_18 = &keylist; *local_18 != 0; local_18 = (long *)(*local_18 + 0x40)) {
  }
  *local_18 = lVar1;
  *(undefined8 *)(lVar1 + 0x40) = 0;
  return;
}

// Function: badfieldspec
void badfieldspec(undefined8 param_1,undefined8 param_2) {
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  
  uVar1 = quote(param_1);
  uVar2 = gettext(param_2);
  uVar3 = gettext("%s: invalid field specification %s");
                    /* WARNING: Subroutine does not return */
  error(2,0,uVar3,uVar2,uVar1);
}

// Function: incompatible_options
void incompatible_options(undefined8 param_1) {
  undefined8 uVar1;
  
  uVar1 = gettext("options \'-%s\' are incompatible");
                    /* WARNING: Subroutine does not return */
  error(2,0,uVar1,param_1);
}

// Function: check_ordering_compatibility
void check_ordering_compatibility(void) {
  long in_FS_OFFSET;
  long local_40;
  undefined local_38 [40];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  for (local_40 = keylist; local_40 != 0; local_40 = *(long *)(local_40 + 0x40)) {
    if (1 < (uint)(byte)(*(long *)(local_40 + 0x20) != 0 |
                        *(byte *)(local_40 + 0x33) | *(byte *)(local_40 + 0x38)) +
            (uint)*(byte *)(local_40 + 0x32) + (uint)*(byte *)(local_40 + 0x34) +
            (uint)*(byte *)(local_40 + 0x35) + (uint)*(byte *)(local_40 + 0x36)) {
      *(undefined *)(local_40 + 0x37) = 0;
      *(undefined *)(local_40 + 0x31) = *(undefined *)(local_40 + 0x37);
      *(undefined *)(local_40 + 0x30) = *(undefined *)(local_40 + 0x31);
      key_to_opts(local_40,local_38);
      incompatible_options(local_38);
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: parse_field_count
undefined8 parse_field_count(undefined8 param_1,long *param_2,long param_3) {
  undefined4 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  undefined8 local_38;
  long local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  uVar1 = xstrtoumax(param_1,&local_38,10,&local_30,&DAT_0010c60c);
  switch(uVar1) {
  case 0:
  case 2:
    *param_2 = local_30;
    if (*param_2 == local_30) break;
  case 1:
  case 3:
    *param_2 = -1;
    break;
  case 4:
    if (param_3 != 0) {
      uVar2 = quote(param_1);
      uVar3 = gettext(param_3);
      local_28 = gettext("%s: invalid count at start of %s");
                    /* WARNING: Subroutine does not return */
      error(2,0,local_28,uVar3,uVar2);
    }
    local_38 = 0;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_38;
}

// Function: set_ordering
char * set_ordering(char *param_1,long param_2,int param_3) {
  char *local_10;
  
  for (local_10 = param_1; *local_10 != '\0'; local_10 = local_10 + 1) {
    switch(*local_10) {
    case 'M':
      *(undefined *)(param_2 + 0x36) = 1;
      break;
    default:
      goto LAB_0010a09d;
    case 'R':
      *(undefined *)(param_2 + 0x33) = 1;
      break;
    case 'V':
      *(undefined *)(param_2 + 0x38) = 1;
      break;
    case 'b':
      if ((param_3 == 0) || (param_3 == 2)) {
        *(undefined *)(param_2 + 0x30) = 1;
      }
      if ((param_3 == 1) || (param_3 == 2)) {
        *(undefined *)(param_2 + 0x31) = 1;
      }
      break;
    case 'd':
      *(undefined1 **)(param_2 + 0x20) = nondictionary;
      break;
    case 'f':
      *(undefined1 **)(param_2 + 0x28) = fold_toupper;
      break;
    case 'g':
      *(undefined *)(param_2 + 0x34) = 1;
      break;
    case 'h':
      *(undefined *)(param_2 + 0x35) = 1;
      break;
    case 'i':
      if (*(long *)(param_2 + 0x20) == 0) {
        *(undefined1 **)(param_2 + 0x20) = nonprinting;
      }
      break;
    case 'n':
      *(undefined *)(param_2 + 0x32) = 1;
      break;
    case 'r':
      *(undefined *)(param_2 + 0x37) = 1;
    }
  }
LAB_0010a09d:
  return local_10;
}

// Function: key_init
void * key_init(void *param_1) {
  memset(param_1,0,0x48);
  *(undefined8 *)((long)param_1 + 0x10) = 0xffffffffffffffff;
  return param_1;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  bool bVar1;
  byte bVar2;
  bool bVar3;
  bool bVar4;
  char cVar5;
  int iVar6;
  uint uVar7;
  char *pcVar8;
  char *pcVar9;
  lconv *plVar10;
  __sighandler_t p_Var11;
  long *plVar12;
  undefined8 uVar13;
  undefined8 uVar14;
  long lVar15;
  ulong uVar16;
  long in_FS_OFFSET;
  bool bVar17;
  char local_428;
  bool local_426;
  char local_423;
  int local_420;
  long *local_3a0;
  char *local_398;
  char *local_390;
  ulong local_388;
  ulong local_380;
  undefined8 *local_378;
  char *local_370;
  char *local_368;
  ulong local_360;
  char *local_358;
  ulong local_350;
  ulong local_348;
  undefined local_288 [80];
  undefined8 local_238 [4];
  undefined8 local_218;
  undefined8 local_210;
  undefined local_208;
  undefined local_207;
  undefined local_206;
  byte local_205;
  undefined local_204;
  undefined local_203;
  undefined local_202;
  undefined local_201;
  undefined local_200;
  undefined4 local_1e8;
  undefined4 uStack_1e4;
  undefined8 *local_1e0;
  sigaction local_c8;
  undefined local_22;
  undefined local_21;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  local_420 = 0;
  local_428 = '\0';
  bVar3 = false;
  local_390 = (char *)0x0;
  local_426 = false;
  local_388 = 0;
  local_380 = 0;
  pcVar8 = getenv("POSIXLY_CORRECT");
  iVar6 = posix2_version();
  if ((iVar6 < 0x30db0) || (0x31068 < iVar6)) {
    bVar1 = true;
  }
  else {
    bVar1 = false;
  }
  local_370 = (char *)0x0;
  local_368 = (char *)0x0;
  set_program_name(*param_2);
  pcVar9 = setlocale(6,"");
  bVar17 = pcVar9 != (char *)0x0;
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(2);
  hard_LC_COLLATE = hard_locale(3);
  hard_LC_TIME = hard_locale(2);
  plVar10 = localeconv();
  decimal_point = *plVar10->decimal_point;
  if ((decimal_point == '\0') || (plVar10->decimal_point[1] != '\0')) {
    decimal_point = '.';
  }
  thousands_sep = (int)*plVar10->thousands_sep;
  if ((thousands_sep != 0) && (plVar10->thousands_sep[1] != '\0')) {
    thousands_sep_ignored = 1;
  }
  if ((thousands_sep == 0) || (plVar10->thousands_sep[1] != '\0')) {
    thousands_sep = 0x80;
  }
  have_read_stdin = '\0';
  inittables();
  sigemptyset((sigset_t *)&caught_signals);
  for (local_360 = 0; local_360 < 10; local_360 = local_360 + 1) {
    sigaction(*(int *)(sig_2 + local_360 * 4),(sigaction *)0x0,&local_c8);
    if (local_c8.__sigaction_handler.sa_handler != (__sighandler_t)0x1) {
      sigaddset((sigset_t *)&caught_signals,*(int *)(sig_2 + local_360 * 4));
    }
  }
  local_c8.__sigaction_handler.sa_handler = sighandler;
  local_c8.sa_mask.__val[0] = caught_signals;
  local_c8.sa_mask.__val[1] = DAT_0010c2a8;
  local_c8.sa_mask.__val[2] = DAT_0010c2b0;
  local_c8.sa_mask.__val[3] = DAT_0010c2b8;
  local_c8.sa_mask.__val[4] = DAT_0010c2c0;
  local_c8.sa_mask.__val[5] = DAT_0010c2c8;
  local_c8.sa_mask.__val[6] = DAT_0010c2d0;
  local_c8.sa_mask.__val[7] = DAT_0010c2d8;
  local_c8.sa_mask.__val[8] = DAT_0010c2e0;
  local_c8.sa_mask.__val[9] = DAT_0010c2e8;
  local_c8.sa_mask.__val[10] = DAT_0010c2f0;
  local_c8.sa_mask.__val[11] = DAT_0010c2f8;
  local_c8.sa_mask.__val[12] = DAT_0010c300;
  local_c8.sa_mask.__val[13] = DAT_0010c308;
  local_c8.sa_mask.__val[14] = DAT_0010c310;
  local_c8.sa_mask.__val[15] = DAT_0010c318;
  local_c8.sa_flags = 0;
  for (local_360 = 0; local_360 < 10; local_360 = local_360 + 1) {
    iVar6 = sigismember((sigset_t *)&caught_signals,*(int *)(sig_2 + local_360 * 4));
    if (iVar6 != 0) {
      sigaction(*(int *)(sig_2 + local_360 * 4),&local_c8,(sigaction *)0x0);
    }
  }
  signal(0x11,(__sighandler_t)0x0);
  p_Var11 = signal(0xd,(__sighandler_t)0x1);
  default_SIGPIPE = p_Var11 == (__sighandler_t)0x0;
  atexit(exit_cleanup);
  key_init(local_238);
  local_238[0] = 0xffffffffffffffff;
  local_378 = (undefined8 *)xnmalloc((long)param_1,8);
  do {
    while( true ) {
      while ((local_1e8 = 0xffffffff, local_420 == -1 ||
             ((((pcVar8 != (char *)0x0 && (local_380 != 0)) &&
               (((!bVar1 ||
                 ((((local_428 != '\0' || (param_1 == _optind)) ||
                   (*(char *)param_2[_optind] != '-')) || (*(char *)(param_2[_optind] + 1) != 'o')))
                 ) || ((*(char *)(param_2[_optind] + 2) == '\0' && (param_1 == _optind + 1)))))) ||
              (local_420 = getopt_long(param_1,param_2,short_options,long_options,&local_1e8),
              local_420 == -1))))) {
        if (param_1 <= _optind) {
          if (local_370 != (char *)0x0) {
            if (local_380 != 0) {
              uVar13 = quotearg_style(4,*local_378);
              uVar14 = gettext("extra operand %s");
                    /* WARNING: Subroutine does not return */
              error(0,0,uVar14,uVar13);
            }
            uVar13 = xfopen(local_370,&DAT_0010d96d);
            readtokens0_init(&local_1e8);
            cVar5 = readtokens0(uVar13,&local_1e8);
            if (cVar5 != '\x01') {
              uVar13 = quotearg_style(4,local_370);
              uVar14 = gettext("cannot read file names from %s");
                    /* WARNING: Subroutine does not return */
              error(2,0,uVar14,uVar13);
            }
            xfclose(uVar13,local_370);
            if (CONCAT44(uStack_1e4,local_1e8) == 0) {
              uVar13 = quotearg_style(4,local_370);
              uVar14 = gettext("no input from %s");
                    /* WARNING: Subroutine does not return */
              error(2,0,uVar14,uVar13);
            }
            free(local_378);
            local_378 = local_1e0;
            local_380 = CONCAT44(uStack_1e4,local_1e8);
            for (local_350 = 0; local_350 < local_380; local_350 = local_350 + 1) {
              cVar5 = streq(local_1e0[local_350],&DAT_0010d83b);
              if (cVar5 != '\0') {
                uVar13 = quotearg_style(4,local_1e0[local_350]);
                uVar14 = gettext(
                                "when reading file names from standard input, no file name of %s allowed"
                                );
                    /* WARNING: Subroutine does not return */
                error(2,0,uVar14,uVar13);
              }
              if (*(char *)local_1e0[local_350] == '\0') {
                uVar13 = quotearg_n_style_colon(0,3,local_370);
                uVar14 = gettext("%s:%lu: invalid zero-length file name");
                    /* WARNING: Subroutine does not return */
                error(2,0,uVar14,uVar13,local_350 + 1);
              }
            }
          }
          for (local_3a0 = (long *)keylist; local_3a0 != (long *)0x0;
              local_3a0 = *(long **)((long)local_3a0 + 0x40)) {
            cVar5 = default_key_compare(local_3a0);
            if ((cVar5 != '\0') && (*(char *)((long)local_3a0 + 0x37) != '\x01')) {
              *(undefined8 *)((long)local_3a0 + 0x20) = local_218;
              *(undefined8 *)((long)local_3a0 + 0x28) = local_210;
              *(undefined *)((long)local_3a0 + 0x30) = local_208;
              *(undefined *)((long)local_3a0 + 0x31) = local_207;
              *(undefined *)((long)local_3a0 + 0x36) = local_202;
              *(undefined *)((long)local_3a0 + 0x32) = local_206;
              *(undefined *)((long)local_3a0 + 0x34) = local_204;
              *(undefined *)((long)local_3a0 + 0x35) = local_203;
              *(undefined *)((long)local_3a0 + 0x38) = local_200;
              *(byte *)((long)local_3a0 + 0x33) = local_205;
              *(undefined *)((long)local_3a0 + 0x37) = local_201;
            }
            local_426 = (*(byte *)((long)local_3a0 + 0x33) | local_426) != 0;
          }
          if ((keylist == 0) && (cVar5 = default_key_compare(local_238), cVar5 != '\x01')) {
            insertkey(local_238);
            local_426 = (local_205 | local_426) != 0;
          }
          check_ordering_compatibility();
          if (debug != '\0') {
            if ((local_428 != '\0') || (local_368 != (char *)0x0)) {
              if (local_428 == '\0') {
                local_428 = 'o';
              }
              opts_1[0] = local_428;
              incompatible_options(opts_1);
            }
            if (bVar17) {
              pcVar8 = setlocale(3,"");
              bVar17 = pcVar8 != (char *)0x0;
            }
            if (!bVar17) {
              uVar13 = gettext("failed to set locale");
                    /* WARNING: Subroutine does not return */
              error(0,0,&DAT_0010db64,uVar13);
            }
            if (hard_LC_COLLATE != '\0') {
              pcVar8 = setlocale(3,(char *)0x0);
              uVar13 = quote(pcVar8);
              uVar14 = gettext("text ordering performed using %s sorting rules");
                    /* WARNING: Subroutine does not return */
              error(0,0,uVar14,uVar13);
            }
            uVar13 = gettext("text ordering performed using simple byte comparison");
                    /* WARNING: Subroutine does not return */
            error(0,0,&DAT_0010db64,uVar13);
          }
          reverse = local_201;
          if (local_426 != false) {
            random_md5_state_init(local_390);
          }
          if (temp_dir_count == 0) {
            pcVar8 = getenv("TMPDIR");
            if (pcVar8 == (char *)0x0) {
              pcVar8 = "/tmp";
            }
            add_temp_dir(pcVar8);
          }
          if (local_380 == 0) {
            local_380 = 1;
            free(local_378);
            local_378 = (undefined8 *)xmalloc(8);
            *local_378 = &DAT_0010d83b;
          }
          if ((sort_size != 0) && (sort_size <= (ulong)nmerge * 0x22)) {
            sort_size = (ulong)nmerge * 0x22;
          }
          if (local_428 != '\0') {
            if (1 < local_380) {
              uVar13 = quotearg_style(4,local_378[1]);
              uVar14 = gettext("extra operand %s not allowed with -%c");
                    /* WARNING: Subroutine does not return */
              error(2,0,uVar14,uVar13,(int)local_428);
            }
            if (local_368 != (char *)0x0) {
              opts_0._0_1_ = local_428;
              incompatible_options(&opts_0);
            }
            uVar7 = check(*local_378,(int)local_428);
                    /* WARNING: Subroutine does not return */
            exit((uVar7 ^ 1) & 0xff);
          }
          check_inputs(local_378,local_380);
          check_output(local_368);
          if (bVar3) {
            lVar15 = xcalloc(local_380,0x10);
            for (local_348 = 0; local_348 < local_380; local_348 = local_348 + 1) {
              *(undefined8 *)(lVar15 + local_348 * 0x10) = local_378[local_348];
            }
            merge(lVar15,0,local_380,local_368);
          }
          else {
            if ((local_388 == 0) && (local_388 = num_processors(2), 8 < local_388)) {
              local_388 = 8;
            }
            uVar16 = 0xffffffffffffff;
            if (local_388 < 0x100000000000000) {
              uVar16 = local_388;
            }
            sort(local_378,local_380,local_368,uVar16);
          }
          if ((have_read_stdin != '\0') && (iVar6 = rpl_fclose(_stdin), iVar6 == -1)) {
            uVar13 = gettext("close failed");
            sort_die(uVar13,&DAT_0010d83b);
          }
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        lVar15 = (long)_optind;
        _optind = _optind + 1;
        local_378[local_380] = param_2[lVar15];
        local_380 = local_380 + 1;
      }
      if (local_420 < 0x88) break;
switchD_0010a75d_caseD_2:
      usage(2);
    }
    if (local_420 < 1) {
      if (local_420 == -0x83) {
LAB_0010b2ba:
        uVar13 = proper_name_lite("Paul Eggert","Paul Eggert");
        uVar14 = proper_name_lite("Mike Haertel","Mike Haertel");
        version_etc(_stdout,&DAT_0010d603,"GNU coreutils",_Version,uVar14,uVar13,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (local_420 == -0x82) {
        usage(0);
        goto LAB_0010b2ba;
      }
      goto switchD_0010a75d_caseD_2;
    }
    cVar5 = (char)local_420;
    switch(local_420) {
    case 1:
      local_3a0 = (long *)0x0;
      if (*_optarg == '+') {
        if (((param_1 == _optind) || (*(char *)param_2[_optind] != '-')) ||
           (cVar5 = c_isdigit((int)*(char *)(param_2[_optind] + 1)), cVar5 == '\0')) {
          bVar4 = false;
        }
        else {
          bVar4 = true;
        }
        if ((!bVar4) || (pcVar8 != (char *)0x0)) {
          bVar2 = 0;
        }
        else {
          bVar2 = 1;
        }
        bVar1 = (bool)(bVar1 | bVar2);
        if (bVar1) {
          local_3a0 = (long *)key_init(local_288);
          local_398 = (char *)parse_field_count(_optarg + 1,local_3a0,0);
          if ((local_398 != (char *)0x0) && (*local_398 == '.')) {
            local_398 = (char *)parse_field_count(local_398 + 1,local_3a0 + 1,0);
          }
          if ((*local_3a0 == 0) && (local_3a0[1] == 0)) {
            *local_3a0 = -1;
          }
          if ((local_398 == (char *)0x0) ||
             (pcVar9 = (char *)set_ordering(local_398,local_3a0,0), *pcVar9 != '\0')) {
            local_3a0 = (long *)0x0;
          }
          else {
            if (bVar4) {
              lVar15 = param_2[_optind];
              _optind = _optind + 1;
              local_398 = (char *)parse_field_count(lVar15 + 1,local_3a0 + 2,
                                                    "invalid number after \'-\'");
              if (*local_398 == '.') {
                local_398 = (char *)parse_field_count(local_398 + 1,local_3a0 + 3,
                                                      "invalid number after \'.\'");
              }
              if ((local_3a0[3] == 0) && (local_3a0[2] != 0)) {
                local_3a0[2] = local_3a0[2] + -1;
              }
              pcVar9 = (char *)set_ordering(local_398,local_3a0,1);
              if (*pcVar9 != '\0') {
                badfieldspec(lVar15,"stray character in field spec");
              }
            }
            *(undefined *)((long)local_3a0 + 0x39) = 1;
            insertkey(local_3a0);
          }
        }
      }
      if (local_3a0 == (long *)0x0) {
        local_378[local_380] = _optarg;
        local_380 = local_380 + 1;
      }
      break;
    default:
      goto switchD_0010a75d_caseD_2;
    case 0x53:
      specify_sort_size(local_1e8,(int)cVar5,_optarg);
      break;
    case 0x54:
      add_temp_dir(_optarg);
      break;
    case 0x6b:
      plVar12 = (long *)key_init(local_288);
      local_398 = (char *)parse_field_count(_optarg,plVar12,"invalid number at field start");
      lVar15 = *plVar12;
      *plVar12 = lVar15 + -1;
      if (lVar15 == 0) {
        badfieldspec(_optarg,"field number is zero");
      }
      if (*local_398 == '.') {
        local_398 = (char *)parse_field_count(local_398 + 1,plVar12 + 1,"invalid number after \'.\'"
                                             );
        lVar15 = plVar12[1];
        plVar12[1] = lVar15 + -1;
        if (lVar15 == 0) {
          badfieldspec(_optarg,"character offset is zero");
        }
      }
      if ((*plVar12 == 0) && (plVar12[1] == 0)) {
        *plVar12 = -1;
      }
      local_398 = (char *)set_ordering(local_398,plVar12,0);
      if (*local_398 == ',') {
        local_398 = (char *)parse_field_count(local_398 + 1,plVar12 + 2,"invalid number after \',\'"
                                             );
        lVar15 = plVar12[2];
        plVar12[2] = lVar15 + -1;
        if (lVar15 == 0) {
          badfieldspec(_optarg,"field number is zero");
        }
        if (*local_398 == '.') {
          local_398 = (char *)parse_field_count(local_398 + 1,plVar12 + 3,
                                                "invalid number after \'.\'");
        }
        local_398 = (char *)set_ordering(local_398,plVar12,1);
      }
      else {
        plVar12[2] = -1;
        plVar12[3] = 0;
      }
      if (*local_398 != '\0') {
        badfieldspec(_optarg,"stray character in field spec");
      }
      insertkey(plVar12);
      break;
    case 0x6d:
      bVar3 = true;
      break;
    case 0x6f:
      if ((local_368 != (char *)0x0) && (cVar5 = streq(local_368,_optarg), cVar5 != '\x01')) {
        uVar13 = gettext("multiple output files specified");
                    /* WARNING: Subroutine does not return */
        error(2,0,uVar13);
      }
      local_368 = _optarg;
      break;
    case 0x73:
      stable = 1;
      break;
    case 0x74:
      local_423 = *_optarg;
      if (local_423 == '\0') {
        uVar13 = gettext("empty tab");
                    /* WARNING: Subroutine does not return */
        error(2,0,uVar13);
      }
      if (_optarg[1] != '\0') {
        cVar5 = streq(_optarg,&DAT_0010e19c);
        if (cVar5 == '\0') {
          uVar13 = quote(_optarg);
          uVar14 = gettext("multi-character tab %s");
                    /* WARNING: Subroutine does not return */
          error(2,0,uVar14,uVar13);
        }
        local_423 = '\0';
      }
      if ((tab != 0x80) && (local_423 != tab)) {
        uVar13 = gettext("incompatible tabs");
                    /* WARNING: Subroutine does not return */
        error(2,0,uVar13);
      }
      tab = (int)local_423;
      break;
    case 0x75:
      unique = 1;
      break;
    case 0x79:
      if ((char *)param_2[(long)_optind + -1] == _optarg) {
        local_358 = _optarg;
        while (cVar5 = c_isdigit((int)*local_358), cVar5 != '\0') {
          local_358 = local_358 + 1;
        }
        _optind = _optind - (uint)(*local_358 != '\0');
      }
      break;
    case 0x7a:
      eolchar = 0;
      break;
    case 0x80:
      if (_optarg == (char *)0x0) {
        local_420 = 99;
      }
      else {
        lVar15 = __xargmatch_internal
                           ("--check",_optarg,check_args,"CCcgeneral-numeric",1,_argmatch_die,1);
        local_420 = (int)"CCcgeneral-numeric"[lVar15];
      }
    case 0x43:
    case 99:
      if ((local_428 != '\0') && (local_420 != local_428)) {
        incompatible_options(&DAT_0010e0b7);
      }
      local_428 = (char)local_420;
      break;
    case 0x81:
      if ((compress_program != (char *)0x0) &&
         (cVar5 = streq(compress_program,_optarg), cVar5 != '\x01')) {
        uVar13 = gettext("multiple compress programs specified");
                    /* WARNING: Subroutine does not return */
        error(2,0,uVar13);
      }
      compress_program = _optarg;
      break;
    case 0x82:
      debug = '\x01';
      break;
    case 0x83:
      local_370 = _optarg;
      break;
    case 0x84:
      specify_nmerge(local_1e8,(int)cVar5,_optarg);
      break;
    case 0x85:
      if ((local_390 != (char *)0x0) && (cVar5 = streq(local_390,_optarg), cVar5 != '\x01')) {
        uVar13 = gettext("multiple random sources specified");
                    /* WARNING: Subroutine does not return */
        error(2,0,uVar13);
      }
      local_390 = _optarg;
      break;
    case 0x86:
      lVar15 = __xargmatch_internal
                         ("--sort",_optarg,sort_args,"ghMnRVwaiting for %s [-d]",1,_argmatch_die,1);
      local_420 = (int)"ghMnRVwaiting for %s [-d]"[lVar15];
    case 0x4d:
    case 0x52:
    case 0x56:
    case 0x62:
    case 100:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6e:
    case 0x72:
      local_22 = (undefined)local_420;
      local_21 = 0;
      set_ordering(&local_22,local_238,2);
      break;
    case 0x87:
      local_388 = specify_nthreads(local_1e8,(int)cVar5,_optarg);
    }
  } while( true );
}

