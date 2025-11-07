// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... FILE1 FILE2\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "For each pair of input lines with identical join fields, write a line to\nstandard output.  The default join field is the first, delimited by blanks.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("\nWhen FILE1 or FILE2 (not both) is -, read standard input.\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n  -a FILENUM             also print unpairable lines from file FILENUM, where\n                           FILENUM is 1 or 2, corresponding to FILE1 or FILE2\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -e STRING              replace missing (empty) input fields with STRING;\n                           I.e., missing fields specified with \'-12jo\' options\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -i, --ignore-case      ignore differences in case when comparing fields\n  -j FIELD               equivalent to \'-1 FIELD -2 FIELD\'\n  -o FORMAT              obey FORMAT while constructing output line\n  -t CHAR                use CHAR as input and output field separator\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -v FILENUM             like -a FILENUM, but suppress joined output lines\n  -1 FIELD               join on this FIELD of file 1\n  -2 FIELD               join on this FIELD of file 2\n      --check-order      check that the input is correctly sorted, even\n                           if all input lines are pairable\n      --nocheck-order    do not check that the input is correctly sorted\n      --header           treat the first line in each file as field headers,\n                           print them without trying to pair them\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -z, --zero-terminated  line delimiter is NUL, not newline\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nUnless -t CHAR is given, leading blanks separate fields and are ignored,\nelse fields are separated by CHAR.  Any FIELD is a field number counted\nfrom 1.  FORMAT is one or more comma or blank separated specifications,\neach being \'FILENUM.FIELD\' or \'0\'.  Default FORMAT outputs the join field,\nthe remaining fields from FILE1, the remaining fields from FILE2, all\nseparated by CHAR.  If FORMAT is the keyword \'auto\', then the first\nline of each file determines the number of fields output for each line.\n\nImportant: FILE1 and FILE2 must be sorted on the join fields.\nE.g., use \"sort -k 1b,1\" if \'join\' has no options,\nor use \"join -t \'\'\" if \'sort\' has no options.\nComparisons honor the rules specified by \'LC_COLLATE\'.\nIf the input is not sorted and some lines cannot be joined, a\nwarning message will be given.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_00103884);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: extract_field
void extract_field(long param_1,undefined8 param_2,undefined8 param_3) {
  undefined8 uVar1;
  
  if (*(long *)(param_1 + 0x20) <= *(long *)(param_1 + 0x18)) {
    uVar1 = xpalloc(*(undefined8 *)(param_1 + 0x28),param_1 + 0x20,1,0xffffffffffffffff,0x10);
    *(undefined8 *)(param_1 + 0x28) = uVar1;
  }
  *(undefined8 *)(*(long *)(param_1 + 0x28) + *(long *)(param_1 + 0x18) * 0x10) = param_2;
  *(undefined8 *)(*(long *)(param_1 + 0x28) + *(long *)(param_1 + 0x18) * 0x10 + 8) = param_3;
  *(long *)(param_1 + 0x18) = *(long *)(param_1 + 0x18) + 1;
  return;
}

// Function: eq_tab
bool eq_tab(undefined8 param_1) {
  int iVar1;
  
  iVar1 = mcel_cmp(param_1,tab);
  return iVar1 == 0;
}

// Function: newline_or_blank
undefined4 newline_or_blank(int param_1) {
  int iVar1;
  
  if ((param_1 != 10) && (iVar1 = c32isblank(param_1), iVar1 == 0)) {
    return 0;
  }
  return 1;
}

// Function: xfields
void xfields(long param_1) {
  ulong uVar1;
  ulong uVar2;
  ulong uVar3;
  ulong local_28;
  
  local_28 = *(ulong *)(param_1 + 0x10);
  uVar1 = local_28 + *(long *)(param_1 + 8) + -1;
  if (local_28 != uVar1) {
    if (tab._5_1_ == '\0') {
      while (uVar2 = skip_buf_matching(local_28,uVar1,newline_or_blank,1), uVar2 < uVar1) {
        local_28 = skip_buf_matching(uVar2,uVar1,newline_or_blank,0);
        extract_field(param_1,uVar2,local_28 - uVar2);
      }
    }
    else {
      if ((int)tab != 10) {
        while (uVar2 = skip_buf_matching(local_28,uVar1,eq_tab,0), uVar2 < uVar1) {
          extract_field(param_1,local_28,uVar2 - local_28);
          uVar3 = mcel_scan(uVar2,uVar1);
          local_28 = uVar2 + (uVar3 >> 0x28 & 0xff);
        }
      }
      extract_field(param_1,local_28,uVar1 - local_28);
    }
  }
  return;
}

