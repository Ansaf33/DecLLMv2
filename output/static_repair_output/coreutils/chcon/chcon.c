// Function: compute_context_from_mask
undefined8 compute_context_from_mask(undefined8 param_1,int *param_2) {
  bool bVar1;
  int iVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  int *piVar6;
  
  bVar1 = true;
  iVar2 = context_new(param_1);
  if (iVar2 == 0) {
    uVar4 = quote(param_1);
    uVar5 = gettext("failed to create security context: %s");
    piVar6 = __errno_location();
    error(0,*piVar6,uVar5,uVar4);
    uVar4 = 1;
  }
  else {
    if (specified_user != 0) {
      iVar3 = context_user_set(iVar2,specified_user);
      if (iVar3 != 0) {
        uVar4 = quote(specified_user);
        uVar5 = gettext("failed to set %s security context component to %s");
        piVar6 = __errno_location();
        error(0,*piVar6,uVar5,&DAT_00101b89,uVar4);
        bVar1 = false;
      }
    }
    if (specified_range != 0) {
      iVar3 = context_range_set(iVar2,specified_range);
      if (iVar3 != 0) {
        uVar4 = quote(specified_range);
        uVar5 = gettext("failed to set %s security context component to %s");
        piVar6 = __errno_location();
        error(0,*piVar6,uVar5,"range",uVar4);
        bVar1 = false;
      }
    }
    if (specified_role != 0) {
      iVar3 = context_role_set(iVar2,specified_role);
      if (iVar3 != 0) {
        uVar4 = quote(specified_role);
        uVar5 = gettext("failed to set %s security context component to %s");
        piVar6 = __errno_location();
        error(0,*piVar6,uVar5,&DAT_00101b8e,uVar4);
        bVar1 = false;
      }
    }
    if (specified_type != 0) {
      iVar3 = context_type_set(iVar2,specified_type);
      if (iVar3 != 0) {
        uVar4 = quote(specified_type);
        uVar5 = gettext("failed to set %s security context component to %s");
        piVar6 = __errno_location();
        error(0,*piVar6,uVar5,&DAT_00101b93,uVar4);
        bVar1 = false;
      }
    }
    if (bVar1) {
      *param_2 = iVar2;
      uVar4 = 0;
    }
    else {
      piVar6 = __errno_location();
      iVar3 = *piVar6;
      context_free(iVar2);
      piVar6 = __errno_location();
      *piVar6 = iVar3;
      uVar4 = 1;
    }
  }
  return uVar4;
}

