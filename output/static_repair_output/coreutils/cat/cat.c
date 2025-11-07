// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [FILE]...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Concatenate FILE(s) to standard output.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n  -A, --show-all           equivalent to -vET\n  -b, --number-nonblank    number nonempty output lines, overrides -n\n  -e                       equivalent to -vE\n  -E, --show-ends          display $ at end of each line\n  -n, --number             number all output lines\n  -s, --squeeze-blank      suppress repeated empty output lines\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -t                       equivalent to -vT\n  -T, --show-tabs          display TAB characters as ^I\n  -u                       (ignored)\n  -v, --show-nonprinting   use ^ and M- notation, except for LFD and TAB\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    uVar2 = _program_name;
    pcVar3 = (char *)gettext(
                            "\nExamples:\n  %s f - g  Output f\'s contents, then standard input, then g\'s contents.\n  %s        Copy standard input to standard output.\n"
                            );
    printf(pcVar3,uVar2,uVar2);
    emit_ancillary_info(&DAT_00101fc9);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: next_line_num
void next_line_num(void) {
  char *pcVar1;
  char cVar2;
  char *local_10;
  
  local_10 = line_num_end;
  do {
    cVar2 = *local_10;
    *local_10 = cVar2 + '\x01';
    if (cVar2 < '9') {
      return;
    }
    pcVar1 = local_10 + -1;
    *local_10 = '0';
    local_10 = pcVar1;
  } while (line_num_start <= pcVar1);
  if (line_num_start < line_buf + 1) {
    line_buf[0] = 0x3e;
  }
  else {
    line_num_start = line_num_start + -1;
    *line_num_start = '1';
  }
  if (line_num_start < line_num_print) {
    line_num_print = line_num_print + -1;
  }
  return;
}

// Function: simple_cat
undefined8 simple_cat(void *param_1,size_t param_2) {
  ssize_t sVar1;
  undefined8 uVar2;
  int *piVar3;
  long lVar4;
  
  while( true ) {
    sVar1 = read(input_desc,param_1,param_2);
    if (sVar1 < 0) {
      uVar2 = quotearg_n_style_colon(0,3,infile);
      piVar3 = __errno_location();
      error(0,*piVar3,&DAT_00101fcd,uVar2);
      return 0;
    }
    if (sVar1 == 0) break;
    lVar4 = full_write(1,param_1,sVar1);
    if (sVar1 != lVar4) {
      write_error();
    }
  }
  return 1;
}

// Function: write_pending
void write_pending(long param_1,long *param_2) {
  long lVar1;
  long lVar2;
  
  lVar1 = *param_2 - param_1;
  if (0 < lVar1) {
    lVar2 = full_write(1,param_1,lVar1);
    if (lVar1 != lVar2) {
      write_error();
    }
    *param_2 = param_1;
  }
  return;
}

