// Function: mbs_logical_cspn
long mbs_logical_cspn(char *param_1,char *param_2) {
  int iVar1;
  size_t sVar2;
  ulong uVar3;
  long lVar4;
  ulong uVar5;
  long local_38;
  char *local_30;
  char *local_28;
  char cStack_13;
  
  local_38 = 0;
  if (*param_2 != '\0') {
    sVar2 = __ctype_get_mb_cur_max();
    local_30 = param_1;
    if (sVar2 < 2) {
      sVar2 = strcspn(param_1,param_2);
      if (param_1[sVar2] != '\0') {
        return sVar2 + 1;
      }
    }
    else {
      for (; *local_30 != '\0'; local_30 = local_30 + (uVar3 >> 0x28 & 0xff)) {
        local_38 = local_38 + 1;
        uVar3 = mcel_scanz(local_30);
        cStack_13 = (char)(uVar3 >> 0x28);
        local_28 = param_2;
        if (cStack_13 == '\x01') {
          lVar4 = mbschr(param_2,(int)*local_30);
          if (lVar4 != 0) {
            return local_38;
          }
        }
        else {
          for (; *local_28 != '\0'; local_28 = local_28 + (uVar5 >> 0x28 & 0xff)) {
            uVar5 = mcel_scanz(local_28);
            iVar1 = mcel_cmp(uVar3,uVar5);
            if (iVar1 == 0) {
              return local_38;
            }
          }
        }
      }
    }
  }
  return 0;
}

// Function: mbs_logical_substr
void mbs_logical_substr(char *param_1,ulong param_2,ulong param_3) {
  size_t sVar1;
  ulong uVar2;
  char *local_50;
  ulong local_40;
  char *local_38;
  ulong local_30;
  ulong local_28;
  
  sVar1 = __ctype_get_mb_cur_max();
  if (sVar1 < 2) {
    uVar2 = strlen(param_1);
  }
  else {
    uVar2 = mbslen(param_1);
  }
  if (uVar2 < param_2) {
    local_40 = 0;
  }
  else {
    local_40 = (uVar2 - param_2) + 1;
  }
  if (param_3 < local_40) {
    local_40 = param_3;
  }
  local_30 = 0;
  local_38 = param_1;
  if ((param_2 != 0) && (param_3 != 0xffffffffffffffff)) {
    if (sVar1 < 2) {
      local_38 = param_1 + (param_2 - 1);
      local_30 = local_40;
    }
    else {
      local_28 = 1;
      local_50 = param_1;
      while ((*local_50 != '\0' && (local_40 != 0))) {
        uVar2 = mcel_scanz(local_50);
        uVar2 = uVar2 >> 0x28 & 0xff;
        if (param_2 <= local_28) {
          if (param_2 == local_28) {
            local_38 = local_50;
          }
          local_40 = local_40 - 1;
          local_30 = local_30 + uVar2;
        }
        local_50 = local_50 + uVar2;
        local_28 = local_28 + 1;
      }
    }
  }
  ximemdup0(local_38,local_30);
  return;
}

