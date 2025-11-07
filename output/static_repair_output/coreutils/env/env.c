// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [-] [NAME=VALUE]... [COMMAND [ARG]...]\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Set each NAME to VALUE in the environment and run COMMAND.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -a, --argv0=ARG      pass ARG as the zeroth argument of COMMAND\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -i, --ignore-environment  start with an empty environment\n  -0, --null           end each output line with NUL, not newline\n  -u, --unset=NAME     remove variable from the environment\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -C, --chdir=DIR      change working directory to DIR\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -S, --split-string=S  process and split S into separate arguments;\n                        used to pass multiple arguments on shebang lines\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --block-signal[=SIG]    block delivery of SIG signal(s) to COMMAND\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --default-signal[=SIG]  reset handling of SIG signal(s) to the default\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --ignore-signal[=SIG]   set handling of SIG signal(s) to do nothing\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --list-signal-handling  list non default signal handling to standard error\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -v, --debug          print verbose information for each processing step\n"
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
                            "\nA mere - implies -i.  If no COMMAND, print the resulting environment.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nSIG may be a signal name like \'PIPE\', or a signal number like \'13\'.\nWithout SIG, all known signals are included.  Multiple signals can be\ncomma-separated.  An empty SIG argument is a no-op.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_exec_status(&DAT_00103248);
    emit_ancillary_info(&DAT_00103248);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: append_unset_var
void append_unset_var(undefined8 param_1) {
  long lVar1;
  
  if (usvars_used == usvars_alloc) {
    usvars = xpalloc(usvars,&usvars_alloc,1,0xffffffffffffffff,8);
  }
  lVar1 = usvars_used * 8;
  usvars_used = usvars_used + 1;
  *(undefined8 *)(usvars + lVar1) = param_1;
  return;
}

// Function: unset_envvars
void unset_envvars(void) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  long local_28;
  
  for (local_28 = 0; local_28 < usvars_used; local_28 = local_28 + 1) {
    if (dev_debug != '\0') {
      fprintf(_stderr,"unset:    %s\n",*(undefined8 *)(usvars + local_28 * 8));
    }
    iVar1 = unsetenv(*(char **)(usvars + local_28 * 8));
    if (iVar1 != 0) {
      uVar2 = quote(*(undefined8 *)(usvars + local_28 * 8));
      uVar3 = gettext("cannot unset %s");
      piVar4 = __errno_location();
      error(0x7d,*piVar4,uVar3,uVar2);
    }
  }
  return;
}

// Function: scan_varname
char * scan_varname(long param_1) {
  char cVar1;
  char *local_10;
  
  if ((*(char *)(param_1 + 1) == '{') &&
     ((cVar1 = c_isalpha((int)*(char *)(param_1 + 2)), cVar1 != '\0' ||
      (*(char *)(param_1 + 2) == '_')))) {
    for (local_10 = (char *)(param_1 + 3);
        (cVar1 = c_isalnum((int)*local_10), cVar1 != '\0' || (*local_10 == '_'));
        local_10 = local_10 + 1) {
    }
    if (*local_10 == '}') {
      return local_10;
    }
  }
  return (char *)0x0;
}

// Function: extract_varname
void * extract_varname(long param_1) {
  long lVar1;
  size_t __n;
  void *pvVar2;
  
  lVar1 = scan_varname(param_1);
  if (lVar1 == 0) {
    pvVar2 = (void *)0x0;
  }
  else {
    __n = (lVar1 - param_1) - 2;
    if (vnlen <= (long)__n) {
      free(varname);
      varname = (void *)xpalloc(0,&vnlen,((lVar1 - param_1) + -1) - vnlen,0xffffffffffffffff,1);
    }
    memcpy(varname,(void *)(param_1 + 2),__n);
    *(undefined *)(__n + (long)varname) = 0;
    pvVar2 = varname;
  }
  return pvVar2;
}

