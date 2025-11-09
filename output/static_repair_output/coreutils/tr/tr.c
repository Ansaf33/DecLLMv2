// Function: es_match
undefined4 es_match(long *param_1,long param_2,char param_3) {
  undefined4 uVar1;
  
  if ((param_3 == *(char *)(param_2 + *param_1)) && (*(char *)(param_2 + param_1[1]) != '\x01')) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... STRING1 [STRING2]\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Translate, squeeze, and/or delete characters from standard input,\nwriting to standard output.  STRING1 and STRING2 specify arrays of\ncharacters ARRAY1 and ARRAY2 that control the action.\n\n  -c, -C, --complement    use the complement of ARRAY1\n  -d, --delete            delete characters in ARRAY1, do not translate\n  -s, --squeeze-repeats   replace each sequence of a repeated character\n                            that is listed in the last specified ARRAY,\n                            with a single occurrence of that character\n  -t, --truncate-set1     first truncate ARRAY1 to length of ARRAY2\n"
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
                            "\nARRAYs are specified as strings of characters.  Most represent themselves.\nInterpreted sequences are:\n\n  \\NNN            character with octal value NNN (1 to 3 octal digits)\n  \\\\              backslash\n  \\a              audible BEL\n  \\b              backspace\n  \\f              form feed\n  \\n              new line\n  \\r              return\n  \\t              horizontal tab\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  \\v              vertical tab\n  CHAR1-CHAR2     all characters from CHAR1 to CHAR2 in ascending order\n  [CHAR*]         in ARRAY2, copies of CHAR until length of ARRAY1\n  [CHAR*REPEAT]   REPEAT copies of CHAR, REPEAT octal if starting with 0\n  [:alnum:]       all letters and digits\n  [:alpha:]       all letters\n  [:blank:]       all horizontal whitespace\n  [:cntrl:]       all control characters\n  [:digit:]       all digits\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  [:graph:]       all printable characters, not including space\n  [:lower:]       all lower case letters\n  [:print:]       all printable characters, including space\n  [:punct:]       all punctuation characters\n  [:space:]       all horizontal or vertical whitespace\n  [:upper:]       all upper case letters\n  [:xdigit:]      all hexadecimal digits\n  [=CHAR=]        all characters which are equivalent to CHAR\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nTranslation occurs if -d is not given and both STRING1 and STRING2 appear.\n-t is only significant when translating.  ARRAY2 is extended to length of\nARRAY1 by repeating its last character as necessary.  Excess characters\nof ARRAY2 are ignored.  Character classes expand in unspecified order;\nwhile translating, [:lower:] and [:upper:] may be used in pairs to\nspecify case conversion.  Squeezing occurs after translation or deletion.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00106873);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: is_equiv_class_member
bool is_equiv_class_member(char param_1,char param_2) {
  return param_1 == param_2;
}

// Function: is_char_class_member
void is_char_class_member(uint param_1) {
                    /* WARNING: Could not recover jumptable at 0x00100507. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(&DAT_00106878 + *(int *)(&DAT_00106878 + (ulong)param_1 * 4)))();
  return;
}

// Function: es_free
void es_free(void **param_1) {
  free(*param_1);
  free(param_1[1]);
  return;
}

// Function: unquote
undefined8 unquote(char *param_1,long *param_2) {
  uint uVar1;
  char cVar2;
  char cVar3;
  char cVar4;
  char cVar5;
  char cVar6;
  char cVar7;
  int iVar8;
  size_t sVar9;
  long lVar10;
  undefined8 uVar11;
  byte local_6d;
  uint local_6c;
  uint local_68;
  
  sVar9 = strlen(param_1);
  lVar10 = xmalloc(sVar9);
  *param_2 = lVar10;
  lVar10 = xcalloc(sVar9,1);
  param_2[1] = lVar10;
  local_6c = 0;
  local_68 = 0;
  do {
    if (param_1[local_68] == '\0') {
      param_2[2] = (ulong)local_6c;
      return 1;
    }
    if (param_1[local_68] == '\\') {
      *(undefined *)((ulong)local_6c + param_2[1]) = 1;
      cVar2 = param_1[local_68 + 1];
      iVar8 = (int)cVar2;
      if (cVar2 == '\0') {
        uVar11 = gettext("warning: an unescaped backslash at end of string is not portable");
        error(0,0,uVar11);
        *(undefined *)((ulong)local_6c + param_2[1]) = 0;
        local_6d = 0x5c;
        uVar1 = local_68 - 1;
        goto LAB_00100a14;
      }
      uVar1 = local_68;
      if (((cVar2 < '\0') || (0x76 < iVar8)) || (iVar8 < 0x30)) {
switchD_001007ca_caseD_38:
        local_6d = param_1[local_68 + 1];
      }
      else {
        switch(iVar8) {
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
          local_6d = param_1[local_68 + 1] - 0x30;
          iVar8 = param_1[local_68 + 2] + -0x30;
          if ((-1 < iVar8) && (iVar8 < 8)) {
            local_6d = (char)iVar8 + local_6d * '\b';
            uVar1 = local_68 + 1;
            iVar8 = param_1[local_68 + 3] + -0x30;
            if ((-1 < iVar8) && (iVar8 < 8)) {
              if ((int)(iVar8 + (uint)local_6d * 8) < 0x100) {
                local_6d = (char)iVar8 + local_6d * '\b';
                uVar1 = local_68 + 2;
              }
              else {
                cVar2 = param_1[local_68 + 3];
                cVar3 = param_1[local_68 + 2];
                cVar4 = param_1[uVar1];
                cVar5 = param_1[local_68 + 3];
                cVar6 = param_1[local_68 + 2];
                cVar7 = param_1[uVar1];
                uVar11 = gettext(
                                "warning: the ambiguous octal escape \\%c%c%c is being\n\tinterpreted as the 2-byte sequence \\0%c%c, %c"
                                );
                error(0,0,uVar11,(int)cVar7,(int)cVar6,(int)cVar5,(int)cVar4,(int)cVar3,(int)cVar2);
              }
            }
          }
          break;
        default:
          goto switchD_001007ca_caseD_38;
        case 0x5c:
          local_6d = 0x5c;
          break;
        case 0x61:
          local_6d = 7;
          break;
        case 0x62:
          local_6d = 8;
          break;
        case 0x66:
          local_6d = 0xc;
          break;
        case 0x6e:
          local_6d = 10;
          break;
        case 0x72:
          local_6d = 0xd;
          break;
        case 0x74:
          local_6d = 9;
          break;
        case 0x76:
          local_6d = 0xb;
        }
      }
LAB_00100a14:
      local_68 = uVar1;
      local_68 = local_68 + 1;
      *(byte *)(*param_2 + (ulong)local_6c) = local_6d;
    }
    else {
      *(char *)(*param_2 + (ulong)local_6c) = param_1[local_68];
    }
    local_6c = local_6c + 1;
    local_68 = local_68 + 1;
  } while( true );
}

// Function: look_up_char_class
uint look_up_char_class(char *param_1,size_t param_2) {
  int iVar1;
  size_t sVar2;
  uint local_c;
  
  local_c = 0;
  while( true ) {
    if (0xb < local_c) {
      return 9999;
    }
    iVar1 = strncmp(param_1,*(char **)(char_class_name + (ulong)local_c * 8),param_2);
    if ((iVar1 == 0) &&
       (sVar2 = strlen(*(char **)(char_class_name + (ulong)local_c * 8)), param_2 == sVar2)) break;
    local_c = local_c + 1;
  }
  return local_c;
}

// Function: make_printable_char
byte * make_printable_char(byte param_1) {
  byte *__s;
  ushort **ppuVar1;
  
  __s = (byte *)xmalloc(5);
  ppuVar1 = __ctype_b_loc();
  if (((*ppuVar1)[param_1] & 0x4000) == 0) {
    sprintf((char *)__s,"\\%03o",(ulong)param_1);
  }
  else {
    *__s = param_1;
    __s[1] = 0;
  }
  return __s;
}

// Function: make_printable_str
char * make_printable_str(long param_1,ulong param_2) {
  byte bVar1;
  char *pcVar2;
  ushort **ppuVar3;
  long in_FS_OFFSET;
  char *local_38;
  ulong local_30;
  byte *local_28;
  byte local_15;
  undefined local_14;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  pcVar2 = (char *)xnmalloc(param_2 + 1,4);
  local_30 = 0;
  local_38 = pcVar2;
  do {
    if (param_2 <= local_30) {
      if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
        return pcVar2;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    bVar1 = *(byte *)(local_30 + param_1);
    if (bVar1 < 0xe) {
      if (bVar1 < 7) goto switchD_00100c3d_caseD_7;
      switch(bVar1) {
      case 7:
        local_28 = &DAT_00106a78;
        break;
      case 8:
        local_28 = &DAT_00106a7b;
        break;
      case 9:
        local_28 = &DAT_00106a87;
        break;
      case 10:
        local_28 = &DAT_00106a81;
        break;
      case 0xb:
        local_28 = &DAT_00106a8a;
        break;
      case 0xc:
        local_28 = &DAT_00106a7e;
        break;
      case 0xd:
        local_28 = &DAT_00106a84;
        break;
      default:
        goto switchD_00100c3d_caseD_7;
      }
    }
    else {
      if (bVar1 == 0x5c) {
        local_28 = &DAT_00106a76;
        goto LAB_00100d0f;
      }
switchD_00100c3d_caseD_7:
      ppuVar3 = __ctype_b_loc();
      if (((*ppuVar3)[bVar1] & 0x4000) == 0) {
        sprintf((char *)&local_15,"\\%03o",(ulong)bVar1);
      }
      else {
        local_14 = 0;
        local_15 = bVar1;
      }
      local_28 = &local_15;
    }
LAB_00100d0f:
    local_38 = stpcpy(local_38,(char *)local_28);
    local_30 = local_30 + 1;
  } while( true );
}

// Function: append_normal_char
void append_normal_char(long param_1,undefined param_2) {
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)xmalloc(0x20);
  *(undefined8 *)(puVar1 + 2) = 0;
  *puVar1 = 0;
  *(undefined *)(puVar1 + 4) = param_2;
  *(undefined4 **)(*(long *)(param_1 + 8) + 8) = puVar1;
  *(undefined4 **)(param_1 + 8) = puVar1;
  return;
}

// Function: append_range
bool append_range(long param_1,byte param_2,byte param_3) {
  void *__ptr;
  void *__ptr_00;
  undefined8 uVar1;
  undefined4 *puVar2;
  
  if (param_2 <= param_3) {
    puVar2 = (undefined4 *)xmalloc(0x20);
    *(undefined8 *)(puVar2 + 2) = 0;
    *puVar2 = 1;
    *(byte *)(puVar2 + 4) = param_2;
    *(byte *)((long)puVar2 + 0x11) = param_3;
    *(undefined4 **)(*(long *)(param_1 + 8) + 8) = puVar2;
    *(undefined4 **)(param_1 + 8) = puVar2;
  }
  else {
    __ptr = (void *)make_printable_char(param_2);
    __ptr_00 = (void *)make_printable_char(param_3);
    uVar1 = gettext("range-endpoints of \'%s-%s\' are in reverse collating sequence order");
    error(0,0,uVar1,__ptr,__ptr_00);
    free(__ptr);
    free(__ptr_00);
  }
  return param_2 <= param_3;
}

// Function: append_char_class
bool append_char_class(long param_1,undefined8 param_2,undefined8 param_3) {
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = look_up_char_class(param_2,param_3);
  if (iVar1 != 9999) {
    puVar2 = (undefined4 *)xmalloc(0x20);
    *(undefined8 *)(puVar2 + 2) = 0;
    *puVar2 = 2;
    puVar2[4] = iVar1;
    *(undefined4 **)(*(long *)(param_1 + 8) + 8) = puVar2;
    *(undefined4 **)(param_1 + 8) = puVar2;
  }
  return iVar1 != 9999;
}

// Function: append_repeated_char
void append_repeated_char(long param_1,undefined param_2,undefined8 param_3) {
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)xmalloc(0x20);
  *(undefined8 *)(puVar1 + 2) = 0;
  *puVar1 = 4;
  *(undefined *)(puVar1 + 4) = param_2;
  *(undefined8 *)(puVar1 + 6) = param_3;
  *(undefined4 **)(*(long *)(param_1 + 8) + 8) = puVar1;
  *(undefined4 **)(param_1 + 8) = puVar1;
  return;
}

// Function: append_equiv_class
bool append_equiv_class(long param_1,undefined *param_2,long param_3) {
  undefined4 *puVar1;
  
  if (param_3 == 1) {
    puVar1 = (undefined4 *)xmalloc(0x20);
    *(undefined8 *)(puVar1 + 2) = 0;
    *puVar1 = 3;
    *(undefined *)(puVar1 + 4) = *param_2;
    *(undefined4 **)(*(long *)(param_1 + 8) + 8) = puVar1;
    *(undefined4 **)(param_1 + 8) = puVar1;
  }
  return param_3 == 1;
}

// Function: find_closing_delim
undefined8 find_closing_delim(long *param_1,ulong param_2,char param_3,ulong *param_4) {
  ulong local_10;
  
  local_10 = param_2;
  while( true ) {
    if (param_1[2] - 1U <= local_10) {
      return 0;
    }
    if ((((param_3 == *(char *)(local_10 + *param_1)) && (*(char *)(*param_1 + local_10 + 1) == ']')
         ) && (*(char *)(local_10 + param_1[1]) != '\x01')) &&
       (*(char *)(param_1[1] + local_10 + 1) != '\x01')) break;
    local_10 = local_10 + 1;
  }
  *param_4 = local_10;
  return 1;
}

// Function: find_bracketed_repeat
undefined8
find_bracketed_repeat(long *param_1,long param_2,undefined *param_3,long *param_4,ulong *param_5) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  char *local_50;
  ulong local_48;
  long local_40;
  char *local_38;
  void *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if ((ulong)param_1[2] <= param_2 + 1U) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("start_idx + 1 < es->len",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tr.c",
                  0x30a,"find_bracketed_repeat");
  }
  cVar1 = es_match(param_1,param_2 + 1,0x2a);
  if (cVar1 == '\x01') {
    for (local_48 = param_2 + 2;
        (local_48 < (ulong)param_1[2] && (*(char *)(local_48 + param_1[1]) != '\x01'));
        local_48 = local_48 + 1) {
      if (*(char *)(local_48 + *param_1) == ']') {
        local_40 = (local_48 - param_2) + -2;
        *param_3 = *(undefined *)(param_2 + *param_1);
        if (local_40 == 0) {
          *param_4 = 0;
        }
        else {
          local_38 = (char *)(*param_1 + param_2 + 2);
          if (*local_38 == '0') {
            uVar3 = 8;
          }
          else {
            uVar3 = 10;
          }
          iVar2 = xstrtoumax(local_38,&local_50,uVar3,param_4,0);
          if (((iVar2 != 0) || (*param_4 == -1)) || (local_38 + local_40 != local_50)) {
            local_30 = (void *)make_printable_str(local_38,local_40);
            uVar3 = quote(local_30);
            local_28 = gettext("invalid repeat count %s in [c*n] construct");
            error(0,0,local_28,uVar3);
            free(local_30);
            uVar3 = 0xfffffffe;
            goto LAB_00101327;
          }
        }
        *param_5 = local_48;
        uVar3 = 0;
        goto LAB_00101327;
      }
    }
    uVar3 = 0xffffffff;
  }
  else {
    uVar3 = 0xffffffff;
  }
LAB_00101327:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar3;
}

// Function: star_digits_closebracket
undefined8 star_digits_closebracket(long *param_1,long param_2) {
  char cVar1;
  undefined8 uVar2;
  ulong local_10;
  
  cVar1 = es_match(param_1,param_2,0x2a);
  if (cVar1 == '\x01') {
    for (local_10 = param_2 + 1; local_10 < (ulong)param_1[2]; local_10 = local_10 + 1) {
      cVar1 = c_isdigit((int)*(char *)(local_10 + *param_1));
      if ((cVar1 != '\x01') || (*(char *)(local_10 + param_1[1]) != '\0')) {
        uVar2 = es_match(param_1,local_10,0x5d);
        return uVar2;
      }
    }
  }
  return 0;
}

// Function: build_spec_list
undefined8 build_spec_list(long *param_1,undefined8 param_2) {
  char cVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  undefined local_a6;
  char local_a5;
  int local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  long local_80;
  long local_78;
  ulong local_70;
  long local_68;
  long local_60;
  long local_58;
  void *local_50;
  void *local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_68 = *param_1;
  local_70 = 0;
LAB_0010189a:
  do {
    if ((ulong)param_1[2] <= local_70 + 2) {
      for (; local_70 < (ulong)param_1[2]; local_70 = local_70 + 1) {
        append_normal_char(param_2,*(undefined *)(local_70 + local_68));
      }
      uVar2 = 1;
LAB_001018f5:
      if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
        return uVar2;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    cVar1 = es_match(param_1,local_70,0x5b);
    if (cVar1 == '\0') {
LAB_001017fe:
      cVar1 = es_match(param_1,local_70 + 1,0x2d);
      if (cVar1 == '\0') {
        append_normal_char(param_2,*(undefined *)(local_70 + local_68));
        local_70 = local_70 + 1;
      }
      else {
        cVar1 = append_range(param_2,*(undefined *)(local_70 + local_68),
                             *(undefined *)(local_68 + local_70 + 2));
        if (cVar1 != '\x01') {
          uVar2 = 0;
          goto LAB_001018f5;
        }
        local_70 = local_70 + 3;
      }
      goto LAB_0010189a;
    }
    local_a5 = '\x01';
    cVar1 = es_match(param_1,local_70 + 1,0x3a);
    if (((cVar1 == '\0') && (cVar1 = es_match(param_1,local_70 + 1,0x3d), cVar1 == '\0')) ||
       (cVar1 = find_closing_delim(param_1,local_70 + 2,(int)*(char *)(local_68 + local_70 + 1),
                                   &local_78), cVar1 == '\0')) goto LAB_00101770;
    local_60 = (local_78 - local_70) + -2;
    local_58 = local_68 + local_70 + 2;
    if (local_60 == 0) {
      if (*(char *)(local_68 + local_70 + 1) == ':') {
        local_40 = gettext("missing character class name \'[::]\'");
        local_a0 = 0;
        local_9c = 0;
        error(0,0,local_40);
      }
      else {
        local_38 = gettext("missing equivalence class character \'[==]\'");
        local_98 = 0;
        local_94 = 0;
        error(0,0,local_38);
      }
      uVar2 = 0;
      goto LAB_001018f5;
    }
    if (*(char *)(local_68 + local_70 + 1) == ':') {
      cVar1 = append_char_class(param_2,local_58,local_60);
      if (cVar1 != '\x01') {
        cVar1 = star_digits_closebracket(param_1,local_70 + 2);
        if (cVar1 != '\0') goto LAB_00101770;
        local_48 = (void *)make_printable_str(local_58,local_60);
        uVar2 = quote(local_48);
        local_30 = gettext("invalid character class %s");
        local_90 = 0;
        local_8c = 0;
        error(0,0,local_30,uVar2);
        free(local_48);
        uVar2 = 0;
        goto LAB_001018f5;
      }
LAB_00101758:
      local_70 = local_78 + 2;
    }
    else {
      cVar1 = append_equiv_class(param_2,local_58,local_60);
      if (cVar1 == '\x01') goto LAB_00101758;
      cVar1 = star_digits_closebracket(param_1,local_70 + 2);
      if (cVar1 == '\0') {
        local_50 = (void *)make_printable_str(local_58,local_60);
        local_28 = gettext("%s: equivalence class operand must be a single character");
        local_88 = 0;
        local_84 = 0;
        error(0,0,local_28,local_50);
        free(local_50);
        uVar2 = 0;
        goto LAB_001018f5;
      }
LAB_00101770:
      local_a4 = find_bracketed_repeat(param_1,local_70 + 1,&local_a6,&local_78,&local_80);
      if (local_a4 == 0) {
        append_repeated_char(param_2,local_a6,local_78);
        local_70 = local_80 + 1;
      }
      else {
        if (local_a4 != -1) {
          uVar2 = 0;
          goto LAB_001018f5;
        }
        local_a5 = '\0';
      }
      if (local_a5 == '\0') goto LAB_001017fe;
    }
  } while( true );
}

// Function: skip_construct
void skip_construct(long param_1) {
  *(undefined8 *)(param_1 + 8) = *(undefined8 *)(*(long *)(param_1 + 8) + 8);
  *(undefined8 *)(param_1 + 0x10) = 0xffffffffffffffff;
  return;
}

// Function: get_next
undefined8 get_next(long *param_1,undefined4 *param_2) {
  undefined8 uVar1;
  
  if (param_2 != (undefined4 *)0x0) {
    *param_2 = 2;
  }
  if (param_1[2] == -2) {
    param_1[1] = *(long *)(*param_1 + 8);
    param_1[2] = -1;
  }
  if ((uint *)param_1[1] == (uint *)0x0) {
    return 0xffffffff;
  }
                    /* WARNING: Could not recover jumptable at 0x001019d3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = (*(code *)(&DAT_00106c7c + *(int *)(&DAT_00106c7c + (ulong)*(uint *)param_1[1] * 4)))();
  return uVar1;
}

// Function: card_of_complement
int card_of_complement(long param_1) {
  int iVar1;
  long lVar2;
  undefined8 *puVar3;
  long in_FS_OFFSET;
  int local_120;
  undefined8 local_118 [33];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_120 = 0x100;
  puVar3 = local_118;
  for (lVar2 = 0x20; lVar2 != 0; lVar2 = lVar2 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  *(undefined8 *)(param_1 + 0x10) = 0xfffffffffffffffe;
  while( true ) {
    iVar1 = get_next(param_1,0);
    if (iVar1 == -1) break;
    local_120 = local_120 - (uint)(*(byte *)((long)local_118 + (long)iVar1) ^ 1);
    *(undefined *)((long)local_118 + (long)iVar1) = 1;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_120;
}

// Function: validate_case_classes
void validate_case_classes(long param_1,long param_2) {
  long lVar1;
  ushort **ppuVar2;
  long in_FS_OFFSET;
  bool bVar3;
  bool bVar4;
  int local_64;
  int local_60;
  int local_5c;
  int local_58;
  int local_54;
  undefined4 local_50;
  undefined4 local_4c;
  long local_48;
  long local_40;
  ulong local_38;
  ulong local_30;
  undefined8 local_28;
  undefined8 local_20;
  undefined8 local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_48 = 0;
  local_40 = 0;
  local_5c = 0;
  local_58 = 0;
  local_38 = *(ulong *)(param_1 + 0x18);
  local_30 = *(ulong *)(param_2 + 0x18);
  local_28 = *(undefined8 *)(param_1 + 8);
  local_20 = *(undefined8 *)(param_2 + 8);
  bVar3 = true;
  bVar4 = true;
  if ((complement == '\0') && (*(char *)(param_2 + 0x31) == '\x01')) {
    for (local_54 = 0; local_54 < 0x100; local_54 = local_54 + 1) {
      ppuVar2 = __ctype_b_loc();
      if (((*ppuVar2)[local_54] & 0x100) != 0) {
        local_48 = local_48 + 1;
      }
      ppuVar2 = __ctype_b_loc();
      if (((*ppuVar2)[local_54] & 0x200) != 0) {
        local_40 = local_40 + 1;
      }
    }
    *(undefined8 *)(param_1 + 0x10) = 0xfffffffffffffffe;
    *(undefined8 *)(param_2 + 0x10) = 0xfffffffffffffffe;
    while ((local_5c != -1 && (local_58 != -1))) {
      local_5c = get_next(param_1,&local_64);
      local_58 = get_next(param_2,&local_60);
      if (((bVar4) && (local_60 != 2)) && ((!bVar3 || (local_64 == 2)))) {
        local_18 = gettext("misaligned [:upper:] and/or [:lower:] construct");
        local_50 = 1;
        local_4c = 0;
        error(1,0,local_18);
      }
      if (local_60 != 2) {
        skip_construct(param_1);
        skip_construct(param_2);
        lVar1 = local_40;
        if (local_64 == 1) {
          lVar1 = local_48;
        }
        *(long *)(param_1 + 0x18) = *(long *)(param_1 + 0x18) - (lVar1 + -1);
        lVar1 = local_40;
        if (local_60 == 1) {
          lVar1 = local_48;
        }
        *(long *)(param_2 + 0x18) = *(long *)(param_2 + 0x18) - (lVar1 + -1);
      }
      bVar3 = *(long *)(param_1 + 0x10) == -1;
      bVar4 = *(long *)(param_2 + 0x10) == -1;
    }
    if ((local_38 < *(ulong *)(param_1 + 0x18)) || (local_30 < *(ulong *)(param_2 + 0x18))) {
                    /* WARNING: Subroutine does not return */
      __assert_fail("old_s1_len >= s1->length && old_s2_len >= s2->length",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tr.c",
                    0x4bf,"validate_case_classes");
    }
    *(undefined8 *)(param_1 + 8) = local_28;
    *(undefined8 *)(param_2 + 8) = local_20;
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: get_spec_stats
void get_spec_stats(long *param_1) {
  long lVar1;
  long in_FS_OFFSET;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  param_1[4] = 0;
  *(undefined *)(param_1 + 6) = 0;
  *(undefined *)((long)param_1 + 0x32) = 0;
  *(undefined *)((long)param_1 + 0x31) = 0;
  if (*(uint **)(*param_1 + 8) != (uint *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x001020fd. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)(&DAT_00106d44 + *(int *)(&DAT_00106d44 + (ulong)**(uint **)(*param_1 + 8) * 4)))();
    return;
  }
  param_1[3] = 0;
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: get_s1_spec_stats
void get_s1_spec_stats(long param_1) {
  int iVar1;
  
  get_spec_stats(param_1);
  if (complement != '\0') {
    iVar1 = card_of_complement(param_1);
    *(long *)(param_1 + 0x18) = (long)iVar1;
  }
  return;
}

// Function: get_s2_spec_stats
void get_s2_spec_stats(long param_1,ulong param_2) {
  get_spec_stats(param_1);
  if ((*(ulong *)(param_1 + 0x18) <= param_2) && (*(long *)(param_1 + 0x20) == 1)) {
    *(ulong *)(*(long *)(param_1 + 0x28) + 0x18) = param_2 - *(long *)(param_1 + 0x18);
    *(ulong *)(param_1 + 0x18) = param_2;
  }
  return;
}

// Function: spec_init
void spec_init(long *param_1) {
  long lVar1;
  
  lVar1 = xmalloc(0x20);
  param_1[1] = lVar1;
  *param_1 = param_1[1];
  *(undefined8 *)(*param_1 + 8) = 0;
  return;
}

// Function: parse_str
undefined parse_str(undefined8 param_1,undefined8 param_2) {
  char cVar1;
  undefined uVar2;
  long in_FS_OFFSET;
  undefined local_28 [24];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  cVar1 = unquote(param_1,local_28);
  if (cVar1 != '\0') {
    cVar1 = build_spec_list(local_28,param_2);
    if (cVar1 != '\0') {
      uVar2 = 1;
      goto LAB_0010243b;
    }
  }
  uVar2 = 0;
LAB_0010243b:
  es_free(local_28);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar2;
}

// Function: string2_extend
void string2_extend(long param_1,long param_2) {
  if (translating == '\0') {
                    /* WARNING: Subroutine does not return */
    __assert_fail("translating",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tr.c",
                  0x555,"string2_extend");
  }
  if (*(ulong *)(param_1 + 0x18) <= *(ulong *)(param_2 + 0x18)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("s1->length > s2->length",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tr.c",
                  0x556,"string2_extend");
  }
  if (*(long *)(param_2 + 0x18) == 0) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("s2->length > 0",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tr.c",
                  0x557,"string2_extend");
  }
                    /* WARNING: Could not recover jumptable at 0x00102553. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(&DAT_00106dfc + *(int *)(&DAT_00106dfc + (ulong)**(uint **)(param_2 + 8) * 4)))();
  return;
}

// Function: homogeneous_spec_list
undefined8 homogeneous_spec_list(long param_1) {
  int iVar1;
  int iVar2;
  
  *(undefined8 *)(param_1 + 0x10) = 0xfffffffffffffffe;
  iVar1 = get_next(param_1,0);
  if (iVar1 != -1) {
    do {
      iVar2 = get_next(param_1,0);
      if (iVar2 == -1) {
        return 1;
      }
    } while (iVar2 == iVar1);
  }
  return 0;
}

// Function: validate
void validate(long param_1,long param_2) {
  char cVar1;
  undefined8 uVar2;
  
  get_s1_spec_stats(param_1);
  if (*(long *)(param_1 + 0x20) != 0) {
    uVar2 = gettext("the [c*] repeat construct may not appear in string1");
    error(1,0,uVar2);
  }
  if (param_2 != 0) {
    get_s2_spec_stats(param_2,*(undefined8 *)(param_1 + 0x18));
    if (1 < *(ulong *)(param_2 + 0x20)) {
      uVar2 = gettext("only one [c*] repeat construct may appear in string2");
      error(1,0,uVar2);
    }
    if (translating != '\0') {
      if (*(char *)(param_2 + 0x30) != '\0') {
        uVar2 = gettext("[=c=] expressions may not appear in string2 when translating");
        error(1,0,uVar2);
      }
      if (*(char *)(param_2 + 0x32) != '\0') {
        uVar2 = gettext(
                       "when translating, the only character classes that may appear in\nstring2 are \'upper\' and \'lower\'"
                       );
        error(1,0,uVar2);
      }
      validate_case_classes(param_1,param_2);
      if ((*(ulong *)(param_2 + 0x18) < *(ulong *)(param_1 + 0x18)) && (truncate_set1 != '\x01')) {
        if (*(long *)(param_2 + 0x18) == 0) {
          uVar2 = gettext("when not truncating set1, string2 must be non-empty");
          error(1,0,uVar2);
        }
        string2_extend(param_1,param_2);
      }
      if (complement == '\0') {
        return;
      }
      if (*(char *)(param_1 + 0x31) == '\0') {
        return;
      }
      if ((*(long *)(param_2 + 0x18) == *(long *)(param_1 + 0x18)) &&
         (cVar1 = homogeneous_spec_list(param_2), cVar1 == '\x01')) {
        return;
      }
      uVar2 = gettext(
                     "when translating with complemented character classes,\nstring2 must map all characters in the domain to one"
                     );
      error(1,0,uVar2);
    }
    if (*(long *)(param_2 + 0x20) != 0) {
      uVar2 = gettext("the [c*] construct may appear in string2 only when translating");
      error(1,0,uVar2);
    }
  }
  return;
}

// Function: squeeze_filter
void squeeze_filter(long param_1,undefined8 param_2,code *param_3) {
  ulong uVar1;
  byte bVar2;
  size_t sVar3;
  int local_30;
  ulong local_28;
  ulong local_20;
  size_t local_18;
  
  local_30 = 0x7fffffff;
  local_28 = 0;
  local_20 = 0;
  do {
    if (local_20 <= local_28) {
      local_20 = (*param_3)(param_1,param_2);
      if (local_20 == 0) {
        return;
      }
      local_28 = 0;
    }
    uVar1 = local_28;
    if (local_30 == 0x7fffffff) {
      while ((local_28 < local_20 &&
             (bVar2 = to_uchar((int)*(char *)(local_28 + param_1)),
             in_squeeze_set[(int)(uint)bVar2] != '\x01'))) {
        local_28 = local_28 + 2;
      }
      if ((local_28 == local_20) &&
         (bVar2 = to_uchar((int)*(char *)(param_1 + (local_28 - 1))),
         in_squeeze_set[(int)(uint)bVar2] != '\0')) {
        local_28 = local_28 - 1;
      }
      if (local_28 < local_20) {
        local_30 = (int)*(char *)(local_28 + param_1);
        local_18 = (local_28 - uVar1) + 1;
        if ((local_28 != 0) && (local_30 == *(char *)(param_1 + (local_28 - 1)))) {
          local_18 = local_28 - uVar1;
        }
        local_28 = local_28 + 1;
      }
      else {
        local_18 = local_20 - uVar1;
      }
      if ((local_18 != 0) &&
         (sVar3 = fwrite_unlocked((void *)(param_1 + uVar1),1,local_18,_stdout), local_18 != sVar3))
      {
        write_error();
      }
    }
    if (local_30 != 0x7fffffff) {
      for (; (local_28 < local_20 && (local_30 == *(char *)(local_28 + param_1)));
          local_28 = local_28 + 1) {
      }
      if (local_28 < local_20) {
        local_30 = 0x7fffffff;
      }
    }
  } while( true );
}

// Function: plain_read
ssize_t plain_read(void *param_1,size_t param_2) {
  ssize_t sVar1;
  undefined8 uVar2;
  int *piVar3;
  
  sVar1 = read(0,param_1,param_2);
  if (sVar1 < 0) {
    uVar2 = gettext("read error");
    piVar3 = __errno_location();
    error(1,*piVar3,uVar2);
  }
  return sVar1;
}

// Function: read_and_delete
ulong read_and_delete(long param_1,undefined8 param_2) {
  byte bVar1;
  ulong uVar2;
  ulong local_20;
  ulong local_18;
  
  do {
    uVar2 = plain_read(param_1,param_2);
    if (uVar2 == 0) {
      return 0;
    }
    local_18 = 0;
    while ((local_18 < uVar2 &&
           (bVar1 = to_uchar((int)*(char *)(local_18 + param_1)),
           in_delete_set[(int)(uint)bVar1] != '\x01'))) {
      local_18 = local_18 + 1;
    }
    local_20 = local_18;
    while (local_18 = local_18 + 1, local_18 < uVar2) {
      bVar1 = to_uchar((int)*(char *)(local_18 + param_1));
      if (in_delete_set[(int)(uint)bVar1] != '\x01') {
        *(undefined *)(param_1 + local_20) = *(undefined *)(param_1 + local_18);
        local_20 = local_20 + 1;
      }
    }
  } while (local_20 == 0);
  return local_20;
}

// Function: read_and_xlate
ulong read_and_xlate(long param_1,undefined8 param_2) {
  byte bVar1;
  ulong uVar2;
  ulong local_18;
  
  uVar2 = plain_read(param_1,param_2);
  for (local_18 = 0; local_18 < uVar2; local_18 = local_18 + 1) {
    bVar1 = to_uchar((int)*(char *)(local_18 + param_1));
    *(undefined1 *)(local_18 + param_1) = xlate[(int)(uint)bVar1];
  }
  return uVar2;
}

// Function: set_initialize
void set_initialize(long param_1,char param_2,long param_3) {
  int iVar1;
  ulong local_10;
  
  *(undefined8 *)(param_1 + 0x10) = 0xfffffffffffffffe;
  while( true ) {
    iVar1 = get_next(param_1,0);
    if (iVar1 == -1) break;
    *(undefined *)(param_3 + iVar1) = 1;
  }
  if (param_2 != '\0') {
    for (local_10 = 0; local_10 < 0x100; local_10 = local_10 + 1) {
      *(bool *)(param_3 + local_10) = *(char *)(local_10 + param_3) == '\0';
    }
  }
  return;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  char cVar1;
  int iVar2;
  char *pcVar3;
  undefined8 uVar4;
  size_t sVar5;
  ushort **ppuVar6;
  int *piVar7;
  long in_FS_OFFSET;
  int local_138;
  int local_134;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  int local_11c;
  int local_118;
  int local_114;
  int local_110;
  int local_10c;
  int local_108;
  int local_104;
  undefined4 local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined4 local_f4;
  undefined4 local_f0;
  undefined4 local_ec;
  undefined4 local_e8;
  int local_e4;
  undefined *local_e0;
  undefined *local_d8;
  size_t local_d0;
  undefined1 *local_c8;
  size_t local_c0;
  undefined8 local_b8;
  undefined8 local_b0;
  undefined8 local_a8;
  undefined8 local_a0;
  undefined local_98 [64];
  undefined local_58 [56];
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  local_d8 = local_98;
  local_e0 = local_58;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
LAB_00103002:
  local_11c = getopt_long(param_1,param_2,"+AcCdst",long_options,0);
  if (local_11c == -1) goto code_r0x00103040;
  if (local_11c == 0x74) {
    truncate_set1 = '\x01';
    goto LAB_00103002;
  }
  if (local_11c < 0x75) {
    if (local_11c == 0x73) {
      squeeze_repeats = 1;
      goto LAB_00103002;
    }
    if (0x73 < local_11c) goto LAB_00102ff8;
    if (local_11c == 100) {
      delete = 1;
      goto LAB_00103002;
    }
    if (local_11c < 0x65) {
      if (local_11c == 99) {
LAB_00102f69:
        complement = '\x01';
        goto LAB_00103002;
      }
      if (local_11c < 100) {
        if (local_11c == 0x43) goto LAB_00102f69;
        if (local_11c < 0x44) {
          if (local_11c == 0x41) {
            setlocale(3,"C");
            setlocale(0,"C");
            goto LAB_00103002;
          }
          if (local_11c < 0x42) {
            if (local_11c == -0x83) {
LAB_00102f9d:
              uVar4 = proper_name_lite("Jim Meyering","Jim Meyering");
              version_etc(_stdout,&DAT_00106873,"GNU coreutils",_Version,uVar4,0);
                    /* WARNING: Subroutine does not return */
              exit(0);
            }
            if (local_11c == -0x82) {
              usage(0);
              goto LAB_00102f9d;
            }
          }
        }
      }
    }
  }
