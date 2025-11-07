// Function: have_same_content
bool have_same_content(undefined4 param_1,undefined4 param_2) {
  char cVar1;
  long lVar2;
  long lVar3;
  
  while( true ) {
    lVar3 = full_read(param_1,a_buff_2,0x1000);
    if (lVar3 < 1) {
      return lVar3 == 0;
    }
    lVar2 = full_read(param_2,b_buff_3,0x1000);
    if (lVar3 != lVar2) break;
    cVar1 = memeq(a_buff_2,b_buff_3,lVar3);
    if (cVar1 != '\x01') {
      return false;
    }
  }
  return false;
}

// Function: extra_mode
undefined4 extra_mode(uint param_1) {
  return CONCAT31((int3)((param_1 & 0xffff0e00) >> 8),(param_1 & 0xffff0e00) != 0);
}

// Function: need_copy
bool need_copy(char *param_1,undefined8 param_2,int param_3,char *param_4,long param_5) {
  char cVar1;
  int iVar2;
  __uid_t _Var3;
  __gid_t _Var4;
  int __fd;
  int *piVar5;
  long in_FS_OFFSET;
  bool bVar6;
  undefined8 local_148;
  undefined8 local_140;
  stat local_138;
  stat local_a8;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  cVar1 = extra_mode(mode);
  if (cVar1 != '\0') {
    bVar6 = true;
    goto LAB_001007ba;
  }
  iVar2 = stat(param_1,&local_138);
  if (iVar2 != 0) {
    bVar6 = true;
    goto LAB_001007ba;
  }
  iVar2 = fstatat(param_3,param_4,&local_a8,0x100);
  if (iVar2 != 0) {
    bVar6 = true;
    goto LAB_001007ba;
  }
  if (((local_138.st_mode & 0xf000) == 0x8000) && ((local_a8.st_mode & 0xf000) == 0x8000)) {
    cVar1 = extra_mode(local_138.st_mode);
    if (cVar1 == '\0') {
      cVar1 = extra_mode(local_a8.st_mode);
      if (cVar1 == '\0') {
        if ((local_138.st_size != local_a8.st_size) || ((local_a8.st_mode & 0xfff) != mode)) {
          bVar6 = true;
          goto LAB_001007ba;
        }
        if (owner_id == 0xffffffff) {
          piVar5 = __errno_location();
          *piVar5 = 0;
          _Var3 = getuid();
          if (_Var3 == 0xffffffff) {
            piVar5 = __errno_location();
            if (*piVar5 == 0) goto LAB_0010057c;
          }
          else {
LAB_0010057c:
            if (_Var3 == local_a8.st_uid) goto LAB_001005ae;
          }
          bVar6 = true;
          goto LAB_001007ba;
        }
        if (local_a8.st_uid != owner_id) {
          bVar6 = true;
          goto LAB_001007ba;
        }
LAB_001005ae:
        if (group_id == 0xffffffff) {
          piVar5 = __errno_location();
          *piVar5 = 0;
          _Var4 = getgid();
          if (_Var4 == 0xffffffff) {
            piVar5 = __errno_location();
            if (*piVar5 == 0) goto LAB_001005e3;
          }
          else {
LAB_001005e3:
            if (_Var4 == local_a8.st_gid) goto LAB_0010060f;
          }
          bVar6 = true;
          goto LAB_001007ba;
        }
        if (local_a8.st_gid != group_id) {
          bVar6 = true;
          goto LAB_001007ba;
        }
LAB_0010060f:
        if ((selinux_enabled != 0) && (*(char *)(param_5 + 0x33) != '\0')) {
          local_148 = 0;
          local_140 = 0;
          iVar2 = getfilecon_raw(param_1,&local_148);
          if (iVar2 == -1) {
            bVar6 = true;
            goto LAB_001007ba;
          }
          iVar2 = getfilecon_raw(param_2,&local_140);
          if (iVar2 == -1) {
            freecon(local_148);
            bVar6 = true;
            goto LAB_001007ba;
          }
          cVar1 = streq(local_148,local_140);
          freecon(local_148);
          freecon(local_140);
          if (cVar1 != '\x01') {
            bVar6 = true;
            goto LAB_001007ba;
          }
        }
        iVar2 = open(param_1,0);
        if (iVar2 < 0) {
          bVar6 = true;
        }
        else {
          __fd = openat(param_3,param_4,0);
          if (__fd < 0) {
            close(iVar2);
            bVar6 = true;
          }
          else {
            cVar1 = have_same_content(iVar2,__fd);
            close(iVar2);
            close(__fd);
            bVar6 = cVar1 == '\0';
          }
        }
        goto LAB_001007ba;
      }
    }
  }
  bVar6 = true;
LAB_001007ba:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return bVar6;
}