// Function: splitbuf_grow
void splitbuf_grow(long *param_1) {
  long lVar1;
  size_t __n;
  long lVar2;
  
  lVar1 = param_1[2];
  __n = *(size_t *)((long)*(int *)(param_1 + 1) * 8 + *param_1);
  lVar2 = xpalloc(*param_1,param_1 + 2,1,0x7fffffff,0x10);
  *param_1 = lVar2;
  memmove((void *)(*param_1 + param_1[2] * 8),(void *)(*param_1 + lVar1 * 8),__n);
  return;
}

// Function: splitbuf_append_byte
void splitbuf_append_byte(long *param_1,undefined param_2) {
  ulong uVar1;
  
  uVar1 = *(ulong *)((long)*(int *)(param_1 + 1) * 8 + *param_1);
  if ((ulong)(param_1[2] << 3) <= uVar1) {
    splitbuf_grow(param_1);
  }
  *(undefined *)(*param_1 + uVar1 + param_1[2] * 8) = param_2;
  *(ulong *)((long)*(int *)(param_1 + 1) * 8 + *param_1) = uVar1 + 1;
  return;
}

// Function: check_start_new_arg
void check_start_new_arg(long *param_1) {
  int iVar1;
  
  if (*(char *)((long)param_1 + 0x1c) != '\0') {
    splitbuf_append_byte(param_1,0);
    iVar1 = *(int *)(param_1 + 1);
    if (param_1[2] <= (long)(iVar1 + *(int *)(param_1 + 3) + 1)) {
      splitbuf_grow(param_1);
    }
    *(undefined8 *)(((long)iVar1 + 1) * 8 + *param_1) = *(undefined8 *)(*param_1 + (long)iVar1 * 8);
    *(int *)(param_1 + 1) = iVar1 + 1;
    *(undefined *)((long)param_1 + 0x1c) = 0;
  }
  return;
}

// Function: splitbuf_finishup
long splitbuf_finishup(long *param_1) {
  int iVar1;
  long lVar2;
  long lVar3;
  long lVar4;
  int local_20;
  
  iVar1 = *(int *)(param_1 + 1);
  lVar2 = *param_1;
  lVar3 = *param_1;
  lVar4 = param_1[2];
  for (local_20 = 1; local_20 < iVar1; local_20 = local_20 + 1) {
    *(long *)(lVar2 + (long)local_20 * 8) =
         lVar4 * 8 + lVar3 + *(long *)(lVar2 + (long)local_20 * 8);
  }
  return lVar2;
}