// Function: mbs_offset_to_chars
long mbs_offset_to_chars(long param_1,ulong param_2) {
  ulong uVar1;
  long local_18;
  ulong local_10;
  
  local_18 = 0;
  for (local_10 = 0; (local_10 < param_2 && (*(char *)(local_10 + param_1) != '\0'));
      local_10 = local_10 + (uVar1 >> 0x28 & 0xff)) {
    local_18 = local_18 + 1;
    uVar1 = mcel_scanz(local_10 + param_1);
  }
  return local_18;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s EXPRESSION\n  or:  %s OPTION\n");
    printf(pcVar3,uVar2,uVar2);
    putchar_unlocked(10);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nPrint the value of EXPRESSION to standard output.  A blank line below\nseparates increasing precedence groups.  EXPRESSION may be:\n\n  ARG1 | ARG2       ARG1 if it is neither null nor 0, otherwise ARG2\n\n  ARG1 & ARG2       ARG1 if neither argument is null or 0, otherwise 0\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n  ARG1 < ARG2       ARG1 is less than ARG2\n  ARG1 <= ARG2      ARG1 is less than or equal to ARG2\n  ARG1 = ARG2       ARG1 is equal to ARG2\n  ARG1 != ARG2      ARG1 is unequal to ARG2\n  ARG1 >= ARG2      ARG1 is greater than or equal to ARG2\n  ARG1 > ARG2       ARG1 is greater than ARG2\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n  ARG1 + ARG2       arithmetic sum of ARG1 and ARG2\n  ARG1 - ARG2       arithmetic difference of ARG1 and ARG2\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n  ARG1 * ARG2       arithmetic product of ARG1 and ARG2\n  ARG1 / ARG2       arithmetic quotient of ARG1 divided by ARG2\n  ARG1 % ARG2       arithmetic remainder of ARG1 divided by ARG2\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n  STRING : REGEXP   anchored pattern match of REGEXP in STRING\n\n  match STRING REGEXP        same as STRING : REGEXP\n  substr STRING POS LENGTH   substring of STRING, POS counted from 1\n  index STRING CHARS         index in STRING where any CHARS is found, or 0\n  length STRING              length of STRING\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  + TOKEN                    interpret TOKEN as a string, even if it is a\n                               keyword like \'match\' or an operator like \'/\'\n\n  ( EXPRESSION )             value of EXPRESSION\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nBeware that many operators need to be escaped or quoted for shells.\nComparisons are arithmetic if both ARGs are numbers, else lexicographical.\nPattern matches return the string matched between \\( and \\) or null; if\n\\( and \\) are not used, they return the number of characters matched or 0.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nExit status is 0 if EXPRESSION is neither null nor 0, 1 if EXPRESSION is null\nor 0, 2 if EXPRESSION is syntactically invalid, and 3 if an error occurred.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00102774);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  char cVar1;
  byte bVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 *local_58;
  int local_4c;
  
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(3);
  atexit((__func *)&close_stdout);
  uVar3 = proper_name_lite("Paul Eggert","Paul Eggert");
  uVar4 = proper_name_lite("James Youngman","James Youngman");
  uVar5 = proper_name_lite("Mike Parker","Mike Parker");
  parse_long_options(param_1,param_2,&DAT_00102774,"GNU coreutils","9.8.77-8db63",usage,uVar5,uVar4,
                     uVar3,0);
  local_58 = param_2;
  local_4c = param_1;
  if (1 < param_1) {
    cVar1 = streq(param_2[1],&DAT_001027d3);
    if (cVar1 != '\0') {
      local_4c = param_1 + -1;
      local_58 = param_2 + 1;
    }
  }
  if (local_4c < 2) {
    uVar3 = gettext("missing operand");
    error(0,0,uVar3);
    usage(2);
  }
  args = local_58 + 1;
  uVar3 = eval(1);
  cVar1 = nomoreargs();
  if (cVar1 != '\x01') {
    uVar4 = quotearg_n_style(0,8,*args);
    uVar5 = gettext("syntax error: unexpected argument %s");
    error(2,0,uVar5,uVar4);
  }
  printv(uVar3);
  bVar2 = null(uVar3);
                    /* WARNING: Subroutine does not return */
  exit((uint)bVar2);
}

// Function: int_value
undefined4 * int_value(undefined8 param_1) {
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)xmalloc(0x18);
  *puVar1 = 0;
  mpz_init_set_ui(puVar1 + 2,param_1);
  return puVar1;
}

// Function: str_value
undefined4 * str_value(undefined8 param_1) {
  undefined4 *puVar1;
  undefined8 uVar2;
  
  puVar1 = (undefined4 *)xmalloc(0x18);
  *puVar1 = 1;
  uVar2 = xstrdup(param_1);
  *(undefined8 *)(puVar1 + 2) = uVar2;
  return puVar1;
}

// Function: freev
void freev(int *param_1) {
  if (*param_1 == 1) {
    free(*(void **)(param_1 + 2));
  }
  else {
    mpz_clear(param_1 + 2);
  }
  free(param_1);
  return;
}

// Function: printv
void printv(int *param_1) {
  if (*param_1 == 0) {
    mpz_out_str(_stdout,10,param_1 + 2);
    putchar_unlocked(10);
  }
  else {
    puts(*(char **)(param_1 + 2));
  }
  return;
}

// Function: null
bool null(int *param_1) {
  int iVar1;
  bool bVar2;
  char *local_10;
  
  if (*param_1 == 0) {
    iVar1 = mpz_sgn(param_1 + 2);
    bVar2 = iVar1 == 0;
  }
  else {
    local_10 = *(char **)(param_1 + 2);
    if (*local_10 == '\0') {
      bVar2 = true;
    }
    else {
      local_10 = local_10 + (*local_10 == '-');
      do {
        if (*local_10 != '0') {
          return false;
        }
        local_10 = local_10 + 1;
      } while (*local_10 != '\0');
      bVar2 = true;
    }
  }
  return bVar2;
}