// Function: freeline
void freeline(long param_1) {
  if (param_1 != 0) {
    free(*(void **)(param_1 + 0x28));
    *(undefined8 *)(param_1 + 0x28) = 0;
    free(*(void **)(param_1 + 0x10));
    *(undefined8 *)(param_1 + 0x10) = 0;
  }
  return;
}

// Function: keycmp
ulong keycmp(long param_1,long param_2,long param_3,long param_4) {
  ulong uVar1;
  size_t sVar2;
  uint local_2c;
  void *local_28;
  void *local_20;
  size_t local_18;
  size_t local_10;
  
  if (param_3 < *(long *)(param_1 + 0x18)) {
    local_28 = *(void **)(*(long *)(param_1 + 0x28) + param_3 * 0x10);
    local_18 = *(size_t *)(*(long *)(param_1 + 0x28) + param_3 * 0x10 + 8);
  }
  else {
    local_28 = (void *)0x0;
    local_18 = 0;
  }
  if (param_4 < *(long *)(param_2 + 0x18)) {
    local_20 = *(void **)(*(long *)(param_2 + 0x28) + param_4 * 0x10);
    local_10 = *(size_t *)(*(long *)(param_2 + 0x28) + param_4 * 0x10 + 8);
  }
  else {
    local_20 = (void *)0x0;
    local_10 = 0;
  }
  if (local_18 == 0) {
    if (local_10 == 0) {
      uVar1 = 0;
    }
    else {
      uVar1 = 0xffffffff;
    }
  }
  else if (local_10 == 0) {
    uVar1 = 1;
  }
  else {
    if (ignore_case == '\0') {
      if (hard_LC_COLLATE != '\0') {
        uVar1 = xmemcoll(local_28,local_18,local_20,local_10);
        return uVar1;
      }
      sVar2 = local_18;
      if ((long)local_10 <= (long)local_18) {
        sVar2 = local_10;
      }
      local_2c = memcmp(local_28,local_20,sVar2);
    }
    else {
      sVar2 = local_18;
      if ((long)local_10 <= (long)local_18) {
        sVar2 = local_10;
      }
      local_2c = memcasecmp(local_28,local_20,sVar2);
    }
    if (local_2c == 0) {
      uVar1 = (ulong)((uint)((long)local_10 < (long)local_18) -
                     (uint)((long)local_18 < (long)local_10));
    }
    else {
      uVar1 = (ulong)local_2c;
    }
  }
  return uVar1;
}

// Function: check_order
void check_order(undefined8 param_1,long param_2,int param_3) {
  undefined8 uVar1;
  undefined8 uVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  bool bVar6;
  ulong local_40;
  
  if ((check_input_order != 2) &&
     (((check_input_order == 1 || (seen_unpairable != '\0')) &&
      ((&issued_disorder_warning)[param_3 + -1] != '\x01')))) {
    uVar4 = join_field_2;
    if (param_3 == 1) {
      uVar4 = join_field_1;
    }
    iVar3 = keycmp(param_1,param_2,uVar4,uVar4);
    if (0 < iVar3) {
      local_40 = *(ulong *)(param_2 + 8);
      if ((0 < (long)local_40) && (*(char *)(*(long *)(param_2 + 0x10) + (local_40 - 1)) == '\n')) {
        local_40 = local_40 - 1;
      }
      if (0x7fffffff < (long)local_40) {
        local_40 = 0x7fffffff;
      }
      bVar6 = check_input_order == 1;
      uVar4 = *(undefined8 *)(param_2 + 0x10);
      uVar1 = *(undefined8 *)(line_no + (long)(param_3 + -1) * 8);
      uVar2 = (&g_names)[param_3 + -1];
      uVar5 = gettext("%s:%ju: is not sorted: %.*s");
      error(bVar6,0,uVar5,uVar2,uVar1,local_40 & 0xffffffff,uVar4);
      (&issued_disorder_warning)[param_3 + -1] = 1;
    }
  }
  return;
}

