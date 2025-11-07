// Function: newline_or_blank
undefined4 newline_or_blank(int param_1) {
  int iVar1;
  undefined4 uVar2;
  
  if (param_1 == 10) {
LAB_00100382:
    uVar2 = 1;
  }
  else {
    iVar1 = c32isblank(param_1);
    if (iVar1 != 0) {
      iVar1 = c32isnbspace(param_1);
      if (iVar1 == 0) goto LAB_00100382;
    }
    uVar2 = 0;
  }
  return uVar2;
}

// Function: default_scale_base
undefined8 default_scale_base(int param_1) {
  undefined8 uVar1;
  
  if (param_1 - 3U < 2) {
    uVar1 = 0x400;
  }
  else {
    uVar1 = 1000;
  }
  return uVar1;
}

// Function: valid_suffix
bool valid_suffix(char param_1) {
  char *pcVar1;
  
  pcVar1 = strchr(valid_suffixes,(int)param_1);
  return pcVar1 != (char *)0x0;
}

// Function: suffix_power
undefined8 suffix_power(undefined param_1) {
  undefined8 uVar1;
  
  switch(param_1) {
  case 0x45:
    uVar1 = 6;
    break;
  default:
    uVar1 = 0;
    break;
  case 0x47:
    uVar1 = 3;
    break;
  case 0x4b:
  case 0x6b:
    uVar1 = 1;
    break;
  case 0x4d:
    uVar1 = 2;
    break;
  case 0x50:
    uVar1 = 5;
    break;
  case 0x51:
    uVar1 = 10;
    break;
  case 0x52:
    uVar1 = 9;
    break;
  case 0x54:
    uVar1 = 4;
    break;
  case 0x59:
    uVar1 = 8;
    break;
  case 0x5a:
    uVar1 = 7;
  }
  return uVar1;
}

// Function: suffix_power_char
char * suffix_power_char(undefined4 param_1) {
  char *pcVar1;
  
  switch(param_1) {
  case 0:
    pcVar1 = "";
    break;
  case 1:
    pcVar1 = "K";
    break;
  case 2:
    pcVar1 = "M";
    break;
  case 3:
    pcVar1 = "G";
    break;
  case 4:
    pcVar1 = "T";
    break;
  case 5:
    pcVar1 = "P";
    break;
  case 6:
    pcVar1 = "E";
    break;
  case 7:
    pcVar1 = "Z";
    break;
  case 8:
    pcVar1 = "Y";
    break;
  case 9:
    pcVar1 = "R";
    break;
  case 10:
    pcVar1 = "Q";
    break;
  default:
    pcVar1 = "(error)";
  }
  return pcVar1;
}

// Function: powerld
void powerld(int param_1) {
  int local_1c;
  
  local_1c = param_1;
  if (param_1 != 0) {
    do {
      local_1c = local_1c + -1;
    } while (local_1c != 0);
  }
  return;
}

// Function: absld
void absld(void) {
  return;
}

// Function: expld
void expld(int param_1,int *param_2) {
  longdouble in_ST0;
  longdouble in_ST1;
  longdouble in_ST2;
  longdouble in_ST3;
  longdouble in_ST4;
  longdouble in_ST5;
  longdouble lVar1;
  undefined8 param_7;
  undefined2 in_stack_00000010;
  int local_c;
  
  local_c = 0;
  if ((_DAT_00105b20 <= (longdouble)CONCAT28(in_stack_00000010,param_7)) &&
     ((longdouble)CONCAT28(in_stack_00000010,param_7) <= _DAT_00105b30)) {
    while (lVar1 = in_ST5, absld(), (longdouble)param_1 <= in_ST0) {
      local_c = local_c + 1;
      in_ST0 = in_ST1;
      in_ST1 = in_ST2;
      in_ST2 = in_ST3;
      in_ST3 = in_ST4;
      in_ST4 = in_ST5;
      in_ST5 = lVar1;
    }
  }
  if (param_2 != (int *)0x0) {
    *param_2 = local_c;
  }
  return;
}

// Function: simple_round_ceiling
long simple_round_ceiling(void) {
  longdouble param_7;
  long local_10;
  
  local_10 = (long)ROUND(param_7);
  if ((longdouble)local_10 < param_7) {
    local_10 = local_10 + 1;
  }
  return local_10;
}

// Function: simple_round_floor
long simple_round_floor(void) {
  long lVar1;
  
  lVar1 = simple_round_ceiling();
  return -lVar1;
}

// Function: simple_round_from_zero
void simple_round_from_zero(void) {
  undefined8 param_7;
  undefined2 in_stack_00000010;
  
  if ((longdouble)0 <= (longdouble)CONCAT28(in_stack_00000010,param_7)) {
    simple_round_ceiling();
  }
  else {
    simple_round_floor();
  }
  return;
}

// Function: simple_round_to_zero
long simple_round_to_zero(void) {
  longdouble param_7;
  
  return (long)ROUND(param_7);
}

// Function: simple_round_nearest
long simple_round_nearest(void) {
  longdouble param_7;
  long local_18;
  
  if ((longdouble)0 <= param_7) {
    param_7 = _DAT_00105b40 + param_7;
  }
  else {
    param_7 = param_7 - _DAT_00105b40;
  }
  local_18 = (long)ROUND(param_7);
  return local_18;
}

// Function: simple_round
void simple_round(undefined4 param_1) {
  switch(param_1) {
  case 0:
    simple_round_ceiling();
    break;
  case 1:
    simple_round_floor();
    break;
  case 2:
    simple_round_from_zero();
    break;
  case 3:
    simple_round_to_zero();
    break;
  case 4:
    simple_round_nearest();
  }
  return;
}