// Function: looks_like_integer
undefined8 looks_like_integer(char *param_1) {
  char cVar1;
  char *local_10;
  
  local_10 = param_1 + (*param_1 == '-');
  do {
    cVar1 = c_isdigit((int)*local_10);
    if (cVar1 != '\x01') {
      return 0;
    }
    local_10 = local_10 + 1;
  } while (*local_10 != '\0');
  return 1;
}

// Function: tostring
void tostring(int *param_1) {
  undefined8 uVar1;
  
  if (*param_1 == 0) {
    uVar1 = mpz_get_str(0,10,param_1 + 2);
    mpz_clear(param_1 + 2);
    *(undefined8 *)(param_1 + 2) = uVar1;
    *param_1 = 1;
  }
  return;
}

// Function: toarith
undefined8 toarith(int *param_1) {
  void *__ptr;
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  
  if (*param_1 == 0) {
    uVar3 = 1;
  }
  else {
    __ptr = *(void **)(param_1 + 2);
    cVar1 = looks_like_integer(__ptr);
    if (cVar1 == '\x01') {
      iVar2 = mpz_init_set_str(param_1 + 2,__ptr,10);
      if (iVar2 != 0) {
        error(3,0x22,&DAT_0010280d,__ptr);
      }
      free(__ptr);
      *param_1 = 0;
      uVar3 = 1;
    }
    else {
      uVar3 = 0;
    }
  }
  return uVar3;
}

// Function: getsize
long getsize(undefined8 param_1) {
  int iVar1;
  long lVar2;
  
  iVar1 = mpz_sgn(param_1);
  if (iVar1 < 0) {
    lVar2 = -1;
  }
  else {
    iVar1 = mpz_fits_ulong_p(param_1);
    if ((iVar1 == 0) || (lVar2 = mpz_get_ui(param_1), lVar2 == -1)) {
      lVar2 = -2;
    }
  }
  return lVar2;
}

// Function: nextarg
byte nextarg(undefined8 param_1) {
  byte bVar1;
  
  if (*args == 0) {
    bVar1 = 0;
  }
  else {
    bVar1 = streq(*args,param_1);
    args = args + bVar1;
  }
  return bVar1;
}

// Function: nomoreargs
undefined8 nomoreargs(void) {
  return CONCAT71((int7)((ulong)*args >> 8),*args == 0);
}

// Function: require_more_args
void require_more_args(void) {
  char cVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  
  cVar1 = nomoreargs();
  if (cVar1 != '\0') {
    uVar2 = quotearg_n_style(0,8,*(undefined8 *)(args + -8));
    uVar3 = gettext("syntax error: missing argument after %s");
    error(2,0,uVar3,uVar2);
  }
  return;
}

// Function: docolon
undefined8 docolon(long param_1,long param_2) {
  int iVar1;
  size_t sVar2;
  char *pcVar3;
  long lVar4;
  undefined8 uVar5;
  int *piVar6;
  long in_FS_OFFSET;
  undefined8 local_1b0;
  re_registers local_188;
  undefined local_168 [64];
  char local_128 [264];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  tostring(param_1);
  tostring(param_2);
  local_188.num_regs = 0;
  local_188.start = (regoff_t *)0x0;
  local_188.end = (regoff_t *)0x0;
  local_168._0_8_ = (uchar *)0x0;
  local_168._8_8_ = 0;
  local_168._32_8_ = local_128;
  local_168._40_8_ = (uchar *)0x0;
  _re_syntax_options = 0x2c6;
  sVar2 = strlen(*(char **)(param_2 + 8));
  pcVar3 = re_compile_pattern(*(char **)(param_2 + 8),sVar2,(re_pattern_buffer *)local_168);
  if (pcVar3 != (char *)0x0) {
    error(2,0,&DAT_0010280d,pcVar3);
  }
  local_168[56] = local_168[56] & 0x7f;
  sVar2 = strlen(*(char **)(param_1 + 8));
  iVar1 = re_match((re_pattern_buffer *)local_168,*(char **)(param_1 + 8),(int)sVar2,0,&local_188);
  if (iVar1 < 0) {
    if (iVar1 == -1) {
      if (local_168._48_8_ == 0) {
        local_1b0 = int_value(0);
      }
      else {
        local_1b0 = str_value(&DAT_00101f34);
      }
    }
    else {
      uVar5 = gettext("error in regular expression matcher");
      if (iVar1 == -2) {
        piVar6 = __errno_location();
        iVar1 = *piVar6;
      }
      else {
        iVar1 = 0x4b;
      }
      error(3,iVar1,uVar5);
    }
  }
  else if (local_168._48_8_ == 0) {
    sVar2 = __ctype_get_mb_cur_max();
    if (sVar2 == 1) {
      lVar4 = (long)iVar1;
    }
    else {
      lVar4 = mbs_offset_to_chars(*(undefined8 *)(param_1 + 8),(long)iVar1);
    }
    local_1b0 = int_value(lVar4);
  }
  else if (local_188.end[1] < 0) {
    local_1b0 = str_value(&DAT_00101f34);
  }
  else {
    *(undefined *)((long)local_188.end[1] + *(long *)(param_1 + 8)) = 0;
    local_1b0 = str_value((long)local_188.start[1] + *(long *)(param_1 + 8));
  }
  if (local_188.num_regs != 0) {
    free(local_188.start);
    free(local_188.end);
  }
  local_168._32_8_ = (char *)0x0;
  regfree((regex_t *)local_168);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_1b0;
}