// Function: cat
undefined8
cat(byte *param_1,size_t param_2,byte *param_3,long param_4,char param_5,char param_6,char param_7,
   char param_8,char param_9,char param_10) {
  byte *pbVar1;
  bool bVar2;
  int iVar3;
  long lVar4;
  int *piVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  byte local_7b;
  int local_78;
  int local_74;
  undefined4 local_70;
  int local_6c;
  undefined4 local_68;
  int local_64;
  byte *local_60;
  byte *local_58;
  byte *local_50;
  byte *local_48;
  size_t local_40;
  ssize_t local_38;
  undefined8 local_30;
  undefined *local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_74 = newlines2;
  bVar2 = true;
  local_50 = param_1 + 1;
  local_60 = param_3;
  local_58 = param_1;
LAB_00100817:
  do {
    pbVar1 = param_3;
    if (param_3 + param_4 <= local_60) {
      do {
        local_48 = pbVar1;
        lVar4 = full_write(1,local_48,param_4);
        if (param_4 != lVar4) {
          write_error();
        }
        local_48 = local_48 + param_4;
        local_40 = (long)local_60 - (long)local_48;
        pbVar1 = local_48;
      } while (param_4 <= (long)local_40);
      memmove(param_3,local_48,local_40);
      local_60 = param_3 + local_40;
    }
    if (local_58 < local_50) {
      local_78 = 0;
      if ((bVar2) && (iVar3 = ioctl(input_desc,0x541b,&local_78), iVar3 < 0)) {
        piVar5 = __errno_location();
        if (((*piVar5 != 0x5f) &&
            (((piVar5 = __errno_location(), *piVar5 != 0x19 &&
              (piVar5 = __errno_location(), *piVar5 != 0x16)) &&
             (piVar5 = __errno_location(), *piVar5 != 0x13)))) &&
           (piVar5 = __errno_location(), *piVar5 != 0x26)) {
          uVar7 = quotearg_style(4,infile);
          uVar6 = gettext("cannot do ioctl on %s");
          piVar5 = __errno_location();
          local_6c = *piVar5;
          local_70 = 0;
          local_30 = uVar6;
          error(0,local_6c,uVar6,uVar7);
          uVar7 = 0;
          goto LAB_00100e23;
        }
        bVar2 = false;
      }
      if (local_78 == 0) {
        write_pending(param_3,&local_60);
      }
      local_38 = read(input_desc,param_1,param_2);
      if (local_38 < 0) {
        uVar7 = quotearg_n_style_colon(0,3,infile);
        piVar5 = __errno_location();
        local_64 = *piVar5;
        local_68 = 0;
        local_28 = &DAT_00101fcd;
        error(0,local_64,&DAT_00101fcd,uVar7);
        write_pending(param_3,&local_60);
        uVar7 = 0;
        goto LAB_00100e23;
      }
      if (local_38 == 0) {
        write_pending(param_3,&local_60);
        uVar7 = 1;
LAB_00100e23:
        if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
          newlines2 = local_74;
                    /* WARNING: Subroutine does not return */
          __stack_chk_fail();
        }
        newlines2 = local_74;
        return uVar7;
      }
      local_58 = param_1 + local_38;
      *local_58 = 10;
      local_50 = param_1;
LAB_00100b93:
      local_7b = *local_50;
    }
    else {
      local_74 = local_74 + 1;
      if (local_74 < 1) {
LAB_00100b3c:
        if (param_9 != '\0') {
          if (pending_cr != '\0') {
            pbVar1 = local_60 + 1;
            *local_60 = 0x5e;
            local_60 = local_60 + 2;
            *pbVar1 = 0x4d;
            pending_cr = '\0';
          }
          *local_60 = 0x24;
          local_60 = local_60 + 1;
        }
        *local_60 = 10;
        local_60 = local_60 + 1;
        goto LAB_00100b93;
      }
      if ((local_74 < 2) || (local_74 = 2, param_10 == '\0')) {
        if ((param_7 != '\0') && (param_8 != '\x01')) {
          next_line_num();
          local_60 = (byte *)stpcpy((char *)local_60,line_num_print);
        }
        goto LAB_00100b3c;
      }
      local_7b = *local_50;
    }
    local_50 = local_50 + 1;
    if (local_7b != 10) {
      if (pending_cr != '\0') {
        *local_60 = 0xd;
        pending_cr = '\0';
        local_60 = local_60 + 1;
      }
      if ((-1 < local_74) && (param_7 != '\0')) {
        next_line_num();
        local_60 = (byte *)stpcpy((char *)local_60,line_num_print);
      }
      if (param_5 == '\0') {
        do {
          if ((local_7b == 9) && (param_6 != '\0')) {
            *local_60 = 0x5e;
            local_60[1] = 0x49;
            local_60 = local_60 + 2;
          }
          else {
            if (local_7b == 10) goto LAB_00100e1e;
            if (((local_7b == 0xd) && (*local_50 == 10)) && (param_9 != '\0')) {
              if (local_50 == local_58) {
                pending_cr = '\x01';
              }
              else {
                *local_60 = 0x5e;
                local_60[1] = 0x4d;
                local_60 = local_60 + 2;
              }
            }
            else {
              *local_60 = local_7b;
              local_60 = local_60 + 1;
            }
          }
          local_7b = *local_50;
          local_50 = local_50 + 1;
        } while( true );
      }
      do {
        if (local_7b < 0x20) {
          if ((local_7b == 9) && (param_6 != '\x01')) {
            *local_60 = 9;
            local_60 = local_60 + 1;
          }
          else {
            if (local_7b == 10) goto LAB_00100e1e;
            pbVar1 = local_60 + 1;
            *local_60 = 0x5e;
            local_60 = local_60 + 2;
            *pbVar1 = local_7b + 0x40;
          }
        }
        else if (local_7b < 0x7f) {
          *local_60 = local_7b;
          local_60 = local_60 + 1;
        }
        else if (local_7b == 0x7f) {
          *local_60 = 0x5e;
          local_60[1] = 0x3f;
          local_60 = local_60 + 2;
        }
        else {
          *local_60 = 0x4d;
          pbVar1 = local_60 + 2;
          local_60[1] = 0x2d;
          if (local_7b < 0xa0) {
            *pbVar1 = 0x5e;
            local_60[3] = local_7b - 0x40;
            local_60 = local_60 + 4;
          }
          else if (local_7b == 0xff) {
            *pbVar1 = 0x5e;
            local_60[3] = 0x3f;
            local_60 = local_60 + 4;
          }
          else {
            *pbVar1 = local_7b + 0x80;
            local_60 = local_60 + 3;
          }
        }
        local_7b = *local_50;
        local_50 = local_50 + 1;
      } while( true );
    }
  } while( true );