// Function: simple_strtod_int
undefined4 simple_strtod_int(char *param_1,char **param_2,unkbyte10 *param_3,char *param_4) {
  bool bVar1;
  char cVar2;
  int iVar3;
  longdouble lVar4;
  undefined8 local_58;
  undefined2 uStack_50;
  char *local_30;
  undefined4 local_24;
  int local_20;
  undefined8 local_18;
  undefined2 uStack_10;
  
  local_24 = 0;
  local_18 = SUB108((longdouble)0,0);
  uStack_10 = (undefined2)((unkuint10)(longdouble)0 >> 0x40);
  local_20 = 0;
  bVar1 = false;
  if (*param_1 == '-') {
    local_30 = param_1 + 1;
    *param_4 = '\x01';
  }
  else {
    *param_4 = '\0';
    local_30 = param_1;
  }
  *param_2 = local_30;
  while( true ) {
    cVar2 = c_isdigit((int)**param_2);
    if (cVar2 == '\0') {
      if ((bVar1) ||
         (iVar3 = strncmp(*param_2,decimal_point,(long)decimal_point_length), iVar3 == 0)) {
        if (*param_4 != '\0') {
          local_58 = SUB108(-(longdouble)CONCAT28(uStack_10,local_18),0);
          uStack_50 = (undefined2)((unkuint10)-(longdouble)CONCAT28(uStack_10,local_18) >> 0x40);
          uStack_10 = uStack_50;
          local_18 = local_58;
        }
        if (param_3 != (unkbyte10 *)0x0) {
          *param_3 = CONCAT28(uStack_10,local_18);
        }
      }
      else {
        local_24 = 3;
      }
      return local_24;
    }
    bVar1 = true;
    if (((longdouble)0 != (longdouble)CONCAT28(uStack_10,local_18)) || (**param_2 + -0x30 != 0)) {
      local_20 = local_20 + 1;
    }
    if (0x12 < local_20) {
      local_24 = 1;
    }
    if (0x21 < local_20) break;
    lVar4 = _DAT_00105b60 * (longdouble)CONCAT28(uStack_10,local_18) +
            (longdouble)(**param_2 + -0x30);
    local_18 = SUB108(lVar4,0);
    uStack_10 = (undefined2)((unkuint10)lVar4 >> 0x40);
    *param_2 = *param_2 + 1;
    if (((0 < thousands_sep_length) &&
        (iVar3 = strncmp(*param_2,thousands_sep,(long)thousands_sep_length), iVar3 == 0)) &&
       (cVar2 = c_isdigit((int)(*param_2)[thousands_sep_length]), cVar2 != '\0')) {
      *param_2 = *param_2 + thousands_sep_length;
    }
  }
  return 2;
}