// Function: build_argv
void build_argv(char *param_1,int param_2,undefined4 *param_3) {
  bool bVar1;
  bool bVar2;
  int iVar3;
  size_t sVar4;
  char *pcVar5;
  undefined8 uVar6;
  long in_FS_OFFSET;
  char *local_c0;
  bool local_ab;
  bool local_aa;
  char local_a9;
  char *local_80;
  long local_48;
  undefined4 local_40;
  long local_38;
  int local_30;
  char local_2c;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_ab = false;
  local_aa = false;
  local_48 = xnmalloc((long)(param_2 + 2),0x10);
  local_40 = 1;
  local_38 = (long)(param_2 + 2);
  local_2c = '\x01';
  *(undefined8 *)(local_48 + 8) = 0;
  local_c0 = param_1;
  local_30 = param_2;
  bVar1 = false;
  bVar2 = false;
LAB_00101011:
  if (*local_c0 == '\0') {
    if ((bVar1) || (bVar2)) {
      uVar6 = gettext("no terminating quote in -S string");
      error(0x7d,0,uVar6);
    }
LAB_00101076:
    splitbuf_append_byte(&local_48,0);
    *param_3 = local_40;
    splitbuf_finishup(&local_48);
    if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    return;
  }
  local_a9 = *local_c0;
  iVar3 = (int)*local_c0;
  pcVar5 = local_c0;
  if (iVar3 < 0x28) {
    if (iVar3 < 9) goto switchD_00100bb0_caseD_e;
    switch(iVar3) {
    case 9:
    case 10:
    case 0xb:
    case 0xc:
    case 0xd:
    case 0x20:
      if ((!bVar2) && (!bVar1)) {
        local_2c = '\x01';
        sVar4 = strspn(local_c0," \t\n\v\f\r");
        local_c0 = local_c0 + sVar4;
        goto LAB_00101011;
      }
      break;
    case 0x22:
      if (!bVar2) {
        local_ab = local_ab == false;
        check_start_new_arg(&local_48);
        local_c0 = local_c0 + 1;
        bVar1 = local_ab;
        goto LAB_00101011;
      }
      break;
    case 0x23:
      if (local_2c == '\x01') goto LAB_00101076;
      break;
    case 0x24:
      if (!bVar2) {
        pcVar5 = (char *)extract_varname(local_c0);
        if (pcVar5 == (char *)0x0) {
          uVar6 = gettext("only ${VARNAME} expansion is supported, error at: %s");
          error(0x7d,0,uVar6,local_c0);
        }
        local_80 = getenv(pcVar5);
        if (local_80 == (char *)0x0) {
          if (dev_debug != '\0') {
            fprintf(_stderr,"replacing ${%s} with null string\n",pcVar5);
          }
        }
        else {
          check_start_new_arg(&local_48);
          if (dev_debug != '\0') {
            uVar6 = quote(local_80);
            fprintf(_stderr,"expanding ${%s} into %s\n",pcVar5,uVar6);
          }
          for (; *local_80 != '\0'; local_80 = local_80 + 1) {
            splitbuf_append_byte(&local_48,(int)*local_80);
          }
        }
        local_c0 = strchr(local_c0,0x7d);
        local_c0 = local_c0 + 1;
        goto LAB_00101011;
      }
      break;
    case 0x27:
      if (!bVar1) {
        local_aa = local_aa == false;
        check_start_new_arg(&local_48);
        local_c0 = local_c0 + 1;
        bVar2 = local_aa;
        goto LAB_00101011;
      }
    }
  }
  else {
    if ((iVar3 != 0x5c) || (((bVar2 && (local_c0[1] != '\\')) && (local_c0[1] != '\''))))
    goto switchD_00100bb0_caseD_e;
    pcVar5 = local_c0 + 1;
    local_a9 = *pcVar5;
    iVar3 = (int)local_a9;
    if (local_a9 == '\0') {
      uVar6 = gettext("invalid backslash at end of string in -S");
      error(0x7d,0,uVar6);
switchD_00100d40_caseD_25:
      uVar6 = gettext("invalid sequence \'\\%c\' in -S");
      error(0x7d,0,uVar6,(int)local_a9);
      goto switchD_00100bb0_caseD_e;
    }
    if (((local_a9 < '\0') || (0x76 < iVar3)) || (iVar3 < 0x22)) goto switchD_00100d40_caseD_25;
    switch(iVar3) {
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x27:
    case 0x5c:
      break;
    default:
      goto switchD_00100d40_caseD_25;
    case 0x5f:
      if (bVar1) {
        local_a9 = ' ';
        break;
      }
      local_c0 = local_c0 + 2;
      local_2c = '\x01';
      goto LAB_00101011;
    case 99:
      if (bVar1) {
        uVar6 = gettext("\'\\c\' must not appear in double-quoted -S string");
        error(0x7d,0,uVar6);
        goto switchD_00100d40_caseD_66;
      }
      goto LAB_00101076;
    case 0x66:
switchD_00100d40_caseD_66:
      local_a9 = '\f';
      break;
    case 0x6e:
      local_a9 = '\n';
      break;
    case 0x72:
      local_a9 = '\r';
      break;
    case 0x74:
      local_a9 = '\t';
      break;
    case 0x76:
      local_a9 = '\v';
    }
  }
switchD_00100bb0_caseD_e:
  local_c0 = pcVar5;
  check_start_new_arg(&local_48);
  splitbuf_append_byte(&local_48,(int)local_a9);
  local_c0 = local_c0 + 1;
  goto LAB_00101011;
}