// Function: change_file_context
undefined4 change_file_context(undefined4 param_1,undefined8 param_2) {
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  long in_FS_OFFSET;
  undefined4 local_80;
  undefined4 local_7c;
  int local_78;
  int local_74;
  undefined4 local_70;
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  int local_5c;
  long local_58;
  long local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  local_58 = 0;
  local_80 = 0;
  local_7c = 0;
  if (specified_context == 0) {
    if (affect_symlink_referent == '\0') {
      local_78 = lgetfileconat(param_1,param_2,&local_58);
    }
    else {
      local_78 = getfileconat(param_1,param_2,&local_58);
    }
    if ((local_78 < 0) && (piVar4 = __errno_location(), *piVar4 != 0x3d)) {
      uVar5 = quotearg_style(4,param_2);
      uVar6 = gettext("failed to get security context of %s");
      piVar4 = __errno_location();
      local_6c = *piVar4;
      local_70 = 0;
      local_48 = uVar6;
      error(0,local_6c,uVar6,uVar5);
      uVar3 = 1;
      goto LAB_00100837;
    }
    if (local_58 == 0) {
      uVar5 = quotearg_style(4,param_2);
      local_40 = gettext("can\'t apply partial context to unlabeled file %s");
      local_68 = 0;
      local_64 = 0;
      error(0,0,local_40,uVar5);
      uVar3 = 1;
      goto LAB_00100837;
    }
    iVar2 = compute_context_from_mask(local_58,&local_80);
    if (iVar2 != 0) {
      uVar3 = 1;
      goto LAB_00100837;
    }
    local_50 = context_str(local_80);
  }
  else {
    local_50 = specified_context;
  }
  if ((local_58 == 0) || (cVar1 = streq(local_50,local_58), cVar1 != '\x01')) {
    if (affect_symlink_referent == '\0') {
      local_74 = lsetfileconat(param_1,param_2,local_50);
    }
    else {
      local_74 = setfileconat(param_1,param_2,local_50);
    }
    if (local_74 != 0) {
      local_7c = 1;
      uVar5 = quote_n(1,local_50);
      uVar6 = quotearg_n_style(0,4,param_2);
      uVar7 = gettext("failed to change context of %s to %s");
      piVar4 = __errno_location();
      local_5c = *piVar4;
      local_60 = 0;
      local_38 = uVar7;
      error(0,local_5c,uVar7,uVar6,uVar5);
    }
  }
  uVar3 = local_7c;
  if (specified_context == 0) {
    context_free(local_80);
    freecon(local_58);
    uVar3 = local_7c;
  }
LAB_00100837:
  if (local_30 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar3;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: process_file
char process_file(long param_1,long param_2) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  char *__format;
  char cVar8;
  
  uVar7 = *(undefined8 *)(param_2 + 0x38);
  uVar3 = *(undefined8 *)(param_2 + 0x30);
  cVar8 = '\x01';
  switch(*(undefined2 *)(param_2 + 0x68)) {
  case 1:
    if (recurse != '\0') {
      if ((root_dev_ino != (ulong *)0x0) &&
         ((*root_dev_ino ^ *(ulong *)(param_2 + 0x78) | *(ulong *)(param_2 + 0x70) ^ root_dev_ino[1]
          ) == 0)) {
        cVar8 = streq(uVar7,&DAT_00101c9d);
        if (cVar8 == '\0') {
          uVar3 = quotearg_n_style(1,4,&DAT_00101c9d);
          uVar7 = quotearg_n_style(0,4,uVar7);
          uVar4 = gettext("it is dangerous to operate recursively on %s (same as %s)");
          error(0,0,uVar4,uVar7,uVar3);
        }
        else {
          uVar7 = quotearg_style(4,uVar7);
          uVar3 = gettext("it is dangerous to operate recursively on %s");
          error(0,0,uVar3,uVar7);
        }
        uVar7 = gettext("use --no-preserve-root to override this failsafe");
        error(0,0,uVar7);
        rpl_fts_set(param_1,param_2,4);
        rpl_fts_read(param_1);
        return '\0';
      }
      return '\x01';
    }
    break;
  case 2:
    cVar1 = cycle_warning_required(param_1,param_2);
    if (cVar1 != '\0') {
      uVar7 = quotearg_n_style_colon(0,3,uVar7);
      uVar3 = gettext(
                     "WARNING: Circular directory structure.\nThis almost certainly means that you have a corrupted file system.\nNOTIFY YOUR SYSTEM MANAGER.\nThe following directory is part of the cycle:\n  %s\n"
                     );
      error(0,0,uVar3,uVar7);
      return '\0';
    }
    break;
  case 4:
    uVar4 = quotearg_style(4,uVar7);
    uVar5 = gettext("cannot read directory %s");
    error(0,*(undefined4 *)(param_2 + 0x40),uVar5,uVar4);
    cVar8 = '\0';
    break;
  case 6:
    if (recurse != '\x01') {
      return '\x01';
    }
    break;
  case 7:
    uVar4 = quotearg_n_style_colon(0,3,uVar7);
    error(0,*(undefined4 *)(param_2 + 0x40),&DAT_00101d52,uVar4);
    cVar8 = '\0';
    break;
  case 10:
    if ((*(long *)(param_2 + 0x58) == 0) && (*(long *)(param_2 + 0x20) == 0)) {
      *(undefined8 *)(param_2 + 0x20) = 1;
      rpl_fts_set(param_1,param_2,1);
      return '\x01';
    }
    uVar4 = quotearg_style(4,uVar7);
    uVar5 = gettext("cannot access %s");
    error(0,*(undefined4 *)(param_2 + 0x40),uVar5,uVar4);
    cVar8 = '\0';
  }
  if ((((*(short *)(param_2 + 0x68) == 6) && (cVar8 != '\0')) && (root_dev_ino != (ulong *)0x0)) &&
     ((*root_dev_ino ^ *(ulong *)(param_2 + 0x78) | *(ulong *)(param_2 + 0x70) ^ root_dev_ino[1]) ==
      0)) {
    cVar8 = streq(uVar7,&DAT_00101c9d);
    if (cVar8 == '\0') {
      uVar4 = quotearg_n_style(1,4,&DAT_00101c9d);
      uVar5 = quotearg_n_style(0,4,uVar7);
      uVar6 = gettext("it is dangerous to operate recursively on %s (same as %s)");
      error(0,0,uVar6,uVar5,uVar4);
    }
    else {
      uVar4 = quotearg_style(4,uVar7);
      uVar5 = gettext("it is dangerous to operate recursively on %s");
      error(0,0,uVar5,uVar4);
    }
    uVar4 = gettext("use --no-preserve-root to override this failsafe");
    error(0,0,uVar4);
    cVar8 = '\0';
  }
  if (cVar8 != '\0') {
    if (verbose != '\0') {
      uVar7 = quotearg_style(4,uVar7);
      __format = (char *)gettext("changing security context of %s\n");
      printf(__format,uVar7);
    }
    iVar2 = change_file_context(*(undefined4 *)(param_1 + 0x2c),uVar3);
    if (iVar2 != 0) {
      cVar8 = '\0';
    }
  }
  if (recurse != '\x01') {
    rpl_fts_set(param_1,param_2,4);
  }
  return cVar8;
}

// Function: process_files
bool process_files(undefined8 param_1,undefined4 param_2) {
  byte bVar1;
  int iVar2;
  undefined8 uVar3;
  long lVar4;
  int *piVar5;
  undefined8 uVar6;
  bool local_49;
  
  local_49 = true;
  uVar3 = xfts_open(param_1,param_2,0);
  while( true ) {
    lVar4 = rpl_fts_read(uVar3);
    if (lVar4 == 0) break;
    bVar1 = process_file(uVar3,lVar4);
    local_49 = (bVar1 & local_49) != 0;
  }
  piVar5 = __errno_location();
  if (*piVar5 != 0) {
    uVar6 = gettext("fts_read failed");
    piVar5 = __errno_location();
    error(0,*piVar5,uVar6);
    local_49 = false;
  }
  iVar2 = rpl_fts_close(uVar3);
  if (iVar2 != 0) {
    uVar3 = gettext("fts_close failed");
    piVar5 = __errno_location();
    error(0,*piVar5,uVar3);
    local_49 = false;
  }
  return local_49;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... CONTEXT FILE...\n  or:  %s [OPTION]... [-u USER] [-r ROLE] [-l RANGE] [-t TYPE] FILE...\n  or:  %s [OPTION]... --reference=RFILE FILE...\n"
                            );
    printf(pcVar3,uVar2,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Change the SELinux security context of each FILE to CONTEXT.\nWith --reference, change the security context of each FILE to that of RFILE.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --dereference      affect the referent of each symbolic link (this is\n                         the default), rather than the symbolic link itself\n  -h, --no-dereference   affect symbolic links instead of any referenced file\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -u, --user=USER        set user USER in the target security context\n  -r, --role=ROLE        set role ROLE in the target security context\n  -t, --type=TYPE        set type TYPE in the target security context\n  -l, --range=RANGE      set range RANGE in the target security context\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --no-preserve-root  do not treat \'/\' specially (the default)\n      --preserve-root    fail to operate recursively on \'/\'\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --reference=RFILE  use RFILE\'s security context rather than specifying\n                         a CONTEXT value\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -R, --recursive        operate on files and directories recursively\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -v, --verbose          output a diagnostic for every file processed\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nThe following options modify how a hierarchy is traversed when the -R\noption is also specified.  If more than one is specified, only the final\none takes effect.\n\n  -H                     if a command line argument is a symbolic link\n                         to a directory, traverse it\n  -L                     traverse every symbolic link to a directory\n                         encountered\n  -P                     do not traverse any symbolic links (default)\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_ancillary_info("chcon");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: main
byte main(int param_1,undefined8 *param_2) {
  bool bVar1;
  bool bVar2;
  byte bVar3;
  int iVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  int *piVar7;
  long in_FS_OFFSET;
  uint local_ac;
  int local_a8;
  undefined8 local_68;
  long local_60;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_ac = 0x10;
  local_a8 = -1;
  bVar2 = false;
  bVar1 = false;
  local_60 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
LAB_00101527:
  iVar4 = getopt_long(param_1,param_2,"HLPRhvu:r:t:l:",long_options,0);
  if (iVar4 != -1) {
    if (iVar4 < 0x84) {
      if (iVar4 < 0x48) {
        if (iVar4 == -0x83) {
LAB_0010149e:
          uVar5 = proper_name_lite("Jim Meyering","Jim Meyering");
          uVar6 = proper_name_lite("Russell Coker","Russell Coker");
          version_etc(_stdout,"chcon","GNU coreutils",_Version,uVar6,uVar5,0);
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        if (iVar4 == -0x82) {
          usage(0);
          goto LAB_0010149e;
        }
        goto switchD_0010139e_caseD_49;
      }
      goto code_r0x0010137e;
    }
    goto switchD_0010139e_caseD_49;
  }
  if (recurse == '\0') {
    local_ac = 0x10;
    affect_symlink_referent = local_a8 != 0;
  }
  else if (local_ac == 0x10) {
    if (local_a8 == 1) {
      local_58 = gettext("-R --dereference requires either -H or -L");
      error(1,0,local_58);
    }
    affect_symlink_referent = false;
  }
  else {
    if (local_a8 == 0) {
      local_50 = gettext("-R -h requires -P");
      error(1,0,local_50);
    }
    affect_symlink_referent = true;
  }
  if ((local_60 != 0) || (bVar1)) {
    iVar4 = 1;
  }
  else {
    iVar4 = 2;
  }
  if (param_1 - _optind < iVar4) {
    if (_optind < param_1) {
      uVar5 = quote(param_2[(long)param_1 + -1]);
      local_40 = gettext("missing operand after %s");
      error(0,0,local_40,uVar5);
    }
    else {
      local_48 = gettext("missing operand");
      error(0,0,local_48);
    }
    usage(1);
  }
  if (local_60 == 0) {
    if (bVar1) {
      specified_context = 0;
    }
    else {
      specified_context = param_2[_optind];
      _optind = _optind + 1;
    }
  }
  else {
    local_68 = 0;
    iVar4 = getfilecon(local_60,&local_68);
    if (iVar4 < 0) {
      uVar5 = quotearg_style(4,local_60);
      uVar6 = gettext("failed to get security context of %s");
      piVar7 = __errno_location();
      local_38 = uVar6;
      error(1,*piVar7,uVar6,uVar5);
    }
    specified_context = local_68;
  }
  if ((local_60 != 0) && (bVar1)) {
    local_30 = gettext("conflicting security context specifiers given");
    error(0,0,local_30);
    usage(1);
  }
  if ((recurse != '\0') && (bVar2)) {
    root_dev_ino = get_root_dev_ino(dev_ino_buf_0);
    if (root_dev_ino != 0) goto LAB_001018fe;
    uVar5 = quotearg_style(4,&DAT_00101c9d);
    uVar6 = gettext("failed to get attributes of %s");
    piVar7 = __errno_location();
    local_28 = uVar6;
    error(1,*piVar7,uVar6,uVar5);
  }
  root_dev_ino = 0;
LAB_001018fe:
  bVar3 = process_files(param_2 + _optind,local_ac | 8);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return bVar3 ^ 1;
code_r0x0010137e:
  switch(iVar4) {
  case 0x48:
    local_ac = 0x11;
    break;
  default:
switchD_0010139e_caseD_49:
    usage(1);
    break;
  case 0x4c:
    local_ac = 2;
    break;
  case 0x50:
    local_ac = 0x10;
    break;
  case 0x52:
    recurse = '\x01';
    break;
  case 0x66:
    break;
  case 0x68:
    local_a8 = 0;
    break;
  case 0x6c:
    specified_range = _optarg;
    bVar1 = true;
    break;
  case 0x72:
    specified_role = _optarg;
    bVar1 = true;
    break;
  case 0x74:
    specified_type = _optarg;
    bVar1 = true;
    break;
  case 0x75:
    specified_user = _optarg;
    bVar1 = true;
    break;
  case 0x76:
    verbose = 1;
    break;
  case 0x80:
    local_a8 = 1;
    break;
  case 0x81:
    bVar2 = false;
    break;
  case 0x82:
    bVar2 = true;
    break;
  case 0x83:
    local_60 = _optarg;
  }
  goto LAB_00101527;
}

