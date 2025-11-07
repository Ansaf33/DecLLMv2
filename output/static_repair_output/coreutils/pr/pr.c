// Function: integer_overflow
int integer_overflow(void) {
  undefined8 uVar1;
  uint uStack_30;
  int iStack_2c;
  long lStack_28;
  
  uVar1 = gettext("integer overflow");
  error(1,0,uVar1);
  iStack_2c = 0;
  lStack_28 = column_vector;
  for (uStack_30 = 0; uStack_30 < columns; uStack_30 = uStack_30 + 1) {
    if (((*(int *)(lStack_28 + 0x10) == 0) || (*(int *)(lStack_28 + 0x10) == 1)) ||
       ((storing_columns != '\0' &&
        ((0 < *(int *)(lStack_28 + 0x2c) && (0 < *(int *)(lStack_28 + 0x30))))))) {
      iStack_2c = iStack_2c + 1;
    }
    lStack_28 = lStack_28 + 0x40;
  }
  return iStack_2c;
}

// Function: cols_ready_to_print
int cols_ready_to_print(void) {
  uint local_18;
  int local_14;
  long local_10;
  
  local_14 = 0;
  local_10 = column_vector;
  for (local_18 = 0; local_18 < columns; local_18 = local_18 + 1) {
    if (((*(int *)(local_10 + 0x10) == 0) || (*(int *)(local_10 + 0x10) == 1)) ||
       ((storing_columns != '\0' &&
        ((0 < *(int *)(local_10 + 0x2c) && (0 < *(int *)(local_10 + 0x30))))))) {
      local_14 = local_14 + 1;
    }
    local_10 = local_10 + 0x40;
  }
  return local_14;
}