// Function: reset_line
void reset_line(long param_1) {
  *(undefined8 *)(param_1 + 0x18) = 0;
  return;
}

// Function: init_linep
undefined8 init_linep(undefined8 *param_1) {
  undefined8 uVar1;
  
  uVar1 = xzalloc(0x30);
  *param_1 = uVar1;
  return uVar1;
}

// Function: get_line
undefined8 get_line(FILE *param_1,long *param_2,int param_3) {
  int iVar1;
  long lVar2;
  undefined8 uVar3;
  int *piVar4;
  long local_30;
  
  lVar2 = *param_2;
  local_30 = lVar2;
  if (lVar2 == *(long *)(prevline + (long)(param_3 + -1) * 8)) {
    local_30 = *(long *)(spareline + (long)(param_3 + -1) * 8);
    *(long *)(spareline + (long)(param_3 + -1) * 8) = lVar2;
    *param_2 = local_30;
  }
  if (local_30 == 0) {
    local_30 = init_linep(param_2);
  }
  else {
    reset_line(local_30);
  }
  lVar2 = readlinebuffer_delim(local_30,param_1,(int)eolchar);
  if (lVar2 == 0) {
    iVar1 = ferror_unlocked(param_1);
    if (iVar1 != 0) {
      uVar3 = gettext("read error");
      piVar4 = __errno_location();
      error(1,*piVar4,uVar3);
    }
    freeline(local_30);
    uVar3 = 0;
  }
  else {
    *(long *)(line_no + (long)(param_3 + -1) * 8) =
         *(long *)(line_no + (long)(param_3 + -1) * 8) + 1;
    xfields(local_30);
    if (*(long *)(prevline + (long)(param_3 + -1) * 8) != 0) {
      check_order(*(undefined8 *)(prevline + (long)(param_3 + -1) * 8),local_30,param_3);
    }
    *(long *)(prevline + (long)(param_3 + -1) * 8) = local_30;
    uVar3 = 1;
  }
  return uVar3;
}

// Function: free_spareline
void free_spareline(void) {
  ulong local_10;
  
  for (local_10 = 0; local_10 < 2; local_10 = local_10 + 1) {
    if (*(long *)(spareline + local_10 * 8) != 0) {
      freeline(*(undefined8 *)(spareline + local_10 * 8));
      free(*(void **)(spareline + local_10 * 8));
    }
  }
  return;
}

// Function: initseq
void initseq(undefined8 *param_1) {
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  return;
}

// Function: getseq
bool getseq(undefined8 param_1,long *param_2,undefined4 param_3) {
  char cVar1;
  long lVar2;
  long local_10;
  
  if (*param_2 == param_2[1]) {
    lVar2 = xpalloc(param_2[2],param_2 + 1,1,0xffffffffffffffff,8);
    param_2[2] = lVar2;
    for (local_10 = *param_2; local_10 < param_2[1]; local_10 = local_10 + 1) {
      *(undefined8 *)(param_2[2] + local_10 * 8) = 0;
    }
  }
  cVar1 = get_line(param_1,param_2[2] + *param_2 * 8,param_3);
  if (cVar1 != '\0') {
    *param_2 = *param_2 + 1;
  }
  return cVar1 != '\0';
}

// Function: advance_seq
void advance_seq(undefined8 param_1,undefined8 *param_2,char param_3,undefined4 param_4) {
  if (param_3 != '\0') {
    *param_2 = 0;
  }
  getseq(param_1,param_2,param_4);
  return;
}

// Function: delseq
void delseq(long param_1) {
  long local_10;
  
  for (local_10 = 0; local_10 < *(long *)(param_1 + 8); local_10 = local_10 + 1) {
    freeline(*(undefined8 *)(*(long *)(param_1 + 0x10) + local_10 * 8));
    free(*(void **)(*(long *)(param_1 + 0x10) + local_10 * 8));
  }
  free(*(void **)(param_1 + 0x10));
  return;
}

// Function: prfield
void prfield(long param_1,long param_2) {
  size_t __n;
  
  if (param_1 < *(long *)(param_2 + 0x18)) {
    __n = *(size_t *)(*(long *)(param_2 + 0x28) + param_1 * 0x10 + 8);
    if (__n == 0) {
      if (empty_filler != (char *)0x0) {
        fputs_unlocked(empty_filler,_stdout);
      }
    }
    else {
      fwrite_unlocked(*(void **)(*(long *)(param_2 + 0x28) + param_1 * 0x10),1,__n,_stdout);
    }
  }
  else if (empty_filler != (char *)0x0) {
    fputs_unlocked(empty_filler,_stdout);
  }
  return;
}

