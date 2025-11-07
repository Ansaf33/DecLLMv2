// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... [FILE]\n  or:  %s -e [OPTION]... [ARG]...\n  or:  %s -i LO-HI [OPTION]...\n"
                            );
    printf(pcVar3,uVar2,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Write a random permutation of the input lines to standard output.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_stdin_note();
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -e, --echo                treat each ARG as an input line\n  -i, --input-range=LO-HI   treat each number LO through HI as an input line\n  -n, --head-count=COUNT    output at most COUNT lines\n  -o, --output=FILE         write result to FILE instead of standard output\n      --random-source=FILE  get random bytes from FILE\n  -r, --repeat              output lines can be repeated\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("  -z, --zero-terminated     line delimiter is NUL, not newline\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info(&DAT_001020d1);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: input_from_argv
void input_from_argv(long param_1,int param_2,char param_3) {
  size_t sVar1;
  char *pcVar2;
  int local_24;
  char *local_20;
  long local_18;
  
  local_18 = (long)param_2;
  for (local_24 = 0; local_24 < param_2; local_24 = local_24 + 1) {
    sVar1 = strlen(*(char **)(param_1 + (long)local_24 * 8));
    local_18 = local_18 + sVar1;
  }
  local_20 = (char *)xmalloc(local_18);
  for (local_24 = 0; local_24 < param_2; local_24 = local_24 + 1) {
    pcVar2 = stpcpy(local_20,*(char **)(param_1 + (long)local_24 * 8));
    *(char **)((long)local_24 * 8 + param_1) = local_20;
    local_20 = pcVar2 + 1;
    *pcVar2 = param_3;
  }
  *(char **)((long)param_2 * 8 + param_1) = local_20;
  return;
}

// Function: next_line
long next_line(void *param_1,char param_2) {
  void *pvVar1;
  
  pvVar1 = rawmemchr(param_1,(int)param_2);
  return (long)pvVar1 + 1;
}

// Function: input_size
long input_size(void) {
  char cVar1;
  int iVar2;
  __off_t _Var3;
  long in_FS_OFFSET;
  stat local_a8;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  iVar2 = fstat(0,&local_a8);
  if (iVar2 == 0) {
    cVar1 = usable_st_size(&local_a8);
    if (cVar1 == '\0') {
      local_a8.st_size = 0x7fffffffffffffff;
    }
    else {
      _Var3 = lseek(0,0,1);
      if (_Var3 < 0) {
        local_a8.st_size = 0x7fffffffffffffff;
      }
      else {
        local_a8.st_size = local_a8.st_size - _Var3;
      }
    }
  }
  else {
    local_a8.st_size = 0x7fffffffffffffff;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_a8.st_size;
}

// Function: read_input_reservoir_sampling
ulong read_input_reservoir_sampling
                (FILE *param_1,char param_2,ulong param_3,undefined8 param_4,long *param_5) {
  int iVar1;
  long lVar2;
  undefined8 uVar3;
  int *piVar4;
  ulong uVar5;
  long in_FS_OFFSET;
  bool bVar6;
  ulong local_78;
  ulong local_70;
  undefined *local_68;
  long local_60;
  ulong local_58;
  ulong local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_78 = 0;
  local_68 = (undefined *)0x0;
  local_60 = 0;
  for (local_70 = 0; local_70 < param_3; local_70 = local_70 + 1) {
    if (local_70 == local_78) {
      local_58 = local_78;
      local_60 = xpalloc(local_60,&local_78,1,param_3,0x18);
      memset((void *)(local_60 + local_70 * 0x18),0,(local_78 - local_58) * 0x18);
    }
    local_68 = (undefined *)readlinebuffer_delim(local_70 * 0x18 + local_60,param_1,(int)param_2);
    if (local_68 == (undefined *)0x0) break;
  }
  if (local_68 != (undefined *)0x0) {
    initbuffer(local_38);
    do {
      local_50 = randint_choose(param_4,local_70 + 1);
      if (local_50 < param_3) {
        local_68 = (undefined *)(local_60 + local_50 * 0x18);
      }
      else {
        local_68 = local_38;
      }
      lVar2 = readlinebuffer_delim(local_68,param_1,(int)param_2);
    } while ((lVar2 != 0) && (uVar5 = local_70 + 1, bVar6 = local_70 != 0, local_70 = uVar5, bVar6))
    ;
    if (local_70 == 0) {
      local_48 = gettext("too many input lines");
      error(1,0x4b,local_48);
    }
    freebuffer(local_38);
  }
  iVar1 = ferror_unlocked(param_1);
  if (iVar1 != 0) {
    uVar3 = gettext("read error");
    piVar4 = __errno_location();
    local_40 = uVar3;
    error(1,*piVar4,uVar3);
  }
  *param_5 = local_60;
  uVar5 = local_70;
  if (param_3 <= local_70) {
    uVar5 = param_3;
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar5;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: write_permuted_output_reservoir
undefined8 write_permuted_output_reservoir(ulong param_1,long param_2,long param_3) {
  long lVar1;
  size_t sVar2;
  ulong local_18;
  
  local_18 = 0;
  while( true ) {
    if (param_1 <= local_18) {
      return 0;
    }
    lVar1 = param_2 + *(long *)(param_3 + local_18 * 8) * 0x18;
    sVar2 = fwrite_unlocked(*(void **)(lVar1 + 0x10),1,*(size_t *)(lVar1 + 8),_stdout);
    if (sVar2 != *(size_t *)(lVar1 + 8)) break;
    local_18 = local_18 + 1;
  }
  return 0xffffffff;
}

// Function: read_input
ulong read_input(undefined8 param_1,char param_2,ulong **param_3) {
  undefined8 uVar1;
  int *piVar2;
  long in_FS_OFFSET;
  long local_60;
  ulong local_58;
  ulong local_50;
  ulong local_48;
  ulong local_40;
  ulong local_38;
  ulong *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_40 = 0;
  local_40 = fread_file(param_1,0,&local_60);
  if (local_40 == 0) {
    uVar1 = gettext("read error");
    piVar2 = __errno_location();
    local_28 = uVar1;
    error(1,*piVar2,uVar1);
  }
  if ((local_60 != 0) && (param_2 != *(char *)(local_40 + local_60 + -1))) {
    *(char *)(local_40 + local_60) = param_2;
    local_60 = local_60 + 1;
  }
  local_38 = local_40 + local_60;
  local_50 = 0;
  for (local_58 = local_40; local_58 < local_38; local_58 = next_line(local_58,(int)param_2)) {
    local_50 = local_50 + 1;
  }
  local_30 = (ulong *)xnmalloc(local_50 + 1,8);
  *param_3 = local_30;
  local_58 = local_40;
  *local_30 = local_40;
  for (local_48 = 1; local_48 <= local_50; local_48 = local_48 + 1) {
    local_58 = next_line(local_58,(int)param_2);
    local_30[local_48] = local_58;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_50;
}

// Function: write_permuted_lines
undefined8 write_permuted_lines(ulong param_1,long param_2,long param_3) {
  void *pvVar1;
  void *pvVar2;
  void **ppvVar3;
  size_t sVar4;
  ulong local_20;
  
  local_20 = 0;
  while( true ) {
    if (param_1 <= local_20) {
      return 0;
    }
    ppvVar3 = (void **)(param_2 + *(long *)(param_3 + local_20 * 8) * 8);
    pvVar1 = ppvVar3[1];
    pvVar2 = *ppvVar3;
    sVar4 = fwrite_unlocked(*ppvVar3,1,(long)pvVar1 - (long)pvVar2,_stdout);
    if ((long)pvVar1 - (long)pvVar2 != sVar4) break;
    local_20 = local_20 + 1;
  }
  return 0xffffffff;
}

// Function: write_permuted_numbers
undefined8 write_permuted_numbers(ulong param_1,long param_2,long param_3,char param_4) {
  int iVar1;
  ulong local_18;
  
  local_18 = 0;
  while( true ) {
    if (param_1 <= local_18) {
      return 0;
    }
    iVar1 = printf("%lu%c",param_2 + *(long *)(param_3 + local_18 * 8),(ulong)(uint)(int)param_4);
    if (iVar1 < 0) break;
    local_18 = local_18 + 1;
  }
  return 0xffffffff;
}

// Function: write_random_numbers
undefined8
write_random_numbers(undefined8 param_1,ulong param_2,long param_3,long param_4,char param_5) {
  int iVar1;
  long lVar2;
  ulong local_20;
  
  local_20 = 0;
  while( true ) {
    if (param_2 <= local_20) {
      return 0;
    }
    lVar2 = randint_choose(param_1,(param_4 - param_3) + 1);
    iVar1 = printf("%lu%c",lVar2 + param_3,(ulong)(uint)(int)param_5);
    if (iVar1 < 0) break;
    local_20 = local_20 + 1;
  }
  return 0xffffffff;
}

// Function: write_random_lines
undefined8 write_random_lines(undefined8 param_1,ulong param_2,long param_3,undefined8 param_4) {
  void *pvVar1;
  void *pvVar2;
  long lVar3;
  void **ppvVar4;
  size_t sVar5;
  ulong local_28;
  
  local_28 = 0;
  while( true ) {
    if (param_2 <= local_28) {
      return 0;
    }
    lVar3 = randint_choose(param_1,param_4);
    ppvVar4 = (void **)(param_3 + lVar3 * 8);
    pvVar1 = ppvVar4[1];
    pvVar2 = *ppvVar4;
    sVar5 = fwrite_unlocked(*ppvVar4,1,(long)pvVar1 - (long)pvVar2,_stdout);
    if ((long)pvVar1 - (long)pvVar2 != sVar5) break;
    local_28 = local_28 + 1;
  }
  return 0xffffffff;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  bool bVar1;
  bool bVar2;
  bool bVar3;
  bool bVar4;
  bool bVar5;
  char cVar6;
  undefined4 uVar7;
  int iVar8;
  undefined8 uVar9;
  undefined8 uVar10;
  char *pcVar11;
  int *piVar12;
  long lVar13;
  long in_FS_OFFSET;
  undefined local_16f;
  int local_16c;
  int local_168;
  undefined8 *local_f8;
  undefined8 local_f0;
  ulong local_e8;
  char *local_e0;
  ulong local_d8;
  ulong local_d0;
  char *local_c8;
  char *local_c0;
  char *local_b8;
  char *local_b0;
  undefined8 *local_a8;
  undefined8 local_a0;
  char *local_98;
  undefined8 *local_90;
  long local_88;
  undefined8 local_80;
  char *local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined *local_48;
  undefined *local_40;
  undefined8 local_38;
  undefined *local_30;
  undefined8 local_28;
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  bVar2 = false;
  bVar1 = false;
  local_d8 = 0xffffffffffffffff;
  local_d0 = 0;
  local_c8 = (char *)0x7fffffffffffffff;
  local_c0 = (char *)0x0;
  local_b8 = (char *)0x0;
  local_16f = 10;
  local_f8 = (undefined8 *)0x0;
  bVar4 = false;
  bVar3 = false;
  local_a8 = (undefined8 *)0x0;
  local_f0 = 0;
  local_a0 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
LAB_0010146e:
  do {
    while( true ) {
      iVar8 = getopt_long(param_1,param_2,"ei:n:o:rz",long_opts,0);
      if (iVar8 == -1) {
        param_1 = param_1 - _optind;
        local_90 = param_2 + _optind;
        if ((bVar2) && (bVar1)) {
          local_58 = gettext("cannot combine -e and -i options");
          error(0,0,local_58);
          usage(1);
        }
        if (bVar1) {
          bVar5 = 0 < param_1;
        }
        else if ((bVar2) || (param_1 < 2)) {
          bVar5 = false;
        }
        else {
          bVar5 = true;
        }
        if (bVar5) {
          if (bVar1) {
            lVar13 = 0;
          }
          else {
            lVar13 = 8;
          }
          uVar10 = quote(*(undefined8 *)((long)local_90 + lVar13));
          local_50 = gettext("extra operand %s");
          error(0,0,local_50,uVar10);
          usage(1);
        }
        if (local_c8 == (char *)0x0) {
          local_b0 = (char *)0x0;
          local_a8 = (undefined8 *)0x0;
        }
        else if (bVar2) {
          input_from_argv(local_90,param_1,local_16f);
          local_b0 = (char *)(long)param_1;
          local_a8 = local_90;
        }
        else if (bVar1) {
          local_b0 = (char *)((local_d0 - local_d8) + 1);
          local_a8 = (undefined8 *)0x0;
        }
        else {
          if (((param_1 == 1) && (cVar6 = streq(*local_90,&DAT_0010225a), cVar6 != '\x01')) &&
             (lVar13 = freopen_safer(*local_90,&DAT_0010225c,_stdin), lVar13 == 0)) {
            uVar10 = quotearg_n_style_colon(0,3,*local_90);
            piVar12 = __errno_location();
            local_48 = &DAT_0010225e;
            error(1,*piVar12,&DAT_0010225e,uVar10);
          }
          fadvise(_stdin,2);
          if (((bVar3) || (local_c8 == (char *)0x7fffffffffffffff)) ||
             (lVar13 = input_size(), lVar13 < 0x800001)) {
            local_b0 = (char *)read_input(_stdin,local_16f,&local_f8);
            local_a8 = local_f8;
          }
          else {
            bVar4 = true;
            local_b0 = (char *)0xffffffffffffffff;
          }
        }
        local_98 = local_c8;
        if ((!bVar3) && (local_b0 <= local_c8)) {
          local_98 = local_b0;
        }
        if ((bVar4) || (bVar3)) {
          uVar10 = 0xffffffffffffffff;
        }
        else {
          uVar10 = randperm_bound(local_98,local_b0);
        }
        local_88 = randint_all_new(local_b8,uVar10);
        if (local_88 == 0) {
          pcVar11 = local_b8;
          if (local_b8 == (char *)0x0) {
            pcVar11 = "getrandom";
          }
          uVar10 = quotearg_n_style_colon(0,3,pcVar11);
          piVar12 = __errno_location();
          local_40 = &DAT_0010225e;
          error(1,*piVar12,&DAT_0010225e,uVar10);
        }
        if (bVar4) {
          local_b0 = (char *)read_input_reservoir_sampling
                                       (_stdin,local_16f,local_98,local_88,&local_f0);
          local_98 = local_b0;
        }
        if ((((local_c8 != (char *)0x0) && (!bVar2)) && (!bVar1)) &&
           (iVar8 = rpl_fclose(_stdin), iVar8 != 0)) {
          uVar10 = gettext("read error");
          piVar12 = __errno_location();
          local_38 = uVar10;
          error(1,*piVar12,uVar10);
        }
        if (!bVar3) {
          local_a0 = randperm_new(local_88,local_98,local_b0);
        }
        if ((local_c0 != (char *)0x0) &&
           (lVar13 = freopen_safer(local_c0,&DAT_0010226b,_stdout), lVar13 == 0)) {
          uVar10 = quotearg_n_style_colon(0,3,local_c0);
          piVar12 = __errno_location();
          local_30 = &DAT_0010225e;
          error(1,*piVar12,&DAT_0010225e,uVar10);
        }
        if (bVar3) {
          if (local_c8 == (char *)0x0) {
            local_16c = 0;
          }
          else {
            if (local_b0 == (char *)0x0) {
              local_28 = gettext("no lines to repeat");
              error(1,0,local_28);
            }
            if (bVar1) {
              local_16c = write_random_numbers(local_88,local_98,local_d8,local_d0,local_16f);
            }
            else {
              local_16c = write_random_lines(local_88,local_98,local_a8,local_b0);
            }
          }
        }
        else if (bVar4) {
          local_16c = write_permuted_output_reservoir(local_b0,local_f0,local_a0);
        }
        else if (bVar1) {
          local_16c = write_permuted_numbers(local_98,local_d8,local_a0,local_16f);
        }
        else {
          local_16c = write_permuted_lines(local_98,local_a8,local_a0);
        }
        if (local_16c != 0) {
          write_error();
        }
        randint_all_free(local_88);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar8 < 0x81) break;
switchD_0010101e_caseD_66:
      usage(1);
    }
    if (iVar8 < 0x65) {
      if (iVar8 == -0x83) {
LAB_00101405:
        uVar10 = proper_name_lite("Paul Eggert","Paul Eggert");
        version_etc(_stdout,&DAT_001020d1,"GNU coreutils",_Version,uVar10,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (iVar8 == -0x82) {
        usage(0);
        goto LAB_00101405;
      }
      goto switchD_0010101e_caseD_66;
    }
    switch(iVar8) {
    case 0x65:
      bVar2 = true;
      break;
    default:
      goto switchD_0010101e_caseD_66;
    case 0x69:
      if (bVar1) {
        local_80 = gettext("multiple -i options specified");
        error(1,0,local_80);
      }
      bVar1 = true;
      local_168 = xstrtoumax(_optarg,&local_e0,10,&local_e8,0);
      if (local_168 == 0) {
        local_d8 = local_e8;
        if (*local_e0 == '-') {
          local_168 = xstrtoumax(local_e0 + 1,0,10,&local_e8,&DAT_00101d94);
          if (local_168 == 0) {
            local_d0 = local_e8;
          }
        }
        else {
          local_168 = 4;
        }
      }
      local_b0 = (char *)((local_d0 - local_d8) + 1);
      if ((local_168 == 0) && ((local_b0 == (char *)0x0) == local_d0 < local_d8)) break;
      uVar10 = quote(_optarg);
      uVar9 = gettext("invalid input range");
      if (local_168 == 1) {
        uVar7 = 0x4b;
      }
      else {
        uVar7 = 0;
      }
      local_78 = "%s: %s";
      error(1,uVar7,"%s: %s",uVar9,uVar10);
    case 0x6e:
      iVar8 = xstrtoumax(_optarg,0,10,&local_e0,&DAT_00101d94);
      if (iVar8 == 0) {
        if (local_e0 <= local_c8) {
          local_c8 = local_e0;
        }
      }
      else if (iVar8 != 1) {
        uVar10 = quote(_optarg);
        local_70 = gettext("invalid line count: %s");
        error(1,0,local_70,uVar10);
switchD_0010101e_caseD_6f:
        if ((local_c0 != (char *)0x0) && (cVar6 = streq(local_c0,_optarg), cVar6 != '\x01')) {
          local_68 = gettext("multiple output files specified");
          error(1,0,local_68);
        }
        local_c0 = _optarg;
      }
      break;
    case 0x6f:
      goto switchD_0010101e_caseD_6f;
    case 0x72:
      bVar3 = true;
      break;
    case 0x7a:
      local_16f = 0;
      break;
    case 0x80:
      goto switchD_0010101e_caseD_80;
    }
  } while( true );
switchD_0010101e_caseD_80:
  if ((local_b8 != (char *)0x0) && (cVar6 = streq(local_b8,_optarg), cVar6 != '\x01')) {
    local_60 = gettext("multiple random sources specified");
    error(1,0,local_60);
  }
  local_b8 = _optarg;
  goto LAB_0010146e;
}