// Function: eval7
undefined8 eval7(undefined param_1) {
  char cVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  
  require_more_args();
  cVar1 = nextarg(&DAT_0010285c);
  if (cVar1 == '\0') {
    cVar1 = nextarg(&DAT_00102885);
    if (cVar1 != '\0') {
      uVar2 = gettext("syntax error: unexpected \')\'");
      error(2,0,uVar2);
    }
    uVar2 = *args;
    args = args + 1;
    uVar2 = str_value(uVar2);
  }
  else {
    uVar2 = eval(param_1);
    cVar1 = nomoreargs();
    if (cVar1 != '\0') {
      uVar3 = quotearg_n_style(0,8,args[-1]);
      uVar4 = gettext("syntax error: expecting \')\' after %s");
      error(2,0,uVar4,uVar3);
    }
    cVar1 = nextarg(&DAT_00102885);
    if (cVar1 != '\x01') {
      uVar3 = quotearg_n_style(0,8,*args);
      uVar4 = gettext("syntax error: expecting \')\' instead of %s");
      error(2,0,uVar4,uVar3);
    }
  }
  return uVar2;
}

// Function: eval6
undefined8 eval6(char param_1) {
  char cVar1;
  long lVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  long lVar5;
  long lVar6;
  void *__ptr;
  undefined8 local_50;
  
  cVar1 = nextarg(&DAT_001028cf);
  if (cVar1 == '\0') {
    cVar1 = nextarg("length");
    if (cVar1 == '\0') {
      cVar1 = nextarg("match");
      if (cVar1 == '\0') {
        cVar1 = nextarg("index");
        if (cVar1 == '\0') {
          cVar1 = nextarg("substr");
          if (cVar1 == '\0') {
            local_50 = eval7(param_1);
          }
          else {
            lVar2 = eval6(param_1);
            lVar5 = eval6(param_1);
            lVar6 = eval6(param_1);
            tostring(lVar2);
            cVar1 = toarith(lVar5);
            if ((cVar1 == '\x01') && (cVar1 = toarith(lVar6), cVar1 == '\x01')) {
              uVar3 = getsize(lVar5 + 8);
              uVar4 = getsize(lVar6 + 8);
              __ptr = (void *)mbs_logical_substr(*(undefined8 *)(lVar2 + 8),uVar3,uVar4);
              local_50 = str_value(__ptr);
              free(__ptr);
            }
            else {
              local_50 = str_value(&DAT_00101f34);
            }
            freev(lVar2);
            freev(lVar5);
            freev(lVar6);
          }
        }
        else {
          lVar2 = eval6(param_1);
          lVar5 = eval6(param_1);
          tostring(lVar2);
          tostring(lVar5);
          uVar3 = mbs_logical_cspn(*(undefined8 *)(lVar2 + 8),*(undefined8 *)(lVar5 + 8));
          local_50 = int_value(uVar3);
          freev(lVar2);
          freev(lVar5);
        }
      }
      else {
        local_50 = eval6(param_1);
        uVar3 = eval6(param_1);
        if (param_1 != '\0') {
          uVar4 = docolon(local_50,uVar3);
          freev(local_50);
          local_50 = uVar4;
        }
        freev(uVar3);
      }
    }
    else {
      lVar2 = eval6(param_1);
      tostring(lVar2);
      uVar3 = mbslen(*(undefined8 *)(lVar2 + 8));
      local_50 = int_value(uVar3);
      freev(lVar2);
    }
  }
  else {
    require_more_args();
    uVar3 = *args;
    args = args + 1;
    local_50 = str_value(uVar3);
  }
  return local_50;
}