// Function: cp_option_init
void cp_option_init(undefined4 *param_1) {
  cp_options_default(param_1);
  *(undefined *)(param_1 + 5) = 1;
  param_1[0x13] = 1;
  param_1[1] = 4;
  *(undefined *)((long)param_1 + 0x15) = 1;
  *(undefined *)((long)param_1 + 0x16) = 0;
  *(undefined *)((long)param_1 + 0x17) = 0;
  param_1[2] = 0;
  *(undefined *)(param_1 + 6) = 0;
  *(undefined *)((long)param_1 + 0x1b) = 1;
  *(undefined *)((long)param_1 + 0x1e) = 0;
  *(undefined *)((long)param_1 + 0x1f) = 0;
  *(undefined *)(param_1 + 0xc) = 0;
  *(undefined *)(param_1 + 8) = 0;
  *(undefined *)((long)param_1 + 0x21) = 0;
  *(undefined *)((long)param_1 + 0x22) = 0;
  *(undefined *)((long)param_1 + 0x37) = 0;
  *(undefined *)((long)param_1 + 0x31) = 1;
  *(undefined *)((long)param_1 + 0x32) = 0;
  *(undefined *)((long)param_1 + 0x36) = 0;
  *(undefined *)(param_1 + 0xe) = 0;
  param_1[3] = 2;
  *(undefined *)((long)param_1 + 0x3a) = 0;
  *param_1 = 0;
  *(undefined *)((long)param_1 + 0x39) = 1;
  param_1[4] = 0x180;
  *(undefined *)((long)param_1 + 0x43) = 0;
  *(undefined *)(param_1 + 0x11) = 0;
  param_1[0xf] = 0;
  *(undefined *)(param_1 + 0xd) = 0;
  *(undefined *)((long)param_1 + 0x33) = 0;
  *(undefined8 *)(param_1 + 10) = 0;
  *(undefined *)((long)param_1 + 0x35) = 0;
  *(undefined *)(param_1 + 0x10) = 0;
  *(undefined8 *)(param_1 + 0x14) = 0;
  *(undefined8 *)(param_1 + 0x16) = 0;
  return;
}

// Function: get_labeling_handle
long get_labeling_handle(void) {
  undefined8 uVar1;
  int *piVar2;
  
  if (initialized_1 != '\x01') {
    initialized_1 = '\x01';
    hnd_0 = selabel_open(0,0,0);
    if (hnd_0 == 0) {
      uVar1 = gettext("warning: security labeling handle failed");
      piVar2 = __errno_location();
      error(0,*piVar2,uVar1);
    }
  }
  return hnd_0;
}

