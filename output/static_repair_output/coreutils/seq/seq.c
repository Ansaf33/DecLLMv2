// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... LAST\n  or:  %s [OPTION]... FIRST LAST\n  or:  %s [OPTION]... FIRST INCREMENT LAST\n"
                            );
    printf(pcVar3,uVar2,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Print numbers from FIRST to LAST, in steps of INCREMENT.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -f, --format=FORMAT      use printf style floating-point FORMAT\n  -s, --separator=STRING   use STRING to separate numbers (default: \\n)\n  -w, --equal-width        equalize width by padding with leading zeroes\n"
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
                            "\nIf FIRST or INCREMENT is omitted, it defaults to 1.  That is, an\nomitted INCREMENT defaults to 1 even when LAST is smaller than FIRST.\nThe sequence of numbers ends when the sum of the current number and\nINCREMENT would become greater than LAST.\nFIRST, INCREMENT, and LAST are interpreted as floating point values.\nINCREMENT is usually positive if FIRST is smaller than LAST, and\nINCREMENT is usually negative if FIRST is greater than LAST.\nINCREMENT must not be 0; none of FIRST, INCREMENT and LAST may be NaN.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "FORMAT must be suitable for printing one argument of type \'double\';\nit defaults to %.PRECf if FIRST, INCREMENT, and LAST are all fixed point\ndecimal numbers with maximum precision PREC, and to %g otherwise.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00102a58);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: scan_arg
undefined8 * scan_arg(undefined8 *param_1,char *param_2) {
  ushort *puVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  ushort **ppuVar8;
  char *pcVar9;
  char *pcVar10;
  size_t sVar11;
  long lVar12;
  ulong uVar13;
  ulong uVar14;
  long in_FS_OFFSET;
  char *local_98;
  ulong local_78;
  char *local_70;
  long local_68;
  undefined8 local_48;
  undefined2 uStack_40;
  undefined6 uStack_3e;
  char *local_38;
  int local_30;
  undefined4 uStack_2c;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  cVar2 = xstrtold(param_2,0,&local_48,&cl_strtold);
  if (cVar2 != '\x01') {
    uVar5 = quote(param_2);
    uVar6 = gettext("invalid floating point argument: %s");
    error(0,0,uVar6,uVar5);
    usage(1);
  }
  local_98 = param_2;
  if (NAN((longdouble)CONCAT28(uStack_40,local_48)) || NAN((longdouble)CONCAT28(uStack_40,local_48))
     ) {
    uVar5 = quote_n(1,param_2);
    uVar6 = quote_n(0,"not-a-number");
    uVar7 = gettext("invalid %s argument: %s");
    error(0,0,uVar7,uVar6,uVar5);
    usage(1);
    goto LAB_001005cd;
  }
  while( true ) {
    ppuVar8 = __ctype_b_loc();
    puVar1 = *ppuVar8;
    bVar3 = to_uchar((int)*local_98);
    if (((puVar1[bVar3] & 0x2000) == 0) && (*local_98 != '+')) break;
LAB_001005cd:
    local_98 = local_98 + 1;
  }
  local_38 = (char *)0x0;
  local_30 = 0x7fffffff;
  pcVar9 = strchr(local_98,0x2e);
  if ((pcVar9 == (char *)0x0) && (pcVar10 = strchr(local_98,0x70), pcVar10 == (char *)0x0)) {
    local_30 = 0;
  }
  sVar11 = strcspn(local_98,"xX");
  if ((local_98[sVar11] == '\0') &&
     ((longdouble)0 == (longdouble)0 * (longdouble)CONCAT28(uStack_40,local_48))) {
    local_78 = 0;
    local_38 = (char *)strlen(local_98);
    if (pcVar9 != (char *)0x0) {
      local_78 = strcspn(pcVar9 + 1,"eE");
      if (local_78 < 0x80000000) {
        local_30 = (int)local_78;
      }
      if (local_78 == 0) {
        iVar4 = -1;
      }
      else if ((pcVar9 == local_98) || (cVar2 = c_isdigit((int)pcVar9[-1]), cVar2 != '\x01')) {
        iVar4 = 1;
      }
      else {
        iVar4 = 0;
      }
      local_38 = local_38 + iVar4;
    }
    local_70 = strchr(local_98,0x65);
    if (local_70 == (char *)0x0) {
      local_70 = strchr(local_98,0x45);
    }
    if (local_70 != (char *)0x0) {
      lVar12 = __isoc23_strtol(local_70 + 1,0,10);
      if (lVar12 < -0x7ffffffffffffffe) {
        uVar13 = 0x8000000000000001;
      }
      else {
        uVar13 = __isoc23_strtol(local_70 + 1,0,10);
      }
      pcVar10 = local_38;
      if ((long)uVar13 < 0) {
        iVar4 = (int)uVar13;
      }
      else {
        uVar14 = uVar13;
        if ((long)local_30 <= (long)uVar13) {
          uVar14 = (long)local_30;
        }
        iVar4 = (int)uVar14;
      }
      local_30 = local_30 - iVar4;
      sVar11 = strlen(local_98);
      local_38 = local_70 + (long)(pcVar10 + (-sVar11 - (long)local_98));
      if ((long)uVar13 < 0) {
        if (pcVar9 == (char *)0x0) {
          local_38 = local_38 + 1;
        }
        else if (local_70 == pcVar9 + 1) {
          local_38 = local_38 + 1;
        }
        local_68 = -uVar13;
      }
      else {
        if (((pcVar9 != (char *)0x0) && (local_30 == 0)) && (local_78 != 0)) {
          local_38 = local_38 + -1;
        }
        if (uVar13 <= local_78) {
          local_78 = uVar13;
        }
        local_68 = uVar13 - local_78;
      }
      local_38 = local_38 + local_68;
    }
  }
  *param_1 = local_48;
  param_1[1] = CONCAT62(uStack_3e,uStack_40);
  param_1[2] = local_38;
  param_1[3] = CONCAT44(uStack_2c,local_30);
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return param_1;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: long_double_format
void * long_double_format(void *param_1,long *param_2) {
  char cVar1;
  size_t sVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  char *pcVar5;
  void *__dest;
  long lVar6;
  bool bVar7;
  size_t local_68;
  long local_60;
  long local_58;
  
  local_60 = 0;
  local_58 = 0;
  for (local_68 = 0;
      (*(char *)(local_68 + (long)param_1) != '%' ||
      (*(char *)((long)param_1 + local_68 + 1) == '%')); local_68 = local_68 + lVar6) {
    if (*(char *)(local_68 + (long)param_1) == '\0') {
      uVar3 = quote(param_1);
      uVar4 = gettext("format %s has no %% directive");
      error(1,0,uVar4,uVar3);
    }
    local_60 = local_60 + 1;
    if (*(char *)(local_68 + (long)param_1) == '%') {
      lVar6 = 2;
    }
    else {
      lVar6 = 1;
    }
  }
  sVar2 = strspn((char *)(local_68 + 1 + (long)param_1),"-+#0 \'");
  lVar6 = local_68 + 1 + sVar2;
  local_68 = strspn((char *)(lVar6 + (long)param_1),"0123456789");
  local_68 = lVar6 + local_68;
  if (*(char *)(local_68 + (long)param_1) == '.') {
    sVar2 = strspn((char *)(local_68 + 1 + (long)param_1),"0123456789");
    local_68 = local_68 + 1 + sVar2;
  }
  sVar2 = local_68;
  bVar7 = *(char *)(local_68 + (long)param_1) == 'L';
  lVar6 = local_68 + bVar7;
  if (*(char *)(lVar6 + (long)param_1) == '\0') {
    uVar3 = quote(param_1);
    uVar4 = gettext("format %s ends in %%");
    error(1,0,uVar4,uVar3);
  }
  pcVar5 = strchr("efgaEFGA",(int)*(char *)(lVar6 + (long)param_1));
  if (pcVar5 == (char *)0x0) {
    cVar1 = *(char *)(lVar6 + (long)param_1);
    uVar3 = quote(param_1);
    uVar4 = gettext("format %s has unknown %%%c directive");
    error(1,0,uVar4,uVar3,(int)cVar1);
  }
  local_68 = lVar6 + 1;
  while( true ) {
    if ((*(char *)(local_68 + (long)param_1) == '%') &&
       (*(char *)((long)param_1 + local_68 + 1) != '%')) {
      uVar3 = quote(param_1);
      uVar4 = gettext("format %s has too many %% directives");
      error(1,0,uVar4,uVar3);
    }
    if (*(char *)(local_68 + (long)param_1) == '\0') break;
    local_58 = local_58 + 1;
    if (*(char *)(local_68 + (long)param_1) == '%') {
      lVar6 = 2;
    }
    else {
      lVar6 = 1;
    }
    local_68 = local_68 + lVar6;
  }
  __dest = (void *)xmalloc(local_68 + 2);
  memcpy(__dest,param_1,sVar2);
  *(undefined *)(sVar2 + (long)__dest) = 0x4c;
  strcpy((char *)((long)__dest + sVar2 + 1),(char *)(bVar7 + sVar2 + (long)param_1));
  *param_2 = local_60;
  param_2[1] = local_58;
  return __dest;
}

// Function: print_numbers
void print_numbers(char *param_1,long param_2,long param_3) {
  bool bVar1;
  char cVar2;
  int iVar3;
  long in_FS_OFFSET;
  bool bVar4;
  longdouble lVar5;
  longdouble param_7;
  longdouble param_8;
  longdouble param_9;
  char *local_68;
  char *local_60;
  longdouble local_58;
  undefined8 local_48;
  undefined2 uStack_40;
  longdouble local_38;
  undefined8 local_28;
  undefined2 uStack_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if ((longdouble)0 <= param_8) {
    bVar1 = param_9 < param_7;
  }
  else {
    bVar1 = param_7 < param_9;
  }
  if (!bVar1) {
    local_48 = SUB108(param_7,0);
    uStack_40 = (undefined2)((unkuint10)param_7 >> 0x40);
    local_38 = (longdouble)1;
    while( true ) {
      local_28 = local_48;
      uStack_20 = uStack_40;
      iVar3 = printf(param_1);
      if (iVar3 < 0) {
        write_error();
      }
      if (bVar1) break;
      lVar5 = param_7 + param_8 * local_38;
      local_48 = SUB108(lVar5,0);
      uStack_40 = (undefined2)((unkuint10)lVar5 >> 0x40);
      if ((longdouble)0 <= param_8) {
        bVar1 = param_9 < lVar5;
      }
      else {
        bVar1 = lVar5 < param_9;
      }
      if (bVar1) {
        bVar4 = false;
        if (locale_ok != '\0') {
          setlocale(1,"C");
        }
        iVar3 = asprintf(&local_68,param_1);
        if (locale_ok != '\0') {
          setlocale(1,"");
        }
        if (iVar3 < 0) {
          xalloc_die();
        }
        local_68[iVar3 - param_3] = '\0';
        cVar2 = xstrtold(local_68 + param_2,0,&local_58,&cl_strtold);
        if ((cVar2 != '\0') && (param_9 == local_58)) {
          local_60 = (char *)0x0;
          iVar3 = asprintf(&local_60,param_1);
          if (iVar3 < 0) {
            xalloc_die();
          }
          local_60[iVar3 - param_3] = '\0';
          cVar2 = streq(local_60,local_68);
          bVar4 = cVar2 == '\0';
          free(local_60);
        }
        free(local_68);
        if (!bVar4) break;
      }
      iVar3 = fputs_unlocked(separator,_stdout);
      if (iVar3 == -1) {
        write_error();
      }
      local_38 = (longdouble)1 + local_38;
    }
    iVar3 = fputs_unlocked((char *)&terminator,_stdout);
    if (iVar3 == -1) {
      write_error();
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: get_default_format
undefined * get_default_format(void) {
  uint uVar1;
  long param_9;
  uint param_10;
  uint param_11;
  long param_12;
  int param_13;
  ulong local_20;
  ulong local_18;
  
  uVar1 = param_10;
  if ((int)param_10 <= (int)param_11) {
    uVar1 = param_11;
  }
  if ((uVar1 != 0x7fffffff) && (param_13 != 0x7fffffff)) {
    if (equal_width == '\0') {
      sprintf(format_buf_0,"%%.%dLf",(ulong)uVar1);
      return format_buf_0;
    }
    local_20 = (int)(uVar1 - param_10) + param_9;
    local_18 = (int)(uVar1 - param_13) + param_12;
    if ((param_13 != 0) && (uVar1 == 0)) {
      local_18 = local_18 - 1;
    }
    if ((param_13 == 0) && (uVar1 != 0)) {
      local_18 = local_18 + 1;
    }
    if ((param_10 == 0) && (uVar1 != 0)) {
      local_20 = local_20 + 1;
    }
    if (local_20 <= local_18) {
      local_20 = local_18;
    }
    if (local_20 < 0x80000000) {
      sprintf(format_buf_0,"%%0%d.%dLf",local_20 & 0xffffffff,(ulong)uVar1);
      return format_buf_0;
    }
  }
  return &DAT_00102b62;
}

// Function: incr_grows
undefined8 incr_grows(char *param_1,char *param_2) {
  char *local_18;
  
  local_18 = param_2;
  do {
    local_18 = local_18 + -1;
    if (*local_18 < '9') {
      *local_18 = *local_18 + '\x01';
      return 0;
    }
    *local_18 = '0';
  } while (param_1 < local_18);
  param_1[-1] = '1';
  return 1;
}

// Function: cmp
int cmp(void *param_1,void *param_2) {
  int iVar1;
  void *in_RCX;
  void *in_RDX;
  
  if (param_2 == in_RCX) {
    iVar1 = memcmp(param_1,in_RDX,(size_t)param_2);
  }
  else {
    iVar1 = (uint)((long)in_RCX < (long)param_2) - (uint)((long)param_2 < (long)in_RCX);
  }
  return iVar1;
}

// Function: trim_leading_zeros
char * trim_leading_zeros(char *param_1) {
  char *local_20;
  
  for (local_20 = param_1; *local_20 == '0'; local_20 = local_20 + 1) {
  }
  if ((*local_20 == '\0') && (local_20 != param_1)) {
    local_20 = local_20 + -1;
  }
  return local_20;
}

// Function: seq_fast
void seq_fast(undefined8 param_1,undefined8 param_2,long param_3) {
  bool bVar1;
  char cVar2;
  bool bVar3;
  byte bVar4;
  int iVar5;
  char *__s;
  char *__s_00;
  undefined *puVar6;
  long lVar7;
  long in_FS_OFFSET;
  size_t local_2080;
  void *local_2078;
  long local_2070;
  void *local_2068;
  undefined *local_2060;
  void *local_2058;
  long local_2050;
  size_t local_2048;
  size_t local_2040;
  undefined *local_2038;
  long local_2030;
  void *local_2028;
  size_t local_2020;
  undefined local_2018 [8192];
  undefined auStack_18 [8];
  undefined8 local_10;
  
  local_10 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  __s = (char *)trim_leading_zeros(param_1);
  __s_00 = (char *)trim_leading_zeros(param_2);
  local_2048 = strlen(__s);
  local_2040 = strlen(__s_00);
  if ((local_2040 == 3) && (cVar2 = memeq(__s_00,&DAT_00102b66,4), cVar2 != '\0')) {
    bVar3 = true;
  }
  else {
    bVar3 = false;
  }
  if ((long)local_2048 < (long)local_2040) {
    bVar1 = 0x1f < (long)local_2040;
  }
  else {
    bVar1 = 0x1e < (long)local_2048;
  }
  if (bVar1) {
    local_2080 = local_2040;
    if ((long)local_2040 <= (long)local_2048) {
      local_2080 = local_2048 + 1;
    }
  }
  else {
    local_2080 = 0x1f;
  }
  local_2078 = (void *)xmalloc(local_2080);
  local_2070 = (long)local_2078 + local_2080;
  local_2068 = memcpy((void *)(local_2070 - local_2048),__s,local_2048);
  local_2038 = auStack_18;
  local_2060 = local_2018;
  while ((bVar3 || (iVar5 = cmp(local_2068,(void *)(local_2070 - (long)local_2068)), iVar5 < 1))) {
    local_2058 = local_2068;
    while ((long)local_2038 - (long)local_2060 <= local_2070 - (long)local_2058) {
      memcpy(local_2060,local_2058,(long)local_2038 - (long)local_2060);
      local_2058 = (void *)((long)local_2058 + ((long)local_2038 - (long)local_2060));
      lVar7 = full_write(1,local_2018,0x2000);
      if (lVar7 != 0x2000) {
        write_error();
      }
      local_2060 = local_2018;
    }
    puVar6 = (undefined *)mempcpy(local_2060,local_2058,local_2070 - (long)local_2058);
    local_2060 = puVar6 + 1;
    *puVar6 = *separator;
    lVar7 = param_3;
    if (local_2068 == local_2078) {
      local_2028 = (void *)xpalloc(0,&local_2080,1,0xffffffffffffffff,1);
      local_2020 = local_2070 - (long)local_2068;
      local_2070 = (long)local_2028 + local_2080;
      local_2068 = memcpy((void *)(local_2070 - local_2020),local_2078,local_2020);
      free(local_2078);
      local_2078 = local_2028;
      lVar7 = param_3;
    }
    while (local_2050 = lVar7, local_2050 != 0) {
      bVar4 = incr_grows(local_2068,local_2070);
      local_2068 = (void *)((long)local_2068 - (ulong)bVar4);
      lVar7 = local_2050 + -1;
    }
  }
  local_2030 = (long)local_2060 - (long)local_2018;
  if (local_2030 != 0) {
    local_2060[-1] = 10;
    lVar7 = full_write(1,local_2018,local_2030);
    if (local_2030 != lVar7) {
      write_error();
    }
  }
                    /* WARNING: Subroutine does not return */
  exit(0);
}

// Function: all_digits_p
undefined4 all_digits_p(char *param_1) {
  char cVar1;
  size_t sVar2;
  size_t sVar3;
  
  sVar2 = strlen(param_1);
  cVar1 = c_isdigit((int)*param_1);
  if ((cVar1 != '\0') && (sVar3 = strspn(param_1,"0123456789"), sVar2 == sVar3)) {
    return 1;
  }
  return 0;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  longdouble lVar1;
  char cVar2;
  int iVar3;
  char *pcVar4;
  undefined8 uVar5;
  size_t sVar6;
  long lVar7;
  ulong uVar8;
  long in_FS_OFFSET;
  ushort in_FPUControlWord;
  undefined8 local_148;
  undefined8 local_140;
  undefined8 local_138;
  ulong local_130;
  undefined8 *local_128;
  ushort local_120;
  int local_11c;
  char local_111;
  int local_110;
  int local_10c;
  undefined4 local_108;
  undefined4 local_104;
  undefined4 local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined4 local_f4;
  undefined4 local_f0;
  undefined4 local_ec;
  char *local_e8;
  char *local_e0;
  char *local_d8;
  undefined *local_d0;
  undefined *local_c8;
  undefined8 local_c0;
  undefined8 local_b8;
  undefined8 local_b0;
  undefined8 local_a8;
  undefined8 local_a0;
  undefined8 local_98;
  undefined8 local_90;
  undefined8 local_88;
  undefined2 uStack_80;
  undefined8 local_78;
  ulong local_70;
  undefined8 local_68;
  undefined2 uStack_60;
  undefined6 uStack_5e;
  undefined8 local_58;
  ulong local_50;
  undefined8 local_48;
  undefined2 uStack_40;
  undefined6 uStack_3e;
  undefined8 local_38;
  ulong local_30;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  local_88 = SUB108((longdouble)1,0);
  uStack_80 = (undefined2)((unkuint10)(longdouble)1 >> 0x40);
  local_78 = 1;
  local_70 = local_70 & 0xffffffff00000000;
  local_68 = SUB108((longdouble)1,0);
  uStack_60 = (undefined2)((unkuint10)(longdouble)1 >> 0x40);
  local_58 = 1;
  local_50 = local_50 & 0xffffffff00000000;
  local_98 = 0;
  local_90 = 0;
  local_d8 = (char *)0x0;
  local_128 = param_2;
  local_11c = param_1;
  set_program_name(*param_2);
  pcVar4 = setlocale(6,"");
  locale_ok = pcVar4 != (char *)0x0;
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  equal_width = '\0';
  separator = "\n";
LAB_001019e0:
  while( true ) {
    if (((local_11c <= _optind) ||
        ((*(char *)local_128[_optind] == '-' &&
         ((local_110 = (int)*(char *)(local_128[_optind] + 1), local_110 == 0x2e ||
          (cVar2 = c_isdigit(local_110), cVar2 != '\0')))))) ||
       (local_110 = getopt_long(local_11c,local_128,"+f:s:w",long_options,0), local_110 == -1)) {
      local_10c = local_11c - _optind;
      if (local_10c < 1) {
        local_b8 = gettext("missing operand");
        local_108 = 0;
        local_104 = 0;
        error(0,0,local_b8);
        usage(1);
      }
      if (3 < local_10c) {
        uVar5 = quote(local_128[(long)_optind + 3]);
        local_b0 = gettext("extra operand %s");
        local_100 = 0;
        local_fc = 0;
        error(0,0,local_b0,uVar5);
        usage(1);
      }
      if (local_d8 != (char *)0x0) {
        local_d8 = (char *)long_double_format(local_d8,&local_98);
      }
      if ((local_d8 != (char *)0x0) && (equal_width != '\0')) {
        local_a8 = gettext("format string may not be specified when printing equal width strings");
        local_f8 = 0;
        local_f4 = 0;
        error(0,0,local_a8);
        usage(1);
      }
      if (local_10c == 1) {
        local_d0 = &DAT_00102c15;
      }
      else {
        local_d0 = (undefined *)local_128[_optind];
      }
      local_111 = '\0';
      if ((local_10c != 3) ||
         ((((cVar2 = all_digits_p(local_128[(long)_optind + 1]), cVar2 != '\0' &&
            (cVar2 = xstrtold(local_128[(long)_optind + 1],0,&local_68,&cl_strtold), cVar2 != '\0'))
           && ((longdouble)0 < (longdouble)CONCAT28(uStack_60,local_68))) &&
          ((longdouble)CONCAT28(uStack_60,local_68) <= _DAT_00102c40)))) {
        local_111 = '\x01';
      }
      cVar2 = all_digits_p(local_128[_optind]);
      if ((((cVar2 != '\0') &&
           ((local_10c == 1 || (cVar2 = all_digits_p(local_128[(long)_optind + 1]), cVar2 != '\0')))
           ) && ((local_10c < 3 ||
                 ((local_111 != '\0' &&
                  (cVar2 = all_digits_p(local_128[(long)_optind + 2]), cVar2 != '\0')))))) &&
         ((equal_width != '\x01' &&
          ((local_d8 == (char *)0x0 && (sVar6 = strlen(separator), sVar6 == 1)))))) {
        local_c8 = local_d0;
        local_c0 = local_128[_optind + local_10c + -1];
        lVar1 = (longdouble)CONCAT28(uStack_60,local_68);
        if (_DAT_00102c50 <= lVar1) {
          uVar8 = (long)ROUND(lVar1 - _DAT_00102c50) ^ 0x8000000000000000;
        }
        else {
          uVar8 = (ulong)ROUND(lVar1);
        }
        local_120 = in_FPUControlWord | 0xc00;
        seq_fast(local_d0,local_c0,uVar8);
      }
      lVar7 = (long)_optind;
      _optind = _optind + 1;
      scan_arg(&local_48,local_128[lVar7]);
      if (_optind < local_11c) {
        local_88 = local_48;
        uStack_80 = uStack_40;
        local_78 = local_38;
        local_70 = local_30;
        lVar7 = (long)_optind;
        _optind = _optind + 1;
        scan_arg(&local_148,local_128[lVar7]);
        local_48 = local_148;
        uStack_40 = (undefined2)local_140;
        uStack_3e = (undefined6)((ulong)local_140 >> 0x10);
        local_38 = local_138;
        local_30 = local_130;
        if (_optind < local_11c) {
          local_68 = local_148;
          local_58 = local_138;
          local_50 = local_130;
          uStack_60 = uStack_40;
          uStack_5e = uStack_3e;
          if ((longdouble)0 == (longdouble)CONCAT28(uStack_40,local_148)) {
            uVar5 = quote(local_128[(long)_optind + -1]);
            local_a0 = gettext("invalid Zero increment value: %s");
            local_f0 = 0;
            local_ec = 0;
            error(0,0,local_a0,uVar5);
            usage(1);
          }
          lVar7 = (long)_optind;
          _optind = _optind + 1;
          scan_arg(&local_148,local_128[lVar7]);
          local_48 = local_148;
          uStack_40 = (undefined2)local_140;
          uStack_3e = (undefined6)((ulong)local_140 >> 0x10);
          local_38 = local_138;
          local_30 = local_130;
        }
      }
      if (((((((int)local_70 == 0) && ((int)local_50 == 0)) && ((int)local_30 == 0)) &&
           ((((longdouble)0 == (longdouble)0 * (longdouble)CONCAT28(uStack_80,local_88) &&
             ((longdouble)0 <= (longdouble)CONCAT28(uStack_80,local_88))) &&
            (((longdouble)0 <= (longdouble)CONCAT28(uStack_40,local_48) &&
             (((longdouble)0 < (longdouble)CONCAT28(uStack_60,local_68) &&
              ((longdouble)CONCAT28(uStack_60,local_68) <= _DAT_00102c40)))))))) &&
          (equal_width != '\x01')) &&
         ((local_d8 == (char *)0x0 && (sVar6 = strlen(separator), sVar6 == 1)))) {
        cVar2 = all_digits_p(local_d0);
        if (cVar2 == '\0') {
          iVar3 = asprintf(&local_e8,"%0.Lf");
          if (iVar3 < 0) {
            xalloc_die();
          }
        }
        else {
          local_e8 = (char *)xstrdup(local_d0);
        }
        if ((longdouble)0 == (longdouble)0 * (longdouble)CONCAT28(uStack_40,local_48)) {
          iVar3 = asprintf(&local_e0,"%0.Lf");
          if (iVar3 < 0) {
            xalloc_die();
          }
        }
        else {
          local_e0 = (char *)xstrdup(&DAT_00102b66);
        }
        if ((*local_e8 != '-') && (*local_e0 != '-')) {
          lVar1 = (longdouble)CONCAT28(uStack_60,local_68);
          if (_DAT_00102c50 <= lVar1) {
            uVar8 = (long)ROUND(lVar1 - _DAT_00102c50) ^ 0x8000000000000000;
          }
          else {
            uVar8 = (ulong)ROUND(lVar1);
          }
          local_120 = in_FPUControlWord | 0xc00;
          seq_fast(local_e8,local_e0,uVar8);
        }
        free(local_e8);
        free(local_e0);
      }
      if (local_d8 == (char *)0x0) {
        local_d8 = (char *)get_default_format();
      }
      print_numbers(local_d8,local_98,local_90);
                    /* WARNING: Subroutine does not return */
      exit(0);
    }
    if (local_110 != 0x77) break;
    equal_width = '\x01';
  }
  if (local_110 < 0x78) {
    if (local_110 == 0x73) {
      separator = _optarg;
      goto LAB_001019e0;
    }
    if (local_110 < 0x74) {
      if (local_110 != 0x66) {
        if (local_110 < 0x67) {
          if (local_110 == -0x83) {
LAB_0010197b:
            uVar5 = proper_name_lite("Ulrich Drepper","Ulrich Drepper");
            version_etc(_stdout,&DAT_00102a58,"GNU coreutils",_Version,uVar5,0);
                    /* WARNING: Subroutine does not return */
            exit(0);
          }
          if (local_110 == -0x82) {
            usage(0);
            goto LAB_0010197b;
          }
        }
        goto LAB_001019d6;
      }
      local_d8 = _optarg;
      goto LAB_001019e0;
    }
  }
LAB_001019d6:
  usage(1);
  goto LAB_001019e0;
}