// Function: parse_split_string
void parse_split_string(undefined8 param_1,int *param_2,int *param_3,long *param_4) {
  undefined8 uVar1;
  long in_FS_OFFSET;
  int local_24;
  int local_20;
  int local_1c;
  undefined8 *local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_1c = *param_3 - *param_2;
  local_18 = (undefined8 *)build_argv(param_1,local_1c,&local_24);
  *local_18 = *(undefined8 *)*param_4;
  if ((dev_debug != '\0') && (1 < local_24)) {
    if (dev_debug != '\0') {
      uVar1 = quote(param_1);
      fprintf(_stderr,"split -S:  %s\n",uVar1);
    }
    if (dev_debug != '\0') {
      uVar1 = quote(local_18[1]);
      fprintf(_stderr," into:    %s\n",uVar1);
    }
    for (local_20 = 2; local_20 < local_24; local_20 = local_20 + 1) {
      if (dev_debug != '\0') {
        uVar1 = quote(local_18[local_20]);
        fprintf(_stderr,"     &    %s\n",uVar1);
      }
    }
  }
  memcpy(local_18 + local_24,(void *)(*param_4 + (long)*param_2 * 8),(long)(local_1c + 1) * 8);
  *param_3 = local_24 + local_1c;
  *param_4 = (long)local_18;
  *param_2 = 0;
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: parse_signal_action_params
void parse_signal_action_params(long param_1,char param_2) {
  int iVar1;
  char *__s;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined4 uVar4;
  int local_40;
  char *local_30;
  
  if (param_1 == 0) {
    for (local_40 = 1; local_40 < 0x41; local_40 = local_40 + 1) {
      if (param_2 == '\0') {
        uVar4 = 4;
      }
      else {
        uVar4 = 2;
      }
      *(undefined4 *)(signals + (long)local_40 * 4) = uVar4;
    }
  }
  else {
    __s = (char *)xstrdup(param_1);
    local_30 = strtok(__s,",");
    while (local_30 != (char *)0x0) {
      iVar1 = operand2sig(local_30);
      if (iVar1 == 0) {
        uVar2 = quote(local_30);
        uVar3 = gettext("%s: invalid signal");
        error(0,0,uVar3,uVar2);
      }
      if (iVar1 < 1) {
        usage(_exit_failure);
      }
      if (param_2 == '\0') {
        uVar4 = 3;
      }
      else {
        uVar4 = 1;
      }
      *(undefined4 *)(signals + (long)iVar1 * 4) = uVar4;
      local_30 = strtok((char *)0x0,",");
    }
    free(__s);
  }
  return;
}

// Function: reset_signal_handlers
void reset_signal_handlers(void) {
  bool bVar1;
  bool bVar2;
  int iVar3;
  undefined8 uVar4;
  int *piVar5;
  char *pcVar6;
  char *pcVar7;
  long in_FS_OFFSET;
  uint local_100;
  int local_fc;
  sigaction local_d8;
  char local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  for (local_100 = 1; (int)local_100 < 0x41; local_100 = local_100 + 1) {
    if (*(int *)(signals + (long)(int)local_100 * 4) != 0) {
      if ((*(int *)(signals + (long)(int)local_100 * 4) == 2) ||
         (*(int *)(signals + (long)(int)local_100 * 4) == 4)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
      if ((*(int *)(signals + (long)(int)local_100 * 4) == 1) ||
         (*(int *)(signals + (long)(int)local_100 * 4) == 2)) {
        bVar2 = true;
      }
      else {
        bVar2 = false;
      }
      local_fc = sigaction(local_100,(sigaction *)0x0,&local_d8);
      if ((local_fc != 0) && (!bVar1)) {
        uVar4 = gettext("failed to get signal action for signal %d");
        piVar5 = __errno_location();
        error(0x7d,*piVar5,uVar4,local_100);
      }
      if (local_fc == 0) {
        local_d8.__sigaction_handler._1_7_ = 0;
        local_d8.__sigaction_handler._0_1_ = !bVar2;
        local_fc = sigaction(local_100,&local_d8,(sigaction *)0x0);
        if ((local_fc != 0) && (!bVar1)) {
          uVar4 = gettext("failed to set signal action for signal %d");
          piVar5 = __errno_location();
          error(0x7d,*piVar5,uVar4,local_100);
        }
      }
      if (dev_debug != '\0') {
        iVar3 = sig2str(local_100,local_38);
        if (iVar3 != 0) {
          snprintf(local_38,0x13,"SIG%d",(ulong)local_100);
        }
        if (dev_debug != '\0') {
          if (local_fc == 0) {
            pcVar7 = "";
          }
          else {
            pcVar7 = " (failure ignored)";
          }
          if (bVar2) {
            pcVar6 = "DEFAULT";
          }
          else {
            pcVar6 = "IGNORE";
          }
          fprintf(_stderr,"Reset signal %s (%d) to %s%s\n",local_38,(ulong)local_100,pcVar6,pcVar7);
        }
      }
    }
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: parse_block_signal_params
void parse_block_signal_params(long param_1,char param_2) {
  int __signo;
  int iVar1;
  undefined1 *puVar2;
  char *__s;
  undefined8 uVar3;
  undefined8 uVar4;
  int *piVar5;
  char *local_38;
  
  if (param_1 == 0) {
    if (param_2 == '\0') {
      puVar2 = unblock_signals;
    }
    else {
      puVar2 = block_signals;
    }
    sigfillset((sigset_t *)puVar2);
    if (param_2 == '\0') {
      puVar2 = block_signals;
    }
    else {
      puVar2 = unblock_signals;
    }
    sigemptyset((sigset_t *)puVar2);
  }
  else if (sig_mask_changed != '\x01') {
    sigemptyset((sigset_t *)block_signals);
    sigemptyset((sigset_t *)unblock_signals);
  }
  sig_mask_changed = 1;
  if (param_1 != 0) {
    __s = (char *)xstrdup(param_1);
    local_38 = strtok(__s,",");
    while (local_38 != (char *)0x0) {
      __signo = operand2sig(local_38);
      if (__signo == 0) {
        uVar3 = quote(local_38);
        uVar4 = gettext("%s: invalid signal");
        error(0,0,uVar4,uVar3);
      }
      if (__signo < 1) {
        usage(_exit_failure);
      }
      if (param_2 == '\0') {
        puVar2 = unblock_signals;
      }
      else {
        puVar2 = block_signals;
      }
      iVar1 = sigaddset((sigset_t *)puVar2,__signo);
      if (iVar1 == -1) {
        if (param_2 != '\0') {
          uVar3 = gettext("failed to block signal %d");
          piVar5 = __errno_location();
          error(0x7d,*piVar5,uVar3,__signo);
          goto LAB_001018ba;
        }
      }
      else {
LAB_001018ba:
        if (param_2 == '\0') {
          puVar2 = block_signals;
        }
        else {
          puVar2 = unblock_signals;
        }
        sigdelset((sigset_t *)puVar2,__signo);
      }
      local_38 = strtok((char *)0x0,",");
    }
    free(__s);
  }
  return;
}

// Function: set_signal_proc_mask
void set_signal_proc_mask(void) {
  int iVar1;
  undefined8 uVar2;
  int *piVar3;
  long in_FS_OFFSET;
  uint local_e4;
  char *local_d0;
  sigset_t local_b8;
  char local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  sigemptyset(&local_b8);
  iVar1 = sigprocmask(0,(sigset_t *)0x0,&local_b8);
  if (iVar1 != 0) {
    uVar2 = gettext("failed to get signal process mask");
    piVar3 = __errno_location();
    error(0x7d,*piVar3,uVar2);
  }
  for (local_e4 = 1; (int)local_e4 < 0x41; local_e4 = local_e4 + 1) {
    iVar1 = sigismember((sigset_t *)block_signals,local_e4);
    if (iVar1 == 0) {
      iVar1 = sigismember((sigset_t *)unblock_signals,local_e4);
      if (iVar1 == 0) {
        local_d0 = (char *)0x0;
      }
      else {
        sigdelset(&local_b8,local_e4);
        local_d0 = "UNBLOCK";
      }
    }
    else {
      sigaddset(&local_b8,local_e4);
      local_d0 = "BLOCK";
    }
    if ((dev_debug != '\0') && (local_d0 != (char *)0x0)) {
      iVar1 = sig2str(local_e4,local_38);
      if (iVar1 != 0) {
        snprintf(local_38,0x13,"SIG%d",(ulong)local_e4);
      }
      if (dev_debug != '\0') {
        fprintf(_stderr,"signal %s (%d) mask set to %s\n",local_38,(ulong)local_e4,local_d0);
      }
    }
  }
  iVar1 = sigprocmask(2,&local_b8,(sigset_t *)0x0);
  if (iVar1 != 0) {
    uVar2 = gettext("failed to set signal process mask");
    piVar3 = __errno_location();
    error(0x7d,*piVar3,uVar2);
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: list_signal_handling
void list_signal_handling(void) {
  int iVar1;
  undefined8 uVar2;
  int *piVar3;
  char *pcVar4;
  char *pcVar5;
  undefined *puVar6;
  long in_FS_OFFSET;
  uint local_184;
  sigset_t local_158;
  sigaction local_d8;
  char local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  sigemptyset(&local_158);
  iVar1 = sigprocmask(0,(sigset_t *)0x0,&local_158);
  if (iVar1 != 0) {
    uVar2 = gettext("failed to get signal process mask");
    piVar3 = __errno_location();
    error(0x7d,*piVar3,uVar2);
  }
  for (local_184 = 1; (int)local_184 < 0x41; local_184 = local_184 + 1) {
    iVar1 = sigaction(local_184,(sigaction *)0x0,&local_d8);
    if (iVar1 == 0) {
      if (local_d8.__sigaction_handler.sa_handler == (__sighandler_t)0x1) {
        pcVar4 = "IGNORE";
      }
      else {
        pcVar4 = "";
      }
      iVar1 = sigismember(&local_158,local_184);
      if (iVar1 == 0) {
        pcVar5 = "";
      }
      else {
        pcVar5 = "BLOCK";
      }
      if ((*pcVar4 == '\0') || (*pcVar5 == '\0')) {
        puVar6 = &DAT_00102be4;
      }
      else {
        puVar6 = &DAT_00103587;
      }
      if ((*pcVar4 != '\0') || (*pcVar5 != '\0')) {
        iVar1 = sig2str(local_184,local_38);
        if (iVar1 != 0) {
          snprintf(local_38,0x13,"SIG%d",(ulong)local_184);
        }
        fprintf(_stderr,"%-10s (%2d): %s%s%s\n",local_38,(ulong)local_184,pcVar5,puVar6,pcVar4);
      }
    }
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: initialize_signals
void initialize_signals(void) {
  int local_c;
  
  signals = xmalloc(0x104);
  for (local_c = 0; local_c < 0x41; local_c = local_c + 1) {
    *(undefined4 *)(signals + (long)local_c * 4) = 0;
  }
  return;
}

// Function: main
undefined8 main(int param_1,undefined8 *param_2) {
  long lVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  char *pcVar7;
  undefined8 uVar8;
  undefined8 *local_f8;
  int local_ec [3];
  char local_df;
  char local_de;
  char local_dd;
  int local_dc;
  int local_d8;
  int local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  int local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  int local_9c;
  undefined4 local_98;
  int local_94;
  undefined4 local_90;
  undefined4 local_8c;
  char *local_88;
  char *local_80;
  long *local_78;
  char *local_70;
  char *local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined *local_28;
  undefined8 local_20;
  
  local_df = '\0';
  local_de = '\0';
  local_88 = (char *)0x0;
  local_80 = (char *)0x0;
  local_f8 = param_2;
  local_ec[0] = param_1;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(0x7d);
  atexit((__func *)&close_stdout);
  initialize_signals();
  do {
    while( true ) {
      local_d8 = getopt_long(local_ec[0],local_f8,shortopts,longopts,0);
      if (local_d8 == -1) {
        if ((_optind < local_ec[0]) &&
           (cVar2 = streq(local_f8[_optind],&DAT_0010377f), cVar2 != '\0')) {
          local_df = '\x01';
          _optind = _optind + 1;
        }
        if (local_df == '\0') {
          unset_envvars();
        }
        else {
          if (dev_debug != '\0') {
            fwrite("cleaning environ\n",1,0x11,_stderr);
          }
          _environ = &dummy_environ_0;
        }
        while ((_optind < local_ec[0] &&
               (local_70 = strchr((char *)local_f8[_optind],0x3d), local_70 != (char *)0x0))) {
          if (dev_debug != '\0') {
            fprintf(_stderr,"setenv:   %s\n",local_f8[_optind]);
          }
          iVar3 = putenv((char *)local_f8[_optind]);
          if (iVar3 != 0) {
            *local_70 = '\0';
            uVar8 = quote(local_f8[_optind]);
            uVar5 = gettext("cannot set %s");
            piVar6 = __errno_location();
            local_bc = *piVar6;
            local_c0 = 0x7d;
            local_50 = uVar5;
            error(0x7d,local_bc,uVar5,uVar8);
          }
          _optind = _optind + 1;
        }
        local_dd = _optind < local_ec[0];
        if ((local_de != '\0') && ((bool)local_dd)) {
          local_48 = gettext("cannot specify --null (-0) with command");
          local_b8 = 0;
          local_b4 = 0;
          error(0,0,local_48);
          usage(0x7d);
        }
        if ((local_88 != (char *)0x0) && (local_dd != '\x01')) {
          local_40 = gettext("must specify command with --chdir (-C)");
          local_b0 = 0;
          local_ac = 0;
          error(0,0,local_40);
          usage(0x7d);
        }
        if ((local_80 != (char *)0x0) && (local_dd != '\x01')) {
          local_38 = gettext("must specify command with --argv0 (-a)");
          local_a8 = 0;
          local_a4 = 0;
          error(0,0,local_38);
          usage(0x7d);
        }
        if (local_dd != '\x01') {
          local_78 = _environ;
          while (*local_78 != 0) {
            if (local_de == '\0') {
              uVar8 = 10;
            }
            else {
              uVar8 = 0;
            }
            lVar1 = *local_78;
            local_78 = local_78 + 1;
            printf("%s%c",lVar1,uVar8);
          }
          return 0;
        }
        reset_signal_handlers();
        if (sig_mask_changed != '\0') {
          set_signal_proc_mask();
        }
        if (report_signal_handling != '\0') {
          list_signal_handling();
        }
        if (local_88 != (char *)0x0) {
          if (dev_debug != '\0') {
            uVar8 = quotearg_style(4,local_88);
            fprintf(_stderr,"chdir:    %s\n",uVar8);
          }
          iVar3 = chdir(local_88);
          if (iVar3 != 0) {
            uVar8 = quotearg_style(4,local_88);
            uVar5 = gettext("cannot change directory to %s");
            piVar6 = __errno_location();
            local_9c = *piVar6;
            local_a0 = 0x7d;
            local_30 = uVar5;
            error(0x7d,local_9c,uVar5,uVar8);
          }
        }
        local_68 = (char *)local_f8[_optind];
        if (local_80 != (char *)0x0) {
          if (dev_debug != '\0') {
            uVar8 = quotearg_style(4,local_80);
            fprintf(_stderr,"argv0:     %s\n",uVar8);
          }
          local_f8[_optind] = local_80;
        }
        if (dev_debug != '\0') {
          if (dev_debug != '\0') {
            fprintf(_stderr,"executing: %s\n",local_68);
          }
          for (local_dc = _optind; local_dc < local_ec[0]; local_dc = local_dc + 1) {
            if (dev_debug != '\0') {
              uVar8 = quote(local_f8[local_dc]);
              fprintf(_stderr,"   arg[%d]= %s\n",(ulong)(uint)(local_dc - _optind),uVar8);
            }
          }
        }
        execvp(local_68,(char **)(local_f8 + _optind));
        piVar6 = __errno_location();
        if (*piVar6 == 2) {
          local_d4 = 0x7f;
        }
        else {
          local_d4 = 0x7e;
        }
        uVar8 = quote(local_68);
        piVar6 = __errno_location();
        local_94 = *piVar6;
        local_98 = 0;
        local_28 = &DAT_00103886;
        error(0,local_94,&DAT_00103886,uVar8);
        if ((local_d4 == 0x7f) && (pcVar7 = strpbrk(local_68," \t\n\v\f\r"), pcVar7 != (char *)0x0))
        {
          local_20 = gettext("use -[v]S to pass options in shebang lines");
          local_90 = 0;
          local_8c = 0;
          error(0,0,local_20);
        }
                    /* WARNING: Subroutine does not return */
        exit(local_d4);
      }
      if (local_d8 < 0x84) break;
switchD_00101f4c_caseD_31:
      usage(0x7d);
    }
    if (local_d8 < 0x30) {
      if (local_d8 == 0x20) {
LAB_0010203b:
        local_58 = gettext("invalid option -- \'%c\'");
        local_c8 = 0;
        local_c4 = 0;
        error(0,0,local_58,local_d8);
        local_60 = gettext("use -[v]S to pass options in shebang lines");
        local_d0 = 0;
        local_cc = 0;
        error(0,0,local_60);
        usage(0x7d);
      }
      else {
        if ((0x20 < local_d8) || (0xd < local_d8)) goto switchD_00101f4c_caseD_31;
        if (8 < local_d8) goto LAB_0010203b;
        if (local_d8 == -0x83) goto LAB_001020df;
        if (local_d8 != -0x82) goto switchD_00101f4c_caseD_31;
      }
      usage(0);
LAB_001020df:
      uVar8 = proper_name_lite("Assaf Gordon","Assaf Gordon");
      uVar5 = proper_name_lite("David MacKenzie","David MacKenzie");
      uVar4 = proper_name_lite("Richard Mlynarik","Richard Mlynarik");
      version_etc(_stdout,&DAT_00103248,"GNU coreutils",_Version,uVar4,uVar5,uVar8,0);
                    /* WARNING: Subroutine does not return */
      exit(0);
    }
    switch(local_d8) {
    case 0x30:
      local_de = '\x01';
      break;
    default:
      goto switchD_00101f4c_caseD_31;
    case 0x43:
      local_88 = _optarg;
      break;
    case 0x53:
      parse_split_string(_optarg,&optind,local_ec,&local_f8);
      break;
    case 0x61:
      local_80 = _optarg;
      break;
    case 0x69:
      local_df = '\x01';
      break;
    case 0x75:
      append_unset_var(_optarg);
      break;
    case 0x76:
      dev_debug = '\x01';
      break;
    case 0x80:
      parse_signal_action_params(_optarg,1);
      parse_block_signal_params(_optarg,0);
      break;
    case 0x81:
      parse_signal_action_params(_optarg,0);
      break;
    case 0x82:
      parse_block_signal_params(_optarg,1);
      break;
    case 0x83:
      report_signal_handling = '\x01';
    }
  } while( true );
}