// Function: first_last_page
undefined8 first_last_page(undefined4 param_1,char param_2,char *param_3) {
  int iVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  char *local_30;
  ulong local_28;
  ulong local_20;
  char *local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_20 = 0xffffffffffffffff;
  iVar1 = xstrtoumax(param_3,&local_30,10,&local_28,&DAT_0010426c);
  if ((iVar1 != 0) && (iVar1 != 2)) {
    xstrtol_fatal(iVar1,param_1,(int)param_2,long_options,param_3);
  }
  if ((param_3 == local_30) || (local_28 == 0)) {
    uVar2 = 0;
  }
  else {
    if (*local_30 == ':') {
      local_18 = local_30 + 1;
      iVar1 = xstrtoumax(local_18,&local_30,10,&local_20,&DAT_0010426c);
      if (iVar1 != 0) {
        xstrtol_fatal(iVar1,param_1,(int)param_2,long_options,param_3);
      }
      if ((local_18 == local_30) || (local_20 < local_28)) {
        uVar2 = 0;
        goto LAB_00100582;
      }
    }
    if (*local_30 == '\0') {
      first_page_number = local_28;
      last_page_number = local_20;
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
  }
LAB_00100582:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar2;
}

// Function: parse_column_count
void parse_column_count(undefined8 param_1) {
  undefined8 uVar1;
  
  uVar1 = gettext("invalid number of columns");
  columns = getoptnum(param_1,1,uVar1);
  explicit_columns = 1;
  return;
}

// Function: separator_string
void separator_string(char *param_1) {
  size_t sVar1;
  
  sVar1 = strlen(param_1);
  if (0x7fffffff < sVar1) {
    integer_overflow();
  }
  col_sep_length = (int)sVar1;
  col_sep_string = param_1;
  return;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  bool bVar1;
  bool bVar2;
  bool bVar3;
  char cVar4;
  int iVar5;
  undefined8 uVar6;
  char *pcVar7;
  EVP_PKEY_CTX *ctx;
  undefined8 uVar8;
  int *piVar9;
  long in_FS_OFFSET;
  undefined4 local_a4;
  uint local_a0;
  uint local_9c;
  int local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  int local_6c;
  long local_68;
  void *local_60;
  long local_58;
  long local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  bVar1 = false;
  bVar2 = false;
  bVar3 = false;
  local_60 = (void *)0x0;
  local_58 = 0;
  local_68 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  local_a0 = 0;
  if (param_1 < 2) {
    local_50 = 0;
  }
  else {
    local_50 = xnmalloc((long)(param_1 + -1),8);
  }
LAB_0010070f:
  do {
    while( true ) {
      local_a4 = 0xffffffff;
      local_98 = getopt_long(param_1,param_2,"-0123456789D:FJN:S::TW:abcde::fh:i::l:mn::o:rs::tvw:",
                             long_options,&local_a4);
      if (local_98 == -1) {
        if (local_60 != (void *)0x0) {
          parse_column_count(local_60);
          free(local_60);
        }
        if (date_format == (char *)0x0) {
          pcVar7 = getenv("POSIXLY_CORRECT");
          if ((pcVar7 == (char *)0x0) || (cVar4 = hard_locale(2), cVar4 == '\x01')) {
            date_format = "%Y-%m-%d %H:%M";
          }
          else {
            date_format = "%b %e %H:%M %Y";
          }
        }
        ctx = (EVP_PKEY_CTX *)getenv("TZ");
        localtz = tzalloc();
        if (first_page_number == 0) {
          first_page_number = 1;
        }
        if ((parallel_files != '\0') && (explicit_columns != '\0')) {
          local_38 = gettext("cannot specify number of columns when printing in parallel");
          local_80 = 1;
          local_7c = 0;
          ctx = (EVP_PKEY_CTX *)0x1;
          error(1,0,local_38);
        }
        if ((parallel_files != '\0') && (print_across_flag != '\0')) {
          local_30 = gettext("cannot specify both printing across and printing in parallel");
          local_78 = 1;
          local_74 = 0;
          ctx = (EVP_PKEY_CTX *)0x1;
          error(1,0,local_30);
        }
        if (bVar1) {
          if (bVar2) {
            if ((parallel_files == '\0') && (explicit_columns == '\0')) {
              join_lines = 1;
            }
            else {
              truncate_lines = '\x01';
              if (bVar3) {
                use_col_separator = '\x01';
              }
            }
          }
          else if (((use_col_separator != '\x01') && (bVar3)) &&
                  ((parallel_files != '\0' || (explicit_columns != '\0')))) {
            if (truncate_lines == '\x01') {
              use_col_separator = '\x01';
            }
            else {
              join_lines = 1;
              if (0 < col_sep_length) {
                use_col_separator = '\x01';
              }
            }
          }
        }
        for (; _optind < param_1; _optind = _optind + 1) {
          *(undefined8 *)((ulong)local_a0 * 8 + local_50) = param_2[_optind];
          local_a0 = local_a0 + 1;
        }
        if (local_a0 == 0) {
          ctx = (EVP_PKEY_CTX *)0x0;
          print_files(0,0);
        }
        else if (parallel_files == '\0') {
          for (local_9c = 0; local_9c < local_a0; local_9c = local_9c + 1) {
            ctx = (EVP_PKEY_CTX *)0x1;
            print_files(1,local_50 + (ulong)local_9c * 8);
          }
        }
        else {
          ctx = (EVP_PKEY_CTX *)(ulong)local_a0;
          print_files(ctx,local_50);
        }
        cleanup(ctx);
        if ((have_read_stdin != '\0') && (iVar5 = rpl_fclose(_stdin), iVar5 == -1)) {
          uVar8 = gettext("standard input");
          piVar9 = __errno_location();
          local_6c = *piVar9;
          local_70 = 1;
          local_28 = uVar8;
          error(1,local_6c,uVar8);
        }
                    /* WARNING: Subroutine does not return */
        exit((uint)failed_opens);
      }
      cVar4 = c_isdigit(local_98);
      if (cVar4 == '\0') break;
      if (local_68 <= local_58 + 1) {
        local_60 = (void *)xpalloc(local_60,&local_68,2,0xffffffffffffffff,1);
      }
      *(char *)((long)local_60 + local_58) = (char)local_98;
      *(undefined *)((long)local_60 + local_58 + 1) = 0;
      local_58 = local_58 + 1;
    }
    local_58 = 0;
    if (0x81 < local_98) goto switchD_0010085a_caseD_2;
    switch(local_98) {
    case 1:
      if (((first_page_number != 0) || (*_optarg != '+')) ||
         (cVar4 = first_last_page(0xfffffffe,0x2b,_optarg + 1), cVar4 != '\x01')) {
        *(char **)((ulong)local_a0 * 8 + local_50) = _optarg;
        local_a0 = local_a0 + 1;
      }
      break;
    case 0x44:
      date_format = _optarg;
      break;
    case 0x46:
    case 0x66:
      use_form_feed = 1;
      break;
    case 0x4a:
      join_lines = 1;
      break;
    case 0x4e:
      skip_count = 0;
      uVar8 = gettext("\'-N NUMBER\' invalid starting line number");
      start_line_num = getoptnum(_optarg,0x80000000,uVar8);
      break;
    case 0x53:
      bVar3 = false;
      col_sep_string = &DAT_0010426c;
      col_sep_length = 0;
      use_col_separator = '\x01';
      if (_optarg != (char *)0x0) {
        separator_string(_optarg);
      }
      break;
    case 0x54:
      extremities = 0;
      keep_FF = 0;
      break;
    case 0x57:
      bVar2 = false;
      truncate_lines = '\x01';
      uVar8 = gettext("\'-W PAGE_WIDTH\' invalid number of characters");
      chars_per_line = getoptnum(_optarg,1,uVar8);
      break;
    case 0x61:
      print_across_flag = '\x01';
      storing_columns = 0;
      break;
    case 0x62:
      balance_columns = 1;
      break;
    case 99:
      use_cntrl_prefix = 1;
      break;
    case 100:
      double_space = 1;
      break;
    case 0x65:
      if (_optarg != (char *)0x0) {
        getoptarg(_optarg,0x65,&input_tab_char,&chars_per_input_tab);
      }
      untabify_input = 1;
      break;
    case 0x68:
      custom_header = _optarg;
      break;
    case 0x69:
      if (_optarg != (char *)0x0) {
        getoptarg(_optarg,0x69,&output_tab_char,&chars_per_output_tab);
      }
      tabify_output = 1;
      break;
    case 0x6c:
      uVar8 = gettext("\'-l PAGE_LENGTH\' invalid number of lines");
      lines_per_page = getoptnum(_optarg,1,uVar8);
      break;
    case 0x6d:
      parallel_files = '\x01';
      storing_columns = 0;
      break;
    case 0x6e:
      numbered_lines = 1;
      if (_optarg != (char *)0x0) {
        getoptarg(_optarg,0x6e,&number_separator,&chars_per_number);
      }
      break;
    case 0x6f:
      uVar8 = gettext("\'-o MARGIN\' invalid line offset");
      chars_per_margin = getoptnum(_optarg,0,uVar8);
      break;
    case 0x72:
      ignore_failed_opens = 1;
      break;
    case 0x73:
      bVar1 = true;
      bVar3 = true;
      if ((use_col_separator != '\x01') && (_optarg != (char *)0x0)) {
        separator_string(_optarg);
      }
      break;
    case 0x74:
      extremities = 0;
      keep_FF = 1;
      break;
    case 0x76:
      use_esc_sequence = 1;
      break;
    case 0x77:
      bVar1 = true;
      bVar2 = true;
      uVar8 = gettext("\'-w PAGE_WIDTH\' invalid number of characters");
      local_94 = getoptnum(_optarg,1,uVar8);
      if (truncate_lines != '\x01') {
        chars_per_line = local_94;
      }
      break;
    case 0x80:
      goto switchD_0010085a_caseD_80;
    case 0x81:
      goto switchD_0010085a_caseD_81;
    default:
      if (local_98 == -0x83) {
LAB_00100d10:
        uVar8 = proper_name_lite("Roland Huebner","Roland Huebner");
        uVar6 = proper_name_lite("Pete TerMaat","Pete TerMaat");
        version_etc(_stdout,&DAT_001045a7,"GNU coreutils",_Version,uVar6,uVar8,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (local_98 == -0x82) {
        usage(0);
        goto LAB_00100d10;
      }
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 0xb:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2a:
    case 0x2b:
    case 0x2c:
    case 0x2d:
    case 0x2e:
    case 0x2f:
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
    case 0x3a:
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x45:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4f:
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x55:
    case 0x56:
    case 0x58:
    case 0x59:
    case 0x5a:
    case 0x5b:
    case 0x5c:
    case 0x5d:
    case 0x5e:
    case 0x5f:
    case 0x60:
    case 0x67:
    case 0x6a:
    case 0x6b:
    case 0x70:
    case 0x71:
    case 0x75:
    case 0x78:
    case 0x79:
    case 0x7a:
    case 0x7b:
    case 0x7c:
    case 0x7d:
    case 0x7e:
    case 0x7f:
switchD_0010085a_caseD_2:
      usage(1);
    }
  } while( true );
switchD_0010085a_caseD_81:
  if (_optarg == (char *)0x0) {
    local_48 = gettext("\'--pages=FIRST_PAGE[:LAST_PAGE]\' missing argument");
    local_90 = 1;
    local_8c = 0;
    error(1,0,local_48);
  }
  cVar4 = first_last_page(local_a4,0,_optarg);
  if (cVar4 != '\x01') {
    uVar8 = quote(_optarg);
    local_40 = gettext("invalid page range %s");
    local_88 = 1;
    local_84 = 0;
    error(1,0,local_40,uVar8);
switchD_0010085a_caseD_80:
    parse_column_count(_optarg);
    free(local_60);
    local_60 = (void *)0x0;
    local_68 = 0;
  }
  goto LAB_0010070f;
}

// Function: getoptnum
void getoptnum(undefined8 param_1,int param_2,undefined8 param_3) {
  undefined8 uVar1;
  
  if (param_2 < 1) {
    uVar1 = 0;
  }
  else {
    uVar1 = 4;
  }
  xnumtoimax(param_1,10,(long)param_2,0x7fffffff,&DAT_0010426c,param_3,0,uVar1);
  return;
}

// Function: getoptarg
void getoptarg(char *param_1,char param_2,char *param_3,undefined4 *param_4) {
  char cVar1;
  undefined8 uVar2;
  undefined4 uVar3;
  long in_FS_OFFSET;
  char *local_60;
  uint local_4c;
  long local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (*param_1 == '\0') {
    uVar2 = quote(param_1);
    local_30 = gettext("\'-%c\': Invalid argument: %s");
    error(0,0,local_30,(int)param_2,uVar2);
    usage(1);
  }
  cVar1 = c_isdigit((int)*param_1);
  local_60 = param_1;
  if (cVar1 != '\x01') {
    local_60 = param_1 + 1;
    *param_3 = *param_1;
  }
  if (*local_60 != '\0') {
    local_4c = xstrtol(local_60,0,10,&local_38,&DAT_0010426c);
    if (local_4c == 0) {
      if (local_38 < 1) {
        local_4c = 4;
      }
      else if (0x7fffffff < local_38) {
        local_4c = 1;
      }
    }
    if (local_4c != 0) {
      uVar2 = quote(local_60);
      local_28 = gettext("\'-%c\' extra characters or invalid number in the argument: %s");
      if ((local_4c & 1) == 0) {
        uVar3 = 0;
      }
      else {
        uVar3 = 0x4b;
      }
      error(0,uVar3,local_28,(int)param_2,uVar2);
      usage(1);
    }
    *param_4 = (int)local_38;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: init_parameters
void init_parameters(int param_1) {
  long lVar1;
  int iVar2;
  undefined8 uVar3;
  long lVar4;
  long in_FS_OFFSET;
  int local_30;
  int local_2c;
  int local_28;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  local_28 = 0;
  lines_per_body = lines_per_page + -10;
  if (lines_per_body < 1) {
    extremities = '\0';
    keep_FF = 1;
  }
  if (extremities != '\x01') {
    lines_per_body = lines_per_page;
  }
  if (double_space != '\0') {
    if (lines_per_body < 2) {
      lines_per_body = 1;
    }
    else {
      lines_per_body = lines_per_body / 2;
    }
  }
  if (param_1 == 0) {
    parallel_files = '\0';
  }
  if (parallel_files != '\0') {
    columns = param_1;
  }
  if (storing_columns != '\0') {
    balance_columns = 1;
  }
  if (columns < 2) {
    storing_columns = '\0';
  }
  else {
    if (use_col_separator == '\x01') {
      if (((join_lines != '\x01') && (col_sep_length == 1)) && (*col_sep_string == '\t')) {
        col_sep_string = column_separator;
      }
    }
    else {
      if (join_lines == '\0') {
        col_sep_string = column_separator;
      }
      else {
        col_sep_string = line_separator;
      }
      col_sep_length = 1;
      use_col_separator = '\x01';
    }
    truncate_lines = 1;
    if ((col_sep_length != 1) || (*col_sep_string != '\t')) {
      untabify_input = 1;
    }
    tabify_output = 1;
  }
  if (join_lines != '\0') {
    truncate_lines = 0;
  }
  if (numbered_lines != '\0') {
    line_count = start_line_num;
    if (number_separator == '\t') {
      number_width = chars_per_number + (8 - (int)chars_per_number % 8);
    }
    else {
      number_width = chars_per_number + 1;
    }
    if (parallel_files != '\0') {
      local_28 = number_width;
    }
  }
  lVar4 = (long)col_sep_length * (long)(columns + -1);
  local_30 = (int)lVar4;
  if (local_30 != lVar4) {
    local_30 = 0x7fffffff;
  }
  local_2c = (chars_per_line - local_28) - local_30;
  if (SBORROW4(chars_per_line - local_28,local_30)) {
    local_2c = 0;
  }
  chars_per_column = local_2c / columns;
  if (chars_per_column < 1) {
    uVar3 = gettext("page width too narrow",columns,(long)local_2c % (long)columns & 0xffffffff);
    error(1,0,uVar3);
  }
  if (numbered_lines != '\0') {
    free(number_buff);
    if (chars_per_number < 0xc) {
      lVar4 = 0xc;
    }
    else {
      lVar4 = (long)(int)chars_per_number + 1;
    }
    number_buff = (void *)xmalloc(lVar4);
  }
  free(clump_buff);
  iVar2 = chars_per_input_tab;
  if (chars_per_input_tab < 8) {
    iVar2 = 8;
  }
  clump_buff = (void *)xmalloc((long)iVar2);
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: init_fps
undefined8 init_fps(int param_1,undefined8 *param_2) {
  FILE *pFVar1;
  FILE **ppFVar2;
  char cVar3;
  int iVar4;
  FILE *pFVar5;
  undefined8 *local_38;
  int local_28;
  int local_24;
  FILE **local_20;
  
  total_files = 0;
  free(column_vector);
  local_20 = (FILE **)xnmalloc((long)columns,0x40);
  column_vector = local_20;
  local_38 = param_2;
  local_28 = param_1;
  if (parallel_files == '\0') {
    if (param_1 < 1) {
      pFVar5 = (FILE *)gettext("standard input");
      local_20[1] = pFVar5;
      *local_20 = _stdin;
      have_read_stdin = 1;
      *(undefined4 *)(local_20 + 2) = 0;
      *(undefined *)((long)local_20 + 0x39) = 0;
      total_files = total_files + 1;
      init_header(&DAT_0010426c,0xffffffff);
      *(undefined4 *)((long)local_20 + 0x2c) = 0;
    }
    else {
      cVar3 = open_file(*param_2,local_20);
      if (cVar3 != '\x01') {
        return 0;
      }
      iVar4 = fileno_unlocked(*local_20);
      init_header(*param_2,iVar4);
      *(undefined4 *)((long)local_20 + 0x2c) = 0;
    }
    pFVar5 = local_20[1];
    pFVar1 = *local_20;
    local_24 = columns;
    ppFVar2 = local_20;
    while( true ) {
      local_20 = ppFVar2 + 8;
      local_24 = local_24 + -1;
      if (local_24 == 0) break;
      ppFVar2[9] = pFVar5;
      *local_20 = pFVar1;
      *(undefined4 *)(ppFVar2 + 10) = 0;
      *(undefined *)((long)ppFVar2 + 0x79) = 0;
      *(undefined4 *)((long)ppFVar2 + 0x6c) = 0;
      ppFVar2 = local_20;
    }
  }
  else {
    while (local_28 != 0) {
      cVar3 = open_file(*local_38,local_20);
      if (cVar3 != '\x01') {
        local_20 = local_20 + -8;
        columns = columns + -1;
      }
      local_20 = local_20 + 8;
      local_38 = local_38 + 1;
      local_28 = local_28 + -1;
    }
    if (columns == 0) {
      return 0;
    }
    init_header(&DAT_0010426c,0xffffffff);
  }
  files_ready_to_read = total_files;
  return 1;
}

// Function: init_funcs
void init_funcs(void) {
  undefined uVar1;
  int local_1c;
  int local_18;
  int local_14;
  long local_10;
  
  if (truncate_lines == '\x01') {
    if ((parallel_files == '\0') || (numbered_lines == '\0')) {
      local_14 = chars_per_margin + chars_per_column;
    }
    else {
      local_14 = number_width + chars_per_column + chars_per_margin;
    }
  }
  else {
    local_14 = 0;
  }
  local_18 = chars_per_margin + col_sep_length;
  local_10 = column_vector;
  for (local_1c = 1; local_1c < columns; local_1c = local_1c + 1) {
    if (storing_columns == '\0') {
      *(code **)(local_10 + 0x20) = print_char;
      *(code **)(local_10 + 0x18) = read_line;
    }
    else {
      *(code **)(local_10 + 0x20) = store_char;
      *(code **)(local_10 + 0x18) = print_stored;
    }
    if ((numbered_lines == '\0') || ((parallel_files == '\x01' && (local_1c != 1)))) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
    *(undefined *)(local_10 + 0x38) = uVar1;
    *(int *)(local_10 + 0x34) = local_18;
    if (truncate_lines == '\x01') {
      local_18 = local_14 + col_sep_length;
      local_14 = local_18 + chars_per_column;
    }
    else {
      local_18 = 0;
      local_14 = 0;
    }
    local_10 = local_10 + 0x40;
  }
  if ((storing_columns == '\0') || (balance_columns == '\0')) {
    *(code **)(local_10 + 0x20) = print_char;
    *(code **)(local_10 + 0x18) = read_line;
  }
  else {
    *(code **)(local_10 + 0x20) = store_char;
    *(code **)(local_10 + 0x18) = print_stored;
  }
  if ((numbered_lines == '\0') || ((parallel_files == '\x01' && (local_1c != 1)))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  *(undefined *)(local_10 + 0x38) = uVar1;
  *(int *)(local_10 + 0x34) = local_18;
  return;
}

// Function: open_file
undefined8 open_file(long param_1,long *param_2) {
  char cVar1;
  long lVar2;
  undefined8 uVar3;
  int *piVar4;
  
  cVar1 = streq(param_1,&DAT_001048e3);
  if (cVar1 == '\0') {
    param_2[1] = param_1;
    lVar2 = fopen_safer(param_1,&DAT_001048e5);
    *param_2 = lVar2;
  }
  else {
    lVar2 = gettext("standard input");
    param_2[1] = lVar2;
    *param_2 = _stdin;
    have_read_stdin = 1;
  }
  if (*param_2 == 0) {
    failed_opens = 1;
    if (ignore_failed_opens != '\x01') {
      uVar3 = quotearg_n_style_colon(0,3,param_1);
      piVar4 = __errno_location();
      error(0,*piVar4,&DAT_001048e7,uVar3);
    }
    uVar3 = 0;
  }
  else {
    fadvise(*param_2,2);
    *(undefined4 *)(param_2 + 2) = 0;
    *(undefined *)((long)param_2 + 0x39) = 0;
    total_files = total_files + 1;
    uVar3 = 1;
  }
  return uVar3;
}

// Function: close_file
void close_file(FILE **param_1) {
  int iVar1;
  int *piVar2;
  undefined8 uVar3;
  int local_28;
  int local_24;
  long local_18;
  
  if (*(int *)(param_1 + 2) != 3) {
    piVar2 = __errno_location();
    local_24 = *piVar2;
    iVar1 = ferror_unlocked(*param_1);
    if (iVar1 == 0) {
      local_24 = 0;
    }
    iVar1 = fileno_unlocked(*param_1);
    if (iVar1 == 0) {
      clearerr_unlocked(*param_1);
    }
    else {
      iVar1 = rpl_fclose(*param_1);
      if ((iVar1 != 0) && (local_24 == 0)) {
        piVar2 = __errno_location();
        local_24 = *piVar2;
      }
    }
    if (local_24 != 0) {
      uVar3 = quotearg_n_style_colon(0,3,param_1[1]);
      error(1,local_24,&DAT_001048e7,uVar3);
    }
    if (parallel_files == '\x01') {
      *(undefined4 *)(param_1 + 2) = 3;
      *(undefined4 *)(param_1 + 6) = 0;
    }
    else {
      local_18 = column_vector;
      for (local_28 = columns; local_28 != 0; local_28 = local_28 + -1) {
        *(undefined4 *)(local_18 + 0x10) = 3;
        if (*(int *)(local_18 + 0x2c) == 0) {
          *(undefined4 *)(local_18 + 0x30) = 0;
        }
        local_18 = local_18 + 0x40;
      }
    }
    files_ready_to_read = files_ready_to_read + -1;
  }
  return;
}

// Function: hold_file
void hold_file(long param_1) {
  int local_14;
  long local_10;
  
  if (parallel_files == '\x01') {
    *(undefined4 *)(param_1 + 0x10) = 2;
  }
  else {
    local_10 = column_vector;
    for (local_14 = columns; local_14 != 0; local_14 = local_14 + -1) {
      if (storing_columns == '\0') {
        *(undefined4 *)(local_10 + 0x10) = 2;
      }
      else {
        *(undefined4 *)(local_10 + 0x10) = 1;
      }
      local_10 = local_10 + 0x40;
    }
  }
  *(undefined4 *)(param_1 + 0x30) = 0;
  files_ready_to_read = files_ready_to_read + -1;
  return;
}

// Function: reset_status
void reset_status(void) {
  int local_14;
  long local_10;
  
  local_10 = column_vector;
  for (local_14 = columns; local_14 != 0; local_14 = local_14 + -1) {
    if (*(int *)(local_10 + 0x10) == 2) {
      *(undefined4 *)(local_10 + 0x10) = 0;
      files_ready_to_read = files_ready_to_read + 1;
    }
    local_10 = local_10 + 0x40;
  }
  if (storing_columns != '\0') {
    if (*(int *)(column_vector + 0x10) == 3) {
      files_ready_to_read = 0;
    }
    else {
      files_ready_to_read = 1;
    }
  }
  return;
}

// Function: print_files
void print_files(undefined4 param_1,undefined8 param_2) {
  char cVar1;
  
  init_parameters(param_1);
  cVar1 = init_fps(param_1,param_2);
  if (cVar1 == '\x01') {
    if (storing_columns != '\0') {
      init_store_cols();
    }
    if (first_page_number < 2) {
      page_number = 1;
    }
    else {
      cVar1 = skip_to_page(first_page_number);
      if (cVar1 != '\x01') {
        return;
      }
      page_number = first_page_number;
    }
    init_funcs();
    line_number = line_count;
    do {
      cVar1 = print_page();
    } while (cVar1 != '\0');
  }
  return;
}

// Function: init_header
void init_header(undefined1 *param_1,int param_2) {
  ulong uVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  long lVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  int local_144;
  char *local_128;
  undefined local_118 [16];
  undefined local_108 [64];
  stat local_c8;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  cVar2 = streq(param_1,&DAT_001048e3);
  local_144 = param_2;
  if (cVar2 != '\0') {
    local_144 = -1;
  }
  if ((local_144 < 0) || (iVar3 = fstat(local_144,&local_c8), iVar3 != 0)) {
    if (timespec_0 == 0) {
      gettime(&timespec_0);
    }
    local_118._8_8_ = DAT_001040c8;
    local_118._0_8_ = timespec_0;
  }
  else {
    local_118 = get_stat_mtime(&local_c8);
  }
  uVar1 = local_118._8_8_;
  lVar6 = localtime_rz(localtz,local_118,local_108);
  if (lVar6 == 0) {
    local_128 = (char *)xmalloc(0x21);
    uVar7 = timetostr(local_118._0_8_,local_38);
    sprintf(local_128,"%s.%09d",uVar7,uVar1 & 0xffffffff);
  }
  else {
    lVar6 = nstrftime(0,0xffffffffffffffff,date_format,local_108,localtz,uVar1 & 0xffffffff);
    local_128 = (char *)xmalloc(lVar6 + 1);
    nstrftime(local_128,lVar6 + 1,date_format,local_108,localtz,uVar1 & 0xffffffff);
  }
  free(date_text);
  iVar3 = chars_per_line;
  date_text = local_128;
  file_text = custom_header;
  if ((custom_header == (undefined1 *)0x0) && (file_text = param_1, local_144 < 0)) {
    file_text = &DAT_0010426c;
  }
  iVar4 = gnu_mbswidth(local_128,0);
  iVar5 = gnu_mbswidth(file_text,0);
  header_width_available = (iVar3 - iVar4) - iVar5;
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: init_page
void init_page(void) {
  int local_14;
  long local_10;
  
  if (storing_columns == '\0') {
    local_10 = column_vector;
    for (local_14 = columns; local_14 != 0; local_14 = local_14 + -1) {
      if (*(int *)(local_10 + 0x10) == 0) {
        *(undefined4 *)(local_10 + 0x30) = lines_per_body;
      }
      else {
        *(undefined4 *)(local_10 + 0x30) = 0;
      }
      local_10 = local_10 + 0x40;
    }
  }
  else {
    store_columns();
    local_10 = column_vector;
    local_14 = columns;
    while (local_14 = local_14 + -1, local_14 != 0) {
      *(undefined4 *)(local_10 + 0x30) = *(undefined4 *)(local_10 + 0x2c);
      local_10 = local_10 + 0x40;
    }
    if (balance_columns == '\0') {
      if (*(int *)(local_10 + 0x10) == 0) {
        *(undefined4 *)(local_10 + 0x30) = lines_per_body;
      }
      else {
        *(undefined4 *)(local_10 + 0x30) = 0;
      }
    }
    else {
      *(undefined4 *)(local_10 + 0x30) = *(undefined4 *)(local_10 + 0x2c);
    }
  }
  return;
}

// Function: align_column
void align_column(long param_1) {
  padding_not_printed = *(int *)(param_1 + 0x34);
  if (col_sep_length < padding_not_printed) {
    pad_across_to(padding_not_printed - col_sep_length);
    padding_not_printed = 0;
  }
  if (use_col_separator != '\0') {
    print_sep_string();
  }
  if (*(char *)(param_1 + 0x38) != '\0') {
    add_line_number(param_1);
  }
  return;
}

// Function: print_page
undefined8 print_page(void) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  bool local_19;
  int local_18;
  int local_14;
  long local_10;
  
  init_page();
  iVar2 = cols_ready_to_print();
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    if (extremities != '\0') {
      print_a_header = 1;
    }
    pad_vertically = 0;
    local_19 = false;
    local_14 = lines_per_body;
    if (double_space != '\0') {
      local_14 = lines_per_body << 1;
    }
    while ((0 < local_14 && (iVar2 = cols_ready_to_print(), iVar2 != 0))) {
      output_position = 0;
      spaces_not_printed = 0;
      separators_not_printed = 0;
      pad_vertically = 0;
      align_empty_cols = 0;
      empty_line = '\x01';
      local_10 = column_vector;
      for (local_18 = 1; local_18 <= columns; local_18 = local_18 + 1) {
        input_position = 0;
        if ((*(int *)(local_10 + 0x30) < 1) && (*(int *)(local_10 + 0x10) != 1)) {
          if (parallel_files != '\0') {
            if (empty_line == '\0') {
              align_column(local_10);
            }
            else {
              align_empty_cols = 1;
            }
          }
        }
        else {
          FF_only = '\0';
          padding_not_printed = *(undefined4 *)(local_10 + 0x34);
          cVar1 = (**(code **)(local_10 + 0x18))(local_10);
          if (cVar1 != '\x01') {
            read_rest_of_line(local_10);
          }
          local_19 = (pad_vertically | local_19) != 0;
          *(int *)(local_10 + 0x30) = *(int *)(local_10 + 0x30) + -1;
          if ((*(int *)(local_10 + 0x30) < 1) && (iVar2 = cols_ready_to_print(), iVar2 == 0)) break;
          if ((parallel_files != '\0') && (*(int *)(local_10 + 0x10) != 0)) {
            if (empty_line == '\0') {
              if ((*(int *)(local_10 + 0x10) == 3) ||
                 ((*(int *)(local_10 + 0x10) == 2 && (FF_only != '\0')))) {
                align_column(local_10);
              }
            }
            else {
              align_empty_cols = 1;
            }
          }
        }
        if (use_col_separator != '\0') {
          separators_not_printed = separators_not_printed + 1;
        }
        local_10 = local_10 + 0x40;
      }
      if (pad_vertically != 0) {
        putchar_unlocked(10);
        local_14 = local_14 + -1;
      }
      iVar2 = cols_ready_to_print();
      if ((iVar2 == 0) && (extremities != '\x01')) break;
      if ((double_space != '\0') && (local_19 != false)) {
        putchar_unlocked(10);
        local_14 = local_14 + -1;
      }
    }
    if (local_14 == 0) {
      local_10 = column_vector;
      for (local_18 = 1; local_18 <= columns; local_18 = local_18 + 1) {
        if (*(int *)(local_10 + 0x10) == 0) {
          *(undefined *)(local_10 + 0x39) = 1;
        }
        local_10 = local_10 + 0x40;
      }
    }
    pad_vertically = local_19;
    if ((local_19 == false) || (extremities == '\0')) {
      if ((keep_FF != '\0') && (print_a_FF != '\0')) {
        putchar_unlocked(0xc);
        print_a_FF = '\0';
      }
    }
    else {
      pad_down(local_14 + 5);
    }
    iVar2 = ferror_unlocked(_stdout);
    if (iVar2 != 0) {
      write_error();
    }
    page_number = page_number + 1;
    if (last_page_number < page_number) {
      uVar3 = 0;
    }
    else {
      reset_status();
      uVar3 = 1;
    }
  }
  return uVar3;
}

// Function: init_store_cols
void init_store_cols(void) {
  long lVar1;
  long lVar2;
  int iVar3;
  int iVar4;
  long in_FS_OFFSET;
  int local_1c;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  iVar3 = (int)((long)columns * (long)lines_per_body);
  if (((((long)iVar3 != (long)columns * (long)lines_per_body) ||
       (local_1c = iVar3 + 1, SCARRY4(iVar3,1))) || (SCARRY4(chars_per_column,1))) ||
     (lVar2 = (long)(chars_per_column + 1) * (long)iVar3, iVar4 = (int)lVar2, iVar4 != lVar2)) {
    integer_overflow();
  }
  else {
    buff_allocated = (long)(int)(use_col_separator + 1) * (long)iVar4;
  }
  free(line_vector);
  line_vector = (void *)xnmalloc((long)local_1c,4);
  free(end_vector);
  end_vector = (void *)xnmalloc((long)iVar3,4);
  free(buff);
  buff = (void *)ximalloc(buff_allocated);
  if (lVar1 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: store_columns
void store_columns(void) {
  char cVar1;
  int local_24;
  int local_20;
  uint local_1c;
  int local_18;
  int local_14;
  long local_10;
  
  local_1c = 0;
  buff_current = 0;
  local_18 = 0;
  if (balance_columns == '\0') {
    local_14 = columns + -1;
  }
  else {
    local_14 = columns;
  }
  local_10 = column_vector;
  for (local_24 = 1; local_24 <= local_14; local_24 = local_24 + 1) {
    *(undefined4 *)(local_10 + 0x2c) = 0;
    local_10 = local_10 + 0x40;
  }
  local_24 = 1;
  local_10 = column_vector;
  while ((local_24 <= local_14 && (files_ready_to_read != 0))) {
    *(uint *)(local_10 + 0x28) = local_1c;
    local_20 = lines_per_body;
    while ((local_20 != 0 && (files_ready_to_read != 0))) {
      if (*(int *)(local_10 + 0x10) == 0) {
        input_position = 0;
        cVar1 = read_line(local_10);
        if (cVar1 != '\x01') {
          read_rest_of_line(local_10);
        }
        if ((*(int *)(local_10 + 0x10) == 0) || (local_18 != buff_current)) {
          *(int *)(local_10 + 0x2c) = *(int *)(local_10 + 0x2c) + 1;
          *(int *)((ulong)local_1c * 4 + line_vector) = local_18;
          *(undefined4 *)(end_vector + (ulong)local_1c * 4) = input_position;
          local_18 = buff_current;
          local_1c = local_1c + 1;
        }
      }
      local_20 = local_20 + -1;
    }
    local_24 = local_24 + 1;
    local_10 = local_10 + 0x40;
  }
  *(int *)((ulong)local_1c * 4 + line_vector) = local_18;
  if (balance_columns != '\0') {
    balance(local_1c);
  }
  return;
}

// Function: balance
void balance(int param_1) {
  int local_1c;
  int local_18;
  int local_14;
  long local_10;
  
  local_14 = 0;
  local_10 = column_vector;
  for (local_1c = 1; local_1c <= columns; local_1c = local_1c + 1) {
    local_18 = param_1 / columns;
    if (local_1c <= param_1 % columns) {
      local_18 = local_18 + 1;
    }
    *(int *)(local_10 + 0x2c) = local_18;
    *(int *)(local_10 + 0x28) = local_14;
    local_14 = local_14 + local_18;
    local_10 = local_10 + 0x40;
  }
  return;
}

// Function: store_char
void store_char(undefined param_1) {
  ulong uVar1;
  
  if (buff_allocated <= (long)(ulong)buff_current) {
    buff = xpalloc(buff,&buff_allocated,1,0xffffffffffffffff,1);
  }
  uVar1 = (ulong)buff_current;
  buff_current = buff_current + 1;
  *(undefined *)(buff + uVar1) = param_1;
  return;
}

// Function: add_line_number
void add_line_number(long param_1) {
  int iVar1;
  uint local_18;
  char *local_10;
  
  iVar1 = sprintf(number_buff,"%*d",(ulong)chars_per_number,(ulong)line_number);
  line_number = line_number + 1;
  local_10 = number_buff + (int)(iVar1 - chars_per_number);
  for (local_18 = chars_per_number; 0 < (int)local_18; local_18 = local_18 - 1) {
    (**(code **)(param_1 + 0x20))((int)*local_10);
    local_10 = local_10 + 1;
  }
  if (columns < 2) {
    (**(code **)(param_1 + 0x20))((int)number_separator);
    if (number_separator == '\t') {
      output_position =
           output_position + (chars_per_output_tab - output_position % chars_per_output_tab);
    }
  }
  else if (number_separator == '\t') {
    local_18 = number_width - chars_per_number;
    while (0 < (int)local_18) {
      (**(code **)(param_1 + 0x20))(0x20);
      local_18 = local_18 + -1;
    }
  }
  else {
    (**(code **)(param_1 + 0x20))((int)number_separator);
  }
  if ((truncate_lines != '\0') && (parallel_files != '\x01')) {
    input_position = number_width + input_position;
  }
  return;
}

// Function: pad_across_to
void pad_across_to(int param_1) {
  int iVar1;
  int local_c;
  
  local_c = output_position;
  if (tabify_output == '\0') {
    while (local_c = local_c + 1, iVar1 = param_1, local_c <= param_1) {
      putchar_unlocked(0x20);
    }
  }
  else {
    spaces_not_printed = param_1 - output_position;
    iVar1 = output_position;
  }
  output_position = iVar1;
  return;
}

// Function: pad_down
void pad_down(int param_1) {
  int local_c;
  
  local_c = param_1;
  if (use_form_feed == '\0') {
    for (; local_c != 0; local_c = local_c + -1) {
      putchar_unlocked(10);
    }
  }
  else {
    putchar_unlocked(0xc);
  }
  return;
}

// Function: read_rest_of_line
void read_rest_of_line(FILE **param_1) {
  FILE *__stream;
  int iVar1;
  
  __stream = *param_1;
  while( true ) {
    iVar1 = getc_unlocked(__stream);
    if (iVar1 == 10) {
      return;
    }
    if (iVar1 == 0xc) break;
    if (iVar1 == -1) {
      close_file(param_1);
      return;
    }
  }
  iVar1 = getc_unlocked(__stream);
  if (iVar1 != 10) {
    ungetc(iVar1,__stream);
  }
  if (keep_FF != '\0') {
    print_a_FF = 1;
  }
  hold_file(param_1);
  return;
}

// Function: skip_read
void skip_read(FILE **param_1,int param_2) {
  FILE *__stream;
  int __c;
  bool bVar1;
  int local_20;
  int local_1c;
  long local_18;
  
  __stream = *param_1;
  local_20 = getc_unlocked(__stream);
  if (((local_20 == 0xc) && (*(char *)((long)param_1 + 0x39) != '\0')) &&
     (local_20 = getc_unlocked(__stream), local_20 == 10)) {
    local_20 = getc_unlocked(__stream);
  }
  *(undefined *)((long)param_1 + 0x39) = 0;
  bVar1 = local_20 != 0xc;
  if (last_line != '\0') {
    *(undefined *)((long)param_1 + 0x39) = 1;
  }
  while( true ) {
    if (local_20 == 10) goto LAB_00102e27;
    if (local_20 == 0xc) break;
    if (local_20 == -1) {
      close_file(param_1);
      goto LAB_00102e27;
    }
    local_20 = getc_unlocked(__stream);
  }
  if (last_line != '\0') {
    if (parallel_files == '\x01') {
      *(undefined *)((long)param_1 + 0x39) = 0;
    }
    else {
      local_18 = column_vector;
      for (local_1c = columns; local_1c != 0; local_1c = local_1c + -1) {
        *(undefined *)(local_18 + 0x39) = 0;
        local_18 = local_18 + 0x40;
      }
    }
  }
  __c = getc_unlocked(__stream);
  if (__c != 10) {
    ungetc(__c,__stream);
  }
  hold_file(param_1);
LAB_00102e27:
  if (((skip_count != '\0') && ((parallel_files != '\x01' || (param_2 == 1)))) && (bVar1)) {
    line_count = line_count + 1;
  }
  return;
}

// Function: print_white_space
void print_white_space(void) {
  int iVar1;
  int iVar2;
  int local_14;
  
  local_14 = output_position;
  iVar1 = output_position + spaces_not_printed;
  while ((1 < iVar1 - local_14 &&
         (iVar2 = local_14 + (chars_per_output_tab - local_14 % chars_per_output_tab),
         iVar2 <= iVar1))) {
    putchar_unlocked((int)output_tab_char);
    local_14 = iVar2;
  }
  while (local_14 = local_14 + 1, local_14 <= iVar1) {
    putchar_unlocked(0x20);
  }
  output_position = iVar1;
  spaces_not_printed = 0;
  return;
}

// Function: print_sep_string
void print_sep_string(void) {
  int iVar1;
  int local_14;
  char *local_10;
  
  local_10 = col_sep_string;
  local_14 = col_sep_length;
  if (separators_not_printed < 1) {
    if (0 < spaces_not_printed) {
      print_white_space();
    }
  }
  else {
    for (; 0 < separators_not_printed; separators_not_printed = separators_not_printed + -1) {
      while( true ) {
        iVar1 = local_14 + -1;
        if (local_14 < 1) break;
        if (*local_10 == ' ') {
          spaces_not_printed = spaces_not_printed + 1;
          local_14 = iVar1;
          local_10 = local_10 + 1;
        }
        else {
          if (0 < spaces_not_printed) {
            print_white_space();
          }
          putchar_unlocked((int)*local_10);
          output_position = output_position + 1;
          local_14 = iVar1;
          local_10 = local_10 + 1;
        }
      }
      if (0 < spaces_not_printed) {
        print_white_space();
      }
      local_14 = iVar1;
    }
  }
  return;
}

// Function: print_clump
void print_clump(long param_1,int param_2,char *param_3) {
  int iVar1;
  char *local_20;
  int local_14;
  
  local_20 = param_3;
  local_14 = param_2;
  while (local_14 != 0) {
    (**(code **)(param_1 + 0x20))((int)*local_20);
    local_20 = local_20 + 1;
    local_14 = local_14 + -1;
  }
  iVar1 = ferror_unlocked(_stdout);
  if (iVar1 != 0) {
    write_error();
  }
  return;
}

// Function: print_char
void print_char(char param_1) {
  ushort *puVar1;
  byte bVar2;
  ushort **ppuVar3;
  
  if (tabify_output != '\0') {
    if (param_1 == ' ') {
      spaces_not_printed = spaces_not_printed + 1;
      return;
    }
    if (0 < spaces_not_printed) {
      print_white_space();
    }
    ppuVar3 = __ctype_b_loc();
    puVar1 = *ppuVar3;
    bVar2 = to_uchar((int)param_1);
    if ((puVar1[bVar2] & 0x4000) == 0) {
      if (param_1 == '\b') {
        output_position = output_position + -1;
      }
    }
    else {
      output_position = output_position + 1;
    }
  }
  putchar_unlocked((int)param_1);
  return;
}

// Function: skip_to_page
bool skip_to_page(ulong param_1) {
  undefined8 uVar1;
  int local_30;
  int local_2c;
  ulong local_20;
  long local_18;
  
  local_20 = 1;
  do {
    if (param_1 <= local_20) {
LAB_00103258:
      return 0 < files_ready_to_read;
    }
    for (local_2c = 1; local_2c < lines_per_body; local_2c = local_2c + 1) {
      local_18 = column_vector;
      for (local_30 = 1; local_30 <= columns; local_30 = local_30 + 1) {
        if (*(int *)(local_18 + 0x10) == 0) {
          skip_read(local_18,local_30);
        }
        local_18 = local_18 + 0x40;
      }
    }
    last_line = 1;
    local_18 = column_vector;
    for (local_30 = 1; local_30 <= columns; local_30 = local_30 + 1) {
      if (*(int *)(local_18 + 0x10) == 0) {
        skip_read(local_18,local_30);
      }
      local_18 = local_18 + 0x40;
    }
    if (storing_columns != '\0') {
      local_18 = column_vector;
      for (local_30 = 1; local_30 <= columns; local_30 = local_30 + 1) {
        if (*(int *)(local_18 + 0x10) != 3) {
          *(undefined4 *)(local_18 + 0x10) = 2;
        }
        local_18 = local_18 + 0x40;
      }
    }
    reset_status();
    last_line = 0;
    if (files_ready_to_read < 1) {
      uVar1 = gettext("starting page number %ju exceeds page count %ju");
      error(0,0,uVar1,param_1,local_20);
      goto LAB_00103258;
    }
    local_20 = local_20 + 1;
  } while( true );
}

// Function: print_header
void print_header(void) {
  long lVar1;
  int iVar2;
  undefined8 uVar3;
  char *__format;
  int iVar4;
  long in_FS_OFFSET;
  char local_138 [280];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  output_position = 0;
  pad_across_to(chars_per_margin);
  print_white_space();
  if (page_number == 0) {
    uVar3 = gettext("page number overflow");
    error(1,0,uVar3);
  }
  lVar1 = page_number;
  __format = (char *)gettext("Page %ju");
  sprintf(local_138,__format,lVar1);
  iVar4 = header_width_available;
  iVar2 = gnu_mbswidth(local_138,0);
  iVar4 = iVar4 - iVar2;
  if (iVar4 < 0) {
    iVar4 = 0;
  }
  printf("\n\n%*s%s%*s%s%*s%s\n\n\n",(ulong)chars_per_margin,&DAT_0010426c,date_text,
         (ulong)(uint)(iVar4 >> 1),&DAT_001042af,file_text,(ulong)(uint)(iVar4 - (iVar4 >> 1)),
         &DAT_001042af,local_138);
  print_a_header = 0;
  output_position = 0;
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: read_line
undefined8 read_line(FILE **param_1) {
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined8 uVar4;
  int local_24;
  undefined4 local_20;
  int local_1c;
  long local_10;
  
  local_20 = 0;
  local_24 = getc_unlocked(*param_1);
  iVar2 = input_position;
  if (((local_24 == 0xc) && (*(char *)((long)param_1 + 0x39) != '\0')) &&
     (local_24 = getc_unlocked(*param_1), local_24 == 10)) {
    local_24 = getc_unlocked(*param_1);
  }
  *(undefined *)((long)param_1 + 0x39) = 0;
  if (local_24 == 0xc) {
    iVar2 = getc_unlocked(*param_1);
    if (iVar2 != 10) {
      ungetc(iVar2,*param_1);
    }
    FF_only = 1;
    if ((print_a_header == '\0') || (storing_columns == '\x01')) {
      if (keep_FF != '\0') {
        print_a_FF = 1;
      }
    }
    else {
      pad_vertically = 1;
      print_header();
    }
    hold_file(param_1);
    return 1;
  }
  if (local_24 < 0xd) {
    if (local_24 == -1) {
      close_file(param_1);
      return 1;
    }
    if (local_24 == 10) goto LAB_0010355b;
  }
  local_20 = char_to_clump((int)(char)local_24);
LAB_0010355b:
  if ((truncate_lines == '\0') || (input_position <= chars_per_column)) {
    if (param_1[4] != (FILE *)store_char) {
      pad_vertically = 1;
      if ((print_a_header != '\0') && (storing_columns != '\x01')) {
        print_header();
      }
      iVar2 = separators_not_printed;
      if ((parallel_files != '\0') && (align_empty_cols != '\0')) {
        separators_not_printed = 0;
        local_10 = column_vector;
        for (local_1c = 1; local_1c <= iVar2; local_1c = local_1c + 1) {
          align_column(local_10);
          separators_not_printed = separators_not_printed + 1;
          local_10 = local_10 + 0x40;
        }
        padding_not_printed = *(int *)((long)param_1 + 0x34);
        if (truncate_lines == '\0') {
          spaces_not_printed = 0;
        }
        else {
          spaces_not_printed = chars_per_column;
        }
        align_empty_cols = '\0';
      }
      if (col_sep_length < padding_not_printed) {
        pad_across_to(padding_not_printed - col_sep_length);
        padding_not_printed = 0;
      }
      if (use_col_separator != '\0') {
        print_sep_string();
      }
    }
    if (*(char *)(param_1 + 7) != '\0') {
      add_line_number(param_1);
    }
    empty_line = 0;
    if (local_24 == 10) {
      uVar4 = 1;
    }
    else {
      print_clump(param_1,local_20,clump_buff);
      while (iVar1 = getc_unlocked(*param_1), iVar2 = input_position, iVar1 != 0xc) {
        if (iVar1 < 0xd) {
          if (iVar1 == -1) {
            close_file(param_1);
            return 1;
          }
          if (iVar1 == 10) {
            return 1;
          }
        }
        uVar3 = char_to_clump((int)(char)iVar1);
        if ((truncate_lines != '\0') && (chars_per_column < input_position)) {
          input_position = iVar2;
          return 0;
        }
        print_clump(param_1,uVar3,clump_buff);
      }
      iVar2 = getc_unlocked(*param_1);
      if (iVar2 != 10) {
        ungetc(iVar2,*param_1);
      }
      if (keep_FF != '\0') {
        print_a_FF = 1;
      }
      hold_file(param_1);
      uVar4 = 1;
    }
  }
  else {
    uVar4 = 0;
    input_position = iVar2;
  }
  return uVar4;
}

// Function: print_stored
undefined8 print_stored(long param_1) {
  int iVar1;
  char *pcVar2;
  int local_28;
  long local_20;
  char *local_18;
  
  iVar1 = *(int *)(param_1 + 0x28);
  *(int *)(param_1 + 0x28) = iVar1 + 1;
  local_18 = (char *)(*(int *)(line_vector + (long)iVar1 * 4) + buff);
  pcVar2 = (char *)(*(int *)(line_vector + ((long)iVar1 + 1) * 4) + buff);
  pad_vertically = 1;
  if (print_a_header != '\0') {
    print_header();
  }
  if (*(int *)(param_1 + 0x10) == 1) {
    local_20 = column_vector;
    for (local_28 = 1; local_28 <= columns; local_28 = local_28 + 1) {
      *(undefined4 *)(local_20 + 0x10) = 2;
      local_20 = local_20 + 0x40;
    }
    if (*(int *)(column_vector + 0x30) < 1) {
      if (extremities == '\x01') {
        return 1;
      }
      pad_vertically = 0;
      return 1;
    }
  }
  if (col_sep_length < padding_not_printed) {
    pad_across_to(padding_not_printed - col_sep_length);
    padding_not_printed = 0;
  }
  if (use_col_separator != '\0') {
    print_sep_string();
  }
  while (local_18 != pcVar2) {
    print_char((int)*local_18);
    local_18 = local_18 + 1;
  }
  if ((spaces_not_printed == 0) &&
     (output_position = *(int *)(end_vector + (long)iVar1 * 4) + *(int *)(param_1 + 0x34),
     *(int *)(param_1 + 0x34) - col_sep_length == chars_per_margin)) {
    output_position = output_position - col_sep_length;
  }
  return 1;
}

// Function: char_to_clump
int char_to_clump(byte param_1) {
  byte *pbVar1;
  ushort **ppuVar2;
  long in_FS_OFFSET;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  byte *local_20;
  byte local_14 [4];
  long local_10;
  
  pbVar1 = clump_buff;
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_20 = clump_buff;
  local_24 = 8;
  local_2c = chars_per_input_tab;
  if ((param_1 == input_tab_char) || (local_2c = local_24, param_1 == 9)) {
    local_2c = local_2c - input_position % local_2c;
    local_30 = local_2c;
    if (untabify_input == '\0') {
      *clump_buff = param_1;
      local_28 = 1;
    }
    else {
      for (; local_28 = local_2c, local_30 != 0; local_30 = local_30 + -1) {
        *local_20 = 0x20;
        local_20 = local_20 + 1;
      }
    }
  }
  else {
    ppuVar2 = __ctype_b_loc();
    if (((*ppuVar2)[param_1] & 0x4000) == 0) {
      if (use_esc_sequence == '\0') {
        if (use_cntrl_prefix == '\0') {
          if (param_1 == 8) {
            local_2c = -1;
            local_28 = 1;
            *pbVar1 = 8;
          }
          else {
            local_2c = 0;
            local_28 = 1;
            *pbVar1 = param_1;
          }
        }
        else if ((char)param_1 < '\0') {
          local_2c = 4;
          local_28 = 4;
          *pbVar1 = 0x5c;
          sprintf((char *)local_14,"%03o",(ulong)param_1);
          local_20 = pbVar1 + 1;
          for (local_30 = 0; local_30 < 3; local_30 = local_30 + 1) {
            *local_20 = local_14[local_30];
            local_20 = local_20 + 1;
          }
        }
        else {
          local_2c = 2;
          local_28 = 2;
          *pbVar1 = 0x5e;
          pbVar1[1] = param_1 ^ 0x40;
        }
      }
      else {
        local_2c = 4;
        local_28 = 4;
        *pbVar1 = 0x5c;
        sprintf((char *)local_14,"%03o",(ulong)param_1);
        local_20 = pbVar1 + 1;
        for (local_30 = 0; local_30 < 3; local_30 = local_30 + 1) {
          *local_20 = local_14[local_30];
          local_20 = local_20 + 1;
        }
      }
    }
    else {
      local_2c = 1;
      local_28 = 1;
      *pbVar1 = param_1;
    }
  }
  if ((local_2c < 0) && (input_position == 0)) {
    local_28 = 0;
    input_position = 0;
  }
  else if ((local_2c < 0) && (input_position <= -local_2c)) {
    input_position = 0;
  }
  else {
    input_position = local_2c + input_position;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_28;
}

// Function: cleanup
void cleanup(EVP_PKEY_CTX *ctx) {
  free(number_buff);
  free(clump_buff);
  free(column_vector);
  free(line_vector);
  free(end_vector);
  free(buff);
  return;
}

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
    pcVar3 = (char *)gettext("Paginate or columnate FILE(s) for printing.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  +FIRST_PAGE[:LAST_PAGE], --pages=FIRST_PAGE[:LAST_PAGE]\n                    begin [stop] printing with page FIRST_[LAST_]PAGE\n  -COLUMN, --columns=COLUMN\n                    output COLUMN columns and print columns down,\n                    unless -a is used. Balance number of lines in the\n                    columns on each page\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -a, --across      print columns across rather than down, used together\n                    with -COLUMN\n  -c, --show-control-chars\n                    use hat notation (^G) and octal backslash notation\n  -d, --double-space\n                    double space the output\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -D, --date-format=FORMAT\n                    use FORMAT for the header date\n  -e[CHAR[WIDTH]], --expand-tabs[=CHAR[WIDTH]]\n                    expand input CHARs (TABs) to tab WIDTH (8)\n  -F, -f, --form-feed\n                    use form feeds instead of newlines to separate pages\n                    (by a 3-line page header with -F or a 5-line header\n                    and trailer without -F)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -h, --header=HEADER\n                    use a centered HEADER instead of filename in page header,\n                    -h \"\" prints a blank line, don\'t use -h\"\"\n  -i[CHAR[WIDTH]], --output-tabs[=CHAR[WIDTH]]\n                    replace spaces with CHARs (TABs) to tab WIDTH (8)\n  -J, --join-lines  merge full lines, turns off -W line truncation, no column\n                    alignment, --sep-string[=STRING] sets separators\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -l, --length=PAGE_LENGTH\n                    set the page length to PAGE_LENGTH (66) lines\n                    (default number of lines of text 56, and with -F 63).\n                    implies -t if PAGE_LENGTH <= 10\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -m, --merge       print all files in parallel, one in each column,\n                    truncate lines, but join lines of full length with -J\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -n[SEP[DIGITS]], --number-lines[=SEP[DIGITS]]\n                    number lines, use DIGITS (5) digits, then SEP (TAB),\n                    default counting starts with 1st line of input file\n  -N, --first-line-number=NUMBER\n                    start counting with NUMBER at 1st line of first\n                    page printed (see +FIRST_PAGE)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -o, --indent=MARGIN\n                    offset each line with MARGIN (zero) spaces, do not\n                    affect -w or -W, MARGIN will be added to PAGE_WIDTH\n  -r, --no-file-warnings\n                    omit warning when a file cannot be opened\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -s[CHAR], --separator[=CHAR]\n                    separate columns by a single character, default for CHAR\n                    is the <TAB> character without -w and \'no char\' with -w.\n                    -s[CHAR] turns off line truncation of all 3 column\n                    options (-COLUMN|-a -COLUMN|-m) except -w is set\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -S[STRING], --sep-string[=STRING]\n                    separate columns by STRING,\n                    without -S: Default separator <TAB> with -J and <space>\n                    otherwise (same as -S\" \"), no effect on column options\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -t, --omit-header  omit page headers and trailers;\n                     implied if PAGE_LENGTH <= 10\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -T, --omit-pagination\n                    omit page headers and trailers, eliminate any pagination\n                    by form feeds set in input files\n  -v, --show-nonprinting\n                    use octal backslash notation\n  -w, --width=PAGE_WIDTH\n                    set page width to PAGE_WIDTH (72) characters for\n                    multiple text-column output only, -s[char] turns off (72)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -W, --page-width=PAGE_WIDTH\n                    set page width to PAGE_WIDTH (72) characters always,\n                    truncate lines, except -J option is set, no interference\n                    with -S or -s\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_001045a7);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