LAB_00100e1e:
  local_74 = -1;
  goto LAB_00100817;
}

// Function: copy_cat
ulong copy_cat(void) {
  char cVar1;
  long lVar2;
  int *piVar3;
  undefined8 uVar4;
  byte local_31;
  
  local_31 = 0;
  while (lVar2 = rpl_copy_file_range(input_desc,0,1,0,0x7fffffffc0000000,0), lVar2 != -1) {
    if (lVar2 == 0) {
      return (ulong)local_31;
    }
    local_31 = 1;
  }
  piVar3 = __errno_location();
  if (*piVar3 != 0x26) {
    piVar3 = __errno_location();
    cVar1 = is_ENOTSUP(*piVar3);
    if ((((cVar1 == '\0') && (piVar3 = __errno_location(), *piVar3 != 0x16)) &&
        (piVar3 = __errno_location(), *piVar3 != 9)) &&
       (((piVar3 = __errno_location(), *piVar3 != 0x12 &&
         (piVar3 = __errno_location(), *piVar3 != 0x1a)) &&
        (piVar3 = __errno_location(), *piVar3 != 1)))) {
      uVar4 = quotearg_n_style_colon(0,3,infile);
      piVar3 = __errno_location();
      error(0,*piVar3,&DAT_00101fcd,uVar4);
      return 0xffffffff;
    }
  }
  return 0;
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  bool bVar1;
  ulong uVar2;
  ulong uVar3;
  char cVar4;
  byte bVar5;
  int iVar6;
  uint uVar7;
  undefined8 uVar8;
  int *piVar9;
  long lVar10;
  long lVar11;
  long lVar12;
  __off_t _Var13;
  __off_t _Var14;
  undefined8 uVar15;
  long lVar16;
  long in_FS_OFFSET;
  bool bVar17;
  char local_182;
  undefined local_181;
  char local_180;
  char local_17f;
  char local_17e;
  char local_17d;
  bool local_17c;
  uint local_174;
  int local_170;
  long local_130;
  undefined8 local_128;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  bVar1 = false;
  local_182 = '\0';
  local_181 = 0;
  local_180 = '\0';
  local_17f = '\0';
  local_17e = '\0';
  local_17d = '\0';
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
LAB_001011c7:
  iVar6 = getopt_long(param_1,param_2,"benstuvAET",long_options_0,0);
  if (iVar6 != -1) {
    if (iVar6 < 0x77) {
      if (iVar6 < 0x41) {
        if (iVar6 == -0x83) {
LAB_0010113e:
          uVar8 = proper_name_lite("Richard M. Stallman","Richard M. Stallman");
          uVar15 = proper_name_lite("Torbjorn Granlund",&DAT_00102012);
          version_etc(_stdout,&DAT_00101fc9,"GNU coreutils",_Version,uVar15,uVar8,0);
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        if (iVar6 == -0x82) {
          usage(0);
          goto LAB_0010113e;
        }
        goto switchD_001010a2_caseD_42;
      }
      goto code_r0x00101082;
    }
    goto switchD_001010a2_caseD_42;
  }
  iVar6 = fstat(1,&local_b8);
  if (iVar6 < 0) {
    uVar8 = gettext("standard output");
    piVar9 = __errno_location();
    error(1,*piVar9,uVar8);
  }
  lVar10 = io_blksize(&local_b8);
  uVar3 = local_b8.st_ino;
  uVar2 = local_b8.st_dev;
  local_174 = 0xfffffffe;
  uVar7 = local_b8.st_mode & 0xf000;
  if (((local_182 != '\x01') && (local_17f != '\x01')) && (local_180 != '\x01')) {
    xset_binary_mode(1,0);
  }
  infile = "-";
  local_170 = _optind;
  local_17c = true;
  iVar6 = getpagesize();
  lVar11 = (long)iVar6;
  do {
    if (local_170 < param_1) {
      infile = (char *)param_2[local_170];
    }
    cVar4 = streq(infile,&DAT_00102060);
    if (cVar4 == '\0') {
      input_desc = open(infile,0);
      if (-1 < input_desc) goto LAB_00101448;
      uVar8 = quotearg_n_style_colon(0,3,infile);
      piVar9 = __errno_location();
      error(0,*piVar9,&DAT_00101fcd,uVar8);
      local_17c = false;
    }
    else {
      bVar1 = true;
      input_desc = 0;
LAB_00101448:
      iVar6 = fstat(input_desc,&local_b8);
      if (iVar6 < 0) {
        uVar8 = quotearg_n_style_colon(0,3,infile);
        piVar9 = __errno_location();
        error(0,*piVar9,&DAT_00101fcd,uVar8);
        local_17c = false;
      }
      else {
        lVar12 = io_blksize(&local_b8);
        fdadvise(input_desc,0,0,2);
        if ((((local_b8.st_mode & 0xf000) != 0x1000) && ((local_b8.st_mode & 0xf000) != 0xc000)) &&
           (((uVar3 ^ local_b8.st_ino | local_b8.st_dev ^ uVar2) == 0 &&
            (_Var13 = lseek(input_desc,0,1), -1 < _Var13)))) {
          if ((int)local_174 < -1) {
            local_174 = rpl_fcntl(1,3);
          }
          if (((int)local_174 < 0) || ((local_174 & 0x400) == 0)) {
            iVar6 = 1;
          }
          else {
            iVar6 = 2;
          }
          _Var14 = lseek(1,0,iVar6);
          if (_Var13 < _Var14) {
            uVar8 = quotearg_n_style_colon(0,3,infile);
            uVar15 = gettext("%s: input file is output file");
            error(0,0,uVar15,uVar8);
            local_17c = false;
            goto LAB_0010190e;
          }
        }
        if ((((local_182 == '\x01') || (local_17f == '\x01')) || (local_17e == '\x01')) ||
           ((local_17d == '\x01' || (local_180 == '\x01')))) {
          local_128 = xalignalloc(lVar11,lVar12 + 1);
          local_130 = lVar12 * 4;
          if ((SEXT816(local_130) != SEXT816(lVar12) * SEXT816(4)) ||
             ((bVar17 = SCARRY8(lVar10,local_130), local_130 = lVar10 + local_130, bVar17 ||
              (bVar17 = SCARRY8(local_130,0x13), local_130 = local_130 + 0x13, bVar17)))) {
            xalloc_die();
          }
          uVar8 = xalignalloc(lVar11,local_130);
          bVar5 = cat(local_128,lVar12,uVar8,lVar10,local_17e,local_17d,local_182,local_181,
                      local_17f,local_180);
          local_17c = (bVar5 & local_17c) != 0;
          alignfree(uVar8);
        }
        else {
          if ((uVar7 == 0x8000) && ((local_b8.st_mode & 0xf000) == 0x8000)) {
            iVar6 = copy_cat();
          }
          else {
            iVar6 = 0;
          }
          if (iVar6 == 0) {
            lVar16 = lVar10;
            if (lVar10 <= lVar12) {
              lVar16 = lVar12;
            }
            local_128 = xalignalloc(lVar11,lVar16);
            bVar5 = simple_cat(local_128,lVar16);
            local_17c = (bVar5 & local_17c) != 0;
          }
          else {
            local_128 = 0;
            local_17c = (0 < iVar6 & local_17c) != 0;
          }
        }
        alignfree(local_128);
      }
LAB_0010190e:
      if ((cVar4 != '\x01') && (iVar6 = close(input_desc), iVar6 < 0)) {
        uVar8 = quotearg_n_style_colon(0,3,infile);
        piVar9 = __errno_location();
        error(0,*piVar9,&DAT_00101fcd,uVar8);
        local_17c = false;
      }
    }
    local_170 = local_170 + 1;
    if (param_1 <= local_170) {
      if ((pending_cr != '\0') && (lVar10 = full_write(1,&DAT_00102080,1), lVar10 != 1)) {
        write_error();
      }
      if ((bVar1) && (iVar6 = close(0), iVar6 < 0)) {
        uVar8 = gettext("closing standard input");
        piVar9 = __errno_location();
        error(1,*piVar9,uVar8);
      }
      if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
        return local_17c ^ 1;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
  } while( true );
code_r0x00101082:
  switch(iVar6) {
  case 0x41:
    local_17e = '\x01';
    local_17f = '\x01';
    local_17d = '\x01';
    break;
  default:
switchD_001010a2_caseD_42:
    usage(1);
    break;
  case 0x45:
    local_17f = '\x01';
    break;
  case 0x54:
    local_17d = '\x01';
    break;
  case 0x62:
    local_182 = '\x01';
    local_181 = 1;
    break;
  case 0x65:
    local_17f = '\x01';
    local_17e = '\x01';
    break;
  case 0x6e:
    local_182 = '\x01';
    break;
  case 0x73:
    local_180 = '\x01';
    break;
  case 0x74:
    local_17d = '\x01';
    local_17e = '\x01';
    break;
  case 0x75:
    break;
  case 0x76:
    local_17e = '\x01';
  }
  goto LAB_001011c7;
}