// Function: simple_strtod_float
int simple_strtod_float(undefined8 param_1,char **param_2,longdouble *param_3,ulong *param_4) {
  longdouble lVar1;
  int iVar2;
  int iVar3;
  long in_FS_OFFSET;
  longdouble in_ST0;
  char local_42;
  char local_41;
  int local_40;
  int local_3c;
  char *local_38;
  ulong local_30;
  longdouble local_28;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_40 = 0;
  if (param_4 != (ulong *)0x0) {
    *param_4 = 0;
  }
  local_40 = simple_strtod_int(param_1,param_2,param_3,&local_42);
  if (((local_40 == 0) || (iVar3 = local_40, local_40 == 1)) &&
     (iVar2 = strncmp(*param_2,decimal_point,(long)decimal_point_length), iVar3 = local_40,
     iVar2 == 0)) {
    local_28 = (longdouble)0;
    *param_2 = *param_2 + decimal_point_length;
    local_3c = simple_strtod_int(*param_2,&local_38,&local_28,&local_41);
    lVar1 = local_28;
    if ((local_3c == 0) || (iVar3 = local_3c, local_3c == 1)) {
      if (local_3c == 1) {
        local_40 = local_3c;
      }
      if (local_41 == '\0') {
        local_30 = (long)local_38 - (long)*param_2;
        powerld(local_30 & 0xffffffff);
        local_28 = lVar1 / in_ST0;
        if (param_3 != (longdouble *)0x0) {
          if (local_42 == '\0') {
            *param_3 = local_28 + *param_3;
          }
          else {
            *param_3 = *param_3 - local_28;
          }
        }
        if (param_4 != (ulong *)0x0) {
          *param_4 = local_30;
        }
        *param_2 = local_38;
        iVar3 = local_40;
      }
      else {
        iVar3 = 3;
      }
    }
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return iVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: simple_strtod_human
int simple_strtod_human(undefined8 param_1,char **param_2,longdouble *param_3,ulong *param_4,
                       int param_5) {
  bool bVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  size_t __n;
  ulong uVar7;
  char *pcVar8;
  longdouble in_ST0;
  uint local_44;
  uint local_40;
  
  local_44 = 0;
  local_40 = default_scale_base(param_5);
  if (dev_debug != '\0') {
    uVar5 = quote_n(1,decimal_point);
    uVar6 = quote_n(0,param_1);
    fprintf(_stderr,
            "simple_strtod_human:\n  input string: %s\n  locale decimal-point: %s\n  MAX_UNSCALED_DIGITS: %d\n"
            ,uVar6,uVar5,0x12);
  }
  iVar3 = simple_strtod_float(param_1,param_2,param_3,param_4);
  if ((iVar3 != 0) && (iVar3 != 1)) {
    return iVar3;
  }
  if (dev_debug != '\0') {
    fprintf(_stderr,"  parsed numeric value: %Lf\n  input precision = %d\n",*param_4 & 0xffffffff);
  }
  if (**param_2 != '\0') {
    bVar1 = false;
    if (unit_separator != (char *)0x0) {
      __n = strlen(unit_separator);
      iVar4 = strncmp(*param_2,unit_separator,__n);
      if (iVar4 == 0) {
        bVar1 = true;
        *param_2 = *param_2 + __n;
      }
    }
    if (!bVar1) {
      uVar7 = mcel_scanz(*param_2);
      iVar4 = c32isblank(uVar7 & 0xffffffff);
      if ((iVar4 != 0) || (iVar4 = c32isnbspace(uVar7 & 0xffffffff), iVar4 != 0)) {
        *param_2 = *param_2 + (uVar7 >> 0x28 & 0xff);
      }
    }
    if (**param_2 != '\0') {
      cVar2 = valid_suffix((int)**param_2);
      if (cVar2 == '\x01') {
        if (param_5 == 0) {
          return 4;
        }
        local_44 = suffix_power((int)**param_2);
        *param_2 = *param_2 + 1;
        if ((param_5 == 1) && (**param_2 == 'i')) {
          local_40 = 0x400;
          *param_2 = *param_2 + 1;
          if (dev_debug != '\0') {
            fprintf(_stderr,"  Auto-scaling, found \'i\', switching to base %d\n",0x400);
          }
        }
        else if (param_5 == 4) {
          if (**param_2 != 'i') {
            return 6;
          }
          *param_2 = *param_2 + 1;
        }
        *param_4 = 0;
        pcVar8 = (char *)skip_str_matching(*param_2,newline_or_blank,1);
        *param_2 = pcVar8;
      }
      else {
        pcVar8 = (char *)skip_str_matching(*param_2,newline_or_blank,1);
        *param_2 = pcVar8;
        if (**param_2 != '\0') {
          return 5;
        }
      }
    }
  }
  powerld(local_44);
  if (dev_debug != '\0') {
    fprintf(_stderr,"  suffix power=%d^%d = %Lf\n",(ulong)local_40,(ulong)local_44);
  }
  *param_3 = in_ST0 * *param_3;
  if (dev_debug != '\0') {
    fprintf(_stderr,"  returning value: %Lf (%LG)\n");
  }
  return iVar3;
}

// Function: simple_strtod_fatal
void simple_strtod_fatal(undefined4 param_1,undefined8 param_2) {
  undefined4 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  char *local_28;
  
  uVar1 = conv_exit_code;
  local_28 = (char *)0x0;
  switch(param_1) {
  case 2:
    local_28 = "value too large to be converted: %s";
    break;
  case 3:
    local_28 = "invalid number: %s";
    break;
  case 4:
    local_28 = "rejecting suffix in input: %s (consider using --from)";
    break;
  case 5:
    local_28 = "invalid suffix in input: %s";
    break;
  case 6:
    local_28 = "missing \'i\' suffix in input: %s (e.g Ki/Mi/Gi)";
  }
  if (inval_style != 3) {
    uVar2 = quote(param_2);
    uVar3 = gettext(local_28);
    error(uVar1,0,uVar3,uVar2);
  }
  return;
}

// Function: double_to_human
void double_to_human(uint param_1,char *param_2,size_t param_3,int param_4,int param_5,
                    undefined4 param_6,undefined8 param_7,undefined8 param_8) {
  int iVar1;
  char *__format;
  undefined *puVar2;
  long lVar3;
  undefined *puVar4;
  undefined *puVar5;
  long in_FS_OFFSET;
  longdouble in_ST1;
  longdouble in_ST2;
  longdouble in_ST3;
  longdouble in_ST4;
  longdouble in_ST5;
  longdouble in_ST6;
  uint local_68;
  undefined4 local_64;
  uint local_60;
  uint local_5c;
  undefined8 *local_58;
  double local_50;
  char local_48;
  undefined local_47;
  undefined8 local_46 [4];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_58 = (undefined8 *)&local_47;
  local_48 = '%';
  if (param_5 != 0) {
    local_58 = local_46;
    local_47 = 0x27;
  }
  if (zero_padding_width != 0) {
    iVar1 = sprintf((char *)local_58,"0%d",(ulong)zero_padding_width);
    local_58 = (undefined8 *)((long)local_58 + (long)iVar1);
  }
  if (dev_debug != '\0') {
    fwrite("double_to_human:\n",1,0x11,_stderr);
  }
  if (param_4 == 0) {
    powerld(param_1);
    simple_round(param_6);
    powerld(param_1);
    if (dev_debug != '\0') {
      if (param_5 == 0) {
        __format = "  no scaling, returning value: %.*Lf\n";
      }
      else {
        __format = "  no scaling, returning (grouped) value: %\'.*Lf\n";
      }
      fprintf(_stderr,__format,(ulong)param_1);
    }
    *(undefined4 *)local_58 = 0x664c2a2e;
    *(undefined4 *)((long)local_58 + 3) = 0x732566;
    puVar2 = suffix;
    if (suffix == (undefined *)0x0) {
      puVar2 = &DAT_00103e04;
    }
    snprintf(param_2,param_3,&local_48,(ulong)param_1,puVar2);
  }
  else {
    iVar1 = default_scale_base(param_4);
    local_50 = (double)iVar1;
    local_68 = 0;
    expld((int)local_50,&local_68);
    if (dev_debug != '\0') {
      fprintf(_stderr,"  scaled value to %Lf * %0.f ^ %d\n",local_50,(ulong)local_68);
    }
    local_64 = 0;
    if (user_precision == -1) {
      in_ST2 = in_ST3;
      in_ST3 = in_ST4;
      in_ST4 = in_ST5;
      in_ST5 = in_ST6;
      absld();
      if (in_ST1 < _DAT_00105b60) {
        local_64 = 1;
      }
    }
    else {
      lVar3 = user_precision;
      if ((int)(local_68 * 3) <= user_precision) {
        lVar3 = (long)(int)(local_68 * 3);
      }
      local_64 = (undefined4)lVar3;
    }
    powerld(local_64);
    simple_round(param_6);
    powerld(local_64);
    in_ST2 = in_ST2 / in_ST3;
    param_7 = SUB108(in_ST2,0);
    param_8._0_2_ = (undefined2)((unkuint10)in_ST2 >> 0x40);
    absld();
    if ((longdouble)local_50 <= in_ST4) {
      param_7 = SUB108(in_ST2 / (longdouble)local_50,0);
      param_8._0_2_ = (undefined2)((unkuint10)(in_ST2 / (longdouble)local_50) >> 0x40);
      local_68 = local_68 + 1;
    }
    if ((((longdouble)0 == (longdouble)CONCAT28((undefined2)param_8,param_7)) ||
        (absld(), _DAT_00105b60 <= in_ST5)) || ((int)local_68 < 1)) {
      local_60 = 0;
    }
    else {
      local_60 = 1;
    }
    if (dev_debug != '\0') {
      fprintf(_stderr,"  after rounding, value=%Lf * %0.f ^ %d\n",local_50,(ulong)local_68);
    }
    *local_58 = 0x73257325664c2a2e;
    *(undefined8 *)((long)local_58 + 5) = 0x73257325732573;
    local_5c = local_60;
    if (user_precision != -1) {
      local_5c = (uint)user_precision;
    }
    puVar2 = suffix;
    if (suffix == (undefined *)0x0) {
      puVar2 = &DAT_00103e04;
    }
    if ((param_4 == 4) && (0 < (int)local_68)) {
      iVar1 = 0;
    }
    else {
      iVar1 = 1;
    }
    if ((local_68 == 1) && (param_4 == 2)) {
      puVar4 = &DAT_00104353;
    }
    else {
      puVar4 = (undefined *)suffix_power_char(local_68);
    }
    if (((int)local_68 < 1) || (puVar5 = unit_separator, unit_separator == (undefined *)0x0)) {
      puVar5 = &DAT_00103e04;
    }
    snprintf(param_2,param_3,&local_48,(ulong)local_5c,puVar5,puVar4,param_7,
             CONCAT62(param_8._2_6_,(undefined2)param_8),&DAT_00104351 + iVar1,(short)puVar2);
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: unit_to_umax
long unit_to_umax(char *param_1) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  char *local_58;
  long local_50;
  char *local_48;
  char *local_40;
  undefined1 *local_38;
  size_t local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_40 = (char *)0x0;
  local_48 = param_1;
  local_30 = strlen(param_1);
  local_58 = (char *)0x0;
  local_38 = valid_suffixes;
  if (local_30 == 0) goto LAB_00101616;
  cVar1 = c_isdigit((int)param_1[local_30 - 1]);
  if (cVar1 == '\x01') goto LAB_00101616;
  local_40 = (char *)xmalloc(local_30 + 2);
  local_58 = local_40 + (local_30 - 1);
  memcpy(local_40,param_1,local_30);
  if ((*local_58 == 'i') && (1 < local_30)) {
    cVar1 = c_isdigit((int)local_58[-1]);
    if (cVar1 == '\x01') goto LAB_001015dd;
    *local_58 = '\0';
  }
  else {
LAB_001015dd:
    local_58[1] = 'B';
    local_58 = local_58 + 2;
    *local_58 = '\0';
    local_38 = zero_and_valid_suffixes;
  }
  local_48 = local_40;
LAB_00101616:
  iVar2 = xstrtoumax(local_48,&local_58,10,&local_50,local_38);
  if (((iVar2 != 0) || (*local_58 != '\0')) || (local_50 == 0)) {
    free(local_40);
    uVar3 = quote(param_1);
    local_28 = gettext("invalid unit size: %s");
    error(1,0,local_28,uVar3);
  }
  free(local_40);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_50;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [NUMBER]...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Reformat NUMBER(s), or the numbers from standard input if none are specified.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --debug          print warnings about invalid input\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -d, --delimiter=X    use X instead of whitespace for field delimiter\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --field=FIELDS   replace the numbers in these input fields (default=1);\n                         see FIELDS below\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --format=FORMAT  use printf style floating-point FORMAT;\n                         see FORMAT below for details\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --from=UNIT      auto-scale input numbers to UNITs; default is \'none\';\n                         see UNIT below\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --from-unit=N    specify the input unit size (instead of the default 1)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --grouping       use locale-defined grouping of digits, e.g. 1,000,000\n                         (which means it has no effect in the C/POSIX locale)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --header[=N]     print (without converting) the first N header lines;\n                         N defaults to 1 if not specified\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --invalid=MODE   failure mode for invalid numbers: MODE can be:\n                         abort (default), fail, warn, ignore\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --padding=N      pad the output to N characters; positive N will\n                         right-align; negative N will left-align;\n                         padding is ignored if the output is wider than N;\n                         the default is to automatically pad if a whitespace\n                         is found\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --round=METHOD   use METHOD for rounding when scaling; METHOD can be:\n                         up, down, from-zero (default), towards-zero, nearest\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --suffix=SUFFIX  add SUFFIX to output numbers, and accept optional\n                         SUFFIX in input numbers\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --unit-separator=SEP  insert SEP between number and unit on output,\n                         and accept optional SEP in input numbers\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --to=UNIT        auto-scale output numbers to UNITs; see UNIT below\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --to-unit=N      the output unit size (instead of the default 1)\n"
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
    pcVar3 = (char *)gettext("\nUNIT options:\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  none       no auto-scaling is done; suffixes will trigger an error\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  auto       accept optional single/two letter suffix:\n               1K = 1000, 1k = 1000,\n               1Ki = 1024,\n               1M = 1000000,\n               1Mi = 1048576,\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  si         accept optional single letter suffix:\n               1k = 1000, 1K = 1000,\n               1M = 1000000,\n               ...\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  iec        accept optional single letter suffix:\n               1K = 1024, 1k = 1024,\n               1M = 1048576,\n               ...\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  iec-i      accept optional two-letter suffix:\n               1Ki = 1024, 1ki = 1024,\n               1Mi = 1048576,\n               ...\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nFIELDS supports cut(1) style field ranges:\n  N    N\'th field, counted from 1\n  N-   from N\'th field, to end of line\n  N-M  from N\'th to M\'th field (inclusive)\n  -M   from first to M\'th field (inclusive)\n  -    all fields\nMultiple fields/ranges can be separated with commas\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nFORMAT must be suitable for printing one floating-point argument \'%f\'.\nOptional quote (%\'f) will enable --grouping (if supported by current locale).\nOptional width value (%10f) will pad output. Optional zero (%010f) width\nwill zero pad the number. Optional negative values (%-10f) will left align.\nOptional precision (%.1f) will override the input determined precision.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    uVar2 = _program_name;
    pcVar3 = (char *)gettext(
                            "\nExit status is 0 if all input numbers were successfully converted.\nBy default, %s will stop at the first conversion error with exit status 2.\nWith --invalid=\'fail\' a warning is printed for each conversion error\nand the exit status is 2.  With --invalid=\'warn\' each conversion error is\ndiagnosed, but the exit status is 0.  With --invalid=\'ignore\' conversion\nerrors are not diagnosed and the exit status is 0.\n"
                            );
    printf(pcVar3,uVar2);
    uVar2 = _program_name;
    pcVar3 = (char *)gettext(
                            "\nExamples:\n  $ %s --to=si 1000\n            -> \"1.0k\"\n  $ %s --to=iec 2048\n           -> \"2.0K\"\n  $ %s --to=iec-i 4096\n           -> \"4.0Ki\"\n  $ echo 1K | %s --from=si\n           -> \"1000\"\n  $ echo 1K | %s --from=iec\n           -> \"1024\"\n  $ df -B1 | %s --header --field 2-4 --to=si\n  $ ls -l  | %s --header --field 5 --to=iec\n  $ ls -lh | %s --header --field 5 --from=iec --padding=10\n  $ ls -lh | %s --header --field 5 --from=iec --format %%10f\n"
                            );
    printf(pcVar3,uVar2,uVar2,uVar2,uVar2,uVar2,uVar2,uVar2,uVar2,uVar2);
    emit_ancillary_info("numfmt");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: parse_format_string
void parse_format_string(long param_1) {
  bool bVar1;
  char cVar2;
  undefined8 uVar3;
  int *piVar4;
  long lVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  undefined *puVar8;
  long in_FS_OFFSET;
  long local_90;
  long local_88;
  long local_80;
  size_t local_78;
  long local_70;
  long local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  local_80 = 0;
  local_90 = 0;
  bVar1 = false;
  for (local_88 = 0;
      (*(char *)(local_88 + param_1) != '%' || (*(char *)(param_1 + local_88 + 1) == '%'));
      local_88 = local_88 + lVar5) {
    if (*(char *)(local_88 + param_1) == '\0') {
      uVar3 = quote(param_1);
      local_60 = gettext("format %s has no %% directive");
      error(1,0,local_60,uVar3);
    }
    local_80 = local_80 + 1;
    if (*(char *)(local_88 + param_1) == '%') {
      lVar5 = 2;
    }
    else {
      lVar5 = 1;
    }
  }
  local_88 = local_88 + 1;
  do {
    while( true ) {
      while( true ) {
        local_78 = strspn((char *)(local_88 + param_1)," ");
        local_88 = local_88 + local_78;
        if (*(char *)(local_88 + param_1) != '\'') break;
        grouping = 1;
        local_88 = local_88 + 1;
      }
      if (*(char *)(local_88 + param_1) != '0') break;
      bVar1 = true;
      local_88 = local_88 + 1;
    }
  } while (local_78 != 0);
  local_70 = __isoc23_strtoimax(param_1 + local_88,&local_90,10);
  if (local_70 != 0) {
    if (((debug != '\0') && (padding_width != 0)) && ((!bVar1 || (local_70 < 1)))) {
      local_58 = gettext("--format padding overriding --padding");
      error(0,0,local_58);
    }
    if (local_70 < 0) {
      padding_width = local_70;
    }
    else if (bVar1) {
      lVar5 = local_70;
      if (0x7fffffff < local_70) {
        lVar5 = 0x7fffffff;
      }
      zero_padding_width = (undefined4)lVar5;
    }
    else {
      padding_width = local_70;
    }
  }
  local_88 = local_90 - param_1;
  if (*(char *)(local_88 + param_1) == '\0') {
    uVar3 = quote(param_1);
    local_50 = gettext("format %s ends in %%");
    error(1,0,local_50,uVar3);
  }
  if (*(char *)(local_88 + param_1) == '.') {
    local_88 = local_88 + 1;
    piVar4 = __errno_location();
    *piVar4 = 0;
    user_precision = __isoc23_strtol(param_1 + local_88,&local_90,10);
    piVar4 = __errno_location();
    if ((((*piVar4 == 0x22) || (user_precision < 0)) ||
        (cVar2 = c_isblank((int)*(char *)(local_88 + param_1)), cVar2 != '\0')) ||
       (*(char *)(local_88 + param_1) == '+')) {
      uVar3 = quote(param_1);
      local_48 = gettext("invalid precision in format %s");
      error(1,0,local_48,uVar3);
    }
    local_88 = local_90 - param_1;
  }
  if (*(char *)(local_88 + param_1) != 'f') {
    uVar3 = quote(param_1);
    local_40 = gettext("invalid format %s, directive must be %%[0][\'][-][N][.][N]f");
    error(1,0,local_40,uVar3);
  }
  local_88 = local_88 + 1;
  local_68 = local_88;
  for (; *(char *)(local_88 + param_1) != '\0'; local_88 = local_88 + lVar5) {
    if ((*(char *)(local_88 + param_1) == '%') && (*(char *)(param_1 + local_88 + 1) != '%')) {
      uVar3 = quote(param_1);
      local_38 = gettext("format %s has too many %% directives");
      error(1,0,local_38,uVar3);
    }
    if (*(char *)(local_88 + param_1) == '%') {
      lVar5 = 2;
    }
    else {
      lVar5 = 1;
    }
  }
  if (local_80 != 0) {
    format_str_prefix = (undefined *)ximemdup0(param_1,local_80);
  }
  if (*(char *)(local_68 + param_1) != '\0') {
    format_str_suffix = (undefined *)xstrdup(local_68 + param_1);
  }
  if (dev_debug != '\0') {
    puVar8 = format_str_suffix;
    if (format_str_suffix == (undefined *)0x0) {
      puVar8 = &DAT_00103e04;
    }
    uVar3 = quote_n(2,puVar8);
    puVar8 = format_str_prefix;
    if (format_str_prefix == (undefined *)0x0) {
      puVar8 = &DAT_00103e04;
    }
    uVar6 = quote_n(1,puVar8);
    lVar5 = padding_width;
    if (grouping == 0) {
      puVar8 = &DAT_001055e9;
    }
    else {
      puVar8 = &DAT_001055e5;
    }
    uVar7 = quote_n(0,param_1);
    fprintf(_stderr,
            "format String:\n  input: %s\n  grouping: %s\n  padding width: %jd\n  prefix: %s\n  suffix: %s\n"
            ,uVar7,puVar8,lVar5,uVar6,uVar3);
  }
  if (local_30 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: parse_human_number
int parse_human_number(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  undefined4 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  int local_40;
  char *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_30 = (char *)0x0;
  local_40 = simple_strtod_human(param_1,&local_30,param_2,param_3,scale_from);
  uVar1 = conv_exit_code;
  if ((local_40 == 0) || (local_40 == 1)) {
    if ((local_30 != (char *)0x0) && (*local_30 != '\0')) {
      if (inval_style != 3) {
        uVar2 = quote_n(1,local_30);
        uVar3 = quote_n(0,param_1);
        local_28 = gettext("invalid suffix in input %s: %s");
        error(uVar1,0,local_28,uVar3,uVar2);
      }
      local_40 = 5;
    }
  }
  else {
    simple_strtod_fatal(local_40,param_1);
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_40;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: prepare_padded_number
undefined8 prepare_padded_number(ulong param_1,long *param_2) {
  undefined8 uVar1;
  long in_FS_OFFSET;
  undefined8 param_7;
  undefined8 param_8;
  int local_80;
  undefined4 local_7c;
  undefined4 local_78;
  int local_74;
  int local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  long local_48;
  long local_40;
  ulong local_38;
  undefined8 local_30;
  undefined8 local_28;
  undefined8 local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_38 = user_precision;
  if (user_precision == 0xffffffffffffffff) {
    local_38 = param_1;
  }
  expld(10,&local_80);
  if ((scale_to == 0) && (0x12 < local_38 + (long)local_80)) {
    if (inval_style != 3) {
      if (local_38 == 0) {
        local_7c = conv_exit_code;
        local_28 = gettext("value too large to be printed: \'%Lg\' (consider using --to)");
        local_60 = local_7c;
        local_5c = 0;
        error(local_7c,0,local_28);
      }
      else {
        local_78 = conv_exit_code;
        local_30 = gettext(
                          "value/precision too large to be printed: \'%Lg/%zu\' (consider using --to)"
                          );
        local_68 = local_78;
        local_64 = 0;
        error(local_78,0,local_30,local_38);
      }
    }
    uVar1 = 0;
  }
  else if (local_80 < 0x21) {
    while( true ) {
      local_74 = double_to_human(local_38 & 0xffffffff,padding_buffer,padding_buffer_size,scale_to,
                                 grouping,round_style,param_7,param_8);
      if ((local_74 < 0) ||
         (local_48 = (long)local_74 - (padding_buffer_size + -1),
         SBORROW8((long)local_74,padding_buffer_size + -1))) {
        local_18 = gettext("failed to prepare value \'%Lf\' for printing");
        local_50 = 1;
        local_4c = 0;
        error(1,0,local_18);
      }
      if (local_48 < 1) break;
      padding_buffer = xpalloc(padding_buffer,&padding_buffer_size,local_48,0xffffffffffffffff,1);
    }
    if (dev_debug != '\0') {
      uVar1 = quote(padding_buffer);
      fprintf(_stderr,"formatting output:\n  value: %Lf\n  humanized: %s\n",uVar1);
    }
    local_40 = 0;
    if ((padding_width != 0) && (local_70 = gnu_mbswidth(padding_buffer,3), -1 < local_70)) {
      if (padding_width < 0) {
        if (padding_width < -local_70) {
          local_40 = padding_width + local_70;
        }
      }
      else if (local_70 < padding_width) {
        local_40 = padding_width - local_70;
      }
    }
    *param_2 = local_40;
    uVar1 = 1;
  }
  else {
    if (inval_style != 3) {
      local_6c = conv_exit_code;
      local_20 = gettext("value too large to be printed: \'%Lg\' (cannot handle values > 999Q)");
      local_58 = local_6c;
      local_54 = 0;
      error(local_6c,0,local_20);
    }
    uVar1 = 0;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar1;
}

// Function: print_padded_number
void print_padded_number(long param_1) {
  long local_18;
  long local_10;
  
  local_18 = param_1;
  if (format_str_prefix != (char *)0x0) {
    fputs_unlocked(format_str_prefix,_stdout);
  }
  for (; 0 < local_18; local_18 = local_18 + -1) {
    putchar_unlocked(0x20);
  }
  fputs_unlocked(padding_buffer,_stdout);
  for (local_10 = param_1; local_10 < 0; local_10 = local_10 + 1) {
    putchar_unlocked(0x20);
  }
  if (format_str_suffix != (char *)0x0) {
    fputs_unlocked(format_str_suffix,_stdout);
  }
  return;
}

// Function: process_suffixed_number
undefined8
process_suffixed_number(char *param_1,longdouble *param_2,undefined8 param_3,long param_4) {
  char cVar1;
  int iVar2;
  size_t sVar3;
  size_t sVar4;
  undefined8 uVar5;
  char *pcVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  longdouble lVar8;
  longdouble lVar9;
  longdouble local_38;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (suffix != (char *)0x0) {
    cVar1 = mbs_endswith(param_1,suffix);
    if (cVar1 == '\0') {
      if (dev_debug != '\0') {
        fwrite("no valid suffix found\n",1,0x16,_stderr);
      }
    }
    else {
      sVar3 = strlen(param_1);
      sVar4 = strlen(suffix);
      param_1[sVar3 - sVar4] = '\0';
      if (dev_debug != '\0') {
        uVar5 = quote(suffix);
        fprintf(_stderr,"trimming suffix %s\n",uVar5);
      }
    }
  }
  pcVar6 = (char *)skip_str_matching(param_1,newline_or_blank,1);
  if (auto_padding != 0) {
    if ((param_1 < pcVar6) || (1 < param_4)) {
      iVar2 = gnu_mbswidth(param_1,3);
      padding_width = (size_t)iVar2;
    }
    else {
      padding_width = 0;
    }
    if ((long)padding_width < 0) {
      padding_width = strlen(param_1);
    }
    if (dev_debug != '\0') {
      fprintf(_stderr,"setting Auto-Padding to %jd characters\n",padding_width);
    }
  }
  local_38 = (longdouble)0;
  iVar2 = parse_human_number(pcVar6,&local_38,param_3);
  if ((iVar2 == 1) && (debug != '\0')) {
    uVar5 = quote(pcVar6);
    uVar7 = gettext("large input value %s: possible precision loss");
    error(0,0,uVar7,uVar5);
  }
  if ((from_unit_size != 1) || (to_unit_size != 1)) {
    lVar8 = (longdouble)from_unit_size;
    if (from_unit_size < 0) {
      lVar8 = _DAT_00105b70 + lVar8;
    }
    lVar9 = (longdouble)to_unit_size;
    if (to_unit_size < 0) {
      lVar9 = _DAT_00105b70 + lVar9;
    }
    local_38 = (local_38 * lVar8) / lVar9;
  }
  *param_2 = local_38;
  if ((iVar2 == 0) || (iVar2 == 1)) {
    uVar5 = 1;
  }
  else {
    uVar5 = 0;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar5;
}

// Function: is_utf8_charset
undefined4 is_utf8_charset(void) {
  long lVar1;
  long in_FS_OFFSET;
  int local_1c;
  undefined local_18 [8];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  if (is_utf8_0 == -1) {
    mbszero(local_18);
    lVar1 = mbrtoc32(&local_1c,&DAT_0010582e,3,local_18);
    if ((lVar1 == 3) && (local_1c == 0x27f8)) {
      is_utf8_0 = 1;
    }
    else {
      is_utf8_0 = 0;
    }
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return CONCAT31((int3)((uint)is_utf8_0 >> 8),is_utf8_0 != 0);
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: mbsmbchr
void mbsmbchr(char *param_1,byte *param_2) {
  byte bVar1;
  char cVar2;
  size_t sVar3;
  
  bVar1 = *param_2;
  if ((bVar1 < 0x30) || (sVar3 = __ctype_get_mb_cur_max(), sVar3 == 1)) {
    strchr(param_1,(uint)bVar1);
    return;
  }
  cVar2 = is_utf8_charset();
  if (cVar2 == '\0') {
    if (param_2[1] == 0) {
      mbschr(param_1,bVar1);
      return;
    }
    mbsstr(param_1,param_2);
    return;
  }
  if (-1 < (char)bVar1) {
    strchr(param_1,(uint)bVar1);
    return;
  }
  strstr(param_1,(char *)param_2);
  return;
}

// Function: next_field
char * next_field(char **param_1) {
  char *__s;
  size_t sVar1;
  undefined8 uVar2;
  char *local_18;
  
  __s = *param_1;
  if (delimiter == 0) {
    uVar2 = skip_str_matching(__s,newline_or_blank,1);
    local_18 = (char *)skip_str_matching(uVar2,newline_or_blank,0);
  }
  else {
    local_18 = (char *)mbsmbchr(__s,delimiter);
    if (local_18 == (char *)0x0) {
      sVar1 = strlen(__s);
      local_18 = __s + sVar1;
    }
  }
  *param_1 = local_18;
  return __s;
}

// Function: include_field
bool include_field(ulong param_1) {
  bool bVar1;
  ulong *local_10;
  
  local_10 = _frp;
  if (_frp == (ulong *)0x0) {
    bVar1 = param_1 == 1;
  }
  else {
    for (; *local_10 != 0xffffffffffffffff; local_10 = local_10 + 2) {
      if ((*local_10 <= param_1) && (param_1 <= local_10[1])) {
        return true;
      }
    }
    bVar1 = false;
  }
  return bVar1;
}

// Function: process_field
char process_field(char *param_1,undefined8 param_2) {
  char cVar1;
  int iVar2;
  long in_FS_OFFSET;
  char local_39;
  undefined8 local_38;
  undefined8 local_30;
  longdouble local_28;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_28 = (longdouble)0;
  local_38 = 0;
  local_39 = '\x01';
  cVar1 = include_field(param_2);
  if (cVar1 == '\0') {
    fputs_unlocked(param_1,_stdout);
  }
  else {
    iVar2 = process_suffixed_number(param_1,&local_28,&local_38,param_2);
    local_39 = iVar2 != 0;
    if ((bool)local_39) {
      local_39 = prepare_padded_number(local_38,&local_30);
    }
    if (local_39 == '\0') {
      fputs_unlocked(param_1,_stdout);
    }
    else {
      print_padded_number(local_30);
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_39;
}

// Function: process_line
undefined process_line(char *param_1,char param_2) {
  char *pcVar1;
  char cVar2;
  int iVar3;
  size_t sVar4;
  char *local_40;
  undefined local_32;
  char local_31;
  long local_30;
  undefined8 local_28;
  ulong local_20;
  
  local_30 = 0;
  local_32 = 1;
  local_40 = param_1;
  while( true ) {
    local_30 = local_30 + 1;
    local_28 = next_field(&local_40);
    if (*local_40 == '\0') break;
    local_31 = *local_40;
    *local_40 = '\0';
    cVar2 = process_field(local_28,local_30);
    if (cVar2 != '\x01') {
      local_32 = 0;
    }
    if (delimiter == (char *)0x0) {
      fputc_unlocked(0x20,_stdout);
    }
    else {
      fputs_unlocked(delimiter,_stdout);
    }
    pcVar1 = local_40;
    if (delimiter == (char *)0x0) {
      *local_40 = local_31;
      local_20 = mcel_scanz(local_40);
      local_40 = local_40 + (local_20 >> 0x28 & 0xff);
    }
    else {
      sVar4 = strlen(delimiter);
      if (sVar4 < 2) {
        sVar4 = 1;
      }
      else {
        sVar4 = strlen(delimiter);
      }
      local_40 = pcVar1 + sVar4;
    }
  }
  cVar2 = process_field(local_28,local_30);
  if (cVar2 != '\x01') {
    local_32 = 0;
  }
  if (param_2 != '\0') {
    putchar_unlocked((uint)line_delim);
  }
  iVar3 = ferror_unlocked(_stdout);
  if (iVar3 != 0) {
    write_error();
  }
  return local_32;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  char cVar1;
  int iVar2;
  uint uVar3;
  char *pcVar4;
  size_t sVar5;
  long lVar6;
  undefined8 uVar7;
  __ssize_t _Var8;
  int *piVar9;
  long in_FS_OFFSET;
  bool bVar10;
  uint local_114;
  int local_110;
  char *local_a0;
  __ssize_t local_98;
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
  undefined8 local_38;
  undefined8 local_30;
  ulong local_28;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  local_114 = 1;
  set_program_name(*param_2);
  pcVar4 = setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  decimal_point = (char *)rpl_nl_langinfo(0x10000);
  if ((decimal_point == (char *)0x0) || (*decimal_point == '\0')) {
    decimal_point = ".";
  }
  sVar5 = strlen(decimal_point);
  decimal_point_length = (undefined4)sVar5;
  thousands_sep = (char *)rpl_nl_langinfo(0x10001);
  if (thousands_sep == (char *)0x0) {
    thousands_sep = "";
  }
  sVar5 = strlen(thousands_sep);
  thousands_sep_length = (int)sVar5;
  atexit((__func *)&close_stdout);
LAB_0010308f:
  do {
    while( true ) {
      iVar2 = getopt_long(param_1,param_2,&DAT_0010584c,longopts,0);
      if (iVar2 == -1) {
        if ((format_str != (char *)0x0) && (grouping != 0)) {
          local_70 = gettext("--grouping cannot be combined with --format");
          error(1,0,local_70);
        }
        if ((debug != '\0') && (pcVar4 == (char *)0x0)) {
          local_68 = gettext("failed to set locale");
          error(0,0,local_68);
        }
        if (((((debug != '\0') && (scale_from == 0)) && (scale_to == 0)) &&
            ((grouping == 0 && (padding_width == 0)))) && (format_str == (char *)0x0)) {
          local_60 = gettext("no conversion option specified");
          error(0,0,local_60);
        }
        if (((debug != '\0') && (unit_separator != (char *)0x0)) && (delimiter == (char *)0x0)) {
          local_58 = gettext("field delimiters have higher precedence than unit separators");
          error(0,0,local_58);
        }
        if (format_str != (char *)0x0) {
          parse_format_string(format_str);
        }
        if (grouping != 0) {
          if (scale_to != 0) {
            local_50 = gettext("grouping cannot be combined with --to");
            error(1,0,local_50);
          }
          if ((debug != '\0') && (thousands_sep_length == 0)) {
            local_48 = gettext("grouping has no effect in this locale");
            error(0,0,local_48);
          }
        }
        if ((padding_width == 0) && (delimiter == (char *)0x0)) {
          auto_padding = 1;
        }
        else {
          auto_padding = 0;
        }
        if (inval_style != 0) {
          conv_exit_code = 0;
        }
        if (_optind < param_1) {
          if ((debug != '\0') && (header != 0)) {
            local_40 = gettext("--header ignored with command-line input");
            error(0,0,local_40);
          }
          for (; _optind < param_1; _optind = _optind + 1) {
            uVar3 = process_line(param_2[_optind],1);
            local_114 = local_114 & uVar3;
          }
        }
        else {
          local_a0 = (char *)0x0;
          local_28 = 0;
          while ((lVar6 = header + -1, bVar10 = header != 0, header = lVar6, bVar10 &&
                 (_Var8 = __getdelim(&local_a0,&local_28,(uint)line_delim,_stdin), 0 < _Var8))) {
            iVar2 = fputs_unlocked(local_a0,_stdout);
            if (iVar2 == -1) {
              write_error();
            }
          }
          while (local_98 = __getdelim(&local_a0,&local_28,(uint)line_delim,_stdin), 0 < local_98) {
            cVar1 = local_a0[local_98 + -1];
            uVar3 = (uint)line_delim;
            if ((int)cVar1 == uVar3) {
              local_a0[local_98 + -1] = '\0';
            }
            uVar3 = process_line(local_a0,(int)cVar1 == uVar3);
            local_114 = local_114 & uVar3;
          }
          iVar2 = ferror_unlocked(_stdin);
          if (iVar2 != 0) {
            uVar7 = gettext("error reading input");
            piVar9 = __errno_location();
            local_38 = uVar7;
            error(1,*piVar9,uVar7);
          }
        }
        if ((debug != '\0') && (local_114 == 0)) {
          local_30 = gettext("failed to convert some of the input numbers");
          error(0,0,local_30);
        }
        local_110 = 0;
        if (((local_114 == 0) && (inval_style != 2)) && (inval_style != 3)) {
          local_110 = 2;
        }
                    /* WARNING: Subroutine does not return */
        exit(local_110);
      }
      if (iVar2 < 0x8f) break;
switchD_0010313d_caseD_65:
      usage(1);
    }
    if (iVar2 < 100) {
      if (iVar2 == -0x83) {
LAB_001035c6:
        uVar7 = proper_name_lite("Assaf Gordon","Assaf Gordon");
        version_etc(_stdout,"numfmt","GNU coreutils",_Version,uVar7,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar2 == -0x82) {
        usage(0);
        goto LAB_001035c6;
      }
      goto switchD_0010313d_caseD_65;
    }
    switch(iVar2) {
    case 100:
      if ((*_optarg != '\0') &&
         (local_28 = mcel_scanz(_optarg), _optarg[local_28 >> 0x28 & 0xff] != '\0')) {
        local_80 = gettext("the delimiter must be a single character");
        error(1,0,local_80);
      }
      delimiter = _optarg;
      break;
    default:
      goto switchD_0010313d_caseD_65;
    case 0x7a:
      line_delim = 0;
      break;
    case 0x80:
      lVar6 = __xargmatch_internal
                        ("--from",_optarg,scale_from_args,scale_from_types,4,_argmatch_die,1);
      scale_from = *(int *)(scale_from_types + lVar6 * 4);
      break;
    case 0x81:
      from_unit_size = unit_to_umax(_optarg);
      break;
    case 0x82:
      lVar6 = __xargmatch_internal
                        (&DAT_00105857,_optarg,scale_to_args,scale_to_types,4,_argmatch_die,1);
      scale_to = *(int *)(scale_to_types + lVar6 * 4);
      break;
    case 0x83:
      to_unit_size = unit_to_umax(_optarg);
      break;
    case 0x84:
      lVar6 = __xargmatch_internal("--round",_optarg,round_args,round_types,4,_argmatch_die,1);
      round_style = *(undefined4 *)(round_types + lVar6 * 4);
      break;
    case 0x85:
      suffix = _optarg;
      break;
    case 0x86:
      grouping = 1;
      break;
    case 0x87:
      uVar3 = xstrtoimax(_optarg,0,10,&padding_width,&DAT_00103e04);
      if ((uVar3 < 2) && (padding_width != 0)) break;
      uVar7 = quote(_optarg);
      local_90 = gettext("invalid padding value %s");
      error(1,0,local_90,uVar7);
    case 0x88:
      if (_n_frp != 0) {
        local_88 = gettext("multiple field specifications");
        error(1,0,local_88);
      }
      set_fields(_optarg,1);
      break;
    case 0x89:
      debug = '\x01';
      break;
    case 0x8a:
      dev_debug = 1;
      debug = '\x01';
      break;
    case 0x8b:
      if (_optarg != (char *)0x0) {
        iVar2 = xstrtoumax(_optarg,0,10,&header,&DAT_00103e04);
        if ((iVar2 == 0) && (header != 0)) break;
        uVar7 = quote(_optarg);
        local_78 = gettext("invalid header value %s");
        error(1,0,local_78,uVar7);
      }
      header = 1;
      break;
    case 0x8c:
      format_str = _optarg;
      break;
    case 0x8d:
      lVar6 = __xargmatch_internal("--invalid",_optarg,inval_args,inval_types,4,_argmatch_die,1);
      inval_style = *(int *)(inval_types + lVar6 * 4);
      break;
    case 0x8e:
      goto switchD_0010313d_caseD_8e;
    }
  } while( true );
switchD_0010313d_caseD_8e:
  unit_separator = _optarg;
  goto LAB_0010308f;
}