LAB_00102ff8:
  usage(1);
  goto LAB_00103002;
LAB_00103886:
  if ((local_10c != -1) && (truncate_set1 == '\0')) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("c1 == -1 || truncate_set1",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tr.c",
                  0x751,(char *)&__PRETTY_FUNCTION___0);
  }
  goto LAB_001038c3;
code_r0x00103040:
  local_118 = param_1 - _optind;
  if ((local_118 == 2) && (delete != 1)) {
    translating = '\x01';
  }
  else {
    translating = '\0';
  }
  if (delete == squeeze_repeats) {
    local_114 = 2;
  }
  else {
    local_114 = 1;
  }
  if (squeeze_repeats < delete) {
    local_110 = 1;
  }
  else {
    local_110 = 2;
  }
  if (local_118 < local_114) {
    if (local_118 == 0) {
      local_b8 = gettext("missing operand");
      local_100 = 0;
      local_fc = 0;
      error(0,0,local_b8);
    }
    else {
      uVar4 = quote(param_2[(long)param_1 + -1]);
      local_b0 = gettext("missing operand after %s");
      local_f8 = 0;
      local_f4 = 0;
      error(0,0,local_b0,uVar4);
      if (squeeze_repeats == 0) {
        pcVar3 = "Two strings must be given when translating.";
      }
      else {
        pcVar3 = "Two strings must be given when both deleting and squeezing repeats.";
      }
      uVar4 = gettext(pcVar3);
      fprintf(_stderr,"%s\n",uVar4);
    }
    usage(1);
  }
  if (local_110 < local_118) {
    uVar4 = quote(param_2[local_110 + _optind]);
    local_a8 = gettext("extra operand %s");
    local_f0 = 0;
    local_ec = 0;
    error(0,0,local_a8,uVar4);
    if (local_118 == 2) {
      uVar4 = gettext("Only one string may be given when deleting without squeezing repeats.");
      fprintf(_stderr,"%s\n",uVar4);
    }
    usage(1);
  }
  spec_init(local_d8);
  cVar1 = parse_str(param_2[_optind],local_d8);
  if (cVar1 != '\x01') {
                    /* WARNING: Subroutine does not return */
    exit(1);
  }
  if (local_118 == 2) {
    spec_init(local_e0);
    cVar1 = parse_str(param_2[(long)_optind + 1],local_e0);
    if (cVar1 != '\x01') {
                    /* WARNING: Subroutine does not return */
      exit(1);
    }
  }
  else {
    local_e0 = (undefined *)0x0;
  }
  validate(local_d8,local_e0);
  xset_binary_mode(0,0);
  xset_binary_mode(1,0);
  fadvise(_stdin,2);
  if ((squeeze_repeats != 0) && (local_118 == 1)) {
    set_initialize(local_d8,complement,in_squeeze_set);
    squeeze_filter(io_buf,0x2000,plain_read);
    goto LAB_00103967;
  }
  if ((delete != 0) && (local_118 == 1)) {
    set_initialize(local_d8,complement,in_delete_set);
    while (local_d0 = read_and_delete(io_buf,0x2000), local_d0 != 0) {
      sVar5 = fwrite_unlocked(io_buf,1,local_d0,_stdout);
      if (local_d0 != sVar5) {
        write_error();
      }
    }
    local_d0 = 0;
    goto LAB_00103967;
  }
  if (((squeeze_repeats != 0) && (delete != 0)) && (local_118 == 2)) {
    set_initialize(local_d8,complement,in_delete_set);
    set_initialize(local_e0,0,in_squeeze_set);
    squeeze_filter(io_buf,0x2000,read_and_delete);
    goto LAB_00103967;
  }
  if (translating == '\0') goto LAB_00103967;
  if (complement == '\0') {
    for (local_128 = 0; local_128 < 0x100; local_128 = local_128 + 1) {
      xlate[local_128] = (char)local_128;
    }
    *(undefined8 *)(local_d8 + 0x10) = 0xfffffffffffffffe;
    *(undefined8 *)(local_e0 + 0x10) = 0xfffffffffffffffe;
    do {
      local_10c = get_next(local_d8,&local_138);
      local_108 = get_next(local_e0,&local_134);
      if ((local_138 == 0) && (local_134 == 1)) {
        for (local_124 = 0; local_124 < 0x100; local_124 = local_124 + 1) {
          ppuVar6 = __ctype_b_loc();
          if (((*ppuVar6)[local_124] & 0x200) != 0) {
            iVar2 = toupper(local_124);
            xlate[local_124] = (char)iVar2;
          }
        }
      }
      else if ((local_138 == 1) && (local_134 == 0)) {
        for (local_120 = 0; local_120 < 0x100; local_120 = local_120 + 1) {
          ppuVar6 = __ctype_b_loc();
          if (((*ppuVar6)[local_120] & 0x100) != 0) {
            iVar2 = tolower(local_120);
            xlate[local_120] = (char)iVar2;
          }
        }
      }
      else {
        if ((local_10c == -1) || (local_108 == -1)) goto LAB_00103886;
        xlate[local_10c] = (char)local_108;
      }
      if (local_134 != 2) {
        skip_construct(local_d8);
        skip_construct(local_e0);
      }
    } while( true );
  }
  local_c8 = in_delete_set;
  set_initialize(local_d8,0,in_delete_set);
  *(undefined8 *)(local_e0 + 0x10) = 0xfffffffffffffffe;
  for (local_130 = 0; local_130 < 0x100; local_130 = local_130 + 1) {
    xlate[local_130] = (char)local_130;
  }
  for (local_12c = 0; local_12c < 0x100; local_12c = local_12c + 1) {
    if (local_c8[local_12c] != '\x01') {
      local_104 = get_next(local_e0,0);
      if ((local_104 == -1) && (truncate_set1 == '\0')) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("ch != -1 || truncate_set1",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/tr.c",
                      0x71c,(char *)&__PRETTY_FUNCTION___0);
      }
      if (local_104 == -1) break;
      xlate[local_12c] = (char)local_104;
    }
  }
LAB_001038c3:
  if (squeeze_repeats == 0) {
    while (local_c0 = read_and_xlate(io_buf,0x2000), local_c0 != 0) {
      sVar5 = fwrite_unlocked(io_buf,1,local_c0,_stdout);
      if (local_c0 != sVar5) {
        write_error();
      }
    }
  }
  else {
    set_initialize(local_e0,0,in_squeeze_set);
    squeeze_filter(io_buf,0x2000,read_and_xlate);
  }
LAB_00103967:
  iVar2 = close(0);
  if (iVar2 != 0) {
    uVar4 = gettext("standard input");
    piVar7 = __errno_location();
    local_e4 = *piVar7;
    local_e8 = 1;
    local_a0 = uVar4;
    error(1,local_e4,uVar4);
  }
                    /* WARNING: Subroutine does not return */
  exit(0);
}