// Function: prfields
void prfields(long param_1,long param_2,long param_3) {
  long local_18;
  
  if (autoformat == '\0') {
    param_3 = *(long *)(param_1 + 0x18);
  }
  for (local_18 = 0; (local_18 < param_2 && (local_18 < param_3)); local_18 = local_18 + 1) {
    fwrite_unlocked(output_separator,1,output_seplen,_stdout);
    prfield(local_18,param_1);
  }
  for (local_18 = param_2 + 1; local_18 < param_3; local_18 = local_18 + 1) {
    fwrite_unlocked(output_separator,1,output_seplen,_stdout);
    prfield(local_18,param_1);
  }
  return;
}

// Function: prjoin
void prjoin(undefined1 *param_1,undefined1 *param_2) {
  int iVar1;
  undefined8 local_28;
  undefined1 *local_20;
  int *local_18;
  
  if (outlist_head._16_8_ == 0) {
    if (param_1 == uni_blank) {
      local_28 = join_field_2;
      local_20 = param_2;
    }
    else {
      local_28 = join_field_1;
      local_20 = param_1;
    }
    prfield(local_28,local_20);
    prfields(param_1,join_field_1,autocount_1);
    prfields(param_2,join_field_2,autocount_2);
    putchar_unlocked((int)eolchar);
  }
  else {
    local_18 = (int *)outlist_head._16_8_;
    while( true ) {
      if (*local_18 == 0) {
        if (param_1 == uni_blank) {
          local_28 = join_field_2;
          local_20 = param_2;
        }
        else {
          local_28 = join_field_1;
          local_20 = param_1;
        }
      }
      else {
        local_20 = param_2;
        if (*local_18 == 1) {
          local_20 = param_1;
        }
        local_28 = *(undefined8 *)(local_18 + 2);
      }
      prfield(local_28,local_20);
      local_18 = *(int **)(local_18 + 4);
      if (local_18 == (int *)0x0) break;
      fwrite_unlocked(output_separator,1,output_seplen,_stdout);
    }
    putchar_unlocked((int)eolchar);
  }
  iVar1 = ferror_unlocked(_stdout);
  if (iVar1 != 0) {
    write_error();
  }
  return;
}

// Function: add_field
void add_field(int param_1,long param_2) {
  int *piVar1;
  
  if (((param_1 != 0) && (param_1 != 1)) && (param_1 != 2)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("file == 0 || file == 1 || file == 2",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/join.c",
                  0x339,"add_field");
  }
  if ((param_1 == 0) && (param_2 != 0)) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("file != 0 || field == 0",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/join.c",
                  0x33a,"add_field");
  }
  piVar1 = (int *)xmalloc(0x18);
  *piVar1 = param_1;
  *(long *)(piVar1 + 2) = param_2;
  *(undefined8 *)(piVar1 + 4) = 0;
  *(int **)(outlist_end + 0x10) = piVar1;
  outlist_end = (undefined *)piVar1;
  return;
}

// Function: string_to_join_field
long string_to_join_field(undefined8 param_1) {
  int iVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  long local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  iVar1 = xstrtoimax(param_1,0,10,&local_30,&DAT_00102e84);
  if (iVar1 == 1) {
    local_30 = 0x7fffffffffffffff;
  }
  else if ((iVar1 != 0) || (local_30 < 1)) {
    uVar2 = quote(param_1);
    local_28 = gettext("invalid field number: %s");
    error(1,0,local_28,uVar2);
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_30 + -1;
}

// Function: decode_field_spec
void decode_field_spec(char *param_1,int *param_2,undefined8 *param_3) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  
  iVar1 = (int)*param_1;
  if (iVar1 == 0x30) {
    if (param_1[1] != '\0') {
      uVar2 = quote(param_1);
      uVar3 = gettext("invalid field specifier: %s");
      error(1,0,uVar3,uVar2);
    }
    *param_2 = 0;
    *param_3 = 0;
  }
  else if ((iVar1 < 0x30) || (1 < iVar1 - 0x31U)) {
    uVar2 = quote(param_1);
    uVar3 = gettext("invalid file number in field spec: %s");
    error(1,0,uVar3,uVar2);
  }
  else {
    if (param_1[1] != '.') {
      uVar2 = quote(param_1);
      uVar3 = gettext("invalid field specifier: %s");
      error(1,0,uVar3,uVar2);
    }
    *param_2 = *param_1 + -0x30;
    uVar2 = string_to_join_field(param_1 + 2);
    *param_3 = uVar2;
  }
  return;
}

