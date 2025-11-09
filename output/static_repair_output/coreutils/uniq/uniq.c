// Function: swap_lines
void swap_lines(undefined8 *param_1,undefined8 *param_2) {
  undefined8 uVar1;
  
  uVar1 = *param_1;
  *param_1 = *param_2;
  *param_2 = uVar1;
  return;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... [INPUT [OUTPUT]]\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Filter adjacent matching lines from INPUT (or standard input),\nwriting to OUTPUT (or standard output).\n\nWith no options, matching lines are merged to the first occurrence.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -c, --count           prefix lines by the number of occurrences\n  -d, --repeated        only print duplicate lines, one for each group\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -D                    print all duplicate lines\n      --all-repeated[=METHOD]  like -D, but allow separating groups\n                                 with an empty line;\n                                 METHOD={none(default),prepend,separate}\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -f, --skip-fields=N   avoid comparing the first N fields\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --group[=METHOD]  show all items, separating groups with an empty line;\n                          METHOD={separate(default),prepend,append,both}\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -i, --ignore-case     ignore differences in case when comparing\n  -s, --skip-chars=N    avoid comparing the first N characters\n  -u, --unique          only print unique lines\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -z, --zero-terminated     line delimiter is NUL, not newline\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -w, --check-chars=N   compare no more than N characters in lines\n")
    ;
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nA field is a run of blanks (usually spaces and/or TABs), then non-blank\ncharacters.  Fields are skipped before chars.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\n\'uniq\' does not detect repeated lines unless they are adjacent.\nYou may want to sort the input first, or use \'sort -u\' without \'uniq\'.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_001020e9);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: strict_posix2
undefined4 strict_posix2(void) {
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = posix2_version();
  if ((iVar1 < 0x30db0) || (0x31068 < iVar1)) {
    uVar2 = 0;
  }
  else {
    uVar2 = 1;
  }
  return uVar2;
}

// Function: size_opt
long size_opt(undefined8 param_1,undefined8 param_2) {
  uint uVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  long local_20;
  char *local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  uVar1 = xstrtoimax(param_1,0,10,&local_20,&DAT_001019d4);
  if ((1 < uVar1) || (local_20 < 0)) {
    uVar2 = gettext(param_2);
    local_18 = "%s: %s";
    error(1,0,"%s: %s",param_1,uVar2);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_20;
}

// Function: newline_or_blank
undefined4 newline_or_blank(int param_1) {
  int iVar1;
  
  if ((param_1 != 10) && (iVar1 = c32isblank(param_1), iVar1 == 0)) {
    return 0;
  }
  return 1;
}

// Function: find_field
ulong find_field(long param_1,long *param_2) {
  ulong uVar1;
  undefined8 uVar2;
  size_t sVar3;
  ulong uVar4;
  ulong local_40;
  long local_38;
  long local_30;
  long local_28;
  ulong local_20;
  long local_18;
  
  local_40 = *(ulong *)(param_1 + 0x10);
  uVar1 = local_40 + *(long *)(param_1 + 8) + -1;
  local_38 = skip_fields;
  while ((0 < local_38 && (local_40 < uVar1))) {
    uVar2 = skip_buf_matching(local_40,uVar1,newline_or_blank,1);
    local_40 = skip_buf_matching(uVar2,uVar1,newline_or_blank,0);
    local_38 = local_38 + -1;
  }
  local_30 = skip_chars;
  for (; (0 < local_30 && (local_40 < uVar1)); local_40 = local_40 + (uVar4 >> 0x28 & 0xff)) {
    uVar4 = mcel_scan(local_40,uVar1);
    local_30 = local_30 + -1;
  }
  if (check_chars < (long)(uVar1 - local_40)) {
    sVar3 = __ctype_get_mb_cur_max();
    if (sVar3 < 2) {
      local_28 = check_chars;
    }
    else {
      local_20 = local_40;
      local_18 = check_chars;
      while ((0 < local_18 && (local_40 < uVar1))) {
        uVar4 = mcel_scan(local_40,uVar1);
        local_20 = local_20 + (uVar4 >> 0x28 & 0xff);
        local_18 = local_18 + -1;
      }
      local_28 = local_20 - local_40;
    }
  }
  else {
    local_28 = uVar1 - local_40;
  }
  *param_2 = local_28;
  return local_40;
}

// Function: different
undefined4 different(void *param_1,void *param_2,size_t param_3,size_t param_4) {
  int iVar1;
  undefined4 uVar2;
  
  if (ignore_case == '\0') {
    if ((param_3 == param_4) && (iVar1 = memcmp(param_1,param_2,param_3), iVar1 == 0)) {
      uVar2 = 0;
    }
    else {
      uVar2 = 1;
    }
  }
  else if ((param_3 == param_4) && (iVar1 = memcasecmp(param_1,param_2,param_3), iVar1 == 0)) {
    uVar2 = 0;
  }
  else {
    uVar2 = 1;
  }
  return uVar2;
}

// Function: writeline
void writeline(long param_1,char param_2,long param_3) {
  byte bVar1;
  size_t sVar2;
  
  if (param_3 == 0) {
    bVar1 = output_unique ^ 1;
  }
  else if (param_2 == '\x01') {
    bVar1 = output_later_repeated ^ 1;
  }
  else {
    bVar1 = output_first_repeated ^ 1;
  }
  if (bVar1 == 0) {
    if (count_occurrences != '\0') {
      printf("%7jd ",param_3 + 1);
    }
    sVar2 = fwrite_unlocked(*(void **)(param_1 + 0x10),1,*(size_t *)(param_1 + 8),_stdout);
    if (sVar2 != *(size_t *)(param_1 + 8)) {
      write_error();
    }
  }
  return;
}

// Function: check_file
void check_file(undefined8 param_1,undefined8 param_2,char param_3) {
  bool bVar1;
  char cVar2;
  int iVar3;
  long lVar4;
  undefined8 uVar5;
  int *piVar6;
  size_t sVar7;
  undefined8 uVar8;
  long in_FS_OFFSET;
  bool bVar9;
  undefined *local_c8;
  undefined *local_c0;
  undefined8 local_b8;
  undefined8 local_b0;
  long local_a8;
  undefined8 local_a0;
  undefined8 local_98;
  long local_90;
  long local_88;
  undefined8 local_80;
  undefined *local_78;
  undefined *local_70;
  undefined8 local_68;
  undefined8 local_60;
  undefined local_58 [16];
  void *local_48;
  undefined local_38 [16];
  void *local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  cVar2 = streq(param_1,&DAT_001020fb);
  if ((cVar2 != '\x01') && (lVar4 = freopen_safer(param_1,&DAT_001020fd,_stdin), lVar4 == 0)) {
    uVar5 = quotearg_n_style_colon(0,3,param_1);
    piVar6 = __errno_location();
    local_78 = &DAT_001020ff;
    error(1,*piVar6,&DAT_001020ff,uVar5);
  }
  cVar2 = streq(param_2,&DAT_001020fb);
  if ((cVar2 != '\x01') && (lVar4 = freopen_safer(param_2,&DAT_00102102,_stdout), lVar4 == 0)) {
    uVar5 = quotearg_n_style_colon(0,3,param_2);
    piVar6 = __errno_location();
    local_70 = &DAT_001020ff;
    error(1,*piVar6,&DAT_001020ff,uVar5);
  }
  fadvise(_stdin,2);
  local_c8 = local_58;
  local_c0 = local_38;
  initbuffer(local_c8);
  initbuffer(local_c0);
  if (((output_unique == '\0') || (output_first_repeated == '\0')) || (count_occurrences == '\x01'))
  {
    lVar4 = readlinebuffer_delim(local_c0,_stdin,(int)param_3);
    if (lVar4 != 0) {
      local_98 = find_field(local_c0,&local_b8);
      local_90 = 0;
      bVar1 = true;
      while (iVar3 = feof_unlocked(_stdin), iVar3 == 0) {
        lVar4 = readlinebuffer_delim(local_c8,_stdin,(int)param_3);
        if (lVar4 == 0) {
          iVar3 = ferror_unlocked(_stdin);
          if (iVar3 != 0) goto LAB_00100fd9;
          break;
        }
        local_80 = find_field(local_c8,&local_b0);
        cVar2 = different(local_80,local_98,local_b0,local_b8);
        bVar9 = cVar2 == '\0';
        local_90 = local_90 + (ulong)bVar9;
        if (local_90 == 0x7fffffffffffffff) {
          if (count_occurrences != '\0') {
            local_68 = gettext("too many repeated lines");
            error(1,0,local_68);
          }
          local_90 = local_90 + -1;
        }
        if (delimit_groups != 0) {
          if (bVar9) {
            if ((local_90 == 1) && ((delimit_groups == 1 || ((delimit_groups == 2 && (!bVar1)))))) {
              putchar_unlocked((int)param_3);
            }
          }
          else if (local_90 != 0) {
            bVar1 = false;
          }
        }
        if ((!bVar9) || (output_later_repeated != '\0')) {
          writeline(local_c0,bVar9,local_90);
          swap_lines(&local_c0,&local_c8);
          local_98 = local_80;
          local_b8 = local_b0;
          if (!bVar9) {
            local_90 = 0;
          }
        }
      }
      writeline(local_c0,0,local_90);
    }
  }
  else {
    local_a8 = 0;
    bVar1 = false;
    while ((iVar3 = feof_unlocked(_stdin), iVar3 == 0 &&
           (lVar4 = readlinebuffer_delim(local_c8,_stdin,(int)param_3), lVar4 != 0))) {
      local_88 = find_field(local_c8,&local_b0);
      if ((local_a8 == 0) || (cVar2 = different(local_88,local_a8,local_b0,local_a0), cVar2 != '\0')
         ) {
        bVar9 = true;
      }
      else {
        bVar9 = false;
      }
      if (((bVar9) && (grouping != 0)) &&
         (((grouping == 1 || (grouping == 4)) || ((bVar1 && ((grouping == 2 || (grouping == 3)))))))
         ) {
        putchar_unlocked((int)param_3);
      }
      if ((bVar9) || (grouping != 0)) {
        sVar7 = fwrite_unlocked(*(void **)(local_c8 + 0x10),1,*(size_t *)(local_c8 + 8),_stdout);
        if (sVar7 != *(size_t *)(local_c8 + 8)) {
          write_error();
        }
        swap_lines(&local_c0,&local_c8);
        local_a8 = local_88;
        local_a0 = local_b0;
        bVar1 = true;
      }
    }
    if (((grouping == 4) || (grouping == 2)) && (bVar1)) {
      putchar_unlocked((int)param_3);
    }
  }
LAB_00100fd9:
  iVar3 = ferror_unlocked(_stdin);
  if ((iVar3 != 0) || (iVar3 = rpl_fclose(_stdin), iVar3 != 0)) {
    uVar5 = quotearg_style(4,param_1);
    uVar8 = gettext("error reading %s");
    piVar6 = __errno_location();
    local_60 = uVar8;
    error(1,*piVar6,uVar8,uVar5);
  }
  free(local_48);
  free(local_28);
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: main
undefined8 main(int param_1,undefined8 *param_2) {
  bool bVar1;
  char cVar2;
  uint uVar3;
  char *pcVar4;
  undefined8 uVar5;
  long lVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  undefined local_9f;
  int local_9c;
  int local_98;
  int local_94;
  undefined8 local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  char *local_38 [3];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_9c = 0;
  pcVar4 = getenv("POSIXLY_CORRECT");
  local_98 = 0;
  local_94 = 0;
  local_9f = 10;
  bVar1 = false;
  local_38[1] = &DAT_001020fb;
  local_38[0] = "-";
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
LAB_0010117f:
  while ((local_9c == -1 ||
         (((pcVar4 != (char *)0x0 && (local_94 != 0)) ||
          (local_9c = getopt_long(param_1,param_2,"-0123456789Dcdf:is:uw:z",longopts,0),
          local_9c == -1))))) {
    if (param_1 <= _optind) {
      if ((grouping != 0) && (bVar1)) {
        local_50 = gettext("--group is mutually exclusive with -c/-d/-D/-u");
        error(0,0,local_50);
        usage(1);
      }
      if ((grouping != 0) && (count_occurrences != '\0')) {
        local_48 = gettext("grouping and printing repeat counts is meaningless");
        error(0,0,local_48);
        usage(1);
      }
      if ((count_occurrences != '\0') && (output_later_repeated != '\0')) {
        local_40 = gettext("printing all duplicated lines and repeat counts is meaningless");
        error(0,0,local_40);
        usage(1);
      }
      check_file(local_38[0],local_38[1],local_9f);
      if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
        return 0;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    if (local_94 == 2) {
      uVar5 = quote(param_2[_optind]);
      local_60 = gettext("extra operand %s");
      error(0,0,local_60,uVar5);
      usage(1);
    }
    local_38[local_94] = (char *)param_2[_optind];
    _optind = _optind + 1;
    local_94 = local_94 + 1;
  }
  if (local_9c < 0x81) {
    if (0x2f < local_9c) {
      switch(local_9c) {
      case 0x30:
      case 0x31:
      case 0x32:
      case 0x33:
      case 0x34:
      case 0x35:
      case 0x36:
      case 0x37:
      case 0x38:
      case 0x39:
        if (local_98 == 2) {
          skip_fields = 0;
        }
        lVar6 = skip_fields * 10;
        if ((SEXT816(lVar6) != SEXT816(skip_fields) * SEXT816(10)) ||
           (skip_fields = (local_9c + -0x30) + lVar6, SCARRY8((long)(local_9c + -0x30),lVar6))) {
          skip_fields = 0x7fffffffffffffff;
        }
        local_98 = 1;
        break;
      default:
        goto switchD_00101334_caseD_3a;
      case 0x44:
        output_unique = 0;
        output_later_repeated = '\x01';
        if (_optarg == (char *)0x0) {
          delimit_groups = 0;
        }
        else {
          lVar6 = __xargmatch_internal
                            ("--all-repeated",_optarg,delimit_method_string,delimit_method_map,4,
                             _argmatch_die,1);
          delimit_groups = *(undefined4 *)(delimit_method_map + lVar6 * 4);
        }
        bVar1 = true;
        break;
      case 99:
        count_occurrences = '\x01';
        bVar1 = true;
        break;
      case 100:
        output_unique = 0;
        bVar1 = true;
        break;
      case 0x66:
        local_98 = 2;
        skip_fields = size_opt(_optarg,"invalid number of fields to skip");
        break;
      case 0x69:
        ignore_case = 1;
        break;
      case 0x73:
        skip_chars = size_opt(_optarg,"invalid number of bytes to skip");
        break;
      case 0x75:
        output_first_repeated = 0;
        bVar1 = true;
        break;
      case 0x77:
        check_chars = size_opt(_optarg,"invalid number of bytes to compare");
        break;
      case 0x7a:
        local_9f = 0;
        break;
      case 0x80:
        if (_optarg == (char *)0x0) {
          grouping = 3;
        }
        else {
          lVar6 = __xargmatch_internal
                            ("--group",_optarg,grouping_method_string,grouping_method_map,4,
                             _argmatch_die,1);
          grouping = *(int *)(grouping_method_map + lVar6 * 4);
        }
      }
      goto LAB_0010117f;
    }
    if (local_9c == 1) {
      if (((*_optarg == '+') && (cVar2 = strict_posix2(), cVar2 != '\x01')) &&
         (uVar3 = xstrtoimax(_optarg,0,10,&local_68,&DAT_001019d4), uVar3 < 2)) {
        skip_chars = local_68;
      }
      else {
        if (local_94 == 2) {
          uVar5 = quote(_optarg);
          local_58 = gettext("extra operand %s");
          error(0,0,local_58,uVar5);
          usage(1);
        }
        local_38[local_94] = _optarg;
        local_94 = local_94 + 1;
      }
      goto LAB_0010117f;
    }
    if (local_9c < 2) {
      if (local_9c != -0x83) {
        if (local_9c != -0x82) goto switchD_00101334_caseD_3a;
        usage(0);
      }
      uVar5 = proper_name_lite("David MacKenzie","David MacKenzie");
      uVar7 = proper_name_lite("Richard M. Stallman","Richard M. Stallman");
      version_etc(_stdout,&DAT_001020e9,"GNU coreutils",_Version,uVar7,uVar5,0);
                    /* WARNING: Subroutine does not return */
      exit(0);
    }
  }
switchD_00101334_caseD_3a:
  usage(1);
  goto LAB_0010117f;
}

