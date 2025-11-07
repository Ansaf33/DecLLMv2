// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... [FILE]...\n  or:  %s [-abcdfilosx]... [FILE] [[+]OFFSET[.][b]]\n  or:  %s --traditional [OPTION]... [FILE] [[+]OFFSET[.][b] [+][LABEL][.][b]]\n"
                            );
    printf(pcVar3,uVar2,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nWrite an unambiguous representation, octal bytes by default,\nof FILE to standard output.  With more than one FILE argument,\nconcatenate them in the listed order to form the input.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nIf first and second call formats both apply, the second format is assumed\nif the last operand begins with + or (if there are 2 operands) a digit.\nAn OFFSET operand means -j OFFSET.  LABEL is the pseudo-address\nat first byte printed, incremented when dump is progressing.\nFor OFFSET and LABEL, a 0x or 0X prefix indicates hexadecimal;\nsuffixes may be . for octal and b for multiply by 512.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -A, --address-radix=RADIX   output format for file offsets; RADIX is one\n                                of [doxn], for Decimal, Octal, Hex or None\n      --endian={big|little}   swap input bytes according the specified order\n  -j, --skip-bytes=BYTES      skip BYTES input bytes first\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -N, --read-bytes=BYTES      limit dump to BYTES input bytes\n  -S BYTES, --strings[=BYTES]  show only NUL terminated strings\n                                of at least BYTES (3) printable characters\n  -t, --format=TYPE           select output format or formats\n  -v, --output-duplicates     do not use * to mark line suppression\n  -w[BYTES], --width[=BYTES]  output BYTES bytes per output line;\n                                32 is implied when BYTES is not specified\n      --traditional           accept arguments in third form above\n"
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
                            "\n\nTraditional format specifications may be intermixed; they accumulate:\n  -a   same as -t a,  select named characters, ignoring high-order bit\n  -b   same as -t o1, select octal bytes\n  -c   same as -t c,  select printable characters or backslash escapes\n  -d   same as -t u2, select unsigned decimal 2-byte units\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -f   same as -t fF, select floats\n  -i   same as -t dI, select decimal ints\n  -l   same as -t dL, select decimal longs\n  -o   same as -t o2, select octal 2-byte units\n  -s   same as -t d2, select decimal 2-byte units\n  -x   same as -t x2, select hexadecimal 2-byte units\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n\nTYPE is made up of one or more of these specifications:\n  a          named character, ignoring high-order bit\n  c          printable character or backslash escape\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  d[SIZE]    signed decimal, SIZE bytes per integer\n  f[SIZE]    floating point, SIZE bytes per float\n  o[SIZE]    octal, SIZE bytes per integer\n  u[SIZE]    unsigned decimal, SIZE bytes per integer\n  x[SIZE]    hexadecimal, SIZE bytes per integer\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nSIZE is a number.  For TYPE in [doux], SIZE may also be C for\nsizeof(char), S for sizeof(short), I for sizeof(int) or L for\nsizeof(long).  If TYPE is f, SIZE may also be B for Brain 16 bit,\nH for Half precision float, F for sizeof(float), D for sizeof(double),\nor L for sizeof(long double).\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nAdding a z suffix to any type displays printable characters at the end of\neach output line.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n\nBYTES is hex with 0x or 0X prefix, and may have a multiplier suffix:\n  b    512\n  KB   1000\n  K    1024\n  MB   1000*1000\n  M    1024*1024\nand so on for G, T, P, E, Z, Y, R, Q.\nBinary prefixes can be used, too: KiB=K, MiB=M, and so on.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_001061de);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: print_n_spaces
void print_n_spaces(long param_1) {
  long local_10;
  
  for (local_10 = param_1; 0 < local_10; local_10 = local_10 + -1) {
    putchar_unlocked(0x20);
  }
  return;
}

// Function: pad_at
long pad_at(long param_1,long param_2,long param_3) {
  return ((param_3 % param_1) * param_2) / param_1 + (param_3 / param_1) * param_2;
}

// Function: pad_at_overflow
bool pad_at_overflow(long param_1) {
  long in_FS_OFFSET;
  
  if (*(long *)(in_FS_OFFSET + 0x28) != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return SEXT816((param_1 + -1) * (param_1 + -1)) != SEXT816(param_1 + -1) * SEXT816(param_1 + -1);
}

// Function: dump_hexl_mode_trailer
void dump_hexl_mode_trailer(long param_1,byte *param_2) {
  byte bVar1;
  ushort **ppuVar2;
  byte *local_28;
  long local_10;
  
  fwrite_unlocked(&DAT_001061e5,1,3,_stdout);
  local_28 = param_2;
  for (local_10 = param_1; 0 < local_10; local_10 = local_10 + -1) {
    bVar1 = *local_28;
    ppuVar2 = __ctype_b_loc();
    if (((*ppuVar2)[bVar1] & 0x4000) == 0) {
      bVar1 = 0x2e;
    }
    putchar_unlocked((uint)bVar1);
    local_28 = local_28 + 1;
  }
  putchar_unlocked(0x3c);
  return;
}

// Function: print_named_ascii
void print_named_ascii(long param_1,long param_2,byte *param_3,undefined8 param_4,int param_5,
                      undefined8 param_6) {
  uint uVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  byte *local_40;
  undefined8 local_38;
  long local_30;
  undefined *local_28;
  undefined local_12;
  undefined local_11;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_40 = param_3;
  local_38 = param_6;
  for (local_30 = param_1; param_2 < local_30; local_30 = local_30 + -1) {
    uVar1 = *local_40 & 0x7f;
    if (uVar1 == 0x7f) {
      local_28 = &DAT_001061e9;
    }
    else if (uVar1 < 0x21) {
      local_28 = charname + (long)(int)uVar1 * 4;
    }
    else {
      local_12 = (undefined)uVar1;
      local_11 = 0;
      local_28 = &local_12;
    }
    uVar2 = pad_at(param_1,local_30 + -1,param_6);
    xprintf(&DAT_001061e1,param_5 + ((int)local_38 - (int)uVar2),local_28);
    local_40 = local_40 + 1;
    local_38 = uVar2;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: print_ascii
void print_ascii(long param_1,long param_2,byte *param_3,undefined8 param_4,int param_5,
                undefined8 param_6) {
  byte bVar1;
  ushort **ppuVar2;
  char *__format;
  undefined8 uVar3;
  long in_FS_OFFSET;
  byte *local_50;
  undefined8 local_48;
  long local_40;
  char *local_38;
  char local_24 [4];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_50 = param_3;
  local_48 = param_6;
  for (local_40 = param_1; param_2 < local_40; local_40 = local_40 + -1) {
    bVar1 = *local_50;
    switch(bVar1) {
    case 0:
      local_38 = "\\0";
      break;
    default:
      ppuVar2 = __ctype_b_loc();
      if (((*ppuVar2)[bVar1] & 0x4000) == 0) {
        __format = "%03o";
      }
      else {
        __format = "%c";
      }
      sprintf(local_24,__format,(ulong)bVar1);
      local_38 = local_24;
      break;
    case 7:
      local_38 = "\\a";
      break;
    case 8:
      local_38 = "\\b";
      break;
    case 9:
      local_38 = "\\t";
      break;
    case 10:
      local_38 = "\\n";
      break;
    case 0xb:
      local_38 = "\\v";
      break;
    case 0xc:
      local_38 = "\\f";
      break;
    case 0xd:
      local_38 = "\\r";
    }
    uVar3 = pad_at(param_1,local_40 + -1,param_6);
    xprintf(&DAT_001061e1,param_5 + ((int)local_48 - (int)uVar3),local_38);
    local_50 = local_50 + 1;
    local_48 = uVar3;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: simple_strtoi
undefined8 simple_strtoi(char *param_1,char **param_2,int *param_3) {
  long lVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  long in_FS_OFFSET;
  char *local_20;
  int local_14;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_14 = 0;
  local_20 = param_1;
  do {
    cVar2 = c_isdigit((int)*local_20);
    if (cVar2 == '\0') {
      *param_2 = local_20;
      *param_3 = local_14;
      uVar4 = 1;
LAB_00101b0f:
      if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return uVar4;
    }
    iVar3 = (int)((long)local_14 * 10);
    if (((long)iVar3 != (long)local_14 * 10) ||
       (local_14 = *local_20 + -0x30 + iVar3, SCARRY4(*local_20 + -0x30,iVar3))) {
      uVar4 = 0;
      goto LAB_00101b0f;
    }
    local_20 = local_20 + 1;
  } while( true );
}

// Function: ispec_to_format
undefined8 ispec_to_format(int param_1,undefined8 param_2,undefined8 param_3) {
  if (param_1 == 4) {
    param_2 = param_3;
  }
  return param_2;
}

// Function: open_next_file
undefined open_next_file(void) {
  char cVar1;
  undefined8 uVar2;
  int *piVar3;
  undefined local_29;
  
  local_29 = 1;
  do {
    input_filename = *file_list;
    if (input_filename == 0) {
      return local_29;
    }
    file_list = file_list + 1;
    cVar1 = streq(input_filename,&DAT_00105344);
    if (cVar1 == '\0') {
      in_stream = (FILE *)rpl_fopen(input_filename,&DAT_001063ff);
      if (in_stream == (FILE *)0x0) {
        uVar2 = quotearg_n_style_colon(0,3,input_filename);
        piVar3 = __errno_location();
        error(0,*piVar3,&DAT_00106401,uVar2);
        local_29 = 0;
      }
    }
    else {
      input_filename = gettext("standard input");
      in_stream = _stdin;
      have_read_stdin = 1;
      xset_binary_mode(0,0);
    }
  } while (in_stream == (FILE *)0x0);
  if ((-1 < end_offset) && (flag_dump_strings != '\x01')) {
    setvbuf(in_stream,(char *)0x0,2,0);
  }
  return local_29;
}

// Function: check_and_close
bool check_and_close(int param_1) {
  char cVar1;
  int iVar2;
  int *piVar3;
  undefined8 uVar4;
  int local_3c;
  bool local_29;
  
  local_29 = true;
  if (in_stream != (FILE *)0x0) {
    iVar2 = ferror_unlocked(in_stream);
    local_3c = param_1;
    if (iVar2 == 0) {
      local_3c = 0;
    }
    cVar1 = streq(*(undefined8 *)(file_list + -8),&DAT_00105344);
    if (cVar1 == '\0') {
      iVar2 = rpl_fclose(in_stream);
      if ((iVar2 != 0) && (local_3c == 0)) {
        piVar3 = __errno_location();
        local_3c = *piVar3;
      }
    }
    else {
      clearerr_unlocked(in_stream);
    }
    if (local_3c != 0) {
      uVar4 = quotearg_n_style_colon(0,3,input_filename);
      error(0,local_3c,&DAT_00106401,uVar4);
    }
    local_29 = local_3c == 0;
    in_stream = (FILE *)0x0;
  }
  iVar2 = ferror_unlocked(_stdout);
  if (iVar2 != 0) {
    uVar4 = gettext("write error");
    error(0,0,uVar4);
    local_29 = false;
  }
  return local_29;
}

// Function: skip
bool skip(size_t param_1) {
  byte bVar1;
  byte bVar2;
  int iVar3;
  int *piVar4;
  size_t sVar5;
  undefined8 uVar6;
  ulong uVar7;
  long in_FS_OFFSET;
  size_t local_2100;
  bool local_20ed;
  int local_20ec;
  size_t local_20d8;
  stat local_20b8;
  undefined local_2028 [8200];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_20ed = true;
  local_20ec = 0;
  local_2100 = param_1;
  if (param_1 == 0) {
    local_20ed = true;
  }
  else {
    while (in_stream != (FILE *)0x0) {
      iVar3 = fileno_unlocked(in_stream);
      iVar3 = fstat(iVar3,&local_20b8);
      if (iVar3 == 0) {
        if ((local_20b8.st_mode & 0xf000) == 0x8000) {
          if ((local_20b8.st_blksize < 1) ||
             (uVar7 = local_20b8.st_blksize, 0x2000000000000000 < (ulong)local_20b8.st_blksize)) {
            uVar7 = 0x200;
          }
          if (local_20b8.st_size <= (long)uVar7) goto LAB_00102af3;
          if (local_20b8.st_size < (long)local_2100) {
            local_2100 = local_2100 - local_20b8.st_size;
          }
          else {
            iVar3 = rpl_fseeko(in_stream,local_2100,1);
            if (iVar3 != 0) {
              piVar4 = __errno_location();
              local_20ec = *piVar4;
              local_20ed = false;
            }
            local_2100 = 0;
          }
        }
        else {
LAB_00102af3:
          if (((local_20b8.st_mode & 0xf000) == 0x8000) ||
             (iVar3 = rpl_fseeko(in_stream,local_2100,1), iVar3 != 0)) {
            local_20d8 = 0x2000;
            do {
              do {
                if ((long)local_2100 < 1) goto LAB_00102c04;
                if ((long)local_2100 < (long)local_20d8) {
                  local_20d8 = local_2100;
                }
                sVar5 = fread_unlocked(local_2028,1,local_20d8,in_stream);
                local_2100 = local_2100 - sVar5;
              } while (sVar5 == local_20d8);
              iVar3 = ferror_unlocked(in_stream);
              if (iVar3 != 0) {
                piVar4 = __errno_location();
                local_20ec = *piVar4;
                local_20ed = false;
                local_2100 = 0;
                break;
              }
              iVar3 = feof_unlocked(in_stream);
            } while (iVar3 == 0);
          }
          else {
            local_2100 = 0;
          }
        }
LAB_00102c04:
        if (local_2100 == 0) break;
      }
      else {
        uVar6 = quotearg_n_style_colon(0,3,input_filename);
        piVar4 = __errno_location();
        error(0,*piVar4,&DAT_00106401,uVar6);
        local_20ed = false;
      }
      bVar1 = check_and_close(local_20ec);
      bVar2 = open_next_file();
      local_20ed = (bVar2 & (bVar1 & local_20ed) != 0) != 0;
    }
    if (local_2100 != 0) {
      uVar6 = gettext("cannot skip past end of combined input");
      error(1,0,uVar6);
    }
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_20ed;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: format_address_none
void format_address_none(void) {
  return;
}

// Function: format_address_paren
void format_address_paren(undefined8 param_1,char param_2) {
  putchar_unlocked(0x28);
  format_address_std(param_1,0x29);
  if (param_2 != '\0') {
    putchar_unlocked((int)param_2);
  }
  return;
}

// Function: format_address_label
void format_address_label(long param_1,char param_2) {
  long lVar1;
  long lVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  format_address_std(param_1,0x20);
  lVar2 = param_1 + pseudo_offset;
  if (SCARRY8(param_1,pseudo_offset)) {
    uVar3 = gettext("pseudo address too large for input");
    error(1,0,uVar3);
  }
  format_address_paren(lVar2,(int)param_2);
  if (lVar1 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: write_block
void write_block(undefined8 param_1,long param_2,undefined8 param_3,undefined8 param_4) {
  int iVar1;
  char cVar2;
  long lVar3;
  long lVar4;
  undefined8 uVar5;
  long lVar6;
  long local_30;
  long local_28;
  
  if ((((abbreviate_duplicate_blocks != '\0') && (first_4 != '\x01')) &&
      (param_2 == bytes_per_block)) &&
     (cVar2 = memeq(param_3,param_4,bytes_per_block), cVar2 != '\0')) {
    if (prev_pair_equal_3 != '\0') {
      first_4 = 0;
      return;
    }
    puts("*");
    first_4 = 0;
    prev_pair_equal_3 = 1;
    return;
  }
  prev_pair_equal_3 = 0;
  for (local_30 = 0; local_30 < n_specs; local_30 = local_30 + 1) {
    lVar3 = bytes_per_block /
            (long)*(int *)(width_bytes + (ulong)*(uint *)(local_30 * 0x30 + spec + 4) * 4);
    lVar6 = (long)*(int *)(width_bytes + (ulong)*(uint *)(local_30 * 0x30 + spec + 4) * 4);
    lVar4 = (bytes_per_block - param_2) / lVar6;
    if (local_30 == 0) {
      (*(code *)format_address)(param_1,0);
    }
    else {
      print_n_spaces((long)address_pad_len,lVar6,(bytes_per_block - param_2) % lVar6);
    }
    (**(code **)(local_30 * 0x30 + spec + 8))
              (lVar3,lVar4,param_4,local_30 * 0x30 + spec + 0x10,
               *(undefined4 *)(local_30 * 0x30 + spec + 0x24),
               *(undefined8 *)(local_30 * 0x30 + spec + 0x28));
    if (*(char *)(local_30 * 0x30 + spec + 0x22) != '\0') {
      iVar1 = *(int *)(local_30 * 0x30 + spec + 0x24);
      for (local_28 = 0; local_28 < lVar4; local_28 = local_28 + 1) {
        print_n_spaces((long)iVar1);
      }
      uVar5 = pad_at(lVar3,lVar4,*(undefined8 *)(local_30 * 0x30 + spec + 0x28));
      print_n_spaces(uVar5);
      dump_hexl_mode_trailer(param_2,param_4);
    }
    putchar_unlocked(10);
  }
  first_4 = 0;
  return;
}

// Function: read_char
bool read_char(int *param_1) {
  byte bVar1;
  byte bVar2;
  int iVar3;
  int *piVar4;
  bool local_9;
  
  local_9 = true;
  *param_1 = -1;
  while( true ) {
    if (in_stream == (FILE *)0x0) {
      return local_9;
    }
    iVar3 = getc_unlocked(in_stream);
    *param_1 = iVar3;
    if (-1 < *param_1) break;
    piVar4 = __errno_location();
    bVar1 = check_and_close(*piVar4);
    bVar2 = open_next_file();
    local_9 = (bVar2 & (bVar1 & local_9) != 0) != 0;
  }
  return local_9;
}

// Function: read_block
bool read_block(long param_1,long param_2,long *param_3) {
  long lVar1;
  byte bVar2;
  byte bVar3;
  size_t sVar4;
  int *piVar5;
  bool local_19;
  
  local_19 = true;
  if ((0 < param_1) && (param_1 <= bytes_per_block)) {
    *param_3 = 0;
    while( true ) {
      if (in_stream == (FILE *)0x0) {
        return local_19;
      }
      lVar1 = *param_3;
      sVar4 = fread_unlocked((void *)(*param_3 + param_2),1,param_1 - lVar1,in_stream);
      *param_3 = *param_3 + sVar4;
      if (sVar4 == param_1 - lVar1) break;
      piVar5 = __errno_location();
      bVar2 = check_and_close(*piVar5);
      bVar3 = open_next_file();
      local_19 = (bVar3 & (bVar2 & local_19) != 0) != 0;
    }
    return local_19;
  }
                    /* WARNING: Subroutine does not return */
  __assert_fail("0 < n && n <= bytes_per_block",
                "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/od.c",0x559,
                "read_block");
}

// Function: get_lcm
int get_lcm(void) {
  int local_14;
  long local_10;
  
  local_14 = 1;
  for (local_10 = 0; local_10 < n_specs; local_10 = local_10 + 1) {
    local_14 = lcm((long)local_14,
                   (long)*(int *)(width_bytes + (ulong)*(uint *)(local_10 * 0x30 + spec + 4) * 4));
  }
  return local_14;
}

// Function: xstr2nonneg
int xstr2nonneg(undefined8 param_1,undefined4 param_2,long *param_3,undefined8 param_4) {
  int iVar1;
  
  iVar1 = xstrtoimax(param_1,0,param_2,param_3,param_4);
  if ((iVar1 != 4) && (*param_3 < 0)) {
    iVar1 = 4;
  }
  return iVar1;
}

// Function: parse_old_offset
bool parse_old_offset(char *param_1,undefined8 param_2) {
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  char *__s;
  undefined8 uVar4;
  bool bVar5;
  char *local_20;
  
  __s = param_1 + (*param_1 == '+');
  cVar1 = c_isdigit((int)*__s);
  if (cVar1 == '\x01') {
    local_20 = strchr(__s,0x2e);
    if (local_20 != (char *)0x0) {
      if ((local_20[1] == 'b') || (local_20[1] == 'B')) {
        iVar3 = 1;
      }
      else {
        iVar3 = 0;
      }
      if (local_20[(long)iVar3 + 1] != '\0') {
        local_20 = (char *)0x0;
      }
    }
    if (local_20 == (char *)0x0) {
      if ((*__s == '0') && ((__s[1] == 'x' || (__s[1] == 'X')))) {
        uVar2 = 0x10;
      }
      else {
        uVar2 = 8;
      }
    }
    else {
      uVar2 = 10;
    }
    if (local_20 != (char *)0x0) {
      *local_20 = local_20[1];
      local_20[1] = '\0';
    }
    iVar3 = xstr2nonneg(__s,uVar2,param_2,&DAT_0010649b);
    if (local_20 != (char *)0x0) {
      local_20[1] = *local_20;
      *local_20 = '.';
    }
    if (iVar3 == 1) {
      uVar4 = quotearg_n_style_colon(0,3,param_1);
      error(1,0x22,&DAT_00106401,uVar4);
    }
    bVar5 = iVar3 == 0;
  }
  else {
    bVar5 = false;
  }
  return bVar5;
}

// Function: dump
bool dump(void) {
  byte bVar1;
  int iVar2;
  long in_FS_OFFSET;
  bool local_56;
  bool local_55;
  long local_50;
  long local_48;
  long local_40;
  long local_38;
  long local_30;
  void *local_28;
  long local_20;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_56 = false;
  local_55 = true;
  local_28 = (void *)xinmalloc(2,bytes_per_block);
  local_20 = (long)local_28 + bytes_per_block;
  local_48 = n_bytes_to_skip;
  do {
    if (end_offset < 0) {
      local_40 = 0x7fffffffffffffff;
    }
    else {
      local_40 = end_offset - local_48;
    }
    if (local_40 < 1) {
      local_50 = 0;
      break;
    }
    local_38 = bytes_per_block;
    if (local_40 <= bytes_per_block) {
      local_38 = local_40;
    }
    bVar1 = read_block(local_38,(&local_28)[(int)(uint)local_56],&local_50);
    local_55 = (bVar1 & local_55) != 0;
    if (local_50 < bytes_per_block) break;
    if (local_50 != bytes_per_block) {
                    /* WARNING: Subroutine does not return */
      __assert_fail("n_bytes_read == bytes_per_block",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/od.c",
                    0x5dc,(char *)&__PRETTY_FUNCTION___1);
    }
    write_block(local_48,local_50,(&local_28)[(int)(uint)(local_56 ^ 1)],
                (&local_28)[(int)(uint)local_56]);
    iVar2 = ferror_unlocked(_stdout);
    local_55 = iVar2 == 0 && local_55;
    local_48 = local_48 + local_50;
    local_56 = local_56 == false;
  } while (local_55);
  if (0 < local_50) {
    iVar2 = get_lcm();
    local_30 = (local_50 + iVar2 + -1) - (local_50 + iVar2 + -1) % (long)iVar2;
    memset((void *)((long)(&local_28)[(int)(uint)local_56] + local_50),0,local_30 - local_50);
    write_block(local_48,local_50,(&local_28)[(int)(uint)(local_56 ^ 1)],
                (&local_28)[(int)(uint)local_56]);
    local_48 = local_48 + local_50;
  }
  (*(code *)format_address)(local_48,10);
  if ((-1 < end_offset) && (end_offset <= local_48)) {
    bVar1 = check_and_close(0);
    local_55 = (bVar1 & local_55) != 0;
  }
  free(local_28);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_55;
}

// Function: dump_strings
bool dump_strings(void) {
  long lVar1;
  byte bVar2;
  ushort **ppuVar3;
  long in_FS_OFFSET;
  bool local_35;
  int local_34;
  long local_30;
  void *local_28;
  long local_20;
  long local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_30 = string_min;
  if (string_min < 99) {
    local_30 = 99;
  }
  local_30 = local_30 + 1;
  local_28 = (void *)ximalloc(local_30);
  local_20 = n_bytes_to_skip;
  local_35 = true;
  do {
    local_18 = 0;
    local_34 = 1;
    if ((-1 < end_offset) && ((end_offset < string_min || (end_offset - string_min < local_20)))) {
      free(local_28);
      bVar2 = check_and_close(0);
      local_35 = (bVar2 & local_35) != 0;
LAB_00103d40:
      if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return local_35;
    }
    do {
      if ((-1 < end_offset) && (end_offset <= local_20)) goto LAB_00103b24;
      if (local_18 == local_30 + -1) {
        local_28 = (void *)xpalloc(local_28,&local_30,1,0xffffffffffffffff,1);
      }
      bVar2 = read_char(&local_34);
      local_35 = (bVar2 & local_35) != 0;
      if (local_34 < 0) {
        free(local_28);
        goto LAB_00103d40;
      }
      local_20 = local_20 + 1;
      lVar1 = local_18 + 1;
      *(char *)((long)local_28 + local_18) = (char)local_34;
      local_18 = lVar1;
      if (local_34 == 0) goto LAB_00103b24;
      ppuVar3 = __ctype_b_loc();
    } while (((*ppuVar3)[local_34] & 0x4000) != 0);
    local_34 = -1;
LAB_00103b24:
    if ((-1 < local_34) && (string_min <= (long)(local_18 - (ulong)(local_34 == 0)))) {
      *(undefined *)((long)local_28 + local_18) = 0;
      (*(code *)format_address)(local_20 - local_18,0x20);
      for (local_18 = 0; local_34 = (int)*(char *)((long)local_28 + local_18), local_34 != 0;
          local_18 = local_18 + 1) {
        switch(local_34) {
        case 7:
          fwrite_unlocked(&DAT_001061f0,1,2,_stdout);
          break;
        case 8:
          fwrite_unlocked(&DAT_001061f3,1,2,_stdout);
          break;
        case 9:
          fwrite_unlocked(&DAT_001061ff,1,2,_stdout);
          break;
        case 10:
          fwrite_unlocked(&DAT_001061f9,1,2,_stdout);
          break;
        case 0xb:
          fwrite_unlocked(&DAT_00106202,1,2,_stdout);
          break;
        case 0xc:
          fwrite_unlocked(&DAT_001061f6,1,2,_stdout);
          break;
        case 0xd:
          fwrite_unlocked(&DAT_001061fc,1,2,_stdout);
          break;
        default:
          putc_unlocked(local_34,_stdout);
        }
      }
      putchar_unlocked(10);
    }
  } while( true );
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  bool bVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  long lVar5;
  undefined8 uVar6;
  int *piVar7;
  long in_FS_OFFSET;
  undefined8 *local_128;
  byte local_115;
  undefined4 local_114;
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
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  int local_c4;
  long local_c0;
  long local_b8;
  long local_b0;
  long local_a8;
  long local_a0;
  long local_98;
  long local_90;
  long local_88;
  long local_80;
  long local_78;
  long local_70;
  long local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined *local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_c0 = 0;
  bVar1 = false;
  local_115 = true;
  local_a0 = 0;
  local_b8 = -1;
  local_98 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
LAB_00103e19:
  do {
    while( true ) {
      local_114 = 0xffffffff;
      local_108 = getopt_long(param_1,param_2,"A:aBbcDdeFfHhIij:LlN:OoS:st:vw::Xx",long_options,
                              &local_114);
      if (local_108 == -1) {
        if ((bool)local_115 == true) {
          if ((flag_dump_strings != '\0') && (0 < n_specs)) {
            local_58 = gettext("no type may be specified when dumping strings");
            local_f8 = 1;
            local_f4 = 0;
            error(1,0,local_58);
          }
          local_110 = param_1 - _optind;
          local_128 = param_2;
          if ((!bVar1) || (traditional != '\0')) {
            if (local_110 == 3) {
              if (((traditional != '\0') &&
                  (cVar3 = parse_old_offset(param_2[(long)_optind + 1],&local_b0), cVar3 != '\0'))
                 && (cVar3 = parse_old_offset(param_2[(long)_optind + 2],&local_a8), cVar3 != '\0'))
              {
                n_bytes_to_skip = local_b0;
                flag_pseudo_start = '\x01';
                local_98 = local_a8;
                param_2[(long)_optind + 2] = param_2[_optind];
                local_128 = param_2 + 2;
                local_110 = local_110 + -2;
              }
            }
            else if (local_110 < 4) {
              if (local_110 == 1) {
                if (((traditional != '\0') || (*(char *)param_2[_optind] == '+')) &&
                   (cVar3 = parse_old_offset(param_2[_optind],&local_b0), cVar3 != '\0')) {
                  n_bytes_to_skip = local_b0;
                  local_110 = local_110 + -1;
                  local_128 = param_2 + 1;
                }
              }
              else if ((local_110 == 2) &&
                      ((((traditional != '\0' || (*(char *)param_2[(long)_optind + 1] == '+')) ||
                        (cVar3 = c_isdigit((int)*(char *)param_2[(long)_optind + 1]), cVar3 != '\0')
                        ) && (cVar3 = parse_old_offset(param_2[(long)_optind + 1],&local_a8),
                             cVar3 != '\0')))) {
                if ((traditional == '\0') ||
                   (cVar3 = parse_old_offset(param_2[_optind],&local_b0), cVar3 == '\0')) {
                  n_bytes_to_skip = local_a8;
                  local_110 = local_110 + -1;
                  param_2[(long)_optind + 1] = param_2[_optind];
                  local_128 = param_2 + 1;
                }
                else {
                  n_bytes_to_skip = local_b0;
                  flag_pseudo_start = '\x01';
                  local_98 = local_a8;
                  local_128 = param_2 + 2;
                  local_110 = local_110 + -2;
                }
              }
            }
            if ((traditional != '\0') && (1 < local_110)) {
              uVar6 = quote(local_128[(long)_optind + 1]);
              local_48 = gettext("extra operand %s");
              local_e8 = 0;
              local_e4 = 0;
              error(0,0,local_48,uVar6);
              uVar6 = gettext("compatibility mode supports at most one file");
              local_f0 = 0;
              local_ec = 0;
              local_50 = &DAT_00106401;
              error(0,0,&DAT_00106401,uVar6);
              usage(1);
            }
          }
          if (flag_pseudo_start != '\0') {
            if ((code *)format_address == format_address_none) {
              address_base = 8;
              address_pad_len = 7;
              format_address = format_address_paren;
            }
            else {
              format_address = format_address_label;
            }
          }
          if ((-1 < local_b8) &&
             (end_offset = local_b8 + n_bytes_to_skip, SCARRY8(local_b8,n_bytes_to_skip))) {
            local_40 = gettext("skip-bytes + read-bytes is too large");
            local_e0 = 1;
            local_dc = 0;
            error(1,0,local_40);
          }
          if (n_specs == 0) {
            decode_format_string(&DAT_00106625);
          }
          if (local_110 < 1) {
            file_list = default_file_list;
          }
          else {
            file_list = (undefined1 *)(local_128 + _optind);
          }
          local_115 = open_next_file();
          if (in_stream != 0) {
            bVar2 = skip(n_bytes_to_skip);
            local_115 = (bVar2 & local_115) != 0;
            if (in_stream != 0) {
              if (flag_pseudo_start == '\0') {
                pseudo_offset = 0;
              }
              else {
                pseudo_offset = local_98 - n_bytes_to_skip;
              }
              local_104 = get_lcm();
              lVar5 = local_c0;
              if (local_c0 == 0) {
                if (local_104 < 0x10) {
                  bytes_per_block = (long)(0x10 - (int)(0x10 % (long)local_104));
                }
                else {
                  bytes_per_block = (long)local_104;
                }
              }
              else if (local_c0 % (long)local_104 == 0) {
                bytes_per_block = local_c0;
              }
              else {
                local_38 = gettext("warning: invalid width %td; using %d instead");
                local_d8 = 0;
                local_d4 = 0;
                error(0,0,local_38,lVar5,local_104);
                bytes_per_block = (long)local_104;
              }
              for (local_90 = 0; local_90 < n_specs; local_90 = local_90 + 1) {
                local_70 = bytes_per_block /
                           (long)*(int *)(width_bytes +
                                         (ulong)*(uint *)(local_90 * 0x30 + spec + 4) * 4);
                cVar3 = pad_at_overflow(local_70,spec,
                                        bytes_per_block %
                                        (long)*(int *)(width_bytes +
                                                      (ulong)*(uint *)(local_90 * 0x30 + spec + 4) *
                                                      4));
                lVar5 = local_c0;
                if (cVar3 != '\0') {
                  local_30 = gettext("%td is too large");
                  local_d0 = 1;
                  local_cc = 0;
                  error(1,0,local_30,lVar5);
                }
                local_68 = (*(int *)(local_90 * 0x30 + spec + 0x24) + 1) * local_70;
                if (local_a0 < local_68) {
                  local_a0 = local_68;
                }
              }
              for (local_88 = 0; local_88 < n_specs; local_88 = local_88 + 1) {
                local_80 = bytes_per_block /
                           (long)*(int *)(width_bytes +
                                         (ulong)*(uint *)(local_88 * 0x30 + spec + 4) * 4);
                local_78 = *(int *)(local_88 * 0x30 + spec + 0x24) * local_80;
                *(long *)(spec + local_88 * 0x30 + 0x28) = local_a0 - local_78;
              }
              if (flag_dump_strings == '\0') {
                bVar2 = dump();
              }
              else {
                bVar2 = dump_strings();
              }
              local_115 = (bVar2 & local_115) != 0;
            }
          }
          if ((have_read_stdin != '\0') && (iVar4 = rpl_fclose(_stdin), iVar4 < 0)) {
            uVar6 = gettext("standard input");
            piVar7 = __errno_location();
            local_c4 = *piVar7;
            local_c8 = 1;
            local_28 = uVar6;
            error(1,local_c4,uVar6);
          }
          local_115 = local_115 ^ 1;
        }
        else {
          local_115 = 1;
        }
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        return local_115;
      }
      if (local_108 < 0x82) break;
switchD_00103ee8_caseD_43:
      usage(1);
    }
    if (local_108 < 0x41) {
      if (local_108 != -0x83) {
        if (local_108 != -0x82) goto switchD_00103ee8_caseD_43;
        goto LAB_001045a6;
      }
      goto LAB_001045b0;
    }
    switch(local_108) {
    case 0x41:
      bVar1 = true;
      cVar3 = *_optarg;
      if (cVar3 == 'x') {
        format_address = format_address_std;
        address_base = 0x10;
        address_pad_len = 6;
        break;
      }
      if (cVar3 < 'y') {
        if (cVar3 == 'o') {
          format_address = format_address_std;
          address_base = 8;
          address_pad_len = 7;
          break;
        }
        if (cVar3 < 'p') {
          if (cVar3 == 'd') {
            format_address = format_address_std;
            address_base = 10;
            address_pad_len = 7;
            break;
          }
          if (cVar3 == 'n') {
            format_address = format_address_none;
            address_pad_len = 0;
            break;
          }
        }
      }
      cVar3 = *_optarg;
      local_60 = gettext("invalid output address radix \'%c\'; it must be one character from [doxn]"
                        );
      local_100 = 1;
      local_fc = 0;
      error(1,0,local_60,(int)cVar3);
      break;
    case 0x42:
    case 0x6f:
      bVar2 = decode_format_string(&DAT_00106568);
      local_115 = (bVar2 & local_115) != 0;
      break;
    default:
      goto switchD_00103ee8_caseD_43;
    case 0x44:
      bVar2 = decode_format_string(&DAT_00106550);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x46:
    case 0x65:
      bVar2 = decode_format_string(&DAT_00106556);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x48:
    case 0x58:
      bVar2 = decode_format_string(&DAT_0010655c);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x49:
    case 0x4c:
    case 0x6c:
      bVar2 = decode_format_string(&DAT_00106562);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x4f:
      bVar2 = decode_format_string(&DAT_00106565);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x53:
      goto switchD_00103ee8_caseD_53;
    case 0x61:
      bVar2 = decode_format_string(&DAT_00106549);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x62:
      bVar2 = decode_format_string(&DAT_0010654b);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 99:
      bVar2 = decode_format_string(&DAT_0010654e);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 100:
      bVar2 = decode_format_string(&DAT_00106553);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x66:
      bVar2 = decode_format_string(&DAT_00106559);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x68:
    case 0x78:
      bVar2 = decode_format_string(&DAT_0010656e);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x69:
      bVar2 = decode_format_string(&DAT_0010655f);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x6a:
      bVar1 = true;
      local_10c = xstr2nonneg(_optarg,0,&n_bytes_to_skip,multipliers_0);
      if (local_10c == 0) break;
      xstrtol_fatal(local_10c,local_114,(int)(char)local_108,long_options,_optarg);
    case 0x4e:
      bVar1 = true;
      local_10c = xstr2nonneg(_optarg,0,&local_b8,multipliers_0);
      if (local_10c != 0) {
        xstrtol_fatal(local_10c,local_114,(int)(char)local_108,long_options,_optarg);
switchD_00103ee8_caseD_53:
        bVar1 = true;
        if (_optarg == (char *)0x0) {
          string_min = 3;
        }
        else {
          local_10c = xstr2nonneg(_optarg,0,&local_b0,multipliers_0);
          if ((local_10c == 0) && (local_a8 = local_b0 + 1, SCARRY8(local_b0,1))) {
            local_10c = 1;
          }
          if (local_10c != 0) {
            xstrtol_fatal(local_10c,local_114,(int)(char)local_108,long_options,_optarg);
          }
          string_min = local_b0;
        }
        flag_dump_strings = '\x01';
      }
      break;
    case 0x73:
      bVar2 = decode_format_string(&DAT_0010656b);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x74:
      bVar1 = true;
      bVar2 = decode_format_string(_optarg);
      local_115 = (bVar2 & local_115) != 0;
      break;
    case 0x76:
      bVar1 = true;
      abbreviate_duplicate_blocks = 0;
      break;
    case 0x77:
      bVar1 = true;
      if (_optarg != (char *)0x0) goto LAB_001044ff;
      local_c0 = 0x20;
      break;
    case 0x80:
      traditional = '\x01';
      break;
    case 0x81:
      lVar5 = __xargmatch_internal("--endian",_optarg,endian_args,&endian_types,4,_argmatch_die,1);
      iVar4 = *(int *)((long)&endian_types + lVar5 * 4);
      if (iVar4 == 0) {
        input_swap = 0;
      }
      else if (iVar4 == 1) {
        input_swap = 1;
      }
    }
  } while( true );
LAB_001044ff:
  local_10c = xstr2nonneg(_optarg,10,&local_a8,&DAT_0010522c);
  if ((local_10c == 0) && (local_c0 = local_a8, local_a8 == 0)) {
    local_10c = 4;
  }
  if (local_10c != 0) {
    xstrtol_fatal(local_10c,local_114,(int)(char)local_108,long_options,_optarg);
LAB_001045a6:
    usage(0);
LAB_001045b0:
    uVar6 = proper_name_lite("Jim Meyering","Jim Meyering");
    version_etc(_stdout,&DAT_001061de,"GNU coreutils",_Version,uVar6,0);
                    /* WARNING: Subroutine does not return */
    exit(0);
  }
  goto LAB_00103e19;
}