// Function: comma_or_blank
undefined4 comma_or_blank(int param_1) {
  int iVar1;
  
  if ((param_1 != 0x2c) && (iVar1 = c32isblank(param_1), iVar1 == 0)) {
    return 0;
  }
  return 1;
}

// Function: add_field_list
void add_field_list(char *param_1) {
  long in_FS_OFFSET;
  undefined4 local_2c;
  char *local_28;
  char *local_20;
  ulong local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_28 = param_1;
  do {
    local_20 = local_28;
    local_28 = (char *)skip_str_matching(local_28,comma_or_blank,0);
    if (*local_28 != '\0') {
      local_18 = mcel_scanz(local_28);
      *local_28 = '\0';
      local_28 = local_28 + (local_18 >> 0x28 & 0xff);
    }
    decode_field_spec(local_20,&local_2c,&local_18);
    add_field(local_2c,local_18);
  } while (*local_28 != '\0');
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: set_join_field
void set_join_field(long *param_1,long param_2) {
  long lVar1;
  undefined8 uVar2;
  
  if ((-1 < *param_1) && (param_2 != *param_1)) {
    lVar1 = *param_1;
    uVar2 = gettext("incompatible join fields %td, %td");
    error(1,0,uVar2,lVar1,param_2);
  }
  *param_1 = param_2;
  return;
}

// Function: add_file_name
void add_file_name(undefined8 param_1,undefined8 *param_2,int *param_3,int *param_4,int *param_5,
                  int *param_6,undefined4 *param_7) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  bool bVar5;
  int local_34;
  
  local_34 = *param_5;
  if (local_34 != 2) goto LAB_00101fdc;
  bVar5 = *param_3 == 0;
  uVar4 = param_2[bVar5];
  iVar1 = param_3[bVar5];
  if (iVar1 == 3) {
    add_field_list(uVar4);
  }
  else if (iVar1 < 4) {
    if (iVar1 == 2) {
      param_4[1] = param_4[1] + -1;
      uVar4 = string_to_join_field(uVar4);
      set_join_field(&join_field_2,uVar4);
    }
    else if (iVar1 < 3) {
      if (iVar1 == 0) {
        uVar2 = quotearg_style(4,param_1);
        uVar3 = gettext("extra operand %s");
        error(0,0,uVar3,uVar2);
        usage(1);
      }
      else if (iVar1 != 1) goto LAB_00101fae;
      *param_4 = *param_4 + -1;
      uVar4 = string_to_join_field(uVar4);
      set_join_field(&join_field_1,uVar4);
    }
  }
LAB_00101fae:
  if (!bVar5) {
    *param_3 = param_3[1];
    *param_2 = param_2[1];
  }
  local_34 = 1;
LAB_00101fdc:
  param_3[local_34] = *param_6;
  param_2[local_34] = param_1;
  *param_5 = local_34 + 1;
  if (*param_6 == 3) {
    *param_7 = 3;
  }
  return;
}