// Function: setdefaultfilecon
void setdefaultfilecon(char *param_1) {
  char cVar1;
  int iVar2;
  int *piVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  long in_FS_OFFSET;
  undefined8 local_e8;
  long local_e0;
  undefined8 local_d8;
  undefined8 local_d0;
  stat local_c8;
  long local_30;
  
  local_30 = *(long *)(in_FS_OFFSET + 0x28);
  local_e8 = 0;
  if (((selinux_enabled == 1) && (iVar2 = lstat(param_1,&local_c8), iVar2 == 0)) &&
     (local_e0 = get_labeling_handle(), local_e0 != 0)) {
    iVar2 = selabel_lookup_raw(local_e0,&local_e8,param_1,local_c8.st_mode);
    if (iVar2 == 0) {
      iVar2 = lsetfilecon_raw(param_1,local_e8);
      if ((iVar2 < 0) && (piVar3 = __errno_location(), *piVar3 != 0x5f)) {
        uVar4 = quote_n(1,local_e8);
        uVar5 = quotearg_n_style_colon(0,3,param_1);
        uVar6 = gettext("warning: %s: failed to change context to %s");
        piVar3 = __errno_location();
        local_d0 = uVar6;
        error(0,*piVar3,uVar6,uVar5,uVar4);
      }
      freecon(local_e8);
    }
    else {
      piVar3 = __errno_location();
      if (*piVar3 != 2) {
        piVar3 = __errno_location();
        cVar1 = ignorable_ctx_err(*piVar3);
        if (cVar1 != '\x01') {
          uVar4 = quotearg_n_style_colon(0,3,param_1);
          uVar5 = gettext("warning: %s: context lookup failed");
          piVar3 = __errno_location();
          local_d8 = uVar5;
          error(0,*piVar3,uVar5,uVar4);
        }
      }
    }
  }
  if (local_30 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: announce_mkdir
void announce_mkdir(undefined8 param_1,long param_2) {
  undefined8 uVar1;
  undefined8 uVar2;
  
  if (*(char *)(param_2 + 0x40) != '\0') {
    uVar1 = quotearg_style(4,param_1);
    uVar2 = gettext("creating directory %s");
    prog_fprintf(_stdout,uVar2,uVar1);
  }
  return;
}

// Function: make_ancestor
int make_ancestor(undefined8 param_1,char *param_2,long param_3) {
  char cVar1;
  int iVar2;
  int *piVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  
  if (*(long *)(param_3 + 0x28) != 0) {
    iVar2 = defaultcon(*(undefined8 *)(param_3 + 0x28),param_2,0x4000);
    if (iVar2 < 0) {
      piVar3 = __errno_location();
      cVar1 = ignorable_ctx_err(*piVar3);
      if (cVar1 != '\x01') {
        uVar4 = quotearg_style(4,param_1);
        uVar5 = gettext("failed to set default creation context for %s");
        piVar3 = __errno_location();
        error(0,*piVar3,uVar5,uVar4);
      }
    }
  }
  iVar2 = mkdir(param_2,0x1ed);
  if (iVar2 == 0) {
    announce_mkdir(param_1,param_3);
  }
  return iVar2;
}

// Function: process_dir
byte process_dir(undefined8 param_1,undefined8 param_2,long param_3) {
  byte bVar1;
  char cVar2;
  undefined8 uVar3;
  int *piVar4;
  undefined8 uVar5;
  
  bVar1 = make_dir_parents(param_1,param_2,make_ancestor,param_3,dir_mode,announce_mkdir,
                           dir_mode_bits,owner_id,group_id,0);
  if (((bVar1 ^ 1) == 0) && (*(long *)(param_3 + 0x28) != 0)) {
    uVar3 = last_component(param_1);
    cVar2 = restorecon(*(undefined8 *)(param_3 + 0x28),uVar3,0);
    if (cVar2 != '\x01') {
      piVar4 = __errno_location();
      cVar2 = ignorable_ctx_err(*piVar4);
      if (cVar2 != '\x01') {
        uVar3 = quotearg_style(4,param_1);
        uVar5 = gettext("failed to restore context for %s");
        piVar4 = __errno_location();
        error(0,*piVar4,uVar5,uVar3);
      }
    }
  }
  return bVar1 ^ 1;
}

// Function: copy_file
int copy_file(EVP_PKEY_CTX *param_1,EVP_PKEY_CTX *param_2,undefined4 param_3,undefined8 param_4,
             undefined8 param_5) {
  long lVar1;
  char cVar2;
  int iVar3;
  long in_FS_OFFSET;
  
  lVar1 = *(long *)(in_FS_OFFSET + 0x28);
  if (copy_only_if_needed != '\0') {
    cVar2 = need_copy(param_1,param_2,param_3,param_4,param_5);
    if (cVar2 != '\x01') {
      iVar3 = 1;
      goto LAB_00100f2f;
    }
  }
  iVar3 = copy(param_1,param_2);
LAB_00100f2f:
  if (lVar1 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return iVar3;
}

// Function: change_attributes
undefined change_attributes(undefined8 param_1,undefined4 param_2,undefined8 param_3) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  undefined local_39;
  
  local_39 = 0;
  if ((owner_id != -1) || (group_id != -1)) {
    iVar1 = lchownat(param_2,param_3,owner_id,group_id);
    if (iVar1 != 0) {
      uVar2 = quotearg_style(4,param_1);
      uVar3 = gettext("cannot change ownership of %s");
      piVar4 = __errno_location();
      error(0,*piVar4,uVar3,uVar2);
      goto LAB_00101063;
    }
  }
  iVar1 = chmodat(param_2,param_3,mode);
  if (iVar1 == 0) {
    local_39 = 1;
  }
  else {
    uVar2 = quotearg_style(4,param_1);
    uVar3 = gettext("cannot change permissions of %s");
    piVar4 = __errno_location();
    error(0,*piVar4,uVar3,uVar2);
  }
LAB_00101063:
  if (use_default_selinux_context != '\0') {
    setdefaultfilecon(param_1);
  }
  return local_39;
}

// Function: change_timestamps
bool change_timestamps(undefined8 param_1,undefined8 param_2,int param_3,char *param_4) {
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  long in_FS_OFFSET;
  timespec local_48;
  undefined local_38 [24];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_48 = (timespec)get_stat_atime(param_1);
  local_38._0_16_ = get_stat_mtime(param_1);
  iVar1 = utimensat(param_3,param_4,&local_48,0);
  if (iVar1 != 0) {
    uVar2 = quotearg_style(4,param_2);
    uVar3 = gettext("cannot set timestamps for %s");
    piVar4 = __errno_location();
    error(0,*piVar4,uVar3,uVar2);
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return iVar1 == 0;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: strip
undefined strip(char *param_1) {
  char cVar1;
  int iVar2;
  __pid_t _Var3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  undefined local_1ed;
  __pid_t local_1ec;
  uint local_1e8;
  int local_1e4;
  undefined4 local_1e0;
  int local_1dc;
  undefined4 local_1d8;
  int local_1d4;
  undefined4 local_1d0;
  undefined4 local_1cc;
  posix_spawnattr_t *local_1c8;
  char *local_1c0;
  char *local_1b8;
  undefined8 local_1b0;
  undefined8 local_1a8;
  undefined8 local_1a0;
  char *local_198;
  char *local_190;
  undefined8 local_188;
  posix_spawnattr_t local_178;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_1c8 = (posix_spawnattr_t *)0x0;
  iVar2 = posix_spawnattr_init(&local_178);
  if (iVar2 == 0) {
    iVar2 = posix_spawnattr_setflags(&local_178,0x40);
    if (iVar2 == 0) {
      local_1c8 = &local_178;
    }
    else {
      posix_spawnattr_destroy(&local_178);
    }
  }
  local_1c0 = (char *)0x0;
  local_1b8 = param_1;
  if ((param_1 != (char *)0x0) && (*param_1 == '-')) {
    local_1c0 = (char *)file_name_concat(&DAT_001053e5,param_1,0);
    local_1b8 = local_1c0;
  }
  local_198 = strip_program;
  local_190 = local_1b8;
  local_188 = 0;
  local_1e4 = posix_spawnp(&local_1ec,strip_program,(posix_spawn_file_actions_t *)0x0,local_1c8,
                           &local_198,_environ);
  local_1ed = 0;
  if (local_1e4 == 0) {
    _Var3 = waitpid(local_1ec,(int *)&local_1e8,0);
    if (_Var3 < 0) {
      uVar4 = gettext("waiting for strip");
      piVar5 = __errno_location();
      local_1d4 = *piVar5;
      local_1d8 = 0;
      local_1a8 = uVar4;
      error(0,local_1d4,uVar4);
    }
    else if (((local_1e8 & 0x7f) == 0) && ((local_1e8 & 0xff00) == 0)) {
      local_1ed = 1;
    }
    else {
      local_1a0 = gettext("strip process terminated abnormally");
      local_1d0 = 0;
      local_1cc = 0;
      error(0,0,local_1a0);
    }
  }
  else {
    uVar4 = quotearg_style(4,strip_program);
    cVar1 = streq(strip_program,"strip");
    if (cVar1 == '\0') {
      local_1b0 = gettext("cannot run strip program %s");
    }
    else {
      local_1b0 = gettext("cannot run %s");
    }
    local_1e0 = 0;
    local_1dc = local_1e4;
    error(0,local_1e4,local_1b0,uVar4);
  }
  free(local_1c0);
  if (local_1c8 != (posix_spawnattr_t *)0x0) {
    posix_spawnattr_destroy(local_1c8);
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_1ed;
}

// Function: get_ids
void get_ids(void) {
  int iVar1;
  undefined8 uVar2;
  long in_FS_OFFSET;
  ulong local_48;
  passwd *local_40;
  group *local_38;
  undefined8 local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (owner_name == (char *)0x0) {
    owner_id = 0xffffffff;
  }
  else {
    local_40 = getpwnam(owner_name);
    if (local_40 == (passwd *)0x0) {
      iVar1 = xstrtoumax(owner_name,0,0,&local_48,&DAT_001051ac);
      if ((iVar1 != 0) || (owner_id = (__uid_t)local_48, local_48 != (local_48 & 0xffffffff))) {
        uVar2 = quotearg_style(4,owner_name);
        local_30 = gettext("invalid user %s");
        error(1,0,local_30,uVar2);
        goto LAB_00101589;
      }
    }
    else {
LAB_00101589:
      owner_id = local_40->pw_uid;
    }
    endpwent();
  }
  if (group_name == (char *)0x0) {
    group_id = 0xffffffff;
    goto code_r0x001016a0;
  }
  local_38 = getgrnam(group_name);
  if (local_38 == (group *)0x0) {
    iVar1 = xstrtoumax(group_name,0,0,&local_48,&DAT_001051ac);
    if ((iVar1 != 0) || (group_id = (__gid_t)local_48, local_48 != (local_48 & 0xffffffff))) {
      uVar2 = quotearg_style(4,group_name);
      local_28 = gettext("invalid group %s");
      error(1,0,local_28,uVar2);
      goto LAB_00101681;
    }
  }
  else {
LAB_00101681:
    group_id = local_38->gr_gid;
  }
  endgrent();
code_r0x001016a0:
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext(
                            "Usage: %s [OPTION]... [-T] SOURCE DEST\n  or:  %s [OPTION]... SOURCE... DIRECTORY\n  or:  %s [OPTION]... -t DIRECTORY SOURCE...\n  or:  %s [OPTION]... -d DIRECTORY...\n"
                            );
    printf(pcVar3,uVar2,uVar2,uVar2,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nThis install program copies files (often just compiled) into destination\nlocations you choose.  If you want to download and install a ready-to-use\npackage on a GNU/Linux system, you should instead be using a package manager\nlike yum(1) or apt-get(1).\n\nIn the first three forms, copy SOURCE to DEST or multiple SOURCE(s) to\nthe existing DIRECTORY, while setting permission modes and owner/group.\nIn the 4th form, create all components of the given DIRECTORY(ies).\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --backup[=CONTROL]  make a backup of each existing destination file\n  -b                  like --backup but does not accept an argument\n  -c                  (ignored)\n  -C, --compare       compare content of source and destination files, and\n                        if no change to content, ownership, and permissions,\n                        do not modify the destination at all\n  -d, --directory     treat all arguments as directory names; create all\n                        components of the specified directories\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -D                  create all leading components of DEST except the last,\n                        or all components of --target-directory,\n                        then copy SOURCE to DEST\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --debug         explain how a file is copied.  Implies -v\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -g, --group=GROUP   set group ownership, instead of process\' current group\n  -m, --mode=MODE     set permission mode (as in chmod), instead of rwxr-xr-x\n  -o, --owner=OWNER   set ownership (super-user only)\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -p, --preserve-timestamps   apply access/modification times of SOURCE files\n                        to corresponding destination files\n  -s, --strip         strip symbol tables\n      --strip-program=PROGRAM  program used to strip binaries\n  -S, --suffix=SUFFIX  override the usual backup suffix\n  -t, --target-directory=DIRECTORY  copy all SOURCE arguments into DIRECTORY\n  -T, --no-target-directory  treat DEST as a normal file\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -v, --verbose       print the name of each created file or directory\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --preserve-context  preserve SELinux security context\n  -Z                      set SELinux security context of destination\n                            file and each created directory to default type\n      --context[=CTX]     like -Z, or if CTX is specified then set the\n                            SELinux or SMACK security context to CTX\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_backup_suffix_note();
    emit_ancillary_info("install");
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: install_file_in_file
undefined8
install_file_in_file(char *param_1,undefined8 param_2,int param_3,char *param_4,long param_5) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (*(char *)(param_5 + 0x21) != '\0') {
    iVar2 = stat(param_1,&local_b8);
    if (iVar2 != 0) {
      uVar3 = quotearg_style(4,param_1);
      uVar4 = gettext("cannot stat %s");
      piVar5 = __errno_location();
      error(0,*piVar5,uVar4,uVar3);
      uVar3 = 0;
      goto LAB_00101b19;
    }
  }
  cVar1 = copy_file(param_1,param_2,param_3,param_4,param_5);
  if (cVar1 == '\x01') {
    if (strip_files != '\0') {
      cVar1 = strip(param_2);
      if (cVar1 != '\x01') {
        iVar2 = unlinkat(param_3,param_4,0);
        if (iVar2 != 0) {
          uVar3 = quotearg_style(4,param_2);
          uVar4 = gettext("cannot unlink %s");
          piVar5 = __errno_location();
          error(1,*piVar5,uVar4,uVar3);
        }
        uVar3 = 0;
        goto LAB_00101b19;
      }
    }
    if ((*(char *)(param_5 + 0x21) != '\0') &&
       ((strip_files != '\0' || ((local_b8.st_mode & 0xf000) != 0x8000)))) {
      cVar1 = change_timestamps(&local_b8,param_2,param_3,param_4);
      if (cVar1 != '\x01') {
        uVar3 = 0;
        goto LAB_00101b19;
      }
    }
    uVar3 = change_attributes(param_2,param_3,param_4);
  }
  else {
    uVar3 = 0;
  }
LAB_00101b19:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar3;
}

// Function: mkancesdirs_safe_wd
bool mkancesdirs_safe_wd(char *param_1,char *param_2,undefined8 param_3,char param_4) {
  int iVar1;
  bool bVar2;
  int iVar3;
  long lVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  int *piVar7;
  long in_FS_OFFSET;
  bool bVar8;
  undefined local_28 [8];
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (((param_4 == '\0') && (*param_1 == '/')) && (*param_2 == '/')) {
    bVar2 = false;
  }
  else {
    bVar2 = true;
  }
  savewd_init(local_28);
  if (!bVar2) {
    savewd_finish(local_28);
  }
  lVar4 = mkancesdirs(param_2,local_28,make_ancestor,param_3);
  bVar8 = lVar4 == -1;
  if (bVar8) {
    uVar5 = quotearg_style(4,param_2);
    uVar6 = gettext("cannot create directory %s");
    piVar7 = __errno_location();
    error(0,*piVar7,uVar6,uVar5);
  }
  if (bVar2) {
    iVar3 = savewd_restore(local_28,bVar8);
    piVar7 = __errno_location();
    iVar1 = *piVar7;
    savewd_finish(local_28);
    if (0 < iVar3) {
      bVar8 = false;
      goto LAB_00101cea;
    }
    if ((iVar3 < 0) && (!bVar8)) {
      uVar5 = quotearg_style(4,param_2);
      uVar6 = gettext("cannot create directory %s");
      error(0,iVar1,uVar6,uVar5);
      bVar8 = false;
      goto LAB_00101cea;
    }
  }
  bVar8 = !bVar8;
LAB_00101cea:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return bVar8;
}

// Function: install_file_in_file_parents
undefined4 install_file_in_file_parents(undefined8 param_1,undefined8 param_2,undefined8 param_3) {
  char cVar1;
  
  cVar1 = mkancesdirs_safe_wd(param_1,param_2,param_3,0);
  if ((cVar1 != '\0') &&
     (cVar1 = install_file_in_file(param_1,param_2,0xffffff9c,param_2,param_3), cVar1 != '\0')) {
    return 1;
  }
  return 0;
}

// Function: install_file_in_dir
char install_file_in_dir(undefined8 param_1,char *param_2,undefined8 param_3,char param_4,
                        int *param_5) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  char local_51;
  int local_50;
  void *local_40;
  undefined8 local_38;
  void *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_38 = last_component(param_1);
  local_30 = (void *)file_name_concat(param_2,local_38,&local_40);
  local_51 = '\x01';
  cVar1 = target_dirfd_valid(*param_5);
  if ((cVar1 != '\x01') && (local_51 = param_4, param_4 != '\0')) {
    local_51 = mkancesdirs_safe_wd(param_1,local_30,param_3,1);
    if (local_51 != '\0') {
      iVar2 = open(param_2,0x210000);
      if (iVar2 < 0) {
        uVar3 = quotearg_style(4,local_30);
        uVar4 = gettext("cannot open %s");
        piVar5 = __errno_location();
        local_28 = uVar4;
        error(0,*piVar5,uVar4,uVar3);
        local_51 = '\0';
      }
      else {
        *param_5 = iVar2;
      }
    }
  }
  if (local_51 != '\0') {
    local_50 = *param_5;
    cVar1 = target_dirfd_valid(local_50);
    if (cVar1 != '\x01') {
      local_50 = -100;
      local_40 = local_30;
    }
    local_51 = install_file_in_file(param_1,local_30,local_50,local_40,param_3);
  }
  free(local_30);
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return local_51;
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  bool bVar1;
  bool bVar2;
  bool bVar3;
  bool bVar4;
  char cVar5;
  undefined uVar6;
  int iVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  int *piVar10;
  long in_FS_OFFSET;
  undefined4 local_290;
  int local_28c;
  int local_288;
  int local_284;
  int local_280;
  undefined4 local_27c;
  undefined4 local_278;
  undefined4 local_274;
  undefined4 local_270;
  undefined4 local_26c;
  undefined4 local_268;
  undefined4 local_264;
  undefined4 local_260;
  undefined4 local_25c;
  undefined4 local_258;
  undefined4 local_254;
  undefined4 local_250;
  undefined4 local_24c;
  undefined4 local_248;
  int local_244;
  undefined4 local_240;
  undefined4 local_23c;
  undefined4 local_238;
  undefined4 local_234;
  undefined4 local_230;
  undefined4 local_22c;
  undefined4 local_228;
  undefined4 local_224;
  undefined4 local_220;
  int local_21c;
  undefined4 local_218;
  int local_214;
  undefined4 local_210;
  undefined4 local_20c;
  undefined4 local_208;
  undefined4 local_204;
  undefined4 local_200;
  undefined4 local_1fc;
  undefined4 local_1f8;
  undefined4 local_1f4;
  undefined4 local_1f0;
  undefined4 local_1ec;
  long local_1e8;
  long local_1e0;
  long local_1d8;
  long local_1d0;
  long local_1c8;
  undefined8 *local_1c0;
  long local_1b8;
  void *local_1b0;
  undefined8 local_1a8;
  undefined8 local_1a0;
  undefined8 local_198;
  undefined8 local_190;
  undefined8 local_188;
  undefined8 local_180;
  undefined8 local_178;
  undefined8 local_170;
  undefined8 local_168;
  undefined8 local_160;
  undefined8 local_158;
  undefined8 local_150;
  undefined8 local_148;
  undefined8 local_140;
  undefined8 local_138;
  undefined8 local_130;
  undefined8 local_128;
  undefined8 local_120;
  undefined4 local_118 [8];
  char local_f7;
  long local_f0;
  char local_e5;
  undefined local_d8;
  undefined local_d6;
  undefined local_b8 [152];
  undefined8 local_20;
  
  local_20 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  local_28c = 0;
  local_1e8 = 0;
  bVar1 = false;
  local_1e0 = 0;
  local_1d8 = 0;
  bVar3 = false;
  local_1d0 = 0;
  bVar2 = false;
  bVar4 = false;
  local_1c8 = 0;
  selinux_enabled = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdin);
  cp_option_init(local_118);
  owner_name = 0;
  group_name = 0;
  strip_files = '\0';
  dir_arg = '\0';
  umask(0);
LAB_001023d3:
  local_280 = getopt_long(param_1,param_2,"bcCsDdg:m:o:pt:TvS:Z",long_options,0);
  if (local_280 != -1) {
    if (local_280 < 0x83) {
      if (local_280 < 0x43) {
        if (local_280 == -0x83) {
LAB_00102367:
          uVar8 = proper_name_lite("David MacKenzie","David MacKenzie");
          version_etc(_stdout,"install","GNU coreutils",_Version,uVar8,0);
                    /* WARNING: Subroutine does not return */
          exit(0);
        }
        if (local_280 == -0x82) {
          usage(0);
          goto LAB_00102367;
        }
        goto switchD_001020c9_caseD_45;
      }
      goto code_r0x001020a9;
    }
    goto switchD_001020c9_caseD_45;
  }
  if ((dir_arg != '\0') && (strip_files != '\0')) {
    local_190 = gettext("the strip option may not be used when installing a directory");
    local_260 = 1;
    local_25c = 0;
    error(1,0,local_190);
  }
  if ((dir_arg != '\0') && (local_1d0 != 0)) {
    local_188 = gettext("target directory not allowed when installing a directory");
    local_258 = 1;
    local_254 = 0;
    error(1,0,local_188);
  }
  if (bVar1) {
    uVar8 = gettext("backup type");
    local_118[0] = xget_version(uVar8,local_1d8);
  }
  else {
    local_118[0] = 0;
  }
  set_simple_backup_suffix(local_1e0);
  if ((local_e5 != '\0') && ((local_f0 != 0 || (local_1c8 != 0)))) {
    local_180 = gettext("cannot set target context and preserve it");
    local_250 = 1;
    local_24c = 0;
    error(1,0,local_180);
  }
  if ((local_1c8 != 0) && (iVar7 = setfscreatecon(local_1c8), iVar7 < 0)) {
    uVar8 = quote(local_1c8);
    uVar9 = gettext("failed to set default file creation context to %s");
    piVar10 = __errno_location();
    local_244 = *piVar10;
    local_248 = 1;
    local_178 = uVar9;
    error(1,local_244,uVar9,uVar8);
  }
  local_288 = param_1 - _optind;
  local_1c0 = param_2 + _optind;
  if ((dir_arg == '\x01') || (local_1d0 != 0)) {
    iVar7 = 0;
  }
  else {
    iVar7 = 1;
  }
  if (local_288 <= iVar7) {
    if (local_288 < 1) {
      local_170 = gettext("missing file operand");
      local_240 = 0;
      local_23c = 0;
      error(0,0,local_170);
    }
    else {
      uVar8 = quotearg_style(4,*local_1c0);
      local_168 = gettext("missing destination file operand after %s");
      local_238 = 0;
      local_234 = 0;
      error(0,0,local_168,uVar8);
    }
    usage(1);
  }
  local_290 = 0xffffff9c;
  if (bVar2) {
    if (local_1d0 != 0) {
      local_160 = gettext("cannot combine --target-directory (-t) and --no-target-directory (-T)");
      local_230 = 1;
      local_22c = 0;
      error(1,0,local_160);
    }
    if (local_288 < 3) goto LAB_001029f0;
    uVar8 = quotearg_style(4,local_1c0[2]);
    local_158 = gettext("extra operand %s");
    local_228 = 0;
    local_224 = 0;
    error(0,0,local_158,uVar8);
    usage(1);
  }
  if (local_1d0 != 0) {
    local_290 = target_directory_operand(local_1d0,local_b8);
    cVar5 = target_dirfd_valid(local_290);
    if ((cVar5 == '\x01') || ((bVar3 && (piVar10 = __errno_location(), *piVar10 == 2))))
    goto LAB_001029f0;
    uVar8 = quotearg_style(4,local_1d0);
    uVar9 = gettext("failed to access %s");
    piVar10 = __errno_location();
    local_21c = *piVar10;
    local_220 = 1;
    local_150 = uVar9;
    error(1,local_21c,uVar9,uVar8);
  }
  if (dir_arg != '\x01') {
    local_1b8 = local_1c0[(long)local_288 + -1];
    local_27c = target_directory_operand(local_1b8,local_b8);
    cVar5 = target_dirfd_valid(local_27c);
    if (cVar5 == '\0') {
      if (2 < local_288) {
        uVar8 = quotearg_style(4,local_1b8);
        uVar9 = gettext("target %s");
        piVar10 = __errno_location();
        local_214 = *piVar10;
        local_218 = 1;
        local_148 = uVar9;
        error(1,local_214,uVar9,uVar8);
      }
    }
    else {
      local_290 = local_27c;
      local_1d0 = local_1b8;
      local_288 = local_288 + -1;
    }
  }
LAB_001029f0:
  if (local_1e8 != 0) {
    local_1b0 = (void *)mode_compile(local_1e8);
    if (local_1b0 == (void *)0x0) {
      uVar8 = quote(local_1e8);
      local_140 = gettext("invalid mode %s");
      local_210 = 1;
      local_20c = 0;
      error(1,0,local_140,uVar8);
    }
    mode = mode_adjust(0,0,0,local_1b0,0);
    dir_mode = mode_adjust(0,1,0,local_1b0,&dir_mode_bits);
    free(local_1b0);
  }
  if ((bVar4) && (strip_files != '\x01')) {
    local_138 = gettext("WARNING: ignoring --strip-program option as -s option was not specified");
    local_208 = 0;
    local_204 = 0;
    error(0,0,local_138);
  }
  if ((copy_only_if_needed != '\0') && (local_f7 != '\0')) {
    local_130 = gettext("options --compare (-C) and --preserve-timestamps are mutually exclusive");
    local_200 = 0;
    local_1fc = 0;
    error(0,0,local_130);
    usage(1);
  }
  if ((copy_only_if_needed != '\0') && (strip_files != '\0')) {
    local_128 = gettext("options --compare (-C) and --strip are mutually exclusive");
    local_1f8 = 0;
    local_1f4 = 0;
    error(0,0,local_128);
    usage(1);
  }
  if ((copy_only_if_needed != '\0') && (cVar5 = extra_mode(mode), cVar5 != '\0')) {
    local_120 = gettext(
                       "the --compare (-C) option is ignored when you specify a mode with non-permission bits"
                       );
    local_1f0 = 0;
    local_1ec = 0;
    error(0,0,local_120);
  }
  get_ids();
  if (dir_arg == '\0') {
    hash_init();
    if (local_1d0 == 0) {
      if (bVar3) {
        cVar5 = install_file_in_file_parents(*local_1c0,local_1c0[1],local_118);
      }
      else {
        cVar5 = install_file_in_file(*local_1c0,local_1c0[1],0xffffff9c,local_1c0[1],local_118);
      }
      if (cVar5 != '\x01') {
        local_28c = 1;
      }
    }
    else {
      dest_info_init(local_118);
      for (local_284 = 0; local_284 < local_288; local_284 = local_284 + 1) {
        if ((local_284 == 0) && (bVar3)) {
          uVar6 = 1;
        }
        else {
          uVar6 = 0;
        }
        cVar5 = install_file_in_dir(local_1c0[local_284],local_1d0,local_118,uVar6,&local_290);
        if (cVar5 != '\x01') {
          local_28c = 1;
        }
      }
    }
  }
  else {
    local_28c = savewd_process_files(local_288,local_1c0,process_dir,local_118);
  }
                    /* WARNING: Subroutine does not return */
  exit(local_28c);
code_r0x001020a9:
  switch(local_280) {
  case 0x43:
    copy_only_if_needed = '\x01';
    break;
  case 0x44:
    bVar3 = true;
    break;
  default:
switchD_001020c9_caseD_45:
    usage(1);
    break;
  case 0x53:
    bVar1 = true;
    local_1e0 = _optarg;
    break;
  case 0x54:
    bVar2 = true;
    break;
  case 0x5a:
    if (selinux_enabled == 0) {
      if (_optarg != 0) {
        local_198 = gettext("warning: ignoring --context; it requires an SELinux-enabled kernel");
        local_268 = 0;
        local_264 = 0;
        error(0,0,local_198);
      }
    }
    else {
      use_default_selinux_context = 0;
      if (_optarg == 0) {
        local_f0 = get_labeling_handle();
      }
      else {
        local_1c8 = _optarg;
      }
    }
    break;
  case 0x62:
    bVar1 = true;
    if (_optarg != 0) {
      local_1d8 = _optarg;
    }
    break;
  case 99:
    break;
  case 100:
    dir_arg = '\x01';
    break;
  case 0x67:
    group_name = _optarg;
    break;
  case 0x6d:
    local_1e8 = _optarg;
    break;
  case 0x6f:
    owner_name = _optarg;
    break;
  case 0x70:
    local_f7 = '\x01';
    break;
  case 0x73:
    strip_files = '\x01';
    signal(0x11,(__sighandler_t)0x0);
    break;
  case 0x74:
    if (local_1d0 != 0) {
      local_1a8 = gettext("multiple target directories specified");
      local_278 = 1;
      local_274 = 0;
      error(1,0,local_1a8);
    }
    local_1d0 = _optarg;
    break;
  case 0x76:
    local_d8 = 1;
    break;
  case 0x80:
    local_d8 = 1;
    local_d6 = 1;
    break;
  case 0x81:
    if (selinux_enabled == 0) {
      local_1a0 = gettext("WARNING: ignoring --preserve-context; this kernel is not SELinux-enabled"
                         );
      local_270 = 0;
      local_26c = 0;
      error(0,0,local_1a0);
    }
    else {
      local_e5 = '\x01';
      use_default_selinux_context = 0;
    }
    break;
  case 0x82:
    strip_program = (undefined *)xstrdup(_optarg);
    bVar4 = true;
  }
  goto LAB_001023d3;
}

