// Function: duinfo_init
void duinfo_init(undefined8 *param_1) {
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0x8000000000000000;
  param_1[3] = 0xffffffffffffffff;
  return;
}

// Function: duinfo_set
void duinfo_set(undefined8 *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4) {
  *param_1 = param_2;
  param_1[1] = 1;
  param_1[2] = param_3;
  param_1[3] = param_4;
  return;
}

// Function: duinfo_add
void duinfo_add(ulong *param_1,long *param_2) {
  int iVar1;
  ulong uVar2;
  
  uVar2 = *param_2 + *param_1;
  if (uVar2 < *param_1) {
    uVar2 = 0xffffffffffffffff;
  }
  *param_1 = uVar2;
  param_1[1] = param_1[1] + param_2[1];
  iVar1 = timespec_cmp(param_1[2],param_1[3],param_2[2],param_2[3]);
  if (iVar1 < 0) {
    uVar2 = param_2[3];
    param_1[2] = param_2[2];
    param_1[3] = uVar2;
  }
  return;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... [FILE]...\n  or:  %s [OPTION]... --files0-from=F\n"
                            );
    printf(pcVar3,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Summarize device usage of the set of FILEs, recursively for directories.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -0, --null            end each output line with NUL, not newline\n  -a, --all             write counts for all files, not just directories\n      --apparent-size   print apparent sizes rather than device usage; although\n                          the apparent size is usually smaller, it may be\n                          larger due to holes in (\'sparse\') files, internal\n                          fragmentation, indirect blocks, and the like\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -B, --block-size=SIZE  scale sizes by SIZE before printing them; e.g.,\n                           \'-BM\' prints sizes in units of 1,048,576 bytes;\n                           see SIZE format below\n  -b, --bytes           equivalent to \'--apparent-size --block-size=1\'\n  -c, --total           produce a grand total\n  -D, --dereference-args  dereference only symlinks that are listed on the\n                          command line\n  -d, --max-depth=N     print the total for a directory (or file, with --all)\n                          only if it is N or fewer levels below the command\n                          line argument;  --max-depth=0 is the same as\n                          --summarize\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --files0-from=F   summarize device usage of the\n                          NUL-terminated file names specified in file F;\n                          if F is -, then read names from standard input\n  -H                    equivalent to --dereference-args (-D)\n  -h, --human-readable  print sizes in human readable format (e.g., 1K 234M 2G)\n      --inodes          list inode usage information instead of block usage\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -k                    like --block-size=1K\n  -L, --dereference     dereference all symbolic links\n  -l, --count-links     count sizes many times if hard linked\n  -m                    like --block-size=1M\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -P, --no-dereference  don\'t follow any symbolic links (this is the default)\n  -S, --separate-dirs   for directories do not include size of subdirectories\n      --si              like -h, but use powers of 1000 not 1024\n  -s, --summarize       display only a total for each argument\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -t, --threshold=SIZE  exclude entries smaller than SIZE if positive,\n                          or entries greater than SIZE if negative\n      --time            show time of the last modification of any file in the\n                          directory, or any of its subdirectories\n      --time=WORD       show time as WORD instead of modification time:\n                          atime, access, use, ctime or status\n      --time-style=STYLE  show times using STYLE, which can be:\n                            full-iso, long-iso, iso, or +FORMAT;\n                            FORMAT is interpreted like in \'date\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -X, --exclude-from=FILE  exclude files that match any pattern in FILE\n      --exclude=PATTERN    exclude files that match PATTERN\n  -x, --one-file-system    skip directories on different file systems\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_blocksize_note(&DAT_00103941);
    emit_size_note();
    emit_ancillary_info(&DAT_00103944);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: hash_ins
bool hash_ins(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  int iVar1;
  
  iVar1 = di_set_insert(param_1,param_3,param_2);
  if (iVar1 < 0) {
    xalloc_die();
  }
  return iVar1 != 0;
}

// Function: print_only_size
void print_only_size(long param_1) {
  FILE *__stream;
  char *__s;
  long in_FS_OFFSET;
  undefined local_2b8 [664];
  long local_20;
  
  __stream = _stdout;
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (param_1 == -1) {
    __s = (char *)gettext("Infinity");
  }
  else {
    __s = (char *)human_readable(param_1,local_2b8,human_output_opts,1,output_block_size);
  }
  fputs_unlocked(__s,__stream);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: print_size
void print_size(undefined8 *param_1,undefined8 param_2) {
  FILE *__stream;
  char cVar1;
  char *__s;
  undefined8 uVar2;
  long in_FS_OFFSET;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (opt_inodes == '\0') {
    uVar2 = *param_1;
  }
  else {
    uVar2 = param_1[1];
  }
  print_only_size(uVar2);
  if (opt_time != '\0') {
    putchar_unlocked(9);
    cVar1 = show_date(time_format,param_1[2],param_1[3],localtz);
    __stream = _stdout;
    if (cVar1 != '\x01') {
      __s = (char *)timetostr(param_1[2],local_38);
      fputs_unlocked(__s,__stream);
    }
  }
  if (opt_nul_terminate_output == '\0') {
    uVar2 = 10;
  }
  else {
    uVar2 = 0;
  }
  printf("\t%s%c",param_2,uVar2);
  fflush_unlocked(_stdout);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: fill_mount_table
void fill_mount_table(void) {
  long lVar1;
  int iVar2;
  long in_FS_OFFSET;
  long local_b8;
  stat local_a8;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_b8 = read_file_system_list(0);
  while (local_b8 != 0) {
    if (((*(byte *)(local_b8 + 0x28) & 2) == 0) && ((*(byte *)(local_b8 + 0x28) & 1) == 0)) {
      iVar2 = stat(*(char **)(local_b8 + 8),&local_a8);
      if (iVar2 == 0) {
        hash_ins(di_mnt,local_a8.st_ino,local_a8.st_dev);
      }
    }
    lVar1 = *(long *)(local_b8 + 0x30);
    free_mount_entry(local_b8);
    local_b8 = lVar1;
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: mount_point_in_fts_cycle
undefined8 mount_point_in_fts_cycle(long *param_1) {
  long *plVar1;
  int iVar2;
  long *local_20;
  
  plVar1 = (long *)*param_1;
  local_20 = param_1;
  if (di_mnt == 0) {
    di_mnt = di_set_alloc();
    if (di_mnt == 0) {
      xalloc_die();
    }
    fill_mount_table();
  }
  for (; (local_20 != (long *)0x0 && (local_20 != plVar1)); local_20 = (long *)local_20[1]) {
    iVar2 = di_set_lookup(di_mnt,local_20[0xe],local_20[0xf]);
    if (0 < iVar2) {
      return 1;
    }
  }
  return 0;
}

// Function: process_file
undefined process_file(long param_1,long param_2) {
  ushort uVar1;
  char cVar2;
  long *plVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  long lVar7;
  ulong uVar8;
  long in_FS_OFFSET;
  bool bVar9;
  undefined auVar10 [16];
  undefined local_f6;
  char local_f5;
  long local_d0;
  ulong local_78;
  ulong local_70;
  undefined8 local_68;
  undefined8 local_60;
  ulong local_58;
  ulong local_50;
  undefined8 local_48;
  undefined8 local_40;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  local_f6 = 1;
  uVar6 = *(undefined8 *)(param_2 + 0x38);
  plVar3 = (long *)(param_2 + 0x70);
  uVar1 = *(ushort *)(param_2 + 0x68);
  if (uVar1 == 4) {
    uVar4 = quotearg_style(4,uVar6);
    uVar5 = gettext("cannot read directory %s");
    error(0,*(undefined4 *)(param_2 + 0x40),uVar5,uVar4);
    local_f6 = 0;
  }
  else if (uVar1 != 6) {
    local_f5 = excluded_file_name(exclude,uVar6);
    if (local_f5 != '\x01') {
      if (uVar1 == 0xb) {
        rpl_fts_set(param_1,param_2,1);
        lVar7 = rpl_fts_read(param_1);
        if (lVar7 != param_2) {
                    /* WARNING: Subroutine does not return */
          __assert_fail("e == ent",
                        "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/du.c"
                        ,0x1fb,"process_file");
        }
        uVar1 = *(ushort *)(param_2 + 0x68);
      }
      if ((uVar1 == 10) || (uVar1 == 0xd)) {
        uVar6 = quotearg_style(4,uVar6);
        uVar4 = gettext("cannot access %s");
        error(0,*(undefined4 *)(param_2 + 0x40),uVar4,uVar6);
        local_f6 = 0;
        goto LAB_0010143e;
      }
      if ((((*(uint *)(param_1 + 0x48) & 0x40) != 0) && (0 < *(long *)(param_2 + 0x58))) &&
         (*(long *)(param_1 + 0x18) != *plVar3)) {
        local_f5 = '\x01';
      }
    }
    if ((local_f5 != '\0') ||
       (((opt_count_all != '\x01' &&
         ((hash_all != '\0' ||
          (((*(uint *)(param_2 + 0x88) & 0xf000) != 0x4000 && (1 < *(ulong *)(param_2 + 0x80)))))))
        && (cVar2 = hash_ins(di_files,*(undefined8 *)(param_2 + 0x78),*plVar3), cVar2 != '\x01'))))
    {
      if (uVar1 == 1) {
        rpl_fts_set(param_1,param_2,4);
        lVar7 = rpl_fts_read(param_1);
        if (lVar7 != param_2) {
                    /* WARNING: Subroutine does not return */
          __assert_fail("e == ent",
                        "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/du.c"
                        ,0x21c,"process_file");
        }
      }
      local_f6 = 1;
      goto LAB_0010143e;
    }
    if (uVar1 == 7) {
      uVar4 = quotearg_n_style_colon(0,3,uVar6);
      error(0,*(undefined4 *)(param_2 + 0x40),&DAT_001039d6,uVar4);
      local_f6 = 0;
    }
    else if (uVar1 < 8) {
      if (uVar1 == 1) {
        local_f6 = 1;
        goto LAB_0010143e;
      }
      if (uVar1 == 2) {
        cVar2 = cycle_warning_required(param_1,param_2);
        if ((cVar2 == '\0') || (cVar2 = mount_point_in_fts_cycle(param_2), cVar2 == '\x01')) {
          local_f6 = 1;
        }
        else {
          uVar6 = quotearg_n_style_colon(0,3,uVar6);
          uVar4 = gettext(
                         "WARNING: Circular directory structure.\nThis almost certainly means that you have a corrupted file system.\nNOTIFY YOUR SYSTEM MANAGER.\nThe following directory is part of the cycle:\n  %s\n"
                         );
          error(0,0,uVar4,uVar6);
          local_f6 = 0;
        }
        goto LAB_0010143e;
      }
    }
  }
  if (time_type == 0) {
    auVar10 = get_stat_mtime(plVar3);
  }
  else if (time_type == 2) {
    auVar10 = get_stat_atime(plVar3);
  }
  else {
    auVar10 = get_stat_ctime(plVar3);
  }
  if (apparent_size == '\0') {
    lVar7 = *(long *)(param_2 + 0xb0) << 9;
  }
  else {
    cVar2 = usable_st_size(plVar3);
    if (cVar2 == '\0') {
      lVar7 = 0;
    }
    else {
      lVar7 = *(long *)(param_2 + 0xa0);
      if (lVar7 < 0) {
        lVar7 = 0;
      }
    }
  }
  duinfo_set(&local_78,lVar7,auVar10._0_8_,auVar10._8_8_);
  lVar7 = *(long *)(param_2 + 0x58);
  local_58 = local_78;
  local_50 = local_70;
  local_48 = local_68;
  local_40 = local_60;
  if (n_alloc_5 == 0) {
    n_alloc_5 = lVar7 + 10;
    dulvl_4 = xcalloc(n_alloc_5,0x40);
  }
  else if (lVar7 != prev_level) {
    if (prev_level < lVar7) {
      local_d0 = prev_level;
      if (n_alloc_5 <= lVar7) {
        dulvl_4 = xpalloc(dulvl_4,&n_alloc_5,(lVar7 - n_alloc_5) + 1,0xffffffffffffffff,0x40);
        local_d0 = prev_level;
      }
      while (local_d0 = local_d0 + 1, local_d0 <= lVar7) {
        duinfo_init(dulvl_4 + local_d0 * 0x40);
        duinfo_init(dulvl_4 + local_d0 * 0x40 + 0x20);
      }
    }
    else {
      if (lVar7 != prev_level + -1) {
                    /* WARNING: Subroutine does not return */
        __assert_fail("level == prev_level - 1",
                      "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/du.c",
                      0x268,"process_file");
      }
      duinfo_add(&local_58,dulvl_4 + prev_level * 0x40);
      if (opt_separate_dirs != '\x01') {
        duinfo_add(&local_58,dulvl_4 + prev_level * 0x40 + 0x20);
      }
      duinfo_add(dulvl_4 + lVar7 * 0x40 + 0x20,dulvl_4 + prev_level * 0x40);
      duinfo_add(dulvl_4 + lVar7 * 0x40 + 0x20,dulvl_4 + prev_level * 0x40 + 0x20);
    }
  }
  prev_level = lVar7;
  if ((opt_separate_dirs != '\x01') || ((uVar1 != 6 && (uVar1 != 4)))) {
    duinfo_add(dulvl_4 + lVar7 * 0x40,&local_78);
  }
  duinfo_add(tot_dui,&local_78);
  if (((((uVar1 == 6) || (uVar1 == 4)) && (lVar7 <= max_depth)) ||
      ((opt_all != '\0' && (lVar7 <= max_depth)))) || (lVar7 == 0)) {
    uVar8 = local_58;
    if (opt_inodes != '\0') {
      uVar8 = local_50;
    }
    if ((long)opt_threshold < 0) {
      bVar9 = uVar8 <= -opt_threshold;
    }
    else {
      bVar9 = opt_threshold <= uVar8;
    }
    if (bVar9) {
      print_size(&local_58,uVar6);
    }
  }
LAB_0010143e:
  if (local_30 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_f6;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: du_files
bool du_files(long *param_1,undefined4 param_2) {
  byte bVar1;
  int iVar2;
  long lVar3;
  long lVar4;
  int *piVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  bool local_49;
  
  local_49 = true;
  if (*param_1 != 0) {
    lVar3 = xfts_open(param_1,param_2,0);
    while( true ) {
      lVar4 = rpl_fts_read(lVar3);
      if (lVar4 == 0) break;
      bVar1 = process_file(lVar3,lVar4);
      local_49 = (bVar1 & local_49) != 0;
    }
    piVar5 = __errno_location();
    if (*piVar5 != 0) {
      uVar6 = quotearg_n_style_colon(0,3,*(undefined8 *)(lVar3 + 0x20));
      uVar7 = gettext("fts_read failed: %s");
      piVar5 = __errno_location();
      error(0,*piVar5,uVar7,uVar6);
      local_49 = false;
    }
    prev_level = 0;
    iVar2 = rpl_fts_close(lVar3);
    if (iVar2 != 0) {
      uVar6 = gettext("fts_close failed");
      piVar5 = __errno_location();
      error(0,*piVar5,uVar6);
      local_49 = false;
    }
  }
  return local_49;
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  bool bVar1;
  bool bVar2;
  char cVar3;
  byte bVar4;
  int iVar5;
  char *pcVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  long lVar10;
  undefined8 uVar11;
  int *piVar12;
  long in_FS_OFFSET;
  bool local_17b;
  undefined4 local_178;
  uint local_174;
  uint local_170;
  int local_16c;
  int local_168;
  int local_164;
  undefined4 local_160;
  undefined4 local_15c;
  undefined4 local_158;
  undefined4 local_154;
  undefined4 local_150;
  int local_14c;
  undefined4 local_148;
  undefined4 local_144;
  undefined4 local_140;
  undefined4 local_13c;
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined4 local_120;
  int local_11c;
  undefined4 local_118;
  int local_114;
  undefined4 local_110;
  undefined4 local_10c;
  undefined4 local_108;
  undefined4 local_104;
  undefined4 local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined4 local_f4;
  uint local_f0;
  undefined4 uStack_ec;
  char *local_e8;
  long local_e0;
  char *local_d8;
  undefined8 *local_d0;
  char *local_c8;
  undefined8 local_c0;
  undefined8 local_b8;
  undefined8 local_b0;
  undefined *local_a8;
  undefined8 local_a0;
  undefined8 local_98;
  undefined8 local_90;
  undefined8 local_88;
  undefined8 local_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined *local_60;
  char *local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  bVar2 = false;
  local_17b = true;
  local_e8 = (char *)0x0;
  local_174 = 8;
  local_170 = 0x10;
  bVar1 = false;
  local_48 = bad_cast(&DAT_00103ad7);
  local_40 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  exclude = new_exclude();
  pcVar6 = getenv("DU_BLOCK_SIZE");
  human_options(pcVar6,&human_output_opts,&output_block_size);
LAB_001016da:
  local_178 = 0xffffffff;
  local_16c = getopt_long(param_1,param_2,"0abd:chHklmst:xB:DLPSX:",long_options,&local_178);
  if (local_16c != -1) {
    if (0x86 < local_16c) {
switchD_001017a9_caseD_31:
      local_17b = false;
      goto LAB_001016da;
    }
    if (local_16c < 0x30) {
      if (local_16c == -0x83) {
LAB_00101c44:
        uVar9 = proper_name_lite("Jim Meyering","Jim Meyering");
        uVar11 = proper_name_lite("Paul Eggert","Paul Eggert");
        uVar7 = proper_name_lite("David MacKenzie","David MacKenzie");
        uVar8 = proper_name_lite("Torbjorn Granlund",&DAT_00103b95);
        version_etc(_stdout,&DAT_00103944,"GNU coreutils",_Version,uVar8,uVar7,uVar11,uVar9,0);
                    /* WARNING: Subroutine does not return */
        exit(0);
      }
      if (local_16c == -0x82) {
        usage(0);
        goto LAB_00101c44;
      }
      goto switchD_001017a9_caseD_31;
    }
    switch(local_16c) {
    case 0x30:
      opt_nul_terminate_output = 1;
      break;
    default:
      goto switchD_001017a9_caseD_31;
    case 0x42:
      local_164 = human_options(_optarg,&human_output_opts,&output_block_size);
      if (local_164 == 0) break;
      xstrtol_fatal(local_164,local_178,(int)(char)local_16c,long_options,_optarg);
    case 0x44:
    case 0x48:
      local_170 = 0x11;
      break;
    case 0x4c:
      local_170 = 2;
      break;
    case 0x50:
      local_170 = 0x10;
      break;
    case 0x53:
      opt_separate_dirs = 1;
      break;
    case 0x58:
      iVar5 = add_exclude_file(add_exclude,exclude,_optarg,0x10000000,10);
      if (iVar5 != 0) {
        uVar9 = quotearg_n_style_colon(0,3,_optarg);
        piVar12 = __errno_location();
        local_14c = *piVar12;
        local_150 = 0;
        local_a8 = &DAT_001039d6;
        error(0,local_14c,&DAT_001039d6,uVar9);
        local_17b = false;
      }
      break;
    case 0x61:
      opt_all = '\x01';
      break;
    case 0x62:
      apparent_size = '\x01';
      human_output_opts = 0;
      output_block_size = 1;
      break;
    case 99:
      print_grand_total = '\x01';
      break;
    case 100:
      iVar5 = xstrtoimax(_optarg,0,0,&local_f0,&DAT_00102ad4);
      if (iVar5 == 0) {
        bVar2 = true;
        max_depth = CONCAT44(uStack_ec,local_f0);
      }
      else {
        uVar9 = quote(_optarg);
        local_b8 = gettext("invalid maximum depth %s");
        local_160 = 0;
        local_15c = 0;
        error(0,0,local_b8,uVar9);
        local_17b = false;
      }
      break;
    case 0x68:
      human_output_opts = 0xb0;
      output_block_size = 1;
      break;
    case 0x6b:
      human_output_opts = 0;
      output_block_size = 0x400;
      break;
    case 0x6c:
      opt_count_all = '\x01';
      break;
    case 0x6d:
      human_output_opts = 0;
      output_block_size = 0x100000;
      break;
    case 0x73:
      bVar1 = true;
      break;
    case 0x74:
      local_168 = xstrtoimax(_optarg,0,0,&opt_threshold,"kKmMGTPEZYRQ0");
      if (local_168 != 0) {
        xstrtol_fatal(local_168,local_178,(int)(char)local_16c,long_options,_optarg);
      }
      if ((opt_threshold == 0) && (*_optarg == '-')) {
        local_b0 = gettext("invalid --threshold argument \'-0\'");
        local_158 = 1;
        local_154 = 0;
        error(1,0,local_b0);
        goto switchD_001017a9_caseD_78;
      }
      break;
    case 0x78:
switchD_001017a9_caseD_78:
      local_174 = local_174 | 0x40;
      break;
    case 0x80:
      apparent_size = '\x01';
      break;
    case 0x81:
      add_exclude(exclude,_optarg,0x10000000);
      break;
    case 0x82:
      local_e8 = _optarg;
      break;
    case 0x83:
      human_output_opts = 0x90;
      output_block_size = 1;
      break;
    case 0x84:
      opt_time = '\x01';
      if (_optarg == (char *)0x0) {
        time_type = 0;
      }
      else {
        lVar10 = __xargmatch_internal("--time",_optarg,time_args,time_types,4,_argmatch_die,1);
        time_type = *(undefined4 *)(time_types + lVar10 * 4);
      }
      pcVar6 = getenv("TZ");
      localtz = tzalloc(pcVar6);
      break;
    case 0x85:
      time_style = _optarg;
      break;
    case 0x86:
      goto switchD_001017a9_caseD_86;
    }
    goto LAB_001016da;
  }
  if (local_17b != true) {
    usage(1);
  }
  if ((opt_all != '\0') && (bVar1)) {
    local_a0 = gettext("cannot both summarize and show all entries");
    local_148 = 0;
    local_144 = 0;
    error(0,0,local_a0);
    usage(1);
  }
  if (((bVar1) && (bVar2)) && (max_depth == 0)) {
    local_98 = gettext("warning: summarizing is the same as using --max-depth=0");
    local_140 = 0;
    local_13c = 0;
    error(0,0,local_98);
  }
  lVar10 = max_depth;
  if (((bVar1) && (bVar2)) && (max_depth != 0)) {
    local_90 = gettext("warning: summarizing conflicts with --max-depth=%td");
    local_138 = 0;
    local_134 = 0;
    error(0,0,local_90,lVar10);
    usage(1);
  }
  if (bVar1) {
    max_depth = 0;
  }
  if (opt_inodes != '\0') {
    if (apparent_size != '\0') {
      local_88 = gettext("warning: options --apparent-size and -b are ineffective with --inodes");
      local_130 = 0;
      local_12c = 0;
      error(0,0,local_88);
    }
    output_block_size = 1;
  }
  if (opt_time != '\0') {
    if (time_style == (char *)0x0) {
      time_style = getenv("TIME_STYLE");
      if ((time_style == (char *)0x0) || (cVar3 = streq(time_style,"locale"), cVar3 != '\0')) {
        time_style = "long-iso";
      }
      else if (*time_style == '+') {
        local_d8 = strchr(time_style,10);
        if (local_d8 != (char *)0x0) {
          *local_d8 = '\0';
        }
      }
      else {
        while (iVar5 = strncmp(time_style,"posix-",prefix_len_3), iVar5 == 0) {
          time_style = time_style + prefix_len_3;
        }
      }
    }
    if (*time_style == '+') {
      time_format = time_style + 1;
    }
    else {
      lVar10 = x_timestyle_match(time_style,0,time_style_args,time_style_types,4,1);
      if (lVar10 == 2) {
        time_format = "%Y-%m-%d";
      }
      else if (lVar10 < 3) {
        if (lVar10 == 0) {
          time_format = "%Y-%m-%d %H:%M:%S.%N %z";
        }
        else if (lVar10 == 1) {
          time_format = "%Y-%m-%d %H:%M";
        }
      }
    }
  }
  if (local_e8 == (char *)0x0) {
    if (_optind < param_1) {
      local_d0 = param_2 + _optind;
    }
    else {
      local_d0 = &local_48;
    }
    local_e0 = argv_iter_init_argv(local_d0);
    if ((_optind + 1 < param_1) || (local_170 == 2)) {
      hash_all = '\x01';
    }
    else {
      hash_all = '\0';
    }
  }
  else {
    if (_optind < param_1) {
      uVar9 = quote(param_2[_optind]);
      local_80 = gettext("extra operand %s");
      local_128 = 0;
      local_124 = 0;
      error(0,0,local_80,uVar9);
      uVar9 = gettext("file operands cannot be combined with --files0-from");
      fprintf(_stderr,"%s\n",uVar9);
      usage(1);
    }
    cVar3 = streq(local_e8,&DAT_00103d40);
    if ((cVar3 != '\x01') && (lVar10 = freopen_safer(local_e8,&DAT_00103d42,_stdin), lVar10 == 0)) {
      uVar9 = quotearg_style(4,local_e8);
      uVar11 = gettext("cannot open %s for reading");
      piVar12 = __errno_location();
      local_11c = *piVar12;
      local_120 = 1;
      local_78 = uVar11;
      error(1,local_11c,uVar11,uVar9);
    }
    local_e0 = argv_iter_init_stream(_stdin);
    hash_all = '\x01';
  }
  if (local_e0 == 0) {
    xalloc_die();
  }
  di_files = di_set_alloc();
  if (di_files == 0) {
    xalloc_die();
  }
  if ((opt_count_all != '\0') || (hash_all != '\x01')) {
    local_174 = local_174 | 0x100;
  }
  local_174 = local_174 | local_170;
  while( true ) {
    bVar1 = false;
    local_c8 = (char *)argv_iter(local_e0,&local_f0);
    if (local_c8 == (char *)0x0) break;
    if (((local_e8 != (char *)0x0) && (cVar3 = streq(local_e8,&DAT_00103d40), cVar3 != '\0')) &&
       (cVar3 = streq(local_c8,&DAT_00103d40), cVar3 != '\0')) {
      uVar9 = quotearg_style(4,local_c8);
      local_68 = gettext("when reading file names from standard input, no file name of %s allowed");
      local_110 = 0;
      local_10c = 0;
      error(0,0,local_68,uVar9);
      bVar1 = true;
    }
    if (*local_c8 == '\0') {
      if (local_e8 == (char *)0x0) {
        uVar9 = gettext("invalid zero-length file name");
        local_108 = 0;
        local_104 = 0;
        local_60 = &DAT_001039d6;
        error(0,0,&DAT_001039d6,uVar9);
      }
      else {
        local_c0 = argv_iter_n_args(local_e0);
        uVar9 = gettext("invalid zero-length file name");
        uVar11 = quotearg_n_style_colon(0,3,local_e8);
        local_100 = 0;
        local_fc = 0;
        local_58 = "%s:%td: %s";
        error(0,0,"%s:%td: %s",uVar11,local_c0,uVar9);
      }
      bVar1 = true;
    }
    if (bVar1) {
      local_17b = false;
    }
    else {
      temp_argv_0._0_8_ = local_c8;
      bVar4 = du_files(temp_argv_0,local_174);
      local_17b = (bVar4 & local_17b) != 0;
    }
  }
  if (local_f0 == 4) {
    uVar9 = quotearg_n_style_colon(0,3,local_e8);
    uVar11 = gettext("%s: read error");
    piVar12 = __errno_location();
    local_114 = *piVar12;
    local_118 = 0;
    local_70 = uVar11;
    error(0,local_114,uVar11,uVar9);
    local_17b = false;
LAB_001025e3:
    argv_iter_free(local_e0);
    di_set_free(di_files);
    if (di_mnt != 0) {
      di_set_free(di_mnt);
    }
    if ((local_e8 != (char *)0x0) &&
       (((iVar5 = ferror_unlocked(_stdin), iVar5 != 0 || (iVar5 = rpl_fclose(_stdin), iVar5 != 0))
        && (local_17b != false)))) {
      uVar9 = quotearg_style(4,local_e8);
      local_50 = gettext("error reading %s");
      local_f8 = 1;
      local_f4 = 0;
      error(1,0,local_50,uVar9);
    }
    if (print_grand_total != '\0') {
      uVar9 = gettext("total");
      print_size(tot_dui,uVar9);
    }
    if (local_30 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    return local_17b ^ 1;
  }
  if (local_f0 < 5) {
    if (local_f0 == 2) goto LAB_001025e3;
    if (local_f0 == 3) {
      xalloc_die();
    }
  }
                    /* WARNING: Subroutine does not return */
  __assert_fail("!\"unexpected error code from argv_iter\"",
                "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/du.c",0x42a,
                (char *)&__PRETTY_FUNCTION___1);
switchD_001017a9_caseD_86:
  opt_inodes = '\x01';
  goto LAB_001016da;
}

