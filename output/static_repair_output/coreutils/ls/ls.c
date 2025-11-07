// Function: file_or_link_mode
undefined4 file_or_link_mode(long param_1) {
  undefined4 uVar1;
  
  if ((color_symlink_as_referent == '\0') || (*(char *)(param_1 + 0xb9) == '\0')) {
    uVar1 = *(undefined4 *)(param_1 + 0x30);
  }
  else {
    uVar1 = *(undefined4 *)(param_1 + 0xac);
  }
  return uVar1;
}

// Function: dired_outbyte
void dired_outbyte(char param_1) {
  dired_pos = dired_pos + 1;
  putchar_unlocked((int)param_1);
  return;
}

// Function: dired_outbuf
void dired_outbuf(void *param_1,size_t param_2) {
  dired_pos = param_2 + dired_pos;
  fwrite_unlocked(param_1,1,param_2,_stdout);
  return;
}

// Function: dired_outstring
void dired_outstring(char *param_1) {
  size_t sVar1;
  
  sVar1 = strlen(param_1);
  dired_outbuf(param_1,sVar1);
  return;
}

// Function: dired_indent
void dired_indent(void) {
  if (dired != '\0') {
    dired_outstring(&DAT_0010dd04);
  }
  return;
}

// Function: push_current_dired_pos
void push_current_dired_pos(long param_1) {
  if (dired != '\0') {
    if ((ulong)(*(long *)(param_1 + 0x20) - *(long *)(param_1 + 0x18)) < 8) {
      rpl_obstack_newchunk(param_1,8);
    }
    memcpy(*(void **)(param_1 + 0x18),&dired_pos,8);
    *(long *)(param_1 + 0x18) = *(long *)(param_1 + 0x18) + 8;
  }
  return;
}

// Function: dev_ino_push
void dev_ino_push(undefined8 param_1,undefined8 param_2) {
  undefined8 *puVar1;
  undefined8 uVar2;
  
  if ((ulong)(dev_ino_obstack._32_8_ - dev_ino_obstack._24_8_) < 0x10) {
    rpl_obstack_newchunk(dev_ino_obstack,0x10);
  }
  uVar2 = dev_ino_obstack._24_8_;
  puVar1 = (undefined8 *)(dev_ino_obstack._24_8_ + 8);
  dev_ino_obstack._24_8_ = dev_ino_obstack._24_8_ + 0x10;
  *puVar1 = param_1;
  *(undefined8 *)uVar2 = param_2;
  return;
}

// Function: dev_ino_pop
undefined  [16] dev_ino_pop(void) {
  if ((ulong)(dev_ino_obstack._24_8_ - dev_ino_obstack._16_8_) < 0x10) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("dev_ino_size <= __extension__ ({ struct obstack const *__o = (&dev_ino_obstack); (size_t) (__o->next_free - __o->object_base); })"
                  ,"/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                  0x441,"dev_ino_pop");
  }
  dev_ino_obstack._24_8_ = dev_ino_obstack._24_8_ + -0x10;
  return *(undefined (*) [16])dev_ino_obstack._24_8_;
}

// Function: assert_matching_dev_ino
void assert_matching_dev_ino(char *param_1,__ino_t param_2,__dev_t param_3) {
  int iVar1;
  long in_FS_OFFSET;
  stat local_a8;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  iVar1 = stat(param_1,&local_a8);
  if (iVar1 < 0) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("0 <= stat (name, &sb)",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                  0x44c,"assert_matching_dev_ino");
  }
  if (local_a8.st_dev != param_3) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("sb.st_dev == di.st_dev",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                  0x44d,"assert_matching_dev_ino");
  }
  if (local_a8.st_ino != param_2) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("sb.st_ino == di.st_ino",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                  0x44e,"assert_matching_dev_ino");
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: dired_dump_obstack
void dired_dump_obstack(char *param_1,long param_2) {
  long lVar1;
  ulong uVar2;
  ulong local_40;
  
  uVar2 = (ulong)(*(long *)(param_2 + 0x18) - *(long *)(param_2 + 0x10)) >> 3;
  if (uVar2 != 0) {
    lVar1 = *(long *)(param_2 + 0x10);
    if (lVar1 == *(long *)(param_2 + 0x18)) {
      *(byte *)(param_2 + 0x50) = *(byte *)(param_2 + 0x50) | 2;
    }
    *(ulong *)(param_2 + 0x18) =
         *(long *)(param_2 + 0x18) + (*(ulong *)(param_2 + 0x30) & -*(long *)(param_2 + 0x18));
    *(undefined8 *)(param_2 + 0x10) = *(undefined8 *)(param_2 + 0x18);
    fputs_unlocked(param_1,_stdout);
    for (local_40 = 0; local_40 < uVar2; local_40 = local_40 + 1) {
      printf(" %jd",*(undefined8 *)(lVar1 + local_40 * 8));
    }
    putchar_unlocked(10);
  }
  return;
}

// Function: get_stat_btime
undefined  [16] get_stat_btime(undefined8 param_1) {
  undefined auVar1 [16];
  
  auVar1 = get_stat_mtime(param_1);
  return auVar1;
}

// Function: time_type_to_statx
uint time_type_to_statx(void) {
  uint uVar1;
  
  if (time_type == 3) {
    uVar1 = 0x800;
  }
  else {
    uVar1 = time_type;
    if (time_type < 4) {
      if (time_type == 2) {
        uVar1 = 0x20;
      }
      else if (time_type < 3) {
        if (time_type == 0) {
          uVar1 = 0x40;
        }
        else if (time_type == 1) {
          uVar1 = 0x80;
        }
      }
    }
  }
  return uVar1;
}