// Function: eval5
undefined8 eval5(char param_1) {
  char cVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 local_20;
  
  local_20 = eval6(param_1);
  while( true ) {
    cVar1 = nextarg(&DAT_001028eb);
    if (cVar1 == '\0') break;
    uVar2 = eval6(param_1);
    if (param_1 != '\0') {
      uVar3 = docolon(local_20,uVar2);
      freev(local_20);
      local_20 = uVar3;
    }
    freev(uVar2);
  }
  return local_20;
}

// Function: eval4
long eval4(char param_1) {
  char cVar1;
  int iVar2;
  long lVar3;
  long lVar4;
  undefined8 uVar5;
  code *pcVar6;
  int local_3c;
  
  lVar3 = eval5(param_1);
  do {
    cVar1 = nextarg(&DAT_001028ed);
    if (cVar1 == '\0') {
      cVar1 = nextarg(&DAT_001028ef);
      if (cVar1 == '\0') {
        cVar1 = nextarg(&DAT_001028f1);
        if (cVar1 == '\0') {
          return lVar3;
        }
        local_3c = 2;
      }
      else {
        local_3c = 1;
      }
    }
    else {
      local_3c = 0;
    }
    lVar4 = eval5(param_1);
    if (param_1 != '\0') {
      cVar1 = toarith(lVar3);
      if (cVar1 == '\x01') {
        cVar1 = toarith(lVar4);
        if (cVar1 != '\x01') goto LAB_0010172e;
      }
      else {
LAB_0010172e:
        uVar5 = gettext("non-integer argument");
        error(2,0,uVar5);
      }
      if (local_3c != 0) {
        iVar2 = mpz_sgn(lVar4 + 8);
        if (iVar2 == 0) {
          uVar5 = gettext("division by zero");
          error(2,0,uVar5);
        }
      }
      if (local_3c == 0) {
        pcVar6 = mpz_mul;
      }
      else if (local_3c == 1) {
        pcVar6 = mpz_tdiv_q;
      }
      else {
        pcVar6 = mpz_tdiv_r;
      }
      (*pcVar6)(lVar3 + 8,lVar3 + 8,lVar4 + 8);
    }
    freev(lVar4);
  } while( true );
}