// Function: main
undefined8 main(int param_1,undefined8 *param_2) {
  char cVar1;
  uint uVar2;
  int iVar3;
  long lVar4;
  undefined8 uVar5;
  int *piVar6;
  long in_FS_OFFSET;
  int local_11c;
  int local_118;
  int local_114;
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
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  int local_d4;
  undefined4 local_d0;
  int local_cc;
  undefined4 local_c8;
  int local_c4;
  undefined4 local_c0;
  int local_bc;
  undefined4 local_b8;
  int local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  long local_a8;
  long local_a0;
  undefined8 local_98;
  undefined8 local_90;
  undefined8 local_88;
  undefined8 local_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined *local_68;
  undefined *local_60;
  undefined8 local_58;
  undefined *local_50;
  undefined *local_48;
  undefined8 local_40;
  ulong local_38;
  undefined local_30 [8];
  int local_28 [2];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_118 = 0;
  local_28[0] = 0;
  local_28[1] = 0;
  local_114 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  hard_LC_COLLATE = hard_locale(3);
  atexit((__func *)&close_stdout);
  atexit(free_spareline);
  print_pairables = 1;
  seen_unpairable = 0;
  DAT_00102ce6 = '\0';
  issued_disorder_warning = '\0';
  check_input_order = 0;
  do {
    local_10c = getopt_long(param_1,param_2,"-a:e:i1:2:j:o:t:v:z",longopts,0);
    if (local_10c == -1) {
      local_118 = 0;
      local_10c = 0xffffffff;
      while (_optind < param_1) {
        lVar4 = (long)_optind;
        _optind = _optind + 1;
        add_file_name(param_2[lVar4],&g_names,local_30,local_28,&local_114,&local_118,&local_11c);
      }
      if (local_114 != 2) {
        if (local_114 == 0) {
          local_78 = gettext("missing operand");
          local_e8 = 0;
          local_e4 = 0;
          error(0,0,local_78);
        }
        else {
          uVar5 = quote(param_2[(long)param_1 + -1]);
          local_70 = gettext("missing operand after %s");
          local_e0 = 0;
          local_dc = 0;
          error(0,0,local_70,uVar5);
        }
        usage(1);
      }
      for (local_110 = 0; local_110 < 2; local_110 = local_110 + 1) {
        if (local_28[local_110] != 0) {
          set_join_field(&join_field_1,(long)local_110);
          set_join_field(&join_field_2,(long)local_110);
        }
      }
      if (join_field_1 < 0) {
        join_field_1 = 0;
      }
      if (join_field_2 < 0) {
        join_field_2 = 0;
      }
      cVar1 = streq(g_names,&DAT_00103abd);
      local_a8 = _stdin;
      if (cVar1 == '\0') {
        local_a8 = fopen_safer(g_names,&DAT_00103abf);
      }
      if (local_a8 == 0) {
        uVar5 = quotearg_n_style_colon(0,3,g_names);
        piVar6 = __errno_location();
        local_d4 = *piVar6;
        local_d8 = 1;
        local_68 = &DAT_00103ac1;
        error(1,local_d4,&DAT_00103ac1,uVar5);
      }
      cVar1 = streq(DAT_00102cc8,&DAT_00103abd);
      local_a0 = _stdin;
      if (cVar1 == '\0') {
        local_a0 = fopen_safer(DAT_00102cc8,&DAT_00103abf);
      }
      if (local_a0 == 0) {
        uVar5 = quotearg_n_style_colon(0,3,DAT_00102cc8);
        piVar6 = __errno_location();
        local_cc = *piVar6;
        local_d0 = 1;
        local_60 = &DAT_00103ac1;
        error(1,local_cc,&DAT_00103ac1,uVar5);
      }
      if (local_a8 == local_a0) {
        uVar5 = gettext("both files cannot be standard input");
        piVar6 = __errno_location();
        local_c4 = *piVar6;
        local_c8 = 1;
        local_58 = uVar5;
        error(1,local_c4,uVar5);
      }
      system_join(local_a8,local_a0);
      iVar3 = rpl_fclose(local_a8);
      if (iVar3 != 0) {
        uVar5 = quotearg_n_style_colon(0,3,g_names);
        piVar6 = __errno_location();
        local_bc = *piVar6;
        local_c0 = 1;
        local_50 = &DAT_00103ac1;
        error(1,local_bc,&DAT_00103ac1,uVar5);
      }
      iVar3 = rpl_fclose(local_a0);
      if (iVar3 != 0) {
        uVar5 = quotearg_n_style_colon(0,3,DAT_00102cc8);
        piVar6 = __errno_location();
        local_b4 = *piVar6;
        local_b8 = 1;
        local_48 = &DAT_00103ac1;
        error(1,local_b4,&DAT_00103ac1,uVar5);
      }
      if ((issued_disorder_warning != '\0') || (DAT_00102ce6 != '\0')) {
        local_40 = gettext("input is not in sorted order");
        local_b0 = 1;
        local_ac = 0;
        error(1,0,local_40);
      }
      if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return 0;
    }
    local_11c = 0;
    if (local_10c < 0x83) {
      if (local_10c < 0x31) {
        if (local_10c != 1) {
          if (local_10c < 2) {
            if (local_10c == -0x83) {
LAB_001026a3:
              uVar5 = proper_name_lite("Mike Haertel","Mike Haertel");
              version_etc(_stdout,&DAT_00103884,"GNU coreutils",_Version,uVar5,0);
                    /* WARNING: Subroutine does not return */
              exit(0);
            }
            if (local_10c == -0x82) {
              usage(0);
              goto LAB_001026a3;
            }
          }
          goto switchD_001021ca_caseD_33;
        }
        add_file_name(_optarg,&g_names,local_30,local_28,&local_114,&local_118,&local_11c);
      }
      else {
        switch(local_10c) {
        case 0x31:
          uVar5 = string_to_join_field(_optarg);
          set_join_field(&join_field_1,uVar5);
          break;
        case 0x32:
          uVar5 = string_to_join_field(_optarg);
          set_join_field(&join_field_2,uVar5);
          break;
        default:
          goto switchD_001021ca_caseD_33;
        case 0x65:
          if ((empty_filler != (char *)0x0) &&
             (cVar1 = streq(empty_filler,_optarg), cVar1 != '\x01')) {
            local_90 = gettext("conflicting empty-field replacement strings");
            local_100 = 1;
            local_fc = 0;
            error(1,0,local_90);
          }
          empty_filler = _optarg;
          break;
        case 0x69:
          ignore_case = 1;
          break;
        case 0x6a:
          if ((((*_optarg == '1') || (*_optarg == '2')) && (_optarg[1] == '\0')) &&
             ((char *)(param_2[(long)_optind + -1] + 2) == _optarg)) {
            cVar1 = *_optarg;
            uVar2 = (uint)(cVar1 == '2');
            local_28[(int)uVar2] = local_28[(int)uVar2] + 1;
            local_11c = (cVar1 == '2') + 1;
          }
          else {
            uVar5 = string_to_join_field(_optarg);
            set_join_field(&join_field_1,uVar5);
            set_join_field(&join_field_2,join_field_1);
          }
          break;
        case 0x6f:
          cVar1 = streq(_optarg,&DAT_00103a34);
          if (cVar1 == '\0') {
            add_field_list(_optarg);
            local_11c = 3;
          }
          else {
            autoformat = 1;
          }
          break;
        case 0x74:
          if (*_optarg == '\0') {
            local_38 = mcel_ch(10,1);
          }
          else {
            cVar1 = streq(_optarg,&DAT_00103a39);
            if (cVar1 == '\0') {
              local_38 = mcel_scanz(_optarg);
              if (_optarg[local_38 >> 0x28 & 0xff] != '\0') {
                uVar5 = quote(_optarg);
                local_88 = gettext("multi-character tab %s");
                local_f8 = 1;
                local_f4 = 0;
                error(1,0,local_88,uVar5);
              }
              output_separator = _optarg;
            }
            else {
              local_38 = mcel_ch(0,1);
              output_separator = &DAT_00102e84;
            }
          }
          if ((tab._5_1_ != '\0') && (iVar3 = mcel_cmp(tab,local_38), iVar3 != 0)) {
            local_80 = gettext("incompatible tabs");
            local_f0 = 1;
            local_ec = 0;
            error(1,0,local_80);
          }
          tab = local_38;
          output_seplen = local_38 >> 0x28 & 0xff;
          break;
        case 0x76:
          print_pairables = 0;
        case 0x61:
          iVar3 = xstrtol(_optarg,0,10,&local_38,&DAT_00102e84);
          if ((iVar3 != 0) || ((local_38 != 1 && (local_38 != 2)))) {
            uVar5 = quote(_optarg);
            local_98 = gettext("invalid file number: %s");
            local_108 = 1;
            local_104 = 0;
            error(1,0,local_98,uVar5);
          }
          if (local_38 == 1) {
            print_unpairables_1 = 1;
          }
          else {
            print_unpairables_2 = 1;
          }
          break;
        case 0x7a:
          eolchar = 0;
          break;
        case 0x80:
          check_input_order = 1;
          break;
        case 0x81:
          check_input_order = 2;
          break;
        case 0x82:
          join_header_lines = 1;
        }
      }
    }
    else {
switchD_001021ca_caseD_33:
      usage(1);
    }
    local_118 = local_11c;
  } while( true );
}