// Function: calc_req_mask
void calc_req_mask(void) {
  if (format == 0) {
    time_type_to_statx();
  }
                    /* WARNING: Could not recover jumptable at 0x00100c67. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(&DAT_0010de24 + *(int *)(&DAT_0010de24 + (ulong)sort_type * 4)))();
  return;
}

// Function: do_statx
int do_statx(undefined4 param_1,undefined8 param_2,long param_3,uint param_4,uint param_5) {
  int iVar1;
  long in_FS_OFFSET;
  undefined auVar2 [16];
  uint local_128 [20];
  undefined8 local_d8;
  undefined8 local_d0;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  iVar1 = statx(param_1,param_2,param_4 | 0x800,param_5,local_128);
  if (-1 < iVar1) {
    statx_to_stat(local_128,param_3);
    if ((param_5 & 0x800) != 0) {
      if ((local_128[0] & 0x800) == 0) {
        *(undefined8 *)(param_3 + 0x60) = 0xffffffffffffffff;
        *(undefined8 *)(param_3 + 0x58) = *(undefined8 *)(param_3 + 0x60);
      }
      else {
        auVar2 = statx_timestamp_to_timespec(local_d8,local_d0);
        *(undefined (*) [16])(param_3 + 0x58) = auVar2;
      }
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return iVar1;
}

// Function: do_stat
void do_stat(undefined8 param_1,undefined8 param_2) {
  undefined4 uVar1;
  
  uVar1 = calc_req_mask();
  do_statx(0xffffff9c,param_1,param_2,0,uVar1);
  return;
}

// Function: do_lstat
void do_lstat(undefined8 param_1,undefined8 param_2) {
  undefined4 uVar1;
  
  uVar1 = calc_req_mask();
  do_statx(0xffffff9c,param_1,param_2,0x100,uVar1);
  return;
}

// Function: stat_for_mode
void stat_for_mode(undefined8 param_1,undefined8 param_2) {
  do_statx(0xffffff9c,param_1,param_2,0,2);
  return;
}

// Function: stat_for_ino
void stat_for_ino(undefined8 param_1,undefined8 param_2) {
  do_statx(0xffffff9c,param_1,param_2,0,0x100);
  return;
}

// Function: fstat_for_ino
void fstat_for_ino(undefined4 param_1,undefined8 param_2) {
  do_statx(param_1,&DAT_0010c134,param_2,0x1000,0x100);
  return;
}

// Function: first_percent_b
char * first_percent_b(char *param_1) {
  char *local_10;
  
  local_10 = param_1;
  do {
    if (*local_10 == '\0') {
      return (char *)0x0;
    }
    if (*local_10 == '%') {
      if (local_10[1] == '%') {
        local_10 = local_10 + 1;
      }
      else if (local_10[1] == 'b') {
        return local_10;
      }
    }
    local_10 = local_10 + 1;
  } while( true );
}

// Function: file_escape_init
void file_escape_init(void) {
  char cVar1;
  byte bVar2;
  int local_c;
  
  for (local_c = 0; local_c < 0x100; local_c = local_c + 1) {
    cVar1 = c_isalnum(local_c);
    if ((((cVar1 == '\0') && (local_c != 0x7e)) && (local_c != 0x2d)) &&
       ((local_c != 0x2e && (local_c != 0x5f)))) {
      bVar2 = 0;
    }
    else {
      bVar2 = 1;
    }
    RFC3986[local_c] = RFC3986[local_c] | bVar2;
  }
  return;
}

// Function: abmon_init
undefined8 abmon_init(long param_1) {
  char cVar1;
  char *pcVar2;
  size_t sVar3;
  char *pcVar4;
  undefined8 uVar5;
  int iVar6;
  long in_FS_OFFSET;
  int local_94;
  int local_90;
  int local_8c;
  int local_88;
  int aiStack_78 [12];
  int aiStack_48 [14];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_94 = 0;
  for (local_90 = 0; local_90 < 0xc; local_90 = local_90 + 1) {
    pcVar2 = (char *)rpl_nl_langinfo(local_90 + 0x2000e);
    sVar3 = strnlen(pcVar2,0x80);
    aiStack_48[local_90] = (int)sVar3;
    if (aiStack_48[local_90] == 0x80) {
      uVar5 = 0;
      goto LAB_00101264;
    }
    pcVar4 = strchr(pcVar2,0x25);
    if (pcVar4 != (char *)0x0) {
      uVar5 = 0;
      goto LAB_00101264;
    }
    pcVar2 = strcpy((char *)((long)local_90 * 0x80 + param_1),pcVar2);
    iVar6 = gnu_mbswidth(pcVar2,3);
    aiStack_78[local_90] = iVar6;
    if (aiStack_78[local_90] < 0) {
      uVar5 = 0;
      goto LAB_00101264;
    }
    iVar6 = aiStack_78[local_90];
    if (aiStack_78[local_90] <= local_94) {
      iVar6 = local_94;
    }
    local_94 = iVar6;
  }
  local_8c = 0;
  do {
    if (0xb < local_8c) {
      uVar5 = 1;
LAB_00101264:
      if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return uVar5;
    }
    iVar6 = local_94 - aiStack_78[local_8c];
    if (0x80 - aiStack_48[local_8c] <= iVar6) {
      uVar5 = 0;
      goto LAB_00101264;
    }
    cVar1 = c_isdigit((int)*(char *)(param_1 + (long)local_8c * 0x80));
    if (cVar1 == '\0') {
      local_88 = aiStack_48[local_8c];
    }
    else {
      memmove((void *)((long)local_8c * 0x80 + (long)iVar6 + param_1),
              (void *)((long)local_8c * 0x80 + param_1),(long)aiStack_48[local_8c]);
      local_88 = 0;
    }
    memset((void *)((long)local_8c * 0x80 + (long)local_88 + param_1),0x20,(long)iVar6);
    *(undefined *)((long)local_8c * 0x80 + param_1 + (long)(iVar6 + aiStack_48[local_8c])) = 0;
    local_8c = local_8c + 1;
  } while( true );
}

// Function: abformat_init
void abformat_init(void) {
  char cVar1;
  long lVar2;
  long in_FS_OFFSET;
  int local_64c;
  int local_648;
  int local_644;
  int local_640;
  long local_628 [2];
  undefined local_618 [1544];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  for (local_64c = 0; local_64c < 2; local_64c = local_64c + 1) {
    lVar2 = first_percent_b(*(undefined8 *)(long_time_format + (long)local_64c * 8));
    local_628[local_64c] = lVar2;
  }
  if (((local_628[0] != 0) || (local_628[1] != 0)) &&
     (cVar1 = abmon_init(local_618), cVar1 == '\x01')) {
    for (local_648 = 0; local_648 < 2; local_648 = local_648 + 1) {
      lVar2 = *(long *)(long_time_format + (long)local_648 * 8);
      for (local_644 = 0; local_644 < 0xc; local_644 = local_644 + 1) {
        if (local_628[local_648] == 0) {
          local_640 = snprintf(abformat + ((long)local_648 * 0xc + (long)local_644) * 0x80,0x80,"%s"
                               ,lVar2);
        }
        else {
          if (0x80 < local_628[local_648] - lVar2) goto LAB_001014df;
          local_640 = snprintf(abformat + ((long)local_648 * 0xc + (long)local_644) * 0x80,0x80,
                               "%.*s%s%s",(ulong)(uint)((int)local_628[local_648] - (int)lVar2),
                               lVar2,local_618 + (long)local_644 * 0x80,local_628[local_648] + 2);
        }
        if ((local_640 < 0) || (0x7f < local_640)) goto LAB_001014df;
      }
    }
    use_abformat = 1;
  }
LAB_001014df:
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: dev_ino_hash
ulong dev_ino_hash(ulong *param_1,ulong param_2) {
  return *param_1 % param_2;
}

// Function: dev_ino_compare
undefined8 dev_ino_compare(ulong *param_1,ulong *param_2) {
  ulong uVar1;
  
  uVar1 = *param_2 ^ *param_1 | param_1[1] ^ param_2[1];
  return CONCAT71((int7)(uVar1 >> 8),uVar1 == 0);
}

// Function: dev_ino_free
void dev_ino_free(void *param_1) {
  free(param_1);
  return;
}

// Function: visit_dir
bool visit_dir(undefined8 param_1,undefined8 param_2) {
  undefined8 *__ptr;
  undefined8 *puVar1;
  
  __ptr = (undefined8 *)xmalloc(0x10);
  *__ptr = param_2;
  __ptr[1] = param_1;
  puVar1 = (undefined8 *)hash_insert(active_dir_set,__ptr);
  if (puVar1 == (undefined8 *)0x0) {
    xalloc_die();
  }
  if (puVar1 != __ptr) {
    free(__ptr);
  }
  return puVar1 != __ptr;
}

// Function: free_pending_ent
void free_pending_ent(void **param_1) {
  free(*param_1);
  free(param_1[1]);
  free(param_1);
  return;
}

// Function: is_colored
bool is_colored(uint param_1) {
  ulong uVar1;
  bool bVar2;
  
  uVar1 = (&color_indicator)[(ulong)param_1 * 2];
  if (uVar1 == 0) {
    bVar2 = false;
  }
  else if (uVar1 < 3) {
    bVar2 = (&PTR___compound_literal_0_00111108)[(ulong)param_1 * 2][uVar1 - 1] != '0' ||
            *(&PTR___compound_literal_0_00111108)[(ulong)param_1 * 2] != '0';
  }
  else {
    bVar2 = true;
  }
  return bVar2;
}

// Function: restore_default_color
void restore_default_color(void) {
  put_indicator(&color_indicator);
  put_indicator(&DAT_00111110);
  return;
}

// Function: set_normal_color
void set_normal_color(void) {
  char cVar1;
  
  if (print_with_color != '\0') {
    cVar1 = is_colored(4);
    if (cVar1 != '\0') {
      put_indicator(&color_indicator);
      put_indicator(&DAT_00111140);
      put_indicator(&DAT_00111110);
    }
  }
  return;
}

// Function: sighandler
void sighandler(int param_1) {
  if (interrupt_signal == 0) {
    interrupt_signal = param_1;
  }
  return;
}

// Function: stophandler
void stophandler(void) {
  if (interrupt_signal == 0) {
    stop_signal_count = stop_signal_count + 1;
  }
  return;
}

// Function: process_signals
void process_signals(void) {
  long in_FS_OFFSET;
  int local_a0;
  sigset_t local_98;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  while ((interrupt_signal != 0 || (stop_signal_count != 0))) {
    if (used_color != '\0') {
      restore_default_color();
    }
    fflush_unlocked(_stdout);
    sigprocmask(0,(sigset_t *)&caught_signals,&local_98);
    local_a0 = interrupt_signal;
    if (stop_signal_count == 0) {
      signal(interrupt_signal,(__sighandler_t)0x0);
    }
    else {
      stop_signal_count = stop_signal_count + -1;
      local_a0 = 0x13;
    }
    raise(local_a0);
    sigprocmask(2,&local_98,(sigset_t *)0x0);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: signal_setup
void signal_setup(char param_1) {
  int iVar1;
  long in_FS_OFFSET;
  int local_ac;
  sigaction local_a8;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (param_1 == '\0') {
    for (local_ac = 0; local_ac < 0xc; local_ac = local_ac + 1) {
      iVar1 = sigismember((sigset_t *)&caught_signals,*(int *)(sig_16 + (long)local_ac * 4));
      if (iVar1 != 0) {
        signal(*(int *)(sig_16 + (long)local_ac * 4),(__sighandler_t)0x0);
      }
    }
  }
  else {
    sigemptyset((sigset_t *)&caught_signals);
    for (local_ac = 0; local_ac < 0xc; local_ac = local_ac + 1) {
      sigaction(*(int *)(sig_16 + (long)local_ac * 4),(sigaction *)0x0,&local_a8);
      if (local_a8.__sigaction_handler.sa_handler != (__sighandler_t)0x1) {
        sigaddset((sigset_t *)&caught_signals,*(int *)(sig_16 + (long)local_ac * 4));
      }
    }
    local_a8.sa_mask.__val[0] = caught_signals;
    local_a8.sa_mask.__val[1] = DAT_0010af08;
    local_a8.sa_mask.__val[2] = DAT_0010af10;
    local_a8.sa_mask.__val[3] = DAT_0010af18;
    local_a8.sa_mask.__val[4] = DAT_0010af20;
    local_a8.sa_mask.__val[5] = DAT_0010af28;
    local_a8.sa_mask.__val[6] = DAT_0010af30;
    local_a8.sa_mask.__val[7] = DAT_0010af38;
    local_a8.sa_mask.__val[8] = DAT_0010af40;
    local_a8.sa_mask.__val[9] = DAT_0010af48;
    local_a8.sa_mask.__val[10] = DAT_0010af50;
    local_a8.sa_mask.__val[11] = DAT_0010af58;
    local_a8.sa_mask.__val[12] = DAT_0010af60;
    local_a8.sa_mask.__val[13] = DAT_0010af68;
    local_a8.sa_mask.__val[14] = DAT_0010af70;
    local_a8.sa_mask.__val[15] = DAT_0010af78;
    local_a8.sa_flags = 0x10000000;
    for (local_ac = 0; local_ac < 0xc; local_ac = local_ac + 1) {
      iVar1 = sigismember((sigset_t *)&caught_signals,*(int *)(sig_16 + (long)local_ac * 4));
      if (iVar1 != 0) {
        if (*(int *)(sig_16 + (long)local_ac * 4) == 0x14) {
          local_a8.__sigaction_handler.sa_handler = stophandler;
        }
        else {
          local_a8.__sigaction_handler.sa_handler = sighandler;
        }
        sigaction(*(int *)(sig_16 + (long)local_ac * 4),&local_a8,(sigaction *)0x0);
      }
    }
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: signal_init
void signal_init(void) {
  signal_setup(1);
  return;
}

// Function: signal_restore
void signal_restore(void) {
  signal_setup(0);
  return;
}

// Function: main
undefined4 main(int param_1,undefined8 *param_2) {
  int iVar1;
  long *plVar2;
  long *plVar3;
  undefined auVar4 [16];
  undefined auVar5 [16];
  long *plVar6;
  char cVar7;
  int iVar8;
  uint uVar9;
  char *pcVar10;
  long lVar11;
  long in_FS_OFFSET;
  undefined auVar12 [16];
  int local_44;
  int local_40;
  undefined local_28 [24];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(2);
  atexit((__func *)&close_stdout);
  exit_status = 0;
  print_dir_name = 1;
  pending_dirs = (long *)0x0;
  current_time = 0x8000000000000000;
  DAT_0010ae18 = 0xffffffffffffffff;
  iVar8 = decode_switches(param_1,param_2);
  if (print_with_color != 0) {
    parse_ls_color();
  }
  if (print_with_color != 0) {
    tabsize = 0;
  }
  if (directories_first == 0) {
    if ((print_with_color != 0) &&
       (((cVar7 = is_colored(0xd), cVar7 != '\0' ||
         ((cVar7 = is_colored(0xe), cVar7 != '\0' && (color_symlink_as_referent != '\0')))) ||
        ((cVar7 = is_colored(0xc), cVar7 != '\0' && (format == 0)))))) {
      check_symlink_mode = 1;
    }
  }
  else {
    check_symlink_mode = 1;
  }
  if (dereference == 0) {
    if (((immediate_dirs == '\0') && (indicator_style != 3)) && (format != 0)) {
      dereference = 3;
    }
    else {
      dereference = 1;
    }
  }
  if (recursive != 0) {
    active_dir_set = hash_initialize(0x1e,0,dev_ino_hash,dev_ino_compare,dev_ino_free);
    if (active_dir_set == 0) {
      xalloc_die();
    }
    rpl_obstack_begin(dev_ino_obstack,0,0,&malloc,free);
  }
  pcVar10 = getenv("TZ");
  localtz = tzalloc(pcVar10);
  format_needs_stat =
       (byte)(print_scontext |
             ((sort_type == 3 || sort_type == 5) || format == 0) | print_block_size |
             print_hyperlink) != 0;
  format_needs_type =
       ((indicator_style != 0 | directories_first | recursive | print_with_color | print_scontext) &
       !(bool)format_needs_stat) != 0;
  if ((print_with_color == 0) || (cVar7 = is_colored(0x15), cVar7 == '\0')) {
    format_needs_capability = 0;
  }
  else {
    format_needs_capability = 1;
  }
  if (dired != '\0') {
    rpl_obstack_begin(dired_obstack,0,0,&malloc,free);
    rpl_obstack_begin(subdired_obstack,0,0,&malloc,free);
  }
  if (print_hyperlink != 0) {
    file_escape_init();
    hostname = (undefined *)xgethostname();
    if (hostname == (undefined *)0x0) {
      hostname = &DAT_0010c134;
    }
  }
  cwd_n_alloc = 100;
  cwd_file = xmalloc(0x5140);
  cwd_n_used = 0;
  clear_files();
  local_44 = iVar8;
  if (param_1 - iVar8 < 1) {
    if (immediate_dirs == '\0') {
      queue_directory(&DAT_0010de67,0,1);
    }
    else {
      gobble_file(&DAT_0010de67,3,0,1,0);
    }
  }
  else {
    do {
      iVar1 = local_44 + 1;
      gobble_file(param_2[local_44],0,0,1,0);
      local_44 = iVar1;
    } while (iVar1 < param_1);
  }
  if ((cwd_n_used != 0) && (sort_files(), immediate_dirs != '\x01')) {
    extract_dirs_from_files(0,1);
  }
  auVar5._8_8_ = local_28._8_8_;
  auVar5._0_8_ = local_28._0_8_;
  auVar4._8_8_ = local_28._8_8_;
  auVar4._0_8_ = local_28._0_8_;
  auVar12._8_8_ = local_28._8_8_;
  auVar12._0_8_ = local_28._0_8_;
  if (cwd_n_used == 0) {
    if (((param_1 - iVar8 < 2) && (local_28._0_16_ = auVar12, pending_dirs != (long *)0x0)) &&
       (local_28._0_16_ = auVar4, pending_dirs[3] == 0)) {
      print_dir_name = 0;
      local_28._0_16_ = auVar5;
    }
  }
  else {
    print_current_files();
    if (pending_dirs != (long *)0x0) {
      dired_outbyte(10);
    }
  }
  while (plVar6 = pending_dirs, pending_dirs != (long *)0x0) {
    if ((active_dir_set == 0) || (*pending_dirs != 0)) {
      plVar2 = pending_dirs + 2;
      plVar3 = pending_dirs + 1;
      lVar11 = *pending_dirs;
      pending_dirs = (long *)pending_dirs[3];
      print_dir(lVar11,*plVar3,*(undefined *)plVar2);
      free_pending_ent(plVar6);
      print_dir_name = 1;
    }
    else {
      pending_dirs = (long *)pending_dirs[3];
      auVar12 = dev_ino_pop();
      local_28._0_16_ = auVar12;
      lVar11 = hash_remove(active_dir_set,local_28);
      if (lVar11 == 0) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("found",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                      0x73b,(char *)&__PRETTY_FUNCTION___15);
      }
      dev_ino_free(lVar11);
      free_pending_ent(plVar6);
    }
  }
  if ((print_with_color == 0) || (used_color == '\0')) goto LAB_0010225f;
  if (color_indicator == 2) {
    cVar7 = memeq(PTR___compound_literal_0_00111108,&DAT_0010de6f,2);
    if (((cVar7 != '\x01') || (DAT_00111110 != 1)) || (*PTR___compound_literal_1_00111118 != 'm'))
    goto LAB_0010220e;
  }
  else {
LAB_0010220e:
    restore_default_color();
  }
  fflush_unlocked(_stdout);
  signal_restore();
  for (local_40 = stop_signal_count; local_40 != 0; local_40 = local_40 + -1) {
    raise(0x13);
  }
  if (interrupt_signal != 0) {
    raise(interrupt_signal);
  }
LAB_0010225f:
  if (dired != '\0') {
    dired_dump_obstack("//DIRED//",dired_obstack);
    dired_dump_obstack("//SUBDIRED//",subdired_obstack);
    uVar9 = get_quoting_style(filename_quoting_options);
    printf("//DIRED-OPTIONS// --quoting-style=%s\n",
           *(undefined8 *)(&quoting_style_args + (ulong)uVar9 * 8));
  }
  if (active_dir_set != 0) {
    lVar11 = hash_get_n_entries(active_dir_set);
    if (lVar11 != 0) {
                    /* WARNING: Subroutine does not return */
      __assert_fail("hash_get_n_entries (active_dir_set) == 0",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                    0x76f,(char *)&__PRETTY_FUNCTION___15);
    }
    hash_free(active_dir_set);
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return exit_status;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: decode_line_length
long decode_line_length(undefined8 param_1) {
  uint uVar1;
  long lVar2;
  long in_FS_OFFSET;
  long local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  lVar2 = xstrtoumax(param_1,0,0,&local_18,&DAT_0010c134);
  uVar1 = (uint)lVar2;
  if (uVar1 < 5) {
    if (uVar1 < 2) {
      if (uVar1 == 0) {
        lVar2 = local_18;
        if (local_18 < 0) {
          lVar2 = 0;
        }
      }
      else if (uVar1 == 1) {
        lVar2 = 0;
      }
    }
    else {
      lVar2 = -1;
    }
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return lVar2;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: stdout_isatty
bool stdout_isatty(void) {
  int iVar1;
  
  if (out_tty_14 < '\0') {
    iVar1 = isatty(1);
    out_tty_14 = (char)iVar1;
  }
  return out_tty_14 != '\0';
}

// Function: decode_switches
undefined4 decode_switches(undefined4 param_1,undefined8 param_2) {
  bool bVar1;
  char cVar2;
  int iVar3;
  char **ppcVar4;
  undefined *puVar5;
  char *pcVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  char *pcVar9;
  long lVar10;
  long in_FS_OFFSET;
  int local_10c;
  int local_108;
  int local_104;
  int local_100;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  int local_ec;
  char *local_b8;
  long local_b0;
  ulong local_a8;
  ulong local_a0;
  char *local_98;
  char *local_90;
  char *local_88;
  undefined2 local_28;
  ushort uStack_26;
  undefined4 uStack_24;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_b8 = (char *)0x0;
  bVar1 = false;
  local_10c = -1;
  local_108 = -1;
  local_104 = -1;
  local_100 = -1;
  local_b0 = -1;
  local_a8 = 0xffffffffffffffff;
LAB_0010249b:
  local_28 = 0xffff;
  uStack_26 = 0xffff;
  iVar3 = getopt_long(param_1,param_2,"abcdfghiklmnopqrstuvw:xABCDFGHI:LNQRST:UXZ1",long_options,
                      &local_28);
  if (iVar3 != -1) {
    switch(iVar3) {
    case 0x31:
      if (local_10c != 0) {
        local_10c = 1;
      }
      break;
    case 0x41:
      ignore_mode = 1;
      break;
    case 0x42:
      add_ignore_pattern(&DAT_0010df2e);
      add_ignore_pattern(&DAT_0010df31);
      break;
    case 0x43:
      local_10c = 2;
      break;
    case 0x44:
      local_10c = 0;
      print_hyperlink = 0;
      dired = 1;
      break;
    case 0x46:
      if (_optarg == (char *)0x0) {
        local_fc = 1;
      }
      else {
        lVar10 = __xargmatch_internal("--classify",_optarg,when_args,when_types,4,_argmatch_die,1);
        local_fc = *(int *)(when_types + lVar10 * 4);
      }
      if ((local_fc == 1) || ((local_fc == 2 && (cVar2 = stdout_isatty(), cVar2 != '\0')))) {
        indicator_style = 3;
      }
      break;
    case 0x47:
      print_group = 0;
      break;
    case 0x48:
      dereference = 2;
      break;
    case 0x49:
      add_ignore_pattern(_optarg);
      break;
    case 0x4c:
      dereference = 4;
      break;
    case 0x4e:
      local_104 = 0;
      break;
    case 0x51:
      local_104 = 5;
      break;
    case 0x52:
      recursive = 1;
      break;
    case 0x53:
      local_100 = 3;
      break;
    case 0x54:
      uVar7 = gettext("invalid tab size");
      local_b0 = xnumtoumax(_optarg,0,0,0x7fffffffffffffff,&DAT_0010c134,uVar7,2,0);
      break;
    case 0x55:
      local_100 = 6;
      break;
    case 0x58:
      local_100 = 1;
      break;
    case 0x5a:
      print_scontext = 1;
      break;
    case 0x61:
      ignore_mode = 2;
      break;
    case 0x62:
      local_104 = 7;
      break;
    case 99:
      time_type = 1;
      explicit_time = '\x01';
      break;
    case 100:
      immediate_dirs = 1;
      break;
    case 0x66:
      ignore_mode = 2;
      local_100 = 6;
      break;
    case 0x67:
      local_10c = 0;
      print_owner = 0;
      break;
    case 0x68:
      human_output_opts = 0xb0;
      file_human_output_opts = 0xb0;
      output_block_size = 1;
      file_output_block_size = 1;
      break;
    case 0x69:
      print_inode = 1;
      break;
    case 0x6b:
      bVar1 = true;
      break;
    case 0x6c:
      local_10c = 0;
      break;
    case 0x6d:
      local_10c = 4;
      break;
    case 0x6e:
      numeric_ids = 1;
      local_10c = 0;
      break;
    case 0x6f:
      local_10c = 0;
      print_group = 0;
      break;
    case 0x70:
      indicator_style = 1;
      break;
    case 0x71:
      local_108 = 1;
      break;
    case 0x72:
      sort_reverse = 1;
      break;
    case 0x73:
      print_block_size = 1;
      break;
    case 0x74:
      local_100 = 5;
      break;
    case 0x75:
      time_type = 2;
      explicit_time = '\x01';
      break;
    case 0x76:
      local_100 = 4;
      break;
    case 0x77:
      local_a8 = decode_line_length(_optarg);
      if (-1 < (long)local_a8) break;
      uVar7 = quote(_optarg);
      uVar8 = gettext("invalid line width");
      error(2,0,"%s: %s",uVar8,uVar7);
    case 0x78:
      local_10c = 3;
      break;
    case 0x80:
      print_author = 1;
      break;
    case 0x81:
      iVar3 = human_options(_optarg,&human_output_opts,&output_block_size);
      if (iVar3 != 0) {
        xstrtol_fatal(iVar3,CONCAT22(uStack_26,local_28),0,long_options,_optarg);
      }
      file_human_output_opts = human_output_opts;
      file_output_block_size = output_block_size;
      break;
    case 0x82:
      if (_optarg == (char *)0x0) {
        local_f8 = 1;
      }
      else {
        lVar10 = __xargmatch_internal("--color",_optarg,when_args,when_types,4,_argmatch_die,1);
        local_f8 = *(int *)(when_types + lVar10 * 4);
      }
      if ((local_f8 == 1) || ((local_f8 == 2 && (cVar2 = stdout_isatty(), cVar2 != '\0')))) {
        print_with_color = '\x01';
      }
      else {
        print_with_color = '\0';
      }
      break;
    case 0x83:
      dereference = 3;
      break;
    case 0x84:
      indicator_style = 2;
      break;
    case 0x85:
      lVar10 = __xargmatch_internal("--format",_optarg,format_args,format_types,4,_argmatch_die,1);
      local_10c = *(int *)(format_types + lVar10 * 4);
      break;
    case 0x86:
      local_10c = 0;
      local_b8 = "full-iso";
      break;
    case 0x87:
      directories_first = 1;
      break;
    case 0x88:
      ppcVar4 = (char **)xmalloc(0x10);
      *ppcVar4 = _optarg;
      ppcVar4[1] = (char *)hide_patterns;
      hide_patterns = ppcVar4;
      break;
    case 0x89:
      if (_optarg == (char *)0x0) {
        local_f4 = 1;
      }
      else {
        lVar10 = __xargmatch_internal("--hyperlink",_optarg,when_args,when_types,4,_argmatch_die,1);
        local_f4 = *(int *)(when_types + lVar10 * 4);
      }
      if ((local_f4 == 1) || ((local_f4 == 2 && (cVar2 = stdout_isatty(), cVar2 != '\0')))) {
        print_hyperlink = 1;
      }
      else {
        print_hyperlink = 0;
      }
      break;
    case 0x8a:
      lVar10 = __xargmatch_internal
                         ("--indicator-style",_optarg,indicator_style_args,indicator_style_types,4,
                          _argmatch_die,1);
      indicator_style = *(uint *)(indicator_style_types + lVar10 * 4);
      break;
    case 0x8b:
      lVar10 = __xargmatch_internal
                         ("--quoting-style",_optarg,&quoting_style_args,&quoting_style_vals,4,
                          _argmatch_die,1);
      local_104 = *(int *)(&quoting_style_vals + lVar10 * 4);
      break;
    case 0x8c:
      local_108 = 0;
      break;
    case 0x8d:
      human_output_opts = 0x90;
      file_human_output_opts = 0x90;
      output_block_size = 1;
      file_output_block_size = 1;
      break;
    case 0x8e:
      lVar10 = __xargmatch_internal("--sort",_optarg,sort_args,sort_types,4,_argmatch_die,1);
      local_100 = *(int *)(sort_types + lVar10 * 4);
      break;
    case 0x8f:
      lVar10 = __xargmatch_internal("--time",_optarg,time_args,time_types,4,_argmatch_die,1);
      time_type = *(undefined4 *)(time_types + lVar10 * 4);
      explicit_time = '\x01';
      break;
    case 0x90:
      local_b8 = _optarg;
      break;
    case 0x91:
      goto switchD_00102528_caseD_91;
    case -0x82:
      usage(0);
    case -0x83:
      uVar7 = proper_name_lite("David MacKenzie","David MacKenzie");
      uVar8 = proper_name_lite("Richard M. Stallman","Richard M. Stallman");
      if (_ls_mode == 1) {
        puVar5 = &DAT_0010dfcb;
      }
      else if (_ls_mode == 2) {
        puVar5 = &DAT_0010dfc2;
      }
      else {
        puVar5 = &DAT_0010dfc6;
      }
      version_etc(_stdout,puVar5,"GNU coreutils",_Version,uVar8,uVar7,0);
                    /* WARNING: Subroutine does not return */
      exit(0);
    default:
      usage(2);
    }
    goto LAB_0010249b;
  }
  if (output_block_size == 0) {
    pcVar6 = getenv("LS_BLOCK_SIZE");
    human_options(pcVar6,&human_output_opts,&output_block_size);
    if ((pcVar6 != (char *)0x0) || (pcVar6 = getenv("BLOCK_SIZE"), pcVar6 != (char *)0x0)) {
      file_human_output_opts = human_output_opts;
      file_output_block_size = output_block_size;
    }
    if (bVar1) {
      human_output_opts = 0;
      output_block_size = 0x400;
    }
  }
  if (local_10c < 0) {
    if (_ls_mode == 1) {
      cVar2 = stdout_isatty();
      if (cVar2 == '\0') {
        local_10c = 1;
      }
      else {
        local_10c = 2;
      }
    }
    else if (_ls_mode == 2) {
      local_10c = 2;
    }
    else {
      local_10c = 0;
    }
  }
  local_a0 = local_a8;
  format = local_10c;
  if ((((local_10c == 2) || (local_10c == 3)) || (local_10c == 4)) || (print_with_color != '\0')) {
    if ((((long)local_a8 < 0) && (cVar2 = stdout_isatty(), cVar2 != '\0')) &&
       ((iVar3 = ioctl(1,0x5413,&local_28), -1 < iVar3 && (uStack_26 != 0)))) {
      local_a0 = (ulong)uStack_26;
    }
    if ((((long)local_a0 < 0) && (pcVar6 = getenv("COLUMNS"), pcVar6 != (char *)0x0)) &&
       ((*pcVar6 != '\0' && (local_a0 = decode_line_length(pcVar6), (long)local_a0 < 0)))) {
      uVar7 = quote(pcVar6);
      uVar8 = gettext("ignoring invalid width in environment variable COLUMNS: %s");
      error(0,0,uVar8,uVar7);
    }
  }
  if ((long)local_a0 < 0) {
    local_a0 = 0x50;
  }
  max_idx = local_a0 / 3 + (ulong)(local_a0 % 3 != 0);
  line_length = local_a0;
  if (((format == 2) || (format == 3)) || (format == 4)) {
    if (local_b0 < 0) {
      tabsize = 8;
      pcVar6 = getenv("TABSIZE");
      if (pcVar6 != (char *)0x0) {
        iVar3 = xstrtoumax(pcVar6,0,0,&local_28,&DAT_0010c134);
        if (iVar3 == 0) {
          tabsize = CONCAT44(uStack_24,CONCAT22(uStack_26,local_28));
        }
        else {
          uVar7 = quote(pcVar6);
          uVar8 = gettext("ignoring invalid tab size in environment variable TABSIZE: %s");
          error(0,0,uVar8,uVar7);
        }
      }
    }
    else {
      tabsize = local_b0;
    }
  }
  if (local_108 < 0) {
    if ((_ls_mode == 1) && (cVar2 = stdout_isatty(), cVar2 != '\0')) {
      qmark_funny_chars = true;
    }
    else {
      qmark_funny_chars = false;
    }
  }
  else {
    qmark_funny_chars = local_108 != 0;
  }
  local_f0 = local_104;
  if (local_104 < 0) {
    local_f0 = getenv_quoting_style();
  }
  if (local_f0 < 0) {
    if (_ls_mode == 1) {
      cVar2 = stdout_isatty();
      if (cVar2 == '\0') {
        local_f0 = -1;
      }
      else {
        local_f0 = 3;
      }
    }
    else {
      local_f0 = 7;
    }
  }
  if (-1 < local_f0) {
    set_quoting_style(0,local_f0);
  }
  iVar3 = get_quoting_style(0);
  if (((format == 0) || (((format == 2 || (format == 3)) && (line_length != 0)))) &&
     (((iVar3 == 1 || (iVar3 == 3)) || (iVar3 == 6)))) {
    align_variable_outer_quotes = 1;
  }
  else {
    align_variable_outer_quotes = 0;
  }
  filename_quoting_options = clone_quoting_options(0);
  if (iVar3 == 7) {
    set_char_quoting(filename_quoting_options,0x20,1);
  }
  if (1 < indicator_style) {
    for (local_98 = "*=>@|" + (indicator_style - 2); *local_98 != '\0'; local_98 = local_98 + 1) {
      set_char_quoting(filename_quoting_options,(int)*local_98,1);
    }
  }
  dirname_quoting_options = clone_quoting_options(0);
  set_char_quoting(dirname_quoting_options,0x3a,1);
  dired = ((print_hyperlink ^ 1) & format == 0 & dired) != 0;
  if ((int)eolbyte < (int)(uint)dired) {
    uVar7 = gettext("--dired and --zero are incompatible");
    error(2,0,uVar7);
  }
  if (local_100 < 0) {
    if ((format == 0) || (explicit_time == '\0')) {
      local_100 = 0;
    }
    else {
      local_100 = 5;
    }
  }
  sort_type = local_100;
  if (format == 0) {
    local_90 = local_b8;
    if ((local_b8 == (char *)0x0) && (local_90 = getenv("TIME_STYLE"), local_90 == (char *)0x0)) {
      local_90 = "locale";
    }
    while (iVar3 = strncmp(local_90,"posix-",6), iVar3 == 0) {
      cVar2 = hard_locale(2);
      if (cVar2 != '\x01') goto LAB_001037cd;
      local_90 = local_90 + 6;
    }
    if (*local_90 == '+') {
      local_90 = local_90 + 1;
      pcVar6 = strchr(local_90,10);
      local_88 = local_90;
      if (pcVar6 != (char *)0x0) {
        pcVar9 = strchr(pcVar6 + 1,10);
        if (pcVar9 != (char *)0x0) {
          uVar7 = quote(local_90);
          uVar8 = gettext("invalid time style format %s");
          error(2,0,uVar8,uVar7);
        }
        local_88 = pcVar6 + 1;
        *pcVar6 = '\0';
      }
      long_time_format._8_8_ = local_88;
      long_time_format._0_8_ = local_90;
    }
    else {
      lVar10 = x_timestyle_match(local_90,1,time_style_args,time_style_types,4,2);
      if (lVar10 == 3) {
        cVar2 = hard_locale(2);
        if (cVar2 != '\0') {
          for (local_ec = 0; local_ec < 2; local_ec = local_ec + 1) {
            uVar7 = dcgettext(0,*(undefined8 *)(long_time_format + (long)local_ec * 8),2);
            *(undefined8 *)(long_time_format + (long)local_ec * 8) = uVar7;
          }
        }
      }
      else if (lVar10 < 4) {
        if (lVar10 == 2) {
          long_time_format._0_8_ = s__Y__m__d_0010e103;
          long_time_format._8_8_ = s__m__d__H__M_0010e10d;
        }
        else if (lVar10 < 3) {
          if (lVar10 == 0) {
            long_time_format._8_8_ = s__Y__m__d__H__M__S__N__z_0010e0dc;
            long_time_format._0_8_ = s__Y__m__d__H__M__S__N__z_0010e0dc;
          }
          else if (lVar10 == 1) {
            long_time_format._8_8_ = s__Y__m__d__H__M_0010e0f4;
            long_time_format._0_8_ = s__Y__m__d__H__M_0010e0f4;
          }
        }
      }
    }
    abformat_init();
  }
LAB_001037cd:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return _optind;
switchD_00102528_caseD_91:
  eolbyte = '\0';
  local_108 = 0;
  if (local_10c != 0) {
    local_10c = 1;
  }
  print_with_color = '\0';
  local_104 = 0;
  goto LAB_0010249b;
}

// Function: get_funky_string
undefined8 get_funky_string(void) {
  undefined8 uVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0010384f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = (*(code *)((long)&switchD_0010384f::switchdataD_0010e570 +
                    (long)(int)switchD_0010384f::switchdataD_0010e570))();
  return uVar1;
}

// Function: known_term_type
undefined8 known_term_type(void) {
  int iVar1;
  char *__name;
  size_t sVar2;
  char *local_18;
  
  __name = getenv("TERM");
  if ((__name != (char *)0x0) && (*__name != '\0')) {
    for (local_18 = "# Configuration file for dircolors, a utility to help you set the";
        local_18 + -0x10c1e0 < (char *)0x15f0; local_18 = local_18 + sVar2 + 1) {
      iVar1 = strncmp(local_18,"TERM ",5);
      if ((iVar1 == 0) && (iVar1 = fnmatch(local_18 + 5,__name,0), iVar1 == 0)) {
        return 1;
      }
      sVar2 = strlen(local_18);
    }
  }
  return 0;
}

// Function: parse_ls_color
void parse_ls_color(void) {
  long lVar1;
  char cVar2;
  char *pcVar3;
  long in_FS_OFFSET;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  pcVar3 = getenv("LS_COLORS");
  if ((pcVar3 != (char *)0x0) && (*pcVar3 != '\0')) {
    color_buf = xstrdup(pcVar3);
                    /* WARNING: Could not recover jumptable at 0x00103d41. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)((long)&switchD_00103d41::switchdataD_0010e724 + (long)(int)UINT_0010e728))();
    return;
  }
  pcVar3 = getenv("COLORTERM");
  if (((pcVar3 == (char *)0x0) || (*pcVar3 == '\0')) && (cVar2 = known_term_type(), cVar2 != '\x01')
     ) {
    print_with_color = 0;
  }
  if (lVar1 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: getenv_quoting_style
undefined4 getenv_quoting_style(void) {
  int iVar1;
  undefined4 uVar2;
  char *pcVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  
  pcVar3 = getenv("QUOTING_STYLE");
  if (pcVar3 == (char *)0x0) {
    uVar2 = 0xffffffff;
  }
  else {
    iVar1 = argmatch(pcVar3,&quoting_style_args,&quoting_style_vals,4);
    if (iVar1 < 0) {
      uVar4 = quote(pcVar3);
      uVar5 = gettext("ignoring invalid value of environment variable QUOTING_STYLE: %s");
      error(0,0,uVar5,uVar4);
      uVar2 = 0xffffffff;
    }
    else {
      uVar2 = *(undefined4 *)(&quoting_style_vals + (long)iVar1 * 4);
    }
  }
  return uVar2;
}

// Function: set_exit_status
void set_exit_status(char param_1) {
  if (param_1 == '\0') {
    if (exit_status == 0) {
      exit_status = 1;
    }
  }
  else {
    exit_status = 2;
  }
  return;
}

// Function: file_failure
void file_failure(undefined param_1,undefined8 param_2,undefined8 param_3) {
  undefined8 uVar1;
  int *piVar2;
  
  uVar1 = quotearg_style(4,param_3);
  piVar2 = __errno_location();
  error(0,*piVar2,param_2,uVar1);
  set_exit_status(param_1);
  return;
}

// Function: queue_directory
void queue_directory(long param_1,long param_2,undefined param_3) {
  undefined8 *puVar1;
  undefined8 uVar2;
  
  puVar1 = (undefined8 *)xmalloc(0x20);
  if (param_2 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = xstrdup(param_2);
  }
  puVar1[1] = uVar2;
  if (param_1 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = xstrdup(param_1);
  }
  *puVar1 = uVar2;
  *(undefined *)(puVar1 + 2) = param_3;
  puVar1[3] = pending_dirs;
  pending_dirs = puVar1;
  return;
}

// Function: print_dir
void print_dir(char *param_1,char *param_2,undefined param_3) {
  char cVar1;
  int iVar2;
  int *piVar3;
  DIR *__dirp;
  undefined8 uVar4;
  char extraout_var;
  char extraout_var_00;
  undefined8 uVar5;
  char *pcVar6;
  dirent *pdVar7;
  long lVar8;
  char *__s;
  size_t sVar9;
  long in_FS_OFFSET;
  long local_380;
  void *local_378;
  undefined8 local_348;
  undefined8 local_340;
  undefined auStack_2b7 [663];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_380 = 0;
  piVar3 = __errno_location();
  *piVar3 = 0;
  __dirp = opendir(param_1);
  if (__dirp == (DIR *)0x0) {
    uVar4 = gettext("cannot open directory %s");
    file_failure(param_3,uVar4,param_1);
  }
  else {
    if (active_dir_set == 0) {
LAB_001046a7:
      clear_files();
      if ((recursive != '\0') || (print_dir_name != '\0')) {
        if (first_11 != '\x01') {
          dired_outbyte(10);
        }
        first_11 = '\0';
        dired_indent();
        local_378 = (void *)0x0;
        if ((print_hyperlink != '\0') &&
           (local_378 = (void *)canonicalize_filename_mode(param_1,2), local_378 == (void *)0x0)) {
          uVar4 = gettext("error canonicalizing %s");
          file_failure(param_3,uVar4,param_1);
        }
        pcVar6 = param_1;
        if (param_2 != (char *)0x0) {
          pcVar6 = param_2;
        }
        quote_name(pcVar6,dirname_quoting_options,0xffffffff,0,1,subdired_obstack,local_378);
        free(local_378);
        dired_outstring(&DAT_0010e819);
      }
      do {
        piVar3 = __errno_location();
        *piVar3 = 0;
        pdVar7 = readdir(__dirp);
        if (pdVar7 == (dirent *)0x0) {
          piVar3 = __errno_location();
          iVar2 = *piVar3;
          if (iVar2 == 0) goto LAB_001048fd;
          uVar4 = gettext("reading directory %s");
          file_failure(param_3,uVar4,param_1);
          if (iVar2 != 0x4b) goto LAB_001048fd;
        }
        else {
          cVar1 = file_ignored(pdVar7->d_name);
          if (cVar1 != '\x01') {
            lVar8 = gobble_file(pdVar7->d_name,(int)(char)d_type_filetype[(int)(uint)pdVar7->d_type]
                                ,0,0,param_1);
            local_380 = local_380 + lVar8;
            if ((((format == 1) && (sort_type == 6)) && (print_block_size != '\x01')) &&
               (recursive != '\x01')) {
              sort_files();
              print_current_files();
              clear_files();
            }
          }
        }
        process_signals();
      } while( true );
    }
    iVar2 = dirfd(__dirp);
    if (iVar2 < 0) {
      stat_for_ino(param_1,&local_348);
      cVar1 = extraout_var_00;
    }
    else {
      fstat_for_ino(iVar2,&local_348);
      cVar1 = extraout_var;
    }
    if (cVar1 < '\0') {
      uVar4 = gettext("cannot determine device and inode of %s");
      file_failure(param_3,uVar4,param_1);
      closedir(__dirp);
    }
    else {
      cVar1 = visit_dir(local_348,local_340);
      if (cVar1 == '\0') {
        dev_ino_push(local_348,local_340);
        goto LAB_001046a7;
      }
      uVar4 = quotearg_n_style_colon(0,3,param_1);
      uVar5 = gettext("%s: not listing already-listed directory");
      error(0,0,uVar5,uVar4);
      closedir(__dirp);
      set_exit_status(1);
    }
  }
  goto LAB_00104a4a;
LAB_001048fd:
  iVar2 = closedir(__dirp);
  if (iVar2 != 0) {
    uVar4 = gettext("closing directory %s");
    file_failure(param_3,uVar4,param_1);
  }
  sort_files();
  if (recursive != '\0') {
    extract_dirs_from_files(param_1,0);
  }
  if ((format == 0) || (print_block_size != '\0')) {
    __s = (char *)human_readable(local_380,auStack_2b7,human_output_opts,0x200,output_block_size);
    sVar9 = strlen(__s);
    pcVar6 = __s + -1;
    *pcVar6 = ' ';
    __s[sVar9] = eolbyte;
    dired_indent();
    uVar4 = gettext("total");
    dired_outstring(uVar4);
    dired_outbuf(pcVar6,__s + sVar9 + (1 - (long)pcVar6));
  }
  if (cwd_n_used != 0) {
    print_current_files();
  }
LAB_00104a4a:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: add_ignore_pattern
void add_ignore_pattern(undefined8 param_1) {
  undefined8 *puVar1;
  
  puVar1 = (undefined8 *)xmalloc(0x10);
  *puVar1 = param_1;
  puVar1[1] = ignore_patterns;
  ignore_patterns = puVar1;
  return;
}

// Function: patterns_match
undefined8 patterns_match(char **param_1,char *param_2) {
  int iVar1;
  char **local_10;
  
  local_10 = param_1;
  while( true ) {
    if (local_10 == (char **)0x0) {
      return 0;
    }
    iVar1 = fnmatch(*local_10,param_2,4);
    if (iVar1 == 0) break;
    local_10 = (char **)local_10[1];
  }
  return 1;
}

// Function: file_ignored
undefined4 file_ignored(char *param_1) {
  char cVar1;
  long lVar2;
  
  if ((ignore_mode != 2) && (*param_1 == '.')) {
    if (ignore_mode == 0) {
      return 1;
    }
    if (param_1[1] == '.') {
      lVar2 = 2;
    }
    else {
      lVar2 = 1;
    }
    if (param_1[lVar2] == '\0') {
      return 1;
    }
  }
  if (((ignore_mode != 0) || (cVar1 = patterns_match(hide_patterns,param_1), cVar1 == '\0')) &&
     (cVar1 = patterns_match(ignore_patterns,param_1), cVar1 == '\0')) {
    return 0;
  }
  return 1;
}

// Function: unsigned_file_size
undefined8 unsigned_file_size(undefined8 param_1) {
  return param_1;
}

// Function: has_capability
undefined8 has_capability(void) {
  int *piVar1;
  
  piVar1 = __errno_location();
  *piVar1 = 0x5f;
  return 0;
}

// Function: free_ent
void free_ent(void **param_1) {
  free(*param_1);
  free(param_1[1]);
  free(param_1[2]);
  return;
}

// Function: clear_files
void clear_files(void) {
  long local_18;
  
  for (local_18 = 0; local_18 < cwd_n_used; local_18 = local_18 + 1) {
    free_ent(*(undefined8 *)(sorted_file + local_18 * 8));
  }
  cwd_n_used = 0;
  cwd_some_quoted = 0;
  any_has_acl = 0;
  inode_number_width = 0;
  block_size_width = 0;
  nlink_width = 0;
  owner_width = 0;
  group_width = 0;
  author_width = 0;
  scontext_width = 0;
  major_device_number_width = 0;
  minor_device_number_width = 0;
  file_size_width = 0;
  return;
}

// Function: file_has_aclinfo_cache
int file_has_aclinfo_cache(undefined8 param_1,long param_2,long *param_3,uint param_4) {
  char cVar1;
  int iVar2;
  int *piVar3;
  
  if (((*(char *)(param_2 + 0xb8) == '\0') || (unsupported_scontext_10 == 0)) ||
     (*(long *)(param_2 + 0x18) != unsupported_device_9)) {
    piVar3 = __errno_location();
    *piVar3 = 0;
    iVar2 = file_has_aclinfo(param_1,param_3,param_4);
    piVar3 = __errno_location();
    if (((*(char *)(param_2 + 0xb8) != '\0') && (iVar2 < 1)) &&
       ((cVar1 = acl_errno_valid(*piVar3), cVar1 != '\x01' &&
        (((param_4 & 0x10000) == 0 ||
         (cVar1 = acl_errno_valid(*(undefined4 *)(param_3 + 3)), cVar1 != '\x01')))))) {
      unsupported_scontext_10 = param_3[2];
      unsupported_scontext_err_8 = *(undefined4 *)(param_3 + 3);
      unsupported_device_9 = *(long *)(param_2 + 0x18);
      unsupported_return_7 = iVar2;
    }
  }
  else {
    *param_3 = (long)param_3 + 0x1c;
    param_3[1] = 0;
    param_3[2] = unsupported_scontext_10;
    *(undefined4 *)(param_3 + 3) = unsupported_scontext_err_8;
    piVar3 = __errno_location();
    *piVar3 = 0x5f;
    iVar2 = unsupported_return_7;
  }
  return iVar2;
}

// Function: has_capability_cache
char has_capability_cache(undefined8 param_1,long param_2) {
  char cVar1;
  char cVar2;
  int *piVar3;
  
  if (((*(char *)(param_2 + 0xb8) == '\0') || (unsupported_cached_6 == '\0')) ||
     (*(long *)(param_2 + 0x18) != unsupported_device_5)) {
    cVar1 = has_capability(param_1);
    if ((*(char *)(param_2 + 0xb8) != '\0') && (cVar1 != '\x01')) {
      piVar3 = __errno_location();
      cVar2 = acl_errno_valid(*piVar3);
      if (cVar2 != '\x01') {
        unsupported_cached_6 = '\x01';
        unsupported_device_5 = *(long *)(param_2 + 0x18);
      }
    }
  }
  else {
    piVar3 = __errno_location();
    *piVar3 = 0x5f;
    cVar1 = '\0';
  }
  return cVar1;
}

// Function: needs_quoting
undefined4 needs_quoting(char *param_1) {
  undefined4 uVar1;
  size_t sVar2;
  size_t sVar3;
  long in_FS_OFFSET;
  char local_12 [2];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  sVar2 = quotearg_buffer(local_12,2,param_1,0xffffffffffffffff,filename_quoting_options);
  if (*param_1 == local_12[0]) {
    sVar3 = strlen(param_1);
    if (sVar2 == sVar3) {
      uVar1 = 0;
      goto LAB_00104f5c;
    }
  }
  uVar1 = 1;
LAB_00104f5c:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar1;
}

// Function: gobble_file
undefined8 gobble_file(char *param_1,uint param_2,long param_3,char param_4,char *param_5) {
  undefined8 *puVar1;
  undefined4 uVar2;
  ulonglong uVar3;
  long lVar4;
  char *pcVar5;
  byte bVar6;
  char cVar7;
  undefined uVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  size_t sVar12;
  size_t sVar13;
  ulong uVar14;
  int *piVar15;
  undefined8 uVar16;
  char *pcVar17;
  char **ppcVar18;
  char **ppcVar19;
  long in_FS_OFFSET;
  char *local_438;
  long local_430;
  char local_428;
  uint local_424;
  char *local_420;
  char local_418;
  byte local_417;
  char local_416;
  undefined local_415;
  char local_414;
  char local_413;
  char local_412;
  byte local_411;
  int local_410;
  uint local_40c;
  int local_408;
  int local_404;
  int local_400;
  int local_3fc;
  int local_3f8;
  int local_3f4;
  int local_3f0;
  int local_3ec;
  int local_3e8;
  int local_3e4;
  undefined4 local_3e0;
  int local_3dc;
  undefined4 local_3d8;
  int local_3d4;
  char *local_3d0;
  undefined8 local_3c8;
  undefined8 *local_3c0;
  char *local_3b8;
  undefined8 local_3b0;
  undefined *local_3a8;
  undefined *local_3a0;
  undefined local_398 [16];
  undefined8 local_388;
  int local_380;
  undefined local_2d8 [32];
  undefined local_2b8 [664];
  long local_20;
  
  ppcVar18 = &local_438;
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_3c8 = 0;
  local_438 = param_5;
  local_430 = param_3;
  local_428 = param_4;
  local_424 = param_2;
  local_420 = param_1;
  if ((param_4 == '\x01') && (param_3 != 0)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("! command_line_arg || inode == NOT_AN_INODE_NUMBER",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                  0xd29,"gobble_file");
  }
  if (cwd_n_used == cwd_n_alloc) {
    cwd_file = xpalloc(cwd_file,&cwd_n_alloc,1,0xffffffffffffffff,0xd0);
  }
  local_3c0 = (undefined8 *)(cwd_n_used * 0xd0 + cwd_file);
  memset(local_3c0,0,0xd0);
  local_3c0[4] = local_430;
  *(uint *)(local_3c0 + 0x15) = local_424;
  local_3c0[0x16] = &UNKNOWN_SECURITY_CONTEXT;
  *(undefined4 *)((long)local_3c0 + 0xc4) = 0xffffffff;
  if ((cwd_some_quoted != '\x01') && (align_variable_outer_quotes != '\0')) {
    bVar6 = needs_quoting(local_420);
    *(uint *)((long)local_3c0 + 0xc4) = (uint)bVar6;
    if (*(int *)((long)local_3c0 + 0xc4) != 0) {
      cwd_some_quoted = '\x01';
    }
  }
  if (((((((local_428 == '\0') && (print_hyperlink == '\0')) && (format_needs_stat == '\0')) &&
        ((format_needs_type == '\0' || (local_424 != 0)))) &&
       ((((local_424 != 3 && (local_424 != 0)) || (print_with_color == '\0')) ||
        (((cVar7 = is_colored(0x13), cVar7 == '\0' && (cVar7 = is_colored(0x12), cVar7 == '\0')) &&
         (cVar7 = is_colored(0x14), cVar7 == '\0')))))) &&
      (((((print_inode == '\0' && (format_needs_type == '\0')) ||
         ((local_424 != 6 && (local_424 != 0)))) ||
        (((dereference != 4 && (color_symlink_as_referent == '\0')) && (check_symlink_mode == 0))))
       && ((print_inode == '\0' || (local_430 != 0)))))) &&
     (((local_424 != 5 && (local_424 != 0)) ||
      ((indicator_style != 3 &&
       ((print_with_color == '\0' ||
        (((cVar7 = is_colored(0xe), cVar7 == '\0' && (cVar7 = is_colored(0x10), cVar7 == '\0')) &&
         (cVar7 = is_colored(0x11), cVar7 == '\0')))))))))) {
    local_417 = 0;
  }
  else {
    local_417 = 1;
  }
  local_3d0 = local_420;
  ppcVar19 = &local_438;
  if ((((byte)(format_needs_capability | print_scontext | local_417) != 0) &&
      (ppcVar19 = &local_438, *local_420 != '/')) &&
     (ppcVar19 = &local_438, local_438 != (char *)0x0)) {
    sVar12 = strlen(local_420);
    sVar13 = strlen(local_438);
    uVar14 = ((sVar13 + sVar12 + 0x19) / 0x10) * 0x10;
    for (; ppcVar18 != (char **)((long)&local_438 - (uVar14 & 0xfffffffffffff000));
        ppcVar18 = (char **)((long)ppcVar18 + -0x1000)) {
      *(undefined8 *)((long)ppcVar18 + -8) = *(undefined8 *)((long)ppcVar18 + -8);
    }
    lVar4 = -(ulong)((uint)uVar14 & 0xfff);
    ppcVar19 = (char **)((long)ppcVar18 + lVar4);
    if ((uVar14 & 0xfff) != 0) {
      *(undefined8 *)((long)ppcVar18 + ((ulong)((uint)uVar14 & 0xfff) - 8) + lVar4) =
           *(undefined8 *)((long)ppcVar18 + ((ulong)((uint)uVar14 & 0xfff) - 8) + lVar4);
    }
    pcVar5 = local_420;
    pcVar17 = local_438;
    local_3b8 = (char *)((ulong)((long)ppcVar18 + lVar4 + 0xf) & 0xfffffffffffffff0);
    *(undefined8 *)((long)ppcVar18 + lVar4 + -8) = 0x1053a4;
    attach((char *)((ulong)((long)ppcVar18 + lVar4 + 0xf) & 0xfffffffffffffff0),pcVar17,pcVar5);
    local_3d0 = local_3b8;
  }
  pcVar17 = local_3d0;
  if (local_417 == 1) {
    if (print_hyperlink != '\0') {
      *(undefined8 *)((long)ppcVar19 + -8) = 0x1053f6;
      uVar16 = canonicalize_filename_mode(pcVar17,2);
      local_3c0[2] = uVar16;
      if (local_3c0[2] == 0) {
        *(undefined8 *)((long)ppcVar19 + -8) = 0x105423;
        uVar16 = gettext("error canonicalizing %s");
        pcVar17 = local_3d0;
        cVar7 = local_428;
        *(undefined8 *)((long)ppcVar19 + -8) = 0x10543e;
        file_failure(cVar7,uVar16,pcVar17);
      }
    }
    pcVar17 = local_3d0;
    if (dereference == 4) {
      puVar1 = local_3c0 + 3;
      *(undefined8 *)((long)ppcVar19 + -8) = 0x10548e;
      local_410 = do_stat(pcVar17,puVar1);
      local_418 = '\x01';
    }
    else if (dereference < 5) {
      if (dereference == 1) {
LAB_00105543:
        pcVar17 = local_3d0;
        puVar1 = local_3c0 + 3;
        *(undefined8 *)((long)ppcVar19 + -8) = 0x105560;
        local_410 = do_lstat(pcVar17,puVar1);
        local_418 = '\0';
      }
      else if ((dereference != 0) && (dereference - 2 < 2)) {
        if (local_428 == '\0') goto LAB_00105543;
        puVar1 = local_3c0 + 3;
        *(undefined8 *)((long)ppcVar19 + -8) = 0x1054ca;
        local_410 = do_stat(pcVar17,puVar1);
        local_418 = '\x01';
        if (dereference != 2) {
          if (local_410 < 0) {
            *(undefined8 *)((long)ppcVar19 + -8) = 0x1054f4;
            piVar15 = __errno_location();
            if (*piVar15 != 2) {
              *(undefined8 *)((long)ppcVar19 + -8) = 0x105500;
              piVar15 = __errno_location();
              if (*piVar15 != 0x28) {
                local_416 = false;
                goto LAB_0010552f;
              }
            }
            local_416 = true;
          }
          else {
            local_416 = (*(uint *)(local_3c0 + 6) & 0xf000) != 0x4000;
          }
LAB_0010552f:
          if ((bool)local_416 == true) goto LAB_00105543;
        }
      }
    }
    if (local_410 != 0) {
      *(undefined8 *)((long)ppcVar19 + -8) = 0x10558b;
      uVar16 = gettext("cannot access %s");
      pcVar17 = local_3d0;
      cVar7 = local_428;
      *(undefined8 *)((long)ppcVar19 + -8) = 0x1055a6;
      file_failure(cVar7,uVar16,pcVar17);
      pcVar17 = local_420;
      if (local_428 == '\0') {
        *(undefined8 *)((long)ppcVar19 + -8) = 0x1055c8;
        uVar16 = xstrdup(pcVar17);
        *local_3c0 = uVar16;
        cwd_n_used = cwd_n_used + 1;
        uVar16 = 0;
      }
      else {
        uVar16 = 0;
      }
      goto LAB_00105e83;
    }
    *(undefined *)(local_3c0 + 0x17) = 1;
    local_424 = (uint)(char)d_type_filetype[*(uint *)(local_3c0 + 6) >> 0xc & 0xf];
    *(uint *)(local_3c0 + 0x15) = local_424;
  }
  else {
    local_418 = dereference == 4;
  }
  puVar1 = local_3c0;
  pcVar17 = local_3d0;
  if (((local_424 == 3) && (local_428 != '\0')) && (immediate_dirs != '\x01')) {
    local_424 = 9;
    *(undefined4 *)(local_3c0 + 0x15) = 9;
  }
  local_415 = (print_scontext | format == 0) != 0;
  local_414 = (local_424 == 5 & format_needs_capability) != 0;
  if ((bool)local_415 || (bool)local_414) {
    if (local_418 == '\0') {
      uVar10 = 0;
    }
    else {
      uVar10 = 0x20000;
    }
    if ((bool)local_415) {
      uVar9 = 0x10000;
    }
    else {
      uVar9 = 0;
    }
    uVar10 = (uint)(byte)filetype_d_type[local_424] | uVar10 | uVar9;
    local_40c = uVar10;
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105740;
    local_408 = file_has_aclinfo_cache(pcVar17,puVar1,local_398,uVar10);
    local_413 = 0 < local_408;
    local_412 = local_380 == 0;
    if (local_408 < 0) {
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105775;
      piVar15 = __errno_location();
      if (*piVar15 != 0xd) {
        *(undefined8 *)((long)ppcVar19 + -8) = 0x105781;
        piVar15 = __errno_location();
        if (*piVar15 != 2) goto LAB_0010578f;
      }
      local_411 = 1;
    }
    else {
LAB_0010578f:
      local_411 = 0;
    }
    iVar11 = local_380;
    pcVar17 = local_3d0;
    if ((local_412 == '\x01') || (local_413 == '\x01')) {
      if ((local_412 == '\0') || (local_413 == '\x01')) {
        uVar10 = 3;
      }
      else {
        uVar10 = 2;
      }
    }
    else {
      uVar10 = (uint)local_411;
    }
    *(uint *)((long)local_3c0 + 0xbc) = uVar10;
    any_has_acl = (*(int *)((long)local_3c0 + 0xbc) != 0 | any_has_acl) != 0;
    if (((format == 0) && (local_408 < 0)) && (local_411 != 1)) {
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105863;
      uVar16 = quotearg_n_style_colon(0,3,pcVar17);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x10586b;
      piVar15 = __errno_location();
      iVar11 = *piVar15;
      local_3e0 = 0;
      local_3a8 = &DAT_0010de40;
      local_3dc = iVar11;
      *(undefined8 *)((long)ppcVar19 + -8) = 0x1058ad;
      error(0,iVar11,&DAT_0010de40,uVar16);
    }
    else if ((print_scontext != 0) && (local_380 != 0)) {
      *(undefined8 *)((long)ppcVar19 + -8) = 0x1058d9;
      cVar7 = is_ENOTSUP(iVar11);
      pcVar17 = local_3d0;
      if ((cVar7 != '\x01') && (local_380 != 0x3d)) {
        *(undefined8 *)((long)ppcVar19 + -8) = 0x105904;
        uVar16 = quotearg_n_style_colon(0,3,pcVar17);
        iVar11 = local_380;
        local_3d8 = 0;
        local_3d4 = local_380;
        local_3a0 = &DAT_0010de40;
        *(undefined8 *)((long)ppcVar19 + -8) = 0x10594a;
        error(0,iVar11,&DAT_0010de40,uVar16);
      }
    }
    if (local_414 != '\0') {
      *(undefined8 *)((long)ppcVar19 + -8) = 0x10596d;
      cVar7 = aclinfo_has_xattr(local_398,"security.capability");
      puVar1 = local_3c0;
      pcVar17 = local_3d0;
      if (cVar7 != '\0') {
        *(undefined8 *)((long)ppcVar19 + -8) = 0x10598a;
        uVar8 = has_capability_cache(pcVar17,puVar1);
        *(undefined *)(local_3c0 + 0x18) = uVar8;
      }
    }
    local_3c0[0x16] = local_388;
    local_388 = 0;
    *(undefined8 *)((long)ppcVar19 + -8) = 0x1059c6;
    aclinfo_free(local_398);
  }
  puVar1 = local_3c0;
  pcVar17 = local_3d0;
  cVar7 = local_428;
  if (((check_symlink_mode | format == 0) & local_424 == 6) != 0) {
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105a17;
    get_link_name(pcVar17,puVar1,cVar7);
    if ((local_3c0[1] != 0) && (*(int *)((long)local_3c0 + 0xc4) == 0)) {
      uVar16 = local_3c0[1];
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105a4b;
      cVar7 = needs_quoting(uVar16);
      if (cVar7 != '\0') {
        *(undefined4 *)((long)local_3c0 + 0xc4) = 0xffffffff;
      }
    }
    pcVar17 = local_3d0;
    if ((local_3c0[1] != 0) && ((1 < indicator_style || (check_symlink_mode != 0)))) {
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105a9f;
      iVar11 = stat_for_mode(pcVar17,local_398);
      if (iVar11 == 0) {
        *(undefined *)((long)local_3c0 + 0xb9) = 1;
        *(int *)((long)local_3c0 + 0xac) = local_380;
      }
    }
  }
  uVar16 = local_3c0[0xb];
  local_3c8 = uVar16;
  if ((format == 0) || (print_block_size != '\0')) {
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105b16;
    uVar16 = human_readable(uVar16,local_2b8,human_output_opts,0x200,output_block_size);
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105b23;
    local_404 = gnu_mbswidth(uVar16,3);
    if (block_size_width < local_404) {
      block_size_width = local_404;
    }
  }
  if (format == 0) {
    if (print_owner != '\0') {
      uVar2 = *(undefined4 *)((long)local_3c0 + 0x34);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105b6d;
      local_400 = format_user_width(uVar2);
      if (owner_width < local_400) {
        owner_width = local_400;
      }
    }
    if (print_group != '\0') {
      uVar2 = *(undefined4 *)(local_3c0 + 7);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105ba9;
      local_3fc = format_group_width(uVar2);
      if (group_width < local_3fc) {
        group_width = local_3fc;
      }
    }
    if (print_author != '\0') {
      uVar2 = *(undefined4 *)((long)local_3c0 + 0x34);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105be5;
      local_3f8 = format_user_width(uVar2);
      if (author_width < local_3f8) {
        author_width = local_3f8;
      }
    }
  }
  if (print_scontext != 0) {
    pcVar17 = (char *)local_3c0[0x16];
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105c26;
    sVar12 = strlen(pcVar17);
    local_3f4 = (int)sVar12;
    if (scontext_width < local_3f4) {
      scontext_width = local_3f4;
    }
  }
  if (format == 0) {
    uVar16 = local_3c0[5];
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105c71;
    pcVar17 = (char *)umaxtostr(uVar16,local_2d8);
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105c79;
    sVar12 = strlen(pcVar17);
    local_3f0 = (int)sVar12;
    if (nlink_width < local_3f0) {
      nlink_width = local_3f0;
    }
    if (local_424 == 4 || local_424 == 2) {
      uVar3 = local_3c0[8];
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105cca;
      uVar10 = gnu_dev_major(uVar3);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105cde;
      pcVar17 = (char *)umaxtostr(uVar10,local_2b8);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105ce6;
      sVar12 = strlen(pcVar17);
      local_3e8 = (int)sVar12;
      if (major_device_number_width < local_3e8) {
        major_device_number_width = local_3e8;
      }
      uVar3 = local_3c0[8];
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105d19;
      uVar10 = gnu_dev_minor(uVar3);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105d2d;
      pcVar17 = (char *)umaxtostr(uVar10,local_2b8);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105d35;
      sVar12 = strlen(pcVar17);
      if (minor_device_number_width < (int)sVar12) {
        minor_device_number_width = (int)sVar12;
      }
      local_3e8 = minor_device_number_width + major_device_number_width + 2;
      if (file_size_width < local_3e8) {
        file_size_width = local_3e8;
      }
    }
    else {
      uVar16 = local_3c0[9];
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105d9f;
      uVar16 = unsigned_file_size(uVar16);
      local_3b0 = uVar16;
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105dd1;
      uVar16 = human_readable(uVar16,local_2b8,file_human_output_opts,1,file_output_block_size);
      *(undefined8 *)((long)ppcVar19 + -8) = 0x105dde;
      local_3ec = gnu_mbswidth(uVar16,3);
      if (file_size_width < local_3ec) {
        file_size_width = local_3ec;
      }
    }
  }
  if (print_inode != '\0') {
    uVar16 = local_3c0[4];
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105e26;
    pcVar17 = (char *)umaxtostr(uVar16,local_2b8);
    *(undefined8 *)((long)ppcVar19 + -8) = 0x105e2e;
    sVar12 = strlen(pcVar17);
    local_3e4 = (int)sVar12;
    if (inode_number_width < local_3e4) {
      inode_number_width = local_3e4;
    }
  }
  pcVar17 = local_420;
  *(undefined8 *)((long)ppcVar19 + -8) = 0x105e5d;
  uVar16 = xstrdup(pcVar17);
  *local_3c0 = uVar16;
  cwd_n_used = cwd_n_used + 1;
  uVar16 = local_3c8;
LAB_00105e83:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar16;
  }
                    /* WARNING: Subroutine does not return */
  *(undefined8 *)((long)ppcVar19 + -8) = 0x105e97;
  __stack_chk_fail();
}

// Function: is_directory
undefined4 is_directory(long param_1) {
  undefined4 uVar1;
  
  if ((*(int *)(param_1 + 0xa8) == 3) || (*(int *)(param_1 + 0xa8) == 9)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: is_linked_directory
undefined4 is_linked_directory(long param_1) {
  undefined4 uVar1;
  
  if (((*(int *)(param_1 + 0xa8) == 3) || (*(int *)(param_1 + 0xa8) == 9)) ||
     ((*(uint *)(param_1 + 0xac) & 0xf000) == 0x4000)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: get_link_name
void get_link_name(undefined8 param_1,long param_2,undefined param_3) {
  undefined8 uVar1;
  
  uVar1 = areadlink_with_size(param_1,*(undefined8 *)(param_2 + 0x48));
  *(undefined8 *)(param_2 + 8) = uVar1;
  if (*(long *)(param_2 + 8) == 0) {
    uVar1 = gettext("cannot read symbolic link %s");
    file_failure(param_3,uVar1,param_1);
  }
  return;
}

// Function: basename_is_dot_or_dotdot
void basename_is_dot_or_dotdot(undefined8 param_1) {
  undefined8 uVar1;
  
  uVar1 = last_component(param_1);
  dot_or_dotdot(uVar1);
  return;
}

// Function: extract_dirs_from_files
void extract_dirs_from_files(long param_1,undefined param_2) {
  char **ppcVar1;
  long lVar2;
  char cVar3;
  void *__ptr;
  long local_30;
  long local_28;
  
  if ((param_1 != 0) && (active_dir_set != 0)) {
    queue_directory(0,param_1,0);
  }
  local_30 = cwd_n_used;
LAB_0010610c:
  do {
    if (local_30 < 1) {
      local_28 = 0;
      for (local_30 = 0; local_30 < cwd_n_used; local_30 = local_30 + 1) {
        lVar2 = *(long *)(sorted_file + local_30 * 8);
        *(long *)(local_28 * 8 + sorted_file) = lVar2;
        local_28 = local_28 + (ulong)(*(int *)(lVar2 + 0xa8) != 9);
      }
      cwd_n_used = local_28;
      return;
    }
    local_30 = local_30 + -1;
    ppcVar1 = *(char ***)(sorted_file + local_30 * 8);
    cVar3 = is_directory(ppcVar1);
  } while (cVar3 == '\0');
  if (param_1 != 0) goto code_r0x0010605c;
  goto LAB_00106076;
code_r0x0010605c:
  cVar3 = basename_is_dot_or_dotdot(*ppcVar1);
  if (cVar3 != '\x01') {
LAB_00106076:
    if ((param_1 == 0) || (**ppcVar1 == '/')) {
      queue_directory(*ppcVar1,ppcVar1[1],param_2);
    }
    else {
      __ptr = (void *)file_name_concat(param_1,*ppcVar1,0);
      queue_directory(__ptr,ppcVar1[1],param_2);
      free(__ptr);
    }
    if (*(int *)(ppcVar1 + 0x15) == 9) {
      free_ent(ppcVar1);
    }
  }
  goto LAB_0010610c;
}

// Function: xstrcoll
int xstrcoll(char *param_1,char *param_2) {
  int iVar1;
  int *piVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  
  piVar2 = __errno_location();
  *piVar2 = 0;
  iVar1 = strcoll(param_1,param_2);
  piVar2 = __errno_location();
  if (*piVar2 != 0) {
    uVar3 = quote_n(1,param_2);
    uVar4 = quote_n(0,param_1);
    uVar5 = gettext("cannot compare file names %s and %s");
    piVar2 = __errno_location();
    error(0,*piVar2,uVar5,uVar4,uVar3);
    set_exit_status(0);
                    /* WARNING: Subroutine does not return */
    longjmp((__jmp_buf_tag *)failed_strcoll,1);
  }
  return iVar1;
}

// Function: dirfirst_check
ulong dirfirst_check(undefined8 param_1,undefined8 param_2,code *param_3) {
  byte bVar1;
  byte bVar2;
  ulong uVar3;
  
  bVar1 = is_linked_directory(param_2);
  bVar2 = is_linked_directory(param_1);
  if ((uint)bVar1 - (uint)bVar2 == 0) {
    uVar3 = (*param_3)(param_1,param_2);
  }
  else {
    uVar3 = (ulong)((uint)bVar1 - (uint)bVar2);
  }
  return uVar3;
}

// Function: cmp_ctime
ulong cmp_ctime(undefined8 *param_1,undefined8 *param_2,code *param_3) {
  uint uVar1;
  ulong uVar2;
  undefined auVar3 [16];
  undefined auVar4 [16];
  
  auVar3 = get_stat_ctime(param_1 + 3);
  auVar4 = get_stat_ctime(param_2 + 3);
  uVar1 = timespec_cmp(auVar4._0_8_,auVar4._8_8_,auVar3._0_8_,auVar3._8_8_);
  if (uVar1 == 0) {
    uVar2 = (*param_3)(*param_1,*param_2);
  }
  else {
    uVar2 = (ulong)uVar1;
  }
  return uVar2;
}

// Function: cmp_mtime
ulong cmp_mtime(undefined8 *param_1,undefined8 *param_2,code *param_3) {
  uint uVar1;
  ulong uVar2;
  undefined auVar3 [16];
  undefined auVar4 [16];
  
  auVar3 = get_stat_mtime(param_1 + 3);
  auVar4 = get_stat_mtime(param_2 + 3);
  uVar1 = timespec_cmp(auVar4._0_8_,auVar4._8_8_,auVar3._0_8_,auVar3._8_8_);
  if (uVar1 == 0) {
    uVar2 = (*param_3)(*param_1,*param_2);
  }
  else {
    uVar2 = (ulong)uVar1;
  }
  return uVar2;
}

// Function: cmp_atime
ulong cmp_atime(undefined8 *param_1,undefined8 *param_2,code *param_3) {
  uint uVar1;
  ulong uVar2;
  undefined auVar3 [16];
  undefined auVar4 [16];
  
  auVar3 = get_stat_atime(param_1 + 3);
  auVar4 = get_stat_atime(param_2 + 3);
  uVar1 = timespec_cmp(auVar4._0_8_,auVar4._8_8_,auVar3._0_8_,auVar3._8_8_);
  if (uVar1 == 0) {
    uVar2 = (*param_3)(*param_1,*param_2);
  }
  else {
    uVar2 = (ulong)uVar1;
  }
  return uVar2;
}

// Function: cmp_btime
ulong cmp_btime(undefined8 *param_1,undefined8 *param_2,code *param_3) {
  uint uVar1;
  ulong uVar2;
  undefined auVar3 [16];
  undefined auVar4 [16];
  
  auVar3 = get_stat_btime(param_1 + 3);
  auVar4 = get_stat_btime(param_2 + 3);
  uVar1 = timespec_cmp(auVar4._0_8_,auVar4._8_8_,auVar3._0_8_,auVar3._8_8_);
  if (uVar1 == 0) {
    uVar2 = (*param_3)(*param_1,*param_2);
  }
  else {
    uVar2 = (ulong)uVar1;
  }
  return uVar2;
}

// Function: off_cmp
int off_cmp(long param_1,long param_2) {
  return (uint)(param_2 < param_1) - (uint)(param_1 < param_2);
}

// Function: cmp_size
ulong cmp_size(undefined8 *param_1,undefined8 *param_2,code *param_3) {
  uint uVar1;
  ulong uVar2;
  
  uVar1 = off_cmp(param_2[9],param_1[9]);
  if (uVar1 == 0) {
    uVar2 = (*param_3)(*param_1,*param_2);
  }
  else {
    uVar2 = (ulong)uVar1;
  }
  return uVar2;
}

// Function: cmp_name
void cmp_name(undefined8 *param_1,undefined8 *param_2,code *param_3) {
  (*param_3)(*param_1,*param_2);
  return;
}

// Function: cmp_extension
ulong cmp_extension(char **param_1,char **param_2,code *param_3) {
  uint uVar1;
  char *pcVar2;
  char *pcVar3;
  ulong uVar4;
  
  pcVar2 = strrchr(*param_1,0x2e);
  pcVar3 = strrchr(*param_2,0x2e);
  if (pcVar3 == (char *)0x0) {
    pcVar3 = "";
  }
  if (pcVar2 == (char *)0x0) {
    pcVar2 = "";
  }
  uVar1 = (*param_3)(pcVar2,pcVar3);
  if (uVar1 == 0) {
    uVar4 = (*param_3)(*param_1,*param_2);
  }
  else {
    uVar4 = (ulong)uVar1;
  }
  return uVar4;
}

// Function: fileinfo_name_width
undefined8 fileinfo_name_width(undefined8 *param_1) {
  undefined8 uVar1;
  
  if (param_1[0x19] == 0) {
    uVar1 = quote_name_width(*param_1,filename_quoting_options,*(undefined4 *)((long)param_1 + 0xc4)
                            );
  }
  else {
    uVar1 = param_1[0x19];
  }
  return uVar1;
}

// Function: cmp_width
ulong cmp_width(undefined8 *param_1,undefined8 *param_2,code *param_3) {
  int iVar1;
  int iVar2;
  ulong uVar3;
  
  iVar1 = fileinfo_name_width(param_1);
  iVar2 = fileinfo_name_width(param_2);
  if (iVar1 - iVar2 == 0) {
    uVar3 = (*param_3)(*param_1,*param_2);
  }
  else {
    uVar3 = (ulong)(uint)(iVar1 - iVar2);
  }
  return uVar3;
}

// Function: cmp_version
int cmp_version(char **param_1,char **param_2) {
  int iVar1;
  
  iVar1 = filevercmp(*param_1,*param_2);
  if (iVar1 == 0) {
    iVar1 = strcmp(*param_1,*param_2);
  }
  return iVar1;
}

// Function: xstrcoll_version
void xstrcoll_version(undefined8 param_1,undefined8 param_2) {
  cmp_version(param_1,param_2);
  return;
}

// Function: rev_xstrcoll_version
void rev_xstrcoll_version(undefined8 param_1,undefined8 param_2) {
  cmp_version(param_2,param_1);
  return;
}

// Function: xstrcoll_df_version
void xstrcoll_df_version(undefined8 param_1,undefined8 param_2) {
  dirfirst_check(param_1,param_2,xstrcoll_version);
  return;
}

// Function: rev_xstrcoll_df_version
void rev_xstrcoll_df_version(undefined8 param_1,undefined8 param_2) {
  dirfirst_check(param_1,param_2,rev_xstrcoll_version);
  return;
}

// Function: initialize_ordering_vector
void initialize_ordering_vector(void) {
  long local_10;
  
  for (local_10 = 0; local_10 < cwd_n_used; local_10 = local_10 + 1) {
    *(long *)(sorted_file + local_10 * 8) = cwd_file + local_10 * 0xd0;
  }
  return;
}

// Function: update_current_files_info
void update_current_files_info(void) {
  long lVar1;
  undefined8 uVar2;
  long local_18;
  
  if ((sort_type == 2) || ((line_length != 0 && ((format == 2 || (format == 3)))))) {
    for (local_18 = 0; local_18 < cwd_n_used; local_18 = local_18 + 1) {
      lVar1 = *(long *)(sorted_file + local_18 * 8);
      uVar2 = fileinfo_name_width(lVar1);
      *(undefined8 *)(lVar1 + 200) = uVar2;
    }
  }
  return;
}

// Function: sort_files
void sort_files(void) {
  int iVar1;
  byte local_9;
  
  if (sorted_file_alloc < (ulong)(cwd_n_used + (cwd_n_used >> 1))) {
    free(sorted_file);
    sorted_file = (void *)xinmalloc(cwd_n_used,0x18);
    sorted_file_alloc = cwd_n_used * 3;
  }
  initialize_ordering_vector();
  update_current_files_info();
  if (sort_type != 6) {
    iVar1 = _setjmp((__jmp_buf_tag *)failed_strcoll);
    if (iVar1 == 0) {
      local_9 = 0;
    }
    else {
      local_9 = 1;
      if (sort_type == 4) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("sort_type != sort_version",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/ls.c",
                      0x1001,"sort_files");
      }
      initialize_ordering_vector();
    }
    iVar1 = time_type;
    if (sort_type != 5) {
      iVar1 = 0;
    }
    mpsort(sorted_file,cwd_n_used,
           *(undefined8 *)
            (sort_functions +
            ((long)(int)(uint)directories_first +
            ((long)(int)(uint)sort_reverse +
            ((long)(int)(uint)local_9 + (ulong)(uint)(iVar1 + sort_type) * 2) * 2) * 2) * 8));
  }
  return;
}

// Function: print_current_files
void print_current_files(void) {
  long local_18;
  long local_10;
  
  switch(format) {
  case 0:
    for (local_10 = 0; local_10 < cwd_n_used; local_10 = local_10 + 1) {
      set_normal_color();
      print_long_format(*(undefined8 *)(sorted_file + local_10 * 8));
      dired_outbyte((int)eolbyte);
    }
    break;
  case 1:
    for (local_18 = 0; local_18 < cwd_n_used; local_18 = local_18 + 1) {
      print_file_name_and_frills(*(undefined8 *)(sorted_file + local_18 * 8),0);
      putchar_unlocked((int)eolbyte);
    }
    break;
  case 2:
    if (line_length == 0) {
      print_with_separator(0x20);
    }
    else {
      print_many_per_line();
    }
    break;
  case 3:
    if (line_length == 0) {
      print_with_separator(0x20);
    }
    else {
      print_horizontal();
    }
    break;
  case 4:
    print_with_separator(0x2c);
  }
  return;
}

// Function: align_nstrftime
void align_nstrftime(undefined8 param_1,undefined8 param_2,byte param_3,long param_4,
                    undefined8 param_5,undefined4 param_6) {
  undefined1 *puVar1;
  
  if (use_abformat == '\0') {
    puVar1 = *(undefined1 **)(long_time_format + (long)(int)(uint)param_3 * 8);
  }
  else {
    puVar1 = abformat + ((long)(int)(uint)param_3 * 0xc + (long)*(int *)(param_4 + 0x10)) * 0x80;
  }
  nstrftime(param_1,param_2,puVar1,param_4,param_5,param_6);
  return;
}

// Function: long_time_expected_width
int long_time_expected_width(void) {
  long lVar1;
  long in_FS_OFFSET;
  undefined8 local_458;
  long local_450;
  undefined local_448 [64];
  undefined local_408 [1016];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (width_2 < 0) {
    local_458 = 0;
    lVar1 = localtime_rz(localtz,&local_458,local_448);
    if (lVar1 != 0) {
      local_450 = align_nstrftime(local_408,0x3e9,0,local_448,localtz,0);
      if (0 < local_450) {
        width_2 = mbsnwidth(local_408,local_450,3);
      }
    }
    if (width_2 < 0) {
      width_2 = 0;
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return width_2;
}

// Function: format_user_or_group
void format_user_or_group(long param_1,undefined8 param_2,uint param_3) {
  int iVar1;
  bool bVar2;
  int local_14;
  
  if (param_1 == 0) {
    iVar1 = printf("%*ju ",(ulong)param_3,param_2);
    dired_pos = dired_pos + iVar1;
  }
  else {
    local_14 = gnu_mbswidth(param_1,3);
    if (local_14 < 0) {
      local_14 = 0;
    }
    else {
      local_14 = param_3 - local_14;
    }
    if (local_14 < 0) {
      local_14 = 0;
    }
    dired_outstring(param_1);
    do {
      dired_outbyte(0x20);
      bVar2 = local_14 != 0;
      local_14 = local_14 + -1;
    } while (bVar2);
  }
  return;
}

// Function: format_user
void format_user(undefined4 param_1,undefined4 param_2,char param_3) {
  undefined1 *puVar1;
  
  if (param_3 == '\x01') {
    if (numeric_ids == '\0') {
      puVar1 = (undefined1 *)getuser(param_1);
    }
    else {
      puVar1 = (undefined1 *)0x0;
    }
  }
  else {
    puVar1 = &DAT_0010e922;
  }
  format_user_or_group(puVar1,param_1,param_2);
  return;
}

// Function: format_group
void format_group(undefined4 param_1,undefined4 param_2,char param_3) {
  undefined1 *puVar1;
  
  if (param_3 == '\x01') {
    if (numeric_ids == '\0') {
      puVar1 = (undefined1 *)getgroup(param_1);
    }
    else {
      puVar1 = (undefined1 *)0x0;
    }
  }
  else {
    puVar1 = &DAT_0010e922;
  }
  format_user_or_group(puVar1,param_1,param_2);
  return;
}

// Function: format_user_or_group_width
void format_user_or_group_width(long param_1,undefined8 param_2) {
  if (param_1 == 0) {
    snprintf((char *)0x0,0,"%ju",param_2);
  }
  else {
    gnu_mbswidth(param_1,3);
  }
  return;
}

// Function: format_user_width
void format_user_width(undefined4 param_1) {
  undefined8 uVar1;
  
  if (numeric_ids == '\0') {
    uVar1 = getuser(param_1);
  }
  else {
    uVar1 = 0;
  }
  format_user_or_group_width(uVar1,param_1);
  return;
}

// Function: format_group_width
void format_group_width(undefined4 param_1) {
  undefined8 uVar1;
  
  if (numeric_ids == '\0') {
    uVar1 = getgroup(param_1);
  }
  else {
    uVar1 = 0;
  }
  format_user_or_group_width(uVar1,param_1);
  return;
}

// Function: format_inode
undefined1 * format_inode(undefined8 param_1,long param_2) {
  undefined1 *puVar1;
  
  if ((*(char *)(param_2 + 0xb8) == '\0') || (*(long *)(param_2 + 0x20) == 0)) {
    puVar1 = &DAT_0010e922;
  }
  else {
    puVar1 = (undefined1 *)umaxtostr(*(undefined8 *)(param_2 + 0x20),param_1);
  }
  return puVar1;
}

// Function: print_long_format
void print_long_format(long param_1) {
  char *pcVar1;
  bool bVar2;
  undefined auVar3 [16];
  undefined auVar4 [16];
  undefined4 uVar5;
  undefined uVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  undefined8 uVar12;
  undefined1 *puVar13;
  undefined8 uVar14;
  long lVar15;
  long in_FS_OFFSET;
  int local_11a4;
  int local_11a0;
  long local_1190;
  char *local_1188;
  char *local_1180;
  char *local_1178;
  undefined local_1168 [16];
  long local_1158;
  undefined8 local_1150;
  undefined local_1148 [68];
  undefined local_1104;
  undefined auStack_1103 [9];
  undefined local_10fa;
  undefined local_10f9;
  undefined local_10f8 [656];
  char local_e68 [3656];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  bVar2 = true;
  if (*(char *)(param_1 + 0xb8) == '\0') {
    local_1104 = filetype_letter[*(uint *)(param_1 + 0xa8)];
    memset(auStack_1103,0x3f,10);
    local_10f9 = 0;
  }
  else {
    filemodestring(param_1 + 0x18,&local_1104);
  }
  auVar4._8_8_ = local_1168._8_8_;
  auVar4._0_8_ = local_1168._0_8_;
  auVar3._8_8_ = local_1168._8_8_;
  auVar3._0_8_ = local_1168._0_8_;
  if (any_has_acl == '\x01') {
    if (*(int *)(param_1 + 0xbc) == 2) {
      local_10fa = 0x2e;
    }
    else if (*(int *)(param_1 + 0xbc) == 3) {
      local_10fa = 0x2b;
    }
    else if (*(int *)(param_1 + 0xbc) == 1) {
      local_10fa = 0x3f;
    }
  }
  else {
    local_10fa = 0;
  }
  if (time_type == 3) {
    local_1168 = get_stat_btime(param_1 + 0x18);
    if ((local_1168._0_8_ == -1) && (local_1168._8_8_ == -1)) {
      bVar2 = false;
    }
  }
  else if (time_type < 4) {
    if (time_type == 2) {
      local_1168 = get_stat_atime(param_1 + 0x18);
    }
    else {
      local_1168 = auVar3;
      if (time_type < 3) {
        if (time_type == 0) {
          local_1168 = get_stat_mtime(param_1 + 0x18);
        }
        else {
          local_1168 = auVar4;
          if (time_type == 1) {
            local_1168 = get_stat_ctime(param_1 + 0x18);
          }
        }
      }
    }
  }
  local_1188 = local_e68;
  if (print_inode != '\0') {
    uVar12 = format_inode(local_10f8,param_1);
    iVar7 = sprintf(local_1188,"%*s ",(ulong)inode_number_width,uVar12);
    local_1188 = local_1188 + iVar7;
  }
  if (print_block_size != '\0') {
    if (*(char *)(param_1 + 0xb8) == '\x01') {
      local_1180 = (char *)human_readable(*(undefined8 *)(param_1 + 0x58),local_10f8,
                                          human_output_opts,0x200,output_block_size);
    }
    else {
      local_1180 = "?";
    }
    local_11a4 = gnu_mbswidth(local_1180,3);
    if (local_11a4 < 0) {
      local_11a4 = 0;
    }
    else {
      local_11a4 = block_size_width - local_11a4;
    }
    for (; pcVar1 = local_1188, 0 < local_11a4; local_11a4 = local_11a4 + -1) {
      *local_1188 = ' ';
      local_1188 = local_1188 + 1;
    }
    do {
      local_1188 = pcVar1;
      *local_1188 = *local_1180;
      pcVar1 = local_1188 + 1;
      local_1180 = local_1180 + 1;
    } while (*local_1188 != '\0');
    *local_1188 = ' ';
    local_1188 = local_1188 + 1;
  }
  if (*(char *)(param_1 + 0xb8) == '\x01') {
    puVar13 = (undefined1 *)umaxtostr(*(undefined8 *)(param_1 + 0x28),local_10f8);
  }
  else {
    puVar13 = &DAT_0010e922;
  }
  iVar7 = sprintf(local_1188,"%s %*s ",&local_1104,(ulong)nlink_width,puVar13);
  local_1188 = local_1188 + iVar7;
  dired_indent();
  if ((((print_owner != '\0') || (print_group != '\0')) || (print_author != '\0')) ||
     (print_scontext != '\0')) {
    dired_outbuf(local_e68,(long)local_1188 - (long)local_e68);
    if (print_owner != '\0') {
      format_user(*(undefined4 *)(param_1 + 0x34),owner_width,*(undefined *)(param_1 + 0xb8));
    }
    if (print_group != '\0') {
      format_group(*(undefined4 *)(param_1 + 0x38),group_width,*(undefined *)(param_1 + 0xb8));
    }
    if (print_author != '\0') {
      format_user(*(undefined4 *)(param_1 + 0x34),author_width,*(undefined *)(param_1 + 0xb8));
    }
    if (print_scontext != '\0') {
      format_user_or_group(*(undefined8 *)(param_1 + 0xb0),0,scontext_width);
    }
    local_1188 = local_e68;
  }
  uVar5 = file_human_output_opts;
  uVar12 = file_output_block_size;
  if ((*(char *)(param_1 + 0xb8) == '\0') ||
     (((*(uint *)(param_1 + 0x30) & 0xf000) != 0x2000 &&
      ((*(uint *)(param_1 + 0x30) & 0xf000) != 0x6000)))) {
    if (*(char *)(param_1 + 0xb8) == '\x01') {
      uVar14 = unsigned_file_size(*(undefined8 *)(param_1 + 0x48));
      local_1178 = (char *)human_readable(uVar14,local_10f8,uVar5,1,uVar12);
    }
    else {
      local_1178 = "?";
    }
    local_11a0 = gnu_mbswidth(local_1178,3);
    if (local_11a0 < 0) {
      local_11a0 = 0;
    }
    else {
      local_11a0 = file_size_width - local_11a0;
    }
    for (; pcVar1 = local_1188, 0 < local_11a0; local_11a0 = local_11a0 + -1) {
      *local_1188 = ' ';
      local_1188 = local_1188 + 1;
    }
    do {
      local_1188 = pcVar1;
      *local_1188 = *local_1178;
      pcVar1 = local_1188 + 1;
      local_1178 = local_1178 + 1;
    } while (*local_1188 != '\0');
    *local_1188 = ' ';
    local_1188 = local_1188 + 1;
  }
  else {
    iVar8 = file_size_width - (minor_device_number_width + major_device_number_width + 2);
    uVar9 = gnu_dev_minor(*(ulonglong *)(param_1 + 0x40));
    uVar11 = minor_device_number_width;
    uVar10 = gnu_dev_major(*(ulonglong *)(param_1 + 0x40));
    iVar7 = 0;
    if (-1 < iVar8) {
      iVar7 = iVar8;
    }
    iVar7 = sprintf(local_1188,"%*ju, %*ju ",(ulong)(uint)(iVar7 + major_device_number_width),
                    (ulong)uVar10,(ulong)uVar11,(ulong)uVar9);
    local_1188 = local_1188 + iVar7;
  }
  local_1190 = 0;
  *local_1188 = '\x01';
  if ((*(char *)(param_1 + 0xb8) == '\0') || (!bVar2)) goto LAB_00108471;
  lVar15 = localtime_rz(localtz,local_1168,local_1148);
  if (lVar15 == 0) goto LAB_00108471;
  iVar7 = timespec_cmp(current_time,DAT_0010ae18,local_1168._0_8_,local_1168._8_8_);
  if (iVar7 < 0) {
    gettime(&current_time);
  }
  local_1158 = current_time + -0xf0c2ac;
  local_1150 = DAT_0010ae18;
  iVar7 = timespec_cmp(local_1158,DAT_0010ae18,local_1168._0_8_,local_1168._8_8_);
  if (iVar7 < 0) {
    iVar7 = timespec_cmp(local_1168._0_8_,local_1168._8_8_,current_time,DAT_0010ae18);
    if (-1 < iVar7) goto LAB_0010840b;
    uVar6 = 1;
  }
  else {
LAB_0010840b:
    uVar6 = 0;
  }
  local_1190 = align_nstrftime(local_1188,0x3e9,uVar6,local_1148,localtz,
                               local_1168._8_8_ & 0xffffffff);
  if (local_1190 < 0) {
    local_1190 = 0;
  }
LAB_00108471:
  if ((local_1190 == 0) && (*local_1188 != '\0')) {
    if ((*(char *)(param_1 + 0xb8) == '\0') || (!bVar2)) {
      puVar13 = &DAT_0010e922;
    }
    else {
      puVar13 = (undefined1 *)timetostr(local_1168._0_8_,local_10f8);
    }
    uVar11 = long_time_expected_width();
    iVar7 = sprintf(local_1188,"%*s ",(ulong)uVar11,puVar13);
    local_1188 = local_1188 + iVar7;
  }
  else {
    pcVar1 = local_1188 + local_1190;
    local_1188 = pcVar1 + 1;
    *pcVar1 = ' ';
  }
  dired_outbuf(local_e68,(long)local_1188 - (long)local_e68);
  lVar15 = print_name_with_quoting(param_1,0,dired_obstack,(long)local_1188 - (long)local_e68);
  if (*(int *)(param_1 + 0xa8) == 6) {
    if (*(long *)(param_1 + 8) != 0) {
      dired_outstring(&DAT_0010e941);
      print_name_with_quoting(param_1,1,0,local_1188 + (lVar15 - (long)local_e68) + 4);
      if (indicator_style != 0) {
        print_type_indicator(1,*(undefined4 *)(param_1 + 0xac),0);
      }
    }
  }
  else if (indicator_style != 0) {
    print_type_indicator
              (*(undefined *)(param_1 + 0xb8),*(undefined4 *)(param_1 + 0x30),
               *(undefined4 *)(param_1 + 0xa8));
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: quote_name_buf
ulong quote_name_buf(char **param_1,ulong param_2,char *param_3,undefined8 param_4,int param_5,
                    ulong *param_6,undefined *param_7) {
  ushort *puVar1;
  bool bVar2;
  bool bVar3;
  byte bVar4;
  undefined uVar5;
  int iVar6;
  size_t sVar7;
  ushort **ppuVar8;
  long in_FS_OFFSET;
  undefined4 local_8c;
  int local_88;
  int local_84;
  char *local_80;
  ulong local_78;
  ulong local_70;
  char *local_68;
  char *local_60;
  long local_58;
  char *local_50;
  char *local_48;
  char *local_40;
  char *local_38;
  char *local_30;
  mbstate_t local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_80 = *param_1;
  local_78 = 0;
  local_70 = 0;
  local_88 = get_quoting_style(param_4);
  if ((qmark_funny_chars == '\0') || (((local_88 != 1 && (local_88 != 2)) && (local_88 != 0)))) {
    bVar2 = false;
  }
  else {
    bVar2 = true;
  }
  if (param_5 == 0) {
    if (bVar2) {
      local_70 = strlen(param_3);
      if (param_2 <= local_70) {
        local_80 = (char *)xmalloc(local_70 + 1);
      }
      memcpy(local_80,param_3,local_70 + 1);
      bVar3 = false;
    }
    else {
      local_70 = strlen(param_3);
      bVar3 = false;
      local_80 = param_3;
    }
  }
  else {
    local_70 = quotearg_buffer(local_80,param_2,param_3,0xffffffffffffffff,param_4);
    if (param_2 <= local_70) {
      local_80 = (char *)xmalloc(local_70 + 1);
      quotearg_buffer(local_80,local_70 + 1,param_3,0xffffffffffffffff,param_4);
    }
    if ((*param_3 == *local_80) && (sVar7 = strlen(param_3), local_70 == sVar7)) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
  }
  if (bVar2) {
    sVar7 = __ctype_get_mb_cur_max();
    if (sVar7 < 2) {
      local_38 = local_80 + local_70;
      for (local_50 = local_80; local_50 < local_38; local_50 = local_50 + 1) {
        ppuVar8 = __ctype_b_loc();
        puVar1 = *ppuVar8;
        bVar4 = to_uchar((int)*local_50);
        if ((puVar1[bVar4] & 0x4000) == 0) {
          *local_50 = '?';
        }
      }
      local_78 = local_70;
    }
    else {
      local_68 = local_80;
      local_30 = local_80 + local_70;
      local_60 = local_80;
      local_78 = 0;
LAB_00108a37:
      if (local_68 < local_30) {
        iVar6 = (int)*local_68;
        if (iVar6 < 0x7f) {
          if (iVar6 < 0x61) {
            if (0x5f < iVar6) goto LAB_00108924;
            if (iVar6 < 0x41) {
              if (iVar6 < 0x24) {
                if (iVar6 < 0x20) goto LAB_00108924;
              }
              else if (0x1a < iVar6 - 0x25U) goto LAB_00108924;
            }
          }
          *local_60 = *local_68;
          local_78 = local_78 + 1;
          local_68 = local_68 + 1;
          local_60 = local_60 + 1;
        }
        else {
LAB_00108924:
          mbszero(&local_28);
          do {
            local_58 = rpl_mbrtoc32(&local_8c,local_68,(long)local_30 - (long)local_68,&local_28);
            if (local_58 == -1) {
              *local_60 = '?';
              local_78 = local_78 + 1;
              local_68 = local_68 + 1;
              local_60 = local_60 + 1;
              break;
            }
            if (local_58 == -2) {
              local_68 = local_30;
              *local_60 = '?';
              local_78 = local_78 + 1;
              local_60 = local_60 + 1;
              break;
            }
            if (local_58 == 0) {
              local_58 = 1;
            }
            local_84 = c32width(local_8c);
            if (local_84 < 0) {
              local_68 = local_68 + local_58;
              *local_60 = '?';
              local_78 = local_78 + 1;
              local_60 = local_60 + 1;
            }
            else {
              for (; local_58 != 0; local_58 = local_58 + -1) {
                *local_60 = *local_68;
                local_68 = local_68 + 1;
                local_60 = local_60 + 1;
              }
              local_78 = local_78 + (long)local_84;
            }
            iVar6 = mbsinit(&local_28);
          } while (iVar6 == 0);
        }
        goto LAB_00108a37;
      }
      local_70 = (long)local_60 - (long)local_80;
    }
  }
  else if (param_6 != (ulong *)0x0) {
    sVar7 = __ctype_get_mb_cur_max();
    if (sVar7 < 2) {
      local_40 = local_80 + local_70;
      local_78 = 0;
      for (local_48 = local_80; local_48 < local_40; local_48 = local_48 + 1) {
        ppuVar8 = __ctype_b_loc();
        puVar1 = *ppuVar8;
        bVar4 = to_uchar((int)*local_48);
        if ((puVar1[bVar4] & 0x4000) != 0) {
          local_78 = local_78 + 1;
        }
      }
    }
    else {
      iVar6 = mbsnwidth(local_80,local_70,3);
      local_78 = (ulong)iVar6;
    }
  }
  if (((align_variable_outer_quotes == '\0') || (cwd_some_quoted == '\0')) || (bVar3)) {
    uVar5 = 0;
  }
  else {
    uVar5 = 1;
  }
  *param_7 = uVar5;
  if (param_6 != (ulong *)0x0) {
    *param_6 = local_78;
  }
  *param_1 = local_80;
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_70;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: quote_name_width
long quote_name_width(undefined *param_1,undefined8 param_2,undefined4 param_3) {
  long in_FS_OFFSET;
  byte local_2029;
  undefined *local_2028;
  long local_2020;
  undefined local_2018 [8200];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_2028 = local_2018;
  quote_name_buf(&local_2028,0x2000,param_1,param_2,param_3,&local_2020,&local_2029);
  if ((local_2028 != local_2018) && (param_1 != local_2028)) {
    free(local_2028);
  }
  local_2020 = local_2020 + (ulong)local_2029;
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_2020;
}

// Function: file_escape
char * file_escape(char *param_1,char param_2) {
  byte bVar1;
  int iVar2;
  size_t sVar3;
  char *pcVar4;
  char *local_20;
  char *local_18;
  
  sVar3 = strlen(param_1);
  pcVar4 = (char *)xnmalloc(3,sVar3 + 1);
  local_20 = param_1;
  local_18 = pcVar4;
  while (*local_20 != '\0') {
    if ((param_2 == '\0') || (*local_20 != '/')) {
      bVar1 = to_uchar((int)*local_20);
      if (RFC3986[(int)(uint)bVar1] == '\0') {
        bVar1 = to_uchar((int)*local_20);
        iVar2 = sprintf(local_18,"%%%02x",(ulong)bVar1);
        local_18 = local_18 + iVar2;
        local_20 = local_20 + 1;
      }
      else {
        *local_18 = *local_20;
        local_20 = local_20 + 1;
        local_18 = local_18 + 1;
      }
    }
    else {
      *local_18 = '/';
      local_20 = local_20 + 1;
      local_18 = local_18 + 1;
    }
  }
  *local_18 = '\0';
  return pcVar4;
}

// Function: quote_name
long quote_name(char *param_1,undefined8 param_2,undefined4 param_3,long param_4,char param_5,
               long param_6,long param_7) {
  undefined *puVar1;
  long in_FS_OFFSET;
  byte local_203a;
  byte local_2039;
  char *local_2038;
  long local_2030;
  void *local_2028;
  char *local_2020;
  char local_2018 [8200];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_2038 = local_2018;
  local_2030 = quote_name_buf(&local_2038,0x2000,param_1,param_2,param_3,0,&local_203a);
  if ((local_203a != 0) && (param_5 != '\0')) {
    dired_outbyte(0x20);
  }
  if (param_4 != 0) {
    print_color_indicator(param_4);
  }
  local_2039 = 0;
  if (param_7 != 0) {
    if (((align_variable_outer_quotes != '\0') && (cwd_some_quoted != '\0')) && (local_203a != 1)) {
      local_2039 = 1;
      putchar_unlocked((int)*local_2038);
    }
    local_2028 = (void *)file_escape(hostname,0);
    local_2020 = (char *)file_escape(param_7,1);
    if (*local_2020 == '/') {
      puVar1 = &DAT_0010c134;
    }
    else {
      puVar1 = &DAT_0010e94d;
    }
    printf("\x1b]8;;file://%s%s%s\a",local_2028,puVar1,local_2020);
    free(local_2028);
    free(local_2020);
  }
  if (param_6 != 0) {
    push_current_dired_pos(param_6);
  }
  fwrite_unlocked(local_2038 + local_2039,1,local_2030 - (int)((uint)local_2039 * 2),_stdout);
  dired_pos = local_2030 + dired_pos;
  if (param_6 != 0) {
    push_current_dired_pos(param_6);
  }
  if (param_7 != 0) {
    fwrite_unlocked(&DAT_0010e963,1,6,_stdout);
    if (local_2039 != 0) {
      putchar_unlocked((int)local_2038[local_2030 + -1]);
    }
  }
  if ((local_2038 != local_2018) && (param_1 != local_2038)) {
    free(local_2038);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_2030 + (ulong)local_203a;
}

// Function: print_name_with_quoting
long print_name_with_quoting(undefined8 *param_1,char param_2,undefined8 param_3,ulong param_4) {
  char cVar1;
  bool bVar2;
  undefined8 uVar3;
  long lVar4;
  ulong uVar5;
  
  if (param_2 == '\0') {
    uVar3 = *param_1;
  }
  else {
    uVar3 = param_1[1];
  }
  if (print_with_color == '\0') {
    lVar4 = 0;
  }
  else {
    lVar4 = get_color_indicator(param_1,param_2);
  }
  if (print_with_color == '\0') {
LAB_00109198:
    bVar2 = false;
  }
  else {
    if (lVar4 == 0) {
      cVar1 = is_colored(4);
      if (cVar1 == '\0') goto LAB_00109198;
    }
    bVar2 = true;
  }
  lVar4 = quote_name(uVar3,filename_quoting_options,*(undefined4 *)((long)param_1 + 0xc4),lVar4,
                     param_2 == '\0',param_3,param_1[2]);
  process_signals();
  if (bVar2) {
    prep_non_filename_text();
    if (line_length != 0) {
      uVar5 = (lVar4 + param_4) - 1;
      if (param_4 / line_length != uVar5 / line_length) {
        put_indicator(&DAT_00111270,line_length,uVar5 % line_length);
      }
    }
  }
  return lVar4;
}

// Function: prep_non_filename_text
void prep_non_filename_text(void) {
  if (_DAT_00111128 == 0) {
    put_indicator(&color_indicator);
    put_indicator(&DAT_00111130);
    put_indicator(&DAT_00111110);
  }
  else {
    put_indicator(&DAT_00111120);
  }
  return;
}

// Function: print_file_name_and_frills
long print_file_name_and_frills(long param_1,undefined8 param_2) {
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined8 uVar4;
  undefined1 *puVar5;
  long in_FS_OFFSET;
  uint local_2c0;
  long local_2b8;
  undefined local_2a8 [664];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  set_normal_color();
  if (print_inode != '\0') {
    uVar4 = format_inode(local_2a8,param_1);
    uVar2 = inode_number_width;
    if (format == 4) {
      uVar2 = 0;
    }
    printf("%*s ",(ulong)uVar2,uVar4);
  }
  if (print_block_size != '\0') {
    if (*(char *)(param_1 + 0xb8) == '\x01') {
      puVar5 = (undefined1 *)
               human_readable(*(undefined8 *)(param_1 + 0x58),local_2a8,human_output_opts,0x200,
                              output_block_size);
    }
    else {
      puVar5 = &DAT_0010e922;
    }
    iVar3 = gnu_mbswidth(puVar5,3);
    local_2c0 = 0;
    if (((-1 < iVar3) && (block_size_width != 0)) && (format != 4)) {
      local_2c0 = block_size_width - iVar3;
    }
    printf("%*s%s ",(ulong)local_2c0,&DAT_0010c134,puVar5);
  }
  if (print_scontext != '\0') {
    uVar2 = scontext_width;
    if (format == 4) {
      uVar2 = 0;
    }
    printf("%*s ",(ulong)uVar2,*(undefined8 *)(param_1 + 0xb0));
  }
  local_2b8 = print_name_with_quoting(param_1,0,0,param_2);
  if (indicator_style != 0) {
    bVar1 = print_type_indicator
                      (*(undefined *)(param_1 + 0xb8),*(undefined4 *)(param_1 + 0x30),
                       *(undefined4 *)(param_1 + 0xa8));
    local_2b8 = local_2b8 + (ulong)bVar1;
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_2b8;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: get_type_indicator
undefined get_type_indicator(char param_1,uint param_2,int param_3) {
  bool bVar1;
  undefined local_9;
  
  if (param_1 == '\0') {
    bVar1 = param_3 == 5;
  }
  else {
    bVar1 = (param_2 & 0xf000) == 0x8000;
  }
  if (bVar1) {
    if (((param_1 == '\0') || (indicator_style != 3)) || ((param_2 & 0x49) == 0)) {
      local_9 = 0;
    }
    else {
      local_9 = 0x2a;
    }
  }
  else {
    if (param_1 == '\0') {
      if ((param_3 == 3) || (param_3 == 9)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
    }
    else {
      bVar1 = (param_2 & 0xf000) == 0x4000;
    }
    if (bVar1) {
      local_9 = 0x2f;
    }
    else if (indicator_style == 1) {
      local_9 = 0;
    }
    else {
      if (param_1 == '\0') {
        bVar1 = param_3 == 6;
      }
      else {
        bVar1 = (param_2 & 0xf000) == 0xa000;
      }
      if (bVar1) {
        local_9 = 0x40;
      }
      else {
        if (param_1 == '\0') {
          bVar1 = param_3 == 1;
        }
        else {
          bVar1 = (param_2 & 0xf000) == 0x1000;
        }
        if (bVar1) {
          local_9 = 0x7c;
        }
        else {
          if (param_1 == '\0') {
            bVar1 = param_3 == 7;
          }
          else {
            bVar1 = (param_2 & 0xf000) == 0xc000;
          }
          if (bVar1) {
            local_9 = 0x3d;
          }
          else {
            local_9 = 0;
          }
        }
      }
    }
  }
  return local_9;
}

// Function: print_type_indicator
bool print_type_indicator(undefined param_1,undefined4 param_2,undefined4 param_3) {
  char cVar1;
  
  cVar1 = get_type_indicator(param_1,param_2,param_3);
  if (cVar1 != '\0') {
    dired_outbyte((int)cVar1);
  }
  return cVar1 != '\0';
}

// Function: print_color_indicator
bool print_color_indicator(long param_1) {
  char cVar1;
  
  if (param_1 != 0) {
    cVar1 = is_colored(4);
    if (cVar1 != '\0') {
      restore_default_color();
    }
    put_indicator(&color_indicator);
    put_indicator(param_1);
    put_indicator(&DAT_00111110);
  }
  return param_1 != 0;
}

// Function: get_color_indicator
ulong * get_color_indicator(char **param_1,char param_2) {
  char cVar1;
  int iVar2;
  size_t sVar3;
  uint local_34;
  uint local_30;
  uint local_2c;
  ulong *local_28;
  char *local_20;
  
  if (param_2 == '\0') {
    local_20 = *param_1;
    local_30 = file_or_link_mode(param_1);
    local_2c = (uint)*(byte *)((long)param_1 + 0xb9);
  }
  else {
    local_20 = param_1[1];
    local_30 = *(uint *)((long)param_1 + 0xac);
    if (*(char *)((long)param_1 + 0xb9) == '\0') {
      local_2c = 0xffffffff;
    }
    else {
      local_2c = 0;
    }
  }
  if ((local_2c == 0xffffffff) && (cVar1 = is_colored(0xc), cVar1 != '\0')) {
    local_34 = 0xc;
  }
  else if (*(char *)(param_1 + 0x17) == '\x01') {
    if ((local_30 & 0xf000) == 0x8000) {
      local_34 = 5;
      if (((local_30 & 0x800) == 0) || (cVar1 = is_colored(0x10), cVar1 == '\0')) {
        if (((local_30 & 0x400) == 0) || (cVar1 = is_colored(0x11), cVar1 == '\0')) {
          if (*(char *)(param_1 + 0x18) == '\0') {
            if (((local_30 & 0x49) == 0) || (cVar1 = is_colored(0xe), cVar1 == '\0')) {
              if (((char *)0x1 < param_1[5]) && (cVar1 = is_colored(0x16), cVar1 != '\0')) {
                local_34 = 0x16;
              }
            }
            else {
              local_34 = 0xe;
            }
          }
          else {
            local_34 = 0x15;
          }
        }
        else {
          local_34 = 0x11;
        }
      }
      else {
        local_34 = 0x10;
      }
    }
    else if ((local_30 & 0xf000) == 0x4000) {
      local_34 = 6;
      if ((((local_30 & 0x200) == 0) || ((local_30 & 2) == 0)) ||
         (cVar1 = is_colored(0x14), cVar1 == '\0')) {
        if (((local_30 & 2) == 0) || (cVar1 = is_colored(0x13), cVar1 == '\0')) {
          if (((local_30 & 0x200) != 0) && (cVar1 = is_colored(0x12), cVar1 != '\0')) {
            local_34 = 0x12;
          }
        }
        else {
          local_34 = 0x13;
        }
      }
      else {
        local_34 = 0x14;
      }
    }
    else if ((local_30 & 0xf000) == 0xa000) {
      local_34 = 7;
    }
    else if ((local_30 & 0xf000) == 0x1000) {
      local_34 = 8;
    }
    else if ((local_30 & 0xf000) == 0xc000) {
      local_34 = 9;
    }
    else if ((local_30 & 0xf000) == 0x6000) {
      local_34 = 10;
    }
    else if ((local_30 & 0xf000) == 0x2000) {
      local_34 = 0xb;
    }
    else {
      local_34 = 0xd;
    }
  }
  else {
    local_34 = *(uint *)(filetype_indicator_1 + (ulong)*(uint *)(param_1 + 0x15) * 4);
  }
  local_28 = (ulong *)0x0;
  if (local_34 == 5) {
    sVar3 = strlen(local_20);
    for (local_28 = color_ext_list; local_28 != (ulong *)0x0; local_28 = (ulong *)local_28[5]) {
      if (*local_28 <= sVar3) {
        if (*(char *)(local_28 + 4) == '\0') {
          iVar2 = c_strncasecmp(local_20 + (sVar3 - *local_28),local_28[1],*local_28);
        }
        else {
          iVar2 = strncmp(local_20 + (sVar3 - *local_28),(char *)local_28[1],*local_28);
        }
        if (iVar2 == 0) break;
      }
    }
  }
  if (((local_34 == 7) && (local_2c == 0)) &&
     ((color_symlink_as_referent != '\0' || (cVar1 = is_colored(0xd), cVar1 != '\0')))) {
    local_34 = 0xd;
  }
  if (local_28 == (ulong *)0x0) {
    local_28 = &color_indicator + (ulong)local_34 * 2;
  }
  else {
    local_28 = local_28 + 2;
  }
  if (local_28[1] == 0) {
    local_28 = (ulong *)0x0;
  }
  return local_28;
}

// Function: put_indicator
void put_indicator(size_t *param_1) {
  __pid_t _Var1;
  
  if (used_color != '\x01') {
    used_color = '\x01';
    _Var1 = tcgetpgrp(1);
    if (-1 < _Var1) {
      signal_init();
    }
    prep_non_filename_text();
  }
  fwrite_unlocked((void *)param_1[1],*param_1,1,_stdout);
  return;
}

// Function: length_of_file_name_and_frills
long length_of_file_name_and_frills(long param_1) {
  char cVar1;
  char *pcVar2;
  size_t sVar3;
  long lVar4;
  long in_FS_OFFSET;
  long local_2b0;
  undefined local_2a8 [664];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_2b0 = 0;
  if (print_inode != '\0') {
    if (format == 4) {
      pcVar2 = (char *)umaxtostr(*(undefined8 *)(param_1 + 0x20),local_2a8);
      sVar3 = strlen(pcVar2);
    }
    else {
      sVar3 = (size_t)inode_number_width;
    }
    local_2b0 = sVar3 + 1;
  }
  if (print_block_size != '\0') {
    if (format == 4) {
      if (*(char *)(param_1 + 0xb8) == '\x01') {
        pcVar2 = (char *)human_readable(*(undefined8 *)(param_1 + 0x58),local_2a8,human_output_opts,
                                        0x200,output_block_size);
      }
      else {
        pcVar2 = "?";
      }
      sVar3 = strlen(pcVar2);
    }
    else {
      sVar3 = (size_t)block_size_width;
    }
    local_2b0 = local_2b0 + sVar3 + 1;
  }
  if (print_scontext != '\0') {
    if (format == 4) {
      sVar3 = strlen(*(char **)(param_1 + 0xb0));
    }
    else {
      sVar3 = (size_t)scontext_width;
    }
    local_2b0 = local_2b0 + sVar3 + 1;
  }
  lVar4 = fileinfo_name_width(param_1);
  local_2b0 = local_2b0 + lVar4;
  if (indicator_style != 0) {
    cVar1 = get_type_indicator(*(undefined *)(param_1 + 0xb8),*(undefined4 *)(param_1 + 0x30),
                               *(undefined4 *)(param_1 + 0xa8));
    local_2b0 = local_2b0 + (ulong)(cVar1 != '\0');
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_2b0;
}

// Function: print_many_per_line
void print_many_per_line(void) {
  undefined8 uVar1;
  long lVar2;
  long lVar3;
  long lVar4;
  long lVar5;
  long local_58;
  long local_50;
  long local_48;
  long local_40;
  
  lVar3 = calculate_columns(1);
  lVar4 = lVar3 * 0x18 + column_info;
  lVar3 = (ulong)(cwd_n_used % lVar3 != 0) + cwd_n_used / lVar3;
  for (local_58 = 0; local_58 < lVar3; local_58 = local_58 + 1) {
    local_50 = 0;
    local_48 = local_58;
    local_40 = 0;
    while( true ) {
      uVar1 = *(undefined8 *)(sorted_file + local_48 * 8);
      lVar5 = length_of_file_name_and_frills(uVar1);
      lVar2 = *(long *)(local_50 * 8 + *(long *)(lVar4 + -8));
      print_file_name_and_frills(uVar1,local_40);
      if (cwd_n_used - lVar3 <= local_48) break;
      local_48 = local_48 + lVar3;
      indent(lVar5 + local_40,local_40 + lVar2);
      local_40 = local_40 + lVar2;
      local_50 = local_50 + 1;
    }
    putchar_unlocked((int)eolbyte);
  }
  return;
}

// Function: print_horizontal
void print_horizontal(void) {
  undefined8 uVar1;
  long lVar2;
  long lVar3;
  long local_48;
  long local_40;
  long local_38;
  long local_30;
  
  local_48 = 0;
  lVar2 = calculate_columns(0);
  lVar3 = lVar2 * 0x18 + -0x18 + column_info;
  uVar1 = *sorted_file;
  local_40 = length_of_file_name_and_frills(uVar1);
  local_38 = **(long **)(lVar3 + 0x10);
  print_file_name_and_frills(uVar1,0);
  for (local_30 = 1; local_30 < cwd_n_used; local_30 = local_30 + 1) {
    if (local_30 % lVar2 == 0) {
      putchar_unlocked((int)eolbyte);
      local_48 = 0;
    }
    else {
      indent(local_40 + local_48,local_48 + local_38);
      local_48 = local_48 + local_38;
    }
    uVar1 = sorted_file[local_30];
    print_file_name_and_frills(uVar1,local_48);
    local_40 = length_of_file_name_and_frills(uVar1);
    local_38 = *(long *)(*(long *)(lVar3 + 0x10) + (local_30 % lVar2) * 8);
  }
  putchar_unlocked((int)eolbyte);
  return;
}

// Function: print_with_separator
void print_with_separator(char param_1) {
  undefined8 uVar1;
  long lVar2;
  char local_29;
  ulong local_28;
  long local_20;
  
  local_28 = 0;
  for (local_20 = 0; local_20 < cwd_n_used; local_20 = local_20 + 1) {
    uVar1 = *(undefined8 *)(sorted_file + local_20 * 8);
    if (line_length == 0) {
      lVar2 = 0;
    }
    else {
      lVar2 = length_of_file_name_and_frills(uVar1);
    }
    if (local_20 != 0) {
      if ((line_length == 0) || ((lVar2 + local_28 + 2 < line_length && (local_28 <= -lVar2 - 3U))))
      {
        local_28 = local_28 + 2;
        local_29 = ' ';
      }
      else {
        local_28 = 0;
        local_29 = eolbyte;
      }
      putchar_unlocked((int)param_1);
      putchar_unlocked((int)local_29);
    }
    print_file_name_and_frills(uVar1,local_28);
    local_28 = local_28 + lVar2;
  }
  putchar_unlocked((int)eolbyte);
  return;
}

// Function: indent
void indent(ulong param_1,ulong param_2) {
  ulong local_10;
  
  local_10 = param_1;
  while (local_10 < param_2) {
    if ((tabsize == 0) || (param_2 / tabsize <= (local_10 + 1) / tabsize)) {
      putchar_unlocked(0x20);
      local_10 = local_10 + 1;
    }
    else {
      putchar_unlocked(9);
      local_10 = local_10 + (tabsize - local_10 % tabsize);
    }
  }
  return;
}

// Function: attach
void attach(char *param_1,char *param_2,char *param_3) {
  char *local_30;
  char *local_20;
  char *local_10;
  
  local_20 = param_1;
  local_10 = param_2;
  if ((*param_2 != '.') ||
     (local_30 = param_3, local_20 = param_1, local_10 = param_2, param_2[1] != '\0')) {
    while (*local_10 != '\0') {
      *local_20 = *local_10;
      local_20 = local_20 + 1;
      local_10 = local_10 + 1;
    }
    local_30 = param_3;
    if ((param_2 < local_10) && (local_30 = param_3, local_10[-1] != '/')) {
      *local_20 = '/';
      local_30 = param_3;
      local_20 = local_20 + 1;
    }
  }
  while (*local_30 != '\0') {
    *local_20 = *local_30;
    local_30 = local_30 + 1;
    local_20 = local_20 + 1;
  }
  *local_20 = '\0';
  return;
}

// Function: init_column_info
void init_column_info(long param_1) {
  long lVar1;
  undefined auVar2 [16];
  long in_FS_OFFSET;
  long local_48;
  long local_40;
  long local_38;
  long local_30;
  long local_28;
  
  local_38 = column_info_alloc_0;
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  if (column_info_alloc_0 < param_1) {
    column_info = xpalloc(column_info,&column_info_alloc_0,param_1 - column_info_alloc_0,
                          0xffffffffffffffff,0x18);
    local_48 = column_info_alloc_0 + local_38 + 1;
    if ((SCARRY8(column_info_alloc_0,local_38 + 1)) ||
       (auVar2 = SEXT816(local_48), local_48 = local_48 * (column_info_alloc_0 - local_38),
       SEXT816(local_48) != auVar2 * SEXT816(column_info_alloc_0 - local_38))) {
      xalloc_die();
    }
    local_40 = xinmalloc(local_48 >> 1,8);
    for (; local_38 < column_info_alloc_0; local_38 = local_38 + 1) {
      *(long *)(column_info + local_38 * 0x18 + 0x10) = local_40;
      local_40 = local_40 + (local_38 + 1) * 8;
    }
  }
  for (local_30 = 0; local_30 < param_1; local_30 = local_30 + 1) {
    *(undefined *)(local_30 * 0x18 + column_info) = 1;
    *(long *)(local_30 * 0x18 + column_info + 8) = (local_30 + 1) * 3;
    for (local_28 = 0; local_28 <= local_30; local_28 = local_28 + 1) {
      *(undefined8 *)(*(long *)(local_30 * 0x18 + column_info + 0x10) + local_28 * 8) = 3;
    }
  }
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: calculate_columns
ulong calculate_columns(char param_1) {
  long lVar1;
  long lVar2;
  ulong uVar3;
  long lVar4;
  long local_48;
  long local_40;
  ulong local_38;
  
  local_38 = cwd_n_used;
  if ((max_idx != 0) && (max_idx < cwd_n_used)) {
    local_38 = max_idx;
  }
  init_column_info(local_38);
  for (local_48 = 0; local_48 < (long)cwd_n_used; local_48 = local_48 + 1) {
    lVar1 = length_of_file_name_and_frills(*(undefined8 *)(sorted_file + local_48 * 8));
    for (local_40 = 0; local_40 < (long)local_38; local_40 = local_40 + 1) {
      if (*(char *)(local_40 * 0x18 + column_info) != '\0') {
        if (param_1 == '\0') {
          lVar2 = local_48 % (local_40 + 1);
        }
        else {
          lVar2 = local_48 / ((long)(local_40 + cwd_n_used) / (local_40 + 1));
        }
        if (lVar2 == local_40) {
          lVar4 = 0;
        }
        else {
          lVar4 = 2;
        }
        uVar3 = lVar1 + lVar4;
        if (*(ulong *)(*(long *)(local_40 * 0x18 + column_info + 0x10) + lVar2 * 8) < uVar3) {
          *(ulong *)(local_40 * 0x18 + column_info + 8) =
               *(long *)(local_40 * 0x18 + column_info + 8) +
               (uVar3 - *(long *)(*(long *)(local_40 * 0x18 + column_info + 0x10) + lVar2 * 8));
          *(ulong *)(lVar2 * 8 + *(long *)(local_40 * 0x18 + column_info + 0x10)) = uVar3;
          *(bool *)(local_40 * 0x18 + column_info) =
               *(ulong *)(local_40 * 0x18 + column_info + 8) < line_length;
        }
      }
    }
  }
  while ((1 < (long)local_38 && (*(char *)(local_38 * 0x18 + -0x18 + column_info) == '\0'))) {
    local_38 = local_38 - 1;
  }
  return local_38;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  undefined *puVar4;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [FILE]...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "List information about the FILEs (the current directory by default).\nSort entries alphabetically if none of -cftuvSUX nor --sort is specified.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -a, --all                  do not ignore entries starting with .\n  -A, --almost-all           do not list implied . and ..\n      --author               with -l, print the author of each file\n  -b, --escape               print C-style escapes for nongraphic characters\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --block-size=SIZE      with -l, scale sizes by SIZE when printing them;\n                             e.g., \'--block-size=M\'; see SIZE format below\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -B, --ignore-backups       do not list implied entries ending with ~\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -c                         with -lt: sort by, and show, ctime (time of last\n                             change of file status information);\n                             with -l: show ctime and sort by name;\n                             otherwise: sort by ctime, newest first\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -C                         list entries by columns\n      --color[=WHEN]         color the output WHEN; more info below\n  -d, --directory            list directories themselves, not their contents\n  -D, --dired                generate output designed for Emacs\' dired mode\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -f                         same as -a -U\n  -F, --classify[=WHEN]      append indicator (one of */=>@|) to entries WHEN\n      --file-type            likewise, except do not append \'*\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --format=WORD          across,horizontal (-x), commas (-m), long (-l),\n                             single-column (-1), verbose (-l), vertical (-C)\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --full-time            like -l --time-style=full-iso\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -g                         like -l, but do not list owner\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --group-directories-first\n                             group directories before files\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -G, --no-group             in a long listing, don\'t print group names\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -h, --human-readable       with -l and -s, print sizes like 1K 234M 2G etc.\n      --si                   likewise, but use powers of 1000 not 1024\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -H, --dereference-command-line\n                             follow symbolic links listed on the command line\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --dereference-command-line-symlink-to-dir\n                             follow each command line symbolic link\n                             that points to a directory\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --hide=PATTERN         do not list implied entries matching shell PATTERN\n                             (overridden by -a or -A)\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --hyperlink[=WHEN]     hyperlink file names WHEN\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --indicator-style=WORD\n                             append indicator with style WORD to entry names:\n                             none (default), slash (-p),\n                             file-type (--file-type), classify (-F)\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -i, --inode                print the index number of each file\n  -I, --ignore=PATTERN       do not list implied entries matching shell PATTERN\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -k, --kibibytes            default to 1024-byte blocks for file system usage;\n                             used only with -s and per directory totals\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -l                         use a long listing format\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -L, --dereference          when showing file information for a symbolic\n                             link, show information for the file the link\n                             references rather than for the link itself\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -m                         fill width with a comma separated list of entries\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -n, --numeric-uid-gid      like -l, but list numeric user and group IDs\n  -N, --literal              print entry names without quoting\n  -o                         like -l, but do not list group information\n  -p, --indicator-style=slash\n                             append / indicator to directories\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -q, --hide-control-chars   print ? instead of nongraphic characters\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --show-control-chars   show nongraphic characters as-is (the default,\n                             unless program is \'ls\' and output is a terminal)\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -Q, --quote-name           enclose entry names in double quotes\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --quoting-style=WORD   use quoting style WORD for entry names:\n                             literal, locale, shell, shell-always,\n                             shell-escape, shell-escape-always, c, escape\n                             (overrides QUOTING_STYLE environment variable)\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -r, --reverse              reverse order while sorting\n  -R, --recursive            list subdirectories recursively\n  -s, --size                 print the allocated size of each file, in blocks\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -S                         sort by file size, largest first\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --sort=WORD            change default \'name\' sort to WORD:\n                               none (-U), size (-S), time (-t),\n                               version (-v), extension (-X), name, width\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --time=WORD            select which timestamp used to display or sort;\n                               access time (-u): atime, access, use;\n                               metadata change time (-c): ctime, status;\n                               modified time (default): mtime, modification;\n                               birth time: birth, creation;\n                             with -l, WORD determines which time to show;\n                             with --sort=time, sort by WORD (newest first)\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --time-style=TIME_STYLE\n                             time/date format with -l; see TIME_STYLE below\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -t                         sort by time, newest first; see --time\n  -T, --tabsize=COLS         assume tab stops at each COLS instead of 8\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -u                         with -lt: sort by, and show, access time;\n                             with -l: show access time and sort by name;\n                             otherwise: sort by access time, newest first\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -U                         do not sort directory entries\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -v                         natural sort of (version) numbers within text\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -w, --width=COLS           set output width to COLS.  0 means no limit\n  -x                         list entries by lines instead of by columns\n  -X                         sort alphabetically by entry extension\n  -Z, --context              print any security context of each file\n      --zero                 end each output line with NUL, not newline\n  -1                         list one file per line\n"
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
                            "\nThe TIME_STYLE argument can be full-iso, long-iso, iso, locale, or +FORMAT.\nFORMAT is interpreted like in date(1).  If FORMAT is FORMAT1<newline>FORMAT2,\nthen FORMAT1 applies to non-recent files and FORMAT2 to recent files.\nTIME_STYLE prefixed with \'posix-\' takes effect only outside the POSIX locale.\nAlso the TIME_STYLE environment variable sets the default style to use.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nThe WHEN argument defaults to \'always\' and can also be \'auto\' or \'never\'.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nUsing color to distinguish file types is disabled both by default and\nwith --color=never.  With --color=auto, ls emits color codes only when\nstandard output is connected to a terminal.  The LS_COLORS environment\nvariable can change the settings.  Use the dircolors(1) command to set it.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nExit status:\n 0  if OK,\n 1  if minor problems (e.g., cannot access subdirectory),\n 2  if serious trouble (e.g., cannot access command-line argument).\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    if (_ls_mode == 1) {
      puVar4 = &DAT_0010dfcb;
    }
    else if (_ls_mode == 2) {
      puVar4 = &DAT_0010dfc2;
    }
    else {
      puVar4 = &DAT_0010dfc6;
    }
    emit_ancillary_info(puVar4);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