// Function: eval3
long eval3(char param_1) {
  bool bVar1;
  char cVar2;
  long lVar3;
  long lVar4;
  undefined8 uVar5;
  code *pcVar6;
  
  lVar3 = eval4(param_1);
  do {
    cVar2 = nextarg(&DAT_001028cf);
    if (cVar2 == '\0') {
      cVar2 = nextarg(&DAT_00102919);
      if (cVar2 == '\0') {
        return lVar3;
      }
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    lVar4 = eval4(param_1);
    if (param_1 != '\0') {
      cVar2 = toarith(lVar3);
      if (cVar2 == '\x01') {
        cVar2 = toarith(lVar4);
        if (cVar2 != '\x01') goto LAB_001018b1;
      }
      else {
LAB_001018b1:
        uVar5 = gettext("non-integer argument");
        error(2,0,uVar5);
      }
      if (bVar1) {
        pcVar6 = mpz_sub;
      }
      else {
        pcVar6 = mpz_add;
      }
      (*pcVar6)(lVar3 + 8,lVar3 + 8,lVar4 + 8);
    }
    freev(lVar4);
  } while( true );
}

// Function: eval2
long eval2(char param_1) {
  char cVar1;
  long lVar2;
  int *piVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  uint local_60;
  long local_40;
  
  local_40 = eval3(param_1);
  do {
    cVar1 = nextarg(&DAT_0010291b);
    if (cVar1 == '\0') {
      cVar1 = nextarg(&DAT_0010291d);
      if (cVar1 == '\0') {
        cVar1 = nextarg(&DAT_00102920);
        if (cVar1 == '\0') {
          cVar1 = nextarg(&DAT_00102922);
          if (cVar1 == '\0') {
            cVar1 = nextarg(&DAT_00102925);
            if (cVar1 == '\0') {
              cVar1 = nextarg(&DAT_00102928);
              if (cVar1 == '\0') {
                cVar1 = nextarg(&DAT_0010292b);
                if (cVar1 == '\0') {
                  return local_40;
                }
                local_60 = 5;
              }
              else {
                local_60 = 4;
              }
            }
            else {
              local_60 = 3;
            }
            goto LAB_00101a2c;
          }
        }
        local_60 = 2;
      }
      else {
        local_60 = 1;
      }
    }
    else {
      local_60 = 0;
    }
LAB_00101a2c:
    lVar2 = eval3(param_1);
    if (param_1 != '\0') break;
    freev(local_40);
    freev(lVar2);
    local_40 = int_value(0);
  } while( true );
  tostring(local_40);
  tostring(lVar2);
  cVar1 = looks_like_integer(*(undefined8 *)(local_40 + 8));
  if (cVar1 != '\0') {
    cVar1 = looks_like_integer(*(undefined8 *)(lVar2 + 8));
    if (cVar1 != '\0') {
      strintcmp(*(undefined8 *)(local_40 + 8),*(undefined8 *)(lVar2 + 8));
      goto LAB_00101bd2;
    }
  }
  piVar3 = __errno_location();
  *piVar3 = 0;
  strcoll(*(char **)(local_40 + 8),*(char **)(lVar2 + 8));
  piVar3 = __errno_location();
  if (*piVar3 != 0) {
    uVar4 = gettext("string comparison failed");
    piVar3 = __errno_location();
    error(0,*piVar3,uVar4);
    uVar4 = gettext("set LC_ALL=\'C\' to work around the problem");
    error(0,0,uVar4);
    uVar4 = quotearg_n_style(1,8,*(undefined8 *)(lVar2 + 8));
    uVar5 = quotearg_n_style(0,8,*(undefined8 *)(local_40 + 8));
    uVar6 = gettext("the strings compared were %s and %s");
    error(2,0,uVar6,uVar5,uVar4);
  }
LAB_00101bd2:
                    /* WARNING: Could not recover jumptable at 0x00101bf3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  lVar2 = (*(code *)(&DAT_0010299c + *(int *)(&DAT_0010299c + (ulong)local_60 * 4)))();
  return lVar2;
}

// Function: eval1
undefined8 eval1(char param_1) {
  char cVar1;
  undefined4 uVar2;
  undefined8 uVar3;
  undefined8 local_18;
  
  local_18 = eval2(param_1);
LAB_00101c93:
  do {
    cVar1 = nextarg(&DAT_001029b4);
    if (cVar1 == '\0') {
      return local_18;
    }
    if (param_1 == '\0') {
LAB_00101cca:
      uVar2 = 0;
    }
    else {
      cVar1 = null(local_18);
      if (cVar1 == '\x01') goto LAB_00101cca;
      uVar2 = 1;
    }
    uVar3 = eval2(uVar2);
    cVar1 = null(local_18);
    if (cVar1 != '\0') {
LAB_00101d00:
      freev(local_18);
      freev(uVar3);
      local_18 = int_value(0);
      goto LAB_00101c93;
    }
    cVar1 = null(uVar3);
    if (cVar1 != '\0') goto LAB_00101d00;
    freev(uVar3);
  } while( true );
}

// Function: eval
undefined8 eval(char param_1) {
  char cVar1;
  undefined4 uVar2;
  undefined8 uVar3;
  undefined8 local_18;
  
  local_18 = eval1(param_1);
  do {
    cVar1 = nextarg(&DAT_001029b6);
    if (cVar1 == '\0') {
      return local_18;
    }
    if (param_1 == '\0') {
LAB_00101d9a:
      uVar2 = 0;
    }
    else {
      cVar1 = null(local_18);
      if (cVar1 == '\0') goto LAB_00101d9a;
      uVar2 = 1;
    }
    uVar3 = eval1(uVar2);
    cVar1 = null(local_18);
    if (cVar1 == '\0') {
      freev(uVar3);
    }
    else {
      freev(local_18);
      cVar1 = null(uVar3);
      local_18 = uVar3;
      if (cVar1 != '\0') {
        freev(uVar3);
        local_18 = int_value(0);
      }
    }
  } while( true );
}

